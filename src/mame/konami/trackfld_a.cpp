// license:BSD-3-Clause
// copyright-holders:Chris Hardy
#include "emu.h"
#include "sound/sn76496.h"
#include "sound/msm5205.h"
#include "trackfld.h"
#include "trackfld_a.h"


#define TIMER_RATE (4096/4)



DEFINE_DEVICE_TYPE(TRACKFLD_AUDIO, trackfld_audio_device, "trackfld_audio", "Track And Field Audio")

trackfld_audio_device::trackfld_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TRACKFLD_AUDIO, tag, owner, clock)
	, m_audiocpu(*this, finder_base::DUMMY_TAG)
	, m_vlm(*this, finder_base::DUMMY_TAG)
	, m_last_addr(0)
	, m_last_irq(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void trackfld_audio_device::device_start()
{
	/* sound */
	save_item(NAME(m_last_addr));
	save_item(NAME(m_last_irq));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void trackfld_audio_device::device_reset()
{
	m_last_addr = 0;
	m_last_irq = 0;
}

/* The timer port on TnF and HyperSports sound hardware is derived from
   a 14.318 MHz clock crystal which is passed  through a couple of 74ls393
    ripple counters.
    Various outputs of the ripper counters clock the various chips.
    The Z80 uses 14.318 MHz / 4 (3.4MHz)
    The SN chip uses 14.318 MHz / 8 (1.7MHz)
    And the timer is connected to 14.318 MHz / 4096
    As we are using the Z80 clockrate as a base value we need to multiply
    the no of cycles by 4 to undo the 14.318/4 operation
*/

uint8_t trackfld_audio_device::trackfld_sh_timer_r()
{
	uint32_t clock = m_audiocpu->total_cycles() / TIMER_RATE;

	return clock & 0xF;
}

uint8_t trackfld_audio_device::trackfld_speech_r()
{
	return m_vlm->bsy() ? 0x10 : 0;
}

void trackfld_audio_device::trackfld_sound_w(offs_t offset, uint8_t data)
{
	int changes = offset ^ m_last_addr;

	/* A7 = data enable for VLM5030 (don't care )          */
	/* A8 = STA pin (1->0 data data  , 0->1 start speech   */
	/* A9 = RST pin 1=reset                                */

	/* A8 VLM5030 ST pin */
	if (changes & 0x100)
		m_vlm->st(offset & 0x100);

	/* A9 VLM5030 RST pin */
	if (changes & 0x200)
		m_vlm->rst(offset & 0x200);

	m_last_addr = offset;
}

uint8_t trackfld_audio_device::hyperspt_sh_timer_r()
{
	uint32_t clock = m_audiocpu->total_cycles() / TIMER_RATE;

	if (m_vlm != nullptr)
		return (clock & 0x3) | (m_vlm->bsy() ? 0x04 : 0);
	else
		return (clock & 0x3);
}

void trackfld_audio_device::hyperspt_sound_w(offs_t offset, uint8_t data)
{
	int changes = offset ^ m_last_addr;

	/* A3 = data enable for VLM5030 (don't care )          */
	/* A4 = STA pin (1->0 data data  , 0->1 start speech   */
	/* A5 = RST pin 1=reset                                */
	/* A6 = VLM5030    output disable (don't care ) */
	/* A7 = kONAMI DAC output disable (don't care ) */
	/* A8 = SN76489AN  output disable (don't care ) */

	/* A4 VLM5030 ST pin */
	if (changes & 0x10)
		m_vlm->st(offset & 0x10);

	/* A5 VLM5030 RST pin */
	if( changes & 0x20 )
		m_vlm->rst(offset & 0x20);

	m_last_addr = offset;
}



void trackfld_audio_device::sh_irqtrigger_w(int state)
{
	if (m_last_irq == 0 && state)
	{
		/* setting bit 0 low then high triggers IRQ on the sound CPU */
		m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
	}

	m_last_irq = state;
}
