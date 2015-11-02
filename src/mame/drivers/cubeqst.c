// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Simutrek Cube Quest

    driver by Phil Bennett

    TODO:
       * Accurate video timings
        - Derive from PROMs
       * More accurate line fill circuitry emulation
        - Use PROMs

    Known bugs:
        * The graphics tend go screwy when you add the first credit on the
          'Cubic History' screen.
        * The guardians' pincer thingies shouldn't distort when they rotate

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/cubeqcpu/cubeqcpu.h"
#include "sound/dac.h"
#include "machine/ldpr8210.h"
#include "machine/nvram.h"


class cubeqst_state : public driver_device
{
public:
	cubeqst_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_laserdisc(*this, "laserdisc"),
			m_rotatecpu(*this, "rotate_cpu"),
			m_linecpu(*this, "line_cpu"),
			m_soundcpu(*this, "sound_cpu"),
			m_screen(*this, "screen"),
			m_generic_paletteram_16(*this, "paletteram") { }

	UINT8 *m_depth_buffer;
	int m_video_field;
	UINT8 m_io_latch;
	UINT8 m_reset_latch;
	required_device<simutrek_special_device> m_laserdisc;
	required_device<cquestrot_cpu_device> m_rotatecpu;
	required_device<cquestlin_cpu_device> m_linecpu;
	required_device<cquestsnd_cpu_device> m_soundcpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<UINT16> m_generic_paletteram_16;
	rgb_t *m_colormap;
	DECLARE_WRITE16_MEMBER(palette_w);
	DECLARE_READ16_MEMBER(line_r);
	DECLARE_WRITE16_MEMBER(laserdisc_w);
	DECLARE_READ16_MEMBER(laserdisc_r);
	DECLARE_WRITE16_MEMBER(ldaud_w);
	DECLARE_WRITE16_MEMBER(control_w);
	DECLARE_WRITE16_MEMBER(reset_w);
	DECLARE_WRITE16_MEMBER(io_w);
	DECLARE_READ16_MEMBER(io_r);
	DECLARE_READ16_MEMBER(chop_r);
	DECLARE_READ16_MEMBER(read_rotram);
	DECLARE_WRITE16_MEMBER(write_rotram);
	DECLARE_READ16_MEMBER(read_sndram);
	DECLARE_WRITE16_MEMBER(write_sndram);
	DECLARE_WRITE16_MEMBER(sound_dac_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_cubeqst(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank);
	TIMER_CALLBACK_MEMBER(delayed_bank_swap);
	void swap_linecpu_banks();
};



/*************************************
 *
 *  Constants
 *
 *************************************/

/* TODO */
#define CUBEQST_HBLANK      320
#define CUBEQST_VCOUNT      280


/*************************************
 *
 *  Video (move to separate file)
 *
 *************************************/

void cubeqst_state::video_start()
{
	m_video_field = 0;
	m_depth_buffer = auto_alloc_array(machine(), UINT8, 512);
}

WRITE16_MEMBER(cubeqst_state::palette_w)
{
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	COMBINE_DATA(&m_generic_paletteram_16[offset]);
}

/* TODO: This is a simplified version of what actually happens */
UINT32 cubeqst_state::screen_update_cubeqst(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int y;

	/*
	 * Clear the display with palette RAM entry 0xff
	 * This will be either transparent or an actual colour
	*/

	/* Bit 3 selects LD/#GRAPHICS */
	bitmap.fill(m_colormap[255], cliprect);

	/* TODO: Add 1 for linebuffering? */
	for (y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		int i;
		int num_entries = m_linecpu->cubeqcpu_get_ptr_ram_val(y);
		UINT32 *stk_ram = m_linecpu->cubeqcpu_get_stack_ram();
		UINT32 *dest = &bitmap.pix32(y);
		UINT32 pen;

		/* Zap the depth buffer */
		memset(m_depth_buffer, 0xff, 512);

		/* Process all the spans on this scanline */
		if (y < 256)
		{
			for (i = 0; i < num_entries; i += 2)
			{
				int color = 0, depth = 0;
				int h1 = 0, h2 = 0;
				int x;

				int entry1 = stk_ram[(y << 7) | ((i + 0) & 0x7f)];
				int entry2 = stk_ram[(y << 7) | ((i + 1) & 0x7f)];

				/* Determine which entry is the start point and which is the stop */
				if ( entry1 & (1 << 19) )
				{
					h1 = (entry2 >> 8) & 0x1ff;
					depth = entry2 & 0xff;

					h2 = (entry1 >> 8) & 0x1ff;
					color = entry1 & 0xff;
				}
				else if ( entry2 & (1 << 19) )
				{
					h1 = (entry1 >> 8) & 0x1ff;
					depth = entry1 & 0xff;

					h2 = (entry2 >> 8) & 0x1ff;
					color = entry2 & 0xff;
				}
				else
				{
					// Shouldn't happen...
				}

				/* Draw the span, testing for depth */
				pen = m_colormap[m_generic_paletteram_16[color]];
				for (x = h1; x <= h2; ++x)
				{
					if (!(m_depth_buffer[x] < depth))
					{
						dest[x] = pen;
						m_depth_buffer[x] = depth;
					}
				}
			}
		}
	}

	return 0;
}

READ16_MEMBER(cubeqst_state::line_r)
{
	/* I think this is unusued */
	return m_screen->vpos();
}

INTERRUPT_GEN_MEMBER(cubeqst_state::vblank)
{
	int int_level = m_video_field == 0 ? 5 : 6;

	device.execute().set_input_line(int_level, HOLD_LINE);

	/* Update the laserdisc */
	m_video_field ^= 1;
}


/*************************************
 *
 *  Laserdisc Interface
 *
 *************************************/

WRITE16_MEMBER(cubeqst_state::laserdisc_w)
{
	m_laserdisc->data_w(data & 0xff);
}

/*
    D0: Command acknowledge
    D1: Seek status (0 = searching, 1 = ready)
*/
READ16_MEMBER(cubeqst_state::laserdisc_r)
{
	int ldp_command_flag = (m_laserdisc->ready_r() == ASSERT_LINE) ? 0 : 1;
	int ldp_seek_status = (m_laserdisc->status_r() == ASSERT_LINE) ? 1 : 0;

	return (ldp_seek_status << 1) | ldp_command_flag;
}


/* LDP audio squelch control */
WRITE16_MEMBER(cubeqst_state::ldaud_w)
{
	m_laserdisc->set_external_audio_squelch(data & 1 ? ASSERT_LINE : CLEAR_LINE);
}

/*
    Control Register
    ================

    D0: Disk on
    D1: H Genlock
    D3: V Genlock

    Note: Can only be written during VBLANK (as with palette RAM)
*/
WRITE16_MEMBER(cubeqst_state::control_w)
{
	m_laserdisc->video_enable(data & 1);
}


/*************************************
 *
 *  General Machine Stuff
 *
 *************************************/

TIMER_CALLBACK_MEMBER(cubeqst_state::delayed_bank_swap)
{
	m_linecpu->cubeqcpu_swap_line_banks();

	/* TODO: This is a little dubious */
	m_linecpu->cubeqcpu_clear_stack();
}


void cubeqst_state::swap_linecpu_banks()
{
	/* Best sync up before we switch banks around */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(cubeqst_state::delayed_bank_swap),this));
}


