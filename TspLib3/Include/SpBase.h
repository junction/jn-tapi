/******************************************************************************/
//                                                                        
// SPBASE.H - Building block objects for TSP++
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

#ifndef _SPBASE_LIB_INC_
#define _SPBASE_LIB_INC_

#ifndef _SPLIB_INC_
	#error "SPLIB.H must be included before SPBASE.H"
#endif

/*----------------------------------------------------------------------------
    INCLUDE FILES
-----------------------------------------------------------------------------*/
#include <new>

////////////////////////////////////////////////////////////////////////////
// DwordAlignPtr
//
// Given an input pointer, return the nearest DWORD aligned address
// moving forward if necessary.
//
#ifndef _X86_
inline LPVOID DwordAlignPtr (LPVOID lpBuff)
{
	DWORD dw = reinterpret_cast<DWORD>(lpBuff);
	dw += 3;
	dw >>= 2;
	dw <<= 2;
	return (LPVOID) dw;

}// DwordAlignPtr
#else
#define DwordAlignPtr(p) p
#endif

////////////////////////////////////////////////////////////////////////////
// GetWStringFromVARBlock
//
// This function retrives a string from a VARSTRUCT where the offset/size
// are pointers into the block.
//
inline LPCWSTR GetWStringFromVARBlock(LPCVOID lpvBuff, DWORD dwSize, DWORD dwOffset)
{
	// Retrieve the string from the buffer
	if (dwSize > 0 && lpvBuff != NULL)
		return reinterpret_cast<LPCWSTR>(reinterpret_cast<const BYTE*>(lpvBuff)+dwOffset);
	return NULL;

}// GetWStringFromVARBlock

////////////////////////////////////////////////////////////////////////////
// AllocMem/FreeMem
//
// This function is globally used to allocate memory.  Most memory
// allocations performed within the library come through this single
// function.
//
// Note: these are MACROs to get around the __FILE__/__LINE__ 
// expansion within the .H file.
//
#define AllocMem(dwSize) (new BYTE[dwSize])
#define FreeMem(p) (delete [] (BYTE*)p)

/******************************************************************************/
// Compiler extension to remove the v-table initialization from the
// constructor/destructor of objects. This increases object creation
// performance, and in some cases can significantly reduce the size of
// the produced image if the vtable is not used.
//
// This should ONLY be used on base classes which cannot be instantiated
// by themselves and which do not make virtual function calls in their
// constructor/destructor.
/******************************************************************************/
#define TSP_NOVTABLE __declspec(novtable)

// DWORD alignment macro
#define ALIGN_VALUE(count) ((count + 3) & ~3)

// If using STLPort, then use the min/max templates from the std namespace.
// MSVC and Dinkumware leave them as macros.
#ifdef STLPORT
	using std::min;
	using std::max;
#endif

// Typedef our base containers -- this allows them to be replaced by some
// other container if desired.  You could, for example, use a hash_map class
// instead of std::map .. this tends to provide slightly better performance
// in many cases, but it is not included in the standard STL implementation.

#ifndef TSP_VECTOR_CLASS
#define tsp_vector std::vector
#else
#define tsp_vector TSP_VECTOR_CLASS
#endif

#ifndef TSP_MAP_CLASS
#define tsp_map std::map
#else
#define tsp_map TSP_MAP_CLASS
#endif

#ifndef TSP_LIST_CLASS
#define tsp_list std::list
#else
#define tsp_list TSP_LIST_CLASS
#endif

