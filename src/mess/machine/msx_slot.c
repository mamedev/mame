/*
 * msx_slot.c : definitions of the different slots
 *
 * Copyright (C) 2004 Sean Young
 *
 * Missing:
 * - Holy Qu'ran
 *   like ascii8, with switch address 5000h/5400h/5800h/5c00h, not working.
 * - Harry Fox
 *   16kb banks, 6000h and 7000h switch address; isn't it really an ascii16?
 * - Halnote
 *   writes to page 0?
 * - Playball
 *   Unemulated D7756C, same as src/drivers/homerun.c
 * - Some ascii8 w/ sram need 32kb sram?
 * - MegaRAM
 * - fmsx painter.rom
 */

#include "emu.h"
#include "emuopts.h"
#include "machine/i8255.h"
#include "includes/msx_slot.h"
#include "includes/msx.h"
#include "machine/wd17xx.h"
#include "sound/k051649.h"
#include "sound/2413intf.h"
#include "sound/dac.h"
#include "sound/ay8910.h"

static void msx_cpu_setbank (running_machine &machine, int page, UINT8 *mem)
{
	msx_state *state = machine.driver_data<msx_state>();
	address_space &space = state->m_maincpu->space(AS_PROGRAM);
	switch (page)
	{
	case 1:
		state->m_bank1->set_base (mem);
		break;
	case 2:
		state->m_bank2->set_base (mem);
		break;
	case 3:
		state->m_bank3->set_base (mem);
		break;
	case 4:
		state->m_bank4->set_base (mem);
		state->m_bank5->set_base (mem + 0x1ff8);
		space.install_read_bank(0x7ff8, 0x7fff, "bank5");
		break;
	case 5:
		state->m_bank6->set_base (mem);
		state->m_bank7->set_base (mem + 0x1800);
		space.install_read_bank(0x9800, 0x9fff, "bank7");
		break;
	case 6:
		state->m_bank8->set_base (mem);
		state->m_bank9->set_base (mem + 0x1800);
		space.install_read_bank(0xb800, 0xbfff, "bank9");
		break;
	case 7:
		state->m_bank10->set_base (mem);
		break;
	case 8:
		state->m_bank11->set_base (mem);
		state->m_top_page = mem;
		break;
	}
}

MSX_SLOT_INIT(empty)
{
	state->m_type = SLOT_EMPTY;

	return 0;
}

MSX_SLOT_RESET(empty)
{
	/* reset void */
}

MSX_SLOT_MAP(empty)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	msx_cpu_setbank (machine, page * 2 + 1, drvstate->m_empty);
	msx_cpu_setbank (machine, page * 2 + 2, drvstate->m_empty);
}

MSX_SLOT_INIT(rom)
{
	state->m_type = SLOT_ROM;
	state->m_mem = mem;
	state->m_size = size;
	state->m_start_page = page;

	return 0;
}

MSX_SLOT_RESET(rom)
{
	/* state-less */
}

MSX_SLOT_MAP(rom)
{
	UINT8 *mem = state->m_mem + (page - state->m_start_page) * 0x4000;

	msx_cpu_setbank (machine, page * 2 + 1, mem);
	msx_cpu_setbank (machine, page * 2 + 2, mem + 0x2000);
}

MSX_SLOT_INIT(ram)
{
	state->m_mem = auto_alloc_array(machine, UINT8, size);
	memset (state->m_mem, 0, size);
	state->m_type = SLOT_RAM;
	state->m_start_page = page;
	state->m_size = size;

	return 0;
}

MSX_SLOT_MAP(ram)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	UINT8 *mem = state->m_mem + (page - state->m_start_page) * 0x4000;

	drvstate->m_ram_pages[page] = mem;
	msx_cpu_setbank (machine, page * 2 + 1, mem);
	msx_cpu_setbank (machine, page * 2 + 2, mem + 0x2000);
}

MSX_SLOT_RESET(ram)
{
}

MSX_SLOT_INIT(rammm)
{
	int i, mask, nsize;

	nsize = 0x10000; /* 64 kb */
	mask = 3;
	for (i=0; i<6; i++)
	{
		if (size == nsize)
		{
			break;
		}
		mask = (mask << 1) | 1;
		nsize <<= 1;
	}
	if (i == 6)
	{
		logerror ("ram mapper: error: must be 64kb, 128kb, 256kb, 512kb, "
					"1mb, 2mb or 4mb\n");
		return 1;
	}
	state->m_mem = auto_alloc_array(machine, UINT8, size);
	memset (state->m_mem, 0, size);

	state->m_type = SLOT_RAM_MM;
	state->m_start_page = page;
	state->m_size = size;
	state->m_bank_mask = mask;

	return 0;
}

MSX_SLOT_RESET(rammm)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	int i;

	for (i=0; i<4; i++)
	{
		drvstate->m_ram_mapper[i] = 3 - i;
	}
}

MSX_SLOT_MAP(rammm)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	UINT8 *mem = state->m_mem +
			0x4000 * (drvstate->m_ram_mapper[page] & state->m_bank_mask);

	drvstate->m_ram_pages[page] = mem;
	msx_cpu_setbank (machine, page * 2 + 1, mem);
	msx_cpu_setbank (machine, page * 2 + 2, mem + 0x2000);
}

MSX_SLOT_INIT(msxdos2)
{
	if (size != 0x10000)
	{
		logerror ("msxdos2: error: rom file must be 64kb\n");
		return 1;
	}
	state->m_type = SLOT_MSXDOS2;
	state->m_mem = mem;
	state->m_size = size;

	return 0;
}

MSX_SLOT_RESET(msxdos2)
{
	state->m_banks[0] = 0;
}

MSX_SLOT_MAP(msxdos2)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	if (page != 1)
	{
		msx_cpu_setbank (machine, page * 2 + 1, drvstate->m_empty);
		msx_cpu_setbank (machine, page * 2 + 2, drvstate->m_empty);
	}
	else
	{
		msx_cpu_setbank (machine, 3, state->m_mem + state->m_banks[0] * 0x4000);
		msx_cpu_setbank (machine, 4, state->m_mem + state->m_banks[0] * 0x4000 + 0x2000);
	}
}

MSX_SLOT_WRITE(msxdos2)
{
	if (addr == 0x6000)
	{
		state->m_banks[0] = val & 3;
		slot_msxdos2_map (machine, state, 1);
	}
}

MSX_SLOT_INIT(konami)
{
	int banks;

	if (size > 0x200000)
	{
		logerror ("konami: warning: truncating to 2mb\n");
		size = 0x200000;
	}
	banks = size / 0x2000;
	if (size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		logerror ("konami: error: must be a 2 power of 8kb\n");
		return 1;
	}
	state->m_type = SLOT_KONAMI;
	state->m_mem = mem;
	state->m_size = size;
	state->m_bank_mask = banks - 1;

	return 0;
}

MSX_SLOT_RESET(konami)
{
	int i;

	for (i=0; i<4; i++) state->m_banks[i] = i;
}

MSX_SLOT_MAP(konami)
{
	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, state->m_mem);
		msx_cpu_setbank (machine, 2, state->m_mem + state->m_banks[1] * 0x2000);
		break;
	case 1:
		msx_cpu_setbank (machine, 3, state->m_mem);
		msx_cpu_setbank (machine, 4, state->m_mem + state->m_banks[1] * 0x2000);
		break;
	case 2:
		msx_cpu_setbank (machine, 5, state->m_mem + state->m_banks[2] * 0x2000);
		msx_cpu_setbank (machine, 6, state->m_mem + state->m_banks[3] * 0x2000);
		break;
	case 3:
		msx_cpu_setbank (machine, 7, state->m_mem + state->m_banks[2] * 0x2000);
		msx_cpu_setbank (machine, 8, state->m_mem + state->m_banks[3] * 0x2000);
	}
}

MSX_SLOT_WRITE(konami)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	switch (addr)
	{
	case 0x6000:
		state->m_banks[1] = val & state->m_bank_mask;
		slot_konami_map (machine, state, 1);
		if (drvstate->m_state[0] == state)
		{
			slot_konami_map (machine, state, 0);
		}
		break;
	case 0x8000:
		state->m_banks[2] = val & state->m_bank_mask;
		slot_konami_map (machine, state, 2);
		if (drvstate->m_state[3] == state)
		{
			slot_konami_map (machine, state, 3);
		}
		break;
	case 0xa000:
		state->m_banks[3] = val & state->m_bank_mask;
		slot_konami_map (machine, state, 2);
		if (drvstate->m_state[3] == state)
		{
			slot_konami_map (machine, state, 3);
		}
	}
}

MSX_SLOT_INIT(konami_scc)
{
	int banks;

	if (size > 0x200000)
	{
		logerror ("konami_scc: warning: truncating to 2mb\n");
		size = 0x200000;
	}
	banks = size / 0x2000;
	if (size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		logerror ("konami_scc: error: must be a 2 power of 8kb\n");
		return 1;
	}

	state->m_type = SLOT_KONAMI_SCC;
	state->m_mem = mem;
	state->m_size = size;
	state->m_bank_mask = banks - 1;

	return 0;
}

