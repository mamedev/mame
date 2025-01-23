// license:BSD-3-Clause
// copyright-holders: Karl Stenerud / NaokiS

/*
	Notes:
		Chip Select Module:
			* Not currently implemented, quite possible it doesn't need to be added for any functional reason aside from completeness/debugging
		System Integration Module:
			* MBAR: should be set up in m68000_musashi_device::x4e7a_movec_l_c() to move the ColdFire IO register map according to the contents of MBAR
						could store m_mbar in m68000_musashi_device and move the address_space_map to the given offset?
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

		* Some of these modules can probably be split back into the /devices/machine device, namely the MBUS and timer modules, possibly SIM too.
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
#define LOG_SIM	    	(1U << 8)
#define VERBOSE 		( LOG_DEBUG | LOG_UART | LOG_SWDT | LOG_MBUS | LOG_DRAM | LOG_SIM )
#include "logmacro.h"

#define INT_LEVEL(N) 	((N&0x1c) >> 2)
#define ICR_USE_AUTOVEC(N)	((N & 0x80) != 0)

#define BITWRITE(x, n, s) ((x & ~(1 << n)) | (s << n))
#define BITSET(x, n) BITWRITE(x, n, 1)
#define BITCLEAR(x, n) BITWRITE(x, n, 0)

DEFINE_DEVICE_TYPE(MCF5206E,    mcf5206e_device,    "mcf5206e",     "Freescale MCF5206E")
DEFINE_DEVICE_TYPE(COLDFIRE_SIM,    coldfire_sim_device,    "coldfire_sim",     "ColdFire SIM Module")
DEFINE_DEVICE_TYPE(COLDFIRE_MBUS,    coldfire_mbus_device,    "coldfire_mbus",     "ColdFire MBUS Module")
DEFINE_DEVICE_TYPE(COLDFIRE_TIMER,    coldfire_timer_device,    "coldfire_timer",     "ColdFire Timer Module")

std::unique_ptr<util::disasm_interface> mcf5206e_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_COLDFIRE);
}

mcf5206e_device::mcf5206e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, MCF5206E, 32, 32, address_map_constructor(FUNC(mcf5206e_device::coldfire_regs_map), this))
	, m_sim(*this, "sim")
	, m_timer(*this, "timer%u", 1U)
	, write_chip_select(*this)
	, m_uart(*this, "coldfire_uart%u", 0U)
	, write_tx1(*this)
	, write_tx2(*this)
	, m_gpio_w_cb(*this)
	, m_mbus(*this, "coldfire_mbus")
	, write_sda(*this)
	, write_scl(*this)
{
}

void mcf5206e_device::device_add_mconfig(machine_config &config){
	COLDFIRE_SIM(config, m_sim, this->clock(), this->tag());

	COLDFIRE_TIMER(config, m_timer[0], (this->clock()));
	m_timer[0]->irq_cb().set(FUNC(mcf5206e_device::timer_1_irq));
	COLDFIRE_TIMER(config, m_timer[1], (this->clock()));
	m_timer[1]->irq_cb().set(FUNC(mcf5206e_device::timer_2_irq));

	MCF5206E_UART(config, m_uart[0], (this->clock() / 32));
	MCF5206E_UART(config, m_uart[1], (this->clock() / 32));
	m_uart[0]->irq_cb().set(FUNC(mcf5206e_device::uart_1_irq));
	m_uart[1]->irq_cb().set(FUNC(mcf5206e_device::uart_2_irq));

	COLDFIRE_MBUS(config, m_mbus, this->clock());
	m_mbus->sda_cb().set(FUNC(mcf5206e_device::mbus_sda_w));
	m_mbus->scl_cb().set(FUNC(mcf5206e_device::mbus_scl_w));
	m_mbus->irq_cb().set(FUNC(mcf5206e_device::mbus_irq_w));
}

void mcf5206e_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_coldfire();

	init_regs(true);

	save_item(NAME(m_dcrr));
	save_item(NAME(m_dctr));
	save_item(NAME(m_dcar0));
	save_item(NAME(m_dcmr0));
	save_item(NAME(m_dccr0));
	save_item(NAME(m_dcar1));
	save_item(NAME(m_dcmr1));
	save_item(NAME(m_dccr1));

	save_item(NAME(m_csar));
	save_item(NAME(m_csmr));
	save_item(NAME(m_cscr));
	save_item(NAME(m_dmcr));

	save_item(NAME(m_ppddr));
	save_item(NAME(m_ppdat_in));
	save_item(NAME(m_ppdat_out));
}

void mcf5206e_device::device_reset()
{
	m68000_musashi_device::device_reset();
	m_timer[0]->reset();
	m_timer[1]->reset();
	m_uart[0]->reset();
	m_uart[1]->reset();
	m_mbus->reset();

	init_regs(false);
}

// should be set up in m68000_musashi_device::x4e7a_movec_l_c() to move this map according to the contents of MBAR
void mcf5206e_device::coldfire_regs_map(address_map &map)
{
	/* SIM Module */
	map(0xf0000000, 0xf00000cf).m(m_sim, FUNC(coldfire_sim_device::sim_map));

	/* dram controller */
	map(0xf0000046, 0xf0000047).rw(FUNC(mcf5206e_device::dcrr_r), FUNC(mcf5206e_device::dcrr_w));
	map(0xf000004a, 0xf000004b).rw(FUNC(mcf5206e_device::dctr_r), FUNC(mcf5206e_device::dctr_w));
	map(0xf000004c, 0xf000004d).rw(FUNC(mcf5206e_device::dcar0_r), FUNC(mcf5206e_device::dcar0_w));
	map(0xf0000050, 0xf0000053).rw(FUNC(mcf5206e_device::dcmr0_r), FUNC(mcf5206e_device::dcmr0_w));
	map(0xf0000057, 0xf0000057).rw(FUNC(mcf5206e_device::dccr0_r), FUNC(mcf5206e_device::dccr0_w));
	map(0xf0000058, 0xf0000059).rw(FUNC(mcf5206e_device::dcar1_r), FUNC(mcf5206e_device::dcar1_w));
	map(0xf000005c, 0xf000005f).rw(FUNC(mcf5206e_device::dcmr1_r), FUNC(mcf5206e_device::dcmr1_w));
	map(0xf0000063, 0xf0000063).rw(FUNC(mcf5206e_device::dccr1_r), FUNC(mcf5206e_device::dccr1_w));

	/* chip select registers */
	map(0xf0000064, 0xf0000067).rw(FUNC(mcf5206e_device::csar0_r), FUNC(mcf5206e_device::csar0_w));
	map(0xf0000068, 0xf000006b).rw(FUNC(mcf5206e_device::csmr0_r), FUNC(mcf5206e_device::csmr0_w));
	map(0xf000006c, 0xf000006f).rw(FUNC(mcf5206e_device::cscr0_r), FUNC(mcf5206e_device::cscr0_w));
	map(0xf0000070, 0xf0000073).rw(FUNC(mcf5206e_device::csar1_r), FUNC(mcf5206e_device::csar1_w));
	map(0xf0000074, 0xf0000077).rw(FUNC(mcf5206e_device::csmr1_r), FUNC(mcf5206e_device::csmr1_w));
	map(0xf0000078, 0xf000007b).rw(FUNC(mcf5206e_device::cscr1_r), FUNC(mcf5206e_device::cscr1_w));
	map(0xf000007c, 0xf000007f).rw(FUNC(mcf5206e_device::csar2_r), FUNC(mcf5206e_device::csar2_w));
	map(0xf0000080, 0xf0000083).rw(FUNC(mcf5206e_device::csmr2_r), FUNC(mcf5206e_device::csmr2_w));
	map(0xf0000084, 0xf0000087).rw(FUNC(mcf5206e_device::cscr2_r), FUNC(mcf5206e_device::cscr2_w));
	map(0xf0000088, 0xf000008b).rw(FUNC(mcf5206e_device::csar3_r), FUNC(mcf5206e_device::csar3_w));
	map(0xf000008c, 0xf000008f).rw(FUNC(mcf5206e_device::csmr3_r), FUNC(mcf5206e_device::csmr3_w));
	map(0xf0000090, 0xf0000093).rw(FUNC(mcf5206e_device::cscr3_r), FUNC(mcf5206e_device::cscr3_w));
	map(0xf0000094, 0xf0000097).rw(FUNC(mcf5206e_device::csar4_r), FUNC(mcf5206e_device::csar4_w));
	map(0xf0000098, 0xf000009b).rw(FUNC(mcf5206e_device::csmr4_r), FUNC(mcf5206e_device::csmr4_w));
	map(0xf000009c, 0xf000009f).rw(FUNC(mcf5206e_device::cscr4_r), FUNC(mcf5206e_device::cscr4_w));
	map(0xf00000a0, 0xf00000a3).rw(FUNC(mcf5206e_device::csar5_r), FUNC(mcf5206e_device::csar5_w));
	map(0xf00000a4, 0xf00000a7).rw(FUNC(mcf5206e_device::csmr5_r), FUNC(mcf5206e_device::csmr5_w));
	map(0xf00000a8, 0xf00000ab).rw(FUNC(mcf5206e_device::cscr5_r), FUNC(mcf5206e_device::cscr5_w));
	map(0xf00000ac, 0xf00000af).rw(FUNC(mcf5206e_device::csar6_r), FUNC(mcf5206e_device::csar6_w));
	map(0xf00000b0, 0xf00000b3).rw(FUNC(mcf5206e_device::csmr6_r), FUNC(mcf5206e_device::csmr6_w));
	map(0xf00000b4, 0xf00000b7).rw(FUNC(mcf5206e_device::cscr6_r), FUNC(mcf5206e_device::cscr6_w));
	map(0xf00000b8, 0xf00000bb).rw(FUNC(mcf5206e_device::csar7_r), FUNC(mcf5206e_device::csar7_w));
	map(0xf00000bc, 0xf00000bf).rw(FUNC(mcf5206e_device::csmr7_r), FUNC(mcf5206e_device::csmr7_w));
	map(0xf00000c0, 0xf00000c3).rw(FUNC(mcf5206e_device::cscr7_r), FUNC(mcf5206e_device::cscr7_w));
	map(0xf00000c4, 0xf00000c7).rw(FUNC(mcf5206e_device::dmcr_r), FUNC(mcf5206e_device::dmcr_w));

	// timer 1
	map(0xf0000100, 0xf000011f).m(m_timer[0], FUNC(coldfire_timer_device::timer_map));
	map(0xf0000120, 0xf000013f).m(m_timer[1], FUNC(coldfire_timer_device::timer_map));

	// uart (mc68681 derrived)
	map(0xf0000140, 0xf000017c).rw(FUNC(mcf5206e_device::uart1_r), FUNC(mcf5206e_device::uart1_w));
	map(0xf0000180, 0xf00001bc).rw(FUNC(mcf5206e_device::uart2_r), FUNC(mcf5206e_device::uart2_w));

	// parallel port
	map(0xf00001c5, 0xf00001c5).rw(FUNC(mcf5206e_device::ppddr_r), FUNC(mcf5206e_device::ppddr_w));
	map(0xf00001c9, 0xf00001c9).rw(FUNC(mcf5206e_device::ppdat_r), FUNC(mcf5206e_device::ppdat_w));

	// mbus (i2c)
	map(0xf00001e0, 0xf00001f3).m(m_mbus, FUNC(coldfire_mbus_device::mbus_map));
}

