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

#include "tlb.h"
#include "z37_fdc.h"
#include "intr_cntrl.h"

#include "cpu/z80/z80.h"
#include "machine/ins8250.h"
#include "machine/ram.h"
#include "machine/timer.h"

namespace {


class h89_state : public driver_device
{
public:
	h89_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_maincpu_region(*this, "maincpu")
		, m_mem_view(*this, "rom_bank")
		, m_ram(*this, RAM_TAG)
		, m_floppy_ram(*this, "floppyram")
		, m_tlb(*this, "tlb")
		, m_h37(*this, "h37")
		, m_intr_cntrl(*this, "intr_cntrl")
		, m_console(*this, "console")
		, m_serial1(*this, "serial1")
		, m_serial2(*this, "serial2")
		, m_serial3(*this, "serial3")
	{
	}

	void h89(machine_config &config);


private:

	required_device<cpu_device> m_maincpu;
	required_memory_region m_maincpu_region;
	memory_view m_mem_view;
	required_device<ram_device> m_ram;
	required_shared_ptr<uint8_t> m_floppy_ram;
	required_device<heath_tlb_device> m_tlb;
	required_device<heath_z37_fdc_device> m_h37;
	required_device<heath_intr_cntrl> m_intr_cntrl;
	required_device<ins8250_device> m_console;
	required_device<ins8250_device> m_serial1;
	required_device<ins8250_device> m_serial2;
	required_device<ins8250_device> m_serial3;

	// General Purpose Port (GPP)
	uint8_t m_gpp;

	bool m_rom_enabled;
	bool m_timer_intr_enabled;
	bool m_floppy_ram_wp;

	// Clocks
	static constexpr XTAL H89_CLOCK = XTAL(12'288'000) / 6;
	static constexpr XTAL INS8250_CLOCK = XTAL(1'843'200);

	static constexpr uint8_t GPP_SINGLE_STEP_BIT = 0;
	static constexpr uint8_t GPP_ENABLE_TIMER_INTERRUPT_BIT = 1;
	static constexpr uint8_t GPP_DISABLE_ROM_BIT = 5;
	static constexpr uint8_t GPP_H17_SIDE_SELECT_BIT = 6;

	void update_mem_view();

	void update_gpp(uint8_t gpp);
	void port_f2_w(uint8_t data);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	TIMER_DEVICE_CALLBACK_MEMBER(h89_irq_timer);
	void h89_io(address_map &map);
	void h89_mem(address_map &map);

	uint8_t raise_NMI_r();
	void raise_NMI_w(uint8_t data);
	void console_intr(uint8_t data);
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
void h89_state::h89_mem(address_map &map)
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

/*                                 PORT
    Use                      |  Hex  |  Octal
   --------------------------+-------+---------
    Not specified, available |  0-77 |   0-167
    Cassette I/O (if used)   | 78-79 | 170-171
    Disk I/O #1              | 78-7B | 170-173
    Disk I/O #2              | 7C-7F | 174-177
    Not specified, reserved  | 80-CF | 200-317
    DCE Serial I/O           | D0-D7 | 320-327
    DTE Serial I/O           | D8-DF | 330-337
    DCE Serial I/O           | EO-E7 | 340-347
    Console I/O              | E8-EF | 350-357
    NMI                      | F0-F1 | 360-361
    General purpose port     | F2    | 362
    NMI                      | FA-FB | 372-373
*/
void h89_state::h89_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	// Disk I/O #1 - 0170-0173 (0x78-0x7b)
	//   Options
	//     - Cassette I/O (only uses 0x78 - 0x79) - Requires MTR-88 ROM
	//     - H37 5-1/4" Soft-sectored Controller - Requires MTR-90 ROM
	//     - H47 Dual 8" Drives - Requires MTR-89 or MTR-90 ROM
	//     - H67 8" Hard disk + 8" Floppy Drives - Requires MTR-90 ROM
	// map(0x78, 0x7b)
	map(0x78, 0x7b).rw(m_h37, FUNC(heath_z37_fdc_device::read), FUNC(heath_z37_fdc_device::write));

	// Disk I/O #2 - 0174-0177 (0x7c-0x7f)
	//   Options
	//     - 5-1/4" Hard-sectored Controller - supported by all ROMs
	//     - H47 Dual 8" Drives - Requires MTR-89 or MTR-90 ROM
	//     - H67 8" Hard disk + 8" Floppy Drives - MTR-90 ROM
	// map(0x7c, 0x7f)

