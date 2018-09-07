/*
* libjingle
* Copyright 2012 Google Inc.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*  3. The name of the author may not be used to endorse or promote products
*     derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "stdafx.h"
#include "peer_connection_client.h"
#include "defaults.h"
#include "func.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/nethelpers.h"
#include "rtc_base/stringutils.h"

#ifdef WIN32
#include "rtc_base/win32socketserver.h"
#endif

#include "ErrorCode.h"
#include "Packetizer.h"

using rtc::sprintfn;

#define HANDLE_READ(ByteBuf, Proc, Type, Var)	\
	Type Var;									\
	if (!ByteBuf.Proc(Var))						\
			{											\
		return;									\
			}

namespace {

	// This is our magical hangup signal.
	const char kByeMessage[] = "BYE";
	// Delay between server connection retries, in milliseconds
	const int kReconnectDelay = 2000;

	rtc::AsyncSocket* CreateClientSocket(int family) {
#ifdef WIN32
		rtc::Win32Socket* sock = new rtc::Win32Socket();
		sock->CreateT(family, SOCK_STREAM);
		return sock;
#elif defined(WEBRTC_POSIX)
		rtc::Thread* thread = rtc::Thread::Current();
		ASSERT(thread != NULL);
		return thread->socketserver()->CreateAsyncSocket(family, SOCK_STREAM);
#else
#error Platform not supported.
#endif
	}

}

PeerConnectionClient::PeerConnectionClient()
	: callback_(NULL)
	, resolver_(NULL)
	, state_(NOT_CONNECTED)
	, my_id_(-1)
	, m_State(1)
	, m_bConnect(false)
	, m_ReceivedData(64 KiB)
	, m_OutPacketBuffer(64 KiB)
{
}


PeerConnectionClient::~PeerConnectionClient()	
{
	
}


void PeerConnectionClient::InitSocketSignals()
{
	ASSERT(control_socket_.get() != NULL);
	control_socket_->SignalCloseEvent.connect(this,
		&PeerConnectionClient::OnClose);
	control_socket_->SignalConnectEvent.connect(this,
		&PeerConnectionClient::OnConnect);
	control_socket_->SignalReadEvent.connect(this,
		&PeerConnectionClient::OnRead);
}

int PeerConnectionClient::id() const
{
	return my_id_;
}

bool PeerConnectionClient::is_connected() const
{
	return m_bConnect;
}

const Peers& PeerConnectionClient::peers() const {
	return peers_;
}

void PeerConnectionClient::RegisterObserver(PeerConnectionClientObserver* callback)
{
	ASSERT(callback);
	callback_ = callback;
}

void PeerConnectionClient::Connect(const AString& server, int port, AString user, AString pass)
{
	ASSERT(!server.empty());
	ASSERT(!user.empty());
	ASSERT(!pass.empty());

	if (state_ != NOT_CONNECTED) {
		RTC_LOG(WARNING)
			<< "The client must not be connected before you can call Connect()";
		callback_->OnServerConnectionFailure();
		return;
	}

	if (server.empty() || user.empty() || pass.empty())
	{
		callback_->OnServerConnectionFailure();
		return;
	}

	if (port <= 0)
		port = PORT;

	server_address_.SetIP(server);
	server_address_.SetPort(port);
	m_strUserName = user;
	m_strPassWord = pass;

	if (server_address_.IsUnresolvedIP())
	{
		state_ = RESOLVING;
		resolver_ = new rtc::AsyncResolver();
		resolver_->SignalDone.connect(this, &PeerConnectionClient::OnResolveResult);
		resolver_->Start(server_address_);
	}
	else {
		DoConnect();
	}
}

bool PeerConnectionClient::RequestToPeer(int peer_id)
{
	cPacketizer pkg(*this, 0x11);//media packet
	pkg.WriteVarInt32(peer_id);		//id
	pkg.WriteVarInt32(0);//请求
	return true;
}


void PeerConnectionClient::OnResolveResult(rtc::AsyncResolverInterface* resolver)
{
	if (resolver_->GetError() != 0)
	{
		callback_->OnServerConnectionFailure();
		resolver_->Destroy(false);
		resolver_ = NULL;
		state_ = NOT_CONNECTED;
	}
	else
	{
		server_address_ = resolver_->address();
		DoConnect();
	}
}

void PeerConnectionClient::DoConnect()
{
	control_socket_.reset(CreateClientSocket(server_address_.ipaddr().family()));
	InitSocketSignals();

	bool ret = ConnectControlSocket();
	if (ret)
		state_ = SIGNING_IN;
	if (!ret) {
		callback_->OnServerConnectionFailure();
	}
}

void PeerConnectionClient::SendData(const char * a_Data, size_t a_Size)
{
	if (!is_connected())
	{
		return;
	}

	ASSERT(control_socket_->GetState() == rtc::Socket::CS_CONNECTED);

	control_socket_->Send(a_Data, a_Size);
}

bool PeerConnectionClient::SendToPeer(int peer_id, const AString& message)
{
	cPacketizer pkg(*this, 0x12);//media packet
	pkg.WriteVarInt32(peer_id);		//id
	pkg.WriteString(message);
	return true;

	return true;
}


bool PeerConnectionClient::SendHangUp(int peer_id) {
	return SendToPeer(peer_id, kByeMessage);
}

bool PeerConnectionClient::IsSendingMessage() {
	return state_ == CONNECTED &&
		control_socket_->GetState() != rtc::Socket::CS_CLOSED;
}

bool PeerConnectionClient::SignOut()
{
	if (state_ == NOT_CONNECTED || state_ == SIGNING_OUT)
		return true;

	if (control_socket_->GetState() == rtc::Socket::CS_CONNECTED) {
		state_ = SIGNING_OUT;
		Close();
		callback_->OnDisconnected();
	}
	else {
		state_ = SIGNING_OUT_WAITING;
	}

	m_strUserName = m_strPassWord = "";

	return true;
}


void PeerConnectionClient::OnMessage(rtc::Message* msg) {
	// ignore msg; there is currently only one supported message ("retry")
	DoConnect();
}


void PeerConnectionClient::Close()
{
	control_socket_->Close();

	if (resolver_ != NULL) {
		resolver_->Destroy(false);
		resolver_ = NULL;
	}
	my_id_ = -1;
	state_ = NOT_CONNECTED;

	m_bConnect = false;

	m_ReceivedData.ClearAll();
	m_OutPacketBuffer.ClearAll();
	m_State = 1;
}

bool PeerConnectionClient::ConnectControlSocket()
{
	ASSERT(control_socket_->GetState() == rtc::Socket::CS_CLOSED);
	int err = control_socket_->Connect(server_address_);
	if (err == SOCKET_ERROR) {
		Close();
		return false;
	}
	return true;
}

void PeerConnectionClient::OnConnect(rtc::AsyncSocket* socket)
{
	m_bConnect = true;

	cPacketizer pkg(*this, 0x00);// Packet type - StatusRequest
	pkg.WriteString("p2pchat");
	pkg.WriteString(SERVER);
	pkg.WriteBEInt32(PORT);
	pkg.WriteBEInt32(2);// NextState - loginStart
}

void PeerConnectionClient::OnRead(rtc::AsyncSocket* socket)
{
	char buffer[0xffff];
	do {
		int bytes = socket->Recv(buffer, sizeof(buffer), nullptr);
		if (bytes <= 0)
			break;

		DataReceived(buffer, bytes);

	} while (true);
}

void PeerConnectionClient::OnClose(rtc::AsyncSocket* socket, int err)
{
	RTC_LOG(INFO) << __FUNCTION__;

	socket->Close();

	Close();

#ifdef WIN32
	if (err != WSAECONNREFUSED) {
#else
	if (err != ECONNREFUSED) {
#endif
		callback_->OnServerConnectionFailure();
	}
	else {
		// 		if (socket == control_socket_.get()) {
		// 			RTC_LOG(WARNING) << "Connection refused; retrying in 2 seconds";
		// 			rtc::Thread::Current()->PostDelayed(kReconnectDelay, this, 0);
		// 		}
		callback_->OnServerConnectionFailure();
	}
}

void PeerConnectionClient::OnMessageFromPeer(int peer_id,
	const AString& message) {
	if (message.length() == (sizeof(kByeMessage) - 1) &&
		message.compare(kByeMessage) == 0) {
		callback_->OnPeerDisconnected(peer_id);
	}
	else {
		callback_->OnMessageFromPeer(peer_id, message);
	}
}

void PeerConnectionClient::DataReceived(const char * a_Data, size_t a_Size)
{
	AddReceivedData(a_Data, a_Size);
}

void PeerConnectionClient::AddReceivedData(const char * a_Data, size_t a_Size)
{
	if (!m_ReceivedData.Write(a_Data, a_Size))
	{
		// Too much data in the incoming queue, report to caller:
		//callback_->PacketBufferFull();
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

			return;
		}

		if (bb.GetReadableSpace() != 1)
		{
			// Read more or less than packet length, report as error
			LOGWARNING("Protocol 1.7: Wrong number of bytes read for packet 0x%x, state %d. Read " SIZE_T_FMT " bytes, packet contained %u bytes",
				PacketType, m_State, bb.GetUsedSpace() - bb.GetReadableSpace(), PacketLen
				);

			ASSERT(!"Read wrong number of bytes!");
		}
	}  // for (ever)
}



bool PeerConnectionClient::HandlePacket(cByteBuffer & a_ByteBuffer, UInt32 a_PacketType)
{
	switch (m_State)
	{
	case 1:
	{
		// Status
		switch (a_PacketType)
		{
 		case 0x00: HandlePacketStatusRequest(a_ByteBuffer); return true;
// 		case 0x01: HandlePacketStatusPing(a_ByteBuffer); return true;
		}
		break;
	}

	case 2:
	{
		// Login
		switch (a_PacketType)
		{
		case 0x00: HandlePacketLoginDisconnect(a_ByteBuffer); return true;
		case 0x01: HandlePacketLoginSuccess(a_ByteBuffer); return true;
		}
		break;
	}

	case 3:
	{
		// Game
		switch (a_PacketType)
		{
		case 0x00: HandlePacketKeepAlive(a_ByteBuffer); return true;
		case 0x01: HandlePacketUserInfo(a_ByteBuffer); return true;			
		case 0x11: HandlePacketMedia(a_ByteBuffer); return true;
		case 0x12: HandlePacketMediaMsg(a_ByteBuffer); return true;
		case 0x40: HandlePacketErrorCode(a_ByteBuffer); return true;
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
	return false;
}

void PeerConnectionClient::HandlePacketStatusRequest(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadBEInt32, int, state);
	HANDLE_READ(a_ByteBuffer, ReadVarUTF8String, AString, strIP);

	if (state>3 || state <1)
	{
		return;
	}

	m_State = state;
	
	//开始登陆
	cPacketizer pkg(*this, 0x00);// Packet type - loginInfo
	pkg.WriteString(m_strUserName);
	pkg.WriteString(m_strPassWord);
}

void PeerConnectionClient::HandlePacketLoginDisconnect(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadBEInt32, int, a_Reason);
	HandleErrorCode(a_Reason);
}

void PeerConnectionClient::HandlePacketLoginSuccess(cByteBuffer & a_ByteBuffer)
{
	m_State = 3;
	callback_->OnSignedIn();
}


void PeerConnectionClient::HandlePacketKeepAlive(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadVarInt32, UInt32, KeepAliveID);
	cPacketizer pkg(*this, 0x00);// Packet type - KeepAlive
	pkg.WriteVarInt32(KeepAliveID);
}

void PeerConnectionClient::HandlePacketUserInfo(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadVarInt32, UInt32, uid);
	HANDLE_READ(a_ByteBuffer, ReadVarUTF8String, AString, name);
	HANDLE_READ(a_ByteBuffer, ReadVarInt32, UInt32, state);

	if (state>3)//offline
	{
		peers_.erase(uid);
		callback_->OnPeerDisconnected(uid);
	}
	else
	{
		peers_[uid] = name;
		callback_->OnPeerConnected(uid, name);
	}

}

void PeerConnectionClient::HandlePacketMedia(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadVarInt32, UInt32, peer_id);
	HANDLE_READ(a_ByteBuffer, ReadVarInt32, UInt32, type);

	if (type == 0)//音视频请求
	{
		int code = callback_->IsBusy() ? -1 : 1;
		cPacketizer pkg(*this, 0x11);// Packet type - KeepAlive
		pkg.WriteVarInt32(peer_id);
		pkg.WriteVarInt32(code);
	}
	else
	{
		callback_->OnPeerResponse(peer_id, type);
	}

}

void PeerConnectionClient::HandlePacketMediaMsg(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadVarInt32, UInt32, peer_id);
	HANDLE_READ(a_ByteBuffer, ReadVarUTF8String, AString, message);

	if (message.length() == (sizeof(kByeMessage) - 1) &&
		message.compare(kByeMessage) == 0) {
		callback_->OnPeerDisconnected(peer_id);
	}
	else {
		callback_->OnMessageFromPeer(peer_id, message);
	}
}

void PeerConnectionClient::HandlePacketErrorCode(cByteBuffer & a_ByteBuffer)
{
	HANDLE_READ(a_ByteBuffer, ReadBEInt32, int, a_Reason);
	HandleErrorCode(a_Reason);
}

void PeerConnectionClient::HandleErrorCode(const int &a_Code)
{
	CString strReason;
	switch (a_Code)
	{
	case  ERROR_CODE_ACCOUNT_NOT_MATCH:
		strReason = _T("用户名密码不匹配！");
		break;
	case ERROR_CODE_ACCOUNT_NOT_EXIST:
		strReason = _T("用户名不存在！");
		break;
	case ERROR_CODE_PACKET_ERROR:
		strReason = _T("协议错误！");
		break;
	case ERROR_CODE_PACKET_UNKONWN:
		strReason = _T("未知协议！");
		break;
	case ERROR_CODE_SERVER_BUSY:
		strReason = _T("服务器繁忙！");
		break;
	case ERROR_CODE_CLIENT_TIMEOUT:
		strReason = _T("哦！你操作超时，稍后再试！");
		break;
	case  ERROR_CODE_ACCOUNT_ALREADY_LOGIN:
		strReason = _T("相同账号已经登陆！");
		break;
	case ERROR_CODE_ACCOUNT_OTHER_LOGIN:
		strReason = _T("相同账号在异地登陆！");
		break;
	case ERROR_CODE_SERVER_SHUTDOWN:
		strReason = _T("服务器已经关闭！");
		break;
	case ERROR_CODE_BAD_APP:
		strReason = _T("非法客户端！");
		break;
	case  ERROR_CODE_INVALID_ENCKEYLENGTH:
		strReason = _T("无效的数据长度！");
		break;
	case  ERROR_CODE_INVALID_ENCNONCELENGTH:
		strReason = _T("无效的数据长度！h");
		break;
	case  ERROR_CODE_HACKED_CLIENT:
		strReason = _T("黑客非法客户端！");
		break;
	default:
		break;
	}
	SignOut();
	callback_->OnDisconnected();
	callback_->PostNotification(strReason);
}