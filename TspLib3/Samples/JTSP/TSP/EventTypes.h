/*******************************************************************/
//
// EVENTTYPES.H
//
// Objects to manage the various events from the PBX
//
// Copyright (C) 1998 JulMar Technology, Inc.
// All rights reserved
//
// TSP++ Version 3.00 PBX/ACD Emulator Projects
//
// Modification History
// ---------------------------------------------------------------
// 09/04/1998	MCS@Julmar	Initial revision
//
/*******************************************************************/

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __JPBX_EVENTS_INC__
#define __JPBX_EVENTS_INC__

/*---------------------------------------------------------------*/
// CONSTANTS
/*---------------------------------------------------------------*/
const UINT DISPLAY_SIZE = 40;

/*---------------------------------------------------------------*/
// PRE-DEFINITIONS
/*---------------------------------------------------------------*/
class CEventBlock;

/*****************************************************************************
** Procedure:  xtoi
**
** Arguments: 'pszValue' - ASCII Value to convert
**
** Returns:   'iValue' - Returned numeric value
**
** Description: This function converts a null terminated ascii string for a 
**              hex number to its integer value.  Should just be the number 
**              with no preceeding "0x" or trailing "H".  Garbage will result 
**              if there are non-hex digits in the string.  Hex digits are: 
**              0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F, a, b, c, d, e, f. 
**              Non-hex digits will be treated like '0'. 
**
*****************************************************************************/
inline unsigned int xtoi(LPCTSTR pszValue)
{ 
	register TCHAR ch; 
	register unsigned int nValue = 0; 

	while ((ch = *pszValue++) != 0) 
	{ 
		if (_istdigit(ch)) 
			ch -= _T('0'); 
		else if (ch >= _T('A') && ch <= _T('F')) 
			ch += (TCHAR)(10 - _T('A')); 
		else if (ch >= _T('a') && ch <= _T('f')) 
			ch += (TCHAR)(10 - _T('a')); 
		else 
			ch = (TCHAR)0; 

		nValue = (16 * nValue + ch); 
	} 
	return nValue; 

}// xtoi

/*****************************************************************************
** Procedure:  GetNextElem
**
** Arguments: 'strCommand' - String from PBX
**
** Returns:    'strElem' - Token parsed out of string
**
** Description: This function parses out the next token from the PBX string.
**              It modifies the input string (removes the token).
**
*****************************************************************************/
inline TString GetNextElem(TString& strBuff)
{
	TString::size_type iPos = strBuff.find(_T(';'));
	if (iPos == 0)
	{
		strBuff = strBuff.substr(iPos+1);
		return _T("");			
	}

	if (iPos == TString::npos)
		iPos = strBuff.length();
		
	TString strReturn = strBuff.substr(0,iPos);
	if (iPos < strBuff.length())
		strBuff = strBuff.substr(iPos+1);
	else
		strBuff.erase();
	
	return strReturn;

}// GetNextElem

/**************************************************************************
** CEventFactory
**
** This factory object creates the proper CDataBlock object which 
** encapsulates the event received from the PBX.
**
***************************************************************************/
class CEventFactory
{
// Class data
protected:
	CEventBlock* m_pHead;		// First element in the linked-list

// Constructor/Destructor
public:
	CEventFactory();
	~CEventFactory();

// Creation method
public:
	CEventBlock* Create(TString& strCommand);
		
// Unavailable methods
private:
	CEventFactory( const CEventFactory& );
	CEventFactory& operator=( const CEventFactory& );
};

/**************************************************************************
** CPBXElement
**
** This object represents a single element from a PBX message
**
***************************************************************************/
class CPBXElement
{
// Class data
public:
#ifdef _DEBUG
	TCHAR chSig[5];
#endif
	enum PBXElement {
		CallID = 0,
		OldCallID,
		Extension,
		TargetExtension,
		Command,
		Response,
		ErrorCode,
		DNIS,
		ANI,
		CallState,
		ConfCallID1,
		ConfCallID2,
		Queue,
		Digit,
		AnswerType,
		Display,
		Gain,
		Volume,
		Hookswitch,
		Lamp,
		AgentID,
		AgentState,
		PrimaryAgentGroup,
		SecondaryAgentGroup,
		Version,
		Trunk,
		CallStatistics,
		QueueStatistics,
	};

// Constructor
public:
	CPBXElement() {
#ifdef _DEBUG
		lstrcpy(chSig, _T("PBXE"));
#endif
	}
	virtual ~CPBXElement() {}
#ifdef _DEBUG
	virtual TString Dump() const { return _T("Error: missing Dump override!\n"); }
#endif
};

/**************************************************************************
** CPECallID
**
** An encapsulated call id object
**
***************************************************************************/
class CPECallID : public CPBXElement
{
// Class data
protected:
	DWORD m_dwCallID;	// Callid

// Constructor
public:
	CPECallID(TString& strData) : m_dwCallID(0) {
	    m_dwCallID = xtoi(GetNextElem(strData).c_str());
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << _T("0x") << hex << m_dwCallID;
		return stream.str();
	}
#endif
	DWORD GetCallID() const { return m_dwCallID; }
};

