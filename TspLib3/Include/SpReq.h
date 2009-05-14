/******************************************************************************/
//                                                                        
// SPREQ.H - Request management support objects for TSP++
//
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
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

#ifndef _SPREQ_LIB_INC_
#define _SPREQ_LIB_INC_

#ifndef _SPLIB_INC_
	#error "SPLIB.H must be included before SPREQ.H"
#endif

/*-------------------------------------------------------------------------------*/
// INCLUDE FILES
/*-------------------------------------------------------------------------------*/
#include <qos.h>

/*************************************************************************/
//
// Constants for each of the TAPI functions which we may or may not
// support - each of these functions is queried for at init-time and
// may later be tested for using GetSP()->CanHandleRequest()
//
// If the function isn't listed here, it is either a required BASIC
// exported function, or is completely implemented within the library
// and requires no user-interaction.
//
/*************************************************************************/

#define TSPI_LINEACCEPT                     0
#define TSPI_LINEADDTOCONFERENCE            1
#define TSPI_LINEANSWER                     2
#define TSPI_LINEBLINDTRANSFER              3
#define TSPI_LINECOMPLETECALL               4
#define TSPI_LINECOMPLETETRANSFER           5
#define TSPI_LINECONDITIONALMEDIADETECTION  6
#define TSPI_LINEDEVSPECIFIC                7
#define TSPI_LINEDEVSPECIFICFEATURE         8
#define TSPI_LINEDIAL                       9
#define TSPI_LINEFORWARD                    10
#define TSPI_LINEGATHERDIGITS               11
#define TSPI_LINEGENERATEDIGITS             12
#define TSPI_LINEGENERATETONE               13
#define TSPI_LINEGETDEVCONFIG               14
#define TSPI_LINEGETEXTENSIONID             15
#define TSPI_LINEGETICON                    16
#define TSPI_LINEGETID                      17
#define TSPI_LINEGETLINEDEVSTATUS           18
#define TSPI_LINEHOLD                       19
#define TSPI_LINEMAKECALL                   20
#define TSPI_LINEMONITORDIGITS              21
#define TSPI_LINEMONITORMEDIA               22
#define TSPI_LINEMONITORTONES               23
#define TSPI_LINENEGOTIATEEXTVERSION        24
#define TSPI_LINEPARK                       25
#define TSPI_LINEPICKUP                     26
#define TSPI_LINEPREPAREADDTOCONFERENCE     27
#define TSPI_LINEREDIRECT                   28
#define TSPI_LINERELEASEUSERUSERINFO        29
#define TSPI_LINEREMOVEFROMCONFERENCE       30
#define TSPI_LINESECURECALL                 31
#define TSPI_LINESELECTEXTVERSION           32
#define TSPI_LINESENDUSERUSERINFO           33
#define TSPI_LINESETCALLDATA                34
#define TSPI_LINESETCALLPARAMS              35
#define TSPI_LINESETCALLQUALITYOFSERVICE    36
#define TSPI_LINESETCALLTREATMENT			37
#define TSPI_LINESETDEVCONFIG               38
#define TSPI_LINESETLINEDEVSTATUS           39
#define TSPI_LINESETMEDIACONTROL            40
#define TSPI_LINESETTERMINAL                41
#define TSPI_LINESETUPCONFERENCE            42
#define TSPI_LINESETUPTRANSFER              43
#define TSPI_LINESWAPHOLD                   44
#define TSPI_LINEUNCOMPLETECALL             45
#define TSPI_LINEUNHOLD                     46
#define TSPI_LINEUNPARK                     47
#define TSPI_PHONEDEVSPECIFIC               48
#define TSPI_PHONEGETBUTTONINFO             49
#define TSPI_PHONEGETDATA                   50
#define TSPI_PHONEGETDISPLAY                51
#define TSPI_PHONEGETEXTENSIONID            52
#define TSPI_PHONEGETGAIN                   53
#define TSPI_PHONEGETHOOKSWITCH             54
#define TSPI_PHONEGETICON                   55
#define TSPI_PHONEGETID                     56
#define TSPI_PHONEGETLAMP                   57
#define TSPI_PHONEGETRING                   58
#define TSPI_PHONEGETVOLUME                 59
#define TSPI_PHONENEGOTIATEEXTVERSION       60
#define TSPI_PHONESELECTEXTVERSION          61
#define TSPI_PHONESETBUTTONINFO             62
#define TSPI_PHONESETDATA                   63
#define TSPI_PHONESETDISPLAY                64
#define TSPI_PHONESETGAIN                   65
#define TSPI_PHONESETHOOKSWITCH             66
#define TSPI_PHONESETLAMP                   67
#define TSPI_PHONESETRING                   68
#define TSPI_PHONESETVOLUME                 69
#define TSPI_PROVIDERCREATELINEDEVICE       70
#define TSPI_PROVIDERCREATEPHONEDEVICE      71
#define TSPI_LINEGETCALLHUBTRACKING         72
#define TSPI_ENDOFLIST                      72

/******************************************************************************/
//
// TCapsArray
//
// This array stores bit field flag definitions.  It is designed to hold
// bit field arrays over 32-bits in length, and automatically allocates
// on a byte-per-byte basis as needed.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef std::bitset<TSPI_ENDOFLIST+1> TCapsArray;

// Define common states used in most state machines
#define STATE_INITIAL        0
#define STATE_IGNORE         0xffff0001
#define STATE_NOTPROCESSED   0xffff0002
#define STATE_COMPLETED      0xffff0003

// This is the token sent to ReceiveData() when the library is initiating
// a new request due to it being added to the asynch request list.
#define STARTING_COMMAND        0x0000

// Define all the asynchronous request types supported directly
// by the base class.  For each TAPI function which passes a 
// DRV_REQUESTID, there should be a request.

// TSPI_line requests
#define REQUEST_ACCEPT           0x0001      // TSPI_lineAccept
#define REQUEST_ADDCONF          0x0002      // TSPI_lineAddToConference
#define REQUEST_ANSWER           0x0003      // TSPI_lineAnswer
#define REQUEST_BLINDXFER        0x0004      // TSPI_lineBlindTransfer
#define REQUEST_COMPLETECALL     0x0005      // TSPI_lineCompleteCall
#define REQUEST_COMPLETEXFER     0x0006      // TSPI_lineCompleteTransfer
#define REQUEST_DIAL             0x0007      // TSPI_lineDial
#define REQUEST_DROPCALL         0x0008      // TSPI_lineDropCall
#define REQUEST_FORWARD          0x0009      // TSPI_lineForward
#define REQUEST_HOLD             0x000A      // TSPI_lineHold
#define REQUEST_MAKECALL         0x000B      // TSPI_lineMakeCall
#define REQUEST_PARK             0x000C      // TSPI_linePark
#define REQUEST_PICKUP           0x000D      // TSPI_linePickup
#define REQUEST_REDIRECT         0x000E      // TSPI_lineRedirect
#define REQUEST_REMOVEFROMCONF   0x000F      // TSPI_lineRemoveFromConference
#define REQUEST_SECURECALL       0x0010      // TSPI_lineSecureCall
#define REQUEST_SENDUSERINFO     0x0011      // TSPI_lineSendUserToUser
#define REQUEST_SETCALLPARAMS    0x0012      // TSPI_lineSetCallParams
#define REQUEST_SETTERMINAL      0x0013      // TSPI_lineSetTerminal
#define REQUEST_SETUPCONF        0x0014      // TSPI_lineSetupConference
#define REQUEST_SETUPXFER        0x0015      // TSPI_lineSetupTransfer
#define REQUEST_SWAPHOLD         0x0016      // TSPI_lineSwapHold
#define REQUEST_UNCOMPLETECALL   0x0017      // TSPI_lineUncompleteCall
#define REQUEST_UNHOLD           0x0018      // TSPI_lineUnhold
#define REQUEST_UNPARK           0x0019      // TSPI_lineUnpark
#define REQUEST_MEDIACONTROL     0x001A      // TSPI_lineSetMediaControl (when event is seen)
#define REQUEST_PREPAREADDCONF   0x001B      // TSPI_linePrepareAddToConference
#define REQUEST_GENERATEDIGITS   0x001C      // TSPI_lineGenerateDigits
#define REQUEST_GENERATETONE     0x001D      // TSPI_lineGenerateTones
#define REQUEST_RELEASEUSERINFO  0x001E      // TSPI_lineReleaseUserUserInfo
#define REQUEST_SETCALLDATA      0x001F      // TSPI_lineSetCallData
#define REQUEST_SETQOS           0x0020      // TSPI_lineSetQualityOfService
#define REQUEST_SETCALLTREATMENT 0x0021		 // TSPI_lineSetCallTreatment
#define REQUEST_SETDEVSTATUS     0x0022      // TSPI_lineSetLineDevStatus

