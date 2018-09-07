
// cServer.h

// Interfaces to the cServer object representing the network server





#pragma once

#include "OSSupport/IsThread.h"
#include "OSSupport/Network.h"

#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable:4127)
	#pragma warning(disable:4244)
	#pragma warning(disable:4231)
	#pragma warning(disable:4189)
	#pragma warning(disable:4702)
#endif

#ifdef _MSC_VER
	#pragma warning(pop)
#endif





// fwd:
class cClientHandle;
typedef std::shared_ptr<cClientHandle> cClientHandlePtr;
typedef std::list<cClientHandlePtr> cClientHandlePtrs;
typedef std::list<cClientHandle *> cClientHandles;
class cCommandOutputCallback;
class cSettingsRepositoryInterface;


namespace Json
{
	class Value;
}


class cServer
{
public:
	virtual ~cServer() {}
	bool InitServer(cSettingsRepositoryInterface & a_Settings, bool a_ShouldAuth);

	// Player counts:
	int  GetNumPlayers(void) const;
	
	/** Check if the player is queued to be transferred to a World.
	Returns true is Player is found in queue. */
	bool IsPlayerInQueue(AString a_Username);
	
	// tolua_end

	bool Start(void);
	
	/** Executes the console command, sends output through the specified callback */
	void ExecuteConsoleCommand(const AString & a_Cmd, cCommandOutputCallback & a_Output);
	
	/** Lists all available console commands and their helpstrings */
	void PrintHelp(const AStringVector & a_Split, cCommandOutputCallback & a_Output);

	void Shutdown(void);

	void KickUser(int a_ClientID, const int & a_Reason);
	
	/** Authenticates the specified user, called by cAuthenticator */
	void AuthenticateUser(int a_ClientID, const AString & a_Name, const AString & a_PassWord, const Json::Value & a_Properties);

	const AString & GetServerID(void) const { return m_ServerID; }  // tolua_export

	//¹ã²¥
	void BroadcastUserInfo(cClientHandle* pClient);
	//×ª·¢
	void ForwardMedia(UInt32 to_id, UInt32 from_id, UInt32 type);
	void ForwardMediaMsg(UInt32 to_id, UInt32 from_id, AString msg);
	
private:

	friend class cRoot;  // so cRoot can create and destroy cServer
	friend class cServerListenCallbacks;  // Accessing OnConnectionAccepted()
	
	/** The server tick thread takes care of the players who aren't yet spawned in a world */
	class cTickThread :
		public cIsThread
	{
		typedef cIsThread super;
		
	public:
		cTickThread(cServer & a_Server);
		
	protected:
		cServer & m_Server;
		
		// cIsThread overrides:
		virtual void Execute(void) override;
	} ;
	
	
	/** The network sockets listening for client connections. */
	cServerHandlePtr m_ServerHandle;
		
	/** Protects m_Clients and m_ClientsToRemove against multithreaded access. */
	cCriticalSection m_CSClients;

	/** Clients that are connected to the server and not yet assigned to a cWorld. */
	cClientHandlePtrs m_Clients;

	/** Clients that have just been moved into a world and are to be removed from m_Clients in the next Tick(). */
	cClientHandles m_ClientsToRemove;
	
	/** Protects m_PlayerCount against multithreaded access. */
	mutable cCriticalSection m_CSPlayerCount;

	/** Number of players currently playing in the server. */
	int m_PlayerCount;

	/** Protects m_PlayerCountDiff against multithreaded access. */
	cCriticalSection m_CSPlayerCountDiff;

	/** Adjustment to m_PlayerCount to be applied in the Tick thread. */
	int m_PlayerCountDiff;


	bool m_bIsConnected;  // true - connected false - not connected

	bool m_bRestarting;

	
	/** Public key for m_PrivateKey, ASN1-DER-encoded */
	AString m_PublicKeyDER;

	cTickThread m_TickThread;
	cEvent m_RestartEvent;
	
	/** The server ID used for client authentication */
	AString m_ServerID;

	/** The list of ports on which the server should listen for connections.
	Initialized in InitServer(), used in Start(). */
	int m_Port;

	cServer(void);


	/** Creates a new cClientHandle instance and adds it to the list of clients.
	Returns the cClientHandle reinterpreted as cTCPLink callbacks. */
	cTCPLink::cCallbacksPtr OnConnectionAccepted(const AString & a_RemoteIPAddress);
	
	bool Tick(float a_Dt);
	
	/** Ticks the clients in m_Clients, manages the list in respect to removing clients */
	void TickClients(float a_Dt);
};  // tolua_export




