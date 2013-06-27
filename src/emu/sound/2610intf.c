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

static void psg_set_clock(void *param, int clock)
{
	ym2610_device *ym2610 = (ym2610_device *) param;
	ay8910_set_clock_ym(ym2610->_psg(), clock);
}

static void psg_write(void *param, int address, int data)
{
	ym2610_device *ym2610 = (ym2610_device *) param;
	ay8910_write_ym(ym2610->_psg(), address, data);
}

static int psg_read(void *param)
{
	ym2610_device *ym2610 = (ym2610_device *) param;
	return ay8910_read_ym(ym2610->_psg());
}

static void psg_reset(void *param)
{
	ym2610_device *ym2610 = (ym2610_device *) param;
	ay8910_reset_ym(ym2610->_psg());
}

static const ssg_callbacks psgintf =
{
	psg_set_clock,
	psg_write,
	psg_read,
	psg_reset
};

void *ym2610_device::_psg()
{
	return m_psg;
}

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

void ym2610_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	ym2610_update_one(m_chip, outputs, samples);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym2610b_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
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
	static const ay8910_interface generic_ay8910 =
	{
		AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
	};

	int rate = clock()/72;
	void *pcmbufa,*pcmbufb;
	int  pcmsizea,pcmsizeb;
	astring name;

	m_irq_handler.resolve();
	m_psg = ay8910_start_ym(this, &generic_ay8910);
	assert_always(m_psg != NULL, "Error creating YM2610/AY8910 chip");

	/* Timer Handler set */
	m_timer[0] = timer_alloc(0);
	m_timer[1] = timer_alloc(1);

	/* stream system initialize */
	m_stream = machine().sound().stream_alloc(*this,0,2,rate);
	/* setup adpcm buffers */
	pcmbufa  = *region();
	pcmsizea = region()->bytes();
	name.printf("%s.deltat", tag());
	pcmbufb  = (void *)(machine().root_device().memregion(name)->base());
	pcmsizeb = machine().root_device().memregion(name)->bytes();
	if (pcmbufb == NULL || pcmsizeb == 0)
	{
		pcmbufb = pcmbufa;
		pcmsizeb = pcmsizea;
	}

	/**** initialize YM2610 ****/
	m_chip = ym2610_init(this,this,clock(),rate,
					pcmbufa,pcmsizea,pcmbufb,pcmsizeb,
					timer_handler,IRQHandler,&psgintf);
	assert_always(m_chip != NULL, "Error creating YM2610 chip");
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2610_device::device_stop()
{
	ym2610_shutdown(m_chip);
	ay8910_stop_ym(m_psg);
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
	: device_t(mconfig, YM2610, "YM2610", tag, owner, clock, "ym2610", __FILE__),
		device_sound_interface(mconfig, *this),
		m_irq_handler(*this)
{
}

ym2610_device::ym2610_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_sound_interface(mconfig, *this),
		m_irq_handler(*this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ym2610_device::device_config_complete()
{
}

const device_type YM2610B = &device_creator<ym2610b_device>;

ym2610b_device::ym2610b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ym2610_device(mconfig, YM2610B, "YM2610B", tag, owner, clock, "ym2610b", __FILE__)
{
}
