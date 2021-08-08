#include "Platform.h"

#ifndef _WIN32
#error "This file is Win32 specific."
#endif

#include <Windows.h>

uint32_t Platform::GetCpuUsage()
{
	MEMORYSTATUSEX memStat = { 0 };
	memStat.dwLength = sizeof( memStat );
	::GlobalMemoryStatusEx( &memStat );
	return uint32_t( memStat.dwMemoryLoad );
}

uint32_t Platform::GetMemoryUsage()
{
	static FILETIME prevIdleTime = { 0 };
	static FILETIME prevKernelTime = { 0 };
	static FILETIME prevUserTime = { 0 };

	FILETIME idleTime;
	FILETIME kernelTime;
	FILETIME userTime;
	BOOL ret = GetSystemTimes( &idleTime, &kernelTime, &userTime );

	__int64 idle = CompareFileTime( &prevIdleTime, &idleTime );
	__int64 kernel = CompareFileTime( &prevKernelTime, &kernelTime );
	__int64 user = CompareFileTime( &prevUserTime, &userTime );

	float rate = (kernel + user - idle) / (1.0 * (kernel + user));

	prevIdleTime = idleTime;
	prevKernelTime = kernelTime;
	prevUserTime = userTime;

	return uint32_t( rate );
}
