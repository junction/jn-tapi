/******************************************************************************/
//                                                                        
// SPLIB.INL - TAPI Service Provider C++ Library header                     
//                                             
// Copyright (C) 1994-1999 JulMar Entertainment Technology, Inc.
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
// INLINE FUNCTIONS
//                                                           
/******************************************************************************/

#ifndef _SPLIB_INL_INC_
#define _SPLIB_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// GLOBAL FUNCTIONS
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// AddDataBlock
//
// Public function to add a string to a VARSTRING type buffer.
//
TSP_INLINE bool AddDataBlock (LPVOID lpVB, DWORD& dwOffset, DWORD& dwSize, LPCWSTR lpszBuff)
{
	int nLength = lstrlenW(lpszBuff);
	if (nLength) {
		++nLength; // For null-termination
		return AddDataBlock(lpVB, dwOffset, dwSize, lpszBuff, nLength * sizeof(wchar_t));
	}
	return false;

}// AddDataBlock

/******************************************************************************/
//
// CTSPIConnection
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::RemoveRequest
//
// Remove a specific request from our request list.
//
TSP_INLINE void CTSPIConnection::RemoveRequest(CTSPIRequest* pReq)
{
	CEnterCode sLock(this);  // Synch access to object
	_TSP_ASSERTE (std::find(m_lstAsynchRequests.begin(), m_lstAsynchRequests.end(), pReq) != m_lstAsynchRequests.end());
	m_lstAsynchRequests.remove(pReq);

}// CTSPIConnection::RemoveRequest

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::WaitForRequest
//
// Halts the current thread and waits for the current request entry
// to complete or timeout.  
//
// WARNING: After completion of this call, the request most likely
// will be deleted!
//
TSP_INLINE LONG CTSPIConnection::WaitForRequest(CTSPIRequest* pReq, DWORD dwMsecs)
{
    _TSP_ASSERTE(pReq != NULL);
    return pReq->WaitForCompletion(dwMsecs);

}// CTSPIConnection::WaitForRequest

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetDeviceInfo
//
// Return the device owner for this connection
//
TSP_INLINE CTSPIDevice* CTSPIConnection::GetDeviceInfo() const
{
    return m_pDevice;

}// CTSPIConnection::GetDeviceInfo

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetFlags
//
// Return the flags associated with the connection
//
TSP_INLINE DWORD CTSPIConnection::GetFlags() const
{
    return m_dwFlags;

}// CTSPIConnection::GetFlags

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetExtVersion
//
// Return the current extension version in effect
//
TSP_INLINE DWORD CTSPIConnection::GetExtVersion() const
{
	return (m_pExtVerInfo == NULL) ? 0 : 
		m_pExtVerInfo->dwSelectedExtVersion;

}// CTSPIConnection::GetExtVersion

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::HasBeenDeleted
//
// Return whether or not this device has been REMOVED from the system.
//
TSP_INLINE bool CTSPIConnection::HasBeenDeleted() const
{
    return ((GetFlags() & _IsDeleted) == _IsDeleted);

}// CTSPIConnection::HasBeenDeleted

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetName
//
// Retrieve the name of the device
//
TSP_INLINE LPCTSTR CTSPIConnection::GetName() const
{
    return m_strName.c_str();

}// CTSPIConnection::GetName

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetDeviceID
//
// Return the device identifier
//
TSP_INLINE DWORD CTSPIConnection::GetDeviceID() const 
{ 
    return m_dwDeviceID;

}// CTSPIConnection::GetDeviceID

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::SetDeviceID
//
// This function is called in response to a LINE_CREATE message (and
// TAPIs subsequent call to providerCreateLineDevice) to reset our new
// device identifier.
//
TSP_INLINE void CTSPIConnection::SetDeviceID(DWORD dwDeviceId)
{
    _TSP_DTRACEX(TRC_DUMP, _T("Connection <0x%lx> changing device id to %ld\n"), this, dwDeviceId);
    m_dwDeviceID = dwDeviceId;
    GetSP()->MapConnectionToID(dwDeviceId, this);

}// CTSPIConnection::SetDeviceID

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::SetName
//
// Set the connection name.  This is not a required piece of data,
// but if supplied, will be placed into the LINE/PHONE capabilities
// structure.
//
TSP_INLINE void CTSPIConnection::SetName(LPCTSTR lpszName)
{
    m_strName = lpszName;
    
}// CTSPIConnection::SetName

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetNegotiatedVersion
//
// Return the negotiated TAPI version for this connection
//
TSP_INLINE DWORD CTSPIConnection::GetNegotiatedVersion() const
{   
    return m_dwNegotiatedVersion;
    
}// CTSPIConnection::GetNegotiatedVersion

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::IsLineDevice
//
// Return whether this connection is a CTSPILineConnection
//
TSP_INLINE bool CTSPIConnection::IsLineDevice() const
{
	// According to ARM, should not throw exception since casting
	// pointer.  Removed try/catch blocks (v3.043)
	return (dynamic_cast<const CTSPILineConnection*>(this) != NULL);

}// CTSPIConnection::IsLineDevice

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::IsPhoneDevice
//
// Return whether this connection is a CTSPIPhoneConnection
//
TSP_INLINE bool CTSPIConnection::IsPhoneDevice() const
{
	// According to ARM, should not throw exception since casting
	// pointer.  Removed try/catch blocks (v3.043)
	return (dynamic_cast<const CTSPIPhoneConnection*>(this) != NULL);

}// CTSPIConnection::IsPhoneDevice

