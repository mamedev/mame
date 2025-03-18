// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    CD-i Mono-I CDIC MCU simulation
    -------------------

    written by Ryan Holtz


*******************************************************************************

STATUS:

- Just enough for the Mono-I CD-i board to work somewhat properly.

TODO:

- Work out more low-level functionality.

*******************************************************************************/

#include "emu.h"
#include "cdicdic.h"

#include "cdrom.h"
#include "sound/cdda.h"

#define LOG_DECODES     (1U << 1)
#define LOG_SAMPLES     (1U << 2)
#define LOG_COMMANDS    (1U << 3)
#define LOG_SECTORS     (1U << 4)
#define LOG_IRQS        (1U << 5)
#define LOG_READS       (1U << 6)
#define LOG_WRITES      (1U << 7)
#define LOG_UNKNOWNS    (1U << 8)
#define LOG_RAM         (1U << 9)
#define LOG_ALL         (LOG_DECODES | LOG_SAMPLES | LOG_COMMANDS | LOG_SECTORS | LOG_IRQS | LOG_READS | LOG_WRITES | LOG_UNKNOWNS | LOG_RAM)

#define VERBOSE         (0)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(CDI_CDIC, cdicdic_device, "cdicdic", "CD-i CDIC")

//**************************************************************************
//  STATIC MEMBERS
//**************************************************************************

const int16_t cdicdic_device::s_xa_filter_coef[4][2] =
{
	{ 0x000,  0x000 },
	{ 0x0F0,  0x000 },
	{ 0x1CC, -0x0D0 },
	{ 0x188, -0x0DC }
};

const int32_t cdicdic_device::s_samples_per_sector = 18 * 28 * 2;

const uint16_t cdicdic_device::s_crc_ccitt_table[256] =
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

#define CRC_CCITT_ROUND(accum, data) (((accum << 8) | data) ^ s_crc_ccitt_table[accum >> 8])

