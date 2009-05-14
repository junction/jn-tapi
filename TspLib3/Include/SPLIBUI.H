/******************************************************************************/
//                                                                        
// SPLIBUI.H - TAPI Service Provider User Interface C++ Library header                     
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
/******************************************************************************/

#ifndef _SPLIBUI_INC_
#define _SPLIBUI_INC_

// Force TAPI 3 support to be added into the code.  This does not
// force the TSP to negotiate to TAPI3, it simply ensures our data
// structures will be laid out in a consistent fashion across the 
// library, UI and TSP projects.
#ifndef _SPLIB_INC_
#if defined(TAPI_CURRENT_VERSION) || defined(TAPI_H) || defined(TSPI_H)
#error "You must include splibui.h before TAPI.H or TSPI.H"
#endif
#endif

// Include TAPI headers
#define TAPI_CURRENT_VERSION 0x00030000
#if !defined(TAPI_H)
#include <tapi.h>
#endif
#if !defined(TSPI_H)
#include <tspi.h>
#endif

#ifdef _UNICODE
#ifdef _DEBUG
#pragma comment(lib, "spluiud.lib")
#else
#pragma comment(lib, "spluiu.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "spluid.lib")
#else
#pragma comment(lib, "splui.lib")
#endif
#endif

#include <afxtempl.h>			// Make sure MFC templates are included

#ifndef _SPLIB_INC_
#define CServiceProvider CServiceProviderUI
#pragma warning(push,3)
#include <string>				// STL string support
#include <tchar.h>				// Unicode support
#include <wtypes.h>				// LPOLESTR support
#include <oleauto.h>			// SysAllocString support
#pragma warning(pop)
#if defined(_UNICODE) || defined(UNICODE)
typedef std::wstring TString;
#else
typedef std::string TString;
#endif

// This key is used during TSPI_providerGenericDialogData
// to convert line/phone identifiers into the associated provider
// identifier.  It is also specified in SPLIB.h
#define GDD_LINEPHONETOPROVIDER		(0xab110301)
#define GDD_LINEPHONETOPROVIDEROK	(0xab110302)
#define GDD_LINEPHONETOPERMANENT    (0xab110303)
#define GDD_LINEPHONETOPERMANENTOK  (0xab110304)
#endif

// Class structures
class CServiceProviderUI;
	class CTSPUIDevice;
		class CTSPUIPhoneConnection;
		class CTSPUILineConnection;
			class CTSPUIAddressInfo;
			class TStream;

namespace tsplibui {
/******************************************************************************/
//
// TAgentGroup
//
// This structure describes a single agent group within our provider
//
/******************************************************************************/
typedef struct
{
	struct {
		DWORD dwGroupID1;		// Group identifier
		DWORD dwGroupID2;
		DWORD dwGroupID3;
		DWORD dwGroupID4;
	} GroupID;
	CString strName;			// Name of group
} TAgentGroup;

/******************************************************************************/
//
// TAgentActivity
//
// This structure describes a single agent activity
//
/******************************************************************************/
typedef struct
{
	DWORD dwID;					// Activity identifier
	CString strName;			// Name
} TAgentActivity;
};

/******************************************************************************/
//
// CServiceProviderUI class
//
// This class is used to field all the calls for the service provider.
// It is based on the CWinApp class and provides the hookups for the
// Microsoft Foundation classes to properly work.
//
// This class manages multiple provider devices, and can have multiple
// DLL instances each with a seperate permanent provider id (PPID).
//
/******************************************************************************/
class CServiceProviderUI : public CWinApp
{
// Class data
protected:
	CMap<HTAPIDIALOGINSTANCE, HTAPIDIALOGINSTANCE, CWnd*, CWnd*> m_mapUIDlgs;
	LPCTSTR m_pszProviderInfo;
	CArray<CTSPUIDevice*> m_arrDevices;
	CRuntimeClass* m_pObjects[4];
	TUISPIDLLCALLBACK m_lpfnCallback;
	DWORD m_dwObjectID;
	DWORD m_dwObjectType;

// Constructor
public:
	CServiceProviderUI(LPCTSTR pszProviderInfo);
	virtual ~CServiceProviderUI();

// Class functions
public:
    // Return provider information string (passed to CServiceProviderUI constructor).
    LPCTSTR GetProviderInfo() const;

