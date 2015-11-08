// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*
   Texas Instruments TMS320C51 DSP Emulator

   Written by Ville Linde
*/

#include "emu.h"
#include "debugger.h"
#include "tms32051.h"

enum
{
	TMS32051_PC = 1,
	TMS32051_ACC,
	TMS32051_ACCB,
	TMS32051_PREG,
	TMS32051_TREG0,
	TMS32051_TREG1,
	TMS32051_TREG2,
	TMS32051_BMAR,
	TMS32051_RPTC,
	TMS32051_BRCR,
	TMS32051_INDX,
	TMS32051_DBMR,
	TMS32051_ARCR,
	TMS32051_DP,
	TMS32051_ARP,
	TMS32051_ARB,
	TMS32051_AR0,
	TMS32051_AR1,
	TMS32051_AR2,
	TMS32051_AR3,
	TMS32051_AR4,
	TMS32051_AR5,
	TMS32051_AR6,
	TMS32051_AR7
};


const device_type TMS32051 = &device_creator<tms32051_device>;


/**************************************************************************
 * Internal memory map
 **************************************************************************/

static ADDRESS_MAP_START( internal_pgm, AS_PROGRAM, 16, tms32051_device )
//  AM_RANGE(0x0000, 0x1fff) AM_ROM                         // ROM          TODO: is off-chip if MP/_MC = 0
	AM_RANGE(0x2000, 0x23ff) AM_RAM AM_SHARE("saram")       // SARAM        TODO: is off-chip if RAM bit = 0
	AM_RANGE(0xfe00, 0xffff) AM_RAM AM_SHARE("daram_b0")    // DARAM B0     TODO: is off-chip if CNF = 0
ADDRESS_MAP_END

static ADDRESS_MAP_START( internal_data, AS_DATA, 16, tms32051_device )
	AM_RANGE(0x0000, 0x005f) AM_READWRITE(cpuregs_r, cpuregs_w)
	AM_RANGE(0x0060, 0x007f) AM_RAM                         // DARAM B2
	AM_RANGE(0x0100, 0x02ff) AM_RAM AM_SHARE("daram_b0")    // DARAM B0     TODO: is unconnected if CNF = 1
	AM_RANGE(0x0300, 0x04ff) AM_RAM                         // DARAM B1
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("saram")       // SARAM        TODO: is off-chip if OVLY = 0
ADDRESS_MAP_END


tms32051_device::tms32051_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, TMS32051, "TMS32051", tag, owner, clock, "tms32051", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 16, -1, ADDRESS_MAP_NAME(internal_pgm))
	, m_data_config("data", ENDIANNESS_LITTLE, 16, 16, -1, ADDRESS_MAP_NAME(internal_data))
{
}


offs_t tms32051_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( tms32051 );
	return CPU_DISASSEMBLE_NAME(tms32051)(this, buffer, pc, oprom, opram, options);
}


#define CYCLES(x)       (m_icount -= x)

#define ROPCODE()       m_direct->read_word((m_pc++) << 1)

void tms32051_device::CHANGE_PC(UINT16 new_pc)
{
	m_pc = new_pc;
}

UINT16 tms32051_device::PM_READ16(UINT16 address)
{
	return m_program->read_word(address << 1);
}

void tms32051_device::PM_WRITE16(UINT16 address, UINT16 data)
{
	m_program->write_word(address << 1, data);
}

UINT16 tms32051_device::DM_READ16(UINT16 address)
{
	return m_data->read_word(address << 1);
}

void tms32051_device::DM_WRITE16(UINT16 address, UINT16 data)
{
	m_data->write_word(address << 1, data);
}

#include "32051ops.cpp"
#include "32051ops.h"

void tms32051_device::op_group_be()
{
	(this->*s_opcode_table_be[m_op & 0xff])();
}

void tms32051_device::op_group_bf()
{
	(this->*s_opcode_table_bf[m_op & 0xff])();
}

void tms32051_device::delay_slot(UINT16 startpc)
{
	m_op = ROPCODE();
	(this->*s_opcode_table[m_op >> 8])();

	while (m_pc - startpc < 2)
	{
		m_op = ROPCODE();
		(this->*s_opcode_table[m_op >> 8])();
	}
}

/*****************************************************************************/

