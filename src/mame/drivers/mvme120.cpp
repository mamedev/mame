// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/*
 *	Motorola MVME120 CPU board.
 *  This is an early MVME system from 1984, using standard Motorola parts
 *  instead of the many ASICs of later boards.
 *
 *	The following configurations were available:
 *  MVME120 - 10MHz 68010, 128KB RAM, 4KB SRAM cache, 68451 MMU
 *  MVME121 - 10MHz 68010, 512KB RAM, 4KB SRAM cache, 68451 MMU
 *  MVME122 - 12.5MHz 68010, 128KB RAM, no cache, no MMU
 *  MVME123 - 12.5MHz 68010, 512KB RAM, 4KB SRAM cache, no MMU

 */
#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "bus/vme/vme.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "machine/mc68901.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04

#define VERBOSE 0
//#define VERBOSE (LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)

#if VERBOSE & LOG_PRINTF
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define MVME120_MASTER_CLOCK 10_MHz_XTAL
#define MVME122_MASTER_CLOCK 12.5_MHz_XTAL

class mvme120_state : public driver_device
{
public:
mvme120_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device (mconfig, type, tag)
		, m_maincpu (*this, "maincpu")
		, m_mfp (*this, "mfp")
	{
	}
	
	void mvme120(machine_config &config);
	void mvme121(machine_config &config);
	void mvme122(machine_config &config);
	void mvme123(machine_config &config);
	
private:
	uint16_t bootvect_r(offs_t offset);
	void bootvect_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	
	virtual void machine_start () override;
	virtual void machine_reset () override;
	
	void mvme120_mem(address_map &map);
	void mvme121_mem(address_map &map);
	void mvme122_mem(address_map &map);
	void mvme123_mem(address_map &map);
	
	// add the rest of the devices here...
	required_device<cpu_device> m_maincpu;
	required_device<mc68901_device> m_mfp;

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	uint16_t  *m_sysrom;
	uint16_t  m_sysram[2];
	uint8_t	  m_boot_memory_cycles;

	// "VME120 Control Register"
	uint8_t		m_ctrlreg;
	uint8_t 	ctrlreg_r(offs_t offset);
	void 		ctrlreg_w(offs_t offset, uint8_t data);
	
	// Add the devices' registers and callbacks here...
	DECLARE_WRITE_LINE_MEMBER(watchdog_reset);	
};

void mvme120_state::mvme120_mem(address_map &map)
{
	map(0x000000, 0x000007).ram().w(FUNC(mvme120_state::bootvect_w));       /* After four memory cycles we act as RAM */
	map(0x000000, 0x000007).rom().r(FUNC(mvme120_state::bootvect_r));       /* ROM mirror just during reset */
	map(0x000008, 0x01ffff).ram(); 											/* 128KB RAM */
	map(0xf00000, 0xf0ffff).rom().region("roms", 0x00000); 					/* ROM/EEPROM bank 1 - 120bug */
	map(0xf10000, 0xf1ffff).rom().region("roms", 0x10000);					/* ROM/EEPROM bank 2 - unpopulated */
	map(0xf20000, 0xf2003f).rw("mfp", FUNC(mc68901_device::read), FUNC(mc68901_device::write)).mirror(0x1ffc0).umask16(0x00ff);
	map(0xf40000, 0xf40000).rw(FUNC(mvme120_state::ctrlreg_r), FUNC(mvme120_state::ctrlreg_w)).mirror(0x1fffc);
	
	// $F60000-F6003F 68451 MMU, mirrored to $F7FFFF
	
	// $F80002-F80003 clear cache bank   2	
	// $F80004-F80005 clear cache bank 1
	// $F80006-F80007 clear cache bank 1+2
	// (above mirrored to $F9FFFF)
	
	// $FA0000-$FEFFFF VMEbus
	// $FF0000-$FFFFFF VMEbus short I/O page
}

