# CLAUDE.md - JHCommon Development Guide

## Project Overview

JHCommon is a C++ platform abstraction and utility library developed by JetHead Development, Inc. It provides cross-platform primitives for threading, networking, file I/O, eventing, timers, HTTP, URI handling, component management (JHCOM), memory tracing, and logging. The library targets Linux and macOS (Darwin) and is licensed under a BSD 3-clause license.

## Repository Structure

```
jhcommon/
├── include/          # Public header files (*.h)
├── src/              # Library source files (*.cpp) and build configs
├── test/             # Test programs and the TestCase framework
├── jhidl/            # IDL compiler for JHCOM interfaces (uses libIDL)
│   └── test/         # IDL test files
├── darwin_build/     # Xcode project files for macOS builds
│   └── jhCommon/
├── doc/              # Documentation (jh_memory.readme)
├── CMakeLists.txt    # Top-level CMake build file
├── Makefile          # Top-level Makefile (depends on external jhbuild)
├── build.mk          # Build fragment for jhbuild integration
├── build.sh          # Helper script to generate Debug/Release CMake dirs
└── TODO.txt          # Project-level task list
```

## Build System

### CMake (primary)

```bash
# Generate build directories for Debug and Release:
./build.sh

# Build Debug:
cd Debug && make

# Build Release:
cd Release && make
```

The CMake build produces three shared libraries:
- **libjhcommon** - Core library with all platform abstractions
- **libjhcomserver** - Component manager server library
- **libjhcomclient** - Component manager client library

Key CMake variables:
- `PLATFORM_DARWIN` - Defined automatically on macOS
- `JH_VERBOSE_LOGGING` - Enabled in Debug builds, enables full logging and tracing
- Links against pthreads (`find_package(Threads REQUIRED)`)
- On Linux, also links `dl`

### Makefile (legacy/jhbuild)

The Makefile system depends on an external `jhbuild` directory at `../jhbuild/`. This is a legacy build system and may not be available in all environments.

### Build variants

The `build.mk` system defines additional library variants:
- **libjhcommonmd** - Built with `-DUSE_WRAPPERS` (memory debug wrappers)
- **libjhcommongb** - Built with `-DUSE_GUARD_BANDS -DUSE_WRAPPERS` (guard band memory protection)

## Libraries and Key Components

### Core (`include/` + `src/`)

| Component | Headers | Description |
|-----------|---------|-------------|
| **Threading** | `Thread.h`, `Mutex.h`, `Condition.h` | POSIX thread wrappers, mutex with AutoLock, condition variables |
| **Eventing** | `Event.h`, `EventQueue.h`, `EventThread.h`, `EventDispatcher.h`, `EventAgent.h`, `EventAgentT.h` | Event-driven programming framework with queues, dispatchers, and timed/periodic events |
| **Selector** | `Selector.h` | poll()-based I/O multiplexer integrated with the event system |
| **Networking** | `Socket.h`, `MulticastSocket.h` | TCP/UDP socket abstraction with async connect, selector integration |
| **HTTP** | `HttpAgent.h`, `HttpRequest.h`, `HttpResponse.h`, `HttpHeader.h`, `HttpHeaderBase.h` | HTTP client/request/response handling |
| **File I/O** | `File.h`, `FdReaderWriter.h`, `IReaderWriter.h`, `Path.h` | File operations, mmap, pipes, directory listing, path manipulation |
| **URI** | `URI.h` | RFC 2396 URI parsing and generation |
| **Component Model** | `JHCOM.h`, `JHCOM_IDL.h`, `ComponentManager.h`, `ComponentManagerUtils.h` | COM-like component system with ISupports, QueryInterface, AddRef/Release |
| **Memory** | `jh_memory.h`, `Allocator.h` | Memory tracing (mtrace-compatible), guard banding, custom allocators |
| **Logging** | `logging.h` | Per-file log level/category system with runtime control |
| **Utilities** | `jh_types.h`, `jh_string.h`, `jh_list.h`, `jh_vector.h`, `RefCount.h`, `SmartPtr`, `Timer.h`, `TimerManager.h`, `TimeUtils.h`, `Regex.h`, `AppArgs.h`, `CodeTimer.h`, `CircularBuffer.h`, `FieldMap.h`, `GlobalConstructor.h`, `ListenerContainer.h`, `ISerializer.h`, `SerializerBase.h` | Standard types, containers, reference counting, smart pointers, timers, regex |

### Error Handling

Use `JetHead::ErrCode` (defined in `JetHead.h`) for return values. Error codes include `kNoError`, `kNotFound`, `kConnectionFailed`, `kTimedOut`, `kOpenFailed`, etc. Map errno values using `JetHead::getErrorCode()`.

## Coding Conventions

### Naming

