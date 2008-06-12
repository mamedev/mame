/***************************************************************************

    Gaelco CG-1V/GAE1 based games

    Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
    I/O ports)

***************************************************************************/

#include "driver.h"
#include "machine/eeprom.h"
#include "includes/gaelco2.h"

/***************************************************************************

    Split even/odd bytes from ROMs in 16 bit mode to different memory areas

***************************************************************************/

static void gaelco2_ROM16_split(int src_reg, int dst_reg, int start, int length, int dest1, int dest2)
{
	int i;

	/* get a pointer to the source data */
	UINT8 *src = (UINT8 *)memory_region(src_reg);

	/* get a pointer to the destination data */
	UINT8 *dst = (UINT8 *)memory_region(dst_reg);

	/* fill destination areas with the proper data */
	for (i = 0; i < length/2; i++){
		dst[dest1 + i] = src[start + i*2 + 0];
		dst[dest2 + i] = src[start + i*2 + 1];
	}
}


/***************************************************************************

    Driver init routines

***************************************************************************/

DRIVER_INIT( alighunt )
{
	/*
    For REGION_GFX2 we have this memory map:
        0x0000000-0x03fffff ROM u48
        0x0400000-0x07fffff ROM u47
        0x0800000-0x0bfffff ROM u50
        0x0c00000-0x0ffffff ROM u49

    and we are going to construct this one for REGION_GFX1:
        0x0000000-0x01fffff ROM u48 even bytes
        0x0200000-0x03fffff ROM u47 even bytes
        0x0400000-0x05fffff ROM u48 odd bytes
        0x0600000-0x07fffff ROM u47 odd bytes
        0x0800000-0x09fffff ROM u50 even bytes
        0x0a00000-0x0bfffff ROM u49 even bytes
        0x0c00000-0x0dfffff ROM u50 odd bytes
        0x0e00000-0x0ffffff ROM u49 odd bytes
    */

	/* split ROM u48 */
	gaelco2_ROM16_split(REGION_GFX2, REGION_GFX1, 0x0000000, 0x0400000, 0x0000000, 0x0400000);

	/* split ROM u47 */
	gaelco2_ROM16_split(REGION_GFX2, REGION_GFX1, 0x0400000, 0x0400000, 0x0200000, 0x0600000);

	/* split ROM u50 */
	gaelco2_ROM16_split(REGION_GFX2, REGION_GFX1, 0x0800000, 0x0400000, 0x0800000, 0x0c00000);

	/* split ROM u49 */
	gaelco2_ROM16_split(REGION_GFX2, REGION_GFX1, 0x0c00000, 0x0400000, 0x0a00000, 0x0e00000);
}


DRIVER_INIT( touchgo )
{
	/*
    For REGION_GFX2 we have this memory map:
        0x0000000-0x03fffff ROM ic65
        0x0400000-0x05fffff ROM ic66
        0x0800000-0x0bfffff ROM ic67

    and we are going to construct this one for REGION_GFX1:
        0x0000000-0x01fffff ROM ic65 even bytes
        0x0200000-0x02fffff ROM ic66 even bytes
        0x0400000-0x05fffff ROM ic65 odd bytes
        0x0600000-0x06fffff ROM ic66 odd bytes
        0x0800000-0x09fffff ROM ic67 even bytes
        0x0c00000-0x0dfffff ROM ic67 odd bytes
    */

	/* split ROM ic65 */
	gaelco2_ROM16_split(REGION_GFX2, REGION_GFX1, 0x0000000, 0x0400000, 0x0000000, 0x0400000);

	/* split ROM ic66 */
	gaelco2_ROM16_split(REGION_GFX2, REGION_GFX1, 0x0400000, 0x0200000, 0x0200000, 0x0600000);

	/* split ROM ic67 */
	gaelco2_ROM16_split(REGION_GFX2, REGION_GFX1, 0x0800000, 0x0400000, 0x0800000, 0x0c00000);
}