/**************************************************************************
** CPECallStats
**
** An encapsulated call statistics object
**
***************************************************************************/
class CPECallStats : public CPBXElement
{
// Class data
protected:
	int m_iSecondsInState;

// Constructor
public:
	CPECallStats(TString& strData) {
	    m_iSecondsInState = _ttoi(GetNextElem(strData).c_str());
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << m_iSecondsInState;
		return stream.str();
	}
#endif
	int GetSecondsInState() const { return m_iSecondsInState; }
};

/**************************************************************************
** CPEQueueStats
**
** An encapsulated queue statistics object
**
***************************************************************************/
class CPEQueueStats : public CPBXElement
{
// Class data
protected:
	int m_iSecondsInState;
	int m_iQueueCount;
	int m_iAgentCount;

// Constructor
public:
	CPEQueueStats(TString& strData) {
		m_iQueueCount = _ttoi(GetNextElem(strData).c_str());
	    m_iSecondsInState = _ttoi(GetNextElem(strData).c_str());
		m_iAgentCount = _ttoi(GetNextElem(strData).c_str());
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << _T("Queued=") << m_iQueueCount;
		stream << _T(",Oldest=") << m_iSecondsInState;
		stream << _T(",Agents=") << m_iAgentCount;
		return stream.str();
	}
#endif
	int GetOldestCallTime() const { return m_iSecondsInState; }
	int GetAgentCount() const { return m_iAgentCount; }
	int GetTotalQueued() const { return m_iQueueCount; }
};

/**************************************************************************
** CPEAnswerType
**
** An encapsulated answer type from a predictive dialer object
**
***************************************************************************/
class CPEAnswerType : public CPBXElement
{
// Class data
public:
	enum AnswerType {
		Busy = 0,
		AnswerVoice,
		AnswerMachine,
		AnswerUnknown,
		NoAnswer
	};
protected:
	AnswerType m_atType;

// Constructor
public:
	CPEAnswerType(TString& strData) {
		LPCTSTR g_pszStates = _T("BVMUNO");
		TString strState = GetNextElem(strData);
		_TSP_ASSERTE(strState.length() > 0);
		for (int i = 0; *(g_pszStates+i) != 0; i++)
		{
			if (*(g_pszStates+i) == toupper(strState[0]))
			{
				m_atType = static_cast<AnswerType>(i);
				break;
			}
		}
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		switch (m_atType)
		{
			case Busy:			stream << _T("Busy"); break;
			case AnswerVoice:	stream << _T("Voice"); break;
			case AnswerMachine: stream << _T("Machine"); break;
			case AnswerUnknown: stream << _T("Unknown"); break;
			case NoAnswer:		stream << _T("No Answer"); break;
		}
		return stream.str();
	}
#endif
	AnswerType GetAnswerType() const { return m_atType; }
};

/**************************************************************************
** CPETrunk
**
** An encapsulated trunk return from the PlaceCall request
**
***************************************************************************/
class CPETrunk : public CPBXElement
{
protected:
	int m_iTrunk;

// Constructor
public:
	CPETrunk(TString& strData) {
		m_iTrunk = _ttoi(GetNextElem(strData).c_str());
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << m_iTrunk;
		return stream.str();
	}
#endif
	int GetTrunk() const { return m_iTrunk; }
};

/**************************************************************************
** CPEDigit
**
** An encapsulated digit object
**
***************************************************************************/
class CPEDigit : public CPBXElement
{
// Class data
protected:
	TCHAR m_chDigit;

// Constructor
public:
	CPEDigit(TString& strData) : m_chDigit(0) {
	    TString strBuff = GetNextElem(strData);
		if (strBuff.length() > 0)
			m_chDigit = strBuff[0];
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << m_chDigit;
		return stream.str();
	}
#endif
	TCHAR GetDigit() const { return m_chDigit; }
};

/**************************************************************************
** CPECallState
**
** An encapsulated call state object
**
***************************************************************************/
class CPECallState : public CPBXElement
{
// Class data
public:
	enum CallState {
		Unknown=0,			// Unknown
		Dialing,			// Dialing
		Ringing,			// Call is offering to a station
		Alerting,			// Station is offering tone for offering call
		Connected,			// Call is connected
		Busy,				// Call is busy
		Disconnected,		// Call is disconnected
		Holding,			// This side is on hold
		OtherSideHolding,	// The other side is on hold
		Queued				// Call is queued
	};

protected:
	CallState m_csState;

// Constructor
public:
	CPECallState(TString& strData) {
		LPCTSTR g_pszStates = _T("UDRACBKHIQ");
		TString strState = GetNextElem(strData);
		_TSP_ASSERTE(strState.length() > 0);
		for (int i = 0; *(g_pszStates+i) != 0; i++)
		{
			if (*(g_pszStates+i) == toupper(strState[0]))
			{
				m_csState = static_cast<CallState>(i);
				break;
			}
		}
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		switch (m_csState)
		{
			case Unknown:			stream << _T("Unknown"); break;
			case Dialing:			stream << _T("Dialing"); break;
			case Ringing:			stream << _T("Ringing"); break;
			case Alerting:			stream << _T("Alerting"); break;
			case Connected:			stream << _T("Connected"); break;
			case Busy:				stream << _T("Busy"); break;
			case Disconnected:		stream << _T("Disconnected"); break;
			case Holding:			stream << _T("Holding"); break;
			case OtherSideHolding:	stream << _T("OtherSideHolding"); break;
			case Queued:			stream << _T("Queued"); break;
		}
		return stream.str();
	}
#endif
	CallState GetCallState() const { return m_csState; }
};

