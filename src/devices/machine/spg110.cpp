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

spg110_device::spg110_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal) :
	unsp_device(mconfig, type, tag, owner, clock, internal),
	device_mixer_interface(mconfig, *this),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_spg_io(*this, "spg_io"),
	m_spg_video(*this, "spg_video"),
	m_spg_audio(*this, "spgaudio"),
	m_porta_out(*this),
	m_portb_out(*this),
	m_portc_out(*this),
	m_porta_in(*this, 0),
	m_portb_in(*this, 0),
	m_portc_in(*this, 0),
	m_adc_in(*this, 0x0fff),
	m_chip_sel(*this)
{
}


spg110_device::spg110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spg110_device(mconfig, SPG110, tag, owner, clock, address_map_constructor(FUNC(spg110_device::internal_map), this))
{
}

void spg110_device::videoirq_w(int state)
{
	set_state_unsynced(UNSP_IRQ0_LINE, state);
}

void spg110_device::timerirq_w(int state)
{
	set_state_unsynced(UNSP_IRQ3_LINE, state);
}

void spg110_device::uartirq_w(int state)
{
	set_state_unsynced(UNSP_IRQ4_LINE, state);
}

void spg110_device::extirq_w(int state)
{
	// External Int1 is IRQ4 (was 5 on SPG2xx)
	// External Int2 is IRQ2 (was 5 on SPG2xx)
	set_state_unsynced(UNSP_IRQ4_LINE, state);
}

void spg110_device::ffreq1_w(int state)
{
	set_state_unsynced(UNSP_IRQ5_LINE, state);
}

void spg110_device::ffreq2_w(int state)
{
	set_state_unsynced(UNSP_IRQ6_LINE, state);
}

// notes about IRQ differences from 2xx
//
// TMB1 / TMB2 are IRQ7 (same as SPG2xx)
// Key Change is IRQ4 (was 7 on SPG2xx)
// LVD (Low Voltage Reset) is IRQ6 (doesn't exist on SPG2xx?)
// ADC is IRQ1 (was 3 on SPG2xx)
//
// on SPG2xx 0x3D2E can redirect any other interrupt to the FIQ
// on SPG110 FIQ is always from SPUIRQ (sound)
//
// on SPG2xx SPU_Ch_Irq (sound) is IRQ1, and SPU_Env_Irq / SPU_Beat_Irq is IRQ4
// SPU_Ch_Irq / SPU_Env_Irq and SPU_Beat_Irq don't exist on SPG110? (assume SPU_Ch_Irq is SPUIRQ?)
//
// SPG2xx has SPI Interrupt on IRQ3, no SPI interrupt on SPG110?

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
	io->adc_in<2>().set(FUNC(spg110_device::adc_r<2>));
	io->adc_in<3>().set(FUNC(spg110_device::adc_r<3>));
	io->chip_select().set(FUNC(spg110_device::cs_w));
//  io->pal_read_callback().set(FUNC(spg110_device::get_pal_r));
	io->write_timer_irq_callback().set(FUNC(spg110_device::timerirq_w));
	io->write_uart_adc_irq_callback().set(FUNC(spg110_device::uartirq_w));
	io->write_external_irq_callback().set(FUNC(spg110_device::extirq_w));
	io->write_ffrq_tmr1_irq_callback().set(FUNC(spg110_device::ffreq1_w));
	io->write_ffrq_tmr2_irq_callback().set(FUNC(spg110_device::ffreq2_w));
}

uint16_t spg110_device::space_r(offs_t offset)
{
	address_space &cpuspace = this->space(AS_PROGRAM);
	return cpuspace.read_word(offset);
}

void spg110_device::audioirq_w(int state)
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

	m_spg_audio->add_route(0, *this, 1.0, 0);
	m_spg_audio->add_route(1, *this, 1.0, 1);
}