// Move to internal namespace to prevent name clashes
namespace tsplib
{
/******************************************************************************/
// Member functor template for void-returning functions
/******************************************************************************/
#if _MSC_VER < 1300
template<class _Ty>
class mem_fun_tv : public std::unary_function<_Ty *, int> {
public:
	explicit mem_fun_tv(void (_Ty::*_Pm)()) : _Ptr(_Pm) {}
	result_type operator()(_Ty *_P) const { ((_P->*_Ptr)()); return 0; }
private:
	void (_Ty::*_Ptr)();
};

template<class _Ty> 
inline mem_fun_tv<_Ty> mem_funv(void (_Ty::*_Pm)())
{ return (mem_fun_tv<_Ty>(_Pm)); }

template<class _Ty> 
inline mem_fun_tv<_Ty> mem_funcv(void (_Ty::*_Pm)() const)
{ typedef void (_Ty::*funcptr)(); return (mem_fun_tv<_Ty>(reinterpret_cast<funcptr>(_Pm))); }

template<class _Ty, class _A>
class mem_fun1_tv : public std::binary_function<_Ty *, _A, int> {
public:
	explicit mem_fun1_tv(void (_Ty::*_Pm)(_A)) : _Ptr(_Pm) {}
	result_type operator()(_Ty *_P, _A _Arg) const { ((_P->*_Ptr)(_Arg)); return 0; }
private:
	void (_Ty::*_Ptr)(_A);
};

template<class _Ty, class _A> 
inline mem_fun1_tv<_Ty, _A> mem_funv1(void (_Ty::*_Pm)(_A))
{ return (mem_fun1_tv<_Ty, _A>(_Pm)); }

#define MEM_FUNV tsplib::mem_funv
#define MEM_FUNV1 tsplib::mem_funv1
#define MEM_FUNCV tsplib::mem_funcv

#else

// VS.NET supports full form of mem_fun
#define MEM_FUNV std::mem_fun
#define MEM_FUNV1 std::mem_fun
#define MEM_FUNCV std::mem_fun

#endif

/******************************************************************************/
// compose_f_gx
//
// Composition adapter f(g(elem))
//
// This is the general form unary compose function.  It allows nested calls of
// unary predicates such that the result of calling predicate g() for elem is
// used as the input for predicate f().  The whole expression then operates as
// a unary predicate.
// 
/******************************************************************************/
template <class OP1, class OP2>
class compose_f_gx_t : public std::unary_function<typename OP2::argument_type, typename OP1::result_type>
{
private:
	OP1 op1; OP2 op2;
public:
	compose_f_gx_t(const OP1& o1, const OP2& o2) : op1(o1), op2(o2) {}
	typename OP1::result_type 
		operator()(const typename OP2::argument_type& x) const {
			return op1(op2(x));
		}
};

template <class OP1, class OP2>
inline compose_f_gx_t<OP1,OP2>
compose_f_gx(const OP1& o1, const OP2& o2) {
	return compose_f_gx_t<OP1,OP2>(o1,o2);
}

/******************************************************************************/
// inauto_ptr
//
// auto_ptr class for intrinsic data types (i.e. non UDT)
//
/******************************************************************************/
template<class _Ty>
class inauto_ptr 
{
public:
	typedef _Ty element_type;
	explicit inauto_ptr(_Ty *_P = 0) throw() : m_fOwner(_P != 0), m_pData(_P) {}
	inauto_ptr(const inauto_ptr<_Ty>& src) throw() : m_fOwner(src.m_fOwner), m_pData(src.release()) {}
	inauto_ptr<_Ty>& operator=(const inauto_ptr<_Ty>& src) throw()
	{
		if (this != &src) {
			if (m_pData != src.get()) {
				if (m_fOwner) 
					delete [] m_pData;
			m_fOwner = src.m_fOwner; 
			}
			else if (src.m_fOwner)
				m_fOwner = true, src.release();
			m_pData = src.release(); 
		}
		return (*this); 
	}

	~inauto_ptr() throw()
	{
		if (m_fOwner)
			delete [] m_pData; 
	}

