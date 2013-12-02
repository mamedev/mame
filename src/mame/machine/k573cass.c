// license:MAME
// copyright-holders:smf
/*
 * Konami 573 Security Cassette
 *
 */

#include "k573cass.h"

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

READ_LINE_MEMBER(konami573_cassette_interface::read_line_dsr)
{
	return 1;
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


const device_type KONAMI573_CASSETTE_X = &device_creator<konami573_cassette_x_device>;

konami573_cassette_x_device::konami573_cassette_x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, KONAMI573_CASSETTE_X, "KONAMI 573 CASSETTE X", tag, owner, clock, "k573cassx", __FILE__),
	device_slot_card_interface(mconfig, *this),
	m_x76f041(*this, "eeprom")
{
}

konami573_cassette_x_device::konami573_cassette_x_device(const machine_config &mconfig, const device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
	device_slot_card_interface(mconfig, *this),
	m_x76f041(*this, "eeprom")
{
}

static MACHINE_CONFIG_FRAGMENT( cassx )
	MCFG_X76F041_ADD( "eeprom" )
MACHINE_CONFIG_END

machine_config_constructor konami573_cassette_x_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cassx );
}

void konami573_cassette_x_device::device_start()
{
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


const device_type KONAMI573_CASSETTE_XI = &device_creator<konami573_cassette_xi_device>;

konami573_cassette_xi_device::konami573_cassette_xi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	konami573_cassette_x_device(mconfig, KONAMI573_CASSETTE_XI, "KONAMI 573 CASSETTE XI", tag, owner, clock, "k573cassxi", __FILE__),
	m_ds2401(*this, "id"),
	m_adc0838(*this, "adc0838")
{
}

static MACHINE_CONFIG_FRAGMENT( cassxi )
	MCFG_X76F041_ADD( "eeprom" )
	MCFG_DS2401_ADD( "id" )
	MCFG_DEVICE_ADD( "adc0838", ADC0838, 0 )
MACHINE_CONFIG_END

machine_config_constructor konami573_cassette_xi_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cassxi );
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


const device_type KONAMI573_CASSETTE_Y = &device_creator<konami573_cassette_y_device>;

konami573_cassette_y_device::konami573_cassette_y_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, KONAMI573_CASSETTE_Y, "KONAMI 573 CASSETTE Y", tag, owner, clock, "k573cassy", __FILE__),
	device_slot_card_interface(mconfig, *this),
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

konami573_cassette_y_device::konami573_cassette_y_device(const machine_config &mconfig, const device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, __FILE__),
	device_slot_card_interface(mconfig, *this),
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

static MACHINE_CONFIG_FRAGMENT( cassy )
	MCFG_X76F100_ADD( "eeprom" )
MACHINE_CONFIG_END

machine_config_constructor konami573_cassette_y_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cassy );
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


const device_type KONAMI573_CASSETTE_YI = &device_creator<konami573_cassette_yi_device>;

konami573_cassette_yi_device::konami573_cassette_yi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	konami573_cassette_y_device(mconfig, KONAMI573_CASSETTE_YI, "KONAMI 573 CASSETTE YI", tag, owner, clock, "k573cassyi", __FILE__),
	m_ds2401(*this, "id")
{
}

static MACHINE_CONFIG_FRAGMENT( cassyi )
	MCFG_X76F100_ADD( "eeprom" )
	MCFG_DS2401_ADD( "id" )
MACHINE_CONFIG_END

machine_config_constructor konami573_cassette_yi_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cassyi );
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


const device_type KONAMI573_CASSETTE_ZI = &device_creator<konami573_cassette_zi_device>;

konami573_cassette_zi_device::konami573_cassette_zi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, KONAMI573_CASSETTE_ZI, "KONAMI 573 CASSETTE ZI", tag, owner, clock, "k573casszi", __FILE__),
	device_slot_card_interface(mconfig, *this),
	m_zs01(*this,"eeprom"),
	m_ds2401(*this, "id")
{
}

