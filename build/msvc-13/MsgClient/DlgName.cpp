// DlgName.cpp : implementation file
//

#include "stdafx.h"
#include "MsgClient.h"
#include "DlgName.h"
#include "afxdialogex.h"


// CDlgName dialog

IMPLEMENT_DYNAMIC(CDlgName, CDialogEx)

CDlgName::CDlgName(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgName::IDD, pParent)
{

}

CDlgName::~CDlgName()
{
}

void CDlgName::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgName, CDialogEx)
    ON_BN_CLICKED(IDC_OK, &CDlgName::OnBnClickedOk)
    ON_BN_CLICKED(IDNO, &CDlgName::OnBnClickedNo)
END_MESSAGE_MAP()


// CDlgName message handlers
CString CDlgName::GetName()
{
    return m_name;
}

void CDlgName::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    GetDlgItemText(IDC_EDIT_NAME, m_name);
    if (m_name.IsEmpty())
    {
        MessageBox("«Î ‰»ÎÍ«≥∆");
        return;
    }
    CDialogEx::OnOK();
}


void CDlgName::OnBnClickedNo()
{
    // TODO: Add your control notification handler code here
    CDialogEx::OnCancel();
}
