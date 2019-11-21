// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Midway Quicksilver II/Graphite skeleton driver
	
Hardware configurations:

Hydro Thunder:   Quicksilver II system and Diego I/O board
Offroad Thunder: Quicksilver II system and Magicbus I/O board	
Arctic Thunder:  Graphite system and Substitute I/O board
	
All of the games communicate with their I/O boards serially.

Quicksilver II hardware:
- Main CPU: Intel Celeron (Pentium II) 333/366MHz
- Motherboard: Intel SE440BX-2
- RAM: 64MB PC100-222-620 non-ecc
- Sound: Integrated YMF740G
- Networking: SMC EZ Card 10 / SMC1208T (probably 10ec:8029 1113:1208)
- Video Card: Quantum Obsidian 3DFX Voodoo 2 (CPLD protected)
- Storage: Hard Drive
- OS: TNT Embedded Kernel

Chipsets (440BX AGPset):
- 82443BX Northbridge
- 82371EB PIIX4 PCI-ISA Southbridge

Note: This was once claimed to run on Windows 95 or 98 but has been proven (mostly) false. The TNT Kernel was a "DOS Extender"
that allows core Windows NT functions to work on MS DOS. It's also possible it runs on a custom made OS as both games do not display
anything DOS related.
	
Graphite hardware:
- Main CPU: Intel Pentium III 733MHz
- Motherboard: BCM GT694VP
- RAM: 128MB PC100/133
- Sound: Integrated AC97 Controller on VT82C686A Southbridge
    -or ES1373/CT5880 Audio Chip
- Networking: SMC EZ Card 10 / SMC1208T (probably 10ec:8029 1113:1208)
- Video Card: 3DFX Voodoo 3
- Storage: Hard Drive (copy protected)
- OS: Windows 2000 Professional

Chipsets (VIA Pro133A):
- VT82C694X Northbridge
- VT82C686A Southbridge
	
Note: Not only a beefed up Quicksilver II but acts like a normal PC. You get the normal bios startup, a Windows 2000 startup sequence and
then the game launcher starts. Another difference is the storage device has a copy protection scheme that "locks" the storage device to the
motherboard's serial number. If a drive doesn't match the motherboard's serial number, the game launcher will give an error.
	
I/O boards:
	
MIDWAY GAMES INC
5770-15983-04
DIEGO

|--------------------------------------------------------------|
|       JP13     JP14       JP15        J1         JP2         |
|                                                              |
|                                      JP1         JP5   JP6   |
|                                                  JP4         |
|                                                  JP3         |
|                                                           U1 |
| JP11                              U2   U3                    |
|                                                              |
|     U12                             JP12                     |
|JP9                                                           |
|      U5  U6               U10                             JP8|
|                                  U8             S1           |
|                           Y2                                 |
|      U7        Q2                                   U4       |
|                                                              |
|      JP15                  J2           J3                   |
|      P1        JP10                                 JP7      |
|--------------------------------------------------------------|

