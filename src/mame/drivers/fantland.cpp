// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************************

                      -= Electronic Devices / International Games =-

                    driver by   Luca Elia (l.elia@tin.it)


CPUs    :   2 x 8086? / V20 + 8088
Sound   :   YM2151 [+ DAC] / 4 x MSM5205
Video   :   2 x I.G.1BB 48844758V

----------------------------------------------------------------------------------------
Year + Game             Main CPU    Sound CPU    Sound            Video
----------------------------------------------------------------------------------------
>=1987  Born To Fight   V20         8088         4 x MSM5205      2 x I.G.1BB 48844758V
>=1987  Fantasy Land    8086?       8086?        YM2151 + DAC     ?
1988?   Wheels Runner   V20         Z80          YM3526 + ?       2 x PLCC84 FPGA
1989    Galaxy Gunners  8086?       8086?        YM2151           ?
----------------------------------------------------------------------------------------

[fantland, galaxygn]

- Clocks are unknown and the cpu might be an 8088 or a later x86.

[fantland, borntofi]

- The year of creation isn't displayed, but it's no sooner than 1987 since
  embedded in the roms is: "MS Run-Time Library - Copyright (c) 1987, Microsoft Corp"

[fantland]

- Slowdowns on the ice level's boss

[borntofi]

- Verify sound. Also speech is a bit garbled / low volume (rom 15)
- Trackball controls don't work well

[wheelrun]

- On a car hit / crash the game tries to produce sound through ports b000 & c000,
  probably connected to the EP1210. This is not emulated.

***************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/nec/nec.h"
#include "cpu/i86/i86.h"
#include "sound/2151intf.h"
#include "sound/3526intf.h"
#include "sound/dac.h"
#include "sound/msm5205.h"
#include "includes/fantland.h"

/***************************************************************************

                            Memory Maps - Main CPU

***************************************************************************/

WRITE8_MEMBER(fantland_state::fantland_nmi_enable_w)
{
	m_nmi_enable = data;

	if ((m_nmi_enable != 0) && (m_nmi_enable != 8))
		logerror("CPU #0 PC = %04X: nmi_enable = %02x\n", space.device().safe_pc(), data);
}

WRITE16_MEMBER(fantland_state::fantland_nmi_enable_16_w)
{
	if (ACCESSING_BITS_0_7)
		fantland_nmi_enable_w(space, offset * 2, data);
}

