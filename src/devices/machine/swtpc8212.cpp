// license:BSD-3-Clause
// copyright-holders:AJR,68bit
/****************************************************************************

Driver for Southwest Technical Products video terminal 8210.

MC6802P, 2xMC6821P, INS8250N, MCM66750, MC6845P, bank of 8 dips, crystals
17.0748 (video), 1.8432 (cpu/uart). On the back is a 25-pin RS-232 port, and a
25-pin printer port.

The 8212 terminal appears similar in design to the CT-82 terminal for which
there is some documentation and a user guide. In particular it appears closest
to the 'Version B1'. There was also a 8209 terminal with an 9 inch CRT versus
the 12 inch CRT in the 8212. This terminal has also been labeled at the
CT8200.  http://www.swtpc.com/mholley/CT_82/CT82_Index.htm

The 8212 has three CRT controller configurations:
 1. 82 x 20 characters, with a height of 14 scan lines.
 2. 82 x 24 characters, with a height of 12 scan lines.
 3. 92 x 22 characters, with a height of 12 scan lines.

There are two character generators:
1. The MCM66750 with 7x9 sized characters with some descending by 3 lines
giving an effective 7x12 sized character set.
2. An EPROM defining 8x16 sized characters for which only 12 scan lines are
used, and this is used for the graphics mode.

These appear to map onto the CT-82 screen formats options as follows:
Format I:   82 by 20, MCM66750 character set. Ctrl-\ crtl-Q
Format II:  82 by 24, MCM66750 character set. Ctrl-\ crtl-R
Format III: 82 by 20, EPROM character set.    Ctrl-\ crtl-S
Format IV:  82 by 24, EPROM character set.    Ctrl-\ crtl-T
Graphics:   92 by 22, EPROM character set.    Ctrl-] crtl-V

The semi-graphics character set is defined in an EPROM, and that EPROM defines
these characters but here is a brief description of the graphics codes. It
includes graphics characters that support a two by three pixel array per
character. The codes 0x20 to 0x5f remain ASCII characters. The codes 0x00 to
0x1f have the following graphics encoding:

   000xxxxx
   ----------
   | D0 | D3 |
   -----------
   | D1 | D4 |
   -----------
   | D2 |  0 |
   -----------

The codes 0x60 to 0x7f have the following graphics encoding:

   011xxxxx
   -----------
   | D0 | D3 |
   -----------
   | D1 | D4 |
   -----------
   | D2 |  1 |
   -----------

The terminal includes a parallel printer output and Ctrl-] Ctrl-K enables
printer pass-through and Ctrl-] Ctrl-G disables pass-through.

The terminal appears to include light pen support, and the positions is read
via Ctrl-] Ctrl-B, TODO.

The numeric keypad is separte to the keyboard, returning a separate 4 bit code
and strobe. The CT-82 documentation suggests that the order of the bits on the
keypad bus is reversed. Photos of the 8212 show a 16 key keypad. The ROM has a
table starting at 0xb8e4 that maps the keypad code read from PIA 0 Port A bits
4 to 7 into a character code. This table has two vectors of 16 bytes, and the
first 16 are used when in the 'cursor' mode and the later 16 are uses when in
the 'numeric' keypad mode. In the 'cursor' mode, four of the keys are
underfined. TODO the labels of all the keypad keys could not be read from the
photo found, so could use a better photo of the keypad and an update here.

The keypad codes in the ROM appear consistent with the CT-82
documentation. However the codes for the left and right cursor keys are not
consistent with a UniFlex termcap entry for the 'ct8200' which has the right
cursor code being 0x1d and the left cursor code being 0x1e. The standard CT-82
codes of Ctrl-D and Ctrl-I for these keys are not good choices for a 'Unix'
like terminal and it appears likely that there is either a ROM revision making
this trivial change to better work with UniFlex or that people did patch the
keypad codes. The keypad codes are single character codes so there is very
limit room to add more, and some of keys were not even assigned a code,
history! A ROM patch assigning code friendlier to Unix is included in this
emulator.

A glitch in the interaction of the keyboard and the keypad has been noted. When
typing fast on the keyboard a character may not be sent. Then if another key is
typed on the keyboard the unsent code is lost. But if another key is typed on
the keypad then the unsent keyboard code is then sent and the keypad code is
lost. The firmware has a one character send buffer. TODO This might be a
firmware issue, or might be a problem with the emulation of the PIA or UART or
the keyboard or keypad strobes etc.

TODO A generic keyboard is used. Might need some work to get a better keyboard
emulation. The CT-82 documentation mentions a 'Break' key.

TODO A generic 'beep' is used. Might want to compare this with the actual 8212
to better emulate the sound.

TODO the CB2 input of 'pia0' at address 0x0080 is polled by the firmware, and
it is the first input checked in the ISR. On a CB2 interrupt the ISR checks
for a 'break' condition on the UART and if not then it clears the UART OUT1
output. There are other suggestions in the firmware that the OUT1 and OUT2
UART lines are being driven. The operation here is unknown?

TODO Confirm the CPU XTAL. The terminal emulation appears slugish compared
with the documented claims, the hardware flow control is needed even at 9600
and it does not operate at 38400 as documented. The CT82 manual mentions an
optional 4MHz crystal and that would run the CPU at 1MHz which is the rated
speed for the CPU and perhaps the 8212 uses that 4MHz XTAL or at least
something faster the current 1.8432MHz XTAL?

The terminfo database does not appear to have an entry for this terminal, and
the following is offered that attempts to use the features of the terminal that
map to ncurses and this helped testing and it appears to run the ncurses tests
and some utilities ok. The cursor keys are set for the patched ROM option. Keep
in mind that it is an ASCII terminal so try an ISO-8859-1 locale, and also that
it has no tabs so it needs tab to space translation.

swtp|ct8212|southwest technical products ct8212,
	cols#82, lines#24,
	bel=^G, civis=^E, clear=^L, cnorm=^U, cr=\r,
	cub=^\^D%p1%c, cub1=^D, cud=^\^B%p1%c, cud1=^B,
	cuf1=^R, cup=^K%p2%{32}%+%c%p1%{32}%+%c,
	cuu=^\^A%p1%c, cuu1=^A, dch1=^\^H, dl1=^Z, ed=^V, el=^F,
	el1=^\^F, home=^P, hpa=^\^W%p1%{32}%+%c, ich1=^\^X,
	il1=^\^Y, ind=^N,
	is2=^_^A$<250>^\^R$<50>^^^D^^^T^_^J\040^^^G^^^O^^^Z^]^W^I^R,
	kbs=^H, kcub1=^B, kcud1=^N, kcuf1=^F, kcuu1=^P, khome=^A,
	ll=^C, mc4=^]^G, mc5=^]^K, nel=\r\n, ri=^O, rmir=, rmso=^^^F,
	smir=, smso=^^^V, vpa=^\^G%p1%{32}%+%c,

****************************************************************************/