Notes:
    J1: Video connector from/to video card
    J2/J3: USB ports (not used)
    JP1: Alternate video connector from/to video card
    JP2: Video output signal to monitor
    JP3: 3 pin jumper: Blue video level, *Open: high, 1-2: low, 2-3: high
    JP4: 3 pin jumper: Green video level, *Open: high, 1-2: low, 2-3: high
    JP5: 3 pin jumper: Red video level, *Open: high, 1-2: low, 2-3: high
    JP6: 3 pin jumper: Video sync polarity, *Open: pos, 1-2: neg, 2-3: pos
    JP7: 13 pin connector for switches and analog controls/potentiometers
    JP8: 14 pin connector for coin, service, volume and test inputs
    JP9: 20 pin ribbon cable connector to wheel driver board
    JP10: 9 pin connector for coin meter
    JP11: 2 pin connector for watchdog reset
    JP12: connector for development use, not used
    JP13-15: connectors, not used
    JP16: Power connector
    P1: DB9 RS-232 port to computer
	Q2: ULN2064B Darlington Transistor
	S1: Dip Switches (8).
		 S1-3: *Off: Game Mode, On: Test Mode
         S1-4: *Off: 25" Cabinet, On: 39" Cabinet
         S1-8: Off: Watchdog Disabled, *On: Watchdog Enabled
	U1: Texas Instruments LS85A Logic Gate
    U2-3: EL244CS Amplifier
    U4: 109B Instrumentation Amplifier
    U5: PC16550DV UART Interface IC
    U6: Motorola MC74HC273A Octal D Flip-Flop (LS273 based)
    U7: DS14185WM RS-232 Interface IC
    U8: CY7C63513-PVC 8-bit RISC Microcontroller
    U10: Atmel 24C01A Serial EEPROM
    U12: MAX707CSA Supervisory Circuit
    Y2: Crystal/XTAL 6.000 MHz


MIDWAY GAMES INC
5770-16226-01
MAGICBUS

|--------------------------------------------------------------------------------------------|
|  J2           JP21      JP22      JP12                        JP9       J1                 |
|                                                    JP24                           JP6      |
|         P1                                                            JP5     JP3 JP2 JP1  |
|       JP23   U19                                                                           |
|                       U22                                                         JP4      |
|JP10                           Q4                                U2        U3        U1     |
|                U17                                                                         |
|     Y1   U18                                                                               |
|                                                              U5                            |
|                 U20                                                                        |
|JP11   U21                                                                                  |
|                                                                                            |
|                                                          U6          S2        S1          |
|                JP20                                                                        |
|                                                                                            |
|JP16      U12                      U16        U15                                           |
|                                                                                            |
|                                                                                      U11   |
|      JP17         JP19           JP18              JP13             JP14           JP15    |
|--------------------------------------------------------------------------------------------|

Notes:
    J1: Video connector from/to video card
    J2: USB port (not used)
    JP1: 3 pin jumper: Blue video level, *Open: high, 1-2: low, 2-3: high
    JP2: 3 pin jumper: Green video level, *Open: high, 1-2: low, 2-3: high
    JP3: 3 pin jumper: Red video level, *Open: high, 1-2: low, 2-3: high
    JP4: 3 pin jumper: Video sync polarity, Open: pos, *1-2: neg, 2-3: pos
    JP5: Alternate video connector from/to video card
    JP6: Video output signal to monitor
    JP9: Power connector
    JP10: 20 pin ribbon cable connector to wheel driver board
    JP11: 20 pin ribbon cable connector, not used
    JP12-14: connectors, not used
    JP15: 14 pin connector for analog controls/potentiometers
    JP16: 15 pin connector for gameplay inputs
    JP17: connector, not used
    JP18: 14 pin connector for coin, service, volume and test inputs
    JP19: connector, not used
    JP20: connector for development use, not used
    JP21: 2 pin connector for watchdog reset
    JP22: 9 pin connector for coin meter
    JP23: Alternate RS232 port
    JP24: connector, not used
    P1: DB9 RS-232 port to computer
	Q4: ULN2064B Darlington Transistor
	S1: Dip Switches (8)
         S1-7: *Off: UART, On: USB
         S1-8: Off: Watchdog Disabled, *On: Watchdog Enabled
	S2: Dip Switches (8), all set to "off"
	U1: LS85A Logic Gate
    U2-3: EL244CS Amplifier
    U5: MAX707CSA Supervisory Circuit
    U6: Motorola MC74HC273A Octal D Flip-Flop (LS273 based)
    U11: ADC0834 Serial I/O Converter
    U12-15: HC541 Octal Buffer
    U17: Atmel 24C01A Serial EEPROM
    U18: CY7C63513-PVC 8-bit RISC Microcontroller
    U19: DS14185WM RS-232 Interface IC
    U20: PC16550DV UART Interface IC
    U21: Oscilator 3.6864 MHz
    U22: HC04 Hex Inverter
    Y1: Crystal/XTAL 6.000 MHz

