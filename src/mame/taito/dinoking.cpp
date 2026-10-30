// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen

/*

Taito mid-2000s medal hardware

Currently these games are dumped:
Hello Kitty Koi no Shugoshin Uranai - ハローキティ 恋の守護神占い (2003) - K11J0958C - F19 ROM code
Dinoking Kids - ダイノキングキッズ (2004) - K11J0985A - F39 ROM code
Dinoking Battle - ダイノキングバトル (2005) - K11J0998A - F54 ROM code

Main CPU: H8S-2394
Video chip: Axell AG-1 AX51102A (see video/axell_ag1.cpp)
Sound chip: OKI M9810B
XTALs: 20 MHz near CPU and sound chip, 25 MHz near video chip

Memory map (Dinoking Kids / Battle):
000000-1fffff program ROM
400000-4fffff work DRAM (refreshed by the H8S bus controller; the games run a
              destructive test of it on every cold boot)
600000-60ffff AG-1 sprite control SRAM (sheet / cell tables)
61f000-61f7ff AG-1 internal sprite control RAM (addresses 0x8000-0x87ff of the chip's list space)
61fc00-61fcff AG-1 registers
800000-800003 OKI M9810

The AG-1 frame interrupt is wired to IRQ0, its other interrupts (blitter / display
list transfer done) to IRQ1.  The games run the AG-1 in its 640x480 "VGA" mode
(25 MHz pixel clock, 800 x 525); hkuranai uses 640x468.

The settings and the bookkeeping live in the on-chip RAM of the H8S, which must be
kept alive by a backup battery (nothing else on the board is non-volatile and the
work RAM is wiped by the boot test): on a cold start without valid data the games
show "BACKUP DATA FAILED - PRESS TEST-SW" and want the method menu (operator
settings) to be saved once.  The H8S RAM is handled as NVRAM here, so this only
happens on the very first run.

Dinoking: the player buys medals with 100 yen coins (COIN TO MEDAL medals are paid
out by the hopper), inserts a medal to play and wins medals.  The main button stops
the attack roulette.

hkuranai: a fortune telling machine that prints the result on a thermal printer;
a coin buys one reading, up / down / left / right / decide select the reading and
enter the name and birth date.  The board differs: the AG-1 sits on an 8-bit bus,
there is a serial RTC and the printer hangs off SCI0.

TODO:
- sound: the OKI M9810 emulation is incomplete
- dinoking: hopper type (TAIPEI / JAPAN method setting, sensor polarity),
  the lamps on P2 are not hooked up to a layout
- hkuranai: DIP switch functions, lamp outputs (not found on the ports, maybe
  driven over a serial link), printer output (font ROM is dumped), paper sensor
- both: SCI1 / SCI2 are set up but nothing is ever sent (a link to a satellite
  board or a service tool?)
- verify AX51102 display list details (see video/axell_ag1.cpp)

*/


#include "emu.h"

#include "cpu/h8/h8s2357.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/watchdog.h"
#include "sound/okim9810.h"
#include "video/axell_ag1.h"

#include "dirtc.h"
#include "diserial.h"
#include "screen.h"
#include "speaker.h"

#include <iterator>


namespace {

/*
    Stand-in for the ALPS thermal printer of hkuranai.  It hangs off SCI0 of
    the H8S (38400 8N1, the BUSY line is read through port F bit 1) and
    speaks ESC/POS: the game initialises it with ESC @ and queries the four
    DLE EOT n real time status bytes before every print job, giving up with
    a "printer error or out of paper" screen when nothing answers.  Only the
    status queries are answered here (always "all fine"), everything else is
    swallowed - the printouts are the fortune telling results.

    TODO: dump of the printer's font ROM is available, so the output could
    be rendered properly some day
*/
class hkuranai_printer_device : public device_t, public device_serial_interface
{
public:
	hkuranai_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto tx_handler() { return m_tx_cb.bind(); }