#include "emu.h"
#include "machine/swtpc8212.h"
#include "machine/input_merger.h"
#include "screen.h"
#include "speaker.h"

swtpc8212_device::swtpc8212_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_maincpu(*this, "maincpu")
	, m_pia0(*this, "pia0")
	, m_pia1(*this, "pia1")
	, m_uart(*this, "uart")
	, m_crtc(*this, "crtc")
	, m_chargen1(*this, "chargen1")
	, m_chargen2(*this, "chargen2")
	, m_video_ram(*this, "videoram")
	, m_config(*this, "CONFIG")
	, m_keypad(*this, "KEYPAD")
	, m_one_stop_bit(*this, "ONE_STOP_BIT")
	, m_bell_timer(nullptr)
	, m_beeper(*this, "beeper")
	, m_printer(*this, "printer")
	, m_rs232_conn_txd_handler(*this)
	, m_rs232_conn_dtr_handler(*this)
	, m_rs232_conn_rts_handler(*this)
{
}

swtpc8212_device::swtpc8212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: swtpc8212_device(mconfig, SWTPC8212, tag, owner, clock)
{
}

WRITE8_MEMBER(swtpc8212_device::latch_w)
{
	// Bits 0 to 3 control outputs that are intended to control and tape
	// 'read' and 'punch' operations. These are strobes, about 10usec, and
	// intended to trigger on the falling edge.
	//
	// Bit 0 - Read on output
	// Bit 1 - Read off output
	// Bit 2 - Punch on output
	// Bit 3 - Punch off output

	// Bit 4 - ?? usually high
	// Bit 5 - ?? usually low

	// Bits 6 and 7 change with the screen format.
	//
	// Bit 6 is zero for formats I and II, and one for formats III and IV
	// and for the graphics format. Assume this selects between the
	// characters sets, so formats III and IV might use an alternative
	// custom text character set.
	//
	// Bit 6 - character set: 0 - standard; 1 - alternate/graphics.
	//
	// Bit 7 is zero in formats I, II, III, and IV, and one for the
	// graphics format. Assume this controls the horizontal
	// inter-character gap, eliminating the gap for the graphics format.
	//
	// Bit 7 - character width: 0 - 9 dots; 1 - 8 dots.
	//
	if (BIT(data, 7) == 0)
		m_crtc->set_char_width(9);
	else
		m_crtc->set_char_width(8);

	m_latch_data = data;
}


