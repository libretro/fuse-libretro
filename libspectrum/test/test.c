#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "test.h"

const char *progname;
static const char *LIBSPECTRUM_MIN_VERSION = "0.4.0";

typedef test_return_t (*test_fn)( void );

#ifndef O_BINARY
#define O_BINARY 0
#endif

int
read_file( libspectrum_byte **buffer, size_t *length, const char *filename )
{
  int fd;
  struct stat info;
  ssize_t bytes;

  fd = open( filename, O_RDONLY | O_BINARY );
  if( fd == -1 ) {
    fprintf( stderr, "%s: couldn't open `%s': %s\n", progname, filename,
	     strerror( errno ) );
    return errno;
  }

  if( fstat( fd, &info ) ) {
    fprintf( stderr, "%s: couldn't stat `%s': %s\n", progname, filename,
	     strerror( errno ) );
    return errno;
  }

  *length = info.st_size;
  *buffer = libspectrum_malloc( *length );

  bytes = read( fd, *buffer, *length );
  if( bytes == -1 ) {
    fprintf( stderr, "%s: error reading from `%s': %s\n", progname, filename,
	     strerror( errno ) );
    return errno;
  } else if( bytes < *length ) {
    fprintf( stderr, "%s: read only %lu of %lu bytes from `%s'\n", progname,
	     (unsigned long)bytes, (unsigned long)*length, filename );
    return 1;
  }

  if( close( fd ) ) {
    fprintf( stderr, "%s: error closing `%s': %s\n", progname, filename,
	     strerror( errno ) );
    return errno;
  }

  return 0;
}

static test_return_t
load_tape( libspectrum_tape **tape, const char *filename,
           libspectrum_error expected_result )
{
  libspectrum_byte *buffer = NULL;
  size_t filesize = 0;

  if( read_file( &buffer, &filesize, filename ) ) return TEST_INCOMPLETE;

  *tape = libspectrum_tape_alloc();

  if( libspectrum_tape_read( *tape, buffer, filesize, LIBSPECTRUM_ID_UNKNOWN,
			     filename ) != expected_result ) {
    fprintf( stderr, "%s: reading `%s' did not give expected result\n",
	     progname, filename );
    libspectrum_tape_free( *tape );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer );

  return TEST_PASS;
}

static test_return_t
read_tape( const char *filename, libspectrum_error expected_result )
{
  libspectrum_tape *tape;
  test_return_t r; 

  r = load_tape( &tape, filename, expected_result );
  if( r != TEST_PASS ) return r;

  if( libspectrum_tape_free( tape ) ) return TEST_INCOMPLETE;

  return TEST_PASS;
}

static test_return_t
read_snap( const char *filename, const char *filename_to_pass,
	   libspectrum_error expected_result )
{
  libspectrum_byte *buffer = NULL;
  size_t filesize = 0;
  libspectrum_snap *snap;

  if( read_file( &buffer, &filesize, filename ) ) return TEST_INCOMPLETE;

  snap = libspectrum_snap_alloc();

  if( libspectrum_snap_read( snap, buffer, filesize, LIBSPECTRUM_ID_UNKNOWN,
			     filename_to_pass ) != expected_result ) {
    fprintf( stderr, "%s: reading `%s' did not give expected result\n",
	     progname, filename );
    libspectrum_snap_free( snap );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer );

  if( libspectrum_snap_free( snap ) ) return TEST_INCOMPLETE;

  return TEST_PASS;
}

static test_return_t
play_tape( const char *filename )
{
  libspectrum_byte *buffer = NULL;
  size_t filesize = 0;
  libspectrum_tape *tape;
  libspectrum_dword tstates;
  int flags;

  if( read_file( &buffer, &filesize, filename ) ) return TEST_INCOMPLETE;

  tape = libspectrum_tape_alloc();

  if( libspectrum_tape_read( tape, buffer, filesize, LIBSPECTRUM_ID_UNKNOWN,
			     filename ) ) {
    libspectrum_tape_free( tape );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer );

  do {

    if( libspectrum_tape_get_next_edge( &tstates, &flags, tape ) ) {
      libspectrum_tape_free( tape );
      return TEST_INCOMPLETE;
    }

  } while( !( flags & LIBSPECTRUM_TAPE_FLAGS_STOP ) );

  if( libspectrum_tape_free( tape ) ) return TEST_INCOMPLETE;

  return TEST_PASS;
}

