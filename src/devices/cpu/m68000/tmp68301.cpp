// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "tmp68301.h"

// Not implemented, for lack of test cases:
// - parallel other than gpio
// - serial flow control
// - serial parity

DEFINE_DEVICE_TYPE(TMP68301, tmp68301_device, "tmp68301", "Toshiba TMP68301")
DEFINE_DEVICE_TYPE(TMP68303, tmp68303_device, "tmp68303", "Toshiba TMP68303")

tmp68301_device::tmp68301_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	m68000_mcu_device(mconfig, type, tag, owner, clock),
	m_parallel_r_cb(*this, 0xffff),
	m_parallel_w_cb(*this),
	m_tx_cb(*this)
{
	auto m = address_map_constructor(FUNC(tmp68301_device::internal_map), this);
	m_program_config.m_internal_map = m;
	m_opcodes_config.m_internal_map = m;
	m_uprogram_config.m_internal_map = m;
	m_uopcodes_config.m_internal_map = m;
	m_cpu_space_config.m_internal_map = address_map_constructor(FUNC(tmp68301_device::cpu_space_map), this);

	m_serial_external_clock = 0;
}

tmp68301_device::tmp68301_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tmp68301_device(mconfig, TMP68301, tag, owner, clock)
{
}


void tmp68301_device::internal_update(uint64_t current_time)
{
	// If current_time == 0, only compute the next event time

	uint64_t event_time = 0;

	// Serial
	for(int i=0; i != 3; i++) {
		// Calling update changes the next event time
		if(m_serial_tx_next_event[i] && current_time >= m_serial_tx_next_event[i])
			serial_tx_update(i);
		if(m_serial_rx_next_event[i] && current_time >= m_serial_rx_next_event[i])
			serial_rx_update(i);

		if(m_serial_tx_next_event[i] && (!event_time || m_serial_tx_next_event[i] < event_time))
			event_time = m_serial_tx_next_event[i];
		if(m_serial_rx_next_event[i] && (!event_time || m_serial_rx_next_event[i] < event_time))
			event_time = m_serial_rx_next_event[i];
	}

	// Timers
	for(int i=0; i != 3; i++) {
		// Calling update changes the next event time
		if(m_timer_next_event[i] && current_time >= m_timer_next_event[i])
			timer_update(i);

		if(m_timer_next_event[i] && (!event_time || m_timer_next_event[i] < event_time))
			event_time = m_timer_next_event[i];
	}

	recompute_bcount(event_time);
}

void tmp68301_device::device_start()
{
	m68000_mcu_device::device_start();

	save_item(NAME(m_external_interrupt_state));

	save_item(NAME(m_amar));
	save_item(NAME(m_aamr));
	save_item(NAME(m_aacr));
	save_item(NAME(m_ator));
	save_item(NAME(m_arelr));

	save_item(NAME(m_icr));
	save_item(NAME(m_imr));
	save_item(NAME(m_ipr));
	save_item(NAME(m_iisr));
	save_item(NAME(m_ivnr));
	save_item(NAME(m_ieir));
	save_item(NAME(m_interrupt_state));

	save_item(NAME(m_pdir));
	save_item(NAME(m_pcr));
	save_item(NAME(m_parallel_mode));
	save_item(NAME(m_psr));
	save_item(NAME(m_pcmr));
	save_item(NAME(m_pmr));
	save_item(NAME(m_ppr1));
	save_item(NAME(m_ppr2));

	save_item(NAME(m_scr));
	save_item(NAME(m_spr));
	save_item(NAME(m_smr));
	save_item(NAME(m_scmr));
	save_item(NAME(m_sbrr));
	save_item(NAME(m_ssr));
	save_item(NAME(m_sdrr));
	save_item(NAME(m_sdrt));
	save_item(NAME(m_serial_tx_next_event));
	save_item(NAME(m_serial_rx_next_event));
	save_item(NAME(m_serial_tx_state));
	save_item(NAME(m_serial_rx_state));
	save_item(NAME(m_serial_tx));
	save_item(NAME(m_serial_rx));
	save_item(NAME(m_serial_rx_line));
	save_item(NAME(m_serial_gclk));

	save_item(NAME(m_tcr));
	save_item(NAME(m_tmcr1));
	save_item(NAME(m_tmcr2));
	save_item(NAME(m_tctr));
	save_item(NAME(m_timer_next_event));
	save_item(NAME(m_timer_last_sync));
}

void tmp68301_device::device_reset()
{
	m68000_mcu_device::device_reset();
	m_external_interrupt_state = 0;

	m_amar[0] = m_amar[1] = 0x00;
	m_aamr[0] = m_aamr[1] = 0xff;
	m_aacr[0] = 0x3d;
	m_aacr[1] = 0x18;
	m_aacr[2] = 0x18;
	m_ator    = 0x08;
	m_arelr   = 0xfffc;
	show_cs(0);
	show_cs(1);

	for(int i=0; i != 10; i++)
		m_icr[i] = 0x07;
	m_imr = 0x7f7;
	m_ipr = 0x000;
	m_iisr = 0x000;
	m_ivnr = 0x00;
	m_ieir = 0x00;
	m_interrupt_state = 0;

	m_pdir = 0x0000;
	m_pcr = 0x00;
	m_parallel_mode = parallel_mode_table[0];
	m_psr = 0x40;
	m_pcmr = 0x00;
	m_pmr = 0x00;
	m_ppr1 = 0x00;
	m_ppr2 = 0x00;

	m_scr = SCR_CKSE|SCR_RES|SCR_INTM;
	m_spr = 0x00;

	for(int i=0; i != 3; i++) {
		m_smr[i] = SMR_RXINTM|SMR_ERINTM|SMR_TXINTM;
		m_scmr[i] = SCMR_ERS;
		m_sbrr[i] = 0x01;
		m_ssr[i] = SSR_TXE;
		m_sdrr[i] = 0x00;
		m_sdrt[i] = 0x00;
		m_sr_intr[i] = 0;
		m_serial_tx_next_event[i] = 0;
		m_serial_rx_next_event[i] = 0;
		m_serial_tx_state[i] = SR_IDLE;
		m_serial_rx_state[i] = SR_IDLE;
		m_serial_tx[i] = 0x00;
		m_serial_rx[i] = 0x00;
		m_serial_rx_line[i] = 0;
	}

	for(int i=0; i != 3; i++) {
		m_tcr[i] = i ? 0x0012 : 0x0052;
		m_tmcr1[i] = 0x0000;
		m_tmcr2[i] = 0x0000;
		m_tctr[i] = 0x0000;
		m_timer_next_event[i] = 0;
		m_timer_last_sync[i] = 0;
	}
}

