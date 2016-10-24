// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        MC-80.xx by Miodrag Milanovic

        15/05/2009 Initial implementation
        12/05/2009 Skeleton driver.

****************************************************************************/

#include "includes/mc80.h"

/*****************************************************************************/
/*                            Implementation for MC80.2x                     */
/*****************************************************************************/

int mc80_state::mc8020_irq_callback(device_t &device, int irqline)
{
	return 0x00;
}

void mc80_state::machine_reset_mc8020()
{
}

void mc80_state::ctc_z0_w(int state)
{
}

void mc80_state::ctc_z1_w(int state)
{
}

void mc80_state::ctc_z2_w(int state)
{
	downcast<z80ctc_device *>(machine().device("z80ctc"))->trg0(state);
	downcast<z80ctc_device *>(machine().device("z80ctc"))->trg1(state);
}

uint8_t mc80_state::mc80_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0;
}

uint8_t mc80_state::mc80_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0;
}

void mc80_state::mc80_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}

void mc80_state::mc80_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}

/*****************************************************************************/
/*                            Implementation for MC80.3x                     */
/*****************************************************************************/

void mc80_state::mc8030_zve_write_protect_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}

void mc80_state::mc8030_vis_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	// reg C
	// 7 6 5 4 -- module
	//         3 - 0 left half, 1 right half
	//           2 1 0
	//           =====
	//           0 0 0 - dark
	//           0 0 1 - light
	//           0 1 0 - in reg pixel
	//           0 1 1 - negate in reg pixel
	//           1 0 x - operation code in B reg
	// reg B
	//
	uint16_t addr = ((offset & 0xff00) >> 2) | ((offset & 0x08) << 2) | (data >> 3);
	static const uint8_t val[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
	int c = offset & 1;
	m_p_videoram[addr] = m_p_videoram[addr] | (val[data & 7]*c);
}

void mc80_state::mc8030_eprom_prog_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}

int mc80_state::mc8030_irq_callback(device_t &device, int irqline)
{
	return 0x20;
}

void mc80_state::machine_reset_mc8030()
{
}

uint8_t mc80_state::zve_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0xff;
}

uint8_t mc80_state::zve_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0xff;
}

void mc80_state::zve_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}

void mc80_state::zve_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}

uint8_t mc80_state::asp_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0xff;
}

uint8_t mc80_state::asp_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0xff;
}

void mc80_state::asp_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}

void mc80_state::asp_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}
