// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    ARM IOMD device emulation

    ARM7 SoC or stand-alone device, upgraded version(s) of the IOC found in Acorn Archimedes.

    TODO:
    - IOCR / IOLINES hookups can be further improved, also DDR bits needs verifying;
    - word-boundary accesses for 8-bit ports;
    - split into different types, add quick notes about where they diverges do in this header;
    - keyboard/mouse interface hookup is wrong for PS/2 and unimplemented for quadrature.
      I guess we can use connectors over a custom handling, with a terminal mock for testing it
      without the overhead of everything else.

**************************************************************************************************/

#include "emu.h"
#include "arm_iomd.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ARM_IOMD, arm_iomd_device, "arm_iomd", "ARM IOMD controller")
// TODO: ssfindo.cpp actually uses a Cirrus Logic 7500FE, is it rebadged?
DEFINE_DEVICE_TYPE(ARM7500FE_IOMD, arm7500fe_iomd_device, "arm_7500fe_soc", "ARM 7500FE SoC")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arm_iomd_device - constructor
//-------------------------------------------------

void arm_iomd_device::base_map(address_map &map)
{
	// I/O
	map(0x000, 0x003).rw(FUNC(arm_iomd_device::iocr_r), FUNC(arm_iomd_device::iocr_w));
	map(0x004, 0x007).rw(FUNC(arm_iomd_device::kbddat_r), FUNC(arm_iomd_device::kbddat_w));
	map(0x008, 0x00b).rw(FUNC(arm_iomd_device::kbdcr_r), FUNC(arm_iomd_device::kbdcr_w));

	// interrupt A/B/fiq + master clock controls
	map(0x010, 0x013).r(FUNC(arm_iomd_device::irqst_r<IRQA>));
	map(0x014, 0x017).rw(FUNC(arm_iomd_device::irqrq_r<IRQA>), FUNC(arm_iomd_device::irqrq_w<IRQA>));
	map(0x018, 0x01b).rw(FUNC(arm_iomd_device::irqmsk_r<IRQA>), FUNC(arm_iomd_device::irqmsk_w<IRQA>));

	map(0x020, 0x023).r(FUNC(arm_iomd_device::irqst_r<IRQB>));
	map(0x024, 0x027).rw(FUNC(arm_iomd_device::irqrq_r<IRQB>), FUNC(arm_iomd_device::irqrq_w<IRQB>));
	map(0x028, 0x02b).rw(FUNC(arm_iomd_device::irqmsk_r<IRQB>), FUNC(arm_iomd_device::irqmsk_w<IRQB>));

//  map(0x030, 0x033).r(FUNC(arm_iomd_device::fiqst_r));
//  map(0x034, 0x037).rw(FUNC(arm_iomd_device::fiqrq_r), FUNC(arm_iomd_device::fiqrq_w));
//  map(0x038, 0x03b).rw(FUNC(arm_iomd_device::fiqmsk_r), FUNC(arm_iomd_device::fiqmsk_w));

	// timers
	map(0x040, 0x043).rw(FUNC(arm_iomd_device::tNlow_r<0>), FUNC(arm_iomd_device::tNlow_w<0>));
	map(0x044, 0x047).rw(FUNC(arm_iomd_device::tNhigh_r<0>), FUNC(arm_iomd_device::tNhigh_w<0>));
	map(0x048, 0x04b).w(FUNC(arm_iomd_device::tNgo_w<0>));
	map(0x04c, 0x04f).w(FUNC(arm_iomd_device::tNlatch_w<0>));

	map(0x050, 0x053).rw(FUNC(arm_iomd_device::tNlow_r<1>), FUNC(arm_iomd_device::tNlow_w<1>));
	map(0x054, 0x057).rw(FUNC(arm_iomd_device::tNhigh_r<1>), FUNC(arm_iomd_device::tNhigh_w<1>));
	map(0x058, 0x05b).w(FUNC(arm_iomd_device::tNgo_w<1>));
	map(0x05c, 0x05f).w(FUNC(arm_iomd_device::tNlatch_w<1>));
	// ROM control
//  map(0x080, 0x083).rw(FUNC(arm_iomd_device::romcr_r<0>), FUNC(arm_iomd_device::romcr_w<0>));
//  map(0x084, 0x087).rw(FUNC(arm_iomd_device::romcr_r<1>), FUNC(arm_iomd_device::romcr_w<1>));
//  map(0x08c, 0x08f).rw(FUNC(arm_iomd_device::refcr_r), FUNC(arm_iomd_device::refcr_w));

	// device identifiers
	map(0x094, 0x097).r(FUNC(arm_iomd_device::id_r<0>));
	map(0x098, 0x09b).r(FUNC(arm_iomd_device::id_r<1>));
	map(0x09c, 0x09f).r(FUNC(arm_iomd_device::version_r));
	// mouse
//  map(0x0a0, 0x0a3) // ...

	// I/O control
//  map(0x0c4, 0x0c7).rw(FUNC(arm_iomd_device::iotcr_r), FUNC(arm_iomd_device::iotcr_w));
//  map(0x0c8, 0x0cb).rw(FUNC(arm_iomd_device::ectcr_r), FUNC(arm_iomd_device::ectcr_w));
	// sound DMA
	// TODO: IOMD actually have a sd1* register, rework this
	map(0x180, 0x183).rw(FUNC(arm_iomd_device::sdcur_r<0>), FUNC(arm_iomd_device::sdcur_w<0>));
	map(0x184, 0x187).rw(FUNC(arm_iomd_device::sdend_r<0>), FUNC(arm_iomd_device::sdend_w<0>));
	map(0x188, 0x18b).rw(FUNC(arm_iomd_device::sdcur_r<1>), FUNC(arm_iomd_device::sdcur_w<1>));
	map(0x18c, 0x18f).rw(FUNC(arm_iomd_device::sdend_r<1>), FUNC(arm_iomd_device::sdend_w<1>));
	map(0x190, 0x193).rw(FUNC(arm_iomd_device::sdcr_r), FUNC(arm_iomd_device::sdcr_w));
	map(0x194, 0x197).r(FUNC(arm_iomd_device::sdst_r));

	// video DMA
//  map(0x1c0, 0x1c3).rw(FUNC(arm_iomd_device::curscur_r), FUNC(arm_iomd_device::curscur_w));
	map(0x1c4, 0x1c7).rw(FUNC(arm_iomd_device::cursinit_r), FUNC(arm_iomd_device::cursinit_w));

//  map(0x1d0, 0x1d3).rw(FUNC(arm_iomd_device::vidcura_r), FUNC(arm_iomd_device::vidcura_w));
	map(0x1d4, 0x1d7).rw(FUNC(arm_iomd_device::vidend_r), FUNC(arm_iomd_device::vidend_w));
//  map(0x1d8, 0x1db).rw(FUNC(arm_iomd_device::vidstart_r), FUNC(arm_iomd_device::vidstart_w));
	map(0x1dc, 0x1df).rw(FUNC(arm_iomd_device::vidinita_r), FUNC(arm_iomd_device::vidinita_w));
	map(0x1e0, 0x1e3).rw(FUNC(arm_iomd_device::vidcr_r), FUNC(arm_iomd_device::vidcr_w));

	// interrupt DMA
	map(0x1f0, 0x1f3).r(FUNC(arm_iomd_device::irqst_r<IRQDMA>));
	map(0x1f4, 0x1f7).rw(FUNC(arm_iomd_device::irqrq_r<IRQDMA>), FUNC(arm_iomd_device::irqrq_w<IRQDMA>));
	map(0x1f8, 0x1fb).rw(FUNC(arm_iomd_device::irqmsk_r<IRQDMA>), FUNC(arm_iomd_device::irqmsk_w<IRQDMA>));

//  TODO: iomd2 has extra regs in 0x200-0x3ff area, others NOPs / mirrors?
}