void tms32051_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_data = &space(AS_DATA);

	m_pcstack_ptr = 0;
	m_op = 0;
	m_acc = 0;
	m_accb = 0;
	m_preg = 0;
	m_treg0 = 0;
	m_treg1 = 0;
	m_treg2 = 0;
	memset(m_ar, 0, sizeof(m_ar));
	m_bmar = 0;
	m_brcr = 0;
	m_paer = 0;
	m_pasr = 0;
	m_indx = 0;
	m_dbmr = 0;
	m_arcr = 0;
	memset(&m_st0, 0, sizeof(m_st0));
	memset(&m_st1, 0, sizeof(m_st1));
	memset(&m_pmst, 0, sizeof(m_pmst));
	memset(m_pcstack, 0, sizeof(m_pcstack));
	m_imr = 0;
	m_cbsr1 = 0;
	m_cber1 = 0;
	m_cbsr2 = 0;
	m_cber2 = 0;
	memset(&m_timer, 0, sizeof(m_timer));
	memset(&m_serial, 0, sizeof(m_serial));

	state_add( TMS32051_PC,    "PC", m_pc).formatstr("%04X");
	state_add( TMS32051_ACC,   "ACC", m_acc).formatstr("%08X");
	state_add( TMS32051_ACCB,  "ACCB", m_accb).formatstr("%08X");
	state_add( TMS32051_PREG,  "PREG", m_preg).formatstr("%08X");
	state_add( TMS32051_TREG0, "TREG0", m_treg0).formatstr("%04X");
	state_add( TMS32051_TREG1, "TREG1", m_treg1).formatstr("%04X");
	state_add( TMS32051_TREG2, "TREG2", m_treg2).formatstr("%04X");
	state_add( TMS32051_BMAR,  "BMAR", m_bmar).formatstr("%08X");
	state_add( TMS32051_RPTC,  "RPTC", m_rptc).formatstr("%08X");
	state_add( TMS32051_BRCR,  "BRCR", m_brcr).formatstr("%08X");
	state_add( TMS32051_INDX,  "INDX", m_indx).formatstr("%04X");
	state_add( TMS32051_DBMR,  "DBMR", m_dbmr).formatstr("%04X");
	state_add( TMS32051_ARCR,  "ARCR", m_arcr).formatstr("%04X");
	state_add( TMS32051_DP,    "DP", m_st0.dp).formatstr("%04X");
	state_add( TMS32051_ARP,   "ARP", m_st0.arp).formatstr("%04X");
	state_add( TMS32051_ARB,   "ARB", m_st1.arb).formatstr("%04X");
	state_add( TMS32051_AR0,   "AR0", m_ar[0]).formatstr("%04X");
	state_add( TMS32051_AR1,   "AR1", m_ar[1]).formatstr("%04X");
	state_add( TMS32051_AR2,   "AR2", m_ar[2]).formatstr("%04X");
	state_add( TMS32051_AR3,   "AR3", m_ar[3]).formatstr("%04X");
	state_add( TMS32051_AR4,   "AR4", m_ar[4]).formatstr("%04X");
	state_add( TMS32051_AR5,   "AR5", m_ar[5]).formatstr("%04X");
	state_add( TMS32051_AR6,   "AR6", m_ar[6]).formatstr("%04X");
	state_add( TMS32051_AR7,   "AR7", m_ar[7]).formatstr("%04X");

	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%04X").noshow();

	m_icountptr = &m_icount;
}

void tms32051_device::device_reset()
{
	// reset registers
	m_st0.intm  = 1;
	m_st0.ov    = 0;
	m_st1.c     = 1;
	m_st1.cnf   = 0;
	m_st1.hm    = 1;
	m_st1.pm    = 0;
	m_st1.sxm   = 1;
	m_st1.xf    = 1;
	m_pmst.avis = 0;
	m_pmst.braf = 0;
	m_pmst.iptr = 0;
	m_pmst.ndx  = 0;
	m_pmst.ovly = 0;
	m_pmst.ram  = 0;
	m_pmst.mpmc = 0; // TODO: this is set to logical pin state at reset
	m_pmst.trm  = 0;
	m_ifr       = 0;
	m_cbcr      = 0;
	m_rptc      = -1;

	// simulate internal rom boot loader (can be removed when the dsp rom(s) is dumped)
	m_st0.intm  = 1;
	m_st1.cnf   = 1;
	m_pmst.ram  = 1;
	m_pmst.ovly = 0;

	int i;
	UINT16 src, dst, length;

	src = 0x7800;
	dst = DM_READ16(src++);
	length = DM_READ16(src++);

	CHANGE_PC(dst);

	/* TODO: if you soft reset on Taito JC it tries to do a 0x7802->0x9007 (0xff00) transfer. */
	for (i=0; i < (length & 0x7ff); i++)
	{
		UINT16 data = DM_READ16(src++);
		PM_WRITE16(dst++, data);
	}
}

