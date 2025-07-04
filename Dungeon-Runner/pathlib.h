#ifndef PATHLIB_H
#define PATHLIB_H
#pragma once
#include <string.h>

#if defined(WIN32)
#  define PATHLIB_DIR_SEPARATOR '\\'
#else
#  define PATHLIB_DIR_SEPARATOR '/'
#endif

void path__join(const char *path1, const char *path2, char *destination);

#endif
