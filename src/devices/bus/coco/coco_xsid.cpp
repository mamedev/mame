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
			, m_sid(*this, "mos8580")
		{
		}

	protected:
		virtual void device_start() override ATTR_COLD { }

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
		{
			SPEAKER(config, "speaker").front_center();
			MOS8580(config, m_sid, 1'000'000); // not sure what it's derived from
			m_sid->add_route(ALL_OUTPUTS, "speaker", 1.0);
		}

		virtual u8 scs_read(offs_t offset) override
		{
			return m_sid->read(offset);
		}

		virtual void scs_write(offs_t offset, u8 data) override
		{
			m_sid->write(offset, data);
		}

	private:
		required_device<mos8580_device> m_sid;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(COCO_XSID, device_cococart_interface, coco_xsid_device, "coco_xsid", "CoCo X-SID")
