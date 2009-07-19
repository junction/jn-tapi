/******************************************************************************/
//                                                                        
// SPLIB.H - TAPI Service Provider C++ Library header                     
//
// Copyright (C) 1994-2005 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// The SPLIB classes provide a basis for developing MS-TAPI complient     
// Service Providers.  They provide basic handling for all of the TSPI    
// APIs and a C-based handler which routes all requests through a set of C++     
// classes.                                                                 
//              
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//
// --------------------------------------------------------------
// PROJECT REVISION HISTORY
// --------------------------------------------------------------
// 3.00		04/12/1998  Initial revision (beta) released 12/1998
// 3.00a	01/15/1999  Minor fixes and updates for Unicode
// 3.00b	02/01/1999  TSPWizard support and addl. features
// 3.01     03/10/1999  Updated with ECPP and MECPP guidelines
// 3.03     06/30/1999  Windows 2000 BETA 3 TAPI 3.0 support
// 3.04     07/15/1999  Thread safety fixes, trace changes
// 3.041    08/05/1999  Misc. SPR closures.
// 3.042    08/24/1999  Quick fix for new versioning bug!
// 3.043    10/01/1999  Various bug fixes and tested against NT4SP6
// 3.044    02/15/2000  Dinkumware SXL testing and misc. fixes.
// 3.045    06/15/2000  Minor fixes and cleanup.
// 3.046    04/15/2001  Updated call hub support; support TAPI 3.1
// 3.047    07/31/2001  Fixes and STL changes for VS.NET beta support
// 3.05		09/30/2002  Gold VS.NET beta
// 3.05		11/15/2002  Final VS.NET (2002)
// 3.051	10/20/2003  VS.NET 2003 support
// 3.052	06/13/2005  VS.NET 2005 Beta 2 support
// 3.053	12/08/2005  VS.NET 2005 Final
//                                                           
/******************************************************************************/

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _SPLIB_INC_
#define _SPLIB_INC_

// For identifying specific features at a particular level
#define SPLIB_CURRENT_VERSION 0x00030053

// Must be Win32 target
#if !defined(_WIN32)
#error ERROR: Only Win32 targets supported!
#endif

// Force TAPI 3 support to be added into the code.  This does not
// force the TSP to negotiate to TAPI3, it simply ensures our data
// structures will be laid out in a consistent fashion across the 
// library, UI and TSP projects.
#if defined(TAPI_CURRENT_VERSION) || defined(TAPI_H) || defined(TSPI_H)
#error "You must include splib.h before TAPI.H or TSPI.H"
#endif

// Include TAPI headers
#define TAPI_CURRENT_VERSION 0x00030000
#if !defined(TAPI_H)
#include <tapi.h>
#endif
#if !defined(TSPI_H)
#include <tspi.h>
#endif

/*-------------------------------------------------------------------------------*/
// DEBUG SUPPORT
/*-------------------------------------------------------------------------------*/
#ifdef DEBUG
#ifndef _DEBUG
#define _DEBUG
#endif
#endif

#ifdef _UNICODE
#ifdef _DEBUG
#pragma comment(lib, "splib32ud.lib")
#else
#pragma comment(lib, "splib32u.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "splib32d.lib")
#else
#pragma comment(lib, "splib32.lib")
#endif
#endif

#ifdef _DEBUG
#define _NOINLINES_
#endif

// Depending on the compiler settings, it may decide not to
// inline functions -- VC6 spits out a warning which causes
// some people to be concerned.  Turn off the warning if VC6
// decides NOT to inline functions.
#pragma warning(disable:4710)

// Define the tracing categories -- this is a bitmask which allows each
// individual tracing event to be turned on/off independantly.
enum { 
	TRC_NONE		= 0x00000000,	// No Tracing
	TRC_MIN			= 0x00000001,	// Minimum tracing, TRACE macro, DTRACE macro
	TRC_API			= 0x00000002,	// TAPI api traces (parameters only)
	TRC_STRUCT		= 0x00000004,	// All structures to/from TAPI
	TRC_DUMP		= 0x00000008,	// Offset/Size pointers within structures to/from TAPI
	TRC_STATS       = 0x00000010,	// Statistics (event notification)
	TRC_OBJECTS     = 0x00000020,	// Basic telephony object creation/destruction (addr/line/phone)
	TRC_THREADS		= 0x00000040,	// Thread creation/destruction
	TRC_REQUESTS	= 0x00000080,	// Request creation/destruction
	TRC_CALLS		= 0x00000100,	// Call creation/destruction
	TRC_CALLMAP		= 0x00000200,	// Call id map
	TRC_WARNINGS	= 0x00000400,	// Warnings/Errors
	TRC_WORKERTHRD  = 0x00000800,	// Worker thread execution
	TRC_LOCKS		= 0x00001000,	// Full lock/unlock notifications
	TRC_CRITSEC     = 0x00002000,	// Win32 Critical section create/destroy
	TRC_AGENTPROXY  = 0x00004000,	// Agent proxy support
	TRC_USERDEFINED = 0x0FF00000,	// Derived TSP traces
	TRC_FULL		= 0x0FFFFFFF,	// All of the above
};

// TRACE	- Shows up in debug *and* retail traces
// DTRACE	- Shows up ONLY in debug builds.
void _cdecl tsplib_TspTrace(int iTraceLevel, LPCTSTR pszTrace, ...);
void _cdecl tsplib_TspTrace2(LPCTSTR pszTrace, ...);
void _cdecl tsplib_TspTrace(int iTraceLevel, UINT uidRes, ...);
void _cdecl tsplib_TspTrace2(UINT uidRes, ...);
extern "C" _declspec(dllexport) void __stdcall SetTraceLevel(int iLevel);
extern "C" _declspec(dllexport) int __stdcall GetTraceLevel();

#undef _TSP_TRACE
#undef _TSP_TRACEX
#undef _TSP_ASSERT
#undef _TSP_ASSERTE
#undef _TSP_VERIFY
#undef _TSP_DTRACE
#undef _TSP_DTRACEX

#define _TSP_TRACEX ::tsplib_TspTrace
#define _TSP_TRACE ::tsplib_TspTrace2

#ifdef _DEBUG
#pragma warning(disable:4786)		// Template expansion > 255 characters
#pragma warning(disable: 4127)		// constant expression for _TSP_ASSERT
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC			// Forces debug memory allocations

// Define the DEBUG_NEW macro for non-MFC users which still have
// the DEBUG_NEW code in place (backward compatibility).
#ifndef DEBUG_NEW
#define DEBUG_NEW new
#endif

// Define the ASSERT macro for TAPI 2.x - The TAPI system
// runs as a subsystem (TAPISRV) and therefore has no UI access.
// Any attempt to pop up a normal UI element will result in a 
// GP-fault or deadlock in NT.
void tsplib_AssertFailedLine(LPCSTR lpszFileName, int nLine, LPCSTR pszExpression);
#define _TSP_ASSERT(f) { if (!(f)) ::tsplib_AssertFailedLine(__FILE__, __LINE__, NULL); }
#define _TSP_ASSERTE(f) { if (!(f)) ::tsplib_AssertFailedLine(__FILE__, __LINE__, #f); }
#define _TSP_DTRACE ::tsplib_TspTrace2
#define _TSP_DTRACEX ::tsplib_TspTrace
#define _TSP_VERIFY(f) _TSP_ASSERTE(f)

#else // #ifndef DEBUG
#define DEBUG_NEW new


#if _MSC_VER >= 1300
#define _TSP_ASSERT(f) __noop
#define _TSP_ASSERTE(f) __noop
//#define _TSP_DTRACE __noop
//#define _TSP_DTRACEX __noop
#define _TSP_DTRACE ::tsplib_TspTrace2
#define _TSP_DTRACEX ::tsplib_TspTrace
#define _TSP_VERIFY(f) ((void)(f))
#else
#define _TSP_ASSERT(f) ((void)0)
#define _TSP_ASSERTE(f) ((void)0)
//#define _TSP_DTRACE ((void)0)
//#define _TSP_DTRACEX ((void)0)
#define _TSP_DTRACE ::tsplib_TspTrace2
#define _TSP_DTRACEX ::tsplib_TspTrace
#define _TSP_VERIFY(f) ((void)(f))
#endif


#endif // _DEBUG

#undef TRACE
#undef VERIFY
#undef TRACE0
#undef TRACE1
#undef TRACE2
#undef TRACE3
#undef ATLASSERT
#define ATLASSERT _TSP_ASSERT

#include <stddef.h>
#include <atlconv.h>  // Use W2A/A2W macros

/*-------------------------------------------------------------------------------*/
// Include the new template-based iostream library.  Streams are
// used for persistant data storage within the library.
/*-------------------------------------------------------------------------------*/
#pragma warning(push,3)
#pragma warning(disable:4702)
#include <sstream>				// Stream support (for debug)
#include <iomanip>				// Stream manipulators

/*-------------------------------------------------------------------------------*/
// STL headers required for building library
/*-------------------------------------------------------------------------------*/
#include <algorithm>			// find, find_one_of
#include <string>				// string, wstring
#include <typeinfo>				// RTTI support
#include <map>					// map support
#include <vector>				// vector support
#include <list>					// list support
#include <bitset>				// bitset support
#pragma warning(default:4702)
#pragma warning(pop)

// Manage UNICODE for STL.
#if defined(_UNICODE) || defined(UNICODE)
typedef std::wstring TString;
typedef std::wstringstream TStringStream;
#else
typedef std::string TString;
typedef std::stringstream TStringStream;
#endif

/*-------------------------------------------------------------------------------*/
// CLASS PRE-DEFINITIONS
/*-------------------------------------------------------------------------------*/
class TStream;
class CSharedMemMgr;
class CServiceProvider;
	class CTSPIDevice;
		class CTSPIConnection;
		class CTSPILineConnection;
			class CTSPIAddressInfo;
				class CTSPICallHub;
				class CTSPICallAppearance;
				class CTSPIConferenceCall;
		class CTSPIPhoneConnection;

// Define known versions of TAPI
#define TAPIVER_13 (0x00010003)		// Add-on to Windows 3.1
#define TAPIVER_14 (0x00010004)		// Shipped with Windows 95
#define TAPIVER_20 (0x00020000)		// Shipped with Windows NT 4.0
#define TAPIVER_21 (0x00020001)		// Add-on to Windows 95, NT
#define TAPIVER_22 (0x00020002)		// Windows 98, NTSP6 released version
#define TAPIVER_30 (0x00030000)     // Windows NT 2000
#define TAPIVER_31 (0x00030001)     // Windows XP and Server 2003

/*-------------------------------------------------------------------------------*/
// OTHER TSP++ LIBRARY INCLUDE FILES
/*-------------------------------------------------------------------------------*/
#include <spbase.h>				// Building block objects
#include <spagent.h>			// Agent-related structures
#include <spreq.h>				// Request Management objects

/*-------------------------------------------------------------------------------*/
// CONSTANTS
/*-------------------------------------------------------------------------------*/
#define TOTAL_STD_BUTTONS		12		// Total # of standard buttons for TAPI (0-9, *, #).
#define TOTAL_DYNAMIC_OBJECTS   6       // Total count of dynamic objects
#define LIBRARY_INTERVAL		500		// Msecs for internal library thread (minimum digit/tone/media timeout)
#define DELETECALL_STALE		(5000)	// Time to wait before really deleting a call object.

// These keys are used during TSPI_providerGenericDialogData
// to convert line/phone identifiers into the associated provider
// identifier.  They are also used in SPLIBUI.H
#define GDD_LINEPHONETOPROVIDER		(0xab110301)
#define GDD_LINEPHONETOPROVIDEROK	(0xab110302)
#define GDD_LINEPHONETOPERMANENT    (0xab110303)
#define GDD_LINEPHONETOPERMANENTOK  (0xab110304)

/******************************************************************************/
// CTSPIDevice
//
// This class defines a specific device controlled through the TSPI
// interface.
//
// This class handles multiple lines and phones by placing each
// connection object into a line or phone array.
//
/******************************************************************************/
class CTSPIDevice : public CTSPIBaseObject
{
// Class data
protected:
    DWORD m_dwProviderId;			// Our permanent provider id assigned by TAPI.
    TString m_strDevInfo;			// Switch Information
	HPROVIDER m_hProvider;			// Our provider handle assigned by TAPI.	
    TLineArray m_arrayLines;		// List of CTSPIConnection line structures
    TPhoneArray m_arrayPhones;		// List of CTSPIConnection phone structures
	TCallMap m_mapCalls;			// Map of calls by CALLID on this device
	HANDLE m_hevtDeviceShutdown;	// Event signalling that this device is going away.
	HANDLE m_hevtDeviceDeleted;		// Event signalling that this device is gone.
	HANDLE m_htIntervalTimer;		// Our interval timer thread
	TConnectionMap m_mapPhoneConnections;		// Map of connection objects on this device.
	TConnectionMap m_mapLineConnections;		// Map of connection objects on this device.
    ASYNC_COMPLETION m_lpfnCompletionProc;		// Our asynchronous completion callback
	volatile DWORD m_dwIntervalTimeout;			// Timeout for the interval timer.
	// WARNING: Future releases will move this information off into a template class.
	// DO NOT RELY ON THE FOLLOWING DATA OR NAMES!
	HANDLE m_htAgentProxy;			// The agent proxy connection thread
	CSharedMemMgr* m_psmProxy;		// Proxy connection object
	TAgentGroupArray m_arrAgentGroups;			// Array of agent groups
	TAgentActivityArray m_arrAgentActivities;	// Array of agent activities
	TAgentExtensionArray m_arrAgentExtensions;	// Array of agent-handler extensions

// Constructors
public:
    CTSPIDevice();  // Should only be created through the CreateObject() method.
    virtual ~CTSPIDevice();

// Methods
public:
    // These retrieves the provider information
    DWORD GetProviderID() const;
    unsigned int GetLineCount() const;
    unsigned int GetPhoneCount() const;
	HPROVIDER GetProviderHandle() const;
    LPCTSTR GetSwitchInfo() const;
	void SetSwitchInfo(LPCTSTR pszSwitchInfo);

	// Agent activity management
	unsigned int GetAgentActivityCount() const;
	const TAgentActivity* GetAgentActivity(unsigned int iPos) const;
	bool DoesAgentActivityExist(DWORD dwActivity) const;
	TString GetAgentActivityById(DWORD dwID) const;
	int AddAgentActivity (DWORD dwID, LPCTSTR pszName);
	void RemoveAgentActivity(DWORD dwID);

	// Agent group management
	unsigned int GetAgentGroupCount() const;
	const TAgentGroup* GetAgentGroup(unsigned int iPos) const;
	bool DoesAgentGroupExist(DWORD dwGroupID1, DWORD dwGroupID2=0, DWORD dwGroupID3=0, DWORD dwGroupID4=0) const;
	TString GetAgentGroupById(DWORD dwGroupID1, DWORD dwGroupID2=0, DWORD dwGroupID3=0, DWORD dwGroupID4=0) const;
	int AddAgentGroup (LPCTSTR pszName, DWORD dwGroupID1, DWORD dwGroupID2=0, DWORD dwGroupID3=0, DWORD dwGroupID4=0);
	void RemoveAgentGroup(DWORD dwGroupID1, DWORD dwGroupID2=0, DWORD dwGroupID3=0, DWORD dwGroupID4=0);