	// 8250 UART DCE 0320 (0xd0)
	map(0xd0, 0xd7).rw(m_serial1, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	// 8250 UART DTE 0330 (0xd8) - typically used for modem
	map(0xd8, 0xdf).rw(m_serial2, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	// 8250 UART DCE 0340 (0xe0) - typically used for printer
	map(0xe0, 0xe7).rw(m_serial3, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));

	// 8250 UART console - this connects internally to the Terminal Logic board that is also used in the H19.
	map(0xe8, 0xef).rw(m_console, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));

	// ports defined on the H8. On the H89, access to these addresses causes a NMI
	map(0xf0, 0xf1).rw(FUNC(h89_state::raise_NMI_r),FUNC(h89_state::raise_NMI_w));

	// General Purpose Port (GPP)
	map(0xf2, 0xf2).w(FUNC(h89_state::port_f2_w)).portr("MTR90_SW501");

	// port defined on the H8. On the H89, access to these addresses causes a NMI
	map(0xfa, 0xfb).rw(FUNC(h89_state::raise_NMI_r), FUNC(h89_state::raise_NMI_w));
}

// Input ports
static INPUT_PORTS_START( h89 )

	// Settings with the MTR-88 ROM (#444-40)
//  PORT_START("MTR88_SW501")
//  PORT_DIPNAME( 0x1f, 0x00, "Unused" )  PORT_DIPLOCATION("S1:1,2,3,4,5")
//  PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )  PORT_DIPLOCATION("S1:6")
//  PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
//  PORT_DIPSETTING( 0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0xc0, 0x00, "Console Baud rate" )  PORT_DIPLOCATION("S1:7")
//  PORT_DIPSETTING( 0x00, "9600" )
//  PORT_DIPSETTING( 0x40, "19200" )
//  PORT_DIPSETTING( 0x80, "38400" )
//  PORT_DIPSETTING( 0xc0, "57600" )

	// Settings with the MTR-89 ROM (#444-62)
//  PORT_START("MTR89_SW501")
//  PORT_DIPNAME( 0x03, 0x00, "Disk I/O #2" )  PORT_DIPLOCATION("S1:1,2")
//  PORT_DIPSETTING( 0x00, "H-88-1" )
//  PORT_DIPSETTING( 0x01, "H/Z-47" )
//  PORT_DIPSETTING( 0x02, "Undefined" )
//  PORT_DIPSETTING( 0x03, "Undefined" )
//  PORT_DIPNAME( 0x0c, 0x00, "Disk I/O #1" )  PORT_DIPLOCATION("S1:3,4")
//  PORT_DIPSETTING( 0x00, "Unused" )
//  PORT_DIPSETTING( 0x04, "H/Z-47" )
//  PORT_DIPSETTING( 0x08, "Undefined" )
//  PORT_DIPSETTING( 0x0c, "Undefined" )
//  PORT_DIPNAME( 0x10, 0x00, "Primary Boot from" )  PORT_DIPLOCATION("S1:5")
//  PORT_DIPSETTING( 0x00, "Disk I/O #2" )
//  PORT_DIPSETTING( 0x10, "Disk I/O #1" )
//  PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )  PORT_DIPLOCATION("S1:6")
//  PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
//  PORT_DIPSETTING( 0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x40, 0x00, "Console Baud rate" )  PORT_DIPLOCATION("S1:7")
//  PORT_DIPSETTING( 0x00, "9600" )
//  PORT_DIPSETTING( 0x40, "19200" )
//  PORT_DIPNAME( 0x80, 0x00, "Boot mode" )  PORT_DIPLOCATION("S1:8")
//  PORT_DIPSETTING( 0x00, DEF_STR( Normal ) )
//  PORT_DIPSETTING( 0x80, "Auto" )

