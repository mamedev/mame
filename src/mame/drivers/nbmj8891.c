// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    nbmj8891 - Nichibutsu Mahjong games for years 1988-1991

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -

******************************************************************************/
/******************************************************************************

Notes:

- mjcamerb and mmcamera is the medal version of mjcamera, however the
   two don't run on the same hardware. mjcamera is in nbmj8688.c.

- In mjfocus(Medal Type), sometimes CPU's hands are forced out from the screen.
  This is correct behaviour.

TODO:

- Telmajan cannot set to JAMMA type. I don't know why.

- Real machine has ROMs for protection, but I don't know how to access the ROM,
  so I'm doing something that works but is probably wrong.
  The interesting thing about that ROM is that it comes from other, older games,
  so it isn't needed, it's just verified for protection.
  mjfocusm does a different check from the others. All the other games read the
  protection ROM through the sound ROMinterface, mjfocusm reads it from somewhere
  else.

- Some games display "GFXROM BANK OVER!!" or "GFXROM ADDRESS OVER!!"
  in Debug build.

- Screen flipping is not perfect.

- taiwanmb needs MCU emulation (color lookup table data output etc.)

- Missing VCR tape / emulation for hnxmasev,hnageman

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "sound/3812intf.h"
#include "includes/nbmj8891.h"
#include "machine/nvram.h"


DRIVER_INIT_MEMBER(nbmj8891_state,gionbana)
{
	UINT8 *prot = memregion("protection")->base();
	int i;

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0x5ece checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */
	for (i = 0;i < 0x20000;i++)
	{
		prot[i] = BITSWAP8(prot[i],2,7,3,5,0,6,4,1);
	}
}

DRIVER_INIT_MEMBER(nbmj8891_state,omotesnd)
{
#if 0
	UINT8 *prot = memregion("protection")->base();
	int i;

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0x5ece checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */
	for (i = 0;i < 0x20000;i++)
	{
		prot[i] = BITSWAP8(prot[i],2,7,3,5,0,6,4,1);
	}
#endif

#if 1
	UINT8 *ROM = memregion("maincpu")->base();

	// Protection ROM check skip
	ROM[0x0106] = 0x00;
	ROM[0x0107] = 0x00;
	ROM[0x0108] = 0x00;

	// Program ROM check skip
	ROM[0x0233] = 0x00;
	ROM[0x0234] = 0x00;
	// Voice ROM check skip
//  ROM[0x0269] = 0x00;
//  ROM[0x026a] = 0x00;
#endif
}

DRIVER_INIT_MEMBER(nbmj8891_state,telmahjn)
{
	UINT8 *prot = memregion("protection")->base();
	int i;

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0x7354 checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */
	for (i = 0;i < 0x20000;i++)
	{
		prot[i] = BITSWAP8(prot[i + 0x20000],7,0,4,1,5,2,6,3);
	}
}

DRIVER_INIT_MEMBER(nbmj8891_state,mgmen89)
{
	UINT8 *prot = memregion("protection")->base();
	int i;

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0x4b98 checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */
	for (i = 0;i < 0x20000;i++)
	{
		prot[i] = BITSWAP8(prot[i],5,6,1,0,7,3,2,4);
	}
}

DRIVER_INIT_MEMBER(nbmj8891_state,mjfocus)
{
	UINT8 *prot = memregion("protection")->base();
	UINT8 *ram = memregion("maincpu")->base() + 0xf800;
	int i;

	/* need to clear RAM otherwise it doesn't boot... */
	for (i = 0; i < 0x800; i++) ram[i] = 0x00;

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0x7354 checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */
	for (i = 0;i < 0x20000;i++)
	{
		prot[i] = BITSWAP8(prot[i + 0x20000],7,0,4,1,5,2,6,3);
	}
}

DRIVER_INIT_MEMBER(nbmj8891_state,mjfocusm)
{
#if 1
	UINT8 *ROM = memregion("maincpu")->base();

	// Protection ROM check skip
	ROM[0x014e] = 0x00;
	ROM[0x014f] = 0x00;
	ROM[0x0150] = 0x00;
#endif
}

DRIVER_INIT_MEMBER(nbmj8891_state,scandal)
{
	UINT8 *ROM = memregion("maincpu")->base();
	int i;

	for (i = 0xf800; i < 0x10000; i++) ROM[i] = 0x00;
}

DRIVER_INIT_MEMBER(nbmj8891_state,mjnanpas)
{
	/* they forgot to enable the protection check in this game... */
#if 0
	UINT8 *prot = memregion("protection")->base();
	int i;

	memregion("maincpu")->base()[0x003d] = 0x01;    // force the protection check to be executed

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0xfe1a checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */
	for (i = 0;i < 0x20000;i++)
	{
		prot[i] = BITSWAP8(prot[i + 0x20000],0,5,2,3,6,7,1,4);
	}
#endif
}

DRIVER_INIT_MEMBER(nbmj8891_state,pairsnb)
{
	UINT8 *prot = memregion("protection")->base();
	int i;

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0x4b98 checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */
	for (i = 0;i < 0x20000;i++)
	{
		prot[i] = BITSWAP8(prot[i],5,6,1,0,7,3,2,4);
	}
}

DRIVER_INIT_MEMBER(nbmj8891_state,pairsten)
{
	UINT8 *prot = memregion("protection")->base();
	int i;

	/* this is one possible way to rearrange the protection ROM data to get the
	   expected 0x8374 checksum. It's probably completely wrong! But since the
	   game doesn't do anything else with that ROM, this is more than enough. I
	   could just fill this are with fake data, the only thing that matters is
	   the checksum. */
	for (i = 0;i < 0x20000;i++)
	{
		prot[i] = BITSWAP8(prot[i + 0x20000],5,6,0,4,3,7,1,2);
	}
}

static ADDRESS_MAP_START( gionbana_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf00f) AM_READWRITE(clut_r, clut_w)
	AM_RANGE(0xf400, 0xf5ff) AM_READWRITE(palette_type1_r, palette_type1_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( mgion_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf1ff) AM_READWRITE(palette_type1_r, palette_type1_w)
	AM_RANGE(0xf400, 0xf40f) AM_READWRITE(clut_r, clut_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( omotesnd_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf400, 0xf5ff) AM_READWRITE(palette_type1_r, palette_type1_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( hanamomo_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf1ff) AM_READWRITE(palette_type1_r, palette_type1_w)
	AM_RANGE(0xf400, 0xf40f) AM_READWRITE(clut_r, clut_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( scandalm_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf400, 0xf5ff) AM_READWRITE(palette_type1_r, palette_type1_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( club90s_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf80f) AM_READWRITE(clut_r, clut_w)
	AM_RANGE(0xfc00, 0xfdff) AM_READWRITE(palette_type1_r, palette_type1_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lovehous_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf00f) AM_READWRITE(clut_r, clut_w)
	AM_RANGE(0xf400, 0xf5ff) AM_READWRITE(palette_type2_r, palette_type2_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( maiko_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf1ff) AM_READWRITE(palette_type2_r, palette_type2_w)
	AM_RANGE(0xf400, 0xf40f) AM_READWRITE(clut_r, clut_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hnxmasev_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf200, 0xf3ff) AM_READWRITE(palette_type2_r, palette_type2_w)
	AM_RANGE(0xf700, 0xf70f) AM_READWRITE(clut_r, clut_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hnageman_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf00f) AM_READWRITE(clut_r, clut_w)
	AM_RANGE(0xf400, 0xf5ff) AM_READWRITE(palette_type2_r, palette_type2_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mmaiko_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf1ff) AM_READWRITE(palette_type2_r, palette_type2_w)
	AM_RANGE(0xf400, 0xf40f) AM_READWRITE(clut_r, clut_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( hanaoji_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf200, 0xf3ff) AM_READWRITE(palette_type2_r, palette_type2_w)
	AM_RANGE(0xf700, 0xf70f) AM_READWRITE(clut_r, clut_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

READ8_MEMBER(nbmj8891_state::taiwanmb_unk_r)
{
	return 0x00;                                                    // MCU or 1413M3 STATUS?
}

static ADDRESS_MAP_START( taiwanmb_map, AS_PROGRAM, 8, nbmj8891_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xec00, 0xedff) AM_READWRITE(palette_type3_r, palette_type3_w)
	AM_RANGE(0xf800, 0xfeff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xff00, 0xff1f) AM_NOP                                 // ?
	AM_RANGE(0xff20, 0xff20) AM_READ(taiwanmb_unk_r)                // MCU or 1413M3 STATUS? (return != 0x00 then loop)
	AM_RANGE(0xff20, 0xff20) AM_WRITE(taiwanmb_mcu_w)      // MCU PARAMETER?
	AM_RANGE(0xff21, 0xff2f) AM_READNOP                             // ?
	AM_RANGE(0xff21, 0xff21) AM_WRITENOP                            // blitter parameter set end (write 0x01 only)
	AM_RANGE(0xff22, 0xff27) AM_WRITE(taiwanmb_blitter_w)  // blitter parameter
	AM_RANGE(0xff28, 0xff28) AM_WRITE(romsel_w)            // gfx rombank select
	AM_RANGE(0xff29, 0xff29) AM_WRITE(taiwanmb_gfxflag_w)  // screen flip flag?
	AM_RANGE(0xff2a, 0xff2a) AM_WRITENOP                            // not used?
	AM_RANGE(0xff2b, 0xff2b) AM_WRITE(clutsel_w)           // color look up table select
	AM_RANGE(0xff2c, 0xff2c) AM_WRITENOP                            // blitter parameter set start (write 0xff only)
	AM_RANGE(0xff2d, 0xff2d) AM_WRITENOP                            // not used?
	AM_RANGE(0xff2e, 0xff2e) AM_WRITENOP                            // not used?
	AM_RANGE(0xff2f, 0xff2f) AM_WRITENOP                            // not used?
	AM_RANGE(0xff30, 0xffff) AM_RAM                                 // RAM?
ADDRESS_MAP_END


static ADDRESS_MAP_START( gionbana_io_map, AS_IO, 8, nbmj8891_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_DEVREAD("nb1413m3", nb1413m3_device, sndrom_r)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("nb1413m3", nb1413m3_device, nmi_clock_w)
	AM_RANGE(0x20, 0x27) AM_WRITE(blitter_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(clutsel_w)
	AM_RANGE(0x60, 0x60) AM_WRITE(romsel_w)
	AM_RANGE(0x70, 0x70) AM_WRITE(scrolly_w)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("fmsnd", ym3812_device, write)
	AM_RANGE(0x90, 0x90) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport0_r)
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport1_r, inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport2_r, sndrombank1_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport3_r) //AM_WRITENOP
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0xe0, 0xe0) AM_WRITE(vramsel_w)
	AM_RANGE(0xf0, 0xf0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, dipsw1_r, outcoin_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mgion_io_map, AS_IO, 8, nbmj8891_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_DEVREAD("nb1413m3", nb1413m3_device, sndrom_r)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("nb1413m3", nb1413m3_device, nmi_clock_w)
	AM_RANGE(0x30, 0x37) AM_WRITE(blitter_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(clutsel_w)
	AM_RANGE(0x60, 0x60) AM_WRITE(romsel_w)
	AM_RANGE(0x70, 0x70) AM_WRITE(scrolly_w)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("fmsnd", ym3812_device, write)
	AM_RANGE(0x90, 0x90) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport0_r)
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport1_r, inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport2_r, sndrombank1_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport3_r) //AM_WRITENOP
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0xe0, 0xe0) AM_WRITE(vramsel_w)
	AM_RANGE(0xf0, 0xf0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, dipsw1_r, outcoin_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( omotesnd_io_map, AS_IO, 8, nbmj8891_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_DEVREAD("nb1413m3", nb1413m3_device, sndrom_r)
	AM_RANGE(0x00, 0x07) AM_WRITE(blitter_w)
	AM_RANGE(0x30, 0x30) AM_WRITE(vramsel_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(romsel_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(clutsel_w)
	AM_RANGE(0x40, 0x4f) AM_WRITE(clut_w)
//  AM_RANGE(0x50, 0x50) AM_DEVWRITE("nb1413m3", nb1413m3_device, nmi_clock_w)
	AM_RANGE(0x70, 0x70) AM_WRITE(scrolly_w)
	AM_RANGE(0x81, 0x81) AM_DEVREAD("fmsnd", ay8910_device, data_r)
	AM_RANGE(0x82, 0x83) AM_DEVWRITE("fmsnd", ay8910_device, data_address_w)
	AM_RANGE(0x90, 0x90) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport0_r) AM_WRITENOP
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport1_r, inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport2_r, sndrombank1_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport3_r) AM_WRITENOP
	AM_RANGE(0xd0, 0xdf) AM_READ(clut_r)
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0xf0, 0xf0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, dipsw1_r, outcoin_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hanamomo_io_map, AS_IO, 8, nbmj8891_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_DEVREAD("nb1413m3", nb1413m3_device, sndrom_r)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("nb1413m3", nb1413m3_device, nmi_clock_w)
	AM_RANGE(0x30, 0x37) AM_WRITE(blitter_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(clutsel_w)
	AM_RANGE(0x60, 0x60) AM_WRITE(romsel_w)
	AM_RANGE(0x70, 0x70) AM_WRITE(scrolly_w)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("fmsnd", ym3812_device, write)
	AM_RANGE(0x90, 0x90) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport0_r)
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport1_r, inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport2_r, sndrombank1_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport3_r) //AM_WRITENOP
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE("dac", dac_device, write_unsigned8)
//  AM_RANGE(0xe0, 0xe0) AM_WRITENOP
	AM_RANGE(0xf0, 0xf0) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw1_r) //AM_WRITENOP
	AM_RANGE(0xf1, 0xf1) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( msjiken_io_map, AS_IO, 8, nbmj8891_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_DEVREAD("nb1413m3", nb1413m3_device, sndrom_r)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("nb1413m3", nb1413m3_device, nmi_clock_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(clutsel_w)
	AM_RANGE(0x50, 0x57) AM_WRITE(blitter_w)
	AM_RANGE(0x60, 0x60) AM_WRITE(romsel_w)
	AM_RANGE(0x70, 0x70) AM_WRITE(scrolly_w)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("fmsnd", ym3812_device, write)
	AM_RANGE(0x90, 0x90) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport0_r)
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport1_r, inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport2_r, sndrombank1_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport3_r) //AM_WRITENOP
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE("dac", dac_device, write_unsigned8)
//  AM_RANGE(0xe0, 0xe0) AM_WRITENOP
	AM_RANGE(0xf0, 0xf0) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw1_r) //AM_WRITENOP
	AM_RANGE(0xf1, 0xf1) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( scandal_io_map, AS_IO, 8, nbmj8891_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_DEVREAD("nb1413m3", nb1413m3_device, sndrom_r)
	AM_RANGE(0x40, 0x4f) AM_WRITE(clut_w)
	AM_RANGE(0x00, 0x07) AM_WRITE(blitter_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(romsel_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(clutsel_w)
	AM_RANGE(0x50, 0x50) AM_WRITE(scrolly_w)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("fmsnd", ym3812_device, write)
	AM_RANGE(0x90, 0x90) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport0_r)
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport1_r, inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport2_r, sndrombank1_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport3_r, nmi_clock_w)
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE("dac", dac_device, write_unsigned8)
//  AM_RANGE(0xe0, 0xe0) AM_WRITENOP
	AM_RANGE(0xf0, 0xf0) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw1_r) //AM_WRITENOP
	AM_RANGE(0xf1, 0xf1) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( scandalm_io_map, AS_IO, 8, nbmj8891_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_DEVREAD("nb1413m3", nb1413m3_device, sndrom_r)
	AM_RANGE(0x00, 0x07) AM_WRITE(blitter_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(romsel_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(clutsel_w)
	AM_RANGE(0x40, 0x4f) AM_WRITE(clut_w)
	AM_RANGE(0x50, 0x50) AM_WRITE(scrolly_w)
	AM_RANGE(0x81, 0x81) AM_DEVREAD("fmsnd", ay8910_device, data_r)
	AM_RANGE(0x82, 0x83) AM_DEVWRITE("fmsnd", ay8910_device, data_address_w)
	AM_RANGE(0x90, 0x90) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport0_r)
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport1_r, inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport2_r, sndrombank1_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport3_r, nmi_clock_w)
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE("dac", dac_device, write_unsigned8)
//  AM_RANGE(0xe0, 0xe0) AM_WRITENOP
	AM_RANGE(0xf0, 0xf0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, dipsw1_r, outcoin_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bananadr_io_map, AS_IO, 8, nbmj8891_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_DEVREAD("nb1413m3", nb1413m3_device, sndrom_r)
	AM_RANGE(0x00, 0x07) AM_WRITE(blitter_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(romsel_w)
	AM_RANGE(0x20, 0x20) AM_WRITE(clutsel_w)
	AM_RANGE(0x30, 0x30) AM_WRITE(vramsel_w)
	AM_RANGE(0x40, 0x4f) AM_WRITE(clut_w)
	AM_RANGE(0x50, 0x50) AM_WRITE(scrolly_w)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("fmsnd", ym3812_device, write)
	AM_RANGE(0x90, 0x90) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport0_r)
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport1_r, inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport2_r, sndrombank1_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport3_r, nmi_clock_w)
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE("dac", dac_device, write_unsigned8)
//  AM_RANGE(0xe0, 0xe0) AM_WRITENOP
	AM_RANGE(0xf0, 0xf0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, dipsw1_r, outcoin_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lovehous_io_map, AS_IO, 8, nbmj8891_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_DEVREAD("nb1413m3", nb1413m3_device, sndrom_r)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("nb1413m3", nb1413m3_device, nmi_clock_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(clutsel_w)
	AM_RANGE(0x50, 0x57) AM_WRITE(blitter_w)
	AM_RANGE(0x60, 0x60) AM_WRITE(romsel_w)
	AM_RANGE(0x70, 0x70) AM_WRITE(scrolly_w)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("fmsnd", ym3812_device, write)
	AM_RANGE(0x90, 0x90) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport0_r) //AM_WRITENOP
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport1_r, inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport2_r, sndrombank1_w)
	AM_RANGE(0xc0, 0xc0) AM_READ_PORT("PORT0-2")
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0xe0, 0xe0) AM_WRITE(vramsel_w)
	AM_RANGE(0xf0, 0xf0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, dipsw1_r, outcoin_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( maiko_io_map, AS_IO, 8, nbmj8891_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_DEVREAD("nb1413m3", nb1413m3_device, sndrom_r)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("nb1413m3", nb1413m3_device, nmi_clock_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(clutsel_w)
	AM_RANGE(0x50, 0x57) AM_WRITE(blitter_w)
	AM_RANGE(0x60, 0x60) AM_WRITE(romsel_w)
	AM_RANGE(0x70, 0x70) AM_WRITE(scrolly_w)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("fmsnd", ym3812_device, write)
	AM_RANGE(0x90, 0x90) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport0_r)
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport1_r, inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport2_r, sndrombank1_w)
	AM_RANGE(0xc0, 0xc0) AM_READ_PORT("PORT0-2") //AM_WRITENOP
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0xe0, 0xe0) AM_WRITE(vramsel_w)
	AM_RANGE(0xf0, 0xf0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, dipsw1_r, outcoin_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( taiwanmb_io_map, AS_IO, 8, nbmj8891_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x7f) AM_DEVREAD("nb1413m3", nb1413m3_device, sndrom_r)
	AM_RANGE(0x81, 0x81) AM_DEVREAD("fmsnd", ay8910_device, data_r)
	AM_RANGE(0x82, 0x83) AM_DEVWRITE("fmsnd", ay8910_device, data_address_w)
	AM_RANGE(0x90, 0x90) AM_DEVREAD("nb1413m3", nb1413m3_device, inputport0_r) //AM_WRITENOP   // ?
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport1_r, inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, inputport2_r, sndrombank1_w)
//  AM_RANGE(0xc0, 0xc0) AM_WRITENOP                    // ?
//  AM_RANGE(0xd0, 0xd0) AM_READ(ff_r)  // irq ack? watchdog?
	AM_RANGE(0xd0, 0xd0) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0xe0, 0xe0) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw3_h_r) AM_WRITE(taiwanmb_gfxdraw_w)  // blitter draw start
	AM_RANGE(0xe1, 0xe1) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw3_l_r)
	AM_RANGE(0xf0, 0xf0) AM_DEVREADWRITE("nb1413m3", nb1413m3_device, dipsw2_r, outcoin_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREAD("nb1413m3", nb1413m3_device, dipsw1_r)
ADDRESS_MAP_END

CUSTOM_INPUT_MEMBER( nbmj8891_state::nb1413m3_busyflag_r )
{
	return m_nb1413m3->m_busyflag & 0x01;
}

/* 2008-08 FP:
 * In ALL games (but pastelg, hyhoo & hyhoo2) nb1413m3_outcoin_flag is read at inputport0.
 * However, a few games (lovehous, maiko, mmaiko, hanaoji and the ones using inputport3_r below)
 * read nb1413m3_outcoin_flag also at inputport3! Is this the correct behaviour for these games
 * or should they only check the flag at inputport3? */
CUSTOM_INPUT_MEMBER( nbmj8891_state::nb1413m3_outcoin_flag_r )
{
	return m_nb1413m3->m_outcoin_flag & 0x01;
}

static INPUT_PORTS_START( hanamomo )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Character Display Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	/* This DIPSW is fake. Type A is common, Type B is rare */
	PORT_START("FONTTYPE")
	PORT_CONFNAME( 0x01, 0x00, "Font Type" )
	PORT_CONFSETTING(    0x00, "Type-A" )
	PORT_CONFSETTING(    0x01, "Type-B" )
	PORT_BIT(            0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( mjcamerb )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Character Display Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( mmcamera )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( msjiken )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Control Type" )
	PORT_DIPSETTING(    0x80, "ROYAL" )
	PORT_DIPSETTING(    0x00, "JAMMA" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )

	PORT_START("JAMMA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JAMMA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( telmahjn )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Control Type" )
	PORT_DIPSETTING(    0x80, "ROYAL" )
	PORT_DIPSETTING(    0x00, "JAMMA" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )

	PORT_START("JAMMA1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JAMMA2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( gionbana )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x00, "Oyaken" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Ino-Shika-Chou" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Tsukimi de Ippai" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Hanami de Ippai" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Shichi-Go-San" )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbhf1_ctrl )
INPUT_PORTS_END

static INPUT_PORTS_START( mgion )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8 (Coin out type)" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbhf1_ctrl )
INPUT_PORTS_END

static INPUT_PORTS_START( omotesnd )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Character Display Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Nudity" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Double Up" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbhf1_ctrl )
INPUT_PORTS_END

static INPUT_PORTS_START( abunai )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Character Display Test2" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x00, "Oyaken" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Ino-Shika-Chou" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Tsukimi de Ippai" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Hanami de Ippai" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Shichi-Go-San" )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbhf1_ctrl )
INPUT_PORTS_END

static INPUT_PORTS_START( mgmen89 )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( mjfocus )

	// I don't have manual for this game.

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, "Character Display Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( mjfocusm )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out" )
	PORT_DIPSETTING(    0x07, "95% (Easy)" )
	PORT_DIPSETTING(    0x06, "90%" )
	PORT_DIPSETTING(    0x05, "85%" )
	PORT_DIPSETTING(    0x04, "80%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x00, "60% (Hard)" )
	PORT_DIPNAME( 0x08, 0x00, "Last Chance" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "W.Bet" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Show summary" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x06, 0x06, "Bet Min" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x00, "Bet Max" )
	PORT_DIPSETTING(    0x18, "8" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x20, 0x20, "Bet1 Only" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Score Pool" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Change Rate" )
	PORT_DIPSETTING(    0x80, "A" )
	PORT_DIPSETTING(    0x00, "B" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( peepshow )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, "Character Display Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( scandal )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "Character Display Test" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( scandalm )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out" )
	PORT_DIPSETTING(    0x07, "90% (Easy)" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x00, "55% (Hard)" )
	PORT_DIPNAME( 0x08, 0x00, "Last Chance" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "W.Bet" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x06, 0x06, "Bet Min" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x00, "Bet Max" )
	PORT_DIPSETTING(    0x18, "8" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x20, 0x20, "Bet1 Only" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Score Pool" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( mjnanpas )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Character Display Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( mjnanpaa )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( bananadr )

	// I don't have manual for this game.

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Character Display Test" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x80, 0x00, "Score Pool" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( club90s )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( lovehous )

	// I don't have manual for this game.

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out" )
	PORT_DIPSETTING(    0x07, "90% (Easy)" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x00, "55% (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x00, "Last Chance" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bonus awarded on" )
	PORT_DIPSETTING(    0x20, "[over BAIMAN]" )
	PORT_DIPSETTING(    0x00, "[over MANGAN]" )
	PORT_DIPNAME( 0x40, 0x40, "Variability of payout rate" )
	PORT_DIPSETTING(    0x40, "[big]" )
	PORT_DIPSETTING(    0x00, "[small]" )
	PORT_DIPNAME( 0x80, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Graphic ROM Test" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "W.Bet" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Bet Min" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x60, 0x60, "Bet Max" )
	PORT_DIPSETTING(    0x60, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Score Pool" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )

	PORT_START("PORT0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_outcoin_flag_r, NULL)    // OUT COIN
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mladyhtr )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Game Sounds" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Game Mode" )
	PORT_DIPSETTING(    0x40, "Beginner" )
	PORT_DIPSETTING(    0x00, "Expert" )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( chinmoku )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Game Sounds" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( maiko )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Oyaken" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Local Rule" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Graphic ROM Test" )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbhf2_ctrl )

	PORT_START("PORT0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_outcoin_flag_r, NULL)    // OUT COIN
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )         //
INPUT_PORTS_END

static INPUT_PORTS_START( mmaiko )

	// I don't have manual for this game.

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out" )
	PORT_DIPSETTING(    0x07, "90% (Easy)" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x00, "55% (Hard)" )
	PORT_DIPNAME( 0x18, 0x18, "Bet Min" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x60, 0x60, "Bet Max" )
	PORT_DIPSETTING(    0x60, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Graphic ROM Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbhf2_ctrl )

	PORT_START("PORT0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_outcoin_flag_r, NULL)    // OUT COIN
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( hanaoji )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x40, 0x40, "Graphic ROM Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbhf2_ctrl )

	PORT_START("PORT0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_outcoin_flag_r, NULL)    // OUT COIN
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )         //
INPUT_PORTS_END

static INPUT_PORTS_START( pairsnb )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )         // Hearts : 10 - Time : 60/60
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )       // Hearts :  7 - Time : 44/60
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )         // Hearts :  5 - Time : 32/60
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )      // Hearts :  3 - Time : 24/60
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x00, "Demo Music" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Game Music" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Character Display Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )          // not in "test mode"

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( taiwanmb )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out" )
	PORT_DIPSETTING(    0x07, "95% (Easy)" )
	PORT_DIPSETTING(    0x06, "90%" )
	PORT_DIPSETTING(    0x05, "85%" )
	PORT_DIPSETTING(    0x04, "80%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x00, "60% (Hard)" )
	PORT_DIPNAME( 0x18, 0x18, "Bet Min" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x60, 0x00, "Bet Max" )
	PORT_DIPSETTING(    0x60, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Drive" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x00, "Last Chance" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, "Number of last chance" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x08, 0x00, "W.Bet" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "BONUS ?_?l???? BET MIN" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, "?? cut" )
	PORT_DIPSETTING(    0x03, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, "16 - ?H??" )
	PORT_DIPSETTING(    0x01, "11 - 15??" )
	PORT_DIPSETTING(    0x00, "8 - 10??" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Credit Max" )
	PORT_DIPSETTING(    0x30, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, "3000" )
	PORT_DIPSETTING(    0x10, "5000" )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPNAME( 0x40, 0x00, "Cancel Hand" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 3-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, nbmj8891_state, nb1413m3_busyflag_r, NULL)    // DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         // COIN OUT
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )       // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2

	PORT_INCLUDE( nbmjcontrols )

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )

	PORT_MODIFY("KEY1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_P )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON )

	PORT_MODIFY("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_NAME("P1 Mahjong Reach(?)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_Q )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_O )

	PORT_MODIFY("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_NAME("P1 Mahjong Small(?)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_NAME("P1 Mahjong Big(?)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_N )

	PORT_MODIFY("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_PON )

	PORT_MODIFY("KEY5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)

	PORT_MODIFY("KEY6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_P ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)

	PORT_MODIFY("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_NAME("P2 Mahjong Reach(?)") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_Q ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_O ) PORT_PLAYER(2)

	PORT_MODIFY("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_NAME("P2 Mahjong Small(?)") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_NAME("P2 Mahjong Big(?)") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)

	PORT_MODIFY("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
INPUT_PORTS_END


static MACHINE_CONFIG_START( gionbana, nbmj8891_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 20000000/4)    /* 5.00 MHz ? */
	MCFG_CPU_PROGRAM_MAP(gionbana_map)
	MCFG_CPU_IO_MAP(gionbana_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nbmj8891_state, irq0_line_hold)

	MCFG_NB1413M3_ADD("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_GIONBANA )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 8, 248-1)
	MCFG_SCREEN_UPDATE_DRIVER(nbmj8891_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("fmsnd", YM3812, 2500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mgion, gionbana )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mgion_map)
	MCFG_CPU_IO_MAP(mgion_io_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_MGION )

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( omotesnd, gionbana )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(omotesnd_map)
	MCFG_CPU_IO_MAP(omotesnd_io_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_OMOTESND )

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* sound hardware */
	MCFG_SOUND_REPLACE("fmsnd", AY8910, 1250000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSWA"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSWB"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( abunai, gionbana )

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_ABUNAI )
MACHINE_CONFIG_END

/* NBMJDRV2 */
static MACHINE_CONFIG_DERIVED( mjcamerb, gionbana )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(hanamomo_io_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_MJCAMERB )

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 16, 240-1)
	MCFG_VIDEO_START_OVERRIDE(nbmj8891_state,_1layer)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mmcamera, gionbana )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(hanamomo_io_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_MMCAMERA )

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 16, 240-1)
	MCFG_VIDEO_START_OVERRIDE(nbmj8891_state,_1layer)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hanamomo, gionbana )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hanamomo_map)
	MCFG_CPU_IO_MAP(hanamomo_io_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_HANAMOMO )

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 16, 240-1)
	MCFG_VIDEO_START_OVERRIDE(nbmj8891_state,_1layer)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( msjiken, hanamomo )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(gionbana_map)
	MCFG_CPU_IO_MAP(msjiken_io_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_MSJIKEN )
MACHINE_CONFIG_END

/* NBMJDRV3 */
static MACHINE_CONFIG_DERIVED( telmahjn, gionbana )

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_TELMAHJN )

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(nbmj8891_state,_1layer)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mgmen89, telmahjn )

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_MGMEN89 )

MACHINE_CONFIG_END

/* NBMJDRV4 */
static MACHINE_CONFIG_DERIVED( mjfocus, gionbana )

	/* basic machine hardware */

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_MJFOCUS )

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(nbmj8891_state,_1layer)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pairsnb, gionbana )

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_PAIRSNB )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pairsten, gionbana )

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_PAIRSTEN )
MACHINE_CONFIG_END

