/*
 * em_inflate.c - fast in-memory inflate (gzip/zlib decompressor) implementation
 *
 * Copyright (C) 2019 Emmanuel Marty
 *
 * gzip crc32 computation by Stephen Brumme - zlib-like license included below
 * adler checksum computation by Mark Adler - zlib license included below
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "em_inflate.h"

/* Comment out this define to disable checksum verification */
#define EM_INFLATE_VERIFY_CHECKSUM

/*--- LSB bitreader ---*/

#if defined(_M_X64) || defined(__x86_64__) || defined(__aarch64__)
#define EM_LSB_BITREADER_64BIT_SHIFTER
#endif /* defined(_M_X64) */

#ifdef EM_LSB_BITREADER_64BIT_SHIFTER
typedef unsigned long long em_lsb_bitreader_shifter_t;
#else
typedef unsigned int em_lsb_bitreader_shifter_t;
#endif

/** Bit reader context */
typedef struct {
   int nDecShifterBitCount;
   em_lsb_bitreader_shifter_t nDecShifterData;
   const unsigned char *pInBlock;
   const unsigned char *pInBlockEnd;
   const unsigned char *pInBlockStart;
} em_lsb_bitreader_t;

/**
 * Initialize bit reader
 *
 * @param pBitReader bit reader context
 * @param pInBlock pointer to start of compressed block
 * @param InBlockEnd pointer to end of compressed block + 1
 */
static inline void em_lsb_bitreader_init(em_lsb_bitreader_t *pBitReader, const unsigned char *pInBlock, const unsigned char *pInBlockEnd) {
   pBitReader->nDecShifterBitCount = 0;
   pBitReader->nDecShifterData = 0;
   pBitReader->pInBlock = pInBlock;
   pBitReader->pInBlockEnd = pInBlockEnd;
   pBitReader->pInBlockStart = pInBlock;
}

/**
 * Refill 32 bits at a time if the architecture allows it, otherwise do nothing.
 *
 * @param pBitReader bit reader context
 */
static inline void em_lsb_bitreader_refill_32(em_lsb_bitreader_t *pBitReader) {
#ifdef EM_LSB_BITREADER_64BIT_SHIFTER
   if (pBitReader->nDecShifterBitCount <= 32 && (pBitReader->pInBlock + 4) <= pBitReader->pInBlockEnd) {
#if defined(_M_X64) || defined(__x86_64__)
      /* Read unaligned word */
      pBitReader->nDecShifterData |= (((em_lsb_bitreader_shifter_t)(*((unsigned int*)pBitReader->pInBlock))) << pBitReader->nDecShifterBitCount);
      pBitReader->nDecShifterBitCount += 32;
      pBitReader->pInBlock += 4;
#else
      /* Read bytes */
      pBitReader->nDecShifterData |= (((em_lsb_bitreader_shifter_t)(*pBitReader->pInBlock++)) << pBitReader->nDecShifterBitCount);
      pBitReader->nDecShifterBitCount += 8;
      pBitReader->nDecShifterData |= (((em_lsb_bitreader_shifter_t)(*pBitReader->pInBlock++)) << pBitReader->nDecShifterBitCount);
      pBitReader->nDecShifterBitCount += 8;
      pBitReader->nDecShifterData |= (((em_lsb_bitreader_shifter_t)(*pBitReader->pInBlock++)) << pBitReader->nDecShifterBitCount);
      pBitReader->nDecShifterBitCount += 8;
      pBitReader->nDecShifterData |= (((em_lsb_bitreader_shifter_t)(*pBitReader->pInBlock++)) << pBitReader->nDecShifterBitCount);
      pBitReader->nDecShifterBitCount += 8;
#endif
   }
#endif /* EM_LSB_BITREADER_64BIT_SHIFTER */
}

/**
 * Read variable bit-length value
 *
 * @param pBitReader bit reader context
 * @param nBits size of value in bits (number of bits to read), 0..16
 *
 * @return value, or -1 for failure
 */
static inline unsigned int em_lsb_bitreader_get_bits(em_lsb_bitreader_t *pBitReader, const int nBits) {
   if (pBitReader->nDecShifterBitCount < nBits) {
      if (pBitReader->pInBlock < pBitReader->pInBlockEnd) {
         pBitReader->nDecShifterData |= (((em_lsb_bitreader_shifter_t)(*pBitReader->pInBlock++)) << pBitReader->nDecShifterBitCount);
         pBitReader->nDecShifterBitCount += 8;

         if (pBitReader->pInBlock < pBitReader->pInBlockEnd) {
            pBitReader->nDecShifterData |= (((em_lsb_bitreader_shifter_t)(*pBitReader->pInBlock++)) << pBitReader->nDecShifterBitCount);
            pBitReader->nDecShifterBitCount += 8;
         }
      }
      else
         return -1;
   }

   unsigned int nValue = pBitReader->nDecShifterData & ((1 << nBits) - 1);
   pBitReader->nDecShifterData >>= nBits;
   pBitReader->nDecShifterBitCount -= nBits;
   return nValue;
}

/**
 * Peek at a 16-bit value in the bitstream (lookahead)
 *
 * @param pBitReader bit reader context
 *
 * @return value
 */
static inline unsigned int em_lsb_bitreader_peek_16bits(em_lsb_bitreader_t *pBitReader) {
   if (pBitReader->nDecShifterBitCount < 16) {
      if (pBitReader->pInBlock < pBitReader->pInBlockEnd) {
         pBitReader->nDecShifterData |= (((em_lsb_bitreader_shifter_t)(*pBitReader->pInBlock++)) << pBitReader->nDecShifterBitCount);
         if (pBitReader->pInBlock < pBitReader->pInBlockEnd)
            pBitReader->nDecShifterData |= (((em_lsb_bitreader_shifter_t)(*pBitReader->pInBlock++)) << (pBitReader->nDecShifterBitCount + 8));
         pBitReader->nDecShifterBitCount += 16;
      }
   }

   return pBitReader->nDecShifterData & 0xffff;
}

/**
 * Consume variable bit-length value, after reading it with em_lsb_bitreader_peek_16bits()
 *
 * @param pBitReader bit reader context
 * @param nBits size of value to consume, in bits
 */
static inline void em_lsb_bitreader_consume_bits(em_lsb_bitreader_t *pBitReader, const int nBits) {
   pBitReader->nDecShifterData >>= nBits;
   pBitReader->nDecShifterBitCount -= nBits;
}

/**
 * Re-align bitstream on a byte
 *
 * @param pBitReader bit reader context
 */
static int em_lsb_bitreader_byte_align(em_lsb_bitreader_t *pBitReader) {
   /* Align on byte */
   while (pBitReader->nDecShifterBitCount >= 8) {
      /* Rewind - the bit reader can load more than 8 bits at a time */
      pBitReader->nDecShifterBitCount -= 8;
      pBitReader->pInBlock--;
      if (pBitReader->pInBlock < pBitReader->pInBlockStart) return -1;
   }

   pBitReader->nDecShifterBitCount = 0;
   pBitReader->nDecShifterData = 0;
   return 0;
}

/*-- Huffman decoder for a LSB bitstream --*/

/** Maximum number of symbols that can be read by the huffman decoder */
#define MAX_SYMBOLS 288

/** Number of symbols used to decode variable code lengths */
#define NCODELENSYMS 19

/** Number of most significant huffman codeword bits that are used to skip the initial code length finding steps */
#define NFASTSYMBOLBITS   10

/** Huffman decoding context */
typedef struct {
   unsigned int nFastSymbol[1 << NFASTSYMBOLBITS];
   unsigned int nStartIndex[16]; /* One per code bit length */
   unsigned int nSymbols;
   int nNumSorted;
   int nStartingPos[16];
} em_lsb_huffman_decoder_t;

/**
 * Prepare huffman tables
 *
 * @param pDecoder decoding context
 * @param pRevSymbolTable array of 2 * nSymbols entries for storing the reverse lookup table
 * @param pCodeLength codeword lengths table
 *
 * @return 0 for success, -1 for failure
 */
