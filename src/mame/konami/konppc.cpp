// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* Konami PowerPC-based 3D games common functions */

#include "emu.h"
#include "konppc.h"

#define LOG_COMM   (1 << 1)
#define LOG_SHARED (1 << 2)

#define LOG_ALL (LOG_COMM | LOG_SHARED)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGCOMM(...)   LOGMASKED(LOG_COMM, __VA_ARGS__)
#define LOGSHARED(...) LOGMASKED(LOG_SHARED, __VA_ARGS__)

static constexpr unsigned DSP_BANK_SIZE      = 0x10000;
static constexpr unsigned DSP_BANK_SIZE_WORD = (DSP_BANK_SIZE / 4);

/*****************************************************************************/


DEFINE_DEVICE_TYPE(KONPPC, konppc_device, "konppc", "Konami PowerPC Common Functions")

konppc_device::konppc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KONPPC, tag, owner, clock)
	, m_dsp(*this, {finder_base::DUMMY_TAG, finder_base::DUMMY_TAG})
	, m_k033906(*this, {finder_base::DUMMY_TAG, finder_base::DUMMY_TAG})
	, m_voodoo(*this, {finder_base::DUMMY_TAG, finder_base::DUMMY_TAG})
	, m_texture_bank(*this, {finder_base::DUMMY_TAG, finder_base::DUMMY_TAG})
	, m_cgboard_type(0)
	, m_num_cgboards(0)
	//, m_cgboard_id(MAX_CG_BOARDS)
{
}

//void konppc_device::init_konami_cgboard(running_machine &machine, int num_boards, int type)

void konppc_device::device_start()
{
	for (int i = 0; i < m_num_cgboards; i++)
	{
		m_dsp_comm_ppc[i][0] = 0x00;
		m_dsp_comm_ppc[i][1] = 0x00;
		m_dsp_comm_sharc[i][0] = 0x00;
		m_dsp_comm_sharc[i][1] = 0x00;
		m_dsp_shared_ram[i] = std::make_unique<uint32_t[]>(DSP_BANK_SIZE * 2/4);
		m_dsp_shared_ram_bank[i] = 0;

		m_dsp_state[i] = 0x80;

		m_nwk_device_sel[i] = 0;
		m_nwk_fifo_read_ptr[i] = 0;
		m_nwk_fifo_write_ptr[i] = 0;

		m_nwk_fifo[i] = std::make_unique<uint32_t[]>(0x800);
		m_nwk_ram[i] = std::make_unique<uint32_t[]>(0x2000);

		save_item(NAME(m_dsp_comm_ppc[i]), i);
		save_item(NAME(m_dsp_comm_sharc[i]), i);
		save_item(NAME(m_dsp_shared_ram_bank[i]), i);
		save_pointer(NAME(m_dsp_shared_ram[i]), DSP_BANK_SIZE * 2 / sizeof(m_dsp_shared_ram[i][0]), i);
		save_item(NAME(m_dsp_state[i]), i);
		save_item(NAME(m_nwk_device_sel[i]), i);
		save_item(NAME(m_nwk_fifo_read_ptr[i]), i);
		save_item(NAME(m_nwk_fifo_write_ptr[i]), i);
		save_pointer(NAME(m_nwk_fifo[i]), 0x800, i);
		save_pointer(NAME(m_nwk_ram[i]), 0x2000, i);
	}
	save_item(NAME(m_cgboard_id));

	if (m_cgboard_type == CGBOARD_TYPE_NWKTR)
	{
		m_nwk_fifo_half_full_r = 0x100;
		m_nwk_fifo_half_full_w = 0xff;
		m_nwk_fifo_full = 0x1ff;
		m_nwk_fifo_mask = 0x1ff;
	}
	if (m_cgboard_type == CGBOARD_TYPE_HANGPLT)
	{
		m_nwk_fifo_half_full_r = 0x3ff;
		m_nwk_fifo_half_full_w = 0x400;
		m_nwk_fifo_full = 0x7ff;
		m_nwk_fifo_mask = 0x7ff;
	}
}

void konppc_device::set_cgboard_id(int board_id)
{
	if (board_id > (m_num_cgboards - 1))
	{
		m_cgboard_id = MAX_CG_BOARDS;
	}
	else
	{
		m_cgboard_id = board_id;
	}
}

int konppc_device::get_cgboard_id(void)
{
	if (m_cgboard_id > (m_num_cgboards - 1))
	{
		return 0;
	}
	else
	{
		return m_cgboard_id;
	}
}

bool konppc_device::output_3d_enabled()
{
	if (m_cgboard_id < MAX_CG_BOARDS)
		return m_enable_3d[m_cgboard_id];
	else
		return false;
}

/*****************************************************************************/

/* CG Board DSP interface for PowerPC */