// TSPI_phone requests
#define REQUEST_SETBUTTONINFO     0x0023     // TSPI_phoneSetButtonInfo
#define REQUEST_SETDISPLAY        0x0024     // TSPI_phoneSetDisplay
#define REQUEST_SETHOOKSWITCHGAIN 0x0025	 // TSPI_phoneSetGain
#define REQUEST_SETHOOKSWITCH     0x0026     // TSPI_phoneSetHookswitch
#define REQUEST_SETLAMP           0x0027     // TSPI_phoneSetLamp
#define REQUEST_SETRING           0x0028     // TSPI_phoneSetRing
#define REQUEST_SETHOOKSWITCHVOL  0x0029     // TSPI_phoneSetVolume
#define REQUEST_SETPHONEDATA      0x002A     // TSPI_phoneSetData
#define REQUEST_GETPHONEDATA      0x002B     // TSPI_phoneGetData

// PROXY agent requests
#define REQUEST_SETAGENTGROUP     0x002C	// lineSetAgentGroup
#define REQUEST_SETAGENTSTATE     0x002D	// lineSetAgentState
#define REQUEST_SETAGENTACTIVITY  0x002E	// lineSetAgentActivity
#define REQUEST_AGENTSPECIFIC     0x002F	// lineAgentSpecific

// TAPI 2.2 PROXY agent requests
#define REQUEST_CREATEAGENT					0x0030	// lineCreateAgent
#define REQUEST_SETAGENTMEASUREMENTPERIOD	0x0031	// lineSetAgentMeasurementPeriod
#define REQUEST_CREATEAGENTSESSION			0x0032	// lineCreateAgentSession
#define REQUEST_SETAGENTSESSIONSTATE		0x0033	// lineSetAgentSessionState
#define REQUEST_SETQUEUEMEASUREMENTPERIOD	0x0034	// lineSetQueueMeasurementPeriod
#define REQUEST_SETAGENTSTATEEX				0x0035	// lineSetAgentStateEx

// All derived request types should follow this entry.  These would
// include specialized request commands for TSPI_lineDevSpecific
// and TSPI_lineDevSpecificFeature processing.  
//
// Since these must be handled completely by the derived class, no 
// command exists for them, this way, the derived class could have 
// SEVERAL commands depending on the parameters which are passed.

#define REQUEST_END              0x1000

// This special flag is used to clear out the asynchronous queue when
// a device is closed.

#define REQUEST_ALL              0xffff      

/******************************************************************************/
//
// CTSPIRequest class
//
// This class defines a basic request to a TAPI device.  Each request
// is built and stored in the asynchnous request list maintained by the
// connection class.  As a request completes, the request id will
// be sent back via the ASYNCH callback.
//
// The state and item data fields are usable by the CServiceProvider class
// to manage the status of the request.  Each request type creates an object
// derived from this class to manage the data specific to the request.
//
/******************************************************************************/
class CTSPIRequest : public CTSPIBaseObject
{
// Class data
protected:
	LPCTSTR m_pszType;				// Request type name for log, not deallocated!
    CTSPIConnection* m_pConnOwner;  // Connection for this request
    CTSPIAddressInfo* m_pAddress;   // Address for request
    CTSPICallAppearance* m_pCall;   // Call connection.
    DRV_REQUESTID m_dwRequestId;    // Asynch request id for this request
    LONG m_lResult;                 // This is the final result for this request.
	HANDLE m_hevtWait;				// Wait event for any threads
	DWORD m_dwStateTime;			// Tickcount when last state change occurred.
    int m_iReqState;                // Current state (set by processor)
    int m_iReqType;                 // Type of request (COMMAND)
    bool m_fResponseSent;			// Flag indicating we have already responded to TAPI.

// Constructor
public:
    CTSPIRequest(LPCTSTR pszType);
	CTSPIRequest(const CTSPIRequest& src);
    virtual ~CTSPIRequest();

// Methods
public:
    // These are the QUERYxxx functions                             
    CTSPIConnection* GetConnectionInfo() const;
	CTSPILineConnection* GetLineOwner() const;
	CTSPIPhoneConnection* GetPhoneOwner() const;
    CTSPICallAppearance* GetCallInfo() const;
    CTSPIAddressInfo* GetAddressInfo() const;
    DRV_REQUESTID GetAsynchRequestId() const;
    int GetCommand() const;
    int GetState() const;
	bool EnterState(int iLookForState, int iNextState);
    bool HaveSentResponse() const;
	DWORD GetStateTime() const;

    // These are the SETxxx functions
    void SetState(int iState);

    // This function pauses a thread when it wants to wait for the request to complete.
    LONG WaitForCompletion(DWORD dwMsecs);

#ifdef _DEBUG
    virtual TString Dump() const;
#endif

	LPCTSTR GetRequestName() const;

// Two-phase initialization
protected:
	friend class CTSPIConnection;
	void Init(CTSPILineConnection* pConn, CTSPIAddressInfo* pAddr, CTSPICallAppearance* pCall, int iRequest, DRV_REQUESTID dwRequestId);
	void Init(CTSPIPhoneConnection* pConn, int iRequest, DRV_REQUESTID dwRequestId);
    void Complete (LONG lResult, bool fSentTapiNotification=true);
	void UnblockThreads();
	virtual void Failed(LONG lResult);

// Locked members
private:
	CTSPIRequest& operator=(const CTSPIRequest& to);
};

/******************************************************************************/
//
// TRequestList
//
// List manager holding individual CTSPIRequest objects.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_list<CTSPIRequest> TRequestList;

/******************************************************************************/
//
// RTUserUserInfo
//
// A request object for a request which contains UserUser information.
// This is a BASE object used for lineAccept, lineAnswer, etc.
//
/******************************************************************************/
class RTUserUserInfo : public CTSPIRequest
{
// Class data
protected:
	SIZEDDATA m_UserUserInfo;

// Constructor
public:
	RTUserUserInfo(LPCTSTR pszType, CTSPICallAppearance* pCall, int iReqType, DRV_REQUESTID dwRequestID, LPCSTR pszUserUserInfo, DWORD dwSize);

// Access methods
public:
	LPCSTR GetUserUserInfo() const;
	DWORD GetSize() const;

// Locked members
private:
	RTUserUserInfo(const RTUserUserInfo& to);
	RTUserUserInfo& operator=(const RTUserUserInfo& cs);
};

/******************************************************************************/
//
// RTDialInfo
//
// A request object which holds dialing information - used for lineBlindTransfer,
// lineMakeCall, etc.
//
/******************************************************************************/
class RTDialInfo : public CTSPIRequest
{
// Class data
protected:
	TDialStringArray m_arrDialInfo;
	DWORD m_dwCountryCode;

// Constructor
public:
	RTDialInfo(LPCTSTR pszType, CTSPICallAppearance* pCall, int iReqType, DRV_REQUESTID dwRequestID, TDialStringArray* arrDial, DWORD dwCountryCode=0);

// Access methods
public:
	DWORD GetCountryCode() const;
	int GetCount() const;
	DIALINFO* GetDialableNumber(unsigned int i) const;
	TDialStringArray* DialArray();

// Locked members
private:
	RTDialInfo(const RTDialInfo& to);
	RTDialInfo& operator=(const RTDialInfo& cs);
};