MSX_SLOT_RESET(konami_scc)
{
	int i;

	for (i=0; i<4; i++) state->m_banks[i] = i;
	state->m_cart.scc.active = 0;
}

READ8_MEMBER(msx_state::konami_scc_bank5)
{
	if (offset & 0x80)
	{
		if ((offset & 0xff) >= 0xe0)
		{
			return m_k051649->k051649_test_r(space, offset & 0xff);
		}
		return 0xff;
	}
	else
	{
		return m_k051649->k051649_waveform_r(space, offset & 0x7f);
	}
}

MSX_SLOT_MAP(konami_scc)
{
	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, state->m_mem + state->m_banks[2] * 0x2000);
		msx_cpu_setbank (machine, 2, state->m_mem + state->m_banks[3] * 0x2000);
		break;
	case 1:
		msx_cpu_setbank (machine, 3, state->m_mem + state->m_banks[0] * 0x2000);
		msx_cpu_setbank (machine, 4, state->m_mem + state->m_banks[1] * 0x2000);
		break;
	case 2:
		msx_cpu_setbank (machine, 5, state->m_mem + state->m_banks[2] * 0x2000);
		msx_cpu_setbank (machine, 6, state->m_mem + state->m_banks[3] * 0x2000);
		if (state->m_cart.scc.active ) {
			msx_state *drvstate = machine.driver_data<msx_state>();
			drvstate->m_maincpu->space(AS_PROGRAM).install_read_handler(0x9800, 0x9fff, read8_delegate(FUNC(msx_state::konami_scc_bank5),drvstate));
		} else {
			msx_state *drvstate = machine.driver_data<msx_state>();
			drvstate->m_maincpu->space(AS_PROGRAM).install_read_bank(0x9800, 0x9fff,"bank7");
		}
		break;
	case 3:
		msx_cpu_setbank (machine, 7, state->m_mem + state->m_banks[0] * 0x2000);
		msx_cpu_setbank (machine, 8, state->m_mem + state->m_banks[1] * 0x2000);
	}
}

MSX_SLOT_WRITE(konami_scc)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	address_space &space = drvstate->m_maincpu->space(AS_PROGRAM);
	if (addr >= 0x5000 && addr < 0x5800)
	{
		state->m_banks[0] = val & state->m_bank_mask;
		slot_konami_scc_map (machine, state, 1);
		if (drvstate->m_state[3] == state)
		{
			slot_konami_scc_map (machine, state, 3);
		}
	}
	else if (addr >= 0x7000 && addr < 0x7800)
	{
		state->m_banks[1] = val & state->m_bank_mask;
		slot_konami_scc_map (machine, state, 1);
		if (drvstate->m_state[3] == state)
		{
			slot_konami_scc_map (machine, state, 3);
		}
	}
	else if (addr >= 0x9000 && addr < 0x9800)
	{
		state->m_banks[2] = val & state->m_bank_mask;
		state->m_cart.scc.active = ((val & 0x3f) == 0x3f);
		slot_konami_scc_map (machine, state, 2);
		if (drvstate->m_state[0] == state)
		{
			slot_konami_scc_map (machine, state, 0);
		}
	}
	else if (state->m_cart.scc.active && addr >= 0x9800 && addr < 0xa000)
	{
		int offset = addr & 0xff;

		if (offset < 0x80)
		{
			drvstate->m_k051649->k051649_waveform_w (space, offset, val);
		}
		else if (offset < 0xa0)
		{
			offset &= 0xf;
			if (offset < 0xa)
			{
				drvstate->m_k051649->k051649_frequency_w (space, offset, val);
			}
			else if (offset < 0xf)
			{
				drvstate->m_k051649->k051649_volume_w (space, offset - 0xa, val);
			}
			else
			{
				drvstate->m_k051649->k051649_keyonoff_w (space, 0, val);
			}
		}
		else if (offset >= 0xe0)
		{
			drvstate->m_k051649->k051649_test_w (space, offset, val);
		}
	}
	else if (addr >= 0xb000 && addr < 0xb800)
	{
		state->m_banks[3] = val & state->m_bank_mask;
		slot_konami_scc_map (machine, state, 2);
		if (drvstate->m_state[0] == state)
		{
			slot_konami_scc_map (machine, state, 0);
		}
	}
}

MSX_SLOT_INIT(ascii8)
{
	int banks;

	if (size > 0x200000)
	{
		logerror ("ascii8: warning: truncating to 2mb\n");
		size = 0x200000;
	}
	banks = size / 0x2000;
	if (size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		logerror ("ascii8: error: must be a 2 power of 8kb\n");
		return 1;
	}
	state->m_type = SLOT_ASCII8;
	state->m_mem = mem;
	state->m_size = size;
	state->m_bank_mask = banks - 1;

	return 0;
}

MSX_SLOT_RESET(ascii8)
{
	int i;

	for (i=0; i<4; i++) state->m_banks[i] = 0;
}

MSX_SLOT_MAP(ascii8)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, drvstate->m_empty);
		msx_cpu_setbank (machine, 2, drvstate->m_empty);
		break;
	case 1:
		msx_cpu_setbank (machine, 3, state->m_mem + state->m_banks[0] * 0x2000);
		msx_cpu_setbank (machine, 4, state->m_mem + state->m_banks[1] * 0x2000);
		break;
	case 2:
		msx_cpu_setbank (machine, 5, state->m_mem + state->m_banks[2] * 0x2000);
		msx_cpu_setbank (machine, 6, state->m_mem + state->m_banks[3] * 0x2000);
		break;
	case 3:
		msx_cpu_setbank (machine, 7, drvstate->m_empty);
		msx_cpu_setbank (machine, 8, drvstate->m_empty);
	}
}

MSX_SLOT_WRITE(ascii8)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	int bank;

	if (addr >= 0x6000 && addr < 0x8000)
	{
		bank = (addr / 0x800) & 3;

		state->m_banks[bank] = val & state->m_bank_mask;
		if (bank <= 1)
		{
			slot_ascii8_map (machine, state, 1);
		}
		else if (drvstate->m_state[2] == state)
		{
			slot_ascii8_map (machine, state, 2);
		}
	}
}

MSX_SLOT_INIT(ascii16)
{
	int banks;

	if (size > 0x400000)
	{
		logerror ("ascii16: warning: truncating to 4mb\n");
		size = 0x400000;
	}
	banks = size / 0x4000;
	if (size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		logerror ("ascii16: error: must be a 2 power of 16kb\n");
		return 1;
	}

	state->m_type = SLOT_ASCII16;
	state->m_mem = mem;
	state->m_size = size;
	state->m_bank_mask = banks - 1;

	return 0;
}

MSX_SLOT_RESET(ascii16)
{
	int i;

	for (i=0; i<2; i++) state->m_banks[i] = 0;
}

MSX_SLOT_MAP(ascii16)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	UINT8 *mem;

	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, drvstate->m_empty);
		msx_cpu_setbank (machine, 2, drvstate->m_empty);
		break;
	case 1:
		mem = state->m_mem + state->m_banks[0] * 0x4000;
		msx_cpu_setbank (machine, 3, mem);
		msx_cpu_setbank (machine, 4, mem + 0x2000);
		break;
	case 2:
		mem = state->m_mem + state->m_banks[1] * 0x4000;
		msx_cpu_setbank (machine, 5, mem);
		msx_cpu_setbank (machine, 6, mem + 0x2000);
		break;
	case 3:
		msx_cpu_setbank (machine, 7, drvstate->m_empty);
		msx_cpu_setbank (machine, 8, drvstate->m_empty);
	}
}

MSX_SLOT_WRITE(ascii16)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	if (addr >= 0x6000 && addr < 0x6800)
	{
		state->m_banks[0] = val & state->m_bank_mask;
		slot_ascii16_map (machine, state, 1);
	}
	else if (addr >= 0x7000 && addr < 0x7800)
	{
		state->m_banks[1] = val & state->m_bank_mask;
		if (drvstate->m_state[2] == state)
		{
			slot_ascii16_map (machine, state, 2);
		}
	}
}

MSX_SLOT_INIT(ascii8_sram)
{
	static const char sramfile[] = "ascii8";
	int banks;

	state->m_cart.sram.mem = auto_alloc_array(machine, UINT8, 0x2000);
	if (size > 0x100000)
	{
		logerror ("ascii8_sram: warning: truncating to 1mb\n");
		size = 0x100000;
	}
	banks = size / 0x2000;
	if (size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		logerror ("ascii8_sram: error: must be a 2 power of 8kb\n");
		return 1;
	}
	memset (state->m_cart.sram.mem, 0, 0x2000);
	state->m_type = SLOT_ASCII8_SRAM;
	state->m_mem = mem;
	state->m_size = size;
	state->m_bank_mask = banks - 1;
	state->m_cart.sram.sram_mask = banks;
	state->m_cart.sram.empty_mask = ~(banks | (banks - 1));
	if (!state->m_sramfile)
	{
		state->m_sramfile = sramfile;
	}

	return 0;
}

