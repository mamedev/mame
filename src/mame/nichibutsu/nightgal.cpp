// license: BSD-3-Clause
// copyright-holders: Angelo Salese, David Haywood
// thanks-to: Charles MacDonald
/*******************************************************************************************

Night Gal (c) 1984 Nichibutsu

a.k.a. same Jangou blitter but with NCS CPU for displaying graphics as protection.

TODO:
 - Fix Sweet Gal/Sexy Gal/Sexy Gal Tropical layer clearances (more protection?);
 - Sexy Gal uses an additional NCS for a sample player, understand how to make it play anything (tries to read port $00 but it's always zero);
 - Sweet Gal hangs after winning a round;
 - Other games in the driver also have sample ROMs, it is unknown how they are supposed to playback tho;
 - Understand why Night Gal Summer title screen gets wiped out and why it doesn't get displayed at all during attract mode (protection issue?);
 - NMI origin for Sweet Gal / Night Gal Summer;
 - unemulated WAIT pin for Z80, MCU asserts it when accessing communication RAM;

 Notes:
 - Night Gal Summer player hand is at $f801 onward

 *******************************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/ymopn.h"
#include "video/jangou_blitter.h"
#include "video/resnet.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define MASTER_CLOCK    XTAL(19'968'000)

class nightgal_state : public driver_device
{
public:
	nightgal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_comms_ram(*this, "comms_ram"),
		m_sound_ram(*this, "sound_ram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_io_cr_clear(*this, "CR_CLEAR"),
		m_io_coins(*this, "COINS"),
		m_io_pl1(*this, "PL1_%u", 1U),
		m_io_pl2(*this, "PL2_%u", 1U),
		m_io_system(*this, "SYSTEM"),
		m_io_dswa(*this, "DSWA"),
		m_io_dswb(*this, "DSWB"),
		m_io_dswc(*this, "DSWC"),
		m_palette(*this, "palette"),
		m_blitter(*this, "blitter")
	{ }

	void ngalsumr(machine_config &config);
	void sexygal(machine_config &config);
	void sweetgal(machine_config &config);
	void sgaltrop(machine_config &config);
	void royalqn(machine_config &config);

	void init_ngalsumr();
	void init_royalqn();

private:
	emu_timer *m_z80_wait_ack_timer;

	required_shared_ptr<uint8_t> m_comms_ram;
	optional_shared_ptr<uint8_t> m_sound_ram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_audiocpu;

	/* memory */
	//void sexygal_nsc_true_blitter_w(uint8_t data);
	void royalqn_blitter_0_w(uint8_t data);
	void royalqn_blitter_1_w(uint8_t data);
	void royalqn_blitter_2_w(uint8_t data);
	uint8_t royalqn_nsc_blit_r(offs_t offset);
	uint8_t royalqn_comm_r(offs_t offset);
	void royalqn_comm_w(offs_t offset, uint8_t data);
	void mux_w(uint8_t data);
	uint8_t input_1p_r();
	uint8_t input_2p_r();
	void output_w(uint8_t data);
	uint8_t sexygal_soundram_r(offs_t offset);
	uint8_t sexygal_unknown_sound_r();
	void sexygal_audioff_w(uint8_t data);
	void sexygal_audionmi_w(uint8_t data);

	void ngalsumr_prot_latch_w(uint8_t data);
	uint8_t ngalsumr_prot_value_r();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void nightgal_palette(palette_device &palette) const;
	uint32_t screen_update_nightgal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void common_nsc_map(address_map &map) ATTR_COLD;
	void common_sexygal_io(address_map &map) ATTR_COLD;
	void royalqn_io(address_map &map) ATTR_COLD;
	void royalqn_map(address_map &map) ATTR_COLD;
	void royalqn_nsc_map(address_map &map) ATTR_COLD;
	void sexygal_audio_map(address_map &map) ATTR_COLD;
	void sexygal_io(address_map &map) ATTR_COLD;
	void sexygal_map(address_map &map) ATTR_COLD;
	void sexygal_nsc_map(address_map &map) ATTR_COLD;
	void sgaltrop_io(address_map &map) ATTR_COLD;
	void sgaltrop_nsc_map(address_map &map) ATTR_COLD;
	void sweetgal_map(address_map &map) ATTR_COLD;

	required_ioport m_io_cr_clear;
	required_ioport m_io_coins;
	required_ioport_array<6> m_io_pl1;
	required_ioport_array<6> m_io_pl2;
	required_ioport m_io_system;
	required_ioport m_io_dswa;
	required_ioport m_io_dswb;
	required_ioport m_io_dswc;
	required_device<palette_device> m_palette;
	required_device<jangou_blitter_device> m_blitter;
	void z80_wait_assert_cb();
	TIMER_CALLBACK_MEMBER( z80_wait_ack_cb );

	std::unique_ptr<bitmap_ind16> m_tmp_bitmap;

	/* video-related */
	uint8_t m_blit_raw_data[3];

	/* misc */
	uint8_t m_nsc_latch;
	uint8_t m_z80_latch;
	uint8_t m_mux_data;
	uint8_t m_pal_bank;

	uint8_t m_sexygal_audioff;
};

void nightgal_state::video_start()
{
	m_tmp_bitmap = std::make_unique<bitmap_ind16>(256, 256);
}

uint32_t nightgal_state::screen_update_nightgal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		const uint8_t *src = &m_blitter->blit_buffer(y, cliprect.min_x);
		uint16_t *dst = &m_tmp_bitmap->pix(y, cliprect.min_x);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			uint32_t srcpix = *src++;
			*dst++ = m_palette->pen((srcpix & 0xf) | m_pal_bank);
			*dst++ = m_palette->pen(((srcpix >> 4) & 0xf) | m_pal_bank);
		}
	}

	copybitmap(bitmap, *m_tmp_bitmap, flip_screen(), flip_screen(),0,0, cliprect);
	return 0;
}

