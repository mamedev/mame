// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Tandy 2000

****************************************************************************/

/*

    TODO:

    - floppy
        - HDL is also connected to WP/TS input where TS is used to detect motor status
        - 3 second motor off delay timer
    - video (video RAM is at memory top - 0x1400, i.e. 0x1ec00)
    - keyboard ROM, same as earlier tandy 1000
    - WD1010
    - hard disk
    - clock/mouse 8042 mcu ROM, probably same as tandy 1000 isa clock/mouse adapter
    - sab3019 rtc

*/

#include "emu.h"
#include "tandy2k.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"


#define LOG 1

// Read/Write Handlers

void tandy2k_state::update_drq()
{
	int drq0 = CLEAR_LINE;
	int drq1 = CLEAR_LINE;

	for (int i = 0; i < 4; i++)
	{
		if (BIT(m_dma_mux, 0 + i))
		{
			if (BIT(m_dma_mux, 4 + i))
				drq1 |= m_busdmarq[i];
			else
				drq0 |= m_busdmarq[i];
		}
	}

	m_maincpu->drq0_w(drq0);
	m_maincpu->drq1_w(drq1);
}

void tandy2k_state::dma_request(int line, int state)
{
	m_busdmarq[line] = state;

	update_drq();
}

void tandy2k_state::speaker_update()
{
	int level = !(m_spkrdata & m_outspkr);

	m_speaker->level_w(level);
}

uint8_t tandy2k_state::char_ram_r(offs_t offset)
{
	return m_char_ram[offset];
}

void tandy2k_state::char_ram_w(offs_t offset, uint8_t data)
{
	m_char_ram[offset] = data;
}

uint8_t tandy2k_state::videoram_r(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	offs_t addr = (m_vram_base << 15) | (offset << 1);
	uint16_t data = program.read_word(addr);

	// character
	m_drb0->write(data & 0xff);

	// attributes
	m_drb1->write(data >> 8);

	return data & 0xff;
}

uint8_t tandy2k_state::enable_r()
{
	/*

	    bit     signal      description

	    0                   RS-232 ring indicator
	    1                   RS-232 carrier detect
	    2
	    3
	    4
	    5
	    6
	    7       _ACLOW

	*/

	uint8_t data = 0x80;

	data |= m_rs232->ri_r();
	data |= m_rs232->dcd_r() << 1;

	return data;
}

void tandy2k_state::enable_w(uint8_t data)
{
	/*

	    bit     signal      description

	    0       KBEN        keyboard enable
	    1       EXTCLK      external baud rate clock
	    2       SPKRGATE    enable periodic speaker output
	    3       SPKRDATA    direct output to speaker
	    4       RFRQGATE    enable refresh and baud rate clocks
	    5       FDCRESET*   reset 8272
	    6       TMRIN0      enable 80186 timer 0
	    7       TMRIN1      enable 80186 timer 1

	*/

	if (LOG) logerror("ENABLE %02x\n", data);

	// keyboard enable
	m_kb->power_w(BIT(data, 0));
	m_pc_keyboard->enable(BIT(data, 0));

	// external baud rate clock
	m_extclk = BIT(data, 1);

	// speaker gate
	m_pit->write_gate0(BIT(data, 2));

	// speaker data
	m_spkrdata = BIT(data, 3);
	speaker_update();

	// refresh and baud rate clocks
	m_pit->write_gate1(BIT(data, 4));
	m_pit->write_gate2(BIT(data, 4));

	// FDC reset
	m_fdc->reset_w(!BIT(data, 5));

	// timer 0 enable
	m_maincpu->tmrin0_w(BIT(data, 6));

	// timer 1 enable
	m_maincpu->tmrin1_w(BIT(data, 7));
}

void tandy2k_state::dma_mux_w(uint8_t data)
{
	/*

	    bit     description

	    0       DMA channel 0 enable
	    1       DMA channel 1 enable
	    2       DMA channel 2 enable
	    3       DMA channel 3 enable
	    4       DMA channel 0 select
	    5       DMA channel 1 select
	    6       DMA channel 2 select
	    7       DMA channel 3 select

	*/

	if (LOG) logerror("DMA MUX %02x\n", data);

	m_dma_mux = data;

	// check for DMA error
	int drq0 = 0;
	int drq1 = 0;

	for (int ch = 0; ch < 4; ch++)
	{
		if (BIT(data, ch)) { if (BIT(data, ch + 4)) drq1++; else drq0++; }
	}

	int dme = (drq0 > 2) || (drq1 > 2);

	m_pic1->ir6_w(dme);

	update_drq();
}

uint8_t tandy2k_state::kbint_clr_r()
{
	if (m_pb_sel == KBDINEN)
	{
		m_kb->busy_w(1);
		m_pic1->ir0_w(CLEAR_LINE);

		return m_pc_keyboard->read();
	}

	return 0xff;
}

