// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/*************************************************************************

    RDI Video Sytems' Thayer's Quest

**************************************************************************

    TODO:
    - No viable solution for reading configuration DIPs at init time,
      so the driver is hard-coded to LD-V1000 mode.
    - conflict between keyboard and service mode default key (F2)

*************************************************************************/

#include "emu.h"
#include "cpu/cop400/cop400.h"
#include "cpu/z80/z80.h"
#include "machine/ldstub.h"
#include "machine/ldv1000hle.h"
#include "sound/ssi263hle.h"
#include "speaker.h"

#include "thayers.lh"

#define LOG_IRQS        (1U << 1)
#define LOG_COP         (1U << 2)
#define LOG_KEYBOARD    (1U << 3)
#define LOG_PLAYER      (1U << 4)
#define LOG_COINS       (1U << 5)
#define LOG_ALL         (LOG_IRQS | LOG_COP | LOG_KEYBOARD | LOG_PLAYER | LOG_COINS)

#define VERBOSE (0)
#include "logmacro.h"

namespace {

class thayers_state : public driver_device
{
public:
	thayers_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_player(*this, "laserdisc")
		, m_maincpu(*this, "maincpu")
		, m_ssi(*this, "ssi")
		, m_row(*this, "ROW.%u", 0)
		, m_coins(*this, "COIN")
		, m_dswb(*this, "DSWB")
		, m_digits(*this, "digit%u", 0U)
	{
	}

	void thayers(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void thayers_map(address_map &map) ATTR_COLD;
	void thayers_io_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(firstirq_tick);
	TIMER_CALLBACK_MEMBER(intrq_tick);
	void ssi_data_request_w(int state);

	required_device<parallel_laserdisc_device> m_player;
	required_device<z80_device> m_maincpu;
	required_device<ssi263hle_device> m_ssi;
	required_ioport_array<10> m_row;
	required_ioport m_coins;
	required_ioport m_dswb;
	output_finder<16> m_digits;

	emu_timer *m_intrq_timer = nullptr;
	emu_timer *m_phoneme_timer = nullptr;

	u8 m_laserdisc_data = 0;
	u8 m_laserdisc_control = 0;

	u8 m_rx_bit = 0;
	u8 m_keylatch = 0;
	u8 m_keydata = 0;
	u8 m_kbdata = 0;
	u8 m_kbclk = 0;

	u8 m_cop_data_latch = 0;
	u8 m_cop_data_latch_enable = 0;
	u8 m_cop_l = 0;
	u8 m_cop_cmd_latch = 0;

	u8 m_z80_int = 0;
	u8 m_periodic_int = 0;
	u8 m_timer_int = 1;
	u8 m_data_rdy_int = 1;
	u8 m_ssi_data_request = 1;
	u8 m_cart_present = 1;

	void periodic_int_ack_w(u8 data);
	u8 irqstate_r();
	void timer_int_ack_w(u8 data);
	void data_rdy_int_ack_w(u8 data);
	void check_interrupt();

	void cop_d_w(u8 data);
	u8 cop_data_r();
	void cop_data_w(u8 data);
	u8 cop_l_r();
	void cop_l_w(u8 data);
	u8 cop_g_r();
	void cop_g_w(u8 data);

	void control_w(u8 data);
	void control2_w(u8 data);
	u8 dsw_b_r();

	int kbdata_r();
	void kbclk_w(int state);

	u8 laserdisc_data_r();
	void laserdisc_data_w(u8 data);
	void laserdisc_control_w(u8 data);

	void den_w(offs_t offset, u8 data);
};

void thayers_state::machine_start()
{
	m_digits.resolve();

	m_intrq_timer = timer_alloc(FUNC(thayers_state::intrq_tick), this);

	save_item(NAME(m_laserdisc_data));
	save_item(NAME(m_laserdisc_control));

	save_item(NAME(m_rx_bit));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_keydata));
	save_item(NAME(m_kbdata));
	save_item(NAME(m_kbclk));

	save_item(NAME(m_cop_data_latch));
	save_item(NAME(m_cop_data_latch_enable));
	save_item(NAME(m_cop_l));
	save_item(NAME(m_cop_cmd_latch));