	// UIMap management for spontaneous dialogs.
	CWnd* FindUIDialogFromInstance(HTAPIDIALOGINSTANCE htDlgInst);
	void AddUIDialog(HTAPIDIALOGINSTANCE htDlgInst, CWnd* pwnd);
	void RemoveUIDialog(CWnd* pwnd);

	// Registry storage manipulation methods which can be used to store information
	// about devices in the provider.
	CString ReadProfileString (DWORD dwDeviceID, LPCTSTR pszEntry, LPCTSTR pszDefault = _T(""));
	DWORD ReadProfileDWord (DWORD dwDeviceID, LPCTSTR pszEntry, DWORD dwDefault = 0L);
	bool WriteProfileString (DWORD dwDeviceID, LPCTSTR pszEntry, LPCTSTR pszValue);
	bool WriteProfileDWord (DWORD dwDeviceID, LPCTSTR pszEntry, DWORD dwValue);
	bool DeleteProfile (DWORD dwDeviceID, LPCTSTR pszEntry=NULL);
	bool RenameProfile (DWORD dwOldDevice, DWORD dwNewDevice);

	// Functions to synchronize objects to the registry
	void ResetConfiguration();
	virtual void LoadObjects();
	virtual void SaveObjects();

	// Read the count of devices (normally just 1)
	unsigned int GetDeviceCount() const;
	unsigned int AddDevice(DWORD dwProviderID);
	unsigned int AddDevice(CTSPUIDevice* pDevice);
	CTSPUIDevice* GetDeviceByIndex(unsigned int i) const;
	CTSPUIDevice* GetDevice(DWORD dwPermanentProviderID) const;

	// Function to call the TSP using the GenericDialogData thunk
	LONG InvokeTSP(LPVOID lpParams, DWORD dwSize);
	LONG InvokeTSP(HTAPIDIALOGINSTANCE htDlgInst, LPVOID lpParams, DWORD dwSize);

	// Functions to help translate TAPI device independant numbers to provider ids.
	LONG IsProviderInstalled(LPCTSTR pszProviderName, LPDWORD lpdwPPid) const;
	LONG GetProviderIDFromLineDeviceID(DWORD dwDeviceID, LPDWORD lpdwPPid);
	LONG GetProviderIDFromPhoneDeviceID(DWORD dwDeviceID, LPDWORD lpdwPPid);
	LONG GetPermanentIDFromLineDeviceID(DWORD dwDeviceID, LPDWORD lpdwPPid);
	LONG GetPermanentIDFromPhoneDeviceID(DWORD dwDeviceID, LPDWORD lpdwPPid);

	// Overrides from CWinApp
	virtual BOOL InitInstance();

// Functions only callable from this object
protected:
    // Set the C++ objects to use for each basic telephony object in the system.  This must be
	// done during the constructor of the service provider.
    void SetRuntimeObjects(CRuntimeClass* pDevObject, CRuntimeClass* pLineObj, CRuntimeClass* pAddrObj=NULL, CRuntimeClass* pPhoneObj=NULL);

// Internal functions
private:
	// Used to delete registry tree during providerRemove when running under Windows NT.
	bool IntRegDeleteKey (HKEY hKeyTelephony, LPCTSTR pszMainDir);

public:
	friend class CTSPUIDevice;
	friend class CTSPUILineConnection;
	friend class CTSPUIPhoneConnection;
	void _SetIntCallback(DWORD dwType, DWORD dwID, TUISPIDLLCALLBACK pfnCallback);

// TAPI callbacks
public:
	// This method is invoked when the user selects our ServiceProvider
	// icon in the control panel.  It should invoke the configuration dialog
	// which must be provided by the derived class.
	virtual LONG providerConfig(DWORD dwPPID, CWnd* pwndOwner);

	// This method is invoked when the TSP is to be installed via the
	// TAPI install code.  It should insure that all the correct files
	// are there, and write out the initial .INI settings.
	virtual LONG providerInstall(DWORD dwPermanentProviderID, CWnd* pwndOwner);

