// license:BSD-3-Clause
// copyright-holders:Grull Osgo
/****************************************************************************************

  Microchip PIC16x8x Emulator

  https://ww1.microchip.com/downloads/en/DeviceDoc/30445D.pdf
  
  Based on MAME's PIC16C5x/62x cpu devices developed by Tony La Porta
  and improvements to SFR's accesss made by ajrhacker.

  Preliminary version to give support to magicle and other games
  on misc/magicard.cpp driver, providing some protection level via I2C bus.

  Coded and partially tested:
    - All instruction set and disassembler.
    - I/O Ports.
    - WDT.
    - Timer/Counter.
    - All (4) interrupt sources.
    - Input lines.
    - Internal EEPROM with default and NVRAM functionality.

 ToDo:
    - Verify if data ram addreess map mirroring is properly coded acording to datasheet description.

    - Choose wich is the best option to set the config word, if via set instruction or from dump data or some sort of combination via default options.

	- Improve the debug section.
	
	- Verify eeprom write sequence (see datashhet) 

  Improvements:
    - SFR's Flag description (bit oriented instructions) in disassembler where possible.

****************************************************************************************/

#include "emu.h"
#include "pic16x8x.h"
#include "16x8xdsm.h"

DEFINE_DEVICE_TYPE(PIC16CR83, pic16cr83_device, "pic16cr83", "Microchip PIC16CR83")
DEFINE_DEVICE_TYPE(PIC16CR84, pic16cr84_device, "pic16cr84", "Microchip PIC16CR84")
DEFINE_DEVICE_TYPE(PIC16F83,  pic16f83_device,  "pic16f83",  "Microchip PIC16F83")
DEFINE_DEVICE_TYPE(PIC16F84,  pic16f84_device,  "pic16f84",  "Microchip PIC16F84")
DEFINE_DEVICE_TYPE(PIC16F84A, pic16f84a_device, "pic16f84a", "Microchip PIC16F84A")

/****************************************************************************
 *  Internal Memory Maps
 ****************************************************************************/

void pic16x8x_device::rom_9(address_map &map)
{
	map(0x000, 0x1ff).rom();
}

void pic16x8x_device::rom_10(address_map &map)
{
	map(0x000, 0x3ff).rom();
}


/* Notes from datasheet:
 The address depends on the device used.
Devices with 36 bytes ends at 2Fh, devices with
68 bytes ends at 4Fh.*/

void pic16x8x_device::core_regs(address_map &map, u8 mirror)
{
	map(0x00, 0x00).noprw().mirror(mirror); // Not an actual register, reading indirectly (FSR=0) returns 0
	map(0x01, 0x01).rw(FUNC(pic16x8x_device::tmr0_r), FUNC(pic16x8x_device::tmr0_w));
	map(0x02, 0x02).rw(FUNC(pic16x8x_device::pcl_r), FUNC(pic16x8x_device::pcl_w)).mirror(mirror);
	map(0x03, 0x03).rw(FUNC(pic16x8x_device::status_r), FUNC(pic16x8x_device::status_w)).mirror(mirror);
	map(0x04, 0x04).rw(FUNC(pic16x8x_device::fsr_r), FUNC(pic16x8x_device::fsr_w)).mirror(mirror);
	map(0x05, 0x05).rw(FUNC(pic16x8x_device::porta_r), FUNC(pic16x8x_device::porta_w));
	map(0x06, 0x06).rw(FUNC(pic16x8x_device::portb_r), FUNC(pic16x8x_device::portb_w));
	map(0x07, 0x07).noprw().mirror(mirror); // Not an actual register, returns 0
	map(0x08, 0x08).rw(FUNC(pic16x8x_device::eedata_r), FUNC(pic16x8x_device::eedata_w));
	map(0x09, 0x09).rw(FUNC(pic16x8x_device::eeadr_r), FUNC(pic16x8x_device::eeadr_w));
	map(0x0a, 0x0a).rw(FUNC(pic16x8x_device::pclath_r), FUNC(pic16x8x_device::pclath_w)).mirror(mirror);
	map(0x0b, 0x0b).rw(FUNC(pic16x8x_device::intcon_r), FUNC(pic16x8x_device::intcon_w)).mirror(mirror);
	map(0x81, 0x81).rw(FUNC(pic16x8x_device::option_r), FUNC(pic16x8x_device::option_w));
	map(0x85, 0x85).rw(FUNC(pic16x8x_device::trisa_r), FUNC(pic16x8x_device::trisa_w));
	map(0x86, 0x86).rw(FUNC(pic16x8x_device::trisb_r), FUNC(pic16x8x_device::trisb_w));
	map(0x88, 0x88).rw(FUNC(pic16x8x_device::eecon1_r), FUNC(pic16x8x_device::eecon1_w));
	map(0x89, 0x89).rw(FUNC(pic16x8x_device::eecon2_r), FUNC(pic16x8x_device::eecon2_w));
}

void pic16x8x_device::ram_6(address_map &map)
{
	// 0x00 - 0x0b SFR's Bank 0
	// 0x0c - 0x2f GPR's
	// 0x80 - 0x8b SFR's Bank 1
	// 0x8c - 0xaf GPR Mirrored to 0x0c - 0x2f
	core_regs(map, 0x80);
	map(0x0c, 0x2f).ram().mirror(0x80);
}

void pic16x8x_device::ram_7(address_map &map)
{
	// 0x00 - 0x0b SFR's Bank 0
	// 0x0c - 0x4f GPR's
	// 0x80 - 0x8b SFR's Bank 1
	// 0x8c - 0xcf GPR Mirrored to 0x0c - 0x4f
	core_regs(map, 0x80);
	map(0x0c, 0x4f).ram().mirror(0x80);
}


pic16x8x_device::pic16x8x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, address_map_constructor program_map, address_map_constructor data_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_region(*this, DEVICE_SELF)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, program_width, -1, program_map)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 8, 0, data_map)
	, m_program_width(program_width)
	, m_CONFIG(0x3fff)
	, m_read_port(*this, 0)
	, m_write_port(*this)
{
}