const uint8_t cdicdic_device::s_sector_scramble[2448] =
{
	// Sector sync area is not scrambled
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	// Remaining data is scrambled
	0x01, 0x80, 0x00, 0x60, 0x00, 0x28, 0x00, 0x1e, 0x80, 0x08, 0x60, 0x06, 0xa8, 0x02, 0xfe, 0x81,
	0x80, 0x60, 0x60, 0x28, 0x28, 0x1e, 0x9e, 0x88, 0x68, 0x66, 0xae, 0xaa, 0xfc, 0x7f, 0x01, 0xe0,
	0x00, 0x48, 0x00, 0x36, 0x80, 0x16, 0xe0, 0x0e, 0xc8, 0x04, 0x56, 0x83, 0x7e, 0xe1, 0xe0, 0x48,
	0x48, 0x36, 0xb6, 0x96, 0xf6, 0xee, 0xc6, 0xcc, 0x52, 0xd5, 0xfd, 0x9f, 0x01, 0xa8, 0x00, 0x7e,
	0x80, 0x20, 0x60, 0x18, 0x28, 0x0a, 0x9e, 0x87, 0x28, 0x62, 0x9e, 0xa9, 0xa8, 0x7e, 0xfe, 0xa0,
	0x40, 0x78, 0x30, 0x22, 0x94, 0x19, 0xaf, 0x4a, 0xfc, 0x37, 0x01, 0xd6, 0x80, 0x5e, 0xe0, 0x38,
	0x48, 0x12, 0xb6, 0x8d, 0xb6, 0xe5, 0xb6, 0xcb, 0x36, 0xd7, 0x56, 0xde, 0xbe, 0xd8, 0x70, 0x5a,
	0xa4, 0x3b, 0x3b, 0x53, 0x53, 0x7d, 0xfd, 0xe1, 0x81, 0x88, 0x60, 0x66, 0xa8, 0x2a, 0xfe, 0x9f,
	0x00, 0x68, 0x00, 0x2e, 0x80, 0x1c, 0x60, 0x09, 0xe8, 0x06, 0xce, 0x82, 0xd4, 0x61, 0x9f, 0x68,
	0x68, 0x2e, 0xae, 0x9c, 0x7c, 0x69, 0xe1, 0xee, 0xc8, 0x4c, 0x56, 0xb5, 0xfe, 0xf7, 0x00, 0x46,
	0x80, 0x32, 0xe0, 0x15, 0x88, 0x0f, 0x26, 0x84, 0x1a, 0xe3, 0x4b, 0x09, 0xf7, 0x46, 0xc6, 0xb2,
	0xd2, 0xf5, 0x9d, 0x87, 0x29, 0xa2, 0x9e, 0xf9, 0xa8, 0x42, 0xfe, 0xb1, 0x80, 0x74, 0x60, 0x27,
	0x68, 0x1a, 0xae, 0x8b, 0x3c, 0x67, 0x51, 0xea, 0xbc, 0x4f, 0x31, 0xf4, 0x14, 0x47, 0x4f, 0x72,
	0xb4, 0x25, 0xb7, 0x5b, 0x36, 0xbb, 0x56, 0xf3, 0x7e, 0xc5, 0xe0, 0x53, 0x08, 0x3d, 0xc6, 0x91,
	0x92, 0xec, 0x6d, 0x8d, 0xed, 0xa5, 0x8d, 0xbb, 0x25, 0xb3, 0x5b, 0x35, 0xfb, 0x57, 0x03, 0x7e,
	0x81, 0xe0, 0x60, 0x48, 0x28, 0x36, 0x9e, 0x96, 0xe8, 0x6e, 0xce, 0xac, 0x54, 0x7d, 0xff, 0x61,
	0x80, 0x28, 0x60, 0x1e, 0xa8, 0x08, 0x7e, 0x86, 0xa0, 0x62, 0xf8, 0x29, 0x82, 0x9e, 0xe1, 0xa8,
	0x48, 0x7e, 0xb6, 0xa0, 0x76, 0xf8, 0x26, 0xc2, 0x9a, 0xd1, 0xab, 0x1c, 0x7f, 0x49, 0xe0, 0x36,
	0xc8, 0x16, 0xd6, 0x8e, 0xde, 0xe4, 0x58, 0x4b, 0x7a, 0xb7, 0x63, 0x36, 0xa9, 0xd6, 0xfe, 0xde,
	0xc0, 0x58, 0x50, 0x3a, 0xbc, 0x13, 0x31, 0xcd, 0xd4, 0x55, 0x9f, 0x7f, 0x28, 0x20, 0x1e, 0x98,
	0x08, 0x6a, 0x86, 0xaf, 0x22, 0xfc, 0x19, 0x81, 0xca, 0xe0, 0x57, 0x08, 0x3e, 0x86, 0x90, 0x62,
	0xec, 0x29, 0x8d, 0xde, 0xe5, 0x98, 0x4b, 0x2a, 0xb7, 0x5f, 0x36, 0xb8, 0x16, 0xf2, 0x8e, 0xc5,
	0xa4, 0x53, 0x3b, 0x7d, 0xd3, 0x61, 0x9d, 0xe8, 0x69, 0x8e, 0xae, 0xe4, 0x7c, 0x4b, 0x61, 0xf7,
	0x68, 0x46, 0xae, 0xb2, 0xfc, 0x75, 0x81, 0xe7, 0x20, 0x4a, 0x98, 0x37, 0x2a, 0x96, 0x9f, 0x2e,
	0xe8, 0x1c, 0x4e, 0x89, 0xf4, 0x66, 0xc7, 0x6a, 0xd2, 0xaf, 0x1d, 0xbc, 0x09, 0xb1, 0xc6, 0xf4,
	0x52, 0xc7, 0x7d, 0x92, 0xa1, 0xad, 0xb8, 0x7d, 0xb2, 0xa1, 0xb5, 0xb8, 0x77, 0x32, 0xa6, 0x95,
	0xba, 0xef, 0x33, 0x0c, 0x15, 0xc5, 0xcf, 0x13, 0x14, 0x0d, 0xcf, 0x45, 0x94, 0x33, 0x2f, 0x55,
	0xdc, 0x3f, 0x19, 0xd0, 0x0a, 0xdc, 0x07, 0x19, 0xc2, 0x8a, 0xd1, 0xa7, 0x1c, 0x7a, 0x89, 0xe3,
	0x26, 0xc9, 0xda, 0xd6, 0xdb, 0x1e, 0xdb, 0x48, 0x5b, 0x76, 0xbb, 0x66, 0xf3, 0x6a, 0xc5, 0xef,
	0x13, 0x0c, 0x0d, 0xc5, 0xc5, 0x93, 0x13, 0x2d, 0xcd, 0xdd, 0x95, 0x99, 0xaf, 0x2a, 0xfc, 0x1f,
	0x01, 0xc8, 0x00, 0x56, 0x80, 0x3e, 0xe0, 0x10, 0x48, 0x0c, 0x36, 0x85, 0xd6, 0xe3, 0x1e, 0xc9,
	0xc8, 0x56, 0xd6, 0xbe, 0xde, 0xf0, 0x58, 0x44, 0x3a, 0xb3, 0x53, 0x35, 0xfd, 0xd7, 0x01, 0x9e,
	0x80, 0x68, 0x60, 0x2e, 0xa8, 0x1c, 0x7e, 0x89, 0xe0, 0x66, 0xc8, 0x2a, 0xd6, 0x9f, 0x1e, 0xe8,
	0x08, 0x4e, 0x86, 0xb4, 0x62, 0xf7, 0x69, 0x86, 0xae, 0xe2, 0xfc, 0x49, 0x81, 0xf6, 0xe0, 0x46,
	0xc8, 0x32, 0xd6, 0x95, 0x9e, 0xef, 0x28, 0x4c, 0x1e, 0xb5, 0xc8, 0x77, 0x16, 0xa6, 0x8e, 0xfa,
	0xe4, 0x43, 0x0b, 0x71, 0xc7, 0x64, 0x52, 0xab, 0x7d, 0xbf, 0x61, 0xb0, 0x28, 0x74, 0x1e, 0xa7,
	0x48, 0x7a, 0xb6, 0xa3, 0x36, 0xf9, 0xd6, 0xc2, 0xde, 0xd1, 0x98, 0x5c, 0x6a, 0xb9, 0xef, 0x32,
	0xcc, 0x15, 0x95, 0xcf, 0x2f, 0x14, 0x1c, 0x0f, 0x49, 0xc4, 0x36, 0xd3, 0x56, 0xdd, 0xfe, 0xd9,
	0x80, 0x5a, 0xe0, 0x3b, 0x08, 0x13, 0x46, 0x8d, 0xf2, 0xe5, 0x85, 0x8b, 0x23, 0x27, 0x59, 0xda,
	0xba, 0xdb, 0x33, 0x1b, 0x55, 0xcb, 0x7f, 0x17, 0x60, 0x0e, 0xa8, 0x04, 0x7e, 0x83, 0x60, 0x61,
	0xe8, 0x28, 0x4e, 0x9e, 0xb4, 0x68, 0x77, 0x6e, 0xa6, 0xac, 0x7a, 0xfd, 0xe3, 0x01, 0x89, 0xc0,
	0x66, 0xd0, 0x2a, 0xdc, 0x1f, 0x19, 0xc8, 0x0a, 0xd6, 0x87, 0x1e, 0xe2, 0x88, 0x49, 0xa6, 0xb6,
	0xfa, 0xf6, 0xc3, 0x06, 0xd1, 0xc2, 0xdc, 0x51, 0x99, 0xfc, 0x6a, 0xc1, 0xef, 0x10, 0x4c, 0x0c,
	0x35, 0xc5, 0xd7, 0x13, 0x1e, 0x8d, 0xc8, 0x65, 0x96, 0xab, 0x2e, 0xff, 0x5c, 0x40, 0x39, 0xf0,
	0x12, 0xc4, 0x0d, 0x93, 0x45, 0xad, 0xf3, 0x3d, 0x85, 0xd1, 0xa3, 0x1c, 0x79, 0xc9, 0xe2, 0xd6,
	0xc9, 0x9e, 0xd6, 0xe8, 0x5e, 0xce, 0xb8, 0x54, 0x72, 0xbf, 0x65, 0xb0, 0x2b, 0x34, 0x1f, 0x57,
	0x48, 0x3e, 0xb6, 0x90, 0x76, 0xec, 0x26, 0xcd, 0xda, 0xd5, 0x9b, 0x1f, 0x2b, 0x48, 0x1f, 0x76,
	0x88, 0x26, 0xe6, 0x9a, 0xca, 0xeb, 0x17, 0x0f, 0x4e, 0x84, 0x34, 0x63, 0x57, 0x69, 0xfe, 0xae,
	0xc0, 0x7c, 0x50, 0x21, 0xfc, 0x18, 0x41, 0xca, 0xb0, 0x57, 0x34, 0x3e, 0x97, 0x50, 0x6e, 0xbc,
	0x2c, 0x71, 0xdd, 0xe4, 0x59, 0x8b, 0x7a, 0xe7, 0x63, 0x0a, 0xa9, 0xc7, 0x3e, 0xd2, 0x90, 0x5d,
	0xac, 0x39, 0xbd, 0xd2, 0xf1, 0x9d, 0x84, 0x69, 0xa3, 0x6e, 0xf9, 0xec, 0x42, 0xcd, 0xf1, 0x95,
	0x84, 0x6f, 0x23, 0x6c, 0x19, 0xed, 0xca, 0xcd, 0x97, 0x15, 0xae, 0x8f, 0x3c, 0x64, 0x11, 0xeb,
	0x4c, 0x4f, 0x75, 0xf4, 0x27, 0x07, 0x5a, 0x82, 0xbb, 0x21, 0xb3, 0x58, 0x75, 0xfa, 0xa7, 0x03,
	0x3a, 0x81, 0xd3, 0x20, 0x5d, 0xd8, 0x39, 0x9a, 0x92, 0xeb, 0x2d, 0x8f, 0x5d, 0xa4, 0x39, 0xbb,
	0x52, 0xf3, 0x7d, 0x85, 0xe1, 0xa3, 0x08, 0x79, 0xc6, 0xa2, 0xd2, 0xf9, 0x9d, 0x82, 0xe9, 0xa1,
	0x8e, 0xf8, 0x64, 0x42, 0xab, 0x71, 0xbf, 0x64, 0x70, 0x2b, 0x64, 0x1f, 0x6b, 0x48, 0x2f, 0x76,
	0x9c, 0x26, 0xe9, 0xda, 0xce, 0xdb, 0x14, 0x5b, 0x4f, 0x7b, 0x74, 0x23, 0x67, 0x59, 0xea, 0xba,
	0xcf, 0x33, 0x14, 0x15, 0xcf, 0x4f, 0x14, 0x34, 0x0f, 0x57, 0x44, 0x3e, 0xb3, 0x50, 0x75, 0xfc,
	0x27, 0x01, 0xda, 0x80, 0x5b, 0x20, 0x3b, 0x58, 0x13, 0x7a, 0x8d, 0xe3, 0x25, 0x89, 0xdb, 0x26,
	0xdb, 0x5a, 0xdb, 0x7b, 0x1b, 0x63, 0x4b, 0x69, 0xf7, 0x6e, 0xc6, 0xac, 0x52, 0xfd, 0xfd, 0x81,
	0x81, 0xa0, 0x60, 0x78, 0x28, 0x22, 0x9e, 0x99, 0xa8, 0x6a, 0xfe, 0xaf, 0x00, 0x7c, 0x00, 0x21,
	0xc0, 0x18, 0x50, 0x0a, 0xbc, 0x07, 0x31, 0xc2, 0x94, 0x51, 0xaf, 0x7c, 0x7c, 0x21, 0xe1, 0xd8,
	0x48, 0x5a, 0xb6, 0xbb, 0x36, 0xf3, 0x56, 0xc5, 0xfe, 0xd3, 0x00, 0x5d, 0xc0, 0x39, 0x90, 0x12,
	0xec, 0x0d, 0x8d, 0xc5, 0xa5, 0x93, 0x3b, 0x2d, 0xd3, 0x5d, 0x9d, 0xf9, 0xa9, 0x82, 0xfe, 0xe1,
	0x80, 0x48, 0x60, 0x36, 0xa8, 0x16, 0xfe, 0x8e, 0xc0, 0x64, 0x50, 0x2b, 0x7c, 0x1f, 0x61, 0xc8,
	0x28, 0x56, 0x9e, 0xbe, 0xe8, 0x70, 0x4e, 0xa4, 0x34, 0x7b, 0x57, 0x63, 0x7e, 0xa9, 0xe0, 0x7e,
	0xc8, 0x20, 0x56, 0x98, 0x3e, 0xea, 0x90, 0x4f, 0x2c, 0x34, 0x1d, 0xd7, 0x49, 0x9e, 0xb6, 0xe8,
	0x76, 0xce, 0xa6, 0xd4, 0x7a, 0xdf, 0x63, 0x18, 0x29, 0xca, 0x9e, 0xd7, 0x28, 0x5e, 0x9e, 0xb8,
	0x68, 0x72, 0xae, 0xa5, 0xbc, 0x7b, 0x31, 0xe3, 0x54, 0x49, 0xff, 0x76, 0xc0, 0x26, 0xd0, 0x1a,
	0xdc, 0x0b, 0x19, 0xc7, 0x4a, 0xd2, 0xb7, 0x1d, 0xb6, 0x89, 0xb6, 0xe6, 0xf6, 0xca, 0xc6, 0xd7,
	0x12, 0xde, 0x8d, 0x98, 0x65, 0xaa, 0xab, 0x3f, 0x3f, 0x50, 0x10, 0x3c, 0x0c, 0x11, 0xc5, 0xcc,
	0x53, 0x15, 0xfd, 0xcf, 0x01, 0x94, 0x00, 0x6f, 0x40, 0x2c, 0x30, 0x1d, 0xd4, 0x09, 0x9f, 0x46,
	0xe8, 0x32, 0xce, 0x95, 0x94, 0x6f, 0x2f, 0x6c, 0x1c, 0x2d, 0xc9, 0xdd, 0x96, 0xd9, 0xae, 0xda,
	0xfc, 0x5b, 0x01, 0xfb, 0x40, 0x43, 0x70, 0x31, 0xe4, 0x14, 0x4b, 0x4f, 0x77, 0x74, 0x26, 0xa7,
	0x5a, 0xfa, 0xbb, 0x03, 0x33, 0x41, 0xd5, 0xf0, 0x5f, 0x04, 0x38, 0x03, 0x52, 0x81, 0xfd, 0xa0,
	0x41, 0xb8, 0x30, 0x72, 0x94, 0x25, 0xaf, 0x5b, 0x3c, 0x3b, 0x51, 0xd3, 0x7c, 0x5d, 0xe1, 0xf9,
	0x88, 0x42, 0xe6, 0xb1, 0x8a, 0xf4, 0x67, 0x07, 0x6a, 0x82, 0xaf, 0x21, 0xbc, 0x18, 0x71, 0xca,
	0xa4, 0x57, 0x3b, 0x7e, 0x93, 0x60, 0x6d, 0xe8, 0x2d, 0x8e, 0x9d, 0xa4, 0x69, 0xbb, 0x6e, 0xf3,
	0x6c, 0x45, 0xed, 0xf3, 0x0d, 0x85, 0xc5, 0xa3, 0x13, 0x39, 0xcd, 0xd2, 0xd5, 0x9d, 0x9f, 0x29,
	0xa8, 0x1e, 0xfe, 0x88, 0x40, 0x66, 0xb0, 0x2a, 0xf4, 0x1f, 0x07, 0x48, 0x02, 0xb6, 0x81, 0xb6,
	0xe0, 0x76, 0xc8, 0x26, 0xd6, 0x9a, 0xde, 0xeb, 0x18, 0x4f, 0x4a, 0xb4, 0x37, 0x37, 0x56, 0x96,
	0xbe, 0xee, 0xf0, 0x4c, 0x44, 0x35, 0xf3, 0x57, 0x05, 0xfe, 0x83, 0x00, 0x61, 0xc0, 0x28, 0x50,
	0x1e, 0xbc, 0x08, 0x71, 0xc6, 0xa4, 0x52, 0xfb, 0x7d, 0x83, 0x61, 0xa1, 0xe8, 0x78, 0x4e, 0xa2,
	0xb4, 0x79, 0xb7, 0x62, 0xf6, 0xa9, 0x86, 0xfe, 0xe2, 0xc0, 0x49, 0x90, 0x36, 0xec, 0x16, 0xcd,
	0xce, 0xd5, 0x94, 0x5f, 0x2f, 0x78, 0x1c, 0x22, 0x89, 0xd9, 0xa6, 0xda, 0xfa, 0xdb, 0x03, 0x1b,
	0x41, 0xcb, 0x70, 0x57, 0x64, 0x3e, 0xab, 0x50, 0x7f, 0x7c, 0x20, 0x21, 0xd8, 0x18, 0x5a, 0x8a,
	0xbb, 0x27, 0x33, 0x5a, 0x95, 0xfb, 0x2f, 0x03, 0x5c, 0x01, 0xf9, 0xc0, 0x42, 0xd0, 0x31, 0x9c,
	0x14, 0x69, 0xcf, 0x6e, 0xd4, 0x2c, 0x5f, 0x5d, 0xf8, 0x39, 0x82, 0x92, 0xe1, 0xad, 0x88, 0x7d,
	0xa6, 0xa1, 0xba, 0xf8, 0x73, 0x02, 0xa5, 0xc1, 0xbb, 0x10, 0x73, 0x4c, 0x25, 0xf5, 0xdb, 0x07,
	0x1b, 0x42, 0x8b, 0x71, 0xa7, 0x64, 0x7a, 0xab, 0x63, 0x3f, 0x69, 0xd0, 0x2e, 0xdc, 0x1c, 0x59,
	0xc9, 0xfa, 0xd6, 0xc3, 0x1e, 0xd1, 0xc8, 0x5c, 0x56, 0xb9, 0xfe, 0xf2, 0xc0, 0x45, 0x90, 0x33,
	0x2c, 0x15, 0xdd, 0xcf, 0x19, 0x94, 0x0a, 0xef, 0x47, 0x0c, 0x32, 0x85, 0xd5, 0xa3, 0x1f, 0x39,
	0xc8, 0x12, 0xd6, 0x8d, 0x9e, 0xe5, 0xa8, 0x4b, 0x3e, 0xb7, 0x50, 0x76, 0xbc, 0x26, 0xf1, 0xda,
	0xc4, 0x5b, 0x13, 0x7b, 0x4d, 0xe3, 0x75, 0x89, 0xe7, 0x26, 0xca, 0x9a, 0xd7, 0x2b, 0x1e, 0x9f,
	0x48, 0x68, 0x36, 0xae, 0x96, 0xfc, 0x6e, 0xc1, 0xec, 0x50, 0x4d, 0xfc, 0x35, 0x81, 0xd7, 0x20,
	0x5e, 0x98, 0x38, 0x6a, 0x92, 0xaf, 0x2d, 0xbc, 0x1d, 0xb1, 0xc9, 0xb4, 0x56, 0xf7, 0x7e, 0xc6,
	0xa0, 0x52, 0xf8, 0x3d, 0x82, 0x91, 0xa1, 0xac, 0x78, 0x7d, 0xe2, 0xa1, 0x89, 0xb8, 0x66, 0xf2,
	0xaa, 0xc5, 0xbf, 0x13, 0x30, 0x0d, 0xd4, 0x05, 0x9f, 0x43, 0x28, 0x31, 0xde, 0x94, 0x58, 0x6f,
	0x7a, 0xac, 0x23, 0x3d, 0xd9, 0xd1, 0x9a, 0xdc, 0x6b, 0x19, 0xef, 0x4a, 0xcc, 0x37, 0x15, 0xd6,
	0x8f, 0x1e, 0xe4, 0x08, 0x4b, 0x46, 0xb7, 0x72, 0xf6, 0xa5, 0x86, 0xfb, 0x22, 0xc3, 0x59, 0x91,
	0xfa, 0xec, 0x43, 0x0d, 0xf1, 0xc5, 0x84, 0x53, 0x23, 0x7d, 0xd9, 0xe1, 0x9a, 0xc8, 0x6b, 0x16,
	0xaf, 0x4e, 0xfc, 0x34, 0x41, 0xd7, 0x70, 0x5e, 0xa4, 0x38, 0x7b, 0x52, 0xa3, 0x7d, 0xb9, 0xe1,
	0xb2, 0xc8, 0x75, 0x96, 0xa7, 0x2e, 0xfa, 0x9c, 0x43, 0x29, 0xf1, 0xde, 0xc4, 0x58, 0x53, 0x7a,
	0xbd, 0xe3, 0x31, 0x89, 0xd4, 0x66, 0xdf, 0x6a, 0xd8, 0x2f, 0x1a, 0x9c, 0x0b, 0x29, 0xc7, 0x5e,
	0xd2, 0xb8, 0x5d, 0xb2, 0xb9, 0xb5, 0xb2, 0xf7, 0x35, 0x86, 0x97, 0x22, 0xee, 0x99, 0x8c, 0x6a,
	0xe5, 0xef, 0x0b, 0x0c, 0x07, 0x45, 0xc2, 0xb3, 0x11, 0xb5, 0xcc, 0x77, 0x15, 0xe6, 0x8f, 0x0a,
	0xe4, 0x07, 0x0b, 0x42, 0x87, 0x71, 0xa2, 0xa4, 0x79, 0xbb, 0x62, 0xf3, 0x69, 0x85, 0xee, 0xe3,
	0x0c, 0x49, 0xc5, 0xf6, 0xd3, 0x06, 0xdd, 0xc2, 0xd9, 0x91, 0x9a, 0xec, 0x6b, 0x0d, 0xef, 0x45,
	0x8c, 0x33, 0x25, 0xd5, 0xdb, 0x1f, 0x1b, 0x48, 0x0b, 0x76, 0x87, 0x66, 0xe2, 0xaa, 0xc9, 0xbf,
	0x16, 0xf0, 0x0e, 0xc4, 0x04, 0x53, 0x43, 0x7d, 0xf1, 0xe1, 0x84, 0x48, 0x63, 0x76, 0xa9, 0xe6,
	0xfe, 0xca, 0xc0, 0x57, 0x10, 0x3e, 0x8c, 0x10, 0x65, 0xcc, 0x2b, 0x15, 0xdf, 0x4f, 0x18, 0x34,
	0x0a, 0x97, 0x47, 0x2e, 0xb2, 0x9c, 0x75, 0xa9, 0xe7, 0x3e, 0xca, 0x90, 0x57, 0x2c, 0x3e, 0x9d,
	0xd0, 0x69, 0x9c, 0x2e, 0xe9, 0xdc, 0x4e, 0xd9, 0xf4, 0x5a, 0xc7, 0x7b, 0x12, 0xa3, 0x4d, 0xb9,
	0xf5, 0xb2, 0xc7, 0x35, 0x92, 0x97, 0x2d, 0xae, 0x9d, 0xbc, 0x69, 0xb1, 0xee, 0xf4, 0x4c, 0x47,
	0x75, 0xf2, 0xa7, 0x05, 0xba, 0x83, 0x33, 0x21, 0xd5, 0xd8, 0x5f, 0x1a, 0xb8, 0x0b, 0x32, 0x87,
	0x55, 0xa2, 0xbf, 0x39, 0xb0, 0x12, 0xf4, 0x0d, 0x87, 0x45, 0xa2, 0xb3, 0x39, 0xb5, 0xd2, 0xf7,
	0x1d, 0x86, 0x89, 0xa2, 0xe6, 0xf9, 0x8a, 0xc2, 0xe7, 0x11, 0x8a, 0x8c, 0x67, 0x25, 0xea, 0x9b,
	0x0f, 0x2b, 0x44, 0x1f, 0x73, 0x48, 0x25, 0xf6, 0x9b, 0x06, 0xeb, 0x42, 0xcf, 0x71, 0x94, 0x24,
	0x6f, 0x5b, 0x6c, 0x3b, 0x6d, 0xd3, 0x6d, 0x9d, 0xed, 0xa9, 0x8d, 0xbe, 0xe5, 0xb0, 0x4b, 0x34,
	0x37, 0x57, 0x56, 0xbe, 0xbe, 0xf0, 0x70, 0x44, 0x24, 0x33, 0x5b, 0x55, 0xfb, 0x7f, 0x03, 0x60,
	0x01, 0xe8, 0x00, 0x4e, 0x80, 0x34, 0x60, 0x17, 0x68, 0x0e, 0xae, 0x84, 0x7c, 0x63, 0x61, 0xe9,
	0xe8, 0x4e, 0xce, 0xb4, 0x54, 0x77, 0x7f, 0x66, 0xa0, 0x2a, 0xf8, 0x1f, 0x02, 0x88, 0x01, 0xa6,
	0x80, 0x7a, 0xe0, 0x23, 0x08, 0x19, 0xc6, 0x8a, 0xd2, 0xe7, 0x1d, 0x8a, 0x89, 0xa7, 0x26, 0xfa,
	0x9a, 0xc3, 0x2b, 0x11, 0xdf, 0x4c, 0x58, 0x35, 0xfa, 0x97, 0x03, 0x2e, 0x81, 0xdc, 0x60, 0x59,
	0xe8, 0x3a, 0xce, 0x93, 0x14, 0x6d, 0xcf, 0x6d, 0x94, 0x2d, 0xaf, 0x5d, 0xbc, 0x39, 0xb1, 0xd2,
	0xf4, 0x5d, 0x87, 0x79, 0xa2, 0xa2, 0xf9, 0xb9, 0x82, 0xf2, 0xe1, 0x85, 0x88, 0x63, 0x26, 0xa9,
	0xda, 0xfe, 0xdb, 0x00, 0x5b, 0x40, 0x3b, 0x70, 0x13, 0x64, 0x0d, 0xeb, 0x45, 0x8f, 0x73, 0x24,
	0x25, 0xdb, 0x5b, 0x1b, 0x7b, 0x4b, 0x63, 0x77, 0x69, 0xe6, 0xae, 0xca, 0xfc, 0x57, 0x01, 0xfe,
	0x80, 0x40, 0x60, 0x30, 0x28, 0x14, 0x1e, 0x8f, 0x48, 0x64, 0x36, 0xab, 0x56, 0xff, 0x7e, 0xc0,
	0x20, 0x50, 0x18, 0x3c, 0x0a, 0x91, 0xc7, 0x2c, 0x52, 0x9d, 0xfd, 0xa9, 0x81, 0xbe, 0xe0, 0x70,
	0x48, 0x24, 0x36, 0x9b, 0x56, 0xeb, 0x7e, 0xcf, 0x60, 0x54, 0x28, 0x3f, 0x5e, 0x90, 0x38, 0x6c,
	0x12, 0xad, 0xcd, 0xbd, 0x95, 0xb1, 0xaf, 0x34, 0x7c, 0x17, 0x61, 0xce, 0xa8, 0x54, 0x7e, 0xbf,
	0x60, 0x70, 0x28, 0x24, 0x1e, 0x9b, 0x48, 0x6b, 0x76, 0xaf, 0x66, 0xfc, 0x2a, 0xc1, 0xdf, 0x10,
	0x58, 0x0c, 0x3a, 0x85, 0xd3, 0x23, 0x1d, 0xd9, 0xc9, 0x9a, 0xd6, 0xeb, 0x1e, 0xcf, 0x48, 0x54,
	0x36, 0xbf, 0x56, 0xf0, 0x3e, 0xc4, 0x10, 0x53, 0x4c, 0x3d, 0xf5, 0xd1, 0x87, 0x1c, 0x62, 0x89,
	0xe9, 0xa6, 0xce, 0xfa, 0xd4, 0x43, 0x1f, 0x71, 0xc8, 0x24, 0x56, 0x9b, 0x7e, 0xeb, 0x60, 0x4f,
	0x68, 0x34, 0x2e, 0x97, 0x5c, 0x6e, 0xb9, 0xec, 0x72, 0xcd, 0xe5, 0x95, 0x8b, 0x2f, 0x27, 0x5c,
	0x1a, 0xb9, 0xcb, 0x32, 0xd7, 0x55, 0x9e, 0xbf, 0x28, 0x70, 0x1e, 0xa4, 0x08, 0x7b, 0x46, 0xa3,
	0x72, 0xf9, 0xe5, 0x82, 0xcb, 0x21, 0x97, 0x58, 0x6e, 0xba, 0xac, 0x73, 0x3d, 0xe5, 0xd1, 0x8b,
	0x1c, 0x67, 0x49, 0xea, 0xb6, 0xcf, 0x36, 0xd4, 0x16, 0xdf, 0x4e, 0xd8, 0x34, 0x5a, 0x97, 0x7b,
	0x2e, 0xa3, 0x5c, 0x79, 0xf9, 0xe2, 0xc2, 0xc9, 0x91, 0x96, 0xec, 0x6e, 0xcd, 0xec, 0x55, 0x8d,
	0xff, 0x25, 0x80, 0x1b, 0x20, 0x0b, 0x58, 0x07, 0x7a, 0x82, 0xa3, 0x21, 0xb9, 0xd8, 0x72, 0xda,
	0xa5, 0x9b, 0x3b, 0x2b, 0x53, 0x5f, 0x7d, 0xf8, 0x21, 0x82, 0x98, 0x61, 0xaa, 0xa8, 0x7f, 0x3e,
	0xa0, 0x10, 0x78, 0x0c, 0x22, 0x85, 0xd9, 0xa3, 0x1a, 0xf9, 0xcb, 0x02, 0xd7, 0x41, 0x9e, 0xb0,
	0x68, 0x74, 0x2e, 0xa7, 0x5c, 0x7a, 0xb9, 0xe3, 0x32, 0xc9, 0xd5, 0x96, 0xdf, 0x2e, 0xd8, 0x1c,
	0x5a, 0x89, 0xfb, 0x26, 0xc3, 0x5a, 0xd1, 0xfb, 0x1c, 0x43, 0x49, 0xf1, 0xf6, 0xc4, 0x46, 0xd3,
	0x72, 0xdd, 0xe5, 0x99
};