/******************************************************************************/
//
// RTAccept
//
// A request object for a lineAccept request.  It contains a pointer
// to the User-User information block which came in with the request.
//
/******************************************************************************/
class RTAccept : public RTUserUserInfo
{
// Constructor
public:
	RTAccept(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, LPCSTR pszUserUserInfo, DWORD dwSize);

// Locked members
private:
	RTAccept(const RTAccept& to);
	RTAccept& operator=(const RTAccept& cs);
};

/******************************************************************************/
//
// RTAddToConference
//
// A request object for a lineAddToConference request.  It contains 
// conferencing information for the request.
//
/******************************************************************************/
class RTAddToConference : public CTSPIRequest
{
// Class data
protected:
    CTSPIConferenceCall* m_pConfCall;  // Conference call we are working with
    CTSPICallAppearance* m_pConsult;   // Call appearance to add to the conference

// Constructor
public:
	RTAddToConference(CTSPIConferenceCall* pConfCall, CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID);
	RTAddToConference(LPCTSTR pszReqName, CTSPIConferenceCall* pConfCall, CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID);

// Access methods
public:
	CTSPIConferenceCall* GetConferenceCall() const;
	CTSPICallAppearance* GetConsultationCall() const;

// Locked members
private:
	RTAddToConference(const RTAddToConference& to);
	RTAddToConference& operator=(const RTAddToConference& cs);
};

/******************************************************************************/
//
// RTAnswer
//
// A request object for a lineAnswer request.  It contains a pointer
// to the User-User information block which came in with the request.
//
/******************************************************************************/
class RTAnswer : public RTUserUserInfo
{
// Constructor
public:
	RTAnswer(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, LPCSTR pszUserUserInfo, DWORD dwSize);

// Locked members
private:
	RTAnswer(const RTAnswer& to);
	RTAnswer& operator=(const RTAnswer& cs);
};

/******************************************************************************/
//
// RTBlindTransfer
//
// A request object for a lineBlindTransfer request.  It contains an array
// of dialing information structures (DIALINFO).
//
/******************************************************************************/
class RTBlindTransfer : public RTDialInfo
{
// Constructor
public:
	RTBlindTransfer(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, TDialStringArray* arrDial, DWORD dwCountryCode);

// Locked members
private:
	RTBlindTransfer(const RTBlindTransfer& to);
	RTBlindTransfer& operator=(const RTBlindTransfer& cs);
};

/******************************************************************************/
//
// RTCompleteCall
//
// A request object for a lineCompleteCall request.  It contains information
// given to the provider on the original completion request.
//
// Note that a portion of this object remains in the service provider 
// while the original call has not completed.  When the original lineCompleteCall
// request is completed by the provider, this object is COPIED into another
// request object which can be found on the line by the completion id or
// call or switch information given.  It may then be used in either a
// subsequent lineUncompleteCall or to notify TAPI when the call completion
// actually finishes on the switch.
//
/******************************************************************************/
class RTCompleteCall : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwCompletionID;			// Completion ID when initial request completes.
    DWORD m_dwCompletionMode;       // Completion mode requested
    DWORD m_dwMessageId;            // Message id to forward to the station
    DWORD m_dwSwitchInfo;			// Unique numeric identifer
    TString m_strSwitchInfo;		// Unique string identifier

// Constructor
public:
	RTCompleteCall(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, DWORD dwCompletionMode, DWORD dwMessageId);
	RTCompleteCall(const RTCompleteCall& pReq);

// Access methods
public:
	DWORD GetCompletionID() const;
	DWORD GetCompletionMode() const;
	DWORD GetMessageID() const;

    // The following should be called by the worker code when the
    // complete call request is sent to the PBX.  In general this would be
    // an extension which will appear on the display, or an id number generated
    // by the switch, etc.  Something to positively identify an incoming call
    // as a call completion request.
	void SetIdentifier(LPCTSTR pszSwitchInfo, DWORD dwSwitchInfo=0);
	void SetIdentifier(DWORD dwSwitchInfo);
	DWORD GetNumericIdentifier() const;
	LPCTSTR GetStringIdentifier() const;

// Locked members
private:
	RTCompleteCall& operator=(const RTCompleteCall& cs);
};

/******************************************************************************/
//
// TCompletionList
//
// Auto-deleting list which contains completion requests pertaining
// to a specific line object.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_list<RTCompleteCall> TCompletionList;

/******************************************************************************/
//
// RTCompleteTransfer
//
// A request object for a lineCompleteTransfer request.  This contains
// the information concerning the TYPE of transfer to complete and the
// resultant conference call which might be used.
//
/******************************************************************************/
class RTCompleteTransfer : public CTSPIRequest
{
// Class data
protected:
    CTSPICallAppearance* m_pConsult;	// Consultation call (destination)
    CTSPIConferenceCall* m_pConfCall;   // New conference call (for complete)
    DWORD m_dwTransferMode;				// Transfer mode

// Constructor
public:
	RTCompleteTransfer(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, 
				       CTSPICallAppearance* pConsultCall, 
					   CTSPIConferenceCall* pConfCall, DWORD dwTransferMode);

// Access methods
public:
	friend class CTSPIConnection;
	DWORD GetTransferMode() const;
	CTSPICallAppearance* GetCallToTransfer() const;
	CTSPICallAppearance* GetConsultationCall() const;
	CTSPIConferenceCall* GetConferenceCall() const;

// Overriden methods
protected:
	virtual void Failed(LONG lResult);

// Locked members
private:
	RTCompleteTransfer(const RTCompleteTransfer& to);
	RTCompleteTransfer& operator=(const RTCompleteTransfer& cs);
};

/******************************************************************************/
//
// RTDial
//
// A request object for a lineDial request.  It contains an array
// of dialing information structures (DIALINFO).
//
/******************************************************************************/
class RTDial : public RTDialInfo
{
// Constructor
public:
	RTDial(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, TDialStringArray* arrDial, DWORD dwCountryCode);

// Locked members
private:
	RTDial(const RTDial& to);
	RTDial& operator=(const RTDial& cs);
};

/******************************************************************************/
//
// RTDropCall
//
// A request object for a lineDrop request.  It contains a pointer
// to the User-User information block which came in with the request.
//
/******************************************************************************/
class RTDropCall : public RTUserUserInfo
{
// Class data
protected:
	bool m_fDropOnClose;		// true if the call is being dropped due to lineClose.
	bool m_fIgnoreDrop;			// Return flag from device code indicating to NOT drop call.

// Constructor
public:
	RTDropCall(CTSPICallAppearance* pCall);
	RTDropCall(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, LPCSTR pszUserUserInfo, DWORD dwSize);

// Access methods
public:
	bool IsImplicitDropFromLineClose() const;
	void IgnoringDrop(bool fIsIgnoring = true);
	bool IsIgnoringDrop() const;

// Overriden methods
protected:
	virtual void Failed(LONG lResult);

// Locked members
private:
	RTDropCall(const RTDropCall& to);
	RTDropCall& operator=(const RTDropCall& cs);
};

