/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/slapfght.h"


/* Perform basic machine initialisation */
MACHINE_RESET( slapfight )
{
	slapfght_state *state = machine.driver_data<slapfght_state>();
	/* MAIN CPU */

	state->m_slapfight_status_state=0;
	state->m_slapfight_status = 0xc7;

	state->m_getstar_sequence_index = 0;
	state->m_getstar_sh_intenabled = 0;	/* disable sound cpu interrupts */

	/* SOUND CPU */
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);

	/* MCU */
	state->m_mcu_val = 0;
}

/* Slapfight CPU input/output ports

  These ports seem to control memory access

*/

/* Reset and hold sound CPU */
WRITE8_MEMBER(slapfght_state::slapfight_port_00_w)
{
	cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);
	m_getstar_sh_intenabled = 0;
}

/* Release reset on sound CPU */
WRITE8_MEMBER(slapfght_state::slapfight_port_01_w)
{
	cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_RESET, CLEAR_LINE);
}

/* Disable and clear hardware interrupt */
WRITE8_MEMBER(slapfght_state::slapfight_port_06_w)
{

	m_irq_mask = 0;
}

/* Enable hardware interrupt */
WRITE8_MEMBER(slapfght_state::slapfight_port_07_w)
{

	m_irq_mask = 1;
}

WRITE8_MEMBER(slapfght_state::slapfight_port_08_w)
{
	UINT8 *RAM = machine().region("maincpu")->base();

	memory_set_bankptr(machine(), "bank1",&RAM[0x10000]);
}

WRITE8_MEMBER(slapfght_state::slapfight_port_09_w)
{
	UINT8 *RAM = machine().region("maincpu")->base();

	memory_set_bankptr(machine(), "bank1",&RAM[0x14000]);
}


/* Status register */
READ8_MEMBER(slapfght_state::slapfight_port_00_r)
{
	static const int states[3]={ 0xc7, 0x55, 0x00 };

	m_slapfight_status = states[m_slapfight_status_state];

	m_slapfight_status_state++;
	if (m_slapfight_status_state > 2) m_slapfight_status_state = 0;

	return m_slapfight_status;
}

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
		m_portA_in = m_from_main;

		if (m_main_sent)
			cputag_set_input_line(machine(), "mcu", 0, CLEAR_LINE);

		m_main_sent = 0;
	}
	if ((m_ddrB & 0x04) && (data & 0x04) && (~m_portB_out & 0x04))
	{
		m_from_mcu = m_portA_out;
		m_mcu_sent = 1;
	}
	if ((m_ddrB & 0x08) && (~data & 0x08) && (m_portB_out & 0x08))
	{
		*m_slapfight_scrollx_lo = m_portA_out;
	}
	if ((m_ddrB & 0x10) && (~data & 0x10) && (m_portB_out & 0x10))
	{
		*m_slapfight_scrollx_hi = m_portA_out;
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

WRITE8_MEMBER(slapfght_state::slapfight_mcu_w)
{
	m_from_main = data;
	m_main_sent = 1;
	cputag_set_input_line(machine(), "mcu", 0, ASSERT_LINE);
}

READ8_MEMBER(slapfght_state::slapfight_mcu_r)
{
	m_mcu_sent = 0;
	return m_from_mcu;
}

READ8_MEMBER(slapfght_state::slapfight_mcu_status_r)
{
	int res = 0;

	if (!m_main_sent)
		res |= 0x02;
	if (!m_mcu_sent)
		res |= 0x04;

	return res;
}

/***************************************************************************

    Get Star MCU simulation

***************************************************************************/

READ8_MEMBER(slapfght_state::getstar_e803_r)
{
	UINT16 tmp = 0;  /* needed for values computed on 16 bits */
	UINT8 getstar_val = 0;
	UINT8 phase_lookup_table[] = {0x00, 0x01, 0x03, 0xff, 0xff, 0x02, 0x05, 0xff, 0xff, 0x05}; /* table at 0x0e05 in 'gtstarb1' */
	UINT8 lives_lookup_table[] = {0x03, 0x05, 0x01, 0x02};                                     /* table at 0x0e62 in 'gtstarb1' */
	UINT8 lgsb2_lookup_table[] = {0x00, 0x03, 0x04, 0x05};                                     /* fake tanle for "test mode" in 'gtstarb2' */

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
					logerror("%04x: getstar_e803_r - cmd = %02x\n",cpu_get_pc(&space.device()),m_getstar_cmd);
					break;
			}
			break;
		case GTSTARB1:
			/* value isn't computed by the bootleg but we want to please the "test mode" */
			if (cpu_get_pc(&space.device()) == 0x6b04) return (lives_lookup_table[m_gs_a]);
			break;
		case GTSTARB2:
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
			if (cpu_get_pc(&space.device()) == 0x056e) return (getstar_val);
			if (cpu_get_pc(&space.device()) == 0x0570) return (getstar_val+1);
			if (cpu_get_pc(&space.device()) == 0x0577) return ((getstar_val+0x05) ^ 0x56);
			/* value isn't computed by the bootleg but we want to please the "test mode" */
			if (cpu_get_pc(&space.device()) == 0x6b04) return (lgsb2_lookup_table[m_gs_a]);
			break;
		default:
			logerror("%04x: getstar_e803_r - cmd = %02x - unknown set !\n",cpu_get_pc(&space.device()),m_getstar_cmd);
			break;
	}
	return getstar_val;
}