//**************************************************************************
//  MEMBER FUNCTIONS
//**************************************************************************

void cdicdic_device::decode_xa_mono(int16_t *cdic_xa_last, const uint8_t *xa, int16_t *dp)
{
	int16_t l0 = cdic_xa_last[0];
	int16_t l1 = cdic_xa_last[1];

	for (int32_t b = 0; b < 18; b++)
	{
		for (int32_t s = 0; s < 4; s++)
		{
			uint8_t flags = xa[4 + (s << 1)];
			uint8_t shift = flags & 0xf;
			uint8_t filter = (flags >> 4) & 3;
			int16_t f0 = s_xa_filter_coef[filter][0];
			int16_t f1 = s_xa_filter_coef[filter][1];

			for (int32_t i = 0; i < 28; i++)
			{
				int16_t d = (xa[16 + (i << 2) + s] & 0xf) << 12;
				d = (d >> shift) + (((l0 * f0) + (l1 * f1) + 128) >> 8);
				*dp = d;
				dp++;
				l1 = l0;
				l0 = d;
			}

			flags = xa[5 + (s << 1)];
			shift = flags & 0xf;
			filter = flags >> 4;
			f0 = s_xa_filter_coef[filter][0];
			f1 = s_xa_filter_coef[filter][1];

			for (int32_t i = 0; i < 28; i++)
			{
				int16_t d = (xa[16 + (i << 2) + s] >> 4) << 12;
				d = (d >> shift) + (((l0 * f0) + (l1 * f1) + 128) >> 8);
				*dp = d;
				dp++;
				l1 = l0;
				l0 = d;
			}
		}

		xa += 128;
	}

	cdic_xa_last[0] = l0;
	cdic_xa_last[1] = l1;
}

