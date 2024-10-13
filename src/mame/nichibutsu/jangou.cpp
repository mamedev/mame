// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese, Philip Bennett
/*******************************************************************************************

Jangou (c) 1983 Nichibutsu

driver by David Haywood, Angelo Salese and Phil Bennett

TODO:
-unemulated screen flipping;
-jngolady: RNG in this isn't working properly...looking at the code,when the mcu isn't on
 irq routine there's a poll to the [8] ram address,unsurprisingly it's the RNG seed.
 The problem is that when the rti opcode occurs the program flow doesn't return to feed the
 RNG but executes another irq,probably there are too many irq pollings and the mcu goes out
 of cycles...for now I'm kludging it,my guess is that it can be either a cpu bug,a comms bug
 or 8 is really connected to a RNG seed and the starting code is just for initializing it.
-dip-switches;

============================================================================================
Debug cheats:

*jangou
$c132 coin counter
$c088-$c095 player tiles

*******************************************************************************************/

#include "emu.h"

#include "machine/segacrpt_device.h"
#include "cpu/m6800/m6800.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/hc55516.h"
#include "sound/msm5205.h"
#include "video/jangou_blitter.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "video/resnet.h"


namespace {

#define MASTER_CLOCK    XTAL(19'968'000)

class cntrygrl_state : public driver_device
{
public:
	cntrygrl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu_0(*this, "cpu0")
		, m_blitter(*this, "blitter")
		, m_palette(*this, "palette")
		, m_keymatrix(*this, { "PL1_1", "PL1_2", "PL2_1", "PL2_2", "PL1_3", "PL2_3" })
	{
	}

	void cntrygrl(machine_config &config) ATTR_COLD;
	void roylcrdn(machine_config &config) ATTR_COLD;
	void luckygrl(machine_config &config) ATTR_COLD;

protected:
	required_device<cpu_device> m_cpu_0;
	required_device<jangou_blitter_device> m_blitter;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mux_w(uint8_t data);
	void output_w(uint8_t data);

private:
	required_device<palette_device> m_palette;
	required_ioport_array<6> m_keymatrix;

	std::unique_ptr<bitmap_ind16> m_tmp_bitmap;

	/* misc */
	uint8_t     m_mux_data = 0;

	uint8_t input_mux_r();

	void init_palette(palette_device &palette) const ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cntrygrl_cpu0_map(address_map &map) ATTR_COLD;
	void cntrygrl_cpu0_io(address_map &map) ATTR_COLD;
	void roylcrdn_cpu0_map(address_map &map) ATTR_COLD;
	void roylcrdn_cpu0_io(address_map &map) ATTR_COLD;
	void luckygrl_cpu0_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
};

class jangou_state_base : public cntrygrl_state
{
protected:
	jangou_state_base(const machine_config &mconfig, device_type type, const char *tag)
		: cntrygrl_state(mconfig, type, tag)
		, m_cpu_1(*this, "cpu1")
		, m_soundlatch(*this, "soundlatch")
	{
	}

protected:
	required_device<cpu_device> m_cpu_1;
	required_device<generic_latch_8_device> m_soundlatch;

	void jangou_base(machine_config &config) ATTR_COLD;

private:
	void jangou_cpu0_io(address_map &map) ATTR_COLD;
};

class jangou_state : public jangou_state_base
{
public:
	jangou_state(const machine_config &mconfig, device_type type, const char *tag)
		: jangou_state_base(mconfig, type, tag)
		, m_cvsd(*this, "cvsd")
	{
	}

	void jangou(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<hc55516_device> m_cvsd;

	/* sound-related */
	emu_timer   *m_cvsd_bit_timer = nullptr;
	uint8_t     m_cvsd_shiftreg = 0;
	int         m_cvsd_shift_cnt = 0;

	void cvsd_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(cvsd_bit_timer_callback);

	void jangou_cpu0_map(address_map &map) ATTR_COLD;
	void jangou_cpu1_map(address_map &map) ATTR_COLD;
	void jangou_cpu1_io(address_map &map) ATTR_COLD;
};

class jngolady_state : public jangou_state_base
{
public:
	jngolady_state(const machine_config &mconfig, device_type type, const char *tag)
		: jangou_state_base(mconfig, type, tag)
		, m_nsc(*this, "nsc")
		, m_nsclatch(*this, "nsclatch")
		, m_msm(*this, "msm")
	{
	}

	void jngolady(machine_config &config) ATTR_COLD;

	void init_jngolady() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_nsc;
	required_device<generic_latch_8_device> m_nsclatch;
	required_device<msm5205_device> m_msm;

	/* sound-related */
	uint8_t     m_adpcm_byte = 0;
	int         m_msm5205_vclk_toggle = 0;

	/* misc */
	uint8_t     m_z80_latch = 0;

	void adpcm_w(uint8_t data);
	uint8_t master_com_r();
	void slave_com_w(uint8_t data);
	uint8_t rng_r();

	void vclk_cb(int state);

	void jngolady_cpu0_map(address_map &map) ATTR_COLD;
	void jngolady_cpu1_io(address_map &map) ATTR_COLD;
	void jngolady_cpu1_map(address_map &map) ATTR_COLD;
	void nsc_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Video Hardware
 *
 *************************************/

// guess: use the same resistor values as Crazy Climber (needs checking on the real hardware)
void cntrygrl_state::init_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double weights_rg[3], weights_b[2];
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 0, 0,
			2, resistances_b,  weights_b,  0, 0,
			0, nullptr, nullptr, 0, 0);

