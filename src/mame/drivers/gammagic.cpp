// license:BSD-3-Clause
// copyright-holders:Grull Osgo
/************************************************************************************

Game Magic (c) 1997 Bally Gaming Co.

Preliminary driver by Grull Osgo

Game Magic

Is a Multigame machine build on a Bally's V8000 platform.

This is the first PC based gaming machine developed by Bally Gaming.

V8000 platform includes:

1 Motherboard MICRONICS M55Hi-Plus PCI/ISA, Chipset INTEL i430HX (TRITON II), 64 MB Ram (4 SIMM M x 16 MB SIMM)
On board Sound Blaster Vibra 16C chipset.
1 TOSHIBA CD-ROM or DVD-ROM Drive w/Bootable CD-ROM with Game.
1 OAK SVGA PCI Video Board.
1 Voodoo Graphics PCI Video Board, connected to the monitor.
1 21" SVGA Color Monitor, 16x9 Aspect, Vertical mount, with touchscreen.
1 Bally's IO-Board, Based on 68000 procesor as interface to all gaming devices
(Buttons, Lamps, Switches, Coin acceptor, Bill Validator, Hopper, Touchscreen, etc...)

PC and IO-Board communicates via RS-232 Serial Port.

Additional CD-ROM games: "99 Bottles of Beer"

*************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
//#include "machine/i82371sb.h"
//#include "machine/i82439tx.h"
#include "machine/lpci.h"
#include "machine/pcshare.h"
#include "machine/pckeybrd.h"
#include "video/pc_vga.h"

class gammagic_state : public pcat_base_state
{
public:
	gammagic_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag) { }

	void gammagic(machine_config &config);

private:
	virtual void machine_start() override;
	void gammagic_io(address_map &map);
	void gammagic_map(address_map &map);
};

// Memory is mostly handled by the chipset
void gammagic_state::gammagic_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000a0000, 0x000bffff).rw("vga", FUNC(vga_device::mem_r), FUNC(vga_device::mem_w));
	map(0x000e0000, 0x000fffff).rom().region("user", 0x20000);/* System BIOS */
	map(0x00100000, 0x07ffffff).ram();
	map(0x08000000, 0xfffdffff).noprw();
	map(0xfffe0000, 0xffffffff).rom().region("user", 0x20000);/* System BIOS */
}

void gammagic_state::gammagic_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x00e8, 0x00ef).noprw();
	map(0x00f0, 0x01ef).noprw();
	map(0x01f8, 0x03af).noprw();
	map(0x03b0, 0x03bf).rw("vga", FUNC(vga_device::port_03b0_r), FUNC(vga_device::port_03b0_w));
	map(0x03c0, 0x03cf).rw("vga", FUNC(vga_device::port_03c0_r), FUNC(vga_device::port_03c0_w));
	map(0x03d0, 0x03df).rw("vga", FUNC(vga_device::port_03d0_r), FUNC(vga_device::port_03d0_w));
	map(0x03e0, 0x03ef).noprw();
	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_legacy_device::read), FUNC(pci_bus_legacy_device::write));
	map(0x0400, 0xffff).noprw();
}

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(text) PORT_CODE(key1)

#if 1
static INPUT_PORTS_START( gammagic )
	PORT_START("pc_keyboard_0")
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED )     /* unused scancode 0 */
	AT_KEYB_HELPER( 0x0002, "Esc",          KEYCODE_Q           ) /* Esc                         01  81 */

	PORT_START("pc_keyboard_1")
	AT_KEYB_HELPER( 0x0010, "T",            KEYCODE_T           ) /* T                           14  94 */
	AT_KEYB_HELPER( 0x0020, "Y",            KEYCODE_Y           ) /* Y                           15  95 */
	AT_KEYB_HELPER( 0x0100, "O",            KEYCODE_O           ) /* O                           18  98 */
	AT_KEYB_HELPER( 0x1000, "Enter",        KEYCODE_ENTER       ) /* Enter                       1C  9C */

	PORT_START("pc_keyboard_2")

	PORT_START("pc_keyboard_3")
	AT_KEYB_HELPER( 0x0001, "B",            KEYCODE_B           ) /* B                           30  B0 */
	AT_KEYB_HELPER( 0x0002, "N",            KEYCODE_N           ) /* N                           31  B1 */
	AT_KEYB_HELPER( 0x0800, "F1",           KEYCODE_S           ) /* F1                          3B  BB */
	AT_KEYB_HELPER( 0x1000, "F2",           KEYCODE_D           ) /* F2                          3C  BC */
	AT_KEYB_HELPER( 0x4000, "F4",           KEYCODE_F           ) /* F4                          3E  BE */


	PORT_START("pc_keyboard_4")
	AT_KEYB_HELPER( 0x0004, "F8",           KEYCODE_F8          ) // f8=42  /f10=44 /minus 4a /plus=4e
	AT_KEYB_HELPER( 0x0010, "F10",          KEYCODE_F10         ) // f8=42  /f10=44 /minus 4a /plus=4e
	AT_KEYB_HELPER( 0x0100, "KP 8(UP)",     KEYCODE_8_PAD       ) /* Keypad 8  (Up arrow)        48  C8 */
	AT_KEYB_HELPER( 0x0400, "KP -",         KEYCODE_MINUS_PAD   ) // f8=42  /f10=44 /minus 4a /plus=4e
	AT_KEYB_HELPER( 0x4000, "KP +",         KEYCODE_PLUS_PAD    ) // f8=42  /f10=44 /minus 4a /plus=4e

	PORT_START("pc_keyboard_5")
	AT_KEYB_HELPER( 0x0001, "KP 2(DN)",     KEYCODE_2_PAD       ) /* Keypad 2  (Down arrow)      50  D0 */

	PORT_START("pc_keyboard_6")
	AT_KEYB_HELPER( 0x0040, "(MF2)Cursor Up",       KEYCODE_UP          ) /* Up                          67  e7 */
	AT_KEYB_HELPER( 0x0080, "(MF2)Page Up",         KEYCODE_PGUP        ) /* Page Up                     68  e8 */
	AT_KEYB_HELPER( 0x0100, "(MF2)Cursor Left",     KEYCODE_LEFT        ) /* Left                        69  e9 */
	AT_KEYB_HELPER( 0x0200, "(MF2)Cursor Right",        KEYCODE_RIGHT       ) /* Right                       6a  ea */
	AT_KEYB_HELPER( 0x0800, "(MF2)Cursor Down",     KEYCODE_DOWN        ) /* Down                        6c  ec */
	AT_KEYB_HELPER( 0x1000, "(MF2)Page Down",       KEYCODE_PGDN        ) /* Page Down                   6d  ed */
	AT_KEYB_HELPER( 0x4000, "Del",                      KEYCODE_A           ) /* Delete                      6f  ef */

	PORT_START("pc_keyboard_7")

