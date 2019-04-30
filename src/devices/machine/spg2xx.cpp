// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation

    TODO:
        - Serial UART
        - I2C
        - SPI

**********************************************************************/

#include "emu.h"
#include "spg2xx.h"

DEFINE_DEVICE_TYPE(SPG24X, spg24x_device, "spg24x", "SPG240-series System-on-a-Chip")
DEFINE_DEVICE_TYPE(SPG28X, spg28x_device, "spg28x", "SPG280-series System-on-a-Chip")


spg2xx_device::spg2xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mixer_interface(mconfig, *this, 2)
	, m_spg_audio(*this, "spgaudio")
	, m_spg_io(*this, "spgio")
	, m_spg_sysdma(*this, "spgsysdma")
	, m_spg_video(*this, "spgvideo")
	, m_rowscrolloffset(15)
	, m_porta_out(*this)
	, m_portb_out(*this)
	, m_portc_out(*this)
	, m_porta_in(*this)
	, m_portb_in(*this)
	, m_portc_in(*this)
	, m_adc_in{{*this}, {*this}}
	, m_eeprom_w(*this)
	, m_eeprom_r(*this)
	, m_uart_tx(*this)
	, m_chip_sel(*this)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
{
}

spg24x_device::spg24x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spg2xx_device(mconfig, SPG24X, tag, owner, clock, 256)
{
}

spg28x_device::spg28x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spg2xx_device(mconfig, SPG28X, tag, owner, clock, 64)
{
}

void spg2xx_device::map(address_map &map)
{
	map(0x000000, 0x0027ff).ram();
	map(0x002800, 0x0028ff).rw(m_spg_video, FUNC(spg2xx_video_device::video_r), FUNC(spg2xx_video_device::video_w));
	map(0x002900, 0x002aff).ram().share("spgvideo:scrollram");
	map(0x002b00, 0x002bff).ram().share("spgvideo:paletteram");
	map(0x002c00, 0x002fff).ram().share("spgvideo:spriteram");
	map(0x003000, 0x0031ff).rw(m_spg_audio, FUNC(spg2xx_audio_device::audio_r), FUNC(spg2xx_audio_device::audio_w));
	map(0x003200, 0x0033ff).rw(m_spg_audio, FUNC(spg2xx_audio_device::audio_phase_r), FUNC(spg2xx_audio_device::audio_phase_w));
	map(0x003400, 0x0037ff).rw(m_spg_audio, FUNC(spg2xx_audio_device::audio_ctrl_r), FUNC(spg2xx_audio_device::audio_ctrl_w));
	map(0x003d00, 0x003d2f).rw(m_spg_io, FUNC(spg2xx_io_device::io_r), FUNC(spg2xx_io_device::io_w));
	map(0x003d30, 0x003dff).rw(m_spg_io, FUNC(spg2xx_io_device::io_extended_r), FUNC(spg2xx_io_device::io_extended_w));
	map(0x003e00, 0x003e03).rw(m_spg_sysdma, FUNC(spg2xx_sysdma_device::dma_r), FUNC(spg2xx_sysdma_device::dma_w));
}

void spg2xx_device::device_start()
{
	m_porta_out.resolve_safe();
	m_portb_out.resolve_safe();
	m_portc_out.resolve_safe();
	m_porta_in.resolve_safe(0);
	m_portb_in.resolve_safe(0);
	m_portc_in.resolve_safe(0);
	m_adc_in[0].resolve_safe(0x0fff);
	m_adc_in[1].resolve_safe(0x0fff);
	m_eeprom_w.resolve_safe();
	m_eeprom_r.resolve_safe(0);
	m_uart_tx.resolve_safe();
	m_chip_sel.resolve_safe();

	save_item(NAME(m_sprite_limit));
	save_item(NAME(m_pal_flag));
}

void spg2xx_device::device_reset()
{
}

WRITE_LINE_MEMBER(spg2xx_device::videoirq_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ0_LINE, state);
}

WRITE_LINE_MEMBER(spg2xx_device::timerirq_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ2_LINE, state);
}

WRITE_LINE_MEMBER(spg2xx_device::uartirq_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ3_LINE, state);
}

WRITE_LINE_MEMBER(spg2xx_device::audioirq_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ4_LINE, state);
}

WRITE_LINE_MEMBER(spg2xx_device::extirq_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ5_LINE, state);
}

WRITE_LINE_MEMBER(spg2xx_device::ffreq1_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ6_LINE, state);
}

WRITE_LINE_MEMBER(spg2xx_device::ffreq2_w)
{
	m_cpu->set_state_unsynced(UNSP_IRQ7_LINE, state);
}



