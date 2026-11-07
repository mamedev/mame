// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Atmel 8-bit AVR simulator

*/

#ifndef MAME_CPU_AVR8_AVR8_H
#define MAME_CPU_AVR8_AVR8_H

#pragma once

#include "avr8timers_classic.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> avr8_base_device

// Used by core CPU interface
class avr8_base_device : public cpu_device
{
public:
	// inline configuration helpers
	void set_eeprom_tag(const char *tag) { m_eeprom.set_tag(tag); }

	// fuse configs
	void set_low_fuses(uint8_t byte);
	void set_high_fuses(uint8_t byte);
	void set_extended_fuses(uint8_t byte);
	void set_lock_bits(uint8_t byte);

	// GPIO
	enum gpio_t : int
	{
		GPIOA,
		GPIOB,
		GPIOC,
		GPIOD,
		GPIOE,
		GPIOF,
		GPIOG,
		GPIOH,
		GPIOJ,
		GPIOK,
		GPIOL,

		GPIO_COUNT
	};

	enum : uint8_t
	{
		ADC0 = 0,
		ADC1,
		ADC2,
		ADC3,
		ADC4,
		ADC5,
		ADC6,
		ADC7
	};

protected:
	avr8_base_device(const machine_config &mconfig,
					 const char *tag,
					 device_t *owner,
					 uint32_t clock,
					 const device_type type,
					 uint32_t address_mask,
					 address_map_constructor internal_map);

	typedef void (avr8_base_device::*op_func) (uint16_t op);

	enum : uint8_t
	{
		AVR8_INT_RESET = 0,
		AVR8_INT_INT0,
		AVR8_INT_INT1,
		AVR8_INT_PCINT0,
		AVR8_INT_PCINT1,
		AVR8_INT_PCINT2,
		AVR8_INT_WDT,
		AVR8_INT_T2COMPA,
		AVR8_INT_T2COMPB,
		AVR8_INT_T2OVF,
		AVR8_INT_T1CAPT,
		AVR8_INT_T1COMPA,
		AVR8_INT_T1COMPB,
		AVR8_INT_T1OVF,
		AVR8_INT_T0COMPA,
		AVR8_INT_T0COMPB,
		AVR8_INT_T0OVF,
		AVR8_INT_SPI_STC,
		AVR8_INT_USART_RX,
		AVR8_INT_USART_UDRE,
		AVR8_INT_USART_TX,
		AVR8_INT_ADC,
		AVR8_INT_EE_RDY,
		AVR8_INT_ANALOG_COMP,
		AVR8_INT_TWI,
		AVR8_INT_SPM_RDY,
	};