(Incomplete)
MIDWAY GAMES INC.
SUBSTITUTE MAGICBUS
5770-16367-02

|--------------------------------------------------------------------------------------------|
|  J2     P1  JP21    JP22       JP12                           JP9       J1                 |
|       JP23                                         JP24                           JP6      |
|        U21         U24          U22                                   JP5     JP3 JP2 JP1  |
|                                                                                            |
|                                                     U23                           JP4      |
|JP10                                                                                        |
|                U11   U7        U8                                                          |
|                                                                                            |
|                                                                                            |
|                                 U6                                                         |
|JP11           Y2    U20                                                                    |
|                                                                                            |
|          U25                                    U4                                         |
|                                                                      S2        S1          |
|JP16      U15   U16   U17    U19   U18    U5     U13    U12    U14                          |
|                                                                                            |
|                                                                                            |
|                                                                                            |
|      JP17         JP19           JP18              JP13             JP14           JP15    |
|--------------------------------------------------------------------------------------------|

Notes:
    J1/JP5: Video connector from/to video card, not used
    J2: USB port, not used
    JP1-4: Video signal jumpers, not used
    JP6: Video output signal to monitor, not used
    JP9: Power connector
    JP10: 20 pin ribbon cable connector to wheel driver board
    JP11: 20 pin ribbon cable connector, not used
    JP12-14: connectors, not used
    JP15: 14 pin connector for analog controls/potentiometers
    JP16: 15 pin connector, attack button
    JP17: connector, not used
    JP18: 14 pin connector for coin, service, volume and test inputs
    JP19: 8 pin connector, start button
    JP20: connector for development use, not used
    JP21: 2 pin connector for watchdog reset
    JP22: 9 pin connector for coin meter
    JP23: Alternate RS232 port
    JP24: connector, not used
    P1: DB9 RS-232 port to computer
	S1: Dip Switches (8)
	S1: Dip Switches (8)
         S1-7: *Off: UART, On: USB
         S1-8: Off: Watchdog Disabled, *On: Watchdog Enabled
	S2: Dip Switches (8)
    U4-5: MC74HC273A Octal D Flip-Flop (LS273 based)
    U6: Not known yet
    U7/U11: Atmel 24C01A Serial EEPROM
    U8: MAX707CSA Supervisory Circuit
    U12-19: HC541 Octal Buffer
    U20: Philips P87C51/2 8-bit Microcontroller
    U21: DS14185WM RS-232 Interface IC
	U22-24: ULN2064B Darlington Transistor
    U25: Not known yet
    Y2: FS14.74 Crystal/Oscilator
	
***************************************************************************/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pcshare.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
#include "video/pc_vga.h"

class midqslvr_state : public pcat_base_state
{
public:
	midqslvr_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
	{
	}

	void midqslvr(machine_config &config);
	void graphite(machine_config &config);

private:
	std::unique_ptr<uint32_t[]> m_bios_ram;
	std::unique_ptr<uint32_t[]> m_bios_ext1_ram;
	std::unique_ptr<uint32_t[]> m_bios_ext2_ram;
	std::unique_ptr<uint32_t[]> m_bios_ext3_ram;
	std::unique_ptr<uint32_t[]> m_bios_ext4_ram;
	std::unique_ptr<uint32_t[]> m_isa_ram1;
	std::unique_ptr<uint32_t[]> m_isa_ram2;
	uint8_t m_mtxc_config_reg[256];
	uint8_t m_piix4_config_reg[4][256];

	DECLARE_WRITE32_MEMBER( isa_ram1_w );
	DECLARE_WRITE32_MEMBER( isa_ram2_w );