DRIVER_INIT( snowboar )
{
	/*
    For REGION_GFX2 we have this memory map:
        0x0000000-0x03fffff ROM sb44
        0x0400000-0x07fffff ROM sb45
        0x0800000-0x0bfffff ROM sb46

    and we are going to construct this one for REGION_GFX1:
        0x0000000-0x01fffff ROM sb44 even bytes
        0x0200000-0x03fffff ROM sb45 even bytes
        0x0400000-0x05fffff ROM sb44 odd bytes
        0x0600000-0x07fffff ROM sb45 odd bytes
        0x0800000-0x09fffff ROM sb46 even bytes
        0x0c00000-0x0dfffff ROM sb46 odd bytes
    */

	/* split ROM sb44 */
	gaelco2_ROM16_split(REGION_GFX2, REGION_GFX1, 0x0000000, 0x0400000, 0x0000000, 0x0400000);

	/* split ROM sb45 */
	gaelco2_ROM16_split(REGION_GFX2, REGION_GFX1, 0x0400000, 0x0400000, 0x0200000, 0x0600000);

	/* split ROM sb46 */
	gaelco2_ROM16_split(REGION_GFX2, REGION_GFX1, 0x0800000, 0x0400000, 0x0800000, 0x0c00000);
}

/***************************************************************************

    Coin counters/lockouts

***************************************************************************/

WRITE16_HANDLER( gaelco2_coin_w )
{
	/* Coin Lockouts */
	coin_lockout_w(0, ~data & 0x01);
	coin_lockout_w(1, ~data & 0x02);

	/* Coin Counters */
	coin_counter_w(0, data & 0x04);
	coin_counter_w(1, data & 0x08);
}

WRITE16_HANDLER( gaelco2_coin2_w )
{
	/* coin counters */
	coin_counter_w(offset & 0x01,  data & 0x01);
}

WRITE16_HANDLER( wrally2_coin_w )
{
	/* coin counters */
	coin_counter_w((offset >> 3) & 0x01,  data & 0x01);
}

/***************************************************************************

    EEPROM (93C66)

***************************************************************************/

static const eeprom_interface gaelco2_eeprom_interface =
{
	8,				/* address bits */
	16,				/* data bits */
	"*110",			/* read command */
	"*101",			/* write command */
	"*111",			/* erase command */
	"*10000xxxxxx",	/* lock command */
	"*10011xxxxxx", /* unlock command */
//  "*10001xxxxxx", /* write all */
//  "*10010xxxxxx", /* erase all */
};

NVRAM_HANDLER( gaelco2 )
{
	if (read_or_write){
		eeprom_save(file);
	} else {
		eeprom_init(&gaelco2_eeprom_interface);

		if (file) eeprom_load(file);
	}
}

READ16_HANDLER( gaelco2_eeprom_r )
{
	/* bit 6 is EEPROM data (DOUT) */
	/* bit 7 is EEPROM ready */
	/* bits 0-5, COINSW, STARTSW & Service */
	return (1 << 7) | (eeprom_read_bit() << 6) | (input_port_read_indexed(machine, 2) & 0x3f);
}

WRITE16_HANDLER( gaelco2_eeprom_cs_w )
{
	/* bit 0 is CS (active low) */
	eeprom_set_cs_line((data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
}

WRITE16_HANDLER( gaelco2_eeprom_sk_w )
{
	/* bit 0 is SK (active high) */
	eeprom_set_clock_line((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE16_HANDLER( gaelco2_eeprom_data_w )
{
	/* bit 0 is EEPROM data (DIN) */
	eeprom_write_bit(data & 0x01);
}

/***************************************************************************

    Protection

***************************************************************************/

UINT16 *snowboar_protection;

/*
    The game writes 2 values and then reads from a memory address.
    If the read value is wrong, the game can crash in some places.
    If we always return 0, the game doesn't crash but you can't see
    the full intro (because it expects 0xffff somewhere).

    The protection handles sound, controls, gameplay and some sprites
*/

READ16_HANDLER( snowboar_protection_r )
{
	logerror("%06x: protection read from %04x\n", activecpu_get_pc(), offset*2);
	return 0x0000;
}

WRITE16_HANDLER( snowboar_protection_w )
{
	COMBINE_DATA(&snowboar_protection[offset]);
	logerror("%06x: protection write %04x to %04x\n", activecpu_get_pc(), data, offset*2);

}
