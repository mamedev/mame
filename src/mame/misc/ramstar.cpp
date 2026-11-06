// license:BSD-3-Clause
// copyright-holders:R. Belmont

/*

Ram Star Video Pulltab (1991)
The program identifies itself as "RAM STAR VIDEO PULLTAB VERSION 80.1".
It comprises at least 2 PCBs, which don't plug in one another.
So very probably missing at least a backplane, if not more PCBs.

RAM STAR VIDEO GAME PCB4201C
W65C816PL CPU
MAX691CPE supervisor
X24C04 serial E2PROM
P5C032 CPLD
10 MHz XTAL
M62X42B RTC (32.768KHz built-in XTAL)
P5164SL SRAM
8-DIP bank
AY-3-8912 sound chip
2x 27C512 ROM
R65C52P2 DACIA
R65C22P3 VIA
3.6864 MHz XTAL
NE5152BN TRIPLE 4-Bit RGB D/A CONVERTER
HM62256LP-10 video RAM
W65C29P IOD
W65C90P EBC

RamStar GAME MODULE CONTROL PCB4204A
SC80C31BCCN40 ROMless MCU
27C640 ROM
6 MHz XTAL
6x card slots

The carts' PCB is RamStar GAME MODULE PCB4203A.
It only contains a 27C512 or 27C1024 and a X24C04.

Main board memory map, worked out from the program:
00:0000-00:1FFF  battery backed RAM
00:8000          switch matrix returns (bits 0-5) and printer status (bits 6-7)
00:8200-00:820F  VIA: PA0-PA3 switch matrix row select, PA5 unknown output, PB2-PB7 button lamps,
                 CA2 pulsed as a watchdog strobe, CB1 vblank, T2 paces the switch matrix scan
00:8400-00:8407  DACIA: channel 1 = game module control board, channel 2 = unknown (19200 baud)
00:8600-00:8601  line counter (read), line compare interrupt (NMI) control (write)
00:8800-00:8801  printer data port and its direction register
00:8A00-00:8A01  AY-3-8912, port A = DIP switches
00:8C00-00:8C01  control port and its direction register, bits described in port_w()
00:8E00-00:8E0F  RTC
00:9000-00:FFFF  upper part of the program ROM
01:0000-01:7FFF  256x256 4bpp framebuffer
01:8000-01:802F  palette
04:0000-04:FFFF  graphics ROM
07:0000-07:FFFF  program ROM

The game module control board holds the pulltab deals. Its 80C31 answers commands from the main board over
the serial link, reading the cartridge ROMs and keeping each deal's progress in the cartridge's X24C04.

TODO:
- neither the main board X24C04 nor any cartridge X24C04 is dumped; their contents are reconstructed
- CPU, VIA and AY clocks and the video timing are unverified
- $8C00 bits 0 and 3, VIA PA5, switch matrix row 2, the DIP switches and DACIA channel 2 are unidentified
*/


#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6502/w65816.h"
#include "cpu/mcs51/i80c51.h"
#include "machine/6522via.h"
#include "machine/i2cmem.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "machine/output_latch.h"
#include "machine/r65c52.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


// X24C04 on a game module cartridge, holding the progress of the deal; its contents travel with the cartridge
class ramstar_deal_eeprom_device : public i2cmem_device
{
public:
	ramstar_deal_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void load(generic_slot_device &cart);
	void save(generic_slot_device &cart);

protected:
	virtual void nvram_default() override { }
	virtual bool nvram_read(util::read_stream &file) override { return true; }
	virtual bool nvram_write(util::write_stream &file) override { return true; }
	virtual bool nvram_can_write() const override { return false; }
};

DEFINE_DEVICE_TYPE(RAMSTAR_DEAL_EEPROM, ramstar_deal_eeprom_device, "ramstar_deal_eeprom", "Ram Star game module X24C04")

