// license:BSD-3-Clause
// copyright-holders:Carl, rfka01

/*************************************************** Olivetti Prodest PC 1 ***

The PC 1 could be considered the Italian equivalent to the German Schneider Euro PC.
Form factor: home computer (keyboard, main unit and floppy drive in one case)
CPU: NEC V40 4.77/8MHz, software switchable, RAM: 256K/512K, expandable to 640K over the bus, ROM: 16K
Chipset: Video V6355D, M5L8255AP-5, Floppy: WD37C65B-PL, Keyboard: MBL8042H
OSC: 21.477270,  16.000
Keyboard: 83 keys, 10 function keys
Mass storage: 1 or 2 3.5" DS/DD floppy disk drive(s) 720K, external 5.25" DS/DD drive (D:) instead of the internal drive B: possible
There's a switch that activates the external drive chain. Drives are assigned their parameters via the DOS mechanism (DRIVPARM/DRIVER.SYS)
Video: CGA, with two monitor connectors (RGB/CGA and Ext. Video/SCART)
Mouse: Cursor emulation, on board: parallel, serial, speaker, audio connector("HIFI-Sound"), IBM compatible expansion bus, external 360K floppy drive
Options (from the manual): "BOX with a single slot of medium-small dimensions" which could house RAM, LAN, Modem, EGA, 3.5" harddisk,
Music Box, TV/teletext adapter

Olivetti Prodest PC 1 HD:  Different motherboard
http://www.ti99iuc.it/web/_upload/image/PC1-OnePage/Docs/GuidaCFModPC1HD/pc1CF-IDE-Guide.pdf
- on board XTA (8-bit IDE) port that was able to connect to either a Conner CP4024XT oder CP4044XT harddisk.
- 768K RAM of which 640K are accessible
Chipset: Video: V6355D, Floppy: WD37C65JM, VLSI ???43559, WD8250NJM, AMD N8255A-5, Keyboard: MBL8042H
OSC: 16.000, 21.477270, 1.8432

*****************************************************************************/

#include "emu.h"
#include "cpu/nec/v5x.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/isa/fdc.h"
#include "machine/pckeybrd.h"
#include "softlist_dev.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/bankdev.h"
#include "imagedev/floppy.h"
#include "sound/spkrdev.h"
#include "speaker.h"


namespace {

class olivpc1_state : public driver_device
{
public:
	olivpc1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_isa(*this, "isa"),
		m_speaker(*this, "speaker"),
		m_keyboard(*this, "pc_keyboard"),
		m_fdc(*this, "fdc"),
		m_bank(*this, "bank")
	{ }
	void pc8_io(address_map &map) ATTR_COLD;
	void pc8_map(address_map &map) ATTR_COLD;
	void bank(address_map &map) ATTR_COLD;
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	void olivpc1(machine_config &config);
private:
	u8 port6x_r(offs_t addr);
	void port6x_w(offs_t addr, u8 data);
	void nmi();
	required_device<v40_device> m_maincpu;
	required_device<isa8_device> m_isa;
	required_device<speaker_sound_device> m_speaker;
	required_device<pc_keyboard_device> m_keyboard;
	required_device<wd37c65c_device> m_fdc;
	required_device<address_map_bank_device> m_bank;
	bool m_obf = false;
};

void olivpc1_state::machine_start()
{
	ram_device *ram = subdevice<ram_device>(RAM_TAG);
	m_bank->space(AS_PROGRAM).install_ram(0, 0x4000 - 1, ram->pointer());
	m_maincpu->space(AS_PROGRAM).install_ram(0x4000, ram->size() - 1, ram->pointer() + 0x4000);
}

void olivpc1_state::machine_reset()
{
	m_keyboard->enable(1);
	m_bank->set_bank(0);
}

void olivpc1_state::pc8_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x03fff).m(m_bank, FUNC(address_map_bank_device::amap8));
	map(0xfc000, 0xfffff).rom().region("bios", 0);
}

void olivpc1_state::bank(address_map &map)
{
	map.unmap_value_high();
	map(0x4000, 0x7fff).rom().region("bios", 0);
}

void olivpc1_state::pc8_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).lrw8(NAME([this]() { nmi(); return 0xff; }), NAME([this](u8 d) { nmi(); }));
	map(0x0060, 0x0067).rw(FUNC(olivpc1_state::port6x_r), FUNC(olivpc1_state::port6x_w));
	map(0x03f2, 0x03f2).w("fdc", FUNC(wd37c65c_device::dor_w));
	map(0x03f4, 0x03f5).m("fdc", FUNC(wd37c65c_device::map));
}

static INPUT_PORTS_START(olivpc1)
INPUT_PORTS_END

// TODO: replace with 8042 dump
u8 olivpc1_state::port6x_r(offs_t addr)
{
	switch(addr)
	{
		case 0:
			m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
			m_obf = false;
			return m_keyboard->read();
		case 4:
			return m_obf;
	}
	return 0xff;
}

