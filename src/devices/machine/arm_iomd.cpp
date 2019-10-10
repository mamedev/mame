// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

	ARM IOMD device emulation
	
	ARM7 SoC, upgraded version(s) of the IOC found in Acorn Archimedes.

	TODO:
	- IOCR / IOLINES hookups can be further improved, also DDR bits needs verifying;
	- word-boundary accesses for 8-bit ports;
	- split into different types, add quick notes about where they diverges do in this header;

***************************************************************************/

#include "emu.h"
#include "arm_iomd.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ARM_IOMD, arm_iomd_device, "arm_iomd", "ARM IOMD controller")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arm_iomd_device - constructor
//-------------------------------------------------

void arm_iomd_device::map(address_map &map)
{
	// I/O
	map(0x000, 0x003).rw(FUNC(arm_iomd_device::iocr_r), FUNC(arm_iomd_device::iocr_w));
	
	map(0x00c, 0x00f).rw(FUNC(arm_iomd_device::iolines_r), FUNC(arm_iomd_device::iolines_w));
	// interrupt A/B/fiq
	map(0x010, 0x013).r(FUNC(arm_iomd_device::irqst_r<0>));
	map(0x014, 0x017).rw(FUNC(arm_iomd_device::irqrq_r<0>), FUNC(arm_iomd_device::irqrq_w<0>));
	map(0x018, 0x01b).rw(FUNC(arm_iomd_device::irqmsk_r<0>), FUNC(arm_iomd_device::irqmsk_w<0>));

	// timers
	map(0x040, 0x043).rw(FUNC(arm_iomd_device::tNlow_r<0>), FUNC(arm_iomd_device::tNlow_w<0>));
	map(0x044, 0x047).rw(FUNC(arm_iomd_device::tNhigh_r<0>), FUNC(arm_iomd_device::tNhigh_w<0>));
	map(0x048, 0x04b).w(FUNC(arm_iomd_device::tNgo_w<0>));
	map(0x04c, 0x04f).w(FUNC(arm_iomd_device::tNlatch_w<0>));
	map(0x050, 0x053).rw(FUNC(arm_iomd_device::tNlow_r<1>), FUNC(arm_iomd_device::tNlow_w<1>));
	map(0x054, 0x057).rw(FUNC(arm_iomd_device::tNhigh_r<1>), FUNC(arm_iomd_device::tNhigh_w<1>));
	map(0x058, 0x05b).w(FUNC(arm_iomd_device::tNgo_w<1>));
	map(0x05c, 0x05f).w(FUNC(arm_iomd_device::tNlatch_w<1>));
	// interrupt C/D
//	map(0x060, 0x063)
	// ROM control
//	map(0x080, 0x083)
	// device identifiers
//	map(0x094, 0x097)
//	map(0x098, 0x09b)
//	map(0x09c, 0x09f)
	// mouse (for PS7500, others may differ)

	// ...
	// DMA
//	map(0x180, 0x183)

//	map(0x1d0, 0x1d3).rw(FUNC(arm_iomd_device::vidcur_r), FUNC(arm_iomd_device::vidcur_w));
	map(0x1d4, 0x1d7).rw(FUNC(arm_iomd_device::vidend_r), FUNC(arm_iomd_device::vidend_w));

	map(0x1dc, 0x1df).rw(FUNC(arm_iomd_device::vidinita_r), FUNC(arm_iomd_device::vidinita_w));
	map(0x1e0, 0x1e3).rw(FUNC(arm_iomd_device::vidcr_r), FUNC(arm_iomd_device::vidcr_w));
	
//  TODO: iomd2 has extra regs in 0x200-0x3ff area, others NOPs / mirrors?
}


