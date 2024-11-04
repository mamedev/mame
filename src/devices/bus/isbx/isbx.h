// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel Multibus I/O Expansion Bus IEEE-P959 (iSBX) emulation

**********************************************************************

                   +12V       1      2       -12V
                    GND       3      4       +5V
                  RESET       5      6       MCLK
                    MA2       7      8       MPST/
                    MA1       9      10      reserved
                    MA0      11      12      MINTR1
                 /IOWRT      13      14      MINTR0
                  /IORD      15      16      MWAIT/
                    GND      17      18      +5V
                    MD7      19      20      MCS1/
                    MD6      21      22      MCS0/
                    MD5      23      24      reserved
                    MD4      25      26      TDMA
                    MD3      27      28      OPT1
                    MD2      29      30      OPT0
                    MD1      31      32      MDACK/
                    MD0      33      34      MDRQT
                    GND      35      36      +5V
                    MDE      37      38      MDF
                    MDC      39      40      MDD
                    MDA      41      42      MDB
                    MD8      43      44      MD9

**********************************************************************/

#ifndef MAME_BUS_ISBX_ISBX_SLOT_H
#define MAME_BUS_ISBX_ISBX_SLOT_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_isbx_card_interface

class isbx_slot_device;

class device_isbx_card_interface : public device_interface
{
public:
	virtual uint8_t mcs0_r(offs_t offset) { return 0xff; }
	virtual void mcs0_w(offs_t offset, uint8_t data) { }
	virtual uint8_t mcs1_r(offs_t offset) { return 0xff; }
	virtual void mcs1_w(offs_t offset, uint8_t data) { }
	virtual uint8_t mdack_r(offs_t offset) { return 0xff; }
	virtual void mdack_w(offs_t offset, uint8_t data) { }
	virtual int opt0_r() { return 1; }
	virtual void opt0_w(int state) { }
	virtual int opt1_r() { return 1; }
	virtual void opt1_w(int state) { }
	virtual void tdma_w(int state) { }
	virtual void mclk_w(int state) { }

protected:
	// construction/destruction
	device_isbx_card_interface(const machine_config &mconfig, device_t &device);

	isbx_slot_device *m_slot;
};


// ======================> isbx_slot_device

class isbx_slot_device : public device_t, public device_single_card_slot_interface<device_isbx_card_interface>
{
public:
	// construction/destruction
	template <typename T>
	isbx_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: isbx_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	isbx_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto mintr0() { return m_write_mintr0.bind(); }
	auto mintr1() { return m_write_mintr1.bind(); }
	auto mdrqt() { return m_write_mdrqt.bind(); }
	auto mwait() { return m_write_mwait.bind(); }

	// computer interface
	uint8_t mcs0_r(offs_t offset) { return m_card ? m_card->mcs0_r(offset) : 0xff; }
	void mcs0_w(offs_t offset, uint8_t data) { if (m_card) m_card->mcs0_w(offset, data); }
	uint8_t mcs1_r(offs_t offset) { return m_card ? m_card->mcs1_r(offset) : 0xff; }
	void mcs1_w(offs_t offset, uint8_t data) { if (m_card) m_card->mcs1_w(offset, data); }
	uint8_t mdack_r(offs_t offset) { return m_card ? m_card->mdack_r(offset) : 0xff; }
	void mdack_w(offs_t offset, uint8_t data) { if (m_card) m_card->mdack_w(offset, data); }
	int mpst_r() { return m_card == nullptr; }
	int opt0_r() { return m_card ? m_card->opt0_r() : 1; }
	void opt0_w(int state) { if (m_card) m_card->opt0_w(state); }
	int opt1_r() { return m_card ? m_card->opt1_r() : 1; }
	void opt1_w(int state) { if (m_card) m_card->opt1_w(state); }
	void tdma_w(int state) { if (m_card) m_card->tdma_w(state); }
	void mclk_w(int state) { if (m_card) m_card->mclk_w(state); }

	// card interface
	void mintr0_w(int state) { m_write_mintr0(state); }
	void mintr1_w(int state) { m_write_mintr1(state); }
	void mdrqt_w(int state) { m_write_mdrqt(state); }
	void mwait_w(int state) { m_write_mwait(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	devcb_write_line   m_write_mintr0;
	devcb_write_line   m_write_mintr1;
	devcb_write_line   m_write_mdrqt;
	devcb_write_line   m_write_mwait;

	device_isbx_card_interface *m_card;
};


// device type definition
DECLARE_DEVICE_TYPE(ISBX_SLOT, isbx_slot_device)


void isbx_cards(device_slot_interface &device);


#endif // MAME_BUS_ISBX_ISBX_SLOT_H
