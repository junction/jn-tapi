#ifndef THREADS_H
#define THREADS_H

#define WAIT_OBJECT_1 (WAIT_OBJECT_0+1)
#define WAIT_OBJECT_2 (WAIT_OBJECT_0+2)

// Class used to check for single or multi-thread operation.
// Methods can be used to determine if being called from
// the same thread the CheckThread object was created,
// or for debug purposes, throw an ASSERT
class CheckThread
{
private:
	volatile DWORD m_dwThreadId;
public:

	CheckThread()
	{	m_dwThreadId = GetCurrentThreadId();	}

	// returns true if called in same thread this object was created
	bool IsSameThread()
	{	return m_dwThreadId == GetCurrentThreadId(); }

	// Reset the threadID to the current executing thread
	void Reset()
	{	m_dwThreadId = GetCurrentThreadId(); }

	// Checks to see if in same thread as create, else in debug mode
	// will ASSERT.  Will log error
	void CheckSameThread();
};

// Simple class wrapper around the Windows Critical Section
class CriticalSection
{
private:
	CRITICAL_SECTION m_cs;
public:
	CriticalSection();
	~CriticalSection();
	void Enter();
	void Leave();
};

// Simple class that will "Enter" a critical section
// on the constructor, and "Leave" on its destructor
// when leaving scope
class CriticalSectionScope
{
private:
	CriticalSection *m_cs;
public:
	CriticalSectionScope(CriticalSection *cs);
	~CriticalSectionScope();
};

// UniqueID class with thread-safety
// on retrieving unique IDs
class UniqueId_ts : UniqueId
{
private:
	CriticalSection m_cs;
	
public:
	long getNextId()
	{
		CriticalSectionScope css(&m_cs);
		return UniqueId::getNextId();
	}
};

class MyThread
{
private:
	HANDLE m_thread;
	DWORD m_threadId;
	volatile bool m_bStopThread;
	volatile DWORD m_exitCode;
	volatile bool m_bInThread;

	static DWORD WINAPI _ThreadMethod(LPVOID lpVoid);

	bool _Create();
	void _Terminate();

protected:
		// Method that is called in the therad
	virtual DWORD Run() = 0;

	// Can be polled by thread to see
	// if signaled to stop via SignalStop.
	bool IsSignalStop() { return m_bStopThread; }

public:
	MyThread();
	~MyThread();

	// Sets flag that the thread should poll
	// to determine if to exit
	void SignalStop() { m_bStopThread=true; }

	// Returns true if thread is running
	bool IsInThread() 
	{ return m_bInThread; }

	void ResumeThread();
	void SuspendThread();
	void Terminate();
	bool Join(DWORD dwMsecs);

	DWORD GetExitCode()
	{	return m_exitCode; }
};

#endif