	for (int i = 0;i < palette.entries(); i++)
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

uint32_t cntrygrl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		const uint8_t *src = &m_blitter->blit_buffer(y, cliprect.min_x);
		uint16_t *dst = &m_tmp_bitmap->pix(y, cliprect.min_x);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			uint32_t const srcpix = *src++;
			*dst++ = m_palette->pen(srcpix & 0xf);
			*dst++ = m_palette->pen((srcpix >> 4) & 0xf);
		}
	}
	//void copybitmap(bitmap_rgb32 &dest, const bitmap_rgb32 &src, int flipx, int flipy, int32_t destx, int32_t desty, const rectangle &cliprect)

	copybitmap(bitmap, *m_tmp_bitmap, flip_screen(), flip_screen(),0,0, cliprect);

	return 0;
}

/*************************************
 *
 *  I/O
 *
 *************************************/

void cntrygrl_state::mux_w(uint8_t data)
{
	m_mux_data = data;
}

void cntrygrl_state::output_w(uint8_t data)
{
	/*
	--x- ---- ? (polls between high and low in irq routine, most likely irq mask)
	---- -x-- flip screen
	---- ---x coin counter
	*/
//  printf("%02x\n", data);
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	flip_screen_set(data & 0x04);
//  machine().bookkeeping().coin_lockout_w(0, ~data & 0x20);
}

uint8_t cntrygrl_state::input_mux_r()
{
	uint8_t result = 0xff;
	for (unsigned i = 0; m_keymatrix.size() > i; ++i)
	{
		if (!BIT(m_mux_data, i))
			result &= m_keymatrix[i]->read();
	}
	return result;
}


/*************************************
 *
 *  Sample Player CPU
 *
 *************************************/

/* Jangou HC-55516 CVSD */
void jangou_state::cvsd_w(uint8_t data)
{
	m_cvsd_shiftreg = data;
}

TIMER_CALLBACK_MEMBER(jangou_state::cvsd_bit_timer_callback)
{
	/* Data is shifted out at the MSB */
	m_cvsd->digit_w(BIT(m_cvsd_shiftreg, 7));
	m_cvsd_shiftreg <<= 1;

	/* Trigger an IRQ for every 8 shifted bits */
	if ((++m_cvsd_shift_cnt & 7) == 0)
		m_cpu_1->set_input_line(0, HOLD_LINE);
}


/* Jangou Lady MSM5218 (MSM5205-compatible) ADPCM */
void jngolady_state::adpcm_w(uint8_t data)
{
	m_adpcm_byte = data;
}

void jngolady_state::vclk_cb(int state)
{
	if (!m_msm5205_vclk_toggle)
	{
		m_msm->data_w(m_adpcm_byte >> 4);
	}
	else
	{
		m_msm->data_w(m_adpcm_byte & 0xf);
		m_cpu_1->set_input_line(0, HOLD_LINE);
	}

	m_msm5205_vclk_toggle ^= 1;
}


/*************************************
 *
 *  Jangou Lady NSC8105 CPU
 *
 *************************************/

uint8_t jngolady_state::master_com_r()
{
	return m_z80_latch;
}

void jngolady_state::slave_com_w(uint8_t data)
{
	m_z80_latch = data;
}

/*************************************
 *
 *  Country Girl Memory Map
 *
 *************************************/

void cntrygrl_state::cntrygrl_cpu0_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
//  map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xefff).ram();
}

void cntrygrl_state::cntrygrl_cpu0_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW"); //dsw + blitter busy flag
	map(0x10, 0x10).w(FUNC(cntrygrl_state::output_w));
	map(0x11, 0x11).w(FUNC(cntrygrl_state::mux_w));
	map(0x12, 0x17).m(m_blitter, FUNC(jangou_blitter_device::blit_v1_regs));
	map(0x20, 0x2f).w(m_blitter, FUNC(jangou_blitter_device::vregs_w));
	map(0x30, 0x30).nopw(); //? polls 0x03 continuously
}

/*************************************
 *
 *  Royal Card Memory Map
 *
 *************************************/

void cntrygrl_state::roylcrdn_cpu0_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0x7000, 0x77ff).ram().share("nvram");   /* MK48Z02B-15 ZEROPOWER RAM */
}

void cntrygrl_state::roylcrdn_cpu0_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW");         /* DSW + blitter busy flag */
	map(0x10, 0x10).nopw();                 /* Writes continuosly 0's in attract mode, and 1's in game */
	map(0x11, 0x11).w(FUNC(cntrygrl_state::mux_w));
	map(0x12, 0x17).m(m_blitter, FUNC(jangou_blitter_device::blit_v1_regs));
	map(0x13, 0x13).nopr();                  /* Often reads bit7 with unknown purposes */
	map(0x20, 0x2f).w(m_blitter, FUNC(jangou_blitter_device::vregs_w));
	map(0x30, 0x30).nopw();                 /* Seems to write 0x10 on each sound event */
}

/*************************************
 *
 *  Lucky Girl Memory Map
 *
 *************************************/

void cntrygrl_state::luckygrl_cpu0_map(address_map &map)
{
	map(0x0000, 0x4fff).rom();
	map(0xc000, 0xc7ff).ram();
}

void cntrygrl_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x4fff).rom().share("decrypted_opcodes");
}

/*************************************
 *
 *  Jangou Memory Map
 *
 *************************************/

void jangou_state::jangou_cpu0_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xc000, 0xc7ff).ram();
}

void jangou_state_base::jangou_cpu0_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).portr("DSW").w(FUNC(jangou_state_base::output_w)); //dsw + blitter busy flag
	map(0x11, 0x11).w(FUNC(jangou_state_base::mux_w));
	map(0x12, 0x17).m(m_blitter, FUNC(jangou_blitter_device::blit_v1_regs));
	map(0x20, 0x2f).w(m_blitter, FUNC(jangou_blitter_device::vregs_w));
	map(0x30, 0x30).nopw(); //? polls 0x03 continuously
	map(0x31, 0x31).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}


