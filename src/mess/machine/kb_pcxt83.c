/**********************************************************************

    IBM PC/XT 5150/5160 83-key keyboard emulation

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
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( ibm_pc_xt_83_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( ibm_pc_xt_83_keyboard )
	MCFG_CPU_ADD(I8048_TAG, I8048, 4000000)
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
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ibm_pc_xt_83_keyboard_device::device_reset()
{
}


//-------------------------------------------------
//  clock_write -
//-------------------------------------------------

WRITE_LINE_MEMBER( ibm_pc_xt_83_keyboard_device::clock_write )
{
}


//-------------------------------------------------
//  data_write -
//-------------------------------------------------

WRITE_LINE_MEMBER( ibm_pc_xt_83_keyboard_device::data_write )
{
}