WRITE8_MEMBER(fantland_state::fantland_soundlatch_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE16_MEMBER(fantland_state::fantland_soundlatch_16_w)
{
	if (ACCESSING_BITS_0_7)
		fantland_soundlatch_w(space, offset * 2, data);
}

/***************************************************************************
                                Fantasy Land
***************************************************************************/

READ16_MEMBER(fantland_state::spriteram_16_r)
{
	UINT8 *spriteram = m_spriteram;
	return spriteram[2 * offset + 0] | (spriteram[2 * offset + 1] << 8);
}

READ16_MEMBER(fantland_state::spriteram2_16_r)
{
	UINT8 *spriteram_2 = m_spriteram2;
	return spriteram_2[2 * offset + 0] | (spriteram_2[2 * offset + 1] << 8);
}

WRITE16_MEMBER(fantland_state::spriteram_16_w)
{
	UINT8 *spriteram = m_spriteram;
	if (ACCESSING_BITS_0_7)
		spriteram[2 * offset + 0] = data;
	if (ACCESSING_BITS_8_15)
		spriteram[2 * offset + 1] = data >> 8;
}

WRITE16_MEMBER(fantland_state::spriteram2_16_w)
{
	UINT8 *spriteram_2 = m_spriteram2;
	if (ACCESSING_BITS_0_7)
		spriteram_2[2 * offset + 0] = data;
	if (ACCESSING_BITS_8_15)
		spriteram_2[2 * offset + 1] = data >> 8;
}

static ADDRESS_MAP_START( fantland_map, AS_PROGRAM, 16, fantland_state )
	AM_RANGE( 0x00000, 0x07fff ) AM_RAM
	AM_RANGE( 0x08000, 0x7ffff ) AM_ROM

	AM_RANGE( 0xa2000, 0xa21ff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE( 0xa3000, 0xa3001 ) AM_READ_PORT("a3000") AM_WRITE(fantland_nmi_enable_16_w )
	AM_RANGE( 0xa3002, 0xa3003 ) AM_READ_PORT("a3002") AM_WRITE(fantland_soundlatch_16_w )

	AM_RANGE( 0xa4000, 0xa67ff ) AM_READWRITE(spriteram_16_r,  spriteram_16_w  ) AM_SHARE("spriteram")
	AM_RANGE( 0xc0000, 0xcffff ) AM_READWRITE(spriteram2_16_r, spriteram2_16_w ) AM_SHARE("spriteram2")

	AM_RANGE( 0xe0000, 0xfffff ) AM_ROM
ADDRESS_MAP_END


/***************************************************************************
                                Galaxy Gunners
***************************************************************************/

static ADDRESS_MAP_START( galaxygn_map, AS_PROGRAM, 8, fantland_state )
	AM_RANGE( 0x00000, 0x07fff ) AM_RAM
	AM_RANGE( 0x10000, 0x2ffff ) AM_ROM

	AM_RANGE( 0x52000, 0x521ff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE( 0x53000, 0x53000 ) AM_READ_PORT("P1") AM_WRITE(fantland_nmi_enable_w )
	AM_RANGE( 0x53001, 0x53001 ) AM_READ_PORT("P2")
	AM_RANGE( 0x53002, 0x53002 ) AM_READ_PORT("DSW1") AM_WRITE(fantland_soundlatch_w )
	AM_RANGE( 0x53003, 0x53003 ) AM_READ_PORT("DSW2")

	AM_RANGE( 0x54000, 0x567ff ) AM_RAM AM_SHARE("spriteram")
	AM_RANGE( 0x60000, 0x6ffff ) AM_RAM AM_SHARE("spriteram2")

	AM_RANGE( 0x70000, 0x7ffff ) AM_ROM
	AM_RANGE( 0xf0000, 0xfffff ) AM_ROM
ADDRESS_MAP_END


/***************************************************************************
                           Born To Fight
***************************************************************************/

WRITE8_MEMBER(fantland_state::borntofi_nmi_enable_w)
{
	m_nmi_enable = data;

	// data & 0x31 changes when lightgun fires

	if ((m_nmi_enable != 0) && (m_nmi_enable != 8))
		logerror("CPU #0 PC = %04X: nmi_enable = %02x\n", space.device().safe_pc(), data);

//  popmessage("%02X", data);
}

// Trackball doesn't work correctly
READ8_MEMBER(fantland_state::borntofi_inputs_r)
{
	int x, y, f;

	switch (ioport("Controls")->read() & 0x03)
	{
		case 3:
		case 1: return ioport(offset ? "P2_GUN" : "P1_GUN")->read();    // Lightgun buttons
		case 2: return ioport(offset ? "P2_JOY" : "P1_JOY")->read();    // Joystick
	}

	// Trackball

	x = ioport(offset ? "P2 Trackball X" : "P1 Trackball X")->read();
	y = ioport(offset ? "P2 Trackball Y" : "P1 Trackball Y")->read();
	f = m_screen->frame_number();

	m_input_ret[offset] = (m_input_ret[offset] & 0x14) | (ioport(offset ? "P2_TRACK" : "P1_TRACK")->read() & 0xc3);

	x = (x & 0x7f) - (x & 0x80);
	y = (y & 0x7f) - (y & 0x80);

	if (m_old_x[offset] > 0)
	{
		m_input_ret[offset] = (m_input_ret[offset] ^ 0x04) | ((m_input_ret[offset] & 0x04) << 1);
		m_old_x[offset]--;
	}
	else if (m_old_x[offset] < 0)
	{
		m_input_ret[offset] = (m_input_ret[offset] ^ 0x04) | (((~m_input_ret[offset]) & 0x04) << 1);
		m_old_x[offset]++;
	}

	if (m_old_y[offset] > 0)
	{
		m_input_ret[offset] = (m_input_ret[offset] ^ 0x10) | ((m_input_ret[offset] & 0x10) << 1);
		m_old_y[offset]--;
	}
	else if (m_old_y[offset] < 0)
	{
		m_input_ret[offset] = (m_input_ret[offset] ^ 0x10) | (((~m_input_ret[offset]) & 0x10) << 1);
		m_old_y[offset]++;
	}

//  if (offset == 0)    popmessage("x %02d y %02d", m_old_x[offset], m_old_y[offset]);

	if ((f - m_old_f[offset]) > 0)
	{
		m_old_x[offset] = x;
		m_old_y[offset] = y;
		m_old_f[offset] = f;
	}

	return m_input_ret[offset];
}

static ADDRESS_MAP_START( borntofi_map, AS_PROGRAM, 8, fantland_state )
	AM_RANGE( 0x00000, 0x07fff ) AM_RAM
	AM_RANGE( 0x10000, 0x2ffff ) AM_ROM

	AM_RANGE( 0x52000, 0x521ff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE( 0x53000, 0x53001 ) AM_READWRITE(borntofi_inputs_r, borntofi_nmi_enable_w )
	AM_RANGE( 0x53002, 0x53002 ) AM_READ_PORT( "DSW" ) AM_WRITE(fantland_soundlatch_w )
	AM_RANGE( 0x53003, 0x53003 ) AM_READ_PORT( "Controls" )

	AM_RANGE( 0x54000, 0x567ff ) AM_RAM AM_SHARE("spriteram")

	AM_RANGE( 0x57000, 0x57000 ) AM_READ_PORT( "P1 Lightgun Y" )
	AM_RANGE( 0x57001, 0x57001 ) AM_READ_PORT( "P1 Lightgun X" )
	AM_RANGE( 0x57002, 0x57002 ) AM_READ_PORT( "P2 Lightgun Y" )
	AM_RANGE( 0x57003, 0x57003 ) AM_READ_PORT( "P2 Lightgun X" )

	AM_RANGE( 0x60000, 0x6ffff ) AM_RAM AM_SHARE("spriteram2")

	AM_RANGE( 0x70000, 0x7ffff ) AM_ROM
	AM_RANGE( 0xf0000, 0xfffff ) AM_ROM
ADDRESS_MAP_END


/***************************************************************************
                           Wheels Runner
***************************************************************************/

static ADDRESS_MAP_START( wheelrun_map, AS_PROGRAM, 8, fantland_state )
	AM_RANGE(0x00000, 0x07fff) AM_RAM

	AM_RANGE(0x30000, 0x3ffff) AM_ROM
	AM_RANGE(0x70000, 0x7ffff) AM_ROM

	AM_RANGE(0x52000, 0x521ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")

	AM_RANGE(0x53000, 0x53000) AM_READ_PORT( "53000" ) AM_WRITE(borntofi_nmi_enable_w )
	AM_RANGE(0x53001, 0x53001) AM_READ_PORT( "53001" )
	AM_RANGE(0x53002, 0x53002) AM_READ_PORT( "53002" ) AM_WRITE(fantland_soundlatch_w )
	AM_RANGE(0x53003, 0x53003) AM_READ_PORT( "53003" ) AM_WRITENOP

	AM_RANGE(0x54000, 0x567ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x60000, 0x6ffff) AM_RAM AM_SHARE("spriteram2")

	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END



/***************************************************************************

                            Memory Maps - Sound CPU

***************************************************************************/

static ADDRESS_MAP_START( fantland_sound_map, AS_PROGRAM, 8, fantland_state )
	AM_RANGE( 0x00000, 0x01fff ) AM_RAM
	AM_RANGE( 0x80000, 0x9ffff ) AM_ROM
	AM_RANGE( 0xc0000, 0xfffff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fantland_sound_iomap, AS_IO, 8, fantland_state )
	AM_RANGE( 0x0080, 0x0080 ) AM_READ(soundlatch_byte_r )
	AM_RANGE( 0x0100, 0x0101 ) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE( 0x0180, 0x0180 ) AM_DEVWRITE("dac", dac_device, write_unsigned8 )
ADDRESS_MAP_END

static ADDRESS_MAP_START( galaxygn_sound_iomap, AS_IO, 8, fantland_state )
	AM_RANGE( 0x0080, 0x0080 ) AM_READ(soundlatch_byte_r )
	AM_RANGE( 0x0100, 0x0101 ) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
ADDRESS_MAP_END


/***************************************************************************
                           Born To Fight
***************************************************************************/

void fantland_state::borntofi_adpcm_start( msm5205_device *device, int voice )
{
	device->reset_w(0);
	m_adpcm_playing[voice] = 1;
	m_adpcm_nibble[voice] = 0;
//  logerror("%s: adpcm start = %06x, stop = %06x\n", device->machine().describe_context(), m_adpcm_addr[0][voice], m_adpcm_addr[1][voice]);
}

void fantland_state::borntofi_adpcm_stop( msm5205_device *device, int voice )
{
	device->reset_w(1);
	m_adpcm_playing[voice] = 0;
}

WRITE8_MEMBER(fantland_state::borntofi_msm5205_w)
{
	int voice = offset / 8;
	int reg = offset % 8;
	msm5205_device *msm;

	switch (voice)
	{
		default:
		case 0: msm = m_msm1; break;
		case 1: msm = m_msm2; break;
		case 2: msm = m_msm3; break;
		case 3: msm = m_msm4; break;
	}

	if (reg == 0)
	{
		// Play / Stop
		switch(data)
		{
			case 0x00:      borntofi_adpcm_stop(msm, voice); break;
			case 0x03:      borntofi_adpcm_start(msm, voice); break;
			default:        logerror("CPU #0 PC = %04X: adpcm reg %d <- %02x\n", space.device().safe_pc(), reg, data);
		}
	}
	else
	{
		int shift = (reg - 1) * 4;
		int mask = ~(0xf << shift);

		m_adpcm_addr[0][voice] = (m_adpcm_addr[0][voice] & mask) | (((data & 0xf0) >> 4) << shift);
		m_adpcm_addr[1][voice] = (m_adpcm_addr[1][voice] & mask) | (((data & 0x0f) >> 0) << shift);
	}
}

void fantland_state::borntofi_adpcm_int( msm5205_device *device, int voice )
{
	UINT8 *rom;
	size_t len;
	int start, stop;

	if (!m_adpcm_playing[voice])
		return;

	rom = memregion("adpcm")->base();
	len = memregion("adpcm")->bytes() * 2;

	start = m_adpcm_addr[0][voice] + m_adpcm_nibble[voice];
	stop = m_adpcm_addr[1][voice];

	if (start >= len)
	{
		borntofi_adpcm_stop(device, voice);
		logerror("adpcm address out of range: %06x\n", start);
		return;
	}

	if (start >= stop)
	{
		borntofi_adpcm_stop(device, voice);
	}
	else
	{
		device->data_w(rom[start / 2] >> ((start & 1) * 4));
		m_adpcm_nibble[voice]++;
	}
}

WRITE_LINE_MEMBER(fantland_state::borntofi_adpcm_int_0) { borntofi_adpcm_int(m_msm1, 0); }
WRITE_LINE_MEMBER(fantland_state::borntofi_adpcm_int_1) { borntofi_adpcm_int(m_msm2, 1); }
WRITE_LINE_MEMBER(fantland_state::borntofi_adpcm_int_2) { borntofi_adpcm_int(m_msm3, 2); }
WRITE_LINE_MEMBER(fantland_state::borntofi_adpcm_int_3) { borntofi_adpcm_int(m_msm4, 3); }


static ADDRESS_MAP_START( borntofi_sound_map, AS_PROGRAM, 8, fantland_state )
	AM_RANGE( 0x00000, 0x003ff ) AM_RAM
	AM_RANGE( 0x04000, 0x04000 ) AM_READ(soundlatch_byte_r)
	AM_RANGE( 0x04000, 0x0401f ) AM_WRITE(borntofi_msm5205_w)
	AM_RANGE( 0x08000, 0x0ffff ) AM_ROM
	AM_RANGE( 0xf8000, 0xfffff ) AM_ROM
ADDRESS_MAP_END


/***************************************************************************
                           Wheels Runner
***************************************************************************/

static ADDRESS_MAP_START( wheelrun_sound_map, AS_PROGRAM, 8, fantland_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE("ymsnd", ym3526_device, read, write)

	AM_RANGE(0xb000, 0xb000) AM_WRITENOP    // on a car crash / hit
	AM_RANGE(0xc000, 0xc000) AM_WRITENOP    // ""

	AM_RANGE(0xd000, 0xd000) AM_READ(soundlatch_byte_r )    // during NMI
ADDRESS_MAP_END



/***************************************************************************

                                Input Ports

***************************************************************************/

/***************************************************************************
                                Fantasy Land
***************************************************************************/

static INPUT_PORTS_START( fantland )
	PORT_START("a3000") /* a3000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON2 )

	/* a3001 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)   // used in test mode only
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("a3002") /* a3002 */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0000, "Invulnerability" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Hardest ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	/* a3003 */
	PORT_DIPNAME( 0x0100, 0x0100, "Test Sound" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e00, 0x0c00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0e00, "1" )
	PORT_DIPSETTING(    0x0c00, "2" )
	PORT_DIPSETTING(    0x0a00, "3" )
	PORT_DIPSETTING(    0x0800, "4" )
	PORT_DIPSETTING(    0x0600, "5" )
	PORT_DIPSETTING(    0x0400, "6" )
	PORT_DIPSETTING(    0x0200, "7" )
	PORT_DIPSETTING(    0x0000, "8" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x3000, "800k" )
	PORT_DIPSETTING(    0x2000, "1600k" )
	PORT_DIPSETTING(    0x1000, "2400k" )
	PORT_DIPSETTING(    0x0000, "3200k" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )  //unused?
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )  //unused?
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************
                                Galaxy Gunners
***************************************************************************/

static INPUT_PORTS_START( galaxygn )
	PORT_START("P1")    /* 53000 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("P2")    /* 53001 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("DSW1")  /* 53002 */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  // Demo Sounds? doesn't work
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  // Allow Continue? doesn't work
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")      /* 53003 */
	PORT_DIPNAME( 0x01, 0x01, "Test Sound" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0e, "1" )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x0a, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x06, "5" )
	PORT_DIPSETTING(    0x04, "6" )
	PORT_DIPSETTING(    0x02, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "10k" )
	PORT_DIPSETTING(    0x20, "20k" )
	PORT_DIPSETTING(    0x10, "30k" )
	PORT_DIPSETTING(    0x00, "40k" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  //unused?
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  //unused?
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************
                           Born To Fight
***************************************************************************/

static INPUT_PORTS_START( borntofi )
	PORT_START("P1_GUN")    /* 53000 (Lightgun) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )  PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)

	PORT_START("P2_GUN")    /* 53001 (Lightgun) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )  PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 ) PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x03) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x03) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x03)

	PORT_START("P1_TRACK")  /* 53000 (Trackball) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_CONDITION("Controls", 0x03, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_START1 )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )   PORT_CONDITION("Controls", 0x03, EQUALS, 0x00) // trackball x
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL )   PORT_CONDITION("Controls", 0x03, EQUALS, 0x00) // trackball x
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL )   PORT_CONDITION("Controls", 0x03, EQUALS, 0x00) // trackball y
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )   PORT_CONDITION("Controls", 0x03, EQUALS, 0x00) // trackball y
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON2 )   PORT_CONDITION("Controls", 0x03, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON1 )   PORT_CONDITION("Controls", 0x03, EQUALS, 0x00)

	PORT_START("P2_TRACK")  /* 53001 (Trackball) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN2 ) PORT_CONDITION("Controls", 0x03, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_START2 )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )   PORT_CONDITION("Controls", 0x03, EQUALS, 0x00) // trackball x
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL )   PORT_CONDITION("Controls", 0x03, EQUALS, 0x00) // trackball x
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL )   PORT_CONDITION("Controls", 0x03, EQUALS, 0x00) // trackball y
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )   PORT_CONDITION("Controls", 0x03, EQUALS, 0x00) // trackball y
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON2 )   PORT_CONDITION("Controls", 0x03, EQUALS, 0x00) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON1 )   PORT_CONDITION("Controls", 0x03, EQUALS, 0x00) PORT_PLAYER(2)

	PORT_START("P1_JOY")    /* 53000 (Joystick) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )  PORT_CONDITION("Controls", 0x03, EQUALS, 0x02)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) PORT_CONDITION("Controls", 0x03, EQUALS, 0x02)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x02)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_CONDITION("Controls", 0x03, EQUALS, 0x02)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_CONDITION("Controls", 0x03, EQUALS, 0x02)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CONDITION("Controls", 0x03, EQUALS, 0x02)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x02)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x02)

	PORT_START("P2_JOY")    /* 53001 (Joystick) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )  PORT_CONDITION("Controls", 0x03, EQUALS, 0x02)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 ) PORT_CONDITION("Controls", 0x03, EQUALS, 0x02)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x02) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_CONDITION("Controls", 0x03, EQUALS, 0x02) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_CONDITION("Controls", 0x03, EQUALS, 0x02) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CONDITION("Controls", 0x03, EQUALS, 0x02) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x02) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )    PORT_CONDITION("Controls", 0x03, EQUALS, 0x02) PORT_PLAYER(2)

	PORT_START("DSW")   /* 53002 */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("DSW1:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium_Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

//  PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START( "Controls" )    /* IN7 - 53003 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Controls ) )     PORT_DIPLOCATION("DSW2:1,2")
//  PORT_DIPSETTING(    0x01, "Lightgun" )
	PORT_DIPSETTING(    0x03, "Lightgun" )
	PORT_DIPSETTING(    0x00, DEF_STR( Trackball ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x04, 0x04, "Sound Test" )        PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Free Bullets" )      PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x0010, "DSW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x0020, "DSW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x0040, "DSW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x0080, "DSW2:8" )

	PORT_START("P1 Lightgun Y")     /* 57000 */
	PORT_BIT( 0xff, 0xb0, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, (352.0 - 12) / 352, 12.0 / 352, 0) PORT_MINMAX(0x80,0xfc) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_PLAYER(1)

	PORT_START("P1 Lightgun X")     /* 57001 */
	PORT_BIT( 0xff, 0x60, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x07,0xb7) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_PLAYER(1)

	PORT_START("P2 Lightgun Y")     /* 57002 */
	PORT_BIT( 0xff, 0xb0, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, (352.0 - 12) / 352, 12.0 / 352, 0) PORT_MINMAX(0x80,0xfc) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START("P2 Lightgun X")     /* 57003 */
	PORT_BIT( 0xff, 0x70, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x07,0xb7) PORT_SENSITIVITY(100) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START("P1 Trackball Y")    /* 53000 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(5) PORT_PLAYER(1) PORT_RESET

	PORT_START("P1 Trackball X")    /* 53000 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(5) PORT_PLAYER(1) PORT_RESET

	PORT_START("P2 Trackball Y")    /* 53001 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START("P2 Trackball X")    /* 53001 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(5) PORT_PLAYER(2)
INPUT_PORTS_END


/***************************************************************************
                           Wheels Runner
***************************************************************************/

CUSTOM_INPUT_MEMBER(fantland_state::wheelrun_wheel_r)
{
	int player = (FPTR)param;
	int delta = ioport(player ? "WHEEL1" : "WHEEL0")->read();
	delta = (delta & 0x7f) - (delta & 0x80) + 4;

	if      (delta > 7) delta = 7;
	else if (delta < 1) delta = 1;

//  if (player == 0)    popmessage("%x",delta);
	return delta;
}

static INPUT_PORTS_START( wheelrun )
	PORT_START("53000")     /* 53000 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, fantland_state,wheelrun_wheel_r, (void *)nullptr)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("53001")     /* 53001 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, fantland_state,wheelrun_wheel_r, (void *)1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("53002")     /* 53002 */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("53003")     /* 53003 */
	PORT_DIPNAME( 0xff, 0xdf, "Wheel Sensitivity" )
	PORT_DIPSETTING(    0x7f, "0" )
	PORT_DIPSETTING(    0xbf, "1" )
	PORT_DIPSETTING(    0xdf, "2" )
	PORT_DIPSETTING(    0xef, "3" )
	PORT_DIPSETTING(    0xf7, "4" )
	PORT_DIPSETTING(    0xfb, "5" )
	PORT_DIPSETTING(    0xfd, "6" )
	PORT_DIPSETTING(    0xfe, "7" )

	PORT_START("WHEEL0")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_RESET PORT_REVERSE PORT_PLAYER(1)
	PORT_START("WHEEL1")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_RESET PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END


/***************************************************************************

                            Graphics Layouts

***************************************************************************/

static const gfx_layout layout16x16x6 =
{
	16,16,
	RGN_FRAC(1,1),
	6,
	{ 0,1,2,3,4,5 },
	{ STEP4(3*6,-6),STEP4(7*6,-6),STEP4(11*6,-6),STEP4(15*6,-6) },
	{ STEP16(0,16*6) },
	16*16*6
};

static GFXDECODE_START( fantland )
	GFXDECODE_ENTRY( "gfx1", 0, layout16x16x6, 0, 4 ) // [0] Sprites
GFXDECODE_END

/***************************************************************************

                                Machine Drivers

***************************************************************************/

MACHINE_START_MEMBER(fantland_state,fantland)
{
	save_item(NAME(m_nmi_enable));
}

MACHINE_RESET_MEMBER(fantland_state,fantland)
{
	m_nmi_enable = 0;
}

INTERRUPT_GEN_MEMBER(fantland_state::fantland_irq)
{
	if (m_nmi_enable & 8)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN_MEMBER(fantland_state::fantland_sound_irq)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x80 / 4);
}

static MACHINE_CONFIG_START( fantland, fantland_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, 8000000)        // ?
	MCFG_CPU_PROGRAM_MAP(fantland_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fantland_state,  fantland_irq)

	MCFG_CPU_ADD("audiocpu", I8088, 8000000)        // ?
	MCFG_CPU_PROGRAM_MAP(fantland_sound_map)
	MCFG_CPU_IO_MAP(fantland_sound_iomap)
	MCFG_CPU_PERIODIC_INT_DRIVER(fantland_state, fantland_sound_irq,  8000)
	// NMI when soundlatch is written

	MCFG_MACHINE_START_OVERRIDE(fantland_state,fantland)
	MCFG_MACHINE_RESET_OVERRIDE(fantland_state,fantland)

	MCFG_QUANTUM_TIME(attotime::from_hz(8000))  // sound irq must feed the DAC at 8kHz

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(352,256)
	MCFG_SCREEN_VISIBLE_AREA(0, 352-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(fantland_state, screen_update_fantland)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fantland)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 3000000)
	MCFG_SOUND_ROUTE(0, "mono", 0.35)
	MCFG_SOUND_ROUTE(1, "mono", 0.35)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


WRITE_LINE_MEMBER(fantland_state::galaxygn_sound_irq)
{
	m_audiocpu->set_input_line_and_vector(0, state ? ASSERT_LINE : CLEAR_LINE, 0x80/4);
}

static MACHINE_CONFIG_START( galaxygn, fantland_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, 8000000)        // ?
	MCFG_CPU_PROGRAM_MAP(galaxygn_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fantland_state,  fantland_irq)

	MCFG_CPU_ADD("audiocpu", I8088, 8000000)        // ?
	MCFG_CPU_PROGRAM_MAP(fantland_sound_map)
	MCFG_CPU_IO_MAP(galaxygn_sound_iomap)
	// IRQ by YM2151, NMI when soundlatch is written

	MCFG_MACHINE_START_OVERRIDE(fantland_state,fantland)
	MCFG_MACHINE_RESET_OVERRIDE(fantland_state,fantland)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(352,256)
	MCFG_SCREEN_VISIBLE_AREA(0, 352-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(fantland_state, screen_update_fantland)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fantland)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 3000000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(fantland_state, galaxygn_sound_irq))
	MCFG_SOUND_ROUTE(0, "mono", 1.0)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
MACHINE_CONFIG_END


MACHINE_START_MEMBER(fantland_state,borntofi)
{
	MACHINE_START_CALL_MEMBER(fantland);

	save_item(NAME(m_old_x));
	save_item(NAME(m_old_y));
	save_item(NAME(m_old_f));
	save_item(NAME(m_input_ret));
	save_item(NAME(m_adpcm_playing));
	save_item(NAME(m_adpcm_addr[0]));
	save_item(NAME(m_adpcm_addr[1]));
	save_item(NAME(m_adpcm_nibble));
}

MACHINE_RESET_MEMBER(fantland_state,borntofi)
{
	int i;

	MACHINE_RESET_CALL_MEMBER(fantland);

	for (i = 0; i < 2; i++)
	{
		m_old_x[i] = 0;
		m_old_y[i] = 0;
		m_old_f[i] = 0;
		m_input_ret[i] = 0;
	}

	for (i = 0; i < 4; i++)
	{
		m_adpcm_playing[i] = 1;
		m_adpcm_addr[0][i] = 0;
		m_adpcm_addr[1][i] = 0;
		m_adpcm_nibble[i] = 0;
	}

	borntofi_adpcm_stop(m_msm1, 0);
	borntofi_adpcm_stop(m_msm2, 1);
	borntofi_adpcm_stop(m_msm3, 2);
	borntofi_adpcm_stop(m_msm4, 3);
}

static MACHINE_CONFIG_START( borntofi, fantland_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V20, 16000000/2)        // D701080C-8 - NEC D70108C-8 V20 CPU, running at 8.000MHz [16/2]
	MCFG_CPU_PROGRAM_MAP(borntofi_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fantland_state,  fantland_irq)

	MCFG_CPU_ADD("audiocpu", I8088, 18432000/3)        // 8088 - AMD P8088-2 CPU, running at 6.144MHz [18.432/3]
	MCFG_CPU_PROGRAM_MAP(borntofi_sound_map)

	MCFG_MACHINE_START_OVERRIDE(fantland_state,borntofi)
	MCFG_MACHINE_RESET_OVERRIDE(fantland_state,borntofi)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(54)    // 54 Hz
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(352,256)
	MCFG_SCREEN_VISIBLE_AREA(0, 352-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(fantland_state, screen_update_fantland)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fantland)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	// OKI M5205 running at 384kHz [18.432/48]. Sample rate = 384000 / 48
	MCFG_SOUND_ADD("msm1", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(fantland_state, borntofi_adpcm_int_0))   /* IRQ handler */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 kHz, 4 Bits  */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("msm2", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(fantland_state, borntofi_adpcm_int_1))   /* IRQ handler */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 kHz, 4 Bits  */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("msm3", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(fantland_state, borntofi_adpcm_int_2))   /* IRQ handler */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 kHz, 4 Bits  */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("msm4", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(fantland_state, borntofi_adpcm_int_3))   /* IRQ handler */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 kHz, 4 Bits  */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( wheelrun, fantland_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V20, XTAL_18MHz/2)      // D701080C-8 (V20)
	MCFG_CPU_PROGRAM_MAP(wheelrun_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fantland_state,  fantland_irq)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_18MHz/2)     // Z8400BB1 (Z80B)
	MCFG_CPU_PROGRAM_MAP(wheelrun_sound_map)
	// IRQ by YM3526, NMI when soundlatch is written

	MCFG_MACHINE_START_OVERRIDE(fantland_state,fantland)
	MCFG_MACHINE_RESET_OVERRIDE(fantland_state,fantland)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256,224)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(fantland_state, screen_update_fantland)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fantland)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3526, XTAL_14MHz/4)
	MCFG_YM3526_IRQ_HANDLER(DEVWRITELINE("audiocpu", z80_device, irq_line))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

