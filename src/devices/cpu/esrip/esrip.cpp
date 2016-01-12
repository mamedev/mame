// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    esrip.c

    Implementation of the Entertainment Sciences
    AM29116-based Real Time Image Processor

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "esrip.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define IPT_RAM_SIZE    (8192 * sizeof(UINT16))


/***************************************************************************
    MACROS
***************************************************************************/

#define RIP_PC      (m_pc | ((m_status_out & 1) << 8))
#define _BIT(x, n)  ((x) & (1 << (n)))
#define RISING_EDGE(old_val, new_val, bit)  (!(old_val & (1 << bit)) && (new_val & (1 << bit)))

#define UNHANDLED   do {printf("%s:UNHANDLED (%x)\n", __FUNCTION__, inst); assert(0);} while (0)
#define INVALID     do {printf("%s:INVALID (%x)\n", __FUNCTION__, inst); assert(0);} while (0)

#define RAM_ADDR    (inst & 0x1f)
#define MODE        (inst & 0x8000)
#define WORD_MODE   (inst & 0x8000)
#define BYTE_MODE   (!WORD_MODE)
#define N           ((inst >> 9) & 0xf)
#define OPCODE      ((inst >> 5) & 0xf)
#define SRC         ((inst >> 9) & 0xf)
#define DST         (inst & 0x1f)   // TEST

#define BW_WORD     (1 << 15)
#define BW_BYTE     (0 << 15)

#define FLAG_3      (1 << 7)
#define FLAG_2      (1 << 6)
#define FLAG_1      (1 << 5)
#define L_FLAG      (1 << 4)
#define V_FLAG      (1 << 3)
#define N_FLAG      (1 << 2)
#define C_FLAG      (1 << 1)
#define Z_FLAG      (1 << 0)

#define CLEAR_FLAGS(a)  (m_new_status &= ~(a))
#define SET_FLAGS(a)    (m_new_status |=  (a))


/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

UINT8 esrip_device::get_rip_status()
{
	return m_status_out;
}


/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

enum ops
{
	ROTR1, TOR1, ROTR2, ROTC, ROTM, BOR2, CRCF, CRCR,
	SVSTR, PRT, SOR, TOR2, SHFTR, TEST, NOP, SETST, RSTST,
	ROTNR, BONR, BOR1, SONR, SHFTNR, PRTNR, TONR
};

void esrip_device::make_ops()
{
	int inst;

	for (inst = 0; inst < 65536; ++inst)
	{
		int quad = (inst >> 13) & 3;

		if (quad == 0)
		{
			if (((inst >> 5) & 0xc) == 0xc)
				m_optable[inst] = ROTR1;
			else
				m_optable[inst] = TOR1;
		}
		else if (quad == 1)
		{
			if (OPCODE < 2)
				m_optable[inst] = ROTR2;
			else if (OPCODE < 6)
				m_optable[inst] = ROTC;
			else
				m_optable[inst] = ROTM;
		}
		else if (quad == 2)
		{
			if (OPCODE > 11)
				m_optable[inst] = BOR2;
			else
			{
				int tmp = (inst >> 5) & 0xff;

				if (tmp == 0x63)
					m_optable[inst] = CRCF;
				else if (tmp == 0x69)
					m_optable[inst] = CRCR;
				else if (tmp == 0x7a)
					m_optable[inst] = SVSTR;
				else
				{
					if ((SRC > 7) && (SRC < 12))
						m_optable[inst] = PRT;
					else if (SRC > 11)
						m_optable[inst] = SOR;
					else if (SRC < 6)
						m_optable[inst] = TOR2;
					else
						m_optable[inst] = SHFTR;
				}
			}
		}
		else
		{
			if (inst == 0x7140)
				m_optable[inst] = NOP;
			else
			{
				int x = (inst & 0xffe0);
				if (x == 0x7340)
					m_optable[inst] = TEST;
				else if (x == 0x7740)
					m_optable[inst] = SETST;
				else if (x == 0x7540)
					m_optable[inst] = RSTST;
				else
				{
					int op = OPCODE;
					if (op == 0xc)
					{
						if ((inst & 0x18) == 0x18)
							m_optable[inst] = ROTNR;
						else
							m_optable[inst] = BONR;
					}
					else if ((op & 0xc) == 0xc)
						m_optable[inst] = BOR1;
					else
					{
						int src = SRC;

						if ((src & 0xc) == 0xc)
							m_optable[inst] = SONR;
						else if ((src & 0x6) == 0x6)
							m_optable[inst] = SHFTNR;
						else if (src & 0x8)
							m_optable[inst] = PRTNR;
						else
							m_optable[inst] = TONR;
					}
				}
			}
		}
	}
}

