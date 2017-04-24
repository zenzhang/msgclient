
// MsgClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MsgClient.h"
#include "MsgClientDlg.h"
#include "DlgName.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMsgClientDlg dialog



CMsgClientDlg::CMsgClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMsgClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMsgClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_LIST, m_friend_list);
}

BEGIN_MESSAGE_MAP(CMsgClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_CBN_SELCHANGE(IDC_COMBO_LIST, &CMsgClientDlg::OnCbnSelchangeComboList)
    ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CMsgClientDlg::OnBnClickedButtonLogin)
    ON_MESSAGE(WM_ZZZ_NOTIFY, &CMsgClientDlg::OnClientNotify)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CMsgClientDlg message handlers

BOOL CMsgClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	// TODO: Add extra initialization here
    streamsdk::the_streamsdk()->set_callback_handle(this);
    streamsdk::the_streamsdk()->start_server();
    
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMsgClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMsgClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMsgClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMsgClientDlg::OnCbnSelchangeComboList()
{
    // TODO: Add your control notification handler code here

    m_friend_list.UpdateData(FALSE);

    CString strBook;
    int nSel = m_friend_list.GetCurSel();
    m_friend_list.GetLBText(nSel, strBook);
    
    std::string name = (LPSTR)(LPCSTR)strBook;
    std::map<std::string, CDlgMessage*>::iterator iter
        = user_dlg_.find(name);
    if (iter == user_dlg_.end())
    {
        CDlgMessage* dlg = new CDlgMessage();
        dlg->SetName((LPSTR)(LPCSTR)m_name, name.c_str());
        dlg->Create(IDD_DIALOG1, this);
        user_dlg_[name] = dlg;
        dlg->ShowWindow(SW_SHOW);
    }
    else
    {
        iter->second->ShowWindow(SW_SHOW);
    }
    
    //user_dlg_
}


void CMsgClientDlg::OnBnClickedButtonLogin()
{
    // TODO: Add your control notification handler code here
    CDlgName dlgName;

    if (IDOK != dlgName.DoModal())
    {
        return;
    }
    CString name = dlgName.GetName();
    std::string message;
    bool ret = streamsdk::the_streamsdk()->login((LPSTR)(LPCSTR)name, message);
    if (!ret)
    {
        name = "登录失败, error reason:";
        name += message.c_str();
        MessageBox(name);
        return;
    }

    m_name = name;
    SetWindowText(m_name);
    ((CButton*)GetDlgItem(IDC_BUTTON_LOGIN))->EnableWindow(FALSE);
    //UpdateUIFriendList();

}

void CMsgClientDlg::UpdateUIFriendList()
{
    std::vector<std::string> list;
    streamsdk::the_streamsdk()->get_friend_list(list);
    std::vector<std::string>::iterator iter_find;

    std::map<std::string, CDlgMessage*>::iterator iter = user_dlg_.begin();
    for (; iter != user_dlg_.end();)
    {
        iter_find = std::find(list.begin(), list.end(), iter->first);
        if (iter_find == list.end())
        {
            CDlgMessage* dd = iter->second;
            dd->ShowWindow(FALSE);
            delete dd;
            user_dlg_.erase(iter);
            iter = user_dlg_.begin();
            continue;
        }
        ++iter;
    }

    m_friend_list.ResetContent();

    
    /*
    user_dlg_
    */


    for (size_t i = 0; i < list.size(); i++)
    {
        m_friend_list.AddString(list[i].c_str());
    }
    UpdateData(FALSE);
}

struct MessageContent
{
    MessageContent()
    {}
    MessageContent(const char* n, const char* msg)
        : name(n)
        , message(msg)
    {

    }
    std::string name;
    std::string message;
};


LRESULT CMsgClientDlg::OnClientNotify(WPARAM wParam, LPARAM lParam)
{
    if (wParam == 1)
    {
        UpdateUIFriendList();
    }
    else if (wParam == 4)
    {
        MessageBox("与服务器断开链接, 请退出应用，重新登录");
    }
    else /*if (wParam == 2)*/
    {//recv message
        SetTimer(1, 1000, NULL);

        MessageContent* msg = (MessageContent*)lParam;
        CDlgMessage* dlg = NULL;
        std::map<std::string, CDlgMessage*>::iterator iter = user_dlg_.find(msg->name);
        if (iter != user_dlg_.end())
        {
            dlg = iter->second;
        }
        else
        {
            dlg = new CDlgMessage();
            dlg->SetName((LPSTR)(LPCSTR)m_name, msg->name.c_str());
            dlg->Create(IDD_DIALOG1, this);
            user_dlg_[msg->name] = dlg;
            dlg->ShowWindow(SW_SHOW);
        }

        if (wParam == 2)
            dlg->OnRecvMessage(msg->message.c_str());
        else 
            dlg->OnErrorMessage(msg->message.c_str());
        delete msg;
        msg = NULL;
    }
    return 0;
}

void CMsgClientDlg::OnMessageCallback(const char* name, const char* msg)
{
    MessageContent* msgContent = new MessageContent(name, msg);
    PostMessage(WM_ZZZ_NOTIFY, 2, (LPARAM)msgContent);
}

void CMsgClientDlg::OnErrorMessageCallback(const char* name, const char* msg)
{
    MessageContent* msgContent = new MessageContent(name, msg);
    PostMessage(WM_ZZZ_NOTIFY, 3, (LPARAM)msgContent);
}

void CMsgClientDlg::UpdateFriendList()
{
    PostMessage(WM_ZZZ_NOTIFY, 1, 0);
}



void CMsgClientDlg::OnError(int code)
{
    PostMessage(WM_ZZZ_NOTIFY, 4, code);
}


void CMsgClientDlg::OnCancel()
{
    // TODO: Add your specialized code here and/or call the base class
    streamsdk::the_streamsdk()->logout();
    __super::OnCancel();
}


void CMsgClientDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: Add your message handler code here and/or call default

    if (nIDEvent == 1)
    {
        if (::GetActiveWindow() == AfxGetMainWnd()->m_hWnd)
        {
            KillTimer(1); //当程序窗口置于最前时,停止闪烁  
        }
        else
        {
            FlashWindow(TRUE);
        }
    }
    __super::OnTimer(nIDEvent);
}