void jangou_state::jangou_cpu1_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().nopw();
}

void jangou_state::jangou_cpu1_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x01, 0x01).w(FUNC(jangou_state::cvsd_w));
	map(0x02, 0x02).nopw(); // Echoes sound command - acknowledge?
}


/*************************************
 *
 *  Jangou Lady Memory Map
 *
 *************************************/

void jngolady_state::jngolady_cpu0_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xc000, 0xc7ff).ram().share("share1");
	map(0xe000, 0xe000).r(FUNC(jngolady_state::master_com_r)).w(m_nsclatch, FUNC(generic_latch_8_device::write));
}


void jngolady_state::jngolady_cpu1_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().nopw();
}

void jngolady_state::jngolady_cpu1_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x01, 0x01).w(FUNC(jngolady_state::adpcm_w));
	map(0x02, 0x02).nopw();
}


void jngolady_state::nsc_map(address_map &map)
{
	map(0x8000, 0x8000).nopw(); //write-only, IRQ-related?
	map(0x9000, 0x9000).r(m_nsclatch, FUNC(generic_latch_8_device::read)).w(FUNC(jngolady_state::slave_com_w));
	map(0xc000, 0xc7ff).ram().share("share1");
	map(0xf000, 0xffff).rom();
}


/*************************************
 *
 *  Input Port Definitions
 *
 *************************************/

static INPUT_PORTS_START( jangou )
	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	/*The "unknown" bits for this port might be actually unused*/
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* there's a bank of 6 dip-switches in there*/
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )     PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )     PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )     PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )     PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen") // guess
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("blitter", jangou_blitter_device, status_r)
INPUT_PORTS_END

static INPUT_PORTS_START( macha )
	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1P A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1P B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1P C")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	/*The "unknown" bits for this port might be actually unused*/
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Analyzer")
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	/* there's a bank of 6 dip-switches in there. */
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen") // guess
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("blitter", jangou_blitter_device, status_r)
INPUT_PORTS_END


static INPUT_PORTS_START( cntrygrl )
	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1P Keep 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1P Keep 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1P Keep 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("1P Keep 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("1P Keep 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("1P Play") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("1P Left") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("1P Right") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("1P Flip Flop")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("1P Bonus") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("1P Take") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("2P Keep 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("2P Keep 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("2P Keep 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("2P Keep 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("2P Keep 5")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("2P Play")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(2) PORT_NAME("1P Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_PLAYER(2) PORT_NAME("1P Right")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2) PORT_NAME("2P Flip Flop")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("1P Bonus")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2) PORT_NAME("1P Take")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x04, 0x04, "Memory Reset" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Analyzer" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x40, 0x40, "Credit Clear" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Two Pairs" )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Maximum Rate")  PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "90" )
	PORT_DIPSETTING(    0x01, "83" )
	PORT_DIPSETTING(    0x02, "76" )
	PORT_DIPSETTING(    0x03, "69" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "2")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x0c, "0" )
	PORT_DIPNAME( 0x10, 0x10, "Maximum Bet" )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x20, 0x20, "Coin A setting" )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "1 Coin / 25 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "Coin B setting"  )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin / 10 Credits"  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("blitter", jangou_blitter_device, status_r)
INPUT_PORTS_END

static INPUT_PORTS_START( jngolady )
	PORT_INCLUDE( jangou )

	PORT_MODIFY("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start / P1 Mahjong Flip Flop")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("P1 Ready")

	PORT_MODIFY("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P2 Start / P2 Mahjong Flip Flop")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2) PORT_NAME("P2 Ready")

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no service switch

	/* 6 or 7 dip-switches here? bit 6 seems used as vblank.*/
	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("blitter", jangou_blitter_device, status_r)
INPUT_PORTS_END

static INPUT_PORTS_START( roylcrdn )
	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("1P Bet1")                /* 1P Bet1 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("1P Bet2")                /* 1P Bet2 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("1P Bet3")                /* 1P Bet3 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("1P Bet4")                /* 1P Bet4 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("1P Bet5")                /* 1P Bet5 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("1P Flip-Flop")    /* 1P Flip-Flop */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("1P Start")               /* 1P Start */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("1P Take Score")          /* 1P Take Score */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("1P Hi-Lo (W-Up)")        /* 1P W-Up */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("1P Hi (Big)")            /* 1P Big */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("1P Lo (Small)")          /* 1P Small */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("1P Stand")               /* 1P Stand */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("1P Hit")                 /* 1P Hit */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("2P Bet1")            /* 2P Bet1 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2P Bet2")            /* 2P Bet2 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("2P Bet3")            /* 2P Bet3 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("2P Bet4")            /* 2P Bet4 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("2P Bet5")            /* 2P Bet5 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("2P Flip-Flop")    /* 2P Flip-Flop */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("2P Start")           /* 2P Start */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)  PORT_NAME("2P Take Score")      /* 2P Take Score */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("2P Hi-Lo (W-Up)")    /* 2P W-Up */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)    PORT_NAME("2P Hi (Big)")        /* 2P Big */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)  PORT_NAME("2P Lo (Small)")      /* 2P Small */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("2P Stand")           /* 2P Stand */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("2P Hit")             /* 2P Hit */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )                                                                 /* Spare 2 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )                                      PORT_NAME("Note In")        /* Note In */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_TOGGLE  PORT_CODE(KEYCODE_9)                         /* Memory Reset */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_TOGGLE  PORT_CODE(KEYCODE_0)  PORT_NAME("Analyzer")       /* Analyzer */
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                                                                         /* Test Mode */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                                      PORT_NAME("Coin In")        /* Coin In */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )                              PORT_NAME("Credit Clear")   /* Credit Clear */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )                                                                 /* Spare 1 */

	PORT_START("DSW")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("blitter", jangou_blitter_device, status_r)
