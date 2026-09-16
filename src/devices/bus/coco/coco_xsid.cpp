// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    CoCo X SID

    Uses X-SID V2.0: SID replacement for 6581/8580 with Audio Boost.

***************************************************************************/

#include "emu.h"
#include "coco_xsid.h"

#include "sound/mos6581.h"

#include "speaker.h"


namespace {

class coco_xsid_device : public device_t, public device_cococart_interface
{
	public:
		// construction/destruction
		coco_xsid_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: device_t(mconfig, COCO_XSID, tag, owner, clock)
			, device_cococart_interface(mconfig, *this )
			, m_speaker(*this, "speaker")
			, m_sid(*this, "mos8580")
			, m_audio_sw(*this, "AUDIO_SW")
		{
		}

		DECLARE_INPUT_CHANGED_MEMBER(audio_switch_changed);

	protected:
		virtual void device_start() override ATTR_COLD { }
		virtual void device_post_load() override;

		virtual ioport_constructor device_input_ports() const override ATTR_COLD;

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
		{
			// dedicated mono output jack, distinct from the cart-audio path
			SPEAKER(config, m_speaker).front_center();
			MOS8580(config, m_sid, 1'000'000); // not sure what it's derived from
			m_sid->add_route(ALL_OUTPUTS, m_speaker, 1.0);
		}

		virtual u8 scs_read(offs_t offset) override
		{
			return m_sid->read(offset);
		}

		virtual void scs_write(offs_t offset, u8 data) override
		{
			m_sid->write(offset, data);
		}

		void device_resolve_objects() override
		{
			// Mono back to the CoCo system cartridge audio input line
			add_sound_route(*m_sid, ALL_OUTPUTS, 0.0);
		}

	private:
		required_device<speaker_device> m_speaker;
		required_device<mos8580_device> m_sid;
		required_ioport m_audio_sw;

		void update_audio_routing()
		{
			if (m_audio_sw->read() == 0)
			{
				// Cartridge: mono back to the CoCo system cartridge audio input line
				set_sound_gain(*m_sid, 0, 1.0);
				m_sid->set_route_gain(0, m_speaker, 0, 0.0);
			}
			else
			{
				// External Jack: SID's own front-panel 2.5mm mono output
				set_sound_gain(*m_sid, 0, 0.0);
				m_sid->set_route_gain(0, m_speaker, 0, 1.0);
			}
		}
};

INPUT_PORTS_START(coco_xsid)
	PORT_START("AUDIO_SW")
	PORT_CONFNAME(0x01, 0x01, "X-SID Audio Output") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(coco_xsid_device::audio_switch_changed), 0)
	PORT_CONFSETTING(0x00, "Cartridge (system audio)")
	PORT_CONFSETTING(0x01, "External Jack")
INPUT_PORTS_END

void coco_xsid_device::device_post_load()
{
	update_audio_routing();
}

ioport_constructor coco_xsid_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(coco_xsid);
}

INPUT_CHANGED_MEMBER(coco_xsid_device::audio_switch_changed)
{
	update_audio_routing();
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(COCO_XSID, device_cococart_interface, coco_xsid_device, "coco_xsid", "CoCo X-SID")
