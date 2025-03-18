// license:BSD-3-Clause
// copyright-holders:David Haywood

/*

Holtek HT1130 MCU family

TODO:
- Interrupts (not used by brke23p2)
- Sound (needs internal frequency ROM data?)
- 1 machine cycle (eg. a 1 byte opcode) takes 4 system clock cycles (from OSC pins).
- The timer rate can be configured with a mask option (system clock / 2^n), n=0-13 (except 6 for some reason).
  So, timer rate can be faster or slower than machine cycle rate.

*/

#include "emu.h"
#include "ht1130.h"
#include "ht1130d.h"

#define LOG_UNHANDLED_OPS       (1U << 1)
#define LOG_UNHANDLED_SOUND_OPS (1U << 2)

#define VERBOSE (LOG_UNHANDLED_OPS)
#include "logmacro.h"

// device type definitions
DEFINE_DEVICE_TYPE(HT1130, ht1130_device, "ht1130", "Holtek HT1130")
DEFINE_DEVICE_TYPE(HT1190, ht1190_device, "ht1190", "Holtek HT1190")



ht1130_device::ht1130_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor data)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_tempram(*this, "tempram")
	, m_displayram(*this, "displayram")
	, m_space_config("program", ENDIANNESS_LITTLE, 8, 12, 0, address_map_constructor(FUNC(ht1130_device::internal_map), this))
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 8, 0, data)
	, m_port_in_pm(*this, 0xff)
	, m_port_in_ps(*this, 0xff)
	, m_port_in_pp(*this, 0xff)
	, m_port_out_pa(*this)
	, m_segment_out(*this)
{
}

ht1130_device::ht1130_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ht1130_device(mconfig, HT1130, tag, owner, clock, address_map_constructor(FUNC(ht1130_device::internal_data_map), this))
{
}

ht1190_device::ht1190_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ht1130_device(mconfig, HT1190, tag, owner, clock, address_map_constructor(FUNC(ht1190_device::internal_data_map_ht1190), this))
{
}

std::unique_ptr<util::disasm_interface> ht1130_device::create_disassembler()
{
	return std::make_unique<ht1130_disassembler>();
}

device_memory_interface::space_config_vector ht1130_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_config),
		std::make_pair(AS_DATA, &m_data_config)
	};
}

void ht1130_device::internal_map(address_map &map)
{
	map(0x000, 0xfff).rom();
}

inline void ht1130_device::tempram_w(offs_t offset, u8 data)
{
	m_tempram[offset] = data & 0xf;
}

inline void ht1130_device::displayram_w(offs_t offset, u8 data)
{
	m_displayram[offset] = data & 0xf;
}

inline void ht1130_device::setreg(u8 which, u8 data)
{
	m_regs[which] = data & 0xf;
}

inline u8 ht1130_device::getreg(u8 which)
{
	return m_regs[which] & 0xf;
}

inline void ht1130_device::setacc(u8 data)
{
	m_acc = data & 0xf;
}

inline u8 ht1130_device::getacc()
{
	return m_acc & 0xf;
}

inline u8 ht1130_device::getcarry()
{
	return m_carry & 1;
}

inline void ht1130_device::setcarry()
{
	m_carry = 1;
}

inline void ht1130_device::clearcarry()
{
	m_carry = 0;
}

inline void ht1130_device::settimer(u8 data)
{
	m_timer = data;
}

inline void ht1130_device::settimer_upper(u8 data)
{
	m_timer = (m_timer & 0xf) | (data & 0xf) << 4;
}

inline void ht1130_device::settimer_lower(u8 data)
{
	m_timer = (m_timer & 0xf0) | (data & 0xf);
}

inline u8 ht1130_device::gettimer_upper()
{
	return (m_timer >> 4) & 0xf;
}

inline u8 ht1130_device::gettimer_lower()
{
	return m_timer & 0xf;
}

inline u8 ht1130_device::getr1r0()
{
	return (getreg(1) << 4) | getreg(0);
}

inline u8 ht1130_device::getr1r0_data()
{
	const u8 dataaddress = getr1r0();
	return m_data.read_byte(dataaddress) & 0xf;
}