// guess: use the same resistor values as Crazy Climber (needs checking on the real HW)
void nightgal_state::nightgal_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double weights_rg[3], weights_b[2];
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 0, 0,
			2, resistances_b,  weights_b,  0, 0,
			0, nullptr, nullptr, 0, 0);

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(weights_rg, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(weights_rg, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

/********************************************
*
* Z80-MCU communications
*
********************************************/

/*
   There are three unidirectional latches that also sends an irq from z80 to MCU.
 */
// TODO: simplify this (error in the document)
void nightgal_state::royalqn_blitter_0_w(uint8_t data)
{
	m_blit_raw_data[0] = data;
}

void nightgal_state::royalqn_blitter_1_w(uint8_t data)
{
	m_blit_raw_data[1] = data;
}

void nightgal_state::royalqn_blitter_2_w(uint8_t data)
{
	m_blit_raw_data[2] = data;
	m_subcpu->set_input_line(0, ASSERT_LINE );
}

uint8_t nightgal_state::royalqn_nsc_blit_r(offs_t offset)
{
	if(offset == 2)
		m_subcpu->set_input_line(0, CLEAR_LINE );

	return m_blit_raw_data[offset];
}

TIMER_CALLBACK_MEMBER(nightgal_state::z80_wait_ack_cb)
{
	m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
}

void nightgal_state::z80_wait_assert_cb()
{
	m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);

	// Note: cycles_to_attotime requires z80 context to work, calling for example m_subcpu as context gives a x4 cycle boost in z80 terms (reads execute_cycles_to_clocks() from NCS?) even if they runs at same speed basically.
	// TODO: needs a getter that tells a given CPU how many cycles requires an executing opcode for the r/w operation, which stacks with wait state penalty for accessing this specific area.
	m_z80_wait_ack_timer->adjust(m_maincpu->cycles_to_attotime(4));
}

uint8_t nightgal_state::royalqn_comm_r(offs_t offset)
{
	z80_wait_assert_cb();
	machine().scheduler().synchronize(); // force resync

	return (m_comms_ram[offset] & 0x80) | (0x7f); //bits 6-0 are undefined, presumably open bus
}

void nightgal_state::royalqn_comm_w(offs_t offset, uint8_t data)
{
	z80_wait_assert_cb();
	machine().scheduler().synchronize(); // force resync
	m_comms_ram[offset] = data & 0x80;
}


/********************************************
*
* Input Multiplexer handling
*
********************************************/

void nightgal_state::mux_w(uint8_t data)
{
	m_mux_data = data;
}

uint8_t nightgal_state::input_1p_r()
{
	uint8_t data = 0xff;

	// mahjong inputs depending on mux
	for (unsigned i = 0; i < 6; i++)
		if (BIT(m_mux_data, i) == 0)
			data &= m_io_pl1[i]->read();

	// credit clear buttons are always read
	data &= m_io_cr_clear->read();

	return data;
}

uint8_t nightgal_state::input_2p_r()
{
	uint8_t data = 0xff;

	// mahjong inputs depending on mux
	for (unsigned i = 0; i < 6; i++)
		if (BIT(m_mux_data, i) == 0)
			data &= m_io_pl2[i]->read();

	// coin inputs are always read
	data &= m_io_coins->read();

	return data;
}

void nightgal_state::output_w(uint8_t data)
{
	/*
	Doesn't match Charles notes?
	--x- ---- unknown, set by Royal Queen on gameplay
	---- x--- color bank, used by Sexy Gal Tropical
	---- -x-- flip screen
	---- --x- out counter
	*/
	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	flip_screen_set((data & 0x04) == 0);
	m_pal_bank = (data & 0x08) << 1;
}

/********************************************
*
* Memory Maps
*
********************************************/


/********************************
* Common
********************************/

void nightgal_state::common_nsc_map(address_map &map)
{
	map(0x0080, 0x0080).portr("BLIT_PORT");
	map(0x0081, 0x0083).r(FUNC(nightgal_state::royalqn_nsc_blit_r));
	map(0x00a0, 0x00af).w(m_blitter, FUNC(jangou_blitter_device::vregs_w));
	map(0x00b0, 0x00b0).w(m_blitter, FUNC(jangou_blitter_device::bltflip_w));
}

/********************************
* Sexy Gal
********************************/

uint8_t nightgal_state::sexygal_soundram_r(offs_t offset)
{
	// bit 7 set = contention?
	return BIT(m_sound_ram[offset], 7) ? 0x00 : 0x40;
}

uint8_t nightgal_state::sexygal_unknown_sound_r()
{
	// value read here stored in (1-bit) shared RAM but not used?
	return 0;
}

// flip flop from main to audio CPU
void nightgal_state::sexygal_audioff_w(uint8_t data)
{
	// causes an irq
	m_audiocpu->set_input_line(0, BIT(data, 6) ? ASSERT_LINE : CLEAR_LINE);

	// bit 4 used, audio cpu reset line?
	// bit 5 used only for access to shared RAM?

	m_sexygal_audioff = data;
}

void nightgal_state::sexygal_audionmi_w(uint8_t data)
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);

	// doesn't seem to be any way around this blatant hack
	m_audiocpu->reset();
}


void nightgal_state::sweetgal_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x807f).lrw8([this](offs_t off) { return m_sound_ram[off]; }, "snd_r", [this](offs_t off, u8 data) { m_sound_ram[off] = data; }, "snd_w");
	map(0xe000, 0xefff).rw(FUNC(nightgal_state::royalqn_comm_r), FUNC(nightgal_state::royalqn_comm_w));
	map(0xf000, 0xffff).ram();
}

void nightgal_state::sexygal_map(address_map &map)
{
	sweetgal_map(map);
	map(0x8000, 0x807f).r(FUNC(nightgal_state::sexygal_soundram_r));
	map(0xa000, 0xa000).w(FUNC(nightgal_state::sexygal_audioff_w));
}

void nightgal_state::common_sexygal_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).portr("DSWA").w(FUNC(nightgal_state::output_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(nightgal_state::mux_w));
	map(0x12, 0x12).mirror(0xe8).portr("DSWB").w(FUNC(nightgal_state::royalqn_blitter_0_w));
	map(0x13, 0x13).mirror(0xe8).portr("DSWC").w(FUNC(nightgal_state::royalqn_blitter_1_w));
	map(0x14, 0x14).mirror(0xe8).nopr().w(FUNC(nightgal_state::royalqn_blitter_2_w));
}

void nightgal_state::sexygal_io(address_map &map)
{
	common_sexygal_io(map);

	map(0x00, 0x01).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}