/**************************************************************************
** CPEExtension
**
** An encapsulated extension object
**
***************************************************************************/
class CPEExtension : public CPBXElement
{
// Class data
protected:
	DWORD m_dwExtension;	// Extension

// Constructor
public:
	CPEExtension(TString& strData) {
	    m_dwExtension = _ttol(GetNextElem(strData).c_str());
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << m_dwExtension;
		return stream.str();
	}
#endif
	DWORD GetExtension() const { return m_dwExtension; }
};

/**************************************************************************
** CPEErrorCode
**
** An encapsulated error code object
**
***************************************************************************/
class CPEErrorCode : public CPBXElement
{
// Class data
public:
	enum Errors {
		None = 0,						// No error
		InvalidDevice,					// Invalid device identifier
		InvalidExtension,				// Invalid extension passed
		InvalidParameter,				// Invalid parameter
		InvalidCallID,					// Invalid Callid
		ResourceUnavailable,			// Resource unavailable on PBX
		BadCRC,							// Bad CRC
		BadCommand,						// Bad command for this device
		BadState,						// Device is in the improper state for command
		InvalidLogon,					// Not logged on
		InvalidAgent,					// Bad agent id
		InvalidGroup,					// Bad agent group
		InvalidAgentState,				// Incorrect agent state
		Failed							// Command failed
	};

protected:
	DWORD m_dwErrorCode;	// Error code

// Constructor
public:
	CPEErrorCode(TString& strData) {
	    m_dwErrorCode = xtoi(GetNextElem(strData).c_str());
		if (m_dwErrorCode > InvalidAgentState)
			m_dwErrorCode = Failed;
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << _T("(") << hex << m_dwErrorCode << _T(") ");
		switch (m_dwErrorCode)
		{
			case None:					stream << _T("None"); break;
			case InvalidDevice:			stream << _T("Invalid Device"); break;
			case InvalidExtension:		stream << _T("Invalid Extension"); break;
			case InvalidParameter:		stream << _T("Invalid Parameter"); break;
			case InvalidCallID:			stream << _T("Invalid Callid"); break;
			case ResourceUnavailable:	stream << _T("Resource Unavailable"); break;
			case BadCRC:				stream << _T("Bad CRC"); break;
			case BadCommand:			stream << _T("Bad Command"); break;
			case BadState:				stream << _T("Bad State"); break;
			case InvalidLogon:			stream << _T("Invalid Logon"); break;
			case InvalidAgent:			stream << _T("Invalid Agent ID"); break;
			case InvalidGroup:			stream << _T("Invalid Agent Group ID"); break;
			case InvalidAgentState:		stream << _T("Invalid Agent State"); break;
			default:					stream << _T("Unknown"); break;
		}
		return stream.str();
	}
#endif
	DWORD GetError() const { return m_dwErrorCode; }
};

/**************************************************************************
** CPECallInfo
**
** An encapsulated DNIS or ANI object
**
***************************************************************************/
class CPECallInfo : public CPBXElement
{
// Class data
protected:
	TString m_strName;
	TString m_strNumber;

// Constructor
public:
	CPECallInfo(TString& strData) {
		TString strDNIS = GetNextElem(strData);
		TString::size_type iPos = strDNIS.find(_T(':'));
		if (iPos == TString::npos)
			m_strNumber = strDNIS;
		else
		{
			if (iPos == 0)
				m_strNumber = strDNIS.substr(1);
			else
			{
				m_strName = strDNIS.substr(0,iPos);
				m_strNumber = strDNIS.substr(iPos+1);
			}
		}
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << _T("Name: ") << m_strName << _T(",Number: ") << m_strNumber;
		return stream.str();
	}
#endif
	TString GetName() const { return m_strName; }
	TString GetNumber() const { return m_strNumber; }
};