void tmp68301_device::internal_map(address_map &map)
{
	map(0xfffc00, 0xffffff).unmaprw();

	// Address decoder
	map(0xfffc00, 0xfffc00).rw(FUNC(tmp68301_device::amar0_r), FUNC(tmp68301_device::amar0_w));
	map(0xfffc01, 0xfffc01).rw(FUNC(tmp68301_device::aamr0_r), FUNC(tmp68301_device::aamr0_w));
	map(0xfffc03, 0xfffc03).rw(FUNC(tmp68301_device::aacr0_r), FUNC(tmp68301_device::aacr0_w));
	map(0xfffc04, 0xfffc04).rw(FUNC(tmp68301_device::amar1_r), FUNC(tmp68301_device::amar1_w));
	map(0xfffc05, 0xfffc05).rw(FUNC(tmp68301_device::aamr1_r), FUNC(tmp68301_device::aamr1_w));
	map(0xfffc07, 0xfffc07).rw(FUNC(tmp68301_device::aacr1_r), FUNC(tmp68301_device::aacr1_w));
	map(0xfffc09, 0xfffc09).rw(FUNC(tmp68301_device::aacr2_r), FUNC(tmp68301_device::aacr2_w));
	map(0xfffc0b, 0xfffc0b).rw(FUNC(tmp68301_device::ator_r),  FUNC(tmp68301_device::ator_w));
	map(0xfffc0c, 0xfffc0d).rw(FUNC(tmp68301_device::arelr_r), FUNC(tmp68301_device::arelr_w));

	// Interrupt controller
	map(0xfffc80, 0xfffc93).rw(FUNC(tmp68301_device::icr_r),   FUNC(tmp68301_device::icr_w)).umask16(0x00ff);
	map(0xfffc94, 0xfffc95).rw(FUNC(tmp68301_device::imr_r),   FUNC(tmp68301_device::imr_w));
	map(0xfffc96, 0xfffc97).rw(FUNC(tmp68301_device::ipr_r),   FUNC(tmp68301_device::ipr_w));
	map(0xfffc98, 0xfffc99).rw(FUNC(tmp68301_device::iisr_r),  FUNC(tmp68301_device::iisr_w));
	map(0xfffc9b, 0xfffc9b).rw(FUNC(tmp68301_device::ivnr_r),  FUNC(tmp68301_device::ivnr_w));
	map(0xfffc9d, 0xfffc9d).rw(FUNC(tmp68301_device::ivnr_r),  FUNC(tmp68301_device::ieir_w));

	// Parallel interface
	map(0xfffd00, 0xfffd01).rw(FUNC(tmp68301_device::pdir_r),  FUNC(tmp68301_device::pdir_w));
	map(0xfffd03, 0xfffd03).rw(FUNC(tmp68301_device::pcr_r),   FUNC(tmp68301_device::pcr_w));
	map(0xfffd05, 0xfffd05).rw(FUNC(tmp68301_device::psr_r),   FUNC(tmp68301_device::psr_w));
	map(0xfffd07, 0xfffd07).rw(FUNC(tmp68301_device::pcmr_r),  FUNC(tmp68301_device::pcmr_w));
	map(0xfffd08, 0xfffd09).rw(FUNC(tmp68301_device::pmr_r),   FUNC(tmp68301_device::pmr_w));
	map(0xfffd0a, 0xfffd0b).rw(FUNC(tmp68301_device::pdr_r),   FUNC(tmp68301_device::pdr_w));
	map(0xfffd0d, 0xfffd0d).rw(FUNC(tmp68301_device::ppr1_r),  FUNC(tmp68301_device::ppr1_w));
	map(0xfffd0f, 0xfffd0f).rw(FUNC(tmp68301_device::ppr2_r),  FUNC(tmp68301_device::ppr2_w));

	// Serial interface
	map(0xfffd81, 0xfffd81).rw(FUNC(tmp68301_device::smr0_r),  FUNC(tmp68301_device::smr0_w));
	map(0xfffd83, 0xfffd83).rw(FUNC(tmp68301_device::scmr0_r), FUNC(tmp68301_device::scmr0_w));
	map(0xfffd85, 0xfffd85).rw(FUNC(tmp68301_device::sbrr0_r), FUNC(tmp68301_device::sbrr0_w));
	map(0xfffd87, 0xfffd87).r (FUNC(tmp68301_device::ssr0_r));
	map(0xfffd89, 0xfffd89).rw(FUNC(tmp68301_device::sdr0_r),  FUNC(tmp68301_device::sdr0_w));
	map(0xfffd8d, 0xfffd8d).rw(FUNC(tmp68301_device::spr_r),  FUNC(tmp68301_device::spr_w));
	map(0xfffd8f, 0xfffd8f).rw(FUNC(tmp68301_device::scr_r),  FUNC(tmp68301_device::scr_w));
	map(0xfffd91, 0xfffd91).rw(FUNC(tmp68301_device::smr1_r),  FUNC(tmp68301_device::smr1_w));
	map(0xfffd93, 0xfffd93).rw(FUNC(tmp68301_device::scmr1_r), FUNC(tmp68301_device::scmr1_w));
	map(0xfffd95, 0xfffd95).rw(FUNC(tmp68301_device::sbrr1_r), FUNC(tmp68301_device::sbrr1_w));
	map(0xfffd97, 0xfffd97).r (FUNC(tmp68301_device::ssr1_r));
	map(0xfffd99, 0xfffd99).rw(FUNC(tmp68301_device::sdr1_r),  FUNC(tmp68301_device::sdr1_w));
	map(0xfffda1, 0xfffda1).rw(FUNC(tmp68301_device::smr2_r),  FUNC(tmp68301_device::smr2_w));
	map(0xfffda3, 0xfffda3).rw(FUNC(tmp68301_device::scmr2_r), FUNC(tmp68301_device::scmr2_w));
	map(0xfffda5, 0xfffda5).rw(FUNC(tmp68301_device::sbrr2_r), FUNC(tmp68301_device::sbrr2_w));
	map(0xfffda7, 0xfffda7).r (FUNC(tmp68301_device::ssr2_r));
	map(0xfffda9, 0xfffda9).rw(FUNC(tmp68301_device::sdr2_r),  FUNC(tmp68301_device::sdr2_w));

	// 16-bit timer
	map(0xfffe00, 0xfffe01).rw(FUNC(tmp68301_device::tcr0_r),  FUNC(tmp68301_device::tcr0_w));
	map(0xfffe04, 0xfffe05).rw(FUNC(tmp68301_device::tmcr01_r),  FUNC(tmp68301_device::tmcr01_w));
	map(0xfffe0c, 0xfffe0d).rw(FUNC(tmp68301_device::tctr0_r),  FUNC(tmp68301_device::tctr0_w));
	map(0xfffe20, 0xfffe21).rw(FUNC(tmp68301_device::tcr1_r),  FUNC(tmp68301_device::tcr1_w));
	map(0xfffe24, 0xfffe25).rw(FUNC(tmp68301_device::tmcr11_r),  FUNC(tmp68301_device::tmcr11_w));
	map(0xfffe28, 0xfffe29).rw(FUNC(tmp68301_device::tmcr12_r),  FUNC(tmp68301_device::tmcr12_w));
	map(0xfffe2c, 0xfffe2d).rw(FUNC(tmp68301_device::tctr1_r),  FUNC(tmp68301_device::tctr1_w));
	map(0xfffe40, 0xfffe41).rw(FUNC(tmp68301_device::tcr1_r),  FUNC(tmp68301_device::tcr2_w));
	map(0xfffe44, 0xfffe45).rw(FUNC(tmp68301_device::tmcr21_r),  FUNC(tmp68301_device::tmcr21_w));
	map(0xfffe48, 0xfffe49).rw(FUNC(tmp68301_device::tmcr22_r),  FUNC(tmp68301_device::tmcr22_w));
	map(0xfffe4c, 0xfffe4d).rw(FUNC(tmp68301_device::tctr2_r),  FUNC(tmp68301_device::tctr2_w));
}

