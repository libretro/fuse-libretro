// Change the call to utils_write_file in snapshot_write so we can trap the write and use libretro to save it for us

#include <libretro.h>
#include <externs.h>
#include <string.h>

libspectrum_error fuse_libspectrum_snap_write( libspectrum_byte **buffer, size_t *length, int *out_flags, libspectrum_snap *snap, libspectrum_id_t type, libspectrum_creator *creator, int in_flags );

#define libspectrum_snap_write fuse_libspectrum_snap_write
#define utils_write_file fuse_write_snapshot
#include <fuse/snapshot.c>
#undef libspectrum_snap_write
#undef utils_write_file

// This is an ugly hack so that we can have the frontend manage snapshots for us
int fuse_write_snapshot(const char *filename, const unsigned char *buffer, size_t length)
{
   void* new_buffer;

   log_cb(RETRO_LOG_DEBUG, "%s(\"%s\", %p, %lu)\n", __FUNCTION__, filename, buffer, length);

   if (snapshot_size < length)
   {
      new_buffer = realloc(snapshot_buffer, length);

      if (!new_buffer)
      {
         free(snapshot_buffer);
         snapshot_buffer = NULL;
         snapshot_size = 0;

         return 1;
      }

      snapshot_buffer = new_buffer;
      snapshot_size = length;
   }

   memcpy(snapshot_buffer, buffer, length);
   return 0;
}

libspectrum_error fuse_libspectrum_snap_write( libspectrum_byte **buffer, size_t *length, int *out_flags, libspectrum_snap *snap, libspectrum_id_t type, libspectrum_creator *creator, int in_flags )
{
   return libspectrum_snap_write( buffer, length, out_flags, snap, type, creator, in_flags | LIBSPECTRUM_FLAG_SNAPSHOT_NO_COMPRESSION );
}