/**************************************************************************
** CPECommand
**
** An encapsulated command code object
**
***************************************************************************/
class CPECommand : public CPBXElement
{
// Class data
public:
	enum Commands {
		Logon = 0,
		AgentLogon,
		AgentState,
		HoldCall,
		RetrieveCall,
		GenerateDigit,
		PlaceCall,
		ReleaseCall,
		AnswerCall,
		TransferCall,
		Version,
		AcceptCall,
		BlindTransfer,
		GetCallStats,
		GetQueueStats,
		PlacePredictiveCall,
		QueryAgentStates,
		SetGain,
		SetVolume,
		SetHookSwitch,
		Invalid,
	};

protected:
	int m_iCommand;	

// Constructor
public:
	CPECommand(TString& strData) {
		static LPCTSTR pszCommands[] = {
			{ _T("LO") },
			{ _T("ALO") },
			{ _T("CAS") },
			{ _T("HC") },
			{ _T("RTC") },
			{ _T("GTD") },
			{ _T("PC") },
			{ _T("RC") },
			{ _T("AN") },
			{ _T("TC") },
			{ _T("VER") },
			{ _T("AC") },
			{ _T("BTC") },
			{ _T("GCS") },
			{ _T("GQS") },
			{ _T("PPC") },
			{ _T("QAS") },
			{ _T("SPG") },
			{ _T("SPV") },
			{ _T("SPH") },
			{ NULL }
		};
	    TString strCommand = GetNextElem(strData);
		for (m_iCommand = Logon; m_iCommand < Invalid; m_iCommand++)
		{
			if (!lstrcmpi(strCommand.c_str(), pszCommands[m_iCommand]))
				break;
		}
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		switch (m_iCommand)
		{
			case Logon:			stream << _T("Logon"); break;
			case AgentLogon:	stream << _T("Agent Logon"); break;
			case AgentState:	stream << _T("Change Agent State"); break;
			case HoldCall:		stream << _T("Hold Call"); break;
			case RetrieveCall:	stream << _T("Retrieve Call"); break;
			case GenerateDigit:	stream << _T("Generate Digit/Tone"); break;
			case PlaceCall:		stream << _T("Place Call"); break;
			case ReleaseCall:	stream << _T("Release Call"); break;
			case AnswerCall:	stream << _T("Answer Call"); break;
			case TransferCall:	stream << _T("Transfer Call"); break;
			case Version:		stream << _T("Version"); break;
			case AcceptCall:	stream << _T("Accept Call"); break;
			case SetGain:		stream << _T("Set Gain"); break;
			case SetVolume:		stream << _T("Set Volume"); break;
			case SetHookSwitch:	stream << _T("Set Hookswitch"); break;
			case BlindTransfer:	stream << _T("Blind Transfer"); break;
			case GetCallStats:  stream << _T("Get Call Statistics"); break;
			case GetQueueStats:	stream << _T("Get Queue Statistics"); break;
			case PlacePredictiveCall: stream << _T("Place Predictive Call"); break;
			case QueryAgentStates:	  stream << _T("Query Agent States"); break;
			case Invalid:		stream << _T("*Invalid*"); break;
		}
		return stream.str();
	}
#endif
	int GetCommand() const { return m_iCommand; }
};

/**************************************************************************
** CPEResponse
**
** An encapsulated command response object
**
***************************************************************************/
class CPEResponse : public CPBXElement
{
// Class data
protected:
	TString m_strCommand;

// Constructor
public:
	CPEResponse(TString& strData) {
	    m_strCommand = GetNextElem(strData);
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << m_strCommand;
		return stream.str();
	}
#endif
	TString GetResponse() const { return m_strCommand; }
};

/**************************************************************************
** CPEDisplay
**
** An encapsulated display object
**
***************************************************************************/
class CPEDisplay : public CPBXElement
{
// Class data
protected:
	TString m_strDisplay;

// Constructor
public:
	CPEDisplay(TString& strData) {
	    m_strDisplay = GetNextElem(strData);
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << _T("[") << m_strDisplay.substr(0,10) << _T("]");
		return stream.str();
	}
#endif
	TString GetDisplay() const { return m_strDisplay; }
	TString GetFirstLine() const { return m_strDisplay.substr(0,DISPLAY_SIZE); }
	TString GetSecondLine() const { return m_strDisplay.substr(DISPLAY_SIZE); }
};

/**************************************************************************
** CPEGain
**
** An encapsulated speaker gain object
**
***************************************************************************/
class CPEGain : public CPBXElement
{
// Class data
protected:
	int m_nValue;

// Constructor
public:
	CPEGain(TString& strData) {
	    m_nValue = _ttoi(GetNextElem(strData).c_str());
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << m_nValue;
		return stream.str();
	}
#endif
	int GetGain() const { return m_nValue; }
};

/**************************************************************************
** CPEVolume
**
** An encapsulated speaker volume object
**
***************************************************************************/
class CPEVolume : public CPBXElement
{
// Class data
protected:
	int m_nValue;

// Constructor
public:
	CPEVolume(TString& strData) {
	    m_nValue = _ttoi(GetNextElem(strData).c_str());
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << m_nValue;
		return stream.str();
	}
#endif
	int GetVolume() const { return m_nValue; }
};

/**************************************************************************
** CPELampInfo
**
** An encapsulated lamp object
**
***************************************************************************/
class CPELampInfo : public CPBXElement
{
// Class data
protected:
	int m_nLampID;
	bool m_fLit;

// Constructor
public:
	CPELampInfo(TString& strData) : m_fLit(false) {
	    m_nLampID = _ttoi(GetNextElem(strData).c_str());
		TString strLamp = GetNextElem(strData);
		if (strLamp.length() > 0 &&	toupper(strLamp[0]) == _T('S'))
			m_fLit = true;
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << _T("Id=") << m_nLampID << _T(",State=");
		if (m_fLit)
			stream << _T("on");
		else
			stream << _T("off");
		return stream.str();
	}
#endif
	int GetLampID() const { return m_nLampID; }
	bool IsLit() const { return m_fLit; }
};

