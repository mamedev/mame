// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Ernesto Corvi, Nicola Salmoria
/***************************************************************************

  gaplus.cpp

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "gaplus.h"


/************************************************************************************
*                                                                                   *
*           Gaplus custom I/O chips (preliminary)                                   *
*                                                                                   *
************************************************************************************/

void gaplus_base_state::customio_3_w(offs_t offset, uint8_t data)
{
	if ((offset == 0x09) && (data >= 0x0f))
		m_samples->start(0,0);

	m_customio_3[offset] = data;
}


uint8_t gaplus_base_state::customio_3_r(offs_t offset)
{
	int mode = m_customio_3[8];

	switch (offset)
	{
		case 0:
			return ioport("IN2")->read();       /* cabinet & test mode */
		case 1:
			return (mode == 2) ? m_customio_3[offset] : 0x0f;
		case 2:
			return (mode == 2) ? 0x0f : 0x0e;
		case 3:
			return (mode == 2) ? m_customio_3[offset] : 0x01;
		default:
			return m_customio_3[offset];
	}
}
