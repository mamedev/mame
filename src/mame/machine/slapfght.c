// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/***************************************************************************

  Toaplan Slap Fight hardware

  Functions to emulate (and simulate) the MCU

***************************************************************************/

#include "emu.h"
#include "includes/slapfght.h"


/***************************************************************************

    Tiger Heli MCU

***************************************************************************/

READ8_MEMBER(slapfght_state::tigerh_mcu_r)
{
	m_mcu_sent = false;
	return m_from_mcu;
}

WRITE8_MEMBER(slapfght_state::tigerh_mcu_w)
{
	m_from_main = data;
	m_main_sent = true;
	m_mcu->set_input_line(0, ASSERT_LINE);
}


/**************************************************************************/

READ8_MEMBER(slapfght_state::tigerh_mcu_status_r)
{
	// d0 is vblank
	UINT8 res = m_screen->vblank() ? 1 : 0;

	if (!m_main_sent)
		res |= 0x02;
	if (!m_mcu_sent)
		res |= 0x04;

	return res;
}


/**************************************************************************/

READ8_MEMBER(slapfght_state::tigerh_68705_portA_r)
{
	return (m_portA_out & m_ddrA) | (m_portA_in & ~m_ddrA);
}

WRITE8_MEMBER(slapfght_state::tigerh_68705_portA_w)
{
	m_portA_out = data;
}

WRITE8_MEMBER(slapfght_state::tigerh_68705_ddrA_w)
{
	m_ddrA = data;
}

READ8_MEMBER(slapfght_state::tigerh_68705_portB_r)
{
	return (m_portB_out & m_ddrB) | (m_portB_in & ~m_ddrB);
}

WRITE8_MEMBER(slapfght_state::tigerh_68705_portB_w)
{
	if ((m_ddrB & 0x02) && (~data & 0x02) && (m_portB_out & 0x02))
	{
		if (m_main_sent)
			m_mcu->set_input_line(0, CLEAR_LINE);

		m_portA_in = m_from_main;
		m_main_sent = false;
	}
	if ((m_ddrB & 0x04) && (data & 0x04) && (~m_portB_out & 0x04))
	{
		m_from_mcu = m_portA_out;
		m_mcu_sent = true;
	}

	m_portB_out = data;
}

WRITE8_MEMBER(slapfght_state::tigerh_68705_ddrB_w)
{
	m_ddrB = data;
}


READ8_MEMBER(slapfght_state::tigerh_68705_portC_r)
{
	m_portC_in = 0;

	if (!m_main_sent)
		m_portC_in |= 0x01;
	if (m_mcu_sent)
		m_portC_in |= 0x02;

	return (m_portC_out & m_ddrC) | (m_portC_in & ~m_ddrC);
}

WRITE8_MEMBER(slapfght_state::tigerh_68705_portC_w)
{
	m_portC_out = data;
}

WRITE8_MEMBER(slapfght_state::tigerh_68705_ddrC_w)
{
	m_ddrC = data;
}



/***************************************************************************

    Slap Fight MCU

***************************************************************************/

READ8_MEMBER(slapfght_state::slapfight_68705_portA_r)
{
	return (m_portA_out & m_ddrA) | (m_portA_in & ~m_ddrA);
}

WRITE8_MEMBER(slapfght_state::slapfight_68705_portA_w)
{
	m_portA_out = data;
}

WRITE8_MEMBER(slapfght_state::slapfight_68705_ddrA_w)
{
	m_ddrA = data;
}

READ8_MEMBER(slapfght_state::slapfight_68705_portB_r)
{
	return (m_portB_out & m_ddrB) | (m_portB_in & ~m_ddrB);
}

WRITE8_MEMBER(slapfght_state::slapfight_68705_portB_w)
{
	if ((m_ddrB & 0x02) && (~data & 0x02) && (m_portB_out & 0x02))
	{
		if (m_main_sent)
			m_mcu->set_input_line(0, CLEAR_LINE);

		m_portA_in = m_from_main;
		m_main_sent = false;
	}
	if ((m_ddrB & 0x04) && (data & 0x04) && (~m_portB_out & 0x04))
	{
		m_from_mcu = m_portA_out;
		m_mcu_sent = true;
	}
	if ((m_ddrB & 0x08) && (~data & 0x08) && (m_portB_out & 0x08))
	{
		m_scrollx_lo = m_portA_out;
	}
	if ((m_ddrB & 0x10) && (~data & 0x10) && (m_portB_out & 0x10))
	{
		m_scrollx_hi = m_portA_out;
	}

	m_portB_out = data;
}

