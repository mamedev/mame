/***************************************************************************

Tiger Road (C) 1987 Romstar/Capcom USA

Please contact Phil Stroffolino (phil@maya.com) if there are any questions
regarding this driver.

F1 Dream protection workaround by Eric Hustvedt

Memory Overview:
    0xfe0800    sprites
    0xfec000    text
    0xfe4000    input ports,dip switches (read); sound out, video reg (write)
    0xfe4002    protection (F1 Dream only)
    0xfe8000    scroll registers
    0xff8200    palette
    0xffC000    working RAM
    0xffEC70    high scores (not saved)

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "includes/tigeroad.h"



/*
 F1 Dream protection code written by Eric Hustvedt (hustvedt@ma.ultranet.com).

 The genuine F1 Dream game uses an 8751 microcontroller as a protection measure.
 Since the microcontroller's ROM is unavailable all interactions with it are handled
 via blackbox algorithm.

 Some notes:
 - The 8751 is triggered via location 0xfe4002, in place of the soundlatch normally
    present. The main cpu writes 0 to the location when it wants the 8751 to perform some work.
 - The 8751 has memory which shadows locations 0xffffe0-0xffffff of the main cpu's address space.
 - The word at 0xffffe0 contains an 'opcode' which is written just before the write to 0xfe4002.
 - Some of the writes to the soundlatch may not be handled. 0x27fc is the main sound routine, the
    other locations are less frequently used.
*/

static const int f1dream_613ea_lookup[16] = {
0x0052, 0x0031, 0x00a7, 0x0043, 0x0007, 0x008a, 0x00b1, 0x0066, 0x009f, 0x00cc, 0x0009, 0x004d, 0x0033, 0x0028, 0x00d0, 0x0025};

static const int f1dream_613eb_lookup[256] = {
0x0001, 0x00b5, 0x00b6, 0x00b6, 0x00b6, 0x00b6, 0x00b6, 0x00b6, 0x00b7, 0x0001, 0x00b8, 0x002f, 0x002f, 0x002f, 0x002f, 0x00b9,
0x00aa, 0x0031, 0x00ab, 0x00ab, 0x00ab, 0x00ac, 0x00ad, 0x00ad, 0x00ae, 0x00af, 0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x0091,
0x009c, 0x009d, 0x009e, 0x009f, 0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x009b, 0x0091,
0x00bc, 0x0092, 0x000b, 0x0009, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0073, 0x0001, 0x0098, 0x0099, 0x009a, 0x009b, 0x0091,
0x00bc, 0x007b, 0x000b, 0x0008, 0x0087, 0x0088, 0x0089, 0x008a, 0x007f, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f, 0x0090, 0x0091,
0x00bd, 0x007b, 0x000b, 0x0007, 0x007c, 0x007d, 0x007e, 0x0001, 0x007f, 0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086,
0x00bc, 0x0070, 0x000b, 0x0006, 0x0071, 0x0072, 0x0073, 0x0001, 0x0074, 0x000d, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a,
0x00bc, 0x00ba, 0x000a, 0x0005, 0x0065, 0x0066, 0x0067, 0x0068, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
0x00bc, 0x0059, 0x0001, 0x0004, 0x005a, 0x005b, 0x0001, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064,
0x0014, 0x004d, 0x0001, 0x0003, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0001, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058,
0x0014, 0x0043, 0x0001, 0x0002, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x00bb, 0x004a, 0x004b, 0x004c, 0x0001, 0x0001,
0x0014, 0x002b, 0x0001, 0x0038, 0x0039, 0x003a, 0x003b, 0x0031, 0x003c, 0x003d, 0x003e, 0x003f, 0x0040, 0x0041, 0x0042, 0x0001,
0x0014, 0x002d, 0x0001, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0001, 0x0014, 0x0037, 0x0001,
0x0014, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x0001, 0x0001, 0x0001, 0x002a, 0x002b, 0x002c,
0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001e, 0x001e, 0x001e, 0x001f, 0x0020,
0x000c, 0x000d, 0x000e, 0x0001, 0x000f, 0x0010, 0x0011, 0x0012, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x0013 };

