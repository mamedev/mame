// license:BSD-3-Clause
// copyright-holders: Karl Stenerud / NaokiS

/*
	Notes:
		Chip Select Module:
			* Not currently implemented, quite possible it doesn't need to be added for any functional reason aside from completeness/debugging
		System Integration Module:
			* MBAR: should be set up in m68000_musashi_device::x4e7a_movec_l_c() to move the ColdFire IO register map according to the contents of MBAR
			* SIMR: Logged and stored, but not used as there is no BDM interface present
			* MARB: Logged and stored, but not used, not required?
		DRAM Controller Module:
			* All functions just log and store the registers, again, possibly not required for any functional reasons.
		UART Modules:
			* Uses the MC68681 device driver as a base as effectively it is an integrated one. The difference however is both UART modules are entirely
				independant of the other, so there is two command registers e.t.c. They also only have a single channel each.
			* In this implementation, the driver uses two MC68681s at the appropriate offsets to emulate this, with the MC68681 driver having an
				entry for the MCF5206E which changes the mapping a little bit. A more appropriate solution would be to have a single UART module and load
				two of them.
		Timer Modules:
			* Slow, could be improved.

		* Some of these modules can probably be split back into the /devices/machine device, namely the MBUS and maybe timer modules, but the 
			System Integration Module (which has an included interrupt controller) needs to have more access to the CPU lines than the other modules.
*/

#include "emu.h"
#include "mcf5206e.h"
#include "m68kdasm.h"

#define LOG_DEBUG       (1U << 1)
#define LOG_INVALID     (1U << 2)
#define LOG_TIMER       (1U << 3)
#define LOG_UART	    (1U << 4)
#define LOG_SWDT	    (1U << 5)
#define LOG_MBUS	    (1U << 6)
#define LOG_DRAM	    (1U << 7)
#define VERBOSE 		( LOG_DEBUG | LOG_UART | LOG_SWDT | LOG_MBUS | LOG_DRAM )
#include "logmacro.h"

#define INT_LEVEL(N) 	((N&0x1c) >> 2)
#define ICR_USE_AUTOVEC(N)	((N & 0x80) != 0)

DEFINE_DEVICE_TYPE(MCF5206E,    mcf5206e_device,    "mcf5206e",     "Freescale MCF5206E")

std::unique_ptr<util::disasm_interface> mcf5206e_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_COLDFIRE);
}

mcf5206e_device::mcf5206e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, MCF5206E, 32, 32, address_map_constructor(FUNC(mcf5206e_device::coldfire_regs_map), this))
	, m_uart(*this, "coldfire_uart%u", 0U)
	, m_swdt(*this, "watchdog")
	, write_chip_select(*this)
	, m_gpio_r_cb(*this, 0xFF)
	, m_gpio_w_cb(*this)
	, write_irq(*this)
	, write_tx1(*this)
	, write_tx2(*this)
	, write_sda(*this)
	, write_scl(*this)
{
}

void mcf5206e_device::device_add_mconfig(machine_config &config){
	MCF5206E_UART(config, m_uart[0], (this->clock() / 32));
	MCF5206E_UART(config, m_uart[1], (this->clock() / 32));
	m_uart[0]->irq_cb().set(FUNC(mcf5206e_device::uart1_irq_w));
	m_uart[1]->irq_cb().set(FUNC(mcf5206e_device::uart2_irq_w));
	
	WATCHDOG_TIMER(config, m_swdt);
}

void mcf5206e_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_coldfire();

	init_regs(true);

	m_swdt->watchdog_enable(0);

	m_timer1 = timer_alloc( FUNC( mcf5206e_device::timer1_callback ), this );
	m_timer2 = timer_alloc( FUNC( mcf5206e_device::timer2_callback ), this );
	m_timer_swdt = timer_alloc( FUNC( mcf5206e_device::swdt_callback ), this );	// For interrupt version
	m_timer_mbus = timer_alloc( FUNC( mcf5206e_device::mbus_callback ), this );

	save_item(NAME(m_SIMR));
	save_item(NAME(m_MARB));

	save_item(NAME(m_DCRR));
	save_item(NAME(m_DCTR));
	save_item(NAME(m_DCAR0));
	save_item(NAME(m_DCMR0));
	save_item(NAME(m_DCCR0));
	save_item(NAME(m_DCAR1));
	save_item(NAME(m_DCMR1));
	save_item(NAME(m_DCCR1));

	save_item(NAME(m_ICR));
	save_item(NAME(m_CSAR));
	save_item(NAME(m_CSMR));
	save_item(NAME(m_CSCR));
	save_item(NAME(m_DMCR));

	save_item(NAME(m_PAR));
	save_item(NAME(m_IMR));
	save_item(NAME(m_IPR));
	save_item(NAME(m_SYPCR));
	save_item(NAME(m_SWIVR));
	save_item(NAME(m_RSR));
	save_item(NAME(m_swdt_w_count));
	save_item(NAME(m_sypcr_locked));

	save_item(NAME(m_TMR1));
	save_item(NAME(m_TRR1));
	save_item(NAME(m_TCR1));
	save_item(NAME(m_TCN1));
	save_item(NAME(m_TER1));
	save_item(NAME(m_TMR2));
	save_item(NAME(m_TRR2));
	save_item(NAME(m_TCR2));
	save_item(NAME(m_TCN2));
	save_item(NAME(m_TER2));

	save_item(NAME(m_PPDDR));
	save_item(NAME(m_PPDATI));
	save_item(NAME(m_PPDATO));
	
	save_item(NAME(m_MADR));
	save_item(NAME(m_MBCR));
	save_item(NAME(m_MBSR));
	save_item(NAME(m_MFDR));
	save_item(NAME(m_MBDR));

	save_item(NAME(m_coldfire_regs));
}

void mcf5206e_device::device_reset()
{
	m68000_musashi_device::device_reset();
	init_regs(false);
	m_timer1->adjust(attotime::never);
	m_timer2->adjust(attotime::never);
}

enum {
	T_RST = (1 << 0),
	T_CL0 = (1 << 1),
	T_CL1 = (1 << 2),
	T_FRR = (1 << 3),
	T_ORI = (1 << 4),
	T_OM  = (1 << 5),
	T_CE0 = (1 << 6),
	T_CE1 = (1 << 7),
	T_PS0 = (1 << 8),
	T_PS1 = (1 << 9),
	T_PS2 = (1 << 10),
	T_PS3 = (1 << 11),
	T_PS4 = (1 << 12),
	T_PS5 = (1 << 13),
	T_PS6 = (1 << 14),
	T_PS7 = (1 << 15)
};

enum {
	T_ECAP = (1 << 0),
	T_EREF = (1 << 1)
};

enum {
	ICR1 = 0,	// Bit 1
	ICR2,		// Bit 2
	ICR3,		// Bit 3
	ICR4,		// Bit 4
	ICR5,		// Bit 5
	ICR6,		// Bit 6
	ICR7,       // Bit 7
	ICR_SWDT,   // Bit 8
	ICR_TMR1,   // Bit 9
	ICR_TMR2,   // Bit 10
	ICR_MBUS,   // Bit 11
	ICR_UART1,  // Bit 12
	ICR_UART2,  // Bit 13
	ICR_DMA0,	// Bit 14
	ICR_DMA1,	// Bit 15
	MAX_ICR
};

// Pin assignment register
enum {
	PAR0 = (1 << 0),
	PAR1 = (1 << 1),
	PAR2 = (1 << 2),
	PAR3 = (1 << 3),
	PAR4 = (1 << 4),
	PAR5 = (1 << 5),
	PAR6 = (1 << 6),
	PAR7 = (1 << 7),
	PAR8 = (1 << 8),
	PAR9 = (1 << 9),
	PAR10 = (1 << 10),
	PAR11 = (1 << 11),
	PAR12 = (1 << 12),
	PAR13 = (1 << 13),
	PAR14 = (1 << 14),
	PAR15 = (1 << 15)
};