///////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetRequestCount
//
// Return the total pending requests.
//
TSP_INLINE int CTSPIConnection::GetRequestCount() const
{
	CEnterCode sLock(this);  // Synch access to object
    return m_lstAsynchRequests.size();

}// CTSPIConnection::GetRequestCount

///////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetRequestList
//
// Return a pointer to the request map
//
TSP_INLINE const tsplib_REQMAP* CTSPIConnection::GetRequestList() const
{
	return NULL;

}// CTSPIConnection::GetRequestList

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::AddDeviceClass
//
// Add a DWORD data object to our device class list
//
TSP_INLINE int CTSPIConnection::AddDeviceClass (LPCTSTR pszClass, DWORD dwData)
{
	return AddDeviceClass (pszClass, STRINGFORMAT_BINARY, &dwData, sizeof(DWORD));

}// CTSPIConnection::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::AddDeviceClass
//
// Add a HANDLE and BUFFER to our device class list
//
TSP_INLINE int CTSPIConnection::AddDeviceClass (LPCTSTR pszClass, HANDLE hHandle, LPCTSTR lpszBuff)
{
	return AddDeviceClass(pszClass, STRINGFORMAT_BINARY, const_cast<LPTSTR>(lpszBuff), (lstrlen(lpszBuff)+1) * sizeof(TCHAR), hHandle);

}// CTSPIConnection::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::AddDeviceClass
//
// Add a HANDLE and BUFFER to our device class list
//
TSP_INLINE int CTSPIConnection::AddDeviceClass (LPCTSTR pszClass, HANDLE hHandle, LPVOID lpBuff, DWORD dwSize)
{
	return AddDeviceClass (pszClass, STRINGFORMAT_BINARY, lpBuff, dwSize, hHandle);

}// CTSPIConnection::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::AddDeviceClass
//
// Add a DWORD data object to our device class list
//
TSP_INLINE int CTSPIConnection::AddDeviceClass (LPCTSTR pszClass, DWORD dwFormat, LPVOID lpBuff, DWORD dwSize, HANDLE hHandle)
{
	CEnterCode sLock(this);
	return GetSP()->AddDeviceClassInfo (m_arrDeviceClass, pszClass, dwFormat, lpBuff, dwSize, hHandle);

}// CTSPIConnection::AddDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::RemoveDeviceClass
//
// Remove a device class list object.
//
TSP_INLINE bool CTSPIConnection::RemoveDeviceClass (LPCTSTR pszClass)
{
	CEnterCode sLock(this);
	return GetSP()->RemoveDeviceClassInfo (m_arrDeviceClass, pszClass);	

}// CTSPIConnection::RemoveDeviceClass

////////////////////////////////////////////////////////////////////////////
// CTSPIConnection::GetDeviceClass
//
// Return the device class information for a specified name.
//
TSP_INLINE DEVICECLASSINFO* CTSPIConnection::GetDeviceClass(LPCTSTR pszClass) const
{
	CEnterCode sLock(this);
	return GetSP()->FindDeviceClassInfo(m_arrDeviceClass, pszClass);

}// CTSPIConnection::GetDeviceClass

