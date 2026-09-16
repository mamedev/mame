// license:BSD-3-Clause
// copyright-holders:Tony La Porta
/************************************************************************

  Microchip PIC16C5x Emulator

  Copyright Tony La Porta
  Originally written for the MAME project.


**** Notes: ****

Addressing architecture is based on the Harvard addressing scheme.

Initially meant to be an interface chip to GI's CP1600, but pretty quickly
marketed as a more generic MCU.

A single scaler is available for the Counter/Timer or WatchDog Timer.
When connected to the Counter/Timer, it functions as a Prescaler,
hence prescale overflows, tick the Counter/Timer.
When connected to the WatchDog Timer, it functions as a Postscaler
hence WatchDog Timer overflows, tick the Postscaler. This scenario
means that the WatchDog timeout occurs when the Postscaler has
reached the scaler rate value, not when the WatchDog reaches zero.


**** TODO: ****

- PIC WatchDog Timer has a separate internal clock. For the moment, we're
  basing the count on a 4MHz input clock, since 4MHz is the typical
  input frequency (but by no means always).
- CLRWDT should prevent the WatchDog Timer from timing out and generating
  a device reset, but how is not known. The manual also mentions that
  the WatchDog Timer can only be disabled during ROM programming, and
  no other means seem to exist???
- RTCC/T0CKI frequency should be limited by internal clock. In other words:
  It shouldn't be able to detect all pulses if the frequency on the pin is
  higher than the CLKOUT frequency. On 4 clocks-per-cycle, the pin state is
  sensed at clock #2 and clock #4.
- get rid of m_picmodel checks (use virtual function overrides in subclasses)


**** Change Log: ****

TLP (06-Apr-2003)
- First Public release.

BO  (07-Apr-2003) Ver 1.01
- Renamed 'sleep' function to 'sleepic' to avoid C conflicts.

TLP (09-Apr-2003) Ver 1.10
- Fixed modification of file register $03 (Status).
- Corrected support for 7FFh (12-bit) size ROMs.
- The 'call' and 'goto' instructions weren't correctly handling the
  STATUS page info correctly.
- The FSR register was incorrectly oring the data with 0xe0 when read.
- Prescaler masking information was set to 3 instead of 7.
- Prescaler assign bit was set to 4 instead of 8.
- Timer source and edge select flags/masks were wrong.
- Corrected the memory bank selection in GET/SET_REGFILE and also the
  indirect register addressing.

BMP (18-May-2003) Ver 1.11
- pic16c5x_get_reg functions were missing 'returns'.

TLP (27-May-2003) Ver 1.12
- Fixed the WatchDog timer count.
- The Prescaler rate was incorrectly being zeroed, instead of the
  actual Prescaler counter in the CLRWDT and SLEEP instructions.
- Added masking to the FSR register. Upper unused bits are always 1.

TLP (27-Aug-2009) Ver 1.13
- Indirect addressing was not taking into account special purpose
  memory mapped locations.
- 'iorlw' instruction was saving the result to memory instead of
  the W register.
- 'tris' instruction no longer modifies Port-C on PIC models that
  do not have Port-C implemented.

TLP (07-Sep-2009) Ver 1.14
- Edge sense control for the T0 count input was incorrectly reversed

LE (05-Feb-2017) Ver 1.15
- Allow writing all bits of the status register except TO and PD.
  This enables e.g. bcf, bsf or clrf to change the flags when the
  status register is the destination.
- Changed rlf and rrf to update the carry flag in the last step.
  Fixes the case where the status register is the destination.

hap (12-Feb-2017) Ver 1.16
- Added basic support for the old GI PIC1650 and PIC1655.
- Made RTCC(aka T0CKI) pin an inputline handler.

pa (12-Jun-2022) Ver 1.17
- Port callback functions pass tristate value in mem_mask.

************************************************************************/

#include "emu.h"
#include "pic16c5x.h"
#include "16c5xdsm.h"


DEFINE_DEVICE_TYPE(PIC16C54, pic16c54_device, "pic16c54", "Microchip PIC16C54")
DEFINE_DEVICE_TYPE(PIC16C55, pic16c55_device, "pic16c55", "Microchip PIC16C55")
DEFINE_DEVICE_TYPE(PIC16C56, pic16c56_device, "pic16c56", "Microchip PIC16C56")
DEFINE_DEVICE_TYPE(PIC16C57, pic16c57_device, "pic16c57", "Microchip PIC16C57")
DEFINE_DEVICE_TYPE(PIC16C58, pic16c58_device, "pic16c58", "Microchip PIC16C58")

DEFINE_DEVICE_TYPE(PIC1650,  pic1650_device,  "pic1650",  "GI PIC1650")
DEFINE_DEVICE_TYPE(PIC1654S, pic1654s_device, "pic1654s", "GI PIC1654S")
DEFINE_DEVICE_TYPE(PIC1655,  pic1655_device,  "pic1655",  "GI PIC1655")


/****************************************************************************
 *  Internal Memory Maps
 ****************************************************************************/

void pic16c5x_device::rom_9(address_map &map)
{
	map(0x000, 0x1ff).rom();
}

void pic16c5x_device::rom_10(address_map &map)
{
	map(0x000, 0x3ff).rom();
}