READ8_MEMBER(swtpc8212_device::pia0_pa_r)
{
	// PA0 controls the 'duplex' mode, the echoing back of characters, and
	// appears to connect to a switch on the outer casing.
	//
	// PA1 On the CT-82 this enabled or disabled use of an optional ROM,
	// but that function is disabled in the 8212, and this is probably
	// unused.
	//
	// PA2 is Jumper B, and PA3 is Jumper A.
	uint8_t config = m_config->read();

	return (m_keypad_data << 4) | config;
}

READ8_MEMBER(swtpc8212_device::pia0_pb_r)
{
	return m_keyboard_data;
}

void swtpc8212_device::keyboard_put(uint8_t data)
{
	m_keyboard_data = data;
	// Triggers on the falling edge.
	m_pia0->cb1_w(ASSERT_LINE);
	m_pia0->cb1_w(CLEAR_LINE);
	m_pia0->cb1_w(ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(swtpc8212_device::keypad_changed)
{
	uint16_t keys = m_keypad->read();
	uint8_t n;

	for (n = 0; n < 16; n++)
	{
		if (BIT(keys, n))
			break;
	}

	m_keypad_data = n;

	if (newval)
	{
		m_pia0->ca1_w(ASSERT_LINE);
		m_pia0->ca1_w(CLEAR_LINE);
		m_pia0->ca1_w(ASSERT_LINE);
	}
}

WRITE_LINE_MEMBER(swtpc8212_device::pia0_ca2_w)
{
	if (state == 0)
	{
		m_beeper->set_state(1);
		m_bell_timer->reset(attotime::from_msec(250));
	}
}

WRITE8_MEMBER(swtpc8212_device::pia1_pa_w)
{
	// External parallel printer data output.
	m_printer_data = data;
}

READ_LINE_MEMBER(swtpc8212_device::pia1_ca1_r)
{
	// External parallel printer busy input.
	return 0;
}

WRITE_LINE_MEMBER(swtpc8212_device::pia1_ca2_w)
{
	// External parallel printer data ready.

	// Trigger on the falling edge.
	if (m_printer_data_ready == 1 && state == 0)
	{
		m_printer->output(m_printer_data);
		// Toggle the printer busy line as the software waits for a
		// falling edge.
		m_pia1->ca1_w(CLEAR_LINE);
		m_pia1->ca1_w(ASSERT_LINE);
		m_pia1->ca1_w(CLEAR_LINE);
	}
	m_printer_data_ready = state;
}

MC6845_UPDATE_ROW(swtpc8212_device::update_row)
{
	int x = 0;
	uint8_t *chargen = BIT(m_latch_data, 6) == 0 ? m_chargen1 : m_chargen2;

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_video_ram[(ma + column) & 0x07ff];
		int dcursor = (column == cursor_x);

		offs_t address = ra < 16 ? (code & 0x7f) | (ra & 0x0f) << 7 : 0;
		uint8_t data = chargen[address];
		uint8_t intensity = BIT(code, 7);

		for (int bit = 0; bit < 8; bit++)
		{
			int dout = BIT(data, 7);
			uint32_t font_color = 0;
			if ((dcursor ^ dout) && de)
			{
				if (intensity)
					font_color = rgb_t(0x10, 0xff, 0x10);
				else
					font_color = rgb_t(0x00, 0xd0, 0x00);
			}
			bitmap.pix32(y, x++) = font_color;
			data <<= 1;
		}

		// Gap between characters
		if (BIT(m_latch_data, 7) == 0)
			x++;
	}
}

void swtpc8212_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case BELL_TIMER_ID:
		m_beeper->set_state(0);
		break;
	}
}