	DECLARE_WRITE32_MEMBER( bios_ext1_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext2_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext3_ram_w );
	DECLARE_WRITE32_MEMBER( bios_ext4_ram_w );

	DECLARE_WRITE32_MEMBER( bios_ram_w );
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void intel82439tx_init();
	void midqslvr_io(address_map &map);
	void midqslvr_map(address_map &map);

	uint8_t mtxc_config_r(int function, int reg);
	void mtxc_config_w(int function, int reg, uint8_t data);
	uint32_t intel82439tx_pci_r(int function, int reg, uint32_t mem_mask);
	void intel82439tx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
	uint8_t piix4_config_r(int function, int reg);
	void piix4_config_w(int function, int reg, uint8_t data);
	uint32_t intel82371ab_pci_r(int function, int reg, uint32_t mem_mask);
	void intel82371ab_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
};


// Intel 82439TX System Controller (MTXC)

uint8_t midqslvr_state::mtxc_config_r(int function, int reg)
{
//  osd_printf_debug("MTXC: read %d, %02X\n", function, reg);

	if((reg & 0xfc) == 0 && function == 0) // return vendor ID
		return (0x71008086 >> (reg & 3)*8) & 0xff;

	return m_mtxc_config_reg[reg];
}

void midqslvr_state::mtxc_config_w(int function, int reg, uint8_t data)
{
	printf("MTXC: write %d, %02X, %02X\n",  function, reg, data);

	/*
	memory banking with North Bridge:
	0x59 (PAM0) xxxx ---- BIOS area 0xf0000-0xfffff
	            ---- xxxx Reserved
	0x5a (PAM1) xxxx ---- ISA add-on BIOS 0xc4000 - 0xc7fff
	            ---- xxxx ISA add-on BIOS 0xc0000 - 0xc3fff
	0x5b (PAM2) xxxx ---- ISA add-on BIOS 0xcc000 - 0xcffff
	            ---- xxxx ISA add-on BIOS 0xc8000 - 0xcbfff
	0x5c (PAM3) xxxx ---- ISA add-on BIOS 0xd4000 - 0xd7fff
	            ---- xxxx ISA add-on BIOS 0xd0000 - 0xd3fff
	0x5d (PAM4) xxxx ---- ISA add-on BIOS 0xdc000 - 0xdffff
	            ---- xxxx ISA add-on BIOS 0xd8000 - 0xdbfff
	0x5e (PAM5) xxxx ---- BIOS extension 0xe4000 - 0xe7fff
	            ---- xxxx BIOS extension 0xe0000 - 0xe3fff
	0x5f (PAM6) xxxx ---- BIOS extension 0xec000 - 0xeffff
	            ---- xxxx BIOS extension 0xe8000 - 0xebfff

	3210 -> 3 = reserved, 2 = Cache Enable, 1 = Write Enable, 0 = Read Enable
	*/

	switch(reg)
	{
		case 0x59: // PAM0
		{
			if (data & 0x10)        // enable RAM access to region 0xf0000 - 0xfffff
				membank("bios_bank")->set_base(m_bios_ram.get());
			else                    // disable RAM access (reads go to BIOS ROM)
				membank("bios_bank")->set_base(memregion("bios")->base() + 0x70000);
			break;
		}
		case 0x5a: // PAM1
		{
			if (data & 0x1)
				membank("video_bank1")->set_base(m_isa_ram1.get());
			else
				membank("video_bank1")->set_base(memregion("video_bios")->base() + 0);

			if (data & 0x10)
				membank("video_bank2")->set_base(m_isa_ram2.get());
			else
				membank("video_bank2")->set_base(memregion("video_bios")->base() + 0x4000);

			break;
		}
		case 0x5e: // PAM5
		{
			if (data & 0x1)
				membank("bios_ext1")->set_base(m_bios_ext1_ram.get());
			else
				membank("bios_ext1")->set_base(memregion("bios")->base() + 0x60000);

			if (data & 0x10)
				membank("bios_ext2")->set_base(m_bios_ext2_ram.get());
			else
				membank("bios_ext2")->set_base(memregion("bios")->base() + 0x64000);

			break;
		}
		case 0x5f: // PAM6
		{
			if (data & 0x1)
				membank("bios_ext3")->set_base(m_bios_ext3_ram.get());
			else
				membank("bios_ext3")->set_base(memregion("bios")->base() + 0x68000);

			if (data & 0x10)
				membank("bios_ext4")->set_base(m_bios_ext4_ram.get());
			else
				membank("bios_ext4")->set_base(memregion("bios")->base() + 0x6c000);

			break;
		}
	}

	m_mtxc_config_reg[reg] = data;
}

