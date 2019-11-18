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

#include "emu.h"
#include "2608intf.h"
#include "fm.h"

const ssg_callbacks ym2608_device::psgintf =
{
	&ym2608_device::psg_set_clock,
	&ym2608_device::psg_write,
	&ym2608_device::psg_read,
	&ym2608_device::psg_reset
};

/* IRQ Handler */
void ym2608_device::irq_handler(int irq)
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

void ym2608_device::timer_handler(int c,int count,int clock)
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

	m_irq_handler.resolve();

	/* Timer Handler set */
	m_timer[0] = timer_alloc(0);
	m_timer[1] = timer_alloc(1);

	/* stream system initialize */
	m_stream = machine().sound().stream_alloc(*this,0,2,rate, stream_update_delegate(&ym2608_device::stream_generate,this));

	/* initialize YM2608 */
	m_chip = ym2608_init(this,clock(),rate,
			&ym2608_device::static_internal_read_byte,
			&ym2608_device::static_external_read_byte, &ym2608_device::static_external_write_byte,
			&ym2608_device::static_timer_handler,&ym2608_device::static_irq_handler,&psgintf);
	if (!m_chip)
		throw emu_fatalerror("ym2608_device(%s): Error creating YM2608 chip", tag());
}

//-------------------------------------------------
//  device_clock_changed
//-------------------------------------------------

void ym2608_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 72);
	ym2608_clock_changed(m_chip, clock(), clock() / 72);
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

//-------------------------------------------------
//  rom_bank_updated
//-------------------------------------------------

void ym2608_device::rom_bank_updated()
{
	m_stream->update();
}


u8 ym2608_device::read(offs_t offset)
{
	return ym2608_read(m_chip, offset & 3);
}

void ym2608_device::write(offs_t offset, u8 data)
{
	ym2608_write(m_chip, offset & 3, data);
}

DEFINE_DEVICE_TYPE(YM2608, ym2608_device, "ym2608", "YM2608 OPNA")

ym2608_device::ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ay8910_device(mconfig, YM2608, tag, owner, clock, PSG_TYPE_YM, 1, 2)
	, device_rom_interface(mconfig, *this, 21)
	, m_stream(nullptr)
	, m_timer{ nullptr, nullptr }
	, m_chip(nullptr)
	, m_irq_handler(*this)
	, m_internal(*this, "internal")
{
}

ROM_START( ym2608 )
	ROM_REGION( 0x2000, "internal", 0 )
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


const tiny_rom_entry *ym2608_device::device_rom_region() const
{
	return ROM_NAME( ym2608 );
}
