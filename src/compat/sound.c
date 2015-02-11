// Compatibility sound functions

#include <libspectrum.h>
#include <externs.h>

int sound_lowlevel_init(const char *device, int *freqptr, int *stereoptr)
{
   (void)device;
   *freqptr = 44100;
   *stereoptr = 1;
   return 0;
}

void sound_lowlevel_end(void)
{
}

void sound_lowlevel_frame(libspectrum_signed_word *data, int len)
{
   audio_cb( data, (size_t)len / 2 );
   some_audio = 1;
}