void esrip_device::device_start()
{
	/* Register configuration structure callbacks */
	m_fdt_r.resolve_safe(0);
	m_fdt_w.resolve_safe();
	m_lbrm = (UINT8*)machine().root_device().memregion(m_lbrm_prom)->base();
	m_status_in.resolve_safe(0);
	m_draw.bind_relative_to(*owner());

	/* Allocate image pointer table RAM */
	m_ipt_ram.resize(IPT_RAM_SIZE/2);

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	// register our state for the debugger
	state_add(STATE_GENPC,     "GENPC",     m_rip_pc).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_status).callimport().callexport().formatstr("%8s").noshow();
	state_add(ESRIP_PC,        "PC:",       m_rip_pc).mask(0xffff);
	state_add(ESRIP_ACC,       "ACC:",      m_acc).mask(0xffff);
	state_add(ESRIP_DLATCH,    "DLATCH:",   m_d_latch).mask(0xff);
	state_add(ESRIP_ILATCH,    "ILATCH:",   m_i_latch).mask(0xffff);
	state_add(ESRIP_RAM00,     "RAM[00]:",  m_ram[0x00]).mask(0xffff);
	state_add(ESRIP_RAM01,     "RAM[01]:",  m_ram[0x01]).mask(0xffff);
	state_add(ESRIP_RAM02,     "RAM[02]:",  m_ram[0x02]).mask(0xffff);
	state_add(ESRIP_RAM03,     "RAM[03]:",  m_ram[0x03]).mask(0xffff);
	state_add(ESRIP_RAM04,     "RAM[04]:",  m_ram[0x04]).mask(0xffff);
	state_add(ESRIP_RAM05,     "RAM[05]:",  m_ram[0x05]).mask(0xffff);
	state_add(ESRIP_RAM06,     "RAM[06]:",  m_ram[0x06]).mask(0xffff);
	state_add(ESRIP_RAM07,     "RAM[07]:",  m_ram[0x07]).mask(0xffff);
	state_add(ESRIP_RAM08,     "RAM[08]:",  m_ram[0x08]).mask(0xffff);
	state_add(ESRIP_RAM09,     "RAM[09]:",  m_ram[0x09]).mask(0xffff);
	state_add(ESRIP_RAM0A,     "RAM[0A]:",  m_ram[0x0a]).mask(0xffff);
	state_add(ESRIP_RAM0B,     "RAM[0B]:",  m_ram[0x0b]).mask(0xffff);
	state_add(ESRIP_RAM0C,     "RAM[0C]:",  m_ram[0x0c]).mask(0xffff);
	state_add(ESRIP_RAM0D,     "RAM[0D]:",  m_ram[0x0d]).mask(0xffff);
	state_add(ESRIP_RAM0E,     "RAM[0E]:",  m_ram[0x0e]).mask(0xffff);
	state_add(ESRIP_RAM0F,     "RAM[0F]:",  m_ram[0x0f]).mask(0xffff);
	state_add(ESRIP_RAM10,     "RAM[10]:",  m_ram[0x10]).mask(0xffff);
	state_add(ESRIP_RAM11,     "RAM[11]:",  m_ram[0x11]).mask(0xffff);
	state_add(ESRIP_RAM12,     "RAM[12]:",  m_ram[0x12]).mask(0xffff);
	state_add(ESRIP_RAM13,     "RAM[13]:",  m_ram[0x13]).mask(0xffff);
	state_add(ESRIP_RAM14,     "RAM[14]:",  m_ram[0x14]).mask(0xffff);
	state_add(ESRIP_RAM15,     "RAM[15]:",  m_ram[0x15]).mask(0xffff);
	state_add(ESRIP_RAM16,     "RAM[16]:",  m_ram[0x16]).mask(0xffff);
	state_add(ESRIP_RAM17,     "RAM[17]:",  m_ram[0x17]).mask(0xffff);
	state_add(ESRIP_RAM18,     "RAM[18]:",  m_ram[0x18]).mask(0xffff);
	state_add(ESRIP_RAM19,     "RAM[19]:",  m_ram[0x19]).mask(0xffff);
	state_add(ESRIP_RAM1A,     "RAM[1A]:",  m_ram[0x1a]).mask(0xffff);
	state_add(ESRIP_RAM1B,     "RAM[1B]:",  m_ram[0x1b]).mask(0xffff);
	state_add(ESRIP_RAM1C,     "RAM[1C]:",  m_ram[0x1c]).mask(0xffff);
	state_add(ESRIP_RAM1D,     "RAM[1D]:",  m_ram[0x1d]).mask(0xffff);
	state_add(ESRIP_RAM1E,     "RAM[1E]:",  m_ram[0x1e]).mask(0xffff);
	state_add(ESRIP_RAM1F,     "RAM[1F]:",  m_ram[0x1f]).mask(0xffff);
	state_add(ESRIP_STATW,     "STAT:",     m_status_out).mask(0xffff);
	state_add(ESRIP_FDTC,      "FDTC:",     m_fdt_cnt).mask(0xffff);
	state_add(ESRIP_IPTC,      "IPTC:",     m_ipt_cnt).mask(0xffff);
	state_add(ESRIP_XSCALE,    "XSCL:",     m_x_scale).mask(0xffff);
	state_add(ESRIP_YSCALE,    "YSCL:",     m_y_scale).mask(0xffff);
	state_add(ESRIP_BANK,      "BANK:",     m_img_bank).mask(0xffff);
	state_add(ESRIP_LINE,      "LINE:",     m_line_latch).mask(0xffff);
	state_add(ESRIP_FIG,       "FIG:",      m_fig_latch).mask(0xffff);
	state_add(ESRIP_ATTR,      "ATTR:",     m_attr_latch).mask(0xffff);
	state_add(ESRIP_ADRL,      "ADRL:",     m_adl_latch).mask(0xffff);
	state_add(ESRIP_ADRR,      "ADRR:",     m_adr_latch).mask(0xffff);
	state_add(ESRIP_COLR,      "COLR:",     m_c_latch).mask(0xffff);
	state_add(ESRIP_IADDR,     "IADR:",     m_iaddr_latch).mask(0xffff);

	/* Create the instruction decode lookup table */
	make_ops();

	/* Register stuff for state saving */
	save_item(NAME(m_acc));
	save_item(NAME(m_ram));
	save_item(NAME(m_d_latch));
	save_item(NAME(m_i_latch));
	save_item(NAME(m_result));
	save_item(NAME(m_new_status));
	save_item(NAME(m_status));
	save_item(NAME(m_inst));
	save_item(NAME(m_immflag));
	save_item(NAME(m_ct));
	save_item(NAME(m_t));
	save_item(NAME(m_l1));
	save_item(NAME(m_l2));
	save_item(NAME(m_l3));
	save_item(NAME(m_l4));
	save_item(NAME(m_l5));
	save_item(NAME(m_l6));
	save_item(NAME(m_l7));
	save_item(NAME(m_pl1));
	save_item(NAME(m_pl2));
	save_item(NAME(m_pl3));
	save_item(NAME(m_pl4));
	save_item(NAME(m_pl5));
	save_item(NAME(m_pl6));
	save_item(NAME(m_pl7));
	save_item(NAME(m_pc));
	save_item(NAME(m_status_out));
	save_item(NAME(m_x_scale));
	save_item(NAME(m_y_scale));
	save_item(NAME(m_img_bank));
	save_item(NAME(m_line_latch));
	save_item(NAME(m_fig_latch));
	save_item(NAME(m_attr_latch));
	save_item(NAME(m_adl_latch));
	save_item(NAME(m_adr_latch));
	save_item(NAME(m_iaddr_latch));
	save_item(NAME(m_c_latch));
	save_item(NAME(m_fdt_cnt));
	save_item(NAME(m_ipt_cnt));
	save_item(NAME(m_fig));
	save_item(NAME(m_fig_cycles));
	save_item(NAME(m_ipt_ram));

	// set our instruction counter
	m_icountptr = &m_icount;
	m_icount = 0;
}


