// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Signetics SCN2651 Programmable Communications Interface (PCI)
    Signetics SCN2661A/B/C Enhanced Programmable Communications Interface
    Signetics SCN2641 Asynchronous Communications Interface

****************************************************************************
                            _____   _____
                    D2   1 |*    \_/     | 28  D1
                    D3   2 |             | 27  D0
                   RxD   3 |             | 26  Vcc
                   GND   4 |             | 25  _RxC/BKDET
                    D4   5 |             | 24  _DTR
                    D5   6 |             | 23  _RTS
                    D6   7 |   SCN2651   | 22  _DSR
                    D7   8 |   SCN2661   | 21  RESET
                  _TxC   9 |   SCN68661  | 20  BRCLK
                    A1  10 |             | 19  TxD
                   _CE  11 |             | 18  _TxEMT/DSCHG
                    A0  12 |             | 17  _CTS
                  _R/W  13 |             | 16  _DCD
                _RxRDY  14 |_____________| 15  _TxRDY

                            _____   _____
                    D3   1 |*    \_/     | 24  D2
                   RxD   2 |             | 23  D1
                   GND   3 |             | 22  D0
                    D4   4 |             | 21  Vcc
                    D5   5 |             | 20  _RxC/BKDET
                    D6   6 |             | 19  _RTS
                    D7   7 |   SCN2641   | 18  RESET
                  _TxC   8 |             | 17  BRCLK
                    A1   9 |             | 16  TxD
                   _CE  10 |             | 15  _CTS
                    A0  11 |             | 14  _DCD
                  _R/W  12 |_____________| 13  _INTR

***************************************************************************/

#ifndef MAME_MACHINE_SCN_PCI_H
#define MAME_MACHINE_SCN_PCI_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> scn_pci_device

class scn_pci_device : public device_t
{
public:
	// callback configuration
	auto dtr_handler() { return m_dtr_callback.bind(); }
	auto rts_handler() { return m_rts_callback.bind(); }
	auto txemt_dschg_handler() { return m_txemt_dschg_callback.bind(); }
	auto txc_handler() { return m_txc_callback.bind(); }
	auto rxc_handler() { return m_rxc_callback.bind(); }
	auto txd_handler() { return m_txd_callback.bind(); }
	auto txrdy_handler() { return m_txrdy_callback.bind(); }
	auto rxrdy_handler() { return m_rxrdy_callback.bind(); }

	// microprocessor interface
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// line write handlers
	void rxd_w(int state);
	void dsr_w(int state);
	void dcd_w(int state);
	void cts_w(int state);
	void txc_w(int state);
	void rxc_w(int state);

	// output polling
	int txrdy_r() { assert(!m_is_aci); return BIT(m_status, 0) ? 0 : 1; }
	int rxrdy_r() { assert(!m_is_aci); return BIT(m_status, 1) ? 0 : 1; }
	int txemt_dschg_r() { assert(!m_is_aci); return BIT(m_status, 2) != 0 ? 0 : 1; }

protected:
	enum class rcvr_state : u8 {
		DISABLED,
		ASYNC_WAIT,
		ASYNC_START,
		HUNT_MODE,
		HUNT_SYN2,
		SYNCED,
		BREAK_DETECT
	};

	enum class xmtr_state : u8 {
		MARKING,
		DATA_BITS,
		STOP_BITS,
		SYN1,
		SYN2
	};

