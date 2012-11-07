/*
    Atmel 8-bit AVR simulator

    - Notes -
      Cycle counts are generally considered to be 100% accurate per-instruction, does not support mid-instruction
      interrupts although no software has been countered yet that requires it. Evidence of cycle accuracy is given
      in the form of the demoscene 'wild' demo, Craft, by [lft], which uses an ATmega88 to write video out a 6-bit
      RGB DAC pixel-by-pixel, synchronously with the frame timing. Intentionally modifying the timing of any of
      the existing opcodes has been shown to wildly corrupt the video output in Craft, so one can assume that the
      existing timing is 100% correct.

	  Unimplemented opcodes: CPSR, LD Z+, ST Z+, ST -Z/-Y/-X, ELPM, SPM, SPM Z+, EIJMP, SLEEP, BREAK, WDR, ICALL,
	                         EICALL, JMP, CALL, SBIW

	- Changelist -
	  30 Oct. 2012
	  - Added FMUL, FMULS, FMULSU opcodes [MooglyGuy]
	  - Fixed incorrect flag calculation in ROR opcode [MooglyGuy]
	  - Fixed incorrect bit testing in SBIC/SBIS opcodes [MooglyGuy]

	  25 Oct. 2012
	  - Added MULS, ANDI, STI Z+, LD -Z, LD -Y, LD -X, LD Y+q, LD Z+q, SWAP, ASR, ROR and SBIS opcodes [MooglyGuy]
	  - Corrected cycle counts for LD and ST opcodes [MooglyGuy]
	  - Moved opcycles init into inner while loop, fixes 2-cycle and 3-cycle opcodes effectively forcing
	    all subsequent 1-cycle opcodes to be 2 or 3 cycles [MooglyGuy]
	  - Fixed register behavior in MULSU, LD -Z, and LD -Y opcodes [MooglyGuy]

	  18 Oct. 2012
	  - Added OR, SBCI, ORI, ST Y+, ADIQ opcodes [MooglyGuy]
	  - Fixed COM, NEG, LSR opcodes [MooglyGuy]

*/

#pragma once

#ifndef __AVR8_H__
#define __AVR8_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CPU_AVR8_CONFIG(_config) \
	avr8_device::static_set_config(*device, _config); \

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class avr8_device;

// ======================> avr8_config

struct avr8_config
{
	const char *eeprom_region;
};


// ======================> avr8_device

// Used by core CPU interface
class avr8_device : public cpu_device,
					public avr8_config
{
public:
	// construction/destruction
	avr8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, const device_type type, UINT32 address_mask);

	// inline configuration helpers
	static void static_set_config(device_t &device, const avr8_config &config);

	// public interfaces
	void update_interrupt(int source);
	UINT64 get_elapsed_cycles()
	{
		return m_elapsed_cycles;
	}

	// register handling
	DECLARE_WRITE8_MEMBER( regs_w );
	DECLARE_READ8_MEMBER( regs_r );

protected:
	avr8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, const device_type type, UINT32 address_mask, address_map_constructor internal_map);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const;
	virtual UINT32 execute_max_cycles() const;
	virtual UINT32 execute_input_lines() const;
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, astring &string);

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_data_config;
	const address_space_config m_io_config;
    UINT8 *m_eeprom;

	// CPU registers
    UINT32 m_pc;
    UINT32 m_shifted_pc;
	UINT8 m_r[256];

	// internal timers
    UINT8 m_timer0_top;
	INT32 m_timer0_increment;
	UINT16 m_timer0_prescale;
	UINT16 m_timer0_prescale_count;

    UINT16 m_timer1_top;
	INT32 m_timer1_increment;
	UINT16 m_timer1_prescale;
	UINT16 m_timer1_prescale_count;

    UINT8 m_timer2_top;
	INT32 m_timer2_increment;
	UINT16 m_timer2_prescale;
	UINT16 m_timer2_prescale_count;

	// SPI
	bool m_spi_active;
	UINT8 m_spi_prescale;
	UINT8 m_spi_prescale_count;
	INT8 m_spi_prescale_countdown;
    static const UINT8 spi_clock_divisor[8];
	void enable_spi();
	void disable_spi();
	void spi_update_masterslave_select();
	void spi_update_clock_polarity();
	void spi_update_clock_phase();
	void spi_update_clock_rate();
	void change_spcr(UINT8 data);
	void change_spsr(UINT8 data);

    // internal CPU state
	UINT32 m_addr_mask;
	bool m_interrupt_pending;

	// other internal states
    int m_icount;
	UINT64 m_elapsed_cycles;

	// memory access
	inline UINT8 program_read8(UINT32 addr);
	inline UINT16 program_read16(UINT32 addr);
	inline void program_write8(UINT32 addr, UINT8 data);
	inline void program_write16(UINT32 addr, UINT16 data);
	inline UINT8 io_read8(UINT16 addr);
	inline void io_write8(UINT16 addr, UINT8 data);
	inline UINT16 opcode_read();
	inline void push(UINT8 val);
	inline UINT8 pop();
	inline bool is_long_opcode(UINT16 op);

	// utility
	void unimplemented_opcode(UINT32 op);

	// interrupts
	void set_irq_line(UINT16 vector, int state);
	void update_interrupt_internal(int source);

	// timers
	void timer_tick(int cycles);

	// timer 0
	void timer0_tick();
	void changed_tccr0a(UINT8 data);
	void changed_tccr0b(UINT8 data);
	void update_timer0_waveform_gen_mode();
	void update_timer0_clock_source();
	void update_ocr0(UINT8 newval, UINT8 reg);
	void timer0_force_output_compare(int reg);

	// timer 1
	void timer1_tick();
	void changed_tccr1a(UINT8 data);
	void changed_tccr1b(UINT8 data);
	void update_timer1_waveform_gen_mode();
	void update_timer1_clock_source();
	void update_timer1_input_noise_canceler();
	void update_timer1_input_edge_select();
	void update_ocr1(UINT16 newval, UINT8 reg);

	// timer 2
	void timer2_tick();
	void changed_tccr2a(UINT8 data);
	void changed_tccr2b(UINT8 data);
	void update_timer2_waveform_gen_mode();
	void update_timer2_clock_source();
	void update_ocr2(UINT8 newval, UINT8 reg);
	void timer2_force_output_compare(int reg);

	// address spaces
    address_space *m_program;
    address_space *m_data;
    address_space *m_io;
};

