// license:GPL2+
// copyright-holders:Felipe Sanches
/***************************************************************************

    Technics SX-WSA1R control panel HLE

    The SX-WSA1 keyboard's panel is a different board and is not modelled here.
    The evidence that this is the panel at all, and the wire format, are in
    wsa1r_cpanel.h.

    Every constant below is quoted with the prom_a / prom_b address that
    establishes it.

***************************************************************************/

#include "emu.h"
#include "wsa1r_cpanel.h"

#define LOG_FRAME (1U << 1)
#define LOG_LED   (1U << 2)
#define LOG_BTN   (1U << 3)

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(WSA1R_CPANEL, wsa1r_cpanel_device, "wsa1r_cpanel", "SX-WSA1R Control Panel HLE")


//-------------------------------------------------
//  the scan matrix
//
//  Eleven ports because the CP1 microcomputer drives SEG0..SEG10 and reads
//  SW0..SW7 (block diagram, manual page II-1).  The (0xC4) == 2 variant, the
//  rack, leaves SEG6 and SEG10 unwired -- PanelWireGroupMap_Variant2 has no entry for
//  wire 0xC6 or 0xCA -- so segment_is_wired() refuses them and a key bound
//  there is inert rather than injecting a packet the firmware files under
//  group id 0x20, "no such control".
//
//  Port names carry the wire position first and the legend in parentheses.
//  Legends printed beside the switch on the CP1/CP2 P.C. Diagram (PDF p.32)
//  cover SEG0..SEG2 and CP2's SEG7/SEG8; the rest are read off the P.C. board
//  page (PDF p.31).  The SX-WSA1 keyboard has two more scan columns and three
//  more pots and will need its own port map.
//
//  A handful of bits have a FUNCTION named by the ROM, marked below: three
//  power-on chords tested by the boot block (Panel_Chord_FactoryClear, Panel_Chord_RomVersion,
//  Panel_Chord_ThirdEntry) and the four service-screen keys in SEG1 (Panel_ServiceScreenSelect).
//
//  All of them read the panel's per-wire shadow at
//  RAM 0x2B20 + ((wire & 0x0F) | ((wire & 0x40) >> 2)), which holds the LAST
//  VALUE, not a change mask: SC1_RxOp0_ThreeByte does
//  `ex (XHL),A` then `xor A,(XHL)`, so the shadow takes the new value and the
//  mask is what gets queued.  Every chord test compares that byte for
//  EQUALITY, so an extra button held in the same segment kills the chord.
//-------------------------------------------------