	// Used by I/O register handling
	enum : uint16_t
	{
		R0 = 0x00,
		R1,
		R2,
		R3,
		R4,
		R5,
		R6,
		R7,
		R8,
		R9,
		R10,
		R11,
		R12,
		R13,
		R14,
		R15,
		R16,
		R17,
		R18,
		R19,
		R20,
		R21,
		R22,
		R23,
		R24,
		R25,
		R26,
		R27,
		R28,
		R29,
		R30,
		R31,
		PINA = 0x20,
		DDRA,
		PORTA,
		PINB,
		DDRB,
		PORTB,
		PINC,
		DDRC,
		PORTC,
		PIND,
		DDRD,
		PORTD,
		PINE,
		DDRE,
		PORTE,
		PINF,
		DDRF,
		PORTF,
		PING,
		DDRG,
		PORTG,
		TIFR0 = 0x35,
		TIFR1,
		TIFR2,
		TIFR3,
		TIFR4,
		TIFR5,
		PCIFR = 0x3B,
		EIFR,
		EIMSK,
		GPIOR0,
		EECR,
		EEDR,
		EEARL,
		EEARH,
		GTCCR,
		TCCR0A,
		TCCR0B,
		TCNT0,
		OCR0A,
		OCR0B,
		//0x49: Reserved
		GPIOR1 = 0x4A,
		GPIOR2,
		SPCR,
		SPSR,
		SPDR,
		//0x4F: Reserved
		ACSR = 0x50,
		OCDR,
		//0x52: Reserved
		SMCR = 0x53,
		MCUSR,
		MCUCR,
		//0x56: Reserved
		SPMCSR = 0x57,
		//0x58: Reserved
		//0x59: Reserved
		//0x5A: Reserved
		RAMPZ = 0x5B,
		EIND,
		SPL,
		SPH,
		SREG,
	//--------------------------
		WDTCSR = 0x60,
		CLKPR,
		//0x62: Reserved
		//0x63: Reserved
		PRR0 = 0x64,
		PRR1,
		OSCCAL,
		//0x67: Reserved
		PCICR = 0x68,
		EICRA,
		EICRB,
		PCMSK0,
		PCMSK1,
		PCMSK2,
		TIMSK0,
		TIMSK1,
		TIMSK2,
		TIMSK3,
		TIMSK4,
		TIMSK5,
		XMCRA,
		XMCRB,
		//0x76: Reserved
		//0x77: Reserved
		ADCL = 0x78,
		ADCH,
		ADCSRA,
		ADCSRB,
		ADMUX,
		DIDR2,
		DIDR0,
		DIDR1,
		TCCR1A,
		TCCR1B,
		TCCR1C,
		//0x83: Reserved
		TCNT1L = 0x84,
		TCNT1H,
		ICR1L,
		ICR1H,
		OCR1AL,
		OCR1AH,
		OCR1BL,
		OCR1BH,
		OCR1CL,
		OCR1CH,
		//0x8E: Reserved
		//0x8F: Reserved
		TCCR3A = 0x90,
		TCCR3B,
		TCCR3C,
		//0x93: Reserved
		TCNT3L = 0x94,
		TCNT3H,
		ICR3L,
		ICR3H,
		OCR3AL,
		OCR3AH,
		OCR3BL,
		OCR3BH,
		OCR3CL,
		OCR3CH,
		//0x9E: Reserved
		//0x9F: Reserved
		TCCR4A = 0xA0,
		TCCR4B,
		TCCR4C,
		//0xA3: Reserved
		TCNT4L = 0xA4,
		TCNT4H,
		ICR4L,
		ICR4H,
		OCR4AL,
		OCR4AH,
		OCR4BL,
		OCR4BH,
		OCR4CL,
		OCR4CH,
		//0xAE: Reserved
		//0xAF: Reserved
		TCCR2A = 0xB0,
		TCCR2B,
		TCNT2,
		OCR2A,
		OCR2B,
		//0xB5: Reserved
		ASSR = 0xB6,
		//0xB7: Reserved
		TWBR = 0xB8,
		TWSR,
		TWAR,
		TWDR,
		TWCR,
		TWAMR,
		//0xBE: Reserved
		//0xBF: Reserved
		UCSR0A = 0xC0,
		UCSR0B,
		UCSR0C,
		//0xC3: Reserved
		UBRR0L = 0xC4,
		UBRR0H,
		UDR0,
		//0xC7: Reserved
		UCSR1A = 0xC8,
		UCSR1B,
		UCSR1C,
		//0xCB: Reserved
		UBRR1L = 0xCC,
		UBRR1H,
		UDR1,
		//0xCF: Reserved
		UCSR2A = 0xD0,
		UCSR2B,
		UCSR2C,
		//0xD3: Reserved
		UBRR2L = 0xD4,
		UBRR2H,
		UDR2,
		//0xD7: Reserved
		//0xD8: Reserved
		//0xD9: Reserved
		//0xDA: Reserved
		//0xDB: Reserved
		//0xDC: Reserved
		//0xDD: Reserved
		//0xDE: Reserved
		//0xDF: Reserved
		//0xE0: Reserved
		//0xE1: Reserved
		//0xE2: Reserved
		//0xE3: Reserved
		//0xE4: Reserved
		//0xE5: Reserved
		//0xE6: Reserved
		//0xE7: Reserved
		//0xE8: Reserved
		//0xE9: Reserved
		//0xEA: Reserved
		//0xEB: Reserved
		//0xEC: Reserved
		//0xED: Reserved
		//0xEE: Reserved
		//0xEF: Reserved
		//0xF0: Reserved
		//0xF1: Reserved
		//0xF2: Reserved
		//0xF3: Reserved
		//0xF4: Reserved
		//0xF5: Reserved
		//0xF6: Reserved
		//0xF7: Reserved
		//0xF8: Reserved
		//0xF9: Reserved
		//0xFA: Reserved
		//0xFB: Reserved
		//0xFC: Reserved
		//0xFD: Reserved
		//0xFE: Reserved
		//0xFF: Reserved
		PINH = 0x100,
		DDRH,
		PORTH,
		PINJ,
		DDRJ,
		PORTJ,
		PINK,
		DDRK,
		PORTK,
		PINL,
		DDRL,
		PORTL,
		//0x10C: Reserved
		//0x10D: Reserved
		//0x10E: Reserved
		//0x10F: Reserved
		//0x110: Reserved
		//0x111: Reserved
		//0x112: Reserved
		//0x113: Reserved
		//0x114: Reserved
		//0x115: Reserved
		//0x116: Reserved
		//0x117: Reserved
		//0x118: Reserved
		//0x119: Reserved
		//0x11A: Reserved
		//0x11B: Reserved
		//0x11C: Reserved
		//0x11D: Reserved
		//0x11E: Reserved
		//0x11F: Reserved
		TCCR5A = 0x120,
		TCCR5B,
		TCCR5C,
		//0x123: Reserved
		TCNT5L = 0x124,
		TCNT5H,
		ICR5L,
		ICR5H,
		OCR5AL,
		OCR5AH,
		OCR5BL,
		OCR5BH,
		OCR5CL,
		OCR5CH,
		//0x12E: Reserved
		//0x12F: Reserved
		UCSR3A = 0x130,
		UCSR3B,
		UCSR3C,
		//0x133: Reserved
		UBRR3L = 0x134,
		UBRR3H,
		UDR3
		//0x137: Reserved
		//  .
		//  . up to
		//  .
		//0x1FF: Reserved
	};

	enum : uint8_t
	{
		AVR8_IO_PORTA = 0,
		AVR8_IO_PORTB,
		AVR8_IO_PORTC,
		AVR8_IO_PORTD,
		AVR8_IO_PORTE,
		AVR8_IO_PORTF,
		AVR8_IO_PORTG,
		AVR8_IO_PORTH,
		AVR8_IO_PORTJ,
		AVR8_IO_PORTK,
		AVR8_IO_PORTL
	};

	enum : uint8_t
	{
		AVR8_REG_A = 0,
		AVR8_REG_B,
		AVR8_REG_C,
		AVR8_REG_D,
		AVR8_REG_E,
		AVR8_REG_F,
		AVR8_REG_G,
		AVR8_REG_H,
		AVR8_REG_J,
		AVR8_REG_K,
		AVR8_REG_L
	};

	enum : uint8_t
	{
		INTIDX_SPI,

		INTIDX_OCF0B,
		INTIDX_OCF0A,
		INTIDX_TOV0,

		INTIDX_ICF1,

		INTIDX_OCF1B,
		INTIDX_OCF1A,
		INTIDX_TOV1,

		INTIDX_OCF2B,
		INTIDX_OCF2A,
		INTIDX_TOV2,

		INTIDX_PCINT0,
		INTIDX_PCINT1,
		INTIDX_PCINT2,

