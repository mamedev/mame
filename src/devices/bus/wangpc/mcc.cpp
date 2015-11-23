// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC-PM043 Multiport Communications Controller emulation

**********************************************************************/

/*

    TODO:

    - all

*/

#include "mcc.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0

#define OPTION_ID       0x1f

#define Z80SIO2_TAG     "z80sio2"
#define Z80DART_TAG     "z80dart"

#define FUNCTION_PORT1_EXT_CLK      BIT(m_option, 0)
#define FUNCTION_PORT1_NRZI         BIT(m_option, 1)
#define FUNCTION_PORT1_RI_IE        BIT(m_option, 2)
#define FUNCTION_PORT2_EXT_CLK      BIT(m_option, 3)
#define FUNCTION_PORT2_NRZI         BIT(m_option, 4)
#define FUNCTION_PORT2_RI_IE        BIT(m_option, 5)
#define FUNCTION_IRQ_MASK           (m_option & 0xc0)
#define FUNCTION_IRQ2               0x00
#define FUNCTION_IRQ3               0x40
#define FUNCTION_IRQ4               0x80
#define FUNCTION_IRQ_INVALID        0xc0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type WANGPC_MCC = &device_creator<wangpc_mcc_device>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( wangpc_mcc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( wangpc_mcc )
	MCFG_Z80SIO2_ADD(Z80SIO2_TAG, 4000000, 0, 0, 0, 0)
	MCFG_Z80DART_ADD(Z80DART_TAG, 4000000, 0, 0, 0, 0)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor wangpc_mcc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( wangpc_mcc );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_irq -
//-------------------------------------------------

inline void wangpc_mcc_device::set_irq(int state)
{
	m_irq = state;

	switch (FUNCTION_IRQ_MASK)
	{
	case FUNCTION_IRQ2: m_bus->irq2_w(m_irq); break;
	case FUNCTION_IRQ3: m_bus->irq3_w(m_irq); break;
	case FUNCTION_IRQ4: m_bus->irq4_w(m_irq); break;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpc_mcc_device - constructor
//-------------------------------------------------

wangpc_mcc_device::wangpc_mcc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WANGPC_MCC, "Wang PC-PM043", tag, owner, clock, "wangpc_mcc", __FILE__),
	device_wangpcbus_card_interface(mconfig, *this),
	m_sio(*this, Z80SIO2_TAG),
	m_dart(*this, Z80DART_TAG), m_option(0), m_irq(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpc_mcc_device::device_start()
{
	// state saving
	save_item(NAME(m_option));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wangpc_mcc_device::device_reset()
{
	m_option = 0;

	set_irq(CLEAR_LINE);
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

UINT16 wangpc_mcc_device::wangpcbus_iorc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0x00/2:
		case 0x02/2:
		case 0x04/2:
		case 0x06/2:
			if (ACCESSING_BITS_0_7)
			{
				data = 0xff00 | m_sio->cd_ba_r(space, offset >> 1);
			}
			break;

		case 0x08/2:
		case 0x0a/2:
		case 0x0c/2:
		case 0x0e/2:
			if (ACCESSING_BITS_0_7)
			{
				data = 0xff00 | m_dart->cd_ba_r(space, offset >> 1);
			}
			break;

		case 0x10/2:
			// board status
			/*

			    bit     description

			    0       SIO channel A WAIT/RDY
			    1       SIO channel B WAIT/RDY
			    2       SIO channel A DSR
			    3       SIO channel B DSR
			    4       SIO channel A RI
			    5       SIO channel B RI
			    6       DART channel A WAIT/RDY
			    7       DART channel A DSR
			    8       DART channel B WAIT/RDY
			    9       0 (1 for PC-PM042)
			    10      0 (1 for PC-PM042)
			    11      0 (1 for PC-PM042)
			    12      1
			    13      1
			    14      1
			    15      1

			*/
			data = 0xf000;
			break;

		case 0xfe/2:
			data = 0xff00 | (m_irq << 7) | OPTION_ID;
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_aiowc_w - I/O write
//-------------------------------------------------

void wangpc_mcc_device::wangpcbus_aiowc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
{
	if (sad(offset) && ACCESSING_BITS_0_7)
	{
		switch (offset & 0x7f)
		{
		case 0x00/2:
		case 0x02/2:
		case 0x04/2:
		case 0x06/2:
			m_sio->cd_ba_w(space, offset >> 1, data & 0xff);
			break;

		case 0x08/2:
		case 0x0a/2:
		case 0x0c/2:
		case 0x0e/2:
			m_dart->cd_ba_w(space, offset >> 1, data & 0xff);
			break;

		case 0x12/2:
			// port 1 baud rate
			break;

		case 0x14/2:
			// port 2 baud rate
			break;

		case 0x16/2:
			{
				// ports 1 and 2 function
				bool irq = (m_irq == ASSERT_LINE);
				bool changed = (FUNCTION_IRQ_MASK != (data & 0xc0));

				if (irq && changed) set_irq(CLEAR_LINE);

				m_option = data & 0xff;

				if (irq && changed) set_irq(ASSERT_LINE);
			}
			break;

		case 0x18/2:
			// port 3 channel A baud rate
			break;

		case 0x1a/2:
			// port 3 channel B baud rate
			break;

		case 0xfc/2:
			device_reset();
			break;
		}
	}
}
