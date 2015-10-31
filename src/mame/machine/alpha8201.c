// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Alpha Denshi ALPHA-8201 family protection emulation

----------------------------------------------------------------------------

abc


TODO:
- x

***************************************************************************/

#include "cpu/hmcs40/hmcs40.h"
#include "alpha8201.h"

/**************************************************************************/

const device_type ALPHA_8201 = &device_creator<alpha_8201_device>;

//-------------------------------------------------
//  alpha_8201_device - constructor
//-------------------------------------------------

alpha_8201_device::alpha_8201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ALPHA_8201, "ALPHA-8201", tag, owner, clock, "alpha8201", __FILE__),
	m_mcu(*this, "mcu")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void alpha_8201_device::device_start()
{
	m_shared_ram = auto_alloc_array_clear(machine(), UINT8, 0x400);
	
	// zerofill
	m_dir = 0;
	m_mcu_address = 0;
	m_mcu_d = 0;
	memset(m_mcu_r, 0, sizeof(m_mcu_r));

	// register for savestates
	save_pointer(NAME(m_shared_ram), 0x400);
	save_item(NAME(m_dir));
	save_item(NAME(m_mcu_address));
	save_item(NAME(m_mcu_d));
	save_item(NAME(m_mcu_r));
}

// machine config additions
static MACHINE_CONFIG_FRAGMENT(alpha8201)

	MCFG_CPU_ADD("mcu", HD44801, DERIVED_CLOCK(1,1)) // 8H
	MCFG_HMCS40_READ_R_CB(0, READ8(alpha_8201_device, mcu_data_r))
	MCFG_HMCS40_READ_R_CB(1, READ8(alpha_8201_device, mcu_data_r))
	MCFG_HMCS40_WRITE_R_CB(0, WRITE8(alpha_8201_device, mcu_data_w))
	MCFG_HMCS40_WRITE_R_CB(1, WRITE8(alpha_8201_device, mcu_data_w))
	MCFG_HMCS40_WRITE_R_CB(2, WRITE8(alpha_8201_device, mcu_data_w))
	MCFG_HMCS40_WRITE_R_CB(3, WRITE8(alpha_8201_device, mcu_data_w))
	MCFG_HMCS40_WRITE_D_CB(WRITE16(alpha_8201_device, mcu_d_w))
MACHINE_CONFIG_END

machine_config_constructor alpha_8201_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(alpha8201);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void alpha_8201_device::device_reset()
{
	m_dir = 0;
	m_mcu->set_input_line(0, CLEAR_LINE);
}



/***************************************************************************

  Internal I/O

***************************************************************************/

void alpha_8201_device::mcu_writeram()
{
	// RAM WR is level-triggered
	if (m_dir && (m_mcu_d & 0xc) == 0xc)
		m_shared_ram[m_mcu_address] = m_mcu_r[0] << 4 | m_mcu_r[1];
}

void alpha_8201_device::mcu_update_address()
{
	m_mcu_address = (m_mcu_d << 8 & 0x300) | m_mcu_r[2] << 4 | m_mcu_r[3];
	mcu_writeram();
}


READ8_MEMBER(alpha_8201_device::mcu_data_r)
{
	UINT8 ret = 0;
	
	if (m_dir && ~m_mcu_d & 4)
		ret = m_shared_ram[m_mcu_address];
//	else
//		logerror("1\n");
	
	if (offset == HMCS40_PORT_R0X)
		ret >>= 4;
	return ret & 0xf;
}

WRITE8_MEMBER(alpha_8201_device::mcu_data_w)
{
	// R0,R1: RAM data
	// R2,R3: RAM A0-A7
	m_mcu_r[offset] = data & 0xf;
	mcu_update_address();
}

WRITE16_MEMBER(alpha_8201_device::mcu_d_w)
{
	// D0,D1: RAM A8,A9
	// D2: _RD
	// D3: WR
	m_mcu_d = data;
	mcu_update_address();
}



/***************************************************************************

  I/O for External Interface

***************************************************************************/

WRITE_LINE_MEMBER(alpha_8201_device::bus_dir_w)
{
	// set bus direction to 0: external, 1: MCU side
	m_dir = (state) ? 1 : 0;
	mcu_writeram();
}

WRITE_LINE_MEMBER(alpha_8201_device::mcu_start_w)
{
	// connected to MCU INT0
	m_mcu->set_input_line(0, (state) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(alpha_8201_device::main_ram_r)
{
	return m_shared_ram[offset & 0x3ff];
}

WRITE8_MEMBER(alpha_8201_device::main_ram_w)
{
	if (!m_dir)
		m_shared_ram[offset & 0x3ff] = data;
}