	_Ty *get() const throw() { return m_pData; }
	_Ty& operator*() const throw() { return (*get()); }
	_Ty *release() const throw()
	{
		const_cast<inauto_ptr<_Ty>*>(this)->m_fOwner = false;
		return (m_pData); 
	}
	void reset(_Ty* pData = 0)
	{
		if (pData != m_pData)
			delete [] m_pData;
		m_fOwner = (pData != static_cast<_Ty*>(NULL));
		m_pData = pData; 
	}

// Class data
private:
	bool m_fOwner;
	_Ty *m_pData;
};

/******************************************************************************/
// is_tdigit
//
// Replaces the isdigit and iswdigit macro/functions. The iswdigit 
// macro/function has a problem when compiled into a library, it causes a
// duplicate symbol error against LIBCMTD in VC6 because we bind this to
// a ptrfun template (apparently the compiler is scoping the name to a
// function within the module and then exporting a linkage record against it
// because we are building a library). Work around this problem by defining
// our own function local to the library itself.
//
/******************************************************************************/
inline bool is_tdigit(TCHAR ch) 
	{return ((ch >= _T('0') && ch <= _T('9')) ? true : false);}

/******************************************************************************/
// STL function to substitute one element for another during a transform
/******************************************************************************/
template<typename _Ty>
class substitue : public std::unary_function<_Ty, _Ty> {
public:
	_Ty _A; _Ty _B;
	substitue(_Ty a, _Ty b) : _A(a), _B(b) {}
	result_type operator() (_Ty a) const {
		if (a == _A) return _B; return a;
	}
};

/******************************************************************************/
// Auto delete function used by the containers
/******************************************************************************/
struct delete_object {
	template <typename _Ty>
	void operator()(_Ty* ptr) const { delete ptr; }
};

/******************************************************************************/
//
// Auto-deleting vector
//
// Our own private vector array object which deletes not only the
// pointers (keys) but also the objects referenced by the pointers when
// the destructor fires.
//
// Note this container requires that the contained data is allocated using
// the "new" operator (i.e. it requires pointers to data).
// INTERNAL DATA STRUCTURES
//
/******************************************************************************/
template<class _Ty>
class ptr_vector : public tsp_vector<_Ty*> 
{
public:
	ptr_vector() {}
	~ptr_vector() { destroy_all(); }
	ptr_vector(const ptr_vector<_Ty>& s) { copy_other(s); }
	_Ty& operator=(const _Ty& s) { if (this != &s) { destroy_all(); copy_other(s); } return (*this); }

private:
	void destroy_all() { std::for_each(begin(), end(), delete_object()); clear(); }
	void copy_other(const ptr_vector<_Ty>& s) {
		for (const_iterator it = s.begin(); it != s.end(); ++it) {
			const _Ty* p = (*it);
			push_back(new _Ty(*p));
		}
	}
};

/******************************************************************************/
//
// Auto-deleting list object
//
// Our own private linked-list object which deletes not only the
// pointers (keys) but also the objects referenced by the pointers when
// the destructor fires.
//
// INTERNAL DATA STRUCTURES
//
/******************************************************************************/
template<class _Ty>
class ptr_list : public tsp_list<_Ty*> {
public:
	ptr_list() {}
	~ptr_list() {	std::for_each(begin(), end(), delete_object()); }
private:
	ptr_list(const ptr_list<_Ty>& l);
	_Ty& operator=(const _Ty& _X);
};

/******************************************************************************/
//
// Auto-deleting map
//
// Our own private map object which deletes not only the
// pointers (keys) but also the objects referenced by the pointers when
// the destructor fires.
//
// INTERNAL DATA STRUCTURES
//
/******************************************************************************/
template<class _Ty, class _Tx>
class ptr_map : public tsp_map<_Ty,_Tx*> {
public:
	ptr_map() {}
	~ptr_map() {
		for (iterator it = begin(); it != end(); ++it)	
			delete (*it).second;
	}
private:
	ptr_map(const ptr_map<_Ty,_Tx>& l);
	_Ty& operator=(const _Ty& _X);
};

/******************************************************************************/
//
// STL algorithm to copy over matching predicate values
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
template<typename inputIterator, typename outputIterator, typename Predicate>
outputIterator copy_if(inputIterator begin, inputIterator end, outputIterator dest, Predicate p)
{
	while (begin != end) {
		if (p(*begin)) *dest++ = *begin;
		++begin;
	}
	return dest;
}

}; // end of tsplib namespace

/******************************************************************************/
//
// TMapDWordToString
//
// New map supporting DWORD indexing with string descriptions.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsp_map<DWORD, TString, std::less<DWORD> > TMapDWordToString;

/******************************************************************************/
//
// TCallMap
//
// New map supporting DWORD callid index to a call hub
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsp_map<DWORD, CTSPICallHub*> TCallMap;

/******************************************************************************/
//
// TConnectionMap
//
// New map supporting DWORD permanent line/phone identifiers to object.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsp_map<DWORD, CTSPIConnection*> TConnectionMap;

/******************************************************************************/
//
// TUIntArray
//
// Vector holding UINT values
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef std::vector<UINT> TUIntArray;

/******************************************************************************/
//
// TDWordArray
//
// Vector holding DWord values
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef std::vector<DWORD> TDWordArray;

/******************************************************************************/
//
// TStringArray and TStringList
//
// String array objects
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef std::vector<TString> TStringArray;
typedef tsp_list<TString> TStringList;

/******************************************************************************/
//
// TDeviceArray
//
// Auto-deleting array with device objects
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef	tsplib::ptr_vector<CTSPIDevice> TDeviceArray;

/******************************************************************************/
//
// TLineArray
//
// Auto-deleting array with line connection structures
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_vector<CTSPILineConnection> TLineArray;

/******************************************************************************/
//
// TAddressArray
//
// Auto-delete vector which contains address objects for a line.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_vector<CTSPIAddressInfo> TAddressArray;

/******************************************************************************/
//
// TPhoneArray
//
// Auto-deleting array with phone connection structures
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_vector<CTSPIPhoneConnection> TPhoneArray;

/******************************************************************************/
//
// TCallList
//
// Auto-deleting list which holds call information for an address.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_list<CTSPICallAppearance> TCallList;
typedef tsp_list<CTSPICallAppearance*> TCallHubList;