Fantasy Land by Electronic Devices of Italy

Fantasyl.ev2  od2  d0  d1 s1  s2 :are 27c010 devices
Fantasyl.ev1 &  od1  are 27c512 devices

s1 & s2 are sound files.  (labeled 5 and 6 on devices)
d0 & d1 are next to the image banks  (labeled  7 and 8 on devices)
ev1 od1  od2  ev2  are believed to be program eproms
     (labeled 2, 1, 3, 4 on devices respectively)
     (also labeled under the eprom, location U3, U8, U7, U2 respectively)


3) - 23c4000 mask roms  "05, 06, 07"           !!!! [ 32 pin devices!!! ]
5) - 23c4100 mask roms  "00, 01, 02, 03, 04"   !!!! [ 40 pin devices!!! ]

Fantasyl.01, 00, 02, 03, 04  was read as if it was a 27c400

Fantasy.05, 06, 07 was read as if it was a 27c040

***************************************************************************/

ROM_START( fantland )
	ROM_REGION( 0x100000, "maincpu", 0 )                    // Main CPU
	ROM_LOAD16_BYTE( "fantasyl.ev2", 0x00000, 0x20000, CRC(f5bdca0e) SHA1(d05cf6f68d4d1a3dcc0171f7cf220c4920bd47bb) )
	ROM_LOAD16_BYTE( "fantasyl.od2", 0x00001, 0x20000, CRC(9db35023) SHA1(81e2accd67dcf8563a68b2c4e35526f23a40150c) )
	ROM_COPY( "maincpu",     0x000000, 0x40000, 0x40000 )
	ROM_LOAD16_BYTE( "fantasyl.ev1", 0xe0000, 0x10000, CRC(70e0ee30) SHA1(5253213da56b3f97e2811f2b10927d0e542447f0) )
	ROM_LOAD16_BYTE( "fantasyl.od1", 0xe0001, 0x10000, CRC(577b4bd7) SHA1(1f08202d99c3e39e0dd1ed4947b928b695a5b411) )

	ROM_REGION( 0x100000, "audiocpu", 0 )                   // Sound CPU
	ROM_LOAD( "fantasyl.s2", 0x80000, 0x20000, CRC(f23837d8) SHA1(4048784f759781e50ae445ea61f1ca908e8e6ac1) )   // samples (8 bit unsigned)
	ROM_LOAD( "fantasyl.s1", 0xc0000, 0x20000, CRC(1a324a69) SHA1(06f6877af6cd19bfaac8a4ea8057ef8faee276f5) )
	ROM_COPY( "audiocpu",          0xc0000, 0xe0000, 0x20000 )

	ROM_REGION( 0x480000, "gfx1", 0 )   // Sprites
	ROMX_LOAD( "fantasyl.m00", 0x000000, 0x80000, CRC(82d819ff) SHA1(2b5b0759de8260eaa84ddded9dc2d12a6e0f5ec9) , ROM_GROUPWORD | ROM_SKIP(1) )
	ROMX_LOAD( "fantasyl.m01", 0x0c0000, 0x80000, CRC(70a94139) SHA1(689fbfa267d60821cde13d5dc2dfe1dea67b434a) , ROM_GROUPWORD | ROM_SKIP(1) )
	ROMX_LOAD( "fantasyl.05",  0x000002, 0x80000, CRC(62b9a00b) SHA1(ecd18e5e7a5e3535956fb693d2f7e35d2bb7ede9) , ROM_SKIP(2) )

	ROMX_LOAD( "fantasyl.m02", 0x180000, 0x80000, CRC(ae52bf37) SHA1(60daa24d1f456cfeb643fa2107119d2939af0ffa) , ROM_GROUPWORD | ROM_SKIP(1) )
	ROMX_LOAD( "fantasyl.m03", 0x240000, 0x80000, CRC(f3f534a1) SHA1(9d47cc5b5a40146ed1d9e57a16d67a1d92f3b5be) , ROM_GROUPWORD | ROM_SKIP(1) )
	ROMX_LOAD( "fantasyl.06",  0x180002, 0x80000, CRC(867fa549) SHA1(9777b4837e5bb25a39639597e88b713d43361a80) , ROM_SKIP(2) )

	ROMX_LOAD( "fantasyl.m04", 0x300000, 0x80000, CRC(e7b1918c) SHA1(97230b21bb54c4c928dced83e0b3396068ab72db) , ROM_GROUPWORD | ROM_SKIP(1) )
	ROMX_LOAD( "fantasyl.d0",  0x3c0001, 0x20000, CRC(0f907f19) SHA1(eea90e7d7e2e29db809e867d9b1205f4fbb7ada8) , ROM_SKIP(2) )
	ROMX_LOAD( "fantasyl.d1",  0x3c0000, 0x20000, CRC(10d10389) SHA1(3a5639050c769eedc62924dfde57c1bf020970c8) , ROM_SKIP(2) )
	ROMX_LOAD( "fantasyl.07",  0x300002, 0x80000, CRC(162ad422) SHA1(0d3609e630481018d1326a908d1d4c204dfcdf13) , ROM_SKIP(2) )
