// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, R. Belmont
/*
 * apollo.c - APOLLO DN3500/DN3000 driver
 *
 *  Created on: May 12, 2010
 *      Author: Hans Ostermeyer
 *
 *  Adapted February 19, 2012 for general MAME/MESS standards by R. Belmont
 *
 *  TODO: use MESS ram device and MCFG_RAM_* to handle RAM sizing.
 *        Remove need for instruction hook.
 *        Convert to modern address map.
 *
 *  see also:
 *  - Domain Series 3000/Series 4000 Hardware Architecture Handbook (Order No. 007861 Rev. 02)
 *  - Domain Personal Workstations and Servers Technical Reference (Apollo Order No. 008778-A01)
 *  - Servicing the Domain Personal Workstations and Servers (Apollo Order No. 007859-A01)
 *  - http://www.bitsavers.org/pdf/apollo/002398-04_Domain_Engineering_Handbook_Rev4_Jan87.pdf
 *  - http://www.bitsavers.org/pdf/apollo/008778-03_DOMAIN_Series_3000_4000_Technical_Reference_Aug87.pdf
 *  - http://www.bitsavers.org/pdf/apollo/AEGIS_Internals_and_Data_Structures_Jan86.pdf
 *  - http://www.bitsavers.org/pdf/apollo/019411-A00_Addendum_to_Domain_Personal_Workstations_and_Servers_Hardware_Architecture_Handbook_1991.pdf
 *  - data sheets from Intel and Motorola
 */

#define VERBOSE 0

#include "includes/apollo.h"
#include "debugger.h"
#include "cpu/m68000/m68kcpu.h"
#include "sound/beep.h"
#include "apollo_dsp.lh"

// we use set_verbose
#include "bus/isa/omti8621.h"
#include "bus/isa/3c505.h"

#define TERMINAL_TAG "terminal"

// we use this to prevent excessive logging (if emulation runs amok)
// error.log will be 10 MB for 100000 lines
#define APOLLO_MAX_NO_OF_LOG_LINES 1000000

// ISA/AT Bus notes
// I/O space: to get the Apollo address = take the PC I/O address, keep the low 3 bits how they are, and shift the rest left 7, inserting zeros.
// then add 0x40000 for the I/O base.
//
// example: 3c503 Ethernet is at I/O 300h on PC, which is (%1100000000 -> 1 1000 0000 0000 0000) + 0x40000 = 0x58000
//
// Memory space: addresses from 0x80000 to 0xffffff are supported, including the possibility of stock PC MDA at a0000

#define ATBUS_IO_BASE       0x040000
#define ATBUS_IO_END        0x05ffff
#define ATBUS_MEMORY_BASE   0x080000
#define ATBUS_MEMORY_END    0xffffff

#define DN3500_RAM_SIZE     16 // 8, 16 or 32 MB

#if DN3500_RAM_SIZE == 8
#define DN3500_RAM_BASE     0x1000000
#define DN3500_RAM_END      0x17fffff
#define DN3500_RAM_CONFIG_BYTE  0x64 // 4-4-0-0
#elif DN3500_RAM_SIZE == 16
#define DN3500_RAM_BASE     0x1000000
#define DN3500_RAM_END      0x1ffffff
#define DN3500_RAM_CONFIG_BYTE 0x60 // 4-4-4-4
#else /* DN3500_RAM_SIZE == 32 */
#define DN3500_RAM_BASE     0x1000000
#define DN3500_RAM_END      0x3ffffff
#define DN3500_RAM_CONFIG_BYTE 0x20 // 8-8-8-8
#endif

#define DN3000_RAM_BASE     0x100000
#define DN3000_RAM_END      0x8fffff
#define DN3000_RAM_CONFIG_8MB  0x20 // 2-2-2-2

#define DN5500_RAM_SIZE     32 // 16 or 32 MB

#if DN5500_RAM_SIZE == 16
#define DN5500_RAM_BASE     0x1000000
#define DN5500_RAM_END      0x1ffffff
#define DN5500_RAM_CONFIG_BYTE  0x14 // 8-8-0-0
#define DN5500_MEM_PRESENT_BYTE 0xAA // 8-8-0-0
#else /* DN5500_RAM_SIZE == 32 */
#define DN5500_RAM_BASE     0x1000000
#define DN5500_RAM_END      0x2ffffff
#define DN5500_RAM_CONFIG_BYTE  0x20 // 8-8-8-8
#define DN5500_MEM_PRESENT_BYTE 0x00 // 8-8-8-8
#endif

#define NODE_TYPE_DN3000 3000
#define NODE_TYPE_DN3500 3500
#define NODE_TYPE_DN5500 5500
#define NODE_TYPE_DSP3000 -3000
#define NODE_TYPE_DSP3500 -3500
#define NODE_TYPE_DSP5500 -5500

#define DEFAULT_NODE_ID 0x12345

static UINT8 cache_control_register = 0x00;
static UINT8 cache_status_register = 0xff;
static UINT8 task_alias_register = 0x00;

static offs_t parity_error_offset = 0;
static UINT16 parity_error_byte_mask = 0;
static int parity_error_handler_is_installed = 0;
static int parity_error_handler_install_counter = 0;

static UINT16 latch_page_on_parity_error_register = 0x0000;
static UINT16 master_req_register = 0x0000;

static UINT32 ram_base_address;
static UINT32 ram_end_address;

static int node_type;

// FIXME: value of ram_config_byte must match with default/selected RAM size
static UINT8 ram_config_byte;

static UINT32 log_line_counter = 0;

/***************************************************************************
 cpu_context - return a string describing which CPU is currently executing and their PC
 ***************************************************************************/

const char *apollo_cpu_context(device_t *cpu) {
	static char statebuf[64]; /* string buffer containing state description */

	/* if we have an executing CPU, output data */
	if (cpu != nullptr) {
		osd_ticks_t t = osd_ticks();
		int s = t / osd_ticks_per_second();
		int ms = (t % osd_ticks_per_second()) / 1000;

		sprintf(statebuf, "%d.%03d %s pc=%08x", s, ms, cpu->tag(), cpu->safe_pcbase());
	} else {
		strcpy(statebuf, "(no context)");
	}
	return statebuf;
}

/*-------------------------------------------------
 apollo_set_cpu_has_fpu - enable/disable the FPU
 -------------------------------------------------*/

void apollo_set_cpu_has_fpu(m68000_base_device *device, int onoff)
{
	if (device == nullptr || (device->type() != M68020PMMU && device->type() != M68030))
	{
		DLOG1(("set_cpu_has_fpu: unexpected CPU device"));
	}
	else
	{
		device->has_fpu = onoff;
		DLOG1(("apollo_set_cpu_has_fpu: FPU has been %s", onoff ? "enabled" : "disabled"));
	}
}

/***************************************************************************
 apollo_check_log - check for excessive logging
 ***************************************************************************/

void apollo_check_log() {
	if (++log_line_counter >= APOLLO_MAX_NO_OF_LOG_LINES) {
		fatalerror("apollo_check_log: maximum number of log lines exceeded.\n");
	}
}

/***************************************************************************
 apollo_is_dn3000 - return 1 if node is DN3000 or DSP3000, 0 otherwise
 ***************************************************************************/

int apollo_is_dn3000(void) {
	return node_type == NODE_TYPE_DN3000 || node_type == NODE_TYPE_DSP3000;
}

/***************************************************************************
 apollo_is_dn5500 - return 1 if node is DN5500 or DSP5500, 0 otherwise
 ***************************************************************************/

int apollo_is_dn5500(void) {
	return node_type == NODE_TYPE_DN5500 || node_type == NODE_TYPE_DSP5500;
}

/***************************************************************************
 apollo_is_dsp3x00 - return 1 if node is DSP3x00 or DSP5500, 0 otherwise
 ***************************************************************************/

int apollo_is_dsp3x00(void) {
	switch (node_type)
	{
	case NODE_TYPE_DSP3000:
	case NODE_TYPE_DSP3500:
	case NODE_TYPE_DSP5500:
		return 1;
	}
	return 0;
}

/***************************************************************************
 apollo_get_ram_config_byte - get the ram configuration byte
 ***************************************************************************/

UINT8 apollo_get_ram_config_byte(void) {
	return ram_config_byte;
}

