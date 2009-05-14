/******************************************************************************/
//                                                                        
// UIAGENT.INL - TAPI Service Provider C++ Library header                     
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

#ifndef _UIAGENT_INL_INC_
#define _UIAGENT_INL_INC_

#ifndef _NOINLINES_
#define TSP_INLINE inline
#else
#define TSP_INLINE
#endif

/******************************************************************************/
//
// CTSPUIDevice class functions
//
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::AddAgentActivity
//
// Add a new agent activity to our array
//
TSP_INLINE unsigned int CTSPUIDevice::AddAgentActivity(DWORD dwID, LPCTSTR pszName)
{
	ASSERT(DoesAgentActivityExist(dwID) == false);

	tsplibui::TAgentActivity* pAct = new tsplibui::TAgentActivity;
	pAct->dwID = dwID;
	pAct->strName = pszName;
	return m_arrActivity.Add(pAct);

}// CTSPUIDevice::AddAgentActivity

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::AddAgentActivity
//
// Add a new agent activity to our array
//
TSP_INLINE unsigned int CTSPUIDevice::AddAgentActivity(tsplibui::TAgentActivity* pAct)
{
	ASSERT(DoesAgentActivityExist(pAct->dwID) == false);
	return m_arrActivity.Add(static_cast<void*>(pAct));

}// CTSPUIDevice::AddAgentActivity

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::GetAgentActivityCount
//
// Returns the count of activity records in our array
//
TSP_INLINE unsigned int CTSPUIDevice::GetAgentActivityCount() const
{
	return m_arrActivity.GetSize();

}// CTSPUIDevice::GetAgentActivityCount

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::RemoveAgentActivity
//
// Remove an agent activity record - does NOT delete the object!
//
TSP_INLINE void CTSPUIDevice::RemoveAgentActivity(DWORD dwID)
{
	for (unsigned int i = 0; i < GetAgentActivityCount(); i++)
	{
		const tsplibui::TAgentActivity* pAct = GetAgentActivity(i);
		if (pAct->dwID == dwID)
		{
			m_arrActivity.RemoveAt(i);
			break;
		}
	}

}// CTSPUIDevice::RemoveAgentActivity

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::GetAgentActivity
//
// Returns a specific agent activity by position
//
TSP_INLINE tsplibui::TAgentActivity* CTSPUIDevice::GetAgentActivity(unsigned int iPos) const
{
	if (iPos < GetAgentActivityCount())
		return static_cast<tsplibui::TAgentActivity*>(m_arrActivity[iPos]);
	return NULL;

}// CTSPUIDevice::GetAgentActivity

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::DoesAgentActivityExist
//
// Return true/false whether an activity exists
//
TSP_INLINE bool CTSPUIDevice::DoesAgentActivityExist(DWORD dwActivity)
{
	return (GetAgentActivityById(dwActivity).IsEmpty() == FALSE);

}// CTSPUIDevice::DoesAgentActivityExist

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::GetAgentActivityById
//
// Returns an activity record by the activity id
//
TSP_INLINE CString CTSPUIDevice::GetAgentActivityById(DWORD dwID)
{
	for (unsigned int i = 0; i < GetAgentActivityCount(); i++)
	{
		const tsplibui::TAgentActivity* pAct = GetAgentActivity(i);
		if (pAct->dwID == dwID)
			return pAct->strName;
	}
	return _T("");

}// CTSPUIDevice::GetAgentActivityById

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::AddAgentGroup
//
// Add a new agent group to our internal array
//
TSP_INLINE int CTSPUIDevice::AddAgentGroup (LPCTSTR pszName, DWORD dwGroupID1, DWORD dwGroupID2, DWORD dwGroupID3, DWORD dwGroupID4)
{
	ASSERT (DoesAgentGroupExist(dwGroupID1,dwGroupID2,dwGroupID3,dwGroupID4) == FALSE);

	tsplibui::TAgentGroup* pGroup = new tsplibui::TAgentGroup;
	pGroup->strName = pszName;
	pGroup->GroupID.dwGroupID1 = dwGroupID1;
	pGroup->GroupID.dwGroupID2 = dwGroupID2;
	pGroup->GroupID.dwGroupID3 = dwGroupID3;
	pGroup->GroupID.dwGroupID4 = dwGroupID4;
	return m_arrGroups.Add(pGroup);

}// CTSPUIDevice::AddAgentGroup

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::GetAgentGroupCount
//
// Returns the agent group count
//
TSP_INLINE unsigned int CTSPUIDevice::GetAgentGroupCount() const
{
	return m_arrGroups.GetSize();

}// CTSPUIDevice::GetAgentGroupCount

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::RemoveAgentGroup
//
// Removes an agent group record
//
TSP_INLINE void CTSPUIDevice::RemoveAgentGroup(DWORD dwGroupID1, DWORD dwGroupID2, DWORD dwGroupID3, DWORD dwGroupID4)
{
	for (unsigned int i = 0; i < GetAgentGroupCount(); i++)
	{
		const tsplibui::TAgentGroup* pGroup = GetAgentGroup(i);
		if (pGroup->GroupID.dwGroupID1 == dwGroupID1 &&
			pGroup->GroupID.dwGroupID2 == dwGroupID2 &&
			pGroup->GroupID.dwGroupID3 == dwGroupID3 &&
			pGroup->GroupID.dwGroupID4 == dwGroupID4)
		{
			m_arrGroups.RemoveAt(i);
			return;
		}
	}

}// CTSPUIDevice::RemoveAgentGroup

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::GetAgentGroup
//
// Returns an agent group record by position
//
TSP_INLINE tsplibui::TAgentGroup* CTSPUIDevice::GetAgentGroup(unsigned int iPos) const
{
	if (iPos < GetAgentGroupCount())
		return static_cast<tsplibui::TAgentGroup*>(m_arrGroups[iPos]);
	return NULL;

}// CTSPUIDevice::GetAgentGroup

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::DoesAgentGroupExist
//
// Returns true/false whether a specific agent group exists
//
TSP_INLINE bool CTSPUIDevice::DoesAgentGroupExist(DWORD dwGroupID1, DWORD dwGroupID2, DWORD dwGroupID3, DWORD dwGroupID4)
{
	for (unsigned int i = 0; i < GetAgentGroupCount(); i++)
	{
		const tsplibui::TAgentGroup* pGroup = GetAgentGroup(i);
		if (pGroup->GroupID.dwGroupID1 == dwGroupID1 &&
			pGroup->GroupID.dwGroupID2 == dwGroupID2 &&
			pGroup->GroupID.dwGroupID3 == dwGroupID3 &&
			pGroup->GroupID.dwGroupID4 == dwGroupID4)
			return true;
	}
	return false;

}// CTSPUIDevice::DoesAgentGroupExist

///////////////////////////////////////////////////////////////////////////
// CTSPUIDevice::GetAgentGroupById
//
// Returns an agent group record by the group id
//
TSP_INLINE CString CTSPUIDevice::GetAgentGroupById(DWORD dwGroupID1, DWORD dwGroupID2, DWORD dwGroupID3, DWORD dwGroupID4)
{
	for (unsigned int i = 0; i < GetAgentGroupCount(); i++)
	{
		const tsplibui::TAgentGroup* pGroup = GetAgentGroup(i);
		if (pGroup->GroupID.dwGroupID1 == dwGroupID1 &&
			pGroup->GroupID.dwGroupID2 == dwGroupID2 &&
			pGroup->GroupID.dwGroupID3 == dwGroupID3 &&
			pGroup->GroupID.dwGroupID4 == dwGroupID4)
			return pGroup->strName;
	}
	return _T("");

}// CTSPUIDevice::GetAgentGroupById

#endif // _UIAGENT_INL_INC_
