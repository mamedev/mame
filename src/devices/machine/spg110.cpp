// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

    SunPlus SPG110-series SoC peripheral emulation

    0032xx looks like it could be the same as 003dxx on spg2xx
    but the video seems to have differences, and data
    is fetched from private buffers filled by DMA instead of
    main space? tile attributes different? palette format different

**********************************************************************/

#include "emu.h"
#include "spg110.h"

DEFINE_DEVICE_TYPE(SPG110, spg110_device, "spg110", "SPG110 System-on-a-Chip")

spg110_device::spg110_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal)
	: unsp_device(mconfig, type, tag, owner, clock, internal)
	, device_mixer_interface(mconfig, *this, 2)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_spg_io(*this, "spg_io")
	, m_spg_video(*this, "spg_video")
	, m_spg_audio(*this, "spgaudio")
	, m_porta_out(*this)
	, m_portb_out(*this)
	, m_portc_out(*this)
	, m_porta_in(*this)
	, m_portb_in(*this)
	, m_portc_in(*this)
	, m_adc_in{{*this}, {*this}}
	, m_chip_sel(*this)
{
}


spg110_device::spg110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spg110_device(mconfig, SPG110, tag, owner, clock, address_map_constructor(FUNC(spg110_device::internal_map), this))
{
}

WRITE_LINE_MEMBER(spg110_device::videoirq_w)
{
	set_state_unsynced(UNSP_IRQ0_LINE, state);
}

void spg110_device::configure_spg_io(spg2xx_io_device* io)
{
	io->porta_in().set(FUNC(spg110_device::porta_r));
	io->portb_in().set(FUNC(spg110_device::portb_r));
	io->portc_in().set(FUNC(spg110_device::portc_r));
	io->porta_out().set(FUNC(spg110_device::porta_w));
	io->portb_out().set(FUNC(spg110_device::portb_w));
	io->portc_out().set(FUNC(spg110_device::portc_w));
	io->adc_in<0>().set(FUNC(spg110_device::adc_r<0>));
	io->adc_in<1>().set(FUNC(spg110_device::adc_r<1>));
	io->chip_select().set(FUNC(spg110_device::cs_w));
//  io->pal_read_callback().set(FUNC(spg110_device::get_pal_r));
//  io->write_timer_irq_callback().set(FUNC(spg110_device::timerirq_w));
//  io->write_uart_adc_irq_callback().set(FUNC(spg110_device::uartirq_w));
//  io->write_external_irq_callback().set(FUNC(spg110_device::extirq_w));
//  io->write_ffrq_tmr1_irq_callback().set(FUNC(spg110_device::ffreq1_w));
//  io->write_ffrq_tmr2_irq_callback().set(FUNC(spg110_device::ffreq2_w));
}

READ16_MEMBER(spg110_device::space_r)
{
	address_space &cpuspace = this->space(AS_PROGRAM);
	return cpuspace.read_word(offset);
}

WRITE_LINE_MEMBER(spg110_device::audioirq_w)
{
	set_state_unsynced(UNSP_FIQ_LINE, state);
}


void spg110_device::device_add_mconfig(machine_config &config)
{
	SPG24X_IO(config, m_spg_io, DERIVED_CLOCK(1, 1), DEVICE_SELF, m_screen);
	configure_spg_io(m_spg_io);

	SPG110_VIDEO(config, m_spg_video, DERIVED_CLOCK(1, 1), DEVICE_SELF, m_screen);
	m_spg_video->write_video_irq_callback().set(FUNC(spg110_device::videoirq_w));

	SPG110_AUDIO(config, m_spg_audio, DERIVED_CLOCK(1, 1));
	m_spg_audio->write_irq_callback().set(FUNC(spg110_device::audioirq_w));
	m_spg_audio->space_read_callback().set(FUNC(spg110_device::space_r));

	m_spg_audio->add_route(0, *this, 1.0, AUTO_ALLOC_INPUT, 0);
	m_spg_audio->add_route(1, *this, 1.0, AUTO_ALLOC_INPUT, 1);

}