/* NBMJDRV5 */
static MACHINE_CONFIG_DERIVED( mjnanpas, gionbana )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(club90s_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_MJNANPAS )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( maiko, mjnanpas )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(maiko_map)
	MCFG_CPU_IO_MAP(maiko_io_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_MAIKO )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mmaiko, maiko )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mmaiko_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_MMAIKO )

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( lovehous, mjnanpas )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(lovehous_map)
	MCFG_CPU_IO_MAP(lovehous_io_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_LOVEHOUS )

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hanaoji, maiko )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hanaoji_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_HANAOJI )

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hnxmasev, maiko )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hnxmasev_map)
	MCFG_CPU_IO_MAP(maiko_io_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( hnageman, maiko )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(hnageman_map)
	MCFG_CPU_IO_MAP(maiko_io_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( scandal, hanamomo )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(scandalm_map)
	MCFG_CPU_IO_MAP(scandal_io_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_SCANDAL )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bananadr, mjnanpas )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(scandalm_map)
	MCFG_CPU_IO_MAP(bananadr_io_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_BANANADR )

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( club90s, mjnanpas )

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_CLUB90S )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mladyhtr, mjnanpas )

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_MLADYHTR )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( chinmoku, mjnanpas )

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_CHINMOKU )
MACHINE_CONFIG_END

/* NBMJDRV6 */
static MACHINE_CONFIG_DERIVED( mjfocusm, gionbana )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(scandalm_map)
	MCFG_CPU_IO_MAP(scandalm_io_map)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_MJFOCUSM )

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 16, 240-1)
	MCFG_VIDEO_START_OVERRIDE(nbmj8891_state,_1layer)

	/* sound hardware */
	MCFG_SOUND_REPLACE("fmsnd", AY8910, 1250000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSWA"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSWB"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( scandalm, mjfocusm )

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_SCANDALM )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( taiwanmb, gionbana )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(taiwanmb_map)
	MCFG_CPU_IO_MAP(taiwanmb_io_map)
//  MCFG_CPU_VBLANK_INT_DRIVER("screen", nbmj8891_state, irq0_line_hold)

	MCFG_DEVICE_MODIFY("nb1413m3")
	MCFG_NB1413M3_TYPE( NB1413M3_TAIWANMB )

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 16, 240-1)
	MCFG_VIDEO_START_OVERRIDE(nbmj8891_state,_1layer)

	/* sound hardware */
	MCFG_SOUND_REPLACE("fmsnd", AY8910, 1250000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSWA"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSWB"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_CONFIG_END


ROM_START( gionbana )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "gion_03.bin", 0x00000, 0x10000, CRC(615e993b) SHA1(6efda8d1f0d5be6418a73dd86b898bb518de3f8b) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "gion_02.bin", 0x00000, 0x10000, CRC(c392eacc) SHA1(0f9da8ebaeb4468218123e4c5b8ceee08695ce63) )
	ROM_LOAD( "gion_01.bin", 0x10000, 0x10000, CRC(c253eff7) SHA1(ed0e7e83726c82547bb4f2d0aabdadae9bcc68bf) )

	ROM_REGION( 0x0c0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "gion_04.bin", 0x000000, 0x10000, CRC(0a1398d2) SHA1(f03d272a8b3fe10a77630632a60ae5832d75e016) )
	ROM_LOAD( "gion_05.bin", 0x010000, 0x10000, CRC(75b2c2e3) SHA1(2e6720d9910dd1c0e4696e489a33ac1833e0d9a0) )
	ROM_LOAD( "gion_06.bin", 0x020000, 0x10000, CRC(cb743f16) SHA1(72abf5658a3e0b49ba5adab372dff0970558c651) )
	ROM_LOAD( "gion_07.bin", 0x030000, 0x10000, CRC(5574f6d2) SHA1(426e6f4f10fd6b7273ab9444f4d4b09057a351a3) )
	ROM_LOAD( "gion_08.bin", 0x040000, 0x10000, CRC(b230ad99) SHA1(f8628eb13be5a986016988a63b703e42e231f580) )
	ROM_LOAD( "gion_09.bin", 0x050000, 0x10000, CRC(cc7d54a8) SHA1(7d070e3725b383be4bd89efb8e0a59f520803afb) )
	ROM_LOAD( "gion_10.bin", 0x060000, 0x10000, CRC(22dd6d9f) SHA1(dff99b44ab08f99546c489d89396614d62dae87e) )
	ROM_LOAD( "gion_11.bin", 0x070000, 0x10000, CRC(f0e81c0b) SHA1(a2fc84a22df3e4073842258fdf425200a8a64a73) )
	ROM_LOAD( "gion_12.bin", 0x080000, 0x10000, CRC(d4e7d308) SHA1(c5ef85e1168da83213c596d5c1615497d5144317) )
	ROM_LOAD( "gion_13.bin", 0x090000, 0x10000, CRC(ff38a134) SHA1(039a8fe32492f8f117f3e987a9a3da3e34b261a4) )
	ROM_LOAD( "gion_14.bin", 0x0a0000, 0x10000, CRC(a4e8b6a0) SHA1(55289f136a08a4b6b25f87d35e12c4ed4a4790e4) )
	ROM_LOAD( "gion_15.bin", 0x0b0000, 0x10000, CRC(d36445e4) SHA1(1922f7327bfe0389fdefd85312e605955c5ccd10) )

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	ROM_LOAD( "gion_m1.bin", 0x00000, 0x40000, CRC(f730ea47) SHA1(f969fa85a91a337ba3fc89e9c458ef116088075e) )   // same as housemnq/2i.bin gfx data
ROM_END

