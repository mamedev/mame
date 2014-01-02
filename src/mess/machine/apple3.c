/***************************************************************************

    machine/apple3.c

    Apple ///


    VIA #0 (D VIA)
        CA1:    1 if a cartridge is inserted, 0 otherwise

    VIA #1 (E VIA)
        CA2:    1 if key pressed, 0 otherwise

***************************************************************************/

#include "emu.h"
#include "includes/apple3.h"
#include "includes/apple2.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/ay3600.h"
#include "machine/applefdc.h"
#include "machine/appldriv.h"
#include "machine/mos6551.h"
#include "machine/ram.h"


static void apple3_update_drives(device_t *device);


#define LOG_MEMORY      1
#define LOG_INDXADDR    1



/* ----------------------------------------------------------------------- */


void apple3_state::apple3_profile_init(void)
{
}



void apple3_state::apple3_profile_statemachine(void)
{
}



UINT8 apple3_state::apple3_profile_r(offs_t offset)
{
	UINT8 result = 0;

	offset %= 4;
	apple3_profile_statemachine();

	m_profile_lastaddr = offset;

	switch(offset)
	{
		case 1:
			m_profile_gotstrobe = 1;
			apple3_profile_statemachine();
			result = m_profile_readdata;
			break;

		case 2:
			if (m_profile_busycount > 0)
			{
				m_profile_busycount--;
				result = 0xFF;
			}
			else
			{
				result = m_profile_busy | m_profile_online;
			}
			break;
	}
	return result;
}



void apple3_state::apple3_profile_w(offs_t offset, UINT8 data)
{
	offset %= 4;
	m_profile_lastaddr = -1;

	switch(offset)
	{
		case 0:
			m_profile_writedata = data;
			m_profile_gotstrobe = 1;
			break;
	}
	apple3_profile_statemachine();
}



/* ----------------------------------------------------------------------- */

READ8_MEMBER(apple3_state::apple3_c0xx_r)
{
	mos6551_device *acia = machine().device<mos6551_device>("acia");
	applefdc_base_device *fdc = machine().device<applefdc_base_device>("fdc");
	UINT8 result = 0xFF;

	switch(offset)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			result = AY3600_keydata_strobe_r(machine());
			break;

		case 0x08: case 0x09: case 0x0A: case 0x0B:
		case 0x0C: case 0x0D: case 0x0E: case 0x0F:
			/* modifier keys */
			result = 0x7d;
			break;

		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1A: case 0x1B:
		case 0x1C: case 0x1D: case 0x1E: case 0x1F:
			AY3600_anykey_clearstrobe_r(machine());
			break;

		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
			/* graphics softswitches */
			if (offset & 1)
				m_flags |= 1 << ((offset - 0x50) / 2);
			else
				m_flags &= ~(1 << ((offset - 0x50) / 2));
			break;

		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6A: case 0x6B:
		case 0x6C: case 0x6D: case 0x6E: case 0x6F:
			/* unsure what these are */
			result = 0x00;
			break;

		case 0xC0: case 0xC1: case 0xC2: case 0xC3:
		case 0xC4: case 0xC5: case 0xC6: case 0xC7:
		case 0xC8: case 0xC9: case 0xCA: case 0xCB:
		case 0xCC: case 0xCD: case 0xCE: case 0xCF:
			/* profile */
			result = apple3_profile_r(offset);
			break;

		case 0xD0: case 0xD1: case 0xD2: case 0xD3:
		case 0xD4: case 0xD5: case 0xD6: case 0xD7:
			/* external drive stuff */
			if (offset & 1)
				m_flags |= VAR_EXTA0 << ((offset - 0xD0) / 2);
			else
				m_flags &= ~(VAR_EXTA0 << ((offset - 0xD0) / 2));
			apple3_update_drives(machine().device("fdc"));
			result = 0x00;
			break;

		case 0xDB:
			apple3_write_charmem();
			break;

		case 0xE0: case 0xE1: case 0xE2: case 0xE3:
		case 0xE4: case 0xE5: case 0xE6: case 0xE7:
		case 0xE8: case 0xE9: case 0xEA: case 0xEB:
		case 0xEC: case 0xED: case 0xEE: case 0xEF:
			result = fdc->read(offset);
			break;

		case 0xF0:
		case 0xF1:
		case 0xF2:
		case 0xF3:
			result = acia->read(space, offset & 0x03);
			break;
	}
	return result;
}



