/****************************************************************************

Royal Mahjong (c) 1981 Nichibutsu
and many other Dyna/Dynax games running in similar bare-bones hardware

driver by Zsolt Vasvari, Nicola Salmoria, Luca Elia

CPU:    Z80 / TLCS-90
Video:  Framebuffer
Sound:  AY-3-8910
OSC:    18.432MHz and 8MHz

---------------------------------------------------------------------------------------------------------------------
Year + Game               Board(s)             CPU      Company            Notes
---------------------------------------------------------------------------------------------------------------------
81  Royal Mahjong                              Z80      Nichibutsu
81? Open Mahjong                               Z80      Sapporo Mechanic
82  Royal Mahjong         ? + FRM-03           Z80      Falcon             bootleg
83  Janyou Part II                             Z80        Cosmo Denshi
84? Jan Oh                FRM-00?              Z80      Toaplan            Incomplete program roms
86  Ippatsu Gyakuten                           Z80      Public/Paradais
86  Don Den Mahjong       D039198L-0           Z80      Dyna Electronics
86  Watashiha Suzumechan                       Z80      Dyna Electronics
87  Mahjong Diplomat      D0706088L1-0         Z80      Dynax
87  Mahjong Studio 101    D1708228L1           Z80      Dynax
87  Tonton                D0908288L1-0         Z80      Dynax
88  Almond Pinky          D1401128L-0 + RM-1D  Z80      Dynax
89  Mahjong Shinkirou     D210301BL2 + FRM-00? TLCS-90  Dynax
89  Mahjong Derringer     D2203018L            Z80      Dynax              Larger palette
90  Mahjong If..?         D29?                 TLCS-90  Dynax              Larger palette
91  Mahjong Vegas         D5011308L1 + FRM-00  TLCS-90
92  Mahjong Cafe Time     D6310128L1-1         TLCS-90  Dynax              Larger palette, RTC
93  Mahjong Cafe Doll     D76052208L-2         TLCS-90  Dynax              Larger palette, RTC, Undumped internal rom
95  Mahjong Tensinhai     D10010318L1          TLCS-90  Dynax              Larger palette, RTC
96  Janputer '96                               Z80      Dynax              Larger palette, RTC
---------------------------------------------------------------------------------------------------------------------

TODO:

- dip switches and inputs in dondenmj, suzume, mjderngr...

- there's something fishy with the bank switching in tontonb/mjdiplob

- majs101b: service mode doesn't work

- mjtensin: random crashes, interrupts related


Stephh's notes (based on the games Z80 code and some tests) :

1) 'royalmah'

  - COIN1 doesn't work correctly, the screen goes black instead of showing the
    credits, and you can start a game but the "phantom" credit is not subtracted;
    with NVRAM support, this means the game would always boot to a black screen.
  - The doesn't seem to be any possibility to play a 2 players game
    (but the inputs are mapped so you can test them in the "test mode").
    P1 IN4 doesn't seem to be needed outside the "test mode" either.

2) 'tontonb'

  - The doesn't seem to be any possibility to play a 2 players game
    (but the inputs are mapped so you can test them in the "test mode")
    P1 IN4 doesn't seem to be needed outside the "test mode" either.

  - I've DELIBERATELY mapped DSW3 before DSW2 to try to spot the common
    things with the other Dynax mahjong games ! Please don't change this !

  - When "Special Combinations" Dip Switch is ON, there is a marker in
    front of a random combination. It's value is *2 then.

3) 'mjdiplob'

  - The doesn't seem to be any possibility to play a 2 players game
    (but the inputs are mapped so you can test them in the "test mode")
    P1 IN4 doesn't seem to be needed outside the "test mode" either.

  - When "Special Combinations" Dip Switch is ON, there is a marker in
    front of a random combination. It's value remains *1 though.
    Could it be a leftover from another game ('tontonb' for exemple) ?

- janptr96: in service mode press in sequence N,Ron,Ron,N to access some
  hidden options. (thanks bnathan)

****************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/tlcs90/tlcs90.h"
#include "machine/msm6242.h"
#include "sound/ay8910.h"


static UINT8 input_port_select, dsw_select, rombank;
static int palette_base;


static PALETTE_INIT( royalmah )
{
	offs_t i;

	for (i = 0; i < memory_region_length(REGION_PROMS); i++)
	{
		UINT8 bit0, bit1, bit2, r, g, b;

		UINT8 data = memory_region(REGION_PROMS)[i];

		/* red component */
		bit0 = (data >> 0) & 0x01;
		bit1 = (data >> 1) & 0x01;
		bit2 = (data >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (data >> 3) & 0x01;
		bit1 = (data >> 4) & 0x01;
		bit2 = (data >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (data >> 6) & 0x01;
		bit2 = (data >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color_rgb(machine,i, r,g,b);
	}
}


static PALETTE_INIT( mjderngr )
{
	offs_t i;

	for (i = 0; i < memory_region_length(REGION_PROMS) / 2; i++)
	{
		UINT16 data = (memory_region(REGION_PROMS)[i] << 8) | memory_region(REGION_PROMS)[i + 0x200];

		/* the bits are in reverse order */
		UINT8 r = BITSWAP8((data >>  0) & 0x1f,7,6,5,0,1,2,3,4 );
		UINT8 g = BITSWAP8((data >>  5) & 0x1f,7,6,5,0,1,2,3,4 );
		UINT8 b = BITSWAP8((data >> 10) & 0x1f,7,6,5,0,1,2,3,4 );

		palette_set_color_rgb(machine,i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}


static WRITE8_HANDLER( royalmah_palbank_w )
{
	/* bit 1 = coin counter */
	coin_counter_w(0,data & 2);

	/* bit 2 always set? */

	/* bit 3 = palette bank */
	palette_base = (data >> 3) & 0x01;
}


static WRITE8_HANDLER( mjderngr_coin_w )
{
	/* bit 1 = coin counter */
	coin_counter_w(0,data & 2);

	/* bit 2 always set? */
}


static WRITE8_HANDLER( mjderngr_palbank_w )
{
	palette_base = data;
}


static VIDEO_UPDATE( royalmah )
{

	offs_t offs;

	for (offs = 0; offs < 0x4000; offs++)
	{
		int i;

		UINT8 data1 = videoram[offs + 0x0000];
		UINT8 data2 = videoram[offs + 0x4000];

		UINT8 y = 255 - (offs >> 6);
		UINT8 x = 255 - (offs << 2);

		for (i = 0; i < 4; i++)
		{
			UINT8 pen = ((data2 >> 1) & 0x08) | ((data2 << 2) & 0x04) | ((data1 >> 3) & 0x02) | ((data1 >> 0) & 0x01);

			*BITMAP_ADDR16(bitmap, y, x) = (palette_base << 4) | pen;

			x = x - 1;
			data1 = data1 >> 1;
			data2 = data2 >> 1;
		}
	}

	return 0;
}





static WRITE8_HANDLER( royalmah_rom_w )
{
	/* using this handler will avoid all the entries in the error log that are the result of
       the RLD and RRD instructions this games uses to print text on the screen */
}


static WRITE8_HANDLER( input_port_select_w )
{
	input_port_select = data;
}

static READ8_HANDLER( royalmah_player_1_port_r )
{
	int ret = (input_port_0_r(machine,offset) & 0xc0) | 0x3f;

	if ((input_port_select & 0x01) == 0)  ret &= input_port_0_r(machine,offset);
	if ((input_port_select & 0x02) == 0)  ret &= input_port_1_r(machine,offset);
	if ((input_port_select & 0x04) == 0)  ret &= input_port_2_r(machine,offset);
	if ((input_port_select & 0x08) == 0)  ret &= input_port_3_r(machine,offset);
	if ((input_port_select & 0x10) == 0)  ret &= input_port_4_r(machine,offset);

	return ret;
}

static READ8_HANDLER( royalmah_player_2_port_r )
{
	int ret = (input_port_5_r(machine,offset) & 0xc0) | 0x3f;

	if ((input_port_select & 0x01) == 0)  ret &= input_port_5_r(machine,offset);
	if ((input_port_select & 0x02) == 0)  ret &= input_port_6_r(machine,offset);
	if ((input_port_select & 0x04) == 0)  ret &= input_port_7_r(machine,offset);
	if ((input_port_select & 0x08) == 0)  ret &= input_port_8_r(machine,offset);
	if ((input_port_select & 0x10) == 0)  ret &= input_port_9_r(machine,offset);

	return ret;
}



static READ8_HANDLER ( majs101b_dsw_r )
{
	switch (dsw_select)
	{
		case 0x00: return readinputport(13);	/* DSW3 */
		case 0x20: return readinputport(14);	/* DSW4 */
		case 0x40: return readinputport(12);	/* DSW2 */
	}
	return 0;
}


static UINT8 suzume_bank;

static READ8_HANDLER ( suzume_dsw_r )
{
	if (suzume_bank & 0x40)
	{
		return suzume_bank;
	}
	else
	{
		switch (suzume_bank)
		{
			case 0x08: return readinputport(14);	/* DSW4 */
			case 0x10: return readinputport(13);	/* DSW3 */
			case 0x18: return readinputport(12);	/* DSW2 */
		}
		return 0;
	}
}

static WRITE8_HANDLER ( suzume_bank_w )
{
	UINT8 *rom = memory_region(REGION_CPU1);
	int address;

	suzume_bank = data;

logerror("%04x: bank %02x\n",activecpu_get_pc(),data);

	/* bits 6, 4 and 3 used for something input related? */

	address = 0x10000 + (data & 0x07) * 0x8000;
	memory_set_bankptr(1,&rom[address]);
}


static WRITE8_HANDLER ( mjapinky_bank_w )
{
	UINT8 *ROM = memory_region(REGION_CPU1);
	rombank = data;
	memory_set_bankptr(1,ROM + 0x10000 + 0x8000 * data);
}

static WRITE8_HANDLER( mjapinky_palbank_w )
{
	flip_screen_set(~data & 4);
	palette_base = (data >> 3) & 0x01;
	coin_counter_w(0,data & 2);	// in
	coin_counter_w(1,data & 1);	// out
}

static READ8_HANDLER( mjapinky_dsw_r )
{
	if (rombank == 0x0e)	return readinputport(13);
	else					return *(memory_region(REGION_CPU1) + 0x10000 + 0x8000 * rombank);
}

static WRITE8_HANDLER ( tontonb_bank_w )
{
	UINT8 *rom = memory_region(REGION_CPU1);
	int address;

logerror("%04x: bank %02x\n",activecpu_get_pc(),data);

	if (data == 0) return;	// tontonb fix?

	data &= 0x0f;

	address = 0x10000 + data * 0x8000;

	memory_set_bankptr(1,&rom[address]);
}


/* bits 5 and 6 seem to affect which Dip Switch to read in 'majs101b' */
static WRITE8_HANDLER ( dynax_bank_w )
{
	UINT8 *rom = memory_region(REGION_CPU1);
	int address;

//logerror("%04x: bank %02x\n",activecpu_get_pc(),data);

	dsw_select = data & 0x60;

	data &= 0x1f;

	address = 0x10000 + data * 0x8000;

	memory_set_bankptr(1,&rom[address]);
}





static ADDRESS_MAP_START( royalmah_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x6fff ) AM_READWRITE( MRA8_ROM, royalmah_rom_w )
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE( 0x8000, 0xffff ) AM_READ( MRA8_BANK1 )	// banked ROMs not present in royalmah
	AM_RANGE( 0x8000, 0xffff ) AM_WRITE( MWA8_RAM ) AM_BASE(&videoram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mjapinky_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x6fff ) AM_READWRITE( MRA8_ROM, royalmah_rom_w )
	AM_RANGE( 0x7000, 0x77ff ) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE( 0x7800, 0x7fff ) AM_RAM
	AM_RANGE( 0x8000, 0x8000 ) AM_READ( mjapinky_dsw_r )
	AM_RANGE( 0x8000, 0xffff ) AM_READ( MRA8_BANK1 )
	AM_RANGE( 0x8000, 0xffff ) AM_WRITE( MWA8_RAM ) AM_BASE(&videoram)
ADDRESS_MAP_END




static ADDRESS_MAP_START( royalmah_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_READ( AY8910_read_port_0_r )
	AM_RANGE( 0x02, 0x02 ) AM_WRITE( AY8910_write_port_0_w )
	AM_RANGE( 0x03, 0x03 ) AM_WRITE( AY8910_control_port_0_w )
	AM_RANGE( 0x10, 0x10 ) AM_READWRITE( input_port_11_r, royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READWRITE( input_port_10_r, input_port_select_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( ippatsu_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_READ( AY8910_read_port_0_r )
	AM_RANGE( 0x02, 0x02 ) AM_WRITE( AY8910_write_port_0_w )
	AM_RANGE( 0x03, 0x03 ) AM_WRITE( AY8910_control_port_0_w )
	AM_RANGE( 0x10, 0x10 ) AM_READWRITE( input_port_11_r, royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READWRITE( input_port_10_r, input_port_select_w )
	AM_RANGE( 0x12, 0x12 ) AM_READ( input_port_12_r )
	AM_RANGE( 0x13, 0x13 ) AM_READ( input_port_13_r )
ADDRESS_MAP_END

static ADDRESS_MAP_START( suzume_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_READ( AY8910_read_port_0_r )
	AM_RANGE( 0x02, 0x02 ) AM_WRITE( AY8910_write_port_0_w )
	AM_RANGE( 0x03, 0x03 ) AM_WRITE( AY8910_control_port_0_w )
	AM_RANGE( 0x10, 0x10 ) AM_READWRITE( input_port_11_r, royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READWRITE( input_port_10_r, input_port_select_w )
	AM_RANGE( 0x80, 0x80 ) AM_READ( suzume_dsw_r )
	AM_RANGE( 0x81, 0x81 ) AM_WRITE( suzume_bank_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( dondenmj_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_READ( AY8910_read_port_0_r )
	AM_RANGE( 0x02, 0x02 ) AM_WRITE( AY8910_write_port_0_w )
	AM_RANGE( 0x03, 0x03 ) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE( 0x10, 0x10 ) AM_READWRITE( input_port_11_r, royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READWRITE( input_port_10_r, input_port_select_w )
	AM_RANGE( 0x85, 0x85 ) AM_READ( input_port_12_r )	// DSW2
	AM_RANGE( 0x86, 0x86 ) AM_READ( input_port_13_r )	// DSW3
	AM_RANGE( 0x87, 0x87 ) AM_WRITE( dynax_bank_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjdiplob_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_READ( AY8910_read_port_0_r )
	AM_RANGE( 0x02, 0x02 ) AM_WRITE( AY8910_write_port_0_w )
	AM_RANGE( 0x03, 0x03 ) AM_WRITE( AY8910_control_port_0_w )
	AM_RANGE( 0x10, 0x10 ) AM_READWRITE( input_port_11_r, royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READWRITE( input_port_10_r, input_port_select_w )
	AM_RANGE( 0x61, 0x61 ) AM_WRITE(tontonb_bank_w)
	AM_RANGE( 0x62, 0x62 ) AM_READ( input_port_12_r )	// DSW2
	AM_RANGE( 0x63, 0x63 ) AM_READ( input_port_13_r )	// DSW3
ADDRESS_MAP_END

static ADDRESS_MAP_START( tontonb_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_READ( AY8910_read_port_0_r )
	AM_RANGE( 0x02, 0x02 ) AM_WRITE( AY8910_write_port_0_w )
	AM_RANGE( 0x03, 0x03 ) AM_WRITE( AY8910_control_port_0_w )
	AM_RANGE( 0x10, 0x10 ) AM_READWRITE( input_port_11_r, royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READWRITE( input_port_10_r, input_port_select_w )
	AM_RANGE( 0x44, 0x44 ) AM_WRITE( tontonb_bank_w )
	AM_RANGE( 0x46, 0x46 ) AM_READ( input_port_13_r )	// DSW2
	AM_RANGE( 0x47, 0x47 ) AM_READ( input_port_12_r )	// DSW3
ADDRESS_MAP_END

static ADDRESS_MAP_START( majs101b_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_READ( AY8910_read_port_0_r )
	AM_RANGE( 0x02, 0x02 ) AM_WRITE( AY8910_write_port_0_w )
	AM_RANGE( 0x03, 0x03 ) AM_WRITE( AY8910_control_port_0_w )
	AM_RANGE( 0x10, 0x10 ) AM_READWRITE( input_port_11_r, royalmah_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READWRITE( input_port_10_r, input_port_select_w )
	AM_RANGE( 0x00, 0x00 ) AM_READWRITE( majs101b_dsw_r, dynax_bank_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjderngr_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x01 ) AM_READ( AY8910_read_port_0_r )
	AM_RANGE( 0x02, 0x02 ) AM_WRITE( AY8910_write_port_0_w )
	AM_RANGE( 0x03, 0x03 ) AM_WRITE( AY8910_control_port_0_w )
//  AM_RANGE( 0x10, 0x10 ) AM_READ( input_port_11_r )
	AM_RANGE( 0x10, 0x10 ) AM_WRITE( mjderngr_coin_w )	// palette bank is set separately
	AM_RANGE( 0x11, 0x11 ) AM_READWRITE( input_port_10_r, input_port_select_w )
	AM_RANGE( 0x20, 0x20 ) AM_WRITE( dynax_bank_w )
	AM_RANGE( 0x40, 0x40 ) AM_READ( input_port_13_r )	// DSW2
	AM_RANGE( 0x4c, 0x4c ) AM_READ( input_port_12_r )	// DSW3
	AM_RANGE( 0x60, 0x60 ) AM_WRITE( mjderngr_palbank_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjapinky_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_WRITE( mjapinky_bank_w )
	AM_RANGE( 0x01, 0x01 ) AM_READ( AY8910_read_port_0_r )
	AM_RANGE( 0x02, 0x02 ) AM_WRITE( AY8910_write_port_0_w )
	AM_RANGE( 0x03, 0x03 ) AM_WRITE( AY8910_control_port_0_w )
	AM_RANGE( 0x04, 0x04 ) AM_READ( input_port_12_r )
	AM_RANGE( 0x10, 0x10 ) AM_READWRITE( input_port_11_r, mjapinky_palbank_w )
	AM_RANGE( 0x11, 0x11 ) AM_READWRITE( input_port_10_r, input_port_select_w )
ADDRESS_MAP_END

/****************************************************************************
                                Janputer '96
****************************************************************************/

static ADDRESS_MAP_START( janptr96_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x5fff) AM_ROM
	AM_RANGE( 0x6000, 0x6fff ) AM_RAMBANK(3)	// nvram
	AM_RANGE( 0x7000, 0x7fff ) AM_RAMBANK(2)	// banked nvram
	AM_RANGE( 0x8000, 0xffff ) AM_READ(MRA8_BANK1)
	AM_RANGE( 0x8000, 0xffff ) AM_WRITE(MWA8_RAM) AM_BASE(&videoram)
ADDRESS_MAP_END

static WRITE8_HANDLER( janptr96_dswsel_w )
{
	// 0x20 = 0 -> hopper on
	// 0x40 ?
	dsw_select = data;
}

static READ8_HANDLER( janptr96_dswsel_r )
{
	return dsw_select;
}

static READ8_HANDLER( janptr96_dsw_r )
{
	if (~dsw_select & 0x01) return readinputportbytag("DSW4");
	if (~dsw_select & 0x02) return readinputportbytag("DSW3");
	if (~dsw_select & 0x04) return readinputportbytag("DSW2");
	if (~dsw_select & 0x08) return readinputportbytag("DSW1");
	if (~dsw_select & 0x10) return readinputportbytag("DSWTOP");
	return 0xff;
}

static WRITE8_HANDLER( janptr96_rombank_w )
{
	UINT8 *ROM = memory_region(REGION_CPU1);
	memory_set_bankptr(1,ROM + 0x10000 + 0x8000 * data);
}

static WRITE8_HANDLER( janptr96_rambank_w )
{
	memory_set_bankptr(2,generic_nvram + 0x1000 + 0x1000 * data);
}

static READ8_HANDLER( janptr96_unknown_r )
{
	// 0x08 = 0 makes the game crash (e.g. in the m-ram test: nested interrupts?)
	return 0xff;
}

static WRITE8_HANDLER( janptr96_coin_counter_w )
{
	flip_screen_set(~data & 4);
	coin_counter_w(0,data & 2);	// in
	coin_counter_w(1,data & 1);	// out
}

static ADDRESS_MAP_START( janptr96_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_WRITE( janptr96_rombank_w )	// BANK ROM Select
	AM_RANGE( 0x1e, 0x1e ) AM_READWRITE( janptr96_dswsel_r, janptr96_dswsel_w )
	AM_RANGE( 0x1c, 0x1c ) AM_READ( janptr96_dsw_r )
	AM_RANGE( 0x20, 0x20 ) AM_READWRITE( janptr96_unknown_r, janptr96_rambank_w )
	AM_RANGE( 0x50, 0x50 ) AM_WRITE( mjderngr_palbank_w )
	AM_RANGE( 0x60, 0x6f ) AM_READWRITE( msm6242_r, msm6242_w )
	AM_RANGE( 0x81, 0x81 ) AM_READ( AY8910_read_port_0_r )
	AM_RANGE( 0x82, 0x82 ) AM_WRITE( AY8910_write_port_0_w )
	AM_RANGE( 0x83, 0x83 ) AM_WRITE( AY8910_control_port_0_w )
	AM_RANGE( 0x93, 0x93 ) AM_WRITE( input_port_select_w )
	AM_RANGE( 0xd8, 0xd8 ) AM_WRITE( janptr96_coin_counter_w )
	AM_RANGE( 0xd9, 0xd9 ) AM_READ( input_port_10_r )
ADDRESS_MAP_END

/****************************************************************************
                                Mahjong If
****************************************************************************/

static UINT8 mjifb_rom_enable;

static WRITE8_HANDLER( mjifb_coin_counter_w )
{
	flip_screen_set( data & 4);
	coin_counter_w(0,data & 2);	// in
	coin_counter_w(1,data & 1);	// out
}

static READ8_HANDLER( mjifb_rom_io_r )
{
	if (mjifb_rom_enable)
		return ((UINT8*)(memory_region(REGION_CPU1) + 0x10000 + rombank * 0x4000))[offset];

	offset += 0x8000;

	switch(offset)
	{
		case 0x8000:	return readinputport(14);		// dsw 4
		case 0x8200:	return readinputport(13);		// dsw 3
		case 0x9001:	return AY8910_read_port_0_r(machine,0);	// inputs
		case 0x9011:	return readinputport(10);
	}

	logerror("%04X: unmapped input read at %04X\n", activecpu_get_pc(), offset);
	return 0xff;
}

static WRITE8_HANDLER( mjifb_rom_io_w )
{
	if (mjifb_rom_enable)
	{
		videoram[offset] = data;
		return;
	}

	offset += 0x8000;

	switch(offset)
	{
		case 0x8e00:	palette_base = data & 0x1f;	return;
		case 0x9002:	AY8910_write_port_0_w(machine,0,data);			return;
		case 0x9003:	AY8910_control_port_0_w(machine,0,data);		return;
		case 0x9010:
			mjifb_coin_counter_w(machine,0,data);
			return;
		case 0x9011:	input_port_select_w(machine,0,data);	return;
		case 0x9013:
//          if (data)   popmessage("%02x",data);
			return;
	}

	logerror("%04X: unmapped input write at %04X = %02X\n", activecpu_get_pc(), offset,data);
}

static WRITE8_HANDLER( mjifb_videoram_w )
{
	videoram[offset + 0x4000] = data;
}

static ADDRESS_MAP_START( mjifb_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x6fff ) AM_ROM
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE( 0x8000, 0xbfff ) AM_READWRITE(mjifb_rom_io_r, mjifb_rom_io_w) AM_BASE(&videoram)
	AM_RANGE( 0xc000, 0xffff ) AM_READWRITE(MRA8_ROM, mjifb_videoram_w)
//  AM_RANGE( 0xc000, 0xffff ) AM_READWRITE(MRA8_ROM, MWA8_RAM)  This should, but doesn't work
ADDRESS_MAP_END

static READ8_HANDLER( mjifb_p3_r )
{
	return readinputport(11) >> 6;
}
static READ8_HANDLER( mjifb_p5_r )
{
	return readinputport(11);
}
static READ8_HANDLER( mjifb_p6_r )
{
	return readinputport(12);
}
static READ8_HANDLER( mjifb_p7_r )
{
	return readinputport(12) >> 4;
}
static READ8_HANDLER( mjifb_p8_r )
{
	return 0xff;
}

static WRITE8_HANDLER( mjifb_p3_w )
{
	rombank = (rombank & 0x0f) | ((data & 0x0c) << 2);
}
static WRITE8_HANDLER( mjifb_p4_w )
{
	rombank = (rombank & 0xf0) | (data & 0x0f);
}
static WRITE8_HANDLER( mjifb_p8_w )
{
	mjifb_rom_enable = (data & 0x08);
}

static ADDRESS_MAP_START( mjifb_iomap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( T90_P3, T90_P3 ) AM_READWRITE( mjifb_p3_r, mjifb_p3_w )
	AM_RANGE( T90_P4, T90_P4 ) AM_WRITE( mjifb_p4_w )
	AM_RANGE( T90_P5, T90_P5 ) AM_READ ( mjifb_p5_r )
	AM_RANGE( T90_P6, T90_P6 ) AM_READ ( mjifb_p6_r )
	AM_RANGE( T90_P7, T90_P7 ) AM_READ ( mjifb_p7_r )
	AM_RANGE( T90_P8, T90_P8 ) AM_READWRITE( mjifb_p8_r, mjifb_p8_w )
ADDRESS_MAP_END


/****************************************************************************
                           Mahjong Shinkirou Deja Vu
****************************************************************************/

static READ8_HANDLER( mjdejavu_rom_io_r )
{
	if (mjifb_rom_enable)
		return ((UINT8*)(memory_region(REGION_CPU1) + 0x10000 + rombank * 0x4000))[offset];

	offset += 0x8000;

	switch(offset)
	{
		case 0x8000:	return readinputport(14);		// dsw 2
		case 0x8001:	return readinputport(13);		// dsw 1
		case 0x9001:	return AY8910_read_port_0_r(machine,0);	// inputs
		case 0x9011:	return readinputport(10);
	}

	logerror("%04X: unmapped input read at %04X\n", activecpu_get_pc(), offset);
	return 0xff;
}

static WRITE8_HANDLER( mjdejavu_rom_io_w )
{
	if (mjifb_rom_enable)
	{
		videoram[offset] = data;
		return;
	}

	offset += 0x8000;
	switch(offset)
	{
		case 0x8802:	palette_base = data & 0x1f;					return;
		case 0x9002:	AY8910_write_port_0_w(machine,0,data);		return;
		case 0x9003:	AY8910_control_port_0_w(machine,0,data);	return;
		case 0x9010:	mjifb_coin_counter_w(machine,0,data);		return;
		case 0x9011:	input_port_select_w(machine,0,data);		return;
		case 0x9013:
//          if (data)   popmessage("%02x",data);
			return;
	}

	logerror("%04X: unmapped input write at %04X = %02X\n", activecpu_get_pc(), offset,data);
}

static ADDRESS_MAP_START( mjdejavu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x6fff ) AM_ROM
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE( 0x8000, 0xbfff ) AM_READWRITE(mjdejavu_rom_io_r, mjdejavu_rom_io_w) AM_BASE(&videoram)
	AM_RANGE( 0xc000, 0xffff ) AM_READWRITE(MRA8_ROM, mjifb_videoram_w)
ADDRESS_MAP_END


/****************************************************************************
                                Mahjong Tensinhai
****************************************************************************/

static READ8_HANDLER( mjtensin_p3_r )
{
	return 0xff;
}

static void mjtensin_update_rombank(void)
{
	memory_set_bankptr( 1, memory_region(REGION_CPU1) + 0x10000 + rombank * 0x8000 );
}
static WRITE8_HANDLER( mjtensin_p4_w )
{
	rombank = (rombank & 0xf0) | (data & 0x0f);
	mjtensin_update_rombank();
}
static WRITE8_HANDLER( mjtensin_6ff3_w )
{
	rombank = (data << 4) | (rombank & 0x0f);
	mjtensin_update_rombank();
}

static ADDRESS_MAP_START( mjtensin_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM
	AM_RANGE( 0x6000, 0x6fbf ) AM_RAM
	AM_RANGE( 0x6fc1, 0x6fc1 ) AM_READ( AY8910_read_port_0_r )
	AM_RANGE( 0x6fc2, 0x6fc2 ) AM_WRITE( AY8910_write_port_0_w )
	AM_RANGE( 0x6fc3, 0x6fc3 ) AM_WRITE( AY8910_control_port_0_w )
	AM_RANGE( 0x6fd0, 0x6fd0 ) AM_WRITE( janptr96_coin_counter_w )
	AM_RANGE( 0x6fd1, 0x6fd1 ) AM_READWRITE( input_port_10_r, input_port_select_w )
	AM_RANGE( 0x6fe0, 0x6fef ) AM_READWRITE( msm6242_r, msm6242_w )
	AM_RANGE( 0x6ff0, 0x6ff0 ) AM_READWRITE( janptr96_dsw_r, janptr96_dswsel_w )
	AM_RANGE( 0x6ff1, 0x6ff1 ) AM_WRITE( mjderngr_palbank_w )
	AM_RANGE( 0x6ff3, 0x6ff3 ) AM_WRITE( mjtensin_6ff3_w )
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE( 0x8000, 0xffff ) AM_READ( MRA8_BANK1 )
	AM_RANGE( 0x8000, 0xffff ) AM_WRITE( MWA8_RAM ) AM_BASE(&videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjtensin_iomap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( T90_P3, T90_P3 ) AM_READ ( mjtensin_p3_r )
	AM_RANGE( T90_P4, T90_P4 ) AM_WRITE( mjtensin_p4_w )
ADDRESS_MAP_END


/****************************************************************************
                                Mahjong Cafe Time
****************************************************************************/

static void cafetime_update_rombank(void)
{
	memory_set_bankptr( 1, memory_region(REGION_CPU1) + 0x10000 + rombank * 0x8000 );
}
static WRITE8_HANDLER( cafetime_p4_w )
{
	rombank = (rombank & 0xf0) | (data & 0x0f);
	cafetime_update_rombank();
}
static WRITE8_HANDLER( cafetime_p3_w )
{
	rombank = (rombank & 0x0f) | ((data & 0x0c) << 2);
	cafetime_update_rombank();
}

static WRITE8_HANDLER( cafetime_dsw_w )
{
	dsw_select = data;
}
static READ8_HANDLER( cafetime_dsw_r )
{
	switch( dsw_select )
	{
		case 0x00: return readinputportbytag("DSW1");
		case 0x01: return readinputportbytag("DSW2");
		case 0x02: return readinputportbytag("DSW3");
		case 0x03: return readinputportbytag("DSW4");
		case 0x04: return readinputportbytag("DSWTOP");
	}
	logerror("%04X: unmapped dsw read %02X\n", activecpu_get_pc(), dsw_select);
	return 0xff;
}

static READ8_HANDLER( cafetime_7fe4_r )
{
	return 0xff;
}
static WRITE8_HANDLER( cafetime_7fe3_w )
{
//  popmessage("%02x",data);
}

static ADDRESS_MAP_START( cafetime_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM
	AM_RANGE( 0x6000, 0x7eff ) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE( 0x7fc1, 0x7fc1 ) AM_READ( AY8910_read_port_0_r )
	AM_RANGE( 0x7fc2, 0x7fc2 ) AM_WRITE( AY8910_write_port_0_w )
	AM_RANGE( 0x7fc3, 0x7fc3 ) AM_WRITE( AY8910_control_port_0_w )
	AM_RANGE( 0x7fd0, 0x7fd0 ) AM_WRITE( janptr96_coin_counter_w )
	AM_RANGE( 0x7fd1, 0x7fd1 ) AM_READWRITE( input_port_10_r, MWA8_NOP )
	AM_RANGE( 0x7fd3, 0x7fd3 ) AM_WRITE( input_port_select_w )
	AM_RANGE( 0x7fe0, 0x7fe0 ) AM_READ( cafetime_dsw_r )
	AM_RANGE( 0x7fe1, 0x7fe1 ) AM_WRITE( cafetime_dsw_w )
	AM_RANGE( 0x7fe2, 0x7fe2 ) AM_WRITE( mjderngr_palbank_w )
	AM_RANGE( 0x7fe3, 0x7fe3 ) AM_WRITE( cafetime_7fe3_w )
	AM_RANGE( 0x7fe4, 0x7fe4 ) AM_READ( cafetime_7fe4_r )
	AM_RANGE( 0x7ff0, 0x7fff ) AM_READWRITE( msm6242_r, msm6242_w )
	AM_RANGE( 0x8000, 0xffff ) AM_READ( MRA8_BANK1 )
	AM_RANGE( 0x8000, 0xffff ) AM_WRITE( MWA8_RAM ) AM_BASE(&videoram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( cafetime_iomap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( T90_P3, T90_P3 ) AM_WRITE( cafetime_p3_w )
	AM_RANGE( T90_P4, T90_P4 ) AM_WRITE( cafetime_p4_w )
ADDRESS_MAP_END


static INPUT_PORTS_START( royalmah )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Credit Clear") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Credit Clear") PORT_CODE(KEYCODE_8)

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )	// "COIN2"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )	// "COIN1", but not working

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START	/* DSW  (inport $10) */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( janyoup2 )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Credit Clear") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Credit Clear") PORT_CODE(KEYCODE_8)

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )	// "COIN2"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )	// "COIN1", but not working

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START	/* DSW  (inport $10) */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START	/* DSW  (inport $12) */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 1-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW  (inport $13) */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( suzume )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2) PORT_CODE(KEYCODE_4)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW2 */
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )
	PORT_DIPSETTING(    0x00, "50 30 20 15 8 6 3 2" )
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW3 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x03, "8" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Background" )
	PORT_DIPSETTING(    0x08, "Black" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Girls" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START	/* DSW4 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tontonb )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START	/* DSW1 (inport $10 -> 0x73b0) */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )		// affects videoram - flip screen ?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode ?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW3 (inport $47 -> 0x73b1) */
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )			// check code at 0x0e6d
	PORT_DIPSETTING(    0x00, "32 24 16 12 8 4 2 1" )	// table at 0x4e7d
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )	// table at 0x4e4d
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )	// table at 0x4e5d
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" )	// table at 0x4e6d
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )		// check code at 0x5184
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )		// stores something at 0x76ff
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )		// check code at 0x1482, 0x18c2, 0x1a1d, 0x1a83, 0x2d2f and 0x2d85
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Maximum Payout ?" )		// check code at 0x1ab7
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "300" )
	PORT_DIPSETTING(    0x60, "500" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )		// check code at 0x18c2, 0x1a1d, 0x2d2f and 0x2d85
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START	/* DSW2 (inport $46 -> 0x73b2) */
	PORT_DIPNAME( 0x01, 0x00, "Special Combinations" )	// see notes
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )		// check code at 0x07c5
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )		// check code at 0x5375
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )		// check code at 0x5241
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )		// untested ?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )		// check code at 0x13aa
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Full Tests" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjdiplob )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START	/* DSW1 (inport $10 -> 0x76fa) */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )		// affects videoram - flip screen ?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode ?" )		// check code at 0x0b94 and 0x0de2
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW2 (inport $62 -> 0x76fb) */
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )			// check code at 0x09cd
	PORT_DIPSETTING(    0x00, "32 24 16 12 8 4 2 1" )	// table at 0x4b82
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )	// table at 0x4b52
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )	// table at 0x4b62
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" )	// table at 0x4b72
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Payout ?" )		// check code at 0x166c
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x10, "200" )
	PORT_DIPSETTING(    0x20, "300" )
	PORT_DIPSETTING(    0x30, "500" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )		// check code at 0x2c64
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )		// check code at 0x2c64
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START	/* DSW3 (inport $63 -> 0x76fc) */
	PORT_DIPNAME( 0x01, 0x00, "Special Combinations" )	// see notes
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )		// check code at 0x531f and 0x5375
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )		// check code at 0x5240
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )		// check code at 0x2411
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )		// check code at 0x2411 and 0x4beb
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )		// check code at 0x24ff, 0x25f2, 0x3fcf and 0x45d7
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" )			// seems to hang after the last animation
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( majs101b )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START	/* DSW1 (inport $10 -> 0x76fd) */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode ?" )		// check code at 0x1635
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW2 (inport $00 (after out 0,$40) -> 0x76fa) */
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )			// check code at 0x14e4
	PORT_DIPSETTING(    0x00, "32 24 16 12 8 4 2 1" )	// table at 0x1539
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )	// table at 0x1509
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )	// table at 0x1519
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" )	// table at 0x1529
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )		// check code at 0x1220, 0x128d, 0x13b1, 0x13cb and 0x2692
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x00, "Maximum Payout ?" )		// check code at 0x12c1
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x10, "300" )
	PORT_DIPSETTING(    0x30, "400" )
	PORT_DIPSETTING(    0x08, "500" )
	PORT_DIPSETTING(    0x28, "600" )
	PORT_DIPSETTING(    0x18, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
//  PORT_DIPSETTING(    0x38, "1000" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )		// check code at 0x1333
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Background" )
	PORT_DIPSETTING(    0x00, "Black" )
	PORT_DIPSETTING(    0x80, "Gray" )

	PORT_START	/* DSW3 (inport $00 (after out 0,$00) -> 0x76fc) */
	PORT_DIPNAME( 0x01, 0x00, "Special Combinations" )	// see notes
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )		// check code at 0x1cf9
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )		// check code at 0x21a9, 0x21dc and 0x2244
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )		// check code at 0x2b7f
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )		// check code at 0x50ba
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )		// check code at 0x1f65
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )		// check code at 0x6412
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )		// check code at 0x2cb2 and 0x2d02
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW4 (inport $00 (after out 0,$20) -> 0x76fb) */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Unknown ) )		// stored at 0x702f - check code at 0x1713,
	PORT_DIPSETTING(    0x00, "0" )				// 0x33d1, 0x3408, 0x3415, 0x347c, 0x3492, 0x350d,
	PORT_DIPSETTING(    0x01, "1" )				// 0x4af9, 0x4b1f and 0x61f6
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPNAME( 0x0c, 0x00, "Difficulty ?" )		// check code at 0x4b5c and 0x6d72
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )				// 0x05 - 0x03, 0x02, 0x02, 0x01
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )			// 0x0a - 0x05, 0x02, 0x02, 0x01
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )				// 0x0f - 0x06, 0x03, 0x02, 0x01
	PORT_DIPSETTING(    0x0c, DEF_STR( Hardest ) )			// 0x14 - 0x0a, 0x06, 0x02, 0x01
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Unknown ) )		// check code at 0x228e
	PORT_DIPSETTING(    0x00, "0x00" )
	PORT_DIPSETTING(    0x10, "0x10" )
	PORT_DIPSETTING(    0x20, "0x20" )
	PORT_DIPSETTING(    0x30, "0x30" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )		// check code at 0x11e4
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" )			// check code at 0x006d
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjapinky )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("DSW1")	/* IN11 */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 1-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 1-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 1-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 1-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x40, 0x00, "Background" )
	PORT_DIPSETTING(    0x40, "Black" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")	/* IN12 */
	PORT_DIPNAME( 0x03, 0x03, "Unknown 2-0&1" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Unknown 2-4&5" )
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 2-6&7" )
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x00, "3" )

	PORT_START_TAG("DSW3")	/* IN13 */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 3-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( janptr96 )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Credit Clear") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Credit Clear") PORT_CODE(KEYCODE_8)

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("DSW4")	/* IN11 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Girls (Demo)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Don Den Key" )
	PORT_DIPSETTING(    0x80, "Start" )
	PORT_DIPSETTING(    0x00, "Flip/Flop" )

	PORT_START_TAG("DSW3")	/* IN12 */
	PORT_DIPNAME( 0x07, 0x07, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x07, "Cut" )
	PORT_DIPSETTING(    0x06, "1 T" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x03, "700" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "1000?" )
	PORT_DIPSETTING(    0x00, "1000?" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")	/* IN13 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Credits To Start" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Payout" )
	PORT_DIPSETTING(    0x30, "300" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x10, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x40, 0x40, "W-BET" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW1")	/* IN14 */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0xc0, "1" )

	PORT_START_TAG("DSWTOP")	/* IN15 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Debug Mode" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Credits Per Note" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjifb )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START	// IN10 - DSW1 (P3 & P5)
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x00, "Maximum Bet" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START	// IN11 - DSW2 (P6 & P7)
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )
	PORT_DIPSETTING(    0x03, "32 24 16 12 8 4 2 1" )
	PORT_DIPSETTING(    0x00, "50 30 15 8 5 3 2 1" )
	PORT_DIPSETTING(    0x01, "100 50 25 10 5 3 2 1" )
	PORT_DIPSETTING(    0x02, "200 100 50 10 5 3 2 1" )
	PORT_DIPNAME( 0x04, 0x04, "Credits Per Note" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x38, 0x38, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x38, "Cut" )
	PORT_DIPSETTING(    0x30, "1 T" )
	PORT_DIPSETTING(    0x28, "300" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x18, "700" )
	PORT_DIPSETTING(    0x10, "1000" )
//  PORT_DIPSETTING(    0x08, "1000?" )
//  PORT_DIPSETTING(    0x00, "1000?" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1/4" )
	PORT_DIPSETTING(    0x80, "2/4" )

	PORT_START	// IN13 - DSW3 ($8200)
	PORT_DIPNAME( 0x01, 0x01, "Unknown 3-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "F-Rate" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Bye-Byte Bonus" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Auto-Mode After Reached" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Background Color" )
	PORT_DIPSETTING(    0x80, "Black" )
	PORT_DIPSETTING(    0x00, "Blue" )

	PORT_START	// IN14 - DSW4 ($8000)
	PORT_DIPNAME( 0x01, 0x01, "Unknown 4-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 4-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Flip-Flop" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Animation" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjtensin )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Credit Clear") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Credit Clear") PORT_CODE(KEYCODE_8)

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("DSW4")	/* IN11 */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x20, "2 3 6 8 12 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Maximum Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START_TAG("DSW3")	/* IN12 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Credits To Start" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Payout" )
	PORT_DIPSETTING(    0x30, "300" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPSETTING(    0x10, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x40, 0x40, "W-BET" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW2")	/* IN13 */
	PORT_DIPNAME( 0x03, 0x03, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x03, "Cut" )
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPSETTING(    0x01, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x18, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x60, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW1")	/* IN14 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Girls (Demo)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Show Clock" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START_TAG("DSWTOP")	/* IN15 */
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cafetime )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START_TAG("DSW1")	/* IN11 */
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x00, "Maximum Bet" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0xc0, 0xc0, "Unknown 1-6&7" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START_TAG("DSW2")	/* IN12 */
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )
	PORT_DIPSETTING(    0x03, "32 24 16 12 8 4 2 1" )
	PORT_DIPSETTING(    0x00, "50 30 15 8 5 3 2 1" )
	PORT_DIPSETTING(    0x01, "100 50 25 10 5 3 2 1" )
	PORT_DIPSETTING(    0x02, "200 100 50 10 5 3 2 1" )
	PORT_DIPNAME( 0x0c, 0x0c, "Unknown 2-2&3" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x70, "Unknown 2-4&5&6" )
	PORT_DIPSETTING(    0x70, "0" )
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x50, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW3")	/* IN13 */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 3-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSW4")	/* IN14 */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 4-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 4-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 4-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Full Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Background" )
	PORT_DIPSETTING(    0x20, "Black" )
	PORT_DIPSETTING(    0x00, "Green" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START_TAG("DSWTOP")	/* IN15 */
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-8" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-9" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Unknown 3-8&9" )
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-8" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-9" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ippatsu )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Credit Clear") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Credit Clear") PORT_CODE(KEYCODE_8)

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )	// "COIN2"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )	// "COIN1", but not working

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START	/* DSW  (inport $10) */
	PORT_DIPNAME( 0x0f, 0x0f, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x40, "7" )
	PORT_DIPSETTING(    0x60, "10" )
	PORT_DIPNAME( 0x80, 0x80, "First Chance" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START	/* DSW  (inport $12) */
	PORT_DIPNAME( 0x03, 0x03, "Cut" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, "2 Bai" )
	PORT_DIPSETTING(    0x01, "3 Bai" )
	PORT_DIPSETTING(    0x03, "Yakuman" )
	PORT_DIPNAME( 0x0c, 0x0c, "Yakuman Bonus" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPSETTING(    0x04, "100" )
	PORT_DIPSETTING(    0x08, "200" )
	PORT_DIPSETTING(    0x0c, "300" )
	PORT_DIPNAME( 0x30, 0x30, "Unknown 1-4&5*" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-6*" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW  (inport $13) */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0*" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Second Bonus" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Allow Bets" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-2" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Unknown 2-3&4*" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjdejavu )
	PORT_START	/* P1 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P1 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN0 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN1 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )PORT_PLAYER(2) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN2 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN3 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* P2 IN4 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* IN10 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )	/* "Note" ("Paper Money") = 10 Credits */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 )	/* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* Analizer (Statistics) */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START	// IN11 - DSW3 (P3 & P5)
	PORT_DIPNAME( 0x03, 0x03, "Unknown 3-0&1*" )
	PORT_DIPSETTING(    0x00, "1 1" )
	PORT_DIPSETTING(    0x02, "3 4" )
	PORT_DIPSETTING(    0x01, "1 2" )
	PORT_DIPSETTING(    0x03, "1 4" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	// IN12 - DSW4 (P6 & P7)
	PORT_DIPNAME( 0x01, 0x01, "Unknown 4-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 4-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 4-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 4-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 4-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Tests" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	// IN13 - DSW1 ($8001)
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPNAME( 0x30, 0x30, "Maximum Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPNAME( 0x40, 0x40, "Credits Per Note" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPNAME( 0x80, 0x80, "Background" )
	PORT_DIPSETTING(    0x80, "Gray" )
	PORT_DIPSETTING(    0x00, "Black" )

	PORT_START	// IN14 - DSW2 ($8000)
	PORT_DIPNAME( 0x03, 0x03, "Winnings" )
	PORT_DIPSETTING(    0x00, "32 24 16 12 8 4 2 1" )
	PORT_DIPSETTING(    0x03, "50 30 15 8 5 3 2 1" )
	PORT_DIPSETTING(    0x02, "100 50 25 10 5 3 2 1" )
	PORT_DIPSETTING(    0x01, "200 100 50 10 5 3 2 1" )
	PORT_DIPNAME( 0x3c, 0x3c, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x3c, "Cut" )
	PORT_DIPSETTING(    0x20, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
//  PORT_DIPSETTING(    0x04, "1000?" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1/4" )
	PORT_DIPSETTING(    0x80, "2/4" )
INPUT_PORTS_END


static const struct AY8910interface ay8910_interface =
{
	royalmah_player_1_port_r,
	royalmah_player_2_port_r
};

static MACHINE_DRIVER_START( royalmah )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", Z80, 3000000)        /* 3.00 MHz ? */
	MDRV_CPU_PROGRAM_MAP(royalmah_map,0)
	MDRV_CPU_IO_MAP(royalmah_iomap,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_VIDEO_UPDATE(royalmah)
	MDRV_PALETTE_LENGTH(16*2)
	MDRV_PALETTE_INIT(royalmah)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 255)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 18432000/12)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( dondenmj )
	MDRV_IMPORT_FROM(royalmah)
	MDRV_CPU_REPLACE("main", Z80, 8000000/2)	/* 4 MHz ? */
	MDRV_CPU_IO_MAP(dondenmj_iomap,0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( ippatsu )
	MDRV_IMPORT_FROM(dondenmj)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_IO_MAP(ippatsu_iomap,0)
MACHINE_DRIVER_END

static INTERRUPT_GEN( suzume_irq )
{
	if ( suzume_bank & 0x40 )
		cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_DRIVER_START( suzume )
	MDRV_IMPORT_FROM(dondenmj)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_IO_MAP(suzume_iomap,0)
	MDRV_CPU_VBLANK_INT("main", suzume_irq)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( tontonb )
	MDRV_IMPORT_FROM(dondenmj)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_IO_MAP(tontonb_iomap,0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( mjdiplob )
	MDRV_IMPORT_FROM(dondenmj)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_IO_MAP(mjdiplob_iomap,0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( majs101b )
	MDRV_IMPORT_FROM(dondenmj)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_IO_MAP(majs101b_iomap,0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( mjapinky )
	MDRV_IMPORT_FROM(dondenmj)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mjapinky_map,0)
	MDRV_CPU_IO_MAP(mjapinky_iomap,0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( mjderngr )
	MDRV_IMPORT_FROM(dondenmj)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_IO_MAP(mjderngr_iomap,0)

	/* video hardware */
	MDRV_PALETTE_LENGTH(16*32)
	MDRV_PALETTE_INIT(mjderngr)
MACHINE_DRIVER_END

/* It runs in IM 2, thus needs a vector on the data bus */
static INTERRUPT_GEN( janptr96_interrupt )
{
	switch(cpu_getiloops())
	{
		case 0:		cpunum_set_input_line_and_vector(machine, 0, 0, HOLD_LINE, 0x80);	break;	// vblank
		case 1:		cpunum_set_input_line_and_vector(machine, 0, 0, HOLD_LINE, 0x82);	break;	// rtc
		default:	cpunum_set_input_line_and_vector(machine, 0, 0, HOLD_LINE, 0x84);			// demo
	}
}

static MACHINE_DRIVER_START( janptr96 )
	MDRV_IMPORT_FROM(mjderngr)
	MDRV_CPU_REPLACE("main",Z80,24000000/4)	/* 6 MHz? */
	MDRV_CPU_PROGRAM_MAP(janptr96_map,0)
	MDRV_CPU_IO_MAP(janptr96_iomap,0)
	MDRV_CPU_VBLANK_INT_HACK(janptr96_interrupt,3)	/* IM 2 needs a vector on the data bus */

	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 9, 255-8)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( mjifb )
	MDRV_IMPORT_FROM(mjderngr)
	MDRV_CPU_REPLACE("main",TMP90841, 8000000)	/* ? */
	MDRV_CPU_PROGRAM_MAP(mjifb_map,0)
	MDRV_CPU_IO_MAP(mjifb_iomap,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 8, 255-8)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( mjdejavu )
	MDRV_IMPORT_FROM(mjderngr)
	MDRV_CPU_REPLACE("main",TMP90841, 8000000)	/* ? */
	MDRV_CPU_PROGRAM_MAP(mjdejavu_map,0)
	MDRV_CPU_IO_MAP(mjifb_iomap,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 8, 255-8)
MACHINE_DRIVER_END


static INTERRUPT_GEN( mjtensin_interrupt )
{
	switch(cpu_getiloops())
	{
		case 0:		cpunum_set_input_line(machine, 0, INPUT_LINE_IRQ0, HOLD_LINE);	break;	// vblank
		case 1:		cpunum_set_input_line(machine, 0, INPUT_LINE_IRQ1, HOLD_LINE);	break;	// rtc
	}
}

static MACHINE_DRIVER_START( mjtensin )
	MDRV_IMPORT_FROM(mjderngr)
	MDRV_CPU_REPLACE("main",TMP90841, 12000000)	/* ? */
	MDRV_CPU_PROGRAM_MAP(mjtensin_map,0)
	MDRV_CPU_IO_MAP(mjtensin_iomap,0)
	MDRV_CPU_VBLANK_INT_HACK( mjtensin_interrupt,2 )

	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 8, 255-8)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( cafetime )
	MDRV_IMPORT_FROM(mjderngr)
	MDRV_CPU_REPLACE("main",TMP90841, 12000000)	/* ? */
	MDRV_CPU_PROGRAM_MAP(cafetime_map,0)
	MDRV_CPU_IO_MAP(cafetime_iomap,0)
	MDRV_CPU_VBLANK_INT_HACK(mjtensin_interrupt,2)

	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 8, 255-8)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/***************************************************************************

Royal Mahjong
(c)1981 Nichibutsu

CPU: Z80
Sound: AY-3-8910
OSC: 18.432MHz

***************************************************************************/

ROM_START( royalmj )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.p1", 0x0000, 0x1000, CRC(549544bb) SHA1(dfb221572c7bfd267a22c0a944830d5f127f9942) )
	ROM_LOAD( "2.p2", 0x1000, 0x1000, CRC(afc8a61e) SHA1(4134f6404f955838fc48fd0f87b83ebc75c1a021) )
	ROM_LOAD( "3.p3", 0x2000, 0x1000, CRC(5d33e54d) SHA1(bf5e0ad5177c086f1cea5c90d7273a841db941bc) )
	ROM_LOAD( "4.p4", 0x3000, 0x1000, CRC(91339560) SHA1(0fb4141e236ab57b3e915dadb982b28ca11d269f) )
	ROM_LOAD( "5.p5", 0x4000, 0x1000, CRC(cc9123a3) SHA1(75276045247a0c9ac5810be01f3b58ad63101f9b) )
	ROM_LOAD( "6.p6", 0x5000, 0x1000, CRC(92150a0f) SHA1(5c97ba5014abdba4afc78e02e7d90e6ca4d777ac) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "18s030n.6k", 0x0000, 0x0020, CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

ROM_START( royalmah )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "rom1",       0x0000, 0x1000, CRC(69b37a62) SHA1(7792528754b0df4e11f4ebe33380b713ac7351a3) )
	ROM_LOAD( "rom2",       0x1000, 0x1000, CRC(0c8351b6) SHA1(9e6b48fd39dd98478d1e3557df839b09652c4349) )
	ROM_LOAD( "rom3",       0x2000, 0x1000, CRC(b7736596) SHA1(4b8bc175d945e695b767b9fb2227ffc1cd4b0547) )
	ROM_LOAD( "rom4",       0x3000, 0x1000, CRC(e3c7c15c) SHA1(a335374cc0f5b1d8e689cc304d006dd97f3e35e7) )
	ROM_LOAD( "rom5",       0x4000, 0x1000, CRC(16c09c73) SHA1(ea712f9ca3200ca27434e4200187b488e24f4c65) )
	ROM_LOAD( "rom6",       0x5000, 0x1000, CRC(92687327) SHA1(4fafba5881dca2a147616d94dd055eba6aa3c653) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "f-rom.bpr",  0x0000, 0x0020, CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

ROM_START( openmj )
	ROM_REGION( 0x7000, REGION_CPU1, ROMREGION_ERASEFF )
	ROM_LOAD( "10", 0x0000, 0x2000, CRC(4042920e) SHA1(19753bcb27ebf391ab824a45c6e41d956826a263) )
	ROM_LOAD( "20", 0x2000, 0x2000, CRC(8fa0f735) SHA1(645154d51c0679b953b9ffc2f1d3b8f2752a0796) )
	ROM_LOAD( "30", 0x4000, 0x2000, CRC(00045cd7) SHA1(0c32995753c1da14dacc8bc6c12dbcbdcae4e1b0) )

	ROM_REGION( 0x20, REGION_PROMS, 0 )
	ROM_LOAD( "82s123.prm", 0x00, 0x20, CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

/***************************************************************************
Janyou Part II
(c)1984 Cosmo Denshi

CPU: Z80
Sound: AY-3-8910
Video: HD46505SP(HD6845SP)
OSC: 18.432MHz

ROMs:
1.C110       [36ebb3d0]
2.C109       [324426d4]
3.C108       [e98b6d34]
4.C107       [377b8ce9]

N82S123N.C98 [d3007282]


Others:
Battery
empty socket for MC68705
***************************************************************************/

ROM_START( janyoup2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.c110",       0x0000, 0x2000, CRC(36ebb3d0) SHA1(39c0cdd1dc5878539768074dad3c39aac4ace8bf) )
	ROM_LOAD( "2.c109",       0x2000, 0x2000, CRC(324426d4) SHA1(409244c8458d9bafa325746c37de9e7b955b3787) )
	ROM_LOAD( "3.c108",       0x4000, 0x2000, CRC(e98b6d34) SHA1(e27ab9a03aff750df78c5db52a112247bdd31328) )
	ROM_LOAD( "4.c107",       0x6000, 0x1000, CRC(377b8ce9) SHA1(a5efc517ae975e54af5325b8b3f4867e9f449d4c) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "n82s123n.c98", 0x0000, 0x0020, CRC(d3007282) SHA1(e4d863ab193e49208ed0f59dcddb1da0492314f6) )
ROM_END

/****************************************************************************

Ippatsu Gyakuten
(c)1986 Public Software / Paradais

modified Royal Mahjong hardware

CPU: Z80
Sound: AY-3-8910

ROMs:
11(27256)
12(27128)
82S123AN

dumped by sayu

****************************************************************************/

ROM_START( ippatsu )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "11", 0x0000, 0x8000, CRC(5f563be7) SHA1(2ce486777bd61a2de789683cd0c8abeefe31775b) )
	ROM_LOAD( "12", 0x8000, 0x4000, CRC(a09a43b0) SHA1(da12e669ccd036da817a69bd549e8668e6a45730) )
	ROM_RELOAD(     0xc000, 0x4000 )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "82s123an", 0x00, 0x20, CRC(3bde1bbd) SHA1(729498483943f960e38c4ada992b099b698b497a) )
ROM_END

ROM_START( suzume )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD( "p1.bin",     0x00000, 0x1000, CRC(e9706967) SHA1(2e3d78178623de6552c9036da90e02f240d94055) )
	ROM_LOAD( "p2.bin",     0x01000, 0x1000, CRC(dd48cd62) SHA1(1ce7b515fabae5054f0ac284a9ed5760f59d18fa) )
	ROM_LOAD( "p3.bin",     0x02000, 0x1000, CRC(10a05c23) SHA1(f13ba660bc5eff9057b1ab46f564f586c76e945d) )
	ROM_LOAD( "p4.bin",     0x03000, 0x1000, CRC(267eaf52) SHA1(56e2f5d7080463dc0f11a2751590ac2b79eb02c5) )
	ROM_LOAD( "p5.bin",     0x04000, 0x1000, CRC(2fde346b) SHA1(7f45aa4427b4cb6bf6cc5919d397b25d53e133f3) )
	ROM_LOAD( "p6.bin",     0x05000, 0x1000, CRC(57f42ac7) SHA1(209b2f62a64ddf544578f144d9ec83478603c8b2) )
	/* bank switched ROMs follow */
	ROM_LOAD( "1.1a",       0x10000, 0x08000, CRC(f670dd47) SHA1(d0236021ae4dd5a10603dde038eb777feeff016f) )	// 0
	ROM_LOAD( "2.1c",       0x18000, 0x08000, CRC(140b11aa) SHA1(6f6a96135434324dcb486596920cb785fe2bf1a2) )	// 1
	ROM_LOAD( "3.1d",       0x20000, 0x08000, CRC(3d437b61) SHA1(175308086e1d7ab566c82dcaeef9f50690edf92a) )	// 2
	ROM_LOAD( "4.1e",       0x28000, 0x08000, CRC(9da8952e) SHA1(956d16b82ff8fe733a7b3135d082e18ea5167dfe) )	// 3
	ROM_LOAD( "5.1h",       0x30000, 0x08000, CRC(04a6f41a) SHA1(37117faf6bc823770413faa7618387ca6f16fa34) )	// 4

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(97e1defe) SHA1(b5002218b2292f7623dd9a205ce183dedeec03f1) )
ROM_END

ROM_START( dondenmj )
	ROM_REGION( 0x90000, REGION_CPU1, 0 )
	ROM_LOAD( "dn5.1h",     0x00000, 0x08000, CRC(3080252e) SHA1(e039087afc36a0c594da093ea599b81a1d757139) )
	/* bank switched ROMs follow */
	ROM_LOAD( "dn1.1e",     0x18000, 0x08000, CRC(1cd9c48a) SHA1(12bc519889dacea59ae49672ad5313fff3a99f12) )	// 1
	ROM_LOAD( "dn2.1d",     0x20000, 0x04000, CRC(7a72929d) SHA1(7955f41883fa53876172bac417955ed0b5eb43f4) )	// 2
	ROM_LOAD( "dn3.2h",     0x30000, 0x08000, CRC(b09d2897) SHA1(0cde3e16ca333be01a5ab3a232f2ea602faec7a2) )	// 4
	ROM_LOAD( "dn4.2e",     0x50000, 0x08000, CRC(67d7dcd6) SHA1(6b708a29de1f4738eb2d4e667327d9433ff7216c) )	// 8

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(97e1defe) SHA1(b5002218b2292f7623dd9a205ce183dedeec03f1) )
ROM_END

ROM_START( mjdiplob )
	ROM_REGION( 0x90000, REGION_CPU1, 0 )
	ROM_LOAD( "071.4l",     0x00000, 0x10000, CRC(81a6d6b0) SHA1(c6169e6d5f35304a0c3efcc2175c3213650f179c) )
	/* bank switched ROMs follow */
	ROM_RELOAD(             0x10000, 0x10000 )				// 0,1
	ROM_LOAD( "072.4k",     0x20000, 0x10000, CRC(a992bb85) SHA1(e60231e04831dac122d1d49a68641ee47b57faaf) )	// 2,3
	ROM_LOAD( "073.4j",     0x30000, 0x10000, CRC(562ed64f) SHA1(42b4a7e5a8de4dde83c12d7b9facf561bc872978) )	// 4,5
	ROM_LOAD( "074.4h",     0x40000, 0x10000, CRC(1eba0140) SHA1(0d0b95be338d7450ad3b24cc47e24e94f86dcefe) )	// 6,7

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(c1e427df) SHA1(9a9980d93dff4b87a940398b18277acaf946eeab) )
ROM_END

ROM_START( tontonb )
	ROM_REGION( 0x90000, REGION_CPU1, 0 )
	ROM_LOAD( "091.5e",   	0x00000, 0x10000, CRC(d8d67b59) SHA1(7e7a85df738f80fc031cda8a104ac9c7b3e24785) )
	/* bank switched ROMs follow */
	ROM_RELOAD(             0x10000, 0x10000 )				// 0,1
	/**/													// 2,3 unused
	ROM_LOAD( "093.5b",   	0x30000, 0x10000, CRC(24b6be55) SHA1(11390d6ed55d7d0b7b84c6d36d4ac5330a06abba) )	// 4,5
	/**/													// 6,7 unused
	ROM_LOAD( "092.5c",   	0x50000, 0x10000, CRC(7ff2738b) SHA1(89a49f89705f499439dc024fc70c87141a84780b) )	// 8,9

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(97e1defe) SHA1(b5002218b2292f7623dd9a205ce183dedeec03f1) )
ROM_END

ROM_START( majs101b )
	ROM_REGION( 0x90000, REGION_CPU1, 0 )
	ROM_LOAD( "171.3e",     0x00000, 0x10000, CRC(fa3c553b) SHA1(fda212559c4d55610a12ad2927afe21f9069c7b6) )
	/* bank switched ROMs follow */
	/**/													// 0,1 unused
	ROM_RELOAD(             0x20000, 0x10000 )				// 2,3
	ROM_LOAD( "172.3f",     0x30000, 0x20000, CRC(7da39a63) SHA1(34d07978a326c83e5b51ce19619d52a75a501795) )	// 4,5,6,7
	ROM_LOAD( "173.3h",     0x50000, 0x20000, CRC(7a9e71ae) SHA1(ce1bde6e05f81b7dbb14015514397ed72f8dd92a) )	// 8,9,a,b
	ROM_LOAD( "174.3j",     0x70000, 0x10000, CRC(972c2cc9) SHA1(ba78d29d1723783dbd0e8c754d2422caad5ab367) )	// c,d

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "ic6k.bin",   0x0000, 0x0020, CRC(c1e427df) SHA1(9a9980d93dff4b87a940398b18277acaf946eeab) )
ROM_END

ROM_START( mjderngr )
	ROM_REGION( 0xb0000, REGION_CPU1, 0 )
	ROM_LOAD( "2201.1a",    0x00000, 0x08000, CRC(54ec531d) SHA1(c5d9c575f6bdc499bae35123d7ad5bd4869b6ed9) )
	/* bank switched ROMs follow */
	ROM_CONTINUE(           0x10000, 0x08000 )				// 0
	ROM_LOAD( "2202.1b",    0x30000, 0x10000, CRC(edcf97f2) SHA1(8143f41d511fa01bd86faf829eb2c139292d705f) )	// 4,5
	ROM_LOAD( "2203.1d",    0x50000, 0x10000, CRC(a33368c0) SHA1(e216b65d7ed59d7cbf2b5d078799915d707b5291) )	// 8,9
	ROM_LOAD( "2204.1e",    0x70000, 0x20000, CRC(ed5fde4b) SHA1(d55487ae1007d43b71f06ae5c407c75db7054515) )	// c,d,e,f
	ROM_LOAD( "2205.1f",    0x90000, 0x20000, CRC(cfb8075d) SHA1(31f613a1a9b5f4295b552aeeddb760605ce2ac70) )	// 0x10,0x11,0x12,0x13

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "ic3g.bin",   0x000, 0x200, CRC(d43f4c7c) SHA1(117d2e4e8d5bea3e5dc903a4b87bd71786ae009c) )
	ROM_LOAD( "ic4g.bin",   0x200, 0x200, CRC(30cf7831) SHA1(b4593d51c6ceb301279a01a98665e4be8a3c403d) )
ROM_END

/***************************************************************************

Mahjong If (BET type)
(c)1990 Dynax

CPU:    Unknown 64P(Toshiba TLCS-90 series?)
Sound:  AY-3-8910
OSC:    8.000MHz
        18.432MHz


2911.1B   prg.
2902.1C
2903.1D
2904.1E
2905.1F
2906.1G

D29-1.4C  color
D29-2.4D

***************************************************************************/

ROM_START( mjifb )
	ROM_REGION( 0xd0000, REGION_CPU1, 0 )
	ROM_LOAD( "2911.1b",    0x00000, 0x10000, CRC(138a31a1) SHA1(7e77c63a968206b8e61aaa423e19a766e4142554) )
	/* bank switched ROMs follow */
	ROM_RELOAD(             0x10000, 0x08000 )	// bank 0 = 8000-bfff
	ROM_CONTINUE(           0x10000, 0x08000 )
	ROM_LOAD( "2903.1d",    0x30000, 0x20000, CRC(90c44965) SHA1(6904bfa7475f9de921bc2abcfc337b3daf7e0fad) )
	ROM_LOAD( "2906.1g",    0x50000, 0x20000, CRC(ad469345) SHA1(914ea4c77a540467da779ea78c52e66b05c30475) )
	ROM_LOAD( "2904.1e",    0x70000, 0x20000, CRC(2791abfa) SHA1(a8fd1a7e1cf4441b447a4605ad2f1c13775f92da) )
	ROM_LOAD( "2905.1f",    0x90000, 0x20000, CRC(b7a73cf7) SHA1(d93111e6d5f84e331f8198d8c595e3500abed133) )
	ROM_LOAD( "2902.1c",    0xb0000, 0x10000, CRC(0ce02a98) SHA1(69f6bca9af8548038401839047a304a4aa97cfe6) )
	ROM_RELOAD(             0xc0000, 0x10000 )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "d29-2.4d",   0x000, 0x200, CRC(78252f6a) SHA1(1869147bc6b7573c2543bdf6b17d6c3c1debdddb) )
	ROM_LOAD( "d29-1.4c",   0x200, 0x200, CRC(4aaec8cf) SHA1(fbe1c3729d078a422ffe68dfde495fcb9f329cdd) )
