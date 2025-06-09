// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ymopl.h"


//*********************************************************
//  YM3526 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM3526, ym3526_device, "ym3526", "YM3526 OPL")

//-------------------------------------------------
//  ym3526_device - constructor
//-------------------------------------------------

ym3526_device::ym3526_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym3526>(mconfig, tag, owner, clock, YM3526)
{
}



//*********************************************************
//  Y8950 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(Y8950, y8950_device, "y8950", "Y8950 OPL MSX-Audio")

//-------------------------------------------------
//  y8950_device - constructor
//-------------------------------------------------

y8950_device::y8950_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::y8950>(mconfig, tag, owner, clock, Y8950),
	device_rom_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void y8950_device::rom_bank_pre_change()
{
	m_stream->update();
}


//-------------------------------------------------
//  ymfm_external_read - callback to read data for
//  the ADPCM-B engine; in this case, from our
//  default address space
//-------------------------------------------------

uint8_t y8950_device::ymfm_external_read(ymfm::access_class type, uint32_t offset)
{
	if (type == ymfm::ACCESS_ADPCM_B)
		return read_byte(offset);
	return parent::ymfm_external_read(type, offset);
}


//-------------------------------------------------
//  ymfm_external_write - callback to write data to
//  the ADPCM-B engine; in this case, to our
//  default address space
//-------------------------------------------------

void y8950_device::ymfm_external_write(ymfm::access_class type, uint32_t offset, uint8_t data)
{
	if (type == ymfm::ACCESS_ADPCM_B)
		return space().write_byte(offset, data);
	parent::ymfm_external_write(type, offset, data);
}



//*********************************************************
//  YM3812 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM3812, ym3812_device, "ym3812", "YM3812 OPL2")

//-------------------------------------------------
//  ym3812_device - constructor
//-------------------------------------------------

ym3812_device::ym3812_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym3812>(mconfig, tag, owner, clock, YM3812)
{
}



//*********************************************************
//  YMF262 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YMF262, ymf262_device, "ymf262", "YMF262 OPL3")

//-------------------------------------------------
//  ymf262_device - constructor
//-------------------------------------------------

ymf262_device::ymf262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ymf262>(mconfig, tag, owner, clock, YMF262)
{
}



//*********************************************************
//  YMF278B DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YMF278B, ymf278b_device, "ymf278b", "YMF278B OPL4")

//-------------------------------------------------
//  ymf278b_device - constructor
//-------------------------------------------------

ymf278b_device::ymf278b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ymf278b>(mconfig, tag, owner, clock, YMF278B),
	device_rom_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void ymf278b_device::rom_bank_pre_change()
{
	m_stream->update();
}


//-------------------------------------------------
//  ymfm_external_read - callback to read data for
//  the ADPCM-B engine; in this case, from our
//  default address space
//-------------------------------------------------

uint8_t ymf278b_device::ymfm_external_read(ymfm::access_class type, uint32_t offset)
{
	if (type == ymfm::ACCESS_PCM)
		return read_byte(offset);
	return 0;
}


//-------------------------------------------------
//  ymfm_external_write - callback to write data to
//  the ADPCM-B engine; in this case, to our
//  default address space
//-------------------------------------------------

void ymf278b_device::ymfm_external_write(ymfm::access_class type, uint32_t offset, uint8_t data)
{
	if (type == ymfm::ACCESS_PCM)
		return space().write_byte(offset, data);
}


//-------------------------------------------------
//  ymfm_external_write - callback to write data to
//  the ADPCM-B engine; in this case, to our
//  default address space
//-------------------------------------------------

void ymf278b_device::sound_stream_update(sound_stream &stream)
{
	// rotate the outputs so that the DO2 outputs are first
	parent::update_internal(stream, 2);
}



//*********************************************************
//  YM2413 DEVICE
//*********************************************************

DEFINE_DEVICE_TYPE(YM2413, ym2413_device, "ym2413", "YM2413 OPLL")

//-------------------------------------------------
//  ym2413_device - constructor
//-------------------------------------------------

ym2413_device::ym2413_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym2413>(mconfig, tag, owner, clock, YM2413),
	m_internal(*this, "internal")
{
}


//-------------------------------------------------
//  device_start - device startup
//-------------------------------------------------

void ym2413_device::device_start()
{
	parent::device_start();
	m_chip.set_instrument_data(m_internal);
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



//*********************************************************
//  YM2423 DEVICE (OPLL-X)
//*********************************************************

DEFINE_DEVICE_TYPE(YM2423, ym2423_device, "ym2423", "YM2423 OPLL-X")

//-------------------------------------------------
//  ym2423_device - constructor
//-------------------------------------------------

ym2423_device::ym2423_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ym2423>(mconfig, tag, owner, clock, YM2423),
	m_internal(*this, "internal")
{
}


//-------------------------------------------------
//  device_start - device startup
//-------------------------------------------------

void ym2423_device::device_start()
{
	parent::device_start();
	m_chip.set_instrument_data(m_internal);
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

DEFINE_DEVICE_TYPE(YMF281, ymf281_device, "ymf281", "YMF281 OPLLP")

//-------------------------------------------------
//  ymf281_device - constructor
//-------------------------------------------------

ymf281_device::ymf281_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ymf281>(mconfig, tag, owner, clock, YMF281),
	m_internal(*this, "internal")
{
}


//-------------------------------------------------
//  device_start - device startup
//-------------------------------------------------

void ymf281_device::device_start()
{
	parent::device_start();
	m_chip.set_instrument_data(m_internal);
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

DEFINE_DEVICE_TYPE(DS1001, ds1001_device, "ds1001", "Yamaha DS1001 / Konami 053982")

//-------------------------------------------------
//  ds1001_device - constructor
//-------------------------------------------------

ds1001_device::ds1001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ymfm_device_base<ymfm::ds1001>(mconfig, tag, owner, clock, DS1001),
	m_internal(*this, "internal")
{
}


//-------------------------------------------------
//  device_start - device startup
//-------------------------------------------------

void ds1001_device::device_start()
{
	parent::device_start();
	m_chip.set_instrument_data(m_internal);
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