uint32_t konppc_device::cgboard_dsp_comm_r_ppc(offs_t offset, uint32_t mem_mask)
{
	if (m_cgboard_id < MAX_CG_BOARDS)
	{
		if (!machine().side_effects_disabled())
			LOGCOMM("%s cgboard_dsp_comm_r_ppc: (board %d) %08X & %08X\n", machine().describe_context(), m_cgboard_id, offset, mem_mask);
		return m_dsp_comm_sharc[m_cgboard_id][offset] | (m_dsp_state[m_cgboard_id] << 16);
	}
	else
	{
		return 0;
	}
}

void konppc_device::cgboard_dsp_comm_w_ppc(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGCOMM("%s cgboard_dsp_comm_w_ppc: (board %d) %08X = %08X & %08X\n", machine().describe_context(), m_cgboard_id, offset, data, mem_mask);

	if (m_cgboard_id < MAX_CG_BOARDS)
	{
		cpu_device &dsp = *m_dsp[m_cgboard_id];
		if (offset == 0)
		{
			if (ACCESSING_BITS_24_31)
			{
				assert(m_cgboard_id >= 0);
				m_dsp_shared_ram_bank[m_cgboard_id] = BIT(data, 24);

				if (BIT(data, 31))
					m_dsp_state[m_cgboard_id] |= 0x10;

				if (m_k033906[m_cgboard_id].found())    /* konami/zr107.cpp has no PCI and some games only have one PCI Bridge */
					m_k033906[m_cgboard_id]->set_reg(BIT(data, 29));

				if (BIT(data, 28))
					dsp.set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				else
					dsp.set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

				if (BIT(data, 25))
					dsp.set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);

				if (BIT(data, 26))
					dsp.set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
			}

			if (ACCESSING_BITS_16_23)
			{
				m_enable_3d[m_cgboard_id] = BIT(data, 22) ? true : false;
			}

			if (ACCESSING_BITS_0_7)
				m_dsp_comm_ppc[m_cgboard_id][offset] = data & 0xff;
		}
		else
			m_dsp_comm_ppc[m_cgboard_id][offset] = data;
	}
}



uint32_t konppc_device::cgboard_dsp_shared_r_ppc(offs_t offset)
{
	if (m_cgboard_id < MAX_CG_BOARDS)
	{
		return m_dsp_shared_ram[m_cgboard_id][offset + (m_dsp_shared_ram_bank[m_cgboard_id] * DSP_BANK_SIZE_WORD)];
	}
	else
	{
		return 0;
	}
}

void konppc_device::cgboard_dsp_shared_w_ppc(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_cgboard_id < MAX_CG_BOARDS)
	{
		machine().scheduler().trigger(10000);     // Remove the timeout (a part of the GTI Club FIFO test workaround)
		COMBINE_DATA(m_dsp_shared_ram[m_cgboard_id].get() + (offset + (m_dsp_shared_ram_bank[m_cgboard_id] * DSP_BANK_SIZE_WORD)));
	}
}

/*****************************************************************************/

/* CG Board DSP interface for SHARC */

uint32_t konppc_device::dsp_comm_sharc_r(int board, int offset)
{
	return m_dsp_comm_ppc[board][offset];
}

void konppc_device::dsp_comm_sharc_w(int board, int offset, uint32_t data)
{
	if (offset >= 2)
	{
		fatalerror("dsp_comm_w: %08X, %08X\n", data, offset);
	}

	switch (m_cgboard_type)
	{
		case CGBOARD_TYPE_ZR107:
		case CGBOARD_TYPE_GTICLUB:
		{
			//m_dsp[board]->set_input_line(SHARC_INPUT_FLAG0, ASSERT_LINE);
			m_dsp[board]->set_flag_input(0, ASSERT_LINE);

			if (offset == 1)
			{
				if (data & 0x03)
				{
					m_dsp[board]->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);
					m_dsp[board]->abort_timeslice();
				}
			}
			break;
		}

		case CGBOARD_TYPE_NWKTR:
		case CGBOARD_TYPE_HANGPLT:
		{
			if (offset == 1)
			{
				m_nwk_device_sel[board] = data;

				if (BIT(data, 0) || BIT(data, 4))
				{
					m_dsp[board]->set_flag_input(1, ASSERT_LINE);
				}

				if (m_texture_bank[board] != nullptr)
				{
					m_texture_bank[board]->set_entry(BIT(data, 3));
				}
			}
			break;
		}

		case CGBOARD_TYPE_HORNET:
		{
			if (offset == 1)
			{
				if (m_texture_bank[board] != nullptr)
				{
					m_texture_bank[board]->set_entry(BIT(data, 3));
				}
			}
			break;
		}
	}

	LOGCOMM("%s:dsp_comm_sharc_w: %08X = %08X\n", machine().describe_context().c_str(), offset, data);

	m_dsp_comm_sharc[board][offset] = data;
}

