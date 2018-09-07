
#ifndef TALK_EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
#define TALK_EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
#pragma once

#include "api/mediastreaminterface.h"
#include "api/peerconnectioninterface.h"
#include "MainFrame.h"
#include "peer_connection_client.h"


namespace webrtc {
	class VideoCaptureModule;
}  // namespace webrtc

namespace cricket {
	class VideoRenderer;
}  // namespace cricket

class Conductor
	: public webrtc::PeerConnectionObserver,
	public webrtc::CreateSessionDescriptionObserver,
	public PeerConnectionClientObserver,
	public MainWndCallback {
public:
	enum CallbackID {
		MEDIA_CHANNELS_INITIALIZED = 1,
		PEER_CONNECTION_CLOSED,
		SEND_MESSAGE_TO_PEER,
		NEW_STREAM_ADDED,
		STREAM_REMOVED,
	};

	Conductor(PeerConnectionClient* client, MainWindow* main_wnd);

	bool connection_active() const;

protected:
	~Conductor();
	bool InitializePeerConnection();
	bool CreatePeerConnection(bool dtls);
	void DeletePeerConnection();
	void EnsureStreamingUI();
	void AddStreams();
	std::unique_ptr<cricket::VideoCapturer> OpenVideoCaptureDevice();

	//
	// PeerConnectionObserver implementation.
	//
	void OnSignalingChange(
		webrtc::PeerConnectionInterface::SignalingState new_state) override {};
	void OnAddStream(
		rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
	void OnRemoveStream(
		rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
	void OnDataChannel(
		rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
	void OnRenegotiationNeeded() override {}
	void OnIceConnectionChange(
		webrtc::PeerConnectionInterface::IceConnectionState new_state) override {};
	void OnIceGatheringChange(
		webrtc::PeerConnectionInterface::IceGatheringState new_state);
	void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
	void OnIceConnectionReceivingChange(bool receiving) override {}

	//
	// PeerConnectionClientObserver implementation.
	//
	virtual void OnSignedIn();

	virtual bool IsBusy();

	virtual void OnDisconnected();

	virtual void OnPeerConnected(int id, const AString& name);

	virtual void OnPeerDisconnected(int id);

	virtual void OnPeerResponse(int peer_id, int code);

	virtual void OnMessageFromPeer(int peer_id, const AString& message);

	virtual void OnMessageSent(int err);

	virtual void OnServerConnectionFailure();

	virtual void PostNotification(const CString& message);

	//
	// MainWndCallback implementation.
	//

	virtual void StartLogin(const AString& server, const AString& username, const AString& password);

	virtual void DisconnectFromServer();

	virtual void ConnectToPeer(int peer_id);

	virtual void DisconnectFromCurrentPeer();

	virtual void UIThreadCallback(int msg_id, void* data);

	virtual void Close();
	// CreateSessionDescriptionObserver implementation.
	virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
	virtual void OnFailure(const AString& error);

protected:
	// Send a message to the remote peer.
	void SendMessage(const AString& json_object);

	int peer_id_;
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
		peer_connection_factory_;
	PeerConnectionClient* client_;
	MainWindow* main_wnd_;
	std::deque<AString*> pending_messages_;
	std::map<AString, rtc::scoped_refptr<webrtc::MediaStreamInterface> >
		active_streams_;
	AString server_;
};

#endif  // TALK_EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
