// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Casio SX1010 and SX1050.
 *
 * Sources:
 *
 * TODO:
 *   - skeleton only
 */

/*
 * Preliminary parts list for 600-CPU-PCA CPU board:
 *
 * MC68010L10
 * D8237AC-5
 * HD63484-8
 * D8251AFC
 * D72065C
 * D8259AC-2
 * D8251AFC
 * D65021G031    gate array
 * D65030G024    gate array
 * HN62301AP AA1 128k mask ROM
 * HN62301AP AA2 128k mask ROM
 * MB89321A      CMOS Programmable CRT Controller
 * MB4108        ASSP Floppy Disk VFO
 * RP5C15        RTC
 * 48.8MHz XTAL
 *
 * D41256C-15    262144x1 DRAM, x36 == 1M with parity main RAM?
 * MB81464-12    262144x1 DRAM, x16 == 512K video RAM?
 * HM6264ALSP-12 8192-word 8-bit High Speed CMOS Static RAM, x2 == 16K non-volatile RAM?
 */
// WIP: keyboard test at f040ae, if bypassed continues with more interesting tests

#include "emu.h"

#include "cpu/m68000/m68010.h"

// memory
#include "machine/ram.h"
#include "machine/nvram.h"

// various hardware
#include "machine/i8251.h"
#include "machine/pic8259.h"
#include "machine/upd765.h"
#include "machine/am9517a.h"
#include "machine/clock.h"

// video
#include "screen.h"
#include "emupal.h"
#include "video/mc6845.h"
#include "video/hd63484.h"

// busses and connectors
#include "bus/rs232/rs232.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class sx1000_state : public driver_device
{
public:
	sx1000_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_vram(*this, "vram")
		, m_eprom(*this, "eprom")
		, m_pic(*this, "pic")
		, m_screen_crtc(*this, "screen_crtc")
		, m_screen_acrtc(*this, "screen_acrtc")
		, m_palette_acrtc(*this, "palette_acrtc")
		, m_crtc(*this, "crtc")
		, m_acrtc(*this, "acrtc")
		, m_serial1(*this, "i8251_1")
		, m_serial2(*this, "i8251_2")
		, m_fontram(*this, "fontram")
		, m_sw1(*this, "SW1")
		, m_sw2(*this, "SW2")
	{
	}

	void sx1010(machine_config &config);
	void init_common();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void cpu_map(address_map &map) ATTR_COLD;
	void acrtc_map(address_map &map) ATTR_COLD;

	void common(machine_config &config);

private:
	MC6845_UPDATE_ROW(crtc_update_row);

	// devices
	required_device<m68010_device> m_cpu;
	required_device<ram_device> m_ram;
	required_shared_ptr<u16> m_vram;
	required_memory_region m_eprom;

	required_device<pic8259_device> m_pic;

	required_device<screen_device> m_screen_crtc;
	required_device<screen_device> m_screen_acrtc;
	required_device<palette_device> m_palette_acrtc;
	required_device<hd6345_device> m_crtc;
	required_device<hd63484_device> m_acrtc;
	required_device<i8251_device>  m_serial1;
	required_device<i8251_device>  m_serial2;

	required_shared_ptr<u16> m_fontram;

	required_ioport m_sw1;
	required_ioport m_sw2;
	u16 f14000_r();
	u16 f16000_r();
};

void sx1000_state::machine_start()
{
}

void sx1000_state::machine_reset()
{
}

void sx1000_state::init_common()
{
}

