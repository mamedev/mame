// license:BSD-3-Clause
// copyright-holders:Carl

#include "emu.h"

#include "cpu/z80/z80.h"
#include "cpu/i86/i86.h"
#include "machine/z80sio.h"
#include "machine/genpc.h"
#include "machine/x2212.h"
#include "machine/pc_lpt.h"
#include "video/cgapal.h"
#include "bus/isa/cga.h"
#include "bus/isa/fdc.h"
#include "bus/isa/isa.h"
#include "bus/rs232/rs232.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"

namespace {

class chameleon_state : public driver_device
{
public:
	chameleon_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_i8088(*this, "i8088"),
		m_z80(*this, "z80"),
		m_i8274(*this, "i8274"),
		m_mb(*this, "mb"),
		m_x2210a(*this, "x2210a"),
		m_lpt(*this, "lpt")
	{
	}

	void chameleon(machine_config &config);
	MC6845_UPDATE_ROW( crtc_update_row );

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void i8088_io(address_map &map) ATTR_COLD;
	void i8088_map(address_map &map) ATTR_COLD;
	void z80_io(address_map &map) ATTR_COLD;
	void z80_map(address_map &map) ATTR_COLD;

	void proc_swap(uint8_t data);

	required_device<cpu_device> m_i8088;
	required_device<cpu_device> m_z80;
	required_device<i8274_device> m_i8274;
	required_device<pc_noppi_mb_device> m_mb;
	required_device<x2210_device> m_x2210a;
	required_device<pc_lpt_device> m_lpt;

	uint32_t m_z80_bank;
	bool m_z80_active;
	address_space *m_8088_mem;
	address_space *m_8088_io;
};

void chameleon_state::machine_start()
{
	m_8088_mem = &m_i8088->space(AS_PROGRAM);
	m_8088_io = &m_i8088->space(AS_IO);
}

void chameleon_state::machine_reset()
{
	m_z80->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_z80_active = false;
}

void chameleon_state::proc_swap(uint8_t data)
{
	if(m_z80_active)
	{
		m_z80->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_i8088->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_z80_active = false;
	}
	else
	{
		m_i8088->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_z80->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_z80_active = true;
	}
}

void chameleon_state::i8088_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(pc_noppi_mb_device::map));
	map(0x0030, 0x0031).rw(m_i8274, FUNC(i8274_device::cd_ba_r), FUNC(i8274_device::cd_ba_w));
	map(0x0378, 0x037b).rw(m_lpt, FUNC(pc_lpt_device::read), FUNC(pc_lpt_device::write));
	map(0x0379, 0x0379).lw8(NAME([this](uint8_t d) { m_z80_bank = (d << 16); }));
	map(0x03de, 0x03de).w(FUNC(chameleon_state::proc_swap));
}

void chameleon_state::i8088_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf3000, 0xf303f).rw(m_x2210a, FUNC(x2210_device::read), FUNC(x2210_device::write)).mirror(0x1c0);
	map(0xf3200, 0xf3200).lr8(NAME([this](){ m_x2210a->recall(1); m_x2210a->recall(0); return 0; }));
	map(0xf3200, 0xf3200).lw8(NAME([this](uint8_t d){ m_x2210a->store(1); m_x2210a->store(0); }));
	map(0xfa000, 0xfbfff).rom().region("slotd", 0);
	map(0xfc000, 0xfdfff).rom().region("slote", 0);
	map(0xfe000, 0xfffff).rom().region("slotf", 0);
}

void chameleon_state::z80_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).lr8(NAME([this](offs_t o) { return m_8088_io->read_byte(o); }));
	map(0x0000, 0xffff).lw8(NAME([this](offs_t o, uint8_t d) { m_8088_io->write_byte(o, d); }));
}

void chameleon_state::z80_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).lr8(NAME([this](offs_t o) { return m_8088_mem->read_byte(o | m_z80_bank); }));
	map(0x0000, 0xffff).lw8(NAME([this](offs_t o, uint8_t d) { m_8088_mem->write_byte(o | m_z80_bank, d); }));
}

void chameleon_devices(device_slot_interface &device)
{
	device.option_add_internal("cga", ISA8_CGA_CHAMELEON);
	device.option_add_internal("fdc", ISA8_FDC_XT);
}