static int em_lsb_huffman_decoder_prepare_table(em_lsb_huffman_decoder_t *pDecoder, unsigned int *pRevSymbolTable, const int nReadSymbols, const int nSymbols, const unsigned char *pCodeLength) {
   int nNumSymbolsPerLen[16];
   int i;

   if (nReadSymbols < 0 || nReadSymbols > MAX_SYMBOLS || nSymbols < 0 || nSymbols > MAX_SYMBOLS || nReadSymbols > nSymbols)
      return -1;
   pDecoder->nSymbols = nSymbols;

   /* Calculate starting positions of symbols for each code length, so we don't have to sort them */

   for (i = 0; i < 16; i++)
      nNumSymbolsPerLen[i] = 0;
   for (i = 0; i < nReadSymbols; i++) {
      if (pCodeLength[i] >= 16) return -1;
      nNumSymbolsPerLen[pCodeLength[i]]++;
   }

   pDecoder->nStartingPos[0] = 0;
   pDecoder->nNumSorted = 0;
   for (i = 1; i < 16; i++) {
      pDecoder->nStartingPos[i] = pDecoder->nNumSorted;
      pDecoder->nNumSorted += nNumSymbolsPerLen[i];
   }

   /* Assign symbols for any non-zero lengths, auto sorted by increasing code length */

   for (i = 0; i < nSymbols; i++)
      pRevSymbolTable[i] = -1;

   for (i = 0; i < nReadSymbols; i++) {
      if (pCodeLength[i]) {
         pRevSymbolTable[pDecoder->nStartingPos[pCodeLength[i]]++] = i;
      }
   }

   return 0;
}

/**
 * Finalize huffman codewords for decoding
 *
 * @param pDecoder decoding context
 * @param pRevSymbolTable array of 2 * nSymbols entries that contains the reverse lookup table
 *
 * @return 0 for success, -1 for failure
 */
static int em_lsb_huffman_decoder_finalize_table(em_lsb_huffman_decoder_t *pDecoder, unsigned int *pRevSymbolTable) {
   const int nSymbols = pDecoder->nSymbols;
   unsigned int nCanonicalCodeWord = 0;
   unsigned int *nRevCodeLengthTable = pRevSymbolTable + nSymbols;
   int nCanonicalLength = 1;
   int i;

   /* Re-create canonical huffman codewords and create left-justified words for em_lsb_huffman_decoder_read_value() */

   for (i = 0; i < (1 << NFASTSYMBOLBITS); i++)
      pDecoder->nFastSymbol[i] = 0;
   for (i = 0; i < 16; i++)
      pDecoder->nStartIndex[i] = 0;

   i = 0;
   while (i < pDecoder->nNumSorted) {
      if (nCanonicalLength >= 16) return -1;
      pDecoder->nStartIndex[nCanonicalLength] = i - nCanonicalCodeWord;

      while (i < pDecoder->nStartingPos[nCanonicalLength]) {
         /* Write canonical order -> codeword mapping */
         if (i >= nSymbols) return -1;
         nRevCodeLengthTable[i] = nCanonicalLength;

         /* Check for malformed table */
         if (nCanonicalCodeWord >= (1U << nCanonicalLength)) return -1;

         /* Write accelerated symbol value + codeword len for the (upside down) top NFASTSYMBOLBITS bits of the codeword, at all bit positions */
         if (nCanonicalLength <= NFASTSYMBOLBITS) {
            unsigned int nRevWord;

            /* Get upside down codeword (branchless method by Eric Biggers) */
            nRevWord = ((nCanonicalCodeWord & 0x5555) << 1) | ((nCanonicalCodeWord & 0xaaaa) >> 1);
            nRevWord = ((nRevWord & 0x3333) << 2) | ((nRevWord & 0xcccc) >> 2);
            nRevWord = ((nRevWord & 0x0f0f) << 4) | ((nRevWord & 0xf0f0) >> 4);
            nRevWord = ((nRevWord & 0x00ff) << 8) | ((nRevWord & 0xff00) >> 8);
            nRevWord = nRevWord >> (16 - nCanonicalLength);

            int nSlots = 1 << (NFASTSYMBOLBITS - nCanonicalLength);
            while (nSlots) {
               pDecoder->nFastSymbol[nRevWord] = (pRevSymbolTable[i] & 0xffffff) | (nCanonicalLength << 24);
               nRevWord += (1 << nCanonicalLength);
               nSlots--;
            }
         }

         i++;
         nCanonicalCodeWord++;
      }
      nCanonicalLength++;
      nCanonicalCodeWord <<= 1;
   }

   while (i < nSymbols) {
      pRevSymbolTable[i] = -1;
      nRevCodeLengthTable[i++] = 0;
   }

   return 0;
}

/**
 * Decode next symbol
 *
 * @param pDecoder decoding context
 * @param pRevSymbolTable reverse lookup table
 * @param pBitReader bit reader context
 *
 * @return symbol, or -1 for error
 */
static inline unsigned int em_lsb_huffman_decoder_read_value(em_lsb_huffman_decoder_t *pDecoder, const unsigned int *pRevSymbolTable, em_lsb_bitreader_t *pBitReader) {
   unsigned int nStream = em_lsb_bitreader_peek_16bits(pBitReader);
   unsigned int nFastSymAndBits = pDecoder->nFastSymbol[nStream & ((1 << NFASTSYMBOLBITS) - 1)];
   if (nFastSymAndBits) {
      /* Fast path: the symbol len is <= NFASTSYMBOLBITS, we have a symbol and bit length for the (upside-down) MSB bits */
      em_lsb_bitreader_consume_bits(pBitReader, nFastSymAndBits >> 24);
      return nFastSymAndBits & 0xffffff;
   }

   /* The symbol len is larger than NFASTSYMBOLBITS, decode using the slower path. By definition, the more frequent symbols are shorter and in the fast path. */
   
   const unsigned int *nRevCodeLengthTable = pRevSymbolTable + pDecoder->nSymbols;
   unsigned int nCodeWord = 0;
   int nBits = 1;

   do {
      nCodeWord |= (nStream & 1);

      unsigned int nTableIndex = pDecoder->nStartIndex[nBits] + nCodeWord;
      if (nTableIndex < pDecoder->nSymbols) {
         if (nBits == nRevCodeLengthTable[nTableIndex]) {
            em_lsb_bitreader_consume_bits(pBitReader, nBits);
            return pRevSymbolTable[nTableIndex];
         }
      }

      nCodeWord <<= 1;
      nStream >>= 1;
      nBits++;
   } while (nBits < 16);
   return -1;
}

/**
 * Read fixed bit size code lengths
 *
 * @param nLenBits number of bits per code length
 * @param nReadSymbols number of symbols actually read
 * @param nSymbols number of symbols to build codes for
 * @param pCodeLength output code lengths table
 * @param pBitReader bit reader context
 *
 * @return 0 for success, -1 for failure
 */
static int em_lsb_huffman_decoder_read_raw_lengths(const int nLenBits, const int nReadSymbols, const int nSymbols, unsigned char *pCodeLength, em_lsb_bitreader_t *pBitReader) {
   static const unsigned char nCodeLenSymIndex[NCODELENSYMS] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };  /* Order in which code lengths are stored, as per the zlib specification */
   int i;

   if (nReadSymbols < 0 || nReadSymbols > MAX_SYMBOLS || nSymbols < 0 || nSymbols > MAX_SYMBOLS || nReadSymbols > nSymbols)
      return -1;

   i = 0;
   while (i < nReadSymbols) {
      unsigned int nLength = em_lsb_bitreader_get_bits(pBitReader, nLenBits);
      if (nLength == -1) return -1;

      pCodeLength[nCodeLenSymIndex[i++]] = nLength;
   }

   while (i < nSymbols) {
      pCodeLength[nCodeLenSymIndex[i++]] = 0;
   }

   return 0;
}

