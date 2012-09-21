/* Konami PowerPC-based 3D games common functions */

#include "emu.h"
#include "cpu/sharc/sharc.h"
#include "machine/k033906.h"
#include "video/voodoo.h"
#include "konppc.h"

#define MAX_CG_BOARDS	2

static UINT32 dsp_comm_ppc[MAX_CG_BOARDS][2];
static UINT32 dsp_comm_sharc[MAX_CG_BOARDS][2];
static UINT8 dsp_shared_ram_bank[MAX_CG_BOARDS];

static INT32 cgboard_id;
static INT32 cgboard_type;
static INT32 num_cgboards;

static UINT32 *dsp_shared_ram[MAX_CG_BOARDS];

#define DSP_BANK_SIZE			0x10000
#define DSP_BANK_SIZE_WORD		(DSP_BANK_SIZE / 4)

static UINT32 dsp_state[MAX_CG_BOARDS];
static UINT32 nwk_device_sel[MAX_CG_BOARDS];
static const char *texture_bank[MAX_CG_BOARDS];

static int nwk_fifo_half_full_r;
static int nwk_fifo_half_full_w;
static int nwk_fifo_full;
static int nwk_fifo_mask;


static UINT32 *nwk_fifo[MAX_CG_BOARDS];
static INT32 nwk_fifo_read_ptr[MAX_CG_BOARDS];
static INT32 nwk_fifo_write_ptr[MAX_CG_BOARDS];

static UINT32 *nwk_ram[MAX_CG_BOARDS];

/*****************************************************************************/

void init_konami_cgboard(running_machine &machine, int num_boards, int type)
{
	int i;
	num_cgboards = num_boards;

	for (i=0; i < num_boards; i++)
	{
		dsp_comm_ppc[i][0] = 0x00;
		dsp_shared_ram[i] = auto_alloc_array(machine, UINT32, DSP_BANK_SIZE * 2/4);
		dsp_shared_ram_bank[i] = 0;

		dsp_state[i] = 0x80;
		texture_bank[i] = NULL;

		nwk_device_sel[i] = 0;
		nwk_fifo_read_ptr[i] = 0;
		nwk_fifo_write_ptr[i] = 0;

		nwk_fifo[i] = auto_alloc_array(machine, UINT32, 0x800);
		nwk_ram[i] = auto_alloc_array(machine, UINT32, 0x2000);

		state_save_register_item_array(machine, "konppc", NULL, i, dsp_comm_ppc[i]);
		state_save_register_item_array(machine, "konppc", NULL, i, dsp_comm_sharc[i]);
		state_save_register_item(machine, "konppc", NULL, i, dsp_shared_ram_bank[i]);
		state_save_register_item_pointer(machine, "konppc", NULL, i, dsp_shared_ram[i], DSP_BANK_SIZE * 2 / sizeof(dsp_shared_ram[i][0]));
		state_save_register_item(machine, "konppc", NULL, i, dsp_state[i]);
		state_save_register_item(machine, "konppc", NULL, i, nwk_device_sel[i]);
		state_save_register_item(machine, "konppc", NULL, i, nwk_fifo_read_ptr[i]);
		state_save_register_item(machine, "konppc", NULL, i, nwk_fifo_write_ptr[i]);
		state_save_register_item_pointer(machine, "konppc", NULL, i, nwk_fifo[i], 0x800);
		state_save_register_item_pointer(machine, "konppc", NULL, i, nwk_ram[i], 0x2000);
	}
	state_save_register_item(machine, "konppc", NULL, 0, cgboard_id);
	cgboard_type = type;

	if (type == CGBOARD_TYPE_NWKTR)
	{
		nwk_fifo_half_full_r = 0x100;
		nwk_fifo_half_full_w = 0xff;
		nwk_fifo_full = 0x1ff;
		nwk_fifo_mask = 0x1ff;
	}
	if (type == CGBOARD_TYPE_HANGPLT)
	{
		nwk_fifo_half_full_r = 0x3ff;
		nwk_fifo_half_full_w = 0x400;
		nwk_fifo_full = 0x7ff;
		nwk_fifo_mask = 0x7ff;
	}
}

void set_cgboard_id(int board_id)
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

int get_cgboard_id(void)
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

void set_cgboard_texture_bank(running_machine &machine, int board, const char *bank, UINT8 *rom)
{
	texture_bank[board] = bank;

	machine.root_device().membank(bank)->configure_entries(0, 2, rom, 0x800000);
}

