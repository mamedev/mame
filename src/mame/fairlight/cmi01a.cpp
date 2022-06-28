// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight CMI-01A Channel Controller Card

***************************************************************************/

#include "emu.h"
#include "cmi01a.h"

#define VERBOSE     (0)
#include "logmacro.h"

#define MASTER_OSCILLATOR       XTAL(34'291'712)

DEFINE_DEVICE_TYPE(CMI01A_CHANNEL_CARD, cmi01a_device, "cmi_01a", "Fairlight CMI-01A Channel Card")


const uint8_t cmi01a_device::s_7497_rate_table[64][64] =
{
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1},
	{1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1},
	{1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1},
	{1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1},
	{1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1},
	{1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1},
	{1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1},
	{1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1},
	{1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1},
	{1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1},
	{1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1},
	{1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1},
	{1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1},
	{1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1},
	{1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1},
	{1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1},
	{1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1},
	{1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1},
	{1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1},
	{1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1},
	{1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1},
	{1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
	{0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1},
	{0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1},
	{0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1},
	{0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1},
	{0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1},
	{0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1},
	{0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1},
	{0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1},
	{0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
	{0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
	{0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
	{0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
	{0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1},
	{0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1},
	{0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1},
	{0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1},
	{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}
};

cmi01a_device::cmi01a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CMI01A_CHANNEL_CARD, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_irq_merger(*this, "cmi01a_irq")
	, m_pia(*this, "cmi01a_pia_%u", 0U)
	, m_ptm(*this, "cmi01a_ptm")
	, m_cmi02_pia(*this, "^cmi02_pia_%u", 1U)
	, m_stream(nullptr)
	, m_irq_cb(*this)
{
}

void cmi01a_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia[0], 0); // pia_cmi01a_1_config
	m_pia[0]->readcb1_handler().set(FUNC(cmi01a_device::tri_r));
	m_pia[0]->readpa_handler().set(FUNC(cmi01a_device::ws_dir_r));
	m_pia[0]->writepa_handler().set(FUNC(cmi01a_device::ws_dir_w));
	m_pia[0]->writepb_handler().set(FUNC(cmi01a_device::rp_w));
	m_pia[0]->ca2_handler().set(FUNC(cmi01a_device::load_w));
	m_pia[0]->cb2_handler().set(FUNC(cmi01a_device::pia_0_cb2_w));
	m_pia[0]->irqa_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<0>));
	m_pia[0]->irqb_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<1>));
	//if (m_channel == 5) m_pia[0]->enable_logging();

	PIA6821(config, m_pia[1], 0); // pia_cmi01a_2_config
	m_pia[1]->readca1_handler().set(FUNC(cmi01a_device::zx_r));
	m_pia[1]->readcb1_handler().set(FUNC(cmi01a_device::eosi_r));
	m_pia[1]->readpa_handler().set(FUNC(cmi01a_device::pia_1_a_r));
	m_pia[1]->writepa_handler().set(FUNC(cmi01a_device::pia_1_a_w));
	m_pia[1]->writepb_handler().set(FUNC(cmi01a_device::pia_1_b_w));
	m_pia[1]->ca2_handler().set(FUNC(cmi01a_device::eload_w));
	m_pia[1]->cb2_handler().set(FUNC(cmi01a_device::wpe_w));
	m_pia[1]->irqa_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<2>));
	m_pia[1]->irqb_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<3>));
	//if (m_channel == 5) m_pia[1]->enable_logging();

	PTM6840(config, m_ptm, DERIVED_CLOCK(1, 1)); // ptm_cmi01a_config
	m_ptm->o1_callback().set(FUNC(cmi01a_device::ptm_o1));
	m_ptm->o2_callback().set(FUNC(cmi01a_device::ptm_o2));
	m_ptm->o3_callback().set(FUNC(cmi01a_device::ptm_o3));
	m_ptm->irq_callback().set(FUNC(cmi01a_device::ptm_irq));

	INPUT_MERGER_ANY_HIGH(config, m_irq_merger).output_handler().set(FUNC(cmi01a_device::cmi01a_irq));
}


