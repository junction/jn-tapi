
#ifndef ONSIPGLOBAL_H
#define ONSIPGLOBAL_H

// Global defines for both TSP Provider and TSP UI

#define PROVIDER_NAME		_T("Junction Networks")

#define SWITCH_NAME 		_T("OnSIP Hosted PBX")

// Format for Line Name
#define LINE_NAME 		_T("OnSIP %s")

// Unique LINE ID used in Julmar framework to map our Line Device
#define LINE_ID_UNIQUE		1

// Registry Value Names

#define REG_SIPADDRESS	_T("SipAddress")
#define REG_PASSWORD	_T("Password")
#define REG_PHONENUMBER	_T("PhoneNumber")

// If this value is changed, it must match the value in OnSipUI.h EXACTLY
#define KEY_VALUE "JunctionNetworksOnSipHostPbx"

#endif
