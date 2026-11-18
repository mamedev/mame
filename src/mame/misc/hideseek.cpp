// license:BSD-3-Clause
// copyright-holders: Peter Wilhelmsen

/*
Hide & Seek

Augus Gaming System (AGS), distributed by Augus with software by Appli.

Guru:
It's called Hide and Seek. I don't know the manufacturer.
I dumped the ROMs and with a byte swap on U10 I see plain text with words like BET, PAYOUT, HIT etc so it looks like some kind of gambling game.
Main program ROMs are 16M flashROMs. The rest are 128M flashROMs. ROMs U8, U9 and U11 are empty.
PCB number is BO-023C
The CPU is a SH2 HD64F7045F28. Connected xtal is 7.3728MHz.
Main RAM is 2x NEC D431000 (128k x8-bit SRAM).
There are 3 chips NEC DX-102 (used on seta2/ssv).
The main gfx chip is a new type not previously used in MAME I think. Number is AG-2 AX51201. Connected OSC is 25MHz
Video RAM is 2x Etrontech EM639165TS-7G (8M x16-bit SDRAM)
Sound is OKI M9810. 409G649 XTAL.
Other stuff: NEC D4992 (RTC) and xtal possibly 32.768kHz, 3V coin battery, 93L46 EEPROM, 1x 8-position DIPSW, 2x Lattice iM4A3-64 CPLDs; one on the main board and one on the ROM board.

The JanJanJango! advertisement in Amusement Journal, September 2007, p. 77,
identifies that game as the seventh AGS release and illustrates the earlier
library: Shijin Retsuden (四神列伝), Raikazan (雷火山), Crystal Magic,
Hide and Seek, Final Emergency and Ultramagic. Their individual release order
is not established. The pictured JanJanJango! cabinet has ten buttons (six
in a center row and two at each side). The input assignments below follow
the program's own I/O test.

Initialize blank battery-backed SRAM by holding Reset Key and Meter Key
during a machine reset, then releasing both.

TODO:
- Coin passage and hopper pulse timings are inferred; electrical sequences
  and meter feedback follow the ROM's checks. DIP meanings need further work.
- CPU PLL multiplier needs measurement.
- SH7045F flash programming/erase and some shared CPU peripherals are incomplete.
- RTC 12-hour mode and interval outputs are not implemented; the game's
  setters use 24-hour time, clock stop/start and the leap-year counter.
- AG-2 effects and video timing are still provisional.
*/



#include "emu.h"

#include "cpu/sh/sh7042.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/upd4992.h"
#include "sound/okim9810.h"
#include "video/axell_ag2.h"

#include "screen.h"
#include "speaker.h"


namespace {

class hideseek_state : public driver_device
{
public:
	hideseek_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_eeprom(*this, "eeprom")
		, m_hopper(*this, "hopper")
		, m_in(*this, "IN%u", 0U)
		, m_system(*this, "SYSTEM%u", 0U)
		, m_battery(*this, "BATTERY")
		, m_lamps(*this, "lamp%u", 0U)
	{
	}