WRITE8_MEMBER(apple3_state::apple3_c0xx_w)
{
	mos6551_device *acia = machine().device<mos6551_device>("acia");
	applefdc_base_device *fdc = machine().device<applefdc_base_device>("fdc");

	switch(offset)
	{
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1A: case 0x1B:
		case 0x1C: case 0x1D: case 0x1E: case 0x1F:
			AY3600_anykey_clearstrobe_r(machine());
			break;

		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
			/* graphics softswitches */
			if (offset & 1)
				m_flags |= 1 << ((offset - 0x50) / 2);
			else
				m_flags &= ~(1 << ((offset - 0x50) / 2));
			break;

		case 0xC0: case 0xC1: case 0xC2: case 0xC3:
		case 0xC4: case 0xC5: case 0xC6: case 0xC7:
		case 0xC8: case 0xC9: case 0xCA: case 0xCB:
		case 0xCC: case 0xCD: case 0xCE: case 0xCF:
			/* profile */
			apple3_profile_w(offset, data);
			break;

		case 0xD0: case 0xD1: case 0xD2: case 0xD3:
		case 0xD4: case 0xD5: case 0xD6: case 0xD7:
			/* external drive stuff */
			if (offset & 1)
				m_flags |= VAR_EXTA0 << ((offset - 0xD0) / 2);
			else
				m_flags &= ~(VAR_EXTA0 << ((offset - 0xD0) / 2));
			apple3_update_drives(machine().device("fdc"));
			break;

		case 0xDB:
			apple3_write_charmem();
			break;

		case 0xE0: case 0xE1: case 0xE2: case 0xE3:
		case 0xE4: case 0xE5: case 0xE6: case 0xE7:
		case 0xE8: case 0xE9: case 0xEA: case 0xEB:
		case 0xEC: case 0xED: case 0xEE: case 0xEF:
			fdc->write(offset, data);
			break;

		case 0xF0:
		case 0xF1:
		case 0xF2:
		case 0xF3:
			acia->write(space, offset & 0x03, data);
			break;
	}
}

INTERRUPT_GEN_MEMBER(apple3_state::apple3_interrupt)
{
	m_via_1->write_ca2((AY3600_keydata_strobe_r(machine()) & 0x80) ? 1 : 0);
	m_via_1->write_cb1(machine().primary_screen->vblank());
	m_via_1->write_cb2(machine().primary_screen->vblank());
}



UINT8 *apple3_state::apple3_bankaddr(UINT16 bank, offs_t offset)
{
	if (bank != (UINT16) ~0)
	{
		bank %= m_ram->size() / 0x8000;
		if ((bank + 1) == (m_ram->size() / 0x8000))
			bank = 0x02;
	}
	offset += ((offs_t) bank) * 0x8000;
	offset %= m_ram->size();
	return &m_ram->pointer()[offset];
}



void apple3_state::apple3_setbank(const char *mame_bank, UINT16 bank, offs_t offset)
{
	UINT8 *ptr;
	ptr = apple3_bankaddr(bank, offset);
	membank(mame_bank)->set_base(ptr);

	#if 0
	if (LOG_MEMORY)
	{
		#ifdef PTR64
		//logerror("\tbank %s --> %02x/%04x [0x%08lx]\n", mame_bank, (unsigned) bank, (unsigned)offset, ptr - m_ram->pointer());
		#else
		logerror("\tbank %s --> %02x/%04x [0x%08lx]\n", mame_bank, (unsigned) bank, (unsigned)offset, ptr - m_ram->pointer());
		#endif
	}
	#endif
}



UINT8 *apple3_state::apple3_get_zpa_addr(offs_t offset)
{
	m_zpa = (((offs_t) m_via_0_b) * 0x100) + offset;

	if (m_via_0_b < 0x20)
		return apple3_bankaddr(~0, m_zpa);
	else if (m_via_0_b > 0x9F)
		return apple3_bankaddr(~0, m_zpa - 0x8000);
	else
		return apple3_bankaddr(m_via_1_a, m_zpa - 0x2000);
}