pic16x83_device::pic16x83_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: pic16x8x_device(mconfig, type, tag, owner, clock, 9, address_map_constructor(FUNC(pic16x8x_device::rom_9), this), address_map_constructor(FUNC(pic16x8x_device::ram_6), this))
{
}

pic16x84_device::pic16x84_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: pic16x8x_device(mconfig, type, tag, owner, clock, 10, address_map_constructor(FUNC(pic16x8x_device::rom_10), this), address_map_constructor(FUNC(pic16x8x_device::ram_7), this))
{
}

pic16cr83_device::pic16cr83_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16x83_device(mconfig, PIC16CR83, tag, owner, clock)
{
}

pic16cr84_device::pic16cr84_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16x84_device(mconfig, PIC16CR84, tag, owner, clock)
{
}

pic16f83_device::pic16f83_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16x83_device(mconfig, PIC16F83, tag, owner, clock)
{
}

pic16f84_device::pic16f84_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16x84_device(mconfig, PIC16F84, tag, owner, clock)
{
}

pic16f84a_device::pic16f84a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pic16x84_device(mconfig, PIC16F84A, tag, owner, clock)
{
}

device_memory_interface::space_config_vector pic16x8x_device::memory_space_config() const
{
	return space_config_vector
	{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

std::unique_ptr<util::disasm_interface> pic16x8x_device::create_disassembler()
{
	return std::make_unique<pic16x8x_disassembler>();
}


/* Notes about NVRAM / PIC eeprom data

Dump size is 4280 Bytes, 16 bits wide.
0000h-03ffh is program data
0400h-3fffh is dummy data
4000h-400fh is user and config data
4010h-41ffh is dummy data
4200h-4280h is eeprom data

NVRAM saved data is 8 bits wide, in accordance with the data width
of the EEPROM memory of the PIC.
This means that we need to perform the conversion from the default
NVRAM dumped data to the NVRAM saved data.
*/

void pic16x8x_device::nvram_default()
{
	u16 dump_size = 0x4280;
	u16 eeprom_dump = 0x4200;

	// populate from a memory region if present
	if (m_region.found())
	{
		if( m_region->bytes() != dump_size ) // pic memory dump total size
		{
			fatalerror( "Region '%s' wrong size (expected size = 0x%X)\n", tag(), dump_size );
		}

		if( m_region->bytewidth() != 2 ) // pic memory dumps are 16 bits wide
		{
			fatalerror( "Region '%s' needs to be an 16-bit region\n", tag() );
		}

		// Get default NVRAM data from memory region
		memcpy(m_buff, m_region->base() + eeprom_dump, m_internal_eeprom_size * 2);

		// Data width conversion
		for (u8 i=0; i < m_internal_eeprom_size; i++)
			m_eeprom_data[i] = m_buff[i] & 0x00ff;
	}

}

bool pic16x8x_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_eeprom_data, m_internal_eeprom_size);
	return !err && (actual == m_internal_eeprom_size);
}

bool pic16x8x_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_eeprom_data, m_internal_eeprom_size);
	return !err && (actual == m_internal_eeprom_size);
}


// EEPROM data access
u8 pic16x8x_device::m_eeread(offs_t offs)
{
	return m_eeprom_data[offs];
}

void pic16x8x_device::m_eewrite(offs_t offs, u8 data)
{
	m_eeprom_data[offs] = data & m_data_mask;
}


// ******** The following is the STATUS flag register definition. ********
// |  7  |  6  |  5  | 4 | 3 | 2 |  1 | 0 |
// | IRP | RP1 | RP0 | T0| PD| Z | DC | C |

// IRP Register Bank Select - Unimplemented on this device
constexpr u8 RP1_FLAG = 0x40;  // Register Bank Select Bit 1
constexpr u8 RP0_FLAG = 0x20;  // Register Bank Select Bit 0
							  // 00 = Bank 0 (00h-7fh)
							  // 01 = Bank 1 (80h-ffh)
							  // 10 = Bank 2 (100h-17fh)
							  // 11 = Bank 3 (180h-1ffh)
							  // Each bank is 128 bytes. Only bit RP0 is used by the PIC16F8X. RP1 should be mantained clear.
constexpr u8 TO_FLAG  = 0x10;  // TO   Time Out flag (WatchDog)
constexpr u8 PD_FLAG  = 0x08;  // PD   Power Down flag
constexpr u8 Z_FLAG   = 0x04;  // Z    Zero Flag
constexpr u8 DC_FLAG  = 0x02;  // DC   Digit Carry/Borrow flag (Nibble)
constexpr u8 C_FLAG   = 0x01;  // C    Carry/Borrow Flag (Byte)

#define RP0     (m_STATUS & RP0_FLAG)
#define TO      (m_STATUS & TO_FLAG)
#define PD      (m_STATUS & PD_FLAG)
#define ZERO    (m_STATUS & Z_FLAG)
#define DC      (m_STATUS & DC_FLAG)
#define CARRY   (m_STATUS & C_FLAG)


// ******** The following is the OPTION flag register definition. ********
// |   7  |    6   |   5  |   4  |  3  |   2 |   1 |   0 |
// | RBPU | INTEDG | T0CS | T0SE | PSA | PS2 | PS1 | PS0 |
constexpr u8 RBPU_FLAG   = 0x80;  // RBPU PORTB Pull-up Enable bit
constexpr u8 INTEDG_FLAG = 0x40;  // INTEDG  Interrupt Edge Select bit
								 // 1 = Interrupt on rising edge of RB0/INT pin
								 // 0 = Interrupt on falling edge of RB0/INT pin
constexpr u8 T0CS_FLAG   = 0x20;  // T0CS Timer 0 clock source select
								 // 1 = Transition on RA4/T0CKI pin
								 // 0 = Internal instruction cycle clock (CLKOUT)
constexpr u8 T0SE_FLAG   = 0x10;  // T0SE Timer 0 clock source edge select
								 // 1 = Increment on high-to-low transition on RA4/T0CKI pin
								 // 0 = Increment on low-to-high transition on RA4/T0CKI pin
