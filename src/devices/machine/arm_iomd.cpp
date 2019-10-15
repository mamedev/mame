// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

	ARM IOMD device emulation
	
	ARM7 SoC or stand-alone device, upgraded version(s) of the IOC found in Acorn Archimedes.

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
//	map(0x004, 0x007).rw(FUNC(arm_iomd_device::kbddat_r), FUNC(arm_iomd_device::kbddat_w));
//	map(0x008, 0x00b).rw(FUNC(arm_iomd_device::kbdcr_r), FUNC(arm_iomd_device::kbdcr_w));
	map(0x00c, 0x00f).rw(FUNC(arm_iomd_device::iolines_r), FUNC(arm_iomd_device::iolines_w));
	// interrupt A/B/fiq
	map(0x010, 0x013).r(FUNC(arm_iomd_device::irqst_r<IRQA>));
	map(0x014, 0x017).rw(FUNC(arm_iomd_device::irqrq_r<IRQA>), FUNC(arm_iomd_device::irqrq_w<IRQA>));
	map(0x018, 0x01b).rw(FUNC(arm_iomd_device::irqmsk_r<IRQA>), FUNC(arm_iomd_device::irqmsk_w<IRQA>));
//	map(0x01c, 0x01f).rw(FUNC(arm_iomd_device::susmode_r), FUNC(arm_iomd_device::susmode_w));

	map(0x020, 0x023).r(FUNC(arm_iomd_device::irqst_r<IRQB>));
	map(0x024, 0x027).rw(FUNC(arm_iomd_device::irqrq_r<IRQB>), FUNC(arm_iomd_device::irqrq_w<IRQB>));
	map(0x028, 0x02b).rw(FUNC(arm_iomd_device::irqmsk_r<IRQB>), FUNC(arm_iomd_device::irqmsk_w<IRQB>));
//	map(0x02c, 0x02f).w(FUNC(arm_iomd_device::stopmode_w));

//	map(0x030, 0x033).r(FUNC(arm_iomd_device::fiqst_r));
//	map(0x034, 0x037).rw(FUNC(arm_iomd_device::fiqrq_r), FUNC(arm_iomd_device::fiqrq_w));
//	map(0x038, 0x03b).rw(FUNC(arm_iomd_device::fiqmsk_r), FUNC(arm_iomd_device::fiqmsk_w));
//	map(0x03c, 0x03f).rw(FUNC(arm_iomd_device::clkctl_r), FUNC(arm_iomd_device::clkctl_w));
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
	map(0x060, 0x063).r(FUNC(arm_iomd_device::irqst_r<IRQC>));
	map(0x064, 0x067).rw(FUNC(arm_iomd_device::irqrq_r<IRQC>), FUNC(arm_iomd_device::irqrq_w<IRQC>));
	map(0x068, 0x06b).rw(FUNC(arm_iomd_device::irqmsk_r<IRQC>), FUNC(arm_iomd_device::irqmsk_w<IRQC>));	
//	map(0x06c, 0x06f).rw(FUNC(arm_iomd_device::vidmux_r), FUNC(arm_iomd_device::vidmux_w));
	map(0x070, 0x073).r(FUNC(arm_iomd_device::irqst_r<IRQD>));
	map(0x074, 0x077).rw(FUNC(arm_iomd_device::irqrq_r<IRQD>), FUNC(arm_iomd_device::irqrq_w<IRQD>));
	map(0x078, 0x07b).rw(FUNC(arm_iomd_device::irqmsk_r<IRQD>), FUNC(arm_iomd_device::irqmsk_w<IRQD>));	
	// ROM control
//	map(0x080, 0x083).rw(FUNC(arm_iomd_device::romcr_r<0>), FUNC(arm_iomd_device::romcr_w<0>));
//	map(0x084, 0x087).rw(FUNC(arm_iomd_device::romcr_r<1>), FUNC(arm_iomd_device::romcr_w<1>));
//	map(0x08c, 0x08f).rw(FUNC(arm_iomd_device::refcr_r), FUNC(arm_iomd_device::refcr_w));

	// device identifiers