void cdicdic_device::decode_xa_mono8(int16_t *cdic_xa_last, const unsigned char *xa, signed short *dp)
{
	int16_t l0 = cdic_xa_last[0];
	int16_t l1 = cdic_xa_last[1];

	for (int32_t b = 0; b < 18; b++)
	{
		for (int32_t s = 0; s < 4; s++)
		{
			uint8_t flags = xa[4 + s];
			uint8_t shift = flags & 0xf;
			uint8_t filter = (flags >> 4) & 3;
			int16_t f0 = s_xa_filter_coef[filter][0];
			int16_t f1 = s_xa_filter_coef[filter][1];

			for (int32_t i = 0; i < 28; i++)
			{
				int16_t d = (xa[16 + (i << 2) + s] << 8);
				d = (d >> shift) + (((l0 * f0) + (l1 * f1) + 128) >> 8);
				*dp = d;
				dp++;
				l1 = l0;
				l0 = d;
			}
		}

		xa += 128;
	}

	cdic_xa_last[0] = l0;
	cdic_xa_last[1] = l1;
}

void cdicdic_device::decode_xa_stereo(int16_t *cdic_xa_last, const uint8_t *xa, int16_t *dp)
{
	int16_t l0 = cdic_xa_last[0];
	int16_t l1 = cdic_xa_last[1];
	int16_t l2 = cdic_xa_last[2];
	int16_t l3 = cdic_xa_last[3];

	for (int32_t b = 0; b < 18; b++)
	{
		for (int32_t s = 0; s < 4; s++)
		{
			uint8_t flags0 = xa[4 + (s << 1)];
			uint8_t shift0 = flags0 & 0xf;
			uint8_t filter0 = (flags0 >> 4) & 3;
			uint8_t flags1 = xa[5 + (s << 1)];
			uint8_t shift1 = flags1 & 0xf;
			uint8_t filter1 = (flags1 >> 4) & 3;

			int16_t f0 = s_xa_filter_coef[filter0][0];
			int16_t f1 = s_xa_filter_coef[filter0][1];
			int16_t f2 = s_xa_filter_coef[filter1][0];
			int16_t f3 = s_xa_filter_coef[filter1][1];

			for (int32_t i = 0; i < 28; i++)
			{
				int16_t d = xa[16 + (i << 2) + s];
				int16_t d0 = (d & 0xf) << 12;
				int16_t d1 = (d >> 4) << 12;
				d0 = (d0 >> shift0) + (((l0 * f0) + (l1 * f1) + 128) >> 8);
				*dp = d0;
				dp++;
				l1 = l0;
				l0 = d0;

				d1 = (d1 >> shift1) + (((l2 * f2) + (l3 * f3) + 128) >> 8);
				*dp = d1;
				dp++;
				l3 = l2;
				l2 = d1;
			}
		}

		xa += 128;
	}

	cdic_xa_last[0] = l0;
	cdic_xa_last[1] = l1;
	cdic_xa_last[2] = l2;
	cdic_xa_last[3] = l3;
}

void cdicdic_device::decode_xa_stereo8(int16_t *cdic_xa_last, const uint8_t *xa, int16_t *dp)
{
	int16_t l0 = cdic_xa_last[0];
	int16_t l1 = cdic_xa_last[1];
	int16_t l2 = cdic_xa_last[2];
	int16_t l3 = cdic_xa_last[3];

	for (int32_t b = 0; b < 18; b++)
	{
		for (int32_t s = 0; s < 4; s += 2)
		{
			uint8_t flags0 = xa[4 + s];
			uint8_t shift0 = flags0 & 0xf;
			uint8_t filter0 = (flags0 >> 4) & 3;
			uint8_t flags1 = xa[5 + s];
			uint8_t shift1 = flags1 & 0xf;
			uint8_t filter1 = (flags1 >> 4) & 3;
			int16_t f0 = s_xa_filter_coef[filter0][0];
			int16_t f1 = s_xa_filter_coef[filter0][1];
			int16_t f2 = s_xa_filter_coef[filter1][0];
			int16_t f3 = s_xa_filter_coef[filter1][1];

			for (int32_t i = 0; i < 28; i++)
			{
				int16_t d0 = (xa[16 + (i << 2) + s + 0] << 8);
				int16_t d1 = (xa[16 + (i << 2) + s + 1] << 8);

				d0 = (d0 >> shift0) + (((l0 * f0) + (l1 * f1) + 128) >> 8);
				*dp = d0;
				dp++;
				l1 = l0;
				l0 = d0;

				d1 = (d1 >> shift1) + (((l2 * f2) + (l3 * f3) + 128) >> 8);
				*dp = d1;
				dp++;
				l3 = l2;
				l2 = d1;
			}
		}

		xa += 128;
	}

	cdic_xa_last[0] = l0;
	cdic_xa_last[1] = l1;
	cdic_xa_last[2] = l2;
	cdic_xa_last[3] = l3;
}

void cdicdic_device::decode_8bit_xa_unit(int channel, uint8_t param, const uint8_t *data, int16_t *out_buffer)
{
	int gain_shift = 8 - (param & 0xf);

	const int16_t *filter = s_xa_filter_coef[(param >> 4) & 3];
	int16_t *old_samples = &m_xa_last[channel << 1];

	for (int i = 0; i < 28; i++)
	{
		int32_t sample = *data;
		if (sample >= 128)
			sample -= 256;
		data += 4;

		sample <<= gain_shift;

		sample += (filter[0] * old_samples[0] + filter[1] * old_samples[1] + 128) / 256;

		int16_t sample16 = (int16_t)sample;
		if (sample < -32768)
			sample16 = -32768;
		else if (sample > 32767)
			sample16 = 32767;

		old_samples[1] = old_samples[0];
		old_samples[0] = sample16;

		out_buffer[i] = sample16;
	}
}