INPUT_PORTS_END

static INPUT_PORTS_START( luckygrl )
	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("1P Bet1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("1P Bet2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("1P Bet3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("1P Bet4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("1P Bet5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("1P Bet6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("1P Start")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("1P Flip-Flop")

	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("1P Take Score")          /* 1P Take Score */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("1P Hi-Lo (W-Up)")        /* 1P W-Up */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("1P Hi (Big)")            /* 1P Big */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("1P Lo (Small)")          /* 1P Small */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("1P Stand")               /* 1P Stand */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("1P Hit")                 /* 1P Hit */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("2P Bet1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2P Bet2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("2P Bet3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("2P Bet4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("2P Bet5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("2P Bet6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("2P Start")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("2P Flip-Flop")

	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)  PORT_NAME("2P Take Score")      /* 2P Take Score */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("2P Hi-Lo (W-Up)")    /* 2P W-Up */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)    PORT_NAME("2P Hi (Big)")        /* 2P Big */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)  PORT_NAME("2P Lo (Small)")      /* 2P Small */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("2P Stand")           /* 2P Stand */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("2P Hit")             /* 2P Hit */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )                                                                 /* Spare 2 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )                                      PORT_NAME("Note In")        /* Note In */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_TOGGLE  PORT_CODE(KEYCODE_9)                         /* Memory Reset */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_TOGGLE  PORT_CODE(KEYCODE_0)  PORT_NAME("Analyzer")       /* Analyzer */
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                                                                         /* Test Mode */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )                                      PORT_NAME("Coin In")        /* Coin In */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )                              PORT_NAME("Credit Clear")   /* Credit Clear */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )                                                                 /* Spare 1 */

	PORT_START("DSW") // 6 dips bank
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("blitter", jangou_blitter_device, status_r)
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void cntrygrl_state::machine_start()
{
	save_item(NAME(m_mux_data));

	m_tmp_bitmap = std::make_unique<bitmap_ind16>(256, 256);
}

void jangou_state::machine_start()
{
	jangou_state_base::machine_start();

	save_item(NAME(m_cvsd_shiftreg));
	save_item(NAME(m_cvsd_shift_cnt));

	// Create a timer to feed the CVSD DAC with sample bits
	m_cvsd_bit_timer = timer_alloc(FUNC(jangou_state::cvsd_bit_timer_callback), this);
	m_cvsd_bit_timer->adjust(attotime::from_hz(MASTER_CLOCK / 1024), 0, attotime::from_hz(MASTER_CLOCK / 1024));
}

void jngolady_state::machine_start()
{
	jangou_state_base::machine_start();

	save_item(NAME(m_adpcm_byte));
	save_item(NAME(m_msm5205_vclk_toggle));
	save_item(NAME(m_z80_latch));
}

void cntrygrl_state::machine_reset()
{
	m_mux_data = 0xff;
}

void jangou_state::machine_reset()
{
	jangou_state_base::machine_reset();

	m_cvsd_shiftreg = 0;
	m_cvsd_shift_cnt = 0;
}

void jngolady_state::machine_reset()
{
	jangou_state_base::machine_reset();

	m_adpcm_byte = 0;
	m_msm5205_vclk_toggle = 0;
	m_z80_latch = 0;
}

/* Note: All frequencies and dividers are unverified */
void cntrygrl_state::cntrygrl(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_cpu_0, MASTER_CLOCK / 8);
	m_cpu_0->set_addrmap(AS_PROGRAM, &cntrygrl_state::cntrygrl_cpu0_map);
	m_cpu_0->set_addrmap(AS_IO, &cntrygrl_state::cntrygrl_cpu0_io);
	m_cpu_0->set_vblank_int("screen", FUNC(jangou_state::irq0_line_hold));

	JANGOU_BLITTER(config, "blitter", MASTER_CLOCK/4);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK/4,320,0,256,264,16,240); // assume same as nightgal.cpp
	screen.set_screen_update(FUNC(cntrygrl_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(cntrygrl_state::init_palette), 32);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", MASTER_CLOCK / 16));
	aysnd.port_a_read_callback().set(FUNC(jangou_state::input_mux_r));
	aysnd.port_b_read_callback().set_ioport("SYSTEM");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.40);
}

void cntrygrl_state::roylcrdn(machine_config &config)
{
	cntrygrl(config);

	/* basic machine hardware */
	m_cpu_0->set_addrmap(AS_PROGRAM, &cntrygrl_state::roylcrdn_cpu0_map);
	m_cpu_0->set_addrmap(AS_IO, &cntrygrl_state::roylcrdn_cpu0_io);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void cntrygrl_state::luckygrl(machine_config &config)
{
	cntrygrl(config);

	sega_315_5096_device &maincpu(SEGA_315_5096(config.replace(), m_cpu_0, MASTER_CLOCK / 8)); // actually Falcon 03155096 encrypted Z80
	maincpu.set_addrmap(AS_PROGRAM, &cntrygrl_state::luckygrl_cpu0_map);
	maincpu.set_addrmap(AS_IO, &cntrygrl_state::cntrygrl_cpu0_io);
	maincpu.set_addrmap(AS_OPCODES, &cntrygrl_state::decrypted_opcodes_map);
	maincpu.set_decrypted_tag(":decrypted_opcodes");
	maincpu.set_size(0x5000);
	maincpu.set_vblank_int("screen", FUNC(cntrygrl_state::irq0_line_hold));
}

void jangou_state_base::jangou_base(machine_config &config)
{
	cntrygrl(config);

	/* basic machine hardware */
	m_cpu_0->set_addrmap(AS_IO, &jangou_state::jangou_cpu0_io);

	Z80(config, m_cpu_1, MASTER_CLOCK / 8);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_cpu_1, INPUT_LINE_NMI);
}

