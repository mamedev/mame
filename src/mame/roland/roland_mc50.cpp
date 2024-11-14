// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland MC-50 & similar MIDI sequencers.

****************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/z180/z180.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"


namespace {

class roland_mc50_state : public driver_device
{
public:
	roland_mc50_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mpu(*this, "mpu")
		, m_wdfdc(*this, "wdfdc")
		, m_wdfdd(*this, "wdfdc:0")
	{
	}

	void mc300(machine_config &config);
	void mc50(machine_config &config);
	void mc50mk2(machine_config &config);

private:
	void mem_map_mc300(address_map &map) ATTR_COLD;
	void mem_map_mc50(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z180_device> m_mpu;
	optional_device<wd1772_device> m_wdfdc;
	//optional_device<hd63266_device> m_hdfdc;
	optional_device<floppy_connector> m_wdfdd;
};

void roland_mc50_state::mem_map_mc300(address_map &map)
{
	map(0x00000, 0x01fff).rom().region("mpurom", 0);
	map(0x40000, 0xfffff).ram();
}

void roland_mc50_state::mem_map_mc50(address_map &map)
{
	map(0x00000, 0x3ffff).rom().region("mpurom", 0);
	map(0x40000, 0x7ffff).ram();
	map(0x80000, 0xfffff).rom().region("mpurom", 0);
}

void roland_mc50_state::io_map(address_map &map)
{
	map(0x0000, 0x003f).noprw(); // internal registers
	map(0x00c0, 0x00c3).mirror(0xff00).rw(m_wdfdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write));
}

static INPUT_PORTS_START(mc50)
INPUT_PORTS_END

static void mc50_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

void roland_mc50_state::mc300(machine_config &config)
{
	Z80180(config, m_mpu, 10_MHz_XTAL);
	m_mpu->set_addrmap(AS_PROGRAM, &roland_mc50_state::mem_map_mc300);
	m_mpu->set_addrmap(AS_IO, &roland_mc50_state::io_map);

	//bu3904s_device &fsk(BU3904S(config, "fsk", 10_MHz_XTAL / 2));
	//fsk.xint_callback().set_inputline(m_mpu, Z180_INPUT_LINE_IRQ0);

	WD1772(config, m_wdfdc, 8_MHz_XTAL);
	m_wdfdc->intrq_wr_callback().set_inputline(m_mpu, Z180_INPUT_LINE_IRQ1);
	m_wdfdc->drq_wr_callback().set_inputline(m_mpu, Z180_INPUT_LINE_DREQ0);

	FLOPPY_CONNECTOR(config, m_wdfdd, mc50_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats);
}

void roland_mc50_state::mc50(machine_config &config)
{
	Z80180(config, m_mpu, 10_MHz_XTAL); // HD64180R1F6
	m_mpu->set_addrmap(AS_PROGRAM, &roland_mc50_state::mem_map_mc50);
	m_mpu->set_addrmap(AS_IO, &roland_mc50_state::io_map);

	//mn53015_device &fsk(MN53015(config, "fsk", 10_MHz_XTAL / 2));
	//fsk.int_callback().set_inputline(m_mpu, Z180_INPUT_LINE_IRQ0);

	WD1772(config, m_wdfdc, 8_MHz_XTAL); // WD1772-02
	m_wdfdc->intrq_wr_callback().set_inputline(m_mpu, Z180_INPUT_LINE_IRQ1);
	m_wdfdc->drq_wr_callback().set_inputline(m_mpu, Z180_INPUT_LINE_DREQ0);
	m_wdfdc->dden_w(0);

	FLOPPY_CONNECTOR(config, m_wdfdd, mc50_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats);

	// TODO: LCD unit (EA-D20225PX-1)
}

void roland_mc50_state::mc50mk2(machine_config &config)
{
	mc50(config);

	//config.device_remove("wdfdc");
	//HD63266(config, m_hdfdc, 16_MHz_XTAL); // HD63266F
}

ROM_START(mc300)
	ROM_REGION(0x2000, "mpurom", 0)
	ROM_SYSTEM_BIOS(0, "v102", "Version 1.02")
	ROMX_LOAD("osv1.02-27c64.bin", 0x0000, 0x2000, CRC(a509d5b2) SHA1(d5b23aa2514610d3a5b3f33bdf4479fdb8dc91bd), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v101", "Version 1.01")
	ROMX_LOAD("osv1.01-27c64.bin", 0x0000, 0x2000, CRC(459fd2a1) SHA1(5cea936807c80945ac39d0e7ecc63d3a2b968664), ROM_BIOS(1))
ROM_END

ROM_START(mc50)
	ROM_REGION(0x80000, "mpurom", 0)
	ROM_LOAD("mc50_mk1.ic9", 0x00000, 0x80000, CRC(322b5f73) SHA1(f5cb5738f03c51d7019a661ac660ecea56b44350))
ROM_END

ROM_START(mc50mk2)
	ROM_REGION(0x80000, "mpurom", 0)
	ROM_LOAD("mc50_mkii-firmware-v.0060.ic7", 0x00000, 0x80000, CRC(a428e378) SHA1(ac411c33ef79cf9ae51c5d5dbb1ed9ea5839db3e))
ROM_END

} // anonymous namespace


SYST(1988, mc300,   0, 0, mc300,   mc50, roland_mc50_state, empty_init, "Roland", "MC-300 Micro Composer", MACHINE_IS_SKELETON)
SYST(1990, mc50,    0, 0, mc50,    mc50, roland_mc50_state, empty_init, "Roland", "MC-50 Micro Composer", MACHINE_IS_SKELETON)
SYST(1992, mc50mk2, 0, 0, mc50mk2, mc50, roland_mc50_state, empty_init, "Roland", "MC-50mkII Micro Composer", MACHINE_IS_SKELETON)
