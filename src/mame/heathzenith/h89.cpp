// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, Mark Garlanger
/***************************************************************************

  Heathkit H89

    Heath Company made several very similar systems, including
      - H88 - kit, came with a cassette interface board instead of floppy controller
      - H89 - kit, came with a hard-sectored floppy disk controller
      - WH89 - was factory assembled

    Heath's parent company Zenith, also sold systems under the Zenith Data
    Systems brand. These were all factory assembled
      - Z-89 - same as Heath's H89, but assembled
      - Z-90 - came with a soft-sectored floppy disk controller

    Monitor Commands (for MTR-90):
      B Boot
      C Convert (number)
      G Go (address)
      I In (address)
      O Out (address,data)
      R Radix (H/O)
      S Substitute (address)
      T Test Memory
      V View

    Monitor Commands (for MTR-88)
      B Boot
      D Dump - dump a program to cassette
      G Go (address)
      L Load - load a program from cassette
      P Program Counter (address) - select an address in the PC
      S Substitute - inspect or change memory

    Monitor Commands (for MTR-89)
      B Boot
      G Go (address)
      P Program Counter (address) - select an address in the PC
      S Substitute - inspect or change memory

****************************************************************************/

#include "emu.h"

#include "h_88_cass.h"
#include "intr_cntrl.h"
#include "mms77316_fdc.h"
#include "sigmasoft_parallel_port.h"
#include "tlb.h"
#include "z37_fdc.h"

#include "cpu/z80/z80.h"
#include "machine/ins8250.h"
#include "machine/ram.h"
#include "machine/timer.h"

#include "softlist_dev.h"

#include "h89.lh"


// Single Step
#define LOG_SS    (1U << 1)

// #define VERBOSE ( LOG_SS )
#include "logmacro.h"

#define LOGSS(...)    LOGMASKED(LOG_SS,    __VA_ARGS__)


namespace {

/**
 * Base Heathkit H89 functionality
 */
class h89_base_state : public driver_device
{
protected:
	h89_base_state(const machine_config &mconfig, device_type type, const char *tag):
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_maincpu_region(*this, "maincpu"),
		m_mem_view(*this, "rom_bank"),
		m_ram(*this, RAM_TAG),
		m_floppy_ram(*this, "floppyram"),
		m_tlbc(*this, "tlbc"),
		m_intr_socket(*this, "intr_socket"),
		m_console(*this, "console"),
		m_serial1(*this, "serial1"),
		m_serial2(*this, "serial2"),
		m_serial3(*this, "serial3"),
		m_config(*this, "CONFIG")
	{
	}

	void h89_base(machine_config &config);

	required_device<z80_device>                          m_maincpu;
	required_memory_region                               m_maincpu_region;
	memory_view                                          m_mem_view;
	required_device<ram_device>                          m_ram;
	required_shared_ptr<u8>                              m_floppy_ram;
	required_device<heath_tlb_connector>                 m_tlbc;
	required_device<heath_intr_socket>                   m_intr_socket;
	required_device<ins8250_device>                      m_console;
	required_device<ins8250_device>                      m_serial1;
	required_device<ins8250_device>                      m_serial2;
	required_device<ins8250_device>                      m_serial3;
	required_ioport                                      m_config;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;

	// General Purpose Port (GPP)
	u8   m_gpp;

	bool m_rom_enabled;
	bool m_timer_intr_enabled;
	bool m_single_step_enabled;
	bool m_floppy_ram_wp;

	// single step flags
	bool m_555a_latch;
	bool m_555b_latch;
	bool m_556b_latch;

	u32  m_cpu_speed_multiplier;

	// Clocks
	static constexpr XTAL H89_CLOCK                      = XTAL(12'288'000) / 6;
	static constexpr XTAL INS8250_CLOCK                  = XTAL(1'843'200);

	static constexpr u8   GPP_SINGLE_STEP_BIT            = 0;
	static constexpr u8   GPP_ENABLE_TIMER_INTERRUPT_BIT = 1;
	static constexpr u8   GPP_SPEED_SELECT_BIT           = 4;
	static constexpr u8   GPP_DISABLE_ROM_BIT            = 5;
	static constexpr u8   GPP_H17_SIDE_SELECT_BIT        = 6;

	void update_mem_view();

	void update_gpp(u8 gpp);
	void port_f2_w(u8 data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	TIMER_DEVICE_CALLBACK_MEMBER(h89_irq_timer);

	void h89_mem(address_map &map) ATTR_COLD;
	void map_fetch(address_map &map) ATTR_COLD;
	u8 m1_r(offs_t offset);

	void h89_base_io(address_map &map) ATTR_COLD;

	void set_wait_state(int data);

	u8 raise_NMI_r();
	void raise_NMI_w(u8 data);
	void console_intr(int data);
	void reset_line(int data);
	void reset_single_step_state();
};

/**
 * Heathkit H88
 *  - BIOS MTR-88
 *  - H-88-5 Cassette Interface Board
 *
 */
class h88_state : public h89_base_state
{
public:
	h88_state(const machine_config &mconfig, device_type type, const char *tag):
		h89_base_state(mconfig, type, tag),
		m_cassette(*this, "h_88_5")
	{
	}

	void h88(machine_config &config);

protected:
	required_device<heath_h_88_cass_device> m_cassette;

	void h88_io(address_map &map) ATTR_COLD;
};


/**
 * Heathkit H89
 *  - Z-89-37 Soft-sectored Floppy Controller
 *
 */
class h89_state : public h89_base_state
{
public:
	h89_state(const machine_config &mconfig, device_type type, const char *tag):
		h89_base_state(mconfig, type, tag),
		m_h37(*this, "h37")
	{
	}

	void h89(machine_config &config);

protected:
	required_device<heath_z37_fdc_device> m_h37;

	void h89_io(address_map &map) ATTR_COLD;
};


class h89_sigmasoft_state : public h89_state
{
public:
	h89_sigmasoft_state(const machine_config &mconfig, device_type type, const char *tag):
		h89_state(mconfig, type, tag),
		m_sigma_parallel(*this, "sigma_parallel")
	{
	}

	void h89_sigmasoft(machine_config &config);

protected:
	required_device<sigmasoft_parallel_port> m_sigma_parallel;

