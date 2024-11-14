// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Philips PCD8544 LCD controller

***************************************************************************/

#include "emu.h"
#include "pcd8544.h"

#include "screen.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PCD8544, pcd8544_device, "pcd8544_device", "Philips PCD8544 LCD Controller")


//**************************************************************************
//  live device
//**************************************************************************

//-------------------------------------------------
//  pcd8544_device - constructor
//-------------------------------------------------

pcd8544_device::pcd8544_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PCD8544, tag, owner, clock),
	m_screen_update_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pcd8544_device::device_start()
{
	m_screen_update_cb.resolve();

	// state saving
	save_item(NAME(m_sdin));
	save_item(NAME(m_sclk));
	save_item(NAME(m_dc));
	save_item(NAME(m_bits));
	save_item(NAME(m_mode));
	save_item(NAME(m_control));
	save_item(NAME(m_op_vol));
	save_item(NAME(m_bias));
	save_item(NAME(m_temp_coef));
	save_item(NAME(m_indata));
	save_item(NAME(m_addr_y));
	save_item(NAME(m_addr_x));
	save_item(NAME(m_vram));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pcd8544_device::device_reset()
{
	m_mode      = 0x04; // PD=1, V=0, H=0
	m_control   = 0x00; // E=0, D=0
	m_addr_y    = 0;
	m_addr_x    = 0;
	m_bias      = 0;
	m_temp_coef = 0;
	m_op_vol    = 0;
	m_bits      = 0;
	m_indata    = 0;
	m_sdin      = 0;
	m_sclk      = 0;
	m_dc        = 0;
}

void pcd8544_device::exec_command(uint8_t cmd)
{
	if (m_mode & 0x01)
	{
		if(cmd & 0x80)
		{
			m_op_vol = cmd & 0x7f;
			LOG("PCD8544: set Vop %d\n", m_op_vol);
		}
		else if(cmd & 0x40)
		{
			logerror("PCD8544: unused command H=1 0x%02x\n", cmd);
		}
		else if(cmd & 0x20)
		{
			m_mode = cmd & 0x07;
			LOG("PCD8544: set PD=%d V=%d H=%d\n", BIT(m_mode, 2), BIT(m_mode, 1), BIT(m_mode, 0));
		}
		else if(cmd & 0x10)
		{
			m_bias = cmd & 0x07;
			LOG("PCD8544: set bias system %d\n", m_bias);
		}
		else if(cmd & 0x08)
		{
			logerror("PCD8544: unused command H=1 0x%02x\n", cmd);
		}
		else if(cmd & 0x04)
		{
			m_temp_coef = cmd & 0x03;
			LOG("PCD8544: set temperature coefficient %d\n", m_temp_coef);
		}
		else if (cmd)
		{
			logerror("PCD8544: unused command H=1 0x%02x\n", cmd);
		}
	}
	else
	{
		if(cmd & 0x80)
		{
			m_addr_x = (cmd & 0x7f) % 84;
			LOG("PCD8544: set X-address %d\n", cmd & 0x7f);
		}
		else if(cmd & 0x40)
		{
			m_addr_y = (cmd & 0x07) % 6;
			LOG("PCD8544: set Y-address %d\n", cmd & 0x07);
		}
		else if(cmd & 0x20)
		{
			m_mode = cmd & 0x07;
			LOG("PCD8544: set PD=%d V=%d H=%d\n", BIT(m_mode, 2), BIT(m_mode, 1), BIT(m_mode, 0));
		}
		else if(cmd & 0x10)
		{
			logerror("PCD8544: unused command H=0 0x%02x\n", cmd);
		}
		else if(cmd & 0x08)
		{
			m_control = ((cmd & 0x04) >> 1) | (cmd & 0x01);
			LOG("PCD8544: set D=%d E=%d\n", BIT(m_control, 1), BIT(m_control, 0));
		}
		else if (cmd)
		{
			logerror("PCD8544: unused command H=0 0x%02x\n", cmd);
		}
	}
}

void pcd8544_device::write_data(uint8_t data)
{
	m_vram[m_addr_y * 84 + m_addr_x] = data;

	if (m_mode & 0x02)
	{
		m_addr_y++;

		if (m_addr_y > 5)
		{
			m_addr_y = 0;
			m_addr_x = (m_addr_x + 1) % 84;
		}
	}
	else
	{
		m_addr_x++;
		if (m_addr_x > 83)
		{
			m_addr_x = 0;
			m_addr_y = (m_addr_y + 1) % 6;
		}
	}
}

void pcd8544_device::sdin_w(int state)
{
	m_sdin = state;
}

void pcd8544_device::sclk_w(int state)
{
	if (!m_sclk && state)
	{
		m_indata = (m_indata << 1) | (m_sdin ? 1 : 0);
		m_bits++;
		if (m_bits == 8)
		{
			if (m_dc)
				write_data(m_indata);

			else
				exec_command(m_indata);

			m_bits = 0;
			m_indata = 0;
		}
	}
	m_sclk = state;
}

void pcd8544_device::dc_w(int state)
{
	m_dc = state;
}

uint32_t pcd8544_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if ((m_mode & 0x04) == 0)
	{
		switch (m_control)
		{
			case 0:     // display blank
			case 1:     // all display segments on
				bitmap.fill(m_control & 1, cliprect);
				break;

			case 2:     // normal mode
			case 3:     // inverse video mode
				if (!m_screen_update_cb.isnull())
					m_screen_update_cb(screen, bitmap, cliprect, m_vram, m_control & 1);
				break;
		}
	}
	else
		bitmap.fill(0, cliprect);

	return 0;
}
