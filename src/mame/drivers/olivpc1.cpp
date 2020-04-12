// license:BSD-3-Clause
// copyright-holders:Carl, rfka01

/*************************************************** Olivetti Prodest PC 1 ***
*****************************************************************************/

#include "emu.h"
#include "cpu/nec/v5x.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/isa/fdc.h"
#include "machine/pckeybrd.h"
#include "softlist.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "imagedev/floppy.h"
#include "sound/spkrdev.h"
#include "speaker.h"

class olivpc1_state : public driver_device
{
public:
	olivpc1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_isa(*this, "isa"),
		m_speaker(*this, "speaker"),
		m_keyboard(*this, "pc_keyboard"),
		m_fdc(*this, "fdc")
	{ }
	void pc8_io(address_map &map);
	void pc8_map(address_map &map);
	void machine_start() override;
	void olivpc1(machine_config &config);
private:
	u8 port6x_r(offs_t addr);
	void port6x_w(offs_t addr, u8 data);
	required_device<v40_device> m_maincpu;
	required_device<isa8_device> m_isa;
	required_device<speaker_sound_device> m_speaker;
	required_device<pc_keyboard_device> m_keyboard;
	required_device<wd37c65c_device> m_fdc;
	u8 m_port61;
	bool m_speaker_data;
};

void olivpc1_state::machine_start()
{
	ram_device *ram = subdevice<ram_device>(RAM_TAG);
	m_maincpu->space(AS_PROGRAM).install_ram(0, ram->size() - 1, ram->pointer());
	save_item(NAME(m_port61));
	save_item(NAME(m_speaker_data));
}

void olivpc1_state::pc8_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void olivpc1_state::pc8_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0060, 0x0067).rw(FUNC(olivpc1_state::port6x_r), FUNC(olivpc1_state::port6x_w));
	map(0x03f2, 0x03f2).w("fdc", FUNC(wd37c65c_device::dor_w));
	map(0x03f4, 0x03f5).m("fdc", FUNC(wd37c65c_device::map));
}

static INPUT_PORTS_START(olivpc1)
	PORT_INCLUDE(pc_keyboard)
INPUT_PORTS_END

// TODO: replace with 8042 dump
u8 olivpc1_state::port6x_r(offs_t addr)
{
	switch(addr)
	{
		case 0:
			return m_keyboard->read();
		case 1:
			return m_port61;
		case 4:
			return 0;
	}
	return 0xff;
}

void olivpc1_state::port6x_w(offs_t addr, u8 data)
{
	switch(addr)
	{
		case 1:
			if (BIT(m_port61, 1) && !BIT(data, 1))
				m_speaker->level_w(0);
			else if (!BIT(m_port61, 1) && BIT(data, 1))
				m_speaker->level_w(m_speaker_data);
			m_maincpu->tctl2_w(BIT(data, 0));
			if(data & 0x80)
				m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);

			m_port61 = data;
			break;
	}
}

static void pc1_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
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

	WD37C65C(config, m_fdc, 16_MHz_XTAL);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ6);
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(v40_device::dreq_w<1>));
	FLOPPY_CONNECTOR(config, "fdc:0", pc1_floppies, "35dd", isa8_fdc_device::floppy_formats).enable_sound(true);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.5);

	PC_KEYB(config, m_keyboard);
	m_keyboard->keypress().set_inputline(m_maincpu, INPUT_LINE_IRQ1);

	isa8_device &isa(ISA8(config, "isa", 0));
	isa.set_memspace("maincpu", AS_PROGRAM);
	isa.set_iospace("maincpu", AS_IO);
	isa.drq1_callback().set(m_maincpu, FUNC(v40_device::dreq_w<0>));
	isa.drq3_callback().set(m_maincpu, FUNC(v40_device::dreq_w<2>));
	isa.irq2_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ2);
	isa.irq3_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ3);
	isa.irq4_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ4);
	isa.irq5_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ5);

	ISA8_SLOT(config, "intcga", 0, isa, pc_isa8_cards, "cga", true);
	ISA8_SLOT(config, "isa1", 0, isa, pc_isa8_cards, nullptr, false);
	RAM(config, RAM_TAG).set_default_size("512K").set_extra_options("128K, 256K, 384K");

	SOFTWARE_LIST(config, "pc_list").set_compatible("ibm5150");
}

ROM_START( olivpc1 )
	ROM_REGION(0x10000, "bios", 0)
	ROM_LOAD("pc1_bios_1_21.bin", 0xc000, 0x4000, CRC(3c44cdbf) SHA1(46e6c8531ad1fe81cf4457f1edb7554a0eaed7e8))

	ROM_REGION(0x2000, "gfx1", 0)
	ROM_LOAD("xu4600_pc1_prodest_font101.bin", 0x0000, 0x2000, CRC(5c21981e) SHA1(d0db791079ece9c9e50bb7f38b5b11024ba7ec99))
ROM_END

COMP( 1987, olivpc1,        ibm5150, 0,      olivpc1,        olivpc1,    olivpc1_state, empty_init,      "Olivetti",                        "Prodest PC 1",          MACHINE_NOT_WORKING )
