// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  apple2.c

  Machine file to handle emulation of the Apple II series.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/apple2.h"
#include "machine/applefdc.h"
#include "machine/sonydriv.h"
#include "machine/appldriv.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cassette.h"
#include "sound/speaker.h"
#include "debugger.h"

#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif /* MAME_DEBUG */

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

#define PROFILER_C00X   PROFILER_USER2
#define PROFILER_C01X   PROFILER_USER2
#define PROFILER_C08X   PROFILER_USER2
#define PROFILER_A2INT  PROFILER_USER2

/* -----------------------------------------------------------------------
 * New Apple II memory manager
 * ----------------------------------------------------------------------- */

READ8_MEMBER(apple2_state::read_floatingbus)
{
	return apple2_getfloatingbusvalue();
}



void apple2_state::apple2_setup_memory(const apple2_memmap_config *config)
{
	m_mem_config = *config;
	m_current_meminfo = nullptr;
	apple2_update_memory();
}



void apple2_state::apple2_update_memory()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	int i, bank;
	char rbank[10], wbank[10];
	int full_update = 0;
	apple2_meminfo meminfo;
	read8_delegate *rh;
	write8_delegate *wh;
	offs_t begin, end_r, end_w;
	UINT8 *rbase, *wbase;
	UINT32 offset;
	bank_disposition_t bank_disposition;
	int wh_nop = 0;

	/* need to build list of current info? */
	if (!m_current_meminfo)
	{
		for (i = 0; m_mem_config.memmap[i].end; i++)
			;
		m_current_meminfo = std::make_unique<apple2_meminfo[]>(i);
		full_update = 1;
	}

	/* loop through the entire memory map */
	bank = m_mem_config.first_bank;
	for (i = 0; m_mem_config.memmap[i].get_meminfo; i++)
	{
		/* retrieve information on this entry */
		memset(&meminfo, 0, sizeof(meminfo));
		m_mem_config.memmap[i].get_meminfo(machine(),m_mem_config.memmap[i].begin, m_mem_config.memmap[i].end, &meminfo);

		bank_disposition = m_mem_config.memmap[i].bank_disposition;

		/* do we need to memory reading? */
		if (full_update
			|| (meminfo.read_mem != m_current_meminfo[i].read_mem)
			|| (meminfo.read_handler != m_current_meminfo[i].read_handler))
		{
			rbase = nullptr;
			sprintf(rbank,"bank%d",bank);
			begin = m_mem_config.memmap[i].begin;
			end_r = m_mem_config.memmap[i].end;
			rh = nullptr;

			LOG(("apple2_update_memory():  Updating RD {%06X..%06X} [#%02d] --> %08X\n",
				begin, end_r, bank, meminfo.read_mem));

			/* read handling */
			if (meminfo.read_handler)
			{
				/* handler */
				rh = meminfo.read_handler;
			}
			else if (meminfo.read_mem == APPLE2_MEM_FLOATING)
			{
				/* floating RAM */
				rh = &read_delegates_master[0];
			}
			else if ((meminfo.read_mem & 0xC0000000) == APPLE2_MEM_AUX)
			{
				/* auxillary memory */
				assert(m_mem_config.auxmem);
				offset = meminfo.read_mem & APPLE2_MEM_MASK;
				rbase = &m_mem_config.auxmem[offset];
			}
			else if ((meminfo.read_mem & 0xC0000000) == APPLE2_MEM_SLOT)
			{
				// slots 1-2
				if ((meminfo.write_mem & APPLE2_MEM_MASK) == 0)
				{
					rh = &read_delegates_master[1];
				}
				else if ((meminfo.write_mem & APPLE2_MEM_MASK) == 0x200)
				{   // slot 3
					rh = &read_delegates_master[2];
				}
				else if ((meminfo.write_mem & APPLE2_MEM_MASK) == 0x300)
				{   // slots 4-7
					rh = &read_delegates_master[3];
				}
				else
				{
					printf("ERROR: Unhandled case for APPLE2_MEM_SLOT write_mem = %x!\n", (meminfo.write_mem & APPLE2_MEM_MASK));
				}
			}
			else if ((meminfo.read_mem & 0xC0000000) == APPLE2_MEM_ROM)
			{
				/* ROM */
				offset = meminfo.read_mem & APPLE2_MEM_MASK;
				rbase = &m_rom[offset % m_rom_length];
			}
			else
			{
				/* RAM */
				if (end_r >= m_ram->size())
					end_r = m_ram->size() - 1;
				offset = meminfo.read_mem & APPLE2_MEM_MASK;
				if (end_r >= begin)
					rbase = &m_ram->pointer()[offset];
			}

			/* install the actual handlers */
			if (begin <= end_r) {
				if (rh) {
					space.install_read_handler(begin, end_r, *rh);
				} else {
					space.install_read_bank(begin, end_r, rbank);
				}
			}

			/* did we 'go past the end?' */
			if (end_r < m_mem_config.memmap[i].end)
				space.nop_read(end_r + 1, m_mem_config.memmap[i].end);

			/* set the memory bank */
			if (rbase)
			{
				membank(rbank)->set_base(rbase);
			}

			/* record the current settings */
			m_current_meminfo[i].read_mem = meminfo.read_mem;
			m_current_meminfo[i].read_handler = meminfo.read_handler;
		}

		/* do we need to memory writing? */
		if (full_update
			|| (meminfo.write_mem != m_current_meminfo[i].write_mem)
			|| (meminfo.write_handler != m_current_meminfo[i].write_handler))
		{
			wbase = nullptr;
			if (bank_disposition == A2MEM_MONO)
				sprintf(wbank,"bank%d",bank);
			else if (bank_disposition == A2MEM_DUAL)
				sprintf(wbank,"bank%d",bank+1);
			begin = m_mem_config.memmap[i].begin;
			end_w = m_mem_config.memmap[i].end;
			wh = nullptr;

			LOG(("apple2_update_memory():  Updating WR {%06X..%06X} [#%02d] --> %08X\n",
				begin, end_w, bank, meminfo.write_mem));

			/* write handling */
			if (meminfo.write_handler)
			{
				/* handler */
				wh = meminfo.write_handler;
			}
			else if ((meminfo.write_mem & 0xC0000000) == APPLE2_MEM_AUX)
			{
				/* auxillary memory */
				assert(m_mem_config.auxmem);
				offset = meminfo.write_mem & APPLE2_MEM_MASK;
				wbase = &m_mem_config.auxmem[offset];
			}
			else if ((meminfo.write_mem & 0xC0000000) == APPLE2_MEM_SLOT)
			{
				/* slot RAM/ROM */

				// slots 1-2
				if ((meminfo.write_mem & APPLE2_MEM_MASK) == 0)
				{
					wh = &write_delegates_master[0];
				}
				else if ((meminfo.write_mem & APPLE2_MEM_MASK) == 0x200)
				{   // slot 3
					wh = &write_delegates_master[1];
				}
				else if ((meminfo.write_mem & APPLE2_MEM_MASK) == 0x300)
				{   // slots 4-7
					wh = &write_delegates_master[2];
				}
			}
			else if ((meminfo.write_mem & 0xC0000000) == APPLE2_MEM_ROM)
			{
				/* ROM */
				wh_nop = 1;
			}
			else
			{
				/* RAM */
				if (end_w >= m_ram->size())
					end_w = m_ram->size() - 1;
				offset = meminfo.write_mem & APPLE2_MEM_MASK;
				if (end_w >= begin)
					wbase = &m_ram->pointer()[offset];
			}


			/* install the actual handlers */
			if (begin <= end_w) {
				if (wh) {
					space.install_write_handler(begin, end_w, *wh);
				} else {
					if (wh_nop) {
						space.nop_write(begin, end_w);
					} else {
						space.install_write_bank(begin, end_w, wbank);
					}
				}
			}

			/* did we 'go past the end?' */
			if (end_w < m_mem_config.memmap[i].end)
				space.nop_write(end_w + 1, m_mem_config.memmap[i].end);

			/* set the memory bank */
			if (wbase)
			{
				membank(wbank)->set_base(wbase);
			}

			/* record the current settings */
			m_current_meminfo[i].write_mem = meminfo.write_mem;
			m_current_meminfo[i].write_handler = meminfo.write_handler;
		}
		bank += bank_disposition;
	}
}



void apple2_state::apple2_update_memory_postload()
{
	apple2_update_memory();
}



/* -----------------------------------------------------------------------
 * Apple II memory map
 * ----------------------------------------------------------------------- */

READ8_MEMBER(apple2_state::apple2_c0xx_r)
{
	if(!space.debugger_access())
	{
		read8_delegate handlers[] =
		{
			read8_delegate(FUNC(apple2_state::apple2_c00x_r), this),
			read8_delegate(FUNC(apple2_state::apple2_c01x_r), this),
			read8_delegate(FUNC(apple2_state::apple2_c02x_r), this),
			read8_delegate(FUNC(apple2_state::apple2_c03x_r), this),
			read8_delegate(FUNC(apple2_state::apple2_c03x_r), this),   // not called here, handled elsewhere
			read8_delegate(FUNC(apple2_state::apple2_c05x_r), this),
			read8_delegate(FUNC(apple2_state::apple2_c06x_r), this),
			read8_delegate(FUNC(apple2_state::apple2_c07x_r), this)
		};

		offset &= 0x7F;

		/* normal handler */
		if (offset / 0x10 != 4)
		{
			return handlers[offset / 0x10](space, offset % 0x10, 0);
		}
	}

	return 0;
}



WRITE8_MEMBER(apple2_state::apple2_c0xx_w)
{
	write8_delegate handlers[] =
	{
		write8_delegate(FUNC(apple2_state::apple2_c00x_w), this),
		write8_delegate(FUNC(apple2_state::apple2_c01x_w), this),
		write8_delegate(FUNC(apple2_state::apple2_c02x_w), this),
		write8_delegate(FUNC(apple2_state::apple2_c03x_w), this),
		write8_delegate(FUNC(apple2_state::apple2_c03x_w), this),      // unused
		write8_delegate(FUNC(apple2_state::apple2_c05x_w), this),
		write8_delegate(FUNC(apple2_state::apple2_c05x_w), this),      // unused
		write8_delegate(FUNC(apple2_state::apple2_c07x_w), this)
	};

	offset &= 0x7F;

	/* normal handler */
	if (((offset / 0x10) != 4) && (offset / 0x10) != 6)
	{
		handlers[offset / 0x10](space, offset % 0x10, data, 0);
	}
}

