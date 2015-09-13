// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    m6809inl.h

    Portable 6809 emulator - Inline functions for the purposes of
    optimization

**********************************************************************/

#include "m6809.h"

//-------------------------------------------------
//  rotate_right
//-------------------------------------------------

template<class T>
inline ATTR_FORCE_INLINE T m6809_base_device::rotate_right(T value)
{
	bool new_carry = (value & 1) ? true : false;
	value = value >> 1;

	T high_bit = ((T) 1) << (sizeof(T) * 8 - 1);
	if (m_cc & CC_C)
		value |= high_bit;
	else
		value &= ~high_bit;

	if (new_carry)
		m_cc |= CC_C;
	else
		m_cc &= ~CC_C;
	return value;
}


//-------------------------------------------------
//  rotate_left
//-------------------------------------------------

template<class T>
inline ATTR_FORCE_INLINE UINT32 m6809_base_device::rotate_left(T value)
{
	T high_bit = ((T) 1) << (sizeof(T) * 8 - 1);
	bool new_carry = (value & high_bit) ? true : false;

	UINT32 new_value = value;
	new_value <<= 1;

	if (m_cc & CC_C)
		new_value |= 1;
	else
		new_value &= ~1;

	if (new_carry)
		m_cc |= CC_C;
	else
		m_cc &= ~CC_C;
	return new_value;
}


//-------------------------------------------------
//  read_operand
//-------------------------------------------------

inline ATTR_FORCE_INLINE UINT8 m6809_base_device::read_operand()
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_EA:            return read_memory(m_ea.w);
		case ADDRESSING_MODE_IMMEDIATE:     return read_opcode_arg();
		case ADDRESSING_MODE_REGISTER_A:    return m_d.b.h;
		case ADDRESSING_MODE_REGISTER_B:    return m_d.b.l;
		default:                            fatalerror("Unexpected");   return 0x00;
	}
}


//-------------------------------------------------
//  read_operand
//-------------------------------------------------

inline ATTR_FORCE_INLINE UINT8 m6809_base_device::read_operand(int ordinal)
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_EA:            return read_memory(m_ea.w + ordinal);
		case ADDRESSING_MODE_IMMEDIATE:     return read_opcode_arg();
		default:                            fatalerror("Unexpected");   return 0x00;
	}
}


//-------------------------------------------------
//  write_operand
//-------------------------------------------------

inline ATTR_FORCE_INLINE void m6809_base_device::write_operand(UINT8 data)
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_IMMEDIATE:     /* do nothing */                break;
		case ADDRESSING_MODE_EA:            write_memory(m_ea.w, data);     break;
		case ADDRESSING_MODE_REGISTER_A:    m_d.b.h = data;                 break;
		case ADDRESSING_MODE_REGISTER_B:    m_d.b.l = data;                 break;
		default:                            fatalerror("Unexpected");       break;
	}
}


//-------------------------------------------------
//  write_operand
//-------------------------------------------------

inline ATTR_FORCE_INLINE void m6809_base_device::write_operand(int ordinal, UINT8 data)
{
	switch(m_addressing_mode)
	{
		case ADDRESSING_MODE_IMMEDIATE:     /* do nothing */                break;
		case ADDRESSING_MODE_EA:            write_memory(m_ea.w + ordinal, data);   break;
		default:                            fatalerror("Unexpected");       break;
	}
}


//-------------------------------------------------
//  daa - decimal arithmetic adjustment instruction
//-------------------------------------------------

inline ATTR_FORCE_INLINE void m6809_base_device::daa()
{
	UINT16 t, cf = 0;
	UINT8 msn = m_d.b.h & 0xF0;
	UINT8 lsn = m_d.b.h & 0x0F;

	// compute the carry
	if (lsn > 0x09 || m_cc & CC_H)  cf |= 0x06;
	if (msn > 0x80 && lsn > 0x09 )  cf |= 0x60;
	if (msn > 0x90 || m_cc & CC_C)  cf |= 0x60;

	// calculate the result
	t = m_d.b.h + cf;

	m_cc &= ~CC_V;
	if (t & 0x0100)     // keep carry from previous operation
		m_cc |= CC_C;

	// and put it back into A
	m_d.b.h = set_flags(CC_NZ, (UINT8) t);
}


//-------------------------------------------------
//  mul
//-------------------------------------------------

inline ATTR_FORCE_INLINE void m6809_base_device::mul()
{
	// perform multiply
	UINT16 result = ((UINT16) m_d.b.h) * ((UINT16) m_d.b.l);

	// set result and Z flag
	m_d.w = set_flags(CC_Z, result);

	// set C flag
	if (m_d.w & 0x0080)
		m_cc |= CC_C;
	else
		m_cc &= ~CC_C;
}


//-------------------------------------------------
//  ireg
//-------------------------------------------------

inline ATTR_FORCE_INLINE UINT16 &m6809_base_device::ireg()
{
	switch(m_opcode & 0x60)
	{
		case 0x00:  return m_x.w;
		case 0x20:  return m_y.w;
		case 0x40:  return m_u.w;
		case 0x60:  return m_s.w;
		default:
			fatalerror("Unexpected");
			return m_x.w;
	}
}


//-------------------------------------------------
//  set_flags
//-------------------------------------------------

template<class T>
inline T m6809_base_device::set_flags(UINT8 mask, T a, T b, UINT32 r)
{
	T hi_bit = (T) (1 << (sizeof(T) * 8 - 1));

	m_cc &= ~mask;
	if (mask & CC_H)
		m_cc |= ((a ^ b ^ r) & 0x10) ? CC_H : 0;
	if (mask & CC_N)
		m_cc |= (r & hi_bit) ? CC_N : 0;
	if (mask & CC_Z)
		m_cc |= (((T)r) == 0) ? CC_Z : 0;
	if (mask & CC_V)
		m_cc |= ((a ^ b ^ r ^ (r >> 1)) & hi_bit) ? CC_V : 0;
	if (mask & CC_C)
		m_cc |= (r & (hi_bit << 1)) ? CC_C : 0;
	return (T) r;
}


//-------------------------------------------------
//  set_flags
//-------------------------------------------------

template<class T>
inline T m6809_base_device::set_flags(UINT8 mask, T r)
{
	return set_flags(mask, (T)0, r, r);
}


//-------------------------------------------------
//  eat_remaining
//-------------------------------------------------

inline void m6809_base_device::eat_remaining()
{
	// we do this in order to be nice to people debugging
	UINT16 real_pc = m_pc.w;

	eat(m_icount);

	m_pc.w = m_ppc.w;
	debugger_instruction_hook(this, m_pc.w);
	m_pc.w = real_pc;
}



//-------------------------------------------------
//  is_register_addressing_mode
//-------------------------------------------------

inline bool m6809_base_device::is_register_addressing_mode()
{
	return (m_addressing_mode != ADDRESSING_MODE_IMMEDIATE)
		&& (m_addressing_mode != ADDRESSING_MODE_EA);
}



//-------------------------------------------------
//  get_pending_interrupt
//-------------------------------------------------

inline UINT16 m6809_base_device::get_pending_interrupt()
{
	if (m_nmi_asserted)
		return VECTOR_NMI;
	else if (!(m_cc & CC_F) && m_firq_line)
		return VECTOR_FIRQ;
	else if (!(m_cc & CC_I) && m_irq_line)
		return VECTOR_IRQ;
	else
		return 0;
}