void tmp68301_device::cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).r(FUNC(tmp68301_device::interrupt_callback)).umask16(0x00ff);
}


// Address decoder management

u8 tmp68301_device::amar0_r() { return m_amar[0]; }
u8 tmp68301_device::amar1_r() { return m_amar[1]; }
void tmp68301_device::amar0_w(u8 data) { amar_w(0, data); }
void tmp68301_device::amar1_w(u8 data) { amar_w(1, data); }
u8 tmp68301_device::aamr0_r() { return m_aamr[0]; }
u8 tmp68301_device::aamr1_r() { return m_aamr[1]; }
void tmp68301_device::aamr0_w(u8 data) { aamr_w(0, data); }
void tmp68301_device::aamr1_w(u8 data) { aamr_w(1, data); }

u8 tmp68301_device::aacr0_r() { return m_aacr[0]; }
u8 tmp68301_device::aacr1_r() { return m_aacr[1]; }
u8 tmp68301_device::aacr2_r() { return m_aacr[2]; }
void tmp68301_device::aacr0_w(u8 data) { aacr_w(0, data); }
void tmp68301_device::aacr1_w(u8 data) { aacr_w(1, data); }
void tmp68301_device::aacr2_w(u8 data) { aacr_w(2, data); }

u8 tmp68301_device::ator_r() { return m_ator; }
u16 tmp68301_device::arelr_r() { return m_arelr; }

void tmp68301_device::show_cs(int cs)
{
	static const char *const dtack_modes[4] = { "?", "internal", "external", "both" };
	u32 base = m_amar[cs] << 16;
	u32 mask = (m_aamr[cs] & 0xfc) << 14;
	if(m_aamr[cs] & 0x02)
		mask |= 0x00fe00;
	if(m_aamr[cs] & 0x01)
		mask |= 0x000100;
	mask |= 0x0000ff;
	logerror("cs%d: %06x - %06x %s dtack=%s wait=%d\n",
			 cs,
			 base, base | mask,
			 m_aacr[cs] & 0x20 ? "enabled" : "disabled",
			 dtack_modes[(m_aacr[cs] >> 3) & 3],
			 m_aacr[cs] & 7);
}

void tmp68301_device::show_iack()
{
	static const char *const dtack_modes[4] = { "?", "internal", "external", "both" };
	logerror("iack: dtack=%s wait=%d\n",
			 dtack_modes[(m_aacr[2] >> 3) & 3],
			 m_aacr[2] & 7);
}

void tmp68301_device::amar_w(int reg, u8 data)
{
	if(m_amar[reg] == data)
		return;
	m_amar[reg] = data;
	show_cs(reg);
}

void tmp68301_device::aamr_w(int reg, u8 data)
{
	if(m_aamr[reg] == data)
		return;
	m_aamr[reg] = data;
	show_cs(reg);
}

void tmp68301_device::aacr_w(int reg, u8 data)
{
	if(reg < 2) {
		data &= 0x3f;
		if(m_aacr[reg] == data)
			return;
		m_aacr[reg] = data;
		show_cs(reg);

	} else {
		data &= 0x1f;
		if(m_aacr[2] == data)
			return;
		m_aacr[2] = data;
		show_iack();
	}
}

void tmp68301_device::ator_w(u8 data)
{
	data &= 0x0f;
	if(m_ator == data)
		return;
	m_ator = data;
	if(m_ator & 0x08)
		logerror("berr timeout: 256 clocks\n");
	else if(m_ator & 0x04)
		logerror("berr timeout: 128 clocks\n");
	else if(m_ator & 0x02)
		logerror("berr timeout: 64 clocks\n");
	else if(m_ator & 0x01)
		logerror("berr timeout: 32 clocks\n");
	else
		logerror("berr timeout: disabled\n");
}

void tmp68301_device::arelr_w(offs_t, u16 data, u16 mem_mask)
{
	u16 old = m_arelr;
	COMBINE_DATA(&m_arelr);
	m_arelr &= 0xfffc;
	if(m_arelr == old)
		return;
	logerror("registers remapping: %06x\n", m_arelr << 8);
}


// Interrupt controller management

const char *const tmp68301_device::interrupt_slot_names[11] = {
	"ext0", "ext1", "ext2", nullptr,
	"serial0", "serial1", "serial2", "parallel", "timer0", "timer1", "timer2"
};

const int tmp68301_device::interrupt_slot_to_priority[11] = { 9, 5, 0, -1, 7, 3, 2, 6, 8, 4, 1 };
const int tmp68301_device::interrupt_vector_to_slot[32] = {
	0, 1, 2, -1, 8, 9, 10, -1,
	4, 4, 4, 4, 5, 5, 5, 5,
	6, 6, 6, 6, 7, 7, 7, 7,
	-1, -1, -1, -1, -1, -1, -1, -1
};

u8 tmp68301_device::icr_r(offs_t reg) { return m_icr[reg]; }
u16 tmp68301_device::imr_r()  { return m_imr; }
u16 tmp68301_device::ipr_r()  { return m_ipr; }
u16 tmp68301_device::iisr_r() { return m_iisr; }
u8 tmp68301_device::ivnr_r()  { return m_ivnr; }
u8 tmp68301_device::ieir_r()  { return m_ieir; }


void tmp68301_device::icr_w(offs_t reg, u8 data)
{
	if(reg < 3) {
		static const char *const edge_mode[4] = { "falling edge", "level low", "rising edge", "level high" };
		data &= 0x1f;
		if(m_icr[reg] == data)
			return;
		m_icr[reg] = data;
		logerror("interrupt ext%d vector=%s %s intlevel=%d\n",
				 reg,
				 m_icr[reg] & 0x20 ? "automatic" : "external",
				 edge_mode[(m_icr[reg] >> 3) & 3],
				 m_icr[reg] & 7);

		// If switching to level and the pin is already on, raise the interrupt
		if((m_icr[reg] & 0x08) && (m_external_interrupt_state & (1 << reg)))
			m_ipr |= 1 << reg;

	} else {
		data &= 0x07;
		if(m_icr[reg] == data)
			return;
		m_icr[reg] = data;
		logerror("interrupt %s irqlevel=%d\n",
				 interrupt_slot_names[reg + 1],
				 m_icr[reg]);
	}
	interrupt_update();
}


void tmp68301_device::imr_w(offs_t, u16 data, u16 mem_mask)
{
	u16 old = m_imr;
	COMBINE_DATA(&m_imr);
	m_imr &= 0x07f7;
	if(m_imr == old)
		return;
	std::string s;
	for(int i=0; i != 11; i++)
		if(i != 3 && (!(m_imr & (1 << i))))
			s = s + ' ' + interrupt_slot_names[i];
	if(s.empty())
		s = " (none)";
	logerror("interrupt mask%s\n", s);
	interrupt_update();
}

