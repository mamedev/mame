// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#include "emu.h"
#include "includes/genpc.h"
#include "machine/pc_fdc.h"
#include "cpu/i86/i86.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "formats/asst128_dsk.h"
#include "bus/pc_kbd/keyboards.h"

class asst128_mb_device : public ibm5150_mb_device
{
public:
	// construction/destruction
	asst128_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: ibm5150_mb_device(mconfig, tag, owner, clock) { }

protected:
	// device-level overrides
	void device_start();
};

void asst128_mb_device::device_start()
{
	install_device(0x0020, 0x0021, 0, 0, read8_delegate(FUNC(pic8259_device::read), (pic8259_device*)m_pic8259), write8_delegate(FUNC(pic8259_device::write), (pic8259_device*)m_pic8259) );
	install_device(0x0040, 0x0043, 0, 0, read8_delegate(FUNC(pit8253_device::read), (pit8253_device*)m_pit8253), write8_delegate(FUNC(pit8253_device::write), (pit8253_device*)m_pit8253) );
	install_device(0x0060, 0x0063, 0, 0, read8_delegate(FUNC(i8255_device::read),   (i8255_device*)m_ppi8255),   write8_delegate(FUNC(i8255_device::write),   (i8255_device*)m_ppi8255)   );
	install_device(0x0080, 0x0087, 0, 0, read8_delegate(FUNC(ibm5160_mb_device::pc_page_r), this), write8_delegate(FUNC(ibm5160_mb_device::pc_page_w),this) );
	install_device(0x00a0, 0x00a1, 0, 0, read8_delegate(), write8_delegate(FUNC(ibm5160_mb_device::nmi_enable_w),this));
	/* MESS managed RAM */
	if ( m_ram->pointer() )
		membank( "bank10" )->set_base( m_ram->pointer() );
}

const device_type ASST128_MOTHERBOARD = &device_creator<asst128_mb_device>;

class asst128_state : public driver_device
{
public:
	// construction/destruction
	asst128_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mb(*this, "mb")
		, m_fdc(*this, "fdc")
	{ }

	required_device<cpu_device>  m_maincpu;
	required_device<asst128_mb_device>  m_mb;
	required_device<pc_fdc_xt_device> m_fdc;

	DECLARE_FLOPPY_FORMATS( asst128_formats );
	DECLARE_WRITE8_MEMBER(asst128_fdc_dor_w);

	void machine_start();
};

void asst128_state::machine_start()
{
	memory_region* font = memregion(":isa_cga:cga_mc1502:gfx1");
	memcpy(font->base(), memregion("gfx1")->base(), 0x2000);
}

WRITE8_MEMBER(asst128_state::asst128_fdc_dor_w)
{
	m_fdc->tc_w((data & 0x80) == 0x80);
	m_fdc->dor_w(space, offset, data, mem_mask);
}

static ADDRESS_MAP_START( asst128_map, AS_PROGRAM, 16, asst128_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x7ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(asst128_io, AS_IO, 16, asst128_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0200, 0x0207) AM_DEVREADWRITE8("pc_joy", pc_joy_device, joy_port_r, joy_port_w, 0xffff)
	AM_RANGE(0x03f2, 0x03f3) AM_WRITE8(asst128_fdc_dor_w, 0xffff)
	AM_RANGE(0x03f4, 0x03f5) AM_DEVICE8("fdc:upd765", upd765a_device, map, 0xffff)
ADDRESS_MAP_END

static SLOT_INTERFACE_START( asst128_floppies )
	SLOT_INTERFACE( "525ssqd", FLOPPY_525_SSQD )
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( asst128_state::asst128_formats )
	FLOPPY_ASST128_FORMAT
FLOPPY_FORMATS_END

static DEVICE_INPUT_DEFAULTS_START( asst128 )
	DEVICE_INPUT_DEFAULTS("DSW0", 0x30, 0x20)
DEVICE_INPUT_DEFAULTS_END

static MACHINE_CONFIG_START( asst128, asst128_state )
	MCFG_CPU_ADD("maincpu", I8086, 4772720)
	MCFG_CPU_PROGRAM_MAP(asst128_map)
	MCFG_CPU_IO_MAP(asst128_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("mb", ASST128_MOTHERBOARD, 0)
	asst128_mb_device::static_set_cputag(*device, "maincpu");
	MCFG_DEVICE_INPUT_DEFAULTS(asst128)

	MCFG_DEVICE_REMOVE("mb:cassette")
	MCFG_CASSETTE_ADD("mb:cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa_cga", pc_isa8_cards, "cga_mc1502", true)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa_lpt", pc_isa8_cards, "lpt", true)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa_com", pc_isa8_cards, "com", true)

	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83)

	MCFG_PC_FDC_XT_ADD("fdc")
	MCFG_PC_FDC_INTRQ_CALLBACK(DEVWRITELINE("mb:pic8259", pic8259_device, ir6_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", asst128_floppies, "525ssqd", asst128_state::asst128_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", asst128_floppies, "525ssqd", asst128_state::asst128_formats)

	MCFG_PC_JOY_ADD("pc_joy")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

ROM_START( asst128 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	ROM_DEFAULT_BIOS("floppy")
	/* BASIC ROM taken from IBM 5150 and needs dumping */
	ROM_LOAD( "basic-1.10.rom",    0xf6000, 0x8000, CRC(ebacb791) SHA1(07449ebca18f979b9ab748582b736e402f2bf940))
	ROM_LOAD( "asf400-f600.bin",   0xf4000, 0x2000, CRC(e3bf22de) SHA1(d4319edc82c0015ca0adc6c8771e887659717e62))
	ROM_SYSTEM_BIOS(0, "floppy", "3rd party floppy support")
	ROMX_LOAD( "rombios7.bin",     0xfc001, 0x2000, CRC(7d7c8d6a) SHA1(a731a65ee547f1d78cfc91461f38166da014f3dc), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "rombios8.bin",     0xfc000, 0x2000, CRC(ba304663) SHA1(b2533b8f8240f72b7315f27c7b64f95ac52687ca), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "stock", "cassette-only BIOS?")
	ROMX_LOAD( "mainbios.bin",     0xfe000, 0x2000, CRC(8426cbf5) SHA1(41d14137ffa651977041da22aa8071c0f7854158), ROM_BIOS(2))
	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	ROM_COPY( "maincpu", 0xffa6e, 0x0000, 0x0400 )
	ROM_COPY( "maincpu", 0xfc000, 0x0400, 0x0400 )
ROM_END

/*    YEAR  NAME        PARENT      COMPAT      MACHINE     INPUT       INIT        COMPANY            FULLNAME */
COMP( 198?, asst128,    ibm5150,    0,          asst128,    0,      driver_device, 0,   "Schetmash", "Assistent 128", MACHINE_NOT_WORKING)
