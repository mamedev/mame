// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

#include "emu.h"
#include "machine/genpc.h"

#include "bus/pc_joy/pc_joy.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i86/i86.h"
#include "imagedev/floppy.h"
#include "machine/pc_fdc.h"

#include "formats/asst128_dsk.h"


DECLARE_DEVICE_TYPE(ASST128_MOTHERBOARD, asst128_mb_device)

class asst128_mb_device : public ibm5150_mb_device
{
public:
	// construction/destruction
	asst128_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0)
		: ibm5150_mb_device(mconfig, ASST128_MOTHERBOARD, tag, owner, clock)
	{ }

	void map(address_map &map);
};

void asst128_mb_device::map(address_map &map)
{
	map(0x0020, 0x002f).rw("pic8259", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x004f).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0060, 0x006f).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0080, 0x008f).w(FUNC(asst128_mb_device::pc_page_w));
	map(0x00a0, 0x00a1).w(FUNC(asst128_mb_device::nmi_enable_w));
}

DEFINE_DEVICE_TYPE(ASST128_MOTHERBOARD, asst128_mb_device, "asst128_mb", "ASST128_MOTHERBOARD")

class asst128_state : public driver_device
{
public:
	// construction/destruction
	asst128_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
	{ }

	void asst128(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<pc_fdc_xt_device> m_fdc;

	static void asst128_formats(format_registration &fr);
	void asst128_fdc_dor_w(uint8_t data);

	void machine_start() override;
	void asst128_io(address_map &map);
	void asst128_map(address_map &map);
};

void asst128_state::machine_start()
{
	memory_region *font = memregion(":board0:cga_mc1502:gfx1");
	memcpy(font->base(), memregion("bios")->base() + 0xfa6e, 0x0400);
	memcpy(font->base() + 0x0400, memregion("bios")->base() + 0x4000, 0x0400);
}

void asst128_state::asst128_fdc_dor_w(uint8_t data)
{
	m_fdc->tc_w((data & 0x80) == 0x80);
	m_fdc->dor_w(data);
}

void asst128_state::asst128_map(address_map &map)
{
	map.unmap_value_high();
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void asst128_state::asst128_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m("mb", FUNC(asst128_mb_device::map));
	map(0x0200, 0x0207).rw("pc_joy", FUNC(pc_joy_device::joy_port_r), FUNC(pc_joy_device::joy_port_w));
	map(0x03f2, 0x03f3).w(FUNC(asst128_state::asst128_fdc_dor_w));
	map(0x03f4, 0x03f5).m("fdc:upd765", FUNC(upd765a_device::map));
}

static void asst128_floppies(device_slot_interface &device)
{
	device.option_add("525ssqd", FLOPPY_525_SSQD);
}

void asst128_state::asst128_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ASST128_FORMAT);
}

static DEVICE_INPUT_DEFAULTS_START( asst128 )
	DEVICE_INPUT_DEFAULTS("DSW0", 0x30, 0x20)
DEVICE_INPUT_DEFAULTS_END

void asst128_state::asst128(machine_config &config)
{
	I8086(config, m_maincpu, 4772720);
	m_maincpu->set_addrmap(AS_PROGRAM, &asst128_state::asst128_map);
	m_maincpu->set_addrmap(AS_IO, &asst128_state::asst128_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	asst128_mb_device &mb(ASST128_MOTHERBOARD(config, "mb"));
	mb.set_cputag(m_maincpu);
	mb.int_callback().set_inputline(m_maincpu, 0);
	mb.nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	mb.kbdclk_callback().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	mb.kbddata_callback().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));
	mb.set_input_default(DEVICE_INPUT_DEFAULTS_NAME(asst128));

	subdevice<cassette_image_device>("mb:cassette")->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);

	// FIXME: determine ISA bus clock
	ISA8_SLOT(config, "board0", 0, "mb:isa", pc_isa8_cards, "cga_mc1502", true);
	ISA8_SLOT(config, "board1", 0, "mb:isa", pc_isa8_cards, "lpt", true);

	pc_kbdc_device &kbd(PC_KBDC(config, "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83));
	kbd.out_clock_cb().set("mb", FUNC(asst128_mb_device::keyboard_clock_w));
	kbd.out_data_cb().set("mb", FUNC(asst128_mb_device::keyboard_data_w));

	PC_FDC_XT(config, m_fdc, 0);
	m_fdc->intrq_wr_callback().set("mb:pic8259", FUNC(pic8259_device::ir6_w));
	FLOPPY_CONNECTOR(config, "fdc:0", asst128_floppies, "525ssqd", asst128_state::asst128_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", asst128_floppies, "525ssqd", asst128_state::asst128_formats);

	PC_JOY(config, "pc_joy");

	RAM(config, RAM_TAG).set_default_size("512K").set_extra_options("64K, 128K, 256K");
}

ROM_START( asst128 )
	ROM_REGION16_LE(0x10000,"bios", 0)
	ROM_LOAD( "extbios.bin",      0x4000, 0x2000, CRC(e3bf22de) SHA1(d4319edc82c0015ca0adc6c8771e887659717e62))
	ROM_LOAD( "basic.bin",        0x6000, 0x8000, CRC(a4ec66f6) SHA1(80e934986022681ccde180e92aa108e716c4f19b))
	ROM_LOAD( "mainbios.bin",     0xe000, 0x2000, CRC(8426cbf5) SHA1(41d14137ffa651977041da22aa8071c0f7854158))

	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	ROM_LOAD( "asst128cg.bin", 0, 0x2000, NO_DUMP )
ROM_END

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT  CLASS          INIT        COMPANY      FULLNAME         FLAGS
COMP( 198?, asst128, ibm5150, 0,      asst128, 0,     asst128_state, empty_init, "Schetmash", "Assistent 128", MACHINE_NOT_WORKING)