/* 
 * DRAM Controller
 * Handles the DRAM refresh and access control circuits
 */
void mcf5206e_device::dcrr_w(uint16_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Refresh Register) DCRR_w: %04x\n", this->machine().describe_context(), data);
	m_dcrr = data;
}

void mcf5206e_device::dctr_w(uint16_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Timing Register) DCTR_w: %04x\n", this->machine().describe_context(), data);
	m_dctr = data;
}

void mcf5206e_device::dcar0_w(uint16_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Access Register 0) DCAR0_w: %04x\n", this->machine().describe_context(), data);
	m_dcar0 = data;
}

void mcf5206e_device::dcmr0_w(uint32_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Mask Register 0) DCMR0_w: %08x\n", this->machine().describe_context(), data);
	m_dcmr0 = data;
}

void mcf5206e_device::dccr0_w(uint8_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Control Register 0) DCCR0_w: %04x\n", this->machine().describe_context(), data);
	m_dccr0 = data;
}

void mcf5206e_device::dcar1_w(uint16_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Access Register 1) DCAR1_w: %04x\n", this->machine().describe_context(), data);
	m_dcar1 = data;
}

void mcf5206e_device::dcmr1_w(uint32_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Mask Register 1) DCMR1_w: %04x\n", this->machine().describe_context(), data);
	m_dcmr1 = data;
}