/******************************************************************************/
//
// CIntCriticalSection
//
// Our own private critical section which is used to gate access to the
// internal TSP data structures.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
class CIntCriticalSection
{
// Class data
protected:
	// Warning the first three LONGs must be DWORD-aligned!
	LONG    m_lInCS;			// Internal "gate"
	LONG    m_lLockCount;		// Total lock count
	LONG	m_lInThreadCount;	// Current thread owner count
	DWORD	m_dwThreadId;		// Current thread owner id	
	DWORD   m_dwLastUse;		// TickCount of event usage	
	HANDLE  m_hevtWait;			// Wait event created when needed
#ifdef _DEBUG
	DWORD   m_dwLastThreadId;	// Last thread id owner
	static int g_iEvtCount;		// Current event count
#endif

// Constructor
public:
	CIntCriticalSection();
	virtual ~CIntCriticalSection();

// Methods
public:
	virtual bool Lock(DWORD dwTimeout = INFINITE);
	virtual bool Unlock();

// Internal methods
protected:
	bool AquireSpinlock(DWORD dwMsecs = 0);
	void ReleaseSpinlock();
	bool BlockThread(DWORD dwTimeout);
	void UnblockWaiter();

// Locked members
private:
	CIntCriticalSection(const CIntCriticalSection& cs);
	CIntCriticalSection& operator=(const CIntCriticalSection& cs);
};

/******************************************************************************/
// 
// CTSPIBaseObject
//
// Basic object for each of the primary telephony objects in the library.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
class TSP_NOVTABLE CTSPIBaseObject
{
// Constructor
public:
	CTSPIBaseObject();
	CTSPIBaseObject(const CTSPIBaseObject& src);
	virtual ~CTSPIBaseObject();

// Scope methods
public:
	void AddRef() const;
	void DecRef() const;
#ifdef _DEBUG
	long GetRefCount() const;
#endif

// Internal methods
protected:
	virtual void DestroyObject();

// Access methods
public:
	// Internal critical section for multi-thread synchronization
	CIntCriticalSection* GetSyncObject() const;
	
	// Developer itemdata associated with each basic object
	DWORD GetItemData() const;
	void* GetItemDataPtr() const;
	void SetItemData(DWORD dwItem);
	void SetItemDataPtr(void* pItem);

#ifdef _DEBUG
	virtual TString Dump() const;
#endif

// Class data
private:
	DWORD m_dwItemData;						// Item data for developer use.
	mutable long m_lRefCount;				// Reference count for deletion
	mutable CIntCriticalSection m_csLock;	// Critical section associated with object
	void operator=(const CTSPIBaseObject&);	// Unavailable
};

/******************************************************************************/
//
// Safe Lock/Unlock object which automatically unlocks any object
// on the destructor.
//
/******************************************************************************/
class CEnterCode
{
// Class data
protected:
	const CTSPIBaseObject* m_pObject;
	int m_lLockCount;

// Constructor
public:
	CEnterCode(const CTSPIBaseObject* pObj, bool fAutoLock=true);
	~CEnterCode();

// Methods
public:
	bool Lock(DWORD dwMsecs = INFINITE);
	void Unlock();

// Locked members
private:
	CEnterCode(const CEnterCode& ec);
	CEnterCode& operator=(const CEnterCode& ec);
};

// This will cause the usage "CEnterCode(object)" to be invalid as
// that goes out of scope immediately and causes an unlock when one
// is not intended. This will cause an error which alerts the user
// to the problem.  The proper syntax is: "CEnterCode var(object)".
#define CEnterCode(x) __unnamed_LockErr

/******************************************************************************/
// CEnterCriticalSection
//
// This wraps an existing CRITICAL_SECTION for lock/unlock.  This object is
// not thread-safe .. use it only within one thread. 
//
// It is provided for exception safety and re-entrancy concerns with a 
// single critical section.
//
/******************************************************************************/
class CEnterCriticalSection
{
// Constructor
public:
	CEnterCriticalSection(CRITICAL_SECTION* pcs) : m_pcs(pcs), m_lLockCount(0) { Lock(); }
	virtual ~CEnterCriticalSection() { Unlock(); }

// Interface
public:
	void Lock() {
		if (m_lLockCount == 0) {
			++m_lLockCount;
			EnterCriticalSection(m_pcs);
		}
	}

	void Unlock() {
		if (m_lLockCount > 0) {
			m_lLockCount = 0;
			LeaveCriticalSection(m_pcs);
		}
	}

// Class data
private:
	CRITICAL_SECTION* m_pcs;
	long m_lLockCount;
};

/******************************************************************************/
// 
// Support for our polymorphic dynamic object creation.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
namespace TDynamicCreate
{    
	void *CreateObject(const char* pszClass);

	// Base factory object
    class tsplib_TObjFactory
	{    
		public:
			virtual void *CreateObject(const char* pszClass) = 0;
	};

    // Factory list manager - maintains list of active objects
	// which may be dynamically created within the library
    class tsplib_TFactoryListMgr    
	{
        friend void *CreateObject(const char* pszClass);    
        static tsplib_TFactoryListMgr* m_pHead;
        tsplib_TFactoryListMgr* m_pNext;
        tsplib_TObjFactory *m_pManufacturer;

