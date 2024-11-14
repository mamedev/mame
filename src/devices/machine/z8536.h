// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Zilog Z8036/Z8536 Counter/Timer and Parallel I/O Unit

**********************************************************************
                            _____   _____
                   AD4   1 |*    \_/     | 40  AD3
                   AD5   2 |             | 39  AD2
                   AD6   3 |             | 38  AD1
                   AD7   4 |             | 37  AD0
                   _DS   5 |             | 36  _CS0
                  R/_W   6 |             | 35  CS1
                   GND   7 |             | 34  _AS
                   PB0   8 |             | 33  PA0
                   PB1   9 |             | 32  PA1
                   PB2  10 |    Z8036    | 31  PA2
                   PB3  11 |             | 30  PA3
                   PB4  12 |             | 29  PA4
                   PB5  13 |             | 28  PA5
                   PB6  14 |             | 27  PA6
                   PB7  15 |             | 26  PA7
                  PCLK  16 |             | 25  _INTACK
                   IEI  17 |             | 24  _INT
                   IEO  18 |             | 23  +5 V
                   PC0  19 |             | 22  PC3
                   PC1  20 |_____________| 21  PC2

                            _____   _____
                    D4   1 |*    \_/     | 40  D3
                    D5   2 |             | 39  D2
                    D6   3 |             | 38  D1
                    D7   4 |             | 37  D0
                   _RD   5 |             | 36  _CE
                   _WR   6 |             | 35  A1
                   GND   7 |             | 34  A0
                   PB0   8 |             | 33  PA0
                   PB1   9 |             | 32  PA1
                   PB2  10 |    Z8536    | 31  PA2
                   PB3  11 |             | 30  PA3
                   PB4  12 |             | 29  PA4
                   PB5  13 |             | 28  PA5
                   PB6  14 |             | 27  PA6
                   PB7  15 |             | 26  PA7
                  PCLK  16 |             | 25  _INTACK
                   IEI  17 |             | 24  _INT
                   IEO  18 |             | 23  +5 V
                   PC0  19 |             | 22  PC3
                   PC1  20 |_____________| 21  PC2

**********************************************************************/

#ifndef MAME_MACHINE_Z8536_H
#define MAME_MACHINE_Z8536_H

#pragma once

#include "machine/z80daisy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cio_base_device

class cio_base_device : public device_t
{
public:

	auto irq_wr_cb() { return m_write_irq.bind(); }
	auto pa_rd_cb() { return m_read_pa.bind(); }
	auto pa_wr_cb() { return m_write_pa.bind(); }
	auto pb_rd_cb() { return m_read_pb.bind(); }
	auto pb_wr_cb() { return m_write_pb.bind(); }
	auto pc_rd_cb() { return m_read_pc.bind(); }
	auto pc_wr_cb() { return m_write_pc.bind(); }

	void pa0_w(int state) { external_port_w(PORT_A, 0, state); }
	void pa1_w(int state) { external_port_w(PORT_A, 1, state); }
	void pa2_w(int state) { external_port_w(PORT_A, 2, state); }
	void pa3_w(int state) { external_port_w(PORT_A, 3, state); }
	void pa4_w(int state) { external_port_w(PORT_A, 4, state); }
	void pa5_w(int state) { external_port_w(PORT_A, 5, state); }
	void pa6_w(int state) { external_port_w(PORT_A, 6, state); }
	void pa7_w(int state) { external_port_w(PORT_A, 7, state); }

	void pb0_w(int state) { external_port_w(PORT_B, 0, state); }
	void pb1_w(int state) { external_port_w(PORT_B, 1, state); }
	void pb2_w(int state) { external_port_w(PORT_B, 2, state); }
	void pb3_w(int state) { external_port_w(PORT_B, 3, state); }
	void pb4_w(int state) { external_port_w(PORT_B, 4, state); }
	void pb5_w(int state) { external_port_w(PORT_B, 5, state); }
	void pb6_w(int state) { external_port_w(PORT_B, 6, state); }
	void pb7_w(int state) { external_port_w(PORT_B, 7, state); }

	void pc0_w(int state) { external_port_w(PORT_C, 0, state); }
	void pc1_w(int state) { external_port_w(PORT_C, 1, state); }
	void pc2_w(int state) { external_port_w(PORT_C, 2, state); }
	void pc3_w(int state) { external_port_w(PORT_C, 3, state); }

	int intack_r();

protected:
	// construction/destruction
	cio_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(advance_counters);

	bool is_reset() const { return (m_register[MASTER_INTERRUPT_CONTROL] & MICR_RESET) != 0; }

	enum : int
	{
		TIMER_1 = 0,
		TIMER_2,
		TIMER_3
	};

	// ports
	enum : int
	{
		PORT_A = 0,
		PORT_B,
		PORT_C,
		CONTROL
	};