void esrip_device::device_reset()
{
	m_pc = 0;

	m_pl1 = 0xff;
	m_pl2 = 0xff;
	m_pl3 = 0xff;
	m_pl4 = 0xff;
	m_pl5 = 0xff;
	m_pl6 = 0xff;
	m_pl7 = 0xff;

	m_l1 = 0xff;
	m_l2 = 0xff;
	m_l3 = 0xff;
	m_l4 = 0xff;
	m_l5 = 0xff;
	m_l6 = 0xff;
	m_l7 = 0xff;

	m_status_out = 0;
	m_immflag = 0;

	m_rip_pc = (m_pc | ((m_status_out & 1) << 8));
}

void esrip_device::device_stop()
{
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *esrip_device::memory_space_config(address_spacenum spacenum) const
{
	if (spacenum == AS_PROGRAM)
	{
		return &m_program_config;
	}
	return nullptr;
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void esrip_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c%c",
				(m_status & 0x80) ? '3' : '.',
				(m_status & 0x40) ? '2' : '.',
				(m_status & 0x20) ? '1' : '.',
				(m_status & 0x10) ? 'L' : '.',
				(m_status & 0x08) ? 'V' : '.',
				(m_status & 0x04) ? 'N' : '.',
				(m_status & 0x02) ? 'C' : '.',
				(m_status & 0x01) ? 'Z' : '.',
				get_hblank() ? 'H' : '.');
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 esrip_device::disasm_min_opcode_bytes() const
{
	return 8;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 esrip_device::disasm_max_opcode_bytes() const
{
	return 8;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t esrip_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( esrip );
	return CPU_DISASSEMBLE_NAME(esrip)(this, buffer, pc, oprom, opram, options);
}


/***************************************************************************
    PRIVATE FUNCTIONS
***************************************************************************/

int esrip_device::get_hblank() const
{
	return machine().first_screen()->hblank();
}

/* Return the state of the LBRM line (Y-scaling related) */
int esrip_device::get_lbrm() const
{
	int addr = ((m_y_scale & 0x3f) << 3) | ((m_line_latch >> 3) & 7);
	int sel = (m_line_latch & 7);

	UINT8 val = m_lbrm[addr];

	return (val >> sel) & 1;
}

int esrip_device::check_jmp(UINT8 jmp_ctrl) const
{
	int ret = 0;

	if (~jmp_ctrl & 0x10)
	{
		switch (jmp_ctrl & 7)
		{
			/* CT */      case 0: ret = m_ct;         break;
			/* T1 */      case 4: ret = BIT(m_t, 0);  break;
			/* T2 */      case 2: ret = BIT(m_t, 1);  break;
			/* T3 */      case 6: ret = BIT(m_t, 2);  break;
			/* T4 */      case 1: ret = BIT(m_t, 3);  break;
			/* /LBRM */   case 5: ret = !get_lbrm();  break;
			/* /HBLANK */ case 3: ret = !get_hblank(); break;
			/* JMP */     case 7: ret = 0;                    break;
		}

		ret ^= 1;
	}
	else if (~jmp_ctrl & 0x08)
	{
		switch (jmp_ctrl & 7)
		{
			/* CT */      case 0: ret = m_ct;        break;
			/* T1 */      case 4: ret = BIT(m_t, 0); break;
			/* T2 */      case 2: ret = BIT(m_t, 1); break;
			/* T3 */      case 6: ret = BIT(m_t, 2); break;
			/* T4 */      case 1: ret = BIT(m_t, 3); break;
			/* /LBRM */   case 5: ret = !get_lbrm(); break;
			/* /FIG */    case 3: ret = !m_fig;      break;
			/* JMP */     case 7: ret = 1;                   break;
		}
	}
	else
	{
		assert(!"RIP: Invalid jump control");
	}

	return ret;
}


void esrip_device::calc_z_flag(UINT16 res)
{
	m_new_status &= ~Z_FLAG;
	m_new_status |= (res == 0);
}

void esrip_device::calc_c_flag_add(UINT16 a, UINT16 b)
{
	m_new_status &= ~C_FLAG;
	m_new_status |= ((UINT16)(b) > (UINT16)(~(a))) ? 2 : 0;
}

void esrip_device::calc_c_flag_sub(UINT16 a, UINT16 b)
{
	m_new_status &= ~C_FLAG;
	m_new_status |= ((UINT16)(b) <= (UINT16)(a)) ? 2 : 0;
}

void esrip_device::calc_n_flag(UINT16 res)
{
	m_new_status &= ~N_FLAG;
	m_new_status |= (res & 0x8000) ? 4 : 0;
}

void esrip_device::calc_v_flag_add(UINT16 a, UINT16 b, UINT32 r)
{
	m_new_status &= ~V_FLAG;
	m_new_status |= ((a ^ r) & (b ^ r) & 0x8000) ? 8 : 0;
}

void esrip_device::calc_v_flag_sub(UINT16 a, UINT16 b, UINT32 r)
{
	m_new_status &= ~V_FLAG;
	m_new_status |= ((a ^ b) & (r ^ b) & 0x8000) ? 8 : 0;
}


/***************************************************************************
    INSTRUCTIONS
***************************************************************************/

enum
{
	ACC,
	Y_BUS,
	STATUS,
	RAM
};

/*************************************
 *
 *  Single operand
 *
 *************************************/

enum
{
	MOVE = 0xc,
	COMP = 0xd,
	INC  = 0xe,
	NEG  = 0xf
};

enum
{
	SORA  = 0x0,
	SORY  = 0x2,
	SORS  = 0x3,
	SOAR  = 0x4,
	SODR  = 0x6,
	SOIR  = 0x7,
	SOZR  = 0x8,
	SOZER = 0x9,
	SOSER = 0xa,
	SORR  = 0xb
};

UINT16 esrip_device::sor_op(UINT16 r, UINT16 opcode)
{
	UINT32 res = 0;

	switch (opcode)
	{
		case MOVE:
		{
			res = r;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}
		case COMP:
		{
			res = r ^ 0xffff;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}
		case INC:
		{
			res = r + 1;
			calc_v_flag_add(r, 1, res);
			calc_n_flag(res);
			calc_c_flag_add(r, 1);
			calc_z_flag(res);
			break;
		}
		case NEG:
		{
			res = (r ^ 0xffff) + 1;
			calc_v_flag_sub(0, r, res);
			calc_n_flag(res);
			calc_c_flag_sub(0, r);
			calc_z_flag(res);
			break;
		}
		default: assert(0);
	}

	return res & 0xffff;
}

void esrip_device::sor(UINT16 inst)
{
	UINT16  r = 0;
	UINT16  dst = 0;
	UINT16  res = 0;

	if (BYTE_MODE)
	{
		printf("Byte Mode! %.4x\n", inst);
		UNHANDLED;
	}

	switch ((inst >> 5) & 0xf)
	{
		case SORA: r = m_ram[RAM_ADDR];     dst = ACC;      break;
		case SORY: r = m_ram[RAM_ADDR];     dst = Y_BUS;    break;
		case SORS: r = m_ram[RAM_ADDR];     dst = STATUS;   break;
		case SOAR: r = m_acc;               dst = RAM;      break;
		case SODR: r = m_d_latch;           dst = RAM;      break;
		case SOIR:
		{
			if (m_immflag == 0)     // Macrofiy this?
			{
				m_i_latch = inst;
				m_immflag = 1;
				return;
			}
			else
			{
				r = m_inst;
				dst = RAM;
				m_immflag = 0;
			}
			break;
		}
		case SOZR: r = 0;                       dst = RAM;      break;
		case SORR: r = m_ram[RAM_ADDR]; dst = RAM;      break;
		default: UNHANDLED;
	}

	/* Operation */
	res = sor_op(r, (inst >> 9) & 0xf);

	switch (dst)
	{
		case Y_BUS: break;
		case ACC: m_acc = res; break;
		case RAM: m_ram[RAM_ADDR] = res; break;
		default: UNHANDLED;
	}

	m_result = res;
}

enum
{
	SOA  = 0x4,
	SOD  = 0x6,
	SOI  = 0x7,
	SOZ  = 0x8,
	SOZE = 0x9,
	SOSE = 0xa
};

enum
{
	NRY  = 0,
	NRA  = 1,
	NRS  = 4,
	NRAS = 5
};

void esrip_device::sonr(UINT16 inst)
{
	UINT16  r = 0;
	UINT16  res = 0;

	switch ((inst >> 5) & 0xf)
	{
		case SOA:   r = m_acc;      break;
		case SOD:   r = m_d_latch;  break;
		case SOI:
		{
			if (m_immflag == 0)
			{
				m_i_latch = inst;
				m_immflag = 1;
				return;
			}
			else
			{
				r = m_inst;
				m_immflag = 0;
			}
			break;
		}
		case SOZ:   r = 0; break;
		default: INVALID;
	}

	/* Operation */
	res = sor_op(r, (inst >> 9) & 0xf);

	/* Destination */
	switch (inst & 0x1f)
	{
		case NRY: break;
		case NRA: m_acc = res; break;
		default: UNHANDLED;
	}

	m_result = res;
}

/*************************************
 *
 *  Two operand
 *
 *************************************/

enum
{
	SUBR  = 0x0,
	SUBRC = 0x1,
	SUBS  = 0x2,
	SUBSC = 0x3,
	ADD   = 0x4,
	ADDC  = 0x5,
	AND   = 0x6,
	NAND  = 0x7,
	EXOR  = 0x8,
	NOR   = 0x9,
	OR    = 0xa,
	EXNOR = 0xb
};

UINT16 esrip_device::tor_op(UINT16 r, UINT16 s, int opcode)
{
	UINT32 res = 0;

	switch (opcode)
	{
		case SUBR:
		{
			res = s - r;
			calc_v_flag_sub(s, r, res);
			calc_n_flag(res);
			calc_c_flag_sub(s, r);
			calc_z_flag(res);
			break;
		}
		case SUBRC: assert(0); break;
		case SUBS:
		{
			res = r - s;
			calc_v_flag_sub(r, s, res);
			calc_n_flag(res);
			calc_c_flag_sub(r, s);
			calc_z_flag(res);
			break;
		}
		case SUBSC: assert(0); break;
		case ADD:
		{
			res = r + s;
			calc_v_flag_add(r, s, res);
			calc_n_flag(res);
			calc_c_flag_add(r, s);
			calc_z_flag(res);
			break;
		}
		case ADDC:
		{
			// TODO TODO CHECK ME ETC
			res = r + s + ((m_status >> 1) & 1);
			calc_v_flag_add(r, s, res);
			calc_n_flag(res);
			calc_c_flag_add(r, s);
			calc_z_flag(res);
			break;
		}
		case AND:
		{
			res = r & s;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}
		case NAND:
		{
			res = (r & s) ^ 0xffff;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}
		case EXOR:
		{
			res = r ^ s;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}
		case NOR:
		{
			res = (r | s) ^ 0xffff;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}
		case OR:
		{
			res = r | s;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}
		case EXNOR:
		{
			res = (r ^ s) ^ 0xffff;
			CLEAR_FLAGS(V_FLAG | N_FLAG | C_FLAG);
			calc_z_flag(res);
			break;
		}
		default: assert(0);
	}

	return res & 0xffff;
}

void esrip_device::tor1(UINT16 inst)
{
	UINT16 r = 0;
	UINT16 s = 0;
	UINT16 dst = 0;
	UINT16  res = 0;

	enum
	{
		TORAA = 0x0,
		TORIA = 0x2,
		TODRA = 0x3,
		TORAY = 0x8,
		TORIY = 0xa,
		TODRY = 0xb,
		TORAR = 0xc,
		TORIR = 0xe,
		TODRR = 0xf
	};

	switch (SRC)
	{
		case TORAA: r = m_ram[RAM_ADDR];    s = m_acc;  dst = ACC;  break;
		case TORIA:
		{
			if (m_immflag == 0)
			{
				m_i_latch = inst;
				m_immflag = 1;
				return;
			}
			else
			{
				r = m_ram[RAM_ADDR];
				s = m_inst;
				dst = ACC;
				m_immflag = 0;
			}
			break;
		}
		case TODRA: r = m_d_latch;      s = m_ram[RAM_ADDR];    dst = ACC;  break;
		case TORAY: r = m_ram[RAM_ADDR];    s = m_acc;          dst = Y_BUS;break;
		case TORIY:
		{
			if (m_immflag == 0)
			{
				m_i_latch = inst;
				m_immflag = 1;
				return;
			}
			else
			{
				r = m_ram[RAM_ADDR];
				s = m_inst;
				dst = Y_BUS;
				m_immflag = 0;
			}
			break;
		}
		case TODRY: r = m_d_latch;      s = m_ram[RAM_ADDR];    dst = Y_BUS;break;
		case TORAR: r = m_ram[RAM_ADDR];    s = m_acc;          dst = RAM;  break;
		case TORIR:
		{
			if (m_immflag == 0)
			{
				m_i_latch = inst;
				m_immflag = 1;
				return;
			}
			else
			{
				r = m_ram[RAM_ADDR];
				s = m_inst;
				dst = RAM;
				m_immflag = 0;
			}
			break;
		}
		case TODRR: r = m_d_latch;      s = m_ram[RAM_ADDR];    dst = RAM;  break;
		default: INVALID;
	}

	/* Operation */
	res = tor_op(r, s, (inst >> 5) & 0xf);

	/* Destination */
	switch (dst)
	{
		case ACC:   m_acc = res; break;
		case Y_BUS: break;
		case RAM:   m_ram[RAM_ADDR] = res; break;
		default:    INVALID;
	}

	m_result = res;
}

void esrip_device::tor2(UINT16 inst)
{
	UINT16 r = 0;
	UINT16 s = 0;
	UINT32 res = 0;

	enum
	{
		TODAR = 0x1,
		TOAIR = 0x2,
		TODIR = 0x5
	};

	switch (SRC)
	{
		case TODAR: r = m_d_latch;  s = m_acc;  break;
		case TOAIR:
		{
			if (m_immflag == 0)
			{
				m_i_latch = inst;
				m_immflag = 1;
				return;
			}
			else
			{
				r = m_acc;
				s = m_inst;
				m_immflag = 0;
			}
			break;
		}
		case TODIR:
		{
			if (m_immflag == 0)
			{
				m_i_latch = inst;
				m_immflag = 1;
				return;
			}
			else
			{
				r = m_d_latch;
				s = m_inst;
				m_immflag = 0;
			}
			break;
		}
		default: INVALID;
	}

	/* Operation */
	res = tor_op(r, s, (inst >> 5) & 0xf);

	/* Destination is always RAM */
	m_ram[RAM_ADDR] = res;

	m_result = res;
}

void esrip_device::tonr(UINT16 inst)
{
	enum
	{
		TODA  = 0x1,
		TOAI  = 0x2,
		TODI  = 0x5
	};

	UINT16 r = 0;
	UINT16 s = 0;
	UINT16  res = 0;

	switch (SRC)
	{
		case TODA:
		{
			r = m_d_latch;
			s = m_acc;
			break;
		}
		case TOAI:
		{
			break;
		}
		case TODI:
		{
			if (m_immflag == 0)
			{
				m_i_latch = inst;
				m_immflag = 1;
				return;
			}
			else
			{
				r = m_d_latch;
				s = m_inst;
				m_immflag = 0;
			}
			break;
		}
		default: INVALID;
	}

	/* Operation */
	res = tor_op(r, s, (inst >> 5) & 0xf);

	/* Destination */
	switch (DST)
	{
		case NRY:
			break;
		case NRA:
			m_acc = res;
			break;
		case NRS:
			UNHANDLED;
			break;
		case NRAS:
			UNHANDLED;
			break;
		default:
			INVALID;
	}
	m_result = res;
}

/*************************************
 *
 *  Bit operation
 *
 *************************************/

void esrip_device::bonr(UINT16 inst)
{
	enum
	{
		TSTNA  = 0x00,
		RSTNA  = 0x01,
		SETNA  = 0x02,
		A2NA   = 0x04,
		S2NA   = 0x05,
		LD2NA  = 0x06,
		LDC2NA = 0x07,
		TSTND  = 0x10,
		RSTND  = 0x11,
		SETND  = 0x12,
		A2NDY  = 0x14,
		S2NDY  = 0x15,
		LD2NY  = 0x16,
		LDC2NY = 0x17
	};

	UINT16  res = 0;

	switch (inst & 0x1f)
	{
		case TSTNA:
		{
			res = m_acc & (1 << N);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}
		case RSTNA:
		{
			res = m_acc & ~(1 << N);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			m_acc = res;
			break;
		}
		case SETNA:
		{
			res = m_acc | (1 << N);
			CLEAR_FLAGS(V_FLAG | C_FLAG | Z_FLAG);
			calc_n_flag(res);
			m_acc = res;
			break;
		}
		case A2NA:
		{
			UINT16 r = m_acc;
			UINT16 s = 1 << N;
			res = r + s;
			calc_z_flag(res);
			calc_n_flag(res);
			calc_c_flag_add(r, s);
			calc_v_flag_add(r, s, res);
			m_acc = res;
			break;
		}
		case S2NA:
		{
			UINT16 r = m_acc;
			UINT16 s = 1 << N;
			res = r - s;
			calc_z_flag(res);
			calc_n_flag(res);
			calc_c_flag_sub(r, s);
			calc_v_flag_sub(r, s, res);
			m_acc = res;
			break;
		}

		case TSTND:
		{
			res = m_d_latch & (1 << N);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}

		case SETND:
		{
			UINT16 r = m_d_latch;
			res = r | (1 << N);
			m_d_latch = res;

			CLEAR_FLAGS(V_FLAG | C_FLAG | Z_FLAG);
			calc_n_flag(res);
			break;
		}
		case LD2NY:
		{
			res = (1 << N);
			CLEAR_FLAGS(V_FLAG | C_FLAG | Z_FLAG);
			calc_n_flag(res);
			break;
		}
		case LDC2NY:
		{
			res = (1 << N) ^ 0xffff;
			CLEAR_FLAGS(Z_FLAG | C_FLAG | V_FLAG);
			calc_n_flag(res);
			break;
		}

		case A2NDY:
		{
			UINT16 r = m_d_latch;
			UINT16 s = 1 << N;
			res = r + s;

			calc_z_flag(res);
			calc_n_flag(res);
			calc_c_flag_add(r, s);
			calc_v_flag_add(r, s, res);
			break;
		}

		default:
			UNHANDLED;
	}

	m_result = res;
}

void esrip_device::bor1(UINT16 inst)
{
	enum
	{
		SETNR  = 0xd,
		RSTNR  = 0xe,
		TSTNR  = 0xf
	};

	UINT16  res = 0;

	switch ((inst >> 5) & 0xf)
	{
		case SETNR:
		{
			res = m_ram[RAM_ADDR] | (1 << N);
			m_ram[RAM_ADDR] = res;
			CLEAR_FLAGS(V_FLAG | C_FLAG | Z_FLAG);
			calc_n_flag(res);
			break;
		}
		case RSTNR:
		{
			res = m_ram[RAM_ADDR] & ~(1 << N);
			m_ram[RAM_ADDR] = res;
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}
		case TSTNR:
		{
			res = m_ram[RAM_ADDR] & (1 << N);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}
		default: INVALID;
	}

	m_result = res;
}

void esrip_device::bor2(UINT16 inst)
{
	enum
	{
		LD2NR  = 0xc,
		LDC2NR = 0xd,
		A2NR   = 0xe,
		S2NR   = 0xf
	};

	UINT32 res = 0;

	switch ((inst >> 5) & 0xf)
	{
		case LD2NR:
		{
			res = 1 << N;
			CLEAR_FLAGS(V_FLAG | C_FLAG | Z_FLAG);
			calc_n_flag(res);
			break;
		}
		case LDC2NR:
		{
			res = (1 << N) ^ 0xffff;
			CLEAR_FLAGS(V_FLAG | C_FLAG | Z_FLAG);
			calc_n_flag(res);
			break;
		}
		case A2NR:
		{
			UINT16 r = m_ram[RAM_ADDR];
			UINT16 s = 1 << N;

			res = r + s;
			calc_v_flag_add(r, s, res);
			calc_n_flag(res);
			calc_c_flag_add(r, s);
			calc_z_flag(res);
			break;
		}
		case S2NR:
		{
			UINT16 r = m_ram[RAM_ADDR];
			UINT16 s = 1 << N;

			res = r - s;
			calc_v_flag_sub(r, s, res);
			calc_n_flag(res);
			calc_c_flag_sub(r, s);
			calc_z_flag(res);
			break;
		}
		default: INVALID;
	}

	/* Destination is RAM */
	m_ram[RAM_ADDR] = res;
	m_result = res;
}

/*************************************
 *
 *  Rotate
 *
 *************************************/

/* TODO Combine these */
void esrip_device::rotr1(UINT16 inst)
{
	enum
	{
		RTRA = 0xc,
		RTRY = 0xd,
		RTRR = 0xf
	};

	UINT16  u = 0;
	UINT16  dst = 0;
	UINT16  res = 0;
	int     n = N;

	switch ((inst >> 5) & 0xf)
	{
		case RTRA: u = m_ram[RAM_ADDR]; dst = ACC;      break;
		case RTRY: u = m_ram[RAM_ADDR]; dst = Y_BUS;    break;
		case RTRR: u = m_ram[RAM_ADDR]; dst = RAM;      break;
		default: INVALID;
	}

	res = (u << n) | (u >> (16 - n));
	CLEAR_FLAGS(V_FLAG | C_FLAG);
	calc_n_flag(res);
	calc_z_flag(res);

	switch (dst)
	{
		case ACC: m_acc = res; break;
		case RAM: m_ram[RAM_ADDR] = res; break;
	}

	m_result = res;
}

void esrip_device::rotr2(UINT16 inst)
{
	enum
	{
		RTAR = 0,
		RTDR = 1
	};

	UINT16  u = 0;
	UINT16  res = 0;

	switch ((inst >> 5) & 0xf)
	{
		case RTAR: u = m_acc;       break;
		case RTDR: u = m_d_latch;   break;
		default: INVALID;
	}

	res = (u << N) | (u >> (16 - N));
	CLEAR_FLAGS(V_FLAG | C_FLAG);
	calc_n_flag(res);
	calc_z_flag(res);
	m_ram[RAM_ADDR] = res;

	m_result = res;
}

void esrip_device::rotnr(UINT16 inst)
{
	enum
	{
		RTDY = 0x18,
		RTDA = 0x19,
		RTAY = 0x1c,
		RTAA = 0x1d
	};

	UINT16  u = 0;
	UINT16  res = 0;
	UINT16  dst = 0;

	switch (inst & 0x1f)
	{
		case RTDY: u = m_d_latch;   dst = Y_BUS;    break;
		case RTDA: u = m_d_latch;   dst = ACC;      break;
		case RTAY: u = m_acc;       dst = Y_BUS;    break;
		case RTAA: u = m_acc;       dst = ACC;      break;
		default: INVALID;
	}

	res = (u << N) | (u >> (16 - N));
	CLEAR_FLAGS(V_FLAG | C_FLAG);
	calc_n_flag(res);
	calc_z_flag(res);

	switch (dst)
	{
		case Y_BUS: break;
		case ACC: m_acc = res; break;
		case RAM: m_ram[RAM_ADDR] = res; break;
		default: UNHANDLED;
	}

	m_result = res;
}

/*************************************
 *
 *  Rotate and compare
 *
 *************************************/

void esrip_device::rotc(UINT16 inst)
{
	UNHANDLED;
}

/*************************************
 *
 *  Rotate and merge
 *
 *************************************/

void esrip_device::rotm(UINT16 inst)
{
	UNHANDLED;
}

/*************************************
 *
 *  Prioritize
 *
 *************************************/

void esrip_device::prt(UINT16 inst)
{
	UNHANDLED;
}

void esrip_device::prtnr(UINT16 inst)
{
	UNHANDLED;
}


/*************************************
 *
 *  CRC
 *
 *************************************/

void esrip_device::crcf(UINT16 inst)
{
	UNHANDLED;
}

void esrip_device::crcr(UINT16 inst)
{
	UNHANDLED;
}

/*************************************
 *
 *  Single bit shift
 *
 *************************************/

enum
{
	SHUPZ  = 0,
	SHUP1  = 1,
	SHUPL  = 2,
	SHDNZ  = 4,
	SHDN1  = 5,
	SHDNL  = 6,
	SHDNC  = 7,
	SHDNOV = 8
};

#define SET_LINK_flag(x)    (m_new_status &= ~L_FLAG); \
							(m_new_status |= x ? L_FLAG : 0)

UINT16 esrip_device::shift_op(UINT16 u, int opcode)
{
	UINT32 res = 0;

	switch (opcode)
	{
		case SHUPZ:
		{
			res = (u << 1);
			SET_LINK_flag(u & 0x8000);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}
		case SHUP1:
		{
			res = (u << 1) | 1;
			SET_LINK_flag(u & 0x8000);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}
		case SHUPL:
		{
			res = (u << 1) | ((m_status & L_FLAG) ? 1 : 0);
			SET_LINK_flag(u & 0x8000);
			CLEAR_FLAGS(V_FLAG | C_FLAG);
			calc_n_flag(res);
			calc_z_flag(res);
			break;
		}

		case SHDNZ:
		case SHDN1:
		case SHDNL:
		case SHDNC:
		case SHDNOV:
		default: assert(0);
	}

	return res;
}

void esrip_device::shftr(UINT16 inst)
{
	enum
	{
		SHRR = 6,
		SHDR = 7
	};

	UINT16  u = 0;
	UINT16  res = 0;

	switch ((inst >> 9) & 0xf)
	{
		case SHRR: u = m_ram[RAM_ADDR]; break;
		case SHDR: u = m_d_latch;           break;
		default: INVALID;
	}

	res = shift_op(u, (inst >> 5) & 0xf);

	/* Destination is always RAM */
	m_ram[RAM_ADDR] = res;
	m_result = res;
}

void esrip_device::shftnr(UINT16 inst)
{
	enum
	{
		SHA = 6,
		SHD = 7
	};

	UINT16  u = 0;
	UINT16  res = 0;

	switch ((inst >> 9) & 0xf)
	{
		case SHA: u = m_acc;            break;
		case SHD: u = m_d_latch;        break;
		default: INVALID;
	}

	res = shift_op(u, (inst >> 5) & 0xf);

	switch (DST)
	{
		case NRY:   break;
		case NRA:   m_acc = res; break;
		default:    INVALID;
	}
	m_result = res;
}


/*************************************
 *
 *  Status
 *
 *************************************/

void esrip_device::svstr(UINT16 inst)
{
	UNHANDLED;
}

void esrip_device::rstst(UINT16 inst)
{
	enum
	{
		RONCZ = 0x3,
		RL    = 0x5,
		RF1   = 0x6,
		RF2   = 0x9,
		RF3   = 0xa
	};

	switch (inst & 0x1f)
	{
		case RONCZ: CLEAR_FLAGS(V_FLAG | N_FLAG | C_FLAG | Z_FLAG); break;
		case RL:    CLEAR_FLAGS(L_FLAG); break;
		case RF1:   CLEAR_FLAGS(FLAG_1); break;
		case RF2:   CLEAR_FLAGS(FLAG_2); break;
		case RF3:   CLEAR_FLAGS(FLAG_3); break;
	}

	m_result = 0;
}

void esrip_device::setst(UINT16 inst)
{
	enum
	{
		SONCZ = 0x3,
		SL    = 0x5,
		SF1   = 0x6,
		SF2   = 0x9,
		SF3   = 0xa
	};

	switch (inst & 0x1f)
	{
		case SONCZ: SET_FLAGS(V_FLAG | N_FLAG | C_FLAG | Z_FLAG); break;
		case SL:    SET_FLAGS(L_FLAG); break;
		case SF1:   SET_FLAGS(FLAG_1); break;
		case SF2:   SET_FLAGS(FLAG_2); break;
		case SF3:   SET_FLAGS(FLAG_3); break;
	}

	m_result = 0xffff;
}

void esrip_device::test(UINT16 inst)
{
	enum
	{
		TNOZ = 0x00,
		TNO  = 0x02,
		TZ   = 0x04,
		TOVR = 0x06,
		TLOW = 0x08,
		TC   = 0x0a,
		TZC  = 0x0c,
		TN   = 0x0e,
		TL   = 0x10,
		TF1  = 0x12,
		TF2  = 0x14,
		TF3  = 0x16
	};

	UINT32 res = 0;

	switch (inst & 0x1f)
	{
		case TNOZ: UNHANDLED; break;
		case TNO:  UNHANDLED; break;
		case TZ:   res = m_status & (Z_FLAG); break;
		case TOVR: res = m_status & (V_FLAG); break;
		case TLOW: UNHANDLED; break;
		case TC:   res = m_status & (C_FLAG); break;
		case TZC:  UNHANDLED; break;
		case TN:   res = m_status & (N_FLAG); break;
		case TL:   res = m_status & (L_FLAG); break;
		case TF1:  res = m_status & (FLAG_1); break;
		case TF2:  res = m_status & (FLAG_2); break;
		case TF3:  res = m_status & (FLAG_3); break;
		default:   INVALID;
	}

	m_ct = res ? 1 : 0;
}


/*************************************
 *
 *  No operation
 *
 *************************************/

void esrip_device::nop(UINT16 inst)
{
	m_result = 0xff;    // Undefined
}

//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

const device_type ESRIP = &device_creator<esrip_device>;

//-------------------------------------------------
//  esrip_device - constructor
//-------------------------------------------------

esrip_device::esrip_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, ESRIP, "ESRIP", tag, owner, clock, "esrip", __FILE__),
		m_program_config("program", ENDIANNESS_BIG, 64, 9, -3),
		m_fdt_r(*this),
		m_fdt_w(*this),
		m_status_in(*this),
		m_lbrm_prom(nullptr)
{
	// build the opcode table
	for (int op = 0; op < 24; op++)
		m_opcode[op] = s_opcodetable[op];
}


//**************************************************************************
//  STATIC OPCODE TABLES
//**************************************************************************

const esrip_device::ophandler esrip_device::s_opcodetable[24] =
{
	&esrip_device::rotr1,   &esrip_device::tor1,    &esrip_device::rotr2,   &esrip_device::rotc,
	&esrip_device::rotm,    &esrip_device::bor2,    &esrip_device::crcf,    &esrip_device::crcr,
	&esrip_device::svstr,   &esrip_device::prt,     &esrip_device::sor,     &esrip_device::tor2,
	&esrip_device::shftr,   &esrip_device::test,    &esrip_device::nop,     &esrip_device::setst,
	&esrip_device::rstst,   &esrip_device::rotnr,   &esrip_device::bonr,    &esrip_device::bor1,
	&esrip_device::sonr,    &esrip_device::shftnr,  &esrip_device::prtnr,   &esrip_device::tonr
};


void esrip_device::am29116_execute(UINT16 inst, int _sre)
{
	/* Status register shadow */
	m_new_status = m_status;

	/* Required for immediate source instructions */
	m_inst = inst;

	if (m_immflag == 1)
		inst = m_i_latch;

	(this->*m_opcode[m_optable[inst]])(inst);

	if (!_sre)
	{
		m_status = m_new_status;
		m_t = m_status;
	}
}


//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 esrip_device::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 esrip_device::execute_max_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 esrip_device::execute_input_lines() const
{
	return 0;
}


//-------------------------------------------------
//  execute_set_input - act on a changed input/
//  interrupt line
//-------------------------------------------------

void esrip_device::execute_set_input(int inputnum, int state)
{
}


void esrip_device::execute_run()
{
	int calldebugger = (machine().debug_flags & DEBUG_FLAG_ENABLED) != 0;
	UINT8 status;

	/* I think we can get away with placing this outside of the loop */
	status = m_status_in(*m_program, 0);

	/* Core execution loop */
	do
	{
		UINT64  inst;
		UINT8   next_pc;
		UINT16  x_bus = 0;
		UINT16  ipt_bus = 0;
		UINT16  y_bus = 0;

		int yoe = _BIT(m_l5, 1);
		int bl46 = BIT(m_l4, 6);
		int bl44 = BIT(m_l4, 4);

		UINT32 in_h;
		UINT32 in_l;

		if (m_fig_cycles)
		{
			if (--m_fig_cycles == 0)
				m_fig = 0;
		}

		/* /OEY = 1 : Y-bus is high imped */
		if (yoe)
		{
			/* Status In */
			if (!_BIT(m_l2, 0))
				y_bus = status | (!m_fig << 3);

			/* FDT RAM: /Enable, Direction and /RAM OE */
			else if (!bl44 && !_BIT(m_l2, 3) && bl46)
				y_bus = m_fdt_r(*m_program, m_fdt_cnt, 0xffff);

			/* IPT RAM: /Enable and /READ */
			else if (!_BIT(m_l2, 6) && !_BIT(m_l4, 5))
				y_bus = m_ipt_ram[m_ipt_cnt];

			/* DLE  - latch the value on the Y-BUS into the data latch */
			if (_BIT(m_l5, 0))
				m_d_latch = y_bus;

			/* Now execute the AM29116 instruction */
			am29116_execute((m_l7 << 8) | m_l6, BIT(m_l5, 2));
		}
		else
		{
			am29116_execute((m_l7 << 8) | m_l6, BIT(m_l5, 2));

			y_bus = m_result;

			if (BIT(m_l5, 0))
				m_d_latch = y_bus;
		}

		/* Determine what value is on the X-Bus */

		/* FDT RAM */
		if (!bl44)
			x_bus = m_fdt_r(*m_program, m_fdt_cnt, 0xffff);

		/* Buffer is enabled - write direction */
		else if (!BIT(m_l2, 3) && !bl46)
		{
			if (!yoe)
				x_bus = y_bus;
			else if ( !BIT(m_l2, 6) && !BIT(m_l4, 5) )
				x_bus = m_ipt_ram[m_ipt_cnt];
		}

		/* IPT BUS */
		if (!BIT(m_l2, 6))
			ipt_bus = m_ipt_ram[m_ipt_cnt];
		else if (!BIT(m_l4, 5))
		{
			if (!BIT(m_l5, 1))
				ipt_bus = y_bus;
			else
				ipt_bus = x_bus;
		}


		/* Write FDT RAM: /Enable, Direction and WRITE */
		if (!BIT(m_l2, 3) && !bl46 && !BIT(m_l4, 3))
			m_fdt_w(*m_program, m_fdt_cnt, x_bus, 0xffff);

		/* Write IPT RAM: /Enable and /WR */
		if (!BIT(m_l2, 7) && !BIT(m_l4, 5))
			m_ipt_ram[m_ipt_cnt] = ipt_bus;


		if ((((m_l5 >> 3) & 0x1f) & 0x18) != 0x18)
		{
			if ( check_jmp((m_l5 >> 3) & 0x1f) )
				next_pc = m_l1;
			else
				next_pc = m_pc + 1;
		}
		else
			next_pc = m_pc + 1;

		m_pl1 = m_l1;
		m_pl2 = m_l2;
		m_pl3 = m_l3;
		m_pl4 = m_l4;
		m_pl5 = m_l5;
		m_pl6 = m_l6;
		m_pl7 = m_l7;

		/* Latch instruction */
		inst = m_direct->read_qword(RIP_PC << 3);

		in_h = inst >> 32;
		in_l = inst & 0xffffffff;

		m_l1 = (in_l >> 8);
		m_l2 = (in_l >> 16);
		m_l3 = (in_l >> 24);

		m_l4 = (in_h >> 0);
		m_l5 = (in_h >> 8);
		m_l6 = (in_h >> 16);
		m_l7 = (in_h >> 24);

		/* Colour latch */
		if (RISING_EDGE(m_pl3, m_l3, 0))
			m_c_latch = (x_bus >> 12) & 0xf;

		/* Right pixel line buffer address */
		if (RISING_EDGE(m_pl3, m_l3, 1))
			m_adr_latch = x_bus & 0xfff;

		/* Left pixel line buffer address */
		if (RISING_EDGE(m_pl3, m_l3, 2))
			m_adl_latch = x_bus & 0xfff;

		/* FIGLD: Start the DMA */
		if (RISING_EDGE(m_pl3, m_l3, 3))
		{
			m_attr_latch = x_bus;

			m_fig = 1;
			m_fig_cycles = m_draw(m_adl_latch, m_adr_latch, m_fig_latch, m_attr_latch, m_iaddr_latch, m_c_latch, m_x_scale, m_img_bank);
		}

		/* X-scale */
		if (RISING_EDGE(m_pl3, m_l3, 4))
			m_x_scale = x_bus >> 8;

		/* Y-scale and image bank */
		if (RISING_EDGE(m_pl4, m_l4, 2))
		{
			m_y_scale = x_bus & 0xff;
			m_img_bank = (y_bus >> 14) & 3;
		}

		/* Image ROM address */
		if (RISING_EDGE(m_pl3, m_l3, 5))
			m_iaddr_latch = y_bus;

		/* IXLLD */
		if (RISING_EDGE(m_pl3, m_l3, 6))
		{
			m_line_latch = ipt_bus >> 10;
			m_fig_latch  = ipt_bus & 0x3ff;
		}

		/* Status write */
		if (RISING_EDGE(m_pl3, m_l3, 7))
			m_status_out = y_bus & 0xff;

		/* FDT address counter */
		if (!BIT(m_pl2, 1))
			m_fdt_cnt = y_bus & 0xfff;
		else if (BIT(m_pl2, 2))
			m_fdt_cnt = (m_fdt_cnt + 1) & 0xfff;

		/* Now we can alter the IPT address counter */
		if (!BIT(m_pl2, 4))
			m_ipt_cnt = y_bus & 0x1fff;
		else if (BIT(m_pl2, 5))
			m_ipt_cnt = (m_ipt_cnt + 1) & 0x1fff;

		if (calldebugger)
			debugger_instruction_hook(this, RIP_PC);

		m_pc = next_pc;
		m_rip_pc = (m_pc | ((m_status_out & 1) << 8));

		m_icount--;
	} while (m_icount > 0);
}
