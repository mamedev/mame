// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* Konami PowerPC-based 3D games common functions */

#include "emu.h"
#include "konppc.h"

#define DSP_BANK_SIZE           0x10000
#define DSP_BANK_SIZE_WORD      (DSP_BANK_SIZE / 4)

/*****************************************************************************/


DEFINE_DEVICE_TYPE(KONPPC, konppc_device, "konppc", "Konami PowerPC Common Functions")

konppc_device::konppc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KONPPC, tag, owner, clock)
	, m_dsp(*this, {"^dsp", "^dsp2"})
	, m_k033906(*this, "^k033906_%u", 1U)
	, m_voodoo(*this, "^voodoo%u", 0U)
	, cgboard_type(0)
	, num_cgboards(0)
	//, cgboard_id(MAX_CG_BOARDS)
{
}

//void konppc_device::init_konami_cgboard(running_machine &machine, int num_boards, int type)

void konppc_device::device_start()
{
	int i;

	for (i=0; i < num_cgboards; i++)
	{
		dsp_comm_ppc[i][0] = 0x00;
		dsp_shared_ram[i] = std::make_unique<uint32_t[]>(DSP_BANK_SIZE * 2/4);
		dsp_shared_ram_bank[i] = 0;

		dsp_state[i] = 0x80;
		texture_bank[i] = nullptr;

		nwk_device_sel[i] = 0;
		nwk_fifo_read_ptr[i] = 0;
		nwk_fifo_write_ptr[i] = 0;

		nwk_fifo[i] = std::make_unique<uint32_t[]>(0x800);
		nwk_ram[i] = std::make_unique<uint32_t[]>(0x2000);

		save_item(NAME(dsp_comm_ppc[i]), i);
		save_item(NAME(dsp_comm_sharc[i]), i);
		save_item(NAME(dsp_shared_ram_bank[i]), i);
		save_pointer(NAME(dsp_shared_ram[i]), DSP_BANK_SIZE * 2 / sizeof(dsp_shared_ram[i][0]), i);
		save_item(NAME(dsp_state[i]), i);
		save_item(NAME(nwk_device_sel[i]), i);
		save_item(NAME(nwk_fifo_read_ptr[i]), i);
		save_item(NAME(nwk_fifo_write_ptr[i]), i);
		save_pointer(NAME(nwk_fifo[i]), 0x800, i);
		save_pointer(NAME(nwk_ram[i]), 0x2000, i);
	}
	save_item(NAME(cgboard_id));

	if (cgboard_type == CGBOARD_TYPE_NWKTR)
	{
		nwk_fifo_half_full_r = 0x100;
		nwk_fifo_half_full_w = 0xff;
		nwk_fifo_full = 0x1ff;
		nwk_fifo_mask = 0x1ff;
	}
	if (cgboard_type == CGBOARD_TYPE_HANGPLT)
	{
		nwk_fifo_half_full_r = 0x3ff;
		nwk_fifo_half_full_w = 0x400;
		nwk_fifo_full = 0x7ff;
		nwk_fifo_mask = 0x7ff;
	}
}

void konppc_device::set_cgboard_id(int board_id)
{
	if (board_id > num_cgboards-1)
	{
		cgboard_id = MAX_CG_BOARDS;
	}
	else
	{
		cgboard_id = board_id;
	}
}

int konppc_device::get_cgboard_id(void)
{
	if (cgboard_id > num_cgboards-1)
	{
		return 0;
	}
	else
	{
		return cgboard_id;
	}
}

void konppc_device::set_cgboard_texture_bank(int board, const char *bank, uint8_t *rom)
{
	texture_bank[board] = bank;

	machine().root_device().membank(bank)->configure_entries(0, 2, rom, 0x800000);
}

bool konppc_device::output_3d_enabled()
{
	if (cgboard_id < MAX_CG_BOARDS)
		return enable_3d[cgboard_id];
	else
		return false;
}

/*****************************************************************************/

/* CG Board DSP interface for PowerPC */

