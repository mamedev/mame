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
  is integrated into the 054544 and 054986A hybrid chips.  The hybrid
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

void k054321_device::main_map(address_map &map)
{
	map(0x0, 0x0).w(FUNC(k054321_device::active_w));
	map(0x2, 0x2).w(FUNC(k054321_device::volume_reset_w));
	map(0x3, 0x3).w(FUNC(k054321_device::volume_up_w));
	map(0x4, 0x4).w(FUNC(k054321_device::dummy_w));
	map(0x6, 0x6).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
	map(0x7, 0x7).w(m_soundlatch[1], FUNC(generic_latch_8_device::write));
	map(0x8, 0x8).r(FUNC(k054321_device::busy_r));
	map(0xa, 0xa).r(m_soundlatch[2], FUNC(generic_latch_8_device::read));
}

void k054321_device::sound_map(address_map &map)
{
	map(0x0, 0x0).w(m_soundlatch[2], FUNC(generic_latch_8_device::write));
	map(0x2, 0x2).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	map(0x3, 0x3).r(m_soundlatch[1], FUNC(generic_latch_8_device::read));
}

k054321_device::k054321_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, K054321, tag, owner, clock),
	m_left(*this, finder_base::DUMMY_TAG),
	m_right(*this, finder_base::DUMMY_TAG),
	m_soundlatch(*this, "soundlatch%u", 0)
{
}

void k054321_device::device_start()
{
	// make sure that device_sound_interface is configured
	if (!m_left->inputs() && !m_right->inputs())
		throw device_missing_dependencies();

	// remember initial input gains
	m_left_gains = std::make_unique<float[]>(m_left->inputs());
	m_right_gains = std::make_unique<float[]>(m_right->inputs());

	for (int i = 0; i < m_left->inputs(); i++)
		m_left_gains[i] = m_left->input_gain(i);
	for (int i = 0; i < m_right->inputs(); i++)
		m_right_gains[i] = m_right->input_gain(i);

	// register for savestates
	save_item(NAME(m_volume));
	save_item(NAME(m_active));
}

void k054321_device::device_add_mconfig(machine_config &config)
{
	for (int i = 0; i < 3; i++)
		GENERIC_LATCH_8(config, m_soundlatch[i]);
}

WRITE8_MEMBER(k054321_device::volume_reset_w)
{
	m_volume = 0;
	propagate_volume();
}

WRITE8_MEMBER(k054321_device::volume_up_w)
{
	// assume that max volume is 64
	if (data && m_volume < 64)
	{
		m_volume++;
		propagate_volume();
	}
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

	for (int i = 0; i < m_left->inputs(); i++)
		m_left->set_input_gain(i, m_active & 2 ? vol * m_left_gains[i] : 0.0);
	for (int i = 0; i < m_right->inputs(); i++)
		m_right->set_input_gain(i, m_active & 1 ? vol * m_right_gains[i] : 0.0);
}