// device type definition
extern const device_type ATMEGA88;
extern const device_type ATMEGA644;

// ======================> atmega88_device

class atmega88_device : public avr8_device
{
public:
	// construction/destruction
	atmega88_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

// ======================> atmega644_device

class atmega644_device : public avr8_device
{
public:
	// construction/destruction
	atmega644_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
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
    AVR8_SP,
};

enum
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
enum
{
	AVR8_REGIDX_R0 = 0,
	AVR8_REGIDX_R1,
	AVR8_REGIDX_R2,
	AVR8_REGIDX_R3,
	AVR8_REGIDX_R4,
	AVR8_REGIDX_R5,
	AVR8_REGIDX_R6,
	AVR8_REGIDX_R7,
	AVR8_REGIDX_R8,
	AVR8_REGIDX_R9,
	AVR8_REGIDX_R10,
	AVR8_REGIDX_R11,
	AVR8_REGIDX_R12,
	AVR8_REGIDX_R13,
	AVR8_REGIDX_R14,
	AVR8_REGIDX_R15,
	AVR8_REGIDX_R16,
	AVR8_REGIDX_R17,
	AVR8_REGIDX_R18,
	AVR8_REGIDX_R19,
	AVR8_REGIDX_R20,
	AVR8_REGIDX_R21,
	AVR8_REGIDX_R22,
	AVR8_REGIDX_R23,
	AVR8_REGIDX_R24,
	AVR8_REGIDX_R25,
	AVR8_REGIDX_R26,
	AVR8_REGIDX_R27,
	AVR8_REGIDX_R28,
	AVR8_REGIDX_R29,
	AVR8_REGIDX_R30,
	AVR8_REGIDX_R31,

	AVR8_REGIDX_PINA = 0x20,
	AVR8_REGIDX_DDRA,
	AVR8_REGIDX_PORTA,
	AVR8_REGIDX_PINB,
	AVR8_REGIDX_DDRB,
	AVR8_REGIDX_PORTB,
	AVR8_REGIDX_PINC,
	AVR8_REGIDX_DDRC,
	AVR8_REGIDX_PORTC,
	AVR8_REGIDX_PIND,
	AVR8_REGIDX_DDRD,
	AVR8_REGIDX_PORTD,

	AVR8_REGIDX_TIFR0 = 0x35,
	AVR8_REGIDX_TIFR1,
	AVR8_REGIDX_TIFR2,

	AVR8_REGIDX_PCIFR = 0x3b,
	AVR8_REGIDX_EIFR,
	AVR8_REGIDX_EIMSK,
	AVR8_REGIDX_GPIOR0,
	AVR8_REGIDX_EECR,
	AVR8_REGIDX_EEDR,
	AVR8_REGIDX_EEARL,
	AVR8_REGIDX_EEARH,
	AVR8_REGIDX_GTCCR,
	AVR8_REGIDX_TCCR0A,
	AVR8_REGIDX_TCCR0B,
	AVR8_REGIDX_TCNT0,
	AVR8_REGIDX_OCR0A,
	AVR8_REGIDX_OCR0B,