// MBCR
enum {
	RSTA = (1 << 2),
	TXAK = (1 << 3),
	MTX  = (1 << 4),
	MSTA = (1 << 5),
	MIEN = (1 << 6),
	MEN  = (1 << 7)
};

// should be set up in m68000_musashi_device::x4e7a_movec_l_c() to move this map according to the contents of MBAR
void mcf5206e_device::coldfire_regs_map(address_map &map)
{
	/* SIM Module */
	map(0xf0000003, 0xf0000003).rw(FUNC(mcf5206e_device::SIMR_r), FUNC(mcf5206e_device::SIMR_w));
	map(0xf0000007, 0xf0000007).rw(FUNC(mcf5206e_device::MARB_r), FUNC(mcf5206e_device::MARB_w));

	/* Interrupt Control Registers */
	map(0xf0000014, 0xf0000014).rw(FUNC(mcf5206e_device::ICR1_r), FUNC(mcf5206e_device::ICR1_w));
	map(0xf0000015, 0xf0000015).rw(FUNC(mcf5206e_device::ICR2_r), FUNC(mcf5206e_device::ICR2_w));
	map(0xf0000016, 0xf0000016).rw(FUNC(mcf5206e_device::ICR3_r), FUNC(mcf5206e_device::ICR3_w));
	map(0xf0000017, 0xf0000017).rw(FUNC(mcf5206e_device::ICR4_r), FUNC(mcf5206e_device::ICR4_w));
	map(0xf0000018, 0xf0000018).rw(FUNC(mcf5206e_device::ICR5_r), FUNC(mcf5206e_device::ICR5_w));
	map(0xf0000019, 0xf0000019).rw(FUNC(mcf5206e_device::ICR6_r), FUNC(mcf5206e_device::ICR6_w));
	map(0xf000001a, 0xf000001a).rw(FUNC(mcf5206e_device::ICR7_r), FUNC(mcf5206e_device::ICR7_w));
	map(0xf000001b, 0xf000001b).rw(FUNC(mcf5206e_device::ICR8_r), FUNC(mcf5206e_device::ICR8_w));
	map(0xf000001c, 0xf000001c).rw(FUNC(mcf5206e_device::ICR9_r), FUNC(mcf5206e_device::ICR9_w));
	map(0xf000001d, 0xf000001d).rw(FUNC(mcf5206e_device::ICR10_r), FUNC(mcf5206e_device::ICR10_w));
	map(0xf000001e, 0xf000001e).rw(FUNC(mcf5206e_device::ICR11_r), FUNC(mcf5206e_device::ICR11_w));
	map(0xf000001f, 0xf000001f).rw(FUNC(mcf5206e_device::ICR12_r), FUNC(mcf5206e_device::ICR12_w));
	map(0xf0000020, 0xf0000020).rw(FUNC(mcf5206e_device::ICR13_r), FUNC(mcf5206e_device::ICR13_w));
	map(0xf0000021, 0xf0000021).rw(FUNC(mcf5206e_device::ICR14_r), FUNC(mcf5206e_device::ICR14_w));
	map(0xf0000022, 0xf0000022).rw(FUNC(mcf5206e_device::ICR15_r), FUNC(mcf5206e_device::ICR15_w));
	map(0xf0000036, 0xf0000037).rw(FUNC(mcf5206e_device::IMR_r), FUNC(mcf5206e_device::IMR_w));
	map(0xf000003a, 0xf000003b).r(FUNC(mcf5206e_device::IPR_r));

	map(0xf0000040, 0xf0000040).rw(FUNC(mcf5206e_device::RSR_r), FUNC(mcf5206e_device::RSR_w));
	map(0xf0000041, 0xf0000041).rw(FUNC(mcf5206e_device::SYPCR_r), FUNC(mcf5206e_device::SYPCR_w));
	map(0xf0000042, 0xf0000042).rw(FUNC(mcf5206e_device::SWIVR_r), FUNC(mcf5206e_device::SWIVR_w));
	map(0xf0000043, 0xf0000043).w(FUNC(mcf5206e_device::SWSR_w));

	/* DRAM Controller */
	map(0xf0000046, 0xf0000047).rw(FUNC(mcf5206e_device::DCRR_r), FUNC(mcf5206e_device::DCRR_w));
	map(0xf000004a, 0xf000004b).rw(FUNC(mcf5206e_device::DCTR_r), FUNC(mcf5206e_device::DCTR_w));
	map(0xf000004c, 0xf000004d).rw(FUNC(mcf5206e_device::DCAR0_r), FUNC(mcf5206e_device::DCAR0_w));
	map(0xf0000050, 0xf0000053).rw(FUNC(mcf5206e_device::DCMR0_r), FUNC(mcf5206e_device::DCMR0_w));
	map(0xf0000057, 0xf0000057).rw(FUNC(mcf5206e_device::DCCR0_r), FUNC(mcf5206e_device::DCCR0_w));
	map(0xf0000058, 0xf0000059).rw(FUNC(mcf5206e_device::DCAR1_r), FUNC(mcf5206e_device::DCAR1_w));
	map(0xf000005c, 0xf000005f).rw(FUNC(mcf5206e_device::DCMR1_r), FUNC(mcf5206e_device::DCMR1_w));
	map(0xf0000063, 0xf0000063).rw(FUNC(mcf5206e_device::DCCR1_r), FUNC(mcf5206e_device::DCCR1_w));

	/* Chip Select registers */
	map(0xf0000064, 0xf0000067).rw(FUNC(mcf5206e_device::CSAR0_r), FUNC(mcf5206e_device::CSAR0_w));
	map(0xf0000068, 0xf000006b).rw(FUNC(mcf5206e_device::CSMR0_r), FUNC(mcf5206e_device::CSMR0_w));
	map(0xf000006c, 0xf000006f).rw(FUNC(mcf5206e_device::CSCR0_r), FUNC(mcf5206e_device::CSCR0_w));
	map(0xf0000070, 0xf0000073).rw(FUNC(mcf5206e_device::CSAR1_r), FUNC(mcf5206e_device::CSAR1_w));
	map(0xf0000074, 0xf0000077).rw(FUNC(mcf5206e_device::CSMR1_r), FUNC(mcf5206e_device::CSMR1_w));
	map(0xf0000078, 0xf000007b).rw(FUNC(mcf5206e_device::CSCR1_r), FUNC(mcf5206e_device::CSCR1_w));
	map(0xf000007c, 0xf000007f).rw(FUNC(mcf5206e_device::CSAR2_r), FUNC(mcf5206e_device::CSAR2_w));
	map(0xf0000080, 0xf0000083).rw(FUNC(mcf5206e_device::CSMR2_r), FUNC(mcf5206e_device::CSMR2_w));
	map(0xf0000084, 0xf0000087).rw(FUNC(mcf5206e_device::CSCR2_r), FUNC(mcf5206e_device::CSCR2_w));
	map(0xf0000088, 0xf000008b).rw(FUNC(mcf5206e_device::CSAR3_r), FUNC(mcf5206e_device::CSAR3_w));
	map(0xf000008c, 0xf000008f).rw(FUNC(mcf5206e_device::CSMR3_r), FUNC(mcf5206e_device::CSMR3_w));
	map(0xf0000090, 0xf0000093).rw(FUNC(mcf5206e_device::CSCR3_r), FUNC(mcf5206e_device::CSCR3_w));
	map(0xf0000094, 0xf0000097).rw(FUNC(mcf5206e_device::CSAR4_r), FUNC(mcf5206e_device::CSAR4_w));
	map(0xf0000098, 0xf000009b).rw(FUNC(mcf5206e_device::CSMR4_r), FUNC(mcf5206e_device::CSMR4_w));
	map(0xf000009c, 0xf000009f).rw(FUNC(mcf5206e_device::CSCR4_r), FUNC(mcf5206e_device::CSCR4_w));
	map(0xf00000a0, 0xf00000a3).rw(FUNC(mcf5206e_device::CSAR5_r), FUNC(mcf5206e_device::CSAR5_w));
	map(0xf00000a4, 0xf00000a7).rw(FUNC(mcf5206e_device::CSMR5_r), FUNC(mcf5206e_device::CSMR5_w));
	map(0xf00000a8, 0xf00000ab).rw(FUNC(mcf5206e_device::CSCR5_r), FUNC(mcf5206e_device::CSCR5_w));
	map(0xf00000ac, 0xf00000af).rw(FUNC(mcf5206e_device::CSAR6_r), FUNC(mcf5206e_device::CSAR6_w));
	map(0xf00000b0, 0xf00000b3).rw(FUNC(mcf5206e_device::CSMR6_r), FUNC(mcf5206e_device::CSMR6_w));
	map(0xf00000b4, 0xf00000b7).rw(FUNC(mcf5206e_device::CSCR6_r), FUNC(mcf5206e_device::CSCR6_w));
	map(0xf00000b8, 0xf00000bb).rw(FUNC(mcf5206e_device::CSAR7_r), FUNC(mcf5206e_device::CSAR7_w));
	map(0xf00000bc, 0xf00000bf).rw(FUNC(mcf5206e_device::CSMR7_r), FUNC(mcf5206e_device::CSMR7_w));
	map(0xf00000c0, 0xf00000c3).rw(FUNC(mcf5206e_device::CSCR7_r), FUNC(mcf5206e_device::CSCR7_w));

	map(0xf00000c4, 0xf00000c7).rw(FUNC(mcf5206e_device::DMCR_r), FUNC(mcf5206e_device::DMCR_w));
	map(0xf00000ca, 0xf00000cb).rw(FUNC(mcf5206e_device::PAR_r), FUNC(mcf5206e_device::PAR_w));

	// Timer 1
	map(0xf0000100, 0xf0000101).rw(FUNC(mcf5206e_device::TMR1_r), FUNC(mcf5206e_device::TMR1_w));
	map(0xf0000104, 0xf0000105).rw(FUNC(mcf5206e_device::TRR1_r), FUNC(mcf5206e_device::TRR1_w));
	map(0xf0000108, 0xf0000109).r(FUNC(mcf5206e_device::TCR1_r));	// TCR is r/only
	map(0xf000010c, 0xf000010d).rw(FUNC(mcf5206e_device::TCN1_r), FUNC(mcf5206e_device::TCN1_w));
	map(0xf0000111, 0xf0000111).rw(FUNC(mcf5206e_device::TER1_r), FUNC(mcf5206e_device::TER1_w));

	// Timer 2
	map(0xf0000120, 0xf0000121).rw(FUNC(mcf5206e_device::TMR2_r), FUNC(mcf5206e_device::TMR2_w));
	map(0xf0000124, 0xf0000125).rw(FUNC(mcf5206e_device::TRR2_r), FUNC(mcf5206e_device::TRR2_w));
	map(0xf0000128, 0xf0000129).r(FUNC(mcf5206e_device::TCR2_r));	// TCR is r/only
	map(0xf000012c, 0xf000012d).rw(FUNC(mcf5206e_device::TCN2_r), FUNC(mcf5206e_device::TCN2_w));
	map(0xf0000131, 0xf0000131).rw(FUNC(mcf5206e_device::TER2_r), FUNC(mcf5206e_device::TER2_w));

	// UART (mc68681 derrived)
	map(0xf0000140, 0xf000017c).rw(FUNC(mcf5206e_device::uart1_r), FUNC(mcf5206e_device::uart1_w));
	map(0xf0000180, 0xf00001bc).rw(FUNC(mcf5206e_device::uart2_r), FUNC(mcf5206e_device::uart2_w));

	// Parallel Port
	map(0xf00001c5, 0xf00001c5).rw(FUNC(mcf5206e_device::PPDDR_r), FUNC(mcf5206e_device::PPDDR_w));
	map(0xf00001c9, 0xf00001c9).rw(FUNC(mcf5206e_device::PPDAT_r), FUNC(mcf5206e_device::PPDAT_w));

	// MBus (I2C)
	map(0xf00001e0, 0xf00001e0).rw(FUNC(mcf5206e_device::MADR_r), FUNC(mcf5206e_device::MADR_w));
	map(0xf00001e4, 0xf00001e4).rw(FUNC(mcf5206e_device::MFDR_r), FUNC(mcf5206e_device::MFDR_w));
	map(0xf00001e8, 0xf00001e8).rw(FUNC(mcf5206e_device::MBCR_r), FUNC(mcf5206e_device::MBCR_w));
	map(0xf00001ec, 0xf00001ec).rw(FUNC(mcf5206e_device::MBSR_r), FUNC(mcf5206e_device::MBSR_w));
	map(0xf00001f0, 0xf00001f0).rw(FUNC(mcf5206e_device::MBDR_r), FUNC(mcf5206e_device::MBDR_w));
}

