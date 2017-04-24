#pragma once


// CDlgName dialog

class CDlgName : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgName)

public:
	CDlgName(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgName();

// Dialog Data
	enum { IDD = IDD_DLG_NAME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedNo();

public:
    CString GetName();
private:
    CString m_name;
};