/**************************************************************************
** CPEHookswitch
**
** An encapsulated lamp object
**
***************************************************************************/
class CPEHookswitch : public CPBXElement
{
// Class data
public:
	enum HSState {
		OnHook = 0,
		OffHook = 1,
		Muted = 2
	};

protected:
	HSState m_hsState;

// Constructor
public:
	CPEHookswitch(TString& strData) : m_hsState(Muted) {
		TString strState = GetNextElem(strData);
		if (strState.length() > 0)
		{
			if (toupper(strState[0]) == _T('O'))
				m_hsState = OnHook;
			else if (toupper(strState[0]) == _T('F'))
				m_hsState = OffHook;
		}
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		if (m_hsState == OnHook)
			stream << _T("OnHook");
		else if (m_hsState == OffHook)
			stream << _T("OffHook (MIC/SPEAKER)");
		else
			stream << _T("OffHook (Muted)");
		return stream.str();
	}
#endif
	HSState GetHookswitchState() const { return m_hsState; }
};

/**************************************************************************
** CPEAgentID
**
** An encapsulated agent id object
**
***************************************************************************/
class CPEAgentID : public CPBXElement
{
// Class data
protected:
	TString m_strAgentID;

// Constructor
public:
	CPEAgentID(TString& strData) {
	    m_strAgentID = GetNextElem(strData);
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << m_strAgentID;
		return stream.str();
	}
#endif
	TString GetAgentID() const { return m_strAgentID; }
};

/**************************************************************************
** CPEAgentState
**
** An encapsulated agent state object
**
***************************************************************************/
class CPEAgentState : public CPBXElement
{
// Class data
public:
	enum AgentState { 
			Unknown = 0,		// Agent state not known
			SignedOut,			// Agent is not signed in to any address
			Ready,				// Agent is ready to receive a call	
			NotReady,			// Agent is not ready
			InCallWork,			// Agent is busy performing after call work
			BusyACD,			// Agent is handling an ACD routed call
			Busy,				// Agent is handled a non-ACD routed call
		};

protected:
	AgentState m_asState;

// Constructor
public:
	CPEAgentState(TString& strData) {
		LPCTSTR g_pszStates = _T("USRNWAB");
		TString strState = GetNextElem(strData);
		_TSP_ASSERTE(strState.length() > 0);
		for (int i = 0; *(g_pszStates+i) != 0; i++)
		{
			if (*(g_pszStates+i) == toupper(strState[0]))
			{
				m_asState = static_cast<AgentState>(i);
				break;
			}
		}
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		switch (m_asState)
		{
			case Unknown:		stream << _T("Unknown"); break;
			case SignedOut:		stream << _T("Signed Out"); break;
			case Ready:			stream << _T("Ready"); break;
			case NotReady:		stream << _T("Not Ready"); break;
			case InCallWork:	stream << _T("In Call Work"); break;
			case BusyACD:		stream << _T("Busy ACD"); break;
			case Busy:			stream << _T("Busy"); break;
		}
		return stream.str();
	}
#endif
	AgentState GetAgentState() const { return m_asState; }
};

/**************************************************************************
** CPEAgentGroup
**
** An encapsulated agent group object
**
***************************************************************************/
class CPEAgentGroup : public CPBXElement
{
// Class data
protected:
	DWORD m_dwAgentGroup;	// Agent Group id

// Constructor
public:
	CPEAgentGroup(TString& strData) {
	    m_dwAgentGroup = xtoi(GetNextElem(strData).c_str());
	}		
	
// Access methods
public:
#ifdef _DEBUG
	virtual TString Dump() const { 
		TStringStream stream;
		stream << _T("0x") << hex << m_dwAgentGroup;
		return stream.str();
	}
#endif
	DWORD GetAgentGroup() const { return m_dwAgentGroup; }
};

/**************************************************************************
** CEventBlock
**
** This object represents a single event from the switch
**
***************************************************************************/
class CEventBlock
{
// Class data
public:
#ifdef _DEBUG
	TCHAR chSig[5];
#endif
	enum PBXEvent {
		Unknown = 0,			// ???
		CommandResponse,		// ACK/NAK
		CallDetected,			// CD
		CallPlaced,				// CP
		CallStateChange,		// CS
		CallReleased,			// CR
		CallConference,			// CC
		CallTransfer,			// CT
		CallQueued,				// CQ
		DigitDetected,			// DD
		CallMediaDetected,		// CMD
		AgentStateChanged,		// ASC
		AgentGroupChanged,		// AGC
		// This is the start of the PHONE events
		// Do NOT put line events after this point!
		DisplayChanged,			// PDC
		VolumeChanged,			// PVC
		GainChanged,			// PGC
		HookswitchChanged,		// PHC
		LampChanged,			// PLC
	};

protected:
	CEventBlock* m_pNext;		// Next block in the list (normally NULL)
	std::map<int, CPBXElement*> m_mapElements;
	LONG m_lRefCount;

// Destructor
public:
	virtual ~CEventBlock() {
		_TSP_ASSERTE(m_lRefCount == 0); 
		for (std::map<int, CPBXElement*>::iterator iPos = m_mapElements.begin();
			iPos != m_mapElements.end(); ++iPos)
			delete (*iPos).second;
	}

// Factory methods
protected:
	CEventBlock* Create(PBXEvent evtType, TString& strData) {
		if (evtType == GetEventType()) return Create(strData);
		else if (m_pNext) return m_pNext->Create(evtType, strData);
		return 0;
	}
	virtual CEventBlock* Create(TString& strData) = 0;

// Constructors
protected:
	CEventBlock() : m_pNext(0), m_lRefCount(1) {
#ifdef _DEBUG
		lstrcpy(chSig, _T("PBXC"));
#endif
	}
	CEventBlock(CEventBlock** ppBlock) : m_lRefCount(0) {
#ifdef _DEBUG
		lstrcpy(chSig, _T("PBXP"));
#endif
		m_pNext = *ppBlock;
		*ppBlock = this;
	}

// Public methods
public:
	virtual PBXEvent GetEventType() const = 0;

