/******************************************************************************/
//                                                                        
// SPACD.H - Interface definitions between ACD PROXY and TSP++ library
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains the interface definitions between our ACD proxy
// application and the TSP++ class library
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                                        
/******************************************************************************/

#ifndef __SPACD_INC__
#define __SPACD_INC__

#define ACDPROXY_INPUT	_T("JTAProx_In")	// Base for INPUT parameters
#define ACDPROXY_OUTPUT	_T("JTAProx_Out")	// Base for OUTPUT parameters
#define MAX_ACD_MAX_SIZE (4*1024*1024)		// Max size allowed for blocks 4M

/*----------------------------------------------------------------------------*/
// ACD Request types passed to/from the service provider
/*----------------------------------------------------------------------------*/
#define ACDTYPE_PROXYREQUEST	1	// LINEPROXY request from ACD
#define ACDTYPE_PROXYRESPONSE	2	// LINEPROXY response from SP
#define ACDTYPE_NOTIFY			3	// AGENT state notification from SP -> TAPI.
#define ACDTYPE_OPEN            4   // Initialization block from TSP
#define ACDTYPE_CLOSE			5   // Shutdown request from TSP

/******************************************************************************/
// AGENTFEATURES
//
// These bits are used to describe the agent proxy features.
//
/******************************************************************************/
const DWORD AGENTFEATURE_SETAGENTGROUP				= 0x00000001;
const DWORD AGENTFEATURE_SETAGENTSTATE				= 0x00000002;
const DWORD AGENTFEATURE_SETAGENTACTIVITY			= 0x00000004;
const DWORD AGENTFEATURE_GETAGENTCAPS				= 0x00000008;
const DWORD AGENTFEATURE_GETAGENTSTATUS				= 0x00000010;
const DWORD AGENTFEATURE_AGENTSPECIFIC				= 0x00000020;
const DWORD AGENTFEATURE_GETAGENTACTIVITYLIST		= 0x00000040;
const DWORD AGENTFEATURE_GETAGENTGROUPLIST			= 0x00000080;
const DWORD AGENTFEATURE_CREATEAGENT				= 0x00000100;
const DWORD AGENTFEATURE_SETAGENTMEASUREMENTPERIOD	= 0x00000200;
const DWORD AGENTFEATURE_GETAGENTINFO				= 0x00000400;
const DWORD AGENTFEATURE_CREATEAGENTSESSION			= 0x00000800;
const DWORD AGENTFEATURE_GETAGENTSESSIONLIST		= 0x00001000;
const DWORD AGENTFEATURE_SETAGENTSESSIONSTATE		= 0x00002000;
const DWORD AGENTFEATURE_GETAGENTSESSIONINFO		= 0x00004000;
const DWORD AGENTFEATURE_GETQUEUELIST				= 0x00008000;
const DWORD AGENTFEATURE_SETQUEUEMEASUREMENTPERIOD	= 0x00010000;
const DWORD AGENTFEATURE_GETQUEUEINFO				= 0x00020000;
const DWORD AGENTFEATURE_GETGROUPLIST				= 0x00040000;
const DWORD AGENTFEATURE_SETAGENTSTATEEX			= 0x00080000;

/******************************************************************************
** ACDREQUEST
**
** This is the I/O request mapped to and from the service provider
** as requests are queued and completed by the service provider.
**
*******************************************************************************/
typedef struct
{
	DWORD dwSize;				// Size of the block
	DWORD dwReserved;			// Touch count for memmgr.
	DWORD dwProviderID;			// Provider ID sending or receiving request
	DWORD dwType;				// ACDTYPE_xxx
	WCHAR chMachine[MAX_COMPUTERNAME_LENGTH+1];
	WCHAR chUserName[MAX_COMPUTERNAME_LENGTH+1];
	DWORD dwLineDeviceID;		// Line Device ID in question
	DWORD_PTR dwReqKey;			// Key set by PROXY application for RESPONSE.
	LONG lResult;				// Result code for ACDTYPE_PROXYRESONSE
	char vPart[1];				// Place holder
	//
	// VARIABLE DATA BLOCK FOLLOWS
	//
} ACDREQUEST;

