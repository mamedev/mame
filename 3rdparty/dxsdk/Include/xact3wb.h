/***************************************************************************
 *
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 *  File:       xact3wb.h
 *  Content:    XACT 3 wave bank definitions.
 *
 ****************************************************************************/

#ifndef __XACT3WB_H__
#define __XACT3WB_H__

#ifdef _XBOX
#   include <xtl.h>
#else
#   include <math.h>
#endif

#include <audiodefs.h>
#include <xma2defs.h>

#pragma warning(push)
#pragma warning(disable:4201)
#pragma warning(disable:4214)   // nonstandard extension used : bit field types other than int

#pragma pack(push, 1)
#if !defined(_X86_)
    #define XACTUNALIGNED __unaligned
#else
    #define XACTUNALIGNED
#endif

#ifdef _M_PPCBE
#pragma bitfield_order(push, lsb_to_msb)
#endif

#define WAVEBANK_HEADER_SIGNATURE               'DNBW'      // WaveBank  RIFF chunk signature
#define WAVEBANK_HEADER_VERSION                 44          // Current wavebank file version

#define WAVEBANK_BANKNAME_LENGTH                64          // Wave bank friendly name length, in characters
#define WAVEBANK_ENTRYNAME_LENGTH               64          // Wave bank entry friendly name length, in characters

#define WAVEBANK_MAX_DATA_SEGMENT_SIZE          0xFFFFFFFF  // Maximum wave bank data segment size, in bytes
#define WAVEBANK_MAX_COMPACT_DATA_SEGMENT_SIZE  0x001FFFFF  // Maximum compact wave bank data segment size, in bytes

typedef DWORD WAVEBANKOFFSET;

//
// Bank flags
//

#define WAVEBANK_TYPE_BUFFER         0x00000000      // In-memory buffer
#define WAVEBANK_TYPE_STREAMING      0x00000001      // Streaming
#define WAVEBANK_TYPE_MASK           0x00000001

#define WAVEBANK_FLAGS_ENTRYNAMES    0x00010000      // Bank includes entry names
#define WAVEBANK_FLAGS_COMPACT       0x00020000      // Bank uses compact format
#define WAVEBANK_FLAGS_SYNC_DISABLED 0x00040000      // Bank is disabled for audition sync
#define WAVEBANK_FLAGS_SEEKTABLES    0x00080000      // Bank includes seek tables.
#define WAVEBANK_FLAGS_MASK          0x000F0000

//
// Entry flags
//

#define WAVEBANKENTRY_FLAGS_READAHEAD       0x00000001  // Enable stream read-ahead
#define WAVEBANKENTRY_FLAGS_LOOPCACHE       0x00000002  // One or more looping sounds use this wave
#define WAVEBANKENTRY_FLAGS_REMOVELOOPTAIL  0x00000004  // Remove data after the end of the loop region
#define WAVEBANKENTRY_FLAGS_IGNORELOOP      0x00000008  // Used internally when the loop region can't be used
#define WAVEBANKENTRY_FLAGS_MASK            0x00000008

//
// Entry wave format identifiers
//

#define WAVEBANKMINIFORMAT_TAG_PCM      0x0     // PCM data
#define WAVEBANKMINIFORMAT_TAG_XMA      0x1     // XMA data
#define WAVEBANKMINIFORMAT_TAG_ADPCM    0x2     // ADPCM data
#define WAVEBANKMINIFORMAT_TAG_WMA      0x3     // WMA data

#define WAVEBANKMINIFORMAT_BITDEPTH_8   0x0     // 8-bit data (PCM only)
#define WAVEBANKMINIFORMAT_BITDEPTH_16  0x1     // 16-bit data (PCM only)

//
// Arbitrary fixed sizes
//
#define WAVEBANKENTRY_XMASTREAMS_MAX          3   // enough for 5.1 channel audio
#define WAVEBANKENTRY_XMACHANNELS_MAX         6   // enough for 5.1 channel audio (cf. XAUDIOCHANNEL_SOURCEMAX)

//
// DVD data sizes
//

#define WAVEBANK_DVD_SECTOR_SIZE    2048
#define WAVEBANK_DVD_BLOCK_SIZE     (WAVEBANK_DVD_SECTOR_SIZE * 16)

