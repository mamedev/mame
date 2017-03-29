// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    (DM)9334 8-Bit Addressable Latch

******************************************************************************

    Connection Diagram:
              ___ ___
       A0  1 |*  u   | 16  Vcc
       A1  2 |       | 15  /C
       A2  3 |       | 14  /E
       Q0  4 |       | 13  D
       Q1  5 |       | 12  Q7
       Q2  6 |       | 11  Q6
       Q3  7 |       | 10  Q5
      GND  8 |_______| 9   Q4

***********************************************************************

    Function Tables:

    /E  /C  Mode
    L   H   Addressable Latch
    H   H   Memory
    L   L   Active High Eight
            Channel Demultiplexer
    H   L   Clear

     ___________________________________________________________________________________________
    |       Inputs              |              Present Output States              |             |
    |---------------------------|-------------------------------------------------|     Mode    |
    | /C  /E  | D | A0  A1  A2  | Q0    Q1    Q2    Q3    Q4    Q5    Q6    Q7    |             |
    |---------|---|-------------|-------------------------------------------------|-------------|
    |  L   H  | X |  X   X   X  |  L     L     L     L     L     L     L     L    |    Clear    |
    |---------|---|-------------|-------------------------------------------------|-------------|
    |  L   L  | L |  L   L   L  |  L     L     L     L     L     L     L     L    |             |
    |  L   L  | H |  L   L   L  |  H     L     L     L     L     L     L     L    |             |
    |  L   L  | L |  H   L   L  |  L     L     L     L     L     L     L     L    |             |
    |  L   L  | H |  H   L   L  |  L     H     L     L     L     L     L     L    |             |
    |  *   *  | * |      *      |                    *                            | Demultiplex |
    |  *   *  | * |      *      |                    *                            |             |
    |  *   *  | * |      *      |                    *                            |             |
    |  *   *  | * |      *      |                    *                            |             |
    |  L   L  | H |  H   H   H  |  L     L     L     L     L     L     L     H    |             |
    |---------|---|-------------|-------------------------------------------------|-------------|
    |  H   H  | X |  X   X   X  |  Qn-1                                           |    Memory   |
    |---------|---|-------------|-------------------------------------------------|-------------|
    |  H   L  | L |  L   L   L  |  L     Qn-1  Qn-1  Qn-1                         |             |
    |  H   L  | H |  L   L   L  |  H     Qn-1  Qn-1                               |             |
    |  H   L  | L |  H   L   L  |  Qn-1  L     Qn-1                               |             |
    |  H   L  | H |  H   L   L  |  Qn-1  H     Qn-1                               |             |
    |  *   *  | * |      *      |                    *                            |             |
    |  *   *  | * |      *      |                    *                            |             |
    |  *   *  | * |      *      |                    *                            |             |
    |  H   L  | L |  H   H   H  |  Qn-1                                Qn-1  L    |             |
    |  H   L  | H |  H   H   H  |  Qn-1                                Qn-1  H    |             |
    |---------|---|-------------|-------------------------------------------------|-------------|

    X = Don't Care Condition
    L = Low Voltage Level
    H = High Voltage Level
    Qn-1 = Previous Output State

**********************************************************************/

#pragma once

#ifndef DM9334_H
#define DM9334_H