/*
    Reset Control
    =============

    D0: /Display (includes rotate and line processors)
    D1: /Sound
    D2: /Disk
*/
WRITE16_MEMBER(cubeqst_state::reset_w)
{
	m_rotatecpu->set_input_line(INPUT_LINE_RESET, data & 1 ? CLEAR_LINE : ASSERT_LINE);
	m_linecpu->set_input_line(INPUT_LINE_RESET, data & 1 ? CLEAR_LINE : ASSERT_LINE);
	m_soundcpu->set_input_line(INPUT_LINE_RESET, data & 2 ? CLEAR_LINE : ASSERT_LINE);

	/* Swap stack and pointer RAM banks on rising edge of display reset */
	if (!BIT(m_reset_latch, 0) && BIT(data, 0))
		swap_linecpu_banks();

	if (!BIT(data, 2))
		m_laserdisc->reset();

	m_reset_latch = data & 0xff;
}


/*************************************
 *
 *  I/O
 *
 *************************************/

WRITE16_MEMBER(cubeqst_state::io_w)
{
	/*
	   0: Spare lamp
	   1: Spare driver
	   2: Coin counter
	   3: Left-front lamp
	   4: Right-front lamp
	   5: Righ back lamp
	   6: Spare lamp
	   7: LED latch clock
	*/

	/* TODO: On rising edge of Q7, status LED latch is written */
	if ( !BIT(m_io_latch, 7) && BIT(data, 7) )
	{
		/*
		    0: Battery failure
		    1: Bad coin
		    2: No laser unit
		*/
	}

	m_io_latch = data;
}

READ16_MEMBER(cubeqst_state::io_r)
{
	UINT16 port_data = ioport("IO")->read();

	/*
	     Certain bits depend on Q7 of the IO latch:

	      5: Cube   / Trackball H clock
	      6: R-Fire / Trackball H data
	      7: L-Fire / Trackball V clock
	     10: Spare  / Trackball V data
	*/

	if ( !BIT(m_io_latch, 7) )
		return port_data;
	else
		/* Return zeroes for the trackball signals for now */
		return port_data & ~0x4e0;
}

/* Trackball ('CHOP') */
READ16_MEMBER(cubeqst_state::chop_r)
{
	return (ioport("TRACK_X")->read() << 8) | ioport("TRACK_Y")->read();
}


/*************************************
 *
 *  Input definitions
 *
 *************************************/

static INPUT_PORTS_START( cubeqst )
	PORT_START("IO")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Cube" )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Right Fire" )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Left Fire" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME( "Free Game" )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACK_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("TRACK_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)
INPUT_PORTS_END


/*************************************
 *
 *  CPU memory handlers
 *
 *************************************/

READ16_MEMBER(cubeqst_state::read_rotram)
{
	return m_rotatecpu->rotram_r(space, offset, mem_mask);
}