	void h89_sigmasoft_io(address_map &map) ATTR_COLD;
};


/**
 * Heathkit H89 with MMS hardware
 *  - MMS 77316 - DD controller
 *
 * Functionality already implemented/Same as Heath options
 *  - MMS 77311 - 16k RAM
 *  - MMS 77312 - ORG-0 CP/M Mod
 *
 * Hardware currently planned to be implemented:
 *  - MMS 77318 - 128k RAM board
 *  - MMS 77319 - Video Output
 *  - MMS 77320 - SASI board
 *  - MMS 77322 - Network Controller
 *
 * Other hardware MMS offered
 *  - MMS 77314 - Remex H47 / IMI(Corvus) / 3 serial port
 *  - MMS 77315 - Cameo I/o
 *  - MMS 77317 - ACT/XCOMP I/O
 *
 */
class h89_mms_state : public h89_base_state
{
public:
	h89_mms_state(const machine_config &mconfig, device_type type, const char *tag):
		h89_base_state(mconfig, type, tag),
		m_mms316(*this, "mms77316")
	{
	}

	void h89_mms(machine_config &config);

protected:
	required_device<mms77316_fdc_device> m_mms316;

	void h89_mms_io(address_map &map) ATTR_COLD;
};


/*
  The H89 supported 16K, 32K, 48K, or 64K of RAM. The first 8K of address space
  is reserved for the monitor ROM, floppy ROM, and scratch pad RAM. For 16k-48K
  sizes, the upper 8k of memory is remapped to the first 8K when the ROM is disabled.
  For systems with 64K of RAM, the upper half of the expansion board is permanently
  mapped to the lower 8K. Even when ROM is mapped, any writes will still occur
  to the RAM.

  H89 Lower 8K address map

        HDOS Mode                       CP/M Mode
  ------------------- 0x2000 (8k) ----------------
  |   Floppy ROM   |                |            |
  ------------------- 0x1800 (6k)   |            |
  |   Floppy RAM   |                |            |
  ------------------- 0x1400 (5k)   |    RAM     |
  |      Open      |                |            |
  ------------------- 0x1000 (4k)   |            |
  |   MTR-90 ROM   |                |            |
  -................-- 0x0800 (2k)   |            |
  | MTR(88/89) ROM |                |            |
  ------------------- 0x0000 (0k) ----------------


        16K RAM Example

      HDOS                           CP/M
  ------------- 24k
  |    RAM    |  ------+
  ------------- 16k    |         ------------- 16k
  |    RAM    |  ------------->  |    RAM    |
  -------------  8k    |         -------------  8k
  |    ROM    |        +------>  |    RAM    |
  -------------  0k              -------------  0k

*/
void h89_base_state::h89_mem(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0xffff).view(m_mem_view);

	// View 0 - ROM / Floppy RAM R/O
	// View 1 - ROM / Floppy RAM R/W
	// monitor ROM
	m_mem_view[0](0x0000, 0x0fff).rom().region("maincpu", 0).unmapw();
	m_mem_view[1](0x0000, 0x0fff).rom().region("maincpu", 0).unmapw();

	// Floppy RAM
	m_mem_view[0](0x1400, 0x17ff).readonly().share(m_floppy_ram);
	m_mem_view[1](0x1400, 0x17ff).ram().share(m_floppy_ram);

	// Floppy ROM
	m_mem_view[0](0x1800, 0x1fff).rom().region("maincpu", 0x1800).unmapw();
	m_mem_view[1](0x1800, 0x1fff).rom().region("maincpu", 0x1800).unmapw();
}

void h89_base_state::map_fetch(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(h89_base_state::m1_r));
}

u8 h89_base_state::m1_r(offs_t offset)
{
	u8 data = m_program.read_byte(offset);

	if (!machine().side_effects_disabled() && m_single_step_enabled && !m_556b_latch)
	{
		LOGSS("single step m1_r - data: 0x%02x, 555a: %d, 555b: %d, 556b: %d\n", data, m_555a_latch, m_555b_latch, m_556b_latch);

		if (!m_555a_latch)
		{
			// Wait for EI instruction
			if (data == 0xfb)
			{
				m_555a_latch = true;
			}
		}
		else if (!m_555b_latch)
		{
			m_555b_latch = true;
		}
		else if (!m_556b_latch)
		{
			m_556b_latch = true;

			m_intr_socket->set_irq_level(2, ASSERT_LINE);
		}
	}

	return data;
}

/*
                                   PORT
    Use                      |  Hex  |  Octal
   --------------------------+-------+---------
    Not specified, available |  0-77 |   0-167
    Disk I/O #1              | 78-7B | 170-173
    Disk I/O #2              | 7C-7F | 174-177
    Not specified, reserved  | 80-CF | 200-317
    DCE Serial I/O           | D0-D7 | 320-327
    DTE Serial I/O           | D8-DF | 330-337
    DCE Serial I/O           | EO-E7 | 340-347
    Console I/O              | E8-EF | 350-357
    NMI                      | F0-F1 | 360-361
    General purpose port     |    F2 |     362
    Cassette I/O(MTR-88 only)| F8-F9 | 370-371
    NMI                      | FA-FB | 372-373

    Disk I/O #1 - 0170-0173 (0x78-0x7b)
       Heath Options
         - H37 5-1/4" Soft-sectored Controller - Requires MTR-90 ROM
         - H47 Dual 8" Drives - Requires MTR-89 or MTR-90 ROM
         - H67 8" Hard disk + 8" Floppy Drives - Requires MTR-90 ROM

    Disk I/O #2 - 0174-0177 (0x7c-0x7f)
       Heath Options
         - 5-1/4" Hard-sectored Controller - supported by all ROMs
         - H47 Dual 8" Drives - Requires MTR-89 or MTR-90 ROM
         - H67 8" Hard disk + 8" Floppy Drives - MTR-90 ROM

*/
void h89_base_state::h89_base_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	// 8250 UART DCE 0320 (0xd0)
	map(0xd0, 0xd7).rw(m_serial1, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	// 8250 UART DTE 0330 (0xd8) - typically used for modem
	map(0xd8, 0xdf).rw(m_serial2, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	// 8250 UART DCE 0340 (0xe0) - typically used for printer
	map(0xe0, 0xe7).rw(m_serial3, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));

	// 8250 UART console - this connects internally to the Terminal Logic board that is also used in the H19.
	map(0xe8, 0xef).rw(m_console, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));

	// ports defined on the H8. On the H89, access to these addresses causes a NMI
	map(0xf0, 0xf1).rw(FUNC(h89_base_state::raise_NMI_r),FUNC(h89_base_state::raise_NMI_w));

	// General Purpose Port (GPP)
	map(0xf2, 0xf2).w(FUNC(h89_base_state::port_f2_w)).portr("SW501");

	// port defined on the H8. On the H89, access to these addresses causes a NMI
	map(0xfa, 0xfb).rw(FUNC(h89_base_state::raise_NMI_r), FUNC(h89_base_state::raise_NMI_w));
}

void h88_state::h88_io(address_map &map)
{
	h89_base_state::h89_base_io(map);

	// Cassette I/O (uses 0xf8 - 0xf9) - Requires MTR-88 ROM
	map(0xf8, 0xf9).rw(m_cassette, FUNC(heath_h_88_cass_device::read), FUNC(heath_h_88_cass_device::write));
}