	// Agent Specific mappings
	unsigned int GetAgentSpecificExtensionCount() const;
	bool DoesAgentSpecificExtensionExist(DWORD dwID1, DWORD dwID2=0, DWORD dwID3=0, DWORD dwID4=0) const;
	const TAgentSpecificEntry* GetAgentSpecificExtensionID(unsigned int iPos) const;
	int AddAgentSpecificExtension(DWORD dwID1, DWORD dwID2=0, DWORD dwID3=0, DWORD dwID4=0);
	void RemoveAgentSpecificExtension(DWORD dwID1, DWORD dwID2=0, DWORD dwID3=0, DWORD dwID4=0);

    // These support Plug&Play for the devices
    CTSPILineConnection* CreateLine(DWORD_PTR dwItemData=0);
    CTSPIPhoneConnection* CreatePhone(DWORD_PTR dwItemData=0);
	void RemoveLine(CTSPILineConnection* pLine);
	void RemovePhone(CTSPIPhoneConnection* pPhone);

	// Save device to registry stream; this can be used to dump the configuration
	// back if it is changed through the TSP rather than through a UI interface.
	bool Save();

	// Function to associate a line/phone together
	void AssociateLineWithPhone(unsigned int iLine, unsigned int iPhone);

	// This function allows you to explicitely complete a request by requestID
	// vs. using a request object.  Note that if a request object is associated with
	// the request it will NOT be updated! This is best used for lineDevSpecific completions.
	void OnAsynchRequestComplete(DRV_REQUESTID drvRequestID, LONG lResult);

    // This set of methods accesses the connection arrays to retrieve connection info structures.
    CTSPILineConnection* GetLineConnectionInfo(unsigned int nIndex) const;
    CTSPIPhoneConnection* GetPhoneConnectionInfo(unsigned int nIndex) const;
    CTSPILineConnection* FindLineConnectionByDeviceID(DWORD dwDeviceId) const;
    CTSPIPhoneConnection* FindPhoneConnectionByDeviceID(DWORD dwDeviceId) const;
	CTSPILineConnection* FindLineConnectionByPermanentID(DWORD dwConnID) const;
	CTSPIPhoneConnection* FindPhoneConnectionByPermanentID(DWORD dwConnID) const;

	// Returns a call hub from a callid
	const CTSPICallHub* FindCallHub(DWORD dwCallID) const;

	// Function to map a line/phone id to an object.
	void MapConnID(DWORD dwOldID, DWORD dwNewID, CTSPIPhoneConnection* pObject);
	void MapConnID(DWORD dwOldID, DWORD dwNewID, CTSPILineConnection* pObject);

	// Set the timer resolution for our interval timer.
	void SetIntervalTimer(DWORD dwTimeout=0);

	// This sets the current connected status of each line device
	void SetDeviceStatus(bool fConnect);

#ifdef _DEBUG
	virtual TString Dump() const;
#endif

// Overridable methods
public:
    // This distributes a received data buffer to all or a particular connection.
	// This is provided for derived usage - it is not invoked by the library
    virtual void ReceiveData (DWORD dwConnID, LPCVOID lpBuff=NULL);

	// This is called by the connection object to open the device.
	virtual bool OpenDevice (CTSPIConnection* pConn);

	// This is called by the connection object to close the device.
	virtual bool CloseDevice (CTSPIConnection* pConn);

	// This is called to process UI dialog data
	virtual LONG GenericDialogData (LPVOID lpParam, DWORD dwSize);

	// Serialization support
	virtual TStream& read(TStream& istm);
	virtual TStream& write(TStream& ostm) const;

// Internal methods
protected:
	friend unsigned __stdcall tsplib_IntervalTimerThread(void* pParam);
	friend unsigned __stdcall tsplib_AgentProxyThread(void* pParam);
	friend class CServiceProvider;
    friend class CTSPILineConnection;
    friend class CTSPIPhoneConnection;
    friend class CTSPIConnection;
	friend class CTSPICallHub;
	friend class CTSPICallAppearance;

	// This runs the interval timer loop.
	void RunTimer();

	// This function is called to allocate a stream for reading persistant object
	// information (v3.0b)
	virtual TStream* AllocStream();

    // This function is called directly after the constructor to actually initialize the
    // device object.  Once this completes, the device should be ready to be queried by TAPI.
    virtual bool Init(DWORD dwProviderId, DWORD dwBaseLine, DWORD dwBasePhone, DWORD dwLines, DWORD dwPhones, HPROVIDER hProvider, ASYNC_COMPLETION lpfnCompletion);
    
    // The asynchronous callback is used when a TAPI request has finished
    // and we used the asynchronous handle given to us by TAPI.
    virtual void OnAsynchRequestComplete(LONG lResult = 0L, CTSPIRequest* pReq = NULL);

	// This function is called peridically to process the interval timer.
    virtual void OnTimer();

	// This is called when a new request is inserted by the line or phone connection.
	// It passes the notification onto the CServiceProvider object by default.
	virtual bool OnNewRequest (CTSPIConnection* pConn, CTSPIRequest* pReq, int* piPos);

    // This method is called when a request is canceled and it has already
    // been started on the device.  It is called by the connection object when
	// the request is being deleted.  Default behavior is to pass through to the
	// CServiceProvider class.
    virtual void OnCancelRequest (CTSPIRequest* pReq);

    // These are called to add internal connection information to arrays.
    void AddLineConnectionInfo(CTSPILineConnection* pConn);
    void AddPhoneConnectionInfo(CTSPIPhoneConnection* pConn);
	CTSPICallHub* GetCallHubFromMap(DWORD dwCallID);
	void RemoveCallHub(DWORD dwCallID, CTSPICallHub* pHub);

	// Agent PROXY connection.  Most of this is called by the library "under the covers"
	// in response to other events (such as CompleteRequest) when it is determined that 
	// it is an action passed through the proxy.
	void EnableAgentProxy(CTSPILineConnection* pLine, DWORD dwAgentFeatures = 0);
	void AgentProxy();
	void ProxyResponse(DRV_REQUESTID dwRequestID, LONG lResult);
	void ProxyNotify(CTSPILineConnection* pLine, DWORD dwMsg, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);

	// Functions to reload our configuration from the registry (v3.0)
	void LoadObjects(TStream& rs);
	void SaveObjects(TStream& rs);

	TAgentActivity* ReadAgentActivity(TStream& istm);
	void WriteAgentActivity(const TAgentActivity* pAct, TStream& ostm) const;
	TAgentGroup* ReadAgentGroup(TStream& istm);
	void WriteAgentGroup(const TAgentGroup* pGroup, TStream& ostm) const;

// Locked members
private:
	CTSPIDevice(const CTSPIDevice& to);
	CTSPIDevice& operator=(const CTSPIDevice& cs);
};

/******************************************************************************/
// CMSPDriver class
//
// This class defines a connection to a single media service provider.
// It is defined and used only for TAPI 3.0 negotiation and compatibility.
//
// Note: this class is currently under development and will evolve (perhaps
// significantly) with future released based on the additions to 
// Windows 2000 before release. It is currently based on BETA 3
//
/******************************************************************************/
class CMSPDriver : public CTSPIBaseObject
{
// Class data
protected:
	DWORD_PTR m_htHandle;				// TAPI opaque handle for this
	CTSPIAddressInfo* m_pAddressOwner;	// Owner of this MSP connection

// Constructor
public:
	CMSPDriver(CTSPIAddressInfo* pOwner, DWORD_PTR htHandle);
	virtual ~CMSPDriver();

// Accessors
public:
	CTSPILineConnection* GetLineOwner() const;
	CTSPIAddressInfo* GetAddressOwner() const;
	void SendData(CTSPICallAppearance* pCall, LPVOID lpData, DWORD dwSize);
	DWORD_PTR GetTapiHandle() const;

// Locked members
private:
	CMSPDriver(const CMSPDriver& md);
	CMSPDriver& operator=(const CMSPDriver& md);
};

/******************************************************************************/
// TMSPArray
//
// Internal array object to hold MSP devices at the line level.
// 
/******************************************************************************/
typedef std::vector<CMSPDriver*> TMSPArray;

/******************************************************************************/
// CTSPIConnection class
//
// This class defines a connection to our TSP.  This base class is
// derived from for a phone or line device.
//
// This class supports multiple requests by keeping each request in 
// an object list.  As a request is fielded by the service provider,
// the next request for this line is pulled from the front of the
// list and activated.
//
/******************************************************************************/
class TSP_NOVTABLE CTSPIConnection : public CTSPIBaseObject
{
// Class data.
public:
	enum {
		_IsDeleted = 0x00000001,		// Line/Phone has been removed.
		_IsRemoved = 0x00000002			// Line/Phone has been removed
			// Additional may be added in the future.
	};

protected:
    CTSPIDevice* m_pDevice;			// Pointer to our device association
    TString m_strName;				// Line/Phone name "MyPhone"
    DWORD m_dwDeviceID;				// TAPI line/phone device identifier.
	DWORD m_dwFlags;				// Flags associated with this connection	
    DWORD m_dwNegotiatedVersion;	// Negotiated TAPI version for this connection.
	TExtVersionInfo* m_pExtVerInfo;	// Extended version information
    TRequestList m_lstAsynchRequests;	// Asynchronous requests list for this connection.
	mutable TDeviceClassArray m_arrDeviceClass; // Device class array for line/phoneGetID

// Constructors
protected:
    CTSPIConnection();
	virtual ~CTSPIConnection();

// Methods
public:
    // These are the QUERYxxx functions
    CTSPIDevice* GetDeviceInfo() const;
    LPCTSTR GetName() const;
    DWORD GetDeviceID() const;
    DWORD GetNegotiatedVersion() const;
    bool IsLineDevice() const;
    bool IsPhoneDevice() const;
	bool HasBeenDeleted() const;

	// These manipulate the flags of the object
	DWORD GetFlags() const;

	// This returns the extension version in effect
	void SetExtVersionInfo(DWORD dwMinVersion, DWORD dwMaxVersion, DWORD dwExtensionID0,DWORD dwExtensionID1=0, DWORD dwExtensionID2=0, DWORD dwExtensionID3=0);
	DWORD GetExtVersion() const;

    // These are the SETxxx functions
    void SetName(LPCTSTR lpszName);   

   	// Functions to manipulate the device class array
	int AddDeviceClass (LPCTSTR pszClass, DWORD dwData);
	int AddDeviceClass (LPCTSTR pszClass, HANDLE hHandle, LPCTSTR lpszBuff);
	int AddDeviceClass (LPCTSTR pszClass, HANDLE hHandle, LPVOID lpBuff, DWORD dwSize);
	int AddDeviceClass (LPCTSTR pszClass, DWORD dwFormat, LPVOID lpBuff, DWORD dwSize, HANDLE hHandle=INVALID_HANDLE_VALUE);
	int AddDeviceClass (LPCTSTR pszClass, LPCTSTR pszBuff, DWORD dwType=-1L);
	bool RemoveDeviceClass (LPCTSTR pszClass);
	DEVICECLASSINFO* GetDeviceClass(LPCTSTR pszClass) const;

    // These functions manage our asynchronous request list.
    int AddAsynchRequest(CTSPIRequest* pReq);
    void RemoveRequest(CTSPIRequest* pRequest);
    int GetRequestCount() const;
    CTSPIRequest* GetRequest(unsigned int iPos, BOOL fAddRef=FALSE) const;
    void RemovePendingRequests(CTSPICallAppearance* pCall = NULL, int nReqType = REQUEST_ALL, LONG lResult=TAPIERR_REQUESTCANCELLED, bool fOnlyIfReqNotStarted=false, const CTSPIRequest* pcurrReq=NULL);
    void CompleteRequest (CTSPIRequest* pReq, LONG lResult = 0, bool fTellTAPI = true, bool fRemove = true);
    bool CompleteCurrentRequest(LONG lResult = 0, bool fTellTAPI = true, bool fRemove = true);
    CTSPIRequest* FindRequest(CTSPICallAppearance* pCall, int iReqType) const;
    void WaitForAllRequests(CTSPICallAppearance* pCall=NULL, int nRequest=REQUEST_ALL);

// Overridable functions
public:
	// Required overrides by derived classes
    virtual DWORD GetPermanentDeviceID() const = 0;

	// Request list management.
	virtual LONG WaitForRequest(CTSPIRequest* pReq, DWORD dwMsecs=INFINITE);
    virtual CTSPIRequest* GetCurrentRequest(BOOL fAddRef=FALSE) const;

    // These functions will call back to the device object if not overridden here.
    virtual bool OpenDevice();
    virtual bool CloseDevice();

	// This function is called when data is received or when a request is started.
	// The default implementation forwards the request to the appropriate
	// ON_TSPI_REQUEST handler using the DispatchRequest method.
    virtual bool ReceiveData (LPCVOID lpBuff = NULL);

	// This function routes the request using the dispatch maps.
	virtual bool DispatchRequest(CTSPIRequest* pRequest, LPCVOID lpBuff);

	// This function is called when the device received data but did not have
	// an active request pending to process it.  It is called by the ReceiveData
	// function.
	virtual bool UnsolicitedEvent(LPCVOID lpBuff);

// TAPI methods which are common between line/phone devices
public:
	virtual LONG NegotiateVersion(DWORD dwLoVersion, DWORD dwHiVersion, LPDWORD lpdwExtVersion);
	virtual LONG NegotiateExtVersion(DWORD dwTSPIVersion, DWORD dwLoVersion, DWORD dwHiVersion, LPDWORD lpdwExtVersion);
	virtual LONG SelectExtVersion(DWORD dwExtVersion);
	virtual LONG GetExtensionID(DWORD dwTSPIVersion, LPEXTENSIONID lpExtensionID);
    virtual LONG GetIcon (const TString& strDevClass, LPHICON lphIcon);
    virtual LONG GetID (const TString& strDevClass, LPVARSTRING lpDeviceID, HANDLE hTargetProcess);

// Internal methods of the object.
protected:
    friend class CTSPIRequest;
    friend class CServiceProvider;
    friend class CTSPIDevice;
    virtual void Init(CTSPIDevice* pDeviceOwner, DWORD dwDeviceID);
	virtual bool AddAsynchRequest (CTSPIRequest* pReq, int iPos) /*throw()*/;
	virtual bool OnNewRequest (CTSPIRequest* pReq, int* piPos);
	virtual void OnCancelRequest (CTSPIRequest* pReq);
    virtual void OnRequestComplete (CTSPIRequest* pReq, LONG lResult) = 0;
    virtual bool IsMatchingRequest (CTSPIRequest* pReq, CTSPICallAppearance* pCall, int iRequest, bool fOnlyIfReqNotStarted);
    virtual void OnTimer();
    virtual void SetDeviceID(DWORD dwId);
	virtual const tsplib_REQMAP* GetRequestList() const;
	virtual TRequestMap* GetRequestMap() const = 0;

// Locked members
private:
	CTSPIConnection(const CTSPIConnection& to);
	CTSPIConnection& operator=(const CTSPIConnection& to);
};