ROM_END

/***************************************************************************

    Colour proms are TBP28S42's

***************************************************************************/

ROM_START( janptr96 )
	ROM_REGION( 0x210000, REGION_CPU1, 0 )
	ROM_LOAD( "503x-1.1h", 0x000000, 0x40000, CRC(39914ecd) SHA1(e5796a95a7e3e7b61da63d50fa089be2946ba611) )
	/* bank switched ROMs follow */
	ROM_RELOAD(            0x010000, 0x40000 )
	ROM_RELOAD(            0x050000, 0x40000 )
	ROM_LOAD( "503x-2.1g", 0x090000, 0x80000, CRC(d4b1ed79) SHA1(e1e266339d1d05c0405bfd32b67f215807696c82) )
	ROM_LOAD( "503x-3.1f", 0x110000, 0x80000, CRC(9ba4deb0) SHA1(e9d44a6ed849ff90c0b1f9321cdd62e18c3fd35c) )
	ROM_LOAD( "503x-4.1e", 0x190000, 0x80000, CRC(e266ca0b) SHA1(d84608e7b474061a680510a266842e667bf2eab5) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "ns503b.3h", 0x000, 0x200, CRC(3b2a6b12) SHA1(ebd2929e6acbde989964bfef602b81f2f2fe04eb) )
	ROM_LOAD( "ns503a.3j", 0x200, 0x200, CRC(fe49b2f0) SHA1(a36ca005380cc92dfe473254c26be2cef2ced9b4) )