void jangou_state::jangou(machine_config &config)
{
	jangou_base(config);

	/* basic machine hardware */
	m_cpu_0->set_addrmap(AS_PROGRAM, &jangou_state::jangou_cpu0_map);

	m_cpu_1->set_addrmap(AS_PROGRAM, &jangou_state::jangou_cpu1_map);
	m_cpu_1->set_addrmap(AS_IO, &jangou_state::jangou_cpu1_io);

	HC55516(config, m_cvsd, MASTER_CLOCK / 1024);
	m_cvsd->add_route(ALL_OUTPUTS, "mono", 0.60);
}

void jngolady_state::jngolady(machine_config &config)
{
	jangou_base(config);

	/* basic machine hardware */
	m_cpu_0->set_addrmap(AS_PROGRAM, &jngolady_state::jngolady_cpu0_map);

	m_cpu_1->set_addrmap(AS_PROGRAM, &jngolady_state::jngolady_cpu1_map);
	m_cpu_1->set_addrmap(AS_IO, &jngolady_state::jngolady_cpu1_io);

	NSC8105(config, m_nsc, MASTER_CLOCK / 8);
	m_nsc->set_addrmap(AS_PROGRAM, &jngolady_state::nsc_map);

	GENERIC_LATCH_8(config, m_nsclatch);
	m_nsclatch->data_pending_callback().set_inputline(m_nsc, 0);

	/* sound hardware */
	MSM5205(config, m_msm, XTAL(400'000));
	m_msm->vck_legacy_callback().set(FUNC(jngolady_state::vclk_cb));
	m_msm->set_prescaler_selector(msm5205_device::S96_4B);
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.80);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/*
JANGOU (C)1982 Nichibutsu

CPU:   Z80 *2
XTAL:  19.968MHz
SOUND: AY-3-8910

Location 2-P: HARRIS HCI-55536-5
Location 3-G: MB7051
*/

ROM_START( jangou )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "jg05.bin", 0x00000, 0x02000, CRC(a3cfe33f) SHA1(9ad34a2167568316d242c990ea6fe42dadd4ac30) )
	ROM_LOAD( "jg06.bin", 0x02000, 0x02000, CRC(d8523478) SHA1(f32c2e866c6aeae29f25f0947b07d725ce61d89d) )
	ROM_LOAD( "jg07.bin", 0x04000, 0x02000, CRC(4b30d1fc) SHA1(6f240aa4b7a343f180446581fe95cf7da0fba57b) )
	ROM_LOAD( "jg08.bin", 0x06000, 0x02000, CRC(bb078813) SHA1(a3b7df84629337c83307f49f52338aa983e531ba) )

	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "jg03.bin", 0x00000, 0x02000, CRC(5a113e90) SHA1(7d9ae481680fc640e03f6836f60bccb933bbef31) )
	ROM_LOAD( "jg04.bin", 0x02000, 0x02000, CRC(accd3ab5) SHA1(46a502801da7a56d73a984614f10b20897e340e8) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "jg01.bin", 0x00000, 0x02000, CRC(5034a744) SHA1(b83212b6ff12aaf730c6d3e3d1470d613bbe0d1d) )
	ROM_LOAD( "jg02.bin", 0x02000, 0x02000, CRC(10e7abfe) SHA1(3f5e0c5911baac19c381686e55f207166fe67d44) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "jg3_g.bin", 0x00, 0x20,  CRC(d389549d) SHA1(763486052b34f8a38247820109b114118a9f829f) )
ROM_END

/*

Monoshiri Quiz Osyaberi Macha (c)1983 Logitec
Same board as jangou

CPU: Z80x2
Sound: AY-3-8910, HC55536
XTAL: 20.000MHz

all EPROMs are 2764

POM1.9D -- Programs
POM2.9E |
POM3.9F |
POM4.9G |
POM5.9H /

POM6.9L -- Programs
POM7.9M |
POM8.9N |
POM9.9P /

POM10.5N -- Graphics
POM11.5M |
POM12.BIN|
POM13.BIN /
(12 and 13 is on small daughter board plugged into the socket 5L)

IC3G.BIN - Color PROM

RAM: HM6116LP-3 (16KbitSRAM location 9C, next to POM1)
     M5K4164NP-15 (64KbitDRAM location 4E, 4F, 4G, 4H)

*/