ramstar_deal_eeprom_device::ramstar_deal_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i2cmem_device(mconfig, RAMSTAR_DEAL_EEPROM, tag, owner, clock, 0x100, 8, 0x200)
{
}

void ramstar_deal_eeprom_device::load(generic_slot_device &cart)
{
	// Not dumped. Reconstruct an unplayed deal from the ROM header, as the game module control board
	// firmware expects it.
	std::vector<uint8_t> fresh(m_data_size, 0xff);
	const uint8_t *const rom = cart.get_rom_base();
	if (rom && (cart.get_rom_size() >= 0x100))
	{
		std::copy_n(&rom[0x0c], 8, &fresh[0x00]);
		std::fill_n(&fresh[0x10], 0x80, 0x00);
		fresh[0x50] = 0x94;
		std::copy_n(&rom[0x91], 4, &fresh[0x60]);
	}
	cart.battery_load(&m_data[0], m_data_size, fresh.data());
}

void ramstar_deal_eeprom_device::save(generic_slot_device &cart)
{
	cart.battery_save(&m_data[0], m_data_size);
}


namespace {

class ramstar_state : public driver_device
{
public:
	ramstar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cartcpu(*this, "cartcpu"),
		m_dacia(*this, "dacia"),
		m_eeprom(*this, "i2cmem"),
		m_centronics(*this, "centronics"),
		m_cent_data_out(*this, "cent_data_out"),
		m_carts(*this, "cartslot%u", 1U),
		m_deal_eeproms(*this, "cart%u_eeprom", 1U),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram"),
		m_inputs(*this, "IN%u", 0U),
		m_raster_timer(nullptr),
		m_input_select(0xff),
		m_cent_busy(0),
		m_cent_fault(1),
		m_cart_rxd(1),
		m_cart_p1(0xff),
		m_via_irq(0),
		m_raster_line(0),
		m_dacia_irq{ 0, 0 },
		m_port_data{ 0, 0 },
		m_port_ddr{ 0, 0 },
		m_palram{}
	{ }

	void ramstar(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<i80c31_device> m_cartcpu;
	required_device<r65c52_device> m_dacia;
	required_device<i2cmem_device> m_eeprom;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device_array<generic_slot_device, 6> m_carts;
	required_device_array<ramstar_deal_eeprom_device, 6> m_deal_eeproms;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_vram;
	required_ioport_array<4> m_inputs;

	emu_timer *m_raster_timer;

	uint8_t m_input_select;
	uint8_t m_cent_busy;
	uint8_t m_cent_fault;
	uint8_t m_cart_rxd;
	uint8_t m_cart_p1;
	uint8_t m_via_irq;
	uint8_t m_raster_line;
	uint8_t m_dacia_irq[2];
	uint8_t m_port_data[2];
	uint8_t m_port_ddr[2];
	uint8_t m_palram[0x30];

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void update_irq();
	template <unsigned N> void dacia_irq_w(int state);

	uint8_t inputs_r();
	uint8_t vpos_r();
	void raster_ack_w(uint8_t data);
	void raster_line_w(uint8_t data);
	TIMER_CALLBACK_MEMBER(raster_hit);
	template <unsigned N> uint8_t port_r(offs_t offset);
	template <unsigned N> void port_w(offs_t offset, uint8_t data);
	void palette_w(offs_t offset, uint8_t data);

	template <unsigned N> DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	template <unsigned N> void cart_unload(device_image_interface &image);
	uint8_t cart_r(offs_t offset);
	uint8_t cart_p1_r();
	void cart_p1_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void cart_program_map(address_map &map) ATTR_COLD;
	void cart_data_map(address_map &map) ATTR_COLD;
};


void ramstar_state::machine_start()
{
	m_raster_timer = timer_alloc(FUNC(ramstar_state::raster_hit), this);

	save_item(NAME(m_input_select));
	save_item(NAME(m_cent_busy));
	save_item(NAME(m_cent_fault));
	save_item(NAME(m_cart_rxd));
	save_item(NAME(m_cart_p1));
	save_item(NAME(m_via_irq));
	save_item(NAME(m_raster_line));
	save_item(NAME(m_dacia_irq));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_ddr));
	save_item(NAME(m_palram));
}