/* Specific tests begin here */

/* Test for bugs #1479451 and #1706994: tape object incorrectly freed
   after reading invalid tape */
static test_return_t
test_1( void )
{
  return read_tape( STATIC_TEST_PATH( "invalid.tzx" ), LIBSPECTRUM_ERROR_UNKNOWN );
}

/* Test for bugs #1720238: TZX turbo blocks with zero pilot pulses and
   #1720270: freeing a turbo block with no data produces segfault */
static test_return_t
test_2( void )
{
  libspectrum_byte *buffer = NULL;
  size_t filesize = 0;
  libspectrum_tape *tape;
  const char *filename = STATIC_TEST_PATH( "turbo-zeropilot.tzx" );
  libspectrum_dword tstates;
  int flags;

  if( read_file( &buffer, &filesize, filename ) ) return TEST_INCOMPLETE;

  tape = libspectrum_tape_alloc();

  if( libspectrum_tape_read( tape, buffer, filesize, LIBSPECTRUM_ID_UNKNOWN,
			     filename ) ) {
    libspectrum_tape_free( tape );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer );

  if( libspectrum_tape_get_next_edge( &tstates, &flags, tape ) ) {
    libspectrum_tape_free( tape );
    return TEST_INCOMPLETE;
  }

  if( flags ) {
    fprintf( stderr, "%s: reading first edge of `%s' gave unexpected flags 0x%04x; expected 0x0000\n",
	     progname, filename, flags );
    libspectrum_tape_free( tape );
    return TEST_FAIL;
  }

  if( tstates != 667 ) {
    fprintf( stderr, "%s: first edge of `%s' was %d tstates; expected 667\n",
	     progname, filename, tstates );
    libspectrum_tape_free( tape );
    return TEST_FAIL;
  }

  if( libspectrum_tape_free( tape ) ) return TEST_INCOMPLETE;

  return TEST_PASS;
}

/* Test for bug #1725864: writing empty .tap file causes crash */
static test_return_t
test_3( void )
{
  libspectrum_tape *tape;
  libspectrum_byte *buffer = (libspectrum_byte*)1;
  size_t length = 0;

  tape = libspectrum_tape_alloc();

  if( libspectrum_tape_write( &buffer, &length, tape, LIBSPECTRUM_ID_TAPE_TAP ) ) {
    libspectrum_tape_free( tape );
    return TEST_INCOMPLETE;
  }

  /* `buffer' should now have been set to NULL */
  if( buffer ) {
    fprintf( stderr, "%s: `buffer' was not NULL after libspectrum_tape_write()\n", progname );
    libspectrum_tape_free( tape );
    return TEST_FAIL;
  }

  if( libspectrum_tape_free( tape ) ) return TEST_INCOMPLETE;

  return TEST_PASS;
}

/* Test for bug #1753279: invalid compressed file causes crash */
static test_return_t
test_4( void )
{
  const char *filename = STATIC_TEST_PATH( "invalid.gz" );
  return read_snap( filename, filename, LIBSPECTRUM_ERROR_UNKNOWN );
}

/* Further test for bug #1753279: invalid compressed file causes crash */
static test_return_t
test_5( void )
{
  return read_snap( STATIC_TEST_PATH( "invalid.gz" ), NULL, LIBSPECTRUM_ERROR_UNKNOWN );
}

/* Test for bug #1753938: pointer wraparound causes segfault */
static test_return_t
test_6( void )
{
  const char *filename = STATIC_TEST_PATH( "invalid.szx" );
  return read_snap( filename, filename, LIBSPECTRUM_ERROR_CORRUPT );
}

