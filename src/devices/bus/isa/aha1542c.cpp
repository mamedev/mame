// license:BSD-3-Clause
// copyright-holders:Darkstar
/**********************************************************************
 *
 *    Adaptec AHA-1542{C,CF,CP} SCSI Controller
 *
 **********************************************************************/

/*
Hardware info by Guru

Adaptec 1993

PCB Layout
----------

Adaptec AHA-1542CF/1542CF
FCC ID: FGT1542CF
FAB 545107-00 REV C
ASSY 545106-00

|-------|---------------|---|-------------------|--|
| J1    |   FLOPPY34    |   |       SCSI50      |  |
|       |---------------|   |-------------------|  |
| S1    DS1                                        |
|                                                  |
|     |------|                DS2107AS DS2107AS   |--|
|     |PC8477|  24MHz   Z84C0010                  |  |
| PAL |BV-1  |                                    |S |
|     |      |                       |----------| |  |
|     |------|                       |          | |C |
|    93C46.U11                 20MHz |          | |  |
|         UM6264   MCODE.U15         |ADAPTEC   | |S |
|                                    |AIC-7970Q | |  |
|                  BIOS.U16          |          | |I |
|                                    |          | |  |
|                                    |----------| |--|
|                                                  |
|-|              |--|                         |----|
  |--------------|  |-------------------------|

Notes:
      J1         - 4-position header for drive activity external LED connection
      S1         - 8-position DIP Switch (See Table A)
      PC8477BV-1 - National Semiconductor PC8477BV-1 'SuperFDC' Advanced Floppy Disk Controller (PLCC68)
                   This is software compatible with NEC uPD765 and pin compatible with Intel 82077AA
                   Labelled "AHA-1542CF/552800-01 F/9513"
                   Clock input 24.000MHz
      SCSI       - 50-pin Centronics style external SCSI connector (DDK 57AE-40500-21D)
      Z84C0010   - Zilog Z84C0010VEC Z80 CPU. Clock input 20/2 [10.000MHz] sourced from AIC-7970Q (PLCC44)
      93C46.U11  - Atmel 93C46 128b x8-bit / 64b x16-bit (1kB) EEPROM labelled '545120A' (SOIC8)
      6264       - Unicorn Microelectronics UM6264BM-10L 8k x8-bit Static RAM (SOJ28)
      MCODE.U15  - ST Microelectronics M27C256B 32k x8-bit EPROM labelled 'ADAPTEC INC 553801-00 E MCODE 4B81' (DIP28)
      BIOS.U16   - ST Microelectronics M27C256B 32k x8-bit EPROM labelled 'ADAPTEC INC 553601-00 E BIOS 7600' (DIP28)
                   BIOS 0x7600h is revision 2.10
      PAL        - AMD PALCE16V8H-15JC/4 PAL (SOJ20)
      AIC-7970Q  - Adaptec AIC-7970Q Fast SCSI Controller IC. Clock input 20.000MHz (QFP144)
      DS2107AS   - Dallas DS2107AS SCSI Active Terminator (SOIC16)
      DS1        - Internal Drive Activity LED
      SCSI50     - 50-pin Right Angled Flat Cable Connector With Support For Up To 7 SCSI Drives
      FLOPPY34   - 34-pin Right Angled Flat Cable Connector With Support For Up To 2 Floppy Drives (360kb,720kb,1.2MB,1.44MB)


Table A - S1 DIP Switch Description
-----------------------------------

Default: All DIP Switches OFF (i.e. Settings changed via BIOS and saved in EEPROM)
Software Defaults = *
----------------------+-----+-----+-----+-----+-----+-----+-----+-----+
                      | SW1 | SW2 | SW3 | SW4 | SW5 | SW6 | SW7 | SW8 |
----------------------|-----+-----+-----+-----+-----+-----+-----+-----+
Termination   Enabled | ON  |     |     |     |     |     |     |     |
          Set In BIOS*| OFF |     |     |     |     |     |     |     |
----------------------+-----+-----+-----+-----+     |     |     |     |
I/O Port         330H*|     | OFF | OFF | OFF |     |     |     |     |
                 334H |     | ON  | OFF | OFF |     |     |     |     |
                 230H |     | OFF | ON  | OFF |     |     |     |     |
                 234H |     | ON  | ON  | OFF |     |     |     |     |
                 130H |     | OFF | OFF | ON  |     |     |     |     |
                 134H |     | ON  | OFF | ON  |     |     |     |     |
             Reserved |     | OFF | ON  | ON  |     |     |     |     |
             Reserved |     | ON  | ON  | ON  |     |     |     |     |
----------------------+-----+-----+-----+-----+-----+     |     |     |
Enable Floppy     Yes*|                       | OFF |     |     |     |
                   No |                       | ON  |     |     |     |
----------------------+-----------------------+-----+-----+-----+-----+
BIOS Address   DC000H*|                             | OFF | OFF | OFF |
               D8000H |                             | ON  | OFF | OFF |
               D4000H |                             | OFF | ON  | OFF |
               D0000H |                             | ON  | ON  | OFF |
               CC000H |                             | OFF | OFF | ON  |
               C8000H |                             | ON  | OFF | ON  |
             Reserved |                             | OFF | ON  | ON  |
        BIOS Disabled |                             | ON  | ON  | ON  |
----------------------+-----------------------------+-----+-----+-----+

Documentation:
Adaptec AHA-1540CF/1542CF Installation Guide
http://download.adaptec.com/pdfs/installation_guides/aha1540cf_ig.pdf
 */

