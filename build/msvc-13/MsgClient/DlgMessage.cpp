// DlgMessage.cpp : implementation file
//

#include "stdafx.h"
#include "MsgClient.h"
#include "DlgMessage.h"
#include "afxdialogex.h"

#include <client/StreamSDK.h>

#include <mmsystem.h>
#pragma comment(lib, "WINMM.LIB")

// CDlgMessage dialog

IMPLEMENT_DYNAMIC(CDlgMessage, CDialogEx)

CDlgMessage::CDlgMessage(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgMessage::IDD, pParent)
    , m_recv_msg(_T(""))
{

}

CDlgMessage::~CDlgMessage()
{
}

void CDlgMessage::SetName(const char* own, const char* remote)
{
    m_own_name = own;
    m_remote_name = remote;
}

void CDlgMessage::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_RECV, m_recv_msg);
}


BEGIN_MESSAGE_MAP(CDlgMessage, CDialogEx)
    ON_BN_CLICKED(ID_BTN_SEND, &CDlgMessage::OnBnClickedBtnSend)
    ON_BN_CLICKED(IDC_BTN_CLEAN, &CDlgMessage::OnBnClickedBtnClean)
    ON_BN_CLICKED(ID_BTN_FILE, &CDlgMessage::OnBnClickedBtnFile)
END_MESSAGE_MAP()


// CDlgMessage message handlers


BOOL CDlgMessage::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    CString title("和 ");
    title += m_remote_name;
    title += "  聊天";
    SetWindowText(title);
    //ModifyStyleEx(WS_EX_TOOLWINDOW, WS_EX_APPWINDOW);
    ModifyStyleEx(0, WS_EX_APPWINDOW, 0);
    // TODO:  Add extra initialization here

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgMessage::OnBnClickedBtnSend()
{
    // TODO: Add your control notification handler code here
    CString sendmsg;
    GetDlgItemText(IDC_EDIT_SEND, sendmsg);
    if (!sendmsg.IsEmpty())
    {
        streamsdk::the_streamsdk()->send_message((LPSTR)(LPCSTR)m_remote_name
            , (LPSTR)(LPCSTR)sendmsg);

        {
            CString msgShow;
            msgShow = m_own_name;
            msgShow += " 说:";
            msgShow += "\r\n";
            msgShow += sendmsg;
            ShowMessage(msgShow);
        }
    }
    SetDlgItemText(IDC_EDIT_SEND, "");
}

std::string GBKToUTF8(const std::string& strGBK)
{
    std::string strOutUTF8 = "";
    WCHAR * str1;
    int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
    str1 = new WCHAR[n];
    MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
    char * str2 = new char[n];
    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
    strOutUTF8 = str2;
    delete[]str1;
    str1 = NULL;
    delete[]str2;
    str2 = NULL;
    return strOutUTF8;
}

// 2、将UTF8转换成GBK

std::string UTF8ToGBK(const std::string& strUTF8)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
    unsigned short * wszGBK = new unsigned short[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)strUTF8.c_str(), -1, (LPWSTR)wszGBK, len);

    len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
    char *szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, szGBK, len, NULL, NULL);
    //strUTF8 = szGBK;
    std::string strTemp(szGBK);
    delete[]szGBK;
    delete[]wszGBK;
    return strTemp;
}


void CDlgMessage::OnBnClickedBtnClean()
{
    // TODO: Add your control notification handler code here
    UpdateData(TRUE);
    m_recv_msg.Empty();
    UpdateData(FALSE);
}

void CDlgMessage::ShowMessage(const CString& msg)
{
    UpdateData(TRUE);
    m_recv_msg += msg;
    m_recv_msg += "\r\n";
    UpdateData(FALSE);
    CEdit* edit = (CEdit*)GetDlgItem(IDC_EDIT_RECV);
    edit->LineScroll(edit->GetLineCount());
}

void CDlgMessage::OnErrorMessage(const char* msg)
{
    CString msgShow;
    msgShow = m_remote_name;
    msgShow += " 说:";
    msgShow += "\r\n";
    msgShow += msg;
    msgShow += "  [发送超时, 请重新发送]";
    ShowMessage(msgShow);
}

void CDlgMessage::OnRecvMessage(const char* msg)
{
    CString msgShow;
    msgShow = m_remote_name;
    msgShow += " 说:";
    msgShow += "\r\n";
    msgShow += msg;
    ShowMessage(msgShow);

    PlaySound("c:\\WINDOWS\\MEDIA\\打钟.wav", NULL, SND_FILENAME | SND_ASYNC);
    /*
    Windows 登录声.wav
    */
}

BOOL CDlgMessage::PreTranslateMessage(MSG* pMsg)
{
    // TODO: Add your specialized code here and/or call the base class

    if (WM_KEYDOWN == pMsg->message && VK_RETURN == pMsg->wParam)
    {
        OnBnClickedBtnSend();
        //send message
        return TRUE;
    }

    return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgMessage::OnBnClickedBtnFile()
{

    MessageBox("暂未开放");
    return;
    CString filter;
    filter = "所有文件(*.*)||";
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);
    if (dlg.DoModal() == IDOK)
    {
        CString str;
        str = dlg.GetPathName();
        streamsdk::the_streamsdk()->send_file((LPSTR)(LPCSTR)m_remote_name, (LPSTR)(LPCSTR)str);
    }

    // TODO: Add your control notification handler code here
}