void h89_state::h89_io(address_map &map)
{
	h89_base_state::h89_base_io(map);

	// H37 5-1/4" Soft-sectored Controller - Requires MTR-90 ROM
	map(0x78, 0x7b).rw(m_h37, FUNC(heath_z37_fdc_device::read), FUNC(heath_z37_fdc_device::write));
}

void h89_sigmasoft_state::h89_sigmasoft_io(address_map &map)
{
	h89_io(map);

	// Add SigmaSoft parallel port board, required for IGC graphics
	map(0x08,0x0f).rw(m_sigma_parallel, FUNC(sigmasoft_parallel_port::read), FUNC(sigmasoft_parallel_port::write));
}

/*
    Memory Map for MMS 444-61C PROM

                                  PORT
    Use                        |  Hex  |
   ----------------------------+-------+
    Not specified, available   |  0-37 |
    MMS 77316                  | 38-3F |
    MMS Internal test fixtures | 40-47 |
    MMS 77317 ACT/XCOMP I/O    | 48-4F |
    MMS 77315 CAMEO I/O        | 50-56 |
    Unused                     |    57 |
    MMS 77314 Corvus I/O       | 58-59 |
    MMS 77314 REMEX I/O        | 5A-5B |
    MMS 77314,15,17 Conf Port  |    5C |
    Unused                     | 5D-77 |
    Disk I/O #1                | 78-7B |
    Disk I/O #2                | 7C-7F |
    HDOS reserved              | 80-CF |
    DCE Serial I/O             | D0-D7 |
    DTE Serial I/O             | D8-DF |
    DCE Serial I/O             | EO-E7 |
    Console I/O                | E8-EF |
    NMI                        | F0-F1 |
    General purpose port       |    F2 |
    Unused                     | F8-F9 |
    NMI                        | FA-FB |
    Unused                     | FC-FF |
 */
void h89_mms_state::h89_mms_io(address_map &map)
{
	h89_base_state::h89_base_io(map);

	// Add MMS 77316 Double Density Controller
	map(0x38,0x3f).rw(m_mms316, FUNC(mms77316_fdc_device::read), FUNC(mms77316_fdc_device::write));
}


// Input ports
static INPUT_PORTS_START( h88 )

	PORT_START("SW501")
	// MTR-88  (444-40)
	PORT_DIPUNUSED_DIPLOC(0x01, 0x00, "SW501:1")
	PORT_DIPUNUSED_DIPLOC(0x02, 0x00, "SW501:2")
	PORT_DIPUNUSED_DIPLOC(0x04, 0x00, "SW501:3")
	PORT_DIPUNUSED_DIPLOC(0x08, 0x00, "SW501:4")
	PORT_DIPUNUSED_DIPLOC(0x10, 0x00, "SW501:5")
	PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )       PORT_DIPLOCATION("SW501:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xc0, 0x00, "Console Baud rate" )                  PORT_DIPLOCATION("SW501:7,8")
	PORT_DIPSETTING(    0x00, "9600" )
	PORT_DIPSETTING(    0x40, "19200" )
	PORT_DIPSETTING(    0x80, "38400" )
	PORT_DIPSETTING(    0xc0, "57600" )

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x03, 0x00, "CPU Clock Speed Upgrade")
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x01, "2 / 4 MHz")
	PORT_CONFSETTING(    0x02, "2 / 6 MHz")

INPUT_PORTS_END


