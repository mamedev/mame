// license:BSD-3-Clause
// copyright-holders:Rob Krum
/***************************************************************************

    Intel QT960 - i960KB Evaluation Board

    Hardware:
    - Intel 80960KB CPU @ 20MHz
    - Intel 82380 Integrated System Peripheral (DMA, Timers, Interrupts)
    - Intel 82510 Asynchronous Serial Controller

    Memory Map (from PLD source 2U8A.PDS - address decode on A27-A31):
    0x00000000  EPROM       NINDY monitor ROM (A31-27 = 00000)
    0x08000000  Flash       Flash EPROM (A31-27 = 00001)
    0x10000000  SRAM        Static RAM (A31-27 = 00010)
    0x18000000  82380       DMA Controller (A31-27 = 00011)
    0x20000000  82510       Serial I/O (A31-27 = 00100)
    0x28000000  CSR         Control/Status (A31-27 = 00101)
    0x30000000  US0         User Space 0 (A31-27 = 00110)
    0x38000000+ OOBA        Out Of Bounds Access

    CSR Implementation (from PLD source 2U6B.ADF - 5C060 EPLD):
    - Only DB0 and DB1 connected (2-bit interface)
    - Address decode via A2, A3, A4:
      A4=0,A3=0,A2=0 (0x28000000): USR0, USR1 (User LEDs 0-1)
      A4=0,A3=0,A2=1 (0x28000004): USR2, USR3 (User LEDs 2-3)
      A4=0,A3=1,A2=0 (0x28000008): SBWAIT0#, SBWAIT1# (Burst wait states)
      A4=0,A3=1,A2=1 (0x2800000C): S1WAIT0#, S1WAIT1# (First cycle wait)
      A4=1,A3=0,A2=0 (0x28000010): WRSTR# (Write stretch, DB0 only)
      A4=1,A3=0,A2=1 (0x28000014): RSTFLG0, RSTFLG1 (Reset flags)
      A4=1,A3=1,A2=0 (0x28000018): TESTPT (Test point, read-only)

    After reset: LEDs=0, wait states=3, write stretch=1

***************************************************************************/

#include "emu.h"
#include "cpu/i960/i960.h"
#include "machine/terminal.h"


namespace {

class qt960_state : public driver_device
{
public:
	qt960_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_ram(*this, "ram")
	{ }

	void qt960(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void mem_map(address_map &map);

	// CSR handlers (directly from 5C060 PLD logic)
	uint32_t csr_r(offs_t offset);
	void csr_w(offs_t offset, uint32_t data);

	// 82510 Serial handlers
	uint32_t serial_r(offs_t offset);
	void serial_w(offs_t offset, uint32_t data);

	// 82380 DMA/Timer handlers (stub)
	uint8_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint8_t data);

	// Terminal callback
	void kbd_put(u8 data);

	required_device<i960_cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_shared_ptr<uint32_t> m_ram;

	// CSR state (directly from PLD: only 2 bits each)
	uint8_t m_usr[4];       // User LEDs (active high)
	uint8_t m_sbwait;       // Burst wait states (2 bits)
	uint8_t m_s1wait;       // First cycle wait states (2 bits)
	uint8_t m_wrstr;        // Write stretch (1 bit)
	uint8_t m_rstflg;       // Reset flags (2 bits)

	// Serial state
	uint8_t m_serial_rx_data;
	bool m_serial_rx_ready;