// SIM Configuration Register - Not really applicable to MAME as there's no BDM port currently but hey.
// MBAR + 0x003
uint8_t mcf5206e_device::SIMR_r(){
	LOGMASKED(LOG_DEBUG, "%s: SIMR_r %02x\n", this->machine().describe_context(), m_SIMR);
	return m_SIMR;
}

void  mcf5206e_device::SIMR_w(uint8_t data){
	LOGMASKED(LOG_DEBUG, "%s: SIMR_w %02x\n", this->machine().describe_context(), data);
	m_SIMR = data;
}

// Bus Master Arbitration Control
// MBAR + 0x003
uint8_t mcf5206e_device::MARB_r(){
	LOGMASKED(LOG_DEBUG, "%s: (Bus Master Arbitration Control) MARB_r %02x\n", this->machine().describe_context(), m_MARB);
	return m_MARB;
}

void mcf5206e_device::MARB_w(uint8_t data){
	LOGMASKED(LOG_DEBUG, "%s: (Bus Master Arbitration Control) MARB_w %02x\n", this->machine().describe_context(), data);
	m_MARB = data;
}

// MBAR + 0x014 -> 0x022
uint8_t mcf5206e_device::ICRx_r(offs_t offset){
	LOGMASKED(LOG_DEBUG, "%s: (Interrupt Control Register %d) read: %02x\n", this->machine().describe_context(), offset, m_ICR[offset]);
	return m_ICR[offset - 1];
}

void mcf5206e_device::ICRx_w(offs_t offset, uint8_t data){
	m_ICR[offset - 1] = data;
	LOGMASKED(LOG_DEBUG, "%s: (Interrupt Control Register %d) write: %02x\n", this->machine().describe_context(), offset, data);
	ICR_info(m_ICR[offset]);
}

/* SIM Module - Interrupts */
// MBAR + 0x036
uint16_t mcf5206e_device::IMR_r()
{
	LOGMASKED(LOG_DEBUG, "%s: (Interrupt Mask Register) IMR_r %04x\n", this->machine().describe_context(), m_IMR);
	return m_IMR;
}

void mcf5206e_device::IMR_w(uint16_t data)
{
	m_IMR = (data & 0xFFFE);
	LOGMASKED(LOG_DEBUG, "%s: (Interrupt Mask Register) IMR_w %04x\n", this->machine().describe_context(), data);
}

// MBAR + 0x03A
uint16_t mcf5206e_device::IPR_r()
{
	LOGMASKED(LOG_DEBUG, "%s: (Interrupt Pending Register) IPR_r\n", this->machine().describe_context());
	return m_IPR;
}


void mcf5206e_device::ICR_info(uint8_t ICR)
{
	LOGMASKED(LOG_DEBUG, "  (AutoVector) AVEC : %01x | ", (ICR&0x80)>>7);
	LOGMASKED(LOG_DEBUG, "(Interrupt Level) IL : %01x | ", INT_LEVEL(ICR)); // if autovector (AVEC) is used then the vectors referenced are at +24 (+0x18) + IL, ie the standard 68k autovectors, otherwise vector must be provided by device
	LOGMASKED(LOG_DEBUG, "(Interrupt Priority) IP : %01x |", (ICR&0x03)>>0);
	LOGMASKED(LOG_DEBUG, "(Unused bits) : %01x\n", (ICR&0x60)>>5);
}