void pic16c5x_device::rom_11(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void pic16c5x_device::core_regs(address_map &map, u8 mirror)
{
	map(0x00, 0x00).noprw().mirror(mirror); // Not an actual register, reading indirectly (FSR=0) returns 0
	map(0x01, 0x01).rw(FUNC(pic16c5x_device::tmr0_r), FUNC(pic16c5x_device::tmr0_w)).mirror(mirror);
	map(0x02, 0x02).rw(FUNC(pic16c5x_device::pcl_r), FUNC(pic16c5x_device::pcl_w)).mirror(mirror);
	map(0x03, 0x03).rw(FUNC(pic16c5x_device::status_r), FUNC(pic16c5x_device::status_w)).mirror(mirror);
	map(0x04, 0x04).rw(FUNC(pic16c5x_device::fsr_r), FUNC(pic16c5x_device::fsr_w)).mirror(mirror);
}

// PIC16C54, PIC16C56: 2 ports (12 pins) + 25 bytes of RAM
void pic16c5x_device::ram_5_2ports(address_map &map)
{
	core_regs(map);
	map(0x05, 0x05).rw(FUNC(pic16c5x_device::porta_r), FUNC(pic16c5x_device::porta_w));
	map(0x06, 0x06).rw(FUNC(pic16c5x_device::portb_r), FUNC(pic16c5x_device::portb_w));
	map(0x07, 0x1f).ram();
}

// PIC16C55: 3 ports (20 pins) + 24 bytes of RAM
void pic16c5x_device::ram_5_3ports(address_map &map)
{
	core_regs(map);
	map(0x05, 0x05).rw(FUNC(pic16c5x_device::porta_r), FUNC(pic16c5x_device::porta_w));
	map(0x06, 0x06).rw(FUNC(pic16c5x_device::portb_r), FUNC(pic16c5x_device::portb_w));
	map(0x07, 0x07).rw(FUNC(pic16c5x_device::portc_r), FUNC(pic16c5x_device::portc_w));
	map(0x08, 0x1f).ram();
}

// PIC1655: 3 ports + 24 bytes of RAM
void pic16c5x_device::ram_1655_3ports(address_map &map)
{
	core_regs(map);
	map(0x05, 0x05).r(FUNC(pic16c5x_device::porta_r)).nopw(); // A is input-only on 1655
	map(0x06, 0x06).rw(FUNC(pic16c5x_device::portb_r), FUNC(pic16c5x_device::portb_w));
	map(0x07, 0x07).rw(FUNC(pic16c5x_device::portc_r), FUNC(pic16c5x_device::portc_w));
	map(0x08, 0x1f).ram();
}

// PIC1650: 4 ports + 23 bytes of RAM
void pic16c5x_device::ram_5_4ports(address_map &map)
{
	core_regs(map);
	map(0x05, 0x05).rw(FUNC(pic16c5x_device::porta_r), FUNC(pic16c5x_device::porta_w));
	map(0x06, 0x06).rw(FUNC(pic16c5x_device::portb_r), FUNC(pic16c5x_device::portb_w));
	map(0x07, 0x07).rw(FUNC(pic16c5x_device::portc_r), FUNC(pic16c5x_device::portc_w));
	map(0x08, 0x08).rw(FUNC(pic16c5x_device::portd_r), FUNC(pic16c5x_device::portd_w));
	map(0x09, 0x1f).ram();
}

// PIC16C58: 2 ports (12 pins) + 73 bytes of RAM
void pic16c5x_device::ram_7_2ports(address_map &map)
{
	core_regs(map, 0x60);
	map(0x05, 0x05).rw(FUNC(pic16c5x_device::porta_r), FUNC(pic16c5x_device::porta_w)).mirror(0x60);
	map(0x06, 0x06).rw(FUNC(pic16c5x_device::portb_r), FUNC(pic16c5x_device::portb_w)).mirror(0x60);
	map(0x07, 0x0f).ram().mirror(0x60);
	map(0x10, 0x1f).ram();
	map(0x30, 0x3f).ram();
	map(0x50, 0x5f).ram();
	map(0x70, 0x7f).ram();
}

// PIC16C57: 3 ports (20 pins) + 72 bytes of RAM
void pic16c5x_device::ram_7_3ports(address_map &map)
{
	core_regs(map, 0x60);
	map(0x05, 0x05).rw(FUNC(pic16c5x_device::porta_r), FUNC(pic16c5x_device::porta_w)).mirror(0x60);
	map(0x06, 0x06).rw(FUNC(pic16c5x_device::portb_r), FUNC(pic16c5x_device::portb_w)).mirror(0x60);
	map(0x07, 0x07).rw(FUNC(pic16c5x_device::portc_r), FUNC(pic16c5x_device::portc_w)).mirror(0x60);
	map(0x08, 0x0f).ram().mirror(0x60);
	map(0x10, 0x1f).ram();
	map(0x30, 0x3f).ram();
	map(0x50, 0x5f).ram();
	map(0x70, 0x7f).ram();
}


pic16c5x_device::pic16c5x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, int data_width, int picmodel, address_map_constructor data_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, program_width, -1,
			((program_width == 9) ? address_map_constructor(FUNC(pic16c5x_device::rom_9), this) :
			((program_width == 10) ? address_map_constructor(FUNC(pic16c5x_device::rom_10), this) :
			address_map_constructor(FUNC(pic16c5x_device::rom_11), this))))
	, m_data_config("data", ENDIANNESS_LITTLE, 8, data_width, 0, data_map)
	, m_picmodel(picmodel)
	, m_data_width(data_width)
	, m_program_width(program_width)
	, m_temp_config(0)
	, m_read_port(*this, 0)
	, m_write_port(*this)
{
}


pic16c54_device::pic16c54_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16c5x_device(mconfig, PIC16C54, tag, owner, clock, 9, 5, 0x16C54, address_map_constructor(FUNC(pic16c54_device::ram_5_2ports), this))
{
}

pic16c55_device::pic16c55_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16c5x_device(mconfig, PIC16C55, tag, owner, clock, 9, 5, 0x16C55, address_map_constructor(FUNC(pic16c55_device::ram_5_3ports), this))
{
}

pic16c56_device::pic16c56_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16c5x_device(mconfig, PIC16C56, tag, owner, clock, 10, 5, 0x16C56, address_map_constructor(FUNC(pic16c56_device::ram_5_2ports), this))
{
}

pic16c57_device::pic16c57_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16c5x_device(mconfig, PIC16C57, tag, owner, clock, 11, 7, 0x16C57, address_map_constructor(FUNC(pic16c57_device::ram_7_3ports), this))
{
}

pic16c58_device::pic16c58_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16c5x_device(mconfig, PIC16C58, tag, owner, clock, 11, 7, 0x16C58, address_map_constructor(FUNC(pic16c58_device::ram_7_2ports), this))
{
}

pic1650_device::pic1650_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16c5x_device(mconfig, PIC1650, tag, owner, clock, 9, 5, 0x1650, address_map_constructor(FUNC(pic1650_device::ram_5_4ports), this))
{
}

pic1654s_device::pic1654s_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16c5x_device(mconfig, PIC1654S, tag, owner, clock, 9, 5, 0x1654, address_map_constructor(FUNC(pic1654s_device::ram_5_2ports), this))
{
}

pic1655_device::pic1655_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16c5x_device(mconfig, PIC1655, tag, owner, clock, 9, 5, 0x1655, address_map_constructor(FUNC(pic1655_device::ram_1655_3ports), this))
{
}

device_memory_interface::space_config_vector pic16c5x_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

std::unique_ptr<util::disasm_interface> pic16c5x_device::create_disassembler()
{
	return std::make_unique<pic16c5x_disassembler>();
}



// ******** The following is the Status Flag register definition. ********
// | 7 | 6 | 5 |  4 |  3 | 2 |  1 | 0 |
// |    PA     | TO | PD | Z | DC | C |
#define PA_REG      0xe0    // PA   Program Page Preselect - bit 8 is unused here
#define TO_FLAG     0x10    // TO   Time Out flag (WatchDog)
#define PD_FLAG     0x08    // PD   Power Down flag
#define Z_FLAG      0x04    // Z    Zero Flag
#define DC_FLAG     0x02    // DC   Digit Carry/Borrow flag (Nibble)
#define C_FLAG      0x01    // C    Carry/Borrow Flag (Byte)

#define PA      (m_STATUS & PA_REG)
#define TO      (m_STATUS & TO_FLAG)
#define PD      (m_STATUS & PD_FLAG)
#define ZERO    (m_STATUS & Z_FLAG)
#define DC      (m_STATUS & DC_FLAG)
#define CARRY   (m_STATUS & C_FLAG)


