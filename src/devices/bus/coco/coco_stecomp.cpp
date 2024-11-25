// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    coco_stecomp.cpp


    Code for emulating The Stereo Composer by Speech Systems

    This cartridge is a simple sound cartridge. It had two 8-bit DACs
    connected thru a PIA. It contained no ROM.

***************************************************************************/

#include "emu.h"
#include "coco_stecomp.h"
#include "cococart.h"

#include "sound/dac.h"
#include "speaker.h"
#include "machine/6821pia.h"


//**************************************************************************
//  STEREO_COMPOSER DEVICE CLASS
//**************************************************************************

namespace
{
	// ======================> coco_stereo_composer_device

	class coco_stereo_composer_device :
		public device_t,
		public device_cococart_interface
	{
	public:
		// construction/destruction
		coco_stereo_composer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: device_t(mconfig, COCO_STEREO_COMPOSER, tag, owner, clock)
			, device_cococart_interface(mconfig, *this)
			, m_pia(*this, "sc_pia")
			, m_ldac(*this, "sc_ldac")
			, m_rdac(*this, "sc_rdac")
		{
		}

	protected:
		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

		// device-level overrides
		virtual void device_start() override
		{
			// install handlers
			install_readwrite_handler( 0xFF70, 0xFF73, read8sm_delegate(*m_pia, FUNC(pia6821_device::read)), write8sm_delegate(*m_pia, FUNC(pia6821_device::write)));
		}

	private:
		// internal state
		required_device<pia6821_device> m_pia;
		required_device<dac_byte_interface> m_ldac;
		required_device<dac_byte_interface> m_rdac;
	};

	//**************************************************************************
	//  STEREO_COMPOSER MACHINE DECLARATIONS
	//**************************************************************************

	void coco_stereo_composer_device::device_add_mconfig(machine_config &config)
	{
		SPEAKER(config, "sc_lspeaker").front_left();
		SPEAKER(config, "sc_rspeaker").front_right();
		DAC_8BIT_R2R(config, m_ldac).add_route(ALL_OUTPUTS, "sc_lspeaker", 0.5);
		DAC_8BIT_R2R(config, m_rdac).add_route(ALL_OUTPUTS, "sc_rspeaker", 0.5);

		pia6821_device &pia(PIA6821(config, "sc_pia"));
		pia.writepa_handler().set("sc_ldac", FUNC(dac_byte_interface::data_w));
		pia.writepb_handler().set("sc_rdac", FUNC(dac_byte_interface::data_w));
	}
}

//**************************************************************************
//  DEVICE DECLARATION
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_STEREO_COMPOSER, device_cococart_interface, coco_stereo_composer_device, "coco_stereo_composer", "Speech Systems Stereo Composer")
