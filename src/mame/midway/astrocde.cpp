// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Mike Coates, Frank Palazzolo, Aaron Giles
/****************************************************************************

    Bally Astrocade style games
    driver by Nicola Salmoria, Mike Coates, Frank Palazzolo, Aaron Giles

    Games supported:
        * Seawolf II
        * Extra Bases
        * Space Zap
        * Wizard of Wor
        * Gorf
        * (The Adventures of) Robby Roto
        * Professor Pac-Man
        * Demons & Dragons
        * Ten Pix Deluxe

    Known bugs:
        * No audio board for Demons & Dragons
        * Demons & Dragons doesn't work with RAM protection enabled
        * Professor Pac-Man fails screen RAM test

****************************************************************************

    Game boards:
        90002 = Seawolf II Motherboard (seawolf2)
        90700 = Seawolf II Logic Board (seawolf2, ebases)
        91312 = Characterization Card (seawolf2)
        91354 = CPU Board (ebases, spacezap, wow, gorf, robby)
        91355 = Pattern Board (spacezap, wow, gorf, robby)
        91356 = RAM Board (ebases, spacezap, wow, gorf, robby)
        91364 = ROM/RAM Board (gorf)
        91397 = Memory Board (wow)
        91423 = Memory Board (robby)
        91465 = 16 Color CPU Card (profpac/tenpindx)
        91466 = Screen RAM Board (profpac/tenpindx)
        91467 = Super Game Memory (profpac/tenpindx)
        91469 = Game Board (profpac)
        91488 = Pattern Mover (appears to be identical to 91355) (profpac/tenpindx)
        91699 = Sound I/O Board (tenpindx)
        91846 = 640K EPROM board (profpac)

    Seawolf II:
        90002 = Seawolf II Motherboard
        90700 = Seawolf II Logic Board
        91312 = Characterization Card

    Extra Bases:
        90700 = Game I/O board
        91354 = CPU Board
        91356 = RAM Board

    Space Zap:
        90706 = Space Zap Game Board
        91354 = CPU Board
        91355 = Pattern Board
        91356 = RAM Board

    Wizard of Wor:
        90708 = Game Board
        91354 = CPU Board
        91355 = Pattern Transfer Board
        91356 = RAM Board
        91397 = Memory Board

    Gorf:
        90708 = Game Board
        91354 = CPU Board
        91355 = Pattern Board
        91356 = RAM Board
        91364 = ROM/RAM Board

    Robby Roto:
        90708 = Game Board
        91354 = CPU Board
        91355 = Pattern Board
        91356 = RAM Board
        91423 = Memory Board

    Professor Pac-Man:
        91465 = 16 Color CPU Card
        91466 = Screen RAM Board
        91467 = Super Game Memory
        91469 = Game Board
        91488 = Pattern Mover (appears to be identical to 91355)
        91846 = 640K EPROM board

    Ten Pin Deluxe:
        91456 = 16 Color CPU Card
        91466 = Screen RAM Board
        91467 = Super Game Memory
        91488 = Pattern Mover
        91699 = Sound I/O Board

****************************************************************************

    Notes:
    - In seawolf2, service mode dip switch turns on memory test. Reset with
      2 pressed to get to an input check screen, reset with 1+2 pressed to
      get to a convergence test screen.
    - Foreign language ROMs aren't tested by the ROM checks

****************************************************************************

    DIP locations verified for:
    - seawolf2 (manual)
    - wow (manual)
    - spacezap (manual)
    - gorf (manual)
    - robby (manual)
    - profpac (manual)

****************************************************************************/

#include "emu.h"
#include "astrocde.h"

#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/nvram.h"
#include "machine/74259.h"
#include "machine/output_latch.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/votrax.h"
#include "sound/flt_rc.h"

#include "speaker.h"

#include "gorf.lh"
#include "seawolf2.lh"
#include "spacezap.lh"
#include "tenpindx.lh"


/*************************************
 *
 *  Protected RAM
 *
 *  If not specified in the memory map,
 *  then it's at the start of the nvram
 *
 *************************************/

void astrocde_state::protected_ram_enable_w(uint8_t data)
{
	m_ram_write_enable = true;
}


uint8_t astrocde_state::protected_ram_r(offs_t offset)
{
	m_ram_write_enable = false;
	return m_protected_ram ? m_protected_ram[offset] : m_nvram[offset];
}


void astrocde_state::protected_ram_w(offs_t offset, uint8_t data)
{
	if (m_ram_write_enable)
		(m_protected_ram ? m_protected_ram : m_nvram)[offset] = data;
	m_ram_write_enable = false;
}



/*************************************
 *
 *  Seawolf II specific input/output
 *
 *************************************/

void seawolf2_state::sound_1_w(uint8_t data) // Port 40
{
	uint8_t rising_bits = data & ~m_port_1_last;
	m_port_1_last = data;

	if (rising_bits & 0x01) m_samples->start(1, 1);  /* Left Torpedo */
	if (rising_bits & 0x02) m_samples->start(0, 0);  /* Left Ship Hit */
	if (rising_bits & 0x04) m_samples->start(4, 4);  /* Left Mine Hit */
	if (rising_bits & 0x08) m_samples->start(6, 1);  /* Right Torpedo */
	if (rising_bits & 0x10) m_samples->start(5, 0);  /* Right Ship Hit */
	if (rising_bits & 0x20) m_samples->start(9, 4);  /* Right Mine Hit */
}


void seawolf2_state::sound_2_w(uint8_t data) // Port 41
{
	uint8_t rising_bits = data & ~m_port_2_last;
	m_port_2_last = data;

	m_samples->set_volume(0, (data & 0x80) ? 1.0 : 0.0);
	m_samples->set_volume(1, (data & 0x80) ? 1.0 : 0.0);
	m_samples->set_volume(3, (data & 0x80) ? 1.0 : 0.0);
	m_samples->set_volume(4, (data & 0x80) ? 1.0 : 0.0);
	m_samples->set_volume(5, (data & 0x80) ? 1.0 : 0.0);
	m_samples->set_volume(6, (data & 0x80) ? 1.0 : 0.0);
	m_samples->set_volume(8, (data & 0x80) ? 1.0 : 0.0);
	m_samples->set_volume(9, (data & 0x80) ? 1.0 : 0.0);

	/* dive panning controlled by low 3 bits */
	m_samples->set_volume(2, (float)(~data & 0x07) / 7.0f);
	m_samples->set_volume(7, (float)(data & 0x07) / 7.0f);

	if (rising_bits & 0x08)
	{
		m_samples->start(2, 2);
		m_samples->start(7, 2);
	}
	if (rising_bits & 0x10) m_samples->start(8, 3);  /* Right Sonar */
	if (rising_bits & 0x20) m_samples->start(3, 3);  /* Left Sonar */

	machine().bookkeeping().coin_counter_w(0, data & 0x40);    /* Coin Counter */
}



/*************************************
 *
 *  Generic input/output
 *
 *************************************/

uint8_t astrocde_state::input_mux_r(offs_t offset)
{
	return m_handle[offset & 3].read_safe(0xff);
}


template<int Coin>
void astrocde_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Coin, state);
}


template<int Bit>
void astrocde_state::sparkle_w(int state)
{
	m_sparkle[Bit] = state;
}



/*************************************
 *
 *  Extra Bases specific input/output
 *
 *************************************/

ioport_value ebases_state::trackball_r()
{
	return (m_trackball[m_input_select]->read() - m_trackball_last) & 0xff;
}


void ebases_state::trackball_select_w(uint8_t data)
{
	m_input_select = data & 3;
	m_trackball_last = m_trackball[m_input_select]->read();
}


void ebases_state::coin_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
}



/*************************************
 *
 *  Gorf specific input/output
 *
 *************************************/

void astrocde_state::gorf_sound_switch_w(int state)
{
	m_astrocade_sound[0]->set_output_gain(0, state ? 0.0 : 1.0);
	m_votrax->set_output_gain(0, state ? 1.0 : 0.0);
}



/*************************************
 *
 *  Demons & Dragons specific input/output
 *
 *************************************/

void astrocde_state::demndrgn_banksw_w(uint8_t data)
{
	int bank = (data >> 5) & 3;
	m_bank4000->set_bank(bank);
	m_bank8000->set_entry(bank);
}

void demndrgn_state::input_select_w(int state)
{
	m_input_select = state;
}

ioport_value demndrgn_state::trackball_r()
{
	return (m_trackball[m_input_select]->read() - m_trackball_last) & 0xff;
}

void demndrgn_state::trackball_reset_w(uint8_t data)
{
	m_trackball_last = m_trackball[m_input_select]->read();
}


void demndrgn_state::sound_w(uint8_t data)
{
	logerror("Trigger sound sample 0x%02x\n",data);
}



/*************************************
 *
 *  Professor Pac-Man specific input/output
 *
 *************************************/


void astrocde_state::profpac_banksw_w(uint8_t data)
{
	demndrgn_banksw_w(data);

	if (data & 0x80)
	{
		/* Note: There is a jumper which could change the base offset to 0xa8 instead */
		int bank = data - 0x80;

		/* if the bank is in range, map the appropriate bank */
		if (bank < 0x28)
			m_bank4000->set_bank(4 + bank);
		else
			m_bank4000->set_bank(4 + 0x28);
	}
}



/*************************************
 *
 *  Ten Pin Deluxe specific input/output
 *
 *************************************/

void tenpindx_state::lamp_w(offs_t offset, uint8_t data)
{
	/* lamps */
	if (offset == 0)
	{
		m_lamps[0] = BIT(data, 2);
		m_lamps[1] = BIT(data, 3);
		m_lamps[2] = BIT(data, 4);
		m_lamps[3] = BIT(data, 5);
		m_lamps[4] = BIT(data, 6);
		m_lamps[5] = BIT(data, 7);
	}
	else
	{
		m_lamps[6] = BIT(data, 0);
		m_lamps[7] = BIT(data, 1);
		m_lamps[8] = BIT(data, 2);
		m_lamps[9] = BIT(data, 3);
	}
}


void tenpindx_state::counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	if (data & 0xfc) osd_printf_debug("tenpindx_counter_w = %02X\n", data);
}


