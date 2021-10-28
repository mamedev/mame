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
 *
 *  Current state, it crashes at $F058D8 while testing CPU exception handling.
 *  If you skip over that address (pc=F058E2 in the debugger) it continues
 *  through the self-test.
 *
 *  Looks like you also have to reboot the system once before the terminal
 *  works properly? Still working on that.
 */
#include "vme_mvme120.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04

//#define VERBOSE 0
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

#define MVME120_MASTER_CLOCK 	20_MHz_XTAL
#define MVME122_MASTER_CLOCK 	25_MHz_XTAL

#define MVME120_CPU_CLOCK		( MVME120_MASTER_CLOCK / 2 )
#define MVME122_CPU_CLOCK		( MVME122_MASTER_CLOCK / 2 )

DEFINE_DEVICE_TYPE(VME_MVME120,   vme_mvme120_card_device,   "mvme120",   "Motorola MVME-120")
DEFINE_DEVICE_TYPE(VME_MVME121,   vme_mvme121_card_device,   "mvme121",   "Motorola MVME-121")
DEFINE_DEVICE_TYPE(VME_MVME122,   vme_mvme122_card_device,   "mvme122",   "Motorola MVME-122")
DEFINE_DEVICE_TYPE(VME_MVME123,   vme_mvme123_card_device,   "mvme123",   "Motorola MVME-123")

INPUT_PORTS_START(mvme120)
	PORT_START("S3")
	
	// described as "autoboot" and "cache disable" in the manual
	PORT_DIPNAME(0x01, 0x00, DEF_STR( Unknown ) )  	PORT_DIPLOCATION("S3:1")	PORT_CHANGED_MEMBER(DEVICE_SELF, vme_mvme120_device, s3_autoboot, 0)
	PORT_DIPSETTING(   0x01, DEF_STR( On ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	
	PORT_DIPNAME(0x02, 0x00, "Baud Rate Select")    PORT_DIPLOCATION("S3:2")	PORT_CHANGED_MEMBER(DEVICE_SELF, vme_mvme120_device, s3_baudrate, 0)
	PORT_DIPSETTING(   0x02, "10.0MHz CPU")
	PORT_DIPSETTING(   0x00, "12.5MHz CPU")
	
	PORT_DIPNAME(0x0C, 0x08, "Reset Vector Source")	PORT_DIPLOCATION("S3:3,4")	//
	PORT_DIPSETTING(   0x08, "Onboard ROM")
	PORT_DIPSETTING(   0x04, "VMEbus")
INPUT_PORTS_END

ioport_constructor vme_mvme120_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mvme120);
}