static INPUT_PORTS_START( h89 )

	PORT_START("SW501")
	// Generic definition
	PORT_DIPNAME( 0x01, 0x00, "Switch 0" )                           PORT_DIPLOCATION("SW501:1")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Switch 1" )                           PORT_DIPLOCATION("SW501:2")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Switch 2" )                           PORT_DIPLOCATION("SW501:3")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Switch 3" )                           PORT_DIPLOCATION("SW501:4")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Switch 4" )                           PORT_DIPLOCATION("SW501:5")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Switch 5" )                           PORT_DIPLOCATION("SW501:6")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Switch 6" )                           PORT_DIPLOCATION("SW501:7")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Switch 7" )                           PORT_DIPLOCATION("SW501:8")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	// MTR-90 (444-84 or 444-142)
	PORT_DIPNAME( 0x03, 0x00, "Disk I/O #2" )                        PORT_DIPLOCATION("SW501:1,2")     PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, "H-88-1 (Not yet implemented)" )
	PORT_DIPSETTING(    0x01, "H/Z-47 (Not yet implemented)" )
	PORT_DIPSETTING(    0x02, "Z-67 (Not yet implemented)" )
	PORT_DIPSETTING(    0x03, "Undefined" )
	PORT_DIPNAME( 0x0c, 0x00, "Disk I/O #1" )                        PORT_DIPLOCATION("SW501:3,4")     PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, "H-89-37" )
	PORT_DIPSETTING(    0x04, "H/Z-47 (Not yet implemented)" )
	PORT_DIPSETTING(    0x08, "Z-67 (Not yet implemented)" )
	PORT_DIPSETTING(    0x0c, "Undefined" )
	PORT_DIPNAME( 0x10, 0x00, "Primary Boot from" )                  PORT_DIPLOCATION("SW501:5")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, "Disk I/O #2" )
	PORT_DIPSETTING(    0x10, "Disk I/O #1" )
	PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )       PORT_DIPLOCATION("SW501:6")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Console Baud rate" )                  PORT_DIPLOCATION("SW501:7")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, "9600" )
	PORT_DIPSETTING(    0x40, "19200" )
	PORT_DIPNAME( 0x80, 0x00, "Boot mode" )                          PORT_DIPLOCATION("SW501:8")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x04)
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, "Auto" )

	// MTR-89 (444-62)
	PORT_DIPNAME( 0x03, 0x00, "Disk I/O #2" )                        PORT_DIPLOCATION("SW501:1,2")     PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, "H-88-1" )
	PORT_DIPSETTING(    0x01, "H/Z-47" )
	PORT_DIPSETTING(    0x02, "Undefined" )
	PORT_DIPSETTING(    0x03, "Undefined" )
	PORT_DIPNAME( 0x0c, 0x00, "Disk I/O #1" )                        PORT_DIPLOCATION("SW501:3,4")     PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, "Unused" )
	PORT_DIPSETTING(    0x04, "H/Z-47" )
	PORT_DIPSETTING(    0x08, "Undefined" )
	PORT_DIPSETTING(    0x0c, "Undefined" )
	PORT_DIPNAME( 0x10, 0x00, "Primary Boot from" )                  PORT_DIPLOCATION("SW501:5")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, "Disk I/O #2" )
	PORT_DIPSETTING(    0x10, "Disk I/O #1" )
	PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )       PORT_DIPLOCATION("SW501:6")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x08)
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Console Baud rate" )                  PORT_DIPLOCATION("SW501:7")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, "9600" )
	PORT_DIPSETTING(    0x40, "19200" )
	PORT_DIPNAME( 0x80, 0x00, "Boot mode" )                          PORT_DIPLOCATION("SW501:8")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, "Auto" )

	// MMS 444-84B (and possibly 444-84A)
	PORT_DIPNAME( 0x03, 0x00, "Disk I/O #2" )                        PORT_DIPLOCATION("SW501:1,2")     PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x0c)
	PORT_DIPSETTING(    0x00, "H-88-1" )
	PORT_DIPSETTING(    0x01, "H/Z-47 (Not yet implemented)" )
	PORT_DIPSETTING(    0x02, "MMS 77320 SASI or Z-67 (Not yet implemented)" )
	PORT_DIPSETTING(    0x03, "MMS 77422 Network Controller" )
	PORT_DIPNAME( 0x0c, 0x00, "Disk I/O #1" )                        PORT_DIPLOCATION("SW501:3,4")     PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x0c)
	PORT_DIPSETTING(    0x00, "H-89-37" )
	PORT_DIPSETTING(    0x04, "H/Z-47 (Not yet implemented)" )
	PORT_DIPSETTING(    0x08, "MMS 77320 SASI or Z-67 (Not yet implemented)" )
	PORT_DIPSETTING(    0x0c, "MMS 77422 Network Controller" )
	PORT_DIPNAME( 0x70, 0x00, "Default Boot Device" )                PORT_DIPLOCATION("SW501:5,6,7")   PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x0c)
	PORT_DIPSETTING(    0x00, "MMS 77316 Dbl Den 5\"" )
	PORT_DIPSETTING(    0x10, "MMS 77316 Dbl Den 8\"" )
	PORT_DIPSETTING(    0x20, "Disk Device at 0x7C" )
	PORT_DIPSETTING(    0x30, "Disk Device at 0x78" )
	PORT_DIPSETTING(    0x40, "reserved for future use" )
	PORT_DIPSETTING(    0x50, "reserved for future use" )
	PORT_DIPSETTING(    0x60, "MMS Network (77422)" )
	PORT_DIPSETTING(    0x70, "Use MMS I/O board Config Port" )
	PORT_DIPNAME( 0x80, 0x00, "Boot mode" )                          PORT_DIPLOCATION("SW501:8")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x0c)
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, "Auto" )

	// Kres KMR-100
	PORT_DIPNAME( 0x0f, 0x00, "Default Boot Device" )                PORT_DIPLOCATION("SW501:1,2,3,4") PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x10)
	PORT_DIPSETTING(    0x00, "H-17 hard-sectored 5\" floppy units 0-2" )
	PORT_DIPSETTING(    0x01, "H-37 soft-sectored 5\" floppy units 0-3" )
	PORT_DIPSETTING(    0x02, "Corvus hard disk/Magnolia interface, partitions 0-8" )
	PORT_DIPSETTING(    0x03, "CDR 5\"/8\" double density floppy, units 0-3" )
	PORT_DIPSETTING(    0x04, "H-47 8\" floppy at port 0x78/0170, units 0-3" )
	PORT_DIPSETTING(    0x05, "H-47 8\" floppy at port 0x7c/0174, units 0-3" )
	PORT_DIPSETTING(    0x06, "Reserved" )
	PORT_DIPSETTING(    0x07, "Help - lists boot devices" )
	PORT_DIPSETTING(    0x08, "Reserved" )
	PORT_DIPSETTING(    0x09, "SASI controller or Z-67 at port 0x78/0170, units 0-7" )
	PORT_DIPSETTING(    0x0a, "SASI controller or Z-67 at port 0x7c/0174, units 0-7" )
	PORT_DIPSETTING(    0x0b, "Livingston 8\" single density floppy, units 0-3" )
	PORT_DIPSETTING(    0x0c, "Magnolia 5\"/8\" double density floppy, units 0-3 (8\"), 4-7 (5\")" )
	PORT_DIPSETTING(    0x0d, "Reserved" )
	PORT_DIPSETTING(    0x0e, "Reserved" )
	PORT_DIPSETTING(    0x0f, "Magnolia 128K pseudo disk, banks 0-1" )
	PORT_DIPNAME( 0x10, 0x00, "Map ROM into RAM on boot" )           PORT_DIPLOCATION("SW501:5")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )       PORT_DIPLOCATION("SW501:6")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x10)
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Have a LLL controller installed" )    PORT_DIPLOCATION("SW501:7")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Boot mode" )                          PORT_DIPLOCATION("SW501:8")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, "Auto" )

	// Ultimeth MTRHEX-4k
	// (values based on testing and comparison with the Kres KMR-100 ROM which was also written by Ultimeth)
	PORT_DIPNAME( 0x0f, 0x00, "Default Boot Device" )                PORT_DIPLOCATION("SW501:1,2,3,4") PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x14)
	PORT_DIPSETTING(    0x00, "H-17 hard-sectored 5\" floppy" )
	PORT_DIPSETTING(    0x01, "H-37 soft-sectored 5\" floppy" )
	PORT_DIPSETTING(    0x02, "? Corvus hard disk/Magnolia interface" )
	PORT_DIPSETTING(    0x03, "? CDR 5\"/8\" double density floppy" )
	PORT_DIPSETTING(    0x04, "? H-47 8\" floppy at port 0x78/0170" )
	PORT_DIPSETTING(    0x05, "? H-47 8\" floppy at port 0x7c/0174" )
	PORT_DIPSETTING(    0x06, "? Reserved" )
	PORT_DIPSETTING(    0x07, "? Reserved" )
	PORT_DIPSETTING(    0x08, "? Reserved" )
	PORT_DIPSETTING(    0x09, "? SASI controller or Z-67 at port 0x78/0170" )
	PORT_DIPSETTING(    0x0a, "? SASI controller or Z-67 at port 0x7c/0174" )
	PORT_DIPSETTING(    0x0b, "? Livingston 8\" single density floppy" )
	PORT_DIPSETTING(    0x0c, "? Magnolia 5\"/8\" double density floppy" )
	PORT_DIPSETTING(    0x0d, "? Reserved" )
	PORT_DIPSETTING(    0x0e, "? Reserved" )
	PORT_DIPSETTING(    0x0f, "? Magnolia 128K pseudo disk" )
	PORT_DIPNAME( 0x10, 0x00, "? Map ROM into RAM on boot" )         PORT_DIPLOCATION("SW501:5")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x14)
	PORT_DIPSETTING(    0x00, "? Yes" )
	PORT_DIPSETTING(    0x10, "? No" )
	PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )       PORT_DIPLOCATION("SW501:6")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x14)
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "? Have a LLL controller installed" )  PORT_DIPLOCATION("SW501:7")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x14)
	PORT_DIPSETTING(    0x00, "? No" )
	PORT_DIPSETTING(    0x40, "? Yes" )
	PORT_DIPNAME( 0x80, 0x00, "Boot mode" )                          PORT_DIPLOCATION("SW501:8")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x14)
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, "Auto" )

	// Ultimeth MTRHEX-2k
	PORT_DIPNAME( 0x03, 0x00, "Default Boot Device" )                PORT_DIPLOCATION("SW501:1,2")     PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x18)
	PORT_DIPSETTING(    0x00, "H-17 hard-sectored 5\" floppy" )
	PORT_DIPSETTING(    0x01, "H-47 8\" floppy at port 0x78/0170" )
	PORT_DIPSETTING(    0x02, "H-47 8\" floppy at port 0x7c/0174" )
	PORT_DIPSETTING(    0x03, "Magnolia 5\"/8\" double density floppy" )
	PORT_DIPUNUSED_DIPLOC(0x04, 0x00, "SW501:3")                                                       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x18)
	PORT_DIPUNUSED_DIPLOC(0x08, 0x00, "SW501:4")                                                       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x18)
	PORT_DIPUNUSED_DIPLOC(0x10, 0x00, "SW501:5")                                                       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x18)
	PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )       PORT_DIPLOCATION("SW501:6")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x18)
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Boot mode" )                          PORT_DIPLOCATION("SW501:7")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x18)
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, "Auto" )
	PORT_DIPNAME( 0x80, 0x00, "Baud Rate" )                          PORT_DIPLOCATION("SW501:8")       PORT_CONDITION("CONFIG", 0x3c, EQUALS, 0x18)
	PORT_DIPSETTING(    0x00, "9600" )
	PORT_DIPSETTING(    0x80, "19200" )


	PORT_START("CONFIG")
	PORT_CONFNAME(0x03, 0x00, "CPU Clock Speed Upgrade")
	PORT_CONFSETTING(   0x00, DEF_STR( None ) )
	PORT_CONFSETTING(   0x01, "2 / 4 MHz")
	PORT_CONFSETTING(   0x02, "2 / 6 MHz")
	PORT_CONFNAME(0x3c, 0x04, "Switch SW501 Definitions")
	PORT_CONFSETTING(   0x00, "Generic" )
	PORT_CONFSETTING(   0x04, "Heath MTR-90")
	PORT_CONFSETTING(   0x08, "Heath MTR-89")
	PORT_CONFSETTING(   0x0c, "MMS 444-84B/444-84A")
	PORT_CONFSETTING(   0x10, "Kres KMR-100")
	PORT_CONFSETTING(   0x14, "Ultimeth MTRHEX-4k")
	PORT_CONFSETTING(   0x18, "Ultimeth MTRHEX-2k")