constexpr u8 PSA_FLAG    = 0x08;  // PSA Prescaler Assignment bit
								 // 1 = Prescaler is assigned to the WDT
								 // 0 = Prescaler is assigned to the Timer0 module
constexpr u8 PS_REG      = 0x07;  // PS Prescaler Rate select

#define RBPU    (m_OPTION & RBPU_FLAG)
#define INTEDG  (m_OPTION & INTEDG_FLAG)
#define T0CS    (m_OPTION & T0CS_FLAG)
#define T0SE    (m_OPTION & T0SE_FLAG)
#define PSA     (m_OPTION & PSA_FLAG)
#define PS      (m_OPTION & PS_REG)


// ******** The following is the CONFIG Flag register definition. ********
// | 11 | 10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 |   2  | 1 | 0 |
// |              CP                     | WDTE |  FOSC |
							// CP       Code Protect (ROM read protect)
constexpr u8 WDTE_FLAG  = 0x04;  // WDTE     WatchDog Timer enable
constexpr u8 FOSC_FLAG  = 0x03;  // FOSC     Oscillator source select

#define WDTE    (m_CONFIG & WDTE_FLAG)
#define FOSC    (m_CONFIG & FOSC_FLAG)


// ******** The following is the INTCON flag register definition. ********
// |   7 |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
// | GIE | EEIE | T0IE | INTE | RBIE | T0IF | INTF | RBIF |

constexpr u8 GIE_FLAG  = 0x80;
constexpr u8 EEIE_FLAG = 0x40;
constexpr u8 T0IE_FLAG = 0x20;
constexpr u8 INTE_FLAG = 0x10;
constexpr u8 RBIE_FLAG = 0x08;
constexpr u8 T0IF_FLAG = 0x04;
constexpr u8 INTF_FLAG = 0x02;
constexpr u8 RBIF_FLAG = 0x01;

#define GIE   (m_INTCON & GIE_FLAG)
#define EEIE  (m_INTCON & EEIE_FLAG)
#define T0IE  (m_INTCON & T0IE_FLAG)
#define INTE  (m_INTCON & INTE_FLAG)
#define RBIE  (m_INTCON & RBIE_FLAG)
#define T0IF  (m_INTCON & T0IF_FLAG)
#define INTF  (m_INTCON & INTF_FLAG)
#define RBIF  (m_INTCON & RBIF_FLAG)

// ******** The following is the EECON1 Flag register definition. ********
// | 7 | 6 | 5 |   4  |   3   |   2  |  1 |  0 |
// |   |   |   | EEIF | WRERR | WREN | WR | RD |

constexpr u8 EEIF_FLAG  = 0x10;
constexpr u8 WRERR_FLAG = 0x08;
constexpr u8 WREN_FLAG  = 0x04;
constexpr u8 EEWR_FLAG  = 0x02;
constexpr u8 EERD_FLAG  = 0x01;

#define EEIF  (m_EECON1 & EEIF_FLAG)
#define WRERR (m_EECON1 & WRERR_FLAG)
#define WREN  (m_EECON1 & WREN_FLAG)
#define EEWR  (m_EECON1 & EEWR_FLAG)
#define EERD  (m_EECON1 & EERD_FLAG)

/************************************************************************
 *  Fixed Vectors
 ************************************************************************/

constexpr u8 RESET_VECTOR = 0x00;
constexpr u8 INT_VECTOR   = 0x04;

/************************************************************************
 *  Shortcuts
 ************************************************************************/

#define RISING_EDGE_RB0   (( (int)(RB0_in - m_old_RB0) > 0) ? 1 : 0)
#define FALLING_EDGE_RB0  (( (int)(RB0_in - m_old_RB0) < 0) ? 1 : 0)

#define M_RDEEPROM(A)     m_eeread(A)
#define M_WREEPROM(A,V)   m_eewrite(A, V)

#define CLR(flagreg, flag)  (flagreg &= u8(~flag))
#define SET(flagreg, flag)  (flagreg |= (flag))

#define ADDR  ((m_opcode.b.l & 0x7f) | (RP0 << 2))
#define BITPOS  ((m_opcode.w >> 7) & 7)


void pic16x8x_device::calc_zero_flag()
{
	if (m_ALU == 0)
		SET(m_STATUS, Z_FLAG);
	else
		CLR(m_STATUS, Z_FLAG);
}

void pic16x8x_device::calc_add_flags(u8 augend)
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

void pic16x8x_device::calc_sub_flags(u8 minuend)
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

void pic16x8x_device::set_pc(u16 addr)
{
	m_PCL = addr & m_program_mask;
}

/*
Notes about Stack (from datasheet)
After the stack has been PUSHed eight times, the ninth
push overwrites the value that was stored from the first
push. The tenth push overwrites the second push (and
so on)
*/

u16 pic16x8x_device::pop_stack()
{
	m_stack_pointer = (m_stack_pointer - 1) & 0x0f;
	u16 data = m_STACK[m_stack_pointer];
	return data & m_program_mask;
}

void pic16x8x_device::push_stack(u16 data)
{
	m_STACK[m_stack_pointer] = data & m_program_mask;
	m_stack_pointer = (m_stack_pointer + 1) & 0x0f;
}

void pic16x8x_device::store_result(u8 addr, u8 data)
{
	if (m_opcode.b.l & 0x80)
		store_regfile(addr, data);
	else
		m_W = data;

}

// *** Special function registers (SFR's) ***

// ***            SFR Access              ***
u8 pic16x8x_device::get_regfile(u8 addr)
{
	if (addr == 0)
	{ // Indirect addressing
		addr = m_FSR & m_data_mask;
	}
	return m_data.read_byte(addr);
}

void pic16x8x_device::store_regfile(u8 addr, u8 data)
{
	if (addr == 0)
	{ // Indirect addressing
		addr = m_FSR & m_data_mask;
	}
	m_data.write_byte(addr, data);
}

// ***            SFR Functions          ***
u8 pic16x8x_device::tmr0_r()
{
	return m_TMR0;
}

