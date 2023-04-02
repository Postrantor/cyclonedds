#include "dds/ddsrt/random.h"

#include <stdio.h>
#include <string.h>

bool ddsrt_prng_makeseed(struct ddsrt_prng_seed * seed)
{
  FILE * rndfile;
  memset(seed->key, 0, sizeof(seed->key));
  if ((rndfile = fopen("/dev/urandom", "rb")) == NULL)
    return false;
  else {
    size_t n = fread(seed->key, sizeof(seed->key), 1, rndfile);
    fclose(rndfile);
    return (n == 1);
  }
}
