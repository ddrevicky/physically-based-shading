#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
	#include <unistd.h>
#endif

#ifdef WIN32
	#define stat _stat
#endif

namespace IOUtil
{
	bool GetFileModificationTime(const char *filename, time_t *modificationTime);
}