WRITE16_MEMBER(cubeqst_state::write_rotram)
{
	m_rotatecpu->rotram_w(space, offset, data, mem_mask);
}

READ16_MEMBER(cubeqst_state::read_sndram)
{
	return m_soundcpu->sndram_r(space, offset, mem_mask);
}

WRITE16_MEMBER(cubeqst_state::write_sndram)
{
	m_soundcpu->sndram_w(space, offset, data, mem_mask);
}

static ADDRESS_MAP_START( m68k_program_map, AS_PROGRAM, 16, cubeqst_state )
	ADDRESS_MAP_GLOBAL_MASK(0x03ffff)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x020000, 0x027fff) AM_READWRITE(read_rotram, write_rotram)
	AM_RANGE(0x028000, 0x028fff) AM_READWRITE(read_sndram, write_sndram)
	AM_RANGE(0x038000, 0x038001) AM_READWRITE(io_r, io_w)
	AM_RANGE(0x038002, 0x038003) AM_READWRITE(chop_r, ldaud_w)
	AM_RANGE(0x038008, 0x038009) AM_READWRITE(line_r, reset_w)
	AM_RANGE(0x03800e, 0x03800f) AM_READWRITE(laserdisc_r, laserdisc_w)
	AM_RANGE(0x03c800, 0x03c9ff) AM_RAM_WRITE(palette_w) AM_SHARE("paletteram")
	AM_RANGE(0x03cc00, 0x03cc01) AM_WRITE(control_w)
	AM_RANGE(0x03e000, 0x03efff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x03f000, 0x03ffff) AM_RAM
ADDRESS_MAP_END


