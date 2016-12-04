// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*****************************************************************************

    5/74164 8-bit parallel-out serial shift registers

***********************************************************************

    Connection Diagram:
              ___ ___
        A  1 |*  u   | 14  Vcc
        B  2 |       | 13  QH
       QA  3 |       | 12  QG
       QB  4 |       | 11  QF
       QC  5 |       | 10  QE
       QD  6 |       |  9  *Clear
      GND  7 |_______|  8  Clock

***********************************************************************
    Function Table:
    +-------------------------+----------------+
    |       Inputs            |  Qutputs*      |
    +-------+-------+---------+----------------+
    | Clear | Clock |  A   B  | QA  QB ... QH  | 
    +-------+-------+---------+----------------+
    |   L   |   X   |  X   X  |  L   L      L  |
    |   H   |   L   |  X   X  | QA0 QB0    QH0 |
    |   H   |   ^   |  H   H  |  H  QAn    QGn |
    |   H   |   ^   |  L   X  |  L  QAn    QGn |
    |   H   |   ^   |  X   L  |  L  QAn    QGn |
    +-------+-------+---------+----------------+

    H = High Level (steady state)
    L = Low Level (steady state)
    X = Don't Care
    ^ = Transition from low to high level
    QA0, QB0 ... QH0 = The level of QA, QB ... QH before the indicated steady-state input conditions were established.
    QAn, QGn = The level of QA or QG before the most recent ^ transition of the clock; indicates a 1 bit shift.

**********************************************************************/

#pragma once

#ifndef TTL74164_H
#define TTL74164_H

#include "emu.h"

#define MCFG_74164_QA_CB(_devcb) \
	devcb = &ttl74164_device::set_qa_cb(*device, DEVCB_##_devcb);

#define MCFG_74164_QB_CB(_devcb) \
	devcb = &ttl74164_device::set_qb_cb(*device, DEVCB_##_devcb);

#define MCFG_74164_QC_CB(_devcb) \
	devcb = &ttl74164_device::set_qc_cb(*device, DEVCB_##_devcb);

#define MCFG_74164_QD_CB(_devcb) \
	devcb = &ttl74164_device::set_qd_cb(*device, DEVCB_##_devcb);

#define MCFG_74164_QE_CB(_devcb) \
	devcb = &ttl74164_device::set_qe_cb(*device, DEVCB_##_devcb);

#define MCFG_74164_QF_CB(_devcb) \
	devcb = &ttl74164_device::set_qf_cb(*device, DEVCB_##_devcb);

#define MCFG_74164_QG_CB(_devcb) \
	devcb = &ttl74164_device::set_qg_cb(*device, DEVCB_##_devcb);

#define MCFG_74164_QH_CB(_devcb) \
	devcb = &ttl74164_device::set_qh_cb(*device, DEVCB_##_devcb);

#define MCFG_74164_OUTPUT_CB(_devcb)									\
	devcb = &ttl74164_device::set_output_cb(*device, DEVCB_##_devcb);

#define MCFG_74164_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TTL74164, 0)


class ttl74164_device : public device_t
{
public:
	// construction/destruction
	ttl74164_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_qa_cb(device_t &device, _Object object) { return downcast<ttl74164_device &>(device).m_qa_func.set_callback(object); }
	template<class _Object> static devcb_base &set_qb_cb(device_t &device, _Object object) { return downcast<ttl74164_device &>(device).m_qb_func.set_callback(object); }
	template<class _Object> static devcb_base &set_qc_cb(device_t &device, _Object object) { return downcast<ttl74164_device &>(device).m_qc_func.set_callback(object); }
	template<class _Object> static devcb_base &set_qd_cb(device_t &device, _Object object) { return downcast<ttl74164_device &>(device).m_qd_func.set_callback(object); }
	template<class _Object> static devcb_base &set_qe_cb(device_t &device, _Object object) { return downcast<ttl74164_device &>(device).m_qe_func.set_callback(object); }
	template<class _Object> static devcb_base &set_qf_cb(device_t &device, _Object object) { return downcast<ttl74164_device &>(device).m_qf_func.set_callback(object); }
	template<class _Object> static devcb_base &set_qg_cb(device_t &device, _Object object) { return downcast<ttl74164_device &>(device).m_qg_func.set_callback(object); }
	template<class _Object> static devcb_base &set_qh_cb(device_t &device, _Object object) { return downcast<ttl74164_device &>(device).m_qh_func.set_callback(object); }
	template<class _Object> static devcb_base &set_output_cb(device_t &device, _Object object) { return downcast<ttl74164_device &>(device).m_output_func.set_callback(object); }

	// public interfaces
	DECLARE_WRITE_LINE_MEMBER( clear_w );
	DECLARE_WRITE_LINE_MEMBER( clock_w );
	DECLARE_WRITE_LINE_MEMBER( a_w );
	DECLARE_WRITE_LINE_MEMBER( b_w );

	DECLARE_READ_LINE_MEMBER( output_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	// callbacks
	devcb_write_line m_qa_func;
	devcb_write_line m_qb_func;
	devcb_write_line m_qc_func;
	devcb_write_line m_qd_func;
	devcb_write_line m_qe_func;
	devcb_write_line m_qf_func;
	devcb_write_line m_qg_func;
	devcb_write_line m_qh_func;
	devcb_write8 m_output_func;

	// inputs
	uint8_t m_clear;    // pin 9
	uint8_t m_clk;      // pin 8
	uint8_t m_a;        // pin 1
	uint8_t m_b;        // pin 2

	// outputs
	uint8_t m_out;      // pin 3-6 + pin 10-13 from LSB to MSB
};

// device type definition
extern const device_type TTL74164;

#endif /* TTL74164_H */