		public:
			tsplib_TFactoryListMgr (tsplib_TObjFactory *pFactory);
			virtual ~tsplib_TFactoryListMgr();
    };

    inline tsplib_TFactoryListMgr::tsplib_TFactoryListMgr(tsplib_TObjFactory *pFactory)
	{
		m_pManufacturer = pFactory;
        m_pNext = m_pHead;
		m_pHead = this;
	}

    #pragma warning(disable:4355)
    template <class T> class TFactory : public tsplib_TObjFactory
    {
        tsplib_TFactoryListMgr m_Manufacturer;
        public:
			TFactory() : m_Manufacturer(this) {}
			virtual void *CreateObject(const char* pszClass)
			{
				std::string strClass = "class ";
				strClass += pszClass;
				return !::lstrcmpiA(typeid(T).name(), strClass.c_str())
						? (void*)( new T ) : (void*)0;
			}
	};
}

#define DECLARE_TSPI_OVERRIDE(n)\
TDynamicCreate::TFactory<n> g_fac##n\

/******************************************************************************/
// 
// DIALINFO
//
// This structure is used to store dialable number information for
// a destination.  The information is broken out by the method
// "CheckDialableAddress" and returned in a set of structure(s) for
// each address found in the passed string.
//
// This is passed in an array for several asynchronous requests.
//
/******************************************************************************/
class DIALINFO
{ 
public:
    TString strNumber;		// Final number to dial, includes "!PTW@$;"
    TString strName;		// Name pulled out of dial string (may be NULL)
    TString strSubAddress;	// Sub address information (ISDN) pulled out of dial string)
    bool fIsPartialAddress; // Address is "partial"

	DIALINFO() : fIsPartialAddress(false) {}
	DIALINFO(bool _fIsPartial, const TString& _strNumber, const TString& _strName, const TString& _strSub) :
		strNumber(_strNumber), strName(_strName), strSubAddress(_strSub), fIsPartialAddress(_fIsPartial) {}
	DIALINFO(const DIALINFO& pd) : 
		strNumber(pd.strNumber), strName(pd.strName), strSubAddress(pd.strSubAddress), fIsPartialAddress(pd.fIsPartialAddress) {}
	DIALINFO& operator=(const DIALINFO& di) {
		if (this != &di) {
			strNumber = di.strNumber;
			strName = di.strName;
			strSubAddress = di.strSubAddress;
			fIsPartialAddress = di.fIsPartialAddress;
		}
		return (*this);
	}	
};

/******************************************************************************/
//
// TDialStringArray
//
// Auto-deleting array with DIALINFO structures
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_vector<DIALINFO> TDialStringArray;

/******************************************************************************/
// 
// TSPIFORWARDINFO
//
// Single forwarding information structure.  This is placed into 
// the "ptrForwardInfo" array for each forward entry found in the
// forward list.
//
// The caller and destination addresses are stored in both input and
// our dialable form so that depending on what is required at the switch
// level is avialable to the derived class worker code.
//
/******************************************************************************/
class TSPIFORWARDINFO
{     
public:
    LONG lRefCount;						// Reference Count
    DWORD dwForwardMode;				// Forwarding mode (LINEFORWARDMODE_xxx)
    DWORD dwDestCountry;				// Destination country for addresses
	DWORD dwTotalSize;					// Total size of all the forwarding information	
	TDialStringArray arrCallerAddress;	// Array of caller information
    TDialStringArray arrDestAddress;	// Array of destination information

	// Member functions
    TSPIFORWARDINFO();					// Constructor
    void IncUsage();					// Reference count manager function
    void DecUsage();					// Reference count manager function

// Locked members
private:
	TSPIFORWARDINFO(const TSPIFORWARDINFO& fi);
	TSPIFORWARDINFO& operator=(const TSPIFORWARDINFO& fi);
};

/******************************************************************************/
//
// TForwardInfoArray
//
// Non-deleting Array holding forwarding information
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsp_vector<TSPIFORWARDINFO*> TForwardInfoArray;

/******************************************************************************/
// 
// TSPIMEDIACONTROL
//
// This structure is passed to control media actions on a particular media stream.  
//
/******************************************************************************/
class TSPIMEDIACONTROL
{
public:
    typedef tsplib::ptr_vector<LINEMEDIACONTROLDIGIT> TDigitArray;
    typedef tsplib::ptr_vector<LINEMEDIACONTROLMEDIA> TMediaArray;
    typedef tsplib::ptr_vector<LINEMEDIACONTROLTONE> TToneArray;
	typedef tsplib::ptr_vector<LINEMEDIACONTROLCALLSTATE> TStateArray;

    TDigitArray arrDigits;		// Array of digit monitoring media structures
    TMediaArray arrMedia;		// Array of media monitoring media structures
    TToneArray arrTones;		// Array of tone monitoring media structures
	TStateArray arrCallStates;	// Array of callstate monitoring media structures
    long lRefCount;				// Reference count for shared structures