vme_mvme120_device::vme_mvme120_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, mvme120_board_t board_id) :
	device_t(mconfig, type, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_maincpu (*this, "maincpu")
	, m_maincpu_region (*this, "maincpu")
	, m_mfp (*this, "mfp")
	, m_rs232 (*this, "rs232")
	, m_input_s3(*this, "S3")
	, m_board_id(board_id)
{

}

vme_mvme120_card_device::vme_mvme120_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: vme_mvme120_card_device(mconfig, VME_MVME120, tag, owner, clock)
{

}

vme_mvme121_card_device::vme_mvme121_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: vme_mvme121_card_device(mconfig, VME_MVME121, tag, owner, clock)
{

}

vme_mvme122_card_device::vme_mvme122_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: vme_mvme122_card_device(mconfig, VME_MVME122, tag, owner, clock)
{
}

vme_mvme123_card_device::vme_mvme123_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: vme_mvme123_card_device(mconfig, VME_MVME123, tag, owner, clock)
{
	
}

void vme_mvme120_device::mvme120_mem(address_map &map)
{
	map(0x000000, 0x000007).ram().w(FUNC(vme_mvme120_card_device::bootvect_w));       // After four memory cycles we act as RAM
	map(0x000000, 0x000007).rom().r(FUNC(vme_mvme120_card_device::bootvect_r));       // ROM mirror just during reset
	map(0x000008, 0x01ffff).ram();
	map(0x020000, 0xefffff).rw(FUNC(vme_mvme120_card_device::vme_a24_r), FUNC(vme_mvme120_card_device::vme_a24_w)); // VMEbus 24-bit addresses
	map(0xf00000, 0xf0ffff).rom().region("maincpu", 0x00000);				// ROM/EEPROM bank 1 - 120bug
	map(0xf10000, 0xf1ffff).rom().region("maincpu", 0x10000);				// ROM/EEPROM bank 2 - unpopulated
	map(0xf20000, 0xf2003f).rw("mfp", FUNC(mc68901_device::read), FUNC(mc68901_device::write)).mirror(0x1ffc0).umask16(0x00ff);
	map(0xf40000, 0xf40000).rw(FUNC(vme_mvme120_card_device::ctrlreg_r), FUNC(vme_mvme120_card_device::ctrlreg_w)).mirror(0x1fffc);
	// $F60000-F6003F 68451 MMU, mirrored to $F7FFFF
	map(0xfa0000, 0xfeffff).rw(FUNC(vme_mvme120_card_device::vme_a24_r), FUNC(vme_mvme120_card_device::vme_a24_w)); // VMEbus 24-bit addresses
	map(0xff0000, 0xffffff).rw(FUNC(vme_mvme120_card_device::vme_a16_r), FUNC(vme_mvme120_card_device::vme_a16_w)); // VMEbus 16-bit addresses
	
	// $F80002-F80003 clear cache bank   2	
	// $F80004-F80005 clear cache bank 1
	// $F80006-F80007 clear cache bank 1+2
	// (above mirrored to $F9FFFF)
}

void vme_mvme120_device::mvme121_mem(address_map &map)
{
	map(0x000000, 0x000007).ram().w(FUNC(vme_mvme120_card_device::bootvect_w));		// After four memory cycles we act as RAM
	map(0x000000, 0x000007).rom().r(FUNC(vme_mvme120_card_device::bootvect_r));		// ROM mirror just during reset
	map(0x000008, 0x07ffff).ram();
	map(0x080000, 0xefffff).rw(FUNC(vme_mvme120_card_device::vme_a24_r), FUNC(vme_mvme120_card_device::vme_a24_w)); // VMEbus 24-bit addresses
	map(0xf00000, 0xf0ffff).rom().region("maincpu", 0x00000);				// ROM/EEPROM bank 1 - 120bug
	map(0xf10000, 0xf1ffff).rom().region("maincpu", 0x10000);				// ROM/EEPROM bank 2 - unpopulated
	map(0xf20000, 0xf2003f).rw("mfp", FUNC(mc68901_device::read), FUNC(mc68901_device::write)).mirror(0x1ffc0).umask16(0x00ff);
	map(0xf40000, 0xf40000).rw(FUNC(vme_mvme120_card_device::ctrlreg_r), FUNC(vme_mvme120_card_device::ctrlreg_w)).mirror(0x1fffc);
	// $F60000-F6003F 68451 MMU, mirrored to $F7FFFF
	map(0xfa0000, 0xfeffff).rw(FUNC(vme_mvme120_card_device::vme_a24_r), FUNC(vme_mvme120_card_device::vme_a24_w)); // VMEbus 24-bit addresses
	map(0xff0000, 0xffffff).rw(FUNC(vme_mvme120_card_device::vme_a16_r), FUNC(vme_mvme120_card_device::vme_a16_w)); // VMEbus 16-bit addresses
	
	// $F80002-F80003 clear cache bank   2	
	// $F80004-F80005 clear cache bank 1
	// $F80006-F80007 clear cache bank 1+2
	// (above mirrored to $F9FFFF)
}

void vme_mvme120_device::mvme122_mem(address_map &map)
{
	map(0x000000, 0x000007).ram().w(FUNC(vme_mvme120_card_device::bootvect_w));       // After four memory cycles we act as RAM
	map(0x000000, 0x000007).rom().r(FUNC(vme_mvme120_card_device::bootvect_r));       // ROM mirror just during reset
	map(0x000008, 0x01ffff).ram();
	map(0x020000, 0xefffff).rw(FUNC(vme_mvme120_card_device::vme_a24_r), FUNC(vme_mvme120_card_device::vme_a24_w)); // VMEbus 24-bit addresses
	map(0xf00000, 0xf0ffff).rom().region("maincpu", 0x00000);				// ROM/EEPROM bank 1 - 120bug
	map(0xf10000, 0xf1ffff).rom().region("maincpu", 0x10000);				// ROM/EEPROM bank 2 - unpopulated
	map(0xf20000, 0xf2003f).rw("mfp", FUNC(mc68901_device::read), FUNC(mc68901_device::write)).mirror(0x1ffc0).umask16(0x00ff);
	map(0xf40000, 0xf40000).rw(FUNC(vme_mvme120_card_device::ctrlreg_r), FUNC(vme_mvme120_card_device::ctrlreg_w)).mirror(0x1fffc);
	// $F60000-F6003F 68451 MMU, mirrored to $F7FFFF
	map(0xfa0000, 0xfeffff).rw(FUNC(vme_mvme120_card_device::vme_a24_r), FUNC(vme_mvme120_card_device::vme_a24_w)); // VMEbus 24-bit addresses
	map(0xff0000, 0xffffff).rw(FUNC(vme_mvme120_card_device::vme_a16_r), FUNC(vme_mvme120_card_device::vme_a16_w)); // VMEbus 16-bit addresses
	
	// $F80002-F80003 clear cache bank   2	
	// $F80004-F80005 clear cache bank 1
	// $F80006-F80007 clear cache bank 1+2
	// (above mirrored to $F9FFFF)
}

void vme_mvme120_device::mvme123_mem(address_map &map)
{
	map(0x000000, 0x000007).ram().w(FUNC(vme_mvme120_card_device::bootvect_w));       // After four memory cycles we act as RAM
	map(0x000000, 0x000007).rom().r(FUNC(vme_mvme120_card_device::bootvect_r));       // ROM mirror just during reset
	map(0x000008, 0x07ffff).ram();
	map(0x020000, 0xefffff).rw(FUNC(vme_mvme120_card_device::vme_a24_r), FUNC(vme_mvme120_card_device::vme_a24_w)); // VMEbus 24-bit addresses
	map(0xf00000, 0xf0ffff).rom().region("maincpu", 0x00000);				// ROM/EEPROM bank 1 - 120bug
	map(0xf10000, 0xf1ffff).rom().region("maincpu", 0x10000);				// ROM/EEPROM bank 2 - unpopulated
	map(0xf20000, 0xf2003f).rw("mfp", FUNC(mc68901_device::read), FUNC(mc68901_device::write)).mirror(0x1ffc0).umask16(0x00ff);
	map(0xf40000, 0xf40000).rw(FUNC(vme_mvme120_card_device::ctrlreg_r), FUNC(vme_mvme120_card_device::ctrlreg_w)).mirror(0x1fffc);
	// $F60000-F6003F 68451 MMU, mirrored to $F7FFFF
	map(0xfa0000, 0xfeffff).rw(FUNC(vme_mvme120_card_device::vme_a24_r), FUNC(vme_mvme120_card_device::vme_a24_w)); // VMEbus 24-bit addresses
	map(0xff0000, 0xffffff).rw(FUNC(vme_mvme120_card_device::vme_a16_r), FUNC(vme_mvme120_card_device::vme_a16_w)); // VMEbus 16-bit addresses
	
	// $F80002-F80003 clear cache bank   2	
	// $F80004-F80005 clear cache bank 1
	// $F80006-F80007 clear cache bank 1+2
	// (above mirrored to $F9FFFF)
}

void vme_mvme120_device::device_start ()
{
	LOG("%s\n", FUNCNAME);

	// Setup pointer to bootvector in ROM for bootvector handler bootvect_r
	m_sysrom = (uint16_t *)m_maincpu_region->base();
	m_boot_memory_cycles = 0;
	m_ctrlreg = 0xFF;		// all bits are set to 1 when /RESET asserted
	
	m_mfp->tai_w(1);		// Tied to +5V

}

void vme_mvme120_device::device_reset ()
{
	LOG("%s\n", FUNCNAME);

	// Reset pointer to bootvector in ROM for bootvector handler bootvect_r
	if (m_sysrom == &m_sysram[0]) // Condition needed because memory map is not setup first time
		m_sysrom = (uint16_t *)m_maincpu_region->base();
	m_boot_memory_cycles = 0;
	m_ctrlreg = 0xFF;		// all bits are set to 1 when /RESET asserted
}

//
WRITE_LINE_MEMBER(vme_mvme120_device::watchdog_reset)
{
	if (state) {
		printf("vme_mvme120_card_device::watchdog_reset\n");
		logerror("vme_mvme120_card_device::watchdog_reset\n");
		machine().schedule_soft_reset();
	}
}

WRITE_LINE_MEMBER(vme_mvme120_device::mfp_interrupt)
{
	// MFP interrupts are gated by bit 2 of the control register.
	if(state && !BIT(m_ctrlreg, 2))
	{
		LOG("%s: MFP interrupt triggered 68K IRQ6\n", FUNCNAME);
		m_maincpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
	}
	if(!state)
	{
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}
}

// Dummy VMEbus access
uint16_t vme_mvme120_device::vme_a24_r()
{
	//LOG("%s\n", FUNCNAME);
	
	// Simulate VMEbus timeout.
	m_mfp->i2_w(ASSERT_LINE);
	m_mfp->i2_w(CLEAR_LINE);
	
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	
	return 0;
}

void vme_mvme120_device::vme_a24_w(uint16_t data)
{	
	// Simulate VMEbus timeout.
	m_mfp->i2_w(ASSERT_LINE);
	m_mfp->i2_w(CLEAR_LINE);
	
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	
	//LOG("%s\n", FUNCNAME);
}

uint16_t vme_mvme120_device::vme_a16_r()
{
	//LOG("%s\n", FUNCNAME);
	return 0;
}

void vme_mvme120_device::vme_a16_w(uint16_t data)
{
	//LOG("%s\n", FUNCNAME);
}

uint8_t vme_mvme120_device::ctrlreg_r(offs_t offset)
{
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

void vme_mvme120_device::ctrlreg_w(offs_t offset, uint8_t data)
{
	LOG("%s: vme120 control register set to $%02X\n", FUNCNAME, data);
	m_ctrlreg = data;
	
	// Set lines according to the new ctrlreg status.
	m_rs232->write_rts(!BIT(m_ctrlreg, 1));
}

// Boot vector handler, the PCB hardwires the first 8 bytes from 0xfff00000 to 0x0 at reset
uint16_t vme_mvme120_device::bootvect_r(offs_t offset)
{
	if(m_boot_memory_cycles++ >= 4)
	{
		m_sysrom = &m_sysram[0]; // We're RAM again.
		m_boot_memory_cycles = 4;
	}
	
	return m_sysrom[offset];
}

void vme_mvme120_device::bootvect_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_sysram[offset % std::size(m_sysram)] &= ~mem_mask;
	m_sysram[offset % std::size(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcoming accesses to masking RAM until reset.
}

static const input_device_default terminal_defaults[] =
{
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
	{ nullptr, 0, 0 }
};

/*
 * Machine configuration
 */
void vme_mvme120_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	M68010(config, m_maincpu, MVME120_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &vme_mvme120_card_device::mvme120_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_mvme120_card_device::mvme120_mem);

	
	MC68901(config, m_mfp, MVME120_CPU_CLOCK / 4);
	m_mfp->set_timer_clock(MVME120_CPU_CLOCK / 4);
	m_mfp->out_so_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	m_mfp->out_tao_cb().set("mfp", FUNC(mc68901_device::tbi_w));
	m_mfp->out_tbo_cb().set(FUNC(vme_mvme120_card_device::watchdog_reset));
	m_mfp->out_tco_cb().set("mfp", FUNC(mc68901_device::rc_w));
	m_mfp->out_tco_cb().append("mfp", FUNC(mc68901_device::tc_w));
	m_mfp->out_irq_cb().set(FUNC(vme_mvme120_card_device::mfp_interrupt));

	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	m_rs232->rxd_handler().set("mfp", FUNC(mc68901_device::si_w));
	m_rs232->set_option_device_input_defaults("terminal", terminal_defaults);

	// Missing: MMU, VMEbus
	VME(config, "vme", 0);
}
 
void vme_mvme120_card_device::device_add_mconfig(machine_config &config)
{
	vme_mvme120_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vme_mvme120_card_device::mvme120_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_mvme120_card_device::mvme120_mem);
}

void vme_mvme121_card_device::device_add_mconfig(machine_config &config)
{
	vme_mvme120_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vme_mvme121_card_device::mvme121_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_mvme121_card_device::mvme121_mem);
}

