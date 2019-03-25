// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68307 */

#include "emu.h"
#include "68307.h"
#include "68307bus.h"
#include "68307sim.h"
#include "68307tmu.h"

DEFINE_DEVICE_TYPE(M68307, m68307_cpu_device, "mc68307", "MC68307")


/* 68307 SERIAL Module */
/* all ports on this are 8-bit? */

/* this is a 68681 'compatible' chip but with only a single channel implemented
  (writes to the other channel have no effects)

  for now at least we piggyback on the existing 68307 emulation rather than having
  a custom verson here, that may change later if subtle differences exist.

*/
READ8_MEMBER( m68307_cpu_device::m68307_internal_serial_r )
{
	if (offset&1) return m_duart->read(offset>>1);
	return 0x0000;
}

WRITE8_MEMBER(m68307_cpu_device::m68307_internal_serial_w)
{
	if (offset & 1) m_duart->write(offset >> 1, data);
}



void m68307_cpu_device::m68307_internal_map(address_map &map)
{
	map(0x000000f0, 0x000000ff).rw(FUNC(m68307_cpu_device::m68307_internal_base_r), FUNC(m68307_cpu_device::m68307_internal_base_w));
}


void m68307_cpu_device::device_add_mconfig(machine_config &config)
{
	MC68681(config, m_duart, 16000000/4); // ?? Mhz - should be specified in inline config
	m_duart->irq_cb().set(FUNC(m68307_cpu_device::m68307_duart_irq_handler));
	m_duart->a_tx_cb().set(FUNC(m68307_cpu_device::m68307_duart_txa));
	m_duart->b_tx_cb().set(FUNC(m68307_cpu_device::m68307_duart_txb));
	m_duart->inport_cb().set(FUNC(m68307_cpu_device::m68307_duart_input_r));
	m_duart->outport_cb().set(FUNC(m68307_cpu_device::m68307_duart_output_w));
}


m68307_cpu_device::m68307_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m68000_device(mconfig, tag, owner, clock, M68307, 16, 24, address_map_constructor(FUNC(m68307_cpu_device::m68307_internal_map), this)),
	m_write_irq(*this),
	m_write_a_tx(*this),
	m_write_b_tx(*this),
	m_read_inport(*this),
	m_write_outport(*this),
	m_duart(*this, "internal68681")
{
	m_m68307SIM = nullptr;
	m_m68307MBUS = nullptr;
	m_m68307TIMER = nullptr;
	m_m68307_base = 0;
	m_m68307_scrhigh = 0;
	m_m68307_scrlow = 0;
	m_m68307_currentcs = 0;
	m_ipl = 0;
}






void m68307_cpu_device::device_reset()
{
	m68000_device::device_reset();

	if (m_m68307SIM) m_m68307SIM->reset();
	if (m_m68307MBUS) m_m68307MBUS->reset();
	if (m_m68307TIMER) m_m68307TIMER->reset();

	m_m68307_base = 0xbfff;
	m_m68307_scrhigh = 0x0007;
	m_m68307_scrlow = 0xf010;

	set_ipl(0);
}

void m68307_cpu_device::m68k_reset_peripherals()
{
	m_duart->reset();

	if (m_m68307MBUS) m_m68307MBUS->reset();
	if (m_m68307TIMER) m_m68307TIMER->reset();
}


/* todo: is it possible to calculate the address map based on CS when they change
   and install handlers?  Going through this logic for every memory access is
   very slow */

inline int m68307_cpu_device::calc_cs(offs_t address) const
{
	m68307_sim const &sim = *m_m68307SIM;
	for (int i=0; i < 4; i++)
	{
		int const br = sim.m_br[i] & 1;
		int const amask = (sim.m_or[i] & 0x1ffc) << 11;
		int const bra = (sim.m_br[i] & 0x1ffc) << 11;
		if (br && ((address & amask) == bra)) return i + 1;
	}
	return 0;
}

