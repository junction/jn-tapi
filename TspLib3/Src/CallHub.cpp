/******************************************************************************/
//                                                                        
// CALLHUB.CPP - Source code for the CTSPICallHub object.
//                                                                        
// Copyright (C) 1994-2004 JulMar Entertainment Technology, Inc.
// All rights reserved                                                    
//                                                                        
// This file contains all the code for managing the call hub array object
// which handles multiple calls with the same callid.
//
// This source code is intended only as a supplement to the
// TSP++ Class Library product documentation.  This source code cannot 
// be used in part or whole in any form outside the TSP++ library.
//                                                                        
/******************************************************************************/

/*---------------------------------------------------------------------------*/
// INCLUDE FILES
/*---------------------------------------------------------------------------*/
#include "stdafx.h"

///////////////////////////////////////////////////////////////////////////
// CTSPICallHub::CTSPICallHub
//
// Constructor for the call hub
//
CTSPICallHub::CTSPICallHub(DWORD dwCallID) : CTSPIBaseObject(),
	m_dwCallID(dwCallID)
{
}// CTSPICallHub::CTSPICallHub

///////////////////////////////////////////////////////////////////////////
// CTSPICallHub::~CTSPICallHub
//
// Destructor for the call hub
//
CTSPICallHub::~CTSPICallHub()
{
}// CTSPICallHub::~CTSPICallHub

///////////////////////////////////////////////////////////////////////////
// CTSPICallHub::AddToHub
//
// Add the given call to this call hub
//
void CTSPICallHub::AddToHub(CTSPICallAppearance* pCall)
{
	_TSP_ASSERTE (pCall->GetCallID() == m_dwCallID);

	CEnterCode Lock(this);
	if (std::find(m_lstHub.begin(), m_lstHub.end(), pCall) == m_lstHub.end())
	{
		m_lstHub.push_back(pCall);
#ifdef _DEBUG
		_TSP_DTRACEX(TRC_CALLMAP, _T("Added 0x%lx to hub %s"), pCall, Dump().c_str());
#endif
	}

}// CTSPICallHub::AddToHub

///////////////////////////////////////////////////////////////////////////
// CTSPICallHub::RemoveFromHub
//
// Remove the given call from this call hub. If this is the final
// call then remove the hub from the system.
//
void CTSPICallHub::RemoveFromHub(const CTSPICallAppearance* pCall)
{
	CEnterCode sLock(this);  // Synch access to object

	// Find and remove the call
	TCallHubList::iterator i = std::find(m_lstHub.begin(), m_lstHub.end(), pCall);
	if (i != m_lstHub.end())
		m_lstHub.erase(i);

#ifdef _DEBUG
	_TSP_DTRACEX(TRC_CALLMAP, _T("Remove 0x%lx from hub %s"), pCall, Dump().c_str());
#endif

	// If we have no more entries then remove this hub
	if (m_lstHub.empty())
		pCall->GetLineOwner()->GetDeviceInfo()->RemoveCallHub(m_dwCallID, this);

}// CTSPICallHub::RemoveFromHub

///////////////////////////////////////////////////////////////////////////
// CTSPICallHub::OnCallStateChange
//
// Forward a callstate change to all calls in our hub. This function
// is called in the context of the CTSPICallAppearance::SetCallState
// function.
//
void CTSPICallHub::OnCallStateChange(CTSPICallAppearance* pCall, DWORD dwState, DWORD dwCurrState) const
{
	// Lock the hub while we copy
	CEnterCode sLock(this);

	// Copy the list so the underlying master can change.
	TCallHubList callList;
	std::copy(m_lstHub.begin(), m_lstHub.end(), std::front_inserter(callList));

	// Bump the ref count on each call so they stick around while we make these calls.
	std::for_each(callList.begin(), callList.end(), std::mem_fun(&CTSPICallAppearance::AddRef));
	sLock.Unlock();

	// Now enumerate through the list
	TCallHubList::const_iterator itEnd = callList.end();
	for (TCallHubList::const_iterator it = callList.begin(); it != itEnd; ++it)
	{
		// Get the call and inform it that a shadow has changed state.
		CTSPICallAppearance* pOtherCall = (*it);
		if (pOtherCall != pCall)
			pOtherCall->OnShadowCallStateChange(pCall, dwState, dwCurrState);
	}

	// Now dec the ref count; calls may be deleted by this.
	std::for_each(callList.begin(), callList.end(), std::mem_fun(&CTSPICallAppearance::DecRef));

}// CTSPICallHub::OnCallStateChange

///////////////////////////////////////////////////////////////////////////
// CTSPICallHub::GetCall
//
// Return the call at the index specified
//
CTSPICallAppearance* CTSPICallHub::GetCall(unsigned int i) const
{   
	CEnterCode sLock(this);  // Synch access to object
    if (i < m_lstHub.size())                                      
	{
		TCallHubList::const_iterator end = m_lstHub.end();
		for (TCallHubList::const_iterator iPos = m_lstHub.begin(); iPos != end; ++iPos, --i)
			if (i == 0) 
				return (*iPos);
	}
    return NULL;        

}// CTSPICallHub::GetCall

///////////////////////////////////////////////////////////////////////////
// CTSPICallHub::GetShadowCall
//
// Return the call which is opposite the given call. This returns NULL
// if more than two calls are in the array
//
CTSPICallAppearance* CTSPICallHub::GetShadowCall(const CTSPICallAppearance* pCall) const
{
	CEnterCode Lock(this);
	if (m_lstHub.size() == 2 && IsCallInHub(pCall))
	{
		TCallHubList::const_iterator it = std::find_if(m_lstHub.begin(), m_lstHub.end(),
			std::bind2nd(std::not_equal_to<const CTSPICallAppearance*>(),pCall));
		return (it != m_lstHub.end()) ? (*it) : NULL;
	}
	return NULL;

}// CTSPICallHub::GetShadowCall

#ifdef _DEBUG
///////////////////////////////////////////////////////////////////////////
// CTSPICallHub::Dump
//
// Debug Dump for the call hub
//
TString CTSPICallHub::Dump() const
{
	TStringStream outstm;

	CEnterCode sLock(this, FALSE);
	outstm << _T("0x") << hex << (DWORD)this;
	outstm << _T(",HubCallID=0x") << hex << m_dwCallID;

	// Only dump the conferenced parties if we can lock the object right now.
	// If not, ignore it and continue. This particular function is called during
	// call removal and causes a deadlock if another call is changing state at 
	// this moment in the hub. (V3.043)
	if (sLock.Lock(0))
	{
		outstm << _T(",Count=0x") << m_lstHub.size() << endl;
		int i = 1;
		for (TCallHubList::const_iterator iPos = m_lstHub.begin(); iPos != m_lstHub.end(); ++iPos)
			outstm << _T("  ") << i++ << _T(":") << (*iPos)->Dump() << endl;
	}
	else outstm << endl;
	
	return(outstm.str());

}// CTSPICallHub::Dump
#endif