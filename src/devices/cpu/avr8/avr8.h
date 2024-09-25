// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Atmel 8-bit AVR simulator

*/

#ifndef MAME_CPU_AVR8_AVR8_H
#define MAME_CPU_AVR8_AVR8_H

#pragma once


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

	// public interfaces
	virtual void update_interrupt(int source);

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
	avr8_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, uint32_t address_mask, address_map_constructor internal_map);

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
		uint8_t m_intindex;
		uint8_t m_intreg;
		uint8_t m_intmask;
		uint8_t m_regindex;
		uint8_t m_regmask;
	};

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
	required_shared_ptr<uint8_t> m_r;
	uint32_t m_pc;

	// internal CPU state
	uint32_t m_addr_mask;
	bool m_interrupt_pending;

	// other internal states
	int m_icount;

	// memory access
	inline void push(uint8_t val);
	inline uint8_t pop();
	inline bool is_long_opcode(uint16_t op);

	// utility
	void unimplemented_opcode(uint32_t op);

	// interrupts
	void set_irq_line(uint16_t vector, int state);

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

	static const interrupt_condition s_int_conditions[INTIDX_COUNT];
	static const interrupt_condition s_mega644_int_conditions[INTIDX_COUNT];
};

// ======================> avr8_device

template <int NumTimers>
class avr8_device : public avr8_base_device
{
public:
	// GPIO
	template <gpio_t Port> auto gpio_out() { return m_gpio_out_cb[Port].bind(); }
	template <gpio_t Port> auto gpio_in() { return m_gpio_in_cb[Port].bind(); }
	template <int Port> uint8_t pin_r();
	template <int Port> void port_w(uint8_t data);
	template <int Port> uint8_t gpio_r();