ROM_START( mgion )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "3.3h",   0x00000, 0x10000, CRC(ec8f5b5f) SHA1(895ea15a1d8fe88d94932273d1df2e535b5d1d58) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "2.2k",   0x00000, 0x10000, CRC(c392eacc) SHA1(0f9da8ebaeb4468218123e4c5b8ceee08695ce63) )    // gionbana/gion_02.bin
	ROM_LOAD( "1.2h",   0x10000, 0x10000, CRC(c253eff7) SHA1(ed0e7e83726c82547bb4f2d0aabdadae9bcc68bf) )    // gionbana/gion_01.bin

	ROM_REGION( 0x0d0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.10p",  0x000000, 0x10000, CRC(0a1398d2) SHA1(f03d272a8b3fe10a77630632a60ae5832d75e016) )   // gionbana/gion_04.bin
	ROM_LOAD( "5.10n",  0x010000, 0x10000, CRC(75b2c2e3) SHA1(2e6720d9910dd1c0e4696e489a33ac1833e0d9a0) )   // gionbana/gion_05.bin
	ROM_LOAD( "6.10m",  0x020000, 0x10000, CRC(cb743f16) SHA1(72abf5658a3e0b49ba5adab372dff0970558c651) )   // gionbana/gion_06.bin
	ROM_LOAD( "7.10l",  0x030000, 0x10000, CRC(5574f6d2) SHA1(426e6f4f10fd6b7273ab9444f4d4b09057a351a3) )   // gionbana/gion_07.bin
	ROM_LOAD( "8.10k",  0x040000, 0x10000, CRC(b230ad99) SHA1(f8628eb13be5a986016988a63b703e42e231f580) )   // gionbana/gion_08.bin
	ROM_LOAD( "9.10h",  0x050000, 0x10000, CRC(cc7d54a8) SHA1(7d070e3725b383be4bd89efb8e0a59f520803afb) )   // gionbana/gion_09.bin
	ROM_LOAD( "10.10f", 0x060000, 0x10000, CRC(22dd6d9f) SHA1(dff99b44ab08f99546c489d89396614d62dae87e) )   // gionbana/gion_10.bin
	ROM_LOAD( "11.10e", 0x070000, 0x10000, CRC(f0e81c0b) SHA1(a2fc84a22df3e4073842258fdf425200a8a64a73) )   // gionbana/gion_11.bin
	ROM_LOAD( "12.10d", 0x080000, 0x10000, CRC(d4e7d308) SHA1(c5ef85e1168da83213c596d5c1615497d5144317) )   // gionbana/gion_12.bin
	ROM_LOAD( "13.10c", 0x090000, 0x10000, CRC(ff38a134) SHA1(039a8fe32492f8f117f3e987a9a3da3e34b261a4) )   // gionbana/gion_13.bin
	ROM_LOAD( "14.9p",  0x0a0000, 0x10000, CRC(a4e8b6a0) SHA1(55289f136a08a4b6b25f87d35e12c4ed4a4790e4) )   // gionbana/gion_14.bin
	ROM_LOAD( "15.9n",  0x0b0000, 0x10000, CRC(d36445e4) SHA1(1922f7327bfe0389fdefd85312e605955c5ccd10) )   // gionbana/gion_15.bin
	ROM_LOAD( "16.9m" , 0x0c0000, 0x10000, CRC(dd833801) SHA1(541309854834a4578ece0d9683bb7440d7a6208d) )

	ROM_REGION( 0x40000, "protection", ROMREGION_ERASE00 ) /* protection data */
	// not used
ROM_END

ROM_START( omotesnd )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "10.4h", 0x00000, 0x10000, CRC(8b9856f6) SHA1(f2687ec47e2006af97e1119d9504eb505d3f9e42) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "9.4j",  0x00000, 0x10000, CRC(5b55faa7) SHA1(8f55dcd756d93f89fb713d030eae4543b69b6a9d) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "1.11p", 0x000000, 0x10000, CRC(0a1398d2) SHA1(f03d272a8b3fe10a77630632a60ae5832d75e016) )    // gionbana/gion_04.bin
	ROM_LOAD( "2.11n", 0x010000, 0x10000, CRC(75b2c2e3) SHA1(2e6720d9910dd1c0e4696e489a33ac1833e0d9a0) )    // gionbana/gion_05.bin
	ROM_LOAD( "3.11m", 0x020000, 0x10000, CRC(877d16ac) SHA1(9d9b663f2f4fab8f36b77aef7d148654f5320e96) )
	ROM_LOAD( "4.11k", 0x030000, 0x10000, CRC(22efc825) SHA1(0c7469d4025bcc35e4dbcc5704edd3a5e086d35a) )
	ROM_LOAD( "5.11j", 0x040000, 0x10000, CRC(7c03473b) SHA1(c22766479c287d62bb9d8b0e2d4de7156f4b9f59) )
	ROM_LOAD( "6.11h", 0x050000, 0x10000, CRC(9a0d3742) SHA1(2ec95b2c012f93e5961c677d088ef9b9698be6f1) )
	ROM_LOAD( "7.11e", 0x060000, 0x10000, CRC(f0adeea1) SHA1(6b9893baca74cd0bd01ddadbd62030def8205655) )
	ROM_LOAD( "8.11d", 0x070000, 0x10000, CRC(f0e81c0b) SHA1(a2fc84a22df3e4073842258fdf425200a8a64a73) )    // gionbana/gion_11.bin

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	ROM_LOAD( "ic6n.bin", 0x00000, 0x40000, CRC(da46163e) SHA1(c6e5f59fe813915f94d81ff28526614c943b7082) )  // same as orangec/ic2.bin gfx data
ROM_END

ROM_START( abunai )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.3h",   0x00000, 0x10000, CRC(8ed2119f) SHA1(e77ad936657dc733a2ab5ed69e5ec387cb7e8b23) )

	ROM_REGION( 0x10000, "voice", ROMREGION_ERASE00 ) /* voice */
	// not used

	ROM_REGION( 0x0e0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "2.8c",   0x00000, 0x10000, CRC(35c9c449) SHA1(5628eeb256991e1efd9478180141218594b9052d) )
	ROM_LOAD( "3.8d",   0x10000, 0x10000, CRC(1e3e2036) SHA1(fc84839d6b3a6e52197b0ec365fb6c9c8ebc85d3) )
	ROM_LOAD( "4.8e",   0x20000, 0x10000, CRC(893d5a5a) SHA1(6a64a7fcecdec046e9fcdab495926582a8dabd66) )
	ROM_LOAD( "5.8f",   0x30000, 0x10000, CRC(140449fe) SHA1(8510ecb4c28577a773259acc3c56e517ab4cddf2) )
	ROM_LOAD( "6.8h",   0x40000, 0x10000, CRC(05f3f698) SHA1(1820f38855e10e2ef9c4b5230bc1cb93ed59c72a) )
	ROM_LOAD( "7.8k",   0x50000, 0x10000, CRC(4594d180) SHA1(a42bacf021b8546093ed3041bd386e9f498b8259) )
	ROM_LOAD( "8.8l",   0x60000, 0x10000, CRC(ca5030fb) SHA1(dcaa4a2e18a355bc386b4ef395effe686ac9b70e) )
	ROM_LOAD( "9.8m",   0x70000, 0x10000, CRC(4d742758) SHA1(112aed03ec34ed300a31b4300b297723f0326a27) )
	ROM_LOAD( "10.8n",  0x80000, 0x10000, CRC(b9bc881e) SHA1(3394ef8286ca257224778cfd77e04430d198de8c) )
	ROM_LOAD( "11.8p",  0x90000, 0x10000, CRC(e5c05c0b) SHA1(e542577001e22b090b6d5d51f1a5936f7ac2f703) )
	ROM_LOAD( "12.10c", 0xa0000, 0x10000, CRC(bde1a97b) SHA1(3bbd97b53fe79e8cc02c2ade0032625a458e0ad4) )
	ROM_LOAD( "13.10d", 0xb0000, 0x10000, CRC(b2a3b31e) SHA1(3d0e8438bd1a4168e0463476c21b1f4151e7eb9b) )
	ROM_LOAD( "14.10e", 0xc0000, 0x10000, CRC(c26d41ab) SHA1(32d31f6d619bda2014354cdddc79466a74ad9daa) )
	ROM_LOAD( "15.10f", 0xd0000, 0x10000, CRC(0e400f12) SHA1(27f8a147da725b8fde98a8ef49134ad794097b58) )
ROM_END

ROM_START( hanamomo )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "hmog_21.bin", 0x00000, 0x10000, CRC(5b59d413) SHA1(9f7b7fe9f50a88958f8f7d819fb7fb4275f43260) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "hmog_22.bin", 0x00000, 0x10000, CRC(ccc15b78) SHA1(f2ca6e8ad4f44aedbfe328273fa106852b8463f4) )
	ROM_LOAD( "hmog_23.bin", 0x10000, 0x10000, CRC(3b166358) SHA1(50967d3202407f9964224807ac474da7da179c41) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "hmog_01.bin", 0x000000, 0x10000, CRC(52e7bf1f) SHA1(055c4906534bf50c7b387a018932c8056ea1b9ed) )
	ROM_LOAD( "hmog_02.bin", 0x010000, 0x10000, CRC(bfe11acc) SHA1(2ab240d6565f3687023a9495a8bdb721a90195d4) )
	ROM_LOAD( "hmog_03.bin", 0x020000, 0x10000, CRC(3b28db4c) SHA1(fc4f6557ba92d92ae5a9e1a3e121a723c01d61df) )
	ROM_LOAD( "hmog_04.bin", 0x030000, 0x10000, CRC(ab0c088d) SHA1(efdbd1a551edb9171c40b57cd0f83a6724dc89f6) )
	ROM_LOAD( "hmog_05.bin", 0x040000, 0x10000, CRC(e42aa74b) SHA1(d36c00c39ffea520dc9dbf51469951b2c1fabb9e) )
	ROM_LOAD( "hmog_06.bin", 0x050000, 0x10000, CRC(8926bfee) SHA1(7b1fb06570c96f987f7ff9fe1e28975e5428b721) )
	ROM_LOAD( "hmog_07.bin", 0x060000, 0x10000, CRC(2a85e88b) SHA1(a77552dd4949bd2437c67e423c74482958a6932d) )
	ROM_LOAD( "hmog_08.bin", 0x070000, 0x10000, CRC(ae0c59ab) SHA1(0bfdb7af5af0daeba9f25ab940c7506dbd63da79) )
	ROM_LOAD( "hmog_09.bin", 0x080000, 0x10000, CRC(15fc1179) SHA1(c1ed99502c67f92e6af5d4fb096060663def943d) )
	ROM_LOAD( "hmog_10.bin", 0x090000, 0x10000, CRC(e289b7c3) SHA1(1065d9048b842ccb38c320c8333c444fe8074078) )
	ROM_LOAD( "hmog_11.bin", 0x0a0000, 0x10000, CRC(87eb1e10) SHA1(49b2ff66c7fcc9df3066bed32ce49d835e86ea0d) )
	ROM_LOAD( "hmog_12.bin", 0x0b0000, 0x10000, CRC(f1abaffb) SHA1(51bc2adec84625ce6049bd2abacf9b13adb84002) )
	ROM_LOAD( "hmog_13.bin", 0x0c0000, 0x10000, CRC(fa38d953) SHA1(edd640719f9376870c2ece1295879b13f927594d) )
	ROM_LOAD( "hmog_14.bin", 0x0d0000, 0x10000, CRC(3f231850) SHA1(78159686ee109262858df8604dd5bf541cbf2d1b) )
	ROM_LOAD( "hmog_15.bin", 0x0e0000, 0x10000, CRC(42baaf57) SHA1(1c1f2e1291f051e8454cc09b921d6685edd84531) )
	ROM_LOAD( "hmog_16.bin", 0x0f0000, 0x10000, CRC(1daf3342) SHA1(13a7972122aa654f49266244e685bb5defbc79ae) )
	ROM_LOAD( "hmog_17.bin", 0x100000, 0x10000, CRC(f1932dc1) SHA1(39eba6592515f6eef309c352affa2616c5c76f56) )
	ROM_LOAD( "hmog_18.bin", 0x110000, 0x10000, CRC(44062920) SHA1(ec27af882da301a3873aa6c0bfb08152b01e95f5) )
	ROM_LOAD( "hmog_19.bin", 0x120000, 0x10000, CRC(81414383) SHA1(8520dcfb26234544d0318de086d249e12a233e32) )
	ROM_LOAD( "hmog_20.bin", 0x130000, 0x10000, CRC(f3edc9d3) SHA1(9a6ff08cbc1630e40bdb233bab436bfe18eafb23) )
ROM_END

ROM_START( hanamomb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "21.bin",      0x00000, 0x10000, CRC(d75920b9) SHA1(7504c5d1774c8c98513ada881472145e4dd23a98) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "hmog_22.bin", 0x00000, 0x10000, CRC(ccc15b78) SHA1(f2ca6e8ad4f44aedbfe328273fa106852b8463f4) )
	ROM_LOAD( "hmog_23.bin", 0x10000, 0x10000, CRC(3b166358) SHA1(50967d3202407f9964224807ac474da7da179c41) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "hmog_01.bin", 0x000000, 0x10000, CRC(52e7bf1f) SHA1(055c4906534bf50c7b387a018932c8056ea1b9ed) )
	ROM_LOAD( "hmog_02.bin", 0x010000, 0x10000, CRC(bfe11acc) SHA1(2ab240d6565f3687023a9495a8bdb721a90195d4) )
	ROM_LOAD( "hmog_03.bin", 0x020000, 0x10000, CRC(3b28db4c) SHA1(fc4f6557ba92d92ae5a9e1a3e121a723c01d61df) )
	ROM_LOAD( "hmog_04.bin", 0x030000, 0x10000, CRC(ab0c088d) SHA1(efdbd1a551edb9171c40b57cd0f83a6724dc89f6) )
	ROM_LOAD( "hmog_05.bin", 0x040000, 0x10000, CRC(e42aa74b) SHA1(d36c00c39ffea520dc9dbf51469951b2c1fabb9e) )
	ROM_LOAD( "hmog_06.bin", 0x050000, 0x10000, CRC(8926bfee) SHA1(7b1fb06570c96f987f7ff9fe1e28975e5428b721) )
	ROM_LOAD( "hmog_07.bin", 0x060000, 0x10000, CRC(2a85e88b) SHA1(a77552dd4949bd2437c67e423c74482958a6932d) )
	ROM_LOAD( "hmog_08.bin", 0x070000, 0x10000, CRC(ae0c59ab) SHA1(0bfdb7af5af0daeba9f25ab940c7506dbd63da79) )
	ROM_LOAD( "hmog_09.bin", 0x080000, 0x10000, CRC(15fc1179) SHA1(c1ed99502c67f92e6af5d4fb096060663def943d) )
	ROM_LOAD( "hmog_10.bin", 0x090000, 0x10000, CRC(e289b7c3) SHA1(1065d9048b842ccb38c320c8333c444fe8074078) )
	ROM_LOAD( "hmog_11.bin", 0x0a0000, 0x10000, CRC(87eb1e10) SHA1(49b2ff66c7fcc9df3066bed32ce49d835e86ea0d) )
	ROM_LOAD( "hmog_12.bin", 0x0b0000, 0x10000, CRC(f1abaffb) SHA1(51bc2adec84625ce6049bd2abacf9b13adb84002) )
	ROM_LOAD( "hmog_13.bin", 0x0c0000, 0x10000, CRC(fa38d953) SHA1(edd640719f9376870c2ece1295879b13f927594d) )
	ROM_LOAD( "hmog_14.bin", 0x0d0000, 0x10000, CRC(3f231850) SHA1(78159686ee109262858df8604dd5bf541cbf2d1b) )
	ROM_LOAD( "hmog_15.bin", 0x0e0000, 0x10000, CRC(42baaf57) SHA1(1c1f2e1291f051e8454cc09b921d6685edd84531) )
	ROM_LOAD( "hmog_16.bin", 0x0f0000, 0x10000, CRC(1daf3342) SHA1(13a7972122aa654f49266244e685bb5defbc79ae) )
	ROM_LOAD( "hmog_17.bin", 0x100000, 0x10000, CRC(f1932dc1) SHA1(39eba6592515f6eef309c352affa2616c5c76f56) )
	ROM_LOAD( "hmog_18.bin", 0x110000, 0x10000, CRC(44062920) SHA1(ec27af882da301a3873aa6c0bfb08152b01e95f5) )
	ROM_LOAD( "hmog_19.bin", 0x120000, 0x10000, CRC(81414383) SHA1(8520dcfb26234544d0318de086d249e12a233e32) )
	ROM_LOAD( "hmog_20.bin", 0x130000, 0x10000, CRC(f3edc9d3) SHA1(9a6ff08cbc1630e40bdb233bab436bfe18eafb23) )
ROM_END

ROM_START( msjiken )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "msjn_11.bin",  0x00000, 0x10000, CRC(723499ef) SHA1(ae709e992372c00791e50932ba59456d3dcbc84b) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "msjn_12.bin",  0x00000, 0x10000, CRC(810e299e) SHA1(b9997226e624fbf3ad7ee99d7901acbd190f31be) )

	ROM_REGION( 0x110000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "msjn_01.bin",  0x000000, 0x10000, CRC(42dc6211) SHA1(6cfd77277cf128be7cc91d8c4ad7564a98a4c5e5) )
	ROM_LOAD( "msjn_02.bin",  0x010000, 0x10000, CRC(3bc29b14) SHA1(e9e490859a713e97158d9b60dfed9338f0c3f1ce) )
	ROM_LOAD( "msjn_03.bin",  0x020000, 0x10000, CRC(442c838d) SHA1(7f037ec7de6e1677a5bdecfd19981a3ecf5b8a63) )
	ROM_LOAD( "msjn_04.bin",  0x030000, 0x10000, CRC(42aff870) SHA1(d6269999f12da220d0e4a92d66c2dfe011721848) )
	ROM_LOAD( "msjn_05.bin",  0x040000, 0x10000, CRC(50735648) SHA1(9e9a6aee442510fbdad7a7d2f385c4742cc681e6) )
	ROM_LOAD( "msjn_06.bin",  0x050000, 0x10000, CRC(76b72d64) SHA1(4086f108bf8030880d5205314a4420e3d07ab013) )
	ROM_LOAD( "msjn_07.bin",  0x060000, 0x10000, CRC(aabd0c75) SHA1(76d48a2c86805c5e8ba4309e8bcbe5adeec4ac0e) )
	ROM_LOAD( "msjn_08.bin",  0x070000, 0x10000, CRC(c87ef18a) SHA1(a1f18acc394951aed7c4c262d4763284f0737e00) )
	ROM_LOAD( "msjn_10r.bin", 0x080000, 0x10000, CRC(274700d2) SHA1(475f0860524215aefb9ee02760e2cbf89bf5d2f2) )
	ROM_LOAD( "msjn_10.bin",  0x090000, 0x10000, CRC(4c1deff9) SHA1(2ae48e546b885f57f292118a0421b36dd5f8309e) )
	ROM_LOAD( "msjn_04r.bin", 0x0a0000, 0x10000, CRC(cac5a5cf) SHA1(b7d998b161eea0c8a3af20849e16b926058ce32e) )
	ROM_LOAD( "msjn_05r.bin", 0x0b0000, 0x10000, CRC(a2200fb2) SHA1(a732f2d007fd8bb5b7c19912c036e79fe969260a) )
	ROM_LOAD( "msjn_06r.bin", 0x0c0000, 0x10000, CRC(528061b1) SHA1(7538f6f79a8a23435dabeb4f070ad8ab182f8d5c) )
	ROM_LOAD( "msjn_07r.bin", 0x0d0000, 0x10000, CRC(d2d2dae6) SHA1(db3637bf46038c526228a98fc66defc533ca4974) )
	ROM_LOAD( "msjn_08r.bin", 0x0e0000, 0x10000, CRC(dec0e799) SHA1(bb3919d6226ee8e09e1b32248e94927bc1c44773) )
	ROM_LOAD( "msjn_09r.bin", 0x0f0000, 0x10000, CRC(552167d9) SHA1(ad4d9cfed79cc1cccff88656cfe9d5ce14ea746a) )
	ROM_LOAD( "msjn_09.bin",  0x100000, 0x10000, CRC(df62249e) SHA1(974fc848d770ac2537d47b734a7d166b3fb980b2) )