static MACHINE_CONFIG_FRAGMENT( casszi )
	MCFG_DS2401_ADD( "id" )
	MCFG_ZS01_ADD( "eeprom" )
	MCFG_ZS01_DS2401( "id" )
MACHINE_CONFIG_END

machine_config_constructor konami573_cassette_zi_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( casszi );
}

void konami573_cassette_zi_device::device_start()
{
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


const device_type KONAMI573_CASSETTE_SLOT_SERIAL = &device_creator<konami573_cassette_slot_serial_device>;

konami573_cassette_slot_serial_device::konami573_cassette_slot_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	psxsiodev_device(mconfig, KONAMI573_CASSETTE_SLOT, "KONAMI 573 CASSETTE SLOT (SERIAL)", tag, owner, clock, "k573cassslotserial", __FILE__)
{
}

void konami573_cassette_slot_serial_device::device_start()
{
	psxsiodev_device::device_start();
}

void konami573_cassette_slot_serial_device::_data_out( int data, int mask )
{
	data_out( data, mask );
}

void konami573_cassette_slot_serial_device::data_in( int data, int mask )
{
}


const device_type KONAMI573_CASSETTE_SLOT = &device_creator<konami573_cassette_slot_device>;

konami573_cassette_slot_device::konami573_cassette_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, KONAMI573_CASSETTE_SLOT, "KONAMI 573 CASSETTE SLOT", tag, owner, clock, "k573cassslot", __FILE__),
	device_slot_interface(mconfig, *this),
	m_serial(*this, ":maincpu:sio1:cassette") /// HACK: this shouldn't know where the actual slot is.
{
}

void konami573_cassette_slot_device::device_start()
{
	m_cassette = dynamic_cast<konami573_cassette_interface *>(get_card_device());

	if( m_cassette != NULL && m_cassette->read_line_dsr() )
	{
		m_serial->_data_out( PSX_SIO_IN_DSR, PSX_SIO_IN_DSR );
	}
	else
	{
		m_serial->_data_out( PSX_SIO_IN_DSR, 0 );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d0)
{
	if( m_cassette )
	{
		m_cassette->write_line_d0( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d1)
{
	if( m_cassette )
	{
		m_cassette->write_line_d1( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d2)
{
	if( m_cassette )
	{
		m_cassette->write_line_d2( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d3)
{
	if( m_cassette )
	{
		m_cassette->write_line_d3( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d4)
{
	if( m_cassette )
	{
		m_cassette->write_line_d4( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d5)
{
	if( m_cassette )
	{
		m_cassette->write_line_d5( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d6)
{
	if( m_cassette )
	{
		m_cassette->write_line_d6( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_d7)
{
	if( m_cassette )
	{
		m_cassette->write_line_d7( state );
	}
}

WRITE_LINE_MEMBER(konami573_cassette_slot_device::write_line_zs01_sda)
{
	if( m_cassette )
	{
		m_cassette->write_line_zs01_sda( state );
	}
}

READ_LINE_MEMBER(konami573_cassette_slot_device::read_line_ds2401)
{
	if( m_cassette )
	{
		return m_cassette->read_line_ds2401();
	}

	return 0;
}

READ_LINE_MEMBER(konami573_cassette_slot_device::read_line_secflash_sda)
{
	if( m_cassette )
	{
		return m_cassette->read_line_secflash_sda();
	}

	return 0;
}

READ_LINE_MEMBER(konami573_cassette_slot_device::read_line_adc083x_do)
{
	if( m_cassette )
	{
		return m_cassette->read_line_adc083x_do();
	}

	return 0;
}

READ_LINE_MEMBER(konami573_cassette_slot_device::read_line_adc083x_sars)
{
	if( m_cassette )
	{
		return m_cassette->read_line_adc083x_sars();
	}

	return 0;
}