/* DRAM Controller */
uint16_t mcf5206e_device::DCRR_r(){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Refresh Register) DCRR_r: %04x\n", this->machine().describe_context(), m_DCRR);
	return m_DCRR;
}

void mcf5206e_device::DCRR_w(uint16_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Refresh Register) DCRR_w: %04x\n", this->machine().describe_context(), data);
	m_DCRR = data;
}

uint16_t mcf5206e_device::DCTR_r(){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Timing Register) DCTR_r: %04x\n", this->machine().describe_context(), m_DCTR);
	return m_DCTR;
}

void mcf5206e_device::DCTR_w(uint16_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Timing Register) DCTR_w: %04x\n", this->machine().describe_context(), data);
	m_DCTR = data;
}

uint16_t mcf5206e_device::DCAR0_r(){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Access Register 0) DCAR0_r: %04x\n", this->machine().describe_context(), m_DCAR0);
	return m_DCAR0;
}

void mcf5206e_device::DCAR0_w(uint16_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Access Register 0) DCAR0_w: %04x\n", this->machine().describe_context(), data);
	m_DCAR0 = data;
}

uint32_t mcf5206e_device::DCMR0_r(){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Mask Register 0) DCMR0_r: %08x\n", this->machine().describe_context(), m_DCMR0);
	return m_DCMR0;
}

void mcf5206e_device::DCMR0_w(uint32_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Mask Register 0) DCMR0_w: %08x\n", this->machine().describe_context(), data);
	m_DCMR0 = data;
}

uint8_t mcf5206e_device::DCCR0_r(){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Control Register 0) DCCR0_r: %04x\n", this->machine().describe_context(), m_DCCR0);
	return m_DCCR0;
}

void mcf5206e_device::DCCR0_w(uint8_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Control Register 0) DCCR0_w: %04x\n", this->machine().describe_context(), data);
	m_DCCR0 = data;
}

uint16_t mcf5206e_device::DCAR1_r(){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Access Register 1) DCAR1_r: %04x\n", this->machine().describe_context(), m_DCAR1);
	return m_DCAR1;
}

void mcf5206e_device::DCAR1_w(uint16_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Access Register 1) DCAR1_w: %04x\n", this->machine().describe_context(), data);
	m_DCAR1 = data;
}

uint32_t mcf5206e_device::DCMR1_r(){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Mask Register 1) DCMR1_r: %08x\n", this->machine().describe_context(), m_DCMR1);
	return m_DCMR1;
}

void mcf5206e_device::DCMR1_w(uint32_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Mask Register 1) DCMR1_w: %04x\n", this->machine().describe_context(), data);
	m_DCMR1 = data;
}

uint8_t mcf5206e_device::DCCR1_r(){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Control Register 1) DCCR1_r: %04x\n", this->machine().describe_context(), m_DCCR1);
	return m_DCCR1;
}

void mcf5206e_device::DCCR1_w(uint8_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Control Register 1) DCCR1_w: %04x\n", this->machine().describe_context(), data);
	m_DCCR1 = data;
}

// Decides whether to forward interrupt to CPU or not
void mcf5206e_device::set_interrupt(IntSrc interrupt){
	LOG("set_interrupt(%d): %d, %d", interrupt, BIT(m_IMR, interrupt), m_IMR & interrupt);
	if(!BIT(m_IMR, interrupt)){
		uint8_t ICR = 0;
		uint8_t vector = ~0;
		uint32_t intbit = (1 << interrupt);
		
		m_IPR = ((m_IPR & ~intbit) | intbit);

		switch (interrupt){
			case WDINT: 								// Never autovector
				ICR = m_ICR[ICR_SWDT]; 
				vector = m_SWIVR;
				break;	
			case TINT1:	ICR = m_ICR[ICR_TMR1]; break;	// Always autovector
			case TINT2: ICR = m_ICR[ICR_TMR2]; break;	// Always autovector
			case MBINT: ICR = m_ICR[ICR_MBUS]; break;	// Always autovector
			case UINT1: 
				ICR = m_ICR[ICR_UART1];
				if (!ICR_USE_AUTOVEC(ICR)) vector = m_uart[0]->get_irq_vector();
				break;
			case UINT2: 
				ICR = m_ICR[ICR_UART2];
				if (!ICR_USE_AUTOVEC(ICR)) vector = m_uart[1]->get_irq_vector();
				break;
			
			default: return;
		}

		if (ICR_USE_AUTOVEC(ICR)){
			LOGMASKED(LOG_DEBUG, "%s: Interrupt from %d: ICR: %02x\n", this->machine().describe_context(), interrupt, ICR);
			set_input_line_and_vector(INT_LEVEL(ICR), HOLD_LINE, INT_LEVEL(ICR) + 24);
		} else {
			LOGMASKED(LOG_DEBUG, "%s: Interrupt & vector from %d: ICR: %02x, Vector: %02x\n", this->machine().describe_context(), interrupt, ICR, vector);
			set_input_line_and_vector(INT_LEVEL(ICR), HOLD_LINE, vector);
		}
		
	}
}

/* Reset status */
// MBAR + 0x040
uint8_t mcf5206e_device::RSR_r()
{
	LOGMASKED(LOG_DEBUG, "%s: (Reset Status Register) RSR_r %02x\n", this->machine().describe_context(), m_RSR);
	return m_RSR;
}

void mcf5206e_device::RSR_w(uint8_t data)
{
	m_RSR &= ~data;
	LOGMASKED(LOG_DEBUG, "%s: (Reset Status Register) IPR_w %02x\n", this->machine().describe_context(), data);
}

/* Watchdog */
// MBAR + 0x041
uint8_t mcf5206e_device::SYPCR_r(){
	LOGMASKED(LOG_SWDT, "%s: (System Protection Control) SYPCR_r %02x\n", this->machine().describe_context(), m_SYPCR);
	return m_SYPCR;
}

void mcf5206e_device::SYPCR_w(uint8_t data){
	if(!m_sypcr_locked){
		// SYPCR is a write-once register, whatever is written first remains until system reset.
		LOGMASKED(LOG_SWDT, "%s: (System Protection Control) SYPCR_w %02x\n", this->machine().describe_context(), data);
		m_sypcr_locked = true;
		m_SYPCR = data;

		// Bus monitoring is not supported (nor will it be?)

		if(BIT(m_SYPCR, 6)){
			m_swdt->watchdog_enable(BIT(data, 7));
		} else {
			// Set timer for interrupt
		}
	} else {
		LOG("%s: Write to SYPCR_w (%02x) when PCR is locked\n", this->machine().describe_context(), data);
	}
}

// MBAR + 0x042
uint8_t mcf5206e_device::SWIVR_r(){
	LOGMASKED(LOG_SWDT, "%s: (Software Watchdog Interrupt Vector) SWIVR_r %02x\n", this->machine().describe_context(), m_SWIVR);
	return m_SWIVR;
}

void mcf5206e_device::SWIVR_w(uint8_t data){
	LOGMASKED(LOG_SWDT, "%s: (Software Watchdog Interrupt Vector) SWIVR_w %02x\n", this->machine().describe_context(), data);
	m_SWIVR = data;
}

// MBAR + 0x043
void mcf5206e_device::SWSR_w(uint8_t data){
	LOGMASKED(LOG_SWDT, "%s: (Software Watchdog Service Routine) SWIVR_r %02x\n", this->machine().describe_context(), data);
	if(data == swdt_reset_sequence[m_swdt_w_count]) m_swdt_w_count++;
	if(m_swdt_w_count == 2) {
		m_swdt->watchdog_reset();
		m_swdt_w_count = 0;
	}
}

TIMER_CALLBACK_MEMBER(mcf5206e_device::swdt_callback)
{
	set_interrupt(WDINT);
}


/* Chip Select Module */