void chameleon_state::chameleon(machine_config &config)
{
	I8088(config, m_i8088, 14.7428_MHz_XTAL / 3); //freq?
	m_i8088->set_addrmap(AS_PROGRAM, &chameleon_state::i8088_map);
	m_i8088->set_addrmap(AS_IO, &chameleon_state::i8088_io);
	m_i8088->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	Z80(config, m_z80, 14.7428_MHz_XTAL / 4); // freq?
	m_z80->set_addrmap(AS_PROGRAM, &chameleon_state::z80_map);
	m_z80->set_addrmap(AS_IO, &chameleon_state::z80_io);

	PCNOPPI_MOTHERBOARD(config, m_mb).set_cputag(m_i8088);
	m_mb->set_cputag(m_i8088);
	m_mb->int_callback().set_inputline(m_i8088, 0);
	m_mb->nmi_callback().set_inputline(m_i8088, INPUT_LINE_NMI);

	ISA8_SLOT(config, "dev0", 0, "mb:isa", chameleon_devices, "cga", true);
	ISA8_SLOT(config, "dev1", 0, "mb:isa", chameleon_devices, "fdc", true);

	I8274(config, m_i8274, 16_MHz_XTAL); // freq?

	X2210(config, m_x2210a);

	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set("mb:pic8259", FUNC(pic8259_device::ir7_w));

	// correct keyboard dump needed
	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "keyboard", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83));
	pc_kbdc.out_clock_cb().set(m_mb, FUNC(pc_noppi_mb_device::keyboard_clock_w));
	pc_kbdc.out_data_cb().set(m_mb, FUNC(pc_noppi_mb_device::keyboard_data_w));

	RAM(config, RAM_TAG).set_default_size("256K").set_extra_options("128K, 512K");
}

ROM_START(chameleon)
	ROM_DEFAULT_BIOS("220")
	ROM_SYSTEM_BIOS(0, "216", "2.16")
	ROM_SYSTEM_BIOS(1, "214", "2.14")
	ROM_SYSTEM_BIOS(2, "220", "2.20")
	ROM_REGION(0x2000, "slotf", 0)
	ROMX_LOAD("seequabios-2-16.bin", 0, 0x2000, CRC(b0516c0c) SHA1(c9c83d8b6649d8262e609967cbedb1b89ea1667a), ROM_BIOS(0))
	ROMX_LOAD("seequa2.14-bios.bin", 0, 0x2000, CRC(ddec9f17) SHA1(8ccb2c3abce550740594bff2e3d0a86c34748465), ROM_BIOS(1))
	ROMX_LOAD("chameleonbios-16.bin", 0, 0x2000, CRC(a28b042d) SHA1(e8b4ef47e08f5734f852b3a43baf0f52dac28257), ROM_BIOS(2))
	ROM_REGION(0x2000, "slote", 0)
	ROMX_LOAD("seequafont-2-16.bin", 0, 0x2000, CRC(6cbefe69) SHA1(ee94959050f371d4dd433a5f2e750f25b6f1a238), ROM_BIOS(0))
	ROMX_LOAD("seequa2.14-font.bin", 0, 0x2000, CRC(05a08abb) SHA1(c30a11be3e3527acaeb932388dc46ebd1ac9056e), ROM_BIOS(1))
	ROMX_LOAD("seequabios-2.20.bin", 0, 0x2000, CRC(f34e79c4) SHA1(cea7083513030e01d1f6c9e56df4e72ea5902523), ROM_BIOS(2))
	ROM_REGION(0x2000, "slotd", 0)
	ROMX_FILL(0x0000, 0x2000, 0xff, ROM_BIOS(0))
	ROMX_FILL(0x0000, 0x2000, 0xff, ROM_BIOS(1))
	ROMX_FILL(0x0000, 0x2000, 0xff, ROM_BIOS(2))
	ROM_REGION(0x40, "x2210a", 0)  // configured for 256k ram
	ROM_LOAD("nvram", 0, 0x40, CRC(ed73313b) SHA1(fa643bf008b937ea9bc2c3e99523d9ed0f92acc4))
ROM_END
}

COMP(1983, chameleon, 0, 0, chameleon, 0, chameleon_state, empty_init, "Seequa", "Chameleon (Seequa PC)", MACHINE_NOT_WORKING)