void tenpindx_state::lights_w(uint8_t data)
{
	/* "flashlights" */
	int which = data >> 4;

	m_lamps[10] = (which == 1);
	m_lamps[11] = (which == 2);
	m_lamps[12] = (which == 3);
	m_lamps[13] = (which == 4);
	m_lamps[14] = (which == 5);
	m_lamps[15] = (which == 6);
	m_lamps[16] = (which == 7);
	m_lamps[17] = (which == 8);
	m_lamps[18] = (which == 9);
}


/*************************************
 *
 *  Votrax SC01 specific input/output
 *
 *************************************/

void astrocde_state::votrax_speech_w(uint8_t data)
{
	// Note that the frequency change is smooth, but we just can't
	// handle that.
	m_votrax->set_clock(data & 0x40 ? 782000 : 756000);
	m_votrax->inflection_w(data & 0x80 ? 0 : 2);
	m_votrax->write(data & 0x3f);
}


int astrocde_state::votrax_speech_status_r()
{
	return m_votrax->request();
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void astrocde_state::seawolf2_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x0000, 0x3fff).w(FUNC(astrocde_state::astrocade_funcgen_w));
	map(0x4000, 0x7fff).ram().share("videoram");
	map(0xc000, 0xc3ff).ram();
}


void ebases_state::ebases_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x0000, 0x3fff).w(FUNC(astrocde_state::astrocade_funcgen_w));
	map(0x4000, 0x7fff).ram().share("videoram");
}


void astrocde_state::spacezap_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x0000, 0x3fff).w(FUNC(astrocde_state::astrocade_funcgen_w));
	map(0x4000, 0x7fff).ram().share("videoram");
	map(0xd000, 0xd03f).rw(FUNC(astrocde_state::protected_ram_r), FUNC(astrocde_state::protected_ram_w)).share("protected_ram");
	map(0xd040, 0xd7ff).ram();
}


void astrocde_state::wow_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x0000, 0x3fff).w(FUNC(astrocde_state::astrocade_funcgen_w));
	map(0x4000, 0x7fff).ram().share("videoram");
	map(0x8000, 0xcfff).rom();
	map(0xd000, 0xd03f).rw(FUNC(astrocde_state::protected_ram_r), FUNC(astrocde_state::protected_ram_w)).share("protected_ram");
	map(0xd040, 0xdfff).ram();
}


void astrocde_state::robby_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x0000, 0x3fff).w(FUNC(astrocde_state::astrocade_funcgen_w));
	map(0x4000, 0x7fff).ram().share("videoram");
	map(0x8000, 0xdfff).rom();
	map(0xe000, 0xe7ff).ram().share("nvram");
	map(0xe000, 0xe1ff).rw(FUNC(astrocde_state::protected_ram_r), FUNC(astrocde_state::protected_ram_w));
	map(0xe800, 0xffff).ram();
}


void astrocde_state::demndrgn_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x0000, 0x3fff).w(FUNC(astrocde_state::astrocade_funcgen_w));
	map(0x4000, 0x7fff).r(m_bank4000, FUNC(address_map_bank_device::read8)).w(FUNC(astrocde_state::profpac_videoram_w));
	map(0x8000, 0xbfff).bankr("bank8000");
	map(0xc000, 0xdfff).rom();
	map(0xe000, 0xe7ff).ram().share("nvram");
	map(0xe800, 0xffff).ram();
}


void astrocde_state::profpac_map(address_map &map)
{
	demndrgn_map(map);
	map(0xe000, 0xe1ff).rw(FUNC(astrocde_state::protected_ram_r), FUNC(astrocde_state::protected_ram_w));
}


void astrocde_state::bank4000_map(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(astrocde_state::profpac_videoram_r));
	map(0x4000, 0x7fff).rom().region("banks", 0x08000);
	map(0x8000, 0xbfff).rom().region("banks", 0x10000);
	map(0xc000, 0xffff).rom().region("banks", 0x18000);
}


void astrocde_state::profpac_bank4000_map(address_map &map)
{
	bank4000_map(map);
	map(0x10000, 0xaffff).rom().region("epromboard", 0);
	map(0xb0000, 0xb3fff).nopr();
}


void tenpindx_state::sub_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xc000, 0xc7ff).ram();
}



/*************************************
 *
 *  Port maps
 *
 *************************************/

void astrocde_state::port_map(address_map &map)
{
	map(0x0000, 0x000f).select(0xff00).rw(FUNC(astrocde_state::video_register_r), FUNC(astrocde_state::video_register_w));
	map(0x0010, 0x001f).select(0xff00).r(m_astrocade_sound[0], FUNC(astrocade_io_device::read));
	map(0x0010, 0x0018).select(0xff00).w(m_astrocade_sound[0], FUNC(astrocade_io_device::write));
	map(0x0019, 0x0019).mirror(0xff00).w(FUNC(astrocde_state::expand_register_w));
}


void seawolf2_state::port_map_discrete(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0f).rw(FUNC(astrocde_state::video_register_r), FUNC(astrocde_state::video_register_w));
	map(0x10, 0x10).portr("P1HANDLE");
	map(0x11, 0x11).portr("P2HANDLE");
	map(0x12, 0x12).portr("P3HANDLE");
	map(0x13, 0x13).portr("P4HANDLE");
	map(0x19, 0x19).w(FUNC(astrocde_state::expand_register_w));
	map(0x40, 0x40).mirror(0x18).w(FUNC(seawolf2_state::sound_1_w));
	map(0x41, 0x41).mirror(0x18).w(FUNC(seawolf2_state::sound_2_w));
	map(0x42, 0x42).mirror(0x18).w("lamplatch1", FUNC(output_latch_device::write));
	map(0x43, 0x43).mirror(0x18).w("lamplatch0", FUNC(output_latch_device::write));
}


void ebases_state::port_map_ebases(address_map &map)
{
	port_map(map);
	map(0x0020, 0x0020).mirror(0xff07).w(FUNC(ebases_state::coin_w));
	map(0x0028, 0x0028).mirror(0xff07).w(FUNC(ebases_state::trackball_select_w));
}


void astrocde_state::port_map_mono_pattern(address_map &map)
{
	port_map(map);
	map(0x0078, 0x007e).mirror(0xff00).w(FUNC(astrocde_state::astrocade_pattern_board_w));
	map(0xa55b, 0xa55b).w(FUNC(astrocde_state::protected_ram_enable_w));
}


void astrocde_state::port_map_stereo_pattern(address_map &map)
{
	port_map_mono_pattern(map);
	map(0x0050, 0x0058).select(0xff00).w(m_astrocade_sound[1], FUNC(astrocade_io_device::write));
}


void astrocde_state::port_map_16col_pattern(address_map &map)
{
	port_map_stereo_pattern(map);
	map(0x00bf, 0x00bf).mirror(0xff00).w(FUNC(astrocde_state::profpac_page_select_w));
	map(0x00c3, 0x00c3).mirror(0xff00).r(FUNC(astrocde_state::profpac_intercept_r));
	map(0x00c0, 0x00c5).mirror(0xff00).w(FUNC(astrocde_state::profpac_screenram_ctrl_w));
	map(0x00f3, 0x00f3).mirror(0xff00).w(FUNC(astrocde_state::profpac_banksw_w));
}


void astrocde_state::port_map_16col_pattern_nosound(address_map &map)
{
	map(0x0000, 0x000f).select(0xff00).rw(FUNC(astrocde_state::video_register_r), FUNC(astrocde_state::video_register_w));
	map(0x0019, 0x0019).mirror(0xff00).w(FUNC(astrocde_state::expand_register_w));
	map(0x0078, 0x007e).mirror(0xff00).w(FUNC(astrocde_state::astrocade_pattern_board_w));
	map(0x00bf, 0x00bf).mirror(0xff00).w(FUNC(astrocde_state::profpac_page_select_w));
	map(0x00c3, 0x00c3).mirror(0xff00).r(FUNC(astrocde_state::profpac_intercept_r));
	map(0x00c0, 0x00c5).mirror(0xff00).w(FUNC(astrocde_state::profpac_screenram_ctrl_w));
	map(0x00f3, 0x00f3).mirror(0xff00).w(FUNC(astrocde_state::demndrgn_banksw_w));
	map(0xa55b, 0xa55b).w(FUNC(astrocde_state::protected_ram_enable_w));
}


void demndrgn_state::port_map_16col_pattern_demndrgn(address_map &map)
{
	port_map_16col_pattern_nosound(map);
	map(0x0010, 0x001f).select(0xff00).r(m_astrocade_sound[0], FUNC(astrocade_io_device::read));
	map(0x0097, 0x0097).mirror(0xff00).w(FUNC(demndrgn_state::sound_w));
}


void tenpindx_state::port_map_16col_pattern_tenpindx(address_map &map)
{
	port_map_16col_pattern_nosound(map);
	map(0x0060, 0x0060).mirror(0xff00).portr("P60");
	map(0x0061, 0x0061).mirror(0xff00).portr("P61");
	map(0x0062, 0x0062).mirror(0xff00).portr("P62");
	map(0x0063, 0x0063).mirror(0xff00).portr("P63");
	map(0x0064, 0x0064).mirror(0xff00).portr("P64");
	map(0x0065, 0x0066).mirror(0xff00).w(FUNC(tenpindx_state::lamp_w));
	map(0x0067, 0x0067).mirror(0xff00).w(FUNC(tenpindx_state::counter_w));
	map(0x0068, 0x0068).mirror(0xff00).w(FUNC(tenpindx_state::lights_w));
	map(0x0097, 0x0097).mirror(0xff00).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}


void tenpindx_state::sub_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x90, 0x93).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x97, 0x97).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x98, 0x98).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x98, 0x98).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x9a, 0x9a).w("aysnd", FUNC(ay8910_device::data_w));
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static const ioport_value controller_table[64] =
{
	0x20, 0x21, 0x23, 0x22, 0x26, 0x27, 0x25, 0x24,
	0x2c, 0x2d, 0x2f, 0x2e, 0x2a, 0x2b, 0x29, 0x28,
	0x38, 0x39, 0x3b, 0x3a, 0x3e, 0x3f, 0x3d, 0x3c,
	0x34, 0x35, 0x37, 0x36, 0x32, 0x33, 0x31, 0x30,
	0x10, 0x11, 0x13, 0x12, 0x16, 0x17, 0x15, 0x14,
	0x1c, 0x1d, 0x1f, 0x1e, 0x1a, 0x1b, 0x19, 0x18,
	0x08, 0x09, 0x0b, 0x0a, 0x0e, 0x0f, 0x0d, 0x0c,
	0x04, 0x05, 0x07, 0x06, 0x02, 0x03, 0x01, 0x00
};