/******************************************************************************/
// 
// RTForward
//
// This request holds all the forwarding information for a lineForward
// request.
//
/******************************************************************************/
class RTForward : public CTSPIRequest
{
// Class data
protected:
	TForwardInfoArray* m_parrForwardInfo;	// A list of forward information structures
    DWORD m_dwNumRingsNoAnswer;				// Number of rings before "no answer"
    CTSPICallAppearance* m_pConsult;		// Call appearance created (consultation)
    LPLINECALLPARAMS m_lpCallParams;		// Call parameters for new call

// Constructor
public:
	RTForward(CTSPILineConnection* pLine, CTSPIAddressInfo* pAddr, 
				DRV_REQUESTID dwRequestID, 
				TForwardInfoArray* parrForwardInfo, DWORD dwNumRings, 
				CTSPICallAppearance* pConsultCall=NULL, LPLINECALLPARAMS pCallParams=NULL);
	virtual ~RTForward();

// Access methods
public:
	DWORD GetNoAnswerRingCount() const;
	CTSPICallAppearance* GetConsultationCall() const;
	LPLINECALLPARAMS GetCallParameters();
	unsigned int GetForwardingAddressCount() const;
	TSPIFORWARDINFO* GetForwardingInfo(unsigned int iIndex);
	TForwardInfoArray* GetForwardingArray();

// Overrides from base class
protected:
	virtual void Failed(LONG lResult);

// Locked members
private:
	RTForward(const RTForward& to);
	RTForward& operator=(const RTForward& cs);
};

/******************************************************************************/
// 
// RTMakeCall
//
// This request holds the information used in a lineMakeCall event.
//
/******************************************************************************/
class RTMakeCall : public RTDialInfo
{
// Class data
protected:
    LPLINECALLPARAMS m_lpCallParams;		// Call parameters for new call

// Constructor
public:
	RTMakeCall(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, 
					TDialStringArray* arrDial, DWORD dwCountryCode,
					LPLINECALLPARAMS pCallParams=NULL);
	virtual ~RTMakeCall();

// Access methods
public:
	LPLINECALLPARAMS GetCallParameters();

// Overriden methods
protected:
	virtual void Failed(LONG lResult);

// Locked members
private:
	RTMakeCall(const RTMakeCall& to);
	RTMakeCall& operator=(const RTMakeCall& cs);
};

/******************************************************************************/
// 
// RTPark
//
// This request holds the information used in a linePark event.
//
/******************************************************************************/
class RTPark : public CTSPIRequest
{
// Class data
protected:
    DWORD m_dwParkMode;					// LINEPARKMODE_xxxx value
    TString m_strDirectedAddress;		// Address for directed park
    LPVARSTRING m_lpNonDirAddress;		// Return buffer for non-directed park

// Constructor
public:
	RTPark(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, 
					DWORD dwParkmode, LPCTSTR pszAddress,
					LPVARSTRING lpNonDirAddress);

// Access methods
public:
	LPCTSTR GetDialableNumber() const;
	DWORD GetParkMode() const;
	bool SetParkedAddress(LPCTSTR pszAddress);
	LPVARSTRING ParkedAddress();

// Locked members
private:
	RTPark(const RTPark& to);
	RTPark& operator=(const RTPark& cs);
};

/******************************************************************************/
// 
// RTPickup
//
// This request manages the information associated with a linePickup
// request.
//
/******************************************************************************/
class RTPickup : public RTDialInfo
{
// Class data
protected:
    TString m_strGroupID;				// Group id to which alerting station belongs.

// Constructor
public:
	RTPickup(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, TDialStringArray* parrAddresses, LPCTSTR pszGroupID);

// Access methods
public:
	LPCTSTR GetGroupID() const;

// Overriden methods
protected:
	virtual void Failed(LONG lResult);

// Locked members
private:
	RTPickup(const RTPickup& to);
	RTPickup& operator=(const RTPickup& cs);
};

/******************************************************************************/
//
// RTRedirect
//
// A request object for a lineRedirect request.  It contains an array
// of dialing information structures (DIALINFO).
//
/******************************************************************************/
class RTRedirect : public RTDialInfo
{
// Constructor
public:
	RTRedirect(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, TDialStringArray* arrDial, DWORD dwCountryCode);

// Locked members
private:
	RTRedirect(const RTRedirect& to);
	RTRedirect& operator=(const RTRedirect& cs);
};

/******************************************************************************/
//
// RTRemoveFromConference
//
// A request object for a lineRemoveFromConference request.  It contains
// the call to remove from the conference.
//
/******************************************************************************/
class RTRemoveFromConference : public CTSPIRequest
{
// Class data
protected:
	CTSPIConferenceCall* m_pConfCall;	// Conference call to remove call from

// Constructor
public:
	RTRemoveFromConference(CTSPIConferenceCall* pConfCall, CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID);

// Access methods
public:
	CTSPIConferenceCall* GetConferenceCall() const;
	CTSPICallAppearance* GetCallToRemove() const;

// Locked members
private:
	RTRemoveFromConference(const RTRemoveFromConference& to);
	RTRemoveFromConference& operator=(const RTRemoveFromConference& cs);
};

/******************************************************************************/
//
// RTSendUserInfo
//
// A request object for a lineSendUserUserInfo request.  It contains a pointer
// to the User-User information block which came in with the request.
//
/******************************************************************************/
class RTSendUserInfo : public RTUserUserInfo
{
// Constructor
public:
	RTSendUserInfo(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, LPCSTR pszUserUserInfo, DWORD dwSize);

// Locked members
private:
	RTSendUserInfo(const RTSendUserInfo& to);
	RTSendUserInfo& operator=(const RTSendUserInfo& cs);
};

/******************************************************************************/
//
// RTSetCallParams
//
// A request object for a lineSetCallParams request.  It contains a pointer
// to the call parameters block to change.
//
/******************************************************************************/
class RTSetCallParams : public CTSPIRequest
{
// Class data
protected:
    DWORD m_dwBearerMode;			// New bearer mode for call
    DWORD m_dwMinRate;				// Low bound for call data rate
    DWORD m_dwMaxRate;				// Hi bound for call data rate
    LINEDIALPARAMS m_DialParams;	// New dial parameters

// Constructor
public:
	RTSetCallParams(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID,
		DWORD dwBearerMode, DWORD dwMinRate, DWORD dwMaxRate,  LPLINEDIALPARAMS pdp);

// Access methods
public:
	DWORD GetBearerMode() const;
	DWORD GetMinDataRate() const;
	DWORD GetMaxDataRate() const;
	void SetBearerMode(DWORD dwBearerMode);
	void SetDataRate(DWORD dwMinRate, DWORD dwMaxRate);
	LPLINEDIALPARAMS GetDialParams();

// Locked members
private:
	RTSetCallParams(const RTSetCallParams& to);
	RTSetCallParams& operator=(const RTSetCallParams& cs);
};

/******************************************************************************/
//
// RTSetTerminal
//
// This request object maps a lineSetTerminal request.
//
/******************************************************************************/
class RTSetTerminal : public CTSPIRequest
{
// Class data
protected:
    CTSPILineConnection* m_pLine;     // Line (may be NULL)
    CTSPIAddressInfo* m_pAddress;     // Address (may be NULL)
    CTSPICallAppearance* m_pCall;     // Call appearance (may be NULL)
    DWORD m_dwTerminalModes;          // Terminal mode
    DWORD m_dwTerminalID;             // Terminal to move to
    bool  m_fEnable;                  // Whether to enable or disable terminal.

// Constructor
public:
	RTSetTerminal(CTSPILineConnection* pLine, CTSPIAddressInfo* pAddress,
		CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID,
		DWORD dwTerminalModes, DWORD dwTerminalID, bool fEnable);

// Access methods
public:
	CTSPILineConnection* GetLine() const;
	CTSPIAddressInfo* GetAddress() const;
	CTSPICallAppearance* GetCall() const;
	DWORD GetTerminalModes() const;
	DWORD GetTerminalID() const;
	bool Enable() const;

// Locked members
private:
	RTSetTerminal(const RTSetTerminal& to);
	RTSetTerminal& operator=(const RTSetTerminal& cs);
};