	// 82380 state
	std::unique_ptr<uint8_t[]> m_dma_regs;
};


void qt960_state::machine_start()
{
	m_dma_regs = std::make_unique<uint8_t[]>(0x200);

	save_item(NAME(m_usr));
	save_item(NAME(m_sbwait));
	save_item(NAME(m_s1wait));
	save_item(NAME(m_wrstr));
	save_item(NAME(m_rstflg));
	save_item(NAME(m_serial_rx_data));
	save_item(NAME(m_serial_rx_ready));
	save_pointer(NAME(m_dma_regs), 0x200);

	// Power-on initialization (only happens once, not on CPU reset)
	m_rstflg = 0;  // Both flags 0 at power-up
}

void qt960_state::machine_reset()
{
	// CSR reset values from PLD (2U6B.ADF):
	// - USR0-3: cleared by RESET
	// - SBWAIT/S1WAIT: set to 3 (inverted logic in PLD, /SBWAITx# resets high)
	// - WRSTR: set to 1 (inverted logic, /WRSTR# resets high)
	// - RSTFLG0: NOT affected by RESET (persists!)
	// - RSTFLG1: cleared by RESET
	m_usr[0] = 0;
	m_usr[1] = 0;
	m_usr[2] = 0;
	m_usr[3] = 0;
	m_sbwait = 3;    // Default 3 wait states
	m_s1wait = 3;    // Default 3 wait states
	m_wrstr = 1;     // Write stretch enabled after reset

	// RSTFLG special handling from PLD:
	// RSTFLG0 does NOT have reset - it persists across CPU reset
	// RSTFLG1 DOES have reset - it gets cleared
	m_rstflg = m_rstflg & 0x01;  // Clear bit 1, preserve bit 0

	// Serial reset
	m_serial_rx_data = 0;
	m_serial_rx_ready = false;

	// 82380 reset
	std::fill_n(m_dma_regs.get(), 0x200, 0);
}


//**************************************************************************
//  CSR - 5C060 EPLD (from 2U6B.ADF PLD source)
//  Address decode: A4, A3, A2 select register (bits 2-4 of address)
//  Data: Only DB0, DB1 connected (2-bit interface)
//**************************************************************************

uint32_t qt960_state::csr_r(offs_t offset)
{
	uint32_t data = 0;

	// Extract A4, A3, A2 from offset (offset is in 32-bit words)
	// Address bits: offset * 4 gives byte address, then A4=bit4, A3=bit3, A2=bit2
	int a2 = (offset >> 0) & 1;
	int a3 = (offset >> 1) & 1;
	int a4 = (offset >> 2) & 1;

	if (a4 == 0 && a3 == 0 && a2 == 0)
	{
		// 0x28000000: USR0 on DB0, USR1 on DB1
		data = (m_usr[0] & 1) | ((m_usr[1] & 1) << 1);
	}
	else if (a4 == 0 && a3 == 0 && a2 == 1)
	{
		// 0x28000004: USR2 on DB0, USR3 on DB1
		data = (m_usr[2] & 1) | ((m_usr[3] & 1) << 1);
	}
	else if (a4 == 0 && a3 == 1 && a2 == 0)
	{
		// 0x28000008: SBWAIT0 on DB0, SBWAIT1 on DB1
		data = m_sbwait & 0x03;
	}
	else if (a4 == 0 && a3 == 1 && a2 == 1)
	{
		// 0x2800000C: S1WAIT0 on DB0, S1WAIT1 on DB1
		data = m_s1wait & 0x03;
	}
	else if (a4 == 1 && a3 == 0 && a2 == 0)
	{
		// 0x28000010: WRSTR on DB0 only
		data = m_wrstr & 0x01;
	}
	else if (a4 == 1 && a3 == 0 && a2 == 1)
	{
		// 0x28000014: RSTFLG0 on DB0, RSTFLG1 on DB1
		data = m_rstflg & 0x03;
	}
	else if (a4 == 1 && a3 == 1 && a2 == 0)
	{
		// 0x28000018: TESTPT on DB0 (active low, directly driven externally)
		data = 0; // Test point reads as 0
	}

	return data;
}

void qt960_state::csr_w(offs_t offset, uint32_t data)
{
	int a2 = (offset >> 0) & 1;
	int a3 = (offset >> 1) & 1;
	int a4 = (offset >> 2) & 1;

	if (a4 == 0 && a3 == 0 && a2 == 0)
	{
		// 0x28000000: USR0 from DB0, USR1 from DB1
		m_usr[0] = (data >> 0) & 1;
		m_usr[1] = (data >> 1) & 1;
	}
	else if (a4 == 0 && a3 == 0 && a2 == 1)
	{
		// 0x28000004: USR2 from DB0, USR3 from DB1
		m_usr[2] = (data >> 0) & 1;
		m_usr[3] = (data >> 1) & 1;
	}
	else if (a4 == 0 && a3 == 1 && a2 == 0)
	{
		// 0x28000008: SBWAIT
		m_sbwait = data & 0x03;
	}
	else if (a4 == 0 && a3 == 1 && a2 == 1)
	{
		// 0x2800000C: S1WAIT
		m_s1wait = data & 0x03;
	}
	else if (a4 == 1 && a3 == 0 && a2 == 0)
	{
		// 0x28000010: WRSTR
		m_wrstr = data & 0x01;
	}
	else if (a4 == 1 && a3 == 0 && a2 == 1)
	{
		// 0x28000014: RSTFLG (writable per PLD equations)
		m_rstflg = data & 0x03;
	}
	// 0x28000018: TESTPT is read-only
}


//**************************************************************************
//  82510 Serial Controller
//**************************************************************************

uint32_t qt960_state::serial_r(offs_t offset)
{
	uint32_t data = 0;

	switch (offset)
	{
	case 0: // RXD
		data = m_serial_rx_data;
		m_serial_rx_ready = false;
		break;

	case 5: // LSR - Line Status Register
		data = 0x60; // THRE + TEMT (TX ready)
		if (m_serial_rx_ready)
			data |= 0x01; // DR (data ready)
		break;

	case 6: // MSR - Modem Status Register
		data = 0x30; // CTS + DSR
		break;
	}

	return data;
}

void qt960_state::serial_w(offs_t offset, uint32_t data)
{
	if (offset == 0) // TXD
	{
		m_terminal->write(data & 0xff);
	}
}

void qt960_state::kbd_put(u8 data)
{
	m_serial_rx_data = data;
	m_serial_rx_ready = true;
}


//**************************************************************************
//  82380 DMA/Timer Controller
//  From qtcommon.h: RESET_ADDR = 0x18000064, RESET_DATA = 0xf0
//**************************************************************************

uint8_t qt960_state::dma_r(offs_t offset)
{
	return m_dma_regs[offset & 0x1ff];
}

void qt960_state::dma_w(offs_t offset, uint8_t data)
{
	m_dma_regs[offset & 0x1ff] = data;

	// Check for CPU reset command (from qtcommon.h)
	// RESET_ADDR = 0x18000064, RESET_DATA = 0xf0
	// offset is relative to 0x18000000, so check for offset 0x64
	if (offset == 0x64 && (data & 0xf0) == 0xf0)
	{
		logerror("82380: CPU reset triggered\n");
		// Schedule a soft reset for the next timeslice
		// This calls machine_reset() which preserves RSTFLG0
		machine().schedule_soft_reset();
	}
}


//**************************************************************************
//  Memory Map
//**************************************************************************

// Every region is BURST: the QT960's bus has no FIFO, so a multi-word access (ldl, ldt,
// ldq, stl, stt, stq) moves on a word for each word, as on the board. MAME's i960 holds
// the address still without the flag (for Model 2's FIFOs): NINDY's "dd" then showed 0
// for the high word of every pair, because its stl wrote both words to one address.
void qt960_state::mem_map(address_map &map)
{
	// EPROM at 0x00000000 (A31-27 = 00000)
	map(0x00000000, 0x0001ffff).rom().region("maincpu", 0).flags(i960_cpu_device::BURST);

	// Flash at 0x08000000 (A31-27 = 00001) - writable for downloads
	map(0x08000000, 0x0801ffff).ram().flags(i960_cpu_device::BURST);

	// SRAM at 0x10000000 (A31-27 = 00010)
	map(0x10000000, 0x101fffff).ram().share("ram").flags(i960_cpu_device::BURST);

	// 82380 at 0x18000000 (A31-27 = 00011)
	map(0x18000000, 0x1801ffff).rw(FUNC(qt960_state::dma_r), FUNC(qt960_state::dma_w)).flags(i960_cpu_device::BURST);

	// 82510 Serial at 0x20000000 (A31-27 = 00100)
	map(0x20000000, 0x2000001f).rw(FUNC(qt960_state::serial_r), FUNC(qt960_state::serial_w)).flags(i960_cpu_device::BURST);

	// CSR at 0x28000000 (A31-27 = 00101)
	map(0x28000000, 0x2800001f).rw(FUNC(qt960_state::csr_r), FUNC(qt960_state::csr_w)).flags(i960_cpu_device::BURST);

	// User Space 0 at 0x30000000 (A31-27 = 00110) - stub as RAM
	map(0x30000000, 0x3001ffff).ram().flags(i960_cpu_device::BURST);
}


//**************************************************************************
//  Machine Configuration
//**************************************************************************

void qt960_state::qt960(machine_config &config)
{
	I80960KB(config, m_maincpu, XTAL(40'000'000) / 2); // 20MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &qt960_state::mem_map);

	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(qt960_state::kbd_put));
}


//**************************************************************************
//  ROM Definitions
//**************************************************************************

ROM_START(qt960)
	ROM_REGION32_LE(0x20000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("nindy.bin", 0x00000, 0x20000, CRC(325251fe) SHA1(eda16ca24366b95d857bb3442fdb41f500f805a8))
ROM_END

} // anonymous namespace


//**************************************************************************
//  System Drivers
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME                        FLAGS
COMP( 1989, qt960, 0,      0,      qt960,   0,     qt960_state, empty_init, "Intel", "QT960 i960KB Evaluation Board", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
