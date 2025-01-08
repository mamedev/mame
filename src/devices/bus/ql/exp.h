// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair QL expansion port emulation

**********************************************************************

                              A     B
                        GND   *  1  *   GND
                         D3   *  2  *   D2
                         D4   *  3  *   D1
                         D5   *  4  *   D0
                         D6   *  5  *   ASL
                         D7   *  6  *   DSL
                        A19   *  7  *   RDWL
                        A18   *  8  *   DTACKL
                        A17   *  9  *   BGL
                        A16   * 10  *   BRL
                     CLKCPU   * 11  *   A15
                        RED   * 12  *   RESETCPUL
                        A14   * 13  *   CSYNCL
                        A13   * 14  *   E
                        A12   * 15  *   VSYNCH
                        A11   * 16  *   VPAL
                        A10   * 17  *   GREEN
                         A9   * 18  *   BLUE
                         A8   * 19  *   FC2
                         A7   * 20  *   FC1
                         A6   * 21  *   FC0
                         A5   * 22  *   A0
                         A4   * 23  *   ROMOEH
                         A3   * 24  *   A1
                       DBGL   * 25  *   A2
                        SP2   * 26  *   SP3
                      DSCML   * 27  *   IPL0L
                        SP1   * 28  *   BERRL
                        SP0   * 29  *   IPL1L
                       VP12   * 30  *   EXTINTL
                       VM12   * 31  *   VIN
                        VIN   * 32  *   VIN

**********************************************************************/

#ifndef MAME_BUS_QL_EXP_H
#define MAME_BUS_QL_EXP_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_ql_expansion_card_interface

class ql_expansion_slot_device;

class device_ql_expansion_card_interface : public device_interface
{
	friend class ql_expansion_slot_device;

public:
	virtual void romoeh_w(int state) { m_romoeh = state; }
	virtual uint8_t read(offs_t offset, uint8_t data) { return data; }
	virtual void write(offs_t offset, uint8_t data) { }

protected:
	// construction/destruction
	device_ql_expansion_card_interface(const machine_config &mconfig, device_t &device);

	void interface_post_start() override;

	ql_expansion_slot_device *const m_slot;

	int m_romoeh;
};


// ======================> ql_expansion_slot_device

class ql_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_ql_expansion_card_interface>
{
public:
	// construction/destruction
	template <typename T>
	ql_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: ql_expansion_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	ql_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto ipl0l_wr_callback() { return m_write_ipl0l.bind(); }
	auto ipl1l_wr_callback() { return m_write_ipl1l.bind(); }
	auto berrl_wr_callback() { return m_write_berrl.bind(); }
	auto extintl_wr_callback() { return m_write_extintl.bind(); }

	// computer interface
	uint8_t read(offs_t offset, uint8_t data) { if (m_card) data = m_card->read(offset, data); return data; }
	void write(offs_t offset, uint8_t data) { if (m_card) m_card->write(offset, data); }
	void romoeh_w(int state) { if (m_card) m_card->romoeh_w(state); }

	// card interface
	void ipl0l_w(int state) { m_write_ipl0l(state); }
	void ipl1l_w(int state) { m_write_ipl1l(state); }
	void berrl_w(int state) { m_write_berrl(state); }
	void extintl_w(int state) { m_write_extintl(state); }

protected:
	// device-level overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	devcb_write_line   m_write_ipl0l;
	devcb_write_line   m_write_ipl1l;
	devcb_write_line   m_write_berrl;
	devcb_write_line   m_write_extintl;

	device_ql_expansion_card_interface *m_card;
};


// device type definition
DECLARE_DEVICE_TYPE(QL_EXPANSION_SLOT, ql_expansion_slot_device)


void ql_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_QL_EXP_H