static INPUT_PORTS_START(wsa1r_cpanel)
	PORT_START("CP_SEG0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG0 SW0 (PLAY MODE SOUND)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG0 SW1 (PLAY MODE COMBI)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG0 SW2 (EDIT MODE SOUND)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG0 SW3 (EDIT MODE COMBI)")
	// power-on chord: 0/4, 0/5 and 0/6 held = ROM-version LED display (Panel_Chord_RomVer_Seg0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG0 SW4 (BANK USER 1)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG0 SW5 (BANK USER 2)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG0 SW6 (BANK ROM/EXT)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG0 SW7 (BANK RE-MAP)")

	// SEG1 is the service-screen keypad.  sub_F953CD, reached from RESET
	// at Boot_ChordCall_ServiceScreen when the model strap says (0xC4) == 2, compares (0x2B31) --
	// wire 0xC1, this segment -- for equality:
	//
	//     0x02 -> recognised, no screen           SvcScreen_None
	//     0x04 -> screen 0xD9  PANEL CPU CHECK    SvcScreen_PanelCpuCheck
	//     0x08 -> screen 0xDA  SINE WAVE CHECK    SvcScreen_SineWaveCheck
	//     0x10 -> screen 0xDB  PANEL SW&LED CHECK SvcScreen_PanelSwLedCheck
	//     0x20 -> screen 0xDC  the screen cycler  SvcScreen_Cycler
	//
	// then stores the id at (0x2070) and sets (0x2071) = 0x80.  Equality, so one
	// key at a time.  Variant 2 only: on the SX-WSA1 the same call reads the
	// keybed instead (sub_F9530B).
	//
	// The boot block tests four chords in address order -- FACTORY CLEAR
	// Boot_ChordCall_FactoryClear, this one Boot_ChordCall_ServiceScreen, ROM VERSION Boot_ChordCall_RomVersion, then Boot_ChordCall_Fourth -- and
	// Panel_Chord_RomVersion's matched arm never returns, so a held ROM-version chord pre-empts
	// a service screen this test already latched.
	PORT_START("CP_SEG1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG1 SW0 (number 0)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG1 SW1 (number 1; power-on: recognised, no screen)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG1 SW2 (number 2; power-on: PANEL CPU CHECK)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG1 SW3 (number 3; power-on: SINE WAVE CHECK)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG1 SW4 (number 4; power-on: PANEL SW&LED CHECK)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG1 SW5 (number 5; power-on: screen cycler)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG1 SW6 (number 6)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG1 SW7 (number 7)")

	PORT_START("CP_SEG2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG2 SW0 (number 8)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG2 SW1 (number 9)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG2 SW2 (+/-)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG2 SW3 (ENTER)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG2 SW4 (PAGE down)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG2 SW5 (PAGE up)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG2 SW6 (COMPARE)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG2 SW7 (not fitted)")

	PORT_START("CP_SEG3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG3 SW0 (LCD soft key, RIGHT column, 1st from top)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG3 SW1 (LCD soft key, RIGHT column, 2nd)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG3 SW2 (LCD soft key, RIGHT column, 3rd)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG3 SW3 (LCD soft key, RIGHT column, 4th)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG3 SW4 (LCD soft key, RIGHT column, 5th)")
	// power-on chord: 3/5, 3/6 and 3/7 held = the third service entry (Panel_Chord_Third_Seg3)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG3 SW5 (-1)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG3 SW6 (+1)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG3 SW7 (EXIT)")

	PORT_START("CP_SEG4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG4 SW0 (under-LCD key, column 1, bottom)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG4 SW1 (under-LCD key, column 1, top)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG4 SW2 (under-LCD key, column 2, bottom)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG4 SW3 (under-LCD key, column 2, top)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG4 SW4 (under-LCD key, column 3, bottom)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG4 SW5 (under-LCD key, column 3, top)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG4 SW6 (under-LCD key, column 4, bottom)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG4 SW7 (under-LCD key, column 4, top)")

	PORT_START("CP_SEG5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG5 SW0 (under-LCD key, column 5, bottom)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG5 SW1 (under-LCD key, column 5, top)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG5 SW2 (under-LCD key, column 6, bottom)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG5 SW3 (under-LCD key, column 6, top)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG5 SW4 (under-LCD key, column 7, bottom)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG5 SW5 (under-LCD key, column 7, top)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG5 SW6 (under-LCD key, column 8, bottom)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG5 SW7 (under-LCD key, column 8, top)")

	PORT_START("CP_SEG7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG7 SW0 (MENU PART)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG7 SW1 (MENU SYSTEM)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG7 SW2 (MENU MIDI)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG7 SW3 (MENU DISK)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG7 SW4 (not fitted)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG7 SW5 (not fitted)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG7 SW6 (not fitted)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG7 SW7 (not fitted)")

	PORT_START("CP_SEG8")
	// power-on chord: FACTORY CLEAR, 8/0 and 8/1 held (Panel_Chord_FactoryClear)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG8 SW0 (REALTIME CREATOR 1~6)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG8 SW1 (REALTIME CREATOR RESET)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG8 SW2 (not fitted)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG8 SW3 (not fitted)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG8 SW4 (not fitted)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG8 SW5 (not fitted)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG8 SW6 (not fitted)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG8 SW7 (not fitted)")

	PORT_START("CP_SEG9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG9 SW0 (LCD soft key, LEFT column, 1st from top)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG9 SW1 (LCD soft key, LEFT column, 2nd)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG9 SW2 (LCD soft key, LEFT column, 3rd)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG9 SW3 (LCD soft key, LEFT column, 4th)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG9 SW4 (LCD soft key, LEFT column, 5th)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG9 SW5 (not fitted)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG9 SW6 (not fitted)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Panel SEG9 SW7 (not fitted)")

	// Wire 0xD3.  The handler at Ctrl_G3Ch3_Normalise maps the byte through the plain,
	// strictly monotone 0..127 ramp at Ctrl_Curve_Identity128 (no plateau anywhere in it).  The
	// rack's mechanical parts list has exactly one VOLUME KNOB.
	PORT_START("CP_VOLUME")
	PORT_ADJUSTER(80, "VOLUME")

	// Wire 0xD7.  Its dispatch slot (Ctrl_HandlerTable entry 31 -> Ctrl_ReportChanged) is a bare
	// `scf` -- no curve, no previous-value compare -- so every packet is accepted.  A
	// control that must never be de-duplicated is a RELATIVE encoder, and the KN5000's
	// twin protocol uses the same wire address 0xD7 for its endless wheel as
	// [0xD7, signed detent count] (kn5000_cpanel.cpp:269-271).  The rack's parts list
	// has exactly one DIAL WHEEL.  The signed-step encoding is inferred from those
	// two facts; prom_a's group-0x0F consumer has not been read.
	PORT_START("CP_DIAL")
	PORT_BIT(0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_NAME("DATA ENTRY DIAL")

	// The SAME wheel, dragged in a circle by src/mame/layout/wsa1r.lay's widget
	// script, which writes it through user_value.  It cannot be the field above:
	// an analog field's only Lua write path, set_value(), latches
	// m_use_adjoverride permanently and detaches the field from the input system,
	// so one drag would kill the keys and the mouse axis for the rest of the
	// session.  An adjuster's user_value has no such side effect, so the two
	// controls coexist and scan_tick() sums their wrap-aware deltas.  Same split,
	// and for the same reason, as the KN5000's ENCODER / ENCODER_DRAG pair
	// (kn5000_cpanel.cpp).  It carries no key binding and needs none.
	PORT_START("CP_DIAL_DRAG")
	PORT_ADJUSTER(50, "DATA ENTRY DIAL (mouse drag)")
INPUT_PORTS_END


ioport_constructor wsa1r_cpanel_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(wsa1r_cpanel);
}


