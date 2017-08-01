// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#define NOMINMAX
#include <windows.h>

#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <set>
#include <atomic>
#include <thread>
#include <utility>
#include <algorithm>
#include <mutex>
#include <condition_variable>


extern std::string   global_out_path;
extern std::ofstream global_out_file;
extern std::ofstream global_out_file_stream;
extern bool    	     global_is_recording;
extern bool    	     global_is_recording_ps;

template<class X> void record_struct(const X &x)
{
	global_out_file.write((char*)(&x), sizeof(X));
}

template<class X> void ft_log__(X x)
{
	std::ofstream lf;
	lf.open("E:\\log.txt", std::ios_base::app);
	lf << x;
	lf.close();
}

void inline ft_clear()
{
	std::ofstream lf;
	lf.open("E:\\log.txt", std::ios_base::binary);
	lf.close();
}

//#define COUNTERS

#ifdef ENABLE_LOG
#define LOG(x) ft_log__(x)
#define CLEAR_LOG ft_clear()
#else
#define LOG(x)
#define CLEAR_LOG
#endif

#define FORCE_LOG(x) ft_log__(x)

template<class ... Args> void force_log_impl(std::ostream& os, Args... args)
{
	return;
}

template<class Arg, class ... Args> void force_log_impl(std::ostream& os, Arg a, Args... args)
{
	os << a;
	return force_log_impl(os, args...);
}

template<class ... Args> void force_log_i(Args... args)
{
	/*std::ofstream lf;
	lf.open("E:\\log.txt", std::ios_base::app);
	force_log_impl(lf, args...);
	lf.close();*/
}

#define force_log(x, ...)

extern LARGE_INTEGER ___ls;

inline int UpdateTimer()
{
	LARGE_INTEGER ll;
	QueryPerformanceCounter(&ll);
	int t = static_cast<int>(ll.QuadPart - ___ls.QuadPart);
	___ls = ll;
	return t;
}

#define DEBUG_MONITOR