arm_iomd_device::arm_iomd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ARM_IOMD, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_vidc(*this, finder_base::DUMMY_TAG)
	, m_iolines_read_cb(*this)
	, m_iolines_write_cb(*this)
	, m_iocr_read_od_cb{{*this}, {*this}}
	, m_iocr_write_od_cb{{*this}, {*this}}
	, m_iocr_read_id_cb(*this)
	, m_iocr_write_id_cb(*this)
{

}

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void arm_iomd_device::device_add_mconfig(machine_config &config)
{
	//DEVICE(config, ...);
	//TODO: keyboard and mouse interfaces at very least, also they differs by device type
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arm_iomd_device::device_start()
{	
	m_iolines_read_cb.resolve_safe(0xff);
	m_iolines_write_cb.resolve_safe();
	for (devcb_read_line &cb : m_iocr_read_od_cb)
		cb.resolve_safe(1);
	
	for (devcb_write_line &cb : m_iocr_write_od_cb)
		cb.resolve_safe();
	
	m_iocr_read_id_cb.resolve_safe(1);
	m_iocr_write_id_cb.resolve_safe();

	save_item(NAME(m_iocr_ddr));
	save_item(NAME(m_iolines_ddr));
	save_item(NAME(m_video_enable));
	save_item(NAME(m_vidinita));
	save_item(NAME(m_vidend));
	save_pointer(NAME(m_irq_mask), 4);
	save_pointer(NAME(m_irq_status), 4);
	
	m_host_space = &m_host_cpu->space(AS_PROGRAM);
	
	m_timer[0] = timer_alloc(T0_TIMER);
	m_timer[1] = timer_alloc(T1_TIMER);
	save_pointer(NAME(m_timer_in), 2);
	save_pointer(NAME(m_timer_out), 2);
	save_pointer(NAME(m_timer_counter), 2);
	save_pointer(NAME(m_timer_readinc), 2);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arm_iomd_device::device_reset()
{
	int i;
	m_iocr_ddr = 0x0b;
	m_iolines_ddr = 0xff;
	m_video_enable = false;
	// TODO: defaults for these
	m_vidinita = 0;
	m_vidend = 0;

	for (i=0; i<4; i++)
	{
		m_irq_status[i] = 0;
		m_irq_mask[i] = 0;
	}
	
	for (i=0; i<2; i++)
		m_timer[i]->adjust(attotime::never);
}

void arm_iomd_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case T0_TIMER:
		case T1_TIMER: 
			trigger_irq(IRQA, id == T1_TIMER ? 0x40 : 0x20);
			break;
	}
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

// TODO: nINT1
READ32_MEMBER( arm_iomd_device::iocr_r )
{
	u8 res = 0;

	res = m_iocr_read_id_cb() << 3;
	res|= m_iocr_read_od_cb[1]() << 1;
	res|= m_iocr_read_od_cb[0]() << 0;
	
	return (m_vidc->flyback_r() << 7) | 0x34 | (res & m_iocr_ddr);
}

WRITE32_MEMBER( arm_iomd_device::iocr_w )
{
	m_iocr_ddr = (data & 0x0b);
	m_iocr_write_id_cb(BIT(m_iocr_ddr,3));
	m_iocr_write_od_cb[1](BIT(m_iocr_ddr,1));
	m_iocr_write_od_cb[0](BIT(m_iocr_ddr,0));
}

READ32_MEMBER( arm_iomd_device::iolines_r )
{
	return m_iolines_read_cb() & m_iolines_ddr;
}

WRITE32_MEMBER( arm_iomd_device::iolines_w )
{
	m_iolines_ddr = data;
	m_iolines_write_cb(m_iolines_ddr);
}

// irqA is special since it has a force (7) and ignore (1) bits
inline u8 arm_iomd_device::update_irqa_type(u8 data)
{
	return (data & 0xfd) | 0x80;
}

// interrupts

inline void arm_iomd_device::flush_irq(unsigned Which)
{
	if (m_irq_status[Which] & m_irq_mask[Which])
		m_host_cpu->pulse_input_line(ARM7_IRQ_LINE, m_host_cpu->minimum_quantum_time());
}

inline void arm_iomd_device::trigger_irq(unsigned Which, u8 irq_type)
{
	m_irq_status[Which] |= irq_type;
	flush_irq(Which);
}

template <unsigned Which> READ32_MEMBER( arm_iomd_device::irqst_r )
{
	return m_irq_status[Which];
}

template <unsigned Which> READ32_MEMBER( arm_iomd_device::irqrq_r )
{
	return m_irq_status[Which] & m_irq_mask[Which];
}

template <unsigned Which> READ32_MEMBER( arm_iomd_device::irqmsk_r )
{
	return m_irq_mask[Which];
}

template <unsigned Which> WRITE32_MEMBER( arm_iomd_device::irqrq_w )
{
	u8 res = m_irq_status[Which] & ~data;
	if (Which == 0)
		res = update_irqa_type(res);
	m_irq_status[Which] = res;
	flush_irq(Which);
}

template <unsigned Which> WRITE32_MEMBER( arm_iomd_device::irqmsk_w )
{
	m_irq_mask[Which] = data & 0xff;
	flush_irq(Which);
}

// timers
inline void arm_iomd_device::trigger_timer(unsigned Which)
{
	int timer_count = m_timer_counter[Which];
	// TODO: it's actually a 2 MHz timer
	int val = timer_count / 2; 

	if(val==0)
		m_timer[Which]->adjust(attotime::never);
	else
		m_timer[Which]->adjust(attotime::from_usec(val), 0, attotime::from_usec(val));	
}

// TODO: live updates aren't really supported here
template <unsigned Which> READ32_MEMBER( arm_iomd_device::tNlow_r )
{
	return m_timer_out[Which] & 0xff;
}

template <unsigned Which> READ32_MEMBER( arm_iomd_device::tNhigh_r )
{
	return (m_timer_out[Which] >> 8) & 0xff;
}

template <unsigned Which> WRITE32_MEMBER( arm_iomd_device::tNlow_w )
{
	m_timer_in[Which] = (m_timer_in[Which] & 0xff00) | (data & 0xff);
}

template <unsigned Which> WRITE32_MEMBER( arm_iomd_device::tNhigh_w )
{
	m_timer_in[Which] = (m_timer_in[Which] & 0x00ff) | ((data & 0xff) << 8);
}

template <unsigned Which> WRITE32_MEMBER( arm_iomd_device::tNgo_w )
{
	m_timer_counter[Which] = m_timer_in[Which];
	trigger_timer(Which);
}

template <unsigned Which> WRITE32_MEMBER( arm_iomd_device::tNlatch_w )
{
	m_timer_readinc[Which] ^=1;
	m_timer_out[Which] = m_timer_counter[Which];
	if(m_timer_readinc[Which])
	{
		m_timer_counter[Which]--;
		if(m_timer_counter[Which] < 0)
			m_timer_counter[Which] += m_timer_in[Which];
	}
}


// DMAs

READ32_MEMBER( arm_iomd_device::vidcr_r )
{
	// bit 6: DRAM mode
	// bits 4-0: qword transfer
	return 0x40 | (m_video_enable << 5) | 0x10;
}

WRITE32_MEMBER( arm_iomd_device::vidcr_w )
{
	m_video_enable = BIT(data, 5);
	if (data & 0x80)
		throw emu_fatalerror("%s VIDCR LCD dual panel mode enabled", this->tag());
}

READ32_MEMBER( arm_iomd_device::vidend_r )
{
	return m_vidend;
}

WRITE32_MEMBER( arm_iomd_device::vidend_w )
{
	COMBINE_DATA(&m_vidend);
	m_vidend &= 0x00fffff8;
}

READ32_MEMBER( arm_iomd_device::vidinita_r )
{
	return m_vidinita;
}

WRITE32_MEMBER( arm_iomd_device::vidinita_w )
{
	COMBINE_DATA(&m_vidinita);
}


//**************************************************************************
//  VIDC comms
//**************************************************************************

WRITE_LINE_MEMBER( arm_iomd_device::vblank_irq )
{
	if (!state)
		return;

	trigger_irq(IRQA, 0x08);
	if (m_video_enable == true)
	{
		// TODO: much more complex, last/end regs, start regs and eventually LCD hooks
		uint32_t src = m_vidinita & 0x1ffffff0;
		uint32_t size = m_vidend;

		// TODO: vidcur can be readback, support it once anything makes use of the 0x1d0 reg for obvious reasons
		// (and using m_ prefix is intentional too)
		for (uint32_t m_vidcur = 0; m_vidcur<size; m_vidcur++)
		{
			m_vidc->write_vram(m_vidcur, m_host_space->read_byte(src));
			src++;
			src &= 0x1fffffff;
		}
	}
}

