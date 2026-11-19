// license:BSD-3-Clause
// copyright-holders:Chris Toshok
/*
 * apollo_dn300_mmu.cpp - the Apollo DN300/DN320 memory management unit
 *
 * Apollo's own MMU: a page translation table (PTT) indexed by the low ten
 * bits of the virtual page number, and a page frame table (PFT) with one
 * entry per physical page, chained into hash threads.
 *
 *  see also:
 *  - http://www.bitsavers.org/pdf/apollo/002398-04_Domain_Engineering_Handbook_Rev4_Jan87.pdf
 */

#include "emu.h"
#include "apollo_dn300_mmu.h"

#define LOG_REGS  (1U << 1) // register accesses
#define LOG_TABLE (1U << 2) // PTT writes
#define LOG_FAULT (1U << 3) // translation faults

#define VERBOSE (0)
#include "logmacro.h"

#define LOGREGS(...)  LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGTABLE(...) LOGMASKED(LOG_TABLE, __VA_ARGS__)
#define LOGFAULT(...) LOGMASKED(LOG_FAULT, __VA_ARGS__)

#define STATUS_MMU_ENABLED        (1 << 0)
#define STATUS_PTT_ACCESS_ENABLED (1 << 1)
// 1 << 2 is unused
#define STATUS_INTERRUPT_PENDING  (1 << 3)
#define STATUS_NORMAL_MODE		  (1 << 4)
#define STATUS_BUS_TIMEOUT        (1 << 5)
#define STATUS_PAGE_FAULT         (1 << 6)
#define STATUS_ACCESS_VIOLATION   (1 << 7)

DEFINE_DEVICE_TYPE(APOLLO_DN300_MMU, apollo_dn300_mmu_device, "apollo_dn300_mmu", "Apollo DN300 MMU")

