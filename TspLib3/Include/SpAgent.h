/******************************************************************************/
//                                                                        
// SPAGENT.H - Agent management functions for TSP++ V3
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

#ifndef _SPAGENT_LIB_INC_
#define _SPAGENT_LIB_INC_

#ifndef _SPLIB_INC_
	#error "SPLIB.H must be included before SPAGENT.H"
#endif

/******************************************************************************/
// TAgentCaps
//
// This structure maps all the agent capability information for a single address
//
/******************************************************************************/
typedef struct
{
	DWORD dwFeatures;				// Agent-related features LINEAGENTFEATURE_xxx
	DWORD dwStates;					// Agent states supported on this address
	DWORD dwNextStates;				// Next agent-states supported on this address
	DWORD dwAgentStatusMessages;	// Reported in LINEAGENTSTATE_xxx
	DWORD dwMaxNumGroupEntries;		// Max number of group entries
} TAgentCaps;

/******************************************************************************/
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
	TString strName;			// Name of group
} TAgentGroup;

/******************************************************************************/
// TAgentGroupArray
//
// Auto-deleting vector which holds agent groups
//
/******************************************************************************/
typedef tsplib::ptr_vector<TAgentGroup> TAgentGroupArray;

/******************************************************************************/
// TAgentStatus
//
// This structure contains the "edittable" agent's status information.
//
/******************************************************************************/
class TAgentStatus
{
public:
	DWORD dwAgentFeatures;		// Agent-related features LINEAGENTFEATURE_xxx
	DWORD dwState;				// Agent states supported on this address
	DWORD dwNextState;			// Next agent-states supported on this address
	DWORD dwActivityID;			// Current activity ID
	DWORD dwValidStates;		// Current valid states
	DWORD dwValidNextStates;	// Current valid "next" states	
	TAgentGroupArray arrGroups;	// Current "logged-on" groups.
	TAgentStatus() : 
		dwAgentFeatures(0), dwState(0), dwNextState(0), dwActivityID(0), 
		dwValidStates(0), dwValidNextStates(0)
	{/* */}
private:
	TAgentStatus(const TAgentStatus&);
};

/******************************************************************************/
// TAgentActivity
//
// This structure describes a single agent activity
//
/******************************************************************************/
typedef struct
{
	DWORD dwID;					// Activity identifier
	TString strName;			// Name
} TAgentActivity;

/******************************************************************************/
// TAgentActivityArray
//
// Auto-deleting vector which holds agent activities
//
/******************************************************************************/
typedef tsplib::ptr_vector<TAgentActivity> TAgentActivityArray;

/******************************************************************************/
// TAgentSpecificEntry
//
// This structure describes a single agent specific 128-bit index entry
//
/******************************************************************************/
#pragma pack(1)
typedef struct
{
	DWORD dwID1;
	DWORD dwID2;
	DWORD dwID3;
	DWORD dwID4;
} TAgentSpecificEntry;
#pragma pack()

/******************************************************************************/
// TAgentExtensionArray
//
// Auto-deleting vector which holds agent extensions
//
/******************************************************************************/
typedef tsplib::ptr_vector<TAgentSpecificEntry> TAgentExtensionArray;

#endif // _SPAGENT_LIB_INC_