//
// Bank alignment presets
//

#define WAVEBANK_ALIGNMENT_MIN  4                           // Minimum alignment
#define WAVEBANK_ALIGNMENT_DVD  WAVEBANK_DVD_SECTOR_SIZE    // DVD-optimized alignment

//
// Wave bank segment identifiers
//

typedef enum WAVEBANKSEGIDX
{
    WAVEBANK_SEGIDX_BANKDATA = 0,       // Bank data
    WAVEBANK_SEGIDX_ENTRYMETADATA,      // Entry meta-data
    WAVEBANK_SEGIDX_SEEKTABLES,         // Storage for seek tables for the encoded waves.
    WAVEBANK_SEGIDX_ENTRYNAMES,         // Entry friendly names
    WAVEBANK_SEGIDX_ENTRYWAVEDATA,      // Entry wave data
    WAVEBANK_SEGIDX_COUNT
} WAVEBANKSEGIDX, *LPWAVEBANKSEGIDX;

typedef const WAVEBANKSEGIDX *LPCWAVEBANKSEGIDX;

//
// Endianness
//

#ifdef __cplusplus

namespace XACTWaveBank
{
    __inline void SwapBytes(XACTUNALIGNED DWORD &dw)
    {

#ifdef _X86_

        __asm
        {
            mov edi, dw
            mov eax, [edi]
            bswap eax
            mov [edi], eax
        }

#else // _X86_

        dw = _byteswap_ulong(dw);

#endif // _X86_

    }

    __inline void SwapBytes(XACTUNALIGNED WORD &w)
    {

#ifdef _X86_

        __asm
        {
            mov edi, w
            mov ax, [edi]
            xchg ah, al
            mov [edi], ax
        }

#else // _X86_

        w = _byteswap_ushort(w);

#endif // _X86_

    }

}

#endif // __cplusplus

//
// Wave bank region in bytes.
//

typedef struct WAVEBANKREGION
{
    DWORD       dwOffset;               // Region offset, in bytes.
    DWORD       dwLength;               // Region length, in bytes.

#ifdef __cplusplus

    void SwapBytes(void)
    {
        XACTWaveBank::SwapBytes(dwOffset);
        XACTWaveBank::SwapBytes(dwLength);
    }

#endif // __cplusplus

} WAVEBANKREGION, *LPWAVEBANKREGION;

typedef const WAVEBANKREGION *LPCWAVEBANKREGION;


//
// Wave bank region in samples.
//

typedef struct WAVEBANKSAMPLEREGION
{
    DWORD       dwStartSample;          // Start sample for the region.
    DWORD       dwTotalSamples;         // Region length in samples.

#ifdef __cplusplus

    void SwapBytes(void)
    {
        XACTWaveBank::SwapBytes(dwStartSample);
        XACTWaveBank::SwapBytes(dwTotalSamples);
    }

#endif // __cplusplus

} WAVEBANKSAMPLEREGION, *LPWAVEBANKSAMPLEREGION;

typedef const WAVEBANKSAMPLEREGION *LPCWAVEBANKSAMPLEREGION;


//
// Wave bank file header
//

typedef struct WAVEBANKHEADER
{
    DWORD           dwSignature;                        // File signature
    DWORD           dwVersion;                          // Version of the tool that created the file
    DWORD           dwHeaderVersion;                    // Version of the file format
    WAVEBANKREGION  Segments[WAVEBANK_SEGIDX_COUNT];    // Segment lookup table

#ifdef __cplusplus

    void SwapBytes(void)
    {
        XACTWaveBank::SwapBytes(dwSignature);
        XACTWaveBank::SwapBytes(dwVersion);
        XACTWaveBank::SwapBytes(dwHeaderVersion);

        for(int i = 0; i < WAVEBANK_SEGIDX_COUNT; i++)
        {
            Segments[i].SwapBytes();
        }
    }

#endif // __cplusplus

} WAVEBANKHEADER, *LPWAVEBANKHEADER;

typedef const WAVEBANKHEADER *LPCWAVEBANKHEADER;

//
// Table for converting WMA Average Bytes per Second values to the WAVEBANKMINIWAVEFORMAT wBlockAlign field
// NOTE: There can be a max of 8 values in the table.
//

