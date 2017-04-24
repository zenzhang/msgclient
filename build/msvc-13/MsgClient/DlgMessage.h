#pragma once

#include <string>

std::string GBKToUTF8(const std::string& strGBK);
std::string UTF8ToGBK(const std::string& strUTF8);

// CDlgMessage dialog

class CDlgMessage : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMessage)

public:
	CDlgMessage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgMessage();

    void SetName(const char* own, const char* remote);

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
    CString m_remote_name;
    CString m_own_name;
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBtnSend();
    afx_msg void OnBnClickedBtnClean();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    void OnErrorMessage(const char* msg);
    void OnRecvMessage(const char* msg);
private:
    CString m_recv_msg;
private:
    void ShowMessage(const CString& msg);
public:
    afx_msg void OnBnClickedBtnFile();
};