void arm_iomd_device::map(address_map &map)
{
	arm_iomd_device::base_map(map);
//  map(0x088, 0x08b).rw(FUNC(arm_iomd_device::dramcr_r), FUNC(arm_iomd_device::dramcr_w));
	// VRAM control
//  map(0x08c, 0x08f).rw(FUNC(arm_iomd_device::vrefcr_r), FUNC(arm_iomd_device::vrefcr_w));
	// flyback line size
//  map(0x090, 0x093).rw(FUNC(arm_iomd_device::fsize_r), FUNC(arm_iomd_device::fsize_w));
	// quadrature mouse control
//  map(0x0a0, 0x0a3).rw(FUNC(arm_iomd_device::mousex_r), FUNC(arm_iomd_device::mousex_w));
//  map(0x0a4, 0x0a7).rw(FUNC(arm_iomd_device::mousey_r), FUNC(arm_iomd_device::mousey_w));
	// DACK timing control
//  map(0x0c0, 0x0c3).rw(FUNC(arm_iomd_device::dmatcr_r), FUNC(arm_iomd_device::dmatcr_w));
	// DMA external control
//  map(0x0cc, 0x0cf).rw(FUNC(arm_iomd_device::dmaext_r), FUNC(arm_iomd_device::dmaext_w));
	// I/O DMA, similar structure as sound DMA
//  map(0x100, 0x11f) ch 0
//  map(0x120, 0x13f) ch 1
//  map(0x140, 0x15f) ch 2
//  map(0x160, 0x17f) ch 3
	// sound DMA
//  map(0x1a0, 0x1bf) ch 1

}