WRITE8_MEMBER(slapfght_state::getstar_e803_w)
{
	switch (m_getstar_id)
	{
		case GETSTAR:
			/* unknown effect - not read back */
			if (cpu_get_pc(&space.device()) == 0x00bf)
			{
				m_getstar_cmd = 0x00;
				GS_RESET_REGS
			}
			/* players inputs */
			if (cpu_get_pc(&space.device()) == 0x0560)
			{
				m_getstar_cmd = 0x25;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x056d)
			{
				m_getstar_cmd = 0x25;
				GS_SAVE_REGS
			}
			/* lose life */
			if (cpu_get_pc(&space.device()) == 0x0a0a)
			{
				m_getstar_cmd = 0x21;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0a17)
			{
				m_getstar_cmd = 0x21;
				GS_SAVE_REGS
			}
			/* unknown effect */
			if (cpu_get_pc(&space.device()) == 0x0a51)
			{
				m_getstar_cmd = 0x29;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0a6e)
			{
				m_getstar_cmd = 0x29;
				GS_SAVE_REGS
			}
			/* continue play */
			if (cpu_get_pc(&space.device()) == 0x0ae3)
			{
				m_getstar_cmd = 0x20;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0af0)
			{
				m_getstar_cmd = 0x20;
				GS_SAVE_REGS
			}
			/* unknown effect - not read back */
			if (cpu_get_pc(&space.device()) == 0x0b62)
			{
				m_getstar_cmd = 0x00;     /* 0x1f */
				GS_RESET_REGS
			}
			/* change player (if 2 players game) */
			if (cpu_get_pc(&space.device()) == 0x0bab)
			{
				m_getstar_cmd = 0x2a;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0bb8)
			{
				m_getstar_cmd = 0x2a;
				GS_SAVE_REGS
			}
			/* game phase */
			if (cpu_get_pc(&space.device()) == 0x0d37)
			{
				m_getstar_cmd = 0x24;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0d44)
			{
				m_getstar_cmd = 0x24;
				GS_SAVE_REGS
			}
			/* starting lives */
			if (cpu_get_pc(&space.device()) == 0x0d79)
			{
				m_getstar_cmd = 0x23;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0d8a)
			{
				m_getstar_cmd = 0x23;
				GS_SAVE_REGS
			}
			/* starting difficulty */
			if (cpu_get_pc(&space.device()) == 0x0dc1)
			{
				m_getstar_cmd = 0x22;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0dd0)
			{
				m_getstar_cmd = 0x22;
				GS_SAVE_REGS
			}
			/* starting lives (again) */
			if (cpu_get_pc(&space.device()) == 0x1011)
			{
				m_getstar_cmd = 0x23;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x101e)
			{
				m_getstar_cmd = 0x23;
				GS_SAVE_REGS
			}
			/* hardware test */
			if (cpu_get_pc(&space.device()) == 0x107a)
			{
				m_getstar_cmd = 0x73;
				GS_RESET_REGS
			}
			/* game phase (again) */
			if (cpu_get_pc(&space.device()) == 0x10c6)
			{
				m_getstar_cmd = 0x24;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x10d3)
			{
				m_getstar_cmd = 0x24;
				GS_SAVE_REGS
			}
			/* background */
			if (cpu_get_pc(&space.device()) == 0x1910)
			{
				m_getstar_cmd = 0x26;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x191d)
			{
				m_getstar_cmd = 0x26;
				GS_SAVE_REGS
			}
			/* foreground */
			if (cpu_get_pc(&space.device()) == 0x19d5)
			{
				m_getstar_cmd = 0x37;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x19e4)
			{
				m_getstar_cmd = 0x37;
				GS_SAVE_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x19f1)
			{
				m_getstar_cmd = 0x37;
				/* do NOT update the registers because there are 2 writes before 2 reads ! */
			}
			/* laser position */
			if (cpu_get_pc(&space.device()) == 0x26af)
			{
				m_getstar_cmd = 0x38;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x26be)
			{
				m_getstar_cmd = 0x38;
				GS_SAVE_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x26cb)
			{
				m_getstar_cmd = 0x38;
				/* do NOT update the registers because there are 2 writes before 2 reads ! */
			}
			/* starting lives (for "test mode") */
			if (cpu_get_pc(&space.device()) == 0x6a27)
			{
				m_getstar_cmd = 0x23;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x6a38)
			{
				m_getstar_cmd = 0x23;
				GS_SAVE_REGS
			}
			break;
		case GETSTARJ:
			/* unknown effect - not read back */
			if (cpu_get_pc(&space.device()) == 0x00bf)
			{
				m_getstar_cmd = 0x00;
				GS_RESET_REGS
			}
			/* players inputs */
			if (cpu_get_pc(&space.device()) == 0x0560)
			{
				m_getstar_cmd = 0x25;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x056d)
			{
				m_getstar_cmd = 0x25;
				GS_SAVE_REGS
			}
			/* lose life */
			if (cpu_get_pc(&space.device()) == 0x0ad5)
			{
				m_getstar_cmd = 0x21;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0ae2)
			{
				m_getstar_cmd = 0x21;
				GS_SAVE_REGS
			}
			/* unknown effect */
			if (cpu_get_pc(&space.device()) == 0x0b1c)
			{
				m_getstar_cmd = 0x29;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0b29)
			{
				m_getstar_cmd = 0x29;
				GS_SAVE_REGS
			}
			/* continue play */
			if (cpu_get_pc(&space.device()) == 0x0bae)
			{
				m_getstar_cmd = 0x20;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0bbb)
			{
				m_getstar_cmd = 0x20;
				GS_SAVE_REGS
			}
			/* unknown effect - not read back */
			if (cpu_get_pc(&space.device()) == 0x0c2d)
			{
				m_getstar_cmd = 0x00;     /* 0x1f */
				GS_RESET_REGS
			}
			/* change player (if 2 players game) */
			if (cpu_get_pc(&space.device()) == 0x0c76)
			{
				m_getstar_cmd = 0x2a;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0c83)
			{
				m_getstar_cmd = 0x2a;
				GS_SAVE_REGS
			}
			/* game phase */
			if (cpu_get_pc(&space.device()) == 0x0e02)
			{
				m_getstar_cmd = 0x24;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0e0f)
			{
				m_getstar_cmd = 0x24;
				GS_SAVE_REGS
			}
			/* starting lives */
			if (cpu_get_pc(&space.device()) == 0x0e44)
			{
				m_getstar_cmd = 0x23;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0e55)
			{
				m_getstar_cmd = 0x23;
				GS_SAVE_REGS
			}
			/* starting difficulty */
			if (cpu_get_pc(&space.device()) == 0x0e8c)
			{
				m_getstar_cmd = 0x22;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x0e9b)
			{
				m_getstar_cmd = 0x22;
				GS_SAVE_REGS
			}
			/* starting lives (again) */
			if (cpu_get_pc(&space.device()) == 0x10d6)
			{
				m_getstar_cmd = 0x23;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x10e3)
			{
				m_getstar_cmd = 0x23;
				GS_SAVE_REGS
			}
			/* hardware test */
			if (cpu_get_pc(&space.device()) == 0x113f)
			{
				m_getstar_cmd = 0x73;
				GS_RESET_REGS
			}
			/* game phase (again) */
			if (cpu_get_pc(&space.device()) == 0x118b)
			{
				m_getstar_cmd = 0x24;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x1198)
			{
				m_getstar_cmd = 0x24;
				GS_SAVE_REGS
			}
			/* background */
			if (cpu_get_pc(&space.device()) == 0x19f8)
			{
				m_getstar_cmd = 0x26;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x1a05)
			{
				m_getstar_cmd = 0x26;
				GS_SAVE_REGS
			}
			/* foreground */
			if (cpu_get_pc(&space.device()) == 0x1abd)
			{
				m_getstar_cmd = 0x37;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x1acc)
			{
				m_getstar_cmd = 0x37;
				GS_SAVE_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x1ad9)
			{
				m_getstar_cmd = 0x37;
				/* do NOT update the registers because there are 2 writes before 2 reads ! */
			}
			/* laser position */
			if (cpu_get_pc(&space.device()) == 0x2792)
			{
				m_getstar_cmd = 0x38;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x27a1)
			{
				m_getstar_cmd = 0x38;
				GS_SAVE_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x27ae)
			{
				m_getstar_cmd = 0x38;
				/* do NOT update the registers because there are 2 writes before 2 reads ! */
			}
			/* starting lives (for "test mode") */
			if (cpu_get_pc(&space.device()) == 0x6ae2)
			{
				m_getstar_cmd = 0x23;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x6af3)
			{
				m_getstar_cmd = 0x23;
				GS_SAVE_REGS
			}
			break;
		case GTSTARB1:
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
               We save the regs though to hack it in 'getstar_e803_r' read handler.
            */
			if (cpu_get_pc(&space.device()) == 0x6ae2)
			{
				m_getstar_cmd = 0x00;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x6af3)
			{
				m_getstar_cmd = 0x00;
				GS_SAVE_REGS
			}
			break;
		case GTSTARB2:
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
               We save the regs though to hack it in 'getstar_e803_r' read handler.
            */
			if (cpu_get_pc(&space.device()) == 0x6ae2)
			{
				m_getstar_cmd = 0x00;
				GS_RESET_REGS
			}
			if (cpu_get_pc(&space.device()) == 0x6af3)
			{
				m_getstar_cmd = 0x00;
				GS_SAVE_REGS
			}
			break;
		default:
			logerror("%04x: getstar_e803_w - data = %02x - unknown set !\n",cpu_get_pc(&space.device()),data);
			break;
	}
}