void nightgal_state::sgaltrop_io(address_map &map)
{
	common_sexygal_io(map);

	// not actually a YM2203?
	map(0x01, 0x01).r("ymsnd", FUNC(ym2203_device::data_r));
	map(0x02, 0x02).w("ymsnd", FUNC(ym2203_device::data_w));
	map(0x03, 0x03).w("ymsnd", FUNC(ym2203_device::address_w));
}

void nightgal_state::sexygal_nsc_map(address_map &map)
{
	common_nsc_map(map);
	map(0x0080, 0x0086).m(m_blitter, FUNC(jangou_blitter_device::blit_v2_regs));
	map(0x1000, 0x13ff).mirror(0x2c00).rw(FUNC(nightgal_state::royalqn_comm_r), FUNC(nightgal_state::royalqn_comm_w)).share("comms_ram");
	map(0xc000, 0xdfff).mirror(0x2000).rom().region("subrom", 0);
}

void nightgal_state::sgaltrop_nsc_map(address_map &map)
{
	common_nsc_map(map);

	map(0x0080, 0x0086).m(m_blitter, FUNC(jangou_blitter_device::blit_v2_regs));
	map(0x1000, 0x13ff).mirror(0x2c00).rw(FUNC(nightgal_state::royalqn_comm_r), FUNC(nightgal_state::royalqn_comm_w)).share("comms_ram");
	map(0xc000, 0xffff).rom().region("subrom", 0);
}



void nightgal_state::sexygal_audio_map(address_map &map)
{
	map(0x0080, 0x0080).r(FUNC(nightgal_state::sexygal_unknown_sound_r));
	map(0x1000, 0x1000).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x2000, 0x207f).ram().share("sound_ram");
	map(0x3000, 0x3000).w(FUNC(nightgal_state::sexygal_audionmi_w));
	map(0x4000, 0xbfff).rom().region("samples", 0);
	map(0xc000, 0xffff).rom().region("audiorom", 0);
}

/********************************
* Royal Queen
********************************/


void nightgal_state::royalqn_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).noprw();
	map(0xc000, 0xdfff).rw(FUNC(nightgal_state::royalqn_comm_r), FUNC(nightgal_state::royalqn_comm_w)).share("comms_ram");
	map(0xe000, 0xffff).ram();
}

void nightgal_state::royalqn_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).mirror(0xec).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).mirror(0xec).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).mirror(0xe8).portr("DSWA").w(FUNC(nightgal_state::output_w));
	map(0x11, 0x11).mirror(0xe8).portr("SYSTEM").w(FUNC(nightgal_state::mux_w));
	map(0x12, 0x12).mirror(0xe8).portr("DSWB").w(FUNC(nightgal_state::royalqn_blitter_0_w));
	map(0x13, 0x13).mirror(0xe8).portr("DSWC").w(FUNC(nightgal_state::royalqn_blitter_1_w));
	map(0x14, 0x14).mirror(0xe8).nopr().w(FUNC(nightgal_state::royalqn_blitter_2_w));
	map(0x15, 0x15).mirror(0xe8).noprw();
	map(0x16, 0x16).mirror(0xe8).noprw();
	map(0x17, 0x17).mirror(0xe8).noprw();
}

void nightgal_state::royalqn_nsc_map(address_map &map)
{
	common_nsc_map(map);

	map(0x0080, 0x0086).m(m_blitter, FUNC(jangou_blitter_device::blit_v1_regs));
	map(0x1000, 0x13ff).mirror(0x2c00).rw(FUNC(nightgal_state::royalqn_comm_r), FUNC(nightgal_state::royalqn_comm_w));
	map(0x4000, 0x4000).noprw();
	map(0x8000, 0x8000).noprw(); //open bus or protection check
	map(0xc000, 0xdfff).mirror(0x2000).rom().region("subrom", 0);
}

/********************************************
*
* Input ports
*
********************************************/

static INPUT_PORTS_START( sexygal )
	PORT_START("CR_CLEAR")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_CUSTOM ) // multiplexed mahjong inputs
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear P1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear P2")

	PORT_START("COINS")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_CUSTOM ) // multiplexed mahjong inputs
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) //player-1 side
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) //player-2 side

	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_CODE(KEYCODE_3)//rate button
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //another D button
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("PL1_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED ) //another opt 1 button
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("PL1_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 4")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("PL1_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Pass") //???
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_CODE(KEYCODE_4) PORT_PLAYER(2)//rate button
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //another D button
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("PL2_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED ) //another opt 1 button
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("PL2_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 1") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 2") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 3") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 4") PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("PL2_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Pass") PORT_PLAYER(2) //???
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Option 0 - Payout")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_TOGGLE  PORT_CODE(KEYCODE_9)                         /* Memory Reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_TOGGLE  PORT_CODE(KEYCODE_0) PORT_NAME("Analyzer")       /* Analyzer */
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x10, 0x00, "SYSTEM" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	// TODO: ok for ngtbunny, not for the others
	PORT_START("DSWA")
	PORT_DIPNAME( 0x0f, 0x0f, "Game Out Rate" ) PORT_DIPLOCATION("SWA:1,2,3,4")
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0xf0, 0xf0, "Rate Max" ) PORT_DIPLOCATION("SWA:5,6,7,8")
//  PORT_DIPSETTING(    0x20, "10" )
//  PORT_DIPSETTING(    0x30, "20" )
//  PORT_DIPSETTING(    0x40, "5" )
//  PORT_DIPSETTING(    0x50, "10" )
//  PORT_DIPSETTING(    0x60, "20" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x90, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0xa0, "4" )
	PORT_DIPSETTING(    0xd0, "5" )
	PORT_DIPSETTING(    0xb0, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0xe0, "10" )
	PORT_DIPSETTING(    0xf0, "20" )
	PORT_DIPSETTING(    0x70, "30" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DSWB" )
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

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, "DSWC" )
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

	PORT_START("BLIT_PORT")
	PORT_DIPNAME( 0x01, 0x01, "BLIT_PORT" )
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
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("blitter", jangou_blitter_device, status_r)
INPUT_PORTS_END

void nightgal_state::machine_start()
{
	m_z80_wait_ack_timer = timer_alloc(FUNC(nightgal_state::z80_wait_ack_cb), this);

	save_item(NAME(m_nsc_latch));
	save_item(NAME(m_z80_latch));
	save_item(NAME(m_mux_data));
	save_item(NAME(m_pal_bank));
	save_item(NAME(m_blit_raw_data));
}

