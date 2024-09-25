// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*******************************************************************************

Tora Tora (c) 1980 Game Plan

driver by Nicola Salmoria, qwijibo


deviations from schematics verified on set 2 pcb:

main pcb:
- U33.3 connected to /IRQ line via inverter U67.9,
  providing timer IRQ for input polling.

- U43 removed from timer circuit, U52.9 wired directly to
  U32.5, increasing timer frequency to 250 Hz.

audio pcb:
- U6.10 wired to U14.21, cut from R14/16
- R16 wired to R1 in series, R1 cut from GND
- R14 wired to GND instead of U6.10

- U5.10 wired to U15.21, cut from R13/15
- R15 wired to R2 in series, R2 cut from GND
- R13 wired to GND instead of U5.10

- EXT VCO DACs (U11, U12) and surrounding logic not placed
- Numerous changes to SN74677 timing component R & C values


TODO:
- The game reads some unmapped memory addresses, missing ROMs? There's an empty
  socket for U3 on the board, which should map at 5000-57ff, however the game
  reads mostly from 4800-4fff, which would be U6 according to the schematics.

- The manual mentions dip switch settings and the schematics show the switches,
  the game reads them but ignores them, forcing 1C/1C and 3 lives.
  Maybe the dump is from a proto?
  Set 2 dump does read dip switches, so likely not a proto.

- Bullet and explosion audio frequencies seem incorrect

- Who designed/developed the game? Taiyo System is mentioned online, but there's
  no solid proof. It's more likely an American game. At the end of tora.u11,
  there's "EMS INC.,1979".

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "sound/sn76477.h"

#include "screen.h"
#include "speaker.h"

#include "toratora.lh"


namespace {

class toratora_state : public driver_device
{
public:
	toratora_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_irq(*this, "irq"),
		m_pia(*this, "pia%u", 1U),
		m_screen(*this, "screen"),
		m_sn(*this, "sn%u", 1U),
		m_videoram(*this, "videoram"),
		m_dsw(*this, "DSW")
	{ }

	void toratora(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_irq;
	required_device_array<pia6821_device, 3> m_pia;
	required_device<screen_device> m_screen;
	required_device_array<sn76477_device, 2> m_sn;
	required_shared_ptr<uint8_t> m_videoram;
	required_ioport m_dsw;

	emu_timer *m_timer;
	uint8_t m_timer_count = 0;
	bool m_timer_clk = false;
	bool m_clear_tv = false;
	bool m_dsw_enable = true;

	void clear_tv_w(uint8_t data);
	uint8_t timer_r();
	void clear_timer_w(uint8_t data);
	uint8_t dsw_r();
	void dsw_enable_w(int state);
	void coin_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	template<int N> void sn_vco_voltage_w(uint8_t data);
	void sn1_w(uint8_t data);
	void sn2_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(toratora_timer);

	void main_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Initialization
 *
 *************************************/

void toratora_state::machine_start()
{
	// 500Hz timer from 2*9602
	m_timer = timer_alloc(FUNC(toratora_state::toratora_timer), this);
	m_timer->adjust(attotime::from_hz(500), 0, attotime::from_hz(500));

	m_pia[0]->ca1_w(0);
	m_pia[0]->ca2_w(0);

	save_item(NAME(m_timer_count));
	save_item(NAME(m_timer_clk));
	save_item(NAME(m_clear_tv));
	save_item(NAME(m_dsw_enable));
}

void toratora_state::machine_reset()
{
	m_timer_count = 0;
	m_clear_tv = false;
}



/*************************************
 *
 *  Video hardware
 *
 *************************************/

uint32_t toratora_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		uint8_t y = offs >> 5;
		uint8_t x = offs << 3;
		uint8_t data = m_videoram[offs];

		for (int i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? rgb_t::white() : rgb_t::black();
			if (cliprect.contains(x, y))
				bitmap.pix(y, x) = pen;

			data = data << 1;
			x = x + 1;
		}

		// the video system clears as it writes out the pixels
		if (m_clear_tv)
			m_videoram[offs] = 0;
	}

	m_clear_tv = false;

