// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Konami 573 Security Cassette
 *
 */

#include "emu.h"
#include "k573cass.h"

// class konami573_cassette_interface

konami573_cassette_interface::konami573_cassette_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<konami573_cassette_slot_device *>(device.owner());
}

konami573_cassette_interface::~konami573_cassette_interface()
{
}



WRITE_LINE_MEMBER(konami573_cassette_interface::write_line_d0)
{
}

WRITE_LINE_MEMBER(konami573_cassette_interface::write_line_d4)
{
}

WRITE_LINE_MEMBER(konami573_cassette_interface::write_line_d5)
{
}

WRITE_LINE_MEMBER(konami573_cassette_interface::write_line_d6)
{
}

WRITE_LINE_MEMBER(konami573_cassette_interface::write_line_d7)
{
}

WRITE_LINE_MEMBER(konami573_cassette_interface::write_line_zs01_sda)
{
}

READ_LINE_MEMBER(konami573_cassette_interface::read_line_ds2401)
{
	return 0;
}

READ_LINE_MEMBER(konami573_cassette_interface::read_line_adc083x_do)
{
	return 0;
}

READ_LINE_MEMBER(konami573_cassette_interface::read_line_adc083x_sars)
{
	return 0;
}


DEFINE_DEVICE_TYPE(KONAMI573_CASSETTE_X, konami573_cassette_x_device, "k573cassx", "Konami 573 Cassette X")

konami573_cassette_x_device::konami573_cassette_x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	konami573_cassette_x_device(mconfig, KONAMI573_CASSETTE_X, tag, owner, clock)
{
}

konami573_cassette_x_device::konami573_cassette_x_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	konami573_cassette_interface(mconfig, *this),
	m_x76f041(*this, "eeprom")
{
}

void konami573_cassette_x_device::device_add_mconfig(machine_config &config)
{
	X76F041( config, m_x76f041 );
}

void konami573_cassette_x_device::device_start()
{
	output_dsr(0);
}

WRITE_LINE_MEMBER(konami573_cassette_x_device::write_line_d0)
{
	m_x76f041->write_sda( state );
}

WRITE_LINE_MEMBER(konami573_cassette_x_device::write_line_d1)
{
	m_x76f041->write_scl( state );
}

WRITE_LINE_MEMBER(konami573_cassette_x_device::write_line_d2)
{
	m_x76f041->write_cs( state );
}

WRITE_LINE_MEMBER(konami573_cassette_x_device::write_line_d3)
{
	m_x76f041->write_rst( state );
}

READ_LINE_MEMBER(konami573_cassette_x_device::read_line_secflash_sda)
{
	return m_x76f041->read_sda();
}


DEFINE_DEVICE_TYPE(KONAMI573_CASSETTE_XI, konami573_cassette_xi_device, "k573cassxi", "Konami 573 Cassette XI")

konami573_cassette_xi_device::konami573_cassette_xi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	konami573_cassette_x_device(mconfig, KONAMI573_CASSETTE_XI, tag, owner, clock),
	m_ds2401(*this, "id"),
	m_adc0838(*this, "adc0838")
{
}

void konami573_cassette_xi_device::device_add_mconfig(machine_config &config)
{
	X76F041( config, m_x76f041 );
	DS2401( config, m_ds2401 );
	ADC0838( config, m_adc0838 );
}

WRITE_LINE_MEMBER(konami573_cassette_xi_device::write_line_d0)
{
	konami573_cassette_x_device::write_line_d0( state ); // shares line with x76f041 sda

	m_adc0838->cs_write( state );
}

WRITE_LINE_MEMBER(konami573_cassette_xi_device::write_line_d1)
{
	konami573_cassette_x_device::write_line_d1( state ); // shares line with x76f041 scl

	m_adc0838->clk_write( state );
}

WRITE_LINE_MEMBER(konami573_cassette_xi_device::write_line_d4)
{
	m_ds2401->write( !state );
}