	// This method is invoked when the TSP is being removed from the
	// system.  It should remove all its files and .INI settings.
	virtual LONG providerRemove(DWORD dwPermanentProviderID, CWnd* pwndOwner);

	// This method is called for the UI DLL when the provider needs a dialog displayed.
	virtual LONG providerGenericDialog(HTAPIDIALOGINSTANCE htDlgInst, LPVOID lpParams, DWORD dwSize, HANDLE hEvent);

	// This method is called for the UI DLL when the provider sends information.
	virtual LONG providerGenericDialogData(HTAPIDIALOGINSTANCE htDlgInst, LPVOID lpParams, DWORD dwSize);

	// This method is called to display the line configuration dialog
	// when the user requests it through either the TAPI api or the control
	// panel applet.
	virtual LONG lineConfigDialog(DWORD dwDeviceID, CWnd* pwndOwner, CString& strDeviceClass);

	// This method is called to display the line configuration dialog with
	// a set of known parameters rather than the set currently in use.
	virtual LONG lineConfigDialogEdit(DWORD dwDeviceID, CWnd* pwndOwner, CString& strDeviceClass, const LPVOID lpDeviceConfigIn, DWORD dwSize, LPVARSTRING lpDeviceConfigOut);

	// This method invokes the parameter configuration dialog for the
	// phone device.
	virtual LONG phoneConfigDialog(DWORD dwDeviceID, CWnd* pwndOwner, CString& strDevClass);
};

/******************************************************************************/
//
// _tspuiBaseObject
//
// This class is used as a basis for all our serializable objects
// so that each may serialize to a data stream.
//
/******************************************************************************/
class _tspuiBaseObject : public CObject
{
// Default Constructor
public:
	_tspuiBaseObject() {}
	DECLARE_DYNCREATE(_tspuiBaseObject)

protected:
	virtual TStream& write( TStream& ostm ) const { return ostm; }
	virtual TStream& read( TStream& istm ) { return istm; }
	friend TStream& operator<<(TStream& ostm, const _tspuiBaseObject& bobj);
	friend TStream& operator>>(TStream& istm, _tspuiBaseObject& bobj);

// Disable copy constructor and assignment
private:
	_tspuiBaseObject(const _tspuiBaseObject&);
};

/******************************************************************************/
//
// CTSPUIDevice class
//
// This class describes a single device for the TSP. There will be at least
// one of these objects defined.
//
// Note this only stores basic information about the device.  If you choose
// to use these objects for serialization, you should override this object
// and supply new "read" and "write" functions.
//
/******************************************************************************/
class CTSPUIDevice : public _tspuiBaseObject
{
// Class data 
protected:
	DWORD m_dwPermProviderID;		// Permanent ProviderID for this object
	CPtrArray m_arrGroups;			// Array of agent groups
	CPtrArray m_arrActivity;		// Array of agent activities
	CArray<CTSPUILineConnection*> m_arrLines;	// Array of lines
	CArray<CTSPUIPhoneConnection*> m_arrPhones;	// Array of phones

// Constructor
public:
	CTSPUIDevice(DWORD dwPermProviderID);
	virtual ~CTSPUIDevice();
protected:
	CTSPUIDevice();
	DECLARE_DYNCREATE(CTSPUIDevice)

// Access methods
public:
	// Delete all the lines/phone/agent information for this device.
	void ResetConfiguration();

	DWORD GetProviderID() const;

	// v3.0 Extensions for new objects
	unsigned int GetLineCount() const;
    CTSPUILineConnection* GetLineConnectionInfo(unsigned int nIndex) const;
	CTSPUILineConnection* FindLineConnectionByPermanentID(DWORD dwPermID);
	unsigned int AddLine(CTSPUILineConnection* pLine);
	void RemoveLine(unsigned int iLine);
	void RemoveLine(CTSPUILineConnection* pLine);

	// Phone management
    unsigned int GetPhoneCount() const;
    CTSPUIPhoneConnection* GetPhoneConnectionInfo(unsigned int nIndex) const;
	CTSPUIPhoneConnection* FindPhoneConnectionByPermanentID(DWORD dwConnID);
	unsigned int AddPhone(CTSPUIPhoneConnection* pPhone);
	void RemovePhone(unsigned int iPhone);
	void RemovePhone(CTSPUIPhoneConnection* pPhone);