//	map(0x094, 0x097).r(FUNC(arm_iomd_device::id_r<0>));
//	map(0x098, 0x09b).r(FUNC(arm_iomd_device::id_r<1>));
//	map(0x09c, 0x09f).r(FUNC(arm_iomd_device::version_r));
	// mouse (for PS7500, others may differ)
//	map(0x0a8, 0x0ab).rw(FUNC(arm_iomd_device::msedat_r), FUNC(arm_iomd_device::msedat_w));
//	map(0x0ac, 0x0af).rw(FUNC(arm_iomd_device::msecr_r), FUNC(arm_iomd_device::msecr_w));

	// I/O control
//	map(0x0c4, 0x0c7).rw(FUNC(arm_iomd_device::iotcr_r), FUNC(arm_iomd_device::iotcr_w));
//	map(0x0c8, 0x0cb).rw(FUNC(arm_iomd_device::ectcr_r), FUNC(arm_iomd_device::ectcr_w));
//	map(0x0cc, 0x0cf).rw(FUNC(arm_iomd_device::astcr_r), FUNC(arm_iomd_device::astcr_w));
	// RAM control
//	map(0x0d0, 0x0d3).rw(FUNC(arm_iomd_device::dramctl_r), FUNC(arm_iomd_device::dramctl_w));
//	map(0x0d4, 0x0d7).rw(FUNC(arm_iomd_device::selfref_r), FUNC(arm_iomd_device::selfref_w));
	// A/D converter
//	map(0x0e0, 0x0e3).rw(FUNC(arm_iomd_device::atodicr_r), FUNC(arm_iomd_device::atodicr_w));
//	map(0x0e4, 0x0e7).r(FUNC(arm_iomd_device::atodsr_r));
//	map(0x0e8, 0x0eb).rw(FUNC(arm_iomd_device::atodcc_r), FUNC(arm_iomd_device::atodcc_w));
//	map(0x0ec, 0x0ef).r(FUNC(arm_iomd_device::atodcnt_r<0>)); 
//	map(0x0f0, 0x0f3).r(FUNC(arm_iomd_device::atodcnt_r<1>));
//	map(0x0f4, 0x0f7).r(FUNC(arm_iomd_device::atodcnt_r<2>));
//	map(0x0f8, 0x0fb).r(FUNC(arm_iomd_device::atodcnt_r<3>));
	// sound DMA
	// note: sound DMA is actually labeled sd0* in the quick r/w sheet but there's no actual sd1, scrapped during HW dev?
	map(0x180, 0x183).rw(FUNC(arm_iomd_device::sdcur_r<0>), FUNC(arm_iomd_device::sdcur_w<0>));
	map(0x184, 0x187).rw(FUNC(arm_iomd_device::sdend_r<0>), FUNC(arm_iomd_device::sdend_w<0>));
	map(0x188, 0x18b).rw(FUNC(arm_iomd_device::sdcur_r<1>), FUNC(arm_iomd_device::sdcur_w<1>));
	map(0x18c, 0x18f).rw(FUNC(arm_iomd_device::sdend_r<1>), FUNC(arm_iomd_device::sdend_w<1>));
	map(0x190, 0x193).rw(FUNC(arm_iomd_device::sdcr_r), FUNC(arm_iomd_device::sdcr_w));
	map(0x194, 0x197).r(FUNC(arm_iomd_device::sdst_r));

	// video DMA
//	map(0x1c0, 0x1c3).rw(FUNC(arm_iomd_device::curscur_r), FUNC(arm_iomd_device::curscur_w));
//	map(0x1c4, 0x1c7).rw(FUNC(arm_iomd_device::cursinit_r), FUNC(arm_iomd_device::cursinit_w));
//	map(0x1c8, 0x1cb).rw(FUNC(arm_iomd_device::vidcurb_r), FUNC(arm_iomd_device::vidcurb_w));