/*****************************************************************************/

/* CG Board DSP interface for PowerPC */

READ32_HANDLER( cgboard_dsp_comm_r_ppc )
{
	if (cgboard_id < MAX_CG_BOARDS)
	{
//      mame_printf_debug("dsp_cmd_r: (board %d) %08X, %08X at %08X\n", cgboard_id, offset, mem_mask, space.device().safe_pc());
		return dsp_comm_sharc[cgboard_id][offset] | (dsp_state[cgboard_id] << 16);
	}
	else
	{
		return 0;
	}
}

WRITE32_HANDLER( cgboard_dsp_comm_w_ppc )
{
	const char *dsptag = (cgboard_id == 0) ? "dsp" : "dsp2";
	const char *pcitag = (cgboard_id == 0) ? "k033906_1" : "k033906_2";
	device_t *dsp = space.machine().device(dsptag);
	device_t *k033906 = space.machine().device(pcitag);
//  mame_printf_debug("dsp_cmd_w: (board %d) %08X, %08X, %08X at %08X\n", cgboard_id, data, offset, mem_mask, space.device().safe_pc());

	if (cgboard_id < MAX_CG_BOARDS)
	{
		if (offset == 0)
		{
			if (ACCESSING_BITS_24_31)
			{
				dsp_shared_ram_bank[cgboard_id] = (data >> 24) & 0x1;

				if (data & 0x80000000)
					dsp_state[cgboard_id] |= 0x10;

				if (k033906 != NULL)	/* zr107.c has no PCI and some games only have one PCI Bridge */
					k033906_set_reg(k033906, (data & 0x20000000) ? 1 : 0);

				if (data & 0x10000000)
					dsp->execute().set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				else
					dsp->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

				if (data & 0x02000000)
					dsp->execute().set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);

				if (data & 0x04000000)
					dsp->execute().set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
			}

			if (ACCESSING_BITS_0_7)
				dsp_comm_ppc[cgboard_id][offset] = data & 0xff;
		}
		else
			dsp_comm_ppc[cgboard_id][offset] = data;
	}
}



READ32_HANDLER( cgboard_dsp_shared_r_ppc )
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

WRITE32_HANDLER( cgboard_dsp_shared_w_ppc )
{
	if (cgboard_id < MAX_CG_BOARDS)
	{
		space.machine().scheduler().trigger(10000);		// Remove the timeout (a part of the GTI Club FIFO test workaround)
		COMBINE_DATA(dsp_shared_ram[cgboard_id] + (offset + (dsp_shared_ram_bank[cgboard_id] * DSP_BANK_SIZE_WORD)));
	}
}

/*****************************************************************************/

/* CG Board DSP interface for SHARC */

static UINT32 dsp_comm_sharc_r(int board, int offset)
{
	return dsp_comm_ppc[board][offset];
}

static void dsp_comm_sharc_w(address_space &space, int board, int offset, UINT32 data)
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
			//machine.device("dsp")->execute().set_input_line(SHARC_INPUT_FLAG0, ASSERT_LINE);
			sharc_set_flag_input(space.machine().device("dsp"), 0, ASSERT_LINE);

			if (offset == 1)
			{
				if (data & 0x03)
					space.machine().device("dsp")->execute().set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);
			}
			break;
		}

		case CGBOARD_TYPE_NWKTR:
		case CGBOARD_TYPE_HANGPLT:
		{
			const char *dsptag = (board == 0) ? "dsp" : "dsp2";
			device_t *device = space.machine().device(dsptag);

			if (offset == 1)
			{
				nwk_device_sel[board] = data;

				if (data & 0x01 || data & 0x10)
				{
					sharc_set_flag_input(device, 1, ASSERT_LINE);
				}

				if (texture_bank[board] != NULL)
				{
					int offset = (data & 0x08) ? 1 : 0;

					space.machine().root_device().membank(texture_bank[board])->set_entry(offset);
				}
			}
			break;
		}

		case CGBOARD_TYPE_HORNET:
		{
			if (offset == 1)
			{
				if (texture_bank[board] != NULL)
				{
					int offset = (data & 0x08) ? 1 : 0;

					space.machine().root_device().membank(texture_bank[board])->set_entry(offset);
				}
			}
			break;
		}
	}

//  printf("%s:cgboard_dsp_comm_w_sharc: %08X, %08X, %08X\n", space.machine().describe_context(), data, offset, mem_mask);

	dsp_comm_sharc[board][offset] = data;
}

