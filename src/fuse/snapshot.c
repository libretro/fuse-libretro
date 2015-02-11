// Change the call to utils_write_file in snapshot_write so we can trap the write and use libretro to save it for us

#define utils_write_file fuse_write_snapshot
#include <fuse/snapshot.c>
#undef utils_write_file

#include <libretro.h>
#include <externs.h>

// This is an ugly hack so that we can have the frontend manage snapshots for us
int fuse_write_snapshot(const char *filename, const unsigned char *buffer, size_t length)
{
   log_cb(RETRO_LOG_DEBUG, "%s(\"%s\", %p, %lu)\n", __FUNCTION__, filename, buffer, length);
   
   if (snapshot_buffer)
   {
      free(snapshot_buffer);
   }
   
   snapshot_buffer = malloc(length);
   
   if (snapshot_buffer)
   {
      memcpy(snapshot_buffer, buffer, length);
      snapshot_size = length;
      return 0;
   }
   
   return 1;
}
