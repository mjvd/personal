#pragma once


// build for Win Server 2008 (and above)
#ifdef NTDDI_VERSION 
#error
#else
#define NTDDI_VERSION NTDDI_WS08
#endif

#ifdef _WIN32_WINNT
#error
#else
#define _WIN32_WINNT _WIN32_WINNT_WS08
#endif


#include <SDKDDKVer.h>