ROM_START( macha )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "pom1.9d", 0x00000, 0x02000, CRC(fbe28b4e) SHA1(2617f8c107b64aa540158a772a725eb68982e095) )
	ROM_LOAD( "pom2.9e", 0x02000, 0x02000, CRC(16a8d176) SHA1(30fe65d3a1744afc70c25c29119db3f5e7126601) )
	ROM_LOAD( "pom3.9f", 0x04000, 0x02000, CRC(c31eeb04) SHA1(65d4873fcaff677f03721139dc80b2fe5108c633) )
	ROM_LOAD( "pom4.9g", 0x06000, 0x02000, CRC(bdb0dd0e) SHA1(d8039fb9996e8707a0c5ca0760d4d6792bbe7270) )
	ROM_LOAD( "pom5.9h", 0x08000, 0x02000, CRC(db6d86e8) SHA1(e9c0f52abd504f39187d0fb7de5b7fffc795204c) )

	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "pom9.9p", 0x00000, 0x02000, CRC(f4d4e0a8) SHA1(914fe35d4434b826ca3b0a230b87017b033dd512) )
	ROM_LOAD( "pom8.9n", 0x02000, 0x02000, CRC(8be49178) SHA1(2233d964a25ef61063b97891f6ad46d6eb10b0c6) )
	ROM_LOAD( "pom7.9m", 0x04000, 0x02000, CRC(48a89180) SHA1(36e916583cc89090880111320537b545620d95fd) )
	ROM_LOAD( "pom6.9l", 0x06000, 0x02000, CRC(7dbafffe) SHA1(2f0c5a340625df30029874ca447f0662ea354547) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "pom10.5n",  0x00000, 0x02000, CRC(5e387db0) SHA1(72fd6d3ae722260cb50d1040faa128f3e5427402) )
	ROM_LOAD( "pom11.5m",  0x02000, 0x02000, CRC(17b54f4e) SHA1(5ecbebc063b5eb888ec1dbf210f54fa3a774ab70) )
	ROM_LOAD( "pom12.bin", 0x04000, 0x02000, CRC(5f1b73ca) SHA1(b8ce01a3975505a2a6b4d4c688b6c7ae4f6df07d) )
	ROM_LOAD( "pom13.bin", 0x06000, 0x02000, CRC(91c489f2) SHA1(a4d2fcebdbdea73ca03722104732e7c6efda5d4d) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "ic3g.bin", 0x00, 0x20,  CRC(d5243aa5) SHA1(d70d5dcb1a3241bec16589ed2eb1cc0054c9ed8e) )
ROM_END

/*
Jangou Lady
(c)1984 Nihon Bussan

CPU:    Z80 x2 (#1,#2)
    (40pin unknown:#3)
SOUND:  AY-3-8910
    MSM5218RS
OSC:    19.968MHz
    400KHz


1.5N    chr.
2.5M
3.5L

4.9P    Z80#1 prg.
5.9N
6.9M
7.9L

8.9H    Z80#2 prg.
9.9G
10.9F
11.9E
12.9D

M13.13  CPU#3 prg. (?)

JL.3G   color
*/

ROM_START( jngolady )
	ROM_REGION( 0xa000, "cpu0", 0 )
	ROM_LOAD( "8.9h",  0x08000, 0x02000, CRC(69e31165) SHA1(81b166c101136ed453a4f4cd88445eb1da5dd0aa) )
	ROM_LOAD( "9.9g",  0x06000, 0x02000, CRC(2faba771) SHA1(d88d0673c9b8cf3783b23c7290253475c9bf397e) )
	ROM_LOAD( "10.9f", 0x04000, 0x02000, CRC(dd311ff9) SHA1(be39ed25343796dc062a612fe82ca19ceb06a9e7) )
	ROM_LOAD( "11.9e", 0x02000, 0x02000, CRC(66cad038) SHA1(c60713615d58a9888e21bfec62fee53558a98eaa) )
	ROM_LOAD( "12.9d", 0x00000, 0x02000, CRC(99c5cc06) SHA1(3a9b3810bb542e252521923ec3026f10f176fa82) )

	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "4.9p", 0x00000, 0x02000, CRC(34cc2c71) SHA1(b407fed069baf3df316f0006a559a6c5e0be5bd0) )
	ROM_LOAD( "5.9n", 0x02000, 0x02000, CRC(42ed7832) SHA1(2681a532049fee494e1d1779d9dc08b17ce6e134) )
	ROM_LOAD( "6.9m", 0x04000, 0x02000, CRC(9e0e7ef4) SHA1(c68d30e60377c1027f4f053c528a80df09b8ee08) )
	ROM_LOAD( "7.9l", 0x06000, 0x02000, CRC(048615d9) SHA1(3c79830db8792ae0746513ed9849cc5d43051ed6) )

	ROM_REGION( 0x10000, "nsc", 0 )
	ROM_LOAD( "m13.13", 0x0f000, 0x01000, CRC(5b20b0e2) SHA1(228d2d931e6daab3572a1f128b5686f84b6a5a29) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "jl.3g", 0x00, 0x20, CRC(15ffff8c) SHA1(5782697f9c9a6bb04bbf7824cd49033c962899f0) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "1.5n", 0x00000, 0x02000, CRC(54027dee) SHA1(0616c12dbf3a0515cf4312fc5e238a61c97f8084) )
	ROM_LOAD( "2.5m", 0x02000, 0x02000, CRC(323dfad5) SHA1(5908acbf80e4b609ee8e5c313ac99717860dd19c) )
	ROM_LOAD( "3.5l", 0x04000, 0x04000, CRC(14688574) SHA1(241eaf1838239e38d11dff3556fb0a609a4b46aa) )
ROM_END

ROM_START( cntrygrl )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "rom4.7l", 0x00000, 0x02000, CRC(adba8e2f) SHA1(2aae77838e3de6e665b32a7fe4ac3f627c35b871)  )
	ROM_LOAD( "rom5.7k", 0x02000, 0x02000, CRC(24d210ed) SHA1(6a0eae9d459975fbaad75bf21284baac3ba4f872) )

	/* wtf,these 2 roms are next to the CPU roms, one is a CPU rom from Moon Quasar, the other a GFX rom from Crazy Climber,
	    I dunno what's going on,the game doesn't appear to need these two....*/
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "rom6.7h", 0x00000, 0x0800, CRC(33965a89) SHA1(92912cea76a472d9b709c664d9818844a07fcc32)  ) // = mq3    Moon Quasar
	ROM_LOAD( "rom7.7j", 0x00800, 0x0800, CRC(481b64cc) SHA1(3f35c545fc784ed4f969aba2d7be6e13a5ae32b7)  ) // = cc06   Crazy Climber (US)

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "rom1.5m", 0x00000, 0x02000, CRC(92033f37) SHA1(aa407c2feb1cbb7cbc6c59656338453c5a670749)  )
	ROM_LOAD( "rom2.5l", 0x02000, 0x02000, CRC(0588cc48) SHA1(f769ece2955eb9f055c499b6243a2fead9d07984)  )
	ROM_LOAD( "rom3.5k", 0x04000, 0x02000, CRC(ce00ff56) SHA1(c5e58707a5dd0f57c34b09de542ef30e96ab95d1)  )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "countrygirl_prom.4f", 0x00, 0x20, CRC(dc54dc52) SHA1(db91a7ae05eb6b6e4b42f91dfe20ac0da6680b46)  )