inline void ht1130_device::setr1r0_data(u8 data)
{
	const u8 dataaddress = getr1r0();
	m_data.write_byte(dataaddress, data & 0xf);
}

inline u8 ht1130_device::getr3r2()
{
	return (getreg(3) << 4) | getreg(2);
}

inline u8 ht1130_device::getr3r2_data()
{
	const u8 dataaddress = getr3r2();
	return m_data.read_byte(dataaddress) & 0xf;
}

inline void ht1130_device::setr3r2_data(u8 data)
{
	const u8 dataaddress = getr3r2();
	m_data.write_byte(dataaddress, data & 0xf);
}


void ht1130_device::internal_data_map(address_map &map)
{
	map(0x00, 0x7f).ram().w(FUNC(ht1130_device::tempram_w)).share(m_tempram);
	map(0xe0, 0xff).ram().w(FUNC(ht1130_device::displayram_w)).share(m_displayram);
}

void ht1190_device::internal_data_map_ht1190(address_map &map)
{
	map(0x00, 0x9f).ram().w(FUNC(ht1190_device::tempram_w)).share(m_tempram);
	map(0xb0, 0xff).ram().w(FUNC(ht1190_device::displayram_w)).share(m_displayram);
}

void ht1130_device::init_lcd(u8 compins)
{
	m_lcd_timer = timer_alloc(FUNC(ht1130_device::update_lcd), this);

	// LCD refresh rate is ~64Hz (not affected by system OSC)
	attotime period = attotime::from_hz(64 * compins);
	m_lcd_timer->adjust(period, compins, period);
}

TIMER_CALLBACK_MEMBER(ht1130_device::update_lcd)
{
	m_segment_out(m_comcount, m_inhalt ? 0 : get_segs(m_comcount));
	m_comcount = (m_comcount + 1) % param;
}

u64 ht1130_device::get_segs(u8 com)
{
	u64 segs = 0;
	for (int i = 0; i < 0x20; i++)
		segs = segs << 1 | BIT(m_displayram[i ^ 0x1f], com & 3);

	return segs;
}

u64 ht1190_device::get_segs(u8 com)
{
	u64 segs = 0;
	for (int i = 0; i < 40; i++)
		segs = segs << 1 | BIT(m_displayram[(i << 1) | (com >> 2)], com & 3);

	return segs;
}