void cdicdic_device::decode_4bit_xa_unit(int channel, uint8_t param, const uint8_t *data, uint8_t shift, int16_t *out_buffer)
{
	int gain_shift = 12 - (param & 0xf);

	const int16_t *filter = s_xa_filter_coef[(param >> 4) & 3];
	int16_t *old_samples = &m_xa_last[channel << 1];

	for (int i = 0; i < 28; i++)
	{
		int32_t sample = (*data >> shift) & 0xf;
		if (BIT(sample, 3))
			sample -= 16;
		data += 4;

		sample <<= gain_shift;

		sample += (filter[0] * old_samples[0] + filter[1] * old_samples[1] + 128) / 256;

		int16_t sample16 = (int16_t)(uint16_t)(sample & 0xffff);
		if (sample < -32768)
			sample16 = -32768;
		else if (sample > 32767)
			sample16 = 32767;

		old_samples[1] = old_samples[0];
		old_samples[0] = sample16;

		out_buffer[i] = sample16;
	}
}

void cdicdic_device::play_raw_group(const uint8_t *data)
{
	int16_t samples[28];
	for (int i = 0; i < 28; i++)
	{
		samples[i] = (int16_t)((data[1] << 8) | data[0]);
		data += 4;
	}

	m_dmadac[0]->transfer(0, 1, 1, 28, samples);
	m_dmadac[1]->transfer(0, 1, 1, 28, samples);
}

void cdicdic_device::play_xa_group(const uint8_t coding, const uint8_t *data)
{
	static const uint16_t s_4bit_header_offsets[8] = { 0, 1, 2, 3, 8, 9, 10, 11 };
	static const uint16_t s_8bit_header_offsets[4] = { 0, 1, 2, 3 };
	static const uint16_t s_4bit_data_offsets[8] = { 16, 16, 17, 17, 18, 18, 19, 19 };
	static const uint16_t s_8bit_data_offsets[4] = { 16, 17, 18, 19 };

	int16_t samples[28];

	switch (coding & (CODING_BPS_MASK | CODING_CHAN_MASK))
	{
		case CODING_4BPS | CODING_MONO:
			for (uint8_t i = 0; i < 8; i++)
			{
				decode_4bit_xa_unit(0, data[s_4bit_header_offsets[i]], data + s_4bit_data_offsets[i], (i & 1) ? 4 : 0, samples);
				m_dmadac[0]->transfer(0, 1, 1, 28, samples);
				m_dmadac[1]->transfer(0, 1, 1, 28, samples);
			}
			return;

		case CODING_4BPS | CODING_STEREO:
			for (uint8_t i = 0; i < 8; i++)
			{
				decode_4bit_xa_unit(i & 1, data[s_4bit_header_offsets[i]], data + s_4bit_data_offsets[i], (i & 1) ? 4 : 0, samples);
				m_dmadac[i & 1]->transfer(0, 1, 1, 28, samples);
			}
			return;

		case CODING_8BPS | CODING_MONO:
			for (uint8_t i = 0; i < 4; i++)
			{
				decode_8bit_xa_unit(0, data[s_8bit_header_offsets[i]], data + s_8bit_data_offsets[i], samples);
				m_dmadac[0]->transfer(0, 1, 1, 28, samples);
				m_dmadac[1]->transfer(0, 1, 1, 28, samples);
			}
			return;

		case CODING_8BPS | CODING_STEREO:
			for (uint8_t i = 0; i < 4; i++)
			{
				decode_8bit_xa_unit(i & 1, data[s_8bit_header_offsets[i]], data + s_8bit_data_offsets[i], samples);
				m_dmadac[i & 1]->transfer(0, 1, 1, 28, samples);
			}
			return;
	}
}

void cdicdic_device::play_cdda_sector(const uint8_t *data)
{
	m_dmadac[0]->set_frequency(44100);
	m_dmadac[1]->set_frequency(44100);
	m_dmadac[0]->set_volume(0x100);
	m_dmadac[1]->set_volume(0x100);

	int16_t samples[2][2352/4];
	for (uint16_t i = 0; i < 2352/4; i++)
	{
		samples[0][i] = (int16_t)((data[(i * 4) + 1] << 8) | data[(i * 4) + 0]);
		samples[1][i] = (int16_t)((data[(i * 4) + 3] << 8) | data[(i * 4) + 2]);
	}

	m_dmadac[0]->transfer(0, 1, 1, SECTOR_SIZE/4, samples[0]);
	m_dmadac[1]->transfer(0, 1, 1, SECTOR_SIZE/4, samples[1]);
}

void cdicdic_device::play_audio_sector(const uint8_t coding, const uint8_t *data)
{
	if ((coding & CODING_CHAN_MASK) > CODING_STEREO || (coding & CODING_BPS_MASK) == CODING_BPS_MPEG || (coding & CODING_RATE_MASK) == CODING_RATE_RESV)
	{
		LOGMASKED(LOG_SECTORS, "Invalid coding (%02x), ignoring\n", coding);
		return;
	}

	int channels = 2;
	//offs_t buffer_length = 1;
	if (!(coding & CODING_STEREO))
	{
		channels = 1;
		//buffer_length *= 2;
	}

	int bits = 4;
	switch (coding & CODING_BPS_MASK)
	{
	case CODING_8BPS:
		bits = 8;
		break;
	case CODING_16BPS:
		bits = 16;
		fatalerror("play_audio_sector: unhandled 16-bit coding mode\n");
		break;
	default:
		bits = 4;
		//buffer_length *= 2;
		break;
	}

	int32_t sample_frequency = 0;
	switch (coding & CODING_RATE_MASK)
	{
	case CODING_37KHZ:
		sample_frequency = clock2() / 512.0f;
		break;
	case CODING_18KHZ:
		sample_frequency = clock2() / 1024.0f;
		break;
	case CODING_44KHZ:
		fatalerror("play_audio_sector: unhandled 44KHz coding mode\n");
		break;
	default:
		// Can't happen due to above early-out
		break;
	}

	LOGMASKED(LOG_SECTORS, "Coding %02x, %d channels, %d bits, %08x frequency\n", coding, channels, bits, sample_frequency);

	m_dmadac[0]->set_frequency(sample_frequency);
	m_dmadac[1]->set_frequency(sample_frequency);
	m_dmadac[0]->set_volume(0x100);
	m_dmadac[1]->set_volume(0x100);

	if (bits == 16 && channels == 2)
	{
		for (uint16_t i = 0; i < SECTOR_AUDIO_SIZE; i += 112, data += 112)
		{
			play_raw_group(data);
		}
	}
	else
	{
		for (uint16_t i = 0; i < SECTOR_AUDIO_SIZE; i += 128, data += 128)
		{
			play_xa_group(coding, data);
		}
	}
}

TIMER_CALLBACK_MEMBER( cdicdic_device::audio_tick )
{
	if (m_audio_sector_counter > 0)
	{
		m_audio_sector_counter--;
		if (m_audio_sector_counter > 0)
		{
			LOGMASKED(LOG_SAMPLES, "Audio sector counter %d, deducting and skipping\n", m_audio_sector_counter);
			return;
		}
		LOGMASKED(LOG_SAMPLES, "Audio sector counter now 0, deducting and playing\n");
	}

	if (m_decoding_audio_map)
	{
		process_audio_map();
	}
}

void cdicdic_device::process_audio_map()
{
	if (m_decode_addr == 0xffff)
	{
		m_audio_sector_counter = 0;
		m_audio_format_sectors = 0;
		m_decoding_audio_map = false;
		return;
	}

	LOGMASKED(LOG_SAMPLES, "Procesing audio map from %04x\n", m_decode_addr);

	uint8_t *ram = &m_ram[m_decode_addr & 0x3ffe];
	m_decode_addr ^= 0x1a00;

	const bool was_decoding = (m_audio_format_sectors != 0);

	const uint8_t coding = ram[(SECTOR_CODING2 - SECTOR_HEADER) ^ 1];
	LOGMASKED(LOG_SAMPLES, "Coding is %02x\n", coding);
	if (coding != 0xff)
	{
		m_decoding_audio_map = true;
		m_audio_format_sectors = get_sector_count_for_coding(coding);
		m_audio_sector_counter = m_audio_format_sectors;

		ram += SECTOR_DATA - SECTOR_HEADER;
		uint8_t swapped_data[(SECTOR_SIZE - (SECTOR_DATA - SECTOR_HEADER))];
		for (uint16_t i = 0; i < (SECTOR_SIZE - (SECTOR_DATA - SECTOR_HEADER)); i++)
		{
			swapped_data[i ^ 1] = ram[i];
		}
		play_audio_sector(coding, swapped_data);
	}
	else
	{
		m_decode_addr = 0xffff;
		m_audio_sector_counter = 0;
	}

	if (was_decoding)
	{
		m_audio_buffer |= 0x8000;
		update_interrupt_state();
	}
}

void cdicdic_device::update_interrupt_state()
{
	const bool interrupt_active = (bool)BIT(m_x_buffer | m_audio_buffer, 15);
	if (!interrupt_active)
		LOGMASKED(LOG_SECTORS, "%s: Clearing CDIC interrupt line\n", machine().describe_context());
	m_intreq_callback(interrupt_active ? ASSERT_LINE : CLEAR_LINE);
}

void cdicdic_device::descramble_sector(uint8_t *buffer)
{
	for (uint32_t i = 12; i < SECTOR_SIZE; i++)
	{
		buffer[i] ^= s_sector_scramble[i];
	}
}

bool cdicdic_device::is_valid_sector(const uint8_t *buffer)
{
	const uint32_t real_lba = m_curr_lba + 150;
	const uint8_t mins = real_lba / (60 * 75);
	const uint8_t secs = (real_lba / 75) % 60;
	const uint8_t frac = real_lba % 75;
	const uint8_t mins_bcd = ((mins / 10) << 4) | (mins % 10);
	const uint8_t secs_bcd = ((secs / 10) << 4) | (secs % 10);
	const uint8_t frac_bcd = ((frac / 10) << 4) | (frac % 10);

	// Verify MSF
	if (mins_bcd != buffer[SECTOR_MINUTES] || secs_bcd != buffer[SECTOR_SECONDS] || frac_bcd != buffer[SECTOR_FRACS])
	{
		LOGMASKED(LOG_SECTORS, "Not valid sector, MSF (%02x:%02x:%02x vs. %02x:%02x:%02x\n", mins_bcd, secs_bcd, frac_bcd, buffer[SECTOR_MINUTES], buffer[SECTOR_SECONDS], buffer[SECTOR_FRACS]);
		return false;
	}

	// Verify mode
	if (buffer[SECTOR_MODE] != 1 && buffer[SECTOR_MODE] != 2)
	{
		LOGMASKED(LOG_SECTORS, "Not valid sector, mode %02x\n", buffer[SECTOR_MODE]);
		return false;
	}

	// Verify duplicate info
	if (buffer[SECTOR_FILE1] != buffer[SECTOR_FILE2])
	{
		LOGMASKED(LOG_SECTORS, "Not valid sector, file %02x vs. %02x\n", buffer[SECTOR_FILE1], buffer[SECTOR_FILE2]);
		return false;
	}
	if (buffer[SECTOR_CHAN1] != buffer[SECTOR_CHAN2])
	{
		LOGMASKED(LOG_SECTORS, "Not valid sector, channel %02x vs. %02x\n", buffer[SECTOR_CHAN1], buffer[SECTOR_CHAN2]);
		return false;
	}
	if (buffer[SECTOR_SUBMODE1] != buffer[SECTOR_SUBMODE2])
	{
		LOGMASKED(LOG_SECTORS, "Not valid sector, channel %02x vs. %02x\n", buffer[SECTOR_SUBMODE1], buffer[SECTOR_SUBMODE2]);
		return false;
	}
	if (buffer[SECTOR_CODING1] != buffer[SECTOR_CODING2])
	{
		LOGMASKED(LOG_SECTORS, "Not valid sector, coding %02x vs. %02x\n", buffer[SECTOR_CODING1], buffer[SECTOR_CODING2]);
		return false;
	}

	return true;
}