WRITE_LINE_MEMBER(swtpc8212_device::rs232_conn_dcd_w)
{
	m_uart->dcd_w(state);
}

WRITE_LINE_MEMBER(swtpc8212_device::rs232_conn_dsr_w)
{
	m_uart->dsr_w(state);
}

WRITE_LINE_MEMBER(swtpc8212_device::rs232_conn_ri_w)
{
	m_uart->ri_w(state);
}

WRITE_LINE_MEMBER(swtpc8212_device::rs232_conn_cts_w)
{
	m_uart->cts_w(state);
}

WRITE_LINE_MEMBER(swtpc8212_device::rs232_conn_rxd_w)
{
	m_uart->rx_w(state);
}

WRITE_LINE_MEMBER(swtpc8212_device::write_txd)
{
	m_rs232_conn_txd_handler(state);
}

WRITE_LINE_MEMBER(swtpc8212_device::write_dtr)
{
	m_rs232_conn_dtr_handler(state);
}

WRITE_LINE_MEMBER(swtpc8212_device::write_rts)
{
	m_rs232_conn_rts_handler(state);
}

void swtpc8212_device::device_resolve_objects()
{
	m_rs232_conn_dtr_handler.resolve_safe();
	m_rs232_conn_rts_handler.resolve_safe();
	m_rs232_conn_txd_handler.resolve_safe();
}

void swtpc8212_device::device_start()
{
	m_bell_timer = timer_alloc(BELL_TIMER_ID);

	save_item(NAME(m_latch_data));
	save_item(NAME(m_keyboard_data));
	save_item(NAME(m_printer_data));
	save_item(NAME(m_printer_data_ready));
}

void swtpc8212_device::device_reset()
{
	m_keyboard_data = 0;
	m_pia0->cb1_w(ASSERT_LINE);
	m_keypad_data = 0;
	m_pia0->ca1_w(ASSERT_LINE);

	m_latch_data = 0x1f;

	m_beeper->set_state(0);

	m_printer_data = 0;
	m_printer_data_ready = 1;
	m_pia1->ca1_w(CLEAR_LINE);

	if (m_one_stop_bit->read())
	{
		// Patch the firmware to use one stop bit.
		uint8_t* program = memregion("program")->base();
		program[0x01ad] = 0x02;
	}
}

void swtpc8212_device::mem_map(address_map &map)
{
	map(0x0000, 0x007f).ram();
	map(0x0080, 0x0083).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x0088, 0x0088).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0089, 0x0089).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x008c, 0x008c).w(FUNC(swtpc8212_device::latch_w));
	map(0x0090, 0x0097).rw("uart", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x0098, 0x009b).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x4000, 0x47ff).mirror(0x1800).ram().share(m_video_ram);
	map(0xb800, 0xbfff).rom().region("program", 0);
	map(0xc000, 0xc7ff).mirror(0x3800).rom().region("program", 0x800);
}




