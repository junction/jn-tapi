#ifndef ONSIP_THREAD_H
#define ONSIP_THREAD_H

#include "onsipxmpp.h"
#include "threads.h"
#include "onsip.h"

// Thread wrapper around OnSipXMPP.
// Runs OnSipXmpp within a thread to keep
// the Xmpp communication and state machines going.
class OnSipThread : public MyThread
{
private:
	auto_ptr<OnSipXmpp> m_xmpp;
	LoginInfo m_loginInfo;
	CriticalSection m_cs;

protected:

	// Main thread method
	virtual DWORD Run();

	// Virtual from within thread to create an instance
	// of the OnSipXmpp. Can be taken over to
	// create difference instance or provide an initialized
	// instance.
	virtual OnSipXmpp *CreateOnSipXMPP();

public:
	// Creates thread and OnSipXmpp using LoginInfo,
	// but created in suspended state.
	// Call start to get things going.
	OnSipThread(LoginInfo& loginInfo);

	void Start();

	// THREAD-SAFE
	//
	// Request to dial the specified number.
	// Returns the unique call-id to track the call.
	// Returns 0 if error.
	long MakeCall(const tstring& number);

	// THREAD-SAFE
	//
	// Request to drop the specified call
	void DropCall(long callId);

};

#endif