	// Locate an element in our map
	const CPBXElement* GetElement(int iKey) const {
		std::map<int, CPBXElement*>::const_iterator iPos = m_mapElements.find(iKey);
		if (iPos == m_mapElements.end())
			return NULL;
		return (*iPos).second;
	}

// Methods
public:
	void AddRef() { 
		InterlockedIncrement(&m_lRefCount);
	}
	void Decrement() { 
		if (InterlockedDecrement(&m_lRefCount) == 0) delete this;
	}

#ifdef _DEBUG
	TString Dump() const {
		TStringStream stream;
		stream << _T("PBXEvent (0x") << hex << (DWORD)this << _T(") ");
		switch (GetEventType())
		{
			case Unknown:			stream << _T("Unknown"); break;
			case CommandResponse:	stream << _T("ACK/NAK"); break;
			case CallDetected:		stream << _T("Call Detected"); break;
			case CallPlaced:		stream << _T("Call Placed"); break;
			case CallStateChange:	stream << _T("Call State Changed"); break;
			case CallReleased:		stream << _T("Call Released"); break;
			case CallConference:	stream << _T("Call Conferenced"); break;
			case CallTransfer:		stream << _T("Call Transferred"); break;
			case CallQueued:		stream << _T("Call Queued"); break;
			case DigitDetected:		stream << _T("Digit Detected"); break;
			case CallMediaDetected:	stream << _T("Media Detected"); break;
			case AgentStateChanged:	stream << _T("Agent State Changed"); break;
			case AgentGroupChanged:	stream << _T("Agent Group Changed"); break;
			case DisplayChanged:	stream << _T("Display Changed"); break;
			case VolumeChanged:		stream << _T("Volume Changed"); break;
			case GainChanged:		stream << _T("Gain Changed"); break;
			case HookswitchChanged:	stream << _T("Hookswitch Changed"); break;
			case LampChanged:		stream << _T("Lamp Changed"); break;
		}
		stream << _T(":");

		// Dump all the keys
		bool fFirst = false;
		for (std::map<int, CPBXElement*>::const_iterator iPos = m_mapElements.begin();
			 iPos != m_mapElements.end(); iPos++)
		{
			if (fFirst)
				stream << _T(",");
			else
				fFirst = true;

			switch ((*iPos).first)
			{
				case CPBXElement::CallID:			stream << _T("CallID"); break;
				case CPBXElement::OldCallID:		stream << _T("Old CallID"); break;
				case CPBXElement::Extension:		stream << _T("Extension"); break;
				case CPBXElement::TargetExtension:	stream << _T("Target Extension"); break;
				case CPBXElement::Command:			stream << _T("Command"); break;
				case CPBXElement::Response:			stream << _T("Response"); break;
				case CPBXElement::ErrorCode:		stream << _T("Error Code"); break;
				case CPBXElement::DNIS:				stream << _T("DNIS"); break;
				case CPBXElement::ANI:				stream << _T("ANI"); break;
				case CPBXElement::CallState:		stream << _T("Call State"); break;
				case CPBXElement::ConfCallID1:		stream << _T("Conf CallID 1"); break;
				case CPBXElement::ConfCallID2:		stream << _T("Conf CallID 2"); break;
				case CPBXElement::Queue:			stream << _T("Queue ID"); break;
				case CPBXElement::Digit:			stream << _T("Digit"); break;
				case CPBXElement::AnswerType:		stream << _T("Answer Type"); break;
				case CPBXElement::Display:			stream << _T("Display"); break;
				case CPBXElement::Gain:				stream << _T("Gain"); break;
				case CPBXElement::Volume:			stream << _T("Volume"); break;
				case CPBXElement::Hookswitch:		stream << _T("Hookswitch"); break;
				case CPBXElement::Lamp:				stream << _T("Lamp"); break;
				case CPBXElement::AgentID:			stream << _T("Agent ID"); break;
				case CPBXElement::AgentState:		stream << _T("Agent State"); break;
				case CPBXElement::PrimaryAgentGroup:	stream << _T("Primary Agent Group"); break;
				case CPBXElement::SecondaryAgentGroup:	stream << _T("Secondary Agent Group"); break;
				case CPBXElement::Version:			stream << _T("Version"); break;
				case CPBXElement::Trunk:			stream << _T("Trunk"); break;
				case CPBXElement::CallStatistics:	stream << _T("CallStats"); break;
				case CPBXElement::QueueStatistics:	stream << _T("QueueStats"); break;
			}

			stream << _T("=") <<  (*iPos).second->Dump();
		}
		stream << endl;
		return stream.str();
	}
#endif		

// Internal methods
protected:
	void AddElement(int iKey, CPBXElement* pElem) {
		m_mapElements[iKey] = pElem; 
	}

// Unavailable methods
private:
	friend class CEventFactory;
	CEventBlock& operator=(const CEventBlock& eb);
	CEventBlock(const CEventBlock& eb);
};