INPUT_PORTS_START(swtpc8212)

	PORT_START("KEYPAD")
	PORT_BIT(0x0002U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("7 / Form") PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x0004U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8 / Xmit") PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x0001U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9 / ??") PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x0008U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("\xC3\xB7 / ??") PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x0200U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4 / Scroll Up") PORT_CODE(KEYCODE_PGDN) PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x0400U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5 / \xE2\x86\x91") PORT_CODE(KEYCODE_UP) PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x0100U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6 / Insert") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x0800U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("\xC3\x97 / ??") PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x0020U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1 / \xE2\x86\x90") PORT_CODE(KEYCODE_LEFT) PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x0040U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2 / Home") PORT_CODE(KEYCODE_HOME) PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x0010U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3 / \xE2\x86\x92") PORT_CODE(KEYCODE_RIGHT) PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x0080U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("- / LF") PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x2000U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("0 / Scroll Down") PORT_CODE(KEYCODE_PGUP) PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x4000U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("0 / \xE2\x86\x93") PORT_CODE(KEYCODE_DOWN) PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x1000U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(". / Delete") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)
	PORT_BIT(0x8000U, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("+ / CR") PORT_CHANGED_MEMBER(DEVICE_SELF, swtpc8212_device, keypad_changed, 0)

	PORT_START("DIP_SWITCHES")
	PORT_DIPNAME(0x1f, 0x19, "Baud Rate") PORT_DIPLOCATION("DIP:4,3,2,1,0")
	PORT_DIPSETTING(0x04, "110")
	PORT_DIPSETTING(0x0a, "300")
	PORT_DIPSETTING(0x0d, "600")
	PORT_DIPSETTING(0x0f, "1200")
	PORT_DIPSETTING(0x12, "2400")
	PORT_DIPSETTING(0x16, "4800")
	PORT_DIPSETTING(0x18, "7200")
	PORT_DIPSETTING(0x19, "9600")
	PORT_DIPSETTING(0x1c, "19200")
	PORT_DIPSETTING(0x1f, "38400")
	PORT_DIPNAME(0x20, 0x00, "Mode switch") PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(0x00, "Conversational")
	PORT_DIPSETTING(0x20, "Page edit")
	PORT_DIPNAME(0x40, 0x00, "No Parity") PORT_DIPLOCATION("DIP:6")
	PORT_DIPSETTING(0x00, "No Parity")
	PORT_DIPSETTING(0x40, "Parity")
	PORT_DIPNAME(0x80, 0x00, "Parity Select") PORT_DIPLOCATION("DIP:7")
	PORT_DIPSETTING(0x00, "Odd or Mark")
	PORT_DIPSETTING(0x80, "Even or Space")

	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x01, "Duplex")
	PORT_CONFSETTING(0x00, "Full duplex")
	PORT_CONFSETTING(0x01, "Half duplex")
	PORT_CONFNAME(0x02, 0x00, "Option ROM (Not used)")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x02, DEF_STR(Off))
	PORT_CONFNAME(0x04, 0x04, "Parity Select (Jumper B)")
	PORT_CONFSETTING(0x00, "Odd or Even (On)")
	PORT_CONFSETTING(0x04, "Mark or Space (Off)")
	PORT_CONFNAME(0x08, 0x08, "Data bits (Jumper A)")
	PORT_CONFSETTING(0x00, "7 bit data (On)")
	PORT_CONFSETTING(0x08, "8 bit data (Off)")

	PORT_START("ONE_STOP_BIT")
	PORT_CONFNAME(1, 1, "One stop bit patch")
	PORT_CONFSETTING(0, "No")
	PORT_CONFSETTING(1, "Yes - apply patch")