void mcf5206e_device::dccr1_w(uint8_t data){
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Control Register 1) DCCR1_w: %04x\n", this->machine().describe_context(), data);
	m_dccr1 = data;
}

/* 
 * Chip Select Module 
 * Controls what address spaces that the configurable chip select pins will be assigned to.
 */

inline uint16_t mcf5206e_device::csar_x_r(offs_t offset)
{
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Address Register) CSAR%d_r\n", this->machine().describe_context(), offset);
	return m_csar[offset];
}

inline void mcf5206e_device::csar_x_w(offs_t offset, uint16_t data)
{
	m_csar[offset] = data;
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Address Register) CSAR%d_w %04x\n", this->machine().describe_context(), offset, data);
}

inline uint32_t mcf5206e_device::csmr_x_r(offs_t offset)
{
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Mask Register) CSMR%d_r\n", this->machine().describe_context(), offset);
	return m_csmr[offset];
}

inline void mcf5206e_device::csmr_x_w(offs_t offset, uint32_t data)
{
	m_csmr[offset] = data;
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Mask Register) CSMR%d_w %08x\n", this->machine().describe_context(), offset, data);
}

inline uint16_t mcf5206e_device::cscr_x_r(offs_t offset)
{	
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Control Register) CSCR%d_r\n", this->machine().describe_context(), offset);
	return m_cscr[offset];
}

inline void mcf5206e_device::cscr_x_w(offs_t offset, uint16_t data)
{
	m_cscr[offset] = data;
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Control Register) CSCR%d_w %04x\n", this->machine().describe_context(), offset, data);
}

void mcf5206e_device::dmcr_w(uint16_t data)
{
	m_dmcr = data;
	LOGMASKED(LOG_DEBUG, "%s: (Default Memory Control Register) DMCR_w %04x\n", this->machine().describe_context(), data);
}


/*  
 * Parallel port 
 * Just a 8 bit GPIO. Nothing to see here
 */

void mcf5206e_device::gpio_pin_w(int pin, int state){
	BITWRITE(m_ppdat_in, pin, state);
}