apollo_dn300_mmu_device::apollo_dn300_mmu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	device_t(mconfig, APOLLO_DN300_MMU, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_physical_config("physical", ENDIANNESS_BIG, 16, 24, 0),
	m_fault_pending(false),
	m_fault_cycles(0),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_status(0),
	m_enabled(0),
	m_asid(0)
{
}

device_memory_interface::space_config_vector apollo_dn300_mmu_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_physical_config) };
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apollo_dn300_mmu_device::device_start()
{
	space(0).specific(m_physical);

	m_pft = std::make_unique<uint16_t[]>(8192);
	m_ptt = std::make_unique<uint16_t[]>(1024);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apollo_dn300_mmu_device::device_reset()
{
	m_enabled = 0;
	m_asid = 0;
	m_fault_pending = false;
}

#define PAGE_SIZE 1024
#define PAGE_SIZE_OFFSET 10

// PAGE FRAME TABLE ENTRY (PFTE), two words per physical page:
//  bits 31-25  ELSID     address space ID
//  bit  24     ELACCESS  supervisor domain
//  bit  23     DOMAIN
//  bit  22     W         write access
//  bit  21     R         read access
//  bit  20     X         execute access
//  bits 19-16  XSVPN     excess virtual page number
//  bit  15     EOC       end of the hash thread chain
//  bit  14     BBMOD     page is modified
//  bit  13     USED      page is referenced
//  bit  12     GLOBAL    page is global
//  bits 11-0   LINK      the next PFTE of the hash thread

offs_t apollo_dn300_mmu_device::translate(offs_t byte_offset, int intention) {
	// from Domain Engineering Handbook:
	// PAGE TRANSLATION TABLE ENTRY (PTTE)
	// (type "ppn_t" in base.ins.pas)
	// 15                    0
	// +---------------------+
	// | XXXX PPPP PPPP PPPP |
	// +---------------------+
	// P..P - Physical page number (PPN)
	// XXX  - Junk - ignore
	// Page Translation Table at [ n/a | 700000]
	// through [ n/a | 800000 ]
	// One PPTE every 1024 bytes in table.
	int vpn = byte_offset >> PAGE_SIZE_OFFSET;

	if ((m_status & STATUS_PTT_ACCESS_ENABLED)
		 && (byte_offset >= 0x700000 && byte_offset < 0x800000)) {
		return byte_offset;
	 }


	// split up the vpn again on the 10 bit boundary into a ptt index and an "excess vpn" (xsvpn)
	int ptt_index = vpn & (PAGE_SIZE-1);
	int xsvpn = vpn >> PAGE_SIZE_OFFSET;

	int byte_offset_within_page = byte_offset % PAGE_SIZE;
	int ppn = m_ptt[ptt_index] & 0x0fff; /* upper nibble = Junk - ignore */

	int cur_ppn = ppn;
	bool seen_eoc = false;
	while (true) {
		uint32_t pfte = m_pft[cur_ppn * 2] << 16 | m_pft[cur_ppn * 2 + 1];

		int pfte_link =   (pfte >> 0)  & 0xfff;
		int pfte_global = (pfte >> 12) & 0x1;
		int pfte_eoc =    (pfte >> 15) & 0x1;
		int pfte_xsvpn =  (pfte >> 16) & 0xf;
		int pfte_elsid =  (pfte >> 25);

		if (pfte_eoc) {
			if (seen_eoc) {
				// if we're at the eoc twice, we've looped and didn't find a matching xsvpn.
				// this should be a bus error.
				break;
			}
			seen_eoc = true;
		}

		if (
			// ppn and xsvpn match
			pfte_xsvpn == xsvpn && (
				// the page is either global or the asid matches
				pfte_global || pfte_elsid == m_asid
			)) {
			// we found a match.

			// (debugger accesses look, but change nothing)
			if (!machine().side_effects_disabled()) {
				// update the PTT to point to this PFT entry first so we'll hit it faster next time
				m_ptt[ptt_index] = cur_ppn;

				// marked it as accessed
				m_pft[cur_ppn * 2 + 1] |= 1 << 13;

				// and if we're writing, mark it as modified
				if (intention & device_memory_interface::TR_WRITE) {
					m_pft[cur_ppn * 2 + 1] |= 1 << 14;
				}
			}

			// return the translated address
			return (cur_ppn << 10) | byte_offset_within_page;
		}

		cur_ppn = pfte_link;
	}


	if (!machine().side_effects_disabled()) {
		LOGFAULT("%s: no translation for %06x (asid %d, ppn %d, xsvpn %d, ptt index %d)\n",
				machine().describe_context(), byte_offset, m_asid, ppn, xsvpn, ptt_index);
		// when we can't find a physical page, add this bit (Page fault) to our
		// status so the FIM can tell why we bus errored.
		m_status |= STATUS_PAGE_FAULT | STATUS_ACCESS_VIOLATION;
		// interrupt pending too?

		// the rest of the instruction must not write (see write_suppressed).
		// A fault while the core fetches the opcode (the word at the
		// instruction's PC, then the prefetch of the word after it) needs no
		// suppression: the core then skips the instruction's handler, and the
		// bus error frame is pushed without the cycle count moving.
		if (((byte_offset - m_cpu->pcbase()) & 0xffffff) > 2) {
			m_fault_pending = true;
			m_fault_cycles = m_cpu->total_cycles();
		}

		// TODO: TR_READ is 0, so this always reports a write
		m_cpu->set_buserror_details(byte_offset, (intention & device_memory_interface::TR_READ) ? 1 : 0, m_cpu->get_fc(), true);
	}
	return 0;
}

//-------------------------------------------------
//  write_suppressed - true while the current
//  instruction has a pending rerun bus error
//-------------------------------------------------

// The 68010 aborts an instruction at the faulting bus cycle and reruns it
// after the page fault is serviced.  The core raises the bus error only after
// the instruction's handler has finished, so a write that follows the fault
// in the same instruction (e.g. after a fault on an extension-word fetch,
// which leaves the handler a bogus operand) must not reach the bus.  The
// fault belongs to the current instruction while the CPU's cycle count has
// not moved: the core charges the instruction's cycles after its handler,
// before it pushes the bus error frame, so the frame's writes go through.
bool apollo_dn300_mmu_device::write_suppressed()
{
	if (!m_fault_pending)
		return false;
	if (m_cpu->total_cycles() == m_fault_cycles)
		return true;
	m_fault_pending = false;
	return false;
}

//-------------------------------------------------
//  virtual space: every CPU bus cycle
//-------------------------------------------------

void apollo_dn300_mmu_device::virtual_map(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(apollo_dn300_mmu_device::virtual_r), FUNC(apollo_dn300_mmu_device::virtual_w));
}

// A 32-bit access is two 16-bit cycles on the 68010's bus, so each word is
// translated on its own.  Byte cycles report the byte's own address on a
// fault (the mask says which half of the word is accessed).
static inline offs_t virtual_byte_address(offs_t offset, uint16_t mem_mask)
{
	return (offset << 1) | (mem_mask == 0x00ff ? 1 : 0);
}

uint16_t apollo_dn300_mmu_device::virtual_r(offs_t offset, uint16_t mem_mask)
{
	offs_t address = virtual_byte_address(offset, mem_mask);
	if (m_enabled)
		address = translate(address, device_memory_interface::TR_READ);
	return m_physical.read_word(address & ~1, mem_mask);
}