// ******** The following is the Option Flag register definition. ********
// | 7 | 6 |   5  |   4  |  3  | 2 | 1 | 0 |
// | 0 | 0 | TOCS | TOSE | PSA |    PS     |
#define T0CS_FLAG   0x20    // TOCS     Timer 0 clock source select
#define T0SE_FLAG   0x10    // TOSE     Timer 0 clock source edge select
#define PSA_FLAG    0x08    // PSA      Prescaler Assignment bit
#define PS_REG      0x07    // PS       Prescaler Rate select

#define T0CS    (m_OPTION & T0CS_FLAG)
#define T0SE    (m_OPTION & T0SE_FLAG)
#define PSA     (m_OPTION & PSA_FLAG)
#define PS      (m_OPTION & PS_REG)


// ******** The following is the Config Flag register definition. ********
// | 11 | 10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 |   2  | 1 | 0 |
// |              CP                     | WDTE |  FOSC |
							// CP       Code Protect (ROM read protect)
#define WDTE_FLAG   0x04    // WDTE     WatchDog Timer enable
#define FOSC_FLAG   0x03    // FOSC     Oscillator source select

#define WDTE    (m_CONFIG & WDTE_FLAG)
#define FOSC    (m_CONFIG & FOSC_FLAG)


/************************************************************************
 *  Shortcuts
 ************************************************************************/

#define CLR(flagreg, flag)  (flagreg &= u8(~flag))
#define SET(flagreg, flag)  (flagreg |= (flag))

#define ADDR    (m_opcode.b.l & 0x1f)
#define BITPOS  ((m_opcode.b.l >> 5) & 7)



void pic16c5x_device::calc_zero_flag()
{
	if (m_ALU == 0)
		SET(m_STATUS, Z_FLAG);
	else
		CLR(m_STATUS, Z_FLAG);
}

void pic16c5x_device::calc_add_flags(u8 augend)
{
	calc_zero_flag();

	if (augend > m_ALU)
		SET(m_STATUS, C_FLAG);
	else
		CLR(m_STATUS, C_FLAG);

	if ((augend & 0x0f) > (m_ALU & 0x0f))
		SET(m_STATUS, DC_FLAG);
	else
		CLR(m_STATUS, DC_FLAG);
}

void pic16c5x_device::calc_sub_flags(u8 minuend)
{
	calc_zero_flag();

	if (minuend < m_ALU)
		CLR(m_STATUS, C_FLAG);
	else
		SET(m_STATUS, C_FLAG);

	if ((minuend & 0x0f) < (m_ALU & 0x0f))
		CLR(m_STATUS, DC_FLAG);
	else
		SET(m_STATUS, DC_FLAG);
}



void pic16c5x_device::set_pc(u16 addr)
{
	m_PC = addr & m_program_mask;
}

u16 pic16c5x_device::pop_stack()
{
	u16 data = m_STACK[1];
	m_STACK[1] = m_STACK[0];
	return data & m_program_mask;
}

void pic16c5x_device::push_stack(u16 data)
{
	m_STACK[0] = m_STACK[1];
	m_STACK[1] = data & m_program_mask;
}



u8 pic16c5x_device::get_regfile(u8 addr) // Read from internal memory
{
	if (addr == 0) { // Indirect addressing
		addr = m_FSR;
	} else if (m_data_width != 5) {
		addr |= (m_FSR & 0x60); // FSR bits 6-5 are used for banking in direct mode
	}

	return m_data.read_byte(addr);
}

void pic16c5x_device::store_regfile(u8 addr, u8 data) // Write to internal memory
{
	if (addr == 0) { // Indirect addressing
		addr = m_FSR;
	} else if (m_data_width != 5) {
		addr |= (m_FSR & 0x60); // FSR bits 6-5 are used for banking in direct mode
	}

	m_data.write_byte(addr, data);
}


void pic16c5x_device::store_result(u8 addr, u8 data)
{
	if (m_opcode.b.l & 0x20)
		store_regfile(addr, data);
	else
		m_W = data;
}



u8 pic16c5x_device::tmr0_r()
{
	return m_TMR0;
}

void pic16c5x_device::tmr0_w(u8 data)
{
	m_delay_timer = 2; // Timer increment is inhibited for 2 cycles
	if (PSA == 0) m_prescaler = 0; // Must clear the Prescaler
	m_TMR0 = data;
}

u8 pic16c5x_device::pcl_r()
{
	return m_PC & 0x0ff;
}

void pic16c5x_device::pcl_w(u8 data)
{
	set_pc(((m_STATUS & PA_REG) << 4) | data);
	if (!machine().side_effects_disabled())
		m_inst_cycles++;
}

u8 pic16c5x_device::status_r()
{
	return m_STATUS;
}

void pic16c5x_device::status_w(u8 data)
{
	// on GI PIC165x, high bits are 1
	if (m_picmodel == 0x1650 || m_picmodel == 0x1654 || m_picmodel == 0x1655)
		m_STATUS = data | u8(~m_status_mask);
	else
		m_STATUS = (m_STATUS & (TO_FLAG | PD_FLAG)) | (data & u8(~(TO_FLAG | PD_FLAG)));
}

u8 pic16c5x_device::fsr_r()
{
	// high bits are 1
	return m_FSR | u8(~m_data_mask);
}

void pic16c5x_device::fsr_w(u8 data)
{
	m_FSR = data & m_data_mask;
}

u8 pic16c5x_device::porta_r()
{
	// read port A
	if ((m_picmodel == 0x1650) || (m_picmodel == 0x1654)) {
		return m_read_port[PORTA](PORTA, 0xff) & m_port_data[PORTA];
	}
	else if (m_picmodel == 0x1655) {
		return m_read_port[PORTA](PORTA, 0xff) & 0x0f;
	}
	else {
		u8 data = m_read_port[PORTA](PORTA, 0xff);
		data &= m_port_tris[PORTA];
		data |= (u8(~m_port_tris[PORTA]) & m_port_data[PORTA]);
		return data & 0x0f; // 4-bit port (only lower 4 bits used)
	}
}

void pic16c5x_device::porta_w(u8 data)
{
	// write port A
	if ((m_picmodel == 0x1650) || (m_picmodel == 0x1654)) {
		m_write_port[PORTA](PORTA, data, 0xff);
	}
	else {
		assert(m_picmodel != 0x1655);
		data &= 0x0f; // 4-bit port (only lower 4 bits used)
		m_write_port[PORTA](PORTA, data & u8(~m_port_tris[PORTA]) & 0x0f, u8(~m_port_tris[PORTA]) & 0x0f);
	}
	m_port_data[PORTA] = data;
}

u8 pic16c5x_device::portb_r()
{
	// read port B
	if ((m_picmodel == 0x1650) || (m_picmodel == 0x1654)) {
		return m_read_port[PORTB](PORTB, 0xff) & m_port_data[PORTB];
	}
	else if (m_picmodel != 0x1655) { // B is output-only on 1655
		u8 data = m_read_port[PORTB](PORTB, 0xff);
		data &= m_port_tris[PORTB];
		data |= (u8(~m_port_tris[PORTB]) & m_port_data[PORTB]);
		return data;
	}
	else
		return m_port_data[PORTB];
}