void midqslvr_state::intel82439tx_init()
{
	m_mtxc_config_reg[0x60] = 0x02;
	m_mtxc_config_reg[0x61] = 0x02;
	m_mtxc_config_reg[0x62] = 0x02;
	m_mtxc_config_reg[0x63] = 0x02;
	m_mtxc_config_reg[0x64] = 0x02;
	m_mtxc_config_reg[0x65] = 0x02;
}

uint32_t midqslvr_state::intel82439tx_pci_r(int function, int reg, uint32_t mem_mask)
{
	uint32_t r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= mtxc_config_r(function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= mtxc_config_r(function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= mtxc_config_r(function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= mtxc_config_r(function, reg + 0) << 0;
	}
	return r;
}

void midqslvr_state::intel82439tx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		mtxc_config_w(function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		mtxc_config_w(function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		mtxc_config_w(function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		mtxc_config_w(function, reg + 0, (data >> 0) & 0xff);
	}
}

// Intel 82371AB PCI-to-ISA / IDE bridge (PIIX4)

uint8_t midqslvr_state::piix4_config_r(int function, int reg)
{
	function &= 3;

	if((reg & 0xfc) == 0) // return vendor ID
		return (((0x71108086 | (function & 3) << 16) >> (reg & 3)*8) & 0xff);

	if(reg == 0xe)
	{
		const uint8_t header_type_val[4] = { 0x80, 0x00, 0x00, 0x00 };
		return header_type_val[function];
	}

	if((reg & 0xfc) == 0x8)
	{
		/* TODO: reg 8 indicates Revision ID */
		const uint32_t class_code_val[4] = { 0x06010000, 0x01018000, 0x0c030000, 0x06800000 };

		return (((class_code_val[function]) >> (reg & 3)*8) & 0xff);
	}

	printf("%08x PIIX4: read %d, %02X\n", m_maincpu->pc(), function, reg);

	return m_piix4_config_reg[function][reg];
}

void midqslvr_state::piix4_config_w(int function, int reg, uint8_t data)
{
	printf("PIIX4: write %d, %02X, %02X\n", function, reg, data);

	function &= 3;

	m_piix4_config_reg[function][reg] = data;
}

uint32_t midqslvr_state::intel82371ab_pci_r(int function, int reg, uint32_t mem_mask)
{
	uint32_t r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= piix4_config_r(function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= piix4_config_r(function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= piix4_config_r(function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= piix4_config_r(function, reg + 0) << 0;
	}
	return r;
}

void midqslvr_state::intel82371ab_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		piix4_config_w(function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		piix4_config_w(function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		piix4_config_w(function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		piix4_config_w(function, reg + 0, (data >> 0) & 0xff);
	}
}


WRITE32_MEMBER(midqslvr_state::isa_ram1_w)
{
	if (m_mtxc_config_reg[0x5a] & 0x2)      // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_isa_ram1.get() + offset);
	}
}

WRITE32_MEMBER(midqslvr_state::isa_ram2_w)
{
	if (m_mtxc_config_reg[0x5a] & 0x2)      // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_isa_ram2.get() + offset);
	}
}

WRITE32_MEMBER(midqslvr_state::bios_ext1_ram_w)
{
	if (m_mtxc_config_reg[0x5e] & 0x2)      // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext1_ram.get() + offset);
	}
}


WRITE32_MEMBER(midqslvr_state::bios_ext2_ram_w)
{
	if (m_mtxc_config_reg[0x5e] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext2_ram.get() + offset);
	}
}


WRITE32_MEMBER(midqslvr_state::bios_ext3_ram_w)
{
	if (m_mtxc_config_reg[0x5f] & 0x2)      // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext3_ram.get() + offset);
	}
}


WRITE32_MEMBER(midqslvr_state::bios_ext4_ram_w)
{
	if (m_mtxc_config_reg[0x5f] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext4_ram.get() + offset);
	}
}