ROM_END

/* this dump had several roms half size however they all appear to be data & gfx roms, the main program looks ok */
ROM_START( fantlanda )
	ROM_REGION( 0x100000, "maincpu", 0 )                    // Main CPU
	ROM_LOAD16_BYTE( "fantasyl.ev2", 0x00000, 0x20000, CRC(f5bdca0e) SHA1(d05cf6f68d4d1a3dcc0171f7cf220c4920bd47bb) ) // 04.bin (was first half only)
	ROM_LOAD16_BYTE( "fantasyl.od2", 0x00001, 0x20000, CRC(9db35023) SHA1(81e2accd67dcf8563a68b2c4e35526f23a40150c) ) // 03.bin (was first half only)
	ROM_COPY( "maincpu",     0x000000, 0x40000, 0x40000 )
	ROM_LOAD16_BYTE( "02.bin",       0xe0000, 0x10000, CRC(8b835eed) SHA1(6a6b3fe116145f685b91dcd5301165f17973697c) )
	ROM_LOAD16_BYTE( "01.bin",       0xe0001, 0x10000, CRC(4fa3eb8b) SHA1(56da42a4e2972a696ef28811116cbc20bb5ba3e8) )

	ROM_REGION( 0x100000, "audiocpu", 0 )                   // Sound CPU
	ROM_LOAD( "fantasyl.s2", 0x80000, 0x20000, CRC(f23837d8) SHA1(4048784f759781e50ae445ea61f1ca908e8e6ac1) ) // 05.bin (was first half only)
	ROM_LOAD( "fantasyl.s1", 0xc0000, 0x20000, CRC(1a324a69) SHA1(06f6877af6cd19bfaac8a4ea8057ef8faee276f5) ) // 06.bin (was first half only)
	ROM_COPY( "audiocpu",          0xc0000, 0xe0000, 0x20000 )

	ROM_REGION( 0x480000, "gfx1", 0 )   // Sprites
	ROMX_LOAD( "fantasyl.m00", 0x000000, 0x80000, CRC(82d819ff) SHA1(2b5b0759de8260eaa84ddded9dc2d12a6e0f5ec9) , ROM_GROUPWORD | ROM_SKIP(1) )
	ROMX_LOAD( "fantasyl.m01", 0x0c0000, 0x80000, CRC(70a94139) SHA1(689fbfa267d60821cde13d5dc2dfe1dea67b434a) , ROM_GROUPWORD | ROM_SKIP(1) )
	ROMX_LOAD( "fantasyl.05",  0x000002, 0x80000, CRC(62b9a00b) SHA1(ecd18e5e7a5e3535956fb693d2f7e35d2bb7ede9) , ROM_SKIP(2) )

	ROMX_LOAD( "fantasyl.m02", 0x180000, 0x80000, CRC(ae52bf37) SHA1(60daa24d1f456cfeb643fa2107119d2939af0ffa) , ROM_GROUPWORD | ROM_SKIP(1) )
	ROMX_LOAD( "fantasyl.m03", 0x240000, 0x80000, CRC(f3f534a1) SHA1(9d47cc5b5a40146ed1d9e57a16d67a1d92f3b5be) , ROM_GROUPWORD | ROM_SKIP(1) )
	ROMX_LOAD( "fantasyl.06",  0x180002, 0x80000, CRC(867fa549) SHA1(9777b4837e5bb25a39639597e88b713d43361a80) , ROM_SKIP(2) )

	ROMX_LOAD( "fantasyl.m04", 0x300000, 0x80000, CRC(e7b1918c) SHA1(97230b21bb54c4c928dced83e0b3396068ab72db) , ROM_GROUPWORD | ROM_SKIP(1) )
	ROMX_LOAD( "fantasyl.d0",  0x3c0001, 0x20000, CRC(0f907f19) SHA1(eea90e7d7e2e29db809e867d9b1205f4fbb7ada8) , ROM_SKIP(2) ) // 07.bin (was first half only)
	ROMX_LOAD( "fantasyl.d1",  0x3c0000, 0x20000, CRC(10d10389) SHA1(3a5639050c769eedc62924dfde57c1bf020970c8) , ROM_SKIP(2) ) // 08.bin (was first half only)
	ROMX_LOAD( "fantasyl.07",  0x300002, 0x80000, CRC(162ad422) SHA1(0d3609e630481018d1326a908d1d4c204dfcdf13) , ROM_SKIP(2) )
