#include <config.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "compat.h"
#include "fuse.h"
#include "ui/ui.h"

const compat_socket_t compat_socket_invalid = -1;
const int compat_socket_EBADF = EBADF;

struct compat_socket_selfpipe_t {
  int read_fd;
  int write_fd;
};

void
compat_socket_networking_init( void )
{
  /* No action necessary */
}

void
compat_socket_networking_end( void )
{
  /* No action necessary */
}

int
compat_socket_blocking_mode( compat_socket_t fd, int blocking )
{
  /* NOT IMPLEMENTED */
  return (int)NULL;
}

int
compat_socket_close( compat_socket_t fd )
{
  /* NOT IMPLEMENTED */
  return (int)NULL;
}

int compat_socket_get_error( void )
{
  /* NOT IMPLEMENTED */
  return (int)NULL;
}

const char *
compat_socket_get_strerror( void )
{
  /* NOT IMPLEMENTED */
  return (const char *)NULL;
}

compat_socket_selfpipe_t* compat_socket_selfpipe_alloc( void )
{
  /* NOT IMPLEMENTED */
}

void compat_socket_selfpipe_free( compat_socket_selfpipe_t *self )
{
  /* NOT IMPLEMENTED */
}

compat_socket_t compat_socket_selfpipe_get_read_fd( compat_socket_selfpipe_t *self )
{
  /* NOT IMPLEMENTED */
}

void compat_socket_selfpipe_wake( compat_socket_selfpipe_t *self )
{
  /* NOT IMPLEMENTED */
}

void compat_socket_selfpipe_discard_data( compat_socket_selfpipe_t *self )
{
  /* NOT IMPLEMENTED */
}