void nightgal_state::machine_reset()
{
	m_nsc_latch = 0;
	m_z80_latch = 0;
	m_mux_data = 0;

	std::fill(std::begin(m_blit_raw_data), std::end(m_blit_raw_data), 0);
}

void nightgal_state::royalqn(machine_config &config)
{
	Z80(config, m_maincpu, MASTER_CLOCK / 8);        /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &nightgal_state::royalqn_map);
	m_maincpu->set_addrmap(AS_IO, &nightgal_state::royalqn_io);
	m_maincpu->set_vblank_int("screen", FUNC(nightgal_state::irq0_line_hold));

	NSC8105(config, m_subcpu, MASTER_CLOCK / 8);
	m_subcpu->set_addrmap(AS_PROGRAM, &nightgal_state::royalqn_nsc_map);

	config.set_perfect_quantum(m_maincpu);

	JANGOU_BLITTER(config, m_blitter, MASTER_CLOCK/4);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK/4,320,0,256,264,16,240);
	screen.set_screen_update(FUNC(nightgal_state::screen_update_nightgal));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(nightgal_state::nightgal_palette), 0x20);

	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", MASTER_CLOCK / 8));
	aysnd.port_a_read_callback().set(FUNC(nightgal_state::input_1p_r));
	aysnd.port_b_read_callback().set(FUNC(nightgal_state::input_2p_r));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.40);
}

void nightgal_state::sexygal(machine_config &config)
{
	royalqn(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &nightgal_state::sexygal_map);
	m_maincpu->set_addrmap(AS_IO, &nightgal_state::sexygal_io);

	m_subcpu->set_addrmap(AS_PROGRAM, &nightgal_state::sexygal_nsc_map);
	m_subcpu->set_vblank_int("screen", FUNC(nightgal_state::irq0_line_hold));

	NSC8105(config, m_audiocpu, MASTER_CLOCK / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &nightgal_state::sexygal_audio_map);

	clock_device &sampleclk(CLOCK(config, "sampleclk", 6000)); // quite a wild guess
	sampleclk.signal_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "mono", 0.25); // unknown DAC

	config.device_remove("aysnd");

	ym2203_device &ymsnd(YM2203(config, "ymsnd", MASTER_CLOCK / 8));
	ymsnd.port_a_read_callback().set(FUNC(nightgal_state::input_1p_r));
	ymsnd.port_b_read_callback().set(FUNC(nightgal_state::input_2p_r));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.40);
}

void nightgal_state::sweetgal(machine_config &config)
{
	sexygal(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &nightgal_state::sweetgal_map);

	// doesn't have the extra NSC8105 (so how does this play samples?)
	config.device_remove("audiocpu");
	config.device_remove("sampleclk");
}

void nightgal_state::ngalsumr(machine_config &config)
{
	royalqn(config);
	// TODO: happens from protection device
	m_maincpu->set_periodic_int(FUNC(nightgal_state::nmi_line_pulse), attotime::from_hz(60));
}

void nightgal_state::sgaltrop(machine_config &config)
{
	sexygal(config);
	m_maincpu->set_addrmap(AS_IO, &nightgal_state::sgaltrop_io);

	m_subcpu->set_addrmap(AS_PROGRAM, &nightgal_state::sgaltrop_nsc_map);

	config.device_remove("audiocpu");
	config.device_remove("sampleclk");
}

/*
Night Gal
(c)1984 NihonBussan Co.,Ltd.

OSC:20MHz
CPU:Z80
SND:AY-3-8910
ETC:CUSTOM(The surface of the chip is scratched, so the name of the chip is unknown), MemoryBackup

NGAL_01.BIN graphic
NGAL_02.BIN graphic
NGAL_03.BIN graphic
NGAL_04.BIN graphic
NGAL_05.BIN graphic
NGAL_06.BIN graphic
NGAL_07.BIN graphic
NGAL_08.BIN graphic
NGAL_09.BIN program
NGAL_10.BIN program
NGAL_11.BIN program
NGAL_12.BIN program
NGAL_BP.BIN color

Dumped by Gastroptosis. 2000/06/04
Dumped by Uki. 2000/06/11
?
*/

ROM_START( nightgal )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ngal_10.bin", 0x00000, 0x02000, CRC(5eb28742) SHA1(d48045b7cbce69093494c4ec764cf4fb3c120bd6) )
	ROM_LOAD( "ngal_11.bin", 0x02000, 0x02000, CRC(c52f7942) SHA1(e23b9e4936f9b3111ea14c0250190ee6de1ed4ab) )
	ROM_LOAD( "ngal_12.bin", 0x04000, 0x02000, CRC(515e69a7) SHA1(234247c829c2b082360d7d44c1488fc5fcf45cd2) )

	ROM_REGION( 0x2000, "subrom", 0 )
	ROM_LOAD( "ngal_09.bin", 0x0000, 0x02000, CRC(da3dcc08) SHA1(6f5319c1777dabf7041286698ac8f25eca1545a1) )

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD( "ngal_01.bin",  0x00000, 0x02000, CRC(8e4c92ad) SHA1(13cebe765ebabe6be79c9c9ac3f778550e450380) )
	ROM_LOAD( "ngal_02.bin",  0x02000, 0x02000, CRC(c60f7dc1) SHA1(273fd05c62e1efe26538efd2d4f0973c5eba65e4) )
	ROM_LOAD( "ngal_03.bin",  0x04000, 0x02000, CRC(824b7d9e) SHA1(04d3340cbb954add0d70c093df4ccb669e5ed12b) )
	ROM_LOAD( "ngal_04.bin",  0x06000, 0x02000, CRC(d1981ad6) SHA1(668a7aaa43b4e727a90a4e11cee659509465e546) )
	ROM_LOAD( "ngal_05.bin",  0x08000, 0x02000, CRC(ed5e4a28) SHA1(5d9441a2c79ad3a3d1b2ad7187bba49bdbd0a76e) )
	ROM_LOAD( "ngal_06.bin",  0x0a000, 0x02000, CRC(81de181d) SHA1(b528c0c82dc240dc34fe5b2fcb77b0e5f5701c7c) )
	ROM_LOAD( "ngal_07.bin",  0x0c000, 0x02000, CRC(de0e6f9b) SHA1(ac3428b0ba560e41f46dfb906da2c5e0f034d31c) )
	ROM_LOAD( "ngal_08.bin",  0x0e000, 0x02000, CRC(2c5cc9a0) SHA1(9ba797eb2fc549e9f16dfee442fd7de53a58d4f0) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "ngal_bp.bin", 0x00, 0x20, CRC(19255a7d) SHA1(4ac6316f7d8b575f28d33564b422b68993a4e484) )
