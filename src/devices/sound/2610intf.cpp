// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

  2610intf.c

  The YM2610 emulator supports up to 2 chips.
  Each chip has the following connections:
  - Status Read / Control Write A
  - Port Read / Data Write A
  - Control Write B
  - Data Write B

***************************************************************************/

#include "emu.h"
#include "2610intf.h"
#include "fm.h"

const ssg_callbacks ym2610_device::psgintf =
{
	&ym2610_device::psg_set_clock,
	&ym2610_device::psg_write,
	&ym2610_device::psg_read,
	&ym2610_device::psg_reset
};

void ym2610_device::irq_handler(int irq)
{
	if (!m_irq_handler.isnull())
		m_irq_handler(irq);
}

/* Timer overflow callback from timer.c */
void ym2610_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case 0:
		ym2610_timer_over(m_chip,0);
		break;

	case 1:
		ym2610_timer_over(m_chip,1);
		break;
	}
}

void ym2610_device::timer_handler(int c,int count,int clock)
{
	if( count == 0 || clock == 0 )
	{   /* Reset FM Timer */
		m_timer[c]->enable(false);
	}
	else
	{   /* Start FM Timer */
		attotime period = attotime::from_hz(clock) * count;

		if (!m_timer[c]->enable(true))
			m_timer[c]->adjust(period);
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------
void ym2610_device::stream_generate(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	ym2610_update_one(m_chip, outputs, samples);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym2610b_device::stream_generate(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	ym2610b_update_one(m_chip, outputs, samples);
}


void ym2610_device::device_post_load()
{
	ym2610_postload(m_chip);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2610_device::device_start()
{
	ay8910_device::device_start();

	int rate = clock()/72;

	m_irq_handler.resolve();

	/* Timer Handler set */
	m_timer[0] = timer_alloc(0);
	m_timer[1] = timer_alloc(1);

	/* stream system initialize */
	m_stream = machine().sound().stream_alloc(*this,0,2,rate, stream_update_delegate(&ym2610_device::stream_generate,this));

	if (!has_configured_map(0) && !has_configured_map(1))
	{
		if (m_adpcm_a_region)
			space(0).install_rom(0, m_adpcm_a_region->bytes() - 1, m_adpcm_a_region->base());

		if (m_adpcm_b_region)
			space(1).install_rom(0, m_adpcm_b_region->bytes() - 1, m_adpcm_b_region->base());
		else if (m_adpcm_a_region)
			space(1).install_rom(0, m_adpcm_a_region->bytes() - 1, m_adpcm_a_region->base());
	}

	/**** initialize YM2610 ****/
	m_chip = ym2610_init(this, clock(), rate,
		&ym2610_device::static_adpcm_a_read_byte, &ym2610_device::static_adpcm_b_read_byte,
		&ym2610_device::static_timer_handler, &ym2610_device::static_irq_handler, &psgintf);
	assert_always(m_chip != nullptr, "Error creating YM2610 chip");
}

//-------------------------------------------------
//  device_clock_changed
//-------------------------------------------------

void ym2610_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 72);
	ym2610_clock_changed(m_chip, clock(), clock() / 72);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2610_device::device_stop()
{
	ym2610_shutdown(m_chip);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2610_device::device_reset()
{
	ym2610_reset_chip(m_chip);
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector ym2610_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_adpcm_a_config),
		std::make_pair(1, &m_adpcm_b_config)
	};
}


u8 ym2610_device::read(offs_t offset)
{
	return ym2610_read(m_chip, offset & 3);
}

void ym2610_device::write(offs_t offset, u8 data)
{
	ym2610_write(m_chip, offset & 3, data);
}


DEFINE_DEVICE_TYPE(YM2610, ym2610_device, "ym2610", "YM2610 OPNB")

ym2610_device::ym2610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ym2610_device(mconfig, YM2610, tag, owner, clock)
{
}

ym2610_device::ym2610_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ay8910_device(mconfig, type, tag, owner, clock, PSG_TYPE_YM, 1, 0)
	, device_memory_interface(mconfig, *this)
	, m_adpcm_a_config("adpcm-a", ENDIANNESS_LITTLE, 8, 24, 0)
	, m_adpcm_b_config("adpcm-b", ENDIANNESS_LITTLE, 8, 24, 0)
	, m_stream(nullptr)
	, m_timer{ nullptr, nullptr }
	, m_irq_handler(*this)
	, m_adpcm_b_region_name("^" + std::string(basetag()) + ".deltat")
	, m_adpcm_a_region(*this, DEVICE_SELF)
	, m_adpcm_b_region(*this, m_adpcm_b_region_name.c_str())
{
}

DEFINE_DEVICE_TYPE(YM2610B, ym2610b_device, "ym2610b", "YM2610B OPNB")

ym2610b_device::ym2610b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ym2610_device(mconfig, YM2610B, tag, owner, clock)
{
}