ROM_END

/***************************************************************************

Mahjong Tensinhai
Dynax, 1995

PCB Layout
----------

Top board

D10010318L1
|----------------------------------------|
|DSW2(1)  DSW4(10)                  DIP16|
|                 |---|                  |
|DSW1(10) DSW3(10)| * |                  |
|                 |---|     PROM2        |
|                                        |
|                           PROM1        |
|                                        |
|                                        |
|                                        |
|                                        |
|                        1001.5E         |
| |-------------|                        |
| |     &       |        1002.4E    |---||
| |-------------|                   | D ||
|12MHz                   1003.3E    | I ||
|                                   | P ||
|BATTERY        32.768kHz           |40 ||
|         CLOCK          6264       |---||
|----------------------------------------|
Notes:
      Most of the chips have their surface scratched off.
      *     - Unknown PLCC44 IC. Possibly Mach110 or similar CPLD
      &     - Unknown SDIP64 IC. Possibly a Toshiba TMP91P640? Clock input 12.000MHz
      CLOCK - Some kind of clock IC, like Oki M6242 or similar
      PROM1 - 82S147 PROM labelled 'D100-1'
      PROM2 - 82S147 PROM labelled 'D100-2'
      DIP16 - Socket for cable that joins to lower board
      DIP40 - Socket for connector that joins to lower board


Bottom board

|--------------------------------------------------------|
|    BATTERY 6116                                        |
|  VOL                                                   |
|                                                        |
|                                              DIP40     |
|                                                        |
|           DSW(8)                              18.432MHz|
|                                                        |
|                                                        |
|M      DIP16                                            |
|A              4164    4164                             |
|H                                                       |
|J              4164    4164                             |
|O                                                       |
|N              4164    4164                             |
|G                                                       |
|2              4164    4164                             |
|8  AY3-8910                                             |
|               4164    4164                             |
|                                                        |
|               4164    4164                             |
|                                                        |
|               4164    4164                             |
|                                                        |
|               4164    4164                             |
|--------------------------------------------------------|
Notes:
      DIP16 - Socket for cable that joins to upper board
      DIP40 - Socket for connector that joins to upper board
      AY3-8910 clock - 1.536 [18.432/12]
      HSync - 15.5kHz
      VSync - 60Hz

***************************************************************************/