void cmi01a_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	if (m_run)
	{
		int mask = m_load ? 0x7fff : 0x7f;
		int addr = m_segment_cnt;

		uint8_t *wave_ptr = &m_wave_ram[m_segment_cnt & 0x3fff];
		auto &buf = outputs[0];

		for (int sampindex = 0; sampindex < buf.samples(); sampindex++)
		{
			const uint8_t sample8 = wave_ptr[addr++ & 0x3fff];
			s32 sample = (int32_t)(int8_t)(sample8 ^ 0x80) * m_env * m_vol_latch;
//          if (m_channel == 5) printf("%08x:%02x:%02x:%02x", (uint32_t)sample, sample8, m_env, m_vol_latch);
			buf.put_int(sampindex, (int16_t)(sample >> 8), 32768);
		}
//      if (m_channel == 5) printf("\n");

		m_segment_cnt = (m_segment_cnt & ~mask) | addr;
	}
	else
		outputs[0].fill(0);
}

void cmi01a_device::device_resolve_objects()
{
	m_irq_cb.resolve_safe();
}

void cmi01a_device::device_start()
{
	m_wave_ram = std::make_unique<uint8_t[]>(0x4000);

	m_zx_timer = timer_alloc(FUNC(cmi01a_device::zx_timer_cb), this);
	m_eosi_timer = timer_alloc(FUNC(cmi01a_device::eosi_timer_cb), this);
	m_bcas_timer = timer_alloc(FUNC(cmi01a_device::bcas_tick), this);

	m_zx_timer->adjust(attotime::never);
	m_eosi_timer->adjust(attotime::never);

	m_stream = stream_alloc(0, 1, 44100);

	m_ptm->set_external_clocks(clock() / 8, clock() / 4, clock() / 4);


}

void cmi01a_device::device_reset()
{
	m_ptm->set_g1(1);
	m_ptm->set_g2(1);
	m_ptm->set_g3(1);

	m_segment_cnt = 0;
	m_new_addr = 0;
	m_vol_latch = 0;
	m_flt_latch = 0;
	m_rp = 0;
	m_ws = 0;
	m_dir = 0;
	m_env = 0;
	m_pia0_cb2_state = 1;
	m_bcas_q1_ticks = 2;
	m_bcas_q1 = 0;
	m_bcas_q2_ticks = 4;
	m_bcas_q2 = 0;
	m_zx_flag = 0;

	m_freq = 0.0;

	m_ptm_o1 = 0;
	m_ptm_o2 = 0;
	m_ptm_o3 = 0;

	m_load = true;
	m_run = false;
	m_gzx = true;
	m_nwpe = true;
	m_tri = true;
	m_pia1_ca2 = false;

	m_eclk = false;
	m_env_clk = false;
	m_ediv_out = true;
	m_ediv_rate = 3;
	m_ediv_count = 0;

	m_pitch = 0;
	m_octave = 0;

	m_zx_timer->adjust(attotime::never);
	m_eosi_timer->adjust(attotime::never);
	//m_bcas_timer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
	m_bcas_timer->adjust(attotime::never);
}

void cmi01a_device::pulse_zcint()
{
	m_pia[0]->ca1_w(0);
	m_pia[0]->ca1_w(1);

	pulse_gzx();
}

void cmi01a_device::pulse_gzx()
{
	if (!m_pia1_ca2)
	{
		return;
	}

	reset_waveform_segment();
	m_env = m_rp;
	m_env_dir = m_dir;
}

void cmi01a_device::reset_waveform_segment()
{
	m_segment_cnt &= 0x007f;
	m_segment_cnt |= (0x4000 | (m_ws << 7));
}

void cmi01a_device::load_w(int state)
{
	const bool old_load = m_load;
	m_load = state ? false : true;

	if (old_load != m_load)
	{
		check_segment_load();
	}
}

void cmi01a_device::check_segment_load()
{
	bool run_strobe = false;
	if (m_load != m_run)
	{
		run_strobe = true;
		m_segment_cnt &= ~0x7f;
	}

	if (!run_strobe)
	{
		return;
	}

	if (!m_nwpe)
	{
		reset_waveform_segment();
	}

	if (m_channel == 5) LOG("CH%d beginning load with m_segment_cnt %04x\n", m_channel, m_segment_cnt);
	m_pia[1]->cb1_w(1);
}

void cmi01a_device::pia_1_a_w(uint8_t data)
{
	m_pitch &= 0x0ff;
	m_pitch |= (data & 3) << 8;
	m_octave = (data >> 2) & 0x0f;
}