/******************************************************************************/
//
// CServiceProvider object
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::Instance
//
// Return the global instance of the CServiceProvider class
//
TSP_INLINE CServiceProvider* CServiceProvider::Instance()
{ 
	_TSP_ASSERTE(g_pAppObject != NULL); 
	return g_pAppObject; 

}// CServiceProvider::Instance

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetResourceInstance
//
// Return the HINSTANCE provided to the DLL.
//
TSP_INLINE HINSTANCE CServiceProvider::GetResourceInstance() const
{
	if (m_hInstance == NULL) {
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(&g_pAppObject, &mbi, sizeof(mbi));
		m_hInstance = reinterpret_cast<HINSTANCE>(mbi.AllocationBase);
	}
	return m_hInstance;

}// CServiceProvider::GetResourceInstance

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetSupportedVersion
//
// Return the highest level of TAPI support this service provider
// has decided to conform to.
//
TSP_INLINE DWORD CServiceProvider::GetSupportedVersion() const
{                               
    return m_dwTapiVerSupported;

}// CServiceProvider::GetSupportedVersion

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetShutdownEvent
//
// Returns the handle to the event which is signalled when the TSP 
// is shutting down
//
TSP_INLINE HANDLE CServiceProvider::GetShutdownEvent() const
{
	return m_hevtShutdown;

}// CServiceProvider::GetShutdownEvent

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetSystemVersion
//
// Return the version which TAPI.DLL is at on this computer.
//
TSP_INLINE DWORD CServiceProvider::GetSystemVersion() const
{                                     
    return m_dwTAPIVersionFound;

}// CServiceProvider::GetSystemVersion

/////////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetCurrentLocation
//
// Return the current location set by TAPI and adjusted by the 
// dial property page.
//
TSP_INLINE DWORD CServiceProvider::GetCurrentLocation() const
{
	if (m_pcurrLocation != NULL)
		return m_pcurrLocation->dwCurrentLocation;
	return 0;
    
}// CServiceProvider::GetCurrentLocation

/////////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetUIManager
//
// Return string name (passed to constructor)
//
TSP_INLINE LPCTSTR CServiceProvider::GetUIManager() const
{                                              
    return m_pszUIName;

}// CServiceProvider::GetUIManager

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::CreateDeviceObject
//
// This method creates a device object
//
TSP_INLINE CTSPIDevice* CServiceProvider::CreateDeviceObject()
{
	return static_cast<CTSPIDevice*>(TDynamicCreate::CreateObject(m_pObjects[2]));

}// CServiceProvider::CreateDeviceObject

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::CreateLineObject
//
// This method creates a line object
//
TSP_INLINE CTSPILineConnection* CServiceProvider::CreateLineObject()
{
	return static_cast<CTSPILineConnection*>(TDynamicCreate::CreateObject(m_pObjects[0]));

}// CServiceProvider::CreateLineObject

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::CreateAddressObject
//
// This method creates an address object
//
TSP_INLINE CTSPIAddressInfo* CServiceProvider::CreateAddressObject()
{
	return static_cast<CTSPIAddressInfo*>(TDynamicCreate::CreateObject(m_pObjects[4]));

}// CServiceProvider::CreateAddressObject

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::CreateCallObject
//
// This method creates a call object
//
TSP_INLINE CTSPICallAppearance* CServiceProvider::CreateCallObject()
{
	return static_cast<CTSPICallAppearance*>(TDynamicCreate::CreateObject(m_pObjects[3]));

}// CServiceProvider::CreateCallObject

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::CreateConferenceCallObject
//
// This method creates a call object
//
TSP_INLINE CTSPIConferenceCall* CServiceProvider::CreateConferenceCallObject()
{
	return static_cast<CTSPIConferenceCall*>(TDynamicCreate::CreateObject(m_pObjects[5]));

}// CServiceProvider::CreateConferenceCallObject

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::CreatePhoneObject
//
// This method creates a phone object
//
TSP_INLINE CTSPIPhoneConnection* CServiceProvider::CreatePhoneObject()
{
	return static_cast<CTSPIPhoneConnection*>(TDynamicCreate::CreateObject(m_pObjects[1]));

}// CServiceProvider::CreatePhoneObject

