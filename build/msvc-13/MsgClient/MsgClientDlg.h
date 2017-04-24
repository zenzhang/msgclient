
// MsgClientDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "DlgMessage.h"

#include <client/StreamSDK.h>


#define WM_ZZZ_NOTIFY (WM_USER+1)

// CMsgClientDlg dialog
class CMsgClientDlg : public CDialogEx
    , public streamsdk::IServerCallback
{
// Construction
public:
	CMsgClientDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MSGCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_friend_list;
    afx_msg void OnCbnSelchangeComboList();
    afx_msg void OnBnClickedButtonLogin();
    afx_msg LRESULT OnClientNotify(WPARAM wParam, LPARAM lParam);
public:
    void UpdateFriendList();
    void OnError(int code);
    void OnMessageCallback(const char* name, const char* msg);
    void OnErrorMessageCallback(const char* name, const char* msg);
private:
    void UpdateUIFriendList();
private:
    CString m_name;
    std::map<std::string, CDlgMessage*> user_dlg_;
public:
//    afx_msg void OnIdcancel();
    virtual void OnCancel();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};