		INTIDX_INT0,
		INTIDX_INT1,

	//------ TODO: review this --------
		INTIDX_OCF3B,
		INTIDX_OCF3A,
		INTIDX_TOV3,

		INTIDX_OCF4B,
		INTIDX_OCF4A,
		INTIDX_TOV4,

		INTIDX_OCF5B,
		INTIDX_OCF5A,
		INTIDX_TOV5,
	//---------------------------------

		// new guys free of the old legacy crap
		INTIDX_OCF1C,
		INTIDX_OCF3C,
		INTIDX_OCF4C,
		INTIDX_OCF5C,

		INTIDX_COUNT
	};

	enum : uint8_t
	{
		ATMEGA644_INT_RESET = 0,
		ATMEGA644_INT_INT0,
		ATMEGA644_INT_INT1,
		ATMEGA644_INT_INT2,
		ATMEGA644_INT_PCINT0,
		ATMEGA644_INT_PCINT1,
		ATMEGA644_INT_PCINT2,
		ATMEGA644_INT_PCINT3,
		ATMEGA644_INT_WDT,
		ATMEGA644_INT_T2COMPA,
		ATMEGA644_INT_T2COMPB,
		ATMEGA644_INT_T2OVF,
		ATMEGA644_INT_T1CAPT,
		ATMEGA644_INT_T1COMPA,
		ATMEGA644_INT_T1COMPB,
		ATMEGA644_INT_T1OVF,
		ATMEGA644_INT_T0COMPA,
		ATMEGA644_INT_T0COMPB,
		ATMEGA644_INT_T0OVF,
		ATMEGA644_INT_SPI_STC,
		ATMEGA644_INT_USART_RX,
		ATMEGA644_INT_USART_UDRE,
		ATMEGA644_INT_USART_TX,
		ATMEGA644_INT_ADC,
		ATMEGA644_INT_EE_RDY,
		ATMEGA644_INT_ANALOG_COMP,
		ATMEGA644_INT_TWI,
		ATMEGA644_INT_SPM_RDY
	};

	// Interrupt vectors for the ATMega640, 1280, 1281, 2560, 2561 series.
	// Vector size is 2 words (=4 bytes).
	// See https://content.arduino.cc/assets/ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf
	enum : uint8_t
	{
		ATMEGA640_INT_RESET = 0,
		ATMEGA640_INT_INT0,
		ATMEGA640_INT_INT1,
		ATMEGA640_INT_INT2,
		ATMEGA640_INT_INT3,
		ATMEGA640_INT_INT4,
		ATMEGA640_INT_INT5,
		ATMEGA640_INT_INT6,
		ATMEGA640_INT_INT7,
		ATMEGA640_INT_PCINT0,
		ATMEGA640_INT_PCINT1,
		ATMEGA640_INT_PCINT2,
		ATMEGA640_INT_WDT,
		ATMEGA640_INT_T2COMPA,
		ATMEGA640_INT_T2COMPB,
		ATMEGA640_INT_T2OVF,
		ATMEGA640_INT_T1CAPT,
		ATMEGA640_INT_T1COMPA,
		ATMEGA640_INT_T1COMPB,
		ATMEGA640_INT_T1COMPC,
		ATMEGA640_INT_T1OVF,
		ATMEGA640_INT_T0COMPA,
		ATMEGA640_INT_T0COMPB,
		ATMEGA640_INT_T0OVF,
		ATMEGA640_INT_SPI_STC,
		ATMEGA640_INT_USART0_RX,
		ATMEGA640_INT_USART0_UDRE,
		ATMEGA640_INT_USART0_TX,
		ATMEGA640_INT_ANALOG_COMP,
		ATMEGA640_INT_ADC,
		ATMEGA640_INT_EE_RDY,
		ATMEGA640_INT_T3CAPT,
		ATMEGA640_INT_T3COMPA,
		ATMEGA640_INT_T3COMPB,
		ATMEGA640_INT_T3COMPC,
		ATMEGA640_INT_T3OVF,
		ATMEGA640_INT_USART1_RX,
		ATMEGA640_INT_USART1_UDRE,
		ATMEGA640_INT_USART1_TX,
		ATMEGA640_INT_TWI,
		ATMEGA640_INT_SPM_RDY,
		ATMEGA640_INT_T4CAPT,
		ATMEGA640_INT_T4COMPA,
		ATMEGA640_INT_T4COMPB,
		ATMEGA640_INT_T4COMPC,
		ATMEGA640_INT_T4OVF,
		ATMEGA640_INT_T5CAPT,
		ATMEGA640_INT_T5COMPA,
		ATMEGA640_INT_T5COMPB,
		ATMEGA640_INT_T5COMPC,
		ATMEGA640_INT_T5OVF,
		ATMEGA640_INT_USART2_RX,
		ATMEGA640_INT_USART2_UDRE,
		ATMEGA640_INT_USART2_TX,
		ATMEGA640_INT_USART3_RX,
		ATMEGA640_INT_USART3_UDRE,
		ATMEGA640_INT_USART3_TX,
	};

