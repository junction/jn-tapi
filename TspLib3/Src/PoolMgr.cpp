/******************************************************************************/
//                                                                        
// POOLMGR.CPP - TSP++ Pool manager template support
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

/*-------------------------------------------------------------------------------*/
// INCLUDE FILES
/*-------------------------------------------------------------------------------*/
#include "stdafx.h"
#include <process.h>
#include "poolmgr.h"

/*****************************************************************************
** Procedure:  PoolThread
**
** Arguments:  'pMgr' - Pool manager object owner
**
** Returns:    void
**
** Description:  Worker thread function which causes the runner to execute
**
*****************************************************************************/
unsigned __stdcall tsplib_PoolThread(void* pParam)
{
	_TSP_DTRACEX(TRC_THREADS, _T("WorkerPoolThread(0x%lx) starting\n"), GetCurrentThreadId());
	reinterpret_cast<CIntThreadMgr*>(pParam)->Runner();
	_TSP_DTRACEX(TRC_THREADS, _T("WorkerPoolThread(0x%lx) ending\n"), GetCurrentThreadId());
	_endthreadex(0);
	return 0;

}// PoolThread