	// registers
	enum
	{
		MASTER_INTERRUPT_CONTROL = 0x00,
		MASTER_CONFIGURATION_CONTROL,
		PORT_A_INTERRUPT_VECTOR,
		PORT_B_INTERRUPT_VECTOR,
		COUNTER_TIMER_INTERRUPT_VECTOR,
		PORT_C_DATA_PATH_POLARITY,
		PORT_C_DATA_DIRECTION,
		PORT_C_SPECIAL_IO_CONTROL,
		PORT_A_COMMAND_AND_STATUS,
		PORT_B_COMMAND_AND_STATUS,
		COUNTER_TIMER_1_COMMAND_AND_STATUS,
		COUNTER_TIMER_2_COMMAND_AND_STATUS,
		COUNTER_TIMER_3_COMMAND_AND_STATUS,
		PORT_A_DATA,
		PORT_B_DATA,
		PORT_C_DATA,
		COUNTER_TIMER_1_CURRENT_COUNT_MS_BYTE = 0x10,
		COUNTER_TIMER_1_CURRENT_COUNT_LS_BYTE,
		COUNTER_TIMER_2_CURRENT_COUNT_MS_BYTE,
		COUNTER_TIMER_2_CURRENT_COUNT_LS_BYTE,
		COUNTER_TIMER_3_CURRENT_COUNT_MS_BYTE,
		COUNTER_TIMER_3_CURRENT_COUNT_LS_BYTE,
		COUNTER_TIMER_1_TIME_CONSTANT_MS_BYTE,
		COUNTER_TIMER_1_TIME_CONSTANT_LS_BYTE,
		COUNTER_TIMER_2_TIME_CONSTANT_MS_BYTE,
		COUNTER_TIMER_2_TIME_CONSTANT_LS_BYTE,
		COUNTER_TIMER_3_TIME_CONSTANT_MS_BYTE,
		COUNTER_TIMER_3_TIME_CONSTANT_LS_BYTE,
		COUNTER_TIMER_1_MODE_SPECIFICATION,
		COUNTER_TIMER_2_MODE_SPECIFICATION,
		COUNTER_TIMER_3_MODE_SPECIFICATION,
		CURRENT_VECTOR,
		PORT_A_MODE_SPECIFICATION = 0x20,
		PORT_A_HANDSHAKE_SPECIFICATION,
		PORT_A_DATA_PATH_POLARITY,
		PORT_A_DATA_DIRECTION,
		PORT_A_SPECIAL_IO_CONTROL,
		PORT_A_PATTERN_POLARITY,
		PORT_A_PATTERN_TRANSITION,
		PORT_A_PATTERN_MASK,
		PORT_B_MODE_SPECIFICATION,
		PORT_B_HANDSHAKE_SPECIFICATION,
		PORT_B_DATA_PATH_POLARITY,
		PORT_B_DATA_DIRECTION,
		PORT_B_SPECIAL_IO_CONTROL,
		PORT_B_PATTERN_POLARITY,
		PORT_B_PATTERN_TRANSITION,
		PORT_B_PATTERN_MASK
	};


	// master interrupt control register
	enum
	{
		MICR_RESET    = 0x01,   // reset
		MICR_RJA      = 0x02,   // right justified address
		MICR_CT_VIS   = 0x04,   // counter/timer vector includes status
		MICR_PB_VIS   = 0x08,   // port B vector includes status
		MICR_PA_VIS   = 0x10,   // port A vector includes status
		MICR_NV       = 0x20,   // no vector
		MICR_DLC      = 0x40,   // disable lower chain
		MICR_MIE      = 0x80    // master interrupt enable
	};


	// master configuration control register
	enum
	{
		MCCR_LC_MASK  = 0x03,   // counter/timer link controls
		MCCR_PAE      = 0x04,   // port A enable
		MCCR_PLC      = 0x08,   // port link control
		MCCR_PCE_CT3E = 0x10,   // port C and counter/timer 3 enable
		MCCR_CT2E     = 0x20,   // counter/timer 2 enable
		MCCR_CT1E     = 0x40,   // counter/timer 1 enable
		MCCR_PBE      = 0x80    // port B enable
	};


	// port mode specification registers
	enum
	{
		PMS_LPM       = 0x01,   // latch on pattern match
		PMS_DTE       = 0x01,   // deskew timer enable
		PMS_PMS_MASK  = 0x06,   // pattern mode specification
		PMS_IMO       = 0x08,   // interrupt on match only
		PMS_SB        = 0x10,   // single buffer
		PMS_ITB       = 0x20,   // interrupt on two bytes
		PMS_PTS_MASK  = 0xc0    // port type select
	};


	// port handshake specification registers
	enum
	{
		PHS_DTS_MASK  = 0x07,   // deskew time specification
		PHS_RWS_MASK  = 0x38,   // request/wait specification
		PHS_HTS_MASK  = 0xc0    // handshake type specification
	};


	// port command and status registers
	enum
	{
		PCS_IOE       = 0x01,   // interrupt on error
		PCS_PMF       = 0x02,   // pattern match flag (read only)
		PCS_IRF       = 0x04,   // input register full (read only)
		PCS_ORE       = 0x08,   // output register empty (read only)
		PCS_ERR       = 0x10,   // interrupt error (read only)
		PCS_IP        = 0x20,   // interrupt pending
		PCS_IE        = 0x40,   // interrupt enable
		PCS_IUS       = 0x80    // interrupt under service
	};