MSX_SLOT_RESET(ascii8_sram)
{
	int i;

	for (i=0; i<4; i++) state->m_banks[i] = 0;
}

static UINT8 *ascii8_sram_bank_select (msx_state *drvstate, slot_state *state, int bankno)
{
	int bank = state->m_banks[bankno];

	if (bank & state->m_cart.sram.empty_mask)
	{
		return drvstate->m_empty;
	}
	else if (bank & state->m_cart.sram.sram_mask)
	{
		return state->m_cart.sram.mem;
	}
	else
	{
		return state->m_mem + (bank & state->m_bank_mask) * 0x2000;
	}
}

MSX_SLOT_MAP(ascii8_sram)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, drvstate->m_empty);
		msx_cpu_setbank (machine, 2, drvstate->m_empty);
		break;
	case 1:
		msx_cpu_setbank (machine, 3, ascii8_sram_bank_select (drvstate, state, 0));
		msx_cpu_setbank (machine, 4, ascii8_sram_bank_select (drvstate, state, 1));
		break;
	case 2:
		msx_cpu_setbank (machine, 5, ascii8_sram_bank_select (drvstate, state, 2));
		msx_cpu_setbank (machine, 6, ascii8_sram_bank_select (drvstate, state, 3));
		break;
	case 3:
		msx_cpu_setbank (machine, 7, drvstate->m_empty);
		msx_cpu_setbank (machine, 8, drvstate->m_empty);
	}
}

MSX_SLOT_WRITE(ascii8_sram)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	int bank;

	if (addr >= 0x6000 && addr < 0x8000)
	{
		bank = (addr / 0x800) & 3;

		state->m_banks[bank] = val;
		if (bank <= 1)
		{
			slot_ascii8_sram_map (machine, state, 1);
		}
		else if (drvstate->m_state[2] == state)
		{
			slot_ascii8_sram_map (machine, state, 2);
		}
	}
	if (addr >= 0x8000 && addr < 0xc000)
	{
		bank = addr < 0xa000 ? 2 : 3;
		if (!(state->m_banks[bank] & state->m_cart.sram.empty_mask) &&
				(state->m_banks[bank] & state->m_cart.sram.sram_mask))
		{
			state->m_cart.sram.mem[addr & 0x1fff] = val;
		}
	}
}

MSX_SLOT_LOADSRAM(ascii8_sram)
{
	if (!state->m_sramfile)
	{
		logerror ("ascii8_sram: error: no sram filename provided\n");
		return 1;
	}
	emu_file f(machine.options().memcard_directory(), OPEN_FLAG_READ);
	file_error filerr = f.open(state->m_sramfile);
	if (filerr == FILERR_NONE)
	{
		if (f.read(state->m_cart.sram.mem, 0x2000) == 0x2000)
		{
			logerror ("ascii8_sram: info: sram loaded\n");
			return 0;
		}
		memset (state->m_cart.sram.mem, 0, 0x2000);
		logerror ("ascii8_sram: warning: could not read sram file\n");
		return 1;
	}

	logerror ("ascii8_sram: warning: could not open sram file for reading\n");

	return 1;
}

MSX_SLOT_SAVESRAM(ascii8_sram)
{
	if (!state->m_sramfile)
	{
		return 0;
	}

	emu_file f(machine.options().memcard_directory(), OPEN_FLAG_WRITE);
	file_error filerr = f.open(state->m_sramfile);
	if (filerr == FILERR_NONE)
	{
		f.write(state->m_cart.sram.mem, 0x2000);
		logerror ("ascii8_sram: info: sram saved\n");

		return 0;
	}

	logerror ("ascii8_sram: warning: could not open sram file for saving\n");

	return 1;
}

MSX_SLOT_INIT(ascii16_sram)
{
	static const char sramfile[] = "ascii16";
	int banks;

	state->m_cart.sram.mem = auto_alloc_array(machine, UINT8, 0x4000);

	if (size > 0x200000)
	{
		logerror ("ascii16_sram: warning: truncating to 2mb\n");
		size = 0x200000;
	}
	banks = size / 0x4000;
	if (size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		logerror ("ascii16_sram: error: must be a 2 power of 16kb\n");
		return 1;
	}

	memset (state->m_cart.sram.mem, 0, 0x4000);
	state->m_type = SLOT_ASCII16_SRAM;
	state->m_mem = mem;
	state->m_size = size;
	state->m_bank_mask = banks - 1;
	state->m_cart.sram.sram_mask = banks;
	state->m_cart.sram.empty_mask = ~(banks | (banks - 1));
	if (!state->m_sramfile)
	{
		state->m_sramfile = sramfile;
	}

	return 0;
}

MSX_SLOT_RESET(ascii16_sram)
{
	int i;

	for (i=0; i<2; i++) state->m_banks[i] = 0;
}

static UINT8 *ascii16_sram_bank_select (msx_state *drvstate, slot_state *state, int bankno)
{
	int bank = state->m_banks[bankno];

	if (bank & state->m_cart.sram.empty_mask)
	{
		return drvstate->m_empty;
	}
	else if (bank & state->m_cart.sram.sram_mask)
	{
		return state->m_cart.sram.mem;
	}
	else
	{
		return state->m_mem + (bank & state->m_bank_mask) * 0x4000;
	}
}

MSX_SLOT_MAP(ascii16_sram)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	UINT8 *mem;

	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, drvstate->m_empty);
		msx_cpu_setbank (machine, 2, drvstate->m_empty);
		break;
	case 1:
		mem = ascii16_sram_bank_select (drvstate, state, 0);
		msx_cpu_setbank (machine, 3, mem);
		msx_cpu_setbank (machine, 4, mem + 0x2000);
		break;
	case 2:
		mem = ascii16_sram_bank_select (drvstate, state, 1);
		msx_cpu_setbank (machine, 5, mem);
		msx_cpu_setbank (machine, 6, mem + 0x2000);
		break;
	case 3:
		msx_cpu_setbank (machine, 7, drvstate->m_empty);
		msx_cpu_setbank (machine, 8, drvstate->m_empty);
	}
}

MSX_SLOT_WRITE(ascii16_sram)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	if (addr >= 0x6000 && addr < 0x6800)
	{
		state->m_banks[0] = val;
		slot_ascii16_sram_map (machine, state, 1);
	}
	else if (addr >= 0x7000 && addr < 0x7800)
	{
		state->m_banks[1] = val;
		if (drvstate->m_state[2] == state)
		{
			slot_ascii16_sram_map (machine, state, 2);
		}
	}
	else if (addr >= 0x8000 && addr < 0xc000)
	{
		if (!(state->m_banks[1] & state->m_cart.sram.empty_mask) &&
				(state->m_banks[1] & state->m_cart.sram.sram_mask))
		{
			int offset, i;

			offset = addr & 0x07ff;
			for (i=0; i<8; i++)
			{
				state->m_cart.sram.mem[offset] = val;
				offset += 0x0800;
			}
		}
	}
}

MSX_SLOT_LOADSRAM(ascii16_sram)
{
	UINT8 *p;

	if (!state->m_sramfile)
	{
		logerror ("ascii16_sram: error: no sram filename provided\n");
		return 1;
	}

	emu_file f(machine.options().memcard_directory(), OPEN_FLAG_READ);
	file_error filerr = f.open(state->m_sramfile);
	if (filerr == FILERR_NONE)
	{
		p = state->m_cart.sram.mem;

		if (f.read(state->m_cart.sram.mem, 0x200) == 0x200)
		{
			int /*offset,*/ i;
			//offset = 0;
			for (i=0; i<7; i++)
			{
				memcpy (p + 0x800, p, 0x800);
				p += 0x800;
			}

			logerror ("ascii16_sram: info: sram loaded\n");
			return 0;
		}
		memset (state->m_cart.sram.mem, 0, 0x4000);
		logerror ("ascii16_sram: warning: could not read sram file\n");
		return 1;
	}

	logerror ("ascii16_sram: warning: could not open sram file for reading\n");

	return 1;
}

MSX_SLOT_SAVESRAM(ascii16_sram)
{
	if (!state->m_sramfile)
	{
		return 0;
	}
	emu_file f(machine.options().memcard_directory(), OPEN_FLAG_WRITE);
	file_error filerr = f.open(state->m_sramfile);
	if (filerr == FILERR_NONE)
	{
		f.write(state->m_cart.sram.mem, 0x200);
		logerror ("ascii16_sram: info: sram saved\n");

		return 0;
	}

	logerror ("ascii16_sram: warning: could not open sram file for saving\n");

	return 1;
}

MSX_SLOT_INIT(rtype)
{
	if (!(size == 0x60000 || size == 0x80000))
	{
		logerror ("rtype: error: rom file should be exactly 384kb\n");
		return 1;
	}

	state->m_type = SLOT_RTYPE;
	state->m_mem = mem;
	state->m_size = size;

	return 0;
}