	return 0;
}

void toratora_state::clear_tv_w(uint8_t data)
{
	m_clear_tv = true;
}



/*************************************
 *
 *  Audio hardware
 *
 *************************************/

template<int N>
void toratora_state::sn_vco_voltage_w(uint8_t data)
{
	m_sn[N]->vco_voltage_w(2.35 * (data & 0x7f) / 128.0);
	m_sn[N]->enable_w(BIT(data, 7));
}

// U14
void toratora_state::sn1_w(uint8_t data)
{
	static const double resistances[] =
	{
		RES_INF, // N/C
		RES_INF, // R39 not placed
		RES_K(10) + RES_K(10) + RES_K(24) + RES_K(51) + RES_K(100) + RES_K(240),
		RES_K(10) + RES_K(10) + RES_K(24) + RES_K(51) + RES_K(100),
		RES_K(10) + RES_K(10) + RES_K(24) + RES_K(51),
		RES_K(10) + RES_K(10) + RES_K(24),
		RES_K(10) + RES_K(10),
		RES_INF
	};

	m_sn[0]->mixer_a_w(BIT(data, 0));
	m_sn[0]->mixer_b_w(BIT(data, 1));
	m_sn[0]->mixer_c_w(BIT(data, 2));
	m_sn[0]->envelope_1_w(BIT(data, 3));
	m_sn[0]->envelope_2_w(BIT(data, 4));

	uint8_t res = data >> 5 & 0x7;
	m_sn[0]->slf_res_w(resistances[res]);
	m_sn[0]->slf_cap_voltage_w(res == 0x7 ? 2.5 : sn76477_device::EXTERNAL_VOLTAGE_DISCONNECT);

	// Seems like the output should be muted when res == 0, but unsure of exact mechanism
	m_sn[0]->amplitude_res_w((res == 0x0) ? RES_INF : RES_K(47));
}

// U15
void toratora_state::sn2_w(uint8_t data)
{
	static const double resistances[] =
	{
		RES_INF, // N/C
		RES_K(10) + RES_K(10) + RES_K(24) + RES_K(51) + RES_K(100) + RES_M(240) + RES_M(2),
		RES_K(10) + RES_K(10) + RES_K(24) + RES_K(51) + RES_K(100) + RES_K(240),
		RES_K(10) + RES_K(10) + RES_K(24) + RES_K(51) + RES_K(100),
		RES_K(10) + RES_K(10) + RES_K(24) + RES_K(51),
		RES_K(10) + RES_K(10) + RES_K(24),
		RES_K(10) + RES_K(10),
		RES_INF
	};

	m_sn[1]->mixer_a_w(BIT(data, 0));
	m_sn[1]->mixer_b_w(BIT(data, 1));
	m_sn[1]->mixer_c_w(BIT(data, 2));
	m_sn[1]->envelope_1_w(BIT(data, 3));
	m_sn[1]->envelope_2_w(BIT(data, 4));

	uint8_t res = data >> 5 & 0x7;
	m_sn[1]->slf_res_w(resistances[res]);

	// TODO: Determine proper 7441 output voltage here.
	// Datasheet lists 2.5 V max under worst conditions, probably much lower in reality?
	// However, shot audio is muted if V < 1.0, as mixer state is set to SLF & VCO.
	// Should SLF FF state be inverted?
	m_sn[1]->slf_cap_voltage_w(res == 0x7 ? 2.5 : sn76477_device::EXTERNAL_VOLTAGE_DISCONNECT);
}



/*************************************
 *
 *  Timer
 *
 *************************************/

TIMER_CALLBACK_MEMBER(toratora_state::toratora_timer)
{
	// RAM refresh circuit halts the cpu for 64 cycles
	m_maincpu->adjust_icount(-64);

	m_timer_clk = !m_timer_clk;

	if (m_timer_clk)
	{
		// timer counting at 250 Hz. (500 Hz / 2 via U52.9)
		// U43 removed from circuit, U52.9 wired to U32.5)
		m_timer_count++;

		// U33 bit 0 routed to /IRQ line after inverting through U67.9
		m_irq->in_w<0>(BIT(m_timer_count, 4));

		// also, when the timer overflows, watchdog would kick in
		if (m_timer_count == 0)
			machine().schedule_soft_reset();
	}
}

uint8_t toratora_state::timer_r()
{
	return m_timer_count;
}

void toratora_state::clear_timer_w(uint8_t data)
{
	m_timer_count = 0;
	m_irq->in_w<0>(0);
}



/*************************************
 *
 *  Misc. I/O
 *
 *************************************/

void toratora_state::dsw_enable_w(int state)
{
	m_dsw_enable = !state;
}

uint8_t toratora_state::dsw_r()
{
	return m_dsw_enable ? m_dsw->read() : 0;
}

void toratora_state::coin_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	data |= ~mem_mask;
	machine().bookkeeping().coin_counter_w(0, data & 0x20);
}