void ramstar_state::machine_reset()
{
	// TODO: the mcs51 core drives TXD low from its first reset until it transmits; the real pin idles high
	m_dacia->write_rxd1(1);

	m_raster_timer->adjust(attotime::never);
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	std::fill(std::begin(m_port_ddr), std::end(m_port_ddr), 0);
	port_w<0>(0, m_port_data[0]);
	port_w<1>(0, m_port_data[1]);
}


void ramstar_state::update_irq()
{
	m_maincpu->set_input_line(w65816_device::IRQ_LINE, (m_via_irq || m_dacia_irq[0] || m_dacia_irq[1]) ? ASSERT_LINE : CLEAR_LINE);
}

template <unsigned N>
void ramstar_state::dacia_irq_w(int state)
{
	m_dacia_irq[N] = state;
	update_irq();
}


// 4x6 switch matrix, rows selected by VIA PA0-PA3 (active low); bit 6 = printer not faulted, bit 7 = printer not busy
uint8_t ramstar_state::inputs_r()
{
	uint8_t data = 0x3f | (m_cent_fault ? 0x40 : 0x00) | (m_cent_busy ? 0x00 : 0x80);
	for (unsigned i = 0; i < 4; i++)
	{
		if (!BIT(m_input_select, i))
		{
			data &= m_inputs[i]->read();
		}
	}

	return data;
}

// the program waits for line $EE or later before touching the palette
uint8_t ramstar_state::vpos_r()
{
	return m_screen->vpos();
}

// Line compare interrupt, used to change colours partway down the screen: writing $8601 sets the line, the NMI
// handler writes $8600 before reprogramming it, and $8C00 bit 1 is held high while the list is restarted.
void ramstar_state::raster_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void ramstar_state::raster_line_w(uint8_t data)
{
	m_raster_line = data;
	m_raster_timer->adjust(m_screen->time_until_pos(data));
}

TIMER_CALLBACK_MEMBER(ramstar_state::raster_hit)
{
	if (!BIT(m_port_data[1] | ~m_port_ddr[1], 1))
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
	m_raster_timer->adjust(m_screen->time_until_pos(m_raster_line));
}


// two 8-bit I/O ports at $8800 and $8C00, each a data register followed by a direction register
template <unsigned N>
uint8_t ramstar_state::port_r(offs_t offset)
{
	if (offset)
	{
		return m_port_ddr[N];
	}

	uint8_t in = 0xff;
	if (N == 1)
	{
		in = (m_eeprom->read_sda() ? 0xff : 0xbf) & (BIT(m_port_data[1] | ~m_port_ddr[1], 5) ? 0xff : 0xbf);
	}

	return (m_port_data[N] & m_port_ddr[N]) | (in & ~m_port_ddr[N]);
}

template <unsigned N>
void ramstar_state::port_w(offs_t offset, uint8_t data)
{
	if (offset)
	{
		m_port_ddr[N] = data;
	}
	else
	{
		m_port_data[N] = data;
	}

	// undriven lines are pulled up
	const uint8_t out = m_port_data[N] | ~m_port_ddr[N];
	if (N == 0)
	{
		m_cent_data_out->write(out);
	}
	else
	{
		// bit 0 = unknown, bit 1 = line compare interrupt disable, bit 2 = game module control board reset (active low)
		// bit 3 = unknown (pulsed low for 16 frames after printing), bit 4 = printer /STROBE
		// bit 5 = X24C04 SDA out, bit 6 = SDA in, bit 7 = SCL
		m_centronics->write_strobe(BIT(out, 4));
		m_cartcpu->set_input_line(INPUT_LINE_RESET, BIT(out, 2) ? CLEAR_LINE : ASSERT_LINE);
		m_eeprom->write_sda(BIT(out, 5));
		m_eeprom->write_scl(BIT(out, 7));
	}
}


