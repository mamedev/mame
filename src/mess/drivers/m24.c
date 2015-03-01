#include "emu.h"

#include "cpu/i86/i86.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "machine/pckeybrd.h"
#include "includes/genpc.h"

class m24_state : public driver_device
{
public:
	m24_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb")
	{ }
	required_device<cpu_device> m_maincpu;
	required_device<pc_noppi_mb_device> m_mb;

	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(keyboard_w);
};

READ8_MEMBER(m24_state::keyboard_r)
{
	return 0;
}

WRITE8_MEMBER(m24_state::keyboard_w)
{

}

static ADDRESS_MAP_START( m24_map, AS_PROGRAM, 16, m24_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xeffff) AM_NOP
	AM_RANGE(0xf8000, 0xfffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(m24_io, AS_IO, 16, m24_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0060, 0x0065) AM_READWRITE8(keyboard_r, keyboard_w, 0xffff)
	AM_RANGE(0x0066, 0x0067) AM_READ_PORT("DSW0")
ADDRESS_MAP_END

static INPUT_PORTS_START( m24 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x8f, 0x89, "RAM banks")
	PORT_DIPSETTING(    0x01, "128K" )
	PORT_DIPSETTING(    0x82, "256K" )
	PORT_DIPSETTING(    0x84, "512K - 256/256" )
	PORT_DIPSETTING(    0x08, "512K - 512/0" )
	PORT_DIPSETTING(    0x85, "640K - 256/384" )
	PORT_DIPSETTING(    0x8d, "640K - 128/512" )
	PORT_DIPSETTING(    0x89, "640K - 512/128" )
	PORT_DIPNAME( 0x10, 0x00, "8087 installed")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x10, DEF_STR(Yes) )
	PORT_DIPNAME( 0x20, 0x00, "Serial Port")
	PORT_DIPSETTING(    0x20, "Z8530 SCC")
	PORT_DIPSETTING(    0x00, "INS8250" )

	//PORT_START("DSW1")
	PORT_DIPNAME( 0x0100, 0x0000, "FDD Type")
	PORT_DIPSETTING(    0x0000, "360K" )
	PORT_DIPSETTING(    0x0100, "720K" )
	PORT_DIPNAME( 0x0200, 0x0200, "FDD spinup")
	PORT_DIPSETTING(    0x0000, "Slow" )
	PORT_DIPSETTING(    0x0200, "Fast" )
	PORT_DIPNAME( 0x0400, 0x0400, "HDD ROM")
	PORT_DIPSETTING(    0x0000, "Internal" )
	PORT_DIPSETTING(    0x0400, "External" )
	PORT_DIPNAME( 0x0800, 0x0000, "Scroll rate")
	PORT_DIPSETTING(    0x0800, "Slow" )
	PORT_DIPSETTING(    0x0000, "Fast")
	PORT_DIPNAME( 0x3000, 0x2000, "Graphics adapter")
	PORT_DIPSETTING(    0x0000, "EGA/VGA" )
	PORT_DIPSETTING(    0x1000, "Color 40x25" )
	PORT_DIPSETTING(    0x2000, "Color 80x25" )
	PORT_DIPSETTING(    0x3000, "Monochrome" )
	PORT_DIPNAME( 0xc000, 0x4000, "Number of floppy drives")
	PORT_DIPSETTING(    0x0000, "1" )
	PORT_DIPSETTING(    0x4000, "2" )
	PORT_DIPSETTING(    0x8000, "3" )
	PORT_DIPSETTING(    0xc000, "4" )

	PORT_INCLUDE(pc_keyboard)
INPUT_PORTS_END

static MACHINE_CONFIG_START( olivetti, m24_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(m24_map)
	MCFG_CPU_IO_MAP(m24_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_PCNOPPI_MOTHERBOARD_ADD("mb", "maincpu")

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "cga", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "lpt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "com", false)

	/* keyboard */
	MCFG_PC_KEYB_ADD("pc_keyboard", DEVWRITELINE("mb:pic8259", pic8259_device, ir1_w))
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5150")
MACHINE_CONFIG_END

ROM_START( m24 )
	ROM_REGION16_LE(0x8000,"maincpu", 0)
	ROMX_LOAD("olivetti_m24_version_1.43_high.bin",0x4001, 0x2000, CRC(04e697ba) SHA1(1066dcc849e6289b5ac6372c84a590e456d497a6), ROM_SKIP(1))
	ROMX_LOAD("olivetti_m24_version_1.43_low.bin", 0x4000, 0x2000, CRC(ff7e0f10) SHA1(13423011a9bae3f3193e8c199f98a496cab48c0f), ROM_SKIP(1))
ROM_END

ROM_START( m240 )
	ROM_REGION16_LE(0x8000,"maincpu", 0)
	ROMX_LOAD("olivetti_m240_pch5_2.04_high.bin", 0x0001, 0x4000, CRC(ceb97b59) SHA1(84fabbeab355e0a4c9445910f2b7d1ec98886642), ROM_SKIP(1))
	ROMX_LOAD("olivetti_m240_pch6_2.04_low.bin",  0x0000, 0x4000, CRC(c463aa94) SHA1(a30c763c1ace9f3ff79e7136b252d624108a50ae), ROM_SKIP(1))
ROM_END

COMP( 1983, m24,        ibm5150,    0,          olivetti,   m24, driver_device,      0,      "Olivetti", "M24", GAME_NOT_WORKING)
COMP( 1987, m240,       ibm5150,    0,          olivetti,   m24, driver_device,      0,      "Olivetti", "M240", GAME_NOT_WORKING)
