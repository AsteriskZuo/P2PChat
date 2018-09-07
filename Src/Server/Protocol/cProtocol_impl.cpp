
#include "stdafx.h"
#include "cProtocol_impl.h"
#include "Packetizer.h"

#include "../ClientHandle.h"
#include "../Root.h"
#include "../Server.h"
#include "../StringCompression.h"


#include "func.h"
#include "ErrorCode.h"



/** The slot number that the client uses to indicate "outside the window". */
static const Int16 SLOT_NUM_OUTSIDE = -999;





#define HANDLE_READ(ByteBuf, Proc, Type, Var) \
	Type Var; \
	if (!ByteBuf.Proc(Var))\
	{\
		return;\
	}





#define HANDLE_PACKET_READ(ByteBuf, Proc, Type, Var) \
	Type Var; \
	{ \
		if (!ByteBuf.Proc(Var)) \
		{ \
			ByteBuf.CheckValid(); \
			return false; \
		} \
		ByteBuf.CheckValid(); \
	}





const int MAX_ENC_LEN = 512;  // Maximum size of the encrypted message; should be 128, but who knows...





// fwd: main.cpp:
extern bool g_ShouldLogCommIn, g_ShouldLogCommOut;





////////////////////////////////////////////////////////////////////////////////
// cProtocol_impl:

cProtocol_impl::cProtocol_impl(cClientHandle * a_Client) 
	: super(a_Client)
	, m_ReceivedData(64 KiB)
	, m_State(1)
{
	// BungeeCord handling:
	// If BC is setup with ip_forward == true, it sends additional data in the login packet's ServerAddress field:
	// hostname\00ip-address\00uuid\00profile-properties-as-json
	AStringVector Params;
	
	// Create the comm log file, if so requested:
	if (g_ShouldLogCommIn || g_ShouldLogCommOut)
	{
		static int sCounter = 0;
		cFile::CreateFolder("CommLogs");
		AString FileName = Printf("CommLogs/%x_%d__%s.log", static_cast<unsigned>(time(nullptr)), sCounter++, a_Client->GetIPString().c_str());
		m_CommLogFile.Open(FileName, cFile::fmWrite);
	}
}

cProtocol_impl::~cProtocol_impl()
{

}


bool cProtocol_impl::HandlePacket(cByteBuffer & a_ByteBuffer, UInt32 a_PacketType)
{
	switch (m_State)
	{
	case 1:
	{
		// Status
		switch (a_PacketType)
		{
		case 0x00: HandlePacketStatusRequest(a_ByteBuffer); return true;
		case 0x01: HandlePacketStatusPing(a_ByteBuffer); return true;
		}
		break;
	}

	case 2:
	{
		// Login
		switch (a_PacketType)
		{
		case 0x00: HandlePacketLoginInfo(a_ByteBuffer); return true;
		}
		break;
	}
	case 3:
	{
		// Work
		switch (a_PacketType)
		{
		case 0x00: HandlePacketKeepAlive(a_ByteBuffer); return true;
		case 0x11: HandlePacketMedia(a_ByteBuffer); return true;
		case 0x12: HandlePacketMediaMsg(a_ByteBuffer); return true;
		}
		break;
	}
	default:
	{
		// Received a packet in an unknown state, report:
		LOGWARNING("Received a packet in an unknown protocol state %d. Ignoring further packets.", m_State);

		// Cannot kick the client - we don't know this state and thus the packet number for the kick packet

		// Switch to a state when all further packets are silently ignored:
		m_State = 255;
		return false;
	}
	case 255:
	{
		// This is the state used for "not processing packets anymore" when we receive a bad packet from a client.
		// Do not output anything (the caller will do that for us), just return failure
		return false;
	}
	}  // switch (m_State)

	// Unknown packet type, report to the ClientHandle:
	m_Client->PacketUnknown(a_PacketType);
	return false;
}

void cProtocol_impl::HandlePacketStatusRequest(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadVarUTF8String, AString, appName);
	HANDLE_READ(a_ByteBuffer, ReadVarUTF8String, AString, server);
	HANDLE_READ(a_ByteBuffer, ReadBEInt32, int, port);
	HANDLE_READ(a_ByteBuffer, ReadBEInt32, int, state);
	if (appName != "p2pchat" || state < 1 || state >2)
	{
		m_Client->Kick(ERROR_CODE_BAD_APP);
		return;
	}
	//下一个状态
	m_State = state;  //wait for client login 

	cPacketizer pkg(*this, 0x00); 	//StatusRequest
	pkg.WriteBEInt32(m_State);		//state
	pkg.WriteString(m_Client->GetIPString());

}

void cProtocol_impl::HandlePacketStatusPing(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadBEInt64, Int64, Timestamp);

	cPacketizer Pkt(*this, 0x01);  // Ping packet
	Pkt.WriteBEInt64(Timestamp);
}

void cProtocol_impl::HandlePacketLoginInfo(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadVarUTF8String, AString, username);
	HANDLE_READ(a_ByteBuffer, ReadVarUTF8String, AString, password);
	m_Client->HandleLogin(username, password);

}

void cProtocol_impl::HandlePacketKeepAlive(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadVarInt32, UInt32, KeepAliveID);
	m_Client->HandleKeepAlive(KeepAliveID);
}

void cProtocol_impl::HandlePacketMedia(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadVarInt32, UInt32, peer_id);
	HANDLE_READ(a_ByteBuffer, ReadVarInt32, UInt32, type);

	cServer *server = cRoot::Get()->GetServer();
	server->ForwardMedia(peer_id, m_Client->GetUniqueID(), type);
}

