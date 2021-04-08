// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    BASF 7100

    Models:
    - 7120: 24k disk controller memory, 3x 5.25" single sided
    - 7125: 32k disk controller memory, 3x 5.25" double sided
    - 7130: 32k disk controller memory, 1x 5.25" double sided, 1x Winchester

    Hardware:
    - Z-80A
    - I8259A
    - I8251
    - COM8116
    - 3x 8255A
    - 64k memory
    - 5.0688 MHz XTAL
    - 4x DSW8
    - External ports: Printer, Dialer, Data Comm I/O, Aux I/O

    Video board:
    - Motorola 160A002-B (?)
    - 2x 8255A
    - 18.720 MHz XTAL

    Floppy controller:
    - Z-80A
    - 24k/32k memory
    - FD1791B-02

    Aux I/O board:
    - I8251
    - COM8116
    - 5.0688 MHz XTAL

    Keyboard:
    - 30293E-054 20-04592-054 (?)

    TODO:
    - The floppy is partially hooked up, everything else needs to be done

    Notes:
    - Runs the BOS operating system, possibly also CP/M?

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class basf7100_state : public driver_device
{
public:
	basf7100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_fdccpu(*this, "fdccpu"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0)
	{ }

	void basf7100(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_fdccpu;
	required_device<fd1791_device> m_fdc;
	required_device_array<floppy_connector, 3> m_floppy;

	void mem_map(address_map &map);
	void io_map(address_map &map);

	void fdc_mem_map(address_map &map);
	void fdc_io_map(address_map &map);

	uint8_t mmio_r(offs_t offset);
	void mmio_w(offs_t offset, uint8_t data);

	IRQ_CALLBACK_MEMBER(fdccpu_irq_callback);

	uint8_t fdc_ctrl_r();
	void fdc_ctrl_w(uint8_t data);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void basf7100_state::mem_map(address_map &map)
{
	map(0xff00, 0xffff).rw(FUNC(basf7100_state::mmio_r), FUNC(basf7100_state::mmio_w));
}

void basf7100_state::io_map(address_map &map)
{
	map.global_mask(0xff);
//	map(0x00, 0x03) 8255 (keyboard)
//	map(0x04, 0x07) 8255 (printer)
//	map(0x08, 0x09).mirror(0x02) 8259
//	map(0x0c, 0x0f) 8255
//	map(0x10, 0x11) 8251 (primary)
//	map(0x12, 0x12) baud rate
//	map(0x14, 0x15) 8251 (secondary)
//	map(0x16, 0x16) baud rate
//	map(0x17, 0x17) rs232 flags/control
//	map(0x18, 0x18) interrupt flags
//	map(0x1c, 0x1f) switches
//	map(0xb0, 0xb3) display hardware clear
//	map(0xb8, 0xbb) 8255 (cursor)
//	map(0xbc, 0xbf) 8255 (sod)
	map(0xc0, 0xc3).rw(m_fdc, FUNC(fd1791_device::read), FUNC(fd1791_device::write));
	map(0xc4, 0xc4).rw(FUNC(basf7100_state::fdc_ctrl_r), FUNC(basf7100_state::fdc_ctrl_w));
}

void basf7100_state::fdc_mem_map(address_map &map)
{
	map(0xd000, 0xefff).ram();
	map(0xfc00, 0xffff).rom().region("fdccpu", 0);
	map(0xff00, 0xffff).rw(FUNC(basf7100_state::mmio_r), FUNC(basf7100_state::mmio_w));
}

void basf7100_state::fdc_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(basf7100_state::mmio_r), FUNC(basf7100_state::mmio_w));
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( basf7100 )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint8_t basf7100_state::fdc_ctrl_r()
{
	// 7-------  unknown, checked after seek cmd
	// -6543210  unknown

	return 0x80;
}

void basf7100_state::fdc_ctrl_w(uint8_t data)
{
	// 7654----  unknown
	// ----3---  select drive 0 or motor on?
	// -----210  unknown

	logerror("fdc_ctrl_w: %04x\n", data);
	
	floppy_image_device *floppy = nullptr;

	// hardcoded to drive 0 for now
	floppy = m_floppy[0]->get_device();
	m_fdc->set_floppy(floppy);
}

uint8_t basf7100_state::mmio_r(offs_t offset)
{
	return m_maincpu->space(AS_IO).read_byte(offset);
}

void basf7100_state::mmio_w(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_IO).write_byte(offset, data);
}

IRQ_CALLBACK_MEMBER( basf7100_state::fdccpu_irq_callback )
{
	return 0x60;
}

void basf7100_state::machine_start()
{
}

void basf7100_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_fdccpu->set_pc(0xfc00);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static void basf7100_floppies(device_slot_interface &device)
{
	device.option_add("basf6106", FLOPPY_525_SSSD);
}

void basf7100_state::basf7100(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &basf7100_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &basf7100_state::io_map);

	Z80(config, m_fdccpu, 4000000);
	m_fdccpu->set_addrmap(AS_PROGRAM, &basf7100_state::fdc_mem_map);
	m_fdccpu->set_addrmap(AS_IO, &basf7100_state::fdc_io_map);
	m_fdccpu->set_irq_acknowledge_callback(FUNC(basf7100_state::fdccpu_irq_callback));

	// floppy
	FD1791(config, m_fdc, 2000000); // unknown clock
	m_fdc->intrq_wr_callback().set_inputline(m_fdccpu, INPUT_LINE_IRQ0);
	FLOPPY_CONNECTOR(config, "fdc:0", basf7100_floppies, "basf6106", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", basf7100_floppies, "basf6106", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", basf7100_floppies, "basf6106", floppy_image_device::default_mfm_floppy_formats);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( basf7120 )
	ROM_REGION(0x400, "fdccpu", 0)
	ROM_LOAD("19-2130-2h.u45", 0x000, 0x400, CRC(cb077c69) SHA1(dfa16082b88275442c48082aeb5f62fe1238ae3e))

	ROM_REGION(0x400, "keyboard", 0)
	ROM_LOAD("19-2114-01e.bin", 0x000, 0x400, CRC(d694b5dd) SHA1(6262379ba565c1de072b2b21dc3141db1ec5129c))

	ROM_REGION(0x20, "cpu_pal", 0)
	ROM_LOAD("cpu_board_im561cje_192113-2.u21", 0x00, 0x20, CRC(4405e26f) SHA1(0f93c47e9f546b42a85e5eced58337e0add443c4))
	
	ROM_REGION(0x50, "floppy_pal", 0)
	ROM_LOAD("floppy_board_pal10l8mj_19-2131-1.u23", 0x00, 0x28, CRC(f37ed4bc) SHA1(824b4405f396c262cf8116f85eb0b548eabb4c04))
	ROM_LOAD("floppy_board_pal10l8mj_19-2132-1.u24", 0x28, 0x28, CRC(b918ff18) SHA1(c6d7cd9642ed32e56b5c1df1ddf3afe09d744ebc))
	
	ROM_REGION(0x100, "video_pal", 0)
	ROM_LOAD("video_board_30_82s129n.bin", 0x000, 0x100, CRC(89175ac9) SHA1(69b2055bee87e11cc74c70cef2f2bebcbd0004c9))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY   FULLNAME  FLAGS
COMP( 1982, basf7120, 0,      0,      basf7100, basf7100, basf7100_state, empty_init, "BASF",   "7120",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