#define MCFG_DM9334_OUTPUT_CB(_devcb) \
	devcb = &dm9334_device::set_out_cb(*device, DEVCB_##_devcb);

#define MCFG_DM9334_Q0_CB(_devcb) \
	devcb = &dm9334_device::set_q0_cb(*device, DEVCB_##_devcb);

#define MCFG_DM9334_Q1_CB(_devcb) \
	devcb = &dm9334_device::set_q1_cb(*device, DEVCB_##_devcb);

#define MCFG_DM9334_Q2_CB(_devcb) \
	devcb = &dm9334_device::set_q2_cb(*device, DEVCB_##_devcb);

#define MCFG_DM9334_Q3_CB(_devcb) \
	devcb = &dm9334_device::set_q3_cb(*device, DEVCB_##_devcb);

#define MCFG_DM9334_Q4_CB(_devcb) \
	devcb = &dm9334_device::set_q4_cb(*device, DEVCB_##_devcb);

#define MCFG_DM9334_Q5_CB(_devcb) \
	devcb = &dm9334_device::set_q5_cb(*device, DEVCB_##_devcb);

#define MCFG_DM9334_Q6_CB(_devcb) \
	devcb = &dm9334_device::set_q6_cb(*device, DEVCB_##_devcb);

#define MCFG_DM9334_Q7_CB(_devcb) \
	devcb = &dm9334_device::set_q7_cb(*device, DEVCB_##_devcb);

#define MCFG_DM9334_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DM9334, 0)

class dm9334_device : public device_t
{
public:
	// construction/destruction
	dm9334_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_out_cb(device_t &device, _Object object) { return downcast<dm9334_device &>(device).m_out_func.set_callback(object); }
	template<class _Object> static devcb_base &set_q0_cb(device_t &device, _Object object) { return downcast<dm9334_device &>(device).m_q0_func.set_callback(object); }
	template<class _Object> static devcb_base &set_q1_cb(device_t &device, _Object object) { return downcast<dm9334_device &>(device).m_q1_func.set_callback(object); }
	template<class _Object> static devcb_base &set_q2_cb(device_t &device, _Object object) { return downcast<dm9334_device &>(device).m_q2_func.set_callback(object); }
	template<class _Object> static devcb_base &set_q3_cb(device_t &device, _Object object) { return downcast<dm9334_device &>(device).m_q3_func.set_callback(object); }
	template<class _Object> static devcb_base &set_q4_cb(device_t &device, _Object object) { return downcast<dm9334_device &>(device).m_q4_func.set_callback(object); }
	template<class _Object> static devcb_base &set_q5_cb(device_t &device, _Object object) { return downcast<dm9334_device &>(device).m_q5_func.set_callback(object); }
	template<class _Object> static devcb_base &set_q6_cb(device_t &device, _Object object) { return downcast<dm9334_device &>(device).m_q6_func.set_callback(object); }
	template<class _Object> static devcb_base &set_q7_cb(device_t &device, _Object object) { return downcast<dm9334_device &>(device).m_q7_func.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( e_w );
	DECLARE_WRITE_LINE_MEMBER( c_w );
	DECLARE_WRITE_LINE_MEMBER( d_w );
	DECLARE_WRITE_LINE_MEMBER( a0_w );
	DECLARE_WRITE_LINE_MEMBER( a1_w );
	DECLARE_WRITE_LINE_MEMBER( a2_w );
	DECLARE_WRITE8_MEMBER( a_w );

	DECLARE_READ8_MEMBER( output_r );
	DECLARE_READ_LINE_MEMBER( q0_r );
	DECLARE_READ_LINE_MEMBER( q1_r );
	DECLARE_READ_LINE_MEMBER( q2_r );
	DECLARE_READ_LINE_MEMBER( q3_r );
	DECLARE_READ_LINE_MEMBER( q4_r );
	DECLARE_READ_LINE_MEMBER( q5_r );
	DECLARE_READ_LINE_MEMBER( q6_r );
	DECLARE_READ_LINE_MEMBER( q7_r );

	uint8_t get_output() const { return m_out; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum class mode_t
	{
		DEMUX = 0x00,
		LATCH = 0x01,
		CLEAR = 0x02,
		MEMORY = 0x03
	};

	void init();
	void tick();

	// callbacks
	devcb_write_line m_out_func;
	devcb_write_line m_q0_func;
	devcb_write_line m_q1_func;
	devcb_write_line m_q2_func;
	devcb_write_line m_q3_func;
	devcb_write_line m_q4_func;
	devcb_write_line m_q5_func;
	devcb_write_line m_q6_func;
	devcb_write_line m_q7_func;

	// inputs
	uint8_t m_e;        // pin 14
	uint8_t m_c;        // pin 15
	uint8_t m_d;        // pin 13
	uint8_t m_a;        // pins 1-3 from LSB to MSB

	// outputs
	uint8_t m_out;      // pins 4-7 and 9-12 from LSB to MSB
};

// device type definition
extern const device_type DM9334;


#endif /* DM9334_H */
