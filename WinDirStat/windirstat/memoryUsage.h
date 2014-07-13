#include "stdafx.h"
#include <Windows.h>
#include <Psapi.h>
#include "globalhelpers.h"

CWinThread* startMemUsage( );

class MemoryUsage : public CWinThread {
	DECLARE_DYNCREATE( MemoryUsage );
protected:
	virtual BOOL InitInstance( );
	LONGLONG m_workingSet;// Current working set (RAM usage)
	unsigned long long m_lastPeriodicalRamUsageUpdate;// Tick count
	void LoopInfoTrace( );

	void UpdateInfo( );
public:
	CString GetMemoryInfoString( );
	//BOOL GetProcessMemoryInfo( _In_ HANDLE Process, _Inout_ PPROCESS_MEMORY_COUNTERS ppsmemCounters, _In_ DWORD cb );
private:
	std::future<void> nullFut;
	CString m_MemUsageCache;
	LONGLONG m_workingSetBefore;
	};