READ8_MEMBER(apple2_state::apple2_c080_r)
{
	if(!space.debugger_access())
	{
		device_a2bus_card_interface *slotdevice;
		int slot;

		offset &= 0x7F;
		slot = offset / 0x10;

		if ((m_machinetype == APPLE_IIC) || (m_machinetype == APPLE_IICPLUS) || (m_machinetype == LASER128))
		{
			if (slot == 1)
			{
				offset &= 0xf;
				if (offset >= 8 && offset <= 0xb)
				{
					return m_acia1->read(space, offset-8);
				}
			}
			else if (slot == 2)
			{
				offset &= 0xf;
				if (offset >= 8 && offset <= 0xb)
				{
					return m_acia2->read(space, offset-8);
				}
			}
		}

		if ((m_machinetype == APPLE_IICPLUS) && (slot == 6))
		{
			offset &= 0xf;
			return m_iicpiwm->read(offset);
		}

		if ((m_machinetype == LASER128) && (slot == 5))
		{
			offset &= 0xf;
			UINT8 retval = m_exp_regs[offset];

			if (offset == 3)
			{
				retval = m_exp_ram[m_exp_liveptr&m_exp_addrmask];
				m_exp_liveptr++;
				m_exp_regs[0] = m_exp_liveptr & 0xff;
				m_exp_regs[1] = (m_exp_liveptr>>8) & 0xff;
				m_exp_regs[2] = ((m_exp_liveptr>>16) & 0xff) | m_exp_bankhior;
			}

			return retval;
		}

		if ((m_machinetype == LASER128) && (slot == 6))
		{
			offset &= 0xf;
			return m_laserudc->read(offset);
		}

		/* now identify the device */
		slotdevice = m_a2bus->get_a2bus_card(slot);

		/* and if we can, read from the slot */
		if (slotdevice != nullptr)
		{
			return slotdevice->read_c0nx(space, offset % 0x10);
		}
	}

	return 0;
}


WRITE8_MEMBER(apple2_state::apple2_c080_w)
{
	device_a2bus_card_interface *slotdevice;
	int slot;

	offset &= 0x7F;
	slot = offset / 0x10;

	if ((m_machinetype == APPLE_IIC) || (m_machinetype == APPLE_IICPLUS) || (m_machinetype == LASER128))
	{
		if (slot == 1)
		{
			offset &= 0xf;
			if (offset >= 8 && offset <= 0xb)
			{
				m_acia1->write(space, offset-8, data);
				return;
			}
		}
		else if (slot == 2)
		{
			offset &= 0xf;
			if (offset >= 8 && offset <= 0xb)
			{
				m_acia2->write(space, offset-8, data);
				return;
			}
		}
	}

	if ((m_machinetype == APPLE_IICPLUS) && (slot == 6))
	{
		offset &= 0xf;
		m_iicpiwm->write(offset, data);
		return;
	}

	if ((m_machinetype == LASER128) && (slot == 5))
	{
		switch (offset & 0xf)
		{
			case 0:
				m_exp_wptr &= ~0xff;
				m_exp_wptr |= data;
				m_exp_regs[0] = m_exp_wptr & 0xff;
				m_exp_regs[1] = (m_exp_wptr>>8) & 0xff;
				m_exp_regs[2] = ((m_exp_wptr>>16) & 0xff) | m_exp_bankhior;
				m_exp_liveptr = m_exp_wptr;
				break;

			case 1:
				m_exp_wptr &= ~0xff00;
				m_exp_wptr |= (data<<8);
				m_exp_regs[0] = m_exp_wptr & 0xff;
				m_exp_regs[1] = (m_exp_wptr>>8) & 0xff;
				m_exp_regs[2] = ((m_exp_wptr>>16) & 0xff) | m_exp_bankhior;
				m_exp_liveptr = m_exp_wptr;
				break;

			case 2:
				m_exp_wptr &= ~0xff0000;
				m_exp_wptr |= (data<<16);
				m_exp_regs[0] = m_exp_wptr & 0xff;
				m_exp_regs[1] = (m_exp_wptr>>8) & 0xff;
				m_exp_regs[2] = ((m_exp_wptr>>16) & 0xff) | m_exp_bankhior;
				m_exp_liveptr = m_exp_wptr;
				break;

			case 3:
	//            printf("Write %02x to RAM[%x]\n", data, m_liveptr);
				m_exp_ram[(m_exp_liveptr&m_exp_addrmask)] = data;
				m_exp_liveptr++;
				m_exp_regs[0] = m_exp_liveptr & 0xff;
				m_exp_regs[1] = (m_exp_liveptr>>8) & 0xff;
				m_exp_regs[2] = ((m_exp_liveptr>>16) & 0xff) | m_exp_bankhior;
				break;

			default:
				m_exp_regs[offset] = data;
				break;
		}
	}

	if ((m_machinetype == LASER128) && (slot == 6))
	{
		offset &= 0xf;
		m_laserudc->write(space, offset, data);
		return;
	}

	/* now identify the device */
	slotdevice = m_a2bus->get_a2bus_card(slot);

	/* and if we can, write to the slot */
	if (slotdevice != nullptr)
	{
		slotdevice->write_c0nx(space, offset % 0x10, data);
	}
}

/* returns default CnXX slotram for a slot space */
INT8 apple2_state::apple2_slotram_r(address_space &space, int slotnum, int offset)
{
	if (m_slot_ram)
	{
		if (!space.debugger_access())
		{
//          printf("slotram_r: taking cnxx_slot to -1\n");
			m_a2_cnxx_slot = -1;
			apple2_update_memory();
		}

		return m_slot_ram[offset];
	}

	// else fall through to floating bus
	return apple2_getfloatingbusvalue();
}

READ8_MEMBER(apple2_state::apple2_c1xx_r )
{
	int slotnum;
	device_a2bus_card_interface *slotdevice;

	slotnum = ((offset>>8) & 0xf) + 1;
	slotdevice = m_a2bus->get_a2bus_card(slotnum);

	if (slotdevice != nullptr)
	{
		if ((slotdevice->take_c800()) && (!space.debugger_access()))
		{
//          printf("c1xx_r: taking cnxx_slot to %d\n", slotnum);
			m_a2_cnxx_slot = slotnum;
			apple2_update_memory();
		}

		return slotdevice->read_cnxx(space, offset&0xff);
	}
	else
	{
		return apple2_slotram_r(space, slotnum, offset);
	}

	// else fall through to floating bus
	// never executed
	//return apple2_getfloatingbusvalue();
}

WRITE8_MEMBER(apple2_state::apple2_c1xx_w )
{
	int slotnum;
	device_a2bus_card_interface *slotdevice;

	slotnum = ((offset>>8) & 0xf) + 1;

	slotdevice = m_a2bus->get_a2bus_card(slotnum);

	if (slotdevice != nullptr)
	{
		slotdevice->write_cnxx(space, offset&0xff, data);
	}
	else
	{
		if (m_slot_ram)
			m_slot_ram[offset] = data;
	}
}

READ8_MEMBER(apple2_state::apple2_c3xx_r )
{
	int slotnum;
	device_a2bus_card_interface *slotdevice;

	slotnum = 3;
	slotdevice = m_a2bus->get_a2bus_card(slotnum);

	// is a card installed in this slot?
	if (slotdevice != nullptr)
	{
		if ((slotdevice->take_c800()) && (!space.debugger_access()))
		{
//          printf("c3xx_r: taking cnxx_slot to %d\n", slotnum);
			m_a2_cnxx_slot = slotnum;
			apple2_update_memory();
		}
		return slotdevice->read_cnxx(space, offset&0xff);
	}
	else
	{
		return apple2_slotram_r(space, slotnum, offset);
	}

	// else fall through to floating bus
	// never executed
	//return apple2_getfloatingbusvalue();
}

WRITE8_MEMBER(apple2_state::apple2_c3xx_w )
{
	int slotnum;
	device_a2bus_card_interface *slotdevice;

	slotnum = 3;
	slotdevice = m_a2bus->get_a2bus_card(slotnum);

	if (slotdevice != nullptr)
	{
		if ((slotdevice->take_c800()) && (!space.debugger_access()))
		{
//          printf("c3xx_w: taking cnxx_slot to %d\n", slotnum);
			m_a2_cnxx_slot = slotnum;
			apple2_update_memory();
		}
		slotdevice->write_cnxx(space, offset&0xff, data);
	}
	else
	{
		if (m_slot_ram)
			m_slot_ram[offset] = data;
	}
}

READ8_MEMBER(apple2_state::apple2_c4xx_r )
{
	int slotnum;
	device_a2bus_card_interface *slotdevice;

	slotnum = ((offset>>8) & 0xf) + 4;
	slotdevice = m_a2bus->get_a2bus_card(slotnum);

	// is a card installed in this slot?
	if (slotdevice != nullptr)
	{
		if (slotdevice->take_c800() && (m_a2_cnxx_slot != slotnum) && (!space.debugger_access()))
		{
			m_a2_cnxx_slot = slotnum;
			apple2_update_memory();
		}
		return slotdevice->read_cnxx(space, offset&0xff);
	}
	else
	{
		return apple2_slotram_r(space, slotnum, offset);
	}

	// else fall through to floating bus
	// never executed
	//return apple2_getfloatingbusvalue();
}

WRITE8_MEMBER ( apple2_state::apple2_c4xx_w )
{
	int slotnum;
	device_a2bus_card_interface *slotdevice;

	slotnum = ((offset>>8) & 0xf) + 4;
	slotdevice = m_a2bus->get_a2bus_card(slotnum);

	if (slotdevice != nullptr)
	{
		if ((slotdevice->take_c800()) && (!space.debugger_access()))
		{
//          printf("c4xx_w: taking cnxx_slot to %d\n", slotnum);
			m_a2_cnxx_slot = slotnum;
			apple2_update_memory();
		}
		slotdevice->write_cnxx(space, offset&0xff, data);
	}
	else
	{
		if (m_slot_ram)
			m_slot_ram[offset] = data;
	}
}

READ8_MEMBER(apple2_state::apple2_cfff_r)
{
	// debugger guard
	if (!space.debugger_access())
	{
//      printf("cfff_r: taking cnxx_slot to -1\n");
		m_a2_cnxx_slot = -1;
		apple2_update_memory();
	}

	return apple2_getfloatingbusvalue();
}