/******************************************************************************
** ACDNOTIFY
**
** This is the I/O request mapped to the end of the above ACDREQUEST
** structure when an agent state changes asynchronously.
**
*******************************************************************************/
typedef struct 
{
	DWORD dwMsg;				// Message to send (LINE_xxx)
	DWORD dwParam1;				// Parameter 1
	DWORD dwParam2;				// Parameter 2
	DWORD dwParam3;				// Parameter 3
} ACDNOTIFY;

/******************************************************************************
** ACDOPEN
**
** This is the I/O request mapped to the end of the above ACDREQUEST
** structure when the TSP first comes online and tells the ACD proxy which
** lines to open and manage agent requests for.
**
*******************************************************************************/
typedef struct
{
	DWORD dwAgentFeatures;			// Supported agent operations (LINEFEATUREAGENT_xxx)
} ACDOPEN;

/******************************************************************************
** SMINFO
**
** This is the header to the shared memory block.
**
*******************************************************************************/
typedef struct
{
	DWORD dwTotalSize;				// Total size currently in use.
	DWORD dwClientCount;			// Total client count using this block
} SMINFO;

/**************************************************************************************
** CSharedMemMgr
**
** Object which manages the shared memory blocks.
**
**************************************************************************************/
class CSharedMemMgr
{
// Class data
protected:
	bool m_fInit;					// true if this object is in-use.
	// Input block
	HANDLE m_hevtInput;				// Event signalling information available
	HANDLE m_hsemInput;				// Semaphore locking access to block
	HANDLE m_hInput;				// Input shared memory block
	LPVOID m_lpvInput;				// Map of input block
	// Output block
	HANDLE m_hevtOutput;			// Event signalling information available
	HANDLE m_hsemOutput;			// Semaphore locking access to block
	HANDLE m_hOutput;				// Output shared memory block
	LPVOID m_lpvOutput;				// Map of output block

// Constructor/Destructor
public:
	CSharedMemMgr();
	~CSharedMemMgr();

// Initialization method
public:
	bool Init(LPCTSTR pszInputFile, DWORD dwMaxInSize, LPCTSTR pszOutputFile, DWORD dwMaxOutSize);

// Access methods
public:
	bool ReadAny(ACDREQUEST** ppvBuff, HANDLE hevtStop);
	bool ReadSpecific(DWORD dwProviderKey, ACDREQUEST** ppvBuff, HANDLE hevtStop);
	bool Write(ACDREQUEST* lpvBuff);
};

/*****************************************************************************
** Procedure:  CSharedMemMgr::CSharedMemMgr
**
** Arguments:  'pszInput'		- Input base key
**             'dwMaxInSize'	- Max size of INPUT memory block
**             'pszOutput'		- Output base key
**             'dwMaxOutSize'	- Max size of OUTPUT memory block
**
** Returns:    void
**
** Description:  Constructor for our TAPI object
**
*****************************************************************************/
inline CSharedMemMgr::CSharedMemMgr() :
	m_hsemInput(NULL), m_hsemOutput(NULL), m_hevtInput(NULL), m_hevtOutput(NULL),
	m_hInput(NULL), m_hOutput(NULL), m_lpvInput(NULL), m_lpvOutput(NULL),
	m_fInit(false)
{
}// CSharedMemMgr::CSharedMemMgr

/*****************************************************************************
** Procedure:  CSharedMemMgr::~CSharedMemMgr
**
** Arguments:  void
**
** Returns:    void
**
** Description:  Destructor for our shared memory object
**
*****************************************************************************/
inline CSharedMemMgr::~CSharedMemMgr()
{
	if (m_fInit == true)
	{
		// Unmark one of our client
		WaitForSingleObject (m_hsemInput, INFINITE);

		SMINFO* pSMInfo = reinterpret_cast<SMINFO*>(m_lpvInput);
		pSMInfo->dwClientCount--;

		// Release the semaphore
		ReleaseSemaphore(m_hsemInput, 1, NULL);

		UnmapViewOfFile(m_lpvInput);
		UnmapViewOfFile(m_lpvOutput);
		
		CloseHandle(m_hsemInput);
		CloseHandle(m_hsemOutput);
		CloseHandle(m_hevtInput);
		CloseHandle(m_hevtOutput);
		CloseHandle(m_hInput);
		CloseHandle(m_hOutput);
	}

}// CSharedMemMgr::~CSharedMemMgr