void pic16x8x_device::tmr0_w(u8 data)
{
	m_delay_timer = 2; // Timer increment is inhibited for 2 cycles
	if (PSA == 0) m_prescaler = 0; // Must clear the Prescaler
	m_TMR0 = data;
}

u8 pic16x8x_device::pcl_r()
{
	return m_PCL;
}

void pic16x8x_device::pcl_w(u8 data)
{
	m_PCL = data;
	set_pc((m_PCLATH << 8) | data);
	m_inst_cycles++;
}

u8 pic16x8x_device::status_r()
{
	return m_STATUS;
}

void pic16x8x_device::status_w(u8 data)
{
	m_STATUS = (m_STATUS & (TO_FLAG | PD_FLAG)) | (data & u8(~(TO_FLAG | PD_FLAG)));
}

u8 pic16x8x_device::fsr_r()
{
	return m_FSR | (uint8_t)(~m_data_mask);
}

void pic16x8x_device::fsr_w(u8 data)
{
	m_FSR = data | u8(~m_data_mask);
}

u8 pic16x8x_device::porta_r()
{
	u8 data = m_read_port[PORTA](PORTA, 0xff);
	data &= m_port_tris[PORTA];
	data |= (u8(~m_port_tris[PORTA]) & m_port_data[PORTA]);
	return data & 0x1f; // 5-bit port (only lower 5 bits used)
}

void pic16x8x_device::porta_w(u8 data)
{
	data &= 0x1f; // 5-bit port (only lower 5 bits used)
	m_write_port[PORTA](PORTA, data & u8(~m_port_tris[PORTA]) & 0x1f, u8(~m_port_tris[PORTA]) & 0x1f);
	m_port_data[PORTA] = data;
}

u8 pic16x8x_device::portb_r()
{
	u8 data = m_read_port[PORTB](PORTB, 0xff);
	data &= m_port_tris[PORTB];
	data |= (u8(~m_port_tris[PORTB]) & m_port_data[PORTB]);
	return  data;
}

void pic16x8x_device::portb_w(u8 data)
{
	m_write_port[PORTB](PORTB, data & u8(~m_port_tris[PORTB]), u8(~m_port_tris[PORTB]));
	m_port_data[PORTB] = data;
}

u8 pic16x8x_device::eedata_r()
{
	return m_EEDATA;
}

void pic16x8x_device::eedata_w(u8 data)
{
	m_EEDATA = data;
}

u8 pic16x8x_device::eeadr_r()
{
	return m_EEADR;
}

void pic16x8x_device::eeadr_w(u8 data)
{
	m_EEADR = data;
}

u8 pic16x8x_device::pclath_r()
{
	return m_PCLATH;
}

void pic16x8x_device::pclath_w(u8 data)
{
	m_PCLATH = data;
}

u8 pic16x8x_device::intcon_r()
{
	return m_INTCON;
}

void pic16x8x_device::intcon_w(u8 data)
{
	m_INTCON = data;
}

u8 pic16x8x_device::trisa_r()
{
	return m_port_tris[PORTA];
}

void pic16x8x_device::trisa_w(u8 data)
{
	if (m_port_tris[PORTA] != data)
	{
		m_port_tris[PORTA] = data | 0xe0;
		m_write_port[PORTA](PORTA, m_port_data[PORTA] & u8(~m_port_tris[PORTA]) & 0x1f, u8(~m_port_tris[PORTA]) & 0x1f);
	}
}

u8 pic16x8x_device::trisb_r()
{
	return m_port_tris[PORTB];
}

void pic16x8x_device::trisb_w(u8 data)
{
	if (m_port_tris[PORTB] != data)
	{
		m_port_tris[PORTB] = data;
		m_write_port[PORTB](PORTB, m_port_data[PORTB] & u8(~m_port_tris[PORTB]), u8(~m_port_tris[PORTB]));
	}
}

u8 pic16x8x_device::eecon1_r()
{
	return  m_EECON1;
}

void pic16x8x_device::eecon1_w(u8 data)
{
	if (!((data & WREN_FLAG) && EEWR))  // The WR bit will be inhibited from being set unless the WREN bit is set.(from datasheet)
		CLR(data, EEWR);                // Clear the flag if it don't meet the requirements

	m_EECON1 = data;                    // update register

	if (EERD)                           // cheack flag condition to read eeprom
	{
		m_EEDATA = M_RDEEPROM(m_EEADR); // do eeprom read
		CLR(m_EECON1, EERD_FLAG);       // clear read enable flag
	}

	if (WREN && EEWR && (m_EECON2 == 0xff)) // check flags conditions to write eeprom
	{
		M_WREEPROM(m_EEADR, data);          // do eeprom write
		CLR(m_EECON1, EEWR_FLAG);           // clear write enable flag
		SET(m_EECON1, EEIF_FLAG);           // assuming write was done, set EEIF
	}
}

u8 pic16x8x_device::eecon2_r()
{
	return m_EECON2;
}

void pic16x8x_device::eecon2_w(u8 data)
{
	if((m_EECON2!=0x55) & (data==0x55))
		m_EECON2 = data;
	else if((m_EECON2==0x55) & (data==0xAA))
		m_EECON2 = 0xff;
	else
		m_EECON2 = data;
}

u8 pic16x8x_device::option_r()
{
	return m_OPTION;
}

void pic16x8x_device::option_w(u8 data)
{
	m_OPTION = data;
}

/************************************************************************
 *  Emulate the Instructions
 ************************************************************************/

void pic16x8x_device::illegal()
{
	logerror("PIC16x8x: PC=%03x, Illegal opcode = %04x\n", m_PREVPC, m_opcode.w);
}

/*
    Note:
    According to the manual, if the STATUS register is the destination for an instruction that affects the Z, DC or C bits
    then the write to these three bits is disabled. These bits are set or cleared according to the device logic.
    To ensure this is correctly emulated, in instructions that write to the file registers, always change the status flags
    *after* storing the result of the instruction.
    e.g. CALCULATE_*, SET(STATUS,*_FLAG) and CLR(STATUS,*_FLAG) should appear as the last steps of the instruction emulation.
*/

