// license:BSD-3-Clause
// copyright-holders:James Wallace, Philip Bennett
/***********************************************************************************************************
Barcrest MPU4 Extension driver by J.Wallace, Phil Bennett and Anonymous.

--- Board Setup ---
For the Barcrest MPU4 Video system, the GAME CARD (cartridge) contains the MPU4 video bios in the usual ROM
space (occupying 16k), an interface card to connect an additional Video board, and a 6850 serial IO to
communicate with said board.

The VIDEO BOARD is driven by a 10mhz 68000 processor, and contains a 6840PTM, 6850 serial IO
(the other end of the communications), an SAA1099 for stereo sound and SCN2674 gfx chip.

The VIDEO CARTRIDGE plugs into the video board, and contains the program ROMs for the video based game.
Like the MPU4 game card, in some cases an extra OKI sound chip is added to the video board's game card,
as well as extra RAM.
There is a protection chip similar to and replacing the MPU4 Characteriser.

No video card schematics ever left the PCB factory, but some decent scans of the board have been made,
now also available for review.

Additional: 68k HALT line is tied to the reset circuit of the MPU4.

--- Preliminary MPU4 Video Interface Card Memorymap  ---

   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+--------------------------------------------------------------------------
 0000-07FF |R/W| D D D D D D D D | 2k RAM
-----------+---+-----------------+--------------------------------------------------------------------------
 0800      |R/W|                 | ACIA 6850
-----------+---+-----------------+--------------------------------------------------------------------------
 0900-     |R/W| D D D D D D D D | MC6840 PTM IC2


  Clock1 <--------------------------------------
     |                                          |
     V                                          |
  Output1 ---> Clock2                           |
                                                |
               Output2 --+-> Clock3             |
                         |                      |
                         |   Output3 ---> 'to audio amp' ??
                         |
                         +--------> CA1 IC3 (

IRQ line connected to CPU

-----------+---+-----------------+--------------------------------------------------------------------------
 0A00-0A03 |R/W| D D D D D D D D | PIA6821 IC3 port A Lamp Drives 1,2,3,4,6,7,8,9 (sic)(IC14)
           |   |                 |
           |   |                 |          CA1 <= output2 from PTM6840 (IC2)
           |   |                 |          CA2 => alpha data
           |   |                 |
           |   |                 |          port B Lamp Drives 10,11,12,13,14,15,16,17 (sic)(IC13)
           |   |                 |
           |   |                 |          CB2 => alpha reset (clock on Dutch systems)
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0B00-0B03 |R/W| D D D D D D D D | PIA6821 IC4 port A = data for 7seg leds (pins 10 - 17, via IC32)
           |   |                 |
           |   |                 |             CA1 INPUT, 50 Hz input (used to generate IRQ)
           |   |                 |             CA2 OUTPUT, connected to pin2 74LS138 CE for multiplexer
           |   |                 |                        (B on LED strobe multiplexer)
           |   |                 |             IRQA connected to IRQ of CPU
           |   |                 |             port B
           |   |                 |                    PB7 = INPUT, serial port Receive data (Rx)
           |   |                 |                    PB6 = INPUT, reel A sensor
           |   |                 |                    PB5 = INPUT, reel B sensor
           |   |                 |                    PB4 = INPUT, reel C sensor
           |   |                 |                    PB3 = INPUT, reel D sensor
           |   |                 |                    PB2 = INPUT, Connected to CA1 (50Hz signal)
           |   |                 |                    PB1 = INPUT, undercurrent sense
           |   |                 |                    PB0 = INPUT, overcurrent  sense
           |   |                 |
           |   |                 |             CB1 INPUT,  used to generate IRQ on edge of serial input line
           |   |                 |             CB2 OUTPUT, enable signal for reel optics
           |   |                 |             IRQB connected to IRQ of CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0C00-0C03 |R/W| D D D D D D D D | PIA6821 IC5 port A
           |   |                 |
           |   |                 |                    PA0-PA7, INPUT AUX1 connector
           |   |                 |
           |   |                 |             CA2  OUTPUT, serial port Transmit line
           |   |                 |             CA1  not connected
           |   |                 |             IRQA connected to IRQ of CPU
           |   |                 |
           |   |                 |             port B
           |   |                 |
           |   |                 |                    PB0-PB7 INPUT, AUX2 connector
           |   |                 |
           |   |                 |             CB1  INPUT,  connected to PB7 (Aux2 connector pin 4)
           |   |                 |
           |   |                 |             CB2  OUTPUT, AY8913 chip select line
           |   |                 |             IRQB connected to IRQ of CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0D00-0D03 |R/W| D D D D D D D D | PIA6821 IC6
           |   |                 |
           |   |                 |  port A
           |   |                 |
           |   |                 |        PA0 - PA7 (INPUT/OUTPUT) data port AY8913 sound chip
           |   |                 |
           |   |                 |        CA1 INPUT,  not connected
           |   |                 |        CA2 OUTPUT, BC1 pin AY8913 sound chip
           |   |                 |        IRQA , connected to IRQ CPU
           |   |                 |
           |   |                 |  port B
           |   |                 |
           |   |                 |        PB0-PB3 OUTPUT, reel A
           |   |                 |        PB4-PB7 OUTPUT, reel B
           |   |                 |
           |   |                 |        CB1 INPUT,  not connected
           |   |                 |        CB2 OUTPUT, B01R pin AY8913 sound chip
           |   |                 |        IRQB , connected to IRQ CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0E00-0E03 |R/W| D D D D D D D D | PIA6821 IC7
           |   |                 |
           |   |                 |  port A
           |   |                 |
           |   |                 |        PA0-PA3 OUTPUT, reel C
           |   |                 |        PA4-PA7 OUTPUT, reel D
           |   |                 |        CA1     INPUT,  not connected
           |   |                 |        CA2     OUTPUT, A on LED strobe multiplexer
           |   |                 |        IRQA , connected to IRQ CPU
           |   |                 |
           |   |                 |  port B
           |   |                 |
           |   |                 |        PB0-PB6 OUTPUT mech meter 1-7 or reel E + F
           |   |                 |        PB7     Voltage drop sensor
           |   |                 |        CB1     INPUT, not connected
           |   |                 |        CB2     OUTPUT,mech meter 8
           |   |                 |        IRQB , connected to IRQ CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 0F00-0F03 |R/W| D D D D D D D D | PIA6821 IC8
           |   |                 |
           |   |                 | port A
           |   |                 |
           |   |                 |        PA0-PA7 INPUT  multiplexed inputs data
           |   |                 |
           |   |                 |        CA1     INPUT, not connected
           |   |                 |        CA2    OUTPUT, C on LED strobe multiplexer
           |   |                 |        IRQA           connected to IRQ CPU
           |   |                 |
           |   |                 | port B
           |   |                 |
           |   |                 |        PB0-PB7 OUTPUT  triacs outputs connector PL6
           |   |                 |        used for slides / hoppers
           |   |                 |
           |   |                 |        CB1     INPUT, not connected
           |   |                 |        CB2    OUTPUT, pin1 alpha display PL7 (clock signal)
           |   |                 |        IRQB           connected to IRQ CPU
           |   |                 |
-----------+---+-----------------+--------------------------------------------------------------------------
 4000-40FF |R/W| D D D D D D D D | RAM ?
-----------+---+-----------------+--------------------------------------------------------------------------
 BE00-BFFF |R/W| D D D D D D D D | RAM
-----------+---+-----------------+--------------------------------------------------------------------------
 C000-FFFF | R | D D D D D D D D | ROM
-----------+---+-----------------+--------------------------------------------------------------------------
Everything here is preliminary...  the boards are quite fussy with regards their self tests
and the timing may have to be perfect for them to function correctly.  (as the comms are
timer driven, the video is capable of various raster effects etc.)

TODO:
      - Correctly implement characteriser protection for each game.
      - Get the BWB games running
        * They have a slightly different 68k memory map. The 6850 is at e00000 and the 6840 is at e01000
        They appear to hang on the handshake with the MPU4 board
      - Layouts needed for the other working games, and DIP switches need checking/altering (no test mode?)
      - BWB Vid5 cabinets seem to have the speakers wired the other way according to test (left/right swapped)
      - BWB sampled sound seems to not play despite hookup.
 ***********************************************************************************************************/
#include "emu.h"
#include "mpu4.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"

#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/meters.h"
#include "machine/nvram.h"
#include "machine/roc10937.h"
#include "machine/steppers.h"

#include "sound/ay8910.h"
#include "sound/okim6376.h"
#include "sound/saa1099.h"

#include "video/ef9369.h"
#include "video/scn2674.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "crmaze2p.lh"
#include "crmaze4p.lh"
#include "v4addlad.lh"
#include "v4barqst.lh"
#include "v4dbltak.lh"
#include "v4psi.lh"
#include "v4strike.lh"

#define VERBOSE (0)
#include "logmacro.h"


#define VIDEO_MASTER_CLOCK          XTAL(10'000'000)

namespace {

class mpu4vid_state : public mpu4_state
{
public:

	mpu4vid_state(const machine_config &mconfig, device_type type, const char *tag) :
		mpu4_state(mconfig, type, tag),
		m_videocpu(*this, "video"),
		m_scn2674(*this, "scn2674_vid"),
		m_vid_vidram(*this, "vid_vidram", 0x20000, ENDIANNESS_BIG),
		m_vid_mainram(*this, "vid_mainram"),
		m_acia_0(*this, "acia6850_0"),
		m_acia_1(*this, "acia6850_1"),
		m_ptm(*this, "6840ptm_68k"),
		m_trackx_port(*this, "TRACKX"),
		m_tracky_port(*this, "TRACKY"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_ef9369(*this, "ef9369")
	{
	}

	void mpu4_vid(machine_config &config);
	void mpu4_vid_strike(machine_config &config);
	void mpu4_vid_cheatchr(machine_config &config);

	void bwbvid(machine_config &config);

	void crmaze_base(machine_config &config);
	void crmaze(machine_config &config);

	void bwbvid_oki(machine_config &config);
	void bwbvid_oki_bt471(machine_config &config);

	void bwbvid_oki_bt471_german(machine_config &config);

	void mating(machine_config &config);
	void vid_oki(machine_config &config);

	void init_v4barqst2();
	void init_quidgrid();
	void init_v4barqst();
	void init_timemchn();
	void init_v4opt3();
	void init_eyesdown();

	void init_crmaze();
	void init_crmaze_flutter();


	void init_prizeinv();
	void init_strikeit();
	void init_v4wize();
	void init_turnover();
	//void init_adders();
	void init_mating();
	void init_skiltrek();
	void init_cybcas();
	void init_bwbhack();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<m68000_base_device> m_videocpu;
	optional_device<scn2674_device> m_scn2674;
	memory_share_creator<uint16_t> m_vid_vidram;
	optional_shared_ptr<uint16_t> m_vid_mainram;
	required_device<acia6850_device> m_acia_0;
	required_device<acia6850_device> m_acia_1;
	required_device<ptm6840_device> m_ptm;
	optional_ioport m_trackx_port;
	optional_ioport m_tracky_port;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_device<ef9369_device> m_ef9369;

	//Video
	uint8_t m_m6840_irq_state;
	uint8_t m_m6850_irq_state;
	uint16_t m_gfx_index;
	int8_t m_cur[2];

	SCN2674_DRAW_CHARACTER_MEMBER(display_pixels);
	void m6809_acia_irq(int state);
	void m68k_acia_irq(int state);
	void cpu1_ptm_irq(int state);
	void vid_o1_callback(int state);
	void vid_o2_callback(int state);
	void vid_o3_callback(int state);
	uint8_t pia_ic5_porta_track_r();
	void update_mpu68_interrupts(int state);
	uint16_t mpu4_vid_vidram_r(offs_t offset);
	void mpu4_vid_vidram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	EF9369_COLOR_UPDATE(ef9369_color_update);

	void vidcharacteriser_w(offs_t offset, uint8_t data);
	uint8_t vidcharacteriser_r(offs_t offset);
	void mpu_video_reset(int state);
	void vram_w(offs_t offset, uint8_t data);
	uint8_t vram_r(offs_t offset);
	void ic3ss_vid_w(offs_t offset, uint8_t data);

	void bwbvidoki_68k_base_map(address_map &map) ATTR_COLD;
	void bwbvidoki_68k_map(address_map &map) ATTR_COLD;
	void bwbvidoki_68k_bt471_map(address_map &map) ATTR_COLD;
	void bwbvid_68k_map(address_map &map) ATTR_COLD;
	void mpu4_68k_map_base(address_map &map) ATTR_COLD;
	void mpu4_68k_map(address_map &map) ATTR_COLD;
	void mpu4_68k_map_strike(address_map &map) ATTR_COLD;
	void mpu4_vram(address_map &map) ATTR_COLD;
	void mpu4oki_68k_map(address_map &map) ATTR_COLD;

	void mpu4_6809_map(address_map &map) ATTR_COLD;
	void mpu4_6809_german_map(address_map &map) ATTR_COLD;

	uint8_t vidcharacteriser_4k_lookup_r(offs_t offset);

	void hack_bwb_startup_protection();

	uint8_t mpu4_vid_bt_a00004_r(offs_t offset);
	void mpu4_vid_bt_a00002_w(offs_t offset, uint8_t data);
	void mpu4_vid_bt_a00008_w(offs_t offset, uint8_t data);
	uint8_t m_bt_palbase;
	uint8_t m_bt_which;
	uint8_t m_btpal_r[0x100];
	uint8_t m_btpal_g[0x100];
	uint8_t m_btpal_b[0x100];
};

/*************************************
 *
 *  Interrupt system
 *
 *************************************/

/* The interrupt system consists of a 74148 priority encoder
   with the following interrupt priorities.  A lower number
   indicates a lower priority:

    7 - Game Card
    6 - Game Card
    5 - Game Card
    4 - Game Card
    3 - 2674 AVDC
    2 - 6850 ACIA
    1 - 6840 PTM
    0 - Unused
*/


void mpu4vid_state::update_mpu68_interrupts(int state)
{
	m_videocpu->set_input_line(1, m_m6840_irq_state ? ASSERT_LINE : CLEAR_LINE);
	m_videocpu->set_input_line(2, m_m6850_irq_state ? ASSERT_LINE : CLEAR_LINE);
}

/* Communications with 6809 board */

void mpu4vid_state::m6809_acia_irq(int state)
{
	m_acia_1->write_cts(state);
	m_maincpu->set_input_line(M6809_IRQ_LINE, state);
}

void mpu4vid_state::m68k_acia_irq(int state)
{
	m_acia_0->write_cts(state);
	m_m6850_irq_state = state;
	update_mpu68_interrupts(1);
}


void mpu4vid_state::cpu1_ptm_irq(int state)
{
	m_m6840_irq_state = state;
	update_mpu68_interrupts(1);
}


void mpu4vid_state::vid_o1_callback(int state)
{
	m_ptm->set_c2(state); /* this output is the clock for timer2 */

	m_acia_0->write_txc(state);
	m_acia_0->write_rxc(state);
	m_acia_1->write_txc(state);
	m_acia_1->write_rxc(state);
}


void mpu4vid_state::vid_o2_callback(int state)
{
	m_ptm->set_c3(state); /* this output is the clock for timer3 */
}


void mpu4vid_state::vid_o3_callback(int state)
{
	m_ptm->set_c1(state); /* this output is the clock for timer1 */
}


static const gfx_layout mpu4_vid_char_8x8_layout =
{
	8,8,
	0x1000, /* 0x1000 tiles (128k of GFX RAM, 0x20 bytes per tile) */
	4,
	{ 0,8,16,24 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32},
	8*32
};

void mpu4vid_state::vram_w(offs_t offset, uint8_t data)
{
	m_vid_mainram[offset] = data | (m_vid_mainram[offset] & 0xff00);
}

uint8_t mpu4vid_state::vram_r(offs_t offset)
{
	return m_vid_mainram[offset];
}

void mpu4vid_state::mpu4_vram(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(mpu4vid_state::vram_r), FUNC(mpu4vid_state::vram_w));
}

SCN2674_DRAW_CHARACTER_MEMBER(mpu4vid_state::display_pixels)
{
	if (!lg)
	{
		uint16_t tile = m_vid_mainram[address & 0x7fff];
		const uint8_t *line = m_gfxdecode->gfx(m_gfx_index+0)->get_data(tile & 0xfff);
		int offset = m_gfxdecode->gfx(m_gfx_index+0)->rowbytes() * linecount;

		for (int i = 0; i < 8; i++)
		{
			uint8_t pen = line[offset + i];
			int extra = tile >> 12;

			if (m_ef9369)
			{
				// TODO: calculate instead?
				static const uint8_t lookup[256] = {
						0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
						0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
						0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2,
						0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
						0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 4, 4,
						0, 1, 0, 1, 4, 5, 4, 5, 0, 1, 0, 1, 4, 5, 4, 5,
						0, 0, 2, 2, 4, 4, 6, 6, 0, 0, 2, 2, 4, 4, 6, 6,
						0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
						0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8,
						0, 1, 0, 1, 0, 1, 0, 1, 8, 9, 8, 9, 8, 9, 8, 9,
						0, 0, 2, 2, 0, 0, 2, 2, 8, 8,10,10, 8, 8,10,10,
						0, 1, 2, 3, 0, 1, 2, 3, 8, 9,10,11, 8, 9,10,11,
						0, 0, 0, 0, 4, 4, 4, 4, 8, 8, 8, 8,12,12,12,12,
						0, 1, 0, 1, 4, 5, 4, 5, 8, 9, 8, 9,12,13,12,13,
						0, 0, 2, 2, 4, 4, 6, 6, 8, 8,10,10,12,12,14,14,
						0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
					};

				bitmap.pix(y, x + i) = m_palette->pen(lookup[(extra << 4) | (pen & 0xf)]);
			}
			else
			{
				bitmap.pix(y, x + i) = m_palette->pen((extra<<4) | (pen & 0xf));
			}
		}
	}
}


uint16_t mpu4vid_state::mpu4_vid_vidram_r(offs_t offset)
{
	return m_vid_vidram[offset];
}


void mpu4vid_state::mpu4_vid_vidram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vid_vidram[offset]);
	offset <<= 1;
	m_gfxdecode->gfx(m_gfx_index+0)->mark_dirty(offset/0x20);
}


void mpu4vid_state::video_start()
{
	memset(m_vid_vidram,0,0x20000);

	/* find first empty slot to decode gfx */
	for (m_gfx_index = 0; m_gfx_index < MAX_GFX_ELEMENTS; m_gfx_index++)
		if (m_gfxdecode->gfx(m_gfx_index) == nullptr)
			break;

	assert(m_gfx_index != MAX_GFX_ELEMENTS);

	/* create the char set (gfx will then be updated dynamically from RAM) */
	m_gfxdecode->set_gfx(
			m_gfx_index+0,
			std::make_unique<gfx_element>(
				m_palette,
				mpu4_vid_char_8x8_layout,
				reinterpret_cast<uint8_t *>(m_vid_vidram.target()),
				NATIVE_ENDIAN_VALUE_LE_BE(8,0),
				m_palette->entries() / 16,
				0));
}

EF9369_COLOR_UPDATE( mpu4vid_state::ef9369_color_update )
{
	m_palette->set_pen_color(entry, pal4bit(cc), pal4bit(cb), pal4bit(ca));
}

/*************************************
 *
 *  Trackball interface
 *
 *************************************/

uint8_t mpu4vid_state::pia_ic5_porta_track_r()
{
	/* The SWP trackball interface connects a standard trackball to the AUX1 port on the MPU4
	mainboard. As per usual, they've taken the cheap route here, reading and processing the
	raw quadrature signal from the encoder wheels for a 4 bit interface, rather than use any
	additional hardware to simplify matters. What makes matters worse is that there is a 45 degree rotation to take into account.
	For our purposes, two fake ports give the X and Y positions, which are then worked back into the signal levels.
	We invert the X and Y data at source due to the use of Schmitt triggers in the interface, which
	clean up the pulses and flip the active phase.*/

	LOG("%s: IC5 PIA Read of Port A (AUX1)\n",machine().describe_context());


	uint8_t data = m_aux1_port->read();

	int8_t dx = m_trackx_port->read();
	int8_t dy = m_tracky_port->read();

	m_cur[0] = dy + dx;
	m_cur[1] = dy - dx;

	uint8_t xa, xb, ya, yb;

	/* generate pulses for the input port (A and B are 1 unit out of phase for direction sensing)*/
	xa = ((m_cur[0] + 1) & 3) <= 1;
	xb = (m_cur[0] & 3) <= 1;
	ya = ((m_cur[1] + 1) & 3) <= 1;
	yb = (m_cur[1] & 3) <= 1;

	data |= (xa << 4); // XA
	data |= (ya << 5); // YA
	data |= (xb << 6); // XB
	data |= (yb << 7); // YB

	return data;
}


/*************************************
 *
 *  Input defintions
 *
 *************************************/
INPUT_PORTS_START( mpu4vid )
	PORT_START("ORANGE1") //0 - 7
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("00")//  20p level
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("01")// 100p level
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("02")// Token 1 level
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("03")// Token 2 level
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("04")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("05")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("06")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("07")

	PORT_START("ORANGE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("08")// 8 - 11 is JP Key
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("09")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("10")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("11")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("12")// 12 - 15 is % Key
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("13")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("14")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("15")

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("16")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("17")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("18")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("19")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("20")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox (Back) Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("24")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("25")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("26")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("27")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("28")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("29")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("30")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START1)

	PORT_START("DIL1")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "1 Pound for change" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "DIL202" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "DIL203" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Attract mode inhibit" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin alarm inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single coin entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A1_7")

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) //Lockouts, in same order as below
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")//PORT_IMPULSE(5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")//PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")//PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")//PORT_IMPULSE(5)
INPUT_PORTS_END


static INPUT_PORTS_START( crmaze )
	PORT_INCLUDE( mpu4vid )
	PORT_MODIFY("ORANGE1")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("ORANGE2")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x1F, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Right Yellow")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right Red")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Left Red")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Left Yellow")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("Getout Yellow")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Escape/Getout Red")/* Labelled Escape on cabinet, Getout in test */
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_CUSTOM)//XA
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_CUSTOM)//YA
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM)//XB
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM)//YB

	PORT_START("TRACKX")//FAKE
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT
	PORT_START("TRACKY")//FAKE
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT
INPUT_PORTS_END

static INPUT_PORTS_START( mating )
	PORT_INCLUDE( crmaze )

	PORT_MODIFY("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left Red")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Left Yellow")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Right Yellow")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Right Red")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

INPUT_PORTS_END

static INPUT_PORTS_START( barquest )
	PORT_INCLUDE( mpu4vid )
	PORT_MODIFY("ORANGE1")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("ORANGE2")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("C")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("B")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("A")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Pass")

	PORT_MODIFY("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("C")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("B")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("A")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Continue")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Collect")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START1)

	PORT_MODIFY("AUX1")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


static INPUT_PORTS_START( skiltrek )
	PORT_INCLUDE( mpu4vid )
	PORT_MODIFY("ORANGE1")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("ORANGE2")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Pass")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("C")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("B")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("A")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Continue")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("C")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("B")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("A")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0xE0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("AUX1")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( turnover )
	PORT_INCLUDE( mpu4vid )

	PORT_MODIFY("ORANGE1")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("ORANGE2")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Pass")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("C")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("B")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("A")

	PORT_MODIFY("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Continue")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("C")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("B")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("A")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START1)

	PORT_MODIFY("AUX1")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


static INPUT_PORTS_START( adders )
	PORT_INCLUDE( mpu4vid )
	PORT_MODIFY("ORANGE1")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("ORANGE2")
	PORT_BIT( 0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("C")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("B")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("A")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Pass")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Collect")

	PORT_MODIFY("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("C")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("B")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("A")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Continue")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START1)

	PORT_MODIFY("AUX1")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


static INPUT_PORTS_START( strike )
	PORT_INCLUDE( mpu4vid )
	PORT_MODIFY("ORANGE1")
	PORT_BIT( 0x0F, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("Freeze")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_NAME("Go On!")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("ORANGE2")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START2)  PORT_NAME("Play")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Green (Left)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Yellow (Left)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Red (Left)")

	PORT_MODIFY("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Help")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Green (Right)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Yellow (Right)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Red (Right)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("Collect")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START1)

	PORT_MODIFY("AUX1")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( bwbvid )
	PORT_START("ORANGE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM )// 20p level
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM )// Short token bottom level
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM )// 100p level
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM )// Long token bottom level
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM )// Prize token level sensor
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM )// Short token top level
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("ORANGE2")
	PORT_CONFNAME( 0x0F, 0x00, "Jackpot / Prize Key" )
	PORT_CONFSETTING(    0x00, "Not fitted"  )
	PORT_CONFSETTING(    0x01, "3 GBP"  )
	PORT_CONFSETTING(    0x02, "4 GBP"  )
	PORT_CONFSETTING(    0x08, "5 GBP"  )
	PORT_CONFSETTING(    0x03, "6 GBP"  )
	PORT_CONFSETTING(    0x04, "6 GBP Token"  )
	PORT_CONFSETTING(    0x05, "8 GBP"  )
	PORT_CONFSETTING(    0x06, "8 GBP Token"  )
	PORT_CONFSETTING(    0x07, "10 GBP"  )
	PORT_CONFSETTING(    0x09, "15 GBP"  )
	PORT_CONFSETTING(    0x0A, "25 GBP"  )
	PORT_CONFSETTING(    0x0B, "25 GBP - Licensed Betting Office Profile"  )
	PORT_CONFSETTING(    0x0C, "35 GBP"  )
	PORT_CONFSETTING(    0x0D, "70 GBP"  )
	PORT_CONFSETTING(    0x0E, "Reserved 0e"  )
	PORT_CONFSETTING(    0x0F, "Reserved 0f"  )

	PORT_CONFNAME( 0xF0, 0x00, "Percentage Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 68% (Invalid for UK Games)"  )
	PORT_CONFSETTING(    0x10, "70" )
	PORT_CONFSETTING(    0x20, "72" )
	PORT_CONFSETTING(    0x30, "74" )
	PORT_CONFSETTING(    0x40, "76" )
	PORT_CONFSETTING(    0x50, "78" )
	PORT_CONFSETTING(    0x60, "80" )
	PORT_CONFSETTING(    0x70, "82" )
	PORT_CONFSETTING(    0x80, "84" )
	PORT_CONFSETTING(    0x90, "86" )
	PORT_CONFSETTING(    0xA0, "88" )
	PORT_CONFSETTING(    0xB0, "90" )
	PORT_CONFSETTING(    0xC0, "92" )
	PORT_CONFSETTING(    0xD0, "94" )
	PORT_CONFSETTING(    0xE0, "96" )
	PORT_CONFSETTING(    0xF0, "98" )

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Button 9")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Button 10")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("Button 11")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER)   PORT_NAME("Button 12")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_CUSTOM)  // Prize Shelf Opto
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox (Back) Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Button 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Button 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Button 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Button 4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Button 5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Button 6")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Button 7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START1) //Button 8

	PORT_START("DIL1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "High Token Payout Proportion" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x08, 0x00, "Low Token Payout Proportion" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0xF0, 0x00, "Target Percentage (if key not fitted)" )PORT_DIPLOCATION("DIL1:05,06,07,08")
	PORT_DIPSETTING(    0x00, "Unset (Program Optimum)"  )
	PORT_DIPSETTING(    0x10, "70" )
	PORT_DIPSETTING(    0x20, "72" )
	PORT_DIPSETTING(    0x30, "74" )
	PORT_DIPSETTING(    0x40, "76" )
	PORT_DIPSETTING(    0x50, "78" )
	PORT_DIPSETTING(    0x60, "80" )
	PORT_DIPSETTING(    0x70, "82" )
	PORT_DIPSETTING(    0x80, "84" )
	PORT_DIPSETTING(    0x90, "86" )
	PORT_DIPSETTING(    0xA0, "88" )
	PORT_DIPSETTING(    0xB0, "90" )
	PORT_DIPSETTING(    0xC0, "92" )
	PORT_DIPSETTING(    0xD0, "94" )
	PORT_DIPSETTING(    0xE0, "96" )
	PORT_DIPSETTING(    0xF0, "98" )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL2:01") //Clear MPU on some machines
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Cash Refill") PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Inhibit Win Banking" ) PORT_DIPLOCATION("DIL2:03") //If on, wins are paid live, as opposed to stored
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "High Prize Payout proportion (if available)" ) PORT_DIPLOCATION("DIL2:04") //Non Prize machines use this to inhibit OCD attract mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x10, 0x00, "Halt Payout when Empty" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Excess Token Lockout" ) PORT_DIPLOCATION("DIL2:07") //If an 'arcade' ROM, this flips, or is unused entirely.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single Credit Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) //Lockouts, in same order as below
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN5) PORT_NAME("Token")// If valid, then 0x04 is unused, 0x01 is token lockout, 0x02 is all other lockouts.
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")
INPUT_PORTS_END


static INPUT_PORTS_START( v4psi )
	PORT_INCLUDE( bwbvid )
	PORT_MODIFY("ORANGE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM )// 20p level
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM )// Short token bottom level
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM )// 100p level
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) // Payout Shelf opto
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("ORANGE2")
	// No. 17 to 24 according to test mode
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Fire")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("BLACK1")
	// No. 9 to 16
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("Continue 30p")

	PORT_MODIFY("BLACK2")
	// No. 1 to 8
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Collect")
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start 30p")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Start 50p")

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Allow Multiple Credits" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x02, DEF_STR( No  ) )
	PORT_DIPNAME( 0x04, 0x00, "Payout when Empty?" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x04, DEF_STR( No  ) )
	PORT_DIPNAME( 0x08, 0x00, "Bank Win" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x08, "Pay Live" )
	PORT_DIPNAME( 0x10, 0x00, "Collect Mode" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, "Credit and Bank" )
	PORT_DIPSETTING(    0x10, "Bank Only" )
	PORT_DIPNAME( 0x20, 0x00, "Hall of Fame Entry" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, "Full Names" )
	PORT_DIPSETTING(    0x20, "Initials" )
	PORT_DIPNAME( 0x40, 0x00, "Long Game" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x80, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( v4tetris )
	PORT_INCLUDE( bwbvid )

	PORT_MODIFY("ORANGE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM )// 20p level
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM )// Short token bottom level
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM )// 100p level
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM )// Long token bottom level
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM )// Prize token level sensor
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM )// Short token top level
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )


	PORT_MODIFY("ORANGE2")
	PORT_BIT(0xFF, IP_ACTIVE_HIGH, IPT_UNUSED)


	PORT_MODIFY("BLACK1")
	// no up also according to cabinet panel
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	// buttons are actually repeated on both left and right on the cabinet panel,
	// with joystick at center
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Rotate Left")
	// left of main screen cab
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start (Practice Mode)")

	PORT_MODIFY("BLACK2")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Rotate Right")
	// right of main screen cab
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Start (Prize Mode)")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Allow Multiple Credits" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x02, DEF_STR( No  ) )
	PORT_DIPNAME( 0x04, 0x00, "Demo Mode" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, "Continuous" )
	PORT_DIPSETTING(    0x04, "Cancelled @5min" )
	PORT_DIPNAME( 0x08, 0x00, "Odd 10p" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, "Lost @2min" )
	PORT_DIPSETTING(    0x08, "Never Lost" )
	PORT_DIPNAME( 0x10, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "DIL205" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "DIL207" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes  ) )

INPUT_PORTS_END


static INPUT_PORTS_START( v4pztet )
	PORT_INCLUDE( v4tetris )

	PORT_MODIFY("BLACK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start (Practice Mode)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Start (Prize Mode)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM )  // Prize Shelf Opto

	PORT_MODIFY("BLACK2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Rotate Left")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Rotate Right")
INPUT_PORTS_END


static INPUT_PORTS_START( v4bulblx )
	PORT_INCLUDE( v4pztet )

	PORT_MODIFY("BLACK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Start (Practice Mode)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start (Prize Mode)")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_CUSTOM)  // Prize Shelf Opto
INPUT_PORTS_END


static INPUT_PORTS_START( v4vgpok )
	PORT_INCLUDE( bwbvid )

	PORT_MODIFY("BLACK1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL ) PORT_NAME("Start / Deal / Draw")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) // Stake
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) // Collect
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH )

	PORT_MODIFY("BLACK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
INPUT_PORTS_END


static INPUT_PORTS_START( v4big40 )
	PORT_INCLUDE( bwbvid )

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("Swop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start / Deal / Draw")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_CUSTOM)  // Prize Shelf Opto
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox (Back) Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_MODIFY("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Cancel/Collect")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Hi/Twist")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("Lo/Stick")

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x80, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes  ) )

INPUT_PORTS_END

static INPUT_PORTS_START( v4mdice )
	PORT_INCLUDE( bwbvid )
	PORT_MODIFY("ORANGE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM )// 20p level
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM )// Short token bottom level
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM )// 100p level
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM )// Long token bottom level
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM )// Prize token level sensor
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM )// Short token top level
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("BLACK1")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("BLACK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Cancel/Collect")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hold/Nudge A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hold/Nudge B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hold/Nudge C")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Hi")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Lo")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Exchange")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_MODIFY("DIL1")
	PORT_DIPNAME( 0x01, 0x00, "Play Jingle" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x01, DEF_STR( No  ) )
	PORT_DIPNAME( 0x02, 0x00, u8"£8 Advert?" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x02, DEF_STR( No  ) )
	PORT_DIPNAME( 0x04, 0x00, "High Token Payout Proportion" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x08, 0x00, "Low Token Payout Proportion" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0xF0, 0x00, "Target Percentage (if key not fitted)" )PORT_DIPLOCATION("DIL1:05,06,07,08")
	PORT_DIPSETTING(    0x00, "Unset (Program Optimum)"  )
	PORT_DIPSETTING(    0x10, "70" )
	PORT_DIPSETTING(    0x20, "72" )
	PORT_DIPSETTING(    0x30, "74" )
	PORT_DIPSETTING(    0x40, "76" )
	PORT_DIPSETTING(    0x50, "78" )
	PORT_DIPSETTING(    0x60, "80" )
	PORT_DIPSETTING(    0x70, "82" )
	PORT_DIPSETTING(    0x80, "84" )
	PORT_DIPSETTING(    0x90, "86" )
	PORT_DIPSETTING(    0xA0, "88" )
	PORT_DIPSETTING(    0xB0, "90" )
	PORT_DIPSETTING(    0xC0, "92" )
	PORT_DIPSETTING(    0xD0, "94" )
	PORT_DIPSETTING(    0xE0, "96" )
	PORT_DIPSETTING(    0xF0, "98" )

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Cash Refill") PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Inhibit Win Banking" ) PORT_DIPLOCATION("DIL2:03") //If on, wins are paid live, as opposed to stored
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "High Prize Payout proportion (if available)" ) PORT_DIPLOCATION("DIL2:04") //Non Prize machines use this to inhibit OCD attract mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x10, 0x00, "Halt Payout when Empty" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Excess Token Lockout" ) PORT_DIPLOCATION("DIL2:07") //If an 'arcade' ROM, this flips, or is unused entirely.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single Credit Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

INPUT_PORTS_END

static INPUT_PORTS_START( v4cshinf )
	PORT_INCLUDE( v4mdice )

	PORT_MODIFY("DIL1")
	PORT_DIPNAME( 0x01, 0x00, "Lower Price of Play?" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x01, DEF_STR( No  ) )
	PORT_DIPNAME( 0x02, 0x00, "Higher Price of Play?" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x02, DEF_STR( No  ) )
	PORT_DIPNAME( 0x04, 0x00, "High Token Payout Proportion" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x08, 0x00, "Low Token Payout Proportion" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0xF0, 0x00, "Target Percentage (if key not fitted)" )PORT_DIPLOCATION("DIL1:05,06,07,08")
	PORT_DIPSETTING(    0x00, "Unset (Program Optimum)"  )
	PORT_DIPSETTING(    0x10, "70" )
	PORT_DIPSETTING(    0x20, "72" )
	PORT_DIPSETTING(    0x30, "74" )
	PORT_DIPSETTING(    0x40, "76" )
	PORT_DIPSETTING(    0x50, "78" )
	PORT_DIPSETTING(    0x60, "80" )
	PORT_DIPSETTING(    0x70, "82" )
	PORT_DIPSETTING(    0x80, "84" )
	PORT_DIPSETTING(    0x90, "86" )
	PORT_DIPSETTING(    0xA0, "88" )
	PORT_DIPSETTING(    0xB0, "90" )
	PORT_DIPSETTING(    0xC0, "92" )
	PORT_DIPSETTING(    0xD0, "94" )
	PORT_DIPSETTING(    0xE0, "96" )
	PORT_DIPSETTING(    0xF0, "98" )

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Cash Refill") PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Inhibit Win Banking" ) PORT_DIPLOCATION("DIL2:03") //If on, wins are paid live, as opposed to stored
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "High Prize Payout proportion (if available)" ) PORT_DIPLOCATION("DIL2:04") //Non Prize machines use this to inhibit OCD attract mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x10, 0x00, "Halt Payout when Empty" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Excess Token Lockout (Token ROMs)" ) PORT_DIPLOCATION("DIL2:07") //If an 'arcade' ROM, this flips, or is unused entirely.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single Credit Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

INPUT_PORTS_END


static INPUT_PORTS_START( v4reno )
	PORT_INCLUDE( v4mdice )

	PORT_MODIFY("DIL1")
	PORT_DIPNAME( 0x01, 0x00, "Set Price of Play?" ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x01, DEF_STR( No  ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0xF0, 0x00, "Target Percentage (if key not fitted)" )PORT_DIPLOCATION("DIL1:05,06,07,08")
	PORT_DIPSETTING(    0x00, "Unset (Program Optimum)"  )
	PORT_DIPSETTING(    0x10, "70" )
	PORT_DIPSETTING(    0x20, "72" )
	PORT_DIPSETTING(    0x30, "74" )
	PORT_DIPSETTING(    0x40, "76" )
	PORT_DIPSETTING(    0x50, "78" )
	PORT_DIPSETTING(    0x60, "80" )
	PORT_DIPSETTING(    0x70, "82" )
	PORT_DIPSETTING(    0x80, "84" )
	PORT_DIPSETTING(    0x90, "86" )
	PORT_DIPSETTING(    0xA0, "88" )
	PORT_DIPSETTING(    0xB0, "90" )
	PORT_DIPSETTING(    0xC0, "92" )
	PORT_DIPSETTING(    0xD0, "94" )
	PORT_DIPSETTING(    0xE0, "96" )
	PORT_DIPSETTING(    0xF0, "98" )

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Cash Refill") PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Inhibit Win Banking" ) PORT_DIPLOCATION("DIL2:03") //If on, wins are paid live, as opposed to stored
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "High Prize Payout proportion (if available)" ) PORT_DIPLOCATION("DIL2:04") //Non Prize machines use this to inhibit OCD attract mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x10, 0x00, "Halt Payout when Empty" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Excess Token Lockout (Token ROMs)" ) PORT_DIPLOCATION("DIL2:07") //If an 'arcade' ROM, this flips, or is unused entirely.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single Credit Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

INPUT_PORTS_END


static INPUT_PORTS_START( v4bigfrt )
	PORT_INCLUDE( bwbvid )

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("Fast/ High Play")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Cancel/Collect")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hold/Nudge A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hi")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hold/Nudge B")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Lo")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Hold/Nudge C")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Take It")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Leave It")

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Cash Refill") PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Inhibit Win Banking" ) PORT_DIPLOCATION("DIL2:03") //If on, wins are paid live, as opposed to stored
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Inhibit Attract Mode" ) PORT_DIPLOCATION("DIL2:04") //Non Prize machines use this to inhibit OCD attract mode
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "Halt Payout when Empty" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL2:07") //If an 'arcade' ROM, this flips, or is unused entirely.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single Credit Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( v4bubbnk )
	PORT_INCLUDE( bwbvid )

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Cancel/Nudge Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hold A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hold B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hold C")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Swop")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Hi")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Lo")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Cash Refill") PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Inhibit Win Banking" ) PORT_DIPLOCATION("DIL2:03") //If on, wins are paid live, as opposed to stored
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Inhibit Attract Mode" ) PORT_DIPLOCATION("DIL2:04") //Non Prize machines use this to inhibit OCD attract mode
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "Halt Payout when Empty" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Excess Token Lockout (Token ROMs)" ) PORT_DIPLOCATION("DIL2:07") //If an 'arcade' ROM, this flips, or is unused entirely.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single Credit Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( v4dbltak )
	PORT_INCLUDE( bwbvid )

	PORT_MODIFY("ORANGE1")
	PORT_CONFNAME( 0xc0, 0xc0, "Stake Key" )
	PORT_CONFSETTING(    0x80, "20p" )
	PORT_CONFSETTING(    0xc0, "25p" )
	// invalid settings
	PORT_CONFSETTING(    0x00, "Invalid (Not fitted)"  )
	PORT_CONFSETTING(    0x40, "Invalid" )

	PORT_MODIFY("ORANGE2")
	PORT_CONFNAME( 0x0f, 0x07, "Jackpot / Prize Key" )
	PORT_CONFSETTING(    0x08, "5 GBP"  )
	PORT_CONFSETTING(    0x07, "10 GBP"  )
	PORT_CONFSETTING(    0x09, "15 GBP"  )
	// invalid settings
	PORT_CONFSETTING(    0x00, "Invalid (Not fitted)"  )
	PORT_CONFSETTING(    0x01, "Invalid (3 GBP)"  )
	PORT_CONFSETTING(    0x02, "Invalid (4 GBP)"  )
	PORT_CONFSETTING(    0x03, "Invalid (6 GBP)"  )
	PORT_CONFSETTING(    0x04, "Invalid (6 GBP Token)"  )
	PORT_CONFSETTING(    0x05, "Invalid (8 GBP)"  )
	PORT_CONFSETTING(    0x06, "Invalid (8 GBP Token)"  )
	PORT_CONFSETTING(    0x0a, "Invalid (25 GBP)"  )
	PORT_CONFSETTING(    0x0b, "Invalid (25 GBP - Licensed Betting Office Profile)"  )
	PORT_CONFSETTING(    0x0c, "Invalid (35 GBP)"  )
	PORT_CONFSETTING(    0x0d, "Invalid (70 GBP)"  )
	PORT_CONFSETTING(    0x0e, "Invalid (Reserved 0e)"  )
	PORT_CONFSETTING(    0x0f, "Invalid (Reserved 0f)"  )
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED) // % Key not used

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("Swop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_NAME("Game Select")

	PORT_MODIFY("BLACK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Cancel/Collect")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hold A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hold B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hold C")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Hold D")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Hold E")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Hi/Twist")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Lo/Stick")

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Cash Refill") PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Inhibit Win Banking" ) PORT_DIPLOCATION("DIL2:03") //If on, wins are paid live, as opposed to stored
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "High Prize Percentage?" ) PORT_DIPLOCATION("DIL2:04") //Non Prize machines use this to inhibit OCD attract mode
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x08, DEF_STR( No  ) )
	PORT_DIPNAME( 0x10, 0x00, "Halt Payout when Empty" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( v4dbltak_perc ) // requires a valid % key
	PORT_INCLUDE( v4dbltak )

	PORT_MODIFY("ORANGE2")
	PORT_CONFNAME( 0xf0, 0x60, "Percentage Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 68% (Invalid for UK Games)"  )
	PORT_CONFSETTING(    0x10, "70" )
	PORT_CONFSETTING(    0x20, "72" )
	PORT_CONFSETTING(    0x30, "74" )
	PORT_CONFSETTING(    0x40, "76" )
	PORT_CONFSETTING(    0x50, "78" )
	PORT_CONFSETTING(    0x60, "80" )
	PORT_CONFSETTING(    0x70, "82" )
	PORT_CONFSETTING(    0x80, "84" )
	PORT_CONFSETTING(    0x90, "86" )
	PORT_CONFSETTING(    0xa0, "88" )
	PORT_CONFSETTING(    0xb0, "90" )
	PORT_CONFSETTING(    0xc0, "92" )
	PORT_CONFSETTING(    0xd0, "94" )
	PORT_CONFSETTING(    0xe0, "96" )
	PORT_CONFSETTING(    0xf0, "98" )
INPUT_PORTS_END


static INPUT_PORTS_START( v4mazbel )
	PORT_INCLUDE( bwbvid )

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Collect/Cancel")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hold A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hold B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hold C")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Hold/Swop")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Hi/Up")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Lo/Down")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Cash Refill") PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Inhibit Win Banking" ) PORT_DIPLOCATION("DIL2:03") //If on, wins are paid live, as opposed to stored
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Inhibit Attract Mode" ) PORT_DIPLOCATION("DIL2:04") //Non Prize machines use this to inhibit OCD attract mode
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "Halt Payout when Empty" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL2:07") //If an 'arcade' ROM, this flips, or is unused entirely.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single Credit Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
INPUT_PORTS_END


static INPUT_PORTS_START( v4redhtp )
	PORT_INCLUDE( bwbvid )

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("Swop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Deal/Draw")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Collect")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hold A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hold B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Hold C")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Hold D")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Hold E")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Hi")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Lo")

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Cash Refill") PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Inhibit Win Banking" ) PORT_DIPLOCATION("DIL2:03") //If on, wins are paid live, as opposed to stored
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Inhibit Attract Mode" ) PORT_DIPLOCATION("DIL2:04") //Non Prize machines use this to inhibit OCD attract mode
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "Halt Payout when Empty" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL2:07") //If an 'arcade' ROM, this flips, or is unused entirely.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single Credit Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( v4shpwnd )
	PORT_INCLUDE( bwbvid )

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start (L)")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Hold A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hold B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Hold C")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Cancel")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Collect")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Auto Start")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("Start (R)")

	PORT_MODIFY("DIL1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "Price of Play" ) PORT_DIPLOCATION("DIL1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x02, DEF_STR( High  ) )
	PORT_DIPNAME( 0x04, 0x00, "High Token Payout Proportion" ) PORT_DIPLOCATION("DIL1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x08, 0x00, "Low Token Payout Proportion" ) PORT_DIPLOCATION("DIL1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0xF0, 0x00, "Target Percentage (if key not fitted)" )PORT_DIPLOCATION("DIL1:05,06,07,08")
	PORT_DIPSETTING(    0x00, "Unset (Program Optimum)"  )
	PORT_DIPSETTING(    0x10, "70" )
	PORT_DIPSETTING(    0x20, "72" )
	PORT_DIPSETTING(    0x30, "74" )
	PORT_DIPSETTING(    0x40, "76" )
	PORT_DIPSETTING(    0x50, "78" )
	PORT_DIPSETTING(    0x60, "80" )
	PORT_DIPSETTING(    0x70, "82" )
	PORT_DIPSETTING(    0x80, "84" )
	PORT_DIPSETTING(    0x90, "86" )
	PORT_DIPSETTING(    0xA0, "88" )
	PORT_DIPSETTING(    0xB0, "90" )
	PORT_DIPSETTING(    0xC0, "92" )
	PORT_DIPSETTING(    0xD0, "94" )
	PORT_DIPSETTING(    0xE0, "96" )
	PORT_DIPSETTING(    0xF0, "98" )

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Cash Refill") PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Inhibit Win Banking" ) PORT_DIPLOCATION("DIL2:03") //If on, wins are paid live, as opposed to stored
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Spin Speed" ) PORT_DIPLOCATION("DIL2:04") //Non Prize machines use this to inhibit OCD attract mode
	PORT_DIPSETTING(    0x00, "Normal" )
	PORT_DIPSETTING(    0x08, "Fast" )
	PORT_DIPNAME( 0x10, 0x00, "Halt Payout when Empty" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "Excess Token Lockout (Token games only)" ) PORT_DIPLOCATION("DIL2:07") //If an 'arcade' ROM, this flips, or is unused entirely.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single Credit Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

INPUT_PORTS_END


static INPUT_PORTS_START( v4timebn )
	PORT_INCLUDE( bwbvid )

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x0F, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_CUSTOM)  // Prize Shelf Opto
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox (Back) Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_MODIFY("BLACK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Collect")
	//Will show as 'unused' in test, but give response when connected
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Hi")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Lo")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Luck")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_MODIFY("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Clear MPU Memory" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x02, 0x00, "Enable Cash Refill") PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x04, 0x00, "Inhibit Win Banking" ) PORT_DIPLOCATION("DIL2:03") //If on, wins are paid live, as opposed to stored
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "Inhibit Attract Mode" ) PORT_DIPLOCATION("DIL2:04") //Non Prize machines use this to inhibit OCD attract mode
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "Halt Payout when Empty" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Alarm Inhibit" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIL2:07") //If an 'arcade' ROM, this flips, or is unused entirely.
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "Single Credit Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( v4cybcas )
	PORT_INCLUDE( bwbvid )

	PORT_MODIFY("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("BLACK2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nehmen")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Halten A")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Halten B")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Halten C")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Ensatz")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Exchange")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Start Super")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start/Risiko")
INPUT_PORTS_END


void mpu4vid_state::mpu_video_reset(int state)
{
	m_ptm->reset();
	m_acia_1->reset();
}

/* machine start (called only once) */
void mpu4vid_state::machine_start()
{
	mpu4_config_common();

	/* setup communications */
	m_link7a_connected = true;
	m_link7b_connected = false;


	save_item(NAME( m_m6840_irq_state ));
	save_item(NAME( m_m6850_irq_state ));
	save_item(NAME( m_gfx_index ));
	save_item(NAME( m_cur ));

	save_item(NAME( m_bt_palbase ));
	save_item(NAME( m_bt_which ));
	save_item(NAME( m_btpal_r ));
	save_item(NAME( m_btpal_g ));
	save_item(NAME( m_btpal_b ));

}

void mpu4vid_state::machine_reset()
{
	m_vfd->reset(); //for debug ports only

	m_lamp_strobe    = 0;
	m_lamp_strobe2   = 0;
	m_led_strobe     = 0;

	m_IC23GC    = 0;
	m_IC23GB    = 0;
	m_IC23GA    = 0;
	m_IC23G1    = 1;
	m_IC23G2A   = 0;
	m_IC23G2B   = 0;

	//m_prot_col  = 0;
	//m_chr_counter    = 0;
	//m_chr_value     = 0;

	m_m6840_irq_state = 0;
	m_m6850_irq_state = 0;
}

void mpu4vid_state::mpu4_68k_map_base(address_map &map)
{
	map(0x000000, 0x7fffff).rom();
	map(0x800000, 0x80ffff).ram().share("vid_mainram");
//  map(0x810000, 0x81ffff).ram(); /* ? */
	map(0x900000, 0x900003).w("saa", FUNC(saa1099_device::write)).umask16(0x00ff).mirror(0x000004);
	map(0xa00001, 0xa00001).rw(m_ef9369, FUNC(ef9369_device::data_r), FUNC(ef9369_device::data_w)).umask16(0x00ff);
	map(0xa00002, 0xa00003).nopr(); // uses a clr instruction on address which generates a dummy read
	map(0xa00003, 0xa00003).w(m_ef9369, FUNC(ef9369_device::address_w)).umask16(0x00ff);
//  map(0xa00004, 0xa0000f).rw(FUNC(mpu4vid_state::mpu4_vid_unmap_r), FUNC(mpu4vid_state::mpu4_vid_unmap_w));
	map(0xb00000, 0xb0000f).rw(m_scn2674, FUNC(scn2674_device::read), FUNC(scn2674_device::write)).umask16(0x00ff);
	map(0xc00000, 0xc1ffff).rw(FUNC(mpu4vid_state::mpu4_vid_vidram_r), FUNC(mpu4vid_state::mpu4_vid_vidram_w)).share("vid_vidram");
	map(0xff8000, 0xff8003).rw(m_acia_1, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0xff9000, 0xff900f).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);
}

void mpu4vid_state::mpu4_68k_map(address_map& map)
{
	mpu4_68k_map_base(map);
	map(0xffd000, 0xffd00f).rw(FUNC(mpu4vid_state::vidcharacteriser_r), FUNC(mpu4vid_state::vidcharacteriser_w)).umask16(0x00ff);
}

void mpu4vid_state::mpu4_68k_map_strike(address_map& map)
{
	mpu4_68k_map_base(map);
	map(0xffd000, 0xffd00f).rw(FUNC(mpu4vid_state::vidcharacteriser_4k_lookup_r), FUNC(mpu4vid_state::vidcharacteriser_w)).umask16(0x00ff);
}


void mpu4vid_state::mpu4oki_68k_map(address_map &map)
{
	map(0x000000, 0x5fffff).rom(); //.nopw();
	map(0x600000, 0x63ffff).ram(); /* The Mating Game has an extra 256kB RAM on the program card */
//  map(0x640000, 0x7fffff).noprw(); /* Possible bug, reads and writes here */
	map(0x800000, 0x80ffff).ram().share("vid_mainram");
	map(0x900000, 0x900003).w("saa", FUNC(saa1099_device::write)).umask16(0x00ff).mirror(0x000004);
	map(0xa00001, 0xa00001).rw(m_ef9369, FUNC(ef9369_device::data_r), FUNC(ef9369_device::data_w)).umask16(0x00ff);
	map(0xa00002, 0xa00003).nopr(); // uses a clr instruction on address which generates a dummy read
	map(0xa00003, 0xa00003).w(m_ef9369, FUNC(ef9369_device::address_w)).umask16(0x00ff);
	map(0xb00000, 0xb0000f).rw(m_scn2674, FUNC(scn2674_device::read), FUNC(scn2674_device::write)).umask16(0x00ff);
	map(0xc00000, 0xc1ffff).rw(FUNC(mpu4vid_state::mpu4_vid_vidram_r), FUNC(mpu4vid_state::mpu4_vid_vidram_w)).share("vid_vidram");
	map(0xff8000, 0xff8003).rw(m_acia_1, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0xff9000, 0xff900f).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);

	map(0xffa040, 0xffa04f).rw("okicard", FUNC(mpu4_oki_sampled_sound::ic3_read), FUNC(mpu4_oki_sampled_sound::ic3_write)).umask16(0x00ff);  // 6840PTM on sampled sound board
	map(0xffa060, 0xffa067).rw("okicard", FUNC(mpu4_oki_sampled_sound::ic4_read), FUNC(mpu4_oki_sampled_sound::ic4_write)).umask16(0x00ff);  // 6821PIA on sampled sound board
	map(0xffd000, 0xffd00f).rw(FUNC(mpu4vid_state::vidcharacteriser_r), FUNC(mpu4vid_state::vidcharacteriser_w)).umask16(0x00ff);
//  map(0xfff000, 0xffffff).noprw(); /* Possible bug, reads and writes here */
}

void mpu4vid_state::bwbvid_68k_map(address_map &map)
{
	map(0x000000, 0x7fffff).rom();
	map(0x800000, 0x80ffff).ram().share("vid_mainram");
	map(0x810000, 0x81ffff).ram(); /* ? */
	map(0x900000, 0x900003).w("saa", FUNC(saa1099_device::write)).umask16(0x00ff).mirror(0x000004);
	map(0xa00001, 0xa00001).rw(m_ef9369, FUNC(ef9369_device::data_r), FUNC(ef9369_device::data_w)).umask16(0x00ff);
	map(0xa00002, 0xa00003).nopr(); // uses a clr instruction on address which generates a dummy read
	map(0xa00003, 0xa00003).w(m_ef9369, FUNC(ef9369_device::address_w)).umask16(0x00ff);
	map(0xb00000, 0xb0000f).rw(m_scn2674, FUNC(scn2674_device::read), FUNC(scn2674_device::write)).umask16(0x00ff);
	map(0xc00000, 0xc1ffff).rw(FUNC(mpu4vid_state::mpu4_vid_vidram_r), FUNC(mpu4vid_state::mpu4_vid_vidram_w)).share("vid_vidram");
	map(0xe00000, 0xe00003).rw(m_acia_1, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0xe01000, 0xe0100f).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);
}

void mpu4vid_state::bwbvidoki_68k_base_map(address_map &map)
{
	map(0x000000, 0x7fffff).rom();
	map(0x800000, 0x80ffff).ram().share("vid_mainram");
	map(0x810000, 0x81ffff).ram(); /* ? */
	map(0x900000, 0x900003).w("saa", FUNC(saa1099_device::write)).umask16(0x00ff).mirror(0x000004);

	map(0xb00000, 0xb0000f).rw(m_scn2674, FUNC(scn2674_device::read), FUNC(scn2674_device::write)).umask16(0x00ff);
	map(0xc00000, 0xc1ffff).rw(FUNC(mpu4vid_state::mpu4_vid_vidram_r), FUNC(mpu4vid_state::mpu4_vid_vidram_w)).share("vid_vidram");
	map(0xe00000, 0xe00003).rw(m_acia_1, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0xe01000, 0xe0100f).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);


	map(0xe02000, 0xe02007).rw("okicard", FUNC(mpu4_oki_sampled_sound::ic4_read), FUNC(mpu4_oki_sampled_sound::ic4_write)).umask16(0xff00);  // 6821PIA on sampled sound board
	map(0xe03000, 0xe0300f).rw("okicard", FUNC(mpu4_oki_sampled_sound::ic3_read), FUNC(mpu4_oki_sampled_sound::ic3_write)).umask16(0xff00);  // 6840PTM on sampled sound board

	map(0xe05000, 0xe05001).noprw();
}

void mpu4vid_state::bwbvidoki_68k_map(address_map& map)
{
	bwbvidoki_68k_base_map(map);

	map(0xa00001, 0xa00001).rw(m_ef9369, FUNC(ef9369_device::data_r), FUNC(ef9369_device::data_w)).umask16(0x00ff);
	map(0xa00002, 0xa00003).nopr(); // uses a clr instruction on address which generates a dummy read
	map(0xa00003, 0xa00003).w(m_ef9369, FUNC(ef9369_device::address_w)).umask16(0x00ff);
}

// TODO: use actual BT471 device, currently seems to not work when hooked up tho? fails detection?

uint8_t mpu4vid_state::mpu4_vid_bt_a00004_r(offs_t offset)
{
	// used by software to detect alt palette type? (same value is written prior to this)
	return 0xaf;
}

void mpu4vid_state::mpu4_vid_bt_a00008_w(offs_t offset, uint8_t data)
{
	m_bt_palbase = data;
	m_bt_which = 0;
}

void mpu4vid_state::mpu4_vid_bt_a00002_w(offs_t offset, uint8_t data)
{
	switch (m_bt_which)
	{
	case 0:
		m_btpal_r[m_bt_palbase] = data;
		[[fallthrough]]; // FIXME: really?
	case 1:
		m_btpal_g[m_bt_palbase] = data;
		[[fallthrough]]; // FIXME: really?
	case 2:
		m_btpal_b[m_bt_palbase] = data;
	}

	m_bt_which++;

	if (m_bt_which == 3)
	{
		m_bt_which = 0;
		m_palette->set_pen_color(m_bt_palbase, pal6bit(m_btpal_r[m_bt_palbase]), pal6bit(m_btpal_g[m_bt_palbase]), pal6bit(m_btpal_b[m_bt_palbase]));
		m_bt_palbase++;
	}
}

void mpu4vid_state::bwbvidoki_68k_bt471_map(address_map& map)
{
	bwbvidoki_68k_base_map(map);

	map(0xa00002, 0xa00003).w(FUNC(mpu4vid_state::mpu4_vid_bt_a00002_w)).umask16(0xffff);
	map(0xa00004, 0xa00005).r(FUNC(mpu4vid_state::mpu4_vid_bt_a00004_r)).umask16(0xffff);
	map(0xa00008, 0xa00009).w(FUNC(mpu4vid_state::mpu4_vid_bt_a00008_w)).umask16(0xffff);
}

/* TODO: Fix up MPU4 map*/
void mpu4vid_state::mpu4_6809_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0801).rw("acia6850_0", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x0880, 0x0881).noprw(); //Read/write here
	map(0x0900, 0x0907).rw("ptm_ic2", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0x0a00, 0x0a03).rw("pia_ic3", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0b00, 0x0b03).rw("pia_ic4", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0c00, 0x0c03).rw("pia_ic5", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0d00, 0x0d03).rw("pia_ic6", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0e00, 0x0e03).rw("pia_ic7", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0f00, 0x0f03).rw("pia_ic8", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x4000, 0x7fff).ram();
	map(0xbe00, 0xbfff).ram();
	map(0xc000, 0xffff).rom().region("maincpu",0);  /* 64k EPROM on board, only this region read */
}

void mpu4vid_state::mpu4_6809_german_map(address_map &map)
{
	mpu4_6809_map(map);

	map(0x4000, 0xbfff).ram();
}

void mpu4vid_state::mpu4_vid(machine_config &config)
{
	MC6809(config, m_maincpu, MPU4_MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4vid_state::mpu4_6809_map);


	AY8913(config, m_ay8913, MPU4_MASTER_CLOCK/4);
	m_ay8913->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8913->set_resistors_load(820, 0, 0);
	m_ay8913->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_ay8913->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);               /* confirm */

	mpu4_common(config);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(VIDEO_MASTER_CLOCK, (63*8)+(17*8), 0, (63*8), (37*8)+17, 0, (37*8));
	// note this directly affects the scanline counters used below, and thus the timing of everything
	screen.set_screen_update("scn2674_vid", FUNC(scn2674_device::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);

	SCN2674(config, m_scn2674, VIDEO_MASTER_CLOCK / 8);
	m_scn2674->intr_callback().set_inputline("video", M68K_IRQ_3);

	m_scn2674->set_character_width(8);
	m_scn2674->set_display_callback(FUNC(mpu4vid_state::display_pixels));
	m_scn2674->set_addrmap(0, &mpu4vid_state::mpu4_vram);

	M68000(config, m_videocpu, VIDEO_MASTER_CLOCK);
	m_videocpu->set_addrmap(AS_PROGRAM, &mpu4vid_state::mpu4_68k_map);
	m_videocpu->reset_cb().set(FUNC(mpu4vid_state::mpu_video_reset));


	PALETTE(config, m_palette).set_entries(ef9369_device::NUMCOLORS);

	EF9369(config, m_ef9369).set_color_update_callback(FUNC(mpu4vid_state::ef9369_color_update));

	PTM6840(config, m_ptm, VIDEO_MASTER_CLOCK / 10); /* 68k E clock */
	m_ptm->set_external_clocks(0, 0, 0);
	m_ptm->o1_callback().set(FUNC(mpu4vid_state::vid_o1_callback));
	m_ptm->o2_callback().set(FUNC(mpu4vid_state::vid_o2_callback));
	m_ptm->o3_callback().set(FUNC(mpu4vid_state::vid_o3_callback));
	m_ptm->irq_callback().set(FUNC(mpu4vid_state::cpu1_ptm_irq));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	/* Present on all video cards */
	saa1099_device &saa(SAA1099(config, "saa", 8000000));
	saa.add_route(0, "lspeaker", 0.5);
	saa.add_route(1, "rspeaker", 0.5);

	ACIA6850(config, m_acia_0, 0);
	m_acia_0->txd_handler().set("acia6850_1", FUNC(acia6850_device::write_rxd));
	m_acia_0->rts_handler().set("acia6850_1", FUNC(acia6850_device::write_dcd));
	m_acia_0->irq_handler().set(FUNC(mpu4vid_state::m6809_acia_irq));

	ACIA6850(config, m_acia_1, 0);
	m_acia_1->txd_handler().set("acia6850_0", FUNC(acia6850_device::write_rxd));
	m_acia_1->rts_handler().set("acia6850_0", FUNC(acia6850_device::write_dcd));
	m_acia_1->irq_handler().set(FUNC(mpu4vid_state::m68k_acia_irq));
}

void mpu4vid_state::mpu4_vid_cheatchr(machine_config &config)
{
	mpu4_vid(config);
	MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
	m_characteriser->set_cpu_tag("video");
	m_characteriser->set_allow_68k_cheat(true);
}

void mpu4vid_state::mpu4_vid_strike(machine_config& config)
{
	mpu4_vid(config);
	m_videocpu->set_addrmap(AS_PROGRAM, &mpu4vid_state::mpu4_68k_map_strike);

	MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
	m_characteriser->set_use_4k_table_sim(true);
}





void mpu4vid_state::crmaze_base(machine_config &config)
{
	mpu4_vid(config);
	m_pia5->readpa_handler().set(FUNC(mpu4vid_state::pia_ic5_porta_track_r));
	m_pia5->writepa_handler().set_nop();
	m_pia5->writepb_handler().set_nop();
}

void mpu4vid_state::crmaze(machine_config& config)
{
	crmaze_base(config);
	MPU4_CHARACTERISER_PAL(config, m_characteriser, 0);
	m_characteriser->set_cpu_tag("video");
	m_characteriser->set_allow_68k_cheat(true);

}

void mpu4vid_state::vid_oki(machine_config &config)
{
	//On MPU4 Video, the sound board is clocked via the 68k E clock,
	//and all samples are adjusted to fit the different clock speed.

	MPU4_OKI_SAMPLED_SOUND(config, m_okicard, VIDEO_MASTER_CLOCK/10);
	m_okicard->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_okicard->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	m_okicard->cb2_handler().set(FUNC(mpu4vid_state::pia_gb_cb2_w));

}

void mpu4vid_state::mating(machine_config &config)
{
	crmaze(config);
	vid_oki(config);
	m_videocpu->set_addrmap(AS_PROGRAM, &mpu4vid_state::mpu4oki_68k_map);
}



void mpu4vid_state::bwbvid(machine_config &config)
{
	mpu4_vid(config);
	m_videocpu->set_addrmap(AS_PROGRAM, &mpu4vid_state::bwbvid_68k_map);
}

void mpu4vid_state::bwbvid_oki(machine_config &config)
{
	mpu4_vid(config);
	m_videocpu->set_addrmap(AS_PROGRAM, &mpu4vid_state::bwbvidoki_68k_map);
	vid_oki(config);
}

void mpu4vid_state::bwbvid_oki_bt471(machine_config &config)
{
	mpu4_vid(config);
	m_videocpu->set_addrmap(AS_PROGRAM, &mpu4vid_state::bwbvidoki_68k_bt471_map);
	vid_oki(config);

	config.device_remove("ef9369");

	PALETTE(config.replace(), m_palette).set_entries(256);

}

void mpu4vid_state::bwbvid_oki_bt471_german(machine_config &config)
{
	bwbvid_oki_bt471(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4vid_state::mpu4_6809_german_map);
}


void mpu4vid_state::vidcharacteriser_w(offs_t offset, uint8_t data)
{
	if (m_characteriser)
		m_characteriser->write(offset, data);
}

uint8_t mpu4vid_state::vidcharacteriser_r(offs_t offset)
{
	uint8_t ret = 0x00;

	if (m_characteriser)
		ret = m_characteriser->read(offset);

	/* hack for 'invalid questions' error on time machine.. I guess it wants them to decode properly for startup check? */
	if (m_videocpu->pcbase()==0x283a)
	{
		ret = 0x00;
	}

	return ret;
}


uint8_t mpu4vid_state::vidcharacteriser_4k_lookup_r(offs_t offset)
{
	uint8_t ret = m_characteriser->read(offset);
	// hack for v4strike, otherwise it reports questions as invalid, even if they decode properly
	// is this a secondary security check, or are they mismatched for the version?
	// it writes '03' to the characteriser, (the question revision or coincidence?)
	// but expects 00 back for that check

	if (m_videocpu->pcbase() == 0x32c4)
	{
		if (m_videocpu->state_int(M68K_A4) == 0xaab54)
			ret = 0x00;
	}

	return ret;
}


/*
void mpu4vid_state::init_adders()
{
    m_reels = 0;//currently no hybrid games
    m_current_chr_table = adders_data;
}
*/

void mpu4vid_state::init_crmaze()
{
	m_reels = 0;//currently no hybrid games
}

void mpu4vid_state::init_crmaze_flutter()
{
	m_reels = 0;//currently no hybrid games
	m_reel_mux = FLUTTERBOX;
}

void mpu4vid_state::init_mating()
{
	m_reels = 0;//currently no hybrid games

	// TODOxx: m_current_chr_table = mating_data;
}

void mpu4vid_state::init_skiltrek()
{
	m_reels = 0;//currently no hybrid games
	// TODOxx: m_current_chr_table = skiltrek_data;
}

void mpu4vid_state::init_timemchn()
{
	m_reels = 0;//currently no hybrid games
	// TODOxx: m_current_chr_table = timemchn_data;
}

void mpu4vid_state::init_strikeit()
{
	m_led_extender = SIMPLE_CARD;
	m_reels = 0;//currently no hybrid games
}

void mpu4vid_state::init_turnover()
{
	m_reels = 0;//currently no hybrid games
	// TODOxx: m_current_chr_table = turnover_data;
}

void mpu4vid_state::init_eyesdown()
{
	m_reels = 0;//currently no hybrid games
	// TODOxx: m_current_chr_table = eyesdown_data;
}

void mpu4vid_state::init_quidgrid()
{
	m_reels = 0;//currently no hybrid games
	// TODOxx: m_current_chr_table = quidgrid_data;
}


void mpu4vid_state::hack_bwb_startup_protection()
{
	// there's no checksum that we need to fix after this
	// so we can't be sure there are no bad dumps?

	uint16_t *rom = (uint16_t*)memregion("video")->base();
	int len = memregion("video")->bytes()/2;

	uint16_t sequence[8] = { 0x3428, 0x0002, 0xb428, 0x0002, 0x671a, 0x2cbc, 0x0002, 0x4001 };

	for (int i = 0; i < len - 8; i++)
	{

		bool matched = true;

		for (int j = 0; j < 8; j++)
		{
			if (sequence[j] != rom[i + j])
				matched = false;
		}

		if (matched)
		{
			logerror("boot protection match found at %08x\n", i * 2);

			rom[i + 5] = 0x6618;
			rom[i + 6] = 0x4E71;
			rom[i + 7] = 0x4E71;
		}
	}
}



void mpu4vid_state::init_prizeinv()
{
	m_reels = 0;//currently no hybrid games
	// TODOxx: m_current_chr_table = prizeinv_data;

	hack_bwb_startup_protection();
}

/*
TODO: use official Barcrest / BWB letters here, some show them, eg D = protocol / datapak, Y = % Key needed
this is based on header info in a few games, could differ, check!

offset 0 : Option Byte

YtSi safD

D = Protocol / Datapak
f = fixed %
a = arcade
s = switchable (or use offset 1 value)
i = 'irish'
S = special (use offset 2)
t = token
Y = % key needed

offset 1: price of pay (per-game, used depending on above)

offset 2: special game type (per-game>)

---- --rg

g = Bingo Gala A
r = Bingo Rank R
*/


void mpu4vid_state::init_bwbhack()
{
	hack_bwb_startup_protection();


	uint8_t* rom = memregion("maincpu")->base();
	int len = memregion("maincpu")->bytes();

	if (len != 0x10000)
	{
		logerror("main CPU size is not 0x10000!\n");
		return;
	}

	for (int i = 0; i < 0x4000; i++)
	{
		if ((rom[i] != rom[i + 0x4000]) ||
			(rom[i] != rom[i + 0x8000]) ||
			(rom[i] != rom[i + 0xc000]))
			logerror("main CPU ROM data not mirroring as expected at %04x!\n", i);
	}

	// helper for sorting out sets
	uint8_t option = rom[0xc000];
	uint8_t price = rom[0xc001];
	uint8_t special = rom[0xc002];

	logerror("option byte is %02x\n", option);
	logerror("bit 0: Datapak     = %d\n", (option >> 0) & 1);
	logerror("bit 1: Fixed %%    = %d\n", (option >> 1) & 1);
	logerror("bit 2: Arcade      = %d\n", (option >> 2) & 1);
	logerror("bit 3: Switchable  = %d\n", (option >> 3) & 1);
	logerror("bit 4: Irish       = %d\n", (option >> 4) & 1);
	logerror("bit 5: Special     = %d\n", (option >> 5) & 1);
	logerror("bit 6: Token       = %d\n", (option >> 6) & 1);
	logerror("bit 7: Percent Key = %d\n", (option >> 7) & 1);

	if (!((option >> 3) & 1))
	{
		logerror("Switchable was not set, so use coinage setting %02x\n", price);
	}

	if ((option >> 5) & 1)
	{
		logerror("Special was set, so use special setting %02x\n", special);
	}
}



void mpu4vid_state::init_cybcas()
{
	//no idea what this should be, use blues boys table for now
	// TODOxx: m_bwb_chr_table1 = cybcas_data1;
	// TODOxx: m_current_chr_table = cybcas_data;

	hack_bwb_startup_protection();

	// hack out half the startup checks for now until we work out what they're checking!
	uint16_t *rom = (uint16_t*)memregion("video")->base();

	for (int i = 0x1e42; i < 0x1e74; i += 2)
		rom[i / 2] = 0x4e71;
}




void mpu4vid_state::init_v4barqst()
{
}

void mpu4vid_state::init_v4barqst2()
{
}

void mpu4vid_state::init_v4wize()
{
}

void mpu4vid_state::init_v4opt3()
{
}


#define VID_BIOS \
	ROM_LOAD("vid.p1",  0x00000, 0x10000,  CRC(e996bc18) SHA1(49798165640627eb31024319353da04380787b10))

ROM_START( v4bios )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS
ROM_END

ROM_START( v4psi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("in220mpu4.p1",  0x00000, 0x04000,  CRC(75ff4b1f) SHA1(a3adaad9a91c30fe6ff42dc2003c34a199b28807) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "in2-20p1.1",  0x000000, 0x10000, CRC(f34d9001) SHA1(2bae06f4a5a5510b15b918261ecb0de9e34a6b53) )
	ROM_LOAD16_BYTE( "in2-20p1.2",  0x000001, 0x10000, CRC(1dc931b4) SHA1(c46626183edd52c7938c5edee2395aacb49e0730) )
	ROM_LOAD16_BYTE( "in2-20p1.3",  0x020000, 0x10000, CRC(107aa448) SHA1(7b3d4053aaae3b97136cddefbc9edd5e61713ff7) )
	ROM_LOAD16_BYTE( "in2-20p1.4",  0x020001, 0x10000, CRC(04933278) SHA1(97462aef782f7fe82b60f4bddcad0e6a6b50f3df) )
ROM_END

ROM_START( v4psid )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "in20d.p1", 0x0000, 0x004000, CRC(e86e62a0) SHA1(97b0d41fa688cdd86bd6a1ef65cf143a34e23fac) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "in2-20p1.1",  0x000000, 0x10000, CRC(f34d9001) SHA1(2bae06f4a5a5510b15b918261ecb0de9e34a6b53) )
	ROM_LOAD16_BYTE( "in2-20p1.2",  0x000001, 0x10000, CRC(1dc931b4) SHA1(c46626183edd52c7938c5edee2395aacb49e0730) )
	ROM_LOAD16_BYTE( "in2-20p1.3",  0x020000, 0x10000, CRC(107aa448) SHA1(7b3d4053aaae3b97136cddefbc9edd5e61713ff7) )
	ROM_LOAD16_BYTE( "in2-20p1.4",  0x020001, 0x10000, CRC(04933278) SHA1(97462aef782f7fe82b60f4bddcad0e6a6b50f3df) )
ROM_END

ROM_START( v4psi14 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "psi20int", 0x0000, 0x004000, CRC(20ee94b4) SHA1(4c77be34e20a843add2bba23c092fd5bba90bc45) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "psi20p1", 0x000000, 0x010000, CRC(1795fe9c) SHA1(c93e0815d76ab4e55e7659d5c332db9d847a00b0) )
	ROM_LOAD16_BYTE( "psi20p2", 0x000001, 0x010000, CRC(c1497581) SHA1(f6b64df97d90aa13ed7b5d42608b9014326a880b) )
	ROM_LOAD16_BYTE( "psi20p3", 0x020000, 0x10000, NO_DUMP )
	ROM_LOAD16_BYTE( "psi20p4", 0x020001, 0x010000, CRC(45ff4d46) SHA1(f3d402fad950adb366e4deb67b4038c0febae004) )
ROM_END

ROM_START( v4psi14a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "in214.p1", 0x0000, 0x004000, CRC(4fb02448) SHA1(c2f2413a460012e3aadf7effbf8a33b40bc02df1) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "psi20p1", 0x000000, 0x010000, CRC(1795fe9c) SHA1(c93e0815d76ab4e55e7659d5c332db9d847a00b0) )
	ROM_LOAD16_BYTE( "psi20p2", 0x000001, 0x010000, CRC(c1497581) SHA1(f6b64df97d90aa13ed7b5d42608b9014326a880b) )
	ROM_LOAD16_BYTE( "psi20p3", 0x020000, 0x10000, NO_DUMP )
	ROM_LOAD16_BYTE( "psi20p4", 0x020001, 0x010000, CRC(45ff4d46) SHA1(f3d402fad950adb366e4deb67b4038c0febae004) )
ROM_END

ROM_START( v4psibc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("psi12m.p1",  0x00000, 0x04000,  CRC(560b2085) SHA1(5dccede70e228d896ff11ff861c9f32b895e807d) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "psi12.p1",  0x000000, 0x10000, CRC(9a3972bd) SHA1(72c747c16b5a31a7efcf561c2f9ce151a559b1ac) )
	ROM_LOAD16_BYTE( "psi12.p2",  0x000001, 0x10000, CRC(1a5da4f4) SHA1(f926cb650e2014a771621a497d4cc228c18f2979) )
	ROM_LOAD16_BYTE( "psi12.p3",  0x020000, 0x10000, CRC(cab2e50b) SHA1(f5ba3ccef87bb7afc59e6aa38c364a492d11b0a2) )
	ROM_LOAD16_BYTE( "psi12.p4",  0x020001, 0x10000, CRC(83781f1a) SHA1(a21d2e0ce6add058b5c6efad778a14128842b71b) )
ROM_END

ROM_START( v4psibcd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "in14d.p1", 0x0000, 0x004000, CRC(cb9a093a) SHA1(225ca4f191f64f6ca3ed6bc7b58819a893fdd36a) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "psi12.p1",  0x000000, 0x10000, CRC(9a3972bd) SHA1(72c747c16b5a31a7efcf561c2f9ce151a559b1ac) )
	ROM_LOAD16_BYTE( "psi12.p2",  0x000001, 0x10000, CRC(1a5da4f4) SHA1(f926cb650e2014a771621a497d4cc228c18f2979) )
	ROM_LOAD16_BYTE( "psi12.p3",  0x020000, 0x10000, CRC(cab2e50b) SHA1(f5ba3ccef87bb7afc59e6aa38c364a492d11b0a2) )
	ROM_LOAD16_BYTE( "psi12.p4",  0x020001, 0x10000, CRC(83781f1a) SHA1(a21d2e0ce6add058b5c6efad778a14128842b71b) )
ROM_END


ROM_START( v4blox )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("blxv___2.0_0",  0x00000, 0x04000, CRC(b399b85e) SHA1(d36391fee4e3126754d6a0fa5f52fe05bc676930) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "blxv___2.0_1",  0x000000, 0x10000, CRC(68134eda) SHA1(b324ae1243164c7be7f5eced7ff93760e2176a4e) )
	ROM_LOAD16_BYTE( "blxv___2.0_2",  0x000001, 0x10000, CRC(6b1f8588) SHA1(e8a443f062555f1ed228e1bfed2031927bcd7015) )
	ROM_LOAD16_BYTE( "blxv___2.0_3",  0x020000, 0x10000, CRC(c62d704b) SHA1(bea0c9519063f1601f70372ccb49fb892fbd6e76) )
	ROM_LOAD16_BYTE( "blxv___2.0_4",  0x020001, 0x10000, CRC(e431471a) SHA1(cf90dc48be3bc5e3c5a8efea5818dbc15fa442e9) )
	ROM_LOAD16_BYTE( "blxv___2.0_5",  0x040000, 0x10000, CRC(98ac6bc7) SHA1(9575014ba21fa4330138a34f53e13d30d312bc8b) )
	ROM_LOAD16_BYTE( "blxv___2.0_6",  0x040001, 0x10000, CRC(a3d92b5b) SHA1(1e7042d5eae4a19a01a3ef7d806c434886dc9f4d) )
ROM_END

ROM_START( v4bloxd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("blxv_d_2.0_0",  0x00000, 0x04000, CRC(2e0891e1) SHA1(3e8ffd0d41227a8a1e311ca0c0bde7590e06dfbd) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "blxv___2.0_1",  0x000000, 0x10000, CRC(68134eda) SHA1(b324ae1243164c7be7f5eced7ff93760e2176a4e) )
	ROM_LOAD16_BYTE( "blxv___2.0_2",  0x000001, 0x10000, CRC(6b1f8588) SHA1(e8a443f062555f1ed228e1bfed2031927bcd7015) )
	ROM_LOAD16_BYTE( "blxv___2.0_3",  0x020000, 0x10000, CRC(c62d704b) SHA1(bea0c9519063f1601f70372ccb49fb892fbd6e76) )
	ROM_LOAD16_BYTE( "blxv___2.0_4",  0x020001, 0x10000, CRC(e431471a) SHA1(cf90dc48be3bc5e3c5a8efea5818dbc15fa442e9) )
	ROM_LOAD16_BYTE( "blxv___2.0_5",  0x040000, 0x10000, CRC(98ac6bc7) SHA1(9575014ba21fa4330138a34f53e13d30d312bc8b) )
	ROM_LOAD16_BYTE( "blxv___2.0_6",  0x040001, 0x10000, CRC(a3d92b5b) SHA1(1e7042d5eae4a19a01a3ef7d806c434886dc9f4d) )
ROM_END

ROM_START( v4tetrs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("tetris22.p0",  0x00000, 0x04000, CRC(b711c7ae) SHA1(767b17ddf9021fdf79ff6c52f04a5d8ea60cf30e) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tetris22.p1",  0x000000, 0x10000, CRC(e81dd182) SHA1(28b460224abf6fe24b637542ccd1c84040674555) )
	ROM_LOAD16_BYTE( "tetris22.p2",  0x000001, 0x10000, CRC(68aa4f15) SHA1(4e4511a64391fc64e5f5b7ccb46a78fd2e1d94d6) )
	ROM_LOAD16_BYTE( "tetris22.p3",  0x020000, 0x10000, CRC(b38b4763) SHA1(d28e77fdd6869cb5b5ec40ed1f300a2a947e0482) )
	ROM_LOAD16_BYTE( "tetris22.p4",  0x020001, 0x10000, CRC(1649f604) SHA1(ca4ac303391a0969d41c8f988b8e81cfcee1a21c) )
	ROM_LOAD16_BYTE( "tetris22.p5",  0x040000, 0x10000, CRC(02859676) SHA1(5293c767021a6b5253eecab0b0568aa082ea7084) )
	ROM_LOAD16_BYTE( "tetris22.p6",  0x040001, 0x10000, CRC(40d24c82) SHA1(7ac3cf148af84ad93eaf11ce3420abbe45d986e2) )
ROM_END

ROM_START( v4tetrs1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tetv1int", 0x0000, 0x004000, CRC(98de975d) SHA1(5b4fc06aa8008d3967c68f364c47f8377a1ba9df) ) // alt / earlier base ROM?

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tetris22.p1",  0x000000, 0x10000, CRC(e81dd182) SHA1(28b460224abf6fe24b637542ccd1c84040674555) )
	ROM_LOAD16_BYTE( "tetris22.p2",  0x000001, 0x10000, CRC(68aa4f15) SHA1(4e4511a64391fc64e5f5b7ccb46a78fd2e1d94d6) )
	ROM_LOAD16_BYTE( "tetris22.p3",  0x020000, 0x10000, CRC(b38b4763) SHA1(d28e77fdd6869cb5b5ec40ed1f300a2a947e0482) )
	ROM_LOAD16_BYTE( "tetris22.p4",  0x020001, 0x10000, CRC(1649f604) SHA1(ca4ac303391a0969d41c8f988b8e81cfcee1a21c) )
	ROM_LOAD16_BYTE( "tetris22.p5",  0x040000, 0x10000, CRC(02859676) SHA1(5293c767021a6b5253eecab0b0568aa082ea7084) )
	ROM_LOAD16_BYTE( "tetris22.p6",  0x040001, 0x10000, CRC(40d24c82) SHA1(7ac3cf148af84ad93eaf11ce3420abbe45d986e2) )
ROM_END

/* Vegas Poker Prototype dumped by HIGHWAYMAN */
ROM_START( v4vgpok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("comms-v2.0.bin",  0x00000, 0x10000,  CRC(1717581f) SHA1(40f8cae39a2ab0c89d2bbfd8a37725aaae229c96))

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("video-1.bin",  0x000000, 0x010000,  CRC(7ba2396c) SHA1(eb24b802b984315fc2eba4f15c208e2c1925c1c8))
	ROM_LOAD16_BYTE("video-2.bin",  0x000001, 0x010000,  CRC(4f9e830b) SHA1(f17bebb289c3620bf4c88b2b358a9dab87ac214f))
	ROM_LOAD16_BYTE("video-3.bin",  0x020000, 0x010000,  CRC(607e0baa) SHA1(9f64a46ef3b9a854e939b5e7f0d1e6e925735922))
	ROM_LOAD16_BYTE("video-4.bin",  0x020001, 0x010000,  CRC(2019f5d3) SHA1(d183b3b92d03be9f9d57b5df1a621cbfe955ed93))
	ROM_LOAD16_BYTE("video-5.bin",  0x040000, 0x010000,  CRC(c029202e) SHA1(b08bb2678c2ff62a58ef67d5440c326d0fadc34e))
	ROM_LOAD16_BYTE("video-6.bin",  0x040001, 0x010000,  CRC(3287ae4e) SHA1(3b05a036de3ca7ec644bfbf04934e44e631d1e28))
	ROM_LOAD16_BYTE("video-7.bin",  0x060000, 0x010000,  CRC(231cf163) SHA1(02b28ef0e1661a82d0fba2ecc5474c79651fa9e7))
	ROM_LOAD16_BYTE("video-8.bin",  0x060001, 0x010000,  CRC(076efdc8) SHA1(bef0a1d8f0e7486ee5dc7407ce5c96854cefa5cf))

	/*characteriser chip*/
	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "ic18-palce16v8h.bin", 0x0000, 0x0117, CRC(f144738a) SHA1(8e563aaff07ad23864e3019737cbdf4490b70d8f) )
ROM_END


ROM_START( v4reno )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_27s__.a60",  0x00000, 0x10000,  CRC(44c9ff47) SHA1(93a3155144b233c113aa3b49bd4eb5969e400a68))

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renoa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rri20s__.a_0",    0x00000, 0x10000,   CRC(0fb9686a) SHA1(a403d4424897fcdc343b277aa0caa032ed970747) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rrixes__.a_0",    0x00000, 0x10000,   CRC(3f055fa1) SHA1(ee6561d6849e5150d7b7b5585e8ed8176e706aeb) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renoc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_20ab_.a_0",    0x00000, 0x10000,   CRC(6da308aa) SHA1(c1f418592942a9f68aac9a5a6f91911b96861d48) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renod )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_20a_p.a_0",    0x00000, 0x10000,   CRC(0dc6b163) SHA1(5a666dec859807cab6478b06f38473997fe49cd6) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renoe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_20a__.a_0",    0x00000, 0x10000,   CRC(9b279f39) SHA1(9e9e80fdc8517a314bac15a5087d7619a84c1e00) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END


ROM_START( v4renof )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_20bgp.a_0",    0x00000, 0x10000,   CRC(7175112b) SHA1(799c822a6dabcf2a7d67b2ef81273a0fba6cf3d9) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renog )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_20bg_.a_0",    0x00000, 0x10000,   CRC(e7943f71) SHA1(490af3fc7d3506ca9c5c049a6fcffb856bf28d1e) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renoh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_20btp.a_0",    0x00000, 0x10000,   CRC(c73e1c28) SHA1(37c5b984311439906cae2ba48aab249caeb1f2ab) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renoi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_20bt_.a_0",    0x00000, 0x10000,   CRC(51df3272) SHA1(c9cc06556e79e09b9b3cd9816b6f7dde92dadfe7) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renoj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_20sbp.a_0",    0x00000, 0x10000,   CRC(0ea4be35) SHA1(2e3950bcc01f4c1ce53873b552cb156a91c74e85) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_20sb_.a_0",    0x00000, 0x10000,   CRC(9845906f) SHA1(693e480d548482c073644513803ddd4e5ed0694c) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renol )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_20s__.a_0",    0x00000, 0x10000,   CRC(6ec107fc) SHA1(46ac2bbb19ff4d562fa2e0029e9831be0bec5def) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renom )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_27sd_.a60",    0x00000, 0x10000,   CRC(0f6c18e6) SHA1(23f07d1ed2340e73abcf6b86581bc5dd768dbab5) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_37sd_.a60",    0x00000, 0x10000,   CRC(807e73c8) SHA1(202d621cead9b2af8fef12ea0d07a6fce6262518) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renoo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_37s__.a60",    0x00000, 0x10000,   CRC(cbdb9469) SHA1(bc802b4c15451feebc332944f6bc09c7fb20ea20) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END


ROM_START( v4renop )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_x7sd_.a60",    0x00000, 0x10000,   CRC(3fd02f2d) SHA1(49ae60e8bdc6681482272d31eefc0098cc6c9667) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renoq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_x7s__.a60",    0x00000, 0x10000,   CRC(7475c88c) SHA1(0425e722321d4f365f6e90de5159721ac8a9d0d2) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renor )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_xeadp.a_0",    0x00000, 0x10000,   CRC(76df6109) SHA1(fbc76a9612a48f1b589e43e2f920459ed6c32c57) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_xead_.a_0",    0x00000, 0x10000,   CRC(e03e4f53) SHA1(17b4bdf82393aacf74765f04fc0d9b1f683114cc) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_xea_p.a_0",    0x00000, 0x10000,   CRC(3d7a86a8) SHA1(98bb8b2c0705219536720eef404c7bbc14a85793) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4renou )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rr_xea__.a_0",    0x00000, 0x10000,   CRC(ab9ba8f2) SHA1(52b77aa66980fa552d286225919fca9910f48326) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rr______.a_1",  0x000000, 0x80000,  CRC(ff27d0ba) SHA1(85cce36495f00a05c1806ecde37274212680e466) )
	ROM_LOAD16_BYTE( "rr______.a_2",  0x000001, 0x80000,  CRC(519b9ae1) SHA1(8ccfe8de0f2c85923df81af8cba6f20af43d2fe2) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END



ROM_START( v4reno8 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rrv8ss",          0x00000, 0x10000,   CRC(a37383a5) SHA1(6c2563967546d810f2c50aa9a269bb1369014c18) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 8, good
	ROM_LOAD16_BYTE( "rr______.8_1",      0x000001, 0x080000,  CRC(eca43ed4) SHA1(e2e4e5d3d4b659ddd74c120316b9658708e188f1) )
	ROM_LOAD16_BYTE( "rr______.8_2",      0x000000, 0x080000,  CRC(c3f25586) SHA1(7335708a7d90c7fbd0088bb6ee5ce0255b9b18ab) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4reno7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("release7.mpu4",          0x00000, 0x10000,  NO_DUMP )

	ROM_REGION( 0x800000, "video", 0 ) // Release 7
	ROM_LOAD16_BYTE( "rr8p1",             0x000001, 0x080000,  CRC(68992dd3) SHA1(75ab1cd02ac627b6191e9b61ee7c072029becaeb) )
	ROM_LOAD16_BYTE( "rr8p2",             0x000000, 0x080000,  CRC(b859020e) SHA1(811ccac82d022ceccc83f1bf6c6b4de6cc313e14) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END

ROM_START( v4reno5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("release5.mpu4",          0x00000, 0x10000,  NO_DUMP )

	ROM_REGION( 0x800000, "video", 0 ) // Release 5
	ROM_LOAD16_BYTE( "reno reels 5-1",    0x000000, 0x080000,  CRC(9ebd0eaf) SHA1(3d326509240fe8a83df9d2369f184838bee2b407) )
	ROM_LOAD16_BYTE( "reno reels 5-2",    0x000001, 0x080000,  CRC(1cbcd9b5) SHA1(989d64e10c67dab7d20229e5c63d24111d556138) )

	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "renosnda.bin",  0x000000, 0x080000,  CRC(a72a5e1b) SHA1(a0d5338a400345a55484848a7612119405f617b1) )
	ROM_LOAD( "renosndb.bin",  0x080000, 0x080000,  CRC(46e9a32f) SHA1(d45835a82368992597e44b3c5b9d00d8b901e733) )
ROM_END


ROM_START( v4redhtp ) // ok
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rp_20s__.3_0",  0x00000, 0x10000,  CRC(b7d02d22) SHA1(f9da1c6dde064bc39d0c48a165dac7acde933397))

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rhpmpu4.p1", 0x0000, 0x010000, CRC(e614757f) SHA1(96e825bedfb1715aa9b5d131e5b492247b17b725) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_10a__.3_0", 0x0000, 0x010000, CRC(90f58c6f) SHA1(c9b412172a6ef361407f486a8fb134ac68fe31e5) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_10ab_.3_0", 0x0000, 0x010000, CRC(66711bfc) SHA1(ee51215479e57646e9490b6e898ca5092172879a) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_10ad_.3_0", 0x0000, 0x010000, CRC(db506bce) SHA1(1f152e58a3a5251aa00f890df9a9930a72d84db6) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_10ak_.3_0", 0x0000, 0x010000, CRC(2dd4fc5d) SHA1(b1e908ef70c98e7467a5f7d3ec26e7e6bfa1c475) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_10bg_.3_0", 0x0000, 0x010000, CRC(ec462c27) SHA1(88379585055f245c8d84bd1295e7f5b6555ad8c3) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_10s__.3_0", 0x0000, 0x010000, CRC(651314aa) SHA1(f11cd9c9419511e179acd2883d83cb475eb4d761) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtph )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_10sb_.3_0", 0x0000, 0x010000, CRC(93978339) SHA1(7c943fe44103d0ac276b4478e20709d6ab0b07d5) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_10sd_.3_0", 0x0000, 0x010000, CRC(2eb6f30b) SHA1(5aa3161888ad50bb9c1945bff32e6d8552ab4c89) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_10sk_.3_0", 0x0000, 0x010000, CRC(d8326498) SHA1(89ab78991ea72f5186488714e8b7daeafa1e9497) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_20a__.3_0", 0x0000, 0x010000, CRC(4236b5e7) SHA1(1bd73c8fd20a176c25288becb6f07b7f0447ede3) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_20ab_.3_0", 0x0000, 0x010000, CRC(b4b22274) SHA1(f5f4d4f88d74454e6857118dc3eba90defd15e50) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_20ad_.3_0", 0x0000, 0x010000, CRC(09935246) SHA1(e82749b575f22373a3a7f24a8f68980e8040ac9d) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_20ak_.3_0", 0x0000, 0x010000, CRC(ff17c5d5) SHA1(1aa1ca348cd8828e616a4781704445d04800b970) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_20bg_.3_0", 0x0000, 0x010000, CRC(3e8515af) SHA1(3df382818b1d527ab4037a0dca24ba28227ab6a4) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_20sb_.3_0", 0x0000, 0x010000, CRC(4154bab1) SHA1(b3d5dcdd276e3b2fc1f5d66a870e85c5064f0f5b) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_20sd_.3_0", 0x0000, 0x010000, CRC(fc75ca83) SHA1(9cf7178c990151c7711c8bca68c0dd85325808ab) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_20sk_.3_0", 0x0000, 0x010000, CRC(0af15d10) SHA1(df1367176f8f2efcc21cf5c841125f9553ad0c8a) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtps )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_xca__.3_0", 0x0000, 0x010000, CRC(728a822c) SHA1(b23e4893325c2dbeb8f72114dd1e5c3f06a786a8) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_xcab_.3_0", 0x0000, 0x010000, CRC(840e15bf) SHA1(9eba10eeebdf1c801186ae2c0206292da28ab7c5) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_xcad_.3_0", 0x0000, 0x010000, CRC(392f658d) SHA1(c730a5e54faab1946554af9e6b9e08dc50bbe9d6) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_xcak_.3_0", 0x0000, 0x010000, CRC(cfabf21e) SHA1(9cec91ea209d250ba885552579044b5ce9d730bd) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_xcbg_.3_0", 0x0000, 0x010000, CRC(0e392264) SHA1(aeb5a4df61b3e67100912ede2f5e127d413b656c) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_xcs__.3_0", 0x0000, 0x010000, CRC(876c1ae9) SHA1(9ea1b2a28f2b87089500dd25e6dc1450be43acbc) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_xcsb_.3_0", 0x0000, 0x010000, CRC(71e88d7a) SHA1(81d38e598514a5a1b9bca0131232b18c3829edfb) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_xcsd_.3_0", 0x0000, 0x010000, CRC(ccc9fd48) SHA1(1069b945b5312eb01139b7671901212f8f65e3bf) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_xcsk_.3_0", 0x0000, 0x010000, CRC(3a4d6adb) SHA1(01dfead74d40af9a5a4a95781b83d078cfb92ac9) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtpab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rpi10___.3_0", 0x0000, 0x010000, CRC(046b7b3c) SHA1(71f74153dbdd52036a55fc0a217120dee84ca230) )

	ROM_REGION( 0x800000, "video", 0 ) // Release 3 Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE("rp______.3_5",  0x040000, 0x010000,  CRC(d9fd05d0) SHA1(330ef58c012b5d5fd018bea54b3ae315b3e45cfd))
	ROM_LOAD16_BYTE("rp______.3_6",  0x040001, 0x010000,  CRC(eeea91ff) SHA1(cc7870a68f62d4dd70c13713a432a61a091821ef))

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END


ROM_START( v4redhtpunk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rhp20ac6", 0x0000, 0x010000, CRC(d6a842b4) SHA1(94f6cc6a9e0efa8a2eeee14f981f9d2407dfb092) )

	ROM_REGION( 0x800000, "video", 0 ) // none of the ROMs are have are commpatible with this?
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END


ROM_START( v4redhtparc ) // ok
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "redhotpoker20_mpu4_interface.bin", 0x0000, 0x010000, CRC(cafbcc82) SHA1(662df1f6409f9e20fac07c07d08eae3ea8ba362a) )

	ROM_REGION( 0x800000, "video", 0 ) // Version 1.9 Arcade Video ROMs
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE( "redhotpokervideoboardp5.bin", 0x040000, 0x010000, CRC(d36189b7) SHA1(7757ce9879754d4b8a450ba1f6067c17c151c13c) )
	ROM_LOAD16_BYTE( "redhotpokervideoboardp6.bin", 0x040001, 0x010000, CRC(c89d164d) SHA1(0cf33db0f85958251624dd7bc2c3024814489040) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END


ROM_START( v4redhtp2 ) // ok
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_05s__.2_0", 0x0000, 0x010000, CRC(62e93168) SHA1(8287eee2d6ac4cc447ce6652de24dfe056015ef3) )

	ROM_REGION( 0x800000, "video", 0 ) // newer 'release 2' 68k ROMs with 1993 date
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE( "rp_05___.2_5", 0x040000, 0x010000, CRC(cc79187b) SHA1(b2e556fd7a1667203dcb196b1dc2d89bff785675) )
	ROM_LOAD16_BYTE( "rp_05___.2_6", 0x040001, 0x010000, CRC(57d1cf7b) SHA1(c8d6f4d0e8a5a383c47300e8d56e13d62295f60f) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtp2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_05a__.2_0", 0x0000, 0x010000, CRC(970fa9ad) SHA1(77550948aae171eae715e077cdbeffaeeadb2436) )

	ROM_REGION( 0x800000, "video", 0 ) // newer 'release 2' 68k ROMs with 1993 date
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE( "rp_05___.2_5", 0x040000, 0x010000, CRC(cc79187b) SHA1(b2e556fd7a1667203dcb196b1dc2d89bff785675) )
	ROM_LOAD16_BYTE( "rp_05___.2_6", 0x040001, 0x010000, CRC(57d1cf7b) SHA1(c8d6f4d0e8a5a383c47300e8d56e13d62295f60f) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtp2b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_05ab_.2_0", 0x0000, 0x010000, CRC(618b3e3e) SHA1(19f7c83957f1d0b36f62607d314a310748b3c84a) )

	ROM_REGION( 0x800000, "video", 0 ) // newer 'release 2' 68k ROMs with 1993 date
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE( "rp_05___.2_5", 0x040000, 0x010000, CRC(cc79187b) SHA1(b2e556fd7a1667203dcb196b1dc2d89bff785675) )
	ROM_LOAD16_BYTE( "rp_05___.2_6", 0x040001, 0x010000, CRC(57d1cf7b) SHA1(c8d6f4d0e8a5a383c47300e8d56e13d62295f60f) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtp2c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_05ad_.2_0", 0x0000, 0x010000, CRC(dcaa4e0c) SHA1(2a037722f5e951513c233227448c3a2e55de8ef9) )

	ROM_REGION( 0x800000, "video", 0 ) // newer 'release 2' 68k ROMs with 1993 date
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE( "rp_05___.2_5", 0x040000, 0x010000, CRC(cc79187b) SHA1(b2e556fd7a1667203dcb196b1dc2d89bff785675) )
	ROM_LOAD16_BYTE( "rp_05___.2_6", 0x040001, 0x010000, CRC(57d1cf7b) SHA1(c8d6f4d0e8a5a383c47300e8d56e13d62295f60f) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtp2d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_05ak_.2_0", 0x0000, 0x010000, CRC(2a2ed99f) SHA1(45215a140b4e9d6b190fff0b89fcbeffc054d732) )

	ROM_REGION( 0x800000, "video", 0 ) // newer 'release 2' 68k ROMs with 1993 date
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE( "rp_05___.2_5", 0x040000, 0x010000, CRC(cc79187b) SHA1(b2e556fd7a1667203dcb196b1dc2d89bff785675) )
	ROM_LOAD16_BYTE( "rp_05___.2_6", 0x040001, 0x010000, CRC(57d1cf7b) SHA1(c8d6f4d0e8a5a383c47300e8d56e13d62295f60f) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtp2e )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_05sb_.2_0", 0x0000, 0x010000, CRC(946da6fb) SHA1(d406f0bea0940b6910dc923ded0c89db9c1f3c61) )

	ROM_REGION( 0x800000, "video", 0 ) // newer 'release 2' 68k ROMs with 1993 date
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE( "rp_05___.2_5", 0x040000, 0x010000, CRC(cc79187b) SHA1(b2e556fd7a1667203dcb196b1dc2d89bff785675) )
	ROM_LOAD16_BYTE( "rp_05___.2_6", 0x040001, 0x010000, CRC(57d1cf7b) SHA1(c8d6f4d0e8a5a383c47300e8d56e13d62295f60f) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtp2f )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_05sd_.2_0", 0x0000, 0x010000, CRC(294cd6c9) SHA1(4f2890c58cfa6d91ddc73d9098d8e45e79f410c3) )

	ROM_REGION( 0x800000, "video", 0 ) // newer 'release 2' 68k ROMs with 1993 date
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE( "rp_05___.2_5", 0x040000, 0x010000, CRC(cc79187b) SHA1(b2e556fd7a1667203dcb196b1dc2d89bff785675) )
	ROM_LOAD16_BYTE( "rp_05___.2_6", 0x040001, 0x010000, CRC(57d1cf7b) SHA1(c8d6f4d0e8a5a383c47300e8d56e13d62295f60f) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4redhtp2g )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp_05sk_.2_0", 0x0000, 0x010000, CRC(dfc8415a) SHA1(5d9f946dc0307d16c9d90856d4ae2b4c8c5013a6) )

	ROM_REGION( 0x800000, "video", 0 ) // newer 'release 2' 68k ROMs with 1993 date
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE( "rp_05___.2_5", 0x040000, 0x010000, CRC(cc79187b) SHA1(b2e556fd7a1667203dcb196b1dc2d89bff785675) )
	ROM_LOAD16_BYTE( "rp_05___.2_6", 0x040001, 0x010000, CRC(57d1cf7b) SHA1(c8d6f4d0e8a5a383c47300e8d56e13d62295f60f) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END





ROM_START( v4redhtp2z ) // ok
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rhpint", 0x0000, 0x010000, CRC(d60e7e30) SHA1(174f69ff2e76837455c107055b06f98875033b5e) )

	ROM_REGION( 0x800000, "video", 0 ) // older 'release 2' 68K ROMs with 1992 date
	ROM_LOAD16_BYTE("rp______.3_1",  0x000000, 0x010000,  CRC(b987406d) SHA1(8c4d386570c0c6298b1cabf50295021b3b0cf625))
	ROM_LOAD16_BYTE("rp______.3_2",  0x000001, 0x010000,  CRC(73e3c12e) SHA1(19e3ed7255fa0c3bfa14b6a4b705c0c3e1a237b6))
	ROM_LOAD16_BYTE("rp______.3_3",  0x020000, 0x010000,  CRC(05a30183) SHA1(302f4926073bf7335da7f0b1e6399b64ea9bbae4))
	ROM_LOAD16_BYTE("rp______.3_4",  0x020001, 0x010000,  CRC(6b122765) SHA1(72cd0fda322790bed8cdc7697306ec01efc43789))
	ROM_LOAD16_BYTE( "rhp1.6p5", 0x040000, 0x010000, CRC(750436a1) SHA1(006a31fc5c22969bd79dbc54e618348ad7832ac7) )
	ROM_LOAD16_BYTE( "rhp1.6p6", 0x040001, 0x010000, CRC(d78839c2) SHA1(e82b769cba4b8d50dcf5c301c03d4ca66e893f70) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	// none present
ROM_END

ROM_START( v4cmaze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "crys.p1", 0x000000, 0x80000,  CRC(40fbde50) SHA1(91bd21c0aaffb9c9b89a114affbc485b12ae9bb4) )
	ROM_LOAD16_BYTE( "cry.p2",  0x000001, 0x80000,  CRC(fa7d006f) SHA1(ecc03b4d7a4089feccc53ad05313c35b33e061d7) )
	ROM_LOAD16_BYTE( "cry.p3",  0x100000, 0x80000,  CRC(e8cf8203) SHA1(e9f42e5c18b97807f51284ad2416346578ed73c4) )
	ROM_LOAD16_BYTE( "cry.p4",  0x100001, 0x80000,  CRC(7b036151) SHA1(7b0040c296059b1e1798ddedf0ecb4582d67ee70) )
	ROM_LOAD16_BYTE( "cry.p5",  0x200000, 0x80000,  CRC(48f17b20) SHA1(711c46fcfd86ded8ff7da883188d70560d20e42f) )
	ROM_LOAD16_BYTE( "cry.p6",  0x200001, 0x80000,  CRC(2b3d9a97) SHA1(7468fffd90d840d245a70475b42308f1e48c5017) )
	ROM_LOAD16_BYTE( "cry.p7",  0x300000, 0x80000,  CRC(20f73433) SHA1(593b40ac17591ac312ad41b4d3a5772626137bba) )
	ROM_LOAD16_BYTE( "cry.p8",  0x300001, 0x80000,  CRC(835da1f2) SHA1(f93e075916d370466832871410591570ad7b9f3b) )
	ROM_LOAD16_BYTE( "cry.p9",  0x400000, 0x80000,  CRC(c0e442ee) SHA1(a3877b200538642fe2bc96cfe8b33f04d8a82a98) )
	ROM_LOAD16_BYTE( "cry.p10", 0x400001, 0x80000,  CRC(500172fa) SHA1(d83a37612daa79ba8425fdb28f39b8324b5736b6) )
ROM_END

ROM_START( v4cmazedat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cryd.p1", 0x000000, 0x80000,  CRC(b245f661) SHA1(85a8baca8797fea74bc36eea077be56bde8e54a9) )
	ROM_LOAD16_BYTE( "cry.p2",  0x000001, 0x80000,  CRC(fa7d006f) SHA1(ecc03b4d7a4089feccc53ad05313c35b33e061d7) )
	ROM_LOAD16_BYTE( "cry.p3",  0x100000, 0x80000,  CRC(e8cf8203) SHA1(e9f42e5c18b97807f51284ad2416346578ed73c4) )
	ROM_LOAD16_BYTE( "cry.p4",  0x100001, 0x80000,  CRC(7b036151) SHA1(7b0040c296059b1e1798ddedf0ecb4582d67ee70) )
	ROM_LOAD16_BYTE( "cry.p5",  0x200000, 0x80000,  CRC(48f17b20) SHA1(711c46fcfd86ded8ff7da883188d70560d20e42f) )
	ROM_LOAD16_BYTE( "cry.p6",  0x200001, 0x80000,  CRC(2b3d9a97) SHA1(7468fffd90d840d245a70475b42308f1e48c5017) )
	ROM_LOAD16_BYTE( "cry.p7",  0x300000, 0x80000,  CRC(20f73433) SHA1(593b40ac17591ac312ad41b4d3a5772626137bba) )
	ROM_LOAD16_BYTE( "cry.p8",  0x300001, 0x80000,  CRC(835da1f2) SHA1(f93e075916d370466832871410591570ad7b9f3b) )
	ROM_LOAD16_BYTE( "cry.p9",  0x400000, 0x80000,  CRC(c0e442ee) SHA1(a3877b200538642fe2bc96cfe8b33f04d8a82a98) )
	ROM_LOAD16_BYTE( "cry.p10", 0x400001, 0x80000,  CRC(500172fa) SHA1(d83a37612daa79ba8425fdb28f39b8324b5736b6) )
ROM_END

ROM_START( v4cmaze_amld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "am1z.p1",  0x000000, 0x80000,  CRC(d2fdda6d) SHA1(96c27dedc3cf1dd478e6169200844d418a276f14) )
	ROM_LOAD16_BYTE( "am1z.p2",  0x000001, 0x80000,  CRC(1637170f) SHA1(fd17a0e7794f01bf4ad7a16b185f87cb060c70ab) )
	ROM_LOAD16_BYTE( "am1g.p1",  0x100000, 0x80000,  CRC(e8cf8203) SHA1(e9f42e5c18b97807f51284ad2416346578ed73c4) )
	ROM_LOAD16_BYTE( "am1g.p2",  0x100001, 0x80000,  CRC(7b036151) SHA1(7b0040c296059b1e1798ddedf0ecb4582d67ee70) )
	ROM_LOAD16_BYTE( "am1g.p3",  0x200000, 0x80000,  CRC(48f17b20) SHA1(711c46fcfd86ded8ff7da883188d70560d20e42f) )
	ROM_LOAD16_BYTE( "am1g.p4",  0x200001, 0x80000,  CRC(2b3d9a97) SHA1(7468fffd90d840d245a70475b42308f1e48c5017) )
	ROM_LOAD16_BYTE( "am1g.p5",  0x300000, 0x80000,  CRC(68286bb1) SHA1(c307e3ad1e0fd92314216c8e554aafa949559452) )
	ROM_LOAD16_BYTE( "am1g.p6",  0x300001, 0x80000,  CRC(a6b498ad) SHA1(117e1a4ec7e2d3c7d530c5a56cbc1d24b0ddc747) )
	ROM_LOAD16_BYTE( "am1g.p7",  0x400000, 0x80000,  CRC(15882699) SHA1(b29a331e51a37554323b91330a7c2b62b33a943a) )
	ROM_LOAD16_BYTE( "am1g.p8",  0x400001, 0x80000,  CRC(6f0f855b) SHA1(ab411d1af0f88049a6c435bafd4b1fa63f5519b1) )
ROM_END

ROM_START( v4cmazeb ) /* Was in a romset called 'Mad Money!' */
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm1.2p1", 0x000000, 0x040000, CRC(481f2fe2) SHA1(7bff9476dbd8c0812a51cd648acf11ca42789e54) )
	ROM_LOAD16_BYTE( "cm1.2p2", 0x000001, 0x040000, CRC(6e6dc121) SHA1(05b29ea935d54aa97279e0a80265412747a92a10) )
	ROM_LOAD16_BYTE( "cry.p3",  0x100000, 0x80000,  CRC(e8cf8203) SHA1(e9f42e5c18b97807f51284ad2416346578ed73c4) )
	ROM_LOAD16_BYTE( "cry.p4",  0x100001, 0x80000,  CRC(7b036151) SHA1(7b0040c296059b1e1798ddedf0ecb4582d67ee70) )
	ROM_LOAD16_BYTE( "cry.p5",  0x200000, 0x80000,  CRC(48f17b20) SHA1(711c46fcfd86ded8ff7da883188d70560d20e42f) )
	ROM_LOAD16_BYTE( "cry.p6",  0x200001, 0x80000,  CRC(2b3d9a97) SHA1(7468fffd90d840d245a70475b42308f1e48c5017) )
	ROM_LOAD16_BYTE( "cry.p7",  0x300000, 0x80000,  CRC(20f73433) SHA1(593b40ac17591ac312ad41b4d3a5772626137bba) )
	ROM_LOAD16_BYTE( "cry.p8",  0x300001, 0x80000,  CRC(835da1f2) SHA1(f93e075916d370466832871410591570ad7b9f3b) )
	ROM_LOAD16_BYTE( "cry.p9",  0x400000, 0x80000,  CRC(c0e442ee) SHA1(a3877b200538642fe2bc96cfe8b33f04d8a82a98) )
	ROM_LOAD16_BYTE( "cry.p10", 0x400001, 0x80000,  CRC(500172fa) SHA1(d83a37612daa79ba8425fdb28f39b8324b5736b6) )
ROM_END

ROM_START( v4cmazec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm1.p1",  0x000000, 0x040000, CRC(40533c1d) SHA1(06f115b7e1008e8797767d3f1e37edb4c1ce6675) )
	ROM_LOAD16_BYTE( "cm1.p2",  0x000001, 0x040000, CRC(8b207eca) SHA1(cbd6b1b177daefe255609f7adc00effa6afc9cb5) )
	ROM_LOAD16_BYTE( "cry.p3",  0x100000, 0x80000,  CRC(e8cf8203) SHA1(e9f42e5c18b97807f51284ad2416346578ed73c4) )
	ROM_LOAD16_BYTE( "cry.p4",  0x100001, 0x80000,  CRC(7b036151) SHA1(7b0040c296059b1e1798ddedf0ecb4582d67ee70) )
	ROM_LOAD16_BYTE( "cry.p5",  0x200000, 0x80000,  CRC(48f17b20) SHA1(711c46fcfd86ded8ff7da883188d70560d20e42f) )
	ROM_LOAD16_BYTE( "cry.p6",  0x200001, 0x80000,  CRC(2b3d9a97) SHA1(7468fffd90d840d245a70475b42308f1e48c5017) )
	ROM_LOAD16_BYTE( "cry.p7",  0x300000, 0x80000,  CRC(20f73433) SHA1(593b40ac17591ac312ad41b4d3a5772626137bba) )
	ROM_LOAD16_BYTE( "cry.p8",  0x300001, 0x80000,  CRC(835da1f2) SHA1(f93e075916d370466832871410591570ad7b9f3b) )
	ROM_LOAD16_BYTE( "cry.p9",  0x400000, 0x80000,  CRC(c0e442ee) SHA1(a3877b200538642fe2bc96cfe8b33f04d8a82a98) )
	ROM_LOAD16_BYTE( "cry.p10", 0x400001, 0x80000,  CRC(500172fa) SHA1(d83a37612daa79ba8425fdb28f39b8324b5736b6) )
ROM_END

ROM_START( v4cmazed )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cmz11p1",  0x000000, 0x040000, CRC(887baf65) SHA1(096133414bff89db4e30405f72f3c994f962be08) )
	ROM_LOAD16_BYTE( "cmz11p2",  0x000001, 0x040000, CRC(f8f095a6) SHA1(6b23fe3712416bf268c6387d3b304f05206d45e1) )
	ROM_LOAD16_BYTE( "cry.p3",  0x100000, 0x80000,  CRC(e8cf8203) SHA1(e9f42e5c18b97807f51284ad2416346578ed73c4) )
	ROM_LOAD16_BYTE( "cry.p4",  0x100001, 0x80000,  CRC(7b036151) SHA1(7b0040c296059b1e1798ddedf0ecb4582d67ee70) )
	ROM_LOAD16_BYTE( "cry.p5",  0x200000, 0x80000,  CRC(48f17b20) SHA1(711c46fcfd86ded8ff7da883188d70560d20e42f) )
	ROM_LOAD16_BYTE( "cry.p6",  0x200001, 0x80000,  CRC(2b3d9a97) SHA1(7468fffd90d840d245a70475b42308f1e48c5017) )
	ROM_LOAD16_BYTE( "cry.p7",  0x300000, 0x80000,  CRC(20f73433) SHA1(593b40ac17591ac312ad41b4d3a5772626137bba) )
	ROM_LOAD16_BYTE( "cry.p8",  0x300001, 0x80000,  CRC(835da1f2) SHA1(f93e075916d370466832871410591570ad7b9f3b) )
	ROM_LOAD16_BYTE( "cry.p9",  0x400000, 0x80000,  CRC(c0e442ee) SHA1(a3877b200538642fe2bc96cfe8b33f04d8a82a98) )
	ROM_LOAD16_BYTE( "cry.p10", 0x400001, 0x80000,  CRC(500172fa) SHA1(d83a37612daa79ba8425fdb28f39b8324b5736b6) )
ROM_END



//The New Crystal Maze Featuring Ocean Zone
ROM_START( v4cmaze2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm2s.p1", 0x000000, 0x80000,  CRC(2bcdff56) SHA1(5ae4d4960db032ec9b8d66c53bc151830d009f61) )
	ROM_LOAD16_BYTE( "cm2s.p2", 0x000001, 0x80000,  CRC(92126def) SHA1(531593dee05954000d9836018aeff9460aecbd26) )
	ROM_LOAD16_BYTE( "cm2.p3",  0x100000, 0x80000,  CRC(88324715) SHA1(c6c8de4e5aeda14232ec7b026da389774b3c7bb1) )
	ROM_LOAD16_BYTE( "cm2.p4",  0x100001, 0x80000,  CRC(8d54a81d) SHA1(37753cf8595647aaf8b8267ca177b6744de9c6d4) )
	ROM_LOAD16_BYTE( "cm2.p5",  0x200000, 0x80000,  CRC(5cf8a2bf) SHA1(2514e78e82842fa5c85d26de35637269cd08b21d) )
	ROM_LOAD16_BYTE( "cm2.p6",  0x200001, 0x80000,  CRC(cf793d2d) SHA1(579c759f57fb6bb87aa27c9d5fb684058913dedc) )
	ROM_LOAD16_BYTE( "cm2.p7",  0x300000, 0x80000,  CRC(008aa4b0) SHA1(b4cec6d11abd0e111c295533700595398ff59075) )
	ROM_LOAD16_BYTE( "cm2.p8",  0x300001, 0x80000,  CRC(bac04f5a) SHA1(130721b7abf28dea1f8162705c8bfc5a4bb78152) )
ROM_END

ROM_START( v4cmaze2d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm2d.p1", 0x000000, 0x80000,  CRC(a150027b) SHA1(f737b0e3b8954c1589a929b7e567cb55df4a3997) )
	ROM_LOAD16_BYTE( "cm2d.p2", 0x000001, 0x80000,  CRC(84ed6bce) SHA1(7a11473e7ec277508952f7ae6cfc7ed28e1b5c99) )
	ROM_LOAD16_BYTE( "cm2.p3",  0x100000, 0x80000,  CRC(88324715) SHA1(c6c8de4e5aeda14232ec7b026da389774b3c7bb1) )
	ROM_LOAD16_BYTE( "cm2.p4",  0x100001, 0x80000,  CRC(8d54a81d) SHA1(37753cf8595647aaf8b8267ca177b6744de9c6d4) )
	ROM_LOAD16_BYTE( "cm2.p5",  0x200000, 0x80000,  CRC(5cf8a2bf) SHA1(2514e78e82842fa5c85d26de35637269cd08b21d) )
	ROM_LOAD16_BYTE( "cm2.p6",  0x200001, 0x80000,  CRC(cf793d2d) SHA1(579c759f57fb6bb87aa27c9d5fb684058913dedc) )
	ROM_LOAD16_BYTE( "cm2.p7",  0x300000, 0x80000,  CRC(008aa4b0) SHA1(b4cec6d11abd0e111c295533700595398ff59075) )
	ROM_LOAD16_BYTE( "cm2.p8",  0x300001, 0x80000,  CRC(bac04f5a) SHA1(130721b7abf28dea1f8162705c8bfc5a4bb78152) )
ROM_END

ROM_START( v4cmaze2_amld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "am2z.p1",  0x000000, 0x80000,  CRC(f27e02f0) SHA1(8637904c201ece4f08ad63b4fd6d06a860fa762f) )
	ROM_LOAD16_BYTE( "am2z.p2",  0x000001, 0x80000,  CRC(4d24f482) SHA1(9e3687db9d0233e56999017f3ed59ec543bce303) )
	ROM_LOAD16_BYTE( "am2g.p1",  0x100000, 0x80000,  CRC(115402db) SHA1(250f2eded1b88a1abf82febb009eadbb90936f8a) )
	ROM_LOAD16_BYTE( "am2g.p2",  0x100001, 0x80000,  CRC(5d804fbb) SHA1(8dc02eb9329f9c29d4bcc9a0315ae96085625d3e) )
	ROM_LOAD16_BYTE( "am2g.p3",  0x200000, 0x80000,  CRC(5ead0c06) SHA1(35d9aefc60e2c391e32f8119a6dc44434d91c09e) )
	ROM_LOAD16_BYTE( "am2g.p4",  0x200001, 0x80000,  CRC(de4fb542) SHA1(4bf8f8f6850fd819d91827d3c474bd488e61e5ac) )
	ROM_LOAD16_BYTE( "am2g.p5",  0x300000, 0x80000,  CRC(80b01ce2) SHA1(4a3a4bcff4bd9affd1a5eeca5781b6af05bbcc16) )
	ROM_LOAD16_BYTE( "am2g.p6",  0x300001, 0x80000,  CRC(3e134ecc) SHA1(1f8cdce62e693eb07c4620b64cc467339c0563de) )
	ROM_LOAD16_BYTE( "am2g.p7",  0x400000, 0x80000,  CRC(6eb36f1d) SHA1(08b9ec184d64bdbdfa61d3e991a3647e74a7756f) )
	ROM_LOAD16_BYTE( "am2g.p8",  0x400001, 0x80000,  CRC(dda353ef) SHA1(56a5b43f0b0bd9dbf348946a5758ebe63eadb8cf) )
ROM_END

ROM_START( v4cmaze2b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cr220.p1", 0x000000, 0x80000,  CRC(0a6ebe65) SHA1(504648c97bc9de6c3e0c9c73eb2f356632e1c562) )
	ROM_LOAD16_BYTE( "cr220.p2", 0x000001, 0x80000,  CRC(699ab8d4) SHA1(8a6516e515d111c29833f6a94845942d156cc54d) )
	ROM_LOAD16_BYTE( "cm2.p3",  0x100000, 0x80000,  CRC(88324715) SHA1(c6c8de4e5aeda14232ec7b026da389774b3c7bb1) )
	ROM_LOAD16_BYTE( "cm2.p4",  0x100001, 0x80000,  CRC(8d54a81d) SHA1(37753cf8595647aaf8b8267ca177b6744de9c6d4) )
	ROM_LOAD16_BYTE( "cm2.p5",  0x200000, 0x80000,  CRC(5cf8a2bf) SHA1(2514e78e82842fa5c85d26de35637269cd08b21d) )
	ROM_LOAD16_BYTE( "cm2.p6",  0x200001, 0x80000,  CRC(cf793d2d) SHA1(579c759f57fb6bb87aa27c9d5fb684058913dedc) )
	ROM_LOAD16_BYTE( "cm2.p7",  0x300000, 0x80000,  CRC(008aa4b0) SHA1(b4cec6d11abd0e111c295533700595398ff59075) )
	ROM_LOAD16_BYTE( "cm2.p8",  0x300001, 0x80000,  CRC(bac04f5a) SHA1(130721b7abf28dea1f8162705c8bfc5a4bb78152) )
ROM_END

ROM_START( v4cmaze2c )//Should be a set, but PROM 2 checksum error?
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm2_7.p1", 0x000000, 0x80000,  CRC(d455d5fe) SHA1(20186a5fff458d3eba15ccb564addc0b064736e0) )
	ROM_LOAD16_BYTE( "cm2_7.p2", 0x000001, 0x80000, BAD_DUMP CRC(55fef843) SHA1(ccc187c0137af54bad8b72181a544d5b78004ed8) )
	ROM_LOAD16_BYTE( "cm2.p3",  0x100000, 0x80000,  CRC(88324715) SHA1(c6c8de4e5aeda14232ec7b026da389774b3c7bb1) )
	ROM_LOAD16_BYTE( "cm2.p4",  0x100001, 0x80000,  CRC(8d54a81d) SHA1(37753cf8595647aaf8b8267ca177b6744de9c6d4) )
	ROM_LOAD16_BYTE( "cm2.p5",  0x200000, 0x80000,  CRC(5cf8a2bf) SHA1(2514e78e82842fa5c85d26de35637269cd08b21d) )
	ROM_LOAD16_BYTE( "cm2.p6",  0x200001, 0x80000,  CRC(cf793d2d) SHA1(579c759f57fb6bb87aa27c9d5fb684058913dedc) )
	ROM_LOAD16_BYTE( "cm2.p7",  0x300000, 0x80000,  CRC(008aa4b0) SHA1(b4cec6d11abd0e111c295533700595398ff59075) )
	ROM_LOAD16_BYTE( "cm2.p8",  0x300001, 0x80000,  CRC(bac04f5a) SHA1(130721b7abf28dea1f8162705c8bfc5a4bb78152) )
ROM_END



//The Crystal Maze Team Challenge
ROM_START( v4cmaze3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm3.p1",  0x000000, 0x80000,  CRC(2d2edee5) SHA1(0281ec97aaaaf4c7969340bd5995ac1541dbad54) )
	ROM_LOAD16_BYTE( "cm3.p2",  0x000001, 0x80000,  CRC(c223d7b9) SHA1(da9d730716a30d0e93f2a02c1efa7f19457ae010) )
	ROM_LOAD16_BYTE( "cm3.p3",  0x100000, 0x80000,  CRC(2959c77b) SHA1(8de533bfad48ad19a635dddcafa2a0825133b4de) )
	ROM_LOAD16_BYTE( "cm3.p4",  0x100001, 0x80000,  CRC(b7873e9a) SHA1(a71fac883e02d5f49aee0a20f92dbdb00640ce8d) )
	ROM_LOAD16_BYTE( "cm3.p5",  0x200000, 0x80000,  CRC(c8375070) SHA1(da2ba6591d8765f896c40d6526da8e945d02a182) )
	ROM_LOAD16_BYTE( "cm3.p6",  0x200001, 0x80000,  CRC(1ea36938) SHA1(43f62935b21232d23f662e1e124663267edb1283) )
	ROM_LOAD16_BYTE( "cm3.p7",  0x300000, 0x80000,  CRC(9de3802e) SHA1(ec792f115a0708d68046ba0beb314b7e1f1eb422) )
	ROM_LOAD16_BYTE( "cm3.p8",  0x300001, 0x80000,  CRC(1e6e60b0) SHA1(5e71714747073dd89852a84585642388ee440325) )
	ROM_LOAD16_BYTE( "cm3.p9",  0x400000, 0x80000,  CRC(bfba55a7) SHA1(22eb9b1f9fe83d3b424fd521b68e2976a1940df9) )
	ROM_LOAD16_BYTE( "cm3.p10",  0x400001, 0x80000,  CRC(07edda81) SHA1(e94525be03f30e407051992925bb0d693f3d809b) )
ROM_END

ROM_START( v4cmaze3d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm3d.p1",  0x000000, 0x80000,  CRC(245f00fa) SHA1(ff5ae1a2ae024dfc0b4104360626e4106f4cd36f) )
	ROM_LOAD16_BYTE( "cm3d.p2",  0x000001, 0x80000,  CRC(091adbcb) SHA1(1466b036d06f6335c90426095ad0f60ea958a29d) )
	ROM_LOAD16_BYTE( "cm3.p3",  0x100000, 0x80000,  CRC(2959c77b) SHA1(8de533bfad48ad19a635dddcafa2a0825133b4de) )
	ROM_LOAD16_BYTE( "cm3.p4",  0x100001, 0x80000,  CRC(b7873e9a) SHA1(a71fac883e02d5f49aee0a20f92dbdb00640ce8d) )
	ROM_LOAD16_BYTE( "cm3.p5",  0x200000, 0x80000,  CRC(c8375070) SHA1(da2ba6591d8765f896c40d6526da8e945d02a182) )
	ROM_LOAD16_BYTE( "cm3.p6",  0x200001, 0x80000,  CRC(1ea36938) SHA1(43f62935b21232d23f662e1e124663267edb1283) )
	ROM_LOAD16_BYTE( "cm3.p7",  0x300000, 0x80000,  CRC(9de3802e) SHA1(ec792f115a0708d68046ba0beb314b7e1f1eb422) )
	ROM_LOAD16_BYTE( "cm3.p8",  0x300001, 0x80000,  CRC(1e6e60b0) SHA1(5e71714747073dd89852a84585642388ee440325) )
	ROM_LOAD16_BYTE( "cm3.p9",  0x400000, 0x80000,  CRC(bfba55a7) SHA1(22eb9b1f9fe83d3b424fd521b68e2976a1940df9) )
	ROM_LOAD16_BYTE( "cm3.p10",  0x400001, 0x80000,  CRC(07edda81) SHA1(e94525be03f30e407051992925bb0d693f3d809b) )
ROM_END

ROM_START( v4cmaze3_amld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "am3z.p1",  0x000000, 0x80000,  CRC(9484e656) SHA1(3c35a8ebeddea56d73ce8db4e93c51cdd9546d59) )
	ROM_LOAD16_BYTE( "am3z.p2",  0x000001, 0x80000,  CRC(1865ee80) SHA1(b3ff8e1631d811b8e88664dd84ae82231ce1f5aa) )
	ROM_LOAD16_BYTE( "am3g.p1",  0x100000, 0x80000,  CRC(49fe36af) SHA1(7c39223b07f53ff57a56c3817299734491372170) )
	ROM_LOAD16_BYTE( "am3g.p2",  0x100001, 0x80000,  CRC(b8823cbd) SHA1(206d3b1b2daff1979f97841041661f8407c35d4d) )
	ROM_LOAD16_BYTE( "am3g.p3",  0x200000, 0x80000,  CRC(b1870f17) SHA1(54c6cabb56e4daa4ccf801d5e44b2789b116d562) )
	ROM_LOAD16_BYTE( "am3g.p4",  0x200001, 0x80000,  CRC(c015d446) SHA1(669007e841afeb1084d9062d0a47c159e4c83cc9) )
	ROM_LOAD16_BYTE( "am3g.p5",  0x300000, 0x80000,  CRC(9de3802e) SHA1(ec792f115a0708d68046ba0beb314b7e1f1eb422) )
	ROM_LOAD16_BYTE( "am3g.p6",  0x300001, 0x80000,  CRC(1e6e60b0) SHA1(5e71714747073dd89852a84585642388ee440325) )
	ROM_LOAD16_BYTE( "am3g.p7",  0x400000, 0x80000,  CRC(a4611e29) SHA1(91b164eea5dbdd1129ad12d7af2dbdb3cd68bcec) )
	ROM_LOAD16_BYTE( "am3g.p8",  0x400001, 0x80000,  CRC(1a10c22e) SHA1(8533a5db3922b80b6e9f74e4e432a2b64bc24fc0) )
ROM_END

ROM_START( v4cmaze3b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm30_8.p1", 0x000000, 0x80000,CRC(fde7333e) SHA1(a94d7b88972af599418b9893124457a11c125fcc) )
	ROM_LOAD16_BYTE( "cm30_8.p2", 0x000001, 0x80000,CRC(35ff7e31) SHA1(52fe305c63cde70de550230bad03d85b53f8e227) )
	ROM_LOAD16_BYTE( "cm3.p3",  0x100000, 0x80000,  CRC(2959c77b) SHA1(8de533bfad48ad19a635dddcafa2a0825133b4de) )
	ROM_LOAD16_BYTE( "cm3.p4",  0x100001, 0x80000,  CRC(b7873e9a) SHA1(a71fac883e02d5f49aee0a20f92dbdb00640ce8d) )
	ROM_LOAD16_BYTE( "cm3.p5",  0x200000, 0x80000,  CRC(c8375070) SHA1(da2ba6591d8765f896c40d6526da8e945d02a182) )
	ROM_LOAD16_BYTE( "cm3.p6",  0x200001, 0x80000,  CRC(1ea36938) SHA1(43f62935b21232d23f662e1e124663267edb1283) )
	ROM_LOAD16_BYTE( "cm3.p7",  0x300000, 0x80000,  CRC(9de3802e) SHA1(ec792f115a0708d68046ba0beb314b7e1f1eb422) )
	ROM_LOAD16_BYTE( "cm3.p8",  0x300001, 0x80000,  CRC(1e6e60b0) SHA1(5e71714747073dd89852a84585642388ee440325) )
	ROM_LOAD16_BYTE( "cm3.p9",  0x400000, 0x80000,  CRC(bfba55a7) SHA1(22eb9b1f9fe83d3b424fd521b68e2976a1940df9) )
	ROM_LOAD16_BYTE( "cm3.p10", 0x400001, 0x80000,  CRC(07edda81) SHA1(e94525be03f30e407051992925bb0d693f3d809b) )
ROM_END

ROM_START( v4cmaze3c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "cm30_6.p1",  0x000000, 0x80000,  CRC(68aa610a) SHA1(d57071a960402b12a13ac3af602cedade03989f8) )
	ROM_LOAD16_BYTE( "cm30_6.p2",  0x000001, 0x80000,  NO_DUMP )
	ROM_LOAD16_BYTE( "cm3.p3",  0x100000, 0x80000,  CRC(2959c77b) SHA1(8de533bfad48ad19a635dddcafa2a0825133b4de) )
	ROM_LOAD16_BYTE( "cm3.p4",  0x100001, 0x80000,  CRC(b7873e9a) SHA1(a71fac883e02d5f49aee0a20f92dbdb00640ce8d) )
	ROM_LOAD16_BYTE( "cm3.p5",  0x200000, 0x80000,  CRC(c8375070) SHA1(da2ba6591d8765f896c40d6526da8e945d02a182) )
	ROM_LOAD16_BYTE( "cm3.p6",  0x200001, 0x80000,  CRC(1ea36938) SHA1(43f62935b21232d23f662e1e124663267edb1283) )
	ROM_LOAD16_BYTE( "cm3.p7",  0x300000, 0x80000,  CRC(9de3802e) SHA1(ec792f115a0708d68046ba0beb314b7e1f1eb422) )
	ROM_LOAD16_BYTE( "cm3.p8",  0x300001, 0x80000,  CRC(1e6e60b0) SHA1(5e71714747073dd89852a84585642388ee440325) )
	ROM_LOAD16_BYTE( "cm3.p9",  0x400000, 0x80000,  CRC(bfba55a7) SHA1(22eb9b1f9fe83d3b424fd521b68e2976a1940df9) )
	ROM_LOAD16_BYTE( "cm3.p10",  0x400001, 0x80000,  CRC(07edda81) SHA1(e94525be03f30e407051992925bb0d693f3d809b) )
ROM_END


ROM_START( v4turnov )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tos.p1",  0x000000, 0x010000,  CRC(4044dbeb) SHA1(3aa553055c56f14564b1e33e1c1975337e639f70) )
	ROM_LOAD16_BYTE( "to.p2",   0x000001, 0x010000,  CRC(4bc4659a) SHA1(0e282134c4fe4e8c1cc7b16957903179e23c7abc) )
	ROM_LOAD16_BYTE( "to.p3",   0x020000, 0x010000,  CRC(273c7c14) SHA1(71feb555a05a0ff1ec674505cab72d93c9fbdf65) )
	ROM_LOAD16_BYTE( "to.p4",   0x020001, 0x010000,  CRC(83d29546) SHA1(cef90455b9d8a92424fe1aa10f20fd075d0e3091) )
	ROM_LOAD16_BYTE( "to.p5",   0x040000, 0x010000,  CRC(dceac511) SHA1(7a6d65464e23d832943f771c4cf580aabc6f0e44) )
	ROM_LOAD16_BYTE( "to.p6",   0x040001, 0x010000,  CRC(54c6afb7) SHA1(b724b87b6f4e47d220310b38c97be2fa73dcd617) )
	ROM_LOAD16_BYTE( "to.p7",   0x060000, 0x010000,  CRC(acf19542) SHA1(ad46ffb3c2c078a8e3712eff27aa61f0d1a7c059) )
	ROM_LOAD16_BYTE( "to.p8",   0x060001, 0x010000,  CRC(a5ca385d) SHA1(8df26a33ea7f5b577761c6f9d2fa4eaed74661f8) )
	ROM_LOAD16_BYTE( "to.p9",   0x080000, 0x010000,  CRC(6e85fde3) SHA1(14868d58829e13987e66f52e1899c4385987a87b) ) // ISSUE1 Q1
	ROM_LOAD16_BYTE( "to.p10",  0x080001, 0x010000,  CRC(fadd11a2) SHA1(2b2fbb0769ef6035688d495464f3ea3bc8c7c660) ) // ISSUE1 Q2
	ROM_LOAD16_BYTE( "to.p11",  0x0a0000, 0x010000,  CRC(2d72a61a) SHA1(ce455ab6fea452f96a3ad365178e0e5a0b437867) ) // ISSUE1 Q3
	ROM_LOAD16_BYTE( "to.p12",  0x0a0001, 0x010000,  CRC(a14eedb6) SHA1(219b887a334ff28a88ed2e50f0caff4b510cd549) ) // ISSUE1 Q4
	ROM_LOAD16_BYTE( "to.p13",  0x0c0000, 0x010000,  CRC(3f66ef6b) SHA1(60be6d3f8da1f3084db15ac1bb2470e55c0271de) ) // ISSUE1 Q5
	ROM_LOAD16_BYTE( "to.p14",  0x0c0001, 0x010000,  CRC(127ba65d) SHA1(e34dcd19efd31dc712daac940277bb17694ea61a) ) // ISSUE1 Q6
	ROM_LOAD16_BYTE( "to.p15",  0x0e0000, 0x010000,  CRC(ad787e31) SHA1(314ba312adfc71e4b3b2d52355ec692c192b74eb) ) // ISSUE1 Q7
	ROM_LOAD16_BYTE( "to.p16",  0x0e0001, 0x010000,  CRC(e635c942) SHA1(08f8b5fdb738647bc0b49938da05533be42a2d60) ) // ISSUE1 Q8
ROM_END

ROM_START( v4turnova )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "to3.p1",  0x000000, 0x010000, CRC(09751994) SHA1(72d4aa40f14411ef8064822de3f5a13bcc84aea3) )
	ROM_LOAD16_BYTE( "to.p2",   0x000001, 0x010000,  CRC(4bc4659a) SHA1(0e282134c4fe4e8c1cc7b16957903179e23c7abc) )
	ROM_LOAD16_BYTE( "to.p3",   0x020000, 0x010000,  CRC(273c7c14) SHA1(71feb555a05a0ff1ec674505cab72d93c9fbdf65) )
	ROM_LOAD16_BYTE( "to.p4",   0x020001, 0x010000,  CRC(83d29546) SHA1(cef90455b9d8a92424fe1aa10f20fd075d0e3091) )
	ROM_LOAD16_BYTE( "to.p5",   0x040000, 0x010000,  CRC(dceac511) SHA1(7a6d65464e23d832943f771c4cf580aabc6f0e44) )
	ROM_LOAD16_BYTE( "to.p6",   0x040001, 0x010000,  CRC(54c6afb7) SHA1(b724b87b6f4e47d220310b38c97be2fa73dcd617) )
	ROM_LOAD16_BYTE( "to.p7",   0x060000, 0x010000,  CRC(acf19542) SHA1(ad46ffb3c2c078a8e3712eff27aa61f0d1a7c059) )
	ROM_LOAD16_BYTE( "to.p8",   0x060001, 0x010000,  CRC(a5ca385d) SHA1(8df26a33ea7f5b577761c6f9d2fa4eaed74661f8) )
	ROM_LOAD16_BYTE( "to.p9",   0x080000, 0x010000,  CRC(6e85fde3) SHA1(14868d58829e13987e66f52e1899c4385987a87b) ) // ISSUE1 Q1
	ROM_LOAD16_BYTE( "to.p10",  0x080001, 0x010000,  CRC(fadd11a2) SHA1(2b2fbb0769ef6035688d495464f3ea3bc8c7c660) ) // ISSUE1 Q2
	ROM_LOAD16_BYTE( "to.p11",  0x0a0000, 0x010000,  CRC(2d72a61a) SHA1(ce455ab6fea452f96a3ad365178e0e5a0b437867) ) // ISSUE1 Q3
	ROM_LOAD16_BYTE( "to.p12",  0x0a0001, 0x010000,  CRC(a14eedb6) SHA1(219b887a334ff28a88ed2e50f0caff4b510cd549) ) // ISSUE1 Q4
	ROM_LOAD16_BYTE( "to.p13",  0x0c0000, 0x010000,  CRC(3f66ef6b) SHA1(60be6d3f8da1f3084db15ac1bb2470e55c0271de) ) // ISSUE1 Q5
	ROM_LOAD16_BYTE( "to.p14",  0x0c0001, 0x010000,  CRC(127ba65d) SHA1(e34dcd19efd31dc712daac940277bb17694ea61a) ) // ISSUE1 Q6
	ROM_LOAD16_BYTE( "to.p15",  0x0e0000, 0x010000,  CRC(ad787e31) SHA1(314ba312adfc71e4b3b2d52355ec692c192b74eb) ) // ISSUE1 Q7
	ROM_LOAD16_BYTE( "to.p16",  0x0e0001, 0x010000,  CRC(e635c942) SHA1(08f8b5fdb738647bc0b49938da05533be42a2d60) ) // ISSUE1 Q8
ROM_END

// this ROM just seems to be a corrupt version of existing ones, bad vector table
//  ROM_LOAD16_BYTE( "todo.p1", 0x000000, 0x010000, CRC(9111e702) SHA1(fa408e1c8fa56a96ffc3422335f105ef328a6edd) )

ROM_START( v4turnovc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "too.p1",  0x000000, 0x010000,  CRC(919b5207) SHA1(770be6e3b00e666c6f939167d35d43bb2d793e14) )
	ROM_LOAD16_BYTE( "to.p2",   0x000001, 0x010000,  CRC(4bc4659a) SHA1(0e282134c4fe4e8c1cc7b16957903179e23c7abc) )
	ROM_LOAD16_BYTE( "to.p3",   0x020000, 0x010000,  CRC(273c7c14) SHA1(71feb555a05a0ff1ec674505cab72d93c9fbdf65) )
	ROM_LOAD16_BYTE( "to.p4",   0x020001, 0x010000,  CRC(83d29546) SHA1(cef90455b9d8a92424fe1aa10f20fd075d0e3091) )
	ROM_LOAD16_BYTE( "to.p5",   0x040000, 0x010000,  CRC(dceac511) SHA1(7a6d65464e23d832943f771c4cf580aabc6f0e44) )
	ROM_LOAD16_BYTE( "to.p6",   0x040001, 0x010000,  CRC(54c6afb7) SHA1(b724b87b6f4e47d220310b38c97be2fa73dcd617) )
	ROM_LOAD16_BYTE( "to.p7",   0x060000, 0x010000,  CRC(acf19542) SHA1(ad46ffb3c2c078a8e3712eff27aa61f0d1a7c059) )
	ROM_LOAD16_BYTE( "to.p8",   0x060001, 0x010000,  CRC(a5ca385d) SHA1(8df26a33ea7f5b577761c6f9d2fa4eaed74661f8) )
	ROM_LOAD16_BYTE( "to.p9",   0x080000, 0x010000,  CRC(6e85fde3) SHA1(14868d58829e13987e66f52e1899c4385987a87b) ) // ISSUE1 Q1
	ROM_LOAD16_BYTE( "to.p10",  0x080001, 0x010000,  CRC(fadd11a2) SHA1(2b2fbb0769ef6035688d495464f3ea3bc8c7c660) ) // ISSUE1 Q2
	ROM_LOAD16_BYTE( "to.p11",  0x0a0000, 0x010000,  CRC(2d72a61a) SHA1(ce455ab6fea452f96a3ad365178e0e5a0b437867) ) // ISSUE1 Q3
	ROM_LOAD16_BYTE( "to.p12",  0x0a0001, 0x010000,  CRC(a14eedb6) SHA1(219b887a334ff28a88ed2e50f0caff4b510cd549) ) // ISSUE1 Q4
	ROM_LOAD16_BYTE( "to.p13",  0x0c0000, 0x010000,  CRC(3f66ef6b) SHA1(60be6d3f8da1f3084db15ac1bb2470e55c0271de) ) // ISSUE1 Q5
	ROM_LOAD16_BYTE( "to.p14",  0x0c0001, 0x010000,  CRC(127ba65d) SHA1(e34dcd19efd31dc712daac940277bb17694ea61a) ) // ISSUE1 Q6
	ROM_LOAD16_BYTE( "to.p15",  0x0e0000, 0x010000,  CRC(ad787e31) SHA1(314ba312adfc71e4b3b2d52355ec692c192b74eb) ) // ISSUE1 Q7
	ROM_LOAD16_BYTE( "to.p16",  0x0e0001, 0x010000,  CRC(e635c942) SHA1(08f8b5fdb738647bc0b49938da05533be42a2d60) ) // ISSUE1 Q8
ROM_END

ROM_START( v4turnovd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tov2.2p1",0x000000, 0x010000, CRC(460a5dd0) SHA1(42bc54b0ca206606b980dd80ccf0cbfb3210769d) )
	ROM_LOAD16_BYTE( "tov2.2p2",0x000000, 0x010000, NO_DUMP )
	// + an unknown number of additional ROMs
ROM_END



ROM_START( v4skltrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st.p1",  0x000000, 0x010000,  CRC(d9de47a5) SHA1(625bf40780203293fc34cd8cea8278b4b4a52a75) )
	ROM_LOAD16_BYTE( "st.p2",  0x000001, 0x010000,  CRC(b62575c2) SHA1(06d75e8a364750663d329650720021279e195236) )
	ROM_LOAD16_BYTE( "st.p3",  0x020000, 0x010000,  CRC(9506da76) SHA1(6ef28ab8ec1af455be8ecfab20243f0823dca7c1) )
	ROM_LOAD16_BYTE( "st.p4",  0x020001, 0x010000,  CRC(6ab447bc) SHA1(d01c209dbf4d19a6a7f878fa54ff1cb51e7dcba5) )
	ROM_LOAD16_BYTE( "st.q1",  0x040000, 0x010000,  CRC(4faca475) SHA1(69b498c543600b8e37ab0ed1863ba57845648f3c) ) // ISSUE1 Q1
	ROM_LOAD16_BYTE( "st.q2",  0x040001, 0x010000,  CRC(9f2c5938) SHA1(85527c4c0b7a1e66576d56607d89750fab082580) ) // ISSUE1 Q2
	ROM_LOAD16_BYTE( "st.q3",  0x060000, 0x010000,  CRC(6b6cb194) SHA1(aeac5dcc0827c17e758e3e821ae8a78a3a16ddce) ) // ISSUE1 Q3
	ROM_LOAD16_BYTE( "st.q4",  0x060001, 0x010000,  CRC(ec57bc17) SHA1(d9f522739dbb190fb941ca654299bbedbb8fb703) ) // ISSUE1 Q4
	ROM_LOAD16_BYTE( "st.q5",  0x080000, 0x010000,  CRC(7740a88b) SHA1(d9a683d3e0d6c1b4b59520f90f825124b7a61168) ) // ISSUE1 Q5
	ROM_LOAD16_BYTE( "st.q6",  0x080001, 0x010000,  CRC(95e97796) SHA1(f1a8de0ad02aca31f79a4fe8ba5044546163e3c4) ) // ISSUE1 Q6
	ROM_LOAD16_BYTE( "st.q7",  0x0a0000, 0x010000,  CRC(f3b8fe7f) SHA1(52d5be3f8cab419103f4727d0fb9d30f34c8f651) ) // ISSUE1 Q7
	ROM_LOAD16_BYTE( "st.q8",  0x0a0001, 0x010000,  CRC(b85e75a2) SHA1(b7b03b090c0ec6d92e9a25abb7fec0507356bdfc) ) // ISSUE1 Q8
	ROM_LOAD16_BYTE( "st.q9",  0x0c0000, 0x010000,  CRC(835f6001) SHA1(2cd9084c102d74bcb578c8ea22bbc9ea58f0ceab) ) // ISSUE1 Q9
	ROM_LOAD16_BYTE( "st.qa",  0x0c0001, 0x010000,  CRC(3fc62a0e) SHA1(0628de4b962d3fcca3757cd4e89b3005c9bfd218) ) // ISSUE1 Q10
ROM_END

ROM_START( v4skltrka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "sws.p1",0x000000, 0x010000, CRC(a96f11ea) SHA1(5e5c548e96674f8f98c1c4eb0a590979371f6f10) )
	ROM_LOAD16_BYTE( "sw.p2", 0x000001, 0x010000, CRC(2598ba38) SHA1(30509bd2f987949bb423600e13a464edaa7f9ce5) )
	ROM_LOAD16_BYTE( "sw.p3", 0x020000, 0x010000, CRC(1290cef3) SHA1(5cf63940c164becf64902846aa4fd5bc628f9ebe) )
	ROM_LOAD16_BYTE( "sw.p4", 0x020001, 0x010000, CRC(176f23c3) SHA1(e64460818329f23e449f65781e3dc56d3a1710a3) )
	ROM_LOAD16_BYTE( "st.q1",  0x040000, 0x010000,  CRC(4faca475) SHA1(69b498c543600b8e37ab0ed1863ba57845648f3c) )
	ROM_LOAD16_BYTE( "st.q2",  0x040001, 0x010000,  CRC(9f2c5938) SHA1(85527c4c0b7a1e66576d56607d89750fab082580) )
	ROM_LOAD16_BYTE( "st.q3",  0x060000, 0x010000,  CRC(6b6cb194) SHA1(aeac5dcc0827c17e758e3e821ae8a78a3a16ddce) )
	ROM_LOAD16_BYTE( "st.q4",  0x060001, 0x010000,  CRC(ec57bc17) SHA1(d9f522739dbb190fb941ca654299bbedbb8fb703) )
	ROM_LOAD16_BYTE( "st.q5",  0x080000, 0x010000,  CRC(7740a88b) SHA1(d9a683d3e0d6c1b4b59520f90f825124b7a61168) )
	ROM_LOAD16_BYTE( "st.q6",  0x080001, 0x010000,  CRC(95e97796) SHA1(f1a8de0ad02aca31f79a4fe8ba5044546163e3c4) )
	ROM_LOAD16_BYTE( "st.q7",  0x0a0000, 0x010000,  CRC(f3b8fe7f) SHA1(52d5be3f8cab419103f4727d0fb9d30f34c8f651) )
	ROM_LOAD16_BYTE( "st.q8",  0x0a0001, 0x010000,  CRC(b85e75a2) SHA1(b7b03b090c0ec6d92e9a25abb7fec0507356bdfc) )
	ROM_LOAD16_BYTE( "st.q9",  0x0c0000, 0x010000,  CRC(835f6001) SHA1(2cd9084c102d74bcb578c8ea22bbc9ea58f0ceab) )
	ROM_LOAD16_BYTE( "st.qa",  0x0c0001, 0x010000,  CRC(3fc62a0e) SHA1(0628de4b962d3fcca3757cd4e89b3005c9bfd218) )
ROM_END


ROM_START( v4sklcsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "scs.p1", 0x000000, 0x010000, CRC(3181524e) SHA1(e801a036b9d509d8e2e5f7c9c6ccd9d9ddf76b2f) )
	ROM_LOAD16_BYTE( "scs.p2", 0x000001, 0x010000, CRC(a7596804) SHA1(f27fc3ee7a17904ea08baa421e14d02226428c42) )
	ROM_LOAD16_BYTE( "scs.p3", 0x020000, 0x010000, CRC(b3ded2bc) SHA1(39cb15fde4aea1187668120d548bfe022835b600) )
	ROM_LOAD16_BYTE( "scs.p4", 0x020001, 0x010000, CRC(2ce2e9d1) SHA1(c492fea2e179796a360131c19a9cba5bbfe3c7fd) )

	/* missing questions */
ROM_END




ROM_START( v4tmach )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tm20.p1",  0x000000, 0x010000,  CRC(6919697c) SHA1(786d7b9ab218dbf54ff839d1f83580c409c725b3) )
	ROM_LOAD16_BYTE( "tm20.p2",  0x000001, 0x010000,  CRC(d13b56e4) SHA1(623e73995da93c07b51ce0a5843dba1f853529dd) )
	ROM_LOAD16_BYTE( "tm20.p3",  0x020000, 0x010000,  CRC(efd3ae64) SHA1(9d2a3b65048e04842205751c6921d2550f38bd52) )
	ROM_LOAD16_BYTE( "tm20.p4",  0x020001, 0x010000,  CRC(602ba3fb) SHA1(7243f58df9a26adfd1a149a1e60630b187787dd0) )
	// questions can go in any slot, game detects what is installed
	ROM_LOAD16_BYTE( "issue3_q16" , 0x040000, 0x010000,  CRC(ce2bf15e) SHA1(29c7f2e718bce415b0b8dc6d902bf74dad6b1ef4) ) // ISSUE3 Q16
	ROM_LOAD16_BYTE( "issue3_q17",  0x040001, 0x010000,  CRC(5cab773e) SHA1(59a235c51a975b341bdbb88e909729507408f75b) ) // ISSUE3 Q17
	ROM_LOAD16_BYTE( "issue3_q18",  0x060000, 0x010000,  CRC(27de90b3) SHA1(625c98e555f7b627ea96653926b8917996a2fdb7) ) // ISSUE3 Q18
	ROM_LOAD16_BYTE( "issue3_q19",  0x060001, 0x010000,  CRC(083f6c65) SHA1(291ad39ee5f8eba9da293d9206b1f6a6d852f9bd) ) // ISSUE3 Q19
	ROM_LOAD16_BYTE( "issue3_q20",  0x080000, 0x010000,  CRC(73747644) SHA1(ae252fc95c069a3c82e155220fbfcb74dd43bf89) ) // ISSUE3 Q20
ROM_END

ROM_START( v4tmachd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tmd.p1",   0x000000, 0x010000,  CRC(e21045e0) SHA1(c4d0e80970ec8558db777a882edc5a0c80767375) )
	ROM_LOAD16_BYTE( "tm20.p2",  0x000001, 0x010000,  CRC(d13b56e4) SHA1(623e73995da93c07b51ce0a5843dba1f853529dd) )
	ROM_LOAD16_BYTE( "tm20.p3",  0x020000, 0x010000,  CRC(efd3ae64) SHA1(9d2a3b65048e04842205751c6921d2550f38bd52) )
	ROM_LOAD16_BYTE( "tm20.p4",  0x020001, 0x010000,  CRC(602ba3fb) SHA1(7243f58df9a26adfd1a149a1e60630b187787dd0) )
	// questions can go in any slot, game detects what is installed
	ROM_LOAD16_BYTE( "issue3_q16" , 0x040000, 0x010000,  CRC(ce2bf15e) SHA1(29c7f2e718bce415b0b8dc6d902bf74dad6b1ef4) ) // ISSUE3 Q16
	ROM_LOAD16_BYTE( "issue3_q17",  0x040001, 0x010000,  CRC(5cab773e) SHA1(59a235c51a975b341bdbb88e909729507408f75b) ) // ISSUE3 Q17
	ROM_LOAD16_BYTE( "issue3_q18",  0x060000, 0x010000,  CRC(27de90b3) SHA1(625c98e555f7b627ea96653926b8917996a2fdb7) ) // ISSUE3 Q18
	ROM_LOAD16_BYTE( "issue3_q19",  0x060001, 0x010000,  CRC(083f6c65) SHA1(291ad39ee5f8eba9da293d9206b1f6a6d852f9bd) ) // ISSUE3 Q19
	ROM_LOAD16_BYTE( "issue3_q20",  0x080000, 0x010000,  CRC(73747644) SHA1(ae252fc95c069a3c82e155220fbfcb74dd43bf89) ) // ISSUE3 Q20
ROM_END


ROM_START( v4tmach1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tm20.p1",  0x000000, 0x010000,  CRC(6919697c) SHA1(786d7b9ab218dbf54ff839d1f83580c409c725b3) )
	ROM_LOAD16_BYTE( "tm20.p2",  0x000001, 0x010000,  CRC(d13b56e4) SHA1(623e73995da93c07b51ce0a5843dba1f853529dd) )
	ROM_LOAD16_BYTE( "tm20.p3",  0x020000, 0x010000,  CRC(efd3ae64) SHA1(9d2a3b65048e04842205751c6921d2550f38bd52) )
	ROM_LOAD16_BYTE( "tm20.p4",  0x020001, 0x010000,  CRC(602ba3fb) SHA1(7243f58df9a26adfd1a149a1e60630b187787dd0) )
	// questions can go in any slot, game detects what is installed
	ROM_LOAD16_BYTE( "issue1_q1", 0x040000, 0x010000, CRC(6af4d58b) SHA1(ee547dad30cd9940f0b017caac97aeb046604f22) ) // ISSUE1 Q1
	ROM_LOAD16_BYTE( "issue1_q2", 0x040001, 0x010000, CRC(b01b7687) SHA1(99db448d7e40c2ec16afef3c10abc8a9493f2ab4) ) // ISSUE1 Q2
	ROM_LOAD16_BYTE( "issue1_q3", 0x060000, 0x010000, CRC(e97f14eb) SHA1(9163d11e1bc5a13d5002a13bc18b65a91e1738c7) ) // ISSUE1 Q3
	ROM_LOAD16_BYTE( "issue1_q4", 0x060001, 0x010000, CRC(6134d918) SHA1(e4b4d6b08d94729d4dcca474d4c7bdcb267530a8) ) // ISSUE1 Q4
	ROM_LOAD16_BYTE( "issue1_q5", 0x080000, 0x010000, CRC(6c07814b) SHA1(a97feada5cfa1bb059837b292637fbad9c7137ac) ) // ISSUE1 Q5
	ROM_LOAD16_BYTE( "issue1_q6", 0x080001, 0x010000, CRC(5f16a536) SHA1(3435282bfb940604fb44e06dc4748e668768f286) ) // ISSUE1 Q6
	ROM_LOAD16_BYTE( "issue1_q7", 0x0a0000, 0x010000, CRC(9afdce0b) SHA1(a969038d9ce2a2cff1e1a75959c05a3f03f08235) ) // ISSUE1 Q7
	ROM_LOAD16_BYTE( "issue1_q8", 0x0a0001, 0x010000, CRC(f1878251) SHA1(b6a8527112bcdf21b9a0acab4d8fa507a96aaba7) ) // ISSUE1 Q8
	ROM_LOAD16_BYTE( "issue1_q9", 0x0c0000, 0x010000, CRC(ace01faa) SHA1(79d6247a74e1bce0d76ea3788d0022d9e50173c4) ) // ISSUE1 Q9
	ROM_LOAD16_BYTE( "issue1_q10",0x0c0001, 0x010000, CRC(021f4523) SHA1(10884665f5700c147c7035d0c98f3889917ff015) ) // ISSUE1 Q10
ROM_END

ROM_START( v4tmach1d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tmd.p1",   0x000000, 0x010000,  CRC(e21045e0) SHA1(c4d0e80970ec8558db777a882edc5a0c80767375) )
	ROM_LOAD16_BYTE( "tm20.p2",  0x000001, 0x010000,  CRC(d13b56e4) SHA1(623e73995da93c07b51ce0a5843dba1f853529dd) )
	ROM_LOAD16_BYTE( "tm20.p3",  0x020000, 0x010000,  CRC(efd3ae64) SHA1(9d2a3b65048e04842205751c6921d2550f38bd52) )
	ROM_LOAD16_BYTE( "tm20.p4",  0x020001, 0x010000,  CRC(602ba3fb) SHA1(7243f58df9a26adfd1a149a1e60630b187787dd0) )
	// questions can go in any slot, game detects what is installed
	ROM_LOAD16_BYTE( "issue1_q1", 0x040000, 0x010000, CRC(6af4d58b) SHA1(ee547dad30cd9940f0b017caac97aeb046604f22) ) // ISSUE1 Q1
	ROM_LOAD16_BYTE( "issue1_q2", 0x040001, 0x010000, CRC(b01b7687) SHA1(99db448d7e40c2ec16afef3c10abc8a9493f2ab4) ) // ISSUE1 Q2
	ROM_LOAD16_BYTE( "issue1_q3", 0x060000, 0x010000, CRC(e97f14eb) SHA1(9163d11e1bc5a13d5002a13bc18b65a91e1738c7) ) // ISSUE1 Q3
	ROM_LOAD16_BYTE( "issue1_q4", 0x060001, 0x010000, CRC(6134d918) SHA1(e4b4d6b08d94729d4dcca474d4c7bdcb267530a8) ) // ISSUE1 Q4
	ROM_LOAD16_BYTE( "issue1_q5", 0x080000, 0x010000, CRC(6c07814b) SHA1(a97feada5cfa1bb059837b292637fbad9c7137ac) ) // ISSUE1 Q5
	ROM_LOAD16_BYTE( "issue1_q6", 0x080001, 0x010000, CRC(5f16a536) SHA1(3435282bfb940604fb44e06dc4748e668768f286) ) // ISSUE1 Q6
	ROM_LOAD16_BYTE( "issue1_q7", 0x0a0000, 0x010000, CRC(9afdce0b) SHA1(a969038d9ce2a2cff1e1a75959c05a3f03f08235) ) // ISSUE1 Q7
	ROM_LOAD16_BYTE( "issue1_q8", 0x0a0001, 0x010000, CRC(f1878251) SHA1(b6a8527112bcdf21b9a0acab4d8fa507a96aaba7) ) // ISSUE1 Q8
	ROM_LOAD16_BYTE( "issue1_q9", 0x0c0000, 0x010000, CRC(ace01faa) SHA1(79d6247a74e1bce0d76ea3788d0022d9e50173c4) ) // ISSUE1 Q9
	ROM_LOAD16_BYTE( "issue1_q10",0x0c0001, 0x010000, CRC(021f4523) SHA1(10884665f5700c147c7035d0c98f3889917ff015) ) // ISSUE1 Q10
ROM_END

ROM_START( v4tmach2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tm20.p1",  0x000000, 0x010000,  CRC(6919697c) SHA1(786d7b9ab218dbf54ff839d1f83580c409c725b3) )
	ROM_LOAD16_BYTE( "tm20.p2",  0x000001, 0x010000,  CRC(d13b56e4) SHA1(623e73995da93c07b51ce0a5843dba1f853529dd) )
	ROM_LOAD16_BYTE( "tm20.p3",  0x020000, 0x010000,  CRC(efd3ae64) SHA1(9d2a3b65048e04842205751c6921d2550f38bd52) )
	ROM_LOAD16_BYTE( "tm20.p4",  0x020001, 0x010000,  CRC(602ba3fb) SHA1(7243f58df9a26adfd1a149a1e60630b187787dd0) )
	// questions can go in any slot, game detects what is installed
	ROM_LOAD16_BYTE( "issue2_q11" ,  0x040000, 0x010000,  CRC(e8ed736f) SHA1(e7068c550aa39a6e8f1692a16794147e996d36b4) ) // ISSUE2 Q11
	ROM_LOAD16_BYTE( "issue2_q12" ,  0x040001, 0x010000,  BAD_DUMP CRC(adddd8a7) SHA1(73a8dd191eda2f4b41b79d4b55723731953b8970) ) // ISSUE2 Q12 // game doesn't recognize this one in any slot? bad? wrong game?
	ROM_LOAD16_BYTE( "issue2_q13" ,  0x060000, 0x010000,  CRC(3de147dd) SHA1(d2111d54d1604fe2da0133102bbfee706f8f542e) ) // ISSUE2 Q13
	ROM_LOAD16_BYTE( "issue2_q14" ,  0x060001, 0x010000,  CRC(02abb026) SHA1(42224678e5913090c91c21672661beb8e27127a8) ) // ISSUE2 Q14
	ROM_LOAD16_BYTE( "issue2_q15" ,  0x080000, 0x010000,  CRC(7894ac8b) SHA1(dc46bd108ac4f67a9062bb7ace91aa51f069cbc8) ) // ISSUE2 Q15
ROM_END


ROM_START( v4tmach2d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tmd.p1",   0x000000, 0x010000,  CRC(e21045e0) SHA1(c4d0e80970ec8558db777a882edc5a0c80767375) )
	ROM_LOAD16_BYTE( "tm20.p2",  0x000001, 0x010000,  CRC(d13b56e4) SHA1(623e73995da93c07b51ce0a5843dba1f853529dd) )
	ROM_LOAD16_BYTE( "tm20.p3",  0x020000, 0x010000,  CRC(efd3ae64) SHA1(9d2a3b65048e04842205751c6921d2550f38bd52) )
	ROM_LOAD16_BYTE( "tm20.p4",  0x020001, 0x010000,  CRC(602ba3fb) SHA1(7243f58df9a26adfd1a149a1e60630b187787dd0) )
	// questions can go in any slot, game detects what is installed
	ROM_LOAD16_BYTE( "issue2_q11" ,  0x040000, 0x010000,  CRC(e8ed736f) SHA1(e7068c550aa39a6e8f1692a16794147e996d36b4) ) // ISSUE2 Q11
	ROM_LOAD16_BYTE( "issue2_q12" ,  0x040001, 0x010000,  BAD_DUMP CRC(adddd8a7) SHA1(73a8dd191eda2f4b41b79d4b55723731953b8970) ) // ISSUE2 Q12 // game doesn't recognize this one in any slot? bad? wrong game?
	ROM_LOAD16_BYTE( "issue2_q13" ,  0x060000, 0x010000,  CRC(3de147dd) SHA1(d2111d54d1604fe2da0133102bbfee706f8f542e) ) // ISSUE2 Q13
	ROM_LOAD16_BYTE( "issue2_q14" ,  0x060001, 0x010000,  CRC(02abb026) SHA1(42224678e5913090c91c21672661beb8e27127a8) ) // ISSUE2 Q14
	ROM_LOAD16_BYTE( "issue2_q15" ,  0x080000, 0x010000,  CRC(7894ac8b) SHA1(dc46bd108ac4f67a9062bb7ace91aa51f069cbc8) ) // ISSUE2 Q15
ROM_END

ROM_START( v4mate )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mats.p1",  0x000000, 0x080000,  CRC(ebc7ea7e) SHA1(11015489a803ba5c8dbdafd632424bbd6080aece) )
	ROM_LOAD16_BYTE( "mats.p2",  0x000001, 0x080000,  CRC(a4c7e9b4) SHA1(30148c0257181bb88159e02d2b7cd79995ee84a7) )
	ROM_LOAD16_BYTE( "matg.p3",  0x100000, 0x080000,  CRC(571f4e8e) SHA1(51babacb5d9fb1cc9e1e56a3b2a355597d04f178) )
	ROM_LOAD16_BYTE( "matg.p4",  0x100001, 0x080000,  CRC(52d8657b) SHA1(e44e1db13c4abd4fedcd72df9dce1df594f74e44) )
	ROM_LOAD16_BYTE( "matg.p5",  0x200000, 0x080000,  CRC(9f0c9552) SHA1(8b1197f20853e18841a8f64fd5ff58cdd0bd1dbd) )
	ROM_LOAD16_BYTE( "matg.p6",  0x200001, 0x080000,  CRC(59f2b6a8) SHA1(4921cf1fc4c3bc50d2598b63726f61f68b41658c) )
	ROM_LOAD16_BYTE( "matg.p7",  0x300000, 0x080000,  CRC(64c0031a) SHA1(a519addd5d8f4696967ec84c163d28cb81ff9f32) )
	ROM_LOAD16_BYTE( "matg.p8",  0x300001, 0x080000,  CRC(22370dae) SHA1(72b1686b458750b5ee9dfe5599c308329d2c79d5) )
	ROM_LOAD16_BYTE( "matq.p9",  0x400000, 0x040000,  CRC(2d42e982) SHA1(80e476d5d65662059daa93a2fd383aecb74903c1) )
	ROM_LOAD16_BYTE( "matq.p10", 0x400001, 0x040000,  CRC(90364c3c) SHA1(6a4d2a3dd2cf9040887503888e6f38341578ad64) )

	/* Mating Game has an extra OKI sound chip */
	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "matsnd.p1",  0x000000, 0x080000,  CRC(f16df9e3) SHA1(fd9b82d73e18e635a9ea4aabd8c0b4aa2c8c6fdb) )
	ROM_LOAD( "matsnd.p2",  0x080000, 0x080000,  CRC(0c041621) SHA1(9156bf17ef6652968d9fbdc0b2bde64d3a67459c) )
	ROM_LOAD( "matsnd.p3",  0x100000, 0x080000,  CRC(c7435af9) SHA1(bd6080afaaaecca0d65e6d4125b46849aa4d1f33) )
	ROM_LOAD( "matsnd.p4",  0x180000, 0x080000,  CRC(d7e65c5b) SHA1(5575fb9f948158f2e94c986bf4bca9c9ee66a489) )
ROM_END

ROM_START( v4mated )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "matd.p1",  0x000000, 0x080000,  CRC(e660909f) SHA1(0acd990264fd7faf1f91a796d2438e8c2c7b83d1) )
	ROM_LOAD16_BYTE( "matd.p2",  0x000001, 0x080000,  CRC(a4c7e9b4) SHA1(30148c0257181bb88159e02d2b7cd79995ee84a7) )
	ROM_LOAD16_BYTE( "matg.p3",  0x100000, 0x080000,  CRC(571f4e8e) SHA1(51babacb5d9fb1cc9e1e56a3b2a355597d04f178) )
	ROM_LOAD16_BYTE( "matg.p4",  0x100001, 0x080000,  CRC(52d8657b) SHA1(e44e1db13c4abd4fedcd72df9dce1df594f74e44) )
	ROM_LOAD16_BYTE( "matg.p5",  0x200000, 0x080000,  CRC(9f0c9552) SHA1(8b1197f20853e18841a8f64fd5ff58cdd0bd1dbd) )
	ROM_LOAD16_BYTE( "matg.p6",  0x200001, 0x080000,  CRC(59f2b6a8) SHA1(4921cf1fc4c3bc50d2598b63726f61f68b41658c) )
	ROM_LOAD16_BYTE( "matg.p7",  0x300000, 0x080000,  CRC(64c0031a) SHA1(a519addd5d8f4696967ec84c163d28cb81ff9f32) )
	ROM_LOAD16_BYTE( "matg.p8",  0x300001, 0x080000,  CRC(22370dae) SHA1(72b1686b458750b5ee9dfe5599c308329d2c79d5) )
	ROM_LOAD16_BYTE( "matq.p9",  0x400000, 0x040000,  CRC(2d42e982) SHA1(80e476d5d65662059daa93a2fd383aecb74903c1) )
	ROM_LOAD16_BYTE( "matq.p10", 0x400001, 0x040000,  CRC(90364c3c) SHA1(6a4d2a3dd2cf9040887503888e6f38341578ad64) )

	/* Mating Game has an extra OKI sound chip */
	ROM_REGION( 0x200000, "okicard:msm6376", 0 )
	ROM_LOAD( "matsnd.p1",  0x000000, 0x080000,  CRC(f16df9e3) SHA1(fd9b82d73e18e635a9ea4aabd8c0b4aa2c8c6fdb) )
	ROM_LOAD( "matsnd.p2",  0x080000, 0x080000,  CRC(0c041621) SHA1(9156bf17ef6652968d9fbdc0b2bde64d3a67459c) )
	ROM_LOAD( "matsnd.p3",  0x100000, 0x080000,  CRC(c7435af9) SHA1(bd6080afaaaecca0d65e6d4125b46849aa4d1f33) )
	ROM_LOAD( "matsnd.p4",  0x180000, 0x080000,  CRC(d7e65c5b) SHA1(5575fb9f948158f2e94c986bf4bca9c9ee66a489) )
ROM_END

ROM_START( v4addlad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "als_21.p1", 0x000000, 0x010000, CRC(c2c4ceea) SHA1(0c5ea4d02ba053191855547b714da9a7f61f3713) )
	ROM_LOAD16_BYTE( "al_21.p2",  0x000001, 0x010000, CRC(d5909619) SHA1(eb8d978ed28cd3afab6b3c1274d79bc6b188d240) )
	ROM_LOAD16_BYTE( "al_21.p3",  0x020000, 0x010000, CRC(5dc65d9f) SHA1(52dffb162a0e5649e056817e9db567cc7653538c) )
	ROM_LOAD16_BYTE( "al_21.p4",  0x020001, 0x010000, CRC(17c77264) SHA1(321372c1c0d25cfc5a1302ccfd5680d9bf62d31b) )
	ROM_LOAD16_BYTE( "al.q1",  0x040000, 0x10000,  CRC(b9b50b70) SHA1(1887ab00ee004e3f27902d6880fa31277a981891) )
	ROM_LOAD16_BYTE( "al.q2",  0x040001, 0x10000,  CRC(1bed86ac) SHA1(8e6563b5441ad9ddd468a3d9ae906733fed7912a) )
	ROM_LOAD16_BYTE( "al.q3",  0x060000, 0x10000,  CRC(294d8f28) SHA1(9f9aca491ba6c4dc5cfb91da867990a9610c3a28) )
	ROM_LOAD16_BYTE( "al.q4",  0x060001, 0x10000,  CRC(aa3e9fbd) SHA1(59d0868f4c8b3f56ca31a11d2e6af83b202bb735) )
	ROM_LOAD16_BYTE( "al.q5",  0x080000, 0x10000,  CRC(503f4193) SHA1(86df379c736598ba59446961bf0666e155164e1d) )
	ROM_LOAD16_BYTE( "al.q6",  0x080001, 0x10000,  CRC(9fd77e52) SHA1(d8fdebb0fd57ab9ea9797dd386168581a45ebc62) )
	ROM_LOAD16_BYTE( "al.q7",  0x0a0000, 0x10000,  CRC(cf3fa7c7) SHA1(fa1edf09c6d3a8b5737474117b0306ef64f7741c) )
	ROM_LOAD16_BYTE( "al.q8",  0x0a0001, 0x10000,  CRC(55058a63) SHA1(cf9edef5264f4301be4ee11f221ab67a5183a603) )
	ROM_LOAD16_BYTE( "al.q9",  0x0c0000, 0x10000,  CRC(22274191) SHA1(9bee5709edcd853e96408f37447c0f5324610903) )
	ROM_LOAD16_BYTE( "al.qa",  0x0c0001, 0x10000,  CRC(1fe98b4d) SHA1(533afeaea42903905f6f1206bba1a023b141bdd9) )

	ROM_REGION( 0x1000, "characteriser:fakechr", 0 )
	// this is a state result dump of the PAL, a 64x64 table of 6-bit values, where a write sets the column index
	// and the result of the previous read sets the row index, and a write of 00 resets the state machine?
	ROM_LOAD( "addersandladders_video_mpu4.chr", 0x0000, 0x1000, CRC(2e191981) SHA1(09d57291f73bea6d87007256137d039f5d279235) )
ROM_END

ROM_START( v4addladd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ald_21.p1", 0x000000, 0x010000, CRC(ecc7c79c) SHA1(e03f470d0b83ed81af737a1d16a02528df733149) )
	ROM_LOAD16_BYTE( "al_21.p2",  0x000001, 0x010000, CRC(d5909619) SHA1(eb8d978ed28cd3afab6b3c1274d79bc6b188d240) )
	ROM_LOAD16_BYTE( "al_21.p3",  0x020000, 0x010000, CRC(5dc65d9f) SHA1(52dffb162a0e5649e056817e9db567cc7653538c) )
	ROM_LOAD16_BYTE( "al_21.p4",  0x020001, 0x010000, CRC(17c77264) SHA1(321372c1c0d25cfc5a1302ccfd5680d9bf62d31b) )
	ROM_LOAD16_BYTE( "al.q1",  0x040000, 0x10000,  CRC(b9b50b70) SHA1(1887ab00ee004e3f27902d6880fa31277a981891) )
	ROM_LOAD16_BYTE( "al.q2",  0x040001, 0x10000,  CRC(1bed86ac) SHA1(8e6563b5441ad9ddd468a3d9ae906733fed7912a) )
	ROM_LOAD16_BYTE( "al.q3",  0x060000, 0x10000,  CRC(294d8f28) SHA1(9f9aca491ba6c4dc5cfb91da867990a9610c3a28) )
	ROM_LOAD16_BYTE( "al.q4",  0x060001, 0x10000,  CRC(aa3e9fbd) SHA1(59d0868f4c8b3f56ca31a11d2e6af83b202bb735) )
	ROM_LOAD16_BYTE( "al.q5",  0x080000, 0x10000,  CRC(503f4193) SHA1(86df379c736598ba59446961bf0666e155164e1d) )
	ROM_LOAD16_BYTE( "al.q6",  0x080001, 0x10000,  CRC(9fd77e52) SHA1(d8fdebb0fd57ab9ea9797dd386168581a45ebc62) )
	ROM_LOAD16_BYTE( "al.q7",  0x0a0000, 0x10000,  CRC(cf3fa7c7) SHA1(fa1edf09c6d3a8b5737474117b0306ef64f7741c) )
	ROM_LOAD16_BYTE( "al.q8",  0x0a0001, 0x10000,  CRC(55058a63) SHA1(cf9edef5264f4301be4ee11f221ab67a5183a603) )
	ROM_LOAD16_BYTE( "al.q9",  0x0c0000, 0x10000,  CRC(22274191) SHA1(9bee5709edcd853e96408f37447c0f5324610903) )
	ROM_LOAD16_BYTE( "al.qa",  0x0c0001, 0x10000,  CRC(1fe98b4d) SHA1(533afeaea42903905f6f1206bba1a023b141bdd9) )

	ROM_REGION( 0x1000, "characteriser:fakechr", 0 )
	// this is a state result dump of the PAL, a 64x64 table of 6-bit values, where a write sets the column index
	// and the result of the previous read sets the row index, and a write of 00 resets the state machine?
	ROM_LOAD( "addersandladders_video_mpu4.chr", 0x0000, 0x1000, CRC(2e191981) SHA1(09d57291f73bea6d87007256137d039f5d279235) )
ROM_END

ROM_START( v4addlad20 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "al.p1",  0x000000, 0x10000,  CRC(97579eb2) SHA1(68f08ab73e0c8044de32f7eea1b3abc1fefff830) )
	ROM_LOAD16_BYTE( "al.p2",  0x000001, 0x10000,  CRC(ca877046) SHA1(7d0e1bb471b7acc89d04953ee5e0b34f64d58325) )
	ROM_LOAD16_BYTE( "al.p3",  0x020000, 0x10000,  CRC(92b5e1c4) SHA1(910126527ddb9d83f5c9c29caf2424ae4ebbc640) )
	ROM_LOAD16_BYTE( "al.p4",  0x020001, 0x10000,  CRC(5e0e3b23) SHA1(3862e7b873a1e2762032151289ce2ad886a8cc3c) )
	ROM_LOAD16_BYTE( "al.q1",  0x040000, 0x10000,  CRC(b9b50b70) SHA1(1887ab00ee004e3f27902d6880fa31277a981891) )
	ROM_LOAD16_BYTE( "al.q2",  0x040001, 0x10000,  CRC(1bed86ac) SHA1(8e6563b5441ad9ddd468a3d9ae906733fed7912a) )
	ROM_LOAD16_BYTE( "al.q3",  0x060000, 0x10000,  CRC(294d8f28) SHA1(9f9aca491ba6c4dc5cfb91da867990a9610c3a28) )
	ROM_LOAD16_BYTE( "al.q4",  0x060001, 0x10000,  CRC(aa3e9fbd) SHA1(59d0868f4c8b3f56ca31a11d2e6af83b202bb735) )
	ROM_LOAD16_BYTE( "al.q5",  0x080000, 0x10000,  CRC(503f4193) SHA1(86df379c736598ba59446961bf0666e155164e1d) )
	ROM_LOAD16_BYTE( "al.q6",  0x080001, 0x10000,  CRC(9fd77e52) SHA1(d8fdebb0fd57ab9ea9797dd386168581a45ebc62) )
	ROM_LOAD16_BYTE( "al.q7",  0x0a0000, 0x10000,  CRC(cf3fa7c7) SHA1(fa1edf09c6d3a8b5737474117b0306ef64f7741c) )
	ROM_LOAD16_BYTE( "al.q8",  0x0a0001, 0x10000,  CRC(55058a63) SHA1(cf9edef5264f4301be4ee11f221ab67a5183a603) )
	ROM_LOAD16_BYTE( "al.q9",  0x0c0000, 0x10000,  CRC(22274191) SHA1(9bee5709edcd853e96408f37447c0f5324610903) )
	ROM_LOAD16_BYTE( "al.qa",  0x0c0001, 0x10000,  CRC(1fe98b4d) SHA1(533afeaea42903905f6f1206bba1a023b141bdd9) )

	ROM_REGION( 0x1000, "characteriser:fakechr", 0 )
	// this is a state result dump of the PAL, a 64x64 table of 6-bit values, where a write sets the column index
	// and the result of the previous read sets the row index, and a write of 00 resets the state machine?
	ROM_LOAD( "addersandladders_video_mpu4.chr", 0x0000, 0x1000, CRC(2e191981) SHA1(09d57291f73bea6d87007256137d039f5d279235) )
ROM_END


// for video CPU
// bp 32c4,a0 == ffd001
// (a4 == aab54) when doing startup questions check
ROM_START( v4strike )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "sils.p1", 0x000000, 0x020000,  CRC(66ed6696) SHA1(a6aa68eb212254213db5573dfb9da1e9e06a8e39) )
	ROM_LOAD16_BYTE( "sil.p2",  0x000001, 0x020000,  CRC(1afc07b7) SHA1(38777d56192b640b003d8dbf4b793cee0c81d9b2) )
	ROM_LOAD16_BYTE( "sil.p3",  0x040000, 0x020000,  CRC(40f5851c) SHA1(0e3dc0dd2a257a955a1f250556d047481ae87269) )
	ROM_LOAD16_BYTE( "sil.p4",  0x040001, 0x020000,  CRC(657e297e) SHA1(306f40376115ca40099a0010650b5edc183a2c57) )
	ROM_LOAD16_BYTE( "sil.p5",  0x080000, 0x020000,  CRC(28bced09) SHA1(7ba5013f1e0f4e921581b23c4a1d4c005a043b66) )
	ROM_LOAD16_BYTE( "sil.p6",  0x080001, 0x020000,  CRC(6f5fc296) SHA1(bd32a937581df6b5a4f08e6ef40c37a2b4278936) )

	ROM_LOAD16_BYTE( "silq-1.bin",  0x0c0000, 0x020000,  CRC(03332145) SHA1(1700f76622a4b195e3ce03d89b886060d7f3ba71) ) // ISSUE3 Q1 v3.1 ('N' questions) (normal)
	ROM_LOAD16_BYTE( "silq-2.bin",  0x0c0001, 0x020000,  CRC(9659b0cb) SHA1(c573752ef40c8501907d0e41ed2d2566a2dddcb8) ) // ISSUE3 Q2 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-3.bin",  0x100000, 0x020000,  CRC(443a6c2d) SHA1(1c111d801d76b07cdfef2b465f7ac759331f1843) ) // ISSUE3 Q3 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-4.bin",  0x100001, 0x020000,  CRC(6e84bd19) SHA1(318649fdd3cfec6f7ab0fa6d183a3b94f25a58f5) ) // ISSUE3 Q4 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-5.bin",  0x140000, 0x020000,  CRC(444c6d8e) SHA1(ae662747382b12bee77c30620fb0705312084e42) ) // ISSUE3 Q5 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-7.bin",  0x180000, 0x020000,  CRC(122f2327) SHA1(5c83f473cbfb7624f6eedd6d6521020b2b838da4) ) // ISSUE3 Q1 v3.1 ('T' questions) (true)
	ROM_LOAD16_BYTE( "silq-6.bin",  0x1c0000, 0x020000,  CRC(0ea36fd5) SHA1(e0649c77007c092fef4cb11fdd71682c88ca82e6) ) // ISSUE3 Q1 v3.1 ('F' questions) (false)

	ROM_REGION( 0x1000, "characteriser:fakechr", 0 )
	// this is a state result dump of the PAL, a 64x64 table of 6-bit values, where a write sets the column index
	// and the result of the previous read sets the row index, and a write of 00 resets the state machine?
	ROM_LOAD( "strikeitlucky_video_mpu4.chr", 0x0000, 0x1000, CRC(ec529c9f) SHA1(9eb2b08afb2955b0a8fe736500888b63f07ace63) )
ROM_END

ROM_START( v4striked )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "sild.p1", 0x000000, 0x020000,  CRC(6886e7c8) SHA1(9ef0895c9a8df762d17a3ea036bf0c6c2a440582) )
	ROM_LOAD16_BYTE( "sil.p2",  0x000001, 0x020000,  CRC(1afc07b7) SHA1(38777d56192b640b003d8dbf4b793cee0c81d9b2) )
	ROM_LOAD16_BYTE( "sil.p3",  0x040000, 0x020000,  CRC(40f5851c) SHA1(0e3dc0dd2a257a955a1f250556d047481ae87269) )
	ROM_LOAD16_BYTE( "sil.p4",  0x040001, 0x020000,  CRC(657e297e) SHA1(306f40376115ca40099a0010650b5edc183a2c57) )
	ROM_LOAD16_BYTE( "sil.p5",  0x080000, 0x020000,  CRC(28bced09) SHA1(7ba5013f1e0f4e921581b23c4a1d4c005a043b66) )
	ROM_LOAD16_BYTE( "sil.p6",  0x080001, 0x020000,  CRC(6f5fc296) SHA1(bd32a937581df6b5a4f08e6ef40c37a2b4278936) )

	ROM_LOAD16_BYTE( "silq-1.bin",  0x0c0000, 0x020000,  CRC(03332145) SHA1(1700f76622a4b195e3ce03d89b886060d7f3ba71) ) // ISSUE3 Q1 v3.1 ('N' questions) (normal)
	ROM_LOAD16_BYTE( "silq-2.bin",  0x0c0001, 0x020000,  CRC(9659b0cb) SHA1(c573752ef40c8501907d0e41ed2d2566a2dddcb8) ) // ISSUE3 Q2 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-3.bin",  0x100000, 0x020000,  CRC(443a6c2d) SHA1(1c111d801d76b07cdfef2b465f7ac759331f1843) ) // ISSUE3 Q3 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-4.bin",  0x100001, 0x020000,  CRC(6e84bd19) SHA1(318649fdd3cfec6f7ab0fa6d183a3b94f25a58f5) ) // ISSUE3 Q4 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-5.bin",  0x140000, 0x020000,  CRC(444c6d8e) SHA1(ae662747382b12bee77c30620fb0705312084e42) ) // ISSUE3 Q5 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-7.bin",  0x180000, 0x020000,  CRC(122f2327) SHA1(5c83f473cbfb7624f6eedd6d6521020b2b838da4) ) // ISSUE3 Q1 v3.1 ('T' questions) (true)
	ROM_LOAD16_BYTE( "silq-6.bin",  0x1c0000, 0x020000,  CRC(0ea36fd5) SHA1(e0649c77007c092fef4cb11fdd71682c88ca82e6) ) // ISSUE3 Q1 v3.1 ('F' questions) (false)

	ROM_REGION( 0x1000, "characteriser:fakechr", 0 )
	// this is a state result dump of the PAL, a 64x64 table of 6-bit values, where a write sets the column index
	// and the result of the previous read sets the row index, and a write of 00 resets the state machine?
	ROM_LOAD( "strikeitlucky_video_mpu4.chr", 0x0000, 0x1000, CRC(ec529c9f) SHA1(9eb2b08afb2955b0a8fe736500888b63f07ace63) )
ROM_END

ROM_START( v4strike2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "sil3.p1", 0x000000, 0x020000,  CRC(51c22f94) SHA1(dbdd30e67df4662d13b32a1df6cfabef2cf9471f) )
	ROM_LOAD16_BYTE( "sil.p2",  0x000001, 0x020000,  CRC(1afc07b7) SHA1(38777d56192b640b003d8dbf4b793cee0c81d9b2) )
	ROM_LOAD16_BYTE( "sil.p3",  0x040000, 0x020000,  CRC(40f5851c) SHA1(0e3dc0dd2a257a955a1f250556d047481ae87269) )
	ROM_LOAD16_BYTE( "sil.p4",  0x040001, 0x020000,  CRC(657e297e) SHA1(306f40376115ca40099a0010650b5edc183a2c57) )
	ROM_LOAD16_BYTE( "sil.p5",  0x080000, 0x020000,  CRC(28bced09) SHA1(7ba5013f1e0f4e921581b23c4a1d4c005a043b66) )
	ROM_LOAD16_BYTE( "sil.p6",  0x080001, 0x020000,  CRC(6f5fc296) SHA1(bd32a937581df6b5a4f08e6ef40c37a2b4278936) )

	ROM_LOAD16_BYTE( "silq-1.bin",  0x0c0000, 0x020000,  CRC(03332145) SHA1(1700f76622a4b195e3ce03d89b886060d7f3ba71) ) // ISSUE3 Q1 v3.1 ('N' questions) (normal)
	ROM_LOAD16_BYTE( "silq-2.bin",  0x0c0001, 0x020000,  CRC(9659b0cb) SHA1(c573752ef40c8501907d0e41ed2d2566a2dddcb8) ) // ISSUE3 Q2 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-3.bin",  0x100000, 0x020000,  CRC(443a6c2d) SHA1(1c111d801d76b07cdfef2b465f7ac759331f1843) ) // ISSUE3 Q3 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-4.bin",  0x100001, 0x020000,  CRC(6e84bd19) SHA1(318649fdd3cfec6f7ab0fa6d183a3b94f25a58f5) ) // ISSUE3 Q4 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-5.bin",  0x140000, 0x020000,  CRC(444c6d8e) SHA1(ae662747382b12bee77c30620fb0705312084e42) ) // ISSUE3 Q5 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-7.bin",  0x180000, 0x020000,  CRC(122f2327) SHA1(5c83f473cbfb7624f6eedd6d6521020b2b838da4) ) // ISSUE3 Q1 v3.1 ('T' questions) (true)
	ROM_LOAD16_BYTE( "silq-6.bin",  0x1c0000, 0x020000,  CRC(0ea36fd5) SHA1(e0649c77007c092fef4cb11fdd71682c88ca82e6) ) // ISSUE3 Q1 v3.1 ('F' questions) (false)

	ROM_REGION( 0x1000, "characteriser:fakechr", 0 )
	// this is a state result dump of the PAL, a 64x64 table of 6-bit values, where a write sets the column index
	// and the result of the previous read sets the row index, and a write of 00 resets the state machine?
	ROM_LOAD( "strikeitlucky_video_mpu4.chr", 0x0000, 0x1000, CRC(ec529c9f) SHA1(9eb2b08afb2955b0a8fe736500888b63f07ace63) )
ROM_END

ROM_START( v4strike2d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "sil3d.p1", 0x000000, 0x020000,  CRC(82def1b5) SHA1(d3e90e8e4e905a3779889305bb6441907d9169f1) )
	ROM_LOAD16_BYTE( "sil.p2",  0x000001, 0x020000,  CRC(1afc07b7) SHA1(38777d56192b640b003d8dbf4b793cee0c81d9b2) )
	ROM_LOAD16_BYTE( "sil.p3",  0x040000, 0x020000,  CRC(40f5851c) SHA1(0e3dc0dd2a257a955a1f250556d047481ae87269) )
	ROM_LOAD16_BYTE( "sil.p4",  0x040001, 0x020000,  CRC(657e297e) SHA1(306f40376115ca40099a0010650b5edc183a2c57) )
	ROM_LOAD16_BYTE( "sil.p5",  0x080000, 0x020000,  CRC(28bced09) SHA1(7ba5013f1e0f4e921581b23c4a1d4c005a043b66) )
	ROM_LOAD16_BYTE( "sil.p6",  0x080001, 0x020000,  CRC(6f5fc296) SHA1(bd32a937581df6b5a4f08e6ef40c37a2b4278936) )

	ROM_LOAD16_BYTE( "silq-1.bin",  0x0c0000, 0x020000,  CRC(03332145) SHA1(1700f76622a4b195e3ce03d89b886060d7f3ba71) ) // ISSUE3 Q1 v3.1 ('N' questions) (normal)
	ROM_LOAD16_BYTE( "silq-2.bin",  0x0c0001, 0x020000,  CRC(9659b0cb) SHA1(c573752ef40c8501907d0e41ed2d2566a2dddcb8) ) // ISSUE3 Q2 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-3.bin",  0x100000, 0x020000,  CRC(443a6c2d) SHA1(1c111d801d76b07cdfef2b465f7ac759331f1843) ) // ISSUE3 Q3 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-4.bin",  0x100001, 0x020000,  CRC(6e84bd19) SHA1(318649fdd3cfec6f7ab0fa6d183a3b94f25a58f5) ) // ISSUE3 Q4 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-5.bin",  0x140000, 0x020000,  CRC(444c6d8e) SHA1(ae662747382b12bee77c30620fb0705312084e42) ) // ISSUE3 Q5 v3.1 ('N' questions)
	ROM_LOAD16_BYTE( "silq-7.bin",  0x180000, 0x020000,  CRC(122f2327) SHA1(5c83f473cbfb7624f6eedd6d6521020b2b838da4) ) // ISSUE3 Q1 v3.1 ('T' questions) (true)
	ROM_LOAD16_BYTE( "silq-6.bin",  0x1c0000, 0x020000,  CRC(0ea36fd5) SHA1(e0649c77007c092fef4cb11fdd71682c88ca82e6) ) // ISSUE3 Q1 v3.1 ('F' questions) (false)

	ROM_REGION( 0x1000, "characteriser:fakechr", 0 )
	// this is a state result dump of the PAL, a 64x64 table of 6-bit values, where a write sets the column index
	// and the result of the previous read sets the row index, and a write of 00 resets the state machine?
	ROM_LOAD( "strikeitlucky_video_mpu4.chr", 0x0000, 0x1000, CRC(ec529c9f) SHA1(9eb2b08afb2955b0a8fe736500888b63f07ace63) )
ROM_END

ROM_START( v4eyedwn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "eds.p1", 0x000000, 0x010000, CRC(bd4f4c5d) SHA1(4ad34d92d45c0b8c70af8454525dd669cab94c5c) )
	ROM_LOAD16_BYTE( "ed.p2",  0x000001, 0x010000, CRC(8804b926) SHA1(cf9d49ca090a435819f2cd2d32dec3a6767f9e10) )
	ROM_LOAD16_BYTE( "ed.p3",  0x020000, 0x010000, CRC(969d2264) SHA1(b55f6881852f81ec9fb2c57c8137c872f4714710) )
	ROM_LOAD16_BYTE( "ed.p4",  0x020001, 0x010000, CRC(80d9addd) SHA1(f6359354c928e69a90cdf6d4f514c4992d3fa64c) )

	ROM_LOAD( "eyesdown_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4eyedwnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "edd.p1", 0x000000, 0x010000, CRC(369a145c) SHA1(c54305690798086b4f809f38d1a6454d1d34fd0c) )
	ROM_LOAD16_BYTE( "ed.p2",  0x000001, 0x010000, CRC(8804b926) SHA1(cf9d49ca090a435819f2cd2d32dec3a6767f9e10) )
	ROM_LOAD16_BYTE( "ed.p3",  0x020000, 0x010000, CRC(969d2264) SHA1(b55f6881852f81ec9fb2c57c8137c872f4714710) )
	ROM_LOAD16_BYTE( "ed.p4",  0x020001, 0x010000, CRC(80d9addd) SHA1(f6359354c928e69a90cdf6d4f514c4992d3fa64c) )

	ROM_LOAD( "eyesdown_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4quidgr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "qgs.p1", 0x000000, 0x010000, CRC(e6d072a9) SHA1(b2d656818c2fb12a26ef8a170152087336c00c66) )
	ROM_LOAD16_BYTE( "qg.p2",  0x000001, 0x010000, CRC(e4d74b11) SHA1(d23ccc57c54823ffaa7869ca824e049dc80f8945) )
	ROM_LOAD16_BYTE( "qg.p3",  0x020000, 0x010000, CRC(5dda7cd8) SHA1(246a801e862990aade98fa358477e53707714b42) )
	ROM_LOAD16_BYTE( "qg.p4",  0x020001, 0x010000, CRC(2106cf5d) SHA1(2073589775139ad92daef05a67afb2c70ece168c) )

	ROM_LOAD( "quidgrid_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4quidgrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "qgd.p1", 0x000000, 0x010000, CRC(c8d37bdf) SHA1(e39a41015dd551fd62efa52faf2c5106bf7d23ac) )
	ROM_LOAD16_BYTE( "qg.p2",  0x000001, 0x010000, CRC(e4d74b11) SHA1(d23ccc57c54823ffaa7869ca824e049dc80f8945) )
	ROM_LOAD16_BYTE( "qg.p3",  0x020000, 0x010000, CRC(5dda7cd8) SHA1(246a801e862990aade98fa358477e53707714b42) )
	ROM_LOAD16_BYTE( "qg.p4",  0x020001, 0x010000, CRC(2106cf5d) SHA1(2073589775139ad92daef05a67afb2c70ece168c) )

	ROM_LOAD( "quidgrid_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4quidgr2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "q2gs.p1", 0x000000, 0x010000, CRC(6d4b92c0) SHA1(05777e0ecc8bf9f3564d8395bb7bfa245498d132) )
	ROM_LOAD16_BYTE( "q2g.p2",  0x000001, 0x010000, CRC(9bf74ae3) SHA1(11e75acad496a9304f612714afbef90e48d1b9cb) )
	ROM_LOAD16_BYTE( "q2g.p3",  0x020000, 0x010000, CRC(78555155) SHA1(e1218fc00f08c19a0cafb203e46044efa617ac16) )
	ROM_LOAD16_BYTE( "q2g.p4",  0x020001, 0x010000, CRC(2a4295b6) SHA1(2aa0b5dbe6b934a7a4c8069c91fd6d85cae02836) )

	ROM_LOAD( "quidgrid_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4quidgr2d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "q2gd.p1", 0x000000, 0x010000, CRC(26185449) SHA1(921d355020788d53e8ffa83af44223dbe5a2dd5e) )
	ROM_LOAD16_BYTE( "q2g.p2",  0x000001, 0x010000, CRC(9bf74ae3) SHA1(11e75acad496a9304f612714afbef90e48d1b9cb) )
	ROM_LOAD16_BYTE( "q2g.p3",  0x020000, 0x010000, CRC(78555155) SHA1(e1218fc00f08c19a0cafb203e46044efa617ac16) )
	ROM_LOAD16_BYTE( "q2g.p4",  0x020001, 0x010000, CRC(2a4295b6) SHA1(2aa0b5dbe6b934a7a4c8069c91fd6d85cae02836) )

	ROM_LOAD( "quidgrid_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END


ROM_START( v4barqst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "bq-6.p1", 0x000000, 0x010000, CRC(f5f81993) SHA1(5975da2b944a8e9cb4bb285d87d64740d9ec045b) )
	ROM_LOAD16_BYTE( "bq-6.p2", 0x000001, 0x010000, CRC(b9ce9f2e) SHA1(9407a83d1713b641dc551dd73f357d99baebbba2) )
	ROM_LOAD16_BYTE( "bq-iss4.p1", 0x020000, 0x010000, CRC(f8c9e00b) SHA1(b0081756667068eaaf02f34a161b52024e163018) ) //ISSUE4
	ROM_LOAD16_BYTE( "bq-iss4.p2", 0x020001, 0x010000, CRC(211d7779) SHA1(c1d835dd4758f28b7bd4e81ea5d3316704fe8ff3) )
	ROM_LOAD16_BYTE( "bq-iss4.p3", 0x040000, 0x010000, CRC(b6066dec) SHA1(63f66f004f6c66dc1bd61385407e1adb4a678ccd) )
	ROM_LOAD16_BYTE( "bq-iss4.p4", 0x040001, 0x010000, CRC(71a2a3cf) SHA1(41d95d34b801f54bf4f1044d7bb88704c9a318a0) )
	ROM_LOAD16_BYTE( "bq-iss4.p5", 0x060000, 0x010000, CRC(13cfb410) SHA1(fca72e403a8c150aba73fd501a1a7cfd62bd28ac) )
	ROM_LOAD16_BYTE( "bq-iss4.p6", 0x060001, 0x010000, CRC(f8847e9d) SHA1(49213302f63efc6e2b6728fed98c0094d023c100) )
	ROM_LOAD16_BYTE( "bq-iss4.p7", 0x080000, 0x010000, CRC(1f3b3d33) SHA1(117f7121123336e3b33a3d1838ddca9161a7868b) )
	ROM_LOAD16_BYTE( "bq-iss4.p8", 0x080001, 0x010000, CRC(54007030) SHA1(65306938b2737f7423223eb0290ee69b2955176e) )
	ROM_LOAD16_BYTE( "bq-iss4.p9", 0x0a0000, 0x010000, CRC(2971f5ca) SHA1(0de9b1d743243d6e127f5417485f1a9fa76d5399) )
	ROM_LOAD16_BYTE( "bq-iss4.p10", 0x0a0001, 0x010000, CRC(d54f961f) SHA1(14273cf78371550dd525843b388915df567342ce) )

	ROM_REGION( 0x1000, "characteriser:fakechr", 0 )
	// this is a state result dump of the PAL, a 64x64 table of 6-bit values, where a write sets the column index
	// and the result of the previous read sets the row index, and a write of 00 resets the state machine?
	ROM_LOAD( "barquest.chr", 0x0000, 0x1000, CRC(bc9971fe) SHA1(902684318ad0755ee062a7f10e2c3171b5c4933f) )
ROM_END

ROM_START( v4barqstd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "bqd.p1", 0x000000, 0x010000, CRC(b574ea46) SHA1(0eb446fbf4f7fcd1b30f35631b4b521730ce26b4) )
	ROM_LOAD16_BYTE( "bq.p2",  0x000001, 0x010000, CRC(b9ce9f2e) SHA1(9407a83d1713b641dc551dd73f357d99baebbba2) )
	ROM_LOAD16_BYTE( "bq-iss4.p1", 0x020000, 0x010000, CRC(f8c9e00b) SHA1(b0081756667068eaaf02f34a161b52024e163018) ) //ISSUE4
	ROM_LOAD16_BYTE( "bq-iss4.p2", 0x020001, 0x010000, CRC(211d7779) SHA1(c1d835dd4758f28b7bd4e81ea5d3316704fe8ff3) )
	ROM_LOAD16_BYTE( "bq-iss4.p3", 0x040000, 0x010000, CRC(b6066dec) SHA1(63f66f004f6c66dc1bd61385407e1adb4a678ccd) )
	ROM_LOAD16_BYTE( "bq-iss4.p4", 0x040001, 0x010000, CRC(71a2a3cf) SHA1(41d95d34b801f54bf4f1044d7bb88704c9a318a0) )
	ROM_LOAD16_BYTE( "bq-iss4.p5", 0x060000, 0x010000, CRC(13cfb410) SHA1(fca72e403a8c150aba73fd501a1a7cfd62bd28ac) )
	ROM_LOAD16_BYTE( "bq-iss4.p6", 0x060001, 0x010000, CRC(f8847e9d) SHA1(49213302f63efc6e2b6728fed98c0094d023c100) )
	ROM_LOAD16_BYTE( "bq-iss4.p7", 0x080000, 0x010000, CRC(1f3b3d33) SHA1(117f7121123336e3b33a3d1838ddca9161a7868b) )
	ROM_LOAD16_BYTE( "bq-iss4.p8", 0x080001, 0x010000, CRC(54007030) SHA1(65306938b2737f7423223eb0290ee69b2955176e) )
	ROM_LOAD16_BYTE( "bq-iss4.p9", 0x0a0000, 0x010000, CRC(2971f5ca) SHA1(0de9b1d743243d6e127f5417485f1a9fa76d5399) )
	ROM_LOAD16_BYTE( "bq-iss4.p10", 0x0a0001, 0x010000, CRC(d54f961f) SHA1(14273cf78371550dd525843b388915df567342ce) )

	ROM_REGION( 0x1000, "characteriser:fakechr", 0 )
	// this is a state result dump of the PAL, a 64x64 table of 6-bit values, where a write sets the column index
	// and the result of the previous read sets the row index, and a write of 00 resets the state machine?
	ROM_LOAD( "barquest.chr", 0x0000, 0x1000, CRC(bc9971fe) SHA1(902684318ad0755ee062a7f10e2c3171b5c4933f) )
ROM_END

ROM_START( v4barqs2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b2qs.p1", 0x000000, 0x010000, CRC(35c1a58b) SHA1(e2d07229e7136c462c48b6c77c9d26a33deb8e34) )
	ROM_LOAD16_BYTE( "b2q.p2", 0x000001, 0x010000, CRC(28a0064a) SHA1(3751d9f3cc93994c44927ca3f72ade6bee22b20b) )

	ROM_LOAD( "barquest2_questions",  0x040000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4wize )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "wms.p1", 0x000000, 0x010000, CRC(712385c1) SHA1(075a98626eba2eae6a31b395c2a74541a31b2582) )
	ROM_LOAD16_BYTE( "wm.p2",  0x000001, 0x010000, CRC(41b5fb2a) SHA1(e9ee484ec7445d58efa9bbfbd705202ef83656f2) )
	ROM_LOAD16_BYTE( "wm.p3",  0x020000, 0x010000, CRC(934da7e4) SHA1(9cac87ccadbc871577640ec0bddd5e07aef139f8) )
	ROM_LOAD16_BYTE( "wm.p4",  0x020001, 0x010000, CRC(463f6c0b) SHA1(ffee4cca73ebe7130e34118031cb16b3c42f03cb) )
	ROM_LOAD16_BYTE( "wm.p5",  0x040000, 0x010000, CRC(eaea2502) SHA1(adeda7148ee4eee98870f4aa529b5c9f36417e2e) )
	ROM_LOAD16_BYTE( "wm.p6",  0x040001, 0x010000, CRC(40a5e980) SHA1(e7bd49308b63a94a9ca0b138de0c48d2316d6aa0) )

	ROM_LOAD( "wizemove_questions",  0x080000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4wized )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "wmd.p1", 0x000000, 0x010000, CRC(45d29045) SHA1(37911346753211801f8404d42b275f49764ba5f4) )
	ROM_LOAD16_BYTE( "wm.p2",  0x000001, 0x010000, CRC(41b5fb2a) SHA1(e9ee484ec7445d58efa9bbfbd705202ef83656f2) )
	ROM_LOAD16_BYTE( "wm.p3",  0x020000, 0x010000, CRC(934da7e4) SHA1(9cac87ccadbc871577640ec0bddd5e07aef139f8) )
	ROM_LOAD16_BYTE( "wm.p4",  0x020001, 0x010000, CRC(463f6c0b) SHA1(ffee4cca73ebe7130e34118031cb16b3c42f03cb) )
	ROM_LOAD16_BYTE( "wm.p5",  0x040000, 0x010000, CRC(eaea2502) SHA1(adeda7148ee4eee98870f4aa529b5c9f36417e2e) )
	ROM_LOAD16_BYTE( "wm.p6",  0x040001, 0x010000, CRC(40a5e980) SHA1(e7bd49308b63a94a9ca0b138de0c48d2316d6aa0) )

	ROM_LOAD( "wizemove_questions",  0x080000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END


ROM_START( v4wizeb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "wm3.p1", 0x000000, 0x010000, CRC(0752f0f1) SHA1(2b0531312bf1d4b489394401c5d78c7f04e12aea) )
	ROM_LOAD16_BYTE( "wm.p2",  0x000001, 0x010000, CRC(41b5fb2a) SHA1(e9ee484ec7445d58efa9bbfbd705202ef83656f2) )
	ROM_LOAD16_BYTE( "wm.p3",  0x020000, 0x010000, CRC(934da7e4) SHA1(9cac87ccadbc871577640ec0bddd5e07aef139f8) )
	ROM_LOAD16_BYTE( "wm.p4",  0x020001, 0x010000, CRC(463f6c0b) SHA1(ffee4cca73ebe7130e34118031cb16b3c42f03cb) )
	ROM_LOAD16_BYTE( "wm.p5",  0x040000, 0x010000, CRC(eaea2502) SHA1(adeda7148ee4eee98870f4aa529b5c9f36417e2e) )
	ROM_LOAD16_BYTE( "wm.p6",  0x040001, 0x010000, CRC(40a5e980) SHA1(e7bd49308b63a94a9ca0b138de0c48d2316d6aa0) )

	ROM_LOAD( "wizemove_questions",  0x080000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4wizec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "wm3d.p1", 0x000000, 0x010000, CRC(4e3ab877) SHA1(64408d1ac1f626390ffe93e024c672ba5acb42d6) )
	ROM_LOAD16_BYTE( "wm.p2",  0x000001, 0x010000, CRC(41b5fb2a) SHA1(e9ee484ec7445d58efa9bbfbd705202ef83656f2) )
	ROM_LOAD16_BYTE( "wm.p3",  0x020000, 0x010000, CRC(934da7e4) SHA1(9cac87ccadbc871577640ec0bddd5e07aef139f8) )
	ROM_LOAD16_BYTE( "wm.p4",  0x020001, 0x010000, CRC(463f6c0b) SHA1(ffee4cca73ebe7130e34118031cb16b3c42f03cb) )
	ROM_LOAD16_BYTE( "wm.p5",  0x040000, 0x010000, CRC(eaea2502) SHA1(adeda7148ee4eee98870f4aa529b5c9f36417e2e) )
	ROM_LOAD16_BYTE( "wm.p6",  0x040001, 0x010000, CRC(40a5e980) SHA1(e7bd49308b63a94a9ca0b138de0c48d2316d6aa0) )

	ROM_LOAD( "wizemove_questions",  0x080000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4wizen )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 ) // has WW21 in ROM, vector table appears corrupt, seems to be different revision
	ROM_LOAD16_BYTE( "w23.p1", 0x000000, 0x010000, BAD_DUMP CRC(a8d8fb2e) SHA1(cf5462b224a7960ade867cf76079d11084f13e4b) )
	ROM_LOAD16_BYTE( "w23.p2", 0x000001, 0x010000, NO_DUMP )
	// + other ROMs

	ROM_LOAD( "wizemove_questions",  0x080000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END

ROM_START( v4wizeo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "wzm1+2p1", 0x000000, 0x010000, CRC(629e38d0) SHA1(bbc2688635c4cb9a1712a0a0d28017d1867417a4) )
	ROM_LOAD16_BYTE( "wzm1+2p2", 0x000001, 0x010000, CRC(96b9c484) SHA1(b02595ba5a5c7d673bdc7675708a4d8f6d907779) )
	ROM_LOAD16_BYTE( "wzm1+2p3", 0x020000, 0x010000, CRC(e545fac0) SHA1(4a3f9e5522bd666d5a1e9ac7878d3d78f3756762) )
	ROM_LOAD16_BYTE( "wzm1+2p4", 0x020001, 0x010000, CRC(df7d4dba) SHA1(b892cfb807421c99ba98bfd9b34d717e17345d83) )
	ROM_LOAD16_BYTE( "wzm1+2p5", 0x040000, 0x010000, NO_DUMP )
	ROM_LOAD16_BYTE( "wzm1+2p6", 0x040001, 0x010000, CRC(3eecbdf8) SHA1(9ecc4fe25e1c1e167aaa413eaf601b55e1a432fb) )

	ROM_LOAD( "wizemove_questions",  0x080000, 0x020000,  NO_DUMP ) // no dumps of question ROMs for this game..
ROM_END




ROM_START( v4opt3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "o3-s.p1",0x000000, 0x020000, CRC(9b825fa1) SHA1(c743bed32fa1af934f9610204a4f4062cfb85193) )
	ROM_LOAD16_BYTE( "o3-.p2", 0x000001, 0x020000, CRC(9545a845) SHA1(24c6656c137baf74c8ea5d930220bf0d0c90b3ab) )
	ROM_LOAD16_BYTE( "o3-.p3", 0x040000, 0x020000, CRC(2f3666cb) SHA1(94010eb56de7f3900fbe1c0b77a43752f9cf3ba6) )
	ROM_LOAD16_BYTE( "o3-.p4", 0x040001, 0x020000, CRC(63b6aa70) SHA1(6f0bef67437caa150ee98f0349d0f6412665e36b) )
	ROM_LOAD16_BYTE( "o3-.p5", 0x080000, 0x020000, CRC(750db328) SHA1(8442b32e2f70ba2e0849d994a1e2502634652c5e) )
	ROM_LOAD16_BYTE( "o3-.p6", 0x080001, 0x020000, CRC(1fed469e) SHA1(351a8cd687c0e11300c498e45e4b6f3a35940615) )
	ROM_LOAD16_BYTE( "o3-.p7", 0x0c0000, 0x020000, CRC(400e304f) SHA1(3abe30cf75990a5ce9066ccd4cfed680e5d4963c) )
	ROM_LOAD16_BYTE( "o3-.p8", 0x0c0001, 0x020000, CRC(0fb22311) SHA1(2329520d4ce763ba10cc81314711d28b4a5fb192) )
	ROM_LOAD16_BYTE( "o3-.p9", 0x100000, 0x020000, CRC(0d71b620) SHA1(103cf5700c876e94e2ca3b63ef71b768bd2d99a2) )
	ROM_LOAD16_BYTE( "o3-.p10",0x100001, 0x020000, CRC(239fa03b) SHA1(c4c77ded07481746b041122a8538036a65a5abe6) )
ROM_END

ROM_START( v4opt3d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	VID_BIOS

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "o3-d.p1",0x000000, 0x020000, CRC(09eef902) SHA1(2ce68fe08c5a1be7daaf3a380aa5f5fef8401944) )
	ROM_LOAD16_BYTE( "o3-.p2", 0x000001, 0x020000, CRC(9545a845) SHA1(24c6656c137baf74c8ea5d930220bf0d0c90b3ab) )
	ROM_LOAD16_BYTE( "o3-.p3", 0x040000, 0x020000, CRC(2f3666cb) SHA1(94010eb56de7f3900fbe1c0b77a43752f9cf3ba6) )
	ROM_LOAD16_BYTE( "o3-.p4", 0x040001, 0x020000, CRC(63b6aa70) SHA1(6f0bef67437caa150ee98f0349d0f6412665e36b) )
	ROM_LOAD16_BYTE( "o3-.p5", 0x080000, 0x020000, CRC(750db328) SHA1(8442b32e2f70ba2e0849d994a1e2502634652c5e) )
	ROM_LOAD16_BYTE( "o3-.p6", 0x080001, 0x020000, CRC(1fed469e) SHA1(351a8cd687c0e11300c498e45e4b6f3a35940615) )
	ROM_LOAD16_BYTE( "o3-.p7", 0x0c0000, 0x020000, CRC(400e304f) SHA1(3abe30cf75990a5ce9066ccd4cfed680e5d4963c) )
	ROM_LOAD16_BYTE( "o3-.p8", 0x0c0001, 0x020000, CRC(0fb22311) SHA1(2329520d4ce763ba10cc81314711d28b4a5fb192) )
	ROM_LOAD16_BYTE( "o3-.p9", 0x100000, 0x020000, CRC(0d71b620) SHA1(103cf5700c876e94e2ca3b63ef71b768bd2d99a2) )
	ROM_LOAD16_BYTE( "o3-.p10",0x100001, 0x020000, CRC(239fa03b) SHA1(c4c77ded07481746b041122a8538036a65a5abe6) )
ROM_END


/*

FRUIT FACTORY                   ID=FF_
-------------

GRAPHIC PROMS
=============

FF_GFX10.P3     4mb
FF_GFX10.P4     4MB
FF_GFX10.P5     4MB
FF_GFX10.P6     4MB
FF_GFX10.P7     4MB
FF_GFX10.P8     4MB

MPU4 PROM
=========

FFMPU416.P1 A209    27C512


GAME PROMS
==========
FF_293.P1   5F95    27C040  20p/?15
FF_293.P2   DCE6    27C040  20p/?15
FF_293D.P2  5F95    27C040  20p/?15 data
FF_293D.P2  DDE5    27C040  20p/?15 data

FF_393.P1   5F9F    27C040  25p/?15
FF_393.P2   DCE6    27C040  25p/?15
FF_393D.P2  5F9F    27C040  25p/?15 data
FF_393D.P2  DDE5    27C040  25p/?15 data

FF_493.P1   5EAA    27C040  30p/?15
FF_493.P2   DCE6    27C040  30p/?15
FF_493D.P2  5EAA    27C040  30p/?15 data
FF_493D.P2  DDE5    27C040  30p/?15 data

---------------------------------------------------------------------
 END

 */

ROM_START( v4frfact )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ffmpu416_a209.p1",  0x00000, 0x10000, NO_DUMP ) // should use "FFMPU416.P1  A209    27C512" according to text file, but ROM was not in archive

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ff_40d.p1", 0x000001, 0x080000, CRC(c91fd349) SHA1(18c36612ee9fed6f12bb3dbf08f6740cf2a463c2) )
	ROM_LOAD16_BYTE( "ff_40d.p2", 0x000000, 0x080000, CRC(e7555936) SHA1(0af7a0be17735831eb037c4bc55d35891a608b23) )
	ROM_LOAD16_BYTE( "ff_gfx10.p3", 0x100001, 0x080000, CRC(a043a1f6) SHA1(0e591f0e7ecdf8b390a20ee826705a22ed6923d5) )
	ROM_LOAD16_BYTE( "ff_gfx10.p4", 0x100000, 0x080000, CRC(58226ff5) SHA1(f2647e43da69e8aa2f78d46f3cfc553440213c36) )
	ROM_LOAD16_BYTE( "ff_gfx10.p5", 0x200001, 0x080000, CRC(9bfc6da8) SHA1(969c4a84392c28d61e87482e7f881bdfda79f879) )
	ROM_LOAD16_BYTE( "ff_gfx10.p6", 0x200000, 0x080000, CRC(240cdfd3) SHA1(8ccf199aff929813df554d957e84484a482c98c6) )
	ROM_LOAD16_BYTE( "ff_gfx10.p7", 0x300001, 0x080000, CRC(42d3bd01) SHA1(6d07875e8f251c3c9c4e7f48ae886b8069c20897) )
	ROM_LOAD16_BYTE( "ff_gfx10.p8", 0x300000, 0x080000, CRC(1951a944) SHA1(b8eca580ae43be855d93cf9f50058b2fb9e8981b) )
ROM_END

ROM_START( v4frfacta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ffmpu416_a209.p1",  0x00000, 0x10000, NO_DUMP ) // should use "FFMPU416.P1  A209    27C512" according to text file, but ROM was not in archive

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ff_493.p1", 0x000001, 0x080000, CRC(1a88e0e6) SHA1(198fffd0d98d557462485b1ca6e3460199e03924) ) // 30p/?15
	ROM_LOAD16_BYTE( "ff_493.p2", 0x000000, 0x080000, CRC(1c948e83) SHA1(571f854c33a2ef68daa8633193b49486ff92d7e2) )
	ROM_LOAD16_BYTE( "ff_gfx10.p3", 0x100001, 0x080000, CRC(a043a1f6) SHA1(0e591f0e7ecdf8b390a20ee826705a22ed6923d5) )
	ROM_LOAD16_BYTE( "ff_gfx10.p4", 0x100000, 0x080000, CRC(58226ff5) SHA1(f2647e43da69e8aa2f78d46f3cfc553440213c36) )
	ROM_LOAD16_BYTE( "ff_gfx10.p5", 0x200001, 0x080000, CRC(9bfc6da8) SHA1(969c4a84392c28d61e87482e7f881bdfda79f879) )
	ROM_LOAD16_BYTE( "ff_gfx10.p6", 0x200000, 0x080000, CRC(240cdfd3) SHA1(8ccf199aff929813df554d957e84484a482c98c6) )
	ROM_LOAD16_BYTE( "ff_gfx10.p7", 0x300001, 0x080000, CRC(42d3bd01) SHA1(6d07875e8f251c3c9c4e7f48ae886b8069c20897) )
	ROM_LOAD16_BYTE( "ff_gfx10.p8", 0x300000, 0x080000, CRC(1951a944) SHA1(b8eca580ae43be855d93cf9f50058b2fb9e8981b) )
ROM_END

ROM_START( v4frfactb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ffmpu416_a209.p1",  0x00000, 0x10000, NO_DUMP ) // should use "FFMPU416.P1  A209    27C512" according to text file, but ROM was not in archive

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ff_493d.p1", 0x000001, 0x080000, CRC(1a88e0e6) SHA1(198fffd0d98d557462485b1ca6e3460199e03924) )   // 30p/?15 data
	ROM_LOAD16_BYTE( "ff_493d.p2", 0x000000, 0x080000, CRC(86830e6c) SHA1(9510bf42a8c2f8ffeafd1f2b0e027a0a59d80b20) )
	ROM_LOAD16_BYTE( "ff_gfx10.p3", 0x100001, 0x080000, CRC(a043a1f6) SHA1(0e591f0e7ecdf8b390a20ee826705a22ed6923d5) )
	ROM_LOAD16_BYTE( "ff_gfx10.p4", 0x100000, 0x080000, CRC(58226ff5) SHA1(f2647e43da69e8aa2f78d46f3cfc553440213c36) )
	ROM_LOAD16_BYTE( "ff_gfx10.p5", 0x200001, 0x080000, CRC(9bfc6da8) SHA1(969c4a84392c28d61e87482e7f881bdfda79f879) )
	ROM_LOAD16_BYTE( "ff_gfx10.p6", 0x200000, 0x080000, CRC(240cdfd3) SHA1(8ccf199aff929813df554d957e84484a482c98c6) )
	ROM_LOAD16_BYTE( "ff_gfx10.p7", 0x300001, 0x080000, CRC(42d3bd01) SHA1(6d07875e8f251c3c9c4e7f48ae886b8069c20897) )
	ROM_LOAD16_BYTE( "ff_gfx10.p8", 0x300000, 0x080000, CRC(1951a944) SHA1(b8eca580ae43be855d93cf9f50058b2fb9e8981b) )
ROM_END

ROM_START( v4frfactc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ffmpu416_a209.p1",  0x00000, 0x10000, NO_DUMP ) // should use "FFMPU416.P1  A209    27C512" according to text file, but ROM was not in archive

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ff_393.p1", 0x000001, 0x080000, CRC(35011cab) SHA1(9e381db93dbe1f71d40b152ae6c68ea7a7b9728c) )    // 25p/?15
	ROM_LOAD16_BYTE( "ff_393.p2", 0x000000, 0x080000, CRC(1c948e83) SHA1(571f854c33a2ef68daa8633193b49486ff92d7e2) )
	ROM_LOAD16_BYTE( "ff_gfx10.p3", 0x100001, 0x080000, CRC(a043a1f6) SHA1(0e591f0e7ecdf8b390a20ee826705a22ed6923d5) )
	ROM_LOAD16_BYTE( "ff_gfx10.p4", 0x100000, 0x080000, CRC(58226ff5) SHA1(f2647e43da69e8aa2f78d46f3cfc553440213c36) )
	ROM_LOAD16_BYTE( "ff_gfx10.p5", 0x200001, 0x080000, CRC(9bfc6da8) SHA1(969c4a84392c28d61e87482e7f881bdfda79f879) )
	ROM_LOAD16_BYTE( "ff_gfx10.p6", 0x200000, 0x080000, CRC(240cdfd3) SHA1(8ccf199aff929813df554d957e84484a482c98c6) )
	ROM_LOAD16_BYTE( "ff_gfx10.p7", 0x300001, 0x080000, CRC(42d3bd01) SHA1(6d07875e8f251c3c9c4e7f48ae886b8069c20897) )
	ROM_LOAD16_BYTE( "ff_gfx10.p8", 0x300000, 0x080000, CRC(1951a944) SHA1(b8eca580ae43be855d93cf9f50058b2fb9e8981b) )
ROM_END

ROM_START( v4frfactd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ffmpu416_a209.p1",  0x00000, 0x10000, NO_DUMP ) // should use "FFMPU416.P1  A209    27C512" according to text file, but ROM was not in archive

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ff_393d.p1", 0x000001, 0x080000, CRC(35011cab) SHA1(9e381db93dbe1f71d40b152ae6c68ea7a7b9728c) )   // 25p/?15 data
	ROM_LOAD16_BYTE( "ff_393d.p2", 0x000000, 0x080000, CRC(86830e6c) SHA1(9510bf42a8c2f8ffeafd1f2b0e027a0a59d80b20) )
	ROM_LOAD16_BYTE( "ff_gfx10.p3", 0x100001, 0x080000, CRC(a043a1f6) SHA1(0e591f0e7ecdf8b390a20ee826705a22ed6923d5) )
	ROM_LOAD16_BYTE( "ff_gfx10.p4", 0x100000, 0x080000, CRC(58226ff5) SHA1(f2647e43da69e8aa2f78d46f3cfc553440213c36) )
	ROM_LOAD16_BYTE( "ff_gfx10.p5", 0x200001, 0x080000, CRC(9bfc6da8) SHA1(969c4a84392c28d61e87482e7f881bdfda79f879) )
	ROM_LOAD16_BYTE( "ff_gfx10.p6", 0x200000, 0x080000, CRC(240cdfd3) SHA1(8ccf199aff929813df554d957e84484a482c98c6) )
	ROM_LOAD16_BYTE( "ff_gfx10.p7", 0x300001, 0x080000, CRC(42d3bd01) SHA1(6d07875e8f251c3c9c4e7f48ae886b8069c20897) )
	ROM_LOAD16_BYTE( "ff_gfx10.p8", 0x300000, 0x080000, CRC(1951a944) SHA1(b8eca580ae43be855d93cf9f50058b2fb9e8981b) )
ROM_END

ROM_START( v4frfacte )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ffmpu416_a209.p1",  0x00000, 0x10000, NO_DUMP ) // should use "FFMPU416.P1  A209    27C512" according to text file, but ROM was not in archive

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ff_293.p1", 0x000001, 0x080000, CRC(4787bc3d) SHA1(a1d53f1640c6d829c9fee8c72057b2801aac4cb2) )    // 20p/?15
	ROM_LOAD16_BYTE( "ff_293.p2", 0x000000, 0x080000, CRC(1c948e83) SHA1(571f854c33a2ef68daa8633193b49486ff92d7e2) )
	ROM_LOAD16_BYTE( "ff_gfx10.p3", 0x100001, 0x080000, CRC(a043a1f6) SHA1(0e591f0e7ecdf8b390a20ee826705a22ed6923d5) )
	ROM_LOAD16_BYTE( "ff_gfx10.p4", 0x100000, 0x080000, CRC(58226ff5) SHA1(f2647e43da69e8aa2f78d46f3cfc553440213c36) )
	ROM_LOAD16_BYTE( "ff_gfx10.p5", 0x200001, 0x080000, CRC(9bfc6da8) SHA1(969c4a84392c28d61e87482e7f881bdfda79f879) )
	ROM_LOAD16_BYTE( "ff_gfx10.p6", 0x200000, 0x080000, CRC(240cdfd3) SHA1(8ccf199aff929813df554d957e84484a482c98c6) )
	ROM_LOAD16_BYTE( "ff_gfx10.p7", 0x300001, 0x080000, CRC(42d3bd01) SHA1(6d07875e8f251c3c9c4e7f48ae886b8069c20897) )
	ROM_LOAD16_BYTE( "ff_gfx10.p8", 0x300000, 0x080000, CRC(1951a944) SHA1(b8eca580ae43be855d93cf9f50058b2fb9e8981b) )
ROM_END

ROM_START( v4frfactf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ffmpu416_a209.p1",  0x00000, 0x10000, NO_DUMP ) // should use "FFMPU416.P1  A209    27C512" according to text file, but ROM was not in archive

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ff_293d.p1", 0x000001, 0x080000, CRC(4787bc3d) SHA1(a1d53f1640c6d829c9fee8c72057b2801aac4cb2) )   // 20p/?15 data
	ROM_LOAD16_BYTE( "ff_293d.p2", 0x000000, 0x080000, CRC(86830e6c) SHA1(9510bf42a8c2f8ffeafd1f2b0e027a0a59d80b20) )
	ROM_LOAD16_BYTE( "ff_gfx10.p3", 0x100001, 0x080000, CRC(a043a1f6) SHA1(0e591f0e7ecdf8b390a20ee826705a22ed6923d5) )
	ROM_LOAD16_BYTE( "ff_gfx10.p4", 0x100000, 0x080000, CRC(58226ff5) SHA1(f2647e43da69e8aa2f78d46f3cfc553440213c36) )
	ROM_LOAD16_BYTE( "ff_gfx10.p5", 0x200001, 0x080000, CRC(9bfc6da8) SHA1(969c4a84392c28d61e87482e7f881bdfda79f879) )
	ROM_LOAD16_BYTE( "ff_gfx10.p6", 0x200000, 0x080000, CRC(240cdfd3) SHA1(8ccf199aff929813df554d957e84484a482c98c6) )
	ROM_LOAD16_BYTE( "ff_gfx10.p7", 0x300001, 0x080000, CRC(42d3bd01) SHA1(6d07875e8f251c3c9c4e7f48ae886b8069c20897) )
	ROM_LOAD16_BYTE( "ff_gfx10.p8", 0x300000, 0x080000, CRC(1951a944) SHA1(b8eca580ae43be855d93cf9f50058b2fb9e8981b) )
ROM_END



/* BWB */
ROM_START( v4bigfrt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("bi_xca__.2_0",  0x00000, 0x10000,  CRC(bdba5ce9) SHA1(e4bce58957230183b96f9d3155575005ffb002c8))  // standard/stake Key

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("bi______.2_1",  0x000000, 0x010000,  CRC(22e7e736) SHA1(478abe8dc9e29b8ceccfac80ddaa5ce2727572cd))
	ROM_LOAD16_BYTE("bi______.2_2",  0x000001, 0x010000,  CRC(9d139ae7) SHA1(fb38b3102355ee5f79f66410ce5a26f74296321a))
	ROM_LOAD16_BYTE("bi______.2_3",  0x020000, 0x010000,  CRC(9f4c424d) SHA1(f20e5e6f43ac0d481b7f2d8ea421fa6b9fa92b38))
	ROM_LOAD16_BYTE("bi______.2_4",  0x020001, 0x010000,  CRC(58f0dcb4) SHA1(79039d489f5ce3a1865fa92a2b6e8b002b63efcf))
	ROM_LOAD16_BYTE("bi______.2_5",  0x040000, 0x010000,  CRC(512c6d1a) SHA1(1c9d04e7e59a95f6975e6d2d5e5def4c1a7777e0))
	ROM_LOAD16_BYTE("bi______.2_6",  0x040001, 0x010000,  CRC(5df850ec) SHA1(5b455f4cfb19c551723f7fd5f4f95e5420f8682f))
	ROM_LOAD16_BYTE("bi______.2_7",  0x060000, 0x010000,  CRC(9ea394a2) SHA1(3b9840627f7676aa7872d2bc022406a4ced7958f))
	ROM_LOAD16_BYTE("bi______.2_8",  0x060001, 0x010000,  CRC(4aa1d37d) SHA1(3c1a3ccacdc33cd4a54b7bbf06ea4c33868705f4))
ROM_END

ROM_START( v4bigfrta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("bi_20bt_.2_0",  0x00000, 0x10000,  CRC(0c5b21c8) SHA1(bec3a0ca87156de82024449ed5acfd9d8877f15c))  // protocol/20p/Rank Bingo

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("bi______.2_1",  0x000000, 0x010000,  CRC(22e7e736) SHA1(478abe8dc9e29b8ceccfac80ddaa5ce2727572cd))
	ROM_LOAD16_BYTE("bi______.2_2",  0x000001, 0x010000,  CRC(9d139ae7) SHA1(fb38b3102355ee5f79f66410ce5a26f74296321a))
	ROM_LOAD16_BYTE("bi______.2_3",  0x020000, 0x010000,  CRC(9f4c424d) SHA1(f20e5e6f43ac0d481b7f2d8ea421fa6b9fa92b38))
	ROM_LOAD16_BYTE("bi______.2_4",  0x020001, 0x010000,  CRC(58f0dcb4) SHA1(79039d489f5ce3a1865fa92a2b6e8b002b63efcf))
	ROM_LOAD16_BYTE("bi______.2_5",  0x040000, 0x010000,  CRC(512c6d1a) SHA1(1c9d04e7e59a95f6975e6d2d5e5def4c1a7777e0))
	ROM_LOAD16_BYTE("bi______.2_6",  0x040001, 0x010000,  CRC(5df850ec) SHA1(5b455f4cfb19c551723f7fd5f4f95e5420f8682f))
	ROM_LOAD16_BYTE("bi______.2_7",  0x060000, 0x010000,  CRC(9ea394a2) SHA1(3b9840627f7676aa7872d2bc022406a4ced7958f))
	ROM_LOAD16_BYTE("bi______.2_8",  0x060001, 0x010000,  CRC(4aa1d37d) SHA1(3c1a3ccacdc33cd4a54b7bbf06ea4c33868705f4))
ROM_END

ROM_START( v4bigfrtb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("bi_20bg_.2_0",  0x00000, 0x10000,  CRC(f1b5cb6a) SHA1(b26040a283d3c2642367e86f66e47d97c1ac07a4))  // protocol/20p/Gala Bingo

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("bi______.2_1",  0x000000, 0x010000,  CRC(22e7e736) SHA1(478abe8dc9e29b8ceccfac80ddaa5ce2727572cd))
	ROM_LOAD16_BYTE("bi______.2_2",  0x000001, 0x010000,  CRC(9d139ae7) SHA1(fb38b3102355ee5f79f66410ce5a26f74296321a))
	ROM_LOAD16_BYTE("bi______.2_3",  0x020000, 0x010000,  CRC(9f4c424d) SHA1(f20e5e6f43ac0d481b7f2d8ea421fa6b9fa92b38))
	ROM_LOAD16_BYTE("bi______.2_4",  0x020001, 0x010000,  CRC(58f0dcb4) SHA1(79039d489f5ce3a1865fa92a2b6e8b002b63efcf))
	ROM_LOAD16_BYTE("bi______.2_5",  0x040000, 0x010000,  CRC(512c6d1a) SHA1(1c9d04e7e59a95f6975e6d2d5e5def4c1a7777e0))
	ROM_LOAD16_BYTE("bi______.2_6",  0x040001, 0x010000,  CRC(5df850ec) SHA1(5b455f4cfb19c551723f7fd5f4f95e5420f8682f))
	ROM_LOAD16_BYTE("bi______.2_7",  0x060000, 0x010000,  CRC(9ea394a2) SHA1(3b9840627f7676aa7872d2bc022406a4ced7958f))
	ROM_LOAD16_BYTE("bi______.2_8",  0x060001, 0x010000,  CRC(4aa1d37d) SHA1(3c1a3ccacdc33cd4a54b7bbf06ea4c33868705f4))
ROM_END

ROM_START( v4bigfrtc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("bi_20sb_.2_0",  0x00000, 0x10000,  CRC(8e646474) SHA1(8b72fba96947fb78a79997b37987eadec522cc4e))  // protocol and % key/20p

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("bi______.2_1",  0x000000, 0x010000,  CRC(22e7e736) SHA1(478abe8dc9e29b8ceccfac80ddaa5ce2727572cd))
	ROM_LOAD16_BYTE("bi______.2_2",  0x000001, 0x010000,  CRC(9d139ae7) SHA1(fb38b3102355ee5f79f66410ce5a26f74296321a))
	ROM_LOAD16_BYTE("bi______.2_3",  0x020000, 0x010000,  CRC(9f4c424d) SHA1(f20e5e6f43ac0d481b7f2d8ea421fa6b9fa92b38))
	ROM_LOAD16_BYTE("bi______.2_4",  0x020001, 0x010000,  CRC(58f0dcb4) SHA1(79039d489f5ce3a1865fa92a2b6e8b002b63efcf))
	ROM_LOAD16_BYTE("bi______.2_5",  0x040000, 0x010000,  CRC(512c6d1a) SHA1(1c9d04e7e59a95f6975e6d2d5e5def4c1a7777e0))
	ROM_LOAD16_BYTE("bi______.2_6",  0x040001, 0x010000,  CRC(5df850ec) SHA1(5b455f4cfb19c551723f7fd5f4f95e5420f8682f))
	ROM_LOAD16_BYTE("bi______.2_7",  0x060000, 0x010000,  CRC(9ea394a2) SHA1(3b9840627f7676aa7872d2bc022406a4ced7958f))
	ROM_LOAD16_BYTE("bi______.2_8",  0x060001, 0x010000,  CRC(4aa1d37d) SHA1(3c1a3ccacdc33cd4a54b7bbf06ea4c33868705f4))
ROM_END

ROM_START( v4bubbnk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("bu_20a__.4_0",  0x00000, 0x10000,  CRC(743a0c56) SHA1(6dfe6733d19b19fcb9be2472615a340237253966))  // Standard/20p

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("bu______.4_1",  0x000000, 0x010000,  CRC(1e85dd5d) SHA1(0f3c35d9f75d3495e2d0cc1fcf0e96dcbbeeacc8))
	ROM_LOAD16_BYTE("bu______.4_2",  0x000001, 0x010000,  CRC(989db70a) SHA1(e44f8900392db51cc3f0a5bf0391bde0d71b9878))
	ROM_LOAD16_BYTE("bu______.4_3",  0x020000, 0x010000,  CRC(b079fc12) SHA1(6cb736fbe07617eca3ea691e85ff17a0ae218faf))
	ROM_LOAD16_BYTE("bu______.4_4",  0x020001, 0x010000,  CRC(7e22a7cf) SHA1(00f6a303684850e0a8a168d4a6da628bc19599ea))
	ROM_LOAD16_BYTE("bu______.4_5",  0x040000, 0x010000,  CRC(225eff49) SHA1(015efc0f058172cc03f9e38794784e52c097e2bc))
	ROM_LOAD16_BYTE("bu______.4_6",  0x040001, 0x010000,  CRC(84ce8822) SHA1(5b9b24bc5e586678d415c1234b8c870776f970cb))
	ROM_LOAD16_BYTE("bu______.4_7",  0x060000, 0x010000,  CRC(a3b00c9f) SHA1(16d9f09a9c20a23521468ea332d19ec485e3606f))
	ROM_LOAD16_BYTE("bu______.4_8",  0x060001, 0x010000,  CRC(7bd03d6e) SHA1(63802bc31115410f5021cdaca2ef832ace59288d))
	ROM_LOAD16_BYTE("bu______.4_9",  0x080000, 0x010000,  CRC(18157aca) SHA1(e6666ecae620a9d8d3c9099f2d269a3cebd05d0f))
	ROM_LOAD16_BYTE("bu______.4_a",  0x080001, 0x010000,  CRC(bc76817d) SHA1(418355740310d431a2803663b5138dff4b45452f))
ROM_END

ROM_START( v4bubbnka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("bui20a__.4_0",  0x00000, 0x10000,  CRC(154263c0) SHA1(8207ce1876be443e2023554629c0de1117544170))  // Irish/20p

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("bu______.4_1",  0x000000, 0x010000,  CRC(1e85dd5d) SHA1(0f3c35d9f75d3495e2d0cc1fcf0e96dcbbeeacc8))
	ROM_LOAD16_BYTE("bu______.4_2",  0x000001, 0x010000,  CRC(989db70a) SHA1(e44f8900392db51cc3f0a5bf0391bde0d71b9878))
	ROM_LOAD16_BYTE("bu______.4_3",  0x020000, 0x010000,  CRC(b079fc12) SHA1(6cb736fbe07617eca3ea691e85ff17a0ae218faf))
	ROM_LOAD16_BYTE("bu______.4_4",  0x020001, 0x010000,  CRC(7e22a7cf) SHA1(00f6a303684850e0a8a168d4a6da628bc19599ea))
	ROM_LOAD16_BYTE("bu______.4_5",  0x040000, 0x010000,  CRC(225eff49) SHA1(015efc0f058172cc03f9e38794784e52c097e2bc))
	ROM_LOAD16_BYTE("bu______.4_6",  0x040001, 0x010000,  CRC(84ce8822) SHA1(5b9b24bc5e586678d415c1234b8c870776f970cb))
	ROM_LOAD16_BYTE("bu______.4_7",  0x060000, 0x010000,  CRC(a3b00c9f) SHA1(16d9f09a9c20a23521468ea332d19ec485e3606f))
	ROM_LOAD16_BYTE("bu______.4_8",  0x060001, 0x010000,  CRC(7bd03d6e) SHA1(63802bc31115410f5021cdaca2ef832ace59288d))
	ROM_LOAD16_BYTE("bu______.4_9",  0x080000, 0x010000,  CRC(18157aca) SHA1(e6666ecae620a9d8d3c9099f2d269a3cebd05d0f))
	ROM_LOAD16_BYTE("bu______.4_a",  0x080001, 0x010000,  CRC(bc76817d) SHA1(418355740310d431a2803663b5138dff4b45452f))
ROM_END

ROM_START( v4bubbnkb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("bu_20ab_.4_0",  0x00000, 0x10000,  CRC(82be9bc5) SHA1(6bd5e08cd157c7c02259aa93ddbce19979dd85f0))  // protocol and % key/20p

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("bu______.4_1",  0x000000, 0x010000,  CRC(1e85dd5d) SHA1(0f3c35d9f75d3495e2d0cc1fcf0e96dcbbeeacc8))
	ROM_LOAD16_BYTE("bu______.4_2",  0x000001, 0x010000,  CRC(989db70a) SHA1(e44f8900392db51cc3f0a5bf0391bde0d71b9878))
	ROM_LOAD16_BYTE("bu______.4_3",  0x020000, 0x010000,  CRC(b079fc12) SHA1(6cb736fbe07617eca3ea691e85ff17a0ae218faf))
	ROM_LOAD16_BYTE("bu______.4_4",  0x020001, 0x010000,  CRC(7e22a7cf) SHA1(00f6a303684850e0a8a168d4a6da628bc19599ea))
	ROM_LOAD16_BYTE("bu______.4_5",  0x040000, 0x010000,  CRC(225eff49) SHA1(015efc0f058172cc03f9e38794784e52c097e2bc))
	ROM_LOAD16_BYTE("bu______.4_6",  0x040001, 0x010000,  CRC(84ce8822) SHA1(5b9b24bc5e586678d415c1234b8c870776f970cb))
	ROM_LOAD16_BYTE("bu______.4_7",  0x060000, 0x010000,  CRC(a3b00c9f) SHA1(16d9f09a9c20a23521468ea332d19ec485e3606f))
	ROM_LOAD16_BYTE("bu______.4_8",  0x060001, 0x010000,  CRC(7bd03d6e) SHA1(63802bc31115410f5021cdaca2ef832ace59288d))
	ROM_LOAD16_BYTE("bu______.4_9",  0x080000, 0x010000,  CRC(18157aca) SHA1(e6666ecae620a9d8d3c9099f2d269a3cebd05d0f))
	ROM_LOAD16_BYTE("bu______.4_a",  0x080001, 0x010000,  CRC(bc76817d) SHA1(418355740310d431a2803663b5138dff4b45452f))
ROM_END

ROM_START( v4bubbnkc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("bu_20ad_.4_0",  0x00000, 0x10000,  CRC(3f9febf7) SHA1(172e7e03d7b4c6fdda6bb8f3f5d93d3803f678c3))  // protocol/20p

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("bu______.4_1",  0x000000, 0x010000,  CRC(1e85dd5d) SHA1(0f3c35d9f75d3495e2d0cc1fcf0e96dcbbeeacc8))
	ROM_LOAD16_BYTE("bu______.4_2",  0x000001, 0x010000,  CRC(989db70a) SHA1(e44f8900392db51cc3f0a5bf0391bde0d71b9878))
	ROM_LOAD16_BYTE("bu______.4_3",  0x020000, 0x010000,  CRC(b079fc12) SHA1(6cb736fbe07617eca3ea691e85ff17a0ae218faf))
	ROM_LOAD16_BYTE("bu______.4_4",  0x020001, 0x010000,  CRC(7e22a7cf) SHA1(00f6a303684850e0a8a168d4a6da628bc19599ea))
	ROM_LOAD16_BYTE("bu______.4_5",  0x040000, 0x010000,  CRC(225eff49) SHA1(015efc0f058172cc03f9e38794784e52c097e2bc))
	ROM_LOAD16_BYTE("bu______.4_6",  0x040001, 0x010000,  CRC(84ce8822) SHA1(5b9b24bc5e586678d415c1234b8c870776f970cb))
	ROM_LOAD16_BYTE("bu______.4_7",  0x060000, 0x010000,  CRC(a3b00c9f) SHA1(16d9f09a9c20a23521468ea332d19ec485e3606f))
	ROM_LOAD16_BYTE("bu______.4_8",  0x060001, 0x010000,  CRC(7bd03d6e) SHA1(63802bc31115410f5021cdaca2ef832ace59288d))
	ROM_LOAD16_BYTE("bu______.4_9",  0x080000, 0x010000,  CRC(18157aca) SHA1(e6666ecae620a9d8d3c9099f2d269a3cebd05d0f))
	ROM_LOAD16_BYTE("bu______.4_a",  0x080001, 0x010000,  CRC(bc76817d) SHA1(418355740310d431a2803663b5138dff4b45452f))
ROM_END

ROM_START( v4bubbnkd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("bu_20ak_.4_0",  0x00000, 0x10000,  CRC(c91b7c64) SHA1(7cb4027e43e524792e1b5e523857f5d52cc7e7d2))  // % Key/20p

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("bu______.4_1",  0x000000, 0x010000,  CRC(1e85dd5d) SHA1(0f3c35d9f75d3495e2d0cc1fcf0e96dcbbeeacc8))
	ROM_LOAD16_BYTE("bu______.4_2",  0x000001, 0x010000,  CRC(989db70a) SHA1(e44f8900392db51cc3f0a5bf0391bde0d71b9878))
	ROM_LOAD16_BYTE("bu______.4_3",  0x020000, 0x010000,  CRC(b079fc12) SHA1(6cb736fbe07617eca3ea691e85ff17a0ae218faf))
	ROM_LOAD16_BYTE("bu______.4_4",  0x020001, 0x010000,  CRC(7e22a7cf) SHA1(00f6a303684850e0a8a168d4a6da628bc19599ea))
	ROM_LOAD16_BYTE("bu______.4_5",  0x040000, 0x010000,  CRC(225eff49) SHA1(015efc0f058172cc03f9e38794784e52c097e2bc))
	ROM_LOAD16_BYTE("bu______.4_6",  0x040001, 0x010000,  CRC(84ce8822) SHA1(5b9b24bc5e586678d415c1234b8c870776f970cb))
	ROM_LOAD16_BYTE("bu______.4_7",  0x060000, 0x010000,  CRC(a3b00c9f) SHA1(16d9f09a9c20a23521468ea332d19ec485e3606f))
	ROM_LOAD16_BYTE("bu______.4_8",  0x060001, 0x010000,  CRC(7bd03d6e) SHA1(63802bc31115410f5021cdaca2ef832ace59288d))
	ROM_LOAD16_BYTE("bu______.4_9",  0x080000, 0x010000,  CRC(18157aca) SHA1(e6666ecae620a9d8d3c9099f2d269a3cebd05d0f))
	ROM_LOAD16_BYTE("bu______.4_a",  0x080001, 0x010000,  CRC(bc76817d) SHA1(418355740310d431a2803663b5138dff4b45452f))
ROM_END

ROM_START( v4mazbel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("maz20dy2.5_0",  0x00000, 0x10000,  CRC(e43220df) SHA1(07b58cc40fa490564a6bf8add65a3b3c4d9b1164))  // protocol % key/20p/

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("maz20__2.5_1",  0x000000, 0x010000,  CRC(6d3b4dbd) SHA1(79479c458395fa718f00e4c1547b186a85153848))
	ROM_LOAD16_BYTE("maz20__2.5_2",  0x000001, 0x010000,  CRC(40074cc3) SHA1(498beeed1a1f86e535474a25f3ec73c1fdb004b9))
	ROM_LOAD16_BYTE("maz20__2.5_3",  0x020000, 0x010000,  CRC(b78bc8a9) SHA1(7436cb9e67a3d72b36cbdd92d8aa3e26e6b7b40d))
	ROM_LOAD16_BYTE("maz20__2.5_4",  0x020001, 0x010000,  CRC(116a6f65) SHA1(10fa599b620f6d595b27723f7ae1c36f6ff61f7d))
ROM_END

ROM_START( v4mazbel15 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("maz20__1.5x0",  0x00000, 0x10000,  CRC(06f95de7) SHA1(6ee8a345b12c9993513dda686ddb8c89d847976f))  // standard/20p

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("maz20__1.5x1",  0x000000, 0x010000,  CRC(a4be5a3f) SHA1(e510a54c7d7551dbb2c5dd450c921fd0e3649db6))
	ROM_LOAD16_BYTE("maz20__1.5x2",  0x000001, 0x010000,  CRC(261466aa) SHA1(ce61f7f1af9abed6e9fad778a270b3c4c167d401))
	ROM_LOAD16_BYTE("maz20__1.5x3",  0x020000, 0x010000,  CRC(a9f02fc6) SHA1(d892bcd3e755c623bb7f2ff4db4876c091c5ae30))
	ROM_LOAD16_BYTE("maz20__1.5x4",  0x020001, 0x010000,  CRC(b5f333b6) SHA1(b848f9d6f39e102d72992d56836cd5b8c16b5554))
ROM_END

ROM_START( v4mazbel15a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("maz20dy1.5x0",  0x00000, 0x10000,  CRC(f07dca74) SHA1(f5c2658911a3d8d082ca3f2289b69e767f5a80f4))  // protocol % key/20p/

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("maz20__1.5x1",  0x000000, 0x010000,  CRC(a4be5a3f) SHA1(e510a54c7d7551dbb2c5dd450c921fd0e3649db6))
	ROM_LOAD16_BYTE("maz20__1.5x2",  0x000001, 0x010000,  CRC(261466aa) SHA1(ce61f7f1af9abed6e9fad778a270b3c4c167d401))
	ROM_LOAD16_BYTE("maz20__1.5x3",  0x020000, 0x010000,  CRC(a9f02fc6) SHA1(d892bcd3e755c623bb7f2ff4db4876c091c5ae30))
	ROM_LOAD16_BYTE("maz20__1.5x4",  0x020001, 0x010000,  CRC(b5f333b6) SHA1(b848f9d6f39e102d72992d56836cd5b8c16b5554))
ROM_END


ROM_START( v4shpwnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sw_20___.2_0",  0x00000, 0x10000,  CRC(c1b49d81) SHA1(dcb077734beb814002046d36091a6407644c1393))  // Standard/20p/

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwnda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_10___.2_0", 0x0000, 0x010000, CRC(1377a409) SHA1(632bae8bf70270fb13caabbc491f81f8b6589ece) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwndb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_10_b_.2_0", 0x0000, 0x010000, CRC(e5f3339a) SHA1(67d79c2939ad0c566e05e470672d7153fdc2fbd5) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwndc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_10_d_.2_0", 0x0000, 0x010000, CRC(58d243a8) SHA1(1f718a153e2a42893c2dcecddf857c93872a780d) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwndd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_10_k_.2_0", 0x0000, 0x010000, CRC(ae56d43b) SHA1(ad73f7ad4c142d3ad629b0df46985456a2f8ebc8) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwnde )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_20_b_.2_0", 0x0000, 0x010000, CRC(37300a12) SHA1(34abe1b28afe9c36810f9c7eba356720d9ff53de) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwndf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_20_d_.2_0", 0x0000, 0x010000, CRC(8a117a20) SHA1(697d97155b3c4f515988390997064ace7402959e) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwndg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_20_k_.2_0", 0x0000, 0x010000, CRC(7c95edb3) SHA1(4a8208c2786eed73a4ade67209a5cb66ba8c4ed8) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwndh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_20bg_.2_0", 0x0000, 0x010000, CRC(48e1a50c) SHA1(7f1aaa3207b86996cb1804ff82685723839e7c7c) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwndi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_20bt_.2_0", 0x0000, 0x010000, CRC(feaaa80f) SHA1(23ec154fe0fdb4bbb558960d700c94674bb4a144) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwndj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_xc___.2_0", 0x0000, 0x010000, CRC(f108aa4a) SHA1(f27456a22b781df3103cc7b7f35fb08bb547e864) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwndk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_xc_b_.2_0", 0x0000, 0x010000, CRC(078c3dd9) SHA1(07515a2476ae15c780da2de58de8b17f30c0ebdf) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwndl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_xc_d_.2_0", 0x0000, 0x010000, CRC(baad4deb) SHA1(ad3d90eefdad4e6baa10558ed5a1415ed94742b6) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

ROM_START( v4shpwndm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sw_xc_k_.2_0", 0x0000, 0x010000, CRC(4c29da78) SHA1(0431016a0a13d97fc833e31bc2897e7abb6a5047) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE("sw______.2_1",  0x000000, 0x010000,  CRC(3346ee38) SHA1(29659ba8173b86bb52057f75dc874465048ef6d9))
	ROM_LOAD16_BYTE("sw______.2_2",  0x000001, 0x010000,  CRC(2c5293c8) SHA1(e0b5fa629e8dbbc6164f16cbbe19d0e42467d281))
	ROM_LOAD16_BYTE("sw______.2_3",  0x020000, 0x010000,  CRC(09144ac0) SHA1(d55658a361ebec0e054e3a9d99eba61c81fde619))
	ROM_LOAD16_BYTE("sw______.2_4",  0x020001, 0x010000,  CRC(a23db51a) SHA1(9516a9655c5f7f000475df03f05f332c5ec09959))
ROM_END

/* Nova */

ROM_START( v4cybcas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ccd_____.6_0", 0x0000, 0x010000, CRC(93aa1a3d) SHA1(dce4cc4bfc616163ca240c417d1ef9a0a19e5901) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ccd_____.6_1", 0x000001, 0x080000, CRC(58c8ae96) SHA1(9f3aea47ae014c61bb2b75664210a3609996f983) )
	ROM_LOAD16_BYTE( "ccd_____.6_2", 0x000000, 0x080000, CRC(f0556749) SHA1(2f35a9f9672219001c2a48d7f103ac99e1b21b32) )
	ROM_LOAD16_BYTE( "ccd_____.6_3", 0x100001, 0x080000, CRC(4d2f5485) SHA1(a8c9b5843384ce55a10531b82766f5498dc0d334) )
	ROM_LOAD16_BYTE( "ccd_____.6_4", 0x100000, 0x080000, CRC(87b29468) SHA1(89a3855754e8a720ca76fa16b11f791ade75fb78) )
	ROM_LOAD16_BYTE( "ccd_____.6_5", 0x200001, 0x080000, CRC(bd8be9ff) SHA1(c5d620774e476e6ec00876e513ff838af102ecc9) )
	ROM_LOAD16_BYTE( "ccd_____.6_6", 0x200000, 0x080000, CRC(faf65e25) SHA1(f50369348740ba4c878455c108ac3e8d2262ef5e) )
	ROM_LOAD16_BYTE( "ccd_____.6_7", 0x300001, 0x080000, CRC(4cd1461c) SHA1(00379212130d5e9c1c364191f67a35cc5eca8b72) )
	ROM_LOAD16_BYTE( "ccd_____.6_8", 0x300000, 0x080000, CRC(08ce378f) SHA1(035b0ff4d18c09a385002db73c54b8dac92dfa8a) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4mdiceger )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mdd4_0.bin", 0x0000, 0x010000, CRC(0d868466) SHA1(3cea446f094ae3b4f56163ccf01cd31c15dca03f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md41.p1", 0x000001, 0x080000, CRC(9567347f) SHA1(5188b3f359ff26c0854d5745df37575851502ae9) )
	ROM_LOAD16_BYTE( "md42.p2", 0x000000, 0x080000, CRC(e0dad693) SHA1(9768331a4a776906935b3c5501b68cb4dd1bd41f) )
	ROM_LOAD16_BYTE( "md43.p3", 0x100001, 0x080000, CRC(9bceb3f6) SHA1(5be84d5f1635f80a9fe8072c2d94012ed00d97be) )
	ROM_LOAD16_BYTE( "md44.p4", 0x100000, 0x080000, CRC(54b8fbfa) SHA1(ca2fb67972507a2eb33d2800a3b2d45d3ee49289) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "md41snd.p1", 0x000000, 0x080000, CRC(7a3ba770) SHA1(e620f521d16b39be9b41b934435812afff3993b2) )
	ROM_LOAD( "md42snd.p2", 0x080000, 0x080000, CRC(8ebc7329) SHA1(82ce25c1486a8619355f363125a26d8cdeb05d33) )
	ROM_LOAD( "md43snd.p3", 0x100000, 0x080000, CRC(14f4d838) SHA1(0508890a9884fbef0e194a38fe3afc5cf5282091) )
ROM_END


ROM_START( v4missis )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mldsf___.6_0", 0x0000, 0x010000, CRC(3f0fe40e) SHA1(6fd9ddede5f11c35d47c7ddd360b0021267e7e3f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mld_____.6_1", 0x000000, 0x080000, CRC(44e26e6e) SHA1(ba96757b74e75d094b6cda340c78754f1b8f6a7c) )
	ROM_LOAD16_BYTE( "mld_____.6_2", 0x000001, 0x080000, CRC(e9219fff) SHA1(80c0211b98c4200ad9512e89b067018ce8600494) )
	ROM_LOAD16_BYTE( "mld_____.6_3", 0x100000, 0x080000, CRC(680bd284) SHA1(33dfd0cb396c1466c2a5cfc474756b8a2d966518) )
	ROM_LOAD16_BYTE( "mld_____.6_4", 0x100001, 0x080000, CRC(0e21022b) SHA1(30465321a976064f7841b2c3314244b744a45092) )
	ROM_LOAD16_BYTE( "mld_____.6_5", 0x200000, 0x080000, CRC(cd6c39d2) SHA1(c4d4b5c7a1f3dcfdc464fff29f10ccee932f265a) )
	ROM_LOAD16_BYTE( "mld_____.6_6", 0x200001, 0x080000, CRC(5ad997a3) SHA1(0fd75b4a9b5991fda9cc3373b86f466492c3b4bb) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mld_snd_.1_1", 0x000000, 0x080000, CRC(8c26fe12) SHA1(0532126d8e283b4567a4cdf2bb807f5471c84832) )
	ROM_LOAD( "mld_snd_.1_2", 0x080000, 0x080000, CRC(9ab841b6) SHA1(7828c6144777621859b85a3ec92760d353576527) )
	ROM_LOAD( "mld_snd_.1_3", 0x100000, 0x080000, CRC(3f068632) SHA1(5e43da287b3aa163493c1be03ebee28ef58c44a1) )
	ROM_LOAD( "mld_snd_.1_4", 0x180000, 0x080000, CRC(f78f7221) SHA1(dac88abee6a5fdf7b69c5d39345ee05c1ac47314) )
ROM_END

ROM_START( v4picdil )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pcdsf___.a_0", 0x0000, 0x010000, CRC(45a15082) SHA1(d7655722c7ad9f3b1b2663d85287ae185917d677) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "pcd_____.a_1", 0x000001, 0x080000, CRC(0ed561d4) SHA1(899c76d2a2352988a01bece2a229deb934a00892) )
	ROM_LOAD16_BYTE( "pcd_____.a_2", 0x000000, 0x080000, CRC(bffe3863) SHA1(ea6fdcce470e8a50c35f43fd86e39524be2db3a3) )
	ROM_LOAD16_BYTE( "pcd_____.a_3", 0x100001, 0x080000, CRC(3b64a328) SHA1(a9fa9dc30b352388906e6021bc0d1ad7c3a28746) )
	ROM_LOAD16_BYTE( "pcd_____.a_4", 0x100000, 0x080000, CRC(25faba03) SHA1(572aaee3af3b915294ba057b7ceb653dd135098b) )
	ROM_LOAD16_BYTE( "pcd_____.a_5", 0x200001, 0x080000, CRC(275f3c1c) SHA1(1d0f8f7d0388d5072ae404f10b2481153979a217) )
	ROM_LOAD16_BYTE( "pcd_____.a_6", 0x200000, 0x080000, CRC(148ecba0) SHA1(2ae0f5529fa3951025539fe19f4e8fdf10f13374) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4picdila )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pcdsf__e.a_0", 0x0000, 0x010000, CRC(0ed1c7af) SHA1(c19ff141fba7fd1f2cf0b152e3c6df61c6b27b46) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "pcd_____.a_1", 0x000001, 0x080000, CRC(0ed561d4) SHA1(899c76d2a2352988a01bece2a229deb934a00892) )
	ROM_LOAD16_BYTE( "pcd_____.a_2", 0x000000, 0x080000, CRC(bffe3863) SHA1(ea6fdcce470e8a50c35f43fd86e39524be2db3a3) )
	ROM_LOAD16_BYTE( "pcd_____.a_3", 0x100001, 0x080000, CRC(3b64a328) SHA1(a9fa9dc30b352388906e6021bc0d1ad7c3a28746) )
	ROM_LOAD16_BYTE( "pcd_____.a_4", 0x100000, 0x080000, CRC(25faba03) SHA1(572aaee3af3b915294ba057b7ceb653dd135098b) )
	ROM_LOAD16_BYTE( "pcd_____.a_5", 0x200001, 0x080000, CRC(275f3c1c) SHA1(1d0f8f7d0388d5072ae404f10b2481153979a217) )
	ROM_LOAD16_BYTE( "pcd_____.a_6", 0x200000, 0x080000, CRC(148ecba0) SHA1(2ae0f5529fa3951025539fe19f4e8fdf10f13374) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4picdilz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pcdsf2_interface_a0.bin", 0x0000, 0x010000, CRC(5e007ac4) SHA1(82fd17da416c29b2e4b75f24bc8d415e57c0e94b))

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "piccadilly_vid_a1_ic15.bin", 0x000000, 0x080000, CRC(22f9ad51) SHA1(f63923e346519833f71a1644d4e504d2f93a173f) )
	ROM_LOAD16_BYTE( "piccadilly_vid_a2_ic7.bin",  0x000001, 0x080000, CRC(6de3bae1) SHA1(c9fae9bb694341b1cca05ae143beb04989326897) )
	ROM_LOAD16_BYTE( "pcd_____.a_3", 0x100001, 0x080000, CRC(3b64a328) SHA1(a9fa9dc30b352388906e6021bc0d1ad7c3a28746) )
	ROM_LOAD16_BYTE( "pcd_____.a_4", 0x100000, 0x080000, CRC(25faba03) SHA1(572aaee3af3b915294ba057b7ceb653dd135098b) )
	ROM_LOAD16_BYTE( "pcd_____.a_5", 0x200001, 0x080000, CRC(275f3c1c) SHA1(1d0f8f7d0388d5072ae404f10b2481153979a217) )
	ROM_LOAD16_BYTE( "pcd_____.a_6", 0x200000, 0x080000, CRC(148ecba0) SHA1(2ae0f5529fa3951025539fe19f4e8fdf10f13374) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "pcdsnd_v1_p1.bin", 0x000000, 0x080000, CRC(01c0bec3) SHA1(a755f939d02500f0a03e399bbf7f842173bf5a71) )
	ROM_LOAD( "pcdsnd_v1_p2.bin", 0x080000, 0x080000, CRC(a9f66e67) SHA1(eba1ff2023356face1d9a6be93417b54a132fe6f) )
	ROM_LOAD( "pcdsnd_v1_p3.bin", 0x100000, 0x080000, BAD_DUMP CRC(d15ea1bd) SHA1(f47e4d901a89ccf83784e582414f3dce08fc4e18) ) // mostly empty, corrupt?
	ROM_LOAD( "pcdsnd_v1_p4.bin", 0x180000, 0x080000, CRC(275cdbe0) SHA1(7239d48c4755072a32c237079e623baa95a32593) )
ROM_END


ROM_START( v4big40 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sja__.3_0", 0x0000, 0x010000, CRC(afdd005e) SHA1(71730b55212fbfc1905caf0d2842d495741305f1) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.3_1", 0x000001, 0x080000, CRC(c388e5a9) SHA1(baafe2da91891f288debd89907d36438494e876a) )
	ROM_LOAD16_BYTE( "b4______.3_2", 0x000000, 0x080000, CRC(cc3ab5c3) SHA1(a3778b462a823fd73c1a3463c53ef0537e8d5ed4) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END

ROM_START( v4big40a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sja__.4_0", 0x0000, 0x010000, CRC(4375e090) SHA1(923eadc4cf322e544102cede2eb7a487354981b5) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.4_1", 0x000001, 0x080000, CRC(fc33c0fc) SHA1(838a7ef4252f9f736639858fe97a4982a89be09f) )
	ROM_LOAD16_BYTE( "b4______.4_2", 0x000000, 0x080000, CRC(f2211865) SHA1(5bcb95a079f57305d3e58fae3899bceec211f44a) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END

ROM_START( v4big40b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sja__.6_0", 0x0000, 0x010000, CRC(f5742e6c) SHA1(42b0b22d2da690b24166fe103a3453dcfc7beacc) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.6_1", 0x000001, 0x080000, CRC(75532ad3) SHA1(0584261faede35f6e846ee398081fac66aea8368) )
	ROM_LOAD16_BYTE( "b4______.6_2", 0x000000, 0x080000, CRC(03ef74c5) SHA1(fa5d27b0e94c8d05f1c50b28b96f7a4ae3ecda4a) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END

ROM_START( v4big40c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sja_n.6_0", 0x0000, 0x010000, CRC(be04b941) SHA1(8d0561a623df6fc447fbcbbc1934425b23df4f58) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.6_1", 0x000001, 0x080000, CRC(75532ad3) SHA1(0584261faede35f6e846ee398081fac66aea8368) )
	ROM_LOAD16_BYTE( "b4______.6_2", 0x000000, 0x080000, CRC(03ef74c5) SHA1(fa5d27b0e94c8d05f1c50b28b96f7a4ae3ecda4a) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END

ROM_START( v4big40d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sjad_.6_0", 0x0000, 0x010000, CRC(bed1c9cd) SHA1(f4cfb305e4ec2c4c1b55356ce32745875f1641d9) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.6_1", 0x000001, 0x080000, CRC(75532ad3) SHA1(0584261faede35f6e846ee398081fac66aea8368) )
	ROM_LOAD16_BYTE( "b4______.6_2", 0x000000, 0x080000, CRC(03ef74c5) SHA1(fa5d27b0e94c8d05f1c50b28b96f7a4ae3ecda4a) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END

ROM_START( v4big40e )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sjadn.6_0", 0x0000, 0x010000, CRC(f5a15ee0) SHA1(d61fc54724d579d7a7055a1cfe20b55e4e59a653) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.6_1", 0x000001, 0x080000, CRC(75532ad3) SHA1(0584261faede35f6e846ee398081fac66aea8368) )
	ROM_LOAD16_BYTE( "b4______.6_2", 0x000000, 0x080000, CRC(03ef74c5) SHA1(fa5d27b0e94c8d05f1c50b28b96f7a4ae3ecda4a) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END

ROM_START( v4big40f )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sjmb_.6_0", 0x0000, 0x010000, CRC(7472fa9c) SHA1(3315b37b1a221e40e6122612c8ee8a27b6131947) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.6_1", 0x000001, 0x080000, CRC(75532ad3) SHA1(0584261faede35f6e846ee398081fac66aea8368) )
	ROM_LOAD16_BYTE( "b4______.6_2", 0x000000, 0x080000, CRC(03ef74c5) SHA1(fa5d27b0e94c8d05f1c50b28b96f7a4ae3ecda4a) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END

ROM_START( v4big40g )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sjs__.6_0", 0x0000, 0x010000, CRC(0092b6a9) SHA1(cead4988d2dc6b0e8724c7b1c6eb416317a9b7c4) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.6_1", 0x000001, 0x080000, CRC(75532ad3) SHA1(0584261faede35f6e846ee398081fac66aea8368) )
	ROM_LOAD16_BYTE( "b4______.6_2", 0x000000, 0x080000, CRC(03ef74c5) SHA1(fa5d27b0e94c8d05f1c50b28b96f7a4ae3ecda4a) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END

ROM_START( v4big40h )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sjs_n.6_0", 0x0000, 0x010000, CRC(4be22184) SHA1(5b779d8fe85db719b539f93ff05a6e3568968557) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.6_1", 0x000001, 0x080000, CRC(75532ad3) SHA1(0584261faede35f6e846ee398081fac66aea8368) )
	ROM_LOAD16_BYTE( "b4______.6_2", 0x000000, 0x080000, CRC(03ef74c5) SHA1(fa5d27b0e94c8d05f1c50b28b96f7a4ae3ecda4a) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END

ROM_START( v4big40i )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sjsb_.6_0", 0x0000, 0x010000, CRC(f616213a) SHA1(bcfc7fd1639ed423faa32d066c7f19877e4d1bc2) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.6_1", 0x000001, 0x080000, CRC(75532ad3) SHA1(0584261faede35f6e846ee398081fac66aea8368) )
	ROM_LOAD16_BYTE( "b4______.6_2", 0x000000, 0x080000, CRC(03ef74c5) SHA1(fa5d27b0e94c8d05f1c50b28b96f7a4ae3ecda4a) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END

ROM_START( v4big40j )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sjsbn.6_0", 0x0000, 0x010000, CRC(bd66b617) SHA1(522c0903405f1a97c140c2aaef543aff2057e6ba) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.6_1", 0x000001, 0x080000, CRC(75532ad3) SHA1(0584261faede35f6e846ee398081fac66aea8368) )
	ROM_LOAD16_BYTE( "b4______.6_2", 0x000000, 0x080000, CRC(03ef74c5) SHA1(fa5d27b0e94c8d05f1c50b28b96f7a4ae3ecda4a) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END

ROM_START( v4big40k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b4_sjsd_.6_0", 0x0000, 0x010000, CRC(4b375108) SHA1(77e5a59ca524550c68231cf64387e8f206f97393) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "b4______.6_1", 0x000001, 0x080000, CRC(75532ad3) SHA1(0584261faede35f6e846ee398081fac66aea8368) )
	ROM_LOAD16_BYTE( "b4______.6_2", 0x000000, 0x080000, CRC(03ef74c5) SHA1(fa5d27b0e94c8d05f1c50b28b96f7a4ae3ecda4a) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "b4__snd_.1_a", 0x0000, 0x080000, CRC(2d630b87) SHA1(e4be02a1356735c47934f8f30e1e2462bf28968c) )
ROM_END


ROM_START( v4bulblx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bv_50___.2_0", 0x0000, 0x010000, CRC(615f9772) SHA1(56bc2c78c8b50b7250906ad43211f26c2e7e17b8) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "bv_50___.2_1", 0x000000, 0x010000, CRC(bb80cd18) SHA1(7e16100c30fe8d6feb1ac206447269e9c29489a5) )
	ROM_LOAD16_BYTE( "bv_50___.2_2", 0x000001, 0x010000, CRC(2a18b8dc) SHA1(c2fccc7eae41176941ec15c421430b8bb20f72bd) )
	ROM_LOAD16_BYTE( "bv______.2_3", 0x020000, 0x010000, CRC(d249b9dd) SHA1(b624759b10ac12146e6026e52d5d96fe4f7663b3) )
	ROM_LOAD16_BYTE( "bv______.2_4", 0x020001, 0x010000, CRC(f5fffa95) SHA1(17c63a7e83feb0cafca34c30b334db082bb2c321) )
	ROM_LOAD16_BYTE( "bv______.2_5", 0x040000, 0x010000, CRC(c5775387) SHA1(b301392ae39298284ae256c819877ae287861cc8) )
	ROM_LOAD16_BYTE( "bv______.2_6", 0x040001, 0x010000, CRC(4443fddc) SHA1(fb4972620f9aa07f8bd62701f64b7902567d34db) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4bulblxa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bv_50_d_.2_0", 0x0000, 0x010000, CRC(2afa70d3) SHA1(11da4db50162054deb753ab85d11566afef2b801) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "bv_50___.2_1", 0x000000, 0x010000, CRC(bb80cd18) SHA1(7e16100c30fe8d6feb1ac206447269e9c29489a5) )
	ROM_LOAD16_BYTE( "bv_50___.2_2", 0x000001, 0x010000, CRC(2a18b8dc) SHA1(c2fccc7eae41176941ec15c421430b8bb20f72bd) )
	ROM_LOAD16_BYTE( "bv______.2_3", 0x020000, 0x010000, CRC(d249b9dd) SHA1(b624759b10ac12146e6026e52d5d96fe4f7663b3) )
	ROM_LOAD16_BYTE( "bv______.2_4", 0x020001, 0x010000, CRC(f5fffa95) SHA1(17c63a7e83feb0cafca34c30b334db082bb2c321) )
	ROM_LOAD16_BYTE( "bv______.2_5", 0x040000, 0x010000, CRC(c5775387) SHA1(b301392ae39298284ae256c819877ae287861cc8) )
	ROM_LOAD16_BYTE( "bv______.2_6", 0x040001, 0x010000, CRC(4443fddc) SHA1(fb4972620f9aa07f8bd62701f64b7902567d34db) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4bulblxb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bv_1p___.2_0", 0x0000, 0x010000, CRC(f3101261) SHA1(e2c82222e3384620f52e15350ab38e62beabe1b3) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "bv_1p___.2_1", 0x000000, 0x010000, CRC(568ea314) SHA1(fa65f2cc4734b9987a153fbf1ac4b32cd0fe4f32) )
	ROM_LOAD16_BYTE( "bv_1p___.2_2", 0x000001, 0x010000, CRC(b91f8c3b) SHA1(82e1582f75595283a4ea9f2dc39e0d5a0d5ccb57) )
	ROM_LOAD16_BYTE( "bv______.2_3", 0x020000, 0x010000, CRC(d249b9dd) SHA1(b624759b10ac12146e6026e52d5d96fe4f7663b3) )
	ROM_LOAD16_BYTE( "bv______.2_4", 0x020001, 0x010000, CRC(f5fffa95) SHA1(17c63a7e83feb0cafca34c30b334db082bb2c321) )
	ROM_LOAD16_BYTE( "bv______.2_5", 0x040000, 0x010000, CRC(c5775387) SHA1(b301392ae39298284ae256c819877ae287861cc8) )
	ROM_LOAD16_BYTE( "bv______.2_6", 0x040001, 0x010000, CRC(4443fddc) SHA1(fb4972620f9aa07f8bd62701f64b7902567d34db) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4bulblxc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bv_1p_d_.2_0", 0x0000, 0x010000, CRC(b8b5f5c0) SHA1(84bf9b5a3ca5ea7eae1ab97af481206fb87bd2c5) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "bv_1p___.2_1", 0x000000, 0x010000, CRC(568ea314) SHA1(fa65f2cc4734b9987a153fbf1ac4b32cd0fe4f32) )
	ROM_LOAD16_BYTE( "bv_1p___.2_2", 0x000001, 0x010000, CRC(b91f8c3b) SHA1(82e1582f75595283a4ea9f2dc39e0d5a0d5ccb57) )
	ROM_LOAD16_BYTE( "bv______.2_3", 0x020000, 0x010000, CRC(d249b9dd) SHA1(b624759b10ac12146e6026e52d5d96fe4f7663b3) )
	ROM_LOAD16_BYTE( "bv______.2_4", 0x020001, 0x010000, CRC(f5fffa95) SHA1(17c63a7e83feb0cafca34c30b334db082bb2c321) )
	ROM_LOAD16_BYTE( "bv______.2_5", 0x040000, 0x010000, CRC(c5775387) SHA1(b301392ae39298284ae256c819877ae287861cc8) )
	ROM_LOAD16_BYTE( "bv______.2_6", 0x040001, 0x010000, CRC(4443fddc) SHA1(fb4972620f9aa07f8bd62701f64b7902567d34db) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4cshinf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_20s__.4_0", 0x0000, 0x010000, CRC(7a62c68b) SHA1(b892e023cd5ec2a508d36075ba5a96320408a4ab) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_05a__.4_0", 0x0000, 0x010000, CRC(3426fb02) SHA1(84be34a727444232c595c7adaa0a1d733b5416a3) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_10a__.4_0", 0x0000, 0x010000, CRC(5d4767c6) SHA1(291cf48c387fc515dce6eb22b1b38aaa65aec4ac) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4cshinfd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_20a__.4_0", 0x0000, 0x010000, CRC(8f845e4e) SHA1(1b8e46fed36db1b1539df3e28fcbb739079d3960) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_20bg_.4_0", 0x0000, 0x010000, CRC(f337fe06) SHA1(eef073d04a08347ad4814cf0d15615d45ecc8314) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinff )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_20bgp.4_0", 0x0000, 0x010000, CRC(65d6d05c) SHA1(3d0bd44cb692ae54ba0b2186d645124dc12dc5b3) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_20bt_.4_0", 0x0000, 0x010000, CRC(457cf305) SHA1(122e7f494043ebba01b5b827409d419ac54b240e) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_20btp.4_0", 0x0000, 0x010000, CRC(d39ddd5f) SHA1(6c97f3c19aea51efec4e510c46b98fbf3a2356a5) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_20sb_.4_0", 0x0000, 0x010000, CRC(8ce65118) SHA1(57207f1ef7e98eac7f31671789fb1e01e87c2aa0) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_20sbp.4_0", 0x0000, 0x010000, CRC(1a077f42) SHA1(141f7c3708bde43302a68df65d74d4a2e7296a05) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_20sd_.4_0", 0x0000, 0x010000, CRC(31c7212a) SHA1(b34aee87d4be460f47c894807ef877700b6be60f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_20sk_.4_0", 0x0000, 0x010000, CRC(c743b6b9) SHA1(0ef3de1663527a770066e872699ee281f1af21eb) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_xea__.4_0", 0x0000, 0x010000, CRC(bf386985) SHA1(a7eec0d70bc0795f537ec0975dc251739b86c3ca) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_xea_p.4_0", 0x0000, 0x010000, CRC(29d947df) SHA1(b08ca18442a701eb6a6f1ffc868333e65aa4aeb2) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_xeab_.4_0", 0x0000, 0x010000, CRC(49bcfe16) SHA1(0feab2f177ef6bd1d1c7ae9507a247c7afab7e97) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_xead_.4_0", 0x0000, 0x010000, CRC(f49d8e24) SHA1(f24b41ae866b324af3605f1bc121cd445b9ecb8c) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_xeak_.4_0", 0x0000, 0x010000, CRC(021919b7) SHA1(d8176be8b4548a5b61af415e7b7d0d1c8742592b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_xes__.4_0", 0x0000, 0x010000, CRC(4adef140) SHA1(1003e666532fd773617d4ed30380eda0089027fe) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinfs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ci_xesd_.4_0", 0x0000, 0x010000, CRC(017b16e1) SHA1(c352c7f635ece2faceb2f3ee6154c9fed86ddc6f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END




ROM_START( v4cshinfu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ciixea__.4_0", 0x0000, 0x010000, CRC(de400613) SHA1(b353c47b01c3918677f1a2c302171bde7386daf3) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4cshinfw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ciixead_.4_0", 0x0000, 0x010000, CRC(95e5e1b2) SHA1(b823b1c873e209fee61f32f458d6d4f5ec7e206b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ci______.4_1", 0x000000, 0x010000, CRC(bd1adec6) SHA1(7a60adc4b5159e672eb4dc33051192f0cf863aef) )
	ROM_LOAD16_BYTE( "ci______.4_2", 0x000001, 0x010000, CRC(e46852ff) SHA1(ad6f0a75578041b06120d1d5f3c3779c827b8f2f) )
	ROM_LOAD16_BYTE( "ci______.4_3", 0x020000, 0x010000, CRC(b63b9edf) SHA1(2a47c97f5ea51bc441fb847da6e81451dde77093) )
	ROM_LOAD16_BYTE( "ci______.4_4", 0x020001, 0x010000, CRC(7ef4daeb) SHA1(f889f4e68dc4ef736eee44ddedaa94832c6cdbc7) )
	ROM_LOAD16_BYTE( "ci______.4_5", 0x040000, 0x010000, CRC(381277b7) SHA1(6d934d400468c067809452981577c17cf425ff4f) )
	ROM_LOAD16_BYTE( "ci______.4_6", 0x040001, 0x010000, CRC(e245c830) SHA1(315b8163731abe5c65bebd94884431e99253740b) )
	ROM_LOAD16_BYTE( "ci______.4_7", 0x060000, 0x010000, CRC(169d6ac5) SHA1(7d9e63d9ab29fbace754e8c46d4e80b1ab2d422d) )
	ROM_LOAD16_BYTE( "ci______.4_8", 0x060001, 0x010000, CRC(12de2ae0) SHA1(734207fffd918f3a0890ec6a2d442efb428adbaa) )
	ROM_LOAD16_BYTE( "ci______.4_9", 0x080000, 0x010000, CRC(f1f9987f) SHA1(0a4b5fa61e237e1e209301a07af2ad1e9fedcc35) )
	ROM_LOAD16_BYTE( "ci______.4_a", 0x080001, 0x010000, CRC(4747cb48) SHA1(ac33d318f6fff67c8a2f7d47c0ee0bcddfc2af8e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinf3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ciixea__.3_0", 0x0000, 0x010000, CRC(252f43e5) SHA1(5443a53a50fb27c3e98fa4612b4ba476cc946662) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD( "release3_video_roms", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4cshinf3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ciixead_.3_0", 0x0000, 0x010000, CRC(6e8aa444) SHA1(63cecb83aa8e4f94b16d6c2ded8dbe8aa98d88a8) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD( "release3_video_roms", 0x000000, 0x010000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4dbltak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dt_sja_n.4_0", 0x0000, 0x010000, CRC(c6578f37) SHA1(e3fedb25a420bc8565fc07fa6a82ce1d584a7fd1) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "dt______.4_1", 0x000001, 0x080000, CRC(786aaee8) SHA1(309b15bbc597891407d384ded0bc246c5aa50106) )
	ROM_LOAD16_BYTE( "dt______.4_2", 0x000000, 0x080000, CRC(0b5024ae) SHA1(4385b06021d349c8ee624afc1736666f94152a85) )
	ROM_LOAD16_BYTE( "dt______.4_3", 0x100001, 0x080000, CRC(b715bff9) SHA1(be8ef30b50c235e78a03ea83eadd7541ad96f7a1) )
	ROM_LOAD16_BYTE( "dt______.4_4", 0x100000, 0x080000, CRC(f41f2566) SHA1(e18e019bb04003e0fdce3a3051da0c618bdaef3d) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4dbltaka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dt_sjadn.4_0", 0x0000, 0x010000, CRC(8df26896) SHA1(6e19e56ba376c063e207b217a120f95041785b0f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "dt______.4_1", 0x000001, 0x080000, CRC(786aaee8) SHA1(309b15bbc597891407d384ded0bc246c5aa50106) )
	ROM_LOAD16_BYTE( "dt______.4_2", 0x000000, 0x080000, CRC(0b5024ae) SHA1(4385b06021d349c8ee624afc1736666f94152a85) )
	ROM_LOAD16_BYTE( "dt______.4_3", 0x100001, 0x080000, CRC(b715bff9) SHA1(be8ef30b50c235e78a03ea83eadd7541ad96f7a1) )
	ROM_LOAD16_BYTE( "dt______.4_4", 0x100000, 0x080000, CRC(f41f2566) SHA1(e18e019bb04003e0fdce3a3051da0c618bdaef3d) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4dbltakb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dt_sjsbn.4_0", 0x0000, 0x010000, CRC(c5358061) SHA1(aaf0b20175b63bb06524edbcb0b2e8a2b3ca7df2) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "dt______.4_1", 0x000001, 0x080000, CRC(786aaee8) SHA1(309b15bbc597891407d384ded0bc246c5aa50106) )
	ROM_LOAD16_BYTE( "dt______.4_2", 0x000000, 0x080000, CRC(0b5024ae) SHA1(4385b06021d349c8ee624afc1736666f94152a85) )
	ROM_LOAD16_BYTE( "dt______.4_3", 0x100001, 0x080000, CRC(b715bff9) SHA1(be8ef30b50c235e78a03ea83eadd7541ad96f7a1) )
	ROM_LOAD16_BYTE( "dt______.4_4", 0x100000, 0x080000, CRC(f41f2566) SHA1(e18e019bb04003e0fdce3a3051da0c618bdaef3d) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4gldrsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_27s__.860", 0x0000, 0x010000, CRC(88dd4ff9) SHA1(8edfbb622cd542828f5184f92b357dd9094c2cb0) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrsha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_20a__.8_0", 0x0000, 0x010000, CRC(57332f87) SHA1(b340107d8679c0a39887e26c7ceddd0e4bbf9047) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_20a_s.8_0", 0x0000, 0x010000, CRC(c1d201dd) SHA1(11d2f2a9022aba402667621d91d29d5a47983d9f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_20ab_.8_0", 0x0000, 0x010000, CRC(a1b7b814) SHA1(5c03e4d6754429b97590e644106d0c963ad46c53) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_20bg_.8_0", 0x0000, 0x010000, CRC(2b808fcf) SHA1(30394973e0cd4f265dbcd9b4e836a67e0894ae04) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_20bgs.8_0", 0x0000, 0x010000, CRC(bd61a195) SHA1(9775e45c4c9dfbbe655f27e4c5cf4bb86f9a8b38) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_20bt_.8_0", 0x0000, 0x010000, CRC(9dcb82cc) SHA1(8283ddaccb4fefbb056883c18165ec52c3df494f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_20bts.8_0", 0x0000, 0x010000, CRC(0b2aac96) SHA1(115643b3b746ae97f1aa8a5f715b8ff5ec05bd83) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_20s__.8_0", 0x0000, 0x010000, CRC(a2d5b742) SHA1(1f6d58794226a09609b07d1b66642fe86e808f2e) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_20sb_.8_0", 0x0000, 0x010000, CRC(545120d1) SHA1(76bcd72c2ffb316ee8cafc6426615f297fe2e845) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_20sbs.8_0", 0x0000, 0x010000, CRC(c2b00e8b) SHA1(71e4a926de3fe90a38407c6bbc59fdf420260cef) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_27sd_.860", 0x0000, 0x010000, CRC(c378a858) SHA1(53f0d357daeb0b7a6bdf9a4bbf852b39bd4a337f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_37s__.860", 0x0000, 0x010000, CRC(07cf24d7) SHA1(305f1f46b7082a25efd32c2960db5961fba249d0) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_37sd_.860", 0x0000, 0x010000, CRC(4c6ac376) SHA1(9d4d39654638e6adad64ab13bc7b145c93278653) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_x7s__.860", 0x0000, 0x010000, CRC(b8617832) SHA1(8a3c8de816a39f3bd0d5ba012e65c690468e5574) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrsho )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_x7sd_.860", 0x0000, 0x010000, CRC(f3c49f93) SHA1(bdd9de7be43dce45e435f2fbc7dd770089406dec) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_xea__.8_0", 0x0000, 0x010000, CRC(678f184c) SHA1(36738f6608fd09e78ae158d0cbcd827d3ae5bb07) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_xea_s.8_0", 0x0000, 0x010000, CRC(f16e3616) SHA1(a4e46bc464201c8552ec6cb7253228ee15f73f74) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4gldrshr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_xead_.8_0", 0x0000, 0x010000, CRC(2c2affed) SHA1(893a819ec8bb99d08eaace47487f22b6edc3f363) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "go_xeads.8_0", 0x0000, 0x010000, CRC(bacbd1b7) SHA1(3dee2487f91a815fa450955cae3afb0a6980921d) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrsht )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goi20s__.8_0", 0x0000, 0x010000, CRC(c3add8d4) SHA1(1b9bd696818eaa57805b5efcf21b5a3a52513d87) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrshu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goixes__.8_0", 0x0000, 0x010000, CRC(f311ef1f) SHA1(22b81313e62a3cd36d97fc6e39b6a3815eb5a7f3) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go______.8_1", 0x000001, 0x080000, CRC(42700f68) SHA1(3050e790292c8afcc0e27e01dffa22c46d97bcc4) )
	ROM_LOAD16_BYTE( "go______.8_2", 0x000000, 0x080000, CRC(a5d3c42e) SHA1(1398d5c8d1402a1dbf7910d00e6201f413dbd898) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4gldrsh3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goldrush.bin", 0x0000, 0x010000, CRC(459834db) SHA1(dac9945553567c6bbc4e8ca9054a0b5448bc19aa) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "go_3.1.bin", 0x000000, 0x080000, CRC(5edc8226) SHA1(c231978be89db23c1b1d38307510ef7e2a278492) )
	ROM_LOAD16_BYTE( "go_3.2.bin", 0x000001, 0x080000, CRC(95c10e74) SHA1(73b230e2281d4e2a564f070c752479af2af32757) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4mdicee )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_22sd_.8_0", 0x0000, 0x010000, CRC(ac8ea8cd) SHA1(31cc328aa803e232a57572fbc9ea6ac90177ad37) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdice )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_37s__.8_0", 0x0000, 0x010000, CRC(bcd47ec0) SHA1(2f32ae16eada13a826cde1162a56f43079f983fb) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_02a__.8_0", 0x0000, 0x010000, CRC(35ed5c15) SHA1(86409c70d7ba5b7a587753abf8180e424d48cc08) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdiceb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_12a__.8_0", 0x0000, 0x010000, CRC(267d19cb) SHA1(a386abfc3296cda0f22f2f9f33902d19a99af16f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_16a__.8_0", 0x0000, 0x010000, CRC(cf218b9c) SHA1(78798e9fcb8260e6c27ab363bcea0d30f8e01ab5) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdiced )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_22a__.8_0", 0x0000, 0x010000, CRC(12cdd7a9) SHA1(457a10f02e3c2093f247e2332612a5200ebf3884) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END




ROM_START( v4mdicef )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_26a__.8_0", 0x0000, 0x010000, CRC(fb9145fe) SHA1(8173755dd82b49a730858ccec27fbc2a6b0c416a) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdiceg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_26a_s.8_0", 0x0000, 0x010000, CRC(6d706ba4) SHA1(0c1323b84a247454d4272ac5be595f453b004037) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdiceh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_26ad_.8_0", 0x0000, 0x010000, CRC(b034a25f) SHA1(70a534b07584f86b3428dbc86b2e9278989b9d0e) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicei )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_26s__.8_0", 0x0000, 0x010000, CRC(0e77dd3b) SHA1(0989d08d6119c36841697974983397d6327d7a3e) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicej )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_26sb_.8_0", 0x0000, 0x010000, CRC(f8f34aa8) SHA1(d5e238ebd62da2fad4d208b2e7ca71cfc3d044d9) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicek )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_26sbs.8_0", 0x0000, 0x010000, CRC(6e1264f2) SHA1(a85e257958e94623bfc63107d3b483a19bc80f10) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_26sd_.8_0", 0x0000, 0x010000, CRC(45d23a9a) SHA1(0a825f50a2e3ddfaad710b9dd4e3740f0b2480b6) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_26sk_.8_0", 0x0000, 0x010000, CRC(b356ad09) SHA1(06e97be819203d183d10c56fa447b57b7fd5d594) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicen )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_27a__.8_0", 0x0000, 0x010000, CRC(5aa2a3db) SHA1(e85a982d2ee515a53956ef457b52f3368da038c7) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdiceo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_27ad_.8_0", 0x0000, 0x010000, CRC(1107447a) SHA1(a5803004b22564611d6073bc6ead88fca4318ce9) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicep )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_27s__.8_0", 0x0000, 0x010000, CRC(af443b1e) SHA1(e13b5880f0abf173596bbd7b029221a35bdc0c6c) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdiceq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_27sb_.8_0", 0x0000, 0x010000, CRC(59c0ac8d) SHA1(5abf26e90e0a63cfb47a3a4ee9ef0bbc56123022) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicer )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_27sd_.8_0", 0x0000, 0x010000, CRC(e4e1dcbf) SHA1(ed0ff22f106350ec614e4ec37f14c1c51468bb66) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdices )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_37a__.8_0", 0x0000, 0x010000, CRC(4932e605) SHA1(6db4ce36708611938ef58586af976b12b45b96fe) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicet )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_37ad_.8_0", 0x0000, 0x010000, CRC(029701a4) SHA1(88e56394664b807812d2e47e96f8f1280beed6d9) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdiceu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_37sb_.8_0", 0x0000, 0x010000, CRC(4a50e953) SHA1(d8b1751eafa6c853f0d054e56df8c9d80e5a00e6) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicev )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_37sd_.8_0", 0x0000, 0x010000, CRC(f7719961) SHA1(6c1a26878adf689cf3aa259356239f364848f681) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdicew )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mdi25a__.8_0", 0x0000, 0x010000, CRC(c3b469d0) SHA1(00cd85bd9acd477f2ab4baf83e7bb10d763b1e93) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md______.8_3", 0x100001, 0x080000, CRC(cde34cd1) SHA1(7874fa070e52e6c34b770aee5bfec522eb3d72c9) )
	ROM_LOAD16_BYTE( "md______.8_4", 0x100000, 0x080000, CRC(39bc1267) SHA1(853e047406fed3c12f55a2e032e8c3d8188da182) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END








ROM_START( v4mdice5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "release5_mpu4.rom", 0x0000, 0x010000, NO_DUMP )
	ROM_LOAD( "mdmpu4.bin", 0x0000, 0x010000, CRC(8e712a33) SHA1(e821167c825b151bcde5eb9c63e7d2da04e1166a) )

	ROM_REGION( 0x800000, "video", 0 ) // release 5 of Miami Dice based on internal strings
	ROM_LOAD16_BYTE( "mdv58p1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "mdv58p2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "mdv58p3", 0x100001, 0x080000, CRC(0d907e37) SHA1(b6ad78a4a7bc877d2152907df2317621f00bdc1c) )
	ROM_LOAD16_BYTE( "mdv58p4", 0x100000, 0x080000, CRC(2e21c249) SHA1(d5192339313a8dd234cb164ca0094d9a7b64ccc2) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END

ROM_START( v4mdice6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md_26a__.6_0", 0x0000, 0x010000, CRC(1902067a) SHA1(ca46375ad859758bd3182653f2614cea47e46e50) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "md______.8_1", 0x000001, 0x080000, CRC(3f3fa0d2) SHA1(bb52111bcea5cd404d1e7adf0f3ebca596a251ac) )
	ROM_LOAD16_BYTE( "md______.8_2", 0x000000, 0x080000, CRC(14bb6b48) SHA1(97025f0899c325d28ac75c54e81fd425b5002064) )
	ROM_LOAD16_BYTE( "md_6_30", 0x100001, 0x080000, CRC(c1526309) SHA1(c6961813310a3873540c9174db3c7ce2347620d5) )
	ROM_LOAD16_BYTE( "md_6_34", 0x100000, 0x080000, CRC(f6b8cc2f) SHA1(d1022b4a8ab3266dab5401127610c864e6e40a7f) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mdsnda", 0x000000, 0x080000, CRC(18651603) SHA1(c6f7557a82cb49f3f001b43250129d10f4f6ab5a) )
	ROM_LOAD( "mdsndb", 0x080000, 0x080000, CRC(2233d677) SHA1(a787dc0bafa310df9467e4b8166274288fe94b4c) )
ROM_END


	/* Some roms were simply in a set marked as Monte Carlo, but the 2 letter code mn) is the same */
	/* The roms were a mixture of
	    VIDEO 5 - MONTE CARLO OR BUST!
	    others contain
	    NM4 Monte Carlo on Options4

	    is the latter just an later revision, or a different game? (date suggests it isn't earlier)

	*/

ROM_START( v4monte )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mnixes__.d_0", 0x0000, 0x010000, CRC(44967b33) SHA1(92d35d1b0edcc2eef1062468722c80ef8208b437) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcop3vd",  0x100001, 0x080000, CRC(721e9ad1) SHA1(fb926debd57301c9c0c3ecb9bb1ac36b0b60ee40) )
	ROM_LOAD16_BYTE( "mcop4vd",  0x100000, 0x080000, CRC(6eba1107) SHA1(c696b620781782c3b4045fe3550ab8e7e905661d) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mcob2025", 0x0000, 0x010000, CRC(0fe6ec1e) SHA1(de5b7edb40b9ab3fc9111eb83061d55ce569afdd) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcop3vd",  0x100001, 0x080000, CRC(721e9ad1) SHA1(fb926debd57301c9c0c3ecb9bb1ac36b0b60ee40) )
	ROM_LOAD16_BYTE( "mcop4vd",  0x100000, 0x080000, CRC(6eba1107) SHA1(c696b620781782c3b4045fe3550ab8e7e905661d) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mcobvd-1", 0x0000, 0x010000, CRC(e0b4bbab) SHA1(bd806a286464d36ff0cbff214ed60ccf81fd2db9) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcop3vd",  0x100001, 0x080000, CRC(721e9ad1) SHA1(fb926debd57301c9c0c3ecb9bb1ac36b0b60ee40) )
	ROM_LOAD16_BYTE( "mcop4vd",  0x100000, 0x080000, CRC(6eba1107) SHA1(c696b620781782c3b4045fe3550ab8e7e905661d) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monted )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mcovd10", 0x0000, 0x010000, CRC(3f5adbd5) SHA1(f1876afd345e8398c1dc00b63be7d5a91f57519f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcop3vd",  0x100001, 0x080000, CRC(721e9ad1) SHA1(fb926debd57301c9c0c3ecb9bb1ac36b0b60ee40) )
	ROM_LOAD16_BYTE( "mcop4vd",  0x100000, 0x080000, CRC(6eba1107) SHA1(c696b620781782c3b4045fe3550ab8e7e905661d) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montee )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xes__.d_0", 0x0000, 0x010000, CRC(25ee14a5) SHA1(2eba1b5b68a9f7ad6aacbdf6b93f6b4569164258) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcop3vd",  0x100001, 0x080000, CRC(721e9ad1) SHA1(fb926debd57301c9c0c3ecb9bb1ac36b0b60ee40) )
	ROM_LOAD16_BYTE( "mcop4vd",  0x100000, 0x080000, CRC(6eba1107) SHA1(c696b620781782c3b4045fe3550ab8e7e905661d) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_37s__.d60", 0x0000, 0x010000, CRC(b048b0fb) SHA1(277622362d4d31b3e6386b27d1e043a88afea0ab) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcop3vd",  0x100001, 0x080000, CRC(721e9ad1) SHA1(fb926debd57301c9c0c3ecb9bb1ac36b0b60ee40) )
	ROM_LOAD16_BYTE( "mcop4vd",  0x100000, 0x080000, CRC(6eba1107) SHA1(c696b620781782c3b4045fe3550ab8e7e905661d) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

//---- these sets work with either the release b, or release 5 '3 & 4' 68k pair
//     v4monteb, v4monteba, v4montebb are set to 'release b' while v4monteba, v4monte5a, v4monte5b are set to 'release 5'
//     they require the 'mn_9' roms to not have corrupt text, so those ROMs despite the name are not for 'release 9'
//     maybe one of these 68k pairs is hacked? investigate

ROM_START( v4monteb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mcob8ac", 0x0000, 0x010000, CRC(6dceb28f) SHA1(dc3daee15c25470501ab11e7b34cfef7edf302d4) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mn_9.1.bin", 0x000001, 0x080000, CRC(c0a89d0b) SHA1(22259724119bd1a61b45ba068df61f0cad3b6d17) )
	ROM_LOAD16_BYTE( "mn_9.2.bin", 0x000000, 0x080000, CRC(308a0f80) SHA1(65f5b9286a0300f3f59a16469ffd247af50c1f07) )
	ROM_LOAD16_BYTE( "mn_b3.releaseb.lo", 0x100001, 0x080000, CRC(b6de7ca1) SHA1(944e6c6ee20d187148c7cd4b20119422663780fd) )
	ROM_LOAD16_BYTE( "mn_b4.releaseb.hi", 0x100000, 0x080000, CRC(5b6ff013) SHA1(ea08978ad469a521a6080fb6ab12033c31134a9d) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mcobarc", 0x0000, 0x010000, CRC(f95045dc) SHA1(463af12feed6dfe5f5e23d584c4eac121672918a) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mn_9.1.bin", 0x000001, 0x080000, CRC(c0a89d0b) SHA1(22259724119bd1a61b45ba068df61f0cad3b6d17) )
	ROM_LOAD16_BYTE( "mn_9.2.bin", 0x000000, 0x080000, CRC(308a0f80) SHA1(65f5b9286a0300f3f59a16469ffd247af50c1f07) )
	ROM_LOAD16_BYTE( "mn_b3.releaseb.lo", 0x100001, 0x080000, CRC(b6de7ca1) SHA1(944e6c6ee20d187148c7cd4b20119422663780fd) )
	ROM_LOAD16_BYTE( "mn_b4.releaseb.hi", 0x100000, 0x080000, CRC(5b6ff013) SHA1(ea08978ad469a521a6080fb6ab12033c31134a9d) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montebb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mnb51020", 0x0000, 0x010000, CRC(c9ec7217) SHA1(006a70fb4050d726ae80678dc49afbd8c2c0c124) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mn_9.1.bin", 0x000001, 0x080000, CRC(c0a89d0b) SHA1(22259724119bd1a61b45ba068df61f0cad3b6d17) )
	ROM_LOAD16_BYTE( "mn_9.2.bin", 0x000000, 0x080000, CRC(308a0f80) SHA1(65f5b9286a0300f3f59a16469ffd247af50c1f07) )
	ROM_LOAD16_BYTE( "mn_b3.releaseb.lo", 0x100001, 0x080000, CRC(b6de7ca1) SHA1(944e6c6ee20d187148c7cd4b20119422663780fd) )
	ROM_LOAD16_BYTE( "mn_b4.releaseb.hi", 0x100000, 0x080000, CRC(5b6ff013) SHA1(ea08978ad469a521a6080fb6ab12033c31134a9d) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mcob8ac", 0x0000, 0x010000, CRC(6dceb28f) SHA1(dc3daee15c25470501ab11e7b34cfef7edf302d4) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mn_9.1.bin", 0x000001, 0x080000, CRC(c0a89d0b) SHA1(22259724119bd1a61b45ba068df61f0cad3b6d17) )
	ROM_LOAD16_BYTE( "mn_9.2.bin", 0x000000, 0x080000, CRC(308a0f80) SHA1(65f5b9286a0300f3f59a16469ffd247af50c1f07) )
	ROM_LOAD16_BYTE( "mn_b3.release5.lo", 0x100001, 0x080000, CRC(a38cfb78) SHA1(3af87c03890bf02dc5bf222fab4ec1326c98ef94) )
	ROM_LOAD16_BYTE( "mn_b4.release5.hi", 0x100000, 0x080000, CRC(ae260cda) SHA1(7139f61c08d2c9f9fdc7314bd89776349c5c1b60) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte5a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mcobarc", 0x0000, 0x010000, CRC(f95045dc) SHA1(463af12feed6dfe5f5e23d584c4eac121672918a) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mn_9.1.bin", 0x000001, 0x080000, CRC(c0a89d0b) SHA1(22259724119bd1a61b45ba068df61f0cad3b6d17) )
	ROM_LOAD16_BYTE( "mn_9.2.bin", 0x000000, 0x080000, CRC(308a0f80) SHA1(65f5b9286a0300f3f59a16469ffd247af50c1f07) )
	ROM_LOAD16_BYTE( "mn_b3.release5.lo", 0x100001, 0x080000, CRC(a38cfb78) SHA1(3af87c03890bf02dc5bf222fab4ec1326c98ef94) )
	ROM_LOAD16_BYTE( "mn_b4.release5.hi", 0x100000, 0x080000, CRC(ae260cda) SHA1(7139f61c08d2c9f9fdc7314bd89776349c5c1b60) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END


ROM_START( v4monte5b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mnb51020", 0x0000, 0x010000, CRC(c9ec7217) SHA1(006a70fb4050d726ae80678dc49afbd8c2c0c124) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mn_9.1.bin", 0x000001, 0x080000, CRC(c0a89d0b) SHA1(22259724119bd1a61b45ba068df61f0cad3b6d17) )
	ROM_LOAD16_BYTE( "mn_9.2.bin", 0x000000, 0x080000, CRC(308a0f80) SHA1(65f5b9286a0300f3f59a16469ffd247af50c1f07) )
	ROM_LOAD16_BYTE( "mn_b3.release5.lo", 0x100001, 0x080000, CRC(a38cfb78) SHA1(3af87c03890bf02dc5bf222fab4ec1326c98ef94) )
	ROM_LOAD16_BYTE( "mn_b4.release5.hi", 0x100000, 0x080000, CRC(ae260cda) SHA1(7139f61c08d2c9f9fdc7314bd89776349c5c1b60) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

//---- no compatible video ROMs

ROM_START( v4montek )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_", 0x0000, 0x010000, CRC(768bcc18) SHA1(d895f5207145a9d7b93b6b88fefc5be5ccf3eb72) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END


ROM_START( v4montel )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_x7s__.e60", 0x0000, 0x010000, CRC(a883207a) SHA1(948990c254eaaee7ab7c4d832290069170b2da78) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xea__.e_0", 0x0000, 0x010000, CRC(776d4004) SHA1(4bc0f5688f6fb39e9398a330317f281c4c7cb1cd) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monten )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xea_s.e_0", 0x0000, 0x010000, CRC(e18c6e5e) SHA1(b008129c50b0b9031f1324fb3feac5ac50695e87) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xead_.e_0", 0x0000, 0x010000, CRC(3cc8a7a5) SHA1(6d00e4ff02aee210d940d53d1bc0a3c8c57a4f76) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montep )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xes__.e_0", 0x0000, 0x010000, CRC(828bd8c1) SHA1(d6411579ec5f5b1a7f83d27d9e222d17e355441f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xes_s.e_0", 0x0000, 0x010000, CRC(146af69b) SHA1(ce261fe2e93cb91a46bf67fcda504b168c84fa27) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monter )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xesb_.e_0", 0x0000, 0x010000, CRC(740f4f52) SHA1(cd811a90abd07d5cea466a631ffdc1329a5d3cff) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montes )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xesd_.e_0", 0x0000, 0x010000, CRC(c92e3f60) SHA1(2a07c11bb9e212b1cb049f087b618ec7f7e9e8bc) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montet )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xesk_.e_0", 0x0000, 0x010000, CRC(3faaa8f3) SHA1(6ddaf4ab3128ed16ad9abd9e77556710f88ec6f0) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mni20s__.e_0", 0x0000, 0x010000, CRC(d34f809c) SHA1(6c51c2b52e06d2b115737690bf22dd0182f73fc2) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montev )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mnixes__.e_0", 0x0000, 0x010000, CRC(e3f3b757) SHA1(f7a285730b7d76de4253c234be652021dc2bf860) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montew )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_20a__.e_0", 0x0000, 0x010000, CRC(47d177cf) SHA1(7d3e59bc399b731880320899169732b20932c39d) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montex )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_20sbs.e_0", 0x0000, 0x010000, CRC(d25256c3) SHA1(b359307021604e405b4bc50e38aa51f7675f73a0) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montey )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_27s__.e60", 0x0000, 0x010000, CRC(983f17b1) SHA1(e57104ff8fc136db11fc81fc2160d1bc6961f362) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montez )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_27sd_.e60", 0x0000, 0x010000, CRC(d39af010) SHA1(71ceb3ab370518b0e367dc93db9101efa4710067) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_20s_s.e_0", 0x0000, 0x010000, CRC(24d6c150) SHA1(4671098dc24185837ace42acf224baabb2581906) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_20sb_.e_0", 0x0000, 0x010000, CRC(44b37899) SHA1(9de7ba3284e6f091d1ae29f7fc861e1da3879daf) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_20sd_.e_0", 0x0000, 0x010000, CRC(f99208ab) SHA1(1ac02645712bd9c2d575b42ea8641cec03347507) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montead )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_20sk_.e_0", 0x0000, 0x010000, CRC(0f169f38) SHA1(66b95512c3f6bfdf0f6824b5a97b12ac55c56c50) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteae )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xesbs.e_0", 0x0000, 0x010000, CRC(e2ee6108) SHA1(838f724c49f1f737dae02d10dd6633994c6480fc) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteaf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_37s__.e60", 0x0000, 0x010000, CRC(172d7c9f) SHA1(303bd7019b91c782255befea7a5e530030a6461b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteag )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_37sd_.e60", 0x0000, 0x010000, CRC(5c889b3e) SHA1(c55c395fc7aa8f6c0ab4f74acf41f04017da8330) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteah )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_x7sd_.e60", 0x0000, 0x010000, CRC(e326c7db) SHA1(d09409b4edca673675bd3b3653de0608b463ea7c) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteai )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_x7s__.f60", 0x0000, 0x010000, CRC(866a18ee) SHA1(b21bc50ef84e5e836da133caffb14b2ef680bdcd) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteaj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_x7sd_.f60", 0x0000, 0x010000, CRC(cdcfff4f) SHA1(9e442cd46684efbdf186df48a1143a2b23f796e7) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_20s__.f_0", 0x0000, 0x010000, CRC(9cded79e) SHA1(0702e57e04bfe860ec63122fe29a1ff920884c49) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_27s__.f60", 0x0000, 0x010000, CRC(b6d62f25) SHA1(1d5f8c40ae12508bb146638706946da9107bacc1) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_27sd_.f60", 0x0000, 0x010000, CRC(fd73c884) SHA1(32a972cf552af2d9b8c46de3d59b1adee9b4286d) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4montean )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_37s__.f60", 0x0000, 0x010000, CRC(39c4440b) SHA1(e54c480420313f3e5d3d67ffc0b5827458f20c9c) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteao )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_37sd_.f60", 0x0000, 0x010000, CRC(7261a3aa) SHA1(08d289b5f2493d058db0779ae7858c3ea9ef7c85) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

// good again

ROM_START( v4monte9 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mni20s__.940", 0x0000, 0x010000, CRC(13ac85ab) SHA1(f1d65613787dc1312ad68bd49dcb5a9a8bc1093c) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mnixes__.940", 0x0000, 0x010000, CRC(2310b260) SHA1(309d5e34c811866cbef634032fddc53ca0b78966) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END


ROM_START( v4monte9b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xesb_.940", 0x0000, 0x010000, CRC(b4ec4a65) SHA1(e360704a434dde21c6fb794f2b75ecd4b8cea2b5) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xea__.940", 0x0000, 0x010000, CRC(b78e4533) SHA1(5084f5434c0ee727e6b15422b5fc5d33a7e732d8) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xeab_.940", 0x0000, 0x010000, CRC(410ad2a0) SHA1(0f781bc89837e7def08f71589834ccbdadca6409) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9e )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_xes__.940", 0x0000, 0x010000, CRC(4268ddf6) SHA1(e81cc5f429ae8c3e868ac6301b60b7a0acd2d190) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9f )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_x7s__.9a0", 0x0000, 0x010000, CRC(6860254d) SHA1(a0247ec07f2ec13fadc543986d27b5763d799f9b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9g )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_20ab_.940", 0x0000, 0x010000, CRC(71b6e56b) SHA1(b87b488398b5554b63324b3c8d47f5ed2210026d) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9h )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_20a__.940", 0x0000, 0x010000, CRC(873272f8) SHA1(7097987c175fc441a6ae6f1a7a0dec3ef5bf53a5) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9i )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_20sb_.940", 0x0000, 0x010000, CRC(84507dae) SHA1(1a2fb9514cbcb44f324574fc913a7f1f5d321e53) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9j )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_27s__.9a0", 0x0000, 0x010000, CRC(58dc1286) SHA1(d90fe58b612d438c082fb7dac8db82bec3faabf9) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_27sd_.9a0", 0x0000, 0x010000, CRC(1379f527) SHA1(e9b84b8844be7c26f2e7db731c48b0c6d2bfd689) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9l )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_37s__.9a0", 0x0000, 0x010000, CRC(d7ce79a8) SHA1(6daa848f79ad65bd2567afbacee2dcbb60317f29) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9m )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_37sd_.9a0", 0x0000, 0x010000, CRC(9c6b9e09) SHA1(1b0d35c2ba906b1e478e58d66fc5432cdf05f36b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9n )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_x7sd_.9a0", 0x0000, 0x010000, CRC(23c5c2ec) SHA1(42568dd981a65c34ae7e25c201ccc90c109bf32e) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monte9o )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mn_20s__.940", 0x0000, 0x010000, CRC(72d4ea3d) SHA1(bf2d3279550351069ccc04064dc3b6966b11ee12) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mcobo4p1", 0x000001, 0x080000, CRC(aaa594f0) SHA1(2bcb13d8b93911a69c181d6f0be43397baf8cbc8) )
	ROM_LOAD16_BYTE( "mcobo4p2", 0x000000, 0x080000, CRC(ab94c22a) SHA1(a8a0ed992c0b95fb763aea37f78c8d7a53732509) )
	ROM_LOAD16_BYTE( "mcobo4p3", 0x100001, 0x080000, CRC(ebe851df) SHA1(61d37a7f91480592da6f5b6ee7ef4b6097ee5c65) )
	ROM_LOAD16_BYTE( "mcobo4p4", 0x100000, 0x080000, CRC(49b0cfd7) SHA1(51fe74371bdac3c507a04aa9faeb522640d1cdf7) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END


ROM_START( v4montezz )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "mpu4.rom", 0x0000, 0x010000, NO_DUMP )

	ROM_REGION( 0x800000, "video", 0 )
	// this seems to be a loose video ROM from an otherwise undumped set? investigate, might belong to something else entirely.
	ROM_LOAD( "mn______.f_1", 0x0000, 0x080000, CRC(1a81b3fb) SHA1(bbf0fe7e48404962a2f2120734efe71dc1eed64c) ) // unmatched rom? (significant changes)

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mcosnda.bin", 0x000000, 0x080000, CRC(801ea236) SHA1(531841d6a4d67f502e93f8d74f3b247ccc46208f) )
	ROM_LOAD( "mcosndb.bin", 0x080000, 0x080000, CRC(fcbad433) SHA1(a8cd32ca5a17e3c35701a7eac3e9ef741aa04105) )
ROM_END

ROM_START( v4monteger )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mnd-a0_interface.bin", 0x0000, 0x010000, CRC(157e7ee4) SHA1(7a2d2caefd6ff609b8059d0ed0fd7ef94d8d36bc) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "mnd-a2_vid.bin", 0x000000, 0x080000, CRC(220f3a14) SHA1(fdfdc41c62330d77735d1840a4a7d429b3257d07) )
	ROM_LOAD16_BYTE( "mnd-a1_vid.bin", 0x000001, 0x080000, CRC(b7b28d00) SHA1(0ae4bc759472f58cb738d7e7c8713a54f4e13686) )
	ROM_LOAD16_BYTE( "mnd-a4_vid.bin", 0x100000, 0x080000, CRC(5fa9d451) SHA1(539438d237b869e97176d031f0014f3e33374eed) )
	ROM_LOAD16_BYTE( "mnd-a3_vid.bin", 0x100001, 0x080000, CRC(5e3a95a4) SHA1(305a7f8b1c5072d86d6f381501886587a2e186ea) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "mnd-1a-snd.bin", 0x000000, 0x080000, CRC(98bcf6fb) SHA1(b2c0d305f64be10f5ff40518ebb1b66c44559578) )
	ROM_LOAD( "mnd-1b-snd.bin", 0x080000, 0x080000, CRC(df2118b4) SHA1(6126baff9dfef7c573e3f77847ea58bdc242fdc2) )
ROM_END


ROM_START( v4ovrmn3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "o3_20s__.4_0", 0x0000, 0x010000, CRC(fab0af50) SHA1(de57130763965bcea42dd17c1094a2d1c363d328) )

	ROM_REGION( 0x800000, "video", 0 ) // first 2 ROMS match Bubbly Bonk ??
	ROM_LOAD16_BYTE( "o3______.4_1", 0x000000, 0x010000,  CRC(1e85dd5d) SHA1(0f3c35d9f75d3495e2d0cc1fcf0e96dcbbeeacc8)) // == bu______.4_1
	ROM_LOAD16_BYTE( "o3______.4_2", 0x000001, 0x010000,  CRC(989db70a) SHA1(e44f8900392db51cc3f0a5bf0391bde0d71b9878)) // == bu______.4_2
	ROM_LOAD16_BYTE( "o3______.4_3", 0x020000, 0x010000, CRC(e6adda98) SHA1(176df97e3f0b22531a4c7b30e951bab0bc6403ad) )
	ROM_LOAD16_BYTE( "o3______.4_4", 0x020001, 0x010000, CRC(b044faef) SHA1(e78f44fa32d9b8428e8ba9f19d95c0e32b8ec1e8) )
	ROM_LOAD16_BYTE( "o3______.4_5", 0x040000, 0x010000, CRC(5bcc553d) SHA1(dd073cccce6fca693b772f6739132382b237bc67) )
	ROM_LOAD16_BYTE( "o3______.4_6", 0x040001, 0x010000, CRC(ea616d69) SHA1(e606f5e999a93f8505f529c95fe209c6452934c3) )
	ROM_LOAD16_BYTE( "o3______.4_7", 0x060000, 0x010000, CRC(e4dc300d) SHA1(8631bd78abfebcfa75f60a891b9709d84c8124e7) )
	ROM_LOAD16_BYTE( "o3______.4_8", 0x060001, 0x010000, CRC(c51ffb5f) SHA1(78b0d8fe04d419d52b68549a61bdc51b3bbda50f) )
	ROM_LOAD16_BYTE( "o3______.4_9", 0x080000, 0x010000, CRC(6201a444) SHA1(a4a419fd94c571a85259f0f0092e1c99ef6b5797) )
	ROM_LOAD16_BYTE( "o3______.4_a", 0x080001, 0x010000, CRC(5b526937) SHA1(dd9de97ee48a157a26e8e70211819aed0a87921c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4ovrmn3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "o3_20sb_.4_0", 0x0000, 0x010000, CRC(0c3438c3) SHA1(dd0fc2db66d7bfb97f7f456c92654fc60c494e49) )

	ROM_REGION( 0x800000, "video", 0 ) // first 2 ROMS match Bubbly Bonk ??
	ROM_LOAD16_BYTE( "o3______.4_1", 0x000000, 0x010000,  CRC(1e85dd5d) SHA1(0f3c35d9f75d3495e2d0cc1fcf0e96dcbbeeacc8)) // == bu______.4_1
	ROM_LOAD16_BYTE( "o3______.4_2", 0x000001, 0x010000,  CRC(989db70a) SHA1(e44f8900392db51cc3f0a5bf0391bde0d71b9878)) // == bu______.4_2
	ROM_LOAD16_BYTE( "o3______.4_3", 0x020000, 0x010000, CRC(e6adda98) SHA1(176df97e3f0b22531a4c7b30e951bab0bc6403ad) )
	ROM_LOAD16_BYTE( "o3______.4_4", 0x020001, 0x010000, CRC(b044faef) SHA1(e78f44fa32d9b8428e8ba9f19d95c0e32b8ec1e8) )
	ROM_LOAD16_BYTE( "o3______.4_5", 0x040000, 0x010000, CRC(5bcc553d) SHA1(dd073cccce6fca693b772f6739132382b237bc67) )
	ROM_LOAD16_BYTE( "o3______.4_6", 0x040001, 0x010000, CRC(ea616d69) SHA1(e606f5e999a93f8505f529c95fe209c6452934c3) )
	ROM_LOAD16_BYTE( "o3______.4_7", 0x060000, 0x010000, CRC(e4dc300d) SHA1(8631bd78abfebcfa75f60a891b9709d84c8124e7) )
	ROM_LOAD16_BYTE( "o3______.4_8", 0x060001, 0x010000, CRC(c51ffb5f) SHA1(78b0d8fe04d419d52b68549a61bdc51b3bbda50f) )
	ROM_LOAD16_BYTE( "o3______.4_9", 0x080000, 0x010000, CRC(6201a444) SHA1(a4a419fd94c571a85259f0f0092e1c99ef6b5797) )
	ROM_LOAD16_BYTE( "o3______.4_a", 0x080001, 0x010000, CRC(5b526937) SHA1(dd9de97ee48a157a26e8e70211819aed0a87921c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4ovrmn3b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "o3_20sd_.4_0", 0x0000, 0x010000, CRC(b11548f1) SHA1(59585394c57a0441817d2bea424abe0bcf09f7ff) )

	ROM_REGION( 0x800000, "video", 0 ) // first 2 ROMS match Bubbly Bonk ??
	ROM_LOAD16_BYTE( "o3______.4_1", 0x000000, 0x010000,  CRC(1e85dd5d) SHA1(0f3c35d9f75d3495e2d0cc1fcf0e96dcbbeeacc8)) // == bu______.4_1
	ROM_LOAD16_BYTE( "o3______.4_2", 0x000001, 0x010000,  CRC(989db70a) SHA1(e44f8900392db51cc3f0a5bf0391bde0d71b9878)) // == bu______.4_2
	ROM_LOAD16_BYTE( "o3______.4_3", 0x020000, 0x010000, CRC(e6adda98) SHA1(176df97e3f0b22531a4c7b30e951bab0bc6403ad) )
	ROM_LOAD16_BYTE( "o3______.4_4", 0x020001, 0x010000, CRC(b044faef) SHA1(e78f44fa32d9b8428e8ba9f19d95c0e32b8ec1e8) )
	ROM_LOAD16_BYTE( "o3______.4_5", 0x040000, 0x010000, CRC(5bcc553d) SHA1(dd073cccce6fca693b772f6739132382b237bc67) )
	ROM_LOAD16_BYTE( "o3______.4_6", 0x040001, 0x010000, CRC(ea616d69) SHA1(e606f5e999a93f8505f529c95fe209c6452934c3) )
	ROM_LOAD16_BYTE( "o3______.4_7", 0x060000, 0x010000, CRC(e4dc300d) SHA1(8631bd78abfebcfa75f60a891b9709d84c8124e7) )
	ROM_LOAD16_BYTE( "o3______.4_8", 0x060001, 0x010000, CRC(c51ffb5f) SHA1(78b0d8fe04d419d52b68549a61bdc51b3bbda50f) )
	ROM_LOAD16_BYTE( "o3______.4_9", 0x080000, 0x010000, CRC(6201a444) SHA1(a4a419fd94c571a85259f0f0092e1c99ef6b5797) )
	ROM_LOAD16_BYTE( "o3______.4_a", 0x080001, 0x010000, CRC(5b526937) SHA1(dd9de97ee48a157a26e8e70211819aed0a87921c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4ovrmn3c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "o3_20sk_.4_0", 0x0000, 0x010000, CRC(4791df62) SHA1(c3d8ecf5f35a95aa5b2fd0a4085aa80b0b6d4447) )

	ROM_REGION( 0x800000, "video", 0 ) // first 2 ROMS match Bubbly Bonk ??
	ROM_LOAD16_BYTE( "o3______.4_1", 0x000000, 0x010000,  CRC(1e85dd5d) SHA1(0f3c35d9f75d3495e2d0cc1fcf0e96dcbbeeacc8)) // == bu______.4_1
	ROM_LOAD16_BYTE( "o3______.4_2", 0x000001, 0x010000,  CRC(989db70a) SHA1(e44f8900392db51cc3f0a5bf0391bde0d71b9878)) // == bu______.4_2
	ROM_LOAD16_BYTE( "o3______.4_3", 0x020000, 0x010000, CRC(e6adda98) SHA1(176df97e3f0b22531a4c7b30e951bab0bc6403ad) )
	ROM_LOAD16_BYTE( "o3______.4_4", 0x020001, 0x010000, CRC(b044faef) SHA1(e78f44fa32d9b8428e8ba9f19d95c0e32b8ec1e8) )
	ROM_LOAD16_BYTE( "o3______.4_5", 0x040000, 0x010000, CRC(5bcc553d) SHA1(dd073cccce6fca693b772f6739132382b237bc67) )
	ROM_LOAD16_BYTE( "o3______.4_6", 0x040001, 0x010000, CRC(ea616d69) SHA1(e606f5e999a93f8505f529c95fe209c6452934c3) )
	ROM_LOAD16_BYTE( "o3______.4_7", 0x060000, 0x010000, CRC(e4dc300d) SHA1(8631bd78abfebcfa75f60a891b9709d84c8124e7) )
	ROM_LOAD16_BYTE( "o3______.4_8", 0x060001, 0x010000, CRC(c51ffb5f) SHA1(78b0d8fe04d419d52b68549a61bdc51b3bbda50f) )
	ROM_LOAD16_BYTE( "o3______.4_9", 0x080000, 0x010000, CRC(6201a444) SHA1(a4a419fd94c571a85259f0f0092e1c99ef6b5797) )
	ROM_LOAD16_BYTE( "o3______.4_a", 0x080001, 0x010000, CRC(5b526937) SHA1(dd9de97ee48a157a26e8e70211819aed0a87921c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4ovrmn3d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "o3i20s__.4_0", 0x0000, 0x010000, CRC(9bc8c0c6) SHA1(19ea9614ca2a9c28e1819d1451bc3117360f143f) )

	ROM_REGION( 0x800000, "video", 0 ) // first 2 ROMS match Bubbly Bonk ??
	ROM_LOAD16_BYTE( "o3______.4_1", 0x000000, 0x010000,  CRC(1e85dd5d) SHA1(0f3c35d9f75d3495e2d0cc1fcf0e96dcbbeeacc8)) // == bu______.4_1
	ROM_LOAD16_BYTE( "o3______.4_2", 0x000001, 0x010000,  CRC(989db70a) SHA1(e44f8900392db51cc3f0a5bf0391bde0d71b9878)) // == bu______.4_2
	ROM_LOAD16_BYTE( "o3______.4_3", 0x020000, 0x010000, CRC(e6adda98) SHA1(176df97e3f0b22531a4c7b30e951bab0bc6403ad) )
	ROM_LOAD16_BYTE( "o3______.4_4", 0x020001, 0x010000, CRC(b044faef) SHA1(e78f44fa32d9b8428e8ba9f19d95c0e32b8ec1e8) )
	ROM_LOAD16_BYTE( "o3______.4_5", 0x040000, 0x010000, CRC(5bcc553d) SHA1(dd073cccce6fca693b772f6739132382b237bc67) )
	ROM_LOAD16_BYTE( "o3______.4_6", 0x040001, 0x010000, CRC(ea616d69) SHA1(e606f5e999a93f8505f529c95fe209c6452934c3) )
	ROM_LOAD16_BYTE( "o3______.4_7", 0x060000, 0x010000, CRC(e4dc300d) SHA1(8631bd78abfebcfa75f60a891b9709d84c8124e7) )
	ROM_LOAD16_BYTE( "o3______.4_8", 0x060001, 0x010000, CRC(c51ffb5f) SHA1(78b0d8fe04d419d52b68549a61bdc51b3bbda50f) )
	ROM_LOAD16_BYTE( "o3______.4_9", 0x080000, 0x010000, CRC(6201a444) SHA1(a4a419fd94c571a85259f0f0092e1c99ef6b5797) )
	ROM_LOAD16_BYTE( "o3______.4_a", 0x080001, 0x010000, CRC(5b526937) SHA1(dd9de97ee48a157a26e8e70211819aed0a87921c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4pztet )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tc______.2_0", 0x0000, 0x010000, CRC(9a94ccaf) SHA1(42988a22c26f88fa07c1ab68a85f15bc3af0a71c) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tc______.2_1", 0x000000, 0x010000, CRC(048f30b5) SHA1(e6cf196cabe5c9b27a40942f547d50135d9633d0) )
	ROM_LOAD16_BYTE( "tc______.2_2", 0x000001, 0x010000, CRC(546719a6) SHA1(17a0fd552f4afa9d4eaeec903f3952f61af345d2) )
	ROM_LOAD16_BYTE( "tc______.2_3", 0x020000, 0x010000, CRC(6afa8148) SHA1(f4bfe41c9b9bb363c4fc1de616d641bea22248ba) )
	ROM_LOAD16_BYTE( "tc______.2_4", 0x020001, 0x010000, CRC(f7f99e42) SHA1(ca3b26fd911b8fc277e14dcdba428cb15288c995) )
	ROM_LOAD16_BYTE( "tc______.2_5", 0x040000, 0x010000, CRC(a2a50948) SHA1(57bf6c0738363da93ec6f379a23e706d1a6fc237) )
	ROM_LOAD16_BYTE( "tc______.2_6", 0x040001, 0x010000, CRC(de2146e4) SHA1(4e65c5d59d561d052834c9a0d139c286839fcf86) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4pzteta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tc____d_.2_0", 0x0000, 0x010000, CRC(d1312b0e) SHA1(d1ba8f49a0b30771f5ffbaaf3f8b6142965a8330) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tc______.2_1", 0x000000, 0x010000, CRC(048f30b5) SHA1(e6cf196cabe5c9b27a40942f547d50135d9633d0) )
	ROM_LOAD16_BYTE( "tc______.2_2", 0x000001, 0x010000, CRC(546719a6) SHA1(17a0fd552f4afa9d4eaeec903f3952f61af345d2) )
	ROM_LOAD16_BYTE( "tc______.2_3", 0x020000, 0x010000, CRC(6afa8148) SHA1(f4bfe41c9b9bb363c4fc1de616d641bea22248ba) )
	ROM_LOAD16_BYTE( "tc______.2_4", 0x020001, 0x010000, CRC(f7f99e42) SHA1(ca3b26fd911b8fc277e14dcdba428cb15288c995) )
	ROM_LOAD16_BYTE( "tc______.2_5", 0x040000, 0x010000, CRC(a2a50948) SHA1(57bf6c0738363da93ec6f379a23e706d1a6fc237) )
	ROM_LOAD16_BYTE( "tc______.2_6", 0x040001, 0x010000, CRC(de2146e4) SHA1(4e65c5d59d561d052834c9a0d139c286839fcf86) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4pztetb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tp______.2_0", 0x0000, 0x010000, CRC(17757b59) SHA1(013690047e2769c420a9422c662990a71e1bd09d) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tp______.2_1", 0x000000, 0x010000, CRC(91f3a03d) SHA1(c1eb60b32052cfe188cf465233a2fa0d0ca0a649) )
	ROM_LOAD16_BYTE( "tp______.2_2", 0x000001, 0x010000, CRC(f5fc5d3d) SHA1(d22f92769c0ad2c30887c60680ff1f984013bd02) )
	ROM_LOAD16_BYTE( "tp______.2_3", 0x020000, 0x010000, CRC(ef015d5e) SHA1(d7477743cbc0cc7c2e8cdf33a2c82c0425dd7d61) )
	ROM_LOAD16_BYTE( "tp______.2_4", 0x020001, 0x010000, CRC(401b5a50) SHA1(1b9bea05d4ba69d874f0067908bcfc19a5d8c6af) )
	ROM_LOAD16_BYTE( "tp______.2_5", 0x040000, 0x010000, CRC(e35cb24b) SHA1(a1c32c195b1d61a99b784c646ad78fa59b8270c4) )
	ROM_LOAD16_BYTE( "tp______.2_6", 0x040001, 0x010000, CRC(fecd48d0) SHA1(67ee3bee7aade5ec5fce1fcfe7ef3982264f1762) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4pztetc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tp____d_.2_0", 0x0000, 0x010000, CRC(5cd09cf8) SHA1(3c288169f9bd49affaaa4e1f5f0fdddf52f381a8) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "tp______.2_1", 0x000000, 0x010000, CRC(91f3a03d) SHA1(c1eb60b32052cfe188cf465233a2fa0d0ca0a649) )
	ROM_LOAD16_BYTE( "tp______.2_2", 0x000001, 0x010000, CRC(f5fc5d3d) SHA1(d22f92769c0ad2c30887c60680ff1f984013bd02) )
	ROM_LOAD16_BYTE( "tp______.2_3", 0x020000, 0x010000, CRC(ef015d5e) SHA1(d7477743cbc0cc7c2e8cdf33a2c82c0425dd7d61) )
	ROM_LOAD16_BYTE( "tp______.2_4", 0x020001, 0x010000, CRC(401b5a50) SHA1(1b9bea05d4ba69d874f0067908bcfc19a5d8c6af) )
	ROM_LOAD16_BYTE( "tp______.2_5", 0x040000, 0x010000, CRC(e35cb24b) SHA1(a1c32c195b1d61a99b784c646ad78fa59b8270c4) )
	ROM_LOAD16_BYTE( "tp______.2_6", 0x040001, 0x010000, CRC(fecd48d0) SHA1(67ee3bee7aade5ec5fce1fcfe7ef3982264f1762) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4rhmaz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rm_10___.4_0", 0x0000, 0x010000, CRC(5e9b282e) SHA1(d40744ae4cb36e49398f2f2e91274f9a1ecbe018) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rhmaza )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rm_10_b_.4_0", 0x0000, 0x010000, CRC(a81fbfbd) SHA1(24fda5a2eaa7291ad4869d2f3061798360c5e862) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rhmazb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rm_10_d_.4_0", 0x0000, 0x010000, CRC(153ecf8f) SHA1(a1d67c2298bbd008be69b7ae7b3b92669972e37f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rhmazc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rm_20___.4_0", 0x0000, 0x010000, CRC(8c5811a6) SHA1(2b51776e7dc78c322173708c6426b9eafde84870) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rhmazd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rm_20_b_.4_0", 0x0000, 0x010000, CRC(7adc8635) SHA1(8af3c8f332ca877f82940bc2a4e7abc4d37d6f48) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rhmaze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rm_20_bl.4_0", 0x0000, 0x010000, CRC(244c3e2c) SHA1(c764fe5a5ec74a4c4bb8bfea1bab2e8ae95cc4e7) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rhmazf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rm_20_d_.4_0", 0x0000, 0x010000, CRC(c7fdf607) SHA1(eab05365451d66e23163cbed433921bcbd415022) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rhmazg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rm_xc___.4_0", 0x0000, 0x010000, CRC(bce4266d) SHA1(018419adc7ee4af6c183868d1b7c3ef0cab8a837) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rhmazh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rm_xc_b_.4_0", 0x0000, 0x010000, CRC(4a60b1fe) SHA1(8effdbcf89aa942c434ebbb5e7127c357608d83f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rhmazi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rm_xc_d_.4_0", 0x0000, 0x010000, CRC(f741c1cc) SHA1(576e250319ea2269982cfd6b5ea68ae4a26f81f2) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rhmazj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rm_xc_k_.4_0", 0x0000, 0x010000, CRC(01c5565f) SHA1(293b7591cd73d12552db751629a60aa315433dbf) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rhmazk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rmixc___.4_0", 0x0000, 0x010000, CRC(dd9c49fb) SHA1(649eb1d5fd05698e60094fe7225413da4971d65b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "rm______.4_1", 0x000000, 0x010000, CRC(51af0692) SHA1(74e52d63adeaaa1f0949dad60f5d4ca1635d2261) )
	ROM_LOAD16_BYTE( "rm______.4_2", 0x000001, 0x010000, CRC(2c0971ee) SHA1(7ee22c7bf0e0dcc548537a6418d56c7088a699a0) )
	ROM_LOAD16_BYTE( "rm______.4_3", 0x020000, 0x010000, CRC(b7981930) SHA1(d4356dcbbfda7486a684d8c8d375648dd0f8e200) )
	ROM_LOAD16_BYTE( "rm______.4_4", 0x020001, 0x010000, CRC(1bc71ac8) SHA1(ac0f56d974a4aba143cb3b8cad8a65dc25b3e145) )
	ROM_LOAD16_BYTE( "rm______.4_5", 0x040000, 0x010000, CRC(71460efc) SHA1(3ae79df9d3ec83abdde059827e06e81316026380) )
	ROM_LOAD16_BYTE( "rm______.4_6", 0x040001, 0x010000, CRC(471678de) SHA1(919394768314bc8707f93875528dca33bcf7e09c) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END






ROM_START( v4sunbst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_10___.4_0", 0x0000, 0x010000, CRC(153ce606) SHA1(2e3fada2d150ac46ca38d67845a97d649f7489f9) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbsta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_10_b_.4_0", 0x0000, 0x010000, CRC(e3b87195) SHA1(f6bc0edd6c1e5396fb10adaee1d1d870262ff084) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_10_d_.4_0", 0x0000, 0x010000, CRC(5e9901a7) SHA1(8e3b620d31e03589330d92bce7fac038a2c2484b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_10_k_.4_0", 0x0000, 0x010000, CRC(a81d9634) SHA1(70aded04e2d014632d832d299b0a88124334d2b6) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_20___.4_0", 0x0000, 0x010000, CRC(c7ffdf8e) SHA1(2dd24eeff88ae62b83302404c1304eeb81b83284) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbste )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_20_b_.4_0", 0x0000, 0x010000, CRC(317b481d) SHA1(f06e3510c0a16efa27e94f455c51e8f8cdbc23ae) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_20_bl.4_0", 0x0000, 0x010000, CRC(6febf004) SHA1(d70a64343d6e509fa10cc2321d14dffda4fb2a4a) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_20_d_.4_0", 0x0000, 0x010000, CRC(8c5a382f) SHA1(76e5927216c6bd12d3e3c7bc614366c12a733865) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END


ROM_START( v4sunbsth )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_20_dl.4_0", 0x0000, 0x010000, CRC(d2ca8036) SHA1(cb2db62defeab671e8407e8139feb3b1f9d91c1f) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbsti )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_20_k_.4_0", 0x0000, 0x010000, CRC(7adeafbc) SHA1(216a3344c5e9e60e4429cfb7813ff250db7eb378) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_20bg_.4_0", 0x0000, 0x010000, CRC(4eaae703) SHA1(bdc2a0d752496708b911579867844870b799ee5b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_20bt_.4_0", 0x0000, 0x010000, CRC(f8e1ea00) SHA1(ce3a0bb34a966d15797e7e398d88342b488e0cb1) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_xc___.4_0", 0x0000, 0x010000, CRC(f743e845) SHA1(af60698c8349a982b9cb5bef0a9d670cf3658a0c) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_xc__l.4_0", 0x0000, 0x010000, CRC(a9d3505c) SHA1(07dd1b9a3c663a6315aa3e02bcaa4bc3f38cc4e0) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_xc_b_.4_0", 0x0000, 0x010000, CRC(01c77fd6) SHA1(7562969f482e20ed4a7d66d204309c12ae1843f6) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbsto )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_xc_d_.4_0", 0x0000, 0x010000, CRC(bce60fe4) SHA1(dd2a7e35afe5ed9b07c7395e746090205f014b54) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_xc_dl.4_0", 0x0000, 0x010000, CRC(e276b7fd) SHA1(b6c295a4f1b7bae5bab0ce75032f514945a0c637) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstq )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "st_xc_k_.4_0", 0x0000, 0x010000, CRC(4a629877) SHA1(8c6e5dba3dc048a727562c53203f26d3ac8fc7ee) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sti10___.4_0", 0x0000, 0x010000, CRC(74448990) SHA1(4a6163f20f0f79c4328c6e9577b7063d91f5ae33) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbsts )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sti20___.4_0", 0x0000, 0x010000, CRC(a687b018) SHA1(a1f2bd6568a01614a08b9f64abda458d7b7d3fd4) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stixc___.4_0", 0x0000, 0x010000, CRC(963b87d3) SHA1(8999b41cf214992ed8aab60c83cfdb06b7a2b312) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sunbstu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stixc_d_.4_0", 0x0000, 0x010000, CRC(dd9e6072) SHA1(a28e3e20863f737e7ba8db434c4563d9bfb15fca) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "st______.4_1", 0x000000, 0x010000, CRC(2cd0bfeb) SHA1(e46261451737a99cbb9b7037224d7839c94df7dd) )
	ROM_LOAD16_BYTE( "st______.4_2", 0x000001, 0x010000, CRC(673b3f88) SHA1(e659c4a4ad5851843f14025b16870a5bd26db038) )
	ROM_LOAD16_BYTE( "st______.4_3", 0x020000, 0x010000, CRC(8ae08d60) SHA1(75addb1a323a44cd745b77269d258de4988368d9) )
	ROM_LOAD16_BYTE( "st______.4_4", 0x020001, 0x010000, CRC(ebab723c) SHA1(4bdeb4ec3970ade52d4385cdf38e44b86e46eee2) )
	ROM_LOAD16_BYTE( "st______.4_5", 0x040000, 0x010000, CRC(9d8d22ad) SHA1(13fad9b138893e2d536ea2bde5d20830db12f30f) )
	ROM_LOAD16_BYTE( "st______.4_6", 0x040001, 0x010000, CRC(a139005a) SHA1(ddd417afa7fff180396d3d619bf2e2aa96f26cb9) )
	ROM_LOAD16_BYTE( "st______.4_7", 0x060000, 0x010000, CRC(d54ef568) SHA1(acce3b37dcd05ca335bbc44bc05d9093a6cebd3c) )
	ROM_LOAD16_BYTE( "st______.4_8", 0x060001, 0x010000, CRC(d9aa0643) SHA1(6de6b14dcc9cb0a3eef3635dd07e5f1c16928e6e) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END





ROM_START( v4timebn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ba_20s__.1_0", 0x0000, 0x010000, CRC(55a7b969) SHA1(2809f207fcfe5357b50df1124f8bf55c6e629308) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ba______.1_1", 0x000000, 0x080000, CRC(df853e0e) SHA1(07b0b9aa8a6dc3a70991236f8c1f88b4a6c6755f) )
	ROM_LOAD16_BYTE( "ba______.1_2", 0x000001, 0x080000, CRC(9a2ab155) SHA1(582b33f9ddbf7fb2da71ec6ad5fbbb20a03eda19) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "ba______.1_a", 0x000000, 0x080000, CRC(976d761b) SHA1(32cceff2cd9bc6ad48caac0a2d95d815a5f24aa9) )
	ROM_LOAD( "ba______.1_b", 0x080000, 0x080000, CRC(25a6c7ef) SHA1(cb614c7b2142e47862127d9fdfc7db50978f7f00) )
ROM_END

ROM_START( v4timebna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ba_20a__.1_0", 0x0000, 0x010000, CRC(a04121ac) SHA1(7ba2f229de8bc5883e6e961d873e5fa524680201) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ba______.1_1", 0x000000, 0x080000, CRC(df853e0e) SHA1(07b0b9aa8a6dc3a70991236f8c1f88b4a6c6755f) )
	ROM_LOAD16_BYTE( "ba______.1_2", 0x000001, 0x080000, CRC(9a2ab155) SHA1(582b33f9ddbf7fb2da71ec6ad5fbbb20a03eda19) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "ba______.1_a", 0x000000, 0x080000, CRC(976d761b) SHA1(32cceff2cd9bc6ad48caac0a2d95d815a5f24aa9) )
	ROM_LOAD( "ba______.1_b", 0x080000, 0x080000, CRC(25a6c7ef) SHA1(cb614c7b2142e47862127d9fdfc7db50978f7f00) )
ROM_END

ROM_START( v4timebnb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ba_20bg_.1_0", 0x0000, 0x010000, CRC(dcf281e4) SHA1(0145b8d558ed95b96590cdf9c2f54b6b5f26971d) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ba______.1_1", 0x000000, 0x080000, CRC(df853e0e) SHA1(07b0b9aa8a6dc3a70991236f8c1f88b4a6c6755f) )
	ROM_LOAD16_BYTE( "ba______.1_2", 0x000001, 0x080000, CRC(9a2ab155) SHA1(582b33f9ddbf7fb2da71ec6ad5fbbb20a03eda19) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "ba______.1_a", 0x000000, 0x080000, CRC(976d761b) SHA1(32cceff2cd9bc6ad48caac0a2d95d815a5f24aa9) )
	ROM_LOAD( "ba______.1_b", 0x080000, 0x080000, CRC(25a6c7ef) SHA1(cb614c7b2142e47862127d9fdfc7db50978f7f00) )
ROM_END

ROM_START( v4timebnc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ba_20bt_.1_0", 0x0000, 0x010000, CRC(6ab98ce7) SHA1(6f56c8ba4681255c505bb52e3a76751d1e8acb0e) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ba______.1_1", 0x000000, 0x080000, CRC(df853e0e) SHA1(07b0b9aa8a6dc3a70991236f8c1f88b4a6c6755f) )
	ROM_LOAD16_BYTE( "ba______.1_2", 0x000001, 0x080000, CRC(9a2ab155) SHA1(582b33f9ddbf7fb2da71ec6ad5fbbb20a03eda19) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "ba______.1_a", 0x000000, 0x080000, CRC(976d761b) SHA1(32cceff2cd9bc6ad48caac0a2d95d815a5f24aa9) )
	ROM_LOAD( "ba______.1_b", 0x080000, 0x080000, CRC(25a6c7ef) SHA1(cb614c7b2142e47862127d9fdfc7db50978f7f00) )
ROM_END

ROM_START( v4timebnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ba_20sb_.1_0", 0x0000, 0x010000, CRC(a3232efa) SHA1(12ca99a3f368e54a82719382820e4778bdaccb6e) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ba______.1_1", 0x000000, 0x080000, CRC(df853e0e) SHA1(07b0b9aa8a6dc3a70991236f8c1f88b4a6c6755f) )
	ROM_LOAD16_BYTE( "ba______.1_2", 0x000001, 0x080000, CRC(9a2ab155) SHA1(582b33f9ddbf7fb2da71ec6ad5fbbb20a03eda19) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "ba______.1_a", 0x000000, 0x080000, CRC(976d761b) SHA1(32cceff2cd9bc6ad48caac0a2d95d815a5f24aa9) )
	ROM_LOAD( "ba______.1_b", 0x080000, 0x080000, CRC(25a6c7ef) SHA1(cb614c7b2142e47862127d9fdfc7db50978f7f00) )
ROM_END

ROM_START( v4timebne )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ba_20sd_.1_0", 0x0000, 0x010000, CRC(1e025ec8) SHA1(87cb62ae81aa2735edba08d34c6aa31c78e545e8) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "ba______.1_1", 0x000000, 0x080000, CRC(df853e0e) SHA1(07b0b9aa8a6dc3a70991236f8c1f88b4a6c6755f) )
	ROM_LOAD16_BYTE( "ba______.1_2", 0x000001, 0x080000, CRC(9a2ab155) SHA1(582b33f9ddbf7fb2da71ec6ad5fbbb20a03eda19) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	ROM_LOAD( "ba______.1_a", 0x000000, 0x080000, CRC(976d761b) SHA1(32cceff2cd9bc6ad48caac0a2d95d815a5f24aa9) )
	ROM_LOAD( "ba______.1_b", 0x080000, 0x080000, CRC(25a6c7ef) SHA1(cb614c7b2142e47862127d9fdfc7db50978f7f00) )
ROM_END




ROM_START( v4sixx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6x_20s__.3_0", 0x0000, 0x010000, CRC(452ba9aa) SHA1(4034bfdba6cb46a63a59ba56e19ae912f5633412) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6x_20a__.3_0", 0x0000, 0x010000, CRC(b0cd316f) SHA1(56e2499f041cc534571f14492b5bd8d64a7f0c7b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6x_20ad_.3_0", 0x0000, 0x010000, CRC(fb68d6ce) SHA1(186230bd84d41cbe45f808a63f06d37edff818b1) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxc)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6x_20ak_.3_0", 0x0000, 0x010000, CRC(0dec415d) SHA1(2f9231ee5a3ab978bbeade3495038c14c13cba09) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6x_20bg_.3_0", 0x0000, 0x010000, CRC(cc7e9127) SHA1(ceeab2d61eec87977528030ea8d4b870d08adae2) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6x_20bt_.3_0", 0x0000, 0x010000, CRC(7a359c24) SHA1(2a3496ef1da06aa5a65b79e598ac4652d4520551) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6x_20sb_.3_0", 0x0000, 0x010000, CRC(b3af3e39) SHA1(2690b8e84e812898390979268ab496e5fb12f6ca) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6x_xca__.3_0", 0x0000, 0x010000, CRC(807106a4) SHA1(29895cb8450dc916bffdd3958f144b0a35644c39) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6x_xcab_.3_0", 0x0000, 0x010000, CRC(76f59137) SHA1(183dac7b11c9a7e8d5fdf44e1c6fa755121af5d3) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6x_xcad_.3_0", 0x0000, 0x010000, CRC(cbd4e105) SHA1(443ea2c4ff8afe5c43ee462bfca91ed10ad0d431) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6x_xcak_.3_0", 0x0000, 0x010000, CRC(3d507696) SHA1(9148e33eeef25bb1f4f40ef65257133f744ad503) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6xi20s__.3_0", 0x0000, 0x010000, CRC(2453c63c) SHA1(c83df6afd45d25de0f315b5a75ecd486bca13018) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6xixcs__.3_0", 0x0000, 0x010000, CRC(14eff1f7) SHA1(a9cb41d2c812e3fa51b859912130a4a1e1c0f43d) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4sixxm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6xixcsd_.3_0", 0x0000, 0x010000, CRC(5f4a1656) SHA1(b541ce749bcfca9ef89e320e46cdf060acbca8c3) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD16_BYTE( "6x______.3_1", 0x000000, 0x010000, CRC(870dc043) SHA1(a0bcc8dbfc3cfdd43ed1d6ed5308c43a6f4f8117) )
	ROM_LOAD16_BYTE( "6x______.3_2", 0x000001, 0x010000, CRC(f9ff2007) SHA1(87833cd36859332195c845fffb8a310eb561ae2b) )
	ROM_LOAD16_BYTE( "6x______.3_3", 0x020000, 0x010000, CRC(e8a4531d) SHA1(c816b3b270f1aeaf3ee90aa65dfeea59e8862d1a) )
	ROM_LOAD16_BYTE( "6x______.3_4", 0x020001, 0x010000, CRC(4129b8af) SHA1(30ad007f543e570021292f3eef728b0697c31bb5) )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END








ROM_START( v4megbuk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mc_xc___.2_0", 0x0000, 0x010000, CRC(372ed757) SHA1(ce9d13bc546b7e64ee377627577b3ae0848e8659) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4megbuka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mc_xc_b_.2_0", 0x0000, 0x010000, CRC(c1aa40c4) SHA1(c1ce0bf5929ccba6d5e4ad7508e41d1df8fe10f2) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4megbukb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mc_xc_d_.2_0", 0x0000, 0x010000, CRC(7c8b30f6) SHA1(d851fc2dcb5cf72d0b63ce6fe47b0350faa94356) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4megbukc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mc_xc_k_.2_0", 0x0000, 0x010000, CRC(8a0fa765) SHA1(d687a324223ac1d32a312c8666f63772dc5d9e2b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END




ROM_START( v4rencas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rc_20a__.3_0", 0x0000, 0x010000, CRC(9727f862) SHA1(7815bef16fa7304108d553bbf897376ce2a45ad6) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rencasa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rc_20ab_.3_0", 0x0000, 0x010000, CRC(61a36ff1) SHA1(33a8bd624e995acc43757689988aee1ac79939f1) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rencasb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rc_20ad_.3_0", 0x0000, 0x010000, CRC(dc821fc3) SHA1(2b7ae1b971d137e4f9dc36bf482d3f69f6737a94) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rencasc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rc_20ak_.3_0", 0x0000, 0x010000, CRC(2a068850) SHA1(87d439e3e03fb5a9ba225dea37d0aaa8e2f475f9) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rencasd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rc_20s__.3_0", 0x0000, 0x010000, CRC(62c160a7) SHA1(ea0979ebe0a12f58b15825e1a4074385453e839c) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rencase )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rc_20sb_.3_0", 0x0000, 0x010000, CRC(9445f734) SHA1(74cff30e57a7cc856e72c1bfd8ab2d8d259a2b72) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rencasf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rc_20sd_.3_0", 0x0000, 0x010000, CRC(29648706) SHA1(649aed977028a57664f181bc159663c93fe86e67) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rencasg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rc_20sk_.3_0", 0x0000, 0x010000, CRC(dfe01095) SHA1(bf7c2b93bce4d5caa3c56ff948439acc99efd75b) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rencash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rci20___.3_0", 0x0000, 0x010000, CRC(03b90f31) SHA1(89c73b74751a648686480a17071b231fdfd9002d) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

ROM_START( v4rencasi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rci20_d_.3_0", 0x0000, 0x010000, CRC(481ce890) SHA1(2fe493728acc2aa0398fcc6559bce572ce921274) )

	ROM_REGION( 0x800000, "video", 0 )
	ROM_LOAD("video_board_roms", 0x0000, 0x10000, NO_DUMP )

	ROM_REGION( 0x200000, "okicard:msm6376", ROMREGION_ERASE00 )
	/* none present */
ROM_END

} // Anonymous namespace


/* Complete sets */
/* Standard sets are the most common setups, while Datapak releases use a BACTA datalogger to record more information about the game operation, for security etc.
AMLD versions do not pay out, and instead just feature highscore tables. These were mainly intended for locations unwilling to pay for gaming licenses.
The AMLD Crystal Maze versions appear to be a mixture of the original game modules and Team Challenge's scoring system. This would suggest they were all made ~1994, despite
the copyright dates recorded.
TODO: Sort these better given the wide variation in dates/versions/core code (SWP version id, for one thing).
*/

GAME(  199?, v4bios,     0,        mod2(),     mpu4vid,     mpu4_state,    init_m4,     ROT0, "Barcrest","MPU4 Video Firmware",MACHINE_IS_BIOS_ROOT )

#define GAME_FLAGS MACHINE_NOT_WORKING
#define GAME_FLAGS_OK (MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)

GAMEL( 1993, v4cmaze,    v4bios,   crmaze,     crmaze,   mpu4vid_state, init_crmaze,  ROT0, "Barcrest","The Crystal Maze (v1.3) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze2p )//SWP 0.9
GAMEL( 1993, v4cmazedat, v4cmaze,  crmaze,     crmaze,   mpu4vid_state, init_crmaze,  ROT0, "Barcrest","The Crystal Maze (v1.3, Datapak) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze2p )//SWP 0.9D
GAMEL( 1993, v4cmazeb,   v4cmaze,  crmaze,     crmaze,   mpu4vid_state, init_crmaze,  ROT0, "Barcrest","The Crystal Maze (v1.2) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze2p )//SWP 0.9
GAMEL( 1993, v4cmazec,   v4cmaze,  crmaze,     crmaze,   mpu4vid_state, init_crmaze,  ROT0, "Barcrest","The Crystal Maze (v1.3 alt) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze2p )//SWP 0.9
GAMEL( 1993, v4cmazed,   v4cmaze,  crmaze,     crmaze,   mpu4vid_state, init_crmaze,  ROT0, "Barcrest","The Crystal Maze (v1.1) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze2p )//SWP 0.6

GAMEL( 1993, v4cmaze_amld,   v4cmaze,  crmaze,     crmaze,   mpu4vid_state, init_crmaze,   ROT0, "Barcrest","The Crystal Maze (v0.1, AMLD) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze2p )//SWP 0.9 (actually newer than the 1.1 set then??)

GAMEL( 1993, v4cmaze2,   v4bios,   crmaze,     crmaze,   mpu4vid_state, init_crmaze,   ROT0, "Barcrest","The New Crystal Maze Featuring Ocean Zone (v2.2) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze4p )//SWP 1.0
GAMEL( 1993, v4cmaze2d,  v4cmaze2, crmaze,     crmaze,   mpu4vid_state, init_crmaze,   ROT0, "Barcrest","The New Crystal Maze Featuring Ocean Zone (v2.2, Datapak) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze4p )//SWP 1.0D
GAMEL( 1993, v4cmaze2b,  v4cmaze2, crmaze,     crmaze,   mpu4vid_state, init_crmaze,   ROT0, "Barcrest","The New Crystal Maze Featuring Ocean Zone (v2.0) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze4p )//SWP 1.0
GAMEL( 1993, v4cmaze2c,  v4cmaze2, crmaze,     crmaze,   mpu4vid_state, init_crmaze,   ROT0, "Barcrest","The New Crystal Maze Featuring Ocean Zone (v?.?) (MPU4 Video)",GAME_FLAGS,layout_crmaze4p )// bad rom?

GAMEL( 1993, v4cmaze2_amld,  v4cmaze2, crmaze,     crmaze,   mpu4vid_state, init_crmaze,  ROT0, "Barcrest","The New Crystal Maze Featuring Ocean Zone (v0.1, AMLD) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze4p )//SWP 1.0 /* unprotected? proto? */

GAMEL( 1994, v4cmaze3,   v4bios,   crmaze,    crmaze,   mpu4vid_state, init_crmaze_flutter,  ROT0, "Barcrest","The Crystal Maze Team Challenge (v0.9) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze4p )//SWP 0.7
GAMEL( 1994, v4cmaze3d,  v4cmaze3, crmaze,    crmaze,   mpu4vid_state, init_crmaze_flutter,  ROT0, "Barcrest","The Crystal Maze Team Challenge (v0.9, Datapak) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze4p )//SWP 0.7D
GAMEL( 1994, v4cmaze3b,  v4cmaze3, crmaze,    crmaze,   mpu4vid_state, init_crmaze_flutter,  ROT0, "Barcrest","The Crystal Maze Team Challenge (v0.8) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze4p )//SWP 0.7
GAMEL( 1994, v4cmaze3c,  v4cmaze3, crmaze,    crmaze,   mpu4vid_state, init_crmaze_flutter,  ROT0, "Barcrest","The Crystal Maze Team Challenge (v0.6) (MPU4 Video)",GAME_FLAGS,layout_crmaze4p )// missing one program rom

GAMEL( 1994, v4cmaze3_amld,  v4cmaze3, crmaze,     crmaze,   mpu4vid_state, init_crmaze_flutter,  ROT0, "Barcrest","The Crystal Maze Team Challenge (v1.2, AMLD) (MPU4 Video)",GAME_FLAGS_OK,layout_crmaze4p )//SWP 0.7

//Year is a guess, based on the use of the 'Coin Man' logo
GAME(  1996?,v4mate,     v4bios,   mating,     mating,   mpu4vid_state, init_mating,    ROT0, "Barcrest","The Mating Game (v0.4) (MPU4 Video)",GAME_FLAGS_OK )//SWP 0.2
GAME(  1996?,v4mated,    v4mate,   mating,     mating,   mpu4vid_state, init_mating,    ROT0, "Barcrest","The Mating Game (v0.4, Datapak) (MPU4 Video)",GAME_FLAGS_OK )//SWP 0.2D

/* Quiz games - Questions decoded */

// the v4addlad / v4addladd sets don't do the usual protection check, but still have the device for scrambling questions
// same sequence as bankrollerclub
// 00 8c 64 84 84 c4 84 84 9c f4 04 cc 24 84 c4 94 54 0c 74 0c 34 04 84 84 c4 84 9c e4 84 84 84 d4 44 84 c4 84 9c e4 84 84 84 8c 60 84 84 84 84 c4 9c f4 04 cc 24 9c f4 04 94 14 44 8c 34 04 9c 00
GAMEL(  1989, v4addlad,   v4bios,   mpu4_vid_strike,   adders,   mpu4vid_state, init_strikeit,  ROT0, "Barcrest","Adders and Ladders (v2.1) (MPU4 Video)",GAME_FLAGS_OK,layout_v4addlad )
GAMEL(  1989, v4addladd,  v4addlad, mpu4_vid_strike,   adders,   mpu4vid_state, init_strikeit,  ROT0, "Barcrest","Adders and Ladders (v2.1d) (MPU4 Video)",GAME_FLAGS_OK,layout_v4addlad )
GAMEL(  1989, v4addlad20, v4addlad, mpu4_vid_strike,   adders,   mpu4vid_state, init_strikeit,  ROT0, "Barcrest","Adders and Ladders (v2.0) (MPU4 Video)",GAME_FLAGS_OK,layout_v4addlad )

// 00 c4 c4 44 c4 44 44 c4 cc 3c 5c 7c 54 24 c4 4c b4 84 cc 34 04 4c 14 24 c4 44 cc 14 04 44 c4 4c 1c 54 2c 1c 7c d4 0c 94 04 c4 c0 4c 94 04 44 44 cc 1c 7c 7c d4 8c 1c 5c 5c 5c 7c 74 04 c4 cc 00
GAMEL(  199?, v4strike,   v4bios,   mpu4_vid_strike,   strike,   mpu4vid_state, init_strikeit,  ROT0, "Barcrest","Strike it Lucky (v0.5) (MPU4 Video)",GAME_FLAGS_OK,layout_v4strike )
GAMEL(  199?, v4striked,  v4strike, mpu4_vid_strike,   strike,   mpu4vid_state, init_strikeit,  ROT0, "Barcrest","Strike it Lucky (v0.5, Datapak) (MPU4 Video)",GAME_FLAGS_OK,layout_v4strike )
GAMEL(  199?, v4strike2,  v4strike, mpu4_vid_strike,   strike,   mpu4vid_state, init_strikeit,  ROT0, "Barcrest","Strike it Lucky (v0.53) (MPU4 Video)",GAME_FLAGS_OK,layout_v4strike ) // The '3' is likely a machine type, not a 'version', 68k Pair ROM doesn't change
GAMEL(  199?, v4strike2d, v4strike, mpu4_vid_strike,   strike,   mpu4vid_state, init_strikeit,  ROT0, "Barcrest","Strike it Lucky (v0.53, Datapak) (MPU4 Video)",GAME_FLAGS_OK,layout_v4strike )

// 00 34 14 0c 54 04 24 34 94 94 0c 5c 6c 44 24 24 3c 6c cc 4c c4 a4 24 24 34 84 b4 1c 64 24 34 04 24 34 8c c4 b4 1c e4 24 34 14 10 84 24 34 04 24 b4 04 24 3c 74 94 0c c4 a4 24 24 34 04 34 94 00
GAMEL(  199?, v4barqst,   v4bios,   mpu4_vid_strike,   barquest,  mpu4vid_state, init_strikeit,  ROT0, "Barcrest","Barquest (v2.6) (MPU4 Video)",GAME_FLAGS_OK,layout_v4barqst )
GAMEL(  199?, v4barqstd,  v4barqst, mpu4_vid_strike,   barquest,  mpu4vid_state, init_strikeit,  ROT0, "Barcrest","Barquest (v2.6d) (MPU4 Video)",GAME_FLAGS_OK,layout_v4barqst )

/* Quiz games - Questions not decoded properly on games below (no complete characteriser table) */

// 00 1c 6c a4 0c 24 0c 34 94 94 44 5c 6c 24 1c ac 64 1c ec 64 0c a4 0c 24 1c ac e4 1c 2c a4 0c a4 0c 24 5c ec e4 1c ac 24 1c 6c 60 0c 34 04 0c 24 9c ec a4 4c 24 9c ec 24 0c 34 04 1c ec 24 9c 00
GAME(  199?, v4turnov,   v4bios,    mpu4_vid_cheatchr,   turnover, mpu4vid_state, init_turnover,  ROT0, "Barcrest","Turnover (v2.3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4turnova,  v4turnov,  mpu4_vid_cheatchr,   turnover, mpu4vid_state, init_turnover,  ROT0, "Barcrest","Turnover (v2.33) (MPU4 Video)",GAME_FLAGS ) // the 2nd 3 is likely be a machine type, because much like Strike It Lucky and Wize Move the pairing 68k ROM doesn't change
GAME(  199?, v4turnovc,  v4turnov,  mpu4_vid_cheatchr,   turnover, mpu4vid_state, init_turnover,  ROT0, "Barcrest","Turnover (v2.3O) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4turnovd,  v4turnov,  mpu4_vid_cheatchr,   turnover, mpu4vid_state, init_turnover,  ROT0, "Barcrest","Turnover (v?.?) (MPU4 Video)",GAME_FLAGS ) // only have a single program ROM

// 00 1c cc 64 1c 4c 64 1c ec e4 0c d4 84 0c 44 2c d4 14 34 14 24 0c 44 0c 44 1c ec 54 04 0c 54 24 0c 44 9c ec e4 1c 6c 54 04 1c c8 64 1c 4c 64 1c ec 64 0c d4 04 3c 6c 44 2c 54 84 1c ec 44 3c 00
GAME(  1990, v4skltrk,   v4bios,    mpu4_vid_cheatchr,   skiltrek, mpu4vid_state, init_skiltrek,  ROT0, "Barcrest","Skill Trek (v1.1) (MPU4 Video, set 1)",GAME_FLAGS ) // 10 pound max
GAME(  1990, v4skltrka,  v4skltrk,  mpu4_vid_cheatchr,   skiltrek, mpu4vid_state, init_skiltrek,  ROT0, "Barcrest","Skill Trek (v1.1) (MPU4 Video, set 2)",GAME_FLAGS ) // 12 pound max

// 00 2c 94 14 04 0c c4 0c d4 64 0c b4 04 0c 84 5c dc 9c dc 9c dc cc 84 0c 84 0c d4 04 2c c4 0c c4 0c 84 1c dc dc 8c d4 44 2c 94 20 0c a4 0c c4 0c d4 14 14 54 04 6c c4 4c c4 0c c4 2c c4 0c d4 00
GAME(  1989, v4tmach,     v4bios,   mpu4_vid_cheatchr,   skiltrek, mpu4vid_state, init_timemchn,  ROT0, "Barcrest","Time Machine (v2.0) (Issue 3 Questions) (MPU4 Video)",GAME_FLAGS )
GAME(  1989, v4tmachd,    v4tmach,  mpu4_vid_cheatchr,   skiltrek, mpu4vid_state, init_timemchn,  ROT0, "Barcrest","Time Machine (v2.0) (Issue 3 Questions) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  1989, v4tmach1,    v4tmach,  mpu4_vid_cheatchr,   skiltrek, mpu4vid_state, init_timemchn,  ROT0, "Barcrest","Time Machine (v2.0) (Issue 1 Questions) (MPU4 Video)",GAME_FLAGS )
GAME(  1989, v4tmach1d,   v4tmach,  mpu4_vid_cheatchr,   skiltrek, mpu4vid_state, init_timemchn,  ROT0, "Barcrest","Time Machine (v2.0) (Issue 1 Questions) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  1989, v4tmach2,    v4tmach,  mpu4_vid_cheatchr,   skiltrek, mpu4vid_state, init_timemchn,  ROT0, "Barcrest","Time Machine (v2.0) (Issue 2 Questions) (MPU4 Video)",GAME_FLAGS )
GAME(  1989, v4tmach2d,   v4tmach,  mpu4_vid_cheatchr,   skiltrek, mpu4vid_state, init_timemchn,  ROT0, "Barcrest","Time Machine (v2.0) (Issue 2 Questions) (Datapak) (MPU4 Video)",GAME_FLAGS )

/* Quiz games - Games below are missing question ROMs */

// winner takes all sequence?
// 00 64 64 24 64 64 24 64 6c 9c bc bc a4 24 64 24 74 44 6c 94 1c ac 84 24 64 64 6c c4 24 24 64 24 24 64 74 04 6c c4 2c c4 24 64 60 24 64 64 24 64 6c 8c 8c 94 14 4c 8c 9c bc ac 8c 94 14 04 6c 00
GAME(  1990, v4sklcsh,   v4bios,   mpu4_vid_cheatchr,   skiltrek, mpu4vid_state, init_v4barqst,  ROT0, "Barcrest","Skill Cash (v1.1) (MPU4 Video)",GAME_FLAGS )

// 00 8c 64 0c c4 0c 54 14 94 94 24 ac 44 0c 44 1c 7c 6c 74 84 3c 4c 44 0c 44 8c 74 84 0c 54 04 1c 7c cc 64 0c 74 84 3c 5c 4c 64 88 74 04 8c 54 04 9c 7c 5c 7c cc 74 04 1c 5c 5c 7c 6c 54 04 9c 00
GAME(  199?, v4eyedwn,   v4bios,   mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_eyesdown,  ROT0, "Barcrest","Eyes Down (v1.3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4eyedwnd,  v4eyedwn, mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_eyesdown,  ROT0, "Barcrest","Eyes Down (v1.3, Datapak) (MPU4 Video)",GAME_FLAGS )

// similar to the 'Winner Takes All' sequence but not the same
// 00 64 64 24 64 64 24 64 74 54 84 a4 24 24 64 24 e4 64 74 44 34 04 24 24 64 64 74 44 64 24 64 24 24 64 e4 24 74 44 34 14 04 64 60 24 64 64 24 64 74 04 24 e4 64 74 04 34 04 64 24 64 24 64 74 00
GAME(  199?, v4quidgr,   v4bios,   mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_quidgrid,  ROT0, "Barcrest","Ten Quid Grid (v1.2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4quidgrd,  v4quidgr, mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_quidgrid,  ROT0, "Barcrest","Ten Quid Grid (v1.2, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4quidgr2,  v4quidgr, mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_quidgrid,  ROT0, "Barcrest","Ten Quid Grid (v2.4) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4quidgr2d, v4quidgr, mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_quidgrid,  ROT0, "Barcrest","Ten Quid Grid (v2.4, Datapak) (MPU4 Video)",GAME_FLAGS )

// 00 34 14 0c 54 04 24 34 94 94 0c 5c 6c 44 24 24 3c 6c cc 4c c4 a4 24 24 34 84 b4 1c 64 24 34 04 24 34 8c c4 b4 1c e4 24 34 14 10 84 24 34 04 24 b4 04 24 3c 74 94 0c c4 a4 24 24 34 04 34 94 00
GAMEL( 199?, v4barqs2,   v4bios,   mpu4_vid_cheatchr,   barquest,   mpu4vid_state, init_v4barqst2, ROT0, "Barcrest","Barquest 2 (v0.3) (MPU4 Video)",GAME_FLAGS,layout_v4barqst )

// 00 34 14 84 24 34 04 34 54 54 84 a4 24 24 34 04 b4 14 54 14 44 64 24 24 34 44 74 14 04 24 34 04 24 34 c4 24 74 14 44 34 04 34 10 44 34 04 24 34 54 84 24 b4 94 54 84 e4 24 34 04 34 04 34 54 00
// again the 2nd '3' seems to indicate a machine type, not a version
GAME(  199?, v4wize,     v4bios,   mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_v4wize,    ROT0, "Barcrest","Wize Move (v1.3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4wized,    v4wize,   mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_v4wize,    ROT0, "Barcrest","Wize Move (v1.3d) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4wizeb,    v4wize,   mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_v4wize,    ROT0, "Barcrest","Wize Move (v1.33) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4wizec,    v4wize,   mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_v4wize,    ROT0, "Barcrest","Wize Move (v1.3d3) (Datapak) (MPU4 Video)",GAME_FLAGS )
// older code, 1x 68k ROM is missing in addition to questions being missing
GAME(  199?, v4wizeo,    v4wize,   mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_v4wize,    ROT0, "Barcrest","Wize Move (v1.2) (MPU4 Video)",GAME_FLAGS )
// newer? code, only 1x 68k ROM is dumped (and it appears to be slightly corrupt)
GAME(  199?, v4wizen,    v4wize,   mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_v4wize,    ROT0, "Barcrest","Wize Move (v?.?) (MPU4 Video)",GAME_FLAGS )

// 00 34 14 0c 54 04 24 34 94 94 0c 5c 6c 44 24 24 3c 6c cc 4c c4 a4 24 24 34 84 b4 1c 64 24 34 04 24 34 8c c4 b4 1c e4 24 34 14 10 84 24 34 04 24 b4 04 24 3c 74 94 0c c4 a4 24 24 34 04 34 94 00
GAME(  1991, v4opt3,     v4bios,   mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_v4opt3,    ROT0, "Barcrest","Option 3 (v1.0) (MPU4 Video)",GAME_FLAGS )
GAME(  1991, v4opt3d,    v4opt3,   mpu4_vid_cheatchr,   mpu4vid,   mpu4vid_state, init_v4opt3,    ROT0, "Barcrest","Option 3 (v1.0) (Datapak) (MPU4 Video)",GAME_FLAGS )

/* ----------------------------------------------------------------  */
/* Games below are newer BWB games and use their own game and revision specific MPU4 base ROMs (which must be correctly paired with video ROMs of the same revision) and sometimes differing hardware setups */
/* ----------------------------------------------------------------  */

GAMEL(  1991, v4psi,      0,        bwbvid,     v4psi,     mpu4vid_state, init_prizeinv,  ROT0, "BWB",           u8"Prize Space Invaders (BWB IN2, £20, 50p/30p Play, 20\" Version 1.1) (MPU4 Video)",GAME_FLAGS,layout_v4psi )
GAMEL(  1991, v4psid,     v4psi,    bwbvid,     v4psi,     mpu4vid_state, init_prizeinv,  ROT0, "BWB",           u8"Prize Space Invaders (BWB IN2, £20, 50p/30p Play, 20\" Version 1.1) (Datapak) (MPU4 Video)",GAME_FLAGS,layout_v4psi )
// the 68k ROMs are bad on these (one missing)
GAMEL(  1991, v4psi14,    v4psi,    bwbvid,     v4psi,     mpu4vid_state, init_prizeinv,  ROT0, "BWB",           u8"Prize Space Invaders (BWB IN2, £20, 50p/30p Play, 14\" Version 1.1, set 1) (MPU4 Video)",GAME_FLAGS,layout_v4psi )
GAMEL(  1991, v4psi14a,   v4psi,    bwbvid,     v4psi,     mpu4vid_state, init_prizeinv,  ROT0, "BWB",           u8"Prize Space Invaders (BWB IN2, £20, 50p/30p Play, 14\" Version 1.1, set 2) (MPU4 Video)",GAME_FLAGS,layout_v4psi )
// these show BWB and Barcrest (older game release 'INV1' but higher version of it)
GAMEL(  1991, v4psibc,    v4psi,    bwbvid,     v4psi,     mpu4vid_state, init_prizeinv,  ROT0, "BWB / Barcrest","Prize Space Invaders (BWB INV1, 50p/30p Play, Version 1.2) (MPU4 Video)",GAME_FLAGS_OK,layout_v4psi )
GAMEL(  1991, v4psibcd,   v4psi,    bwbvid,     v4psi,     mpu4vid_state, init_prizeinv,  ROT0, "BWB / Barcrest","Prize Space Invaders (BWB INV1, 50p/30p Play, Version 1.2) (Datapak) (MPU4 Video)",GAME_FLAGS_OK,layout_v4psi )


// Tetris games, these were all sold as different machines so are not set as clones
GAME(  1989, v4tetrs,    0,        bwbvid,     v4tetris,   mpu4vid_state, init_bwbhack,     ROT0, "BWB / Barcrest","Tetris Payout (BWB TET1 Version 2.2, set 1) (MPU4 Video)",GAME_FLAGS_OK )
GAME(  1989, v4tetrs1,   v4tetrs,  bwbvid,     v4tetris,   mpu4vid_state, init_bwbhack,     ROT0, "BWB / Barcrest","Tetris Payout (BWB TET1 Version 2.2, set 2) (MPU4 Video)",GAME_FLAGS_OK )
// Blox is an later version of Payout Tetris, without Tetris license? (SJM = Stuart McArthur?)
GAME(  1990, v4blox,     0,        bwbvid,     v4tetris,   mpu4vid_state, init_bwbhack,     ROT0, "BWB / Barcrest","Blox (SJM BLOX, 50p/20p Play, Version 2.0) (MPU4 Video)",GAME_FLAGS_OK )
GAME(  1990, v4bloxd,    v4blox,   bwbvid,     v4tetris,   mpu4vid_state, init_bwbhack,     ROT0, "BWB / Barcrest","Blox (SJM BLOX, 50p/20p Play, Version 2.0) (Datapak) (MPU4 Video)",GAME_FLAGS_OK )
// Prize Tetris / Bullion Blox have quite different attract presentation to the above
GAME(  1994, v4pztet,    0,        bwbvid,     v4pztet,    mpu4vid_state, init_bwbhack,     ROT0, "BWB",           "Prize Tetris (BWB) (MPU4 Video)",GAME_FLAGS_OK )
GAME(  1994, v4pzteta,   v4pztet,  bwbvid,     v4pztet,    mpu4vid_state, init_bwbhack,     ROT0, "BWB",           "Prize Tetris (BWB) (Datapak) (MPU4 Video)",GAME_FLAGS_OK )
GAME(  1994, v4pztetb,   v4pztet,  bwbvid,     v4pztet,    mpu4vid_state, init_bwbhack,     ROT0, "BWB",           "Prize Tetris (BWB) (Showcase) (MPU4 Video)",GAME_FLAGS_OK ) // screen telling you to exchange tickets for prizes in the 'showcase' during attract
GAME(  1994, v4pztetc,   v4pztet,  bwbvid,     v4pztet,    mpu4vid_state, init_bwbhack,     ROT0, "BWB",           "Prize Tetris (BWB) (Showcase) (Datapak) (MPU4 Video)",GAME_FLAGS_OK )
// this appears to be a version of Prize Tetris without the Tetris license. These don't have proper alarms, eg coin1 stuck is 'undefined'
GAME(  1994, v4bulblx,   0,        bwbvid,     v4bulblx,   mpu4vid_state, init_bwbhack,     ROT0, "BWB",           "Bullion Blox (BWB) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1994, v4bulblxb,  v4bulblx, bwbvid,     v4bulblx,   mpu4vid_state, init_bwbhack,     ROT0, "BWB",           "Bullion Blox (BWB) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1994, v4bulblxa,  v4bulblx, bwbvid,     v4bulblx,   mpu4vid_state, init_bwbhack,     ROT0, "BWB",           "Bullion Blox (BWB) (Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1994, v4bulblxc,  v4bulblx, bwbvid,     v4bulblx,   mpu4vid_state, init_bwbhack,     ROT0, "BWB",           "Bullion Blox (BWB) (Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )

// doesn't have payout so no shelf error (no payout on prototype?), runs with door closed

GAME(  199?, v4vgpok,    0,        bwbvid,     v4vgpok,    mpu4vid_state, init_bwbhack,     ROT0, "BWB","Vegas Poker (prototype, release 2) (MPU4 Video)",GAME_FLAGS_OK )

// boot and run

GAME(  199?, v4redhtp,   0,        bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Fixed, Cash+Token) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpk,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Fixed, Cash+Token) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpl,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Fixed, Cash+Token, Datapak, % Key) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpp,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Fixed, Cash+Token, Datapak, % Key) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpm,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Fixed, Cash+Token, Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpo,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Fixed, Cash+Token, Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpq,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Fixed, Cash+Token, Datapak) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpn,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Fixed, Cash+Token, % Key) (set 1)  (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpr,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Fixed, Cash+Token, % Key) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtps,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Switchable to 10p, Cash+Token) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpx,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Switchable to 10p, Cash+Token) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpt,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Switchable to 10p, Cash+Token, Datapak, % Key) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpy,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Switchable to 10p, Cash+Token, Datapak, % Key) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpu,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Switchable to 10p, Cash+Token, Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpw,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Switchable to 10p, Cash+Token, Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpz,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Switchable to 10p, Cash+Token, Datapak) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpv,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Switchable to 10p, Cash+Token, % Key) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpaa, v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Switchable to 10p, Cash+Token, % Key) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpb,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 10p Fixed, Cash+Token) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpg,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 10p Fixed, Cash+Token) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpc,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 10p Fixed, Cash+Token, Datapak) (set 1, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtph,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 10p Fixed, Cash+Token, Datapak) (set 2, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpd,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 10p Fixed, Cash+Token, Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpf,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 10p Fixed, Cash+Token, Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpi,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 10p Fixed, Cash+Token, Datapak) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpe,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 10p Fixed, Cash+Token, % Key) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpj,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 10p Fixed, Cash+Token, % Key) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpab, v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 10p Fxed, All-Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4redhtpa,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 3, 20p Switchable to 10p, All - Cash) (MPU4 Video)",GAME_FLAGS )
// 'version 1.9' 68k ROMs - different numbering format?
GAME(  199?, v4redhtparc,v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Arcade, Cash+Tokens, 1993 Awards, 20p Fixed, Version 1.9) (MPU4 Video)",GAME_FLAGS )
// release 2 68k ROMs
GAME(  1993, v4redhtp2,  v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 2, 1993 copyright, 5p Fixed, Cash+Token) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1993, v4redhtp2a, v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 2, 1993 copryight, 5p Fixed, Cash+Token) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1993, v4redhtp2b, v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 2, 1993 copryight, 5p Fixed, Cash+Token, Datapak, % Key) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1993, v4redhtp2e, v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 2, 1993 copryight, 5p Fixed, Cash+Token, Datapak, % Key) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1993, v4redhtp2c, v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 2, 1993 copryight, 5p Fixed, Cash+Token, Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1993, v4redhtp2f, v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 2, 1993 copryight, 5p Fixed, Cash+Token, Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1993, v4redhtp2d, v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 2, 1993 copryight, 5p Fixed, Cash+Token, % Key) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1993, v4redhtp2g, v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 2, 1993 copryight, 5p Fixed, Cash+Token, % Key) (set 2) (MPU4 Video)",GAME_FLAGS )
// this is older despite still being 'release 2'
GAME(  1992, v4redhtp2z, v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (Release 2, 1992 copyright, 20p Fixed, Cash+Token) (MPU4 Video)",GAME_FLAGS )
// no matching 68k ROMs for this one
GAME(  199?, v4redhtpunk,v4redhtp, bwbvid,     v4redhtp, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Poker (unknown release) (MPU4 Video)",GAME_FLAGS )


GAME(  199?, v4bubbnk,   0,        bwbvid,     v4bubbnk, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Bubbly Bonk (v4.0?) (20p Fixed, Cash+Token) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4bubbnkb,  v4bubbnk, bwbvid,     v4bubbnk, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Bubbly Bonk (v4.0?) (20p Fixed, Cash+Token, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4bubbnkc,  v4bubbnk, bwbvid,     v4bubbnk, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Bubbly Bonk (v4.0?) (20p Fixed, Cash+Token, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4bubbnkd,  v4bubbnk, bwbvid,     v4bubbnk, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Bubbly Bonk (v4.0?) (20p Fixed, Cash+Token, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4bubbnka,  v4bubbnk, bwbvid,     v4bubbnk, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Bubbly Bonk (v4.0?) (20p Fixed, All - Cash) (MPU4 Video)",GAME_FLAGS )


GAME(  199?, v4ovrmn3,   0,        bwbvid,     v4bubbnk, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Over Moon Pt3 (BWB) (20p Fixed, Cash+Token) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4ovrmn3a,  v4ovrmn3, bwbvid,     v4bubbnk, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Over Moon Pt3 (BWB) (20p Fixed, Cash+Token, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4ovrmn3b,  v4ovrmn3, bwbvid,     v4bubbnk, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Over Moon Pt3 (BWB) (20p Fixed, Cash+Token, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4ovrmn3c,  v4ovrmn3, bwbvid,     v4bubbnk, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Over Moon Pt3 (BWB) (20p Fixed, Cash+Token, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4ovrmn3d,  v4ovrmn3, bwbvid,     v4bubbnk, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Over Moon Pt3 (BWB) (20p Fixed, All - Cash) (MPU4 Video)",GAME_FLAGS )


GAME(  199?, v4mazbel,   0,        bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Mazooma Belle (v2.5 DY, S/Site, Cash+Token, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4mazbel15, v4mazbel, bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Mazooma Belle (v1.5, Arcade, Cash+Token) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4mazbel15a,v4mazbel, bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Mazooma Belle (v1.5 DY, Arcade, Cash+Token, % Key) (MPU4 Video)",GAME_FLAGS )


GAME(  199?, v4rhmaz,    0,        bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Mazooma Belle (BWB) (Version 1.4, Cash+Token, 1993 Awards, 10p Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rhmaza,   v4rhmaz,  bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Mazooma Belle (BWB) (Version 1.4 DY, Cash+Token, 1993 Awards, 10p Fixed, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rhmazb,   v4rhmaz,  bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Mazooma Belle (BWB) (Version 1.4 D, Cash+Token, 1993 Awards, 10p Fixed, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rhmazc,   v4rhmaz,  bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Mazooma Belle (BWB) (Version 1.4, Cash+Token, 1993 Awards, 20p Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rhmazd,   v4rhmaz,  bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Mazooma Belle (BWB) (Version 1.4 DY, Cash+Token, 1993 Awards, 20p Fixed, Datapak, % Key) (set 1) (MPU4 Video)",GAME_FLAGS ) // B   (Data Port failure on first boot?)
GAME(  199?, v4rhmaze,   v4rhmaz,  bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Mazooma Belle (BWB) (Version 1.4 DY, Cash+Token, 1993 Awards, 20p Fixed, Datapak, % Key) (set 2) (MPU4 Video)",GAME_FLAGS ) // BL  (Data Port failure on first boot?)
GAME(  199?, v4rhmazf,   v4rhmaz,  bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Mazooma Belle (BWB) (Version 1.4 D, Cash+Token, 1993 Awards, 20p Fixed, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rhmazg,   v4rhmaz,  bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Mazooma Belle (BWB) (Version 1.4, Cash+Token, 1993 Awards, 20p Switchable to 10p) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rhmazh,   v4rhmaz,  bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Mazooma Belle (BWB) (Version 1.4 DY, Cash+Token, 1993 Awards, 20p Switchable to 10p, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rhmazi,   v4rhmaz,  bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Mazooma Belle (BWB) (Version 1.4 D, Cash+Token, 1993 Awards, 20p Switchable to 10p, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rhmazj,   v4rhmaz,  bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Mazooma Belle (BWB) (Version 1.4 Y, Cash+Token, 1993 Awards, 20p Switchable to 10p, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rhmazk,   v4rhmaz,  bwbvid,     v4mazbel, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Red Hot Mazooma Belle (BWB) (Version 1.4 C, Cash+Token, 1993 Awards, 20p Switchable to 10p) (MPU4 Video)",GAME_FLAGS )


GAME(  199?, v4shpwnd,   0,        bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 20p Fixed, Cash + Special BWB Token) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwndi,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 20p Fixed, Cash + Special BWB Token) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwnde,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 20p Fixed, Cash + Special BWB Token, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwndf,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 20p Fixed, Cash + Special BWB Token, Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwndh,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 20p Fixed, Cash + Special BWB Token, Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwndg,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 20p Fixed, Cash + Special BWB Token, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwnda,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 10p Fixed, Cash + Special BWB Token) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwndb,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 10p Fixed, Cash + Special BWB Token, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwndc,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 10p Fixed, Cash + Special BWB Token, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwndd,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 10p Fixed, Cash + Special BWB Token, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwndj,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 20p Switchable to 10p, Cash + Special BWB Token) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwndk,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 20p Switchable to 10p, Cash + Special BWB Token, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwndl,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 20p Switchable to 10p, Cash + Special BWB Token, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4shpwndm,  v4shpwnd, bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Shop Window (v2.0) (Release 2, 20p Switchable to 10p, Cash + Special BWB Token, % Key) (MPU4 Video)",GAME_FLAGS )


GAME(  199?, v4sixx,     0,        bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Fixed, Cash+Token) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxa,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Fixed, Cash+Token) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxe,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Fixed, Cash+Token) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxb,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Fixed, Cash+Token, Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxd,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Fixed, Cash+Token, Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxf,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Fixed, Cash+Token, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxc,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Fixed, Cash+Token, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxg,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Switchable to 10p, Cash+Token) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxh,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Switchable to 10p, Cash+Token, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxi,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Switchable to 10p, Cash+Token, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxj,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Switchable to 10p, Cash+Token, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxk,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Fixed, All - Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxl,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Switchable to 10p, All - Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sixxm,    v4sixx,   bwbvid,     v4shpwnd, mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"£6-X (BWB) (Release 3, 20p Switchable to 10p, All - Cash, Datapak) (MPU4 Video)",GAME_FLAGS )


GAME(  199?, v4cshinf,   0,        bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Fixed, Cash+Token) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfd,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Fixed, Cash+Token) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfg,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Fixed, Cash+Token) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfe,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Fixed, Cash+Token, Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfk,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Fixed, Cash+Token, Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfi,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Fixed, Cash+Token, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfh,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Fixed, Cash+Token, Showcase) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinff,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Fixed, Cash+Token, Showcase, Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfj,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Fixed, Cash+Token, Showcase, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfl,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Fixed, Cash+Token, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfm,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Switchable 20p/10p/5p, Cash+Token) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfr,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Switchable 20p/10p/5p, Cash+Token) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfp,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Switchable 20p/10p/5p, Cash+Token, Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfs,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Switchable 20p/10p/5p, Cash+Token, Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfo,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Switchable 20p/10p/5p, Cash+Token, Datapak, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfn,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Switchable 20p/10p/5p, Cash+Token, Showcase) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfq,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Switchable 20p/10p/5p, Cash+Token, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfu,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Switchable 20p/10p/5p, All - Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfw,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 20p Switchable 20p/10p/5p, All - Cash, Datapak)  (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfb,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 10p Fixed, Cash+Token) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinfa,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 4, 5p Fixed, All - Cash) (MPU4 Video)",GAME_FLAGS )
// no 68k program is dumped for Release 3
GAME(  199?, v4cshinf3,  v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 3) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4cshinf3a, v4cshinf, bwbvid,     v4cshinf,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Cash Inferno (BWB) (Release 3) (set 2) (MPU4 Video)",GAME_FLAGS )



// the onscreen 'version' display doesn't quite align with the labels, 'DY' seems to be represented as 'B' on the labels, rather than the individual flags?
// the labels seem closer to the Barcrest standard used on MPU4 fruit machines
// gfx look wrong in test mode, uses BT chip?
GAME(  199?, v4sunbst,   0,        bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4, Cash+Token, 1993 Awards, 10p Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbsta,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 DY, Cash+Token, 1993 Awards, 10p Fixed, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbstb,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 D, Cash + Token, 1993 Awards, 10p Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbstc,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 Y, Cash+Token, 1993 Awards, 10p Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbstd,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4, Cash+Token, 1993 Awards, 20p Fixed) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbstk,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4, Cash+Token, 1993 Awards, 20p Fixed) (set 2) (MPU4 Video)",GAME_FLAGS ) // BT
GAME(  199?, v4sunbste,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 DY, Cash+Token, 1993 Awards, 20p Fixed, % Key) (set 1) (MPU4 Video)",GAME_FLAGS )  // B
GAME(  199?, v4sunbstf,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 DY, Cash+Token, 1993 Awards, 20p Fixed, % Key) (set 2) (MPU4 Video)",GAME_FLAGS )  // BL
GAME(  199?, v4sunbstg,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 D, Cash+Token, 1993 Awards, 20p Fixed) (set 1) (MPU4 Video)",GAME_FLAGS ) // D
GAME(  199?, v4sunbsth,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 D, Cash+Token, 1993 Awards, 20p Fixed) (set 2) (MPU4 Video)",GAME_FLAGS ) // DL
GAME(  199?, v4sunbsti,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 Y, Cash+Token, 1993 Awards, 20p Fixed, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbstj,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 D, Cash+Token, 1993 Awards, 20p Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbstl,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4, Cash+Token, 1993 Awards, 20p Switchable to 10p) (set 1) (MPU4 Video)",GAME_FLAGS ) // XC
GAME(  199?, v4sunbstm,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4, Cash+Token, 1993 Awards, 20p Switchable to 10p) (set 2) (MPU4 Video)",GAME_FLAGS ) // XC L
GAME(  199?, v4sunbstn,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 DY, Cash+Token, 1993 Awards, 20p Switchable to 10p, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbsto,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 D, Cash+Token, 1993 Awards, 20p Switchable to 10p (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbstp,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 D, Cash+Token, 1993 Awards, 20p Switchable to 10p) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbstq,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 Y, Cash+Token, 1993 Awards, 20p Switchable to 10p, % Key) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbstr,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 C, 1993 Awards, 10p Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbsts,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 C, 1993 Awards, 20p Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbstt,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 C, 1993 Awards, 20p Switchable to 10p) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4sunbstu,  v4sunbst, bwbvid,     v4cshinf,   mpu4vid_state, init_bwbhack,     ROT0, "BWB","Sunburst (BWB) (Version 1.4 IC, 1993 Awards, 20p Switchable to 10p) (MPU4 Video)",GAME_FLAGS )

//gfx look wrong in test mode, uses BT chip?
GAME(  199?, v4bigfrt,   0,        bwbvid,     v4bigfrt,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big Fruits (v2.0?) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4bigfrta,  v4bigfrt, bwbvid,     v4bigfrt,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big Fruits (v2.0?) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4bigfrtb,  v4bigfrt, bwbvid,     v4bigfrt,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big Fruits (v2.0?) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4bigfrtc,  v4bigfrt, bwbvid,     v4bigfrt,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big Fruits (v2.0?) (% Key) (MPU4 Video)",GAME_FLAGS )


//gfx look wrong in test mode, uses BT chip?
GAME(  1996, v4reno,     0,        bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, All - Cash) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renoa,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, All - Cash) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renom,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, All - Cash) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renoe,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, Cash+Token) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renoi,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, Cash+Token) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renol,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, Cash+Token) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renoc,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, Cash+Token) (Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renog,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, Cash+Token) (Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renok,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, Cash+Token) (Datapak) (set 3) (MPU4 Video)",GAME_FLAGS ) // gives a Data Port error first itme?
GAME(  1996, v4renod,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, Cash+Token) (Showcase) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renoh,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, Cash+Token) (Showcase) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renof,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, Cash+Token) (Showcase) (set 1) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renoj,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Fixed, Cash+Token) (Showcase) (set 2) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renoo,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 25p Fixed, All - Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renon,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 25p Fixed, All - Cash) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renoq,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Switchable 20p/25p, All - Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renop,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Switchable 20p/25p, All - Cash) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renob,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Switchable 20p/10p/5p, All - Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renos,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Switchable 20p/10p/5p, Cash+Token) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renou,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Switchable 20p/10p/5p, Cash+Token) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renor,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Switchable 20p/10p/5p, Cash+Token) (Showcase) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4renot,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release A, 20p Switchable 20p/10p/5p, Cash+Token) (Showcase) (set 2) (MPU4 Video)",GAME_FLAGS )
// older 68k version
GAME(  1996, v4reno8,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release 8, 20p Fixed, Cash+Token) (MPU4 Video)",GAME_FLAGS )
// 68k ROMs below have no matching base roms
GAME(  1996, v4reno7,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release 7) (MPU4 Video)",GAME_FLAGS ) // no base ROM
GAME(  1996, v4reno5,    v4reno,   bwbvid_oki,    v4reno,   mpu4vid_state, init_prizeinv,    ROT0, "BWB","Reno Reels (Release 5) (MPU4 Video)",GAME_FLAGS ) // no base ROM


GAME(  1996, v4big40,    0,        bwbvid_oki_bt471,     v4big40,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big 40 Poker (BWB) (Arcade Standard) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4big40a,   v4big40,  bwbvid_oki_bt471,     v4big40,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big 40 Poker (BWB) (Arcade Standard) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4big40b,   v4big40,  bwbvid_oki_bt471,     v4big40,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big 40 Poker (BWB) (Arcade Standard) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4big40c,   v4big40,  bwbvid_oki_bt471,     v4big40,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big 40 Poker (BWB) (Arcade Standard) (set 4) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4big40d,   v4big40,  bwbvid_oki_bt471,     v4big40,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big 40 Poker (BWB) (Arcade Data) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4big40e,   v4big40,  bwbvid_oki_bt471,     v4big40,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big 40 Poker (BWB) (Arcade Data) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4big40g,   v4big40,  bwbvid_oki_bt471,     v4big40,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big 40 Poker (BWB) (S_Site Standard) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4big40h,   v4big40,  bwbvid_oki_bt471,     v4big40,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big 40 Poker (BWB) (S_Site Standard) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4big40k,   v4big40,  bwbvid_oki_bt471,     v4big40,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big 40 Poker (BWB) (S_Site Data) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4big40i,   v4big40,  bwbvid_oki_bt471,     v4big40,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big 40 Poker (BWB) (S_Site Data + %-Key) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4big40j,   v4big40,  bwbvid_oki_bt471,     v4big40,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big 40 Poker (BWB) (S_Site Data + %-Key) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1996, v4big40f,   v4big40,  bwbvid_oki_bt471,     v4big40,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Big 40 Poker (BWB) (S_Site Data + %-Key + OCDM) (MPU4 Video)",GAME_FLAGS )


GAMEL(  1997, v4dbltak,   0,        bwbvid_oki_bt471, v4dbltak,     mpu4vid_state, init_bwbhack,     ROT0, u8"BWB","Double Take (BWB) (Release 4, Arcade Standard, 20p/25p Stake Key, £5/£10/£15 Prize Key) (MPU4 Video)",GAME_FLAGS,layout_v4dbltak )
GAMEL(  1997, v4dbltaka,  v4dbltak, bwbvid_oki_bt471, v4dbltak,     mpu4vid_state, init_bwbhack,     ROT0, u8"BWB","Double Take (BWB) (Release 4, Arcade Data, 20p/25p Stake Key, £5/£10/£15 Prize Key) (MPU4 Video)",GAME_FLAGS,layout_v4dbltak )
GAMEL(  1997, v4dbltakb,  v4dbltak, bwbvid_oki_bt471, v4dbltak_perc,mpu4vid_state, init_bwbhack,     ROT0, u8"BWB","Double Take (BWB) (Release 4, S_Site Data, 20p/25p Stake Key, £5/£10/£15 Prize Key, % Key) (MPU4 Video)",GAME_FLAGS,layout_v4dbltak )


GAME(  199?, v4gldrsh,   0,        bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, All - Cash) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrsht,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, All - Cash) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshk,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, All - Cash) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshl,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 25p Fixed, All - Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshm,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 25p Fixed, All - Cash) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshn,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, Switchable 20p/25p, All - Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrsho,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, Switchable 20p/25p, All - Cash) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshu,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, Switchable 20p/10p/5p, All - Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshc,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, Cash+Token) (Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshd,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, Cash+Token) (Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrsha,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, Cash+Token) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshf,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, Cash+Token) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshb,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, Cash+Token) (Showcase) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshg,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, Cash+Token) (Showcase) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshe,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, Cash+Token) (Showcase) (Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshj,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, Cash+Token) (Showcase) (Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshh,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, Cash+Token) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshi,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, 20p Fixed, Cash+Token) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshp,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, Switchable 20p/10p/5p, Cash+Token) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshq,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, Switchable 20p/10p/5p, Cash+Token) (Showcase) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshr,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, Switchable 20p/10p/5p, Cash+Token) (Datapak) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4gldrshs,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 8, Switchable 20p/10p/5p, Cash+Token) (Showcase) (Datapak) (MPU4 Video)",GAME_FLAGS )

GAME(  1994, v4gldrsh3,  v4gldrsh, bwbvid,     v4reno,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Gold Rush (BWB) (Release 3, 20p Fixed, Cash+Token) (MPU4 Video)",GAME_FLAGS )


GAME(  199?, v4timebn,   0,        bwbvid_oki,    v4timebn, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Time Bandit (BWB) (Release 1, 20p Fixed, Cash + Tokens) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4timebna,  v4timebn, bwbvid_oki,    v4timebn, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Time Bandit (BWB) (Release 1, 20p Fixed, Cash + Tokens) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4timebnc,  v4timebn, bwbvid_oki,    v4timebn, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Time Bandit (BWB) (Release 1, 20p Fixed, Cash + Tokens) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4timebnb,  v4timebn, bwbvid_oki,    v4timebn, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Time Bandit (BWB) (Release 1, 20p Fixed, Cash + Tokens) (Datapak) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4timebnd,  v4timebn, bwbvid_oki,    v4timebn, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Time Bandit (BWB) (Release 1, 20p Fixed, Cash + Tokens) (Datapak) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4timebne,  v4timebn, bwbvid_oki,    v4timebn, mpu4vid_state, init_bwbhack,     ROT0, "BWB","Time Bandit (BWB) (Release 1, 20p Fixed, Cash + Tokens) (Datapak) (set 3) (MPU4 Video)",GAME_FLAGS )


// 'Release D'
GAME(  199?, v4monte,    0,        bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release D, S/Site Standard, 20p Switchable, £8 All Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montea,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release D, S/Site Standard, 20p Switchable, £10 All Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montee,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release D, S/Site Standard, 20p Switchable, Cash and Tokens) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monted,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release D, S/Site Standard, 20p Fixed, £10 All Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteg,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release D, S/Site Standard, 25p Fixed, £10 All Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montec,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release D, Arcade Special, 20p Fixed, Cash and Tokens) (MPU4 Video)",GAME_FLAGS )
// 'Release 9'
GAME(  199?, v4monte9,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release 9, S/Site Standard, Options 4 Cabinet, 20p Fixed, £8 All Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9a,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release 9, S/Site Standard, Options 4 Cabinet, 20p Switchable, £8 All Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9e,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release 9, S/Site Standard, Options 4 Cabinet, 20p Switchable, Cash and Tokens) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9b,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release 9, S/Site Standard, Options 4 Cabinet, 20p Switchable, Cash and Tokens) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9f,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release 9, S/Site Standard, Options 4 Cabinet, 20p Switchable, £10 All Cash) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9n,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release 9, S/Site Standard, Options 4 Cabinet, 20p Switchable, £10 All Cash) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9j,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release 9, S/Site Standard, Options 4 Cabinet, 20p Fixed, £10 All Cash) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9k,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release 9, S/Site Standard, Options 4 Cabinet, 20p Fixed, £10 All Cash) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9l,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release 9, S/Site Standard, Options 4 Cabinet, 25p Fixed, £10 All Cash) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9m,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release 9, S/Site Standard, Options 4 Cabinet, 25p Fixed, £10 All Cash) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9o,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release 9, S/Site Standard, Options 4 Cabinet, 20p Fixed, Cash and Tokens) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9i,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release 9, S/Site Standard, Options 4 Cabinet, 20p Fixed, Cash and Tokens) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9c,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release 9, Arcade Special, Options 4 Cabinet, 20p Switchable, Cash and Tokens) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9d,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release 9, Arcade Special, Options 4 Cabinet, 20p Switchable, Cash and Tokens) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9h,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release 9, Arcade Special, Options 4 Cabinet, 20p Fixed, Cash and Tokens) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte9g,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release 9, Arcade Special, Options 4 Cabinet, 20p Fixed, Cash and Tokens) (set 2) (MPU4 Video)",GAME_FLAGS )
// 'Release B'
GAME(  199?, v4monteb,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release B, S/Site Standard, 20p Fixed, £8 All Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteba,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release B, Arcade Special, 20p Fixed, Cash and Tokens) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montebb,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release B, Arcade Special, 20p Switchable, Cash and Tokens) (MPU4 Video)",GAME_FLAGS )
// 'Release 5' using the same base ROMs as 'Release B' (hacked?)
GAME(  199?, v4monte5,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Monte Carlo Or Bust (BWB) (Release 5, S/Site Standard, 20p Fixed, £8 All Cash) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte5a,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release 5, Arcade Special, 20p Fixed, Cash and Tokens) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monte5b,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",   "Monte Carlo Or Bust (BWB) (Release 5, Arcade Special, 20p Switchable, Cash and Tokens) (MPU4 Video)",GAME_FLAGS )
// no suitable 68k ROMs for these
GAME(  199?, v4montek,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montel,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montem,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monten,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 4) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteo,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 5) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montep,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 6) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteq,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 7) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monter,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 8) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montes,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 9) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montet,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 10) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteu,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 11) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montev,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 12) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montew,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 13) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montex,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 14) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montey,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 15) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montez,   v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 16) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteaa,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 17) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteab,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 18) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteac,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 19) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montead,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 20) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteae,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 21) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteaf,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 22) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteag,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 23) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteah,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 24) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteai,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 25) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteaj,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 26) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteak,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 27) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteal,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 28) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteam,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 29) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4montean,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 30) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4monteao,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 31) (MPU4 Video)",GAME_FLAGS )
// only have a single loose 68k ROM from this which doesn't match any other set
GAME(  199?, v4montezz,  v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (Release ?, set 32) (MPU4 Video)",GAME_FLAGS )
// German Release
GAME(  199?, v4monteger, v4monte,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB","Monte Carlo Or Bust (BWB) (German) (MPU4 Video)",GAME_FLAGS )


GAME(  1995, v4mdice,    0,        bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, S/Site Standard, 25p-£10 Cash - Fixed) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicee,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, S/Site Standard, 20p-£4 Cash - Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicep,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, S/Site Standard, 20p-£10 Cash - Fixed) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdiceq,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, S/Site Standard, 20p-£10 Cash - Fixed) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicer,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, S/Site Standard, 20p-£10 Cash - Fixed) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdiceu,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, S/Site Standard, 25p-£10 Cash - Fixed) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicev,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, S/Site Standard, 25p-£10 Cash - Fixed) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicei,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, S/Site Standard, 20p-£8 Token - Fixed) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicej,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, S/Site Standard, 20p-£8 Token - Fixed) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicel,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, S/Site Standard, 20p-£8 Token - Fixed) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicem,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, S/Site Standard, 20p-£8 Token - Fixed) (set 4) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicek,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, S/Site Showcase, 20p-£8 Token - Fixed) (set 5) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicea,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, Arcade Standard, 5p-£4 Cash - Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdiceb,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, Arcade Standard, 10p-£4 Cash - Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdiced,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, Arcade Standard, 20p-£4 Cash - Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicew,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, Arcade Standard, 20p-£8 Cash - Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicen,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, Arcade Standard, 20p-£10 Cash - Fixed) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdiceo,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, Arcade Standard, 20p-£10 Cash - Fixed) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdices,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, Arcade Standard, 25p-£10 Cash - Fixed) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicet,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, Arcade Standard, 25p-£10 Cash - Fixed) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicec,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, Arcade Standard, 10p-£8 Token - Fixed) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdicef,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, Arcade Standard, 20p-£8 Token - Fixed) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdiceh,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, Arcade Standard, 20p-£8 Token - Fixed) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  1995, v4mdiceg,   v4mdice,  bwbvid_oki,    v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB", u8"Miami Dice (BWB) (Release 8, Arcade Showcase, 20p-£8 Token - Fixed) (set 3) (MPU4 Video)",GAME_FLAGS )
// 'Release 6'
GAME(  1995, v4mdice6,   v4mdice,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"Miami Dice (BWB) (Release 6, Arcade Standard, 20p-£8 Token - Fixed) (MPU4 Video)", GAME_FLAGS )
// 'Release 5'
GAME(  1995, v4mdice5,   v4mdice,  bwbvid_oki,     v4mdice,  mpu4vid_state, init_bwbhack,     ROT0, "BWB",u8"Miami Dice (BWB) (Release 5, Arcade Showcase, 20p-£8 Token - Fixed) (MPU4 Video)",GAME_FLAGS )
// this is a German version of v4mdice, produced by Nova
GAME(  199?, v4mdiceger, v4mdice,  bwbvid_oki,    mpu4,     mpu4vid_state, init_bwbhack,      ROT0, "BWB (Nova license)","Miami Dice (Nova, German) (MPU4 Video)",GAME_FLAGS )


// other issues

// no video ROMs dumped!
GAME(  199?, v4megbuk,   0,        bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Megabucks Poker (BWB) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4megbuka,  v4megbuk, bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Megabucks Poker (BWB) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4megbukb,  v4megbuk, bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Megabucks Poker (BWB) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4megbukc,  v4megbuk, bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Megabucks Poker (BWB) (set 4) (MPU4 Video)",GAME_FLAGS )


// no video ROMs dumped!
GAME(  199?, v4rencas,   0,        bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Reno Casino (BWB) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rencasa,  v4rencas, bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Reno Casino (BWB) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rencasb,  v4rencas, bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Reno Casino (BWB) (set 3) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rencasc,  v4rencas, bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Reno Casino (BWB) (set 4) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rencasd,  v4rencas, bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Reno Casino (BWB) (set 5) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rencase,  v4rencas, bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Reno Casino (BWB) (set 6) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rencasf,  v4rencas, bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Reno Casino (BWB) (set 7) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rencasg,  v4rencas, bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Reno Casino (BWB) (set 8) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rencash,  v4rencas, bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Reno Casino (BWB) (set 9) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4rencasi,  v4rencas, bwbvid,     mpu4,     mpu4vid_state, init_bwbhack,     ROT0, "BWB","Reno Casino (BWB) (set 10) (MPU4 Video)",GAME_FLAGS )


/* Uncertain BIOS */
// has a Barcrest style Characteriser check, not a BWB one?
// 00 44 44 54 58 24 54 50 58 3c 34 18 7c 34 48 30 58 7c 7c 2c 70 00 04 4c 70 18 3c 64 44 54 00 14 48 70 58 3c 3c 64 04 44 44 44 5c 34 58 74 58 74 58 3c 7c 3c 64 54 58 34 50 18 7c 2c 70 00 5c 00
GAME(  199?, v4frfact,   v4bios,   crmaze,     bwbvid,   mpu4vid_state, empty_init,    ROT0, "BWB","Fruit Factory (BWB) (set 1) (MPU4 Video)", GAME_FLAGS )
GAME(  199?, v4frfacta,  v4frfact, crmaze,     bwbvid,   mpu4vid_state, empty_init,    ROT0, "BWB","Fruit Factory (BWB) (set 2) (MPU4 Video)", GAME_FLAGS )
GAME(  199?, v4frfactb,  v4frfact, crmaze,     bwbvid,   mpu4vid_state, empty_init,    ROT0, "BWB","Fruit Factory (BWB) (set 3) (MPU4 Video)", GAME_FLAGS )
GAME(  199?, v4frfactc,  v4frfact, crmaze,     bwbvid,   mpu4vid_state, empty_init,    ROT0, "BWB","Fruit Factory (BWB) (set 4) (MPU4 Video)", GAME_FLAGS )
GAME(  199?, v4frfactd,  v4frfact, crmaze,     bwbvid,   mpu4vid_state, empty_init,    ROT0, "BWB","Fruit Factory (BWB) (set 5) (MPU4 Video)", GAME_FLAGS )
GAME(  199?, v4frfacte,  v4frfact, crmaze,     bwbvid,   mpu4vid_state, empty_init,    ROT0, "BWB","Fruit Factory (BWB) (set 6) (MPU4 Video)", GAME_FLAGS )
GAME(  199?, v4frfactf,  v4frfact, crmaze,     bwbvid,   mpu4vid_state, empty_init,    ROT0, "BWB","Fruit Factory (BWB) (set 7) (MPU4 Video)", GAME_FLAGS )

/* Nova - is this the same video board? One of the games displays 'Resetting' but the others do nothing interesting and access strange addresses */
/* All contain BWB video in the BIOS rom tho, Cyber Casino also needs a Jackpot link? */
/* These seem to use the other palette chip (BT471). and use the German BWB bank setup, so may need more work */

GAME(  199?, v4cybcas,   0,        bwbvid_oki_bt471_german,    v4cybcas,   mpu4vid_state, init_cybcas,     ROT0, "BWB (Nova license)","Cyber Casino (Nova, German) (MPU4 Video)",GAME_FLAGS )

GAME(  199?, v4missis,   0,        bwbvid_oki_bt471_german,    v4cybcas,   mpu4vid_state, init_bwbhack,    ROT0, "BWB (Nova license)","Mississippi Lady (Nova, German) (MPU4 Video)",GAME_FLAGS ) // different hardware type? extra ram on mpu4 side?

GAME(  199?, v4picdil,   0,        bwbvid_oki_bt471_german,    v4cybcas,    mpu4vid_state, init_bwbhack,    ROT0, "BWB (Nova license)","Piccadilly Night (Nova, German) (set 1) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4picdila,  v4picdil, bwbvid_oki_bt471_german,    v4cybcas,    mpu4vid_state, init_bwbhack,    ROT0, "BWB (Nova license)","Piccadilly Night (Nova, German) (set 2) (MPU4 Video)",GAME_FLAGS )
GAME(  199?, v4picdilz,  v4picdil, bwbvid_oki_bt471_german,    v4cybcas,    mpu4vid_state, init_bwbhack,    ROT0, "BWB (Nova license)","Piccadilly Night (Nova, German) (set 3) (MPU4 Video)",GAME_FLAGS )

