/*
	Windows thread wrapper.

	Copyright (C) 2001-2021 Feng Dai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#if !defined(AFX_THREAD_H__B7E64237_FE70_11D4_9C10_94A0307D5A3E__INCLUDED_)
#define AFX_THREAD_H__B7E64237_FE70_11D4_9C10_94A0307D5A3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CThread  
{
public:
	struct SThreadParam
	{
		CThread *m_pThreadObj;
		LPVOID m_pParam;
	};
public:
	CWinThread *m_pWinThread;
protected:
	SThreadParam m_sThreadParam;

	BOOL m_bIsDynamicallyCreated;
	BOOL m_bAutoDelete;
	static BOOL bHasOperatorNewExecuted;
	static CMutex mtx;

	volatile BOOL m_bCanAsynExit;
public:
	CThread();
	virtual ~CThread();

	void SetAutoDelete()
	{
		m_bAutoDelete = TRUE;
	}

	BOOL CreateThread(LPVOID pParam, 
					  int nPriority = THREAD_PRIORITY_NORMAL,
					  UINT nStackSize = 0,
					  LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);
	inline BOOL SetPriority(int nPriority) const;
	inline int GetPriority() const;
	inline BOOL Start();
	inline DWORD Suspend() const;
	inline DWORD Resume() const;
	BOOL DestroyThread();	//Dangerous, should be enhanced
	inline BOOL GetExitCodeThread(DWORD &dwThreadExitCode) const;
	virtual void AsynTerminateThread();
	
	static UINT WaitFor(CThread *pThread, DWORD dwTimeout = INFINITE);

	void * operator new(size_t nSize);
	void operator delete(void * pPointer);
#ifdef _DEBUG
	void * operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
#endif
	
	//! ! ! ! --N O T E-- ! ! ! ! !
	//The following functions will be executed in the new thread.
	//Please be noticed that all other member functions associated with 
	//these functions must NOT call any one of the member functions or 
	//any one of the data members declared above.

	//This is the main worker function.
	//You should overload this function to implement thread actions.
	//You can add any member functions in the derived class of CThread
	//and call them inside Worker.
	virtual UINT Worker(LPVOID pParam) = 0;
	
	//This is the substitution for AfxEndThread.
	//You MUST call this function to terminate a thread
	//inside the thread instead of calling AfxEndThread.
	void EndThread(UINT nExitCode);

	//This function acts as a sensor if you use the asynchronous thread 
	//exiting mechanism.
	//It will return TRUE if father thread has demanded a thread ending.
	BOOL SenseAsynExit();
	
	//This function will be called when a thread is to be terminated.
	virtual void OnExitingThread();

	//!!!!!Please do not disturb the following functions and data members!!!!!
protected:
	UINT ThreadStartsHere(LPVOID pParam);
	UINT m_nReturnCode;

	friend UINT GeneralThreadFunction(LPVOID pParam);
};

#define NBDNORMALTHREAD(ThreadClass,pParam) StartThread(new ThreadClass,pParam)

CThread *StartThread(CThread *pThread, 
					 LPVOID pParam = NULL, 
					 int nPriority = THREAD_PRIORITY_NORMAL,
					 UINT nStackSize = 0,
					 LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);


#endif // !defined(AFX_THREAD_H__B7E64237_FE70_11D4_9C10_94A0307D5A3E__INCLUDED_)