arm_iomd_device::arm_iomd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_vidc(*this, finder_base::DUMMY_TAG)
	, m_kbdc(*this, finder_base::DUMMY_TAG)
	, m_iocr_read_od_cb(*this, 1)
	, m_iocr_write_od_cb(*this)
	, m_iocr_read_id_cb(*this, 1)
	, m_iocr_write_id_cb(*this)
	, m_sndcur(0)
	, m_sndend(0)
	, m_sndcur_reg{ 0, 0 }
	, m_sndend_reg{ 0, 0 }
	, m_sndstop_reg{ false, false }
	, m_sndlast_reg{ false, false }
	, m_sndbuffer_ok{ false, false }
	, m_sound_dma_on(false)
	, m_sndcur_buffer(0)
{
}

arm_iomd_device::arm_iomd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: arm_iomd_device(mconfig, ARM_IOMD, tag, owner, clock)
{
	m_id = 0xd4e7;
	m_version = 0;
}

void arm7500fe_iomd_device::map(address_map &map)
{
	arm_iomd_device::base_map(map);

	map(0x00c, 0x00f).rw(FUNC(arm7500fe_iomd_device::iolines_r), FUNC(arm7500fe_iomd_device::iolines_w));
	// master clock controls
//  map(0x01c, 0x01f).rw(FUNC(arm7500fe_iomd_device::susmode_r), FUNC(arm7500fe_iomd_device::susmode_w));
//  map(0x02c, 0x02f).w(FUNC(arm7500fe_iomd_device::stopmode_w));
	map(0x03c, 0x03f).rw(FUNC(arm7500fe_iomd_device::clkctl_r), FUNC(arm7500fe_iomd_device::clkctl_w));
	// interrupt C/D
	map(0x060, 0x063).r(FUNC(arm7500fe_iomd_device::irqst_r<IRQC>));
	map(0x064, 0x067).rw(FUNC(arm7500fe_iomd_device::irqrq_r<IRQC>), FUNC(arm7500fe_iomd_device::irqrq_w<IRQC>));
	map(0x068, 0x06b).rw(FUNC(arm7500fe_iomd_device::irqmsk_r<IRQC>), FUNC(arm7500fe_iomd_device::irqmsk_w<IRQC>));
//  map(0x06c, 0x06f).rw(FUNC(arm7500fe_iomd_device::vidmux_r), FUNC(arm7500fe_iomd_device::vidmux_w));
	map(0x070, 0x073).r(FUNC(arm7500fe_iomd_device::irqst_r<IRQD>));
	map(0x074, 0x077).rw(FUNC(arm7500fe_iomd_device::irqrq_r<IRQD>), FUNC(arm7500fe_iomd_device::irqrq_w<IRQD>));
	map(0x078, 0x07b).rw(FUNC(arm7500fe_iomd_device::irqmsk_r<IRQD>), FUNC(arm7500fe_iomd_device::irqmsk_w<IRQD>));

	// PS/2 mouse
//  map(0x0a8, 0x0ab).rw(FUNC(arm7500fe_iomd_device::msedat_r), FUNC(arm7500fe_iomd_device::msedat_w));
	map(0x0ac, 0x0af).rw(FUNC(arm7500fe_iomd_device::msecr_r), FUNC(arm7500fe_iomd_device::msecr_w));
	// I/O control
//  map(0x0cc, 0x0cf).rw(FUNC(arm7500fe_iomd_device::astcr_r), FUNC(arm7500fe_iomd_device::astcr_w));
	// RAM control
//  map(0x0d0, 0x0d3).rw(FUNC(arm7500fe_iomd_device::dramctl_r), FUNC(arm7500fe_iomd_device::dramctl_w));
//  map(0x0d4, 0x0d7).rw(FUNC(arm7500fe_iomd_device::selfref_r), FUNC(arm7500fe_iomd_device::selfref_w));
	// A/D converter
//  map(0x0e0, 0x0e3).rw(FUNC(arm7500fe_iomd_device::atodicr_r), FUNC(arm7500fe_iomd_device::atodicr_w));
//  map(0x0e4, 0x0e7).r(FUNC(arm7500fe_iomd_device::atodsr_r));
//  map(0x0e8, 0x0eb).rw(FUNC(arm7500fe_iomd_device::atodcc_r), FUNC(arm7500fe_iomd_device::atodcc_w));
//  map(0x0ec, 0x0ef).r(FUNC(arm7500fe_iomd_device::atodcnt_r<0>));
//  map(0x0f0, 0x0f3).r(FUNC(arm7500fe_iomd_device::atodcnt_r<1>));
//  map(0x0f4, 0x0f7).r(FUNC(arm7500fe_iomd_device::atodcnt_r<2>));
//  map(0x0f8, 0x0fb).r(FUNC(arm7500fe_iomd_device::atodcnt_r<3>));
	// video DMA
//  map(0x1c8, 0x1cb).rw(FUNC(arm7500fe_iomd_device::vidcurb_r), FUNC(arm7500fe_iomd_device::vidcurb_w));
//  map(0x1e8, 0x1eb).rw(FUNC(arm7500fe_iomd_device::vidinitb_r), FUNC(arm7500fe_iomd_device::vidinitb_w));

}