wsa1r_cpanel_device::wsa1r_cpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WSA1R_CPANEL, tag, owner, clock),
	m_pos(0), m_len(2),
	m_resp_len(0), m_resp_pos(0),
	m_rx_enabled(false),
	m_scan_timer(nullptr), m_byte_timer(nullptr), m_req_timer(nullptr),
	m_requesting(false),
	m_vol_prev(0), m_vol_synced(false),
	m_dial_prev(0), m_dial_synced(false),
	m_dial_drag_prev(0), m_dial_drag_synced(false),
	m_atn_cb(*this), m_busy_cb(*this), m_sclk_cb(*this), m_rxd_cb(*this),
	m_seg(*this, "CP_SEG%u", 0U),
	m_volume(*this, "CP_VOLUME"),
	m_dial(*this, "CP_DIAL"),
	m_dial_drag(*this, "CP_DIAL_DRAG"),
	m_led_out(*this, "led%u", 0U)
{
	std::fill(std::begin(m_frame), std::end(m_frame), 0);
	std::fill(std::begin(m_resp), std::end(m_resp), 0);
	std::fill(std::begin(m_seg_prev), std::end(m_seg_prev), 0);
	std::fill(std::begin(m_led), std::end(m_led), 0);
}


void wsa1r_cpanel_device::device_start()
{
	m_scan_timer = timer_alloc(FUNC(wsa1r_cpanel_device::scan_tick), this);
	m_byte_timer = timer_alloc(FUNC(wsa1r_cpanel_device::deliver_byte), this);
	m_req_timer  = timer_alloc(FUNC(wsa1r_cpanel_device::request_tick), this);

	save_item(NAME(m_frame));
	save_item(NAME(m_pos));
	save_item(NAME(m_len));
	save_item(NAME(m_resp));
	save_item(NAME(m_resp_len));
	save_item(NAME(m_resp_pos));
	save_item(NAME(m_rx_enabled));
	save_item(NAME(m_requesting));
	save_item(NAME(m_last_tx));
	save_item(NAME(m_seg_prev));
	save_item(NAME(m_vol_prev));
	save_item(NAME(m_vol_synced));
	save_item(NAME(m_dial_prev));
	save_item(NAME(m_dial_synced));
	save_item(NAME(m_dial_drag_prev));
	save_item(NAME(m_dial_drag_synced));
	save_item(NAME(m_led));
}