	// ADC
	template<uint8_t Pin> auto adc_in() { return m_adc_in_cb[Pin].bind(); }

protected:
	avr8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, uint32_t address_mask, address_map_constructor internal_map);

	typedef delegate<void (void)> timer_func;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// address maps
	void base_internal_map(address_map &map) ATTR_COLD;

	// Miscellaneous registers
	void wdtcsr_w(uint8_t data);
	void clkpr_w(uint8_t data);
	void prr0_w(uint8_t data);
	void prr1_w(uint8_t data);
	void osccal_w(uint8_t data);
	void pcicr_w(uint8_t data);
	void eicra_w(uint8_t data);
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

	// SPI
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

	// timers
	void gtccr_w(uint8_t data);

	template <int Timer> void update_timer_clock_source(uint8_t selection, const uint8_t old_clock_select);
	void update_timer_waveform_gen_mode(uint8_t timer, uint8_t mode);

	int32_t m_timer_top[6];
	uint16_t m_timer_prescale[6];
	uint16_t m_timer_prescale_count[6];

	// timer 0
	void tccr0a_w(uint8_t data);
	void tccr0b_w(uint8_t data);
	void ocr0a_w(uint8_t data);
	void ocr0b_w(uint8_t data);
	void tifr0_w(uint8_t data);
	void timsk0_w(uint8_t data);

	void timer0_tick_norm();
	void timer0_tick_pwm_pc();
	void timer0_tick_ctc_norm();
	void timer0_tick_ctc_toggle();
	void timer0_tick_ctc_clear();
	void timer0_tick_ctc_set();
	void timer0_tick_fast_pwm();
	void timer0_tick_pwm_pc_cmp();
	void timer0_tick_fast_pwm_cmp();
	void timer0_tick_default();

	void update_ocr0(uint8_t newval, uint8_t reg);
	void timer0_force_output_compare(int reg);

	timer_func m_timer0_ticks[8*4];
	timer_func m_timer0_tick;

	// timer 1
	enum wgm1_mode_t
	{
		WGM1_NORMAL = 0,
		WGM1_PWM_8_PC,
		WGM1_PWM_9_PC,
		WGM1_PWM_10_PC,
		WGM1_CTC_OCR,
		WGM1_FAST_PWM_8,
		WGM1_FAST_PWM_9,
		WGM1_FAST_PWM_10,
		WGM1_PWM_PFC_ICR,
		WGM1_PWM_PFC_OCR,
		WGM1_PWM_PC_ICR,
		WGM1_PWM_PC_OCR,
		WGM1_CTC_ICR,
		WGM1_RESERVED,
		WGM1_FAST_PWM_ICR,
		WGM1_FAST_PWM_OCR
	};

	enum ocr_mode_t
	{
		OCR_NORM = 0,
		OCR_TOGGLE,
		OCR_CLEAR,
		OCR_SET
	};

	void tccr1a_w(uint8_t data);
	void tccr1b_w(uint8_t data);
	void tccr1c_w(uint8_t data);
	void tcnt1l_w(uint8_t data);
	void tcnt1h_w(uint8_t data);
	void icr1l_w(uint8_t data);
	void icr1h_w(uint8_t data);
	void ocr1al_w(uint8_t data);
	void ocr1ah_w(uint8_t data);
	void ocr1bl_w(uint8_t data);
	void ocr1bh_w(uint8_t data);
	void ocr1cl_w(uint8_t data);
	void ocr1ch_w(uint8_t data);
	void tifr1_w(uint8_t data);
	void timsk1_w(uint8_t data);

	template <int TimerMode, int ChannelModeA, int ChannelModeB> void timer1_tick();
	void timer1_tick_normal() { timer1_tick<WGM1_NORMAL, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_pwm8_pc() { timer1_tick<WGM1_PWM_8_PC, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_pwm9_pc() { timer1_tick<WGM1_PWM_9_PC, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_pwm10_pc() { timer1_tick<WGM1_PWM_10_PC, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_ctc_ocr_norm_norm() { timer1_tick<WGM1_CTC_OCR, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_ctc_ocr_norm_toggle() { timer1_tick<WGM1_CTC_OCR, OCR_NORM, OCR_TOGGLE>(); };
	void timer1_tick_ctc_ocr_norm_clear() { timer1_tick<WGM1_CTC_OCR, OCR_NORM, OCR_CLEAR>(); };
	void timer1_tick_ctc_ocr_norm_set() { timer1_tick<WGM1_CTC_OCR, OCR_NORM, OCR_SET>(); };
	void timer1_tick_ctc_ocr_toggle_norm() { timer1_tick<WGM1_CTC_OCR, OCR_TOGGLE, OCR_NORM>(); };
	void timer1_tick_ctc_ocr_toggle_toggle() { timer1_tick<WGM1_CTC_OCR, OCR_TOGGLE, OCR_TOGGLE>(); };
	void timer1_tick_ctc_ocr_toggle_clear() { timer1_tick<WGM1_CTC_OCR, OCR_TOGGLE, OCR_CLEAR>(); };
	void timer1_tick_ctc_ocr_toggle_set() { timer1_tick<WGM1_CTC_OCR, OCR_TOGGLE, OCR_SET>(); };
	void timer1_tick_ctc_ocr_clear_norm() { timer1_tick<WGM1_CTC_OCR, OCR_CLEAR, OCR_NORM>(); };
	void timer1_tick_ctc_ocr_clear_toggle() { timer1_tick<WGM1_CTC_OCR, OCR_CLEAR, OCR_TOGGLE>(); };
	void timer1_tick_ctc_ocr_clear_clear() { timer1_tick<WGM1_CTC_OCR, OCR_CLEAR, OCR_CLEAR>(); };
	void timer1_tick_ctc_ocr_clear_set() { timer1_tick<WGM1_CTC_OCR, OCR_CLEAR, OCR_SET>(); };
	void timer1_tick_ctc_ocr_set_norm() { timer1_tick<WGM1_CTC_OCR, OCR_SET, OCR_NORM>(); };
	void timer1_tick_ctc_ocr_set_toggle() { timer1_tick<WGM1_CTC_OCR, OCR_SET, OCR_TOGGLE>(); };
	void timer1_tick_ctc_ocr_set_clear() { timer1_tick<WGM1_CTC_OCR, OCR_SET, OCR_CLEAR>(); };
	void timer1_tick_ctc_ocr_set_set() { timer1_tick<WGM1_CTC_OCR, OCR_SET, OCR_SET>(); };
	void timer1_tick_fast_pwm8() { timer1_tick<WGM1_FAST_PWM_8, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_fast_pwm9() { timer1_tick<WGM1_FAST_PWM_9, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_fast_pwm10() { timer1_tick<WGM1_FAST_PWM_10, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_pwm_pfc_icr() { timer1_tick<WGM1_PWM_PFC_ICR, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_pwm_pfc_ocr() { timer1_tick<WGM1_PWM_PFC_OCR, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_pwm_pc_icr() { timer1_tick<WGM1_PWM_PC_ICR, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_pwm_pc_ocr() { timer1_tick<WGM1_PWM_PC_OCR, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_ctc_icr() { timer1_tick<WGM1_CTC_ICR, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_resv() { timer1_tick<WGM1_RESERVED, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_fast_pwm_icr_norm_norm() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_fast_pwm_icr_norm_toggle() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_NORM, OCR_TOGGLE>(); };
	void timer1_tick_fast_pwm_icr_norm_clear() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_NORM, OCR_CLEAR>(); };
	void timer1_tick_fast_pwm_icr_norm_set() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_NORM, OCR_SET>(); };
	void timer1_tick_fast_pwm_icr_toggle_norm() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_TOGGLE, OCR_NORM>(); };
	void timer1_tick_fast_pwm_icr_toggle_toggle() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_TOGGLE, OCR_TOGGLE>(); };
	void timer1_tick_fast_pwm_icr_toggle_clear() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_TOGGLE, OCR_CLEAR>(); };
	void timer1_tick_fast_pwm_icr_toggle_set() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_TOGGLE, OCR_SET>(); };
	void timer1_tick_fast_pwm_icr_clear_norm() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_CLEAR, OCR_NORM>(); };
	void timer1_tick_fast_pwm_icr_clear_toggle() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_CLEAR, OCR_TOGGLE>(); };
	void timer1_tick_fast_pwm_icr_clear_clear() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_CLEAR, OCR_CLEAR>(); };
	void timer1_tick_fast_pwm_icr_clear_set() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_CLEAR, OCR_SET>(); };
	void timer1_tick_fast_pwm_icr_set_norm() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_SET, OCR_NORM>(); };
	void timer1_tick_fast_pwm_icr_set_toggle() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_SET, OCR_TOGGLE>(); };
	void timer1_tick_fast_pwm_icr_set_clear() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_SET, OCR_CLEAR>(); };
	void timer1_tick_fast_pwm_icr_set_set() { timer1_tick<WGM1_FAST_PWM_ICR, OCR_SET, OCR_SET>(); };
	void timer1_tick_fast_pwm_ocr_norm_norm() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_NORM, OCR_NORM>(); };
	void timer1_tick_fast_pwm_ocr_norm_toggle() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_NORM, OCR_TOGGLE>(); };
	void timer1_tick_fast_pwm_ocr_norm_clear() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_NORM, OCR_CLEAR>(); };
	void timer1_tick_fast_pwm_ocr_norm_set() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_NORM, OCR_SET>(); };
	void timer1_tick_fast_pwm_ocr_toggle_norm() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_TOGGLE, OCR_NORM>(); };
	void timer1_tick_fast_pwm_ocr_toggle_toggle() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_TOGGLE, OCR_TOGGLE>(); };
	void timer1_tick_fast_pwm_ocr_toggle_clear() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_TOGGLE, OCR_CLEAR>(); };
	void timer1_tick_fast_pwm_ocr_toggle_set() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_TOGGLE, OCR_SET>(); };
	void timer1_tick_fast_pwm_ocr_clear_norm() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_CLEAR, OCR_NORM>(); };
	void timer1_tick_fast_pwm_ocr_clear_toggle() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_CLEAR, OCR_TOGGLE>(); };
	void timer1_tick_fast_pwm_ocr_clear_clear() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_CLEAR, OCR_CLEAR>(); };
	void timer1_tick_fast_pwm_ocr_clear_set() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_CLEAR, OCR_SET>(); };
	void timer1_tick_fast_pwm_ocr_set_norm() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_SET, OCR_NORM>(); };
	void timer1_tick_fast_pwm_ocr_set_toggle() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_SET, OCR_TOGGLE>(); };
	void timer1_tick_fast_pwm_ocr_set_clear() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_SET, OCR_CLEAR>(); };
	void timer1_tick_fast_pwm_ocr_set_set() { timer1_tick<WGM1_FAST_PWM_OCR, OCR_SET, OCR_SET>(); };
	void update_timer1_input_noise_canceler();
	void update_timer1_input_edge_select();
	void update_ocr1(uint16_t newval, uint8_t reg);

	timer_func m_timer1_ticks[16*16];
	timer_func m_timer1_tick;
	uint16_t m_ocr1[3];

	// timer 2
	void tccr2a_w(uint8_t data);
	void tccr2b_w(uint8_t data);
	void tcnt2_w(uint8_t data);
	void ocr2a_w(uint8_t data);
	void ocr2b_w(uint8_t data);
	void tifr2_w(uint8_t data);
	void timsk2_w(uint8_t data);

	void timer2_tick_default();
	void timer2_tick_norm();
	void timer2_tick_fast_pwm();
	void timer2_tick_fast_pwm_cmp();
	void update_ocr2(uint8_t newval, uint8_t reg);
	void timer2_force_output_compare(int reg);

	timer_func m_timer2_ticks[8];
	timer_func m_timer2_tick;
	bool m_ocr2_not_reached_yet;

	// timer 3
	void tccr3a_w(uint8_t data);
	void tccr3b_w(uint8_t data);
	void tccr3c_w(uint8_t data);
	void tcnt3l_w(uint8_t data);
	void tcnt3h_w(uint8_t data);
	void icr3l_w(uint8_t data);
	void icr3h_w(uint8_t data);
	void ocr3al_w(uint8_t data);
	void ocr3ah_w(uint8_t data);
	void ocr3bl_w(uint8_t data);
	void ocr3bh_w(uint8_t data);
	void ocr3cl_w(uint8_t data);
	void ocr3ch_w(uint8_t data);
	void timsk3_w(uint8_t data);

	void timer3_tick();

	// timer 4
	void tccr4a_w(uint8_t data);
	void tccr4b_w(uint8_t data);
	void tccr4c_w(uint8_t data);
	void tcnt4l_w(uint8_t data);
	void tcnt4h_w(uint8_t data);
	void icr4l_w(uint8_t data);
	void icr4h_w(uint8_t data);
	void ocr4al_w(uint8_t data);
	void ocr4ah_w(uint8_t data);
	void ocr4bl_w(uint8_t data);
	void ocr4bh_w(uint8_t data);
	void ocr4cl_w(uint8_t data);
	void ocr4ch_w(uint8_t data);
	void timsk4_w(uint8_t data);

	void timer4_tick();

	// timer 5
	void tccr5a_w(uint8_t data);
	void tccr5b_w(uint8_t data);
	void timsk5_w(uint8_t data);

	void timer5_tick();
};

