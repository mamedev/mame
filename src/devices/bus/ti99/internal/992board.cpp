// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    TI-99/2 main board custom circuits

    This component implements the custom video controller and interface chip
    from the TI-99/2 console.

    May 2018

***************************************************************************/

#include "emu.h"

#define LOG_WARN        (1U<<1)   // Warnings
#define LOG_CRU         (1U<<2)     // CRU logging
#define LOG_CASSETTE    (1U<<3)     // Cassette logging
#define LOG_HEXBUS      (1U<<4)     // Hexbus logging
#define LOG_BANK        (1U<<5)     // Change ROM banks
#define LOG_KEYBOARD    (1U<<6)   // Keyboard operation

#define VERBOSE ( LOG_WARN )

#include "logmacro.h"
#include "992board.h"

/*
    Emulation of the CRT Gate Array of the TI-99/2

    Video display controller

    RF-modulated, composite output
    for standard black/white television
    Selectable channel 3 or 4 VHF

    625 lines for US markets
    525 lines for European markets

    Display: 24 rows, 32 columns

    The controller accesses ROM and RAM space of the 9995 CPU. It makes use
    of the HOLD line to gain access. Thus, the controller has DMA control
    while producing each scan line. The CPU has a chance to execute instructions
    in border time, horizontal retrace, vertical retrace.

    In order to get more computing time, a special character (BEOL - bank end
    of line) is used to indicate the last drawable character on the line. After
    this character, the buses are released.

    24K version: BEOL = any character from 0x70 to 0xff
    32K version: BEOL = any character from 0x80 to 0xff

    CRU Bit VIDENA: disables the scan line generation; blank white screen

    Clock: 10.7 MHz

    Scanline refresh:
    - Pull down HOLD
    - Wait for a short time (some dots)
    - Use row, column, dot_line counters
      - Get the value c at 0xEC00 + row*32+col
      - Get the byte b from 0x1C00 + c*8 + (dot_line%8)
      - Push the byte to the shift register
      - Get the bits for the scanline from the register

    EF00: Control byte
         +--+--+--+--+--+--+--+--+
         |- |- |- |- |- |T |B |S |
         +--+--+--+--+--+--+--+--+

         Fabrice's 99/2:
         T: Text color (1=white)
         B: border color (1=white)
         S: Background color (1=text color, 0=inverted)

                 text border back
         0 0 0    b     b     w
         0 0 1    b     b     b
         0 1 0    b     w     w
         0 1 1    b     w     b
         1 0 0    w     b     b
         1 0 1    w     b     w
         1 1 0    w     w     b
         1 1 1    w     w     w

   Counters:
     dotline 9 bit
        after reaching 261, resets to 0

        224..236: Top blanking
        237..261: Top border
        000..191: Display
        192..217: Bottom border
        218..220: Bottom blanking
        221..223: Vert sync

     dotcolumn 9 bit
        increments every clock tick until reaching 341, resets to 0, and incr dotline

        305..328: Left blanking
        329..341: Left border
        000..255: Display
        256..270: Right border
        271..278: Right blanking
        279..304: Hor sync
   ------------------------------

   Later versions define a "bitmap mode". [2]
   There are no known consoles with this capability, and it would require at
   least 6 KiB of RAM.

   EF00: Control byte
         +--+--+--+--+--+--+--+--+
         |- |- |- |- |- |C |B |M |
         +--+--+--+--+--+--+--+--+

         C: character color (1=white)
         B: border color (1=white)
         M: bitmap mode (1=bitmap)

   [1] Ground Squirrel Personal Computer Product Specification
   [2] VDC Controller CF40052
*/

DEFINE_DEVICE_TYPE_NS(VIDEO99224, bus::ti99::internal, video992_24_device, "video992_24", "TI-99/2 CRT Controller 24K version")
DEFINE_DEVICE_TYPE_NS(VIDEO99232, bus::ti99::internal, video992_32_device, "video992_32", "TI-99/2 CRT Controller 32K version")
DEFINE_DEVICE_TYPE_NS(IO99224, bus::ti99::internal, io992_24_device, "io992_24", "TI-99/2 I/O controller 24K version")
DEFINE_DEVICE_TYPE_NS(IO99232, bus::ti99::internal, io992_32_device, "io992_32", "TI-99/2 I/O controller 32K version")