uint8_t cmi01a_device::pia_1_a_r()
{
	return ((m_pitch >> 8) & 3) | (m_octave << 2);
}

void cmi01a_device::pia_1_b_w(uint8_t data)
{
	m_pitch &= 0xf00;
	m_pitch |= data;
}

void cmi01a_device::rp_w(uint8_t data)
{
	m_rp = data;
	m_ediv_rate = ((m_rp >> 2) & 0x3c) | 0x03;
	if (m_channel == 5) LOG("CH%d: Initial ramp value: %02x, EDIV divider: %02x\n", m_channel, data, m_ediv_rate);
}

void cmi01a_device::ws_dir_w(uint8_t data)
{
	if (m_channel == 5) LOG("CH%d: WS/DIR write: %02x\n", m_channel, data);
	m_ws = data & 0x7f;
	m_dir = (data >> 7) & 1;
}

uint8_t cmi01a_device::ws_dir_r()
{
	return m_ws | (m_dir << 7);
}

READ_LINE_MEMBER( cmi01a_device::tri_r )
{
	const bool top_terminal_count = (m_env_dir == ENV_DIR_UP && m_env == 0xff);
	const bool bottom_terminal_count = (m_env_dir == ENV_DIR_DOWN && m_env == 0x00);
	const int state = (top_terminal_count || bottom_terminal_count) ? 1 : 0;
	if (m_channel == 5) LOG("CH%d: PIA0 CB1 Read (/TRI): %d\n", m_channel, state);
	return state;
}

WRITE_LINE_MEMBER( cmi01a_device::cmi01a_irq )
{
	m_irq_cb(state ? ASSERT_LINE : CLEAR_LINE);
}

void cmi01a_device::reset_bcas_counter()
{
	m_bcas_q1_ticks = 2;
	m_bcas_q1 = 0;
	m_bcas_q2_ticks = 4;
	m_bcas_q2 = 0;

	m_ptm->set_clock(0, 0);
	m_ptm->set_clock(1, 0);
	m_ptm->set_clock(2, 0);
}

TIMER_CALLBACK_MEMBER(cmi01a_device::bcas_tick)
{
	if (m_ptm_o1 != m_zx_ff)
		return;
	// TODO
}

TIMER_CALLBACK_MEMBER(cmi01a_device::eosi_timer_cb)
{
	m_stream->update();
	m_segment_cnt &= ~0x4000;
	m_pia[1]->cb1_w(0);

	if (m_channel == 5) LOG("CH%d: End of sound\n", m_channel);
}

TIMER_CALLBACK_MEMBER(cmi01a_device::zx_timer_cb)
{
	// Toggle ZX
	m_zx_flag ^= 1;

	// Update ZX input to PIA 1
	m_pia[1]->ca1_w(m_zx_flag);

	// 74LS74 A12 (1) is clocked by /ZX, so a 1->0 transition of the ZX flag is a positive clock transition
	if (m_zx_flag == 0)
	{
		// Pulse /ZCINT if the O1 output of the PTM has changed
		if (m_ptm_o1 != m_zx_ff)
		{
			reset_bcas_counter();
			pulse_zcint();
		}

		m_zx_ff = m_ptm_o1;
	}

	update_eclk();
}

void cmi01a_device::update_eclk()
{
	bool eclk = (m_ptm_o2 && m_zx_ff) || (m_ptm_o3 && !m_zx_ff);
	if (m_channel == 5) logerror("CH%d: eclk = (%d && %d) || (%d && %d) = %d\n", m_channel, m_ptm_o2, m_zx_ff, m_ptm_o3, m_zx_ff ? 0 : 1, eclk ? 1 : 0);
	m_stream->update();
	set_eclk(eclk);
}

void cmi01a_device::wpe_w(int state)
{
	m_nwpe = state ? false : true;
}

void cmi01a_device::eload_w(int state)
{
	if (m_channel == 5) LOG("CH%d PIA1 CA2: %d\n", m_channel, state);
	m_pia1_ca2 = state;
}