// device type definition
DECLARE_DEVICE_TYPE(ATMEGA88,   atmega88_device)
DECLARE_DEVICE_TYPE(ATMEGA168,  atmega168_device)
DECLARE_DEVICE_TYPE(ATMEGA328,  atmega328_device)
DECLARE_DEVICE_TYPE(ATMEGA644,  atmega644_device)
DECLARE_DEVICE_TYPE(ATMEGA1280, atmega1280_device)
DECLARE_DEVICE_TYPE(ATMEGA2560, atmega2560_device)
DECLARE_DEVICE_TYPE(ATTINY15,   attiny15_device)

// ======================> atmega88_device

class atmega88_device : public avr8_device<3>
{
public:
	// construction/destruction
	atmega88_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void atmega88_internal_map(address_map &map) ATTR_COLD;
};

// ======================> atmega168_device

class atmega168_device : public avr8_device<3>
{
public:
	// construction/destruction
	atmega168_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void update_interrupt(int source) override;
	void atmega168_internal_map(address_map &map) ATTR_COLD;
};

// ======================> atmega328_device

class atmega328_device : public avr8_device<3>
{
public:
	// construction/destruction
	atmega328_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void update_interrupt(int source) override;
	void atmega328_internal_map(address_map &map) ATTR_COLD;
};

// ======================> atmega644_device

class atmega644_device : public avr8_device<3>
{
public:
	// construction/destruction
	atmega644_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void update_interrupt(int source) override;
	void atmega644_internal_map(address_map &map) ATTR_COLD;
};

// ======================> atmega1280_device

class atmega1280_device : public avr8_device<6>
{
public:
	// construction/destruction
	atmega1280_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void update_interrupt(int source) override;
	void atmega1280_internal_map(address_map &map) ATTR_COLD;
};

// ======================> atmega2560_device

class atmega2560_device : public avr8_device<6>
{
public:
	// construction/destruction
	atmega2560_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void update_interrupt(int source) override;
	void atmega2560_internal_map(address_map &map) ATTR_COLD;
};

// ======================> attiny15_device

class attiny15_device : public avr8_device<2>
{
public:
	// construction/destruction
	attiny15_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void attiny15_internal_map(address_map &map) ATTR_COLD;
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
