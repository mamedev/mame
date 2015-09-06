// license:BSD-3-Clause
// copyright-holders:?
/***************************************************************************

    XBOX (c) 2001 Microsoft

    Skeleton driver

***************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/idectrl.h"
#include "machine/idehd.h"
#include "video/poly.h"
#include "bitmap.h"
#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "debug/debugcpu.h"
#include "includes/chihiro.h"
#include "includes/xbox.h"

#define CPU_DIV 64

class xbox_state : public xbox_base_state
{
public:
	xbox_state(const machine_config &mconfig, device_type type, const char *tag) :
		xbox_base_state(mconfig, type, tag),
		usbhack_index(-1),
		usbhack_counter(0)
	{ }

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();

	virtual void hack_eeprom();
	virtual void hack_usb();

	struct chihiro_devices {
		bus_master_ide_controller_device    *ide;
	} xbox_devs;
	int usbhack_index;
	int usbhack_counter;
};

void xbox_state::video_start()
{
}

static ADDRESS_MAP_START(xbox_map, AS_PROGRAM, 32, xbox_state)
	AM_IMPORT_FROM(xbox_base_map)
ADDRESS_MAP_END

static ADDRESS_MAP_START(xbox_map_io, AS_IO, 32, xbox_state)
	AM_IMPORT_FROM(xbox_base_map_io)
ADDRESS_MAP_END

static INPUT_PORTS_START( xbox )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void xbox_state::hack_eeprom()
{
	// 8003b744,3b744=0x90 0x90
	/*m_maincpu->space(0).write_byte(0x3b744, 0x90);
	m_maincpu->space(0).write_byte(0x3b745, 0x90);
	m_maincpu->space(0).write_byte(0x3b766, 0xc9);
	m_maincpu->space(0).write_byte(0x3b767, 0xc3);*/
}

/*static const struct {
    const char *game_name;
    struct {
        UINT32 address;
        UINT8 write_byte;
    } modify[16];
} hacks[] = { { "chihiro",{ { 0x6a79f, 0x01 },{ 0x6a7a0, 0x00 },{ 0x6b575, 0x00 },{ 0x6b576, 0x00 },{ 0x6b5af, 0x75 },{ 0x6b78a, 0x75 },{ 0x6b7ca, 0x00 },{ 0x6b7b8, 0x00 },{ 0x8f5b2, 0x75 },{ 0x79a9e, 0x74 },{ 0x79b80, 0x74 },{ 0x79b97, 0x74 },{ 0, 0 } } },
{ "outr2",{ { 0x12e4cf, 0x01 },{ 0x12e4d0, 0x00 },{ 0x4793e, 0x01 },{ 0x4793f, 0x00 },{ 0x47aa3, 0x01 },{ 0x47aa4, 0x00 },{ 0x14f2b6, 0x84 },{ 0x14f2d1, 0x75 },{ 0x8732f, 0x7d },{ 0x87384, 0x7d },{ 0x87388, 0xeb },{ 0, 0 } } } };*/

void xbox_state::hack_usb()
{
	int p;

	if (usbhack_counter == 0)
		p = 0;
	else if (usbhack_counter == 1) // after game loaded
		p = usbhack_index;
	else
		p = -1;
	if (p >= 0) {
		/*for (int a = 0; a < 16; a++) {
		    if (hacks[p].modify[a].address == 0)
		        break;
		    m_maincpu->space(0).write_byte(hacks[p].modify[a].address, hacks[p].modify[a].write_byte);
		}*/
	}
	usbhack_counter++;
}

void xbox_state::machine_start()
{
	xbox_base_state::machine_start();
	xbox_devs.ide = machine().device<bus_master_ide_controller_device>("ide");
	usbhack_index = -1;
	/*for (int a = 1; a < 2; a++)
	    if (strcmp(machine().basename(), hacks[a].game_name) == 0) {
	        usbhack_index = a;
	        break;
	    }*/
	usbhack_counter = 0;
	// savestates
	save_item(NAME(usbhack_counter));
}

void xbox_state::machine_reset()
{
}

static MACHINE_CONFIG_DERIVED_CLASS(xbox, xbox_base, xbox_state)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(xbox_map)
	MCFG_CPU_IO_MAP(xbox_map_io)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( xbox )
	ROM_REGION( 0x200, "mcpx", 0 )
	ROM_LOAD( "mcpx_1_0.bin", 0, 0x200, CRC(f31429fc) SHA1(a9ecbf8896d10db81594923e485862aa3aac7b58) )
	ROM_LOAD( "mcpx_1_1.bin", 0, 0x200, CRC(94ce376b) SHA1(6c875f17f773aaec51eb434068bb6c657c4343c0) )

	ROM_REGION( 0x80000, "bios", 0)
	ROM_LOAD( "xbox-5530.bin", 0x000000, 0x040000, CRC(9569c4d3) SHA1(40fa73277013be3168135e1768b09623a987ff63) )
	ROM_LOAD( "xbox-5713.bin", 0x040000, 0x040000, CRC(58fd8173) SHA1(8b7ccc4648ccd78cdb7b65cfca09621eaf2d4238) )
	ROM_COPY( "mcpx", 0, 0x7fe00, 0x200 )


	ROM_REGION( 0x1000000, "tbp", 0 ) // To Be Processed, of course
	ROM_LOAD( "3944_1024k.bin", 0x000000, 0x100000, CRC(32a9ecb6) SHA1(67054fc88bda94e33e86f1b19be60efec0724fb6) )
	ROM_LOAD( "4034_1024k.bin", 0x000000, 0x100000, CRC(0d6fc88f) SHA1(ab676b712204fb1728bf89f9cd541a8f5a64ab97) )
	ROM_LOAD( "4134_1024k.bin", 0x000000, 0x100000, CRC(49d8055a) SHA1(d46cef771a63dc8024fe36d7ab5b959087ac999f) )
	ROM_LOAD( "4817_1024k.bin", 0x000000, 0x100000, CRC(3f30863a) SHA1(dc955bd4d3ca71e01214a49e5d0aba615270c03c) )
	ROM_LOAD( "5101_256k.bin", 0x000000, 0x040000, CRC(e8a9224e) SHA1(5108e1025f48071c07a6823661d708c66dee97a9) )
	ROM_LOAD( "5838_256k.bin", 0x000000, 0x040000, CRC(5be2413d) SHA1(b9489e883c650b5e5fe2f83a32237dbf74f0e9f1) )
ROM_END
// See src/emu/gamedrv.h for details
// For a game:
// GAME(YEAR,NAME,PARENT,MACHINE,INPUT,CLASS,INIT,MONITOR,COMPANY,FULLNAME,FLAGS)

// For a console:
// CONS(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS)

// For a computer:
// COMP(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS)

// For a generic system:
// SYST(YEAR,NAME,PARENT,COMPAT,MACHINE,INPUT,CLASS,INIT,COMPANY,FULLNAME,FLAGS)

CONS( 2001, xbox,  0,  0,   xbox,  xbox, driver_device,  0,       "Microsoft",      "XBOX", MACHINE_IS_SKELETON )