void pic16x8x_device::addlw()
{
	// New
	m_ALU = m_opcode.b.l + m_W;
	m_W = m_ALU;
	calc_zero_flag();
}

void pic16x8x_device::addwf()
{
	u8 augend = get_regfile(ADDR);
	m_ALU = augend + m_W;
	store_result(ADDR, m_ALU);
	calc_add_flags(augend);
}

void pic16x8x_device::andlw()
{
	m_ALU = m_opcode.b.l & m_W;
	m_W = m_ALU;
	calc_zero_flag();
}

void pic16x8x_device::andwf()
{
	m_ALU = get_regfile(ADDR) & m_W;
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}

void pic16x8x_device::bcf()
{
	m_ALU = get_regfile(ADDR);
	m_ALU &= ~(1 << BITPOS);
	store_regfile(ADDR, m_ALU);
}

void pic16x8x_device::bsf()
{
	m_ALU = get_regfile(ADDR);
	m_ALU |= 1 << BITPOS;
	store_regfile(ADDR, m_ALU);
}

void pic16x8x_device::btfss()
{
	if (BIT(get_regfile(ADDR), BITPOS))
	{
		set_pc(m_PCL + 1);
		m_inst_cycles++; // Add NOP cycles
	}
}

void pic16x8x_device::btfsc()
{
	if (!BIT(get_regfile(ADDR), BITPOS))
	{
		set_pc(m_PCL + 1);
		m_inst_cycles++; // Add NOP cycles
	}
}

void pic16x8x_device::call()
{
	push_stack(m_PCL);
	set_pc(((m_PCLATH & 0x18) << 8 ) | (m_opcode.w & 0x07ff));
}


void pic16x8x_device::clrw()
{
	m_W = 0;
	SET(m_STATUS, Z_FLAG);
}

void pic16x8x_device::clrf()
{
	store_regfile(ADDR, 0);
	SET(m_STATUS, Z_FLAG);
}

void pic16x8x_device::clrwdt()
{
	m_WDT = 0;
	if (PSA) m_prescaler = 0;
	SET(m_STATUS, TO_FLAG);
	SET(m_STATUS, PD_FLAG);
}

void pic16x8x_device::comf()
{
	m_ALU = u8(~(get_regfile(ADDR)));
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}

void pic16x8x_device::decf()
{
	m_ALU = get_regfile(ADDR) - 1;
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}

void pic16x8x_device::decfsz()
{
	m_ALU = get_regfile(ADDR) - 1;
	store_result(ADDR, m_ALU);
	if (m_ALU == 0)
	{
		set_pc(m_PCL + 1);
		m_inst_cycles++; // Add NOP cycles
	}
}

void pic16x8x_device::goto_op()
{
	set_pc(((m_PCLATH & 0x18) << 8 ) | (m_opcode.w & 0x07ff));
}

void pic16x8x_device::incf()
{
	m_ALU = get_regfile(ADDR) + 1;
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}

void pic16x8x_device::incfsz()
{
	m_ALU = get_regfile(ADDR) + 1;
	store_result(ADDR, m_ALU);
	if (m_ALU == 0)
	{
		set_pc(m_PCL + 1);
		m_inst_cycles++; // Add NOP cycles
	}
}

void pic16x8x_device::iorlw()
{
	m_ALU = m_opcode.b.l | m_W;
	m_W = m_ALU;
	calc_zero_flag();
}

void pic16x8x_device::iorwf()
{
	m_ALU = get_regfile(ADDR) | m_W;
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}

void pic16x8x_device::movf()
{
	m_ALU = get_regfile(ADDR);
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}

void pic16x8x_device::movlw()
{
	m_W = m_opcode.b.l;
}

void pic16x8x_device::movwf()
{
	store_regfile(ADDR, m_W);
}

void pic16x8x_device::nop()
{
	// Do nothing
}

void pic16x8x_device::retfie()  // new for 16x8x
{
	// New
	set_pc(pop_stack());
	m_irq_in_progress = false;
}

void pic16x8x_device::retlw()
{
	m_W = m_opcode.b.l;
	set_pc(pop_stack());
}

void pic16x8x_device::retrn()   // new for 16x8x
{
	set_pc(pop_stack());
}

void pic16x8x_device::rlf()
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

void pic16x8x_device::rrf()
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

void pic16x8x_device::sleepic()
{
	if (WDTE) m_WDT = 0;
	if (PSA) m_prescaler = 0;
	SET(m_STATUS, TO_FLAG);
	CLR(m_STATUS, PD_FLAG);
}

void pic16x8x_device::sublw()   // new for 16x8x : Substract W from literal
{
	// New
	u8 minuend = m_opcode.b.l;
	m_ALU = minuend - m_W;
	m_W = m_ALU;
	calc_sub_flags(minuend);
}

void pic16x8x_device::subwf()  // Substract W from f
{
	u8 minuend = get_regfile(ADDR);
	m_ALU = minuend - m_W;
	store_result(ADDR, m_ALU);
	calc_sub_flags(minuend);
}

void pic16x8x_device::swapf()
{
	u8 reg = get_regfile(ADDR);
	m_ALU = reg << 4 | reg >> 4;
	store_result(ADDR, m_ALU);
}

void pic16x8x_device::xorlw()
{
	m_ALU = m_W ^ m_opcode.b.l;
	m_W = m_ALU;
	calc_zero_flag();
}

void pic16x8x_device::xorwf()
{
	m_ALU = get_regfile(ADDR) ^ m_W;
	store_result(ADDR, m_ALU);
	calc_zero_flag();
}


/***********************************************************************
 *  Opcode Table (Cycles, Instruction)
 ***********************************************************************/