	// Vectors for the ATMega16U4 and ATMega32U4.
	// Vector size is 2 words (=4 bytes).
	enum : uint8_t
	{
		ATMEGA32U4_INT_RESET = 0,
		ATMEGA32U4_INT_INT0,
		ATMEGA32U4_INT_INT1,
		ATMEGA32U4_INT_INT2,
		ATMEGA32U4_INT_INT3,
		ATMEGA32U4_INT_RESERVED_0,
		ATMEGA32U4_INT_RESERVED_1,
		ATMEGA32U4_INT_INT6,
		ATMEGA32U4_INT_RESERVED_2,
		ATMEGA32U4_INT_PCINT0,
		ATMEGA32U4_INT_USB_GENERAL,
		ATMEGA32U4_INT_USB_ENDPOINT,
		ATMEGA32U4_INT_WDT,
		ATMEGA32U4_INT_RESERVED_3,
		ATMEGA32U4_INT_RESERVED_4,
		ATMEGA32U4_INT_RESERVED_5,
		ATMEGA32U4_INT_T1CAPT,
		ATMEGA32U4_INT_T1COMPA,
		ATMEGA32U4_INT_T1COMPB,
		ATMEGA32U4_INT_T1COMPC,
		ATMEGA32U4_INT_T1OVF,
		ATMEGA32U4_INT_T0COMPA,
		ATMEGA32U4_INT_T0COMPB,
		ATMEGA32U4_INT_T0OVF,
		ATMEGA32U4_INT_SPI_STC,
		ATMEGA32U4_INT_USART_RX,
		ATMEGA32U4_INT_USART_UDRE,
		ATMEGA32U4_INT_USART_TX,
		ATMEGA32U4_INT_ANALOG_COMP,
		ATMEGA32U4_INT_ADC,
		ATMEGA32U4_INT_EE_RDY,
		ATMEGA32U4_INT_T3CAPT,
		ATMEGA32U4_INT_T3COMPA,
		ATMEGA32U4_INT_T3COMPB,
		ATMEGA32U4_INT_T3COMPC,
		ATMEGA32U4_INT_T3OVF,
		ATMEGA32U4_INT_TWI,
		ATMEGA32U4_INT_SPM_RDY,
		ATMEGA32U4_INT_T4COMPA,
		ATMEGA32U4_INT_T4COMPB,
		ATMEGA32U4_INT_T4COMPD,
		ATMEGA32U4_INT_T4OVF,
		ATMEGA32U4_INT_T4FPF,
	};

	// lock bit masks
	enum : uint8_t
	{
		LB1     = (1 << 0),
		LB2     = (1 << 1),
		BLB01   = (1 << 2),
		BLB02   = (1 << 3),
		BLB11   = (1 << 4),
		BLB12   = (1 << 5)
	};

	// extended fuses bit masks
	enum : uint8_t
	{
		BODLEVEL0 = (1 << 0),
		BODLEVEL1 = (1 << 1),
		BODLEVEL2 = (1 << 2)
	};

	// high fuses bit masks
	enum : uint8_t
	{
		BOOTRST = (1 << 0),
		BOOTSZ0 = (1 << 1),
		BOOTSZ1 = (1 << 2),
		EESAVE  = (1 << 3),
		WDTON   = (1 << 4),
		SPIEN   = (1 << 5),
		JTAGEN  = (1 << 6),
		OCDEN   = (1 << 7)
	};

	// low fuses bit masks
	enum : uint8_t
	{
		CKSEL0  = (1 << 0),
		CKSEL1  = (1 << 1),
		CKSEL2  = (1 << 2),
		CKSEL3  = (1 << 3),
		SUT0    = (1 << 4),
		SUT1    = (1 << 5),
		CKOUT   = (1 << 6),
		CKDIV8  = (1 << 7)
	};

	enum : uint8_t
	{
		EEARH_MASK = (1 << 0),

		SPSR_SPR2X_MASK = (1 << 0),
		SPSR_SPIF_SHIFT = 7,
		SPSR_SPIF_MASK = (1 << SPSR_SPIF_SHIFT),

		SPCR_SPR_MASK = (3 << 0),
		SPCR_CPHA_MASK = (1 << 2),
		SPCR_CPOL_MASK = (1 << 3),
		SPCR_MSTR_MASK = (1 << 4),
		SPCR_DORD_MASK = (1 << 5),
		SPCR_SPE_MASK = (1 << 6),
		SPCR_SPIE_MASK = (1 << 7)
	};

	struct interrupt_condition
	{
		uint8_t  m_intidx;       // one of the INTIDX_x universal interrupt mappings
		uint8_t  m_intvector;    // interrupt vector number
		uint8_t  m_intondev;     // interrupt is handled by a device (m_intreg, m_intmask, m_regindex, m_regmask all ignored)
		uint8_t  m_intreg;       // register containing interrupt enable flag
		uint8_t  m_intmask;      // bitmask for the interrupt enable flag itself
		uint8_t  m_regindex;     // event indication register containing event flag e.g., TOV0
		uint8_t  m_regmask;      // bitmask for the event flag itself
	};

	// maps a GPIO port to its pin-change interrupt group, if it has one:
	// pcmsk_reg is the PCMSKn register gating which pins in the port raise the interrupt,
	// group is the bit position shared by PCICR/PCIFR for that group (also INTIDX's implicit ordering),
	// intidx is the INTIDX_PCINTn value when signalling an interrupt
	virtual bool pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const { return false; }

	// PORTB bit masks for the hardware SPI pins (default: ATmega88/168/328 mapping)
	virtual void spi_pins(uint8_t &mosi_mask, uint8_t &miso_mask, uint8_t &sck_mask) const
	{
		mosi_mask = 0x08; // PB3
		miso_mask = 0x10; // PB4
		sck_mask  = 0x20; // PB5
	}

	// valid bits of EEARH, which depend on how large this chip's EEPROM is
	// (default: 512-byte EEPROM, e.g. ATmega88/168)
	virtual uint8_t eearh_mask() const { return EEARH_MASK; }