	// Settings with the MTR-90 ROM (#444-84 or 444-142)
	PORT_START("MTR90_SW501")
	PORT_DIPNAME( 0x03, 0x00, "Disk I/O #2" )  PORT_DIPLOCATION("SW501:1,2")
	PORT_DIPSETTING( 0x00, "H-88-1 (Not yet implemented)" )
	PORT_DIPSETTING( 0x01, "H/Z-47 (Not yet implemented)" )
	PORT_DIPSETTING( 0x02, "Z-67 (Not yet implemented)" )
	PORT_DIPSETTING( 0x03, "Undefined" )
	PORT_DIPNAME( 0x0c, 0x00, "Disk I/O #1" )  PORT_DIPLOCATION("SW501:3,4")
	PORT_DIPSETTING( 0x00, "H-89-37" )
	PORT_DIPSETTING( 0x04, "H/Z-47 (Not yet implemented)" )
	PORT_DIPSETTING( 0x08, "Z-67 (Not yet implemented)" )
	PORT_DIPSETTING( 0x0c, "Undefined" )
	PORT_DIPNAME( 0x10, 0x00, "Primary Boot from" )  PORT_DIPLOCATION("SW501:5")
	PORT_DIPSETTING( 0x00, "Disk I/O #2" )
	PORT_DIPSETTING( 0x10, "Disk I/O #1" )
	PORT_DIPNAME( 0x20, 0x20, "Perform memory test at start" )  PORT_DIPLOCATION("SW501:6")
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Console Baud rate" )  PORT_DIPLOCATION("SW501:7")
	PORT_DIPSETTING( 0x00, "9600" )
	PORT_DIPSETTING( 0x40, "19200" )
	PORT_DIPNAME( 0x80, 0x00, "Boot mode" )  PORT_DIPLOCATION("SW501:8")
	PORT_DIPSETTING( 0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x80, "Auto" )
INPUT_PORTS_END


void h89_state::machine_start()
{
	save_item(NAME(m_gpp));
	save_item(NAME(m_rom_enabled));
	save_item(NAME(m_timer_intr_enabled));
	save_item(NAME(m_floppy_ram_wp));

	// update RAM mappings based on RAM size
	u8 *m_ram_ptr = m_ram->pointer();
	u32 ram_size = m_ram->size();

	if (ram_size == 0x10000)
	{
		// system has a full 64k
		m_maincpu->space(AS_PROGRAM).install_ram(0x2000, 0xffff, m_ram_ptr);

		// TODO: install shadow writing to RAM when in ROM mode

		// Only the CP/M - Org 0 view will have RAM at the lower 8k
		m_mem_view[2].install_ram(0x0000, 0x1fff, m_ram_ptr + 0xe000);
	}
	else
	{
		// less than 64k

		// for views with ROM visible, the top of memory is 8k higher than
		// the memory size, since the base starts at 8k.
		u32 ram_top = ram_size + 0x1fff;

		m_mem_view[0].install_ram(0x2000, ram_top, m_ram_ptr);
		m_mem_view[1].install_ram(0x2000, ram_top, m_ram_ptr);

		// when ROM is not active, memory still starts at 8k, but is 8k smaller
		m_mem_view[2].install_ram(0x2000, ram_size - 1, m_ram_ptr);

		// remap the top 8k down to addr 0
		m_mem_view[2].install_ram(0x0000, 0x1fff, m_ram_ptr + ram_size - 0x2000);
	}

	m_rom_enabled = true;
	m_timer_intr_enabled = true;
	m_floppy_ram_wp = false;
	update_gpp(0);
	update_mem_view();
}


void h89_state::machine_reset()
{
	m_rom_enabled = true;
	m_timer_intr_enabled = true;
	m_floppy_ram_wp = false;

	update_gpp(0);
	update_mem_view();
}


uint8_t h89_state::raise_NMI_r()
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::from_usec(2));

	return 0x00;
}

void h89_state::raise_NMI_w(uint8_t)
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::from_usec(2));
}