/******************************************************************************/
// CTSPILineConnection class
//
// This class describes a line connection for TAPI.  It is based
// off the above CTSPIConnection class but contains data and
// methods specific to controlling a line device.
//
// This class in turn holds an array of calls, one of which may be
// active on the line.  Each call can have an address, a call state,
// and report activity to TAPI.
//
/******************************************************************************/
class CTSPILineConnection : public CTSPIConnection
{
// Class data
public:
	// This must match the UI side
	enum {
		Station				= 1,
		Queue				= 2,
		RoutePoint			= 3,
		PredictiveDialer	= 4,
		VRU					= 5,
		Trunk			    = 6,
		Other				= 7
		// Private line types can be defined after this point and dealt with
		// in derived line class objects
	};

protected:
	enum {
		IsAgentEnabled = 0x00000004,	// Line has agent features
		NotifyNumCalls = 0x00000008,	// Transient flag -- set on PreCallStateChange
		DelCallsOnClose = 0x00000010	// Deletes all call appearances (without dropping calls) on lineClose.
			// Additional may be added in the future.
	};

	int m_iLineType;				// Type of line device
    HTAPILINE m_htLine;             // TAPI opaque line handle
    LINEDEVCAPS m_LineCaps;         // Line device capabilities
    LINEDEVSTATUS m_LineStatus;     // Line device status
    LINEEVENT m_lpfnEventProc;      // TAPI event callback for line events
    DWORD m_dwLineMediaModes;       // Current media modes of interest to TAPI.
    DWORD m_dwLineStates;           // Which status messages need to be sent to TAPI.DLL?
	DWORD m_dwConnectedCallCount;	// Total count of CONNECTED cals on line	
	TUIntArray m_arrRingCounts;		// Ring counts for each of the ring modes.
	TAddressArray m_arrAddresses;	// Addresses on this line (fixed)
	TUIList m_lstUIDialogs;			// List of active UI dialogs
    TTerminalInfoArray m_arrTerminalInfo;  // Terminal information
    TCompletionList m_lstCompletions; // Current outstanding call completions
	static TRequestMap g_mapRequests; // Request map for all lines
	GUID m_guidMSP;					// GUID for the media service provider
	DWORD m_dwCallHubTracking;		// Tracking bits for call-hub tracking

// Constructor
public:
    CTSPILineConnection();          // Should only be created by the CreateObject method.
    virtual ~CTSPILineConnection();

// Methods
public:
    // The TAPI line handle defines the line we are connected to for TAPI.
    // It is passed as the first parameter for the callbacks.
    HTAPILINE GetLineHandle() const;

    // Function which must be called by the derived class to initialize all the
    // addresses available on this line.  Each address must be added in order to
    // have call appearances appear for this line.
    CTSPIAddressInfo* CreateAddress (LPCTSTR lpszDialableAddr=NULL, LPCTSTR lpszAddrName=NULL, 
                         bool fAllowIncoming=true, bool fAllowOutgoing=true, DWORD dwAvailMediaModes=LINEMEDIAMODE_UNKNOWN,
                         DWORD dwBearerMode=LINEBEARERMODE_VOICE, DWORD dwMinRate=0L, DWORD dwMaxRate=0L, LPLINEDIALPARAMS lpDialParams=NULL,
                         DWORD dwMaxNumActiveCalls=1, DWORD dwMaxNumOnHoldCalls=0, DWORD dwMaxNumOnHoldPendCalls=0, DWORD dwMaxNumConference=0,
                         DWORD dwMaxNumTransConf=0,DWORD dwAddressType=LINEADDRESSTYPE_PHONENUMBER);
	CTSPIAddressInfo* CreateMonitoredAddress(LPCTSTR lpszDialableAddr, LPCTSTR lpszAddrName, 
						DWORD dwAvailMediaModes=LINEMEDIAMODE_UNKNOWN, DWORD dwBearerMode=LINEBEARERMODE_VOICE,
						DWORD dwMinRate=0L, DWORD dwMaxRate=0L, DWORD dwMaxNumActiveCalls=1, DWORD dwMaxNumOnHoldCalls=0, DWORD dwMaxNumOnHoldPendCalls=0,DWORD dwAddressType=LINEADDRESSTYPE_PHONENUMBER);
    int GetAddressCount() const;
    CTSPIAddressInfo* GetAddress(unsigned int iAddressID) const;
    CTSPIAddressInfo* FindAddress(LPCTSTR lpszDialableAddr) const;

	// Line type
	int GetLineType() const;

	// If your switch hardware supports a query feature which allows you to read existing
	// call information, you probably don't need the built-in support for tracking calls on
	// un-opened lines. If so, call this function which will delete existing calls off the
	// line when it is closed, but not drop the calls themselves. You must then add the code
	// to re-read the switch state when the line is opened.
	void DeleteCallsOnClose();

	// Create a new UI instance dialog
	HTAPIDIALOGINSTANCE CreateUIDialog(DRV_REQUESTID dwRequestID, LPVOID lpItemData, LPVOID lpParams=NULL, DWORD dwSize=0L, LPCTSTR lpszUIDLLName=NULL);
	void SendDialogInstanceData(HTAPIDIALOGINSTANCE htDlgInstance, LPVOID lpParams=NULL, DWORD dwSize=0L);
	LPVOID GetUIDialogItem(HTAPIDIALOGINSTANCE htDlgInst) const;
	void OnUIDialogClosed(HTAPIDIALOGINSTANCE htDlgInst, LPVOID lpItemData);

	// Returns the associated phone (if any)
	CTSPIPhoneConnection* GetAssociatedPhone() const;

    // This function will run through all our addresses and see if any support
    // the specified media modes.  It will return success if any do.    
    DWORD GetDefaultMediaDetection() const;

	// This turns on the agent support for this line.
	void EnableAgentProxy();
	bool SupportsAgents() const;

    // Get a pointer to the line device capabilities
    LPLINEDEVCAPS GetLineDevCaps();
    LPLINEDEVSTATUS GetLineDevStatus();
    const LINEDEVCAPS* GetLineDevCaps() const;
    const LINEDEVSTATUS* GetLineDevStatus() const;

	// Call location methods
    CTSPICallAppearance* FindCallByState(DWORD dwCallState) const;
	CTSPICallAppearance* FindCallByType(int iType) const;
	CTSPICallAppearance* FindCallByCallID(DWORD dwCallID) const;

    // Call completion support
    RTCompleteCall* FindCallCompletionRequest (DWORD dwSwitchInfo, LPCTSTR pszSwitchInfo) const;
	RTCompleteCall* FindCallCompletionRequest(CTSPICallAppearance* pCall) const;
	RTCompleteCall* FindCallCompletionRequest(DWORD dwCompletionID) const;
    void RemoveCallCompletionRequest(DWORD dwCompletionID, bool fNotifyTAPI=false);

    // Terminal support functions.  A terminal is a notification device of a line.  The various
    // types are defined by the LINETERMMODE_xxx bits.  To support terminals, simply add each terminal
    // at any time (TAPI is notified).
    int AddTerminal (LPCTSTR lpszName, LINETERMCAPS& Caps, DWORD dwModes=0L);
    void RemoveTerminal (unsigned int iTerminalId);
    int GetTerminalCount() const;
    DWORD GetTerminalInformation (unsigned int iTerminalID) const;

	// This is used to send a notification to TAPI for this line device.
    void Send_TAPI_Event(CTSPICallAppearance* pCall, DWORD dwMsg, DWORD dwP1 = 0L, DWORD dwP2 = 0L, DWORD dwP3 = 0L);

    // Members which set status information for this line.
    void SetBatteryLevel (DWORD dwBattery);
    void SetSignalLevel (DWORD dwSignal);
    void SetRoamMode (DWORD dwRoamMode);
    void SetRingMode (DWORD dwRingMode);
    void SetTerminalModes (unsigned int iTerminalID, DWORD dwTerminalModes, bool fRouteToTerminal);
	void SetLineFeatures (DWORD dwFeatures);
    void SetMediaControl (TSPIMEDIACONTROL* lpMediaControl);

	// Device status flags
    void SetDeviceStatusFlags (DWORD dwStatus);
	void DevStatusInService(bool fInservice=true);
	void DevStatusConnected(bool fConnected=true);
	void DevLocked(bool fLocked=true);
	void DevMsgWaiting(bool fMessageWaiting=true);

	// Force the line to close
	void ForceClose();

    // Method which should be called when a ring is detected on this line.
    void OnRingDetected (DWORD dwRingMode, bool fFirstRing=false);

	// Allows setting the MSP guid on a line-by-line basis.
	void SetMSPGUID(GUID& guidMSP);

#ifdef _DEBUG
	virtual TString Dump() const;
#endif

// Overridable functions
public:
    // Unique id for this line/phone device
	void SetPermanentLineID(DWORD dwLineID);
    virtual DWORD GetPermanentDeviceID() const;

    // Method which is called to verify CallParameters supported
    virtual LONG CanSupportCall (const LPLINECALLPARAMS lpCallParams) const;

    // Method which validates a media control list for this line (called by lineSetMediaControl).
    virtual LONG ValidateMediaControlList(TSPIMEDIACONTROL* lpMediaControl) const;

	// Function which locates the address to use for a new call against a line object.
    virtual CTSPIAddressInfo* FindAvailableAddress (const LPLINECALLPARAMS lpCallParams, DWORD dwFeature=0) const;

	// Serialization support
	virtual TStream& read(TStream& istm);
	virtual TStream& write(TStream& ostm) const;

    // TAPI events
    virtual LONG Open(HTAPILINE htLine, LINEEVENT lpfnEventProc, DWORD dwTSPIVersion);
    virtual LONG Close();
    virtual LONG GetAddressID(LPDWORD lpdwAddressId, DWORD dwAddressMode, LPCTSTR lpszAddress, DWORD dwSize);
	virtual LONG Forward(DRV_REQUESTID dwRequestId, CTSPIAddressInfo* pAddr, TForwardInfoArray* parrForwardInfo, DWORD dwNumRingsNoAnswer, HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall, LPLINECALLPARAMS lpCallParams);
    virtual LONG MakeCall(DRV_REQUESTID dwRequestID, HTAPICALL htCall, LPHDRVCALL lphdCall, LPCTSTR lpszDestAddr, DWORD dwCountryCode, LPLINECALLPARAMS const lpCallParams);
    virtual LONG SetDefaultMediaDetection (DWORD dwMediaModes);
    virtual LONG SetStatusMessages(DWORD dwLineStates, DWORD dwAddressStates);
	virtual LONG SetTerminal(DRV_REQUESTID dwRequestID, CTSPIAddressInfo* pAddr, CTSPICallAppearance* pCall, DWORD dwTerminalModes, DWORD dwTerminalID, bool bEnable);
	virtual LONG SetupTransfer(DRV_REQUESTID dwRequestID, CTSPICallAppearance *pCall, HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall, const LPLINECALLPARAMS lpCallParams);
	virtual LONG SetupConference(DRV_REQUESTID dwRequestID, CTSPICallAppearance* pCall, HTAPICALL htConfCall, LPHDRVCALL lphdConfCall, HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall, DWORD dwNumParties, const LPLINECALLPARAMS lpLineCallParams);
    virtual LONG ConditionalMediaDetection(DWORD dwMediaModes, const LPLINECALLPARAMS lpCallParams);
    virtual LONG GatherCapabilities (DWORD dwTSPIVersion, DWORD dwExtVer, LPLINEDEVCAPS lpLineCaps);
    virtual LONG GatherStatus (LPLINEDEVSTATUS lpStatus);
    virtual LONG UncompleteCall (DRV_REQUESTID dwRequestID, DWORD dwCompletionID);
    virtual LONG GetDevConfig(const TString& strDeviceClass, LPVARSTRING lpDeviceConfig);
    virtual LONG SetDevConfig(const TString& strDeviceClass, const LPVOID lpDevConfig, DWORD dwSize);
    virtual LONG DevSpecificFeature(DWORD dwFeature, DRV_REQUESTID dwRequestId, LPVOID lpParams, DWORD dwSize);
	virtual LONG SetLineDevStatus (DRV_REQUESTID dwRequestID, DWORD dwStatusToChange, bool fStatus);
	virtual LONG GenericDialogData (LPVOID lpvItemData, LPVOID lpParam, DWORD dwSize);
	virtual LONG DevSpecific(CTSPIAddressInfo* pAddr, CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, LPVOID lpParam, DWORD dwSize);
	virtual LONG CreateAgent(DRV_REQUESTID dwRequestID, LPHAGENT lphAgent, LPCTSTR pszMachineName, LPCTSTR pszUserName, LPCTSTR pszAgentID, LPCTSTR pszAgentPIN);
	virtual LONG SetAgentMeasurementPeriod(DRV_REQUESTID dwRequestID, HAGENT hAgent, DWORD dwMeasurementPeriod);
	virtual LONG GetAgentInfo(HAGENT hAgent, LPLINEAGENTINFO lpAgentInfo);
	virtual LONG CreateAgentSession(DRV_REQUESTID dwRequestID, LPHAGENTSESSION lphSession, HAGENT hAgent, LPCTSTR pszAgentPIN, const GUID& guid, DWORD dwWorkingAddressID);
	virtual LONG GetAgentSessionList(HAGENT hAgent, LPLINEAGENTSESSIONLIST lpSessionList);
	virtual LONG SetAgentSessionState(DRV_REQUESTID dwRequestID, HAGENTSESSION hSession, DWORD dwAgentSessionState, DWORD dwNextAgentSessionState);
	virtual LONG GetAgentSessionInfo(HAGENTSESSION hSession, LPLINEAGENTSESSIONINFO lpSessionInfo);
	virtual LONG GetQueueList(const GUID& GroupID, LPLINEQUEUELIST lpQueueList);
	virtual LONG SetQueueMeasurementPeriod(DRV_REQUESTID dwRequestID, DWORD dwQueueID, DWORD dwMeasurementPeriod);
	virtual LONG GetQueueInfo(DWORD dwQueueID, LPLINEQUEUEINFO lpQueueInfo);
	virtual LONG GetGroupList(LPLINEAGENTGROUPLIST lpGroupList);
	virtual LONG SetAgentStateEx(DRV_REQUESTID dwRequestID, HAGENT hAgent, DWORD dwState, DWORD dwNextState);
	virtual LONG MSPIdentify(GUID* pGUID);
	virtual LONG ReceiveMSPData(CMSPDriver* pMSP, CTSPICallAppearance* pCall, LPVOID lpData, DWORD dwSize);
	virtual LONG GetCallHubTracking(LPLINECALLHUBTRACKINGINFO lpTrackingInfo);
	virtual LONG SetCallHubTracking(LPLINECALLHUBTRACKINGINFO lpTrackingInfo);

