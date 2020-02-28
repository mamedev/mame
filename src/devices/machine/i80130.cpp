// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 80130 iRMX Operating System Processor emulation

**********************************************************************/

#include "emu.h"
#include "i80130.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(I80130, i80130_device, "i80130", "I80130")


void i80130_device::rom_map(address_map &map)
{
	//map(0x0000, 0x3fff).rom().region("rom", 0);
}

void i80130_device::io_map(address_map &map)
{
	map(0x00, 0x0f).rw(FUNC(i80130_device::io_r), FUNC(i80130_device::io_w));
	//map(0x00, 0x01).mirror(0x2).rw("pic", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	//map(0x08, 0x0f).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
}

READ16_MEMBER( i80130_device::io_r )
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0: case 1:
		if (ACCESSING_BITS_0_7)
		{
			data = m_pic->read(offset & 0x01);
		}
		break;

	case 4: case 5: case 6: case 7:
		if (ACCESSING_BITS_0_7)
		{
			data = m_pit->read(offset & 0x03);
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
			m_pic->write(offset & 0x01, data & 0xff);
		}
		break;

	case 4: case 5: case 6: case 7:
		if (ACCESSING_BITS_0_7)
		{
			m_pit->write(offset & 0x03, data & 0xff);
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

const tiny_rom_entry *i80130_device::device_rom_region() const
{
	return ROM_NAME( i80130 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void i80130_device::device_add_mconfig(machine_config &config)
{
	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set(FUNC(i80130_device::irq_w));

	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(0);
	m_pit->out_handler<0>().set(FUNC(i80130_device::systick_w));
	m_pit->set_clk<1>(0);
	m_pit->out_handler<1>().set(FUNC(i80130_device::delay_w));
	m_pit->set_clk<2>(0);
	m_pit->out_handler<2>().set(FUNC(i80130_device::baud_w));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i80130_device - constructor
//-------------------------------------------------

i80130_device::i80130_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, I80130, tag, owner, clock),
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