MSX_SLOT_RESET(rtype)
{
	state->m_banks[0] = 15;
}

MSX_SLOT_MAP(rtype)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	UINT8 *mem;

	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, drvstate->m_empty);
		msx_cpu_setbank (machine, 2, drvstate->m_empty);
		break;
	case 1:
		mem = state->m_mem + 15 * 0x4000;
		msx_cpu_setbank (machine, 3, mem);
		msx_cpu_setbank (machine, 4, mem + 0x2000);
		break;
	case 2:
		mem = state->m_mem + state->m_banks[0] * 0x4000;
		msx_cpu_setbank (machine, 5, mem);
		msx_cpu_setbank (machine, 6, mem + 0x2000);
		break;
	case 3:
		msx_cpu_setbank (machine, 7, drvstate->m_empty);
		msx_cpu_setbank (machine, 8, drvstate->m_empty);
	}
}

MSX_SLOT_WRITE(rtype)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	if (addr >= 0x7000 && addr < 0x8000)
	{
		int data ;

		if (val & 0x10)
		{
			data = 0x10 | (val & 7);
		}
		else
		{
			data = val & 0x0f;
		}
		state->m_banks[0] = data;
		if (drvstate->m_state[2] == state)
		{
			slot_rtype_map (machine, state, 2);
		}
	}
}

MSX_SLOT_INIT(gmaster2)
{
	UINT8 *p;
	static const char sramfile[] = "GameMaster2";

	if (size != 0x20000)
	{
		logerror ("gmaster2: error: rom file should be 128kb\n");
		return 1;
	}
	state->m_type = SLOT_GAMEMASTER2;
	state->m_size = size;
	state->m_mem = mem;

	p = auto_alloc_array(machine, UINT8, 0x4000);
	memset (p, 0, 0x4000);
	state->m_cart.sram.mem = p;
	if (!state->m_sramfile)
	{
		state->m_sramfile = sramfile;
	}

	return 0;
}

MSX_SLOT_RESET(gmaster2)
{
	int i;

	for (i=0; i<4; i++)
	{
		state->m_banks[i] = i;
	}
}

MSX_SLOT_MAP(gmaster2)
{
	switch (page)
	{
	case 0:
	case 1:
		msx_cpu_setbank (machine, 1 + page * 2, state->m_mem); /* bank 0 is hardwired */
		if (state->m_banks[1] > 15)
		{
			msx_cpu_setbank (machine, 2 + page * 2, state->m_cart.sram.mem +
					(state->m_banks[1] - 16) * 0x2000);
		}
		else
		{
			msx_cpu_setbank (machine, 2 + page * 2, state->m_mem + state->m_banks[1] * 0x2000);
		}
		break;
	case 2:
	case 3:
		if (state->m_banks[2] > 15)
		{
			msx_cpu_setbank (machine, 5 + page * 2, state->m_cart.sram.mem +
					(state->m_banks[2] - 16) * 0x2000);
		}
		else
		{
			msx_cpu_setbank (machine, 5 + page * 2, state->m_mem + state->m_banks[2] * 0x2000);
		}
		if (state->m_banks[3] > 15)
		{
			msx_cpu_setbank (machine, 6 + page * 2, state->m_cart.sram.mem +
					(state->m_banks[3] - 16) * 0x2000);
		}
		else
		{
			msx_cpu_setbank (machine, 6 + page * 2, state->m_mem + state->m_banks[3] * 0x2000);
		}
		break;
	}
}

MSX_SLOT_WRITE(gmaster2)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	if (addr >= 0x6000 && addr < 0x7000)
	{
		if (val & 0x10)
		{
			val = val & 0x20 ? 17 : 16;
		}
		else
		{
			val = val & 15;
		}
		state->m_banks[1] = val;
		slot_gmaster2_map (machine, state, 1);
		if (drvstate->m_state[0] == state)
		{
			slot_gmaster2_map (machine, state, 0);
		}
	}
	else if (addr >= 0x8000 && addr < 0x9000)
	{
		if (val & 0x10)
		{
			val = val & 0x20 ? 17 : 16;
		}
		else
		{
			val = val & 15;
		}
		state->m_banks[2] = val;
		slot_gmaster2_map (machine, state, 2);
		if (drvstate->m_state[3] == state)
		{
			slot_gmaster2_map (machine, state, 3);
		}
	}
	else if (addr >= 0xa000 && addr < 0xb000)
	{
		if (val & 0x10)
		{
			val = val & 0x20 ? 17 : 16;
		}
		else
		{
			val = val & 15;
		}
		state->m_banks[3] = val;
		slot_gmaster2_map (machine, state, 2);
		if (drvstate->m_state[3] == state)
		{
			slot_gmaster2_map (machine, state, 3);
		}
	}
	else if (addr >= 0xb000 && addr < 0xc000)
	{
		addr &= 0x0fff;
		switch (state->m_banks[3])
		{
		case 16:
			state->m_cart.sram.mem[addr] = val;
			state->m_cart.sram.mem[addr + 0x1000] = val;
			break;
		case 17:
			state->m_cart.sram.mem[addr + 0x2000] = val;
			state->m_cart.sram.mem[addr + 0x3000] = val;
			break;
		}
	}
}

MSX_SLOT_LOADSRAM(gmaster2)
{
	UINT8 *p;

	p = state->m_cart.sram.mem;

	emu_file f(machine.options().memcard_directory(), OPEN_FLAG_READ);
	file_error filerr = f.open(state->m_sramfile);
	if (filerr == FILERR_NONE)
	{
		if (f.read(p + 0x1000, 0x2000) == 0x2000)
		{
			memcpy (p, p + 0x1000, 0x1000);
			memcpy (p + 0x3000, p + 0x2000, 0x1000);
			logerror ("gmaster2: info: sram loaded\n");
			return 0;
		}
		memset (p, 0, 0x4000);
		logerror ("gmaster2: warning: could not read sram file\n");
		return 1;
	}

	logerror ("gmaster2: warning: could not open sram file for reading\n");

	return 1;
}

MSX_SLOT_SAVESRAM(gmaster2)
{
	emu_file f(machine.options().memcard_directory(), OPEN_FLAG_WRITE);
	file_error filerr = f.open(state->m_sramfile);
	if (filerr == FILERR_NONE)
	{
		f.write(state->m_cart.sram.mem + 0x1000, 0x2000);
		logerror ("gmaster2: info: sram saved\n");

		return 0;
	}

	logerror ("gmaster2: warning: could not open sram file for saving\n");

	return 1;
}

MSX_SLOT_INIT(diskrom)
{
	if (size != 0x4000)
	{
		logerror ("diskrom: error: the diskrom should be 16kb\n");
		return 1;
	}

	state->m_type = SLOT_DISK_ROM;
	state->m_mem = mem;
	state->m_size = size;

	return 0;
}

MSX_SLOT_RESET(diskrom)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	drvstate->m_wd179x->reset();
}

READ8_MEMBER(msx_state::msx_diskrom_page1_r)
{
	switch (offset)
	{
	case 0: return m_wd179x->status_r (space, 0);
	case 1: return m_wd179x->track_r (space, 0);
	case 2: return m_wd179x->sector_r (space, 0);
	case 3: return m_wd179x->data_r (space, 0);
	case 7: return m_dsk_stat;
	default:
		return m_state[1]->m_mem[offset + 0x3ff8];
	}
}

READ8_MEMBER(msx_state::msx_diskrom_page2_r)
{
	if (offset >= 0x7f8)
	{
		switch (offset)
		{
		case 0x7f8:
			return m_wd179x->status_r (space, 0);
		case 0x7f9:
			return m_wd179x->track_r (space, 0);
		case 0x7fa:
			return m_wd179x->sector_r (space, 0);
		case 0x7fb:
			return m_wd179x->data_r (space, 0);
		case 0x7ff:
			return m_dsk_stat;
		default:
			return m_state[2]->m_mem[offset + 0x3800];
		}
	}
	else
	{
		return 0xff;
	}
}

MSX_SLOT_MAP(diskrom)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	address_space &space = drvstate->m_maincpu->space(AS_PROGRAM);
	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, drvstate->m_empty);
		msx_cpu_setbank (machine, 2, drvstate->m_empty);
		break;
	case 1:
		msx_cpu_setbank (machine, 3, state->m_mem);
		msx_cpu_setbank (machine, 4, state->m_mem + 0x2000);
		space.install_read_handler(0x7ff8, 0x7fff, read8_delegate(FUNC(msx_state::msx_diskrom_page1_r),drvstate));
		break;
	case 2:
		msx_cpu_setbank (machine, 5, drvstate->m_empty);
		msx_cpu_setbank (machine, 6, drvstate->m_empty);
		space.install_read_handler(0xb800, 0xbfff, read8_delegate(FUNC(msx_state::msx_diskrom_page2_r),drvstate));
		break;
	case 3:
		msx_cpu_setbank (machine, 7, drvstate->m_empty);
		msx_cpu_setbank (machine, 8, drvstate->m_empty);
		break;
	}
}