/////////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetProviderInfo
//
// Return the provider information
//
TSP_INLINE LPCTSTR CServiceProvider::GetProviderInfo() const
{
    return m_pszProviderInfo;

}// CServiceProvider::GetProviderInfo

/////////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetLineCreateProc
//
// Return the line creation procedure if available.
//
// WARNING: This may be NULL!
//
TSP_INLINE LINEEVENT CServiceProvider::GetLineCreateProc() const
{
    return m_lpfnLineCreateProc;

}// CServiceProvider::GetLineCreateProc

/////////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetPhoneCreateProc
//
// Return the phone creation procedure if available.
//
// WARNING: This may be NULL!
//
TSP_INLINE PHONEEVENT CServiceProvider::GetPhoneCreateProc() const
{
    return m_lpfnPhoneCreateProc;

}// CServiceProvider::GetPhoneCreateProc

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetDeviceByIndex
//
// Return a device object based on an index into our device
// map.
//
TSP_INLINE CTSPIDevice* CServiceProvider::GetDeviceByIndex(unsigned int i) const
{
	return (GetDeviceCount() > i) ? m_arrDevices[i] : NULL;

}// CServiceProvider::GetDeviceByIndex

/////////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetConnInfoFromLineDeviceID
//
// Return the CTSPIConnection object from a line device id.
//
TSP_INLINE CTSPILineConnection* CServiceProvider::GetConnInfoFromLineDeviceID (DWORD dwDeviceID)
{
	TConnectionMap::iterator theIterator = m_mapLineConn.find(dwDeviceID);
	return (theIterator != m_mapLineConn.end()) ?
		dynamic_cast<CTSPILineConnection*>((*theIterator).second) : NULL;

}// CServiceProvider::GetConnInfoFromLineDeviceID

/////////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetConnInfoFromPhoneDeviceID
//
// Return the CTSPIConnection object from a phone device id.
//
TSP_INLINE CTSPIPhoneConnection* CServiceProvider::GetConnInfoFromPhoneDeviceID (DWORD dwDeviceID) 
{
	TConnectionMap::iterator theIterator = m_mapPhoneConn.find(dwDeviceID);
	return (theIterator != m_mapPhoneConn.end()) ?
		dynamic_cast<CTSPIPhoneConnection*>((*theIterator).second) : NULL;

}// CServiceProvider::GetConnInfoFromPhoneDeviceID

///////////////////////////////////////////////////////////////////////////
// CServiceProvider::GetDeviceCount
//
// Return the count of devices.
//
TSP_INLINE DWORD CServiceProvider::GetDeviceCount() const
{
    return m_arrDevices.size();

}// CServiceProvider::GetDeviceCount

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::ProcessCallParameters
//
// This function is used to determine if the passed call parameters
// are valid for the device they are being used for.
//
TSP_INLINE LONG CServiceProvider::ProcessCallParameters (CTSPIAddressInfo* pAddr, LPLINECALLPARAMS lpCallParams)
{
    return ProcessCallParameters (pAddr->GetLineOwner(), lpCallParams);

}// CServiceProvider::ProcessCallParameters

////////////////////////////////////////////////////////////////////////////
// CServiceProvider::ProcessCallParameters
//
// This function is used to determine if the passed call parameters
// are valid for the device they are being used for.
//
TSP_INLINE LONG CServiceProvider::ProcessCallParameters (CTSPICallAppearance* pCall, LPLINECALLPARAMS lpCallParams)
{   
    return ProcessCallParameters (pCall->GetLineOwner(), lpCallParams);

}// CServiceProvider::ProcessCallParameters