arm7500fe_iomd_device::arm7500fe_iomd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arm_iomd_device(mconfig, ARM7500FE_IOMD, tag, owner, clock)
	, m_iolines_read_cb(*this, 0xff)
	, m_iolines_write_cb(*this)
{
	m_id = 0xaa7c;
	m_version = 0;
}

// vanilla ARM7500 m_id = 0x5b98; m_version = 0;
// IOMD2 m_id = 0xd5e8; m_version = 1;

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------

void arm_iomd_device::device_add_mconfig(machine_config &config)
{
	//DEVICE(config, ...);
	//TODO: keyboard and mouse interfaces at very least, also they differs by device type
}

void arm7500fe_iomd_device::device_add_mconfig(machine_config &config)
{
	//DEVICE(config, ...);
	//TODO: above plus new sub-devices
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arm_iomd_device::device_start()
{
	save_item(NAME(m_iocr_ddr));
	save_item(NAME(m_video_enable));
	save_item(NAME(m_vidinita));
	save_item(NAME(m_vidend));
	save_item(NAME(m_vidlast));
	save_item(NAME(m_videqual));
	save_item(NAME(m_cursor_enable));
	save_item(NAME(m_cursinit));
	save_pointer(NAME(m_irq_mask), std::size(m_irq_mask));
	save_pointer(NAME(m_irq_status), std::size(m_irq_status));

	m_host_space = &m_host_cpu->space(AS_PROGRAM);

	m_timer[0] = timer_alloc(FUNC(arm_iomd_device::timer_elapsed), this);
	m_timer[1] = timer_alloc(FUNC(arm_iomd_device::timer_elapsed), this);
	save_pointer(NAME(m_timer_in), std::size(m_timer_in));
	save_pointer(NAME(m_timer_out), std::size(m_timer_out));
	save_pointer(NAME(m_timer_counter), std::size(m_timer_counter));
	save_pointer(NAME(m_timer_readinc), std::size(m_timer_readinc));

	save_item(NAME(m_sndcur));
	save_item(NAME(m_sndend));
	save_item(NAME(m_sound_dma_on));
	save_item(NAME(m_sndcur_buffer));

	save_pointer(NAME(m_sndcur_reg), std::size(m_sndcur_reg));
	save_pointer(NAME(m_sndend_reg), std::size(m_sndend_reg));
	save_pointer(NAME(m_sndstop_reg), std::size(m_sndstop_reg));
	save_pointer(NAME(m_sndlast_reg), std::size(m_sndlast_reg));
	save_pointer(NAME(m_sndbuffer_ok), std::size(m_sndbuffer_ok));

	// TODO: jumps to EASI space at $0c0016xx for RiscPC if POR is on?
}

void arm7500fe_iomd_device::device_start()
{
	arm_iomd_device::device_start();

	save_item(NAME(m_iolines_ddr));

	save_item(NAME(m_cpuclk_divider));
	save_item(NAME(m_memclk_divider));
	save_item(NAME(m_ioclk_divider));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arm_iomd_device::device_reset()
{
	m_iocr_ddr = 0x0b;
	m_video_enable = false;
	// TODO: defaults for these
	m_vidinita = 0;
	m_vidend = 0;

	std::fill_n(m_irq_status, std::size(m_irq_status), 0);
	std::fill_n(m_irq_mask, std::size(m_irq_mask), 0);

	for (int i = 0; i < std::size(m_timer); i++)
		m_timer[i]->adjust(attotime::never);

	m_sndcur = 0;
	m_sndend = 0;
	std::fill_n(m_sndcur_reg, std::size(m_sndcur_reg), 0);
	std::fill_n(m_sndend_reg, std::size(m_sndend_reg), 0);
	std::fill_n(m_sndstop_reg, std::size(m_sndstop_reg), false);
	std::fill_n(m_sndlast_reg, std::size(m_sndlast_reg), false);
	std::fill_n(m_sndbuffer_ok, std::size(m_sndbuffer_ok), false);
	m_sound_dma_on = false;
	m_sndcur_buffer = 0;

	// ...
}

void arm7500fe_iomd_device::device_reset()
{
	arm_iomd_device::device_reset();

	m_cpuclk_divider = m_ioclk_divider = m_memclk_divider = false;
	refresh_host_cpu_clocks();

	m_iolines_ddr = 0xff;
}

TIMER_CALLBACK_MEMBER(arm_iomd_device::timer_elapsed)
{
	trigger_irq<IRQA>((uint8_t)param);
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

// TODO: nINT1
u32 arm_iomd_device::iocr_r()
{
	u8 res = 0;

	res = m_iocr_read_id_cb() << 3;
	res|= m_iocr_read_od_cb[1]() << 1;
	res|= m_iocr_read_od_cb[0]() << 0;

	return (m_vidc->flyback_r() << 7) | 0x34 | (res & m_iocr_ddr);
}

void arm_iomd_device::iocr_w(u32 data)
{
	m_iocr_ddr = (data & 0x0b);
	m_iocr_write_id_cb(BIT(m_iocr_ddr,3));
	m_iocr_write_od_cb[1](BIT(m_iocr_ddr,1));
	m_iocr_write_od_cb[0](BIT(m_iocr_ddr,0));
}

u32 arm_iomd_device::kbddat_r()
{
	if (m_kbdc.found())
		return m_kbdc->data_r();

	logerror("%s attempted to read kbddat with no controller\n", this->tag());
	return 0xff;
}

u32 arm_iomd_device::kbdcr_r()
{
	if (m_kbdc.found())
		return m_kbdc->status_r();

	logerror("%s attempted to read kbdcr with no controller\n", this->tag());
	return 0xff;
}

void arm_iomd_device::kbddat_w(u32 data)
{
	if (m_kbdc.found())
	{
		m_kbdc->data_w(data & 0xff);
		return;
	}

	logerror("%s attempted to write %02x on kbddat with no controller\n", this->tag(),data & 0xff);
}

void arm_iomd_device::kbdcr_w(u32 data)
{
	if (m_kbdc.found())
	{
		m_kbdc->command_w(data & 0xff);
		return;
	}

	logerror("%s attempted to write %02x on kbdcr with no controller\n", this->tag(),data & 0xff);
}

u32 arm7500fe_iomd_device::msecr_r()
{
	// a7000p wants a TX empty otherwise it outright refuses to boot.
	return 0x80;
}

void arm7500fe_iomd_device::msecr_w(u32 data)
{
	// ...
}

u32 arm7500fe_iomd_device::iolines_r()
{
	return m_iolines_read_cb() & m_iolines_ddr;
}

void arm7500fe_iomd_device::iolines_w(u32 data)
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
		m_host_cpu->pulse_input_line(arm7_cpu_device::ARM7_IRQ_LINE, m_host_cpu->minimum_quantum_time());
}

template <unsigned Which> inline void arm_iomd_device::trigger_irq(u8 irq_type)
{
	m_irq_status[Which] |= irq_type;
	flush_irq(Which);
}

template <unsigned Which> u32 arm_iomd_device::irqst_r()
{
	return m_irq_status[Which];
}

template <unsigned Which> u32 arm_iomd_device::irqrq_r()
{
	return m_irq_status[Which] & m_irq_mask[Which];
}

template <unsigned Which> u32 arm_iomd_device::irqmsk_r()
{
	return m_irq_mask[Which];
}

template <unsigned Which> void arm_iomd_device::irqrq_w(u32 data)
{
	u8 res = m_irq_status[Which] & ~data;
	if (Which == IRQA)
		res = update_irqa_type(res);
	m_irq_status[Which] = res;
	flush_irq(Which);
}

template <unsigned Which> void arm_iomd_device::irqmsk_w(u32 data)
{
	m_irq_mask[Which] = data & 0xff;
	flush_irq(Which);
}

// master clock control
inline void arm7500fe_iomd_device::refresh_host_cpu_clocks()
{
	m_host_cpu->set_unscaled_clock(this->clock() >> (m_cpuclk_divider == false));
}

u32 arm7500fe_iomd_device::clkctl_r()
{
	return (m_cpuclk_divider << 2) | (m_memclk_divider << 1) | (m_ioclk_divider);
}

void arm7500fe_iomd_device::clkctl_w(u32 data)
{
	m_cpuclk_divider = BIT(data, 2);
	m_memclk_divider = BIT(data, 1);
	m_ioclk_divider = BIT(data, 0);
	refresh_host_cpu_clocks();
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
		m_timer[Which]->adjust(attotime::from_usec(val), Which ? 0x40 : 0x20, attotime::from_usec(val));
}

// TODO: live updates aren't really supported here
template <unsigned Which> u32 arm_iomd_device::tNlow_r()
{
	return m_timer_out[Which] & 0xff;
}

template <unsigned Which> u32 arm_iomd_device::tNhigh_r()
{
	return (m_timer_out[Which] >> 8) & 0xff;
}

template <unsigned Which> void arm_iomd_device::tNlow_w(u32 data)
{
	m_timer_in[Which] = (m_timer_in[Which] & 0xff00) | (data & 0xff);
}

template <unsigned Which> void arm_iomd_device::tNhigh_w(u32 data)
{
	m_timer_in[Which] = (m_timer_in[Which] & 0x00ff) | ((data & 0xff) << 8);
}

template <unsigned Which> void arm_iomd_device::tNgo_w(u32 data)
{
	m_timer_counter[Which] = m_timer_in[Which];
	trigger_timer(Which);
}

template <unsigned Which> void arm_iomd_device::tNlatch_w(u32 data)
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

// device identifiers
template <unsigned Nibble> u32 arm_iomd_device::id_r()
{
	return (m_id >> (Nibble*8)) & 0xff;
}

u32 arm_iomd_device::version_r()
{
	return m_version;
}

// sound DMA

template <unsigned Which> u32 arm_iomd_device::sdcur_r() { return m_sndcur_reg[Which]; }
template <unsigned Which> void arm_iomd_device::sdcur_w(offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_sndcur_reg[Which]); }
template <unsigned Which> u32 arm_iomd_device::sdend_r()
{
	return (m_sndstop_reg[Which] << 31) | (m_sndlast_reg[Which] << 30) | (m_sndend_reg[Which] & 0x00fffff0);
}

template <unsigned Which> void arm_iomd_device::sdend_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sndend_reg[Which]);
	m_sndend_reg[Which] &= 0x00fffff0;
	m_sndstop_reg[Which] = BIT(data, 31);
	m_sndlast_reg[Which] = BIT(data, 30);
	m_sndbuffer_ok[Which] = true;
}