	void rx_w(int state) { device_serial_interface::rx_w(state); }
	int busy_r() { return 0; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rcv_complete() override;
	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	void send(u8 data);

	devcb_write_line m_tx_cb;
	u8 m_rx_state;
	u8 m_tx_queue[8];
	u8 m_tx_head;
	u8 m_tx_tail;
};

DEFINE_DEVICE_TYPE_PRIVATE(HKURANAI_PRINTER, hkuranai_printer_device, hkuranai_printer_device, "hkuranai_printer", "Hello Kitty Uranai thermal printer")

hkuranai_printer_device::hkuranai_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HKURANAI_PRINTER, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_tx_cb(*this)
	, m_tx_queue{ }
{
}

void hkuranai_printer_device::device_start()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(38400);

	save_item(NAME(m_rx_state));
	save_item(NAME(m_tx_queue));
	save_item(NAME(m_tx_head));
	save_item(NAME(m_tx_tail));
}

void hkuranai_printer_device::device_reset()
{
	receive_register_reset();
	transmit_register_reset();
	m_tx_cb(1);
	m_rx_state = 0;
	m_tx_head = m_tx_tail = 0;
}

void hkuranai_printer_device::rcv_complete()
{
	receive_register_extract();
	const u8 data = get_received_char();

	// DLE EOT n: real time status request
	if (m_rx_state == 2)
	{
		if (data >= 1 && data <= 4)
			send(0x12); // no error, paper present, online
		m_rx_state = 0;
	}
	else if (m_rx_state == 1)
	{
		m_rx_state = (data == 0x04) ? 2 : (data == 0x10) ? 1 : 0;
	}
	else if (data == 0x10)
	{
		m_rx_state = 1;
	}
}

void hkuranai_printer_device::tra_callback()
{
	m_tx_cb(transmit_register_get_data_bit());
}

void hkuranai_printer_device::tra_complete()
{
	if (m_tx_head != m_tx_tail)
	{
		transmit_register_setup(m_tx_queue[m_tx_tail]);
		m_tx_tail = (m_tx_tail + 1) % std::size(m_tx_queue);
	}
}

void hkuranai_printer_device::send(u8 data)
{
	if (is_transmit_register_empty())
	{
		transmit_register_setup(data);
	}
	else
	{
		m_tx_queue[m_tx_head] = data;
		m_tx_head = (m_tx_head + 1) % std::size(m_tx_queue);
	}
}


/*
    Serial RTC, bit-banged on port 1: bit 2 = chip select (active high),
    bit 1 = clock (idle high, data sampled on the rising edge), bit 0 =
    bidirectional data (LSB first).  A transfer
    starts with a command byte whose low nibble is 0x3 for a write or 0xc
    for a read and whose high nibble is the first register; the address
    auto-increments.  Registers 0-6 are the BCD time (seconds, minutes,
    hours (24h), weekday, day, month, year), 7-15 are alarm / control
    registers which the game only ever clears.  The exact chip has not
    been identified.
*/
class hkuranai_rtc_device : public device_t, public device_rtc_interface
{
public:
	hkuranai_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void ce_w(int state);
	void clk_w(int state);
	void data_w(int state) { m_data_in = state ? 1 : 0; }
	int data_r();

protected:
	virtual void device_start() override ATTR_COLD;

	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	TIMER_CALLBACK_MEMBER(tick);
	void ce_falling();
	void clk_rising();
	void clk_falling();

	emu_timer *m_timer;

