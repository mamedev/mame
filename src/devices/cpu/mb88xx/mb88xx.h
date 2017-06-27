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
    PORT CONFIGURATION
***************************************************************************/

// K (K3-K0): input-only port
#define MCFG_MB88XX_READ_K_CB(_devcb) \
	devcb = &mb88_cpu_device::set_read_k_callback(*device, DEVCB_##_devcb);

// O (O7-O4 = OH, O3-O0 = OL): output through PLA
#define MCFG_MB88XX_WRITE_O_CB(_devcb) \
	devcb = &mb88_cpu_device::set_write_o_callback(*device, DEVCB_##_devcb);

// P (P3-P0): output-only port
#define MCFG_MB88XX_WRITE_P_CB(_devcb) \
	devcb = &mb88_cpu_device::set_write_p_callback(*device, DEVCB_##_devcb);

// R0 (R3-R0): input/output port
#define MCFG_MB88XX_READ_R0_CB(_devcb) \
	devcb = &mb88_cpu_device::set_read_r_callback(*device, 0, DEVCB_##_devcb);
#define MCFG_MB88XX_WRITE_R0_CB(_devcb) \
	devcb = &mb88_cpu_device::set_write_r_callback(*device, 0, DEVCB_##_devcb);

// R1 (R7-R4): input/output port
#define MCFG_MB88XX_READ_R1_CB(_devcb) \
	devcb = &mb88_cpu_device::set_read_r_callback(*device, 1, DEVCB_##_devcb);
#define MCFG_MB88XX_WRITE_R1_CB(_devcb) \
	devcb = &mb88_cpu_device::set_write_r_callback(*device, 1, DEVCB_##_devcb);

// R2 (R11-R8): input/output port
#define MCFG_MB88XX_READ_R2_CB(_devcb) \
	devcb = &mb88_cpu_device::set_read_r_callback(*device, 2, DEVCB_##_devcb);
#define MCFG_MB88XX_WRITE_R2_CB(_devcb) \
	devcb = &mb88_cpu_device::set_write_r_callback(*device, 2, DEVCB_##_devcb);

// R3 (R15-R12): input/output port
#define MCFG_MB88XX_READ_R3_CB(_devcb) \
	devcb = &mb88_cpu_device::set_read_r_callback(*device, 3, DEVCB_##_devcb);
#define MCFG_MB88XX_WRITE_R3_CB(_devcb) \
	devcb = &mb88_cpu_device::set_write_r_callback(*device, 3, DEVCB_##_devcb);

// SI: serial input
#define MCFG_MB88XX_READ_SI_CB(_devcb) \
	devcb = &mb88_cpu_device::set_read_si_callback(*device, DEVCB_##_devcb);

// SO: serial output
#define MCFG_MB88XX_WRITE_SO_CB(_devcb) \
	devcb = &mb88_cpu_device::set_write_so_callback(*device, DEVCB_##_devcb);

// Configure 32 byte PLA; if nullptr (default) assume direct output
#define MCFG_MB88XX_OUTPUT_PLA(_pla) \
	mb88_cpu_device::set_pla(*device, _pla);

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


class mb88_cpu_device : public cpu_device
{
public:
	// static configuration helpers
	template <class Object> static devcb_base &set_read_k_callback(device_t &device, Object &&cb) { return downcast<mb88_cpu_device &>(device).m_read_k.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_write_o_callback(device_t &device, Object &&cb) { return downcast<mb88_cpu_device &>(device).m_write_o.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_write_p_callback(device_t &device, Object &&cb) { return downcast<mb88_cpu_device &>(device).m_write_p.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_read_r_callback(device_t &device, int n, Object &&cb) { assert(n >= 0 && n < 4); return downcast<mb88_cpu_device &>(device).m_read_r[n].set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_write_r_callback(device_t &device, int n, Object &&cb) { assert(n >= 0 && n < 4); return downcast<mb88_cpu_device &>(device).m_write_r[n].set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_read_si_callback(device_t &device, Object &&cb) { return downcast<mb88_cpu_device &>(device).m_read_si.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_write_so_callback(device_t &device, Object &&cb) { return downcast<mb88_cpu_device &>(device).m_write_so.set_callback(std::forward<Object>(cb)); }
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
	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : (spacenum == AS_DATA) ? &m_data_config : nullptr; }

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

	/* PLA configuration and port callbacks */
	uint8_t * m_PLA;
	devcb_read8 m_read_k;
	devcb_write8 m_write_o;
	devcb_write8 m_write_p;
	devcb_read8 m_read_r[4];
	devcb_write8 m_write_r[4];
	devcb_read_line m_read_si;
	devcb_write_line m_write_so;

	/* IRQ handling */
	uint8_t m_pending_interrupt;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
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


DECLARE_DEVICE_TYPE(MB88201, mb88201_cpu_device)
DECLARE_DEVICE_TYPE(MB88202, mb88202_cpu_device)
DECLARE_DEVICE_TYPE(MB8841,  mb8841_cpu_device)
DECLARE_DEVICE_TYPE(MB8842,  mb8842_cpu_device)
DECLARE_DEVICE_TYPE(MB8843,  mb8843_cpu_device)
DECLARE_DEVICE_TYPE(MB8844,  mb8844_cpu_device)

#endif // MAME_CPU_MB88XX_MB88XX_H