static const int f1dream_17b74_lookup[128] = {
0x0003, 0x0040, 0x0005, 0x0080, 0x0003, 0x0080, 0x0005, 0x00a0, 0x0003, 0x0040, 0x0005, 0x00c0, 0x0003, 0x0080, 0x0005, 0x00e0,
0x0003, 0x0040, 0x0006, 0x0000, 0x0003, 0x0080, 0x0006, 0x0020, 0x0003, 0x0040, 0x0006, 0x0040, 0x0003, 0x0080, 0x0006, 0x0060,
0x0000, 0x00a0, 0x0009, 0x00e0, 0x0000, 0x00e0, 0x000a, 0x0000, 0x0000, 0x00a0, 0x000a, 0x0020, 0x0000, 0x00e0, 0x000a, 0x0040,
0x0000, 0x00a0, 0x000a, 0x0060, 0x0000, 0x00e0, 0x000a, 0x0080, 0x0000, 0x00a0, 0x000a, 0x00a0, 0x0000, 0x00e0, 0x000a, 0x00c0,
0x0003, 0x0040, 0x0005, 0x0080, 0x0003, 0x0080, 0x0005, 0x00a0, 0x0003, 0x0040, 0x0005, 0x00c0, 0x0003, 0x0080, 0x0005, 0x00e0,
0x0003, 0x0040, 0x0006, 0x0000, 0x0003, 0x0080, 0x0006, 0x0020, 0x0003, 0x0040, 0x0006, 0x0040, 0x0003, 0x0080, 0x0006, 0x0060,
0x0000, 0x00a0, 0x0009, 0x00e0, 0x0000, 0x00e0, 0x000a, 0x0000, 0x0000, 0x00a0, 0x000a, 0x0020, 0x0000, 0x00e0, 0x000a, 0x0040,
0x0000, 0x00a0, 0x000a, 0x0060, 0x0000, 0x00e0, 0x000a, 0x0080, 0x0000, 0x00a0, 0x000a, 0x00a0, 0x0000, 0x00e0, 0x000a, 0x00c0 };

static const int f1dream_2450_lookup[32] = {
0x0003, 0x0080, 0x0006, 0x0060, 0x0000, 0x00e0, 0x000a, 0x00c0, 0x0003, 0x0080, 0x0006, 0x0060, 0x0000, 0x00e0, 0x000a, 0x00c0,
0x0003, 0x0080, 0x0006, 0x0060, 0x0000, 0x00e0, 0x000a, 0x00c0, 0x0003, 0x0080, 0x0006, 0x0060, 0x0000, 0x00e0, 0x000a, 0x00c0 };

static void f1dream_protection_w(address_space *space)
{
	tigeroad_state *state = space->machine().driver_data<tigeroad_state>();
	int indx;
	int value = 255;
	int prevpc = cpu_get_previouspc(&space->device());

	if (prevpc == 0x244c)
	{
		/* Called once, when a race is started.*/
		indx = state->m_ram16[0x3ff0/2];
		state->m_ram16[0x3fe6/2] = f1dream_2450_lookup[indx];
		state->m_ram16[0x3fe8/2] = f1dream_2450_lookup[++indx];
		state->m_ram16[0x3fea/2] = f1dream_2450_lookup[++indx];
		state->m_ram16[0x3fec/2] = f1dream_2450_lookup[++indx];
	}
	else if (prevpc == 0x613a)
	{
		/* Called for every sprite on-screen.*/
		if (state->m_ram16[0x3ff6/2] < 15)
		{
			indx = f1dream_613ea_lookup[state->m_ram16[0x3ff6/2]] - state->m_ram16[0x3ff4/2];
			if (indx > 255)
			{
				indx <<= 4;
				indx += state->m_ram16[0x3ff6/2] & 0x00ff;
				value = f1dream_613eb_lookup[indx];
			}
		}

		state->m_ram16[0x3ff2/2] = value;
	}
	else if (prevpc == 0x17b70)
	{
		/* Called only before a real race, not a time trial.*/
		if (state->m_ram16[0x3ff0/2] >= 0x04) indx = 128;
		else if (state->m_ram16[0x3ff0/2] > 0x02) indx = 96;
		else if (state->m_ram16[0x3ff0/2] == 0x02) indx = 64;
		else if (state->m_ram16[0x3ff0/2] == 0x01) indx = 32;
		else indx = 0;

		indx += state->m_ram16[0x3fee/2];
		if (indx < 128)
		{
			state->m_ram16[0x3fe6/2] = f1dream_17b74_lookup[indx];
			state->m_ram16[0x3fe8/2] = f1dream_17b74_lookup[++indx];
			state->m_ram16[0x3fea/2] = f1dream_17b74_lookup[++indx];
			state->m_ram16[0x3fec/2] = f1dream_17b74_lookup[++indx];
		}
		else
		{
			state->m_ram16[0x3fe6/2] = 0x00ff;
			state->m_ram16[0x3fe8/2] = 0x00ff;
			state->m_ram16[0x3fea/2] = 0x00ff;
			state->m_ram16[0x3fec/2] = 0x00ff;
		}
	}
	else if ((prevpc == 0x27f8) || (prevpc == 0x511a) || (prevpc == 0x5142) || (prevpc == 0x516a))
	{
		/* The main CPU stuffs the byte for the soundlatch into 0xfffffd.*/
		state->soundlatch_w(*space,2,state->m_ram16[0x3ffc/2]);
	}
}