/*****************************************************************************
** Procedure:  CSharedMemMgr::Init
**
** Arguments:  'pszInput'		- Input base key
**             'dwMaxInSize'	- Max size of INPUT memory block
**             'pszOutput'		- Output base key
**             'dwMaxOutSize'	- Max size of OUTPUT memory block
**
** Returns:    true/false if object initialized correctly.
**
** Description:  Shared memory object INIT function
**
*****************************************************************************/
inline bool CSharedMemMgr::Init(LPCTSTR pszInputFile, DWORD dwMaxInSize, 
								LPCTSTR pszOutputFile, DWORD dwMaxOutSize)
{
	// If we are already initialized, fail.
	if (m_fInit == true)
		return true;

	// Create an ACL that EVERYONE can read - this is necessary since we may
	// be running in the context of TAPISRV which can be configured to run under
	// any known user id and by default runs as the sysadmin.
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;

	// Create a SID which may be accessed by any user.
    SID_IDENTIFIER_AUTHORITY authWorld = SECURITY_WORLD_SID_AUTHORITY;
    PSID psidWorld = NULL;
    if (!AllocateAndInitializeSid(&authWorld, 1, SECURITY_WORLD_RID, 0,
					0,0,0,0,0,0, &psidWorld))
		return false;

	DWORD cbAcl = GetLengthSid(psidWorld) + sizeof(ACL) + 
		(2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));

    PACL pAcl = (PACL) HeapAlloc(GetProcessHeap(), 0, cbAcl);
    if (pAcl != NULL)
	{
		InitializeAcl(pAcl, cbAcl, ACL_REVISION);
		AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidWorld);
		InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&sd, true, pAcl, false);
    }
	else
	{
		FreeSid(psidWorld);
		return false;
	}

	// Add the security descriptor to the sa structure
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = false;

	// Create our objects
	m_hsemInput = CreateSemaphore(&sa, 1, 1, (TString(pszInputFile)+_T("s")).c_str());
	m_hsemOutput = CreateSemaphore(&sa, 1, 1, (TString(pszOutputFile)+_T("s")).c_str());
	m_hevtInput = CreateEvent(&sa, true, false, (TString(pszInputFile)+_T("e")).c_str());
	m_hevtOutput = CreateEvent(&sa, true, false, (TString(pszOutputFile)+_T("e")).c_str());

	bool fCreatedInput = true, fCreatedOutput = true;
	m_hInput = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, dwMaxInSize, pszInputFile);
	if (m_hInput && GetLastError() == ERROR_ALREADY_EXISTS)
		fCreatedInput = false;
	m_hOutput = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, dwMaxOutSize, pszOutputFile);
	if (m_hOutput && GetLastError() == ERROR_ALREADY_EXISTS)
		fCreatedOutput = false;

	// Map the files.
	if (m_hInput)  m_lpvInput  = MapViewOfFile(m_hInput, FILE_MAP_WRITE, 0, 0, 0);
	if (m_hOutput) m_lpvOutput = MapViewOfFile(m_hOutput, FILE_MAP_WRITE, 0, 0, 0);

	// If any of them failed, exit.
	if (m_hsemInput == NULL || m_hsemOutput == NULL || m_hevtInput == NULL ||
		m_hevtOutput == NULL || m_hInput == NULL || m_hOutput == NULL ||
		m_lpvInput == NULL || m_lpvOutput == NULL)
	{
		if (m_lpvInput) UnmapViewOfFile(m_lpvInput);
		if (m_lpvOutput) UnmapViewOfFile(m_lpvOutput);
		if (m_hsemInput)  CloseHandle(m_hsemInput);
		if (m_hsemOutput) CloseHandle(m_hsemOutput);
		if (m_hevtInput)  CloseHandle(m_hevtInput);
		if (m_hevtOutput) CloseHandle(m_hevtOutput);
		if (m_hInput)     CloseHandle(m_hInput);
		if (m_hOutput)    CloseHandle(m_hOutput);
		if (pAcl != NULL) HeapFree(GetProcessHeap(), 0, pAcl);
		if (psidWorld != NULL) FreeSid(psidWorld);
		return false;
	}

	// Delete our ACL stuff
	HeapFree(GetProcessHeap(), 0, pAcl);
	FreeSid(psidWorld);

	// Wait for the INPUT block
	WaitForSingleObject (m_hsemInput, INFINITE);

	// See if we created the memory block.  If so, initialize it.
	SMINFO* pSMInfo = reinterpret_cast<SMINFO*>(m_lpvInput);
	if (fCreatedInput)
	{
		pSMInfo->dwTotalSize = sizeof(SMINFO);
		pSMInfo->dwClientCount = 1;
	}
	else
		pSMInfo->dwClientCount++;

	// Release the semaphore
	ReleaseSemaphore(m_hsemInput, 1, NULL);

	// Wait for the OUTPUT block
	WaitForSingleObject (m_hsemOutput, INFINITE);
	if (fCreatedOutput)
	{
		SMINFO* pSMInfo = reinterpret_cast<SMINFO*>(m_lpvOutput);
		pSMInfo->dwTotalSize = sizeof(SMINFO);
		pSMInfo->dwClientCount = 0;
	}

	// Release the semaphore
	ReleaseSemaphore(m_hsemOutput, 1, NULL);

	m_fInit = true;

	return true;

}// CSharedMemMgr::Init

