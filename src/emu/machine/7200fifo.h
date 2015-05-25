// license:BSD-3-Clause
// copyright-holders:hap
/**********************************************************************

    IDT7200 series 9-bit Asynchronous FIFO Emulation

**********************************************************************
                            _____   _____
                    _W   1 |*    \_/     | 28  Vcc
                    D8   2 |             | 27  D4
                    D3   3 |             | 26  D5
                    D2   4 |             | 25  D6
                    D1   5 |             | 24  D7
                    D0   6 |             | 23  _FL/_RT
                   _XI   7 |     7200    | 22  _MR
                   _FF   8 |             | 21  _EF
                    Q0   9 |             | 20  _XO/_HF
                    Q1  10 |             | 19  Q7
                    Q2  11 |             | 18  Q6
                    Q3  12 |             | 17  Q5
                    Q8  13 |             | 16  Q4
                   GND  14 |_____________| 15  _R


Known chips and buffer sizes are listed below. Note that in width or depth
expansion mode (using more than one chip and XO/XI), it may be increased more.

    256x9     512x9     1Kx9      2Kx9      4Kx9      8Kx9      16Kx9     32Kx9     64Kx9
  -------------------------------------------------------------------------------------------
    IDT7200   IDT7201   IDT7202   IDT7203   IDT7204   IDT7205   IDT7206   IDT7207   IDT7208

The following chips are functionally equivalent and pin-compatible.

    AM7200    AM7201    AM7202    AM7203    AM7204
    MS7200    MS7201    MS7202    MS7203    MS7204

    LH5495    LH5496    LH5497    LH5498    LH5499
              LH540201  LH540202  LH540203  LH540204  LH540205  LH540206

    CY7C419   CY7C420   CY7C424   CY7C428   CY7C432
              CY7C421   CY7C425   CY7C429   CY7C433

32-pin PLCC/LCC or TQFP configurations are also available.


**********************************************************************/

#pragma once

#ifndef _7200FIFO_H
#define _7200FIFO_H

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_FIFO7200_ADD(_tag, _ramsize) \
	MCFG_DEVICE_ADD(_tag, FIFO7200, 0) \
	fifo7200_device::set_ram_size(*device, _ramsize);

#define MCFG_FIFO7200_EF_HANDLER(_devcb) \
	devcb = &fifo7200_device::set_ef_handler(*device, DEVCB_##_devcb);

#define MCFG_FIFO7200_FF_HANDLER(_devcb) \
	devcb = &fifo7200_device::set_ff_handler(*device, DEVCB_##_devcb);

#define MCFG_FIFO7200_HF_HANDLER(_devcb) \
	devcb = &fifo7200_device::set_hf_handler(*device, DEVCB_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fifo7200_device

class fifo7200_device : public device_t
{
public:
	fifo7200_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_ef_handler(device_t &device, _Object object) { return downcast<fifo7200_device &>(device).m_ef_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_ff_handler(device_t &device, _Object object) { return downcast<fifo7200_device &>(device).m_ff_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_hf_handler(device_t &device, _Object object) { return downcast<fifo7200_device &>(device).m_hf_handler.set_callback(object); }
	static void set_ram_size(device_t &device, int size) { downcast<fifo7200_device &>(device).m_ram_size = size; }

	DECLARE_READ_LINE_MEMBER( ef_r ) { return !m_ef; } // _EF
	DECLARE_READ_LINE_MEMBER( ff_r ) { return !m_ff; } // _FF
	DECLARE_READ_LINE_MEMBER( hf_r ) { return !m_hf; } // _HF

	// normal configuration
	DECLARE_WRITE16_MEMBER( data_word_w ) { fifo_write(data); }
	DECLARE_READ16_MEMBER( data_word_r ) { return (UINT16)fifo_read(); }

	// use these for simple configurations that don't have d8/q8 connected
	DECLARE_WRITE8_MEMBER( data_byte_w ) { fifo_write(data); }
	DECLARE_READ8_MEMBER( data_byte_r ) { return (UINT8)fifo_read(); }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	void fifo_write(UINT16 data);
	UINT16 fifo_read();

	std::vector<UINT16> m_buffer;
	int m_ram_size;

	int m_read_ptr;
	int m_write_ptr;

	int m_ef; // empty flag
	int m_ff; // full flag
	int m_hf; // half-full flag

	devcb_write_line m_ef_handler;
	devcb_write_line m_ff_handler;
	devcb_write_line m_hf_handler;
};

// device type definition
extern const device_type FIFO7200;


#endif /* _7200FIFO_H */