/* Test for bug #1755124: lack of sanity check in GDB code */
static test_return_t
test_7( void )
{
  return read_tape( STATIC_TEST_PATH( "invalid-gdb.tzx" ), LIBSPECTRUM_ERROR_CORRUPT );
}

/* Test for bug #1755372: empty DRB causes segfault */
static test_return_t
test_8( void )
{
  return read_tape( STATIC_TEST_PATH( "empty-drb.tzx" ), LIBSPECTRUM_ERROR_NONE );
}

/* Test for bug #1755539: problems with invalid archive info block */
static test_return_t
test_9( void )
{
  return read_tape( STATIC_TEST_PATH( "invalid-archiveinfo.tzx" ), LIBSPECTRUM_ERROR_CORRUPT );
}

/* Test for bug #1755545: invalid hardware info blocks can leak memory */
static test_return_t
test_10( void )
{
  return read_tape( STATIC_TEST_PATH( "invalid-hardwareinfo.tzx" ), LIBSPECTRUM_ERROR_CORRUPT );
}

/* Test for bug #1756375: invalid Warajevo tape block offset causes segfault */
static test_return_t
test_11( void )
{
  return read_tape( STATIC_TEST_PATH( "invalid-warajevo-blockoffset.tap" ), LIBSPECTRUM_ERROR_CORRUPT );
}

/* Test for bug #1757587: invalid custom info block causes memory leak */
static test_return_t
test_12( void )
{
  return read_tape( STATIC_TEST_PATH( "invalid-custominfo.tzx" ), LIBSPECTRUM_ERROR_CORRUPT );
}

/* Test for bug #1758860: loop end without a loop start block accesses
   uninitialised memory */
static test_return_t
test_13( void )
{
  libspectrum_byte *buffer = NULL;
  size_t filesize = 0;
  libspectrum_tape *tape;
  const char *filename = STATIC_TEST_PATH( "loopend.tzx" );
  libspectrum_dword tstates;
  int flags;

  if( read_file( &buffer, &filesize, filename ) ) return TEST_INCOMPLETE;

  tape = libspectrum_tape_alloc();

  if( libspectrum_tape_read( tape, buffer, filesize, LIBSPECTRUM_ID_UNKNOWN,
			     filename ) ) {
    libspectrum_tape_free( tape );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer );

  if( libspectrum_tape_get_next_edge( &tstates, &flags, tape ) ) {
    libspectrum_tape_free( tape );
    return TEST_INCOMPLETE;
  }

  if( libspectrum_tape_free( tape ) ) return TEST_INCOMPLETE;

  return TEST_PASS;
}

/* Test for bug #1758860: TZX loop blocks broken */
static test_return_t
test_14( void )
{
  return play_tape( STATIC_TEST_PATH( "loop.tzx" ) );
}

/* Test for bug #1802607: TZX loop blocks still broken */
static test_return_t
test_16( void )
{
  return play_tape( STATIC_TEST_PATH( "loop2.tzx" ) );
}

/* Test for bug #1802618: TZX jump blocks broken */
static test_return_t
test_17( void )
{
  return play_tape( STATIC_TEST_PATH( "jump.tzx" ) );
}

/* Test for bug #1821425: crashes writing and reading empty CSW files */
static test_return_t
test_18( void )
{
  return play_tape( STATIC_TEST_PATH( "empty.csw" ) );
}

/* Test for bug #1828945: .tap writing code does not handle all block types */
static test_return_t
test_19( void )
{
  libspectrum_byte *buffer = NULL;
  size_t length = 0;
  libspectrum_tape *tape;
  const char *filename = DYNAMIC_TEST_PATH( "complete-tzx.tzx" );
  test_return_t r;

  r = load_tape( &tape, filename, LIBSPECTRUM_ERROR_NONE );
  if( r ) return r;

  if( libspectrum_tape_write( &buffer, &length, tape,
                              LIBSPECTRUM_ID_TAPE_TAP ) ) {
    fprintf( stderr, "%s: writing `%s' to a .tap file was not successful\n",
             progname, filename );
    libspectrum_tape_free( tape );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer );

  if( libspectrum_tape_free( tape ) ) return TEST_INCOMPLETE;

  return TEST_PASS;
}