ROM_END

/*

Night Bunny
(c)1984 Nichibutsu

CPU: Z80
Sound: AY-3-8910
OSC: 20.000MHz
Other: surface scratched DIP40 (NB1413M3?)

ROMs:
1.3A
2.3C
3.3D
4.3F
5.3M
6.3N
7.3P
8.3S (2764)
MB7051.6S


dumped by sayu
--- Team Japump!!! ---


*/
ROM_START( ngtbunny )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "6.3n", 0x00000, 0x02000, CRC(3e39c0ef) SHA1(56287fb19ff1a61ff606454315e433ecd2e9318a) )
	ROM_LOAD( "7.3p", 0x02000, 0x02000, CRC(34024380) SHA1(ba535e2b198f55e68a45ad7030b12c9aa1389aea) )
	ROM_LOAD( "8.3s", 0x04000, 0x02000, CRC(9bf96168) SHA1(f0e9302bc9577fe779b56cb72035672368c94481) )

	ROM_REGION( 0x2000, "subrom", 0 )
	ROM_LOAD( "5.3m",  0x0000, 0x02000, CRC(b8a82966) SHA1(9f86b3208fb48f9735cfc4f8e62680f0cb4a92f0) )

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD( "1.3a",  0x00000, 0x02000, CRC(16776c5f) SHA1(a2925eaed938ae3985ea796658b62d6fafb6412b) )
	ROM_LOAD( "2.3c",  0x02000, 0x02000, CRC(dffd2cc6) SHA1(34f45b20596f69c44dc01c7aef765ab3ddaa076b) )
	ROM_LOAD( "3.3d",  0x04000, 0x02000, CRC(c532ca49) SHA1(b01b08e99e24649c45ce1833f830775d6f532f6b) )
	ROM_LOAD( "4.3f",  0x06000, 0x02000, CRC(ade1d0d8) SHA1(c3ad6bfeed878132d02770d97f6392daa509de5f) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "mb7051.6s", 0x00, 0x20, CRC(006b42d6) SHA1(ced119a299a9a7694d2fa0ef178b69d76abd0d6f) )
ROM_END

ROM_START( royalngt )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rn6.3n", 0x00000, 0x02000, CRC(abbf38b9) SHA1(455dcec2ac2187b7216ff53fbbb8975b763fb981) )
	ROM_LOAD( "rn7.3p", 0x02000, 0x02000, CRC(ae9c082b) SHA1(ee3effea653f972fd732453e9ab72f48e75410f8) )
	ROM_LOAD( "rn8.3s", 0x04000, 0x02000, CRC(1371a83a) SHA1(c7107b62534837dd51bb4a93ba9a690f91393930) )

	ROM_REGION( 0x2000, "subrom", 0 )
	ROM_LOAD( "rn5.3l",  0x00000, 0x02000, CRC(b8a82966) SHA1(9f86b3208fb48f9735cfc4f8e62680f0cb4a92f0) )

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD( "rn1.3a",  0x00000, 0x02000, CRC(16776c5f) SHA1(a2925eaed938ae3985ea796658b62d6fafb6412b) )
	ROM_LOAD( "rn2.3c",  0x02000, 0x02000, CRC(dffd2cc6) SHA1(34f45b20596f69c44dc01c7aef765ab3ddaa076b) )
	ROM_LOAD( "rn3.3d",  0x04000, 0x02000, CRC(31fb1d47) SHA1(41441bc2613c95dc810cad569cbaa0c023c819ba) )
	ROM_LOAD( "rn4.3e",  0x06000, 0x02000, CRC(ade1d0d8) SHA1(c3ad6bfeed878132d02770d97f6392daa509de5f) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "f5.6s", 0x00, 0x20, CRC(006b42d6) SHA1(ced119a299a9a7694d2fa0ef178b69d76abd0d6f) )
ROM_END