ROM_END

/***************************************************************************
                                Galaxy Gunners
***************************************************************************/

ROM_START( galaxygn )
	ROM_REGION( 0x100000, "maincpu", 0 )                    // Main CPU
	ROM_LOAD( "gg03.bin", 0x10000, 0x10000, CRC(9e469189) SHA1(07e5d36ca9665bdd13e3bb4241d34b9042371b79) )
	ROM_LOAD( "gg02.bin", 0x20000, 0x10000, CRC(b87a438f) SHA1(96c39cc4d51a2fc0779f148971220117967173c0) )
	ROM_LOAD( "gg01.bin", 0xf0000, 0x10000, CRC(ad0e5b29) SHA1(f9a7ebce9f47a009af213e4e10811bb1c26f891a) )
	ROM_COPY( "maincpu",0xf0000, 0x70000, 0x10000 )

	ROM_REGION( 0x100000, "audiocpu", 0 )                   // Sound CPU
	ROM_LOAD( "gg20.bin", 0xc0000, 0x10000, CRC(f5c65a85) SHA1(a094fa9531ea4e68ec0a448568e7d4b2307c8185) )
	ROM_COPY( "audiocpu", 0xc0000, 0xd0000, 0x10000 )
	ROM_COPY( "audiocpu", 0xc0000, 0xe0000, 0x10000 )
	ROM_COPY( "audiocpu", 0xc0000, 0xf0000, 0x10000 )

	ROM_REGION( 0x1b0000, "gfx1", 0 )   // Sprites
	ROMX_LOAD( "gg54.bin", 0x000000, 0x10000, CRC(b3621119) SHA1(66ade772077e57f872ef1c8f45e244f4006023f0) , ROM_SKIP(2) )
	ROMX_LOAD( "gg38.bin", 0x000001, 0x10000, CRC(52b70f3e) SHA1(65f11d5700337d6d9b6325ff70c86d076e1bdc26) , ROM_SKIP(2) )
	ROMX_LOAD( "gg22.bin", 0x000002, 0x10000, CRC(ea49fee4) SHA1(29ae3e5dfade421a5e97efe5be1cb17862fdcea1) , ROM_SKIP(2) )
	ROMX_LOAD( "gg55.bin", 0x030000, 0x10000, CRC(bffe278f) SHA1(b8077794c4af5aa13ea6f231988e698c1bf229bd) , ROM_SKIP(2) )
	ROMX_LOAD( "gg39.bin", 0x030001, 0x10000, CRC(3f7df1e6) SHA1(c1ef3a2f7aaf2e7e850c884dc5d3c1f1545a2526) , ROM_SKIP(2) )
	ROMX_LOAD( "gg23.bin", 0x030002, 0x10000, CRC(4dcbbc99) SHA1(57ba915043a2c3867bb28875a0d0bc3f81ae69d5) , ROM_SKIP(2) )
	ROMX_LOAD( "gg56.bin", 0x060000, 0x10000, CRC(0306069e) SHA1(e10b142a23a93caac0b505d59b6d5d6a4e195d4b) , ROM_SKIP(2) )
	ROMX_LOAD( "gg40.bin", 0x060001, 0x10000, CRC(f635aa7e) SHA1(3e5ce1c3b25cb2c0387ae5dfe650040ccc353d8a) , ROM_SKIP(2) )
	ROMX_LOAD( "gg24.bin", 0x060002, 0x10000, CRC(733f5dcf) SHA1(e516537dcb3f18da2af307ffded3ee7914e28e20) , ROM_SKIP(2) )
	ROMX_LOAD( "gg57.bin", 0x090000, 0x10000, CRC(c3919bef) SHA1(1eebe888135c51c053d689fda3031769f1efa70a) , ROM_SKIP(2) )
	ROMX_LOAD( "gg41.bin", 0x090001, 0x10000, CRC(1f2757de) SHA1(c853f5ce08141e0197988a9d9e5c0032a29a5824) , ROM_SKIP(2) )
	ROMX_LOAD( "gg25.bin", 0x090002, 0x10000, CRC(1d094f95) SHA1(bcb654c7edd8eec9a655261ebc7c812144d4ff6d) , ROM_SKIP(2) )
	ROMX_LOAD( "gg58.bin", 0x0c0000, 0x10000, CRC(4a459cb8) SHA1(e17de49c56d91942a274206d005dd7bf0f074a7f) , ROM_SKIP(2) )
	ROMX_LOAD( "gg42.bin", 0x0c0001, 0x10000, CRC(ae7a8e1e) SHA1(8714d2b58a26138a9644f9117fcdd89256135a98) , ROM_SKIP(2) )
	ROMX_LOAD( "gg26.bin", 0x0c0002, 0x10000, CRC(c2f310b4) SHA1(510e3bc773456b69609c4e29583753f21dac6165) , ROM_SKIP(2) )
	ROMX_LOAD( "gg59.bin", 0x0f0000, 0x10000, CRC(c8d4fbc2) SHA1(f8e9e7d31fa4c7920a3db36295999ef0ee86dbe9) , ROM_SKIP(2) )
	ROMX_LOAD( "gg43.bin", 0x0f0001, 0x10000, CRC(74d3a0df) SHA1(c8d611c969898f135a254ea53b465305715625c6) , ROM_SKIP(2) )
	ROMX_LOAD( "gg27.bin", 0x0f0002, 0x10000, CRC(c2cfd3f9) SHA1(8c2ad28aa64a124d3c97fde804bf167378ba4c20) , ROM_SKIP(2) )
	ROMX_LOAD( "gg60.bin", 0x120000, 0x10000, CRC(6e32b549) SHA1(541860ad2f2b197fdf3877d8aeded0609ccb7fb0) , ROM_SKIP(2) )
	ROMX_LOAD( "gg44.bin", 0x120001, 0x10000, CRC(fcda6efa) SHA1(b4eb575dee8f78c4f0d3ae26204315924d4ce9bd) , ROM_SKIP(2) )
	ROMX_LOAD( "gg28.bin", 0x120002, 0x10000, CRC(4d4fc01c) SHA1(1ab5186ac440dc004140d5a8bf19df521b60e62d) , ROM_SKIP(2) )
	ROMX_LOAD( "gg61.bin", 0x150000, 0x10000, CRC(177a767a) SHA1(09e2883eaefeb20782bdd5aad069fe35b06b8329) , ROM_SKIP(2) )
	ROMX_LOAD( "gg45.bin", 0x150001, 0x10000, CRC(2ba49d47) SHA1(380943bde589dd2ea079a54fa7bcf327da2990a7) , ROM_SKIP(2) )
	ROMX_LOAD( "gg29.bin", 0x150002, 0x10000, CRC(c1c68148) SHA1(171d25aa20accf6638e1b0886a15db9fac2d8b85) , ROM_SKIP(2) )
	ROMX_LOAD( "gg62.bin", 0x180000, 0x10000, CRC(0fb2d41a) SHA1(41b179e4e9ae142b3057e7cdad6eee8efcd952c4) , ROM_SKIP(2) )
	ROMX_LOAD( "gg46.bin", 0x180001, 0x10000, CRC(5f1bf8ad) SHA1(b831ea433ff008377b522a3be4666d6d1b86cbb4) , ROM_SKIP(2) )
	ROMX_LOAD( "gg30.bin", 0x180002, 0x10000, CRC(ded7cacf) SHA1(adbfaa8f46e5ce8df264d5b5a201d75ca2b3dbeb) , ROM_SKIP(2) )