/* Tests for bug #1841085: SP not sanity checked when reading .sna files;
   also tests bug #1841111: compressed snapshots cause segfault */
static test_return_t
test_20( void )
{
  const char *filename = STATIC_TEST_PATH( "sp-2000.sna.gz" );
  return read_snap( filename, filename, LIBSPECTRUM_ERROR_CORRUPT );
} 
  
static test_return_t
test_21( void )
{
  const char *filename = STATIC_TEST_PATH( "sp-ffff.sna.gz" );
  return read_snap( filename, filename, LIBSPECTRUM_ERROR_CORRUPT );
} 

/* Tests for bug #2002682: .mdr code does not correctly handle write protect
   flag */
static test_return_t
test_22( void )
{
  libspectrum_byte *buffer = NULL;
  size_t filesize = 0;
  libspectrum_microdrive *mdr;
  const char *filename = STATIC_TEST_PATH( "writeprotected.mdr" );
  test_return_t r;

  if( read_file( &buffer, &filesize, filename ) ) return TEST_INCOMPLETE;

  /* writeprotected.mdr deliberately includes an extra 0 on the end;
     we want this in the buffer so we know what happens if we read off the
     end of the file; however, we don't want it in the length */
  filesize--;

  mdr = libspectrum_microdrive_alloc();

  if( libspectrum_microdrive_mdr_read( mdr, buffer, filesize ) ) {
    libspectrum_microdrive_free( mdr );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer );

  r = libspectrum_microdrive_write_protect( mdr ) ? TEST_PASS : TEST_FAIL;

  libspectrum_microdrive_free( mdr );

  return r;
}

static test_return_t
test_23( void )
{
  libspectrum_byte *buffer = NULL;
  size_t filesize = 0, length;
  libspectrum_microdrive *mdr;
  const char *filename = STATIC_TEST_PATH( "writeprotected.mdr" );
  test_return_t r;

  if( read_file( &buffer, &filesize, filename ) ) return TEST_INCOMPLETE;

  /* writeprotected.mdr deliberately includes an extra 0 on the end;
     we want this in the buffer so we know what happens if we read off the
     end of the file; however, we don't want it in the length */
  filesize--;

  mdr = libspectrum_microdrive_alloc();

  if( libspectrum_microdrive_mdr_read( mdr, buffer, filesize ) ) {
    libspectrum_microdrive_free( mdr );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer ); buffer = NULL;

  libspectrum_microdrive_mdr_write( mdr, &buffer, &length );

  libspectrum_microdrive_free( mdr );

  r = ( length == filesize && buffer[ length - 1 ] == 1 ) ? TEST_PASS : TEST_FAIL;

  libspectrum_free( buffer );

  return r;
}