WRITE8_MEMBER(apple2_state::apple2_cfff_w)
{
	if (!space.debugger_access())
	{
//      printf("cfff_w: taking cnxx_slot to -1\n");
		m_a2_cnxx_slot = -1;
		apple2_update_memory();
	}
}

READ8_MEMBER(apple2_state::apple2_c800_r )
{
	device_a2bus_card_interface *slotdevice;

	slotdevice = m_a2bus->get_a2bus_card(m_a2_cnxx_slot);

	if (slotdevice != nullptr)
	{
		return slotdevice->read_c800(space, offset&0xfff);
	}

	return apple2_getfloatingbusvalue();
}

WRITE8_MEMBER(apple2_state::apple2_c800_w )
{
	device_a2bus_card_interface *slotdevice;

	slotdevice = m_a2bus->get_a2bus_card(m_a2_cnxx_slot);

	if (slotdevice != nullptr)
	{
		slotdevice->write_c800(space, offset&0xfff, data);
	}
}

READ8_MEMBER(apple2_state::apple2_ce00_r )
{
	device_a2bus_card_interface *slotdevice;

	slotdevice = m_a2bus->get_a2bus_card(m_a2_cnxx_slot);

	if (slotdevice != nullptr)
	{
		return slotdevice->read_c800(space, (offset&0xfff) + 0x600);
	}

	return apple2_getfloatingbusvalue();
}

WRITE8_MEMBER(apple2_state::apple2_ce00_w )
{
	device_a2bus_card_interface *slotdevice;

	slotdevice = m_a2bus->get_a2bus_card(m_a2_cnxx_slot);

	if (slotdevice != nullptr)
	{
		slotdevice->write_c800(space, (offset&0xfff)+0x600, data);
	}
}

READ8_MEMBER(apple2_state::apple2_inh_d000_r )
{
	device_a2bus_card_interface *slotdevice;

	slotdevice = m_a2bus->get_a2bus_card(m_inh_slot);

	if (slotdevice != nullptr)
	{
		return slotdevice->read_inh_rom(space, offset & 0xfff);
	}

	return apple2_getfloatingbusvalue();
}

WRITE8_MEMBER(apple2_state::apple2_inh_d000_w )
{
	device_a2bus_card_interface *slotdevice;

	slotdevice = m_a2bus->get_a2bus_card(m_inh_slot);

	if (slotdevice != nullptr)
	{
		return slotdevice->write_inh_rom(space, offset & 0xfff, data);
	}
}

READ8_MEMBER(apple2_state::apple2_inh_e000_r )
{
	device_a2bus_card_interface *slotdevice;

	slotdevice = m_a2bus->get_a2bus_card(m_inh_slot);

	if (slotdevice != nullptr)
	{
		return slotdevice->read_inh_rom(space, (offset & 0x1fff) + 0x1000);
	}

	return apple2_getfloatingbusvalue();
}

WRITE8_MEMBER(apple2_state::apple2_inh_e000_w )
{
	device_a2bus_card_interface *slotdevice;

	slotdevice = m_a2bus->get_a2bus_card(m_inh_slot);

	if (slotdevice != nullptr)
	{
		slotdevice->write_inh_rom(space, (offset & 0x1fff) + 0x1000, data);
	}
}

static void apple2_mem_0000(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();
	meminfo->read_handler       = (state->m_flags & VAR_ALTZP)  ? &state->read_delegates_0000[0] : &state->read_delegates_0000[1];
	meminfo->write_handler      = (state->m_flags & VAR_ALTZP)  ? &state->write_delegates_0000[0] : &state->write_delegates_0000[1];
}

static void apple2_mem_0200(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();
	meminfo->read_handler       = (state->m_flags & VAR_RAMRD)  ? &state->read_delegates_0200[0] : &state->read_delegates_0200[1];
	meminfo->write_handler      = (state->m_flags & VAR_RAMWRT) ? &state->write_delegates_0200[0] : &state->write_delegates_0200[1];
}

static void apple2_mem_0400(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();

	if (state->m_flags & VAR_80STORE)
	{
		meminfo->read_handler   = (state->m_flags & VAR_PAGE2)  ? &state->read_delegates_0400[0] : &state->read_delegates_0400[1];
		meminfo->write_handler  = (state->m_flags & VAR_PAGE2)  ? &state->write_delegates_0400[0] : &state->write_delegates_0400[1];
	}
	else
	{
		meminfo->read_handler   = (state->m_flags & VAR_RAMRD)  ? &state->read_delegates_0400[0] : &state->read_delegates_0400[1];
		meminfo->write_handler  = (state->m_flags & VAR_RAMWRT) ? &state->write_delegates_0400[0] : &state->write_delegates_0400[1];
	}
}

static void apple2_mem_0800(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();
	meminfo->read_handler       = (state->m_flags & VAR_RAMRD)  ? &state->read_delegates_0800[0] : &state->read_delegates_0800[1];
	meminfo->write_handler      = (state->m_flags & VAR_RAMWRT) ? &state->write_delegates_0800[0] : &state->write_delegates_0800[1];
}

static void apple2_mem_2000(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();
	if ((state->m_flags & (VAR_80STORE|VAR_HIRES)) == (VAR_80STORE|VAR_HIRES))
	{
		meminfo->read_handler   = (state->m_flags & VAR_PAGE2)  ? &state->read_delegates_2000[0] : &state->read_delegates_2000[1];
		meminfo->write_handler  = (state->m_flags & VAR_PAGE2)  ? &state->write_delegates_2000[0] : &state->write_delegates_2000[1];
	}
	else
	{
		meminfo->read_handler   = (state->m_flags & VAR_RAMRD)  ? &state->read_delegates_2000[0] : &state->read_delegates_2000[1];
		meminfo->write_handler  = (state->m_flags & VAR_RAMWRT) ? &state->write_delegates_2000[0] : &state->write_delegates_2000[1];
	}
}

static void apple2_mem_4000(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();
	meminfo->read_handler       = (state->m_flags & VAR_RAMRD)  ? &state->read_delegates_4000[0] : &state->read_delegates_4000[1];
	meminfo->write_handler      = (state->m_flags & VAR_RAMWRT) ? &state->write_delegates_4000[0] : &state->write_delegates_4000[1];
}

static void apple2_mem_C000(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();
	meminfo->read_handler = &state->rd_c000;
	meminfo->write_handler = &state->wd_c000;
}

static void apple2_mem_C080(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();

	meminfo->read_handler = &state->rd_c080;
	meminfo->write_handler = &state->wd_c080;
}

static void tk2000_mem_C100(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();

	if (state->m_flags & VAR_TK2000RAM)
	{
		meminfo->read_mem = 0x00C100;
		meminfo->write_mem = 0x00C100;
	}
	else
	{
		meminfo->read_mem = (begin & 0x3fff) | APPLE2_MEM_ROM;
		meminfo->write_mem = 0x00C100;
	}
}

static void apple2_mem_CFFF(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();

	meminfo->read_handler = &state->rd_cfff;
	meminfo->write_handler = &state->wd_cfff;
}

static void apple2_mem_Cx00(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();
	if (state->m_flags & VAR_INTCXROM)
	{
		meminfo->read_mem       = (begin & 0x0FFF) | (state->m_flags & VAR_ROMSWITCH ? 0x4000 : 0x0000) | APPLE2_MEM_ROM;
		meminfo->write_mem      = APPLE2_MEM_FLOATING;
	}
	else
	{
		meminfo->read_mem       = ((begin & 0x0FFF) - 0x100) | APPLE2_MEM_SLOT;
		meminfo->write_mem      = ((begin & 0x0FFF) - 0x100) | APPLE2_MEM_SLOT;
	}
}

static void apple2_mem_C300(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();

	if (((state->m_flags & (VAR_INTCXROM|VAR_SLOTC3ROM)) != VAR_SLOTC3ROM) && !(state->m_flags_mask & VAR_SLOTC3ROM))
	{
		meminfo->read_mem       = (begin & 0x0FFF) | (state->m_flags & VAR_ROMSWITCH ? 0x4000 : 0x0000) | APPLE2_MEM_ROM;
		meminfo->write_mem      = APPLE2_MEM_FLOATING;
	}
	else
	{
		meminfo->read_mem       = ((begin & 0x0FFF) - 0x100) | APPLE2_MEM_SLOT;
		meminfo->write_mem      = ((begin & 0x0FFF) - 0x100) | APPLE2_MEM_SLOT;
	}
}

static void apple2_mem_C800(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();
	if ((state->m_flags & VAR_INTCXROM) || (state->m_a2_cnxx_slot == -1))
	{
		meminfo->read_mem           = (begin & 0x0FFF) | (state->m_flags & VAR_ROMSWITCH ? 0x4000 : 0x0000) | APPLE2_MEM_ROM;
		meminfo->write_mem          = APPLE2_MEM_FLOATING;
	}
	else
	{
		meminfo->read_handler = &state->rd_c800;
		meminfo->write_handler = &state->wd_c800;
	}
}

static void apple2_mem_CE00(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();
	if ((state->m_flags & VAR_ROMSWITCH) && !strcmp(machine.system().name, "apple2cp"))
	{
		meminfo->read_mem       = APPLE2_MEM_AUX;
		meminfo->write_mem      = APPLE2_MEM_AUX;
	}
	else
	{
		if ((state->m_flags & VAR_INTCXROM) || (state->m_a2_cnxx_slot == -1))
		{
			meminfo->read_mem       = (begin & 0x0FFF) | (state->m_flags & VAR_ROMSWITCH ? 0x4000 : 0x0000) | APPLE2_MEM_ROM;
			meminfo->write_mem      = APPLE2_MEM_FLOATING;
		}
		else
		{
			meminfo->read_handler = &state->rd_ce00;
			meminfo->write_handler = &state->wd_ce00;
		}
	}
}