void wsa1r_cpanel_device::device_reset()
{
	m_pos = 0;
	m_len = 2;
	m_resp_len = m_resp_pos = 0;
	m_rx_enabled = false;
	m_requesting = false;
	m_last_tx = attotime::zero;
	m_byte_timer->reset();
	m_req_timer->reset();
	std::fill(std::begin(m_seg_prev), std::end(m_seg_prev), 0);
	m_vol_synced = false;
	m_dial_synced = false;
	m_dial_drag_synced = false;

	// The two lines SC1_WaitTxDrain and SC1_TxFlush_Body test before they will touch the
	// link (SC1_WaitTxDrain / SC1_LineFree_PBBit4): "free" is P8 bit 5 HIGH and PB bit 4 LOW.  Get
	// this wrong and nothing is ever transmitted -- the four-way test bails to
	// SC1_TxFlush_Exit and no LED frame and no command leaves the CPU.
	m_sclk_cb(1);
	m_busy_cb(0);
	m_atn_cb(0);

	// 250 Hz, the same rate kn_cpanel_base_device uses.  Nothing in the WSA1 ROMs measures
	// the real MCU's scan period; this is a driver choice and is marked as one.
	m_scan_timer->adjust(attotime::from_hz(250), 0, attotime::from_hz(250));
}


//-------------------------------------------------
//  variant geometry
//-------------------------------------------------

bool wsa1r_cpanel_device::segment_is_wired(int seg) const
{
	if (seg < 0 || seg >= NUM_SEG)
		return false;
	return (seg != 6) && (seg != 10);      // prom_a 0xF8A189: 0xC6 and 0xCA are 0x20 = none
}


//-------------------------------------------------
//  CPU 1 -> panel
//
//  The frame length rule is the firmware's own, from the two places it sets
//  the "bytes still expected" counter (0x2A81) after a first byte:
//  SC1_State08_TxFromRing (transmit, SC1_State08_TxFromRing) and SC1_State20_RxFirstByte
//  (receive, SC1_State20_RxFirstByte).  Both are
//      (0x2A81) = 2 ; if ((b & 0x3F) >= 0x30) (0x2A81) = (b & 0x0F) + 3
//  which is exactly what SC1_TxOp3_Run emits (header + (n & 0x0F) + 2 more)
//  and what SC1_RxOp6_Run consumes (header + address + (n & 0x0F) + 1 data).
//
//  So the two run encoders agree, at n+3 bytes per message, and the length
//  counter is a third independent witness to it.
//-------------------------------------------------

void wsa1r_cpanel_device::tx_byte(u8 data)
{
	m_last_tx = machine().time();

	if (m_pos == 0)
		m_len = ((data & 0x3f) >= 0x30) ? ((data & 0x0f) + 3) : 2;

	if (m_pos < int(sizeof(m_frame)))
		m_frame[m_pos] = data;
	m_pos++;

	if (m_pos >= m_len)
	{
		frame_complete();
		m_pos = 0;
		m_len = 2;
	}
}