WRITE32_MEMBER(midqslvr_state::bios_ram_w)
{
	if (m_mtxc_config_reg[0x59] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram.get() + offset);
	}
}

void midqslvr_state::midqslvr_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000a0000, 0x000bffff).rw("vga", FUNC(vga_device::mem_r), FUNC(vga_device::mem_w));
	map(0x000c0000, 0x000c3fff).bankr("video_bank1").w(FUNC(midqslvr_state::isa_ram1_w));
	map(0x000c4000, 0x000c7fff).bankr("video_bank2").w(FUNC(midqslvr_state::isa_ram2_w));
	map(0x000e0000, 0x000e3fff).bankr("bios_ext1").w(FUNC(midqslvr_state::bios_ext1_ram_w));
	map(0x000e4000, 0x000e7fff).bankr("bios_ext2").w(FUNC(midqslvr_state::bios_ext2_ram_w));
	map(0x000e8000, 0x000ebfff).bankr("bios_ext3").w(FUNC(midqslvr_state::bios_ext3_ram_w));
	map(0x000ec000, 0x000effff).bankr("bios_ext4").w(FUNC(midqslvr_state::bios_ext4_ram_w));
	map(0x000f0000, 0x000fffff).bankr("bios_bank").w(FUNC(midqslvr_state::bios_ram_w));
	map(0x00100000, 0x01ffffff).ram();
	map(0xfff80000, 0xffffffff).rom().region("bios", 0);    /* System BIOS */
}

void midqslvr_state::midqslvr_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x00e8, 0x00ef).noprw();

	map(0x01f0, 0x01f7).rw("ide", FUNC(ide_controller_device::cs0_r), FUNC(ide_controller_device::cs0_w));
	map(0x03b0, 0x03bf).rw("vga", FUNC(vga_device::port_03b0_r), FUNC(vga_device::port_03b0_w));
	map(0x03c0, 0x03cf).rw("vga", FUNC(vga_device::port_03c0_r), FUNC(vga_device::port_03c0_w));
	map(0x03d0, 0x03df).rw("vga", FUNC(vga_device::port_03d0_r), FUNC(vga_device::port_03d0_w));
	map(0x03f0, 0x03f7).rw("ide", FUNC(ide_controller_device::cs1_r), FUNC(ide_controller_device::cs1_w));

	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_legacy_device::read), FUNC(pci_bus_legacy_device::write));
}

void midqslvr_state::machine_start()
{
	m_bios_ram = std::make_unique<uint32_t[]>(0x10000/4);
	m_bios_ext1_ram = std::make_unique<uint32_t[]>(0x4000/4);
	m_bios_ext2_ram = std::make_unique<uint32_t[]>(0x4000/4);
	m_bios_ext3_ram = std::make_unique<uint32_t[]>(0x4000/4);
	m_bios_ext4_ram = std::make_unique<uint32_t[]>(0x4000/4);
	m_isa_ram1 = std::make_unique<uint32_t[]>(0x4000/4);
	m_isa_ram2 = std::make_unique<uint32_t[]>(0x4000/4);
	intel82439tx_init();

}

