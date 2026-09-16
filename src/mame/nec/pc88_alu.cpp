// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC-8*01 mkIISR ALU

A rudimentary RMW video device for PC-8801 GVRAM

**************************************************************************************************/

#include "emu.h"
#include "pc88_alu.h"

DEFINE_DEVICE_TYPE(PC88_ALU,   pc88_alu_device,   "pc88_alu",     "NEC PC-8*01 mkIISR ALU")

pc88_alu_device::pc88_alu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC88_ALU, tag, owner, clock)
	, m_gvram_read(*this, 0)
	, m_gvram_write(*this)
{
}

void pc88_alu_device::device_start()
{
	save_item(NAME(m_reg));
	save_item(NAME(m_ctrl1));
	save_item(NAME(m_ctrl2));
}

void pc88_alu_device::device_reset()
{
	// initialize ALU, assume disabled by default
	for(int i = 0; i < 3; i++)
		m_reg[i] = 0x00;
	m_ctrl1 = m_ctrl2 = 0;
}

void pc88_alu_device::ctrl1_w(u8 data)
{
	m_ctrl1 = data;
}

void pc88_alu_device::ctrl2_w(u8 data)
{
	m_ctrl2 = data;
}

u8 pc88_alu_device::alu_r(offs_t offset)
{
	uint8_t b, r, g;

	// ignore for debugger, wouldn't make sense anyway
	if (machine().side_effects_disabled())
		return 0xff;

	offset &= 0x3fff;

	/* store data to ALU regs */
	for(int i = 0; i < 3; i++)
		m_reg[i] = m_gvram_read(i * 0x4000 + offset);

	b = m_reg[0];
	r = m_reg[1];
	g = m_reg[2];
	if(!(m_ctrl2 & 1)) { b ^= 0xff; }
	if(!(m_ctrl2 & 2)) { r ^= 0xff; }
	if(!(m_ctrl2 & 4)) { g ^= 0xff; }

	return b & r & g;
}

void pc88_alu_device::alu_w(offs_t offset, u8 data)
{
	int i;

	offset &= 0x3fff;

	// ALU write mode
	switch(m_ctrl2 & 0x30)
	{
		// RMW logic operation
		case 0x00:
		{
			uint8_t logic_op;

			for(i = 0; i < 3; i++)
			{
				logic_op = (m_ctrl1 & (0x11 << i)) >> i;

				const u32 gvram_offset = i * 0x4000 + offset;
				u8 res = m_gvram_read(gvram_offset);
				switch(logic_op)
				{
					case 0x00: { res &= ~data; } break;
					case 0x01: { res |= data; } break;
					case 0x10: { res ^= data; } break;
					case 0x11: break; // NOP
				}
				m_gvram_write(gvram_offset, res);
			}
		}
		break;

		// restore data from ALU regs
		case 0x10:
		{
			for(i = 0; i < 3; i++)
				m_gvram_write(i * 0x4000 + offset, m_reg[i]);
		}
		break;

		// swap ALU reg 1 into R GVRAM
		case 0x20:
			m_gvram_write(0x0000 + offset, m_reg[1]);
			break;

		// swap ALU reg 0 into B GVRAM
		case 0x30:
			m_gvram_write(0x4000 + offset, m_reg[0]);
			break;
	}
}