void tmp68301_device::ipr_w(offs_t, u16 data, u16 mem_mask)
{
	u16 old = m_ipr;
	u16 n = m_ipr;
	COMBINE_DATA(&n);

	// Level external interrupts can't be cleared through that register
	if(m_icr[0] & 0x08)
		n |= 0x001;
	if(m_icr[1] & 0x08)
		n |= 0x002;
	if(m_icr[2] & 0x08)
		n |= 0x004;

	m_ipr &= n;
	if(m_ipr == old)
		return;
	// Not certain, but meh
	u32 vector_mask = 0;
	for(int i=0; i != 32; i++)
		if(interrupt_vector_to_slot[i] == -1 || !(m_ipr & (1 << interrupt_vector_to_slot[i])))
			vector_mask |= 1U << i;
	m_interrupt_state &= vector_mask;

	std::string s;
	for(int i=0; i != 11; i++)
		if(i != 3 && (m_ipr & (1 << i)))
			s = s + ' ' + interrupt_slot_names[i];
	if(s.empty())
		s = " (none)";
	logerror("interrupt pending%s\n", s);
	interrupt_update();
}

void tmp68301_device::iisr_w(offs_t, u16 data, u16 mem_mask)
{
	u16 old = m_iisr;
	u16 n = m_iisr;
	COMBINE_DATA(&n);
	m_iisr &= n;
	if(m_iisr == old)
		return;
	std::string s;
	for(int i=0; i != 11; i++)
		if(i != 3 && (m_iisr & (1 << i)))
			s = s + ' ' + interrupt_slot_names[i];
	if(s.empty())
		s = " (none)";
	//  logerror("interrupt in service%s\n", s);
}

void tmp68301_device::ivnr_w(u8 data)
{
	data &= 0xe0;
	if(m_ivnr == data)
		return;
	m_ivnr = data;
	logerror("interrupt base vector %02x\n", m_ivnr);
}

void tmp68301_device::ieir_w(u8 data)
{
	data &= 0x7f;
	if(m_ieir == data)
		return;
	m_ieir = data;
	std::string s;
	for(int i=0; i != 7; i++)
		if(!(m_ieir & (1 << i)))
			s = s + ' ' + interrupt_slot_names[i-4];
	if(s.empty())
		s = " (none)";
	logerror("interrupt external override%s\n", s);
}


void tmp68301_device::execute_set_input(int inputnum, int state)
{
	int prev_state = (m_external_interrupt_state >> inputnum) & 1;
	if(prev_state == state)
		return;
	m_external_interrupt_state ^= 1 << inputnum;

	if(inputnum < 3) {
		// Raising/falling is messed up with the ASSERT/CLEAR
		// logical concepts.  Only use level/edge.

		if(m_icr[inputnum] & 0x08) {
			// Level
			if(state) {
				m_ipr |= 1 << inputnum;
				m_interrupt_state |= 1U << inputnum;
			} else {
				m_ipr &= ~(1 << inputnum);
				m_interrupt_state &= ~(1U << inputnum);
			}
		} else {
			// Edge
			if(state) {
				m_ipr |= 1 << inputnum;
				m_interrupt_state |= 1U << inputnum;
			}
		}
	} else {
		// Only use the expanded interrupt if the ieir register allows it
		if(state && (m_ieir & (1 << (inputnum - 3))))
			m_ipr |= 1 << (inputnum + 1);
	}

	interrupt_update();
}


std::tuple<u32, u8, u32> tmp68301_device::interrupt_get_current() const
{
	u32 level = 0;
	u8 vector = 0x1f;
	u32 slot = 0;

	u16 active = m_ipr & ~m_imr;
	for(int vect=0; vect != 32; vect++) {
		int nslot = interrupt_vector_to_slot[vect];
		if(nslot != -1 && (m_interrupt_state & (1 << vect)) && (active & (1 << nslot))) {
			u32 nlevel = m_icr[nslot >= 3 ? nslot-1 : nslot] & 7;
			if(!nlevel || nlevel < level || (nlevel == level && interrupt_slot_to_priority[nslot] < interrupt_slot_to_priority[slot]))
				continue;
			level = nlevel;
			slot = nslot;
			vector = vect;
		}
	}
	return std::tie(level, vector, slot);
}

void tmp68301_device::interrupt_update()
{
	auto [level, vector, slot] = interrupt_get_current();
	set_current_interrupt_level(level);
	//  logerror("interrupt update ipr=%03x imr=%03x state=%08x (%x, %02x, %2d)\n", m_ipr, m_imr, m_interrupt_state, level, vector, slot);
}

void tmp68301_device::interrupt_internal_trigger(int vector)
{
	//  logerror("interrupt internal trigger %02x\n", vector);
	m_interrupt_state |= 1 << vector;
	m_ipr |= 1 << interrupt_vector_to_slot[vector];
	interrupt_update();
}

u8 tmp68301_device::interrupt_callback()
{
	auto [level, vector, slot] = interrupt_get_current();
	//  logerror("interrupt callback ipr=%03x imr=%03x (%x, %02x, %d)\n", m_ipr, m_imr, level, vector, slot);
	if(slot < 3)
		standard_irq_callback(slot, m_pc);
	if(vector != 0x1f) {
		m_iisr |= 1 << slot;
		if(slot >= 3 || !(m_icr[slot] & 0x08))
			m_ipr &= ~(1 << slot);
		m_interrupt_state &= ~(1U << vector);
		if(vector >= 8 && vector < 0x14 && !serial_validate_interrupt(vector))
			vector |= 3;
	}
	interrupt_update();
	return m_ivnr | vector;
}



// Parallel interface management

const int tmp68301_device::parallel_mode_table[16] = { 0, 2, 6, 6, 1, 3, 7, 7, 0, 4, 8, 8, 1, 5, 9, 9 };
const char *const tmp68301_device::parallel_mode_names[10] = { "0.0", "0.1", "1.0", "1.1", "1.2", "1.3", "2.0", "2.1", "2.2", "2.3" };

u8 tmp68301_device::parallel_get_interrupt() const
{
	return 0;
}

u16 tmp68301_device::pdir_r() { return m_pdir; }
u8 tmp68301_device::pcr_r()   { return m_pcr; }
u8 tmp68301_device::psr_r()   { return m_psr; }
u8 tmp68301_device::pcmr_r()  { return m_pcmr; }
u8 tmp68301_device::pmr_r()   { return m_pmr; }
u8 tmp68301_device::ppr1_r()  { return m_ppr1; }
u8 tmp68301_device::ppr2_r()  { return m_ppr2; }

void tmp68301_device::pdir_w(offs_t, u16 data, u16 mem_mask)
{
	u16 old = m_pdir;
	COMBINE_DATA(&m_pdir);
	if(m_pdir == old)
		return;
	logerror("parallel direction %04x\n", m_pdir);

	if(m_pdir)
		m_parallel_w_cb(m_pdr & m_pdir);
}

void tmp68301_device::pcr_w(u8 data)
{
	data &= 0x0f;
	if(m_pcr == data)
		return;
	m_pcr = data;
	m_parallel_mode = parallel_mode_table[m_pcr];
	logerror("parallel control mode %s\n", parallel_mode_names[m_parallel_mode]);
}