/***************************************************************************
  apollo_instruction_hook
  must be called by the CPU core before executing each instruction
***************************************************************************/
READ32_MEMBER(apollo_state::apollo_instruction_hook)
{
	static UINT16 idle_counter = 0;

	// m_maincpu->ir still has previous instruction
	UINT16 last_ir = m_maincpu->ir;

	// get next instruction (or 0 if unavailable)
	UINT16 next_ir = (m_maincpu->pref_addr == REG_PC(m_maincpu)) ? m_maincpu->pref_data : 0;

	// check for NULLPROC:
	// 027C F8FF AND.W #F8FF,SR
	// 60FA      BRA *-4

	if ((next_ir == 0x60fa && last_ir == 0x027c) || (next_ir == 0x027c  && last_ir == 0x60fa))
	{
		// we are within the idle loop, slow down CPU to reduce power usage
		m_maincpu->remaining_cycles -= 500;

		if (apollo_config(APOLLO_CONF_IDLE_SLEEP) && apollo_is_dsp3x00() && ++idle_counter >= 1000)
		{
			// slow down even more on DSP3x00
			idle_counter -= 100;
			// sleep 1 ms
			osd_sleep(osd_ticks_per_second() / 1000);
		}
	}
	else
	{
		// we are outside of the idle loop
		idle_counter = 0;
	}

	if (!m_maincpu->has_fpu && !m_maincpu->pmmu_enabled && (m_maincpu->ir & 0xff00) == 0xf200)
	{
		// set APOLLO_CSR_SR_FP_TRAP in cpu status register for /sau7/self_test
		apollo_csr_set_status_register(APOLLO_CSR_SR_FP_TRAP, APOLLO_CSR_SR_FP_TRAP);
	}

	if (m_maincpu->t1_flag && !m_maincpu->s_flag)
	{
		// FIXME: trace emulation is disabled in m68kcpu.h; why???
		m68ki_exception_trace(m_maincpu);
	}

	return apollo_debug_instruction_hook(m_maincpu, offset);
}

/***************************************************************************
 apollo bus error
 ***************************************************************************/

void apollo_state::apollo_bus_error()
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);

	apollo_csr_set_status_register(APOLLO_CSR_SR_CPU_TIMEOUT, APOLLO_CSR_SR_CPU_TIMEOUT);
}

IRQ_CALLBACK_MEMBER(apollo_state::apollo_irq_acknowledge)
{
	int result = M68K_INT_ACK_AUTOVECTOR;

	m_maincpu->set_input_line(irqline, CLEAR_LINE);

	MLOG2(("apollo_irq_acknowledge: interrupt level=%d", irqline));

	if (irqline == 6) {
		result = apollo_pic_acknowledge(device, irqline);
	}
	return result;
}

/***************************************************************************
 DN3500 Cache Control/Status Register at 0x10200
 ***************************************************************************/

WRITE8_MEMBER(apollo_state::cache_control_register_w){
	if (apollo_is_dn5500())
	{
		SLOG1(("Error: writing DN5500 Cache Status Register at offset %02x = %02x", offset, data));
	}
	else
	{
		cache_control_register = data;
		cache_status_register = (cache_status_register & 0x7f) | (cache_control_register & 0x80);
		SLOG2(("writing Cache Control Register at offset %02x = %02x", offset, data));
	}
}

READ8_MEMBER(apollo_state::cache_status_register_r){
	UINT8 data = cache_status_register;

	if (apollo_is_dn5500()) {
#define DN5500_CSR_NOT_HSI_PRESENT 8
#define DN5500_CSR_MEM_TIME 1
		data |= DN5500_CSR_NOT_HSI_PRESENT;
	}

	SLOG2(("reading Cache Status Register at offset %02x = %02x", offset, data));
	return data;
}

void apollo_set_cache_status_register(device_t *device,UINT8 mask, UINT8 data) {
	UINT16 new_value = (cache_status_register & ~mask) | (data & mask);
	if (new_value != cache_status_register) {
		cache_status_register = new_value;
		DLOG2(("setting Cache Status Register with data=%02x and mask=%02x to %02x",
				data, mask, cache_status_register));
	}
}

/***************************************************************************
 DN3500 Task Alias Register at 0x10300
 ***************************************************************************/

WRITE8_MEMBER(apollo_state::task_alias_register_w){
	task_alias_register = data;
	apollo_set_cache_status_register(this,0x07,  data);
	SLOG(("writing Task Alias Register at offset %02x = %02x",offset, data));
}

READ8_MEMBER(apollo_state::task_alias_register_r){
	UINT8 data = 0xff;
	SLOG(("reading Task Alias Register at offset %02x = %02x", offset, data));
	return data;
}

/***************************************************************************
 DN3000/DN3500 Latch Page on Parity Error Register at 0x9300/0x11300
 ***************************************************************************/

WRITE16_MEMBER(apollo_state::latch_page_on_parity_error_register_w){
	latch_page_on_parity_error_register = data;
	SLOG1(("writing Latch Page on Error Parity Register at offset %02x = %04x", offset*2, data));
}

READ16_MEMBER(apollo_state::latch_page_on_parity_error_register_r){
	UINT16 data = latch_page_on_parity_error_register;
	SLOG2(("reading Latch Page on Error Parity Register at offset %02x = %04x", offset*2, data));
	return data;
}

/***************************************************************************
 DN3500 Master REQ Register at 0x11600
 ***************************************************************************/

WRITE8_MEMBER(apollo_state::master_req_register_w){
	master_req_register = data;
	SLOG2(("writing Master REQ Register at offset %02x = %02x", offset, data));
}

READ8_MEMBER(apollo_state::master_req_register_r){
	UINT8 data = 0xff;
	SLOG1(("reading Master REQ Register at offset %02x = %02x", offset, data));
	return data;
}

/***************************************************************************
 DN3500 Selective Clear Locations at 0x11600
 ***************************************************************************/

WRITE16_MEMBER(apollo_state::selective_clear_locations_w){
	SLOG2(("writing Selective Clear Locations at offset %02x = %02x", offset*2, data));
	switch (offset * 2) {
	case 0x00: // Clear All
		apollo_csr_set_status_register(APOLLO_CSR_SR_CLEAR_ALL, 0);
		break;
	case 0x04: // clear floating-point trap
		apollo_csr_set_status_register(APOLLO_CSR_SR_FP_TRAP, 0);
		break;
	case 0x06: // clear Parity error interrupt
		apollo_csr_set_status_register(APOLLO_CSR_SR_PARITY_BYTE_MASK, 0);
		break;
	case 0x08: // clear Bus Error Status (CPU Timeout)
		apollo_csr_set_status_register(APOLLO_CSR_SR_CPU_TIMEOUT, 0);
		break;
	case 0x0e: // Clear (Flush) Cache
		break;
	}
}

READ16_MEMBER(apollo_state::selective_clear_locations_r){
	UINT16 data = 0xffff;
	SLOG1(("reading Selective Clear Locations at offset %02x = %02x", offset*2, data));
	return data;
}

/***************************************************************************
 DN3000/DN3500 RAM with parity (and null proc loop delay for DomainOS)
 ***************************************************************************/

READ32_MEMBER(apollo_state::ram_with_parity_r){
	UINT32 data = m_messram_ptr[parity_error_offset+offset];

	SLOG2(("memory dword read with parity error at %08x = %08x & %08x parity_byte=%04x",
			ram_base_address + parity_error_offset*4 + offset*4,data, mem_mask, parity_error_byte_mask));

	if (parity_error_byte_mask != 0) {
		latch_page_on_parity_error_register = (ram_base_address + parity_error_offset * 4) >> 10;

		apollo_csr_set_status_register(APOLLO_CSR_CR_PARITY_BYTE_MASK,  apollo_csr_get_status_register() |parity_error_byte_mask);

		if (apollo_csr_get_control_register() & APOLLO_CSR_CR_INTERRUPT_ENABLE) {
			// force parity error (if NMI is enabled)
//          cpu_set_input_line_and_vector(&space.device(), 7, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);
			m_maincpu->set_input_line_and_vector(7, ASSERT_LINE, M68K_INT_ACK_AUTOVECTOR);

		}
	}
	return data;
}

WRITE32_MEMBER(apollo_state::ram_with_parity_w){
	COMBINE_DATA(m_messram_ptr+offset);

	if (apollo_csr_get_control_register() & APOLLO_CSR_CR_FORCE_BAD_PARITY) {
		parity_error_byte_mask = (apollo_csr_get_control_register()
				& APOLLO_CSR_CR_PARITY_BYTE_MASK);

		if (!apollo_is_dn3000()) {
			parity_error_byte_mask ^= APOLLO_CSR_CR_PARITY_BYTE_MASK;
		}

		parity_error_offset = offset;

//      SLOG1(("memory dword write with parity to %08x = %08x & %08x parity_byte=%04x",
//              ram_base_address +offset * 4, data, mem_mask, parity_error_byte_mask));

		if (parity_error_handler_is_installed == 0) {
			// no more than 192 read/write handlers may be used
			// see table_assign_handler in memory.c
			if (parity_error_handler_install_counter < 40) {
				//memory_install_read32_handler(space, ram_base_address+offset*4, ram_base_address+offset*4+3, 0xffffffff, 0, ram_with_parity_r);
				space.install_read_handler(ram_base_address+offset*4, ram_base_address+offset*4+3, 0xffffffff,0,read32_delegate(FUNC(apollo_state::ram_with_parity_r),this));
				parity_error_handler_is_installed = 1;
				parity_error_handler_install_counter++;
			}
		}
	} else if (parity_error_handler_is_installed && offset == parity_error_offset) {
		SLOG1(("memory dword write with parity to %08x = %08x & %08x reset %d",
				ram_base_address +parity_error_offset*4, data, mem_mask, parity_error_handler_install_counter));

		// uninstall not supported, reinstall previous read handler instead

		// memory_install_rom(space, ram_base_address, ram_end_address, 0xffffffff, 0, messram_ptr.v);
		space.install_rom(ram_base_address,ram_end_address,0xffffffff,0,&m_messram_ptr[0]);

		parity_error_handler_is_installed = 0;
		parity_error_byte_mask = 0;
	}
}