static void apple2_mem_D000(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();

	if (state->m_inh_slot == -1)
	{
		if (state->m_flags & VAR_LCRAM)
		{
			if (state->m_flags & VAR_LCRAM2)
			{
				meminfo->read_handler = (state->m_flags & VAR_ALTZP)  ? &state->read_delegates_c000[0] : &state->read_delegates_c000[1];
			}
			else
			{
				meminfo->read_handler = (state->m_flags & VAR_ALTZP)  ? &state->read_delegates_d000[0] : &state->read_delegates_d000[1];
			}
		}
		else
		{
			meminfo->read_mem       = (state->m_flags & VAR_ROMSWITCH) ? 0x005000 : 0x001000;
			meminfo->read_mem       |= APPLE2_MEM_ROM;
		}

		if (state->m_flags & VAR_LCWRITE)
		{
			if (state->m_flags & VAR_LCRAM2)
			{
				meminfo->write_handler  = (state->m_flags & VAR_ALTZP) ? &state->write_delegates_c000[0] : &state->write_delegates_c000[1];
			}
			else
			{
				meminfo->write_handler  = (state->m_flags & VAR_ALTZP) ? &state->write_delegates_d000[0] : &state->write_delegates_d000[1];
			}
		}
		else
		{
			meminfo->write_mem = APPLE2_MEM_FLOATING;
		}
	}
	else
	{
		meminfo->read_handler = &state->rd_inh_d000;
		meminfo->write_handler = &state->wd_inh_d000;
	}
}

static void apple2_mem_E000(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo)
{
	apple2_state *state = machine.driver_data<apple2_state>();

	if (state->m_inh_slot == -1)
	{
		if (state->m_flags & VAR_LCRAM)
		{
			meminfo->read_handler = (state->m_flags & VAR_ALTZP)  ? &state->read_delegates_e000[0] : &state->read_delegates_e000[1];
		}
		else
		{
			meminfo->read_mem       = (state->m_flags & VAR_ROMSWITCH) ? 0x006000 : 0x002000;
			meminfo->read_mem       |= APPLE2_MEM_ROM;
		}

		if (state->m_flags & VAR_LCWRITE)
		{
			meminfo->write_handler  = (state->m_flags & VAR_ALTZP) ? &state->write_delegates_e000[0] : &state->write_delegates_e000[1];
		}
		else
		{
			meminfo->write_mem      = APPLE2_MEM_FLOATING;
		}
	}
	else
	{
		meminfo->read_handler = &state->rd_inh_e000;
		meminfo->write_handler = &state->wd_inh_e000;
	}
}



static const apple2_memmap_entry apple2_memmap_entries[] =
{
	{ 0x0000, 0x01FF, apple2_mem_0000, A2MEM_MONO },
	{ 0x0200, 0x03FF, apple2_mem_0200, A2MEM_DUAL },
	{ 0x0400, 0x07FF, apple2_mem_0400, A2MEM_DUAL },
	{ 0x0800, 0x1FFF, apple2_mem_0800, A2MEM_DUAL },
	{ 0x2000, 0x3FFF, apple2_mem_2000, A2MEM_DUAL },
	{ 0x4000, 0xBFFF, apple2_mem_4000, A2MEM_DUAL },
	{ 0xC000, 0xC07F, apple2_mem_C000, A2MEM_IO },
	{ 0xC080, 0xC0FF, apple2_mem_C080, A2MEM_IO },
	{ 0xC100, 0xC2FF, apple2_mem_Cx00, A2MEM_MONO },
	{ 0xC300, 0xC3FF, apple2_mem_C300, A2MEM_MONO },
	{ 0xC400, 0xC7FF, apple2_mem_Cx00, A2MEM_MONO },
	{ 0xC800, 0xCDFF, apple2_mem_C800, A2MEM_MONO },
	{ 0xCE00, 0xCFFE, apple2_mem_CE00, A2MEM_MONO },
	{ 0xCFFF, 0xCFFF, apple2_mem_CFFF, A2MEM_IO },
	{ 0xD000, 0xDFFF, apple2_mem_D000, A2MEM_DUAL },
	{ 0xE000, 0xFFFF, apple2_mem_E000, A2MEM_DUAL },
	{ 0 }
};


static const apple2_memmap_entry tk2000_memmap_entries[] =
{
	{ 0x0000, 0x01FF, apple2_mem_0000, A2MEM_MONO },
	{ 0x0200, 0x03FF, apple2_mem_0200, A2MEM_DUAL },
	{ 0x0400, 0x07FF, apple2_mem_0400, A2MEM_DUAL },
	{ 0x0800, 0x1FFF, apple2_mem_0800, A2MEM_DUAL },
	{ 0x2000, 0x3FFF, apple2_mem_2000, A2MEM_DUAL },
	{ 0x4000, 0xBFFF, apple2_mem_4000, A2MEM_DUAL },
	{ 0xC000, 0xC07F, apple2_mem_C000, A2MEM_IO },
	{ 0xC080, 0xC0FF, apple2_mem_C080, A2MEM_IO },
	{ 0xC100, 0xFFFF, tk2000_mem_C100, A2MEM_DUAL },
	{ 0 }
};

void apple2_state::apple2_setvar(UINT32 val, UINT32 mask)
{
	LOG(("apple2_setvar(): val=0x%06x mask=0x%06x pc=0x%04x\n", val, mask,
					(unsigned int) m_maincpu->pc()));

	assert((val & mask) == val);

	/* apply mask and set */
	val &= m_a2_mask;
	val |= m_a2_set;

	/* change the softswitch */
	m_flags &= ~mask;
	m_flags |= val;

	// disable flags that don't apply (INTCXROM/SLOTC3ROM on II/II+ for instance)
	m_flags &= ~m_flags_mask;

	apple2_update_memory();
}



/* -----------------------------------------------------------------------
 * Floating bus code
 *
 *     preliminary floating bus video scanner code - look for comments
 *     with FIX:
 * ----------------------------------------------------------------------- */

UINT8 apple2_state::apple2_getfloatingbusvalue()
{
	enum
	{
		// scanner types
		kScannerNone = 0, kScannerApple2, kScannerApple2e,

		// scanner constants
		kHBurstClock      =    53, // clock when Color Burst starts
		kHBurstClocks     =     4, // clocks per Color Burst duration
		kHClock0State     =  0x18, // H[543210] = 011000
		kHClocks          =    65, // clocks per horizontal scan (including HBL)
		kHPEClock         =    40, // clock when HPE (horizontal preset enable) goes low
		kHPresetClock     =    41, // clock when H state presets
		kHSyncClock       =    49, // clock when HSync starts
		kHSyncClocks      =     4, // clocks per HSync duration
		kNTSCScanLines    =   262, // total scan lines including VBL (NTSC)
		kNTSCVSyncLine    =   224, // line when VSync starts (NTSC)
		kPALScanLines     =   312, // total scan lines including VBL (PAL)
		kPALVSyncLine     =   264, // line when VSync starts (PAL)
		kVLine0State      = 0x100, // V[543210CBA] = 100000000
		kVPresetLine      =   256, // line when V state presets
		kVSyncLines       =     4, // lines per VSync duration
		kClocksPerVSync   = kHClocks * kNTSCScanLines // FIX: NTSC only?
	};

	// vars
	//
	int i, Hires, Mixed, Page2, _80Store, ScanLines, /* VSyncLine, ScanCycles,*/
		h_clock, h_state, h_0, h_1, h_2, h_3, h_4, h_5,
		v_line, v_state, v_A, v_B, v_C, v_0, v_1, v_2, v_3, v_4, /* v_5, */
		_hires, addend0, addend1, addend2, sum, address;

	// video scanner data
	//
	i = m_maincpu->total_cycles() % kClocksPerVSync; // cycles into this VSync

	// machine state switches
	//
	Hires    = (m_flags & VAR_HIRES) ? 1 : 0;
	Mixed    = (m_flags & VAR_MIXED) ? 1 : 0;
	Page2    = (m_flags & VAR_PAGE2) ? 1 : 0;
	_80Store = (m_flags & VAR_80STORE) ? 1 : 0;

	// calculate video parameters according to display standard
	//
	ScanLines  = 1 ? kNTSCScanLines : kPALScanLines; // FIX: NTSC only?
	// VSyncLine  = 1 ? kNTSCVSyncLine : kPALVSyncLine; // FIX: NTSC only?
	// ScanCycles = ScanLines * kHClocks;

	// calculate horizontal scanning state
	//
	h_clock = (i + kHPEClock) % kHClocks; // which horizontal scanning clock
	h_state = kHClock0State + h_clock; // H state bits
	if (h_clock >= kHPresetClock) // check for horizontal preset
	{
		h_state -= 1; // correct for state preset (two 0 states)
	}
	h_0 = (h_state >> 0) & 1; // get horizontal state bits
	h_1 = (h_state >> 1) & 1;
	h_2 = (h_state >> 2) & 1;
	h_3 = (h_state >> 3) & 1;
	h_4 = (h_state >> 4) & 1;
	h_5 = (h_state >> 5) & 1;

	// calculate vertical scanning state
	//
	v_line  = i / kHClocks; // which vertical scanning line
	v_state = kVLine0State + v_line; // V state bits
	if ((v_line >= kVPresetLine)) // check for previous vertical state preset
	{
		v_state -= ScanLines; // compensate for preset
	}
	v_A = (v_state >> 0) & 1; // get vertical state bits
	v_B = (v_state >> 1) & 1;
	v_C = (v_state >> 2) & 1;
	v_0 = (v_state >> 3) & 1;
	v_1 = (v_state >> 4) & 1;
	v_2 = (v_state >> 5) & 1;
	v_3 = (v_state >> 6) & 1;
	v_4 = (v_state >> 7) & 1;
	//v_5 = (v_state >> 8) & 1;

	// calculate scanning memory address
	//
	_hires = Hires;
	if (Hires && Mixed && (v_4 & v_2))
	{
		_hires = 0; // (address is in text memory)
	}

	addend0 = 0x68; // 1            1            0            1
	addend1 =              (h_5 << 5) | (h_4 << 4) | (h_3 << 3);
	addend2 = (v_4 << 6) | (v_3 << 5) | (v_4 << 4) | (v_3 << 3);
	sum     = (addend0 + addend1 + addend2) & (0x0F << 3);

	address = 0;
	address |= h_0 << 0; // a0
	address |= h_1 << 1; // a1
	address |= h_2 << 2; // a2
	address |= sum;      // a3 - aa6
	address |= v_0 << 7; // a7
	address |= v_1 << 8; // a8
	address |= v_2 << 9; // a9
	address |= ((_hires) ? v_A : (1 ^ (Page2 & (1 ^ _80Store)))) << 10; // a10
	address |= ((_hires) ? v_B : (Page2 & (1 ^ _80Store))) << 11; // a11
	if (_hires) // hires?
	{
		// Y: insert hires only address bits
		//
		address |= v_C << 12; // a12
		address |= (1 ^ (Page2 & (1 ^ _80Store))) << 13; // a13
		address |= (Page2 & (1 ^ _80Store)) << 14; // a14
	}
	else
	{
		// N: text, so no higher address bits unless Apple ][, not Apple //e
		//
		if ((1) && // Apple ][? // FIX: check for Apple ][? (FB is most useful in old games)
			(kHPEClock <= h_clock) && // Y: HBL?
			(h_clock <= (kHClocks - 1)))
		{
			address |= 1 << 12; // Y: a12 (add $1000 to address!)
		}
	}

	// update VBL' state
	//
	if (v_4 & v_3) // VBL?
	{
		//CMemory::mState &= ~CMemory::kVBLBar; // Y: VBL' is false // FIX: MESS?
	}
	else
	{
		//CMemory::mState |= CMemory::kVBLBar; // N: VBL' is true // FIX: MESS?
	}

	return m_ram->pointer()[address % m_ram->size()]; // FIX: this seems to work, but is it right!?
}