	// These can be used to notify TAPI about changes to LINEDEVCAPS or LINEDEVSTATUS.
    virtual void OnLineCapabiltiesChanged();
    virtual void OnLineStatusChange (DWORD dwState, DWORD dwP2=0L, DWORD dwP3=0L);
	virtual void OnMediaConfigChanged();
    virtual void OnMediaControl (CTSPICallAppearance* pCall, DWORD dwMediaControl);

	// Force a recalc of the line features based on current address/call counts.
	virtual void RecalcLineFeatures(bool fRecalcAllAddresses=false);

// Internal methods
protected:
    friend class CTSPIAddressInfo;
    friend class CTSPICallAppearance;
    friend class CServiceProvider;
	friend class CTSPIConnection;
    friend class CTSPIDevice;

	// Original v2.x init function - not called if streams are used.
    virtual void Init(CTSPIDevice* pDev, DWORD dwLineDeviceID, DWORD dwPos, DWORD_PTR dwItemData=0);

	// Serialization stream support (v3).  Override "read" member.
	void InitWithStream(CTSPIDevice* pDevice, DWORD dwLineID, DWORD dwPos, TStream& istm);
	void SaveToStream(TStream& ostm);

	// Overridable functions and notifications
	virtual DWORD OnAddressFeaturesChanged (CTSPIAddressInfo* pAddr, DWORD dwFeatures);
    virtual void OnCallDeleted(CTSPICallAppearance* pCall);
	virtual DWORD OnCallFeaturesChanged(CTSPICallAppearance* pCall, DWORD dwCallFeatures);
    virtual void OnPreCallStateChange (CTSPIAddressInfo* pAddr, CTSPICallAppearance* pCall, DWORD dwNewState, DWORD dwOldState);
    virtual void OnCallStateChange (CTSPIAddressInfo* pAddr, CTSPICallAppearance* pCall, DWORD dwNewState, DWORD dwOldState);
	virtual DWORD OnLineFeaturesChanged(DWORD dwLineFeatures);
    virtual void OnRequestComplete (CTSPIRequest* pReq, LONG lResult);
	virtual void OnConnectedCallCountChange(CTSPIAddressInfo* pInfo, int iDelta);
    virtual void SetDeviceID(DWORD dwId);
	virtual bool CreateTapiTerminalDeviceClassInfo();
	virtual TString ConvertDialableToCanonical(LPCTSTR pszPartyID, DWORD dwCountryCode, bool fInbound=false);

	// Retrieves the private request map for line objects. This may
	// be overridden to provide line-by-line request management.
	virtual TRequestMap* GetRequestMap() const;

	// Callable functions from within the line object
    bool IsConferenceAvailable(CTSPICallAppearance* pCall);
    bool IsTransferConsultAvailable(CTSPICallAppearance* pCall);
	LONG FreeDialogInstance(HTAPIDIALOGINSTANCE htDlgInst);

	// Internal basic initialization called from both v2.x and v3.x versions
	void BaseInit(CTSPIDevice* pDevice, DWORD dwDeviceId);

	// Set the line type for use when non-stream INIT is used.
	void SetLineType(int iLineType);

	// Offers calls back up to TAPI when a line has been opened once again.
	friend unsigned __stdcall tsplib_OfferCallThread(void* pParam);
	bool OfferCallsToTAPISrv();

// Locked members
private:
	CTSPILineConnection(const CTSPILineConnection& to);
	CTSPILineConnection& operator=(const CTSPILineConnection& to);
};

/******************************************************************************/
// CTSPIAddressInfo
//
// This class defines the address information for a single address on 
// a line.  The address may contain one or more call appearances (although 
// generally only one is active at a time).
//
/******************************************************************************/
class CTSPIAddressInfo : public CTSPIBaseObject
{
// Class data
protected:
	enum {
		InputAvail		= 0x1,			// Address may answer calls
		OutputAvail		= 0x2,			// Address may place calls
		NotUseDN		= 0x4,			// Don't Show DN in AddressCaps
		NotifyNumCalls	= 0x8			// Transient flag -- set on PreCallStateChange
	};

	DWORD m_dwFlags;					// Flags for this address
    CTSPILineConnection* m_pLine;		// Line owner
    LINEADDRESSCAPS m_AddressCaps;		// Basic address capabilities
    LINEADDRESSSTATUS m_AddressStatus;	// Current available status on this address
    DWORD m_dwAddressID;				// Address identifier (0-numAddr).
    DWORD m_dwAddressStates;			// Which address state change messages need to be sent to TAPI.DLL?
    DWORD m_dwBearerMode;				// Available bearer mode for this address
    DWORD m_dwMinRateAvail;				// Minimum data rate on data stream (0 if not supported)
    DWORD m_dwMaxRateAvail;				// Maximum data rate on data stream (0 if not supported)
    DWORD m_dwCurrRate;					// Current data rate on address
	DWORD m_dwConnectedCallCount;		// Total count of CONNECTED cals on address
    TString m_strAddress;				// Dialable address (phone#).
    TString m_strName;					// Name of owner for this address (for callerid on outgoing calls)
	TDWordArray m_arrTerminals;			// Terminal array
    TSPIMEDIACONTROL* m_lpMediaControl; // Current MEDIACONTROL in effect (NULL if none).
	TDeviceClassArray m_arrDeviceClass; // Device class array for lineGetID
	TMapDWordToString m_mapCallTreatment; // Call treatments available on this address.
	TStringArray m_arrCompletionMsgs;	// Completion message information
	TCallList m_lstCalls;				// Call appearances on this address (dynamic).
	TForwardInfoArray m_arrForwardInfo;	// Forwarding information
	TAgentCaps m_AgentCaps;				// Agent capabilities on this address
	TAgentStatus m_AgentStatus;			// Agent status on this address
	LINEDIALPARAMS m_DialParams;		// Dialing parameters for this address
	TMSPArray m_arrMSPInstance;			// MSP tracking array
	DWORD m_dwAddressType;				// Type of address (IP, phone, domain, etc.)

// Constructor
public:
    CTSPIAddressInfo();					// Should only be created by the CreateObject method.
    virtual ~CTSPIAddressInfo();

// Access methods
public:
    // The following are the QUERYxxx functions for the static non-changing data
    DWORD GetAddressID() const;
    CTSPILineConnection* GetLineOwner() const;
    bool CanAnswerCalls() const;
    bool CanMakeCalls() const;
    DWORD GetBearerMode() const;
    DWORD GetCurrentRate() const;
	const LINEDIALPARAMS* GetDialingParameters() const;

	// TAPI address structures
    LPLINEADDRESSCAPS GetAddressCaps();
    LPLINEADDRESSSTATUS GetAddressStatus();
    const LINEADDRESSCAPS* GetAddressCaps() const;
    const LINEADDRESSSTATUS* GetAddressStatus() const;

	// This "moves" a call to this line/address.
	CTSPICallAppearance* MoveCall(CTSPICallAppearance* pCall, DWORD dwReason=0, DWORD dwState=0, DWORD dwMode=0, DWORD dwWaitTime=0);

	// Agent support
	const TAgentCaps* GetAgentCaps() const;
	const TAgentStatus* GetAgentStatus() const;
	TAgentCaps* GetAgentCaps();
	TAgentStatus* GetAgentStatus();

	// These allow changing the name/address of the object if it cannot be determined
	// at INIT time (such as configuring a newly added address.
    LPCTSTR GetDialableAddress() const;
	void SetDialableAddress(LPCTSTR pwszAddress, bool fShowInCaps=true);
    LPCTSTR GetName() const;
	void SetName (LPCTSTR pwszName);

    // Media mode support
    DWORD GetAvailableMediaModes () const;

	// Address type support - TAPI 3.0
	DWORD GetAddressType() const;

	// This function creates a new call on the address.  This would be used for incoming/outgoing calls.
    CTSPICallAppearance* CreateCallAppearance(HTAPICALL hCall=NULL, DWORD dwCallParamFlags=0, DWORD dwOrigin=LINECALLORIGIN_UNKNOWN,
                                    DWORD dwReason=LINECALLREASON_UNKNOWN, DWORD dwTrunk=0xffffffff, DWORD dwCompletionID=0);

	// This function creates a new CONFERENCE object to manage a conference call on the address.
    CTSPIConferenceCall* CreateConferenceCall(HTAPICALL hCall);

	// This function removes a call object (either normal or conference).  The call is deleted.
    void RemoveCallAppearance(CTSPICallAppearance* pCall);

    // Methods to access/manipulate the call list.
	int GetCallCount() const;
    CTSPICallAppearance* GetCallInfo(int iPos) const;
    CTSPICallAppearance* FindCallByState(DWORD dwCallState) const;
    CTSPICallAppearance* FindCallByHandle (HTAPICALL hCall) const;
    CTSPICallAppearance* FindCallByCallID (DWORD dwCallID) const;
	CTSPICallAppearance* FindAttachedCall (CTSPICallAppearance* pSCall) const;

    // Forwarding information - for existing forward information to be added.  For any forwards
    // performed by service provider, the information will automatically be added to the
    // array when the forward request completes and returns success.
    int AddForwardEntry (DWORD dwForwardMode, LPCTSTR pszCaller, LPCTSTR pszDestination, DWORD dwDestCountry);

	// Functions to manipulate the device class array
	int AddDeviceClass (LPCTSTR pszClass, DWORD dwData);
	int AddDeviceClass (LPCTSTR pszClass, LPCTSTR lpszBuff, DWORD dwType = -1L);
	int AddDeviceClass (LPCTSTR pszClass, HANDLE hHandle, LPCTSTR lpszBuff);
	int AddDeviceClass (LPCTSTR pszClass, HANDLE hHandle, LPVOID lpBuff, DWORD dwSize);
	int AddDeviceClass (LPCTSTR pszClass, DWORD dwFormat, LPVOID lpBuff, DWORD dwSize, HANDLE hHandle=INVALID_HANDLE_VALUE);
	bool RemoveDeviceClass (LPCTSTR pszClass);
	DEVICECLASSINFO* GetDeviceClass(LPCTSTR pszClass);

	// This function adds a new call treatment entry to our array.
	void AddCallTreatment (DWORD dwCallTreatment, LPCTSTR pszName);
	void RemoveCallTreatment (DWORD dwCallTreatment);
	TString GetCallTreatmentName (DWORD dwCallTreatment) const;

    // Completion message support
    void AddCompletionMessage(LPCTSTR pszBuff);
    unsigned int GetCompletionMessageCount() const;
    TString GetCompletionMessage (unsigned int iPos) const;

	// Some quick access functions for agent support based on the current agent
	// logged into the address.
	DWORD GetCurrentAgentState() const;
	unsigned int GetCurrentAgentGroupCount() const;
	const TAgentGroup* GetCurrentAgentGroup(unsigned int iPos) const;
	void AddAgentGroup(DWORD dwGroupID1, DWORD dwGroupID2=0, DWORD dwGroupID3=0, DWORD dwGroupID4=0, LPCTSTR pszGroupName=NULL);
	void RemoveAllAgentGroups();

	// Retrieve the logged on agent handle (TAPI)
	HAGENT GetActiveAgentHandle() const;

    // The following are the SETxxx functions
    void SetNumRingsNoAnswer (DWORD dwNumRings);
    void SetTerminalModes (unsigned int iTerminalID, DWORD dwTerminalModes, bool fRouteToTerminal);
	void SetCurrentRate (DWORD dwRate);
	void SetAddressFeatures(DWORD dwFeatures);
    void SetMediaControl (TSPIMEDIACONTROL* lpMediaControl);
	void SetAgentState(DWORD dwState, DWORD dwNextState);
	void SetAgentFeatures(DWORD dwFeatures);
	void SetAgentGroup(TAgentGroupArray* parrGroups);
	void SetAgentActivity(DWORD dwActivity);
	void SetValidAgentStates(DWORD dwStates);
	void SetValidNextAgentStates(DWORD dwNextStates);
	
// Overridable functions
public:
	// Serialization support (v3.x)
	virtual TStream& read(TStream& istm);
	virtual TStream& write(TStream& ostm) const;

    // Method which can be overridden to affect call selection during lineMakeCall and
    // verification of a CallParams structure.
    virtual LONG CanSupportCall (const LPLINECALLPARAMS lpCallParams) const;

    // Method which can be overridden to check forwarding information for this address.
	virtual LONG CanForward(TForwardInfoArray* parrForwardInfo, LPDWORD pdwNumRings, int iCount);
                 
    // This is called to determine if the address can support a particular set of media modes.
    virtual bool CanSupportMediaModes (DWORD dwMediaModes) const;

    // TAPI methods
    virtual LONG Unpark (DRV_REQUESTID dwRequestID, HTAPICALL htCall, LPHDRVCALL lphdCall, TDialStringArray* parrAddresses);
    virtual LONG Pickup (DRV_REQUESTID dwRequestID, HTAPICALL htCall, LPHDRVCALL lphdCall, TDialStringArray* parrDial, LPCTSTR pszGroupID);
    virtual LONG SetupTransfer(DRV_REQUESTID dwRequestID, CTSPICallAppearance* pCall, HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall, LPLINECALLPARAMS lpCallParams);
    virtual LONG CompleteTransfer (CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestId, CTSPICallAppearance* pConsult, HTAPICALL htConfCall, LPHDRVCALL lphdConfCall, DWORD dwTransferMode);
    virtual LONG SetupConference (DRV_REQUESTID dwRequestID, CTSPICallAppearance* pCall, HTAPICALL htConfCall, LPHDRVCALL lphdConfCall, HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall, DWORD dwNumParties, LPLINECALLPARAMS lpCallParams);
    virtual LONG Forward (DRV_REQUESTID dwRequestId, TForwardInfoArray* parrForwardInfo, DWORD dwNumRingsNoAnswer, HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall, LPLINECALLPARAMS lpCallParams);
    virtual LONG GatherCapabilities (DWORD dwTSPIVersion,DWORD dwExtVersion,LPLINEADDRESSCAPS lpAddressCaps);
    virtual LONG GatherStatusInformation (LPLINEADDRESSSTATUS lpAddressStatus);
    virtual void SetStatusMessages(DWORD dwStates);
	virtual LONG DevSpecific(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, LPVOID lpParam, DWORD dwSize);
	virtual LONG SetAgentGroup(DRV_REQUESTID dwRequestID, LPLINEAGENTGROUPLIST lpGroupList);
	virtual LONG SetAgentState(DRV_REQUESTID dwRequestID, DWORD dwState, DWORD dwNextState);
	virtual LONG SetAgentActivity(DRV_REQUESTID dwRequestID, DWORD dwActivity);
	virtual LONG AgentSpecific(DRV_REQUESTID dwRequestID, DWORD dwAgentExtensionIDIndex, LPVOID lpvBuff, DWORD dwSize);
	virtual LONG GatherAgentCapabilities(LPLINEAGENTCAPS lpAgentCaps);
	virtual LONG GatherAgentStatus(LPLINEAGENTSTATUS lpAgentStatus);
	virtual LONG GetAgentActivityList(LPLINEAGENTACTIVITYLIST lpActivityList);
	virtual LONG GetAgentGroupList(LPLINEAGENTGROUPLIST lpGroupList);
    virtual LONG GetID(const TString& strDevClass, LPVARSTRING lpDeviceID, HANDLE hTargetProcess);
	virtual LONG CreateMSPInstance(HTAPIMSPLINE htMSPLine, LPHDRVMSPLINE lphdMSPLine);
	virtual LONG CloseMSPInstance(CMSPDriver* pMSP);

#ifdef _DEBUG
	virtual TString Dump() const;
#endif