/***************************************************************************
 DN3000/DN3500 unmapped memory
 ***************************************************************************/

READ32_MEMBER(apollo_state::apollo_unmapped_r)
{
	offs_t address = offset * 4;

	m68000_base_device *m68k = m_maincpu;

	if ((address & 0xfff00000) == 0xfa800000 && VERBOSE < 2) {
		// ?
	} else if ((address & 0xfff00ff7) == 0xfd800000 && VERBOSE < 2) {
		// omit logging for memory sizing in FPA address space
		// strange: MD seems to search for the 3C505 Boot ROM
		// note (6.10.2010): might be color7 address space (!?!)
	} else if ((address & 0xfc03ffff) == 0x00000000 && VERBOSE < 2) {
		// omit logging for memory sizing in standalone utilities
	} else if (address == 0xfff90000 && VERBOSE < 2) {
		// omit logging for FPA trial access
	} else if (address == 0x00030000 && VERBOSE < 2) {
		// omit logging for Bus error test address in DN3500 boot prom and self_test
	} else if (address == 0x0000ac00 && VERBOSE < 2) {
		// omit logging for Bus error test address in DN3000 boot prom
	} else {
		SLOG1(("unmapped memory dword read from %08x with mask %08x (ir=%04x)", address , mem_mask, m68k->ir));
	}

	/* unmapped; access causes a bus error */
	apollo_bus_error();
	return 0xffffffff;
}

WRITE32_MEMBER(apollo_state::apollo_unmapped_w)
{
	SLOG(("unmapped memory dword write to %08x = %08x & %08x", offset * 4, data, mem_mask));

	/* unmapped; access causes a bus error */
	apollo_bus_error();
}

/***************************************************************************
 DN3000/DN3500 ROM write
 ***************************************************************************/

WRITE32_MEMBER(apollo_state::apollo_rom_w)
{
	offs_t address =  offset * 4;
	offs_t pc = space.device().safe_pcbase();

	if (pc == 0x00002c1c && address == 0x00000004 && VERBOSE < 2) {
		// don't log invalid code in 3500_boot_12191_7.bin
	} else {
		SLOG1(("ROM dword write to %08x = %08x & %08x", offset * 4, data, mem_mask));
	}
}

/***************************************************************************
 DN3000/DN3500 AT Bus I/O space
 ***************************************************************************/

READ16_MEMBER(apollo_state::apollo_atbus_io_r)
{
	UINT32 isa_addr = (offset & 3) + ((offset & ~0x1ff) >> 7);

	// Motorola CPU is MSB first, ISA Bus is LSB first
	UINT16 data = m_isa->io16_swap_r(space, isa_addr, mem_mask);

	SLOG2(("apollo_atbus_io_r at %08x -> %04x = %04x & %04x", ATBUS_IO_BASE + offset*2, isa_addr*2, data, mem_mask));

	return data;
}

WRITE16_MEMBER(apollo_state::apollo_atbus_io_w)
{
	UINT32 isa_addr = (offset & 3) + ((offset & ~0x1ff) >> 7);

	SLOG2(("apollo_atbus_io_w at %08x -> %04x = %04x & %04x", ATBUS_IO_BASE + offset*2, isa_addr*2, data, mem_mask));

	// Motorola CPU is MSB first, ISA Bus is LSB first
	m_isa->io16_swap_w(space, isa_addr, data, mem_mask);
}

/***************************************************************************
 DN3000/DN3500 AT Bus memory space
 ***************************************************************************/

READ16_MEMBER(apollo_state::apollo_atbus_memory_r)
{
	UINT16 data;

	// Motorola CPU is MSB first, ISA Bus is LSB first
	data = m_isa->prog16_swap_r(space, offset, mem_mask);

	SLOG2(("apollo_atbus_memory_r at %08x = %04x & %04x", ATBUS_MEMORY_BASE + offset * 2, data, mem_mask));
	return data;
}

WRITE16_MEMBER(apollo_state::apollo_atbus_memory_w)
{
	SLOG2(("apollo_atbus_memory_w at %08x = %04x & %04x", ATBUS_MEMORY_BASE + offset*2, data, mem_mask));

	// Motorola CPU is MSB first, ISA Bus is LSB first
	m_isa->prog16_swap_w(space, offset, data, mem_mask);
}

/***************************************************************************
 DN3000/DN3500 AT Bus unmapped read/write
 ***************************************************************************/

READ16_MEMBER(apollo_state::apollo_atbus_unmap_io_r)
{
	// ISA bus has 0xff for unmapped addresses
	UINT16 data = 0xffff;
	UINT32 isa_addr = (offset & 3) + ((offset & ~0x1ff) >> 7);
	SLOG1(("apollo_atbus_unmap_io_r at %08x -> %04x = %04x & %04x", ATBUS_IO_BASE + offset*2, isa_addr*2, data, mem_mask));
	return data;
}

WRITE16_MEMBER(apollo_state::apollo_atbus_unmap_io_w)
{
	UINT32 isa_addr = (offset & 3) + ((offset & ~0x1ff) >> 7);
	SLOG1(("apollo_atbus_unmap_io_w at %08x -> %04x = %04x & %04x", ATBUS_IO_BASE + offset*2, isa_addr*2, data, mem_mask));
}

READ8_MEMBER(apollo_state::apollo_atbus_unmap_r)
{
	// ISA bus has 0xff for unmapped addresses
	UINT8 data = 0xff;
	SLOG2(("apollo_atbus_unmap_r at %08x = %02x & %02x", ATBUS_MEMORY_BASE + offset, data, mem_mask));
	return data;
}

WRITE8_MEMBER(apollo_state::apollo_atbus_unmap_w)
{
	SLOG1(("apollo_atbus_unmap_w at %08x = %02x & %02x", ATBUS_MEMORY_BASE + offset, data, mem_mask));
}

/***************************************************************************
 DN5500 Memory Present Register at 0x11400-0x114ff
 Strange: documented but not used
 ***************************************************************************/

WRITE8_MEMBER(apollo_state::dn5500_memory_present_register_w){
	SLOG(("Error: writing DN5500 Memory Present Register at offset %02x = %02x", offset, data));
}

READ8_MEMBER(apollo_state::dn5500_memory_present_register_r){
	UINT8 data = DN5500_MEM_PRESENT_BYTE;
	SLOG(("reading DN5500 Memory Present Register at offset %02x = %02x", offset, data));
	return data;
}

/***************************************************************************
 DN5500 11500 Registers at 0x11500-0x115ff (undocumented, what does it do?)
 ***************************************************************************/

WRITE8_MEMBER(apollo_state::dn5500_11500_w){
	SLOG1(("writing DN5500 11500 at offset %02x = %02x", offset, data));
}

READ8_MEMBER(apollo_state::dn5500_11500_r){
	UINT8 data = 0xff;
	SLOG1(("reading DN5500 11500 at offset %02x = %02x", offset, data));
	return data;
}

/***************************************************************************
 DN5500 I/O Protection Map at 0x7000000-0x700FFFF
 ***************************************************************************/

WRITE8_MEMBER(apollo_state::dn5500_io_protection_map_w){
	// TODO
	SLOG1(("writing DN5500 I/O Protection Map at offset %02x = %02x", offset, data));
}

READ8_MEMBER(apollo_state::dn5500_io_protection_map_r){
	UINT8 data = 0xff;
	SLOG1(("reading DN5500 I/O Protection Map at offset %02x = %02x", offset, data));
	return data;
}

#if 0
/***************************************************************************
 DN3000/DN3500 at f8000000 - ffffffff (used by fpa and/or color7?)
 ***************************************************************************/

READ32_MEMBER(apollo_state::apollo_f8_r){
	offs_t address = 0xf8000000 + offset * 4;
	UINT32 data = 0xffffffff;
	SLOG2(("unexpected memory dword read from %08x = %08x & %08x",
					address, data, mem_mask));
	return data;
}