bool cdicdic_device::is_mode2_sector_selected(const uint8_t *buffer)
{
	if ((buffer[SECTOR_FILE2] << 8) != m_file)
	{
		LOGMASKED(LOG_SECTORS, "Mode 2 sector is not selected, current file: %04x, disc file: %04x\n", m_file, buffer[SECTOR_FILE2]);
		return false;
	}

	if (buffer[SECTOR_SUBMODE2] & SUBMODE_EOF)
	{
		LOGMASKED(LOG_SECTORS, "Mode 2 sector is EOF, queueing end of read\n");
		m_disc_command = 0;
	}

	// End-of-File, End-of-Record, or Trigger sectors skip selection beyond initial file selection.
	if (buffer[SECTOR_SUBMODE2] & (SUBMODE_EOF | SUBMODE_TRIG | SUBMODE_EOR))
	{
		LOGMASKED(LOG_SECTORS, "Mode 2 sector is selected due to EOF, TRIG, or EOR (%02x)\n", buffer[SECTOR_SUBMODE2]);
		return true;
	}

	// Sectors with no applicable data are skipped.
	if (!(buffer[SECTOR_SUBMODE2] & (SUBMODE_DATA | SUBMODE_AUDIO | SUBMODE_VIDEO)))
	{
		LOGMASKED(LOG_SECTORS, "Mode 2 sector is not selected due to being a message sector (%02x)\n", buffer[SECTOR_SUBMODE2]);
		return false;
	}

	// Select based on the specified channel mask.
	const bool channel_selected = (bool)BIT(m_channel, buffer[SECTOR_CHAN2]);

	LOGMASKED(LOG_SECTORS, "Mode 2 sector is %sselected due to channel (register %04x, buffer channel %04x)\n", channel_selected ? "" : "not ", m_channel, buffer[SECTOR_CHAN2]);

	return channel_selected;
}

bool cdicdic_device::is_mode2_audio_selected(const uint8_t *buffer)
{
	// Non-Mode-2, Non-Audio sectors are never selected for audio playback.
	if (!(buffer[SECTOR_SUBMODE2] & SUBMODE_FORM) || !(buffer[SECTOR_SUBMODE2] & SUBMODE_AUDIO))
	{
		LOGMASKED(LOG_SECTORS, "Audio is not selected; submode %02x\n", buffer[SECTOR_SUBMODE2]);
		return false;
	}

	// Select based on the specified audio channel mask.
	const bool channel_selected = (bool)BIT(m_audio_channel, buffer[SECTOR_CHAN2]);

	LOGMASKED(LOG_SECTORS, "Mode 2 audio is %sselected due to channel (register %04x, buffer channel %04x)\n", channel_selected ? "" : "not ", m_audio_channel, buffer[SECTOR_CHAN2]);

	return channel_selected;
}

TIMER_CALLBACK_MEMBER( cdicdic_device::sector_tick )
{
	if (m_disc_command == 0)
	{
		return;
	}

	if (m_disc_spinup_counter != 0)
	{
		LOGMASKED(LOG_SECTORS, "Sector tick, waiting on spinup\n");
		m_disc_spinup_counter--;
		return;
	}

	LOGMASKED(LOG_SECTORS, "About to process a disc sector\n");

	process_disc_sector();

	if (m_disc_command == 0)
	{
		LOGMASKED(LOG_SECTORS, "Disc command has been reset after processing; stopping processing.\n");
		cancel_disc_read();
		return;
	}

	m_curr_lba++;
}

uint8_t cdicdic_device::get_sector_count_for_coding(uint8_t coding)
{
	uint8_t base_count = 2;

	switch (coding & CODING_BPS_MASK)
	{
	case CODING_4BPS:
		// Twice as many 4bpp audio frames fit as usual
		base_count *= 2;
		break;
	case CODING_8BPS:
	case CODING_16BPS:
		// No multiplier vs. base
		break;
	case CODING_BPS_MPEG:
		// Unsupported; clear to zero for now
		base_count = 0;
		break;
	}

	switch (coding & CODING_RATE_MASK)
	{
	case CODING_18KHZ:
		// Twice as many half-rate audio frames fit as usual
		base_count *= 2;
		break;
	case CODING_37KHZ:
	case CODING_44KHZ:
		// No multiplier vs. base
		break;
	case CODING_RATE_RESV:
		// Unsupported reserved mode; clear to zero for now
		base_count = 0;
		break;
	}

	switch (coding & CODING_CHAN_MASK)
	{
	case CODING_MONO:
		// Twice as many mono audio frames fit vs. stereo
		base_count *= 2;
		break;
	case CODING_STEREO:
		// No multiplier vs. base
		break;
	case CODING_CHAN_RESV:
	case CODING_CHAN_MPEG:
		// MPEG mode and reserved modes are unsupported; clear to zero for now
		base_count = 0;
		break;
	}

	return base_count;
}