void vme_mvme122_card_device::device_add_mconfig(machine_config &config)
{
	vme_mvme120_device::device_add_mconfig(config);
	
	m_maincpu->set_addrmap(AS_PROGRAM, &vme_mvme122_card_device::mvme122_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_mvme122_card_device::mvme122_mem);
	m_maincpu->set_clock(MVME122_CPU_CLOCK);
	
	m_mfp->set_clock(MVME122_CPU_CLOCK / 4);
	m_mfp->set_timer_clock(MVME122_CPU_CLOCK / 4);
}

void vme_mvme123_card_device::device_add_mconfig(machine_config &config)
{
	vme_mvme120_device::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vme_mvme123_card_device::mvme123_mem);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_mvme123_card_device::mvme123_mem);
	
	m_maincpu->set_clock(MVME122_CPU_CLOCK);
	
	m_mfp->set_clock(MVME122_CPU_CLOCK / 4);
	m_mfp->set_timer_clock(MVME122_CPU_CLOCK / 4);
}

// DIP switch and jumpers
INPUT_CHANGED_MEMBER(vme_mvme120_device::s3_autoboot)
{
	// TODO: verify
	//m_mfp->i0_w(BIT(m_input_s3->read(), 0));
}

INPUT_CHANGED_MEMBER(vme_mvme120_device::s3_baudrate)
{
	// TODO: verify
	//m_mfp->i1_w(BIT(m_input_s3->read(), 1));
}