static test_return_t
test_24( void )
{
  const char *filename = DYNAMIC_TEST_PATH( "complete-tzx.tzx" );
  libspectrum_byte *buffer;
  size_t filesize;
  libspectrum_tape *tape;
  libspectrum_tape_iterator it;
  libspectrum_tape_block *block;
  libspectrum_dword expected_sizes[20] = {
    15216886,	/* ROM */
    3493371,	/* Turbo */
    356310,	/* Pure tone */
    1761,	/* Pulses */
    1993724,	/* Pure data */
    2163000,	/* Pause */
    0,		/* Group start */
    0,		/* Group end */
    0,		/* Jump */
    205434,	/* Pure tone */
    0,		/* Loop start */
    154845,	/* Pure tone */
    0,		/* Loop end */
    0,		/* Stop tape if in 48K mode */
    0,		/* Comment */
    0,		/* Message */
    0,		/* Archive info */
    0,		/* Hardware */
    0,		/* Custom info */
    771620,	/* Pure tone */
  };
  libspectrum_dword *next_size = &expected_sizes[ 0 ];
  test_return_t r = TEST_PASS;

  if( read_file( &buffer, &filesize, filename ) ) return TEST_INCOMPLETE;

  tape = libspectrum_tape_alloc();

  if( libspectrum_tape_read( tape, buffer, filesize, LIBSPECTRUM_ID_UNKNOWN,
			     filename ) ) {
    libspectrum_tape_free( tape );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer );

  block = libspectrum_tape_iterator_init( &it, tape );

  while( block )
  {
    libspectrum_dword actual_size = libspectrum_tape_block_length( block ); 

    if( actual_size != *next_size )
    {
      fprintf( stderr, "%s: block had length %lu, but expected %lu\n", progname, (unsigned long)actual_size, (unsigned long)*next_size );
      r = TEST_FAIL;
      break;
    }

    block = libspectrum_tape_iterator_next( &it );
    next_size++;
  }

  if( libspectrum_tape_free( tape ) ) return TEST_INCOMPLETE;

  return r;
}

static test_return_t
test_25( void )
{
  const char *filename = STATIC_TEST_PATH( "empty.z80" );
  libspectrum_byte *buffer = NULL;
  size_t filesize = 0, length = 0;
  libspectrum_snap *snap;
  int flags;
  test_return_t r = TEST_INCOMPLETE;

  if( read_file( &buffer, &filesize, filename ) ) return TEST_INCOMPLETE;

  snap = libspectrum_snap_alloc();

  if( libspectrum_snap_read( snap, buffer, filesize, LIBSPECTRUM_ID_UNKNOWN,
			     filename ) != LIBSPECTRUM_ERROR_NONE ) {
    fprintf( stderr, "%s: reading `%s' failed\n", progname, filename );
    libspectrum_snap_free( snap );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer );
  buffer = NULL;

  if( libspectrum_snap_write( &buffer, &length, &flags, snap,
                              LIBSPECTRUM_ID_SNAPSHOT_SNA, NULL, 0 ) != 
      LIBSPECTRUM_ERROR_NONE ) {
    fprintf( stderr, "%s: serialising to SNA failed\n", progname );
    libspectrum_snap_free( snap );
    return TEST_INCOMPLETE;
  }

  libspectrum_snap_free( snap );
  snap = libspectrum_snap_alloc();

  if( libspectrum_snap_read( snap, buffer, length, LIBSPECTRUM_ID_SNAPSHOT_SNA,
                             NULL ) != LIBSPECTRUM_ERROR_NONE ) {
    fprintf( stderr, "%s: restoring from SNA failed\n", progname );
    libspectrum_snap_free( snap );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer );

  if( libspectrum_snap_pc( snap ) != 0x1234 ) {
    fprintf( stderr, "%s: PC is 0x%04x, not the expected 0x1234\n", progname,
             libspectrum_snap_pc( snap ) );
    r = TEST_FAIL;
  } else if( libspectrum_snap_sp( snap ) != 0x8000 ) {
    fprintf( stderr, "%s: SP is 0x%04x, not the expected 0x8000\n", progname,
             libspectrum_snap_sp( snap ) );
    r = TEST_FAIL;
  } else {
    r = TEST_PASS;
  }

  libspectrum_snap_free( snap );

  return r;
}

/* Tests for bug #3078262: last out to 0x1ffd is not serialised into .z80
   files */