MSX_SLOT_WRITE(diskrom)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	address_space &space = drvstate->m_maincpu->space(AS_PROGRAM);
	if (addr >= 0xa000 && addr < 0xc000)
	{
		addr -= 0x4000;
	}
	switch (addr)
	{
	case 0x7ff8:
		drvstate->m_wd179x->command_w (space, 0, val);
		break;
	case 0x7ff9:
		drvstate->m_wd179x->track_w (space, 0, val);
		break;
	case 0x7ffa:
		drvstate->m_wd179x->sector_w (space, 0, val);
		break;
	case 0x7ffb:
		drvstate->m_wd179x->data_w (space, 0, val);
		break;
	case 0x7ffc:
		drvstate->m_wd179x->set_side (val & 1);
		state->m_mem[0x3ffc] = val | 0xfe;
		break;
	case 0x7ffd:
		drvstate->m_wd179x->set_drive (val & 1);
		if ((state->m_mem[0x3ffd] ^ val) & 0x40)
		{
			set_led_status (machine, 0, !(val & 0x40));
		}
		state->m_mem[0x3ffd] = (val | 0x7c) & ~0x04;
		break;
	}
}

MSX_SLOT_INIT(diskrom2)
{
	if (size != 0x4000)
	{
		logerror ("diskrom2: error: the diskrom2 should be 16kb\n");
		return 1;
	}

	state->m_type = SLOT_DISK_ROM2;
	state->m_mem = mem;
	state->m_size = size;

	return 0;
}

MSX_SLOT_RESET(diskrom2)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	drvstate->m_wd179x->reset();
}

READ8_MEMBER(msx_state::msx_diskrom2_page1_r)
{
	switch (offset)
	{
	case 0: return m_wd179x->status_r(space, 0);
	case 1: return m_wd179x->track_r(space, 0);
	case 2: return m_wd179x->sector_r(space, 0);
	case 3: return m_wd179x->data_r(space, 0);
	case 4: return m_dsk_stat;
	default:
		return m_state[1]->m_mem[offset + 0x3ff8];
	}
}

READ8_MEMBER(msx_state::msx_diskrom2_page2_r)
{
	if (offset >= 0x7b8)
	{
		switch (offset)
		{
		case 0x7b8:
			return m_wd179x->status_r (space, 0);
		case 0x7b9:
			return m_wd179x->track_r (space, 0);
		case 0x7ba:
			return m_wd179x->sector_r (space, 0);
		case 0x7bb:
			return m_wd179x->data_r (space, 0);
		case 0x7bc:
			return m_dsk_stat;
		default:
			return m_state[2]->m_mem[offset + 0x3800];
		}
	}
	else
	{
		return 0xff;
	}
}

MSX_SLOT_MAP(diskrom2)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	address_space &space = drvstate->m_maincpu->space(AS_PROGRAM);
	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, drvstate->m_empty);
		msx_cpu_setbank (machine, 2, drvstate->m_empty);
		break;
	case 1:
		msx_cpu_setbank (machine, 3, state->m_mem);
		msx_cpu_setbank (machine, 4, state->m_mem + 0x2000);
		space.install_read_handler(0x7fb8, 0x7fbc, read8_delegate(FUNC(msx_state::msx_diskrom2_page1_r),drvstate));
		break;
	case 2:
		msx_cpu_setbank (machine, 5, drvstate->m_empty);
		msx_cpu_setbank (machine, 6, drvstate->m_empty);
		space.install_read_handler(0xb800, 0xbfbc, read8_delegate(FUNC(msx_state::msx_diskrom2_page2_r),drvstate));
		break;
	case 3:
		msx_cpu_setbank (machine, 7, drvstate->m_empty);
		msx_cpu_setbank (machine, 8, drvstate->m_empty);
	}
}

MSX_SLOT_WRITE(diskrom2)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	address_space &space = drvstate->m_maincpu->space(AS_PROGRAM);
	if (addr >= 0xa000 && addr < 0xc000)
	{
		addr -= 0x4000;
	}
	switch (addr)
	{
	case 0x7fb8:
		drvstate->m_wd179x->command_w (space, 0, val);
		break;
	case 0x7fb9:
		drvstate->m_wd179x->track_w (space, 0, val);
		break;
	case 0x7fba:
		drvstate->m_wd179x->sector_w (space, 0, val);
		break;
	case 0x7fbb:
		drvstate->m_wd179x->data_w (space, 0, val);
		break;
	case 0x7fbc:
		drvstate->m_wd179x->set_side (val & 1);
		state->m_mem[0x3fbc] = val | 0xfe;
		drvstate->m_wd179x->set_drive (val & 1);
		if ((state->m_mem[0x3fbc] ^ val) & 0x40)
		{
			set_led_status (machine, 0, !(val & 0x40));
		}
		state->m_mem[0x3fbc] = (val | 0x7c) & ~0x04;
		break;
	}
}

MSX_SLOT_INIT(synthesizer)
{
	if (size != 0x8000)
	{
		logerror ("synthesizer: error: rom file must be 32kb\n");
		return 1;
	}
	state->m_type = SLOT_SYNTHESIZER;
	state->m_mem = mem;
	state->m_size = size;

	return 0;
}

MSX_SLOT_RESET(synthesizer)
{
	/* empty */
}

MSX_SLOT_MAP(synthesizer)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, drvstate->m_empty);
		msx_cpu_setbank (machine, 2, drvstate->m_empty);
		break;
	case 1:
		msx_cpu_setbank (machine, 3, state->m_mem);
		msx_cpu_setbank (machine, 4, state->m_mem + 0x2000);
		break;
	case 2:
		msx_cpu_setbank (machine, 5, state->m_mem + 0x4000);
		msx_cpu_setbank (machine, 6, state->m_mem + 0x6000);
		break;
	case 3:
		msx_cpu_setbank (machine, 7, drvstate->m_empty);
		msx_cpu_setbank (machine, 8, drvstate->m_empty);
	}
}

MSX_SLOT_WRITE(synthesizer)
{
	if (addr >= 0x4000 && addr < 0x8000 && !(addr & 0x0010))
	{
		msx_state *drvstate = machine.driver_data<msx_state>();
		drvstate->m_dac->write_unsigned8(val);
	}
}

MSX_SLOT_INIT(majutsushi)
{
	if (size != 0x20000)
	{
		logerror ("majutsushi: error: rom file must be 128kb\n");
		return 1;
	}
	state->m_type = SLOT_MAJUTSUSHI;
	state->m_mem = mem;
	state->m_size = size;
	state->m_bank_mask = 0x0f;

	return 0;
}

MSX_SLOT_RESET(majutsushi)
{
	int i;

	for (i=0; i<4; i++)
	{
		state->m_banks[i] = i;
	}
}

MSX_SLOT_MAP(majutsushi)
{
	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, state->m_mem + state->m_banks[0] * 0x2000);
		msx_cpu_setbank (machine, 2, state->m_mem + state->m_banks[1] * 0x2000);
		break;
	case 1:
		msx_cpu_setbank (machine, 3, state->m_mem + state->m_banks[0] * 0x2000);
		msx_cpu_setbank (machine, 4, state->m_mem + state->m_banks[1] * 0x2000);
		break;
	case 2:
		msx_cpu_setbank (machine, 5, state->m_mem + state->m_banks[2] * 0x2000);
		msx_cpu_setbank (machine, 6, state->m_mem + state->m_banks[3] * 0x2000);
		break;
	case 3:
		msx_cpu_setbank (machine, 7, state->m_mem + state->m_banks[2] * 0x2000);
		msx_cpu_setbank (machine, 8, state->m_mem + state->m_banks[3] * 0x2000);
		break;
	}
}

MSX_SLOT_WRITE(majutsushi)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	if (addr >= 0x5000 && addr < 0x6000)
	{
		drvstate->m_dac->write_unsigned8(val);
	}
	else if (addr >= 0x6000 && addr < 0x8000)
	{
		state->m_banks[1] = val & 0x0f;
		slot_majutsushi_map (machine, state, 1);
		if (drvstate->m_state[0] == state)
		{
			slot_konami_map (machine, state, 0);
		}
	}
	else if (addr >= 0x8000 && addr < 0xc000)
	{
		state->m_banks[addr < 0xa000 ? 2 : 3] = val & 0x0f;
		slot_majutsushi_map (machine, state, 2);
		if (drvstate->m_state[3] == state)
		{
			slot_konami_map (machine, state, 3);
		}
	}
}

