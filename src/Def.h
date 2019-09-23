#pragma once

#include <cinttypes>
#include <cassert>

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) \
			((sizeof(a) / sizeof(*(a))) / \
			static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))
#endif