inline uint16_t mcf5206e_device::CSARx_r(offs_t offset)
{
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Address Register) CSAR%d_r\n", this->machine().describe_context(), offset);
	return m_CSAR[offset];
}

inline void mcf5206e_device::CSARx_w(offs_t offset, uint16_t data)
{
	m_CSAR[offset] = data;
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Address Register) CSAR%d_w %04x\n", this->machine().describe_context(), offset, data);
}

inline uint32_t mcf5206e_device::CSMRx_r(offs_t offset)
{
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Mask Register) CSMR%d_r\n", this->machine().describe_context(), offset);
	return m_CSMR[offset];
}

inline void mcf5206e_device::CSMRx_w(offs_t offset, uint32_t data)
{
	m_CSMR[offset] = data;
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Mask Register) CSMR%d_w %08x\n", this->machine().describe_context(), offset, data);
}

inline uint16_t mcf5206e_device::CSCRx_r(offs_t offset)
{	
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Control Register) CSCR%d_r\n", this->machine().describe_context(), offset);
	return m_CSCR[offset];
}

inline void mcf5206e_device::CSCRx_w(offs_t offset, uint16_t data)
{
	m_CSCR[offset] = data;
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Control Register) CSCR%d_w %04x\n", this->machine().describe_context(), offset, data);
}

uint16_t mcf5206e_device::DMCR_r()
{
	LOGMASKED(LOG_DEBUG, "%s: (Default Memory Control Register) DMCR_r %04x\n", this->machine().describe_context(), m_DMCR);
	return m_DMCR;
}

void mcf5206e_device::DMCR_w(uint16_t data)
{
	m_DMCR = data;
	LOGMASKED(LOG_DEBUG, "%s: (Default Memory Control Register) DMCR_w %04x\n", this->machine().describe_context(), data);
}


uint16_t mcf5206e_device::PAR_r()
{
	LOGMASKED(LOG_DEBUG, "%s: (Pin Assignment Register) PAR_r %04x\n", this->machine().describe_context(), m_PAR);
	return m_PAR;
}

void mcf5206e_device::PAR_w(uint16_t data)
{
	m_PAR = data;
	LOGMASKED(LOG_DEBUG, "%s: (Pin Assignment Register) PAR_w %04x\n", this->machine().describe_context(), data);
}


/* Parallel port */

uint8_t mcf5206e_device::PPDDR_r()
{
	LOGMASKED(LOG_DEBUG, "%s: (Port A Data Direction Register) PPDDR_r\n", this->machine().describe_context());
	return m_PPDDR;
}

void mcf5206e_device::PPDDR_w(uint8_t data)
{
	LOGMASKED(LOG_DEBUG, "%s: (Port A Data Direction Register) PPDDR_w %02x\n", this->machine().describe_context(), data);

	if(m_PPDDR != data){
		// Updating the register updates the pins immediately
		uint8_t mask = 0;
		if(!(m_PAR & PAR4)) mask |= 0x0F;	// PP 0-3 / DDATA 0-3
		if(!(m_PAR & PAR5)) mask |= 0xF0;	// PP 4-7 / PST 0-3

		m_gpio_w_cb(m_PPDATO, (m_PPDDR & mask));
	}

	m_PPDDR = data;
}

uint8_t mcf5206e_device::PPDAT_r()
{
	LOGMASKED(LOG_DEBUG, "%s: (Port A Data Register) PPDAT_r\n", this->machine().describe_context());
	m_PPDATI = (m_gpio_r_cb() | (m_PPDATO & m_PPDDR));	// Return the inputs and current outputs
	return m_PPDATI;
}

void mcf5206e_device::PPDAT_w(uint8_t data)
{
	LOGMASKED(LOG_DEBUG, "%s: (Port A Data Register) PPDAT_w %02x\n", this->machine().describe_context(), data);
	m_PPDATO = data;

	uint8_t mask = 0;
	if(!(m_PAR & PAR4)) mask |= 0x0F;	// PP 0-3 / DDATA 0-3
	if(!(m_PAR & PAR5)) mask |= 0xF0;	// PP 4-7 / PST 0-3

	m_gpio_w_cb(m_PPDATO, (m_PPDDR & mask));
}


/* MBUS */

TIMER_CALLBACK_MEMBER(mcf5206e_device::mbus_callback){

}

uint8_t mcf5206e_device::MADR_r()
{
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Control Register) MADR_r: %02x\n", this->machine().describe_context(), m_MADR);
	return m_MADR;
}

void mcf5206e_device::MADR_w(uint8_t data)
{
	m_MADR = (data & 0xFE);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Control Register) MADR_w: %02x\n", this->machine().describe_context(), data);
}

uint8_t mcf5206e_device::MFDR_r()
{
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Frequency Divider Register) MFDR_r: %02x\n", this->machine().describe_context(), m_MFDR);
	return m_MFDR;
}

void mcf5206e_device::MFDR_w(uint8_t data)
{
	m_MFDR = (data & 0x3F);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Frequency Divider Register) MFDR_w: %02x\n", this->machine().describe_context(), data);
}

uint8_t mcf5206e_device::MBCR_r()
{
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Control Register) MBCR_r: %02x\n", this->machine().describe_context(), m_MBCR);
	return m_MBCR;
}

void mcf5206e_device::MBCR_w(uint8_t data)
{
	m_MBCR = (data & 0xFC);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Control Register) MBCR_w: %02x\n", this->machine().describe_context(), data);
}

uint8_t mcf5206e_device::MBSR_r()
{
	int hack = 0x00;

	hack ^= (machine().rand()&0xff);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Status Register) MBSR_r: %02x\n", this->machine().describe_context(), m_MBSR);
	return m_MBSR ^ hack; // will loop on this after a while
}

void mcf5206e_device::MBSR_w(uint8_t data)
{
	m_MBSR = (data & 0x14);	// MAL & MIF
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Status Register) MBSR_w: %02x\n", this->machine().describe_context(), data);
}

uint8_t mcf5206e_device::MBDR_r()
{
	int hack = 0x00;
	hack ^= (machine().rand()&0xff);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Data I/O Register) MBDR_r: %02x\n", this->machine().describe_context(), m_MBDR);
	return m_MBDR ^ hack;
}

void mcf5206e_device::MBDR_w(uint8_t data)
{
	m_MBDR = data;
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Data I/O Register) MBDR_w: %02x\n", this->machine().describe_context(), data);
}

/* Timer 1 */

TIMER_CALLBACK_MEMBER(mcf5206e_device::timer1_callback)
{
	if(m_TMR1 & T_RST){
		//LOGMASKED(LOG_DEBUG, "timer1_callback\n");
		if ((m_TMR1 & T_CL0) || (m_TMR1 & T_CL0)){
			// Counter is running
			m_TCN1++;
		}
		
		if(m_TCN1 == m_TRR1 && !(m_TER1 & T_EREF)){
			if(m_TMR1& T_FRR){
				// FRR resets counter to 0
				m_TCN1 = 0;
			}
			m_TER1 |= T_EREF;

			// technically we should do the vector check in the IRQ callback as well as various checks based on the IRQ masks before asserting the interrupt
			set_interrupt(TINT1);
		}
	}
}

uint16_t mcf5206e_device::TMR1_r()
{
	LOGMASKED(LOG_TIMER, "%s: (Timer 1 Mode Register) TMR1_r: %04x\n", this->machine().describe_context(), m_TMR1);
	return m_TMR1;
}

