// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 80130 iRMX Operating System Processor emulation

**********************************************************************/

#include "i80130.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type I80130 = &device_creator<i80130_device>;


DEVICE_ADDRESS_MAP_START( rom_map, 16, i80130_device )
	//AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("rom", 0)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( io_map, 16, i80130_device )
	AM_RANGE(0x00, 0x0f) AM_READWRITE(io_r, io_w)
	//AM_RANGE(0x00, 0x01) AM_MIRROR(0x2) AM_DEVREADWRITE8("pic", pic8259_device, read, write, 0x00ff)
	//AM_RANGE(0x08, 0x0f) AM_DEVREADWRITE8("pit", pit8254_device, read, write, 0x00ff)
ADDRESS_MAP_END

READ16_MEMBER( i80130_device::io_r )
{
	UINT16 data = 0;

	switch (offset)
	{
	case 0: case 1:
		if (ACCESSING_BITS_0_7)
		{
			data = m_pic->read(space, offset & 0x01);
		}
		break;

	case 4: case 5: case 6: case 7:
		if (ACCESSING_BITS_0_7)
		{
			data = m_pit->read(space, offset & 0x03);
		}
		break;
	}

	return data;
}

WRITE16_MEMBER( i80130_device::io_w )
{
	switch (offset)
	{
	case 0: case 1:
		if (ACCESSING_BITS_0_7)
		{
			m_pic->write(space, offset & 0x01, data & 0xff);
		}
		break;

	case 4: case 5: case 6: case 7:
		if (ACCESSING_BITS_0_7)
		{
			m_pit->write(space, offset & 0x03, data & 0xff);
		}
		break;
	}
}


//-------------------------------------------------
//  ROM( i80130 )
//-------------------------------------------------

ROM_START( i80130 )
	ROM_REGION16_LE( 0x4000, "rom", 0 )
	ROM_LOAD( "80130", 0x0000, 0x4000, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *i80130_device::device_rom_region() const
{
	return ROM_NAME( i80130 );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( i80130 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( i80130 )
	MCFG_PIC8259_ADD("pic", DEVWRITELINE(DEVICE_SELF, i80130_device, irq_w), VCC, NULL)

	MCFG_DEVICE_ADD("pit", PIT8254, 0)
	MCFG_PIT8253_CLK0(0)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(i80130_device, systick_w))
	MCFG_PIT8253_CLK1(0)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(i80130_device, delay_w))
	MCFG_PIT8253_CLK2(0)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(i80130_device, baud_w))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor i80130_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( i80130 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i80130_device - constructor
//-------------------------------------------------

i80130_device::i80130_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, I80130, "I80130", tag, owner, clock, "i80130", __FILE__),
		m_pic(*this, "pic"),
		m_pit(*this, "pit"),
		m_write_irq(*this),
		m_write_ack(*this),
		m_write_lir(*this),
		m_write_systick(*this),
		m_write_delay(*this),
		m_write_baud(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i80130_device::device_start()
{
	// resolve callbacks
	m_write_irq.resolve_safe();
	m_write_ack.resolve_safe();
	m_write_lir.resolve_safe();
	m_write_systick.resolve_safe();
	m_write_delay.resolve_safe();
	m_write_baud.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i80130_device::device_reset()
{
	// set PIT clocks
	m_pit->set_clockin(0, clock());
	m_pit->set_clockin(1, clock());
	m_pit->set_clockin(2, clock());
}