ROM_END

ROM_START( telmahjn )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "telm_03.bin", 0x00000, 0x10000, CRC(851bff09) SHA1(850c0cf58646dfe49df68e607e8461a6e98c2137) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "telm_02.bin", 0x00000, 0x10000, CRC(5b278b68) SHA1(72010d5f39a5d9089fa28418f21e468fef17e516) )
	ROM_LOAD( "telm_01.bin", 0x10000, 0x10000, CRC(06f00282) SHA1(66aa44eac3dced06858a84a7749c045ee9d2bc34) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "telm_04.bin", 0x000000, 0x10000, CRC(54114564) SHA1(d917b9ed3de45236e2a11a52a62da6caff150856) )
	ROM_LOAD( "telm_05.bin", 0x010000, 0x10000, CRC(369b2c83) SHA1(6df47312d01c9ce821b60a78be92c230e7cad262) )
	ROM_LOAD( "telm_06.bin", 0x020000, 0x10000, CRC(790e8016) SHA1(2d5c2dad9602f89306f8df84aa729dafec0a4fc8) )
	ROM_LOAD( "telm_07.bin", 0x030000, 0x10000, CRC(55ee68e8) SHA1(07a9f0bb525406ce571e7c346f4395bef9e49e2a) )
	ROM_LOAD( "telm_08.bin", 0x040000, 0x10000, CRC(f0928fb0) SHA1(d815375cf0661b97215cbf165ffa803afc77afb8) )
	ROM_LOAD( "telm_09.bin", 0x050000, 0x10000, CRC(ecc99d13) SHA1(4ca7a3878ecde11fe7a2cd96b94dd0b8cb9a8fea) )
	ROM_LOAD( "telm_10.bin", 0x060000, 0x10000, CRC(2036f1bd) SHA1(488dbe4bde73cc82ace829802030f1d56a795081) )
	ROM_LOAD( "telm_11.bin", 0x070000, 0x10000, CRC(1cc59a34) SHA1(6efa69b76b80a60efd43ce3e88bf317a3b71e181) )
	ROM_LOAD( "telm_12.bin", 0x080000, 0x10000, CRC(ea719867) SHA1(f3747cd9dbfdfcdaee7260ec0559e4c2bd9704a2) )
	ROM_LOAD( "telm_13.bin", 0x090000, 0x10000, CRC(e23049d2) SHA1(24a5024096f97b2f811e119e0b8890dbba6af975) )
	ROM_LOAD( "telm_14.bin", 0x0a0000, 0x10000, CRC(61e773c0) SHA1(4fde215a7c1485158ab7b7d86d394a90621b7bb9) )
	ROM_LOAD( "telm_15.bin", 0x0b0000, 0x10000, CRC(c062cf30) SHA1(e18347298e6c07b6ac6b9a8c5ed4d055ea38fd89) )
	ROM_LOAD( "telm_16.bin", 0x0c0000, 0x10000, CRC(ceb37abd) SHA1(5c582e2475973a2c925591ce4b2d06633ca54171) )
	ROM_LOAD( "telm_17.bin", 0x0d0000, 0x10000, CRC(5e0cab0c) SHA1(9a586f4730acf8b9620a8365e174719690720ef5) )
	ROM_LOAD( "telm_18.bin", 0x0e0000, 0x10000, CRC(8ca01f4e) SHA1(5fba3af68d0a95d5a30866f9689867ea3758b235) )
	ROM_LOAD( "telm_19.bin", 0x0f0000, 0x10000, CRC(07362f98) SHA1(21b8cfb776a5a6359d0059b296c7d7154c814981) )

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	ROM_LOAD( "telm_m1.bin", 0x00000, 0x40000, CRC(2199e3e9) SHA1(965af4a29db4ff909dbeeebab1b828eb4f23f57e) )   // same as housemnq/1i.bin gfx data
ROM_END

ROM_START( mgmen89 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "mg89_03.bin", 0x00000, 0x10000, CRC(1ac5cd84) SHA1(15cdfb95b586bd037c9584808911c6f38ed5eace) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "mg89_02.bin", 0x00000, 0x10000, CRC(1ca17bda) SHA1(61022ed38fa666a3dafefb30558fefc0d38836ad) )
	ROM_LOAD( "mg89_01.bin", 0x10000, 0x10000, CRC(9a8c1ac5) SHA1(a2c5666c3d1a77a0a30852474e2eb788a1bdc05b) )

	ROM_REGION( 0x0e0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mg89_04.bin", 0x000000, 0x10000, CRC(4c7d3afb) SHA1(057fdf4a0aac3bb8cea4f92a04b7e05d0a21a634) )
	ROM_LOAD( "mg89_05.bin", 0x010000, 0x10000, CRC(a0b9e4b7) SHA1(4aa29e15488db4423945147191cab11a6739782d) )
	ROM_LOAD( "mg89_06.bin", 0x020000, 0x10000, CRC(7adb3527) SHA1(02ebeaf953a9f7224e806a61083f36b16bb2f29b) )
	ROM_LOAD( "mg89_07.bin", 0x030000, 0x10000, CRC(22ea0472) SHA1(36abed9d811c9d24deabe43b50688f4164357561) )
	ROM_LOAD( "mg89_08.bin", 0x040000, 0x10000, CRC(27343e42) SHA1(cfa247dc0d60a652b6a59cf9baa7053a2fb160a2) )
	ROM_LOAD( "mg89_09.bin", 0x050000, 0x10000, CRC(270addf1) SHA1(0a0af0b8a3d819eb9852a57921a45b210b0bd59a) )
	ROM_LOAD( "mg89_10.bin", 0x060000, 0x10000, CRC(4a2e60ab) SHA1(651ab72389b8cc1894cf26099cf4eaa7ec9994b1) )
	ROM_LOAD( "mg89_11.bin", 0x070000, 0x10000, CRC(4e5d563a) SHA1(3d53f6cc27bec13ba5b73976bb994b84510dc996) )
	ROM_LOAD( "mg89_12.bin", 0x080000, 0x10000, CRC(faf72b35) SHA1(6fc6f1575103d122843fd9787def40c928bd334d) )
	ROM_LOAD( "mg89_13.bin", 0x090000, 0x10000, CRC(68521b30) SHA1(6e0af3e5351288ff7f34d8374302d34ed9f3da2a) )
	ROM_LOAD( "mg89_14.bin", 0x0a0000, 0x10000, CRC(3c70f85e) SHA1(8956aa87bded297a843ba11b3af1a845d1cfaaf0) )
	ROM_LOAD( "mg89_15.bin", 0x0b0000, 0x10000, CRC(993e3b4d) SHA1(67015065dd38fc158ed05a8b66c431b9460198a2) )
	ROM_LOAD( "mg89_16.bin", 0x0c0000, 0x10000, CRC(b66c3b87) SHA1(b0964d87e4b9c59357dde550d6671fd9e3750c21) )
	ROM_LOAD( "mg89_17.bin", 0x0d0000, 0x10000, CRC(3bd5c16b) SHA1(7759e4695f9a7ad40eed69dd3bb96daaeef22fd9) )

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	ROM_LOAD( "mg89_m1.bin", 0x00000, 0x40000, CRC(77ba1eaf) SHA1(bde55b4d2938f44fd07ff7d5b5a845f2ea64b4fc) )   // same as housemnq/5i.bin gfx data
ROM_END

ROM_START( mjfocus )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "2_3h",   0x00000, 0x10000, CRC(fd88b3e6) SHA1(3cb47cfaba421d8539268db353735174809d1506) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "1.2k",   0x00000, 0x10000, CRC(e933d3c8) SHA1(d13687ea61d141c0300e73033723ac0c7a322dc0) )

	ROM_REGION( 0x130000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "3_8c",   0x000000, 0x10000, CRC(4177d71f) SHA1(bdec569fe3352397392ff91785fcae353d2d8dcf) )
	ROM_LOAD( "4_8d",   0x010000, 0x10000, CRC(aba5d761) SHA1(a1156ee91cc9ed22272a5d86f78ca7625490e957) )
	ROM_LOAD( "5_8e",   0x020000, 0x10000, CRC(59c9680e) SHA1(739facc8f6e23bb8da0c1e0d31c90a1ac9812d7c) )
	ROM_LOAD( "6_8f",   0x030000, 0x10000, CRC(582cce83) SHA1(4c6611c1c3187270206a80dadd2164affb67eed3) )
	ROM_LOAD( "7_8h",   0x040000, 0x10000, CRC(e83499c1) SHA1(4015006672622d004acb6bb44e9c414481b4a6c8) )
	ROM_LOAD( "8_8j",   0x050000, 0x10000, CRC(cc583392) SHA1(a18e5f45f947451d433ff942f1464da879f318fd) )
	ROM_LOAD( "9_8k",   0x060000, 0x10000, CRC(9f84e9d2) SHA1(bd4d94b95b818ee69cb46ac13098ab0c5ab1ed14) )
	ROM_LOAD( "10_8l",  0x070000, 0x10000, CRC(c57fa2a3) SHA1(9d79080803f146bca1acdd308f56aa2fbe39d3a7) )
	ROM_LOAD( "11_8n",  0x080000, 0x10000, CRC(4bd661b8) SHA1(d8ba23c8a450aa1fbcff65a5d4d28125b3747db2) )
	ROM_LOAD( "12_8p",  0x090000, 0x10000, CRC(7e4aaad1) SHA1(28f3ea320c663ab325106222b949f39fe220dd82) )
	ROM_LOAD( "13_10c", 0x0a0000, 0x10000, CRC(4e3b155d) SHA1(c356422ab3e1ef2f579dbb962ac29bf736d8beec) )
	ROM_LOAD( "14_10d", 0x0b0000, 0x10000, CRC(703431d1) SHA1(63c9155fcc2d71a841fd432969b54123fe72679c) )
	ROM_LOAD( "15_10e", 0x0c0000, 0x10000, CRC(9d97e0f9) SHA1(49988bc963384fdc30b88aa6ea8dcc8ab49ab600) )
	ROM_LOAD( "16_10f", 0x0d0000, 0x10000, CRC(1d31fcb5) SHA1(0db437fdbbf05201e62a41b2a5913eaaa095721c) )
	ROM_LOAD( "17_10h", 0x0e0000, 0x10000, CRC(c0775836) SHA1(8a385aa7203bd12a1fbb59af3f79658dcbc51f98) )
	ROM_LOAD( "18_10j", 0x0f0000, 0x10000, CRC(31ff6ef1) SHA1(5fef3b54beb49440685ffc486a07a5cc9079cbb7) )
	ROM_LOAD( "19_10k", 0x100000, 0x10000, CRC(86d39bb4) SHA1(50329a199b7c4822bb21618bc9854e613d03dc2a) )
	ROM_LOAD( "20_10l", 0x110000, 0x10000, CRC(53f33c46) SHA1(39249f7b37c2162a484ef3e439f1f513ce13a17f) )
	ROM_LOAD( "21_10n", 0x120000, 0x10000, CRC(68c5b271) SHA1(6b387c9e5cb33f2896033cadb91259fcdba1fe2f) )

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	/* this ROM was not dumped, but the program expects the same checksum
	   as peepshow, so it's a safe assumption that it's the same. */
	ROM_LOAD( "mask",   0x00000, 0x40000, CRC(2199e3e9) SHA1(965af4a29db4ff909dbeeebab1b828eb4f23f57e) )    // same as housemnq/1i.bin gfx data
ROM_END

ROM_START( mjfocusm )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "mfcs_02m.bin", 0x00000, 0x10000, CRC(409d4f0b) SHA1(c19196e8315337a075d44f0814630fb820688788) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "1.2k",         0x00000, 0x10000, CRC(e933d3c8) SHA1(d13687ea61d141c0300e73033723ac0c7a322dc0) )

	ROM_REGION( 0x110000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "3.8c",         0x000000, 0x10000, CRC(4c8d6ca9) SHA1(68f0b676ddadf4eb8c073c1723dee84b4fb18de7) )
	ROM_LOAD( "mfcs_04m.bin", 0x010000, 0x10000, CRC(e73d7804) SHA1(2579739540c27d679a34f0499e10124d6d719e74) )
	ROM_LOAD( "5.8e",         0x020000, 0x10000, CRC(f4d7e344) SHA1(690711684d8bec17db11f2ccd48232d1fe865174) )
	ROM_LOAD( "mfcs_06m.bin", 0x030000, 0x10000, CRC(e4d638f6) SHA1(4dea1401017bb28ee635cc0ff30e28ea667b31c8) )
	ROM_LOAD( "mfcs_07m.bin", 0x040000, 0x10000, CRC(45be433a) SHA1(af2677ce82959ffd43e5011ae2b4128ba76d57bb) )
	ROM_LOAD( "mfcs_08m.bin", 0x050000, 0x10000, CRC(a7e1d761) SHA1(1e8879b89c7218050c61c20ca30d1b963839dcb8) )
	ROM_LOAD( "mfcs_09m.bin", 0x060000, 0x10000, CRC(21cbe481) SHA1(7bc4b6283fe897553a12394c0ea671e8502ac338) )
	ROM_LOAD( "mfcs_10m.bin", 0x070000, 0x10000, CRC(5430d20a) SHA1(a48549e53ac99b4bac2c9637c498dd6387bf0131) )
	ROM_LOAD( "11.8n",        0x080000, 0x10000, CRC(c9bdf0a8) SHA1(861c23388ec39c6204df2ae86f26f5aa8a726f86) )
	ROM_LOAD( "12.8p",        0x090000, 0x10000, CRC(777cbe0e) SHA1(a5f0b048d29687bd8bb335ea4a65cf52daa0343f) )
	ROM_LOAD( "mfcs_13m.bin", 0x0a0000, 0x10000, CRC(6bdb28c2) SHA1(ac6594dcd59bf4ee389fe824cf934f97af02a72e) )
	ROM_LOAD( "14.10d",       0x0b0000, 0x10000, CRC(c86da643) SHA1(d76425dcff49a47dacb667ac5feaefec6294089a) )
	ROM_LOAD( "15.10e",       0x0c0000, 0x10000, CRC(cdf4c1e9) SHA1(87566e6900ebb26fda0909b2f18a59b169843c81) )
	ROM_LOAD( "16.10f",       0x0d0000, 0x10000, CRC(65ac5a6d) SHA1(27e288b7767082da3ef3fb6deff4585e54454f7c) )
	ROM_LOAD( "17.10h",       0x0e0000, 0x10000, CRC(383ece66) SHA1(693c5fcc33116e435b7ac2a6e1ed6a73f8903552) )
	ROM_LOAD( "18.10j",       0x0f0000, 0x10000, CRC(b2cc3586) SHA1(e2629303726f8f135bee10c4f72b283123e3c85d) )
	ROM_LOAD( "mfcs_19m.bin", 0x100000, 0x10000, CRC(45c08364) SHA1(ff83c1c4f6a0623691d3a35b14439387918a7108) )

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	ROM_LOAD( "mfcs_m1m.bin", 0x00000, 0x40000, CRC(da46163e) SHA1(c6e5f59fe813915f94d81ff28526614c943b7082) )  // same as orangec/ic2.bin gfx data
ROM_END

ROM_START( peepshow )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "2.3h",   0x00000, 0x10000, CRC(8db1746c) SHA1(2735988352a831537efeb369a52f041c6c2d47b0) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "1.2k",   0x00000, 0x10000, CRC(e933d3c8) SHA1(d13687ea61d141c0300e73033723ac0c7a322dc0) )

	ROM_REGION( 0x110000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "3.8c",   0x000000, 0x10000, CRC(4c8d6ca9) SHA1(68f0b676ddadf4eb8c073c1723dee84b4fb18de7) )
	ROM_LOAD( "4.8d",   0x010000, 0x10000, CRC(9e80455f) SHA1(4169ebeeb32870d01ea68a31daee7644b04ce0f6) )
	ROM_LOAD( "5.8e",   0x020000, 0x10000, CRC(f4d7e344) SHA1(690711684d8bec17db11f2ccd48232d1fe865174) )
	ROM_LOAD( "6.8f",   0x030000, 0x10000, CRC(91dcf9a5) SHA1(b093569b5da1257d663ab47cf3a3b4c2b8a2113d) )
	ROM_LOAD( "7.8h",   0x040000, 0x10000, CRC(dbc58b78) SHA1(8a908822aa88aaaf6a9e8f6b90fb96b34bb2021f) )
	ROM_LOAD( "8.8j",   0x050000, 0x10000, CRC(0ee9d5cb) SHA1(804b929b18247ca0db013d0f2575b44cd775b60c) )
	ROM_LOAD( "9.8k",   0x060000, 0x10000, CRC(bc00bb95) SHA1(de0488741c9b8989e82a76655d7ab06da38f62fe) )
	ROM_LOAD( "10.8l",  0x070000, 0x10000, CRC(77e62065) SHA1(6f1d49f4bacae4b9331d6567d041855d91aecc2a) )
	ROM_LOAD( "11.8n",  0x080000, 0x10000, CRC(c9bdf0a8) SHA1(861c23388ec39c6204df2ae86f26f5aa8a726f86) )
	ROM_LOAD( "12.8p",  0x090000, 0x10000, CRC(777cbe0e) SHA1(a5f0b048d29687bd8bb335ea4a65cf52daa0343f) )
	ROM_LOAD( "13.10c", 0x0a0000, 0x10000, CRC(97a9ad73) SHA1(14dce4c8d664b55c454f8e109068af2813e08b18) )
	ROM_LOAD( "14.10d", 0x0b0000, 0x10000, CRC(c86da643) SHA1(d76425dcff49a47dacb667ac5feaefec6294089a) )
	ROM_LOAD( "15.10e", 0x0c0000, 0x10000, CRC(cdf4c1e9) SHA1(87566e6900ebb26fda0909b2f18a59b169843c81) )
	ROM_LOAD( "16.10f", 0x0d0000, 0x10000, CRC(65ac5a6d) SHA1(27e288b7767082da3ef3fb6deff4585e54454f7c) )
	ROM_LOAD( "17.10h", 0x0e0000, 0x10000, CRC(383ece66) SHA1(693c5fcc33116e435b7ac2a6e1ed6a73f8903552) )
	ROM_LOAD( "18.10j", 0x0f0000, 0x10000, CRC(b2cc3586) SHA1(e2629303726f8f135bee10c4f72b283123e3c85d) )
	ROM_LOAD( "19.10k", 0x100000, 0x10000, CRC(b6b40e4d) SHA1(4d6f641d08f2c9814510fe1d01f66af4f19ca88a) )

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	/* this ROM was not dumped correctly - FIXED BITS (xxxxxxxx11111111).
	   However, what's in there matches the telmahjn one, and the program expects
	   the same checksum, so it's a safe assumption that it's the same. */
	ROM_LOAD( "mask",   0x00000, 0x40000, CRC(2199e3e9) SHA1(965af4a29db4ff909dbeeebab1b828eb4f23f57e) )    // same as housemnq/1i.bin gfx data