READ16_MEMBER(spg2xx_device::space_r)
{
	address_space &cpuspace = m_cpu->space(AS_PROGRAM);
	return cpuspace.read_word(offset);
}

void spg2xx_device::configure_spg_io(spg2xx_io_device* io)
{
	io->porta_in().set(FUNC(spg2xx_device::porta_r));
	io->portb_in().set(FUNC(spg2xx_device::portb_r));
	io->portc_in().set(FUNC(spg2xx_device::portc_r));
	io->porta_out().set(FUNC(spg2xx_device::porta_w));
	io->portb_out().set(FUNC(spg2xx_device::portb_w));
	io->portc_out().set(FUNC(spg2xx_device::portc_w));
	io->adc_in<0>().set(FUNC(spg2xx_device::adc_r<0>));
	io->adc_in<1>().set(FUNC(spg2xx_device::adc_r<1>));
	io->eeprom_w().set(FUNC(spg2xx_device::eepromx_w));
	io->eeprom_r().set(FUNC(spg2xx_device::eepromx_r));
	io->uart_tx().set(FUNC(spg2xx_device::tx_w));
	io->chip_select().set(FUNC(spg2xx_device::cs_w));
	io->pal_read_callback().set(FUNC(spg2xx_device::get_pal_ntsc));
	io->write_timer_irq_callback().set(FUNC(spg2xx_device::timerirq_w));
	io->write_uart_adc_irq_callback().set(FUNC(spg2xx_device::uartirq_w));
	io->write_external_irq_callback().set(FUNC(spg2xx_device::extirq_w));
	io->write_ffrq_tmr1_irq_callback().set(FUNC(spg2xx_device::ffreq1_w));
	io->write_ffrq_tmr2_irq_callback().set(FUNC(spg2xx_device::ffreq2_w));
}

void spg24x_device::device_add_mconfig(machine_config &config)
{
	SPG2XX_AUDIO(config, m_spg_audio, DERIVED_CLOCK(1, 1));
	m_spg_audio->write_irq_callback().set(FUNC(spg24x_device::audioirq_w));
	m_spg_audio->space_read_callback().set(FUNC(spg24x_device::space_r));

	m_spg_audio->add_route(0, *this, 1.0, AUTO_ALLOC_INPUT, 0);
	m_spg_audio->add_route(1, *this, 1.0, AUTO_ALLOC_INPUT, 1);

	SPG24X_IO(config, m_spg_io, DERIVED_CLOCK(1, 1), m_cpu, m_screen);
	configure_spg_io(m_spg_io);

	SPG2XX_SYSDMA(config, m_spg_sysdma, DERIVED_CLOCK(1, 1), m_cpu);

	SPG24X_VIDEO(config, m_spg_video, DERIVED_CLOCK(1, 1), m_cpu, m_screen);
	m_spg_video->sprlimit_read_callback().set(FUNC(spg24x_device::get_sprlimit));
	m_spg_video->rowscrolloffset_read_callback().set(FUNC(spg24x_device::get_rowscrolloffset));
	m_spg_video->write_video_irq_callback().set(FUNC(spg24x_device::videoirq_w));
}

void spg28x_device::device_add_mconfig(machine_config &config)
{
	SPG2XX_AUDIO(config, m_spg_audio, DERIVED_CLOCK(1, 1));
	m_spg_audio->write_irq_callback().set(FUNC(spg28x_device::audioirq_w));
	m_spg_audio->space_read_callback().set(FUNC(spg28x_device::space_r));

	m_spg_audio->add_route(0, *this, 1.0, AUTO_ALLOC_INPUT, 0);
	m_spg_audio->add_route(1, *this, 1.0, AUTO_ALLOC_INPUT, 1);

	SPG28X_IO(config, m_spg_io, DERIVED_CLOCK(1, 1), m_cpu, m_screen);
	configure_spg_io(m_spg_io);

	SPG2XX_SYSDMA(config, m_spg_sysdma, DERIVED_CLOCK(1, 1), m_cpu);

	SPG24X_VIDEO(config, m_spg_video, DERIVED_CLOCK(1, 1), m_cpu, m_screen);
	m_spg_video->sprlimit_read_callback().set(FUNC(spg28x_device::get_sprlimit));
	m_spg_video->rowscrolloffset_read_callback().set(FUNC(spg28x_device::get_rowscrolloffset));
	m_spg_video->write_video_irq_callback().set(FUNC(spg28x_device::videoirq_w));
}