	void hideseek(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	ioport_value coin_sensors_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<sh7045f_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<hopper_device> m_hopper;

	required_ioport_array<3> m_in;
	required_ioport_array<2> m_system;
	required_ioport m_battery;
	output_finder<10> m_lamps;

	emu_timer *m_coin_timer = nullptr;
	u8 m_output_latch = 0;
	u8 m_coin_phase = 0;
	u8 m_coin_sensors = 0xc0;

	void outputs_w(u8 data);
	void portd_w(u32 data);
	void porte_w(u16 data);
	TIMER_CALLBACK_MEMBER(coin_tick);
	u16 portf_r();

	void mem_map(address_map &map) ATTR_COLD;
};

void hideseek_state::machine_start()
{
	m_coin_timer = timer_alloc(FUNC(hideseek_state::coin_tick), this);

	save_item(NAME(m_output_latch));
	save_item(NAME(m_coin_phase));
	save_item(NAME(m_coin_sensors));
}

void hideseek_state::machine_reset()
{
	outputs_w(0);

	m_coin_phase = 0;
	m_coin_sensors = 0xc0;
	m_coin_timer->adjust(attotime::never);
}

INPUT_CHANGED_MEMBER(hideseek_state::coin_inserted)
{
	if (newval && !m_coin_phase && BIT(m_output_latch, 1))
	{
		// The acceptor has two overlapping optical sensors. The program at
		// 0x6620 checks their order and reports COIN IN REVERSE otherwise.
		m_coin_phase = 1;
		m_coin_sensors = 0x40;
		m_coin_timer->adjust(attotime::from_msec(20));
	}
}

TIMER_CALLBACK_MEMBER(hideseek_state::coin_tick)
{
	static constexpr u8 SEQUENCE[] = { 0xc0, 0x40, 0x00, 0x80 };
	m_coin_phase = (m_coin_phase + 1) & 3;
	m_coin_sensors = SEQUENCE[m_coin_phase];

	if (m_coin_phase)
		m_coin_timer->adjust(attotime::from_msec(20));
}

ioport_value hideseek_state::coin_sensors_r()
{
	return m_coin_sensors >> 6;
}

void hideseek_state::outputs_w(u8 data)
{
	// 0x61a0 controls the hopper; 0x6120 controls the coin acceptor enable.
	m_output_latch = data;

	m_hopper->motor_w(BIT(data, 0));

	machine().bookkeeping().coin_lockout_w(0, !BIT(data, 1));

	for (u32 i = 0; i != 5; ++i)
		machine().bookkeeping().coin_counter_w(i, BIT(data, 7 - i));
}

void hideseek_state::portd_w(u32 data)
{
	// Output-test table 0x40c82c: Hold 1-5, Start, 1 Bet, Repeat Bet.
	for (u32 i = 0; i != 8; ++i)
		m_lamps[i] = BIT(data, 24 + i);
}

void hideseek_state::porte_w(u16 data)
{
	// Internal ROM 0x9450/0x94b0: CS, clock and serial data respectively.
	m_eeprom->di_write(BIT(data, 15));
	m_eeprom->cs_write(BIT(data, 13));
	m_eeprom->clk_write(BIT(data, 14));

	m_lamps[8] = BIT(data, 5); // Payout / Collect
	m_lamps[9] = BIT(data, 7); // Max Bet
}

u16 hideseek_state::portf_r()
{
	// Internal ROM 0x9720 reads the EEPROM's dummy bit and 16 data bits here.
	return 0xffdb | (m_eeprom->do_read() << 2) | (m_battery->read() << 5);
}

void hideseek_state::mem_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom(); // On-chip ROM
	map(0x00200000, 0x0023ffff).ram().share("nvram"); // CS0, battery-backed SRAM
	map(0x00400000, 0x005fffff).rom().region("prg", 0); // CS1
	// CS2 - CS3
	map(0x00800000, 0x00800002).lr8(NAME([this] (offs_t offset) { return m_in[offset]->read(); }));
	map(0x00800100, 0x00800101).lr8(NAME([this] (offs_t offset) -> u8 {
		// The five meter drive lines have feedback contacts, checked at 0x6390.
		return offset ? ((m_system[1]->read() & 0x07) | (m_output_latch & 0xf8)) : m_system[0]->read();
	}));
	map(0x00800102, 0x00800102).w(FUNC(hideseek_state::outputs_w));

	map(0x00840000, 0x00840007).rw("rtc", FUNC(upd4992_device::read), FUNC(upd4992_device::write));

	// 0x8ef0 queues (command, data); 0x93c0 writes data at +1, then command at +0.
	map(0x00880000, 0x00880000).rw("oki", FUNC(okim9810_device::read_status), FUNC(okim9810_device::write_command));
	map(0x00880001, 0x00880001).w("oki", FUNC(okim9810_device::write_tmp_register));
	// Register pointers in on-chip code: 6e40, 733e, 8882, etc.
	map(0x00c00000, 0x00c1ffff).rw("ag2", FUNC(axell_ag2_device::host_r), FUNC(axell_ag2_device::host_w));

	map(0x01000000, 0x01ffffff).ram(); // DRAM
}



static INPUT_PORTS_START( hideseek )
	// Masks from the program's INPUT TEST table at 0x40c100.
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Max Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / Double Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Repeat Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("1 Bet")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Payout / Collect")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(hideseek_state::coin_sensors_r))

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )

	PORT_START("IN2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_DOOR ) PORT_NAME("Logic Door") PORT_CODE(KEYCODE_O) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_DOOR ) PORT_NAME("Main Door") PORT_CODE(KEYCODE_D) PORT_TOGGLE
	PORT_BIT( 0x6b, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))

	PORT_START("SYSTEM0") // 8-position DIP bank, read inverted by 0x5db0
	// 0x5cc0/0x6220 select the expected sensor level. The hopper itself is
	// wired active low; changing this switch does not rewire that hardware.
	PORT_DIPNAME(0x01, 0x01, "Hopper Sensor Polarity") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x01, "Active Low")
	PORT_DIPSETTING(0x00, "Active High")
	PORT_DIPNAME(0x02, 0x02, "Settings Door Check") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("SYSTEM1")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hideseek_state::coin_inserted), 0)

	PORT_START("BATTERY")
	PORT_CONFNAME(0x01, 0x01, "Backup Battery")
	PORT_CONFSETTING(0x01, "Good")
	PORT_CONFSETTING(0x00, "Low")