u32 arm_iomd_device::sdcr_r()
{
	return (m_sound_dma_on << 5) | dmaid_size;
}

void arm_iomd_device::sdcr_w(u32 data)
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

	// TODO: sound DMA reset
	// eats samples in ppcar
//  if (BIT(data, 7))
//      m_sndbuffer_ok[0] = m_sndbuffer_ok[1] = false;
}

u32 arm_iomd_device::sdst_r()
{
	// TODO: confirm implementation
	bool sound_overrun = m_sndbuffer_ok[0] == false && m_sndbuffer_ok[1] == false;
	bool sound_int = m_sndbuffer_ok[0] == false || m_sndbuffer_ok[1] == false;

	return (sound_overrun << 2) | (sound_int << 1) | m_sndcur_buffer;
}

// video DMA
u32 arm_iomd_device::cursinit_r()
{
	return m_cursinit;
}

void arm_iomd_device::cursinit_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_cursinit);
	m_cursinit &= 0x1ffffff0;
	m_cursor_enable = true;
	m_vidc->set_cursor_enable(m_cursor_enable);
}

u32 arm_iomd_device::vidcr_r()
{
	// bit 6: DRAM mode
	// bits 4-0: qword transfer
	return 0x40 | (m_video_enable << 5) | dmaid_size;
}