void m68307_cpu_device::init16_m68307(address_space &space)
{
	m_space = &space;
	auto cache = space.cache<1, 0, ENDIANNESS_BIG>();

	m_readimm16 = [cache](offs_t address) -> u16 { /* m_m68307_currentcs = calc_cs(address); */ return cache->read_word(address); };
	m_read8   = [this](offs_t address) -> u8     { /* m_m68307_currentcs = calc_cs(address); */ return m_space->read_byte(address); };
	m_read16  = [this](offs_t address) -> u16    { /* m_m68307_currentcs = calc_cs(address); */ return m_space->read_word(address); };
	m_read32  = [this](offs_t address) -> u32    { /* m_m68307_currentcs = calc_cs(address); */ return m_space->read_dword(address); };
	m_write8  = [this](offs_t address, u8 data)  { /* m_m68307_currentcs = calc_cs(address); */ m_space->write_byte(address, data); };
	m_write16 = [this](offs_t address, u16 data) { /* m_m68307_currentcs = calc_cs(address); */ m_space->write_word(address, data); };
	m_write32 = [this](offs_t address, u32 data) { /* m_m68307_currentcs = calc_cs(address); */ m_space->write_dword(address, data); };
}



void m68307_cpu_device::set_port_callbacks(
		porta_read_delegate &&porta_r,
		porta_write_delegate &&porta_w,
		portb_read_delegate &&portb_r,
		portb_write_delegate &&portb_w)
{
	m_porta_r = std::move(porta_r);
	m_porta_w = std::move(porta_w);
	m_portb_r = std::move(portb_r);
	m_portb_w = std::move(portb_w);
}





uint16_t m68307_cpu_device::get_cs(offs_t address)
{
	m_m68307_currentcs = calc_cs(address);

	return m_m68307_currentcs;
}


/* 68307 specifics - MOVE */