void midqslvr_state::machine_reset()
{
	membank("bios_bank")->set_base(memregion("bios")->base() + 0x70000);
	membank("bios_ext1")->set_base(memregion("bios")->base() + 0x60000);
	membank("bios_ext2")->set_base(memregion("bios")->base() + 0x64000);
	membank("bios_ext3")->set_base(memregion("bios")->base() + 0x68000);
	membank("bios_ext4")->set_base(memregion("bios")->base() + 0x6c000);
	membank("video_bank1")->set_base(memregion("video_bios")->base() + 0);
	membank("video_bank2")->set_base(memregion("video_bios")->base() + 0x4000);
}

void midqslvr_state::midqslvr(machine_config &config)
{
	PENTIUM2(config, m_maincpu, 333000000); //Verified this Celeron to be Pentium II based.
	m_maincpu->set_addrmap(AS_PROGRAM, &midqslvr_state::midqslvr_map);
	m_maincpu->set_addrmap(AS_IO, &midqslvr_state::midqslvr_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);

	pci_bus_legacy_device &pcibus(PCI_BUS_LEGACY(config, "pcibus", 0, 0));
	pcibus.set_device( 0, FUNC(midqslvr_state::intel82439tx_pci_r), FUNC(midqslvr_state::intel82439tx_pci_w));
	pcibus.set_device(31, FUNC(midqslvr_state::intel82371ab_pci_r), FUNC(midqslvr_state::intel82371ab_pci_w));

	ide_controller_device &ide(IDE_CONTROLLER(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir6_w));

	/* video hardware */
	pcvideo_vga(config);
}

void midqslvr_state::graphite(machine_config &config) //Todo: The entire Pro133A chipset :).
{
	PENTIUM3(config, m_maincpu, 733000000); //Verified
	m_maincpu->set_addrmap(AS_PROGRAM, &midqslvr_state::midqslvr_map);
	m_maincpu->set_addrmap(AS_IO, &midqslvr_state::midqslvr_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);

	pci_bus_legacy_device &pcibus(PCI_BUS_LEGACY(config, "pcibus", 0, 0));
	pcibus.set_device( 0, FUNC(midqslvr_state::intel82439tx_pci_r), FUNC(midqslvr_state::intel82439tx_pci_w));
	pcibus.set_device(31, FUNC(midqslvr_state::intel82371ab_pci_r), FUNC(midqslvr_state::intel82371ab_pci_w));

	ide_controller_device &ide(IDE_CONTROLLER(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir6_w));

	/* video hardware */
	pcvideo_vga(config);
}


ROM_START( hydrthnd )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	ROM_REGION( 0x2000, "iocpu", 0 )   /* Diego board CY7C63513-PVC MCU code */
	ROM_LOAD( "diego.u8", 0x0000, 0x2000, NO_DUMP ) // 8KB internal EPROM

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "hydro", 0,  SHA1(d481d178782943c066b41764628a419cd55f676d) )
ROM_END

ROM_START( offrthnd )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD( "lh28f004sct.u8b1", 0x000000, 0x080000, CRC(ab04a343) SHA1(ba77933400fe470f45ab187bc0d315922caadb12) )

	ROM_REGION( 0x2000, "iocpu", 0 )   /* Magicbus board CY7C63513-PVC MCU code */
	ROM_LOAD( "magicbus.u18", 0x0000, 0x2000, NO_DUMP ) // 8KB internal EPROM

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "offrthnd", 0, SHA1(d88f1c5b75361a1e310565a8a5a09c674a4a1a22) )
ROM_END