/**
 * Read huffman-encoded code lengths
 *
 * @param pTablesDecoder decoding context for code lengths
 * @param pTablesRevSymbolTable reverse lookup table for code lengths
 * @param nReadSymbols number of symbols actually read
 * @param nSymbols number of symbols to build codes for
 * @param pCodeLength output code lengths table
 * @param pBitReader bit reader context
 *
 * @return 0 for success, -1 for failure
 */
static int em_lsb_huffman_decoder_read_var_lengths(em_lsb_huffman_decoder_t *pTablesDecoder, const unsigned int *pTablesRevSymbolTable,
                                                   const int nReadSymbols, const int nSymbols, unsigned char *pCodeLength, em_lsb_bitreader_t *pBitReader) {
   int i;

   if (nReadSymbols < 0 || nSymbols < 0 || nReadSymbols > nSymbols)
      return -1;

   i = 0;
   unsigned int nPrevLength = 0;

   while (i < nReadSymbols) {
      unsigned int nLength = em_lsb_huffman_decoder_read_value(pTablesDecoder, pTablesRevSymbolTable, pBitReader);
      if (nLength == -1) return -1;

      if (nLength < 16) {
         nPrevLength = nLength;
         pCodeLength[i++] = nPrevLength;
      }
      else {
         unsigned int nRunLen = 0;

         if (nLength == 16) {
            int nExtraRunLen = em_lsb_bitreader_get_bits(pBitReader, 2);
            if (nExtraRunLen == -1) return -1;
            nRunLen = 3 + nExtraRunLen;
         }
         else if (nLength == 17) {
            int nExtraRunLen = em_lsb_bitreader_get_bits(pBitReader, 3);
            if (nExtraRunLen == -1) return -1;
            nPrevLength = 0;
            nRunLen = 3 + nExtraRunLen;
         }
         else if (nLength == 18) {
            int nExtraRunLen = em_lsb_bitreader_get_bits(pBitReader, 7);
            if (nExtraRunLen == -1) return -1;
            nPrevLength = 0;
            nRunLen = 11 + nExtraRunLen;
         }

         while (nRunLen && i < nReadSymbols) {
            pCodeLength[i++] = nPrevLength;
            nRunLen--;
         }
      }
   }

   while (i < nSymbols)
      pCodeLength[i++] = 0;
   return 0;
}

/*-- zlib stored blocks copier --*/

static size_t em_inflate_copy_stored(em_lsb_bitreader_t *pBitReader, unsigned char *pOutData, size_t nOutDataOffset, size_t nBlockMaxSize) {
   /* Align on byte */
   if (em_lsb_bitreader_byte_align(pBitReader) < 0)
      return -1;

   if ((pBitReader->pInBlock + 4) > pBitReader->pInBlockEnd) return -1;

   /* Read block length, and its two's complement verification value */
   unsigned short nStoredLen = ((unsigned short)pBitReader->pInBlock[0]) | (((unsigned short)pBitReader->pInBlock[1]) << 8);
   pBitReader->pInBlock += 2;

   unsigned short nNegStoredLen = ((unsigned short)pBitReader->pInBlock[0]) | (((unsigned short)pBitReader->pInBlock[1]) << 8);
   pBitReader->pInBlock += 2;

   /* Make sure that the len and the two's complement match */
   if (nStoredLen != ((~nNegStoredLen) & 0xffff)) return -1;

   /* Make sure there is room */
   if (nStoredLen > nBlockMaxSize) return -1;

   /* Copy stored data */
   memcpy(pOutData + nOutDataOffset, pBitReader->pInBlock, nStoredLen);
   pBitReader->pInBlock += nStoredLen;
   
   return (size_t)nStoredLen;
}

/*-- zlib static and dynamic blocks inflater --*/

#define NCODELENBITS 3
#define NLITERALSYMS 288
#define NEODMARKERSYM 256
#define NMATCHLENSYMSTART 257
#define NMATCHLENSYMS 29
#define NOFFSETSYMS 32
#define MIN_MATCH_SIZE  3
#define MATCHLEN_PAIR(__base,__dispbits) ((__base) | ((__dispbits) << 16) | 0x8000)
#define OFFSET_PAIR(__base,__dispbits) ((__base) | ((__dispbits) << 16))

/** Base value and number of extra displacement bits for each match length codeword */
static const unsigned int em_inflate_matchlen_code[NMATCHLENSYMS] = {
   MATCHLEN_PAIR(MIN_MATCH_SIZE + 0, 0), MATCHLEN_PAIR(MIN_MATCH_SIZE + 1, 0), MATCHLEN_PAIR(MIN_MATCH_SIZE + 2, 0), MATCHLEN_PAIR(MIN_MATCH_SIZE + 3, 0), MATCHLEN_PAIR(MIN_MATCH_SIZE + 4, 0), 
   MATCHLEN_PAIR(MIN_MATCH_SIZE + 5, 0), MATCHLEN_PAIR(MIN_MATCH_SIZE + 6, 0), MATCHLEN_PAIR(MIN_MATCH_SIZE + 7, 0), MATCHLEN_PAIR(MIN_MATCH_SIZE + 8, 1), MATCHLEN_PAIR(MIN_MATCH_SIZE + 10, 1),
   MATCHLEN_PAIR(MIN_MATCH_SIZE + 12, 1), MATCHLEN_PAIR(MIN_MATCH_SIZE + 14, 1), MATCHLEN_PAIR(MIN_MATCH_SIZE + 16, 2), MATCHLEN_PAIR(MIN_MATCH_SIZE + 20, 2), MATCHLEN_PAIR(MIN_MATCH_SIZE + 24, 2), 
   MATCHLEN_PAIR(MIN_MATCH_SIZE + 28, 2), MATCHLEN_PAIR(MIN_MATCH_SIZE + 32, 3), MATCHLEN_PAIR(MIN_MATCH_SIZE + 40, 3), MATCHLEN_PAIR(MIN_MATCH_SIZE + 48, 3), MATCHLEN_PAIR(MIN_MATCH_SIZE + 56, 3),
   MATCHLEN_PAIR(MIN_MATCH_SIZE + 64, 4), MATCHLEN_PAIR(MIN_MATCH_SIZE + 80, 4), MATCHLEN_PAIR(MIN_MATCH_SIZE + 96, 4), MATCHLEN_PAIR(MIN_MATCH_SIZE + 112, 4), MATCHLEN_PAIR(MIN_MATCH_SIZE + 128, 5), 
   MATCHLEN_PAIR(MIN_MATCH_SIZE + 160, 5), MATCHLEN_PAIR(MIN_MATCH_SIZE + 192, 5), MATCHLEN_PAIR(MIN_MATCH_SIZE + 224, 5), MATCHLEN_PAIR(MIN_MATCH_SIZE + 255, 0),
};

/** Base value and number of extra displacement bits for each offset codeword */
static const unsigned int em_inflate_offset_code[NOFFSETSYMS] = {
   OFFSET_PAIR(1, 0), OFFSET_PAIR(2, 0), OFFSET_PAIR(3, 0), OFFSET_PAIR(4, 0), OFFSET_PAIR(5, 1), OFFSET_PAIR(7, 1), OFFSET_PAIR(9, 2), OFFSET_PAIR(13, 2), OFFSET_PAIR(17, 3), OFFSET_PAIR(25, 3),
   OFFSET_PAIR(33, 4), OFFSET_PAIR(49, 4), OFFSET_PAIR(65, 5), OFFSET_PAIR(97, 5), OFFSET_PAIR(129, 6), OFFSET_PAIR(193, 6), OFFSET_PAIR(257, 7), OFFSET_PAIR(385, 7), OFFSET_PAIR(513, 8), OFFSET_PAIR(769, 8),
   OFFSET_PAIR(1025, 9), OFFSET_PAIR(1537, 9), OFFSET_PAIR(2049, 10), OFFSET_PAIR(3073, 10), OFFSET_PAIR(4097, 11), OFFSET_PAIR(6145, 11), OFFSET_PAIR(8193, 12), OFFSET_PAIR(12289, 12), OFFSET_PAIR(16385, 13), OFFSET_PAIR(24577, 13),
};