/* -----------------------------------------------------------------------
 * Machine reset
 * ----------------------------------------------------------------------- */

void apple2_state::machine_reset()
{
	int need_intcxrom;

	m_reset_flag = 0;
	m_rambase = m_ram->pointer();
	apple2_refresh_delegates();

	need_intcxrom = !strcmp(machine().system().name, "apple2c")
		|| !strcmp(machine().system().name, "apple2c0")
		|| !strcmp(machine().system().name, "apple2c3")
		|| !strcmp(machine().system().name, "apple2c4")
		|| !strcmp(machine().system().name, "prav8c")
		|| !strcmp(machine().system().name, "apple2cp")
		|| !strncmp(machine().system().name, "apple2g", 7);
	apple2_setvar(need_intcxrom ? VAR_INTCXROM : 0, ~0);

	// IIgs ROM 0 cannot boot unless language card bank 2 is write-enabled (but read ROM) on startup
	// Peter Ferrie reports this is also the default on the IIe/IIc at least
	apple2_setvar(VAR_LCWRITE|VAR_LCRAM2, VAR_LCWRITE | VAR_LCRAM | VAR_LCRAM2);

	m_a2_speaker_state = 0;

	m_a2_cnxx_slot = -1; // bank in ROM at C800 on reset

	m_joystick_x1_time = m_joystick_y1_time = 0;
	m_joystick_x2_time = m_joystick_y2_time = 0;

	memset(m_exp_regs, 0, sizeof(UINT8) * 0x10);
	m_exp_wptr = m_exp_liveptr = 0;

}

int apple2_state::a2_no_ctrl_reset()
{
	return (((m_kbrepeat != nullptr) && (m_resetdip == nullptr)) ||
			((m_resetdip != nullptr) && !m_resetdip->read()));
}

/* -----------------------------------------------------------------------
 * Apple II interrupt; used to force partial updates
 * ----------------------------------------------------------------------- */

TIMER_DEVICE_CALLBACK_MEMBER(apple2_state::apple2_interrupt)
{
	int scanline = param;

	if((scanline % 8) == 0)
		machine().first_screen()->update_partial(machine().first_screen()->vpos());
	if ((m_kbspecial->read() & 0x80) &&
		(a2_no_ctrl_reset() || (m_kbspecial->read() & 0x08)))
	{
			if (!m_reset_flag)
			{
				m_reset_flag = 1;
				/* using PULSE_LINE does not allow us to press and hold key */
				m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			}
			return;
	}

	if (m_reset_flag)
	{
		m_reset_flag = 0;
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		machine().schedule_soft_reset();
	}
}



/***************************************************************************
    apple2_mainramxx00_r
    apple2_mainramxx00_w
    apple2_auxramxx00_r
    apple2_auxramxx00_w
***************************************************************************/

READ8_MEMBER ( apple2_state::apple2_mainram0000_r )
{
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_mainram0200_r )
{
	offset += 0x200;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_mainram0400_r )
{
	offset += 0x400;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_mainram0800_r )
{
	offset += 0x800;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_mainram2000_r )
{
	offset += 0x2000;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_mainram4000_r )
{
	offset += 0x4000;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_mainramc000_r )
{
	offset += 0xc000;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_mainramd000_r )
{
	offset += 0xd000;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_mainrame000_r )
{
	offset += 0xe000;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_auxram0000_r )
{
	if (m_auxslotdevice)
	{
		return m_auxslotdevice->read_auxram(offset);
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return 0xff;
	}

	offset += 0x10000;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_auxram0200_r )
{
	if (m_auxslotdevice)
	{
		return m_auxslotdevice->read_auxram(offset+0x200);
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return 0xff;
	}

	offset += 0x10200;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_auxram0400_r )
{
	if (m_auxslotdevice)
	{
		return m_auxslotdevice->read_auxram(offset+0x400);
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return 0xff;
	}

	offset += 0x10400;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_auxram0800_r )
{
	if (m_auxslotdevice)
	{
		return m_auxslotdevice->read_auxram(offset+0x800);
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return 0xff;
	}

	offset += 0x10800;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_auxram2000_r )
{
	if (m_auxslotdevice)
	{
		return m_auxslotdevice->read_auxram(offset+0x2000);
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return 0xff;
	}

	offset += 0x12000;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_auxram4000_r )
{
	if (m_auxslotdevice)
	{
		return m_auxslotdevice->read_auxram(offset+0x4000);
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return 0xff;
	}

	offset += 0x14000;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_auxramc000_r )
{
	if (m_auxslotdevice)
	{
		return m_auxslotdevice->read_auxram(offset+0xc000);
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return 0xff;
	}

	offset += 0x1c000;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_auxramd000_r )
{
	if (m_auxslotdevice)
	{
		return m_auxslotdevice->read_auxram(offset+0xd000);
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return 0xff;
	}

	offset += 0x1d000;
	return m_rambase[offset];
}

READ8_MEMBER ( apple2_state::apple2_auxrame000_r )
{
	if (m_auxslotdevice)
	{
		return m_auxslotdevice->read_auxram(offset+0xe000);
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return 0xff;
	}

	offset += 0x1e000;
	return m_rambase[offset];
}


WRITE8_MEMBER ( apple2_state::apple2_mainram0000_w )
{
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_mainram0200_w )
{
	offset += 0x200;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_mainram0400_w )
{
	offset += 0x400;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_mainram0800_w )
{
	offset += 0x800;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_mainram2000_w )
{
	offset += 0x2000;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_mainram4000_w )
{
	offset += 0x4000;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_mainramc000_w )
{
	offset += 0xc000;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_mainramd000_w )
{
	offset += 0xd000;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_mainrame000_w )
{
	offset += 0xe000;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_auxram0000_w )
{
	if (m_auxslotdevice)
	{
		m_auxslotdevice->write_auxram(offset, data);
		return;
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return;
	}

	offset += 0x10000;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_auxram0200_w )
{
	if (m_auxslotdevice)
	{
		m_auxslotdevice->write_auxram(offset+0x200, data);
		return;
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return;
	}

	offset += 0x10200;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_auxram0400_w )
{
	if (m_auxslotdevice)
	{
		m_auxslotdevice->write_auxram(offset+0x400, data);
		return;
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return;
	}

	offset += 0x10400;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_auxram0800_w )
{
	if (m_auxslotdevice)
	{
		m_auxslotdevice->write_auxram(offset+0x800, data);
		return;
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return;
	}

	offset += 0x10800;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_auxram2000_w )
{
	if (m_auxslotdevice)
	{
		m_auxslotdevice->write_auxram(offset+0x2000, data);
		return;
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return;
	}

	offset += 0x12000;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_auxram4000_w )
{
	if (m_auxslotdevice)
	{
		m_auxslotdevice->write_auxram(offset+0x4000, data);
		return;
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return;
	}

	offset += 0x14000;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_auxramc000_w )
{
	if (m_auxslotdevice)
	{
		m_auxslotdevice->write_auxram(offset+0xc000, data);
		return;
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return;
	}

	offset += 0x1c000;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_auxramd000_w )
{
	if (m_auxslotdevice)
	{
		m_auxslotdevice->write_auxram(offset+0xd000, data);
		return;
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return;
	}

	offset += 0x1d000;
	m_rambase[offset] = data;
}

WRITE8_MEMBER ( apple2_state::apple2_auxrame000_w )
{
	if (m_auxslotdevice)
	{
		m_auxslotdevice->write_auxram(offset+0xe000, data);
		return;
	}
	else if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		return;
	}

	offset += 0x1e000;
	m_rambase[offset] = data;
}

/***************************************************************************
  apple2_c00x_r
***************************************************************************/

READ8_MEMBER ( apple2_state::apple2_c00x_r )
{
	UINT8 result = 0;

	if(!space.debugger_access())
	{
		/* Read the keyboard data and strobe */
		g_profiler.start(PROFILER_C00X);
		result = m_transchar | m_strobe;
		g_profiler.stop();
	}



	return result;
}



/***************************************************************************
  apple2_c00x_w

  C000  80STOREOFF
  C001  80STOREON - use 80-column memory mapping
  C002  RAMRDOFF
  C003  RAMRDON - read from aux 48k
  C004  RAMWRTOFF
  C005  RAMWRTON - write to aux 48k
  C006  INTCXROMOFF
  C007  INTCXROMON
  C008  ALTZPOFF
  C009  ALTZPON - use aux ZP, stack and language card area
  C00A  SLOTC3ROMOFF
  C00B  SLOTC3ROMON - use external slot 3 ROM
  C00C  80COLOFF
  C00D  80COLON - use 80-column display mode
  C00E  ALTCHARSETOFF
  C00F  ALTCHARSETON - use alt character set
***************************************************************************/

WRITE8_MEMBER ( apple2_state::apple2_c00x_w )
{
	UINT32 mask;

	mask = 1 << (offset / 2);
	apple2_setvar((offset & 1) ? mask : 0, mask);
}



/***************************************************************************
  apple2_c01x_r
***************************************************************************/