uint32_t konppc_device::cgboard_dsp_comm_r_ppc(offs_t offset, uint32_t mem_mask)
{
	if (cgboard_id < MAX_CG_BOARDS)
	{
//      osd_printf_debug("%s dsp_cmd_r: (board %d) %08X, %08X\n", machine().describe_context(), cgboard_id, offset, mem_mask);
		return dsp_comm_sharc[cgboard_id][offset] | (dsp_state[cgboard_id] << 16);
	}
	else
	{
		return 0;
	}
}

void konppc_device::cgboard_dsp_comm_w_ppc(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  osd_printf_debug("%s dsp_cmd_w: (board %d) %08X, %08X, %08X\n", machine().describe_context(), cgboard_id, data, offset, mem_mask);

	if (cgboard_id < MAX_CG_BOARDS)
	{
		cpu_device &dsp = *m_dsp[cgboard_id];
		if (offset == 0)
		{
			if (ACCESSING_BITS_24_31)
			{
				assert(cgboard_id >= 0);
				dsp_shared_ram_bank[cgboard_id] = (data >> 24) & 0x1;

				if (data & 0x80000000)
					dsp_state[cgboard_id] |= 0x10;

				if (m_k033906[cgboard_id].found())    /* zr107.c has no PCI and some games only have one PCI Bridge */
					m_k033906[cgboard_id]->set_reg((data & 0x20000000) ? 1 : 0);

				if (data & 0x10000000)
					dsp.set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				else
					dsp.set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

				if (data & 0x02000000)
					dsp.set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);

				if (data & 0x04000000)
					dsp.set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
			}

			if (ACCESSING_BITS_16_23)
			{
				enable_3d[cgboard_id] = (data & 0x400000) ? true : false;
			}

			if (ACCESSING_BITS_0_7)
				dsp_comm_ppc[cgboard_id][offset] = data & 0xff;
		}
		else
			dsp_comm_ppc[cgboard_id][offset] = data;
	}
}



uint32_t konppc_device::cgboard_dsp_shared_r_ppc(offs_t offset)
{
	if (cgboard_id < MAX_CG_BOARDS)
	{
		return dsp_shared_ram[cgboard_id][offset + (dsp_shared_ram_bank[cgboard_id] * DSP_BANK_SIZE_WORD)];
	}
	else
	{
		return 0;
	}
}

void konppc_device::cgboard_dsp_shared_w_ppc(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (cgboard_id < MAX_CG_BOARDS)
	{
		machine().scheduler().trigger(10000);     // Remove the timeout (a part of the GTI Club FIFO test workaround)
		COMBINE_DATA(dsp_shared_ram[cgboard_id].get() + (offset + (dsp_shared_ram_bank[cgboard_id] * DSP_BANK_SIZE_WORD)));
	}
}

/*****************************************************************************/

/* CG Board DSP interface for SHARC */

uint32_t konppc_device::dsp_comm_sharc_r(int board, int offset)
{
	return dsp_comm_ppc[board][offset];
}

void konppc_device::dsp_comm_sharc_w(int board, int offset, uint32_t data)
{
	if (offset >= 2)
	{
		fatalerror("dsp_comm_w: %08X, %08X\n", data, offset);
	}

	switch (cgboard_type)
	{
		case CGBOARD_TYPE_ZR107:
		case CGBOARD_TYPE_GTICLUB:
		{
			//m_dsp[0]->set_input_line(SHARC_INPUT_FLAG0, ASSERT_LINE);
			m_dsp[0]->set_flag_input(0, ASSERT_LINE);

			if (offset == 1)
			{
				if (data & 0x03)
					m_dsp[0]->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);
			}
			break;
		}

		case CGBOARD_TYPE_NWKTR:
		case CGBOARD_TYPE_HANGPLT:
		{
			if (offset == 1)
			{
				nwk_device_sel[board] = data;

				if (data & 0x01 || data & 0x10)
				{
					m_dsp[board]->set_flag_input(1, ASSERT_LINE);
				}

				if (texture_bank[board] != nullptr)
				{
					int offset = (data & 0x08) ? 1 : 0;
					machine().root_device().membank(texture_bank[board])->set_entry(offset);
				}
			}
			break;
		}

		case CGBOARD_TYPE_HORNET:
		{
			if (offset == 1)
			{
				if (texture_bank[board] != nullptr)
				{
					int offset = (data & 0x08) ? 1 : 0;

					machine().root_device().membank(texture_bank[board])->set_entry(offset);
				}
			}
			break;
		}
	}