MSX_SLOT_INIT(fmpac)
{
	static const char sramfile[] = "fmpac.rom";
	UINT8 *p;
	int banks;

	if (size > 0x400000)
	{
		logerror ("fmpac: warning: truncating rom to 4mb\n");
		size = 0x400000;
	}
	banks = size / 0x4000;
	if (size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		logerror ("fmpac: error: must be a 2 power of 16kb\n");
		return 1;
	}

	if (!strncmp ((char*)mem + 0x18, "PAC2", 4))
	{
		state->m_cart.fmpac.sram_support = 1;
		p = auto_alloc_array(machine, UINT8, 0x4000);
		memset (p, 0, 0x2000);
		memset (p + 0x2000, 0xff, 0x2000);
		state->m_cart.fmpac.mem = p;
	}
	else
	{
		state->m_cart.fmpac.sram_support = 0;
		state->m_cart.fmpac.mem = NULL;
	}

	state->m_type = SLOT_FMPAC;
	state->m_size = size;
	state->m_mem = mem;
	state->m_bank_mask = banks - 1;
	if (!state->m_sramfile)
	{
		state->m_sramfile = sramfile;
	}

	return 0;
}

MSX_SLOT_RESET(fmpac)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	int i;

	state->m_banks[0] = 0;
	state->m_cart.fmpac.sram_active = 0;
	state->m_cart.fmpac.opll_active = 0;
	drvstate->m_opll_active = 0;
	for (i=0; i<=state->m_bank_mask; i++)
	{
		state->m_mem[0x3ff6 + i * 0x4000] = 0;
	}

	/* NPW 21-Feb-2004 - Adding check for null */
	if (state->m_cart.fmpac.mem)
	{
		state->m_cart.fmpac.mem[0x3ff6] = 0;
		state->m_cart.fmpac.mem[0x3ff7] = 0;
	}

	/* IMPROVE: reset sound chip */
}

MSX_SLOT_MAP(fmpac)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	if (page == 1)
	{
		if (state->m_cart.fmpac.sram_active)
		{
			msx_cpu_setbank (machine, 3, state->m_cart.fmpac.mem);
			msx_cpu_setbank (machine, 4, state->m_cart.fmpac.mem + 0x2000);
		}
		else
		{
			msx_cpu_setbank (machine, 3, state->m_mem + state->m_banks[0] * 0x4000);
			msx_cpu_setbank (machine, 4, state->m_mem + state->m_banks[0] * 0x4000 + 0x2000);
		}
	}
	else
	{
		msx_cpu_setbank (machine, page * 2 + 1, drvstate->m_empty);
		msx_cpu_setbank (machine, page * 2 + 2, drvstate->m_empty);
	}
}

MSX_SLOT_WRITE(fmpac)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	address_space &space = drvstate->m_maincpu->space(AS_PROGRAM);
	int i, data;

	if (addr >= 0x4000 && addr < 0x6000 && state->m_cart.fmpac.sram_support)
	{
		if (state->m_cart.fmpac.sram_active || addr >= 0x5ffe)
		{
			state->m_cart.fmpac.mem[addr & 0x1fff] = val;
		}

		state->m_cart.fmpac.sram_active =
				(state->m_cart.fmpac.mem[0x1ffe] == 0x4d &&
					state->m_cart.fmpac.mem[0x1fff] == 0x69);
	}

	switch (addr)
	{
	case 0x7ff4:
		if (state->m_cart.fmpac.opll_active)
		{
			drvstate->m_ym->write(space, 0, val);
		}
		break;
	case 0x7ff5:
		if (state->m_cart.fmpac.opll_active)
		{
			drvstate->m_ym->write(space, 1, val);
		}
		break;
	case 0x7ff6:
		data = val & 0x11;
		for (i=0; i<=state->m_bank_mask; i++)
		{
			state->m_mem[0x3ff6 + i * 0x4000] = data;
		}
		state->m_cart.fmpac.mem[0x3ff6] = data;
		state->m_cart.fmpac.opll_active = val & 1;
		if ((drvstate->m_opll_active ^ val) & 1)
		{
			logerror ("FM-PAC: OPLL %sactivated\n", val & 1 ? "" : "de");
		}
		drvstate->m_opll_active = val & 1;
		break;
	case 0x7ff7:
		state->m_banks[0] = val & state->m_bank_mask;
		state->m_cart.fmpac.mem[0x3ff7] = val & state->m_bank_mask;
		slot_fmpac_map (machine, state, 1);
		break;
	}
}

static const char PAC_HEADER[] = "PAC2 BACKUP DATA";
#define PAC_HEADER_LEN (16)

MSX_SLOT_LOADSRAM(fmpac)
{
	char buf[PAC_HEADER_LEN];

	if (!state->m_cart.fmpac.sram_support)
	{
		logerror ("Your fmpac.rom does not support sram\n");
		return 1;
	}

	if (!state->m_sramfile)
	{
		logerror ("No sram filename provided\n");
		return 1;
	}
	emu_file f(machine.options().memcard_directory(), OPEN_FLAG_READ);
	file_error filerr = f.open(state->m_sramfile);
	if (filerr == FILERR_NONE)
	{
		if ((f.read(buf, PAC_HEADER_LEN) == PAC_HEADER_LEN) &&
			!strncmp (buf, PAC_HEADER, PAC_HEADER_LEN) &&
			f.read(state->m_cart.fmpac.mem, 0x1ffe))
		{
			logerror ("fmpac: info: sram loaded\n");
			return 0;
		}
		else
		{
			logerror ("fmpac: warning: failed to load sram\n");
			return 1;
		}
	}

	logerror ("fmpac: warning: could not open sram file\n");
	return 1;
}

MSX_SLOT_SAVESRAM(fmpac)
{
	if (!state->m_cart.fmpac.sram_support || !state->m_sramfile)
	{
		return 0;
	}

	emu_file f(machine.options().memcard_directory(), OPEN_FLAG_WRITE);
	file_error filerr = f.open(state->m_sramfile);
	if (filerr == FILERR_NONE)
	{
		if ((f.write(PAC_HEADER, PAC_HEADER_LEN) == PAC_HEADER_LEN) &&
			(f.write(state->m_cart.fmpac.mem, 0x1ffe) == 0x1ffe))
		{
			logerror ("fmpac: info: sram saved\n");
			return 0;
		}
		else
		{
			logerror ("fmpac: warning: sram save to file failed\n");
			return 1;
		}
	}

	logerror ("fmpac: warning: could not open sram file for writing\n");

	return 1;
}

MSX_SLOT_INIT(superloderunner)
{
	if (size != 0x20000)
	{
		logerror ("superloderunner: error: rom file should be exactly "
					"128kb\n");
		return 1;
	}
	state->m_type = SLOT_SUPERLODERUNNER;
	state->m_mem = mem;
	state->m_size = size;
	state->m_start_page = page;
	state->m_bank_mask = 7;

	return 0;
}

MSX_SLOT_RESET(superloderunner)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	drvstate->m_superloderunner_bank = 0;
}

MSX_SLOT_MAP(superloderunner)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	if (page == 2)
	{
		UINT8 *mem = state->m_mem +
				(drvstate->m_superloderunner_bank & state->m_bank_mask) * 0x4000;

		msx_cpu_setbank (machine, 5, mem);
		msx_cpu_setbank (machine, 6, mem + 0x2000);
	}
	else
	{
		msx_cpu_setbank (machine, page * 2 + 1, drvstate->m_empty);
		msx_cpu_setbank (machine, page * 2 + 2, drvstate->m_empty);
	}
}

MSX_SLOT_INIT(crossblaim)
{
	if (size != 0x10000)
	{
		logerror ("crossblaim: error: rom file should be exactly 64kb\n");
		return 1;
	}
	state->m_type = SLOT_CROSS_BLAIM;
	state->m_mem = mem;
	state->m_size = size;

	return 0;
}

MSX_SLOT_RESET(crossblaim)
{
	state->m_banks[0] = 1;
}

MSX_SLOT_MAP(crossblaim)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	UINT8 *mem;

	/* This might look odd, but it's what happens on the real cartridge */

	switch (page)
	{
	case 0:
		if (state->m_banks[0] < 2){
			mem = state->m_mem + state->m_banks[0] * 0x4000;
			msx_cpu_setbank (machine, 1, mem);
			msx_cpu_setbank (machine, 2, mem + 0x2000);
		}
		else
		{
			msx_cpu_setbank (machine, 1, drvstate->m_empty);
			msx_cpu_setbank (machine, 2, drvstate->m_empty);
		}
		break;
	case 1:
		msx_cpu_setbank (machine, 3, state->m_mem);
		msx_cpu_setbank (machine, 4, state->m_mem + 0x2000);
		break;
	case 2:
		mem = state->m_mem + state->m_banks[0] * 0x4000;
		msx_cpu_setbank (machine, 5, mem);
		msx_cpu_setbank (machine, 6, mem + 0x2000);
		break;
	case 3:
		if (state->m_banks[0] < 2){
			mem = state->m_mem + state->m_banks[0] * 0x4000;
			msx_cpu_setbank (machine, 7, mem);
			msx_cpu_setbank (machine, 8, mem + 0x2000);
		}
		else
		{
			msx_cpu_setbank (machine, 7, drvstate->m_empty);
			msx_cpu_setbank (machine, 8, drvstate->m_empty);
		}
	}
}