/******************************************************************************/
//
// RTSetupConference
//
// This request object maps a lineSetupConference request.
//
/******************************************************************************/
class RTSetupConference : public CTSPIRequest
{
// Class data
protected:
    CTSPIConferenceCall* m_pConfCall;     // Conference call we are working with
    CTSPICallAppearance* m_pConsult;      // Call appearance created as consultation
	CTSPICallAppearance* m_pOrigParty;	  // Original call party
    DWORD m_dwPartyCount;                 // Initial party count
    LPLINECALLPARAMS m_lpCallParams;      // Call Parameters for consultation call

// Constructor
public:
	RTSetupConference(CTSPIConferenceCall* pConfCall,
		CTSPICallAppearance* pOrigCall,
		CTSPICallAppearance* pConsult, DRV_REQUESTID dwRequestID,
		DWORD dwNumParties, LPLINECALLPARAMS lpCallParams);
	virtual ~RTSetupConference();

// Access methods
public:
	friend class CTSPIConnection;
	CTSPICallAppearance* GetConsultationCall() const;
	CTSPICallAppearance* GetOriginalCall() const;
	CTSPIConferenceCall* GetConferenceCall() const;
	DWORD GetInitialPartyCount() const;
	LPLINECALLPARAMS GetCallParameters();

// Overriden methods
protected:
	virtual void Failed(LONG lResult);

// Locked members
private:
	RTSetupConference(const RTSetupConference& to);
	RTSetupConference& operator=(const RTSetupConference& cs);
};

/******************************************************************************/
//
// RTSetupTransfer
//
// This request object maps a lineSetupTransfer request.
//
/******************************************************************************/
class RTSetupTransfer : public CTSPIRequest
{
// Class data
protected:
    CTSPICallAppearance* m_pConsult;      // Call appearance created as consultation
    LPLINECALLPARAMS m_lpCallParams;      // Call Parameters for consultation call

// Constructor
public:
	RTSetupTransfer(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID,
		CTSPICallAppearance* pConsult, LPLINECALLPARAMS lpCallParams);
	virtual ~RTSetupTransfer();

// Access methods
public:
	friend class CTSPIConnection;
	CTSPICallAppearance* GetCallToTransfer() const;
	CTSPICallAppearance* GetConsultationCall() const;
	LPLINECALLPARAMS GetCallParameters();

// Overriden methods
protected:
	virtual void Failed(LONG lResult);

// Locked members
private:
	RTSetupTransfer(const RTSetupTransfer& to);
	RTSetupTransfer& operator=(const RTSetupTransfer& cs);
};

/******************************************************************************/
//
// RTSwapHold
//
// This request object maps a lineSwapHold request.
//
/******************************************************************************/
class RTSwapHold : public CTSPIRequest
{
// Class data
protected:
    CTSPICallAppearance* m_pConsult;	// Call appearance to swap with

// Constructor
public:
	RTSwapHold(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, CTSPICallAppearance* pConsult);

// Access methods
public:
	CTSPICallAppearance* GetHoldingCall() const;
	CTSPICallAppearance* GetActiveCall() const;

// Locked members
private:
	RTSwapHold(const RTSwapHold& to);
	RTSwapHold& operator=(const RTSwapHold& cs);
};

/******************************************************************************/
//
// RTUncompleteCall
//
// This request object maps a lineUncompleteCall request.
//
/******************************************************************************/
class RTUncompleteCall : public CTSPIRequest
{
// Class data
protected:
	RTCompleteCall* m_pRequest;

// Constructor
public:
	RTUncompleteCall(CTSPILineConnection* pLine, DRV_REQUESTID dwRequestID, RTCompleteCall* pReq);

// Access methods
public:
	RTCompleteCall* GetRTCompleteCall() const;

// Locked members
private:
	RTUncompleteCall(const RTUncompleteCall& to);
	RTUncompleteCall& operator=(const RTUncompleteCall& cs);
};

/******************************************************************************/
//
// RTHold
//
// This request object maps a lineHold request.
//
/******************************************************************************/
class RTHold : public CTSPIRequest
{
// Constructor
public:
	RTHold(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID);

// Locked members
private:
	RTHold(const RTHold& to);
	RTHold& operator=(const RTHold& cs);
};

/******************************************************************************/
//
// RTUnhold
//
// This request object maps a lineUnhold request.
//
/******************************************************************************/
class RTUnhold : public CTSPIRequest
{
// Constructor
public:
	RTUnhold(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID);

// Locked members
private:
	RTUnhold(const RTUnhold& to);
	RTUnhold& operator=(const RTUnhold& cs);
};

/******************************************************************************/
//
// RTSecureCall
//
// This request object maps a lineSecureCall request.
//
/******************************************************************************/
class RTSecureCall : public CTSPIRequest
{
// Constructor
public:
	RTSecureCall(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID);

// Locked members
private:
	RTSecureCall(const RTSecureCall& to);
	RTSecureCall& operator=(const RTSecureCall& cs);
};

/******************************************************************************/
//
// RTSetCallTreatment
//
// This request object maps a lineSetCallTreatment request.
//
/******************************************************************************/
class RTSetCallTreatment : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwCallTreatment;

// Constructor
public:
	RTSetCallTreatment(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, DWORD dwTreatment);

// Methods
public:
	DWORD GetCallTreatment() const;

// Locked members
private:
	RTSetCallTreatment(const RTSetCallTreatment& to);
	RTSetCallTreatment& operator=(const RTSetCallTreatment& cs);
};

/******************************************************************************/
// 
// RTUnpark
//
// This request holds the information used in a lineUnpark event.
//
/******************************************************************************/
class RTUnpark : public RTDialInfo
{
// Constructor
public:
	RTUnpark(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, TDialStringArray* parrAddress);

// Overriden methods
protected:
	virtual void Failed(LONG lResult);

// Locked members
private:
	RTUnpark(const RTUnpark& to);
	RTUnpark& operator=(const RTUnpark& cs);
};

/******************************************************************************/
// 
// RTSetMediaControl
//
// This request holds the information used in a lineSetMediaControl
//
/******************************************************************************/
class RTSetMediaControl : public CTSPIRequest
{
// Class data
protected:
    CTSPILineConnection* m_pLine;     // Line (may be NULL)
    CTSPIAddressInfo* m_pAddress;     // Address (may be NULL)
    CTSPICallAppearance* m_pCall;     // Call appearance (may be NULL)
	TSPIMEDIACONTROL* m_pMediaControl;// Media control object in question.

// Constructor
public:
	RTSetMediaControl(CTSPILineConnection* pLine, CTSPIAddressInfo* pAddress,
		CTSPICallAppearance* pCall, TSPIMEDIACONTROL* pMediaControl);
	virtual ~RTSetMediaControl();

// Access methods
public:
	CTSPILineConnection* GetLine() const;
	CTSPIAddressInfo* GetAddress() const;
	CTSPICallAppearance* GetCall() const;
	TSPIMEDIACONTROL* GetMediaControlInfo();

// Locked members
private:
	RTSetMediaControl(const RTSetMediaControl& to);
	RTSetMediaControl& operator=(const RTSetMediaControl& cs);
};

/******************************************************************************/
//
// RTPrepareAddToConference
//
// A request object for a linePrepareAddToConference request.  It contains 
// conferencing information for the request.
//
/******************************************************************************/
class RTPrepareAddToConference : public RTAddToConference
{
// Class data
protected:
    LPLINECALLPARAMS m_lpCallParams;		// Call parameters for new call

// Constructor
public:
	RTPrepareAddToConference(CTSPIConferenceCall* pConfCall, CTSPICallAppearance* pCall, 
		DRV_REQUESTID dwRequestID, LPLINECALLPARAMS pCallParams);
	virtual ~RTPrepareAddToConference();

// Methods
public:
	LPLINECALLPARAMS GetCallParameters();

// Overriden methods
protected:
	virtual void Failed(LONG lResult);

// Locked members
private:
	RTPrepareAddToConference(const RTPrepareAddToConference& to);
	RTPrepareAddToConference& operator=(const RTPrepareAddToConference& cs);
};