	u8 m_regs[16];
	u8 m_ce;
	u8 m_clk;
	u8 m_data_in;
	u8 m_shift;
	u8 m_bits;
	u8 m_addr;
	bool m_have_cmd;
	bool m_reading;
	bool m_started;
	u8 m_data_out;
};

DEFINE_DEVICE_TYPE_PRIVATE(HKURANAI_RTC, hkuranai_rtc_device, hkuranai_rtc_device, "hkuranai_rtc", "Hello Kitty Uranai serial RTC")

hkuranai_rtc_device::hkuranai_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HKURANAI_RTC, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, m_timer(nullptr)
	, m_regs{ }
	, m_ce(0)
	, m_clk(0)
	, m_data_in(0)
	, m_shift(0)
	, m_bits(0)
	, m_addr(0)
	, m_have_cmd(false)
	, m_reading(false)
	, m_started(false)
	, m_data_out(1)
{
}

void hkuranai_rtc_device::device_start()
{
	m_timer = timer_alloc(FUNC(hkuranai_rtc_device::tick), this);
	m_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	save_item(NAME(m_regs));
	save_item(NAME(m_ce));
	save_item(NAME(m_clk));
	save_item(NAME(m_data_in));
	save_item(NAME(m_shift));
	save_item(NAME(m_bits));
	save_item(NAME(m_addr));
	save_item(NAME(m_have_cmd));
	save_item(NAME(m_reading));
	save_item(NAME(m_started));
	save_item(NAME(m_data_out));
}

void hkuranai_rtc_device::ce_w(int state)
{
	const u8 ce = state ? 1 : 0;
	if (m_ce && !ce)
		ce_falling();
	m_ce = ce;
}

void hkuranai_rtc_device::clk_w(int state)
{
	const u8 clk = state ? 1 : 0;
	if (clk != m_clk && m_ce)
	{
		if (clk)
			clk_rising();
		else
			clk_falling();
	}
	m_clk = clk;
}

int hkuranai_rtc_device::data_r()
{
	return (m_reading && m_ce) ? m_data_out : 1;
}

void hkuranai_rtc_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_regs[0] = convert_to_bcd(second);
	m_regs[1] = convert_to_bcd(minute);
	m_regs[2] = convert_to_bcd(hour);
	m_regs[3] = day_of_week - 1;
	m_regs[4] = convert_to_bcd(day);
	m_regs[5] = convert_to_bcd(month);
	m_regs[6] = convert_to_bcd(year);
}

TIMER_CALLBACK_MEMBER(hkuranai_rtc_device::tick)
{
	advance_seconds();
}

void hkuranai_rtc_device::ce_falling()
{
	m_have_cmd = false;
	m_reading = false;
	m_started = false;
	m_bits = 0;
	m_shift = 0;
}

void hkuranai_rtc_device::clk_rising()
{
	// the clock idles high: the first rising edge after chip select just
	// establishes the idle level, data is transferred on the following ones
	if (!m_started)
		return;

	if (m_reading)
	{
		// the host samples the data line while the clock is high
		m_data_out = BIT(m_shift, 0);
		m_shift >>= 1;
		if (++m_bits == 8)
		{
			m_bits = 0;
			m_addr = (m_addr + 1) & 0x0f;
			m_shift = m_regs[m_addr];
		}
		return;
	}

	// data is clocked in LSB first
	m_shift = (m_shift >> 1) | (m_data_in << 7);
	if (++m_bits < 8)
		return;

	m_bits = 0;
	if (!m_have_cmd)
	{
		m_have_cmd = true;
		m_addr = m_shift >> 4;
		m_reading = (m_shift & 0x0f) == 0x0c;
		if (m_reading)
			m_shift = m_regs[m_addr];
		else if ((m_shift & 0x0f) != 0x03)
			logerror("unknown command %02x\n", m_shift);
	}
	else
	{
		m_regs[m_addr] = m_shift;
		if (m_addr < 7)
			set_time(false, bcd_to_integer(m_regs[6]), bcd_to_integer(m_regs[5]), bcd_to_integer(m_regs[4]), m_regs[3] + 1, bcd_to_integer(m_regs[2]), bcd_to_integer(m_regs[1]), bcd_to_integer(m_regs[0]));
		m_addr = (m_addr + 1) & 0x0f;
	}
}

void hkuranai_rtc_device::clk_falling()
{
	m_started = true;
}

// common hardware: H8S, AG-1, sound, external watchdog
class dinoking_base_state : public driver_device
{
public:
	dinoking_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ag1(*this, "ag1")
		, m_screen(*this, "screen")
		, m_watchdog(*this, "watchdog")
		, m_oki(*this, "oki")
	{
	}

protected:
	required_device<h8s2394_device> m_maincpu;
	required_device<ax51102_device> m_ag1;
	required_device<screen_device> m_screen;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<okim9810_device> m_oki;