const pic16x8x_device::pic16x8x_opcode pic16x8x_device::s_opcode_main[128]=
{
	{1, &pic16x8x_device::nop     },{1, &pic16x8x_device::movwf   },{1, &pic16x8x_device::clrw    },{1, &pic16x8x_device::clrf    }, // 00
	{1, &pic16x8x_device::subwf   },{1, &pic16x8x_device::subwf   },{1, &pic16x8x_device::decf    },{1, &pic16x8x_device::decf    }, // 04
	{1, &pic16x8x_device::iorwf   },{1, &pic16x8x_device::iorwf   },{1, &pic16x8x_device::andwf   },{1, &pic16x8x_device::andwf   }, // 08
	{1, &pic16x8x_device::xorwf   },{1, &pic16x8x_device::xorwf   },{1, &pic16x8x_device::addwf   },{1, &pic16x8x_device::addwf   }, // 0C
	{1, &pic16x8x_device::movf    },{1, &pic16x8x_device::movf    },{1, &pic16x8x_device::comf    },{1, &pic16x8x_device::comf    }, // 10
	{1, &pic16x8x_device::incf    },{1, &pic16x8x_device::incf    },{1, &pic16x8x_device::decfsz  },{1, &pic16x8x_device::decfsz  }, // 14
	{1, &pic16x8x_device::rrf     },{1, &pic16x8x_device::rrf     },{1, &pic16x8x_device::rlf     },{1, &pic16x8x_device::rlf     }, // 18
	{1, &pic16x8x_device::swapf   },{1, &pic16x8x_device::swapf   },{1, &pic16x8x_device::incfsz  },{1, &pic16x8x_device::incfsz  }, // 1C
	{1, &pic16x8x_device::bcf     },{1, &pic16x8x_device::bcf     },{1, &pic16x8x_device::bcf     },{1, &pic16x8x_device::bcf     }, // 20
	{1, &pic16x8x_device::bcf     },{1, &pic16x8x_device::bcf     },{1, &pic16x8x_device::bcf     },{1, &pic16x8x_device::bcf     },
	{1, &pic16x8x_device::bsf     },{1, &pic16x8x_device::bsf     },{1, &pic16x8x_device::bsf     },{1, &pic16x8x_device::bsf     }, // 28
	{1, &pic16x8x_device::bsf     },{1, &pic16x8x_device::bsf     },{1, &pic16x8x_device::bsf     },{1, &pic16x8x_device::bsf     },
	{1, &pic16x8x_device::btfsc   },{1, &pic16x8x_device::btfsc   },{1, &pic16x8x_device::btfsc   },{1, &pic16x8x_device::btfsc   }, // 30
	{1, &pic16x8x_device::btfsc   },{1, &pic16x8x_device::btfsc   },{1, &pic16x8x_device::btfsc   },{1, &pic16x8x_device::btfsc   },
	{1, &pic16x8x_device::btfss   },{1, &pic16x8x_device::btfss   },{1, &pic16x8x_device::btfss   },{1, &pic16x8x_device::btfss   }, // 38
	{1, &pic16x8x_device::btfss   },{1, &pic16x8x_device::btfss   },{1, &pic16x8x_device::btfss   },{1, &pic16x8x_device::btfss   },
	{2, &pic16x8x_device::call    },{2, &pic16x8x_device::call    },{2, &pic16x8x_device::call    },{2, &pic16x8x_device::call    }, // 40
	{2, &pic16x8x_device::call    },{2, &pic16x8x_device::call    },{2, &pic16x8x_device::call    },{2, &pic16x8x_device::call    },
	{2, &pic16x8x_device::call    },{2, &pic16x8x_device::call    },{2, &pic16x8x_device::call    },{2, &pic16x8x_device::call    }, // 48
	{2, &pic16x8x_device::call    },{2, &pic16x8x_device::call    },{2, &pic16x8x_device::call    },{2, &pic16x8x_device::call    },
	{2, &pic16x8x_device::goto_op },{2, &pic16x8x_device::goto_op },{2, &pic16x8x_device::goto_op },{2, &pic16x8x_device::goto_op }, // 50
	{2, &pic16x8x_device::goto_op },{2, &pic16x8x_device::goto_op },{2, &pic16x8x_device::goto_op },{2, &pic16x8x_device::goto_op },
	{2, &pic16x8x_device::goto_op },{2, &pic16x8x_device::goto_op },{2, &pic16x8x_device::goto_op },{2, &pic16x8x_device::goto_op }, // 58
	{2, &pic16x8x_device::goto_op },{2, &pic16x8x_device::goto_op },{2, &pic16x8x_device::goto_op },{2, &pic16x8x_device::goto_op },
	{1, &pic16x8x_device::movlw   },{1, &pic16x8x_device::movlw   },{1, &pic16x8x_device::movlw   },{1, &pic16x8x_device::movlw   }, // 60
	{1, &pic16x8x_device::movlw   },{1, &pic16x8x_device::movlw   },{1, &pic16x8x_device::movlw   },{1, &pic16x8x_device::movlw   },
	{2, &pic16x8x_device::retlw   },{2, &pic16x8x_device::retlw   },{2, &pic16x8x_device::retlw   },{2, &pic16x8x_device::retlw   }, // 68
	{2, &pic16x8x_device::retlw   },{2, &pic16x8x_device::retlw   },{2, &pic16x8x_device::retlw   },{2, &pic16x8x_device::retlw   },
	{1, &pic16x8x_device::iorlw   },{1, &pic16x8x_device::iorlw   },{1, &pic16x8x_device::andlw   },{1, &pic16x8x_device::andlw   }, // 70
	{1, &pic16x8x_device::xorlw   },{1, &pic16x8x_device::xorlw   },{1, &pic16x8x_device::xorlw   },{1, &pic16x8x_device::xorlw   }, // 74
	{1, &pic16x8x_device::sublw   },{1, &pic16x8x_device::sublw   },{1, &pic16x8x_device::sublw   },{1, &pic16x8x_device::sublw   }, // 78
	{1, &pic16x8x_device::addlw   },{1, &pic16x8x_device::addlw   },{1, &pic16x8x_device::addlw   },{1, &pic16x8x_device::addlw   }  // 7C
};