ROM_START( mjtensin )
	ROM_REGION( 0x290000, REGION_CPU1, 0 )
	ROM_LOAD( "1001.5e", 0x000000, 0x80000, CRC(960e1fe9) SHA1(11f5164b2c75c0e684e910ee8e09de978bdaff2f) )
	/* bank switched ROMs follow */
	ROM_RELOAD(          0x010000, 0x80000 )
	ROM_RELOAD(          0x090000, 0x80000 )

	ROM_LOAD( "1002.4e", 0x110000, 0x80000, CRC(240eb7af) SHA1(2309e1c251fe55f6e6b97b5db94fa2fe914b88f4) )

	ROM_LOAD( "1003.3e", 0x210000, 0x80000, CRC(876081bf) SHA1(fe962cfa9318a9444123bcaf3406e22fb08e8c4e) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "d100-2.7e",  0x000, 0x200, CRC(6edeed23) SHA1(f4420c473ebbe3df92b0f5b1f0e4d5495fcb9fda) )
	ROM_LOAD( "d100-1.6e",  0x200, 0x200, CRC(88befd59) SHA1(cbcb437f9f6b5e542dc69f5c9e85ccbae47080af) )
ROM_END

/***************************************************************************

Almond Pinky
Dynax, 1988

This game runs on Royal Mahjong hardware.
It appears Royal Mahjong was originally manufactured by Nichibutsu
This PCB says "(C) 1983 Nichibutsu" on it.

Top PCB
-------
D1401128L-0
|------------------------------------------|
|         |---------|                      |
|DIP40    |         |                      |
| Z80A    |   &     |                      |
|         |---------|                      |
|     8MHz                           DSW1  |
|                                          |
|                                          |
|                                          |
|                                          |
|                                    DSW2  |
|                                          |
|                                          |
|                                          |
|DYNAX DYNAX DYNAX DYNAX DYNAX DYNAX       |
| 146   145   144   143   142   141  6116 *|
|------------------------------------------|
Notes:
      Every chip has it's surface scratched
      *     - 4 pin power connector joined to main PCB
      DSWx  - have 8 switches each
      DIP40 - Socket joins to main PCB
      &     - Large Dynax ceramic SIP module (DAC or similar)
      Z80   - clock 4MHz [8/2]
      All ROMs type 27512

Note! On Royal Mahjong-based PCBs where there is a 3 pin RGB connector (3 wires) tied to the main
board and joining to the top board, the color PROM is located on the top daughterboard. Usually that
chip is an 82S123 (32 bytes), 82S129 (256 bytes) or 82S147 (512 bytes). In any case, you can be sure
there is a PROM(s) on the PCB somewhere if the RGB connector cable is present.


Main PCB
--------
RM-1D (C) Nichibutsu 1983
|------------------------------------------------|
|  6116 DIP24  P6    P5    P4    P3     P2    P1 |
|MB3712                                          |
|     BATTERY                                    |
|              DSW(8)                    DIP40   |
|                                                |
|1                                      18.432MHz|
|8    TBP18S030.6K                               |
|W                                               |
|A                                               |
|Y                                               |
|   AY3-8910                                     |
|            4116    4116                        |
|1           4116    4116                        |
|0           4116    4116                        |
|W           4116    4116                        |
|A           4116    4116                        |
|Y           4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|------------------------------------------------|
Notes:
      DIP40 - Socket joins to top PCB
      DIP24 - Unpopulated DIP24 position (no socket)
      TBP18S030.6K - Color PROM (32 bytes)
      P1-P6 - Program ROM sockets (DIP24)

***************************************************************************/