/*
Adaptec QFP ASICs
— AHA-1540/42C: AIC-???? (covered by sticker)
— AHA-1540/42CP: AIC-7970Q
— AHA-1540/42CF: AIC-3370P, AIC-???? (covered by sticker)
*/

#include "emu.h"
#include "aha1542c.h"
#include "cpu/z80/z80.h"

// I/O Port interface
// READ  Port x+0: STATUS
// WRITE Port x+0: CONTROL
//
// READ  Port x+1: DATA
// WRITE Port x+1: COMMAND
//
// READ  Port x+2: INTERRUPT STATUS
// WRITE Port x+2: (undefined?)
//
// R/W   Port x+3: (undefined)

// READ STATUS flags
#define STAT_STST   0x80    // self-test in progress
#define STAT_DIAGF  0x40    // internal diagnostic failure
#define STAT_INIT   0x20    // mailbox initialization required
#define STAT_IDLE   0x10    // HBA is idle
#define STAT_CDFULL 0x08    // Command/Data output port is full
#define STAT_DFULL  0x04    // Data input port is full
#define STAT_INVCMD 0x01    // Invalid command

// READ INTERRUPT STATUS flags
#define INTR_ANY    0x80    // any interrupt
#define INTR_SRCD   0x08    // SCSI reset detected
#define INTR_HACC   0x04    // HA command complete
#define INTR_MBOA   0x02    // MBO empty
#define INTR_MBIF   0x01    // MBI full

// WRITE CONTROL commands
#define CTRL_HRST   0x80    // Hard reset
#define CTRL_SRST   0x40    // Soft reset
#define CTRL_IRST   0x20    // interrupt reset
#define CTRL_SCRST  0x10    // SCSI bus reset

// READ/WRITE DATA commands
#define CMD_NOP         0x00    // No operation
#define CMD_MBINIT      0x01    // mailbox initialization
#define CMD_START_SCSI  0x02    // Start SCSI command
#define CMD_BIOSCMD     0x03    // undocumented BIOS command (shadow RAM etc.)
#define CMD_INQUIRY     0x04    // Adapter inquiry
#define CMD_EMBOI       0x05    // enable Mailbox Out Interrupt
#define CMD_SELTIMEOUT  0x06    // Set SEL timeout
#define CMD_BUSON_TIME  0x07    // set bus-On time
#define CMD_BUSOFF_TIME 0x08    // set bus-off time
#define CMD_DMASPEED    0x09    // set ISA DMA speed
#define CMD_RETDEVS     0x0a    // return installed devices
#define CMD_RETCONF     0x0b    // return configuration data
#define CMD_TARGET      0x0c    // set HBA to target mode
#define CMD_RETSETUP    0x0d    // return setup data
#define CMD_ECHO        0x1f    // ECHO command data (NetBSD says it is 0x1e)

