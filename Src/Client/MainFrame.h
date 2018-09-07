#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP

#pragma once

#include "api/mediastreaminterface.h"
#include "api/video/video_frame.h"
#include "rtc_base/win32.h"
#include "media/base/mediachannel.h"
#include "media/base/videocommon.h"


#include "peer_connection_client.h"
#include "TrayIcon.h"


class MainWndCallback {
public:
	virtual void StartLogin(const AString& server, const AString& username, const AString& password) = 0;
	virtual void DisconnectFromServer() = 0;
	virtual void ConnectToPeer(int peer_id) = 0;
	virtual void DisconnectFromCurrentPeer() = 0;
	virtual void UIThreadCallback(int msg_id, void* data) = 0;
	virtual void Close() = 0;
protected:
	virtual ~MainWndCallback() {}
};

// Pure virtual interface for the main window.
class MainWindow {
public:
	virtual ~MainWindow() {}

	enum UI {
		CONNECT_TO_SERVER,
		LIST_PEERS,
		STREAMING,
	};
	virtual HWND handle() = 0;
	virtual void RegisterObserver(MainWndCallback* callback) = 0;

	virtual bool IsWindow() = 0;
	virtual void MessageBox(const char* caption, const char* text,
		bool is_error) = 0;

	virtual UI current_ui() = 0;

	virtual void SwitchToConnectUI() = 0;
	virtual void SwitchToPeerList(const Peers& peers) = 0;
	virtual void SwitchToStreamingUI() = 0;

	virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video) = 0;
	virtual void StopLocalRenderer() = 0;
	virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video) = 0;
	virtual void StopRemoteRenderer() = 0;

	virtual void QueueUIThreadCallback(int msg_id, void* data) = 0;
	virtual void PostNotification(const CString& message) = 0;
};

class MainFrame : public WindowImplBase, public MainWindow, public IListCallbackUI
{
public:
	enum WindowMessages {
		UI_THREAD_CALLBACK = WM_APP + 1000,
	};
public:
	MainFrame();
	~MainFrame();

public:
	virtual HWND handle();
	virtual void RegisterObserver(MainWndCallback* callback);
	virtual bool IsWindow();
	virtual void SwitchToConnectUI();
	virtual void SwitchToPeerList(const Peers& peers);
	virtual void SwitchToStreamingUI();
	virtual void MessageBox(const char* caption, const char* text,bool is_error);
	virtual UI current_ui();
	virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
	virtual void StopLocalRenderer();
	virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
	virtual void StopRemoteRenderer();
	virtual void QueueUIThreadCallback(int msg_id, void* data);
	virtual void PostNotification(const CString& message);
public:
	LPCTSTR GetWindowClassName() const;
	virtual void OnFinalMessage(HWND hWnd);
	virtual void InitWindow();
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
//	virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LPCTSTR GetResourceID() const;

	/*
	* 关键的回调函数，IListCallbackUI 中的一个虚函数，渲染时候会调用,在[1]中设置了回调对象
	*/
	virtual LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem);

public:
	//virtual void OnDisconnected();
	virtual void PacketBufferFull();
	virtual void PacketError(UInt32);
	virtual void PacketUnknown(UInt32);
protected:
	void Notify(TNotifyUI& msg);
	void OnExit(TNotifyUI& msg);
	void OnTimer(TNotifyUI& msg);

private:
	LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnTrayMessage(WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdateImg(WPARAM wParam, LPARAM lParam);

private://action
	void doLogin();
	void doReset();
	void doUiChanged();
	void onBtnVideoClick();

	class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame>
	{
	public:
		VideoRenderer(HWND wnd, int width, int height,
			webrtc::VideoTrackInterface* track_to_render);
		virtual ~VideoRenderer();

		void Lock() 
		{
			::EnterCriticalSection(&buffer_lock_);
		}

		void Unlock() 
		{
			::LeaveCriticalSection(&buffer_lock_);
		}

		// VideoRendererInterface implementation
		virtual void SetSize(int width, int height);
		void OnFrame(const webrtc::VideoFrame& frame) override;

		const BITMAPINFO& bmi() const { return bmi_; }
		const uint8_t* image() const { return image_.get(); }

	protected:
		enum 
		{
			SET_SIZE,
			RENDER_FRAME,
		};

		HWND wnd_;
		BITMAPINFO bmi_;
		std::unique_ptr<uint8_t[]> image_;
		CRITICAL_SECTION buffer_lock_;
		rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
	};

	// A little helper class to make sure we always to proper locking and
	// unlocking when working with VideoRenderer buffers.
	template <typename T>
	class AutoLock 
	{
	public:
		explicit AutoLock(T* obj) : obj_(obj) { obj_->Lock(); }
		~AutoLock() { obj_->Unlock(); }
	protected:
		T* obj_;
	};


private:
	CTrayIcon	m_TrayIcon;

private://UI
	CButtonUI	*pBtnLogin;
	CButtonUI	*pBtnVideo;
	CEditUI		*pEditUser;
	CEditUI		*pEditPass;
	CEditUI		*pEditServer;
	CListUI		*pListUsers;
	CControlUI	*pCtrlVideo;
private:
	AString m_strUserName;
	AString m_strPassWord;

	UI ui_;
	DWORD ui_thread_id_;
	Peers userInfo_;
	MainWndCallback *callback_;
	std::unique_ptr<VideoRenderer> local_renderer_;
	std::unique_ptr<VideoRenderer> remote_renderer_;
};

#endif // MAINFRAME_HPP