static test_return_t
test_26( void )
{
  const char *filename = STATIC_TEST_PATH( "plus3.z80" );
  libspectrum_byte *buffer = NULL;
  size_t filesize = 0, length = 0;
  libspectrum_snap *snap;
  int flags;
  test_return_t r = TEST_INCOMPLETE;

  if( read_file( &buffer, &filesize, filename ) ) return TEST_INCOMPLETE;

  snap = libspectrum_snap_alloc();

  if( libspectrum_snap_read( snap, buffer, filesize, LIBSPECTRUM_ID_UNKNOWN,
			     filename ) != LIBSPECTRUM_ERROR_NONE ) {
    fprintf( stderr, "%s: reading `%s' failed\n", progname, filename );
    libspectrum_snap_free( snap );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer );
  buffer = NULL;

  if( libspectrum_snap_write( &buffer, &length, &flags, snap,
                              LIBSPECTRUM_ID_SNAPSHOT_Z80, NULL, 0 ) != 
      LIBSPECTRUM_ERROR_NONE ) {
    fprintf( stderr, "%s: serialising to Z80 failed\n", progname );
    libspectrum_snap_free( snap );
    return TEST_INCOMPLETE;
  }

  libspectrum_snap_free( snap );
  snap = libspectrum_snap_alloc();

  if( libspectrum_snap_read( snap, buffer, length, LIBSPECTRUM_ID_SNAPSHOT_Z80,
                             NULL ) != LIBSPECTRUM_ERROR_NONE ) {
    fprintf( stderr, "%s: restoring from Z80 failed\n", progname );
    libspectrum_snap_free( snap );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  if( libspectrum_snap_out_plus3_memoryport( snap ) == 0xaa ) {
    r = TEST_PASS;
  } else {
    fprintf( stderr,
             "%s: Last out to 0x1ffd is 0x%02x, not the expected 0xaa\n",
             progname, libspectrum_snap_out_plus3_memoryport( snap ) );
    r = TEST_FAIL;
  }

  libspectrum_snap_free( snap );

  return r;
}

/* Tests for bug #2857419: SZX files were written with A and F reversed */
static test_return_t
test_27( void )
{
  const char *filename = STATIC_TEST_PATH( "empty.szx" );
  libspectrum_byte *buffer = NULL;
  size_t filesize = 0;
  libspectrum_snap *snap;
  test_return_t r = TEST_INCOMPLETE;

  if( read_file( &buffer, &filesize, filename ) ) return TEST_INCOMPLETE;

  snap = libspectrum_snap_alloc();

  if( libspectrum_snap_read( snap, buffer, filesize, LIBSPECTRUM_ID_UNKNOWN,
			     filename ) != LIBSPECTRUM_ERROR_NONE ) {
    fprintf( stderr, "%s: reading `%s' failed\n", progname, filename );
    libspectrum_snap_free( snap );
    libspectrum_free( buffer );
    return TEST_INCOMPLETE;
  }

  libspectrum_free( buffer );

  if( libspectrum_snap_a( snap ) != 0x12 ) {
    fprintf( stderr, "%s: A is 0x%02x, not the expected 0x12\n", progname,
             libspectrum_snap_a( snap ) );
    r = TEST_FAIL;
  } else if( libspectrum_snap_f( snap ) != 0x34 ) {
    fprintf( stderr, "%s: F is 0x%02x, not the expected 0x34\n", progname,
             libspectrum_snap_f( snap ) );
    r = TEST_FAIL;
  } else if( libspectrum_snap_a_( snap ) != 0x56 ) {
    fprintf( stderr, "%s: A' is 0x%02x, not the expected 0x56\n", progname,
             libspectrum_snap_a_( snap ) );
    r = TEST_FAIL;
  } else if( libspectrum_snap_f_( snap ) != 0x78 ) {
    fprintf( stderr, "%s: F' is 0x%02x, not the expected 0x78\n", progname,
             libspectrum_snap_f_( snap ) );
    r = TEST_FAIL;
  } else {
    r = TEST_PASS;
  }

  return r;
}

struct test_description {

  test_fn test;
  const char *description;
  int active;

};

static struct test_description tests[] = {
  { test_1, "Tape with unknown block", 0 },
  { test_2, "TZX turbo data with zero pilot pulses and zero data", 0 },
  { test_3, "Writing empty .tap file", 0 },
  { test_4, "Invalid compressed file 1", 0 },
  { test_5, "Invalid compressed file 2", 0 },
  { test_6, "Pointer wraparound in SZX file", 0 },
  { test_7, "Invalid TZX GDB", 0 },
  { test_8, "Empty TZX DRB", 0 },
  { test_9, "Invalid TZX archive info block", 0 },
  { test_10, "Invalid hardware info block causes memory leak", 0 },
  { test_11, "Invalid Warajevo tape file", 0 },
  { test_12, "Invalid TZX custom info block causes memory leak", 0 },
  { test_13, "TZX loop end block with loop start block", 0 },
  { test_14, "TZX loop blocks", 0 },
  { test_15, "Complete TZX file", 0 },
  { test_16, "TZX loop blocks 2", 0 },
  { test_17, "TZX jump blocks", 0 },
  { test_18, "CSW empty file", 0 },
  { test_19, "Complete TZX to TAP conversion", 0 },
  { test_20, "SNA file with SP < 0x4000", 0 },
  { test_21, "SNA file with SP = 0xffff", 0 },
  { test_22, "MDR write protection 1", 0 },
  { test_23, "MDR write protection 2", 0 },
  { test_24, "Complete TZX timings", 0 },
  { test_25, "Writing SNA file", 0 },
  { test_26, "Writing +3 .Z80 file", 0 },
  { test_27, "Reading old SZX file", 0 },
};

static size_t test_count = sizeof( tests ) / sizeof( tests[0] );

static void
parse_test_specs( char **specs, int count )
{
  int i, j;

  for( i = 0; i < count; i++ ) {

    const char *spec = specs[i];
    const char *dash = strchr( spec, '-' );

    if( dash ) {
      int begin = atoi( spec ), end = atoi( dash + 1 );
      if( begin < 1 ) begin = 1;
      if( end == 0 || end > test_count ) end = test_count;
      for( j = begin; j <= end; j++ ) tests[j-1].active = 1;
    } else {
      int test = atoi( spec );
      if( test < 1 || test > test_count ) continue;
      tests[ test - 1 ].active = 1;
    }
    
  }
}

int
main( int argc, char *argv[] )
{
  struct test_description *test;
  size_t i;
  int tests_done = 0, tests_skipped = 0;
  int pass = 0, fail = 0, incomplete = 0;

  progname = argv[0];

  if( libspectrum_check_version( LIBSPECTRUM_MIN_VERSION ) ) {
    if( libspectrum_init() ) return 2;
  } else {
    fprintf( stderr, "%s: libspectrum version %s found, but %s required",
	     progname, libspectrum_version(), LIBSPECTRUM_MIN_VERSION );
    return 2;
  }

  if( argc < 2 ) {
    for( i = 0; i < test_count; i++ ) tests[i].active = 1;
  } else {
    parse_test_specs( &argv[1], argc - 1 );
  }

  for( i = 0, test = tests;
       i < test_count;
       i++, test++ ) {
    printf( "Test %d: %s... ", (int)i + 1, test->description );
    if( test->active ) {
      tests_done++;
      switch( test->test() ) {
      case TEST_PASS:
	printf( "passed\n" );
	pass++;
	break;
      case TEST_FAIL:
	printf( "FAILED\n" );
	fail++;
	break;
      case TEST_INCOMPLETE:
	printf( "NOT COMPLETE\n" );
	incomplete++;
	break;
      }
    } else {
      tests_skipped++;
      printf( "skipped\n" );
    }
      
  }

  /* Stop silly divisions occuring */
  if( !tests_done ) tests_done = 1;

  printf( "\n%3d tests run\n\n", (int)test_count );
  printf( "%3d     passed (%6.2f%%)\n", pass, 100 * (float)pass/tests_done );
  printf( "%3d     failed (%6.2f%%)\n", fail, 100 * (float)fail/tests_done );
  printf( "%3d incomplete (%6.2f%%)\n", incomplete, 100 * (float)incomplete/tests_done );
  printf( "%3d    skipped\n", tests_skipped );

  if( fail == 0 && incomplete == 0 ) {
    return 0;
  } else {
    return 1;
  }
}