// these are for 1541C only:
#define CMD_RETDEVSHI   0x23    // return devices 8-15 (from NetBSD)
#define CMD_EXTBIOS     0x28    // return extended BIOS information
#define CMD_MBENABLE    0x29    // set mailbox interface enable

DEFINE_DEVICE_TYPE(AHA1542C, aha1542c_device, "aha1542c", "AHA-1542C SCSI Controller")
DEFINE_DEVICE_TYPE(AHA1542CF, aha1542cf_device, "aha1542cf", "AHA-1542CF SCSI Controller")
DEFINE_DEVICE_TYPE(AHA1542CP, aha1542cp_device, "aha1542cp", "AHA-1542CP SCSI Controller")

#define Z84C0010_TAG "z84c0010"

u8 aha1542c_device::aha1542_r(offs_t offset)
{
	logerror("%s aha1542_r(): offset=%d\n", machine().describe_context(), offset);
	return 0xff;
}

void aha1542c_device::aha1542_w(offs_t offset, u8 data)
{
	logerror("%s aha1542_w(): offset=%d data=0x%02x\n", machine().describe_context(), offset, data);
}


ROM_START( aha1542c )
	ROM_REGION( 0x8000, "aha1542", 0 )
	ROM_SYSTEM_BIOS( 0, "v101", "AHA-1540C/1542C BIOS v1.01" )
	ROMX_LOAD( "adaptec_inc_534201-00_d_bios_144c_1993.u15", 0x0000, 0x8000, CRC(35178004) SHA1(2b38f2e40cd02a1b32966ead7b202b0fca130cb8), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v102", "AHA-1540C/1542C BIOS v1.02" )
	ROMX_LOAD( "b_91c5.bin", 0x0000, 0x8000, CRC(076ac252) SHA1(d640b980e85d07029d8ce11a52fa26ba0f93c5de), ROM_BIOS(1) )

	ROM_REGION( 0x8000, Z84C0010_TAG, 0 )
	ROMX_LOAD( "adaptec_inc_534001-00_d_mcode_a3c2_1993.u5", 0x0000, 0x8000, CRC(220dd5a2) SHA1(4fc51c9dd63b45a50edcd56baa706d61decbef38), ROM_BIOS(0) )
	ROMX_LOAD( "m_866a.bin", 0x0000, 0x8000, CRC(ef09053a) SHA1(ae7900653357d5f32a2734bc13d9ec63bd805597), ROM_BIOS(1) )
ROM_END

ROM_START( aha1542cf )
	ROM_REGION( 0x8000, "aha1542", 0 )
	ROM_SYSTEM_BIOS( 0, "v201", "Adaptec 1540CF/1542CF BIOS v2.01" )
	ROMX_LOAD( "adaptec_inc_553601-00_c_bios_c38d_1993.u16", 0x0000, 0x8000, CRC(ab22fc02) SHA1(f9f783e0272fc14ba3de32316997f1f6cadc67d0), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v210", "Adaptec 1540CF/1542CF BIOS v2.10" )
	ROMX_LOAD( "adaptec_inc_553601-00_e_bios_7600_1994.u16", 0x0000, 0x8000, CRC(8f3a2692) SHA1(b9dbd49baeec55098195131d0ed1a9bfe8463640), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v211", "Adaptec 1540CF/1542CF BIOS v2.11" )
	ROMX_LOAD( "adaptec_inc_553601-00_g_bios_b402_1995.u16", 0x0000, 0x8000, CRC(fddd0b83) SHA1(aabd227cb338d8812e0bb5c17c08ea06c5aedd36), ROM_BIOS(2) )

	ROM_REGION( 0x8000, Z84C0010_TAG, 0 )
	ROMX_LOAD( "adaptec_inc_553801-00_c_mcode_563d_1993.u15", 0x0000, 0x8000, CRC(7824397e) SHA1(35bc2c8fab31aad3190a478f2dc8f3a72958cf04), ROM_BIOS(0) )
	ROMX_LOAD( "adaptec_inc_553801-00_e_mcode_4b81_1994.u15", 0x0000, 0x8000, CRC(dd651476) SHA1(cda508281302be53ebdcf8daa61754c89ad12111), ROM_BIOS(1) )
	ROMX_LOAD( "adaptec_inc_553801-00_g_mcode_2cde_1995.u15", 0x0000, 0x8000, CRC(896873cd) SHA1(6edbdd9b0b15ef31ca0741cac31556d2d5266b6e), ROM_BIOS(2) )
ROM_END

ROM_START( aha1542cp )
	ROM_REGION( 0x8000, "aha1542", 0 )
	ROM_LOAD( "adaptec_inc_908501-00_d_bios_a91e_1995.u7", 0x0000, 0x8000, CRC(0646c35e) SHA1(3a7c2731abd8295438cfa1f2a525be53e9512b1a) )

	ROM_REGION( 0x8000, Z84C0010_TAG, 0 )
	ROM_LOAD( "908301-00_f_mcode_17c9.u12", 0x0000, 0x8000, CRC(04494022) SHA1(431dfc26312556ddd24fccc429b2b3e93bac5c2f) )
ROM_END


void aha1542c_device::local_latch_w(u8 data)
{
	m_eeprom->cs_write(BIT(data, 2));
	m_eeprom->clk_write(BIT(data, 1));
	m_eeprom->di_write(BIT(data, 0));
	// TODO: several other bits are used
}

void aha1542c_device::z84c0010_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region(Z84C0010_TAG, 0);
	map(0x8000, 0x9fff).ram();        // 2kb RAM chip
	map(0xa000, 0xa000).portr(m_switches);
	map(0xb000, 0xb000).w(FUNC(aha1542c_device::local_latch_w));
	map(0xe000, 0xe0ff).ram();        // probably PC<->Z80 communication area
	map(0xe003, 0xe003).lr8(NAME([] () { return 0x20; }));
}

u8 aha1542cp_device::eeprom_r()
{
	// TODO: bits 4 and 5 are also used
	return m_eeprom->do_read();
}

void aha1542cp_device::eeprom_w(u8 data)
{
	m_eeprom->cs_write(BIT(data, 2));
	m_eeprom->clk_write(BIT(data, 1));
	m_eeprom->di_write(BIT(data, 0));
}

void aha1542cp_device::local_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom().region(Z84C0010_TAG, 0);
	map(0x8000, 0x9fff).ram();
	map(0xc000, 0xc000).portr(m_switches);
	map(0xc001, 0xc001).rw(FUNC(aha1542cp_device::eeprom_r), FUNC(aha1542cp_device::eeprom_w));
	map(0xe003, 0xe003).nopr();
}

static INPUT_PORTS_START( aha1542c )
	PORT_START("SWITCHES")
	PORT_DIPNAME(0x07, 0x07, "I/O Port Address") PORT_DIPLOCATION("S1:2,3,4")
	PORT_DIPSETTING(0x07, "330-333h")
	PORT_DIPSETTING(0x06, "334-337h")
	PORT_DIPSETTING(0x05, "230-233h")
	PORT_DIPSETTING(0x04, "234-237h")
	PORT_DIPSETTING(0x03, "130-133h")
	PORT_DIPSETTING(0x02, "134-137h")
	PORT_DIPSETTING(0x00, "Diagnostics Only") // not documented
	PORT_DIPNAME(0x08, 0x08, "SCSI Termination") PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(0x00, "Installed")
	PORT_DIPSETTING(0x08, "Software Controlled")
	PORT_DIPNAME(0x70, 0x70, "BIOS Address") PORT_DIPLOCATION("S1:6,7,8")
	PORT_DIPSETTING(0x00, "Disabled")
	PORT_DIPSETTING(0x20, "C8000h")
	PORT_DIPSETTING(0x30, "CC000h")
	PORT_DIPSETTING(0x40, "D0000h")
	PORT_DIPSETTING(0x50, "D4000h")
	PORT_DIPSETTING(0x60, "D8000h")
	PORT_DIPSETTING(0x70, "DC000h")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("FDC_CONFIG")
	PORT_DIPNAME(0x1, 0x1, "Floppy Disk Controller") PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING(0x0, "Disabled")
	PORT_DIPSETTING(0x1, "Enabled")
INPUT_PORTS_END

static INPUT_PORTS_START( aha1542cp )
	PORT_START("SWITCHES")
	PORT_DIPNAME(0x0e, 0x0e, "I/O Port Address") PORT_DIPLOCATION("S1:2,3,4")
	PORT_DIPSETTING(0x0e, "330-333h")
	PORT_DIPSETTING(0x0c, "334-337h")
	PORT_DIPSETTING(0x0a, "230-233h")
	PORT_DIPSETTING(0x08, "234-237h")
	PORT_DIPSETTING(0x06, "130-133h")
	PORT_DIPSETTING(0x04, "134-137h")
	PORT_DIPSETTING(0x00, "Diagnostics Only") // not documented
	PORT_DIPNAME(0x10, 0x10, "Plug and Play") PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(0x00, "Disabled")
	PORT_DIPSETTING(0x10, "Enabled")
	PORT_DIPNAME(0xe0, 0xe0, "BIOS Address") PORT_DIPLOCATION("S1:6,7,8")
	PORT_DIPSETTING(0x00, "Disabled")
	PORT_DIPSETTING(0x40, "C8000h")
	PORT_DIPSETTING(0x60, "CC000h")
	PORT_DIPSETTING(0x80, "D0000h")
	PORT_DIPSETTING(0xa0, "D4000h")
	PORT_DIPSETTING(0xc0, "D8000h")
	PORT_DIPSETTING(0xe0, "DC000h")

	PORT_START("FDC_CONFIG")
	PORT_DIPNAME(0x1, 0x1, "Floppy Disk Controller") PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING(0x0, "Disabled")
	PORT_DIPSETTING(0x1, "Enabled")
INPUT_PORTS_END

ioport_constructor aha1542c_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( aha1542c );
}