namespace bus { namespace ti99 { namespace internal {

video992_device::video992_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	  device_video_interface(mconfig, *this),
	  m_mem_read_cb(*this),
	  m_hold_cb(*this),
	  m_int_cb(*this),
	  m_videna(true)
{
}

video992_24_device::video992_24_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: video992_device(mconfig, VIDEO99224, tag, owner, clock)
{
	m_beol = 0x70;
}

video992_32_device::video992_32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: video992_device(mconfig, VIDEO99232, tag, owner, clock)
{
	m_beol = 0x7f;
}

std::string video992_device::tts(attotime t)
{
	char buf[256];
	const char *sign = "";
	if(t.seconds() < 0) {
		t = attotime::zero-t;
		sign = "-";
	}
	int nsec = t.attoseconds() / ATTOSECONDS_PER_NANOSECOND;
	sprintf(buf, "%s%04d.%03d,%03d,%03d", sign, int(t.seconds()), nsec/1000000, (nsec/1000)%1000, nsec % 1000);
	return buf;
}


void video992_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int raw_vpos = screen().vpos();

	if (id == HOLD_TIME)
	{
		// logerror("release time: %s, diff: %s\n", tts(machine().time()), tts(machine().time()-m_hold_time));
		// We're holding the CPU; release it until the next start
		m_hold_cb(CLEAR_LINE);
		m_free_timer->adjust(screen().time_until_pos((raw_vpos+1) % screen().height(), HORZ_DISPLAY_START));
		return;
	}

	// logerror("hold time: %s\n", tts(machine().time()));
	if (m_videna)
	{
		// Hold the CPU
		// We should expect the HOLDA (HOLD acknowledge) line to go to the
		// same circuit that issued the HOLD, but this is not the case.
		// The HOLDA line goes to the I/O circuit, which (obviously) allows
		// access to the RAM at that point. There is no
		// indication of any other effect on the video controller.
		m_hold_time = machine().time();
		m_hold_cb(ASSERT_LINE);
	}

	int vpos = raw_vpos * m_vertical_size / screen().height();
	uint32_t *p = &m_tmpbmp.pix32(vpos);
	bool endofline = false;

	int linelength = 0;

	// logerror("draw line %d\n", vpos);
	// Get control byte
	uint8_t control = m_mem_read_cb(0xef00);
	bool text_white = ((control & 0x04)!=0);
	bool border_white = ((control & 0x02)!=0);
	bool background_white = ((control & 0x01)!=0)? text_white : !text_white;

	int y = vpos - m_top_border;
	if (y < 0 || y >= 192)
	{
		// Draw border colour
		for (int i = 0; i < TOTAL_HORZ; i++)
			p[i] = border_white? rgb_t::white() : rgb_t::black();

		// vblank is set at the last cycle of the first inactive line
		// not confirmed by the specs, just doing like 9928A.
		if ( y == 193 )
		{
			m_int_cb( ASSERT_LINE );
			m_int_cb( CLEAR_LINE );
		}
	}
	else
	{
		// Draw regular line
		// Left border
		for (int i = 0; i < HORZ_DISPLAY_START; i++)
			p[i] = border_white? rgb_t::white() : rgb_t::black();

		int addr = ((y << 2) & 0x3e0) | 0xec00;

		// Active display
		for (int x = HORZ_DISPLAY_START; x<HORZ_DISPLAY_START+256; x+=8)
		{
			uint8_t charcode = 0;
			uint8_t pattern = 0;
			if (!endofline && m_videna)
			{
				// Get character code at the location
				charcode = m_mem_read_cb(addr) & 0x7f;

				// Is it the BEOL (blank end-of-line)?
				if (charcode >= m_beol)
					endofline = true;
			}
			if (!endofline && m_videna)
			{
				// Get the pattern
				int addrp = 0x1c00 | (charcode << 3) | (y%8);
				pattern = m_mem_read_cb(addrp);
				linelength++;
			}
			for (int i = 0; i < 8; i++)
			{
				if ((pattern & 0x80)!=0)
					p[x+i] = text_white? rgb_t::white() : rgb_t::black();
				else
					p[x+i] = background_white? rgb_t::white() : rgb_t::black();

				pattern <<= 1;
			}
			addr++;
		}

		// Right border
		for (int i = HORZ_DISPLAY_START + 256; i < TOTAL_HORZ; i++)
			p[i] = border_white? rgb_t::white() : rgb_t::black();
	}

