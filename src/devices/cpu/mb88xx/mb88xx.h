// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    mb88xx.h
    Core implementation for the portable Fujitsu MB88xx series MCU emulator.

    Written by Ernesto Corvi

***************************************************************************/

#ifndef MAME_CPU_MB88XX_MB88XX_H
#define MAME_CPU_MB88XX_MB88XX_H

#pragma once


/***************************************************************************
    PORT ENUMERATION
***************************************************************************/

enum
{
	MB88_PORTK = 0, /* input only, 4 bits */
	MB88_PORTO,     /* output only, PLA function output */
	MB88_PORTP,     /* 4 bits */
	MB88_PORTR0,    /* R0-R3, 4 bits */
	MB88_PORTR1,    /* R4-R7, 4 bits */
	MB88_PORTR2,    /* R8-R11, 4 bits */
	MB88_PORTR3,    /* R12-R15, 4 bits */
	MB88_PORTSI     /* SI, 1 bit */
};

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	MB88_PC=1,
	MB88_PA,
	MB88_FLAGS,
	MB88_SI,
	MB88_A,
	MB88_X,
	MB88_Y,
	MB88_PIO,
	MB88_TH,
	MB88_TL,
	MB88_SB
};

#define MB88_IRQ_LINE       0


CPU_DISASSEMBLE( mb88 );


// Configure 32 byte PLA, if nullptr (default) assume direct output */
#define MCFG_MB88_PLA(_pla) mb88_cpu_device::set_pla(*device, _pla);


class mb88_cpu_device : public cpu_device
{
public:
	// construction/destruction
	mb88_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	static void set_pla(device_t &device, uint8_t *pla) { downcast<mb88_cpu_device &>(device).m_PLA = pla; }

	DECLARE_WRITE_LINE_MEMBER( clock_w );

protected:
	mb88_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int program_width, int data_width);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 3; }
	virtual uint32_t execute_input_lines() const override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const override { return (clocks + 6 - 1) / 6; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const override { return (cycles * 6); }

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : (spacenum == AS_DATA) ? &m_data_config : (spacenum == AS_IO) ? &m_io_config : nullptr; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 1; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	uint8_t   m_PC;     /* Program Counter: 6 bits */
	uint8_t   m_PA;     /* Page Address: 4 bits */
	uint16_t  m_SP[4];  /* Stack is 4*10 bit addresses deep, but we also use 3 top bits per address to store flags during irq */
	uint8_t   m_SI;     /* Stack index: 2 bits */
	uint8_t   m_A;      /* Accumulator: 4 bits */
	uint8_t   m_X;      /* Index X: 4 bits */
	uint8_t   m_Y;      /* Index Y: 4 bits */
	uint8_t   m_st;     /* State flag: 1 bit */
	uint8_t   m_zf;     /* Zero flag: 1 bit */
	uint8_t   m_cf;     /* Carry flag: 1 bit */
	uint8_t   m_vf;     /* Timer overflow flag: 1 bit */
	uint8_t   m_sf;     /* Serial Full/Empty flag: 1 bit */
	uint8_t   m_nf;     /* Interrupt flag: 1 bit */

	/* Peripheral Control */
	uint8_t   m_pio; /* Peripheral enable bits: 8 bits */

	/* Timer registers */
	uint8_t   m_TH; /* Timer High: 4 bits */
	uint8_t   m_TL; /* Timer Low: 4 bits */
	uint8_t   m_TP; /* Timer Prescale: 6 bits? */
	uint8_t   m_ctr; /* current external counter value */

	/* Serial registers */
	uint8_t   m_SB; /* Serial buffer: 4 bits */
	uint16_t  m_SBcount;    /* number of bits received */
	emu_timer *m_serial;

	/* PLA configuration */
	uint8_t * m_PLA;

	/* IRQ handling */
	uint8_t m_pending_interrupt;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	address_space *m_io;
	int m_icount;

	// For the debugger
	uint16_t m_debugger_pc;
	uint8_t m_debugger_flags;

	TIMER_CALLBACK_MEMBER( serial_timer );
	int pla( int inA, int inB );
	void update_pio_enable( uint8_t newpio );
	void increment_timer();
	void update_pio( int cycles );

};


class mb88201_cpu_device : public mb88_cpu_device
{
public:
	// construction/destruction
	mb88201_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class mb88202_cpu_device : public mb88_cpu_device
{
public:
	// construction/destruction
	mb88202_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class mb8841_cpu_device : public mb88_cpu_device
{
public:
	// construction/destruction
	mb8841_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class mb8842_cpu_device : public mb88_cpu_device
{
public:
	// construction/destruction
	mb8842_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class mb8843_cpu_device : public mb88_cpu_device
{
public:
	// construction/destruction
	mb8843_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class mb8844_cpu_device : public mb88_cpu_device
{
public:
	// construction/destruction
	mb8844_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(MB88,    mb88_cpu_device)
DECLARE_DEVICE_TYPE(MB88201, mb88201_cpu_device)
DECLARE_DEVICE_TYPE(MB88202, mb88202_cpu_device)
DECLARE_DEVICE_TYPE(MB8841,  mb8841_cpu_device)
DECLARE_DEVICE_TYPE(MB8842,  mb8842_cpu_device)
DECLARE_DEVICE_TYPE(MB8843,  mb8843_cpu_device)
DECLARE_DEVICE_TYPE(MB8844,  mb8844_cpu_device)

#endif // MAME_CPU_MB88XX_MB88XX_H