void olivpc1_state::port6x_w(offs_t addr, u8 data)
{
	switch(addr)
	{
		case 7:
			//m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			//m_bank->set_bank(0);
			break;
	}
}

void olivpc1_state::nmi()
{
	//m_bank->set_bank(1);
	//m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

static void pc1_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("525dd", FLOPPY_525_DD);
}

void olivpc1_state::olivpc1(machine_config &config)
{
	V40(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &olivpc1_state::pc8_map);
	m_maincpu->set_addrmap(AS_IO, &olivpc1_state::pc8_io);
	m_maincpu->set_tclk(14.318181_MHz_XTAL / 12);
	m_maincpu->out_hreq_cb().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_maincpu->out_hreq_cb().append(m_maincpu, FUNC(v40_device::hack_w));
	m_maincpu->out_eop_cb().set([this](int state) { m_fdc->tc_w(state); });
	m_maincpu->in_memr_cb().set([this](offs_t offset) { return m_maincpu->space(AS_PROGRAM).read_byte(offset); });
	m_maincpu->out_memw_cb().set([this](offs_t offset, u8 data) { m_maincpu->space(AS_PROGRAM).write_byte(offset, data); });
	m_maincpu->in_ior_cb<0>().set([this]() { return m_isa->dack_r(0); });
	m_maincpu->out_iow_cb<0>().set([this](u8 data) { m_isa->dack_w(0, data); });
	m_maincpu->in_ior_cb<1>().set("fdc", FUNC(wd37c65c_device::dma_r));
	m_maincpu->out_iow_cb<1>().set("fdc", FUNC(wd37c65c_device::dma_w));
	m_maincpu->in_ior_cb<2>().set([this]() { return m_isa->dack_r(2); });
	m_maincpu->out_iow_cb<2>().set([this](u8 data) { m_isa->dack_w(2, data); });

	ADDRESS_MAP_BANK(config, m_bank).set_map(&olivpc1_state::bank).set_options(ENDIANNESS_LITTLE, 8, 15, 0x4000);

	WD37C65C(config, m_fdc, 16_MHz_XTAL);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ6);
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(v40_device::dreq_w<1>));
	FLOPPY_CONNECTOR(config, "fdc:0", pc1_floppies, "35dd", isa8_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc1_floppies, nullptr, isa8_fdc_device::floppy_formats).enable_sound(true);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.5);

	PC_KEYB(config, m_keyboard);
	m_keyboard->keypress().set([this](int state) { m_obf = state; m_maincpu->set_input_line(INPUT_LINE_IRQ1, state); });

	isa8_device &isa(ISA8(config, "isa", 0));
	isa.set_memspace("maincpu", AS_PROGRAM);
	isa.set_iospace("maincpu", AS_IO);
	isa.drq1_callback().set(m_maincpu, FUNC(v40_device::dreq_w<0>));
	isa.drq3_callback().set(m_maincpu, FUNC(v40_device::dreq_w<2>));
	isa.irq2_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ2);
	isa.irq3_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ3);
	isa.irq4_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ4);
	isa.irq5_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ5);
	isa.iochck_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	ISA8_SLOT(config, "intcga", 0, isa, pc_isa8_cards, "cga", true);
	ISA8_SLOT(config, "isa1", 0, isa, pc_isa8_cards, nullptr, false);
	RAM(config, RAM_TAG).set_default_size("512K").set_extra_options("128K, 256K, 384K");

	SOFTWARE_LIST(config, "pc_list").set_compatible("ibm5150");
}

ROM_START( olivpc1 )
	ROM_REGION(0x4000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "rev121", "Rev. 1.21")
	ROMX_LOAD("pc1_bios_1_21.bin", 0x0000, 0x4000, CRC(3c44cdbf) SHA1(46e6c8531ad1fe81cf4457f1edb7554a0eaed7e8), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "rev106", "Rev. 1.06 20/08/87")
	ROMX_LOAD("pc1_bios_1_06.bin", 0x0000, 0x4000, CRC(679d2235) SHA1(621d0abafcc1ee7edc090c01ca496f056c184410), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "rev107", "Rev. 1.07 12/10/88")
	ROMX_LOAD("pc1_bios_1_07.bin", 0x0000, 0x4000, CRC(3b84757c) SHA1(a2288ecc7616767d3ea5dd3262bfad98cf1b39d3), ROM_BIOS(2))

	ROM_REGION(0x2000, "gfx1", 0)
	ROM_LOAD("xu4600_pc1_prodest_font101.bin", 0x0000, 0x2000, CRC(5c21981e) SHA1(d0db791079ece9c9e50bb7f38b5b11024ba7ec99))
ROM_END

} // anonymous namespace


COMP( 1987, olivpc1,        ibm5150, 0,      olivpc1,        olivpc1,    olivpc1_state, empty_init,      "Olivetti",                        "Prodest PC 1",          MACHINE_NOT_WORKING )