void mvme120_state::mvme121_mem(address_map &map)
{
	map(0x000000, 0x000007).ram().w(FUNC(mvme120_state::bootvect_w));       /* After four memory cycles we act as RAM */
	map(0x000000, 0x000007).rom().r(FUNC(mvme120_state::bootvect_r));       /* ROM mirror just during reset */
	map(0x000008, 0x07ffff).ram(); 											/* 512KB RAM */
	map(0xf00000, 0xf0ffff).rom().region("roms", 0x00000); 					/* ROM/EEPROM bank 1 - 120bug */
	map(0xf10000, 0xf1ffff).rom().region("roms", 0x10000); 					/* ROM/EEPROM bank 2 - unpopulated */
	map(0xf20000, 0xf2003f).rw("mfp", FUNC(mc68901_device::read), FUNC(mc68901_device::write)).mirror(0x1ffc0).umask16(0x00ff);
	map(0xf40000, 0xf40000).rw(FUNC(mvme120_state::ctrlreg_r), FUNC(mvme120_state::ctrlreg_w)).mirror(0x1fffc);
	// $F60000-F6003F MMU, mirrored to $F7FFFF
	
	// $F80002-F80003 clear cache bank   2	
	// $F80004-F80005 clear cache bank 1
	// $F80006-F80007 clear cache bank 1+2
	// (above mirrored to $F9FFFF)
	
	// $FA0000-$FEFFFF VMEbus
	// $FF0000-$FFFFFF VMEbus short I/O page
}

void mvme120_state::mvme122_mem(address_map &map)
{
	map(0x000000, 0x000007).ram().w(FUNC(mvme120_state::bootvect_w));       /* After four memory cycles we act as RAM */
	map(0x000000, 0x000007).rom().r(FUNC(mvme120_state::bootvect_r));       /* ROM mirror just during reset */
	map(0x000008, 0x01ffff).ram(); 											/* 128KB RAM */
	map(0xf00000, 0xf0ffff).rom().region("roms", 0x00000); 					/* ROM/EEPROM bank 1 - 120bug */
	map(0xf10000, 0xf1ffff).rom().region("roms", 0x10000); 					/* ROM/EEPROM bank 2 - unpopulated */
	map(0xf20000, 0xf2003f).rw("mfp", FUNC(mc68901_device::read), FUNC(mc68901_device::write)).mirror(0x1ffc0).umask16(0x00ff);
	map(0xf40000, 0xf40000).rw(FUNC(mvme120_state::ctrlreg_r), FUNC(mvme120_state::ctrlreg_w)).mirror(0x1fffc);
	
	// $F80002-F80003 clear cache bank   2	
	// $F80004-F80005 clear cache bank 1
	// $F80006-F80007 clear cache bank 1+2
	// (above mirrored to $F9FFFF)
	
	// $FA0000-$FEFFFF VMEbus
	// $FF0000-$FFFFFF VMEbus short I/O page
}

void mvme120_state::mvme123_mem(address_map &map)
{
	map(0x000000, 0x000007).ram().w(FUNC(mvme120_state::bootvect_w));       /* After four memory cycles we act as RAM */
	map(0x000000, 0x000007).rom().r(FUNC(mvme120_state::bootvect_r));       /* ROM mirror just during reset */
	map(0x000008, 0x07ffff).ram(); 											/* 512KB RAM */
	map(0xf00000, 0xf0ffff).rom().region("roms", 0x00000); 					/* ROM/EEPROM bank 1 - 120bug */
	map(0xf10000, 0xf1ffff).rom().region("roms", 0x10000);					/* ROM/EEPROM bank 2 - unpopulated */
	map(0xf20000, 0xf2003f).rw("mfp", FUNC(mc68901_device::read), FUNC(mc68901_device::write)).mirror(0x1ffc0).umask16(0x00ff);
	map(0xf40000, 0xf40000).rw(FUNC(mvme120_state::ctrlreg_r), FUNC(mvme120_state::ctrlreg_w)).mirror(0x1fffc);
	
	// $F80002-F80003 clear cache bank   2	
	// $F80004-F80005 clear cache bank 1
	// $F80006-F80007 clear cache bank 1+2
	// (above mirrored to $F9FFFF)
	
	// $FA0000-$FEFFFF VMEbus
	// $FF0000-$FFFFFF VMEbus short I/O page
}

/* Input ports */
static INPUT_PORTS_START (mvme120)
INPUT_PORTS_END

/* Start it up */
void mvme120_state::machine_start ()
{
	LOG("%s\n", FUNCNAME);

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint16_t*)(memregion ("roms")->base ());
	m_boot_memory_cycles = 0;
	m_ctrlreg = 0xFF;		/* all bits are set to 1 when /RESET asserted */
	
	m_mfp->tai_w(1);		/* Tied to +5V */

}

void mvme120_state::machine_reset ()
{
	LOG("%s\n", FUNCNAME);

	/* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
	if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
		m_sysrom = (uint16_t*)(memregion ("roms")->base ());
	m_boot_memory_cycles = 0;
	m_ctrlreg = 0xFF;		/* all bits are set to 1 when /RESET asserted */
}