uint8_t tandy2k_state::clkmouse_r(offs_t offset)
{
	uint8_t ret = 0;
	switch (offset)
	{
		case 0:
			if (!m_clkmouse_cnt)
				return 0;
			ret = m_clkmouse_cmd[--m_clkmouse_cnt];
			m_pic1->ir2_w(0);
			if (m_clkmouse_cnt > 0)
				m_mcu_delay->adjust(attotime::from_msec(1));
			break;
		case 2:
			ret = m_buttons->read();
			if (m_clkmouse_cnt)
				ret |= 1;
			break;
	}
	return ret;
}

void tandy2k_state::clkmouse_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_pic1->ir2_w(0);
			if (m_clkmouse_cnt < 8)
				m_clkmouse_cmd[m_clkmouse_cnt++] = data;
			break;
		case 1:
			break;
		case 2:
			if (m_clkmouse_cnt < 8)
				m_clkmouse_cmd[m_clkmouse_cnt++] = data;
			switch (m_clkmouse_cmd[0])
			{
				case 0x01: //set time
					break;
				case 0x02: //read time
					break;
				case 0x08:
					if(m_clkmouse_cmd[1] > 0)
						m_clkmouse_irq |= MO_IRQ;
					else
						m_clkmouse_irq &= ~MO_IRQ;
					if(m_clkmouse_cmd[2] > 0)
						m_clkmouse_irq |= BT_IRQ;
					else
						m_clkmouse_irq &= ~BT_IRQ;
					break;
				case 0x20:
					if(m_clkmouse_cmd[1] > 0)
						m_mouse_timer->adjust(attotime::from_hz(40), 0, attotime::from_hz(40));
					else
						m_mouse_timer->adjust(attotime::never);
					break;
			}
			m_clkmouse_cnt = 0;
			break;
		case 3:
			m_pic1->ir2_w(0);
			m_clkmouse_cnt = 0;
			m_clkmouse_irq = 0;
			m_mouse_x = m_x_axis->read();
			m_mouse_y = m_y_axis->read();
			break;
	}
}

uint8_t tandy2k_state::fldtc_r()
{
	if (LOG) logerror("FLDTC\n");

	fldtc_w(0);

	return 0;
}

void tandy2k_state::fldtc_w(uint8_t data)
{
	m_fdc->tc_w(1);
	m_fdc->tc_w(false);
}

void tandy2k_state::addr_ctrl_w(uint8_t data)
{
	/*

	    bit     signal      description

	    8       A15         A15 of video access
	    9       A16         A16 of video access
	    10      A17         A17 of video access
	    11      A18         A18 of video access
	    12      A19         A19 of video access
	    13      CLKSP0      clock speed (0 = 22.4 MHz, 1 = 28 MHz)
	    14      CLKCNT      dots/char (0 = 10 [800x400], 1 = 8 [640x400])
	    15      VIDOUTS     selects the video source for display on monochrome monitor

	*/

	if (LOG) logerror("Address Control %02x\n", data);

	// video access
	m_vram_base = data & 0x1f;

	// video clock speed
	int clkspd = BIT(data, 5);
	int clkcnt = BIT(data, 6);

	if (m_clkspd != clkspd || m_clkcnt != clkcnt)
	{
		const XTAL busdotclk = 16_MHz_XTAL * 28 / (clkspd ? 16 : 20);
		const XTAL vidcclk = busdotclk / (clkcnt ? 8 : 10);

		m_vpac->set_character_width(clkcnt ? 8 : 10);
		m_vpac->set_unscaled_clock(vidcclk);

		m_vac->set_unscaled_clock(busdotclk);

		m_timer_vidldsh->adjust(attotime::from_hz(vidcclk), 0, attotime::from_hz(vidcclk));

		m_clkspd = clkspd;
		m_clkcnt = clkcnt;
	}

	// video source select
	m_vidouts = BIT(data, 7);
}

// Memory Maps

void tandy2k_state::vrambank_mem(address_map &map)
{
	map(0x00000, 0x17fff).ram().share("hires_ram");
	map(0x18000, 0x1ffff).noprw();
}

void tandy2k_state::tandy2k_mem(address_map &map)
{
	map.unmap_value_high();
	map(0xe0000, 0xe7fff).rw(m_vrambank, FUNC(address_map_bank_device::read16), FUNC(address_map_bank_device::write16));
	map(0xf8000, 0xfbfff).rw(FUNC(tandy2k_state::char_ram_r), FUNC(tandy2k_state::char_ram_w)).umask16(0x00ff);
	map(0xfc000, 0xfdfff).mirror(0x2000).rom().region(I80186_TAG, 0);
}