void cProtocol_impl::HandlePacketMediaMsg(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadVarInt32, UInt32, peer_id);
	HANDLE_READ(a_ByteBuffer, ReadVarUTF8String, AString, strMsg);

	cServer *server = cRoot::Get()->GetServer();
	server->ForwardMediaMsg(peer_id, m_Client->GetUniqueID(), strMsg);
}

void cProtocol_impl::SendKeepAlive(UInt32 a_PingID)
{
	// Drop the packet if the protocol is not in the Game state yet (caused a client crash):
	if (m_State != 3)
	{
		LOGWARNING("Trying to send a KeepAlive packet to a player who's not yet fully logged in (%d). The protocol class prevented the packet.", m_State);
		return;
	}

	cPacketizer Pkt(*this, 0x00);  // Keep Alive packet
	Pkt.WriteVarInt32(a_PingID);
}

void cProtocol_impl::BroadcastUserInfo(const UInt32 a_Id, const AString &a_Name, const UInt32 a_State)
{
	cPacketizer Pkt(*this, 0x01);
	Pkt.WriteVarInt32(a_Id);
	Pkt.WriteString(a_Name);
	Pkt.WriteVarInt32(a_State);
}

void cProtocol_impl::ForwardMedia(UInt32 from_id, UInt32 type)
{
	cPacketizer pkg(*this, 0x11);	//media packet
	pkg.WriteVarInt32(from_id);		//id
	pkg.WriteVarInt32(type);		//type
}

void cProtocol_impl::ForwardMediaMsg(UInt32 from_id, AString msg)
{
	cPacketizer pkg(*this, 0x12);//media packet
	pkg.WriteVarInt32(from_id);		//id
	pkg.WriteString(msg);
}

void cProtocol_impl::SendData(const char * a_Data, size_t a_Size)
{
	m_Client->SendData(a_Data, a_Size);
}

void cProtocol_impl::SendPacket(cPacketizer & a_Packet)
{
	AString LenToSend;
	AString DataToSend;

	// Send the packet length
	UInt32 PacketLen = static_cast<UInt32>(m_OutPacketBuffer.GetUsedSpace());

	m_OutPacketLenBuffer.WriteVarInt32(PacketLen);
	m_OutPacketLenBuffer.ReadAll(LenToSend);
	m_OutPacketLenBuffer.CommitRead();

	// Send the packet data:
	m_OutPacketBuffer.ReadAll(DataToSend);

	DataToSend = LenToSend + DataToSend;
	SendData(DataToSend.data(), DataToSend.size());
	m_OutPacketBuffer.CommitRead();
}

void cProtocol_impl::SendDisconnect(const int & a_Reason)
{
	switch (m_State)
	{
	case 2:
	{
		// During login:
		cPacketizer Pkt(*this, 0);
		Pkt.WriteBEInt32(a_Reason);
		break;
	}
	case 3:
	{
		// In-Work:
		cPacketizer Pkt(*this, 0x40);
		Pkt.WriteBEUInt32(a_Reason);
		break;
	}
	}
}

void cProtocol_impl::DataReceived(const char * a_Data, size_t a_Size)
{
	AddReceivedData(a_Data, a_Size);
}

void cProtocol_impl::AddReceivedData(const char * a_Data, size_t a_Size)
{
	if (!m_ReceivedData.Write(a_Data, a_Size))
	{
		// Too much data in the incoming queue, report to caller:
		m_Client->PacketBufferFull();
		return;
	}

	// Handle all complete packets:
	for (;;)
	{
		UInt32 PacketLen;
		if (!m_ReceivedData.ReadVarInt(PacketLen))
		{
			// Not enough data
			m_ReceivedData.ResetRead();
			break;
		}
		if (!m_ReceivedData.CanReadBytes(PacketLen))
		{
			// The full packet hasn't been received yet
			m_ReceivedData.ResetRead();
			break;
		}
		cByteBuffer bb(PacketLen + 1);
		VERIFY(m_ReceivedData.ReadToByteBuffer(bb, static_cast<size_t>(PacketLen)));
		m_ReceivedData.CommitRead();

		UInt32 PacketType;
		if (!bb.ReadVarInt(PacketType))
		{
			// Not enough data
			break;
		}

		// Write one NUL extra, so that we can detect over-reads
		bb.Write("\0", 1);


		if (!HandlePacket(bb, PacketType))
		{
			// Unknown packet, already been reported, but without the length. Log the length here:
			LOGWARNING("Unhandled packet: type 0x%x, state %d, length %u", PacketType, m_State, PacketLen);

#ifdef _DEBUG
			// Dump the packet contents into the log:
			bb.ResetRead();
			AString Packet;
			bb.ReadAll(Packet);
			Packet.resize(Packet.size() - 1);  // Drop the final NUL pushed there for over-read detection
			AString Out;
			CreateHexDump(Out, Packet.data(), Packet.size(), 24);
			LOGD("Packet contents:\n%s", Out.c_str());
#endif  // _DEBUG


			return;
		}
		int sizes = bb.GetReadableSpace();
		if (sizes != 1)
		{
			// Read more or less than packet length, report as error
			LOGWARNING("Protocol: Wrong number of bytes read for packet 0x%x, state %d. Read " SIZE_T_FMT " bytes, packet contained %u bytes",
				PacketType, m_State, bb.GetUsedSpace() - bb.GetReadableSpace(), PacketLen
				);

			ASSERT(!"Read wrong number of bytes!");
			m_Client->PacketError(PacketType);
		}
	}  // for (ever)

}


void cProtocol_impl::SendLoginSuccess(void)
{
	ASSERT(m_State == 2);  // State: login?

	{
		cPacketizer Pkt(*this, 0x01);  // Login success 
	}

	m_State = 3;  // State = Work


}

AString cProtocol_impl::GetName()
{
	return m_Client->GetUsername();
}

AString cProtocol_impl::GetIPString()
{
	return m_Client->GetIPString();
}

