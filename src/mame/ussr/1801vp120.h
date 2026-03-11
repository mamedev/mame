// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    1801VP1-120 Gate Array emulation

**********************************************************************
                            _____   _____
                  _AD0   1 |*    \_/     | 42  +5V
                  _AD1   2 |             | 41  DS0
                  _AD2   3 |             | 30  DS1
                  _AD3   4 |             | 39  DS2
                  _AD4   5 |             | 38  DS3
                  _AD5   6 |             | 37  MSW
                  _AD6   7 |             | 36  HS
                  _AD7   8 |             | 35  DIR
                  _AD8   9 |             | 34  ST
                  /AD9  10 |             | 33  TR0
                 _AD10  11 | 1801VP1-120 | 32  RDY
                 _AD11  12 |             | 31  WPR
                 _AD12  13 |             | 30  REZ
                 _AD13  14 |             | 29  DI
                 _AD14  15 |             | 28  WRE
                 _AD15  16 |             | 27  D01
                  SYNC  17 |             | 26  D02
                   DIN  18 |             | 25  D03
                  DOUT  19 |             | 24  IND
                  INIT  20 |             | 23  RPLY
                   GND  21 |_____________| 22  CLC

**********************************************************************/

#ifndef MAME_USSR_1801VP120_H
#define MAME_USSR_1801VP120_H

#pragma once

#include "machine/pdp11.h"
#include "machine/z80daisy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> k1801vp120_device

class k1801vp120_pri_device : public device_t,
				  public device_z80daisy_interface
{
public:
	// construction/destruction
	k1801vp120_pri_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto virq_wr_callback() { return m_write_virq.bind(); }
	template <int ch> auto ack_wr_callback() { return m_write_ack[ch].bind(); }
	template <int ch> auto out_wr_callback() { return m_write_out[ch].bind(); }

	template <int ch> auto write_data(uint8_t data)
	{
		m_channel[ch].rbuf = data; m_channel[ch].rcsr |= CSR_DONE;
		raise_virq(m_write_virq, m_channel[ch].rcsr, CSR_IE, m_channel[ch].rxrdy);
	}
	template <int ch> auto write_ack(int state)
	{
		m_channel[ch].tcsr |= CSR_DONE;
		raise_virq(m_write_virq, m_channel[ch].tcsr, CSR_IE, m_channel[ch].txrdy);
	}

	template <int ch> uint16_t read(offs_t offset) { return channel_read(&m_channel[ch], offset & 3); }
	template <int ch> void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { channel_write(&m_channel[ch], offset & 3, data, mem_mask); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override {};

private:
	static constexpr uint16_t PPxCSR_RD = CSR_DONE | CSR_IE;
	static constexpr uint16_t PPxCSR_WR = CSR_IE;

	struct xchannel
	{
		int channel;
		uint8_t rcsr;
		uint8_t rbuf;
		uint8_t tcsr;
		line_state rxrdy;
		line_state txrdy;
	} m_channel[3];

	devcb_write_line m_write_virq;
	devcb_write_line::array<2> m_write_ack;
	devcb_write8::array<3> m_write_out;

	uint16_t channel_read(struct xchannel *ch, offs_t offset);
	void channel_write(struct xchannel *ch, offs_t offset, uint16_t data, uint16_t mem_mask);
};


class k1801vp120_sub_device : public device_t,
				  public device_z80daisy_interface
{
public:
	// construction/destruction
	k1801vp120_sub_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto virq_wr_callback() { return m_write_virq.bind(); }
	template <int ch> auto ack_wr_callback() { return m_write_ack[ch].bind(); }
	template <int ch> auto out_wr_callback() { return m_write_out[ch].bind(); }

	template <int ch> auto write_data(uint8_t data)
	{
		m_channel[ch].rbuf = data; m_rcsr |= (PPRCSR_0RDY << ch);
		raise_virq(m_write_virq, m_rcsr, (PPRCSR_0IE << ch), m_channel[ch].rxrdy);
	}
	template <int ch> auto write_ack(int state)
	{
		m_tcsr |= (PPTCSR_0DONE << ch);
		raise_virq(m_write_virq, m_tcsr, (PPTCSR_0IE << ch), m_channel[ch].txrdy);
	}

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override {};

private:
	static constexpr uint16_t PPTCSR_1DONE   = 0020;
	static constexpr uint16_t PPTCSR_0DONE   = 0010;
	static constexpr uint16_t PPTCSR_0UNMAP  = 0004; // not implemented
	static constexpr uint16_t PPTCSR_1IE     = 0002;
	static constexpr uint16_t PPTCSR_0IE     = 0001;
	static constexpr uint16_t PPTCSR_RD      = PPTCSR_1DONE | PPTCSR_0DONE | PPTCSR_1IE | PPTCSR_0IE;
	static constexpr uint16_t PPTCSR_WR      = PPTCSR_0UNMAP | PPTCSR_1IE | PPTCSR_0IE;

	static constexpr uint16_t PPRCSR_RIE     = 0100; // not implemented
	static constexpr uint16_t PPRCSR_2RDY    = 0040;
	static constexpr uint16_t PPRCSR_1RDY    = 0020;
	static constexpr uint16_t PPRCSR_0RDY    = 0010;
	static constexpr uint16_t PPRCSR_2IE     = 0004;
	static constexpr uint16_t PPRCSR_1IE     = 0002;
	static constexpr uint16_t PPRCSR_0IE     = 0001;
	static constexpr uint16_t PPRCSR_RD      = PPRCSR_RIE | PPRCSR_2RDY | PPRCSR_1RDY | PPRCSR_0RDY | PPRCSR_2IE | PPRCSR_1IE | PPRCSR_0IE;
	static constexpr uint16_t PPRCSR_WR      = PPRCSR_RIE | PPRCSR_2IE | PPRCSR_1IE | PPRCSR_0IE;

	struct xchannel
	{
		uint8_t rbuf;
		line_state rxrdy;
		line_state txrdy;
	} m_channel[3];

	devcb_write_line m_write_virq;
	devcb_write_line::array<3> m_write_ack;
	devcb_write8::array<2> m_write_out;

	line_state m_reset;

	uint16_t m_rcsr;
	uint16_t m_tcsr;
};


// device type definition
DECLARE_DEVICE_TYPE(K1801VP120_PRI, k1801vp120_pri_device)
DECLARE_DEVICE_TYPE(K1801VP120_SUB, k1801vp120_sub_device)

#endif // MAME_USSR_1801VP120_H