void mcf5206e_device::TMR1_w(uint16_t data)
{
	uint16_t cmd = data;

	if((m_TMR1 & T_RST) && !(cmd & T_RST)){
		// T_RST High to low resets the entire timer
		m_TMR1 = 0;
		m_TRR1 = 0xFFFF;
		m_TCN1 = 0;
		m_TCR1 = 0;
		m_TER1 = 0;
		m_timer1->adjust(attotime::never);
		return;
	}

	m_TMR1 = cmd;

	if (m_TMR1 & T_RST){
		// TODO: Add TIN pin support
		int div;
		div = ((m_TMR1 & 0xFF00) >> 8) + 1; // 1 -> 256 division scale
		if ((m_TMR1 & T_CL1) && !(m_TMR1 & T_CL0)) div = div * 16;	// Input clock / 16
		m_timer1->adjust(clocks_to_attotime(div), 0, clocks_to_attotime(div));
	}

	LOGMASKED(LOG_TIMER, "%s: (Timer 1 Mode Register) TMR1_w: %04x\n", this->machine().describe_context(), data);
	LOGMASKED(LOG_TIMER, " (Prescale) PS : %02x  (Capture Edge/Interrupt) CE : %01x (Output Mode) OM : %01x  (Output Reference Interrupt En) ORI : %01x   Free Run (FRR) : %01x  Input Clock Source (ICLK) : %01x  (Reset Timer) RST : %01x  \n", (m_TMR1 & 0xff00)>>8, (m_TMR1 & 0x00c0)>>6,  (m_TMR1 & 0x0020)>>5, (m_TMR1 & 0x0010)>>4, (m_TMR1 & 0x0008)>>3, (m_TMR1 & 0x0006)>>1, (m_TMR1 & 0x0001)>>0);
}

uint16_t mcf5206e_device::TRR1_r()
{
	LOGMASKED(LOG_TIMER, "%s: (Timer 1 Reference Register) TRR1_r: %04x\n", this->machine().describe_context(), m_TRR1);
	return m_TRR1;
}

void mcf5206e_device::TRR1_w(uint16_t data)
{
	m_TRR1 = data;
	LOGMASKED(LOG_TIMER, "%s: (Timer 1 Reference Register) TRR1_w: %04x\n", this->machine().describe_context(), data);
}

uint16_t mcf5206e_device::TCN1_r()
{
	LOGMASKED(LOG_TIMER, "%s: (Timer 1 Counter) TCN1_r: %04x\n", this->machine().describe_context(), m_TCN1);
	return m_TCN1;
}

void mcf5206e_device::TCN1_w(uint16_t data)
{
	// Writing any value resets the counter
	m_TCN1 = 0;
	LOGMASKED(LOG_TIMER, "%s: (Timer 1 Counter Reset) TCN1_w: %04x\n", this->machine().describe_context(), data);
}

uint16_t mcf5206e_device::TCR1_r(){
	// Read only
	LOGMASKED(LOG_TIMER, "%s: (Timer 1 Capture) TCR1_r %04x\n", this->machine().describe_context(), m_TCR1);
	return m_TCR1;
}

uint8_t mcf5206e_device::TER1_r()
{
	LOGMASKED(LOG_TIMER, "%s: (Timer 1 Event) TER1_r: %02x\n", this->machine().describe_context(), m_TER1);
	return m_TER1; // set on timer events, cleared by writing below
}

void mcf5206e_device::TER1_w(uint8_t data)
{
	m_TER1 &= ~data; // Programmer must write bit to clear it. IE write 0x80 to clear bit 7.
	LOGMASKED(LOG_TIMER, "%s: (Timer 1 Event) TER1_w: %02x\n", this->machine().describe_context(), data);
}


/* Timer 2 */

TIMER_CALLBACK_MEMBER(mcf5206e_device::timer2_callback)
{
	if(m_TMR2 & T_RST){
		//LOGMASKED(LOG_TIMER, "timer2_callback\n");
		if ((m_TMR2 & T_CL0) || (m_TMR2 & T_CL0)){
			// Counter is running
			m_TCN2++;
		}
		
		if(m_TCN2 == m_TRR2 && !(m_TER2 & T_EREF)){
			if(m_TMR2 & T_FRR){
				// FRR resets counter to 0
				m_TCN2 = 0;
			}
			m_TER2 |= T_EREF;
			set_interrupt(TINT2);
		}
	}
}

uint16_t mcf5206e_device::TMR2_r()
{
	LOGMASKED(LOG_TIMER, "%s: (Timer 2 Mode Register) TMR2_r: %04x\n", this->machine().describe_context(), m_TMR2);
	return m_TMR2;
}

void mcf5206e_device::TMR2_w(uint16_t data)
{
	uint16_t cmd = data;

	if((m_TMR2 & T_RST) && !(cmd & T_RST)){
		// T_RST High to low resets the entire timer
		m_TMR2 = 0;
		m_TRR2 = 0xFFFF;
		m_TCN2 = 0;
		m_TCR2 = 0;
		m_TER2 = 0;
		m_timer2->adjust(attotime::never);
		return;
	}

	m_TMR2 = cmd;

	if (m_TMR2 & T_RST){
		// TODO: Add TIN pin support
		int div;
		div = ((m_TMR2 & 0xFF00) >> 8) + 1; // 1 -> 256 division scale
		if ((m_TMR2 & T_CL1) && !(m_TMR2 & T_CL0)) div = div * 16;	// Input clock / 16
		m_timer2->adjust(clocks_to_attotime(div), 0, clocks_to_attotime(div));
	}

	LOGMASKED(LOG_TIMER, "%s: (Timer 2 Mode Register) TMR2_w: %04x\n", this->machine().describe_context(), data);
	LOGMASKED(LOG_TIMER, " (Prescale) PS : %02x  (Capture Edge/Interrupt) CE : %01x (Output Mode) OM : %01x  (Output Reference Interrupt En) ORI : %01x   Free Run (FRR) : %01x  Input Clock Source (ICLK) : %01x  (Reset Timer) RST : %01x  \n", (m_TMR1 & 0xff00)>>8, (m_TMR1 & 0x00c0)>>6,  (m_TMR1 & 0x0020)>>5, (m_TMR1 & 0x0010)>>4, (m_TMR1 & 0x0008)>>3, (m_TMR1 & 0x0006)>>1, (m_TMR1 & 0x0001)>>0);
}

uint16_t mcf5206e_device::TRR2_r()
{
	LOGMASKED(LOG_TIMER, "%s: (Timer 2 Reference Register) TRR2_r: %04x\n", this->machine().describe_context(), m_TRR2);
	return m_TRR2;
}

void mcf5206e_device::TRR2_w(uint16_t data)
{
	m_TRR2 = data;
	LOGMASKED(LOG_TIMER, "%s: (Timer 2 Reference Register) TRR2_w: %04x\n", this->machine().describe_context(), data);
}

uint16_t mcf5206e_device::TCN2_r()
{
	LOGMASKED(LOG_TIMER, "%s: (Timer 2 Counter) TCN2_r: %04x\n", this->machine().describe_context(), m_TCN2);
	return m_TCN2;
}

void mcf5206e_device::TCN2_w(uint16_t data)
{
	// Writing any value resets counter
	m_TCN2 = 0;
	LOGMASKED(LOG_TIMER, "%s: (Timer 2 Counter Reset) TCN2_w: %04x\n", this->machine().describe_context(), data);
}

uint16_t mcf5206e_device::TCR2_r(){
	// Read only
	LOGMASKED(LOG_TIMER, "%s: (Timer 2 Capture) TCR2_r: %04x\n", this->machine().describe_context(), m_TCR2);
	return m_TCR2;
}

uint8_t mcf5206e_device::TER2_r()
{
	LOGMASKED(LOG_TIMER, "%s: (Timer 2 Event) TER2_r: %02x\n", this->machine().describe_context(), m_TER2);
	return m_TER2; // set on timer events, cleared by writing below
}

void mcf5206e_device::TER2_w(uint8_t data)
{
	m_TER2 &= ~data; // Programmer must write bit to clear it. IE write 0x80 to clear bit 7.
	LOGMASKED(LOG_TIMER, "%s: (Timer 2 Event) TER2_w: %02x\n", this->machine().describe_context(), data);
}

/* UART */
uint8_t mcf5206e_device::uart1_r(offs_t offset) { 
	return m_uart[0]->read(offset); 
}

uint8_t mcf5206e_device::uart2_r(offs_t offset) { 
	return m_uart[1]->read(offset); 
}

void mcf5206e_device::uart1_w(offs_t offset, uint8_t data) { 
	m_uart[0]->write(offset, data); 
}