ROM_END

ROM_START( fruitbun )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "04.8l", 0x00000, 0x02000, CRC(480cb9bf) SHA1(f5e960cd8eaaa85336797fb5d895fa40f1fc4767) )
	ROM_LOAD( "05.8k", 0x02000, 0x02000, CRC(817add97) SHA1(ee5e1b193c22dfd36ac7836ff6bc14e783cb0e86) )

	/* same non-sense like Country Girl... */
	ROM_REGION( 0x1000, "user1", 0 )
	// 06.bin              = galx.1h               Galaxian Part X (moonaln hack)
	ROM_LOAD( "06.8j", 0x00000, 0x0800, CRC(e8810654) SHA1(b6924c7ad765c32714e6abd5bb56b2732edd5855) )
	// 07.bin              = rp9.6h                97.705078%
	// (a gfx rom from an undumped (later? licensed to Nichibutsu?) River Patrol set, it shows the body of the Orca logo if you replace it in rpatrol)
	ROM_LOAD( "07.8h", 0x00800, 0x0800, CRC(3717fa41) SHA1(373e5ef6bef3407da084508c48522c7058568188) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "01.5m", 0x00000, 0x02000, CRC(5ce7f40c) SHA1(ec1998f218a30a1e19ce20b71f170425b4330ea5) )
	ROM_LOAD( "02.5l", 0x02000, 0x02000, CRC(4ca0e465) SHA1(72af3f9e0534ba9a94854513250f6fa82d790549) )
	ROM_LOAD( "03.5k", 0x04000, 0x02000, CRC(8a8f6abd) SHA1(143607c423bfe337e6feed7cd4cc8be5e09fbd5e) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "tbp18s30n.4f", 0x00, 0x20, CRC(dc54dc52) SHA1(db91a7ae05eb6b6e4b42f91dfe20ac0da6680b46) ) //verified on real hardware
ROM_END

ROM_START( cntrygrla )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "5bunny.7l", 0x00000, 0x02000, CRC(ef07e029) SHA1(4be1bea9acaa37c615e937c3d0e0b8454aff2a8c) )
	ROM_LOAD( "5bunny.7k", 0x02000, 0x02000, CRC(cdf822b0) SHA1(a3cae79713cf7ff94a98705b7cba621730dbac1f) )

	/* same non-sense like Country Girl... */
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "5bunny.7h", 0x00000, 0x0800, CRC(18da8863) SHA1(2151bc67173507dc35edc2426c2ef97d7937d01c) )
	ROM_LOAD( "5bunny.7j", 0x00800, 0x0800, CRC(06666bbf) SHA1(3d8eb4ea2d4fc6f3f327e710e19bcb68d8466d80) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "rom1.5m", 0x00000, 0x02000, CRC(92033f37) SHA1(aa407c2feb1cbb7cbc6c59656338453c5a670749)  ) //5bunny.m5
	ROM_LOAD( "rom2.5l", 0x02000, 0x02000, CRC(0588cc48) SHA1(f769ece2955eb9f055c499b6243a2fead9d07984)  ) //5bunny.l5
	ROM_LOAD( "rom3.5k", 0x04000, 0x02000, CRC(ce00ff56) SHA1(c5e58707a5dd0f57c34b09de542ef30e96ab95d1)  ) //5bunny.k5

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "tbp18s30n.4f", 0x00, 0x20, CRC(dc54dc52) SHA1(db91a7ae05eb6b6e4b42f91dfe20ac0da6680b46) ) //verified on real hardware
ROM_END

/****************************************

  Royal Card.
  Amusement.
  1985-01-01

  PCB silkscreened "FD-510"

  1x Z80 @ 2.5 MHz. (measured)
  1x AY-3-8910 @ 1.25 MHz. (measured)

  1x MK48Z02B-15 ZEROPOWER RAM.

  1x Xtal 20.000 MHz.

****************************************/