ROM_START( mjapinky )
	ROM_REGION( 0x90000, REGION_CPU1, 0 )
	ROM_LOAD( "141.4d",     0x00000, 0x10000, CRC(0c4fb83a) SHA1(5d467e8fae715ca4acf88f8e9437c7cdf9f876bd) )
	/* bank switched ROMs follow */
	ROM_RELOAD(             0x10000, 0x10000 )
	ROM_LOAD( "142.4e",     0x20000, 0x10000, CRC(129806f0) SHA1(d12d2c5bb0c653f2e4974c47004ada128ac30bea) )
	ROM_LOAD( "143.4f",     0x30000, 0x10000, CRC(3d0bc452) SHA1(ad61eaa892121f90f31a6baf83158a11e6051430) )
	ROM_LOAD( "144.4h",     0x40000, 0x10000, CRC(24509a18) SHA1(ab9daed2cbc72d02c2168a4c93f70ebfe3916ea2) )
	ROM_LOAD( "145.4j",     0x50000, 0x10000, CRC(fea3375a) SHA1(cbb89b72cfba9c0448d152dfdbedb20b9896516e) )
	ROM_LOAD( "146.4k",     0x60000, 0x10000, CRC(be27a9b9) SHA1(f12402182f598391e445245b345f49084a69620a) )

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "18s030n.clr",   0x0000, 0x0020, CRC(5736d0aa) SHA1(298b51340d2697347842cfaa5921f31c7b7f9748) )
ROM_END


