/* HuffEnc.h -- Huffman encoding
2009-02-07 : Igor Pavlov : Public domain */

#ifndef __HUFF_ENC_H
#define __HUFF_ENC_H

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
Conditions:
  num <= 1024 = 2 ^ NUM_BITS
  Sum(freqs) < 4M = 2 ^ (32 - NUM_BITS)
  maxLen <= 16 = kMaxLen
  Num_Items(p) >= HUFFMAN_TEMP_SIZE(num)
*/
 
void Huffman_Generate(const UInt32 *freqs, UInt32 *p, Byte *lens, UInt32 num, UInt32 maxLen);

#ifdef __cplusplus
}
#endif

#endif