    // The following is called when any of the ADDRESSSTATE entries have changed.  It
    // is used internally, but should also be called if the derived service provider
    // changes address state information itself.
    virtual void OnAddressStateChange (DWORD dwAddressState);

    // The following should be called if any of the address capabilities change
    // during the life of the service provider.
    virtual void OnAddressCapabiltiesChanged();

	// The following should be called when any of the agent capabilities or
	// status information changes on this address.
    virtual void OnAgentCapabiltiesChanged();
    virtual void OnAgentStatusChanged(DWORD dwState, DWORD dwP2=0L);

	// Force a recalc of the address features based on call counts and line status
	virtual void RecalcAddrFeatures();

// Internal methods
protected:
    friend class CTSPILineConnection;
    friend class CTSPIRequest;
    friend class CTSPICallAppearance;
	friend class CTSPIConferenceCall;
	friend class CServiceProvider;

    // This method is called when the address connection is created directly after
    // the constructor, it is called by the CTSPILineConnection object
    virtual void Init(CTSPILineConnection* pLine, DWORD dwAddressID, LPCTSTR lpszAddress, 
					   LPCTSTR lpszName, bool fIncoming, bool fOutgoing, DWORD dwAvailMediaModes, 
                       DWORD dwAvailBearerModes, DWORD dwMinRate, DWORD dwMaxRate,
                       DWORD dwMaxNumActiveCalls, DWORD dwMaxNumOnHoldCalls, 
                       DWORD dwMaxNumOnHoldPendCalls, DWORD dwMaxNumConference, 
                       DWORD dwMaxNumTransConf, DWORD dwAddressSharing, const LPLINEDIALPARAMS lpDialParams, DWORD dwAddressType);

	// Internal function to dump the ADDRESSINFO structure to a stream.
	void SaveToStream(TStream& ostm) const;

    // Notification for a new call appearance created
    virtual void OnCreateCall (CTSPICallAppearance* pCall);

	// This method is called right before TAPI is notified about a call state change.
    virtual void OnPreCallStateChange (CTSPICallAppearance* pCall, DWORD dwState, DWORD dwOldState);

    // This method is called by the call appearances when the call state changes.
    virtual void OnCallStateChange (CTSPICallAppearance* pCall, DWORD dwState, DWORD dwOldState);

    // This method is called whenever the terminal line count changes.              
    virtual void OnTerminalCountChanged (bool fAdded, int iPos, DWORD dwMode=0L);

	// Called when a request completes on this address
    virtual void OnRequestComplete (CTSPIRequest* pReq, LONG lResult);
    
	// The following is called by the call appearance when the call features for the
	// call have changed.  The return value is the adjusted call features.
	virtual DWORD OnCallFeaturesChanged(CTSPICallAppearance* pCall, DWORD dwCallFeatures);
	virtual DWORD OnAddressFeaturesChanged (DWORD dwFeatures);

	// The following is called to check and send the LINEADDRESSTATUS_INUSEZERO notifcation.
	// If your device handles this a bit differently, override this functon.
	virtual bool NotifyInUseZero(); // v3.043

    // Various support methods
    int AddAsynchRequest(CTSPIRequest* pReq);
    void DeleteForwardingInfo();
    DWORD GetTerminalInformation (unsigned int iTerminalID) const;

// Locked members
private:
	CTSPIAddressInfo(const CTSPIAddressInfo& to);
	CTSPIAddressInfo& operator=(const CTSPIAddressInfo& to);
};

/******************************************************************************/
//
// CTSPICallHub
//
// This class defines a owner for calls which share a common call-id.
// The library refers to these as "shadow calls" since they are normally
// used to map two in-switch calls connected to each other.
//
/******************************************************************************/
class CTSPICallHub : public CTSPIBaseObject
{
// Class data
protected:
	DWORD m_dwCallID;		// Call hub callid
	TCallHubList m_lstHub;	// Array of CTSPICallAppearance ptrs

// Constructor
public:
	CTSPICallHub(DWORD dwCallID);
	virtual ~CTSPICallHub();

// Methods to manipulate the call hub list
public:
    unsigned int GetHubCount() const;
    CTSPICallAppearance* GetCall(unsigned int iPos) const;
	CTSPICallAppearance* GetShadowCall(const CTSPICallAppearance* pCall) const;
    bool IsCallInHub(const CTSPICallAppearance* pCall) const;
	void AddToHub(CTSPICallAppearance* pCall);
    void RemoveFromHub(const CTSPICallAppearance* pCall);
	void OnCallStateChange(CTSPICallAppearance* pCall, DWORD dwState, DWORD dwCurrState) const;

#ifdef _DEBUG
	virtual TString Dump() const;
#endif

// Locked members
private:
	CTSPICallHub(const CTSPICallHub& to);
	CTSPICallHub& operator=(const CTSPICallHub& to);
};

/******************************************************************************/
//
// CTSPICallAppearance
//
// This class defines a specific call on a line device.  Each line
// can have one or more available calls on it.  Each call has a 
// particular state on the line, and is associated to a specific address.
//
/******************************************************************************/
class CTSPICallAppearance : public CTSPIBaseObject
{
// Class data
public:
	enum Flags { 
		_IsRealCall = 0x0001,			// Bit is set when call is a "real" call on the HW.
		_IsDeleted	= 0x0002,			// Bit is set when call is no longer active.
		_IsDropped	= 0x0004,			// Bit is set when call is being dropped.
		_InitNotify = 0x0008,			// Bit is set when TAPI is told about call 1st time.
		_ChgState   = 0x0010,			// Bit is set during SetCallState
		_Outgoing   = 0x0020			// Bit is set when this is an outgoing call
	};

	enum CallType {
		Normal		= 1,				// Normal call created with CreateCallAppearance
		Conference	= 2,				// Conference call created with CreateConferenceCall	
		Consultant  = 3					// Consultation call created by Xfer/Conference event
	};
protected:
    CTSPIAddressInfo* m_pAddr;			// Address identifier for this call.
	DWORD m_dwFlags;					// Call flags
	DWORD m_lDeleteTime;				// >0 indicates when object should be deleted.
    LINECALLINFO m_CallInfo;			// Current call information for this object
    LINECALLSTATUS m_CallStatus;		// Current call status
    HTAPICALL m_htCall;					// TAPI opaque call handle
    CALLIDENTIFIER m_CallerID;			// Caller ID information
    CALLIDENTIFIER m_CalledID;			// Called ID information
    CALLIDENTIFIER m_ConnectedID;		// Connected ID information
    CALLIDENTIFIER m_RedirectionID;		// Redirection ID information
    CALLIDENTIFIER m_RedirectingID;		// Redirecting ID information
    TDWordArray m_arrTerminals;			// Terminal array
    TSPIMEDIACONTROL* m_lpMediaControl; // Current MEDIACONTROL in effect (NULL if none).
    CTSPICallAppearance* m_pConsult;	// Attached consultant call (NULL if none)
	CTSPIConferenceCall* m_pConf;		// Attached conference call (NULL if none)
	CTSPICallHub* m_pCallHub;			// Attached call hub (NULL if none)
	SIZEDDATA m_sdCallData;				// Call Data
	SIZEDDATA m_sdSendingFS;			// Sending FLOWSPEC (QOS)
	SIZEDDATA m_sdReceivingFS;			// Receiving FLOWSPEC (QOS)
	SIZEDDATA m_sdChargingInfo;			// Charging information
	SIZEDDATA m_sdLowLevelInfo;			// Low level compatibility information
	SIZEDDATA m_sdHiLevelInfo;			// Hi level compatibility information
	TDeviceClassArray m_arrDeviceClass; // Device class array for lineGetID
	TMonitorToneArray m_arrMonitorTones;// Current tones being monitored for.
	TTimerEventArray m_arrEvents;		// Pending timer events for MEDIACONTROL and TONE DETECT.
	TUserInfoArray m_arrUserUserInfo;	// Array of UserUserInfo structures.
    int m_iCallType;					// Call type (CALLTYPE_xxxx)
	TString m_strDialedDigits;			// Partially dialed digits
	std::auto_ptr<TSPIDIGITGATHER> m_lpGather;	// Current DIGITGATHER in effect (NULL if none).

// Constructor
public:
    CTSPICallAppearance();
    virtual ~CTSPICallAppearance();
	void CopyCall(CTSPICallAppearance* pCall, bool fShadowCall=true, bool fReplaceCallerID=true);

// Public methods
public:
    // The TAPI call handle defines the current call we are working on.
    // There will always be a line handle when connected, but there will
    // only be a call handle when something specific is being performed
    // on a line device through TAPI.
    HTAPICALL GetCallHandle() const;
	bool CreateCallHandle();

	// Returns whether the object has been deallocated by TAPI.
	bool HasBeenDeleted() const;

	// The following are for PBX systems which cannot do systematic dialing (i.e.
	// partial-dial).  The call object can "cache" the dialed number so that
	// it may be used in a subsequent dialing request.
	bool IsRealCall() const;
	void MarkReal(bool fIsReal=true);
	TString& GetPartiallyDialedDigits();

    // The following are the QUERYxxx functions for the state information of the call.      
    CTSPILineConnection* GetLineOwner() const;
    CTSPIAddressInfo* GetAddressOwner() const;
    DWORD GetCallState() const;
    CTSPICallAppearance* GetAttachedCall() const;
	CTSPIConferenceCall* GetConferenceOwner() const;

	// TAPI Call structures
    LPLINECALLINFO GetCallInfo();
    LPLINECALLSTATUS GetCallStatus();
    const LINECALLINFO* GetCallInfo() const;
    const LINECALLSTATUS* GetCallStatus() const;

	// This supports our call hub mechanism for shared callid calls. To change
	// the active call hub for a call simply call SetCallID().
	CTSPICallHub* GetCallHub() const;

	// This is a quick function to retrieve the "shadow" call for this
	// call if there are only two calls in the call hub.
	CTSPICallAppearance* GetShadowCall() const;

	// Consultation call attachment (v3.0b)
	CTSPICallAppearance* CreateConsultationCall(HTAPICALL htCall=NULL, DWORD dwCallParamFlags=0);
	void SetConsultationCall(CTSPICallAppearance* pCall);
	CTSPICallAppearance* GetConsultationCall() const;

	// This allows query/set of the call type (consultant/conference/normal)
    int GetCallType() const;
    void SetCallType (int iCallType);

	// Returns the current set of call flags.
	DWORD& GetCallFlags();

	// Quick function to retrieve Callid from LINECALLINFO.
	DWORD GetCallID() const;

	// Returns whether this is an outgoing call
	bool IsOutgoingCall() const;

	// Functions to manipulate the device class array
	int AddDeviceClass (LPCTSTR pszClass, DWORD dwData);
	int AddDeviceClass (LPCTSTR pszClass, LPCTSTR lpszBuff, DWORD dwType = -1L);
	int AddDeviceClass (LPCTSTR pszClass, HANDLE hHandle, LPCTSTR lpszBuff);
	int AddDeviceClass (LPCTSTR pszClass, HANDLE hHandle, LPVOID lpBuff, DWORD dwSize);
	int AddDeviceClass (LPCTSTR pszClass, DWORD dwFormat, LPVOID lpBuff, DWORD dwSize, HANDLE hHandle=INVALID_HANDLE_VALUE);
	bool RemoveDeviceClass (LPCTSTR pszClass);
	DEVICECLASSINFO* GetDeviceClass(LPCTSTR pszClass);

    // Internal methods for consultant call management.
    void AttachCall(CTSPICallAppearance* pCall);
	void SetConferenceOwner(CTSPIConferenceCall* pCall);
    void DetachCall();

#ifdef _DEBUG
    LPCTSTR GetCallStateName (DWORD dwState=0L) const;
#endif
    // Return whether the supplied callstate is ACTIVE according to TAPI rules.
    static bool IsActiveCallState(DWORD dwState);
	static bool IsConnectedCallState(DWORD dwState);

    // This method should be called whenever a digit is detected.  It manages any monitor
    // or gathering being performed on the call.  Two versions are supplied so that
	// the passed digit may or may not be double-byte.
#ifdef _UNICODE
    void OnDigit (DWORD dwType, char cDigit);
#endif
	void OnDigit (DWORD dwType, TCHAR cDigit);
            
    // This method should be called whenever a tone generation is detected.  It manages
    // any monitor or gathering being performed on the call.                
    void OnTone (DWORD dwFreq1, DWORD dwFreq2=0, DWORD dwFreq3=0);

    // This method is called during the initial call setup, or when a new media type
    // begins playing over the media stream.  It is automatically called during call state
    // changes if the media mode is adjusted by the SetCallState method.
    void OnDetectedNewMediaModes (DWORD dwMediaModes);
    
    // This method should be called if user-user information is received from
    // the underlying network.  The data is COPIED into an internal buffer and
    // may be deleted after the call.
    void OnReceivedUserUserInfo (const void* lpBuff, DWORD dwSize);

	// Force the call to recalc its featureset.
	virtual void RecalcCallFeatures(DWORD dwState=0);

	// Callerid information
	const CALLIDENTIFIER& GetCallerIDInformation() const;
	const CALLIDENTIFIER& GetCalledIDInformation() const;
	const CALLIDENTIFIER& GetConnectedIDInformation() const;
	const CALLIDENTIFIER& GetRedirectingIDInformation() const;
	const CALLIDENTIFIER& GetRedirectionIDInformation() const;
    