ROM_END

ROM_START( scandal )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.3h",   0x00000, 0x10000, CRC(97e73a9c) SHA1(53d2cecb30b146da55674ea6bdde1b687597cf98) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "2.3j",   0x00000, 0x10000, CRC(9a5f7907) SHA1(939e2dd2765a922aaf3c6a104caf459f1478863f) )

	ROM_REGION( 0x0d0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "15.11p", 0x000000, 0x10000, CRC(4677f0d0) SHA1(e2fc7dfdb1e4d85964937a1a0deaa4e7e2ef40db) )
	ROM_LOAD( "14.11n", 0x010000, 0x10000, CRC(f935a681) SHA1(764d69ca149cfcc42676c0a3c2f347f723b22f3f) )
	ROM_LOAD( "13.11m", 0x020000, 0x10000, CRC(80c5109e) SHA1(85f99c76ecc177bca628f307baeaa59dc3ef9bc0) )
	ROM_LOAD( "12.11k", 0x030000, 0x10000, CRC(2a408850) SHA1(c1317804b0523d911542f077628e4802f0767a71) )
	ROM_LOAD( "11.11j", 0x040000, 0x10000, CRC(34f525af) SHA1(595a2569049ba3deb818e1bbe48af435d2ab68da) )
	ROM_LOAD( "10.11f", 0x050000, 0x10000, CRC(12a30207) SHA1(5684a2d6a2760726e8e85244b7aafd934c59a279) )
	ROM_LOAD( "9.11e",  0x060000, 0x10000, CRC(04918709) SHA1(606d87bdebeeaa14aaa1ce643f0919c67bda3c1a) )
	ROM_LOAD( "8.11d",  0x070000, 0x10000, CRC(5d87d1b7) SHA1(04b60dc248d8c09b0407ec3c09351768a73277fc) )
	ROM_LOAD( "7.11c",  0x080000, 0x10000, CRC(d8f3dcbb) SHA1(a79d70722eb7947835a63346c3b954ddb0be7472) )
	ROM_LOAD( "6.11a",  0x090000, 0x10000, CRC(6ea1e009) SHA1(5d60f4adb0228d96b1c721a5457c4e346ecc67b5) )
	ROM_LOAD( "5.10p",  0x0a0000, 0x10000, CRC(60472080) SHA1(4d3f8bc02bc4c9abbe0ce08c3061aa68407ebb03) )
	ROM_LOAD( "4.10n",  0x0b0000, 0x10000, CRC(d9267e88) SHA1(4778e7c10085736c481c67672aecde0cfe0aee62) )
	ROM_LOAD( "3.10m",  0x0c0000, 0x10000, CRC(9e303eda) SHA1(14a988c8df572aa16bc0464bcb9fd627c8b57537) )
ROM_END

ROM_START( scandalm )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "scmm_01.bin", 0x00000, 0x10000, CRC(9811bab6) SHA1(05a0d9e2f038d5bf0588a66f71ac55a7c0386dac) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "2.3j",        0x00000, 0x10000, CRC(9a5f7907) SHA1(939e2dd2765a922aaf3c6a104caf459f1478863f) )

	ROM_REGION( 0x0d0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "15.11p",      0x000000, 0x10000, CRC(4677f0d0) SHA1(e2fc7dfdb1e4d85964937a1a0deaa4e7e2ef40db) )
	ROM_LOAD( "14.11n",      0x010000, 0x10000, CRC(f935a681) SHA1(764d69ca149cfcc42676c0a3c2f347f723b22f3f) )
	ROM_LOAD( "13.11m",      0x020000, 0x10000, CRC(80c5109e) SHA1(85f99c76ecc177bca628f307baeaa59dc3ef9bc0) )
	ROM_LOAD( "12.11k",      0x030000, 0x10000, CRC(2a408850) SHA1(c1317804b0523d911542f077628e4802f0767a71) )
	ROM_LOAD( "11.11j",      0x040000, 0x10000, CRC(34f525af) SHA1(595a2569049ba3deb818e1bbe48af435d2ab68da) )
	ROM_LOAD( "10.11f",      0x050000, 0x10000, CRC(12a30207) SHA1(5684a2d6a2760726e8e85244b7aafd934c59a279) )
	ROM_LOAD( "9.11e",       0x060000, 0x10000, CRC(04918709) SHA1(606d87bdebeeaa14aaa1ce643f0919c67bda3c1a) )
	ROM_LOAD( "8.11d",       0x070000, 0x10000, CRC(5d87d1b7) SHA1(04b60dc248d8c09b0407ec3c09351768a73277fc) )
	ROM_LOAD( "7.11c",       0x080000, 0x10000, CRC(d8f3dcbb) SHA1(a79d70722eb7947835a63346c3b954ddb0be7472) )
	ROM_LOAD( "6.11a",       0x090000, 0x10000, CRC(6ea1e009) SHA1(5d60f4adb0228d96b1c721a5457c4e346ecc67b5) )
	ROM_LOAD( "5.10p",       0x0a0000, 0x10000, CRC(60472080) SHA1(4d3f8bc02bc4c9abbe0ce08c3061aa68407ebb03) )
	ROM_LOAD( "4.10n",       0x0b0000, 0x10000, CRC(d9267e88) SHA1(4778e7c10085736c481c67672aecde0cfe0aee62) )
	ROM_LOAD( "3.10m",       0x0c0000, 0x10000, CRC(9e303eda) SHA1(14a988c8df572aa16bc0464bcb9fd627c8b57537) )
ROM_END