void tmp68301_device::psr_w(u8 data)
{
	data &= 0x80;
	// Datasheet unclear on whether the IF bit needs to be cleared
	logerror("parallel status write %02x\n", data);
}

void tmp68301_device::pcmr_w(u8 data)
{
	data &= 0x1f;
	if(m_pcmr == data)
		return;
	m_pcmr = data;
	logerror("parallel command complete=%s change=%s txready=%s fault=%c prime=%c\n",
			 m_pcmr & 0x10 ? "masked" : "interrupt",
			 m_pcmr & 0x08 ? "masked" : "interrupt",
			 m_pcmr & 0x04 ? "masked" : "interrupt",
			 m_pcmr & 0x02 ? '1' : '0',
			 m_pcmr & 0x01 ? '1' : '0');
}

void tmp68301_device::pmr_w(u8 data)
{
	data &= 0x0f;
	if(m_pmr == data)
		return;
	m_pmr = data;
	logerror("parallel mode dstb=%s ack=%s busy=%s transfer=%s\n",
			 m_pmr & 0x08 ? "falling" : "rising",
			 m_pmr & 0x04 ? "auto" : "manual",
			 m_pmr & 0x02 ? "ack" : "-",
			 m_pmr & 0x01 ? "external" : "internal");

}

u16 tmp68301_device::pdr_r()
{
	if(m_parallel_mode == 0 || m_parallel_mode == 1) {
		if(m_pdir == 0xffff)
			return m_pdr;
		return (m_pdr & m_pdir) | (m_parallel_r_cb() & ~m_pdir);
	}
	logerror("WARNING: parallel read access in non-gpio mode\n");
	return m_pdr;
}

void tmp68301_device::pdr_w(offs_t, u16 data, u16 mem_mask)
{
	u16 old = m_pdr;
	COMBINE_DATA(&m_pdr);
	if(m_pdr == old)
		return;
	//  logerror("parallel data %04x\n", m_pdr);
	if(m_parallel_mode == 0 || m_parallel_mode == 1) {
		if(m_pdir == 0x0000)
			return;
		m_parallel_w_cb(m_pdr & m_pdir);
		return;
	}
	logerror("WARNING: parallel write access in non-gpio mode\n");
}

void tmp68301_device::ppr1_w(u8 data)
{
	data &= 0x7f;
	if(m_ppr1 == data)
		return;
	m_ppr1 = data;
	logerror("parallel delay %02x\n", m_ppr1);
}

void tmp68301_device::ppr2_w(u8 data)
{
	data &= 0x7f;
	if(m_ppr2 == data)
		return;
	m_ppr2 = data;
	logerror("parallel width %02x\n", m_ppr2);
}




// Serial interface management

u8 tmp68301_device::sbrr_to_div(u8 value)
{
	// Zero all the bits under the top 1 bit, set to 1 if zero

	// Ensure all bits under the top bit are 1
	value |= value >> 1; // 0..01?..?    -> 0..011?..?
	value |= value >> 2; // 0..011?..?   -> 0..01111?..?
	value |= value >> 4; // 0..01111?..? -> 0..011111111

	// Ensure the bottom bit is 1 (to force 0 -> 1)
	value |= 0x01;

	// The byte is now of the form 0{n}1{8-n}, so clear the bottom bits to get 0{n}10{8-n-1}
	value -= value >> 1;
	return value;
}

bool tmp68301_device::serial_validate_interrupt(u8 vector) const
{
	int slot = vector & 3;
	int ch = (vector - 8) >> 2;
	if(m_sr_intr[ch] & 1 << slot)
		return true;
	return false;
}

void tmp68301_device::serial_check_interrupt(int ch)
{
	u8 old_interrupt = m_sr_intr[ch];
	u8 intr = 0;
	if(!(m_scr & SCR_INTM)) {
		if(!(m_smr[ch] & SMR_ERINTM) && (m_scmr[ch] & SCMR_RXEN) && (m_ssr[ch] & (SSR_PE | SSR_OE | SSR_FE)))
			intr |= 1 << SR_INT_ERR;
		if(!(m_smr[ch] & SMR_RXINTM) && (m_scmr[ch] & SCMR_RXEN) && (m_ssr[ch] & SSR_RXRDY))
			intr |= 1 << SR_INT_RX;
		if(!(m_smr[ch] & SMR_TXINTM) && (m_ssr[ch] & SSR_TXRDY))
			intr |= 1 << SR_INT_TX;
		if(!(m_smr[ch] & SMR_RXINTM) && (m_ssr[ch] & SSR_RBRK))
			intr |= 1 << SR_INT_ERR;
	}
	if(intr != old_interrupt) {
		//      logerror("serial intr mask %x\n", intr);
		for(int i=0; i != 3; i++)
			if((intr & (1 << i)) && !(m_sr_intr[ch] & (1 << i)))
				interrupt_internal_trigger(8 + 4*ch + i);
	}
	m_sr_intr[ch] = intr;
}


void tmp68301_device::ssr_set(int ch, u8 val, u8 mask)
{
	m_ssr[ch] = (m_ssr[ch] & ~mask) | val;
	serial_check_interrupt(ch);
}

u8 tmp68301_device::smr0_r() { return m_smr[0]; }
u8 tmp68301_device::smr1_r() { return m_smr[1]; }
u8 tmp68301_device::smr2_r() { return m_smr[2]; }
void tmp68301_device::smr0_w(u8 data) { smr_w(0, data); }
void tmp68301_device::smr1_w(u8 data) { smr_w(1, data); }
void tmp68301_device::smr2_w(u8 data) { smr_w(2, data); }

u8 tmp68301_device::scmr0_r() { return m_scmr[0]; }
u8 tmp68301_device::scmr1_r() { return m_scmr[1]; }
u8 tmp68301_device::scmr2_r() { return m_scmr[2]; }
void tmp68301_device::scmr0_w(u8 data) { scmr_w(0, data); }
void tmp68301_device::scmr1_w(u8 data) { scmr_w(1, data); }
void tmp68301_device::scmr2_w(u8 data) { scmr_w(2, data); }

u8 tmp68301_device::sbrr0_r() { return m_sbrr[0]; }
u8 tmp68301_device::sbrr1_r() { return m_sbrr[1]; }
u8 tmp68301_device::sbrr2_r() { return m_sbrr[2]; }
void tmp68301_device::sbrr0_w(u8 data) { sbrr_w(0, data); }
void tmp68301_device::sbrr1_w(u8 data) { sbrr_w(1, data); }
void tmp68301_device::sbrr2_w(u8 data) { sbrr_w(2, data); }

u8 tmp68301_device::ssr0_r() { return m_ssr[0]; }
u8 tmp68301_device::ssr1_r() { return m_ssr[1]; }
u8 tmp68301_device::ssr2_r() { return m_ssr[2]; }

u8 tmp68301_device::sdr0_r() { return sdr_r(0); }
u8 tmp68301_device::sdr1_r() { return sdr_r(1); }
u8 tmp68301_device::sdr2_r() { return sdr_r(2); }
void tmp68301_device::sdr0_w(u8 data) { sdr_w(0, data); }
void tmp68301_device::sdr1_w(u8 data) { sdr_w(1, data); }
void tmp68301_device::sdr2_w(u8 data) { sdr_w(2, data); }