WRITE_LINE_MEMBER(konami573_cassette_xi_device::write_line_d5)
{
	m_adc0838->di_write( state );
}

READ_LINE_MEMBER(konami573_cassette_xi_device::read_line_ds2401)
{
	return m_ds2401->read();
}

READ_LINE_MEMBER(konami573_cassette_xi_device::read_line_adc083x_do)
{
	return m_adc0838->do_read();
}

READ_LINE_MEMBER(konami573_cassette_xi_device::read_line_adc083x_sars)
{
	return m_adc0838->sars_read();
}


DEFINE_DEVICE_TYPE(KONAMI573_CASSETTE_Y, konami573_cassette_y_device, "k573cassy", "Konami 573 Cassette Y")

konami573_cassette_y_device::konami573_cassette_y_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	konami573_cassette_y_device(mconfig, KONAMI573_CASSETTE_Y, tag, owner, clock)
{
}

konami573_cassette_y_device::konami573_cassette_y_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	konami573_cassette_interface(mconfig, *this),
	m_x76f100(*this, "eeprom"),
	m_d0_handler(*this),
	m_d1_handler(*this),
	m_d2_handler(*this),
	m_d3_handler(*this),
	m_d4_handler(*this),
	m_d5_handler(*this),
	m_d6_handler(*this),
	m_d7_handler(*this)
{
}

void konami573_cassette_y_device::device_add_mconfig(machine_config &config)
{
	X76F100( config, m_x76f100 );
}

void konami573_cassette_y_device::device_start()
{
	m_d0_handler.resolve_safe();
	m_d1_handler.resolve_safe();
	m_d2_handler.resolve_safe();
	m_d3_handler.resolve_safe();
	m_d4_handler.resolve_safe();
	m_d5_handler.resolve_safe();
	m_d6_handler.resolve_safe();
	m_d7_handler.resolve_safe();

	output_dsr(0);
}

READ_LINE_MEMBER(konami573_cassette_y_device::read_line_secflash_sda)
{
	return m_x76f100->read_sda();
}

WRITE_LINE_MEMBER(konami573_cassette_y_device::write_line_d0)
{
	m_d0_handler( state );
	m_x76f100->write_sda( state );
}

WRITE_LINE_MEMBER(konami573_cassette_y_device::write_line_d1)
{
	m_d1_handler( state );
	m_x76f100->write_scl( state );
}

WRITE_LINE_MEMBER(konami573_cassette_y_device::write_line_d2)
{
	m_d2_handler( state );
	m_x76f100->write_cs( state );
}

WRITE_LINE_MEMBER(konami573_cassette_y_device::write_line_d3)
{
	m_d3_handler( state );
	m_x76f100->write_rst( state );
}

WRITE_LINE_MEMBER(konami573_cassette_y_device::write_line_d4)
{
	m_d4_handler( state );
}

WRITE_LINE_MEMBER(konami573_cassette_y_device::write_line_d5)
{
	m_d5_handler( state );
}

WRITE_LINE_MEMBER(konami573_cassette_y_device::write_line_d6)
{
	m_d6_handler( state );
}

WRITE_LINE_MEMBER(konami573_cassette_y_device::write_line_d7)
{
	m_d7_handler( state );
}


DEFINE_DEVICE_TYPE(KONAMI573_CASSETTE_YI, konami573_cassette_yi_device, "k573cassyi", "Konami 573 Cassette YI")

konami573_cassette_yi_device::konami573_cassette_yi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	konami573_cassette_y_device(mconfig, KONAMI573_CASSETTE_YI, tag, owner, clock),
	m_ds2401(*this, "id")
{
}

void konami573_cassette_yi_device::device_add_mconfig(machine_config &config)
{
	X76F100( config, m_x76f100 );
	DS2401( config, m_ds2401 );
}

WRITE_LINE_MEMBER(konami573_cassette_yi_device::write_line_d4)
{
	konami573_cassette_y_device::write_line_d4( state );

	m_ds2401->write( !state );
}