MSX_SLOT_WRITE(crossblaim)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	UINT8 block = val & 3;

	if (!block) block = 1;
	state->m_banks[0] = block;

	if (drvstate->m_state[0] == state)
	{
		slot_crossblaim_map (machine, state, 0);
	}
	if (drvstate->m_state[2] == state)
	{
		slot_crossblaim_map (machine, state, 2);
	}
	if (drvstate->m_state[3] == state)
	{
		slot_crossblaim_map (machine, state, 3);
	}
}

MSX_SLOT_INIT(korean80in1)
{
	int banks;

	if (size > 0x200000)
	{
		logerror ("korean-80in1: warning: truncating to 2mb\n");
		size = 0x200000;
	}
	banks = size / 0x2000;
	if (size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		logerror ("korean-80in1: error: must be a 2 power of 8kb\n");
		return 1;
	}
	state->m_type = SLOT_KOREAN_80IN1;
	state->m_mem = mem;
	state->m_size = size;
	state->m_bank_mask = banks - 1;

	return 0;
}

MSX_SLOT_RESET(korean80in1)
{
	int i;

	for (i=0; i<4; i++)
	{
		state->m_banks[i] = i;
	}
}

MSX_SLOT_MAP(korean80in1)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, drvstate->m_empty);
		msx_cpu_setbank (machine, 2, drvstate->m_empty);
		break;
	case 1:
		msx_cpu_setbank (machine, 3, state->m_mem + state->m_banks[0] * 0x2000);
		msx_cpu_setbank (machine, 4, state->m_mem + state->m_banks[1] * 0x2000);
		break;
	case 2:
		msx_cpu_setbank (machine, 5, state->m_mem + state->m_banks[2] * 0x2000);
		msx_cpu_setbank (machine, 6, state->m_mem + state->m_banks[3] * 0x2000);
		break;
	case 3:
		msx_cpu_setbank (machine, 7, drvstate->m_empty);
		msx_cpu_setbank (machine, 8, drvstate->m_empty);
	}
}

MSX_SLOT_WRITE(korean80in1)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	int bank;

	if (addr >= 0x4000 && addr < 0x4004)
	{
		bank = addr & 3;

		state->m_banks[bank] = val & state->m_bank_mask;
		if (bank <= 1)
		{
			slot_korean80in1_map (machine, state, 1);
		}
		else if (drvstate->m_state[2] == state)
		{
			slot_korean80in1_map (machine, state, 2);
		}
	}
}

MSX_SLOT_INIT(korean90in1)
{
	int banks;

	if (size > 0x100000)
	{
		logerror ("korean-90in1: warning: truncating to 1mb\n");
		size = 0x100000;
	}
	banks = size / 0x4000;
	if (size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		logerror ("korean-90in1: error: must be a 2 power of 16kb\n");
		return 1;
	}
	state->m_type = SLOT_KOREAN_90IN1;
	state->m_mem = mem;
	state->m_size = size;
	state->m_bank_mask = banks - 1;

	return 0;
}

MSX_SLOT_RESET(korean90in1)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	drvstate->m_korean90in1_bank = 0;
}

MSX_SLOT_MAP(korean90in1)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	UINT8 *mem;
	UINT8 mask = (drvstate->m_korean90in1_bank & 0xc0) == 0x80 ? 0x3e : 0x3f;
	mem = state->m_mem +
		((drvstate->m_korean90in1_bank & mask) & state->m_bank_mask) * 0x4000;

	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, drvstate->m_empty);
		msx_cpu_setbank (machine, 2, drvstate->m_empty);
		break;
	case 1:
		msx_cpu_setbank (machine, 3, mem);
		msx_cpu_setbank (machine, 4, mem + 0x2000);
		break;
	case 2:
		switch (drvstate->m_korean90in1_bank & 0xc0)
		{
		case 0x80: /* 32 kb mode */
			mem += 0x4000;
		default: /* ie. 0x00 and 0x40: same memory as page 1 */
			msx_cpu_setbank (machine, 5, mem);
			msx_cpu_setbank (machine, 6, mem + 0x2000);
			break;
		case 0xc0: /* same memory as page 1, but swap lower/upper 8kb */
			msx_cpu_setbank (machine, 5, mem + 0x2000);
			msx_cpu_setbank (machine, 6, mem);
			break;
		}
		break;
	case 3:
		msx_cpu_setbank (machine, 7, drvstate->m_empty);
		msx_cpu_setbank (machine, 8, drvstate->m_empty);
	}
}

MSX_SLOT_INIT(korean126in1)
{
	int banks;

	if (size > 0x400000)
	{
		logerror ("korean-126in1: warning: truncating to 4mb\n");
		size = 0x400000;
	}
	banks = size / 0x4000;
	if (size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		logerror ("korean-126in1: error: must be a 2 power of 16kb\n");
		return 1;
	}

	state->m_type = SLOT_KOREAN_126IN1;
	state->m_mem = mem;
	state->m_size = size;
	state->m_bank_mask = banks - 1;

	return 0;
}

MSX_SLOT_RESET(korean126in1)
{
	int i;

	for (i=0; i<2; i++) state->m_banks[i] = i;
}

MSX_SLOT_MAP(korean126in1)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	UINT8 *mem;

	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, drvstate->m_empty);
		msx_cpu_setbank (machine, 2, drvstate->m_empty);
		break;
	case 1:
		mem = state->m_mem + state->m_banks[0] * 0x4000;
		msx_cpu_setbank (machine, 3, mem);
		msx_cpu_setbank (machine, 4, mem + 0x2000);
		break;
	case 2:
		mem = state->m_mem + state->m_banks[1] * 0x4000;
		msx_cpu_setbank (machine, 5, mem);
		msx_cpu_setbank (machine, 6, mem + 0x2000);
		break;
	case 3:
		msx_cpu_setbank (machine, 7, drvstate->m_empty);
		msx_cpu_setbank (machine, 8, drvstate->m_empty);
	}
}

MSX_SLOT_WRITE(korean126in1)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	if (addr >= 0x4000 && addr < 0x4002)
	{
		int bank = addr & 1;
		state->m_banks[bank] = val & state->m_bank_mask;
		if (bank == 0)
		{
			slot_korean126in1_map (machine, state, 1);
		}
		else if (drvstate->m_state[2] == state)
		{
			slot_korean126in1_map (machine, state, 2);
		}
	}
}

MSX_SLOT_INIT(soundcartridge)
{
	UINT8 *p;

	p = auto_alloc_array(machine, UINT8, 0x20000);
	memset (p, 0, 0x20000);

	state->m_mem = p;
	state->m_size = 0x20000;
	state->m_bank_mask = 15;
	state->m_type = SLOT_SOUNDCARTRIDGE;

	return 0;
}

MSX_SLOT_RESET(soundcartridge)
{
	int i;

	for (i=0; i<4; i++)
	{
		state->m_banks[i] = i;
		state->m_cart.sccp.ram_mode[i] = 0;
		state->m_cart.sccp.banks_saved[i] = i;
	}
	state->m_cart.sccp.mode = 0;
	state->m_cart.sccp.scc_active = 0;
	state->m_cart.sccp.sccp_active = 0;
}

READ8_MEMBER(msx_state::soundcartridge_scc)
{
	int reg;


	if (offset >= 0x7e0)
	{
		return m_state[2]->m_mem[
				m_state[2]->m_banks[2] * 0x2000 + 0x1800 + offset];
	}

	reg = offset & 0xff;

	if (reg < 0x80)
	{
		return m_k051649->k051649_waveform_r (space, reg);
	}
	else if (reg < 0xa0)
	{
		/* nothing */
	}
	else if (reg < 0xc0)
	{
		/* read wave 5 */
		return m_k051649->k051649_waveform_r (space, 0x80 + (reg & 0x1f));
	}
	else if (reg < 0xe0)
	{
		return m_k051649->k051649_test_r (space, reg);
	}

	return 0xff;
}

READ8_MEMBER(msx_state::soundcartridge_sccp)
{
	int reg;

	if (offset >= 0x7e0)
	{
		return m_state[2]->m_mem[
				m_state[2]->m_banks[3] * 0x2000 + 0x1800 + offset];
	}

	reg = offset & 0xff;

	if (reg < 0xa0)
	{
		return m_k051649->k051649_waveform_r (space, reg);
	}
	else if (reg >= 0xc0 && reg < 0xe0)
	{
		return m_k051649->k051649_test_r (space, reg);
	}

	return 0xff;
}

