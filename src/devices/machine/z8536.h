// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Zilog Z8536 Counter/Timer and Parallel I/O emulation

**********************************************************************
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

#pragma once

#ifndef __Z8536__
#define __Z8536__

#include "emu.h"
#include "cpu/z80/z80daisy.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_Z8536_IRQ_CALLBACK(_write) \
	devcb = &z8536_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_Z8536_PA_IN_CALLBACK(_read) \
	devcb = &z8536_device::set_pa_rd_callback(*device, DEVCB_##_read);

#define MCFG_Z8536_PA_OUT_CALLBACK(_write) \
	devcb = &z8536_device::set_pa_wr_callback(*device, DEVCB_##_write);

#define MCFG_Z8536_PB_IN_CALLBACK(_read) \
	devcb = &z8536_device::set_pb_rd_callback(*device, DEVCB_##_read);

#define MCFG_Z8536_PB_OUT_CALLBACK(_write) \
	devcb = &z8536_device::set_pb_wr_callback(*device, DEVCB_##_write);

#define MCFG_Z8536_PC_IN_CALLBACK(_read) \
	devcb = &z8536_device::set_pc_rd_callback(*device, DEVCB_##_read);

#define MCFG_Z8536_PC_OUT_CALLBACK(_write) \
	devcb = &z8536_device::set_pc_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z8536_device

class z8536_device :  public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	z8536_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<z8536_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_pa_rd_callback(device_t &device, _Object object) { return downcast<z8536_device &>(device).m_read_pa.set_callback(object); }
	template<class _Object> static devcb_base &set_pa_wr_callback(device_t &device, _Object object) { return downcast<z8536_device &>(device).m_write_pa.set_callback(object); }
	template<class _Object> static devcb_base &set_pb_rd_callback(device_t &device, _Object object) { return downcast<z8536_device &>(device).m_read_pb.set_callback(object); }
	template<class _Object> static devcb_base &set_pb_wr_callback(device_t &device, _Object object) { return downcast<z8536_device &>(device).m_write_pb.set_callback(object); }
	template<class _Object> static devcb_base &set_pc_rd_callback(device_t &device, _Object object) { return downcast<z8536_device &>(device).m_read_pc.set_callback(object); }
	template<class _Object> static devcb_base &set_pc_wr_callback(device_t &device, _Object object) { return downcast<z8536_device &>(device).m_write_pc.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	int intack_r();

	DECLARE_WRITE_LINE_MEMBER( pa0_w ) { external_port_w(PORT_A, 0, state); }
	DECLARE_WRITE_LINE_MEMBER( pa1_w ) { external_port_w(PORT_A, 1, state); }
	DECLARE_WRITE_LINE_MEMBER( pa2_w ) { external_port_w(PORT_A, 2, state); }
	DECLARE_WRITE_LINE_MEMBER( pa3_w ) { external_port_w(PORT_A, 3, state); }
	DECLARE_WRITE_LINE_MEMBER( pa4_w ) { external_port_w(PORT_A, 4, state); }
	DECLARE_WRITE_LINE_MEMBER( pa5_w ) { external_port_w(PORT_A, 5, state); }
	DECLARE_WRITE_LINE_MEMBER( pa6_w ) { external_port_w(PORT_A, 6, state); }
	DECLARE_WRITE_LINE_MEMBER( pa7_w ) { external_port_w(PORT_A, 7, state); }

	DECLARE_WRITE_LINE_MEMBER( pb0_w ) { external_port_w(PORT_B, 0, state); }
	DECLARE_WRITE_LINE_MEMBER( pb1_w ) { external_port_w(PORT_B, 1, state); }
	DECLARE_WRITE_LINE_MEMBER( pb2_w ) { external_port_w(PORT_B, 2, state); }
	DECLARE_WRITE_LINE_MEMBER( pb3_w ) { external_port_w(PORT_B, 3, state); }
	DECLARE_WRITE_LINE_MEMBER( pb4_w ) { external_port_w(PORT_B, 4, state); }
	DECLARE_WRITE_LINE_MEMBER( pb5_w ) { external_port_w(PORT_B, 5, state); }
	DECLARE_WRITE_LINE_MEMBER( pb6_w ) { external_port_w(PORT_B, 6, state); }
	DECLARE_WRITE_LINE_MEMBER( pb7_w ) { external_port_w(PORT_B, 7, state); }

	DECLARE_WRITE_LINE_MEMBER( pc0_w ) { external_port_w(PORT_C, 0, state); }
	DECLARE_WRITE_LINE_MEMBER( pc1_w ) { external_port_w(PORT_C, 1, state); }
	DECLARE_WRITE_LINE_MEMBER( pc2_w ) { external_port_w(PORT_C, 2, state); }
	DECLARE_WRITE_LINE_MEMBER( pc3_w ) { external_port_w(PORT_C, 3, state); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

private:
	enum
	{
		TIMER_1 = 0,
		TIMER_2,
		TIMER_3
	};

	// states
	enum
	{
		STATE_RESET = -1,
		STATE_0,
		STATE_1
	};


	// ports
	enum
	{
		PORT_C = 0,
		PORT_B,
		PORT_A,
		CONTROL
	};


	// registers
	enum
	{
		MASTER_INTERRUPT_CONTROL = 0,
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
		COUNTER_TIMER_1_CURRENT_COUNT_MS_BYTE,
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
		PORT_A_MODE_SPECIFICATION,
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

	UINT8 read_register(offs_t offset);
	UINT8 read_register(offs_t offset, UINT8 mask);
	void write_register(offs_t offset, UINT8 data);
	void write_register(offs_t offset, UINT8 data, UINT8 mask);

	bool counter_enabled(device_timer_id id);
	bool counter_external_output(device_timer_id id);
	bool counter_external_count(device_timer_id id);
	bool counter_external_trigger(device_timer_id id);
	bool counter_external_gate(device_timer_id id);
	bool counter_gated(device_timer_id id);
	void count(device_timer_id id);
	void trigger(device_timer_id id);
	void gate(device_timer_id id, int state);
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
	int m_state;
	UINT8 m_register[48];
	UINT8 m_pointer;

	// input/output port state
	UINT8 m_input[3];
	UINT8 m_output[3];
	UINT8 m_buffer[3];
	UINT8 m_match[3];

	// timers
	emu_timer *m_timer;
	UINT16 m_counter[3];
};


// device type definition
extern const device_type Z8536;



#endif