void m68307_cpu_device::set_ipl(int level)
{
	if (level != m_ipl)
	{
		if (m_ipl != 0)
			set_input_line(m_ipl, CLEAR_LINE);
		m_ipl = level;
		if (m_ipl != 0)
			set_input_line(m_ipl, ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER(m68307_cpu_device::timer0_interrupt)
{
	int prioritylevel = (m_m68307SIM->m_picr & 0x7000) >> 12;
	if (state && m_ipl < prioritylevel)
		set_ipl(prioritylevel);
	else if (!state && m_ipl == prioritylevel)
		set_ipl(m_m68307SIM->get_ipl(this));
}

WRITE_LINE_MEMBER(m68307_cpu_device::timer1_interrupt)
{
	int prioritylevel = (m_m68307SIM->m_picr & 0x0700) >> 8;
	if (state && m_ipl < prioritylevel)
		set_ipl(prioritylevel);
	else if (!state && m_ipl == prioritylevel)
		set_ipl(m_m68307SIM->get_ipl(this));
}

WRITE_LINE_MEMBER(m68307_cpu_device::m68307_duart_irq_handler)
{
	int prioritylevel = (m_m68307SIM->m_picr & 0x0070) >> 4;
	if (state && m_ipl < prioritylevel)
		set_ipl(prioritylevel);
	else if (!state && m_ipl == prioritylevel)
		set_ipl(m_m68307SIM->get_ipl(this));
}

WRITE_LINE_MEMBER(m68307_cpu_device::mbus_interrupt)
{
	int prioritylevel = (m_m68307SIM->m_picr & 0x0007) >> 0;
	if (state && m_ipl < prioritylevel)
		set_ipl(prioritylevel);
	else if (!state && m_ipl == prioritylevel)
		set_ipl(m_m68307SIM->get_ipl(this));
}

void m68307_cpu_device::licr2_interrupt()
{
	m_m68307SIM->m_licr2 |= 0x8;

	int prioritylevel = (m_m68307SIM->m_licr2 & 0x0007) >> 0;
	if (m_ipl < prioritylevel)
		set_ipl(prioritylevel);
}

IRQ_CALLBACK_MEMBER(m68307_cpu_device::int_ack)
{
	uint8_t type = m_m68307SIM->get_int_type(this, irqline);
	logerror("Interrupt acknowledged: level %d, type %01X\n", irqline, type);

	// UART provides its own vector
	if (type == 0x0c)
		return m_duart->get_irq_vector();
	else
		return (m_m68307SIM->m_pivr & 0xf0) | type;
}

void m68307_cpu_device::device_config_complete()
{
	set_irq_acknowledge_callback(device_irq_acknowledge_delegate(FUNC(m68307_cpu_device::int_ack), this));
}

void m68307_cpu_device::device_start()
{
	init_cpu_m68000();

	/* basic CS logic, timers, mbus, serial logic
	   set via remappable register
	*/

	init16_m68307(*m_program);

	m_m68307SIM    = new m68307_sim();
	m_m68307MBUS   = new m68307_mbus();
	m_m68307TIMER  = new m68307_timer();

	m_m68307TIMER->init(this);

	m_m68307SIM->reset();
	m_m68307MBUS->reset();
	m_m68307TIMER->reset();

	m_internal = &space(AS_PROGRAM);
	m_m68307_base = 0xbfff;
	m_m68307_scrhigh = 0x0007;
	m_m68307_scrlow = 0xf010;

	m_write_irq.resolve_safe();
	m_write_a_tx.resolve_safe();
	m_write_b_tx.resolve_safe();
	m_read_inport.resolve();
	m_write_outport.resolve_safe();

	set_port_callbacks(porta_read_delegate(), porta_write_delegate(), portb_read_delegate(), portb_write_delegate());
}



READ16_MEMBER( m68307_cpu_device::m68307_internal_base_r )
{
	logerror("%08x m68307_internal_base_r %08x, (%04x)\n", m_ppc, offset*2,mem_mask);

	switch (offset<<1)
	{
		case 0x2: return m_m68307_base;
		case 0x4: return m_m68307_scrhigh;
		case 0x6: return m_m68307_scrlow;
	}

	logerror("(read was illegal?)\n");

	return 0x0000;
}

WRITE16_MEMBER( m68307_cpu_device::m68307_internal_base_w )
{
	logerror("%08x m68307_internal_base_w %08x, %04x (%04x)\n", m_ppc, offset*2,data,mem_mask);
	int base;
	//int mask = 0;

	switch (offset<<1)
	{
		case 0x2:
			/* remove old internal handler */
			base = (m_m68307_base & 0x0fff) << 12;
			//mask = (m_m68307_base & 0xe000) >> 13;
			//if ( m_m68307_base & 0x1000 ) mask |= 7;
			m_internal->unmap_readwrite(base+0x000, base+0x04f);
			m_internal->unmap_readwrite(base+0x100, base+0x11f);
			m_internal->unmap_readwrite(base+0x120, base+0x13f);
			m_internal->unmap_readwrite(base+0x140, base+0x149);

			/* store new base address */
			COMBINE_DATA(&m_m68307_base);

			/* install new internal handler */
			base = (m_m68307_base & 0x0fff) << 12;
			//mask = (m_m68307_base & 0xe000) >> 13;
			//if ( m_m68307_base & 0x1000 ) mask |= 7;
			m_internal->install_readwrite_handler(base + 0x000, base + 0x04f, read16_delegate(FUNC(m68307_cpu_device::m68307_internal_sim_r),this),    write16_delegate(FUNC(m68307_cpu_device::m68307_internal_sim_w),this));
			m_internal->install_readwrite_handler(base + 0x100, base + 0x11f, read8_delegate(FUNC(m68307_cpu_device::m68307_internal_serial_r),this), write8_delegate(FUNC(m68307_cpu_device::m68307_internal_serial_w),this), 0xffff);
			m_internal->install_readwrite_handler(base + 0x120, base + 0x13f, read16_delegate(FUNC(m68307_cpu_device::m68307_internal_timer_r),this),  write16_delegate(FUNC(m68307_cpu_device::m68307_internal_timer_w),this));
			m_internal->install_readwrite_handler(base + 0x140, base + 0x149, read8_delegate(FUNC(m68307_cpu_device::m68307_internal_mbus_r),this),   write8_delegate(FUNC(m68307_cpu_device::m68307_internal_mbus_w),this), 0xffff);


			break;

		case 0x4:
			COMBINE_DATA(&m_m68307_scrhigh);
			break;

		case 0x6:
			COMBINE_DATA(&m_m68307_scrlow);
			break;

		default:
			logerror("(write was illegal?)\n");
			break;
	}
}