ROM_END

/*
Born To Fight
?, ?

PCB Layout
----------

Bottom Board

|-------------------------------------------------------------------|
|     PAL                                                           |
|-|   PAL    PAL                     4464    4464                   |
| |          PAL          PAL                                       |
| |          PAL   PAL               4464    4464                   |
| |                                                                 |
| |CN1                    PAL        4464    4464    2018           |
| |                PAL    PAL                                       |
| |                                  4464    4464    2018         |-|
|-|                      |---------|                  |---------| | |
|                        |I.G.1BB  |                  |I.G.1BB  | | |
|J                       |48844758V|                  |48844758V| | |
|A                       |         |                  |         | | |
|M                       |         |                  |         | | |CN2
|M                       |---------|                  |---------| | |
|A             16MHz                                              |-|
|                                PAL        PAL      PAL            |
|    |----------|                PAL                                |
|    |D70108C-8 |                                                   |
|    |----------|                                                   |
|                                                                   |
|          1.BIN                            62256                   |
|          2.BIN  PAL                       62256     2063          |
|DSW1      3.BIN  2018                                              |
|DSW2      62256  PAL                       PAL       PAL       PAL |
|-------------------------------------------------------------------|
Notes:
      D701080C-8 - NEC D70108C-8 V20 CPU, running at 8.000MHz [16/2]
      I.G.1BB    - PLCC84 FPGA, PCB labelled 'INGA 1' & 'INGA 2'
      2018       - 2K x8 SRAM (DIP24)
      2063       - 8K x8 SRAM (DIP28)
      62256      - 32K x8 SRAM (DIP28)
      4464       - 64K x4 DRAM (DIP18)
      CN1/2      - Flat cable connectors joining bottom board to top board
      VSync      - 54Hz

Top Board

|--------------------------------------------------------|
|  VOL                                                   |
|-|                                                      |
| |                                                      |
| |   8088              6.BIN   22.BIN  38.BIN   54.BIN  |
| |                     7.BIN   23.BIN  39.BIN   55.BIN  |
| |   5.BIN             8.BIN   24.BIN  40.BIN   56.BIN  |
| |   6116              9.BIN   25.BIN  41.BIN   57.BIN  |
| |                     10.BIN  26.BIN  42.BIN   58.BIN |-|
| |               PAL   11.BIN  27.BIN  43.BIN   59.BIN | |
|-|                     12.BIN  28.BIN  44.BIN   60.BIN | |
|     6116              13.BIN  29.BIN  45.BIN   61.BIN | |
|                       14.BIN  30.BIN  46.BIN   62.BIN | |
|                       15.BIN  31.BIN  47.BIN   63.BIN | |
|          PAL    M5205 16.BIN    *       *        *    | |
|PAL                    17.BIN    *       *        *    | |
|          PAL    M5205   *       *       *        *    |-|
|                         *       *       *        *     |
|          M5205  PAL     *       *       *        *     |
|18.432MHz                *       *       *        *     |
|          M5205  PAL                                    |
|                                                        |
|          PAL                                           |
|--------------------------------------------------------|
Notes:
      8088 - AMD P8088-2 CPU, running at 6.144MHz [18.432/3]
      M5205- OKI M5205 running at 384kHz [18.432/48]. Sample rate = 384000 / 48
      *    - Unpopulated positions for ROMs

*/