    // TAPI methods called by the the SPDLL wrappers class.
    virtual LONG Close();
    virtual LONG Drop(DRV_REQUESTID dwRequestId=0, LPCSTR lpszUserUserInfo=NULL, DWORD dwSize=0);
    virtual LONG Accept(DRV_REQUESTID dwRequestID, LPCSTR lpszUserUserInfo, DWORD dwSize);
    virtual LONG Answer(DRV_REQUESTID dwReq, LPCSTR lpszUserUserInfo, DWORD dwSize);
    virtual LONG BlindTransfer(DRV_REQUESTID dwRequestId, TDialStringArray* parrDestAddr, DWORD dwCountryCode);
    virtual LONG Dial (DRV_REQUESTID dwRequestID, TDialStringArray* parrAddresses, DWORD dwCountryCode);
	virtual LONG DevSpecific(DRV_REQUESTID dwRequestID, LPVOID lpParam, DWORD dwSize);
    virtual LONG Hold (DRV_REQUESTID dwRequestID);
    virtual LONG SwapHold(DRV_REQUESTID dwRequestID, CTSPICallAppearance* pCall);
    virtual LONG Unhold (DRV_REQUESTID dwRequestID);
    virtual LONG Secure (DRV_REQUESTID dwRequestID);
    virtual LONG SendUserUserInfo (DRV_REQUESTID dwRequestID, LPCSTR lpszUserUserInfo, DWORD dwSize);
    virtual LONG Park (DRV_REQUESTID dwRequestID, DWORD dwParkMode, TDialStringArray* parrAddresses, LPVARSTRING lpNonDirAddress);
    virtual LONG Unpark (DRV_REQUESTID dwRequestID, TDialStringArray* parrAddresses);
    virtual LONG Pickup (DRV_REQUESTID dwRequestID, TDialStringArray* parrDial, LPCTSTR pszGroupID);
    virtual LONG Redirect (DRV_REQUESTID dwRequestID, TDialStringArray* parrAddresses, DWORD dwCountryCode);
    virtual LONG SetCallParams (DRV_REQUESTID dwRequestID, DWORD dwBearerMode, DWORD dwMinRate, DWORD dwMaxRate, LPLINEDIALPARAMS lpDialParams);
    virtual LONG MakeCall (DRV_REQUESTID dwRequestID, TDialStringArray* parrDialInfo, DWORD dwCountryCode, LPLINECALLPARAMS lpCallParams);
    virtual LONG GatherDigits (TSPIDIGITGATHER* lpGather);
    virtual LONG GenerateDigits (DWORD dwEndToEndID, DWORD dwDigitMode, LPCTSTR lpszDigits, DWORD dwDuration);
    virtual LONG GenerateTone (DWORD dwEndToEndID, DWORD dwToneMode, DWORD dwDuration, DWORD dwNumTones, LPLINEGENERATETONE lpTones);
    virtual LONG SetMediaMode (DWORD dwMediaMode); 
    virtual LONG MonitorDigits (DWORD dwDigitModes);
    virtual LONG MonitorMedia (DWORD dwMediaModes);
    virtual LONG MonitorTones (TSPITONEMONITOR* lpMon);
    virtual LONG CompleteCall (DRV_REQUESTID dwRequestId, LPDWORD lpdwCompletionID, DWORD dwCompletionMode, DWORD dwMessageID);
    virtual LONG GatherStatusInformation(LPLINECALLSTATUS lpCallStatus);
    virtual LONG GatherCallInformation (LPLINECALLINFO lpCallInfo);
    virtual LONG GetID (const TString& strDevClass, LPVARSTRING lpDeviceID, HANDLE hTargetProcess);
    virtual LONG ReleaseUserUserInfo(DRV_REQUESTID dwRequest);
    virtual LONG SetAppSpecificData(DWORD dwAppSpecific);
	virtual LONG SetQualityOfService (DRV_REQUESTID dwRequestID, LPVOID lpSendingFlowSpec, DWORD dwSendingFlowSpecSize, LPVOID lpReceivingFlowSpec, DWORD dwReceivingFlowSpecSize);
	virtual LONG SetCallTreatment(DRV_REQUESTID dwRequestID, DWORD dwCallTreatment);
	virtual LONG SetCallData (DRV_REQUESTID dwRequestID, LPVOID lpCallData, DWORD dwSize);
	virtual LONG GetCallIDs(LPDWORD lpdwAddressID,LPDWORD lpdwCallID, LPDWORD lpdwRelatedCallID);

    // The following are the SETxxx functions for the CALLINFO of the call appearance.
    // They will cause the appropriate LINECALLSTATE message to be generated.
    void SetBearerMode(DWORD dwBearerMode);
    void SetDataRate(DWORD dwDataRate);
    void SetCallID (DWORD dwCallID);
    void SetRelatedCallID (DWORD dwCallID);
    void SetCallParameterFlags (DWORD dwFlags);
	void SetDialParameters (LPLINEDIALPARAMS pdp);
    void SetCallOrigin(DWORD dwOrigin);
    void SetCallReason(DWORD dwReason);
    void SetDestinationCountry (DWORD dwCountryCode);
    void SetTrunkID (DWORD dwTrunkID);
    void SetCallerIDInformation (DWORD dwFlags, LPCTSTR lpszPartyID=NULL, LPCTSTR lpszName=NULL, DWORD dwCountryCode=0, DWORD dwAddressType=0xffffffff);
    void SetCalledIDInformation (DWORD dwFlags, LPCTSTR lpszPartyID=NULL, LPCTSTR lpszName=NULL, DWORD dwCountryCode=0, DWORD dwAddressType=0xffffffff);
    void SetConnectedIDInformation (DWORD dwFlags, LPCTSTR lpszPartyID=NULL, LPCTSTR lpszName=NULL, DWORD dwCountryCode=0, DWORD dwAddressType=0xffffffff);
    void SetRedirectionIDInformation (DWORD dwFlags, LPCTSTR lpszPartyID=NULL, LPCTSTR lpszName=NULL, DWORD dwCountryCode=0, DWORD dwAddressType=0xffffffff);
    void SetRedirectingIDInformation (DWORD dwFlags, LPCTSTR lpszPartyID=NULL, LPCTSTR lpszName=NULL, DWORD dwCountryCode=0, DWORD dwAddressType=0xffffffff);
    void SetCallerIDInformation (const CALLIDENTIFIER& ci);
    void SetCalledIDInformation (const CALLIDENTIFIER& ci);
    void SetConnectedIDInformation (const CALLIDENTIFIER& ci);
    void SetRedirectionIDInformation (const CALLIDENTIFIER& ci);
    void SetRedirectingIDInformation (const CALLIDENTIFIER& ci);
    void SetCallState(DWORD dwState, DWORD dwMode=0L, DWORD dwMediaMode=0L, bool fTellTapi=true);
    void SetTerminalModes (unsigned int iTerminalID, DWORD dwTerminalModes, bool fRouteToTerminal);
	void SetCallData (LPCVOID lpvCallData, DWORD dwSize);
	void SetQualityOfService (LPCVOID lpSendingFlowSpec, DWORD dwSendSize, LPCVOID lpReceivingFlowSpec, DWORD dwReceiveSize);
	void SetCallTreatment(DWORD dwCallTreatment);
    void SetDigitMonitor(DWORD dwDigitModes);
    void SetMediaMonitor(DWORD dwModes);
	void SetCallFeatures (DWORD dwFeatures, bool fNotify=true);
	void SetCallFeatures2(DWORD dwCallFeatures2, bool fNotify=true);
    void SetMediaControl (TSPIMEDIACONTROL* lpMediaControl);
	void ReleaseUserUserInfo();

	// Use to receive data from an attached TAPI3 Media Service Provider
	virtual LONG ReceiveMSPData(CMSPDriver* pMSP, LPVOID lpData, DWORD dwSize);

	// Charging information values (v3.0b)
	const SIZEDDATA GetChargingInformation() const;
	void SetChargingInformation(LPCVOID lpChargingInformation, DWORD dwSize);

	// Low level compatibility information (v3.0b)
	const SIZEDDATA GetLowLevelCompatibilityInformation() const;
	void SetLowLevelCompatibilityInformation(LPCVOID lpLLCInformation, DWORD dwSize);

	// Hi level compatibility information(v3.0b)
	const SIZEDDATA GetHiLevelCompatibilityInformation() const;
	void SetHiLevelCompatibilityInformation(LPCVOID lpHLCInformation, DWORD dwSize);

	// Query methods for quality of service FLOWSPEC information (v3.0b)
	const SIZEDDATA GetSendingFlowSpec() const;
	const SIZEDDATA GetReceivingFlowSpec() const;

#ifdef _DEBUG
	virtual TString Dump() const;
#endif

// Internal methods
protected:
	friend class CServiceProvider;
    friend class CTSPIAddressInfo;
    friend class CTSPIConferenceCall;
    friend class CTSPIRequest;
	friend class CTSPILineConnection;
	friend class CTSPICallHub;
	friend class RTDropCall;

    void CompleteDigitGather (DWORD dwReason);
    void DeleteToneMonitorList();
    int AddAsynchRequest(CTSPIRequest* pReq);
	void NotifyCallStatusChanged();

    // This is the INIT function which is called directly after the constructor
    virtual void Init (CTSPIAddressInfo* pAddr, DWORD dwBearerMode = LINEBEARERMODE_VOICE,
                       DWORD dwRate=0, DWORD dwCallParamFlags=0, DWORD dwOrigin=LINECALLORIGIN_UNKNOWN,
                       DWORD dwReason=LINECALLREASON_UNKNOWN, DWORD dwTrunk=0xffffffff, 
                       DWORD dwCompletionID=0);
	void SetCallHandle(HTAPICALL htCall);

	// This is called when the call object is closed -OR- when the line is closed
	// and the call hasn't yet been dropped.
	void DropOnClose();

    // This method is called when information in our CALL status record has changed.
    // If the derived class changes data directly in the LINECALLSTATUS record, it
    // should invoke this method to tell TAPI. (Generally this won't happen).
    virtual void OnCallStatusChange (DWORD dwCallState, DWORD dwCallInfo, DWORD dwMediaModes);
    
    // This method is called when a Media Control event is detected.
    virtual void OnMediaControl (DWORD dwMediaControl);
    
    // This method is called when a TONE being monitored for is detected.
    virtual void OnToneMonitorDetect (DWORD dwToneListID, DWORD dwAppSpecific);

    // This method is called whenever the line terminal count is changed.
    virtual void OnTerminalCountChanged (bool fAdded, int iPos, DWORD dwMode=0L);

	// This method is called when an attached consultation call goes IDLE.
	virtual void OnConsultantCallIdle(CTSPICallAppearance* pConsultCall);

    // This method is called whenever information in our CALL information record
    // has changed.  If the derived class changes data directly in the LINECALLINFO
    // record, it should invoke this method to tell TAPI.
    virtual void OnCallInfoChange (DWORD dwCallInfo);

    // This method is called whenever a call which is related to this call changes
    // state.  The call relationship is made through the CALLINFO dwRelatedCallID field
    // and is used by conference and consultation calls to relate them to a call appearance.
    virtual void OnRelatedCallStateChange(CTSPICallAppearance* pCall, DWORD dwState, DWORD dwOldState);

	// This method is called when the other side of the call appearance
	// changes it's callstate.  This is used in station-to-station calls
	// within a PBX or ACD server environment.
	virtual void OnShadowCallStateChange(CTSPICallAppearance* pCall, DWORD dwState, DWORD dwCurrState);

    // This method is called internally during the library periodic interval timer.  
	// It is used to cancel digit gathering on timeouts.  If true is returned, the
	// call is removed from the "need timer" list.
    virtual bool OnInternalTimer();

	// This method is invoked when any request associated with this call has completed.
    virtual void OnRequestComplete (CTSPIRequest* pReq, LONG lResult);

	// This method is called to delete the call object -- it is overridden to
	// move the call object to a SP-owned array for client/server TAPI.
	virtual void DestroyObject();

	// This method is used to add a new timed event (TIMEREVENT) to the timed event list and
	// marks this call as a timer-required call.
	void AddTimedEvent(int nType, DWORD dwDuration, DWORD dwData1=0, DWORD dwData2=0);

// Locked members
private:
	CTSPICallAppearance(const CTSPICallAppearance& to);
	CTSPICallAppearance& operator=(const CTSPICallAppearance& to);
};

/******************************************************************************/
// 
// CTSPIConferenceCall
//
// This class describes a conference call.  It is derived from a basic
// call appearance, but is a special type of call which maintains a list
// of calls which are part of the conference.  Anything performed to the
// conference call could potentially effect all the calls within the 
// conference itself.
//
// It can be identified with a CallType of CALLTYPE_CONFERENCE.
//
/******************************************************************************/
class CTSPIConferenceCall : public CTSPICallAppearance
{
// Class data
protected:
	TCallHubList m_lstConference;	// Array of CTSPICallAppearance ptrs

// Constructor
public:
    CTSPIConferenceCall();
	virtual ~CTSPIConferenceCall();
    
// Methods specific for conferencing.   
public:                                                                                                           
    virtual LONG PrepareAddToConference(DRV_REQUESTID dwRequestID, HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall, LPLINECALLPARAMS lpCallParams);
    virtual LONG AddToConference (DRV_REQUESTID dwRequestID, CTSPICallAppearance* pCall);
    virtual LONG RemoveFromConference(DRV_REQUESTID dwRequestID, CTSPICallAppearance* pCall);
    
    // Methods to manipulate the conference call list.
    unsigned int GetConferenceCount() const;
    CTSPICallAppearance* GetConferenceCall(unsigned int iPos);
	void AddToConference(CTSPICallAppearance* pCall);
    void RemoveConferenceCall(CTSPICallAppearance* pCall, bool fForceBreakdown=true);
#ifdef _DEBUG
	virtual TString Dump() const;
#endif

// Internal methods
protected:
    friend CTSPICallAppearance;
    bool IsCallInConference(CTSPICallAppearance* pCall) const;
    bool CanRemoveFromConference(CTSPICallAppearance* pCall) const;
    virtual void OnRelatedCallStateChange (CTSPICallAppearance* pCall, DWORD dwState, DWORD dwOldState);
    virtual void OnCallStatusChange (DWORD dwState, DWORD dwMode, DWORD dwMediaMode);
    virtual void OnRequestComplete (CTSPIRequest* pReq, LONG lResult);

// Locked members
private:
	CTSPIConferenceCall(const CTSPIConferenceCall& to);
	CTSPIConferenceCall& operator=(const CTSPIConferenceCall& to);
};

/******************************************************************************/
//
// CPhoneButtonInfo
//
// This class contains all the elements from the PHONEBUTTONINFO
// structure, but allows the object to be stored in an object list
// and serialized.
//
// INTERNAL OBJECT
//
/******************************************************************************/
class CPhoneButtonInfo
{
// Internal predicate functions
public:
	class btn_pred_fm : public std::unary_function<CPhoneButtonInfo*, bool>
	{
	// Class data
	protected:
		DWORD m_dwMode;
		DWORD m_dwFunction;
	// Constructor
	public:
		btn_pred_fm(DWORD dwFunction, DWORD dwMode) : m_dwMode(dwMode), m_dwFunction(dwFunction){}
		bool operator() (CPhoneButtonInfo* pButton) const
		{ return (pButton->GetButtonMode() == m_dwMode && pButton->GetFunction() == m_dwFunction); }
	};
	class btn_pred_name : public std::unary_function<CPhoneButtonInfo*, bool>
	{
	// Class data
	protected:
		TString m_strName;
	// Constructor
	public:
		btn_pred_name(LPCTSTR pszName) : m_strName(pszName) {}
		bool operator() (CPhoneButtonInfo* pButton) const
		{ return (!lstrcmpi(m_strName.c_str(), pButton->GetDescription())); }
	};

// Class data
protected:
    DWORD m_dwButtonMode;			// Button mode (PHONEBUTTONMODE_xxx)
    DWORD m_dwButtonFunction;		// Button function (PHONEBUTTONFUNCTION_xxx)
    DWORD m_dwLampMode;				// Current lamp mode (PHONELAMPMODE_xxx)
    DWORD m_dwAvailLampModes;		// Available lamp modes
    DWORD m_dwButtonState;			// Current button state (PHONEBUTTONSTATE_xxx)
    TString m_strButtonDescription;	// Button description

// Constructor
public:      
    CPhoneButtonInfo();
    CPhoneButtonInfo(DWORD dwButtonFunction, DWORD dwButtonMode, DWORD dwAvailLamp, DWORD dwLampMode, LPCTSTR lpszDesc);

// Access Methods
public:
    LPCTSTR GetDescription() const;
    DWORD GetFunction() const;
    DWORD GetButtonMode() const;
    DWORD GetButtonState() const;
    DWORD GetLampMode() const;
    DWORD GetAvailLampModes() const;
    void SetLampMode(DWORD dwLampMode);
    CPhoneButtonInfo& operator=(LPPHONEBUTTONINFO lpPhoneInfo);
    void SetButtonInfo (DWORD dwFunction, DWORD dwMode, LPCTSTR lpszDesc);
    void SetButtonState (DWORD dwState);

// Locked members
private:
	CPhoneButtonInfo(const CPhoneButtonInfo& to);
	CPhoneButtonInfo& operator=(const CPhoneButtonInfo& to);
};