void wsa1r_cpanel_device::frame_complete()
{
	const u8 hdr = m_frame[0];
	LOGMASKED(LOG_FRAME, "panel <- CPU: %d bytes, hdr %02X\n", m_len, hdr);

	if ((hdr & 0x30) == 0x30)
	{
		// Run frame: [HDR][FIRST_ADDR][DATA] x ((HDR & 0x0F) + 1), addresses stepping by 1.
		// SC1_TxOp3_Run builds it; nothing in this firmware has been seen
		// to produce one, but the codec accepts it, so decode it.
		const int n = (hdr & 0x0f) + 1;
		u8 addr = (hdr & 0xc0) | (m_frame[1] & 0x1f);
		for (int i = 0; i < n && (2 + i) < m_len; i++, addr++)
			led_frame(addr, m_frame[2 + i]);
		return;
	}

	const u8 addr = hdr, data = m_frame[1];

	// The LED wire table is the authority, not the shape of the address: register 7
	// maps to wire address 0x00, and Panel_RefreshLeds walks all eight registers
	// (`ld B,0x08` at Panel_RefreshLeds), so that frame really is emitted and does not
	// look like an LED address at all.
	if (led_frame(addr, data))
		return;

	if ((addr & 0xf0) == 0xc0)
	{
		LOGMASKED(LOG_LED, "LED frame for a wire address the table does not map: %02X = %02X\n",
				addr, data);
		return;
	}

	// Everything else is a command.  The seven the firmware ever sends first are, in ROM
	// order: 0xDF 0xD2 / 0xDF 0x1A / 0xDD 0x03 / 0xDE 0x80 (SC1_ConfigurePort, prom_b
	// SC1_ConfigurePort_Cmds..SC1_ConfigurePort_LastCmd), 0xE0 0x00 (SC1_Cmd_E0_ReadStatus), 0xE3 0x00 / 0xE2 0x08 /
	// 0xE3 0x10 (SC1_Cmd_E3_E2_E3) and 0xEF 0x00 (SC1_Cmd_EF).
	//
	// What they ask for is not established.  What is, is what the firmware does with
	// the answer: SC1_Cmd_E0_ReadStatus zeroes both rx ring indices,
	// sends (0xE0,0x00), waits six ticks and sets bit 3 of (0x2A85) if the WRITE index
	// moved.  So the only thing it measures is "did the panel answer at all".  Answering
	// with a two-byte packet whose type field is 3, 4 or 5 satisfies that and is then
	// DISCARDED by SC1_RxOp3_Discard without entering the message queue --
	// exactly the KN5000's TYPE 3 sync packet (kn5000_cpanel.cpp, send_sync_packet).
	//
	// The header byte itself is a CHOICE: 0xD8 is type 3 (bits 5:3) with bits 7:6 = 11,
	// which is what every live address on THIS link carries.  The KN5000 sends 0x18, the
	// same type with bits 7:6 = 00.  Only the TYPE field is decoded by the receiver, so
	// both work; the exact byte a real M37471M2196S sends here is unknown.
	static const u8 sync[2] = { 0xd8, 0x00 };
	switch (addr)
	{
	case 0xdd: case 0xde: case 0xdf:   // the open sequence, sent with interrupts masked
	case 0xe0: case 0xe2: case 0xe3: case 0xef:
		queue_frame(sync, 2);
		break;
	default:
		LOGMASKED(LOG_FRAME, "unhandled command %02X %02X\n", addr, data);
		break;
	}
}


//-------------------------------------------------
//  LED registers
//
//  Panel_RefreshLeds walks EIGHT registers, comparing the
//  want-buffer at RAM 0x20D0..0x20D7 with the sent-shadow at 0x20F0..0x20F7
//  and calling Panel_SetLedRegister for each one that differs.
//  That routine maps the register INDEX through one of two tables --
//  Panel_LedWireTable on this machine -- to the wire address, then pushes [ADDR][DATA]
//  into the outbound queue at 0x2BA0.
//
//  Eight, not seven: the table ends C1 C2 C9 CA CB CC C3 00 and the loop emits
//  that trailing 0x00 as a wire address like any other.
//-------------------------------------------------

bool wsa1r_cpanel_device::led_frame(u8 addr, u8 data)
{
	// the register -> wire address table at Panel_LedWireTable
	static const u8 wire[8] = { 0xc1, 0xc2, 0xc9, 0xca, 0xcb, 0xcc, 0xc3, 0x00 };

	for (int reg = 0; reg < 8; reg++)
	{
		if (wire[reg] != addr)
			continue;
		if (m_led[reg] == data)
			return true;
		m_led[reg] = data;
		for (int bit = 0; bit < 8; bit++)
			m_led_out[reg * 8 + bit] = BIT(data, bit);
		LOGMASKED(LOG_LED, "LED reg %d (wire %02X) = %02X\n", reg, addr, data);
		return true;
	}
	return false;
}


//-------------------------------------------------
//  panel -> CPU 1
//
//  One INT6 per MESSAGE, not per byte: INT6_SC1_PeerRequest
//  turns RXE on, sets SC1CR bit 0 (the panel now clocks), selects INTES1=0x05
//  (receive only) and sets state 0x20.  SC1_State20_RxFirstByte then takes the
//  length from the first byte and SC1_State24_RxNextByte counts the rest down,
//  re-arming INT6 when it is done.  So: raise ATN, wait for the firmware to
//  enable RX, then push the message's bytes one at a time.
//-------------------------------------------------