void cdicdic_device::process_disc_sector()
{
	const uint32_t real_lba = m_curr_lba + 150;
	const uint8_t mins = real_lba / (60 * 75);
	const uint8_t secs = (real_lba / 75) % 60;
	const uint8_t frac = real_lba % 75;
	const uint8_t mins_bcd = ((mins / 10) << 4) | (mins % 10);
	const uint8_t secs_bcd = ((secs / 10) << 4) | (secs % 10);
	const uint8_t frac_bcd = ((frac / 10) << 4) | (frac % 10);

	LOGMASKED(LOG_SECTORS, "Disc sector, current LBA: %08x, MSF: %02x %02x %02x\n", real_lba, mins_bcd, secs_bcd, frac_bcd);

	uint8_t buffer[2560] = { 0 };
	m_cdrom->read_data(m_curr_lba, buffer, cdrom_file::CD_TRACK_RAW_DONTCARE);

	// Detect (badly) if we're dealing with a byteswapped loose-bin image
	if (buffer[0] == 0xff && buffer[1] == 0x00)
	{
		LOGMASKED(LOG_SECTORS, "Byteswapping\n");
		m_cd_byteswap = true;
	}

	if (m_cd_byteswap)
	{
		for (uint16_t i = 0; i < 2560; i += 2)
		{
			std::swap(buffer[i], buffer[i + 1]);
		}
	}

	if (!is_valid_sector(buffer))
	{
		uint8_t descramble_buffer[2560];
		memcpy(descramble_buffer, buffer, sizeof(descramble_buffer));
		LOGMASKED(LOG_SECTORS, "Sector seems to be encoded, attempting to apply descrambling\n");
		descramble_sector(descramble_buffer);

		if (!is_valid_sector(descramble_buffer))
		{
			LOGMASKED(LOG_SECTORS, "Sector remains invalid after descrambling, giving up and proceeding as normal\n");
		}
		else
		{
			memcpy(buffer, descramble_buffer, sizeof(descramble_buffer));
		}
	}

	LOGMASKED(LOG_SECTORS, "Sector header data: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
		buffer[ 0], buffer[ 1], buffer[ 2], buffer[ 3], buffer[ 4], buffer[ 5], buffer[ 6], buffer[ 7], buffer[ 8], buffer[ 9],
		buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15], buffer[16], buffer[17], buffer[18], buffer[19],
		buffer[20], buffer[21], buffer[22], buffer[23]);

	if (buffer[SECTOR_MODE] == 2 && m_disc_mode == DISC_MODE2)
	{
		// First, filter whether we want to process this sector at all.
		if (!is_mode2_sector_selected(buffer))
		{
			return;
		}

		// Next, determine if we want to process this sector as an audio sector.
		if (is_mode2_audio_selected(buffer))
		{
			LOGMASKED(LOG_SECTORS, "Audio is selected\n");
			m_audio_sector_counter = get_sector_count_for_coding(buffer[SECTOR_CODING2]);
			m_decoding_audio_map = false;

			play_audio_sector(buffer[SECTOR_CODING2], buffer + SECTOR_DATA);
		}
	}
	else if (m_disc_mode == DISC_CDDA)
	{
		m_audio_sector_counter = 2;
		m_decoding_audio_map = false;

		// Byteswap if not already detected as byteswapped
		if (!m_cd_byteswap)
		{
			uint8_t swapped_buffer[2560];
			for (uint16_t i = 0; i < 2560; i += 2)
			{
				swapped_buffer[i + 1] = buffer[i + 0];
				swapped_buffer[i + 0] = buffer[i + 1];
			}
			play_cdda_sector(swapped_buffer);
		}
		else
		{
			play_cdda_sector(buffer);
		}

		if (frac != 0)
		{
			return;
		}
	}

	// Calculate subcode data
	uint8_t subcode_buffer[96];
	memset(subcode_buffer, 0, sizeof(subcode_buffer));

	if (m_disc_mode == DISC_TOC)
	{
		uint8_t *toc_buffer = buffer;
		const cdrom_file::toc &toc = m_cdrom->get_toc();
		uint32_t entry_count = 0;

		// Determine total frame count for data, and total audio track count
		uint32_t frames = toc.tracks[0].pregap;
		int audio_tracks = 0;
		int other_tracks = 0;
		uint32_t audio_starts[cdrom_file::MAX_TRACKS];
		for (uint32_t i = 0; i < toc.numtrks; i++)
		{
			if (toc.tracks[i].trktype != cdrom_file::CD_TRACK_AUDIO)
			{
				frames += toc.tracks[i].frames + toc.tracks[i].extraframes;
			}
			else
			{
				audio_starts[audio_tracks++] = toc.tracks[i].logframeofs;
			}
		}

		// Determine last-frame MSF
		const uint8_t total_mins = frames / (60 * 75);
		const uint8_t total_secs = (frames / 75) % 60;
		const uint8_t total_frac = frames % 75;

		// Specify any audio tracks first
		for (int i = 0; i < audio_tracks; i++)
		{
			const uint8_t audio_mins = audio_starts[i] / (60 * 75);
			const uint8_t audio_secs = (audio_starts[i] / 75) % 60;
			const uint8_t audio_frac = audio_starts[i] % 75;
			const uint8_t audio_mins_bcd = ((audio_mins / 10) << 4) | (audio_mins % 10);
			const uint8_t audio_secs_bcd = ((audio_secs / 10) << 4) | (audio_secs % 10);
			const uint8_t audio_frac_bcd = ((audio_frac / 10) << 4) | (audio_frac % 10);

			const uint8_t track_bcd = (((i + 1) / 10) << 4) | ((i + 1) % 10);

			for (int j = 0; j < 3; j++)
			{
				*toc_buffer++ = 0x01;       // Track type (CD-DA)
				*toc_buffer++ = track_bcd;  // Track number
				*toc_buffer++ = audio_mins_bcd;
				*toc_buffer++ = audio_secs_bcd;
				*toc_buffer++ = audio_frac_bcd;
				entry_count++;
			}
		}

		// Packet A0 (lead-in)
		for (int i = 0; i < 3; i++)
		{
			*toc_buffer++ = (other_tracks > 0) ? 0x41 : 0x01;
			*toc_buffer++ = 0xa0;
			*toc_buffer++ = 0x01;
			*toc_buffer++ = (other_tracks > 0) ? 0x10 : 0x00;
			*toc_buffer++ = 0x00;
			entry_count++;
		}

		// Packet A1
		for (int i = 0; i < 3; i++)
		{
			*toc_buffer++ = (audio_tracks > 0) ? 0x01 : 0x41;
			*toc_buffer++ = 0xa1;
			if (audio_tracks > 0)
			{
				uint8_t last_audio_track = (uint8_t)(audio_tracks - 1);
				*toc_buffer++ = ((last_audio_track / 10) << 4) | (last_audio_track % 10);
			}
			else
			{
				*toc_buffer++ = 0x00;
			}
			*toc_buffer++ = 0x00;
			*toc_buffer++ = 0x00;
			entry_count++;
		}

		// Packet A2 (lead-out)
		for (int i = 0; i < 3; i++)
		{
			*toc_buffer++ = (audio_tracks > 0) ? 0x01 : 0x41;
			*toc_buffer++ = 0xa2;
			*toc_buffer++ = ((total_mins / 10) << 4) | (total_mins % 10);
			*toc_buffer++ = ((total_secs / 10) << 4) | (total_secs % 10);
			*toc_buffer++ = ((total_frac / 10) << 4) | (total_frac % 10);
			entry_count++;
		}

		uint8_t *toc_data = &buffer[(m_curr_lba % entry_count) * 5];

		subcode_buffer[SUBCODE_Q_CONTROL] = toc_data[0];
		subcode_buffer[SUBCODE_Q_TRACK] = 0x00;
		subcode_buffer[SUBCODE_Q_INDEX] = toc_data[1];
		subcode_buffer[SUBCODE_Q_MODE1_MINS] = 0xa0;
		subcode_buffer[SUBCODE_Q_MODE1_SECS] = secs_bcd;
		subcode_buffer[SUBCODE_Q_MODE1_FRAC] = frac_bcd;
		subcode_buffer[SUBCODE_Q_MODE1_ZERO] = 0x00;
		subcode_buffer[SUBCODE_Q_MODE1_AMINS] = toc_data[2];
		subcode_buffer[SUBCODE_Q_MODE1_ASECS] = toc_data[3];
		subcode_buffer[SUBCODE_Q_MODE1_AFRAC] = toc_data[4];
		subcode_buffer[SUBCODE_Q_CRC0] = 0xff;
		subcode_buffer[SUBCODE_Q_CRC1] = 0xff;
	}
	else
	{
		subcode_buffer[SUBCODE_Q_CONTROL] = (m_disc_mode == DISC_CDDA ? 0x01 : 0x41);
		subcode_buffer[SUBCODE_Q_TRACK] = 0x01;
		subcode_buffer[SUBCODE_Q_INDEX] = 0x01;
		subcode_buffer[SUBCODE_Q_MODE1_MINS] = mins_bcd;
		subcode_buffer[SUBCODE_Q_MODE1_SECS] = secs_bcd;
		subcode_buffer[SUBCODE_Q_MODE1_FRAC] = frac_bcd;
		subcode_buffer[SUBCODE_Q_MODE1_ZERO] = 0x00;
		subcode_buffer[SUBCODE_Q_MODE1_AMINS] = mins_bcd;
		subcode_buffer[SUBCODE_Q_MODE1_ASECS] = secs_bcd;
		subcode_buffer[SUBCODE_Q_MODE1_AFRAC] = frac_bcd;
		subcode_buffer[SUBCODE_Q_CRC0] = 0xff;
		subcode_buffer[SUBCODE_Q_CRC1] = 0xff;
	}

	uint16_t crc_accum = 0;
	for (int i = 0; i < 12; i++)
		crc_accum = CRC_CCITT_ROUND(crc_accum, subcode_buffer[SUBCODE_Q_CONTROL + i]);

	subcode_buffer[SUBCODE_Q_CRC0] = (uint8_t)(crc_accum >> 8);
	subcode_buffer[SUBCODE_Q_CRC1] = (uint8_t)crc_accum;

	process_sector_data(buffer, subcode_buffer);
}

void cdicdic_device::process_sector_data(const uint8_t *buffer, const uint8_t *subcode_buffer)
{
	m_data_buffer ^= 0x0001;
	m_data_buffer &= ~0x0004;

	uint16_t *dev_buffer = (uint16_t *)&m_ram[(m_data_buffer & 0x0005) * 0xa00];

	for (int i = SECTOR_HEADER; i < SECTOR_FILE2; i += 2)
		*dev_buffer++ = ((uint16_t)buffer[i] << 8) | buffer[i + 1];

	if (m_command == 0x2a && is_mode2_audio_selected(buffer))
	{
		m_data_buffer |= 0x0004;
		dev_buffer += 0x1400;
	}

	for (int i = SECTOR_FILE2; i < SECTOR_SIZE; i += 2)
		*dev_buffer++ = ((uint16_t)buffer[i] << 8) | buffer[i + 1];

	for (int i = SUBCODE_Q_CONTROL; i <= SUBCODE_Q_CRC1; i++)
		*dev_buffer++ = subcode_buffer[i];

	m_x_buffer |= 0x8000;
	m_data_buffer |= 0x4000;
	update_interrupt_state();

	if (m_command == 0x23 || m_command == 0x24) // Reset? If so, stop.
		cancel_disc_read();
}

uint16_t cdicdic_device::regs_r(offs_t offset, uint16_t mem_mask)
{
	uint32_t addr = offset + 0x3c00/2;

	switch (addr)
	{
		case 0x3c00/2: // Command register
			LOGMASKED(LOG_READS, "%s: cdic_r: Command Register = %04x & %04x\n", machine().describe_context(), m_command, mem_mask);
			return m_command;

		case 0x3c02/2: // Time register (MSW)
			LOGMASKED(LOG_READS, "%s: cdic_r: Time Register (MSW) = %04x & %04x\n", machine().describe_context(), m_time >> 16, mem_mask);
			return m_time >> 16;

		case 0x3c04/2: // Time register (LSW)
			LOGMASKED(LOG_READS, "%s: cdic_r: Time Register (LSW) = %04x & %04x\n", machine().describe_context(), (uint16_t)(m_time & 0x0000ffff), mem_mask);
			return m_time & 0x0000ffff;

		case 0x3c06/2: // File register
			LOGMASKED(LOG_READS, "%s: cdic_r: File Register = %04x & %04x\n", machine().describe_context(), m_file, mem_mask);
			return m_file;

		case 0x3c08/2: // Channel register (MSW)
			LOGMASKED(LOG_READS, "%s: cdic_r: Channel Register (MSW) = %04x & %04x\n", machine().describe_context(), m_channel >> 16, mem_mask);
			return m_channel >> 16;

		case 0x3c0a/2: // Channel register (LSW)
			LOGMASKED(LOG_READS, "%s: cdic_r: Channel Register (LSW) = %04x & %04x\n", machine().describe_context(), m_channel & 0x0000ffff, mem_mask);
			return m_channel & 0x0000ffff;

		case 0x3c0c/2: // Audio Channel register
			LOGMASKED(LOG_READS, "%s: cdic_r: Audio Channel Register = %04x & %04x\n", machine().describe_context(), m_audio_channel, mem_mask);
			return m_audio_channel;

		case 0x3ff4/2: // ABUF
		{
			uint16_t temp = m_audio_buffer;
			LOGMASKED(LOG_READS, "%s: cdic_r: Audio Buffer Register = %04x & %04x\n", machine().describe_context(), temp, mem_mask);
			m_audio_buffer &= 0x7fff;
			update_interrupt_state();
			return temp;
		}

		case 0x3ff6/2: // XBUF
		{
			uint16_t temp = m_x_buffer;
			LOGMASKED(LOG_READS, "%s: cdic_r: X-Buffer Register = %04x & %04x\n", machine().describe_context(), temp, mem_mask);
			m_x_buffer &= 0x7fff;
			update_interrupt_state();
			return temp;
		}

		case 0x3ffa/2: // AUDCTL
			if (!m_decoding_audio_map)
				m_z_buffer ^= 0x0001;
			LOGMASKED(LOG_READS, "%s: cdic_r: Z-Buffer Register Read: %04x & %04x\n", machine().describe_context(), m_z_buffer, mem_mask);
			return m_z_buffer;

		case 0x3ffe/2:
			LOGMASKED(LOG_READS, "%s: cdic_r: Data buffer Register = %04x & %04x\n", machine().describe_context(), m_data_buffer, mem_mask);
			return m_data_buffer;

		default:
			LOGMASKED(LOG_READS | LOG_UNKNOWNS, "%s: cdic_r: Unknown address: %04x & %04x\n", machine().describe_context(), addr*2, mem_mask);
			return 0;
	}
}

