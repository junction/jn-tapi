/******************************************************************************/
//                                                                        
// CALLHUB.INL - Source code for the CTSPICallHub object.
//                      
// Copyright (C) 1994-1999 JulMar Entertainment Technology, Inc.
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

#ifndef _SPCALLHUB_INL_INC_
#define _SPCALLHUB_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// CTSPICallHub
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPICallHub::GetHubCount
//
// Returns the number of calls in our hub
//
TSP_INLINE unsigned int CTSPICallHub::GetHubCount() const
{
	CEnterCode sLock(this);  // Synch access to object
	return m_lstHub.size();

}// CTSPICallHub::GetHubCount

///////////////////////////////////////////////////////////////////////////
// CTSPICallHub::IsCallInHub
//
// Return true/false whether a call is in the hub
//
TSP_INLINE bool CTSPICallHub::IsCallInHub(const CTSPICallAppearance* pCall) const
{
	// Quick sanity check - match callids.
	if (pCall->GetCallID() == m_dwCallID)
	{
		CEnterCode sLock(this);  // Synch access to object
		return (std::find(m_lstHub.begin(), m_lstHub.end(), pCall) != m_lstHub.end());
	}
	return false;

}// CTSPICallAppearance::IsCallInHub

#endif // _SPCALLHUB_INL_INC_
