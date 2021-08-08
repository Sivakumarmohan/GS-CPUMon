#pragma once

#include <stdint.h>

class Platform
{
public:
	static uint32_t GetCpuUsage();
	static uint32_t GetMemoryUsage();
};


