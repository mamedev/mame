// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Stephane Humbert
/***************************************************************************

  Toaplan Slap Fight hardware

  Functions to emulate (and simulate) the MCU

***************************************************************************/

#include "emu.h"
#include "slapfght.h"


/**************************************************************************/

uint8_t slapfght_state::tigerh_mcu_status_r()
{
	return
			(m_screen->vblank() ? 0x01 : 0x00) |
			((m_bmcu && (CLEAR_LINE == m_bmcu->host_semaphore_r())) ? 0x02 : 0x00) |
			((m_bmcu && (CLEAR_LINE == m_bmcu->mcu_semaphore_r())) ? 0x04 : 0x00);
}

void slapfght_state::scroll_from_mcu_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x01: m_scrollx_lo = data; break;  // PB3
	case 0x02: m_scrollx_hi = data; break;  // PB4
	}
}

/***************************************************************************

    Get Star MCU simulation :(

***************************************************************************/

uint8_t slapfght_state::getstar_mcusim_status_r()
{
	static const int states[3]={ 0xc7, 0x55, 0x00 };

	m_getstar_status = states[m_getstar_status_state];

	m_getstar_status_state++;
	if (m_getstar_status_state > 2) m_getstar_status_state = 0;

	return m_getstar_status;
}

uint8_t slapfght_state::getstar_mcusim_r()
{
	uint8_t getstar_val = 0;
	uint8_t lives_lookup_table[] = {0x03, 0x05, 0x01, 0x02};                                     /* table at 0x0e62 in 'getstarb1' */
	uint8_t lgsb2_lookup_table[] = {0x00, 0x03, 0x04, 0x05};                                     /* fake tanle for "test mode" in 'getstarb2' */

	switch (m_getstar_id)
	{
		case GETSTARB1:
			/* value isn't computed by the bootleg but we want to please the "test mode" */
			if (m_maincpu->pc() == 0x6b04) return (lives_lookup_table[m_gs_a]);
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
			if (m_maincpu->pc() == 0x056e) return (getstar_val);
			if (m_maincpu->pc() == 0x0570) return (getstar_val+1);
			if (m_maincpu->pc() == 0x0577) return ((getstar_val+0x05) ^ 0x56);
			/* value isn't computed by the bootleg but we want to please the "test mode" */
			if (m_maincpu->pc() == 0x6b04) return (lgsb2_lookup_table[m_gs_a]);
			break;
		default:
			logerror("%04x: getstar_mcusim_r - cmd = %02x - unknown set !\n",m_maincpu->pc(),m_getstar_cmd);
			break;
	}
	return getstar_val;
}

void slapfght_state::getstar_mcusim_w(uint8_t data)
{
	/* due to code at 0x108d (GUARDIAN) or 0x1152 (GETSTARJ), register C is a unaltered copy of register A */
	#define GS_SAVE_REGS  m_gs_a = m_maincpu->state_int(Z80_BC) >> 0; \
		m_gs_d = m_maincpu->state_int(Z80_DE) >> 8; \
		m_gs_e = m_maincpu->state_int(Z80_DE) >> 0;

	#define GS_RESET_REGS m_gs_a = 0; \
		m_gs_d = 0; \
		m_gs_e = 0;

	switch (m_getstar_id)
	{
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
			if (m_maincpu->pc() == 0x6ae2)
			{
				m_getstar_cmd = 0x00;
				GS_RESET_REGS
			}
			if (m_maincpu->pc() == 0x6af3)
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
			if (m_maincpu->pc() == 0x6ae2)
			{
				m_getstar_cmd = 0x00;
				GS_RESET_REGS
			}
			if (m_maincpu->pc() == 0x6af3)
			{
				m_getstar_cmd = 0x00;
				GS_SAVE_REGS
			}
			break;
		default:
			logerror("%04x: getstar_mcusim_w - data = %02x - unknown set !\n",m_maincpu->pc(),data);
			break;
	}
}



/***************************************************************************

    Protection kludges for some bootleg sets

***************************************************************************/

uint8_t slapfght_state::tigerhb1_prot_r()
{
	uint8_t tigerhb_val = 0;
	switch (m_tigerhb_cmd)
	{
		case 0x73:  /* avoid "BAD HW" message */
			tigerhb_val = 0x83;
			break;
		default:
			logerror("%04x: tigerhb1_prot_r - cmd = %02x\n", m_maincpu->pc(), m_getstar_cmd);
			break;
	}
	return tigerhb_val;
}

void slapfght_state::tigerhb1_prot_w(uint8_t data)
{
	switch (data)
	{
		/* hardware test */
		case 0x73:
			m_tigerhb_cmd = 0x73;
			break;
		default:
			logerror("%04x: tigerhb1_prot_w - data = %02x\n",m_maincpu->pc(),data);
			m_tigerhb_cmd = 0x00;
			break;
	}
}


/**************************************************************************/

uint8_t slapfght_state::getstarb1_prot_r()
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
	if (m_maincpu->pc() == 0x6d1e) return 0;
	if (m_maincpu->pc() == 0x6d24) return 6;
	if (m_maincpu->pc() == 0x6d2c) return 2;
	if (m_maincpu->pc() == 0x6d34) return 4;

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
	if (m_maincpu->pc() == 0x6ad6) return 2; /* bit 1 must be ON */
	if (m_maincpu->pc() == 0x6ae4) return 2; /* bit 1 must be ON */
	if (m_maincpu->pc() == 0x6af5) return 0; /* bit 2 must be OFF */

	logerror("Port Read PC=%04x\n",m_maincpu->pc());

	return 0;
}