/**
 * Decompress one data block
 *
 * @param pInBlock pointer to compressed data
 * @param nDynamicBlock non-zero for a dynamic huffman tables block, zero for a static huffman tables block
 * @param nInBlockSize size of compressed data, in bytes
 * @param pOutData pointer to output decompression buffer (previously decompressed bytes + room for decompressing this block)
 * @param nOutDataOffset starting index of where to store decompressed bytes in output buffer (and size of previously decompressed bytes)
 * @param nBlockMaxSize total size of output decompression buffer, in bytes
 *
 * @return size of decompressed data in bytes, or -1 for error
 */
static size_t em_inflate_decompress_block(em_lsb_bitreader_t *pBitReader, int nDynamicBlock, unsigned char *pOutData, size_t nOutDataOffset, size_t nBlockMaxSize) {
   em_lsb_huffman_decoder_t literalsDecoder;
   em_lsb_huffman_decoder_t offsetDecoder;
   unsigned int nLiteralsRevSymbolTable[NLITERALSYMS * 2];
   unsigned int nOffsetRevSymbolTable[NOFFSETSYMS * 2];
   int i;

   if (nDynamicBlock) {
      em_lsb_huffman_decoder_t tablesDecoder;
      unsigned char nCodeLength[NLITERALSYMS + NOFFSETSYMS];
      unsigned int nTablesRevSymbolTable[NCODELENSYMS * 2];

      /* Read the encoded number of literals/match len symbols, offset symbols, and code length symbols */
      unsigned int nLiteralSyms = em_lsb_bitreader_get_bits(pBitReader, 5);
      if (nLiteralSyms == -1) return -1;
      nLiteralSyms += 257;
      if (nLiteralSyms > NLITERALSYMS) return -1;

      unsigned int nOffsetSyms = em_lsb_bitreader_get_bits(pBitReader, 5);
      if (nOffsetSyms == -1) return -1;
      nOffsetSyms += 1;
      if (nOffsetSyms > NOFFSETSYMS) return -1;

      unsigned int nCodeLenSyms = em_lsb_bitreader_get_bits(pBitReader, 4);
      if (nCodeLenSyms == -1) return -1;
      nCodeLenSyms += 4;
      if (nCodeLenSyms > NCODELENSYMS) return -1;

      /* Read code lengths table */
      if (em_lsb_huffman_decoder_read_raw_lengths(NCODELENBITS /* code length bits */, nCodeLenSyms /* symbols */, NCODELENSYMS, nCodeLength, pBitReader) < 0)
         return -1;
      if (em_lsb_huffman_decoder_prepare_table(&tablesDecoder, nTablesRevSymbolTable, NCODELENSYMS, NCODELENSYMS, nCodeLength) < 0)
         return -1;
      if (em_lsb_huffman_decoder_finalize_table(&tablesDecoder, nTablesRevSymbolTable) < 0)
         return -1;

      /* Use code lengths table to read literals/match len and offset tables */
      if (em_lsb_huffman_decoder_read_var_lengths(&tablesDecoder, nTablesRevSymbolTable, nLiteralSyms + nOffsetSyms /* read symbols */, NLITERALSYMS + NOFFSETSYMS /* total symbols */, nCodeLength, pBitReader) < 0)
         return -1;
      if (em_lsb_huffman_decoder_prepare_table(&literalsDecoder, nLiteralsRevSymbolTable, nLiteralSyms, NLITERALSYMS, nCodeLength) < 0)
         return -1;
      if (em_lsb_huffman_decoder_prepare_table(&offsetDecoder, nOffsetRevSymbolTable, nOffsetSyms, NOFFSETSYMS, nCodeLength + nLiteralSyms) < 0)
         return -1;
   }
   else {
      unsigned char nFixedLiteralCodeLen[NLITERALSYMS];
      unsigned char nFixedOffsetCodeLen[NOFFSETSYMS];

      /* Fixed huffman codes for this block. Build fixed code lengths for literals/match lens and offsets, as per the zlib specification */

      for (i = 0; i < 144; i++)
         nFixedLiteralCodeLen[i] = 8;
      for (; i < 256; i++)
         nFixedLiteralCodeLen[i] = 9;
      for (; i < 280; i++)
         nFixedLiteralCodeLen[i] = 7;
      for (; i < NLITERALSYMS; i++)
         nFixedLiteralCodeLen[i] = 8;

      for (i = 0; i < NOFFSETSYMS; i++)
         nFixedOffsetCodeLen[i] = 5;

      if (em_lsb_huffman_decoder_prepare_table(&literalsDecoder, nLiteralsRevSymbolTable, NLITERALSYMS, NLITERALSYMS, nFixedLiteralCodeLen) < 0)
         return -1;
      if (em_lsb_huffman_decoder_prepare_table(&offsetDecoder, nOffsetRevSymbolTable, NOFFSETSYMS, NOFFSETSYMS, nFixedOffsetCodeLen) < 0)
         return -1;
   }

   /* Replace codewords in the offsets table by the base + extra bit count, save table lookups when decoding */
   for (i = 0; i < NOFFSETSYMS; i++) {
      unsigned int n = nOffsetRevSymbolTable[i];
      if (n < NOFFSETSYMS) {
         nOffsetRevSymbolTable[i] = em_inflate_offset_code[n];
      }
   }

   /* Replace match codewords in the literals/match len table by the base + extra bit count, save table lookups when decoding */
   for (i = 0; i < NLITERALSYMS; i++) {
      unsigned int n = nLiteralsRevSymbolTable[i];
      if (n >= NMATCHLENSYMSTART && n < NLITERALSYMS) {
         int index = n - NMATCHLENSYMSTART;
         if (index >= 0 && index < NMATCHLENSYMS) {
            nLiteralsRevSymbolTable[i] = em_inflate_matchlen_code[index];
         }
         else {
            /* Handle 286 and 287 codes, used for constructing a complete canonical tree, present in static huffman tables but not used */
            nLiteralsRevSymbolTable[i] = 0;
         }
      }
   }

   /* Finalize huffman tables for decoding, after setting up the bases + extra bits as the symbols */
   if (em_lsb_huffman_decoder_finalize_table(&literalsDecoder, nLiteralsRevSymbolTable) < 0)
      return -1;
   if (em_lsb_huffman_decoder_finalize_table(&offsetDecoder, nOffsetRevSymbolTable) < 0)
      return -1;

   /* Finally, loop to read all the literals/match len codewords in the block to decompress it */

   unsigned char *pCurOutData = pOutData + nOutDataOffset;
   const unsigned char *pOutDataEnd = pCurOutData + nBlockMaxSize;
   const unsigned char *pOutDataFastEnd = pOutDataEnd - 15;

   while (1) {
      em_lsb_bitreader_refill_32(pBitReader);

      unsigned int nLiteralsCodeword = em_lsb_huffman_decoder_read_value(&literalsDecoder, nLiteralsRevSymbolTable, pBitReader);
      if (nLiteralsCodeword < 256) {
         /* 0..255: literal. copy. */
         if (pCurOutData < pOutDataEnd)
            *pCurOutData++ = nLiteralsCodeword;
         else
            return -1;
      }
      else {
         if (nLiteralsCodeword == NEODMARKERSYM) break;     /* EOD marker, all done */
         if (nLiteralsCodeword == -1) return -1;
         
         /* 256..284: match. decode match length. */

         unsigned int nMatchLen = em_lsb_bitreader_get_bits(pBitReader, (nLiteralsCodeword >> 16) & 15);
         if (nMatchLen == -1) return -1;

         nMatchLen += (nLiteralsCodeword & 0x7fff);

         /* Read match offset */

         unsigned int nOffsetCodeword = em_lsb_huffman_decoder_read_value(&offsetDecoder, nOffsetRevSymbolTable, pBitReader);
         if (nOffsetCodeword == -1) return -1;

         unsigned int nMatchOffset = em_lsb_bitreader_get_bits(pBitReader, (nOffsetCodeword >> 16) & 15);
         if (nMatchOffset == -1) return -1;

         nMatchOffset += (nOffsetCodeword & 0x7fff);

         /* Copy match */

         const unsigned char *pSrc = pCurOutData - nMatchOffset;
         if (pSrc >= pOutData) {
            if (nMatchOffset >= 16 && (pCurOutData + nMatchLen) <= pOutDataFastEnd) {
               const unsigned char *pCopySrc = pSrc;
               unsigned char *pCopyDst = pCurOutData;
               const unsigned char *pCopyEndDst = pCurOutData + nMatchLen;

               do {
                  memcpy(pCopyDst, pCopySrc, 16);
                  pCopySrc += 16;
                  pCopyDst += 16;
               } while (pCopyDst < pCopyEndDst);

               pCurOutData += nMatchLen;
            }
            else {
               if ((pCurOutData + nMatchLen) > pOutDataEnd) return -1;

               while (nMatchLen--) {
                  *pCurOutData++ = *pSrc++;
               }
            }
         }
         else
            return -1;
      }
   }

   return (size_t)(pCurOutData - (pOutData + nOutDataOffset));
}