ioport_constructor aha1542cp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( aha1542cp );
}

const tiny_rom_entry *aha1542c_device::device_rom_region() const
{
	return ROM_NAME( aha1542c );
}

const tiny_rom_entry *aha1542cf_device::device_rom_region() const
{
	return ROM_NAME( aha1542cf );
}

const tiny_rom_entry *aha1542cp_device::device_rom_region() const
{
	return ROM_NAME( aha1542cp );
}

void aha1542c_device::device_add_mconfig(machine_config &config)
{
	z80_device &cpu(Z80(config, Z84C0010_TAG, 10'000'000));
	cpu.set_addrmap(AS_PROGRAM, &aha1542c_device::z84c0010_mem);

	EEPROM_93C46_16BIT(config, m_eeprom);
}

void aha1542cp_device::device_add_mconfig(machine_config &config)
{
	z80_device &cpu(Z80(config, Z84C0010_TAG, 10'000'000));
	cpu.set_addrmap(AS_PROGRAM, &aha1542cp_device::local_mem);

	EEPROM_93C46_16BIT(config, m_eeprom);
}

aha1542c_device::aha1542c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_eeprom(*this, "eeprom")
	, m_switches(*this, "SWITCHES")
{
}

aha1542c_device::aha1542c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: aha1542c_device(mconfig, AHA1542C, tag, owner, clock)
{
}

aha1542cf_device::aha1542cf_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: aha1542c_device(mconfig, AHA1542CF, tag, owner, clock)
{
}

aha1542cp_device::aha1542cp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: aha1542c_device(mconfig, AHA1542CP, tag, owner, clock)
{
}

void aha1542c_device::device_start()
{
	set_isa_device();
	m_isa->install_rom(this, 0xdc000, 0xdffff, "aha1542");
	m_isa->install_device(0x330, 0x333,
			read8sm_delegate(*this, FUNC(aha1542cf_device::aha1542_r)),
			write8sm_delegate(*this, FUNC(aha1542cf_device::aha1542_w)));
}


void aha1542c_device::device_reset()
{
}