	save_item(NAME(m_z80_int));
	save_item(NAME(m_periodic_int));
	save_item(NAME(m_timer_int));
	save_item(NAME(m_data_rdy_int));
	save_item(NAME(m_ssi_data_request));
	save_item(NAME(m_cart_present));
}

void thayers_state::machine_reset()
{
	m_intrq_timer->adjust(attotime::never);

	m_laserdisc_data = 0;
	m_laserdisc_control = 0;

	m_rx_bit = 0;
	m_keylatch = 0;
	m_keydata = m_row[m_keylatch]->read();
	m_kbdata = 1;
	m_kbclk = 0;

	m_cop_data_latch = 0;
	m_cop_data_latch_enable = 0;
	m_cop_l = 0;
	m_cop_cmd_latch = 0;

	m_z80_int = 0;
	m_periodic_int = 0;
	m_timer_int = 1;
	m_data_rdy_int = 1;
	m_ssi_data_request = 1;
	m_cart_present = 1;
}


TIMER_CALLBACK_MEMBER(thayers_state::intrq_tick)
{
	m_periodic_int = 0;
	check_interrupt();
}

void thayers_state::ssi_data_request_w(int state)
{
	m_ssi_data_request = state;
	check_interrupt();
}

void thayers_state::check_interrupt()
{
	const bool old_int = m_z80_int;
	m_z80_int = (!m_timer_int || !m_data_rdy_int || !m_ssi_data_request || !m_periodic_int);
	if (m_z80_int == old_int)
		return;

	if (m_z80_int)
		LOGMASKED(LOG_IRQS, "%s: Raising Z80 IRQ (timer %d, data %d, SSI %d, periodic %d)\n", machine().describe_context(), m_timer_int, m_data_rdy_int, m_ssi_data_request, m_periodic_int);
	else
		LOGMASKED(LOG_IRQS, "%s: Clearing Z80 IRQ\n", machine().describe_context());

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_z80_int ? ASSERT_LINE : CLEAR_LINE);
}

void thayers_state::periodic_int_ack_w(u8 data)
{
	// T = 1.1 * R30 * C53 = 1.1 * 750K * 0.01uF = 8.25 ms
	m_periodic_int = 1;
	check_interrupt();

	m_intrq_timer->adjust(attotime::from_usec(8250));
	LOGMASKED(LOG_IRQS, "%s: periodic_int_ack_w: %02x\n", machine().describe_context(), data);
}