#define MAX_WMA_AVG_BYTES_PER_SEC_ENTRIES 7

static const DWORD aWMAAvgBytesPerSec[] =
{
    12000,
    24000,
    4000,
    6000,
    8000,
    20000,
    2500
};
// bitrate = entry * 8

//
// Table for converting WMA Block Align values to the WAVEBANKMINIWAVEFORMAT wBlockAlign field
// NOTE: There can be a max of 32 values in the table.
//

#define MAX_WMA_BLOCK_ALIGN_ENTRIES 17

static const DWORD aWMABlockAlign[] =
{
    929,
    1487,
    1280,
    2230,
    8917,
    8192,
    4459,
    5945,
    2304,
    1536,
    1485,
    1008,
    2731,
    4096,
    6827,
    5462,
    1280
};

struct WAVEBANKENTRY;

//
// Entry compressed data format
//

typedef union WAVEBANKMINIWAVEFORMAT
{
    struct
    {
        DWORD       wFormatTag      : 2;        // Format tag
        DWORD       nChannels       : 3;        // Channel count (1 - 6)
        DWORD       nSamplesPerSec  : 18;       // Sampling rate
        DWORD       wBlockAlign     : 8;        // Block alignment.  For WMA, lower 6 bits block alignment index, upper 2 bits bytes-per-second index.
        DWORD       wBitsPerSample  : 1;        // Bits per sample (8 vs. 16, PCM only); WMAudio2/WMAudio3 (for WMA)
    };

    DWORD           dwValue;

#ifdef __cplusplus

    void SwapBytes(void)
    {
        XACTWaveBank::SwapBytes(dwValue);
    }

    WORD BitsPerSample() const
    {
        if (wFormatTag == WAVEBANKMINIFORMAT_TAG_XMA)
            return XMA_OUTPUT_SAMPLE_BITS; // First, because most common on Xbox 360
        if (wFormatTag == WAVEBANKMINIFORMAT_TAG_WMA)
            return 16;
        if (wFormatTag == WAVEBANKMINIFORMAT_TAG_ADPCM)
            return 4; // MSADPCM_BITS_PER_SAMPLE == 4

        // wFormatTag must be WAVEBANKMINIFORMAT_TAG_PCM (2 bits can only represent 4 different values)
        return (wBitsPerSample == WAVEBANKMINIFORMAT_BITDEPTH_16) ? 16 : 8;
    }

    #define ADPCM_MINIWAVEFORMAT_BLOCKALIGN_CONVERSION_OFFSET 22
    DWORD BlockAlign() const
    {
        DWORD dwReturn = 0;

        switch (wFormatTag)
        {
        case WAVEBANKMINIFORMAT_TAG_PCM:
            dwReturn = wBlockAlign;
            break;

        case WAVEBANKMINIFORMAT_TAG_XMA:
            dwReturn = nChannels * XMA_OUTPUT_SAMPLE_BITS / 8;
            break;

        case WAVEBANKMINIFORMAT_TAG_ADPCM:
            dwReturn = (wBlockAlign + ADPCM_MINIWAVEFORMAT_BLOCKALIGN_CONVERSION_OFFSET) * nChannels;
            break;

        case WAVEBANKMINIFORMAT_TAG_WMA:
            {
                DWORD dwBlockAlignIndex = wBlockAlign & 0x1F;
                if (dwBlockAlignIndex < MAX_WMA_BLOCK_ALIGN_ENTRIES)
                        dwReturn = aWMABlockAlign[dwBlockAlignIndex];
            }
            break;
        }

        return dwReturn;
    }

    DWORD AvgBytesPerSec() const
    {
        DWORD dwReturn = 0;

        switch (wFormatTag)
        {
        case WAVEBANKMINIFORMAT_TAG_PCM:
        case WAVEBANKMINIFORMAT_TAG_XMA:
            dwReturn = nSamplesPerSec * wBlockAlign;
            break;

        case WAVEBANKMINIFORMAT_TAG_ADPCM:
            {
                DWORD blockAlign = BlockAlign();
                DWORD samplesPerAdpcmBlock = AdpcmSamplesPerBlock();
                dwReturn = blockAlign * nSamplesPerSec / samplesPerAdpcmBlock;
            }
            break;

        case WAVEBANKMINIFORMAT_TAG_WMA:
            {
                DWORD dwBytesPerSecIndex = wBlockAlign >> 5;
                if (dwBytesPerSecIndex < MAX_WMA_AVG_BYTES_PER_SEC_ENTRIES)
                    dwReturn = aWMAAvgBytesPerSec[dwBytesPerSecIndex];
            }
            break;
        }

        return dwReturn;
    }

    DWORD EncodeWMABlockAlign(DWORD dwBlockAlign, DWORD dwAvgBytesPerSec) const
    {
        DWORD dwReturn = 0;
        DWORD dwBlockAlignIndex = 0;
        DWORD dwBytesPerSecIndex = 0;

        for (; dwBlockAlignIndex < MAX_WMA_BLOCK_ALIGN_ENTRIES && dwBlockAlign != aWMABlockAlign[dwBlockAlignIndex]; dwBlockAlignIndex++);

        if (dwBlockAlignIndex < MAX_WMA_BLOCK_ALIGN_ENTRIES)
        {
            for (; dwBytesPerSecIndex < MAX_WMA_AVG_BYTES_PER_SEC_ENTRIES && dwAvgBytesPerSec != aWMAAvgBytesPerSec[dwBytesPerSecIndex]; dwBytesPerSecIndex++);

            if (dwBytesPerSecIndex < MAX_WMA_AVG_BYTES_PER_SEC_ENTRIES)
            {
                dwReturn = dwBlockAlignIndex | (dwBytesPerSecIndex << 5);
            }
        }

        return dwReturn;
    }


    void XMA2FillFormatEx(XMA2WAVEFORMATEX *fmt, WORD blockCount, const struct WAVEBANKENTRY* entry) const;

    DWORD AdpcmSamplesPerBlock() const
    {
        DWORD nBlockAlign = (wBlockAlign + ADPCM_MINIWAVEFORMAT_BLOCKALIGN_CONVERSION_OFFSET) * nChannels;
        return nBlockAlign * 2 / (DWORD)nChannels - 12;
    }

    void AdpcmFillCoefficientTable(ADPCMWAVEFORMAT *fmt) const
    {
        // These are fixed since we are always using MS ADPCM
        fmt->wNumCoef = 7; /* MSADPCM_NUM_COEFFICIENTS */

        static ADPCMCOEFSET aCoef[7] = { { 256, 0}, {512, -256}, {0,0}, {192,64}, {240,0}, {460, -208}, {392,-232} };
        memcpy( &fmt->aCoef, aCoef, sizeof(aCoef) );
    }

#endif // __cplusplus

} WAVEBANKMINIWAVEFORMAT, *LPWAVEBANKMINIWAVEFORMAT;