void tandy2k_state::tandy2k_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x00000).mirror(0x8).rw(FUNC(tandy2k_state::enable_r), FUNC(tandy2k_state::enable_w));
	map(0x00002, 0x00002).mirror(0x8).w(FUNC(tandy2k_state::dma_mux_w));
	map(0x00004, 0x00004).mirror(0x8).rw(FUNC(tandy2k_state::fldtc_r), FUNC(tandy2k_state::fldtc_w));
	map(0x00010, 0x00013).mirror(0xc).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x00030, 0x00033).mirror(0xc).m(m_fdc, FUNC(i8272a_device::map)).umask16(0x00ff);
	map(0x00040, 0x00047).mirror(0x8).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x00050, 0x00057).mirror(0x8).rw(m_i8255a, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x00052, 0x00052).mirror(0x8).r(FUNC(tandy2k_state::kbint_clr_r));
	map(0x00060, 0x00063).mirror(0xc).rw(m_pic0, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x00070, 0x00073).mirror(0xc).rw(m_pic1, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x00080, 0x00080).mirror(0xe).rw(m_fdc, FUNC(i8272a_device::dma_r), FUNC(i8272a_device::dma_w));
	map(0x00100, 0x0017f).rw(m_vpac, FUNC(crt9007_device::read), FUNC(crt9007_device::write)).umask16(0x00ff);
	map(0x00100, 0x0017f).w(FUNC(tandy2k_state::addr_ctrl_w)).umask16(0xff00);
	map(0x00180, 0x00180).r(FUNC(tandy2k_state::hires_status_r)).umask16(0x00ff);
	map(0x00180, 0x0018f).mirror(0x10).w(m_colpal, FUNC(palette_device::write8)).umask16(0x00ff).share("colpal");
	map(0x001a0, 0x001a0).w(FUNC(tandy2k_state::hires_plane_w)).umask16(0x00ff);
	map(0x002fc, 0x002ff).rw(FUNC(tandy2k_state::clkmouse_r), FUNC(tandy2k_state::clkmouse_w));
}

void tandy2k_state::tandy2k_hd_io(address_map &map)
{
	tandy2k_io(map);
//  map(0x000e0, 0x000ff).w(FUNC(tandy2k_state::hdc_dack_w).umask16(0x00ff));
//  map(0x0026c, 0x0026c).rw(WD1010_TAG, FUNC(wd1010_device::hdc_reset_r), FUNC(wd1010_device::hdc_reset_w));
//  map(0x0026e, 0x0027e).rw(WD1010_TAG, FUNC(wd1010_device::wd1010_r), FUNC(wd1010_device::wd1010_w));
}

void tandy2k_state::vpac_mem(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(tandy2k_state::videoram_r));
}

// Input Ports

static INPUT_PORTS_START( tandy2k )
	// defined in machine/tandy2kb.c
	PORT_START("MOUSEBTN")
	PORT_BIT( 0xff8f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, tandy2k_state, input_changed, 1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, tandy2k_state, input_changed, 1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )  /* this would be button three but AFAIK no tandy mouse ever had one */

	PORT_START("MOUSEX")
	PORT_BIT( 0xffff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, tandy2k_state, input_changed, 0)

	PORT_START("MOUSEY")
	PORT_BIT( 0xffff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, tandy2k_state, input_changed, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( tandy2kb_hle )
	PORT_INCLUDE(pc_keyboard)

	PORT_MODIFY("pc_keyboard_2")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Up") PORT_CODE(KEYCODE_UP) /*                             29  A9 */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Left") PORT_CODE(KEYCODE_LEFT) /*                             2B  AB */

	PORT_MODIFY("pc_keyboard_3")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE   /* Caps Lock                   3A  BA */

	PORT_MODIFY("pc_keyboard_4")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NumLock") PORT_CODE(KEYCODE_NUMLOCK) PORT_TOGGLE /* Num Lock                    45  C5 */
	/* Hold corresponds to Scroll Lock, but pauses the system when pressed - leaving unmapped by default to avoid conflicting with the UI Toggle key */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Hold")     /*                            46  C6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 7 \\") PORT_CODE(KEYCODE_7_PAD) /* Keypad 7                    47  C7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 8 ~") PORT_CODE(KEYCODE_8_PAD) /* Keypad 8                    48  C8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 9 (PgUp)") PORT_CODE(KEYCODE_9_PAD) /* Keypad 9  (PgUp)            49  C9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Down") PORT_CODE(KEYCODE_DOWN) /*                             4A  CA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 4 |") PORT_CODE(KEYCODE_4_PAD) /* Keypad 4                    4B  CB */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 6") PORT_CODE(KEYCODE_6_PAD) /* Keypad 6                    4D  CD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Right") PORT_CODE(KEYCODE_RIGHT) /*                             4E  CE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 1 (End)") PORT_CODE(KEYCODE_1_PAD) /* Keypad 1  (End)             4F  CF */

	PORT_MODIFY("pc_keyboard_5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 2 `") PORT_CODE(KEYCODE_2_PAD) /* Keypad 2                    50  D0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 3 (PgDn)") PORT_CODE(KEYCODE_3_PAD) /* Keypad 3  (PgDn)            51  D1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 0") PORT_CODE(KEYCODE_0_PAD) /* Keypad 0                    52  D2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP - (Del)") PORT_CODE(KEYCODE_MINUS_PAD) /* - Delete                    53  D3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_STOP) /* Break                       54  D4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ Insert") PORT_CODE(KEYCODE_PLUS_PAD) /* + Insert                    55  D5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_DEL_PAD) /* .                           56  D6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER_PAD) /* Enter                       57  D7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME) /* HOME                        58  D8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F11") PORT_CODE(KEYCODE_F11) /* F11                         59  D9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F12") PORT_CODE(KEYCODE_F12) /* F12                         5a  Da */