/**************************************************************************
** CEBResponse
**
** This object represents an ACK/NAK response from the PBX switch
**
***************************************************************************/
class CEBResponse : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBResponse(strData); }
	CEBResponse(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBResponse(TString& strData) {
		CPECommand* pCommand = new CPECommand(strData);
		AddElement(CPBXElement::Command, pCommand);
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::ErrorCode, new CPEErrorCode(strData));

		switch (pCommand->GetCommand())
		{
			case CPECommand::AgentLogon:
				AddElement(CPBXElement::AgentID, new CPEAgentID(strData));
				break;
			case CPECommand::Version:
				AddElement(CPBXElement::Version, new CPEResponse(strData));
				break;
			case CPECommand::PlaceCall:
				AddElement(CPBXElement::Trunk, new CPETrunk(strData));
				AddElement(CPBXElement::CallID, new CPECallID(strData));
				break;
			case CPECommand::GetCallStats:
				AddElement(CPBXElement::CallStatistics, new CPECallStats(strData));
				break;
			case CPECommand::GetQueueStats:
				AddElement(CPBXElement::QueueStatistics, new CPEQueueStats(strData));
				break;
			default:
				AddElement(CPBXElement::Response, new CPEResponse(strData));
				break;
		}
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::CommandResponse; }
private:
	CEBResponse(const CEBResponse&);
	void operator=(const CEBResponse&);
};