READ8_MEMBER( apple2_state::apple2_c01x_r )
{
	UINT8 result = apple2_getfloatingbusvalue() & 0x7F;

	if(!space.debugger_access())
	{
		g_profiler.start(PROFILER_C01X);

		LOG(("a2 softswitch_r: %04x\n", offset + 0xc010));
		switch (offset)
		{
			case 0x00:          result |= m_transchar | m_strobe;  m_strobe = 0;  break;
			case 0x01:          result |= (m_flags & VAR_LCRAM2)        ? 0x80 : 0x00;  break;
			case 0x02:          result |= (m_flags & VAR_LCRAM)     ? 0x80 : 0x00;  break;
			case 0x03:          result |= (m_flags & VAR_RAMRD)     ? 0x80 : 0x00;  break;
			case 0x04:          result |= (m_flags & VAR_RAMWRT)        ? 0x80 : 0x00;  break;
			case 0x05:          result |= (m_flags & VAR_INTCXROM)  ? 0x80 : 0x00;  break;
			case 0x06:          result |= (m_flags & VAR_ALTZP)     ? 0x80 : 0x00;  break;
			case 0x07:          result |= (m_flags & VAR_SLOTC3ROM) ? 0x80 : 0x00;  break;
			case 0x08:          result |= (m_flags & VAR_80STORE)   ? 0x80 : 0x00;  break;
			case 0x09:          result |= !space.machine().first_screen()->vblank()     ? 0x80 : 0x00;  break;
			case 0x0A:          result |= (m_flags & VAR_TEXT)      ? 0x80 : 0x00;  break;
			case 0x0B:          result |= (m_flags & VAR_MIXED)     ? 0x80 : 0x00;  break;
			case 0x0C:          result |= (m_flags & VAR_PAGE2)     ? 0x80 : 0x00;  break;
			case 0x0D:          result |= (m_flags & VAR_HIRES)     ? 0x80 : 0x00;  break;
			case 0x0E:          result |= (m_flags & VAR_ALTCHARSET)    ? 0x80 : 0x00;  break;
			case 0x0F:          result |= (m_flags & VAR_80COL)     ? 0x80 : 0x00;  break;
		}

		g_profiler.stop();
	}

	return result;
}



/***************************************************************************
  apple2_c01x_w
***************************************************************************/

WRITE8_MEMBER( apple2_state::apple2_c01x_w )
{
	/* Clear the keyboard strobe */
	g_profiler.start(PROFILER_C01X);
	m_strobe = 0;
	g_profiler.stop();
}



/***************************************************************************
  apple2_c02x_r
***************************************************************************/

READ8_MEMBER( apple2_state::apple2_c02x_r )
{
	if(!space.debugger_access())
	{
		apple2_c02x_w(space, offset, 0, 0);
	}
	return apple2_getfloatingbusvalue();
}



/***************************************************************************
  apple2_c02x_w
***************************************************************************/

WRITE8_MEMBER( apple2_state::apple2_c02x_w )
{
	switch(offset)
	{
		case 0x08:
			apple2_setvar((m_flags & VAR_ROMSWITCH) ^ VAR_ROMSWITCH, VAR_ROMSWITCH);
			break;
	}
}



/***************************************************************************
  apple2_c03x_r
***************************************************************************/

READ8_MEMBER ( apple2_state::apple2_c03x_r )
{
	if(!space.debugger_access())
	{
		if (!offset)
		{
			speaker_sound_device *speaker = space.machine().device<speaker_sound_device>("a2speaker");

			m_a2_speaker_state ^= 1;
			speaker->level_w(m_a2_speaker_state);
		}
	}
	return apple2_getfloatingbusvalue();
}



/***************************************************************************
  apple2_c03x_w
***************************************************************************/

WRITE8_MEMBER ( apple2_state::apple2_c03x_w )
{
	apple2_c03x_r(space, offset, 0);
}



/***************************************************************************
  apple2_c05x_r
***************************************************************************/

READ8_MEMBER ( apple2_state::apple2_c05x_r )
{
	if(!space.debugger_access())
	{
		UINT32 mask;

		if (m_machinetype == TK2000)
		{
			if (offset == 0xa)  // RAM
			{
				apple2_setvar(VAR_TK2000RAM, ~0);
			}
			else if (offset == 0xb) // ROM
			{
				apple2_setvar(0, ~VAR_TK2000RAM);
			}
		}

		/* ANx has reverse SET logic */
		if (offset >= 8)
		{
			offset ^= 1;
		}

		mask = 0x100 << (offset / 2);
		apple2_setvar((offset & 1) ? mask : 0, mask);
	}
	return apple2_getfloatingbusvalue();
}



/***************************************************************************
  apple2_c05x_w
***************************************************************************/

WRITE8_MEMBER ( apple2_state::apple2_c05x_w )
{
	apple2_c05x_r(space, offset, 0);
}



/***************************************************************************
  apple2_c06x_r
***************************************************************************/

READ8_MEMBER ( apple2_state::apple2_c06x_r )
{
	int result = 0;
	if(!space.debugger_access())
	{
		switch (offset & 0x0F)
		{
			case 0x00:
				/* Cassette input */
				{
					if (m_cassette)
					{
						result = m_cassette->input() > 0.0 ? 0x80 : 0;
					}
					else
					{
						result = 0;
					}
				}
				break;
			case 0x01:
				/* Open-Apple/Joystick button 0 */
				result = apple2_pressed_specialkey(0x10);
				break;
			case 0x02:
				/* Closed-Apple/Joystick button 1 */
				result = apple2_pressed_specialkey(0x20);
				break;
			case 0x03:
				/* Joystick button 2. Later revision motherboards connected this to SHIFT also */
				result = apple2_pressed_specialkey(0x40);
				break;
			case 0x04:
				/* X Joystick 1 axis */
				result = space.machine().time().as_double() < m_joystick_x1_time;
				break;
			case 0x05:
				/* Y Joystick 1 axis */
				result = space.machine().time().as_double() < m_joystick_y1_time;
				break;
			case 0x06:
				/* X Joystick 2 axis */
				result = space.machine().time().as_double() < m_joystick_x2_time;
				break;
			case 0x07:
				/* Y Joystick 2 axis */
				result = space.machine().time().as_double() < m_joystick_y2_time;
				break;
			default:
				/* c060 Empty Cassette head read
				 * and any other non joystick c06 port returns this according to applewin
				 */
				return apple2_getfloatingbusvalue();
		}
	}
	return result ? 0x80 : 0x00;
}



/***************************************************************************
  apple2_c07x_r
***************************************************************************/

READ8_MEMBER ( apple2_state::apple2_c07x_r )
{
	if(!space.debugger_access())
	{
		double x_calibration = attotime::from_usec(12).as_double();
		double y_calibration = attotime::from_usec(13).as_double();

		if (offset == 0)
		{
			m_joystick_x1_time = machine().time().as_double() + x_calibration * m_joy1x->read();
			m_joystick_y1_time = machine().time().as_double() + y_calibration * m_joy1y->read();
			m_joystick_x2_time = machine().time().as_double() + x_calibration * m_joy2x->read();
			m_joystick_y2_time = machine().time().as_double() + y_calibration * m_joy2y->read();
		}
	}
	return 0;
}



/***************************************************************************
  apple2_c07x_w
***************************************************************************/

WRITE8_MEMBER ( apple2_state::apple2_c07x_w )
{
	// this a machine with an aux slot?
	if (m_auxslotdevice)
	{
		m_auxslotdevice->write_c07x(space, offset&0xf, data);
	}

	// AE RamWorks manual indicates that even if the auxslot card sees the c07x write,
	// so does the motherboard and it will trigger the paddles.  So always call this.
	apple2_c07x_r(space, offset, 0);
}



/* -----------------------------------------------------------------------
 * Floppy disk controller
 * ----------------------------------------------------------------------- */


int apple2_state::apple2_fdc_has_35()
{
	return (floppy_get_count(machine())); // - apple525_get_count(machine)) > 0;
}

int apple2_state::apple2_fdc_has_525()
{
	return 1; //apple525_get_count(machine) > 0;
}

static void apple2_fdc_set_lines(device_t *device, UINT8 lines)
{
	apple2_state *state = device->machine().driver_data<apple2_state>();
	if (state->m_fdc_diskreg & 0x40)
	{
		if (state->apple2_fdc_has_35())
		{
			/* slot 5: 3.5" disks */
			sony_set_lines(device,lines);
		}
	}
	else
	{
		if (state->apple2_fdc_has_525())
		{
			/* slot 6: 5.25" disks */
			apple525_set_lines(device,lines);
		}
	}
}



static void apple2_fdc_set_enable_lines(device_t *device,int enable_mask)
{
	apple2_state *state = device->machine().driver_data<apple2_state>();
	int slot5_enable_mask = 0;
	int slot6_enable_mask = 0;

	if (state->m_fdc_diskreg & 0x40)
		slot5_enable_mask = enable_mask;
	else
		slot6_enable_mask = enable_mask;

	if (state->apple2_fdc_has_35())
	{
		/* set the 3.5" enable lines */
		sony_set_enable_lines(device,slot5_enable_mask);
	}

	if (state->apple2_fdc_has_525())
	{
		/* set the 5.25" enable lines */
		apple525_set_enable_lines(device,slot6_enable_mask);
	}
}



static UINT8 apple2_fdc_read_data(device_t *device)
{
	apple2_state *state = device->machine().driver_data<apple2_state>();
	UINT8 result = 0x00;

	if (state->m_fdc_diskreg & 0x40)
	{
		if (state->apple2_fdc_has_35())
		{
			/* slot 5: 3.5" disks */
			result = sony_read_data(device);
		}
	}
	else
	{
		if (state->apple2_fdc_has_525())
		{
			/* slot 6: 5.25" disks */
			result = apple525_read_data(device);
		}
	}
	return result;
}



static void apple2_fdc_write_data(device_t *device, UINT8 data)
{
	apple2_state *state = device->machine().driver_data<apple2_state>();
	if (state->m_fdc_diskreg & 0x40)
	{
		if (state->apple2_fdc_has_35())
		{
			/* slot 5: 3.5" disks */
			sony_write_data(device,data);
		}
	}
	else
	{
		if (state->apple2_fdc_has_525())
		{
			/* slot 6: 5.25" disks */
			apple525_write_data(device,data);
		}
	}
}



static int apple2_fdc_read_status(device_t *device)
{
	apple2_state *state = device->machine().driver_data<apple2_state>();
	int result = 0;

	if (state->m_fdc_diskreg & 0x40)
	{
		if (state->apple2_fdc_has_35())
		{
			/* slot 5: 3.5" disks */
			result = sony_read_status(device);
		}
	}
	else
	{
		if (state->apple2_fdc_has_525())
		{
			/* slot 6: 5.25" disks */
			result = apple525_read_status(device);
		}
	}
	return result;
}