/***************************************************************************

Mahjong Cafe Time
(c)1992 Dynax
Modified Royal Mahjong PCB
D6310128L1-1 (Sub PCB)

CPU: Z80
Sound: AY-3-8910

ROMs:
6301.2E      [1fc10e7c]
6302.3E      [02bbdf78]
6303.5E      [0e71eea8]
6304.6E      [53c581d6]

D63-1.7F     [e7410136] MB7124H
D63-2.8F     [af735b42] /

***************************************************************************/

ROM_START( cafetime )
	ROM_REGION( 0x210000, REGION_CPU1, 0 )
	ROM_LOAD( "6301.2e", 0x000000, 0x40000, CRC(1fc10e7c) SHA1(0ed6bfd4cc6fc64bbf55bd3c6bde2d8ba9da2afb) )
	/* bank switched ROMs follow */
	ROM_RELOAD(          0x010000, 0x40000 )
	ROM_RELOAD(          0x050000, 0x40000 )
	ROM_LOAD( "6302.3e", 0x090000, 0x80000, CRC(02bbdf78) SHA1(e1e107541236ed92854fac4e12c9b300dbac9822) )
	ROM_LOAD( "6303.5e", 0x110000, 0x80000, CRC(0e71eea8) SHA1(f95c3b7acee6deabff4aca83b490e255648e2f19) )
	ROM_LOAD( "6304.6e", 0x190000, 0x80000, CRC(53c581d6) SHA1(d9cfda63a8f2e92873f69c673d3efe5c22cfa0de) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "d63-2.8f", 0x000, 0x200, CRC(af735b42) SHA1(deddde3e276d5b9de72e267f65399d80783c6244) )
	ROM_LOAD( "d63-1.7f", 0x200, 0x200, CRC(e7410136) SHA1(54d3aec0d11485d4f419e76f9c4071ab9b817937) )