	// Agent activity management
	unsigned int AddAgentActivity (DWORD dwID, LPCTSTR pszName);
	unsigned int AddAgentActivity (tsplibui::TAgentActivity* pAct);
	unsigned int GetAgentActivityCount() const;
	void RemoveAgentActivity(DWORD dwID);
	tsplibui::TAgentActivity* GetAgentActivity(unsigned int iPos) const;
	bool DoesAgentActivityExist(DWORD dwActivity);
	CString GetAgentActivityById(DWORD dwID);

	// Agent group management
	int AddAgentGroup (LPCTSTR pszName, DWORD dwGroupID1, DWORD dwGroupID2=0, DWORD dwGroupID3=0, DWORD dwGroupID4=0);
	unsigned int GetAgentGroupCount() const;
	void RemoveAgentGroup(DWORD dwGroupID1, DWORD dwGroupID2=0, DWORD dwGroupID3=0, DWORD dwGroupID4=0);
	tsplibui::TAgentGroup* GetAgentGroup(unsigned int iPos) const;
	bool DoesAgentGroupExist(DWORD dwGroupID1, DWORD dwGroupID2=0, DWORD dwGroupID3=0, DWORD dwGroupID4=0);
	CString GetAgentGroupById(DWORD dwGroupID1, DWORD dwGroupID2=0, DWORD dwGroupID3=0, DWORD dwGroupID4=0);

// Methods for just the derived device
protected:
	friend class CServiceProviderUI;
	friend class CTSPUILineConnection;
	friend class CTSPUIPhoneConnection;

// Internal methods
private:
	void LoadFromStream(TStream& istm);
	void SaveIntoStream(TStream& istm);
	tsplibui::TAgentActivity* ReadAgentActivity(TStream& istm);
	void WriteAgentActivity(const tsplibui::TAgentActivity* pAct, TStream& ostm) const;
	tsplibui::TAgentGroup* ReadAgentGroup(TStream& istm);
	void WriteAgentGroup(const tsplibui::TAgentGroup* pGroup, TStream& ostm) const;

// Serialization support
protected:
	virtual TStream* AllocStream();
	virtual TStream& write(TStream& ostm) const;
	virtual TStream& read(TStream& istm);
};

/******************************************************************************/
//
// CTSPUILineConnection class
//
// This class describes a line connection for TAPI.  It is used to 
// store and retrieve line information for the UI to configure.
//
// Note this only stores basic information about the line.  If you choose
// to use these objects for serialization, you should override this object
// and supply new "read" and "write" functions.
//
/******************************************************************************/
class CTSPUILineConnection : public _tspuiBaseObject
{
// Class data
public:
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
	CTSPUIDevice* m_pDevice;	// Device owner
	DWORD m_dwDeviceID;			// Permanent Device ID for this line
	CString m_strName;			// Name of the line device
	int m_iType;				// Type of stored line device (used to create line)
	bool m_fSupportsAgents;		// TRUE if this line has agent features
	CArray<CTSPUIAddressInfo*> m_arrAddress; // Array of addresses
	DWORD m_dwPhoneID;			// Device ID of the connected phone
	GUID m_guidMSP;				// MSP guid
	GUID m_clsidProtocol;		// CLSID of the protocol for this line

// Constructor
public:
	CTSPUILineConnection(DWORD dwDeviceID, int iType, LPCTSTR pszName);
	const CTSPUILineConnection& operator=(const CTSPUILineConnection& lc);
	virtual ~CTSPUILineConnection();

// Serializable constructor
protected:
	CTSPUILineConnection();
	DECLARE_DYNCREATE(CTSPUILineConnection)

// Class members
public:
	int GetLineType() const;
	DWORD GetPermanentDeviceID() const;
	LPCTSTR GetName() const;
	void SetLineType(int iType);
	void SetPermanentDeviceID(DWORD dwID);
	void SetName(LPCTSTR pszName);

	void SetMSPGUID(const GUID& guid);
	void SetProtocolCLSID(const GUID& clsid);

	bool SupportsAgents() const;
	void EnableAgentSupport(bool fAgentSupport=true);

	void SetAssociatedPhone(DWORD dwPhoneID);
	CTSPUIPhoneConnection* GetAssociatedPhone() const;

