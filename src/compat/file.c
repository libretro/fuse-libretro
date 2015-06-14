// Compatibility file functions

#include <libretro.h>
#include <externs.h>
#include <compat.h>
#include <ui/ui.h>
#include <utils.h>
#include <string.h>

#include <fuse/roms/48.h>
#include <fuse/roms/128-0.h>
#include <fuse/roms/128-1.h>
#include <fuse/roms/plus2-0.h>
#include <fuse/roms/plus2-1.h>
#include <fuse/roms/plus3-0.h>
#include <fuse/roms/plus3-1.h>
#include <fuse/roms/plus3-2.h>
#include <fuse/roms/plus3-3.h>
#include <fuse/roms/plus3e-0.h>
#include <fuse/roms/plus3e-1.h>
#include <fuse/roms/plus3e-2.h>
#include <fuse/roms/plus3e-3.h>
#include <fuse/roms/se-0.h>
#include <fuse/roms/se-1.h>
#include <fuse/roms/tc2048.h>
#include <fuse/roms/tc2068-0.h>
#include <fuse/roms/tc2068-1.h>

#include <fuse/lib/compressed/tape_16.h>
#include <fuse/lib/compressed/tape_48.h>
#include <fuse/lib/compressed/tape_48_ntsc.h>
#include <fuse/lib/compressed/tape_128.h>
#include <fuse/lib/compressed/tape_plus2.h>
#include <fuse/lib/compressed/tape_plus2a.h>
#include <fuse/lib/compressed/tape_plus3.h>
#include <fuse/lib/compressed/tape_plus3e.h>
#include <fuse/lib/compressed/tape_se.h>
#include <fuse/lib/compressed/tape_2048.h>
#include <fuse/lib/compressed/tape_2068.h>
#include <fuse/lib/compressed/tape_ts2068.h>
#include <fuse/lib/compressed/tape_pentagon.h>
#include <fuse/lib/compressed/tape_scorpion.h>

typedef struct
{
   const char *name;
   const unsigned char* ptr;
   size_t size;
}
entry_t;

