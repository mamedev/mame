// license:BSD-3-Clause
// copyright-holders:wurthless-elektroniks
/***************************************************************************

    mtouchfrc.cpp: Merit Megatouch Force hardware

    This is PC-based hardware that ran Merit's bartop coin munchers for much of the 2000s.
    Along with software updates for the new versions, Merit also sold operators new
    stickers and external branding, so it's common to see these machines rebranded
    to newer versions.

    The CRT-based units are renowned among collectors and operators for their magical
    exploding monitors. The monitor is a Tatung C5GNEKC (Merit part number EC9387-01-01,
    "15" VGA MN CHASSIS") and its flyback transformer is prone to failure.
    When the flyback goes, it takes a yellow film/polymer capacitor nearby with it.
    Later versions of the Force switched to LCD monitors.

    PC configuration:
    - Motherboard: ECS P6IWP-Fe
    - CPU: Intel Celeron SL5L5: 566 MHz, 128 KB L2, 66 MHz FSB
    - RAM: 1x 64 MB, Samsung, PC133U-333-542 ("64MB Sync 133 MHz CL3").
      Expandable; later versions of the Force software require 128 MB.
    - OS: Linux. (Not sure what version yet).
    - COM1: 3M MicroTouch touchscreen and controller (14-120-02 REV 2.3 F/W 8.3) 
    - COM2: Modem, if present. Typically a Conexant SmartV.92
    - Audio: Output from integrated 3.5mm speaker connector
    - USB: Custom I/O board that runs everything else.

    The PCI device tree is (as listed in BIOS):

    Bus | Dev | Func | Vendor | Dev  | Class | Comments
    ----+-----+------+--------+------+-------+-----------------------------------------------------------------
    0   | 1   | 0    | 8086   | 7125 | 0300  | Intel 82810E DC-133 Integrated Graphics Controller (VGA)
    0   | 31  | 1    | 8086   | 2411 | 0101  | Intel 82801AA (ICH) Dual Ultra ATA/66 IDE Controller
    0   | 31  | 2    | 8086   | 2412 | 0c03  | Intel 82801AA (ICH) USB 1.1 UHCI Host Controller
    1   | 7   | 0    | 10ec   | 8139 | 0200  | Realtek RTL8139 10/100 Fast Ethernet Controller (Onboard LAN)
    1   | 10  | 0    | 13f6   | 0111 | 0401  | C-Media CMI8738 PCI Audio Chip (Onboard Hardware Sound)

    Also on the board:
    - Intel FW82801AA southbridge
    - IT8712F-A Super I/O controller

    Sticker on board (from the Merit factory) gives a date of 05/12/03.

    Though the BIOS sticker reads "PhoenixBios (tm) D686 BIOS (c) PHOENIX 1998"
    the board has a customized Award BIOS with a Megatouch Force themed splash screen.

    The I/O board:
    ----------------------------------------------------------------------------

    The I/O board (part number SA10055-01) connects to the PC motherboard over USB.
    It is controlled by a Cypress EZ-USB FX, more specifically a CY7C64603-80NC,
    and it contains the following:

    - J1: Coin mech (if digitally controlled)
    - J2: Coin counters
    - J3: "MECHANICAL MECH"
    - J6: Power connector (can also be powered over USB if JP4 is set correctly)
    - U4: Cypress EZ-USB FX CY7C64603-80NC
    - U5: Battery slot for iButton 1-wire protection dongle
    - U7: 64 kbit EEPROM, reads blank (all ff's)

    The EZ-USB has the option of booting from the EEPROM, but in practice the software
    usually uploads the firmware at run time. The firmware is contained in Intel HEX
    in files named "fxdata.dat" for I/O boards with older EZ-USB MCUs or "fx1data.dat".

    AI traced the code and came up with the following port mappings:

    - PA0-PA7: Coin/bill accepter inputs, active high
    - PB0-PB1: GPIO outputs controlled by the host, active low
    - PB2-PB3: UART 1, goes to the printer
    - PB4-PB5: Seemingly unused
    - PB6-PB7: Coin cointers
    - PC0-PC1: UART 0, goes to the card reader
    - PC2: SETUP button, active low
    - PC3: CALIBRATE button, active low
    - PC4: Unclear. Software drives it low during init.
    - PC5: 1-wire interface to the security key
    - PC6: 1-wire interface to an external player-specific key
    - PC7: Reset output to the PC
    - PD0-PD7: Dipswitches
    - PE0-PE7: Unclear
    
    The software:
    ----------------------------------------------------------------------------

    Mostly an evolution of the Maxx software, it seems.

    The main executable ("start") is packed and has anti-debugging features.
    If you start with a debugger attached, it quits. Otherwise, it attaches a
    stub to itself (so gdb and friends can't attach), and decrypts its payload
    to a temporary directory and runs it. The payload is just a normal XOR cypher,
    where the index is the number of bytes read from the file so far.
    When decrypted you'll be greeted with an ELF file with symbols intact.

    Protection:
    ----------------------------------------------------------------------------

    Merit shipped all upgrade kits with an install CD plus a security key,
    which installs into the I/O board. The key is a Dallas iButton which is
    either a DS1991 or a DS1996. Its main purpose is copy protection, but
    it also defines the regional configuration of the machine, including
    what AccessMerit server to call home to, which languages can be selected,
    and other things.

    Newer versions use the Dallas DS1996 iButton, which is trivial to dump.
    The catch of course is that the iButtons have hardcoded device IDs set in ROM,
    and the device ID is used to decrypt the memory contents, to prevent people
    from copying the keys. The algorithm won't be described here as there's no
    guarantee that it is constant between software revisions.

    In MAME land we can just pass the security key's ID to the software and
    it will decrypt it for us. In the real world operators usually fall back
    on "keyless" cracks as that's easier for them to do.
    
    Additionally, the Force 2004 software will run a protection check against
    the hard drive. If it doesn't find the hard drive it expects, it throws
    up an error and halts. The protection data is stored in the IDENTIFY block.

    TODOs:
    ----------------------------------------------------------------------------

    Everything.
    - i810E GMCH
    - 82801AA ICH (IDE, USB, LPC)
    - IT8712F Super I/O
    - CMI8738 audio
    - RTL8139 LAN
    - Touchscreen
    - I/O board emulation, including:
        - Security dongle
        - Coin slots, bill acceptors, etc.

    The main blocker here is USB, which MAME doesn't have host support for yet.

***************************************************************************/

