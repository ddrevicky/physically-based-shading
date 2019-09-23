#include <iostream>

#include "IOUtil.h"

bool IOUtil::GetFileModificationTime(const char *filename, time_t *modificationTime)
{
	struct stat result;
	if (stat(filename, &result) != 0)
	{
		std::cerr << "Error calling stat on file " << filename << "\n";
		return false;
	}

	*modificationTime = result.st_mtime;
	return true;
}