//	map(0x1d0, 0x1d3).rw(FUNC(arm_iomd_device::vidcura_r), FUNC(arm_iomd_device::vidcura_w));
	map(0x1d4, 0x1d7).rw(FUNC(arm_iomd_device::vidend_r), FUNC(arm_iomd_device::vidend_w));
//	map(0x1d8, 0x1db).rw(FUNC(arm_iomd_device::vidstart_r), FUNC(arm_iomd_device::vidstart_w));
	map(0x1dc, 0x1df).rw(FUNC(arm_iomd_device::vidinita_r), FUNC(arm_iomd_device::vidinita_w));
	map(0x1e0, 0x1e3).rw(FUNC(arm_iomd_device::vidcr_r), FUNC(arm_iomd_device::vidcr_w));
//	map(0x1e8, 0x1eb).rw(FUNC(arm_iomd_device::vidinitb_r), FUNC(arm_iomd_device::vidinitb_w));
	// interrupt DMA
	map(0x1f0, 0x1f3).r(FUNC(arm_iomd_device::irqst_r<IRQDMA>));
	map(0x1f4, 0x1f7).rw(FUNC(arm_iomd_device::irqrq_r<IRQDMA>), FUNC(arm_iomd_device::irqrq_w<IRQDMA>));
	map(0x1f8, 0x1fb).rw(FUNC(arm_iomd_device::irqmsk_r<IRQDMA>), FUNC(arm_iomd_device::irqmsk_w<IRQDMA>));	

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
	save_item(NAME(m_vidlast));
	save_item(NAME(m_videqual));
	save_pointer(NAME(m_irq_mask), IRQ_SOURCES_SIZE);
	save_pointer(NAME(m_irq_status), IRQ_SOURCES_SIZE);
	
	m_host_space = &m_host_cpu->space(AS_PROGRAM);
	
	m_timer[0] = timer_alloc(T0_TIMER);
	m_timer[1] = timer_alloc(T1_TIMER);
	save_pointer(NAME(m_timer_in), timer_ch_size);
	save_pointer(NAME(m_timer_out), timer_ch_size);
	save_pointer(NAME(m_timer_counter), timer_ch_size);
	save_pointer(NAME(m_timer_readinc), timer_ch_size);
	
	save_item(NAME(m_sndcur));
	save_item(NAME(m_sndend));
	save_item(NAME(m_sound_dma_on));
	save_item(NAME(m_sndcur_buffer));
	save_item(NAME(m_snd_overrun));
	save_item(NAME(m_snd_int));

	save_pointer(NAME(m_sndcur_reg), sounddma_ch_size);
	save_pointer(NAME(m_sndend_reg), sounddma_ch_size);
	save_pointer(NAME(m_sndstop_reg), sounddma_ch_size);
	save_pointer(NAME(m_sndlast_reg), sounddma_ch_size);
	save_pointer(NAME(m_sndbuffer_ok), sounddma_ch_size);
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

	for (i=0; i<IRQ_SOURCES_SIZE; i++)
	{
		m_irq_status[i] = 0;
		m_irq_mask[i] = 0;
	}
	
	for (i=0; i<timer_ch_size; i++)
		m_timer[i]->adjust(attotime::never);

	m_sound_dma_on = false;
	for (i=0; i<sounddma_ch_size; i++)
		m_sndbuffer_ok[i] = false;
	// ...
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
	if (Which == IRQA)
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


// sound DMA

template <unsigned Which> READ32_MEMBER( arm_iomd_device::sdcur_r ) { return m_sndcur_reg[Which]; }
template <unsigned Which> WRITE32_MEMBER( arm_iomd_device::sdcur_w ) { COMBINE_DATA(&m_sndcur_reg[Which]); }
template <unsigned Which> READ32_MEMBER( arm_iomd_device::sdend_r ) 
{
	return (m_sndstop_reg[Which] << 31) | (m_sndlast_reg[Which] << 30) | (m_sndend_reg[Which] & 0x00fffff0); 
}