/*************************************
 *
 *  Memory handlers
 *
 *  No mirrors, all addresses are
 *  fully decoded by the hardware!
 *
 *************************************/

void toratora_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x7fff).rom(); // not fully populated
	map(0x8000, 0x9fff).ram().share(m_videoram);
	map(0xa000, 0xf047).noprw();
	map(0xf048, 0xf049).noprw();
	map(0xf04a, 0xf04a).w(FUNC(toratora_state::clear_tv_w)); // the read is mark !LEDEN, but not used
	map(0xf04b, 0xf04b).rw(FUNC(toratora_state::timer_r), FUNC(toratora_state::clear_timer_w));
	map(0xf04c, 0xf09f).noprw();
	map(0xf0a0, 0xf0a3).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xf0a4, 0xf0a7).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xf0a8, 0xf0ab).rw(m_pia[2], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xf0ac, 0xf7ff).noprw();
	map(0xf800, 0xffff).rom();
}



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( toratora )
	PORT_START("INPUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER("pia1", pia6821_device, ca2_w)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_WRITE_LINE_DEVICE_MEMBER("pia1", pia6821_device, ca1_w)

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )     PORT_DIPLOCATION("U13:!1,!2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )     PORT_DIPLOCATION("U13:!3,!4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )     PORT_DIPLOCATION("U13:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) )      PORT_DIPLOCATION("U13:!6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void toratora_state::toratora(machine_config &config)
{
	// basic machine hardware
	M6800(config, m_maincpu, 5.185_MHz_XTAL / 8); // 5.185 MHz XTAL divided by 8 (@ U94.12)
	m_maincpu->set_addrmap(AS_PROGRAM, &toratora_state::main_map);

	INPUT_MERGER_ANY_HIGH(config, m_irq);
	m_irq->output_handler().set_inputline(m_maincpu, 0);

	PIA6821(config, m_pia[0]); // U1
	m_pia[0]->readpa_handler().set_ioport("INPUT");
	m_pia[0]->writepb_handler().set(FUNC(toratora_state::coin_w));
	m_pia[0]->irqa_handler().set(m_irq, FUNC(input_merger_device::in_w<1>));
	m_pia[0]->irqb_handler().set(m_irq, FUNC(input_merger_device::in_w<2>));

	PIA6821(config, m_pia[1]); // U2
	m_pia[1]->writepa_handler().set(FUNC(toratora_state::sn_vco_voltage_w<0>));
	m_pia[1]->readpb_handler().set(FUNC(toratora_state::dsw_r));
	m_pia[1]->writepb_handler().set(FUNC(toratora_state::sn1_w));
	m_pia[1]->ca2_handler().set(m_sn[0], FUNC(sn76477_device::vco_w));
	m_pia[1]->cb2_handler().set(FUNC(toratora_state::dsw_enable_w));

	PIA6821(config, m_pia[2]); // U3
	m_pia[2]->writepa_handler().set(FUNC(toratora_state::sn_vco_voltage_w<1>));
	m_pia[2]->writepb_handler().set(FUNC(toratora_state::sn2_w));
	m_pia[2]->ca2_handler().set(m_sn[1], FUNC(sn76477_device::vco_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_raw(5.185_MHz_XTAL, 256+40, 0, 256, 256+31+4, 8, 248);
	m_screen->set_screen_update(FUNC(toratora_state::screen_update));

	// audio hardware
	SPEAKER(config, "mono").front_center();

	for (int i = 0; i < 2; i++)
	{
		SN76477(config, m_sn[i]);
		m_sn[i]->set_noise_params(RES_K(47), RES_K(470), CAP_P(470));
		m_sn[i]->set_decay_res(RES_M(2));
		m_sn[i]->set_attack_params(CAP_U(0.2), RES_K(3.3));
		m_sn[i]->set_amp_res(RES_K(47));
		m_sn[i]->set_feedback_res(RES_K(50));
		m_sn[i]->set_vco_params(0, CAP_U(0.1), RES_K(51));
		m_sn[i]->set_pitch_voltage(5.0);
		m_sn[i]->set_slf_params(CAP_U(1.0), RES_K(10));
		m_sn[i]->set_oneshot_params(CAP_U(0.1), RES_K(100));
		m_sn[i]->set_vco_mode(0);
		m_sn[i]->set_mixer_params(0, 0, 0);
		m_sn[i]->set_envelope_params(0, 0);
		m_sn[i]->set_enable(1);
		m_sn[i]->add_route(ALL_OUTPUTS, "mono", 0.50);
	}
}



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( toratora )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tora.u1",  0x1000, 0x0800, CRC(413c743a) SHA1(a887dfaaee557327a1699bb424488b934dab8612) )
	ROM_LOAD( "tora.u10", 0x1800, 0x0800, CRC(dc771b1c) SHA1(1bd81decb4d0a854878227c52d45ac0eea0602ec) )
	ROM_LOAD( "tora.u2",  0x2000, 0x0800, CRC(c574c664) SHA1(9f41a53ca51d04e5bec7525fe83c5f4bdfcf128d) )
	ROM_LOAD( "tora.u9",  0x2800, 0x0800, CRC(b67aa11f) SHA1(da9e77255640a4b32eed2be89b686b98a248bd72) )
	ROM_LOAD( "tora.u11", 0xf800, 0x0800, CRC(55135d6f) SHA1(c48f180a9d6e894aafe87b2daf74e9a082f4600e) )
ROM_END


/* Tora Tora? Game Plan?
Etched in copper on top of board            20-00047C
                                            20-10051A

Etched in copper on back of daughter board  20-00048C
                                            20-10052A

ROM text showed     TORA TOR*               * was A with bit 7 set
                    1980 GAME PLAN,INC

and war stuff (PLANE, BOMB, SQUAD, etc)

.u2   2716  handwritten sticker U-2
.u9   2716  handwritten sticker U-9
.u10  2716  handwritten sticker U-10
.u11  2716  handwritten sticker U-11

open 24 pin socket @ U1 and U3
open 40 pin socket @ U42

Main board
crystal with 5 185 on the top
5280        x8
socketed    ds8833  x2
socketed    ds8t28  x2

Daughter board
open 40 pin socket @ U3 @ U2
76477       x2 */

ROM_START( toratorab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1027.u1",  0x1000, 0x0800, BAD_DUMP CRC(413c743a) SHA1(a887dfaaee557327a1699bb424488b934dab8612) ) // rom u1 is missing in this set, using the toratora one
	ROM_LOAD( "1027.u10", 0x1800, 0x0800, CRC(6a906292) SHA1(4ceff91b7dcd398e57cd19a91d2199c09cb37c39) )
	ROM_LOAD( "1027.u2",  0x2000, 0x0800, CRC(c1331648) SHA1(379101c6c1b8dab3e043ece01579cc96f6bb18a9) )
	ROM_LOAD( "1027.u9",  0x2800, 0x0800, CRC(59b021b5) SHA1(ea5a0c1f58c0e08231969ad161b79af6e1ae4431) )
	ROM_LOAD( "1027.u11", 0xf800, 0x0800, CRC(336f6659) SHA1(ea1151db54b68316908874da6983d6de5c94c29e) )
ROM_END

} // anonymous namespace



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAMEL( 1980, toratora,  0,        toratora, toratora, toratora_state, empty_init, ROT90, "Game Plan", "Tora Tora (prototype?)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_toratora )
GAMEL( 1980, toratorab, toratora, toratora, toratora, toratora_state, empty_init, ROT90, "Game Plan", "Tora Tora (set 2)",      MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_toratora )