- **Classes**: PascalCase (`EventThread`, `ServerSocket`, `TimerManager`)
- **Methods**: camelCase (`getPort()`, `setSelector()`, `handleData()`)
- **Member variables**: `m` prefix + PascalCase (`mRefCount`, `mSelector`, `mSockStream`)
- **Constants/Enums**: `k` prefix + PascalCase (`kNoError`, `kMaxPollFds`)
- **Macros**: ALL_CAPS with underscores (`LOG_LVL_ERR`, `JH_ARRAY_SIZE`)
- **Type aliases**: snake_case with `_t` suffix for C types (`jh_ptr_int_t`, `jh_off64_t`)
- **Namespaces**: PascalCase (`JetHead`, `JHCOM`, `JHThread`)
- **Files**: PascalCase for C++ classes (`EventThread.cpp`), lowercase with `jh_` prefix for C-style utilities (`jh_memory.cpp`, `jh_types.h`)

### Header Guards

Use `#ifndef` / `#define` style (not `#pragma once`):
```cpp
#ifndef JH_SOCKET_H_
#define JH_SOCKET_H_
// ...
#endif // JH_SOCKET_H_
```

### License Header

All files must include the BSD 3-clause copyright header at the top:
```cpp
/*
 * Copyright (c) 2010, JetHead Development, Inc.
 * All rights reserved.
 * ...
 */
```

### Formatting

- Tab indentation (not spaces)
- Spaces inside parentheses: `if ( condition )`, `func( arg1, arg2 )`
- Opening brace on same line for control structures, next line for function definitions
- Doxygen-style comments with `@param`, `@return`, `@brief` tags
- `//!` for inline member documentation

### Interfaces and Patterns

- **Listener pattern**: Classes ending in `Listener` (e.g., `SocketListener`, `FileListener`, `IEventListener`) define callback interfaces with virtual methods prefixed `handle` (e.g., `handleData`, `handleClose`, `handleAccept`)
- **Reference counting**: Use `RefCount` base class or `JHCOM::ISupports` with `AddRef()`/`Release()`. Use `SmartPtr<T>` for automatic reference management
- **JHCOM**: COM-like component model. Use `JHCOM_DEFINE_IID()`, `JHCOM_DECL_ISUPPORTS*` macros. Implement `QueryInterface` via `JHCOM_IMPL_QUERY_INTERFACE*` macros
- **Event system**: Derive events from `Event`, use `SMART_CASTABLE(id)` macro, cast with `event_cast<T>()`
- **Selector integration**: Classes that participate in I/O multiplexing implement `SelectorListener::processFileEvents()`

### Logging

Every `.cpp` file that uses logging should set its category and level at the top:
```cpp
#include "logging.h"
SET_LOG_CAT( LOG_CAT_ALL );
SET_LOG_LEVEL( LOG_LVL_NOTICE );
```

Do NOT include `logging.h` in header files. Use the level-specific macros:
- `LOG_ERR(fmt, ...)` / `LOG_ERR_PERROR(fmt, ...)` - Always enabled
- `LOG_WARN(fmt, ...)` / `LOG_WARN_PERROR(fmt, ...)` - Always enabled
- `LOG_NOTICE(fmt, ...)` - Startup/shutdown messages
- `LOG_INFO(fmt, ...)` - Infrequent operations
- `LOG_NOISE(fmt, ...)` - High-frequency streaming data
- `TRACE_BEGIN(level)` / `TRACE_END()` - Function entry/exit tracing (Debug only)

### Platform Handling

- Use `#ifdef PLATFORM_DARWIN` for macOS-specific code
- The library wraps POSIX APIs; avoid direct system calls where a JHCommon wrapper exists

## Testing

### Test Framework

Tests use a custom framework defined in `test/TestCase.h`:
- Derive test classes from `TestCase`
- Call `SetTestName()` in the constructor
- Implement `Run()` as the test body
- Call `TestPassed()` on success or `TestFailed(fmt, ...)` on failure
- Group tests in a `TestSuite` and execute with `TestRunner`
- Each test runs in its own thread

### Running Tests

Tests are individual executables built by CMake in the `test/` directory:
```bash
cd Debug && make
./listenerContainerTest
./eventThreadTest
./selectorTest
./timerTest
./URITest
./FileTest
./regexTest
./stringTest
./pathTest
./circularBufTest
./allocatorTest
# etc.
```

### Test Guidelines

1. Test every method with positive and negative cases
2. Test classes in isolation where possible
3. Don't bypass JHCommon APIs to validate results
4. Use the TestCase/TestSuite/TestRunner framework for all new tests
5. Tests must be self-sufficient - no dependency on other test execution order
6. Create and destroy all objects within the test case, before calling `TestPassed`
7. Prefer table-driven tests for easy extension and low code complexity

### Test Quality Ratings

Tests are rated G (good), S (so-so), N (needs improvement), B (bad). See `test/README.txt` for the full assessment. Well-rated tests: `allocatorTest`, `circularBufTest`, `FileTest`, `regexTest`, `stringTest`, `URITest`.

## Dependencies

- **POSIX**: pthreads, poll, sockets, file I/O
- **C++ Standard Library**: Minimal usage; project provides its own `jh_list.h`, `jh_vector.h`, `jh_string.h` wrappers
- **libIDL** (optional): Required only for the `jhidl` IDL compiler
- **CMake >= 2.6**: Build system
- No external package manager; all core dependencies are system-level