#include "emu.h"

#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/ram.h"

namespace {

class mtouchfrc_state : public driver_device
{
public:
	mtouchfrc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG)
	{ }

	void mtouchfrc(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;
};

void mtouchfrc_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint8_t* const ram = m_ram->pointer();

	// conventional memory, then everything above 1 MB; 0xa0000-0xfffff is the VGA/BIOS hole
	space.install_ram(0x00000000, 0x0009ffff, ram);
	space.install_ram(0x00100000, m_ram->size() - 1, ram + 0x00100000);
}


void mtouchfrc_state::main_map(address_map &map)
{
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);
}

void mtouchfrc_state::main_io(address_map &map)
{
}


void mtouchfrc_state::mtouchfrc(machine_config &config)
{
	PENTIUM3(config, m_maincpu, 566'666'666); // Celeron SL5L5: 66.67 MHz FSB x 8.5
	m_maincpu->set_addrmap(AS_PROGRAM, &mtouchfrc_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &mtouchfrc_state::main_io);

	RAM(config, m_ram).set_default_size("64M").set_extra_options("128M,256M,512M");

	PCI_ROOT(config, "pci");
	// ...
}

// TODO: there are likely several BIOS revisions with different years on them.
#define MTOUCHFRC_BIOS \
	ROM_REGION32_LE( 0x40000, "bios", 0 ) \
	ROM_LOAD( "ic29.bin", 0x00000, 0x40000, NO_DUMP ) // W49V002FAP, 2 megabits

ROM_START( mtfbios )
	MTOUCHFRC_BIOS
ROM_END

ROM_START( mtf2k75 )
	MTOUCHFRC_BIOS

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "mtf2k75", 0, NO_DUMP )

    // security dongle "SA3526-33 R00 C2006 MII"
    // dongle ROM key ID is: 8c 6b 0d 00 00 40 00 de 
    // this is for Canada, region id "CAN-VER2"
    ROM_REGION( 0x2000, "ds1996", 0 )
    ROM_LOAD( "sa3626-33_r00.bin", 0x000, 0x2000, CRC(f6457a0e) SHA1(9beb1563afe439a0e84ab0495a8dac4ef3e49c21) )
ROM_END

} // anonymous namespace

/*    YEAR   NAME     PARENT   COMPAT  MACHINE    INPUT  CLASS            INIT        COMPANY             FULLNAME */
COMP( 2002,  mtfbios, 0,       0,      mtouchfrc, 0,     mtouchfrc_state, empty_init, "Merit Industries", "Megatouch Force BIOS",   MACHINE_IS_BIOS_ROOT )
COMP( 2007,  mtf2k75, mtfbios, 0,      mtouchfrc, 0,     mtouchfrc_state, empty_init, "Merit Industries", "Megatouch Force 2007.5", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