INPUT_PORTS_END



void hideseek_state::hideseek(machine_config &config)
{
	// basic machine hardware
	SH7045F(config, m_maincpu, 7.3728_MHz_XTAL * 4); // PLL multiplier unverified
	m_maincpu->set_addrmap(AS_PROGRAM, &hideseek_state::mem_map);
	m_maincpu->write_portd().set(FUNC(hideseek_state::portd_w));
	m_maincpu->write_porte().set(FUNC(hideseek_state::porte_w));
	m_maincpu->read_portf().set(FUNC(hideseek_state::portf_r));
	// populated vectors:
	// 0x100 ~ 0x10c
	// 0x160
	// 0x210 ~ 0x21c
	// ROM vectors 64-67 call 7090/7150/70f0/71c0, acknowledging
	// AG-2 event bits 6/4/2/0 respectively.

	HOPPER(config, m_hopper, attotime::from_msec(100));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	EEPROM_93C46_16BIT(config, "eeprom");

	UPD4992(config, "rtc", 32.768_kHz_XTAL);

	// video hardware
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(128*8, 64*8);
	screen.set_visarea(0*8, 640-1, 0*8, 400-1);
	screen.set_screen_update("ag2", FUNC(axell_ag2_device::screen_update));

	axell_ag2_device &ag2(AXELL_AG2(config, "ag2", 25_MHz_XTAL, 0x2000000)); // 2x 8M x16 SDRAM
	ag2.set_screen("screen");
	ag2.event_cb<3>().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ag2.event_cb<2>().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	ag2.event_cb<1>().set_inputline(m_maincpu, INPUT_LINE_IRQ2);
	ag2.event_cb<0>().set_inputline(m_maincpu, INPUT_LINE_IRQ3);

	SPEAKER(config, "speaker", 2).front();

	okim9810_device &oki(OKIM9810(config, "oki", 4.096_MHz_XTAL));
	oki.add_route(0, "speaker", 1.00, 0);
	oki.add_route(1, "speaker", 1.00, 1);
}


ROM_START( hideseek )
	ROM_REGION( 0x40000, "maincpu", 0)
	ROM_LOAD( "hideseek_sh2_hd64f7045f28-vga.u6", 0x00000, 0x40000, CRC(a6d11053) SHA1(90b7a4238f398f4831355aa2d31bc17595fab56d) ) // on chip ROM

	ROM_REGION32_BE( 0x400000, "prg", 0 ) // SH2 code
	ROM_LOAD16_WORD_SWAP( "29f160te.u10", 0x000000, 0x200000, CRC(44539f9b) SHA1(2e531455e5445e09e99494e47d96866e8ee07135) )
	ROM_LOAD16_WORD_SWAP( "29f160te.u11", 0x200000, 0x200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) ) // empty!

	ROM_REGION( 0x4000000, "ag2", 0 )
	ROM_LOAD16_WORD_SWAP( "s29gl128n.u6", 0x0000000, 0x1000000,  CRC(2d6632f3) SHA1(e8d91bcc6975f5c07b438d29e8a23d403c7e52aa) )
	ROM_LOAD16_WORD_SWAP( "s29gl128n.u7", 0x1000000, 0x1000000,  CRC(9af057bf) SHA1(e905d0dfa5b866d7317adafe03fcdfadd1e44d78) )
	ROM_LOAD16_WORD_SWAP( "s29gl128n.u8", 0x2000000, 0x1000000,  CRC(a47ca14a) SHA1(3b4417fc421cee30a9ad0fd9319220a8dae32da2) ) // empty!
	ROM_LOAD16_WORD_SWAP( "s29gl128n.u9", 0x3000000, 0x1000000,  CRC(a47ca14a) SHA1(3b4417fc421cee30a9ad0fd9319220a8dae32da2) ) // empty!

	ROM_REGION( 0x1000000, "oki", 0 )
	ROM_LOAD( "s29gl128n.u4", 0x000000, 0x1000000, CRC(b8eb7cdb) SHA1(a90c70058e50f3369a6e517e0a534b9ef1e0087c) )

	ROM_REGION( 0x40000, "nvram", 0 ) // pre-initialized
	ROM_LOAD( "nvram", 0x00000, 0x40000, CRC(3e4c1ca5) SHA1(a1ec203e25ae27e1abcfaab2de01fb4f281085b6) )
ROM_END

} // anonymous namespace


GAME( 200?, hideseek, 0, hideseek, hideseek, hideseek_state, empty_init, ROT0, "Appli", "Hide & Seek", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