	op_func m_op_funcs[0x10000];
	int m_op_cycles[0x10000];
	int m_opcycles;
	std::unique_ptr<uint8_t[]> m_add_flag_cache;
	std::unique_ptr<uint8_t[]> m_adc_flag_cache;
	std::unique_ptr<uint8_t[]> m_sub_flag_cache;
	std::unique_ptr<uint8_t[]> m_sbc_flag_cache;
	std::unique_ptr<uint8_t[]> m_bool_flag_cache;
	std::unique_ptr<uint8_t[]> m_shift_flag_cache;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 4; }

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_data_config;
	required_region_ptr<uint8_t> m_eeprom;

	// bootloader
	uint16_t m_boot_size;

	// Fuses
	uint8_t m_lfuses;
	uint8_t m_hfuses;
	uint8_t m_efuses;
	uint8_t m_lock_bits;

	// CPU registers
	uint8_t m_r[0x200]; // no longer shared because it's getting broken up
	uint32_t m_pc;

	// internal CPU state
	uint32_t m_addr_mask;
	bool m_sleeping;
	bool m_sei_delay_pending;

	// other internal states
	int m_icount;

	// Interrupt handling tables
	uint8_t m_int_statuses[INTIDX_COUNT];   // persistent, must be savestate-able
	devcb_write_line::array<INTIDX_COUNT> m_int_ack_cbs; // rebuilt at device_start
	std::unique_ptr<interrupt_condition[]> m_int_conditions_table; // rebuilt at device_start

	// Interrupt handling size for the AVR8s is usually 2; some devices will only use 1
	virtual uint8_t vector_size_in_words() const { return 2; }

	// memory access
	inline void push(uint8_t val);
	inline uint8_t pop();
	inline bool is_long_opcode(uint16_t op);

	// utility
	void unimplemented_opcode(uint32_t op);

	// interrupt handlers
	void fire_interrupts();
	inline bool interrupt_condition_entry_is_terminator(interrupt_condition condition);
	void populate_interrupt_condition_table();
	void update_interrupt(uint8_t intidx);

	// temporary; for registers in m_r that are not yet brought out to devices,
	// or don't have dedicated _r() functions
	template <int Reg> uint8_t reg_r() { return m_r[Reg]; }
	template <int Reg> void reg_w(uint8_t data) { m_r[Reg] = data; }

	// also temporary; gives CPU access to ranges of registers (should be the first 32 bytes of m_r)
	uint8_t regs_r(offs_t offset) { return m_r[offset]; }
	void regs_w(offs_t offset, uint8_t data) { m_r[offset] = data; }

	void set_interrupt_line(uint8_t intidx, uint8_t state);

	// for callback installation in devices; they can't call set_interrupt_line directly
	template <uint8_t IntIdx>
	void int_line_w(uint8_t state);

	/**
	 * Set interrupt flag for the given intidx. If the interrupt is also enabled,
	 * set it to fire.
	 */
	void set_interrupt(uint8_t intidx);

	/**
	 * Clear interrupt flag for the given intidx. If the interrupt was pending,
	 * then disable it.
	 */
	void clear_interrupt(uint8_t intidx);

	// ops
	void populate_ops();
	void populate_add_flag_cache();
	void populate_adc_flag_cache();
	void populate_sub_flag_cache();
	void populate_sbc_flag_cache();
	void populate_bool_flag_cache();
	void populate_shift_flag_cache();
	void op_nop(uint16_t op);
	void op_movw(uint16_t op);
	void op_muls(uint16_t op);
	void op_mulsu(uint16_t op);
	void op_fmul(uint16_t op);
	void op_fmuls(uint16_t op);
	void op_fmulsu(uint16_t op);
	void op_cpc(uint16_t op);
	void op_sbc(uint16_t op);
	void op_add(uint16_t op);
	void op_cpse(uint16_t op);
	void op_cp(uint16_t op);
	void op_sub(uint16_t op);
	void op_adc(uint16_t op);
	void op_and(uint16_t op);
	void op_eor(uint16_t op);
	void op_or(uint16_t op);
	void op_mov(uint16_t op);
	void op_cpi(uint16_t op);
	void op_sbci(uint16_t op);
	void op_subi(uint16_t op);
	void op_ori(uint16_t op);
	void op_andi(uint16_t op);
	void op_lddz(uint16_t op);
	void op_lddy(uint16_t op);
	void op_stdz(uint16_t op);
	void op_stdy(uint16_t op);
	void op_lds(uint16_t op);
	void op_ldzi(uint16_t op);
	void op_ldzd(uint16_t op);
	void op_lpmz(uint16_t op);
	void op_lpmzi(uint16_t op);
	void op_elpmz(uint16_t op);
	void op_elpmzi(uint16_t op);
	void op_ldyi(uint16_t op);
	void op_ldyd(uint16_t op);
	void op_ldx(uint16_t op);
	void op_ldxi(uint16_t op);
	void op_ldxd(uint16_t op);
	void op_pop(uint16_t op);
	void op_sts(uint16_t op);
	void op_stzi(uint16_t op);
	void op_stzd(uint16_t op);
	void op_styi(uint16_t op);
	void op_styd(uint16_t op);
	void op_stx(uint16_t op);
	void op_stxi(uint16_t op);
	void op_stxd(uint16_t op);
	void op_push(uint16_t op);
	void op_com(uint16_t op);
	void op_neg(uint16_t op);
	void op_swap(uint16_t op);
	void op_inc(uint16_t op);
	void op_asr(uint16_t op);
	void op_lsr(uint16_t op);
	void op_ror(uint16_t op);
	void op_setf(uint16_t op);
	void op_clrf(uint16_t op);
	void op_ijmp(uint16_t op);
	void op_eijmp(uint16_t op);
	void op_dec(uint16_t op);
	void op_jmp(uint16_t op);
	void op_call(uint16_t op);
	void op_ret(uint16_t op);
	void op_reti(uint16_t op);
	void op_sleep(uint16_t op);
	void op_break(uint16_t op);
	void op_wdr(uint16_t op);
	void op_lpm(uint16_t op);
	void op_elpm(uint16_t op);
	void op_spm(uint16_t op);
	void op_spmzi(uint16_t op);
	void op_icall(uint16_t op);
	void op_eicall(uint16_t op);
	void op_adiw(uint16_t op);
	void op_sbiw(uint16_t op);
	void op_cbi(uint16_t op);
	void op_sbic(uint16_t op);
	void op_sbi(uint16_t op);
	void op_sbis(uint16_t op);
	void op_mul(uint16_t op);
	void op_out(uint16_t op);
	void op_in(uint16_t op);
	void op_rjmp(uint16_t op);
	void op_rcall(uint16_t op);
	void op_ldi(uint16_t op);
	void op_brset(uint16_t op);
	void op_brclr(uint16_t op);
	void op_bst(uint16_t op);
	void op_bld(uint16_t op);
	void op_sbrs(uint16_t op);
	void op_sbrc(uint16_t op);
	void op_unimpl(uint16_t op);

	// address spaces
	address_space *m_program;
	address_space *m_data;

	static const interrupt_condition s_int_conditions[];
	static const interrupt_condition s_mega640_int_conditions[];
	static const interrupt_condition s_mega644_int_conditions[];
	static const interrupt_condition s_mega32u4_int_conditions[];

	// override this in subclasses and return the intended int_conditions tables above.
	// the table will be read on device init to populate m_int_conditions_table.
	virtual const interrupt_condition* interrupt_conditions() { return s_int_conditions; }
};

