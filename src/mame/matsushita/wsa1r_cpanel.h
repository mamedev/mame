// license:GPL2+
// copyright-holders:Felipe Sanches
/***************************************************************************

    Technics SX-WSA1R control panel HLE

    One Mitsubishi M37471M2196S on the CONTROL PANEL 1 board -- the same part
    as the two panel MCUs in kn5000_cpanel.cpp -- scans the button matrix and
    talks to CPU 1 over the TMP95C061's serial channel 1 in I/O-interface
    (synchronous) mode.

    Signals: P8.3 = TXD1 to panel SIN, P8.4 = RXD1 from panel SOUT, P8.5 =
    SCLK1 driven by whoever transmits, PB.4 = the panel's busy line, INT6 =
    its attention request.  Idle is P8.5 high and PB.4 low.  The pin names are
    the databook's; what is established here is which bits the CPU drives and
    reads.

    A frame is a length byte, an address byte and a body.  The address byte is
    read through four different masks rather than one field layout, so no
    single bit decode is asserted here.

***************************************************************************/

#ifndef MAME_MATSUSHITA_WSA1R_CPANEL_H
#define MAME_MATSUSHITA_WSA1R_CPANEL_H

#pragma once

class wsa1r_cpanel_device : public device_t
{
public:
	wsa1r_cpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// --- to CPU 1 ---
	auto atn()  { return m_atn_cb.bind(); }    // the panel's request line -> INT6
	auto busy() { return m_busy_cb.bind(); }   // PB bit 4: 1 while the panel holds the link
	auto sclk() { return m_sclk_cb.bind(); }   // P8 bit 5: 1 when the clock line is idle high
	auto rxd()  { return m_rxd_cb.bind(); }    // one byte -> SC1BUF, and raise INTRX1

	// --- from CPU 1 ---
	void tx_byte(u8 data);        // sc1buf_w: one byte clocked out to the panel
	void rx_enable(int state);    // SC1MOD bit 5 (RXE): the firmware is ready to receive

	// Firmware-authoritative LED state, for a layout or a debug view.
	u8 led_register(int n) const { return (n >= 0 && n < 8) ? m_led[n] : 0; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(scan_tick);     // periodic matrix + analogue scan
	TIMER_CALLBACK_MEMBER(deliver_byte);  // one queued reply byte
	TIMER_CALLBACK_MEMBER(request_tick);  // the attention line's pulse / retry

private:
	static constexpr int NUM_SEG = 11;    // SEG0..SEG10, the CP1 MCU's scan lines
	static constexpr int RESP_MAX = 64;

	void queue_frame(const u8 *bytes, int n);
	void start_request();
	void frame_complete();
	bool led_frame(u8 addr, u8 data);
	bool segment_is_wired(int seg) const;
	static s32 dial_delta(optional_ioport &port, s32 &prev, bool &synced, s32 modulus);

	// Inbound frame assembly (CPU -> panel).  m_len is set from the first byte by the
	// firmware's own rule, so a run frame is consumed whole.
	u8  m_frame[19];              // max = (0x0F) + 3 = 18, plus one for safety
	int m_pos, m_len;

	// Outbound queue (panel -> CPU).
	u8  m_resp[RESP_MAX];
	int m_resp_len, m_resp_pos;
	bool m_rx_enabled;

	emu_timer *m_scan_timer;
	emu_timer *m_byte_timer;
	emu_timer *m_req_timer;
	bool m_requesting;            // a message is queued and INT6 has not been taken yet
	attotime m_last_tx;           // when CPU 1 last put a byte on the wire (see request_tick)

	// Scan shadows, so only CHANGED segments are reported -- which is what the firmware's
	// own change-mask table at RAM 0x2B20 expects to be fed.
	u8   m_seg_prev[NUM_SEG];
	u8   m_vol_prev;
	bool m_vol_synced;
	s32  m_dial_prev;
	bool m_dial_synced;
	s32  m_dial_drag_prev;
	bool m_dial_drag_synced;

	devcb_write_line m_atn_cb;
	devcb_write_line m_busy_cb;
	devcb_write_line m_sclk_cb;
	devcb_write8     m_rxd_cb;

	optional_ioport_array<NUM_SEG> m_seg;
	optional_ioport m_volume;   // wire 0xD3
	optional_ioport m_dial;      // wire 0xD7, the DATA ENTRY DIAL (keys / mouse axis)
	optional_ioport m_dial_drag; // the SAME wheel, dragged in a circle by the layout

	// 8 LED registers x 8 bits, output led%u with %u = register * 8 + bit.  The
	// name stays positional because the wire position is what the ROM
	// establishes for both variants; which lamp sits there is per-variant and
	// belongs in the layout.
	//
	// For the rack, prom_a's variant-2 switch->LED table at Panel_SwitchLedTable_V2 holds one
	// u16 per switch, and that u16 is (register << 8) | bit mask: Panel_SetSwitchLed
	// reads it with `ld WA,(XHL)` and calls Panel_SetLedRegister_Unguarded, which maps W
	// through the register->wire table and queues [wire][A].  A button with an indicator
	// lights its own, so
	//
	//   reg1 bits 0..3  PLAY MODE SOUND / COMBI, EDIT MODE SOUND / COMBI
	//   reg0 bits 0..3  BANK USER 1 / USER 2 / ROM-EXT / RE-MAP
	//   reg4 bits 0,1   MENU PART / MENU SYSTEM
	//   reg5 bits 0,1   MENU MIDI / MENU DISK
	//   reg6 bit 2      COMPARE           -- the LCD-key family indicator
	//   reg6 bit 3      MIDI/NUMBER PAD   -- the numeric family indicator
	//
	// and {reg2 bit0, reg2 bit1, reg3 bit0, reg3 bit1} are the four REALTIME
	// CREATOR ring lamps as a set, in an order nothing read so far pins down.
	//
	// Only 47 of the 64 positions are real lamps.  The PANEL SW&LED CHECK
	// screen's all-on sweep (sub_F956B0) walks the table at Panel_LedCheckMaskTable, which is
	//
	//     reg0=FF reg1=FF reg2=FF reg3=FF reg4=FF reg5=03 reg6=0F reg7=02
	//
	// so led42-47, led52-56 and led58-63 are never lit by the firmware's own
	// all-lamps test and no layout lamp may bind to them.  led%u is the wire
	// position, so the numbering runs to 63.
	//
	// The lamps have two writers.  Panel_SetLedRegister opens with
	// `cp (0x207A),0xDB / jr Z`, so it refuses every LED write while the PANEL SW&LED
	// CHECK screen (id 0xDB) is up.  The service module uses
	// Panel_SetLedRegister_Unguarded, which does not make that test.  On that
	// screen the lamps belong to the test.
	u8 m_led[8];
	output_finder<64> m_led_out;
};

DECLARE_DEVICE_TYPE(WSA1R_CPANEL, wsa1r_cpanel_device)

#endif // MAME_MATSUSHITA_WSA1R_CPANEL_H
