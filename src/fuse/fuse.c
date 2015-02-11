// Rename fuse_init and fuse_end so we can call them

#define fuse_init fuse_init_static
#define fuse_end fuse_end_static
#include <fuse/fuse.c>
#undef fuse_init
#undef fuse_end

int fuse_init(int argc, char** argv)
{
   return fuse_init_static(argc, argv);
}

int fuse_end(void)
{
  return fuse_end_static();
}
