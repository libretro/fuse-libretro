#include "test.h"

static test_edge_sequence_t
complete_edges_list[] = 
{
  /* Standard speed data block */
  { 2168, 3223, 0 },	/* Pilot */
  {  667,    1, 0 },	/* Sync 1 */
  {  735,    1, 0 },	/* Sync 2 */

  { 1710,    2, 0 },	/* Bit 1 */
  {  855,    2, 0 },	/* Bit 2 */
  { 1710,    2, 0 },	/* Bit 3 */
  {  855,    2, 0 },	/* Bit 4 */
  { 1710,    2, 0 },	/* Bit 5 */
  {  855,    2, 0 },	/* Bit 6 */
  { 1710,    2, 0 },	/* Bit 7 */
  {  855,    2, 0 },	/* Bit 8 */

  { 8207500, 1, 0 },	/* Pause */

  /* Turbo speed data block */
  { 1000,    5, 0 },	/* Pilot */
  {  123,    1, 0 },	/* Sync 1 */
  {  456,    1, 0 },	/* Sync 2 */

  {  789,   16, 0 },	/* Byte 1, bits 1-8 */
  {  400,   16, 0 },	/* Byte 2, bits 1-8 */

  {  789,    2, 0 },	/* Byte 3, bit 1 */
  {  400,    2, 0 },	/* Byte 3, bit 2 */
  {  789,    2, 0 },	/* Byte 3, bit 3 */
  {  400,    2, 0 },	/* Byte 3, bit 4 */
  {  789,    2, 0 },	/* Byte 3, bit 5 */
  {  400,    2, 0 },	/* Byte 3, bit 6 */
  {  789,    2, 0 },	/* Byte 3, bit 7 */
  {  400,    2, 0 },	/* Byte 3, bit 8 */

  {  400,    2, 0 },	/* Byte 4, bit 1 */
  {  789,    2, 0 },	/* Byte 4, bit 2 */
  {  400,    2, 0 },	/* Byte 4, bit 3 */
  {  789,    2, 0 },	/* Byte 4, bit 4 */

  { 3454500, 1, 0 },	/* Pause */

  /* Pure tone block */
  {  535,  666, 0 },

  /* List of pulses */
  {  772,    1, 0 },
  {  297,    1, 0 },
  {  692,    1, 0 },

  /* Pure data block */
  { 1639,   16, 0 },	/* Byte 1, bits 1-8 */
  {  552,   16, 0 },	/* Byte 2, bits 1-8 */
  { 1639,   12, 0 },	/* Byte 3, bits 1-6 */
  { 1939000, 1, 0 },	/* Pause */

  /* Pause block */
  { 2163000, 1, 0 },

  /* Group start block */
  {    0,    1, 0 },

  /* Group end block */
  {    0,    1, 0 },

  /* Jump block */
  {    0,    1, 0 },

  /* Pure tone block skipped */

  /* Loop start block */
  {    0,    1, 0 },

    /* Iteration 1 */
    {  837,  185, 0 },	/* Pure tone block */
    {    0,    1, 0 },	/* Loop end block */

    /* Iteration 2 */
    {  837,  185, 0 },	/* Pure tone block */
    {    0,    1, 0 },	/* Loop end block */

    /* Iteration 3 */
    {  837,  185, 0 },	/* Pure tone block */
    {    0,    1, 0 },	/* Loop end block */

  /* Stop tape if in 48K mode block */
  {    0,    1, 4 },

  /* Text description block */
  {    0,    1, 0 },

  /* Message block */
  {    0,    1, 0 },

  /* Archive info block */
  {    0,    1, 0 },

  /* Hardware info block */
  {    0,    1, 0 },

  /* Custom info block */
  {    0,    1, 0 },

  /* Pure tone block */
  {  820,  940, 0 },
  {  820,    1, 2 },

  { -1, 0, 0 }		/* End marker */

};

test_return_t
test_15( void )
{
  return check_edges( DYNAMIC_TEST_PATH( "complete-tzx.tzx" ), complete_edges_list,
		      LIBSPECTRUM_TAPE_FLAGS_STOP | LIBSPECTRUM_TAPE_FLAGS_STOP48 );
}
