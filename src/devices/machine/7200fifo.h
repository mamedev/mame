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
              QS7201    QS7202    QS7203    QS7204

    LH5495    LH5496    LH5497    LH5498    LH5499
              LH540201  LH540202  LH540203  LH540204  LH540205  LH540206

    CY7C419   CY7C420   CY7C424   CY7C428   CY7C432
              CY7C421   CY7C425   CY7C429   CY7C433

32-pin PLCC/LCC or TQFP configurations are also available.


**********************************************************************/

#ifndef MAME_MACHINE_7200FIFO_H
#define MAME_MACHINE_7200FIFO_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fifo7200_device

class fifo7200_device : public device_t
{
public:
	auto ef_handler() { return m_ef_handler.bind(); }
	auto ff_handler() { return m_ff_handler.bind(); }
	auto hf_handler() { return m_hf_handler.bind(); }

	int ef_r() { return !m_ef; } // _EF
	int ff_r() { return !m_ff; } // _FF
	int hf_r() { return !m_hf; } // _HF

	// normal configuration
	void data_word_w(uint16_t data) { fifo_write(data); }
	uint16_t data_word_r() { return (uint16_t)fifo_read(); }

	// use these for simple configurations that don't have d8/q8 connected
	void data_byte_w(uint8_t data) { fifo_write(data); }
	uint8_t data_byte_r() { return (uint8_t)fifo_read(); }

protected:
	fifo7200_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, int size);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void fifo_write(uint16_t data);
	uint16_t fifo_read();
	int fifo_used();

	std::vector<uint16_t> m_buffer;
	const int m_ram_size;

	int m_read_ptr;
	int m_write_ptr;

	int m_ef; // empty flag
	int m_ff; // full flag
	int m_hf; // half-full flag

	devcb_write_line m_ef_handler;
	devcb_write_line m_ff_handler;
	devcb_write_line m_hf_handler;
};

// ======================> idt7200_device

class idt7200_device : public fifo7200_device
{
public:
	idt7200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// ======================> idt7201_device

class idt7201_device : public fifo7200_device
{
public:
	idt7201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// ======================> idt7202_device

class idt7202_device : public fifo7200_device
{
public:
	idt7202_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// device type definitions
DECLARE_DEVICE_TYPE(IDT7200, idt7200_device)
DECLARE_DEVICE_TYPE(IDT7201, idt7201_device)
DECLARE_DEVICE_TYPE(IDT7202, idt7202_device)

#endif // MAME_MACHINE_7200FIFO_H