//  Every message this device sends is exactly two bytes, and the queue is a FIFO
//  of two-byte messages rather than a byte stream, because the CPU's receive
//  state machine is per message: INT6_SC1_PeerRequest accepts one request,
//  SC1_State20_RxFirstByte takes the length from the first byte -- 2 for every
//  address this device uses, since (addr & 0x3F) < 0x30 for 0xC0..0xD7 -- and
//  SC1_State24_RxNextByte re-arms INT6 when the count runs out.  Concatenating
//  two messages into one delivery would hand the second one to a state machine
//  that has stopped expecting bytes.
void wsa1r_cpanel_device::queue_frame(const u8 *bytes, int n)
{
	if (m_resp_pos >= m_resp_len)
		m_resp_pos = m_resp_len = 0;
	if (m_resp_len + n > RESP_MAX)
		return;                              // the real MCU's queue would drop it too
	const bool was_idle = (m_resp_pos == m_resp_len);
	for (int i = 0; i < n; i++)
		m_resp[m_resp_len++] = bytes[i];
	if (was_idle && !m_requesting)
		start_request();
}


void wsa1r_cpanel_device::start_request()
{
	m_requesting = true;
	m_req_timer->adjust(attotime::zero, 0);
}


//-------------------------------------------------
//  the attention line: a pulse, and a retry
//
//  Holding the line high does not work.  INT6's request flag is bit 3 of
//  INTE67, and the SC1 module writes that register with bit 3 clear at
//  eighteen sites -- 0x85 to arm INT6 at level 5, 0x8F to park it at level 7,
//  which is never dispatched (SC1_Inte67_Arm, SC1_Inte67_Park and sixteen more).  A
//  request raised while the module is transmitting latches into a masked flag
//  and is discarded by the write that re-arms INT6.
//
//  So the panel asks again: a short pulse, repeated every 2 ms until the CPU
//  turns RXE on.  The real part's pulse width and retry interval are not
//  known; neither constant below is claimed to be the hardware's.
//
//  The busy line is not raised while merely asking.  PB bit 4 blocks CPU 1
//  from transmitting at all (SC1_WaitTxDrain), so asserting it before the CPU
//  has accepted the request would silence the conversation the panel is trying
//  to join.  It goes high in rx_enable() and low when the last byte is gone.
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(wsa1r_cpanel_device::request_tick)
{
	if (!m_requesting)
	{
		m_atn_cb(0);
		return;
	}

	if (param == 0)
	{
		// Never ask while CPU 1 is mid-frame.  INT6_SC1_PeerRequest (prom_b
		// INT6_SC1_PeerRequest) opens `cp (0x2A81),0x00` -- the bytes-still-expected counter,
		// used for transmit as well as receive -- and if it is not zero it takes
		// the arm at INT6_PeerRequest_BusyArm, which steps the RX ring's write index back one on
		// the assumption that the peer will re-send the byte it just took.  This
		// device never re-sends, so such an edge desynchronises the ring by one
		// byte permanently and every later message pairs the previous message's
		// data byte with this one's address byte.
		//
		// m_pos alone is not enough: the firmware sets (0x2A81) before writing the
		// first byte to SC1BUF and clears it after the last (SC1_State08_TxFromRing / SC1_RxDone_ClearCount),
		// so a quiet window is needed as well.  1 ms is a driver choice, like the
		// pulse width above it.
		if (m_pos != 0 || (machine().time() - m_last_tx) < attotime::from_usec(1000))
		{
			m_req_timer->adjust(attotime::from_usec(2000), 0);
			return;
		}
		m_atn_cb(1);
		m_req_timer->adjust(attotime::from_usec(50), 1);
	}
	else
	{
		m_atn_cb(0);
		m_req_timer->adjust(attotime::from_usec(2000), 0);
	}
}


void wsa1r_cpanel_device::rx_enable(int state)
{
	m_rx_enabled = bool(state);

	if (!m_rx_enabled || m_resp_pos >= m_resp_len)
		return;

	// The CPU has accepted: stop asking, take the link, and start clocking bytes in.
	m_requesting = false;
	m_req_timer->reset();
	m_atn_cb(0);
	m_busy_cb(1);
	m_byte_timer->adjust(attotime::from_usec(60), 0);
}


TIMER_CALLBACK_MEMBER(wsa1r_cpanel_device::deliver_byte)
{
	if (m_resp_pos >= m_resp_len)
		return;

	m_rxd_cb(m_resp[m_resp_pos++]);

	// param counts bytes within THIS message: 0 was the address, 1 was the data.
	if (param == 0)
	{
		m_byte_timer->adjust(attotime::from_usec(120), 1);
		return;
	}

	// Message complete: give the link back.  Anything still queued has to ask again,
	// because the firmware re-arms INT6 at the end of a message and expects the next
	// one to arrive the same way this one did.
	m_busy_cb(0);
	m_atn_cb(0);

	if (m_resp_pos >= m_resp_len)
		m_resp_pos = m_resp_len = 0;
	else
		start_request();
}