/******************************************************************************/
//
// CPhoneDisplay class
//
// This class manages a virtual "display" with cursor positioning, and
// linefeed interpretation.
//
// INTERNAL OBJECT
//
/******************************************************************************/
class CPhoneDisplay
{
// Class data
protected:
    SIZE m_sizDisplay;		 // Buffer size
    POINT m_ptCursor;		 // Cursor position
    TCHAR m_cLF;             // Line feed character
	tsplib::inauto_ptr<TCHAR> m_lpsDisplay; // Buffer for our display
    
// Methods
public:
    CPhoneDisplay();
    ~CPhoneDisplay();

// Access methods
public:
    void Init(int iCols, int iRows, TCHAR cLF=_T('\n'));
    
    // These query different capabilities of the device.
    const LPTSTR GetTextBuffer() const;
    const POINT& GetCursorPosition() const;
    const SIZE& GetDisplaySize() const;
	TCHAR GetLFChar() const;
    
    // These modify the display buffer
    void AddCharacter(TCHAR c);
    void AddString(LPCTSTR lpszText);
    void SetCharacterAtPosition(int iCol=-1, int iRow=-1, TCHAR c = 0);
    void SetCursorPosition(int iCol=-1, int iRow=-1);                  
    void Reset();
    void ClearRow(int iRow);

// Locked members
private:
	CPhoneDisplay(const CPhoneDisplay& to);
	CPhoneDisplay& operator=(const CPhoneDisplay& to);
};

/******************************************************************************/
//
// CTSPIPhoneConnection class
//
// This class describes a phone connection for TAPI.  It is based
// off the above CTSPIConnection class but contains data and methods
// specific to controlling a phone device.
//
/******************************************************************************/
class CTSPIPhoneConnection : public CTSPIConnection
{
// Typedefs
protected:
	typedef tsplib::ptr_vector<CPhoneButtonInfo> CPhoneButtonArray;
	
// Class data
protected:
    HTAPIPHONE m_htPhone;               // TAPI opaque phone handle
    PHONECAPS m_PhoneCaps;              // Phone capabilities
    PHONESTATUS m_PhoneStatus;          // Phone status
    PHONEEVENT m_lpfnEventProc;         // TAPI event callback for phone events
    DWORD m_dwPhoneStates;              // Notify states for phone
    DWORD m_dwButtonModes;              // Notify modes for all buttons
    DWORD m_dwButtonStates;             // Notify states for all buttons      
    CPhoneDisplay m_Display;            // Phone display
    CPhoneButtonArray m_arrButtonInfo;  // Button Information array
    TDWordArray m_arrUploadBuffers;		// Data buffers in upload area on phone
    TDWordArray m_arrDownloadBuffers;	// Sizes of each of the download areas available on phone.
	static TRequestMap g_mapRequests;	// Request map for all phones

// Constructor
public:
    CTSPIPhoneConnection();
	virtual ~CTSPIPhoneConnection();

// Methods
public:
    // The TAPI phone handle defines the phone device we are connected
    // to.  It is passed as the first parameter for a phone event callback.
    HTAPIPHONE GetPhoneHandle() const;

    // These functions should be called during intial setup (providerInit)
    // to setup the phone device with the correct count of upload/download buffers
    // display dimensions, and buttons.
    bool AddUploadBuffer(DWORD dwSizeOfBuffer);
    bool AddDownloadBuffer(DWORD dwSizeOfBuffer);
    bool AddButton (DWORD dwFunction, DWORD dwMode, DWORD dwAvailLampStates, DWORD dwLampState, LPCTSTR lpszText);
    void SetupDisplay (int iColumns, int iRows, char cLineFeed=_T('\n'));
	void AddHookSwitchDevice (DWORD dwHookSwitchDev, DWORD dwAvailModes, DWORD dwCurrMode, 
							  DWORD dwVolume=-1L, DWORD dwGain=-1L, 
							  DWORD dwSettableModes=-1L, DWORD dwMonitoredModes=-1L,
							  bool fSupportsVolumeChange=false, bool fSupportsGainChange = false);

    // These methods manage the button/lamp pairs for the phone.
    unsigned int GetButtonCount() const;
    const CPhoneButtonInfo* GetButtonInfo(unsigned int iButtonID) const;
	const CPhoneButtonInfo* FindButtonInfo(DWORD dwFunction, DWORD dwMode) const;
	const CPhoneButtonInfo* FindButtonInfo(LPCTSTR pszText) const;
    DWORD GetLampMode (int iButtonId);

	// TAPI Phone structures
    LPPHONECAPS GetPhoneCaps();
    LPPHONESTATUS GetPhoneStatus();
    const PHONECAPS* GetPhoneCaps() const;
    const PHONESTATUS* GetPhoneStatus() const;

    // These methods modify the display
    TString GetDisplayBuffer() const;
    const POINT& GetCursorPos() const;
    void AddDisplayChar (TCHAR cChar);
    void SetDisplayChar (int iColumn, int iRow, TCHAR cChar);
    void ResetDisplay();        
    void SetDisplay(LPCTSTR pszDisplayBuff);
    void SetDisplayCursorPos (int iColumn, int iRow);
    void ClearDisplayLine (int iRow);
    void AddDisplayString (LPCTSTR lpszText);    

	// Returns the associated line (if any)
	CTSPILineConnection* GetAssociatedLine() const;
    
// Overridable methods
public:
    // Unique id for this line/phone device
	void SetPermanentPhoneID(DWORD dwPhoneID);
    virtual DWORD GetPermanentDeviceID() const;

	// Serialization support
	virtual TStream& read(TStream& istm);
	virtual TStream& write(TStream& ostm) const;

    // TAPI requests (not all of these are asynchronous).
    virtual LONG Open (HTAPIPHONE htPhone, PHONEEVENT lpfnEventProc, DWORD dwTSPIVersion);
    virtual LONG Close ();
    virtual LONG GetButtonInfo (DWORD dwButtonId, LPPHONEBUTTONINFO lpButtonInfo);
	virtual LONG SetButtonInfo (DRV_REQUESTID dwRequestID, DWORD dwButtonId, LPPHONEBUTTONINFO const lpPhoneInfo);
    virtual LONG SetLamp (DRV_REQUESTID dwRequestID, DWORD dwButtonLampID, DWORD dwLampMode);
    virtual LONG GetDisplay (LPVARSTRING lpVarString);
    virtual LONG GetGain (DWORD dwHookSwitchDevice, LPDWORD lpdwGain);
    virtual LONG GetHookSwitch (LPDWORD lpdwHookSwitch);
    virtual LONG GetLamp (DWORD dwButtonId, LPDWORD lpdwLampMode);
    virtual LONG GetVolume (DWORD dwHookSwitchDev, LPDWORD lpdwVolume);
    virtual LONG SetGain (DRV_REQUESTID dwRequestId, DWORD dwHookSwitchDev, DWORD dwGain);
    virtual LONG SetVolume (DRV_REQUESTID dwRequestId, DWORD dwHookSwitchDev, DWORD dwVolume);
    virtual LONG SetHookSwitch (DRV_REQUESTID dwRequestId, DWORD dwHookSwitchDev, DWORD dwHookSwitchMode);
    virtual LONG SetRing (DRV_REQUESTID dwRequestID, DWORD dwRingMode, DWORD dwVolume);
    virtual LONG SetDisplay (DRV_REQUESTID dwRequestID, DWORD dwRow, DWORD dwCol, LPCTSTR lpszDisplay, DWORD dwSize);
    virtual LONG GetData (DWORD dwDataID, LPVOID lpData, DWORD dwSize);
    virtual LONG GetRing (LPDWORD lpdwRingMode, LPDWORD lpdwVolume);
    virtual LONG SetData (DRV_REQUESTID dwRequestID, DWORD dwDataID, LPCVOID lpData, DWORD dwSize);
    virtual LONG SetStatusMessages(DWORD dwPhoneStates, DWORD dwButtonModes, DWORD dwButtonStates);
    virtual LONG GatherCapabilities(DWORD dwTSPIVersion, DWORD dwExtVersion, LPPHONECAPS lpPhoneCaps);
    virtual LONG GatherStatus (LPPHONESTATUS lpPhoneStatus);
    virtual LONG DevSpecific(DRV_REQUESTID dwRequestId, LPVOID lpParams, DWORD dwSize);
	virtual LONG GenericDialogData (LPVOID lpParam, DWORD dwSize);
    
    // These are the SETxxx functions which notify TAPI.  They should only
    // be called by the worker code (not by TAPI).
    void SetButtonInfo (int iButtonID, DWORD dwFunction, DWORD dwMode, LPCTSTR pszName);
    DWORD SetLampState (int iButtonID, DWORD dwLampState);
    DWORD SetButtonState (int iButtonId, DWORD dwButtonState);
    DWORD SetStatusFlags (DWORD dwStatus);
    void SetRingMode (DWORD dwRingMode);
    void SetRingVolume (DWORD dwRingVolume);
    void SetHookSwitch (DWORD dwHookSwitchDev, DWORD dwMode);
    void SetVolume (DWORD dwHookSwitchDev, DWORD dwVolume);
    void SetGain (DWORD dwHookSwitchDev, DWORD dwGain);
	void SetPhoneFeatures (DWORD dwFeatures);

	// Force the phone to close
	void ForceClose();

	// Functions which notify TAPI about changes within data structures
    virtual void OnPhoneCapabiltiesChanged();
    virtual void OnPhoneStatusChange(DWORD dwState, DWORD dwParam = 0);

    // The event procedure is used as a callback into TAPISRV.EXE when
    // some event (phone state change, etc.) happens.  It will
    // be initialized when the phone is opened.
    void Send_TAPI_Event(DWORD dwMsg, DWORD dwP1 = 0L, DWORD dwP2 = 0L, DWORD dwP3 = 0L);

#ifdef _DEBUG
	virtual TString Dump() const;
#endif

// Internal methods
protected:
    friend class CTSPIDevice;
	friend class CTSPIConnection;
	friend class CServiceProvider;

    // This is called directly after the constructor to INIT the phone device.
    virtual void Init(CTSPIDevice* pDevice, DWORD dwPhoneId, DWORD dwPos, DWORD_PTR dwItemData=0);

	// Serialization support (v3) override "read" member
	void InitWithStream(CTSPIDevice* pDevice, DWORD dwPhoneID, DWORD dwPos, TStream& istm);
	void SaveToStream(TStream& ostm);

    // Method to send phonestate notifications
    virtual void OnRequestComplete (CTSPIRequest* pReq, LONG lResult);
    virtual void OnButtonStateChange (DWORD dwButtonID, DWORD dwMode, DWORD dwState);
	int FindButton (DWORD dwButtonFunction, DWORD dwButtonMode, int iCount);

	// Internal basic initialization called from both v2.x and v3.x versions
	void BaseInit(CTSPIDevice* pDevice, DWORD dwDeviceId);

	// Retrieves a "writable" pointer to a button
    CPhoneButtonInfo* GetWButtonInfo(unsigned int iButtonID);

	// Retrieves the private request map for phone objects. This may
	// be overridden to provide phone-by-phone request management.
	virtual TRequestMap* GetRequestMap() const;

// Locked members
private:
	CTSPIPhoneConnection(const CTSPIPhoneConnection& to);
	CTSPIPhoneConnection& operator=(const CTSPIPhoneConnection& to);
};

/******************************************************************************/
//
// CServiceProvider class
//
// This class is used to field all the calls for the service provider.
// It is based on the CWinApp class and provides the hookups for the
// Microsoft Foundation classes to properly work.
//
// This class manages multiple provider devices, and can have multiple
// DLL instances each with a seperate permanent provider id (PPID).
//
/******************************************************************************/
class CServiceProvider
{
// Class data
private:
	static CServiceProvider* g_pAppObject;  // Main application object
	HPROVIDER m_hProvider;				// Handle to device for this provider
	mutable HINSTANCE m_hInstance;		// HINSTANCE for this DLL
	HANDLE m_hevtShutdown;				// Shutdown event.
	HANDLE m_htLibraryTimer;			// Library internal thread handle
    LINEEVENT m_lpfnLineCreateProc;		// Support Plug&Play dynamic line creation
    PHONEEVENT m_lpfnPhoneCreateProc;	// Support Plug&Play dynamic phone creation
    DWORD m_dwTapiVerSupported;			// TAPI versions supported
    DWORD m_dwTAPIVersionFound;			// TAPI version on this system.
	TDeviceArray m_arrDevices;			// List of CTSPIDevice structures
    TCapsArray m_arrProviderCaps;		// Provider capabilities
    LPCTSTR m_pszProviderInfo;			// Provider information
	LPCTSTR m_pszUIName;				// User interface DLL name
	LPCSTR m_pObjects[TOTAL_DYNAMIC_OBJECTS];
	CRITICAL_SECTION m_csProvider;		// Critical section for PROVIDER data.
	TCallList m_lstDeletedCalls;		// List of "deleted" calls.
	TCallList m_lstTimedCalls;			// List of calls which need internal interval timer.
	TConnectionMap m_mapLineConn;		// Map of line connections within all devices.
	TConnectionMap m_mapPhoneConn;		// Map of phone connections within all devices.
	LOCATIONINFO* m_pcurrLocation;		// Current location information

// Constructor/Destructor
public:
    CServiceProvider(LPCTSTR pszUIName, LPCTSTR pszProviderInfo, DWORD dwTapiVer = TAPIVER_30); 
	virtual ~CServiceProvider();

// Initialization
public:
	static CServiceProvider* Instance();
	HINSTANCE GetResourceInstance() const;
	HANDLE GetShutdownEvent() const;
       
// Access methods
public:
    // This method retreives specific device connection objects
    CTSPIDevice* GetDevice(DWORD dwProviderID) const;
	DWORD GetDeviceCount() const;
	CTSPIDevice* GetDeviceByIndex(unsigned int iIndex) const;

    // Return what TAPI says the current location of this computer is.
    DWORD GetCurrentLocation() const;

	// Return whether we are operating under TAPISRV.EXE
	bool IsRunningUnderTapiSrv();

    // Return the name of the executable module (passed to CServiceProvider constructor).
    LPCTSTR GetUIManager() const;

