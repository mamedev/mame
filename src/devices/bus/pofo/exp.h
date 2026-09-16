// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio Expansion Port emulation

**********************************************************************

                   ABUF       1      2       5VS
                   REDY       3      4       VCC
                   BCOM       5      6       NCC1
                   NMD1       7      8       WAKE
                    DTR       9      10      DEN
                   PDET      11      12      IINT
                   CCLK      13      14      MRST
                   HLDA      15      16      HLDO
                   IACK      17      18      CDET
                    IOM      19      20      A19
                    A18      21      22      A17
                    A16      23      24      A15
                    A14      25      26      A13
                    A12      27      28      A11
                    A10      29      30      A9
                     A8      31      32      VRAM
                   HLDI      33      34      ALE
                    GND      35      36      NMIO
                    OA7      37      38      OA6
                    OA5      39      40      OA4
                    OA3      41      42      OA2
                    OA1      43      44      OA0
                    AD0      45      46      AD1
                    AD2      47      48      AD3
                    AD4      49      50      AD5
                    AD6      51      52      AD7
                   EINT      53      54      NRDI
                   VEXT      55      56      EACK
                   BATD      57      58      NWRI
                    5VS      59      60      BBUF

**********************************************************************/

#ifndef MAME_BUS_POFO_EXP_H
#define MAME_BUS_POFO_EXP_H

#pragma once




//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_portfolio_expansion_slot_interface

class portfolio_expansion_slot_device;

class device_portfolio_expansion_slot_interface : public device_interface
{
public:
	virtual bool nmd1() { return 1; }
	virtual bool pdet() { return 0; }
	virtual bool cdet() { return 1; }

	virtual uint8_t iack_r() { return 0xff; }
	virtual uint8_t eack_r() { return 0xff; }

	virtual void ncc1_w(int state) { }
	virtual void iint_w(int state) { }
	virtual void nmio_w(int state) { }

	void eint_w(int state);
	void wake_w(int state);

protected:
	// construction/destruction
	device_portfolio_expansion_slot_interface(const machine_config &mconfig, device_t &device);

	portfolio_expansion_slot_device *m_slot;
};


// ======================> portfolio_expansion_slot_device

class portfolio_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_portfolio_expansion_slot_interface>
{
public:
	// construction/destruction
	template <typename T>
	portfolio_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: portfolio_expansion_slot_device(mconfig, tag, owner, clock)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}

	portfolio_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_memspace(T &&tag, int spacenum) { m_memspace.set_tag(std::forward<T>(tag), spacenum); }
	template <typename T> void set_iospace(T &&tag, int spacenum) { m_iospace.set_tag(std::forward<T>(tag), spacenum); }

	void set_memspace(address_space &space) { m_memspace_ptr = &space; }
	void set_iospace(address_space &space) { m_iospace_ptr = &space; }

	auto eint_wr_cb() { return m_write_eint.bind(); }
	auto wake_wr_cb() { return m_write_wake.bind(); }

	// computer interface
	int nmd1_r() { return (m_card != nullptr) ? m_card->nmd1() : 1; }
	int pdet_r() { return (m_card != nullptr) ? m_card->pdet() : 0; }
	int cdet_r() { return (m_card != nullptr) ? m_card->cdet() : 1; }

	uint8_t iack_r() { return (m_card != nullptr) ? m_card->iack_r() : 0xff; }
	uint8_t eack_r() { return (m_card != nullptr) ? m_card->eack_r() : 0xff; }

	void ncc1_w(int state) { if (m_card != nullptr) m_card->ncc1_w(state); }

	void iint_w(int state) { if (m_card != nullptr) m_card->iint_w(state); }
	void nmio_w(int state) { if (m_card != nullptr) m_card->nmio_w(state); }

	// peripheral interface
	void eint_w(int state) { m_write_eint(state); }
	void wake_w(int state) { m_write_wake(state); }

	// card interface
	address_space &memspace() { return m_memspace_ptr ? *m_memspace_ptr : *m_memspace; }
	address_space &iospace() { return m_iospace_ptr ? *m_iospace_ptr : *m_iospace; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	optional_address_space m_memspace;
	optional_address_space m_iospace;
	address_space *m_memspace_ptr = nullptr;
	address_space *m_iospace_ptr = nullptr;

	devcb_write_line   m_write_eint;
	devcb_write_line   m_write_wake;

	device_portfolio_expansion_slot_interface *m_card;
};


// device type definition
DECLARE_DEVICE_TYPE(PORTFOLIO_EXPANSION_SLOT, portfolio_expansion_slot_device)


void portfolio_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_POFO_EXP_H
