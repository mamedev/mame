// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    coco_max.cpp

    Code for emulating CoCo Max Hi-Res Input Module

***************************************************************************/

#include "emu.h"
#include "coco_max.h"

#define MOUSE_SENSITIVITY   75
#define COCOMAX_X_TAG       "cocomax_x"
#define COCOMAX_Y_TAG       "cocomax_y"
#define COCOMAX_BUTTONS     "cocomax_buttons"

// #define VERBOSE (LOG_GENERAL )
#include "logmacro.h"

//-------------------------------------------------
//  INPUT_PORTS( cocomax_mouse )
//-------------------------------------------------

INPUT_PORTS_START( cocomax_mouse )
	PORT_START(COCOMAX_X_TAG)
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_X) PORT_NAME("CoCo Max Mouse X") PORT_SENSITIVITY(MOUSE_SENSITIVITY) PORT_MINMAX(0x00,0xff) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_4_PAD) PORT_CODE_INC(KEYCODE_6_PAD) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1)
	PORT_START(COCOMAX_Y_TAG)
	PORT_BIT( 0xff, 0x80,  IPT_AD_STICK_Y) PORT_NAME("CoCo Max Mouse Y") PORT_SENSITIVITY(MOUSE_SENSITIVITY) PORT_MINMAX(0x00,0xff) PORT_KEYDELTA(10) PORT_CODE_DEC(KEYCODE_8_PAD) PORT_CODE_INC(KEYCODE_2_PAD) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(1)
	PORT_START(COCOMAX_BUTTONS)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("CoCo Max Left Button") PORT_CODE(KEYCODE_0_PAD) PORT_CODE(MOUSECODE_BUTTON1) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("CoCo Max Right Button") PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(MOUSECODE_BUTTON2) PORT_PLAYER(1)
INPUT_PORTS_END

//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

namespace
{
	// ======================> coco_pak_device

	class coco_pak_max_device :
			public device_t,
			public device_cococart_interface
	{
	public:
		// construction/destruction
		coco_pak_max_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	protected:
		// device-level overrides
		virtual void device_start() override ATTR_COLD;
		virtual void device_reset() override ATTR_COLD;
		virtual ioport_constructor device_input_ports() const override ATTR_COLD;

		u8 ff90_read(offs_t offset);

	private:
		required_ioport m_mouse_x;
		required_ioport m_mouse_y;
		required_ioport m_buttons;

		uint8_t m_a2d_result;
	};
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_PAK_MAX, device_cococart_interface, coco_pak_max_device, "cocopakmax", "CoCo Max HI-RES input module")



//-------------------------------------------------
//  coco_pak_device - constructor
//-------------------------------------------------

coco_pak_max_device::coco_pak_max_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, COCO_PAK_MAX, tag, owner, clock)
	, device_cococart_interface(mconfig, *this)
	, m_mouse_x(*this, COCOMAX_X_TAG)
	, m_mouse_y(*this, COCOMAX_Y_TAG)
	, m_buttons(*this, COCOMAX_BUTTONS)
	, m_a2d_result(0)
{
}



ioport_constructor coco_pak_max_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cocomax_mouse);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_pak_max_device::device_start()
{
	// initial state
	m_a2d_result = 0;

	// save state
	save_item(NAME(m_a2d_result));

	// install $ff90-$ff97 handler
	install_read_handler(0xff90, 0xff97, read8sm_delegate(*this, FUNC(coco_pak_max_device::ff90_read)));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void coco_pak_max_device::device_reset()
{
	m_a2d_result = 0;
}



//-------------------------------------------------
//  ff90_read
//-------------------------------------------------

u8 coco_pak_max_device::ff90_read(offs_t offset)
{
	uint8_t result = m_a2d_result;

	switch (offset & 0x07)
	{
		case 0:
			m_a2d_result = m_mouse_y->read();
			break;
		case 1:
			m_a2d_result = m_mouse_x->read();
			break;
		case 2:
			m_a2d_result = BIT(m_buttons->read(), 0) ? 0 : 0xff;
			break;
		case 3:
			m_a2d_result = BIT(m_buttons->read(), 1) ? 0 : 0xff;
			break;
		case 4:
			/* not connected*/
			break;
		case 5:
			/* not connected*/
			break;
		case 6:
			/* not connected*/
			break;
		case 7:
			/* not connected*/
			break;
		default:
			break;
	}

	return result;
}