/* Enable hardware interrupt of sound cpu */
WRITE8_MEMBER(slapfght_state::getstar_sh_intenable_w)
{
	m_getstar_sh_intenabled = 1;
	logerror("cpu #1 PC=%d: %d written to a0e0\n",cpu_get_pc(&space.device()),data);
}



/* Generate interrups only if they have been enabled */
INTERRUPT_GEN( getstar_interrupt )
{
	slapfght_state *state = device->machine().driver_data<slapfght_state>();
	if (state->m_getstar_sh_intenabled)
		device_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

#ifdef UNUSED_FUNCTION
WRITE8_MEMBER(slapfght_state::getstar_port_04_w)
{
//  cpu_halt(0,0);
}
#endif


/***************************************************************************

    Tiger Heli MCU

***************************************************************************/

READ8_MEMBER(slapfght_state::tigerh_68705_portA_r)
{
	return (m_portA_out & m_ddrA) | (m_portA_in & ~m_ddrA);
}

WRITE8_MEMBER(slapfght_state::tigerh_68705_portA_w)
{
	m_portA_out = data;//?
	m_from_mcu = m_portA_out;
	m_mcu_sent = 1;
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
		m_portA_in = m_from_main;
		if (m_main_sent) cputag_set_input_line(machine(), "mcu", 0, CLEAR_LINE);
		m_main_sent = 0;
	}
	if ((m_ddrB & 0x04) && (data & 0x04) && (~m_portB_out & 0x04))
	{
		m_from_mcu = m_portA_out;
		m_mcu_sent = 1;
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
	if (!m_main_sent) m_portC_in |= 0x01;
	if (m_mcu_sent) m_portC_in |= 0x02;
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

WRITE8_MEMBER(slapfght_state::tigerh_mcu_w)
{
	m_from_main = data;
	m_main_sent = 1;
	m_mcu_sent = 0;
	cputag_set_input_line(machine(), "mcu", 0, ASSERT_LINE);
}

READ8_MEMBER(slapfght_state::tigerh_mcu_r)
{
	m_mcu_sent = 0;
	return m_from_mcu;
}

READ8_MEMBER(slapfght_state::tigerh_mcu_status_r)
{
	int res = 0;
	if (!m_main_sent) res |= 0x02;
	if (!m_mcu_sent) res |= 0x04;
	return res;
}


READ8_MEMBER(slapfght_state::tigerhb_e803_r)
{
	UINT8 tigerhb_val = 0;
	switch (m_tigerhb_cmd)
	{
		case 0x73:  /* avoid "BAD HW" message */
			tigerhb_val = 0x83;
			break;
		default:
			logerror("%04x: tigerhb_e803_r - cmd = %02x\n",cpu_get_pc(&space.device()),m_getstar_cmd);
			break;
	}
	return tigerhb_val;
}

WRITE8_MEMBER(slapfght_state::tigerhb_e803_w)
{
	switch (data)
	{
		/* hardware test */
		case 0x73:
			m_tigerhb_cmd = 0x73;
			break;
		default:
			logerror("%04x: tigerhb_e803_w - data = %02x\n",cpu_get_pc(&space.device()),data);
			m_tigerhb_cmd = 0x00;
			break;
	}
}


/***************************************************************************

    Performan

***************************************************************************/

READ8_MEMBER(slapfght_state::perfrman_port_00_r)
{
	/* TODO */
	return machine().rand() & 1;
}