/*-- zlib adler32 calculation --*/

#ifdef EM_INFLATE_VERIFY_CHECKSUM

/*
  By Mark Adler - https://github.com/madler/zlib/blob/master/adler32.c
 
  Copyright (C) 1995-2017 Jean-loup Gailly and Mark Adler
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu
  The data format used by the zlib library is described by RFCs (Request for
  Comments) 1950 to 1952 in the files http://tools.ietf.org/html/rfc1950
  (zlib format), rfc1951 (deflate format) and rfc1952 (gzip format).
 */

#define BASE 65521U     /* largest prime smaller than 65536 */
#define NMAX 5552
#define DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);
#  define MOD(a) a %= BASE
#  define MOD28(a) a %= BASE
#  define MOD63(a) a %= BASE

static unsigned int em_inflate_adler32_z(unsigned int adler, const unsigned char *buf, size_t len) {
   unsigned long sum2;
   unsigned n;

   /* split Adler-32 into component sums */
   sum2 = (adler >> 16) & 0xffff;
   adler &= 0xffff;

   /* in case user likes doing a byte at a time, keep it fast */
   if (len == 1) {
      adler += buf[0];
      if (adler >= BASE)
         adler -= BASE;
      sum2 += adler;
      if (sum2 >= BASE)
         sum2 -= BASE;
      return adler | (sum2 << 16);
   }

   /* initial Adler-32 value (deferred check for len == 1 speed) */
   if (buf == NULL)
      return 1L;

   /* in case short lengths are provided, keep it somewhat fast */
   if (len < 16) {
      while (len--) {
         adler += *buf++;
         sum2 += adler;
      }
      if (adler >= BASE)
         adler -= BASE;
      MOD28(sum2);            /* only added so many BASE's */
      return adler | (sum2 << 16);
   }

   /* do length NMAX blocks -- requires just one modulo operation */
   while (len >= NMAX) {
      len -= NMAX;
      n = NMAX / 16;          /* NMAX is divisible by 16 */
      do {
         DO16(buf);          /* 16 sums unrolled */
         buf += 16;
      } while (--n);
      MOD(adler);
      MOD(sum2);
   }

   /* do remaining bytes (less than NMAX, still just one modulo) */
   if (len) {                  /* avoid modulos if none remaining */
      while (len >= 16) {
         len -= 16;
         DO16(buf);
         buf += 16;
      }
      while (len--) {
         adler += *buf++;
         sum2 += adler;
      }
      MOD(adler);
      MOD(sum2);
   }

   /* return recombined sums */
   return adler | (sum2 << 16);
}

/*-- gzip crc32 calculation --*/

/**
By Stephen Brumme - https://create.stephan-brumme.com/crc32/

Unless otherwise noted in a file's first 5 lines, all source code published on http://create.stephan-brumme.com and its sub-pages is licensed similar to the zlib license:

This software is provided 'as-is', without any express or implied warranty. In no event will the author be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 */

