// license:BSD-3-Clause
// copyright-holders:Nathan Woods,R. Belmont
/***************************************************************************

    machine/apple3.c

    Apple ///

    VIA #0 (D VIA)
    CA1: IRQ from the MM58167 RTC
        CA2: 1 if key pressed, 0 otherwise
        CB1/CB2: connected to VBL

        Port A: Environment register (all bits out)
            bit 7: 1 for 1 MHz, 0 for 2 MHz
            bit 6: 1 for I/O at C000-CFFF
            bit 5: 1 to enable video
            bit 4: 1 to enable NMI/Reset
            bit 3: 1 to write-protect RAM in system bank C000-FFFF
            bit 2: 1 to force primary stack at 0100-01FF
            bit 1: 1 for primary ROM, 0 for secondary (Apple III doesn't have a secondary ROM, so this should always be '1' when bit 0 is)
            bit 0: 1 to enable ROM in F000-FFFF

        Port B: Zero page high 8 address bits, also MM58167 RTC register select (all bits out)

    VIA #1 (E VIA)
        CA1: OR of all 4 slots' IRQ status
        CA2: SW1 (Open Apple key?)
        CB1: SW3/SCO
        CB2: SER

        Port A:
            bits 0-2: bank select for $2000-$9FFF range
            bit 3: n/c
            bit 4: slot 4 IRQ (in)
            bit 5: slot 3 IRQ (in)
            bit 6: Apple II mode trap output (out)
            bit 7: IRQ status (in) (0 = IRQ, 1 = no IRQ)

        Port B:
            bits 0-5: 6-bit audio DAC output
            bit 6: screen blank
            bit 7: OR of NMI from slots

***************************************************************************/

#include "emu.h"
#include "includes/apple3.h"
#include "includes/apple2.h"

#define LOG_MEMORY      1
#define LOG_INDXADDR    1

#define ENV_SLOWSPEED   (0x80)
#define ENV_IOENABLE    (0x40)
#define ENV_VIDENABLE   (0x20)
#define ENV_NMIENABLE   (0x10)
#define ENV_WRITEPROT   (0x08)
#define ENV_STACK1XX    (0x04)
#define ENV_PRIMARYROM  (0x02)
#define ENV_ROMENABLE   (0x01)