static const entry_t mem_entries[] = {
   { "48.rom",           fuse_roms_48_rom,                       sizeof(fuse_roms_48_rom) },
   { "128-0.rom",        fuse_roms_128_0_rom,                    sizeof(fuse_roms_128_0_rom) },
   { "128-1.rom",        fuse_roms_128_1_rom,                    sizeof(fuse_roms_128_1_rom) },
   { "plus2-0.rom",      fuse_roms_plus2_0_rom,                  sizeof(fuse_roms_plus2_0_rom) },
   { "plus2-1.rom",      fuse_roms_plus2_1_rom,                  sizeof(fuse_roms_plus2_1_rom) },
   { "plus3-0.rom",      fuse_roms_plus3_0_rom,                  sizeof(fuse_roms_plus3_0_rom) },
   { "plus3-1.rom",      fuse_roms_plus3_1_rom,                  sizeof(fuse_roms_plus3_1_rom) },
   { "plus3-2.rom",      fuse_roms_plus3_2_rom,                  sizeof(fuse_roms_plus3_2_rom) },
   { "plus3-3.rom",      fuse_roms_plus3_3_rom,                  sizeof(fuse_roms_plus3_3_rom) },
   { "plus3e-0.rom",     fuse_roms_plus3e_0_rom,                 sizeof(fuse_roms_plus3e_0_rom) },
   { "plus3e-1.rom",     fuse_roms_plus3e_1_rom,                 sizeof(fuse_roms_plus3e_1_rom) },
   { "plus3e-2.rom",     fuse_roms_plus3e_2_rom,                 sizeof(fuse_roms_plus3e_2_rom) },
   { "plus3e-3.rom",     fuse_roms_plus3e_3_rom,                 sizeof(fuse_roms_plus3e_3_rom) },
   { "se-0.rom",         fuse_roms_se_0_rom,                     sizeof(fuse_roms_se_0_rom) },
   { "se-1.rom",         fuse_roms_se_1_rom,                     sizeof(fuse_roms_se_1_rom) },
   { "tc2048.rom",       fuse_roms_tc2048_rom,                   sizeof(fuse_roms_tc2048_rom) },
   { "tc2068-0.rom",     fuse_roms_tc2068_0_rom,                 sizeof(fuse_roms_tc2068_0_rom) },
   { "tc2068-1.rom",     fuse_roms_tc2068_1_rom,                 sizeof(fuse_roms_tc2068_1_rom) },
   
   { "tape_16.szx",       fuse_lib_compressed_tape_16_szx,       sizeof(fuse_lib_compressed_tape_16_szx) },
   { "tape_48.szx",       fuse_lib_compressed_tape_48_szx,       sizeof(fuse_lib_compressed_tape_48_szx) },
   { "tape_48_ntsc.szx",  fuse_lib_compressed_tape_48_ntsc_szx,  sizeof(fuse_lib_compressed_tape_48_ntsc_szx) },
   { "tape_128.szx",      fuse_lib_compressed_tape_128_szx,      sizeof(fuse_lib_compressed_tape_128_szx) },
   { "tape_plus2.szx",    fuse_lib_compressed_tape_plus2_szx,    sizeof(fuse_lib_compressed_tape_plus2_szx) },
   { "tape_plus2a.szx",   fuse_lib_compressed_tape_plus2a_szx,   sizeof(fuse_lib_compressed_tape_plus2a_szx) },
   { "tape_plus3.szx",    fuse_lib_compressed_tape_plus3_szx,    sizeof(fuse_lib_compressed_tape_plus3_szx) },
   { "tape_plus3e.szx",   fuse_lib_compressed_tape_plus3e_szx,   sizeof(fuse_lib_compressed_tape_plus3e_szx) },
   { "tape_se.szx",       fuse_lib_compressed_tape_se_szx,       sizeof(fuse_lib_compressed_tape_se_szx) },
   { "tape_2048.szx",     fuse_lib_compressed_tape_2048_szx,     sizeof(fuse_lib_compressed_tape_2048_szx) },
   { "tape_2068.szx",     fuse_lib_compressed_tape_2068_szx,     sizeof(fuse_lib_compressed_tape_2068_szx) },
   { "tape_ts2068.szx",   fuse_lib_compressed_tape_ts2068_szx,   sizeof(fuse_lib_compressed_tape_ts2068_szx) },
   { "tape_pentagon.szx", fuse_lib_compressed_tape_pentagon_szx, sizeof(fuse_lib_compressed_tape_pentagon_szx) },
   { "tape_scorpion.szx", fuse_lib_compressed_tape_scorpion_szx, sizeof(fuse_lib_compressed_tape_scorpion_szx) },
};

static const entry_t* find_entry(const char *path)
{
   static entry_t tape;
   
   int i;
   size_t len = strlen(path);
   
   // * signals us to load the tape data
   if (path[0] == '*')
   {
      tape.name = NULL;
      tape.ptr = (const unsigned char*)tape_data;
      tape.size = tape_size;
      return &tape;
   }
   
   for (i = 0; i < sizeof(mem_entries) / sizeof(mem_entries[0]); i++)
   {
      size_t len2 = strlen(mem_entries[i].name);
      
      if (!strcmp(path + len - len2, mem_entries[i].name))
      {
         return mem_entries + i;
      }
   }
   
   return NULL;
}

typedef struct
{
   const char* ptr;
   size_t length, remain;
}
compat_fd_internal;

const compat_fd COMPAT_FILE_OPEN_FAILED = NULL;

