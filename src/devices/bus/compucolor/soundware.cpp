// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CAP Electronics SoundWare

    Internal amplifier on the serial port TxD line. Software makes tones
    by toggling the TMS5501 break bit.

    TxD -> 470K -> 10K volume pot -> tantalum coupling capacitor ->
    LM380N-8 (gain 50) -> 220uF -> 15 ohm speaker

**********************************************************************/

#include "emu.h"
#include "soundware.h"

#include "machine/rescap.h"
#include "sound/flt_rc.h"
#include "sound/spkrdev.h"

#include "speaker.h"


namespace {

class compucolor_soundware_device : public device_t, public device_rs232_port_interface
{
public:
	compucolor_soundware_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, COMPUCOLOR_SOUNDWARE, tag, owner, clock)
		, device_rs232_port_interface(mconfig, *this)
		, m_speaker(*this, "speaker")
	{
	}

	// device_rs232_port_interface implementation
	virtual void input_txd(int state) override { m_speaker->level_w(state); }

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD { }

private:
	required_device<speaker_sound_device> m_speaker;
};


void compucolor_soundware_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "filter", 1.0);

	FILTER_RC(config, "filter").set_rc(filter_rc_device::HIGHPASS, 15, 0, 0, CAP_U(220)).add_route(ALL_OUTPUTS, "mono", 0.5);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(COMPUCOLOR_SOUNDWARE, device_rs232_port_interface, compucolor_soundware_device, "compclr_soundware", "CAP Electronics SoundWare")