READ8_MEMBER(apple3_state::apple3_00xx_r)
{
	return *apple3_get_zpa_addr(offset);
}



WRITE8_MEMBER(apple3_state::apple3_00xx_w)
{
	*apple3_get_zpa_addr(offset) = data;
}



void apple3_state::apple3_update_memory()
{
	UINT16 bank;
	UINT8 page;
	address_space& space = m_maincpu->space(AS_PROGRAM);

	if (LOG_MEMORY)
	{
		logerror("apple3_update_memory(): via_0_b=0x%02x via_1_a=0x0x%02x\n", m_via_0_b, m_via_1_a);
	}

	machine().device("maincpu")->set_unscaled_clock((m_via_0_a & 0x80) ? 1000000 : 2000000);

	/* bank 2 (0100-01FF) */
	if (!(m_via_0_a & 0x04))
	{
		if (m_via_0_b < 0x20)
		{
			bank = ~0;  /* system bank */
			page = m_via_0_b ^ 0x01;
		}
		else if (m_via_0_b >= 0xA0)
		{
			bank = ~0;  /* system bank */
			page = (m_via_0_b ^ 0x01) - 0x80;
		}
		else
		{
			bank = m_via_1_a;
			page = (m_via_0_b ^ 0x01) - 0x20;
		}
	}
	else
	{
		bank = ~0;
		page = 0x01;
	}
	apple3_setbank("bank2", bank, ((offs_t) page) * 0x100);

	/* bank 3 (0200-1FFF) */
	apple3_setbank("bank3", ~0, 0x0200);

	/* bank 4 (2000-9FFF) */
	apple3_setbank("bank4", m_via_1_a, 0x0000);

	/* bank 5 (A000-BFFF) */
	apple3_setbank("bank5", ~0, 0x2000);

	/* install bank 8 (C000-CFFF) */
	if (m_via_0_a & 0x40)
	{
		space.install_read_handler(0xC000, 0xC0FF, read8_delegate(FUNC(apple3_state::apple3_c0xx_r),this));
		space.install_write_handler(0xC000, 0xC0FF, write8_delegate(FUNC(apple3_state::apple3_c0xx_w),this));
	}
	else
	{
		space.install_read_bank(0xC000, 0xC0FF, "bank8");
		if (m_via_0_a & 0x08)
			space.unmap_write(0xC000, 0xC0FF);
		else
			space.install_write_bank(0xC000, 0xC0FF, "bank8");
		apple3_setbank("bank8", ~0, 0x4000);
	}

	/* install bank 9 (C100-C4FF) */
	if (m_via_0_a & 0x40)
	{
		space.nop_readwrite(0xC100, 0xC4FF);
	}
	else
	{
		space.install_read_bank(0xC100, 0xC4FF, "bank9");
		if (m_via_0_a & 0x08)
			space.unmap_write(0xC100, 0xC4FF);
		else
			space.install_write_bank(0xC100, 0xC4FF, "bank9");
		apple3_setbank("bank9", ~0, 0x4100);
	}

	/* install bank 10 (C500-C7FF) */
	space.install_read_bank(0xC500, 0xC7FF, "bank10");
	if (m_via_0_a & 0x08)
		space.unmap_write(0xC500, 0xC7FF);
	else
		space.install_write_bank(0xC500, 0xC7FF, "bank10");
	apple3_setbank("bank10", ~0, 0x4500);

	/* install bank 11 (C800-CFFF) */
	if (m_via_0_a & 0x40)
	{
		space.nop_readwrite(0xC800, 0xCFFF);
	}
	else
	{
		space.install_read_bank(0xC800, 0xCFFF, "bank11");
		if (m_via_0_a & 0x08)
			space.unmap_write(0xC800, 0xCFFF);
		else
			space.install_write_bank(0xC800, 0xCFFF, "bank11");
		apple3_setbank("bank11", ~0, 0x4800);
	}

	/* install bank 6 (D000-EFFF) */
	space.install_read_bank(0xD000, 0xEFFF, "bank6");
	if (m_via_0_a & 0x08)
		space.unmap_write(0xD000, 0xEFFF);
	else
		space.install_write_bank(0xD000, 0xEFFF, "bank6");
	apple3_setbank("bank6", ~0, 0x5000);

	/* install bank 7 (F000-FFFF) */
	space.install_read_bank(0xF000, 0xFFFF, "bank7");
	if (m_via_0_a & 0x09)
		space.unmap_write(0xF000, 0xFFFF);
	else
		space.install_write_bank(0xF000, 0xFFFF, "bank7");
	if (m_via_0_a & 0x01)
		membank("bank7")->set_base(memregion("maincpu")->base());
	else
		apple3_setbank("bank7", ~0, 0x7000);

	/* reinstall VIA handlers */
	{
		space.install_readwrite_handler(0xFFD0, 0xFFDF, 0, 0, read8_delegate(FUNC(via6522_device::read),m_via_0.target()), write8_delegate(FUNC(via6522_device::write),m_via_0.target()));
		space.install_readwrite_handler(0xFFE0, 0xFFEF, 0, 0, read8_delegate(FUNC(via6522_device::read),m_via_1.target()), write8_delegate(FUNC(via6522_device::write),m_via_1.target()));
	}
}