void mcf5206e_device::gpio_port_w(u8 state){
	m_ppdat_in = state;
}

void mcf5206e_device::ppddr_w(uint8_t data)
{
	LOGMASKED(LOG_DEBUG, "%s: (Port A Data Direction Register) PPDDR_w %02x\n", this->machine().describe_context(), data);

	if(m_ppddr != data){
		// Updating the register updates the pins immediately
		uint8_t mask = 0;
		if(!BIT(m_sim->get_par(), 4)) mask |= 0x0f;	// PP 0-3 / DDATA 0-3
		if(!BIT(m_sim->get_par(), 5)) mask |= 0xf0;	// PP 4-7 / PST 0-3

		// GPIO pins will physically be set to the current input and output state, and masked according to PAR
		m_gpio_w_cb(((m_ppdat_out & m_ppddr) | (m_ppdat_in & ~m_ppddr)) & mask);
	}

	m_ppddr = data;
}

void mcf5206e_device::ppdat_w(uint8_t data)
{
	LOGMASKED(LOG_DEBUG, "%s: (Port A Data Register) PPDAT_w %02x\n", this->machine().describe_context(), data);
	m_ppdat_out = data;

	uint8_t mask = 0;
	if(!BIT(m_sim->get_par(), 4)) mask |= 0x0f;	// PP 0-3 / DDATA 0-3
	if(!BIT(m_sim->get_par(), 5)) mask |= 0xf0;	// PP 4-7 / PST 0-3

	m_gpio_w_cb(((m_ppdat_out & m_ppddr) | (m_ppdat_in & ~m_ppddr)) & mask);
}


/* 
 * System Integration Module
 * Handles the interrupt system and bus
*/

