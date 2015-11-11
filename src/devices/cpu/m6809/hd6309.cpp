// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Tim Lindner
/*********************************************************************

    hd6309.c

    Copyright John Butler
    Copyright Tim Lindner

    References:

        HD63B09EP Technical Reference Guide, by Chet Simpson with addition
                            by Alan Dekok
        6809 Simulator V09, By L.C. Benschop, Eindhoven The Netherlands.

        m6809: Portable 6809 emulator, DS (6809 code in MAME, derived from
            the 6809 Simulator V09)

        6809 Microcomputer Programming & Interfacing with Experiments"
            by Andrew C. Staugaard, Jr.; Howard W. Sams & Co., Inc.

    System dependencies:    UINT16 must be 16 bit unsigned int
                            UINT8 must be 8 bit unsigned int
                            UINT32 must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

    History:

March 2013 NPW:
    Rewrite of 6809/6309/Konami CPU; attempted to make cycle exact.

070614 ZV:
    Fixed N flag setting in DIV overflow

991026 HJB:
    Fixed missing calls to cpu_changepc() for the TFR and EXG ocpodes.
    Replaced m6809_slapstic checks by a macro (CHANGE_PC). ESB still
    needs the tweaks.

991024 HJB:
    Tried to improve speed: Using bit7 of cycles1/2 as flag for multi
    byte opcodes is gone, those opcodes now call fetch_effective_address().
    Got rid of the slow/fast flags for stack (S and U) memory accesses.
    Minor changes to use 32 bit values as arguments to memory functions
    and added defines for that purpose (e.g. X = 16bit XD = 32bit).

990312 HJB:
    Added bugfixes according to Aaron's findings.
    Reset only sets CC_II and CC_IF, DP to zero and PC from reset vector.
990311 HJB:
    Added _info functions. Now uses static m6808_Regs struct instead
    of single statics. Changed the 16 bit registers to use the generic
    PAIR union. Registers defined using macros. Split the core into
    four execution loops for M6802, M6803, M6808 and HD63701.
    TST, TSTA and TSTB opcodes reset carry flag.
    Modified the read/write stack handlers to push LSB first then MSB
    and pull MSB first then LSB.

990228 HJB:
    Changed the interrupt handling again. Now interrupts are taken
    either right at the moment the lines are asserted or whenever
    an interrupt is enabled and the corresponding line is still
    asserted. That way the pending_interrupts checks are not
    needed anymore. However, the CWAI and SYNC flags still need
    some flags, so I changed the name to 'int_state'.
    This core also has the code for the old interrupt system removed.

990225 HJB:
    Cleaned up the code here and there, added some comments.
    Slightly changed the SAR opcodes (similiar to other CPU cores).
    Added symbolic names for the flag bits.
    Changed the way CWAI/Interrupt() handle CPU state saving.
    A new flag M6809_STATE in pending_interrupts is used to determine
    if a state save is needed on interrupt entry or already done by CWAI.
    Added M6809_IRQ_LINE and M6809_FIRQ_LINE defines to m6809.h
    Moved the internal interrupt_pending flags from m6809.h to m6809.c
    Changed CWAI cycles2[0x3c] to be 2 (plus all or at least 19 if
    CWAI actually pushes the entire state).
    Implemented undocumented TFR/EXG for undefined source and mixed 8/16
    bit transfers (they should transfer/exchange the constant $ff).
    Removed unused jmp/jsr _slap functions from 6809ops.c,
    m6809_slapstick check moved into the opcode functions.

000809 TJL:
    Started converting m6809 into hd6309

001217 TJL:
    Finished:
        All opcodes
        Dual Timing
    To Do:
        Verify new DIV opcodes.

070805 TJL:
    Fixed ADDR and ADCR opcodes not to clear the H condition code. Fixed ANDR,
    EORR, ORR, ADDR, ADCR, SBCR, and SUBR to evaluate condition codes after
    the destination register was set. Fixed BITMD opcode to only effect the Z
    condition code. Fixed BITMD opcode to clear only tested flags. Fixed EXG
    and TFR register promotion and demotion. Fixed illegal instruction handler
    to not set I and F condition codes. Credit to Darren Atkinson for the
    discovery of these bugs.

090907 TJL:
    The SEXW instruction is clearing the Overflow flag (V). It should not do
    that. When an invalid source or destination register is specified for
    the TFM instructions, real hardware invokes the Illegal Instruction
    trap, whereas the emulator simply ignores the instruction. Credit to
    Darren Atkinson for the discovery of these bugs.

*****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "hd6309.h"
#include "m6809inl.h"


//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

const device_type HD6309 = &device_creator<hd6309_device>;


//-------------------------------------------------
//  hd6309_device - constructor
//-------------------------------------------------

hd6309_device::hd6309_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m6809_base_device(mconfig, "HD6309", tag, owner, clock, HD6309, 4, "hd6309", __FILE__), 
	m_md(0), 
	m_temp_im(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd6309_device::device_start()
{
	super::device_start();

	// register our state for the debugger
	state_add(HD6309_E,         "E",            m_w.b.h).mask(0xff);
	state_add(HD6309_F,         "F",            m_w.b.l).mask(0xff);
	state_add(HD6309_W,         "W",            m_w.w).mask(0xffff);
	state_add(HD6309_V,         "V",            m_v.w).mask(0xffff);
	state_add(HD6309_MD,        "MD",           m_md).mask(0xff);

	// initialize variables
	m_w.w = 0x0000;
	m_v.w = 0x0000;
	m_md = 0x00;
	m_temp_im = 0x00;

	// setup regtable
	save_item(NAME(m_w.w));
	save_item(NAME(m_v.w));
	save_item(NAME(m_md));
	save_item(NAME(m_temp_im));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hd6309_device::device_reset()
{
	super::device_reset();

	// initialize variables
	m_md = 0x00;
}


//-------------------------------------------------
//  device_pre_save - device-specific pre-save
//-------------------------------------------------

void hd6309_device::device_pre_save()
{
	if      (m_reg8 == &m_d.b.h)    m_reg = HD6309_A;
	else if (m_reg8 == &m_d.b.l)    m_reg = HD6309_B;
	else if (m_reg8 == &m_w.b.h)    m_reg = HD6309_E;
	else if (m_reg8 == &m_w.b.l)    m_reg = HD6309_F;
	else if (m_reg8 == &m_cc)       m_reg = HD6309_CC;
	else if (m_reg8 == &m_dp)       m_reg = HD6309_DP;
	else if (m_reg8 == &m_md)       m_reg = HD6309_MD;
	else if (m_reg8 == &m_temp.b.l) m_reg = HD6309_ZERO_BYTE;

	else if (m_reg16 == &m_d)       m_reg = HD6309_D;
	else if (m_reg16 == &m_x)       m_reg = HD6309_X;
	else if (m_reg16 == &m_y)       m_reg = HD6309_Y;
	else if (m_reg16 == &m_u)       m_reg = HD6309_U;
	else if (m_reg16 == &m_s)       m_reg = HD6309_S;
	else if (m_reg16 == &m_pc)      m_reg = HD6309_PC;
	else if (m_reg16 == &m_w)       m_reg = HD6309_W;
	else if (m_reg16 == &m_v)       m_reg = HD6309_V;
	else if (m_reg16 == &m_temp)    m_reg = HD6309_ZERO_WORD;
	else
		m_reg = 0;
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void hd6309_device::device_post_load()
{
	m_reg8 = NULL;
	m_reg16 = NULL;

	switch(m_reg)
	{
		case HD6309_A:
			set_regop8(m_d.b.h);
			break;
		case HD6309_B:
			set_regop8(m_d.b.l);
			break;
		case HD6309_E:
			set_regop8(m_w.b.h);
			break;
		case HD6309_F:
			set_regop8(m_w.b.l);
			break;
		case HD6309_CC:
			set_regop8(m_cc);
			break;
		case HD6309_DP:
			set_regop8(m_dp);
			break;
		case HD6309_MD:
			set_regop8(m_md);
			break;
		case HD6309_ZERO_BYTE:
			set_regop8(m_temp.b.l);
			break;

		case HD6309_D:
			set_regop16(m_d);
			break;
		case HD6309_X:
			set_regop16(m_x);
			break;
		case HD6309_Y:
			set_regop16(m_y);
			break;
		case HD6309_U:
			set_regop16(m_u);
			break;
		case HD6309_S:
			set_regop16(m_s);
			break;
		case HD6309_PC:
			set_regop16(m_pc);
			break;
		case HD6309_W:
			set_regop16(m_w);
			break;
		case HD6309_V:
			set_regop16(m_v);
			break;
		case HD6309_ZERO_WORD:
			set_regop16(m_temp);
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 hd6309_device::disasm_min_opcode_bytes() const
{
	return 1;
}



//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 hd6309_device::disasm_max_opcode_bytes() const
{
	return 5;
}



//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t hd6309_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( hd6309 );
	return CPU_DISASSEMBLE_NAME(hd6309)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  read_operand
//-------------------------------------------------

inline UINT8 hd6309_device::read_operand()
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_EA:            return read_memory(m_ea.w);
		case ADDRESSING_MODE_IMMEDIATE:     return read_opcode_arg();
		case ADDRESSING_MODE_REGISTER_A:    return m_d.b.h;
		case ADDRESSING_MODE_REGISTER_B:    return m_d.b.l;
		case ADDRESSING_MODE_REGISTER_E:    return m_w.b.h;
		case ADDRESSING_MODE_REGISTER_F:    return m_w.b.l;
		default:                            fatalerror("Unexpected");
	}
}


//-------------------------------------------------
//  read_operand
//-------------------------------------------------

inline UINT8 hd6309_device::read_operand(int ordinal)
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_EA:            return read_memory(m_ea.w + ordinal);
		case ADDRESSING_MODE_IMMEDIATE:     return read_opcode_arg();
		case ADDRESSING_MODE_REGISTER_D:    return (ordinal & 1) ? m_d.b.l : m_d.b.h;
		case ADDRESSING_MODE_REGISTER_W:    return (ordinal & 1) ? m_w.b.l : m_w.b.h;
		case ADDRESSING_MODE_REGISTER_X:    return (ordinal & 1) ? m_x.b.l : m_x.b.h;
		case ADDRESSING_MODE_REGISTER_Y:    return (ordinal & 1) ? m_y.b.l : m_y.b.h;
		case ADDRESSING_MODE_REGISTER_U:    return (ordinal & 1) ? m_u.b.l : m_u.b.h;
		case ADDRESSING_MODE_REGISTER_S:    return (ordinal & 1) ? m_s.b.l : m_s.b.h;
		case ADDRESSING_MODE_REGISTER_V:    return (ordinal & 1) ? m_v.b.l : m_v.b.h;
		case ADDRESSING_MODE_REGISTER_PC:   return (ordinal & 1) ? m_pc.b.l : m_pc.b.h;
		case ADDRESSING_MODE_ZERO:          return 0x00;
		default:                            fatalerror("Unexpected");
	}
}


//-------------------------------------------------
//  write_operand
//-------------------------------------------------

inline void hd6309_device::write_operand(UINT8 data)
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_EA:            write_memory(m_ea.w, data);     break;
		case ADDRESSING_MODE_REGISTER_A:    m_d.b.h = data;                 break;
		case ADDRESSING_MODE_REGISTER_B:    m_d.b.l = data;                 break;
		case ADDRESSING_MODE_REGISTER_E:    m_w.b.h = data;                 break;
		case ADDRESSING_MODE_REGISTER_F:    m_w.b.l = data;                 break;
		case ADDRESSING_MODE_ZERO:                                          break;
		default:                            fatalerror("Unexpected");
	}
}


//-------------------------------------------------
//  write_operand
//-------------------------------------------------

inline void hd6309_device::write_operand(int ordinal, UINT8 data)
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_EA:            write_memory(m_ea.w + ordinal, data);               break;
		case ADDRESSING_MODE_REGISTER_D:    *((ordinal & 1) ? &m_d.b.l : &m_d.b.h) = data;      break;
		case ADDRESSING_MODE_REGISTER_W:    *((ordinal & 1) ? &m_w.b.l : &m_w.b.h) = data;      break;
		case ADDRESSING_MODE_REGISTER_X:    *((ordinal & 1) ? &m_x.b.l : &m_x.b.h) = data;      break;
		case ADDRESSING_MODE_REGISTER_Y:    *((ordinal & 1) ? &m_y.b.l : &m_y.b.h) = data;      break;
		case ADDRESSING_MODE_REGISTER_U:    *((ordinal & 1) ? &m_u.b.l : &m_u.b.h) = data;      break;
		case ADDRESSING_MODE_REGISTER_S:    *((ordinal & 1) ? &m_s.b.l : &m_s.b.h) = data;      break;
		case ADDRESSING_MODE_REGISTER_V:    *((ordinal & 1) ? &m_v.b.l : &m_v.b.h) = data;      break;
		case ADDRESSING_MODE_REGISTER_PC:   *((ordinal & 1) ? &m_pc.b.l : &m_pc.b.h) = data;    break;
		case ADDRESSING_MODE_ZERO:                                                              break;
		default:                            fatalerror("Unexpected");
	}
}


//-------------------------------------------------
//  bittest_register
//-------------------------------------------------

inline UINT8 &hd6309_device::bittest_register()
{
	switch(m_temp_im & 0xC0)
	{
		case 0x00:  return m_cc;
		case 0x40:  return m_d.b.h;
		case 0x80:  return m_d.b.l;
		default:    return m_temp.b.l;
	}
}


//-------------------------------------------------
//  bittest_source
//-------------------------------------------------

inline bool hd6309_device::bittest_source()
{
	return (m_temp.b.l & (1 << ((m_temp_im >> 3) & 0x07))) ? true : false;
}


//-------------------------------------------------
//  bittest_dest
//-------------------------------------------------

inline bool hd6309_device::bittest_dest()
{
	return (bittest_register() & (1 << ((m_temp_im >> 0) & 0x07))) ? true : false;
}


//-------------------------------------------------
//  bittest_set
//-------------------------------------------------

inline void hd6309_device::bittest_set(bool result)
{
	if (result)
		bittest_register() |= (1 << ((m_temp_im >> 0) & 0x07));
	else
		bittest_register() &= ~(1 << ((m_temp_im >> 0) & 0x07));
	eat(4);
}


//-------------------------------------------------
//  read_exgtfr_register
//-------------------------------------------------

inline m6809_base_device::exgtfr_register hd6309_device::read_exgtfr_register(UINT8 reg)
{
	UINT16 value;

	switch(reg & 0x0F)
	{
		case  0: value = m_d.w;                             break;  // D
		case  1: value = m_x.w;                             break;  // X
		case  2: value = m_y.w;                             break;  // Y
		case  3: value = m_u.w;                             break;  // U
		case  4: value = m_s.w;                             break;  // S
		case  5: value = m_pc.w;                            break;  // PC
		case  6: value = m_w.w;                             break;  // W
		case  7: value = m_v.w;                             break;  // V
		case  8: value = ((UINT16) m_d.b.h) << 8 | m_d.b.h; break;  // A
		case  9: value = ((UINT16) m_d.b.l) << 8 | m_d.b.l; break;  // B
		case 10: value = ((UINT16) m_cc) << 8 | m_cc;       break;  // CC
		case 11: value = ((UINT16) m_dp) << 8 | m_dp;       break;  // DP
		case 12: value = 0;                                 break;  // 0
		case 13: value = 0;                                 break;  // 0
		case 14: value = ((UINT16) m_w.b.h) << 8 | m_w.b.h; break;  // E
		case 15: value = ((UINT16) m_w.b.l) << 8 | m_w.b.l; break;  // F
		default:
			fatalerror("Should not reach here");
	}

	exgtfr_register result;
	result.byte_value = (UINT8)value;
	result.word_value = value;
	return result;
}



//-------------------------------------------------
//  write_exgtfr_register
//-------------------------------------------------

inline void hd6309_device::write_exgtfr_register(UINT8 reg, m6809_base_device::exgtfr_register value)
{
	switch(reg & 0x0F)
	{
		case  0: m_d.w   = value.word_value;                break;  // D
		case  1: m_x.w   = value.word_value;                break;  // X
		case  2: m_y.w   = value.word_value;                break;  // Y
		case  3: m_u.w   = value.word_value;                break;  // U
		case  4: m_s.w   = value.word_value;                break;  // S
		case  5: m_pc.w  = value.word_value;                break;  // PC
		case  6: m_w.w   = value.word_value;                break;  // W
		case  7: m_v.w   = value.word_value;                break;  // V
		case  8: m_d.b.h = (UINT8) (value.word_value >> 8); break;  // A
		case  9: m_d.b.l = (UINT8) (value.word_value >> 0); break;  // B
		case 10: m_cc    = (UINT8) (value.word_value >> 0); break;  // CC
		case 11: m_dp    = (UINT8) (value.word_value >> 8); break;  // DP
		case 12:                                            break;  // 0
		case 13:                                            break;  // 0
		case 14: m_w.b.h = (UINT8) (value.word_value >> 8); break;  // E
		case 15: m_w.b.l = (UINT8) (value.word_value >> 0); break;  // F
		default:
			fatalerror("Should not reach here");
	}
}



//-------------------------------------------------
//  tfr_read
//-------------------------------------------------

inline bool hd6309_device::tfr_read(UINT8 opcode, UINT8 arg, UINT8 &data)
{
	PAIR16 *reg;

	switch(arg & 0xF0)
	{
		case 0x00:      reg = &m_d; break;
		case 0x10:      reg = &m_x; break;
		case 0x20:      reg = &m_y; break;
		case 0x30:      reg = &m_u; break;
		case 0x40:      reg = &m_s; break;
		default:        return false;
	}

	data = read_memory(reg->w);

	switch(opcode & 0x03)
	{
		case 0x00:  reg->w++;   break;  // TFM R0+,R1+
		case 0x01:  reg->w--;   break;  // TFM R0-,R1-
		case 0x02:  reg->w++;   break;  // TFM R0+,R1
		case 0x03:              break;  // TFM R0,R1+
	}

	return true;
}



//-------------------------------------------------
//  tfr_write
//-------------------------------------------------

inline bool hd6309_device::tfr_write(UINT8 opcode, UINT8 arg, UINT8 data)
{
	PAIR16 *reg;

	switch(arg & 0x0F)
	{
		case 0x00:      reg = &m_d; break;
		case 0x01:      reg = &m_x; break;
		case 0x02:      reg = &m_y; break;
		case 0x03:      reg = &m_u; break;
		case 0x04:      reg = &m_s; break;
		default:        return false;
	}

	write_memory(reg->w, data);

	switch(opcode & 0x03)
	{
		case 0x00:  reg->w++;   break;  // TFM R0+,R1+
		case 0x01:  reg->w--;   break;  // TFM R0-,R1-
		case 0x02:              break;  // TFM R0+,R1
		case 0x03:  reg->w++;   break;  // TFM R0,R1+
	}

	return true;
}



//-------------------------------------------------
//  register_register_op
//-------------------------------------------------

void hd6309_device::register_register_op()
{
	UINT8 operand = read_opcode_arg();

	// if the 8/16 bit values are mismatched, we need to promote
	bool promote = ((operand & 0x80) ? true : false) != ((operand & 0x08) ? true : false);

	// we're using m_temp as "register 0"
	m_temp.w = 0;

	// set destination
	switch((operand >> 0) & 0x0F)
	{
		case  0: set_regop16(m_d);                                                  break;  // D
		case  1: set_regop16(m_x);                                                  break;  // X
		case  2: set_regop16(m_y);                                                  break;  // Y
		case  3: set_regop16(m_u);                                                  break;  // U
		case  4: set_regop16(m_s);                                                  break;  // S
		case  5: set_regop16(m_pc);                                                 break;  // PC
		case  6: set_regop16(m_w);                                                  break;  // W
		case  7: set_regop16(m_v);                                                  break;  // V
		case  8: if (promote) set_regop16(m_d);     else set_regop8(m_d.b.h);       break;  // A
		case  9: if (promote) set_regop16(m_d);     else set_regop8(m_d.b.l);       break;  // B
		case 10: if (promote) set_regop16(m_temp);  else set_regop8(m_cc);          break;  // CC
		case 11: if (promote) set_regop16(m_temp);  else set_regop8(m_dp);          break;  // DP
		case 12: if (promote) set_regop16(m_temp);  else set_regop8(m_temp.b.l);    break;  // 0
		case 13: if (promote) set_regop16(m_temp);  else set_regop8(m_temp.b.l);    break;  // 0
		case 14: if (promote) set_regop16(m_w);     else set_regop8(m_w.b.h);       break;  // E
		case 15: if (promote) set_regop16(m_w);     else set_regop8(m_w.b.l);       break;  // F
		default:
			fatalerror("Should not reach here");
	}

	// set source
	switch((operand >> 4) & 0x0F)
	{
		case  0: m_addressing_mode = ADDRESSING_MODE_REGISTER_D;                                        break;  // D
		case  1: m_addressing_mode = ADDRESSING_MODE_REGISTER_X;                                        break;  // X
		case  2: m_addressing_mode = ADDRESSING_MODE_REGISTER_Y;                                        break;  // Y
		case  3: m_addressing_mode = ADDRESSING_MODE_REGISTER_U;                                        break;  // U
		case  4: m_addressing_mode = ADDRESSING_MODE_REGISTER_S;                                        break;  // S
		case  5: m_addressing_mode = ADDRESSING_MODE_REGISTER_PC;                                       break;  // PC
		case  6: m_addressing_mode = ADDRESSING_MODE_REGISTER_W;                                        break;  // W
		case  7: m_addressing_mode = ADDRESSING_MODE_REGISTER_V;                                        break;  // V
		case  8: m_addressing_mode = promote ? ADDRESSING_MODE_REGISTER_D : ADDRESSING_MODE_REGISTER_A; break;  // A
		case  9: m_addressing_mode = promote ? ADDRESSING_MODE_REGISTER_D : ADDRESSING_MODE_REGISTER_B; break;  // B
		case 10: m_addressing_mode = promote ? ADDRESSING_MODE_ZERO : ADDRESSING_MODE_REGISTER_CC;      break;  // CC
		case 11: m_addressing_mode = promote ? ADDRESSING_MODE_ZERO : ADDRESSING_MODE_REGISTER_DP;      break;  // DP
		case 12: m_addressing_mode = ADDRESSING_MODE_ZERO;                                              break;  // 0
		case 13: m_addressing_mode = ADDRESSING_MODE_ZERO;                                              break;  // 0
		case 14: m_addressing_mode = promote ? ADDRESSING_MODE_REGISTER_W : ADDRESSING_MODE_REGISTER_E; break;  // E
		case 15: m_addressing_mode = promote ? ADDRESSING_MODE_REGISTER_W : ADDRESSING_MODE_REGISTER_F; break;  // F
		default:
			fatalerror("Should not reach here");
	}

	// eat a single CPU cycle
	eat(1);
}


//-------------------------------------------------
//  get_q
//-------------------------------------------------

UINT32 hd6309_device::get_q()
{
	PAIR result;
	result.w.h = m_d.w;
	result.w.l = m_w.w;
	return result.d;
}


//-------------------------------------------------
//  put_q
//-------------------------------------------------

void hd6309_device::put_q(UINT32 value)
{
	PAIR pair;
	pair.d = value;
	m_d.w = pair.w.h;
	m_w.w = pair.w.l;
}


//-------------------------------------------------
//  muld - (Q := D * operand)
//-------------------------------------------------

void hd6309_device::muld()
{
	UINT32 result;
	result = ((INT16) m_d.w) * ((INT16) m_temp.w);
	put_q(set_flags<UINT32>(CC_NZ, result));
	m_cc &= ~CC_VC;
}


//-------------------------------------------------
//  divq - (D := Q / operand; W := Q % operand)
//-------------------------------------------------

bool hd6309_device::divq()
{
	INT32 result;

	// check for divide by zero
	if (m_temp.w == 0)
		return false;

	INT32 q = get_q();
	INT32 old_q = q;

	// do the divide/modulo
	result = q / (INT16) m_temp.w;
	m_d.w = q % (INT16) m_temp.w;

	// set NZ condition codes
	m_w.w = set_flags<UINT16>(CC_NZ, result);

	// set C condition code
	if (m_w.w & 0x0001)
		m_cc |= CC_C;
	else
		m_cc &= ~CC_C;

	if ((result > 32768) || (result < -32767))
	{
		// soft overflow
		m_cc |= CC_V;

		if  ((result > 65536 ) || (result < -65535 ))
		{
			// hard overflow - division is aborted
			if (old_q < 0)
				m_cc |= CC_N;
			else if (old_q == 0 )
				m_cc |= CC_Z;

			put_q(old_q);
		}
	}
	else
	{
		// no overflow
		m_cc &= ~CC_V;
	}

	return true;
}


//-------------------------------------------------
//  divd - (D := D / operand; W := D % operand)
//-------------------------------------------------

bool hd6309_device::divd()
{
	// check for divide by zero
	if (m_temp.b.l == 0)
		return false;

	INT16 old_d = m_d.w;
	INT16 result;

	// do the divide/modulo
	result = ((INT16) m_d.w) / (INT8) m_temp.b.l;
	m_d.b.h = ((INT16) m_d.w) % (INT8) m_temp.b.l;

	// set NZ condition codes
	m_d.b.l = set_flags<UINT8>(CC_NZ, result);

	// set C condition code
	if (m_d.b.l & 0x01)
		m_cc |= CC_C;
	else
		m_cc &= ~CC_C;

	if ((result > 128) || (result < -127))
	{
		// soft overflow
		m_cc |= CC_V;

		if ((result > 256 ) || (result < -255 ))
		{
			// hard overflow - division is aborted
			set_flags<UINT16>(CC_NZ, old_d);
			m_d.w = abs(old_d);
		}
	}
	else
	{
		// no overflow
		m_cc &= ~CC_V;
	}

	return true;
}


//-------------------------------------------------
//  execute_one - try to execute a single instruction
//-------------------------------------------------

inline void hd6309_device::execute_one()
{
	switch(pop_state())
	{
#include "cpu/m6809/hd6309.inc"
	}
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void hd6309_device::execute_run()
{
	do
	{
		execute_one();
	} while(m_icount > 0);
}
