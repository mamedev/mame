// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

	Namco C148 Interrupt Controller

	TODO: 
	- vblank is likely to be sent by mast
	
***************************************************************************/
/*
Interrupt Controller C148          1C0000-1FFFFF  R/W  D00-D02
    ????????                       1C0XXX
    ????????                       1C2XXX
    ????????                       1C4XXX * bit 1: operation mode?
    Master/Slave IRQ level         1C6XXX              D00-D02
    EXIRQ level                    1C8XXX              D00-D02
    POSIRQ level                   1CAXXX              D00-D02
    SCIRQ level                    1CCXXX              D00-D02
    VBLANK IRQ level               1CEXXX              D00-D02
    ????????                       1D0XXX
    ????????                       1D4000 trigger master/slave INT?

    Acknowlegde Master/Slave IRQ   1D6XXX ack master/slave INT
    Acknowledge EXIRQ              1D8XXX
    Acknowledge POSIRQ             1DAXXX
    Acknowledge SCIRQ              1DCXXX
    Acknowledge VBLANK IRQ         1DEXXX

    EEPROM Ready status            1E0XXX         R    D01
    Sound CPU Reset control        1E2XXX           W  D01
    Slave 68000 & IO CPU Reset     1E4XXX           W  D01
    Watchdog reset kicker          1E6XXX           W
 */

#include "emu.h"
#include "namco_c148.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type NAMCO_C148 = &device_creator<namco_c148_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  namco_c148_device - constructor
//-------------------------------------------------

namco_c148_device::namco_c148_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCO_C148, "Namco C148 Interrupt Controller", tag, owner, clock, "namco_c148", __FILE__),
	m_hostcpu_tag(nullptr)
{
}

// (*) denotes master CPU only
DEVICE_ADDRESS_MAP_START( map, 16, namco_c148_device )
	AM_RANGE(0x06000, 0x07fff) AM_READWRITE8(cpu_irq_level_r,cpu_irq_level_w,0x00ff) // CPUIRQ lv
	AM_RANGE(0x08000, 0x09fff) AM_READWRITE8(ex_irq_level_r,ex_irq_level_w,0x00ff) // EXIRQ lv
	AM_RANGE(0x0a000, 0x0bfff) AM_READWRITE8(pos_irq_level_r,pos_irq_level_w,0x00ff) // POSIRQ lv
	AM_RANGE(0x0c000, 0x0dfff) AM_READWRITE8(sci_irq_level_r,sci_irq_level_w,0x00ff) // SCIRQ lv
	AM_RANGE(0x0e000, 0x0ffff) AM_READWRITE8(vblank_irq_level_r,vblank_irq_level_w,0x00ff) // VBlank IRQ lv

	AM_RANGE(0x16000, 0x17fff) AM_READWRITE(cpu_irq_ack_r, cpu_irq_ack_w) // CPUIRQ ack
	AM_RANGE(0x18000, 0x19fff) AM_READWRITE(ex_irq_ack_r, ex_irq_ack_w) // EXIRQ ack
	AM_RANGE(0x1a000, 0x1bfff) AM_READWRITE(pos_irq_ack_r, pos_irq_ack_w) // POSIRQ ack
	AM_RANGE(0x1c000, 0x1dfff) AM_READWRITE(sci_irq_ack_r, sci_irq_ack_w) // SCIRQ ack
	AM_RANGE(0x1e000, 0x1ffff) AM_READWRITE(vblank_irq_ack_r, vblank_irq_ack_w) // VBlank IRQ ack
//	AM_RANGE(0x20000, 0x21fff) // EEPROM ready status (*)
	AM_RANGE(0x22000, 0x23fff) AM_WRITE8(ext2_w,0x00ff) // sound CPU reset (*)
//	AM_RANGE(0x24000, 0x25fff) // slave & i/o reset (*)
	AM_RANGE(0x26000, 0x27fff) AM_NOP // watchdog