	AVR8_REGIDX_GPIOR1 = 0x4a,
	AVR8_REGIDX_GPIOR2,
	AVR8_REGIDX_SPCR,
	AVR8_REGIDX_SPSR,
	AVR8_REGIDX_SPDR,

	AVR8_REGIDX_ACSR = 0x50,

	AVR8_REGIDX_SMCR = 0x53,
	AVR8_REGIDX_MCUSR,
	AVR8_REGIDX_MCUCR,

	AVR8_REGIDX_SPMCSR = 0x57,

	AVR8_REGIDX_SPL = 0x5d,
	AVR8_REGIDX_SPH,
	AVR8_REGIDX_SREG,
	AVR8_REGIDX_WDTCSR,
	AVR8_REGIDX_CLKPR,

	AVR8_REGIDX_PRR = 0x64,

	AVR8_REGIDX_OSCCAL = 0x66,

	AVR8_REGIDX_PCICR = 0x68,
	AVR8_REGIDX_EICRA,

	AVR8_REGIDX_PCMSK0 = 0x6B,
	AVR8_REGIDX_PCMSK1,
	AVR8_REGIDX_PCMSK2,
	AVR8_REGIDX_TIMSK0,
	AVR8_REGIDX_TIMSK1,
	AVR8_REGIDX_TIMSK2,

	AVR8_REGIDX_ADCL = 0x78,
	AVR8_REGIDX_ADCH,
	AVR8_REGIDX_ADCSRA,
	AVR8_REGIDX_ADCSRB,
	AVR8_REGIDX_ADMUX,

	AVR8_REGIDX_DIDR0 = 0x7e,
	AVR8_REGIDX_DIDR1,
	AVR8_REGIDX_TCCR1A,
	AVR8_REGIDX_TCCR1B,
	AVR8_REGIDX_TCCR1C,

	AVR8_REGIDX_TCNT1L = 0x84,
	AVR8_REGIDX_TCNT1H,
	AVR8_REGIDX_ICR1L,
	AVR8_REGIDX_ICR1H,
	AVR8_REGIDX_OCR1AL,
	AVR8_REGIDX_OCR1AH,
	AVR8_REGIDX_OCR1BL,
	AVR8_REGIDX_OCR1BH,

	AVR8_REGIDX_TCCR2A = 0xb0,
	AVR8_REGIDX_TCCR2B,
	AVR8_REGIDX_TCNT2,
	AVR8_REGIDX_OCR2A,
	AVR8_REGIDX_OCR2B,

	AVR8_REGIDX_ASSR = 0xb6,

	AVR8_REGIDX_TWBR = 0xb8,
	AVR8_REGIDX_TWSR,
	AVR8_REGIDX_TWAR,
	AVR8_REGIDX_TWDR,
	AVR8_REGIDX_TWCR,
	AVR8_REGIDX_TWAMR,

	AVR8_REGIDX_UCSR0A = 0xc0,
	AVR8_REGIDX_UCSR0B,
	AVR8_REGIDX_UCSR0C,

	AVR8_REGIDX_UBRR0L = 0xc4,
	AVR8_REGIDX_UBRR0H,
	AVR8_REGIDX_UDR0
};

enum
{
	AVR8_REG_A = 0,
	AVR8_REG_B,
	AVR8_REG_C,
	AVR8_REG_D,
};

enum
{
	AVR8_INTIDX_SPI,
	AVR8_INTIDX_OCF0B,
	AVR8_INTIDX_OCF0A,
	AVR8_INTIDX_TOV0,
	AVR8_INTIDX_ICF1,
	AVR8_INTIDX_OCF1B,
	AVR8_INTIDX_OCF1A,
	AVR8_INTIDX_TOV1,
	AVR8_INTIDX_OCF2B,
	AVR8_INTIDX_OCF2A,
	AVR8_INTIDX_TOV2,

	AVR8_INTIDX_COUNT,
};

#define AVR8_EECR_EERE			0x01

#define AVR8_EEARH_MASK			0x01

#define AVR8_SPSR_SPIF_MASK		0x80
#define AVR8_SPSR_SPIF_SHIFT	7
#define AVR8_SPSR_SPR2X_MASK	0x01

#define AVR8_SPCR_SPIE_MASK		0x80
#define AVR8_SPCR_SPE_MASK		0x40
#define AVR8_SPCR_DORD_MASK		0x20
#define AVR8_SPCR_MSTR_MASK		0x10
#define AVR8_SPCR_CPOL_MASK		0x08
#define AVR8_SPCR_CPHA_MASK		0x04
#define AVR8_SPCR_SPR_MASK		0x03

CPU_DISASSEMBLE( avr8 );

#endif /* __AVR8_H__ */