/******************************************************************************/
//
// RTReleaseUserInfo
//
// This request object maps a lineReleaseUserUserInfo request.
//
/******************************************************************************/
class RTReleaseUserInfo : public CTSPIRequest
{
// Constructor
public:
	RTReleaseUserInfo(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID);

// Locked members
private:
	RTReleaseUserInfo(const RTReleaseUserInfo& to);
	RTReleaseUserInfo& operator=(const RTReleaseUserInfo& cs);
};

/******************************************************************************/
//
// RTGenerateDigits
//
// This request object maps a lineGenerateDigits request
//
/******************************************************************************/
class RTGenerateDigits : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwEndToEndID;		// Identifier for this request
	DWORD m_dwDigitMode;        // LINEDIGITMODE_xxxx
	TString m_strDigits;		// Digits to be generated
	DWORD m_dwDuration;			// mSec duration for digits and inter-digit generation.

// Constructor
public:
	RTGenerateDigits(CTSPICallAppearance* pCall, DWORD dwEndToEndID, DWORD dwDigitMode,
					 LPCTSTR lpszDigits, DWORD dwDuration);

// Methods
public:
	DWORD GetIdentifier() const;
	DWORD GetDigitMode() const;
	TString& GetDigits();
	DWORD GetDuration() const;

// Overrides
protected:
	virtual void Failed(LONG lResult);

// Locked members
private:
	RTGenerateDigits(const RTGenerateDigits& to);
	RTGenerateDigits& operator=(const RTGenerateDigits& cs);
};

/******************************************************************************/
//
// RTGenerateTone
//
// This request object maps a lineGenerateTone request
//
/******************************************************************************/
class RTGenerateTone : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwEndToEndID;		// Identifier for this request
	DWORD m_dwToneMode;			// LINETONEMODE_xxxx
	DWORD m_dwDuration;			// mSec duration for digits and inter-digit generation.
	TGenerateToneArray* m_parrTones; // Array of tones to generate

// Constructor
public:
	RTGenerateTone(CTSPICallAppearance* pCall, DWORD dwEndToEndID, DWORD dwToneMode,
					 DWORD dwDuration, TGenerateToneArray* arrTones);
	virtual ~RTGenerateTone();

// Methods
public:
	DWORD GetIdentifier() const;
	DWORD GetToneMode() const;
	DWORD GetDuration() const;
	unsigned int GetToneCount() const;
	LPLINEGENERATETONE GetTone(unsigned int iIndex);
	TGenerateToneArray* GetToneArray();

// Overrides
protected:
	virtual void Failed(LONG lResult);

// Locked members
private:
	RTGenerateTone(const RTGenerateTone& to);
	RTGenerateTone& operator=(const RTGenerateTone& cs);
};

/******************************************************************************/
//
// RTSetCallData
//
// This request object maps a lineSetCallData request
//
/******************************************************************************/
class RTSetCallData : public CTSPIRequest
{
// Class data
protected:
	SIZEDDATA m_CallData;

// Constructor
public:
	RTSetCallData(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, LPVOID lpCallData, DWORD dwSize);

// Methods
public:
	DWORD GetSize() const;
	LPCVOID GetData() const;

// Locked members
private:
	RTSetCallData(const RTSetCallData& to);
	RTSetCallData& operator=(const RTSetCallData& cs);
};

/******************************************************************************/
//
// RTSetQualityOfService
//
// This request object maps a lineSetQualityOfService request
//
/******************************************************************************/
class RTSetQualityOfService : public CTSPIRequest
{
// Class data
protected:
	SIZEDDATA m_SendingFlowSpec;
	SIZEDDATA m_RecvFlowSpec;

// Constructor
public:
	RTSetQualityOfService(CTSPICallAppearance* pCall, DRV_REQUESTID dwRequestID, 
					LPVOID lpvSFS, DWORD dwSFSSize, LPVOID lpvRFS, DWORD dwRFSSize);

// Methods
public:
	const FLOWSPEC* GetSendingFlowSpec() const;
	const FLOWSPEC* GetReceivingFlowSpec() const;
	DWORD GetSendingFlowSpecSize() const;
	DWORD GetReceivingFlowSpecSize() const;

// Locked members
private:
	RTSetQualityOfService(const RTSetQualityOfService& to);
	RTSetQualityOfService& operator=(const RTSetQualityOfService& cs);
};

/******************************************************************************/
//
// RTSetLineDevStatus
//
// This request object maps a lineSetLineDevStatus request
//
/******************************************************************************/
class RTSetLineDevStatus : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwLineStatus;	// Status bits to change
	bool  m_fSetBits;		// Whether to turn them on or off.

// Constructor
public:
	RTSetLineDevStatus(CTSPILineConnection* pLine, DRV_REQUESTID dwRequestID, 
					DWORD dwStatusBits, bool fSet);

// Methods
public:
	DWORD GetStatusBitsToChange() const;
	bool TurnOnBits() const;

// Locked members
private:
	RTSetLineDevStatus(const RTSetLineDevStatus& to);
	RTSetLineDevStatus& operator=(const RTSetLineDevStatus& cs);
};

/******************************************************************************/
//
// RTSetButtonInfo
//
// This request object maps a phoneSetButtonInfo request
//
/******************************************************************************/
class RTSetButtonInfo : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwButtonID;			// Button/Lamp identifier
	LPPHONEBUTTONINFO m_lpbi;	// Phone button information

// Constructor
public:
	RTSetButtonInfo(CTSPIPhoneConnection* pPhone, DRV_REQUESTID dwRequestID,
					DWORD dwButtonLampID, LPPHONEBUTTONINFO lpbi);
	~RTSetButtonInfo();

// Methods
public:
	DWORD GetButtonLampID() const;
	DWORD GetButtonMode() const;
	DWORD GetButtonFunction() const;
	LPCTSTR GetButtonText() const;
	LPVOID GetDevSpecificInfo();
	LPPHONEBUTTONINFO GetButtonInfo();

// Locked members
private:
	RTSetButtonInfo(const RTSetButtonInfo& to);
	RTSetButtonInfo& operator=(const RTSetButtonInfo& cs);
};

/******************************************************************************/
//
// RTSetLampInfo
//
// This request object maps a phoneSetLampInfo request
//
/******************************************************************************/
class RTSetLampInfo : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwLampID;			// Button/Lamp identifier
	DWORD m_dwMode;				// New lamp mode

// Constructor
public:
	RTSetLampInfo(CTSPIPhoneConnection* pPhone, DRV_REQUESTID dwRequestID, DWORD dwButtonLampID, DWORD dwMode);

// Methods
public:
	DWORD GetButtonLampID() const;
	DWORD GetLampMode() const;

// Locked members
private:
	RTSetLampInfo(const RTSetLampInfo& to);
	RTSetLampInfo& operator=(const RTSetLampInfo& cs);
};

/******************************************************************************/
//
// RTSetRing
//
// This request object maps a phoneSetRing request
//
/******************************************************************************/
class RTSetRing : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwRingMode;			// Ringer mode
	DWORD m_dwVolume;			// New volume

// Constructor
public:
	RTSetRing(CTSPIPhoneConnection* pPhone, DRV_REQUESTID dwRequestID, DWORD dwRingMode, DWORD dwVolume);

// Methods
public:
	DWORD GetRingMode() const;
	DWORD GetVolume() const;

// Locked members
private:
	RTSetRing(const RTSetRing& to);
	RTSetRing& operator=(const RTSetRing& cs);
};

