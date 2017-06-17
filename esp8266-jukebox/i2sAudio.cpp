#include <Arduino.h>
#include "i2s.h"


const uint32_t ICACHE_RODATA_ATTR _i2s_to_pwm[32] = {
  0x00000010, 0x00000410, 0x00400410, 0x00400C10, 0x00500C10, 0x00D00C10, 0x20D00C10, 0x21D00C10, 0x21D80C10, 0xA1D80C10, 
  0xA1D80D10, 0xA1D80D30, 0xA1DC0D30, 0xA1DC8D30, 0xB1DC8D30, 0xB9DC8D30, 0xB9FC8D30, 0xBDFC8D30, 0xBDFE8D30, 0xBDFE8D32, 
  0xBDFE8D33, 0xBDFECD33, 0xFDFECD33, 0xFDFECD73, 0xFDFEDD73, 0xFFFEDD73, 0xFFFEDD7B, 0xFFFEFD7B, 0xFFFFFD7B, 0xFFFFFDFB, 
  0xFFFFFFFB, 0xFFFFFFFF
};

static void i2s_write_pwm(int16_t s) {
  //Instead of having a nice PCM signal, we fake a PWM signal here.
  static int err=0;
  int samp=s;
  samp=(samp+32768);  //to unsigned
  samp-=err;      //Add the error we made when rounding the previous sample (error diffusion)
  //clip value
  if (samp>65535) samp=65535;
  if (samp<0) samp=0;
  samp=_i2s_to_pwm[samp>>11];//send pwm value for sample value
  err=(samp&0x7ff); //Save rounding error.
  i2s_write_sample(samp);
}