// 16 colours, stored as 16 red nibbles, then 16 blue, then 16 green, feeding the NE5152 DACs
// (the order is confirmed by the background colour setup menu's names and values at $07:E5E5)
void ramstar_state::palette_w(offs_t offset, uint8_t data)
{
	m_screen->update_partial(m_screen->vpos());
	m_palram[offset] = data & 0x0f;

	const unsigned entry = offset & 0x0f;
	m_palette->set_pen_color(entry, pal4bit(m_palram[entry]), pal4bit(m_palram[entry | 0x20]), pal4bit(m_palram[entry | 0x10]));
}

// 256x256 4bpp framebuffer, 128 bytes per line, left pixel in the high nibble
uint32_t ramstar_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *const pens = m_palette->pens();

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const uint8_t *const src = &m_vram[y << 7];
		uint32_t *const dst = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			dst[x] = pens[BIT(src[x >> 1], BIT(x, 0) ? 0 : 4, 4)];
		}
	}

	return 0;
}


// game module control board

template <unsigned N>
DEVICE_IMAGE_LOAD_MEMBER(ramstar_state::cart_load)
{
	const uint32_t size = m_carts[N]->common_get_size("rom");
	if ((size != 0x10000) && (size != 0x20000))
	{
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be 64K or 128K)");
	}

	m_carts[N]->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_carts[N]->common_load_rom(m_carts[N]->get_rom_base(), size, "rom");
	m_deal_eeproms[N]->load(*m_carts[N]);

	return std::make_pair(std::error_condition(), std::string());
}

template <unsigned N>
void ramstar_state::cart_unload(device_image_interface &image)
{
	m_deal_eeproms[N]->save(*m_carts[N]);
}

// P1.0-P1.2 select a slot, P1.7 is ROM A16
uint8_t ramstar_state::cart_r(offs_t offset)
{
	const unsigned slot = m_cart_p1 & 7;
	if ((slot >= 6) || !m_carts[slot]->exists())
	{
		return 0xff;
	}

	return m_carts[slot]->read_rom((BIT(m_cart_p1, 7) << 16 | offset) & (m_carts[slot]->get_rom_size() - 1));
}

uint8_t ramstar_state::cart_p1_r()
{
	const unsigned slot = m_cart_p1 & 7;
	const int sda = (slot < 6) ? m_deal_eeproms[slot]->read_sda() : 1;
	return sda ? 0xff : 0xdf;
}

// P1.3 = activity LED, P1.4 = X24C04 SDA out, P1.5 = SDA in, P1.6 = SCL
void ramstar_state::cart_p1_w(uint8_t data)
{
	m_cart_p1 = data;

	const unsigned slot = data & 7;
	for (unsigned i = 0; i < 6; i++)
	{
		m_deal_eeproms[i]->write_sda((i == slot) ? BIT(data, 4) : 1);
		m_deal_eeproms[i]->write_scl((i == slot) ? BIT(data, 6) : 1);
	}
}