MSX_SLOT_MAP(soundcartridge)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	address_space &space = drvstate->m_maincpu->space(AS_PROGRAM);
	switch (page)
	{
	case 0:
		msx_cpu_setbank (machine, 1, state->m_mem + state->m_banks[2] * 0x2000);
		msx_cpu_setbank (machine, 2, state->m_mem + state->m_banks[3] * 0x2000);
		break;
	case 1:
		msx_cpu_setbank (machine, 3, state->m_mem + state->m_banks[0] * 0x2000);
		msx_cpu_setbank (machine, 4, state->m_mem + state->m_banks[1] * 0x2000);
		break;
	case 2:
		msx_cpu_setbank (machine, 5, state->m_mem + state->m_banks[2] * 0x2000);
		msx_cpu_setbank (machine, 6, state->m_mem + state->m_banks[3] * 0x2000);
		if (state->m_cart.sccp.scc_active) {
			space.install_read_handler(0x9800, 0x9fff, read8_delegate(FUNC(msx_state::soundcartridge_scc),drvstate));
		} else {
			space.install_read_bank(0x9800, 0x9fff, "bank7");
		}
		if (state->m_cart.sccp.scc_active) {
			space.install_read_handler(0xb800, 0xbfff, read8_delegate(FUNC(msx_state::soundcartridge_sccp),drvstate));
		} else {
			space.install_read_bank(0xb800, 0xbfff, "bank9");
		}
		break;
	case 3:
		msx_cpu_setbank (machine, 7, state->m_mem + state->m_banks[0] * 0x2000);
		msx_cpu_setbank (machine, 8, state->m_mem + state->m_banks[1] * 0x2000);
		break;
	}
}

MSX_SLOT_WRITE(soundcartridge)
{
	msx_state *drvstate = machine.driver_data<msx_state>();
	address_space &space = drvstate->m_maincpu->space(AS_PROGRAM);
	int i;

	if (addr < 0x4000)
	{
		return;
	}
	else if (addr < 0x6000)
	{
		if (state->m_cart.sccp.ram_mode[0])
		{
			state->m_mem[state->m_banks[0] * 0x2000 + (addr & 0x1fff)] = val;
		}
		else if (addr >= 0x5000 && addr < 0x5800)
		{
			state->m_banks[0] = val & state->m_bank_mask;
			state->m_cart.sccp.banks_saved[0] = val;
			slot_soundcartridge_map (machine, state, 1);
			if (drvstate->m_state[3] == state)
			{
				slot_soundcartridge_map (machine, state, 3);
			}
		}
	}
	else if (addr < 0x8000)
	{
		if (state->m_cart.sccp.ram_mode[1])
		{
			state->m_mem[state->m_banks[1] * 0x2000 + (addr & 0x1fff)] = val;
		}
		else if (addr >= 0x7000 && addr < 0x7800)
		{
			state->m_banks[1] = val & state->m_bank_mask;
			state->m_cart.sccp.banks_saved[1] = val;
			if (drvstate->m_state[3] == state)
			{
				slot_soundcartridge_map (machine, state, 3);
			}
			slot_soundcartridge_map (machine, state, 1);
		}
	}
	else if (addr < 0xa000)
	{
		if (state->m_cart.sccp.ram_mode[2])
		{
			state->m_mem[state->m_banks[2] * 0x2000 + (addr & 0x1fff)] = val;
		}
		else if (addr >= 0x9000 && addr < 0x9800)
		{
			state->m_banks[2] = val & state->m_bank_mask;
			state->m_cart.sccp.banks_saved[2] = val;
			state->m_cart.sccp.scc_active =
					(((val & 0x3f) == 0x3f) && !(state->m_cart.sccp.mode & 0x20));

			slot_soundcartridge_map (machine, state, 2);
			if (drvstate->m_state[0] == state)
			{
				slot_soundcartridge_map (machine, state, 0);
			}
		}
		else if (addr >= 0x9800 && state->m_cart.sccp.scc_active)
		{
			int offset = addr & 0xff;

			if (offset < 0x80)
			{
				drvstate->m_k051649->k051649_waveform_w (space, offset, val);
			}
			else if (offset < 0xa0)
			{
				offset &= 0xf;

				if (offset < 0xa)
				{
					drvstate->m_k051649->k051649_frequency_w (space, offset, val);
				}
				else if (offset < 0x0f)
				{
					drvstate->m_k051649->k051649_volume_w (space, offset - 0xa, val);
				}
				else if (offset == 0x0f)
				{
					drvstate->m_k051649->k051649_keyonoff_w (space, 0, val);
				}
			}
			else if (offset < 0xe0)
			{
				drvstate->m_k051649->k051649_test_w (space, offset, val);
			}
		}
	}
	else if (addr < 0xbffe)
	{
		if (state->m_cart.sccp.ram_mode[3])
		{
			state->m_mem[state->m_banks[3] * 0x2000 + (addr & 0x1fff)] = val;
		}
		else if (addr >= 0xb000 && addr < 0xb800)
		{
			state->m_cart.sccp.banks_saved[3] = val;
			state->m_banks[3] = val & state->m_bank_mask;
			state->m_cart.sccp.sccp_active =
					(val & 0x80) && (state->m_cart.sccp.mode & 0x20);
			slot_soundcartridge_map (machine, state, 2);
			if (drvstate->m_state[0] == state)
			{
				slot_soundcartridge_map (machine, state, 0);
			}
		}
		else if (addr >= 0xb800 && state->m_cart.sccp.sccp_active)
		{
			int offset = addr & 0xff;

			if (offset < 0xa0)
			{
				drvstate->m_k051649->k051649_waveform_w (space, offset, val);
			}
			else if (offset < 0xc0)
			{
				offset &= 0x0f;

				if (offset < 0x0a)
				{
					drvstate->m_k051649->k051649_frequency_w (space, offset, val);
				}
				else if (offset < 0x0f)
				{
					drvstate->m_k051649->k051649_volume_w (space, offset - 0x0a, val);
				}
				else if (offset == 0x0f)
				{
					drvstate->m_k051649->k051649_keyonoff_w (space, 0, val);
				}
			}
			else if (offset < 0xe0)
			{
				drvstate->m_k051649->k051649_test_w (space, offset, val);
			}
		}
	}
	else if (addr < 0xc000)
	{
		/* write to mode register */
		if ((state->m_cart.sccp.mode ^ val) & 0x20)
		{
			logerror ("soundcartrige: changed to %s mode\n",
							val & 0x20 ? "scc+" : "scc");
		}
		state->m_cart.sccp.mode = val;
		if (val & 0x10)
		{
			/* all ram mode */
			for (i=0; i<4; i++)
			{
				state->m_cart.sccp.ram_mode[i] = 1;
			}
		}
		else
		{
			state->m_cart.sccp.ram_mode[0] = val & 1;
			state->m_cart.sccp.ram_mode[1] = val & 2;
			state->m_cart.sccp.ram_mode[2] = (val & 4) && (val & 0x20);
			state->m_cart.sccp.ram_mode[3] = 0;

		}

		state->m_cart.sccp.scc_active =
			(((state->m_cart.sccp.banks_saved[2] & 0x3f) == 0x3f) &&
			!(val & 0x20));

		state->m_cart.sccp.sccp_active =
				((state->m_cart.sccp.banks_saved[3] & 0x80) && (val & 0x20));

		slot_soundcartridge_map (machine, state, 2);
	}
}

MSX_SLOT_START
	MSX_SLOT_ROM (SLOT_EMPTY, empty)
	MSX_SLOT (SLOT_MSXDOS2, msxdos2)
	MSX_SLOT (SLOT_KONAMI_SCC, konami_scc)
	MSX_SLOT (SLOT_KONAMI, konami)
	MSX_SLOT (SLOT_ASCII8, ascii8)
	MSX_SLOT (SLOT_ASCII16, ascii16)
	MSX_SLOT_SRAM (SLOT_GAMEMASTER2, gmaster2)
	MSX_SLOT_SRAM (SLOT_ASCII8_SRAM, ascii8_sram)
	MSX_SLOT_SRAM (SLOT_ASCII16_SRAM, ascii16_sram)
	MSX_SLOT (SLOT_RTYPE, rtype)
	MSX_SLOT (SLOT_MAJUTSUSHI, majutsushi)
	MSX_SLOT_SRAM (SLOT_FMPAC, fmpac)
	MSX_SLOT_ROM (SLOT_SUPERLODERUNNER, superloderunner)
	MSX_SLOT (SLOT_SYNTHESIZER, synthesizer)
	MSX_SLOT (SLOT_CROSS_BLAIM, crossblaim)
	MSX_SLOT (SLOT_DISK_ROM, diskrom)
	MSX_SLOT (SLOT_KOREAN_80IN1, korean80in1)
	MSX_SLOT (SLOT_KOREAN_126IN1, korean126in1)
	MSX_SLOT_ROM (SLOT_KOREAN_90IN1, korean90in1)
	MSX_SLOT (SLOT_SOUNDCARTRIDGE, soundcartridge)
	MSX_SLOT_ROM (SLOT_ROM, rom)
	MSX_SLOT_RAM (SLOT_RAM, ram)
	MSX_SLOT_RAM (SLOT_RAM_MM, rammm)
	MSX_SLOT_NULL (SLOT_CARTRIDGE1)
	MSX_SLOT_NULL (SLOT_CARTRIDGE2)
	MSX_SLOT (SLOT_DISK_ROM2, diskrom2)
MSX_SLOT_END