const pic16x8x_device::pic16x8x_opcode pic16x8x_device::s_opcode_00x[128]=
{
	{1, &pic16x8x_device::nop     },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 00
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{2, &pic16x8x_device::retrn   },{2, &pic16x8x_device::retfie    },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 08
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 10
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 18
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::nop     },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 20
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 28
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 30
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 38
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::nop     },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 40
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 48
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 50
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 58
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::nop     },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::sleepic   }, // 60
	{1, &pic16x8x_device::clrwdt  },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 68
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 70
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }, // 78
	{1, &pic16x8x_device::illegal },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   },{1, &pic16x8x_device::illegal   }
};


/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

enum
{
	PIC16X8x_PC=1, PIC16X8x_W, PIC16X8x_ALU, PIC16X8x_WDT, PIC16X8x_PSCL, PIC16X8x_CONFIG
};

void pic16x8x_device::device_start()
{
	space(AS_PROGRAM).cache(m_program);
	space(AS_DATA).specific(m_data);

	m_program_mask = (1 << m_program_width) - 1;
	m_data_mask = 0xff;
	m_status_mask = 0xff;
	m_PCL = 0;
	m_PREVPC = 0;
	m_W = 0;
	m_OPTION = 0;
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
	m_stack_pointer = 0;
	m_debugger_temp = 0;
	m_portb_chdetect_temp = 0xff;
	m_irq_in_progress = false;

	// save states
	save_item(NAME(m_PCL));
	save_item(NAME(m_PREVPC));
	save_item(NAME(m_CONFIG));
	save_item(NAME(m_W));
	save_item(NAME(m_OPTION));
	save_item(NAME(m_ALU));
	save_item(NAME(m_WDT));
	save_item(NAME(m_TMR0));
	save_item(NAME(m_STATUS));
	save_item(NAME(m_FSR));
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_tris));
	save_item(NAME(m_EEDATA));
	save_item(NAME(m_EEADR));
	save_item(NAME(m_EECON1));
	save_item(NAME(m_EECON2));
	save_item(NAME(m_PCLATH));
	save_item(NAME(m_INTCON));
	save_item(NAME(m_STACK));
	save_item(NAME(m_prescaler));
	save_item(NAME(m_opcode.w));
	save_item(NAME(m_delay_timer));
	save_item(NAME(m_rtcc));
	save_item(NAME(m_count_cycles));
	save_item(NAME(m_inst_cycles));
	save_item(NAME(m_stack_pointer));
	save_item(NAME(m_old_RB0));
	save_item(NAME(m_irq_in_progress));
	save_item(NAME(m_eeprom_data));
	

	// debugger
	state_add( PIC16X8x_PC,   "PC",   m_PCL).mask(0xfff).formatstr("%03X");
	state_add( PIC16X8x_W,    "W",    m_W).formatstr("%02X");
	state_add( PIC16X8x_ALU,  "ALU",  m_ALU).formatstr("%02X");
	state_add( PIC16X8x_WDT,  "WDT",  m_WDT).formatstr("%04X");
	state_add( PIC16X8x_CONFIG,  "CNF",  m_CONFIG).formatstr("%04X");
	state_add( PIC16X8x_PSCL, "PSCL", m_debugger_temp).callimport().formatstr("%3s");

	state_add( STATE_GENPC, "GENPC", m_PCL).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_PREVPC).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_OPTION).formatstr("%13s").noshow();

	set_icountptr(m_icount);
}


void pic16x8x_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case PIC16X8x_PSCL:
			m_prescaler = m_debugger_temp;
			break;
	}
}

void pic16x8x_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
	}
}

void pic16x8x_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case PIC16X8x_PSCL:
			str = string_format("%c%02X", ((m_OPTION & 0x08) ? 'W' : 'T'), m_prescaler);
			break;

		case STATE_GENFLAGS:
			str = string_format("%01x%c%c%c%c%c %c%c%c%03x",
				(m_STATUS & 0xe0) >> 5,
				m_STATUS & 0x10 ? '.':'O',      // WDT Overflow
				m_STATUS & 0x08 ? 'P':'D',      // Power/Down
				m_STATUS & 0x04 ? 'Z':'.',      // Zero
				m_STATUS & 0x02 ? 'c':'b',      // Nibble Carry/Borrow
				m_STATUS & 0x01 ? 'C':'B',      // Carry/Borrow

				m_OPTION & 0x20 ? 'C':'T',    // Counter/Timer
				m_OPTION & 0x10 ? 'N':'P',    // Negative/Positive
				m_OPTION & 0x08 ? 'W':'T',    // WatchDog/Timer
				m_OPTION & 0x08 ? (1<<(m_OPTION&7)) : (2<<(m_OPTION&7)));
			break;
	}
}


/****************************************************************************
 *  Reset registers to their initial values
 ****************************************************************************/

void pic16x8x_device::reset_regs()
{
	set_pc(RESET_VECTOR);
	m_port_tris[PORTA] = 0x1f;
	m_port_tris[PORTB] = 0xff;
	m_OPTION = T0CS_FLAG | T0SE_FLAG | PSA_FLAG | PS_REG;
	m_PREVPC = m_PCL;
	m_STATUS = 0x18;
	m_PCL = 0;
	m_prescaler = 0;
	m_delay_timer = 0;
	m_inst_cycles = 0;
	m_count_cycles = 0;
	m_stack_pointer = 0;
	m_portb_chdetect_temp = 0xff;
	m_old_RB0 = 0;
}

void pic16x8x_device::set_config(u16 data)
{
	logerror("Writing %04x to the PIC16x8x configuration bits\n", data);
	m_CONFIG = (data & 0x3fff);
}

void pic16x8x_device::watchdog_reset()
{
	SET(m_STATUS, TO_FLAG | PD_FLAG | Z_FLAG | DC_FLAG | C_FLAG);
	reset_regs();
}

void pic16x8x_device::device_reset()
{
	reset_regs();
	CLR(m_STATUS, RP0_FLAG);
	CLR(m_STATUS, RP1_FLAG);

	// Setting TO_FLAG from Config Word
	if(WDTE)
		SET(m_STATUS, TO_FLAG);
	else
		CLR(m_STATUS, TO_FLAG);

	SET(m_STATUS, PD_FLAG);
	store_regfile(3, m_STATUS);
	store_regfile(4, m_FSR);
}


