// ClientSvrSocket.cpp : implementation file
//

#include "stdafx.h"
#include "CSocketDemo.h"
#include "ClientSvrSocket.h"
#include "SendFileDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClientSvrSocket

CClientSvrSocket::CClientSvrSocket()
{
	tag=0;
	closed=0;
	fileTran=NULL;
}

CClientSvrSocket::~CClientSvrSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CClientSvrSocket, CAsyncSocket)
	//{{AFX_MSG_MAP(CClientSvrSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CClientSvrSocket member functions
void CClientSvrSocket::OnReceive(int nErrorCode)
{
	CString ip;UINT port;
	CString buf,text;
	CString _fileName;
	byte* mybuf=new byte[PACKAGESIZE];
	int total=Receive(mybuf,PACKAGESIZE);
	buf.Format("%s",mybuf);
	GetPeerName(ip,port);

	if (buf.Find("<?Request FILETRANSFER?>")>-1)
	{
		closed=0;
		int pos=buf.Find(":");
		int posEnd=buf.Find(":",pos+1);
		_fileName=buf.Mid(pos+1,posEnd-pos-1);
		fileLen=buf.Mid(posEnd+1,buf.GetLength()-posEnd-1);
		fileLength=atol(fileLen.GetBuffer(0));
		fileName=_fileName;
		CString request;
		request.Format("����%s���û����㷢���ļ�%s���Ƿ���գ�",ip,_fileName);
		int ret=MessageBox(pcwnd->GetSafeHwnd(),request,"�ļ���������",MB_ICONQUESTION|MB_YESNO);
		if (ret==IDYES)
		{
			CString suffix=fileName.Right(fileName.GetLength()-fileName.ReverseFind('.')-1);
			CFileDialog dlg(FALSE,suffix,fileName);
			dlg.m_ofn.lpstrTitle="��ѡ�񱣴�λ��";
			dlg.m_ofn.lpstrFilter="�����ļ�����(*.*)\0*.*\0\0";
			if (IDOK==dlg.DoModal())
			{
				filePath=dlg.GetPathName();
				fileName=dlg.GetFileName();
				destFile.Open(filePath,CFile::modeWrite|CFile::modeNoTruncate|CFile::modeCreate|CFile::typeBinary,NULL);
				if (!fileTran)
				{
					CSendFileDlg *sfDlg=new CSendFileDlg(SOCKETTYPECLIENT,this);
					sfDlg->Create(IDD_SENDFILE_DIALOG,CWnd::FromHandle(::GetDesktopWindow()));
					fileTran=sfDlg;
					fileTran->ShowWindow(SW_SHOW);
 				}
				fileTran->SetDlgItemText(IDC_STATIC_SRCA,thePeerIp);
				fileTran->SetDlgItemText(IDC_STATIC_DESA,localIp);
				fileTran->SetDlgItemText(IDC_STATIC_FINFO,fileName+"\n��С: "+fileLen+" �ֽ�");
				CProgressCtrl *proSF=(CProgressCtrl*)fileTran->GetDlgItem(IDC_PROGRESS_TRANFILE);
				proSF->SetRange32(0,fileLength);
				proSF->SetPos(0);
				Send("<?Response FILETRANSFER?>:ACCEPT",256);
			}
			else
				Send("<?Response FILETRANSFER?>:REFUSE",256);
		}
		else
		{
			Send("<?Response FILETRANSFER?>:REFUSE",256);
		}
	}
	else if (buf.Find("<?FileStop?>")>-1)
	{
		if (closed==0)
		{
			destFile.Flush();
			destFile.Close();
			closed=1;
		}
	}
	else
	{
		destFile.SeekToEnd();
		destFile.Write(mybuf,total);
		//destFile.Flush();
		CProgressCtrl *proSF=(CProgressCtrl*)fileTran->GetDlgItem(IDC_PROGRESS_TRANFILE);
		proSF->SetPos(proSF->GetPos()+total);
		CString status;
		status.Format("�ѽ���%u �ֽ��е�%d �ֽ�",fileLength,proSF->GetPos());
		fileTran->SetDlgItemText(IDC_STATIC_STATUS,status);
		if (fileLength==proSF->GetPos() &&closed==0)
		{
			destFile.Flush();
			destFile.Close();
			closed=1;
		}
		Send("<?FileContinue?>",256);
	}

	delete[] mybuf;
	buf.Empty();
	CAsyncSocket::OnReceive(nErrorCode);
}

void CClientSvrSocket::OnAccept(int nErrorCode)
{
	CAsyncSocket::OnAccept(nErrorCode);
}

void CClientSvrSocket::OnClose(int nErrorCode)
{
	Close();

	CAsyncSocket::OnClose(nErrorCode);
}