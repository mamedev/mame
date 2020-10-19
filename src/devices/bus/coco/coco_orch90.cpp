// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_orch90.cpp

    Code for emulating the CoCo Orch-90 (Orchestra 90) sound cartridge

    The Orch-90 was a simple sound cartridge; it had two 8-bit DACs
    supporting stereo sound.  The left channel was at $FF7A, and the right
    channel was at $FF7B


    Hidden Code exists in the ORCH-90 to produce a higher quality output.
    Once you start the PAK, hit "SHIFT+ENTER" at the title screen.  Once you
    are at command, type the following (playing the William Tell default):
    "S" + "ENTER"
    "P" + "SHIFT+ENTER"

    The output will be cleaner due to code causing a change in CPU speed by
    addressing CPU registers to run the 6809 at 2x speed.

    "P" + "ENTER" will play at regular CPU speed.  The difference should be
    very noticeable.

****************************************************************************

	Code for emulating The Stereo Composer by Speech Systems

	This cartridge is a simple sound cartridge. It had two 8-bit DACs
	connected thru a PIA. It contained no ROM.

***************************************************************************/

#include "emu.h"
#include "coco_orch90.h"
#include "cococart.h"

#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"
#include "machine/6821pia.h"


//**************************************************************************
//  ROM DECLARATIONS
//**************************************************************************

ROM_START(coco_orch90)
	ROM_REGION(0x2000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("orchestra 90,1984,26 - 3143,tandy.rom", 0x0000, 0x2000, CRC(15fb39af) SHA1(6a20fee9c70b36a6435ac8378f31d5b626017df0))
ROM_END


//**************************************************************************
//  ORCH90 DEVICE CLASS
//**************************************************************************

namespace
{
	// ======================> coco_orch90_device

	class coco_orch90_device :
		public device_t,
		public device_cococart_interface
	{
	public:
		// construction/destruction
		coco_orch90_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: device_t(mconfig, COCO_ORCH90, tag, owner, clock)
			, device_cococart_interface(mconfig, *this)
			, m_eprom(*this, "eprom")
			, m_ldac(*this, "ldac")
			, m_rdac(*this, "rdac")
		{
		}

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override;

	protected:
		// device-level overrides
		virtual void device_start() override
		{
			// install handlers
			install_write_handler(0xFF7A, 0xFF7A, write8smo_delegate(*this, FUNC(coco_orch90_device::write_left)));
			install_write_handler(0xFF7B, 0xFF7B, write8smo_delegate(*this, FUNC(coco_orch90_device::write_right)));

			// Orch-90 ties CART to Q
			set_line_value(line::CART, line_value::Q);
		}

		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(coco_orch90);
		}

		// CoCo cartridge level overrides
		virtual u8 *get_cart_base() override
		{
			return m_eprom->base();
		}

		virtual memory_region *get_cart_memregion() override
		{
			return m_eprom;
		}

		virtual u8 cts_read(offs_t offset) override;

	private:
		void write_left(u8 data)   { m_ldac->write(data); }
		void write_right(u8 data)  { m_rdac->write(data); }

		// internal state
		required_memory_region m_eprom;
		required_device<dac_byte_interface> m_ldac;
		required_device<dac_byte_interface> m_rdac;
	};
};


//**************************************************************************
//  MACHINE AND ROM DECLARATIONS
//**************************************************************************

void coco_orch90_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DAC_8BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, "lspeaker", 0.5); // ls374.ic5 + r7 (8x20k) + r9 (8x10k)
	DAC_8BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, "rspeaker", 0.5); // ls374.ic4 + r6 (8x20k) + r8 (8x10k)
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "ldac", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "ldac", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "rdac", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "rdac", -1.0, DAC_VREF_NEG_INPUT);
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 coco_orch90_device::cts_read(offs_t offset)
{
	return m_eprom->base()[offset & 0x1fff];
}

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
			: device_t(mconfig, COCO_ORCH90, tag, owner, clock)
			, device_cococart_interface(mconfig, *this)
			, m_pia(*this, "sc_pia")
			, m_ldac(*this, "sc_ldac")
			, m_rdac(*this, "sc_rdac")
		{
		}

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override;

	protected:
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
};

//**************************************************************************
//  STEREO_COMPOSER MACHINE DECLARATIONS
//**************************************************************************

void coco_stereo_composer_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "sc_lspeaker").front_left();
	SPEAKER(config, "sc_rspeaker").front_right();
	DAC_8BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, "sc_lspeaker", 0.5);
	DAC_8BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, "sc_rspeaker", 0.5);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "sc_vref"));
	vref.add_route(0, "sc_ldac", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "sc_ldac", -1.0, DAC_VREF_NEG_INPUT);
	vref.add_route(0, "sc_rdac", 1.0, DAC_VREF_POS_INPUT); vref.add_route(0, "sc_rdac", -1.0, DAC_VREF_NEG_INPUT);

	pia6821_device &pia(PIA6821(config, "sc_pia", 0));
	pia.writepa_handler().set("sc_ldac", FUNC(dac_byte_interface::data_w));
	pia.writepb_handler().set("sc_rdac", FUNC(dac_byte_interface::data_w));

}

//**************************************************************************
//  DEVICE DECLARATION
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_ORCH90, device_cococart_interface, coco_orch90_device, "coco_orch90", "CoCo Orch-90 PAK")
DEFINE_DEVICE_TYPE_PRIVATE(COCO_STEREO_COMPOSER, device_cococart_interface, coco_stereo_composer_device, "coco_stereo_composer", "Speech Systems Stereo Composer")