// ======================> avr8_device

/**
 * TODO: rename this class to something like avr8_classic_base_device
 */
class avr8_device : public avr8_base_device
{
public:
	// GPIO
	template <gpio_t Port> auto gpio_out() { return m_gpio_out_cb[Port].bind(); }
	template <gpio_t Port> auto gpio_in() { return m_gpio_in_cb[Port].bind(); }
	template <int Port> uint8_t pin_r();
	template <int Port> void port_w(uint8_t data);
	template <int Port> void ddr_w(uint8_t data);
	template <int Port> uint8_t gpio_r();

	// call when an external device pin changes: updates that bit of the port's internal input shadow
	// (used verbatim if no gpio_in callback is bound, ignored as a value but still triggering a resample
	// otherwise) and raises the port's pin-change/external interrupt if enabled
	template <int Port> void pin_w(int bit, int state);

	// single-pin form, e.g. FUNC(atmega1284_device::pin_w<atmega1284_device::GPIOC, 0>) as a write-line target
	template <gpio_t Port, int Bit> void pin_w(int state) { pin_w<Port>(Bit, state); }
	template <int Bit> void pa_w(int state) { pin_w<GPIOA, Bit>(state); }
	template <int Bit> void pb_w(int state) { pin_w<GPIOB, Bit>(state); }
	template <int Bit> void pc_w(int state) { pin_w<GPIOC, Bit>(state); }
	template <int Bit> void pd_w(int state) { pin_w<GPIOD, Bit>(state); }
	template <int Bit> void pe_w(int state) { pin_w<GPIOE, Bit>(state); }
	template <int Bit> void pf_w(int state) { pin_w<GPIOF, Bit>(state); }
	template <int Bit> void pg_w(int state) { pin_w<GPIOG, Bit>(state); }

	// checks INT0 (line 0, PD2) or INT1 (line 1, PD3) against EICRA's sense-control bits
	void check_extint(int line, uint8_t prev, uint8_t state);

	// ADC
	template<uint8_t Pin> auto adc_in() { return m_adc_in_cb[Pin].bind(); }

protected:
	avr8_device(const machine_config &mconfig,
				const char *tag,
				device_t *owner,
				uint32_t clock,
				const device_type type,
				uint32_t address_mask,
				address_map_constructor internal_map);

	typedef delegate<void (void)> timer_func;


	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// address maps
	void base_internal_map(address_map &map) ATTR_COLD;

	// Miscellaneous registers

	// TODO: which regs are common on the classic AVR8s?
	// ones that are megaAVR-specific need to be moved out
	void wdtcsr_w(uint8_t data);
	void clkpr_w(uint8_t data);
	void prr0_w(uint8_t data);
	void prr1_w(uint8_t data);
	void osccal_w(uint8_t data);
	void pcicr_w(uint8_t data);
	void pcifr_w(uint8_t data);
	void eicra_w(uint8_t data);
	void eifr_w(uint8_t data);
	void eicrb_w(uint8_t data);
	void pcmsk0_w(uint8_t data);
	void pcmsk1_w(uint8_t data);
	void pcmsk2_w(uint8_t data);
	void xmcra_w(uint8_t data);
	void xmcrb_w(uint8_t data);
	void didr0_w(uint8_t data);
	void didr1_w(uint8_t data);
	void didr2_w(uint8_t data);
	void assr_w(uint8_t data);
	void twbr_w(uint8_t data);
	uint8_t twsr_r();
	void twsr_w(uint8_t data);
	void twar_w(uint8_t data);
	void twdr_w(uint8_t data);
	void twcr_w(uint8_t data);
	void twamr_w(uint8_t data);
	void ucsr0a_w(uint8_t data);
	void ucsr0b_w(uint8_t data);
	void ucsr0c_w(uint8_t data);

	// EEPROM
	void eecr_w(uint8_t data);

	void gpior0_w(uint8_t data);
	void gpior1_w(uint8_t data);
	void gpior2_w(uint8_t data);

	devcb_write8::array<11> m_gpio_out_cb;
	devcb_read8::array<11> m_gpio_in_cb;
	uint8_t m_gpio_in[GPIO_COUNT]{};
	uint8_t m_pcint_last[11]{};

	// returns the port's current sampled level: the bound gpio_in callback if set, else the internal input shadow
	template <int Port> uint8_t sample_port();