u8 tmp68301_device::spr_r() { return m_spr; }
u8 tmp68301_device::scr_r() { return m_scr; }

void tmp68301_device::spr_w(u8 data)
{
	if(m_spr == data)
		return;
	m_spr = data;
	logerror("serial prescaler %d\n", m_spr ? m_spr : 256);
	serial_clock_update(0);
	serial_clock_update(1);
	serial_clock_update(2);
}

void tmp68301_device::scr_w(u8 data)
{
	data &= 0xa1;
	if(m_scr == data)
		return;
	m_scr = data;
	logerror("serial clk=%s reset=%s interrupt=%s\n",
			 m_scr & SCR_CKSE ? "internal" : "external",
			 m_scr & SCR_RES  ? "on" : "off",
			 m_scr & SCR_INTM ? "disabled" : "enabled");
	serial_clock_update(0);
	serial_clock_update(1);
	serial_clock_update(2);
	serial_check_interrupt(0);
	serial_check_interrupt(1);
	serial_check_interrupt(2);
}

void tmp68301_device::smr_w(int ch, u8 data)
{
	if(m_smr[ch] == data)
		return;
	m_smr[ch] = data;
	logerror("serial %d %c%c%c rxint=%s erint=%s txint=%s\n",
			 ch,
			 "5678"[(m_smr[ch] >> SMR_CL_SFT) & 3], "NENO"[(m_smr[ch] >> 4) & 3],
			 m_smr[ch] & SMR_ST ? '2' : '1',
			 m_smr[ch] & SMR_RXINTM ? "off" : "on",
			 m_smr[ch] & SMR_ERINTM ? "off" : "on",
			 m_smr[ch] & SMR_TXINTM ? "off" : "on");
	serial_check_interrupt(ch);
}

void tmp68301_device::scmr_w(int ch, u8 data)
{
	data &= ch == 0 ? 0x3f : 0x1d;
	u8 diff = m_scmr[ch] ^ data;
	if(!diff)
		return;
	m_scmr[ch] = data;
	if(ch == 0)
		logerror("serial %d command rts=%s error=%s break=%s rx=%s dtr=%s tx=%s\n",
				 ch,
				 m_scmr[ch] & SCMR_RTS  ? "low" : "high",
				 m_scmr[ch] & SCMR_ERS  ? "reset" : "nop",
				 m_scmr[ch] & SCMR_SBRK ? "on" : "off",
				 m_scmr[ch] & SCMR_RXEN ? "on" : "off",
				 m_scmr[ch] & SCMR_DTR  ? "low" : "high",
				 m_scmr[ch] & SCMR_TXEN ? "on" : "off");
	else
		logerror("serial %d command error=%s break=%s rx=%s tx=%s\n",
				 ch,
				 m_scmr[ch] & SCMR_ERS  ? "reset" : "nop",
				 m_scmr[ch] & SCMR_SBRK ? "on" : "off",
				 m_scmr[ch] & SCMR_RXEN ? "on" : "off",
				 m_scmr[ch] & SCMR_TXEN ? "on" : "off");


	if((m_scmr[ch] & SCMR_TXEN) && (m_ssr[ch] & SSR_TXE))
		m_ssr[ch] |= SSR_TXRDY;
	serial_check_interrupt(ch);
}

void tmp68301_device::sbrr_w(int ch, u8 data)
{
	if(m_sbrr[ch] == data)
		return;
	m_sbrr[ch] = data;
	logerror("serial %d baud rate divider %d\n", ch, sbrr_to_div(m_sbrr[ch]));
	serial_clock_update(ch);
}

u8 tmp68301_device::sdr_r(int ch)
{
	if(m_ssr[ch] & SSR_RXRDY)
		ssr_set(ch, 0, SSR_RXRDY);
	else
		ssr_set(ch, SSR_OE, SSR_OE);
	return m_sdrr[ch];
}

void tmp68301_device::sdr_w(int ch, u8 data)
{
	m_sdrt[ch] = data;
	if(m_scmr[ch] & SCMR_TXEN) {
		ssr_set(ch, 0, SSR_TXE|SSR_TXRDY);
		if(m_serial_tx_state[ch] == SR_DONE)
			m_serial_tx_state[ch] = SR_START;
		if(m_serial_tx_state[ch] == SR_IDLE) {
			m_serial_tx_state[ch] = SR_START;
			m_serial_tx_next_event[ch] = serial_get_next_edge(ch);
			internal_update();
		}
	}
}

void tmp68301_device::serial_clock_update(int ch)
{
	u32 divider = (m_spr ? m_spr : 256) * sbrr_to_div(m_sbrr[ch]);
	u32 bclk = m_scr & 0x80 ? clock() : m_serial_external_clock;
	logerror("serial %d divider %d baud %d\n", ch, divider, bclk/divider/8);

	if(m_scr & 0x80)
		m_serial_gclk[ch] = divider;
	else
		m_serial_gclk[ch] = clock() * divider / m_serial_external_clock;
}

u64 tmp68301_device::serial_get_next_edge(int ch)
{
	u64 current_time = total_cycles();
	u64 step = m_serial_gclk[ch];
	return ((current_time / step) + 1) * step;
}

void tmp68301_device::serial_rx_w(int ch, int state)
{
	if(m_serial_rx_line[ch] == state)
		return;
	m_serial_rx_line[ch] = state;
	if(m_serial_rx_state[ch] == SR_IDLE && !state && (m_scmr[ch] & SCMR_RXEN)) {
		m_serial_rx_state[ch] = SR_START;
		m_serial_rx_next_event[ch] = serial_get_next_edge(ch) + 4*m_serial_gclk[ch];
		internal_update();
	}
}

