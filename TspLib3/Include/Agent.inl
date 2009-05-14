/******************************************************************************/
//                                                                        
// AGENT.INL - TAPI Service Provider C++ Library header                     
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

#ifndef _SPAGENT_INL_INC_
#define _SPAGENT_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// RTSetAgentGroup
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// RTSetAgentGroup::RTSetAgentGroup
//
// Constructor for the lineSetAgentGroup request.
//
TSP_INLINE RTSetAgentGroup::RTSetAgentGroup(CTSPIAddressInfo* pAddr, 
			DRV_REQUESTID dwRequestID, TAgentGroupArray* pGroups) :
	CTSPIRequest(_T("lineSetAgentGroup")), m_parrGroups(pGroups)
{
	CTSPIRequest::Init(pAddr->GetLineOwner(), pAddr, NULL, REQUEST_SETAGENTGROUP, dwRequestID);

}// RTSetAgentGroup::RTSetAgentGroup

///////////////////////////////////////////////////////////////////////////
// RTSetAgentGroup::GetCount
//
// Returns the count of groups in our array
//
TSP_INLINE unsigned int RTSetAgentGroup::GetCount() const
{
	return (m_parrGroups) ? m_parrGroups->size() : 0;

}// RTSetAgentGroup::GetCount

///////////////////////////////////////////////////////////////////////////
// RTSetAgentGroup::GetGroup
//
// Returns the specified group index
//
TSP_INLINE const TAgentGroup* RTSetAgentGroup::GetGroup(unsigned int i) const
{
	return (m_parrGroups && i < m_parrGroups->size()) ? m_parrGroups->at(i) : NULL;

}// RTSetAgentGroup::GetGroup

///////////////////////////////////////////////////////////////////////////
// RTSetAgentGroup::GetGroupArray
//
// Returns the group array pointer
//
TSP_INLINE TAgentGroupArray* RTSetAgentGroup::GetGroupArray()
{
	return m_parrGroups;

}// RTSetAgentGroup::GetGroupArray

/******************************************************************************/
//
// RTSetAgentState
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// RTSetAgentState::RTSetAgentState
//
// Request modeling the lineSetAgentState
//
TSP_INLINE RTSetAgentState::RTSetAgentState(CTSPIAddressInfo* pAddr, 
		DRV_REQUESTID dwRequestID, DWORD dwState, DWORD dwNextState) :
	CTSPIRequest(_T("lineSetAgentState")), m_dwState(dwState), m_dwNextState(dwNextState)
{
	CTSPIRequest::Init(pAddr->GetLineOwner(), pAddr, NULL, REQUEST_SETAGENTSTATE, dwRequestID);

}// RTSetAgentState::RTSetAgentState

///////////////////////////////////////////////////////////////////////////
// RTSetAgentState::GetAgentState
//
// Returns the state of the agent
//
TSP_INLINE DWORD RTSetAgentState::GetAgentState() const
{
	return m_dwState;

}// RTSetAgentState::GetAgentState

///////////////////////////////////////////////////////////////////////////
// RTSetAgentState::GetNextAgentState
//
// Returns the next state of the agent
//
TSP_INLINE DWORD RTSetAgentState::GetNextAgentState() const
{
	return m_dwNextState;

}// RTSetAgentState::GetNextAgentState

///////////////////////////////////////////////////////////////////////////
// RTSetAgentState::SetAgentState
//
// Sets the new state of the request block - will be placed into
// the agent if the request completes successfully
//
TSP_INLINE void RTSetAgentState::SetAgentState(DWORD dwState)
{
	m_dwState = dwState;

}// RTSetAgentState::SetAgentState

///////////////////////////////////////////////////////////////////////////
// RTSetAgentState::SetNextAgentState
//
// Sets the new state of the request block - will be placed into
// the agent if the request completes successfully
//
TSP_INLINE void RTSetAgentState::SetNextAgentState(DWORD dwState)
{
	m_dwNextState = dwState;

}// RTSetAgentState::SetNextAgentState

/******************************************************************************/
//
// RTSetAgentActivity
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// RTSetAgentActivity::RTSetAgentActivity
//
// Request modeling the lineSetAgentActivity
//
TSP_INLINE RTSetAgentActivity::RTSetAgentActivity(CTSPIAddressInfo* pAddr, 
				DRV_REQUESTID dwRequestID, DWORD dwActivity) :
	CTSPIRequest(_T("lineSetAgentActivity")), m_dwActivityID(dwActivity)
{
	CTSPIRequest::Init(pAddr->GetLineOwner(), pAddr, NULL, REQUEST_SETAGENTACTIVITY, dwRequestID);

}// RTSetAgentActivity::RTSetAgentActivity

///////////////////////////////////////////////////////////////////////////
// RTSetAgentActivity::GetActivity
//
// Returns the activity of the agent
//
TSP_INLINE DWORD RTSetAgentActivity::GetActivity() const
{
	return m_dwActivityID;

}// RTSetAgentActivity::GetActivity

/******************************************************************************/
//
// RTAgentSpecific
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// RTAgentSpecific::RTAgentSpecific
//
// Request modeling the lineAgentSpecific
//
TSP_INLINE RTAgentSpecific::RTAgentSpecific(CTSPIAddressInfo* pAddr,
			DRV_REQUESTID dwRequestID, DWORD dwAgentExtensionID, LPVOID lpvBuff, DWORD dwSize) :
	CTSPIRequest(_T("lineSetAgentSpecific")), m_dwAgentExtensionIDIndex(dwAgentExtensionID)
{
	CTSPIRequest::Init(pAddr->GetLineOwner(), pAddr, NULL, REQUEST_AGENTSPECIFIC, dwRequestID);
	m_lpData.SetPtr(lpvBuff, dwSize);

}// RTAgentSpecific::RTAgentSpecific

///////////////////////////////////////////////////////////////////////////
// RTAgentSpecific::GetExtensionID
//
// Returns the extension ID specified with this AgentSpecific request
//
TSP_INLINE DWORD RTAgentSpecific::GetExtensionID() const
{
	return m_dwAgentExtensionIDIndex;

}// RTAgentSpecific::GetExtensionID

///////////////////////////////////////////////////////////////////////////
// RTAgentSpecific::GetBuffer
//
// Returns the user-supplied buffer
//
TSP_INLINE LPVOID RTAgentSpecific::GetBuffer()
{
	return m_lpData.GetWPtr();

}// RTAgentSpecific::GetBuffer

///////////////////////////////////////////////////////////////////////////
// RTAgentSpecific::GetBufferSize
//
// Returns the user-supplied buffer size
//
TSP_INLINE DWORD RTAgentSpecific::GetBufferSize() const
{
	return m_lpData.GetSize();

}// RTAgentSpecific::GetBufferSize

#endif // _SPAGENT_INL_INC_
