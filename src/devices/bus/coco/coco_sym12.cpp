// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    coco_sym12.cpp


    Code for emulating The Symphony 12

    Made by Speech Systems, 1982.

    This cartridge is a complex sound cartridge. It had 4 AY-3-8910 PSG
    connected thru a PIA. It contained no ROM.

***************************************************************************/

#include "emu.h"
#include "coco_sym12.h"
#include "cococart.h"

#include "speaker.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"

//**************************************************************************
//  SYMPHONY_TWELVE DEVICE CLASS
//**************************************************************************

namespace
{
	// ======================> coco_symphony_twelve_device

	class coco_symphony_twelve_device :
		public device_t,
		public device_cococart_interface
	{
	public:
		// construction/destruction
		coco_symphony_twelve_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: device_t(mconfig, COCO_SYM12, tag, owner, clock)
			, device_cococart_interface(mconfig, *this)
			, m_pia(*this, "s12_pia")
			, m_ay8910(*this, "s12_ay8910.%u", 1)
		{
		}

	protected:
		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

		// device-level overrides
		virtual void device_start() override
		{
			// install handlers
			install_readwrite_handler( 0xff60, 0xff63, read8sm_delegate(*m_pia, FUNC(pia6821_device::read)), write8sm_delegate(*m_pia, FUNC(pia6821_device::write)));
		}

		u8 read_porta();
		void write_porta(u8 data);
		void write_portb(u8 data);
		void write_psg(u8 bus, u8 data);

	private:
		// internal state
		required_device<pia6821_device> m_pia;
		required_device_array<ay8910_device, 4> m_ay8910;
	};

	//**************************************************************************
	//  SYMPHONY_TWELVE MACHINE DECLARATIONS
	//**************************************************************************

	void coco_symphony_twelve_device::device_add_mconfig(machine_config &config)
	{
		pia6821_device &pia(PIA6821(config, "s12_pia"));
		pia.writepa_handler().set(*this, FUNC(coco_symphony_twelve_device::write_porta));
		pia.readpa_handler().set(*this, FUNC(coco_symphony_twelve_device::read_porta));
		pia.writepb_handler().set(*this, FUNC(coco_symphony_twelve_device::write_portb));

		SPEAKER(config, "s12_l").front_left();
		SPEAKER(config, "s12_r").front_right();
		AY8910(config, m_ay8910[0], DERIVED_CLOCK(1, 1));
		m_ay8910[0]->set_flags(AY8910_SINGLE_OUTPUT);
		m_ay8910[0]->add_route(ALL_OUTPUTS, "s12_l", 0.50);

		AY8910(config, m_ay8910[1], DERIVED_CLOCK(1, 1));
		m_ay8910[1]->set_flags(AY8910_SINGLE_OUTPUT);
		m_ay8910[1]->add_route(ALL_OUTPUTS, "s12_l", 0.50);

		AY8910(config, m_ay8910[2], DERIVED_CLOCK(1, 1));
		m_ay8910[2]->set_flags(AY8910_SINGLE_OUTPUT);
		m_ay8910[2]->add_route(ALL_OUTPUTS, "s12_r", 0.50);

		AY8910(config, m_ay8910[3], DERIVED_CLOCK(1, 1));
		m_ay8910[3]->set_flags(AY8910_SINGLE_OUTPUT);
		m_ay8910[3]->add_route(ALL_OUTPUTS, "s12_r", 0.50);

	}

	//**************************************************************************
	//  SYMPHONY_TWELVE PSG I/O
	//**************************************************************************

	u8 coco_symphony_twelve_device::read_porta()
	{
		uint8_t b_output = m_pia->b_output();
		u8 result = 0;

		if ((b_output & 0x03) == 0x01) {
			result |= m_ay8910[0]->data_r();
		}

		if ((b_output & 0x0c) == 0x04) {
			result |= m_ay8910[1]->data_r();
		}

		if ((b_output & 0x30) == 0x10) {
			result |= m_ay8910[2]->data_r();
		}

		if ((b_output & 0xc0) == 0x40) {
			result |= m_ay8910[3]->data_r();
		}

		return result;
	}

	void coco_symphony_twelve_device::write_porta(u8 data)
	{
		write_psg(m_pia->b_output(), data);
	}

	void coco_symphony_twelve_device::write_portb(u8 data)
	{
		write_psg(data, m_pia->a_output());
	}

	void coco_symphony_twelve_device::write_psg(u8 bus, u8 data)
	{
		if ((bus & 0x03) == 0x03) {
			m_ay8910[0]->address_w(data);
		}

		if ((bus & 0x03) == 0x02) {
			m_ay8910[0]->data_w(data);
		}

		if ((bus & 0x0c) == 0x0c) {
			m_ay8910[1]->address_w(data);
		}

		if ((bus & 0x0c) == 0x08) {
			m_ay8910[1]->data_w(data);
		}

		if ((bus & 0x30) == 0x30) {
			m_ay8910[2]->address_w(data);
		}

		if ((bus & 0x30) == 0x20) {
			m_ay8910[2]->data_w(data);
		}

		if ((bus & 0xc0) == 0xc0) {
			m_ay8910[3]->address_w(data);
		}

		if ((bus & 0xc0) == 0x80) {
			m_ay8910[3]->data_w(data);
		}
	}
}


//**************************************************************************
//  DEVICE DECLARATION
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_SYM12, device_cococart_interface, coco_symphony_twelve_device, "coco_symphony_twelve", "Speech Systems Symphony Twelve")