static INPUT_PORTS_START( seawolf2 )
	PORT_START("P1HANDLE")
	PORT_BIT( 0x3f, 0x1f, IPT_POSITIONAL ) PORT_PLAYER(2) PORT_POSITIONS(64) PORT_REMAP_TABLE(controller_table) PORT_SENSITIVITY(20) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_CROSSHAIR(X, 2.0, -0.40, 34.0 / 240.0)
	PORT_DIPNAME( 0x40, 0x00, "Language 1" )        PORT_DIPLOCATION("S2:!1")
	PORT_DIPSETTING(    0x00, "Language 2" )
	PORT_DIPSETTING(    0x40, DEF_STR( French ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("P2HANDLE")
	PORT_BIT( 0x3f, 0x1f, IPT_POSITIONAL ) PORT_PLAYER(1) PORT_POSITIONS(64) PORT_REMAP_TABLE(controller_table) PORT_SENSITIVITY(20) PORT_KEYDELTA(4) PORT_CENTERDELTA(0) PORT_CROSSHAIR(X, 2.0, -0.45, 34.0 / 240.0)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("P3HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x08, 0x00, "Language 2" )        PORT_DIPLOCATION("S2:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x08, DEF_STR( German ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P4HANDLE")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )  PORT_DIPLOCATION("S1:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x06, 0x00, "Play Time" )         PORT_DIPLOCATION("S1:!3,!4")
	PORT_DIPSETTING(    0x06, "1P 40s/2P 45s" )     /* Extended play: 1P 20s/2P 20s */
	PORT_DIPSETTING(    0x04, "1P 50s/2P 60s" )     /* Extended play: 1P 25s/2P 30s */
	PORT_DIPSETTING(    0x02, "1P 60s/2P 75s" )     /* Extended play: 1P 30s/2P 35s */
	PORT_DIPSETTING(    0x00, "1P 70s/2P 90s" )     /* Extended play: 1P 35s/2P 45s */
	PORT_DIPNAME( 0x08, 0x08, "2 Players Game" )    PORT_DIPLOCATION("S1:!1")
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x08, "2 Credits" )
	PORT_DIPNAME( 0x30, 0x00, "Extended Play" )     PORT_DIPLOCATION("S1:!5,!6")
	PORT_DIPSETTING(    0x10, "5000" )
	PORT_DIPSETTING(    0x20, "6000" )
	PORT_DIPSETTING(    0x30, "7000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, "Monitor" )           PORT_DIPLOCATION("S1:!7")
	PORT_DIPSETTING(    0x40, "Color" )
	PORT_DIPSETTING(    0x00, "B/W" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "S1:!8")
INPUT_PORTS_END


static INPUT_PORTS_START( ebases )
	PORT_START("P1HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0c, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Monitor" )           PORT_DIPLOCATION( "JU:1" )
	PORT_DIPSETTING(    0x00, "Color" )
	PORT_DIPSETTING(    0x10, "B/W" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )  PORT_DIPLOCATION( "JU:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3HANDLE")
	PORT_DIPNAME( 0x01, 0x00, "2 Players Game" )    PORT_DIPLOCATION( "S1:1" )
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x01, "2 Credits" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "S1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "S1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "S1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "S1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "S1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "S1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "S1:8" )

	PORT_START("P4HANDLE")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(ebases_state::trackball_r))

	PORT_START("TRACKX1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("TRACKY1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("TRACKX2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("TRACKY2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("FAKE")
	/* Cocktail cabinets had a B/W monitor with color overlay (same one as Space Zap!),
	   upright cabinets at factory default had a B/W monitor too, with a baseball stadium background. */
	PORT_CONFNAME( 0x01, 0x00, "Monitor" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrocde_state::spacezap_monitor), 0)
	PORT_CONFSETTING(    0x00, "B/W" )
	PORT_CONFSETTING(    0x01, "Color" )
INPUT_PORTS_END


INPUT_CHANGED_MEMBER(astrocde_state::spacezap_monitor)
{
	if (newval)
		m_video_config &= ~AC_MONITOR_BW;
	else
		m_video_config |= AC_MONITOR_BW;
}

static INPUT_PORTS_START( spacezap )
	PORT_START("P1HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )  // manual says this dip is unused on cocktail cabs
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 ) // starts a 1 player game if 1 credit
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Aim Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Aim Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_NAME("P2 Aim Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_NAME("P2 Aim Right")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) ) PORT_DIPLOCATION("JU:1")
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Aim Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Aim Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Aim Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Aim Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("JU:2")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4HANDLE")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coin_A ) )   PORT_DIPLOCATION( "S1:1" )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coin_B ) )   PORT_DIPLOCATION( "S1:2,3" )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "S1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "S1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "S1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "S1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "S1:8" )

	PORT_START("FAKE")
	/* Dedicated cabinets had a B/W monitor and color overlay,
	   some (unofficial/repaired?) cabinets had a color monitor. */
	PORT_CONFNAME( 0x01, 0x00, "Monitor" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrocde_state::spacezap_monitor), 0)
	PORT_CONFSETTING(    0x00, "B/W" )
	PORT_CONFSETTING(    0x01, "Color" )
INPUT_PORTS_END


static INPUT_PORTS_START( wow )
	PORT_START("P1HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("JU:1") /* Undocumented, jumper? */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P2HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(astrocde_state::votrax_speech_status_r))

	PORT_START("P4HANDLE")
	/* "If S1:1,2,3 are all ON or all OFF, only coin meter number 1 will count." */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("S1:2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Language ) )     PORT_DIPLOCATION("S1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, "Foreign (NEED ROM)" )    /* "Requires A082-91374-A000" */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING(    0x10, "2 for 1 Credit / 5 for 2 Credits" )
	PORT_DIPSETTING(    0x00, "3 for 1 Credit / 7 for 2 Credits" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("S1:6")
	PORT_DIPSETTING(    0x20, "After 3rd Level" )
	PORT_DIPSETTING(    0x00, "After 4th Level" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("S1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("S1:8")
	PORT_DIPSETTING(    0x00, "On only when controls are touched" ) /* "Touching controls will enable attract sound for 1 cycle." */
	PORT_DIPSETTING(    0x80, "Always On"  )
INPUT_PORTS_END


static INPUT_PORTS_START( wowg )
	PORT_INCLUDE(wow)

	PORT_MODIFY("P4HANDLE")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("S1:4") /* Default it to Foreign because this set has the German ROM */
	PORT_DIPSETTING(    0x08, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, "Foreign (German ROM)" )
INPUT_PORTS_END


static INPUT_PORTS_START( gorf )
	PORT_START("P1HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("JU:1")    /* Jumper */
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Speech" )                PORT_DIPLOCATION("JU:2")    /* Jumper */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P2HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(astrocde_state::votrax_speech_status_r))

	PORT_START("P4HANDLE")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("S1:2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Language ) )     PORT_DIPLOCATION("S1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, "Foreign (NEED ROM)" )    /* "Requires A082-91374-A000" */
	PORT_DIPNAME( 0x10, 0x00, "Lives per Credit" )      PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("S1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "Mission 5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("S1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("S1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gorfpgm1f )
	PORT_INCLUDE(gorf)

	PORT_MODIFY("P4HANDLE")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("S1:4") /* Default it to Foreign because this set has the French ROM */
	PORT_DIPSETTING(    0x08, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, "Foreign (French ROM)" )
INPUT_PORTS_END

static INPUT_PORTS_START( gorfpgm1g )
	PORT_INCLUDE(gorf)

	PORT_MODIFY("P4HANDLE")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("S1:4") /* Default it to Foreign because this set has the German ROM */
	PORT_DIPSETTING(    0x08, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, "Foreign (German ROM)" )
INPUT_PORTS_END

static INPUT_PORTS_START( gorfirec )
	PORT_INCLUDE(gorf)

	PORT_MODIFY("P4HANDLE")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Language ) )     PORT_DIPLOCATION("S1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x00, "Foreign (NEED ROM)" )    // "Requires A082-91374-A000"
INPUT_PORTS_END


static INPUT_PORTS_START( robby )
	PORT_START("P1HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPUNUSED( 0x80, 0x00 )

	PORT_START("P2HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4HANDLE")
	PORT_DIPNAME( 0x01, 0x01, "Use NVRAM" )             PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Use Service Mode Settings" ) PORT_DIPLOCATION("S1:2")
	PORT_DIPSETTING(    0x00, "Reset" )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("S1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("S1:4")    /* Listed as "Unused". */
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "S1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "S1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "S1:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("S1:8")    /* Listed as "Unused". */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( profpac )
	PORT_START("P1HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* Left A */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* Left B */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) /* Left C */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* Right A */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* Right B */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* Right C */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P3HANDLE")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P4HANDLE")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )  /* Upright or Mini */
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, "Reset on powerup" )  PORT_DIPLOCATION("S1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Halt on error" )     PORT_DIPLOCATION("S1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Beep" )              PORT_DIPLOCATION("S1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "ROM's Used" )        PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING(    0x10, "8K & 16K ROM's" )
	PORT_DIPSETTING(    0x00, "32K ROM's" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "S1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "S1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "S1:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( demndrgn )
	PORT_START("P1HANDLE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2HANDLE")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(demndrgn_state::trackball_r))

	PORT_START("P3HANDLE")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P4HANDLE")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "S1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "S1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "S1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "S1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "S1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "S1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "S1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "S1:8" )

	PORT_START("MOVEX")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(2)

	PORT_START("MOVEY")
	PORT_BIT( 0xff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(2)

	PORT_START("FIREX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_REVERSE

	PORT_START("FIREY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( tenpindx )
	PORT_START("P60")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )    /* select game */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )    /* number of players */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )     /* start game */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P61")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, "Lockup" )            PORT_DIPLOCATION("S1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Reset" )             PORT_DIPLOCATION("S1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Beep" )              PORT_DIPLOCATION("S1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Regulation" )        PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Ticket Dispenser" )  PORT_DIPLOCATION("S1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Bill Acceptor" )     PORT_DIPLOCATION("S1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "S1:8" )

	PORT_START("P62")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_CUSTOM )    /* F1-F8 */

	PORT_START("P63")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_CUSTOM )    /* F9-F0,P1-P6 */

	PORT_START("P64")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_CUSTOM )    /* P7-P0 */

	PORT_START("DIPSW")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "S2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "S2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "S2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "S2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "S2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "S2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "S2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "S2:8" )
INPUT_PORTS_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const char *const seawolf_sample_names[] =
{
	"*seawolf",
	"shiphit",
	"torpedo",
	"dive",
	"sonar",
	"minehit",
	nullptr
};



/*************************************
 *
 *  CPU configurations
 *
 *************************************/

static const z80_daisy_config tenpin_daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};



/*************************************
 *
 *  Generic machine drivers
 *
 *************************************/

void astrocde_state::astrocade_base(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, ASTROCADE_CLOCK/4);
	/* each game has its own map */

	/* video hardware */
	PALETTE(config, m_palette, FUNC(astrocde_state::astrocade_palette), 512);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(ASTROCADE_CLOCK, 455, 0, 352, 262, 0, 240);
	m_screen->set_default_position(1.1, 0.0, 1.18, -0.018); // clip out borders
	m_screen->set_screen_update(FUNC(astrocde_state::screen_update_astrocde));
	m_screen->set_palette(m_palette);
}

void astrocde_state::astrocade_16color_base(machine_config &config)
{
	astrocade_base(config);

	/* basic machine hardware */
	ADDRESS_MAP_BANK(config, m_bank4000).set_map(&astrocde_state::bank4000_map).set_options(ENDIANNESS_LITTLE, 8, 16, 0x4000);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	m_palette->set_entries(4096);
	m_palette->set_init(FUNC(astrocde_state::profpac_palette));

	MCFG_VIDEO_START_OVERRIDE(astrocde_state,profpac)

	m_screen->set_screen_update(FUNC(astrocde_state::screen_update_profpac));
}


void astrocde_state::astrocade_mono_sound(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ASTROCADE_IO(config, m_astrocade_sound[0], ASTROCADE_CLOCK/4);
	m_astrocade_sound[0]->si_cb().set(FUNC(astrocde_state::input_mux_r));
	m_astrocade_sound[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}


void astrocde_state::astrocade_stereo_sound(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	ASTROCADE_IO(config, m_astrocade_sound[0], ASTROCADE_CLOCK/4);
	m_astrocade_sound[0]->si_cb().set(FUNC(astrocde_state::input_mux_r));
	m_astrocade_sound[0]->so_cb<0>().set("watchdog", FUNC(watchdog_timer_device::reset_w));
	m_astrocade_sound[0]->add_route(ALL_OUTPUTS, "speaker", 0.35, 0);

	ASTROCADE_IO(config, m_astrocade_sound[1], ASTROCADE_CLOCK/4).add_route(ALL_OUTPUTS, "speaker", 0.35, 1);

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 128); // MC14024B on CPU board at U18, CLK = VERTDR, Q7 used for RESET
}



/*************************************
 *
 *  Game specific machine drivers
 *
 *************************************/

void seawolf2_state::seawolf2(machine_config &config)
{
	astrocade_base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &seawolf2_state::seawolf2_map);
	m_maincpu->set_addrmap(AS_IO, &seawolf2_state::port_map_discrete);

	output_latch_device &lamplatch0(OUTPUT_LATCH(config, "lamplatch0")); // 74174 on game board at N2
	lamplatch0.bit_handler<0>().set_output("lamp6"); // right player torpedo 4 available
	lamplatch0.bit_handler<1>().set_output("lamp5"); // right player torpedo 3 available
	lamplatch0.bit_handler<2>().set_output("lamp4"); // right player torpedo 2 available
	lamplatch0.bit_handler<3>().set_output("lamp3"); // right player torpedo 1 available
	lamplatch0.bit_handler<4>().set_output("lamp2"); // right player ready
	lamplatch0.bit_handler<4>().append_output("lamp1").invert(); // right player reload (active low)
	lamplatch0.bit_handler<5>().set_output("lamp0"); // right player explosion (hit)

	output_latch_device &lamplatch1(OUTPUT_LATCH(config, "lamplatch1")); // 74174 on game board at P2
	lamplatch1.bit_handler<0>().set_output("lamp13"); // left player torpedo 4 available
	lamplatch1.bit_handler<1>().set_output("lamp12"); // left player torpedo 3 available
	lamplatch1.bit_handler<2>().set_output("lamp11"); // left player torpedo 2 available
	lamplatch1.bit_handler<3>().set_output("lamp10"); // left player torpedo 1 available
	lamplatch1.bit_handler<4>().set_output("lamp9"); // left player ready
	lamplatch1.bit_handler<4>().append_output("lamp8").invert(); // left player reload (active low)
	lamplatch1.bit_handler<5>().set_output("lamp7"); // left player explosion (hit)

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	SAMPLES(config, m_samples);
	m_samples->set_channels(10); /* 5*2 channels */
	m_samples->set_samples_names(seawolf_sample_names);
	m_samples->add_route(0, "speaker", 0.25, 0);
	m_samples->add_route(1, "speaker", 0.25, 0);
	m_samples->add_route(2, "speaker", 0.25, 0);
	m_samples->add_route(3, "speaker", 0.25, 0);
	m_samples->add_route(4, "speaker", 0.25, 0);
	m_samples->add_route(5, "speaker", 0.25, 1);
	m_samples->add_route(6, "speaker", 0.25, 1);
	m_samples->add_route(7, "speaker", 0.25, 1);
	m_samples->add_route(8, "speaker", 0.25, 1);
	m_samples->add_route(9, "speaker", 0.25, 1);
}

void ebases_state::ebases(machine_config &config)
{
	astrocade_base(config);
	astrocade_mono_sound(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &ebases_state::ebases_map);
	m_maincpu->set_addrmap(AS_IO, &ebases_state::port_map_ebases);

	m_astrocade_sound[0]->so_cb<1>().set("watchdog", FUNC(watchdog_timer_device::reset_w));

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 128); // MC14024 on CPU board at U18, CLK = VERTDR, Q7 used for RESET
}

void astrocde_state::spacezap(machine_config &config)
{
	astrocade_base(config);
	astrocade_mono_sound(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &astrocde_state::spacezap_map);
	m_maincpu->set_addrmap(AS_IO, &astrocde_state::port_map_mono_pattern);

	m_astrocade_sound[0]->so_cb<0>().set("watchdog", FUNC(watchdog_timer_device::reset_w));
	m_astrocade_sound[0]->so_cb<3>().set("outlatch", FUNC(output_latch_device::write));

	output_latch_device &outlatch(OUTPUT_LATCH(config, "outlatch", 0)); // MC14174B on game board at U16
	outlatch.bit_handler<0>().set(FUNC(astrocde_state::coin_counter_w<0>));
	outlatch.bit_handler<1>().set(FUNC(astrocde_state::coin_counter_w<1>));

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 128); // MC14024 on CPU board at U18, CLK = VERTDR, Q7 used for RESET
}