void tms32051_device::check_interrupts()
{
	if (m_st0.intm == 0 && m_ifr != 0)
	{
		for (int i = 0; i < 16; i++)
		{
			if (m_ifr & (1 << i))
			{
				m_st0.intm = 1;
				PUSH_STACK(m_pc);

				m_pc = (m_pmst.iptr << 11) | ((i+1) << 1);
				m_ifr &= ~(1 << i);

				save_interrupt_context();
				break;
			}
		}
	}
}

void tms32051_device::save_interrupt_context()
{
	m_shadow.acc = m_acc;
	m_shadow.accb = m_accb;
	m_shadow.arcr = m_arcr;
	m_shadow.indx = m_indx;
	m_shadow.preg = m_preg;
	m_shadow.treg0 = m_treg0;
	m_shadow.treg1 = m_treg1;
	m_shadow.treg2 = m_treg2;
	memcpy(&m_shadow.pmst, &m_pmst, sizeof(TMS32051_PMST));
	memcpy(&m_shadow.st0, &m_st0, sizeof(TMS32051_ST0));
	memcpy(&m_shadow.st1, &m_st1, sizeof(TMS32051_ST1));
}

void tms32051_device::restore_interrupt_context()
{
	m_acc = m_shadow.acc;
	m_accb = m_shadow.accb;
	m_arcr = m_shadow.arcr;
	m_indx = m_shadow.indx;
	m_preg = m_shadow.preg;
	m_treg0 = m_shadow.treg0;
	m_treg1 = m_shadow.treg1;
	m_treg2 = m_shadow.treg2;
	memcpy(&m_pmst, &m_shadow.pmst, sizeof(TMS32051_PMST));
	memcpy(&m_st0, &m_shadow.st0, sizeof(TMS32051_ST0));
	memcpy(&m_st1, &m_shadow.st1, sizeof(TMS32051_ST1));
}

void tms32051_device::execute_set_input(int irq, int state)
{
	if (state == ASSERT_LINE)
	{
		if ((m_imr & (1 << irq)) != 0)
		{
			m_ifr |= 1 << irq;
		}

		check_interrupts();
	}
}


void tms32051_device::execute_run()
{
	while (m_icount > 0)
	{
		UINT16 ppc;

		// handle block repeat
		if (m_pmst.braf)
		{
			if (m_pc == m_paer)
			{
				if (m_brcr > 0)
				{
					CHANGE_PC(m_pasr);
				}

				m_brcr--;
				if (m_brcr <= 0)
				{
					m_pmst.braf = 0;
				}
			}
		}

		ppc = m_pc;
		debugger_instruction_hook(this, m_pc);

		m_op = ROPCODE();
		(this->*s_opcode_table[m_op >> 8])();

		// handle single repeat
		if (m_rptc > 0)
		{
			if (ppc == m_rpt_end)
			{
				CHANGE_PC(m_rpt_start);
				m_rptc--;
			}
		}
		else
		{
			m_rptc = 0;
		}

		m_timer.psc--;
		if (m_timer.psc <= 0)
		{
			m_timer.psc = m_timer.tddr;
			m_timer.tim--;
			if (m_timer.tim <= 0)
			{
				// reset timer
				m_timer.tim = m_timer.prd;

				execute_set_input(TMS32051_TINT, ASSERT_LINE);
			}
		}
	}
}


/*****************************************************************************/