/******************************************************************************/
//
// CTSPIBaseObject
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::GetItemData
//
// Return the DWORD item data setup by the developer
//
TSP_INLINE DWORD CTSPIBaseObject::GetItemData() const
{
	CEnterCode sLock(this);
	return m_dwItemData;

}// CTSPIBaseObject::GetItemData

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::GetItemDataPtr
//
// Return the DWORD item data setup by the developer
//
TSP_INLINE void* CTSPIBaseObject::GetItemDataPtr() const
{
	CEnterCode sLock(this);
	return reinterpret_cast<void*>(m_dwItemData);

}// CTSPIBaseObject::GetItemDataPtr

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::SetItemData
//
// Set the DWORD item data 
//
TSP_INLINE void CTSPIBaseObject::SetItemData(DWORD dwItem)
{
	CEnterCode sLock(this);
	m_dwItemData = dwItem;

}// CTSPIBaseObject::SetItemData

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::AddRef
//
// Increments the reference count on the object
//
TSP_INLINE void CTSPIBaseObject::AddRef() const
{
	InterlockedIncrement(&m_lRefCount);

}// CTSPIBaseObject::AddRef

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::DecRef
//
// Decrements the reference count on the object
//
TSP_INLINE void CTSPIBaseObject::DecRef() const
{
	if (InterlockedDecrement(&m_lRefCount) == 0)
	{
		m_lRefCount = -32767;	// Stop re-entry
		const_cast<CTSPIBaseObject*>(this)->DestroyObject();
	}

}// CTSPIBaseObject::DecRef

#ifdef _DEBUG
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::GetRefCount
//
// Access method for the reference count -- only used for debug assertions!
//
TSP_INLINE long CTSPIBaseObject::GetRefCount() const
{
	return m_lRefCount;

}// CTSPIBaseObject::GetRefCount
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::CTSPIBaseObject
//
// Constructor -- allocates the critical section object
//
TSP_INLINE CTSPIBaseObject::CTSPIBaseObject() : m_dwItemData(0), m_lRefCount(1)
{
}// CTSPIBaseObject::CTSPIBaseObject

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::CTSPIBaseObject
//
// Copy Constructor -- allocates the critical section object
//
TSP_INLINE CTSPIBaseObject::CTSPIBaseObject(const CTSPIBaseObject& src) : m_dwItemData(src.m_dwItemData), m_lRefCount(1)
{
}// CTSPIBaseObject::CTSPIBaseObject

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::GetSyncObject
//
// Return the synchronization object
//
TSP_INLINE CIntCriticalSection* CTSPIBaseObject::GetSyncObject() const
{
	return &m_csLock;

}// CTSPIBaseObject::GetSyncObject

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CTSPIBaseObject::SetItemDataPtr
//
// Set the DWORD item data
//
TSP_INLINE void CTSPIBaseObject::SetItemDataPtr(void* pItem)
{
	CEnterCode sLock(this);
	m_dwItemData = reinterpret_cast<DWORD>(pItem);

}// CTSPIBaseObject::SetItemDataPtr

/******************************************************************************/
//
// TSPIFORWARDINFO
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// TSPIFORWARDINFO::TSPIFORWARDINFO
//
// Constructor for the TSPIFORWARDINFO object request
//
TSP_INLINE TSPIFORWARDINFO::TSPIFORWARDINFO() : 
	dwForwardMode(0), dwDestCountry(0), lRefCount(0)
{                                   
}// TSPIFORWARDINFO::TSPIFORWARDINFO

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// TSPIFORWARDINFO::IncUsage
//
// Increment the reference counter
//
TSP_INLINE void TSPIFORWARDINFO::IncUsage()
{                               
    InterlockedIncrement(&lRefCount);
    
}// TSPIFORWARDINFO::IncUsage

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// TSPIFORWARDINFO::DecUsage
//
// Increment the reference counter
//
TSP_INLINE void TSPIFORWARDINFO::DecUsage()
{                               
    if (InterlockedDecrement(&lRefCount) <= 0)
        delete this;
    
}// TSPIFORWARDINFO::DecUsage

/******************************************************************************/
//
// TSPIDIGITGATHER
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// TSPIDIGITGATHER::TSPIDIGITGATHER
//
// Constructor for the TSPIDIGITGATHER object request
//
TSP_INLINE TSPIDIGITGATHER::TSPIDIGITGATHER() : 
	dwEndToEndID(0), dwDigitModes(0), lpBuffer(NULL), dwSize(0), dwCount(0),
	dwFirstDigitTimeout(0), dwInterDigitTimeout(0),
	dwLastTime(0)
{                                   
}// TSPIDIGITGATHER::TSPIDIGITGATHER

