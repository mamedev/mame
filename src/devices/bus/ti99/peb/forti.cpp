// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*******************************************************************************
    FORTi Sound card

    4 x TMS9919 sound generators
    4 sound outputs; may be coupled to two stereo outputs

    Mapping:

    1000 01xx xxxD CBA0

    8402: Sound chip 1 (A)
    8404: Sound chip 2 (B)
    8408: Sound chip 3 (C)
    8410: Sound chip 4 (D)

    Sound chips may be addressed in parallel:
    8406: Sound chips 1+2
    841a: Sound chips 1+3+4

    All sound READY lines are combined by a wired-AND as the common
    READY line to the CPU.

    Michael Zapf
    March 2020

*******************************************************************************/

#include "emu.h"
#include "forti.h"

#define LOG_READY       (1U << 1)

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

#define FORTI_GEN1_TAG "soundchip1"
#define FORTI_GEN2_TAG "soundchip2"
#define FORTI_GEN3_TAG "soundchip3"
#define FORTI_GEN4_TAG "soundchip4"

DEFINE_DEVICE_TYPE(TI99_FORTI, bus::ti99::peb::forti_device, "ti99_forti", "FORTi Sound Card")

namespace bus::ti99::peb {

forti_device::forti_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	  device_t(mconfig, TI99_FORTI, tag, owner, clock),
	  device_ti99_peribox_card_interface(mconfig, *this),
	  m_generator1(*this, FORTI_GEN1_TAG),
	  m_generator2(*this, FORTI_GEN2_TAG),
	  m_generator3(*this, FORTI_GEN3_TAG),
	  m_generator4(*this, FORTI_GEN4_TAG)
{
}

/*
    No read access. The FORTi card does not support any reading.
*/
void forti_device::readz(offs_t offset, uint8_t *value)
{
	return;
}

/*
    READY callbacks from the sound chips.
*/
void forti_device::ready_sound(int state)
{
	LOGMASKED(LOG_READY, "READY (%d, %d, %d, %d)\n",  m_generator1->ready_r(),
		m_generator2->ready_r(), m_generator3->ready_r(), m_generator4->ready_r());

	line_state ready = (m_generator1->ready_r() && m_generator2->ready_r()
		 && m_generator3->ready_r() && m_generator4->ready_r())? ASSERT_LINE : CLEAR_LINE;

	m_slot->set_ready(ready);
}

void forti_device::write(offs_t offset, uint8_t data)
{
	// Decode for 8400-87ff
	if ((offset & 0xfc01) == 0x8400)
	{
		// The generators can be accessed in parallel
		if ((offset & 0x2)!=0)
			m_generator1->write(data);

		if ((offset & 0x4)!=0)
			m_generator2->write(data);

		if ((offset & 0x8)!=0)
			m_generator3->write(data);

		if ((offset & 0x10)!=0)
			m_generator4->write(data);
	}
}

void forti_device::device_add_mconfig(machine_config& config)
{
	// 1 and 3 are mixed to left channel
	// 2 and 4 are moxed to right channel

	SPEAKER(config, "forti", 2).front();

	SN94624(config, m_generator1, XTAL(3'579'545)/8);
	m_generator1->ready_cb().set(FUNC(forti_device::ready_sound));
	m_generator1->add_route(ALL_OUTPUTS, "forti", 0.75, 0);

	SN94624(config, m_generator2, XTAL(3'579'545)/8);
	m_generator2->ready_cb().set(FUNC(forti_device::ready_sound));
	m_generator2->add_route(ALL_OUTPUTS, "forti", 0.75, 1);

	SN94624(config, m_generator3, XTAL(3'579'545)/8);
	m_generator3->ready_cb().set(FUNC(forti_device::ready_sound));
	m_generator3->add_route(ALL_OUTPUTS, "forti", 0.75, 0);

	SN94624(config, m_generator4, XTAL(3'579'545)/8);
	m_generator4->ready_cb().set(FUNC(forti_device::ready_sound));
	m_generator4->add_route(ALL_OUTPUTS, "forti", 0.75, 1);
}

void forti_device::device_start()
{
}

void forti_device::device_reset()
{
}

} // end namespace bus::ti99::peb
