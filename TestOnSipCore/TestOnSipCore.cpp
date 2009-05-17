// TestOnSipCore.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "OnSipXmpp.h"
#include "conio.h"
#include <sstream>
#include "OnSipThread.h"
#include <stdlib.h>

int _tmain(int argc, _TCHAR* argv[])
{
	Logger::SetConsoleLevel( Logger::LEVEL_DEBUG );
	Logger::SetWin32Level( Logger::LEVEL_DEBUG );

	LoginInfo logon;

	if ( argc == 4 )
	{
	  Logger::log_debug("using %s command line values", argv[1]);
	  logon = LoginInfo(argv[1],argv[2],argv[3]);
	}
#if _DEBUG
	else
	{
	  printf("use rlangham default value\n");
	  logon = LoginInfo("ronscottlangham","boating12","rlangham.onsip.com");
	}
#else
	else
	{
	  printf("must pass login...\r\n   name  password  domain   \r\nvalues as command line parameters\r\n\r\n");
	  printf("e.g.     Console  bob password  bob.onsip.com  \n");
	  return -1;
	}
#endif

	OnSipThread thread(logon);
	thread.Start();

	// Stay in thread until empty line input
	while (true)
	{
		char buffer[1000];
		gets(buffer);
		if ( buffer[0] == 0 )
			break;
		Logger::log_debug(_T("MAIN input:%s"),buffer);
		long callId = 0;
		// If starts with "-", then it is callid to drop
		if ( buffer[0] == '-' )
		{
			callId = atoi(buffer+1);
		}
		if ( callId == 0 )
		{
			thread.MakeCall(buffer);
			Logger::log_debug( _T("MAIN MakeCall callId=%ld"), callId );
		}
		else
		{
			Logger::log_debug( _T("MAIN DropCall callid=%ld"), callId );
			thread.DropCall(callId);
		}
	}

	// Stop XMPP thread
	Logger::log_debug(_T("MAIN signalStop"));
	thread.SignalStop();
	Logger::log_debug(_T("MAIN waiting stop.."));
	thread.Join(INFINITE);
	DWORD dwExitCode = thread.GetExitCode();
	Logger::log_debug(_T("MAIN stopped. exitCode=%d"),dwExitCode);

	return 0;
}