/* For the bit-sliced CPUs */
static ADDRESS_MAP_START( rotate_map, AS_PROGRAM, 64, cubeqst_state )
	AM_RANGE(0x000, 0x1ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( line_sound_map, AS_PROGRAM, 64, cubeqst_state )
	AM_RANGE(0x000, 0x0ff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Initialisation
 *
 *************************************/

void cubeqst_state::machine_start()
{
	/* TODO: Use resistor values */
	int i;

	m_colormap = auto_alloc_array(machine(), rgb_t, 65536);
	for (i = 0; i < 65536; ++i)
	{
		UINT8 a, r, g, b, y;

		a = (i >> 3) & 1;
		b = (i >> 0) & 7;
		g = (i >> 4) & 7;
		r = (i >> 8) & 7;
		y = ((i >> 12) & 0xf) * 2;

		m_colormap[i] = rgb_t(a ? 0 : 255, y*r, y*g, y*b);
	}
}

void cubeqst_state::machine_reset()
{
	m_reset_latch = 0;

	/* Auxillary CPUs are held in reset */
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_rotatecpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_linecpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


/*************************************
 *
 *  Sound definitions
 *
 *************************************/

/*
 *  The sound CPU outputs to a 12-bit 7521 DAC
 *  The DAC output is multiplexed between
 *  16 channels (8 per side).
 */

/* Called by the sound CPU emulation */
WRITE16_MEMBER( cubeqst_state::sound_dac_w )
{
	static const char *const dacs[] =
	{
		"rdac0", "ldac0",
		"rdac1", "ldac1",
		"rdac2", "ldac2",
		"rdac3", "ldac3",
		"rdac4", "ldac4",
		"rdac5", "ldac5",
		"rdac6", "ldac6",
		"rdac7", "ldac7"
	};
	machine().device<dac_device>(dacs[data & 15])->write_signed16((data & 0xfff0) ^ 0x8000);
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( cubeqst, cubeqst_state )
	MCFG_CPU_ADD("main_cpu", M68000, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(m68k_program_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cubeqst_state,  vblank)

	MCFG_CPU_ADD("rotate_cpu", CQUESTROT, XTAL_10MHz / 2)
	MCFG_CPU_PROGRAM_MAP(rotate_map)
	MCFG_CQUESTROT_CONFIG( DEVWRITE16( "line_cpu", cquestlin_cpu_device, linedata_w ) )

	MCFG_CPU_ADD("line_cpu", CQUESTLIN, XTAL_10MHz / 2)
	MCFG_CPU_PROGRAM_MAP(line_sound_map)
	MCFG_CQUESTLIN_CONFIG( DEVREAD16( "rotate_cpu", cquestrot_cpu_device, linedata_r ) )

	MCFG_CPU_ADD("sound_cpu", CQUESTSND, XTAL_10MHz / 2)
	MCFG_CPU_PROGRAM_MAP(line_sound_map)
	MCFG_CQUESTSND_CONFIG( WRITE16( cubeqst_state, sound_dac_w ), "soundproms" )

	MCFG_QUANTUM_TIME(attotime::from_hz(48000))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_LASERDISC_SIMUTREK_ADD("laserdisc")
	MCFG_LASERDISC_OVERLAY_DRIVER(CUBEQST_HBLANK, CUBEQST_VCOUNT, cubeqst_state, screen_update_cubeqst)
	MCFG_LASERDISC_OVERLAY_CLIP(0, 320-1, 0, 256-8)
	MCFG_LASERDISC_OVERLAY_POSITION(0.002f, -0.018f)
	MCFG_LASERDISC_OVERLAY_SCALE(1.0f, 1.030f)
	MCFG_LASERDISC_OVERLAY_PALETTE("palette")

	MCFG_LASERDISC_SCREEN_ADD_NTSC("screen", "laserdisc")


	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_MODIFY("laserdisc")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_DAC_ADD("rdac0")
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.125)
	MCFG_DAC_ADD("ldac0")
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.125)
	MCFG_DAC_ADD("rdac1")
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.125)
	MCFG_DAC_ADD("ldac1")
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.125)
	MCFG_DAC_ADD("rdac2")
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.125)
	MCFG_DAC_ADD("ldac2")
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.125)
	MCFG_DAC_ADD("rdac3")
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.125)
	MCFG_DAC_ADD("ldac3")
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.125)
	MCFG_DAC_ADD("rdac4")
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.125)
	MCFG_DAC_ADD("ldac4")
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.125)
	MCFG_DAC_ADD("rdac5")
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.125)
	MCFG_DAC_ADD("ldac5")
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.125)
	MCFG_DAC_ADD("rdac6")
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.125)
	MCFG_DAC_ADD("ldac6")
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.125)
	MCFG_DAC_ADD("rdac7")
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.125)
	MCFG_DAC_ADD("ldac7")
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.125)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( cubeqst )
	ROM_REGION( 0x200000, "main_cpu", 0 )
	ROM_LOAD16_BYTE( "eprom_bd_2764.1a", 0x000000, 0x02000, CRC(870a28f5) SHA1(6db2020fa10d03bfaf7bd2892ba7266e744567e9) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.1b", 0x000001, 0x02000, CRC(0c29b2d2) SHA1(1c67dbfaeb6ae1e6ccd870a815bd51ebc520a34c) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.2a", 0x004000, 0x02000, CRC(f03a76be) SHA1(794a9e5a4fddc16950836468157ea252a82c0771) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.2b", 0x004001, 0x02000, CRC(c995b786) SHA1(0370e0be81ceea647ea9e27eac22b6c2354e695b) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.3a", 0x008000, 0x02000, CRC(dedeabf3) SHA1(d06138336bdbd5ebee44188e92e5721ae63715ad) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.3b", 0x008001, 0x02000, CRC(7353e970) SHA1(9566335868352d3c652621809b812ede309ebded) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.4a", 0x00c000, 0x02000, CRC(3f8ae830) SHA1(13411f7bee31ae13a2e139ad576e48130838c472) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.4b", 0x00c001, 0x02000, CRC(6d4fc7cf) SHA1(8f95e2f9e313413ea6b9bcd1ec5d8c11962584db) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.5a", 0x010000, 0x02000, CRC(ea1a92d0) SHA1(71a6820c67aad951d05571cd29f52c2cebe78c3c) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.5b", 0x010001, 0x02000, CRC(d12ed62f) SHA1(3e897ef88f51d6b2010944b50f1b5387801d92ce) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.6a", 0x014000, 0x02000, CRC(6d31f22b) SHA1(36ecfb1c88be9a100a272564d8ff8df02a501f88) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.6b", 0x014001, 0x02000, CRC(2110f613) SHA1(e12e3b902e9d755fc55ff67851c905e40482a98f) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.7a", 0x018000, 0x02000, CRC(8b624074) SHA1(592891b384bdab3f851799e50f51b5f04c880490) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.7b", 0x018001, 0x02000, CRC(f31f2e81) SHA1(b4ae0726ac849662fb1878094a956aa3a9fe94a2) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.8a", 0x01c000, 0x02000, CRC(8ac5ab0c) SHA1(0cf6593c53184686d7020acd27e2c26aab29137c) )
	ROM_LOAD16_BYTE( "eprom_bd_2764.8b", 0x01c001, 0x02000, CRC(39804640) SHA1(0f09cf6ef73a9199318278df15983a0cc01ece0d) )

	ROM_REGION64_BE( 0x1000, "rotate_cpu", 0)
	ROMX_LOAD( "rotate_video_board_82s131.8h",  0x0, 0x200, CRC(1dd1cc65) SHA1(3c83d48f90ba70e81c0d85fb182527afe1f1f68d), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.9h",  0x0, 0x200, CRC(2bbf0aa1) SHA1(1a4ef24a8dc27ac3b01f176aac9b1c1c194bd67b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.10f", 0x1, 0x200, CRC(89b234eb) SHA1(23d84873b4a5088130fff960b3caded329ea70b5), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.10h", 0x1, 0x200, CRC(d86561b2) SHA1(ebc5374568d9b598237d98f0fb95b68b36a3ddff), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.7h",  0x2, 0x200, CRC(4e0909e2) SHA1(e97dcbd23e0d9eccc138e62334e016b1b333dec3), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12k", 0x2, 0x200, CRC(84db792f) SHA1(03858d92f4ad5aae39ad1925738df958cc4dd020), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12j", 0x3, 0x200, CRC(661350ba) SHA1(f1ccc31c7ce1d71f6128c575d508aa04a4a5e15f), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12d", 0x3, 0x200, CRC(c60762ec) SHA1(d113eb1ce0db4d10a55057332fb17852b312396c), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12e", 0x4, 0x200, CRC(605b924c) SHA1(899103f20ccec68e432c4abf11f554b0600f8b3c), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12c", 0x4, 0x200, CRC(fbb881b7) SHA1(b8c1592cfa082fa7dc4abd5392816ba923760cbf), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12h", 0x5, 0x200, CRC(6fb1d6f0) SHA1(5a0bf681aedde073f0164264620085e9f40eca9e), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12f", 0x5, 0x200, CRC(5c00d30e) SHA1(ad7bbcce9eff348f78cb6e0782faa175ca928c11), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))

	ROM_REGION64_BE( 0x800, "line_cpu", 0)
	ROMX_LOAD( "line_drawer_82s129_u27.6b", 0x0, 0x100, CRC(a9777c19) SHA1(dcb83cc5fce3ff9760dfdfce078895ef71558f67), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u28.3b", 0x0, 0x100, CRC(2248790c) SHA1(8473276bcc154a440745b714a31c89c459fef411), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u29.3c", 0x1, 0x100, CRC(6accc743) SHA1(a5263a021c594117c14a3d4fbc207a4043001172), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u30.3f", 0x1, 0x100, CRC(ba02ac36) SHA1(89473146f04f60bbed6644603ddc646b3d4e04eb), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u31.2j", 0x2, 0x100, CRC(bbf50d89) SHA1(f49d4322123c3d4a7d6f664b7164dfe24ddd6eed), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u5.3g",  0x2, 0x100, CRC(fa5c239b) SHA1(bbf526362af263aefea87341538157f4ecad686f), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u6.2g",  0x3, 0x100, CRC(59874b50) SHA1(d304b4546c187b8c73aaf8887f0037426e60dd5f), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u7.3h",  0x3, 0x100, CRC(886956d6) SHA1(ee150a9f99f3177fcc94be5af3f87433f39b311f), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u8.3j",  0x4, 0x100, CRC(8d2b17ad) SHA1(f7408104f75e292539aaec688e220c143f762f67), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u9.3k",  0x4, 0x100, CRC(0a678253) SHA1(06d7a8f9556eb156ae03184772e76c84ab8d75ea), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))

	ROM_REGION64_BE( 0x800, "sound_cpu", 0)
	ROMX_LOAD( "mother_sounds_82s129.7e",  0x0, 0x100, CRC(cf89ad06) SHA1(2f04ca6d57a8c425f51a288f553841db680f5a7e), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.7f",  0x0, 0x100, CRC(5208036d) SHA1(ec29af4c10a098fb25424d1a5187a7bd2ec9fb92), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.11f", 0x1, 0x100, CRC(af327360) SHA1(2b45358a39daaf5721b5a94c4fd55bd3d0d90ec3), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.11e", 0x1, 0x100, CRC(b50d3e43) SHA1(9f2c071c6940767d20a563f6f2c9224e1896a5bf), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.9f",  0x2, 0x100, CRC(cd93e3c9) SHA1(f0c11db65ddf2096f6df424b7747242d9e3155da), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.10f", 0x2, 0x100, CRC(0e8b8df7) SHA1(37426ffdaf2d21273ac7e88789206fdafb14bc83), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.8f",  0x3, 0x100, CRC(07deae27) SHA1(c6a2d7cbf4ea4120f43ae088a3de90bb7a42d4a4), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.10e", 0x3, 0x100, CRC(2aaf765e) SHA1(fafe96834f5323fca71b8ab0c013f45c5c47182d), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.9e",  0x4, 0x100, CRC(598687e7) SHA1(c5045ddaab7123ff0a4c8f4c2489f9d70b63fc76), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.8e",  0x4, 0x100, CRC(68de17ed) SHA1(efefcb4ccdd012b767c4765304c6022b0c091066), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))

	ROM_REGION16_BE( 0x1000, "soundproms", 0)
	ROMX_LOAD( "mother_sounds_82s185.17f", 0x0, 0x800, CRC(0f49d40e) SHA1(40340833ab27ccb5b60baf44ad01930f204f5318), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "mother_sounds_82s185.19f", 0x0, 0x800, CRC(a041ce92) SHA1(9bc92992de22b830e479933c50650c7dc23f5713), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )
	ROMX_LOAD( "mother_sounds_82s185.16f", 0x1, 0x800, CRC(75b1749f) SHA1(71da8224e72fb5250e2097666a47314d0a818ee9), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "mother_sounds_82s185.18f", 0x1, 0x800, CRC(ae4c78ae) SHA1(76fdb8eab372065882caaa5b93ce9e21efe54da5), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )

	ROM_REGION( 0x1000, "videoproms", 0 )
	/* Horizontal and vertical video timing PROMs */
	ROM_LOAD( "rotate_video_board_82s129.4h", 0x000, 0x100, CRC(6ed1b8d3) SHA1(c392855e1ff8637e551891aefd6c6a3bbcf273a5) )
	ROM_LOAD( "rotate_video_board_82s129.5h", 0x100, 0x100, CRC(32d839d4) SHA1(7b82e5c1ea5a351eba60c19560a10e4702ae2c22) )

	/* Four sets of (identical) fill circuit state PROMs */
	ROM_LOAD( "fill_board_82s129.2p",  0x200, 0x100, CRC(c525b091) SHA1(bd172ed3d15f410d09017207ec68faf1d1cb4512) )
	ROM_LOAD( "fill_board_82s129.3p",  0x300, 0x100, CRC(7ed6c192) SHA1(b02bbcd0e04d3a6c54bec755df55fe6f97d9513f) )
	ROM_LOAD( "fill_board_82s129.4p",  0x400, 0x100, CRC(6b572b73) SHA1(4a065cb05c12ce34e5598341e0de0cc571b2d387) )
	ROM_LOAD( "fill_board_82s129.5p",  0x200, 0x100, CRC(c525b091) SHA1(bd172ed3d15f410d09017207ec68faf1d1cb4512) )
	ROM_LOAD( "fill_board_82s129.6p",  0x300, 0x100, CRC(7ed6c192) SHA1(b02bbcd0e04d3a6c54bec755df55fe6f97d9513f) )
	ROM_LOAD( "fill_board_82s129.7p",  0x400, 0x100, CRC(6b572b73) SHA1(4a065cb05c12ce34e5598341e0de0cc571b2d387) )
	ROM_LOAD( "fill_board_82s129.9p",  0x200, 0x100, CRC(c525b091) SHA1(bd172ed3d15f410d09017207ec68faf1d1cb4512) )
	ROM_LOAD( "fill_board_82s129.10p", 0x300, 0x100, CRC(7ed6c192) SHA1(b02bbcd0e04d3a6c54bec755df55fe6f97d9513f) )
	ROM_LOAD( "fill_board_82s129.11p", 0x400, 0x100, CRC(6b572b73) SHA1(4a065cb05c12ce34e5598341e0de0cc571b2d387) )
	ROM_LOAD( "fill_board_82s129.12p", 0x200, 0x100, CRC(c525b091) SHA1(bd172ed3d15f410d09017207ec68faf1d1cb4512) )
	ROM_LOAD( "fill_board_82s129.13p", 0x300, 0x100, CRC(7ed6c192) SHA1(b02bbcd0e04d3a6c54bec755df55fe6f97d9513f) )
	ROM_LOAD( "fill_board_82s129.14p", 0x400, 0x100, CRC(6b572b73) SHA1(4a065cb05c12ce34e5598341e0de0cc571b2d387) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY_OPTIONAL( "cubeqst", 0, SHA1(d0e24bb5d0ae424e0816110ec7d6b21189044d57) )
ROM_END


ROM_START( cubeqsta )
	ROM_REGION( 0x200000, "main_cpu", 0 )
	ROM_LOAD16_BYTE( "ep00_h001.1a", 0x000000, 0x02000, CRC(2a7add5a) SHA1(72a2dbbec2d4c9884cdde821c9fd377c17f9dca2) )
	ROM_LOAD16_BYTE( "ep00_l001.1b", 0x000001, 0x02000, CRC(7fd171f6) SHA1(c07d08d7f1b587e928e1977337c16c69e8a0302a) )
	ROM_LOAD16_BYTE( "ep04_h001.2a", 0x004000, 0x02000, CRC(e93678ee) SHA1(18c39154ba260935780686f777aebd0afe134793) )
	ROM_LOAD16_BYTE( "ep04_l001.2b", 0x004001, 0x02000, CRC(a8a2587c) SHA1(1b3e9c2d25eb074afe9eefdf5e0215b5acc527bd) )
	ROM_LOAD16_BYTE( "ep08_h001.3a", 0x008000, 0x02000, CRC(4d1f7df9) SHA1(1a6cf51be671f55d278a91ed8ce80b9d576729be) )
	ROM_LOAD16_BYTE( "ep08_l001.3b", 0x008001, 0x02000, CRC(f915f83c) SHA1(ed8503feda9af4382201a75bd1c155d65a38a7b5) )
	ROM_LOAD16_BYTE( "ep0c_h001.4a", 0x00c000, 0x02000, CRC(e37d1efc) SHA1(57f1d488a1f01bbc3dc81d6d1f013e87c86001a2) )
	ROM_LOAD16_BYTE( "ep0c_l001.4b", 0x00c001, 0x02000, CRC(901852ba) SHA1(9ab484a27707d1c551a96e8cba4f689034026af5) )
	ROM_LOAD16_BYTE( "ep10_h001.5a", 0x010000, 0x02000, CRC(3a61b1ae) SHA1(c44b5bb5c61c6d77b08c3429171f4347e2fc91c2) )
	ROM_LOAD16_BYTE( "ep10_l001.5b", 0x010001, 0x02000, CRC(35580dcd) SHA1(992073325ed34e0f0cd90698bd40c705e0e01343) )
	ROM_LOAD16_BYTE( "ep14_h001.6a", 0x014000, 0x02000, CRC(76c8f76b) SHA1(2513c813af014b9f9f57ed58561bcd85aa5d3178) )
	ROM_LOAD16_BYTE( "ep14_l001.6b", 0x014001, 0x02000, CRC(aa884571) SHA1(17ba0818b7ef229e46883b0b60ceac637a17e78b) )
	ROM_LOAD16_BYTE( "e018_h001.7a", 0x018000, 0x02000, CRC(a3d4bc13) SHA1(575b132a0be9cb189981c23763fa469b6345c395) )
	ROM_LOAD16_BYTE( "ep18_l001.7b", 0x018001, 0x02000, CRC(913f1bbf) SHA1(344155a9c0c6ecc24568c69ae0aeec18e93957f1) )
	ROM_LOAD16_BYTE( "ep1c_h001.8a", 0x01c000, 0x02000, CRC(e5a53800) SHA1(47591c588b7cb88c83e1f78f9b02472c360a617e) )
	ROM_LOAD16_BYTE( "ep1c_l001.8b", 0x01c001, 0x02000, CRC(39804640) SHA1(0f09cf6ef73a9199318278df15983a0cc01ece0d) )

	ROM_REGION64_BE( 0x1000, "rotate_cpu", 0)
	ROMX_LOAD( "rotate_video_board_82s131.8h",  0x0, 0x200, CRC(1dd1cc65) SHA1(3c83d48f90ba70e81c0d85fb182527afe1f1f68d), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.9h",  0x0, 0x200, CRC(2bbf0aa1) SHA1(1a4ef24a8dc27ac3b01f176aac9b1c1c194bd67b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.10f", 0x1, 0x200, CRC(89b234eb) SHA1(23d84873b4a5088130fff960b3caded329ea70b5), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.10h", 0x1, 0x200, CRC(d86561b2) SHA1(ebc5374568d9b598237d98f0fb95b68b36a3ddff), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.7h",  0x2, 0x200, CRC(4e0909e2) SHA1(e97dcbd23e0d9eccc138e62334e016b1b333dec3), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12k", 0x2, 0x200, CRC(84db792f) SHA1(03858d92f4ad5aae39ad1925738df958cc4dd020), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12j", 0x3, 0x200, CRC(661350ba) SHA1(f1ccc31c7ce1d71f6128c575d508aa04a4a5e15f), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12d", 0x3, 0x200, CRC(c60762ec) SHA1(d113eb1ce0db4d10a55057332fb17852b312396c), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12e", 0x4, 0x200, CRC(605b924c) SHA1(899103f20ccec68e432c4abf11f554b0600f8b3c), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12c", 0x4, 0x200, CRC(fbb881b7) SHA1(b8c1592cfa082fa7dc4abd5392816ba923760cbf), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12h", 0x5, 0x200, CRC(6fb1d6f0) SHA1(5a0bf681aedde073f0164264620085e9f40eca9e), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "rotate_video_board_82s131.12f", 0x5, 0x200, CRC(5c00d30e) SHA1(ad7bbcce9eff348f78cb6e0782faa175ca928c11), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))

	ROM_REGION64_BE( 0x800, "line_cpu", 0)
	ROMX_LOAD( "line_drawer_82s129_u27.6b", 0x0, 0x100, CRC(a9777c19) SHA1(dcb83cc5fce3ff9760dfdfce078895ef71558f67), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u28.3b", 0x0, 0x100, CRC(2248790c) SHA1(8473276bcc154a440745b714a31c89c459fef411), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u29.3c", 0x1, 0x100, CRC(6accc743) SHA1(a5263a021c594117c14a3d4fbc207a4043001172), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u30.3f", 0x1, 0x100, CRC(ba02ac36) SHA1(89473146f04f60bbed6644603ddc646b3d4e04eb), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u31.2j", 0x2, 0x100, CRC(bbf50d89) SHA1(f49d4322123c3d4a7d6f664b7164dfe24ddd6eed), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u5.3g",  0x2, 0x100, CRC(fa5c239b) SHA1(bbf526362af263aefea87341538157f4ecad686f), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u6.2g",  0x3, 0x100, CRC(59874b50) SHA1(d304b4546c187b8c73aaf8887f0037426e60dd5f), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u7.3h",  0x3, 0x100, CRC(886956d6) SHA1(ee150a9f99f3177fcc94be5af3f87433f39b311f), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u8.3j",  0x4, 0x100, CRC(8d2b17ad) SHA1(f7408104f75e292539aaec688e220c143f762f67), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "line_drawer_82s129_u9.3k",  0x4, 0x100, CRC(0a678253) SHA1(06d7a8f9556eb156ae03184772e76c84ab8d75ea), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))

	ROM_REGION64_BE( 0x800, "sound_cpu", 0)
	ROMX_LOAD( "mother_sounds_82s129.7e",  0x0, 0x100, CRC(cf89ad06) SHA1(2f04ca6d57a8c425f51a288f553841db680f5a7e), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.7f",  0x0, 0x100, CRC(5208036d) SHA1(ec29af4c10a098fb25424d1a5187a7bd2ec9fb92), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.11f", 0x1, 0x100, CRC(af327360) SHA1(2b45358a39daaf5721b5a94c4fd55bd3d0d90ec3), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.11e", 0x1, 0x100, CRC(b50d3e43) SHA1(9f2c071c6940767d20a563f6f2c9224e1896a5bf), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.9f",  0x2, 0x100, CRC(cd93e3c9) SHA1(f0c11db65ddf2096f6df424b7747242d9e3155da), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.10f", 0x2, 0x100, CRC(0e8b8df7) SHA1(37426ffdaf2d21273ac7e88789206fdafb14bc83), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.8f",  0x3, 0x100, CRC(07deae27) SHA1(c6a2d7cbf4ea4120f43ae088a3de90bb7a42d4a4), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.10e", 0x3, 0x100, CRC(2aaf765e) SHA1(fafe96834f5323fca71b8ab0c013f45c5c47182d), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.9e",  0x4, 0x100, CRC(598687e7) SHA1(c5045ddaab7123ff0a4c8f4c2489f9d70b63fc76), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(7))
	ROMX_LOAD( "mother_sounds_82s129.8e",  0x4, 0x100, CRC(68de17ed) SHA1(efefcb4ccdd012b767c4765304c6022b0c091066), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(7))

	ROM_REGION16_BE( 0x1000, "soundproms", 0)
	ROMX_LOAD( "mother_sounds_82s185.17f", 0x0, 0x800, CRC(0f49d40e) SHA1(40340833ab27ccb5b60baf44ad01930f204f5318), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "mother_sounds_82s185.19f", 0x0, 0x800, CRC(a041ce92) SHA1(9bc92992de22b830e479933c50650c7dc23f5713), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )
	ROMX_LOAD( "mother_sounds_82s185.16f", 0x1, 0x800, CRC(75b1749f) SHA1(71da8224e72fb5250e2097666a47314d0a818ee9), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "mother_sounds_82s185.18f", 0x1, 0x800, CRC(ae4c78ae) SHA1(76fdb8eab372065882caaa5b93ce9e21efe54da5), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )

	ROM_REGION( 0x1000, "videoproms", 0 )
	/* Horizontal and vertical video timing PROMs */
	ROM_LOAD( "rotate_video_board_82s129.4h", 0x000, 0x100, CRC(6ed1b8d3) SHA1(c392855e1ff8637e551891aefd6c6a3bbcf273a5) )
	ROM_LOAD( "rotate_video_board_82s129.5h", 0x100, 0x100, CRC(32d839d4) SHA1(7b82e5c1ea5a351eba60c19560a10e4702ae2c22) )

	/* Four sets of (identical) fill circuit state PROMs */
	ROM_LOAD( "fill_board_82s129.2p",  0x200, 0x100, CRC(c525b091) SHA1(bd172ed3d15f410d09017207ec68faf1d1cb4512) )
	ROM_LOAD( "fill_board_82s129.3p",  0x300, 0x100, CRC(7ed6c192) SHA1(b02bbcd0e04d3a6c54bec755df55fe6f97d9513f) )
	ROM_LOAD( "fill_board_82s129.4p",  0x400, 0x100, CRC(6b572b73) SHA1(4a065cb05c12ce34e5598341e0de0cc571b2d387) )
	ROM_LOAD( "fill_board_82s129.5p",  0x200, 0x100, CRC(c525b091) SHA1(bd172ed3d15f410d09017207ec68faf1d1cb4512) )
	ROM_LOAD( "fill_board_82s129.6p",  0x300, 0x100, CRC(7ed6c192) SHA1(b02bbcd0e04d3a6c54bec755df55fe6f97d9513f) )
	ROM_LOAD( "fill_board_82s129.7p",  0x400, 0x100, CRC(6b572b73) SHA1(4a065cb05c12ce34e5598341e0de0cc571b2d387) )
	ROM_LOAD( "fill_board_82s129.9p",  0x200, 0x100, CRC(c525b091) SHA1(bd172ed3d15f410d09017207ec68faf1d1cb4512) )
	ROM_LOAD( "fill_board_82s129.10p", 0x300, 0x100, CRC(7ed6c192) SHA1(b02bbcd0e04d3a6c54bec755df55fe6f97d9513f) )
	ROM_LOAD( "fill_board_82s129.11p", 0x400, 0x100, CRC(6b572b73) SHA1(4a065cb05c12ce34e5598341e0de0cc571b2d387) )
	ROM_LOAD( "fill_board_82s129.12p", 0x200, 0x100, CRC(c525b091) SHA1(bd172ed3d15f410d09017207ec68faf1d1cb4512) )
	ROM_LOAD( "fill_board_82s129.13p", 0x300, 0x100, CRC(7ed6c192) SHA1(b02bbcd0e04d3a6c54bec755df55fe6f97d9513f) )
	ROM_LOAD( "fill_board_82s129.14p", 0x400, 0x100, CRC(6b572b73) SHA1(4a065cb05c12ce34e5598341e0de0cc571b2d387) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY_OPTIONAL( "cubeqst", 0, SHA1(d0e24bb5d0ae424e0816110ec7d6b21189044d57) )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, cubeqst,  0,       cubeqst, cubeqst, driver_device, 0, ROT0, "Simutrek", "Cube Quest (01/04/84)", 0 )
GAME( 1983, cubeqsta, cubeqst, cubeqst, cubeqst, driver_device, 0, ROT0, "Simutrek", "Cube Quest (12/30/83)", 0 )