	CTSPUIDevice* GetDeviceInfo() const;
		
// Address management
public:
    unsigned int GetAddressCount() const;
    CTSPUIAddressInfo* GetAddress (unsigned int iAddressID) const;
    CTSPUIAddressInfo* GetAddress (LPCTSTR lpszDialableAddr) const;
	unsigned int AddAddress(CTSPUIAddressInfo* pAddr);
    unsigned int CreateAddress (LPCTSTR lpszDialableAddr, LPCTSTR lpszAddrName, bool fAllowIncoming, bool fAllowOutgoing,
                       DWORD dwAvailMediaModes, DWORD dwBearerMode, DWORD dwMinDataRate, DWORD dwMaxDataRate, LPLINEDIALPARAMS lpDialParams,
                       DWORD dwMaxNumActiveCalls, DWORD dwMaxNumOnHoldCalls, DWORD dwMaxNumOnHoldPendCalls, 
					   DWORD dwMaxNumConference, DWORD dwMaxNumTransConf, DWORD dwAddressType=LINEADDRESSTYPE_PHONENUMBER);
	void RemoveAddress(unsigned int iAddressID);

// Serialization support
protected:
	friend class CTSPUIDevice;
	virtual TStream& write(TStream& ostm) const;
	virtual TStream& read(TStream& istm);
};

namespace tsplibui {
/******************************************************************************/
//
// CPhoneButtonInfo
//
// This class contains all the elements from the PHONEBUTTONINFO
// structure, but allows the object to be stored in an object list
// and serialized.
//
// INTERNAL STRUCTURE
//
/******************************************************************************/
class CPhoneButtonInfo : public _tspuiBaseObject
{
// Class data
protected:
    DWORD m_dwButtonMode;			// Button mode (PHONEBUTTONMODE_xxx)
    DWORD m_dwButtonFunction;		// Button function (PHONEBUTTONFUNCTION_xxx)
    DWORD m_dwLampModes;			// Available lamp modes
    CString m_strButtonDescription;	// Button description

// Constructor
public:      
	CPhoneButtonInfo() : m_dwButtonMode(0), m_dwButtonFunction(0), m_dwLampModes(0) {}
    CPhoneButtonInfo(DWORD dwButtonFunction, DWORD dwButtonMode, DWORD dwLampMode, LPCTSTR lpszDesc) :
		m_dwButtonMode(dwButtonMode), m_dwButtonFunction(dwButtonFunction), 
		m_dwLampModes(dwLampMode), m_strButtonDescription(lpszDesc) {}

// Serialize support
protected:
	friend class CTSPUIPhoneConnection;
	virtual TStream& write(TStream& ostm) const;
	virtual TStream& read(TStream& istm);
};

/******************************************************************************/
//
// CPhoneHSInfo
//
// This class contains information about a single hook switch device.
//
// INTERNAL STRUCTURE
//
/******************************************************************************/
class CPhoneHSInfo : public _tspuiBaseObject
{
// Class data
protected:
	DWORD m_dwDevice;		// Hookswitch device type
	DWORD m_dwModes;		// Hookswitch modes supported on device
	DWORD m_dwVolume;		// Default volume level
	DWORD m_dwGain;			// Default gain level
	DWORD m_dwSetModes;		// Modes which may be set
	DWORD m_dwMonModes;		// Modes which may be monitored
	bool m_fSupportsVolChange;
	bool m_fSupportsGainChange;

// Constructor
public:
	CPhoneHSInfo() : m_dwDevice(0), m_dwModes(0), m_dwVolume(0), m_dwGain(0), m_dwSetModes(0), m_dwMonModes(0), m_fSupportsVolChange(false), m_fSupportsGainChange(false) {}
	CPhoneHSInfo(DWORD dwHookSwitchDev, DWORD dwAvailModes, DWORD dwVolume, DWORD dwGain, DWORD dwSettableModes, DWORD dwMonitoredModes, bool fSupportsVolChange=false, bool fSupportsGainChange=false) :
		m_dwDevice(dwHookSwitchDev), m_dwModes(dwAvailModes), m_dwVolume(dwVolume),
		m_dwGain(dwGain), m_dwSetModes(dwSettableModes), m_dwMonModes(dwMonitoredModes),
		m_fSupportsVolChange(fSupportsVolChange), m_fSupportsGainChange(fSupportsGainChange) {}

// Serialize support
protected:
	friend class CTSPUIPhoneConnection;
	virtual TStream& write(TStream& ostm) const;
	virtual TStream& read(TStream& istm);
};
};