void pic16c5x_device::portb_w(u8 data)
{
	// write port B
	if (m_picmodel == 0x1650 || m_picmodel == 0x1654 || m_picmodel == 0x1655) {
		m_write_port[PORTB](PORTB, data, 0xff);
	}
	else {
		m_write_port[PORTB](PORTB, data & u8(~m_port_tris[PORTB]), u8(~m_port_tris[PORTB]));
	}
	m_port_data[PORTB] = data;
}

u8 pic16c5x_device::portc_r()
{
	// read port C
	if (m_picmodel == 0x1650 || m_picmodel == 0x1655) {
		return m_read_port[PORTC](PORTC, 0xff) & m_port_data[PORTC];
	}
	else {
		assert((m_picmodel == 0x16C55) || (m_picmodel == 0x16C57));
		u8 data = m_read_port[PORTC](PORTC, 0xff);
		data &= m_port_tris[PORTC];
		data |= (u8(~m_port_tris[PORTC]) & m_port_data[PORTC]);
		return data;
	}
}

void pic16c5x_device::portc_w(u8 data)
{
	// write port C
	if (m_picmodel == 0x1650 || m_picmodel == 0x1655) {
		m_write_port[PORTC](PORTC, data, 0xff);
	}
	else {
		assert((m_picmodel == 0x16C55) || (m_picmodel == 0x16C57));
		m_write_port[PORTC](PORTC, data & u8(~m_port_tris[PORTC]), u8(~m_port_tris[PORTC]));
	}
	m_port_data[PORTC] = data;
}

u8 pic16c5x_device::portd_r()
{
	// read port D
	assert(m_picmodel == 0x1650);
	return m_read_port[PORTD](PORTD, 0xff) & m_port_data[PORTD];
}

void pic16c5x_device::portd_w(u8 data)
{
	// write port D
	assert(m_picmodel == 0x1650);
	m_write_port[PORTD](PORTD, data, 0xff);
	m_port_data[PORTD] = data;
}


/************************************************************************
 *  Emulate the Instructions
 ************************************************************************/

void pic16c5x_device::illegal()
{
	logerror("PIC16C5x: PC=%03x, Illegal opcode = %04x\n", m_PREVPC, m_opcode.w);
}

/*
    Note:
    According to the manual, if the STATUS register is the destination for an instruction that affects the Z, DC or C bits
    then the write to these three bits is disabled. These bits are set or cleared according to the device logic.
    To ensure this is correctly emulated, in instructions that write to the file registers, always change the status flags
    *after* storing the result of the instruction.
    e.g. CALCULATE_*, SET(STATUS,*_FLAG) and CLR(STATUS,*_FLAG) should appear as the last steps of the instruction emulation.
*/

void pic16c5x_device::addwf()
{
	u8 augend = get_regfile(ADDR);
	m_ALU = augend + m_W;
	store_result(ADDR, m_ALU);
	calc_add_flags(augend);
}

void pic16c5x_device::andwf()
{
	m_ALU = get_regfile(ADDR) & m_W;
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}

void pic16c5x_device::andlw()
{
	m_ALU = m_opcode.b.l & m_W;
	m_W = m_ALU;
	calc_zero_flag();
}

void pic16c5x_device::bcf()
{
	m_ALU = get_regfile(ADDR);
	m_ALU &= ~(1 << BITPOS);
	store_regfile(ADDR, m_ALU);
}

void pic16c5x_device::bsf()
{
	m_ALU = get_regfile(ADDR);
	m_ALU |= 1 << BITPOS;
	store_regfile(ADDR, m_ALU);
}

void pic16c5x_device::btfss()
{
	if (BIT(get_regfile(ADDR), BITPOS)) {
		set_pc(m_PC + 1);
		m_inst_cycles++; // Add NOP cycles
	}
}

void pic16c5x_device::btfsc()
{
	if (!BIT(get_regfile(ADDR), BITPOS)) {
		set_pc(m_PC + 1);
		m_inst_cycles++; // Add NOP cycles
	}
}

void pic16c5x_device::call()
{
	push_stack(m_PC);
	set_pc((((m_STATUS & PA_REG) << 4) | m_opcode.b.l) & 0x6ff);
}

void pic16c5x_device::clrw()
{
	m_W = 0;
	SET(m_STATUS, Z_FLAG);
}

void pic16c5x_device::clrf()
{
	store_regfile(ADDR, 0);
	SET(m_STATUS, Z_FLAG);
}

void pic16c5x_device::clrwdt()
{
	m_WDT = 0;
	if (PSA) m_prescaler = 0;
	SET(m_STATUS, TO_FLAG);
	SET(m_STATUS, PD_FLAG);
}

void pic16c5x_device::comf()
{
	m_ALU = u8(~(get_regfile(ADDR)));
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}

void pic16c5x_device::decf()
{
	m_ALU = get_regfile(ADDR) - 1;
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}

void pic16c5x_device::decfsz()
{
	m_ALU = get_regfile(ADDR) - 1;
	store_result(ADDR, m_ALU);
	if (m_ALU == 0) {
		set_pc(m_PC + 1);
		m_inst_cycles++; // Add NOP cycles
	}
}

void pic16c5x_device::goto_op()
{
	set_pc(((m_STATUS & PA_REG) << 4) | (m_opcode.w & 0x1ff));
}

void pic16c5x_device::incf()
{
	m_ALU = get_regfile(ADDR) + 1;
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}

void pic16c5x_device::incfsz()
{
	m_ALU = get_regfile(ADDR) + 1;
	store_result(ADDR, m_ALU);
	if (m_ALU == 0) {
		set_pc(m_PC + 1);
		m_inst_cycles++; // Add NOP cycles
	}
}

void pic16c5x_device::iorlw()
{
	m_ALU = m_opcode.b.l | m_W;
	m_W = m_ALU;
	calc_zero_flag();
}

void pic16c5x_device::iorwf()
{
	m_ALU = get_regfile(ADDR) | m_W;
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}

void pic16c5x_device::movf()
{
	m_ALU = get_regfile(ADDR);
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}

void pic16c5x_device::movlw()
{
	m_W = m_opcode.b.l;
}

void pic16c5x_device::movwf()
{
	store_regfile(ADDR, m_W);
}

void pic16c5x_device::nop()
{
	// Do nothing
}

void pic16c5x_device::option()
{
	m_OPTION = m_W & (T0CS_FLAG | T0SE_FLAG | PSA_FLAG | PS_REG);
}

void pic16c5x_device::retlw()
{
	m_W = m_opcode.b.l;
	set_pc(pop_stack());
}

void pic16c5x_device::rlf()
{
	m_ALU = get_regfile(ADDR);
	int carry = BIT(m_ALU, 7);
	m_ALU <<= 1;
	if (m_STATUS & C_FLAG) m_ALU |= 1;
	store_result(ADDR, m_ALU);

	if (carry)
		SET(m_STATUS, C_FLAG);
	else
		CLR(m_STATUS, C_FLAG);
}