void sx1000_state::cpu_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region(m_eprom, 0); // FIXME: probably mapped/unmapped during reset

	// FIXME: additional 1M RAM board seems to be mandatory?
	map(0x010000, 0x1fffff).ram();

	map(0xa00000, 0xbfffff).ram(); // Banking on f00000 writes, if it makes any sense? Would provide up to 32M of ram, bank is 4 bits

	map(0xf00000, 0xf0ffff).rom().region(m_eprom, 0);

	map(0xf11001, 0xf11001).rw(m_serial1, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xf11003, 0xf11003).rw(m_serial1, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xf11004, 0xf11005).lr16(NAME([this]() { return m_serial1->txrdy_r() ? 2 : 0; }));

	map(0xf13801, 0xf13801).w(m_crtc, FUNC(hd6345_device::address_w));
	map(0xf13901, 0xf13901).rw(m_crtc, FUNC(hd6345_device::register_r), FUNC(hd6345_device::register_w));

	// reads of f14000.w don't entirely pan out as pic, let's cheat a little
	map(0xf14001, 0xf14001).lrw8([this]() { return m_pic->read(0); }, "pic_r0", [this](u8 data) { m_pic->write(0, data); }, "pic_w0");
	map(0xf14101, 0xf14101).lrw8([this]() { return m_pic->read(1); }, "pic_r1", [this](u8 data) { m_pic->write(1, data); }, "pic_w1");

	map(0xf14000, 0xf14001).r(FUNC(sx1000_state::f14000_r));
	map(0xf16000, 0xf16001).r(FUNC(sx1000_state::f16000_r));

	map(0xf1a001, 0xf1a001).rw(m_serial2, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xf1a003, 0xf1a003).rw(m_serial2, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xf1a004, 0xf1a005).lr16(NAME([this]() { return m_serial2->txrdy_r() ? 2 : 0; }));

	map(0xf20000, 0xf23fff).ram().share(m_vram);

	map(0xf28000, 0xf28001).lrw16([this]() { return m_acrtc->read16(0); }, "acrtc_status_r", [this](u16 data) { m_acrtc->write16(0, data); }, "acrtc_address_w");
	map(0xf28100, 0xf28101).lrw16([this]() { return m_acrtc->read16(1); }, "acrtc_data_r", [this](u16 data) { m_acrtc->write16(1, data); }, "acrtc_data_w");

	map(0xf2e000, 0xf2ffff).ram().share(m_fontram);
}

void sx1000_state::acrtc_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
}

u16 sx1000_state::f14000_r()
{
	u8 res = m_sw2->read();

	if(m_crtc->vsync_r())
		res |= 0x20;

	return res;
}

u16 sx1000_state::f16000_r()
{
	u8 res = m_sw1->read();

	return res;
}

MC6845_UPDATE_ROW( sx1000_state::crtc_update_row )
{
	//  logerror("ma=%x ra=%d y=%d x_count=%d cursor_x=%d de=%d hbp=%d vbp=%d\n", ma*2, ra, y, x_count, cursor_x, de, hbp, vbp);
	const u16 *vram = m_vram + ma;
	u32 *dest = &bitmap.pix(y);
	for(u32 x0 = 0; x0 != x_count; x0 ++) {
		u16 const data = *vram++;
		u16 const bitmap = m_fontram[((data & 0xff) << 4) | (ra)];
		for(u32 x1 = 0; x1 != 8; x1 ++) {
			u32 color = BIT(bitmap, 7-x1) ? 0xffffff : 0x000000;
			*dest ++ = color;
			*dest ++ = color;
		}
	}
}

