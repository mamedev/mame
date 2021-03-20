// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ym2413.h"


DEFINE_DEVICE_TYPE(YM2413, ym2413_device, "ym2413", "YM2413 OPLL")
DEFINE_DEVICE_TYPE(YM2423, ym2423_device, "ym2423", "YM2423 OPLL-X")
DEFINE_DEVICE_TYPE(YMF281, ymf281_device, "ymf281", "YMF281 OPLLP")
DEFINE_DEVICE_TYPE(DS1001, ds1001_device, "ds1001", "Yamaha DS1001 / Konami 053982")


//*********************************************************
//  YM2413 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2413_device - constructor
//-------------------------------------------------

ym2413_device::ym2413_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type, u8 const *instruments) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_address(0),
	m_stream(nullptr),
	m_internal(*this, "internal"),
	m_fm(*this)
{
}


//-------------------------------------------------
//  address_w - write to the address port (A0=0)
//-------------------------------------------------

void ym2413_device::address_w(u8 value)
{
	m_address = value;
}


//-------------------------------------------------
//  data_w - write to the data port (A0=1)
//-------------------------------------------------

void ym2413_device::data_w(u8 value)
{
	// force an update
	m_stream->update();

	// write to FM
	m_fm.write(m_address, value);
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2413_device::write(offs_t offset, u8 value)
{
	// A0 selects between address/data
	if ((offset & 1) == 0)
		address_w(value);
	else
		data_w(value);
}


//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

void ym2413_device::device_start()
{
	// create our stream
	m_stream = stream_alloc(0, fm_engine::OUTPUTS, m_fm.sample_rate(clock()));

	// save our data
	save_item(YMFM_NAME(m_address));

	// save the engines
	m_fm.save(*this);

	// set up the instrument data
	m_fm.set_instrument_data(m_internal);
}


//-------------------------------------------------
//  device_reset - start of emulation
//-------------------------------------------------

void ym2413_device::device_reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void ym2413_device::device_clock_changed()
{
	m_stream->set_sample_rate(m_fm.sample_rate(clock()));
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region
//-------------------------------------------------

ROM_START( ym2413 )
	ROM_REGION( 0x90, "internal", 0 )
	//
	// This is not the exact format
	//
	ROM_LOAD16_WORD( "ym2413_instruments.bin", 0x0000, 0x0090, CRC(6f582d01) SHA1(bb5537717e0b34849456b5ca7d405403dc3f8fda) )
ROM_END

const tiny_rom_entry *ym2413_device::device_rom_region() const
{
	return ROM_NAME( ym2413 );
}


//-------------------------------------------------
//  sound_stream_update - update the sound stream
//-------------------------------------------------

void ym2413_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// iterate over all target samples
	for (int sampindex = 0; sampindex < outputs[0].samples(); sampindex++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM; YM3812 is 9-bit, unsure of clipping but guessing
		// it is similar to YM2612
		s32 sums[fm_engine::OUTPUTS] = { 0 };
		m_fm.output(sums, 5, 256, fm_engine::ALL_CHANNELS);

		// the YM3812 is time multiplexed; just simulate this by summing all the
		// channels and dividing down
		for (int outnum = 0; outnum < fm_engine::OUTPUTS; outnum++)
			outputs[outnum].put_int(sampindex, sums[outnum], 256*6*2);
	}
}


//*********************************************************
//  YM2423 DEVICE (OPLL-X)
//*********************************************************

//-------------------------------------------------
//  ym2423_device - constructor
//-------------------------------------------------

ym2423_device::ym2423_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ym2413_device(mconfig, tag, owner, clock, YM2423)
{
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region
//-------------------------------------------------

ROM_START( ym2423 )
	ROM_REGION( 0x90, "internal", 0 )
	//
	// This is not the exact format
	//
	ROM_LOAD16_WORD( "ym2423_instruments.bin", 0x0000, 0x0090, CRC(cc51dd1b) SHA1(59c51918f02891d6a0e917f7ebc27e42f7eadd15) )
ROM_END

const tiny_rom_entry *ym2423_device::device_rom_region() const
{
	return ROM_NAME( ym2423 );
}


//*********************************************************
//  YMF281 DEVICE (OPLLP)
//*********************************************************

//-------------------------------------------------
//  ymf281_device - constructor
//-------------------------------------------------

ymf281_device::ymf281_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ym2413_device(mconfig, tag, owner, clock, YMF281)
{
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region
//-------------------------------------------------

ROM_START( ymf281 )
	ROM_REGION( 0x90, "internal", 0 )
	//
	// This is not the exact format
	//
	ROM_LOAD16_WORD( "ymf281_instruments.bin", 0x0000, 0x0090, CRC(1c68abba) SHA1(5242d7b9c677c48e156ba5753db1a73db627a1a9) )
ROM_END

const tiny_rom_entry *ymf281_device::device_rom_region() const
{
	return ROM_NAME( ymf281 );
}


//*********************************************************
//  DS1001 DEVICE (Konami VRC7)
//*********************************************************

//-------------------------------------------------
//  ds1001_device - constructor
//-------------------------------------------------

ds1001_device::ds1001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ym2413_device(mconfig, tag, owner, clock, DS1001)
{
}


//-------------------------------------------------
//  device_rom_region - return a pointer to our
//  ROM region
//-------------------------------------------------

ROM_START( ds1001 )
	ROM_REGION( 0x90, "internal", 0 )
	//
	// This is not the exact format
	//
	ROM_LOAD16_WORD( "ds1001_instruments.bin", 0x0000, 0x0090, CRC(9d699efc) SHA1(7adf1d77bab12c50ebfa9921774f9aea1e74dd7b) )
ROM_END

const tiny_rom_entry *ds1001_device::device_rom_region() const
{
	return ROM_NAME( ds1001 );
}
