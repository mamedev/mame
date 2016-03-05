// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#include "emu.h"
#include "machine/genpc.h"
#include "bus/pc_kbd/keyboards.h"
#include "machine/pc_fdc.h"
#include "formats/asst128_dsk.h"

class asst128_mb_device : public ibm5150_mb_device
{
public:
	// construction/destruction
	asst128_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: ibm5150_mb_device(mconfig, tag, owner, clock) { }

	DECLARE_ADDRESS_MAP(map, 8);
};

DEVICE_ADDRESS_MAP_START( map, 8, asst128_mb_device )
	AM_RANGE(0x0020, 0x002f) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE(0x0040, 0x004f) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0x0060, 0x006f) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x0080, 0x008f) AM_WRITE(pc_page_w)
	AM_RANGE(0x00a0, 0x00a1) AM_WRITE(nmi_enable_w)
ADDRESS_MAP_END

const device_type ASST128_MOTHERBOARD = &device_creator<asst128_mb_device>;

class asst128_state : public driver_device
{
public:
	// construction/destruction
	asst128_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pc_fdc_xt_device> m_fdc;

	DECLARE_FLOPPY_FORMATS( asst128_formats );
	DECLARE_WRITE8_MEMBER(asst128_fdc_dor_w);

	void machine_start() override;
};

void asst128_state::machine_start()
{
	memory_region* font = memregion(":board0:cga_mc1502:gfx1");
	memcpy(font->base(), memregion("bios")->base()+0xfa6e, 0x0400);
	memcpy(font->base()+0x0400, memregion("bios")->base()+0x4000, 0x0400);
}

WRITE8_MEMBER(asst128_state::asst128_fdc_dor_w)
{
	m_fdc->tc_w((data & 0x80) == 0x80);
	m_fdc->dor_w(space, offset, data, mem_mask);
}

static ADDRESS_MAP_START( asst128_map, AS_PROGRAM, 16, asst128_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(asst128_io, AS_IO, 16, asst128_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE8("mb", asst128_mb_device, map, 0xffff)
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

	MCFG_DEVICE_MODIFY("mb:cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_ISA8_SLOT_ADD("mb:isa", "board0", pc_isa8_cards, "cga_mc1502", true)
	MCFG_ISA8_SLOT_ADD("mb:isa", "board1", pc_isa8_cards, "lpt", true)

	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83)

	MCFG_PC_FDC_XT_ADD("fdc")
	MCFG_PC_FDC_INTRQ_CALLBACK(DEVWRITELINE("mb:pic8259", pic8259_device, ir6_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", asst128_floppies, "525ssqd", asst128_state::asst128_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", asst128_floppies, "525ssqd", asst128_state::asst128_formats)

	MCFG_PC_JOY_ADD("pc_joy")

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
	MCFG_RAM_EXTRA_OPTIONS("64K, 128K, 256K")
MACHINE_CONFIG_END

ROM_START( asst128 )
	ROM_REGION16_LE(0x10000,"bios", 0)
	ROM_LOAD( "extbios.bin",      0x4000, 0x2000, CRC(e3bf22de) SHA1(d4319edc82c0015ca0adc6c8771e887659717e62))
	ROM_LOAD( "basic.bin",        0x6000, 0x8000, CRC(a4ec66f6) SHA1(80e934986022681ccde180e92aa108e716c4f19b))
	ROM_LOAD( "mainbios.bin",     0xe000, 0x2000, CRC(8426cbf5) SHA1(41d14137ffa651977041da22aa8071c0f7854158))

	// XXX needs dumping
	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	ROM_LOAD( "asst128cg.bin", 0, 0x2000, NO_DUMP )
ROM_END

/*    YEAR  NAME        PARENT      COMPAT      MACHINE     INPUT       INIT        COMPANY            FULLNAME */
COMP( 198?, asst128,    ibm5150,    0,          asst128,    0,      driver_device, 0,   "Schetmash", "Assistent 128", MACHINE_NOT_WORKING)