static DEVICE_INPUT_DEFAULTS_START( terminal1 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( terminal2 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void sx1000_state::common(machine_config &config)
{
	M68010(config, m_cpu, 10'000'000);
	m_cpu->set_addrmap(AS_PROGRAM, &sx1000_state::cpu_map);

	// 36 x D41256C-15 (256Kb DRAM) on CPU board
	RAM(config, m_ram);
	m_ram->set_default_size("1M");

	//  NVRAM(config, "nvram");

	PIC8259(config, m_pic);

	// M6845 config screen: HTOTAL: 944  VTOTAL: 444  MAX_X: 639  MAX_Y: 399  HSYNC: 720-823  VSYNC: 416-425  Freq: 76.347534fps
	SCREEN(config, m_screen_crtc, SCREEN_TYPE_RASTER);
	m_screen_crtc->set_raw(48'800'000, 944, 0, 640, 444, 0, 400);
	m_screen_crtc->set_screen_update(m_crtc, FUNC(hd6345_device::screen_update));

	// ACRTC: full 944x449 vis (200, 18)-(847, 417)
	SCREEN(config, m_screen_acrtc, SCREEN_TYPE_RASTER);
	m_screen_acrtc->set_raw(48'800'000, 944, 200, 847, 449, 18, 417);
	m_screen_acrtc->set_screen_update(m_acrtc, FUNC(hd63484_device::update_screen));
	m_screen_acrtc->set_palette(m_palette_acrtc);

	PALETTE(config, m_palette_acrtc).set_entries(16);

	// htotal = 117
	// hdisp = 80
	// vtotal = 26
	// vdisp = 25
	// mrast = 15
	// MB89321A
	HD6345(config, m_crtc, 4'000'000);
	m_crtc->set_screen(m_screen_crtc);
	m_crtc->set_update_row_callback(FUNC(sx1000_state::crtc_update_row));
	m_crtc->out_vsync_callback().set(m_pic, FUNC(pic8259_device::ir5_w));
	m_crtc->set_hpixels_per_column(8);

	HD63484(config, m_acrtc, 8'000'000);
	m_acrtc->set_screen(m_screen_acrtc);
	m_acrtc->set_addrmap(0, &sx1000_state::acrtc_map);

	auto &rs232_1(RS232_PORT(config, "serial1", default_rs232_devices, nullptr));
	rs232_1.rxd_handler().set(m_serial1, FUNC(i8251_device::write_rxd));
	rs232_1.dsr_handler().set(m_serial1, FUNC(i8251_device::write_dsr));
	rs232_1.cts_handler().set(m_serial1, FUNC(i8251_device::write_cts));
	rs232_1.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal1));

	I8251(config, m_serial1, 4'000'000);
	m_serial1->txd_handler().set(rs232_1, FUNC(rs232_port_device::write_txd));
	m_serial1->dtr_handler().set(rs232_1, FUNC(rs232_port_device::write_dtr));
	m_serial1->rts_handler().set(rs232_1, FUNC(rs232_port_device::write_rts));

	CLOCK(config, "clock1", 1200*16).signal_handler().set(m_serial1, FUNC(i8251_device::write_txc));

	auto &rs232_2(RS232_PORT(config, "serial2", default_rs232_devices, nullptr));
	rs232_2.rxd_handler().set(m_serial2, FUNC(i8251_device::write_rxd));
	rs232_2.dsr_handler().set(m_serial2, FUNC(i8251_device::write_dsr));
	rs232_2.cts_handler().set(m_serial2, FUNC(i8251_device::write_cts));
	rs232_2.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal2));

	I8251(config, m_serial2, 4'000'000);
	m_serial2->txd_handler().set(rs232_2, FUNC(rs232_port_device::write_txd));
	m_serial2->dtr_handler().set(rs232_2, FUNC(rs232_port_device::write_dtr));
	m_serial2->rts_handler().set(rs232_2, FUNC(rs232_port_device::write_rts));

	CLOCK(config, "clock2", 9600*16).signal_handler().set(m_serial2, FUNC(i8251_device::write_txc));
}

void sx1000_state::sx1010(machine_config &config)
{
	common(config);
}

static INPUT_PORTS_START(sx1010)
	PORT_START("SW1")
	PORT_DIPNAME( 0x20, 0x00, "Ignore keyboard error" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x20, "On" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x02, 0x00, "Serial console" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x02, "On" )
INPUT_PORTS_END

ROM_START(sx1010)
	ROM_REGION16_BE(0x10000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "sx1010", "sx1010")
	ROMX_LOAD("pc1a.u6l8", 0x8001, 0x4000, CRC(75d0f02c) SHA1(dfcc7efc1b5e7b43fc1ee030bef5c75c23d5e742), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_CONTINUE (         0x0001, 0x4000)
	ROMX_LOAD("pc1a.u6h8", 0x8000, 0x4000, CRC(a928c0c9) SHA1(712601ee889a0790a7579fe06df20ec7e0a4bb49), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_CONTINUE (         0x0000, 0x4000)
ROM_END

} // anonymous namespace

/*   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY  FULLNAME  FLAGS */
COMP(1987, sx1010, 0,      0,      sx1010,  sx1010, sx1000_state, init_common, "Casio", "SX1010", MACHINE_IS_SKELETON)
