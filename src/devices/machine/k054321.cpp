// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

/***************************************************************************/
/*                                                                         */
/*    054321 / 054544 / 054986A                                            */
/*                                                                         */
/*    Konami sound control chip                                            */
/*                                                                         */
/***************************************************************************/

/*
  The 054321 is a sound communication latch/volume manager chip, that
  is integrated into the 054544 and 05489A hybrid chips.  The hybrid
  chips also include the DACs, capacitors, etc needed for the audio
  output.

  The 054321 manages three latches (maybe four) to allow communication
  between the main cpu and the sound cpu, and provides two independent
  busses to ensure decoupling.  It also manages one global volume and
  a per-channel mute for two channels.

  Volume setting is a little strange, with one address to reset it to
  zero and another to increment it by one.  Accepted volume range
  seems to be 1-60, with 40 for "normal".

  The chip seems to be an intermediate step between the 053260
  (full-on sound chip with dual busses and internal latches for
  communication) and the 056800 (MIRAC, similar to the 054321 but
  4-channel).
*/

#include "emu.h"
#include "k054321.h"

#include <math.h>

DEFINE_DEVICE_TYPE(K054321, k054321_device, "k054321", "K054321 Maincpu-Soundcpu interface")

ADDRESS_MAP_START(k054321_device::main_map)
	AM_RANGE(0x0, 0x0) AM_WRITE(active_w)
	AM_RANGE(0x2, 0x2) AM_WRITE(volume_reset_w)
	AM_RANGE(0x3, 0x3) AM_WRITE(volume_up_w)
	AM_RANGE(0x4, 0x4) AM_WRITE(dummy_w)
	AM_RANGE(0x6, 0x6) AM_WRITE(main1_w)
	AM_RANGE(0x7, 0x7) AM_WRITE(main2_w)
	AM_RANGE(0x8, 0x8) AM_READ(busy_r)
	AM_RANGE(0xa, 0xa) AM_READ(sound1_r)
ADDRESS_MAP_END

ADDRESS_MAP_START(k054321_device::sound_map)
	AM_RANGE(0x0, 0x0) AM_WRITE(sound1_w)
	AM_RANGE(0x2, 0x2) AM_READ(main1_r)
	AM_RANGE(0x3, 0x3) AM_READ(main2_r)
ADDRESS_MAP_END

k054321_device::k054321_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K054321, tag, owner, clock),
	  m_left(*this, finder_base::DUMMY_TAG),
	  m_right(*this, finder_base::DUMMY_TAG)
{
}

void k054321_device::set_gain_devices(const char *_left, const char *_right)
{
	m_left.set_tag(_left);
	m_right.set_tag(_right);
}

void k054321_device::device_start()
{
	save_item(NAME(m_main1));
	save_item(NAME(m_main2));
	save_item(NAME(m_sound1));
	save_item(NAME(m_volume));
	save_item(NAME(m_active));
}

READ8_MEMBER( k054321_device::main1_r)
{
	return m_main1;
}

WRITE8_MEMBER(k054321_device::main1_w)
{
	m_main1 = data;
}

READ8_MEMBER( k054321_device::main2_r)
{
	return m_main2;
}

WRITE8_MEMBER(k054321_device::main2_w)
{
	m_main2 = data;
}

READ8_MEMBER( k054321_device::sound1_r)
{
	return m_sound1;
}

WRITE8_MEMBER(k054321_device::sound1_w)
{
	m_sound1 = data;
}

WRITE8_MEMBER(k054321_device::volume_reset_w)
{
	m_volume = 0;
	propagate_volume();
}

WRITE8_MEMBER(k054321_device::volume_up_w)
{
	m_volume ++;
	propagate_volume();
}

READ8_MEMBER(k054321_device::busy_r)
{
	return 0; // bit0 = 1 means busy
}

WRITE8_MEMBER(k054321_device::active_w)
{
	m_active = data;
	propagate_volume();
}

WRITE8_MEMBER(k054321_device::dummy_w)
{
	if(data != 0x4a)
		logerror("unexpected dummy_w %02x\n", data);
}

void k054321_device::propagate_volume()
{
	double vol = pow(2, (m_volume - 40)/10.0);
	m_left->set_input_gain(0, m_active & 2 ? vol : 0.0);
	m_right->set_input_gain(0, m_active & 1 ? vol : 0.0);
}