	// +1 for the minimum hold time
	// logerror("line length: %d\n", linelength);
	m_hold_timer->adjust(screen().time_until_pos(raw_vpos, HORZ_DISPLAY_START + linelength*8 + 1));
}


uint32_t video992_device::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	copybitmap( bitmap, m_tmpbmp, 0, 0, 0, 0, cliprect );
	return 0;
}

/*
    VIDENA pin, positive logic
*/
WRITE_LINE_MEMBER( video992_device::videna )
{
	m_videna = (state==ASSERT_LINE);
}

void video992_device::device_start()
{
	m_top_border = VERT_DISPLAY_START_NTSC;
	m_vertical_size = TOTAL_VERT_NTSC;
	m_tmpbmp.allocate(TOTAL_HORZ, TOTAL_VERT_NTSC);

	m_hold_timer = timer_alloc(HOLD_TIME);
	m_free_timer = timer_alloc(FREE_TIME);

	m_border_color = rgb_t::black();
	m_background_color = rgb_t::white();
	m_text_color = rgb_t::black();

	m_mem_read_cb.resolve();
	m_hold_cb.resolve();
	m_int_cb.resolve();
}

void video992_device::device_reset()
{
	m_free_timer->adjust(screen().time_until_pos(0, HORZ_DISPLAY_START));
}

/*
    Emulation of the I/O Gate Array of the TI-99/2 [3]

    The I/O controller is a TAL004 Low Power Schottky-TTL bipolar Gate Array
    that provides the interface between the CPU, the keyboard, the Hexbus,
    and the cassette interface. It delivers memory select lines for use by the
    CPU and by the VDC. It also offers a synchronized RESET signal and a
    divider for the CPU clock (which is seemingly not used in the real
    machines).

    It is mapped into the CRU I/O address space at addresses E000 and E800:

    I/O map (CRU map)
    -----------------
    0000 - 1DFE: unmapped
    1E00 - 1EFE: TMS9995-internal CRU addresses
    1F00 - 1FD8: unmapped
    1FDA:        TMS9995 MID flag
    1FDC - 1FFF: unmapped
    2000 - DFFE: unmapped
    E000 - E00E: Read: Keyboard column input
    E000 - E00A: Write: Keyboard row selection
    E00C:        Write: unmapped
    E00E:        Write: Video enable (VIDENA)
    E010 - E7FE: Mirrors of the above
    E800 - E80C: Hexbus
       E800 - E806: Data lines
       E808: HSK line
       E80A: BAV line
       E80C: Inhibit (Write only)
    E80E: Cassette

    [3] I/O Controller CF40051, Preliminary specification, Texas Instruments
*/

io992_device::io992_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: bus::hexbus::hexbus_chained_device(mconfig, type, tag, owner, clock),
		m_hexbus(*this, "^" TI_HEXBUS_TAG),
		m_cassette(*this, "^" TI_CASSETTE),
		m_videoctrl(*this, "^" TI992_VDC_TAG),
		m_keyboard(*this, "LINE%u", 0U),
		m_set_rom_bank(*this),
		m_key_row(0),
		m_latch_out(0xd7),
		m_latch_in(0xd7),
		m_communication_disable(true),
		m_response_phase(false)
{
}

io992_24_device::io992_24_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: io992_device(mconfig, IO99224, tag, owner, clock)
{
	m_have_banked_rom = false;
}

io992_32_device::io992_32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: io992_device(mconfig, IO99232, tag, owner, clock)
{
	m_have_banked_rom = true;
}

/*
    54-key keyboard
*/
static INPUT_PORTS_START( keys992 )