void apollo_dn300_mmu_device::virtual_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!machine().side_effects_disabled() && write_suppressed())
		return;                 // instruction already faulted: no side effects
	offs_t address = virtual_byte_address(offset, mem_mask);
	if (m_enabled)
		address = translate(address, device_memory_interface::TR_WRITE);
	m_physical.write_word(address & ~1, data, mem_mask);
}

void apollo_dn300_mmu_device::ptt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!(m_status & STATUS_PTT_ACCESS_ENABLED)) {
		// ptt access disabled
		return;
	}

	// values exist only on even 1024 byte multiples, so 512 word multiples
	offs_t ptt_offset = offset;
	if (ptt_offset % 512 != 0) {
		return;
	}

	LOGTABLE("writing PTT at offset %02x [index=%d] = %04x & %04x\n", offset, offset/512, data, mem_mask);
	m_ptt[offset/512] = data & mem_mask;
}

uint16_t apollo_dn300_mmu_device::ptt_r(offs_t offset, uint16_t mem_mask)
{
	// values exist only on even 1024 byte multiples, so 512 word multiples
	offs_t ptt_offset = offset;
	if (ptt_offset % 512 != 0) {
		return 0;
	}

	return m_ptt[offset/512] & mem_mask;
}

void apollo_dn300_mmu_device::pft_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_pft[offset] = data & mem_mask;
}

uint16_t apollo_dn300_mmu_device::pft_r(offs_t offset, uint16_t mem_mask)
{
	return m_pft[offset] & mem_mask;
}

void apollo_dn300_mmu_device::unk_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGREGS("UNKNOWN MMU WRITE at offset %02x = %04x & %04x\n", offset, data, mem_mask);
}

uint16_t apollo_dn300_mmu_device::unk_r(offs_t offset, uint16_t mem_mask)
{
	LOGREGS("UNKNOWN MMU READ at offset %04x & %04x\n", offset, mem_mask);

	return 0;
}

void apollo_dn300_mmu_device::pid_priv_power_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGREGS("writing PID/PRIV/POWER at offset %02x = %04x & %04x\n", offset, data, mem_mask);
	COMBINE_DATA(&m_pid_priv_power);

	bool was_enabled = m_enabled;

	m_enabled = m_pid_priv_power & 0x01;
	m_asid = m_pid_priv_power >> 8;
	m_domain = (m_pid_priv_power >> 2) & 0x01;

	bool ptt_access_enabled = m_pid_priv_power & 0x02;
	LOGREGS("mmu enabled? %s.  asid %d.  domain %d.  ptt access? %s.\n",
			m_enabled ? "yes" : "no",
			m_asid,
			m_domain,
			ptt_access_enabled ? "yes" : "no");

	if (!m_enabled && was_enabled) {
		// switching the MMU off resets the status register (the SR10.2
		// CPT1 diagnostic tests this)
		m_status = 0;
	}

	if (m_enabled) {
		m_status |= STATUS_MMU_ENABLED;
	} else {
		m_enabled &= ~STATUS_MMU_ENABLED;
	}

	if (ptt_access_enabled) {
		m_status |= STATUS_PTT_ACCESS_ENABLED;
	} else {
		m_status &= ~STATUS_PTT_ACCESS_ENABLED;
	}
	m_cpu->set_emmu_enable(m_enabled);
}
uint16_t apollo_dn300_mmu_device::pid_priv_power_r(offs_t offset, uint16_t mem_mask)
{
	// clear the lower 2 bits on read (PTT enabled / MMU enabled - both are write-only, and their state is read from the status reg)
	return m_pid_priv_power & 0xfffc & mem_mask;
}

void apollo_dn300_mmu_device::status_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	LOGREGS("status_w offset %02x = %02x & %02x\n", offset, data, mem_mask);
	// EH87 says any write to register clears bits 5-7.
	m_status &= ~(STATUS_ACCESS_VIOLATION | STATUS_PAGE_FAULT | STATUS_BUS_TIMEOUT);
}

uint8_t apollo_dn300_mmu_device::status_r(offs_t offset, uint8_t mem_mask)
{
	LOGREGS("status_r %02x\n", m_status);
	return m_status;
}

void apollo_dn300_mmu_device::fpu_owner_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	LOGREGS("fpu_owner_w offset %02x = %02x & %02x\n", offset, data, mem_mask);
	m_fpu_owner = data;
}

uint8_t apollo_dn300_mmu_device::fpu_owner_r(offs_t offset, uint8_t mem_mask)
{
	LOGREGS("fpu_owner_r %02x\n", m_fpu_owner);
	return m_fpu_owner;
}