INPUT_PORTS_END

class tandy2kb_hle_device : public pc_keyboard_device
{
public:
	tandy2kb_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

DEFINE_DEVICE_TYPE(TANDY2K_HLE_KEYB, tandy2kb_hle_device, "tandy2kb_hle", "Tandy 2000 Keyboard HLE")

tandy2kb_hle_device::tandy2kb_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pc_keyboard_device(mconfig, TANDY2K_HLE_KEYB, tag, owner, clock)
{
	m_type = KEYBOARD_TYPE::PC;
}

ioport_constructor tandy2kb_hle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tandy2kb_hle);
}

INPUT_CHANGED_MEMBER(tandy2k_state::input_changed)
{
	if (m_clkmouse_cnt || !m_clkmouse_irq)
		return;

	const bool is_button = bool(param);

	if ((m_clkmouse_irq & BT_IRQ) && is_button)
	{
		m_clkmouse_cnt = 1;
		m_clkmouse_cmd[0] = 'B';
	}
	else if ((m_clkmouse_irq & MO_IRQ))
	{
		uint16_t x = m_x_axis->read();
		uint16_t y = m_y_axis->read();
		uint16_t dx = x - m_mouse_x;
		uint16_t dy = y - m_mouse_y;
		m_mouse_x = x;
		m_mouse_y = y;
		m_clkmouse_cnt = 5;
		m_clkmouse_cmd[4] = 'M';
		m_clkmouse_cmd[3] = dx & 0xff;
		m_clkmouse_cmd[2] = dx >> 8;
		m_clkmouse_cmd[1] = dy & 0xff;
		m_clkmouse_cmd[0] = dy >> 8;
	}
	m_pic1->ir2_w(1);
}

// Video

uint32_t tandy2k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *cpen = m_colpal->pens();
	address_space &program = m_maincpu->space(AS_PROGRAM);

	for (int y = 0; y < 400; y++)
	{
		uint8_t cgra = y % 16;

		for (int sx = 0; sx < 80; sx++)
		{
			if (m_hires_en & 2)
			{
				uint8_t a = ((uint8_t *)m_hires_ram.target())[(y * 80) + sx];
				uint8_t b = ((uint8_t *)m_hires_ram.target())[(y * 80) + sx + 0x8000];
				uint8_t c = ((uint8_t *)m_hires_ram.target())[(y * 80) + sx + 0x8000 * 2];
				for (int x = 0; x < 8; x++)
				{
					int color = BIT(a, x) | (BIT(b, x) << 1) | (BIT(c, x) << 2);
					bitmap.pix(y, (sx * 8) + (7 - x)) = cpen[color];
				}
			}
			else
			{
				offs_t addr = m_ram->size() - 0x1400 + (((y / 16) * 80) + sx) * 2;
				uint16_t vidla = program.read_word(addr);
				uint8_t attr = vidla >> 8;
				uint8_t data = m_char_ram[((vidla & 0xff) << 4) | cgra];
				if(attr & 0x80)
					data = ~data;

				for (int x = 0; x < 8; x++)
				{
					int color = 4 | (BIT(attr, 6) << 1) | BIT(data, 7);
					bitmap.pix(y, (sx * 8) + x) = cpen[color];
					data <<= 1;
				}
			}
		}
	}

	return 0;
}

void tandy2k_state::vpac_vlt_w(int state)
{
	m_drb0->ren_w(state);
	m_drb0->clrcnt_w(state);

	m_drb1->ren_w(state);
	m_drb1->clrcnt_w(state);
}

void tandy2k_state::vpac_drb_w(int state)
{
	m_drb0->tog_w(state);
	m_drb1->tog_w(state);
}

void tandy2k_state::vpac_wben_w(int state)
{
	m_drb0->wen1_w(state);
	m_drb1->wen1_w(state);
}