compat_fd compat_file_open(const char *path, int write)
{
   if (write)
   {
      log_cb(RETRO_LOG_ERROR, "Cannot open \"%s\" for writing\n", path);
      return COMPAT_FILE_OPEN_FAILED;
   }

   compat_fd_internal *fd = (compat_fd_internal*)malloc(sizeof(compat_fd_internal));
   
   if (!fd)
   {
      log_cb(RETRO_LOG_ERROR, "Out of memory while opening \"%s\"\n", path);
      return COMPAT_FILE_OPEN_FAILED;
   }
    
   const entry_t* entry = find_entry(path);
   
   if (entry != NULL)
   {
      fd->ptr = (const char*)entry->ptr;
      fd->length = fd->remain = entry->size;
     
      log_cb(RETRO_LOG_INFO, "Opened \"%s\" from memory\n", path);
      return (compat_fd)fd;
   }
   
   log_cb(RETRO_LOG_INFO, "Could not find file \"%s\", trying file system\n", path);
   
   const char *sys;
   
   if (!env_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &sys) || !sys)
   {
      log_cb(RETRO_LOG_ERROR, "Error getting the system folder while opening \"%s\"\n", path);
      free(fd);
      return COMPAT_FILE_OPEN_FAILED;
   }
   
   char system[MAX_PATH];
   strncpy(system, sys, MAX_PATH);
   system[MAX_PATH - 1] = 0;
   
   strncat(system, "/fuse", MAX_PATH);
   system[MAX_PATH - 1] = 0;
   
   strncat(system, path, MAX_PATH);
   system[MAX_PATH - 1] = 0;
   
   log_cb(RETRO_LOG_INFO, "Trying to open \"%s\" from the file system\n", system);
   FILE* file = fopen(system, "rb");
   
   if (!file)
   {
      log_cb(RETRO_LOG_ERROR, "Could not find file \"%s\" on the file system\n", system);
      free(fd);
      return COMPAT_FILE_OPEN_FAILED;
   }
   
   long size;
   
   if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) < 0 || fseek(file, 0, SEEK_SET) != 0)
   {
      log_cb(RETRO_LOG_ERROR, "Could not determine size of \"%s\"\n", system);
      fclose(file);
      free(fd);
      return COMPAT_FILE_OPEN_FAILED;
   }
   
   void* ptr = malloc(size);
   
   if (!ptr)
   {
      log_cb(RETRO_LOG_ERROR, "Out of memory while opening \"%s\"\n", system);
      fclose(file);
      free(fd);
      return COMPAT_FILE_OPEN_FAILED;
   }
   
   if (fread(ptr, 1, size, file) != size)
   {
      log_cb(RETRO_LOG_ERROR, "Error reading from \"%s\"\n", system);
      free(ptr);
      fclose(file);
      free(fd);
      return COMPAT_FILE_OPEN_FAILED;
   }
   
   fclose(file);
   
   fd->ptr = (const char*)ptr;
   fd->length = fd->remain = size;
   
   log_cb(RETRO_LOG_INFO, "Opened \"%s\" from the file system\n", system);
   return (compat_fd)fd;
}

off_t compat_file_get_length(compat_fd cfd)
{
   compat_fd_internal *fd = (compat_fd_internal*)cfd;
   return (off_t)fd->length;
}

int compat_file_read(compat_fd cfd, utils_file *file)
{
   compat_fd_internal *fd = (compat_fd_internal*)cfd;
   size_t numread;
   
   numread = file->length < fd->remain ? file->length : fd->remain;
   memcpy(file->buffer, fd->ptr, numread);
   fd->ptr += numread;
   fd->remain -= numread;
   
   if (numread == file->length)
   {
      return 0;
   }
   
   ui_error( UI_ERROR_ERROR,
             "error reading file: expected %lu bytes, but read only %lu",
             (unsigned long)file->length, (unsigned long)numread );
   return 1;
}

int compat_file_write(compat_fd cfd, const unsigned char *buffer, size_t length)
{
   (void)cfd;
   (void)buffer;
   (void)length;
   return 1;
}

int compat_file_close(compat_fd cfd)
{
   free(cfd);
   return 0;
}

int compat_file_exists(const char *path)
{
   log_cb(RETRO_LOG_INFO, "Checking if \"%s\" exists\n", path);
   
   int exists = 0;
   compat_fd fd = compat_file_open(path, 0);
   
   if (fd != COMPAT_FILE_OPEN_FAILED)
   {
      compat_file_close(fd);
      exists = 1;
   }
   
   return exists;
}