ROM_END

/***************************************************************************

Mahjong Cafe Doll
Dynax, 1993

This game runs on Royal Mahjong hardware.

Top PCB
-------
D76052208L-2
|-----------------------------------|
|   7601  6264        RTC   BATTERY |
|DIP40                              |
|   7602                  8MHz      |
|                PAL           PAL  |
|   7603           |-----------|    |
|                  |    CPU    |    |
|   DIP32          |-----------|    |
|                                   |
|                                   |
|                                   |
|                                   |
|                                   |
| 82s147.7F                        &|
|*                      DSW3   DSW1 |
| 82S147.9F  %          DSW4   DSW2 |
|-----------------------------------|
Notes:
      Every chip has it's surface scratched, except the PROMs
      *     - Connector joined to main PCB
      &     - Power input connector
      %     - RGB Video output
      DIP32 - Empty DIP32 socket
      DSWx  - have 10 switches each
      DIP40 - Socket joins to main PCB
      CPU   - unknown SDIP64 chip. Possibly TMP90P640 or similar TLCS-90 type CPU


Main PCB
--------
no pcb number
|------------------------------------------------|
|   6116 DIP24 DIP24 DIP24 DIP28 DIP28 DIP28     |
|        DIP24                                   |
|HA1368                                          |
|                                        DIP40   |
|                                                |
|             DSW(8)                    18.432MHz|
|M                                               |
|A                                               |
|H                                               |
|J                                               |
|O  AY3-8910                                     |
|N           4116    4116                        |
|G           4116    4116                        |
|2           4116    4116                        |
|8           4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|------------------------------------------------|
Notes:
      DIP40 - Sockets joins to top PCB
      DIP24/28 - Unpopulated sockets

***************************************************************************/

ROM_START( cafedoll )
	ROM_REGION( 0x190000, REGION_CPU1, 0 )
	ROM_LOAD( "76xx.tmp90841", 0x00000, 0x02000, NO_DUMP )
	ROM_LOAD( "7601", 0x000000, 0x80000, CRC(20c80ad9) SHA1(e45edd101c6e26c0fa3c3f15f4a4152a853e41bd) )
	/* bank switched ROMs follow */
	ROM_RELOAD(       0x010000, 0x80000 )
	ROM_LOAD( "7602", 0x090000, 0x80000, CRC(f472960c) SHA1(cc2feb4374ba94035101114c73e1690cfeac9b91) )
	ROM_LOAD( "7603", 0x110000, 0x80000, CRC(c4293019) SHA1(afd717844e9e681ada14e80cd10dce0ed60d4259) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "d76-2_82s147.9f", 0x000, 0x200, CRC(9c1d0512) SHA1(3ca82d4271badc890701ecc76b97e80b16509b50) )
	ROM_LOAD( "d76-1_82s147.7f", 0x200, 0x200, CRC(9a75349c) SHA1(2071132267aafd8facf1d7841093d9a45c30a8d3) )
ROM_END