/******************************************************************************/
//
// CTSPUIPhoneConnection class
//
// This class describes a phone connection for TAPI.  It is used to 
// store and retrieve phone information for the UI to configure.
//
// Note this only stores basic information about the phone.  If you choose
// to use these objects for serialization, you should override this object
// and supply new "read" and "write" functions.
//
/******************************************************************************/
class CTSPUIPhoneConnection : public _tspuiBaseObject
{
// Class data
protected:
	CTSPUIDevice* m_pDevice;	// Device owner
	DWORD m_dwDeviceID;			// Permanent Device ID for this line
	DWORD m_dwLineID;			// Permanent Device ID of the line for this phone.
	CString m_strName;			// Name of the phone device
	SIZE m_sizDisplay;			// Size of the display
	char m_chDisplay;			// Separator for display
	CArray<tsplibui::CPhoneButtonInfo*> m_arrButtons;
	CArray<tsplibui::CPhoneHSInfo*> m_arrHookswitch;
	CDWordArray m_arrUploadBuffers;
	CDWordArray m_arrDownloadBuffers;

// Public Constructor
public:
	CTSPUIPhoneConnection(DWORD dwDeviceID, LPCTSTR pszName);
	const CTSPUIPhoneConnection& operator=(const CTSPUIPhoneConnection& lc);
	virtual ~CTSPUIPhoneConnection();

// Serializable constructor
protected:
	CTSPUIPhoneConnection();
	DECLARE_DYNCREATE(CTSPUIPhoneConnection)

// Class members
public:
	DWORD GetPermanentDeviceID() const;
	LPCTSTR GetName() const;
	void SetPermanentDeviceID(DWORD dwID);
	void SetName(LPCTSTR pszName);
	void SetAssociatedLine(DWORD dwLineID);
	CTSPUILineConnection* GetAssociatedLine() const;
	CTSPUIDevice* GetDeviceInfo() const;

	// Functions which add components to the phone - note these are "add-only"
	// It is assumed that phones are static devices that don't change once they are
	// added (i.e. the display will ALWAYS be 40x2 or whatever, there will ALWAYS be
	// a handset hookswitch, etc.) If the device DOES change, delete the phone and re-create it.
    int AddUploadBuffer (DWORD dwSizeOfBuffer);
    int AddDownloadBuffer (DWORD dwSizeOfBuffer);
    int AddButton (DWORD dwFunction, DWORD dwMode, DWORD dwLampStates, LPCTSTR lpszText);
	int AddHookSwitchDevice (DWORD dwHookSwitchDev, DWORD dwAvailModes, DWORD dwVolume=-1L, DWORD dwGain=-1L, DWORD dwSettableModes=-1L, DWORD dwMonitoredModes=-1L, bool fSupportsVolumeChange = false, bool fSupportsGainChange = false);
    void SetupDisplay (int iColumns, int iRows, char cChar='\n');

// Serialization support
protected:
	friend class CTSPUIDevice;
	virtual TStream& write(TStream& ostm) const;
	virtual TStream& read(TStream& istm);
};