void cdicdic_device::regs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t addr = offset + 0x3c00/2;

	switch (addr)
	{
		case 0x3c00/2: // Command register
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Command Register = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_command);
			break;

		case 0x3c02/2: // Time register (MSW)
			m_time &= ~(mem_mask << 16);
			m_time |= (data & mem_mask) << 16;
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Time Register (MSW) = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;

		case 0x3c04/2: // Time register (LSW)
			m_time &= ~mem_mask;
			m_time |= data & mem_mask;
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Time Register (LSW) = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;

		case 0x3c06/2: // File register
			LOGMASKED(LOG_WRITES, "%s: cdic_w: File Register = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_file);
			break;

		case 0x3c08/2: // Channel register (MSW)
			m_channel &= ~(mem_mask << 16);
			m_channel |= (data & mem_mask) << 16;
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Channel Register (MSW) = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;

		case 0x3c0a/2: // Channel register (LSW)
			m_channel &= ~mem_mask;
			m_channel |= data & mem_mask;
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Channel Register (LSW) = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			break;

		case 0x3c0c/2: // Audio Channel register
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Audio Channel Register = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_audio_channel);
			break;

		case 0x3ff4/2:
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Audio Buffer Register = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_audio_buffer);
			break;

		case 0x3ff6/2:
			LOGMASKED(LOG_WRITES, "%s: cdic_w: X Buffer Register = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_x_buffer);
			break;

		case 0x3ff8/2:
		{
			uint32_t start = m_scc->dma().channel[0].memory_address_counter;
			uint32_t count = m_scc->dma().channel[0].transfer_counter;
			uint32_t device_index = (data & 0x3fff) >> 1;
			uint16_t *ram = (uint16_t *)m_ram.get();
			LOGMASKED(LOG_WRITES, "%s: cdic_w: DMA Control Register = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			LOGMASKED(LOG_WRITES, "%s: Memory address counter: %08x\n", machine().describe_context(), m_scc->dma().channel[0].memory_address_counter);
			LOGMASKED(LOG_WRITES, "%s: Doing copy, transferring %04x bytes %s\n", machine().describe_context(), count * 2, (m_scc->dma().channel[0].operation_control & SCC68070_OCR_D) ? "to main RAM" : "to device RAM");
			for (uint32_t index = start / 2; index < (start / 2 + count); index++)
			{
				if (m_scc->dma().channel[0].operation_control & SCC68070_OCR_D)
				{
					m_memory_space->write_word(index * 2, ram[device_index++]);
				}
				else
				{
					ram[device_index++] = m_memory_space->read_word(index * 2);
				}
			}
			m_scc->dma().channel[0].memory_address_counter += m_scc->dma().channel[0].transfer_counter * 2;
			break;
		}

		case 0x3ffa/2:
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Z-Buffer Register Write: %04x & %04x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_z_buffer);
			if (!(m_z_buffer & 0x2000))
			{
				m_decode_addr = 0xffff;
			}
			else if (!m_decoding_audio_map)
			{
				m_decode_addr = m_z_buffer & 0x3a00;
				m_audio_format_sectors = 0;
				m_audio_sector_counter = 1;
				m_decoding_audio_map = true;
				std::fill_n(&m_xa_last[0], 4, 0);
			}
			break;

		case 0x3ffc/2:
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Interrupt Vector Register = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_interrupt_vector);
			break;

		case 0x3ffe/2:
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Data Buffer Register = %04x & %04x\n", machine().describe_context(), data, mem_mask);
			COMBINE_DATA(&m_data_buffer);
			if (m_data_buffer & 0x8000)
			{
				LOGMASKED(LOG_WRITES, "%s: cdic_w: Data Buffer high-bit set, beginning command processing\n", machine().describe_context());
				handle_cdic_command();
			}
			if (!(m_data_buffer & 0x4000))
			{
				m_disc_command = 0;
				m_disc_mode = 0;
				m_disc_spinup_counter = 0;
				m_curr_lba = 0;
			}
			break;

		default:
			LOGMASKED(LOG_WRITES | LOG_UNKNOWNS, "%s: cdic_w: Unknown address: %04x = %04x & %04x\n", machine().describe_context(), addr*2, data, mem_mask);
			break;
	}
}

void cdicdic_device::init_disc_read(uint8_t disc_mode)
{
	m_disc_command = m_command;
	m_disc_mode = disc_mode;
	m_curr_lba = lba_from_time();
	m_disc_spinup_counter = 1;
}

void cdicdic_device::cancel_disc_read()
{
	m_disc_command = 0;
	m_disc_mode = 0;
	m_curr_lba = 0;
	m_disc_spinup_counter = 0;
}

void cdicdic_device::handle_cdic_command()
{
	switch (m_command)
	{
		case 0x23: // Reset Mode 1
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Reset Mode 1 command\n", machine().describe_context());
			if (m_disc_command == 0)
				init_disc_read(DISC_MODE1);
			break;
		case 0x24: // Reset Mode 2
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Reset Mode 2 command\n", machine().describe_context());
			if (m_disc_command == 0)
				init_disc_read(DISC_MODE1);
			break;
		case 0x2b: // Stop CDDA
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Stop CDDA command\n", machine().describe_context());
			cancel_disc_read();
			break;
		case 0x2e: // Update
			LOGMASKED(LOG_WRITES, "%s: cdic_w: Update command\n", machine().describe_context());
			break;
		case 0x27: // Fetch TOC
			init_disc_read(DISC_TOC);
			break;
		case 0x28: // Play CDDA
			init_disc_read(DISC_CDDA);
			break;
		case 0x29: // Read Mode 1
		case 0x2c: // Seek
			init_disc_read(DISC_MODE1);
			break;
		case 0x2a: // Read Mode 2
			init_disc_read(DISC_MODE2);
			break;
	}

	m_data_buffer &= ~0x8000;
}

uint32_t cdicdic_device::lba_from_time()
{
	const uint8_t bcd_mins = (m_time >> 24) & 0xff;
	const uint8_t mins_upper_digit = bcd_mins >> 4;
	const uint8_t mins_lower_digit = bcd_mins & 0xf;
	const uint8_t raw_mins = (mins_upper_digit * 10) + mins_lower_digit;

	const uint8_t bcd_secs = (m_time >> 16) & 0xff;
	const uint8_t secs_upper_digit = bcd_secs >> 4;
	const uint8_t secs_lower_digit = bcd_secs & 0xf;
	const uint8_t raw_secs = (secs_upper_digit * 10) + secs_lower_digit;

	uint32_t lba = ((raw_mins * 60) + raw_secs) * 75;

	const uint8_t bcd_frac = (m_time >> 8) & 0xff;
	const bool even_second = BIT(bcd_frac, 7);
	if (!even_second)
	{
		const uint8_t frac_upper_digit = bcd_frac >> 4;
		const uint8_t frac_lower_digit = bcd_frac & 0xf;
		const uint8_t raw_frac = (frac_upper_digit * 10) + frac_lower_digit;
		lba += raw_frac;
	}

	if (lba >= 150)
		lba -= 150;

	return lba;
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdicdic_device - constructor
//-------------------------------------------------

cdicdic_device::cdicdic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CDI_CDIC, tag, owner, clock)
	, m_intreq_callback(*this)
	, m_memory_space(*this, ":maincpu", AS_PROGRAM)
	, m_dmadac(*this, ":dac%u", 1U)
	, m_scc(*this, ":maincpu")
	, m_cdrom(*this, ":cdrom")
	, m_clock2(clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdicdic_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x4000);
	m_samples[0] = std::make_unique<int16_t[]>(s_samples_per_sector * 8 + 16);
	m_samples[1] = std::make_unique<int16_t[]>(s_samples_per_sector * 8 + 16);

	save_pointer(NAME(m_ram), 0x4000);
	save_pointer(NAME(m_samples[0]), s_samples_per_sector * 8 + 16);
	save_pointer(NAME(m_samples[1]), s_samples_per_sector * 8 + 16);

	save_item(NAME(m_command));
	save_item(NAME(m_time));
	save_item(NAME(m_file));
	save_item(NAME(m_channel));
	save_item(NAME(m_audio_channel));
	save_item(NAME(m_audio_buffer));
	save_item(NAME(m_x_buffer));
	save_item(NAME(m_dma_control));
	save_item(NAME(m_z_buffer));
	save_item(NAME(m_interrupt_vector));
	save_item(NAME(m_data_buffer));

	save_item(NAME(m_disc_command));
	save_item(NAME(m_disc_mode));
	save_item(NAME(m_disc_spinup_counter));
	save_item(NAME(m_curr_lba));

	save_item(NAME(m_audio_sector_counter));
	save_item(NAME(m_audio_format_sectors));
	save_item(NAME(m_decoding_audio_map));
	save_item(NAME(m_decode_addr));

	save_item(NAME(m_xa_last));

	m_audio_timer = timer_alloc(FUNC(cdicdic_device::audio_tick), this);
	m_audio_timer->adjust(attotime::never);

	m_sector_timer = timer_alloc(FUNC(cdicdic_device::sector_tick), this);
	m_sector_timer->adjust(attotime::never);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cdicdic_device::device_reset()
{
	m_command = 0;
	m_time = 0;
	m_file = 0;
	m_channel = 0xffffffff;
	m_audio_channel = 0xffff;
	m_audio_buffer = 0;
	m_x_buffer = 0;
	m_dma_control = 0;
	m_z_buffer = 0;
	m_interrupt_vector = 0x0f;
	m_data_buffer = 0;

	m_cd_byteswap = false;

	m_disc_command = 0;
	m_disc_mode = 0;
	m_disc_spinup_counter = 0;
	m_curr_lba = 0;

	m_audio_sector_counter = 0;
	m_audio_format_sectors = 0;
	m_decoding_audio_map = false;
	m_decode_addr = 0;

	m_audio_timer->adjust(attotime::from_hz(75), 0, attotime::from_hz(75));
	m_sector_timer->adjust(attotime::from_hz(75), 0, attotime::from_hz(75));

	m_intreq_callback(CLEAR_LINE);

	m_dmadac[0]->enable(1);
	m_dmadac[1]->enable(1);

	std::fill_n(&m_xa_last[0], 4, 0);
}

void cdicdic_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_RAM, "%s: ram_w: %04x = %04x & %04x\n", machine().describe_context(), offset << 1, data, mem_mask);
	COMBINE_DATA((uint16_t *)&m_ram[offset << 1]);
}

uint16_t cdicdic_device::ram_r(offs_t offset, uint16_t mem_mask)
{
	const uint16_t data = ((uint16_t)m_ram[(offset << 1) + 1] << 8) | m_ram[offset << 1];
	LOGMASKED(LOG_RAM, "%s: ram_r: %04x : %04x & %04x\n", machine().describe_context(), offset << 1, data, mem_mask);
	return data;
}

uint8_t cdicdic_device::intack_r()
{
	return m_interrupt_vector & 0xff;
}