//
WRITE_LINE_MEMBER(mvme120_state::watchdog_reset)
{
	if (state) {
		printf("mvme120_state::watchdog_reset\n");
		logerror("mvme120_state::watchdog_reset\n");
		machine().schedule_soft_reset();
	}
}

uint8_t mvme120_state::ctrlreg_r(offs_t offset){
	// Control Register
	// b0 - BRDFAIL
	// b1 - /CTS
	// b2 - /IE
	// b3 - /PAREN
	// b4 - CACHEN
	// b5 - FREEZE
	// b6 - /ALTCLR
	// b7 - /WWP
	
	return m_ctrlreg;
}

void mvme120_state::ctrlreg_w(offs_t offset, uint8_t data){
	LOG("%s: vme120 control register set to $%02X\n", FUNCNAME, data);
	m_ctrlreg = data;
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xfff00000 to 0x0 at reset*/
uint16_t mvme120_state::bootvect_r(offs_t offset){
	if(m_boot_memory_cycles++ >= 4)
	{
		m_sysrom = &m_sysram[0]; // We're RAM again.
		m_boot_memory_cycles = 4;
	}
	
	return m_sysrom[offset];
}

void mvme120_state::bootvect_w(offs_t offset, uint16_t data, uint16_t mem_mask){
	m_sysram[offset % std::size(m_sysram)] &= ~mem_mask;
	m_sysram[offset % std::size(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcoming accesses to masking RAM until reset.
}

static const input_device_default terminal_defaults[] =
{
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
	{ nullptr, 0, 0 }
};

/*
 * Machine configuration
 */
void mvme120_state::mvme120(machine_config &config)
{
	/* basic machine hardware */
	M68010(config, m_maincpu, MVME120_MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &mvme120_state::mvme120_mem);
	
	MC68901(config, m_mfp, MVME120_MASTER_CLOCK / 4);
	m_mfp->set_timer_clock(MVME120_MASTER_CLOCK / 4);
	m_mfp->out_so_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	m_mfp->out_tao_cb().set("mfp", FUNC(mc68901_device::tbi_w));
	m_mfp->out_tbo_cb().set(FUNC(mvme120_state::watchdog_reset));
	m_mfp->out_tco_cb().set("mfp", FUNC(mc68901_device::rc_w));
	m_mfp->out_tco_cb().append("mfp", FUNC(mc68901_device::tc_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("mfp", FUNC(mc68901_device::si_w));
	rs232.set_option_device_input_defaults("terminal", terminal_defaults);

	// Missing: MMU, VMEbus
	//VME(config, "vme", 0);
	//VME_SLOT(config, "slot1", mvme120_vme_cards, nullptr, 1, "vme");
}

void mvme120_state::mvme121(machine_config &config)
{
	mvme120(config);
}

void mvme120_state::mvme122(machine_config &config)
{
	mvme120(config);
	
	m_maincpu->set_clock(MVME122_MASTER_CLOCK);
	
	m_mfp->set_clock(MVME120_MASTER_CLOCK / 4);
	m_mfp->set_timer_clock(MVME122_MASTER_CLOCK / 4);
}

void mvme120_state::mvme123(machine_config &config)
{
	mvme122(config);
}

/* ROM definitions */
ROM_START (mvme120)
	ROM_REGION16_BE(0x20000, "roms", 0)
	ROM_DEFAULT_BIOS("120bug-v2.0")

	ROM_SYSTEM_BIOS(0, "120bug-v2.0", "MVME120 120bug v2.0")
	ROMX_LOAD("120bug-2.0-u44.bin", 0x0000, 0x4000, CRC (87d62dac) SHA1 (c57eb9f8aefe29794b8fc5f0afbaff9b59d38c73), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("120bug-2.0-u52.bin", 0x0001, 0x4000, CRC (5651b61d) SHA1 (0d0004dff3c88b2f0b18951b4f2acd7f65f701b1), ROM_SKIP(1) | ROM_BIOS(0))
	
	ROM_SYSTEM_BIOS(1, "120bug-v1.1", "MVME120 120bug v1.1")
	ROMX_LOAD("120bug-1.1-u44.bin", 0x0000, 0x4000, CRC (87d62dac) SHA1 (c57eb9f8aefe29794b8fc5f0afbaff9b59d38c73), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("120bug-1.1-u52.bin", 0x0001, 0x4000, CRC (5651b61d) SHA1 (0d0004dff3c88b2f0b18951b4f2acd7f65f701b1), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

ROM_START (mvme121)
	ROM_REGION16_BE(0x20000, "roms", 0)
	ROM_DEFAULT_BIOS("120bug-v2.0")

	ROM_SYSTEM_BIOS(0, "120bug-v2.0", "MVME120 120bug v2.0")
	ROMX_LOAD("120bug-2.0-u44.bin", 0x0000, 0x4000, CRC (87d62dac) SHA1 (c57eb9f8aefe29794b8fc5f0afbaff9b59d38c73), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("120bug-2.0-u52.bin", 0x0001, 0x4000, CRC (5651b61d) SHA1 (0d0004dff3c88b2f0b18951b4f2acd7f65f701b1), ROM_SKIP(1) | ROM_BIOS(0))
	
	ROM_SYSTEM_BIOS(1, "120bug-v1.1", "MVME120 120bug v1.1")
	ROMX_LOAD("120bug-1.1-u44.bin", 0x0000, 0x4000, CRC (87d62dac) SHA1 (c57eb9f8aefe29794b8fc5f0afbaff9b59d38c73), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("120bug-1.1-u52.bin", 0x0001, 0x4000, CRC (5651b61d) SHA1 (0d0004dff3c88b2f0b18951b4f2acd7f65f701b1), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

ROM_START (mvme122)
	ROM_REGION16_BE(0x20000, "roms", 0)
	ROM_DEFAULT_BIOS("120bug-v2.0")

	ROM_SYSTEM_BIOS(0, "120bug-v2.0", "MVME120 120bug v2.0")
	ROMX_LOAD("120bug-2.0-u44.bin", 0x0000, 0x4000, CRC (87d62dac) SHA1 (c57eb9f8aefe29794b8fc5f0afbaff9b59d38c73), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("120bug-2.0-u52.bin", 0x0001, 0x4000, CRC (5651b61d) SHA1 (0d0004dff3c88b2f0b18951b4f2acd7f65f701b1), ROM_SKIP(1) | ROM_BIOS(0))
	
	ROM_SYSTEM_BIOS(1, "120bug-v1.1", "MVME120 120bug v1.1")
	ROMX_LOAD("120bug-1.1-u44.bin", 0x0000, 0x4000, CRC (87d62dac) SHA1 (c57eb9f8aefe29794b8fc5f0afbaff9b59d38c73), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("120bug-1.1-u52.bin", 0x0001, 0x4000, CRC (5651b61d) SHA1 (0d0004dff3c88b2f0b18951b4f2acd7f65f701b1), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

ROM_START (mvme123)
	ROM_REGION16_BE(0x20000, "roms", 0)
	ROM_DEFAULT_BIOS("120bug-v2.0")

	ROM_SYSTEM_BIOS(0, "120bug-v2.0", "MVME120 120bug v2.0")
	ROMX_LOAD("120bug-2.0-u44.bin", 0x0000, 0x4000, CRC (87d62dac) SHA1 (c57eb9f8aefe29794b8fc5f0afbaff9b59d38c73), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("120bug-2.0-u52.bin", 0x0001, 0x4000, CRC (5651b61d) SHA1 (0d0004dff3c88b2f0b18951b4f2acd7f65f701b1), ROM_SKIP(1) | ROM_BIOS(0))
	
	ROM_SYSTEM_BIOS(1, "120bug-v1.1", "MVME120 120bug v1.1")
	ROMX_LOAD("120bug-1.1-u44.bin", 0x0000, 0x4000, CRC (87d62dac) SHA1 (c57eb9f8aefe29794b8fc5f0afbaff9b59d38c73), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("120bug-1.1-u52.bin", 0x0001, 0x4000, CRC (5651b61d) SHA1 (0d0004dff3c88b2f0b18951b4f2acd7f65f701b1), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

/* Driver */
//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY     FULLNAME    FLAGS
COMP( 1984, mvme120, 0,       0,      mvme120, mvme120, mvme120_state, empty_init, "Motorola", "MVME-120", MACHINE_IS_SKELETON )
COMP( 1984, mvme121, mvme120, 0,      mvme121, mvme120, mvme120_state, empty_init, "Motorola", "MVME-121", MACHINE_IS_SKELETON )
COMP( 1984, mvme122, mvme120, 0,      mvme122, mvme120, mvme120_state, empty_init, "Motorola", "MVME-122", MACHINE_IS_SKELETON )
COMP( 1984, mvme123, mvme120, 0,      mvme123, mvme120, mvme120_state, empty_init, "Motorola", "MVME-123", MACHINE_IS_SKELETON )