u8 thayers_state::irqstate_r()
{
	//  bit     description
	//
	//  0
	//  1
	//  2       SSI263 A/_R
	//  3       tied to +5V
	//  4       _TIMER INT
	//  5       _DATA RDY INT
	//  6       _CART PRES
	//  7

	const u8 data = (m_cart_present << 6) | (m_data_rdy_int << 5) | (m_timer_int << 4) | 0x08 | (m_ssi_data_request << 2);
	LOGMASKED(LOG_IRQS, "%s: irqstate_r: %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_IRQS, "%s:             SSI263 A/_R: %d\n", machine().describe_context(), BIT(data, 2));
	LOGMASKED(LOG_IRQS, "%s:             _TIMER_INT: %d\n", machine().describe_context(), BIT(data, 4));
	LOGMASKED(LOG_IRQS, "%s:             _DATA_RDY_INT: %d\n", machine().describe_context(), BIT(data, 5));
	LOGMASKED(LOG_IRQS, "%s:             _CART_PRES: %d\n", machine().describe_context(), BIT(data, 6));
	return data;
}

void thayers_state::timer_int_ack_w(u8 data)
{
	LOGMASKED(LOG_IRQS, "%s: timer_int_ack_w: %02x\n", machine().describe_context(), data);
	m_timer_int = 1;
	check_interrupt();
}

void thayers_state::data_rdy_int_ack_w(u8 data)
{
	LOGMASKED(LOG_IRQS, "%s: data_rdy_int_ack_w: %02x\n", machine().describe_context(), data);
	m_data_rdy_int = 1;
	check_interrupt();
}

void thayers_state::cop_d_w(u8 data)
{
	//  bit     description
	//
	//  D0      _TIMER INT
	//  D1      _DATA RDY INT
	//  D2
	//  D3

	LOGMASKED(LOG_COP, "%s: cop_d_w = %02x\n", machine().describe_context(), data);
	if (!BIT(data, 0))
	{
		LOGMASKED(LOG_COP | LOG_IRQS, "%s:           /TIMER_INT: %d\n", machine().describe_context(), BIT(data, 0));
		m_timer_int = 0;
	}

	if (!BIT(data, 1))
	{
		LOGMASKED(LOG_COP | LOG_IRQS, "%s:           /DATA_RDY_INT: %d\n", machine().describe_context(), BIT(data, 1));
		m_data_rdy_int = 0;
	}

	check_interrupt();
}


u8 thayers_state::cop_data_r()
{
	u8 data = m_cop_l;
	if (!m_cop_data_latch_enable)
	{
		data = m_cop_data_latch;
	}

	LOGMASKED(LOG_COP, "%s: cop_data_r: %02x (data latch enable: %d, %02x/%02x)\n", machine().describe_context(), data, m_cop_data_latch_enable, m_cop_l, m_cop_data_latch);
	return data;
}

void thayers_state::cop_data_w(u8 data)
{
	m_cop_data_latch = data;
	LOGMASKED(LOG_COP, "%s: cop_data_w: %02x (data latch enable: %d)\n", machine().describe_context(), m_cop_data_latch, m_cop_data_latch_enable);
}

u8 thayers_state::cop_l_r()
{
	u8 data = 0;
	if (!m_cop_data_latch_enable)
	{
		data = m_cop_data_latch;
	}

	LOGMASKED(LOG_COP, "%s: cop_l_r: %02x (data latch enable: %d, %02x/%02x)\n", machine().describe_context(), data, m_cop_data_latch_enable, 0, m_cop_data_latch);
	return data;
}

void thayers_state::cop_l_w(u8 data)
{
	LOGMASKED(LOG_COP, "%s: cop_l_w = %02x\n", machine().describe_context(), data);
	m_cop_l = data;
}

u8 thayers_state::cop_g_r()
{
	//  bit     description
	//
	//  G0      U16 Q0
	//  G1      U16 Q1
	//  G2      U16 Q2
	//  G3

	const u8 data = m_cop_cmd_latch;
	LOGMASKED(LOG_COP, "%s: cop_g_r: %02x\n", machine().describe_context(), data);
	return data;
}

void thayers_state::control_w(u8 data)
{
	//  bit     description
	//
	//  0
	//  1       _CS128A
	//  2       _BANKSEL1
	//  3
	//  4
	//  5       COP G0
	//  6       COP G1
	//  7       COP G2

	m_cop_cmd_latch = (data >> 5) & 0x07;
	LOGMASKED(LOG_COP, "%s: control_w: %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_COP, "%s:            /CS128A: %d\n", machine().describe_context(), BIT(data, 1));
	LOGMASKED(LOG_COP, "%s:            /BANKSEL1: %d\n", machine().describe_context(), BIT(data, 2));
	LOGMASKED(LOG_COP, "%s:            COP G0: %d\n", machine().describe_context(), BIT(data, 5));
	LOGMASKED(LOG_COP, "%s:            COP G1: %d\n", machine().describe_context(), BIT(data, 6));
	LOGMASKED(LOG_COP, "%s:            COP G2: %d\n", machine().describe_context(), BIT(data, 7));
}

void thayers_state::cop_g_w(u8 data)
{
	//  bit     description
	//
	//  G0
	//  G1
	//  G2
	//  G3      U17 (L-port latch) enable

	m_cop_data_latch_enable = BIT(data, 3);
	LOGMASKED(LOG_COP, "%s: cop_g_w = %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_COP, "%s:           U17 Enable: %d\n", machine().describe_context(), BIT(data, 3));
}


int thayers_state::kbdata_r()
{
	const u8 data = m_kbdata;
	LOGMASKED(LOG_KEYBOARD, "%s: kbdata_r: %02x\n", machine().describe_context(), data);
	return data;
}

void thayers_state::kbclk_w(int state)
{
	if (m_kbclk == state)
		return;

	m_kbclk = state;

	if (m_kbclk && state)
		return;

	m_rx_bit++;

	// 1, 1, 0, 1, Q9, P3, P2, P1, P0, 0
	switch (m_rx_bit)
	{
		case 0:
		case 1:
		case 3:
			m_kbdata = 1;
			break;

		case 2:
		case 9:
			m_kbdata = 0;
			break;

		case 10:
			m_rx_bit = 0;
			m_kbdata = 1;

			m_keylatch++;

			if (m_keylatch == 10)
				m_keylatch = 0;

			m_keydata = m_row[m_keylatch]->read();
			break;

		case 4:
			m_kbdata = (m_keylatch == 9);
			break;

		default:
			m_kbdata = BIT(m_keydata, 3);
			m_keydata <<= 1;
			break;
	}
}


void thayers_state::control2_w(u8 data)
{
	//  bit     description
	//
	//  0
	//  1       _RESOI
	//  2       _ENCARTDET
	//  3
	//  4
	//  5
	//  6
	//  7

	LOG("%s: control2_w: %02x\n", machine().describe_context(), data);
	LOG("%s:             /RESOI: %d\n", machine().describe_context(), BIT(data, 1));
	LOG("%s:             /ENCARTDET: %d\n", machine().describe_context(), BIT(data, 2));
}

u8 thayers_state::dsw_b_r()
{
	const u8 data = (m_coins->read() & 0xf0) | (m_dswb->read() & 0x0f);
	LOGMASKED(LOG_COINS, "%s: dsw_b_r: %02x, data & 0x30 is %02x\n", machine().describe_context(), data, data & 0x30);
	return data;
}

u8 thayers_state::laserdisc_data_r()
{
	const u8 data = m_player->data_r();
	LOGMASKED(LOG_PLAYER, "%s: laserdisc_data_r: %02x\n", machine().describe_context(), data);
	return data;
}

void thayers_state::laserdisc_data_w(u8 data)
{
	LOGMASKED(LOG_PLAYER, "%s: laserdisc_data_w: %02x\n", machine().describe_context(), data);
	m_laserdisc_data = data;
	if (BIT(m_laserdisc_control, 5))
	{
		m_player->data_w(m_laserdisc_data);
	}
}

void thayers_state::laserdisc_control_w(u8 data)
{
	//  bit     description
	//
	//  0
	//  1
	//  2
	//  3
	//  4       coin counter
	//  5       U16 (laserdisc data) output enable
	//  6       ENTER if switch B5 closed
	//  7       INT/_EXT

	LOGMASKED(LOG_PLAYER, "%s: laserdisc_control_w: %02x\n", machine().describe_context(), data);
	LOGMASKED(LOG_PLAYER, "%s:                      Coin Counter: %d\n", machine().describe_context(), BIT(data, 4));
	LOGMASKED(LOG_PLAYER, "%s:                      U16 Enable: %d\n", machine().describe_context(), BIT(data, 5));
	LOGMASKED(LOG_PLAYER, "%s:                      ENTER: %d\n", machine().describe_context(), BIT(data, 6));
	LOGMASKED(LOG_PLAYER, "%s:                      INT/_EXT: %d\n", machine().describe_context(), BIT(data, 7));

	const u8 diff = m_laserdisc_control ^ data;
	m_laserdisc_control = data;

	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));

	if (BIT(diff, 5) && BIT(data, 5))
	{
		m_player->data_w(m_laserdisc_data);
		m_player->enter_w(BIT(data, 6) ? CLEAR_LINE : ASSERT_LINE);
	}
}

void thayers_state::den_w(offs_t offset, u8 data)
{
	//  bit     description
	//
	//  0       DD0
	//  1       DD1
	//  2       DD2
	//  3       DD3
	//  4       DA0
	//  5       DA1
	//  6       DA2
	//  7       N/C

	static constexpr u8 LED_MAP[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x00 };
	m_digits[(offset << 3 & 8) | (data >> 4 & 7)] = LED_MAP[data & 0x0f];
}


void thayers_state::thayers_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).ram();
	map(0xc000, 0xdfff).rom();
}