coldfire_sim_device::coldfire_sim_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) : 
	device_t(mconfig, COLDFIRE_SIM, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_swdt(*this, "watchdog")
{
}

void coldfire_sim_device::device_add_mconfig(machine_config &config){
	WATCHDOG_TIMER(config, m_swdt);
}

void coldfire_sim_device::device_start(){

	m_swdt->watchdog_enable(0);
	m_timer_swdt = timer_alloc( FUNC( coldfire_sim_device::swdt_callback ), this );	// For interrupt version

	save_item(NAME(m_simr));
	save_item(NAME(m_marb));

	save_item(NAME(m_par));
	save_item(NAME(m_icr));
	save_item(NAME(m_imr));
	save_item(NAME(m_ipr));
	save_item(NAME(m_sypcr));
	save_item(NAME(m_swivr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_swdt_w_count));
	save_item(NAME(m_sypcr_locked));
	save_item(NAME(m_external_ipl));
}

void coldfire_sim_device::device_reset(){
	if(!(m_rsr & 0x20)) m_rsr = 0x00;	// don't clear if watchdog triggered
	else m_rsr = 0x70;

	m_simr = 0xc0;
	m_marb = 0x00;
	m_ipr = 0x3ffe;
	m_sypcr	= 0x00;
	m_swivr = 0x0f;
	m_imr = 0x3ffe;
	m_ipr = 0x0000;
	m_par = 0x0000;

	m_swdt_w_count = 0;
	m_sypcr_locked = false;

	m_icr[ICR1] =   0x04;
	m_icr[ICR2] =   0x08;
	m_icr[ICR3] =   0x0c;
	m_icr[ICR4] =   0x10;
	m_icr[ICR5] =   0x14;
	m_icr[ICR6] =   0x18;
	m_icr[ICR7] =   0x1c;
	m_icr[ICR_SWDT] =   0x1c;
	m_icr[ICR_TMR1] =   0x80;
	m_icr[ICR_TMR2] =  0x80;
	m_icr[ICR_MBUS] =  0x80;
	m_icr[ICR_UART1] =  0x00;
	m_icr[ICR_UART2] =  0x00;
	m_icr[ICR_DMA0] =  0x00;
	m_icr[ICR_DMA1] =  0x00;

	m_external_ipl = 0;
}

void coldfire_sim_device::sim_map(address_map &map){
	map(0x03, 0x03).rw(FUNC(coldfire_sim_device::simr_r), FUNC(coldfire_sim_device::simr_w));
	map(0x07, 0x07).rw(FUNC(coldfire_sim_device::marb_r), FUNC(coldfire_sim_device::marb_w));

	/* interrupt control registers */
	map(0x14, 0x22).rw(FUNC(coldfire_sim_device::icr_r), FUNC(coldfire_sim_device::icr_w));
	map(0x36, 0x37).rw(FUNC(coldfire_sim_device::imr_r), FUNC(coldfire_sim_device::imr_w));
	map(0x3a, 0x3b).r(FUNC(coldfire_sim_device::ipr_r));

	map(0x40, 0x40).rw(FUNC(coldfire_sim_device::rsr_r), FUNC(coldfire_sim_device::rsr_w));
	map(0x41, 0x41).rw(FUNC(coldfire_sim_device::sypcr_r), FUNC(coldfire_sim_device::sypcr_w));
	map(0x42, 0x42).rw(FUNC(coldfire_sim_device::swivr_r), FUNC(coldfire_sim_device::swivr_w));
	map(0x43, 0x43).w(FUNC(coldfire_sim_device::swsr_w));

	map(0xca, 0xcb).rw(FUNC(coldfire_sim_device::par_r), FUNC(coldfire_sim_device::par_w));
}

// MBAR + 0x003: SIM Configuration Register - Not really applicable to MAME as there's no BDM port currently but hey.
void  coldfire_sim_device::simr_w(uint8_t data){
	LOGMASKED(LOG_DEBUG, "%s: SIMR_w %02x\n", this->machine().describe_context(), data);
	m_simr = data;
}

// MBAR + 0x003: Bus Master Arbitration Control
void coldfire_sim_device::marb_w(uint8_t data){
	LOGMASKED(LOG_DEBUG, "%s: (Bus Master Arbitration Control) MARB_w %02x\n", this->machine().describe_context(), data);
	m_marb = data;
}

// MBAR + 0x014 -> 0x022: Interupt Control Registers
uint8_t coldfire_sim_device::icr_r(offs_t offset){
	if(offset > 15){
		logerror("%s: Request to read invalid ICR offset received: %d", this->machine().describe_context(), offset);
		return 0;
	}
	LOGMASKED(LOG_DEBUG, "%s: (Interrupt Control Register %d) read: %02x\n", this->machine().describe_context(), offset, m_icr[offset]);
	return m_icr[offset];
}

void coldfire_sim_device::icr_w(offs_t offset, uint8_t data){
	switch (offset){
		case 0: m_icr[offset] = (data & 0x83) + (1 << 2); break;
		case 1: m_icr[offset] = (data & 0x83) + (2 << 2); break;
		case 2: m_icr[offset] = (data & 0x83) + (3 << 2); break;
		case 3: m_icr[offset] = (data & 0x83) + (4 << 2); break;
		case 4: m_icr[offset] = (data & 0x83) + (5 << 2); break;
		case 5: m_icr[offset] = (data & 0x83) + (6 << 2); break;
		case 6: m_icr[offset] = (data & 0x83) + (7 << 2); break;
		case 7: m_icr[offset] = (data & 0x03) + (7 << 2); break;	// IPL7 and SWDT share same level, also you cannot use autovector on SWT.
		case 8: m_icr[offset] = (data & 0x1f) + 0x80; break;		// Timer 1 *must* use autovector
		case 9: m_icr[offset] = (data & 0x1f) + 0x80; break;		// Timer 2 *must* use autovector
		case 10: m_icr[offset] = (data & 0x1f) + 0x80; break;		// MBUS *must* use autovector
		case 11: m_icr[offset] = (data & 0x9f); break;
		case 12: m_icr[offset] = (data & 0x9f); break;
		case 13: m_icr[offset] = (data & 0x9f); break;
		case 14: m_icr[offset] = (data & 0x9f); break;
		default: logerror("%s: Implausible ICR offset received: %d", this->machine().describe_context(), offset);
	}
	//ICR_info(m_icr[offset]);
}

// MBAR + 0x036: Interrupt Mask Register
void coldfire_sim_device::imr_w(uint16_t data)
{
	m_imr = (data & 0xfffe);
	LOGMASKED(LOG_DEBUG, "%s: (Interrupt Mask Register) IMR_w %04x\n", this->machine().describe_context(), data);
}

void coldfire_sim_device::icr_info(uint8_t icr)
{
	LOGMASKED(LOG_DEBUG, "  (AutoVector) AVEC : %01x | ", (icr&0x80)>>7);
	LOGMASKED(LOG_DEBUG, "(Interrupt Level) IL : %01x | ", INT_LEVEL(icr)); // if autovector (AVEC) is used then the vectors referenced are at +24 (+0x18) + IL, ie the standard 68k autovectors, otherwise vector must be provided by device
	LOGMASKED(LOG_DEBUG, "(Interrupt Priority) IP : %01x |", (icr&0x03)>>0);
	LOGMASKED(LOG_DEBUG, "(Unused bits) : %01x\n", (icr&0x60)>>5);
}

void coldfire_sim_device::par_w(uint16_t data)
{
	m_par = data;
	LOGMASKED(LOG_DEBUG, "%s: (Pin Assignment Register) PAR_w %04x\n", this->machine().describe_context(), data);
}

void coldfire_sim_device::set_external_interrupt(int level, int state){
	// State here is inverted, inputs are active low
	if(BIT(m_par, 6)){
		// External IPL pins are encoded (IPL 1-7 levels)
		m_external_ipl = level;
	} else {
		// External IPL pins are discrete (IRQ1, IRQ4, IRQ7)
		switch(level){
			case 1: BITWRITE(m_external_ipl, 0, state); break;
			case 4: BITWRITE(m_external_ipl, 1, state); break;
			case 7: BITWRITE(m_external_ipl, 2, state); break;
			default: break;
		}
	}
}

// Decides whether to forward interrupt to CPU or not
void coldfire_sim_device::set_interrupt(int interrupt){
	LOG("set_interrupt(%d): %d, %d", interrupt, BIT(m_imr, interrupt), m_imr & interrupt);
	if(!BIT(m_imr, interrupt)){
		uint8_t ICR = 0;
		uint8_t vector = ~0;
		uint32_t intbit = (1 << interrupt);
		
		m_ipr = ((m_ipr & ~intbit) | intbit);
	
		switch (interrupt){
			case WATCHDOG_IRQ: 								// Never autovector
				ICR = m_icr[ICR_SWDT]; 
				vector = m_swivr;
				break;	
			case TIMER_1_IRQ:	ICR = m_icr[ICR_TMR1]; break;	// Always autovector
			case TIMER_2_IRQ: 	ICR = m_icr[ICR_TMR2]; break;	// Always autovector
			case MBUS_IRQ: 		ICR = m_icr[ICR_MBUS]; break;	// Always autovector
			case UART_1_IRQ: 
				ICR = m_icr[ICR_UART1];
				if (!ICR_USE_AUTOVEC(ICR)) vector = m_maincpu->m_uart[0]->get_irq_vector();
				break;
			case UART_2_IRQ: 
				ICR = m_icr[ICR_UART2];
				if (!ICR_USE_AUTOVEC(ICR)) vector = m_maincpu->m_uart[1]->get_irq_vector();
				break;
			
			default: return;
		}

		if (ICR_USE_AUTOVEC(ICR)){
			//LOGMASKED(LOG_DEBUG, "%s: Interrupt from %d: ICR: %02x\n", this->machine().describe_context(), interrupt, ICR);
			m_maincpu->set_input_line(INT_LEVEL(ICR), HOLD_LINE);
		} else {
			LOGMASKED(LOG_SIM, "%s: Interrupt & vector from %d: ICR: %02x, Vector: %02x\n", this->machine().describe_context(), interrupt, ICR, vector);
			//m_maincpu->set_input_line(INT_LEVEL(ICR), HOLD_LINE);
			m_maincpu->m_cpu_space->write_byte(0xfffffff1 | (INT_LEVEL(ICR) << 1), vector);
			m_maincpu->m68ki_exception_interrupt(INT_LEVEL(ICR));
		}
		m_ipr = 0;
	}
}

/* Reset status */
// MBAR + 0x040
void coldfire_sim_device::rsr_w(uint8_t data)
{
	m_rsr &= ~data;
	LOGMASKED(LOG_DEBUG, "%s: (Reset Status Register) RSR_w %02x\n", this->machine().describe_context(), data);
}

/* Watchdog */
// MBAR + 0x041
void coldfire_sim_device::sypcr_w(uint8_t data){
	if(!m_sypcr_locked){
		// SYPCR is a write-once register, whatever is written first remains until system reset.
		LOGMASKED(LOG_SWDT, "%s: (System Protection Control) SYPCR_w %02x\n", this->machine().describe_context(), data);
		m_sypcr_locked = true;
		m_sypcr = data;

		// Bus monitoring is not supported (nor will it be?)

		if(BIT(m_sypcr, 6)){
			m_swdt->watchdog_enable(BIT(data, 7));
		} else {
			// Set timer for interrupt
		}
	} else {
		LOG("%s: Write to SYPCR_w (%02x) when PCR is locked\n", this->machine().describe_context(), data);
	}
}

// MBAR + 0x042
void coldfire_sim_device::swivr_w(uint8_t data){
	LOGMASKED(LOG_SWDT, "%s: (Software Watchdog Interrupt Vector) SWIVR_w %02x\n", this->machine().describe_context(), data);
	m_swivr = data;
}

// MBAR + 0x043
void coldfire_sim_device::swsr_w(uint8_t data){
	LOGMASKED(LOG_SWDT, "%s: (Software Watchdog Service Routine) SWIVR_r %02x\n", this->machine().describe_context(), data);
	if(data == swdt_reset_sequence[m_swdt_w_count]) m_swdt_w_count++;
	if(m_swdt_w_count == 2) {
		m_swdt->watchdog_reset();
		m_swdt_w_count = 0;
	}
}

TIMER_CALLBACK_MEMBER(coldfire_sim_device::swdt_callback)
{
	// Todo
	//set_interrupt(WDINT);
}

/* 
 * Timer Module/s 
 * Creates an MCF5206e compatible 16-bit timer
 */
coldfire_timer_device::coldfire_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) : 
	device_t(mconfig, COLDFIRE_TIMER, tag, owner, clock)
	, m_timer_irq(*this)
{
}