INPUT_PORTS_END


void h89_base_state::machine_start()
{
	save_item(NAME(m_gpp));
	save_item(NAME(m_rom_enabled));
	save_item(NAME(m_timer_intr_enabled));
	save_item(NAME(m_single_step_enabled));
	save_item(NAME(m_floppy_ram_wp));
	save_item(NAME(m_cpu_speed_multiplier));
	save_item(NAME(m_555a_latch));
	save_item(NAME(m_555b_latch));
	save_item(NAME(m_556b_latch));

	m_maincpu->space(AS_PROGRAM).specific(m_program);

	// update RAM mappings based on RAM size
	u8 *ram_ptr  = m_ram->pointer();
	u32 ram_size = m_ram->size();

	if (ram_size == 0x10000)
	{
		// system has a full 64k
		m_maincpu->space(AS_PROGRAM).install_ram(0x2000, 0xffff, ram_ptr);

		// install shadow writing to RAM when in ROM mode and Floppy RAM is write-protected.
		m_mem_view[0].install_writeonly(0x0000, 0x1fff, ram_ptr + 0xe000);
		// when Floppy RAM is in write enable mode, must use write_tap so writes occur to both RAMs
		// NOTE: the H89 had space reserved for additional RAM (without write protection) in the ROM space, but not aware
		// it was ever used. If that was added to this emulation, m_mem_view[0] would also need to be a write_tap.
		m_mem_view[1].install_write_tap(0x0000, 0x1fff, "shadow_w", [ram_ptr](offs_t offset, u8 &data, u8 mem_mask)
		{
			ram_ptr[0xe000 + offset] = data;
		});

		// The Org-0 (often used for CP/M) view has RAM at the lower 8k
		m_mem_view[2].install_ram(0x0000, 0x1fff, ram_ptr + 0xe000);
	}
	else
	{
		// less than 64k

		// for views with ROM visible, the top of memory is 8k higher than
		// the memory size, since the base starts at 8k.
		u32 ram_top = ram_size + 0x1fff;

		m_mem_view[0].install_ram(0x2000, ram_top, ram_ptr);
		m_mem_view[1].install_ram(0x2000, ram_top, ram_ptr);

		// when ROM is not active, memory still starts at 8k, but is 8k smaller so the last 8k can be mapped to addr 0.
		m_mem_view[2].install_ram(0x2000, ram_size - 1, ram_ptr);

		// remap the top 8k down to addr 0
		m_mem_view[2].install_ram(0x0000, 0x1fff, ram_ptr + ram_size - 0x2000);
	}
}

void h89_base_state::machine_reset()
{
	m_rom_enabled         = true;
	m_timer_intr_enabled  = true;
	m_single_step_enabled = false;
	m_floppy_ram_wp       = false;
	reset_single_step_state();

	ioport_value const cfg(m_config->read());

	// CPU clock speed
	const u8 selected_clock_upgrade = cfg & 0x3;

	switch (selected_clock_upgrade)
	{
	case 0x01:
		// 4 MHz was offered by several companies including Kres, ANAPRO, and an article
		// in REMark magazine.
		m_cpu_speed_multiplier = 2;
		break;
	case 0x02:
		// 6 MHz was offered by at least ANAPRO, and a how to article in CHUG newsletter
		m_cpu_speed_multiplier = 3;
		break;
	case 0x00:
	default:
		// No speed upgrade installed - Standard Clock
		m_cpu_speed_multiplier = 1;
		break;
	}

	update_gpp(0);
	update_mem_view();
}

void h89_base_state::set_wait_state(int data)
{
	m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, data);
	if (data)
	{
		machine().scheduler().synchronize();
		m_maincpu->defer_access();
	}
}

u8 h89_base_state::raise_NMI_r()
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::from_usec(2));

	return 0x00;
}

void h89_base_state::raise_NMI_w(u8)
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::from_usec(2));
}