	// diffs a freshly-sampled port level against the last-known one, raising PCINT/INT0/INT1 as needed
	template <int Port> void latch_pin_change(uint8_t state);

	template <int Port, int Bit>
	void toggle_gpio_bit(uint8_t _); // value written doesn't matter

	template <int Port, int Bit>
	void set_gpio_bit(uint8_t data);

	// ADC
	uint8_t adcl_r();
	void adcl_w(uint8_t data);
	uint8_t adch_r();
	void adch_w(uint8_t data);
	void adcsra_w(uint8_t data);
	void adcsrb_w(uint8_t data);
	void admux_w(uint8_t data);

	void adc_start_conversion();
	TIMER_CALLBACK_MEMBER(adc_conversion_complete);

	devcb_read16::array<8> m_adc_in_cb;
	emu_timer *m_adc_timer;
	uint16_t m_adc_sample;
	uint16_t m_adc_result;
	uint16_t m_adc_data;
	bool m_adc_first;
	bool m_adc_hold;

	// SPI (only on megaAVRs, the tinyAVRs don't have it)
	void spsr_w(uint8_t data);
	void spcr_w(uint8_t data);
	void spdr_w(uint8_t data);

	void spi_tick();
	void enable_spi();
	void disable_spi();
	void spi_update_masterslave_select();
	void spi_update_clock_polarity();
	void spi_update_clock_phase();
	void spi_update_clock_rate();
	void change_spcr(uint8_t data);
	void change_spsr(uint8_t data);

	bool m_spi_active;
	uint8_t m_spi_prescale;
	uint8_t m_spi_prescale_count;
	int8_t m_spi_prescale_countdown;
	uint8_t m_spi_rx_shift;

	// timers
	optional_device_array<avr8_base_timer_device, 6> m_timers; // optional, because some timers will almost certainly not be present
	required_device<avr8_10bit_prescaler_device> m_syncprescale;
	optional_device<avr8_fakertc_async_controller_device> m_asyncunit;
};

// ======================> megaavr_classic_base_device

/**
 * This is the common foundation for the classic megaAVRs,
 * except for the ATMega8/16/32 series, which use different data address maps
 * and registers closer to the classic tinyAVRs.
 */
class megaavr_classic_base_device : public avr8_device
{

protected:
	megaavr_classic_base_device(const machine_config &mconfig,
								const char *tag,
								device_t *owner,
								uint32_t clock,
								const device_type type,
								uint32_t address_mask,
								address_map_constructor internal_map);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// GTCCR on most of the classic megaAVRs is pretty much the same
	void gtccr_w(uint8_t data);
	uint8_t gtccr_r();

	void megaavr_classic_base_internal_map(address_map &map) ATTR_COLD;

	// GTCCR flags, MUST BE SAVESTATE-ABLE
	bool m_tsm;
	bool m_psrasy;
	bool m_psrsync;
};

// device type definition
DECLARE_DEVICE_TYPE(ATMEGA88,   atmega88_device)
DECLARE_DEVICE_TYPE(ATMEGA168,  atmega168_device)
DECLARE_DEVICE_TYPE(ATMEGA328,  atmega328_device)
DECLARE_DEVICE_TYPE(ATMEGA32U4, atmega32u4_device)
DECLARE_DEVICE_TYPE(ATMEGA644,  atmega644_device)
DECLARE_DEVICE_TYPE(ATMEGA1284, atmega1284_device)
DECLARE_DEVICE_TYPE(ATMEGA1280, atmega1280_device)
DECLARE_DEVICE_TYPE(ATMEGA2560, atmega2560_device)


///////////////////////////////////////////////////////////////
// Classic ATMegax8 series devices
///////////////////////////////////////////////////////////////

class atmegax8_base_device : public megaavr_classic_base_device
{
protected:
	// construction/destruction
	atmegax8_base_device(const machine_config &mconfig,
						 const char *tag,
						 device_t *owner,
						 uint32_t clock,
						 const device_type type,
						 uint32_t address_mask,
						 address_map_constructor internal_map);


	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void atmegax8_base_internal_map(address_map &map) ATTR_COLD;

};

// ======================> atmega88_device

class atmega88_device : public atmegax8_base_device
{
public:
	// construction/destruction
	atmega88_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void atmega88_internal_map(address_map &map) ATTR_COLD;

protected:
	// TODO: pcint_group unification in atmegax8_base_device
	virtual bool pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const override;

	virtual uint8_t vector_size_in_words() const override { return 1; }
};

// ======================> atmega168_device

class atmega168_device : public atmegax8_base_device
{
public:
	// construction/destruction
	atmega168_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void atmega168_internal_map(address_map &map) ATTR_COLD;

protected:
	// TODO: pcint_group unification in atmegax8_base_device
	virtual bool pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const override;
};

// ======================> atmega328_device

class atmega328_device : public atmegax8_base_device
{
public:
	// construction/destruction
	atmega328_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void atmega328_internal_map(address_map &map) ATTR_COLD;

protected:
	// TODO: pcint_group unification in atmegax8_base_device
	virtual bool pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const override;

	virtual uint8_t eearh_mask() const override { return 0x03; } // 1024-byte EEPROM
};


///////////////////////////////////////////////////////////////
// Classic ATMegaxU4 series devices
///////////////////////////////////////////////////////////////

class atmegaxu4_base_device : public megaavr_classic_base_device
{
protected:
	// construction/destruction
	atmegaxu4_base_device(const machine_config &mconfig,
						  const char *tag,
						  device_t *owner,
						  uint32_t clock,
						  const device_type type,
						  uint32_t address_mask,
						  address_map_constructor internal_map);


	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void atmegaxu4_base_internal_map(address_map &map) ATTR_COLD;
};