ROM_START( royalqn )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "b10.3s", 0x00000, 0x02000, CRC(67a4abfe) SHA1(1f408f7540185ce136507a8aca8d3beb234979d5) )
	ROM_LOAD( "a11.3t", 0x02000, 0x02000, CRC(e7c5395b) SHA1(5131ab9b0fbf1b7b4d410aa2a57eceaf47f8ec3a) )
	ROM_LOAD( "a12.3v", 0x04000, 0x02000, CRC(4e8efda4) SHA1(1959491fd899a4d85fd067d7674592ec25188a75) )

	ROM_REGION( 0x2000, "subrom", 0 )
	ROM_LOAD( "rq9.3p",  0x0000, 0x02000, CRC(34b4cf82) SHA1(01f49ca11a695d41c181e92217e228bc1656ee57) )

	ROM_REGION( 0xc000, "samples", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD( "rq1.3a",  0x00000, 0x02000, CRC(066449dc) SHA1(34838f5e3569b313306ce465e481b934e938c837) )
	ROM_LOAD( "rq2.3c",  0x02000, 0x02000, CRC(c467adb5) SHA1(755ebde6229bbf0c7d9293e0becb7506d9aa9d49) )
	ROM_LOAD( "rq3.3d",  0x04000, 0x02000, CRC(7e5a7a2d) SHA1(5770cd832de59ff4f61ac40eca8c2238ff7b582d) )
	ROM_LOAD( "rq4.3f",  0x06000, 0x02000, CRC(afb3e333) SHA1(a3ddf800925df748db4f71a9dcb05ff0e838d767) )
	ROM_LOAD( "rq5.3j",  0x08000, 0x02000, CRC(1e81d0f6) SHA1(f38fbaf1f2cfabb5ba0e4a06964f9a2862b7569d) )
	ROM_LOAD( "rq6.3k",  0x0a000, 0x02000, CRC(45b2bb9c) SHA1(935e72d45585576b8f8c140ef2fdedfe6578d1c8) )
	ROM_LOAD( "rq7.3l",  0x0c000, 0x02000, CRC(c43ee2dd) SHA1(235e15d0a5e3ccbdf47960241faf747eaa2524f6) )
	ROM_LOAD( "rq8.3n",  0x0e000, 0x02000, CRC(3a79b3cc) SHA1(0b7b13cd1ee35ec3475d33c734c6d8f757dddd96) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "ng.6s", 0x00, 0x20, CRC(19255a7d) SHA1(4ac6316f7d8b575f28d33564b422b68993a4e484) )
ROM_END

/*

Sexy Gal
(c)1985 Nihon Bussan

XG-1B (main board)
SGP-A (sub board)

CPU:    Z80-A
SOUND:  YM2203C
    DAC
OSC:    20.000MHz
    10.000MHz
    6.000MHz (sub board)
Chips:  CPU? 40pin
    CPU? 40pin (sub board)


1.3A    prg?

2.3C    chr.
3.3D
4.3E
5.3F
6.3H
7.3JK
8.3KL
9.3M

10.3N   Z80 prg.
11.3PR

12.S8B  prg./samples?
13.S7B
14.S6B

SG.7E   color


*/

ROM_START( sexygal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10.3n",  0x00000, 0x04000, CRC(53425b74) SHA1(1239c0527d00d693313366b7e3da669565f99ffd) )
	ROM_LOAD( "11.3pr", 0x04000, 0x04000, CRC(a3138b42) SHA1(1bf7f6e2c4020251379cc72fa731c17795f35e2e) )

	ROM_REGION( 0x4000, "subrom", 0 )
	ROM_LOAD( "1.3a",   0x00000, 0x04000, CRC(f814cf27) SHA1(ceba1f14a202d926380039d7cb4669eb8be58539) ) // has a big (16 byte wide) ASCII 'Y.M' art, written in YMs (!)

	ROM_REGION( 0x4000, "audiorom", 0)
	ROM_LOAD( "14.s6b",  0x00000, 0x04000, CRC(b4a2497b) SHA1(7231f57b4548899c886625e883b9972c0f30e9f2) )

	ROM_REGION( 0x8000, "samples", 0 )
	ROM_LOAD( "12.s8b",  0x00000, 0x04000, CRC(7ac4a984) SHA1(7b41c522387938fe7625c9a6c62a385d6635cc5e) )
	ROM_LOAD( "13.s7b",  0x04000, 0x04000, CRC(5eb75f56) SHA1(b7d81d786d1ac8d65a6a122140954eb89d76e8b4) )

	ROM_REGION( 0x40000, "gfx", ROMREGION_ERASEFF )
	ROM_LOAD( "2.3c",  0x00000, 0x04000, CRC(f719e09d) SHA1(c78411b4f974b3dd261d51e522e086fc30a96fcb) )
	ROM_LOAD( "3.3d",  0x04000, 0x04000, CRC(a84d9a89) SHA1(91d5978e35ba4acf9353a13ec22c22aeb8a35f12) )
	ROM_LOAD( "4.3e",  0x08000, 0x04000, CRC(f1cdbedb) SHA1(caacf2887a3a05e498d57d570a1e9873f95a5d5f) )
	ROM_LOAD( "5.3f",  0x0c000, 0x04000, CRC(76569186) SHA1(79cb32c1f1a96f90d59f331a01ca548936933b87) )
	ROM_LOAD( "6.3h",  0x10000, 0x04000, CRC(8b6268e4) SHA1(c57bb7fe8f079d8f202f370cd7bdce1cf0596ede) )
	ROM_LOAD( "7.3jk", 0x14000, 0x04000, CRC(c88f68b8) SHA1(512019f465c298ba8fbf0f6c285a9b0d6c8f7411) )
	ROM_LOAD( "8.3kl", 0x18000, 0x04000, CRC(4631e092) SHA1(961b10b556defe9e4ba84180149bb2ef4042dbe9) )
	ROM_LOAD( "9.3m",  0x1c000, 0x04000, CRC(198df711) SHA1(adf9531ee7058db2314811aba7568bd332632947) )
	ROM_FILL(          0x20000, 0x20000, 0x11 )
	//ROM_FILL(          0x28000, 0x08000, 0x22 )
	//ROM_FILL(          0x30000, 0x08000, 0x33 )
	//ROM_FILL(          0x38000, 0x08000, 0x44 )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "sg.7e", 0x00, 0x20, CRC(5786a035) SHA1(29d95a6fb076d64ca217206fcadde51993830a88) )
ROM_END

ROM_START( sweetgal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10.3n", 0x00000, 0x04000, CRC(0f6c4bf0) SHA1(50e5c6f08e124641f5df8938ccfcdebde18f6a0f) ) // sldh
	ROM_LOAD( "11.3p", 0x04000, 0x04000, CRC(7388e9b3) SHA1(e318d2d3888679bbd43a0aab68252fd359b7969d) )

	ROM_REGION( 0x2000, "subrom", 0 )
	ROM_LOAD( "1.3a",  0x0000, 0x2000, CRC(5342c757) SHA1(b4ff84c45bd2c6a6a468f1d0daaf5b19c4dbf8fe) ) // sldh

	ROM_REGION( 0xc000, "samples", 0 ) // sound samples
	ROM_LOAD( "v2_12.bin",  0x00000, 0x04000, CRC(66a35be2) SHA1(4f0d73d753387acacc5ccc90e91d848a5ecce55e) )
	ROM_LOAD( "v2_13.bin",  0x04000, 0x04000, CRC(60785a0d) SHA1(71eaec3512c0b18b93c083c1808eec51cfd4f520) )
	ROM_LOAD( "v2_14.bin",  0x08000, 0x04000, CRC(149e84c1) SHA1(5c4e18637bef2f31bc3578cae6525fb6280fbc06) )

	ROM_REGION( 0x40000, "gfx", 0 )
	ROM_LOAD( "2.3c",  0x00000, 0x04000, CRC(3a3d78f7) SHA1(71e35529f30c43ee8ec2363f85fe17042f1d304e) ) // sldh
	ROM_LOAD( "3.3d",  0x04000, 0x04000, CRC(c6f9b884) SHA1(32d6fe1906a3f1f528f30dbd3f89971b2ea1925b) ) // sldh
	// all roms below match sexygal
	ROM_LOAD( "4.3e",  0x08000, 0x04000, CRC(f1cdbedb) SHA1(caacf2887a3a05e498d57d570a1e9873f95a5d5f) )
	ROM_LOAD( "5.3f",  0x0c000, 0x04000, CRC(76569186) SHA1(79cb32c1f1a96f90d59f331a01ca548936933b87) )
	ROM_LOAD( "6.3h",  0x10000, 0x04000, CRC(8b6268e4) SHA1(c57bb7fe8f079d8f202f370cd7bdce1cf0596ede) )
	ROM_LOAD( "7.3jk", 0x14000, 0x04000, CRC(c88f68b8) SHA1(512019f465c298ba8fbf0f6c285a9b0d6c8f7411) )
	ROM_LOAD( "8.3kl", 0x18000, 0x04000, CRC(4631e092) SHA1(961b10b556defe9e4ba84180149bb2ef4042dbe9) )
	ROM_LOAD( "9.3m",  0x1c000, 0x04000, CRC(198df711) SHA1(adf9531ee7058db2314811aba7568bd332632947) )
	ROM_FILL(          0x20000, 0x20000, 0x11 )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "sg.7e", 0x00, 0x20, CRC(5786a035) SHA1(29d95a6fb076d64ca217206fcadde51993830a88) )
ROM_END

/*

Night Gal Summer (JPN Ver.)
(c)1985 Nihon Bussan

CPU:    Z80
SOUND:  AY-3-8910
    DAC
OSC:    20.000MHz
    6.000MHz (sub board)

Chips:  NG138507 (CPU?)
    Unknown 40pin
    Unknown 40pin


1S.IC7  prg./samples?
2S.IC6
3S.IC5

1.3A    chr.
2.3C
3.3D
4.3F
5.3H
6.3L

7.3P    main prg.
8.3S
9.3T
10.3V

NG2.6U  color



*/

ROM_START( ngalsumr )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "8.3s", 0x00000, 0x02000, CRC(30f81b12) SHA1(e264b0cdc6ff400643cba56847344c270e96a204) )
	ROM_LOAD( "9.3t", 0x02000, 0x02000, CRC(879fc493) SHA1(ec7c6928b5d4e46dcc99271466e7eb801f601a70) )
	ROM_LOAD( "10.3v", 0x04000, 0x02000, CRC(31211088) SHA1(960b781c420602be3de66565a030cf5ebdcc2ffb) )

	ROM_REGION( 0x2000, "subrom", 0 )
	ROM_LOAD( "7.3p",  0x0000, 0x02000, CRC(20c55a25) SHA1(9dc88cb6c016b594264f7272d4fd5f30567e7c5d) )

	ROM_REGION( 0xc000, "samples", 0 )
	ROM_LOAD( "1s.ic7", 0x00000, 0x04000, CRC(47ad8a0f) SHA1(e3b1e13f0a5c613bd205338683bef8d005b54830) )
	ROM_LOAD( "2s.ic6", 0x04000, 0x04000, CRC(ca2a735f) SHA1(5980525a67fb0ffbfa04b82d805eee2463236ce3) )
	ROM_LOAD( "3s.ic5", 0x08000, 0x04000, CRC(5cf15267) SHA1(72e4b2aa59a50af6b1b25d5279b3b125bfe06d86) )

	ROM_REGION( 0x40000, "gfx", ROMREGION_ERASEFF )
	ROM_LOAD( "1.3a",  0x00000, 0x04000, CRC(9626f812) SHA1(ca7162811a0ba05dfaa2aa8cc93a2e898b326e9e) )
	ROM_LOAD( "3.3d",  0x04000, 0x04000, CRC(2fb2ec0b) SHA1(2f1735e33906783b8c0b283455a2a079431e6f11) )
	ROM_LOAD( "5.3h",  0x08000, 0x04000, CRC(feaca6a3) SHA1(6658c01ac5769e8317a1c7eec6802e7c96885710) )
	ROM_LOAD( "2.3c",  0x10000, 0x04000, CRC(0d59cf7a) SHA1(600bc70d29853fb936f8adaef048d925cbae0ce9) )
	ROM_RELOAD(        0x20000, 0x04000 )
	ROM_LOAD( "4.3f",  0x14000, 0x04000, CRC(c7b85199) SHA1(1c4ed2faf82f45d8a23c168793b02969f1201df6) )
	ROM_RELOAD(        0x24000, 0x04000 )
	ROM_LOAD( "6.3l",  0x18000, 0x04000, CRC(de9e05f8) SHA1(724468eade222b513b7f39f0a24515f343428130) )
	ROM_RELOAD(        0x28000, 0x04000 )
	ROM_RELOAD(        0x0c000, 0x04000 ) // gameplay elements
	// debug code
	ROM_FILL(          0x1c000, 0x04000, 0x22 )
	ROM_FILL(          0x2c000, 0x04000, 0x33 )
	ROM_FILL(          0x30000, 0x10000, 0x44 )


	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "ng2.6u", 0x00, 0x20, CRC(0162a24a) SHA1(f7e1623c5bca3725f2e59ae2096b9bc42e0363bf) )
