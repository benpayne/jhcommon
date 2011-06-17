
class IORBConnection
{
}


class ObjectManager
{
public:
	ObjectManager() {}
	~ObjectManager() {}
	
	typedef struct {
		std::string server;
		uint32_t	oid;
	} OID;
	
	OID	addObject( void *obj );
	JHCOM::ErrCode remoteObject( OID oid );
	
	void	*getObject( OID );

	void	addConnection( std::string server_name, IORBConnection *connection );
	
	IComponentManager	*getServerComponentManager( std::string server_name );
	
private:
	std::map<OID,SmartPtr<ISuport> >	mObjectMap;
};

class ObjectServer
{
public:
	ObjectServer( int port );
	~ObjectServer();
	
	void Start( IComponentManager *cm );
};