template <unsigned Which> WRITE32_MEMBER( arm_iomd_device::sdend_w ) 
{ 
	COMBINE_DATA(&m_sndend_reg[Which]); 
	m_sndend_reg[Which] &= 0x00fffff0;
	m_sndstop_reg[Which] = BIT(data, 31);
	m_sndlast_reg[Which] = BIT(data, 30);
}

READ32_MEMBER( arm_iomd_device::sdcr_r )
{
	return (m_sound_dma_on << 5) | dmaid_size;
}

WRITE32_MEMBER( arm_iomd_device::sdcr_w )
{
	m_sound_dma_on = BIT(data, 5);

	m_vidc->update_sound_mode(m_sound_dma_on);
	if (m_sound_dma_on)
	{
		m_sndcur_buffer = 0;
		sounddma_swap_buffer();
	}
	else
	{
		// ...
	}
	
	// TODO: bit 7 resets sound DMA
}

READ32_MEMBER( arm_iomd_device::sdst_r )
{
	return (m_snd_overrun << 2) | (m_snd_int << 1) | m_sndcur_buffer;
}

// video DMA

READ32_MEMBER( arm_iomd_device::vidcr_r )
{
	// bit 6: DRAM mode
	// bits 4-0: qword transfer
	return 0x40 | (m_video_enable << 5) | dmaid_size;
}

WRITE32_MEMBER( arm_iomd_device::vidcr_w )
{
	m_video_enable = BIT(data, 5);
	if (data & 0x80)
		throw emu_fatalerror("%s VIDCR LCD dual panel mode enabled", this->tag());
}

READ32_MEMBER( arm_iomd_device::vidend_r )
{
	return (m_vidend & 0x00fffff0);
}

WRITE32_MEMBER( arm_iomd_device::vidend_w )
{
	COMBINE_DATA(&m_vidend);
	m_vidend &= 0x00fffff0;
}

READ32_MEMBER( arm_iomd_device::vidinita_r )
{
	return (m_vidlast << 30) | (m_videqual << 29) | (m_vidinita & 0x1ffffff0);
}

WRITE32_MEMBER( arm_iomd_device::vidinita_w )
{
	COMBINE_DATA(&m_vidinita);
	m_vidinita &= 0x1ffffff0;
	m_vidlast = BIT(data, 30);
	m_videqual = BIT(data, 29);
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
		uint32_t src = m_vidinita;
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

inline void arm_iomd_device::sounddma_swap_buffer()
{
	m_sndcur = m_sndcur_reg[m_sndcur_buffer];
	m_sndend = m_sndcur + (m_sndend_reg[m_sndcur_buffer] + 0x10);
	m_sndbuffer_ok[m_sndcur_buffer] = true;
	
	// TODO: actual condition for int
	m_snd_overrun = false;
	m_snd_int = false;
}

WRITE_LINE_MEMBER( arm_iomd_device::sound_drq )
{
	if (!state)
		return;
		
	if (m_vidc->get_dac_mode() == true)
	{
		for (int ch=0;ch<2;ch++)
			m_vidc->write_dac32(ch, (m_host_space->read_word(m_sndcur + ch*2)));
		
		m_sndcur += 4;
		
		if (m_sndcur >= m_sndend)
		{
			// TODO: interrupt bit

			m_vidc->update_sound_mode(m_sound_dma_on);
			if (m_sound_dma_on)
			{
				m_sndbuffer_ok[m_sndcur_buffer] = false;
				m_sndcur_buffer ^= 1;
				m_snd_overrun = (m_sndbuffer_ok[m_sndcur_buffer] == false);
				sounddma_swap_buffer();
			}
			else
			{
				// ...
			}
		}
	}
	else
	{
		// ...
	}
}