void pic16c5x_device::rrf()
{
	m_ALU = get_regfile(ADDR);
	int carry = BIT(m_ALU, 0);
	m_ALU >>= 1;
	if (m_STATUS & C_FLAG) m_ALU |= 0x80;
	store_result(ADDR, m_ALU);

	if (carry)
		SET(m_STATUS, C_FLAG);
	else
		CLR(m_STATUS, C_FLAG);
}

void pic16c5x_device::sleepic()
{
	if (WDTE) m_WDT = 0;
	if (PSA) m_prescaler = 0;
	SET(m_STATUS, TO_FLAG);
	CLR(m_STATUS, PD_FLAG);
}

void pic16c5x_device::subwf()
{
	u8 minuend = get_regfile(ADDR);
	m_ALU = minuend - m_W;
	store_result(ADDR, m_ALU);
	calc_sub_flags(minuend);
}

void pic16c5x_device::swapf()
{
	u8 reg = get_regfile(ADDR);
	m_ALU = reg << 4 | reg >> 4;
	store_result(ADDR, m_ALU);
}

void pic16c5x_device::tris()
{
	switch (m_opcode.b.l & 0x7)
	{
		case 5:
			if (m_port_tris[PORTA] != m_W) {
				m_port_tris[PORTA] = m_W | 0xf0;
				m_write_port[PORTA](PORTA, m_port_data[PORTA] & u8(~m_port_tris[PORTA]) & 0x0f, u8(~m_port_tris[PORTA]) & 0x0f);
			}
			break;

		case 6:
			if (m_port_tris[PORTB] != m_W) {
				m_port_tris[PORTB] = m_W;
				m_write_port[PORTB](PORTB, m_port_data[PORTB] & u8(~m_port_tris[PORTB]), u8(~m_port_tris[PORTB]));
			}
			break;

		case 7:
			if ((m_picmodel == 0x16C55) || (m_picmodel == 0x16C57)) {
				if (m_port_tris[PORTC] != m_W) {
					m_port_tris[PORTC] = m_W;
					m_write_port[PORTC](PORTC, m_port_data[PORTC] & u8(~m_port_tris[PORTC]), u8(~m_port_tris[PORTC]));
				}
			}
			else {
				illegal();
			}
			break;

		default:
			illegal();
			break;
	}
}

void pic16c5x_device::xorlw()
{
	m_ALU = m_W ^ m_opcode.b.l;
	m_W = m_ALU;
	calc_zero_flag();
}

void pic16c5x_device::xorwf()
{
	m_ALU = get_regfile(ADDR) ^ m_W;
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}



/***********************************************************************
 *  Opcode Table (Cycles, Instruction)
 ***********************************************************************/

