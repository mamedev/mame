// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

  2608intf.c

  The YM2608 emulator supports up to 2 chips.
  Each chip has the following connections:
  - Status Read / Control Write A
  - Port Read / Data Write A
  - Control Write B
  - Data Write B

***************************************************************************/

#include "2608intf.h"
#include "fm.h"

static void psg_set_clock(void *param, int clock)
{
	ym2608_device *ym2608 = (ym2608_device *) param;
	ym2608->ay_set_clock(clock);
}

static void psg_write(void *param, int address, int data)
{
	ym2608_device *ym2608 = (ym2608_device *) param;
	ym2608->ay8910_write_ym(address, data);
}

static int psg_read(void *param)
{
	ym2608_device *ym2608 = (ym2608_device *) param;
	return ym2608->ay8910_read_ym();
}

static void psg_reset(void *param)
{
	ym2608_device *ym2608 = (ym2608_device *) param;
	ym2608->ay8910_reset_ym();
}

static const ssg_callbacks psgintf =
{
	psg_set_clock,
	psg_write,
	psg_read,
	psg_reset
};

/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	ym2608_device *ym2608 = (ym2608_device *) param;
	ym2608->_IRQHandler(irq);
}

void ym2608_device::_IRQHandler(int irq)
{
	if (!m_irq_handler.isnull())
		m_irq_handler(irq);
}

/* Timer overflow callback from timer.c */
void ym2608_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case 0:
		ym2608_timer_over(m_chip,0);
		break;

	case 1:
		ym2608_timer_over(m_chip,1);
		break;
	}
}

static void timer_handler(void *param,int c,int count,int clock)
{
	ym2608_device *ym2608 = (ym2608_device *) param;
	ym2608->_timer_handler(c, count, clock);
}

void ym2608_device::_timer_handler(int c,int count,int clock)
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
void ym2608_update_request(void *param)
{
	ym2608_device *ym2608 = (ym2608_device *) param;
	ym2608->_ym2608_update_request();
}

void ym2608_device::_ym2608_update_request()
{
	m_stream->update();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ym2608_device::stream_generate(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	ym2608_update_one(m_chip, outputs, samples);
}

void ym2608_device::device_post_load()
{
	ym2608_postload(m_chip);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ym2608_device::device_start()
{
	ay8910_device::device_start();

	int rate = clock()/72;
	void *pcmbufa;
	int  pcmsizea;

	m_irq_handler.resolve();

	/* Timer Handler set */
	m_timer[0] = timer_alloc(0);
	m_timer[1] = timer_alloc(1);

	/* stream system initialize */
	m_stream = machine().sound().stream_alloc(*this,0,2,rate, stream_update_delegate(FUNC(ym2608_device::stream_generate),this));
	/* setup adpcm buffers */
	pcmbufa  = region()->base();
	pcmsizea = region()->bytes();

	/* initialize YM2608 */
	m_chip = ym2608_init(this,this,clock(),rate,
					pcmbufa,pcmsizea,
					timer_handler,IRQHandler,&psgintf);
	assert_always(m_chip != nullptr, "Error creating YM2608 chip");
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void ym2608_device::device_stop()
{
	ym2608_shutdown(m_chip);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ym2608_device::device_reset()
{
	ym2608_reset_chip(m_chip);
}


READ8_MEMBER( ym2608_device::read )
{
	return ym2608_read(m_chip, offset & 3);
}

WRITE8_MEMBER( ym2608_device::write )
{
	ym2608_write(m_chip, offset & 3, data);
}

const device_type YM2608 = &device_creator<ym2608_device>;

ym2608_device::ym2608_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ay8910_device(mconfig, YM2608, "YM2608", tag, owner, clock, PSG_TYPE_YM, 1, 2, "ym2608", __FILE__),
		m_irq_handler(*this)
{
}

ROM_START( ym2608 )
	ROM_REGION( 0x2000, "ym2608", 0 )
	/*
	This data is derived from the chip's output - internal ROM can't be read.
	It was verified, using real YM2608, that this ADPCM stream produces 100% correct output signal.
	*/
	// see YM2608_ADPCM_ROM_addr table in fm.c for current sample offsets
	// original offset comments from Jarek:
	// offset 0:
		/* Source: 01BD.ROM */
		/* Length: 448 / 0x000001C0 */
	// offset 0x1C0:
		/* Source: 02SD.ROM */
		/* Length: 640 / 0x00000280 */
	// offset 0x440:
		/* Source: 04TOP.ROM */
		/* Length: 5952 / 0x00001740 */
	// offset 0x1B80:
		/* Source: 08HH.ROM */
		/* Length: 384 / 0x00000180 */
	// offset 0x1D00
		/* Source: 10TOM.ROM */
		/* Length: 640 / 0x00000280 */
	// offset 0x1F80
		/* Source: 20RIM.ROM */
		/* Length: 128 / 0x00000080 */
	/* while this rom was dumped by output analysis, not decap, it was tested
	  by playing it back into the chip as an external adpcm sample and produced
	  an identical dac result. a decap would be nice to verify things 100%,
	  but there is currently no reason to think this rom dump is incorrect. */
	ROM_LOAD16_WORD( "ym2608_adpcm_rom.bin", 0x0000, 0x2000, CRC(23c9e0d8) SHA1(50b6c3e288eaa12ad275d4f323267bb72b0445df) )
ROM_END


const rom_entry *ym2608_device::device_rom_region() const
{
	return ROM_NAME( ym2608 );
}
