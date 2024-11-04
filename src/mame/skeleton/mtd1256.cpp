// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************************

    Skeleton driver for data acquisition station by Geonica S.A.

**********************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "machine/bankdev.h"
//#include "machine/icl7109.h"
#include "machine/mm58274c.h"
#include "machine/mm74c922.h"
//#include "machine/nvram.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"


namespace {

class mtd1256_state : public driver_device
{
public:
	mtd1256_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bank0(*this, "bank0")
		, m_encoder(*this, "encoder")
		, m_bankreg(0)
		, m_portd(0xff)
	{
	}

	void mtd1256(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(pixel_update);

	u8 porta_r();
	void portd_w(u8 data);

	void bank0_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<mc68hc11_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bank0;
	required_device<mm74c923_device> m_encoder;

	u8 m_bankreg;
	u8 m_portd;
};

void mtd1256_state::machine_start()
{
	save_item(NAME(m_bankreg));
	save_item(NAME(m_portd));
}

HD44780_PIXEL_UPDATE(mtd1256_state::pixel_update)
{
	// Is this the right layout?
	if (pos < 20)
		bitmap.pix(line * 16 + y, pos * 6 + x) = state;
	else if (pos < 40)
		bitmap.pix(line * 16 + 8 + y, (pos - 20) * 6 + x) = state;
}


u8 mtd1256_state::porta_r()
{
	// FIXME: bit 2 should be ICL7109 busy output
	return m_encoder->da_r() | (machine().rand() & 0x04);
}

void mtd1256_state::portd_w(u8 data)
{
	if (!BIT(m_portd, 4) && BIT(data, 4))
		m_bankreg = (m_bankreg << 1) | BIT(data, 5);

	m_bank0->set_bank(BIT(data, 5) ? m_bankreg : 0);

	m_portd = data;
}

void mtd1256_state::bank0_map(address_map &map)
{
	map(0x000000, 0x007fff).ram();
	map(0x040000, 0x047fff).ram();
	map(0x080000, 0x087fff).ram();
	map(0x0c0000, 0x0c000f).rw("rtc", FUNC(mm58274c_device::read), FUNC(mm58274c_device::write));
	map(0x0c0020, 0x0c0020).nopw();
	map(0x100000, 0x100000).r(m_encoder, FUNC(mm74c922_device::read));
	map(0x100020, 0x100021).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x180000, 0x187fff).ram();
	map(0x1c0000, 0x1dffff).ram();
}

void mtd1256_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).m(m_bank0, FUNC(address_map_bank_device::amap8));
	map(0x2000, 0xffff).rom().region("program", 0x2000); // partly overlaid by internal spaces
}


static INPUT_PORTS_START(mtd1256)
INPUT_PORTS_END

void mtd1256_state::mtd1256(machine_config &config)
{
	MC68HC11A1(config, m_maincpu, 1.8432_MHz_XTAL); // yes, this appears to be the CPU XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &mtd1256_state::mem_map);
	m_maincpu->in_pa_callback().set(FUNC(mtd1256_state::porta_r));
	m_maincpu->out_pd_callback().set(FUNC(mtd1256_state::portd_w));

	ADDRESS_MAP_BANK(config, m_bank0);
	m_bank0->set_addrmap(0, &mtd1256_state::bank0_map);
	m_bank0->set_data_width(8);
	m_bank0->set_endianness(ENDIANNESS_BIG);
	m_bank0->set_addr_width(21);
	m_bank0->set_stride(0x2000);

	// TODO: NVRAM is 4x NEC µPD431000AGW-70L 128Kx8 SRAMs + 3V lithium battery

	MM58274C(config, "rtc", 32.768_kHz_XTAL); // TODO: 1 second interrupt output configured

	MM74C923(config, m_encoder, 0); // timing parameters unknown

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(20*6, 32);
	screen.set_visarea(0, 20*6-1, 0, 32-1);
	screen.set_palette("palette");

	hd44780_device &hd44780(HD44780(config, "lcdc", 270'000)); // TODO: clock not measured, datasheet typical clock used
	hd44780.set_lcd_size(2, 16);
	hd44780.set_pixel_update_cb(FUNC(mtd1256_state::pixel_update));

	PALETTE(config, "palette").set_entries(2);

	// TODO: add Maxim ICL7109CQH 12-bit A/D converter
	// TODO: add RS232 port (through ADM202JRW)
}


ROM_START(mtd1256)
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("cieenres.b_26-11-92.u7", 0x00000, 0x10000, CRC(a507effd) SHA1(46b3399c0c26c6952a5582c79c14663515e3e180))
ROM_END

} // anonymous namespace


SYST(1992, mtd1256, 0, 0, mtd1256, mtd1256, mtd1256_state, empty_init, "Geonica", "Meteodata 1256", MACHINE_IS_SKELETON)