	PORT_START("LINE0")    /* col 0 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 ! DEL") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 @ INS") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $ CLEAR") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 % BEGIN") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 ^ PROC'D") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 & AID") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 * REDO") PORT_CODE(KEYCODE_8)

	PORT_START("LINE1")    /* col 1 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("q Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("w W ~") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("e E (UP)") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("r R [") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t T ]") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("y Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("i I ?") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 ( BACK") PORT_CODE(KEYCODE_9)

	PORT_START("LINE2")    /* col 2 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("a A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("s S (LEFT)") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("d D (RIGHT)") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f F {") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("h H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("u U _") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("o O '") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0)

	PORT_START("LINE3")    /* col 3 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("z Z \\") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("x X (DOWN)") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c C `") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("g G }") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("j J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("p P \"") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= + QUIT") PORT_CODE(KEYCODE_EQUALS)

	PORT_START("LINE4")    /* col 4 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("v V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("n N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("l L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ -") PORT_CODE(KEYCODE_SLASH)

	PORT_START("LINE5")    /* col 5 */
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(SPACE)") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("m M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FCTN") PORT_CODE(KEYCODE_LALT)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)

	PORT_START("LINE6")    /* col 6 */
		PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE7")    /* col 7 */
		PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNUSED)

INPUT_PORTS_END

void io992_device::device_start()
{
	set_outbound_hexbus(m_hexbus.target());

	// Establish callback for inbound propagations
	m_hexbus_outbound->set_chain_element(this);

	m_set_rom_bank.resolve();
}

uint8_t io992_device::cruread(offs_t offset)
{
	int address = offset << 1;
	uint8_t value = 0x7f;  // All Hexbus lines high
	double inp = 0;
	int i;
	uint8_t bit = 1;

	switch (address & 0xf800)
	{
	case 0xe000:
		// CRU E000-E7fE: Keyboard
		//   Read: 0000 1110 0*** **** (mirror 007f)
		value = m_keyboard[m_key_row]->read();
		break;
	case 0xe800:
		// CRU E800-EFFE: Hexbus and other functions
		//  Read: 0000 1110 1*** **** (mirror 007f)
		for (i=0; i < 6; i++)
		{
			if ((m_latch_in & m_hexbval[i])==0) value &= ~bit;
			bit <<= 1;
		}
		// e80c (bit 6) seems to be a latch for the response phase
		if (!m_response_phase)
			value &= ~0x40;

		inp = m_cassette->input();
		if (inp > 0)
			value |= 0x80;
		LOGMASKED(LOG_CASSETTE, "value=%f\n", inp);
		break;
	default:
		LOGMASKED(LOG_CRU, "Unknown access to %04x\n", address);
	}

	LOGMASKED(LOG_CRU, "CRU %04x ->  %02x\n", address, value);

	return BIT(value, offset & 7);
}