void h89_base_state::console_intr(int data)
{
	m_intr_socket->set_irq_level(3, data);
}

void h89_base_state::reset_line(int data)
{
	if (bool(data))
	{
		reset();
	}
	m_maincpu->set_input_line(INPUT_LINE_RESET, data);
}

TIMER_DEVICE_CALLBACK_MEMBER(h89_base_state::h89_irq_timer)
{
	if (m_timer_intr_enabled)
	{
		m_intr_socket->set_irq_level(1, ASSERT_LINE);
	}
}

void h89_base_state::update_mem_view()
{
	m_mem_view.select(m_rom_enabled ? (m_floppy_ram_wp ? 0 : 1) : 2);
}

void h89_base_state::reset_single_step_state()
{
	LOGSS("reset_single_step_state\n");
	m_555a_latch = false;
	m_555b_latch = false;
	m_556b_latch = false;
	m_intr_socket->set_irq_level(2, CLEAR_LINE);
}

// General Purpose Port
//
// Bit     OUTPUT
// ---------------------
//  0    Single-step enable
//  1    2 mSec interrupt enable
//  2    Latched bit MEM 1 H on memory exp connector
//  3    Not used
//  4    Latched bit MEM 0 H on memory expansion connector (Commonly used for Speed upgrades)
//  5    ORG-0 (CP/M map)
//  6    Latched bit I/O 0 on I/O exp connector
//  7    Latched bit I/O 1 on I/O exp connector
//
void h89_base_state::update_gpp(u8 gpp)
{
	u8 changed_gpp = gpp ^ m_gpp;

	m_gpp = gpp;

	m_timer_intr_enabled = bool(BIT(m_gpp, GPP_ENABLE_TIMER_INTERRUPT_BIT));

	if (BIT(changed_gpp, GPP_SINGLE_STEP_BIT))
	{
		LOGSS("single step enable: %d\n", BIT(m_gpp, GPP_SINGLE_STEP_BIT));
		m_single_step_enabled = bool(BIT(m_gpp, GPP_SINGLE_STEP_BIT));

		if (!m_single_step_enabled)
		{
			reset_single_step_state();
		}
	}

	if (BIT(changed_gpp, GPP_DISABLE_ROM_BIT))
	{
		m_rom_enabled = BIT(m_gpp, GPP_DISABLE_ROM_BIT) == 0;

		update_mem_view();
	}

	if (BIT(changed_gpp, GPP_SPEED_SELECT_BIT))
	{
		m_maincpu->set_clock(BIT(m_gpp, GPP_SPEED_SELECT_BIT) ?
			H89_CLOCK * m_cpu_speed_multiplier : H89_CLOCK);
	}
}

// General Purpose Port
void h89_base_state::port_f2_w(u8 data)
{
	update_gpp(data);

	m_intr_socket->set_irq_level(1, CLEAR_LINE);
}

static void tlb_options(device_slot_interface &device)
{
	device.option_add("heath",      HEATH_TLB);
	device.option_add("gp19",       HEATH_GP19);
	device.option_add("imaginator", HEATH_IMAGINATOR);
	device.option_add("super19",    HEATH_SUPER19);
	device.option_add("superset",   HEATH_SUPERSET);
	device.option_add("ultrarom",   HEATH_ULTRA);
	device.option_add("watzman",    HEATH_WATZ);
}


static void sigma_tlb_options(device_slot_interface *device)
{
	device->option_reset();
	device->option_add("igc",          HEATH_IGC);
	device->option_add("igc_super19",  HEATH_IGC_SUPER19);
	device->option_add("igc_ultrarom", HEATH_IGC_ULTRA);
	device->option_add("igc_watzman",  HEATH_IGC_WATZ);
	device->set_default_option("igc");
}

static void intr_ctrl_options(device_slot_interface &device)
{
	device.option_add("original", HEATH_INTR_CNTRL);
	device.option_add("h37",      HEATH_Z37_INTR_CNTRL);
	device.option_add("mms",      HEATH_MMS_INTR_CNTRL);
}

void h89_base_state::h89_base(machine_config &config)
{
	config.set_default_layout(layout_h89);

	// basic machine hardware
	Z80(config, m_maincpu, H89_CLOCK);
	m_maincpu->set_m1_map(&h89_base_state::map_fetch);
	m_maincpu->set_memory_map(&h89_base_state::h89_mem);
	m_maincpu->set_irq_acknowledge_callback("intr_socket", FUNC(heath_intr_socket::irq_callback));

	RAM(config, m_ram).set_default_size("64K").set_extra_options("16K,32K,48K").set_default_value(0x00);

	HEATH_INTR_SOCKET(config, m_intr_socket, intr_ctrl_options, nullptr);
	m_intr_socket->irq_line_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	INS8250(config, m_console, INS8250_CLOCK);
	m_console->out_int_callback().set(FUNC(h89_base_state::console_intr));

	HEATH_TLB_CONNECTOR(config, m_tlbc, tlb_options, "heath");

	// Connect the console port on CPU board to TLB connector
	m_console->out_tx_callback().set(m_tlbc, FUNC(heath_tlb_connector::serial_in_w));
	m_console->out_rts_callback().set(m_tlbc, FUNC(heath_tlb_connector::cts_in_w));
	m_console->out_dtr_callback().set(m_tlbc, FUNC(heath_tlb_connector::dsr_in_w));
	m_tlbc->serial_data_callback().set(m_console, FUNC(ins8250_uart_device::rx_w));
	m_tlbc->rts_callback().set(m_console, FUNC(ins8250_uart_device::cts_w));
	m_tlbc->dtr_callback().set(m_console, FUNC(ins8250_uart_device::dsr_w));

	m_tlbc->reset_cb().set(FUNC(h89_base_state::reset_line));

	// H-88-3 3-port serial board
	INS8250(config, m_serial1, INS8250_CLOCK);
	INS8250(config, m_serial2, INS8250_CLOCK);
	INS8250(config, m_serial3, INS8250_CLOCK);

	// H89 interrupt interval is 2mSec
	TIMER(config, "irq_timer", 0).configure_periodic(FUNC(h89_base_state::h89_irq_timer), attotime::from_msec(2));
}

void h88_state::h88(machine_config &config)
{
	h89_base(config);

	m_maincpu->set_io_map(&h88_state::h88_io);

	m_intr_socket->set_default_option("original");
	m_intr_socket->set_fixed(true);

	SOFTWARE_LIST(config, "cass_list").set_original("h88_cass");

	// H-88-5 Cassette interface board
	HEATH_H88_CASS(config, m_cassette, H89_CLOCK);
}