/***************************************************************************

Mahjong Vegas
Dynax, 199?

This game runs on Royal Mahjong hardware.

Top PCB
-------
D5011308L1
|-----------------------------------|
| DIP32 DIP32 5003  5002 DIP32 5001A|
|DIP40                          62XX|
|                         32.768kHz |
|                             RTC   |
|                   |-----------|   |
|                   |    CPU    |   |
|                   |-----------|   |
|DSW4   DSW2                        |
|    DSW3   DSW1           PAL      |
|                                   |
|                                   |
|                                   |
|    D50-2                          |
|*      D50-1            8MHz       |
|  %         &             BATTERY  |
|-----------------------------------|
Notes:
      Every chip has it's surface scratched
      *     - Cable connector joined to main PCB (to original PROM socket on main board)
      %     - RGB Video output
      &     - +12V input to top PCB
      DIP32 - Empty DIP32 socket
      DSWx  - have 10 switches each
      DIP40 - Socket joins to main PCB
      CPU   - unknown SDIP64 chip. Possibly TMP90P640 or similar TLCS-90 type CPU
              Pins 9-14 have been broken off and removed!
      62XX  - 6264 or 62256 SRAM
      D50-* - 82S147 color PROMs


Main PCB
--------
FRM-00 (with Falcon Logo.... PCB is made by Falcon)
|------------------------------------------------|
|  6116 DIP24 ROM6  ROM5  ROM4  ROM3  ROM2  ROM1 |
|HA1368                                          |
|                                                |
|     VOL                                DIP40   |
|                                                |
|             DSW(8)                    18.432MHz|
|M                                               |
|A                                               |
|H                                               |
|J                                               |
|O  AY3-8910                                     |
|N           4116    4116                        |
|G           4116    4116                        |
|2           4116    4116                        |
|8           4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|------------------------------------------------|
Notes:
      DIP40 - Socket joins to top PCB
      DIP24 - Unpopulated socket
      AY-3-8910 - clock 1.536MHz (18.432/12]
      ROM* - Unpopulated DIP24 sockets

***************************************************************************/

ROM_START( mjvegas )
	ROM_REGION( 0xd0000, REGION_CPU1, 0 )
	ROM_LOAD( "50xx.tmp90841", 0x00000, 0x02000, NO_DUMP )
	ROM_LOAD( "5001a.1b", 0x00000, 0x20000, CRC(91859a47) SHA1(3c452405bf28f5e7302eaccdf472e91b64629a67) )
	/* bank switched ROMs follow */
	ROM_RELOAD(           0x10000, 0x20000 )
	ROM_LOAD( "5002.1d",  0x30000, 0x80000, CRC(016c0a32) SHA1(5c5fdd631eacb36a0ee7dba9e070c2d3d3d8fd5b) )
	ROM_LOAD( "5003.1e",  0xb0000, 0x20000, CRC(5323cc85) SHA1(58b75ba560f05a0568024f52ee89f54713219452) )

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "d50-2_82s147.4h", 0x000, 0x200, CRC(3c960ea2) SHA1(65e05e3f129e9e6fcb14b7d44a75a76919c54d52) )
	ROM_LOAD( "d50-1_82s147.4g", 0x200, 0x200, CRC(50c0d0ec) SHA1(222899456cd2e15391d8d0f771bbd5e5333d6ba3) )
ROM_END

/***************************************************************************

Mahjong Shinkirou Deja Vu (+ some roms from Jan Oh (Toapan) !?)

This game runs on Royal Mahjong hardware.

Top PCB
-------
D210301BL2
|-----------------------------------|
| DIP32 DIP32 2104  2103  2102 2101 |
|DIP40                          6116|
|                                   |
|DSW3                               |
|  |-----------|                    |
|  |    CPU    |                    |
|  |-----------|                    |
|DSW4                               |
|             8MHz                  |
|                         DSW2  DSW1|
|                                   |
|                                   |
|                                   |
|*                                  |
|  %                                |
|-----------------------------------|
Notes:
      Every chip has it's surface scratched
      *     - Connector joined to main PCB
      %     - RGB Video output
      DIP32 - Empty DIP32 socket
      DSWx  - have 8 switches each
      DIP40 - Socket joins to main PCB
      CPU   - unknown SDIP64 chip. Possibly TMP90P640 or similar TLCS-90 type CPU


Main PCB
--------
FRM-00
|------------------------------------------------|
|  6116 DIP24 ROM6  ROM5  ROM4  ROM3  ROM2  ROM1 |
|HA1368                                          |
|                                                |
|                                        DIP40   |
|                                                |
|             DSW(8)                    18.432MHz|
|M                                               |
|A                                               |
|H                                               |
|J                                               |
|O  AY3-8910                                     |
|N           4116    4116                        |
|G           4116    4116                        |
|2           4116    4116                        |
|8           4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|            4116    4116                        |
|------------------------------------------------|
Notes:
      DIP40 - Sockets joins to top PCB
      DIP24 - Unpopulated socket

***************************************************************************/

ROM_START( mjdejavu )
	ROM_REGION( 0xd0000, REGION_CPU1, 0 )
	ROM_LOAD( "2101.1b", 0x00000, 0x10000, CRC(b0426ea7) SHA1(ac39cbf5d78acdaa4b01d948917965c3aa2761b8) )
	/* bank switched ROMs follow */
	ROM_RELOAD(          0x10000, 0x08000 )
	ROM_CONTINUE(        0x10000, 0x08000 )	// 0
	// unused
	ROM_LOAD( "2103.1d", 0x30000, 0x20000, CRC(ed5fde4b) SHA1(d55487ae1007d43b71f06ae5c407c75db7054515) )	// 8
	// unused
	ROM_LOAD( "2104.1e", 0x70000, 0x20000, CRC(cfb8075d) SHA1(31f613a1a9b5f4295b552aeeddb760605ce2ac70) )	// 18
	// unused
	ROM_LOAD( "2102.1c", 0xb0000, 0x20000, CRC(f461e422) SHA1(c3505feb32650fdd5c0d7f30faed69b65d94937a) )	// 28

	ROM_REGION( 0x400, REGION_PROMS, 0 )
	ROM_LOAD( "82s147.4d", 0x000, 0x200, CRC(d43f4c7c) SHA1(117d2e4e8d5bea3e5dc903a4b87bd71786ae009c) )
	ROM_LOAD( "82s147.4c", 0x200, 0x200, CRC(30cf7831) SHA1(b4593d51c6ceb301279a01a98665e4be8a3c403d) )
ROM_END

// Incomplete romset (missing rom7 at $6000): "Jan Oh" by Toaplan, on royalmah hardware (try pc=64f).
ROM_START( janoh )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "rom1.p1",  0x0000, 0x1000, CRC(8fc19963) SHA1(309e941c059a97b117090fd9dd69a00031aa6109) )	// "1984 JAN OH"
	ROM_LOAD( "rom2.p12", 0x1000, 0x1000, CRC(e1141ae1) SHA1(38f7a71b367a607bb20a5cbe62e7c87c96c6997c) )
	ROM_LOAD( "rom3.p2",  0x2000, 0x1000, CRC(66e6d2f4) SHA1(d7e00e5bfee60daf844c46d36b1f4860fba70759) )	// "JANOH TOAPLAN 84"
	ROM_LOAD( "rom4.p3",  0x3000, 0x1000, CRC(9186f02c) SHA1(b7dc2d6c19e67dd3f841cbb56df9589e3e6941f7) )
	ROM_LOAD( "rom5.p4",  0x4000, 0x1000, CRC(f3c478a8) SHA1(02a8504457cbcdd3e67e7f5ba60fb789f198a51d) )
	ROM_LOAD( "rom6.p5",  0x5000, 0x1000, CRC(92687327) SHA1(4fafba5881dca2a147616d94dd055eba6aa3c653) )
	ROM_LOAD( "rom7.p6",  0x6000, 0x1000, NO_DUMP )

	ROM_REGION( 0x20, REGION_PROMS, 0 )
	ROM_LOAD( "janho.color", 0x00, 0x20, NO_DUMP )
ROM_END


static DRIVER_INIT( ippatsu )	{	memory_set_bankptr(1, memory_region(REGION_CPU1) + 0x8000 );	}

static DRIVER_INIT( janptr96 )
{
	generic_nvram_size = 0x1000 * 9;
	generic_nvram = auto_malloc( generic_nvram_size );

	memory_set_bankptr(3,generic_nvram);
}


GAME( 1981,  royalmj,  0,        royalmah, royalmah, 0,        ROT0, "Nichibutsu",                 "Royal Mahjong (Japan, v1.13)",          0 )
GAME( 1981?, openmj,   royalmj,  royalmah, royalmah, 0,        ROT0, "Sapporo Mechanic",           "Open Mahjong [BET] (Japan)",            0 )
GAME( 1982,  royalmah, royalmj,  royalmah, royalmah, 0,        ROT0, "bootleg",                    "Royal Mahjong (Falcon bootleg, v1.01)", 0 )
GAME( 1983,  janyoup2, royalmj,  ippatsu,  janyoup2, 0,        ROT0, "Cosmo Denshi",               "Janyou Part II (ver 7.03, July 1 1983)",0 )
GAME( 1984,  janoh,    0,        royalmah, royalmah, 0,        ROT0, "Toaplan",                    "Jan Oh",                                GAME_NOT_WORKING )
GAME( 1986,  dondenmj, 0,        dondenmj, majs101b, 0,        ROT0, "Dyna Electronics",           "Don Den Mahjong [BET] (Japan)",         0 )
GAME( 1986,  ippatsu,  0,        ippatsu,  ippatsu,  ippatsu,  ROT0, "Public Software / Paradais", "Ippatsu Gyakuten [BET] (Japan)",        0 )
GAME( 1986,  suzume,   0,        suzume,   suzume,   0,        ROT0, "Dyna Electronics",           "Watashiha Suzumechan (Japan)",          0 )
GAME( 1987,  mjdiplob, 0,        mjdiplob, mjdiplob, 0,        ROT0, "Dynax",                      "Mahjong Diplomat [BET] (Japan)",        0 )
GAME( 1987,  tontonb,  0,        tontonb,  tontonb,  0,        ROT0, "Dynax",                      "Tonton [BET] (Japan)",                  0 )
GAME( 1988,  majs101b, 0,        majs101b, majs101b, 0,        ROT0, "Dynax",                      "Mahjong Studio 101 [BET] (Japan)",      0 )
GAME( 1988,  mjapinky, 0,        mjapinky, mjapinky, 0,        ROT0, "Dynax",                      "Almond Pinky [BET] (Japan)",            0 )
GAME( 1989,  mjdejavu, 0,        mjdejavu, mjdejavu, 0,        ROT0, "Dynax",                      "Mahjong Shinkirou Deja Vu (Japan)",     0 )
GAME( 1989,  mjderngr, 0,        mjderngr, majs101b, 0,        ROT0, "Dynax",                      "Mahjong Derringer (Japan)",             0 )
GAME( 1990,  mjifb,    0,        mjifb,    mjifb,    0,        ROT0, "Dynax",                      "Mahjong If...? [BET]",                  0 )
GAME( 1991,  mjvegas,  0,        mjifb,    mjifb,    0,        ROT0, "Dynax",                      "Mahjong Vegas (Japan)",                 GAME_NOT_WORKING )
GAME( 1992,  cafetime, 0,        cafetime, cafetime, 0,        ROT0, "Dynax",                      "Mahjong Cafe Time",                     0 )
GAME( 1993,  cafedoll, 0,        mjifb,    mjifb,    0,        ROT0, "Dynax",                      "Mahjong Cafe Doll (Japan)",             GAME_NOT_WORKING )
GAME( 1995,  mjtensin, 0,        mjtensin, mjtensin, 0,        ROT0, "Dynax",                      "Mahjong Tensinhai (Japan)",             GAME_NOT_WORKING )
GAME( 1996,  janptr96, 0,        janptr96, janptr96, janptr96, ROT0, "Dynax",                      "Janputer '96 (Japan)",                  0 )