	// counter/timer mode specification registers
	enum
	{
		CTMS_DCS_MASK = 0x03,   // output duty cycle
		CTMS_REB      = 0x04,   // retrigger enable bit
		CTMS_EDE      = 0x08,   // external gate enable
		CTMS_ETE      = 0x10,   // external trigger enable
		CTMS_ECE      = 0x20,   // external count enable
		CTMS_EOE      = 0x40,   // external output enable
		CTMS_CSC      = 0x80    // continuous/single cycle
	};


	// counter/timer command and status registers
	enum
	{
		CTCS_CIP      = 0x01,   // count in progress (read only)
		CTCS_TCB      = 0x02,   // trigger command bit (write only - read returns 0)
		CTCS_GCB      = 0x04,   // gate command bit
		CTCS_RCC      = 0x08,   // read counter control (read/set only - cleared by reading CCR LSB)
		CTCS_ERR      = 0x10,   // interrupt error (read only)
		CTCS_IP       = 0x20,   // interrupt pending
		CTCS_IE       = 0x40,   // interrupt enable
		CTCS_IUS      = 0x80    // interrupt under service
	};


	// interrupt control
	enum
	{
		IC_NULL = 0,
		IC_CLEAR_IP_IUS,
		IC_SET_IUS,
		IC_CLEAR_IUS,
		IC_SET_IP,
		IC_CLEAR_IP,
		IC_SET_IE,
		IC_CLEAR_IE
	};


	// counter/timer link control
	enum
	{
		LC_INDEPENDENT = 0,
		LC_CT1_GATES_CT2,
		LC_CT1_TRIGGERS_CT2,
		LC_CT1_COUNTS_CT2
	};


	// port type select
	enum
	{
		PTS_BIT = 0,
		PTS_INPUT,
		PTS_OUTPUT,
		PTS_BIDIRECTIONAL
	};



	// pattern mode specification
	enum
	{
		PMS_DISABLE = 0,
		PMS_AND,
		PMS_OR,
		PMS_OR_PEV
	};

	// handshake specification
	enum
	{
		HTS_INTERLOCKED = 0,
		HTS_STROBED,
		HTS_PULSED,
		HTS_3_WIRE
	};


	// request/wait specification
	enum
	{
		RWS_DISABLED = 0,
		RWS_OUTPUT_WAIT,
		RWS_INPUT_WAIT = 3,
		RWS_SPECIAL_REQUEST,
		RWS_OUTPUT_REQUEST,
		RWS_INPUT_REQUEST = 7
	};


	// pattern specification
	enum
	{
		BIT_MASKED_OFF = 0,
		ANY_TRANSITION,
		ZERO = 4,
		ONE,
		ONE_TO_ZERO,
		ZERO_TO_ONE
	};


	// output duty cycle
	enum
	{
		DCS_PULSE,
		DCS_ONE_SHOT,
		DCS_SQUARE_WAVE,
		DCS_DO_NOT_USE
	};

	void get_interrupt_vector();
	void check_interrupt();

	u8 read_register(offs_t offset);
	u8 read_register(offs_t offset, u8 mask);
	void write_register(offs_t offset, u8 data);
	void write_register(offs_t offset, u8 data, u8 mask);

	bool counter_enabled(int id);
	bool counter_external_output(int id);
	bool counter_external_count(int id);
	bool counter_external_trigger(int id);
	bool counter_external_gate(int id);
	bool counter_gated(int id);
	void count(int id);
	void trigger(int id);
	void gate(int id, int state);
	void match_pattern(int port);
	void external_port_w(int port, int bit, int state);

	devcb_write_line       m_write_irq;

	devcb_read8            m_read_pa;
	devcb_write8           m_write_pa;

	devcb_read8            m_read_pb;
	devcb_write8           m_write_pb;

	devcb_read8            m_read_pc;
	devcb_write8           m_write_pc;

	// interrupt state
	int m_irq;

	// register state
	u8 m_register[48];

	// input/output port state
	u8 m_input[3];
	u8 m_output[3];
	u8 m_buffer[3];
	u8 m_match[3];

	// timers
	emu_timer *m_timer;
	u16 m_counter[3];
};

// ======================> z8036_device

class z8036_device : public cio_base_device
{
public:
	// construction/destruction
	z8036_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
};

// ======================> z8536_device

class z8536_device : public cio_base_device, public device_z80daisy_interface
{
public:
	// construction/destruction
	z8536_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

private:
	// direct external access to ports
	enum
	{
		EXT_PORT_C = 0,
		EXT_PORT_B,
		EXT_PORT_A,
		EXT_CONTROL
	};

	// control state machine
	bool m_state0;
	u8 m_pointer;
};


// device type definition
DECLARE_DEVICE_TYPE(Z8036, z8036_device)
DECLARE_DEVICE_TYPE(Z8536, z8536_device)

#endif // MAME_MACHINE_Z8536_H
