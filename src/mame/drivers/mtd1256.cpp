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
//#include "screen.h"

class mtd1256_state : public driver_device
{
public:
	mtd1256_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bank0(*this, "bank0")
	{
	}

	void mtd1256(machine_config &config);

private:
	void portd_w(u8 data);

	void bank0_map(address_map &map);
	void mem_map(address_map &map);

	required_device<mc68hc11_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bank0;
};


void mtd1256_state::portd_w(u8 data)
{
	m_bank0->set_bank(BIT(data, 5));
}

void mtd1256_state::bank0_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x100f).rw("rtc", FUNC(mm58274c_device::read), FUNC(mm58274c_device::write));
}

void mtd1256_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).m(m_bank0, FUNC(address_map_bank_device::amap8));
	map(0x1000, 0x1fff).ram();
	map(0x2000, 0xffff).rom().region("program", 0x2000); // partly overlaid by internal spaces
}


static INPUT_PORTS_START(mtd1256)
INPUT_PORTS_END

void mtd1256_state::mtd1256(machine_config &config)
{
	MC68HC11A1(config, m_maincpu, 1.8432_MHz_XTAL); // yes, this appears to be the CPU XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &mtd1256_state::mem_map);
	m_maincpu->out_pd_callback().set(FUNC(mtd1256_state::portd_w));

	ADDRESS_MAP_BANK(config, m_bank0);
	m_bank0->set_addrmap(0, &mtd1256_state::bank0_map);
	m_bank0->set_data_width(8);
	m_bank0->set_endianness(ENDIANNESS_BIG);
	m_bank0->set_addr_width(13);
	m_bank0->set_stride(0x1000);

	MM58274C(config, "rtc", 32.768_kHz_XTAL); // TODO: 1 second interrupt output configured

	MM74C923(config, "encoder", 0); // timing parameters unknown

	// TODO: add 4x NEC µPD431000AGW-70L 128Kx8 SRAMs + 3V lithium battery
	// TODO: add Maxim ICL7109CQH 12-bit A/D converter
	// TODO: add STN LCD panel
	// TODO: add RS232 port (through ADM202JRW)
}


ROM_START(mtd1256)
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("cieenres.b_26-11-92.u7", 0x00000, 0x10000, CRC(a507effd) SHA1(46b3399c0c26c6952a5582c79c14663515e3e180))
ROM_END

SYST(1992, mtd1256, 0, 0, mtd1256, mtd1256, mtd1256_state, empty_init, "Geonica", "Meteodata 1256", MACHINE_IS_SKELETON)