ROM_START( roylcrdn )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "prg.p1",     0x0000, 0x1000, CRC(9c3b1662) SHA1(b874f88521a21ba6cf9670ed4d81b5d275cf4d12) )
	ROM_LOAD( "prg.p2",     0x1000, 0x1000, CRC(7e10259d) SHA1(d1279922a8c2475c3c73d9960b0a728c0ef851fb) )
	ROM_LOAD( "prg.p3",     0x2000, 0x1000, CRC(06ef7073) SHA1(d3f990d710629b23daec76cd7ad6ccc7e066e710) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "chrgen.cr1", 0x0000, 0x1000, CRC(935d0e1c) SHA1(0d5b067f6931585c8138b211cf73e5f585af8101) )
	ROM_LOAD( "chrgen.cr2", 0x1000, 0x1000, CRC(4429362e) SHA1(0bbb6dedf919e0453be2db6343827c5787d139f3) )
	ROM_LOAD( "chrgen.cr3", 0x2000, 0x1000, CRC(dc059cc9) SHA1(3041e83b9a265adfe4e1da889ae6a18593de0894) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mb7051.3h",  0x0000, 0x0020, CRC(cb6f1aec) SHA1(84136393f9cf8bd836123a31483e9a746ca00cdc) )
ROM_END

/*
  Royal Card (Part Two)
  Miki Corp
  Ver. 1.02
  1982-04-10

  PCB silkscreened (c) MIKI CORP

*/
ROM_START( roylcrdna )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "rc4.ic11",     0x0000, 0x1000, CRC(4a7d9479) SHA1(0c695128b0a66c077013991d062c28a3f00857d1) )
	ROM_LOAD( "rc5.ic12",     0x1000, 0x1000, CRC(577ca478) SHA1(ab138673f74a618f6842f8a6bdf840ef661f27b8) )
	ROM_LOAD( "rc6.ic13",     0x2000, 0x1000, CRC(42507bb5) SHA1(458b5580d9d20687972cccb12490bd58d9b09175) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "rc1.k5", 0x0000, 0x1000, CRC(935d0e1c) SHA1(0d5b067f6931585c8138b211cf73e5f585af8101) )
	ROM_LOAD( "rc2.l5", 0x1000, 0x1000, CRC(4429362e) SHA1(0bbb6dedf919e0453be2db6343827c5787d139f3) )
	ROM_LOAD( "rc3.n5", 0x2000, 0x1000, CRC(50562072) SHA1(7f2ef4f4237d55198af307f6b81af6306c07ce95) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mb7051.h3",  0x0000, 0x0020, CRC(cb6f1aec) SHA1(84136393f9cf8bd836123a31483e9a746ca00cdc) )
ROM_END


ROM_START( luckygrl )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "5.9c", 0x00000, 0x01000, CRC(79b34eb2) SHA1(4b4916e09bfd6573fd2c7a7254fa4419164e0c4d) )
	ROM_LOAD( "6.9e", 0x02000, 0x01000, CRC(06850aa8) SHA1(c23cb6b7b26d5586b1a095dee88228d1613ae7d0) )
	ROM_LOAD( "7.9f", 0x04000, 0x01000, CRC(14a44d23) SHA1(4f84a8f986a8fd9d5ac0636be1bb036c3b2746c2) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "1.5r",      0x00000, 0x2000, CRC(fb429678) SHA1(00e37e90550d9190d06977a5f5ed75b691750cc1) )
	ROM_LOAD( "piggy2.5r", 0x02000, 0x2000, CRC(a3919845) SHA1(45fffe34b7a29ecf8c8feb4152b5c7330ea3ad83) )
	ROM_LOAD( "3.5n",      0x04000, 0x2000, CRC(130cfb89) SHA1(86b2a2142675cbd69d7cccab9b00f4c8863cdcbc) )
	ROM_LOAD( "piggy4.5r", 0x06000, 0x2000, CRC(f5641c95) SHA1(e824b95c80d00789f07391aa5dcc02505bb8e141) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "prom_mb7051.3h", 0x00, 0x20, CRC(dff9f7a1) SHA1(593c99434df5dfd900ec57e3efa459903b589d96) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

// Temporary kludge to make the RNG work
void jngolady_state::init_jngolady()
{
	m_nsc->space(AS_PROGRAM).install_read_handler(0x08, 0x08, read8smo_delegate(*this, NAME([this] () { return u8(machine().rand()); })));
}

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983,  jangou,    0,        jangou,   jangou,   jangou_state,   empty_init,    ROT0, "Nichibutsu",     "Jangou (Japan)",                        MACHINE_SUPPORTS_SAVE )
GAME( 1983,  macha,     0,        jangou,   macha,    jangou_state,   empty_init,    ROT0, "Logitec",        "Monoshiri Quiz Oshaberi Macha (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1984,  jngolady,  0,        jngolady, jngolady, jngolady_state, init_jngolady, ROT0, "Nichibutsu",     "Jangou Lady (Japan)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1984,  cntrygrl,  0,        cntrygrl, cntrygrl, cntrygrl_state, empty_init,    ROT0, "Royal Denshi",   "Country Girl (Japan set 1)",            MACHINE_SUPPORTS_SAVE )
GAME( 1984,  cntrygrla, cntrygrl, cntrygrl, cntrygrl, cntrygrl_state, empty_init,    ROT0, "Nichibutsu",     "Country Girl (Japan set 2)",            MACHINE_SUPPORTS_SAVE )
GAME( 1984,  fruitbun,  cntrygrl, cntrygrl, cntrygrl, cntrygrl_state, empty_init,    ROT0, "Nichibutsu",     "Fruits & Bunny (World?)",               MACHINE_SUPPORTS_SAVE )
GAME( 1985,  roylcrdn,  0,        roylcrdn, roylcrdn, cntrygrl_state, empty_init,    ROT0, "Amusement",      "Royal Card (Nichibutsu HW)",            MACHINE_SUPPORTS_SAVE )
GAME( 1982,  roylcrdna, roylcrdn, roylcrdn, roylcrdn, cntrygrl_state, empty_init,    ROT0, "Miki Corp.",     "Royal Card Part-Two (Nichibutsu HW, Ver. 1.02)",  MACHINE_SUPPORTS_SAVE )
GAME( 1985,  luckygrl,  0,        luckygrl, luckygrl, cntrygrl_state, empty_init,    ROT0, "Wing Co., Ltd.", "Lucky Girl (Wing)",                     MACHINE_SUPPORTS_SAVE )

/*
Some other games that might run on this HW:
    Jangou (non-BET version) (WR score listed on MyCom magazines)
    Jangou Night (first "mature" mahjong ever made)
    Jangou Lady (BET version) (images on the flyer, it might not exists)
    Hana Royal
    Hana Puter
*/
