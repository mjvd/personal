#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif

#ifndef UNICODE
#define UNICODE
#endif


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

#include <sdkddkver.h>