ROM_START( mjnanpas )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "mnst_03.bin", 0x00000, 0x10000, CRC(ece14e07) SHA1(de952a69fb9ecc676a43f5d4f0fd6159420fcc4f) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "mnst_02.bin", 0x00000, 0x10000, CRC(22c7ddce) SHA1(bc7106622592b6d7ccb839e0ce7a1760068209b7) )
	ROM_LOAD( "mnst_01.bin", 0x10000, 0x10000, CRC(13b79c41) SHA1(0e2446e04510f1ec0f0ed8d4f0239d3029341afe) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mnst_04.bin", 0x000000, 0x10000, CRC(7b8fb5f2) SHA1(5a6024d2a42046268cefa565a5f3c2fa5af8d74f) )
	ROM_LOAD( "mnst_05.bin", 0x010000, 0x10000, CRC(6e48b612) SHA1(09704181204ed5b3e24d47c743b6115edf4bd312) )
	ROM_LOAD( "mnst_06.bin", 0x020000, 0x10000, CRC(1ea7db2e) SHA1(6b80b9c31900568c44afd4b4fe225d12ce9071b1) )
	ROM_LOAD( "mnst_07.bin", 0x030000, 0x10000, CRC(2930acbb) SHA1(65753a6d68abb4102d19f41f40f99fd8cb536873) )
	ROM_LOAD( "mnst_08.bin", 0x040000, 0x10000, CRC(cd632b5c) SHA1(4a24d027769ec7e14d3878f4e5490ce949f2fa63) )
	ROM_LOAD( "mnst_09.bin", 0x050000, 0x10000, CRC(77116d9e) SHA1(7224c5d21b582fdb93d80d8b2919d85aa546ffc3) )
	ROM_LOAD( "mnst_10.bin", 0x060000, 0x10000, CRC(5502e478) SHA1(d3c884c33be322f516cea16587e49806505c82c5) )
	ROM_LOAD( "mnst_11.bin", 0x070000, 0x10000, CRC(3f739fb1) SHA1(c2c8fea787adef1eacca1096db7b84111d5a12aa) )
	ROM_LOAD( "mnst_12.bin", 0x080000, 0x10000, CRC(2741f576) SHA1(bcb9ab6965899c9f24a46118518fa19a6626792b) )
	ROM_LOAD( "mnst_13.bin", 0x090000, 0x10000, CRC(10132020) SHA1(0d711407d11281e0b81c54a238cffdd66e3616dc) )
	ROM_LOAD( "mnst_14.bin", 0x0a0000, 0x10000, CRC(03b32fa7) SHA1(fc42f4f96ba256e382b50e0fbcf44aee0dc8ec55) )
	ROM_LOAD( "mnst_15.bin", 0x0b0000, 0x10000, CRC(4bb85dd7) SHA1(c61b8f855203d5fd1072e93401af77bf6fe49faf) )
	ROM_LOAD( "mnst_16.bin", 0x0c0000, 0x10000, CRC(38de91de) SHA1(a19dbefc977fe3a42ca81a7da45f0f31c3737ae8) )
	ROM_LOAD( "mnst_17.bin", 0x0d0000, 0x10000, CRC(23cac7e3) SHA1(ca7aeb8a6aa6d69d81dca52fb199cfa883a20219) )
	ROM_LOAD( "mnst_18.bin", 0x0e0000, 0x10000, CRC(af62af24) SHA1(dd5ee31bfe683878728353254f508dac7b5d9722) )
	ROM_LOAD( "mnst_19.bin", 0x0f0000, 0x10000, CRC(e18dc023) SHA1(67db3fc7e1665f8b64591effc9d740f8c31310ad) )
	ROM_LOAD( "mnst_20.bin", 0x100000, 0x10000, CRC(ca706644) SHA1(8e9cd483be766126b76d3b4b1189591ac922fabc) )
	ROM_LOAD( "mnst_21.bin", 0x110000, 0x10000, CRC(0a609495) SHA1(c8bbe94f3a18198bffc3ca357ab7d25cf7cfd067) )
	ROM_LOAD( "mnst_22.bin", 0x120000, 0x10000, CRC(3468f36f) SHA1(5723b6ba22268b6eca310e97fe19e4b8e4c57ca9) )
	ROM_LOAD( "mnst_23.bin", 0x130000, 0x10000, CRC(8d1a64a6) SHA1(01dd8bf26d166a058fe771cafe4cee14eb5f813c) )

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	/* the protection data is not used at all! They forgot a debug flag set in the
	   code which skips the protection check. */
	ROM_LOAD( "mnst_m1.bin", 0x00000, 0x40000, CRC(77ba1eaf) SHA1(bde55b4d2938f44fd07ff7d5b5a845f2ea64b4fc) )   // same as housemnq/5i.bin gfx data
ROM_END

ROM_START( mjnanpaa )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "mnst_03.old", 0x00000, 0x10000, CRC(a105b2b8) SHA1(3aa9a41fc8a1ffd37f89b660a986f0c8e48d61f8) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "mnst_02.bin", 0x00000, 0x10000, CRC(22c7ddce) SHA1(bc7106622592b6d7ccb839e0ce7a1760068209b7) )
	ROM_LOAD( "mnst_01.bin", 0x10000, 0x10000, CRC(13b79c41) SHA1(0e2446e04510f1ec0f0ed8d4f0239d3029341afe) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mnst_04.bin", 0x000000, 0x10000, CRC(7b8fb5f2) SHA1(5a6024d2a42046268cefa565a5f3c2fa5af8d74f) )
	ROM_LOAD( "mnst_05.bin", 0x010000, 0x10000, CRC(6e48b612) SHA1(09704181204ed5b3e24d47c743b6115edf4bd312) )
	ROM_LOAD( "mnst_06.bin", 0x020000, 0x10000, CRC(1ea7db2e) SHA1(6b80b9c31900568c44afd4b4fe225d12ce9071b1) )
	ROM_LOAD( "mnst_07.bin", 0x030000, 0x10000, CRC(2930acbb) SHA1(65753a6d68abb4102d19f41f40f99fd8cb536873) )
	ROM_LOAD( "mnst_08.bin", 0x040000, 0x10000, CRC(cd632b5c) SHA1(4a24d027769ec7e14d3878f4e5490ce949f2fa63) )
	ROM_LOAD( "mnst_09.bin", 0x050000, 0x10000, CRC(77116d9e) SHA1(7224c5d21b582fdb93d80d8b2919d85aa546ffc3) )
	ROM_LOAD( "mnst_10.bin", 0x060000, 0x10000, CRC(5502e478) SHA1(d3c884c33be322f516cea16587e49806505c82c5) )
	ROM_LOAD( "mnst_11.bin", 0x070000, 0x10000, CRC(3f739fb1) SHA1(c2c8fea787adef1eacca1096db7b84111d5a12aa) )
	ROM_LOAD( "mnst_12.bin", 0x080000, 0x10000, CRC(2741f576) SHA1(bcb9ab6965899c9f24a46118518fa19a6626792b) )
	ROM_LOAD( "mnst_13.bin", 0x090000, 0x10000, CRC(10132020) SHA1(0d711407d11281e0b81c54a238cffdd66e3616dc) )
	ROM_LOAD( "mnst_14.bin", 0x0a0000, 0x10000, CRC(03b32fa7) SHA1(fc42f4f96ba256e382b50e0fbcf44aee0dc8ec55) )
	ROM_LOAD( "mnst_15.bin", 0x0b0000, 0x10000, CRC(4bb85dd7) SHA1(c61b8f855203d5fd1072e93401af77bf6fe49faf) )
	ROM_LOAD( "mnst_16.bin", 0x0c0000, 0x10000, CRC(38de91de) SHA1(a19dbefc977fe3a42ca81a7da45f0f31c3737ae8) )
	ROM_LOAD( "mnst_17.bin", 0x0d0000, 0x10000, CRC(23cac7e3) SHA1(ca7aeb8a6aa6d69d81dca52fb199cfa883a20219) )
	ROM_LOAD( "mnst_18.bin", 0x0e0000, 0x10000, CRC(af62af24) SHA1(dd5ee31bfe683878728353254f508dac7b5d9722) )
	ROM_LOAD( "mnst_19.bin", 0x0f0000, 0x10000, CRC(e18dc023) SHA1(67db3fc7e1665f8b64591effc9d740f8c31310ad) )
	ROM_LOAD( "mnst_20.bin", 0x100000, 0x10000, CRC(ca706644) SHA1(8e9cd483be766126b76d3b4b1189591ac922fabc) )
	ROM_LOAD( "mnst_21.bin", 0x110000, 0x10000, CRC(0a609495) SHA1(c8bbe94f3a18198bffc3ca357ab7d25cf7cfd067) )
	ROM_LOAD( "mnst_22.bin", 0x120000, 0x10000, CRC(3468f36f) SHA1(5723b6ba22268b6eca310e97fe19e4b8e4c57ca9) )
	ROM_LOAD( "mnst_23.bin", 0x130000, 0x10000, CRC(8d1a64a6) SHA1(01dd8bf26d166a058fe771cafe4cee14eb5f813c) )

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	/* the protection data is not used at all! They forgot a debug flag set in the
	   code which skips the protection check. */
	ROM_LOAD( "mnst_m1.bin", 0x00000, 0x40000, CRC(77ba1eaf) SHA1(bde55b4d2938f44fd07ff7d5b5a845f2ea64b4fc) )   // same as housemnq/5i.bin gfx data
ROM_END

ROM_START( mjnanpau )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "03.bin",      0x00000, 0x10000, CRC(f96bdda7) SHA1(cea176ef11db0607137da70479ccde575bf7524a) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "mnst_02.bin", 0x00000, 0x10000, CRC(22c7ddce) SHA1(bc7106622592b6d7ccb839e0ce7a1760068209b7) )
	ROM_LOAD( "mnst_01.bin", 0x10000, 0x10000, CRC(13b79c41) SHA1(0e2446e04510f1ec0f0ed8d4f0239d3029341afe) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mnst_04.bin", 0x000000, 0x10000, CRC(7b8fb5f2) SHA1(5a6024d2a42046268cefa565a5f3c2fa5af8d74f) )
	ROM_LOAD( "05.bin",      0x010000, 0x10000, CRC(98219498) SHA1(76e2408d7b5e5d2cf0000d3a7f8e436917311268) )
	ROM_LOAD( "mnst_06.bin", 0x020000, 0x10000, CRC(1ea7db2e) SHA1(6b80b9c31900568c44afd4b4fe225d12ce9071b1) )
	ROM_LOAD( "mnst_07.bin", 0x030000, 0x10000, CRC(2930acbb) SHA1(65753a6d68abb4102d19f41f40f99fd8cb536873) )
	ROM_LOAD( "mnst_08.bin", 0x040000, 0x10000, CRC(cd632b5c) SHA1(4a24d027769ec7e14d3878f4e5490ce949f2fa63) )
	ROM_LOAD( "mnst_09.bin", 0x050000, 0x10000, CRC(77116d9e) SHA1(7224c5d21b582fdb93d80d8b2919d85aa546ffc3) )
	ROM_LOAD( "mnst_10.bin", 0x060000, 0x10000, CRC(5502e478) SHA1(d3c884c33be322f516cea16587e49806505c82c5) )
	ROM_LOAD( "11.bin",      0x070000, 0x10000, CRC(c4808c77) SHA1(cdae8844cfa70d42e63fd2799776a6bffb5c4bd4) )
	ROM_LOAD( "12.bin",      0x080000, 0x10000, CRC(f7be103c) SHA1(bfac1409d3f2b64b7998784950049b9a36cf4776) )
	ROM_LOAD( "13.bin",      0x090000, 0x10000, CRC(7eb39bb1) SHA1(9bad051a249388c6c37cbe6cc5274df1377ca784) )
	ROM_LOAD( "mnst_14.bin", 0x0a0000, 0x10000, CRC(03b32fa7) SHA1(fc42f4f96ba256e382b50e0fbcf44aee0dc8ec55) )
	ROM_LOAD( "15.bin",      0x0b0000, 0x10000, CRC(19acab3a) SHA1(a4433f84a5b6cfecdc9de23e892658614021d2d9) )
	ROM_LOAD( "16.bin",      0x0c0000, 0x10000, CRC(51e3d3e1) SHA1(a9c04379d656a25604a5dd51e1d33998e09aa875) )
	ROM_LOAD( "mnst_17.bin", 0x0d0000, 0x10000, CRC(23cac7e3) SHA1(ca7aeb8a6aa6d69d81dca52fb199cfa883a20219) )
	ROM_LOAD( "18.bin",      0x0e0000, 0x10000, CRC(754834f8) SHA1(055ade1994b5a01ac0c5a6661bfc59734bb13078) )
	ROM_LOAD( "19.bin",      0x0f0000, 0x10000, CRC(d72d9d75) SHA1(dc6c333be19789a3c55a1f149787c3ff3841e3b1) )
	ROM_LOAD( "20.bin",      0x100000, 0x10000, CRC(a87061c3) SHA1(5e51e0b69675835ebd439a24108915cd865a4c7f) )
	ROM_LOAD( "21.bin",      0x110000, 0x10000, CRC(14c5be81) SHA1(278dde6280f9651b3be123e621b0da60a9e09aee) )
	ROM_LOAD( "mnst_22.bin", 0x120000, 0x10000, CRC(3468f36f) SHA1(5723b6ba22268b6eca310e97fe19e4b8e4c57ca9) )
	ROM_LOAD( "23.bin",      0x130000, 0x10000, CRC(def886e1) SHA1(25a10ea8cf5905262197661dd6a22c0bd7d5ac6e) )

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	/* the protection data is not used at all! They forgot a debug flag set in the
	   code which skips the protection check. */
	ROM_LOAD( "mnst_m1.bin", 0x00000, 0x40000, CRC(77ba1eaf) SHA1(bde55b4d2938f44fd07ff7d5b5a845f2ea64b4fc) )   // same as housemnq/5i.bin gfx data
ROM_END


/*
Pairs
System Ten Co. Ltd., 1989

Hardware is by Nichibutsu with official seal, All ROMs
have official Nichibutsu stickers on them.

PCB No: GH1701
CPU   : Z80B
SOUND : YM3812, Y3014B
OSC   : 20.000MHz
RAM   : SONY CXK5814P-35L (x2), MCM514256 (x2), SANYO LC3517AL-10 (x1)
DIPSW : 8 position (x2)
OTHER : Unknown 40 Pin DIP (surface scratched, near Z80, PCB doesn't work at all if it is removed)
        Unknown 40 Pin DIP (surface scratched, made by Fujitsu, near gfx ROMs)
        Volume Pot (x2, labelled VOICE and MAIN)
        2 Unpopulated sockets for what appears to be 2 more M514256 RAMs (located next to the two M514256 RAMs)
PALs  : (x2, one near gfx ROMs, other near Z80)
PROMs : None

ROMs  : (All ROMs type 27C512)
------------------------------
1.J2      \ Main Program (3 grouped together)
2.K2       |Possibly one of them is sound related or for use with unknown 40 pin DIP IC?
3.J3      /

4.C8 -> 18.H10  GFX
*/

ROM_START( pairsnb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.bin",   0x00000, 0x10000, CRC(86cb9301) SHA1(ab0c1d01aac9a6e689ebf7a45e6cfae6e47bec85) )

	ROM_REGION( 0x20000, "voice", 0 )
	ROM_LOAD( "2.bin",   0x00000, 0x10000, CRC(f44ec73a) SHA1(9f13ea340ebc8affe47f43b8b831e9cdf9878823) )
	ROM_LOAD( "1.bin",   0x10000, 0x10000, CRC(5ca5bd18) SHA1(ac12a64a402c2d57062099e239bf26f00f7104f0) )

	ROM_REGION( 0xf0000, "gfx1", 0 )
	ROM_LOAD( "4.bin",   0x00000, 0x10000, CRC(dd13e9ec) SHA1(a3797ab372d6e5d375aeaa82c58a787d53b45852) )
	ROM_LOAD( "5.bin",   0x10000, 0x10000, CRC(42b55fa6) SHA1(7f9687fa6115a21d659a7d0d1c5ea8572d4cee23) )
	ROM_LOAD( "6.bin",   0x20000, 0x10000, CRC(5f901bf2) SHA1(923467860e1f446f2d0cde5104e3f34579776fbc) )
	ROM_LOAD( "7.bin",   0x30000, 0x10000, CRC(3c00e87d) SHA1(a16c653fc87b96b4e0efb0bf7bb838edcf4fe290) )
	ROM_LOAD( "8.bin",   0x40000, 0x10000, CRC(1b5b3ed2) SHA1(28a8b547b1e37e497c2a5eea86fd577aa42932a9) )
	ROM_LOAD( "9.bin",   0x50000, 0x10000, CRC(117175d1) SHA1(4fed4b724f1fa0788434c4d54faab7f60c4376ad) )
	ROM_LOAD( "10.bin",  0x60000, 0x10000, CRC(52228349) SHA1(24dc8f9fca148780c20e8636c5895d949722aea7) )
	ROM_LOAD( "11.bin",  0x70000, 0x10000, CRC(4e9606ff) SHA1(42dc8caf84007135980bef86bb0549c2e035bdf1) )
	ROM_LOAD( "12.bin",  0x80000, 0x10000, CRC(6c39e2e0) SHA1(cdf00882aba42d8d3c3a66112dcf4a697b2a6cda) )
	ROM_LOAD( "13.bin",  0x90000, 0x10000, CRC(f033769f) SHA1(422b62a44ef869ee4bbf491394f5b6c2864c99e1) )
	ROM_LOAD( "14.c10",  0xa0000, 0x10000, CRC(df263b08) SHA1(20932f244686f0dfd76fc6cc47840ddbc2fe8f94) )
	ROM_LOAD( "15.bin",  0xb0000, 0x10000, CRC(684842cb) SHA1(2303696c73ecd77d7d3d9d5bb7b938b711070359) )
	ROM_LOAD( "16.bin",  0xc0000, 0x10000, CRC(e7c1284e) SHA1(9189e0adb75f88b0e60fecc9beda760e7d410454) )
	ROM_LOAD( "17.bin",  0xd0000, 0x10000, CRC(c8d79e7f) SHA1(cec37d71ea47d8a30be6d91866f7c05fc8195716) )
	ROM_LOAD( "18.bin",  0xe0000, 0x10000, CRC(e3138cbc) SHA1(61c6fe7d6e77b68873891388186122f75a6fe7e6) )

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	ROM_LOAD( "mask.f2", 0x00000, 0x40000, CRC(77ba1eaf) SHA1(bde55b4d2938f44fd07ff7d5b5a845f2ea64b4fc) )   // same as housemnq/5i.bin gfx data
ROM_END

ROM_START( pairsten )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.j3",    0x00000, 0x10000, CRC(037d6acb) SHA1(9a01f9765fd4cd459e22fc639b23306e50d2f051) )

	ROM_REGION( 0x20000, "voice", 0 )
	ROM_LOAD( "2.k2",    0x00000, 0x10000, CRC(f44ec73a) SHA1(9f13ea340ebc8affe47f43b8b831e9cdf9878823) )
	ROM_LOAD( "1.j2",    0x10000, 0x10000, CRC(5ca5bd18) SHA1(ac12a64a402c2d57062099e239bf26f00f7104f0) )

	ROM_REGION( 0xf0000, "gfx1", 0 )
	ROM_LOAD( "4.c8",    0x00000, 0x10000, CRC(dd13e9ec) SHA1(a3797ab372d6e5d375aeaa82c58a787d53b45852) )
	ROM_LOAD( "5.d8",    0x10000, 0x10000, CRC(42b55fa6) SHA1(7f9687fa6115a21d659a7d0d1c5ea8572d4cee23) )
	ROM_LOAD( "6.e8",    0x20000, 0x10000, CRC(5f901bf2) SHA1(923467860e1f446f2d0cde5104e3f34579776fbc) )
	ROM_LOAD( "7.f8",    0x30000, 0x10000, CRC(3c00e87d) SHA1(a16c653fc87b96b4e0efb0bf7bb838edcf4fe290) )
	ROM_LOAD( "8.h8",    0x40000, 0x10000, CRC(1b5b3ed2) SHA1(28a8b547b1e37e497c2a5eea86fd577aa42932a9) )
	ROM_LOAD( "9.k8",    0x50000, 0x10000, CRC(117175d1) SHA1(4fed4b724f1fa0788434c4d54faab7f60c4376ad) )
	ROM_LOAD( "10.l8",   0x60000, 0x10000, CRC(52228349) SHA1(24dc8f9fca148780c20e8636c5895d949722aea7) )
	ROM_LOAD( "11.m8",   0x70000, 0x10000, CRC(4e9606ff) SHA1(42dc8caf84007135980bef86bb0549c2e035bdf1) )
	ROM_LOAD( "12.n8",   0x80000, 0x10000, CRC(6c39e2e0) SHA1(cdf00882aba42d8d3c3a66112dcf4a697b2a6cda) )
	ROM_LOAD( "13.p8",   0x90000, 0x10000, CRC(f033769f) SHA1(422b62a44ef869ee4bbf491394f5b6c2864c99e1) )
	ROM_LOAD( "14.c10",  0xa0000, 0x10000, CRC(df263b08) SHA1(20932f244686f0dfd76fc6cc47840ddbc2fe8f94) )
	ROM_LOAD( "15.d10",  0xb0000, 0x10000, CRC(684842cb) SHA1(2303696c73ecd77d7d3d9d5bb7b938b711070359) )
	ROM_LOAD( "16.e10",  0xc0000, 0x10000, CRC(e7c1284e) SHA1(9189e0adb75f88b0e60fecc9beda760e7d410454) )
	ROM_LOAD( "17.f10",  0xd0000, 0x10000, CRC(c8d79e7f) SHA1(cec37d71ea47d8a30be6d91866f7c05fc8195716) )
	ROM_LOAD( "18.h10",  0xe0000, 0x10000, CRC(e3138cbc) SHA1(61c6fe7d6e77b68873891388186122f75a6fe7e6) )

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	ROM_LOAD( "mask.f2", 0x00000, 0x40000, CRC(77ba1eaf) SHA1(bde55b4d2938f44fd07ff7d5b5a845f2ea64b4fc) )   // same as housemnq/5i.bin gfx data
ROM_END

ROM_START( bananadr )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4h",   0x00000, 0x10000, CRC(a6344e0d) SHA1(ee8df28fb2f579d3eb10d8aa454c6289de4a9239) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "2.4j",   0x00000, 0x20000, CRC(d6f24371) SHA1(4d99fa3fcbf3719975a0fe17a317e6e456d44326) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "3.11p",  0x000000, 0x10000, CRC(bcb94d00) SHA1(9261bd976094dd36e39f100b82a331919c79c0fa) )
	ROM_LOAD( "4.11n",  0x010000, 0x10000, CRC(90642607) SHA1(07f68572e7aef140e56ebb18e4bed56ce48a206a) )
	ROM_LOAD( "5.11m",  0x020000, 0x10000, CRC(1ea7db2e) SHA1(6b80b9c31900568c44afd4b4fe225d12ce9071b1) )
	ROM_LOAD( "6.11k",  0x030000, 0x10000, CRC(2930acbb) SHA1(65753a6d68abb4102d19f41f40f99fd8cb536873) )
	ROM_LOAD( "7.11j",  0x040000, 0x10000, CRC(cd632b5c) SHA1(4a24d027769ec7e14d3878f4e5490ce949f2fa63) )
	ROM_LOAD( "8.11h",  0x050000, 0x10000, CRC(77116d9e) SHA1(7224c5d21b582fdb93d80d8b2919d85aa546ffc3) )
	ROM_LOAD( "9.11e",  0x060000, 0x10000, CRC(5502e478) SHA1(d3c884c33be322f516cea16587e49806505c82c5) )
	ROM_LOAD( "10.11d", 0x070000, 0x10000, CRC(c4808c77) SHA1(cdae8844cfa70d42e63fd2799776a6bffb5c4bd4) )
	ROM_LOAD( "11.11c", 0x080000, 0x10000, CRC(f7be103c) SHA1(bfac1409d3f2b64b7998784950049b9a36cf4776) )
	ROM_LOAD( "12.11a", 0x090000, 0x10000, CRC(7eb39bb1) SHA1(9bad051a249388c6c37cbe6cc5274df1377ca784) )
	ROM_LOAD( "13.10p", 0x0a0000, 0x10000, CRC(03b32fa7) SHA1(fc42f4f96ba256e382b50e0fbcf44aee0dc8ec55) )
	ROM_LOAD( "14.10n", 0x0b0000, 0x10000, CRC(19acab3a) SHA1(a4433f84a5b6cfecdc9de23e892658614021d2d9) )
	ROM_LOAD( "15.10m", 0x0c0000, 0x10000, CRC(51e3d3e1) SHA1(a9c04379d656a25604a5dd51e1d33998e09aa875) )
	ROM_LOAD( "16.10k", 0x0d0000, 0x10000, CRC(23cac7e3) SHA1(ca7aeb8a6aa6d69d81dca52fb199cfa883a20219) )
	ROM_LOAD( "17.10j", 0x0e0000, 0x10000, CRC(754834f8) SHA1(055ade1994b5a01ac0c5a6661bfc59734bb13078) )
	ROM_LOAD( "18.10h", 0x0f0000, 0x10000, CRC(d72d9d75) SHA1(dc6c333be19789a3c55a1f149787c3ff3841e3b1) )
	ROM_LOAD( "19.10e", 0x100000, 0x10000, CRC(e8155a37) SHA1(467d2936839a4b47c596d5b5de76c33cdfe8d12b) )
	ROM_LOAD( "20.10d", 0x110000, 0x10000, CRC(3e44d46a) SHA1(1226f69f5dab2ba511a311472e24d1d6fbae2490) )
	ROM_LOAD( "21.10c", 0x120000, 0x10000, CRC(320c0d74) SHA1(abd33da491de2deb898550e988b0ad20b90d6ca0) )
	ROM_LOAD( "22.10a", 0x130000, 0x10000, CRC(def886e1) SHA1(25a10ea8cf5905262197661dd6a22c0bd7d5ac6e) )
ROM_END

ROM_START( club90s )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "c90s_03.bin", 0x00000, 0x10000, CRC(f8148ba5) SHA1(befff52276c369d4a8f2cc78ae88ecb6d90e7543) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "c90s_02.bin", 0x00000, 0x10000, CRC(b7938ed8) SHA1(b40a6e0baa94673c4ff61faf0c724355fdfb53bc) )
	ROM_LOAD( "c90s_01.bin", 0x10000, 0x10000, CRC(baaf17bd) SHA1(6579c841912087604ae328ce8bf80159f43622a3) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "c90s_04.bin", 0x080000, 0x20000, CRC(2c7d74ef) SHA1(953c84e4a7e2d296b0576434c4a39c3ea59f54f2) )
	ROM_LOAD( "c90s_05.bin", 0x0a0000, 0x20000, CRC(98d1f969) SHA1(577d33fd95b8f5767a0f9801ff8fc7c44cdf795e) )
	ROM_LOAD( "c90s_06.bin", 0x0c0000, 0x20000, CRC(509c1499) SHA1(a271c30660bdf74c822335f9742182ac19f5af53) )
	ROM_LOAD( "c90s_07.bin", 0x0e0000, 0x20000, CRC(8a8e2301) SHA1(e0e99835b5638bbd06c46bfe70133fa0c0bcd1f3) )
	ROM_LOAD( "c90s_08.bin", 0x100000, 0x20000, CRC(60fb6006) SHA1(ca2e5059e3ecfa5d30227f7c7ba30c72b8cff412) )
	ROM_LOAD( "c90s_09.bin", 0x120000, 0x20000, CRC(2fb74265) SHA1(e3421942a7c2bd2f18f694408feec9dac2f3945b) )
	ROM_LOAD( "c90s_10.bin", 0x140000, 0x20000, CRC(ca858e2c) SHA1(4d4ac9c662669159b7a736c93cf223e1be839c49) )
	ROM_LOAD( "c90s_11.bin", 0x160000, 0x20000, CRC(56ca8768) SHA1(876f284d3c8b3dcd131e5484358862b7883e4a8d) )
ROM_END

ROM_START( club90sa )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "c90s_23.bin", 0x00000, 0x10000, CRC(60433c11) SHA1(58a07271d1c7c3578cd4857bfaf9c9568b22a049) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "c90s_02.bin", 0x00000, 0x10000, CRC(b7938ed8) SHA1(b40a6e0baa94673c4ff61faf0c724355fdfb53bc) )
	ROM_LOAD( "c90s_01.bin", 0x10000, 0x10000, CRC(baaf17bd) SHA1(6579c841912087604ae328ce8bf80159f43622a3) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "c90s_04.bin", 0x080000, 0x20000, CRC(2c7d74ef) SHA1(953c84e4a7e2d296b0576434c4a39c3ea59f54f2) )
	ROM_LOAD( "c90s_05.bin", 0x0a0000, 0x20000, CRC(98d1f969) SHA1(577d33fd95b8f5767a0f9801ff8fc7c44cdf795e) )
	ROM_LOAD( "c90s_06.bin", 0x0c0000, 0x20000, CRC(509c1499) SHA1(a271c30660bdf74c822335f9742182ac19f5af53) )
	ROM_LOAD( "c90s_07.bin", 0x0e0000, 0x20000, CRC(8a8e2301) SHA1(e0e99835b5638bbd06c46bfe70133fa0c0bcd1f3) )
	ROM_LOAD( "c90s_08.bin", 0x100000, 0x20000, CRC(60fb6006) SHA1(ca2e5059e3ecfa5d30227f7c7ba30c72b8cff412) )
	ROM_LOAD( "c90s_09.bin", 0x120000, 0x20000, CRC(2fb74265) SHA1(e3421942a7c2bd2f18f694408feec9dac2f3945b) )
	ROM_LOAD( "c90s_10.bin", 0x140000, 0x20000, CRC(ca858e2c) SHA1(4d4ac9c662669159b7a736c93cf223e1be839c49) )
	ROM_LOAD( "c90s_11.bin", 0x160000, 0x20000, CRC(56ca8768) SHA1(876f284d3c8b3dcd131e5484358862b7883e4a8d) )
ROM_END

ROM_START( lovehous )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "2.3f",        0x00000, 0x10000, CRC(c3a0ed85) SHA1(93cc83f50e151fdf1a179f049604a02b590d4ec3) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "1.4c",        0x00000, 0x20000, CRC(afe8ce67) SHA1(facd7eb09fa326387b833db51bd1647abe8e38dd) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */

	ROM_LOAD( "c90s_04.bin", 0x000000, 0x20000, CRC(2c7d74ef) SHA1(953c84e4a7e2d296b0576434c4a39c3ea59f54f2) )
	ROM_LOAD( "4.10c",       0x020000, 0x20000, CRC(08bf9719) SHA1(302d50d4d6d98abb0eddc8277dc54825040d112b) )
	ROM_LOAD( "5.10d",       0x040000, 0x20000, CRC(79e9244c) SHA1(bad8868cafa8391d44be77e8f8258cb29c3c277f) )
	ROM_LOAD( "c90s_07.bin", 0x060000, 0x20000, CRC(8a8e2301) SHA1(e0e99835b5638bbd06c46bfe70133fa0c0bcd1f3) )
	ROM_LOAD( "c90s_08.bin", 0x080000, 0x20000, CRC(60fb6006) SHA1(ca2e5059e3ecfa5d30227f7c7ba30c72b8cff412) )
	ROM_LOAD( "c90s_09.bin", 0x0a0000, 0x20000, CRC(2fb74265) SHA1(e3421942a7c2bd2f18f694408feec9dac2f3945b) )
	ROM_LOAD( "c90s_10.bin", 0x0c0000, 0x20000, CRC(ca858e2c) SHA1(4d4ac9c662669159b7a736c93cf223e1be839c49) )
	ROM_LOAD( "c90s_11.bin", 0x0e0000, 0x20000, CRC(56ca8768) SHA1(876f284d3c8b3dcd131e5484358862b7883e4a8d) )
	ROM_LOAD( "11.10m",      0x100000, 0x20000, CRC(7cf6aad1) SHA1(44c90d3ecdcdec776d71c707474ba34588a4815d) )
ROM_END