READ8_MEMBER(apple3_state::apple3_c0xx_r)
{
	UINT8 result = 0xFF;
	device_a2bus_card_interface *slotdevice;

	switch(offset)
	{
		/* keystrobe */
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			result = (m_transchar & 0x7f) | m_strobe;
			break;

		/* modifier keys */
		case 0x08: case 0x09: case 0x0A: case 0x0B:
		case 0x0C: case 0x0D: case 0x0E: case 0x0F:
			{
				UINT8 tmp = m_kbspecial->read();

				result = 0x7c | (m_transchar & 0x80);

				if (m_strobe)
				{
					result |= 1;
				}

				if (tmp & 0x06)
				{
					result |= 0x02;
				}
				if (tmp & 0x08)
				{
					result &= ~0x04;
				}
				if (tmp & 0x01)
				{
					result &= ~0x08;
				}
				if (tmp & 0x10)
				{
					result &= ~0x10;
				}
				if (tmp & 0x20)
				{
					result &= ~0x20;
				}
			}
//          printf("modifier = %02x\n", result);
			break;

		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1A: case 0x1B:
		case 0x1C: case 0x1D: case 0x1E: case 0x1F:
			m_strobe = 0;
			break;

		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2A: case 0x2B:
		case 0x2C: case 0x2D: case 0x2E: case 0x2F:
			m_cnxx_slot = -1;
			break;

		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3A: case 0x3B:
		case 0x3C: case 0x3D: case 0x3E: case 0x3F:
			m_speaker_state ^= 1;
			m_speaker->level_w(m_speaker_state);
			break;

		case 0x40: case 0x41: case 0x42: case 0x43:
		case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4A: case 0x4B:
		case 0x4C: case 0x4D: case 0x4E: case 0x4F:
			m_c040_time = 200;
			break;

		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
			/* graphics softswitches */
			if (offset & 1)
				m_flags |= 1 << ((offset - 0x50) / 2);
			else
				m_flags &= ~(1 << ((offset - 0x50) / 2));
			break;

		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:
			pdl_handler(offset);
			break;

		case 0x60:  // joystick switch 0
		case 0x68:
			result = (m_joybuttons->read() & 1) ? 0x80 : 0x00;
			break;

		case 0x61:  // joystick switch 1 (margin switch for Silentype)
		case 0x69:
			result = (m_joybuttons->read() & 4) ? 0x80 : 0x00;
			break;

		case 0x62:  // joystick switch 2
		case 0x6a:
			result = (m_joybuttons->read() & 2) ? 0x80 : 0x00;
			break;

		case 0x63:  // joystick switch 3 (serial clock for silentype)
		case 0x6b:
			result = (m_joybuttons->read() & 8) ? 0x80 : 0x00;
			break;

		case 0x66:  // paddle A/D conversion done (bit 7 = 1 while counting, 0 when done)
		case 0x6e:
			result = m_ramp_active ? 0x80 : 0x00;
			break;

		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B:
		case 0x7C: case 0x7D: case 0x7E: case 0x7F:
			result = m_rtc->read(space, m_via_0_b);
			break;

		case 0x90: case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			slotdevice = m_a2bus->get_a2bus_card(1);
			if (slotdevice != nullptr)
			{
				result = slotdevice->read_c0nx(space, offset&0xf);
			}
			break;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
		case 0xac: case 0xad: case 0xae: case 0xaf:
			slotdevice = m_a2bus->get_a2bus_card(2);
			if (slotdevice != nullptr)
			{
				result = slotdevice->read_c0nx(space, offset&0xf);
			}
			break;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			slotdevice = m_a2bus->get_a2bus_card(3);
			if (slotdevice != nullptr)
			{
				result = slotdevice->read_c0nx(space, offset&0xf);
			}
			break;

		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
		case 0xcc: case 0xcd: case 0xce: case 0xcf:
			slotdevice = m_a2bus->get_a2bus_card(4);
			if (slotdevice != nullptr)
			{
				result = slotdevice->read_c0nx(space, offset&0xf);
			}
			break;

		case 0xD0: case 0xD1: case 0xD2: case 0xD3:
		case 0xD4: case 0xD5: case 0xD6: case 0xD7:
			/* external drive stuff */
			m_fdc->read_c0dx(space, offset&0xf);
			result = 0x00;
			break;

		case 0xd8: case 0xd9:
			m_smoothscr = offset & 1;
			break;

		case 0xDB:
			apple3_write_charmem();
			break;

		case 0xE0: case 0xE1:
			result = m_fdc->read(space, offset&0xf);
			m_va = offset & 1;
			break;

		case 0xE2: case 0xE3:
			result = m_fdc->read(space, offset&0xf);
			m_vb = offset & 1;
			break;

		case 0xE4: case 0xE5:
			result = m_fdc->read(space, offset&0xf);
			m_vc = offset & 1;
			break;

		case 0xE6: case 0xE7: case 0xE8: case 0xE9:
		case 0xEA: case 0xEB: case 0xEC: case 0xED:
		case 0xEE: case 0xEF:
			result = m_fdc->read(space, offset&0xf);
			break;

		case 0xF0:
		case 0xF1:
		case 0xF2:
		case 0xF3:
			result = m_acia->read(space, offset & 0x03);
			break;
	}
	return result;
}



