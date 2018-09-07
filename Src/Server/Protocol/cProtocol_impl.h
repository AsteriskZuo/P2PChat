
// Protocol17x.h



#pragma once

#include "Protocol.h"
#include "../ByteBuffer.h"

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
namespace Json
{
	class Value;
}





class cProtocol_impl :
	public cProtocol
{
	typedef cProtocol super;
	
public:

	cProtocol_impl(cClientHandle * a_Client);
	~cProtocol_impl();


	/** Reads and handles the packet. The packet length and type have already been read.
	Returns true if the packet was understood, false if it was an unknown packet
	*/
	bool HandlePacket(cByteBuffer & a_ByteBuffer, UInt32 a_PacketType);
	/** Called when client sends some data: */
	virtual void DataReceived(const char * a_Data, size_t a_Size) override;
	/** Sending stuff to clients (alphabetically sorted): */
	virtual void SendDisconnect(const int & a_Reason);
	virtual void SendLoginSuccess(void);
	virtual void SendKeepAlive(UInt32 a_PingID);
	virtual void BroadcastUserInfo(const UInt32 a_Id, const AString &a_Name, const UInt32 a_State);
	//×ª·¢
	virtual void ForwardMedia(UInt32 from_id, UInt32 type);
	virtual void ForwardMediaMsg(UInt32 from_id, AString msg);
	virtual AString GetName();
	virtual AString GetIPString();
	
protected:
	// Packet handlers while in the Status state (m_State == 1):
	void HandlePacketStatusRequest(cByteBuffer & a_ByteBuffer);
	void HandlePacketStatusPing(cByteBuffer & a_ByteBuffer);	
	// Packet handlers while in the Login state (m_State == 2):
	void HandlePacketLoginInfo(cByteBuffer & a_ByteBuffer);
	// Packet handlers while in the Game state (m_State == 3):
	void HandlePacketKeepAlive(cByteBuffer & a_ByteBuffer);
	void HandlePacketMedia(cByteBuffer & a_ByteBuffer);
	void HandlePacketMediaMsg(cByteBuffer & a_ByteBuffer);

private:
	
	/** Adds the received (unencrypted) data to m_ReceivedData, parses complete packets */
	void AddReceivedData(const char * a_Data, size_t a_Size);

	/** Sends the data to the client, encrypting them if needed. */
	virtual void SendData(const char * a_Data, size_t a_Size) override;

	/** Sends the packet to the client. Called by the cPacketizer's destructor. */
	virtual void SendPacket(cPacketizer & a_Packet) override;	

protected:
	/** State of the protocol. 1 = status, 2 = login, 3 = work */
	UInt32 m_State;

	/** Buffer for the received data */
	cByteBuffer m_ReceivedData;

	/** The logfile where the comm is logged, when g_ShouldLogComm is true */
	cFile m_CommLogFile;
} ;