ROM_START( borntofi )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V20 */
	ROM_LOAD( "3.bin", 0x10000, 0x10000, CRC(5f07f4a2) SHA1(240864d1d5d9e26d229bc21aa39ee03f4bd25814) )
	ROM_LOAD( "2.bin", 0x20000, 0x10000, CRC(5d2b3395) SHA1(ac87f352f380b67802c26232824663063549ac7b) )
	ROM_LOAD( "1.bin", 0xf0000, 0x10000, CRC(0a5e2f32) SHA1(5167a85329e5ea35c686af85e44d62227cf5800e) )
	ROM_COPY( "maincpu", 0xf0000, 0x70000, 0x10000 )

	ROM_REGION( 0x100000, "audiocpu", 0 ) /* 8088 */
	ROM_LOAD( "5.bin", 0xf8000, 0x08000, CRC(b5d587ce) SHA1(07687abd264ec80a6eb473cb3f3ab97ec6b365a2) )
	ROM_COPY( "audiocpu", 0xf8000, 0x08000, 0x08000 )

	ROM_REGION( 0xc0000, "adpcm", 0 ) /* m5205 samples */
	ROM_LOAD( "6.bin",  0x00000, 0x10000, CRC(731c523b) SHA1(b649a838ce70d5af607f8b9faf8b012e2ff1104b) )
	ROM_LOAD( "7.bin",  0x10000, 0x10000, CRC(a0cbdf10) SHA1(d33c157aceb52683db36d2f666f1e2d952654633) )
	ROM_LOAD( "8.bin",  0x20000, 0x10000, CRC(82fa8592) SHA1(81510160d645a3496131255c11c94fe47bb75988) )
	ROM_LOAD( "9.bin",  0x30000, 0x10000, CRC(dca5d205) SHA1(01c212fc0dbf9cd1ad0c633288925add853640a1) )
	ROM_LOAD( "10.bin", 0x40000, 0x10000, CRC(0ecd5714) SHA1(4dbc840f9b2707e28cb0e03220a51ad8d9268fe7) )
	ROM_LOAD( "11.bin", 0x50000, 0x10000, CRC(4fd86d07) SHA1(025ecaf7c51882286e2d25ab1636ad79dfe3250d) )
	ROM_LOAD( "12.bin", 0x60000, 0x10000, CRC(cb81236e) SHA1(96423a41e851900ad764e7ec9fbe9a9e34d188ef) )
	ROM_LOAD( "13.bin", 0x70000, 0x10000, CRC(cfcc4bdb) SHA1(b7e1bf189ac964958254e6e8cb7c466b6e7b473f) )
	ROM_LOAD( "14.bin", 0x80000, 0x10000, CRC(c6fa0707) SHA1(831b27f24fee641c30ffd39723a24e7a9a966dc6) )
	ROM_LOAD( "15.bin", 0x90000, 0x10000, CRC(101cbd6b) SHA1(22668f362499f7ae017d9334f426ac644498e0b7) )
	ROM_LOAD( "16.bin", 0xa0000, 0x10000, CRC(edab01a9) SHA1(ce9c6b48807c9b312067b27928d27b4532319c60) )
	ROM_LOAD( "17.bin", 0xb0000, 0x10000, CRC(ea361ea5) SHA1(a2b38a250ab477226da5d56bd07ea3b2f3aa9625) )

	ROM_REGION( 0x1e0000, "gfx1",0 ) /* gfx */
	ROMX_LOAD( "22.bin",  0x000002, 0x10000, CRC(a3afc57f) SHA1(2713fa4b6ad571748f47d25c72a0d40d80f8fcf6), ROM_SKIP(2) )
	ROMX_LOAD( "38.bin",  0x000001, 0x10000, CRC(1c64d329) SHA1(6bb82143de07548b90bc7ba70d12fd0959e56545), ROM_SKIP(2) )
	ROMX_LOAD( "54.bin",  0x000000, 0x10000, CRC(56209405) SHA1(e1f5fd709cde965c400f94837a34d5b414e0118e), ROM_SKIP(2) )
	ROMX_LOAD( "23.bin",  0x030002, 0x10000, CRC(df385140) SHA1(202ef05ffad58ae9be2b264208188722154ec022), ROM_SKIP(2) )
	ROMX_LOAD( "39.bin",  0x030001, 0x10000, CRC(7f789bdb) SHA1(c4220a60d8c11d77037c1f4bfc12254ee024e375), ROM_SKIP(2) )
	ROMX_LOAD( "55.bin",  0x030000, 0x10000, CRC(93f82275) SHA1(132372c2301b57ed2ad47d7e7684f3caa455dc3f), ROM_SKIP(2) )
	ROMX_LOAD( "24.bin",  0x060002, 0x10000, CRC(becc5891) SHA1(0aeb6f2d2d39dd237ef5929674de62bbe0f6e2e6), ROM_SKIP(2) )
	ROMX_LOAD( "40.bin",  0x060001, 0x10000, CRC(707a6ddd) SHA1(30ae010bb2bd41b847790d1612a0702bc0c8b646), ROM_SKIP(2) )
	ROMX_LOAD( "56.bin",  0x060000, 0x10000, CRC(a4e8c94a) SHA1(2514097d449a7883c6442a8d42376bfcc1a87e93), ROM_SKIP(2) )
	ROMX_LOAD( "25.bin",  0x090002, 0x10000, CRC(500d4d3b) SHA1(54a448e5e784633513b94fdd4ad79e66b5210108), ROM_SKIP(2) )
	ROMX_LOAD( "41.bin",  0x090001, 0x10000, CRC(e3bf2b57) SHA1(04619e9de339f4fb5269336ebd2a6749e402243e), ROM_SKIP(2) )
	ROMX_LOAD( "57.bin",  0x090000, 0x10000, CRC(b8d57360) SHA1(5133a5db3d8ee05662662d19b31f0045e833ed42), ROM_SKIP(2) )
	ROMX_LOAD( "26.bin",  0x0c0002, 0x10000, CRC(13801b0e) SHA1(7f830af2d9bf5201c321637ea2400c6b42948ebd), ROM_SKIP(2) )
	ROMX_LOAD( "42.bin",  0x0c0001, 0x10000, CRC(4725835e) SHA1(d60108aa5e7cd7434dd07a5d0553be56c591f4b1), ROM_SKIP(2) )
	ROMX_LOAD( "58.bin",  0x0c0000, 0x10000, CRC(391f220a) SHA1(66de5034b31f99e99005e224ee2e60cf870f1bcb), ROM_SKIP(2) )
	ROMX_LOAD( "27.bin",  0x0f0002, 0x10000, CRC(d8a47ed9) SHA1(4f5f68c35d198f1fdbbcf288bf25c548876ba823), ROM_SKIP(2) )
	ROMX_LOAD( "43.bin",  0x0f0001, 0x10000, CRC(815be7ea) SHA1(478cabf2a683be14e75cbe1b1d63aae521469315), ROM_SKIP(2) )
	ROMX_LOAD( "59.bin",  0x0f0000, 0x10000, CRC(7966e68f) SHA1(13608b781ae10e96eb2802479e082b87f55c3ec1), ROM_SKIP(2) )
	ROMX_LOAD( "28.bin",  0x120002, 0x10000, CRC(e8846d67) SHA1(f3b155836a2f96a3b0b0dfb2034212def0b1dc92), ROM_SKIP(2) )
	ROMX_LOAD( "44.bin",  0x120001, 0x10000, CRC(c5d29821) SHA1(ce45e2e039f3ee7965cc9354a98c77f3db83b950), ROM_SKIP(2) )
	ROMX_LOAD( "60.bin",  0x120000, 0x10000, CRC(47d2a385) SHA1(0b275abe7811972ac475c2dff2e99f2a1f951ee5), ROM_SKIP(2) )
	ROMX_LOAD( "29.bin",  0x150002, 0x10000, CRC(169cba64) SHA1(d881c1cd802da5929688fdaae8c63e581e396dd7), ROM_SKIP(2) )
	ROMX_LOAD( "45.bin",  0x150001, 0x10000, CRC(1a58b8d0) SHA1(c1002e16581face369a11e4308596ba419d82c4d), ROM_SKIP(2) )
	ROMX_LOAD( "61.bin",  0x150000, 0x10000, CRC(f20b88b3) SHA1(f6b4f0a6cf0cdc6bab9f0a580a866477a0925a28), ROM_SKIP(2) )
	ROMX_LOAD( "30.bin",  0x180002, 0x10000, CRC(15529cdd) SHA1(7b80d820476fefb51434870bd9fc4955de1a7323), ROM_SKIP(2) )
	ROMX_LOAD( "46.bin",  0x180001, 0x10000, CRC(2f145546) SHA1(10677effd27ee8702be488fe1485582da465aaf8), ROM_SKIP(2) )
	ROMX_LOAD( "62.bin",  0x180000, 0x10000, CRC(5bbc0154) SHA1(0c7b144cd2be1f8bcd5316093d6f9f0071746593), ROM_SKIP(2) )
	ROMX_LOAD( "31.bin",  0x1b0002, 0x10000, CRC(1446ddc5) SHA1(7bd2ae336514d939c71bdf52a72e710e28d14bd9), ROM_SKIP(2) )
	ROMX_LOAD( "47.bin",  0x1b0001, 0x10000, CRC(b3147a84) SHA1(dfb9c8293a477697017f5632e203c9fb88cadc81), ROM_SKIP(2) )
	ROMX_LOAD( "63.bin",  0x1b0000, 0x10000, CRC(5f530559) SHA1(d1d3edc2082985ccec9fe8ca0b27810623cb5b89), ROM_SKIP(2) )
