// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
    Mitsumi FX5400W CD-ROM Drive
*/
#include "emu.h"
#include "fx5400w.h"


DEFINE_DEVICE_TYPE(FX5400W, mitsumi_fx5400w_device, "fx5400w", "Mitsumi FX5400W CD-ROM Drive")

mitsumi_fx5400w_device::mitsumi_fx5400w_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	atapi_cdrom_device(mconfig, FX5400W, tag, owner, clock)
{
}

void mitsumi_fx5400w_device::device_start()
{
	atapi_cdrom_device::device_start();

	m_identify_buffer[ 27 ] = ('M' << 8) | 'I';
	m_identify_buffer[ 28 ] = ('T' << 8) | 'S';
	m_identify_buffer[ 29 ] = ('U' << 8) | 'M';
	m_identify_buffer[ 30 ] = ('I' << 8) | ' ';
	m_identify_buffer[ 31 ] = ('C' << 8) | 'D';
	m_identify_buffer[ 32 ] = ('-' << 8) | 'R';
	m_identify_buffer[ 33 ] = ('O' << 8) | 'M';
	m_identify_buffer[ 34 ] = (' ' << 8) | 'F';
	m_identify_buffer[ 35 ] = ('X' << 8) | '5';
	m_identify_buffer[ 36 ] = ('4' << 8) | '+';
	m_identify_buffer[ 37 ] = ('+' << 8) | 'W';
	m_identify_buffer[ 38 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 39 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 40 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 41 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 42 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 43 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 44 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 45 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 46 ] = (' ' << 8) | ' ';
}