void h89_state::console_intr(uint8_t data)
{
	if (data == CLEAR_LINE)
	{
		m_intr_cntrl->lower_irq(3);
	}
	else
	{
		m_intr_cntrl->raise_irq(3);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(h89_state::h89_irq_timer)
{
	if (m_timer_intr_enabled)
	{
		m_intr_cntrl->raise_irq(1);
	}
}

void h89_state::update_mem_view()
{
	m_mem_view.select(m_rom_enabled ? (m_floppy_ram_wp ? 0 : 1) : 2);
}

void h89_state::update_gpp(uint8_t gpp)
{
	m_gpp = gpp;

	bool new_rom_enabled = BIT(m_gpp, GPP_DISABLE_ROM_BIT) == 0;

	if (m_rom_enabled != new_rom_enabled)
	{
		m_rom_enabled = new_rom_enabled;

		update_mem_view();
	}

	m_timer_intr_enabled = bool(BIT(m_gpp, GPP_ENABLE_TIMER_INTERRUPT_BIT));
}

// General Purpose Port
void h89_state::port_f2_w(uint8_t data)
{
	update_gpp(data);

	m_intr_cntrl->lower_irq(1);
}

void h89_state::h89(machine_config & config)
{
	// basic machine hardware
	Z80(config, m_maincpu, H89_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &h89_state::h89_mem);
	m_maincpu->set_addrmap(AS_IO, &h89_state::h89_io);
	m_maincpu->set_irq_acknowledge_callback("intr_cntrl", FUNC(heath_intr_cntrl::irq_callback));

	HEATH_Z37_INTR_CNTRL(config, m_intr_cntrl);
	m_intr_cntrl->irq_line_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	RAM(config, m_ram).set_default_size("64K").set_extra_options("16K,32K,48K").set_default_value(0x00);

	INS8250(config, m_console, INS8250_CLOCK);
	m_console->out_int_callback().set(FUNC(h89_state::console_intr));

	HEATH_TLB(config, m_tlb);

	// Connect the console port on CPU board to serial port on TLB
	m_console->out_tx_callback().set(m_tlb, FUNC(heath_tlb_device::cb1_w));
	m_tlb->serial_data_callback().set(m_console, FUNC(ins8250_uart_device::rx_w));

	HEATH_Z37_FDC(config, m_h37);
	m_h37->drq_cb().set(m_intr_cntrl, FUNC(z37_intr_cntrl::set_drq));
	m_h37->irq_cb().set(m_intr_cntrl, FUNC(z37_intr_cntrl::set_intrq));
	m_h37->block_interrupt_cb().set(m_intr_cntrl, FUNC(z37_intr_cntrl::block_interrupts));

	// H-88-3 3-port serial board
	INS8250(config, m_serial1, INS8250_CLOCK);
	INS8250(config, m_serial2, INS8250_CLOCK);
	INS8250(config, m_serial3, INS8250_CLOCK);

	// H89 interrupt interval is 2mSec
	TIMER(config, "irq_timer", 0).configure_periodic(FUNC(h89_state::h89_irq_timer), attotime::from_msec(2));
}

// ROM definition
ROM_START( h89 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("mtr90")

	ROM_LOAD( "2716_444-19_h17.rom", 0x1800, 0x0800, CRC(26e80ae3) SHA1(0c0ee95d7cb1a760f924769e10c0db1678f2435c))

	ROM_SYSTEM_BIOS(0, "mtr90", "MTR-90 (444-142)")
	ROMX_LOAD("2732_444-142_mtr90.rom", 0x0000, 0x1000, CRC(c4ff47c5) SHA1(d6f3d71ff270a663003ec18a3ed1fa49f627123a), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "mtr88", "MTR-88 (444-40)")
	ROMX_LOAD("2716_444-40_mtr88.bin", 0x0000, 0x0800, CRC(093afb79) SHA1(bcc1569ad9da7babf0a4199cab96d8cd59b2dd78), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "mtr89", "MTR-89 (444-62)")
	ROMX_LOAD("2716_444-62_mtr89.bin", 0x0000, 0x0800, CRC(8f507972) SHA1(ac6c6c1344ee4e09fb60d53c85c9b761217fe9dc), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "mms84a", "MMS 84a (not working)")
	ROMX_LOAD("2732_mms84a_magnoliamms.bin", 0x0000, 0x1000, CRC(5563f42a) SHA1(1b74cafca8213d5c083f16d8a848933ab56eb43b), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "mtr90a", "MTR-90 (444-84 - not working)")
	ROMX_LOAD("2732_444-84_mtr84.rom", 0x0000, 0x1000, CRC(c98e5f4c) SHA1(03347206dca145ff69ca08435db822b70ce106af), ROM_BIOS(4))
ROM_END

} // anonymous namespace


// Driver

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY          FULLNAME        FLAGS
COMP( 1979, h89,  0,      0,      h89,     h89,   h89_state, empty_init, "Heath Company", "Heathkit H89", MACHINE_SUPPORTS_SAVE)