void apple3_state::apple3_via_out(UINT8 *var, UINT8 data)
{
	if (*var != data)
	{
		*var = data;
		apple3_update_memory();
	}
}


WRITE8_MEMBER(apple3_state::apple3_via_0_out_a)
{
	apple3_via_out(&m_via_0_a, data);
}

WRITE8_MEMBER(apple3_state::apple3_via_0_out_b)
{
	apple3_via_out(&m_via_0_b, data);
}

WRITE8_MEMBER(apple3_state::apple3_via_1_out_a)
{
	apple3_via_out(&m_via_1_a, data);
}

WRITE8_MEMBER(apple3_state::apple3_via_1_out_b)
{
	apple3_via_out(&m_via_1_b, data);
}

WRITE_LINE_MEMBER(apple3_state::apple2_via_1_irq_func)
{
	if (!m_via_1_irq && state)
	{
		m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
		m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	}
	m_via_1_irq = state;
}



MACHINE_RESET_MEMBER(apple3_state,apple3)
{
}



UINT8 *apple3_state::apple3_get_indexed_addr(offs_t offset)
{
	UINT8 n;
	UINT8 *result = NULL;

	if ((m_via_0_b >= 0x18) && (m_via_0_b <= 0x1F))
	{
		n = *apple3_bankaddr(~0, m_zpa ^ 0x0C00);

		if (LOG_INDXADDR)
		{
			if (m_last_n != n)
			{
				logerror("indxaddr: zpa=0x%04x n=0x%02x\n", m_zpa, n);
				m_last_n = n;
			}
		}

		if (n == 0x8F)
		{
			/* get at that special ram under the VIAs */
			if ((offset >= 0xFFD0) && (offset <= 0xFFEF))
				result = apple3_bankaddr(~0, offset & 0x7FFF);
			else if (offset < 0x2000)
				result = apple3_bankaddr(~0, offset - 0x2000);
			else if (offset > 0x9FFF)
				result = apple3_bankaddr(~0, offset - 0x8000);
			else
				result = &m_ram->pointer()[offset - 0x2000];
		}
		else if ((n >= 0x80) && (n <= 0x8E))
		{
			if (offset < 0x0100)
				result = apple3_bankaddr(~0, ((offs_t) m_via_0_b) * 0x100 + offset);
			else
				result = apple3_bankaddr(n, offset);
		}
		else if (n == 0xFF)
		{
			if (offset < 0x2000)
				result = apple3_bankaddr(~0, offset - 0x2000);
			else if (offset < 0xA000)
				result = apple3_bankaddr(m_via_1_a, offset - 0x2000);
			else if (offset < 0xC000)
				result = apple3_bankaddr(~0, offset - 0x8000);
			else if (offset < 0xD000)
				result = NULL;
			else if (offset < 0xF000)
				result = apple3_bankaddr(~0, offset - 0x8000);
			else
				result = (UINT8 *) ~0;
		}
		else if (offset < 0x0100)
		{
			result = apple3_bankaddr(~0, ((offs_t) m_via_0_b) * 0x100 + offset);
		}
	}
	else if ((offset >= 0xF000) && (m_via_0_a & 0x01))
	{
#if 0
		/* The Apple /// Diagnostics seems to expect that indexed writes
		 * always write to RAM.  That image jumps to an address that is
		 * undefined unless this code is enabled.  However, the Sara
		 * emulator does not have corresponding code here, though Chris
		 * Smolinski does not rule out the possibility
		 */
		result = apple3_bankaddr(~0, offset - 0x8000);
#endif
	}

	return result;
}