void tandy2k_state::vpac_cblank_w(int state)
{
	m_cblank = state;
}

void tandy2k_state::vpac_slg_w(int state)
{
	m_slg = state;

	m_vac->slg_w(state);
}

void tandy2k_state::vpac_sld_w(int state)
{
	m_sld = state;

	m_vac->sld_w(state);
}

void tandy2k_state::hires_plane_w(uint8_t data)
{
	int bank = 3;
	if (((data & 1) + ((data >> 1) & 1) + ((data >> 2) & 1)) == 1)
		bank = (data & 1) ? 0 : (data & 2) ? 1 : (data & 4) ? 2 : 0;
	m_vrambank->set_bank(bank);
	m_hires_en = (data >> 4) & 3;
}

// bit 0 - 0 = hires board installed
// bit 1 - 0 = 1 plane, 1 = 3 planes
// bit 2-4 - board rev
uint8_t tandy2k_state::hires_status_r()
{
	return 2;
}

void tandy2k_state::vidla_w(uint8_t data)
{
	m_vidla = data;
}

void tandy2k_state::drb_attr_w(uint8_t data)
{
	/*

	    bit     description

	    0       BLC -> DBLC (delayed 2 CCLKs)
	    1       BKC -> DBKC (delayed 2 CCLKs)
	    2       CHABL
	    3       MS0
	    4       MS1
	    5       BLINK
	    6       INT
	    7       REVID

	*/

	m_blc = BIT(data, 0);
	m_bkc = BIT(data, 1);
	m_vac->chabl_w(BIT(data, 2));
	m_vac->ms0_w(BIT(data, 3));
	m_vac->ms1_w(BIT(data, 4));
	m_vac->blink_w(BIT(data, 5));
	m_vac->intin_w(BIT(data, 6));
	m_vac->revid_w(BIT(data, 7));
}

CRT9021_DRAW_CHARACTER_MEMBER( tandy2k_state::vac_draw_character )
{
	const pen_t *pen = m_colpal->pens();

	for (int i = 0; i < 8; i++)
	{
		int color = BIT(video, 7 - i);

		bitmap.pix(y, x++) = pen[color];
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( tandy2k_state::vidldsh_tick )
{
	m_drb0->rclk_w(0);
	m_drb0->wclk_w(0);
	m_drb1->rclk_w(0);
	m_drb1->wclk_w(0);
	m_vac->ld_sh_w(0);

	// 1 busdotclk later
	m_vac->blc_w(BIT(m_dblc, 0));
	m_dblc >>= 1;
	m_dblc |= m_blc << 2;

	m_vac->bkc_w(BIT(m_dbkc, 0));
	m_dbkc >>= 1;
	m_dbkc |= m_bkc << 2;

	m_vac->retbl_w(BIT(m_dblank, 0));
	m_dblank >>= 1;
	m_dblank |= m_cblank << 2;

	if (!m_slg)
	{
		m_cgra >>= 1;
		m_cgra |= m_sld << 3;
	}

	uint8_t vidd = m_char_ram[(m_vidla << 4) | m_cgra];
	m_vac->write(vidd);

	m_drb0->rclk_w(1);
	m_drb0->wclk_w(1);
	m_drb1->rclk_w(1);
	m_drb1->wclk_w(1);
	m_vac->ld_sh_w(1);
}

// Intel 8251A Interface

void tandy2k_state::rxrdy_w(int state)
{
	m_rxrdy = state;
	m_pic0->ir2_w(m_rxrdy || m_txrdy);
}

void tandy2k_state::txrdy_w(int state)
{
	m_txrdy = state;
	m_pic0->ir2_w(m_rxrdy || m_txrdy);
}

// Intel 8253 Interface

void tandy2k_state::outspkr_w(int state)
{
	m_outspkr = state;
	speaker_update();
}

void tandy2k_state::intbrclk_w(int state)
{
	if (!m_extclk)
	{
		m_uart->write_txc(state);
		m_uart->write_rxc(state);
	}
}

void tandy2k_state::rfrqpulse_w(int state)
{
	// memory refresh counter up
}

// Intel 8255A Interface

void tandy2k_state::write_centronics_ack(int state)
{
	m_centronics_ack = state;
	m_i8255a->pc6_w(state);
}

void tandy2k_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

void tandy2k_state::write_centronics_perror(int state)
{
	m_centronics_perror = state;
}

void tandy2k_state::write_centronics_select(int state)
{
	m_centronics_select = state;
}

void tandy2k_state::write_centronics_fault(int state)
{
	m_centronics_fault = state;
}

uint8_t tandy2k_state::ppi_pb_r()
{
	/*

	    bit     signal          description

	    0       LPRIN0          auxiliary input 0
	    1       LPRIN1          auxiliary input 1
	    2       LPRIN2          auxiliary input 2
	    3       _LPRACK         acknowledge
	    4       _LPRFLT         fault
	    5       _LPRSEL         select
	    6       LPRPAEM         paper empty
	    7       LPRBSY          busy

	*/

	uint8_t data = 0;

	switch (m_pb_sel)
	{
	case LPINEN:
		// printer acknowledge
		data |= m_centronics_ack << 3;

		// printer fault
		data |= m_centronics_fault << 4;

		// printer select
		data |= m_centronics_select << 5;

		// paper empty
		data |= m_centronics_perror << 6;

		// printer busy
		data |= m_centronics_busy << 7;
		break;

	case KBDINEN:
		// keyboard data
		data = m_kbdin;
		break;

	case PORTINEN:
		// PCB revision
		data = 0x03;
		break;
	}

	return data;
}

void tandy2k_state::ppi_pc_w(uint8_t data)
{
	/*

	    bit     signal          description

	    0                       port A direction
	    1                       port B input select bit 0
	    2                       port B input select bit 1
	    3       LPRINT13        interrupt
	    4       STROBE IN
	    5       INBUFFULL
	    6       _LPRACK
	    7       _LPRDATSTB

	*/

	// input select
	m_pb_sel = (data >> 1) & 0x03;

	// interrupt
	m_pic1->ir3_w(BIT(data, 3));

	// printer strobe
	m_centronics->write_strobe(BIT(data, 7));
}


// Intel 8259 Interfaces

/*

    IR0     MEMINT00
    IR1     TMOINT01
    IR2     SERINT02
    IR3     BUSINT03
    IR4     FLDINT04
    IR5     BUSINT05
    IR6     HDCINT06
    IR7     BUSINT07

*/

/*

    IR0     KBDINT10
    IR1     VIDINT11
    IR2     RATINT12
    IR3     LPRINT13
    IR4     MCPINT14
    IR5     MEMINT15
    IR6     DMEINT16
    IR7     BUSINT17

*/

// Intel 8272 Interface

void tandy2k_state::fdc_drq_w(int state)
{
	dma_request(0, state);
}

void tandy2k_state::fdc_hdl_w(int state)
{
	m_floppy0->mon_w(!state);
	m_floppy1->mon_w(!state);
}

void tandy2k_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_TANDY_2000_FORMAT);
}