void thayers_state::thayers_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).m(m_ssi, FUNC(ssi263hle_device::map));
	map(0x20, 0x20).w(FUNC(thayers_state::control_w));
	map(0x40, 0x40).rw(FUNC(thayers_state::irqstate_r), FUNC(thayers_state::control2_w));
	map(0x80, 0x80).rw(FUNC(thayers_state::cop_data_r), FUNC(thayers_state::cop_data_w));
	map(0xa0, 0xa0).w(FUNC(thayers_state::timer_int_ack_w));
	map(0xc0, 0xc0).w(FUNC(thayers_state::data_rdy_int_ack_w));
	map(0xf0, 0xf0).r(FUNC(thayers_state::laserdisc_data_r));
	map(0xf1, 0xf1).r(FUNC(thayers_state::dsw_b_r));
	map(0xf2, 0xf2).portr("DSWA");
	map(0xf3, 0xf3).w(FUNC(thayers_state::periodic_int_ack_w));
	map(0xf4, 0xf4).w(FUNC(thayers_state::laserdisc_data_w));
	map(0xf5, 0xf5).w(FUNC(thayers_state::laserdisc_control_w));
	map(0xf6, 0xf7).w(FUNC(thayers_state::den_w));
}


static INPUT_PORTS_START( thayers )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Time Per Coin" ) PORT_DIPLOCATION( "A:3,2,1" )
	PORT_DIPSETTING(    0x07, "110 Seconds" )
	PORT_DIPSETTING(    0x06, "95 Seconds" )
	PORT_DIPSETTING(    0x05, "80 Seconds" )
	PORT_DIPSETTING(    0x04, "70 Seconds" )
	PORT_DIPSETTING(    0x03, "60 Seconds" )
	PORT_DIPSETTING(    0x02, "45 Seconds" )
	PORT_DIPSETTING(    0x01, "30 Seconds" )
	PORT_DIPSETTING(    0x00, DEF_STR ( Free_Play ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) ) PORT_DIPLOCATION( "A:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) ) PORT_DIPLOCATION( "A:5" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION( "A:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Attract Mode Audio" ) PORT_DIPLOCATION( "A:7" )
	PORT_DIPSETTING(    0x40, "Always Playing" )
	PORT_DIPSETTING(    0x00, "One Out of 8 Times" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "A:8" )

	PORT_START("DSWB")
	PORT_SERVICE_DIPLOC( 0x01, 0x01, "B:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "B:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "B:3" )
	PORT_DIPNAME( 0x18, 0x18, "LD Player" ) PORT_DIPLOCATION( "B:5,4" )
	PORT_DIPSETTING(    0x18, "LDV-1000" )
	PORT_DIPSETTING(    0x00, "PR-7820 (Not Working)" )
	PORT_DIPUNUSED_DIPLOC( 0xe0, IP_ACTIVE_LOW, "B:8,7,6" )

	PORT_START("COIN")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("laserdisc", FUNC(parallel_laserdisc_device::status_strobe_r))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("laserdisc", FUNC(parallel_laserdisc_device::ready_r))

	PORT_START("ROW.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Yes") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 / Clear") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Items") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W / Amulet") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z / Spell of Release") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Drop Item") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("E / Black Mace") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("S / Dagger") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("X / Scepter") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Give Score") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R / Blood Sword") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("D / Great Circlet") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("C / Spell of Seeing") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Replay") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("T / Chalice") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F / Hunting Horn") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("V / Shield") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Combine Action") PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y / Coins") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("G / Long Bow") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("B / Silver Wheat") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Save Game") PORT_CODE(KEYCODE_F7)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("U / Cold Fire") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("H / Medallion") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("N / Staff") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Update") PORT_CODE(KEYCODE_F8)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("I / Crown") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("J / Onyx Seal") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M / Spell of Understanding") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW.8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Hint") PORT_CODE(KEYCODE_F9)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("O / Crystal") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K / Orb of Quoid") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 / Space") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW.9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("No") PORT_CODE(KEYCODE_F10)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 / Enter") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void thayers_state::thayers(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &thayers_state::thayers_map);
	m_maincpu->set_addrmap(AS_IO, &thayers_state::thayers_io_map);

	cop421_cpu_device &mcu(COP421(config, "mcu", XTAL(4'000'000)/2)); // COP421L-PCA/N
	mcu.set_config(COP400_CKI_DIVISOR_32, COP400_CKO_OSCILLATOR_OUTPUT, false);
	mcu.read_l().set(FUNC(thayers_state::cop_l_r));
	mcu.write_l().set(FUNC(thayers_state::cop_l_w));
	mcu.read_g().set(FUNC(thayers_state::cop_g_r));
	mcu.write_g().set(FUNC(thayers_state::cop_g_w));
	mcu.write_d().set(FUNC(thayers_state::cop_d_w));
	mcu.read_si().set(FUNC(thayers_state::kbdata_r));
	mcu.write_so().set(FUNC(thayers_state::kbclk_w));

	config.set_maximum_quantum(attotime::from_hz(262));

	// video hardware
	PIONEER_LDV1000HLE(config, m_player, 0);
	m_player->set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_SELF_RENDER);
	screen.set_raw(XTAL(14'318'181)*2, 910, 0, 704, 525, 44, 524);
	screen.set_screen_update(m_player, FUNC(pioneer_ldv1000hle_device::screen_update));

	// sound hardware
	SPEAKER(config, "speaker", 2).front();
	m_player->add_route(0, "speaker", 1.0, 0);
	m_player->add_route(1, "speaker", 1.0, 1);

	SSI263HLE(config, m_ssi, 860000);
	m_ssi->ar_callback().set(FUNC(thayers_state::ssi_data_request_w));
	m_ssi->add_route(0, "speaker", 1.0, 0);
	m_ssi->add_route(1, "speaker", 1.0, 0);
}