WRITE32_MEMBER(apollo_state::apollo_f8_w){
	offs_t address = 0xf8000000 +offset * 4;

	SLOG2(("unexpected memory dword write to %08x = %08x & %08x",
					address, data, mem_mask));
}
#endif

/***************************************************************************
 ADDRESS MAPS
 ***************************************************************************/

static ADDRESS_MAP_START(dn3500_map, AS_PROGRAM, 32, apollo_state )
		AM_RANGE(0x000000, 0x00ffff) AM_ROM /* boot ROM  */
		AM_RANGE(0x000000, 0x00ffff) AM_WRITE(apollo_rom_w)
		AM_RANGE(0x010000, 0x0100ff) AM_READWRITE16(apollo_csr_status_register_r, apollo_csr_status_register_w, 0xffffffff)
		AM_RANGE(0x010100, 0x0101ff) AM_READWRITE16(apollo_csr_control_register_r, apollo_csr_control_register_w, 0xffffffff)
		AM_RANGE(0x010200, 0x0102ff) AM_READWRITE8(cache_status_register_r, cache_control_register_w, 0xffffffff )
		AM_RANGE(0x010300, 0x0103ff) AM_READWRITE8(task_alias_register_r , task_alias_register_w , 0xffffffff )
		AM_RANGE(0x010400, 0x0104ff) AM_DEVREADWRITE8(APOLLO_SIO_TAG, apollo_sio, read, write, 0xffffffff )
		AM_RANGE(0x010500, 0x0105ff) AM_DEVREADWRITE8(APOLLO_SIO2_TAG, mc68681_device, read, write, 0x00ff00ff )
		AM_RANGE(0x010800, 0x0108ff) AM_DEVREADWRITE8(APOLLO_PTM_TAG, ptm6840_device, read, write, 0x00ff00ff )
		AM_RANGE(0x010900, 0x0109ff) AM_READWRITE8(apollo_rtc_r, apollo_rtc_w, 0xffffffff )
		AM_RANGE(0x010c00, 0x010cff) AM_READWRITE8(/*"dma1",*/apollo_dma_1_r, apollo_dma_1_w, 0xffffffff )
		AM_RANGE(0x010d00, 0x010dff) AM_READWRITE8(/*"dma2",*/apollo_dma_2_r, apollo_dma_2_w, 0xffffffff )
		AM_RANGE(0x011000, 0x0110ff) AM_DEVREADWRITE8(APOLLO_PIC1_TAG, pic8259_device, read, write, 0xffffffff)
		AM_RANGE(0x011100, 0x0111ff) AM_DEVREADWRITE8(APOLLO_PIC2_TAG, pic8259_device, read, write, 0xffffffff)
		AM_RANGE(0x011200, 0x0112ff) AM_DEVREADWRITE16(APOLLO_NI_TAG, apollo_ni, read, write, 0xffffffff)
		AM_RANGE(0x011300, 0x0113ff) AM_READWRITE16(latch_page_on_parity_error_register_r, latch_page_on_parity_error_register_w, 0xffffffff )
		AM_RANGE(0x011600, 0x0116ff) AM_READWRITE8(master_req_register_r, master_req_register_w, 0xffffffff)

		AM_RANGE(0x016400, 0x0164ff) AM_READWRITE16(selective_clear_locations_r, selective_clear_locations_w, 0xffffffff )
		AM_RANGE(0x017000, 0x017fff) AM_READWRITE16(apollo_address_translation_map_r, apollo_address_translation_map_w, 0xffffffff )

		AM_RANGE(0x05d800, 0x05dc07) AM_DEVREADWRITE8(APOLLO_SCREEN_TAG, apollo_graphics_15i, apollo_mcr_r, apollo_mcr_w, 0xffffffff)
		AM_RANGE(0xfa0000, 0xfdffff) AM_DEVREADWRITE16(APOLLO_SCREEN_TAG, apollo_graphics_15i, apollo_mgm_r, apollo_mgm_w, 0xffffffff)

		AM_RANGE(0x05e800, 0x05ec07) AM_DEVREADWRITE8(APOLLO_SCREEN_TAG, apollo_graphics_15i, apollo_ccr_r, apollo_ccr_w, 0xffffffff)
		AM_RANGE(0x0a0000, 0x0bffff) AM_DEVREADWRITE16(APOLLO_SCREEN_TAG, apollo_graphics_15i, apollo_cgm_r, apollo_cgm_w, 0xffffffff)

		AM_RANGE(ATBUS_IO_BASE, ATBUS_IO_END) AM_READWRITE16(apollo_atbus_io_r, apollo_atbus_io_w, 0xffffffff)

		// FIXME: must match with RAM size in driver/apollo_sio.c
		// AM_RANGE(DN3500_RAM_BASE, DN3500_RAM_END) AM_RAM /* 8MB RAM */
		AM_RANGE(DN3500_RAM_BASE, DN3500_RAM_END) AM_RAM_WRITE(ram_with_parity_w) AM_SHARE("messram")

		AM_RANGE(ATBUS_MEMORY_BASE, ATBUS_MEMORY_END) AM_READWRITE16(apollo_atbus_memory_r, apollo_atbus_memory_w, 0xffffffff)

//      AM_RANGE(0x03020000, 0x0303ffff) Cache Tag Store (DN4500 only)
//      AM_RANGE(0x04000000, 0x0400ffff) Cache Tag Data (DN4500 only)
//      AM_RANGE(0x0e000000, 0x0fffffff) FPA address space