WRITE8_MEMBER(slapfght_state::slapfight_68705_ddrB_w)
{
	m_ddrB = data;
}

READ8_MEMBER(slapfght_state::slapfight_68705_portC_r)
{
	m_portC_in = 0;

	if (m_main_sent)
		m_portC_in |= 0x01;
	if (!m_mcu_sent)
		m_portC_in |= 0x02;

	return (m_portC_out & m_ddrC) | (m_portC_in & ~m_ddrC);
}

WRITE8_MEMBER(slapfght_state::slapfight_68705_portC_w)
{
	m_portC_out = data;
}

WRITE8_MEMBER(slapfght_state::slapfight_68705_ddrC_w)
{
	m_ddrC = data;
}



/***************************************************************************

    Get Star MCU simulation :(

***************************************************************************/

READ8_MEMBER(slapfght_state::getstar_mcusim_status_r)
{
	static const int states[3]={ 0xc7, 0x55, 0x00 };

	m_getstar_status = states[m_getstar_status_state];

	m_getstar_status_state++;
	if (m_getstar_status_state > 2) m_getstar_status_state = 0;

	return m_getstar_status;
}

READ8_MEMBER(slapfght_state::getstar_mcusim_r)
{
	UINT16 tmp = 0;  /* needed for values computed on 16 bits */
	UINT8 getstar_val = 0;
	UINT8 phase_lookup_table[] = {0x00, 0x01, 0x03, 0xff, 0xff, 0x02, 0x05, 0xff, 0xff, 0x05}; /* table at 0x0e05 in 'getstarb1' */
	UINT8 lives_lookup_table[] = {0x03, 0x05, 0x01, 0x02};                                     /* table at 0x0e62 in 'getstarb1' */
	UINT8 lgsb2_lookup_table[] = {0x00, 0x03, 0x04, 0x05};                                     /* fake tanle for "test mode" in 'getstarb2' */

	switch (m_getstar_id)
	{
		case GETSTAR:
		case GETSTARJ:
			switch (m_getstar_cmd)
			{
				case 0x20:  /* continue play */
					getstar_val = ((m_gs_a & 0x30) == 0x30) ? 0x20 : 0x80;
					break;
				case 0x21:  /* lose life */
					getstar_val = (m_gs_a << 1) | (m_gs_a >> 7);
					break;
				case 0x22:  /* starting difficulty */
					getstar_val = ((m_gs_a & 0x0c) >> 2) + 1;
					break;
				case 0x23:  /* starting lives */
					getstar_val = lives_lookup_table[m_gs_a];
					break;
				case 0x24:  /* game phase */
					getstar_val = phase_lookup_table[((m_gs_a & 0x18) >> 1) | (m_gs_a & 0x03)];
					break;
				case 0x25:  /* players inputs */
					getstar_val = BITSWAP8(m_gs_a, 3, 2, 1, 0, 7, 5, 6, 4);
					break;
				case 0x26:  /* background (1st read) */
					tmp = 0x8800 + (0x001f * m_gs_a);
					getstar_val = (tmp & 0x00ff) >> 0;
					m_getstar_cmd |= 0x80;     /* to allow a second consecutive read */
					break;
				case 0xa6:  /* background (2nd read) */
					tmp = 0x8800 + (0x001f * m_gs_a);
					getstar_val = (tmp & 0xff00) >> 8;
					break;
				case 0x29:  /* unknown effect */
					getstar_val = 0x00;
					break;
				case 0x2a:  /* change player (if 2 players game) */
					getstar_val = (m_gs_a ^ 0x40);
					break;
				case 0x37:  /* foreground (1st read) */
					tmp = ((0xd0 + ((m_gs_e >> 2) & 0x0f)) << 8) | (0x40 * (m_gs_e & 03) + m_gs_d);
					getstar_val = (tmp & 0x00ff) >> 0;
					m_getstar_cmd |= 0x80;     /* to allow a second consecutive read */
					break;
				case 0xb7:  /* foreground (2nd read) */
					tmp = ((0xd0 + ((m_gs_e >> 2) & 0x0f)) << 8) | (0x40 * (m_gs_e & 03) + m_gs_d);
					getstar_val = (tmp & 0xff00) >> 8;
					break;
				case 0x38:  /* laser position (1st read) */
					tmp = 0xf740 - (((m_gs_e >> 4) << 8) | ((m_gs_e & 0x08) ? 0x80 : 0x00)) + (0x02 + (m_gs_d >> 2));
					getstar_val = (tmp & 0x00ff) >> 0;
					m_getstar_cmd |= 0x80;     /* to allow a second consecutive read */
					break;
				case 0xb8:  /* laser position (2nd read) */
					tmp = 0xf740 - (((m_gs_e >> 4) << 8) | ((m_gs_e & 0x08) ? 0x80 : 0x00)) + (0x02 + (m_gs_d >> 2));
					getstar_val = (tmp & 0xff00) >> 8;
					break;
				case 0x73:  /* avoid "BAD HW" message */
					getstar_val = 0x76;
					break;
				default:
					logerror("%04x: getstar_mcusim_r - cmd = %02x\n",space.device().safe_pc(),m_getstar_cmd);
					break;
			}
			break;
		case GETSTARB1:
			/* value isn't computed by the bootleg but we want to please the "test mode" */
			if (space.device().safe_pc() == 0x6b04) return (lives_lookup_table[m_gs_a]);
			break;
		case GETSTARB2:
			/*
			056B: 21 03 E8      ld   hl,$E803
			056E: 7E            ld   a,(hl)
			056F: BE            cp   (hl)
			0570: 28 FD         jr   z,$056F
			0572: C6 05         add  a,$05
			0574: EE 56         xor  $56
			0576: BE            cp   (hl)
			0577: C2 6E 05      jp   nz,$056E
			*/
			if (space.device().safe_pc() == 0x056e) return (getstar_val);
			if (space.device().safe_pc() == 0x0570) return (getstar_val+1);
			if (space.device().safe_pc() == 0x0577) return ((getstar_val+0x05) ^ 0x56);
			/* value isn't computed by the bootleg but we want to please the "test mode" */
			if (space.device().safe_pc() == 0x6b04) return (lgsb2_lookup_table[m_gs_a]);
			break;
		default:
			logerror("%04x: getstar_mcusim_r - cmd = %02x - unknown set !\n",space.device().safe_pc(),m_getstar_cmd);
			break;
	}
	return getstar_val;
}