	// Member functions
    TSPIMEDIACONTROL();         // Constructor
    void DecUsage();            // Auto-delete mechanism after no more usage
    void IncUsage();            // Incremenent usage count

// Locked members
private:
	TSPIMEDIACONTROL(const TSPIMEDIACONTROL& fi);
	TSPIMEDIACONTROL& operator=(const TSPIMEDIACONTROL& fi);
};

/******************************************************************************/
//
// TIMEREVENT
//
// This is used by the call appearance to manage duration cases for
// different events.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
class TIMEREVENT
{                        
public:
    enum EventType { 
		MediaControlMedia = 1,	// Timer started by Media duration
		MediaControlTone,		// Timer started by Tone duration 
		ToneDetect				// Timer stated by tone detection.
	};
    int iEventType;             // Event type (EventType or user-defined)
    DWORD dwEndTime;            // Time this event expires (TickCount)
    DWORD dwData1;              // Data dependant upon event type.
    DWORD dwData2;              // Data dependant upon event type.

// Constructor
public:
	TIMEREVENT(int _eventtype, DWORD _dwEndTime, DWORD _dwData1=0, DWORD _dwData2=0) :
	  iEventType(_eventtype), dwEndTime(_dwEndTime), dwData1(_dwData1), dwData2(_dwData2) {/* */}
};

/******************************************************************************/
//
// TTimerEventArray
//
// Auto-deleting array which timer events for a line object
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_vector<TIMEREVENT> TTimerEventArray;

/******************************************************************************/
//
// CALLIDENTIFIER
//
// This structure defines a call which is connected to one of our call 
// appearances as a desintation.  It is used to provider CALLERID information
// to the CALLINFO structure.
//       
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
class CALLIDENTIFIER
{
public:
    TString strPartyId;            // Party id number information
    TString strPartyName;          // Name of party
	CALLIDENTIFIER& operator=(const CALLIDENTIFIER& src);
};

/******************************************************************************/
// 
// TERMINALINFO
//
// This structure defines a terminal to our line connection.  Each added
// terminal will have a structure of this type added to the 'arrTerminals'.
// list in the line connection.  Each address/call will have a DWORD list
// describing the mode of the terminal info (superceding the dwMode here).
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef struct
{
    TString strName;			// Name of the terminal
    DWORD dwMode;				// Base line terminal mode
    LINETERMCAPS Capabilities;  // Capabilities of the terminal
} TERMINALINFO;

/******************************************************************************/
//
// TTerminalInfoArray
//
// Auto-deleting vector which contains TERMINFO structures
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_vector<TERMINALINFO> TTerminalInfoArray;

/******************************************************************************/
//
// SIZEDDATA
//
// This is a structure with an embedded void* and size.  It may be used
// to store a variety of data.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
class SIZEDDATA
{
// Class data
public:
	DWORD m_dwSize;					// Size of the data
	tsplib::inauto_ptr<BYTE> m_lpvData;		// Pointer to the data

// Constructors (default, copy, and parameterized)
public:
	SIZEDDATA() : m_dwSize(0) {}
	SIZEDDATA(const SIZEDDATA& sd) : m_dwSize(0) { SetPtr(sd.m_lpvData.get(), sd.m_dwSize); }
	SIZEDDATA(LPCVOID lpvData, DWORD dwSize) : m_dwSize(dwSize), m_lpvData(new BYTE[dwSize]) {
		CopyMemory(m_lpvData.get(), lpvData, dwSize);
	}

// Destructor
public:
	virtual ~SIZEDDATA() {};

// Operators (assigment, LPCVOID)
public:
	SIZEDDATA& operator=(const SIZEDDATA& sd) { SetPtr(sd.m_lpvData.get(), sd.m_dwSize); return *this; }
    operator LPCVOID() const { return GetPtr(); }

// Methods
public:
	DWORD GetSize() const { return m_dwSize; }
	LPCVOID GetPtr() const { return reinterpret_cast<LPCVOID>(m_lpvData.get()); }
	LPVOID GetWPtr() { return reinterpret_cast<LPVOID>(m_lpvData.get()); }
	void SetPtr(LPCVOID lpvData, DWORD dwSize) {
		if (dwSize > 0)	{
			_TSP_ASSERTE(lpvData != NULL);
			_TSP_ASSERTE(!IsBadReadPtr(lpvData, dwSize));
			m_lpvData.reset(new BYTE[dwSize]);
			CopyMemory(m_lpvData.get(), lpvData, dwSize);
		}
		else m_lpvData.reset();
		m_dwSize = dwSize;
	}
};