WRITE16_MEMBER(tigeroad_state::f1dream_control_w)
{
	logerror("protection write, PC: %04x  FFE1 Value:%01x\n",cpu_get_pc(&space.device()), m_ram16[0x3fe0/2]);
	f1dream_protection_w(&space);
}

WRITE16_MEMBER(tigeroad_state::tigeroad_soundcmd_w)
{
	if (ACCESSING_BITS_8_15)
		soundlatch_w(space,offset,data >> 8);
}

static WRITE8_DEVICE_HANDLER( msm5205_w )
{
	msm5205_reset_w(device,(data>>7)&1);
	msm5205_data_w(device,data);
	msm5205_vclk_w(device,1);
	msm5205_vclk_w(device,0);
}


/***************************************************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, tigeroad_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0xfe0800, 0xfe0cff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xfe0d00, 0xfe1807) AM_RAM		/* still part of OBJ RAM */
	AM_RANGE(0xfe4000, 0xfe4001) AM_READ_PORT("P1_P2") AM_WRITE(tigeroad_videoctrl_w)	/* char bank, coin counters, + ? */
	AM_RANGE(0xfe4002, 0xfe4003) AM_READ_PORT("SYSTEM")
/*  AM_RANGE(0xfe4002, 0xfe4003) AM_WRITE(tigeroad_soundcmd_w) added by init_tigeroad() */
	AM_RANGE(0xfe4004, 0xfe4005) AM_READ_PORT("DSW")
	AM_RANGE(0xfec000, 0xfec7ff) AM_RAM_WRITE(tigeroad_videoram_w) AM_BASE(m_videoram)
	AM_RANGE(0xfe8000, 0xfe8003) AM_WRITE(tigeroad_scroll_w)
	AM_RANGE(0xfe800e, 0xfe800f) AM_WRITEONLY    /* fe800e = watchdog or IRQ acknowledge */
	AM_RANGE(0xff8200, 0xff867f) AM_RAM_WRITE(paletteram16_xxxxRRRRGGGGBBBB_word_w) AM_SHARE("paletteram")
	AM_RANGE(0xffc000, 0xffffff) AM_RAM AM_BASE(m_ram16)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, tigeroad_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8001) AM_DEVREADWRITE_LEGACY("ym1", ym2203_r, ym2203_w)
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE_LEGACY("ym2", ym2203_r, ym2203_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_port_map, AS_IO, 8, tigeroad_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x7f, 0x7f) AM_WRITE(soundlatch2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sample_map, AS_PROGRAM, 8, tigeroad_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sample_port_map, AS_IO, 8, tigeroad_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch2_r)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE_LEGACY("msm", msm5205_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( tigeroad )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ))
	PORT_DIPSETTING(      0x0400, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x1800, "20000 70000 70000" )
	PORT_DIPSETTING(      0x1000, "20000 80000 80000" )
	PORT_DIPSETTING(      0x0800, "30000 80000 80000" )
	PORT_DIPSETTING(      0x0000, "30000 90000 90000" )
	PORT_DIPNAME( 0x6000, 0x6000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( toramich )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ))
	PORT_DIPSETTING(      0x0400, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x0800, "20000 70000 70000" )
	PORT_DIPSETTING(      0x0000, "20000 80000 80000" )
	PORT_DIPNAME( 0x1000, 0x1000, "Allow Level Select" )
	PORT_DIPSETTING(      0x1000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x6000, 0x6000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Difficult ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( f1dream )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ))
	PORT_DIPSETTING(      0x0400, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x1800, 0x1800, "F1 Up Point" )
	PORT_DIPSETTING(      0x1800, "12" )
	PORT_DIPSETTING(      0x1000, "16" )
	PORT_DIPSETTING(      0x0800, "18" )
	PORT_DIPSETTING(      0x0000, "20" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Version ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( World ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Japan ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Yes ) )
INPUT_PORTS_END



