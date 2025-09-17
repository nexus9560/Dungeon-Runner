#include "pathlib.h"

#ifndef PATHLIB_MAX_PATH
#define PATHLIB_MAX_PATH 260
#endif

void path__join(const char *path1, const char *path2, char *destination) {
  if (path1 && *path1) {
    size_t len = strlen(path1);
    #ifdef _WIN32
        strcpy_s(destination, PATHLIB_MAX_PATH, path1);
    #else
        strncpy(destination, path1, PATHLIB_MAX_PATH - 1);
        destination[PATHLIB_MAX_PATH - 1] = '\0';
    #endif

    if (destination[len - 1] == PATHLIB_DIR_SEPARATOR) {
      if (path2 && *path2) {
#ifdef _WIN32
        strcpy_s(destination + len, PATHLIB_MAX_PATH - len, (*path2 == PATHLIB_DIR_SEPARATOR) ? (path2 + 1) : path2);
#else 
        strncpy(destination + len, (*path2 == PATHLIB_DIR_SEPARATOR) ? (path2 + 1) : path2, PATHLIB_MAX_PATH - len - 1);
		destination[PATHLIB_MAX_PATH - 1] = '\0';
#endif
      }
    }
    else {
        if (path2 && *path2) {
            if (*path2 == PATHLIB_DIR_SEPARATOR) {
#ifdef _WIN32
                strcpy_s(destination + len, PATHLIB_MAX_PATH - len, path2);
#else
				strncpy(destination + len, path2, PATHLIB_MAX_PATH - len - 1);
				destination[PATHLIB_MAX_PATH - 1] = '\0';
#endif
            }
        else {
          destination[len] = PATHLIB_DIR_SEPARATOR;
#ifdef _WIN32
          strcpy_s(destination + len + 1, PATHLIB_MAX_PATH - len - 1, path2);
#else
          strncpy(destination + len + 1, path2, PATHLIB_MAX_PATH - len - 2);
		  destination[PATHLIB_MAX_PATH - 1] = '\0';
#endif
        }
      }
    }
  }
  else if (path2 && *path2) {
#ifdef _WIN32
      strcpy_s(destination, PATHLIB_MAX_PATH, path2);
#else
      strncpy(destination, path2, PATHLIB_MAX_PATH - 1);
      destination[PATHLIB_MAX_PATH - 1] = '\0';
#endif
  }
  else
    destination[0] = '\0';
}