// ROM definitions
ROM_START (mvme120)
	ROM_REGION16_BE(0x20000, "maincpu", 0)
	ROM_DEFAULT_BIOS("12xbug-v2.0")

	ROM_SYSTEM_BIOS(0, "12xbug-v2.0", "MVME120 12xbug v2.0")
	ROMX_LOAD("12xbug-2.0-u44.bin", 0x0000, 0x4000, CRC(87d62dac) SHA1(c57eb9f8aefe29794b8fc5f0afbaff9b59d38c73), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("12xbug-2.0-u52.bin", 0x0001, 0x4000, CRC(5651b61d) SHA1(0d0004dff3c88b2f0b18951b4f2acd7f65f701b1), ROM_SKIP(1) | ROM_BIOS(0))
	
	ROM_SYSTEM_BIOS(1, "12xbug-v1.1", "MVME120 12xbug v1.1")
	ROMX_LOAD("12xbug-1.1-u44.bin", 0x0000, 0x4000, CRC(bf4d6cf1) SHA1(371bb55611eddeb6231a92af7c1e34d4ec0321b5), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("12xbug-1.1-u52.bin", 0x0001, 0x4000, CRC(76fabe32) SHA1(2f933d0eb46d00db0051ce23c3e53ccef75a2c69), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

const tiny_rom_entry *vme_mvme120_device::device_rom_region() const
{
	return ROM_NAME(mvme120);
}