void cmi01a_device::clock_envelope()
{
	m_stream->update();
	const bool old_tri = m_tri;
	if (m_env_dir == ENV_DIR_DOWN)
	{
		if (m_env > 0)
		{
			m_env--;
			m_ediv_rate = ((m_env >> 2) & 0x3c) | 0x03;
			if (m_channel == 5) LOG("CH%d, Clocking envelope down, new rp: %02x\n", m_channel, m_env);
		}
		m_tri = m_env == 0x00;
	}
	else
	{
		if (m_env < 0xff)
		{
			m_env++;
			m_ediv_rate = ((~m_env >> 2) & 0x3c) | 0x03;
			if (m_channel == 5) LOG("CH%d, Clocking envelope up, new rp: %02x\n", m_channel, m_env);
		}
		m_tri = m_env == 0xff;
	}
	if (old_tri != m_tri)
	{
		m_pia[0]->cb1_w(m_tri ? 0 : 1);
	}
}

void cmi01a_device::tick_ediv()
{
	if (m_channel == 5) logerror("CH%d ticking ediv, rate: %02x\n", m_channel, m_ediv_rate);
	m_ediv_out = s_7497_rate_table[m_ediv_rate][m_ediv_count];
	m_ediv_count = (m_ediv_count + 1) & 0x3f;
}

void cmi01a_device::set_eclk(bool eclk)
{
	bool old_eclk = m_eclk;
	m_eclk = eclk;

	if (old_eclk == m_eclk)
		return;

	if (!old_eclk && m_eclk)
	{
		tick_ediv();
	}

	//  A   B   !(A && B)   !A || !B
	//  0   0   1           1
	//  0   1   1           1
	//  1   0   1           1
	//  1   1   0           0

	const bool a = !m_load || !eclk;
	const bool b =  m_load || !m_ediv_out;

	const bool old_env_clk = m_env_clk;
	m_env_clk = !a || !b;
	LOG("CH%d checking envelope: A: !!load(%d) || !eclk(%d) = %d\n", m_channel, m_load ? 0 : 1, eclk ? 0 : 1, a);
	LOG("CH%d checking envelope: B:  !load(%d) || !eout(%d) = %d\n", m_channel, m_load ? 1 : 0, m_ediv_out ? 0 : 1, b);
	LOG("CH%d checking envelope: C: !%d || !%d = %d\n", m_channel, a ? 1 : 0, b ? 1 : 0, m_env_clk ? 1 : 0);
	if (!old_env_clk && m_env_clk)
	{
		clock_envelope();
	}
}

void cmi01a_device::run_voice()
{
	if (m_channel == 5) LOG("CH%d running voice: Pitch = %04x\n", m_channel, (uint16_t)m_pitch);
	if (m_channel == 5) LOG("CH%d running voice: o_val = %x\n", m_channel, m_octave);

	int m_tune = m_cmi02_pia[0]->b_output();
	if (m_channel == 5) LOG("CH%d running voice: Tuning = %02x\n", m_channel, (uint8_t)m_tune);
	double mfreq = (double)(0xf00 | m_tune) * ((MASTER_OSCILLATOR.dvalue() / 2.0) / 4096.0);
	if (m_channel == 5) LOG("CH%d running voice: mfreq = %f (%03x * %f)\n", m_channel, mfreq, 0xf00 | m_tune, (MASTER_OSCILLATOR.dvalue() / 2.0) / 4096.0);

	double cfreq = ((double)(0x800 | (m_pitch << 1)) * mfreq) / 4096.0;
	if (m_channel == 5) LOG("CH%d running voice: cfreq = %f (%04x * %f) / 4096.0\n", m_channel, cfreq, 0x800 | (m_pitch << 1), mfreq, cfreq);

	if (cfreq > MASTER_OSCILLATOR.dvalue())
	{
		if (m_channel == 5) LOG("CH%d Ignoring voice run due to excessive frequency\n");
		return;
	}

	if (m_channel == 5) LOG("CH%d Running voice\n", m_channel);
	/* Octave register enabled? */
	if (!BIT(m_octave, 3))
		cfreq /= (double)(2 << ((7 ^ m_octave) & 7));

	cfreq /= 16.0;

	m_freq = cfreq;

	if (m_channel == 5) LOG("CH%d running voice: Final freq: %f\n", m_channel, m_freq);

	m_stream->set_sample_rate(cfreq);

	// Set timers and things
	m_zx_flag = 0;
	attotime zx_period = attotime::from_ticks(64, cfreq);
	m_zx_timer->adjust(zx_period, 0, zx_period);

	if (m_load)
	{
		int samples = 0x4000 - (m_segment_cnt & 0x3fff);
		if (m_channel == 5) LOG("CH%d voice is %04x samples long\n", m_channel, samples);
		m_eosi_timer->adjust(attotime::from_ticks(samples, cfreq));
	}
}