void coldfire_timer_device::timer_map(address_map &map){
	map(0x00, 0x01).rw(FUNC(coldfire_timer_device::tmr_r), FUNC(coldfire_timer_device::tmr_w));
	map(0x04, 0x05).rw(FUNC(coldfire_timer_device::trr_r), FUNC(coldfire_timer_device::trr_w));
	map(0x08, 0x09).r(FUNC(coldfire_timer_device::tcr_r));	// TCR is r/only
	map(0x0c, 0x0d).rw(FUNC(coldfire_timer_device::tcn_r), FUNC(coldfire_timer_device::tcn_w));
	map(0x11, 0x11).rw(FUNC(coldfire_timer_device::ter_r), FUNC(coldfire_timer_device::ter_w));
}

void coldfire_timer_device::device_start(){
	m_timer = timer_alloc( FUNC( coldfire_timer_device::timer_callback ), this );

	save_item(NAME(m_tmr));
	save_item(NAME(m_trr));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tcn));
	save_item(NAME(m_ter));
	save_item(NAME(m_timer_start_time));
}

void coldfire_timer_device::device_reset(){
	m_tmr = 0x0000;
	m_trr = 0xffff;
	m_tcn = 0x0000;
	m_tcr = 0x0000;
	m_ter = 0x00;
	m_timer_start_time = attotime::zero;
	m_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(coldfire_timer_device::timer_callback)
{
	if(m_tmr& T_FRR){
		// FRR resets counter to 0
		m_tcn = 0;
	}
	m_ter |= T_EREF;
	m_timer_irq(0);
}

void coldfire_timer_device::tmr_w(uint16_t data)
{
	uint16_t cmd = data;

	if((m_tmr & T_RST) && !(cmd & T_RST)){
		// T_RST high to low resets the entire timer
		device_reset();
		return;
	}

	m_tmr = cmd;

	if (m_tmr & T_RST){
		// todo: add tin pin support
		int div, start, interval;
		start = (m_trr - m_tcn);
		div = ((m_tmr & 0xff00) >> 8) + 1; 						// 1 -> 256 division scale
		if ((m_tmr & T_CL1) && !(m_tmr & T_CL0)) div = div * 16;	// input clock / 16
		if (!(m_tmr & T_FRR)) interval = (0xffff * div);			// don't reset tcn to 0
		else interval = start;										// else tcn is reset to 0
		m_timer_start_time = machine().time();
		m_timer->adjust(clocks_to_attotime(start * div), 0, clocks_to_attotime(interval * div));
	}

	LOGMASKED(LOG_TIMER, "%s: (Timer Mode Register) TMR_w: %04x\n", this->machine().describe_context(), data);
	//LOGMASKED(LOG_TIMER, " (Prescale) PS : %02x  (Capture Edge/Interrupt) CE : %01x (Output Mode) OM : %01x  (Output Reference Interrupt En) ORI : %01x   Free Run (FRR) : %01x  Input Clock Source (ICLK) : %01x  (Reset Timer) RST : %01x  \n", (m_TMR1 & 0xff00)>>8, (m_TMR1 & 0x00c0)>>6,  (m_TMR1 & 0x0020)>>5, (m_TMR1 & 0x0010)>>4, (m_TMR1 & 0x0008)>>3, (m_TMR1 & 0x0006)>>1, (m_TMR1 & 0x0001)>>0);
}

void coldfire_timer_device::trr_w(uint16_t data)
{
	m_trr = data;
	LOGMASKED(LOG_TIMER, "%s: (Timer Reference Register) TRR_w: %04x\n", this->machine().describe_context(), data);
}

uint16_t coldfire_timer_device::tcn_r()
{
	m_tcn = (attotime_to_clocks(machine().time() - m_timer_start_time) & 0xffff);
	return m_tcn; 
}

void coldfire_timer_device::tcn_w(uint16_t data)
{
	// Writing any value resets the counter
	m_tcn = 0;
	LOGMASKED(LOG_TIMER, "%s: (Timer Counter Reset) TCN_w: %04x\n", this->machine().describe_context(), data);
}

void coldfire_timer_device::ter_w(uint8_t data)
{
	m_ter &= ~data; // Programmer must write bit to clear it. IE write 0x80 to clear bit 7.
	LOGMASKED(LOG_TIMER, "%s: (Timer Event) TER_w: %02x\n", this->machine().describe_context(), data);
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

#define UNINIT 0
#define UNINIT_NOTE 0

/*
 * init_regs
 * Resets the internal registers to their POR states.
 * first_init is used during boot, set to false during reset
 */
void mcf5206e_device::init_regs(bool first_init)
{
	m_dcrr = 0x0000;
	m_dctr = 0x0000; 
	m_dcar0 = UNINIT;
	m_dcmr0 = UNINIT;
	m_dccr0 = 0x00;
	m_dcar1 = UNINIT;
	m_dcmr1 = UNINIT;
	m_dccr1 = 0x00;

	m_csar[0] = 0x0000;
	m_csmr[0] = 0x00000000;
	m_cscr[0] = 0x3c1f; /* 3C1F, 3C5F, 3C9F, 3CDF, 3D1F, 3D5F, 3D9F, 3DDF |  AA set by IRQ 7 at reset, PS1 set by IRQ 4 at reset, PS0 set by IRQ 1 at reset*/

	if (first_init)
	{
		for (int x=1;x<8;x++)
		{
			m_csar[1] = UNINIT;
			m_csmr[1] = UNINIT;
			m_cscr[1] = UNINIT_NOTE; // except BRST=ASET=WRAH=RDAH=WR=RD=0
		}
	}

	m_dmcr = 0x0000;

	m_ppddr = 0x00;
	m_ppdat_in = 0x00;
	m_ppdat_out = 0x00;
}


/*
 * MBUS Module
 * Hosts I2C and Motorola extensions to the format. Can act as a device or host.
*/

coldfire_mbus_device::coldfire_mbus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) : 
	device_t(mconfig, COLDFIRE_MBUS, tag, owner, clock)
	, write_sda(*this)
	, write_scl(*this)
	, write_irq(*this)
{
}


void coldfire_mbus_device::device_start(){
	m_timer_mbus = timer_alloc( FUNC( coldfire_mbus_device::mbus_callback ), this );

	save_item(NAME(m_madr));
	save_item(NAME(m_mbcr));
	save_item(NAME(m_mbsr));
	save_item(NAME(m_mfdr));
	save_item(NAME(m_mbdr));

	save_item(NAME(m_tx_in_progress));
	save_item(NAME(m_clk_state));
	save_item(NAME(m_tx_bit));
	save_item(NAME(m_tx_out));
	save_item(NAME(m_tx_in));

	device_reset();
}

void coldfire_mbus_device::device_reset(){
	m_madr = 0x00;
	m_mfdr = 0x00;
	m_mbcr = 0x00;
	m_mbsr = 0x81;
	m_mbdr = 0x00;

	m_tx_in_progress = false;
	m_clk_state = 1;
	m_tx_bit = 0;
	m_tx_out = 0;
	m_tx_in = 0;

	m_timer_mbus->adjust(attotime::never);
	write_scl(1);
	write_sda(1);
}


void coldfire_mbus_device::mbus_map(address_map &map){
	map(0x00, 0x00).rw(FUNC(coldfire_mbus_device::madr_r), FUNC(coldfire_mbus_device::madr_w));
	map(0x04, 0x04).rw(FUNC(coldfire_mbus_device::mfdr_r), FUNC(coldfire_mbus_device::mfdr_w));
	map(0x08, 0x08).rw(FUNC(coldfire_mbus_device::mbcr_r), FUNC(coldfire_mbus_device::mbcr_w));
	map(0x0c, 0x0c).rw(FUNC(coldfire_mbus_device::mbsr_r), FUNC(coldfire_mbus_device::mbsr_w));
	map(0xf0, 0xf0).rw(FUNC(coldfire_mbus_device::mbdr_r), FUNC(coldfire_mbus_device::mbdr_w));
}

TIMER_CALLBACK_MEMBER(coldfire_mbus_device::mbus_callback){
	// Do bit transfers etc
}

void coldfire_mbus_device::madr_w(uint8_t data)
{
	m_madr = (data & 0xfe);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Control Register) madr_w: %02x\n", this->machine().describe_context(), data);
}