void ramstar_state::main_map(address_map &map)
{
	map(0x000000, 0x001fff).ram().share("nvram"); // P5164SL, battery backed through the MAX691
	map(0x008000, 0x008000).r(FUNC(ramstar_state::inputs_r));
	map(0x008200, 0x00820f).m("via", FUNC(via6522_device::map));
	map(0x008400, 0x008407).m(m_dacia, FUNC(r65c52_device::map));
	map(0x008600, 0x008600).w(FUNC(ramstar_state::raster_ack_w));
	map(0x008601, 0x008601).rw(FUNC(ramstar_state::vpos_r), FUNC(ramstar_state::raster_line_w));
	map(0x008800, 0x008801).rw(FUNC(ramstar_state::port_r<0>), FUNC(ramstar_state::port_w<0>));
	map(0x008a00, 0x008a01).rw("ay", FUNC(ay8912_device::data_r), FUNC(ay8912_device::address_data_w));
	map(0x008c00, 0x008c01).rw(FUNC(ramstar_state::port_r<1>), FUNC(ramstar_state::port_w<1>));
	map(0x008e00, 0x008e0f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0x009000, 0x00ffff).rom().region("maincpu", 0x9000);
	map(0x010000, 0x017fff).ram().share(m_vram); // HM62256, video RAM according to PCB silkscreen
	map(0x018000, 0x01802f).w(FUNC(ramstar_state::palette_w));
	map(0x040000, 0x04ffff).rom().region("gfx", 0);
	map(0x070000, 0x07ffff).rom().region("maincpu", 0);
}

void ramstar_state::cart_program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void ramstar_state::cart_data_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(ramstar_state::cart_r));
}


static INPUT_PORTS_START( ramstar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Audit Ticket")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Play")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Print Ticket")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_DOOR ) PORT_NAME("Logic Door") PORT_CODE(KEYCODE_L) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_DOOR ) PORT_NAME("Cash Door") PORT_CODE(KEYCODE_D) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Printer Paper Out") PORT_CODE(KEYCODE_P) PORT_TOGGLE
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // 6?
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	// coin and bill pulses must last 2-7 frames; 8 or more is reported as stuck
	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BILL1 ) PORT_IMPULSE(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Select")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Pop")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(4)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END