static void tandy2k_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

// Keyboard

void tandy2k_state::kbdclk_w(int state)
{
	if (!m_kbdclk && state)
	{
		m_kbdin >>= 1;
		m_kbdin |= m_kb->data_r() << 7;
	}

	m_kbdclk = state;
}

void tandy2k_state::kbddat_w(int state)
{
	if (!m_kbddat && state)
	{
		m_kb->busy_w(m_kbdclk);
		m_pic1->ir0_w(!m_kbdclk);
	}

	m_kbddat = state;
}

uint8_t tandy2k_state::irq_callback(offs_t offset)
{
	return (offset ? m_pic1 : m_pic0)->acknowledge();
}

// Machine Initialization

void tandy2k_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();
	int ram_size = m_ram->size();

	program.install_ram(0x00000, ram_size - 1, ram);

	m_mouse_timer = timer_alloc(FUNC(tandy2k_state::update_mouse), this);
	m_mcu_delay = timer_alloc(FUNC(tandy2k_state::mcu_delay_cb), this);

	// register for state saving
	save_item(NAME(m_dma_mux));
	save_item(NAME(m_kbdclk));
	save_item(NAME(m_kbddat));
	save_item(NAME(m_kbdin));
	save_item(NAME(m_extclk));
	save_item(NAME(m_rxrdy));
	save_item(NAME(m_txrdy));
	save_item(NAME(m_pb_sel));
	save_item(NAME(m_vidouts));
	save_item(NAME(m_clkspd));
	save_item(NAME(m_clkcnt));
	save_item(NAME(m_outspkr));
	save_item(NAME(m_spkrdata));
	save_item(NAME(m_clkmouse_cmd));
	save_item(NAME(m_clkmouse_cnt));
	save_item(NAME(m_clkmouse_irq));
	save_item(NAME(m_mouse_x));
	save_item(NAME(m_mouse_y));
	save_item(NAME(m_hires_en));
}

void tandy2k_state::machine_reset()
{
	m_hires_en = 0;
	m_clkmouse_cnt = 0;
	m_clkmouse_irq = 0;
}

void tandy2k_state::device_reset_after_children()
{
	m_pc_keyboard->enable(0);
}