READ16_MEMBER( tms32051_device::cpuregs_r )
{
	switch (offset)
	{
		case 0x04:  return m_imr;
		case 0x06:  return m_ifr;

		case 0x07: // PMST
		{
			UINT16 r = 0;
			r |= m_pmst.iptr << 11;
			r |= m_pmst.avis << 7;
			r |= m_pmst.ovly << 5;
			r |= m_pmst.ram << 4;
			r |= m_pmst.mpmc << 3;
			r |= m_pmst.ndx << 2;
			r |= m_pmst.trm << 1;
			r |= m_pmst.braf << 0;
			return r;
		}

		case 0x09: return m_brcr;
		case 0x10: return m_ar[0];
		case 0x11: return m_ar[1];
		case 0x12: return m_ar[2];
		case 0x13: return m_ar[3];
		case 0x14: return m_ar[4];
		case 0x15: return m_ar[5];
		case 0x16: return m_ar[6];
		case 0x17: return m_ar[7];
		case 0x18: return m_indx;
		case 0x19: return m_arcr;
		case 0x1a: return m_cbsr1;
		case 0x1b: return m_cber1;
		case 0x1c: return m_cbsr2;
		case 0x1d: return m_cber2;
		case 0x1e: return m_cbcr;
		case 0x1f: return m_bmar;

		case 0x20: return m_serial.drr;
		case 0x21: return m_serial.dxr;

		case 0x24: return m_timer.tim;
		case 0x25: return m_timer.prd;

		case 0x26: // TCR
		{
			UINT16 r = 0;
			r |= (m_timer.psc & 0xf) << 6;
			r |= (m_timer.tddr & 0xf);
			return r;
		}

		case 0x28: // PDWSR
			return 0;

		default:
			if (!space.debugger_access())
				fatalerror("32051: cpuregs_r: unimplemented memory-mapped register %02X at %04X\n", offset, m_pc-1);
	}

	return 0;
}

WRITE16_MEMBER( tms32051_device::cpuregs_w )
{
	switch (offset)
	{
		case 0x00: break;
		case 0x04: m_imr = data; break;

		case 0x06: // IFR
		{
			for (int i = 0; i < 16; i++)
			{
				if (data & (1 << i))
				{
					m_ifr &= ~(1 << i);
				}
			}
			break;
		}

		case 0x07: // PMST
		{
			m_pmst.iptr = (data >> 11) & 0x1f;
			m_pmst.avis = (data & 0x80) ? 1 : 0;
			m_pmst.ovly = (data & 0x20) ? 1 : 0;
			m_pmst.ram = (data & 0x10) ? 1 : 0;
			m_pmst.mpmc = (data & 0x08) ? 1 : 0;
			m_pmst.ndx = (data & 0x04) ? 1 : 0;
			m_pmst.trm = (data & 0x02) ? 1 : 0;
			m_pmst.braf = (data & 0x01) ? 1 : 0;
			break;
		}

		case 0x09: m_brcr = data; break;
		case 0x0e: m_treg2 = data; break;
		case 0x0f: m_dbmr = data; break;
		case 0x10: m_ar[0] = data; break;
		case 0x11: m_ar[1] = data; break;
		case 0x12: m_ar[2] = data; break;
		case 0x13: m_ar[3] = data; break;
		case 0x14: m_ar[4] = data; break;
		case 0x15: m_ar[5] = data; break;
		case 0x16: m_ar[6] = data; break;
		case 0x17: m_ar[7] = data; break;
		case 0x18: m_indx = data; break;
		case 0x19: m_arcr = data; break;
		case 0x1a: m_cbsr1 = data; break;
		case 0x1b: m_cber1 = data; break;
		case 0x1c: m_cbsr2 = data; break;
		case 0x1d: m_cber2 = data; break;
		case 0x1e: m_cbcr = data; break;
		case 0x1f: m_bmar = data; break;

		case 0x20: m_serial.drr = data; break;
		case 0x21: m_serial.dxr = data; break;
		case 0x22: m_serial.spc = data; break;

		case 0x24: m_timer.tim = data; break;
		case 0x25: m_timer.prd = data; break;

		case 0x26: // TCR
		{
			m_timer.tddr = data & 0xf;
			m_timer.psc = (data >> 6) & 0xf;

			if (data & 0x20)
			{
				m_timer.tim = m_timer.prd;
				m_timer.psc = m_timer.tddr;
			}
			break;
		}

		case 0x28: // PDWSR
			break;

		default:
			if (!space.debugger_access())
				fatalerror("32051: cpuregs_w: unimplemented memory-mapped register %02X, data %04X at %04X\n", offset, data, m_pc-1);
	}
}


bool tms32051_device::memory_read(address_spacenum spacenum, offs_t offset, int size, UINT64 &value)
{
	/* TODO: alignment if offset is odd */
	if (spacenum == AS_PROGRAM)
	{
		value = (PM_READ16(offset>>1));
	}
	else if (spacenum == AS_DATA)
	{
		value = (DM_READ16(offset>>1));
	}
	return 1;
}