READ_LINE_MEMBER(konami573_cassette_yi_device::read_line_ds2401)
{
	return m_ds2401->read();
}


DEFINE_DEVICE_TYPE(KONAMI573_CASSETTE_ZI, konami573_cassette_zi_device, "k573casszi", "Konami 573 Cassette ZI")

konami573_cassette_zi_device::konami573_cassette_zi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, KONAMI573_CASSETTE_ZI, tag, owner, clock),
	konami573_cassette_interface(mconfig, *this),
	m_zs01(*this,"eeprom"),
	m_ds2401(*this, "id")
{
}

void konami573_cassette_zi_device::device_add_mconfig(machine_config &config)
{
	DS2401( config, m_ds2401 );
	ZS01( config, m_zs01 ).set_ds2401_tag( "id" );
}

void konami573_cassette_zi_device::device_start()
{
	output_dsr(0);
}

WRITE_LINE_MEMBER(konami573_cassette_zi_device::write_line_d1)
{
	m_zs01->write_scl( state );
}

WRITE_LINE_MEMBER(konami573_cassette_zi_device::write_line_d2)
{
	m_zs01->write_cs( state );
}

WRITE_LINE_MEMBER(konami573_cassette_zi_device::write_line_d3)
{
	m_zs01->write_rst( state );
}

WRITE_LINE_MEMBER(konami573_cassette_zi_device::write_line_d4)
{
	m_ds2401->write( !state );
}

WRITE_LINE_MEMBER(konami573_cassette_zi_device::write_line_zs01_sda)
{
	m_zs01->write_sda( state );
}

READ_LINE_MEMBER(konami573_cassette_zi_device::read_line_ds2401)
{
	return m_ds2401->read();
}

READ_LINE_MEMBER(konami573_cassette_zi_device::read_line_secflash_sda)
{
	return m_zs01->read_sda();
}


DEFINE_DEVICE_TYPE(KONAMI573_CASSETTE_SLOT, konami573_cassette_slot_device, "k572cassslot", "Konami 573 Cassette Slot")

konami573_cassette_slot_device::konami573_cassette_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, KONAMI573_CASSETTE_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_dsr_handler(*this)
{
}

void konami573_cassette_slot_device::device_start()
{
	m_dsr_handler.resolve_safe();

	m_dev = dynamic_cast<konami573_cassette_interface *>(get_card_device());
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d0)
{
	if( m_dev )
	{
		m_dev->write_line_d0( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d1)
{
	if( m_dev )
	{
		m_dev->write_line_d1( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d2)
{
	if( m_dev )
	{
		m_dev->write_line_d2( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d3)
{
	if( m_dev )
	{
		m_dev->write_line_d3( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d4)
{
	if( m_dev )
	{
		m_dev->write_line_d4( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d5)
{
	if( m_dev )
	{
		m_dev->write_line_d5( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d6)
{
	if( m_dev )
	{
		m_dev->write_line_d6( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d7)
{
	if( m_dev )
	{
		m_dev->write_line_d7( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_zs01_sda)
{
	if( m_dev )
	{
		m_dev->write_line_zs01_sda( state );
	}
}

READ_LINE_MEMBER(konami573_cassette_slot_device::read_line_ds2401)
{
	if( m_dev )
	{
		return m_dev->read_line_ds2401();
	}

	return 0;
}

READ_LINE_MEMBER(konami573_cassette_slot_device::read_line_secflash_sda)
{
	if( m_dev )
	{
		return m_dev->read_line_secflash_sda();
	}

	return 0;
}

READ_LINE_MEMBER(konami573_cassette_slot_device::read_line_adc083x_do)
{
	if( m_dev )
	{
		return m_dev->read_line_adc083x_do();
	}

	return 0;
}

READ_LINE_MEMBER(konami573_cassette_slot_device::read_line_adc083x_sars)
{
	if( m_dev )
	{
		return m_dev->read_line_adc083x_sars();
	}

	return 0;
}