void coldfire_mbus_device::mfdr_w(uint8_t data)
{
	m_mfdr = (data & 0x3F);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Frequency Divider Register) mfdr_w: %02x\n", this->machine().describe_context(), data);
}

void coldfire_mbus_device::mbcr_w(uint8_t data)
{
	m_mbcr = (data & 0xfc);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Control Register) mbcr_w: %02x\n", this->machine().describe_context(), data);
}

uint8_t coldfire_mbus_device::mbsr_r()
{
	int hack = 0x00;

	hack ^= (machine().rand()&0xff);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Status Register) mbsr_r: %02x\n", this->machine().describe_context(), m_mbsr);
	return m_mbsr ^ hack; // will loop on this after a while
}

void coldfire_mbus_device::mbsr_w(uint8_t data)
{
	m_mbsr = (data & 0x14);	// MAL & MIF
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Status Register) mbsr_w: %02x\n", this->machine().describe_context(), data);
}

uint8_t coldfire_mbus_device::mbsr_r()
{
	int hack = 0x00;
	hack ^= (machine().rand()&0xff);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Data I/O Register) mbdr_r: %02x\n", this->machine().describe_context(), m_mbdr);
	return m_mbdr ^ hack;
}

void coldfire_mbus_device::mbdr_w(uint8_t data)
{
	m_mbdr = data;
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Data I/O Register) mbdr_w: %02x\n", this->machine().describe_context(), data);
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
$1E0                    madr        8       M-Bus Address Register                              00                                                  R/W
$1E4*-                  mfdr        8       M-Bus Frequency Divider Register                    00                                                  R/W
$1E8*-                  mbcr        8       M-Bus Control Register                              00                                                  R/W
$1EC*-                  mbsr        8       M-Bus Status Register                               00                                                  R/W
$1F0*-                  mbdr        8       M-Bus Data I/O Register                             00                                                  R/W
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
