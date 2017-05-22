// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_rs232.cpp

    Code for emulating the CoCo RS-232 PAK

***************************************************************************/

#include "emu.h"
#include "cococart.h"
#include "machine/mos6551.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define UART_TAG        "uart"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_rs232_device

namespace
{
	class coco_rs232_device :
		public device_t,
		public device_cococart_interface
	{
	public:
		// construction/destruction
		coco_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
			: device_t(mconfig, COCO_RS232, tag, owner, clock)
			, device_cococart_interface(mconfig, *this)
			, m_uart(*this, UART_TAG)
		{
		}

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override;


	protected:
		// device-level overrides
		virtual void device_start() override
		{
			install_readwrite_handler(0xFF68, 0xFF6B,
				read8_delegate(FUNC(mos6551_device::read), (mos6551_device *)m_uart),
				write8_delegate(FUNC(mos6551_device::write), (mos6551_device *)m_uart));
		}

	private:
		// internal state
		required_device<mos6551_device> m_uart;
	};
};


/***************************************************************************
IMPLEMENTATION
***************************************************************************/

MACHINE_CONFIG_MEMBER(coco_rs232_device::device_add_mconfig)
	MCFG_DEVICE_ADD(UART_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
MACHINE_CONFIG_END


//**************************************************************************
//  DEVICE DECLARATION
//**************************************************************************

DEFINE_DEVICE_TYPE(COCO_RS232, coco_rs232_device, "coco_rs232", "CoCo RS-232 PAK")
