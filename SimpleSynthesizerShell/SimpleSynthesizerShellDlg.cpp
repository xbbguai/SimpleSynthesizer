
// SimpleSynthesizerShellDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "SimpleSynthesizerShell.h"
#include "SimpleSynthesizerShellDlg.h"
#include "afxdialogex.h"
#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>
#include "thread.h"
#include "..\SimpleSynthesizer\WaveformTone.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//-----------------------------------------------------
//Codes for playing back a midi file
//

//Playback buffer
constexpr int BUFFER_SIZE = (int)SAMPLE_RATE;
constexpr int BUFFER_COUNT = 2;
char buffers[BUFFER_COUNT][BUFFER_SIZE];

bool bShouldPrepareData = false;
LPWAVEHDR pWaveHeader;

void CALLBACK WaveOutProc(HWAVEOUT hwo,
	UINT uMsg,
	DWORD dwInstance,
	DWORD dwParam1,
	DWORD dwParam2)
{
	if (uMsg == WOM_DONE)
	{
		pWaveHeader = (LPWAVEHDR)dwParam1;
		bShouldPrepareData = true;
	}
	return;
}

class PlaybackThread : public CThread
{
protected:

	HWAVEOUT        hWaveOut{};
	WAVEHDR         waveHeader[BUFFER_COUNT]{};	//For each buffer.
	WAVEFORMATEX    waveFormatEx{};
	HANDLE hWait{};

	bool playing{ false };
public:
	UINT Worker(LPVOID pParam);
	void OnExitingThread();

	bool IsPlaying()
	{
		return playing;
	}
};
PlaybackThread pbt;

void PlaybackThread::OnExitingThread()
{
	playing = false;
}

UINT PlaybackThread::Worker(LPVOID pParam)
{
	MidiPlayback* mpbPointer = (MidiPlayback*)pParam;
	bShouldPrepareData = false;
	mpbPointer->Rewind();
	playing = true;

	waveFormatEx.wFormatTag = WAVE_FORMAT_PCM;	//PCM 
	waveFormatEx.nChannels = 2;
	waveFormatEx.nSamplesPerSec = static_cast<DWORD>(SAMPLE_RATE);	//
	waveFormatEx.nBlockAlign = waveFormatEx.nChannels * 2;	//in bytes
	waveFormatEx.nAvgBytesPerSec = static_cast<DWORD>(SAMPLE_RATE) * waveFormatEx.nBlockAlign; //
	waveFormatEx.wBitsPerSample = 16;
	waveFormatEx.cbSize = 0;

	waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormatEx, (DWORD)WaveOutProc, 0L, CALLBACK_FUNCTION);//Get a handle for wave playback.

	int currentBuffer = 0;

	waveHeader[0].lpData = buffers[0];
	waveHeader[0].dwBufferLength = BUFFER_SIZE;
	waveHeader[0].dwFlags = 0L;
	waveHeader[0].dwLoops = 0L;
	waveOutPrepareHeader(hWaveOut, &waveHeader[0], sizeof(WAVEHDR));

	waveHeader[1].lpData = buffers[1];
	waveHeader[1].dwBufferLength = BUFFER_SIZE;
	waveHeader[1].dwFlags = 0L;
	waveHeader[1].dwLoops = 0L;
	waveOutPrepareHeader(hWaveOut, &waveHeader[1], sizeof(WAVEHDR));

	bool continuePlaying = true;
	mpbPointer->PrepareBuffer(buffers[0], BUFFER_SIZE);
	continuePlaying = mpbPointer->PrepareBuffer(buffers[1], BUFFER_SIZE);
	waveOutWrite(hWaveOut, &waveHeader[0], sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &waveHeader[1], sizeof(WAVEHDR));

	while (continuePlaying && !SenseAsynExit())
	{
		if (bShouldPrepareData)
		{
			bShouldPrepareData = false;
			continuePlaying = mpbPointer->PrepareBuffer(pWaveHeader->lpData, BUFFER_SIZE);
			waveOutUnprepareHeader(hWaveOut, pWaveHeader, sizeof(WAVEHDR));
			waveOutPrepareHeader(hWaveOut, pWaveHeader, sizeof(WAVEHDR));
			waveOutWrite(hWaveOut, pWaveHeader, sizeof(WAVEHDR));
		}
		else
			Sleep(5);
	}

	if (!SenseAsynExit())
		Sleep(500);

	waveOutClose(hWaveOut);
	return 0;
}

//-------------------------------------------------------------------------

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSimpleSynthesizerShellDlg 对话框



CSimpleSynthesizerShellDlg::CSimpleSynthesizerShellDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SIMPLESYNTHESIZERSHELL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSimpleSynthesizerShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSimpleSynthesizerShellDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTNLOAD, &CSimpleSynthesizerShellDlg::OnBnClickedBtnload)
	ON_BN_CLICKED(IDC_BTNPLAY, &CSimpleSynthesizerShellDlg::OnBnClickedBtnplay)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTNABOUT, &CSimpleSynthesizerShellDlg::OnBnClickedBtnabout)
	ON_BN_CLICKED(IDC_BTNEXPORT, &CSimpleSynthesizerShellDlg::OnBnClickedBtnexport)
END_MESSAGE_MAP()


// CSimpleSynthesizerShellDlg 消息处理程序

BOOL CSimpleSynthesizerShellDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
	{
		CProgressCtrl* pBar = (CProgressCtrl*)GetDlgItem(IDC_CH1 + i);
		pBar->SetRange(0, 100);
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSimpleSynthesizerShellDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSimpleSynthesizerShellDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSimpleSynthesizerShellDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


std::string ws2s(const std::wstring& ws)
{
	size_t i;
	std::string curLocale = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "chs");
	const wchar_t* _source = ws.c_str();
	size_t _dsize = 2 * ws.size() + 1;
	char* _dest = new char[_dsize];
	memset(_dest, 0x0, _dsize);
	wcstombs_s(&i, _dest, _dsize, _source, _dsize);
	std::string result = _dest;
	delete[] _dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

void CSimpleSynthesizerShellDlg::OnBnClickedBtnload()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog fdlg(TRUE, L"MIDI File(*.mid)|*.mid", NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"MIDI File(*.mid)|*.mid", this);
	if (fdlg.DoModal() == IDOK)
	{
		//CString to std::string
		std::string fileName = ws2s((LPCWSTR)fdlg.GetPathName());

		try
		{
			BeginWaitCursor();
			mpb.LoadMidiFile(fileName);

			SetWindowText(L"SimpleSynthesizer Shell - " + fdlg.GetFileName());
			GetDlgItem(IDC_BTNPLAY)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTNEXPORT)->EnableWindow(TRUE);
			EndWaitCursor();
		}
		catch (...)
		{
			GetDlgItem(IDC_BTNPLAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTNEXPORT)->EnableWindow(FALSE);
			MessageBox(L"Not a MIDI file or file does not exist.", L"Error");
		}
	}
}

void CSimpleSynthesizerShellDlg::OnBnClickedBtnplay()
{
	// TODO: 在此添加控件通知处理程序代码
	if (pbt.IsPlaying())
	{
		pbt.AsynTerminateThread();
		CThread::WaitFor(&pbt, 1200);
		for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
		{
			CProgressCtrl* pBar = (CProgressCtrl*)GetDlgItem(IDC_CH1 + i);
			pBar->SetPos(0);
		}

		((CProgressCtrl*)GetDlgItem(IDC_L))->SetPos(0);
		((CProgressCtrl*)GetDlgItem(IDC_R))->SetPos(0);

		KillTimer(0);
		GetDlgItem(IDC_BTNPLAY)->SetWindowText(L"Play");
		GetDlgItem(IDC_BTNLOAD)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNEXPORT)->EnableWindow(TRUE);
		SetDlgItemText(IDC_CPUUSAGE, L"0.0%");

		for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
		{
			CProgressCtrl* pBar = (CProgressCtrl*)GetDlgItem(IDC_CH1 + i);
			pBar->SetPos(0);
		}
	}
	else
	{
		for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
		{
			SetDlgItemInt(IDC_00 + i * 10, 0);
			SetDlgItemInt(IDC_01 + i * 10, 0);
			SetDlgItemInt(IDC_02 + i * 10, 0);
			SetDlgItemInt(IDC_03 + i * 10, 0);
			SetDlgItemInt(IDC_04 + i * 10, 0);
			SetDlgItemInt(IDC_05 + i * 10, 0);
			SetDlgItemInt(IDC_06 + i * 10, 0);
			SetDlgItemInt(IDC_07 + i * 10, 0);
		}

		pbt.CreateThread((LPVOID)&mpb);
		pbt.Start();
		SetTimer(0, 100, NULL);
		GetDlgItem(IDC_BTNLOAD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTNEXPORT)->EnableWindow(FALSE);
	}
}


void CSimpleSynthesizerShellDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ASSERT(MAX_MIDI_CHANNELS <= 16);	//The dialog does not have more than 16 channels to display.

	CString strCPU;
	if (pbt.IsPlaying())
	{
		GetDlgItem(IDC_BTNPLAY)->SetWindowText(L"Stop");
		strCPU.Format(L"%4.1f%%", mpb.cpuPercentage * 100);

		//Level bars
		int channelPeaks[MAX_MIDI_CHANNELS]{};
		for (auto& track : mpb.tracksStatus)
		{
			for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
			{
				int peak = track.channels[i].GetPeak();
				channelPeaks[i] = max(channelPeaks[i], peak);
			}
		}
		for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
		{
			CProgressCtrl *pBar = (CProgressCtrl *)GetDlgItem(IDC_CH1 + i);
			pBar->SetPos(channelPeaks[i]);
		}

		int masterPeakL, masterPeakR;
		mpb.GetPeak(masterPeakL, masterPeakR);
		((CProgressCtrl*)GetDlgItem(IDC_L))->SetPos(masterPeakL);
		((CProgressCtrl*)GetDlgItem(IDC_R))->SetPos(masterPeakR);

		SetDlgItemInt(IDC_MASTERVOL, (int)(mpb.masterVolume * 127));

		//Other params
		for (auto& track : mpb.tracksStatus)
		{
			for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
			{
				auto& channel = track.channels[i];
				if (channel.IsInUse())
				{
					SetDlgItemInt(IDC_00 + i * 10, channel.volume);
					SetDlgItemInt(IDC_01 + i * 10, channel.pan);
					SetDlgItemInt(IDC_02 + i * 10, channel.percussionBank >= 0 ? channel.percussionBank + 512 :  channel.instrumentBank);
					SetDlgItemInt(IDC_03 + i * 10, channel.instrumentID);
					SetDlgItemInt(IDC_04 + i * 10, channel.expression);
					SetDlgItemInt(IDC_05 + i * 10, channel.reverbDepth);
					SetDlgItemInt(IDC_06 + i * 10, channel.chorusDepth);
					SetDlgItemInt(IDC_07 + i * 10, channel.echoDepth);
				}
			}
		}
	}
	else
	{
		GetDlgItem(IDC_BTNPLAY)->SetWindowText(L"Play");
		GetDlgItem(IDC_BTNLOAD)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTNEXPORT)->EnableWindow(TRUE);
		strCPU = L"0.0%";
		for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
		{
			CProgressCtrl* pBar = (CProgressCtrl*)GetDlgItem(IDC_CH1 + i);
			pBar->SetPos(0);
		}
	}

	SetDlgItemText(IDC_CPUUSAGE, strCPU);
	CDialogEx::OnTimer(nIDEvent);
}


void CSimpleSynthesizerShellDlg::OnDestroy()
{
	if (pbt.IsPlaying())
	{
		pbt.AsynTerminateThread();
		CThread::WaitFor(&pbt, 1000);
	}
	CDialogEx::OnDestroy();
}


void CSimpleSynthesizerShellDlg::OnBnClickedBtnabout()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(L"A GUI Shell for SimpleSythesizer V0.21\r\nCopyright (C) 2021 Feng Dai\r\nSimpleSynthesizer is an experimental waveform synthesizer core.\r\nReleased under GPL 3.0", L"About");
}


void CSimpleSynthesizerShellDlg::OnBnClickedBtnexport()
{
	// TODO: 
	RIFFHeader riffHeader;
	WaveFormat format;
	WaveDataHeader dataHeader;

	riffHeader.id = 0x46464952;
	riffHeader.type = 0x45564157;

	format.id = 0x20746d66;
	format.size = sizeof(WaveFormat) - sizeof(format.id) - sizeof(format.size);
	format.audioFormat = 1;
	format.bitsPerSample = 16;
	format.blockAlign = 4;
	format.byteRate = static_cast<uint32_t>(SAMPLE_RATE) * 2 * 2;
	format.numChannels = 2;
	format.sampleRate = SAMPLE_RATE;

	dataHeader.id = 0x61746164;
	dataHeader.size = 0;

	CFileDialog fdlg(FALSE, L"WAVE File(*.wav)|*.wav", NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"WAVE File(*.wav)|*.wav", this);
	if (fdlg.DoModal() == IDOK)
	{
		CFile file;
		if (file.Open(fdlg.GetPathName(), CFile::OpenFlags::modeCreate | CFile::OpenFlags::modeWrite))
		{
			char buffer[1024];
			mpb.Rewind();
			BeginWaitCursor();

			//Write headers
			file.Write(&riffHeader, sizeof(RIFFHeader));
			file.Write(&format, sizeof(WaveFormat));
			file.Write(&dataHeader, sizeof(WaveDataHeader));

			while (mpb.PrepareBuffer(buffer, 1024))
			{
				file.Write(buffer, 1024);
				dataHeader.size += 1024;
			}
			file.Write(buffer, 1024);
			dataHeader.size += 1024;

			file.Flush();

			riffHeader.size = sizeof(riffHeader.type) + sizeof(WaveFormat) + sizeof(WaveDataHeader) + dataHeader.size;

			file.Seek(0, CFile::begin);
			//Write headers, again.
			file.Write(&riffHeader, sizeof(RIFFHeader));
			file.Write(&format, sizeof(WaveFormat));
			file.Write(&dataHeader, sizeof(WaveDataHeader));

			file.Close();
			EndWaitCursor();
		}
		else
			MessageBox(L"Unable to create file " + file.GetFileName());
	}
}