/*****************************************************************************
** Procedure:  CSharedMemMgr::ReadSpecific
**
** Arguments:  'ppvBuff'	- Buffer to allocate
**             'hevtStop'	- Event to wait on to kill this request
**
** Returns:    Size of allocated block (Zero for none, -1L for exit)
**
** Description:  This is used to read from our INPUT stream.
**
*****************************************************************************/
inline bool CSharedMemMgr::ReadSpecific(DWORD dwProviderKey, ACDREQUEST** ppvBuff, HANDLE hevtStop)
{
	SMINFO* lpSMInfo = reinterpret_cast<SMINFO*>(m_lpvInput);
	ACDREQUEST* pBlock = reinterpret_cast<ACDREQUEST*> ((LPBYTE)m_lpvInput + sizeof(SMINFO));

	// Pre-set our inputs.
	*ppvBuff = NULL;

	// Create an array to wait on.
	HANDLE arrWait[2] = { hevtStop, m_hevtInput };
	if (WaitForMultipleObjects(2, arrWait, false, INFINITE) == WAIT_OBJECT_0)
		return false;

	// Get the semaphore for the block
	WaitForSingleObject (m_hsemInput, INFINITE);

	// If there are no blocks waiting, exit.
	if (lpSMInfo->dwTotalSize > sizeof(SMINFO))
	{
		// Look through the current list of entries and see if any are for us.
		for (DWORD dwPos = lpSMInfo->dwTotalSize; dwPos > 0; dwPos -= pBlock->dwSize)
		{
			// If the provider key for this block matches ours, or it is 
			// a global message intended for ALL providers, copy it out.
			if (pBlock->dwProviderID == dwProviderKey ||
				pBlock->dwProviderID == -1L)
			{
				// Copy the block to our own memory.
				BYTE* pMyBuff = new BYTE[pBlock->dwSize];
				MoveMemory(pMyBuff, pBlock, pBlock->dwSize);
				*ppvBuff = reinterpret_cast<ACDREQUEST*>(pMyBuff);

				// Mark the block as being read by us.
				pBlock->dwReserved++;

				// If the provider key matches ours, remove the block.
				if ((pBlock->dwProviderID == dwProviderKey) ||
					(pBlock->dwReserved >= lpSMInfo->dwClientCount))
				{
					DWORD dwSize = lpSMInfo->dwTotalSize - static_cast<DWORD>(reinterpret_cast<LPBYTE>(pBlock) - reinterpret_cast<LPBYTE>(m_lpvInput)) - pBlock->dwSize;
					lpSMInfo->dwTotalSize -= pBlock->dwSize;
					MoveMemory(pBlock, ((LPBYTE)pBlock + pBlock->dwSize), dwSize);
				}
				break;
			}
			pBlock = reinterpret_cast<ACDREQUEST*>((LPBYTE)pBlock + pBlock->dwSize);
		}
	}

	// If there is no more data, then re-block all threads.
	if (lpSMInfo->dwTotalSize == sizeof(SMINFO))
		ResetEvent(m_hevtInput);

	// Release the semaphore
	ReleaseSemaphore(m_hsemInput, 1, NULL);

	return true;

}// CSharedMemMgr::Read