/****************************************************************************
 *  WatchDog
 ****************************************************************************/

void pic16x8x_device::update_watchdog(int counts)
{
	/*
	WatchDog is set up to count 18,000 (0x464f hex) ticks to provide
	the timeout period of 0.018ms based on a 4MHz input clock.
	Note: the 4MHz clock should be divided by the PIC16C5x_CLOCK_DIVIDER
	which effectively makes the PIC run at 1MHz internally.

	If the current instruction is CLRWDT or SLEEP, don't update the WDT
	*/

	if ((m_opcode.w != 3) && (m_opcode.w != 4))
	{
		u16 old_WDT = m_WDT;

		m_WDT -= counts;

		if (m_WDT > 0x464f)
		{
			m_WDT = 0x464f - (0xffff - m_WDT);
		}

		if (((old_WDT != 0) && (old_WDT < m_WDT)) || (m_WDT == 0))
		{
			if (PSA)
			{
				m_prescaler++;
				if (m_prescaler >= (1 << PS))
				{ // Prescale values from 1 to 128
					m_prescaler = 0;
					CLR(m_STATUS, TO_FLAG);
					watchdog_reset();
				}
			}
			else
			{
				CLR(m_STATUS, TO_FLAG);
				watchdog_reset();
			}
		}
	}
}


/****************************************************************************
 *  Update Timer
 ****************************************************************************/

void pic16x8x_device::update_timer(int counts)
{
	if (m_delay_timer > 0)
	{ // Timer increment is inhibited
		int dt = m_delay_timer;
		m_delay_timer -= m_inst_cycles;
		counts -= dt;
	}

	if (m_delay_timer > 0 || counts <= 0)
		return;

	if (PSA == 0)
	{   // if PSA is cleared, prescaler is assigned to timer
		m_prescaler += counts;
		if (m_prescaler >= (2 << PS))
		{ // Prescale values from 2 to 256
			m_TMR0 += (m_prescaler / (2 << PS));
			if(m_TMR0 == 0)
				SET(m_INTCON, T0IF_FLAG);
			m_prescaler %= (2 << PS); // Overflow prescaler
		}
	}
	else
	{
		m_TMR0 += counts;
		if(m_TMR0 == 0)
			SET(m_INTCON, T0IF_FLAG);
	}
}

void pic16x8x_device::execute_set_input(int line, int state)
{
	u8 RB0_in;
	switch (line)
	{
		// RTCC/RA4_T0CKI pin
		case PIC16x8x_T0CKI:
			if (T0CS && state != m_rtcc)
			{ // Count mode, edge triggered
				if ((T0SE && !state) || (!T0SE && state))
					m_count_cycles++;
			}
			m_rtcc = state;
			break;

		case PIC16x8x_RB0INT:
			RB0_in = state;
			if (INTEDG)
			{   /* Interrupt on Rising edge RB0 input */
				if (RISING_EDGE_RB0)
				{
					SET(m_INTCON, INTF_FLAG);
				}
			}
			else
			{   /* Interrupt on Falling edge RB0 input */
				if (FALLING_EDGE_RB0)
				{
					SET(m_INTCON, INTF_FLAG);
				}
			}
			m_old_RB0 = RB0_in;
			break;

		default:
			break;
	}
}

/*-------------------------------------------------
    check_irqs - check for and process IRQs
-------------------------------------------------*/

void pic16x8x_device::check_irqs()
{
	// if something is in progress, we do nothing
	if (m_irq_in_progress)
		return;

	if(m_read_port[PORTB](PORTB, 0xf0) != m_portb_chdetect_temp)
	{
		m_portb_chdetect_temp = m_read_port[PORTB](PORTB, 0xf0);
		SET(m_INTCON, RBIF_FLAG);
	}

	else if(GIE && RBIE && RBIF)
	{
		push_stack(m_PCL);
		set_pc(INT_VECTOR);
		standard_irq_callback(0, m_PCL);
		m_irq_in_progress = true;
	}

	else if(GIE && INTE && INTF)
	{
		push_stack(m_PCL);
		set_pc(INT_VECTOR);
		standard_irq_callback(1, m_PCL);
		m_irq_in_progress = true;
	}

	else if(GIE && T0IE && T0IF)
	{
		push_stack(m_PCL);
		set_pc(INT_VECTOR);
		standard_irq_callback(2, m_PCL);
		m_irq_in_progress = true;
	}
	
	else if(GIE && EEIE && EEIF)
	{
		push_stack(m_PCL);
		set_pc(INT_VECTOR);
		standard_irq_callback(3, m_PCL);
		m_irq_in_progress = true;
	}
}


/****************************************************************************
 *  Execute IPeriod. Return 0 if emulation should be stopped
 ****************************************************************************/

void pic16x8x_device::execute_run()
{
	do
	{
		// check interrupts
		check_irqs();

		if (PD == 0)
		{ // Sleep Mode
			m_count_cycles = 0;
			m_inst_cycles = 1;
			debugger_instruction_hook(m_PCL);
		}
		else
		{
			m_PREVPC = m_PCL;

			debugger_instruction_hook(m_PCL);

			m_opcode.w = m_program.read_word(m_PCL);
			set_pc(m_PCL + 1);

			if ( (m_opcode.w & 0x3f80) != 0x0000)
			{ // Do all opcodes except the 00? ones
				m_inst_cycles = s_opcode_main[((m_opcode.w >> 7) & 0x7f)].cycles;
				(this->*s_opcode_main[((m_opcode.w >> 7) & 0x7f)].function)();
			}
			else
			{ // Opcode 0x00? has many opcodes in its minor nibble
				m_inst_cycles = s_opcode_00x[(m_opcode.b.l & 0x7f)].cycles;
				(this->*s_opcode_00x[(m_opcode.b.l & 0x7f)].function)();
			}

			update_timer(T0CS ? m_count_cycles : m_inst_cycles);
			m_count_cycles = 0;
		}

		if (WDTE)
		{
			update_watchdog(m_inst_cycles);
		}

		m_icount -= m_inst_cycles;

	} while (m_icount > 0);
}