WRITE8_MEMBER(slapfght_state::getstar_mcusim_w)
{
	/* due to code at 0x108d (GUARDIAN) or 0x1152 (GETSTARJ), register C is a unaltered copy of register A */
	#define GS_SAVE_REGS  m_gs_a = space.device().state().state_int(Z80_BC) >> 0; \
		m_gs_d = space.device().state().state_int(Z80_DE) >> 8; \
		m_gs_e = space.device().state().state_int(Z80_DE) >> 0;

	#define GS_RESET_REGS m_gs_a = 0; \
		m_gs_d = 0; \
		m_gs_e = 0;

	switch (m_getstar_id)
	{
		case GETSTAR:
			/* unknown effect - not read back */
			if (space.device().safe_pc() == 0x00bf)
			{
				m_getstar_cmd = 0x00;
				GS_RESET_REGS
			}
			/* players inputs */
			if (space.device().safe_pc() == 0x0560)
			{
				m_getstar_cmd = 0x25;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x056d)
			{
				m_getstar_cmd = 0x25;
				GS_SAVE_REGS
			}
			/* lose life */
			if (space.device().safe_pc() == 0x0a0a)
			{
				m_getstar_cmd = 0x21;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0a17)
			{
				m_getstar_cmd = 0x21;
				GS_SAVE_REGS
			}
			/* unknown effect */
			if (space.device().safe_pc() == 0x0a51)
			{
				m_getstar_cmd = 0x29;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0a6e)
			{
				m_getstar_cmd = 0x29;
				GS_SAVE_REGS
			}
			/* continue play */
			if (space.device().safe_pc() == 0x0ae3)
			{
				m_getstar_cmd = 0x20;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0af0)
			{
				m_getstar_cmd = 0x20;
				GS_SAVE_REGS
			}
			/* unknown effect - not read back */
			if (space.device().safe_pc() == 0x0b62)
			{
				m_getstar_cmd = 0x00;     /* 0x1f */
				GS_RESET_REGS
			}
			/* change player (if 2 players game) */
			if (space.device().safe_pc() == 0x0bab)
			{
				m_getstar_cmd = 0x2a;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0bb8)
			{
				m_getstar_cmd = 0x2a;
				GS_SAVE_REGS
			}
			/* game phase */
			if (space.device().safe_pc() == 0x0d37)
			{
				m_getstar_cmd = 0x24;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0d44)
			{
				m_getstar_cmd = 0x24;
				GS_SAVE_REGS
			}
			/* starting lives */
			if (space.device().safe_pc() == 0x0d79)
			{
				m_getstar_cmd = 0x23;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0d8a)
			{
				m_getstar_cmd = 0x23;
				GS_SAVE_REGS
			}
			/* starting difficulty */
			if (space.device().safe_pc() == 0x0dc1)
			{
				m_getstar_cmd = 0x22;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0dd0)
			{
				m_getstar_cmd = 0x22;
				GS_SAVE_REGS
			}
			/* starting lives (again) */
			if (space.device().safe_pc() == 0x1011)
			{
				m_getstar_cmd = 0x23;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x101e)
			{
				m_getstar_cmd = 0x23;
				GS_SAVE_REGS
			}
			/* hardware test */
			if (space.device().safe_pc() == 0x107a)
			{
				m_getstar_cmd = 0x73;
				GS_RESET_REGS
			}
			/* game phase (again) */
			if (space.device().safe_pc() == 0x10c6)
			{
				m_getstar_cmd = 0x24;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x10d3)
			{
				m_getstar_cmd = 0x24;
				GS_SAVE_REGS
			}
			/* background */
			if (space.device().safe_pc() == 0x1910)
			{
				m_getstar_cmd = 0x26;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x191d)
			{
				m_getstar_cmd = 0x26;
				GS_SAVE_REGS
			}
			/* foreground */
			if (space.device().safe_pc() == 0x19d5)
			{
				m_getstar_cmd = 0x37;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x19e4)
			{
				m_getstar_cmd = 0x37;
				GS_SAVE_REGS
			}
			if (space.device().safe_pc() == 0x19f1)
			{
				m_getstar_cmd = 0x37;
				/* do NOT update the registers because there are 2 writes before 2 reads ! */
			}
			/* laser position */
			if (space.device().safe_pc() == 0x26af)
			{
				m_getstar_cmd = 0x38;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x26be)
			{
				m_getstar_cmd = 0x38;
				GS_SAVE_REGS
			}
			if (space.device().safe_pc() == 0x26cb)
			{
				m_getstar_cmd = 0x38;
				/* do NOT update the registers because there are 2 writes before 2 reads ! */
			}
			/* starting lives (for "test mode") */
			if (space.device().safe_pc() == 0x6a27)
			{
				m_getstar_cmd = 0x23;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x6a38)
			{
				m_getstar_cmd = 0x23;
				GS_SAVE_REGS
			}
			break;
		case GETSTARJ:
			/* unknown effect - not read back */
			if (space.device().safe_pc() == 0x00bf)
			{
				m_getstar_cmd = 0x00;
				GS_RESET_REGS
			}
			/* players inputs */
			if (space.device().safe_pc() == 0x0560)
			{
				m_getstar_cmd = 0x25;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x056d)
			{
				m_getstar_cmd = 0x25;
				GS_SAVE_REGS
			}
			/* lose life */
			if (space.device().safe_pc() == 0x0ad5)
			{
				m_getstar_cmd = 0x21;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0ae2)
			{
				m_getstar_cmd = 0x21;
				GS_SAVE_REGS
			}
			/* unknown effect */
			if (space.device().safe_pc() == 0x0b1c)
			{
				m_getstar_cmd = 0x29;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0b29)
			{
				m_getstar_cmd = 0x29;
				GS_SAVE_REGS
			}
			/* continue play */
			if (space.device().safe_pc() == 0x0bae)
			{
				m_getstar_cmd = 0x20;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0bbb)
			{
				m_getstar_cmd = 0x20;
				GS_SAVE_REGS
			}
			/* unknown effect - not read back */
			if (space.device().safe_pc() == 0x0c2d)
			{
				m_getstar_cmd = 0x00;     /* 0x1f */
				GS_RESET_REGS
			}
			/* change player (if 2 players game) */
			if (space.device().safe_pc() == 0x0c76)
			{
				m_getstar_cmd = 0x2a;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0c83)
			{
				m_getstar_cmd = 0x2a;
				GS_SAVE_REGS
			}
			/* game phase */
			if (space.device().safe_pc() == 0x0e02)
			{
				m_getstar_cmd = 0x24;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0e0f)
			{
				m_getstar_cmd = 0x24;
				GS_SAVE_REGS
			}
			/* starting lives */
			if (space.device().safe_pc() == 0x0e44)
			{
				m_getstar_cmd = 0x23;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0e55)
			{
				m_getstar_cmd = 0x23;
				GS_SAVE_REGS
			}
			/* starting difficulty */
			if (space.device().safe_pc() == 0x0e8c)
			{
				m_getstar_cmd = 0x22;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x0e9b)
			{
				m_getstar_cmd = 0x22;
				GS_SAVE_REGS
			}
			/* starting lives (again) */
			if (space.device().safe_pc() == 0x10d6)
			{
				m_getstar_cmd = 0x23;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x10e3)
			{
				m_getstar_cmd = 0x23;
				GS_SAVE_REGS
			}
			/* hardware test */
			if (space.device().safe_pc() == 0x113f)
			{
				m_getstar_cmd = 0x73;
				GS_RESET_REGS
			}
			/* game phase (again) */
			if (space.device().safe_pc() == 0x118b)
			{
				m_getstar_cmd = 0x24;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x1198)
			{
				m_getstar_cmd = 0x24;
				GS_SAVE_REGS
			}
			/* background */
			if (space.device().safe_pc() == 0x19f8)
			{
				m_getstar_cmd = 0x26;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x1a05)
			{
				m_getstar_cmd = 0x26;
				GS_SAVE_REGS
			}
			/* foreground */
			if (space.device().safe_pc() == 0x1abd)
			{
				m_getstar_cmd = 0x37;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x1acc)
			{
				m_getstar_cmd = 0x37;
				GS_SAVE_REGS
			}
			if (space.device().safe_pc() == 0x1ad9)
			{
				m_getstar_cmd = 0x37;
				/* do NOT update the registers because there are 2 writes before 2 reads ! */
			}
			/* laser position */
			if (space.device().safe_pc() == 0x2792)
			{
				m_getstar_cmd = 0x38;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x27a1)
			{
				m_getstar_cmd = 0x38;
				GS_SAVE_REGS
			}
			if (space.device().safe_pc() == 0x27ae)
			{
				m_getstar_cmd = 0x38;
				/* do NOT update the registers because there are 2 writes before 2 reads ! */
			}
			/* starting lives (for "test mode") */
			if (space.device().safe_pc() == 0x6ae2)
			{
				m_getstar_cmd = 0x23;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x6af3)
			{
				m_getstar_cmd = 0x23;
				GS_SAVE_REGS
			}
			break;
		case GETSTARB1:
			/* "Test mode" doesn't compute the lives value :
			    6ADA: 3E 23         ld   a,$23
			    6ADC: CD 52 11      call $1152
			    6ADF: 32 03 E8      ld   ($E803),a
			    6AE2: DB 00         in   a,($00)
			    6AE4: CB 4F         bit  1,a
			    6AE6: 28 FA         jr   z,$6AE2
			    6AE8: 3A 0A C8      ld   a,($C80A)
			    6AEB: E6 03         and  $03
			    6AED: CD 52 11      call $1152
			    6AF0: 32 03 E8      ld   ($E803),a
			    6AF3: DB 00         in   a,($00)
			    6AF5: CB 57         bit  2,a
			    6AF7: 20 FA         jr   nz,$6AF3
			    6AF9: 00            nop
			    6AFA: 00            nop
			    6AFB: 00            nop
			    6AFC: 00            nop
			    6AFD: 00            nop
			    6AFE: 00            nop
			    6AFF: 00            nop
			    6B00: 00            nop
			    6B01: 3A 03 E8      ld   a,($E803)
			   We save the regs though to hack it in 'getstar_mcusim_r' read handler.
			*/
			if (space.device().safe_pc() == 0x6ae2)
			{
				m_getstar_cmd = 0x00;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x6af3)
			{
				m_getstar_cmd = 0x00;
				GS_SAVE_REGS
			}
			break;
		case GETSTARB2:
			/* "Test mode" doesn't compute the lives value :
			    6ADA: 3E 23         ld   a,$23
			    6ADC: CD 52 11      call $1152
			    6ADF: 32 03 E8      ld   ($E803),a
			    6AE2: DB 00         in   a,($00)
			    6AE4: CB 4F         bit  1,a
			    6AE6: 00            nop
			    6AE7: 00            nop
			    6AE8: 3A 0A C8      ld   a,($C80A)
			    6AEB: E6 03         and  $03
			    6AED: CD 52 11      call $1152
			    6AF0: 32 03 E8      ld   ($E803),a
			    6AF3: DB 00         in   a,($00)
			    6AF5: CB 57         bit  2,a
			    6AF7: 00            nop
			    6AF8: 00            nop
			    6AF9: 00            nop
			    6AFA: 00            nop
			    6AFB: 00            nop
			    6AFC: 00            nop
			    6AFD: 00            nop
			    6AFE: 00            nop
			    6AFF: 00            nop
			    6B00: 00            nop
			    6B01: 3A 03 E8      ld   a,($E803)
			   We save the regs though to hack it in 'getstar_mcusim_r' read handler.
			*/
			if (space.device().safe_pc() == 0x6ae2)
			{
				m_getstar_cmd = 0x00;
				GS_RESET_REGS
			}
			if (space.device().safe_pc() == 0x6af3)
			{
				m_getstar_cmd = 0x00;
				GS_SAVE_REGS
			}
			break;
		default:
			logerror("%04x: getstar_mcusim_w - data = %02x - unknown set !\n",space.device().safe_pc(),data);
			break;
	}
}