//-------------------------------------------------
//  the periodic scan
//
//  Buttons are reported as [0xC0 | segment][bitmask].  The header's type field
//  falls out of the segment number for free -- segments 0..7 give type 0 and
//  8..15 type 1, which is exactly why SC1_RxOpTable entries [0] and [1] are
//  the same handler.  SC1_RxOp0_ThreeByte then XORs the mask against its own
//  shadow at RAM 0x2B20 + ((addr & 0x0F) | ((addr & 0x40) >> 2)) -- the +0x10 is
//  CONDITIONAL on address bit 6 (SC1_RxOp0_ThreeByte does `and W,0x4F` and then
//  `sub W,0x30` only when bit 6 is set), which is true of every 0xC0..0xCF
//  address but is not the rule -- and hands the foreground
//  {address, mask, CHANGED-bits} -- so sending the whole segment state, not
//  just the change, is correct and is what the shadow table is there for.
//
//  Analogue controls are [0xD0 | sub][value]; the firmware appends its own
//  0xFF third byte (SC1_RxOp2_AppendFF), so it must NOT be sent here.
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(wsa1r_cpanel_device::scan_tick)
{
	for (int seg = 0; seg < NUM_SEG; seg++)
	{
		if (!segment_is_wired(seg))
			continue;
		const u8 v = m_seg[seg].read_safe(0);
		if (v == m_seg_prev[seg])
			continue;
		m_seg_prev[seg] = v;
		const u8 pkt[2] = { u8(0xc0 | seg), v };
		LOGMASKED(LOG_BTN, "segment %d = %02X\n", seg, v);
		queue_frame(pkt, 2);
	}

	// Wire 0xD3.  Ctrl_G3Ch3_Normalise halves the byte and looks it up in the 0..127 ramp at
	// Ctrl_Curve_Identity128, so the wire value is a full 8-bit pot reading.
	{
		const u8 v = u8((m_volume.read_safe(0) * 255 + 50) / 100);
		if (!m_vol_synced)
		{
			m_vol_prev = v;
			m_vol_synced = true;         // adopt silently: a frame nobody is servicing yet
		}                                //  would sit in the queue and block every later ATN
		else if (v != m_vol_prev)
		{
			m_vol_prev = v;
			const u8 pkt[2] = { 0xd3, v };
			queue_frame(pkt, 2);
		}
	}

	// Wire 0xD7, the DATA ENTRY DIAL.  Sent as a SIGNED STEP -- see the ioport comment for
	// why, and for the fact that this is inference and not decode.
	//
	// TWO controls feed one wheel: the IPT_DIAL (keys and the mouse axis, 256 positions)
	// and the layout's drag adjuster (0..100, wrapped by the script).  Both are relative,
	// so their deltas simply add; neither one's absolute value means anything.
	{
		s32 d = dial_delta(m_dial, m_dial_prev, m_dial_synced, 256);
		d += dial_delta(m_dial_drag, m_dial_drag_prev, m_dial_drag_synced, 101);
		if (d != 0)
		{
			const s8 step = s8(std::clamp<s32>(d, -64, 63));
			const u8 pkt[2] = { 0xd7, u8(step) };
			queue_frame(pkt, 2);
		}
	}
}


//-------------------------------------------------
//  one relative control's movement since the last scan
//
//  Wrap-aware, because both fields wrap: the IPT_DIAL at 256 and the layout's
//  adjuster at 101 (the script does `user_value = (user_value + n) % 101`).  The
//  first read only ADOPTS the position -- a step reported before the firmware is
//  servicing the link would sit in the queue and block every later request.
//-------------------------------------------------

s32 wsa1r_cpanel_device::dial_delta(optional_ioport &port, s32 &prev, bool &synced, s32 modulus)
{
	if (!port)
		return 0;

	const s32 pos = port->read();
	s32 d = pos - prev;
	if (d > modulus / 2) d -= modulus;
	else if (d < -modulus / 2) d += modulus;

	prev = pos;
	if (!synced)
	{
		synced = true;
		return 0;
	}
	return d;
}