    // Return provider information string (passed to CServiceProvider constructor).
    LPCTSTR GetProviderInfo() const;

    // Returns the support TAPI version (passed to CServiceProvider constructor).
    DWORD GetSupportedVersion() const;

	// Returns the version of TAPI installed in the system.
    DWORD GetSystemVersion() const;

	// Functions which add/remove call from our "need timer" list.
	void AddTimedCall(CTSPICallAppearance* pCall);
	void RemoveTimedCall(CTSPICallAppearance* pCall);

	// Registry storage manipulation methods which can be used to store information
	// about devices in the provider.
	TString ReadProfileString (DWORD dwDeviceID, LPCTSTR pszEntry, LPCTSTR pszDefault = _T(""));
	DWORD ReadProfileDWord (DWORD dwDeviceID, LPCTSTR pszEntry, DWORD dwDefault = 0L);
	bool WriteProfileString (DWORD dwDeviceID, LPCTSTR pszEntry, LPCTSTR pszValue);
	bool WriteProfileDWord (DWORD dwDeviceID, LPCTSTR pszEntry, DWORD dwValue);
	bool DeleteProfile (DWORD dwDeviceID, LPCTSTR pszEntry=NULL);
	bool RenameProfile (DWORD dwOldDevice, DWORD dwNewDevice);

    // These methods search all our connections for a specific phone or line connection entry.
    CTSPILineConnection* GetConnInfoFromLineDeviceID(DWORD dwDevId);
    CTSPIPhoneConnection* GetConnInfoFromPhoneDeviceID(DWORD dwDevId);

// Overridable methods
public:
    // This method checks the call parameters based on the line and specific address.  
    // It runs through all the addresses depending on the parameters passed.
    virtual LONG ProcessCallParameters(CTSPILineConnection* pLine, LPLINECALLPARAMS lpCallParams);

	// Function to add a call to our "delete" list.
	virtual void DeleteCall(CTSPICallAppearance* pCall);

    // This method checks the number to determine if we support the
    // dialable address.  The final form address is returned in the buffer specified.
    virtual LONG CheckDialableNumber(CTSPILineConnection* pLine, CTSPIAddressInfo* pAddr, LPCTSTR lpszDigits, TDialStringArray* parrEntries, DWORD dwCountry, LPCTSTR pszValidChars=NULL);
    virtual TString GetDialableNumber (LPCTSTR pszNumber, LPCTSTR pszAllowChar=NULL) const;

	// This is the main function which converts a dialable number from TAPI into the
	// displayed canonical format.  It utilizes the below two routines.
    virtual TString ConvertDialableToCanonical (LPCTSTR pszNumber, DWORD dwCountry=0, bool fInbound=false);

	// This function attempts to determine the proper country code from a 3-digit string.
	virtual TString DetermineCountryCode(const TString& strInput) const;

	// This function attempts to determine the proper area code from the dialing string.
	virtual TString DetermineAreaCode(const TString& strCountryCode, TString& strInput);

    // This function is called when matching a tone against one seen on the media stream.
    virtual bool MatchTones (DWORD dwSFreq1, DWORD dwSFreq2, DWORD dwSFreq3, DWORD dwTFreq1, DWORD dwTFreq2, DWORD dwTFreq3);

	// This is called to output an event to the debug log. It may be overridden to
	// dump information to a file, socket, etc.
	virtual void TraceOut(const TString& strBuff);

// Set methods for the derived class to use
protected:
    // Set the C++ objects to use for each basic telephony object in the system.  This must be
	// done during the constructor of the service provider.
    void SetRuntimeObjects(LPCSTR pszDevObj, LPCSTR pszLineObj = NULL, LPCSTR pszAddrObj = NULL, 
							LPCSTR pszCallObj = NULL, LPCSTR pszConfCall = NULL, 
							LPCSTR pszPhoneObj = NULL);

    // This method is used to determine if an asynch request should be
    // generated for each available event type.  By default, the request will
    // be generated if the function is exported by the DLL.
    bool CanHandleRequest(DWORD dwFunction) const;

//---- START OF INTERNAL METHODS - UNDOCUMENTED ----
// These should not be called by anything except the class library as they may be
// changed in future releases.
public:
	// New v3.0 extensions which allow serialization between UI and TSP
	bool ReadDeviceCount(DWORD dwProviderID, LPDWORD lpdwNumLines=NULL, LPDWORD lpdwNumPhones=NULL, LPDWORD lpdwNumAct=NULL, LPDWORD lpdwNumGroups=NULL);
		
	// Function which takes an input dialable number from TAPI and changes it to our
	// internal TDialStringArray object.
	LONG DialInputToArray(CTSPILineConnection* pLine, CTSPIAddressInfo* pAddr, LPCTSTR lpszDestAddr, DWORD dwCountryCode, TDialStringArray* pArray);
protected:    
	// Internal interval timer support
	friend unsigned __stdcall tsplib_LibraryTimerThread(void* pParam);
	void IntervalTimer();

    // Internal methods used for the dynamic object creation
    // code.  This allows the base class to derive new classes for
    // each of the main objects, but the internal files may still
    // allocate the objects for the derived class through these pointers.
	CTSPIDevice* CreateDeviceObject();
	CTSPILineConnection* CreateLineObject();
	CTSPIAddressInfo* CreateAddressObject();
	CTSPICallAppearance* CreateCallObject();
	CTSPIConferenceCall* CreateConferenceCallObject();
	CTSPIPhoneConnection* CreatePhoneObject();

	// This function is used to map line/phone device identifiers to specific
	// objects in our provider.  It allows for a quick lookup across all the
	// devices in the provider and allows for dynamic line/phone creation.
    void MapConnectionToID(DWORD dwDeviceId, CTSPIConnection* pConn);
	void RemoveConnectionFromMap(DWORD dwDeviceId, CTSPIConnection* pConn);

    // These methods retreive the line/phone creation procedures.  Simply add a new
    // line to the device to trigger these after the service provider has initialized.
    LINEEVENT GetLineCreateProc() const;
    PHONEEVENT GetPhoneCreateProc() const;
    
    // These all funnel back through the virtual function with the appropriate
    // paramaters filled out.
    LONG ProcessCallParameters (CTSPIAddressInfo* pAddr, LPLINECALLPARAMS lpCallParams);
    LONG ProcessCallParameters (CTSPICallAppearance* pCall, LPLINECALLPARAMS lpCallParams);

	// These are called to perform actions on the device class array by all
	// the difference objects (line,address,call,phone).
	int AddDeviceClassInfo (TDeviceClassArray& arrElem, LPCTSTR pszName, DWORD dwType, LPVOID lpBuff, DWORD dwSize, HANDLE hHandle=INVALID_HANDLE_VALUE);
	bool RemoveDeviceClassInfo (TDeviceClassArray& arrElem, LPCTSTR pszName);
	DEVICECLASSINFO* FindDeviceClassInfo (TDeviceClassArray& arrElem, LPCTSTR pszName);
	LONG CopyDeviceClass (DEVICECLASSINFO* pDeviceClass, LPVARSTRING lpDeviceID, HANDLE hTargetProcess);

private:
    // Determine the service provider capabilities and setup our flag array.
    void DetermineProviderCapabilities();

	// Used to delete registry tree during providerRemove when running under Windows NT.
	bool IntRegDeleteKey (HKEY hKeyTelephony, LPCTSTR pszMainDir);

//---- END OF UNDOCUMENTED FUNCTIONS ----

// TAPI callbacks
public:
    // This function is called before the TSPI_providerInit to determine
    // the number of line and phone devices supported by the service provider.
    // If the function is not available, then TAPI will read the information
    // out of the TELEPHON.INI file per TAPI 1.0.  TAPI 1.4 function
    virtual LONG providerEnumDevices(DWORD dwProviderId, LPDWORD lpNumLines,
                                 LPDWORD lpNumPhones, HPROVIDER hProvider,
                                 LINEEVENT lpfnLineCreateProc, 
                                 PHONEEVENT lpfnPhoneCreateProc);

    // This function is called by TAPI in response to the receipt of a 
    // LINE_CREATE message from the service provider which allows the dynamic
    // creation of a new line device.  The passed deviceId identifies this
    // line from TAPIs perspective.  TAPI 1.4 function
    virtual LONG providerCreateLineDevice(DWORD_PTR dwTempId, DWORD dwDeviceId);

    // This function is called by TAPI in response to the receipt of a
    // PHONE_CREATE message from the service provider which allows the dynamic
    // creation of a new phone device.  The passed deviceId identifies this
    // phone from TAPIs perspective.  TAPI 1.4 function
    virtual LONG providerCreatePhoneDevice(DWORD_PTR dwTempId, DWORD dwDeviceId);

	// This method is called when the service provider is first initialized.
	// It supplies the base line/phone ids for us and our permanent provider
	// id which has been assigned by TAPI.
	virtual LONG providerInit(DWORD dwTSPVersion, DWORD dwProviderId, DWORD dwLineIdBase,
                                 DWORD dwPhoneIdBase, DWORD_PTR dwNumLines, DWORD_PTR dwNumPhones, 
								 ASYNC_COMPLETION lpfnCompletionProc, LPDWORD lpdwTSPIOptions);

	// This method is called to shutdown our service provider.  It will
	// be called to shutdown a particular instance of our TSP.
	virtual LONG providerShutdown(DWORD dwTSPVersion, CTSPIDevice* pDevice);

	// This method is called when a UI dialog is terminating.
	virtual LONG providerFreeDialogInstance (HDRVDIALOGINSTANCE hdDlgInstance);

	// This method is called when the UI DLL sends information to the provider.
	virtual LONG providerGenericDialogData (CTSPIDevice* pDev, CTSPILineConnection* pLine, CTSPIPhoneConnection* pPhone, 
											HDRVDIALOGINSTANCE hdDlgInstance, LPVOID lpBuff, DWORD dwSize);

	// This method is called to identify the provider UI DLL
	virtual LONG providerUIIdentify (LPWSTR lpszUIDLLName);

	// This function is used as a general extension mechanims to allow
	// service providers to provide access to features not described in
	// other operations.
	virtual LONG lineDevSpecific(CTSPILineConnection* pLine, CTSPIAddressInfo* pAddr, CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestId, LPVOID lpParams, DWORD dwSize);

	// This function forwards calls destined for the specified address
	// according to the specified forwarding instructions.
	// When an origination address is forwarded, the incoming calls for that
	// address are deflected to the other number by the switch.  This function
	// provides a combination of forward and do-not-disturb features.  This
	// function can also cancel specific forwarding currently in effect.
	virtual LONG lineForward(CTSPILineConnection* pLine, CTSPIAddressInfo* pAddr, DRV_REQUESTID dwRequestId, const LPLINEFORWARDLIST lpForwardList, DWORD dwNumRingsAnswer, HTAPICALL htConsultCall, LPHDRVCALL lphdConsultCall,
							const LPLINECALLPARAMS lpCallParams);

	// This function is called to negotiate a compatible version between TAPI and 
	// this service provider.  It is only called for the initial global negotiation.
	virtual LONG lineNegotiateTSPIVersion(DWORD dwDeviceId, DWORD dwLowVersion, DWORD dwHiVersion, LPDWORD lpdwTSPVersion);

	// This function is called by TAPI whenever the address translation location
	// is changed by the user (in the Dial Helper dialog or 
	// 'lineSetCurrentLocation' function.  SPs which store parameters specific
	// to a location (e.g. touch-tone sequences specific to invoke a particular
	// PBX function) would use the location to select the set of parameters 
	// applicable to the new location.  Windows 95 only.
	virtual LONG lineSetCurrentLocation(DWORD dwLocation);

	// This function enables and disables control actions on the media stream
	// associated with the specified line, address, or call.  Media control actions
	// can be triggered by the detection of specified digits, media modes,
	// custom tones, and call states.  The new specified media controls replace all
	// the ones that were in effect for this line, address, or call prior to this
	// request.
	virtual LONG lineSetMediaControl(CTSPILineConnection *pConn, CTSPIAddressInfo* pAddr, CTSPICallAppearance* pCall, 
				  const LPLINEMEDIACONTROLDIGIT lpDigitList, DWORD dwNumDigitEntries, 
				  const LPLINEMEDIACONTROLMEDIA lpMediaList, DWORD dwNumMediaEntries, 
				  const LPLINEMEDIACONTROLTONE lpToneList, DWORD dwNumToneEntries, 
				  const LPLINEMEDIACONTROLCALLSTATE lpCallStateList, DWORD dwNumCallStateEntries);

// Define the friends of this class.
public:
    friend class CTSPIDevice;
    friend class CTSPIConnection;
    friend class CTSPILineConnection;
    friend class CTSPIPhoneConnection;
    friend class CTSPIAddressInfo;
    friend class CTSPICallAppearance;
    friend class CTSPIConferenceCall;

// Locked members
private:
	CServiceProvider(const CServiceProvider& to);
	CServiceProvider& operator=(const CServiceProvider& to);
};

/******************************************************************************/
// GetSP
//
// Public method to retrieve a pointer to the main service provider
// application object.
//
/******************************************************************************/
inline CServiceProvider* GetSP() { return CServiceProvider::Instance(); }

/******************************************************************************/
//
// CopyVarString
//
// Copy the buffer (either LPCTSTR or LPVOID) into the VARSTRING pointer.
//
/******************************************************************************/
void CopyVarString (LPVARSTRING lpVarString, LPCTSTR lpszBuff);
void CopyVarString (LPVARSTRING lpVarString, LPVOID lpBuff, DWORD dwSize);

/******************************************************************************/
// CopyCallParams
//
// This function copies the LINECALLPARAMS buffer and performs any required
// conversion on the buffers.
//
/******************************************************************************/
LINECALLPARAMS* CopyCallParams(LPLINECALLPARAMS const lpcpIn);

/******************************************************************************/
// 
// ConvertWideToAnsi
//
// Convert a WIDE (unicode) string into a single-byte ANSI string.
//
/******************************************************************************/
inline TString ConvertWideToAnsi(LPCWSTR lpszInput)
{ 
	USES_CONVERSION;
	return (lpszInput == NULL) ? _T("") : W2T(const_cast<LPWSTR>(lpszInput)); 
}

/******************************************************************************/
// 
// AddDataBlock
// 
// Fill in a VARLEN buffer with data.
//
/******************************************************************************/
bool AddDataBlock(LPVOID lpVB, DWORD& dwOffset, DWORD& dwSize,  LPCVOID lpBuffer, DWORD dwBufferSize);
bool AddDataBlock(LPVOID lpVB, DWORD& dwOffset, DWORD& dwSize, LPCSTR lpszBuff);
bool AddDataBlock(LPVOID lpVB, DWORD& dwOffset, DWORD& dwSize, LPCWSTR lpszBuff);

#ifndef _NOINLINES_
#include "splib.inl"
#include "address.inl"
#include "call.inl"
#include "callhub.inl"
#include "device.inl"
#include "line.inl"
#include "phone.inl"
#include "request.inl"
#include "agent.inl"
#endif

#ifndef RC_INVOKED
#pragma pack()		// Revert to original packing
#endif 

#endif // _SPLIB_INC_