void apple2_state::apple2_iwm_setdiskreg(UINT8 data)
{
	m_fdc_diskreg = data & 0xC0;
	if (apple2_fdc_has_35())
		sony_set_sel_line( machine().device("fdc"),m_fdc_diskreg & 0x80);
}


const applefdc_interface apple2_fdc_interface =
{
	apple2_fdc_set_lines,           /* set_lines */
	apple2_fdc_set_enable_lines,    /* set_enable_lines */

	apple2_fdc_read_data,           /* read_data */
	apple2_fdc_write_data,          /* write_data */
	apple2_fdc_read_status          /* read_status */
};



/* -----------------------------------------------------------------------
 * Driver init
 * ----------------------------------------------------------------------- */

void apple2_state::apple2_init_common()
{
	m_inh_slot = -1;
	m_flags = 0;
	m_fdc_diskreg = 0;

	// do these lookups once at startup
	m_rom = memregion("maincpu")->base();
	m_rom_length = memregion("maincpu")->bytes() & ~0xFFF;
	m_slot_length = memregion("maincpu")->bytes() - m_rom_length;
	m_slot_ram = (m_slot_length > 0) ? &m_rom[m_rom_length] : nullptr;

	m_auxslotdevice = nullptr;
	if (m_machinetype == APPLE_IIE || m_machinetype == TK3000)
	{
		m_auxslotdevice = m_a2eauxslot->get_a2eauxslot_card();
	}

	/* state save registers */
	save_item(NAME(m_flags));
	machine().save().register_postload(save_prepost_delegate(FUNC(apple2_state::apple2_update_memory_postload), this));

	/* --------------------------------------------- *
	 * set up the softswitch mask/set                *
	 * --------------------------------------------- */
	m_a2_mask = ~0;
	m_a2_set = 0;

	/* disable VAR_ROMSWITCH if the ROM is only 16k */
	if (memregion("maincpu")->bytes() < 0x8000)
		m_a2_mask &= ~VAR_ROMSWITCH;

	if (m_ram->size() <= 64*1024)
		m_a2_mask &= ~(VAR_RAMRD | VAR_RAMWRT | VAR_80STORE | VAR_ALTZP | VAR_80COL);

	apple2_refresh_delegates();
}

void apple2_state::apple2eplus_init_common(void *apple2cp_ce00_ram)
{
	apple2_memmap_config mem_cfg;

	m_flags_mask = 0;

	apple2_init_common();

	/* setup memory */
	memset(&mem_cfg, 0, sizeof(mem_cfg));
	mem_cfg.first_bank = 1;
	mem_cfg.memmap = apple2_memmap_entries;
	mem_cfg.auxmem = (UINT8*)apple2cp_ce00_ram;
	apple2_setup_memory(&mem_cfg);
}

MACHINE_START_MEMBER(apple2_state,apple2c)
{
	m_machinetype = APPLE_IIC;

	apple2eplus_init_common((void *)nullptr);
}

MACHINE_START_MEMBER(apple2_state,tk3000)
{
	m_machinetype = TK3000; // enhanced IIe clone with Z80 keyboard scanner subcpu

	apple2eplus_init_common((void *)nullptr);
}

MACHINE_START_MEMBER(apple2_state,apple2cp)
{
	void *apple2cp_ce00_ram;

	/* there appears to be some hidden RAM that is swapped in on the Apple
	 * IIc plus; I have not found any official documentation but the BIOS
	 * clearly uses this area as writeable memory */
	apple2cp_ce00_ram = auto_alloc_array(machine(), UINT8, 0x200);
	memset(apple2cp_ce00_ram, 0, sizeof(UINT8) * 0x200);

	m_machinetype = APPLE_IICPLUS;

	apple2eplus_init_common(apple2cp_ce00_ram);
}

MACHINE_START_MEMBER(apple2_state,apple2e)
{
	apple2_memmap_config mem_cfg;

	m_flags_mask = 0;

	m_machinetype = APPLE_IIE;

	apple2_init_common();

	/* setup memory */
	memset(&mem_cfg, 0, sizeof(mem_cfg));
	mem_cfg.first_bank = 1;
	mem_cfg.memmap = apple2_memmap_entries;
	mem_cfg.auxmem = (UINT8*)nullptr;
	apple2_setup_memory(&mem_cfg);
}

MACHINE_START_MEMBER(apple2_state,laser128)
{
	apple2_memmap_config mem_cfg;

	m_flags_mask = 0;
	m_machinetype = LASER128;

	apple2_init_common();

	// 1 MB of expansion RAM in slot 5
	m_exp_ram = std::make_unique<UINT8[]>(1024*1024);
	memset(m_exp_ram.get(), 0xff, 1024*1024);

	m_exp_bankhior = 0xf0;
	m_exp_addrmask = 0xfffff;

	// save memory expansion vars
	save_item(NAME(m_exp_regs));
	save_item(NAME(m_exp_wptr));
	save_item(NAME(m_exp_liveptr));
	save_item(NAME(m_exp_bankhior));
	save_item(NAME(m_exp_addrmask));

	/* setup memory */
	memset(&mem_cfg, 0, sizeof(mem_cfg));
	mem_cfg.first_bank = 1;
	mem_cfg.memmap = apple2_memmap_entries;
	mem_cfg.auxmem = (UINT8*)nullptr;
	apple2_setup_memory(&mem_cfg);
}

MACHINE_START_MEMBER(apple2_state,apple2orig)
{
	apple2_memmap_config mem_cfg;
	void *apple2cp_ce00_ram = nullptr;

	// II and II+ have no internal ROM or internal slot 3 h/w, so don't allow these states
	m_flags_mask = VAR_INTCXROM|VAR_SLOTC3ROM;

	m_machinetype = APPLE_II;

	apple2_init_common();

	/* setup memory */
	memset(&mem_cfg, 0, sizeof(mem_cfg));
	mem_cfg.first_bank = 1;
	mem_cfg.memmap = apple2_memmap_entries;
	mem_cfg.auxmem = (UINT8*)apple2cp_ce00_ram;
	apple2_setup_memory(&mem_cfg);
}

MACHINE_START_MEMBER(apple2_state,space84)
{
	apple2_memmap_config mem_cfg;
	void *apple2cp_ce00_ram = nullptr;

	// II and II+ have no internal ROM or internal slot 3 h/w, so don't allow these states
	m_flags_mask = VAR_INTCXROM|VAR_SLOTC3ROM;

	m_machinetype = SPACE84;

	apple2_init_common();

	/* setup memory */
	memset(&mem_cfg, 0, sizeof(mem_cfg));
	mem_cfg.first_bank = 1;
	mem_cfg.memmap = apple2_memmap_entries;
	mem_cfg.auxmem = (UINT8*)apple2cp_ce00_ram;
	apple2_setup_memory(&mem_cfg);
}

MACHINE_START_MEMBER(apple2_state,laba2p)
{
	apple2_memmap_config mem_cfg;
	void *apple2cp_ce00_ram = nullptr;

	// II and II+ have no internal ROM or internal slot 3 h/w, so don't allow these states
	m_flags_mask = VAR_INTCXROM|VAR_SLOTC3ROM;

	m_machinetype = LABA2P;

	apple2_init_common();

	/* setup memory */
	memset(&mem_cfg, 0, sizeof(mem_cfg));
	mem_cfg.first_bank = 1;
	mem_cfg.memmap = apple2_memmap_entries;
	mem_cfg.auxmem = (UINT8*)apple2cp_ce00_ram;
	apple2_setup_memory(&mem_cfg);
}

MACHINE_START_MEMBER(apple2_state,tk2000)
{
	apple2_memmap_config mem_cfg;

	// II and II+ have no internal ROM or internal slot 3 h/w, so don't allow these states
	m_flags_mask = VAR_INTCXROM|VAR_SLOTC3ROM;

	m_machinetype = TK2000;

	apple2_init_common();

	/* setup memory */
	memset(&mem_cfg, 0, sizeof(mem_cfg));
	mem_cfg.first_bank = 1;
	mem_cfg.memmap = tk2000_memmap_entries;
	mem_cfg.auxmem = (UINT8*)nullptr;
	apple2_setup_memory(&mem_cfg);
}

int apple2_state::apple2_pressed_specialkey(UINT8 key)
{
	return ((m_kbspecial ? m_kbspecial->read() : 0) & key)
		|| ((m_joybuttons ? m_joybuttons->read() : 0) & key);
}

