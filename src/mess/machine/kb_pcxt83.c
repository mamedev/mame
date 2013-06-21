/**********************************************************************

    IBM Model F PC/XT 5150/5160 83-key keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "kb_pcxt83.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "i8048"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PC_KBD_IBM_PC_XT_83 = &device_creator<ibm_pc_xt_83_keyboard_device>;


//-------------------------------------------------
//  ROM( ibm_pc_xt_83_keyboard )
//-------------------------------------------------

ROM_START( ibm_pc_xt_83_keyboard )
	ROM_REGION( 0x400, I8048_TAG, 0 )
	/*
	Keyboard Part No. 1501105

	MOI 74 01
	PN 4584751
	GX 344231

	i 4429745
	ZO P 379297
	8143 P
	(C) INTEL 76
	*/
	ROM_LOAD( "4584751.bin", 0x000, 0x400, CRC(c59aa9d1) SHA1(4f5b2a075c68f6493310ec1e2a24271ceea330df) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *ibm_pc_xt_83_keyboard_device::device_rom_region() const
{
	return ROM_NAME( ibm_pc_xt_83_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( kb_io )
//-------------------------------------------------

static ADDRESS_MAP_START( ibm_pc_xt_83_keyboard_io, AS_IO, 8, ibm_pc_xt_83_keyboard_device )
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_WRITE(bus_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READ(p1_r) AM_WRITENOP
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(p2_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(t1_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( ibm_pc_xt_83_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( ibm_pc_xt_83_keyboard )
	MCFG_CPU_ADD(I8048_TAG, I8048, MCS48_LC_CLOCK(IND_U(47), CAP_P(20)))
	MCFG_CPU_IO_MAP(ibm_pc_xt_83_keyboard_io)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor ibm_pc_xt_83_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ibm_pc_xt_83_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( ibm_pc_xt_83_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( ibm_pc_xt_83_keyboard )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ibm_pc_xt_83_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ibm_pc_xt_83_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ibm_pc_xt_83_keyboard_device - constructor
//-------------------------------------------------

ibm_pc_xt_83_keyboard_device::ibm_pc_xt_83_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC_KBD_IBM_PC_XT_83, "IBM PC/XT 5150/5160 Keyboard", tag, owner, clock, "kb_pcxt83", __FILE__),
	  device_pc_kbd_interface(mconfig, *this),
	  m_maincpu(*this, I8048_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ibm_pc_xt_83_keyboard_device::device_start()
{
	// state saving
	save_item(NAME(m_cnt));
	save_item(NAME(m_key_depressed));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ibm_pc_xt_83_keyboard_device::device_reset()
{
	m_maincpu->reset();
}


//-------------------------------------------------
//  bus_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_xt_83_keyboard_device::bus_w )
{
	m_cnt = data;
}


//-------------------------------------------------
//  p1_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_xt_83_keyboard_device::p1_r )
{
	/*
	
	    bit     description
	
	    0       -REQ IN
	    1       DATA IN
	    2       
	    3       
	    4       
	    5       
	    6       
	    7       
	
	*/
	
	UINT8 data = 0;

	data |= clock_signal();
	data |= data_signal() << 1;

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_xt_83_keyboard_device::p2_w )
{
	/*
	
	    bit     description
	
	    0       -MATRIX STROBE
	    1       CLOCK OUT
	    2       DATA OUT
	    3       
	    4       
	    5       
	    6       
	    7       
	
	*/

	m_pc_kbdc->clock_write_from_kb(BIT(data, 1));
	m_pc_kbdc->data_write_from_kb(BIT(data, 2));
}


//-------------------------------------------------
//  t1_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_xt_83_keyboard_device::t1_r )
{
	return m_key_depressed;
}