/**************************************************************************
** CEBCallDetected
**
** This object represents an CD event from the switch
**
***************************************************************************/
class CEBCallDetected : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBCallDetected(strData); }
	CEBCallDetected(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBCallDetected(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::CallID, new CPECallID(strData));
		AddElement(CPBXElement::DNIS, new CPECallInfo(strData));
		AddElement(CPBXElement::ANI, new CPECallInfo(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::CallDetected; }
private:
	CEBCallDetected(const CEBCallDetected&);
	void operator=(const CEBCallDetected&);
};

/**************************************************************************
** CEBCallPlaced
**
** This object represents an CP event from the switch
**
***************************************************************************/
class CEBCallPlaced : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBCallPlaced(strData); }
	CEBCallPlaced(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBCallPlaced(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::CallID, new CPECallID(strData));
		AddElement(CPBXElement::DNIS, new CPECallInfo(strData));
		AddElement(CPBXElement::ANI, new CPECallInfo(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::CallPlaced; }
private:
	CEBCallPlaced(const CEBCallPlaced&);
	void operator=(const CEBCallPlaced&);
};

/**************************************************************************
** CEBCallStateChange
**
** This object represents an CS event from the switch
**
***************************************************************************/
class CEBCallStateChange : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBCallStateChange(strData); }
	CEBCallStateChange(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBCallStateChange(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::CallID, new CPECallID(strData));
		AddElement(CPBXElement::CallState, new CPECallState(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::CallStateChange; }
private:
	CEBCallStateChange(const CEBCallStateChange&);
	void operator=(const CEBCallStateChange&);
};

/**************************************************************************
** CEBCallReleased
**
** This object represents an CR event from the switch
**
***************************************************************************/
class CEBCallReleased : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBCallReleased(strData); }
	CEBCallReleased(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBCallReleased(TString& strData) {
		AddElement(CPBXElement::CallID, new CPECallID(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::CallReleased; }
private:
	CEBCallReleased(const CEBCallReleased&);
	void operator=(const CEBCallReleased&);
};

/**************************************************************************
** CEBCallConference
**
** This object represents an CC event from the switch
**
***************************************************************************/
class CEBCallConference : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBCallConference(strData); }
	CEBCallConference(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBCallConference(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::CallID, new CPECallID(strData));
		AddElement(CPBXElement::ConfCallID1, new CPECallID(strData));
		AddElement(CPBXElement::ConfCallID2, new CPECallID(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::CallConference; }
private:
	CEBCallConference(const CEBCallConference&);
	void operator=(const CEBCallConference&);
};

/**************************************************************************
** CEBCallTransfer
**
** This object represents an CT event from the switch
**
***************************************************************************/
class CEBCallTransfer : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBCallTransfer(strData); }
	CEBCallTransfer(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBCallTransfer(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::CallID, new CPECallID(strData));
		AddElement(CPBXElement::OldCallID, new CPECallID(strData));
		AddElement(CPBXElement::TargetExtension, new CPEExtension(strData));
		AddElement(CPBXElement::DNIS, new CPECallInfo(strData));
		AddElement(CPBXElement::ANI, new CPECallInfo(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::CallTransfer; }
private:
	CEBCallTransfer(const CEBCallTransfer&);
	void operator=(const CEBCallTransfer&);
};

/**************************************************************************
** CEBCallQueued
**
** This object represents an CQ event from the switch
**
***************************************************************************/
class CEBCallQueued : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBCallQueued(strData); }
	CEBCallQueued(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBCallQueued(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::CallID, new CPECallID(strData));
		AddElement(CPBXElement::Queue, new CPEExtension(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::CallQueued; }
private:
	CEBCallQueued(const CEBCallQueued&);
	void operator=(const CEBCallQueued&);
};

/**************************************************************************
** CEBDigitDetected
**
** This object represents an DD event from the switch
**
***************************************************************************/
class CEBDigitDetected : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBDigitDetected(strData); }
	CEBDigitDetected(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBDigitDetected(TString& strData) {
		AddElement(CPBXElement::CallID, new CPECallID(strData));
		AddElement(CPBXElement::Digit, new CPEDigit(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::DigitDetected; }
private:
	CEBDigitDetected(const CEBDigitDetected&);
	void operator=(const CEBDigitDetected&);
};

/**************************************************************************
** CEBCallMediaDetected
**
** This object represents an CMD event from the switch
**
***************************************************************************/
class CEBCallMediaDetected : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBCallMediaDetected(strData); }
	CEBCallMediaDetected(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBCallMediaDetected(TString& strData) {
		AddElement(CPBXElement::CallID, new CPECallID(strData));
		AddElement(CPBXElement::AnswerType, new CPEAnswerType(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::CallMediaDetected; }
private:
	CEBCallMediaDetected(const CEBCallMediaDetected&);
	void operator=(const CEBCallMediaDetected&);
};

/**************************************************************************
** CEBDisplayChanged
**
** This object represents an PDC event from the switch
**
***************************************************************************/
class CEBDisplayChanged : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBDisplayChanged(strData); }
	CEBDisplayChanged(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBDisplayChanged(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::Display, new CPEDisplay(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::DisplayChanged; }
private:
	CEBDisplayChanged(const CEBDisplayChanged&);
	void operator=(const CEBDisplayChanged&);
};

/**************************************************************************
** CEBVolumeChanged
**
** This object represents an PVC event from the switch
**
***************************************************************************/
class CEBVolumeChanged : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBVolumeChanged(strData); }
	CEBVolumeChanged(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBVolumeChanged(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::Volume, new CPEVolume(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::VolumeChanged; }
private:
	CEBVolumeChanged(const CEBVolumeChanged&);
	void operator=(const CEBVolumeChanged&);
};

/**************************************************************************
** CEBGainChanged
**
** This object represents an PGC event from the switch
**
***************************************************************************/
class CEBGainChanged : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBGainChanged(strData); }
	CEBGainChanged(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBGainChanged(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::Gain, new CPEGain(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::GainChanged; }
private:
	CEBGainChanged(const CEBGainChanged&);
	void operator=(const CEBGainChanged&);
};

/**************************************************************************
** CEBHookswitchChanged
**
** This object represents an PGC event from the switch
**
***************************************************************************/
class CEBHookswitchChanged : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBHookswitchChanged(strData); }
	CEBHookswitchChanged(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBHookswitchChanged(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::Hookswitch, new CPEHookswitch(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::HookswitchChanged; }
private:
	CEBHookswitchChanged(const CEBHookswitchChanged&);
	void operator=(const CEBHookswitchChanged&);
};

/**************************************************************************
** CEBLampChanged
**
** This object represents an PGC event from the switch
**
***************************************************************************/
class CEBLampChanged : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBLampChanged(strData); }
	CEBLampChanged(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBLampChanged(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::Lamp, new CPELampInfo(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::LampChanged; }
private:
	CEBLampChanged(const CEBLampChanged&);
	void operator=(const CEBLampChanged&);
};

/**************************************************************************
** CEBAgentStateChanged
**
** This object represents an PGC event from the switch
**
***************************************************************************/
class CEBAgentStateChanged : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBAgentStateChanged(strData); }
	CEBAgentStateChanged(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBAgentStateChanged(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::AgentID, new CPEAgentID(strData));
		AddElement(CPBXElement::AgentState, new CPEAgentState(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::AgentStateChanged; }
private:
	CEBAgentStateChanged(const CEBAgentStateChanged&);
	void operator=(const CEBAgentStateChanged&);
};

/**************************************************************************
** CEBAgentGroupChanged
**
** This object represents an PGC event from the switch
**
***************************************************************************/
class CEBAgentGroupChanged : public CEventBlock
{
// Constructor
protected:
	friend class CEventFactory;
	virtual CEventBlock* Create(TString& strData) { return new CEBAgentGroupChanged(strData); }
	CEBAgentGroupChanged(CEventBlock** pBlock) : CEventBlock(pBlock) {}
	CEBAgentGroupChanged(TString& strData) {
		AddElement(CPBXElement::Extension, new CPEExtension(strData));
		AddElement(CPBXElement::AgentID, new CPEAgentID(strData));
		AddElement(CPBXElement::PrimaryAgentGroup, new CPEAgentGroup(strData));
		AddElement(CPBXElement::SecondaryAgentGroup, new CPEAgentGroup(strData));
	}

// Access methods
public:
	virtual PBXEvent GetEventType() const { return PBXEvent::AgentGroupChanged; }
private:
	CEBAgentGroupChanged(const CEBAgentGroupChanged&);
	void operator=(const CEBAgentGroupChanged&);
};

#endif // __JPBX_EVENTS_INC__