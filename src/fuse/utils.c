// Fix a bug in utils_find_file_path where stat is used instead of compat_file_exists

// Pre-include sys/stat.h tp avoid errors with the stat macro below
#include <sys/stat.h>
#include <unistd.h>

#define stat(a, b) (!compat_file_exists(a))
#include <fuse/utils.c>
#undef stat
