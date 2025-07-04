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

DEFINE_DEVICE_TYPE(SPG24X,     spg24x_device,     "spg24x", "SPG240-series System-on-a-Chip") // 256 sprites
DEFINE_DEVICE_TYPE(SPG2XX_128, spg2xx_128_device, "spg2xx_128", "SPG2xx-series System-on-a-Chip (128 sprites)") // exact SPG part number unknown
DEFINE_DEVICE_TYPE(SPG28X,     spg28x_device,     "spg28x", "SPG280-series System-on-a-Chip") // 64 sprites


spg2xx_device::spg2xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint16_t sprite_limit, address_map_constructor internal) :
	unsp_device(mconfig, type, tag, owner, clock, internal),
	device_mixer_interface(mconfig, *this),
	m_spg_audio(*this, "spgaudio"),
	m_spg_io(*this, "spgio"),
	m_spg_sysdma(*this, "spgsysdma"),
	m_spg_video(*this, "spgvideo"),
	m_sprite_limit(sprite_limit),
	m_porta_out(*this),
	m_portb_out(*this),
	m_portc_out(*this),
	m_porta_in(*this, 0),
	m_portb_in(*this, 0),
	m_portc_in(*this, 0),
	m_adc_in(*this, 0x0fff),
	m_guny_in(*this, 0),
	m_gunx_in(*this, 0),
	m_i2c_w(*this),
	m_i2c_r(*this, 0),
	m_uart_tx(*this),
	m_spi_tx(*this),
	m_chip_sel(*this),
	m_screen(*this, finder_base::DUMMY_TAG)
{
}

spg24x_device::spg24x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spg2xx_device(mconfig, SPG24X, tag, owner, clock, 256, address_map_constructor(FUNC(spg24x_device::internal_map), this))
{
}

spg24x_device::spg24x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint16_t sprite_limit, address_map_constructor internal) :
	spg2xx_device(mconfig, type, tag, owner, clock, sprite_limit, internal)
{
}


spg2xx_128_device::spg2xx_128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spg24x_device(mconfig, SPG2XX_128, tag, owner, clock, 128, address_map_constructor(FUNC(spg2xx_128_device::internal_map), this))
{
}

spg28x_device::spg28x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	spg24x_device(mconfig, SPG28X, tag, owner, clock, 64, address_map_constructor(FUNC(spg28x_device::internal_map), this))
{
}

void spg2xx_device::internal_map(address_map &map)
{
	map(0x000000, 0x0027ff).ram();
	map(0x002800, 0x0028ff).rw(m_spg_video, FUNC(spg2xx_video_device::video_r), FUNC(spg2xx_video_device::video_w));
	map(0x002900, 0x0029ff).ram().share("spgvideo:scrollram");
	map(0x002a00, 0x002aff).ram().share("spgvideo:hcompram"); // not all models?
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
	unsp_device::device_start();

	save_item(NAME(m_sprite_limit));
	save_item(NAME(m_pal_flag));
	save_item(NAME(m_fiq_vector));
}

void spg2xx_device::device_reset()
{
	unsp_device::device_reset();
	m_fiq_vector = 0xff;
}

void spg2xx_device::fiq_vector_w(uint8_t data)
{
	m_fiq_vector = data;
}

void spg2xx_device::videoirq_w(int state)
{
	if (m_fiq_vector == 0)
	{
		set_state_unsynced(UNSP_FIQ_LINE, state);
	}
	else
	{
		set_state_unsynced(UNSP_IRQ0_LINE, state);
	}
}

void spg2xx_device::timerirq_w(int state)
{
	set_state_unsynced(UNSP_IRQ2_LINE, state);
}

void spg2xx_device::uartirq_w(int state)
{
	set_state_unsynced(UNSP_IRQ3_LINE, state);
}

void spg2xx_device::audioirq_w(int state)
{
	set_state_unsynced(UNSP_IRQ4_LINE, state);
}

void spg2xx_device::audiochirq_w(int state)
{
	set_state_unsynced(UNSP_FIQ_LINE, state);
}

void spg2xx_device::extirq_w(int state)
{
	set_state_unsynced(UNSP_IRQ5_LINE, state);
}

void spg2xx_device::ffreq1_w(int state)
{
	set_state_unsynced(UNSP_IRQ6_LINE, state);
}

void spg2xx_device::ffreq2_w(int state)
{
	set_state_unsynced(UNSP_IRQ7_LINE, state);
}



uint16_t spg2xx_device::space_r(offs_t offset)
{
	address_space &cpuspace = this->space(AS_PROGRAM);
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
	io->adc_in<2>().set(FUNC(spg2xx_device::adc_r<2>));
	io->adc_in<3>().set(FUNC(spg2xx_device::adc_r<3>));
	io->i2c_w().set(FUNC(spg2xx_device::eepromx_w));
	io->i2c_r().set(FUNC(spg2xx_device::eepromx_r));
	io->uart_tx().set(FUNC(spg2xx_device::uart_tx_w));
	io->spi_tx().set(FUNC(spg2xx_device::spi_tx_w));
	io->chip_select().set(FUNC(spg2xx_device::cs_w));
	io->pal_read_callback().set(FUNC(spg2xx_device::get_pal_ntsc));
	io->write_timer_irq_callback().set(FUNC(spg2xx_device::timerirq_w));
	io->write_uart_adc_irq_callback().set(FUNC(spg2xx_device::uartirq_w));
	io->write_external_irq_callback().set(FUNC(spg2xx_device::extirq_w));
	io->write_ffrq_tmr1_irq_callback().set(FUNC(spg2xx_device::ffreq1_w));
	io->write_ffrq_tmr2_irq_callback().set(FUNC(spg2xx_device::ffreq2_w));
	io->write_fiq_vector_callback().set(FUNC(spg2xx_device::fiq_vector_w));

	m_spg_video->guny_in().set(FUNC(spg2xx_device::guny_in_r));
	m_spg_video->gunx_in().set(FUNC(spg2xx_device::gunx_in_r));
}

void spg24x_device::device_add_mconfig(machine_config &config)
{
	SPG2XX_AUDIO(config, m_spg_audio, DERIVED_CLOCK(1, 1));
	m_spg_audio->write_irq_callback().set(FUNC(spg24x_device::audioirq_w));
	m_spg_audio->channel_irq_callback().set(FUNC(spg24x_device::audiochirq_w));
	m_spg_audio->space_read_callback().set(FUNC(spg24x_device::space_r));

	m_spg_audio->add_route(0, *this, 1.0, 0);
	m_spg_audio->add_route(1, *this, 1.0, 1);

	SPG24X_IO(config, m_spg_io, DERIVED_CLOCK(1, 1), DEVICE_SELF, m_screen);

	SPG2XX_SYSDMA(config, m_spg_sysdma, DERIVED_CLOCK(1, 1), DEVICE_SELF);

	SPG24X_VIDEO(config, m_spg_video, DERIVED_CLOCK(1, 1), DEVICE_SELF, m_screen);
	m_spg_video->sprlimit_read_callback().set(FUNC(spg24x_device::get_sprlimit));
	m_spg_video->write_video_irq_callback().set(FUNC(spg24x_device::videoirq_w));

	configure_spg_io(m_spg_io);
}