void h89_state::h89(machine_config &config)
{
	h89_base(config);

	m_maincpu->set_io_map(&h89_state::h89_io);

	m_intr_socket->set_default_option("h37");
	m_intr_socket->set_fixed(true);

	// Z-89-37 Soft-sectored controller
	HEATH_Z37_FDC(config, m_h37);
	m_h37->drq_cb().set(m_intr_socket, FUNC(heath_intr_socket::set_drq));
	m_h37->irq_cb().set(m_intr_socket, FUNC(heath_intr_socket::set_irq));
	m_h37->block_interrupt_cb().set(m_intr_socket, FUNC(heath_intr_socket::block_interrupts));
}

void h89_sigmasoft_state::h89_sigmasoft(machine_config &config)
{
	h89(config);
	m_maincpu->set_addrmap(AS_IO, &h89_sigmasoft_state::h89_sigmasoft_io);

	sigma_tlb_options(m_tlbc);

	SIGMASOFT_PARALLEL_PORT(config, m_sigma_parallel);
	m_sigma_parallel->ctrl_r_cb().set(m_tlbc, FUNC(heath_tlb_connector::sigma_ctrl_r));
	m_sigma_parallel->video_mem_r_cb().set(m_tlbc, FUNC(heath_tlb_connector::sigma_video_mem_r));
	m_sigma_parallel->video_mem_cb().set(m_tlbc, FUNC(heath_tlb_connector::sigma_video_mem_w));
	m_sigma_parallel->io_lo_cb().set(m_tlbc, FUNC(heath_tlb_connector::sigma_io_lo_addr_w));
	m_sigma_parallel->io_hi_cb().set(m_tlbc, FUNC(heath_tlb_connector::sigma_io_hi_addr_w));
	m_sigma_parallel->window_lo_cb().set(m_tlbc, FUNC(heath_tlb_connector::sigma_window_lo_addr_w));
	m_sigma_parallel->window_hi_cb().set(m_tlbc, FUNC(heath_tlb_connector::sigma_window_hi_addr_w));
	m_sigma_parallel->ctrl_cb().set(m_tlbc, FUNC(heath_tlb_connector::sigma_ctrl_w));
}

void h89_mms_state::h89_mms(machine_config &config)
{
	h89_base(config);
	m_maincpu->set_addrmap(AS_IO, &h89_mms_state::h89_mms_io);

	m_intr_socket->set_default_option("mms");
	m_intr_socket->set_fixed(true);

	MMS77316_FDC(config, m_mms316);
	m_mms316->drq_cb().set(m_intr_socket, FUNC(heath_intr_socket::set_drq));
	m_mms316->irq_cb().set(m_intr_socket, FUNC(heath_intr_socket::set_irq));
	m_mms316->wait_cb().set(FUNC(h89_mms_state::set_wait_state));
}


ROM_START( h88 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )

	ROM_LOAD( "2716_444-19_h17.u520",     0x1800, 0x0800, CRC(26e80ae3) SHA1(0c0ee95d7cb1a760f924769e10c0db1678f2435c))

	ROM_LOAD("2716_444-40_mtr88.u518",    0x0000, 0x0800, CRC(093afb79) SHA1(bcc1569ad9da7babf0a4199cab96d8cd59b2dd78))
ROM_END