void spg110_device::internal_map(address_map &map)
{
	map(0x000000, 0x000fff).ram();

	// vregs are at 2000?
	map(0x002010, 0x002015).rw(m_spg_video, FUNC(spg110_video_device::tmap0_regs_r), FUNC(spg110_video_device::tmap0_regs_w));
	map(0x002016, 0x00201b).rw(m_spg_video, FUNC(spg110_video_device::tmap1_regs_r), FUNC(spg110_video_device::tmap1_regs_w));

#if 1 // more vregs?
	map(0x00201c, 0x00201c).w(m_spg_video, FUNC(spg110_video_device::spg110_201c_w));

	map(0x002020, 0x002020).w(m_spg_video, FUNC(spg110_video_device::spg110_2020_w));

	map(0x002028, 0x002028).rw(m_spg_video, FUNC(spg110_video_device::spg110_2028_r), FUNC(spg110_video_device::spg110_2028_w));
	map(0x002029, 0x002029).rw(m_spg_video, FUNC(spg110_video_device::spg110_2029_r), FUNC(spg110_video_device::spg110_2029_w));

	map(0x002031, 0x002031).w(m_spg_video, FUNC(spg110_video_device::spg110_2031_w)); // sometimes 14a?
	map(0x002032, 0x002032).w(m_spg_video, FUNC(spg110_video_device::spg110_2032_w)); // always 14a?
	map(0x002033, 0x002033).w(m_spg_video, FUNC(spg110_video_device::spg110_2033_w));
	map(0x002034, 0x002034).w(m_spg_video, FUNC(spg110_video_device::spg110_2034_w));
	map(0x002035, 0x002035).w(m_spg_video, FUNC(spg110_video_device::spg110_2035_w));
	map(0x002036, 0x002036).w(m_spg_video, FUNC(spg110_video_device::spg110_2036_w)); // possible scroll register?
	map(0x002037, 0x002037).rw(m_spg_video, FUNC(spg110_video_device::spg110_2037_r), FUNC(spg110_video_device::spg110_2037_w));

	map(0x002039, 0x002039).w(m_spg_video, FUNC(spg110_video_device::spg110_2039_w));

	map(0x00203c, 0x00203c).w(m_spg_video, FUNC(spg110_video_device::spg110_203c_w));

	map(0x00203d, 0x00203d).w(m_spg_video, FUNC(spg110_video_device::spg110_203d_w)); // possible scroll register?

	map(0x002042, 0x002042).rw(m_spg_video, FUNC(spg110_video_device::spg110_2042_r),FUNC(spg110_video_device::spg110_2042_w));

	map(0x002045, 0x002045).w(m_spg_video, FUNC(spg110_video_device::spg110_2045_w));
#endif

	// seems to be 16 entries for.. something? on jak_capb these seem connected to the palette DMA operations, 0x2050 for 0x8000, 0x2051 for 0x8020, 0x2052 for 0x8040 etc. maybe 1 bit per pen?
	map(0x002050, 0x00205f).ram().w(m_spg_video, FUNC(spg110_video_device::spg110_205x_w)).share("spg_video:palctrlram");

	// everything (dma? and interrupt flag?!)
	map(0x002060, 0x002060).w(m_spg_video, FUNC(spg110_video_device::dma_dst_w));
	map(0x002061, 0x002061).w(m_spg_video, FUNC(spg110_video_device::dma_unk_2061_w));
	map(0x002062, 0x002062).rw(m_spg_video, FUNC(spg110_video_device::dma_len_status_r),FUNC(spg110_video_device::dma_len_trigger_w));
	map(0x002063, 0x002063).rw(m_spg_video, FUNC(spg110_video_device::spg110_2063_r),FUNC(spg110_video_device::spg110_2063_w)); // Video IRQ source / ack (3 different things checked here instead of 2 on spg2xx?)
	map(0x002064, 0x002064).w(m_spg_video, FUNC(spg110_video_device::dma_dst_step_w));
	map(0x002065, 0x002065).rw(m_spg_video, FUNC(spg110_video_device::dma_manual_r), FUNC(spg110_video_device::dma_manual_w));
	map(0x002066, 0x002066).w(m_spg_video, FUNC(spg110_video_device::dma_src_w));
	map(0x002067, 0x002067).w(m_spg_video, FUNC(spg110_video_device::dma_unk_2067_w));
	map(0x002068, 0x002068).w(m_spg_video, FUNC(spg110_video_device::dma_src_step_w));

	map(0x002100, 0x0021ff).ram(); // jak_spdmo only
	map(0x002200, 0x0022ff).ram(); // looks like per-pen brightness or similar? strange because palette isn't memory mapped here (maybe rowscroll?)

	/// sound registers? seems to be 8 long entries, only uses up to 0x7f? (register mapping seems similar to spg2xx, maybe with less channels?)
	map(0x003000, 0x00307f).rw(m_spg_audio, FUNC(spg110_audio_device::audio_r), FUNC(spg110_audio_device::audio_w));
	map(0x003080, 0x0030ff).ram(); // extra ram? doesn't seem to be phase, and there only appear to be 8 channels on SPG110

	map(0x003100, 0x00310f).rw(m_spg_audio, FUNC(spg110_audio_device::audio_ctrl_r), FUNC(spg2xx_audio_device::audio_ctrl_w));


	// 0032xx looks like it could be the same as 003d00 on spg2xx
	map(0x003200, 0x00322f).rw(m_spg_io, FUNC(spg2xx_io_device::io_r), FUNC(spg2xx_io_device::io_w));
}


void spg110_device::device_start()
{
	unsp_device::device_start();

	m_porta_out.resolve_safe();
	m_portb_out.resolve_safe();
	m_portc_out.resolve_safe();
	m_porta_in.resolve_safe(0);
	m_portb_in.resolve_safe(0);
	m_portc_in.resolve_safe(0);
	m_adc_in[0].resolve_safe(0x0fff);
	m_adc_in[1].resolve_safe(0x0fff);
	m_chip_sel.resolve_safe();
}

void spg110_device::device_reset()
{
	unsp_device::device_reset();
}