/******************************************************************************/
//
// TUserInfoArray
//
// Auto-deleting vector which contains USERUSERINFO structures
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_vector<SIZEDDATA> TUserInfoArray;

/******************************************************************************/
//
// DEVICECLASSINFO
//
// This structure describes a device-class information record which can
// be retrieved through the lineGetID and phoneGetID apis.  Each line, address,
// call, and phone, have an array which holds these structures.
//
/******************************************************************************/
class DEVICECLASSINFO
{
public:
	TString	strName;		// Name of the device class ("tapi/line")
	DWORD dwStringFormat;	// String format (STRINGFORMAT_xxx)
	DWORD dwSize;			// Size of data
	HANDLE hHandle;			// Handle which is COPIED
	tsplib::inauto_ptr<BYTE> lpvData; // Data (may be NULL)
	DEVICECLASSINFO(LPCTSTR pszName, DWORD dwStringFormat, LPVOID lpData=NULL, DWORD dwSize=0, HANDLE hHandle=INVALID_HANDLE_VALUE);
	~DEVICECLASSINFO();

// Locked members
private:
	DEVICECLASSINFO(const DEVICECLASSINFO& dci);
	DEVICECLASSINFO& operator=(const DEVICECLASSINFO& dci);
};

/******************************************************************************/
//
// TDeviceClassArray
//
// Auto-deleting map with DEVICECLASSINFO structures
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_map<TString, DEVICECLASSINFO> TDeviceClassArray;

/******************************************************************************/
// 
// LINEUIDIALOG
//
// This structure is used to manage dynamically created dialogs
// so that we can track them.  It is stored in an array maintained
// by the line connection object.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
class LINEUIDIALOG
{
public:
	DRV_REQUESTID dwRequestID;			// Request ID which generated this dialog.
	HTAPIDIALOGINSTANCE htDlgInstance;	// TAPI's handle to dialog	
	CTSPILineConnection* pLineOwner;	// Owner of UI dialog
	LPVOID lpvItemData;					// Item data to pass to derived class

// Constructor
public:
	LINEUIDIALOG() : dwRequestID(0), htDlgInstance(0), pLineOwner(0), lpvItemData(0) {}

// Locked members
private:
	LINEUIDIALOG(const LINEUIDIALOG& dci);
	LINEUIDIALOG& operator=(const LINEUIDIALOG& dci);
};

/******************************************************************************/
//
// TUIList
//
// Auto-deleting list which contains information concerning active
// UI dialogs on client systems.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_vector<LINEUIDIALOG> TUIList;

/******************************************************************************/
//
// TSPIDIGITGATHER
//
// This structure is passed to manage digit gathering. It is stored in
// the CTSPICallAppearance object.
//
/******************************************************************************/
class TSPIDIGITGATHER
{
public:
    DWORD dwEndToEndID;                 // Unique identifier for this request to TAPI.
    DWORD dwDigitModes;                 // LINEDIGITMODE_xxx 
    DWORD dwSize;                       // Number of digits before finished
    DWORD dwCount;                      // Count of digits we have placed in the buffer.
    DWORD dwFirstDigitTimeout;          // mSec timeout for first digit
    DWORD dwInterDigitTimeout;          // mSec timeout between any digits
    DWORD dwLastTime;                   // Last mSec when digit seen.
    LPWSTR lpBuffer;                    // Buffer for the call to place collected digits
    TString strTerminationDigits;		// Digits which will terminate gathering.
    TSPIDIGITGATHER();                  // Constructor

// Locked members
private:
	TSPIDIGITGATHER(const TSPIDIGITGATHER& dg);
	TSPIDIGITGATHER& operator=(const TSPIDIGITGATHER& dg);
};

/******************************************************************************/
//
// TGenerateToneArray
//
// Auto-deleting array which contains information about tones to generate.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_vector<LINEGENERATETONE> TGenerateToneArray;

/******************************************************************************/
// 
// TSPITONEMONITOR                                                                 
//
// This structure is used for tone monitoring on a call appearance.
//
/******************************************************************************/
class TSPITONEMONITOR
{
public:
    DWORD dwToneListID;							// Tone Identifier passed on LINEMONITORTONE msg.
	tsplib::ptr_vector<LINEMONITORTONE> arrTones;	// Tones to monitor for.
    TSPITONEMONITOR();							// Constructor for the object
private:
	TSPITONEMONITOR(const TSPITONEMONITOR&);
};    

/******************************************************************************/
//
// TMonitorToneArray
//
// Auto-deleting array which holds monitor events for lineMonitorTone.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef tsplib::ptr_vector<TSPITONEMONITOR> TMonitorToneArray;