static const gfx_layout text_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tile_layout =
{
	32,32,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{
		0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
		64*8+0, 64*8+1, 64*8+2, 64*8+3, 64*8+8+0, 64*8+8+1, 64*8+8+2, 64*8+8+3,
		2*64*8+0, 2*64*8+1, 2*64*8+2, 2*64*8+3, 2*64*8+8+0, 2*64*8+8+1, 2*64*8+8+2, 2*64*8+8+3,
		3*64*8+0, 3*64*8+1, 3*64*8+2, 3*64*8+3, 3*64*8+8+0, 3*64*8+8+1, 3*64*8+8+2, 3*64*8+8+3,
	},
	{
		0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
		24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16
	},
	256*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static GFXDECODE_START( tigeroad )
	GFXDECODE_ENTRY( "text", 0, text_layout,   512, 16 )   /* colors 512-575 */
	GFXDECODE_ENTRY( "tiles", 0, tile_layout,     0, 16 )   /* colors   0-255 */
	GFXDECODE_ENTRY( "sprites", 0, sprite_layout, 256, 16 )   /* colors 256-511 */
GFXDECODE_END



/* handler called by the 2203 emulator when the internal timers cause an IRQ */
static void irqhandler(device_t *device, int irq)
{
	cputag_set_input_line(device->machine(), "audiocpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2203_interface ym2203_config =
{
	{
			AY8910_LEGACY_OUTPUT,
			AY8910_DEFAULT_LOADS,
			DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	},
	irqhandler
};

static const msm5205_interface msm5205_config =
{
	0,				/* interrupt function */
	MSM5205_SEX_4B	/* 4KHz playback ?  */
};


static MACHINE_CONFIG_START( tigeroad, tigeroad_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT("screen", irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_port_map)

	/* IRQs are triggered by the YM2203 */

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.08)   /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(tigeroad)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)

	MCFG_GFXDECODE(tigeroad)
	MCFG_PALETTE_LENGTH(576)

	MCFG_VIDEO_START(tigeroad)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_SOUND_CONFIG(ym2203_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ym2", YM2203, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


/* same as above but with additional Z80 for samples playback */
static MACHINE_CONFIG_DERIVED( toramich, tigeroad )

	/* basic machine hardware */

	MCFG_CPU_ADD("sample", Z80, 3579545) /* ? */
	MCFG_CPU_PROGRAM_MAP(sample_map)
	MCFG_CPU_IO_MAP(sample_port_map)
	MCFG_CPU_PERIODIC_INT(irq0_line_hold,4000)	/* ? */

	/* sound hardware */
	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_SOUND_CONFIG(msm5205_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tigeroad )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code */
	ROM_LOAD16_BYTE( "tru02.bin",    0x00000, 0x20000, CRC(8d283a95) SHA1(eb6c9225f79f62c22ae1e8980a557d896f598947) )
	ROM_LOAD16_BYTE( "tru04.bin",    0x00001, 0x20000, CRC(72e2ef20) SHA1(57ab7df2050042690ccfb1f2d170840f926dcf46) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "tru05.bin",    0x0000, 0x8000, CRC(f9a7c9bf) SHA1(4d37c71aa6523ac21c6e8b23f9957e75ec4304bf) )

	/* no samples player in the English version */

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "tr01.bin",     0x00000, 0x08000, CRC(74a9f08c) SHA1(458958c8d9a2af5df88bb24c9c5bcbd37d6856bc) ) /* 8x8 text */

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "tr-01a.bin",   0x00000, 0x20000, CRC(a8aa2e59) SHA1(792f50d688a4ffb574e41257816bc304d41f0458) ) /* tiles */
	ROM_LOAD( "tr-04a.bin",   0x20000, 0x20000, CRC(8863a63c) SHA1(11bfce5b09c5b8a781c658f035d5658c3710d189) )
	ROM_LOAD( "tr-02a.bin",   0x40000, 0x20000, CRC(1a2c5f89) SHA1(2a2aa2f1e2a0cdd4bbdb25236e49c7cc573db9e9) )
	ROM_LOAD( "tr05.bin",     0x60000, 0x20000, CRC(5bf453b3) SHA1(5eef151974c6b818a17756549d24a702e1f3a859) )
	ROM_LOAD( "tr-03a.bin",   0x80000, 0x20000, CRC(1e0537ea) SHA1(bc65f7104d5f7728b68b3dcb45151c41fc30aa0d) )
	ROM_LOAD( "tr-06a.bin",   0xa0000, 0x20000, CRC(b636c23a) SHA1(417e289745996bd00114df6ade591e702265d3a5) )
	ROM_LOAD( "tr-07a.bin",   0xc0000, 0x20000, CRC(5f907d4d) SHA1(1820c5c6e0b078db9c64655c7983ea115ad81036) )
	ROM_LOAD( "tr08.bin",     0xe0000, 0x20000, CRC(adee35e2) SHA1(6707cf43a697eb9465449a144ae4508afe2e6496) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "tr-09a.bin",   0x00000, 0x20000, CRC(3d98ad1e) SHA1(f12cdf50e1708ddae092b9784d4319a7d5f092bc) ) /* sprites */
	ROM_LOAD( "tr-10a.bin",   0x20000, 0x20000, CRC(8f6f03d7) SHA1(08a02cfb373040ea5ffbf5604f68df92a1338bb0) )
	ROM_LOAD( "tr-11a.bin",   0x40000, 0x20000, CRC(cd9152e5) SHA1(6df3c43c0c41289890296c2b2aeca915dfdae3b0) )
	ROM_LOAD( "tr-12a.bin",   0x60000, 0x20000, CRC(7d8a99d0) SHA1(af8221cfd2ce9aa3bf296981fb7fddd1e9ef4599) )

	ROM_REGION( 0x08000, "gfx4", 0 )	/* background tilemaps */
	ROM_LOAD( "tr13.bin",     0x0000, 0x8000, CRC(a79be1eb) SHA1(4191ccd48f7650930f9a4c2be0790239d7420bb1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "trprom.bin",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )	/* priority (not used) */
ROM_END

ROM_START( toramich )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code */
	ROM_LOAD16_BYTE( "tr_02.bin",    0x00000, 0x20000, CRC(b54723b1) SHA1(dfad82e96dff072c967dd59e3db71fb3b43b6dcb) )
	ROM_LOAD16_BYTE( "tr_04.bin",    0x00001, 0x20000, CRC(ab432479) SHA1(b8ec547f7bab67107a7c83931c7ed89142a7af69) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "tr_05.bin",    0x0000, 0x8000, CRC(3ebe6e62) SHA1(6f5708b6ff8c91bc706f73300e0785f15999d570) )

	ROM_REGION( 0x10000, "sample", 0 ) /* samples player */
	ROM_LOAD( "tr_03.bin",    0x0000, 0x10000, CRC(ea1807ef) SHA1(f856e7b592c6df81586821284ea2220468c5ea9d) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "tr01.bin",     0x00000, 0x08000, CRC(74a9f08c) SHA1(458958c8d9a2af5df88bb24c9c5bcbd37d6856bc) ) /* 8x8 text */

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "tr-01a.bin",   0x00000, 0x20000, CRC(a8aa2e59) SHA1(792f50d688a4ffb574e41257816bc304d41f0458) ) /* tiles */
	ROM_LOAD( "tr-04a.bin",   0x20000, 0x20000, CRC(8863a63c) SHA1(11bfce5b09c5b8a781c658f035d5658c3710d189) )
	ROM_LOAD( "tr-02a.bin",   0x40000, 0x20000, CRC(1a2c5f89) SHA1(2a2aa2f1e2a0cdd4bbdb25236e49c7cc573db9e9) )
	ROM_LOAD( "tr05.bin",     0x60000, 0x20000, CRC(5bf453b3) SHA1(5eef151974c6b818a17756549d24a702e1f3a859) )
	ROM_LOAD( "tr-03a.bin",   0x80000, 0x20000, CRC(1e0537ea) SHA1(bc65f7104d5f7728b68b3dcb45151c41fc30aa0d) )
	ROM_LOAD( "tr-06a.bin",   0xa0000, 0x20000, CRC(b636c23a) SHA1(417e289745996bd00114df6ade591e702265d3a5) )
	ROM_LOAD( "tr-07a.bin",   0xc0000, 0x20000, CRC(5f907d4d) SHA1(1820c5c6e0b078db9c64655c7983ea115ad81036) )
	ROM_LOAD( "tr08.bin",     0xe0000, 0x20000, CRC(adee35e2) SHA1(6707cf43a697eb9465449a144ae4508afe2e6496) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "tr-09a.bin",   0x00000, 0x20000, CRC(3d98ad1e) SHA1(f12cdf50e1708ddae092b9784d4319a7d5f092bc) ) /* sprites */
	ROM_LOAD( "tr-10a.bin",   0x20000, 0x20000, CRC(8f6f03d7) SHA1(08a02cfb373040ea5ffbf5604f68df92a1338bb0) )
	ROM_LOAD( "tr-11a.bin",   0x40000, 0x20000, CRC(cd9152e5) SHA1(6df3c43c0c41289890296c2b2aeca915dfdae3b0) )
	ROM_LOAD( "tr-12a.bin",   0x60000, 0x20000, CRC(7d8a99d0) SHA1(af8221cfd2ce9aa3bf296981fb7fddd1e9ef4599) )

	ROM_REGION( 0x08000, "gfx4", 0 )	/* background tilemaps */
	ROM_LOAD( "tr13.bin",     0x0000, 0x8000, CRC(a79be1eb) SHA1(4191ccd48f7650930f9a4c2be0790239d7420bb1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "trprom.bin",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )	/* priority (not used) */
ROM_END

ROM_START( tigeroadb )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code */
	ROM_LOAD16_BYTE( "tgrroad.3",    0x00000, 0x10000, CRC(14c87e07) SHA1(31363b56dd9d387f3ebd7ca1c209148c389ec1aa) )
	ROM_LOAD16_BYTE( "tgrroad.5",    0x00001, 0x10000, CRC(0904254c) SHA1(9ce7b8a699bc21618032db9b0c5494242ad77a6b) )
	ROM_LOAD16_BYTE( "tgrroad.2",    0x20000, 0x10000, CRC(cedb1f46) SHA1(bc2d5730ff809fb0f38327d72485d472ab9da54d) )
	ROM_LOAD16_BYTE( "tgrroad.4",    0x20001, 0x10000, CRC(e117f0b1) SHA1(ed0050247789bedaeb213c3d7c2d2cdb239bb4b4) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "tru05.bin",    0x0000, 0x8000, CRC(f9a7c9bf) SHA1(4d37c71aa6523ac21c6e8b23f9957e75ec4304bf) )

	/* no samples player in the English version */

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "tr01.bin",     0x00000, 0x08000, CRC(74a9f08c) SHA1(458958c8d9a2af5df88bb24c9c5bcbd37d6856bc) ) /* 8x8 text */

	ROM_REGION( 0x100000, "tiles", 0 )
	ROM_LOAD( "tr-01a.bin",   0x00000, 0x20000, CRC(a8aa2e59) SHA1(792f50d688a4ffb574e41257816bc304d41f0458) ) /* tiles */
	ROM_LOAD( "tr-04a.bin",   0x20000, 0x20000, CRC(8863a63c) SHA1(11bfce5b09c5b8a781c658f035d5658c3710d189) )
	ROM_LOAD( "tr-02a.bin",   0x40000, 0x20000, CRC(1a2c5f89) SHA1(2a2aa2f1e2a0cdd4bbdb25236e49c7cc573db9e9) )
	ROM_LOAD( "tr05.bin",     0x60000, 0x20000, CRC(5bf453b3) SHA1(5eef151974c6b818a17756549d24a702e1f3a859) )
	ROM_LOAD( "tr-03a.bin",   0x80000, 0x20000, CRC(1e0537ea) SHA1(bc65f7104d5f7728b68b3dcb45151c41fc30aa0d) )
	ROM_LOAD( "tr-06a.bin",   0xa0000, 0x20000, CRC(b636c23a) SHA1(417e289745996bd00114df6ade591e702265d3a5) )
	ROM_LOAD( "tr-07a.bin",   0xc0000, 0x20000, CRC(5f907d4d) SHA1(1820c5c6e0b078db9c64655c7983ea115ad81036) )
	ROM_LOAD( "tgrroad.17",   0xe0000, 0x10000, CRC(3f7539cc) SHA1(ca3ef1fabcb0c7abd7bc211ba128d2433e3dbf26) )
	ROM_LOAD( "tgrroad.18",   0xf0000, 0x10000, CRC(e2e053cb) SHA1(eb9432140fc167dec5d3273112933201be2be1b3) )

	ROM_REGION( 0x080000, "sprites", 0 )
	ROM_LOAD( "tr-09a.bin",   0x00000, 0x20000, CRC(3d98ad1e) SHA1(f12cdf50e1708ddae092b9784d4319a7d5f092bc) ) /* sprites */
	ROM_LOAD( "tr-10a.bin",   0x20000, 0x20000, CRC(8f6f03d7) SHA1(08a02cfb373040ea5ffbf5604f68df92a1338bb0) )
	ROM_LOAD( "tr-11a.bin",   0x40000, 0x20000, CRC(cd9152e5) SHA1(6df3c43c0c41289890296c2b2aeca915dfdae3b0) )
	ROM_LOAD( "tr-12a.bin",   0x60000, 0x20000, CRC(7d8a99d0) SHA1(af8221cfd2ce9aa3bf296981fb7fddd1e9ef4599) )

	ROM_REGION( 0x08000, "gfx4", 0 )	/* background tilemaps */
	ROM_LOAD( "tr13.bin",     0x0000, 0x8000, CRC(a79be1eb) SHA1(4191ccd48f7650930f9a4c2be0790239d7420bb1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "trprom.bin",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )	/* priority (not used) */
ROM_END

ROM_START( f1dream )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code */
	ROM_LOAD16_BYTE( "06j_02.bin",   0x00000, 0x20000, CRC(3c2ec697) SHA1(bccb431ad92455484420f91770e91db6d69b09ec) )
	ROM_LOAD16_BYTE( "06k_03.bin",   0x00001, 0x20000, CRC(85ebad91) SHA1(000f5c617417ff20ee9b378166776fecfacdff95) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "12k_04.bin",   0x0000, 0x8000, CRC(4b9a7524) SHA1(19004958c19ac0af35f2c97790b0082ee2c15bc4) )

	ROM_REGION( 0x1000, "mcu", 0 )	/* i8751 microcontroller */
	ROM_LOAD( "c8751h-88",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "10d_01.bin",   0x00000, 0x08000, CRC(361caf00) SHA1(8a109e4e116d0c5eea86f9c57c05359754daa5b9) ) /* 8x8 text */

	ROM_REGION( 0x060000, "tiles", 0 )
	ROM_LOAD( "03f_12.bin",   0x00000, 0x10000, CRC(bc13e43c) SHA1(f9528839858d7a45395062a43b71d80400c73173) ) /* tiles */
	ROM_LOAD( "01f_10.bin",   0x10000, 0x10000, CRC(f7617ad9) SHA1(746a0ec433d5246ac4dbae17d6498e3d154e2df1) )
	ROM_LOAD( "03h_14.bin",   0x20000, 0x10000, CRC(e33cd438) SHA1(89a6faea19e8a01b38ba45413609603e559877e9) )
	ROM_LOAD( "02f_11.bin",   0x30000, 0x10000, CRC(4aa49cd7) SHA1(b7052d51a3cb570299f4db1492a1293c4d8b067f) )
	ROM_LOAD( "17f_09.bin",   0x40000, 0x10000, CRC(ca622155) SHA1(00ae4a8e9cad2c42a10b410b594b0e414ada6cfe) )
	ROM_LOAD( "02h_13.bin",   0x50000, 0x10000, CRC(2a63961e) SHA1(a35e9bf0408716f460487a8d2ae336572a98d2fb) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "03b_06.bin",   0x00000, 0x10000, CRC(5e54e391) SHA1(475c968bfeb41b0448e621f59724c7b70d184d36) ) /* sprites */
	ROM_LOAD( "02b_05.bin",   0x10000, 0x10000, CRC(cdd119fd) SHA1(e279ada53f5a1e2ada0195b93399731af213f518) )
	ROM_LOAD( "03d_08.bin",   0x20000, 0x10000, CRC(811f2e22) SHA1(cca7e8cc43408c2c3067a731a98a8a6418a000aa) )
	ROM_LOAD( "02d_07.bin",   0x30000, 0x10000, CRC(aa9a1233) SHA1(c2079ad81d67b54483ea5f69ac2edf276ad58ca9) )

	ROM_REGION( 0x08000, "gfx4", 0 )	/* background tilemaps */
	ROM_LOAD( "07l_15.bin",   0x0000, 0x8000, CRC(978758b7) SHA1(ebd415d70e2f1af3b1bd51f40e7d60f22369638c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "09e_tr.bin",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )	/* priority (not used) */
ROM_END

ROM_START( f1dreamb )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 256K for 68000 code */
	ROM_LOAD16_BYTE( "f1d_04.bin",   0x00000, 0x10000, CRC(903febad) SHA1(73726b220ce45e1f13798e50fb6455671f1150f3) )
	ROM_LOAD16_BYTE( "f1d_05.bin",   0x00001, 0x10000, CRC(666fa2a7) SHA1(f38e71293368ddc586f437c38ced1d8ce91527ea) )
	ROM_LOAD16_BYTE( "f1d_02.bin",   0x20000, 0x10000, CRC(98973c4c) SHA1(a73d396a1c3e43e6250d9e0ab1902d6f754d1ed9) )
	ROM_LOAD16_BYTE( "f1d_03.bin",   0x20001, 0x10000, CRC(3d21c78a) SHA1(edee180131a5b4d507ce0490fd3890bdd03ce62f) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "12k_04.bin",   0x0000, 0x8000, CRC(4b9a7524) SHA1(19004958c19ac0af35f2c97790b0082ee2c15bc4) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "10d_01.bin",   0x00000, 0x08000, CRC(361caf00) SHA1(8a109e4e116d0c5eea86f9c57c05359754daa5b9) ) /* 8x8 text */

	ROM_REGION( 0x060000, "tiles", 0 )
	ROM_LOAD( "03f_12.bin",   0x00000, 0x10000, CRC(bc13e43c) SHA1(f9528839858d7a45395062a43b71d80400c73173) ) /* tiles */
	ROM_LOAD( "01f_10.bin",   0x10000, 0x10000, CRC(f7617ad9) SHA1(746a0ec433d5246ac4dbae17d6498e3d154e2df1) )
	ROM_LOAD( "03h_14.bin",   0x20000, 0x10000, CRC(e33cd438) SHA1(89a6faea19e8a01b38ba45413609603e559877e9) )
	ROM_LOAD( "02f_11.bin",   0x30000, 0x10000, CRC(4aa49cd7) SHA1(b7052d51a3cb570299f4db1492a1293c4d8b067f) )
	ROM_LOAD( "17f_09.bin",   0x40000, 0x10000, CRC(ca622155) SHA1(00ae4a8e9cad2c42a10b410b594b0e414ada6cfe) )
	ROM_LOAD( "02h_13.bin",   0x50000, 0x10000, CRC(2a63961e) SHA1(a35e9bf0408716f460487a8d2ae336572a98d2fb) )

	ROM_REGION( 0x040000, "sprites", 0 )
	ROM_LOAD( "03b_06.bin",   0x00000, 0x10000, CRC(5e54e391) SHA1(475c968bfeb41b0448e621f59724c7b70d184d36) ) /* sprites */
	ROM_LOAD( "02b_05.bin",   0x10000, 0x10000, CRC(cdd119fd) SHA1(e279ada53f5a1e2ada0195b93399731af213f518) )
	ROM_LOAD( "03d_08.bin",   0x20000, 0x10000, CRC(811f2e22) SHA1(cca7e8cc43408c2c3067a731a98a8a6418a000aa) )
	ROM_LOAD( "02d_07.bin",   0x30000, 0x10000, CRC(aa9a1233) SHA1(c2079ad81d67b54483ea5f69ac2edf276ad58ca9) )

	ROM_REGION( 0x08000, "gfx4", 0 )	/* background tilemaps */
	ROM_LOAD( "07l_15.bin",   0x0000, 0x8000, CRC(978758b7) SHA1(ebd415d70e2f1af3b1bd51f40e7d60f22369638c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "09e_tr.bin",   0x0000, 0x0100, CRC(ec80ae36) SHA1(397ec8fc1b106c8b8d4bf6798aa429e8768a101a) )	/* priority (not used) */
ROM_END



static DRIVER_INIT( tigeroad )
{
	tigeroad_state *state = machine.driver_data<tigeroad_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0xfe4002, 0xfe4003, write16_delegate(FUNC(tigeroad_state::tigeroad_soundcmd_w),state));
}

static DRIVER_INIT( f1dream )
{
	tigeroad_state *state = machine.driver_data<tigeroad_state>();
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_write_handler(0xfe4002, 0xfe4003, write16_delegate(FUNC(tigeroad_state::f1dream_control_w),state));
}



GAME( 1987, tigeroad, 0,        tigeroad, tigeroad, tigeroad, ROT0, "Capcom (Romstar license)", "Tiger Road (US)", 0 )
GAME( 1987, toramich, tigeroad, toramich, toramich, tigeroad, ROT0, "Capcom", "Tora-he no Michi (Japan)", 0 )
GAME( 1987, tigeroadb,tigeroad, tigeroad, tigeroad, tigeroad, ROT0, "bootleg", "Tiger Road (US bootleg)", 0 )

/* F1 Dream has an Intel 8751 microcontroller for protection */
GAME( 1988, f1dream,  0,        tigeroad, f1dream,  f1dream,  ROT0, "Capcom (Romstar license)", "F-1 Dream", 0 )
GAME( 1988, f1dreamb, f1dream,  tigeroad, f1dream,  tigeroad, ROT0, "bootleg", "F-1 Dream (bootleg)", 0 )
