﻿#include "StdAfx.h"
#include "conductor.h"

#include "defaults.h"

#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/test/fakeconstraints.h"
// #include "api/video_codecs/builtin_video_decoder_factory.h"
// #include "api/video_codecs/builtin_video_encoder_factory.h"
#include "media/engine/webrtcvideocapturerfactory.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture_factory.h"
#include "rtc_base/checks.h"
#include "rtc_base/json.h"
#include "rtc_base/logging.h"


Json::Value& Json::Value::operator=(Json::Value other) {
	swap(other);
	return *this;
}

// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

#define DTLS_ON  true
#define DTLS_OFF false

class DummySetSessionDescriptionObserver
	: public webrtc::SetSessionDescriptionObserver {
public:
	static DummySetSessionDescriptionObserver* Create() {
		return
			new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
	}
	virtual void OnSuccess() {
		RTC_LOG(INFO) << __FUNCTION__;
	}
	virtual void OnFailure(const std::string& error) {
		RTC_LOG(INFO) << __FUNCTION__ << " " << error;
	}

protected:
	DummySetSessionDescriptionObserver() {}
	~DummySetSessionDescriptionObserver() {}
};

Conductor::Conductor(PeerConnectionClient* client, MainWindow* main_wnd)
	: peer_id_(-1),
	client_(client),
	main_wnd_(main_wnd) {
	client_->RegisterObserver(this);
	main_wnd->RegisterObserver(this);
}

Conductor::~Conductor() {
	RTC_DCHECK(peer_connection_.get() == NULL);
}

bool Conductor::connection_active() const {
	return peer_connection_.get() != NULL;
}

bool Conductor::InitializePeerConnection() {
	RTC_DCHECK(peer_connection_factory_.get() == NULL);
	RTC_DCHECK(peer_connection_.get() == NULL);

	peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
		nullptr /* network_thread */, nullptr /* worker_thread */,
		nullptr /* signaling_thread */, nullptr /* default_adm */,
		webrtc::CreateBuiltinAudioEncoderFactory(),
		webrtc::CreateBuiltinAudioDecoderFactory(),
		nullptr,
		nullptr, nullptr /* audio_mixer */,
		nullptr /* audio_processing */);

	if (!peer_connection_factory_.get()) {
		main_wnd_->MessageBox("Error",
			"Failed to initialize PeerConnectionFactory", true);
		DeletePeerConnection();
		return false;
	}

	if (!CreatePeerConnection(DTLS_ON)) {
		main_wnd_->MessageBox("Error",
			"CreatePeerConnection failed", true);
		DeletePeerConnection();
	}
	AddStreams();
	return peer_connection_.get() != NULL;
}

bool Conductor::CreatePeerConnection(bool dtls) {
	RTC_DCHECK(peer_connection_factory_.get() != NULL);
	RTC_DCHECK(peer_connection_.get() == NULL);
	webrtc::PeerConnectionInterface::RTCConfiguration config;
	webrtc::PeerConnectionInterface::IceServer server;
	server.uri = GetStunServerString();
	config.servers.push_back(server);

	webrtc::PeerConnectionInterface::IceServer server2;
	server2.uri = GetTurnServerString();
	server2.password = "ling1234";
	server2.username = "ling";
	config.servers.push_back(server2);

	webrtc::FakeConstraints constraints;
	if (dtls) {
		constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp,
			"true");
	}
	else {
		constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp,
			"false");
	}
	peer_connection_ = peer_connection_factory_->CreatePeerConnection(
		config, &constraints, NULL, NULL, this);
	return peer_connection_.get() != NULL;
}

void Conductor::DeletePeerConnection() {
	peer_connection_ = NULL;
	active_streams_.clear();
	main_wnd_->StopLocalRenderer();
	main_wnd_->StopRemoteRenderer();
	peer_connection_factory_ = NULL;
	peer_id_ = -1;
}

void Conductor::EnsureStreamingUI() {
	RTC_DCHECK(peer_connection_.get() != NULL);
	if (main_wnd_->IsWindow()) {
		if (main_wnd_->current_ui() != MainWindow::STREAMING)
			main_wnd_->SwitchToStreamingUI();
	}
}

//
// PeerConnectionObserver implementation.
//

// Called when a remote stream is added
void Conductor::OnAddStream(
	rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
	RTC_LOG(INFO) << __FUNCTION__ << " " << stream->label();
	main_wnd_->QueueUIThreadCallback(NEW_STREAM_ADDED, stream.release());
}

