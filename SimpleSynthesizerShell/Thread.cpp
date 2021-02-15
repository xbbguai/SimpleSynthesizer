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

#include "pch.h"
#include "Thread.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////
//Global functions
/////////////////////////////////////////////////////////////////////
CThread *StartThread(CThread *pThread, 
					 LPVOID pParam /* = NULL */, 
					 int nPriority /* = THREAD_PRIORITY_NORMAL */,
					 UINT nStackSize /* = 0 */,
					 LPSECURITY_ATTRIBUTES lpSecurityAttrs /* = NULL */)
{
	if (pThread)
	{
		pThread->CreateThread(pParam, nPriority, nStackSize, lpSecurityAttrs);
		if (pThread->Start())
			return pThread;
		else
			return NULL;
	}
	else
		return NULL;
}


///////////////////////////////////////////////////////////////////
//Static members of CThread:
///////////////////////////////////////////////////////////////////

BOOL CThread::bHasOperatorNewExecuted = FALSE;
CMutex CThread::mtx;

UINT GeneralThreadFunction(LPVOID pParam)
{
	CThread::SThreadParam *psThreadParam = (CThread::SThreadParam *)pParam;
	UINT nRetCode = psThreadParam->m_pThreadObj->ThreadStartsHere(psThreadParam->m_pParam);
	
	if (psThreadParam->m_pThreadObj->m_bAutoDelete && psThreadParam->m_pThreadObj->m_bIsDynamicallyCreated)
	{
		delete psThreadParam->m_pThreadObj;
	}
	return nRetCode;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CThread::CThread()
{
	m_sThreadParam.m_pParam = NULL;
	m_sThreadParam.m_pThreadObj = this;
	m_bIsDynamicallyCreated = bHasOperatorNewExecuted;
	
	bHasOperatorNewExecuted = FALSE;
	if (m_bIsDynamicallyCreated)
		mtx.Unlock();

	m_bCanAsynExit = FALSE;
	m_pWinThread = NULL;
	m_bAutoDelete = FALSE;
}

CThread::~CThread()
{
	if (m_pWinThread)
	{
		AsynTerminateThread();
		WaitFor(this, 10000);

//		delete m_pWinThread;	//This may cause memory leak if thread is still running!!!
	}
}

void * CThread::operator new (size_t nSize)
{
	mtx.Lock(); //It will be unlocked in the constructor
	bHasOperatorNewExecuted = TRUE;
//	mtx.Unlock();
	
	void *p = malloc(nSize);
//	TRACE("CThread object %x newed.\n",p);
	return p;
}

#ifdef _DEBUG
//For compatibility with DEBUG_NEW
void * CThread::operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
{
	mtx.Lock();	//It will be unlocked in the constructor
	bHasOperatorNewExecuted = TRUE;
//	mtx.Unlock();

	void *p = malloc(nSize);
//	TRACE("CThread object %x newed.\n",p);
	return p;
}
#endif

void CThread::operator delete(void *pPointer)
{
	free(pPointer);
//	TRACE("CThread object %x deleted.\n",pPointer);
	return;
}

BOOL CThread::CreateThread(LPVOID pParam, 
				 		   int nPriority /* = THREAD_PRIORITY_NORMAL*/,
						   UINT nStackSize /* = 0 */,
						   LPSECURITY_ATTRIBUTES lpSecurityAttrs /* = NULL */)
{
	m_sThreadParam.m_pParam = pParam;
	m_pWinThread = AfxBeginThread(GeneralThreadFunction, 
							      (LPVOID)&m_sThreadParam,
							      nPriority,
							      nStackSize,
							      CREATE_SUSPENDED,
							      lpSecurityAttrs);
	m_pWinThread->m_bAutoDelete = TRUE;
	return (m_pWinThread != NULL);
}

void CThread::AsynTerminateThread()
{
	//Will cause assertion if thread is not created.
	//If this message is traced, you must call CreateThread first.
	ASSERT(m_pWinThread != NULL);

	m_bCanAsynExit = TRUE;
}

BOOL CThread::DestroyThread()
{
	//Will cause assertion if thread is not created.
	//If this message is traced, you must call CreateThread first.
	ASSERT(m_pWinThread != NULL);

	//Dangerous
	//THIS MAY CAUSE MEMORY LEAK!!!
	delete m_pWinThread;
	m_pWinThread = NULL;
	return TRUE;
}

UINT CThread::WaitFor(CThread *pThread, DWORD dwTimeout /* = INFINITE */)
{
	try
	{
		if (pThread == NULL)
			throw 0;
		ASSERT(pThread->m_pWinThread != NULL);

		DWORD dwThreadCode;

		if (pThread == NULL)
			throw 0;
		if (pThread->m_pWinThread == NULL)
			throw 0;
		if (!pThread->GetExitCodeThread(dwThreadCode))
			throw 0;
		if (dwThreadCode == STILL_ACTIVE)
		{
			::WaitForSingleObject(pThread->m_pWinThread->m_hThread, dwTimeout);
			::GetExitCodeThread(pThread->m_pWinThread->m_hThread, &dwThreadCode);

			if (dwThreadCode == STILL_ACTIVE)
				TerminateThread(pThread->m_pWinThread->m_hThread, 0);
//				delete pThread->m_pWinThread;	//This may cause memory leak if thread is still running!!!

		}
		return dwThreadCode;
	}
	catch(...)	//If exception catched, it must be that pThread is an invalid pointer.
	{
		return 0;
	}
}

void CThread::OnExitingThread()
{
	//Do nothing here.
}

void CThread::EndThread(UINT nExitCode)
{
	m_nReturnCode = nExitCode;
	throw(this);
}

BOOL CThread::SenseAsynExit()
{
	//It is safe not to protect m_bCanAsynExit because there is only one thread
	//can change its value, and there is only one thread reading it.
	return m_bCanAsynExit;
}

UINT CThread::ThreadStartsHere(LPVOID pParam)
{
	UINT nReturnCode;
	try
	{
		nReturnCode = Worker(pParam);
	}
	catch (CThread *p)	//The pointer p is only a dummy.
	{
		nReturnCode = p->m_nReturnCode;
	}
	catch (...)	//If any disaster happened, just throw it.
	{
		throw;
	}
	OnExitingThread();
//	TRACE("Thread %x normally exited.\n",this);
	return nReturnCode;
}

////////////////////////////////////////////////////////
// Implementation of inline member functions
////////////////////////////////////////////////////////
AFX_INLINE BOOL CThread::Start()
{
	//Will cause assertion if thread is not created.
	//If this message is traced, you must call CreateThread first.
	ASSERT(m_pWinThread != NULL);
	
	m_bCanAsynExit = FALSE;
	return (m_pWinThread->ResumeThread() != 0xffffffff);
}

AFX_INLINE DWORD CThread::Suspend() const
{
	//Will cause assertion if thread is not created.
	//If this message is traced, you must call CreateThread first.
	ASSERT(m_pWinThread != NULL);

	return (m_pWinThread->SuspendThread() != 0xffffffff);
}

AFX_INLINE DWORD CThread::Resume() const
{
	//Will cause assertion if thread is not created.
	//If this message is traced, you must call CreateThread first.
	ASSERT(m_pWinThread != NULL);
	
	return (m_pWinThread->ResumeThread() != 0xffffffff);
}

AFX_INLINE BOOL CThread::SetPriority(int nPriority) const
{
	//Will cause assertion if thread is not created.
	//If this message is traced, you must call CreateThread first.
	ASSERT(m_pWinThread != NULL);

	return m_pWinThread->SetThreadPriority(nPriority);
}

AFX_INLINE int CThread::GetPriority() const
{
	//Will cause assertion if thread is not created.
	//If this message is traced, you must call CreateThread first.
	ASSERT(m_pWinThread != NULL);

	return m_pWinThread->GetThreadPriority();
}

AFX_INLINE BOOL CThread::GetExitCodeThread(DWORD &dwThreadExitCode) const
{
	//Will cause assertion if thread is not created.
	//If this message is traced, you must call CreateThread first.
	ASSERT(m_pWinThread != NULL);

	return ::GetExitCodeThread(m_pWinThread->m_hThread, &dwThreadExitCode);
}