/******************************************************************************/
//
// RTSetDisplay
//
// This request object maps a phoneSetDisplay request
//
/******************************************************************************/
class RTSetDisplay : public CTSPIRequest
{
// Class data
protected:
	unsigned int m_iColumn;		// Column to set
	unsigned int m_iRow;		// Row to set
	SIZEDDATA m_Display;		// Display information

// Constructor
public:
	RTSetDisplay(CTSPIPhoneConnection* pPhone, DRV_REQUESTID dwRequestID, unsigned int Row, unsigned int Column, LPTSTR pszDisplay, DWORD dwSize);

// Methods
public:
	unsigned int GetColumn() const;
	unsigned int GetRow() const;
	LPCTSTR GetBufferPtr() const;
	DWORD GetBufferSize() const;

// Locked members
private:
	RTSetDisplay(const RTSetDisplay& to);
	RTSetDisplay& operator=(const RTSetDisplay& cs);
};

/******************************************************************************/
//
// RTSetGain
//
// This request object maps a phoneSetGain request
//
/******************************************************************************/
class RTSetGain : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwDevice;		// Device to set
	DWORD m_dwGain;			// Value to set

// Constructor
public:
	RTSetGain(CTSPIPhoneConnection* pPhone, DRV_REQUESTID dwRequestID, DWORD dwHookswitch, DWORD dwGain);

// Methods
public:
	DWORD GetHookswitchDevice() const;
	DWORD GetGain() const;

// Locked members
private:
	RTSetGain(const RTSetGain& to);
	RTSetGain& operator=(const RTSetGain& cs);
};

/******************************************************************************/
//
// RTSetVolume
//
// This request object maps a phoneSetVolume request
//
/******************************************************************************/
class RTSetVolume : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwDevice;		// Device to set
	DWORD m_dwVolume;		// Value to set

// Constructor
public:
	RTSetVolume(CTSPIPhoneConnection* pPhone, DRV_REQUESTID dwRequestID, DWORD dwHookswitch, DWORD dwVolume);

// Methods
public:
	DWORD GetHookswitchDevice() const;
	DWORD GetVolume() const;

// Locked members
private:
	RTSetVolume(const RTSetVolume& to);
	RTSetVolume& operator=(const RTSetVolume& cs);
};

/******************************************************************************/
//
// RTSetHookswitch
//
// This request object maps a phoneSetVolume request
//
/******************************************************************************/
class RTSetHookswitch : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwDevice;		// Device to set
	DWORD m_dwState;		// Value to set

// Constructor
public:
	RTSetHookswitch(CTSPIPhoneConnection* pPhone, DRV_REQUESTID dwRequestID, DWORD dwHookswitch, DWORD dwState);

// Methods
public:
	DWORD GetHookswitchDevice() const;
	DWORD GetHookswitchState() const;

// Locked members
private:
	RTSetHookswitch(const RTSetHookswitch& to);
	RTSetHookswitch& operator=(const RTSetHookswitch& cs);
};

/******************************************************************************/
//
// RTGetPhoneData
//
// This request object maps a phoneGetData request
//
/******************************************************************************/
class RTGetPhoneData : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwIndex;		// Which buffer to retrieve
	LPVOID m_lpBuff;		// Buffer to store data into
	DWORD m_dwSize;			// Size of the buffer

// Constructor
public:
	RTGetPhoneData(CTSPIPhoneConnection* pPhone, DWORD dwIndex, LPVOID lpBuff, DWORD dwSize);

// Methods
public:
	DWORD GetUploadIndex() const;
	DWORD GetSize() const;
	LPVOID GetBuffer();

// Locked members
private:
	RTGetPhoneData(const RTGetPhoneData& to);
	RTGetPhoneData& operator=(const RTGetPhoneData& cs);
};

/******************************************************************************/
//
// RTSetPhoneData
//
// This request object maps a phoneSetData request
//
/******************************************************************************/
class RTSetPhoneData : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwIndex;		// Which buffer to retrieve
	SIZEDDATA m_lpData;		// Phone data

// Constructor
public:
	RTSetPhoneData(CTSPIPhoneConnection* pPhone, DRV_REQUESTID dwRequestID,	DWORD dwIndex, LPVOID lpBuff, DWORD dwSize);

// Methods
public:
	DWORD GetDownloadIndex() const;
	DWORD GetSize() const;
	LPCVOID GetBuffer() const;

// Locked members
private:
	RTSetPhoneData(const RTSetPhoneData& to);
	RTSetPhoneData& operator=(const RTSetPhoneData& cs);
};

/******************************************************************************/
//
// RTSetAgentGroup
//
// This request object maps a lineSetAgentGroup request (PROXY)
//
/******************************************************************************/
class RTSetAgentGroup : public CTSPIRequest
{
// Class data
protected:
	TAgentGroupArray* m_parrGroups;

// Constructor
public:
	RTSetAgentGroup(CTSPIAddressInfo* pAddr, DRV_REQUESTID dwRequestID, TAgentGroupArray* pGroups);
	~RTSetAgentGroup();

// Methods
public:
	unsigned int GetCount() const;
	const TAgentGroup* GetGroup(unsigned int iIndex) const;
	TAgentGroupArray* GetGroupArray();

// Locked members
private:
	RTSetAgentGroup(const RTSetAgentGroup& to);
	RTSetAgentGroup& operator=(const RTSetAgentGroup& cs);
};

/******************************************************************************/
//
// RTSetAgentState
//
// This request object maps a lineSetAgentState request (PROXY)
//
/******************************************************************************/
class RTSetAgentState : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwState;
	DWORD m_dwNextState;

// Constructor
public:
	RTSetAgentState(CTSPIAddressInfo* pAddr, DRV_REQUESTID dwRequestID, DWORD dwState, DWORD dwNextState);

// Methods
public:
	DWORD GetAgentState() const;
	DWORD GetNextAgentState() const;
	void SetAgentState(DWORD dwState);
	void SetNextAgentState(DWORD dwNextState);

// Locked members
private:
	RTSetAgentState(const RTSetAgentState& to);
	RTSetAgentState& operator=(const RTSetAgentState& cs);
};

/******************************************************************************/
//
// RTSetAgentActivity
//
// This request object maps a lineSetAgentActivity request (PROXY)
//
/******************************************************************************/
class RTSetAgentActivity : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwActivityID;

// Constructor
public:
	RTSetAgentActivity(CTSPIAddressInfo* pAddr, DRV_REQUESTID dwRequestID, DWORD dwActivity);

// Methods
public:
	DWORD GetActivity() const;

// Locked members
private:
	RTSetAgentActivity(const RTSetAgentActivity& to);
	RTSetAgentActivity& operator=(const RTSetAgentActivity& cs);
};

/******************************************************************************/
//
// RTAgentSpecific
//
// This request object maps a lineAgentSpecific request (PROXY)
//
/******************************************************************************/
class RTAgentSpecific : public CTSPIRequest
{
// Class data
protected:
	DWORD m_dwAgentExtensionIDIndex;
	SIZEDDATA m_lpData;

// Constructor
public:
	RTAgentSpecific(CTSPIAddressInfo* pAddr, DRV_REQUESTID dwRequestID, DWORD dwAgentExtensionID, LPVOID lpvBuff, DWORD dwSize);

// Methods
public:
	DWORD GetExtensionID() const;
	DWORD GetBufferSize() const;
	LPVOID GetBuffer();

// Locked members
private:
	RTAgentSpecific(const RTAgentSpecific& to);
	RTAgentSpecific& operator=(const RTAgentSpecific& cs);
};

/******************************************************************************/
//
// DECLARE_TSPI_REQUESTS, ON_TSPI_REQUEST, BEGIN_TSPI_REQUEST, END_TSPI_REQUEST
//
// This is our map-handler macros for using the automatic ReceiveData
// support built into the CTSPIConnection object
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/

typedef bool (CTSPIConnection::*tsplib_REQPROCNULL)(void);
typedef bool (CTSPIConnection::*tsplib_REQPROC)(CTSPIRequest*, LPCVOID);

#pragma pack(4)
typedef struct
{
	UINT nRequest;
	tsplib_REQPROCNULL fpReq;
} tsplib_REQMAP;
#pragma pack()