void tmp68301_device::serial_rx_update(int ch)
{
	//  logerror("rx update channel %d state %d\n", ch, m_serial_rx_state[ch]);
	u64 next = m_serial_rx_next_event[ch] + 8*m_serial_gclk[ch];
	u8 nstate = m_serial_rx_state[ch] + 1;
	u8 line = m_serial_rx_line[ch];
	switch(m_serial_rx_state[ch]) {
	case SR_START:
		if(line) {
			next = 0;
			nstate = SR_IDLE;
		}
		m_serial_rx[ch] = 0;
		break;

	case SR_BIT_0:
		m_serial_rx[ch] |= line << 0;
		break;

	case SR_BIT_1:
		m_serial_rx[ch] |= line << 1;
		break;

	case SR_BIT_2:
		m_serial_rx[ch] |= line << 2;
		break;

	case SR_BIT_3:
		m_serial_rx[ch] |= line << 3;
		break;

	case SR_BIT_4:
		m_serial_rx[ch] |= line << 4;
		if(((m_smr[ch] >> SMR_CL_SFT) & 3) == 0)
			nstate = m_smr[ch] & SMR_PEN ? SR_PARITY : SR_STOP;
		break;

	case SR_BIT_5:
		m_serial_rx[ch] |= line << 5;
		if(((m_smr[ch] >> SMR_CL_SFT) & 3) == 1)
			nstate = m_smr[ch] & SMR_PEN ? SR_PARITY : SR_STOP;
		break;

	case SR_BIT_6:
		m_serial_rx[ch] |= line << 6;
		if(((m_smr[ch] >> SMR_CL_SFT) & 3) == 2)
			nstate = m_smr[ch] & SMR_PEN ? SR_PARITY : SR_STOP;
		break;

	case SR_BIT_7:
		m_serial_rx[ch] |= line << 7;
		nstate = m_smr[ch] & SMR_PEN ? SR_PARITY : SR_STOP;
		break;

	case SR_PARITY:
		abort();

	case SR_STOP:
		if(!line) {
			if(m_ssr[ch] & SSR_FE)
				ssr_set(ch, SSR_RBRK, SSR_RBRK);
			else
				ssr_set(ch, SSR_FE, SSR_FE);

		} else if(m_ssr[ch] & SSR_RXRDY)
			ssr_set(ch, SSR_OE, SSR_OE);

		else {
			m_sdrr[ch] = m_serial_rx[ch];
			logerror("serial %d receive %02x '%c'\n", ch, m_sdrr[ch], m_sdrr[ch] >= 32 && m_sdrr[ch] < 127 ? m_sdrr[ch] : '.');
			ssr_set(ch, SSR_RXRDY, SSR_RXRDY);
		}

		next = 0;
		nstate = SR_IDLE;
		break;
	}

	m_serial_rx_next_event[ch] = next;
	m_serial_rx_state[ch] = nstate;
}

void tmp68301_device::serial_tx_update(int ch)
{
	//  logerror("tx update channel %d state %d\n", ch, m_serial_tx_state[ch]);
	u64 next = m_serial_tx_next_event[ch] + 8*m_serial_gclk[ch];
	u8 nstate = m_serial_tx_state[ch] + 1;
	switch(m_serial_tx_state[ch]) {
	case SR_START:
		m_serial_tx[ch] = m_sdrt[ch];
		logerror("serial %d transmit %02x '%c'\n", ch, m_sdrt[ch], m_sdrt[ch] >= 32 && m_sdrt[ch] < 127 ? m_sdrt[ch] : '.');
		ssr_set(ch, SSR_TXE|SSR_TXRDY, SSR_TXE|SSR_TXRDY);
		m_tx_cb[ch](0);
		break;

	case SR_BIT_0:
		m_tx_cb[ch]((m_serial_tx[ch] >> 0) & 1);
		break;

	case SR_BIT_1:
		m_tx_cb[ch]((m_serial_tx[ch] >> 1) & 1);
		break;

	case SR_BIT_2:
		m_tx_cb[ch]((m_serial_tx[ch] >> 2) & 1);
		break;

	case SR_BIT_3:
		m_tx_cb[ch]((m_serial_tx[ch] >> 3) & 1);
		break;

	case SR_BIT_4:
		m_tx_cb[ch]((m_serial_tx[ch] >> 4) & 1);
		if(((m_smr[ch] >> SMR_CL_SFT) & 3) == 0)
			nstate = m_smr[ch] & SMR_PEN ? SR_PARITY : SR_STOP;
		break;

	case SR_BIT_5:
		m_tx_cb[ch]((m_serial_tx[ch] >> 5) & 1);
		if(((m_smr[ch] >> SMR_CL_SFT) & 3) == 1)
			nstate = m_smr[ch] & SMR_PEN ? SR_PARITY : SR_STOP;
		break;

	case SR_BIT_6:
		m_tx_cb[ch]((m_serial_tx[ch] >> 6) & 1);
		if(((m_smr[ch] >> SMR_CL_SFT) & 3) == 2)
			nstate = m_smr[ch] & SMR_PEN ? SR_PARITY : SR_STOP;
		break;

	case SR_BIT_7:
		m_tx_cb[ch]((m_serial_tx[ch] >> 7) & 1);
		nstate = m_smr[ch] & SMR_PEN ? SR_PARITY : SR_STOP;
		break;

	case SR_PARITY: {
		u32 parity = m_smr[ch] & SMR_PEO ? 0 : 1;
		for(u32 i = 0; i != 5 + ((m_smr[ch] >> SMR_CL_SFT) & 3); i++)
			if((m_serial_tx[ch] >> i) & 1)
				parity = parity ^ 1;
		m_tx_cb[ch](parity);
		nstate = SR_STOP;
		break;
	}

	case SR_STOP:
		m_tx_cb[ch](1);
		if(m_smr[ch] & SMR_ST)
			next += 8*m_serial_gclk[ch];
		if((m_scmr[ch] & SCMR_TXEN) && !(m_ssr[ch] & SSR_TXE))
			nstate = SR_START;
		break;

	case SR_DONE:
		next = 0;
		nstate = SR_IDLE;
		break;
	}

	m_serial_tx_next_event[ch] = next;
	m_serial_tx_state[ch] = nstate;
	machine().scheduler().synchronize();
}


// 16-bit timer management

const int tmp68301_device::timer_source_id[3][2] = { { 1, 2 }, { 0, 2 }, { 0, 1 }};

const char *const tmp68301_device::timer_source_names[3][4] = {
	{ "internal", "external", "ch1", "ch2" },
	{ "internal", "external", "ch0", "ch2" },
	{ "internal", "external", "ch0", "ch1" }
};

const int tmp68301_device::timer_divider[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8 };

u16 tmp68301_device::tcr0_r() { return m_tcr[0]; }
u16 tmp68301_device::tcr1_r() { return m_tcr[1]; }
u16 tmp68301_device::tcr2_r() { return m_tcr[2]; }
u16 tmp68301_device::tmcr01_r() { return m_tmcr1[0]; }
u16 tmp68301_device::tmcr11_r() { return m_tmcr1[1]; }
u16 tmp68301_device::tmcr12_r() { return m_tmcr2[1]; }
u16 tmp68301_device::tmcr21_r() { return m_tmcr1[2]; }
u16 tmp68301_device::tmcr22_r() { return m_tmcr2[2]; }
u16 tmp68301_device::tctr0_r() { timer_sync(0); return m_tctr[0]; }
u16 tmp68301_device::tctr1_r() { timer_sync(1); return m_tctr[1]; }
u16 tmp68301_device::tctr2_r() { timer_sync(2); return m_tctr[2]; }

void tmp68301_device::tcr0_w(offs_t, u16 data, u16 mem_mask) { tcr_w(0, data, mem_mask); }
void tmp68301_device::tcr1_w(offs_t, u16 data, u16 mem_mask) { tcr_w(1, data, mem_mask); }
void tmp68301_device::tcr2_w(offs_t, u16 data, u16 mem_mask) { tcr_w(2, data, mem_mask); }

void tmp68301_device::tmcr01_w(offs_t, u16 data, u16 mem_mask) { tmcr1_w(0, data, mem_mask); }

void tmp68301_device::tmcr11_w(offs_t, u16 data, u16 mem_mask) { tmcr1_w(1, data, mem_mask); }
void tmp68301_device::tmcr12_w(offs_t, u16 data, u16 mem_mask) { tmcr2_w(1, data, mem_mask); }