void Conductor::OnRemoveStream(
	rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
	RTC_LOG(INFO) << __FUNCTION__ << " " << stream->label();
	main_wnd_->QueueUIThreadCallback(STREAM_REMOVED, stream.release());
}

void Conductor::OnIceGatheringChange(
	webrtc::PeerConnectionInterface::IceGatheringState new_state)
{
	if (new_state == webrtc::PeerConnectionInterface::kIceGatheringComplete)
	{
		RTC_LOG(INFO) << __FUNCTION__;

		AString sdp;
		auto desc = peer_connection_->local_description();
		desc->ToString(&sdp);

		RTC_LOG(INFO) << sdp;

		Json::StyledWriter writer;
		Json::Value jmessage;
		jmessage[kSessionDescriptionTypeName] = desc->type();
		jmessage[kSessionDescriptionSdpName] = sdp;
		SendMessage(writer.write(jmessage));
	}
}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
	RTC_LOG(INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();
	// For loopback test. To save some connecting delay.

	Json::StyledWriter writer;
	Json::Value jmessage;

	jmessage[kCandidateSdpMidName] = candidate->sdp_mid();
	jmessage[kSessionDescriptionTypeName] = kCandidateSdpName;
	jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();
	std::string sdp;
	if (!candidate->ToString(&sdp)) {
		RTC_LOG(LS_ERROR) << "Failed to serialize candidate";
		return;
	}
	jmessage[kCandidateSdpName] = sdp;
//	SendMessage(writer.write(jmessage));
}

//
// PeerConnectionClientObserver implementation.
//
void Conductor::OnSignedIn() {
	RTC_LOG(INFO) << __FUNCTION__;
	main_wnd_->SwitchToPeerList(client_->peers());
}

bool Conductor::IsBusy()
{
	return peer_id_ != -1 && peer_connection_.get() != nullptr;
}

void Conductor::OnDisconnected() {
	RTC_LOG(INFO) << __FUNCTION__;

	DeletePeerConnection();

	if (main_wnd_->IsWindow())
		main_wnd_->SwitchToConnectUI();
}

void Conductor::OnPeerConnected(int id, const std::string& name) {
	RTC_LOG(INFO) << __FUNCTION__;
	// Refresh the list if we're showing it.
	if (main_wnd_->current_ui() == MainWindow::LIST_PEERS)
		main_wnd_->SwitchToPeerList(client_->peers());
}

void Conductor::OnPeerDisconnected(int id) {
	RTC_LOG(INFO) << __FUNCTION__;
	if (id == peer_id_) {
		RTC_LOG(INFO) << "Our peer disconnected";
		main_wnd_->QueueUIThreadCallback(PEER_CONNECTION_CLOSED, NULL);
	}
	else {
		// Refresh the list if we're showing it.
		if (main_wnd_->current_ui() == MainWindow::LIST_PEERS)
			main_wnd_->SwitchToPeerList(client_->peers());
	}
}

void Conductor::OnPeerResponse(int peer_id, int code)
{
	RTC_DCHECK(peer_id_ == peer_id || peer_id_ == -1);
	if (code == 1)//对方同意通话
	{
		//修改来自 ConnectToPeer
		//
		if (peer_connection_.get()) {
			main_wnd_->MessageBox("Error",
				"We only support connecting to one peer at a time", true);
			return;
		}

		if (InitializePeerConnection()) {
			peer_id_ = peer_id;
			peer_connection_->CreateOffer(this, NULL);
		}
		else {
			main_wnd_->MessageBox("Error", "Failed to initialize PeerConnection", true);
		}
	}
	else if(code == -1)//拒绝同意通话
	{
		peer_id_ = -1;
	}

}

