/*
* libjingle
* Copyright 2011 Google Inc.
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

#ifndef PEERCONNECTION_SAMPLES_CLIENT_PEER_CONNECTION_CLIENT_H_
#define PEERCONNECTION_SAMPLES_CLIENT_PEER_CONNECTION_CLIENT_H_
#pragma once

#include <map>
#include <string>
#include "ByteBuffer.h"
#include "rtc_base/nethelpers.h"
#include "rtc_base/physicalsocketserver.h"
#include "rtc_base/signalthread.h"
#include "rtc_base/sigslot.h"

class cAesCfb128Decryptor;
class cAesCfb128Encryptor;


typedef std::map<int, std::string> Peers;

struct PeerConnectionClientObserver {
	virtual void OnDisconnected() = 0;
	virtual void OnSignedIn() = 0;  // Called when we're logged on.
	virtual bool IsBusy() = 0;
	virtual void OnPeerConnected(int id, const std::string& name) = 0;
	virtual void OnPeerDisconnected(int peer_id) = 0;
	virtual void OnPeerResponse(int peer_id,int code) = 0;
	virtual void OnMessageFromPeer(int peer_id, const std::string& message) = 0;
	virtual void OnMessageSent(int err) = 0;
	virtual void OnServerConnectionFailure() = 0;
	virtual void PostNotification(const CString& message) = 0;
protected:
	virtual ~PeerConnectionClientObserver() {}
};

class PeerConnectionClient : public sigslot::has_slots<>, public rtc::MessageHandler 
{
	friend class cPacketizer;

public:
	enum State {
		NOT_CONNECTED,
		RESOLVING,
		SIGNING_IN,
		CONNECTED,
		SIGNING_OUT_WAITING,
		SIGNING_OUT,
	};

	PeerConnectionClient();
	virtual ~PeerConnectionClient();

public:
	int id() const;
	bool is_connected() const;
	const Peers& peers() const;

	void RegisterObserver(PeerConnectionClientObserver* callback);

	void Connect(const AString& server, int port, AString user, AString pass);
	bool RequestToPeer(int peer_id);
	void SendData(const char * a_Data, size_t a_Size);
	bool SendToPeer(int peer_id, const AString& message);
	bool SendHangUp(int peer_id);
	bool IsSendingMessage();

	bool SignOut();

	// implements the MessageHandler interface
	void OnMessage(rtc::Message* msg);

protected:
	void DoConnect();
	void Close();
	void InitSocketSignals();
	bool ConnectControlSocket();
	void OnConnect(rtc::AsyncSocket* socket);
	void OnRead(rtc::AsyncSocket* socket);
	void OnClose(rtc::AsyncSocket* socket, int err);
	void OnResolveResult(rtc::AsyncResolverInterface* resolver);
	void OnMessageFromPeer(int peer_id, const AString& message);

protected:
	void DataReceived(const char * a_Data, size_t a_Size);
	void AddReceivedData(const char * a_Data, size_t a_Size);
	bool HandlePacket(cByteBuffer & a_ByteBuffer, UInt32 a_PacketType);
private:

	void HandlePacketStatusRequest(cByteBuffer & a_ByteBuffer);

	void HandlePacketLoginDisconnect(cByteBuffer & a_ByteBuffer);
	void HandlePacketLoginSuccess(cByteBuffer & a_ByteBuffer);


	void HandlePacketKeepAlive(cByteBuffer & a_ByteBuffer);
	void HandlePacketUserInfo(cByteBuffer & a_ByteBuffer);
	void HandlePacketMedia(cByteBuffer & a_ByteBuffer);
	void HandlePacketMediaMsg(cByteBuffer & a_ByteBuffer);
	void HandlePacketErrorCode(cByteBuffer & a_ByteBuffer);

private:
	void HandleErrorCode(const int &a_Code);

protected:
	/** Buffer for the received data */
	cByteBuffer m_ReceivedData;
	cCriticalSection m_CSPacket;
	/** Buffer for composing the outgoing packets, through cPacketizer */
	cByteBuffer m_OutPacketBuffer;
private:
	PeerConnectionClientObserver* callback_;
	rtc::SocketAddress server_address_;
	rtc::AsyncResolver* resolver_;
	std::unique_ptr<rtc::AsyncSocket> control_socket_;
	State state_;
	int my_id_;

	Peers peers_;

	bool m_bConnect;

	AString m_strUserName;
	AString m_strPassWord;

	/** State of the protocol. 1 = status, 2 = login, 3 = work */
	UInt32 m_State;
};

#endif