void arm_iomd_device::vidcr_w(u32 data)
{
	m_video_enable = BIT(data, 5);
	if (m_video_enable == false)
	{
		m_cursor_enable = false;
		m_vidc->set_cursor_enable(m_cursor_enable);
	}

	if (data & 0x80)
		throw emu_fatalerror("%s VIDCR LCD dual panel mode enabled", this->tag());
}

u32 arm_iomd_device::vidend_r()
{
	return (m_vidend & 0x00fffff0);
}

void arm_iomd_device::vidend_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vidend);
	m_vidend &= 0x00fffff0;
}

u32 arm_iomd_device::vidinita_r()
{
	return (m_vidlast << 30) | (m_videqual << 29) | (m_vidinita & 0x1ffffff0);
}

void arm_iomd_device::vidinita_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_vidinita);
	m_vidinita &= 0x1ffffff0;
	m_vidlast = BIT(data, 30);
	m_videqual = BIT(data, 29);
}


//**************************************************************************
//  IRQ/DRQ/Reset signals
//**************************************************************************

void arm_iomd_device::vblank_irq(int state)
{
	if (!state)
		return;

	trigger_irq<IRQA>(0x08);
	if (m_video_enable == true)
	{
		// TODO: much more complex, last/end regs, start regs and eventually LCD hooks
		u32 src = m_vidinita;
		u32 size = m_vidend;

		// TODO: vidcur can be readback, support it once anything makes use of the 0x1d0 reg for obvious reasons
		// (and using m_ prefix is intentional too)
		for (u32 m_vidcur = 0; m_vidcur<size; m_vidcur++)
		{
			m_vidc->write_vram(m_vidcur, m_host_space->read_byte(src));
			src++;
			src &= 0x1fffffff;
		}

		if (m_cursor_enable == true)
		{
			src = m_cursinit;
			size = m_vidc->get_cursor_size();

			// TODO: same as above
			for (u32 m_curscur = 0; m_curscur<size; m_curscur++)
			{
				m_vidc->write_cram(m_curscur, m_host_space->read_byte(src));
				src++;
				src &= 0x1fffffff;
			}
		}
	}
}

