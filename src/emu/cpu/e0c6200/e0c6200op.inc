// E0C6200 opcode handlers

enum
{
	FLAG_C = 1,
	FLAG_Z = 2,
	FLAG_D = 4,
	FLAG_I = 8
};


// internal data memory read/write

// MX/MY

inline UINT8 e0c6200_cpu_device::read_mx()
{
	UINT16 address = m_xp << 8 | m_xh << 4 | m_xl;
	return m_data->read_byte(address) & 0xf;
}

inline UINT8 e0c6200_cpu_device::read_my()
{
	UINT16 address = m_yp << 8 | m_yh << 4 | m_yl;
	return m_data->read_byte(address) & 0xf;
}

inline void e0c6200_cpu_device::write_mx(UINT8 data)
{
	UINT16 address = m_xp << 8 | m_xh << 4 | m_xl;
	m_data->write_byte(address, data);
}

inline void e0c6200_cpu_device::write_my(UINT8 data)
{
	UINT16 address = m_yp << 8 | m_yh << 4 | m_yl;
	m_data->write_byte(address, data);
}

// Mn(RP)

inline UINT8 e0c6200_cpu_device::read_mn()
{
	return m_data->read_byte(m_op & 0xf) & 0xf;
}

inline void e0c6200_cpu_device::write_mn(UINT8 data)
{
	m_data->write_byte(m_op & 0xf, data);
}


// common stack ops

inline void e0c6200_cpu_device::push(UINT8 data)
{
	m_data->write_byte(--m_sp, data);
}

inline UINT8 e0c6200_cpu_device::pop()
{
	return m_data->read_byte(m_sp++) & 0xf;
}

inline void e0c6200_cpu_device::push_pc()
{
	// the highest bit(bank bit) is not pushed onto the stack
	push(m_pc >> 8 & 0xf);
	push(m_pc >> 4 & 0xf);
	push(m_pc & 0xf);
}

inline void e0c6200_cpu_device::pop_pc()
{
	// the highest bit(bank bit) is unchanged
	UINT16 bank = m_pc & 0x1000;
	m_pc = pop();
	m_pc |= pop() << 4;
	m_pc |= pop() << 8;
	m_pc |= bank;
}


// misc internal helpers

inline void e0c6200_cpu_device::set_cf(UINT8 data)
{
	// set carry flag if bit 4 is set, reset otherwise
	m_f = (m_f & ~FLAG_C) | ((data & 0x10) ? FLAG_C : 0);
}

inline void e0c6200_cpu_device::set_zf(UINT8 data)
{
	// set zero flag if 4-bit data is 0, reset otherwise
	m_f = (m_f & ~FLAG_Z) | ((data & 0xf) ? 0 : FLAG_Z);
}

inline void e0c6200_cpu_device::inc_x()
{
	// increment X (XH.XL)
	m_xl++;
	m_xh = (m_xh + (m_xl >> 4 & 1)) & 0xf;
	m_xl &= 0xf;
}

inline void e0c6200_cpu_device::inc_y()
{
	// increment Y (YH.YL)
	m_yl++;
	m_yh = (m_yh + (m_yl >> 4 & 1)) & 0xf;
	m_yl &= 0xf;
}


// common opcodes
// note: it is implied that all handled opcodes below take 7 clock cycles (5 already deducted)

// artithmetic instructions

UINT8 e0c6200_cpu_device::op_inc(UINT8 x)
{
	// INC x: increment x (affect flags: C, Z)
	m_icount -= 2;
	x++;
	set_cf(x); set_zf(x);
	return x & 0xf;
}

UINT8 e0c6200_cpu_device::op_dec(UINT8 x)
{
	// DEC x: decrement x (affect flags: C, Z)
	m_icount -= 2;
	x--;
	set_cf(x); set_zf(x);
	return x & 0xf;
}


// logical instructions

UINT8 e0c6200_cpu_device::op_and(UINT8 x, UINT8 y)
{
	// AND x,y: logical AND x with y (affect flags: Z)
	m_icount -= 2;
	x &= y;
	set_zf(x);
	return x;
}

UINT8 e0c6200_cpu_device::op_or(UINT8 x, UINT8 y)
{
	// OR x,y: logical OR x with y (affect flags: Z)
	m_icount -= 2;
	x |= y;
	set_zf(x);
	return x;
}

UINT8 e0c6200_cpu_device::op_xor(UINT8 x, UINT8 y)
{
	// XOR x,y: exclusive-OR x with y (affect flags: Z)
	m_icount -= 2;
	x ^= y;
	set_zf(x);
	return x;
}