TIMER_CALLBACK_MEMBER(tandy2k_state::update_mouse)
{
	uint16_t x = m_x_axis->read();
	uint16_t y = m_y_axis->read();
	uint16_t dx = x - m_mouse_x;
	uint16_t dy = y - m_mouse_y;
	m_mouse_x = x;
	m_mouse_y = y;
	m_clkmouse_cnt = 5;
	m_clkmouse_cmd[4] = 'A';
	m_clkmouse_cmd[3] = dx & 0xff;
	m_clkmouse_cmd[2] = dx >> 8;
	m_clkmouse_cmd[1] = dy & 0xff;
	m_clkmouse_cmd[0] = dy >> 8;
	m_pic1->ir2_w(1);
}

TIMER_CALLBACK_MEMBER(tandy2k_state::mcu_delay_cb)
{
	m_pic1->ir2_w(1);
}

rgb_t tandy2k_state::IRGB(uint32_t raw)
{
	uint8_t i = (raw >> 3) & 1;
	return rgb_t(pal2bit(((raw & 4) >> 1) | i), pal2bit((raw & 2) | i), pal2bit(((raw & 1) << 1) | i));
}

// Machine Driver

void tandy2k_state::tandy2k(machine_config &config)
{
	// basic machine hardware
	I80186(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tandy2k_state::tandy2k_mem);
	m_maincpu->set_addrmap(AS_IO, &tandy2k_state::tandy2k_io);
	m_maincpu->read_slave_ack_callback().set(FUNC(tandy2k_state::irq_callback));

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(640, 400);
	screen.set_visarea(0, 640-1, 0, 400-1);
	//screen.set_screen_update(CRT9021B_TAG, FUNC(crt9021_device::screen_update));
	screen.set_screen_update(FUNC(tandy2k_state::screen_update));

	PALETTE(config, m_colpal).set_format(1, &tandy2k_state::IRGB, 8);

	crt9007_device &vpac(CRT9007(config, CRT9007_TAG, 16_MHz_XTAL * 28 / 20 / 8));
	vpac.set_addrmap(0, &tandy2k_state::vpac_mem);
	vpac.set_character_width(8);
	vpac.int_callback().set(I8259A_1_TAG, FUNC(pic8259_device::ir1_w));
	vpac.vs_callback().set(CRT9021B_TAG, FUNC(crt9021_device::vsync_w));
	vpac.vlt_callback().set(FUNC(tandy2k_state::vpac_vlt_w));
	vpac.curs_callback().set(CRT9021B_TAG, FUNC(crt9021_device::cursor_w));
	vpac.drb_callback().set(FUNC(tandy2k_state::vpac_drb_w));
	vpac.wben_callback().set(FUNC(tandy2k_state::vpac_wben_w));
	vpac.cblank_callback().set(FUNC(tandy2k_state::vpac_cblank_w));
	vpac.slg_callback().set(FUNC(tandy2k_state::vpac_slg_w));
	vpac.sld_callback().set(FUNC(tandy2k_state::vpac_sld_w));
	vpac.set_screen(SCREEN_TAG);

	CRT9212(config, m_drb0, 0);
	m_drb0->set_wen2(1);
	m_drb0->dout().set(FUNC(tandy2k_state::vidla_w));

	CRT9212(config, m_drb1, 0);
	m_drb1->set_wen2(1);
	m_drb1->dout().set(FUNC(tandy2k_state::drb_attr_w));

	CRT9021(config, m_vac, 16_MHz_XTAL * 28 / 20);
	m_vac->set_screen(SCREEN_TAG);

	ADDRESS_MAP_BANK(config, m_vrambank).set_map(&tandy2k_state::vrambank_mem).set_options(ENDIANNESS_LITTLE, 16, 17, 0x8000);

	TIMER(config, "vidldsh").configure_generic(FUNC(tandy2k_state::vidldsh_tick));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	// devices
	I8255A(config, m_i8255a);
	m_i8255a->out_pa_callback().set("cent_data_out", FUNC(output_latch_device::write));
	m_i8255a->in_pb_callback().set(FUNC(tandy2k_state::ppi_pb_r));
	m_i8255a->out_pc_callback().set(FUNC(tandy2k_state::ppi_pc_w));

	I8251(config, m_uart, 0);
	m_uart->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_uart->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_uart->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_uart->rxrdy_handler().set(FUNC(tandy2k_state::rxrdy_w));
	m_uart->txrdy_handler().set(FUNC(tandy2k_state::txrdy_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_uart, FUNC(i8251_device::write_rxd));
	m_rs232->dsr_handler().set(m_uart, FUNC(i8251_device::write_dsr));
	// TODO pin 15 external transmit clock
	// TODO pin 17 external receiver clock

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(16_MHz_XTAL / 16);
	m_pit->out_handler<0>().set(FUNC(tandy2k_state::outspkr_w));
	m_pit->set_clk<1>(16_MHz_XTAL / 8);
	m_pit->out_handler<1>().set(FUNC(tandy2k_state::intbrclk_w));
	//m_pit->set_clk<2>(16_MHz_XTAL / 8);
	//m_pit->out_handler<2>().set(FUNC(tandy2k_state::rfrqpulse_w));

	PIC8259(config, m_pic0, 0);
	m_pic0->out_int_callback().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));

	PIC8259(config, m_pic1, 0);
	m_pic1->out_int_callback().set(m_maincpu, FUNC(i80186_cpu_device::int1_w));

	I8272A(config, m_fdc, 16_MHz_XTAL / 4, true);
	m_fdc->set_select_lines_connected(true);
	m_fdc->intrq_wr_callback().set(m_pic0, FUNC(pic8259_device::ir4_w));
	m_fdc->drq_wr_callback().set(FUNC(tandy2k_state::fdc_drq_w));
	m_fdc->hdl_wr_callback().set(FUNC(tandy2k_state::fdc_hdl_w));
	FLOPPY_CONNECTOR(config, I8272A_TAG ":0", tandy2k_floppies, "525qd", tandy2k_state::floppy_formats);
	FLOPPY_CONNECTOR(config, I8272A_TAG ":1", tandy2k_floppies, "525qd", tandy2k_state::floppy_formats);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(tandy2k_state::write_centronics_ack));
	m_centronics->busy_handler().set(FUNC(tandy2k_state::write_centronics_busy));
	m_centronics->perror_handler().set(FUNC(tandy2k_state::write_centronics_perror));
	m_centronics->select_handler().set(FUNC(tandy2k_state::write_centronics_select));
	m_centronics->fault_handler().set(FUNC(tandy2k_state::write_centronics_fault));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	TANDY2K_KEYBOARD(config, m_kb, 0);
	m_kb->clock_wr_callback().set(FUNC(tandy2k_state::kbdclk_w));
	m_kb->data_wr_callback().set(FUNC(tandy2k_state::kbddat_w));

	// temporary until the tandy keyboard has a rom dump
	TANDY2K_HLE_KEYB(config, m_pc_keyboard, 0).keypress().set(I8259A_1_TAG, FUNC(pic8259_device::ir0_w));

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("tandy2k");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("128K").set_extra_options("256K,384K,512K,640K,768K,896K");
}