	void base(machine_config &config) ATTR_COLD;
};


// Dinoking Kids / Battle: medal hopper, battery backed H8S RAM
class dinoking_state : public dinoking_base_state
{
public:
	dinoking_state(const machine_config &mconfig, device_type type, const char *tag)
		: dinoking_base_state(mconfig, type, tag)
		, m_hopper(*this, "hopper")
		, m_nvram(*this, "nvram")
		, m_selector(*this, "selector%u", 0U)
		, m_lamps(*this, "lamp%u", 3U)
	{
	}

	void dinoking(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<hopper_device> m_hopper;
	required_device<nvram_device> m_nvram;
	output_finder<2> m_selector;
	output_finder<4> m_lamps;

	u8 m_port6 = 0;

	void port1_w(u8 data);
	void port2_w(u8 data);
	void port6_w(u8 data);

	void ag1_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
};


// Hello Kitty Koi no Shugoshin Uranai: AG-1 on an 8-bit bus, serial RTC, printer
class hkuranai_state : public dinoking_base_state
{
public:
	hkuranai_state(const machine_config &mconfig, device_type type, const char *tag)
		: dinoking_base_state(mconfig, type, tag)
		, m_printer(*this, "printer")
		, m_rtc(*this, "rtc")
	{
	}

	void hkuranai(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<hkuranai_printer_device> m_printer;
	required_device<hkuranai_rtc_device> m_rtc;

	u8 m_portg = 0;

	void port1_w(u8 data);
	void port6_w(u8 data);
	void portg_w(u8 data);

	void hk_ag1_map(address_map &map) ATTR_COLD;
	void hk_mem_map(address_map &map) ATTR_COLD;
};


void dinoking_state::machine_start()
{
	dinoking_base_state::machine_start();

	// the H8S is kept alive by a backup battery: its on-chip RAM holds the
	// settings and the bookkeeping (the external RAM is DRAM and is wiped by
	// the test the games run on a cold boot)
	m_nvram->set_base(m_maincpu->space(AS_PROGRAM).get_write_ptr(0xff7c00), 0x8000);

	save_item(NAME(m_port6));
}

/*
    I/O (as labelled by the games' INPUT OUTPUT TEST):
      P1.0  medal selector (acceptor enable)
      P1.2  coin selector
      P2.0  coin counter (pulsed when a coin is accepted)
      P2.1  medal counter (pulsed for every medal paid out)
      P2.2  hopper motor
      P2.3  main button lamp (blinks in attract mode)
      P2.4-P2.6 more lamps
      P4.0  coin sensor (100 yen)
      P4.1  test switch
      P4.2  hopper sensor (a medal leaving the hopper)
      P4.3  select switch
      P4.4  enter switch
      P4.5  main button
      P6.2  watchdog kick (output)
      P6.3  medal sensor (a medal inserted by the player)
    The player buys medals with coins (COIN TO MEDAL in the method menu,
    paid out by the hopper), inserts them to play and wins more medals.
*/
void dinoking_state::port1_w(u8 data)
{
	m_selector[0] = BIT(data, 0);
	m_selector[1] = BIT(data, 2);
}

void dinoking_state::port2_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	m_hopper->motor_w(BIT(data, 2));
	for (u32 i = 0; i < 4; i++)
		m_lamps[i] = BIT(data, 3 + i);
}

// bit 2 is toggled on every main loop iteration to kick an external watchdog, the
// games rely on it to reboot after the test mode settings have been saved
void dinoking_state::port6_w(u8 data)
{
	if (BIT(data ^ m_port6, 2))
		m_watchdog->watchdog_reset();
	m_port6 = data;
}

void dinoking_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x400000, 0x4fffff).ram(); // DRAM, refreshed by the H8S bus controller

	map(0x600000, 0x60ffff).ram().share("ag1ram");
	map(0x61f000, 0x61f7ff).rw(m_ag1, FUNC(ax51102_device::sprram_r), FUNC(ax51102_device::sprram_w));
	map(0x61fc00, 0x61fcff).rw(m_ag1, FUNC(ax51102_device::read), FUNC(ax51102_device::write));

	map(0x800001, 0x800001).w(m_oki, FUNC(okim9810_device::tmp_register_w));
	map(0x800000, 0x800000).w(m_oki, FUNC(okim9810_device::write));
	map(0x800002, 0x800002).r(m_oki, FUNC(okim9810_device::read));
}