WRITE_LINE_MEMBER( cmi01a_device::pia_0_cb2_w )
{
	int old_state = m_pia0_cb2_state;
	m_pia0_cb2_state = state;
	if (m_channel == 5) LOG("CH%d PIA0 CB2: %d\n", m_channel, state);

	m_stream->update();

	/* RUN */
	if (!old_state && m_pia0_cb2_state)
	{
		m_run = true;

		/* Clear /EOSI */
		m_pia[1]->cb1_w(1);

		/* Only reset address counter if LOAD not asserted */
		if (!m_load)
		{
			m_segment_cnt = 0x4000 | (m_ws << 7);
			m_new_addr = 1;
		}

		/* Clear ZX */
		m_pia[1]->ca1_w(0);

		/* Clear /ZCINT */
		m_pia[0]->ca1_w(1);

		m_ptm->set_g1(0);
		m_ptm->set_g2(0);
		m_ptm->set_g3(0);

		run_voice();
	}

	if (old_state && !m_pia0_cb2_state)
	{
		m_run = false;

		/* Set /EOSI */
		m_pia[1]->cb1_w(0);

		m_ptm->set_g1(1);
		m_ptm->set_g2(1);
		m_ptm->set_g3(1);

		m_zx_timer->adjust(attotime::never);
		m_eosi_timer->adjust(attotime::never);
		m_zx_ff = 0;
	}

}

void cmi01a_device::update_wave_addr(int inc)
{
	int old_cnt = m_segment_cnt;

	if (inc)
		++m_segment_cnt;

	/* Update end of sound interrupt flag */
	m_pia[1]->cb1_w((m_segment_cnt & 0x4000) >> 14);

	/* TODO Update zero crossing flag */
	m_pia[1]->ca1_w((m_segment_cnt & 0x40) >> 6);

	/* Clock a latch on a transition */
	if ((old_cnt & 0x40) && !(m_segment_cnt & 0x40))
	{
		m_pia[1]->ca2_w(1);
		m_pia[1]->ca2_w(0);
	}

	/* Zero crossing interrupt is a pulse */
}

WRITE_LINE_MEMBER( cmi01a_device::ptm_irq )
{
	m_irq_merger->in_w<4>(state);
}

WRITE_LINE_MEMBER( cmi01a_device::ptm_o1 )
{
	m_ptm_o1 = state;
	if (m_ptm_o1 != m_zx_ff)
		reset_bcas_counter();
	update_eclk();
}

WRITE_LINE_MEMBER( cmi01a_device::ptm_o2 )
{
	m_ptm_o2 = state;
	update_eclk();
}

WRITE_LINE_MEMBER( cmi01a_device::ptm_o3 )
{
	m_ptm_o3 = state;
	update_eclk();
}

READ_LINE_MEMBER( cmi01a_device::eosi_r )
{
	if (m_channel == 5) LOG("CH%d PIA1 CB1 Read: %d\n", m_channel, BIT(m_segment_cnt, 14));
	return BIT(m_segment_cnt, 14);
}

READ_LINE_MEMBER( cmi01a_device::zx_r )
{
	return (m_segment_cnt & 0x40) >> 6;
}