/******************************************************************************/
//
// CTSPUIAddressInfo
//
// This class describes a single dialable address for TAPI.  It is used to 
// store and retrieve line address information for the UI to configure.
//
// Note this only stores basic information about the address.  If you choose
// to use these objects for serialization, you should override this object
// and supply new "read" and "write" functions.
//
/******************************************************************************/
class CTSPUIAddressInfo : public _tspuiBaseObject
{
// Class Data
protected:
	CString m_strDN;				// Dialable number
	CString m_strName;				// Name for the address
	bool m_fAllowIncoming;			// Allow incoming calls
	bool m_fAllowOutgoing;			// Allow outgoing calls
	DWORD m_dwAvailMediaModes;		// Available media modes on this address
	DWORD m_dwBearerMode;			// Bearer modes
	DWORD m_dwMinRate;				// Minimum data rate
	DWORD m_dwMaxRate;				// Maximum data rate
    DWORD m_dwMaxNumActiveCalls;	// Maximum number of active calls	
	DWORD m_dwMaxNumOnHoldCalls;	// Maximum number of onHold calls
	DWORD m_dwMaxNumOnHoldPendCalls; // Maximum number of onHoldPendxxx calls
	DWORD m_dwMaxNumConference;		// Maximum number in a conference
	DWORD m_dwMaxNumTransConf;		// Maximum number transferred.
	DWORD m_dwAddressType;			// Address type (TAPI 3.0)
	LINEDIALPARAMS m_DialParams;	// Dialing parameters

// Public destructor and assignment operator
public:
	const CTSPUIAddressInfo& operator=(const CTSPUIAddressInfo& lc);
	virtual ~CTSPUIAddressInfo();
	CTSPUIAddressInfo(const CTSPUIAddressInfo& addr);
	CTSPUIAddressInfo(LPCTSTR lpszDialableAddr, LPCTSTR lpszAddrName, 
                     bool fAllowIncoming, bool fAllowOutgoing,
                     DWORD dwAvailMediaModes, DWORD dwBearerMode,
                     DWORD dwMinRate, DWORD dwMaxRate, LPLINEDIALPARAMS lpDialParams,
                     DWORD dwMaxNumActiveCalls, DWORD dwMaxNumOnHoldCalls, 
                     DWORD dwMaxNumOnHoldPendCalls, DWORD dwMaxNumConference, 
                     DWORD dwMaxNumTransConf, DWORD dwAddressType);

// Serializable constructor
protected:
	friend class CTSPUILineConnection;
	CTSPUIAddressInfo();
	virtual void Init(LPCTSTR lpszDialableAddr, LPCTSTR lpszAddrName, 
                         bool fAllowIncoming, bool fAllowOutgoing,
                         DWORD dwAvailMediaModes, DWORD dwBearerMode,
                         DWORD dwMinRate, DWORD dwMaxRate, LPLINEDIALPARAMS lpDialParams,
                         DWORD dwMaxNumActiveCalls, DWORD dwMaxNumOnHoldCalls, 
                         DWORD dwMaxNumOnHoldPendCalls, DWORD dwMaxNumConference, 
                         DWORD dwMaxNumTransConf, DWORD dwAddressType);
	DECLARE_DYNCREATE(CTSPUIAddressInfo)

// Access methods - note there is no support for changing many of the setup
// parameters since it is assumed that these don't change - recreate the object
// if they do or provide additional functions in a derived class.
public:
    LPCTSTR GetDialableAddress() const;
	void SetDialableAddress(LPCTSTR pwszAddress);
    LPCTSTR GetName() const;
	void SetName (LPCTSTR pwszName);
    bool CanAnswerCalls() const;
    bool CanMakeCalls() const;

    // Media mode support
    DWORD GetAvailableMediaModes () const;
    DWORD GetBearerMode() const;

	// Call stats
	DWORD GetMinimumDataRate() const;
	DWORD GetMaximumDataRate() const;
	DWORD GetMaxNumActiveCalls() const;
	DWORD GetMaxNumOnHoldCalls() const;
	DWORD GetMaxNumOnHoldPendCalls() const;
	DWORD GetMaxNumInConference() const;
	DWORD GetMaxNumInTransfConference() const;

	// Dialing parameters
	LPLINEDIALPARAMS GetDialParams();

// Serialization support
protected:
	virtual TStream& write(TStream& ostm) const;
	virtual TStream& read(TStream& istm);
};

/******************************************************************************/
// GetUISP
//
// Public method to retrieve a pointer to the main service provider
// application object.
//
/******************************************************************************/
inline CServiceProviderUI* GetUISP() { return (CServiceProviderUI*) AfxGetApp(); }

#ifndef _NOINLINES_
#include "uiline.inl"
#include "uidevice.inl"
#include "uiaddr.inl"
#include "uiagent.inl"
#include "uiphone.inl"
#endif

#ifndef RC_INVOKED
#pragma pack()		// Revert to original packing
#endif 

#endif // _SPLIBUI_INC__