ROM_START( mladyhtr )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "mlht_03.bin", 0x00000, 0x10000, CRC(bda76c24) SHA1(c779b9420162c5b077a16e2a20a592a56b088b2e) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "mlht_02.bin", 0x00000, 0x10000, CRC(e841696d) SHA1(bf1862b458f4363a53933959ddb28a52e617e051) )
	ROM_LOAD( "mlht_01.bin", 0x10000, 0x10000, CRC(75c35c62) SHA1(0b15abfa1f07f22e5116b06405a15c1b85f296cb) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mj-1802.bin", 0x000000, 0x80000, CRC(e6213f10) SHA1(377399e9cd20fc2055b680eb28d024824161b2ff) )
	ROM_LOAD( "mlht_04.bin", 0x080000, 0x20000, CRC(5896f484) SHA1(5eca71bacaed3cdde2554f43b565e7dd5a14e71e) )
	ROM_LOAD( "mlht_05.bin", 0x0a0000, 0x20000, CRC(bc26f689) SHA1(671e5261168c107d0f233a6079c5d14540552ad7) )
	ROM_LOAD( "mlht_06.bin", 0x0c0000, 0x20000, CRC(c24a9d5e) SHA1(b532f92dc946f31a31a8447a59c46be65d114a86) )
	ROM_LOAD( "mlht_07.bin", 0x0e0000, 0x10000, CRC(68c55f45) SHA1(1f69d5a47bcb631b932d6a5e006ad69efec46230) )
	ROM_LOAD( "mlht_08.bin", 0x0f0000, 0x10000, CRC(110afc31) SHA1(23725e500b038268ae5b3ff90e601e6af315258c) )
	ROM_LOAD( "mlht_09.bin", 0x100000, 0x10000, CRC(01739671) SHA1(138234a4d211196a99dd1649165e4eda1ab36b34) )
	ROM_LOAD( "mlht_10.bin", 0x110000, 0x10000, CRC(f0663672) SHA1(7f7eaeac115357c7fa0cb1e926d76f15fc196316) )
	ROM_LOAD( "mlht_11.bin", 0x120000, 0x10000, CRC(b8485904) SHA1(a3fb8690225cffa3621982d03bbd4d6f5d57af92) )
	ROM_LOAD( "mlht_12.bin", 0x130000, 0x10000, CRC(d58ac691) SHA1(cec3820f6768c91d431f678a35496a7aa8943fa6) )
	ROM_LOAD( "mlht_13.bin", 0x140000, 0x10000, CRC(a066e193) SHA1(4ca7cf7e50dc1563a6209e94004d7a3d14106fb4) )
	ROM_LOAD( "mlht_14.bin", 0x150000, 0x10000, CRC(b956b9e2) SHA1(825eeac2e8c571981ef7073cccb5e9383dcc5e71) )
	ROM_LOAD( "mlht_15.bin", 0x160000, 0x10000, CRC(af80f2a1) SHA1(e49e219199b58e75fe64268868f99f160edbd1a3) )
	ROM_LOAD( "mlht_16.bin", 0x170000, 0x10000, CRC(0775bbda) SHA1(a6758e3bc1abc875e6bc7e92815aeddfdc812987) )
	ROM_LOAD( "mlht_17.bin", 0x180000, 0x10000, CRC(b25d515b) SHA1(41aee09c4e7768e9cc7c15e4f0b1e7d2d4a10ced) )
	ROM_LOAD( "mlht_18.bin", 0x190000, 0x10000, CRC(30c30b07) SHA1(9110485c7b99fbb3a691ac1ce2075ca3de144e9d) )
	ROM_LOAD( "mlht_19.bin", 0x1a0000, 0x10000, CRC(5056763d) SHA1(2f6f2dda454d0f9eb6e8670c720148b6e2b4a027) )
	ROM_LOAD( "mlht_20.bin", 0x1b0000, 0x10000, CRC(a58edec9) SHA1(c7a7b4ad54140eb335e519bfb7ac638f6d62cd28) )
	ROM_LOAD( "mlht_21.bin", 0x1c0000, 0x10000, CRC(c7769608) SHA1(330cfd5e60a91a0e0e06804223cc063e337b67d3) )
ROM_END

ROM_START( chinmoku )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "3.3h",   0x00000, 0x10000, CRC(eddff33e) SHA1(b16ff69466463eeda01dc16ba7e62eac23bc8348) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "2.2k",   0x00000, 0x10000, CRC(0d6306e3) SHA1(a1d526ff5164ce527baf783f86545ac7596315f1) )
	ROM_LOAD( "1.2h",   0x10000, 0x10000, CRC(a85e681c) SHA1(c1b49f52216c8971e04b7848b03fecf585560ef6) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.8d",   0x020000, 0x20000, CRC(5b5234f6) SHA1(e3c5a358b7c766a974988a9be82df5a75d676918) )
	ROM_LOAD( "5.8e",   0x040000, 0x20000, CRC(56bf9a23) SHA1(e81ff1c2e931cbd8bb9861f9d490915886680ac4) )
	ROM_LOAD( "6.8f",   0x060000, 0x20000, CRC(188bdbd6) SHA1(5e0c205e94ba6a509e2942b8e9ec336610b924e1) )
	ROM_LOAD( "7.8h",   0x080000, 0x20000, CRC(eecb02e2) SHA1(01404f3aa3265f50276ad5efb3f41ae5997ba7ca) )
	ROM_LOAD( "8.8k",   0x0a0000, 0x20000, CRC(b3953fb2) SHA1(b72fc95ccdc32dc573e001ec9f553166930a0352) )
	ROM_LOAD( "9.8l",   0x0c0000, 0x20000, CRC(c1432f82) SHA1(ec90dacd643f6327b17cd6cb37a37351250efe48) )
	ROM_LOAD( "10.8m",  0x0e0000, 0x20000, CRC(9ec1f110) SHA1(3c5ae2c87ce4c5acd521877704e7d66b9f55c2f5) )
	ROM_LOAD( "11.8n",  0x100000, 0x20000, CRC(a5031090) SHA1(bd2f3f0d152f0bfc82ca4797b9ddd92f8ffdc880) )
	ROM_LOAD( "12.8p",  0x120000, 0x20000, CRC(900369a7) SHA1(b7253d63ae0eae6e924f9af21f22104631710fce) )
	ROM_LOAD( "13.10c", 0x140000, 0x10000, CRC(b38dd44d) SHA1(d514570b5f38cad3cc64e4ba348934968d77332a) )
	ROM_LOAD( "14.10d", 0x150000, 0x10000, CRC(e4a37c9a) SHA1(6b823b056547119d68239044158e6d143520aa94) )
	ROM_LOAD( "15.10e", 0x160000, 0x10000, CRC(ab443c6d) SHA1(e37be2cfecb84839fd953600e3ed0d06e0e4aca2) )
	ROM_LOAD( "16.10f", 0x170000, 0x10000, CRC(30c11267) SHA1(029359ca87444b9c51070c1d68faf0512422b01e) )
	ROM_LOAD( "17.10h", 0x180000, 0x10000, CRC(d0a17fcc) SHA1(e5fa97a7b4b3621f22157de0b7c7db7ae91432d0) )
	ROM_LOAD( "18.10k", 0x190000, 0x10000, CRC(8445fce2) SHA1(ec9796718cb63a4c6b1df5df5ffc81d5319b6c84) )
	ROM_LOAD( "19.10l", 0x1a0000, 0x10000, CRC(65b90ea1) SHA1(f2392454c49f1c03e31be92bd1cb2950f123f8f0) )
	ROM_LOAD( "20.10m", 0x1b0000, 0x10000, CRC(1445d8b0) SHA1(929193045fb05e8c24d7ed1dec9ef082dd90953b) )
	ROM_LOAD( "21.10n", 0x1c0000, 0x10000, CRC(38620a45) SHA1(662b1b47b9c786ceee44d0465d4dfd38daadd27f) )
	ROM_LOAD( "22.10p", 0x1d0000, 0x10000, CRC(85119fce) SHA1(3203e54f260f8dc2b28cdce84c1995f57d3d001a) )
ROM_END

ROM_START( maiko )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "mikb_02.bin", 0x00000, 0x10000, CRC(fbf68ebd) SHA1(0ddc9fc39bc362563462c57a728f1fc4ce3f682b) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "mikb_01.bin", 0x00000, 0x20000, CRC(713b3f8f) SHA1(460e9dcfc4a31f8e6d3f40ba77d6639257d9762f) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mikb_03.bin", 0x000000, 0x20000, CRC(0c949a6f) SHA1(c8d0011e22d62e46be20d1ac8328f5b2c47f3d31) )
	ROM_LOAD( "mikb_04.bin", 0x020000, 0x20000, CRC(8c841482) SHA1(94d7fe911ebfa8b19c65b56c774b29897df6bae8) )
	ROM_LOAD( "mikb_05.bin", 0x040000, 0x20000, CRC(7c61b4f7) SHA1(e23bb0f051d53846c70b298ca50f38f12585f958) )
	ROM_LOAD( "mikb_06.bin", 0x060000, 0x20000, CRC(7cc39a22) SHA1(957c9ec3ea3006649e993a406b30d79e32c05e32) )
	ROM_LOAD( "mikb_07.bin", 0x080000, 0x20000, CRC(0aaf5033) SHA1(1818f81ebd66ed7e5591551f223ac924f57ef921) )
	ROM_LOAD( "mikb_08.bin", 0x0a0000, 0x20000, CRC(2628caa1) SHA1(2163035dd9a0561d7a7ddf540d2c532fcb681ddc) )
	ROM_LOAD( "mj-1802.bin", 0x180000, 0x80000, CRC(e6213f10) SHA1(377399e9cd20fc2055b680eb28d024824161b2ff) )
ROM_END

ROM_START( mmaiko )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "2.3f",        0x00000, 0x10000, CRC(82b63476) SHA1(98c120b82953782f532c09b9305a8e6b5dedb374) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "1.4c",        0x00000, 0x20000, CRC(05fc906c) SHA1(d9979de8656cb43e1cd39d79e902662686bd19b0) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "3.10a",       0x000000, 0x20000, CRC(26ba1686) SHA1(565e7ee0e75c635bd6801efce136a4bc4bf72229) )
	ROM_LOAD( "4.10c",       0x020000, 0x20000, CRC(6ce799cb) SHA1(b06c83533f129544b9d70e3c1dda48a7f1404ebc) )
	ROM_LOAD( "mikb_05.bin", 0x040000, 0x20000, CRC(7c61b4f7) SHA1(e23bb0f051d53846c70b298ca50f38f12585f958) )
	ROM_LOAD( "mikb_06.bin", 0x060000, 0x20000, CRC(7cc39a22) SHA1(957c9ec3ea3006649e993a406b30d79e32c05e32) )
	ROM_LOAD( "7.10h",       0x080000, 0x20000, CRC(eb5a3971) SHA1(e4ad81a09c16b15faaea06ccd782a261a8e4bf7f) )
	ROM_LOAD( "8.10j",       0x0a0000, 0x20000, CRC(2ed61cdb) SHA1(1cdb897cacdd4416107f83b927493d6c41e600d2) )
	ROM_LOAD( "mj-1802.bin", 0x180000, 0x80000, CRC(e6213f10) SHA1(377399e9cd20fc2055b680eb28d024824161b2ff) )
ROM_END

ROM_START( hanaoji )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "hnoj_02.bin", 0x00000, 0x10000, CRC(580cd095) SHA1(e798e9db64072d14c46840235c88dcdcc3d3ec6a) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "hnoj_01.bin", 0x00000, 0x10000, CRC(3f7fcb94) SHA1(7bb0bc3a8c34b1b707b39ba52be40900cca0f015) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "hnoj_03.bin", 0x000000, 0x20000, CRC(fbbe1dce) SHA1(f742bb8e06a1e71e7c586d0a821f96238bdbc6ac) )
	ROM_LOAD( "hnoj_04.bin", 0x020000, 0x20000, CRC(2074b04f) SHA1(e759e49474bcb1caeea5a60708844ec53aed64c6) )
	ROM_LOAD( "hnoj_05.bin", 0x040000, 0x20000, CRC(84d20ba6) SHA1(0f270d43cdb390492f349b3680978e2e36a6a5d4) )
	ROM_LOAD( "hnoj_06.bin", 0x060000, 0x20000, CRC(f85fedd8) SHA1(224a5b05c28b1f84df0bd32b32cb2aa416156460) )
	ROM_LOAD( "hnoj_07.bin", 0x080000, 0x20000, CRC(c72cdde1) SHA1(877cd52461ecc9cd44d5b328c36ac8878056059d) )
	ROM_LOAD( "hnoj_08.bin", 0x0a0000, 0x20000, CRC(12e70429) SHA1(4728a5a0f636f793099c5a3a7bc998931921623f) )
	ROM_LOAD( "hnoj_09.bin", 0x0c0000, 0x20000, CRC(4ec74a59) SHA1(92803e99aa6fb5c8f2227db3b7cc875266249ed1) )
	ROM_LOAD( "hnoj_10.bin", 0x0e0000, 0x20000, CRC(e9212fc5) SHA1(c09f4a93f01630696acb0e80b1c6adb711377319) )
	ROM_LOAD( "hnoj_11.bin", 0x100000, 0x20000, CRC(bfe38671) SHA1(6c81864caab61ea60dfe446b390221bdcfb0895e) )
ROM_END

ROM_START( hanaojia )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "02.f3.bin", 0x00000, 0x10000, CRC(2f493c0b) SHA1(0c2b2ece744556f8b2d25fde9017680a77afcf6b) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "hnoj_01.bin", 0x00000, 0x10000, CRC(3f7fcb94) SHA1(7bb0bc3a8c34b1b707b39ba52be40900cca0f015) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "hnoj_03.bin", 0x000000, 0x20000, CRC(fbbe1dce) SHA1(f742bb8e06a1e71e7c586d0a821f96238bdbc6ac) )
	ROM_LOAD( "hnoj_04.bin", 0x020000, 0x20000, CRC(2074b04f) SHA1(e759e49474bcb1caeea5a60708844ec53aed64c6) )
	ROM_LOAD( "hnoj_05.bin", 0x040000, 0x20000, CRC(84d20ba6) SHA1(0f270d43cdb390492f349b3680978e2e36a6a5d4) )
	ROM_LOAD( "hnoj_06.bin", 0x060000, 0x20000, CRC(f85fedd8) SHA1(224a5b05c28b1f84df0bd32b32cb2aa416156460) )
	ROM_LOAD( "hnoj_07.bin", 0x080000, 0x20000, CRC(c72cdde1) SHA1(877cd52461ecc9cd44d5b328c36ac8878056059d) )
	ROM_LOAD( "hnoj_08.bin", 0x0a0000, 0x20000, CRC(12e70429) SHA1(4728a5a0f636f793099c5a3a7bc998931921623f) )
	ROM_LOAD( "hnoj_09.bin", 0x0c0000, 0x20000, CRC(4ec74a59) SHA1(92803e99aa6fb5c8f2227db3b7cc875266249ed1) )
	ROM_LOAD( "hnoj_10.bin", 0x0e0000, 0x20000, CRC(e9212fc5) SHA1(c09f4a93f01630696acb0e80b1c6adb711377319) )
	ROM_LOAD( "hnoj_11.bin", 0x100000, 0x20000, CRC(bfe38671) SHA1(6c81864caab61ea60dfe446b390221bdcfb0895e) )
ROM_END

ROM_START( mjcamerb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "2.3h",        0x00000, 0x10000, CRC(3a0f110b) SHA1(8923136ed25ed91c90f93c3f75f5532ff8f9d420) )

	ROM_REGION( 0x30000, "voice", 0 ) /* voice */
	ROM_LOAD( "1.2k",        0x00000, 0x10000, CRC(fe8e975e) SHA1(7287f5654aebc1f27c957d4af997480fa380b15a) )
	/* protection data is mapped at 20000-2ffff */

	ROM_REGION( 0x100000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "3.8c",        0x000000, 0x10000, CRC(273fb8bc) SHA1(d5aa20570a1ad7a97d2b4eb12039c51d85288a96) )
	ROM_LOAD( "4.8d",        0x010000, 0x10000, CRC(82995399) SHA1(0aa4dcbed01f6a4893c31487469989035fd791c3) )
	ROM_LOAD( "5.8e",        0x020000, 0x10000, CRC(a7c51d54) SHA1(b553136a1e2bf3e6cd61a0b0ebd0341a389ac65f) )
	ROM_LOAD( "6.8f",        0x030000, 0x10000, CRC(f221700c) SHA1(e8f640bd8a155be96e70bc8bc4e29b614695b0db) )
	ROM_LOAD( "7.8h",        0x040000, 0x10000, CRC(6baa4d45) SHA1(61a5470f85260ad957069cfa422c918905659f02) )
	ROM_LOAD( "8.8k",        0x050000, 0x10000, CRC(91d9c868) SHA1(a584f6fa46defbee49c5c6441b50233749a45118) )
	ROM_LOAD( "9.8l",        0x060000, 0x10000, CRC(56a35d4b) SHA1(1c769798661531f760da5d54af7f86d45e8e6c0f) )
	ROM_LOAD( "10.8m",       0x070000, 0x10000, CRC(480e23c4) SHA1(e357134a3bc68437b5f36a563c69ef7583861aab) )
	ROM_LOAD( "11.8n",       0x080000, 0x10000, CRC(2c29accc) SHA1(e3ff6db06e4001262093d28cb44c0912de16989a) )
	ROM_LOAD( "12.8p",       0x090000, 0x10000, CRC(902d73f8) SHA1(cead5c1a072fb95847f50af2e65f6108ef5f4928) )
	ROM_LOAD( "13.10c",      0x0a0000, 0x10000, CRC(fcba0179) SHA1(34b1e9a4908dbed3dcbbeafe5b05dccee6aef13a) )
	ROM_LOAD( "14.10d",      0x0b0000, 0x10000, CRC(ee2c37a9) SHA1(fff260eade85ee3c01b32d3eea6133c85a22d645) )
	ROM_LOAD( "15.10e",      0x0c0000, 0x10000, CRC(90fd36f8) SHA1(ec8e9e6a52a5a8a9e3f688a400e946dae643f747) )
	ROM_LOAD( "16.10f",      0x0d0000, 0x10000, CRC(41265f7f) SHA1(98d02ed1af3adeaf9aa261d98e48d2745a0eec28) )
	ROM_LOAD( "17.10h",      0x0e0000, 0x10000, CRC(78cef468) SHA1(aedd94d3fcf097587e77f52d03a50a63606bdab6) )
	ROM_LOAD( "18.10k",      0x0f0000, 0x10000, CRC(3a3da341) SHA1(198ea75aedff187b02a740d5a1cc49c76340831f) )

	ROM_REGION( 0x40000, "protection", 0 ) /* protection data */
	ROM_LOAD( "mcam_m1.bin", 0x00000, 0x40000, CRC(f85c5b07) SHA1(0fc55e9b60ccc630a0d77862eb5e64a3ba366947) )   // same as housemnq/3i.bin gfx data
ROM_END