const pic16c5x_device::pic16c5x_opcode pic16c5x_device::s_opcode_main[256]=
{
	{1, &pic16c5x_device::nop     },{1, &pic16c5x_device::illegal   },{1, &pic16c5x_device::movwf     },{1, &pic16c5x_device::movwf     }, // 00
	{1, &pic16c5x_device::clrw    },{1, &pic16c5x_device::illegal   },{1, &pic16c5x_device::clrf      },{1, &pic16c5x_device::clrf      },
	{1, &pic16c5x_device::subwf   },{1, &pic16c5x_device::subwf     },{1, &pic16c5x_device::subwf     },{1, &pic16c5x_device::subwf     }, // 08
	{1, &pic16c5x_device::decf    },{1, &pic16c5x_device::decf      },{1, &pic16c5x_device::decf      },{1, &pic16c5x_device::decf      },
	{1, &pic16c5x_device::iorwf   },{1, &pic16c5x_device::iorwf     },{1, &pic16c5x_device::iorwf     },{1, &pic16c5x_device::iorwf     }, // 10
	{1, &pic16c5x_device::andwf   },{1, &pic16c5x_device::andwf     },{1, &pic16c5x_device::andwf     },{1, &pic16c5x_device::andwf     },
	{1, &pic16c5x_device::xorwf   },{1, &pic16c5x_device::xorwf     },{1, &pic16c5x_device::xorwf     },{1, &pic16c5x_device::xorwf     }, // 18
	{1, &pic16c5x_device::addwf   },{1, &pic16c5x_device::addwf     },{1, &pic16c5x_device::addwf     },{1, &pic16c5x_device::addwf     },
	{1, &pic16c5x_device::movf    },{1, &pic16c5x_device::movf      },{1, &pic16c5x_device::movf      },{1, &pic16c5x_device::movf      }, // 20
	{1, &pic16c5x_device::comf    },{1, &pic16c5x_device::comf      },{1, &pic16c5x_device::comf      },{1, &pic16c5x_device::comf      },
	{1, &pic16c5x_device::incf    },{1, &pic16c5x_device::incf      },{1, &pic16c5x_device::incf      },{1, &pic16c5x_device::incf      }, // 28
	{1, &pic16c5x_device::decfsz  },{1, &pic16c5x_device::decfsz    },{1, &pic16c5x_device::decfsz    },{1, &pic16c5x_device::decfsz    },
	{1, &pic16c5x_device::rrf     },{1, &pic16c5x_device::rrf       },{1, &pic16c5x_device::rrf       },{1, &pic16c5x_device::rrf       }, // 30
	{1, &pic16c5x_device::rlf     },{1, &pic16c5x_device::rlf       },{1, &pic16c5x_device::rlf       },{1, &pic16c5x_device::rlf       },
	{1, &pic16c5x_device::swapf   },{1, &pic16c5x_device::swapf     },{1, &pic16c5x_device::swapf     },{1, &pic16c5x_device::swapf     }, // 38
	{1, &pic16c5x_device::incfsz  },{1, &pic16c5x_device::incfsz    },{1, &pic16c5x_device::incfsz    },{1, &pic16c5x_device::incfsz    },
	{1, &pic16c5x_device::bcf     },{1, &pic16c5x_device::bcf       },{1, &pic16c5x_device::bcf       },{1, &pic16c5x_device::bcf       }, // 40
	{1, &pic16c5x_device::bcf     },{1, &pic16c5x_device::bcf       },{1, &pic16c5x_device::bcf       },{1, &pic16c5x_device::bcf       },
	{1, &pic16c5x_device::bcf     },{1, &pic16c5x_device::bcf       },{1, &pic16c5x_device::bcf       },{1, &pic16c5x_device::bcf       }, // 48
	{1, &pic16c5x_device::bcf     },{1, &pic16c5x_device::bcf       },{1, &pic16c5x_device::bcf       },{1, &pic16c5x_device::bcf       },
	{1, &pic16c5x_device::bsf     },{1, &pic16c5x_device::bsf       },{1, &pic16c5x_device::bsf       },{1, &pic16c5x_device::bsf       }, // 50
	{1, &pic16c5x_device::bsf     },{1, &pic16c5x_device::bsf       },{1, &pic16c5x_device::bsf       },{1, &pic16c5x_device::bsf       },
	{1, &pic16c5x_device::bsf     },{1, &pic16c5x_device::bsf       },{1, &pic16c5x_device::bsf       },{1, &pic16c5x_device::bsf       }, // 58
	{1, &pic16c5x_device::bsf     },{1, &pic16c5x_device::bsf       },{1, &pic16c5x_device::bsf       },{1, &pic16c5x_device::bsf       },
	{1, &pic16c5x_device::btfsc   },{1, &pic16c5x_device::btfsc     },{1, &pic16c5x_device::btfsc     },{1, &pic16c5x_device::btfsc     }, // 60
	{1, &pic16c5x_device::btfsc   },{1, &pic16c5x_device::btfsc     },{1, &pic16c5x_device::btfsc     },{1, &pic16c5x_device::btfsc     },
	{1, &pic16c5x_device::btfsc   },{1, &pic16c5x_device::btfsc     },{1, &pic16c5x_device::btfsc     },{1, &pic16c5x_device::btfsc     }, // 68
	{1, &pic16c5x_device::btfsc   },{1, &pic16c5x_device::btfsc     },{1, &pic16c5x_device::btfsc     },{1, &pic16c5x_device::btfsc     },
	{1, &pic16c5x_device::btfss   },{1, &pic16c5x_device::btfss     },{1, &pic16c5x_device::btfss     },{1, &pic16c5x_device::btfss     }, // 70
	{1, &pic16c5x_device::btfss   },{1, &pic16c5x_device::btfss     },{1, &pic16c5x_device::btfss     },{1, &pic16c5x_device::btfss     },
	{1, &pic16c5x_device::btfss   },{1, &pic16c5x_device::btfss     },{1, &pic16c5x_device::btfss     },{1, &pic16c5x_device::btfss     }, // 78
	{1, &pic16c5x_device::btfss   },{1, &pic16c5x_device::btfss     },{1, &pic16c5x_device::btfss     },{1, &pic16c5x_device::btfss     },
	{2, &pic16c5x_device::retlw   },{2, &pic16c5x_device::retlw     },{2, &pic16c5x_device::retlw     },{2, &pic16c5x_device::retlw     }, // 80
	{2, &pic16c5x_device::retlw   },{2, &pic16c5x_device::retlw     },{2, &pic16c5x_device::retlw     },{2, &pic16c5x_device::retlw     },
	{2, &pic16c5x_device::retlw   },{2, &pic16c5x_device::retlw     },{2, &pic16c5x_device::retlw     },{2, &pic16c5x_device::retlw     }, // 88
	{2, &pic16c5x_device::retlw   },{2, &pic16c5x_device::retlw     },{2, &pic16c5x_device::retlw     },{2, &pic16c5x_device::retlw     },
	{2, &pic16c5x_device::call    },{2, &pic16c5x_device::call      },{2, &pic16c5x_device::call      },{2, &pic16c5x_device::call      }, // 90
	{2, &pic16c5x_device::call    },{2, &pic16c5x_device::call      },{2, &pic16c5x_device::call      },{2, &pic16c5x_device::call      },
	{2, &pic16c5x_device::call    },{2, &pic16c5x_device::call      },{2, &pic16c5x_device::call      },{2, &pic16c5x_device::call      }, // 98
	{2, &pic16c5x_device::call    },{2, &pic16c5x_device::call      },{2, &pic16c5x_device::call      },{2, &pic16c5x_device::call      },
	{2, &pic16c5x_device::goto_op },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   }, // A0
	{2, &pic16c5x_device::goto_op },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   },
	{2, &pic16c5x_device::goto_op },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   }, // A8
	{2, &pic16c5x_device::goto_op },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   },
	{2, &pic16c5x_device::goto_op },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   }, // B0
	{2, &pic16c5x_device::goto_op },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   },
	{2, &pic16c5x_device::goto_op },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   }, // B8
	{2, &pic16c5x_device::goto_op },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   },{2, &pic16c5x_device::goto_op   },
	{1, &pic16c5x_device::movlw   },{1, &pic16c5x_device::movlw     },{1, &pic16c5x_device::movlw     },{1, &pic16c5x_device::movlw     }, // C0
	{1, &pic16c5x_device::movlw   },{1, &pic16c5x_device::movlw     },{1, &pic16c5x_device::movlw     },{1, &pic16c5x_device::movlw     },
	{1, &pic16c5x_device::movlw   },{1, &pic16c5x_device::movlw     },{1, &pic16c5x_device::movlw     },{1, &pic16c5x_device::movlw     }, // C8
	{1, &pic16c5x_device::movlw   },{1, &pic16c5x_device::movlw     },{1, &pic16c5x_device::movlw     },{1, &pic16c5x_device::movlw     },
	{1, &pic16c5x_device::iorlw   },{1, &pic16c5x_device::iorlw     },{1, &pic16c5x_device::iorlw     },{1, &pic16c5x_device::iorlw     }, // D0
	{1, &pic16c5x_device::iorlw   },{1, &pic16c5x_device::iorlw     },{1, &pic16c5x_device::iorlw     },{1, &pic16c5x_device::iorlw     },
	{1, &pic16c5x_device::iorlw   },{1, &pic16c5x_device::iorlw     },{1, &pic16c5x_device::iorlw     },{1, &pic16c5x_device::iorlw     }, // D8
	{1, &pic16c5x_device::iorlw   },{1, &pic16c5x_device::iorlw     },{1, &pic16c5x_device::iorlw     },{1, &pic16c5x_device::iorlw     },
	{1, &pic16c5x_device::andlw   },{1, &pic16c5x_device::andlw     },{1, &pic16c5x_device::andlw     },{1, &pic16c5x_device::andlw     }, // E0
	{1, &pic16c5x_device::andlw   },{1, &pic16c5x_device::andlw     },{1, &pic16c5x_device::andlw     },{1, &pic16c5x_device::andlw     },
	{1, &pic16c5x_device::andlw   },{1, &pic16c5x_device::andlw     },{1, &pic16c5x_device::andlw     },{1, &pic16c5x_device::andlw     }, // E8
	{1, &pic16c5x_device::andlw   },{1, &pic16c5x_device::andlw     },{1, &pic16c5x_device::andlw     },{1, &pic16c5x_device::andlw     },
	{1, &pic16c5x_device::xorlw   },{1, &pic16c5x_device::xorlw     },{1, &pic16c5x_device::xorlw     },{1, &pic16c5x_device::xorlw     }, // F0
	{1, &pic16c5x_device::xorlw   },{1, &pic16c5x_device::xorlw     },{1, &pic16c5x_device::xorlw     },{1, &pic16c5x_device::xorlw     },
	{1, &pic16c5x_device::xorlw   },{1, &pic16c5x_device::xorlw     },{1, &pic16c5x_device::xorlw     },{1, &pic16c5x_device::xorlw     }, // F8
	{1, &pic16c5x_device::xorlw   },{1, &pic16c5x_device::xorlw     },{1, &pic16c5x_device::xorlw     },{1, &pic16c5x_device::xorlw     }
};


