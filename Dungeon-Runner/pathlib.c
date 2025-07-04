#include "pathlib.h"

void path__join(const char *path1, const char *path2, char *destination) {
  if (path1 && *path1) {
    size_t len = strlen(path1);
    strcpy(destination, path1);

    if (destination[len - 1] == PATHLIB_DIR_SEPARATOR) {
      if (path2 && *path2) {
        strcpy(destination + len, (*path2 == PATHLIB_DIR_SEPARATOR) ? (path2 + 1) : path2);
      }
    }
    else {
      if (path2 && *path2) {
        if (*path2 == PATHLIB_DIR_SEPARATOR)
          strcpy(destination + len, path2);
        else {
          destination[len] = PATHLIB_DIR_SEPARATOR;
          strcpy(destination + len + 1, path2);
        }
      }
    }
  }
  else if (path2 && *path2)
    strcpy(destination, path2);
  else
    destination[0] = '\0';
}