ROM_END

/***************************************************************************

Wheels Runner by International Games

PCB:
(revision 8801)

1x NEC D70108C-8 (NEC V20)
1x SGS Z8400BB1 (Z80B)
1x YM3526 (sound)
1x Y3014B (DAC)
1x LM324A (sound)
1x TDA2002 (sound)
1x oscillator 18.000
1x oscillator 14.000
1x ALTERA EP1210PC
2x INGA (black quad chips with 84 legs, maybe FPGA)

ROMs:

15x M27512
3x PAL16R6CN (read protected)
2x PAL20L8aCNS (read protected)
5x TIBPAL16l8-25CN (read protected)
4x TIBPAL16r4-25CN (read protected)
1x TIBPAL16r8-25CN (read protected)
eprom location 2,5,6 are empty

Notes:

1x JAMMA edge connector
1x trimmer (volume)
2x 8 bit dip switch

Hardware info by f205v

***************************************************************************/

ROM_START( wheelrun )
	ROM_REGION( 0x100000, "maincpu", 0 ) // V20
	ROM_LOAD( "4.4", 0x30000, 0x10000, CRC(359303df) SHA1(583b70f65b775e99856ffda61334be3b85046ed1) )
	ROM_LOAD( "3.3", 0x70000, 0x10000, CRC(c28d0b31) SHA1(add8c4ffe529755c101b72a3b0530e796948876b) )
	ROM_COPY( "maincpu", 0x70000, 0xf0000, 0x10000 )

	ROM_REGION( 0x100000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "1.1", 0x00000, 0x10000, CRC(67b5f31f) SHA1(5553b132077686221fb7a21a0246fd55cb443332) )   // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0xc0000, "gfx1",0 ) // gfx
	ROMX_LOAD( "7.7",   0x00000, 0x10000, CRC(e0e5ff64) SHA1(e2ed5ea5b75ed627a9d305864196160267cad438), ROM_SKIP(2) )
	ROMX_LOAD( "11.11", 0x00001, 0x10000, CRC(ce9718fb) SHA1(ade47deedd5d0c927fdf8626aa1b0fac470f03a0), ROM_SKIP(2) )
	ROMX_LOAD( "15.15", 0x00002, 0x10000, CRC(f6665f31) SHA1(e308a049697622bcda9d3c630e061d30c2b70687), ROM_SKIP(2) )

	ROMX_LOAD( "8.8",   0x30000, 0x10000, CRC(fa1ec091) SHA1(bd436788651fc2f679897ccd0f7ec51014eb9e90), ROM_SKIP(2) )
	ROMX_LOAD( "12.12", 0x30001, 0x10000, CRC(8923dce4) SHA1(a8f8aeb6f214454c6125a36043aebdf7cc79c253), ROM_SKIP(2) )
	ROMX_LOAD( "16.16", 0x30002, 0x10000, CRC(49801733) SHA1(4d8f79afbb5bf33787ad437d04b95a17e4008894), ROM_SKIP(2) )

	ROMX_LOAD( "9.9",   0x60000, 0x10000, CRC(9fea30d0) SHA1(308caa360f556e085ce05f35e26856d6944b03af), ROM_SKIP(2) )
	ROMX_LOAD( "13.13", 0x60001, 0x10000, CRC(8b0aae8d) SHA1(413821fdbf599004b57f3588360ccf881547e104), ROM_SKIP(2) )
	ROMX_LOAD( "17.17", 0x60002, 0x10000, CRC(be8ab48d) SHA1(1520d70eb9c65f84deddc2d7c8de7ae2cbb1ec09), ROM_SKIP(2) )

	ROMX_LOAD( "10.10", 0x90000, 0x10000, CRC(c5bdd367) SHA1(c432762d23b8799643fd5f1775a44d31582e7290), ROM_SKIP(2) )   // 1111xxxxxxxxxxxx = 0x00
	ROMX_LOAD( "14.14", 0x90001, 0x10000, CRC(e592302f) SHA1(d4f668d259ec649e3126db27d990a2e5fa9cad8d), ROM_SKIP(2) )
	ROMX_LOAD( "18.18", 0x90002, 0x10000, CRC(6bd42d8e) SHA1(0745428a54da85707d4435f20cc2094576a95e5b), ROM_SKIP(2) )   // 1111xxxxxxxxxxxx = 0x00

	ROM_REGION( 0x144, "plds",0 ) // pals
	ROM_LOAD( "pal16r6cn.pal3",        0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r6cn.pal4",        0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16r6cn.pal5",        0x000, 0x104, NO_DUMP )
	ROM_LOAD( "tibpal16l8-25cn.pal1",  0x000, 0x104, NO_DUMP )
	ROM_LOAD( "tibpal16l8-25cn.pal13", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "tibpal16l8-25cn.pal14", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "tibpal16l8-25cn.pal7",  0x000, 0x104, NO_DUMP )
	ROM_LOAD( "tibpal16l8-25cn.pal8",  0x000, 0x104, NO_DUMP )
	ROM_LOAD( "tibpal16r4-25cn.pal10", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "tibpal16r4-25cn.pal15", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "tibpal16r4-25cn.pal6",  0x000, 0x104, NO_DUMP )
	ROM_LOAD( "tibpal16r4-25cn.pal9",  0x000, 0x104, NO_DUMP )
	ROM_LOAD( "tibpal16r8-25cn.pal2",  0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal20l8acns.pal11",     0x000, 0x144, NO_DUMP )
	ROM_LOAD( "pal20l8acns.pal12",     0x000, 0x144, NO_DUMP )
ROM_END


GAME( 19??, borntofi,  0,        borntofi, borntofi, driver_device, 0, ROT0,  "International Games",       "Born To Fight",        MACHINE_SUPPORTS_SAVE )
GAME( 19??, fantland,  0,        fantland, fantland, driver_device, 0, ROT0,  "Electronic Devices Italy",  "Fantasy Land (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 19??, fantlanda, fantland, fantland, fantland, driver_device, 0, ROT0,  "Electronic Devices Italy",  "Fantasy Land (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 19??, wheelrun,  0,        wheelrun, wheelrun, driver_device, 0, ROT0,  "International Games",       "Wheels Runner",        MACHINE_SUPPORTS_SAVE )
GAME( 1989, galaxygn,  0,        galaxygn, galaxygn, driver_device, 0, ROT90, "Electronic Devices Italy",  "Galaxy Gunners",       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