const pic16c5x_device::pic16c5x_opcode pic16c5x_device::s_opcode_00x[16]=
{
	{1, &pic16c5x_device::nop     },{1, &pic16c5x_device::illegal   },{1, &pic16c5x_device::option    },{1, &pic16c5x_device::sleepic   }, // 00
	{1, &pic16c5x_device::clrwdt  },{1, &pic16c5x_device::tris      },{1, &pic16c5x_device::tris      },{1, &pic16c5x_device::tris      },
	{1, &pic16c5x_device::illegal },{1, &pic16c5x_device::illegal   },{1, &pic16c5x_device::illegal   },{1, &pic16c5x_device::illegal   }, // 08
	{1, &pic16c5x_device::illegal },{1, &pic16c5x_device::illegal   },{1, &pic16c5x_device::illegal   },{1, &pic16c5x_device::illegal   }
};



/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

enum
{
	PIC16C5x_PC=1, PIC16C5x_STK0, PIC16C5x_STK1, PIC16C5x_FSR,
	PIC16C5x_W,    PIC16C5x_ALU,  PIC16C5x_STR,  PIC16C5x_OPT,
	PIC16C5x_TMR0, PIC16C5x_PRTA, PIC16C5x_PRTB, PIC16C5x_PRTC, PIC16C5x_PRTD,
	PIC16C5x_WDT,  PIC16C5x_TRSA, PIC16C5x_TRSB, PIC16C5x_TRSC,
	PIC16C5x_PSCL
};

void pic16c5x_device::device_start()
{
	bool is_nmospic = (m_picmodel == 0x1650 || m_picmodel == 0x1654 || m_picmodel == 0x1655);

	space(AS_PROGRAM).cache(m_program);
	space(AS_DATA).specific(m_data);

	m_program_mask = (1 << m_program_width) - 1;
	m_data_mask = (1 << m_data_width) - 1;
	m_status_mask = is_nmospic ? 0x07 : 0xff;

	m_PC = 0;
	m_PREVPC = 0;
	m_W = 0;
	m_OPTION = 0;
	m_CONFIG = 0;
	m_ALU = 0;
	m_WDT = 0;
	m_TMR0 = 0;
	m_STATUS = ~m_status_mask;
	m_FSR = 0;
	std::fill(std::begin(m_port_data), std::end(m_port_data), 0);
	std::fill(std::begin(m_port_tris), std::end(m_port_tris), 0);
	std::fill(std::begin(m_STACK), std::end(m_STACK), 0);
	m_prescaler = 0;
	m_opcode.w = 0;

	m_delay_timer = 0;
	m_rtcc = 0;
	m_count_cycles = 0;
	m_inst_cycles = 0;
	m_debugger_temp = 0;

	// save states
	save_item(NAME(m_PC));
	save_item(NAME(m_PREVPC));
	save_item(NAME(m_W));
	save_item(NAME(m_OPTION));
	save_item(NAME(m_CONFIG));
	save_item(NAME(m_ALU));
	save_item(NAME(m_WDT));
	save_item(NAME(m_TMR0));
	save_item(NAME(m_STATUS));
	save_item(NAME(m_FSR));
	save_item(NAME(m_port_data));
	if (!is_nmospic)
		save_item(NAME(m_port_tris));
	save_item(NAME(m_STACK));
	save_item(NAME(m_prescaler));
	save_item(NAME(m_opcode.w));

	save_item(NAME(m_delay_timer));
	save_item(NAME(m_temp_config));
	save_item(NAME(m_rtcc));
	save_item(NAME(m_count_cycles));
	save_item(NAME(m_inst_cycles));

	// debugger
	state_add( PIC16C5x_PC,   "PC",   m_PC).mask(0xfff).formatstr("%03X");
	state_add( PIC16C5x_W,    "W",    m_W).formatstr("%02X");
	state_add( PIC16C5x_ALU,  "ALU",  m_ALU).formatstr("%02X");
	state_add( PIC16C5x_STR,  "STR",  m_debugger_temp).mask(0xff).callimport().callexport().formatstr("%02X");
	state_add( PIC16C5x_TMR0, "TMR",  m_debugger_temp).mask(0xff).callimport().callexport().formatstr("%02X");
	state_add( PIC16C5x_WDT,  "WDT",  m_WDT).formatstr("%04X");
	state_add( PIC16C5x_OPT,  "OPT",  m_OPTION).formatstr("%02X");
	state_add( PIC16C5x_STK0, "STK0", m_STACK[0]).mask(0xfff).formatstr("%03X");
	state_add( PIC16C5x_STK1, "STK1", m_STACK[1]).mask(0xfff).formatstr("%03X");
	if (m_picmodel != 0x1655)
		state_add( PIC16C5x_PRTA, "PRTA", m_debugger_temp).mask(0xff).callimport().callexport().formatstr("%02X");
	state_add( PIC16C5x_PRTB, "PRTB", m_debugger_temp).mask(0xff).callimport().callexport().formatstr("%02X");
	if (m_picmodel != 0x1654 && m_picmodel != 0x16C54 && m_picmodel != 0x16C56 && m_picmodel != 0x16C58)
		state_add( PIC16C5x_PRTC, "PRTC", m_debugger_temp).mask(0xff).callimport().callexport().formatstr("%02X");
	if (m_picmodel == 0x1650)
		state_add( PIC16C5x_PRTD, "PRTD", m_debugger_temp).mask(0xff).callimport().callexport().formatstr("%02X");
	if (!is_nmospic) {
		state_add( PIC16C5x_TRSA, "TRSA", m_port_tris[PORTA]).mask(0xf).formatstr("%01X");
		state_add( PIC16C5x_TRSB, "TRSB", m_port_tris[PORTB]).formatstr("%02X");
		if (m_picmodel != 0x16C54 && m_picmodel != 0x16C56 && m_picmodel != 0x16C58)
			state_add( PIC16C5x_TRSC, "TRSC", m_port_tris[PORTC]).formatstr("%02X");
	}
	state_add( PIC16C5x_FSR,  "FSR",  m_debugger_temp).mask(m_data_mask).formatstr("%02X");
	state_add( PIC16C5x_PSCL, "PSCL", m_debugger_temp).callimport().formatstr("%3s");

	state_add( STATE_GENPC, "GENPC", m_PC).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_PREVPC).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_OPTION).formatstr("%13s").noshow();

	set_icountptr(m_icount);
}


void pic16c5x_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case PIC16C5x_STR:
			m_STATUS = m_debugger_temp | u8(~m_status_mask);
			break;
		case PIC16C5x_TMR0:
			m_TMR0 = m_debugger_temp;
			break;
		case PIC16C5x_PRTA:
			m_port_data[PORTA] = m_debugger_temp;
			break;
		case PIC16C5x_PRTB:
			m_port_data[PORTB] = m_debugger_temp;
			break;
		case PIC16C5x_PRTC:
			m_port_data[PORTC] = m_debugger_temp;
			break;
		case PIC16C5x_PRTD:
			m_port_data[PORTD] = m_debugger_temp;
			break;
		case PIC16C5x_PSCL:
			m_prescaler = m_debugger_temp;
			break;
	}
}