static const unsigned int ema_inflate_Crc32Lookup[4][256] = {
   {
      0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,
      0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
      0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,
      0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
      0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,
      0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
      0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
      0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
      0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,
      0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
      0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,
      0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
      0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,
      0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
      0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
      0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
      0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,
      0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
      0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,
      0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
      0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
      0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
      0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,
      0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
      0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,
      0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
      0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,
      0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
      0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,
      0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
      0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,
      0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D,
    }

    // beyond this point only relevant for Slicing-by-4, Slicing-by-8 and Slicing-by-16
    ,{
      0x00000000,0x191B3141,0x32366282,0x2B2D53C3,0x646CC504,0x7D77F445,0x565AA786,0x4F4196C7,
      0xC8D98A08,0xD1C2BB49,0xFAEFE88A,0xE3F4D9CB,0xACB54F0C,0xB5AE7E4D,0x9E832D8E,0x87981CCF,
      0x4AC21251,0x53D92310,0x78F470D3,0x61EF4192,0x2EAED755,0x37B5E614,0x1C98B5D7,0x05838496,
      0x821B9859,0x9B00A918,0xB02DFADB,0xA936CB9A,0xE6775D5D,0xFF6C6C1C,0xD4413FDF,0xCD5A0E9E,
      0x958424A2,0x8C9F15E3,0xA7B24620,0xBEA97761,0xF1E8E1A6,0xE8F3D0E7,0xC3DE8324,0xDAC5B265,
      0x5D5DAEAA,0x44469FEB,0x6F6BCC28,0x7670FD69,0x39316BAE,0x202A5AEF,0x0B07092C,0x121C386D,
      0xDF4636F3,0xC65D07B2,0xED705471,0xF46B6530,0xBB2AF3F7,0xA231C2B6,0x891C9175,0x9007A034,
      0x179FBCFB,0x0E848DBA,0x25A9DE79,0x3CB2EF38,0x73F379FF,0x6AE848BE,0x41C51B7D,0x58DE2A3C,
      0xF0794F05,0xE9627E44,0xC24F2D87,0xDB541CC6,0x94158A01,0x8D0EBB40,0xA623E883,0xBF38D9C2,
      0x38A0C50D,0x21BBF44C,0x0A96A78F,0x138D96CE,0x5CCC0009,0x45D73148,0x6EFA628B,0x77E153CA,
      0xBABB5D54,0xA3A06C15,0x888D3FD6,0x91960E97,0xDED79850,0xC7CCA911,0xECE1FAD2,0xF5FACB93,
      0x7262D75C,0x6B79E61D,0x4054B5DE,0x594F849F,0x160E1258,0x0F152319,0x243870DA,0x3D23419B,
      0x65FD6BA7,0x7CE65AE6,0x57CB0925,0x4ED03864,0x0191AEA3,0x188A9FE2,0x33A7CC21,0x2ABCFD60,
      0xAD24E1AF,0xB43FD0EE,0x9F12832D,0x8609B26C,0xC94824AB,0xD05315EA,0xFB7E4629,0xE2657768,
      0x2F3F79F6,0x362448B7,0x1D091B74,0x04122A35,0x4B53BCF2,0x52488DB3,0x7965DE70,0x607EEF31,
      0xE7E6F3FE,0xFEFDC2BF,0xD5D0917C,0xCCCBA03D,0x838A36FA,0x9A9107BB,0xB1BC5478,0xA8A76539,
      0x3B83984B,0x2298A90A,0x09B5FAC9,0x10AECB88,0x5FEF5D4F,0x46F46C0E,0x6DD93FCD,0x74C20E8C,
      0xF35A1243,0xEA412302,0xC16C70C1,0xD8774180,0x9736D747,0x8E2DE606,0xA500B5C5,0xBC1B8484,
      0x71418A1A,0x685ABB5B,0x4377E898,0x5A6CD9D9,0x152D4F1E,0x0C367E5F,0x271B2D9C,0x3E001CDD,
      0xB9980012,0xA0833153,0x8BAE6290,0x92B553D1,0xDDF4C516,0xC4EFF457,0xEFC2A794,0xF6D996D5,
      0xAE07BCE9,0xB71C8DA8,0x9C31DE6B,0x852AEF2A,0xCA6B79ED,0xD37048AC,0xF85D1B6F,0xE1462A2E,
      0x66DE36E1,0x7FC507A0,0x54E85463,0x4DF36522,0x02B2F3E5,0x1BA9C2A4,0x30849167,0x299FA026,
      0xE4C5AEB8,0xFDDE9FF9,0xD6F3CC3A,0xCFE8FD7B,0x80A96BBC,0x99B25AFD,0xB29F093E,0xAB84387F,
      0x2C1C24B0,0x350715F1,0x1E2A4632,0x07317773,0x4870E1B4,0x516BD0F5,0x7A468336,0x635DB277,
      0xCBFAD74E,0xD2E1E60F,0xF9CCB5CC,0xE0D7848D,0xAF96124A,0xB68D230B,0x9DA070C8,0x84BB4189,
      0x03235D46,0x1A386C07,0x31153FC4,0x280E0E85,0x674F9842,0x7E54A903,0x5579FAC0,0x4C62CB81,
      0x8138C51F,0x9823F45E,0xB30EA79D,0xAA1596DC,0xE554001B,0xFC4F315A,0xD7626299,0xCE7953D8,
      0x49E14F17,0x50FA7E56,0x7BD72D95,0x62CC1CD4,0x2D8D8A13,0x3496BB52,0x1FBBE891,0x06A0D9D0,
      0x5E7EF3EC,0x4765C2AD,0x6C48916E,0x7553A02F,0x3A1236E8,0x230907A9,0x0824546A,0x113F652B,
      0x96A779E4,0x8FBC48A5,0xA4911B66,0xBD8A2A27,0xF2CBBCE0,0xEBD08DA1,0xC0FDDE62,0xD9E6EF23,
      0x14BCE1BD,0x0DA7D0FC,0x268A833F,0x3F91B27E,0x70D024B9,0x69CB15F8,0x42E6463B,0x5BFD777A,
      0xDC656BB5,0xC57E5AF4,0xEE530937,0xF7483876,0xB809AEB1,0xA1129FF0,0x8A3FCC33,0x9324FD72,
    },

    {
      0x00000000,0x01C26A37,0x0384D46E,0x0246BE59,0x0709A8DC,0x06CBC2EB,0x048D7CB2,0x054F1685,
      0x0E1351B8,0x0FD13B8F,0x0D9785D6,0x0C55EFE1,0x091AF964,0x08D89353,0x0A9E2D0A,0x0B5C473D,
      0x1C26A370,0x1DE4C947,0x1FA2771E,0x1E601D29,0x1B2F0BAC,0x1AED619B,0x18ABDFC2,0x1969B5F5,
      0x1235F2C8,0x13F798FF,0x11B126A6,0x10734C91,0x153C5A14,0x14FE3023,0x16B88E7A,0x177AE44D,
      0x384D46E0,0x398F2CD7,0x3BC9928E,0x3A0BF8B9,0x3F44EE3C,0x3E86840B,0x3CC03A52,0x3D025065,
      0x365E1758,0x379C7D6F,0x35DAC336,0x3418A901,0x3157BF84,0x3095D5B3,0x32D36BEA,0x331101DD,
      0x246BE590,0x25A98FA7,0x27EF31FE,0x262D5BC9,0x23624D4C,0x22A0277B,0x20E69922,0x2124F315,
      0x2A78B428,0x2BBADE1F,0x29FC6046,0x283E0A71,0x2D711CF4,0x2CB376C3,0x2EF5C89A,0x2F37A2AD,
      0x709A8DC0,0x7158E7F7,0x731E59AE,0x72DC3399,0x7793251C,0x76514F2B,0x7417F172,0x75D59B45,
      0x7E89DC78,0x7F4BB64F,0x7D0D0816,0x7CCF6221,0x798074A4,0x78421E93,0x7A04A0CA,0x7BC6CAFD,
      0x6CBC2EB0,0x6D7E4487,0x6F38FADE,0x6EFA90E9,0x6BB5866C,0x6A77EC5B,0x68315202,0x69F33835,
      0x62AF7F08,0x636D153F,0x612BAB66,0x60E9C151,0x65A6D7D4,0x6464BDE3,0x662203BA,0x67E0698D,
      0x48D7CB20,0x4915A117,0x4B531F4E,0x4A917579,0x4FDE63FC,0x4E1C09CB,0x4C5AB792,0x4D98DDA5,
      0x46C49A98,0x4706F0AF,0x45404EF6,0x448224C1,0x41CD3244,0x400F5873,0x4249E62A,0x438B8C1D,
      0x54F16850,0x55330267,0x5775BC3E,0x56B7D609,0x53F8C08C,0x523AAABB,0x507C14E2,0x51BE7ED5,
      0x5AE239E8,0x5B2053DF,0x5966ED86,0x58A487B1,0x5DEB9134,0x5C29FB03,0x5E6F455A,0x5FAD2F6D,
      0xE1351B80,0xE0F771B7,0xE2B1CFEE,0xE373A5D9,0xE63CB35C,0xE7FED96B,0xE5B86732,0xE47A0D05,
      0xEF264A38,0xEEE4200F,0xECA29E56,0xED60F461,0xE82FE2E4,0xE9ED88D3,0xEBAB368A,0xEA695CBD,
      0xFD13B8F0,0xFCD1D2C7,0xFE976C9E,0xFF5506A9,0xFA1A102C,0xFBD87A1B,0xF99EC442,0xF85CAE75,
      0xF300E948,0xF2C2837F,0xF0843D26,0xF1465711,0xF4094194,0xF5CB2BA3,0xF78D95FA,0xF64FFFCD,
      0xD9785D60,0xD8BA3757,0xDAFC890E,0xDB3EE339,0xDE71F5BC,0xDFB39F8B,0xDDF521D2,0xDC374BE5,
      0xD76B0CD8,0xD6A966EF,0xD4EFD8B6,0xD52DB281,0xD062A404,0xD1A0CE33,0xD3E6706A,0xD2241A5D,
      0xC55EFE10,0xC49C9427,0xC6DA2A7E,0xC7184049,0xC25756CC,0xC3953CFB,0xC1D382A2,0xC011E895,
      0xCB4DAFA8,0xCA8FC59F,0xC8C97BC6,0xC90B11F1,0xCC440774,0xCD866D43,0xCFC0D31A,0xCE02B92D,
      0x91AF9640,0x906DFC77,0x922B422E,0x93E92819,0x96A63E9C,0x976454AB,0x9522EAF2,0x94E080C5,
      0x9FBCC7F8,0x9E7EADCF,0x9C381396,0x9DFA79A1,0x98B56F24,0x99770513,0x9B31BB4A,0x9AF3D17D,
      0x8D893530,0x8C4B5F07,0x8E0DE15E,0x8FCF8B69,0x8A809DEC,0x8B42F7DB,0x89044982,0x88C623B5,
      0x839A6488,0x82580EBF,0x801EB0E6,0x81DCDAD1,0x8493CC54,0x8551A663,0x8717183A,0x86D5720D,
      0xA9E2D0A0,0xA820BA97,0xAA6604CE,0xABA46EF9,0xAEEB787C,0xAF29124B,0xAD6FAC12,0xACADC625,
      0xA7F18118,0xA633EB2F,0xA4755576,0xA5B73F41,0xA0F829C4,0xA13A43F3,0xA37CFDAA,0xA2BE979D,
      0xB5C473D0,0xB40619E7,0xB640A7BE,0xB782CD89,0xB2CDDB0C,0xB30FB13B,0xB1490F62,0xB08B6555,
      0xBBD72268,0xBA15485F,0xB853F606,0xB9919C31,0xBCDE8AB4,0xBD1CE083,0xBF5A5EDA,0xBE9834ED,
    },

    {
      0x00000000,0xB8BC6765,0xAA09C88B,0x12B5AFEE,0x8F629757,0x37DEF032,0x256B5FDC,0x9DD738B9,
      0xC5B428EF,0x7D084F8A,0x6FBDE064,0xD7018701,0x4AD6BFB8,0xF26AD8DD,0xE0DF7733,0x58631056,
      0x5019579F,0xE8A530FA,0xFA109F14,0x42ACF871,0xDF7BC0C8,0x67C7A7AD,0x75720843,0xCDCE6F26,
      0x95AD7F70,0x2D111815,0x3FA4B7FB,0x8718D09E,0x1ACFE827,0xA2738F42,0xB0C620AC,0x087A47C9,
      0xA032AF3E,0x188EC85B,0x0A3B67B5,0xB28700D0,0x2F503869,0x97EC5F0C,0x8559F0E2,0x3DE59787,
      0x658687D1,0xDD3AE0B4,0xCF8F4F5A,0x7733283F,0xEAE41086,0x525877E3,0x40EDD80D,0xF851BF68,
      0xF02BF8A1,0x48979FC4,0x5A22302A,0xE29E574F,0x7F496FF6,0xC7F50893,0xD540A77D,0x6DFCC018,
      0x359FD04E,0x8D23B72B,0x9F9618C5,0x272A7FA0,0xBAFD4719,0x0241207C,0x10F48F92,0xA848E8F7,
      0x9B14583D,0x23A83F58,0x311D90B6,0x89A1F7D3,0x1476CF6A,0xACCAA80F,0xBE7F07E1,0x06C36084,
      0x5EA070D2,0xE61C17B7,0xF4A9B859,0x4C15DF3C,0xD1C2E785,0x697E80E0,0x7BCB2F0E,0xC377486B,
      0xCB0D0FA2,0x73B168C7,0x6104C729,0xD9B8A04C,0x446F98F5,0xFCD3FF90,0xEE66507E,0x56DA371B,
      0x0EB9274D,0xB6054028,0xA4B0EFC6,0x1C0C88A3,0x81DBB01A,0x3967D77F,0x2BD27891,0x936E1FF4,
      0x3B26F703,0x839A9066,0x912F3F88,0x299358ED,0xB4446054,0x0CF80731,0x1E4DA8DF,0xA6F1CFBA,
      0xFE92DFEC,0x462EB889,0x549B1767,0xEC277002,0x71F048BB,0xC94C2FDE,0xDBF98030,0x6345E755,
      0x6B3FA09C,0xD383C7F9,0xC1366817,0x798A0F72,0xE45D37CB,0x5CE150AE,0x4E54FF40,0xF6E89825,
      0xAE8B8873,0x1637EF16,0x048240F8,0xBC3E279D,0x21E91F24,0x99557841,0x8BE0D7AF,0x335CB0CA,
      0xED59B63B,0x55E5D15E,0x47507EB0,0xFFEC19D5,0x623B216C,0xDA874609,0xC832E9E7,0x708E8E82,
      0x28ED9ED4,0x9051F9B1,0x82E4565F,0x3A58313A,0xA78F0983,0x1F336EE6,0x0D86C108,0xB53AA66D,
      0xBD40E1A4,0x05FC86C1,0x1749292F,0xAFF54E4A,0x322276F3,0x8A9E1196,0x982BBE78,0x2097D91D,
      0x78F4C94B,0xC048AE2E,0xD2FD01C0,0x6A4166A5,0xF7965E1C,0x4F2A3979,0x5D9F9697,0xE523F1F2,
      0x4D6B1905,0xF5D77E60,0xE762D18E,0x5FDEB6EB,0xC2098E52,0x7AB5E937,0x680046D9,0xD0BC21BC,
      0x88DF31EA,0x3063568F,0x22D6F961,0x9A6A9E04,0x07BDA6BD,0xBF01C1D8,0xADB46E36,0x15080953,
      0x1D724E9A,0xA5CE29FF,0xB77B8611,0x0FC7E174,0x9210D9CD,0x2AACBEA8,0x38191146,0x80A57623,
      0xD8C66675,0x607A0110,0x72CFAEFE,0xCA73C99B,0x57A4F122,0xEF189647,0xFDAD39A9,0x45115ECC,
      0x764DEE06,0xCEF18963,0xDC44268D,0x64F841E8,0xF92F7951,0x41931E34,0x5326B1DA,0xEB9AD6BF,
      0xB3F9C6E9,0x0B45A18C,0x19F00E62,0xA14C6907,0x3C9B51BE,0x842736DB,0x96929935,0x2E2EFE50,
      0x2654B999,0x9EE8DEFC,0x8C5D7112,0x34E11677,0xA9362ECE,0x118A49AB,0x033FE645,0xBB838120,
      0xE3E09176,0x5B5CF613,0x49E959FD,0xF1553E98,0x6C820621,0xD43E6144,0xC68BCEAA,0x7E37A9CF,
      0xD67F4138,0x6EC3265D,0x7C7689B3,0xC4CAEED6,0x591DD66F,0xE1A1B10A,0xF3141EE4,0x4BA87981,
      0x13CB69D7,0xAB770EB2,0xB9C2A15C,0x017EC639,0x9CA9FE80,0x241599E5,0x36A0360B,0x8E1C516E,
      0x866616A7,0x3EDA71C2,0x2C6FDE2C,0x94D3B949,0x090481F0,0xB1B8E695,0xA30D497B,0x1BB12E1E,
      0x43D23E48,0xFB6E592D,0xE9DBF6C3,0x516791A6,0xCCB0A91F,0x740CCE7A,0x66B96194,0xDE0506F1,
    }
};

