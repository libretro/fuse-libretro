// Compatibility directory functions

#include <compat.h>

compat_dir compat_opendir(const char* path)
{
   (void)path;
   return NULL;
}

compat_dir_result_t compat_readdir(compat_dir directory, char* name, size_t length)
{
   (void)directory;
   (void)name;
   (void)length;
   return COMPAT_DIR_RESULT_ERROR;
}

int compat_closedir(compat_dir directory)
{
   (void)directory;
   return -1;
}