static UINT32 dsp_shared_ram_r_sharc(int board, int offset)
{
//  printf("dsp_shared_r: (board %d) %08X, (%08X, %08X)\n", cgboard_id, offset, (UINT32)dsp_shared_ram[(offset >> 1)], (UINT32)dsp_shared_ram[offset]);

	if (offset & 0x1)
	{
		return (dsp_shared_ram[board][(offset >> 1) + ((dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] >> 0) & 0xffff;
	}
	else
	{
		return (dsp_shared_ram[board][(offset >> 1) + ((dsp_shared_ram_bank[board] ^ 1) * DSP_BANK_SIZE_WORD)] >> 16) & 0xffff;
	}
}

static void dsp_shared_ram_w_sharc(int board, int offset, UINT32 data)
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

READ32_HANDLER( cgboard_0_comm_sharc_r )
{
	return dsp_comm_sharc_r(0, offset);
}

WRITE32_HANDLER( cgboard_0_comm_sharc_w )
{
	dsp_comm_sharc_w(space, 0, offset, data);
}

READ32_HANDLER( cgboard_0_shared_sharc_r )
{
	return dsp_shared_ram_r_sharc(0, offset);
}

WRITE32_HANDLER( cgboard_0_shared_sharc_w )
{
	dsp_shared_ram_w_sharc(0, offset, data);
}

READ32_HANDLER( cgboard_1_comm_sharc_r )
{
	return dsp_comm_sharc_r(1, offset);
}

WRITE32_HANDLER( cgboard_1_comm_sharc_w )
{
	dsp_comm_sharc_w(space, 1, offset, data);
}

READ32_HANDLER( cgboard_1_shared_sharc_r )
{
	return dsp_shared_ram_r_sharc(1, offset);
}

WRITE32_HANDLER( cgboard_1_shared_sharc_w )
{
	dsp_shared_ram_w_sharc(1, offset, data);
}

/*****************************************************************************/

static UINT32 nwk_fifo_r(address_space &space, int board)
{
	const char *dsptag = (board == 0) ? "dsp" : "dsp2";
	device_t *device = space.machine().device(dsptag);
	UINT32 data;

	if (nwk_fifo_read_ptr[board] < nwk_fifo_half_full_r)
	{
		sharc_set_flag_input(device, 1, CLEAR_LINE);
	}
	else
	{
		sharc_set_flag_input(device, 1, ASSERT_LINE);
	}

	if (nwk_fifo_read_ptr[board] < nwk_fifo_full)
	{
		sharc_set_flag_input(device, 2, ASSERT_LINE);
	}
	else
	{
		sharc_set_flag_input(device, 2, CLEAR_LINE);
	}

	data = nwk_fifo[board][nwk_fifo_read_ptr[board]];
	nwk_fifo_read_ptr[board]++;
	nwk_fifo_read_ptr[board] &= nwk_fifo_mask;

	return data;
}

static void nwk_fifo_w(running_machine &machine, int board, UINT32 data)
{
	const char *dsptag = (board == 0) ? "dsp" : "dsp2";
	device_t *device = machine.device(dsptag);

	if (nwk_fifo_write_ptr[board] < nwk_fifo_half_full_w)
	{
		sharc_set_flag_input(device, 1, ASSERT_LINE);
	}
	else
	{
		sharc_set_flag_input(device, 1, CLEAR_LINE);
	}

	sharc_set_flag_input(device, 2, ASSERT_LINE);

	nwk_fifo[board][nwk_fifo_write_ptr[board]] = data;
	nwk_fifo_write_ptr[board]++;
	nwk_fifo_write_ptr[board] &= nwk_fifo_mask;
}

/*****************************************************************************/

/* Konami K033906 PCI bridge (most emulation is now in src/emu/machine/k033906.c ) */

READ32_HANDLER( K033906_0_r )
{
	device_t *k033906_1 = space.machine().device("k033906_1");
	if (nwk_device_sel[0] & 0x01)
		return nwk_fifo_r(space, 0);
	else
		return k033906_r(k033906_1, space, offset, mem_mask);
}

WRITE32_HANDLER( K033906_0_w )
{
	device_t *k033906_1 = space.machine().device("k033906_1");
	k033906_w(k033906_1, space, offset, data, mem_mask);
}

READ32_HANDLER( K033906_1_r )
{
	device_t *k033906_2 = space.machine().device("k033906_2");
	if (nwk_device_sel[1] & 0x01)
		return nwk_fifo_r(space, 1);
	else
		return k033906_r(k033906_2, space, offset, mem_mask);
}

WRITE32_HANDLER(K033906_1_w)
{
	device_t *k033906_2 = space.machine().device("k033906_2");
	k033906_w(k033906_2, space, offset, data, mem_mask);
}

/*****************************************************************************/

WRITE32_DEVICE_HANDLER(nwk_fifo_0_w)
{
	if (nwk_device_sel[0] & 0x01)
	{
		nwk_fifo_w(space.machine(), 0, data);
	}
	else if (nwk_device_sel[0] & 0x02)
	{
		int addr = ((offset >> 8) << 9) | (offset & 0xff);
		nwk_ram[0][addr] = data;
	}
	else
	{
		voodoo_w(device, space, offset ^ 0x80000, data, mem_mask);
	}
}

WRITE32_DEVICE_HANDLER(nwk_fifo_1_w)
{
	if (nwk_device_sel[1] & 0x01)
	{
		nwk_fifo_w(space.machine(), 1, data);
	}
	else if (nwk_device_sel[1] & 0x02)
	{
		int addr = ((offset >> 8) << 9) | (offset & 0xff);
		nwk_ram[1][addr] = data;
	}
	else
	{
		voodoo_w(device, space, offset ^ 0x80000, data, mem_mask);
	}
}

READ32_DEVICE_HANDLER(nwk_voodoo_0_r)
{
	if ((nwk_device_sel[0] == 0x4) && offset >= 0x100000 && offset < 0x200000)
	{
		return nwk_ram[0][offset & 0x1fff];
	}
	else
	{
		return voodoo_r(device, space, offset, mem_mask);
	}
}

READ32_DEVICE_HANDLER(nwk_voodoo_1_r)
{
	if ((nwk_device_sel[1] == 0x4) && offset >= 0x100000 && offset < 0x200000)
	{
		return nwk_ram[1][offset & 0x1fff];
	}
	else
	{
		return voodoo_r(device, space, offset, mem_mask);
	}
}

WRITE32_DEVICE_HANDLER(nwk_voodoo_0_w)
{
	if (nwk_device_sel[0] & 0x01)
	{
		nwk_fifo_w(space.machine(), 0, data);
	}
	else if (nwk_device_sel[0] & 0x02)
	{
		int addr = ((offset >> 8) << 9) | (offset & 0xff);
		nwk_ram[0][addr] = data;
	}
	else
	{
		voodoo_w(device, space, offset, data, mem_mask);
	}
}

WRITE32_DEVICE_HANDLER(nwk_voodoo_1_w)
{
	if (nwk_device_sel[1] & 0x01)
	{
		nwk_fifo_w(space.machine(), 1, data);
	}
	else if (nwk_device_sel[1] & 0x02)
	{
		int addr = ((offset >> 8) << 9) | (offset & 0xff);
		nwk_ram[1][addr] = data;
	}
	else
	{
		voodoo_w(device, space, offset, data, mem_mask);
	}
}

/*****************************************************************************/

#define LED_ON		0xff00ff00

void draw_7segment_led(bitmap_rgb32 &bitmap, int x, int y, UINT8 value)
{
	if ((value & 0x7f) == 0x7f)
	{
		return;
	}

	bitmap.plot_box(x-1, y-1, 7, 11, 0x00000000);

	/* Top */
	if( (value & 0x40) == 0 ) {
		bitmap.plot_box(x+1, y+0, 3, 1, LED_ON);
	}
	/* Middle */
	if( (value & 0x01) == 0 ) {
		bitmap.plot_box(x+1, y+4, 3, 1, LED_ON);
	}
	/* Bottom */
	if( (value & 0x08) == 0 ) {
		bitmap.plot_box(x+1, y+8, 3, 1, LED_ON);
	}
	/* Top Left */
	if( (value & 0x02) == 0 ) {
		bitmap.plot_box(x+0, y+1, 1, 3, LED_ON);
	}
	/* Top Right */
	if( (value & 0x20) == 0 ) {
		bitmap.plot_box(x+4, y+1, 1, 3, LED_ON);
	}
	/* Bottom Left */
	if( (value & 0x04) == 0 ) {
		bitmap.plot_box(x+0, y+5, 1, 3, LED_ON);
	}
	/* Bottom Right */
	if( (value & 0x10) == 0 ) {
		bitmap.plot_box(x+4, y+5, 1, 3, LED_ON);
	}
}