uint32_t konppc_device::dsp_shared_ram_r_sharc(int board, int offset)
{
	if (!machine().side_effects_disabled())
		LOGSHARED("dsp_shared_ram_r_sharc: (board %d) %08X = %08X\n", board, offset, (uint32_t)m_dsp_shared_ram[board][ + ((m_dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)]);

	if (BIT(offset, 0))
	{
		return (m_dsp_shared_ram[board][(offset >> 1) + ((m_dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] >> 0) & 0xffff;
	}
	else
	{
		return (m_dsp_shared_ram[board][(offset >> 1) + ((m_dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] >> 16) & 0xffff;
	}
}

void konppc_device::dsp_shared_ram_w_sharc(int board, int offset, uint32_t data)
{
	LOGSHARED("dsp_shared_ram_w_sharc: (board %d) %08X = %08X\n", board, offset, data);
	if (BIT(offset, 0))
	{
		m_dsp_shared_ram[board][(offset >> 1) + ((m_dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] &= 0xffff0000;
		m_dsp_shared_ram[board][(offset >> 1) + ((m_dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] |= (data & 0xffff);
	}
	else
	{
		m_dsp_shared_ram[board][(offset >> 1) + ((m_dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] &= 0x0000ffff;
		m_dsp_shared_ram[board][(offset >> 1) + ((m_dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] |= ((data & 0xffff) << 16);
	}
}

/*****************************************************************************/

uint32_t konppc_device::nwk_fifo_r(int board)
{
	if (m_nwk_fifo_read_ptr[board] < m_nwk_fifo_half_full_r)
	{
		m_dsp[board]->set_flag_input(1, CLEAR_LINE);
	}
	else
	{
		m_dsp[board]->set_flag_input(1, ASSERT_LINE);
	}

	if (m_nwk_fifo_read_ptr[board] < m_nwk_fifo_full)
	{
		m_dsp[board]->set_flag_input(2, ASSERT_LINE);
	}
	else
	{
		m_dsp[board]->set_flag_input(2, CLEAR_LINE);
	}

	uint32_t const data = m_nwk_fifo[board][m_nwk_fifo_read_ptr[board]];
	if (!machine().side_effects_disabled())
	{
		m_nwk_fifo_read_ptr[board]++;
		m_nwk_fifo_read_ptr[board] &= m_nwk_fifo_mask;
	}
	return data;
}

void konppc_device::nwk_fifo_w(int board, uint32_t data)
{
	if (m_nwk_fifo_write_ptr[board] < m_nwk_fifo_half_full_w)
	{
		m_dsp[board]->set_flag_input(1, ASSERT_LINE);
	}
	else
	{
		m_dsp[board]->set_flag_input(1, CLEAR_LINE);
	}

	m_dsp[board]->set_flag_input(2, ASSERT_LINE);

	m_nwk_fifo[board][m_nwk_fifo_write_ptr[board]] = data;
	m_nwk_fifo_write_ptr[board]++;
	m_nwk_fifo_write_ptr[board] &= m_nwk_fifo_mask;
}

/*****************************************************************************/

/* Konami K033906 PCI bridge (most emulation is now in src/devices/machine/k033906.cpp ) */

uint32_t konppc_device::k033906_r(int board, offs_t offset)
{
	if (BIT(m_nwk_device_sel[board], 0))
		return nwk_fifo_r(board);
	else
		return m_k033906[board]->read(offset);
}

void konppc_device::k033906_w(int board, offs_t offset, uint32_t data)
{
	m_k033906[board]->write(offset, data);
}

/*****************************************************************************/

uint32_t konppc_device::voodoo_r(int board, offs_t offset)
{
	if ((m_nwk_device_sel[board] == 0x4) && offset >= 0x100000 && offset < 0x200000)
	{
		return m_nwk_ram[board][offset & 0x1fff];
	}
	else
	{
		return m_voodoo[board]->read(offset);
	}
}

void konppc_device::voodoo_w(int board, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (BIT(m_nwk_device_sel[board], 0))
	{
		nwk_fifo_w(board, data);
	}
	else if (BIT(m_nwk_device_sel[board], 1))
	{
		int const addr = ((offset >> 8) << 9) | (offset & 0xff);
		m_nwk_ram[board][addr] = data;
	}
	else
	{
		m_voodoo[board]->write(offset, data, mem_mask);
	}
}

void konppc_device::voodoo_fifo_w(int board, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (BIT(m_nwk_device_sel[board], 0))
	{
		nwk_fifo_w(board, data);
	}
	else if (BIT(m_nwk_device_sel[board], 1))
	{
		int const addr = ((offset >> 8) << 9) | (offset & 0xff);
		m_nwk_ram[board][addr] = data;
	}
	else
	{
		m_voodoo[board]->write(offset ^ 0x80000, data, mem_mask);
	}
}