void ht1130_device::init_common()
{
	space(AS_PROGRAM).specific(m_space);
	space(AS_DATA).specific(m_data);

	set_icountptr(m_icount);

	// debugger
	state_add(HT1130_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();

	state_add(HT1130_ACC, "ACC", m_acc);

	for (int i = 0; i < 5; i++)
		state_add(HT1130_R0 + i, string_format("R%d", i).c_str(), m_regs[i]);

	state_add(HT1130_TIMER_EN, "TIMER_EN", m_timer_en);
	state_add(HT1130_TIMER, "TIMER", m_timer);

	// zerofill
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_acc = 0;
	m_stackaddr = 0;
	m_stackcarry = 0;

	m_pc = 0;
	m_carry = 0;
	m_irqen = 0;
	m_timer_en = 0;
	m_inhalt = 0;
	m_wakeline = 0;
	m_timerover = 0;
	m_timer = 0;
	m_comcount = 0;

	// savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_regs));
	save_item(NAME(m_acc));
	save_item(NAME(m_carry));
	save_item(NAME(m_irqen));
	save_item(NAME(m_timer_en));
	save_item(NAME(m_inhalt));
	save_item(NAME(m_wakeline));
	save_item(NAME(m_timerover));
	save_item(NAME(m_timer));
	save_item(NAME(m_comcount));
	save_item(NAME(m_stackaddr));
	save_item(NAME(m_stackcarry));
}

void ht1130_device::device_start()
{
	init_common();
	init_lcd(4); // 4 COM pins
}

void ht1190_device::device_start()
{
	init_common();
	init_lcd(8); // 8 COM pins
}

void ht1130_device::device_reset()
{
	m_pc = 0;
	m_carry = 0;
	m_irqen = 0;
	m_timer_en = 0;
	m_inhalt = 0;
	m_timerover = 0;
	m_timer = 0;
}

void ht1130_device::cycle()
{
	m_icount--;
	if (m_timer_en)
		m_timer++;
}

u8 ht1130_device::fetch()
{
	cycle();
	return m_space.read_byte(m_pc++);
}

void ht1130_device::do_op()
{
	const u8 inst = fetch();

	switch (inst)
	{
	case 0b00001000: // ADC A,[R1R0] : Add data memory contents and carry to the accumulator
	{
		const u8 data = getr1r0_data();

		u8 acc = getacc();
		acc = acc + data + getcarry();
		if (acc & 0x10)
			setcarry();
		else
			clearcarry();
		setacc(acc);

		return;
	}

	case 0b00001001: // ADD A,[R1R0] : Add data memory contents to the accumulator
	{
		const u8 data = getr1r0_data();

		u8 acc = getacc();
		acc = acc + data;
		if (acc & 0x10)
			setcarry();
		else
			clearcarry();
		setacc(acc);

		return;
	}

	case 0b00011010: // AND A,[R1R0] : Logical AND accumulator with data memory
	{
		const u8 data = getr1r0_data();
		const u8 acc = getacc();
		setacc(acc & data);
		return;
	}

	case 0b00011101: // AND [R1R0],A : Logical AND data memory with accumulator
	{
		const u8 data = getr1r0_data();
		const u8 acc = getacc();
		setr1r0_data(data & acc);
		return;
	}

	case 0b00101010: // CLC : Clear carry flag
	{
		clearcarry();
		return;
	}

	case 0b00110110: // DAA : Decimal-Adjust accumulator
	{
		u8 acc = getacc();
		if (acc > 9 || getcarry())
		{
			acc = (acc + 6) & 0xf;
			setcarry();
			setacc(acc);
		}
		return;
	}

	case 0b00111111: // DEC A : Decrement accumulator
	{
		const u8 acc = getacc();
		setacc(acc-1);
		return;
	}

	case 0b00001101: // DEC [R1R0] : Decrement data memory
	{
		const u8 data = getr1r0_data();
		setr1r0_data(data-1);
		return;
	}

	case 0b00001111: // DEC [R3R2] : Decrement data memory
	{
		const u8 data = getr3r2_data();
		setr3r2_data(data-1);
		return;
	}

	case 0b00101101: // DI : Disable interrupt
	{
		m_irqen = 0;
		return;
	}

	case 0b00101100: // EI : Enable interrupt
	{
		m_irqen = 1;
		return;
	}

	case 0b00110010: // IN A,PM : Input port to accumulator (doesn't exist on HT1190, does on HT1130?)
	{
		const u8 data = m_port_in_pm() & 0xf;
		setacc(data);
		return;
	}

	case 0b00110011: // IN A,PS : Input port to accumulator
	{
		const u8 data = m_port_in_ps() & 0xf;
		setacc(data);
		return;
	}

	case 0b00110100: // IN A,PP : Input port to accumulator
	{
		const u8 data = m_port_in_pp() & 0xf;
		setacc(data);
		return;
	}

	case 0b00110001: // INC A : Increment accumulator
	{
		const u8 temp = getacc();
		setacc(temp+1);
		return;
	}

	case 0b00001100: // INC [R1R0] : Increment data memory
	{
		const u8 data = getr1r0_data();
		setr1r0_data(data+1);
		return;
	}

	case 0b00001110: // INC [R3R2] : Increment data memory
	{
		const u8 data = getr3r2_data();
		setr3r2_data(data+1);
		return;
	}

	case 0b00000100: // MOV A,[R1R0] : Move data memory to accumulator
	{
		const u8 data = getr1r0_data();
		setacc(data);
		return;
	}

	case 0b00000110: // MOV A,[R3R2] : Move data memory to accumulator
	{
		const u8 data = getr3r2_data();
		setacc(data);
		return;
	}

	case 0b00000101: // MOV [R1R0],A : Move accumulator to data memory
	{
		const u8 acc = getacc();
		setr1r0_data(acc);
		return;
	}

	case 0b00000111: // MOV [R3R2],A : Move accumulator to data memory
	{
		const u8 acc = getacc();
		setr3r2_data(acc);
		return;
	}

	case 0b00111110: // NOP : No operation
	{
		// nothing
		return;
	}

	case 0b00011100: // OR A,[R1R0] : Logical OR accumulator with data memory
	{
		const u8 data = getr1r0_data();
		const u8 acc = getacc();
		setacc(acc | data);
		return;
	}

	case 0b00011111: // OR [R1R0],A : Logical OR data memory with accumulator
	{
		const u8 data = getr1r0_data();
		const u8 acc = getacc();
		setr1r0_data(data | acc);
		return;
	}

	case 0b00110000: // OUT PA,A : Output accumulator data to port A
	{
		const u8 data = getacc();
		m_port_out_pa(data);
		return;
	}

	case 0b01001110: // READ MR0A : Read ROM code of current page to M(R1,R0) and ACC
	{
		cycle();
		const u16 dataddress = (m_pc & 0xf00) | (getacc() << 4) | (getreg(4));
		const u8 data = m_space.read_byte(dataddress);
		setr1r0_data((data >> 4) & 0xf);
		setacc(data & 0xf);
		return;
	}

	case 0b01001100: // READ R4A : Read ROM code of current page to R4 and accumulator
	{
		cycle();
		const u16 dataddress = (m_pc & 0xf00) | (getacc() << 4) | (getr1r0_data());
		const u8 data = m_space.read_byte(dataddress);
		setreg(4, (data >> 4) & 0xf);
		setacc(data & 0xf);
		return;
	}

	case 0b01001111: // READF MR0A : Read ROM Code of page F to M(R1,R0) and ACC
	{
		cycle();
		const u16 dataddress = 0xf00 | (getacc() << 4) | (getreg(4));
		const u8 data = m_space.read_byte(dataddress);
		setr1r0_data((data >> 4) & 0xf);
		setacc(data & 0xf);
		return;
	}

	case 0b01001101: // READF R4A : Read ROM code of page F to R4 and accumulator
	{
		cycle();
		const u16 dataddress = 0xf00 | (getacc() << 4) | (getr1r0_data());
		const u8 data = m_space.read_byte(dataddress);
		setreg(4, (data >> 4) & 0xf);
		setacc(data & 0xf);
		return;
	}

	case 0b00000001: // RL A : Rotate accumulator left
	{
		u8 acc = getacc();
		const u8 oldr3 = (acc & 8)>>3;
		if (oldr3)
			setcarry();
		else
			clearcarry();
		acc = (acc << 1) | oldr3;
		setacc(acc);
		return;
	}

	case 0b00000011: // RLC A : Rotate accumulator left through carry
	{
		const u8 oldcarry = getcarry();
		u8 acc = getacc();
		if (acc & 0x8)
			setcarry();
		else
			clearcarry();
		acc = (acc << 1) | oldcarry;
		setacc(acc);
		return;
	}

	case 0b00000000: // RR A : Rotate accumulator right
	{
		u8 acc = getacc();
		const u8 oldr0 = acc & 1;
		if (oldr0)
			setcarry();
		else
			clearcarry();
		acc = (acc >> 1) | oldr0 << 3;
		setacc(acc);
		return;
	}

	case 0b00000010: // RRC A : Rotate accumulator right through carry
	{
		const u8 oldcarry = getcarry();
		u8 acc = getacc();
		if (acc & 0x1)
			setcarry();
		else
			clearcarry();
		acc = (acc >> 1) | (oldcarry << 3);
		setacc(acc);
		return;
	}

	case 0b00001010: // SBC A,[R1R0] : Subtract data memory contents and carry from accumulator
	{
		const u8 data = getr1r0_data();

		u8 acc = getacc();
		acc = acc + (0xf - data) + getcarry();
		if (acc & 0x10)
			setcarry();
		else
			clearcarry();
		setacc(acc);

		return;
	}

	case 0b00101011: // STC : Set carry flag
	{
		setcarry();
		return;
	}

	case 0b00001011: // SUB A,[R1R0] : Subtract data memory contents from accumulator
	{
		const u8 data = getr1r0_data();

		u8 acc = getacc();
		acc = acc + (0xf - data) + 1;
		if (acc & 0x10)
			setcarry();
		else
			clearcarry();
		setacc(acc);

		return;
	}

	case 0b00111001: // TIMER OFF : Set timer to stop counting
	{
		m_timer_en = 0;
		return;
	}

	case 0b00111000: // TIMER ON : Set timer to start counting
	{
		m_timer_en = 1;
		return;
	}

	case 0b00011011: // XOR A,[R1R0] : Logical XOR accumulator with data memory
	{
		const u8 data = getr1r0_data();
		const u8 acc = getacc();
		setacc(acc ^ data);
		return;
	}

	case 0b00011110: // XOR [R1R0],A : Logical XOR data memory with accumulator
	{
		const u8 data = getr1r0_data();
		const u8 acc = getacc();
		setr1r0_data(data ^ acc);
		return;
	}

	//// Opcodes with XH Immediates

	case 0b01000000: // (with 4-bit immediate) : ADD A,XH : Add immediate data to the accumulator
	{
		const u8 operand = fetch() & 0x0f;
		u8 acc = getacc();
		acc += operand;
		if (acc & 0x10)
			setcarry();
		else
			clearcarry();
		setacc(acc);
		return;
	}

	case 0b01000010: // (with 4-bit immediate) : AND A,XH : Logical AND immediate data to accumulator
	{
		const u8 operand = fetch() & 0x0f;
		const u8 acc = getacc();
		setacc(acc & operand);
		return;
	}

	case 0b01000110: // (with 4-bit immediate) : MOV R4,XH : Move immediate data to R4
	{
		const u8 operand = fetch() & 0x0f;
		setreg(4, operand);
		return;
	}

	case 0b01000100: // (with 4-bit immediate) : OR A,XH :  Logical OR immediate data to accumulator
	{
		const u8 operand = fetch() & 0x0f;
		const u8 acc = getacc();
		setacc(acc | operand);
		return;
	}

	case 0b01000001: // (with 4-bit immediate) : SUB A,XH : Subtract immediate data from accumulator
	{
		const u8 operand = fetch() & 0x0f;
		u8 acc = getacc();
		acc += (0xf-operand) + 1;
		if (acc & 0x10)
			setcarry();
		else
			clearcarry();
		setacc(acc);
		return;
	}

	case 0b01000011: // (with 4-bit immediate) : XOR A,XH : Logical XOR immediate data to accumulator
	{
		const u8 operand = fetch() & 0x0f;
		const u8 acc = getacc();
		setacc(acc ^ operand);
		return;
	}

	// case 0b0111dddd: // MOV A,XH : Move immediate data to accumulator
	case 0b01110000: case 0b01110001: case 0b01110010: case 0b01110011:
	case 0b01110100: case 0b01110101: case 0b01110110: case 0b01110111:
	case 0b01111000: case 0b01111001: case 0b01111010: case 0b01111011:
	case 0b01111100: case 0b01111101: case 0b01111110: case 0b01111111:
	{
		const u8 operand = inst & 0x0f;
		setacc(operand);
		return;
	}

	// Ops using registers

	// case 0b0001nnn1: DEC Rn : Decrement register (R0-R4)
	case 0b00010001: case 0b00010011: case 0b00010101: case 0b00010111:
	case 0b00011001:
	{
		const u8 reg = (inst & 0x0e) >> 1;
		const u8 temp = getreg(reg);
		setreg(reg, temp-1);
		return;
	}

	// case 0b0001nnn0: INC Rn : Increment register
	case 0b00010000: case 0b00010010: case 0b00010100: case 0b00010110:
	case 0b00011000:
	{
		const u8 reg = (inst & 0x0e) >> 1;
		const u8 temp = getreg(reg);
		setreg(reg, temp+1);
		return;
	}

	// case 0b0010nnn1: MOV A,Rn : Move register to accumulator
	case 0b00100001: case 0b00100011: case 0b00100101: case 0b00100111:
	case 0b00101001:
	{
		const u8 reg = (inst & 0x0e) >> 1;
		const u8 temp = getreg(reg);
		setacc(temp);
		return;
	}

	// case 0b0010nnn0: MOV Rn,A : Move accumulator to register
	case 0b00100000: case 0b00100010: case 0b00100100: case 0b00100110:
	case 0b00101000:
	{
		const u8 reg = (inst & 0x0e) >> 1;
		const u8 temp = getacc();
		setreg(reg, temp);
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// 2 reg Move ops
	///////////////////////////////////////////////////////////////////////////////////////

	// case 0b0101dddd: // (with 4-bit immediate) : MOV R1R0,XXH : Move immediate data to R1 and R0
	case 0b01010000: case 0b01010001: case 0b01010010: case 0b01010011:
	case 0b01010100: case 0b01010101: case 0b01010110: case 0b01010111:
	case 0b01011000: case 0b01011001: case 0b01011010: case 0b01011011:
	case 0b01011100: case 0b01011101: case 0b01011110: case 0b01011111:
	{
		const u8 operand = fetch();
		setreg(1, operand & 0xf);
		setreg(0, inst & 0xf);
		return;
	}

	// case 0b0110dddd: // (with 4-bit immediate) : MOV R3R2,XXH : Move immediate data to R3 and R2
	case 0b01100000: case 0b01100001: case 0b01100010: case 0b01100011:
	case 0b01100100: case 0b01100101: case 0b01100110: case 0b01100111:
	case 0b01101000: case 0b01101001: case 0b01101010: case 0b01101011:
	case 0b01101100: case 0b01101101: case 0b01101110: case 0b01101111:
	{
		const u8 operand = fetch();
		setreg(3, operand & 0xf);
		setreg(2, inst & 0xf);
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Jump / Call Ops (full address)
	///////////////////////////////////////////////////////////////////////////////////////

	// case 0b1111aaaa: // (with 8-bit immediate) : CALL address : Subroutine call
	case 0b11110000: case 0b11110001: case 0b11110010: case 0b11110011:
	case 0b11110100: case 0b11110101: case 0b11110110: case 0b11110111:
	case 0b11111000: case 0b11111001: case 0b11111010: case 0b11111011:
	case 0b11111100: case 0b11111101: case 0b11111110: case 0b11111111:
	{
		const u8 operand = fetch();
		const u16 fulladdr = ((inst & 0x0f) << 8) | operand;
		m_stackaddr = m_pc;
		m_pc = fulladdr;
		return;
	}

	// case 0b1110aaaa: // (with 8-bit immediate) : JMP address : Direct jump
	case 0b11100000: case 0b11100001: case 0b11100010: case 0b11100011:
	case 0b11100100: case 0b11100101: case 0b11100110: case 0b11100111:
	case 0b11101000: case 0b11101001: case 0b11101010: case 0b11101011:
	case 0b11101100: case 0b11101101: case 0b11101110: case 0b11101111:
	{
		const u8 operand = fetch();
		const u16 fulladdr = ((inst & 0x0f) << 8) | operand;
		m_pc = fulladdr;
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Jump / Call Ops (partial address)
	///////////////////////////////////////////////////////////////////////////////////////

	// case 0b11000aaa: (with 8-bit immediate) : JC address : Jump if carry is set
	case 0b11000000: case 0b11000001: case 0b11000010: case 0b11000011:
	case 0b11000100: case 0b11000101: case 0b11000110: case 0b11000111:
	{
		const u8 operand = fetch();
		const u16 fulladdr = ((inst & 0x07) << 8) | operand | (m_pc & 0x800);

		if (getcarry())
			m_pc = fulladdr;

		return;
	}

	// case 0b11001aaa: (with 8-bit immediate) : JNC address : Jump if carry is not set
	case 0b11001000: case 0b11001001: case 0b11001010: case 0b11001011:
	case 0b11001100: case 0b11001101: case 0b11001110: case 0b11001111:
	{
		const u8 operand = fetch();
		const u16 fulladdr = ((inst & 0x07) << 8) | operand | (m_pc & 0x800);

		if (!getcarry())
			m_pc = fulladdr;

		return;
	}

	// case 0b10111aaa: (with 8-bit immediate) : JNZ A,address : Jump if accumulator is not 0
	case 0b10111000: case 0b10111001: case 0b10111010: case 0b10111011:
	case 0b10111100: case 0b10111101: case 0b10111110: case 0b10111111:
	{
		const u8 operand = fetch();
		const u16 fulladdr = ((inst & 0x07) << 8) | operand | (m_pc & 0x800);

		if (getacc())
			m_pc = fulladdr;

		return;
	}

	// case 0b10100aaa: (with 8-bit immediate) : JNZ R0,address : Jump if register is not 0
	case 0b10100000: case 0b10100001: case 0b10100010: case 0b10100011:
	case 0b10100100: case 0b10100101: case 0b10100110: case 0b10100111:
	{
		const u8 operand = fetch();
		const u16 fulladdr = ((inst & 0x07) << 8) | operand | (m_pc & 0x800);

		if (getreg(0))
			m_pc = fulladdr;

		return;
	}

	// case 0b10101aaa: (with 8-bit immediate) : JNZ R1,address : Jump if register is not 0
	case 0b10101000: case 0b10101001: case 0b10101010: case 0b10101011:
	case 0b10101100: case 0b10101101: case 0b10101110: case 0b10101111:
	{
		const u8 operand = fetch();
		const u16 fulladdr = ((inst & 0x07) << 8) | operand | (m_pc & 0x800);

		if (getreg(1))
			m_pc = fulladdr;

		return;
	}

	// case 0b11011aaa: (with 8-bit immediate) : JNZ R4,address : Jump if register is not 0
	case 0b11011000: case 0b11011001: case 0b11011010: case 0b11011011:
	case 0b11011100: case 0b11011101: case 0b11011110: case 0b11011111:
	{
		const u8 operand = fetch();
		const u16 fulladdr = ((inst & 0x07) << 8) | operand | (m_pc & 0x800);

		if (getreg(4))
			m_pc = fulladdr;

		return;
	}

	// case 0b11010aaa: (with 8-bit immediate) : JTMR address : Jump if time-out
	case 0b11010000: case 0b11010001: case 0b11010010: case 0b11010011:
	case 0b11010100: case 0b11010101: case 0b11010110: case 0b11010111:
	{
		const u8 operand = fetch();
		const u16 fulladdr = ((inst & 0x07) << 8) | operand | (m_pc & 0x800);

		if (m_timerover)
		{
			m_pc = fulladdr;
			m_timerover = 0;
		}
		return;
	}

	// case 0b10110aaa: (with 8-bit immediate) : JZ A,address : Jump if accumulator is 0
	case 0b10110000: case 0b10110001: case 0b10110010: case 0b10110011:
	case 0b10110100: case 0b10110101: case 0b10110110: case 0b10110111:
	{
		const u8 operand = fetch();
		const u16 fulladdr = ((inst & 0x07) << 8) | operand | (m_pc & 0x800);

		if (!getacc())
			m_pc = fulladdr;

		return;
	}

	// case 0b100nnaaa: // (with 8-bit immediate) : JAn address : Jump if accumulator bit n is set
	case 0b10000000: case 0b10000001: case 0b10000010: case 0b10000011:
	case 0b10000100: case 0b10000101: case 0b10000110: case 0b10000111:
	case 0b10001000: case 0b10001001: case 0b10001010: case 0b10001011:
	case 0b10001100: case 0b10001101: case 0b10001110: case 0b10001111:
	case 0b10010000: case 0b10010001: case 0b10010010: case 0b10010011:
	case 0b10010100: case 0b10010101: case 0b10010110: case 0b10010111:
	case 0b10011000: case 0b10011001: case 0b10011010: case 0b10011011:
	case 0b10011100: case 0b10011101: case 0b10011110: case 0b10011111:
	{
		const u8 operand = fetch();
		const u16 fulladdr = ((inst & 0x07) << 8) | operand | (m_pc & 0x800);
		const u8 bit = (inst & 0x18) >> 3;

		if (BIT(getacc(),bit))
			m_pc = fulladdr;

		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Return Ops
	///////////////////////////////////////////////////////////////////////////////////////

	case 0b00101110: // RET : Return from subroutine or interrupt
	{
		m_pc = m_stackaddr;
		return;
	}

	case 0b00101111: // RETI : Return from interrupt subroutine
	{
		m_pc = m_stackaddr;
		if (m_stackcarry)
			setcarry();
		else
			clearcarry();
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Timer Ops
	///////////////////////////////////////////////////////////////////////////////////////

	case 0b00111011: // MOV A,TMRH : Move timer high nibble to accumulator
	{
		const u8 data = gettimer_upper();
		setacc(data);
		return;
	}

	case 0b00111010: // MOV A,TMRL : Move timer low nibble to accumulator
	{
		const u8 data = gettimer_lower();
		setacc(data);
		return;
	}

	case 0b00111101: // MOV TMRH,A : Move accumulator to timer high nibble
	{
		const u8 acc = getacc();
		settimer_upper(acc);
		return;
	}

	case 0b00111100: // MOV TMRL,A : Move accumulator to timer low nibble
	{
		const u8 acc = getacc();
		settimer_lower(acc);
		return;
	}

	case 0b01000111: // (with 8-bit immediate) : TIMER XXH : Set immediate data to timer counter
	{
		const u8 operand = fetch();
		settimer(operand);
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// SOUND Ops (unimplemented)
	///////////////////////////////////////////////////////////////////////////////////////

	case 0b01001011: // SOUND A : Activate sound channel with accumulator
	{
		LOGMASKED(LOG_UNHANDLED_SOUND_OPS, "SOUND A");
		return;
	}

	case 0b01001001: // SOUND LOOP : Turn on sound repeat cycle
	{
		LOGMASKED(LOG_UNHANDLED_SOUND_OPS, "SOUND LOOP");
		return;
	}

	case 0b01001010: // SOUND OFF : Turn off sound
	{
		LOGMASKED(LOG_UNHANDLED_SOUND_OPS, "SOUND OFF");
		return;
	}

	case 0b01001000: // SOUND ONE : Turn on sound 1 cycle
	{
		LOGMASKED(LOG_UNHANDLED_SOUND_OPS, "SOUND ONE");
		return;
	}

	case 0b01000101: // (with 4 bit immediate) : SOUND n : Activate sound channel n
	{
		u8 operand = fetch() & 0x0f;
		LOGMASKED(LOG_UNHANDLED_SOUND_OPS, "SOUND %d", operand);
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// other Ops
	///////////////////////////////////////////////////////////////////////////////////////

	case 0b00110111: // (with 00111110) : HALT : Halt system clock
	{
		const u8 operand = fetch();
		if (operand == 0b00111110) // this is a 'NOP' must HALT always be followed by NOP to work?
		{
			m_inhalt = 1;
			if (m_icount > 0)
				m_icount = 0;
		}
		else
		{
			LOGMASKED(LOG_UNHANDLED_OPS, "<ill HALT %02x %02x>", inst, operand);
		}
		return;
	}

	default:
	{
		LOGMASKED(LOG_UNHANDLED_OPS, "<ill %02x>", inst);
		return;
	}
	}
}

void ht1130_device::execute_run()
{
	if (m_inhalt)
	{
		debugger_wait_hook();
		m_icount = 0;
		return;
	}

	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

		do_op();

		if (m_timer & 0x100)
		{
			m_timer -= 0x100;
			m_timerover = 1; // can also generate an interrupt
		}
	}
}

void ht1130_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case HT1130_EXT_WAKEUP_LINE:
		// wake up is edge triggered
		if (state && !m_wakeline)
			m_inhalt = 0;
		m_wakeline = state;
		break;

	default:
		break;
	}
}