ROM_START( h89 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("mtr90")

	ROM_LOAD( "2716_444-19_h17.u520",     0x1800, 0x0800, CRC(26e80ae3) SHA1(0c0ee95d7cb1a760f924769e10c0db1678f2435c))

	ROM_SYSTEM_BIOS(0, "mtr90", "Zenith Data Systems MTR-90 (444-142)")
	ROMX_LOAD("2732_444-142_mtr90.u518",  0x0000, 0x1000, CRC(c4ff47c5) SHA1(d6f3d71ff270a663003ec18a3ed1fa49f627123a), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "mtr89", "Heath MTR-89 (444-62)")
	ROMX_LOAD("2716_444-62_mtr89.u518",   0x0000, 0x0800, CRC(8f507972) SHA1(ac6c6c1344ee4e09fb60d53c85c9b761217fe9dc), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "mms84b", "Magnolia MicroSystems 444-84B")
	ROMX_LOAD("2732_444_84b_mms.u518",    0x0000, 0x1000, CRC(7e75d6f4) SHA1(baf34e036388d1a191197e31f8a93209f04fc58b), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "kmr-100", "Kres KMR-100 V3.a.02")
	ROMX_LOAD("2732_kmr100_v3_a_02.u518", 0x0000, 0x1000, CRC(fd491592) SHA1(3d5803f95c38b237b07cd230353cd9ddc9858c13), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "mtrhex_4k", "Ultimeth 4k ROM")
	ROMX_LOAD("2732_mtrhex_4k.u518",      0x0000, 0x1000, CRC(e26b29a9) SHA1(ba13d6c9deef682a9a8262bc910d46b577929a13), ROM_BIOS(4))

	ROM_SYSTEM_BIOS(5, "mtr90-84", "Zenith Data Systems MTR-90 (444-84 - Superseded by 444-142)")
	ROMX_LOAD("2732_444-84_mtr90.u518",   0x0000, 0x1000, CRC(f10fca03) SHA1(c4a978153af0f2dfcc9ba05be4c1033d33fee30b), ROM_BIOS(5))

	ROM_SYSTEM_BIOS(6, "mms84a", "Magnolia MicroSystems 444-84A (Superseded by MMS 444-84B)")
	ROMX_LOAD("2732_444_84a_mms.u518",    0x0000, 0x1000, CRC(0e541a7e) SHA1(b1deb620fc89c1068e2e663e14be69d1f337a4b9), ROM_BIOS(6))

	ROM_SYSTEM_BIOS(7, "mtrhex", "Ultimeth 2k ROM")
	ROMX_LOAD("2716_mtrhex.u518",         0x0000, 0x0800, CRC(842a306a) SHA1(ddbc2b8bb127464af9eda8e7c56e6be7c8b43a16), ROM_BIOS(7))
ROM_END

ROM_START( h89_sigmasoft )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("mtr90")

	ROM_LOAD( "2716_444-19_h17.u520",     0x1800, 0x0800, CRC(26e80ae3) SHA1(0c0ee95d7cb1a760f924769e10c0db1678f2435c))

	ROM_SYSTEM_BIOS(0, "mtr90", "Zenith Data Systems MTR-90 (444-142)")
	ROMX_LOAD("2732_444-142_mtr90.u518",  0x0000, 0x1000, CRC(c4ff47c5) SHA1(d6f3d71ff270a663003ec18a3ed1fa49f627123a), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "mtr89", "Heath MTR-89 (444-62)")
	ROMX_LOAD("2716_444-62_mtr89.u518",   0x0000, 0x0800, CRC(8f507972) SHA1(ac6c6c1344ee4e09fb60d53c85c9b761217fe9dc), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "mms84b", "Magnolia MicroSystems 444-84B")
	ROMX_LOAD("2732_444_84b_mms.u518",    0x0000, 0x1000, CRC(7e75d6f4) SHA1(baf34e036388d1a191197e31f8a93209f04fc58b), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "kmr-100", "Kres KMR-100 V3.a.02")
	ROMX_LOAD("2732_kmr100_v3_a_02.u518", 0x0000, 0x1000, CRC(fd491592) SHA1(3d5803f95c38b237b07cd230353cd9ddc9858c13), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "mtrhex_4k", "Ultimeth 4k ROM")
	ROMX_LOAD("2732_mtrhex_4k.u518",      0x0000, 0x1000, CRC(e26b29a9) SHA1(ba13d6c9deef682a9a8262bc910d46b577929a13), ROM_BIOS(4))

	ROM_SYSTEM_BIOS(5, "mtr90-84", "Zenith Data Systems MTR-90 (444-84 - Superseded by 444-142)")
	ROMX_LOAD("2732_444-84_mtr90.u518",   0x0000, 0x1000, CRC(f10fca03) SHA1(c4a978153af0f2dfcc9ba05be4c1033d33fee30b), ROM_BIOS(5))

	ROM_SYSTEM_BIOS(6, "mms84a", "Magnolia MicroSystems 444-84A (Superseded by MMS 444-84B)")
	ROMX_LOAD("2732_444_84a_mms.u518",    0x0000, 0x1000, CRC(0e541a7e) SHA1(b1deb620fc89c1068e2e663e14be69d1f337a4b9), ROM_BIOS(6))

	ROM_SYSTEM_BIOS(7, "mtrhex", "Ultimeth 2k ROM")
	ROMX_LOAD("2716_mtrhex.u518",         0x0000, 0x0800, CRC(842a306a) SHA1(ddbc2b8bb127464af9eda8e7c56e6be7c8b43a16), ROM_BIOS(7))
ROM_END

ROM_START( h89_mms )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("mms84b")

	ROM_LOAD( "2716_444-19_h17.u520",     0x1800, 0x0800, CRC(26e80ae3) SHA1(0c0ee95d7cb1a760f924769e10c0db1678f2435c))

	ROM_SYSTEM_BIOS(0, "mms84b", "MMS 444-84B")
	ROMX_LOAD("2732_444_84b_mms.u518",    0x0000, 0x1000, CRC(7e75d6f4) SHA1(baf34e036388d1a191197e31f8a93209f04fc58b), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "kmr-100", "Kres KMR-100 V3.a.02")
	ROMX_LOAD("2732_kmr100_v3_a_02.u518", 0x0000, 0x1000, CRC(fd491592) SHA1(3d5803f95c38b237b07cd230353cd9ddc9858c13), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "mtrhex_4k", "Ultimeth 4k ROM")
	ROMX_LOAD("2732_mtrhex_4k.u518",      0x0000, 0x1000, CRC(e26b29a9) SHA1(ba13d6c9deef682a9a8262bc910d46b577929a13), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "mms84a", "MMS 444-84A (Superseded by MMS 444-84B)")
	ROMX_LOAD("2732_444_84a_mms.u518",    0x0000, 0x1000, CRC(0e541a7e) SHA1(b1deb620fc89c1068e2e663e14be69d1f337a4b9), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "mtrhex", "Ultimeth 2k ROM")
	ROMX_LOAD("2716_mtrhex.u518",         0x0000, 0x0800, CRC(842a306a) SHA1(ddbc2b8bb127464af9eda8e7c56e6be7c8b43a16), ROM_BIOS(4))
ROM_END

ROM_START( z90 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("mtr90")

	ROM_LOAD( "2716_444-19_h17.u520",     0x1800, 0x0800, CRC(26e80ae3) SHA1(0c0ee95d7cb1a760f924769e10c0db1678f2435c))

	ROM_SYSTEM_BIOS(0, "mtr90", "Zenith Data Systems MTR-90 (444-142)")
	ROMX_LOAD("2732_444-142_mtr90.u518",  0x0000, 0x1000, CRC(c4ff47c5) SHA1(d6f3d71ff270a663003ec18a3ed1fa49f627123a), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "mms84b", "Magnolia MicroSystems 444-84B")
	ROMX_LOAD("2732_444_84b_mms.u518",    0x0000, 0x1000, CRC(7e75d6f4) SHA1(baf34e036388d1a191197e31f8a93209f04fc58b), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "kmr-100", "Kres KMR-100 V3.a.02")
	ROMX_LOAD("2732_kmr100_v3_a_02.u518", 0x0000, 0x1000, CRC(fd491592) SHA1(3d5803f95c38b237b07cd230353cd9ddc9858c13), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "mtrhex_4k", "Ultimeth 4k ROM")
	ROMX_LOAD("2732_mtrhex_4k.u518",      0x0000, 0x1000, CRC(e26b29a9) SHA1(ba13d6c9deef682a9a8262bc910d46b577929a13), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "mtr90-84", "Zenith Data Systems MTR-90 (444-84 - Superseded by 444-142)")
	ROMX_LOAD("2732_444-84_mtr90.u518",   0x0000, 0x1000, CRC(f10fca03) SHA1(c4a978153af0f2dfcc9ba05be4c1033d33fee30b), ROM_BIOS(4))

	ROM_SYSTEM_BIOS(5, "mms84a", "Magnolia MicroSystems 444-84A (Superseded by MMS 444-84B)")
	ROMX_LOAD("2732_444_84a_mms.u518",    0x0000, 0x1000, CRC(0e541a7e) SHA1(b1deb620fc89c1068e2e663e14be69d1f337a4b9), ROM_BIOS(5))
ROM_END

} // anonymous namespace


//    year  name           parent compat machine        input class                init        company                fullname                   flags
COMP( 1979, h88,           h89,   0,     h88,           h88,  h88_state,           empty_init, "Heath Company",       "H-88",                    MACHINE_SUPPORTS_SAVE)
COMP( 1979, h89,           0,     0,     h89,           h89,  h89_state,           empty_init, "Heath Company",       "H-89",                    MACHINE_SUPPORTS_SAVE)
COMP( 1981, h89_mms,       h89,   0,     h89_mms,       h89,  h89_mms_state,       empty_init, "Heath Company",       "H-89 with MMS Equipment", MACHINE_SUPPORTS_SAVE)
COMP( 1981, z90,           h89,   0,     h89,           h89,  h89_state,           empty_init, "Zenith Data Systems", "Z-90",                    MACHINE_SUPPORTS_SAVE)
COMP( 1984, h89_sigmasoft, h89,   0,     h89_sigmasoft, h89,  h89_sigmasoft_state, empty_init, "Heath Company",       "H-89 with SigmaSoft IGC", MACHINE_SUPPORTS_SAVE)