ROM_END

/*

Sexy Gal Tropical
(c)1985 Nihon Bussan

XGトロピカル (main board)

Chips/hybrid modules (all Nichibutsu silkscreen):
    2P  GF 136027 (40-pin CPU or PSG?)
    4C  XG 1985-05 (40-pin)
    4P  XGZ 60-04 (40-pin)
    5D  NB 1984-06 (28-pin)

Program:
    3A  27128       1   NSC8105 program
    3B  27128       2
    3C  27128       3
    3E  27256       4
    3F  27256       5
    3H  27256       6
    3J  [empty socket]
    3K  27256       7
    3M  [empty socket]
    3N  27256       8
    3P  [empty socket]
    3R  27128       9   Z80 program
    3S  27128       10  Z80 program
    5A  20-pin PAL  GT  no dump
    7F  82S123      GT

RAM:
    3T  6116LP-3

ROM loading is mostly guessed just to get dumps in

*/

ROM_START(sgaltrop)
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "10.3s", 0x0000, 0x4000, CRC(f252d959) SHA1(a1747d1f0c248ae8d9e304ee017b8195fff9c4a2) )
	ROM_LOAD( "9.3r",  0x4000, 0x4000, CRC(834b62b6) SHA1(43fb7733d734158082126ee4f15c022c8bd53106) )

	ROM_REGION( 0x4000, "subrom", 0 )
	ROM_LOAD( "1.3a",  0x0000, 0x4000, CRC(bf096542) SHA1(8c20e70114b7e3369c15b4d8efacd55a16b2252c) )

	ROM_REGION( 0xc000, "samples", 0 )
	ROM_LOAD( "2.3b",  0x4000, 0x4000, CRC(1723d18d) SHA1(8447c8838941559e5496d2e0834884c27a46375c) )
	ROM_LOAD( "3.3c",  0x8000, 0x4000, CRC(cdb2057b) SHA1(e60b46813e082ede0694f28f0c2c7a7fdf323ac9) )

	ROM_REGION( 0x80000, "gfx", ROMREGION_ERASEFF )
	ROM_LOAD( "4.3e",  0x00000, 0x08000, CRC(e10a3c91) SHA1(f77f85527afd59d57cd9cf1deb68c22e35722c78) )
	ROM_LOAD( "7.3k",  0x10000, 0x08000, CRC(bd76eb88) SHA1(43cc8269a539153601619381c5dd0c50dd8d6a00) )
	ROM_LOAD( "5.3f",  0x20000, 0x08000, CRC(ec482f8e) SHA1(d4d6f618400949141a84ac981ad548ded105bfef) )
	// next two are unconfirmed
	ROM_LOAD( "6.3h",  0x30000, 0x08000, CRC(571e5f93) SHA1(ef9e27a2121a0d63ac9aa5e4168c73c39d06c60a) )
	ROM_RELOAD(        0x08000, 0x08000 ) // attract mode, after a demo match
	ROM_LOAD( "8.3n",  0x40000, 0x08000, CRC(5029a16f) SHA1(a89ac8283b3e487d9be5f1a8a1e37ba0bf0cd654) )
	ROM_RELOAD(        0x18000, 0x08000 ) // gal select
	// debug code, to be removed at some point
	ROM_FILL(          0x28000, 0x08000, 0x33 )
	ROM_FILL(          0x38000, 0x08000, 0x44 )
	ROM_FILL(          0x48000, 0x08000, 0x55 )
	ROM_FILL(          0x50000, 0x30000, 0x66 )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "gt.7f", 0x00, 0x20, CRC(59e36d6e) SHA1(2e0f3d4809ec727518e6ec883f67ede8831681bf) )