#define DECLARE_TSPI_REQUESTS() \
private: static const tsplib_REQMAP _requestMapEntries[]; \
protected: virtual const tsplib_REQMAP* GetRequestList() const { return &_requestMapEntries[0]; }\

#define BEGIN_TSPI_REQUEST(thisClass) const tsplib_REQMAP thisClass::_requestMapEntries[] = {
#define ON_TSPI_REQUEST(rid, func) { rid, (tsplib_REQPROCNULL)(tsplib_REQPROC)&func },
#define ON_TSPI_REQUEST_ACCEPT(func) { REQUEST_ACCEPT, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTAccept*, LPCVOID))&func },
#define ON_TSPI_REQUEST_ADDCONF(func) { REQUEST_ADDCONF, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTAddToConference*, LPCVOID))&func },
#define ON_TSPI_REQUEST_ANSWER(func) { REQUEST_ANSWER, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTAnswer*, LPCVOID))&func },
#define ON_TSPI_REQUEST_BLINDXFER(func) { REQUEST_BLINDXFER, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTBlindTransfer*, LPCVOID))&func },
#define ON_TSPI_REQUEST_COMPLETECALL(func) { REQUEST_COMPLETECALL, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTCompleteCall*, LPCVOID))&func },
#define ON_TSPI_REQUEST_COMPLETEXFER(func) { REQUEST_COMPLETEXFER, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTCompleteTransfer*, LPCVOID))&func },
#define ON_TSPI_REQUEST_DIAL(func) { REQUEST_DIAL, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTDial*, LPCVOID))&func },
#define ON_TSPI_REQUEST_DROPCALL(func) { REQUEST_DROPCALL, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTDropCall*, LPCVOID))&func },
#define ON_TSPI_REQUEST_FORWARD(func) { REQUEST_FORWARD, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTForward*, LPCVOID))&func },
#define ON_TSPI_REQUEST_HOLD(func) { REQUEST_HOLD, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTHold*, LPCVOID))&func },
#define ON_TSPI_REQUEST_MAKECALL(func) { REQUEST_MAKECALL, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTMakeCall*, LPCVOID))&func },
#define ON_TSPI_REQUEST_PARK(func) { REQUEST_PARK, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTPark*, LPCVOID))&func },
#define ON_TSPI_REQUEST_PICKUP(func) { REQUEST_PICKUP, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTPickup*, LPCVOID))&func },
#define ON_TSPI_REQUEST_REDIRECT(func) { REQUEST_REDIRECT, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTRedirect*, LPCVOID))&func },
#define ON_TSPI_REQUEST_REMOVEFROMCONF(func) { REQUEST_REMOVEFROMCONF, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTRemoveFromConference*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SECURECALL(func) { REQUEST_SECURECALL, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSecureCall*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SENDUSERINFO(func) { REQUEST_SENDUSERINFO, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSendUserInfo*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETCALLPARAMS(func) { REQUEST_SETCALLPARAMS, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetCallParams*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETTERMINAL(func) { REQUEST_SETTERMINAL, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetTerminal*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETUPCONF(func) { REQUEST_SETUPCONF, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetupConference*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETUPXFER(func) { REQUEST_SETUPXFER, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetupTransfer*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SWAPHOLD(func) { REQUEST_SWAPHOLD, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSwapHold*, LPCVOID))&func },
#define ON_TSPI_REQUEST_UNCOMPLETECALL(func) { REQUEST_UNCOMPLETECALL, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTUncompleteCall*, LPCVOID))&func },
#define ON_TSPI_REQUEST_UNHOLD(func) { REQUEST_UNHOLD, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTUnhold*, LPCVOID))&func },
#define ON_TSPI_REQUEST_UNPARK(func) { REQUEST_UNPARK, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTUnpark*, LPCVOID))&func },
#define ON_TSPI_REQUEST_MEDIACONTROL(func) { REQUEST_MEDIACONTROL, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetMediaControl*, LPCVOID))&func },
#define ON_TSPI_REQUEST_PREPAREADDCONF(func) { REQUEST_PREPAREADDCONF, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTPrepareAddToConference*, LPCVOID))&func },
#define ON_TSPI_REQUEST_GENERATEDIGITS(func) { REQUEST_GENERATEDIGITS, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTGenerateDigits*, LPCVOID))&func },
#define ON_TSPI_REQUEST_GENERATETONE(func) { REQUEST_GENERATETONE, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTGenerateTone*, LPCVOID))&func },
#define ON_TSPI_REQUEST_RELEASEUSERINFO(func) { REQUEST_RELEASEUSERINFO, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTReleaseUserInfo*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETCALLDATA(func) { REQUEST_SETCALLDATA, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetCallData*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETQOS(func) { REQUEST_SETQOS, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetQualityOfService*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETCALLTREATMENT(func) { REQUEST_SETCALLTREATMENT, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetCallTreatment*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETDEVSTATUS(func) { REQUEST_SETDEVSTATUS, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetLineDevStatus*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETBUTTONINFO(func) { REQUEST_SETBUTTONINFO, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetButtonInfo*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETDISPLAY(func) { REQUEST_SETDISPLAY, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetDisplay*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETHOOKSWITCHGAIN(func) { REQUEST_SETHOOKSWITCHGAIN, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetGain*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETHOOKSWITCH(func) { REQUEST_SETHOOKSWITCH, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetHookswitch*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETLAMP(func) { REQUEST_SETLAMP, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetLampInfo*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETRING(func) { REQUEST_SETRING, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetRing*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETHOOKSWITCHVOL(func) { REQUEST_SETHOOKSWITCHVOL, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetVolume*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETPHONEDATA(func) { REQUEST_SETPHONEDATA, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetPhoneData*, LPCVOID))&func },
#define ON_TSPI_REQUEST_GETPHONEDATA(func) { REQUEST_GETPHONEDATA, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTGetPhoneData*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETAGENTGROUP(func) { REQUEST_SETAGENTGROUP, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetAgentGroup*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETAGENTSTATE(func) { REQUEST_SETAGENTSTATE, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetAgentState*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETAGENTACTIVITY(func) { REQUEST_SETAGENTACTIVITY, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetAgentActivity*, LPCVOID))&func },
#define ON_TSPI_REQUEST_AGENTSPECIFIC(func) { REQUEST_AGENTSPECIFIC, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTAgentSpecific*, LPCVOID))&func },
#define ON_TSPI_REQUEST_CREATEAGENT(func) { REQUEST_CREATEAGENT, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTCreateAgent*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETAGENTMEASUREMENTPERIOD(func) { REQUEST_SETAGENTMEASUREMENTPERIOD, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetAgentMeasurementPeriod*, LPCVOID))&func },
#define ON_TSPI_REQUEST_CREATEAGENTSESSION(func) { REQUEST_CREATEAGENTSESSION, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTCreateAgentSession*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETAGENTSESSIONSTATE(func) { REQUEST_SETAGENTSESSIONSTATE, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetAgentSessionState*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETQUEUEMEASUREMENTPERIOD(func) { REQUEST_SETQUEUEMEASUREMENTPERIOD, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetQueueMeasurementPeriod*, LPCVOID))&func },
#define ON_TSPI_REQUEST_SETAGENTSTATEEX(func) { REQUEST_SETAGENTSTATEEX, (tsplib_REQPROCNULL)(bool (CTSPIConnection::*)(RTSetAgentStateEx*, LPCVOID))&func },
#define ON_AUTO_TSPI_REQUEST(rid) { rid, (tsplib_REQPROCNULL) 0 },
#define END_TSPI_REQUEST() { 0, (tsplib_REQPROCNULL)0 } };

/******************************************************************************/
// 
// TRequestMap
//
// Maps request functions to numeric identifiers
//
/******************************************************************************/
typedef std::map<UINT, tsplib_REQPROC> TRequestMap;

#endif // _SPREQ_LIB_INC_