//      AM_RANGE(0xf8000000, 0xffffffff) AM_READWRITE(apollo_f8_r, apollo_f8_w)
		AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(apollo_unmapped_r, apollo_unmapped_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(dsp3500_map, AS_PROGRAM, 32, apollo_state )
		AM_RANGE(0x000000, 0x00ffff) AM_ROM /* boot ROM  */
		AM_RANGE(0x000000, 0x00ffff) AM_WRITE(apollo_rom_w)
		AM_RANGE(0x010000, 0x0100ff) AM_READWRITE16(apollo_csr_status_register_r, apollo_csr_status_register_w, 0xffffffff)
		AM_RANGE(0x010100, 0x0101ff) AM_READWRITE16(apollo_csr_control_register_r, apollo_csr_control_register_w, 0xffffffff)
		AM_RANGE(0x010200, 0x0102ff) AM_READWRITE8(cache_status_register_r, cache_control_register_w, 0xffffffff )
		AM_RANGE(0x010300, 0x0103ff) AM_READWRITE8(task_alias_register_r , task_alias_register_w , 0xffffffff )
		AM_RANGE(0x010400, 0x0104ff) AM_DEVREADWRITE8(APOLLO_SIO_TAG, apollo_sio, read, write, 0xffffffff )
		AM_RANGE(0x010500, 0x0105ff) AM_DEVREADWRITE8(APOLLO_SIO2_TAG, mc68681_device, read, write, 0x00ff00ff )
		AM_RANGE(0x010800, 0x0108ff) AM_DEVREADWRITE8(APOLLO_PTM_TAG, ptm6840_device, read, write, 0x00ff00ff )
		AM_RANGE(0x010900, 0x0109ff) AM_READWRITE8(apollo_rtc_r, apollo_rtc_w, 0xffffffff )
		AM_RANGE(0x010c00, 0x010cff) AM_READWRITE8(/*"dma1",*/apollo_dma_1_r, apollo_dma_1_w, 0xffffffff )
		AM_RANGE(0x010d00, 0x010dff) AM_READWRITE8(/*"dma2",*/apollo_dma_2_r, apollo_dma_2_w, 0xffffffff )
		AM_RANGE(0x011000, 0x0110ff) AM_DEVREADWRITE8(APOLLO_PIC1_TAG, pic8259_device, read, write, 0xffffffff)
		AM_RANGE(0x011100, 0x0111ff) AM_DEVREADWRITE8(APOLLO_PIC2_TAG, pic8259_device, read, write, 0xffffffff)
		AM_RANGE(0x011200, 0x0112ff) AM_DEVREADWRITE16(APOLLO_NI_TAG, apollo_ni, read, write, 0xffffffff)
		AM_RANGE(0x011300, 0x0113ff) AM_READWRITE16(latch_page_on_parity_error_register_r, latch_page_on_parity_error_register_w, 0xffffffff )
		AM_RANGE(0x011600, 0x0116ff) AM_READWRITE8(master_req_register_r, master_req_register_w, 0xffffffff)

		AM_RANGE(0x016400, 0x0164ff) AM_READWRITE16(selective_clear_locations_r, selective_clear_locations_w, 0xffffffff )
		AM_RANGE(0x017000, 0x017fff) AM_READWRITE16(apollo_address_translation_map_r, apollo_address_translation_map_w, 0xffffffff )

		AM_RANGE(ATBUS_IO_BASE, ATBUS_IO_END) AM_READWRITE16(apollo_atbus_io_r, apollo_atbus_io_w, 0xffffffff)

		AM_RANGE(DN3500_RAM_BASE, DN3500_RAM_END) AM_RAM_WRITE(ram_with_parity_w) AM_SHARE("messram")

		AM_RANGE(ATBUS_MEMORY_BASE, ATBUS_MEMORY_END) AM_READWRITE16(apollo_atbus_memory_r, apollo_atbus_memory_w, 0xffffffff)

//      AM_RANGE(0xf8000000, 0xffffffff) AM_READWRITE(apollo_f8_r, apollo_f8_w)
		AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(apollo_unmapped_r, apollo_unmapped_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(dn3000_map, AS_PROGRAM, 32, apollo_state )
		AM_RANGE(0x000000, 0x007fff) AM_ROM /* boot ROM  */
		AM_RANGE(0x000000, 0x007fff) AM_WRITE(apollo_rom_w)
		AM_RANGE(0x008000, 0x0080ff) AM_READWRITE16(apollo_csr_status_register_r, apollo_csr_status_register_w, 0xffffffff)
		AM_RANGE(0x008100, 0x0081ff) AM_READWRITE16(apollo_csr_control_register_r, apollo_csr_control_register_w, 0xffffffff)
		AM_RANGE(0x008400, 0x0087ff) AM_DEVREADWRITE8(APOLLO_SIO_TAG, apollo_sio, read, write, 0xffffffff )
		AM_RANGE(0x008800, 0x0088ff) AM_DEVREADWRITE8(APOLLO_PTM_TAG, ptm6840_device, read, write, 0x00ff00ff )
		AM_RANGE(0x008900, 0x0089ff) AM_READWRITE8(apollo_rtc_r, apollo_rtc_w, 0xffffffff )
		AM_RANGE(0x009000, 0x0090ff) AM_READWRITE8(/*"dma1",*/apollo_dma_1_r, apollo_dma_1_w, 0xffffffff )
		AM_RANGE(0x009100, 0x0091ff) AM_READWRITE8(/*"dma2",*/apollo_dma_2_r, apollo_dma_2_w, 0xffffffff )
		AM_RANGE(0x009200, 0x0092ff) AM_READWRITE8(apollo_dma_page_register_r, apollo_dma_page_register_w, 0xffffffff )
		AM_RANGE(0x009300, 0x0093ff) AM_READWRITE16(latch_page_on_parity_error_register_r, latch_page_on_parity_error_register_w, 0xffffffff )
		AM_RANGE(0x009400, 0x0094ff) AM_DEVREADWRITE8(APOLLO_PIC1_TAG, pic8259_device, read, write, 0xffffffff)
		AM_RANGE(0x009500, 0x0095ff) AM_DEVREADWRITE8(APOLLO_PIC2_TAG, pic8259_device, read, write, 0xffffffff)
		AM_RANGE(0x009600, 0x0096ff) AM_DEVREADWRITE16(APOLLO_NI_TAG, apollo_ni, read, write, 0xffffffff)

		AM_RANGE(0x05d800, 0x05dc07) AM_DEVREADWRITE8(APOLLO_SCREEN_TAG, apollo_graphics_15i, apollo_mcr_r, apollo_mcr_w, 0xffffffff)
		AM_RANGE(0xfa0000, 0xfdffff) AM_DEVREADWRITE16(APOLLO_SCREEN_TAG, apollo_graphics_15i, apollo_mgm_r, apollo_mgm_w, 0xffffffff)

		AM_RANGE(0x05e800, 0x05ec07) AM_DEVREADWRITE8(APOLLO_SCREEN_TAG, apollo_graphics_15i, apollo_ccr_r, apollo_ccr_w, 0xffffffff)
		AM_RANGE(0x0a0000, 0x0bffff) AM_DEVREADWRITE16(APOLLO_SCREEN_TAG, apollo_graphics_15i, apollo_cgm_r, apollo_cgm_w, 0xffffffff)

		AM_RANGE(ATBUS_IO_BASE, ATBUS_IO_END) AM_READWRITE16(apollo_atbus_io_r, apollo_atbus_io_w, 0xffffffff)

		// FIXME: must match with RAM size in driver/apollo_sio.c
		// AM_RANGE(DN3000_RAM_BASE, DN3000_RAM_END) AM_RAM  /* 8MB RAM */
		AM_RANGE(DN3000_RAM_BASE, DN3000_RAM_END) AM_RAM_WRITE(ram_with_parity_w) AM_SHARE("messram")

		AM_RANGE(ATBUS_MEMORY_BASE, ATBUS_MEMORY_END) AM_READWRITE16(apollo_atbus_memory_r, apollo_atbus_memory_w, 0xffffffff)

		AM_RANGE(0x000000, 0xffffff) AM_READWRITE(apollo_unmapped_r, apollo_unmapped_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(dsp3000_map, AS_PROGRAM, 32, apollo_state )
		AM_RANGE(0x000000, 0x007fff) AM_ROM /* boot ROM  */
		AM_RANGE(0x000000, 0x007fff) AM_WRITE(apollo_rom_w)
		AM_RANGE(0x008000, 0x0080ff) AM_READWRITE16(apollo_csr_status_register_r, apollo_csr_status_register_w, 0xffffffff)
		AM_RANGE(0x008100, 0x0081ff) AM_READWRITE16(apollo_csr_control_register_r, apollo_csr_control_register_w, 0xffffffff)
		AM_RANGE(0x008400, 0x0087ff) AM_DEVREADWRITE8(APOLLO_SIO_TAG, apollo_sio, read, write, 0xffffffff )
		AM_RANGE(0x008800, 0x0088ff) AM_DEVREADWRITE8(APOLLO_PTM_TAG, ptm6840_device, read, write, 0x00ff00ff )
		AM_RANGE(0x008900, 0x0089ff) AM_READWRITE8(apollo_rtc_r, apollo_rtc_w, 0xffffffff )

		AM_RANGE(0x009000, 0x0090ff) AM_READWRITE8(/*"dma1",*/apollo_dma_1_r, apollo_dma_1_w, 0xffffffff )
		AM_RANGE(0x009100, 0x0091ff) AM_READWRITE8(/*"dma2",*/apollo_dma_2_r, apollo_dma_2_w, 0xffffffff )
		AM_RANGE(0x009200, 0x0092ff) AM_READWRITE8(apollo_dma_page_register_r, apollo_dma_page_register_w, 0xffffffff )
		AM_RANGE(0x009300, 0x0093ff) AM_READWRITE16(latch_page_on_parity_error_register_r, latch_page_on_parity_error_register_w, 0xffffffff )
		AM_RANGE(0x009400, 0x0094ff) AM_DEVREADWRITE8(APOLLO_PIC1_TAG, pic8259_device, read, write, 0xffffffff)
		AM_RANGE(0x009500, 0x0095ff) AM_DEVREADWRITE8(APOLLO_PIC2_TAG, pic8259_device, read, write, 0xffffffff)
		AM_RANGE(0x009600, 0x0096ff) AM_DEVREADWRITE16(APOLLO_NI_TAG, apollo_ni, read, write, 0xffffffff)

		AM_RANGE(ATBUS_IO_BASE, ATBUS_IO_END) AM_READWRITE16(apollo_atbus_io_r, apollo_atbus_io_w, 0xffffffff)

		// FIXME: must match with RAM size in driver/apollo_sio.c
		// AM_RANGE(DN3000_RAM_BASE, DN3000_RAM_END) AM_RAM  /* 8MB RAM */
		AM_RANGE(DN3000_RAM_BASE, DN3000_RAM_END) AM_RAM_WRITE(ram_with_parity_w) AM_SHARE("messram")

		AM_RANGE(ATBUS_MEMORY_BASE, ATBUS_MEMORY_END) AM_READWRITE16(apollo_atbus_memory_r, apollo_atbus_memory_w, 0xffffffff)

		AM_RANGE(0x000000, 0xffffff) AM_READWRITE(apollo_unmapped_r, apollo_unmapped_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START(dn5500_map, AS_PROGRAM, 32, apollo_state )
		AM_RANGE(0x000000, 0x00ffff) AM_ROM /* boot ROM  */
		AM_RANGE(0x000000, 0x00ffff) AM_WRITE(apollo_rom_w)
		AM_RANGE(0x010000, 0x0100ff) AM_READWRITE16(apollo_csr_status_register_r, apollo_csr_status_register_w, 0xffffffff)
		AM_RANGE(0x010100, 0x0101ff) AM_READWRITE16(apollo_csr_control_register_r, apollo_csr_control_register_w, 0xffffffff)
		AM_RANGE(0x010200, 0x0102ff) AM_READWRITE8(cache_status_register_r, cache_control_register_w, 0xffffffff )
		AM_RANGE(0x010300, 0x0103ff) AM_READWRITE8(task_alias_register_r , task_alias_register_w , 0xffffffff )
		AM_RANGE(0x010400, 0x0104ff) AM_DEVREADWRITE8(APOLLO_SIO_TAG, apollo_sio, read, write, 0xffffffff )
		AM_RANGE(0x010500, 0x0105ff) AM_DEVREADWRITE8(APOLLO_SIO2_TAG, mc68681_device, read, write, 0x00ff00ff )
		AM_RANGE(0x010800, 0x0108ff) AM_DEVREADWRITE8(APOLLO_PTM_TAG, ptm6840_device, read, write, 0x00ff00ff )
		AM_RANGE(0x010900, 0x0109ff) AM_READWRITE8(apollo_rtc_r, apollo_rtc_w, 0xffffffff )
		AM_RANGE(0x010c00, 0x010cff) AM_READWRITE8(/*"dma1",*/apollo_dma_1_r, apollo_dma_1_w, 0xffffffff )
		AM_RANGE(0x010d00, 0x010dff) AM_READWRITE8(/*"dma2",*/apollo_dma_2_r, apollo_dma_2_w, 0xffffffff )
		AM_RANGE(0x011000, 0x0110ff) AM_DEVREADWRITE8(APOLLO_PIC1_TAG, pic8259_device, read, write, 0xffffffff)
		AM_RANGE(0x011100, 0x0111ff) AM_DEVREADWRITE8(APOLLO_PIC2_TAG, pic8259_device, read, write, 0xffffffff)
		AM_RANGE(0x011200, 0x0112ff) AM_DEVREADWRITE16(APOLLO_NI_TAG, apollo_ni, read, write, 0xffffffff)
		AM_RANGE(0x011300, 0x0113ff) AM_READWRITE16(latch_page_on_parity_error_register_r, latch_page_on_parity_error_register_w, 0xffffffff )
		AM_RANGE(0x011400, 0x0114ff) AM_READWRITE8(dn5500_memory_present_register_r, dn5500_memory_present_register_w, 0xffffffff )
		AM_RANGE(0x011500, 0x0115ff) AM_READWRITE8(dn5500_11500_r, dn5500_11500_w, 0xffffffff )
		AM_RANGE(0x011600, 0x0116ff) AM_READWRITE8(master_req_register_r, master_req_register_w, 0xffffffff)

		AM_RANGE(0x016400, 0x0164ff) AM_READWRITE16(selective_clear_locations_r, selective_clear_locations_w, 0xffffffff )
		AM_RANGE(0x017000, 0x017fff) AM_READWRITE16(apollo_address_translation_map_r, apollo_address_translation_map_w, 0xffffffff )

		AM_RANGE(0x05d800, 0x05dc07) AM_DEVREADWRITE8(APOLLO_SCREEN_TAG, apollo_graphics_15i, apollo_mcr_r, apollo_mcr_w, 0xffffffff)
		AM_RANGE(0xfa0000, 0xfdffff) AM_DEVREADWRITE16(APOLLO_SCREEN_TAG, apollo_graphics_15i, apollo_mgm_r, apollo_mgm_w, 0xffffffff)

		AM_RANGE(0x05e800, 0x05ec07) AM_DEVREADWRITE8(APOLLO_SCREEN_TAG, apollo_graphics_15i, apollo_ccr_r, apollo_ccr_w, 0xffffffff)
		AM_RANGE(0x0a0000, 0x0bffff) AM_DEVREADWRITE16(APOLLO_SCREEN_TAG, apollo_graphics_15i, apollo_cgm_r, apollo_cgm_w, 0xffffffff)

		AM_RANGE(ATBUS_IO_BASE, ATBUS_IO_END) AM_READWRITE16(apollo_atbus_io_r, apollo_atbus_io_w, 0xffffffff)

		// FIXME: must match with RAM size in driver/apollo_sio.c
		// AM_RANGE(DN3500_RAM_BASE, DN3500_RAM_END) AM_RAM  /* 8MB RAM */
		AM_RANGE(DN5500_RAM_BASE, DN5500_RAM_END) AM_RAM_WRITE(ram_with_parity_w) AM_SHARE("messram")

		AM_RANGE(ATBUS_MEMORY_BASE, ATBUS_MEMORY_END) AM_READWRITE16(apollo_atbus_memory_r, apollo_atbus_memory_w, 0xffffffff)

//      AM_RANGE(0x03020000, 0x0303ffff) Cache Tag Store (DN4500 only)
//      AM_RANGE(0x04000000, 0x0400ffff) Cache Tag Data (DN4500 only)
		AM_RANGE(0x07000000, 0x0700FFFF) AM_READWRITE8(dn5500_io_protection_map_r, dn5500_io_protection_map_w, 0xffffffff )
//      AM_RANGE(0x0e000000, 0x0fffffff) FPA address space

//      AM_RANGE(0xf8000000, 0xffffffff) AM_READWRITE(apollo_f8_r, apollo_f8_w)
		AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(apollo_unmapped_r, apollo_unmapped_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(dsp5500_map, AS_PROGRAM, 32, apollo_state )
		AM_RANGE(0x000000, 0x00ffff) AM_ROM /* boot ROM  */
		AM_RANGE(0x000000, 0x00ffff) AM_WRITE(apollo_rom_w)
		AM_RANGE(0x010000, 0x0100ff) AM_READWRITE16(apollo_csr_status_register_r, apollo_csr_status_register_w, 0xffffffff)
		AM_RANGE(0x010100, 0x0101ff) AM_READWRITE16(apollo_csr_control_register_r, apollo_csr_control_register_w, 0xffffffff)
		AM_RANGE(0x010200, 0x0102ff) AM_READWRITE8(cache_status_register_r, cache_control_register_w, 0xffffffff )
		AM_RANGE(0x010300, 0x0103ff) AM_READWRITE8(task_alias_register_r , task_alias_register_w , 0xffffffff )
		AM_RANGE(0x010400, 0x0104ff) AM_DEVREADWRITE8(APOLLO_SIO_TAG, apollo_sio, read, write, 0xffffffff )
		AM_RANGE(0x010500, 0x0105ff) AM_DEVREADWRITE8(APOLLO_SIO2_TAG, mc68681_device, read, write, 0x00ff00ff )
		AM_RANGE(0x010800, 0x0108ff) AM_DEVREADWRITE8(APOLLO_PTM_TAG, ptm6840_device, read, write, 0x00ff00ff )
		AM_RANGE(0x010900, 0x0109ff) AM_READWRITE8(apollo_rtc_r, apollo_rtc_w, 0xffffffff )
		AM_RANGE(0x010c00, 0x010cff) AM_READWRITE8(/*"dma1",*/apollo_dma_1_r, apollo_dma_1_w, 0xffffffff )
		AM_RANGE(0x010d00, 0x010dff) AM_READWRITE8(/*"dma2",*/apollo_dma_2_r, apollo_dma_2_w, 0xffffffff )
		AM_RANGE(0x011000, 0x0110ff) AM_DEVREADWRITE8(APOLLO_PIC1_TAG, pic8259_device, read, write, 0xffffffff)
		AM_RANGE(0x011100, 0x0111ff) AM_DEVREADWRITE8(APOLLO_PIC2_TAG, pic8259_device, read, write, 0xffffffff)
		AM_RANGE(0x011200, 0x0112ff) AM_DEVREADWRITE16(APOLLO_NI_TAG, apollo_ni, read, write, 0xffffffff)
		AM_RANGE(0x011300, 0x0113ff) AM_READWRITE16(latch_page_on_parity_error_register_r, latch_page_on_parity_error_register_w, 0xffffffff )
		AM_RANGE(0x011400, 0x0114ff) AM_READWRITE8(dn5500_memory_present_register_r, dn5500_memory_present_register_w, 0xffffffff )
		AM_RANGE(0x011500, 0x0115ff) AM_READWRITE8(dn5500_11500_r, dn5500_11500_w, 0xffffffff )
		AM_RANGE(0x011600, 0x0116ff) AM_READWRITE8(master_req_register_r, master_req_register_w, 0xffffffff)

		AM_RANGE(0x016400, 0x0164ff) AM_READWRITE16(selective_clear_locations_r, selective_clear_locations_w, 0xffffffff )
		AM_RANGE(0x017000, 0x017fff) AM_READWRITE16(apollo_address_translation_map_r, apollo_address_translation_map_w, 0xffffffff )

		AM_RANGE(ATBUS_IO_BASE, ATBUS_IO_END) AM_READWRITE16(apollo_atbus_io_r, apollo_atbus_io_w, 0xffffffff)

		// FIXME: must match with RAM size in driver/apollo_sio.c
		AM_RANGE(DN5500_RAM_BASE, DN5500_RAM_END) AM_RAM_WRITE(ram_with_parity_w) AM_SHARE("messram")

		AM_RANGE(ATBUS_MEMORY_BASE, ATBUS_MEMORY_END) AM_READWRITE16(apollo_atbus_memory_r, apollo_atbus_memory_w, 0xffffffff)

		AM_RANGE(0x07000000, 0x0700FFFF) AM_READWRITE8(dn5500_io_protection_map_r, dn5500_io_protection_map_w, 0xffffffff )
//      AM_RANGE(0xf8000000, 0xffffffff) AM_READWRITE(apollo_f8_r, apollo_f8_w)
		AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(apollo_unmapped_r, apollo_unmapped_w)
ADDRESS_MAP_END

/***************************************************************************
 Machine Reset
 ***************************************************************************/

void apollo_state::machine_reset()
{
	MLOG1(("machine_reset"));

	MACHINE_RESET_CALL_MEMBER(apollo);

#ifdef APOLLO_XXL
	// set configuration
	omti8621_device::set_verbose(apollo_config(APOLLO_CONF_DISK_TRACE));
	threecom3c505_device::set_verbose(apollo_config(APOLLO_CONF_NET_TRACE));

	if (apollo_config(APOLLO_CONF_NODE_ID))
	{
		// set node ID from UID of logical volume 1 of logical unit 0
		m_node_id->set_node_id_from_disk();
	}
#endif

	m_maincpu->set_instruction_hook(read32_delegate(FUNC(apollo_state::apollo_instruction_hook),this));
}

WRITE_LINE_MEMBER(apollo_state::apollo_reset_instr_callback)
{
	MLOG1(("apollo_reset_instr_callback"));

	// reset the CPU board devices
	MACHINE_RESET_CALL_MEMBER(apollo);

	// reset the ISA bus devices
	m_isa->reset();

	if (!apollo_is_dsp3x00())
	{
		machine().device(APOLLO_SCREEN_TAG)->reset();
		machine().device(APOLLO_KBD_TAG )->reset();
#ifdef APOLLO_XXL
		machine().device(APOLLO_STDIO_TAG )->reset();
#endif
	}
}

/***************************************************************************
 Machine Start
 ***************************************************************************/

void apollo_state::machine_start(){
	memory_share *messram = memshare("messram");
	//MLOG1(("machine_start_dn3500: ram size is %d MB", (int)messram->bytes()/(1024*1024)));

	// clear ram
	memset(messram->ptr(), 0x55, messram->bytes());

	MACHINE_START_CALL_MEMBER(apollo);

	// install nop handlers for unmapped ISA bus addresses
	m_isa->install16_device(0, ATBUS_IO_END, 0, 0, read16_delegate(FUNC(apollo_state::apollo_atbus_unmap_io_r), this), write16_delegate(FUNC(apollo_state::apollo_atbus_unmap_io_w), this));
	m_isa->install_memory(0, ATBUS_MEMORY_END, 0, 0, read8_delegate(FUNC(apollo_state::apollo_atbus_unmap_r), this), write8_delegate(FUNC(apollo_state::apollo_atbus_unmap_w), this));
}

/***************************************************************************
 Driver Init
 ***************************************************************************/

DRIVER_INIT_MEMBER(apollo_state,dn3500)
{
//  MLOG1(("driver_init_dn3500"));

	/* hook the RESET line, which resets a slew of other components */
	m_maincpu->set_reset_callback(write_line_delegate(FUNC(apollo_state::apollo_reset_instr_callback),this));

	ram_base_address = DN3500_RAM_BASE;
	ram_end_address = DN3500_RAM_END;

	node_type=  NODE_TYPE_DN3500;
	ram_config_byte= DN3500_RAM_CONFIG_BYTE;

	DRIVER_INIT_CALL(apollo);
}

DRIVER_INIT_MEMBER(apollo_state,dsp3500)
{
	DRIVER_INIT_CALL( dn3500 );
//  MLOG1(("driver_init_dsp3500"));
	node_type = NODE_TYPE_DSP3500;
}

DRIVER_INIT_MEMBER(apollo_state,dn3000)
{
	DRIVER_INIT_CALL( dn3500 );
//  MLOG1(("driver_init_dn3000"));

	ram_base_address = DN3000_RAM_BASE;
	ram_end_address = DN3000_RAM_END;

	node_type = NODE_TYPE_DN3000;
	ram_config_byte= DN3000_RAM_CONFIG_8MB;
}

DRIVER_INIT_MEMBER(apollo_state,dsp3000)
{
	DRIVER_INIT_CALL( dn3000 );
//  MLOG1(("driver_init_dsp3000"));
	node_type = NODE_TYPE_DSP3000;
}

DRIVER_INIT_MEMBER(apollo_state,dn5500)
{
	DRIVER_INIT_CALL( dn3500 );
//  MLOG1(("driver_init_dn5500"));

	ram_base_address = DN5500_RAM_BASE;
	ram_end_address = DN5500_RAM_END;

	node_type = NODE_TYPE_DN5500;
	ram_config_byte= DN5500_RAM_CONFIG_BYTE;
}

DRIVER_INIT_MEMBER(apollo_state,dsp5500)
{
	DRIVER_INIT_CALL( dn5500 );
//  MLOG1(("driver_init_dsp5500"));
	node_type = NODE_TYPE_DSP5500;
}

/***************************************************************************
 Input Ports
 ***************************************************************************/

static INPUT_PORTS_START( dn3500 )
	PORT_INCLUDE(apollo_config)
	PORT_INCLUDE(apollo_kbd)
INPUT_PORTS_END

static INPUT_PORTS_START( dsp3500 )
	PORT_INCLUDE(apollo_config)
INPUT_PORTS_END

READ_LINE_MEMBER( apollo_state::apollo_kbd_is_german )
{
	return (apollo_config(APOLLO_CONF_GERMAN_KBD) != 0) ? ASSERT_LINE : CLEAR_LINE;
}

/***************************************************************************
 MACHINE DRIVERS
 ***************************************************************************/

static MACHINE_CONFIG_START( dn3500, apollo_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU, M68030, 25000000) /* 25 MHz 68030 */
	MCFG_CPU_PROGRAM_MAP(dn3500_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(apollo_state,apollo_irq_acknowledge)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_FRAGMENT_ADD( apollo )

	/* keyboard beeper */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beep", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* internal ram */
	MCFG_RAM_ADD("messram")
	MCFG_RAM_DEFAULT_SIZE("8M")
	MCFG_RAM_EXTRA_OPTIONS("4M,8M,16M,32M")

#ifdef APOLLO_XXL
	MCFG_DEVICE_ADD(APOLLO_STDIO_TAG, APOLLO_STDIO, 0)
	MCFG_APOLLO_STDIO_TX_CALLBACK(DEVWRITELINE(APOLLO_SIO_TAG, apollo_sio, rx_b_w))
#endif
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( dsp3500, apollo_state )
	MCFG_CPU_ADD(MAINCPU, M68030, 25000000) /* 25 MHz 68030 */
	MCFG_CPU_PROGRAM_MAP(dsp3500_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(apollo_state,apollo_irq_acknowledge)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_FRAGMENT_ADD( apollo_terminal )

	/* keyboard beeper */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beep", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* internal ram */
	MCFG_RAM_ADD("messram")
	MCFG_RAM_DEFAULT_SIZE("8M")
	MCFG_RAM_EXTRA_OPTIONS("4M,8M,16M,32M")

	/* terminal hardware */
	MCFG_DEFAULT_LAYOUT( layout_apollo_dsp )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dn3500_19i, dn3500 )
	/* video hardware 19" monochrome */
	MCFG_APOLLO_MONO19I_ADD(APOLLO_SCREEN_TAG)
	MCFG_DEVICE_ADD(APOLLO_KBD_TAG, APOLLO_KBD, 0)
	MCFG_APOLLO_KBD_TX_CALLBACK(DEVWRITELINE(APOLLO_SIO_TAG, apollo_sio, rx_a_w))
	MCFG_APOLLO_KBD_GERMAN_CALLBACK(READLINE(apollo_state, apollo_kbd_is_german))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dn3500_15i, dn3500 )
	/* video hardware is 15" monochrome or color */
	MCFG_APOLLO_GRAPHICS_ADD(APOLLO_SCREEN_TAG)
	MCFG_DEVICE_ADD(APOLLO_KBD_TAG, APOLLO_KBD, 0)
	MCFG_APOLLO_KBD_TX_CALLBACK(DEVWRITELINE(APOLLO_SIO_TAG, apollo_sio, rx_a_w))
	MCFG_APOLLO_KBD_GERMAN_CALLBACK(READLINE(apollo_state, apollo_kbd_is_german))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dn3000, dn3500 )
	MCFG_CPU_REPLACE(MAINCPU, M68020PMMU, 12000000) /* 12 MHz */
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(apollo_state,apollo_irq_acknowledge)
	MCFG_CPU_PROGRAM_MAP(dn3000_map)
	MCFG_DEVICE_REMOVE( APOLLO_SIO2_TAG )
	MCFG_RAM_MODIFY("messram")
	MCFG_RAM_DEFAULT_SIZE("8M")
	MCFG_RAM_EXTRA_OPTIONS("4M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( dsp3000, apollo_state )
	MCFG_CPU_ADD(MAINCPU, M68020PMMU, 12000000) /* 12 MHz */
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(apollo_state,apollo_irq_acknowledge)
	MCFG_CPU_PROGRAM_MAP(dsp3000_map)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_FRAGMENT_ADD( apollo_terminal )

	/* keyboard beeper */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beep", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* internal ram */
	MCFG_RAM_ADD("messram")
	MCFG_RAM_DEFAULT_SIZE("8M")
	MCFG_RAM_EXTRA_OPTIONS("4M")

	MCFG_DEVICE_REMOVE( APOLLO_SIO2_TAG )
	MCFG_RAM_MODIFY("messram")

	/* terminal hardware */
	MCFG_DEFAULT_LAYOUT( layout_apollo_dsp )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dn3000_19i, dn3000 )
	/* video hardware 19" monochrome */
	MCFG_APOLLO_MONO19I_ADD(APOLLO_SCREEN_TAG)
	MCFG_DEVICE_ADD(APOLLO_KBD_TAG, APOLLO_KBD, 0)
	MCFG_APOLLO_KBD_TX_CALLBACK(DEVWRITELINE(APOLLO_SIO_TAG, apollo_sio, rx_a_w))
	MCFG_APOLLO_KBD_GERMAN_CALLBACK(READLINE(apollo_state, apollo_kbd_is_german))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dn3000_15i, dn3000 )
	/* video hardware 15" monochrome */
	MCFG_APOLLO_GRAPHICS_ADD(APOLLO_SCREEN_TAG)
	MCFG_DEVICE_ADD(APOLLO_KBD_TAG, APOLLO_KBD, 0)
	MCFG_APOLLO_KBD_TX_CALLBACK(DEVWRITELINE(APOLLO_SIO_TAG, apollo_sio, rx_a_w))
	MCFG_APOLLO_KBD_GERMAN_CALLBACK(READLINE(apollo_state, apollo_kbd_is_german))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dn5500, dn3500 )
	MCFG_CPU_REPLACE(MAINCPU, M68040, 25000000) /* 25 MHz */
	MCFG_CPU_PROGRAM_MAP(dn5500_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( dsp5500, apollo_state )
	MCFG_CPU_ADD(MAINCPU, M68040, 25000000) /* 25 MHz */
	MCFG_CPU_PROGRAM_MAP(dsp5500_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(apollo_state,apollo_irq_acknowledge)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_FRAGMENT_ADD(apollo_terminal)

	/* keyboard beeper */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beep", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* terminal hardware */
	MCFG_DEFAULT_LAYOUT( layout_apollo_dsp )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dn5500_19i, dn5500 )
	/* video hardware 19" monochrome */
	MCFG_APOLLO_MONO19I_ADD(APOLLO_SCREEN_TAG)
	MCFG_DEVICE_ADD(APOLLO_KBD_TAG, APOLLO_KBD, 0)
	MCFG_APOLLO_KBD_TX_CALLBACK(DEVWRITELINE(APOLLO_SIO_TAG, apollo_sio, rx_a_w))
	MCFG_APOLLO_KBD_GERMAN_CALLBACK(READLINE(apollo_state, apollo_kbd_is_german))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dn5500_15i, dn5500 )
	/* video hardware 15" monochrome */
	MCFG_APOLLO_GRAPHICS_ADD(APOLLO_SCREEN_TAG)
	MCFG_DEVICE_ADD(APOLLO_KBD_TAG, APOLLO_KBD, 0)
	MCFG_APOLLO_KBD_TX_CALLBACK(DEVWRITELINE(APOLLO_SIO_TAG, apollo_sio, rx_a_w))
	MCFG_APOLLO_KBD_GERMAN_CALLBACK(READLINE(apollo_state, apollo_kbd_is_german))
MACHINE_CONFIG_END

/***************************************************************************
 ROM Definitions
 ***************************************************************************/

ROM_START( dn3500 )
	// dn3500 boot ROM (Note: use sha1sum -b <rom file>)
	ROM_REGION( 0x0100000, MAINCPU, 0 ) /* 68000 code */

	// http://www.bitsavers.org/bits/Apollo/firmware/3500_BOOT_12191_7.bin
	// Note: file name must be converted to lower case (i.e. 3500_boot_12191_7.bin)
	// Note: this duplicates boot rom md7c-rev-8.00-1989-08-16-17-23-52.bin
	ROM_SYSTEM_BIOS( 0, "md7c-rev-8.00", "MD7C REV 8.00, 1989/08/16.17:23:52" )
	ROMX_LOAD( "3500_boot_12191_7.bin", 0x00000, 0x10000, CRC(3132067d) SHA1(36f3c83d9f2df42f2537b09ca2f051a8c9dfbfc2) , ROM_BIOS(1) )
ROM_END

ROM_START( dn5500 )
	// dn5500 boot ROM (Note: use sha1sum -b <rom file>)
	ROM_REGION( 0x0100000, MAINCPU, 0 ) /* 68000 code */

	ROM_SYSTEM_BIOS( 0, "md7c-rev-8.00", "MD7C REV 8.00, 1989/08/16.17:23:52" )
	ROMX_LOAD( "5500_boot_a1631-80046_1-30-92.bin", 0x00000, 0x10000, CRC(7b9ed610) SHA1(7315a884ec4551c44433c6079cc06509223cb02b) , ROM_BIOS(1) )
ROM_END

ROM_START( dn3000)
	ROM_REGION( 0x0090000, MAINCPU, 0 ) /* 68000 code */

	ROM_SYSTEM_BIOS( 0, "md8-rev-7.0", "MD8 REV 7.0, 1988/08/16.15:14:39" )
	ROMX_LOAD( "3000_boot_8475_7.bin",  0x00000, 0x08000, CRC(0fe2d471) SHA1(6c383d2266719a3d069b7bf015f6945179395e7a), ROM_BIOS(1) )
ROM_END

#define rom_dsp3500    rom_dn3500
#define rom_dn3500_15i rom_dn3500
#define rom_dn3500_19i rom_dn3500

#define rom_dsp3000    rom_dn3000
#define rom_dn3000_19i rom_dn3000

#define rom_dsp5500    rom_dn5500
#define rom_dn5500_15i rom_dn5500
#define rom_dn5500_19i rom_dn5500

/***************************************************************************
    GAME DRIVERS
 ***************************************************************************/

#define DN_FLAGS 0
#define DSP_FLAGS 0
//#define DSP_FLAGS MACHINE_NO_SOUND

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT   INIT    COMPANY     FULLNAME                         FLAGS */
COMP( 1989, dn3500,          0, 0,      dn3500_15i, dn3500, apollo_state, dn3500, "Apollo",   "Apollo DN3500", DN_FLAGS )
COMP( 1989, dsp3500,    dn3500, 0,      dsp3500,    dsp3500, apollo_state,dsp3500,"Apollo",   "Apollo DSP3500",                DSP_FLAGS )
COMP( 1989, dn3500_19i, dn3500, 0,      dn3500_19i, dn3500, apollo_state, dn3500, "Apollo",   "Apollo DN3500 19\" Monochrome", DN_FLAGS )

COMP( 1988, dn3000,     dn3500, 0,      dn3000_15i, dn3500, apollo_state, dn3000, "Apollo",   "Apollo DN3000", DN_FLAGS )
COMP( 1988, dsp3000,    dn3500, 0,      dsp3000,    dsp3500, apollo_state,dsp3000,"Apollo",   "Apollo DSP3000",                DSP_FLAGS )
COMP( 1988, dn3000_19i, dn3500, 0,      dn3000_19i, dn3500, apollo_state, dn3000, "Apollo",   "Apollo DN3000 19\" Monochrome", DN_FLAGS )

COMP( 1991, dn5500,     dn3500, 0,      dn5500_15i, dn3500, apollo_state, dn5500, "Apollo",   "Apollo DN5500", MACHINE_NOT_WORKING )
COMP( 1991, dsp5500,    dn3500, 0,      dsp5500,    dsp3500, apollo_state,dsp5500,"Apollo",   "Apollo DSP5500",                MACHINE_NOT_WORKING )
COMP( 1991, dn5500_19i, dn3500, 0,      dn5500_19i, dn3500, apollo_state, dn5500, "Apollo",   "Apollo DN5500 19\" Monochrome", MACHINE_NOT_WORKING )