ADDRESS_MAP_END



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_c148_device::device_start()
{
	m_hostcpu = machine().device<cpu_device>(m_hostcpu_tag);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namco_c148_device::device_reset()
{
	m_irqlevel.vblank = 0;
	m_irqlevel.pos = 0;
}

//**************************************************************************
//  IRQ section
//**************************************************************************

READ8_MEMBER( namco_c148_device::pos_irq_level_r ) 	  	{ return m_irqlevel.pos & 7; }
READ8_MEMBER( namco_c148_device::vblank_irq_level_r ) 	{ return m_irqlevel.vblank & 7; }
READ8_MEMBER( namco_c148_device::cpu_irq_level_r )    	{ return m_irqlevel.cpu & 7; }
READ8_MEMBER( namco_c148_device::ex_irq_level_r ) 	  	{ return m_irqlevel.ex & 7; }
READ8_MEMBER( namco_c148_device::sci_irq_level_r ) 		{ return m_irqlevel.sci & 7; }

inline void namco_c148_device::flush_irq_acks()
{
	// If writing an IRQ priority register, clear any pending IRQs.
	// Dirt Fox and Winning Run require this behaviour
	// TODO: literal behaviour, Winning Run GPU doesn't seem to care about irq ack ports?
	m_hostcpu->set_input_line(m_irqlevel.pos, CLEAR_LINE);
	m_hostcpu->set_input_line(m_irqlevel.vblank, CLEAR_LINE);
	m_hostcpu->set_input_line(m_irqlevel.cpu, CLEAR_LINE);
	m_hostcpu->set_input_line(m_irqlevel.ex, CLEAR_LINE);
	m_hostcpu->set_input_line(m_irqlevel.sci, CLEAR_LINE);
}

WRITE8_MEMBER( namco_c148_device::pos_irq_level_w ) 	{ m_irqlevel.pos = data & 7; 	flush_irq_acks(); 	}
WRITE8_MEMBER( namco_c148_device::vblank_irq_level_w ) 	{ m_irqlevel.vblank = data & 7; flush_irq_acks(); 	}
WRITE8_MEMBER( namco_c148_device::cpu_irq_level_w )		{ m_irqlevel.cpu = data & 7;	flush_irq_acks(); 	}
WRITE8_MEMBER( namco_c148_device::ex_irq_level_w )		{ m_irqlevel.ex = data & 7;		flush_irq_acks(); 	}
WRITE8_MEMBER( namco_c148_device::sci_irq_level_w )		{ m_irqlevel.sci = data & 7;	flush_irq_acks(); 	}

READ16_MEMBER( namco_c148_device::vblank_irq_ack_r ) 	{ m_hostcpu->set_input_line(m_irqlevel.vblank, CLEAR_LINE); return 0; }
READ16_MEMBER( namco_c148_device::pos_irq_ack_r ) 		{ m_hostcpu->set_input_line(m_irqlevel.pos, CLEAR_LINE); 	return 0; }
READ16_MEMBER( namco_c148_device::cpu_irq_ack_r ) 		{ m_hostcpu->set_input_line(m_irqlevel.cpu, CLEAR_LINE); 	return 0; }
READ16_MEMBER( namco_c148_device::ex_irq_ack_r ) 		{ m_hostcpu->set_input_line(m_irqlevel.ex, CLEAR_LINE); 	return 0; }
READ16_MEMBER( namco_c148_device::sci_irq_ack_r ) 		{ m_hostcpu->set_input_line(m_irqlevel.sci, CLEAR_LINE); 	return 0; }

WRITE16_MEMBER( namco_c148_device::vblank_irq_ack_w ) 	{ m_hostcpu->set_input_line(m_irqlevel.vblank, CLEAR_LINE); }
WRITE16_MEMBER( namco_c148_device::pos_irq_ack_w ) 		{ m_hostcpu->set_input_line(m_irqlevel.pos, CLEAR_LINE); }
WRITE16_MEMBER( namco_c148_device::cpu_irq_ack_w ) 		{ m_hostcpu->set_input_line(m_irqlevel.cpu, CLEAR_LINE); }
WRITE16_MEMBER( namco_c148_device::ex_irq_ack_w ) 		{ m_hostcpu->set_input_line(m_irqlevel.ex, CLEAR_LINE); }
WRITE16_MEMBER( namco_c148_device::sci_irq_ack_w ) 		{ m_hostcpu->set_input_line(m_irqlevel.sci, CLEAR_LINE); }

//**************************************************************************
//  IRQ section
//**************************************************************************

WRITE8_MEMBER( namco_c148_device::ext2_w )
{
	// TODO: bit 1 might be irq enable?
}

READ8_MEMBER( namco_c148_device::ext_posirq_line_r )
{
	return m_posirq_line;
}

WRITE8_MEMBER( namco_c148_device::ext_posirq_line_w )
{
	m_posirq_line = data;
}

//**************************************************************************
//  GETTERS/SETTERS
//**************************************************************************

void namco_c148_device::vblank_irq_trigger()
{
	m_hostcpu->set_input_line(m_irqlevel.vblank, ASSERT_LINE);
}

void namco_c148_device::pos_irq_trigger()
{
	m_hostcpu->set_input_line(m_irqlevel.pos, ASSERT_LINE);
}

void namco_c148_device::cpu_irq_trigger()
{
	m_hostcpu->set_input_line(m_irqlevel.cpu, ASSERT_LINE);
}

void namco_c148_device::ex_irq_trigger()
{
	m_hostcpu->set_input_line(m_irqlevel.ex, ASSERT_LINE);
}

void namco_c148_device::sci_irq_trigger()
{
	m_hostcpu->set_input_line(m_irqlevel.sci, ASSERT_LINE);
}

uint8_t namco_c148_device::get_posirq_line()
{
	return m_posirq_line;
}