void apple2_state::apple2_refresh_delegates()
{
	read_delegates_master[0] = read8_delegate(FUNC(apple2_state::read_floatingbus), this);
	read_delegates_master[1] = read8_delegate(FUNC(apple2_state::apple2_c1xx_r), this);
	read_delegates_master[2] = read8_delegate(FUNC(apple2_state::apple2_c3xx_r), this);
	read_delegates_master[3] = read8_delegate(FUNC(apple2_state::apple2_c4xx_r), this);
	write_delegates_master[0] = write8_delegate(FUNC(apple2_state::apple2_c1xx_w), this);
	write_delegates_master[1] = write8_delegate(FUNC(apple2_state::apple2_c3xx_w), this);
	write_delegates_master[2] = write8_delegate(FUNC(apple2_state::apple2_c4xx_w), this);

	rd_c000 = read8_delegate(FUNC(apple2_state::apple2_c0xx_r), this);
	wd_c000 = write8_delegate(FUNC(apple2_state::apple2_c0xx_w), this);

	rd_c080 = read8_delegate(FUNC(apple2_state::apple2_c080_r), this);
	wd_c080 = write8_delegate(FUNC(apple2_state::apple2_c080_w), this);

	rd_cfff = read8_delegate(FUNC(apple2_state::apple2_cfff_r), this);
	wd_cfff = write8_delegate(FUNC(apple2_state::apple2_cfff_w), this);

	rd_c800 = read8_delegate(FUNC(apple2_state::apple2_c800_r), this);
	wd_c800 = write8_delegate(FUNC(apple2_state::apple2_c800_w), this);

	rd_ce00 = read8_delegate(FUNC(apple2_state::apple2_ce00_r), this);
	wd_ce00 = write8_delegate(FUNC(apple2_state::apple2_ce00_w), this);

	rd_inh_d000 = read8_delegate(FUNC(apple2_state::apple2_inh_d000_r), this);
	wd_inh_d000 = write8_delegate(FUNC(apple2_state::apple2_inh_d000_w), this);

	rd_inh_e000 = read8_delegate(FUNC(apple2_state::apple2_inh_e000_r), this);
	wd_inh_e000 = write8_delegate(FUNC(apple2_state::apple2_inh_e000_w), this);

	read_delegates_0000[0] = read8_delegate(FUNC(apple2_state::apple2_auxram0000_r), this);
	read_delegates_0000[1] = read8_delegate(FUNC(apple2_state::apple2_mainram0000_r), this);
	read_delegates_0200[0] = read8_delegate(FUNC(apple2_state::apple2_auxram0200_r), this);
	read_delegates_0200[1] = read8_delegate(FUNC(apple2_state::apple2_mainram0200_r), this);
	read_delegates_0400[0] = read8_delegate(FUNC(apple2_state::apple2_auxram0400_r), this);
	read_delegates_0400[1] = read8_delegate(FUNC(apple2_state::apple2_mainram0400_r), this);
	read_delegates_0800[0] = read8_delegate(FUNC(apple2_state::apple2_auxram0800_r), this);
	read_delegates_0800[1] = read8_delegate(FUNC(apple2_state::apple2_mainram0800_r), this);
	read_delegates_2000[0] = read8_delegate(FUNC(apple2_state::apple2_auxram2000_r), this);
	read_delegates_2000[1] = read8_delegate(FUNC(apple2_state::apple2_mainram2000_r), this);
	read_delegates_4000[0] = read8_delegate(FUNC(apple2_state::apple2_auxram4000_r), this);
	read_delegates_4000[1] = read8_delegate(FUNC(apple2_state::apple2_mainram4000_r), this);
	read_delegates_c000[0] = read8_delegate(FUNC(apple2_state::apple2_auxramc000_r), this);
	read_delegates_c000[1] = read8_delegate(FUNC(apple2_state::apple2_mainramc000_r), this);
	read_delegates_d000[0] = read8_delegate(FUNC(apple2_state::apple2_auxramd000_r), this);
	read_delegates_d000[1] = read8_delegate(FUNC(apple2_state::apple2_mainramd000_r), this);
	read_delegates_e000[0] = read8_delegate(FUNC(apple2_state::apple2_auxrame000_r), this);
	read_delegates_e000[1] = read8_delegate(FUNC(apple2_state::apple2_mainrame000_r), this);

	write_delegates_0000[0] = write8_delegate(FUNC(apple2_state::apple2_auxram0000_w), this);
	write_delegates_0000[1] = write8_delegate(FUNC(apple2_state::apple2_mainram0000_w), this);
	write_delegates_0200[0] = write8_delegate(FUNC(apple2_state::apple2_auxram0200_w), this);
	write_delegates_0200[1] = write8_delegate(FUNC(apple2_state::apple2_mainram0200_w), this);
	write_delegates_0400[0] = write8_delegate(FUNC(apple2_state::apple2_auxram0400_w), this);
	write_delegates_0400[1] = write8_delegate(FUNC(apple2_state::apple2_mainram0400_w), this);
	write_delegates_0800[0] = write8_delegate(FUNC(apple2_state::apple2_auxram0800_w), this);
	write_delegates_0800[1] = write8_delegate(FUNC(apple2_state::apple2_mainram0800_w), this);
	write_delegates_2000[0] = write8_delegate(FUNC(apple2_state::apple2_auxram2000_w), this);
	write_delegates_2000[1] = write8_delegate(FUNC(apple2_state::apple2_mainram2000_w), this);
	write_delegates_4000[0] = write8_delegate(FUNC(apple2_state::apple2_auxram4000_w), this);
	write_delegates_4000[1] = write8_delegate(FUNC(apple2_state::apple2_mainram4000_w), this);
	write_delegates_c000[0] = write8_delegate(FUNC(apple2_state::apple2_auxramc000_w), this);
	write_delegates_c000[1] = write8_delegate(FUNC(apple2_state::apple2_mainramc000_w), this);
	write_delegates_d000[0] = write8_delegate(FUNC(apple2_state::apple2_auxramd000_w), this);
	write_delegates_d000[1] = write8_delegate(FUNC(apple2_state::apple2_mainramd000_w), this);
	write_delegates_e000[0] = write8_delegate(FUNC(apple2_state::apple2_auxrame000_w), this);
	write_delegates_e000[1] = write8_delegate(FUNC(apple2_state::apple2_mainrame000_w), this);
}

READ_LINE_MEMBER(apple2_state::ay3600_shift_r)
{
	// either shift key
	if (m_kbspecial->read() & 0x06)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

READ_LINE_MEMBER(apple2_state::ay3600_control_r)
{
	if (m_kbspecial->read() & 0x08)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

static const UINT8 a2_key_remap[0x32][4] =
{
/*    norm shft ctrl both */
	{ 0x33,0x23,0x33,0x23 },    /* 3 #     00     */
	{ 0x34,0x24,0x34,0x24 },    /* 4 $     01     */
	{ 0x35,0x25,0x35,0x25 },    /* 5 %     02     */
	{ 0x36,0x5e,0x35,0x53 },    /* 6 ^     03     */
	{ 0x37,0x26,0x37,0x26 },    /* 7 &     04     */
	{ 0x38,0x2a,0x38,0x2a },    /* 8 *     05     */
	{ 0x39,0x28,0x39,0x28 },    /* 9 (     06     */
	{ 0x30,0x29,0x30,0x29 },    /* 0 )     07     */
	{ 0x3b,0x3a,0x3b,0x3a },    /* ; :     08     */
	{ 0x2d,0x5f,0x2d,0x1f },    /* - _     09     */
	{ 0x51,0x51,0x11,0x11 },    /* q Q     0a     */
	{ 0x57,0x57,0x17,0x17 },    /* w W     0b     */
	{ 0x45,0x45,0x05,0x05 },    /* e E     0c     */
	{ 0x52,0x52,0x12,0x12 },    /* r R     0d     */
	{ 0x54,0x54,0x14,0x14 },    /* t T     0e     */
	{ 0x59,0x59,0x19,0x19 },    /* y Y     0f     */
	{ 0x55,0x55,0x15,0x15 },    /* u U     10     */
	{ 0x49,0x49,0x09,0x09 },    /* i I     11     */
	{ 0x4f,0x4f,0x0f,0x0f },    /* o O     12     */
	{ 0x50,0x50,0x10,0x10 },    /* p P     13     */
	{ 0x44,0x44,0x04,0x04 },    /* d D     14     */
	{ 0x46,0x46,0x06,0x06 },    /* f F     15     */
	{ 0x47,0x47,0x07,0x07 },    /* g G     16     */
	{ 0x48,0x48,0x08,0x08 },    /* h H     17     */
	{ 0x4a,0x4a,0x0a,0x0a },    /* j J     18     */
	{ 0x4b,0x4b,0x0b,0x0b },    /* k K     19     */
	{ 0x4c,0x4c,0x0c,0x0c },    /* l L     1a     */
	{ 0x3d,0x2b,0x3d,0x2b },    /* = +     1b     */
	{ 0x08,0x08,0x08,0x08 },    /* Left    1c     */
	{ 0x15,0x15,0x15,0x15 },    /* Right   1d     */
	{ 0x5a,0x5a,0x1a,0x1a },    /* z Z     1e     */
	{ 0x58,0x58,0x18,0x18 },    /* x X     1f     */
	{ 0x43,0x43,0x03,0x03 },    /* c C     20     */
	{ 0x56,0x56,0x16,0x16 },    /* v V     21     */
	{ 0x42,0x42,0x02,0x02 },    /* b B     22     */
	{ 0x4e,0x4e,0x0e,0x0e },    /* n N     23     */
	{ 0x4d,0x4d,0x0d,0x0d },    /* m M     24     */
	{ 0x2c,0x3c,0x2c,0x3c },    /* , <     25     */
	{ 0x2e,0x3e,0x2e,0x3e },    /* . >     26     */
	{ 0x2f,0x3f,0x2f,0x3f },    /* / ?     27     */
	{ 0x53,0x53,0x13,0x13 },    /* s S     28     */
	{ 0x32,0x40,0x32,0x00 },    /* 2 @     29     */
	{ 0x31,0x21,0x31,0x31 },    /* 1 !     2a     */
	{ 0x9b,0x9b,0x9b,0x9b },    /* Escape  2b     */
	{ 0x41,0x41,0x01,0x01 },    /* a A     2c     */
	{ 0x20,0x20,0x20,0x20 },    /* Space   2d     */
	{ 0x00,0x00,0x00,0x00 },    /* 0x2e unused    */
	{ 0x00,0x00,0x00,0x00 },    /* 0x2f unused    */
	{ 0x00,0x00,0x00,0x00 },    /* 0x30 unused    */
	{ 0x0d,0x0d,0x0d,0x0d },    /* Enter   31     */
};

WRITE_LINE_MEMBER(apple2_state::ay3600_data_ready_w)
{
	if (state == ASSERT_LINE)
	{
		int mod = 0;
		m_lastchar = m_ay3600->b_r();

		mod = (m_kbspecial->read() & 0x06) ? 0x01 : 0x00;
		mod |= (m_kbspecial->read() & 0x08) ? 0x02 : 0x00;

		m_transchar = a2_key_remap[m_lastchar&0x3f][mod];

		if (m_transchar != 0)
		{
			m_strobe = 0x80;
//          printf("new char = %04x (%02x)\n", m_lastchar&0x3f, m_transchar);
		}
	}
}

WRITE_LINE_MEMBER(apple2_state::ay3600_iie_data_ready_w)
{
	if (state == ASSERT_LINE)
	{
		UINT8 *decode = m_kbdrom->base();
		UINT16 trans;

		m_lastchar = m_ay3600->b_r();

		trans = m_lastchar & ~(0x1c0);  // clear the 3600's control/shift stuff
		trans |= (m_lastchar & 0x100)>>2;   // bring the 0x100 bit down to the 0x40 place
		trans <<= 2;                    // 4 entries per key
		trans |= (m_kbspecial->read() & 0x06) ? 0x00 : 0x01;    // shift is bit 1 (active low)
		trans |= (m_kbspecial->read() & 0x08) ? 0x00 : 0x02;    // control is bit 2 (active low)
		trans |= (m_kbspecial->read() & 0x01) ? 0x0000 : 0x0200;    // caps lock is bit 9 (active low)

		m_transchar = decode[trans];
		m_strobe = 0x80;

//      printf("new char = %04x (%02x)\n", m_lastchar, m_transchar);
	}
}
