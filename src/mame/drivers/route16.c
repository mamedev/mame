// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

 Route 16/Stratovox memory map (preliminary)

 driver by Zsolt Vasvari

 Notes: Route 16 and Stratovox use identical hardware with the following
        exceptions: Stratovox has a DAC for voice.
        Route 16 has the added ability to turn off each bitplane individually.
        This looks like an afterthought, as one of the same bits that control
        the palette selection is doubly utilized as the bitmap enable bit.

 Space Echo:
        when all astronauts are taken the game over tune ends with 5 bad notes,
        this appears to be a bug in the ROM from a changed instruction at 2EB3.

        service mode shows a garbled screen as most of the code for it has been
        replaced by other routines, however the sound tests still work. it's
        possible that the service switch isn't connected on the real hardware.

        the game hangs if it doesn't pass the startup test, a best guess is implemented
        rather than patching out the test. code for the same test is in stratvox but
        isn't called, speakres has a very similar test but doesn't care about the result.

        interrupts per frame for cpu1 is a best guess based on how stratvox uses the DAC,
        writing up to 195 times per frame with each byte from the ROM written 4 times.
        spacecho writes one byte per interrupt so 195/4 or 48 is used. a lower number
        increases the chance of a sound interrupting itself, which for most sounds
        is buggy and causes the game to freeze until the first sound completes.

 CPU1

 0000-2fff ROM
 4000-43ff Shared RAM
 8000-bfff Video RAM

 I/O Read

 48xx IN0 - DIP Switches
 50xx IN1 - Input Port 1
 58xx IN2 - Input Port 2
 60xx IN3 - Unknown (Speak & Rescue/Space Echo only)

 I/O Write

 48xx OUT0 - D0-D4 color select for VRAM 0
             D5    coin counter
 50xx OUT1 - D0-D4 color select for VRAM 1
             D5    VIDEO I/II (Flip Screen)
 58xx OUT2 - Unknown (Speak & Rescue/Space Echo only)

 I/O Port Write

 6800 AY-8910 Write Port
 6900 AY-8910 Control Port


 CPU2

 0000-1fff ROM
 4000-43ff Shared RAM
 8000-bfff Video RAM

 I/O Write

 2800      DAC output (Stratovox only)

 ***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/ay8910.h"
#include "includes/route16.h"






/*************************************
 *
 *  Shared RAM handling
 *
 *************************************/



WRITE8_MEMBER(route16_state::route16_sharedram_w)
{
	m_sharedram[offset] = data;

	// 4313-4319 are used in Route 16 as triggers to wake the other CPU
	if (offset >= 0x0313 && offset <= 0x0319 && data == 0xff)
	{
		// Let the other CPU run
		space.device().execute().yield();
	}
}



/*************************************
 *
 *  Stratovox's extra sound effects
 *
 *************************************/

WRITE8_MEMBER(route16_state::stratvox_sn76477_w)
{
	/***************************************************************
	 * AY8910 output bits are connected to...
	 * 7    - direct: 5V * 30k/(100+30k) = 1.15V - via DAC??
	 * 6    - SN76477 mixer C
	 * 5    - SN76477 mixer B
	 * 4    - SN76477 mixer A
	 * 3    - SN76477 envelope 2
	 * 2    - SN76477 envelope 1
	 * 1    - SN76477 vco
	 * 0    - SN76477 enable
	 ***************************************************************/
	m_sn->enable_w((data >> 0) & 1);
	m_sn->vco_w((data >> 1) & 1);
	m_sn->envelope_1_w((data >> 2) & 1);
	m_sn->envelope_2_w((data >> 3) & 1);
	m_sn->mixer_a_w((data >> 4) & 1);
	m_sn->mixer_b_w((data >> 5) & 1);
	m_sn->mixer_c_w((data >> 6) & 1);
}



/*************************************
 *
 *  T.T Mahjong's multiplixed ports
 *
 *************************************/



WRITE8_MEMBER(route16_state::ttmahjng_input_port_matrix_w)
{
	m_ttmahjng_port_select = data;
}


READ8_MEMBER(route16_state::ttmahjng_input_port_matrix_r)
{
	UINT8 ret = 0;

	switch (m_ttmahjng_port_select)
	{
	case 1:  ret = ioport("KEY0")->read(); break;
	case 2:  ret = ioport("KEY1")->read(); break;
	case 4:  ret = ioport("KEY2")->read(); break;
	case 8:  ret = ioport("KEY3")->read(); break;
	default: break;
	}

	return ret;
}



/***************************************************************************
  guessing that the unconnected IN3 and OUT2 on the stratvox schematic
  are hooked up for speakres and spacecho to somehow read the variable
  resistors (eg a voltage ramp), using a write to OUT2 as a trigger
  and then bits 0-2 of IN3 going low when each pot "matches". the VRx
  values can be seen when IN0=0x55 and p1b1 is held during power on.
  this would then be checking that the sounds are mixed correctly.
***************************************************************************/