void Conductor::OnMessageFromPeer(int peer_id, const std::string& message) {
	RTC_DCHECK(peer_id_ == peer_id || peer_id_ == -1);
	RTC_DCHECK(!message.empty());

	if (!peer_connection_.get()) {
		RTC_DCHECK(peer_id_ == -1);
		peer_id_ = peer_id;

		if (!InitializePeerConnection()) {
			RTC_LOG(LS_ERROR) << "Failed to initialize our PeerConnection instance";
			client_->SignOut();
			return;
		}
	}
	else if (peer_id != peer_id_) {
		RTC_DCHECK(peer_id_ != -1);
		RTC_LOG(WARNING) << "Received a message from unknown peer while already in a "
			"conversation with a different peer.";
		return;
	}

	Json::Reader reader;
	Json::Value jmessage;
	if (!reader.parse(message, jmessage)) {
		RTC_LOG(WARNING) << "Received unknown message. " << message;
		return;
	}
	std::string type;
	std::string json_object;

	rtc::GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName, &type);
	if (type != kCandidateSdpName) {
		
		std::string sdp;
		if (!rtc::GetStringFromJsonObject(jmessage, kSessionDescriptionSdpName,
			&sdp)) {
			RTC_LOG(WARNING) << "Can't parse received session description message.";
			return;
		}
		webrtc::SdpParseError error;
		webrtc::SessionDescriptionInterface* session_description(
			webrtc::CreateSessionDescription(type, sdp, &error));
		if (!session_description) {
			RTC_LOG(WARNING) << "Can't parse received session description message.";
			return;
		}
		RTC_LOG(INFO) << " Received session description :" << message;
		peer_connection_->SetRemoteDescription(
			DummySetSessionDescriptionObserver::Create(), session_description);
		if (session_description->type() ==
			webrtc::SessionDescriptionInterface::kOffer) {
			peer_connection_->CreateAnswer(this, NULL);
		}
		return;
	}
	else {
		std::string sdp_mid;
		int sdp_mlineindex = 0;
		std::string sdp;
		if (!rtc::GetStringFromJsonObject(jmessage, kCandidateSdpMidName,
			&sdp_mid) ||
			!rtc::GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName,
			&sdp_mlineindex) ||
			!rtc::GetStringFromJsonObject(jmessage, kCandidateSdpName, &sdp)) {
			RTC_LOG(WARNING) << "Can't parse received message.";
			return;
		}
		webrtc::SdpParseError error;
		std::unique_ptr<webrtc::IceCandidateInterface> candidate(
			webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, &error));
		if (!candidate.get()) {
			RTC_LOG(WARNING) << "Can't parse received candidate message.";
			return;
		}
		if (!peer_connection_->AddIceCandidate(candidate.get())) {
			RTC_LOG(WARNING) << "Failed to apply the received candidate";
			return;
		}
		RTC_LOG(INFO) << " Received candidate :" << message;
		return;
	}
}

void Conductor::OnMessageSent(int err) {
	// Process the next pending message if any.
	main_wnd_->QueueUIThreadCallback(SEND_MESSAGE_TO_PEER, NULL);
}

void Conductor::OnServerConnectionFailure() {
	main_wnd_->MessageBox("Error", ("Failed to connect to " + server_).c_str(),
		true);
}

void Conductor::PostNotification(const CString& message)
{
	main_wnd_->PostNotification(message);
}

//
// MainWndCallback implementation.
//

void Conductor::StartLogin(const AString& server, const std::string& username, const std::string& password) {
	if (client_->is_connected())
		return;
	server_ = server;
	client_->Connect(server_, PORT, username, password);
}

void Conductor::DisconnectFromServer() {
	if (client_->is_connected())
		client_->SignOut();
}

void Conductor::ConnectToPeer(int peer_id) {
	RTC_DCHECK(peer_id_ == -1);
	RTC_DCHECK(peer_id != -1);

	//请求通话
	client_->RequestToPeer(peer_id);
	//

// 	if (peer_connection_.get()) {
// 		main_wnd_->MessageBox("Error",
// 			"We only support connecting to one peer at a time", true);
// 		return;
// 	}
// 
// 	if (InitializePeerConnection()) {
// 		peer_id_ = peer_id;
// 		peer_connection_->CreateOffer(this, NULL);
// 	}
// 	else {
// 		main_wnd_->MessageBox("Error", "Failed to initialize PeerConnection", true);
// 	}
}

std::unique_ptr<cricket::VideoCapturer> Conductor::OpenVideoCaptureDevice() {
	std::vector<std::string> device_names;
	{
		std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
			webrtc::VideoCaptureFactory::CreateDeviceInfo());
		if (!info) {
			return nullptr;
		}
		int num_devices = info->NumberOfDevices();
		for (int i = 0; i < num_devices; ++i) {
			const uint32_t kSize = 256;
			char name[kSize] = { 0 };
			char id[kSize] = { 0 };
			if (info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
				device_names.push_back(name);
			}
		}
	}
	cricket::WebRtcVideoDeviceCapturerFactory factory;
	std::unique_ptr<cricket::VideoCapturer> capturer;
	for (const auto& name : device_names) {
		capturer = factory.Create(cricket::Device(name, 0));
		if (capturer) {
			break;
		}
	}
	return capturer;
}