ROM_START( mmcamera )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "2.3ha",  0x00000, 0x10000, CRC(b6eed2cf) SHA1(87171ba9ba247e54244867f720738f9b88a1213e) )

	ROM_REGION( 0x10000, "voice", 0 ) /* voice */
	ROM_LOAD( "1.2k",   0x00000, 0x10000, CRC(fe8e975e) SHA1(7287f5654aebc1f27c957d4af997480fa380b15a) )

	ROM_REGION( 0x110000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "3.8c",   0x000000, 0x10000, CRC(273fb8bc) SHA1(d5aa20570a1ad7a97d2b4eb12039c51d85288a96) )
	ROM_LOAD( "4.8d",   0x010000, 0x10000, CRC(82995399) SHA1(0aa4dcbed01f6a4893c31487469989035fd791c3) )
	ROM_LOAD( "5.8e",   0x020000, 0x10000, CRC(a7c51d54) SHA1(b553136a1e2bf3e6cd61a0b0ebd0341a389ac65f) )
	ROM_LOAD( "6.8f",   0x030000, 0x10000, CRC(f221700c) SHA1(e8f640bd8a155be96e70bc8bc4e29b614695b0db) )
	ROM_LOAD( "7.8h",   0x040000, 0x10000, CRC(6baa4d45) SHA1(61a5470f85260ad957069cfa422c918905659f02) )
	ROM_LOAD( "8.8k",   0x050000, 0x10000, CRC(91d9c868) SHA1(a584f6fa46defbee49c5c6441b50233749a45118) )
	ROM_LOAD( "9.8l",   0x060000, 0x10000, CRC(56a35d4b) SHA1(1c769798661531f760da5d54af7f86d45e8e6c0f) )
	ROM_LOAD( "10.8m",  0x070000, 0x10000, CRC(480e23c4) SHA1(e357134a3bc68437b5f36a563c69ef7583861aab) )
	ROM_LOAD( "11.8n",  0x080000, 0x10000, CRC(2c29accc) SHA1(e3ff6db06e4001262093d28cb44c0912de16989a) )
	ROM_LOAD( "12.8p",  0x090000, 0x10000, CRC(902d73f8) SHA1(cead5c1a072fb95847f50af2e65f6108ef5f4928) )
	ROM_LOAD( "13.10c", 0x0a0000, 0x10000, CRC(fcba0179) SHA1(34b1e9a4908dbed3dcbbeafe5b05dccee6aef13a) )
	ROM_LOAD( "14.10d", 0x0b0000, 0x10000, CRC(ee2c37a9) SHA1(fff260eade85ee3c01b32d3eea6133c85a22d645) )
	ROM_LOAD( "15.10e", 0x0c0000, 0x10000, CRC(90fd36f8) SHA1(ec8e9e6a52a5a8a9e3f688a400e946dae643f747) )
	ROM_LOAD( "16.10f", 0x0d0000, 0x10000, CRC(41265f7f) SHA1(98d02ed1af3adeaf9aa261d98e48d2745a0eec28) )
	ROM_LOAD( "17.10h", 0x0e0000, 0x10000, CRC(78cef468) SHA1(aedd94d3fcf097587e77f52d03a50a63606bdab6) )
	ROM_LOAD( "18.10ka",0x0f0000, 0x10000, CRC(59182700) SHA1(cdf6b5dba205254e26dcfc4b33238f270eb71551) )
	ROM_LOAD( "9.10l",  0x100000, 0x10000, CRC(0429ae8f) SHA1(e380e159b2dcafcbfd3e9991ee9e76b842189e37) )
ROM_END

ROM_START( taiwanmb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.3d",        0x00000, 0x10000, CRC(2165310e) SHA1(18151e849fede6f7e5275ab27e50ce8e4c227332) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "3.7n",        0x00000, 0x20000, CRC(1890902f) SHA1(1ef66fd70fefa09d31f77d0834a406fcc70e0261) )

	ROM_REGION( 0x0a0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.10a",       0x000000, 0x10000, CRC(6ccaab80) SHA1(7751012d555d7179f1e28244bf30240dbd1c0b38) )
	ROM_LOAD( "5.10c",       0x010000, 0x10000, CRC(ef29a87e) SHA1(f6af16ffd49009b18cefe50e937c1901a37daa3c) )
	ROM_LOAD( "6.10d",       0x020000, 0x10000, CRC(539c2443) SHA1(accb2f58a97f91e7e5c378d2f4887a78a055e1b7) )
	ROM_LOAD( "7.10e",       0x030000, 0x10000, CRC(247b90b1) SHA1(b4ea60401c673b7d2124ea15cd6a53601b2bedb9) )
	ROM_LOAD( "8.10h",       0x040000, 0x10000, CRC(416af9b7) SHA1(1d6fe7e711eeb2467229e59eec256bdf5d843aa5) )
	ROM_LOAD( "9.10j",       0x050000, 0x10000, CRC(33a1c8a4) SHA1(75b108cee1257505f140677f6ae0907f5a4b8005) )
	ROM_LOAD( "10.10k",      0x060000, 0x10000, CRC(375b568b) SHA1(63a929629163d7fd421716871809d39eb6d8f308) )
	ROM_LOAD( "11.10m",      0x070000, 0x10000, CRC(bf3256b7) SHA1(426a87103e85580d57655ab9aee3ed89fd094c13) )
	ROM_LOAD( "12.10n",      0x080000, 0x10000, CRC(ec59b720) SHA1(fd4eb63337885d7fa58ea2f3507d32fda4dee343) )
	ROM_LOAD( "13.10p",      0x090000, 0x10000, CRC(6542e7da) SHA1(be6db10178f532ac273599808c52126037ac78a0) )

	ROM_REGION( 0x00800, "protection", 0 ) /* protection (Intel D87??; mcu data that is extracted from the real board!) */
	ROM_LOAD( "clut_ram.8f", 0x000000, 0x00800, CRC(bd80fa09) SHA1(896421e1e01beef0de6acebbbc255224e4e0438b) )
ROM_END

/*
AV Hanafuda Hana no Christmas Eve
(c)1990 Nichibutsu

The PCB accepts composite video and monaural audio input from VCR.
Then converts the signal to RGB (uses NEC uPC1352C).

CPU: Z80B
Sound: YM3812 Y3014B
OSC: 3.579545MHz, 20.00000MHz
Custom: 1413M3

ROMs:
A.4C
B.4F
C.10A
D.10C
E.10D
F.10E
G.10F
H.10J
I.10K
J.10L
K.10M


dumped by sayu
--- Team Japump!!! ---

*/

ROM_START( hnxmasev )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "b.3f",   0x00000, 0x10000, CRC(45e34624) SHA1(db7f880a8b2f36d5bed939bd0b2694f27e29141b) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "a.3c",    0x00000, 0x20000, CRC(713b3f8f) SHA1(460e9dcfc4a31f8e6d3f40ba77d6639257d9762f) ) //same as maiko

	ROM_REGION( 0x80000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "c.10a", 0x000000, 0x10000, CRC(e46a1baa) SHA1(95ccf45a3c542391b67bd7993b5f7828ab525ebc) )
	ROM_LOAD( "d.10c", 0x010000, 0x10000, CRC(0777dad4) SHA1(7c502d78f778402614a5850bc9066322dda3f73d) )
	ROM_LOAD( "e.10d", 0x020000, 0x10000, CRC(eedd6244) SHA1(b636dacc4b2064c50ecaffef082735159031d333) )
	ROM_LOAD( "f.10e", 0x030000, 0x10000, CRC(9a8f2cf0) SHA1(31b504785c8e0747bc61a06d1684d96dbd84c261) )
	ROM_LOAD( "g.10h", 0x040000, 0x10000, CRC(a78fe88a) SHA1(d5e8a02d2266bd0e61ee662c5d3fe9ea4f3ebfb3) )
	ROM_LOAD( "h.10j", 0x050000, 0x10000, CRC(4810eb2e) SHA1(89b91b444f41127559e21f794eda4922b56b50bd) )
	ROM_LOAD( "i.10k", 0x060000, 0x10000, CRC(cf0c26cf) SHA1(5f64779abc578388e712abb381e2bbdbf4f78e0c) )
	ROM_LOAD( "j.10l", 0x070000, 0x10000, CRC(b0fb3334) SHA1(99032e00ccfbc903dc068d174f8d51211269b99c) )

	ROM_REGION( 0x10000, "vcr", 0 ) /* vcr prg?data? */
	ROM_LOAD( "k.10m",   0x00000, 0x10000, CRC(e29e9ef2) SHA1(5a3ea8f771f3191fad88d237b70301634353b7bb) )

	DISK_REGION( "vhs" ) /* Video Home System tape */
	DISK_IMAGE_READONLY( "hnxmasev", 0, NO_DUMP )
ROM_END

/*
AV Hanafuda Hana no Ageman
(c)1990 Nichibutsu / AV Japan

The PCB accepts composite video and monaural audio input from VCR.
Then converts the signal to RGB (uses NEC uPC1352C).

CPU: Z80B
Sound: YM3812 Y3014B
OSC: 3.579545MHz, 20.00000MHz
Custom: 1413M3

ROMs:
1.4C
2.4F
3.10A
4.10C
5.10D
6.10E
7.10F
8.10J
9.10K


dumped by sayu
--- Team Japump!!! ---
http://japump.i.am/

*/

ROM_START( hnageman )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "2.3f",   0x00000, 0x10000, CRC(155ed09a) SHA1(254f199063fe525c574032ae69d4d21b0debb4c5) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "1.3c",    0x00000, 0x20000, CRC(713b3f8f) SHA1(460e9dcfc4a31f8e6d3f40ba77d6639257d9762f) ) //same as maiko

	ROM_REGION( 0xd0000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "3.10a", 0x000000, 0x20000, CRC(080e0daa) SHA1(331137392c46fededdc55bd731c3a0bee88e59e3) )
	ROM_LOAD( "4.10c", 0x020000, 0x20000, CRC(d297ee95) SHA1(a89d438175b6f0b3af83a4706744a19c0d742904) )
	ROM_LOAD( "5.10d", 0x040000, 0x20000, CRC(c2eb8ced) SHA1(d58ef5f7d47da4624595058c23e949e193ced40a) )
	ROM_LOAD( "6.10e", 0x060000, 0x20000, CRC(36106de2) SHA1(15f2957c3e83d05b5706a0b98068a3d0d8d57ae9) )
	ROM_LOAD( "7.10h", 0x080000, 0x20000, CRC(0bb99a3a) SHA1(1c896f50f74a52fcf96a40ca536502908bab85c9) )
	ROM_LOAD( "8.10j", 0x0a0000, 0x20000, CRC(23fad43a) SHA1(7276b92623dc4559e62d2f4742c8a68233c1dfe5) )
	ROM_LOAD( "9.10k", 0x0c0000, 0x10000, CRC(c995c1da) SHA1(c8a1f4919296221c375763b0d9838f31ed53135d) )

	DISK_REGION( "vhs" ) /* Video Home System tape */
	DISK_IMAGE_READONLY( "hnageman", 0, NO_DUMP )
ROM_END


//     YEAR,     NAME,   PARENT,  MACHINE,    INPUT,     INIT, MONITOR,COMPANY,FULLNAME,FLAGS)
GAME( 1988, msjiken,   0,        msjiken,  msjiken,   driver_device,        0, ROT270, "Nichibutsu", "Mahjong Satsujin Jiken (Japan 881017)", GAME_SUPPORTS_SAVE )
GAME( 1988, hanamomo,  0,        hanamomo, hanamomo,  driver_device,        0, ROT0,   "Nichibutsu", "Mahjong Hana no Momoko gumi (Japan 881201)", GAME_SUPPORTS_SAVE )
GAME( 1988, hanamomb,  hanamomo, hanamomo, hanamomo,  driver_device,        0, ROT0,   "Nichibutsu", "Mahjong Hana no Momoko gumi (Japan 881125)", GAME_SUPPORTS_SAVE )
GAME( 1988, telmahjn,  0,        telmahjn, telmahjn, nbmj8891_state, telmahjn, ROT270, "Nichibutsu", "Telephone Mahjong (Japan 890111)", GAME_SUPPORTS_SAVE )
GAME( 1989, gionbana,  0,        gionbana, gionbana, nbmj8891_state, gionbana, ROT0,   "Nichibutsu", "Gionbana (Japan 890120)", GAME_SUPPORTS_SAVE )
GAME( 1989, mgion,     0,        mgion,    mgion,     driver_device,        0, ROT0,   "Nichibutsu", "Gionbana [BET] (Japan 890207)", GAME_SUPPORTS_SAVE )
GAME( 1989, omotesnd,  0,        omotesnd, omotesnd, nbmj8891_state, omotesnd, ROT0,   "Anime Tec", "Omotesandou (Japan 890215)", GAME_SUPPORTS_SAVE )
GAME( 1989, abunai,    0,        abunai,   abunai,    driver_device,        0, ROT0,   "Green Soft", "Abunai Houkago - Mou Matenai (Japan 890325)", GAME_SUPPORTS_SAVE )
GAME( 1989, mjfocus,   0,        mjfocus,  mjfocus, nbmj8891_state,  mjfocus,  ROT0,   "Nichibutsu", "Mahjong Focus (Japan 890313)", GAME_SUPPORTS_SAVE )
GAME( 1989, mjfocusm,  mjfocus,  mjfocusm, mjfocusm, nbmj8891_state, mjfocusm, ROT0,   "Nichibutsu", "Mahjong Focus [BET] (Japan 890510)", GAME_SUPPORTS_SAVE )
GAME( 1989, peepshow,  mjfocus,  mjfocus,  peepshow, nbmj8891_state, mjfocus,  ROT0,   "AC", "Nozokimeguri Mahjong Peep Show (Japan 890404)", GAME_SUPPORTS_SAVE )
GAME( 1989, mjcamerb,  0,        mjcamerb, mjcamerb, driver_device,         0, ROT0,   "Miki Syouji", "Mahjong Camera Kozou (set 2) (Japan 881109)", GAME_SUPPORTS_SAVE )
GAME( 1989, mmcamera,  mjcamerb, mmcamera, mmcamera, driver_device,         0, ROT0,   "Miki Syouji", "Mahjong Camera Kozou [BET] (Japan 890509)", GAME_SUPPORTS_SAVE )
GAME( 1989, scandal,   0,        scandal,  scandal, nbmj8891_state,  scandal,  ROT0,   "Nichibutsu", "Scandal Mahjong (Japan 890213)", GAME_SUPPORTS_SAVE )
GAME( 1989, scandalm,  scandal,  scandalm, scandalm, driver_device,         0, ROT0,   "Nichibutsu", "Scandal Mahjong [BET] (Japan 890217)", GAME_SUPPORTS_SAVE )
GAME( 1989, mgmen89,   0,        mgmen89,  mgmen89,  nbmj8891_state, mgmen89,  ROT0,   "Nichibutsu", "Mahjong G-MEN'89 (Japan 890425)", GAME_SUPPORTS_SAVE )
GAME( 1989, mjnanpas,  0,        mjnanpas, mjnanpas, nbmj8891_state, mjnanpas, ROT0,   "Brooks", "Mahjong Nanpa Story (Japan 890713)", GAME_SUPPORTS_SAVE )
GAME( 1989, mjnanpaa,  mjnanpas, mjnanpas, mjnanpaa, nbmj8891_state, mjnanpas, ROT0,   "Brooks", "Mahjong Nanpa Story (Japan 890712)", GAME_SUPPORTS_SAVE )
GAME( 1989, mjnanpau,  mjnanpas, mjnanpas, mjnanpas, nbmj8891_state, mjnanpas, ROT0,   "Brooks", "Mahjong Nanpa Story (Ura) (Japan 890805)", GAME_SUPPORTS_SAVE )
GAME( 1989, bananadr,  0,        bananadr, bananadr, driver_device,         0, ROT0,   "Digital Soft", "Mahjong Banana Dream [BET] (Japan 891124)", GAME_SUPPORTS_SAVE )
GAME( 1990, mladyhtr,  0,        mladyhtr, mladyhtr, driver_device,         0, ROT0,   "Nichibutsu", "Mahjong The Lady Hunter (Japan 900509)", GAME_SUPPORTS_SAVE )
GAME( 1990, chinmoku,  0,        chinmoku, chinmoku, driver_device,         0, ROT0,   "Nichibutsu", "Mahjong Chinmoku no Hentai (Japan 900511)", GAME_SUPPORTS_SAVE )
GAME( 1990, maiko,     0,        maiko,    maiko,    driver_device,         0, ROT0,   "Nichibutsu", "Maikobana (Japan 900802)", GAME_SUPPORTS_SAVE )
GAME( 1990, mmaiko,    0,        mmaiko,   mmaiko,   driver_device,         0, ROT0,   "Nichibutsu", "Maikobana [BET] (Japan 900911)", GAME_SUPPORTS_SAVE )
GAME( 1990, hnxmasev,  0,        hnxmasev, maiko,    driver_device,         0, ROT180, "Nichibutsu / AV Japan", "AV Hanafuda Hana no Christmas Eve (Japan 901204)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1990, hnageman,  0,        hnageman, maiko,    driver_device,         0, ROT180, "Nichibutsu / AV Japan", "AV Hanafuda Hana no Ageman (Japan 900716)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1990, club90s,   0,        club90s,  club90s,  driver_device,         0, ROT0,   "Nichibutsu", "Mahjong CLUB 90's (set 1) (Japan 900919)", GAME_SUPPORTS_SAVE )
GAME( 1990, club90sa,  club90s,  club90s,  club90s,  driver_device,         0, ROT0,   "Nichibutsu", "Mahjong CLUB 90's (set 2) (Japan 900919)", GAME_SUPPORTS_SAVE )
GAME( 1990, lovehous,  club90s,  lovehous, lovehous, driver_device,         0, ROT0,   "Nichibutsu", "Mahjong Love House [BET] (Japan 901024)", GAME_SUPPORTS_SAVE )
GAME( 1991, hanaoji,   0,        hanaoji,  hanaoji,  driver_device,         0, ROT0,   "Nichibutsu", "Hana to Ojisan [BET] (ver 1.01, 1991/12/09)", GAME_SUPPORTS_SAVE )
GAME( 1991, hanaojia,  hanaoji,  hanaoji,  hanaoji,  driver_device,         0, ROT0,   "Nichibutsu", "Hana to Ojisan [BET] (ver 1.00, 1991/08/23)", GAME_SUPPORTS_SAVE )
GAME( 1988, taiwanmb,  0,        taiwanmb, taiwanmb, driver_device,         0, ROT0,   "Miki Syouji", "Taiwan Mahjong [BET] (Japan 881208)", GAME_SUPPORTS_SAVE )
GAME( 1989, pairsnb,   0,        pairsnb, pairsnb,  nbmj8891_state,  pairsnb,  ROT0,   "Nichibutsu", "Pairs (Nichibutsu) (Japan 890822)", GAME_SUPPORTS_SAVE )
GAME( 1989, pairsten,  pairsnb,  pairsten, pairsnb, nbmj8891_state,  pairsten, ROT0,   "System Ten", "Pairs (System Ten) (Japan 890826)", GAME_SUPPORTS_SAVE )