	// construction/destruction
	scn_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, const u16 *br_div, bool is_enhanced, bool is_aci);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// receiver helpers
	virtual void set_rxrdy(bool state);
	void rx_load_async(u8 data, bool pe, bool fe);
	void rx_load_sync(u8 data, bool pe);
	u8 read_rhr();
	void rcvr_sync();
	void rcvr_update(bool internal, bool output_1x);

	// transmitter helpers
	void set_txd(bool state);
	virtual void set_txrdy(bool state);
	virtual void set_txemt(bool state);
	void write_thr(u8 data);
	void loopback_retransmit(u8 data);
	void tx_load(u8 data);
	void xmtr_update(bool internal, bool output_1x);

	// modem control helpers
	virtual void set_dschg(bool state);
	void set_dtr(bool state);
	void set_rts(bool state);

	// mode setting helpers
	void write_syn(u8 data);
	u8 read_mode();
	void write_mode(u8 data);

	// other helpers
	u8 read_status();
	u8 read_command();
	void write_command(u8 data);
	TIMER_CALLBACK_MEMBER(brg_tick);

	// static tables
	static const u16 s_br_divisors_1[16];
	static const u16 s_br_divisors_2[16];
	static const u16 s_br_divisors_3[16];
	static const char *const s_stop_bit_desc[4];

	// callback objects
	devcb_write_line m_dtr_callback;
	devcb_write_line m_rts_callback;
	devcb_write_line m_txemt_dschg_callback;
	devcb_write_line m_txc_callback;
	devcb_write_line m_rxc_callback;
	devcb_write_line m_txd_callback;
	devcb_write_line m_txrdy_callback;
	devcb_write_line m_rxrdy_callback;

	// constant parameters
	const u16 *const m_br_div;
	const bool m_is_enhanced;
	const bool m_is_aci;

	// baud rate timer
	emu_timer *m_brg_timer;

	// primary registers
	u8 m_rhr;
	u8 m_thr;
	u8 m_status;
	u8 m_syn[3];
	u8 m_mode[2];
	u8 m_command;

	// initialization pointers
	u8 m_mode_pointer;
	u8 m_syn_pointer;

	// input line states
	bool m_rxd;
	bool m_cts;
	bool m_txc_input;
	bool m_rxc_input;

	// receiver state
	u16 m_rsr;
	u8 m_rbits;
	bool m_rparity;
	rcvr_state m_rcvr_state;
	u8 m_rcvr_clocks;
	bool m_null_frame_received;
	bool m_pre_syndet;

	// transmitter state
	u8 m_tsr;
	u8 m_tbits;
	bool m_tparity;
	xmtr_state m_xmtr_state;
	u8 m_xmtr_clocks;
	bool m_txd;
	bool m_thr_loaded;
	bool m_txemt;

	// misc. status
	bool m_dschg;
	bool m_dtr;
	bool m_rts;
	bool m_syn1_parity;
	bool m_brg_output;
};

// ======================> scn2651_device

class scn2651_device : public scn_pci_device
{
public:
	// device type constructor
	scn2651_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> scn2661a_device

class scn2661a_device : public scn_pci_device
{
public:
	// device type constructor
	scn2661a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> scn2661b_device

class scn2661b_device : public scn_pci_device
{
public:
	// device type constructor
	scn2661b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> scn2661c_device

class scn2661c_device : public scn_pci_device
{
public:
	// device type constructor
	scn2661c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> scn2641_device

class scn2641_device : public scn_pci_device
{
public:
	// device type constructor
	scn2641_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto intr_handler() { return m_intr_callback.bind(); }

	// output polling
	int intr_r() { return (m_status & 0x07) != 0 ? 0 : 1; }

protected:
	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override;

	// scn_pci_device overrides
	virtual void set_rxrdy(bool state) override;
	virtual void set_txrdy(bool state) override;
	virtual void set_txemt(bool state) override;
	virtual void set_dschg(bool state) override;

private:
	static const u16 s_br_divisors[16];

	devcb_write_line m_intr_callback;
};

DECLARE_DEVICE_TYPE(SCN2651,  scn2651_device)
DECLARE_DEVICE_TYPE(SCN2661A, scn2661a_device)
DECLARE_DEVICE_TYPE(SCN2661B, scn2661b_device)
DECLARE_DEVICE_TYPE(SCN2661C, scn2661c_device)
DECLARE_DEVICE_TYPE(SCN2641,  scn2641_device)

#endif // MAME_MACHINE_SCN_PCI_H