void tandy2k_state::tandy2k_hd(machine_config &config)
{
	tandy2k(config);
	// basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &tandy2k_state::tandy2k_hd_io);

	// Tandon TM502 hard disk
	HARDDISK(config, "harddisk0", 0);
	//MCFG_WD1010_ADD(WD1010_TAG, wd1010_intf)
	//MCFG_WD1100_11_ADD(WD1100_11_TAG, wd1100_11_intf)
}

// ROMs

ROM_START( tandy2k )
	ROM_REGION( 0x2000, I80186_TAG, 0 )
	ROM_LOAD16_BYTE( "484a00.u48", 0x0000, 0x1000, CRC(a5ee3e90) SHA1(4b1f404a4337c67065dd272d62ff88dcdee5e34b) )
	ROM_LOAD16_BYTE( "474600.u47", 0x0001, 0x1000, CRC(345701c5) SHA1(a775cbfa110b7a88f32834aaa2a9b868cbeed25b) )

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "82s153.u62", 0x000, 0x100, NO_DUMP ) // interrupt/DMA
	ROM_LOAD( "82s153.u68", 0x000, 0x100, NO_DUMP ) // video
	ROM_LOAD( "82s153.u95", 0x000, 0x100, NO_DUMP ) // memory timing
	ROM_LOAD( "pal10l8.u82", 0x000, 0x100, NO_DUMP ) // video
	ROM_LOAD( "pal16l8a.u102", 0x000, 0x100, NO_DUMP ) // bus interface
	ROM_LOAD( "pal16l8a.u103", 0x000, 0x100, NO_DUMP ) // bus interface
	ROM_LOAD( "pal20l8.u103", 0x000, 0x100, NO_DUMP ) // bus interface, alternate
	ROM_LOAD( "pal16r6a.u16", 0x000, 0x100, NO_DUMP ) // HDC
ROM_END

#define rom_tandy2khd rom_tandy2k

// System Drivers

//    YEAR  NAME       PARENT   COMPAT  MACHINE     INPUT    CLASS          INIT        COMPANY              FULLNAME        FLAGS
COMP( 1983, tandy2k,   0,       0,      tandy2k,    tandy2k, tandy2k_state, empty_init, "Tandy Radio Shack", "Tandy 2000",   MACHINE_NOT_WORKING )
COMP( 1983, tandy2khd, tandy2k, 0,      tandy2k_hd, tandy2k, tandy2k_state, empty_init, "Tandy Radio Shack", "Tandy 2000HD", MACHINE_NOT_WORKING )
