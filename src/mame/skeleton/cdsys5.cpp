// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for "CD System 5" jukebox by Sound Leisure Ltd.

****************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "machine/74259.h"
#include "machine/mc146818.h"
#include "machine/nvram.h"


namespace {

class cdsys5_state : public driver_device
{
public:
	cdsys5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_shift_register(0)
	{
	}

	void minijook(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void shift_data_w(u8 data);
	void shift_latch_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<mc68hc11_cpu_device> m_maincpu;

	u16 m_shift_register;
};


void cdsys5_state::machine_start()
{
	save_item(NAME(m_shift_register));
}

void cdsys5_state::shift_data_w(u8 data)
{
	m_shift_register = (m_shift_register << 1) | (data & 1);
}

void cdsys5_state::shift_latch_w(u8 data)
{
	//logerror("Latching $%04X from shift register\n", m_shift_register);
}

void cdsys5_state::mem_map(address_map &map)
{
	map(0x0401, 0x0401).nopw(); // watchdog?
	//map(0x0420, 0x0421).w("lcdc", FUNC(hd44780_device::write));
	map(0x0440, 0x0441).nopr();
	map(0x0802, 0x0803).nopr();
	map(0x0807, 0x0807).nopr();
	map(0x0810, 0x081f).w("outlatch1", FUNC(cd4099_device::write_a0));
	map(0x0820, 0x082f).w("outlatch2", FUNC(cd4099_device::write_a0));
	map(0x0833, 0x0833).nopr();
	map(0x0850, 0x085f).w("outlatch5", FUNC(cd4099_device::write_a0));
	map(0x0860, 0x086f).w("outlatch6", FUNC(cd4099_device::write_a0));
	map(0x0880, 0x088f).w("outlatch8", FUNC(cd4099_device::write_a0));
	map(0x08a0, 0x08a0).w(FUNC(cdsys5_state::shift_latch_w));
	map(0x08b0, 0x08b0).w(FUNC(cdsys5_state::shift_data_w));
	map(0x0c00, 0x0c3f).rw("rtc", FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct));
	map(0x4000, 0x47ff).ram().share("novram");
	map(0x8000, 0xffff).rom().region("program", 0);
}


static INPUT_PORTS_START(minijook)
	PORT_START("DIP1") // next to a TC4512BP 8-to-1 multiplexer
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown)) PORT_DIPLOCATION("DIL1:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown)) PORT_DIPLOCATION("DIL1:2")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown)) PORT_DIPLOCATION("DIL1:3")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown)) PORT_DIPLOCATION("DIL1:4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown)) PORT_DIPLOCATION("DIL1:5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown)) PORT_DIPLOCATION("DIL1:6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown)) PORT_DIPLOCATION("DIL1:7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown)) PORT_DIPLOCATION("DIL1:8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	// DIL2 next to DIL1 with its own TC4512BP, both unpopulated
	// 3 more TC4512BP on board, likely for additional inputs
INPUT_PORTS_END


void cdsys5_state::minijook(machine_config &config)
{
	MC68HC11D0(config, m_maincpu, 8_MHz_XTAL); // 44-pin PLCC next to unpopulated 40-pin DIP "68HC11"
	m_maincpu->set_addrmap(AS_PROGRAM, &cdsys5_state::mem_map);
	// RS232 I/O through MAX232CPE

	NVRAM(config, "novram", nvram_device::DEFAULT_ALL_0); // Xicor X20C17P-55 Autostore NOVRAM

	MC146818(config, "rtc", 32.768_kHz_XTAL);

	CD4099(config, "outlatch1");
	CD4099(config, "outlatch2");
	CD4099(config, "outlatch5");
	CD4099(config, "outlatch6");
	CD4099(config, "outlatch8"); // 5 on board
}


ROM_START(minijook)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("mj-sl-s-v3.4.u10", 0x0000, 0x8000, CRC(7b8f03ce) SHA1(9d7fb9a9f5051ecadad5a33860117bc4dfd8ab30)) // 0xxxxxxxxxxxxxx = 0xFF
ROM_END

} // anonymous namespace


SYST(199?, minijook, 0, 0, minijook, minijook, cdsys5_state, empty_init, "Sound Leisure", "MiniJook", MACHINE_IS_SKELETON_MECHANICAL)