ROM_START( thayers )
	ROM_REGION( 0xe000, "maincpu", 0 )
	ROM_LOAD( "tq_u33.bin", 0x0000, 0x8000, CRC(82df5d89) SHA1(58dfd62bf8c5a55d1eba397d2c284e99a4685a3f) )
	ROM_LOAD( "tq_u1.bin",  0xc000, 0x2000, CRC(e8e7f566) SHA1(df7b83ef465c65446c8418bc6007447693b75021) )

	ROM_REGION( 0x400, "mcu", 0 )
	ROM_LOAD( "tq_cop.bin", 0x000, 0x400, CRC(6748e6b3) SHA1(5d7d1ecb57c1501ef6a2d9691eecc9970586606b) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "thayers", 0, SHA1(cfc517b1cc6f756ef76ae18e84092435b9af0511) )
ROM_END

ROM_START( thayersa )
	ROM_REGION( 0xe000, "maincpu", 0 )
	ROM_LOAD( "tq_u33.bin", 0x0000, 0x8000, CRC(82df5d89) SHA1(58dfd62bf8c5a55d1eba397d2c284e99a4685a3f) )
	ROM_LOAD( "tq_u1.bin",  0xc000, 0x2000, CRC(33817e25) SHA1(f9750da863dd57fe2f5b6e8fce9c6695dc5c9adc) )

	ROM_REGION( 0x400, "mcu", 0 )
	ROM_LOAD( "tq_cop.bin", 0x000, 0x400, CRC(6748e6b3) SHA1(5d7d1ecb57c1501ef6a2d9691eecc9970586606b) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "thayers", 0, SHA1(cfc517b1cc6f756ef76ae18e84092435b9af0511) )
ROM_END

} // anonymous namespace


//     YEAR  NAME      PARENT   MACHINE  INPUT    CLASS          INIT        MONITOR  COMPANY               FULLNAME                   FLAGS                                            LAYOUT
GAMEL( 1984, thayers,  0,       thayers, thayers, thayers_state, empty_init, ROT0,    "RDI Video Systems",  "Thayer's Quest (set 1)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_thayers)
GAMEL( 1984, thayersa, thayers, thayers, thayers, thayers_state, empty_init, ROT0,    "RDI Video Systems",  "Thayer's Quest (set 2)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_thayers)