/******************************************************************************/
//
// EXTENSIONID
//
// This is a combination of a LINEEXTENSIONID and PHONEEXTENSIONID structure
// so our CTSPIConnection object can manage both.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef struct 
{
    DWORD dwExtensionID0;
    DWORD dwExtensionID1;
    DWORD dwExtensionID2;
    DWORD dwExtensionID3;
} EXTENSIONID, *LPEXTENSIONID;

/******************************************************************************/
//
// TExtVersionInfo
//
// This pulls together all the extended version information so it can
// be stored in the line/phone objects as a dynamically allocated element.
// since many providers don't support this, it is only allocated when the
// provider uses the SetExtVersionInfo() method to create it.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
typedef struct
{
	DWORD dwMinExtVersion;		// Minimum extension version
	DWORD dwSelectedExtVersion;	// Negotiate extension version
	DWORD dwMaxExtVersion;		// Maximum extension version
	EXTENSIONID ExtensionID;		// Extension information
} TExtVersionInfo;

/******************************************************************************/
//
// LOCATIONINFO
//
// This pulls together all the required country information when the
// current location changes. It gives the provider enough information to
// convert from dialable to canonical format.
//
// INTERNAL DATA STRUCTURE
//
/******************************************************************************/
class LOCATIONINFO
{
// Private implementation
private:
	class LLHelper
	{
	public:
		LLHelper() 
		{
			m_hInstance = LoadLibraryA("TAPI32.DLL");
			pfnGetTranslateCaps = (LONG (WINAPI*)(HLINEAPP, DWORD, LPLINETRANSLATECAPS)) GetProcAddress(m_hInstance, "lineGetTranslateCapsA");
			pfnGetCountry = (LONG (WINAPI*)(DWORD, DWORD, LPLINECOUNTRYLIST)) GetProcAddress(m_hInstance, "lineGetCountryA");
		}
		~LLHelper() { if (m_hInstance) FreeLibrary(m_hInstance); }

		LINETRANSLATECAPS* tapiGetTranslateCaps(void) {
			LPLINETRANSLATECAPS lpCaps = NULL;
			DWORD dwSize = 1024;
			if (pfnGetTranslateCaps)
			{
				for(;;)
				{
					lpCaps = reinterpret_cast<LPLINETRANSLATECAPS>(AllocMem(dwSize));
					if (lpCaps == NULL) return NULL;
					ZeroMemory(lpCaps, dwSize);
					lpCaps->dwTotalSize = dwSize;
					if (pfnGetTranslateCaps(NULL, TAPIVER_20, lpCaps) != 0)
						return NULL;
					if (lpCaps->dwNeededSize > lpCaps->dwTotalSize) {
						dwSize = lpCaps->dwNeededSize;
						FreeMem(lpCaps);
					}
					else break;
				}
			}
			return lpCaps;
		}
		LINECOUNTRYLIST* tapiGetCountry(DWORD dwCountryID) {
			LPLINECOUNTRYLIST lpList = NULL;
			DWORD dwSize = 1024;
			if (pfnGetCountry)
			{
				for (;;)
				{
					lpList = reinterpret_cast<LPLINECOUNTRYLIST>(AllocMem(dwSize));
					if (lpList == NULL) return NULL;

					ZeroMemory(lpList, dwSize);
					lpList->dwTotalSize = dwSize;
					if (pfnGetCountry(dwCountryID, TAPIVER_20, lpList) != 0)
						return NULL;
					if (lpList->dwNeededSize > lpList->dwTotalSize) {
						dwSize = lpList->dwNeededSize;
						FreeMem(lpList);
					}
					else break;
				}
			}
			return lpList;
		}
	private:
		HMODULE m_hInstance;
		LONG (WINAPI* pfnGetTranslateCaps)(HLINEAPP, DWORD, LPLINETRANSLATECAPS);
		LONG (WINAPI* pfnGetCountry)(DWORD, DWORD, LPLINECOUNTRYLIST);
	};

// Class data
protected:
	bool m_fLoaded;
public:
	DWORD dwCurrentLocation;
	TString strCountryCode;
	TString strAreaCode;
	TString strIntlCode;
	TString strLocalAccess;
	TString strLongDistanceAccess;
	TString strCallWaiting;
	TString strNationalCode;

// Constructor
public:
	LOCATIONINFO() : dwCurrentLocation(0) { Reset(); }

// Methods
public:
	bool IsLoaded() const { return m_fLoaded; }
	void Reset() {
		m_fLoaded = false;
		strCountryCode = _T("1");
		strIntlCode = _T("011");
		strAreaCode = _T("");
		strCallWaiting = _T("");
		strLocalAccess = _T("");
		strLongDistanceAccess = _T("");
		strNationalCode = _T("");
	}
	bool Reload();

// Locked members
private:
	LOCATIONINFO(const LOCATIONINFO& li);
	LOCATIONINFO& operator=(const LOCATIONINFO& li);
};

#endif // _SPBASE_LIB_INC_