// define endianess and some integer data types
#if defined(_MSC_VER) || defined(__MINGW32__)
#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN    4321
#define __BYTE_ORDER    __LITTLE_ENDIAN
#elif defined(__APPLE__)
#include <sys/param.h>
#define __BYTE_ORDER    __DARWIN_BYTE_ORDER
#else
  // defines __BYTE_ORDER as __LITTLE_ENDIAN or __BIG_ENDIAN
#include <sys/param.h>
#endif

/// swap endianess
static inline unsigned int em_inflate_crc32_swap(unsigned int x) {
#if defined(__GNUC__) || defined(__clang__)
   return __builtin_bswap32(x);
#else
   return (x >> 24) |
      ((x >> 8) & 0x0000FF00) |
      ((x << 8) & 0x00FF0000) |
      (x << 24);
#endif
}

static unsigned int em_inflate_crc32_4bytes(const void* data, size_t length, unsigned int previousCrc32) {
   unsigned int  crc = ~previousCrc32; // same as previousCrc32 ^ 0xFFFFFFFF
   const unsigned int* current = (const unsigned int*)data;

   // process four bytes at once (Slicing-by-4)
   while (length >= 4)
   {
#if __BYTE_ORDER == __BIG_ENDIAN
      unsigned int one = *current++ ^ em_inflate_crc32_swap(crc);
      crc = ema_inflate_Crc32Lookup[0][one & 0xFF] ^
         ema_inflate_Crc32Lookup[1][(one >> 8) & 0xFF] ^
         ema_inflate_Crc32Lookup[2][(one >> 16) & 0xFF] ^
         ema_inflate_Crc32Lookup[3][(one >> 24) & 0xFF];
#else
      unsigned int one = *current++ ^ crc;
      crc = ema_inflate_Crc32Lookup[0][(one >> 24) & 0xFF] ^
         ema_inflate_Crc32Lookup[1][(one >> 16) & 0xFF] ^
         ema_inflate_Crc32Lookup[2][(one >> 8) & 0xFF] ^
         ema_inflate_Crc32Lookup[3][one & 0xFF];
#endif

      length -= 4;
   }

   const unsigned char* currentChar = (const unsigned char*)current;
   // remaining 1 to 3 bytes (standard algorithm)
   while (length-- != 0)
      crc = (crc >> 8) ^ ema_inflate_Crc32Lookup[0][(crc & 0xFF) ^ *currentChar++];

   return ~crc; // same as crc ^ 0xFFFFFFFF
}

#endif /* EM_INFLATE_VERIFY_CHECKSUM */