void ramstar_state::ramstar(machine_config &config)
{
	// basic machine hardware
	W65816(config, m_maincpu, 10_MHz_XTAL); // (absence of) divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &ramstar_state::main_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	I2C_X2404P(config, m_eeprom); // actually X24C04

	MSM6242(config, "rtc", 32.768_kHz_XTAL); // actually M62X42B

	via6522_device &via(R65C22(config, "via", 3.6864_MHz_XTAL / 2)); // divider not verified
	via.writepa_handler().set([this] (uint8_t data) { m_input_select = data; });
	via.irq_handler().set([this] (int state) { m_via_irq = state; update_irq(); });

	// channel 1 talks to the game module control board at 2400 baud, channel 2 is programmed for 19200 baud
	R65C52(config, m_dacia, 3.6864_MHz_XTAL);
	m_dacia->irq1_handler().set(FUNC(ramstar_state::dacia_irq_w<0>)).invert();
	m_dacia->irq2_handler().set(FUNC(ramstar_state::dacia_irq_w<1>)).invert();
	m_dacia->txd1_handler().set([this] (int state) { m_cart_rxd = state; });
	m_dacia->txd2_handler().set("rs232", FUNC(rs232_port_device::write_txd));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set([this] (int state) { m_cent_busy = state; });
	m_centronics->fault_handler().set([this] (int state) { m_cent_fault = state; });

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_dacia, FUNC(r65c52_device::write_rxd2));

	I80C31(config, m_cartcpu, 6_MHz_XTAL); // (absence of) divider not verified
	m_cartcpu->set_addrmap(AS_PROGRAM, &ramstar_state::cart_program_map);
	m_cartcpu->set_addrmap(AS_DATA, &ramstar_state::cart_data_map);
	m_cartcpu->port_in_cb<1>().set(FUNC(ramstar_state::cart_p1_r));
	m_cartcpu->port_out_cb<1>().set(FUNC(ramstar_state::cart_p1_w));
	m_cartcpu->port_in_cb<3>().set([this] () { return 0xfe | m_cart_rxd; });
	m_cartcpu->port_out_cb<3>().set([this] (uint8_t data) { m_dacia->write_rxd1(BIT(data, 1)); });

	for (unsigned i = 0; i < 6; i++)
	{
		RAMSTAR_DEAL_EEPROM(config, m_deal_eeproms[i]);
	}

	GENERIC_CARTSLOT(config, m_carts[0], generic_plain_slot, "ramstar").set_device_load(FUNC(ramstar_state::cart_load<0>));
	GENERIC_CARTSLOT(config, m_carts[1], generic_plain_slot, "ramstar").set_device_load(FUNC(ramstar_state::cart_load<1>));
	GENERIC_CARTSLOT(config, m_carts[2], generic_plain_slot, "ramstar").set_device_load(FUNC(ramstar_state::cart_load<2>));
	GENERIC_CARTSLOT(config, m_carts[3], generic_plain_slot, "ramstar").set_device_load(FUNC(ramstar_state::cart_load<3>));
	GENERIC_CARTSLOT(config, m_carts[4], generic_plain_slot, "ramstar").set_device_load(FUNC(ramstar_state::cart_load<4>));
	GENERIC_CARTSLOT(config, m_carts[5], generic_plain_slot, "ramstar").set_device_load(FUNC(ramstar_state::cart_load<5>));
	m_carts[0]->set_device_unload(FUNC(ramstar_state::cart_unload<0>));
	m_carts[1]->set_device_unload(FUNC(ramstar_state::cart_unload<1>));
	m_carts[2]->set_device_unload(FUNC(ramstar_state::cart_unload<2>));
	m_carts[3]->set_device_unload(FUNC(ramstar_state::cart_unload<3>));
	m_carts[4]->set_device_unload(FUNC(ramstar_state::cart_unload<4>));
	m_carts[5]->set_device_unload(FUNC(ramstar_state::cart_unload<5>));

	SOFTWARE_LIST(config, "cart_list").set_original("ramstar");

	// video hardware
	SCREEN(config, m_screen);
	m_screen->set_raw(10_MHz_XTAL / 2, 320, 0, 256, 262, 0, 240); // TODO: guessed; the program only needs vblank to start at or after line $EE
	m_screen->set_screen_update(FUNC(ramstar_state::screen_update));
	m_screen->screen_vblank().set("via", FUNC(via6522_device::write_cb1)).invert(); // falling edge interrupt

	PALETTE(config, m_palette).set_entries(16);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8912_device &ay(AY8912(config, "ay", 10_MHz_XTAL / 10)); // divider not verified
	ay.port_a_read_callback().set_ioport("DSW1");
	ay.add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( ramstar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512.u46", 0x00000, 0x10000, CRC(8079f1f1) SHA1(82070e1b350e8a91723f892fb21e1fe00c2e5bb2) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "27c512.u45", 0x00000, 0x10000, CRC(9a5d02fd) SHA1(f33baa7cac6f334dd167afb36a50bc0c040d0645) )

	ROM_REGION( 0x2000, "cartcpu", 0 )
	ROM_LOAD( "27c640.u5", 0x0000, 0x2000, CRC(68529c08) SHA1(47aa8693f8c1028e4807e3d94301ef51fab98909) )

	// not dumped; the program refuses to run unless bytes $10-$17 match the hardware ID at $07:D000
	ROM_REGION( 0x200, "i2cmem", ROMREGION_ERASEFF )
	ROM_FILL( 0x10, 1, 0x01 )
	ROM_FILL( 0x11, 1, 0x08 )
	ROM_FILL( 0x12, 1, 0x09 )
	ROM_FILL( 0x13, 1, 0x02 )
	ROM_FILL( 0x14, 2, 0x00 )
	ROM_FILL( 0x16, 1, 0x02 )
	ROM_FILL( 0x17, 1, 0x00 )
ROM_END

} // anonymous namespace


GAME( 1991, ramstar, 0, ramstar, ramstar, ramstar_state, empty_init, ROT0, "Ram Star", "Video Pulltab (version 80.1)", MACHINE_NOT_WORKING )
