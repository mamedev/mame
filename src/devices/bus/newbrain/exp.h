// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Grundy NewBrain Expansion Port emulation

**********************************************************************

                    GND       1      26      A6
                   1/8C       2      27      _RAMENB
                    A14       3      28      EXRM2
                    A13       4      29      EXRM1
                     D5       5      30      EXRM0
                   RMSL       6      31      _ROMOV
                     D4       7      32      _BUSRQ
                     D3       8      33      _M1
                     D6       9      34      _RST
                     D7      10      35      _RFRSH
                    A11      11      36      _WAIT
                    A10      12      37      A4
                     A8      13      38      _BUSAK
                     A9      14      39      A15
                    A12      15      40      _WR
                     A7      16      41      _INT
                     A3      17      42      _RD
                     A2      18      43      _NMI
                     A1      19      44      _HALT
                     A0      20      45      _MREQ
                     D0      21      46      _IORQ
                     D1      22      47      PRTOV
                  _FCTR      23      48      _RAMINH
                     D2      24      49      +5V
                     A5      25      50      .

**********************************************************************/

#ifndef MAME_BUS_NEWBRAIN_EXP_H
#define MAME_BUS_NEWBRAIN_EXP_H

#pragma once




//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> newbrain_expansion_slot_device

class device_newbrain_expansion_slot_interface;

class newbrain_expansion_slot_device : public device_t,
									public device_single_card_slot_interface<device_newbrain_expansion_slot_interface>
{
public:
	// construction/destruction
	template <typename T>
	newbrain_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: newbrain_expansion_slot_device(mconfig, tag, owner, clock)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}

	newbrain_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_callback() { return m_write_int.bind(); }
	auto m1_wait_callback() { return m_write_m1_wait.bind(); }
	auto busrq_callback() { return m_write_busrq.bind(); }
	auto host_mreq_r_callback() { return m_read_host.bind(); }
	auto host_mreq_w_callback() { return m_write_host.bind(); }
	auto host_main_r_callback() { return m_read_main.bind(); }
	auto host_main_w_callback() { return m_write_main.bind(); }
	auto host_paged_r_callback() { return m_read_paged.bind(); }
	auto host_paged_w_callback() { return m_write_paged.bind(); }

	// computer interface
	uint8_t mreq_r(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh);
	void mreq_w(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh);

	uint8_t iorq_r(offs_t offset, uint8_t data, bool &prtov);
	void iorq_w(offs_t offset, uint8_t data, bool &prtov);
	uint8_t page_r(int page, offs_t offset, uint8_t data);
	void page_w(int page, offs_t offset, uint8_t data);
	void busak_w(int state);

	// card interface
	void int_w(int state) { m_write_int(state); }
	void m1_wait_w(int state) { m_write_m1_wait(state); }
	void busrq_w(int state) { m_write_busrq(state); }
	uint8_t host_mreq_r(offs_t offset) { return m_read_host(offset); }
	void host_mreq_w(offs_t offset, uint8_t data) { m_write_host(offset, data); }
	uint8_t host_main_r(offs_t offset) { return m_read_main(offset); }
	void host_main_w(offs_t offset, uint8_t data) { m_write_main(offset, data); }
	uint8_t host_paged_r(offs_t offset) { return m_read_paged.isunset() ? m_read_host(offset & 0xffff) : m_read_paged(offset); }
	void host_paged_w(offs_t offset, uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	device_newbrain_expansion_slot_interface *m_card;

	devcb_write_line m_write_int;
	devcb_write_line m_write_m1_wait;
	devcb_write_line m_write_busrq;
	devcb_read8 m_read_host;
	devcb_write8 m_write_host;
	devcb_read8 m_read_main;
	devcb_write8 m_write_main;
	devcb_read8 m_read_paged;
	devcb_write8 m_write_paged;
};


// ======================> device_newbrain_expansion_slot_interface

// class representing interface-specific live newbrain_expansion card
class device_newbrain_expansion_slot_interface : public device_interface
{
public:
	// memory access
	virtual uint8_t mreq_r(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh) { return data; }
	virtual void mreq_w(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh) { }

	// I/O access
	virtual uint8_t iorq_r(offs_t offset, uint8_t data, bool &prtov) { return data; }
	virtual void iorq_w(offs_t offset, uint8_t data, bool &prtov) { }

	// paged memory access from the expansion interface module
	virtual uint8_t page_r(int page, offs_t offset, uint8_t data) { return data; }
	virtual void page_w(int page, offs_t offset, uint8_t data) { }

	// bus acknowledge from the computer
	virtual void busak_w(int state) { }

protected:
	// construction/destruction
	device_newbrain_expansion_slot_interface(const machine_config &mconfig, device_t &device);

	newbrain_expansion_slot_device *m_slot;
};


// device type declaration
DECLARE_DEVICE_TYPE(NEWBRAIN_EXPANSION_SLOT, newbrain_expansion_slot_device)


void newbrain_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_NEWBRAIN_EXP_H