/*****************************************************************************
** Procedure:  CSharedMemMgr::ReadAny
**
** Arguments:  'ppvBuff'	- Buffer to allocate
**             'hevtStop'	- Event to wait on to kill this request
**
** Returns:    Size of allocated block (Zero for none, -1L for exit)
**
** Description:  This is used to read from our INPUT stream.
**
*****************************************************************************/
inline bool CSharedMemMgr::ReadAny(ACDREQUEST** ppvBuff, HANDLE hevtStop)
{
	SMINFO* lpSMInfo = reinterpret_cast<SMINFO*>(m_lpvInput);
	ACDREQUEST* pBlock = reinterpret_cast<ACDREQUEST*>((LPBYTE)m_lpvInput + sizeof(SMINFO));

	// Pre-set our inputs.
	*ppvBuff = NULL;

	// Create an array to wait on.
	HANDLE arrWait[2] = { hevtStop, m_hevtInput };
	if (WaitForMultipleObjects(2, arrWait, false, INFINITE) == WAIT_OBJECT_0)
		return false;

	// Get the semaphore for the block
	WaitForSingleObject (m_hsemInput, INFINITE);

	// Grab the first one in the queue.
	if (lpSMInfo->dwTotalSize > sizeof(SMINFO))
	{
		// Copy the block over
		char* pMyBuff = new char[pBlock->dwSize];
		MoveMemory(pMyBuff, pBlock, pBlock->dwSize);
		*ppvBuff = reinterpret_cast<ACDREQUEST*>(pMyBuff);

		// Remove the block
		lpSMInfo->dwTotalSize -= pBlock->dwSize;
		MoveMemory(pBlock, ((LPBYTE)pBlock + pBlock->dwSize), lpSMInfo->dwTotalSize);
	}

	// If there is no more data, then re-block all threads.
	if (lpSMInfo->dwTotalSize == sizeof(SMINFO))
		ResetEvent(m_hevtInput);

	// Release the semaphore
	ReleaseSemaphore(m_hsemInput, 1, NULL);

	return true;

}// CSharedMemMgr::Read

/*****************************************************************************
** Procedure:  CSharedMemMgr::Write
**
** Arguments:  'lpvBuff'	- Buffer to write
**             'dwSize'		- Size to write
**
** Returns:    void
**
** Description:  This function writes a block into our shared memory block.
**
*****************************************************************************/
inline bool CSharedMemMgr::Write(ACDREQUEST* lpRequest)
{
	SMINFO* lpSMInfo = reinterpret_cast<SMINFO*>(m_lpvOutput);
	ACDREQUEST* pBlock = NULL;

	// Mark the block as "unread".
	lpRequest->dwReserved = 0;

	// Get the semaphore for the block
	WaitForSingleObject (m_hsemOutput, INFINITE);

	// Only add the block if we have clients.
	bool fWritten = false;
	if (lpRequest->dwSize > 0 && lpSMInfo->dwClientCount > 0)
	{
		// Find the END of the queue.
		pBlock = reinterpret_cast<ACDREQUEST*>((LPBYTE)m_lpvOutput + lpSMInfo->dwTotalSize);

		// Add the block to the END of the queue.
		MoveMemory(pBlock, lpRequest, lpRequest->dwSize);

		// Add the count to our total size.
		lpSMInfo->dwTotalSize += lpRequest->dwSize;

		// Flag the event so the other thread wakes up.
		SetEvent(m_hevtOutput);

		fWritten = true;
	}

	// Release the semaphore
	ReleaseSemaphore(m_hsemOutput, 1, NULL);
	return fWritten;

}// CSharedMemMgr::Write

#endif // __SPACD_INC__