INPUT_PORTS_END


ROM_START( swtpc8212 )
	ROM_REGION( 0x1000, "program", 0 )
	ROM_LOAD( "8224g_ver.1.1_6oct80.ic1", 0x0000, 0x0800, CRC(7d7f3c21) SHA1(f7e6e20b36a1c724a4e348bc784d0b7b5fb462a3) )
	ROM_LOAD( "8224g_ver.1.1_6oct80.ic2", 0x0800, 0x0800, CRC(2b118c22) SHA1(5fa031c834c7c582d5715764941499fcef51f477) )

	ROM_REGION( 0x0800, "chargen1", 0 )
	ROM_LOAD( "mcm66750.rom",  0x0000, 0x0800, CRC(aedc2830) SHA1(49ce17d5b5cefb24e89ed3fd59887a652501b919) )
	ROM_REGION( 0x0800, "chargen2", 0 )
	ROM_LOAD( "grafix_8x12_22aug80.bin",  0x0000, 0x0800, CRC(a525ed65) SHA1(813d2e85ddb258c5b032b959e695ad33200cbcc4) )
ROM_END

void swtpc8212_device::device_add_mconfig(machine_config &config)
{
	M6802(config, m_maincpu, 1.8432_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &swtpc8212_device::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6802_IRQ_LINE);

	// PA - various jumpers and keypad inputs.
	// PB - parallel keyboard data input.
	// CA1 - parallel keypad input strobe.
	// CA2 - output, bell.
	// CB1 - parallel keyboard input strobe.
	// CB2 ??
	PIA6821(config, m_pia0);
	m_pia0->readpa_handler().set(FUNC(swtpc8212_device::pia0_pa_r));
	m_pia0->readpb_handler().set(FUNC(swtpc8212_device::pia0_pb_r));
	m_pia0->ca2_handler().set(FUNC(swtpc8212_device::pia0_ca2_w));
	m_pia0->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_pia0->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	// PA - parallel printer data outputs.
	// PB - various config inputs.
	// CA1 - parallel printer, 'busy' input.
	// CA2 - parallel printer, 'data ready' output.
	// CB1 - Handshake input?
	// CB2 - Handshake output?
	PIA6821(config, m_pia1);
	m_pia1->writepa_handler().set(FUNC(swtpc8212_device::pia1_pa_w));
	m_pia1->readca1_handler().set(FUNC(swtpc8212_device::pia1_ca1_r));
	m_pia1->ca2_handler().set(FUNC(swtpc8212_device::pia1_ca2_w));
	m_pia1->readpb_handler().set_ioport("DIP_SWITCHES");

	INS8250(config, m_uart, 1.8432_MHz_XTAL);
	m_uart->out_tx_callback().set(FUNC(swtpc8212_device::write_txd));
	m_uart->out_dtr_callback().set(FUNC(swtpc8212_device::write_dtr));
	m_uart->out_rts_callback().set(FUNC(swtpc8212_device::write_rts));
	m_uart->out_int_callback().set("mainirq", FUNC(input_merger_device::in_w<2>));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(17.0748_MHz_XTAL, 918, 0, 738, 310, 0, 280);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, 17.0748_MHz_XTAL / 9);
	m_crtc->set_char_width(9);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_update_row_callback(FUNC(swtpc8212_device::update_row));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(swtpc8212_device::keyboard_put));

	SPEAKER(config, "bell").front_center();
	BEEP(config, m_beeper, 2000);
	m_beeper->add_route(ALL_OUTPUTS, "bell", 0.25);

	PRINTER(config, m_printer, 0);
}

ioport_constructor swtpc8212_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(swtpc8212);
}

const tiny_rom_entry *swtpc8212_device::device_rom_region() const
{
	return ROM_NAME(swtpc8212);
}

DEFINE_DEVICE_TYPE(SWTPC8212, swtpc8212_device, "swtpc8212_device", "SWTPC8212")