//  printf("%s:cgboard_dsp_comm_w_sharc: %08X, %08X, %08X\n", machine().describe_context().c_str(), data, offset, mem_mask);

	dsp_comm_sharc[board][offset] = data;
}

uint32_t konppc_device::dsp_shared_ram_r_sharc(int board, int offset)
{
//  printf("dsp_shared_r: (board %d) %08X, (%08X, %08X)\n", cgboard_id, offset, (uint32_t)dsp_shared_ram[(offset >> 1)], (uint32_t)dsp_shared_ram[offset]);

	if (offset & 0x1)
	{
		return (dsp_shared_ram[board][(offset >> 1) + ((dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] >> 0) & 0xffff;
	}
	else
	{
		return (dsp_shared_ram[board][(offset >> 1) + ((dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] >> 16) & 0xffff;
	}
}

void konppc_device::dsp_shared_ram_w_sharc(int board, int offset, uint32_t data)
{
//  printf("dsp_shared_w: (board %d) %08X, %08X\n", cgboard_id, offset, data);
	if (offset & 0x1)
	{
		dsp_shared_ram[board][(offset >> 1) + ((dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] &= 0xffff0000;
		dsp_shared_ram[board][(offset >> 1) + ((dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] |= (data & 0xffff);
	}
	else
	{
		dsp_shared_ram[board][(offset >> 1) + ((dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] &= 0x0000ffff;
		dsp_shared_ram[board][(offset >> 1) + ((dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] |= ((data & 0xffff) << 16);
	}
}

uint32_t konppc_device::cgboard_0_comm_sharc_r(offs_t offset)
{
	return dsp_comm_sharc_r(0, offset);
}

void konppc_device::cgboard_0_comm_sharc_w(offs_t offset, uint32_t data)
{
	dsp_comm_sharc_w(0, offset, data);
}

uint32_t konppc_device::cgboard_0_shared_sharc_r(offs_t offset)
{
	return dsp_shared_ram_r_sharc(0, offset);
}

void konppc_device::cgboard_0_shared_sharc_w(offs_t offset, uint32_t data)
{
	dsp_shared_ram_w_sharc(0, offset, data);
}

uint32_t konppc_device::cgboard_1_comm_sharc_r(offs_t offset)
{
	return dsp_comm_sharc_r(1, offset);
}

void konppc_device::cgboard_1_comm_sharc_w(offs_t offset, uint32_t data)
{
	dsp_comm_sharc_w(1, offset, data);
}

uint32_t konppc_device::cgboard_1_shared_sharc_r(offs_t offset)
{
	return dsp_shared_ram_r_sharc(1, offset);
}

void konppc_device::cgboard_1_shared_sharc_w(offs_t offset, uint32_t data)
{
	dsp_shared_ram_w_sharc(1, offset, data);
}

/*****************************************************************************/

uint32_t konppc_device::nwk_fifo_r(int board)
{
	if (nwk_fifo_read_ptr[board] < nwk_fifo_half_full_r)
	{
		m_dsp[board]->set_flag_input(1, CLEAR_LINE);
	}
	else
	{
		m_dsp[board]->set_flag_input(1, ASSERT_LINE);
	}

	if (nwk_fifo_read_ptr[board] < nwk_fifo_full)
	{
		m_dsp[board]->set_flag_input(2, ASSERT_LINE);
	}
	else
	{
		m_dsp[board]->set_flag_input(2, CLEAR_LINE);
	}

	uint32_t data = nwk_fifo[board][nwk_fifo_read_ptr[board]];
	nwk_fifo_read_ptr[board]++;
	nwk_fifo_read_ptr[board] &= nwk_fifo_mask;

	return data;
}

void konppc_device::nwk_fifo_w(int board, uint32_t data)
{
	if (nwk_fifo_write_ptr[board] < nwk_fifo_half_full_w)
	{
		m_dsp[board]->set_flag_input(1, ASSERT_LINE);
	}
	else
	{
		m_dsp[board]->set_flag_input(1, CLEAR_LINE);
	}

	m_dsp[board]->set_flag_input(2, ASSERT_LINE);

	nwk_fifo[board][nwk_fifo_write_ptr[board]] = data;
	nwk_fifo_write_ptr[board]++;
	nwk_fifo_write_ptr[board] &= nwk_fifo_mask;
}

/*****************************************************************************/

/* Konami K033906 PCI bridge (most emulation is now in src/devices/machine/k033906.cpp ) */

uint32_t konppc_device::K033906_0_r(offs_t offset)
{
	if (nwk_device_sel[0] & 0x01)
		return nwk_fifo_r(0);
	else
		return m_k033906[0]->read(offset);
}

void konppc_device::K033906_0_w(offs_t offset, uint32_t data)
{
	m_k033906[0]->write(offset, data);
}

uint32_t konppc_device::K033906_1_r(offs_t offset)
{
	if (nwk_device_sel[1] & 0x01)
		return nwk_fifo_r(1);
	else
		return m_k033906[1]->read(offset);
}

void konppc_device::K033906_1_w(offs_t offset, uint32_t data)
{
	m_k033906[1]->write(offset, data);
}

/*****************************************************************************/

void konppc_device::nwk_fifo_0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (nwk_device_sel[0] & 0x01)
	{
		nwk_fifo_w(0, data);
	}
	else if (nwk_device_sel[0] & 0x02)
	{
		int addr = ((offset >> 8) << 9) | (offset & 0xff);
		nwk_ram[0][addr] = data;
	}
	else
	{
		m_voodoo[0]->voodoo_w(offset ^ 0x80000, data, mem_mask);
	}
}

void konppc_device::nwk_fifo_1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (nwk_device_sel[1] & 0x01)
	{
		nwk_fifo_w(1, data);
	}
	else if (nwk_device_sel[1] & 0x02)
	{
		int addr = ((offset >> 8) << 9) | (offset & 0xff);
		nwk_ram[1][addr] = data;
	}
	else
	{
		m_voodoo[1]->voodoo_w(offset ^ 0x80000, data, mem_mask);
	}
}

uint32_t konppc_device::nwk_voodoo_0_r(offs_t offset)
{
	if ((nwk_device_sel[0] == 0x4) && offset >= 0x100000 && offset < 0x200000)
	{
		return nwk_ram[0][offset & 0x1fff];
	}
	else
	{
		return m_voodoo[0]->voodoo_r(offset);
	}
}

uint32_t konppc_device::nwk_voodoo_1_r(offs_t offset)
{
	if ((nwk_device_sel[1] == 0x4) && offset >= 0x100000 && offset < 0x200000)
	{
		return nwk_ram[1][offset & 0x1fff];
	}
	else
	{
		return m_voodoo[1]->voodoo_r(offset);
	}
}

void konppc_device::nwk_voodoo_0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (nwk_device_sel[0] & 0x01)
	{
		nwk_fifo_w(0, data);
	}
	else if (nwk_device_sel[0] & 0x02)
	{
		int addr = ((offset >> 8) << 9) | (offset & 0xff);
		nwk_ram[0][addr] = data;
	}
	else
	{
		m_voodoo[0]->voodoo_w(offset, data, mem_mask);
	}
}

void konppc_device::nwk_voodoo_1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (nwk_device_sel[1] & 0x01)
	{
		nwk_fifo_w(1, data);
	}
	else if (nwk_device_sel[1] & 0x02)
	{
		int addr = ((offset >> 8) << 9) | (offset & 0xff);
		nwk_ram[1][addr] = data;
	}
	else
	{
		m_voodoo[1]->voodoo_w(offset, data, mem_mask);
	}
}