void astrocde_state::wow(machine_config &config)
{
	astrocade_base(config);
	astrocade_stereo_sound(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &astrocde_state::wow_map);
	m_maincpu->set_addrmap(AS_IO, &astrocde_state::port_map_stereo_pattern);

	cd4099_device &outlatch(CD4099(config, "outlatch"));
	outlatch.q_out_cb<0>().set(FUNC(astrocde_state::coin_counter_w<0>));
	outlatch.q_out_cb<1>().set(FUNC(astrocde_state::coin_counter_w<1>));
	outlatch.q_out_cb<2>().set(FUNC(astrocde_state::sparkle_w<0>));
	outlatch.q_out_cb<3>().set(FUNC(astrocde_state::sparkle_w<1>));
	outlatch.q_out_cb<4>().set(FUNC(astrocde_state::sparkle_w<2>));
	outlatch.q_out_cb<5>().set(FUNC(astrocde_state::sparkle_w<3>));
	outlatch.q_out_cb<7>().set(FUNC(astrocde_state::coin_counter_w<2>));

	/* video hardware */
	m_screen->set_default_position(1.0, 0.0, 1.0, 0.0); // adjusted to match screenshots
//  m_screen->set_default_position(1.066, -0.004, 1.048, -0.026); // adjusted to match flyer

	/* sound hardware */
	SPEAKER(config, "center").front_center();

	m_astrocade_sound[0]->so_cb<5>().set("outlatch", FUNC(cd4099_device::write_nibble_d0));
	m_astrocade_sound[0]->so_cb<7>().set(FUNC(astrocde_state::votrax_speech_w));

	VOTRAX_SC01(config, m_votrax, 756000);

	m_votrax->add_route(0, "f1", 0.65);
	FILTER_RC(config, "f1").set_lowpass(110e3, 560e-12).add_route(0, "f2", 1.00);
	FILTER_RC(config, "f2").set_lowpass(110e3, 560e-12).add_route(0, "f3", 1.00);
	FILTER_RC(config, "f3").set_lowpass(110e3, 560e-12).add_route(0, "f4", 1.00);
	FILTER_RC(config, "f4").set_lowpass(110e3, 560e-12).add_route(0, "center", 1.00);
}