WRITE8_MEMBER(apple3_state::apple3_c0xx_w)
{
	device_a2bus_card_interface *slotdevice;

	switch(offset)
	{
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1A: case 0x1B:
		case 0x1C: case 0x1D: case 0x1E: case 0x1F:
			m_strobe = 0;
			break;

		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2A: case 0x2B:
		case 0x2C: case 0x2D: case 0x2E: case 0x2F:
			m_cnxx_slot = -1;
			break;

		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3A: case 0x3B:
		case 0x3C: case 0x3D: case 0x3E: case 0x3F:
			m_speaker_state ^= 1;
			m_speaker->level_w(m_speaker_state);
			break;

		case 0x40: case 0x41: case 0x42: case 0x43:
		case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4A: case 0x4B:
		case 0x4C: case 0x4D: case 0x4E: case 0x4F:
			m_c040_time = 200;
			break;

		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
			/* graphics softswitches */
			if (offset & 1)
				m_flags |= 1 << ((offset - 0x50) / 2);
			else
				m_flags &= ~(1 << ((offset - 0x50) / 2));
			break;

		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:
			pdl_handler(offset);
			break;

		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B:
		case 0x7C: case 0x7D: case 0x7E: case 0x7F:
			m_rtc->write(space, m_via_0_b, data);
			break;


		case 0x90: case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			slotdevice = m_a2bus->get_a2bus_card(1);
			if (slotdevice != nullptr)
			{
				slotdevice->write_c0nx(space, offset&0xf, data);
			}
			break;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
		case 0xac: case 0xad: case 0xae: case 0xaf:
			slotdevice = m_a2bus->get_a2bus_card(2);
			if (slotdevice != nullptr)
			{
				slotdevice->write_c0nx(space, offset&0xf, data);
			}
			break;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			slotdevice = m_a2bus->get_a2bus_card(3);
			if (slotdevice != nullptr)
			{
				slotdevice->write_c0nx(space, offset&0xf, data);
			}
			break;

		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
		case 0xcc: case 0xcd: case 0xce: case 0xcf:
			slotdevice = m_a2bus->get_a2bus_card(4);
			if (slotdevice != nullptr)
			{
				slotdevice->write_c0nx(space, offset&0xf, data);
			}
			break;

		case 0xD0: case 0xD1: case 0xD2: case 0xD3:
		case 0xD4: case 0xD5: case 0xD6: case 0xD7:
			/* external drive stuff */
			m_fdc->write_c0dx(space, offset&0xf, data);
			break;

		case 0xd9:
			popmessage("Smooth scroll enabled, contact MESSdev");
			break;

		case 0xDB:
			apple3_write_charmem();
			break;

		case 0xE0: case 0xE1: case 0xE2: case 0xE3:
		case 0xE4: case 0xE5: case 0xE6: case 0xE7:
		case 0xE8: case 0xE9: case 0xEA: case 0xEB:
		case 0xEC: case 0xED: case 0xEE: case 0xEF:
			m_fdc->write(space, offset&0xf, data);
			break;

		case 0xF0:
		case 0xF1:
		case 0xF2:
		case 0xF3:
			m_acia->write(space, offset & 0x03, data);
			break;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(apple3_state::apple3_interrupt)
{
	m_via_1->write_cb1(machine().first_screen()->vblank());
	m_via_1->write_cb2(machine().first_screen()->vblank());
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

void apple3_state::apple3_update_memory()
{
	UINT16 bank;
	UINT8 page;

	if (LOG_MEMORY)
	{
		logerror("apple3_update_memory(): via_0_b=0x%02x via_1_a=0x0x%02x\n", m_via_0_b, m_via_1_a);
	}

	machine().device("maincpu")->set_unscaled_clock((m_via_0_a & ENV_SLOWSPEED) ? 1021800 : 2000000);

	/* bank 2 (0100-01FF) */
	if (!(m_via_0_a & ENV_STACK1XX))
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
	m_bank2 = apple3_bankaddr(bank, ((offs_t) page) * 0x100);

	/* bank 3 (0200-1FFF) */
	m_bank3 = apple3_bankaddr(~0, 0x0200);

	/* bank 4 (2000-9FFF) */
	m_bank4 = apple3_bankaddr(m_via_1_a, 0x0000);

	/* bank 5 (A000-BFFF) */
	m_bank5 = apple3_bankaddr(~0, 0x2000);

	/* bank 8 (C000-C0FF) */
	if (!(m_via_0_a & ENV_IOENABLE))
	{
		m_bank8 = apple3_bankaddr(~0, 0x4000);
	}

	/* bank 9 (C100-C4FF) */
	if (!(m_via_0_a & ENV_IOENABLE))
	{
		m_bank9 = apple3_bankaddr(~0, 0x4100);
	}

	/* bank 10 (C500-C7FF) */
	m_bank10 = apple3_bankaddr(~0, 0x4500);

	/* bank 11 (C800-CFFF) */
	if (!(m_via_0_a & ENV_IOENABLE))
	{
		m_bank11 = apple3_bankaddr(~0, 0x4800);
	}

	/* install bank 6 (D000-EFFF) */
	m_bank6 = apple3_bankaddr(~0, 0x5000);

	/* install bank 7 (F000-FFFF) */
	m_bank7wr = apple3_bankaddr(~0, 0x7000);
	if (m_via_0_a & ENV_ROMENABLE)
	{
		m_bank7rd = memregion("maincpu")->base();
	}
	else
	{
		m_rom_has_been_disabled = true;
		m_bank7rd = m_bank7wr;

		// if we had an IRQ waiting for RAM to be paged in...
		apple3_irq_update();
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
//  printf("ZP to %02x\n", data);
	apple3_via_out(&m_via_0_b, data);
}

WRITE8_MEMBER(apple3_state::apple3_via_1_out_a)
{
	apple3_via_out(&m_via_1_a, data);
}

WRITE8_MEMBER(apple3_state::apple3_via_1_out_b)
{
	m_dac->write_unsigned8(data<<2);
	apple3_via_out(&m_via_1_b, data);
}

void apple3_state::apple3_irq_update()
{
	if (m_acia_irq || m_via_1_irq || m_via_0_irq)
	{
		// HACK: SOS floppy driver enables ROM at Fxxx *before* trying to
		// suppress IRQs.  IRQ hits at inopportune time -> bad vector -> system crash.
		// This breaks the Confidence Test, but the Confidence Test
		// never disables the ROM so checking for that gets us
		// working in all cases.
		// Bonus points: for some reason this isn't a problem with -debug.
		// m6502 heisenbug maybe?
		if ((m_via_0_a & ENV_ROMENABLE) && (m_rom_has_been_disabled))
		{
			return;
		}
//      printf("   setting IRQ\n");
		m_maincpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
		m_via_1->write_pa7(0);  // this is active low
	}
	else
	{
//      printf("   clearing IRQ\n");
		m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
		m_via_1->write_pa7(1);
	}
}

WRITE_LINE_MEMBER(apple3_state::apple3_acia_irq_func)
{
//  printf("acia IRQ: %d\n", state);
	m_acia_irq = state;
	apple3_irq_update();
}

WRITE_LINE_MEMBER(apple3_state::apple3_via_1_irq_func)
{
//  printf("via 1 IRQ: %d\n", state);
	m_via_1_irq = state;
	apple3_irq_update();
}


WRITE_LINE_MEMBER(apple3_state::apple3_via_0_irq_func)
{
//  printf("via 0 IRQ: %d\n", state);
	m_via_0_irq = state;
	apple3_irq_update();
}

MACHINE_RESET_MEMBER(apple3_state,apple3)
{
	m_indir_bank = 0;
	m_sync = false;
	m_speaker_state = 0;
	m_speaker->level_w(0);
	m_c040_time = 0;
	m_strobe = 0;
	m_lastchar = 0x0d;
	m_rom_has_been_disabled = false;
	m_cnxx_slot = -1;
	m_analog_sel = 0;
	m_ramp_active = false;

	m_fdc->set_floppies_4(floppy0, floppy1, floppy2, floppy3);
}



UINT8 *apple3_state::apple3_get_indexed_addr(offs_t offset)
{
	UINT8 *result = nullptr;

	// m_indir_bank is guaranteed to be between 0x80 and 0x8f
	if (m_indir_bank == 0x8f)
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
	else
	{
		result = apple3_bankaddr(m_indir_bank, offset);
	}

	return result;
}

DRIVER_INIT_MEMBER(apple3_state,apple3)
{
	m_enable_mask = 0;

	m_flags = 0;
	m_acia_irq = 0;
	m_via_0_a = ~0;
	m_via_1_a = ~0;
	m_via_0_irq = 0;
	m_via_1_irq = 0;
	m_va = 0;
	m_vb = 0;
	m_vc = 0;
	m_smoothscr = 0;

	// kludge round +12v pull up resistors, which after conversion will bring this low when nothing is plugged in. issue also affects dcd/dsr but those don't affect booting.
	m_acia->write_cts(0);

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

	save_item(NAME(m_acia_irq));
	save_item(NAME(m_via_0_a));
	save_item(NAME(m_via_0_b));
	save_item(NAME(m_via_1_a));
	save_item(NAME(m_via_1_b));
	save_item(NAME(m_via_0_irq));
	save_item(NAME(m_via_1_irq));
	save_item(NAME(m_zpa));
	save_item(NAME(m_last_n));
	save_item(NAME(m_sync));
	save_item(NAME(m_rom_has_been_disabled));
	save_item(NAME(m_indir_bank));
	save_item(NAME(m_cnxx_slot));
	save_item(NAME(m_speaker_state));
	save_item(NAME(m_c040_time));
	save_item(NAME(m_lastchar));
	save_item(NAME(m_strobe));
	save_item(NAME(m_transchar));
	save_item(NAME(m_flags));
	save_item(NAME(m_char_mem));
	save_item(NAME(m_analog_sel));
	save_item(NAME(m_ramp_active));
	save_item(NAME(m_pdl_charge));
	save_item(NAME(m_va));
	save_item(NAME(m_vb));
	save_item(NAME(m_vc));
	save_item(NAME(m_smoothscr));

	machine().save().register_postload(save_prepost_delegate(FUNC(apple3_state::apple3_postload), this));
}

void apple3_state::apple3_postload()
{
	apple3_update_memory();
}

READ8_MEMBER(apple3_state::apple3_memory_r)
{
	UINT8 rv = 0xff;

	// (zp), y or (zp,x) read
	if (!space.debugger_access())
	{
		if ((m_indir_bank & 0x80) && (offset >= 0x100))
		{
			UINT8 *test;
			test = apple3_get_indexed_addr(offset);

			if (test)
			{
				return *test;
			}
		}
	}

	if (offset < 0x100)
	{
		rv = *apple3_get_zpa_addr(offset);

		if ((!m_sync) && (m_via_0_b >= 0x18) && (m_via_0_b <= 0x1F))
		{
			// fetch the "X byte"
			m_indir_bank = *apple3_bankaddr(~0, m_zpa ^ 0x0C00) & 0x8f;
		}
	}
	else if (offset < 0x200)
	{
		rv = m_bank2[offset-0x100];
	}
	else if (offset < 0x2000)
	{
		rv = m_bank3[offset-0x200];
	}
	else if (offset < 0xa000)
	{
		rv = m_bank4[offset-0x2000];
	}
	else if (offset < 0xc000)
	{
		rv = m_bank5[offset-0xa000];
	}
	else if (offset < 0xc100)
	{
		if (m_via_0_a & ENV_IOENABLE)
		{
			if (!space.debugger_access())
			{
				rv = apple3_c0xx_r(space, offset-0xc000);
			}
		}
		else
		{
			rv = m_bank8[offset - 0xc000];
		}
	}
	else if (offset < 0xc500)
	{
		if (!(m_via_0_a & ENV_IOENABLE))
		{
			rv = m_bank9[offset - 0xc100];
		}
		else
		{
			/* now identify the device */
			device_a2bus_card_interface *slotdevice = m_a2bus->get_a2bus_card((offset>>8) & 0x7);

			if (slotdevice != nullptr)
			{
				if (slotdevice->take_c800())
				{
					m_cnxx_slot = ((offset>>8) & 7);
				}

				return slotdevice->read_cnxx(space, offset&0xff);
			}
		}
	}
	else if (offset < 0xc800)
	{
		rv = m_bank10[offset - 0xc500];
	}
	else if (offset < 0xd000)
	{
		if (!(m_via_0_a & ENV_IOENABLE))
		{
			rv = m_bank11[offset - 0xc800];
		}
		else
		{
			if (offset == 0xcfff)
			{
				m_cnxx_slot = -1;
			}

			if (m_cnxx_slot != -1)
			{
				device_a2bus_card_interface *slotdevice = m_a2bus->get_a2bus_card(m_cnxx_slot);

				if (slotdevice != nullptr)
				{
					rv = slotdevice->read_c800(space, offset&0x7ff);
				}
			}
		}
	}
	else if (offset < 0xf000)
	{
		rv = m_bank6[offset - 0xd000];
	}
	else
	{
		if (offset >= 0xffd0 && offset <= 0xffdf)
		{
			rv = m_via_0->read(space, offset);
		}
		else if (offset >= 0xffe0 && offset <= 0xffef)
		{
			rv = m_via_1->read(space, offset);
		}
		else
		{
			rv = m_bank7rd[offset - 0xf000];
		}
	}

	return rv;
}

WRITE8_MEMBER(apple3_state::apple3_memory_w)
{
	if ((m_indir_bank & 0x80) && (offset >= 0x100))
	{
		UINT8 *test;
		test = apple3_get_indexed_addr(offset);

		if (test)
		{
			*test = data;
			return;
		}
	}

	if (offset < 0x100)
	{
		*apple3_get_zpa_addr(offset) = data;
	}
	else if (offset < 0x200)
	{
		m_bank2[offset-0x100] = data;
	}
	else if (offset < 0x2000)
	{
		m_bank3[offset-0x200] = data;
	}
	else if (offset < 0xa000)
	{
		m_bank4[offset-0x2000] = data;
	}
	else if (offset < 0xc000)
	{
		m_bank5[offset-0xa000] = data;
	}
	else if (offset < 0xc100)
	{
		if (m_via_0_a & ENV_IOENABLE)
		{
			if (!space.debugger_access())
			{
				apple3_c0xx_w(space, offset-0xc000, data);
			}
		}
		else
		{
			// is this page write protected?
			if (!(m_via_0_a & ENV_WRITEPROT))
			{
				m_bank8[offset - 0xc000] = data;
			}
		}
	}
	else if (offset < 0xc500)
	{
		if (!(m_via_0_a & ENV_IOENABLE))
		{
			if (!(m_via_0_a & ENV_WRITEPROT))
			{
				m_bank9[offset - 0xc100] = data;
			}
		}
		else
		{
			/* now identify the device */
			device_a2bus_card_interface *slotdevice = m_a2bus->get_a2bus_card((offset>>8) & 0x7);

			if (slotdevice != nullptr)
			{
				if (slotdevice->take_c800())
				{
					m_cnxx_slot = ((offset>>8) & 7);
				}

				slotdevice->write_cnxx(space, offset&0xff, data);
			}
		}
	}
	else if (offset < 0xc800)
	{
		if (!(m_via_0_a & ENV_WRITEPROT))
		{
			m_bank10[offset - 0xc500] = data;
		}
	}
	else if (offset < 0xd000)
	{
		if (!(m_via_0_a & ENV_IOENABLE))
		{
			if (!(m_via_0_a & ENV_WRITEPROT))
			{
				m_bank11[offset - 0xc800] = data;
			}
		}
		else
		{
			if (offset == 0xcfff)
			{
				m_cnxx_slot = -1;
			}

			if (m_cnxx_slot != -1)
			{
				device_a2bus_card_interface *slotdevice = m_a2bus->get_a2bus_card(m_cnxx_slot);

				if (slotdevice != nullptr)
				{
					slotdevice->write_c800(space, offset&0x7ff, data);
				}
			}
		}
	}
	else if (offset < 0xf000)
	{
		if (!(m_via_0_a & ENV_WRITEPROT))
		{
			m_bank6[offset - 0xd000] = data;
		}
	}
	else
	{
		if (offset >= 0xffd0 && offset <= 0xffdf)
		{
			if (!space.debugger_access())
			{
				m_via_0->write(space, offset, data);
			}
		}
		else if (offset >= 0xffe0 && offset <= 0xffef)
		{
			if (!space.debugger_access())
			{
				m_via_1->write(space, offset, data);
			}
		}
		else
		{
			if (!(m_via_0_a & ENV_WRITEPROT))
			{
				m_bank7wr[offset - 0xf000] = data;
			}
		}
	}
}

WRITE_LINE_MEMBER(apple3_state::apple3_sync_w)
{
//  printf("sync: %d\n", state);
	m_sync = (state == ASSERT_LINE) ? true : false;

	if (m_sync)
	{
		m_indir_bank = 0;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(apple3_state::apple3_c040_tick)
{
	if (m_c040_time > 0)
	{
		m_speaker_state ^= 1;
		m_speaker->level_w(m_speaker_state);
		m_c040_time--;
	}
}

READ_LINE_MEMBER(apple3_state::ay3600_shift_r)
{
	// either shift key
	if (m_kbspecial->read() & 0x06)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

READ_LINE_MEMBER(apple3_state::ay3600_control_r)
{
	if (m_kbspecial->read() & 0x08)
	{
		return ASSERT_LINE;
	}

	return CLEAR_LINE;
}

static const UINT8 key_remap[0x50][4] =
{
/*    norm shft ctrl both */
	{ 0x9b,0x9b,0x9b,0x9b },    /* Escape  00     */
	{ 0x31,0x21,0x31,0x31 },    /* 1 !     01     */
	{ 0x32,0x40,0x32,0x00 },    /* 2 @     02     */
	{ 0x33,0x23,0x33,0x23 },    /* 3 #     03     */
	{ 0x34,0x24,0x34,0x24 },    /* 4 $     04     */
	{ 0x35,0x25,0x35,0x25 },    /* 5 %     05     */
	{ 0x36,0x5e,0x35,0x53 },    /* 6 ^     06     */
	{ 0x37,0x26,0x37,0x26 },    /* 7 &     07     */
	{ 0x38,0x2a,0x38,0x2a },    /* 8 *     08     */
	{ 0x39,0x28,0x39,0x28 },    /* 9 (     09     */
	{ 0x89,0x89,0x89,0x89 },    /* Tab     0a     */
	{ 0x51,0x51,0x11,0x11 },    /* q Q     0b     */
	{ 0x57,0x57,0x17,0x17 },    /* w W     0c     */
	{ 0x45,0x45,0x05,0x05 },    /* e E     0d     */
	{ 0x52,0x52,0x12,0x12 },    /* r R     0e     */
	{ 0x54,0x54,0x14,0x14 },    /* t T     0f     */
	{ 0x59,0x59,0x19,0x19 },    /* y Y     10     */
	{ 0x55,0x55,0x15,0x15 },    /* u U     11     */
	{ 0x49,0x49,0x09,0x09 },    /* i I     12     */
	{ 0x4f,0x4f,0x0f,0x0f },    /* o O     13     */
	{ 0x41,0x41,0x01,0x01 },    /* a A     14     */
	{ 0x53,0x53,0x13,0x13 },    /* s S     15     */
	{ 0x44,0x44,0x04,0x04 },    /* d D     16     */
	{ 0x46,0x46,0x06,0x06 },    /* f F     17     */
	{ 0x48,0x48,0x08,0x08 },    /* h H     18     */
	{ 0x47,0x47,0x07,0x07 },    /* g G     19     */
	{ 0x4a,0x4a,0x0a,0x0a },    /* j J     1a     */
	{ 0x4b,0x4b,0x0b,0x0b },    /* k K     1b     */
	{ 0x3b,0x3a,0x3b,0x3a },    /* ; :     1c     */
	{ 0x4c,0x4c,0x0c,0x0c },    /* l L     1d     */
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
	{ 0x00,0x00,0x00,0x00 },    /* 0x28 unused    */
	{ 0xb9,0xb9,0xb9,0xb9 },    /* 9 (KP)  29     */
	{ 0x00,0x00,0x00,0x00 },    /* 0x2a unused    */
	{ 0xb8,0xb8,0xb8,0xb8 },    /* 8 (KP)  2b     */
	{ 0x00,0x00,0x00,0x00 },    /* 0x2c unused    */
	{ 0xb7,0xb7,0xb7,0xb7 },    /* 7 (KP)  2d     */
	{ 0x5c,0x7c,0x7f,0x1c },    /* \ |     2e     */
	{ 0x3d,0x2b,0x3d,0x2b },    /* = +     2f     */
	{ 0x30,0x29,0x30,0x29 },    /* 0 )     30     */
	{ 0x2d,0x5f,0x2d,0x1f },    /* - _     31     */
	{ 0x00,0x00,0x00,0x00 },    /* 0x32 unused    */
	{ 0xb6,0xb6,0xb6,0xb6 },    /* 6 (KP)  33     */
	{ 0x00,0x00,0x00,0x00 },    /* 0x34 unused    */
	{ 0xb5,0xb5,0xb5,0xb5 },    /* 5 (KP)  35     */
	{ 0x00,0x00,0x00,0x00 },    /* 0x36 unused    */
	{ 0xb4,0xb4,0xb4,0xb4 },    /* 4 (KP)  37     */
	{ 0x60,0x7e,0x60,0x7e },    /* ` ~     38     */
	{ 0x5d,0x7d,0x1d,0x1d },    /* ] }     39     */
	{ 0x50,0x50,0x10,0x10 },    /* p P     3a     */
	{ 0x5b,0x7b,0x1b,0x1b },    /* [ {     3b     */
	{ 0x00,0x00,0x00,0x00 },    /* 0x3c unused    */
	{ 0xb3,0xb3,0xb3,0xb3 },    /* 3 (KP)  3d     */
	{ 0xae,0xae,0xae,0xae },    /* . (KP)  3e     */
	{ 0xb2,0xb2,0xb2,0xb2 },    /* 2 (KP)  3f     */
	{ 0xb0,0xb0,0xb0,0xb0 },    /* 0 (KP)  40     */
	{ 0xb1,0xb1,0xb1,0xb1 },    /* 1 (KP)  41     */
	{ 0x0d,0x0d,0x0d,0x0d },    /* Enter   42     */
	{ 0x8b,0x8b,0x8b,0x8b },    /* Up      43     */
	{ 0x00,0x00,0x00,0x00 },    /* 0x44 unused    */
	{ 0x27,0x22,0x27,0x22 },    /* ' "     45     */
	{ 0x00,0x00,0x00,0x00 },    /* 0x46 unused    */
	{ 0x00,0x00,0x00,0x00 },    /* 0x47 unused    */
	{ 0x8d,0x8d,0x8d,0x8d },    /* Ent(KP) 48     */
	{ 0xa0,0xa0,0xa0,0xa0 },    /* Space   49     */
	{ 0x00,0x00,0x00,0x00 },    /* 0x4a unused    */
	{ 0xad,0xad,0xad,0xad },    /* - (KP)  4b     */
	{ 0x95,0x95,0x95,0x95 },    /* Right   4c     */
	{ 0x8a,0x8a,0x8a,0x8a },    /* Down    4d     */
	{ 0x88,0x88,0x88,0x88 },    /* Left    4e     */
	{ 0x00,0x00,0x00,0x00 }     /* 0x4f unused    */
};

WRITE_LINE_MEMBER(apple3_state::ay3600_data_ready_w)
{
	m_via_1->write_ca2(state);

	if (state == ASSERT_LINE)
	{
		UINT16 trans;
		int mod = 0;
		m_lastchar = m_ay3600->b_r();

		trans = m_lastchar & ~(0x1c0);  // clear the 3600's control/shift stuff
		trans |= (m_lastchar & 0x100)>>2;   // bring the 0x100 bit down to the 0x40 place

		mod = (m_kbspecial->read() & 0x06) ? 0x01 : 0x00;
		mod |= (m_kbspecial->read() & 0x08) ? 0x02 : 0x00;

		m_transchar = key_remap[trans][mod];

		if (m_transchar != 0)
		{
			m_strobe = 0x80;
//          printf("new char = %04x (%02x)\n", trans, m_transchar);
		}
	}
}

void apple3_state::pdl_handler(int offset)
{
	UINT8 pdlread;

	switch (offset)
	{
		case 0x58:
			m_analog_sel &= ~1;
			break;

		case 0x59:
			m_analog_sel |= 1;
			break;

		case 0x5a:
			m_analog_sel &= ~4;
			break;

		case 0x5b:
			m_analog_sel |= 4;
			break;

		case 0x5c:
			m_ramp_active = false;
			m_pdl_charge = 0;
			m_pdltimer->adjust(attotime::from_hz(1000000.0));
			break;

		case 0x5d:
			switch (m_analog_sel)
			{
				case 1:
					pdlread = m_joy1x->read();
					break;

				case 2:
					pdlread = m_joy1y->read();
					break;

				case 4:
					pdlread = m_joy2x->read();
					break;

				case 5:
					pdlread = m_joy2y->read();
					break;

				default:
					pdlread = 127;
					break;
			}

			// help the ROM self-test
			if (m_pdl_charge > 82)
			{
				m_pdl_charge += (pdlread*4);
				m_pdl_charge -= 93;
			}
			m_pdltimer->adjust(attotime::from_hz(1000000.0));
			m_ramp_active = true;
			break;

		case 0x5e:
			m_analog_sel &= ~2;
			break;

		case 0x5f:
			m_analog_sel |= 2;
			break;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(apple3_state::paddle_timer)
{
	if (m_ramp_active)
	{
		m_pdl_charge--;

		if (m_pdl_charge > 0)
		{
			m_pdltimer->adjust(attotime::from_hz(1000000.0));
		}
		else
		{
			m_pdltimer->adjust(attotime::never);
			m_ramp_active = false;
		}
	}
	else
	{
		m_pdl_charge++;
		m_pdltimer->adjust(attotime::from_hz(1000000.0));
	}
}

WRITE_LINE_MEMBER(apple3_state::a2bus_irq_w)
{
	UINT8 irq_mask = m_a2bus->get_a2bus_irq_mask();

	m_via_1->write_ca1(state);
	m_via_1->write_pa7(state);

	if (irq_mask & (1<<4))
	{
		m_via_1->write_pa4(ASSERT_LINE);
	}
	else
	{
		m_via_1->write_pa4(CLEAR_LINE);
	}

	if (irq_mask & (1<<3))
	{
		m_via_1->write_pa5(ASSERT_LINE);
	}
	else
	{
		m_via_1->write_pa5(CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER(apple3_state::a2bus_nmi_w)
{
	m_via_1->write_pb7(state);

	if (m_via_0_a & ENV_NMIENABLE)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, state);
	}
}
