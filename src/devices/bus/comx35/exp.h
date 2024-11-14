// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 Expansion Slot emulation

**********************************************************************

                    GND       1      A       GND
                     NC       2      B       DS
                    +5V       3      C       V+
                     D0       4      D       D1
                     D2       5      E       D3
                     D4       6      F       D5
                     D6       7      H       D7
                    _DP       8      J       Q
                 _CLEAR       9      K       _MRD
                    TPA      10      L       N0
                     N1      11      M       N2
                   _RAS      12      N       _INT
                  _WAIT      13      P       CLOCK
                    SC1      14      R       SC0
                   _EF4      15      S       _CASE
                    TPB      16      T       _A15
                   _MWR      17      U       A14
                    MA7      18      V       _A14
                    MA5      19      W       MA6
                    MA4      20      X       MA3
                    MA2      21      Y       _EXTROM
                    MA1      22      Z       MA0

**********************************************************************/

#ifndef MAME_BUS_COMX35_EXP_H
#define MAME_BUS_COMX35_EXP_H

#pragma once

#define COMX_EXPANSION_BUS_TAG      "comxexp"

// ======================> comx_expansion_slot_device

class device_comx_expansion_card_interface;

class comx_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_comx_expansion_card_interface>
{
public:
	// construction/destruction
	template <typename T>
	comx_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: comx_expansion_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	comx_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_callback() { return m_write_irq.bind(); }

	uint8_t mrd_r(offs_t offset, int *extrom);
	void mwr_w(offs_t offset, uint8_t data);

	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	int ef4_r();

	void ds_w(int state);
	void q_w(int state);

	void irq_w(int state) { m_write_irq(state); }

	void sc_w(offs_t offset, uint8_t data);
	void tpb_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	devcb_write_line   m_write_irq;

	device_comx_expansion_card_interface *m_card;
};


// ======================> device_comx_expansion_card_interface

// class representing interface-specific live comx_expansion card
class device_comx_expansion_card_interface : public device_interface
{
	friend class comx_expansion_slot_device;

protected:
	// construction/destruction
	device_comx_expansion_card_interface(const machine_config &mconfig, device_t &device);

	// signals
	virtual int comx_ef4_r() { return CLEAR_LINE; }
	virtual void comx_ds_w(int state) { m_ds = state; }
	virtual void comx_q_w(int state) { }
	virtual void comx_sc_w(int n, int sc) { }
	virtual void comx_tpb_w(int state) { }

	// memory access
	virtual uint8_t comx_mrd_r(offs_t offset, int *extrom) { return 0; }
	virtual void comx_mwr_w(offs_t offset, uint8_t data) { }

	// I/O access
	virtual uint8_t comx_io_r(offs_t offset) { return 0; }
	virtual void comx_io_w(offs_t offset, uint8_t data) { }

	comx_expansion_slot_device *m_slot;

	int m_ds;
};

DECLARE_DEVICE_TYPE(COMX_EXPANSION_SLOT, comx_expansion_slot_device)

void comx_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_COMX35_EXP_H