inline void arm_iomd_device::sounddma_swap_buffer()
{
	m_sndcur = m_sndcur_reg[m_sndcur_buffer];
	m_sndend = m_sndcur + (m_sndend_reg[m_sndcur_buffer] + 0x10);
//  m_sndbuffer_ok[m_sndcur_buffer] = true;
}

void arm_iomd_device::sound_drq(int state)
{
	if (!state)
		return;

	if (m_vidc->get_dac_mode() == true)
	{
		if (!m_sndbuffer_ok[m_sndcur_buffer])
			return;

		for (int ch = 0; ch < 2; ch++)
			m_vidc->write_dac32(ch, (m_host_space->read_word(m_sndcur + ch*2)));

		m_sndcur += 4;

		if (m_sndcur >= m_sndend)
		{
			m_vidc->update_sound_mode(m_sound_dma_on);
			if (m_sound_dma_on)
			{
				m_sndbuffer_ok[m_sndcur_buffer] = false;
				m_sndcur_buffer ^= 1;
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

void arm_iomd_device::keyboard_irq(int state)
{
	printf("IRQ %d\n",state);
	if (!state)
		return;

	trigger_irq<IRQB>(0x80);
}

void arm_iomd_device::keyboard_reset(int state)
{
	printf("RST %d\n",state);
}