ROM_END

void nightgal_state::init_royalqn()
{
	uint8_t *ROM = memregion("subrom")->base();

	/* patch open bus / protection */
	ROM[0x027e] = 0x02;
	ROM[0x027f] = 0x02;
}

// Night Gal Summer uses a protection latch device to get some layer clearances width/height values.
void nightgal_state::ngalsumr_prot_latch_w(uint8_t data)
{
	m_z80_latch = data;
}

uint8_t nightgal_state::ngalsumr_prot_value_r()
{
	switch(m_z80_latch)
	{
		case 0:
			return 0;
		case 1:
			return 0x14;

		case 0x4: // cpu hand height on winning
			return 62;

		case 0x3: // game over msg height
			return 12;
		case 0xf: // game over msg width
			return 255;

		case 0xa: // girl width (title screen)
			return 0x40;
		case 0xb: // girl height (title screen)
			return 0x60;

		case 0xc: // score table blink width
			return 120;
		case 0x2: // score table blink height
			return 8;

		case 0x6: // player hand height on losing
			return 28;
		case 0x7: // player discards height on losing
			return 38;

		case 0xd: // player discards width on losing
			return 142;
		case 0xe: // player hand width on losing
			return 200;
		case 0xff:
			return 0;
	}

	logerror("ngalsumr protection device unemulated value latched = %02x\n",m_z80_latch);

	return 0;
}

void nightgal_state::init_ngalsumr()
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x6000, 0x6000, write8smo_delegate(*this, FUNC(nightgal_state::ngalsumr_prot_latch_w)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x6001, 0x6001, read8smo_delegate(*this, FUNC(nightgal_state::ngalsumr_prot_value_r)));
	// 0x6003 some kind of f/f state
}

} // anonymous namespace


/* Type 1 HW */
GAME( 1984, nightgal, 0,        royalqn,  sexygal, nightgal_state, empty_init,    ROT0, "Nichibutsu",   "Night Gal (Japan 840920 AG 1-00)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // ナイトギャル
GAME( 1984, ngtbunny, 0,        royalqn,  sexygal, nightgal_state, empty_init,    ROT0, "Nichibutsu",   "Night Bunny (Japan 840601 MRN 2-10)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // ナイトバニー
GAME( 1984, royalngt, ngtbunny, royalqn,  sexygal, nightgal_state, empty_init,    ROT0, "Royal Denshi", "Royal Night (Japan 840220 RN 2-00)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // ロイヤルナイト
GAME( 1984, royalqn,  0,        royalqn,  sexygal, nightgal_state, init_royalqn,  ROT0, "Royal Denshi", "Royal Queen (Japan 841010 RQ 0-07)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // ロイヤルクイーン
/* Type 2 HW */
GAME( 1985, sexygal,  0,        sexygal,  sexygal, nightgal_state, empty_init,    ROT0, "Nichibutsu",   "Sexy Gal (Japan 850501 SXG 1-00)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // セクシーギャル
GAME( 1985, sweetgal, sexygal,  sweetgal, sexygal, nightgal_state, empty_init,    ROT0, "Nichibutsu",   "Sweet Gal (Japan 850510 SWG 1-02)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // スイートギャル
/* Type 3 HW */
GAME( 1985, ngalsumr, 0,        ngalsumr, sexygal, nightgal_state, init_ngalsumr, ROT0, "Nichibutsu",   "Night Gal Summer (Japan 850702 NGS 0-01)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) // ナイトギャルサマー
/* Type 4 HW */
GAME( 1985, sgaltrop, 0,        sgaltrop, sexygal, nightgal_state, empty_init,    ROT0, "Nichibutsu",   "Sexy Gal Tropical (Japan 850805 SXG T-02)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