// AG-1 sprite control memory: external SRAM
void dinoking_state::ag1_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("ag1ram");
}


/*
    hkuranai wires the AG-1 to the upper byte lane of the H8S bus with the
    chip's address lines shifted up by one, so its whole 17 bit host space
    appears at 0x400000-0x43ffff with one byte per word:
      400000-41ffff sprite control SRAM (chip 0x00000-0x0ffff)
      43e000-43efff built-in sprite control RAM (chip 0x1f000-0x1f7ff)
      43f800-43f9ff registers (chip 0x1fc00-0x1fcff)
    The game keeps the base address in a RAM variable and only ever uses
    the on-chip RAM of the H8S for its own data.
*/
void hkuranai_state::hk_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);

	map(0x400000, 0x41ffff).rw(m_ag1, FUNC(ax51102_device::list_r), FUNC(ax51102_device::list_w)).umask16(0xff00);
	map(0x43e000, 0x43efff).rw(m_ag1, FUNC(ax51102_device::sprram_r), FUNC(ax51102_device::sprram_w)).umask16(0xff00);
	map(0x43f800, 0x43f9ff).rw(m_ag1, FUNC(ax51102_device::read), FUNC(ax51102_device::write)).umask16(0xff00);

	map(0x600001, 0x600001).w(m_oki, FUNC(okim9810_device::tmp_register_w));
	map(0x600000, 0x600000).w(m_oki, FUNC(okim9810_device::write));
	map(0x600002, 0x600002).r(m_oki, FUNC(okim9810_device::read));
}

void hkuranai_state::hk_ag1_map(address_map &map)
{
	map(0x0000, 0xffff).ram();
}



void hkuranai_state::machine_start()
{
	dinoking_base_state::machine_start();

	save_item(NAME(m_portg));
}

void hkuranai_state::port6_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));
}

// the external watchdog is kicked from port G bit 0 on this board
void hkuranai_state::portg_w(u8 data)
{
	if (BIT(data ^ m_portg, 0))
		m_watchdog->watchdog_reset();
	m_portg = data;
}

// bit 2 = RTC chip select, bit 1 = RTC clock, bit 0 = RTC data
void hkuranai_state::port1_w(u8 data)
{
	m_rtc->data_w(BIT(data, 0));
	m_rtc->ce_w(BIT(data, 2));
	m_rtc->clk_w(BIT(data, 1));
}


/*
    The games assemble a 17 bit active low input word from the H8S ports:
    bits 0-4 = P4 bits 0-4, bits 5-7 = PA bits 5-7, bit 8 = PF bit 7,
    bits 9-12 = P1 bits 4-7, bits 13-15 = P4 bits 5-7, bit 16 = P6 bit 3.
    Dinoking only uses the ones listed in its I/O test.
*/
static INPUT_PORTS_START( dinoking )
	PORT_START("P1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // 100 Yen
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // hopper sensor, low while a medal passes it
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Select")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Enter")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) // Medal
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PF")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*
    hkuranai (labels from its I/O TEST): P4 = coin, test, service, up, down,
    left, right, decide; a 7 position DIP switch bank is read through P1
    bits 3-5, P6 bit 0 and PA bits 5-7 (P1 bits 0-2 drive the RTC, P6 bit 2
    is the coin counter output).  All switches are off on the real machine
    as far as is known; their meaning has not been investigated.