void spg110_device::internal_map(address_map &map)
{
	map(0x000000, 0x000fff).ram();

	// vregs are at 2000?
	map(0x002010, 0x002015).rw(m_spg_video, FUNC(spg110_video_device::tmap0_regs_r), FUNC(spg110_video_device::tmap0_regs_w));
	map(0x002016, 0x00201b).rw(m_spg_video, FUNC(spg110_video_device::tmap1_regs_r), FUNC(spg110_video_device::tmap1_regs_w));

	map(0x00201c, 0x00201c).w(m_spg_video, FUNC(spg110_video_device::vcomp_val_201c_w)); // P_VComp_Value (vertical compression)

	map(0x002020, 0x002027).w(m_spg_video, FUNC(spg110_video_device::segment_202x_w)); // P_Segment0-7 (tilebases)
	map(0x002028, 0x002028).rw(m_spg_video, FUNC(spg110_video_device::adr_mode_2028_r), FUNC(spg110_video_device::adr_mode_2028_w)); // P_Adr_mode
	map(0x002029, 0x002029).rw(m_spg_video, FUNC(spg110_video_device::ext_bus_2029_r), FUNC(spg110_video_device::ext_bus_2029_w)); // P_Ext_Bus
	// 0x202a // P_Blending

	// 0x2030 // P_Eff_color
	map(0x002031, 0x002031).w(m_spg_video, FUNC(spg110_video_device::win_mask_1_2031_w)); // P_Win_mask1 - sometimes 14a?
	map(0x002032, 0x002032).w(m_spg_video, FUNC(spg110_video_device::win_mask_2_2032_w)); // P_Win_mask2 - always 14a?
	map(0x002033, 0x002033).w(m_spg_video, FUNC(spg110_video_device::win_attribute_w)); // P_Win_attrribute
	map(0x002034, 0x002034).w(m_spg_video, FUNC(spg110_video_device::win_mask_3_2034_w)); // P_Win_mask3
	map(0x002035, 0x002035).w(m_spg_video, FUNC(spg110_video_device::win_mask_4_2035_w)); // P_Win_mask4
	map(0x002036, 0x002036).w(m_spg_video, FUNC(spg110_video_device::irq_tm_v_2036_w)); // P_IRQTMV
	map(0x002037, 0x002037).rw(m_spg_video, FUNC(spg110_video_device::irq_tm_h_2037_r), FUNC(spg110_video_device::irq_tm_h_2037_w)); // P_IRQTMH
	// 0x2038 // P_Effect_color (not the same as 2030)
	map(0x002039, 0x002039).w(m_spg_video, FUNC(spg110_video_device::effect_control_2039_w)); // P_Effect_control
	// 0x203a // P_Mix_offset
	// 0x203b // P_Fan_effect_th
	map(0x00203c, 0x00203c).w(m_spg_video, FUNC(spg110_video_device::huereference_203c_w)); // P_203C_HueRefer (should be set based on PAL/NTSC)
	map(0x00203d, 0x00203d).w(m_spg_video, FUNC(spg110_video_device::lum_adjust_203d_w)); // P_Lum_Adjust
	// 0x203e // P_LPVPosition
	// 0x203f // P_LPHPosition

	map(0x002042, 0x002042).rw(m_spg_video, FUNC(spg110_video_device::sp_control_2042_r),FUNC(spg110_video_device::sp_control_2042_w)); // P_Sp_control

	map(0x002045, 0x002045).w(m_spg_video, FUNC(spg110_video_device::spg110_2045_w)); // not documented?

	map(0x002050, 0x00205f).ram().w(m_spg_video, FUNC(spg110_video_device::transparent_color_205x_w)).share("spg_video:palctrlram"); // P_Trptcolor0 - 15

	map(0x002060, 0x002060).w(m_spg_video, FUNC(spg110_video_device::dma_dst_2060_w)); // P_DMA_Target_adr
	map(0x002061, 0x002061).w(m_spg_video, FUNC(spg110_video_device::dma_dst_seg_2061_w)); // P_DMA_Target_seg
	map(0x002062, 0x002062).rw(m_spg_video, FUNC(spg110_video_device::dma_len_status_2062_r),FUNC(spg110_video_device::dma_len_trigger_2062_w)); // P_DMA_numbr
	map(0x002063, 0x002063).rw(m_spg_video, FUNC(spg110_video_device::spg110_2063_r),FUNC(spg110_video_device::spg110_2063_w)); // P_DMA_control - Video IRQ source / ack (3 different things checked here instead of 2 on spg2xx?)
	map(0x002064, 0x002064).w(m_spg_video, FUNC(spg110_video_device::dma_dst_step_2064_w)); // P_DMA_Target_step
	map(0x002065, 0x002065).rw(m_spg_video, FUNC(spg110_video_device::dma_manual_2065_r), FUNC(spg110_video_device::dma_manual_2065_w)); // P_DMA_data
	map(0x002066, 0x002066).w(m_spg_video, FUNC(spg110_video_device::dma_source_2066_w)); // P_DMA_Source_adr
	map(0x002067, 0x002067).w(m_spg_video, FUNC(spg110_video_device::dma_source_seg_2067_w)); // P_DMA_Source_seg
	map(0x002068, 0x002068).rw(m_spg_video, FUNC(spg110_video_device::dma_src_step_2068_r), FUNC(spg110_video_device::dma_src_step_2068_w)); // P_DMA_Source_step

	map(0x002100, 0x0021ff).ram(); // P_Tx_Hvoffset0 - P_Tx_Hvoffset255 // rowscroll table
	map(0x002200, 0x0022ff).ram(); // P_HComp_Value0 - P_HComp_Value255 // horizontal compression table

	/// sound registers? seems to be 8 long entries, only uses up to 0x7f? (register mapping seems similar to spg2xx, maybe with less channels?)
	map(0x003000, 0x00307f).rw(m_spg_audio, FUNC(spg110_audio_device::audio_r), FUNC(spg110_audio_device::audio_w));
	map(0x003080, 0x0030ff).ram(); // extra ram? doesn't seem to be phase, and there only appear to be 8 channels on SPG110

	map(0x003100, 0x00310f).rw(m_spg_audio, FUNC(spg110_audio_device::audio_ctrl_r), FUNC(spg2xx_audio_device::audio_ctrl_w));

	// 0032xx looks like it could be the same as 003d00 on spg2xx
	map(0x003200, 0x00322f).rw(m_spg_io, FUNC(spg2xx_io_device::io_r), FUNC(spg2xx_io_device::io_w));
}


void spg110_device::device_reset()
{
	unsp_device::device_reset();
}