/***************************************************************************

    Protection kludges for some bootleg sets

***************************************************************************/

READ8_MEMBER(slapfght_state::tigerhb1_prot_r)
{
	UINT8 tigerhb_val = 0;
	switch (m_tigerhb_cmd)
	{
		case 0x73:  /* avoid "BAD HW" message */
			tigerhb_val = 0x83;
			break;
		default:
			logerror("%04x: tigerhb1_prot_r - cmd = %02x\n", space.device().safe_pc(), m_getstar_cmd);
			break;
	}
	return tigerhb_val;
}

WRITE8_MEMBER(slapfght_state::tigerhb1_prot_w)
{
	switch (data)
	{
		/* hardware test */
		case 0x73:
			m_tigerhb_cmd = 0x73;
			break;
		default:
			logerror("%04x: tigerhb1_prot_w - data = %02x\n",space.device().safe_pc(),data);
			m_tigerhb_cmd = 0x00;
			break;
	}
}


/**************************************************************************/

READ8_MEMBER(slapfght_state::getstarb1_prot_r)
{
	/* The bootleg has it's own 'protection' on startup ?
	    6D1A: 06 04         ld   b,$04
	    6D1C: DB 00         in   a,($00)
	    6D1E: E6 06         and  $06
	    6D20: 20 FA         jr   nz,$6D1C
	    6D22: DB 00         in   a,($00)
	    6D24: E6 06         and  $06
	    6D26: FE 06         cp   $06
	    6D28: 20 F8         jr   nz,$6D22
	    6D2A: DB 00         in   a,($00)
	    6D2C: E6 06         and  $06
	    6D2E: FE 02         cp   $02
	    6D30: 20 F8         jr   nz,$6D2A
	    6D32: DB 00         in   a,($00)
	    6D34: E6 06         and  $06
	    6D36: FE 04         cp   $04
	    6D38: 20 F8         jr   nz,$6D32
	    6D3A: 10 E0         djnz $6D1C
	*/
	if (space.device().safe_pc() == 0x6d1e) return 0;
	if (space.device().safe_pc() == 0x6d24) return 6;
	if (space.device().safe_pc() == 0x6d2c) return 2;
	if (space.device().safe_pc() == 0x6d34) return 4;

	/* The bootleg hangs in the "test mode" before diplaying (wrong) lives settings :
	    6AD4: DB 00         in   a,($00)
	    6AD6: CB 4F         bit  1,a
	    6AD8: 28 FA         jr   z,$6AD4
	    6ADA: 3E 23         ld   a,$23
	    6ADC: CD 52 11      call $1152
	    6ADF: 32 03 E8      ld   ($E803),a
	    6AE2: DB 00         in   a,($00)
	    6AE4: CB 4F         bit  1,a
	    6AE6: 28 FA         jr   z,$6AE2
	    6AE8: 3A 0A C8      ld   a,($C80A)
	    6AEB: E6 03         and  $03
	    6AED: CD 52 11      call $1152
	    6AF0: 32 03 E8      ld   ($E803),a
	    6AF3: DB 00         in   a,($00)
	    6AF5: CB 57         bit  2,a
	    6AF7: 20 FA         jr   nz,$6AF3
	   This seems to be what used to be the MCU status.
	*/
	if (space.device().safe_pc() == 0x6ad6) return 2; /* bit 1 must be ON */
	if (space.device().safe_pc() == 0x6ae4) return 2; /* bit 1 must be ON */
	if (space.device().safe_pc() == 0x6af5) return 0; /* bit 2 must be OFF */

	logerror("Port Read PC=%04x\n",space.device().safe_pc());

	return 0;
}