READ8_MEMBER(route16_state::speakres_in3_r)
{
	int bit2=4, bit1=2, bit0=1;

	/* just using a counter, the constants are the number of reads
	   before going low, each read is 40 cycles apart. the constants
	   were chosen based on the startup tests and for vr0=vr2 */
	m_speakres_vrx++;
	if(m_speakres_vrx>0x300) bit0=0;        /* VR0 100k ohm - speech */
	if(m_speakres_vrx>0x200) bit1=0;        /* VR1  50k ohm - main volume */
	if(m_speakres_vrx>0x300) bit2=0;        /* VR2 100k ohm - explosion */

	return 0xf8|bit2|bit1|bit0;
}

WRITE8_MEMBER(route16_state::speakres_out2_w)
{
	m_speakres_vrx=0;
}



/*************************************
 *
 *  CPU memory maps
 *
 *************************************/

static ADDRESS_MAP_START( route16_cpu1_map, AS_PROGRAM, 8, route16_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	/*AM_RANGE(0x3000, 0x3001) AM_NOP   protection device */
	AM_RANGE(0x4000, 0x43ff) AM_RAM_WRITE(route16_sharedram_w) AM_SHARE("sharedram")
	AM_RANGE(0x4800, 0x4800) AM_READ_PORT("DSW") AM_WRITE(out0_w)
	AM_RANGE(0x5000, 0x5000) AM_READ_PORT("P1") AM_WRITE(out1_w)
	AM_RANGE(0x5800, 0x5800) AM_READ_PORT("P2")
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_SHARE("videoram1")
ADDRESS_MAP_END


static ADDRESS_MAP_START( routex_cpu1_map, AS_PROGRAM, 8, route16_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM_WRITE(route16_sharedram_w) AM_SHARE("sharedram")
	AM_RANGE(0x4800, 0x4800) AM_READ_PORT("DSW") AM_WRITE(out0_w)
	AM_RANGE(0x5000, 0x5000) AM_READ_PORT("P1") AM_WRITE(out1_w)
	AM_RANGE(0x5800, 0x5800) AM_READ_PORT("P2")
	AM_RANGE(0x6400, 0x6400) AM_READ(routex_prot_read)
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_SHARE("videoram1")
ADDRESS_MAP_END


static ADDRESS_MAP_START( stratvox_cpu1_map, AS_PROGRAM, 8, route16_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0x4800, 0x4800) AM_READ_PORT("DSW") AM_WRITE(out0_w)
	AM_RANGE(0x5000, 0x5000) AM_READ_PORT("P1") AM_WRITE(out1_w)
	AM_RANGE(0x5800, 0x5800) AM_READ_PORT("P2")
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_SHARE("videoram1")
ADDRESS_MAP_END


static ADDRESS_MAP_START( speakres_cpu1_map, AS_PROGRAM, 8, route16_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0x4800, 0x4800) AM_READ_PORT("DSW") AM_WRITE(out0_w)
	AM_RANGE(0x5000, 0x5000) AM_READ_PORT("P1") AM_WRITE(out1_w)
	AM_RANGE(0x5800, 0x5800) AM_READ_PORT("P2") AM_WRITE(speakres_out2_w)
	AM_RANGE(0x6000, 0x6000) AM_READ(speakres_in3_r)
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_SHARE("videoram1")
ADDRESS_MAP_END


static ADDRESS_MAP_START( ttmahjng_cpu1_map, AS_PROGRAM, 8, route16_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0x4800, 0x4800) AM_READ_PORT("DSW") AM_WRITE(out0_w)
	AM_RANGE(0x5000, 0x5000) AM_READ_PORT("IN0") AM_WRITE(out1_w)
	AM_RANGE(0x5800, 0x5800) AM_READWRITE(ttmahjng_input_port_matrix_r, ttmahjng_input_port_matrix_w)
	AM_RANGE(0x6800, 0x6800) AM_DEVWRITE("ay8910", ay8910_device, data_w)
	AM_RANGE(0x6900, 0x6900) AM_DEVWRITE("ay8910", ay8910_device, address_w)
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_SHARE("videoram1")
ADDRESS_MAP_END


static ADDRESS_MAP_START( route16_cpu2_map, AS_PROGRAM, 8, route16_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM_WRITE(route16_sharedram_w) AM_SHARE("sharedram")
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_SHARE("videoram2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( stratvox_cpu2_map, AS_PROGRAM, 8, route16_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2800, 0x2800) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_SHARE("videoram2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( cpu1_io_map, AS_IO, 8, route16_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1ff)
	AM_RANGE(0x0000, 0x0000) AM_MIRROR(0x00ff) AM_DEVWRITE("ay8910", ay8910_device, data_w)
	AM_RANGE(0x0100, 0x0100) AM_MIRROR(0x00ff) AM_DEVWRITE("ay8910", ay8910_device, address_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( route16 )
	PORT_START("DSW")       /* DSW 1 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) // Doesn't seem to
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )                    // be referenced
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) // Doesn't seem to
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )                    // be referenced
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
//  PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) ) // Same as 0x08
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")        /* Input Port 1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("P2")        /* Input Port 2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )
INPUT_PORTS_END


static INPUT_PORTS_START( stratvox )
	PORT_START("DSW")       /* IN0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x00, "Replenish Astronouts" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x00, "2 Attackers At Wave" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, "Astronauts Kidnapped" )
	PORT_DIPSETTING(    0x00, "Less Often" )
	PORT_DIPSETTING(    0x10, "More Often" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Demo Voices" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")        /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("P2")        /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )
INPUT_PORTS_END


static INPUT_PORTS_START( speakres )
	PORT_START("DSW")       /* IN0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, "2 Attackers At Wave" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "8000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Demo Voices" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")        /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("P2")        /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )
INPUT_PORTS_END


static INPUT_PORTS_START( spacecho )
	PORT_START("DSW")       /* IN0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x00, "Replenish Astronouts" )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x00, "2 Attackers At Wave" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x10, 0x00, "Astronauts Kidnapped" )
	PORT_DIPSETTING(    0x00, "Less Often" )
	PORT_DIPSETTING(    0x10, "More Often" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Demo Voices" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")        /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("P2")        /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )
INPUT_PORTS_END


static INPUT_PORTS_START( ttmahjng )
	PORT_START("DSW")       /* IN0 */
	PORT_DIPNAME( 0x01, 0x00, "Unknown 01" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x01, "01" )
	PORT_DIPNAME( 0x02, 0x00, "Unknown 02" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x02, "02" )
	PORT_DIPNAME( 0x04, 0x00, "Unknown 04" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x04, "04" )
	PORT_DIPNAME( 0x08, 0x00, "Unknown 08" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x08, "08" )
	PORT_DIPNAME( 0x10, 0x00, "Unknown 10" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPNAME( 0x20, 0x00, "Unknown 20" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPNAME( 0x40, 0x00, "Unknown 40" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x40, "40" )
	PORT_DIPNAME( 0x80, 0x00, "Unknown 80" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x80, "80" )

	PORT_START("IN0")       /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("KEY0")      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )     // START2?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")      /* IN3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )     // START1?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")      /* IN4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")      /* IN5 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

MACHINE_START_MEMBER(route16_state, speakres)
{
	save_item(NAME(m_speakres_vrx));
}

MACHINE_START_MEMBER(route16_state, ttmahjng)
{
	save_item(NAME(m_ttmahjng_port_select));
}


static MACHINE_CONFIG_START( route16, route16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("cpu1", Z80, 2500000)  /* 10MHz / 4 = 2.5MHz */
	MCFG_CPU_PROGRAM_MAP(route16_cpu1_map)
	MCFG_CPU_IO_MAP(cpu1_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", route16_state,  irq0_line_hold)

	MCFG_CPU_ADD("cpu2", Z80, 2500000)  /* 10MHz / 4 = 2.5MHz */
	MCFG_CPU_PROGRAM_MAP(route16_cpu2_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)   /* frames per second, vblank duration */
	MCFG_SCREEN_UPDATE_DRIVER(route16_state, screen_update_route16)

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, 10000000/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( routex, route16 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("cpu1")
	MCFG_CPU_PROGRAM_MAP(routex_cpu1_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( stratvox, route16 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("cpu1")
	MCFG_CPU_PROGRAM_MAP(stratvox_cpu1_map)

	MCFG_CPU_MODIFY("cpu2")
	MCFG_CPU_PROGRAM_MAP(stratvox_cpu2_map)


	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(route16_state, screen_update_ttmahjng)

	/* sound hardware */
	MCFG_SOUND_MODIFY("ay8910")
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(route16_state, stratvox_sn76477_w))  /* SN76477 commands (not used in Route 16?) */

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(RES_K(47), RES_K(150), CAP_U(0.001)) // noise + filter
	MCFG_SN76477_DECAY_RES(RES_M(3.3))                  // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(1), RES_K(4.7))    // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(200))                    // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(55))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(5.0 * 2/ (2 + 10), CAP_U(0.022), RES_K(100)) // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                     // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(75))      // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_U(2.2), RES_K(4.7)) // oneshot caps + res
	MCFG_SN76477_VCO_MODE(0)                            // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                  // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(0, 0)                  // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                              // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( speakres, stratvox )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("cpu1")
	MCFG_CPU_PROGRAM_MAP(speakres_cpu1_map)

	MCFG_MACHINE_START_OVERRIDE(route16_state, speakres)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( spacecho, speakres )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("cpu2")
	MCFG_CPU_PERIODIC_INT_DRIVER(route16_state, irq0_line_hold, 48*60)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ttmahjng, route16 )
	MCFG_CPU_MODIFY("cpu1")
	MCFG_CPU_PROGRAM_MAP(ttmahjng_cpu1_map)
	MCFG_CPU_IO_MAP(0)

	MCFG_MACHINE_START_OVERRIDE(route16_state, ttmahjng)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(route16_state, screen_update_ttmahjng)

	MCFG_DEVICE_REMOVE("palette")
	MCFG_PALETTE_ADD_3BIT_BGR("palette")
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( route16 )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "route16.a0",   0x0000, 0x0800, CRC(8f9101bd) SHA1(b2c0156d41e295282387fb85fc272b031a6d1b64) )
	ROM_LOAD( "route16.a1",   0x0800, 0x0800, CRC(389bc077) SHA1(b0606f6e647e81ceae7148bda96bd4673a51e823) )
	ROM_LOAD( "route16.a2",   0x1000, 0x0800, CRC(1065a468) SHA1(4a707a42fb5a718043c173cb98ff3523eb274ccc) )
	ROM_LOAD( "route16.a3",   0x1800, 0x0800, CRC(0b1987f3) SHA1(9b8abd6ec1ae15ca0d5e4de6b8a7ebf6c929d767) )
	ROM_LOAD( "route16.a4",   0x2000, 0x0800, CRC(f67d853a) SHA1(7479e84082e78f8670cc50858ce6a006d3063413) )
	ROM_LOAD( "route16.a5",   0x2800, 0x0800, CRC(d85cf758) SHA1(5af21250ee44ab1a43b844ede5a777a3d33b78b5) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "route16.b0",   0x0000, 0x0800, CRC(0f9588a7) SHA1(dfaffec4dbabd98cdc21a416bd2966d9d3ae6ad1) )
	ROM_LOAD( "route16.b1",   0x0800, 0x0800, CRC(2b326cf9) SHA1(c6602a9440a982c39f5836c6ab72283b6f9241be) )
	ROM_LOAD( "route16.b2",   0x1000, 0x0800, CRC(529cad13) SHA1(b533d20df1f2580e237c3d60bfe3483486ad9a48) )
	ROM_LOAD( "route16.b3",   0x1800, 0x0800, CRC(3bd8b899) SHA1(bc0c7909dbf5ea85eba5a1bb815fdd98c3aa794e) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( route16a )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "vg-54",        0x0000, 0x0800, CRC(0c966319) SHA1(2f57e9a30dab864bbee2ccb0107c1b4212c5abaf) )
	ROM_LOAD( "vg-55",        0x0800, 0x0800, CRC(a6a8c212) SHA1(a4a695d401b1e495c863c6938296a99592df0e7d) )
	ROM_LOAD( "vg-56",        0x1000, 0x0800, CRC(5c74406a) SHA1(f106c27da6cac597afbabdef3ec7fa7d203905b0) )
	ROM_LOAD( "vg-57",        0x1800, 0x0800, CRC(313e68ab) SHA1(01fa83898123eb92a14bffc6fe774e00b083e86c) )
	ROM_LOAD( "vg-58",        0x2000, 0x0800, CRC(40824e3c) SHA1(bc157e6babf00d2119b389fdb9d5822e1c764f51) )
	ROM_LOAD( "vg-59",        0x2800, 0x0800, CRC(9313d2c2) SHA1(e08112f44ca454820752800d8b3b6408b73a4284) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "route16.b0",   0x0000, 0x0800, CRC(0f9588a7) SHA1(dfaffec4dbabd98cdc21a416bd2966d9d3ae6ad1) )
	ROM_LOAD( "vg-61",        0x0800, 0x0800, CRC(b216c88c) SHA1(d011ef9f3727f87ae3482e271a0c2496f76036b4) )
	ROM_LOAD( "route16.b2",   0x1000, 0x0800, CRC(529cad13) SHA1(b533d20df1f2580e237c3d60bfe3483486ad9a48) )
	ROM_LOAD( "route16.b3",   0x1800, 0x0800, CRC(3bd8b899) SHA1(bc0c7909dbf5ea85eba5a1bb815fdd98c3aa794e) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( route16b )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "rt16.0",       0x0000, 0x0800, CRC(b1f0f636) SHA1(f21915ed40ebdf64970fb7e3cd8071ebfc4aa0b5) )
	ROM_LOAD( "rt16.1",       0x0800, 0x0800, CRC(3ec52fe5) SHA1(451969b5caedd665231ef78cf262679d6d4c8507) )
	ROM_LOAD( "rt16.2",       0x1000, 0x0800, CRC(a8e92871) SHA1(68a709c14309d2b617997b76ae9d7b80fd326f39) )
	ROM_LOAD( "rt16.3",       0x1800, 0x0800, CRC(a0fc9fc5) SHA1(7013750c1b3d403b12eac10282a930538ed9c73e) )
	ROM_LOAD( "rt16.4",       0x2000, 0x0800, CRC(6dcaf8c4) SHA1(27d84cc29f2b75280678e9c77f270ee39af50228) )
	ROM_LOAD( "rt16.5",       0x2800, 0x0800, CRC(63d7b05b) SHA1(d1e3473be283c92063674b9e69575081115bc456) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "rt16.6",       0x0000, 0x0800, CRC(fef605f3) SHA1(bfbffa0ded3e285c034f0ad832864021ef3f2256) )
	ROM_LOAD( "rt16.7",       0x0800, 0x0800, CRC(d0d6c189) SHA1(75cec891e20cf05aae354c8950857aea83c6dadc) )
	ROM_LOAD( "rt16.8",       0x1000, 0x0800, CRC(defc5797) SHA1(aec8179e647de70016e0e63b720f932752adacc1) )
	ROM_LOAD( "rt16.9",       0x1800, 0x0800, CRC(88d94a66) SHA1(163e952ada7c05110d1f1c681bd57d3b9ea8866e) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( routex )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "routex01.a0",  0x0000, 0x0800, CRC(99b500e7) SHA1(2561c04a1425d7ac3309faf29fcfde63a0cda4da) )
	ROM_LOAD( "rt16.1",       0x0800, 0x0800, CRC(3ec52fe5) SHA1(451969b5caedd665231ef78cf262679d6d4c8507) )
	ROM_LOAD( "rt16.2",       0x1000, 0x0800, CRC(a8e92871) SHA1(68a709c14309d2b617997b76ae9d7b80fd326f39) )
	ROM_LOAD( "rt16.3",       0x1800, 0x0800, CRC(a0fc9fc5) SHA1(7013750c1b3d403b12eac10282a930538ed9c73e) )
	ROM_LOAD( "routex05.a4",  0x2000, 0x0800, CRC(2fef7653) SHA1(ba3477da249ca402d096704e57ea638fde6abe9c) )
	ROM_LOAD( "routex06.a5",  0x2800, 0x0800, CRC(a39ef648) SHA1(866095d9880b60b01f7ca66b332f5f6c4b41a5ac) )
	ROM_LOAD( "routex07.a6",  0x3000, 0x0800, CRC(89f80c1c) SHA1(dff37e0f2446a99890135891c59dc501866a25cc) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "routex11.b0",  0x0000, 0x0800, CRC(b51edd1d) SHA1(1ca10afd6851875c98b1d29aee457234c20ce0bf) )
	ROM_LOAD( "rt16.7",       0x0800, 0x0800, CRC(d0d6c189) SHA1(75cec891e20cf05aae354c8950857aea83c6dadc) )
	ROM_LOAD( "rt16.8",       0x1000, 0x0800, CRC(defc5797) SHA1(aec8179e647de70016e0e63b720f932752adacc1) )
	ROM_LOAD( "rt16.9",       0x1800, 0x0800, CRC(88d94a66) SHA1(163e952ada7c05110d1f1c681bd57d3b9ea8866e) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( speakres )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "speakres.1",   0x0000, 0x0800, CRC(6026e4ea) SHA1(77975620b489f10e5b5de834e812c2802315e889) )
	ROM_LOAD( "speakres.2",   0x0800, 0x0800, CRC(93f0d4da) SHA1(bf3d2931d12a436bb4f0d0556806008ca722f070) )
	ROM_LOAD( "speakres.3",   0x1000, 0x0800, CRC(a3874304) SHA1(ca243364d077fa70d6c46b950ba6666617a56cc2) )
	ROM_LOAD( "speakres.4",   0x1800, 0x0800, CRC(f484be3a) SHA1(5befa61c5f3a3cde3d7d6cae2130021288ed8454) )
	ROM_LOAD( "speakres.5",   0x2000, 0x0800, CRC(61b12a67) SHA1(a1a636ecde16ffdc9f0bb460bd12f945ec66d36f) )
	ROM_LOAD( "speakres.6",   0x2800, 0x0800, CRC(220e0ab2) SHA1(9fb4abf50ff28995cb1f7ba807e15eb87127f520) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "speakres.7",   0x0000, 0x0800, CRC(d417be13) SHA1(6f1f76a911579b49bb0e1992296e7c3acf2bd517) )
	ROM_LOAD( "speakres.8",   0x0800, 0x0800, CRC(52485d60) SHA1(28b708a71d16428d1cd58f3b7aa326ccda85533c) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( speakresb )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "hmi1.27",      0x0000, 0x0800, CRC(6026e4ea) SHA1(77975620b489f10e5b5de834e812c2802315e889) )
	ROM_LOAD( "hmi2.28",      0x0800, 0x0800, CRC(93f0d4da) SHA1(bf3d2931d12a436bb4f0d0556806008ca722f070) )
	ROM_LOAD( "hmi3.29",      0x1000, 0x0800, CRC(a3874304) SHA1(ca243364d077fa70d6c46b950ba6666617a56cc2) )
	ROM_LOAD( "hmi4.30",      0x1800, 0x0800, CRC(f484be3a) SHA1(5befa61c5f3a3cde3d7d6cae2130021288ed8454) )
	ROM_LOAD( "hmi5.31",      0x2000, 0x0800, CRC(aa2aaabe) SHA1(eae34bc16ffa1c8dba966c367fae793c52e0cb61) )
	ROM_LOAD( "hmi6.32",      0x2800, 0x0800, CRC(220e0ab2) SHA1(9fb4abf50ff28995cb1f7ba807e15eb87127f520) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "hmi.33",       0x0000, 0x0800, CRC(beafe7c5) SHA1(058d08b4ded46f71053af6bec5e476e21f240608) )
	ROM_LOAD( "hmi.34",       0x0800, 0x0800, CRC(12ecd87b) SHA1(a279711f2a12574126aa626ae2c1acd45660231c) )

	ROM_REGION( 0x0200, "proms", 0 ) /* 6301 proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "hmi.62",       0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "hmi.64",       0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( stratvox )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "ls01.bin",     0x0000, 0x0800, CRC(bf4d582e) SHA1(456f37e16d037a30dc4c1c460ebf9a248bf1a57c) )
	ROM_LOAD( "ls02.bin",     0x0800, 0x0800, CRC(16739dd4) SHA1(cd1f7d1b52ca1ab458d11b969f4f1f5af3ec7353) )
	ROM_LOAD( "ls03.bin",     0x1000, 0x0800, CRC(083c28de) SHA1(82e159f218f60e9c06ff78f2e52572f8f5a6c530) )
	ROM_LOAD( "ls04.bin",     0x1800, 0x0800, CRC(b0927e3b) SHA1(cc5f030dcbc93d5265dbf17a2425acdb921ab18b) )
	ROM_LOAD( "ls05.bin",     0x2000, 0x0800, CRC(ccd25c4e) SHA1(d6d5722d746dd22cecacfea407e798f4531eea99) )
	ROM_LOAD( "ls06.bin",     0x2800, 0x0800, CRC(07a907a7) SHA1(0c41eac01ac9fd67ef19752c47414c4bd90324b4) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "ls07.bin",     0x0000, 0x0800, CRC(4d333985) SHA1(371405b92b2ee8040e48ec7ad715d1a960746aac) )
	ROM_LOAD( "ls08.bin",     0x0800, 0x0800, CRC(35b753fc) SHA1(179e21f531e8be507f1754159590c111be1b44ff) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( stratvoxb )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "j0-1",         0x0000, 0x0800, CRC(93c78274) SHA1(d7c8b5a064eaf96bcfd261b9857f06249477f6b8) )
	ROM_LOAD( "j0-2",         0x0800, 0x0800, CRC(93b2b02d) SHA1(f08772d581f9825976199f39cb6d85fb3aa83db0) )
	ROM_LOAD( "j0-3",         0x1000, 0x0800, CRC(655facb5) SHA1(1ffb1ed65c358846b3de4ead74e86f94ed6ff9df) )
	ROM_LOAD( "j0-4",         0x1800, 0x0800, CRC(b0927e3b) SHA1(cc5f030dcbc93d5265dbf17a2425acdb921ab18b) ) /* Same as ls04.bin of stratvox */
	ROM_LOAD( "j0-5",         0x2000, 0x0800, CRC(9d2178d9) SHA1(7b27dbb2add2c9dda4526c6f1bf52307fe2c6335) )
	ROM_LOAD( "j0-6",         0x2800, 0x0800, CRC(79118ffc) SHA1(d4659f1773e9d55d81185d6c59881c08528e2ab6) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "b0-a",         0x0000, 0x0800, CRC(4d333985) SHA1(371405b92b2ee8040e48ec7ad715d1a960746aac) ) /* Same as ls07.bin of stratvox */
	ROM_LOAD( "j0-a",         0x0800, 0x0800, CRC(3416a830) SHA1(9cbe773968e20455be3e107b29cb8d4dc38632a9) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( spacecho )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "rom.a0",       0x0000, 0x0800, CRC(40d74dce) SHA1(891d7fde1d4b0b66c38fa7f8933480e201c68113) )
	ROM_LOAD( "rom.a1",       0x0800, 0x0800, CRC(a5f0a34f) SHA1(359e7a9954dedb464f7456cd071db77b2219ab2c) )
	ROM_LOAD( "rom.a2",       0x1000, 0x0800, CRC(cbbb3acb) SHA1(3dc71683f31da39a544382b463ece39cca8124b3) )
	ROM_LOAD( "rom.a3",       0x1800, 0x0800, CRC(311050ca) SHA1(ed4a5cb7ec0306654178dae8f30b39b9c8db0ce3) )
	ROM_LOAD( "rom.a4",       0x2000, 0x0800, CRC(28943803) SHA1(4904e6d092494bfca064d25d094ab9e9049fa9ca) )
	ROM_LOAD( "rom.a5",       0x2800, 0x0800, CRC(851c9f28) SHA1(c7bb4e25b74eb71e8b394214f9cbd95f59a1fa58) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "rom.b0",       0x0000, 0x0800, CRC(db45689d) SHA1(057a8dc2629f57fdeebb6262de2bdd78b4e66dca) )
	ROM_LOAD( "rom.b2",       0x1000, 0x0800, CRC(1e074157) SHA1(cb2073415aff7804ac85e2137bef2005bf6cf239) )
	ROM_LOAD( "rom.b3",       0x1800, 0x0800, CRC(d50a8b20) SHA1(d733fa327d2e7dfe08c84015c6c326ed8ab39e3d) )

	ROM_REGION( 0x0200, "proms", 0 ) /* Intersil IM5623CPE proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "im5623.f10",   0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "im5623.f12",   0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

/*
rom.b0                  cb7.5b                  IDENTICAL
rom.a5                  cb6.2.3t                IDENTICAL
rom.a2                  c3.4.5t                 IDENTICAL
rom.a1                  c2.5t                   IDENTICAL
rom.a4                  cb5.3t                  IDENTICAL
rom.b3                  cb10.3b                 IDENTICAL
rom.b2                  cb9.4b                  IDENTICAL
rom.a3                  c4.4t                   IDENTICAL
rom.a0                  c11.5.6t                99.853516%
                        mb7052.6k               NO MATCH
                        mb7052.6m               NO MATCH

Only 3 bytes different between rom.a0 (spacecho) and c11.5.6t (spacecho2), at offset 0x8b.

Spacecho:    0x008b:  call $2929    cd 29 29

Spacech2:    0x008b:  im 1          ed 56
             0x008d:  nop           00

So... spacecho2 is avoiding to enter the sub at $2929.

*/
ROM_START( spacecho2 )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "c11.5.6t",     0x0000, 0x0800, CRC(90637f25) SHA1(820d2f326a5d8d0a04a0fca46b035624dfd7222c) )    // 3 bytes different at 0x8e
	ROM_LOAD( "c2.5t",        0x0800, 0x0800, CRC(a5f0a34f) SHA1(359e7a9954dedb464f7456cd071db77b2219ab2c) )
	ROM_LOAD( "c3.4.5t",      0x1000, 0x0800, CRC(cbbb3acb) SHA1(3dc71683f31da39a544382b463ece39cca8124b3) )
	ROM_LOAD( "c4.4t",        0x1800, 0x0800, CRC(311050ca) SHA1(ed4a5cb7ec0306654178dae8f30b39b9c8db0ce3) )
	ROM_LOAD( "cb5.3t",       0x2000, 0x0800, CRC(28943803) SHA1(4904e6d092494bfca064d25d094ab9e9049fa9ca) )
	ROM_LOAD( "cb6.2.3t",     0x2800, 0x0800, CRC(851c9f28) SHA1(c7bb4e25b74eb71e8b394214f9cbd95f59a1fa58) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "cb7.5b",       0x0000, 0x0800, CRC(db45689d) SHA1(057a8dc2629f57fdeebb6262de2bdd78b4e66dca) )
	ROM_LOAD( "cb9.4b",       0x1000, 0x0800, CRC(1e074157) SHA1(cb2073415aff7804ac85e2137bef2005bf6cf239) )
	ROM_LOAD( "cb10.3b",      0x1800, 0x0800, CRC(d50a8b20) SHA1(d733fa327d2e7dfe08c84015c6c326ed8ab39e3d) )

	ROM_REGION( 0x0200, "proms", 0 ) /* mb7052 proms compatible with 82s129 */
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "mb7052.6k",    0x0000, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* top bitmap */
	ROM_LOAD( "mb7052.6m",    0x0100, 0x0100, CRC(08793ef7) SHA1(bfc27aaf25d642cd57c0fbe73ab575853bd5f3ca) ) /* bottom bitmap */
ROM_END

ROM_START( ttmahjng )
	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "ju04",         0x0000, 0x1000, CRC(fe7c693a) SHA1(be0630557e0bcd9ec2e9542cc4a4d947889ec57a) )
	ROM_LOAD( "ju05",         0x1000, 0x1000, CRC(985723d3) SHA1(9d7499c48cfc242875a95d01459b8f3252ea41bc) )
	ROM_LOAD( "ju06",         0x2000, 0x1000, CRC(2cd69bc8) SHA1(a0a55c972291d043da9f76faf551dba790d5d103) )
	ROM_LOAD( "ju07",         0x3000, 0x1000, CRC(30e8ec63) SHA1(9c6a2b5e436b5e469c15f04c557839b6f07eb22e) )

	ROM_REGION( 0x10000, "cpu2", 0 )
	ROM_LOAD( "ju01",         0x0000, 0x0800, CRC(0f05ca3c) SHA1(6af547b2ec4f69069b4ad62d96d109ec0105dd8b) )
	ROM_LOAD( "ju02",         0x0800, 0x0800, CRC(c1ffeceb) SHA1(18cf337ef2c9b51f1e9e4f08743225755c4ff420) )
	ROM_LOAD( "ju08",         0x1000, 0x0800, CRC(2dcc76b5) SHA1(1732bcf5492dda34425681e7f28775ad7a5e04af) )

	ROM_REGION( 0x0200, "proms", 0 )
	/* The upper 128 bytes are 0's, used by the hardware to blank the display */
	ROM_LOAD( "ju03",         0x0000, 0x0100, CRC(27d47624) SHA1(ee04ce8043216be8b91413b546479419fca2b917) )
	ROM_LOAD( "ju09",         0x0100, 0x0100, CRC(27d47624) SHA1(ee04ce8043216be8b91413b546479419fca2b917) )
ROM_END



/*************************************
 *
 *  Protection handling
 *
 *************************************/

READ8_MEMBER(route16_state::routex_prot_read)
{
	if (space.device().safe_pc() == 0x2f) return 0xfb;

	logerror ("cpu '%s' (PC=%08X): unmapped prot read\n", space.device().tag(), space.device().safe_pc());
	return 0x00;

}


/*************************************
 *
 *  Drivers specific initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(route16_state,route16)
{
	UINT8 *ROM = memregion("cpu1")->base();
	/* Is this actually a bootleg? some of the protection has
	   been removed */

	/* patch the protection */
	ROM[0x00e9] = 0x3a;

	ROM[0x0754] = 0xc3;
	ROM[0x0755] = 0x63;
	ROM[0x0756] = 0x07;
}


DRIVER_INIT_MEMBER(route16_state,route16a)
{
	UINT8 *ROM = memregion("cpu1")->base();
	/* TO DO : Replace these patches with simulation of the protection device */

	/* patch the protection */
	ROM[0x00e9] = 0x3a;

	ROM[0x0105] = 0x00; /* jp nz,$4109 (nirvana) - NOP's in route16 */
	ROM[0x0106] = 0x00;
	ROM[0x0107] = 0x00;

	ROM[0x0731] = 0x00; /* jp nz,$4238 (nirvana) */
	ROM[0x0732] = 0x00;
	ROM[0x0733] = 0x00;

	ROM[0x0747] = 0xc3;
	ROM[0x0748] = 0x56;
	ROM[0x0749] = 0x07;
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1981, route16,  0,        route16,  route16, route16_state,  route16,  ROT270, "Tehkan / Sun Electronics (Centuri license)", "Route 16 (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, route16a, route16,  route16,  route16, route16_state,  route16a, ROT270, "Tehkan / Sun Electronics (Centuri license)", "Route 16 (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, route16b, route16,  route16,  route16, driver_device,  0,        ROT270, "bootleg", "Route 16 (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, routex,   route16,  routex,   route16, driver_device,  0,        ROT270, "bootleg", "Route X (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, speakres, 0,        speakres, speakres, driver_device, 0,        ROT270, "Sun Electronics", "Speak & Rescue", MACHINE_SUPPORTS_SAVE )
GAME( 1980, speakresb,speakres, speakres, speakres, driver_device, 0,        ROT270, "bootleg", "Speak & Rescue (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, stratvox, speakres, stratvox, stratvox, driver_device, 0,        ROT270, "Sun Electronics (Taito license)", "Stratovox", MACHINE_SUPPORTS_SAVE )
GAME( 1980, stratvoxb,speakres, stratvox, stratvox, driver_device, 0,        ROT270, "bootleg", "Stratovox (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacecho, speakres, spacecho, spacecho, driver_device, 0,        ROT270, "bootleg (Gayton Games)", "Space Echo (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, spacecho2,speakres, spacecho, spacecho, driver_device, 0,        ROT270, "bootleg (Gayton Games)", "Space Echo (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, ttmahjng, 0,        ttmahjng, ttmahjng, driver_device, 0,        ROT0,   "Taito", "T.T Mahjong", MACHINE_SUPPORTS_SAVE )