void astrocde_state::gorf(machine_config &config)
{
	astrocade_base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &astrocde_state::wow_map);
	m_maincpu->set_addrmap(AS_IO, &astrocde_state::port_map_stereo_pattern);

	cd4099_device &outlatch(CD4099(config, "outlatch")); // MC14099B on game board at U6
	outlatch.q_out_cb<0>().set(FUNC(astrocde_state::coin_counter_w<0>));
	outlatch.q_out_cb<1>().set(FUNC(astrocde_state::coin_counter_w<1>));
	outlatch.q_out_cb<2>().set(FUNC(astrocde_state::sparkle_w<0>));
	outlatch.q_out_cb<3>().set(FUNC(astrocde_state::sparkle_w<1>));
	outlatch.q_out_cb<4>().set(FUNC(astrocde_state::sparkle_w<2>));
	outlatch.q_out_cb<5>().set(FUNC(astrocde_state::sparkle_w<3>));
	outlatch.q_out_cb<6>().set(FUNC(astrocde_state::gorf_sound_switch_w));
	outlatch.q_out_cb<7>().set_output("lamp6");

	cd4099_device &lamplatch(CD4099(config, "lamplatch")); // MC14099B on game board at U7
	lamplatch.q_out_cb<0>().set_output("lamp0");
	lamplatch.q_out_cb<1>().set_output("lamp1");
	lamplatch.q_out_cb<2>().set_output("lamp2");
	lamplatch.q_out_cb<3>().set_output("lamp3");
	lamplatch.q_out_cb<4>().set_output("lamp4");
	lamplatch.q_out_cb<5>().set_output("lamp5");
	lamplatch.q_out_cb<6>().set_nop(); // n/c
	lamplatch.q_out_cb<7>().set_output("lamp7");

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 128); // MC14024 on CPU board at U18, CLK = VERTDR, Q7 used for RESET

	/* video hardware */
	m_screen->set_default_position(1.0, 0.0, 1.0, 0.0); // adjusted to match flyer

	/* sound hardware */
	SPEAKER(config, "upper", 0.0, 0.0, 1.0);
	SPEAKER(config, "lower", 0.0, -0.5, 1.0);

	ASTROCADE_IO(config, m_astrocade_sound[0], ASTROCADE_CLOCK/4);
	m_astrocade_sound[0]->si_cb().set(FUNC(astrocde_state::input_mux_r));
	m_astrocade_sound[0]->so_cb<0>().set("watchdog", FUNC(watchdog_timer_device::reset_w));
	m_astrocade_sound[0]->so_cb<5>().set("outlatch", FUNC(cd4099_device::write_nibble_d0));
	m_astrocade_sound[0]->so_cb<6>().set("lamplatch", FUNC(cd4099_device::write_nibble_d0));
	m_astrocade_sound[0]->so_cb<7>().set(FUNC(astrocde_state::votrax_speech_w));
	m_astrocade_sound[0]->add_route(ALL_OUTPUTS, "upper", 0.35);

	ASTROCADE_IO(config, m_astrocade_sound[1], ASTROCADE_CLOCK/4).add_route(ALL_OUTPUTS, "lower", 0.35);

	VOTRAX_SC01(config, m_votrax, 756000);
	m_votrax->add_route(ALL_OUTPUTS, "upper", 0.45);
}

void astrocde_state::robby(machine_config &config)
{
	astrocade_base(config);
	astrocade_stereo_sound(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &astrocde_state::robby_map);
	m_maincpu->set_addrmap(AS_IO, &astrocde_state::port_map_stereo_pattern);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + battery

	cd4099_device &outlatch(CD4099(config, "outlatch")); // on game board at U1
	outlatch.q_out_cb<0>().set(FUNC(astrocde_state::coin_counter_w<0>));
	outlatch.q_out_cb<1>().set(FUNC(astrocde_state::coin_counter_w<1>));
	outlatch.q_out_cb<2>().set(FUNC(astrocde_state::coin_counter_w<2>));
	outlatch.q_out_cb<6>().set_output("led0");
	outlatch.q_out_cb<7>().set_output("led1");

	m_astrocade_sound[0]->so_cb<5>().set("outlatch", FUNC(cd4099_device::write_nibble_d0));
}

void astrocde_state::profpac(machine_config &config)
{
	astrocade_16color_base(config);
	astrocade_stereo_sound(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &astrocde_state::profpac_map);
	m_maincpu->set_addrmap(AS_IO, &astrocde_state::port_map_16col_pattern);

	m_bank4000->set_map(&astrocde_state::profpac_bank4000_map);
	m_bank4000->set_addr_width(20);

	output_latch_device &outlatch(OUTPUT_LATCH(config, "outlatch", 0)); // 74LS174 on game board at U6
	outlatch.bit_handler<0>().set(FUNC(astrocde_state::coin_counter_w<0>));
	outlatch.bit_handler<1>().set(FUNC(astrocde_state::coin_counter_w<1>));
	outlatch.bit_handler<2>().set_output("led0");
	outlatch.bit_handler<3>().set_output("led1");

	output_latch_device &lamplatch(OUTPUT_LATCH(config, "lamplatch", 0)); // 74LS174 on game board at U7
	lamplatch.bit_handler<0>().set_output("lamp0"); // left lamp A
	lamplatch.bit_handler<1>().set_output("lamp1"); // left lamp B
	lamplatch.bit_handler<2>().set_output("lamp2"); // left lamp C
	lamplatch.bit_handler<4>().set_output("lamp3"); // right lamp A
	lamplatch.bit_handler<5>().set_output("lamp4"); // right lamp B
	lamplatch.bit_handler<6>().set_output("lamp5"); // right lamp C

	m_astrocade_sound[0]->so_cb<4>().set("outlatch", FUNC(output_latch_device::write));
	m_astrocade_sound[0]->so_cb<5>().set("lamplatch", FUNC(output_latch_device::write));
}

void demndrgn_state::demndrgn(machine_config &config)
{
	astrocade_16color_base(config);
	astrocade_mono_sound(config); // used only for I/O

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &demndrgn_state::demndrgn_map);
	m_maincpu->set_addrmap(AS_IO, &demndrgn_state::port_map_16col_pattern_demndrgn);

	output_latch_device &outlatch(OUTPUT_LATCH(config, "outlatch", 0));
	outlatch.bit_handler<0>().set(FUNC(astrocde_state::coin_counter_w<0>));
	outlatch.bit_handler<1>().set(FUNC(astrocde_state::coin_counter_w<1>));
	outlatch.bit_handler<2>().set_output("led0");
	outlatch.bit_handler<3>().set_output("led1");
	outlatch.bit_handler<4>().set(FUNC(demndrgn_state::input_select_w));

	m_astrocade_sound[0]->so_cb<4>().set("outlatch", FUNC(output_latch_device::write));
	m_astrocade_sound[0]->so_cb<4>().append(FUNC(demndrgn_state::trackball_reset_w));
	m_astrocade_sound[0]->pot_cb<0>().set_ioport("FIREX");
	m_astrocade_sound[0]->pot_cb<1>().set_ioport("FIREY");
}

void tenpindx_state::tenpindx(machine_config &config)
{
	astrocade_16color_base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &tenpindx_state::profpac_map);
	m_maincpu->set_addrmap(AS_IO, &tenpindx_state::port_map_16col_pattern_tenpindx);

	Z80(config, m_subcpu, ASTROCADE_CLOCK/4); // real clock unknown
	m_subcpu->set_daisy_config(tenpin_daisy_chain);
	m_subcpu->set_addrmap(AS_PROGRAM, &tenpindx_state::sub_map);
	m_subcpu->set_addrmap(AS_IO, &tenpindx_state::sub_io_map);

	z80ctc_device& ctc(Z80CTC(config, "ctc", ASTROCADE_CLOCK/4 /* same as "sub" */));
	ctc.intr_callback().set_inputline(m_subcpu, INPUT_LINE_IRQ0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_subcpu, INPUT_LINE_NMI);

	ay8912_device &aysnd(AY8912(config, "aysnd", ASTROCADE_CLOCK/4)); // real clock unknown
	aysnd.port_a_read_callback().set_ioport("DIPSW");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.33);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( seawolf2 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "sw2x1.bin",    0x0000, 0x0800, CRC(ad0103f6) SHA1(c6e411444a824ce54b0eee10f7dc15e4229ec070) )
	ROM_LOAD( "sw2x2.bin",    0x0800, 0x0800, CRC(e0430f0a) SHA1(63d8c6b77e0aa536b4f5bb774bc9285f736d4265) )
	ROM_LOAD( "sw2x3.bin",    0x1000, 0x0800, CRC(05ad1619) SHA1(c9dbeaa4540dc95f98970f501a420b18b9898c91) )
	ROM_LOAD( "sw2x4.bin",    0x1800, 0x0800, CRC(1a1a14a2) SHA1(57d0ddea9f8bf082f50d0468a726fd91aaabf4e4) )
ROM_END


ROM_START( spacezap )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "0662.01",      0x0000, 0x1000, CRC(a92de312) SHA1(784ac67c75c7c101f97ebfd39b2b3f7bf7fa470a) )
	ROM_LOAD( "0663.xx",      0x1000, 0x1000, CRC(4836ebf1) SHA1(ad0e8c34a209c827c1336f0250cc61fee667fb03) )
	ROM_LOAD( "0664.xx",      0x2000, 0x1000, CRC(d8193a80) SHA1(72151e773562da62acd2c1d9638711711cbc13a3) )
	ROM_LOAD( "0665.xx",      0x3000, 0x1000, CRC(3784228d) SHA1(5aabd720a106158a892368c4920d9cd0f5235e34) )
ROM_END


ROM_START( ebases )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "m761a",        0x0000, 0x1000, CRC(34422147) SHA1(6483ca1359b675b0dd739605db2a1dbd4b7fb8cb) )
	ROM_LOAD( "m761b",        0x1000, 0x1000, CRC(4f28dfd6) SHA1(52e571e671fa61b0f9ab397a5947094c24f6c388) )
	ROM_LOAD( "m761c",        0x2000, 0x1000, CRC(bff6c97e) SHA1(e41fb9db919039c8a48b4caebf80821a066d7ccf) )
	ROM_LOAD( "m761d",        0x3000, 0x1000, CRC(5173781a) SHA1(e60c3f4b075f8b811ff6a8637c4aa0b089847a82) )
ROM_END


ROM_START( wow )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wow.x1",       0x0000, 0x1000, CRC(c1295786) SHA1(1e4f30cc15537aed6603b4e664e6e60f4bccb5c5) )
	ROM_LOAD( "wow.x2",       0x1000, 0x1000, CRC(9be93215) SHA1(0bc8ee6d8391104eb217b612f32856b105946682) )
	ROM_LOAD( "wow.x3",       0x2000, 0x1000, CRC(75e5a22e) SHA1(50a8ca11909ce49412c47de4da69e39a083ce5af) )
	ROM_LOAD( "wow.x4",       0x3000, 0x1000, CRC(ef28eb84) SHA1(d6318b3649fccafc2d0a05e5530e88819d299356) )
	ROM_LOAD( "wow.x5",       0x8000, 0x1000, CRC(16912c2b) SHA1(faf9c96d99bc111c5f1618f6863f22fd9269027b) )
	ROM_LOAD( "wow.x6",       0x9000, 0x1000, CRC(35797f82) SHA1(376bba29e88c16d95438fa996913b76581df0937) )
	ROM_LOAD( "wow.x7",       0xa000, 0x1000, CRC(ce404305) SHA1(a52c6c7b77842f25c79515460be6b7ed959b5edb) )
	//ROM_LOAD( "wow.x11",      0xc000, 0x1000 NO_DUMP ) // here would go the foreign language ROM
ROM_END