READ8_MEMBER(apple3_state::apple3_indexed_read)
{
	UINT8 result;
	UINT8 *addr;
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	addr = apple3_get_indexed_addr(offset);
	if (!addr)
		result = prog_space.read_byte(offset);
	else if (addr != (UINT8 *) ~0)
		result = *addr;
	else
		result = memregion("maincpu")->base()[offset % memregion("maincpu")->bytes()];
	return result;
}



WRITE8_MEMBER(apple3_state::apple3_indexed_write)
{
	UINT8 *addr;
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	addr = apple3_get_indexed_addr(offset);
	if (!addr)
		prog_space.write_byte(offset, data);
	else if (addr != (UINT8 *) ~0)
		*addr = data;
}



DIRECT_UPDATE_MEMBER(apple3_state::apple3_opbase)
{
	UINT8 *opptr;

	if ((address & 0xFF00) == 0x0000)
	{
		opptr = apple3_get_zpa_addr(address);
		direct.explicit_configure(address, address, ~0, opptr - address);
		address = ~0;
	}
	return address;
}



static void apple3_update_drives(device_t *device)
{
	apple3_state *state = device->machine().driver_data<apple3_state>();
	int enable_mask = 0x00;

	if (state->m_enable_mask & 0x01)
		enable_mask |= 0x01;

	if (state->m_enable_mask & 0x02)
	{
		switch(state->m_flags & (VAR_EXTA0 | VAR_EXTA1))
		{
			case VAR_EXTA0:
				enable_mask |= 0x02;
				break;
			case VAR_EXTA1:
				enable_mask |= 0x04;
				break;
			case VAR_EXTA1|VAR_EXTA0:
				enable_mask |= 0x08;
				break;
		}
	}

	apple525_set_enable_lines(device,enable_mask);
}



static void apple3_set_enable_lines(device_t *device,int enable_mask)
{
	apple3_state *state = device->machine().driver_data<apple3_state>();
	state->m_enable_mask = enable_mask;
	apple3_update_drives(device);
}



const applefdc_interface apple3_fdc_interface =
{
	apple525_set_lines,
	apple3_set_enable_lines,
	apple525_read_data,
	apple525_write_data
};



DRIVER_INIT_MEMBER(apple3_state,apple3)
{
	/* hack to get around VIA problem */
	memregion("maincpu")->base()[0x0685] = 0x00;

	m_enable_mask = 0;
	apple3_update_drives(machine().device("fdc"));

	AY3600_init(machine());

	apple3_profile_init();

	m_flags = 0;
	m_via_0_a = ~0;
	m_via_1_a = ~0;
	m_via_1_irq = 0;

	/* these are here to appease the Apple /// confidence tests */
	m_via_1->write_pa0(1);
	m_via_1->write_pa1(1);
	m_via_1->write_pa2(1);
	m_via_1->write_pa3(1);
	m_via_1->write_pa4(1);
	m_via_1->write_pa5(1);
	m_via_1->write_pa6(1);
	m_via_1->write_pa7(1);

	m_via_1->write_pb0(1);
	m_via_1->write_pb1(1);
	m_via_1->write_pb2(1);
	m_via_1->write_pb3(1);
	m_via_1->write_pb4(1);
	m_via_1->write_pb5(1);
	m_via_1->write_pb6(1);
	m_via_1->write_pb7(1);

	apple3_update_memory();

	m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(apple3_state::apple3_opbase), this));
}