/*-- Top level zlib stream inflater --*/

/* What type of checksum to verify, depending on the stream */
typedef enum { EM_INFLATE_CHECKSUM_NONE = 0, EM_INFLATE_CHECKSUM_GZIP, EM_INFLATE_CHECKSUM_ZLIB } em_inflate_checksum_type_t;

/**
 * Inflate gzip or zlib data
 *
 * @param pCompressedData pointer to start of zlib data
 * @param nCompressedDataSize size of zlib data, in bytes
 * @param pOutData pointer to start of decompression buffer
 * @param nMaxOutDataSize maximum size of decompression buffer, in bytes
 *
 * @return number of bytes decompressed, or -1 in case of an error
 */
size_t em_inflate(const void *pCompressedData, size_t nCompressedDataSize, unsigned char *pOutData, size_t nMaxOutDataSize) {
   const unsigned char *pCurCompressedData = (const unsigned char *)pCompressedData;
   const unsigned char *pEndCompressedData = pCurCompressedData + nCompressedDataSize;
   em_lsb_bitreader_t bitReader;
   unsigned int nIsFinalBlock;
   size_t nCurOutOffset;
   em_inflate_checksum_type_t nCheckSumType = EM_INFLATE_CHECKSUM_NONE;
   unsigned long nCheckSum = 0;

   if ((pCurCompressedData + 2) > pEndCompressedData) return -1;

   /* Check header */
   if (pCurCompressedData[0] == 0x1f && pCurCompressedData[1] == 0x8b) {
      /* gzip wrapper */
      pCurCompressedData += 2;
      if ((pCurCompressedData + 8) > pEndCompressedData || pCurCompressedData[0] != 0x08 /* deflate */)
         return -1;
      pCurCompressedData++;

      /* Read flags and skip over the rest of the header */
      unsigned char flags = *pCurCompressedData++;
      pCurCompressedData += 6;

      if (flags & 0x02) {  /* Part number present */         
         if ((pCurCompressedData + 2) > pEndCompressedData) return -1;
         pCurCompressedData += 2;
      }

      if (flags & 0x04) {  /* Extra field present, starts with two-byte length */        
         if ((pCurCompressedData + 2) > pEndCompressedData) return -1;
         unsigned short nExtraFieldLen = ((unsigned short)pCurCompressedData[0]) | (((unsigned short)pCurCompressedData[1]) << 8);
         pCurCompressedData += 2;

         if ((pCurCompressedData + nExtraFieldLen) > pEndCompressedData) return -1;
         pCurCompressedData += nExtraFieldLen;
      }

      if (flags & 0x08) {  /* Original filename present, zero terminated */         
         do {
            if (pCurCompressedData >= pEndCompressedData) return -1;
            pCurCompressedData++;
         } while (pCurCompressedData[-1]);
      }

      if (flags & 0x10) {  /* File comment present, zero terminated */         
         do {
            if (pCurCompressedData >= pEndCompressedData) return -1;
            pCurCompressedData++;
         } while (pCurCompressedData[-1]);
      }

      if (flags & 0x20) {  /* Encryption header present */
         return -1;
      }

      nCheckSumType = EM_INFLATE_CHECKSUM_GZIP;
   }
   else if ((pCurCompressedData[0] & 0x0f) == 0x08) {
      /* zlib wrapper? */
      unsigned char CMF = pCurCompressedData[0];
      unsigned char FLG = pCurCompressedData[1];
      unsigned short nCheck = FLG | (((unsigned short)CMF) << 8);

      if ((CMF >> 4) <= 7 && (nCheck % 31) == 0) {
         /* Looks like a valid zlib wrapper */
         pCurCompressedData += 2;
         if (FLG & 0x20) { /* Preset dictionary present */            
            if ((pCurCompressedData + 4) > pEndCompressedData) return -1;
            pCurCompressedData += 4;
         }
      }

      nCheckSumType = EM_INFLATE_CHECKSUM_ZLIB;
   }

#ifdef EM_INFLATE_VERIFY_CHECKSUM
   /* Initialize checksum */
   if (nCheckSumType == EM_INFLATE_CHECKSUM_ZLIB)
      nCheckSum = em_inflate_adler32_z(0, NULL, 0);
#endif /* EM_INFLATE_VERIFY_CHECKSUM */

   /* The rest of the data is bit-packed. Initialize bit reader */
   em_lsb_bitreader_init(&bitReader, pCurCompressedData, pEndCompressedData);

   /* Process all the blocks */
   nCurOutOffset = 0;

   do {
      unsigned int nBlockType;
      size_t nBlockResult;

      nIsFinalBlock = em_lsb_bitreader_get_bits(&bitReader, 1);
      nBlockType = em_lsb_bitreader_get_bits(&bitReader, 2);
      
      switch (nBlockType) {
      case 0:  /* Stored */         
         nBlockResult = em_inflate_copy_stored(&bitReader, pOutData, nCurOutOffset, nMaxOutDataSize - nCurOutOffset);
         break;

      case 1:  /* Static huffman */         
         nBlockResult = em_inflate_decompress_block(&bitReader, 0 /* static */, pOutData, nCurOutOffset, nMaxOutDataSize - nCurOutOffset);
         break;

      case 2:  /* Dynamic huffman */         
         nBlockResult = em_inflate_decompress_block(&bitReader, 1 /* dynamic */, pOutData, nCurOutOffset, nMaxOutDataSize - nCurOutOffset);
         break;

      case 3:  /* Invalid */         
         return -1;
      }

      if (nBlockResult == -1) return -1;

#ifdef EM_INFLATE_VERIFY_CHECKSUM
      /* Update checksum with the decompressed block's contents */
      if (nCheckSumType == EM_INFLATE_CHECKSUM_GZIP)
         nCheckSum = em_inflate_crc32_4bytes(pOutData + nCurOutOffset, nBlockResult, nCheckSum);
      else if (nCheckSumType == EM_INFLATE_CHECKSUM_ZLIB)
         nCheckSum = em_inflate_adler32_z(nCheckSum, pOutData + nCurOutOffset, nBlockResult);
#endif /* EM_INFLATE_VERIFY_CHECKSUM */

      nCurOutOffset += nBlockResult;
   } while (!nIsFinalBlock);

   /* Align on a byte again */
   em_lsb_bitreader_byte_align(&bitReader);
   pCurCompressedData = bitReader.pInBlock;

#ifdef EM_INFLATE_VERIFY_CHECKSUM
   /* Read stored checksum and verify it */

   unsigned int nStoredCheckSum;
   switch (nCheckSumType) {
   case EM_INFLATE_CHECKSUM_GZIP:   /* gzip - little endian crc32 */
      if ((pCurCompressedData + 4) > pEndCompressedData) return -1;
      nStoredCheckSum = ((unsigned int)pCurCompressedData[0]);
      nStoredCheckSum |= ((unsigned int)pCurCompressedData[1]) << 8;
      nStoredCheckSum |= ((unsigned int)pCurCompressedData[2]) << 16;
      nStoredCheckSum |= ((unsigned int)pCurCompressedData[3]) << 24;
      if (nStoredCheckSum != nCheckSum) return -1;
      pCurCompressedData += 4;
      break;

   case EM_INFLATE_CHECKSUM_ZLIB:   /* zlib - big endian adler32 */
      if ((pCurCompressedData + 4) > pEndCompressedData) return -1;
      nStoredCheckSum = ((unsigned int)pCurCompressedData[0]) << 24;
      nStoredCheckSum |= ((unsigned int)pCurCompressedData[1]) << 16;
      nStoredCheckSum |= ((unsigned int)pCurCompressedData[2]) << 8;
      nStoredCheckSum |= ((unsigned int)pCurCompressedData[3]);
      if (nStoredCheckSum != nCheckSum) return -1;
      pCurCompressedData += 4;
      break;

   default: /* None */
      break;
   }
#endif /* EM_INFLATE_VERIFY_CHECKSUM */

   /* Success, return decompressed size */
   return nCurOutOffset;
}
