========================================================================
       JULMAR TSP++ VERSION 3.0 CLASS LIBRARY : OnSip
========================================================================

TspWizard has created this OnSip TAPI Service Provider for you.  
It contains all the options you requested for your service provider and serves
as the starting point for writing your TSP.

This file contains a summary of what you will find in each of the files that
make up your OnSip TSP.

OnSip.dsp
    This file (the project file) contains information at the project level and
    is used to build a single project or subproject. Other users can share the
    project (.dsp) file, but they should export the makefiles locally.

OnSip.h
    This is the main header file for the TSP.  It includes other
    project specific headers (including Resource.h) and declares the
    required TSP++ objects.

OnSip.cpp
    This is the main application source file that contains the CServiceProvider
    override class COnSipServiceProvider.

OnSip.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes a basic version resource which is required for
	all TAPI service provider. This file can be directly edited in Microsoft
	Visual C++.

OnSip.def
	This is the Windows Definition file which details all the exported 
	functions from the service provider.

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named OnSip.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

device.cpp
	This is the CTSPIDevice override class COnSipDevice. This
	file contains all the generated device communication code.

line.cpp
	This is the CTSPILineConnection override class COnSipLine.
	This file contains the request map handler which details the call control
	requests which are processed by this service provider.

DropCall.cpp
	This file contains the code to process the TSPI_lineDrop request.

MakeCall.cpp
	This file contains the code to process the TSPI_lineMakeCall and TSPI_lineDial
	requests.

Unsolicited.cpp
	This file contains the unsolicited event handler for the service provider.

/////////////////////////////////////////////////////////////////////////////
Other notes:

TspWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

For several functions, AppWizard has included suggested code for you
to build upon. This code is simply a suggestion and in no way implies
that it is the only way to perform the required functionallity. You may
rewrite and customize any function in the created code.

Since the TSP has not been fleshed out with real code, it may not work
as expected right now or when modified. No warranty is given for the
generated code.

/////////////////////////////////////////////////////////////////////////////