INPUT_PORTS_END
#endif

void gammagic_state::machine_start()
{
}

void gammagic_state::gammagic(machine_config &config)
{
	PENTIUM(config, m_maincpu, 133000000); // Intel Pentium 133
	m_maincpu->set_addrmap(AS_PROGRAM, &gammagic_state::gammagic_map);
	m_maincpu->set_addrmap(AS_IO, &gammagic_state::gammagic_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);

	PCI_BUS_LEGACY(config, "pcibus", 0, 0);
//  pcibus.set_device_read (0, FUNC(gammagic_state::intel82439tx_pci_r), this);
//  pcibus.set_device_write(0, FUNC(gammagic_state::intel82439tx_pci_w), this);
//  pcibus.set_device_read (7, FUNC(gammagic_state::intel82371ab_pci_r), this);
//  pcibus.set_device_write(7, FUNC(gammagic_state::intel82371ab_pci_w), this);

	/* video hardware */
	pcvideo_vga(config);
}


ROM_START( gammagic )
	ROM_REGION32_LE(0x40000, "user", 0)
	//Original Memory Set
	//ROM_LOAD("m7s04.rom",   0, 0x40000, CRC(3689f5a9) SHA1(8daacdb0dc6783d2161680564ffe83ac2515f7ef))
	//ROM_LOAD("otivga_tx2953526.rom", 0x0000, 0x8000, CRC(916491af) SHA1(d64e3a43a035d70ace7a2d0603fc078f22d237e1))

	//Temp. Memory Set (Only for initial driver development stage)
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )
	ROM_LOAD("5hx29.bin",   0x20000, 0x20000, CRC(07719a55) SHA1(b63993fd5186cdb4f28c117428a507cd069e1f68))

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "gammagic", 0,SHA1(caa8fc885d84dbc07fb0604c76cd23c873a65ce6) )
ROM_END

ROM_START( 99bottles )
	ROM_REGION32_LE(0x40000, "user", 0)
	//Original BIOS/VGA-BIOS Rom Set
	//ROM_LOAD("m7s04.rom",   0, 0x40000, CRC(3689f5a9) SHA1(8daacdb0dc6783d2161680564ffe83ac2515f7ef))
	//ROM_LOAD("otivga_tx2953526.rom", 0x0000, 0x8000, CRC(916491af) SHA1(d64e3a43a035d70ace7a2d0603fc078f22d237e1))

	//Temporary (Chipset compatible Rom Set, only for driver development stage)
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )
	ROM_LOAD("5hx29.bin",   0x20000, 0x20000, CRC(07719a55) SHA1(b63993fd5186cdb4f28c117428a507cd069e1f68))

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "99bottles", 0, SHA1(0b874178c8dd3cfc451deb53dc7936dc4ad5a04f))
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/
/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT        ROT   COMPANY             FULLNAME              FLAGS
GAME( 1999, gammagic,  0,        gammagic, gammagic, gammagic_state, empty_init, ROT0, "Bally Gaming Co.", "Game Magic",         MACHINE_IS_SKELETON )
GAME( 1999, 99bottles, gammagic, gammagic, gammagic, gammagic_state, empty_init, ROT0, "Bally Gaming Co.", "99 Bottles of Beer", MACHINE_IS_SKELETON )