typedef const WAVEBANKMINIWAVEFORMAT *LPCWAVEBANKMINIWAVEFORMAT;

//
// Entry meta-data
//

typedef struct WAVEBANKENTRY
{
    union
    {
        struct
        {
            // Entry flags
            DWORD                   dwFlags  :  4;

            // Duration of the wave, in units of one sample.
            // For instance, a ten second long wave sampled
            // at 48KHz would have a duration of 480,000.
            // This value is not affected by the number of
            // channels, the number of bits per sample, or the
            // compression format of the wave.
            DWORD                   Duration : 28;
        };
        DWORD dwFlagsAndDuration;
    };

    WAVEBANKMINIWAVEFORMAT  Format;         // Entry format.
    WAVEBANKREGION          PlayRegion;     // Region within the wave data segment that contains this entry.
    WAVEBANKSAMPLEREGION    LoopRegion;     // Region within the wave data (in samples) that should loop.

#ifdef __cplusplus

    void SwapBytes(void)
    {
        XACTWaveBank::SwapBytes(dwFlagsAndDuration);
        Format.SwapBytes();
        PlayRegion.SwapBytes();
        LoopRegion.SwapBytes();
    }

#endif // __cplusplus

} WAVEBANKENTRY, *LPWAVEBANKENTRY;

typedef const WAVEBANKENTRY *LPCWAVEBANKENTRY;

//
// Compact entry meta-data
//