ROM_START( wowg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wow.x1",       0x0000, 0x1000, CRC(c1295786) SHA1(1e4f30cc15537aed6603b4e664e6e60f4bccb5c5) )
	ROM_LOAD( "wow.x2",       0x1000, 0x1000, CRC(9be93215) SHA1(0bc8ee6d8391104eb217b612f32856b105946682) )
	ROM_LOAD( "wow.x3",       0x2000, 0x1000, CRC(75e5a22e) SHA1(50a8ca11909ce49412c47de4da69e39a083ce5af) )
	ROM_LOAD( "wow.x4",       0x3000, 0x1000, CRC(ef28eb84) SHA1(d6318b3649fccafc2d0a05e5530e88819d299356) )
	ROM_LOAD( "wow.x5",       0x8000, 0x1000, CRC(16912c2b) SHA1(faf9c96d99bc111c5f1618f6863f22fd9269027b) )
	//ROM_LOAD( "x6.bin",     0x9000, 0x1000, CRC(74fccdf8) SHA1(539d074241e98048ab8340c9df3dd59dd1a2b623) ) // This was different too, but is bad (ROM test fails and 0x980-0x9ff has bit 0x04 stuck)
	ROM_LOAD( "wow.x6",       0x9000, 0x1000, CRC(35797f82) SHA1(376bba29e88c16d95438fa996913b76581df0937) )
	ROM_LOAD( "wow.x7",       0xa000, 0x1000, CRC(ce404305) SHA1(a52c6c7b77842f25c79515460be6b7ed959b5edb) )
	ROM_LOAD( "german.x11",   0xc000, 0x1000, CRC(16f84d73) SHA1(f426cfdedcd70b157d81b0031df5a65cacea5fb6) )
ROM_END


ROM_START( gorf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gorf-a.bin",   0x0000, 0x1000, CRC(5b348321) SHA1(76e2e3ad1a66755f1a369167fdb157690fd44a52) )
	ROM_LOAD( "gorf-b.bin",   0x1000, 0x1000, CRC(62d6de77) SHA1(2601faf12d0ab4972c5535ffd722b03ecd8c097c) )
	ROM_LOAD( "gorf-c.bin",   0x2000, 0x1000, CRC(1d3bc9c9) SHA1(0b363a71d7585a4828e08668ebb2999c55e02721) )
	ROM_LOAD( "gorf-d.bin",   0x3000, 0x1000, CRC(70046e56) SHA1(392214cc6ed4155bfe022d36f0f86c2594a5ab57) )
	ROM_LOAD( "gorf-e.bin",   0x8000, 0x1000, CRC(2d456eb5) SHA1(720fb8b48e20c1fc281d8804259016c3c5364a07) )
	ROM_LOAD( "gorf-f.bin",   0x9000, 0x1000, CRC(f7e4e155) SHA1(9c9d6d3bfee6556dc7a01de81d6148dd02f04fc9) )
	ROM_LOAD( "gorf-g.bin",   0xa000, 0x1000, CRC(4e2bd9b9) SHA1(9edccceea5af015275582553ed238c40c73d8f4f) )
	ROM_LOAD( "gorf-h.bin",   0xb000, 0x1000, CRC(fe7b863d) SHA1(5aa8d824814ee1c30eaf0044da78d3aa8220dcaa) )
ROM_END

ROM_START( gorfpgm1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "873a.x1",      0x0000, 0x1000, CRC(97cb4a6a) SHA1(efdae9a437c665fb861665a38c6cb13fd848ad91) )
	ROM_LOAD( "873b.x2",      0x1000, 0x1000, CRC(257236f8) SHA1(d1e8555fe5e6705ef88535bcd6071d1072b01386) )
	ROM_LOAD( "873c.x3",      0x2000, 0x1000, CRC(16b0638b) SHA1(65e1e2e4df80140976915e0982ce3219b14beece) )
	ROM_LOAD( "873d.x4",      0x3000, 0x1000, CRC(b5e821dc) SHA1(152840e353d567cbf5a86206dde70e5b64b27236) )
	ROM_LOAD( "873e.x5",      0x8000, 0x1000, CRC(8e82804b) SHA1(24250edb30efa63c80514629c86c9372b7ca3020) )
	ROM_LOAD( "873f.x6",      0x9000, 0x1000, CRC(715fb4d9) SHA1(c9f33162093e6ed7e3cb6bb716419e5bc43c0381) )
	ROM_LOAD( "873g.x7",      0xa000, 0x1000, CRC(8a066456) SHA1(f64bcdadbc62566b55573039b03baf5358e24a36) )
	ROM_LOAD( "873h.x8",      0xb000, 0x1000, CRC(56d40c7c) SHA1(c7c9a618d9438a76121972ac029ad7036bcf8c6f) )
ROM_END

ROM_START( gorfpgm1f )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gorf_a.x1", 0x0000, 0x1000, CRC(97cb4a6a) SHA1(efdae9a437c665fb861665a38c6cb13fd848ad91) )
	ROM_LOAD( "gorf_b.x2", 0x1000, 0x1000, CRC(257236f8) SHA1(d1e8555fe5e6705ef88535bcd6071d1072b01386) )
	ROM_LOAD( "gorf_c.x3", 0x2000, 0x1000, CRC(16b0638b) SHA1(65e1e2e4df80140976915e0982ce3219b14beece) )
	ROM_LOAD( "gorf_d.x4", 0x3000, 0x1000, CRC(b5e821dc) SHA1(152840e353d567cbf5a86206dde70e5b64b27236) )
	ROM_LOAD( "gorf_e.x5", 0x8000, 0x1000, CRC(8e82804b) SHA1(24250edb30efa63c80514629c86c9372b7ca3020) )
	ROM_LOAD( "gorf_f.x6", 0x9000, 0x1000, CRC(715fb4d9) SHA1(c9f33162093e6ed7e3cb6bb716419e5bc43c0381) )
	ROM_LOAD( "gorf_g.x7", 0xa000, 0x1000, CRC(8a066456) SHA1(f64bcdadbc62566b55573039b03baf5358e24a36) )
	ROM_LOAD( "gorf_h.x8", 0xb000, 0x1000, CRC(56d40c7c) SHA1(c7c9a618d9438a76121972ac029ad7036bcf8c6f) )
	ROM_LOAD( "french_gorf.x11", 0xc000, 0x1000, CRC(759d7f66) SHA1(339b719fc1ffe3c2be49fbd5cf562d06134abadc) )
ROM_END

ROM_START( gorfpgm1g )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "873a.x1",      0x0000, 0x1000, CRC(97cb4a6a) SHA1(efdae9a437c665fb861665a38c6cb13fd848ad91) )
	ROM_LOAD( "873b.x2",      0x1000, 0x1000, CRC(257236f8) SHA1(d1e8555fe5e6705ef88535bcd6071d1072b01386) )
	ROM_LOAD( "873c.x3",      0x2000, 0x1000, CRC(16b0638b) SHA1(65e1e2e4df80140976915e0982ce3219b14beece) )
	ROM_LOAD( "873d.x4",      0x3000, 0x1000, CRC(b5e821dc) SHA1(152840e353d567cbf5a86206dde70e5b64b27236) )
	ROM_LOAD( "873e.x5",      0x8000, 0x1000, CRC(8e82804b) SHA1(24250edb30efa63c80514629c86c9372b7ca3020) )
	ROM_LOAD( "873f.x6",      0x9000, 0x1000, CRC(715fb4d9) SHA1(c9f33162093e6ed7e3cb6bb716419e5bc43c0381) )
	ROM_LOAD( "873g.x7",      0xa000, 0x1000, CRC(8a066456) SHA1(f64bcdadbc62566b55573039b03baf5358e24a36) )
	ROM_LOAD( "873h.x8",      0xb000, 0x1000, CRC(56d40c7c) SHA1(c7c9a618d9438a76121972ac029ad7036bcf8c6f) )
	ROM_LOAD( "german.x11",   0xc000, 0x1000, CRC(3a3dbdcb) SHA1(e20895d41d66d1a23cc445e4ae4628b16ebf83f2) )
ROM_END

/* PCB engraved as "MIDWAY MFC  A IRECSA".
   ROM test fails in test mode, it seems Irecsa didn't bother to change the checksums. */
ROM_START( gorfirec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a.bin", 0x0000, 0x1000, CRC(97cb4a6a) SHA1(efdae9a437c665fb861665a38c6cb13fd848ad91) )
	ROM_LOAD( "b.bin", 0x1000, 0x1000, CRC(e35701c0) SHA1(30794aa97e1ec1c949f74e20a830224e1b5ae8b8) )
	ROM_LOAD( "c.bin", 0x2000, 0x1000, CRC(16b0638b) SHA1(65e1e2e4df80140976915e0982ce3219b14beece) )
	ROM_LOAD( "d.bin", 0x3000, 0x1000, CRC(0dccdfc4) SHA1(f55217e98169df41b8906f880ac78a2d0d56f68a) )
	ROM_LOAD( "e.bin", 0x8000, 0x1000, CRC(8e82804b) SHA1(24250edb30efa63c80514629c86c9372b7ca3020) )
	ROM_LOAD( "f.bin", 0x9000, 0x1000, CRC(715fb4d9) SHA1(c9f33162093e6ed7e3cb6bb716419e5bc43c0381) )
	ROM_LOAD( "g.bin", 0xa000, 0x1000, CRC(fb5b7131) SHA1(865ec4f912418f08ff08910ddd61ac7ebd7b30d5) )
	ROM_LOAD( "h.bin", 0xb000, 0x1000, CRC(578ade88) SHA1(78a27e08aeef7acb648ab9db62a3e87720792aff) )
ROM_END


ROM_START( robby )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rotox1.bin",   0x0000, 0x1000, CRC(a431b85a) SHA1(3478da56addba1cdd98cbef7a15b17fca9aed2cd) )
	ROM_LOAD( "rotox2.bin",   0x1000, 0x1000, CRC(33cdda83) SHA1(ccbc741a2fc0b7385ca42afe5b377432249b44cb) )
	ROM_LOAD( "rotox3.bin",   0x2000, 0x1000, CRC(dbf97491) SHA1(11574baf04af02b38ae147be8409de7c34e87611) )
	ROM_LOAD( "rotox4.bin",   0x3000, 0x1000, CRC(a3b90ac8) SHA1(8c585d26011c9ea047895a0388835ff2bb80e1ff) )
	ROM_LOAD( "rotox5.bin",   0x8000, 0x1000, CRC(46ae8a94) SHA1(218edcc5257c9cc58c5e667fff64767b313daaab) )
	ROM_LOAD( "rotox6.bin",   0x9000, 0x1000, CRC(7916b730) SHA1(c5166625a404da4a93a1a7ae21d01fdb6e78680e) )
	ROM_LOAD( "rotox7.bin",   0xa000, 0x1000, CRC(276dc4a5) SHA1(d740b30c28f6a94ee2348291e80d57af5c2e2d99) )
	ROM_LOAD( "rotox8.bin",   0xb000, 0x1000, CRC(1ef13457) SHA1(4dc1ee9ce2a28c4ba75e630fbfe4659cd68d3a66) )
	ROM_LOAD( "rotox9.bin",   0xc000, 0x1000, CRC(370352bf) SHA1(72cd35b4306b46de3d2a3e4e46fa4917ed9d18cb) )
	ROM_LOAD( "rotox10.bin",  0xd000, 0x1000, CRC(e762cbda) SHA1(48c274a859963097a90f80c48366250301eddb5f) )
ROM_END


ROM_START( profpac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pps1",         0x0000, 0x2000, CRC(a244a62d) SHA1(f7a9606ce6d66c3e6d210cc25572904aeab2b6c8) )
	ROM_LOAD( "pps2",         0x2000, 0x2000, CRC(8a9a6653) SHA1(b730b24088dcfddbe954670ff9212b7383c923f6) )
	ROM_LOAD( "pps9",         0xc000, 0x2000, CRC(17a0b418) SHA1(8b7ed84090dbc5181deef6f55ec755c05d4c0d5e) )

	ROM_REGION( 0x20000, "banks", ROMREGION_ERASEFF )
	ROM_LOAD( "pps3",         0x04000, 0x2000, CRC(15717fd8) SHA1(ffbb156f417d20478117b39de28a15680993b528) )
	ROM_LOAD( "pps4",         0x06000, 0x2000, CRC(36540598) SHA1(33c797c690801afded45091d822347e1ecc72b54) )
	ROM_LOAD( "pps5",         0x08000, 0x2000, CRC(8dc89a59) SHA1(fb4d3ba40697425d69ee19bfdcf00aea1df5fa80) )
	ROM_LOAD( "pps6",         0x0a000, 0x2000, CRC(5a2186c3) SHA1(f706cef6518b7d839377aa8a7c75fdeed4985c57) )
	ROM_LOAD( "pps7",         0x0c000, 0x2000, CRC(f9c26aba) SHA1(201b930cca9669114ffc97978cade69587e34a0f) )
	ROM_LOAD( "pps8",         0x0e000, 0x2000, CRC(4d201e41) SHA1(786b30cd7a7db55bdde05909d7a1a7f122b6e546) )

	ROM_REGION( 0xa0000, "epromboard", ROMREGION_ERASEFF )
	ROM_LOAD( "ppq1",         0x00000, 0x4000, CRC(dddc2ccc) SHA1(d81caaa639f63d971a0d3199b9da6359211edf3d) )
	ROM_LOAD( "ppq2",         0x04000, 0x4000, CRC(33bbcabe) SHA1(f9455868c70f479ede0e0621f21f69da165d9b7a) )
	ROM_LOAD( "ppq3",         0x08000, 0x4000, CRC(3534d895) SHA1(24fb14c6b31b7f27e0737605cfbf963d29dd3fc5) )
	ROM_LOAD( "ppq4",         0x0c000, 0x4000, CRC(17e3581d) SHA1(92d2391e4c8aef46cc8e92b8cf9a8ec9a1b5ff68) )
	ROM_LOAD( "ppq5",         0x10000, 0x4000, CRC(80882a93) SHA1(d5d6afaadb022b109c14c3911eceb0769204df6c) )
	ROM_LOAD( "ppq6",         0x14000, 0x4000, CRC(e5ddaee5) SHA1(45b4925709da6790676319268398f6cfcf12794b) )
	ROM_LOAD( "ppq7",         0x18000, 0x4000, CRC(c029cd34) SHA1(f2f09fdb13920012a6a43958b640d7a06c0c8e69) )
	ROM_LOAD( "ppq8",         0x1c000, 0x4000, CRC(fb3a1ac9) SHA1(e8fe02c85e90320680a14ad560204d5c235730ad) )
	ROM_LOAD( "ppq9",         0x20000, 0x4000, CRC(5e944488) SHA1(2f03f799c319309b5ebf9a5299891d1824398ba5) )
	ROM_LOAD( "ppq10",        0x24000, 0x4000, CRC(ed72a81f) SHA1(db991b93001d2da16b398ee8e9b01b8f0dfe5740) )
	ROM_LOAD( "ppq11",        0x28000, 0x4000, CRC(98295020) SHA1(7f68a8b89117b7ab8724869401a861fe7cff28d9) )
	ROM_LOAD( "ppq12",        0x2c000, 0x4000, CRC(e01a8dbe) SHA1(c7052bf9ce9d2006dda5ddc07ad164d0119b86ea) )
	ROM_LOAD( "ppq13",        0x30000, 0x4000, CRC(87165d4f) SHA1(d47655300c8747698a46f30deb65fe762073e869) )
	ROM_LOAD( "ppq14",        0x34000, 0x4000, CRC(ecb861de) SHA1(73d28a79b76795d3016dd608f9ab3d255f40e477) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "pls153a_cpu.u12",  0x00000, 0x00eb, CRC(499a6fc5) SHA1(633d647bcae2f762847a2abe8069741ac33b15b8) )
	ROM_LOAD( "pls153a_cpu.u16",  0x00100, 0x00eb, CRC(9a5ea540) SHA1(8619c7626e58eac09a4d91f5ad49742240f5f71e) )
	ROM_LOAD( "pls153a_epr.u6",   0x00200, 0x00eb, CRC(d8454bf7) SHA1(5e074a2cbac99ebbf02bc4cd331679ede30eea3f) )
	ROM_LOAD( "pls153a_epr.u7",   0x00300, 0x00eb, CRC(fa831d9f) SHA1(ca8c3d8db24e99537c682aaf9726cbcef86728dd) )
	ROM_LOAD( "pls153a_gam.u10",  0x00400, 0x00eb, CRC(fe2157b0) SHA1(577e6839190054f9b3aec6425e9d2a1810e11a08) )
	ROM_LOAD( "pls153a_gam.u11",  0x00500, 0x00eb, CRC(5772f6d8) SHA1(01a02aa67a42ff61e38e12683b02bf81c16519b8) )
	ROM_LOAD( "pls153a_gam.u5",   0x00600, 0x00eb, CRC(b3f2c6b8) SHA1(e49fb4ca7d9c8a769c145fd497b1244d6696831f) )
	ROM_LOAD( "pls153a_scr.u19",  0x00700, 0x00eb, CRC(b5fff2db) SHA1(beae4fc5664d15a4b83a885d97d21efd14977380) )
	ROM_LOAD( "pls153a_scr.u39",  0x00800, 0x00eb, CRC(ba7ef5dd) SHA1(7aea6e17edbf87dc1d47ca8c640b50ebdb65dd29) )
	ROM_LOAD( "pls153a_scr.u55",  0x00900, 0x00eb, CRC(c3f47134) SHA1(78ae2cc1d8b761b077e36343d4a91517298ce9e8) )
ROM_END


ROM_START( demndrgn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dd-x1.bin",     0x0000, 0x2000, CRC(9aeaf79e) SHA1(c70aa1a31bc085cca904d497c34e55d49fef49b7) )
	ROM_LOAD( "dd-x2.bin",     0x2000, 0x2000, CRC(0c63b624) SHA1(3eaeb4e0820e9dda7233a13bb146acc44402addd) )
	ROM_LOAD( "dd-x9.bin",     0xc000, 0x2000, CRC(3792d632) SHA1(da053df344f39a8f25a2c57fb1a908131c10f248) )

	ROM_REGION( 0x20000, "banks", ROMREGION_ERASEFF )
	ROM_LOAD( "dd-x5.bin",     0x08000, 0x2000, CRC(e377e831) SHA1(f53e74b3138611f9385845d6bdeab891b5d15931) )
	ROM_LOAD( "dd-x6.bin",     0x0a000, 0x2000, CRC(0fcb46ad) SHA1(5611135f9e341bd394d6da7912167b05fff17a93) )
	ROM_LOAD( "dd-x7.bin",     0x0c000, 0x2000, CRC(0675e4fa) SHA1(59668e32271ff9bac0b4411cc0c541d2825ee145) )
	ROM_LOAD( "dd-x10.bin",    0x10000, 0x2000, CRC(4a22c4f9) SHA1(d86ca38727fcf1896ea64c3b6255e3230054b5d6) )
	ROM_LOAD( "dd-x11.bin",    0x12000, 0x2000, CRC(d3158845) SHA1(510bb8d459625263370ee68f6f63d6d7abc6d26d) )
	ROM_LOAD( "dd-x12.bin",    0x14000, 0x2000, CRC(592c1d9a) SHA1(c884aabf4cff9f9b974e497fc3a1f8cdd0753680) )
	ROM_LOAD( "dd-x13.bin",    0x16000, 0x2000, CRC(492d7b7e) SHA1(a9a89a61179b154a8afa429e11e984609f787d74) )
	ROM_LOAD( "dd-x14.bin",    0x18000, 0x2000, CRC(7843c818) SHA1(4756bf7dd07071b86645908d61891edcdffdde83) )
	ROM_LOAD( "dd-x15.bin",    0x1a000, 0x2000, CRC(6e6bc1b6) SHA1(b8c5ed8df6a709a6502dac47be88271ad22b9203) )
	ROM_LOAD( "dd-x16.bin",    0x1c000, 0x2000, CRC(7a4a343b) SHA1(4eb82ae38ce1b14778fb29d8549c61a46bc3ee66) )
ROM_END


ROM_START( tenpindx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tpd_x1.bin",     0x0000, 0x2000, CRC(ef424484) SHA1(d70f8491059e24775ee05140c8c3b0afa6c1107c) )
	ROM_LOAD( "tpd_x2.bin",     0x2000, 0x2000, CRC(a0f53af2) SHA1(966a98f73bdd358601f105ca97c823575b33bab3) )
	ROM_LOAD( "tpd_x9.bin",     0xc000, 0x2000, CRC(ce9a9bd4) SHA1(fcc48a66b6079c56992afef2ca8ab95c66f8f431) )

	ROM_REGION( 0x4000, "sub", 0 )
	ROM_LOAD( "tpd_axfd.bin",   0x0000, 0x4000, CRC(0aed11f3) SHA1(09575cceda38178a77c6753074be82825d368334) )

	ROM_REGION( 0x20000, "banks", ROMREGION_ERASEFF )
	ROM_LOAD( "tpd_x3.bin",     0x04000, 0x2000, CRC(d4645f6d) SHA1(185bcd58f1ba69e26274475c57219de0353267e1) )
	ROM_LOAD( "tpd_x4.bin",     0x06000, 0x2000, CRC(acf474ba) SHA1(b324dccac0991660f8ba2a70cbbdb06c9d25c361) )
	ROM_LOAD( "tpd_x5.bin",     0x08000, 0x2000, CRC(e206913f) SHA1(bb9476516bca7bf7066df058db36e4fdd52a6ed2) )
	ROM_LOAD( "tpd_x6.bin",     0x0a000, 0x2000, CRC(d90142fb) SHA1(01ac2ba60105a8dc84359a8feafd2fea2a635369) )
	ROM_LOAD( "tpd_x8.bin",     0x0e000, 0x2000, CRC(ae22cf50) SHA1(bc6aa59e41cffc840e5cffcd352d8c12c053982d) )
	ROM_LOAD( "tpd_x10.bin",    0x10000, 0x2000, CRC(85d5b970) SHA1(24206cb2829674910e7648ccd30b9fddda9bec95) )
	ROM_LOAD( "tpd_x11.bin",    0x12000, 0x2000, CRC(7bd3c90f) SHA1(d6c2f547c83972668e123203906b3e7fb52c1696) )
	ROM_LOAD( "tpd_x12.bin",    0x14000, 0x2000, CRC(46078cc7) SHA1(50ac83f73dcca74c736961001c0808b379a949f4) )
	ROM_LOAD( "tpd_x13.bin",    0x16000, 0x2000, CRC(b49767b4) SHA1(71aba2967afe95f0d78f539aae9e7bf9a1315aa0) )
	ROM_LOAD( "tpd_x14.bin",    0x18000, 0x2000, CRC(29a28d40) SHA1(c4d6b4b1881e674337e47f801291e94d73a0fcbd) )
	ROM_LOAD( "tpd_x15.bin",    0x1a000, 0x2000, CRC(2ae98fb2) SHA1(a8b4058189bf23fec281da3ff73393346bfead6f) )
	ROM_LOAD( "tpd_x16.bin",    0x1c000, 0x2000, CRC(8839d0e1) SHA1(5f1e581066d1851ee996f152ebec83db40aa7073) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "pls153a_cpu.u12",  0x00000, 0x00eb, CRC(499a6fc5) SHA1(633d647bcae2f762847a2abe8069741ac33b15b8) )
	ROM_LOAD( "pls153a_cpu.u16",  0x00100, 0x00eb, CRC(9a5ea540) SHA1(8619c7626e58eac09a4d91f5ad49742240f5f71e) )
	ROM_LOAD( "pls153a_epr.u6",   0x00200, 0x00eb, CRC(d8454bf7) SHA1(5e074a2cbac99ebbf02bc4cd331679ede30eea3f) )
	ROM_LOAD( "pls153a_epr.u7",   0x00300, 0x00eb, CRC(fa831d9f) SHA1(ca8c3d8db24e99537c682aaf9726cbcef86728dd) )
	ROM_LOAD( "pls153a_gam.u10",  0x00400, 0x00eb, CRC(fe2157b0) SHA1(577e6839190054f9b3aec6425e9d2a1810e11a08) )
	ROM_LOAD( "pls153a_gam.u11",  0x00500, 0x00eb, CRC(5772f6d8) SHA1(01a02aa67a42ff61e38e12683b02bf81c16519b8) )
	ROM_LOAD( "pls153a_gam.u5",   0x00600, 0x00eb, CRC(b3f2c6b8) SHA1(e49fb4ca7d9c8a769c145fd497b1244d6696831f) )
	ROM_LOAD( "pls153a_scr.u19",  0x00700, 0x00eb, CRC(b5fff2db) SHA1(beae4fc5664d15a4b83a885d97d21efd14977380) )
	ROM_LOAD( "pls153a_scr.u39",  0x00800, 0x00eb, CRC(ba7ef5dd) SHA1(7aea6e17edbf87dc1d47ca8c640b50ebdb65dd29) )
	ROM_LOAD( "pls153a_scr.u55",  0x00900, 0x00eb, CRC(c3f47134) SHA1(78ae2cc1d8b761b077e36343d4a91517298ce9e8) )
ROM_END



/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

void astrocde_state::init_seawolf2()
{
	m_video_config = 0x00;
}


void astrocde_state::init_ebases()
{
	m_video_config = AC_SOUND_PRESENT | AC_MONITOR_BW;
}


void astrocde_state::init_spacezap()
{
	m_video_config = AC_SOUND_PRESENT | AC_MONITOR_BW;
}


void astrocde_state::init_wow()
{
	m_video_config = AC_SOUND_PRESENT | AC_LIGHTPEN_INTS | AC_STARS;
}


void astrocde_state::init_gorf()
{
	m_video_config = AC_SOUND_PRESENT | AC_LIGHTPEN_INTS | AC_STARS;
}


void astrocde_state::init_robby()
{
	m_video_config = AC_SOUND_PRESENT;
}


void astrocde_state::init_profpac()
{
	m_video_config = AC_SOUND_PRESENT;

	/* configure banking */
	m_bank8000->configure_entries(0, 4, memregion("banks")->base() + 0x4000, 0x8000);
	m_bank8000->set_entry(0);
}


void astrocde_state::init_demndrgn()
{
	m_video_config = 0x00;

	/* configure banking */
	m_bank8000->configure_entries(0, 4, memregion("banks")->base() + 0x4000, 0x8000);
	m_bank8000->set_entry(0);
}


void astrocde_state::init_tenpindx()
{
	m_video_config = 0x00;

	/* configure banking */
	m_bank8000->configure_entries(0, 4, memregion("banks")->base() + 0x4000, 0x8000);
	m_bank8000->set_entry(0);
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

// 90002 CPU board + 90700 game board + 91312 "characterization card"
GAMEL( 1978, seawolf2,  0,    seawolf2, seawolf2,  seawolf2_state, init_seawolf2, ROT0,   "Dave Nutting Associates / Midway", "Sea Wolf II", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_seawolf2 )

// 91354 CPU board + 90700 game board + 91356 RAM board
GAMEL( 1980, ebases,    0,    ebases,   ebases,    ebases_state,   init_ebases,   ROT0,   "Dave Nutting Associates / Midway", "Extra Bases", MACHINE_SUPPORTS_SAVE, layout_spacezap )

// 91354 CPU board + 90706 game board + 91356 RAM board + 91355 pattern board
GAMEL( 1980, spacezap,  0,    spacezap, spacezap,  astrocde_state, init_spacezap, ROT0,   "Midway", "Space Zap (Midway)", MACHINE_SUPPORTS_SAVE, layout_spacezap ) // there's also an older version by Game-A-Tron on different hardware

// 91354 CPU board + 90708 game board + 91356 RAM board + 91355 pattern board + 91397 memory board
GAME(  1980, wow,       0,    wow,      wow,       astrocde_state, init_wow,      ROT0,   "Dave Nutting Associates / Midway", "Wizard of Wor",                            MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME(  1980, wowg,      wow,  wow,      wowg,      astrocde_state, init_wow,      ROT0,   "Dave Nutting Associates / Midway", "Wizard of Wor (with German language ROM)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

// 91354 CPU board + 90708 game board + 91356 RAM board + 91355 pattern board + 91364 ROM/RAM board
GAMEL( 1981, gorf,      0,    gorf,     gorf,      astrocde_state, init_gorf,     ROT270, "Dave Nutting Associates / Midway",                  "Gorf",                                       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_gorf )
GAMEL( 1981, gorfpgm1,  gorf, gorf,     gorf,      astrocde_state, init_gorf,     ROT270, "Dave Nutting Associates / Midway",                  "Gorf (program 1)",                           MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_gorf )
GAMEL( 1981, gorfpgm1f, gorf, gorf,     gorfpgm1f, astrocde_state, init_gorf,     ROT270, "Dave Nutting Associates / Midway",                  "Gorf (program 1, with French language ROM)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_gorf )
GAMEL( 1981, gorfpgm1g, gorf, gorf,     gorfpgm1g, astrocde_state, init_gorf,     ROT270, "Dave Nutting Associates / Midway",                  "Gorf (program 1, with German language ROM)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_gorf )
GAMEL( 1981, gorfirec,  gorf, gorf,     gorfirec,  astrocde_state, init_gorf,     ROT270, "Dave Nutting Associates / Midway (Irecsa license)", "Gorf (Spain, Irecsa license)",               MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_gorf )


// 91354 CPU board + 90708 game board + 91356 RAM board + 91355 pattern board + 91423 memory board
GAME(  1981, robby,     0,    robby,    robby,     astrocde_state, init_robby,    ROT0,   "Dave Nutting Associates / Bally Midway", "The Adventures of Robby Roto!", MACHINE_SUPPORTS_SAVE )

// 91465 CPU board + 91469 game board + 91466 RAM board + 91488 pattern board + 91467 memory board + 91846 EPROM board
GAME(  1983, profpac,   0,    profpac,  profpac,   astrocde_state, init_profpac,  ROT0,   "Dave Nutting Associates / Bally Midway", "Professor Pac-Man", MACHINE_SUPPORTS_SAVE )

// 91465 CPU board + 91699 game board + 91466 RAM board + 91488 pattern board + 91467 memory board
GAME(  1982, demndrgn,  0,    demndrgn, demndrgn,  demndrgn_state, init_demndrgn, ROT0,   "Dave Nutting Associates / Bally Midway", "Demons & Dragons (prototype)", MACHINE_IS_INCOMPLETE | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAMEL( 1983, tenpindx,  0,    tenpindx, tenpindx,  tenpindx_state, init_tenpindx, ROT0,   "Dave Nutting Associates / Bally Midway", "Ten Pin Deluxe",               MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL, layout_tenpindx )