*/
static INPUT_PORTS_START( hkuranai )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("rtc", FUNC(hkuranai_rtc_device::data_r))
	PORT_BIT( 0x06, IP_ACTIVE_LOW, IPT_UNUSED ) // RTC clock / chip select (outputs)
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:8" )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P6")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:4" )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:3" )
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PF")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("printer", FUNC(hkuranai_printer_device::busy_r))
	PORT_BIT( 0x7c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) // printer connected / paper present
INPUT_PORTS_END


void dinoking_base_state::base(machine_config &config)
{
	H8S2394(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->read_port4().set_ioport("P4");
	m_maincpu->read_port6().set_ioport("P6");
	m_maincpu->read_porta().set_ioport("PA");

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_seconds(3));

	SCREEN(config, m_screen);
	m_screen->set_raw(25_MHz_XTAL, 800, 0, 640, 525, 0, 480);
	m_screen->set_screen_update(m_ag1, FUNC(ax51102_device::screen_update));

	AX51102(config, m_ag1, 25_MHz_XTAL);
	m_ag1->set_screen(m_screen);
	m_ag1->set_cgrom_tag("gfx");

	SPEAKER(config, "speaker", 2).front();

	OKIM9810(config, m_oki, 4'096'000); // TODO: clock not verified, the PCB only has 20 MHz and 25 MHz crystals
	m_oki->add_route(0, "speaker", 0.80, 0);
	m_oki->add_route(1, "speaker", 0.80, 1);
}


void dinoking_state::dinoking(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &dinoking_state::mem_map);
	m_maincpu->read_port1().set_ioport("P1");
	m_maincpu->write_port1().set(FUNC(dinoking_state::port1_w));
	m_maincpu->write_port2().set(FUNC(dinoking_state::port2_w));
	m_maincpu->write_port6().set(FUNC(dinoking_state::port6_w));
	m_maincpu->read_portf().set_ioport("PF");

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	// medal hopper, roughly six medals a second
	HOPPER(config, m_hopper, attotime::from_msec(80));

	m_ag1->set_addrmap(0, &dinoking_state::ag1_map);
	// the frame interrupt has its own CPU input, the other interrupt outputs are tied together
	input_merger_device &ag1irq1(INPUT_MERGER_ANY_HIGH(config, "ag1irq1"));
	ag1irq1.output_handler().set_inputline(m_maincpu, 1);
	m_ag1->irq_callback<6>().set_inputline(m_maincpu, 0);
	m_ag1->irq_callback<1>().set(ag1irq1, FUNC(input_merger_device::in_w<1>));
	m_ag1->irq_callback<2>().set(ag1irq1, FUNC(input_merger_device::in_w<2>)); // blitter done
	m_ag1->irq_callback<3>().set(ag1irq1, FUNC(input_merger_device::in_w<3>));
	m_ag1->irq_callback<4>().set(ag1irq1, FUNC(input_merger_device::in_w<4>)); // display list transferred
	m_ag1->irq_callback<5>().set(ag1irq1, FUNC(input_merger_device::in_w<5>)); // second frame interrupt
}


void hkuranai_state::hkuranai(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &hkuranai_state::hk_mem_map);
	m_maincpu->read_port1().set_ioport("P1");
	m_maincpu->write_port1().set(FUNC(hkuranai_state::port1_w));
	m_maincpu->write_port6().set(FUNC(hkuranai_state::port6_w));
	m_maincpu->read_portf().set_ioport("PF");
	m_maincpu->write_portg().set(FUNC(hkuranai_state::portg_w));
	// ALPS thermal printer on SCI0
	m_maincpu->write_sci_tx<0>().set(m_printer, FUNC(hkuranai_printer_device::rx_w));

	// no hopper (the prizes are printouts); the MCU RAM is not battery backed (TODO: verify)

	HKURANAI_PRINTER(config, m_printer, 0);
	m_printer->tx_handler().set(m_maincpu, FUNC(h8s2394_device::sci_rx_w<0>));

	// serial RTC on port 1
	HKURANAI_RTC(config, m_rtc, 0);

	m_ag1->set_addrmap(0, &hkuranai_state::hk_ag1_map);

	// the four interrupt outputs this game uses go to separate CPU inputs
	m_ag1->irq_callback<1>().set_inputline(m_maincpu, 3); // sprite RAM write collision (never generated)
	m_ag1->irq_callback<2>().set_inputline(m_maincpu, 2); // blitter done
	m_ag1->irq_callback<4>().set_inputline(m_maincpu, 1); // display list transferred
	m_ag1->irq_callback<5>().set_inputline(m_maincpu, 0); // frame (second frame interrupt output)
}