class atmega32u4_device : public atmegaxu4_base_device
{
public:
	// construction/destruction
	atmega32u4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void atmega32u4_internal_map(address_map &map) ATTR_COLD;

protected:
	virtual const interrupt_condition* interrupt_conditions() override { return s_mega32u4_int_conditions; }
	virtual bool pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const override;
	virtual void spi_pins(uint8_t &mosi_mask, uint8_t &miso_mask, uint8_t &sck_mask) const override
	{
		sck_mask  = 0x02; // PB1
		mosi_mask = 0x04; // PB2
		miso_mask = 0x08; // PB3
	}
	virtual uint8_t eearh_mask() const override { return 0x03; } // 1024-byte EEPROM
};


///////////////////////////////////////////////////////////////
// Classic ATMegax4 series devices
///////////////////////////////////////////////////////////////

class atmegax4_base_device : public megaavr_classic_base_device
{
protected:
	// construction/destruction
	atmegax4_base_device(const machine_config &mconfig,
						  const char *tag,
						  device_t *owner,
						  uint32_t clock,
						  const device_type type,
						  uint32_t address_mask,
						  address_map_constructor internal_map);


	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void atmegax4_base_internal_map(address_map &map) ATTR_COLD;
};

// ======================> atmega644_device

class atmega644_device : public atmegax4_base_device
{
public:
	// construction/destruction
	atmega644_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void atmega644_internal_map(address_map &map) ATTR_COLD;

protected:
	virtual const interrupt_condition* interrupt_conditions() override { return s_mega644_int_conditions; }
	virtual bool pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const override;
	virtual void spi_pins(uint8_t &mosi_mask, uint8_t &miso_mask, uint8_t &sck_mask) const override
	{
		mosi_mask = 0x20; // PB5
		miso_mask = 0x40; // PB6
		sck_mask  = 0x80; // PB7
	}
	virtual uint8_t eearh_mask() const override { return 0x07; } // 2048-byte EEPROM
};

// ======================> atmega1284_device

class atmega1284_device : public atmegax4_base_device
{
public:
	// construction/destruction
	atmega1284_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void atmega1284_internal_map(address_map &map) ATTR_COLD;

protected:
	virtual const interrupt_condition* interrupt_conditions() override { return s_mega644_int_conditions; }
	virtual bool pcint_group(gpio_t port, uint8_t &pcmsk_reg, int &group) const override;
	virtual void spi_pins(uint8_t &mosi_mask, uint8_t &miso_mask, uint8_t &sck_mask) const override
	{
		mosi_mask = 0x20; // PB5
		miso_mask = 0x40; // PB6
		sck_mask  = 0x80; // PB7
	}
	virtual uint8_t eearh_mask() const override { return 0x0f; } // 4096-byte EEPROM
};


///////////////////////////////////////////////////////////////
// Classic ATMegax0 series devices
// Not to be confused with the modern megaAVR 0-series
///////////////////////////////////////////////////////////////

class atmegax0_base_device : public megaavr_classic_base_device
{
protected:
	// construction/destruction
	atmegax0_base_device(const machine_config &mconfig,
						  const char *tag,
						  device_t *owner,
						  uint32_t clock,
						  const device_type type,
						  uint32_t address_mask,
						  address_map_constructor internal_map);


	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void atmegax0_base_internal_map(address_map &map) ATTR_COLD;
};


// ======================> atmega1280_device

class atmega1280_device : public atmegax0_base_device
{
public:
	// construction/destruction
	atmega1280_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void atmega1280_internal_map(address_map &map) ATTR_COLD;

protected:
	virtual const interrupt_condition* interrupt_conditions() override { return s_mega640_int_conditions; }
	virtual void spi_pins(uint8_t &mosi_mask, uint8_t &miso_mask, uint8_t &sck_mask) const override
	{
		mosi_mask = 0x04; // PB2
		miso_mask = 0x08; // PB3
		sck_mask  = 0x02; // PB1
	}
	virtual uint8_t eearh_mask() const override { return 0x0f; } // 4096-byte EEPROM
};

// ======================> atmega2560_device

class atmega2560_device : public atmegax0_base_device
{
public:
	// construction/destruction
	atmega2560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void atmega2560_internal_map(address_map &map) ATTR_COLD;

protected:
	virtual const interrupt_condition* interrupt_conditions() override { return s_mega640_int_conditions; }
	virtual void spi_pins(uint8_t &mosi_mask, uint8_t &miso_mask, uint8_t &sck_mask) const override
	{
		mosi_mask = 0x04; // PB2
		miso_mask = 0x08; // PB3
		sck_mask  = 0x02; // PB1
	}
	virtual uint8_t eearh_mask() const override { return 0x0f; } // 4096-byte EEPROM
};

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum : uint8_t
{
	AVR8_SREG = 1,
	AVR8_PC,
	AVR8_R0,
	AVR8_R1,
	AVR8_R2,
	AVR8_R3,
	AVR8_R4,
	AVR8_R5,
	AVR8_R6,
	AVR8_R7,
	AVR8_R8,
	AVR8_R9,
	AVR8_R10,
	AVR8_R11,
	AVR8_R12,
	AVR8_R13,
	AVR8_R14,
	AVR8_R15,
	AVR8_R16,
	AVR8_R17,
	AVR8_R18,
	AVR8_R19,
	AVR8_R20,
	AVR8_R21,
	AVR8_R22,
	AVR8_R23,
	AVR8_R24,
	AVR8_R25,
	AVR8_R26,
	AVR8_R27,
	AVR8_R28,
	AVR8_R29,
	AVR8_R30,
	AVR8_R31,
	AVR8_X,
	AVR8_Y,
	AVR8_Z,
	AVR8_SPH,
	AVR8_SPL
};



#endif /* MAME_CPU_AVR8_AVR8_H */
