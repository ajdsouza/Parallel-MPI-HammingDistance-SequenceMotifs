// Implement your solutions in this file
#include "hamming.h"

unsigned int hamming(uint64_t x, uint64_t y)
{
  // TODO: calculate the hamming distance and return it
  unsigned int       hammingDist = 0;
  uint64_t diff = x ^ y;    // XOR sets only diff to 1
 
  // count the number of 1s in the diff list
  while (diff != 0)
    {
      // this clears a bit each time from diff 
      hammingDist++;
      diff &= diff - 1;
    }
 
  return hammingDist;
}