void Conductor::AddStreams() {
	if (active_streams_.find(kStreamLabel) != active_streams_.end())
		return;  // Already added.

	rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
		peer_connection_factory_->CreateLocalMediaStream(kStreamLabel);

	rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
		peer_connection_factory_->CreateAudioTrack(
		kAudioLabel, peer_connection_factory_->CreateAudioSource(NULL)));

	std::unique_ptr<cricket::VideoCapturer> videoCapturer = Conductor::OpenVideoCaptureDevice();
	if (videoCapturer)
	{
		rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track(
			peer_connection_factory_->CreateVideoTrack(
				kVideoLabel,
				peer_connection_factory_->CreateVideoSource(std::move(videoCapturer),
					NULL)));

		main_wnd_->StartLocalRenderer(video_track);
		
		stream->AddTrack(video_track);
	}

	if (audio_track)
	{
		stream->AddTrack(audio_track);
	}

	if (!peer_connection_->AddStream(stream)) {
		RTC_LOG(LS_ERROR) << "Adding stream to PeerConnection failed";
	}
	typedef std::pair<std::string,
		rtc::scoped_refptr<webrtc::MediaStreamInterface> >
		MediaStreamPair;
	active_streams_.insert(MediaStreamPair(stream->label(), stream));
	main_wnd_->SwitchToStreamingUI();
}

void Conductor::DisconnectFromCurrentPeer() {
	RTC_LOG(INFO) << __FUNCTION__;
	if (peer_connection_.get()) {
		client_->SendHangUp(peer_id_);
		DeletePeerConnection();
	}

	if (main_wnd_->IsWindow())
		main_wnd_->SwitchToPeerList(client_->peers());
}

void Conductor::UIThreadCallback(int msg_id, void* data) {
	switch (msg_id) {
	case PEER_CONNECTION_CLOSED:
		RTC_LOG(INFO) << "PEER_CONNECTION_CLOSED";
		DeletePeerConnection();

		RTC_DCHECK(active_streams_.empty());

		if (main_wnd_->IsWindow()) {
			if (client_->is_connected()) {
				main_wnd_->SwitchToPeerList(client_->peers());
			}
			else {
				main_wnd_->SwitchToConnectUI();
			}
		}
		else {
			DisconnectFromServer();
		}
		break;

	case SEND_MESSAGE_TO_PEER: {
		RTC_LOG(INFO) << "SEND_MESSAGE_TO_PEER";
		std::string* msg = reinterpret_cast<std::string*>(data);
		if (msg) {
			// For convenience, we always run the message through the queue.
			// This way we can be sure that messages are sent to the server
			// in the same order they were signaled without much hassle.
			pending_messages_.push_back(msg);
		}

		if (!pending_messages_.empty() && !client_->IsSendingMessage()) {
			msg = pending_messages_.front();
			pending_messages_.pop_front();

			if (!client_->SendToPeer(peer_id_, *msg) && peer_id_ != -1) {
				RTC_LOG(LS_ERROR) << "SendToPeer failed";
				DisconnectFromServer();
			}
			delete msg;
		}

		if (!peer_connection_.get())
			peer_id_ = -1;

		break;
	}

	case NEW_STREAM_ADDED: {
		webrtc::MediaStreamInterface* stream =
			reinterpret_cast<webrtc::MediaStreamInterface*>(
			data);
		webrtc::VideoTrackVector tracks = stream->GetVideoTracks();
		// Only render the first track.
		if (!tracks.empty()) {
			webrtc::VideoTrackInterface* track = tracks[0];
			main_wnd_->StartRemoteRenderer(track);
		}
		stream->Release();
		break;
	}

	case STREAM_REMOVED: {
		// Remote peer stopped sending a stream.
		webrtc::MediaStreamInterface* stream =
			reinterpret_cast<webrtc::MediaStreamInterface*>(
			data);
		stream->Release();
		break;
	}

	default:
		RTC_DCHECK(false);
		break;
	}
}

void Conductor::Close()
{
	client_->SignOut();
	DeletePeerConnection();
}

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
	peer_connection_->SetLocalDescription(
		DummySetSessionDescriptionObserver::Create(), desc);

	std::string sdp;
	desc->ToString(&sdp);

// 	Json::StyledWriter writer;
// 	Json::Value jmessage;
// 	jmessage[kSessionDescriptionTypeName] = desc->type();
// 	jmessage[kSessionDescriptionSdpName] = sdp;
// 	SendMessage(writer.write(jmessage));
}

void Conductor::OnFailure(const std::string& error) {
	RTC_LOG(LERROR) << error;
}

void Conductor::SendMessage(const std::string& json_object) {
	std::string* msg = new std::string(json_object);
	RTC_LOG(WARNING) << "SendMessage: " << msg->c_str();

	main_wnd_->QueueUIThreadCallback(SEND_MESSAGE_TO_PEER, msg);
}