void tmp68301_device::tmcr21_w(offs_t, u16 data, u16 mem_mask) { tmcr1_w(2, data, mem_mask); }
void tmp68301_device::tmcr22_w(offs_t, u16 data, u16 mem_mask) { tmcr2_w(2, data, mem_mask); }

void tmp68301_device::tctr0_w(offs_t, u16 data, u16 mem_mask) { tctr_w(0, data, mem_mask); }
void tmp68301_device::tctr1_w(offs_t, u16 data, u16 mem_mask) { tctr_w(1, data, mem_mask); }
void tmp68301_device::tctr2_w(offs_t, u16 data, u16 mem_mask) { tctr_w(2, data, mem_mask); }

void tmp68301_device::tcr_w(int ch, u16 data, u16 mem_mask)
{
	timer_sync(ch);

	static const char *const count_mode[4] = { "normal", "?", "start", "wait" };
	static const char *const max_mode[4] = { "?", "1", "2", "1&2" };
	u16 old = m_tmcr1[ch];
	COMBINE_DATA(&m_tcr[ch]);
	m_tcr[ch] &= ch == 0 ? 0xff87 : 0xfff7;
	if(!ch)
		m_tcr[ch] |= 0x0050;

	if(m_tcr[ch] == old)
		return;
	if(ch == 0)
		logerror("timer %d clk=%s div=%d mode=%s repeat=%s intr=%s %s%s\n",
				 ch,
				 timer_source_names[ch][(m_tcr[ch] >> TCR_CK) & 3],
				 1 << timer_divider[(m_tcr[ch] >> TCR_P) & 15],
				 count_mode[(m_tcr[ch] >> TCR_T) & 3],
				 m_tcr[ch] & TCR_N1 ? "on" : "off",
				 m_tcr[ch] & TCR_INT ? "on" : "off",
				 m_tcr[ch] & TCR_CS ? "stopped" : "running",
				 m_tcr[ch] & TCR_TS ? "" : " clear");
	else
		logerror("timer %d clk=%s div=%d mode=%s repeat=%s output=%s max=%s intr=%s %s%s\n",
				 ch,
				 timer_source_names[ch][(m_tcr[ch] >> TCR_CK) & 3],
				 1 << timer_divider[(m_tcr[ch] >> TCR_P) & 15],
				 count_mode[(m_tcr[ch] >> TCR_T) & 3],
				 m_tcr[ch] & TCR_N1 ? "on" : "off",
				 m_tcr[ch] & TCR_RP ? "invert" : "pulse",
				 max_mode[(m_tcr[ch] >> TCR_MR) & 3],
				 m_tcr[ch] & TCR_INT ? "on" : "off",
				 m_tcr[ch] & TCR_CS ? "stopped" : "running",
				 m_tcr[ch] & TCR_TS ? "" : " clear");

	timer_predict(ch);
}

void tmp68301_device::tmcr1_w(int ch, u16 data, u16 mem_mask)
{
	timer_sync(ch);
	u16 old = m_tmcr1[ch];
	COMBINE_DATA(&m_tmcr1[ch]);
	if(m_tmcr1[ch] == old)
		return;
	logerror("timer %d max 1 %04x\n", ch, m_tmcr1[ch]);
	timer_predict(ch);
}

void tmp68301_device::tmcr2_w(int ch, u16 data, u16 mem_mask)
{
	timer_sync(ch);
	u16 old = m_tmcr2[ch];
	COMBINE_DATA(&m_tmcr2[ch]);
	if(m_tmcr2[ch] == old)
		return;
	logerror("timer %d max 2 %04x\n", ch, m_tmcr2[ch]);
	timer_predict(ch);
}

void tmp68301_device::tctr_w(int ch, u16 data, u16 mem_mask)
{
	timer_sync(ch);
	logerror("timer %d counter reset\n", ch);
	if(ch)
		m_tctr[ch] = 0;
	timer_predict(ch);
}

void tmp68301_device::timer_update(int ch)
{
	timer_sync(ch);
	timer_predict(ch);
}

void tmp68301_device::timer_sync(int ch)
{
	if(!(m_tcr[ch] & TCR_TS)) {
		m_tctr[ch] = 0;
		return;
	}

	if(m_tcr[ch] & TCR_CS)
		return;

	u32 div = timer_divider[(m_tcr[ch] >> TCR_P) & 15];
	u64 ctime = total_cycles();
	// Don't fold the shifts, the computation would be incorrect
	u32 ntctr = m_tctr[ch] + ((ctime >> div) - (m_timer_last_sync[ch] >> div));

	u32 maxmode = (m_tcr[ch] >> TCR_MR) & 3;
	if(maxmode == 1 || maxmode == 2) {
		u32 max = (maxmode == 1) ? m_tmcr1[ch] : m_tmcr2[ch];
		if(max == 0)
			max = 0x10000;
		if(m_tctr[ch] >= max) {
			if(ntctr >= 0x10000)
				ntctr -= 0x10000;
			else
				max = 0x10000;
		}
		if(ntctr >= max) {
			if(m_tcr[ch] & TCR_INT)
			{
				// On '303 T0 can't irq, so we compensate by -1 there
				// (pkspirit cares for '7750 inputs)
				interrupt_internal_trigger(base_timer_irq() + ch);
			}
			ntctr = ntctr % max;
		}
	}
	m_tctr[ch] = ntctr;
	m_timer_last_sync[ch] = ctime;
}

void tmp68301_device::timer_predict(int ch)
{
	if(!(m_tcr[ch] & TCR_TS))
		m_tctr[ch] = 0;

	if((m_tcr[ch] & (TCR_INT|TCR_CS|TCR_TS)) != (TCR_INT|TCR_TS)) {
		m_timer_next_event[ch] = 0;
		return;
	}

	u32 maxmode = (m_tcr[ch] >> TCR_MR) & 3;

	if(maxmode == 0) {
		logerror("timer %d no max selected ?\n", ch);
		return;
	}

	if(maxmode == 3) {
		logerror("timer %d alternating max mode unsupported\n", ch);
		return;
	}

	if((m_tcr[ch] & TCR_N1) == 0) {
		// Need to add a flag to say "counter done" to make it work, reset on mode change.
		logerror("timer %d single-shot mode unsupported\n");
		return;
	}

	if(((m_tcr[ch] >> TCR_CK) & 3) != 0) {
		logerror("timer %d source %s unsupported\n", ch, timer_source_names[ch][(m_tcr[ch] >> TCR_CK) & 3]);
		return;
	}

	s32 delta = ((maxmode == 1) ? m_tmcr1[ch] : m_tmcr2[ch]) - m_tctr[ch];
	if(delta <= 0)
		delta += 0x10000;
	u32 div = timer_divider[(m_tcr[ch] >> TCR_P) & 15];
	u64 ctime = total_cycles();
	m_timer_next_event[ch] = (((ctime >> div) + delta) << div);
	recompute_bcount(ctime);
}

// 68303 overrides

// TODO:
// - Stub, needs internal map overrides (DMA, new timers, irq changes, stepping motor controller, other)
// - DMAC for pkspirit

tmp68303_device::tmp68303_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tmp68301_device(mconfig, TMP68303, tag, owner, clock)
{
}