void mcf5206e_device::uart2_w(offs_t offset, uint8_t data) { 
	m_uart[1]->write(offset, data);
}

void mcf5206e_device::uart1_irq_w(int state){
	if(BIT(m_IMR, UINT1) && state){
		LOGMASKED(LOG_UART, "%s: UART1 Interrupt (%u)\n", this->machine().describe_context(), state);
	}
	set_interrupt(UINT1);
}

void mcf5206e_device::uart2_irq_w(int state){
	if(BIT(m_IMR, UINT2) && state){
		LOGMASKED(LOG_UART, "%s: UART2 Interrupt (%u)\n", this->machine().describe_context(), state);
	}
	set_interrupt(UINT2);
}

#define UNINIT 0
#define UNINIT_NOTE 0

void mcf5206e_device::init_regs(bool first_init)
{
	m_SIMR = 0xc0;
	m_MARB = 0x00;

	m_DCRR = 0x0000;
	m_DCTR = 0x0000; 
	m_DCAR0 = UNINIT;
	m_DCMR0 = UNINIT;
	m_DCCR0 = 0x00;
	m_DCAR1 = UNINIT;
	m_DCMR1 = UNINIT;
	m_DCCR1 = 0x00;

	m_CSAR[0] = 0x0000;
	m_CSMR[0] = 0x00000000;
	m_CSCR[0] =   0x3C1F; /* 3C1F, 3C5F, 3C9F, 3CDF, 3D1F, 3D5F, 3D9F, 3DDF |  AA set by IRQ 7 at reset, PS1 set by IRQ 4 at reset, PS0 set by IRQ 1 at reset*/

	if(!(m_RSR & 0x20)) m_RSR = 0x00;	// Don't clear if watchdog triggered

	if (first_init)
	{
		for (int x=1;x<8;x++)
		{
			m_CSAR[1] = UNINIT;
			m_CSMR[1] = UNINIT;
			m_CSCR[1] = UNINIT_NOTE; // except BRST=ASET=WRAH=RDAH=WR=RD=0
		}
		m_RSR = 0x70;
	}

	m_DMCR = 0x0000;
	m_PAR = 0x0000;
	m_ICR[ICR1] =   0x04;
	m_ICR[ICR2] =   0x08;
	m_ICR[ICR3] =   0x0C;
	m_ICR[ICR4] =   0x10;
	m_ICR[ICR5] =   0x14;
	m_ICR[ICR6] =   0x18;
	m_ICR[ICR7] =   0x1C;
	m_ICR[ICR_SWDT] =   0x1C;
	m_ICR[ICR_TMR1] =   0x80;
	m_ICR[ICR_TMR2] =  0x80;
	m_ICR[ICR_MBUS] =  0x80;
	m_ICR[ICR_UART1] =  0x00;
	m_ICR[ICR_UART2] =  0x00;
	m_ICR[ICR_DMA0] =  0x00;
	m_ICR[ICR_DMA1] =  0x00;
	m_IPR = 0x3FFE;
	m_SYPCR	= 0x00;
	m_sypcr_locked = false;
	m_SWIVR = 0x0F;

	m_TMR1 = 0x0000;
	m_TRR1 = 0xffff;
	m_TCN1 = 0x0000;
	m_TCR1 = 0x0000;
	m_TER1 = 0x00;

	m_TMR2 = 0x0000;
	m_TRR2 = 0xffff;
	m_TCN2 = 0x0000;
	m_TCR2 = 0x0000;
	m_TER2 = 0x00;

	m_PPDDR = 0x00;
	m_PPDATI = 0x00;
	m_PPDATO = 0x00;

	m_IMR = 0x3FFE;
	m_IPR = 0x0000;

	m_MADR = 0x00;
	m_MFDR = 0x00;
	m_MBCR = 0x00;
	m_MBSR = 0x81;
	m_MBDR = 0x00;

	m_swdt_w_count = 0;
}