void pic16c5x_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case PIC16C5x_STR:
			m_debugger_temp = m_STATUS | u8(~m_status_mask);
			break;
		case PIC16C5x_TMR0:
			m_debugger_temp = m_TMR0;
			break;
		case PIC16C5x_PRTA:
			m_debugger_temp = m_port_data[PORTA];
			break;
		case PIC16C5x_PRTB:
			m_debugger_temp = m_port_data[PORTB];
			break;
		case PIC16C5x_PRTC:
			m_debugger_temp = m_port_data[PORTC];
			break;
		case PIC16C5x_PRTD:
			m_debugger_temp = m_port_data[PORTD];
			break;
	}
}

void pic16c5x_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case PIC16C5x_PSCL:
			str = string_format("%c%02X", ((m_OPTION & 0x08) ? 'W' : 'T'), m_prescaler);
			break;

		case STATE_GENFLAGS:
			str = string_format("%01x%c%c%c%c%c %c%c%c%03x",
				(m_STATUS & 0xe0) >> 5,
				m_STATUS & 0x10 ? '.':'O', // WDT Overflow
				m_STATUS & 0x08 ? 'P':'D', // Power/Down
				m_STATUS & 0x04 ? 'Z':'.', // Zero
				m_STATUS & 0x02 ? 'c':'b', // Nibble Carry/Borrow
				m_STATUS & 0x01 ? 'C':'B', // Carry/Borrow

				m_OPTION & 0x20 ? 'C':'T', // Counter/Timer
				m_OPTION & 0x10 ? 'N':'P', // Negative/Positive
				m_OPTION & 0x08 ? 'W':'T', // WatchDog/Timer
				m_OPTION & 0x08 ? (1<<(m_OPTION&7)) : (2<<(m_OPTION&7)));
			break;
	}
}


/****************************************************************************
 *  Reset registers to their initial values
 ****************************************************************************/

void pic16c5x_device::reset_regs()
{
	m_CONFIG = m_temp_config;
	m_port_tris[PORTA] = 0xff;
	m_port_tris[PORTB] = 0xff;
	m_port_tris[PORTC] = 0xff;
	m_OPTION = T0CS_FLAG | T0SE_FLAG | PSA_FLAG | PS_REG;
	set_pc(m_program_mask);
	m_PREVPC = m_PC;

	m_prescaler = 0;
	m_delay_timer = 0;
	m_inst_cycles = 0;
	m_count_cycles = 0;
}

void pic16c5x_device::watchdog_reset()
{
	SET(m_STATUS, TO_FLAG | PD_FLAG | Z_FLAG | DC_FLAG | C_FLAG);
	reset_regs();
}

void pic16c5x_device::set_config(u16 data)
{
	logerror("Writing %04x to the PIC16C5x config register\n", data);
	m_temp_config = data;
}


void pic16c5x_device::device_reset()
{
	reset_regs();
	CLR(m_STATUS, PA_REG);
	SET(m_STATUS, TO_FLAG | PD_FLAG);
}


/****************************************************************************
 *  WatchDog
 ****************************************************************************/

void pic16c5x_device::update_watchdog(int counts)
{
	/*
	WatchDog is set up to count 18,000 (0x464f hex) ticks to provide
	the timeout period of 0.018ms based on a 4MHz input clock.
	Note: the 4MHz clock should be divided by the PIC16C5x_CLOCK_DIVIDER
	which effectively makes the PIC run at 1MHz internally.

	If the current instruction is CLRWDT or SLEEP, don't update the WDT
	*/

	if ((m_opcode.w != 3) && (m_opcode.w != 4)) {
		u16 old_WDT = m_WDT;

		m_WDT -= counts;

		if (m_WDT > 0x464f) {
			m_WDT = 0x464f - (0xffff - m_WDT);
		}

		if (((old_WDT != 0) && (old_WDT < m_WDT)) || (m_WDT == 0)) {
			if (PSA) {
				m_prescaler++;
				if (m_prescaler >= (1 << PS)) { // Prescale values from 1 to 128
					m_prescaler = 0;
					CLR(m_STATUS, TO_FLAG);
					watchdog_reset();
				}
			}
			else {
				CLR(m_STATUS, TO_FLAG);
				watchdog_reset();
			}
		}
	}
}


/****************************************************************************
 *  Update Timer
 ****************************************************************************/

void pic16c5x_device::update_timer(int counts)
{
	if (m_delay_timer > 0) { // Timer increment is inhibited
		int dt = m_delay_timer;
		m_delay_timer -= m_inst_cycles;
		counts -= dt;
	}

	if (m_delay_timer > 0 || counts <= 0)
		return;

	if (PSA == 0) {
		m_prescaler += counts;
		if (m_prescaler >= (2 << PS)) { // Prescale values from 2 to 256
			m_TMR0 += (m_prescaler / (2 << PS));
			m_prescaler %= (2 << PS); // Overflow prescaler
		}
	}
	else {
		m_TMR0 += counts;
	}
}

void pic16c5x_device::execute_set_input(int line, int state)
{
	switch (line)
	{
		// RTCC/T0CKI pin
		case PIC16C5x_RTCC:
			if (T0CS && state != m_rtcc) { // Count mode, edge triggered
				if ((T0SE && !state) || (!T0SE && state))
					m_count_cycles++;
			}
			m_rtcc = state;
			break;

		default:
			break;
	}
}


/****************************************************************************
 *  Execute IPeriod. Return 0 if emulation should be stopped
 ****************************************************************************/

void pic16c5x_device::execute_run()
{
	do {
		if (PD == 0) { // Sleep Mode
			m_count_cycles = 0;
			m_inst_cycles = 1;
			debugger_instruction_hook(m_PC);
		}
		else {
			m_PREVPC = m_PC;

			debugger_instruction_hook(m_PC);

			m_opcode.w = m_program.read_word(m_PC);
			set_pc(m_PC + 1);

			if (m_picmodel == 0x1650 || m_picmodel == 0x1654 || m_picmodel == 0x1655 || (m_opcode.w & 0xff0) != 0x000) { // Do all opcodes except the 00? ones
				m_inst_cycles = s_opcode_main[((m_opcode.w >> 4) & 0xff)].cycles;
				(this->*s_opcode_main[((m_opcode.w >> 4) & 0xff)].function)();
			}
			else { // Opcode 0x00? has many opcodes in its minor nibble
				m_inst_cycles = s_opcode_00x[(m_opcode.b.l & 0x1f)].cycles;
				(this->*s_opcode_00x[(m_opcode.b.l & 0x1f)].function)();
			}

			update_timer(T0CS ? m_count_cycles : m_inst_cycles);
			m_count_cycles = 0;
		}

		if (WDTE) {
			update_watchdog(m_inst_cycles);
		}

		m_icount -= m_inst_cycles;
	} while (m_icount > 0);
}