typedef struct WAVEBANKENTRYCOMPACT
{
    DWORD       dwOffset            : 21;       // Data offset, in sectors
    DWORD       dwLengthDeviation   : 11;       // Data length deviation, in bytes

#ifdef __cplusplus

    void SwapBytes(void)
    {
        XACTWaveBank::SwapBytes(*(LPDWORD)this);
    }

#endif // __cplusplus

} WAVEBANKENTRYCOMPACT, *LPWAVEBANKENTRYCOMPACT;

typedef const WAVEBANKENTRYCOMPACT *LPCWAVEBANKENTRYCOMPACT;

//
// Bank data segment
//

typedef struct WAVEBANKDATA
{
    DWORD                   dwFlags;                                // Bank flags
    DWORD                   dwEntryCount;                           // Number of entries in the bank
    CHAR                    szBankName[WAVEBANK_BANKNAME_LENGTH];   // Bank friendly name
    DWORD                   dwEntryMetaDataElementSize;             // Size of each entry meta-data element, in bytes
    DWORD                   dwEntryNameElementSize;                 // Size of each entry name element, in bytes
    DWORD                   dwAlignment;                            // Entry alignment, in bytes
    WAVEBANKMINIWAVEFORMAT  CompactFormat;                          // Format data for compact bank
    FILETIME                BuildTime;                              // Build timestamp

#ifdef __cplusplus

    void SwapBytes(void)
    {
        XACTWaveBank::SwapBytes(dwFlags);
        XACTWaveBank::SwapBytes(dwEntryCount);
        XACTWaveBank::SwapBytes(dwEntryMetaDataElementSize);
        XACTWaveBank::SwapBytes(dwEntryNameElementSize);
        XACTWaveBank::SwapBytes(dwAlignment);
        CompactFormat.SwapBytes();
        XACTWaveBank::SwapBytes(BuildTime.dwLowDateTime);
        XACTWaveBank::SwapBytes(BuildTime.dwHighDateTime);
    }

#endif // __cplusplus

} WAVEBANKDATA, *LPWAVEBANKDATA;

typedef const WAVEBANKDATA *LPCWAVEBANKDATA;

inline void WAVEBANKMINIWAVEFORMAT::XMA2FillFormatEx(XMA2WAVEFORMATEX *fmt, WORD blockCount, const WAVEBANKENTRY* entry) const
{
    // Note caller is responsbile for filling out fmt->wfx with other helper functions.

    fmt->NumStreams = (WORD)( (nChannels + 1) / 2 );

    switch (nChannels)
    {
        case 1: fmt->ChannelMask =  SPEAKER_MONO; break;
        case 2: fmt->ChannelMask =  SPEAKER_STEREO; break;
        case 3: fmt->ChannelMask =  SPEAKER_2POINT1; break;
        case 4: fmt->ChannelMask =  SPEAKER_QUAD; break;
        case 5: fmt->ChannelMask =  SPEAKER_4POINT1; break;
        case 6: fmt->ChannelMask =  SPEAKER_5POINT1; break;
        case 7: fmt->ChannelMask =  SPEAKER_5POINT1 | SPEAKER_BACK_CENTER; break;
        case 8: fmt->ChannelMask =  SPEAKER_7POINT1; break;
        default: fmt->ChannelMask = 0; break;
    }

    fmt->SamplesEncoded = entry->Duration;
    fmt->BytesPerBlock = 65536; /* XACT_FIXED_XMA_BLOCK_SIZE */

    fmt->PlayBegin = entry->PlayRegion.dwOffset;
    fmt->PlayLength = entry->PlayRegion.dwLength;

    if (entry->LoopRegion.dwTotalSamples > 0)
    {
        fmt->LoopBegin = entry->LoopRegion.dwStartSample;
        fmt->LoopLength = entry->LoopRegion.dwTotalSamples;
        fmt->LoopCount = 0xff; /* XACTLOOPCOUNT_INFINITE */
    }
    else
    {
        fmt->LoopBegin = 0;
        fmt->LoopLength = 0;
        fmt->LoopCount = 0;
    }

    fmt->EncoderVersion = 4; // XMAENCODER_VERSION_XMA2

    fmt->BlockCount = blockCount;
}

#ifdef _M_PPCBE
#pragma bitfield_order(pop)
#endif

#pragma warning(pop)
#pragma pack(pop)

#endif // __XACTWB_H__