/*

ADDRESS (LE)            REG         WIDTH   NAME/DESCRIPTION                                    INIT VALUE (MR=Master Reset, NR=Normal Reset)       Read or Write access
* = inited
- = skeleton handler

op MOVEC with $C0F      MBAR        32      Module Base Address Register                        uninit (except V=0)                                 W
$003 √                  SIMR        8       SIM Configuration Register                          C0                                                  R/W
$014*-                  ICR1        8       Interrupt Control Register 1 - External IRQ1/IPL1   04                                                  R/W
$015*-                  ICR2        8       Interrupt Control Register 2 - External IPL2        08                                                  R/W
$016*-                  ICR3        8       Interrupt Control Register 3 - External IPL3        0C                                                  R/W
$017*-                  ICR4        8       Interrupt Control Register 4 - External IRQ4/IPL4   10                                                  R/W
$018*                   ICR5        8       Interrupt Control Register 5 - External IPL5        14                                                  R/W
$019*                   ICR6        8       Interrupt Control Register 6 - External IPL6        18                                                  R/W
$01A*                   ICR7        8       Interrupt Control Register 7 - External IRQ7/IPL7   1C                                                  R/W
$01B*                   ICR8        8       Interrupt Control Register 8 - SWT                  1C                                                  R/W
$01C*-                  ICR9        8       Interrupt Control Register 9 - Timer 1 Interrupt    80                                                  R/W
$01D*-                  ICR10       8       Interrupt Control Register 10 - Timer 2 Interrupt   80                                                  R/W
$01E*-                  ICR11       8       Interrupt Control Register 11 - MBUS Interrupt      80                                                  R/W
$01F*-                  ICR12       8       Interrupt Control Register 12 - UART 1 Interrupt    00                                                  R/W
$020*-                  ICR13       8       Interrupt Control Register 13 - UART 2 Interrupt    00                                                  R/W
$020*-                  ICR14       8       Interrupt Control Register 14 - DMA 0  Interrupt    00                                                  R/W
$020*-                  ICR15       8       Interrupt Control Register 15 - DMA 1  Interrupt    00                                                  R/W
$036*-                  IMR         16      Interrupt Mask Register                             3FFE                                                R/W
$03A                    IPR         16      Interrupt Pending Register                          0000                                                R
$040                    RSR         8       Reset Status Register                               80 / 20                                             R/W
$041                    SYPCR       8       System Protection Control Register                  00                                                  R/W
$042                    SWIVR       8       Software Watchdog Interrupt Vector Register         0F                                                  R/W
$043                    SWSR        8       Software Watchdog Service Register                  uninit                                              W
$046                    DCRR        16      DRAM Controller Refresh                             MR 0000   - NR uninit                               R/W
$04A                    DCTR        16      DRAM Controller Timing Register                     MR 0000   - NR uninit                               R/W
$04C                    DCAR0       16      DRAM Controller 0 Address Register                  MR uninit - NR uninit                               R/W
$050                    DCMR0       32      DRAM Controller 0 Mask Register                     MR uninit - NR uninit                               R/W
$057                    DCCR0       8       DRAM Controller 0 Control Register                  MR 00     - NR 00                                   R/W
$058                    DCAR1       16      DRAM Controller 1 Address Register                  MR uninit - NR uninit                               R/W
$05C                    DCMR1       32      DRAM Controller 1 Mask Register                     MR uninit - NR uninit                               R/W
$063                    DCCR1       8       DRAM Controller 1 Control Register                  MR 00     - NR 00                                   R/W
--------- CHIP SELECTS -----------
$064*-                  CSAR0       16      Chip-Select 0 Address Register                      0000                                                R/W
$068*-                  CSMR0       32      Chip-Select 0 Mask Register                         00000000                                            R/W
$06E*-                  CSCR0       16      Chip-Select 0 Control Register                      3C1F, 3C5F, 3C9F, 3CDF, 3D1F, 3D5F, 3D9F, 3DDF      R/W
                                                                                                AA set by IRQ 7 at reset
                                                                                                PS1 set by IRQ 4 at reset
                                                                                                PS0 set by IRQ 1 at reset
$070*-                  CSAR1       16      Chip-Select 1 Address Register                      uninit                                              R/W
$074*-                  CSMR1       32      Chip-Select 1 Mask Register                         uninit                                              R/W
$07A*-                  CSCR1       16      Chip-Select 1 Control Register                      uninit *1                                           R/W
$07C*-                  CSAR2       16      Chip-Select 2 Address Register                      uninit                                              R/W
$080*-                  CSMR2       32      Chip-Select 2 Mask Register                         uninit                                              R/W
$086*-                  CSCR2       16      Chip-Select 2 Control Register                      uninit *1                                           R/W
$088*-                  CSAR3       16      Chip-Select 3 Address Register                      uninit                                              R/W
$08C*-                  CSMR3       32      Chip-Select 3 Mask Register                         uninit                                              R/W
$092*-                  CSCR3       16      Chip-Select 3 Control Register                      uninit *1                                           R/W
$094*-                  CSAR4       16      Chip-Select 4 Address Register                      uninit                                              R/W
$098*-                  CSMR4       32      Chip-Select 4 Mask Register                         uninit                                              R/W
$09E*-                  CSCR4       16      Chip-Select 4 Control Register                      uninit *1                                           R/W
$0A0*-                  CSAR5       16      Chip-Select 5 Address Register                      uninit                                              R/W
$0A4*-                  CSMR5       32      Chip-Select 5 Mask Register                         uninit                                              R/W
$0AA*-                  CSCR5       16      Chip-Select 5 Control Register                      uninit *1                                           R/W
$0AC*-                  CSAR6       16      Chip-Select 6 Address Register                      uninit                                              R/W
$0B0*-                  CSMR6       32      Chip-Select 6 Mask Register                         uninit                                              R/W
$0B6*-                  CSCR6       16      Chip-Select 6 Control Register                      uninit *1                                           R/W
$0B8*-                  CSAR7       16      Chip-Select 7 Address Register                      uninit                                              R/W
$0BC*-                  CSMR7       32      Chip-Select 7 Mask Register                         uninit                                              R/W
$0C2*-                  CSCR7       16      Chip-Select 7 Control Register                      uninit *1                                           R/W
$0C6*-                  DMCR        16      Default Memory Control Register                     0000                                                R/W
$0CA*-                  PAR         16      Pin Assignment Register                             00                                                  R/W
--------- TIMER MODULE -----------
$100 √                  TMR1        16      Timer 1 Mode Register                               0000                                                R/W
$104 √                  TRR1        16      Timer 1 Reference Register                          FFFF                                                R/W
$108 √*                 TCR1        16      Timer 1 Capture Register                            0000                                                R
$10C √                  TCN1        16      Timer 1 Counter                                     0000                                                R/W
$111 √                  TER1        8       Timer 1 Event Register                              00                                                  R/W
$120 √                  TMR2        16      Timer 2 Mode Register                               0000                                                R/W
$124 √                  TRR2        16      Timer 2 Reference Register                          FFFF                                                R/W
$128 √*                 TCR2        16      Timer 2 Capture Register                            0000                                                R
$12C √                  TCN2        16      Timer 2 Counter                                     0000                                                R/W
$131 √                  TER2        8       Timer 2 Event Register                              00                                                  R/W
------------ UART SERIAL PORTS  -----------
Using the mc68681 base device driver with the second port removed... not far removed from the actual implementation.
$140 √                  UMR1,2      8       UART 1 Mode Registers                               00                                                  R/W
$144 √                  USR         8       UART 1 Status Register                              00                                                  R
                        UCSR        8       UART 1 Clock-Select Register                        DD                                                  W
$148 √                  UCR         8       UART 1 Command Register                             00                                                  W
$14C √                  URB         8       UART 1 Receive Buffer                               FF                                                  R
                        UTB         8       UART 1 Transmit Buffer                              00                                                  W
$150 √                  UIPCR       8       UART Input Port Change Register                     0F                                                  R
                        UACR        8       UART 1 Auxilary Control Register                    00                                                  W
$154 √                  UISR        8       UART 1 Interrupt Status Register                    00                                                  R
                        UIMR        8       UART 1 Interrupt Mask Register                      00                                                  W
$158 √                  UBG1        8       UART 1 Baud Rate Generator Prescale MSB             uninit                                              W
$15C √                  UBG2        8       UART 1 Baud Rate Generator Prescale LSB             uninit                                              W
$170 √                  UIVR        8       UART 1 Interrupt Vector Register                    0F                                                  R/W
$174 √                  UIP         8       UART 1 Input Port Register                          FF                                                  R
$178 √                  UOP1        8       UART 1 Output Port Bit Set CMD                      UOP1[7-1]=undef; UOP1=0                             W
$17C √                  UOP0        8       UART 1 Output Port Bit Reset CMD                    uninit                                              W

$180 √                  UMR1,2      8       UART 2 Mode Registers                               00                                                  R/W
$184 √                  USR         8       UART 2 Status Register                              00                                                  R
                        UCSR        8       UART 2 Clock-Select Register                        DD                                                  W
$188 √                  UCR         8       UART 2 Command Register                             00                                                  W
$18C √                  URB         8       UART 2 Receive Buffer                               FF                                                  R
                        UTB         8       UART 2 Transmit Buffer                              00                                                  W
$190 √                  UIPCR       8       UART 2 Input Port Change Register                   0F                                                  R
                        UACR        8       UART 2 Auxilary Control Register                    00                                                  W
$194 √                  UISR        8       UART 2 Interrupt Status Register                    00                                                  R
                        UIMR        8       UART 2 Interrupt Mask Register                      00                                                  W
$198 √                  UBG1        8       UART 2 Baud Rate Generator Prescale MSB             uninit                                              R/W
$19C √                  UBG2        8       UART 2 Barud Rate Generator Prescale LSB            uninit                                              R/W
$1B0 √                  UIVR        8       UART 2 Interrupt Vector Register                    0F                                                  R/W
$1B4 √                  UIP         8       UART 2 Input Port Register                          FF                                                  R
$1B8 √                  UOP1        8       UART 2 Output Port Bit Set CMD                      UOP1[7-1]=undef; UOP1=0                             W
$1BC √                  UOP0        8       UART 2 Output Port Bit Reset CMD                    uninit                                              W
------------ GPIO -----------
$1C5 √                  PPDDR       8       Port A Data Direction Register                      00                                                  R/W
$1C9 √                  PPDAT       8       Port A Data Register                                00                                                  R/W
------------ MBUS  -----------
$1E0                    MADR        8       M-Bus Address Register                              00                                                  R/W
$1E4*-                  MFDR        8       M-Bus Frequency Divider Register                    00                                                  R/W
$1E8*-                  MBCR        8       M-Bus Control Register                              00                                                  R/W
$1EC*-                  MBSR        8       M-Bus Status Register                               00                                                  R/W
$1F0*-                  MBDR        8       M-Bus Data I/O Register                             00                                                  R/W
------------ DMA Controller -----------
$200                    DMASAR0     32      Source Address Register 0                           00                                                  R/W
$204                    DMADAR0     32      Destination Address Register 0                      00                                                  R/W
$208                    DCR0        16      DMA Control Register 0                              00                                                  R/W
$20C                    BCR0        16      Byte Count Register 0                               00                                                  R/W
$210                    DSR0        8       Status Register 0                                   00                                                  R/W
$214                    DIVR0       8       Interrupt Vector Register 0                         0F                                                  R/W
$240                    DMASAR1     32      Source Address Register 1                           00                                                  R/W
$244                    DMADAR1     32      Destination Address Register 1                      00                                                  R/W
$248                    DCR1        16      DMA Control Register 1                              00                                                  R/W
$24C                    BCR1        16      Byte Count Register 1                               00                                                  R/W
$250                    DSR1        8       Status Register 1                                   00                                                  R/W
$254                    DIVR1       8       Interrupt Vector Register 1                         0F                                                  R/W

*1 - uninit except BRST=ASET=WRAH=RDAH=WR=RD=0

*/