/******************************************************************************/
//
// TSPITONEMONITOR
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// TSPITONEMONITOR::TSPITONEMONITOR
//
// Constructor for the tone monitor request object
//
TSP_INLINE TSPITONEMONITOR::TSPITONEMONITOR() :
	dwToneListID(0)
{
}// TSPITONEMONITOR::TSPITONEMONITOR

/******************************************************************************/
//
// TSPIMEDIACONTROL
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// TSPIMEDIACONTROL::TSPIMEDIACONTROL
//
// Constructor for the TSPIMEDIACONTROL object request
//
TSP_INLINE TSPIMEDIACONTROL::TSPIMEDIACONTROL() : lRefCount(0)
{                                     
}// TSPIMEDIACONTROL::TSPIMEDIACONTROL

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// TSPIMEDIACONTROL::IncUsage
//
// Increment our usage count
//
TSP_INLINE void TSPIMEDIACONTROL::IncUsage()
{                             
    InterlockedIncrement(&lRefCount);

}// TSPIMEDIACONTROL::IncUsage

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// TSPIMEDIACONTROL::DecUsage
//
// Decrement our usage count and delete when it hits zero
//
// WARNING: THIS OBJECT MUST BE ALLOCATED WITH "new"!!!
//
TSP_INLINE void TSPIMEDIACONTROL::DecUsage()
{                             
    if (InterlockedDecrement(&lRefCount) <= 0)
        delete this;

}// TSPIMEDIACONTROL::DecUsage

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CALLIDENTIFIER::operator=
//
// Copy operator for CALLERID block
//
TSP_INLINE CALLIDENTIFIER& CALLIDENTIFIER::operator=(const CALLIDENTIFIER& src)
{
	if (this != &src)
	{
		strPartyId = src.strPartyId;
		strPartyName = src.strPartyName;
	}
	return *this;

}// CALLIDENTIFIER::operator=

/******************************************************************************/
//
// CMSPDrivder
//
/******************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CMSPDriver::CMSPDriver
//
// Constructor for the MSP driver object
//
TSP_INLINE CMSPDriver::CMSPDriver(CTSPIAddressInfo* pOwner, HTAPIMSPLINE htHandle) : 
		m_pAddressOwner(pOwner), m_htHandle(htHandle)
{
}// CMSPDriver::CMSPDriver

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CMSPDriver::~CMSPDriver
//
// Destructor for the MSP driver object
//
TSP_INLINE CMSPDriver::~CMSPDriver()
{
	// TODO: Notify MSP stream that the connection is gone?

}// CMSPDriver::CMSPDriver

////////////////////////////////////////////////////////////////////////////
// CMSPDriver::GetAddressOwner
//
// Returns the line owner object for this instance
//
TSP_INLINE CTSPIAddressInfo* CMSPDriver::GetAddressOwner() const
{
	_TSP_ASSERT (m_pAddressOwner != NULL);
	return m_pAddressOwner;

}// CMSPDriver::GetAddressOwner

////////////////////////////////////////////////////////////////////////////
// CMSPDriver::GetLineOwner
//
// Returns the line owner object for this instance
//
TSP_INLINE CTSPILineConnection* CMSPDriver::GetLineOwner() const
{
	_TSP_ASSERT (m_pAddressOwner != NULL);
	return m_pAddressOwner->GetLineOwner();

}// CMSPDriver::GetLineOwner

////////////////////////////////////////////////////////////////////////////
// CMSPDriver::SendData
//
// Sends an associated MSP a piece of arbitrary information
//
TSP_INLINE void CMSPDriver::SendData(CTSPICallAppearance* pCall, LPVOID lpData, DWORD dwSize)
{
	GetLineOwner()->Send_TAPI_Event(pCall, LINE_SENDMSPDATA, (LPARAM)GetTapiHandle(), (LPARAM)lpData, dwSize);

}// CMSPDriver::SendData

////////////////////////////////////////////////////////////////////////////
// CMSPDriver::GetTapiHandle
//
// Returns the TAPI opaque handle for this device
//
TSP_INLINE HTAPIMSPLINE CMSPDriver::GetTapiHandle() const
{
	_TSP_ASSERT(m_htHandle != NULL);
	return m_htHandle;

}// CMSPDriver::GetTapiHandle

#endif // _SPLIB_INL_INC_