void cmi01a_device::write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x0:
			if (m_channel == 5) LOG("%s: CH%d Porthole Write to %04x: %02x\n", machine().describe_context(), m_channel, m_segment_cnt & 0x3fff, data);
			if (m_new_addr)
				m_new_addr = 0;

			m_wave_ram[m_segment_cnt & 0x3fff] = data;
			update_wave_addr(1);
			break;

		case 0x3:
			if (m_channel == 5) LOG("%s: CH%d set Envelope Dir Down (%02x)\n", machine().describe_context(), m_channel, data);
			m_env_dir = ENV_DIR_DOWN;
			break;

		case 0x4:
			if (m_channel == 5) LOG("%s: CH%d set Envelope Dir Up (%02x)\n", machine().describe_context(), m_channel, data);
			m_env_dir = ENV_DIR_UP;
			break;

		case 0x5:
			if (m_channel == 5) LOG("%s: CH%d set Volume Latch: %02x\n", machine().describe_context(), m_channel, data);
			m_vol_latch = data;
			break;

		case 0x6:
			if (m_channel == 5) LOG("%s: CH%d set Filter Latch: %02x\n", machine().describe_context(), m_channel, data);
			m_flt_latch = data;
			break;

		case 0x8: case 0x9: case 0xa: case 0xb:
			if (m_channel == 5) LOG("CH%d PIA0 Write: %d = %02x\n", m_channel, offset & 3, data);
			m_pia[0]->write(offset & 3, data);
			break;

		case 0xc: case 0xd: case 0xe: case 0xf:
			if (m_channel == 5) LOG("CH%d PIA1 Write: %d = %02x\n", m_channel, (BIT(offset, 0) << 1) | BIT(offset, 1), data);
			m_pia[1]->write((BIT(offset, 0) << 1) | BIT(offset, 1), data);
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		{
			/* PTM addressing is a little funky */
			int a0 = offset & 1;
			int a1 = (m_ptm_o1 && BIT(offset, 3)) || (!BIT(offset, 3) && BIT(offset, 2));
			int a2 = BIT(offset, 1);

			if ((offset == 5 || offset == 7) && (data < 0x30))
				data = 0xff;

			if (m_channel == 5) LOG("CH%d PTM Write: %d = %02x\n", m_channel, (a2 << 2) | (a1 << 1) | a0, data);
			m_ptm->write((a2 << 2) | (a1 << 1) | a0, data);
			break;
		}

		default:
			if (m_channel == 5) LOG("%s: Unknown channel card write to E0%02X = %02X\n", machine().describe_context(), offset, data);
			break;
	}
}

uint8_t cmi01a_device::read(offs_t offset)
{
	if (machine().side_effects_disabled())
		return 0;

	uint8_t data = 0;

	switch (offset)
	{
		case 0x0:
			if (m_new_addr)
			{
				m_new_addr = 0;
				break;
			}
			data = m_wave_ram[m_segment_cnt & 0x3fff];
			if (m_channel == 5) LOG("%s: CH%d Porthole Read: %02x\n", machine().describe_context(), m_channel, data);
			update_wave_addr(1);
			break;

		case 0x3:
			if (m_channel == 5) LOG("%s: CH%d set Envelope Dir Down (R)\n", machine().describe_context(), m_channel);
			m_env_dir = ENV_DIR_DOWN;
			break;

		case 0x4:
			if (m_channel == 5) LOG("%s: CH%d set Envelope Dir Up (R)\n", machine().describe_context(), m_channel);
			m_env_dir = ENV_DIR_UP;
			break;

		case 0x5:
			if (m_channel == 5) LOG("%s: CH%d read Volume Latch (ff)\n", machine().describe_context(), m_channel);
			data = 0xff;
			break;

		case 0x8: case 0x9: case 0xa: case 0xb:
			data = m_pia[0]->read(offset & 3);
			if (m_channel == 5) LOG("CH%d PIA0 Read: %d = %02x\n", m_channel, offset & 3, data);
			break;

		case 0xc: case 0xd: case 0xe: case 0xf:
			data = m_pia[1]->read((BIT(offset, 0) << 1) | BIT(offset, 1));
			if (m_channel == 5) LOG("CH%d PIA1 Read: %d = %02x\n", m_channel, (BIT(offset, 0) << 1) | BIT(offset, 1), data);
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		{
			int a0 = offset & 1;
			int a1 = (m_ptm_o1 && BIT(offset, 3)) || (!BIT(offset, 3) && BIT(offset, 2));
			int a2 = BIT(offset, 1);

			data = m_ptm->read((a2 << 2) | (a1 << 1) | a0);

			if (m_channel == 5) LOG("CH%d PTM Read: %d = %02x\n", m_channel, (a2 << 2) | (a1 << 1) | a0, data);
			break;
		}

		default:
			if (m_channel == 5) LOG("%s: Unknown channel card %d read from E0%02X\n", machine().describe_context(), m_channel, offset);
			break;
	}

	if (m_channel == 5) LOG("%s: channel card %d read: %02x = %02x\n", machine().describe_context(), m_channel, offset, data);

	return data;
}
