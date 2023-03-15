// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega G-80 raster hardware

***************************************************************************/

#include "emu.h"
#include "segag80_m.h"


/****************************************************************************/
/* MB 971025 - Emulate Sega G80 security chip 315-0062                      */
/****************************************************************************/
static uint8_t sega_decrypt62(offs_t pc, uint8_t lo)
{
	uint32_t i = 0;
	uint32_t b = lo;

	switch (pc & 0x03)
	{
		case 0x00:
			/* D */
			i=b & 0x23;
			i+=((b    & 0xC0) >> 4);
			i+=((b    & 0x10) << 2);
			i+=((b    & 0x08) << 1);
			i+=(((~b) & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x01:
			/* C */
			i=b & 0x03;
			i+=((b    & 0x80) >> 4);
			i+=(((~b) & 0x40) >> 1);
			i+=((b    & 0x20) >> 1);
			i+=((b    & 0x10) >> 2);
			i+=((b    & 0x08) << 3);
			i+=((b    & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x02:
			/* B */
			i=b & 0x03;
			i+=((b    & 0x80) >> 1);
			i+=((b    & 0x60) >> 3);
			i+=((~b) & 0x10);
			i+=((b    & 0x08) << 2);
			i+=((b    & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x03:
			/* A */
			i=b;
			break;
	}

	return i;
}

/****************************************************************************/
/* MB 971025 - Emulate Sega G80 security chip 315-0063                      */
/****************************************************************************/
static uint8_t sega_decrypt63(offs_t pc, uint8_t lo)
{
	uint32_t i = 0;
	uint32_t b = lo;

	switch (pc & 0x09)
	{
		case 0x00:
			/* D */
			i=b & 0x23;
			i+=((b    & 0xC0) >> 4);
			i+=((b    & 0x10) << 2);
			i+=((b    & 0x08) << 1);
			i+=(((~b) & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x01:
			/* C */
			i=b & 0x03;
			i+=((b    & 0x80) >> 4);
			i+=(((~b) & 0x40) >> 1);
			i+=((b    & 0x20) >> 1);
			i+=((b    & 0x10) >> 2);
			i+=((b    & 0x08) << 3);
			i+=((b    & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x08:
			/* B */
			i=b & 0x03;
			i+=((b    & 0x80) >> 1);
			i+=((b    & 0x60) >> 3);
			i+=((~b) & 0x10);
			i+=((b    & 0x08) << 2);
			i+=((b    & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x09:
			/* A */
			i=b;
			break;
	}

	return i;
}

/****************************************************************************/
/* MB 971025 - Emulate Sega G80 security chip 315-0064                      */
/****************************************************************************/
static uint8_t sega_decrypt64(offs_t pc, uint8_t lo)
{
	uint32_t i = 0;
	uint32_t b = lo;

	switch (pc & 0x03)
	{
		case 0x00:
			/* A */
			i=b;
			break;
		case 0x01:
			/* B */
			i=b & 0x03;
			i+=((b    & 0x80) >> 1);
			i+=((b    & 0x60) >> 3);
			i+=((~b) & 0x10);
			i+=((b    & 0x08) << 2);
			i+=((b    & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x02:
			/* C */
			i=b & 0x03;
			i+=((b    & 0x80) >> 4);
			i+=(((~b) & 0x40) >> 1);
			i+=((b    & 0x20) >> 1);
			i+=((b    & 0x10) >> 2);
			i+=((b    & 0x08) << 3);
			i+=((b    & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x03:
			/* D */
			i=b & 0x23;
			i+=((b    & 0xC0) >> 4);
			i+=((b    & 0x10) << 2);
			i+=((b    & 0x08) << 1);
			i+=(((~b) & 0x04) << 5);
			i &= 0xFF;
			break;
	}

	return i;
}


/****************************************************************************/
/* MB 971025 - Emulate Sega G80 security chip 315-0070                      */
/****************************************************************************/
static uint8_t sega_decrypt70(offs_t pc, uint8_t lo)
{
	uint32_t i = 0;
	uint32_t b = lo;

	switch (pc & 0x09)
	{
		case 0x00:
			/* B */
			i=b & 0x03;
			i+=((b    & 0x80) >> 1);
			i+=((b    & 0x60) >> 3);
			i+=((~b) & 0x10);
			i+=((b    & 0x08) << 2);
			i+=((b    & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x01:
			/* A */
			i=b;
			break;
		case 0x08:
			/* D */
			i=b & 0x23;
			i+=((b    & 0xC0) >> 4);
			i+=((b    & 0x10) << 2);
			i+=((b    & 0x08) << 1);
			i+=(((~b) & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x09:
			/* C */
			i=b & 0x03;
			i+=((b    & 0x80) >> 4);
			i+=(((~b) & 0x40) >> 1);
			i+=((b    & 0x20) >> 1);
			i+=((b    & 0x10) >> 2);
			i+=((b    & 0x08) << 3);
			i+=((b    & 0x04) << 5);
			i &= 0xFF;
			break;
	}

	return i;
}

/****************************************************************************/
/* MB 971025 - Emulate Sega G80 security chip 315-0076                      */
/****************************************************************************/
static uint8_t sega_decrypt76(offs_t pc, uint8_t lo)
{
	uint32_t i = 0;
	uint32_t b = lo;

	switch (pc & 0x09)
	{
		case 0x00:
			/* A */
			i=b;
			break;
		case 0x01:
			/* B */
			i=b & 0x03;
			i+=((b    & 0x80) >> 1);
			i+=((b    & 0x60) >> 3);
			i+=((~b) & 0x10);
			i+=((b    & 0x08) << 2);
			i+=((b    & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x08:
			/* C */
			i=b & 0x03;
			i+=((b    & 0x80) >> 4);
			i+=(((~b) & 0x40) >> 1);
			i+=((b    & 0x20) >> 1);
			i+=((b    & 0x10) >> 2);
			i+=((b    & 0x08) << 3);
			i+=((b    & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x09:
			/* D */
			i=b & 0x23;
			i+=((b    & 0xC0) >> 4);
			i+=((b    & 0x10) << 2);
			i+=((b    & 0x08) << 1);
			i+=(((~b) & 0x04) << 5);
			i &= 0xFF;
			break;
	}

	return i;
}

/****************************************************************************/
/* MB 971025 - Emulate Sega G80 security chip 315-0082                      */
/****************************************************************************/
static uint8_t sega_decrypt82(offs_t pc, uint8_t lo)
{
	uint32_t i = 0;
	uint32_t b = lo;

	switch (pc & 0x11)
	{
		case 0x00:
			/* A */
			i=b;
			break;
		case 0x01:
			/* B */
			i=b & 0x03;
			i+=((b    & 0x80) >> 1);
			i+=((b    & 0x60) >> 3);
			i+=((~b) & 0x10);
			i+=((b    & 0x08) << 2);
			i+=((b    & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x10:
			/* C */
			i=b & 0x03;
			i+=((b    & 0x80) >> 4);
			i+=(((~b) & 0x40) >> 1);
			i+=((b    & 0x20) >> 1);
			i+=((b    & 0x10) >> 2);
			i+=((b    & 0x08) << 3);
			i+=((b    & 0x04) << 5);
			i &= 0xFF;
			break;
		case 0x11:
			/* D */
			i=b & 0x23;
			i+=((b    & 0xC0) >> 4);
			i+=((b    & 0x10) << 2);
			i+=((b    & 0x08) << 1);
			i+=(((~b) & 0x04) << 5);
			i &= 0xFF;
			break;
	}

	return i;
}

/****************************************************************************/
/* MB 971031 - Emulate no Sega G80 security chip                            */
/****************************************************************************/
static uint8_t sega_decrypt0(offs_t pc, uint8_t lo)
{
		return lo;
}

/****************************************************************************/
/* MB 971025 - Set the security chip to be used                             */
/****************************************************************************/
segag80_decrypt_func segag80_security(int chip)
{
	switch (chip)
	{
		case 62:
			return sega_decrypt62;
		case 63:
			return sega_decrypt63;
		case 64:
			return sega_decrypt64;
		case 70:
			return sega_decrypt70;
		case 76:
			return sega_decrypt76;
		case 82:
			return sega_decrypt82;
		default:
			return sega_decrypt0;
	}
}
