// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Lee Data 1214 terminal (IBM 3278-compatible).

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/eeprompar.h"
#include "machine/z80sio.h"
//#include "video/mc6845.h"
//#include "screen.h"


namespace {

class lee1214_state : public driver_device
{
public:
	lee1214_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void lee1214(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	[[maybe_unused]] u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
};

void lee1214_state::mem_map(address_map &map)
{
	map(0x00000, 0x007ff).ram();
	map(0x80000, 0x80fff).ram();
	map(0xa0000, 0xa1fff).ram();
	map(0xf8000, 0xfffff).rom().region("maincpu", 0);
}

void lee1214_state::io_map(address_map &map)
{
	map(0x0000, 0x0003).rw("mpsc", FUNC(i8274_device::cd_ba_r), FUNC(i8274_device::cd_ba_w));
	//map(0x0100, 0x0100).w("crtc", FUNC(mc6845_device::address_w));
	//map(0x0101, 0x0101).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

void lee1214_state::machine_start()
{
}

u32 lee1214_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START(lee1214)
INPUT_PORTS_END

void lee1214_state::lee1214(machine_config &config)
{
	I80188(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &lee1214_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &lee1214_state::io_map);

	i8274_device &mpsc(I8274(config, "mpsc", 4'000'000));
	mpsc.out_int_callback().set("maincpu", FUNC(i80188_cpu_device::int0_w));

	EEPROM_2816(config, "eeprom");
}

ROM_START(lee1214d) // X1 = 12 MHz, X2 = 41.028 MHz, X3 = 24.823 MHz, X5 = 5.9335 MHz
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("13311502_u41_27256.bin", 0x0000, 0x8000, CRC(6f469221) SHA1(deef1d83c41fdbe09b830dc45698988cf89003e0))

	ROM_REGION(0x0800, "eeprom", 0)
	ROM_LOAD("13300001_u19_2816.bin", 0x0000, 0x0800, CRC(30411ecd) SHA1(8755a1e0a36fe96d438bf2ee35cb0917fbc97e52))
ROM_END

} // anonymous namespace


COMP(1985, lee1214d, 0, 0, lee1214, lee1214, lee1214_state, empty_init, "Lee Data", "1214D Display Terminal", MACHINE_IS_SKELETON)