ROM_START( arctthnd )
	ROM_REGION32_LE(0x80000, "bios", ROMREGION_ERASEFF)
	ROM_LOAD( "m29f002bt.u6", 0x040000, 0x040000, CRC(012c9290) SHA1(cdee6f19d5e5ea5bb1dd6a5ec397ac70b3452790) )

	ROM_REGION( 0x2000, "iocpu", 0 )   /* Substitute board P87C51/2 MCU code */
	ROM_LOAD( "substitute.u20", 0x0000, 0x2000, NO_DUMP ) // 8KB internal EPROM

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "arctthnd", 0,  SHA1(f4373e57c3f453ac09c735b5d8d99ff811416a23) )
ROM_END

ROM_START( ultarctc )
	ROM_REGION32_LE(0x80000, "bios", ROMREGION_ERASEFF)
	ROM_LOAD( "m29f002bt.u6", 0x040000, 0x040000, CRC(012c9290) SHA1(cdee6f19d5e5ea5bb1dd6a5ec397ac70b3452790) )

	ROM_REGION( 0x2000, "iocpu", 0 )   /* Substitute board P87C51/2 MCU code */
	ROM_LOAD( "substitute.u20", 0x0000, 0x2000, NO_DUMP ) // 8KB internal EPROM

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "uarctict", 0, SHA1(8557a1d7ae8dc41c879350cb1c228f4c27a0dd09) )
ROM_END

/* This is an update CD. This CD along with a dongle was released as a kit to update a normal Arctic Thunder to Ultimate.
Ultimate Arctic Thunder requires a dongle to work. If the dongle isn't detected both during and after installation,
the game will revert back to normal Arctic Thunder. */
ROM_START( ultarctcup )
	ROM_REGION32_LE(0x80000, "bios", ROMREGION_ERASEFF)
	ROM_LOAD( "m29f002bt.u6", 0x040000, 0x040000, CRC(012c9290) SHA1(cdee6f19d5e5ea5bb1dd6a5ec397ac70b3452790) )

	ROM_REGION( 0x2000, "iocpu", 0 )   /* Substitute board P87C51/2 MCU code */
	ROM_LOAD( "substitute.u20", 0x0000, 0x2000, NO_DUMP ) // 8KB internal EPROM

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "uarctict", 0, SHA1(8557a1d7ae8dc41c879350cb1c228f4c27a0dd09) )

	DISK_REGION( "cd" )
	DISK_IMAGE( "040503_1309", 0, SHA1(453adb81e204b0580ad02c2d98f68525757ec2a1) )
// sourced from these
//    ROM_LOAD( "040503_1309.CUE", 0x0000, 0x000004d, CRC(4a9e2de5) SHA1(04d3d90ad4b235c0ac4606557e16a1410d018fa9) )
//    ROM_LOAD( "040503_1309.BIN", 0x0000, 0x6bd9960, CRC(48a63422) SHA1(9d1cacf07526c5bddf4205c667a9010802f74859) )

ROM_END

// there are almost certainly multiple versions of these; updates were offered on floppy disk.  The version numbers for the existing CHDs are unknown.
GAME(1999, hydrthnd,    0,        midqslvr, at_keyboard, midqslvr_state, empty_init, ROT0, "Midway Games", "Hydro Thunder", MACHINE_IS_SKELETON)

GAME(2000, offrthnd,    0,        midqslvr, at_keyboard, midqslvr_state, empty_init, ROT0, "Midway Games", "Offroad Thunder", MACHINE_IS_SKELETON)

GAME(2001, arctthnd,    0,        graphite, at_keyboard, midqslvr_state, empty_init, ROT0, "Midway Games", "Arctic Thunder (v1.002)", MACHINE_IS_SKELETON)

GAME(2001, ultarctc,    0,        graphite, at_keyboard, midqslvr_state, empty_init, ROT0, "Midway Games", "Ultimate Arctic Thunder", MACHINE_IS_SKELETON)
GAME(2004, ultarctcup,  ultarctc, graphite, at_keyboard, midqslvr_state, empty_init, ROT0, "Midway Games", "Ultimate Arctic Thunder Update CD ver 1.950 (5/3/04)", MACHINE_IS_SKELETON)