ROM_START( hkuranai ) // slightly different PCB, but mostly same components. Adds a printer.
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "f19-04.ic28", 0x000000, 0x200000, CRC(2e8e409f) SHA1(5845c4e5ea1d95d57f2a31bc590b98e09d74c5b4) ) // silkscreened PRG M27C160-120F1 on PCB

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD( "f19-01.ic18", 0x000000, 0x200000, CRC(260213f9) SHA1(128b21a7288d5386e286358d0bc57c4c6494f56a) ) // silkscreened CG-ROM1 MSM27C1602CZ on PCB
	 // empty socket marked as CG-ROM2 MSM27C1602CZ on PCB at ic19

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "f19-03.ic20", 0x000000, 0x200000, CRC(5e844985) SHA1(6e31cab29d7e6754e345dac94551b59a700356db) ) // silkscreened PCM MSM27C1602CZ on PCB

	ROM_REGION( 0x100000, "printer_font", 0 ) // on ALPS PTCBL11/21-MAIN PCB
	ROM_LOAD( "165-02.u1", 0x000000, 0x100000, CRC(5a9e98ac) SHA1(fc7dd89abebbc32cd833f3abd3bbb10c5f5b790e) )
ROM_END

ROM_START( dkkids )
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "f39-03.ic3", 0x000000, 0x200000, CRC(061ea9e6) SHA1(9a6957a24e228aa1f44bf91c635894ecd4871623) ) // ST-M27C160-100

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "f39-01.ic30", 0x000000, 0x1000000, CRC(1eb9efa6) SHA1(08aa33cb9a5e1e57702d23d7692ad9a289b9a4ed) ) // ST-M59PW1282-100MIT

	ROM_REGION( 0x400000, "oki", 0 )
	ROM_LOAD( "f39-02.ic27", 0x000000, 0x400000, CRC(f0a38e0e) SHA1(30ee275d4cce5bb50141e807dc64d6509eaf4937) ) // MBM29F033C-90PTN
ROM_END

ROM_START( dkbattle )
	ROM_REGION16_BE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "f54-03.ic3", 0x000000, 0x200000, CRC(eeb61085) SHA1(33304c1fd068dc4fd0bacfd4e47e699aa4ae0b82) ) // ST-M27C160-100

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_LOAD( "f54-01.ic30", 0x000000, 0x1000000, CRC(f2006d02) SHA1(3ba1832eb3440b175014a4d00b56e24a9d8d1994) ) // ST-M59PW1282-100MIT

	ROM_REGION( 0x400000, "oki", 0 )
	ROM_LOAD( "f54-02.ic27", 0x000000, 0x400000, CRC(bd8e10e9) SHA1(a86fde5860501b06f63bafbc02ebc6160c682b1e) ) // MBM29F033C-90PTN
ROM_END

} // anonymous namespace


GAME( 2003, hkuranai, 0, hkuranai, hkuranai, hkuranai_state, empty_init, ROT0, "Taito", "Hello Kitty Koi no Shugoshin Uranai", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NODEVICE_PRINTER )
GAME( 2004, dkkids,   0, dinoking, dinoking, dinoking_state, empty_init, ROT0, "Taito", "Dinoking Kids",                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 2005, dkbattle, 0, dinoking, dinoking, dinoking_state, empty_init, ROT0, "Taito", "Dinoking Battle",                     MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
