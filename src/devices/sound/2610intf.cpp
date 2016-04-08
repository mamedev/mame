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

#include "2610intf.h"
#include "fm.h"

const char* YM2610_TAG = "ymsnd";
const char* YM2610_DELTAT_TAG = "ymsnd.deltat";

static void psg_set_clock(void *param, int clock)
{
	ym2610_device *ym2610 = (ym2610_device *) param;
	ym2610->ay_set_clock(clock);
}

static void psg_write(void *param, int address, int data)
{
	ym2610_device *ym2610 = (ym2610_device *) param;
	ym2610->ay8910_write_ym(address, data);
}

static int psg_read(void *param)
{
	ym2610_device *ym2610 = (ym2610_device *) param;
	return ym2610->ay8910_read_ym();
}

static void psg_reset(void *param)
{
	ym2610_device *ym2610 = (ym2610_device *) param;
	ym2610->ay8910_reset_ym();
}

static const ssg_callbacks psgintf =
{
	psg_set_clock,
	psg_write,
	psg_read,
	psg_reset
};

/*------------------------- TM2610 -------------------------------*/
/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	ym2610_device *ym2610 = (ym2610_device *) param;
	ym2610->_IRQHandler(irq);
}

void ym2610_device::_IRQHandler(int irq)
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

static void timer_handler(void *param,int c,int count,int clock)
{
	ym2610_device *ym2610 = (ym2610_device *) param;
	ym2610->_timer_handler(c, count, clock);
}

void ym2610_device::_timer_handler(int c,int count,int clock)
{
	if( count == 0 )
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

/* update request from fm.c */
void ym2610_update_request(void *param)
{
	ym2610_device *ym2610 = (ym2610_device *) param;
	ym2610->_ym2610_update_request();
}

void ym2610_device::_ym2610_update_request()
{
	m_stream->update();
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
	m_stream = machine().sound().stream_alloc(*this,0,2,rate, stream_update_delegate(FUNC(ym2610_device::stream_generate),this));

	/* setup adpcm buffers */
	void *pcmbufa = m_region->base();
	int pcmsizea = m_region->bytes();

	std::string name = tag() + std::string(".deltat");
	memory_region *deltat_region = machine().root_device().memregion(name.c_str());
	void *pcmbufb = pcmbufa;
	int pcmsizeb = pcmsizea;
	if (deltat_region != nullptr && deltat_region->base() != nullptr && deltat_region->bytes() != 0)
	{
		pcmbufb = deltat_region->base();
		pcmsizeb = deltat_region->bytes();
	}

	/**** initialize YM2610 ****/
	m_chip = ym2610_init(this,this,clock(),rate,
					pcmbufa,pcmsizea,pcmbufb,pcmsizeb,
					timer_handler,IRQHandler,&psgintf);
	assert_always(m_chip != nullptr, "Error creating YM2610 chip");
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


READ8_MEMBER( ym2610_device::read )
{
	return ym2610_read(m_chip, offset & 3);
}

WRITE8_MEMBER( ym2610_device::write )
{
	ym2610_write(m_chip, offset & 3, data);
}


const device_type YM2610 = &device_creator<ym2610_device>;

ym2610_device::ym2610_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ay8910_device(mconfig, YM2610, "YM2610", tag, owner, clock, PSG_TYPE_YM, 1, 0, "ym2610", __FILE__)
	, m_irq_handler(*this)
	, m_region(*this, DEVICE_SELF)
{
}

ym2610_device::ym2610_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: ay8910_device(mconfig, type, name, tag, owner, clock, PSG_TYPE_YM, 1, 0, shortname, source)
	, m_irq_handler(*this)
	, m_region(*this, DEVICE_SELF)
{
}

const device_type YM2610B = &device_creator<ym2610b_device>;

ym2610b_device::ym2610b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ym2610_device(mconfig, YM2610B, "YM2610B", tag, owner, clock, "ym2610b", __FILE__)
{
}