void io992_device::cruwrite(offs_t offset, uint8_t data)
{
	int address = (offset << 1) & 0xf80e;

	LOGMASKED(LOG_CRU, "CRU %04x <- %1x\n", address, data);

	uint8_t olddata = m_latch_out;

	switch (address)
	{
	// Select the current keyboard row. Also, bit 0 is used to switch the
	// ROM bank. I guess that means we won't be able to read the keyboard
	// when processing that particular ROM area.
	// CRU E000-E7fE: Keyboard
	//   Write: 1110 0*** **** XXX0 (mirror 07f0)
	case 0xe000:
		if (m_have_banked_rom)
		{
			LOGMASKED(LOG_BANK, "set bank = %d\n", data);
			m_set_rom_bank(data==1);
		}
		// no break
	case 0xe002:
	case 0xe004:
	case 0xe006:
	case 0xe008:
	case 0xe00a:
		if (data == 0) m_key_row = offset&7;
		break;
	case 0xe00c:
		LOGMASKED(LOG_WARN, "Unmapped CRU write to address e00c\n");
		break;
	case 0xe00e:
		LOGMASKED(LOG_CRU, "VIDENA = %d\n", data);
		m_videoctrl->videna(data);
		break;

	//  Write: 1110 1*** **** XXX0 (mirror 07f0)
	case 0xe800:  // ID0
	case 0xe802:  // ID1
	case 0xe804:  // ID2
	case 0xe806:  // ID3
		if (data != 0) m_latch_out |= m_hexbval[offset & 0x07];
		else m_latch_out &= ~m_hexbval[offset & 0x07];
		LOGMASKED(LOG_HEXBUS, "Hexbus latch out = %02x\n", m_latch_out);
		break;
	case 0xe80a:  // BAV
		// Undocumented, but makes sense according to the ROM
		if (data==0)
			m_response_phase = false;
		// no break
	case 0xe808:  // HSK
		if (data != 0) m_latch_out |= m_hexbval[offset & 0x07];
		else m_latch_out &= ~m_hexbval[offset & 0x07];

		if (m_latch_out != olddata)
		{
			LOGMASKED(LOG_HEXBUS, "%s %s\n", (address==0xe808)? "HSK*" : "BAV*", (data==0)? "assert" : "clear");
			if (m_communication_disable)
			{
				// This is not explicitly stated in the specs, but since they
				// also claim that communication is disabled on power-up,
				// we must turn it on here; the ROM fails to do it.
				LOGMASKED(LOG_HEXBUS, "Enabling Hexbus\n");
				m_communication_disable = false;
			}
			LOGMASKED(LOG_HEXBUS, "Writing to Hexbus: BAV*=%d, HSK*=%d, DATA=%01x\n", (m_latch_out&0x04)!=0, (m_latch_out&0x10)!=0, ((m_latch_out>>4)&0x0c)|(m_latch_out&0x03));
			hexbus_write(m_latch_out);
			// Check how the bus has changed. This depends on the states of all
			// connected peripherals
			m_latch_in = hexbus_read();
		}
		break;
	case 0xe80c:
		LOGMASKED(LOG_HEXBUS, "Hexbus inhibit = %d\n", data);
		if (data == 1)
		{
			m_latch_in = 0xd7;
			m_communication_disable = true;
		}
		else m_communication_disable = false;
		break;
	case 0xe80e:
		LOGMASKED(LOG_CRU, "Cassette output = %d\n", data);
		// Tape output. See also ti99_4x.cpp.
		m_cassette->output((data==1)? +1 : -1);
		break;
	}
}

/*
    Input from the Hexbus. Since it cannot trigger any interrupt on the
    CPU, it must be polled via CRU.

            Line state received via the Hexbus
    +------+------+------+------+------+------+------+------+
    | ID3  | ID2  |  -   | HSK* |  0   | BAV* | ID1  | ID0  |
    +------+------+------+------+------+------+------+------+

*/
void io992_device::hexbus_value_changed(uint8_t data)
{
	// Only latch the incoming data when BAV* is asserted and the Hexbus
	// is not inhibited
	bool bav_asserted = ((m_latch_out & bus::hexbus::HEXBUS_LINE_BAV)==0);
	if (!m_communication_disable && bav_asserted)
	{
		LOGMASKED(LOG_HEXBUS, "Hexbus changed and latched: %02x\n", data);
		m_latch_in = data;

		if ((data & bus::hexbus::HEXBUS_LINE_HSK)==0)
		{
			// If HSK* is lowered and we as the host have it up, this indicates
			// the response phase.
			if (!m_response_phase && (m_myvalue & bus::hexbus::HEXBUS_LINE_HSK)!=0)
			{
				LOGMASKED(LOG_HEXBUS, "Incoming response\n");
				m_response_phase = true;
			}
			// We cannot wait for the CPU explicitly latching HSK*
			// as designed, because this implies a true parallel execution
			LOGMASKED(LOG_HEXBUS, "Latching HSK*\n");
			m_myvalue &= ~bus::hexbus::HEXBUS_LINE_HSK;
		}
	}
	else
		LOGMASKED(LOG_HEXBUS, "Ignoring Hexbus change (to %02x), latch=%s, BAV*=%d\n", data, m_communication_disable? "inhibit":"enabled", bav_asserted? 0:1);
}

ioport_constructor io992_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keys992 );
}

}   }   }


