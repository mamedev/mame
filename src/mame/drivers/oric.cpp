// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    drivers/oric.c

    Systems supported by this driver:

    Oric 1,
    Oric Atmos,
    Oric Telestrat,
    Pravetz 8D

    Pravetz is a Bulgarian copy of the Oric Atmos and uses
    Apple 2 disc drives for storage.

    This driver originally by Paul Cook, rewritten by Kevin Thacker,
    re-rewritten by Olivier Galibert.

*********************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/oricext/oricext.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "formats/oric_dsk.h"
#include "formats/oric_tap.h"


class oric_state : public driver_device
{
public:
	// Permanent attributes (kept from one line to the other) and line
	// attributes (reset at start of line)
	enum {
		PATTR_50HZ  = 0x02,
		PATTR_HIRES = 0x04,
		LATTR_ALT   = 0x01,
		LATTR_DSIZE = 0x02,
		LATTR_BLINK = 0x04
	};

	oric_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_psg(*this, "ay8912")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_cassette(*this, "cassette")
		, m_via(*this, "via6522")
		, m_ram(*this, "ram")
		, m_rom(*this, "maincpu")
		, m_bank_c000_r(*this, "bank_c000_r")
		, m_bank_e000_r(*this, "bank_e000_r")
		, m_bank_f800_r(*this, "bank_f800_r")
		, m_bank_c000_w(*this, "bank_c000_w")
		, m_bank_e000_w(*this, "bank_e000_w")
		, m_bank_f800_w(*this, "bank_f800_w")
		, m_config(*this, "CONFIG")
		, m_kbd_row(*this, "ROW%u", 0U)
		, m_tape_timer(nullptr)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(nmi_pressed);
	DECLARE_WRITE8_MEMBER(via_a_w);
	DECLARE_WRITE8_MEMBER(via_b_w);
	DECLARE_WRITE_LINE_MEMBER(via_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(via_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(via_irq_w);
	DECLARE_WRITE_LINE_MEMBER(ext_irq_w);
	DECLARE_WRITE8_MEMBER(psg_a_w);
	TIMER_CALLBACK_MEMBER(update_tape);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_oric(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_w);

	void oric(machine_config &config, bool add_ext = true);
	void prav8d(machine_config &config);
	void oric_mem(address_map &map);
protected:
	required_device<m6502_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<ay8910_device> m_psg;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<cassette_image_device> m_cassette;
	required_device<via6522_device> m_via;
	required_shared_ptr<uint8_t> m_ram;
	optional_memory_region m_rom;
	required_memory_bank m_bank_c000_r;
	optional_memory_bank m_bank_e000_r;
	optional_memory_bank m_bank_f800_r;
	required_memory_bank m_bank_c000_w;
	optional_memory_bank m_bank_e000_w;
	optional_memory_bank m_bank_f800_w;
	required_ioport m_config;
	required_ioport_array<8> m_kbd_row;

	emu_timer *m_tape_timer;

	int m_blink_counter;
	uint8_t m_pattr;
	uint8_t m_via_a, m_via_b, m_psg_a;
	bool m_via_ca2, m_via_cb2, m_via_irq;
	bool m_ext_irq;

	virtual void update_irq();
	void update_psg();
	void update_keyboard();
	void machine_start_common();
};

class telestrat_state : public oric_state
{
public:
	telestrat_state(const machine_config &mconfig, device_type type, const char *tag) :
		oric_state(mconfig, type, tag),
		m_via2(*this, "via6522_2"),
		m_fdc(*this, "fdc"),
		m_telmatic(*this, "telmatic"),
		m_teleass(*this, "teleass"),
		m_hyperbas(*this, "hyperbas"),
		m_telmon24(*this, "telmon24"),
		m_joy1(*this, "JOY1"),
		m_joy2(*this, "JOY2")
	{ }

	DECLARE_WRITE8_MEMBER(via2_a_w);
	DECLARE_WRITE8_MEMBER(via2_b_w);
	DECLARE_WRITE_LINE_MEMBER(via2_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(via2_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(via2_irq_w);

	DECLARE_WRITE_LINE_MEMBER(acia_irq_w);

	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_hld_w);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void telstrat(machine_config &config);
	void telestrat_mem(address_map &map);
protected:
	enum {
		P_IRQEN  = 0x01,
		P_DDS    = 0x04,
		P_DDEN   = 0x08,
		P_SS     = 0x10,
		P_DRIVE  = 0x60
	};

	required_device<via6522_device> m_via2;
	required_device<fd1793_device> m_fdc;
	required_memory_region m_telmatic;
	required_memory_region m_teleass;
	required_memory_region m_hyperbas;
	required_memory_region m_telmon24;
	required_ioport m_joy1;
	required_ioport m_joy2;

	floppy_image_device *m_floppies[4];
	uint8_t m_port_314;
	uint8_t m_via2_a, m_via2_b;
	bool m_via2_ca2, m_via2_cb2, m_via2_irq;
	bool m_acia_irq;
	bool m_fdc_irq, m_fdc_drq, m_fdc_hld;

	uint8_t m_junk_read[0x4000], m_junk_write[0x4000];

	virtual void update_irq() override;
	void remap();
	void port_314_w(u8 data);
	u8 port_314_r();
	u8 port_318_r();
};

/* Ram is 64K, with 16K hidden by the rom.  The 300-3ff is also hidden by the i/o */
void oric_state::oric_mem(address_map &map)
{
	map(0x0000, 0xffff).ram().share("ram");
	map(0x0300, 0x030f).m(m_via, FUNC(via6522_device::map)).mirror(0xf0);
	map(0xc000, 0xdfff).bankr("bank_c000_r").bankw("bank_c000_w");
	map(0xe000, 0xf7ff).bankr("bank_e000_r").bankw("bank_e000_w");
	map(0xf800, 0xffff).bankr("bank_f800_r").bankw("bank_f800_w");
}

/*
The telestrat has the memory regions split into 16k blocks.
Memory region &c000-&ffff can be ram or rom. */
void telestrat_state::telestrat_mem(address_map &map)
{
	map(0x0000, 0xffff).ram().share("ram");
	map(0x0300, 0x030f).m(m_via, FUNC(via6522_device::map));
	map(0x0310, 0x0313).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x0314, 0x0314).rw(FUNC(telestrat_state::port_314_r), FUNC(telestrat_state::port_314_w));
	map(0x0318, 0x0318).r(FUNC(telestrat_state::port_318_r));
	map(0x031c, 0x031f).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x0320, 0x032f).m(m_via2, FUNC(via6522_device::map));
	map(0xc000, 0xffff).bankr("bank_c000_r").bankw("bank_c000_w");
}

uint32_t oric_state::screen_update_oric(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bool blink_state = m_blink_counter & 0x20;
	m_blink_counter = (m_blink_counter + 1) & 0x3f;

	uint8_t pattr = m_pattr;

	for(int y=0; y<224; y++) {
		// Line attributes and current colors
		uint8_t lattr = 0;
		uint32_t fgcol = m_palette->pen_color(7);
		uint32_t bgcol = m_palette->pen_color(0);

		uint32_t *p = &bitmap.pix32(y);

		for(int x=0; x<40; x++) {
			// Lookup the byte and, if needed, the pattern data
			uint8_t ch, pat;
			if((pattr & PATTR_HIRES) && y < 200)
				ch = pat = m_ram[0xa000 + y*40 + x];

			else {
				ch = m_ram[0xbb80 + (y>>3)*40 + x];
				int off = (lattr & LATTR_DSIZE ? y >> 1 : y ) & 7;
				const uint8_t *base;
				if(pattr & PATTR_HIRES)
					if(lattr & LATTR_ALT)
						base = m_ram + 0x9c00;
					else
						base = m_ram + 0x9800;
				else
					if(lattr & LATTR_ALT)
						base = m_ram + 0xb800;
					else
						base = m_ram + 0xb400;
				pat = base[((ch & 0x7f) << 3) | off];
			}

			// Handle state-chaging attributes
			if(!(ch & 0x60)) {
				pat = 0x00;
				switch(ch & 0x18) {
				case 0x00: fgcol = m_palette->pen_color(ch & 7); break;
				case 0x08: lattr = ch & 7; break;
				case 0x10: bgcol = m_palette->pen_color(ch & 7); break;
				case 0x18: pattr = ch & 7; break;
				}
			}

			// Pick up the colors for the pattern
			uint32_t c_fgcol = fgcol;
			uint32_t c_bgcol = bgcol;

			//    inverse video
			if(ch & 0x80) {
				c_bgcol = c_bgcol ^ 0xffffff;
				c_fgcol = c_fgcol ^ 0xffffff;
			}
			//    blink
			if((lattr & LATTR_BLINK) && blink_state)
				c_fgcol = c_bgcol;

			// Draw the pattern
			*p++ = pat & 0x20 ? c_fgcol : c_bgcol;
			*p++ = pat & 0x10 ? c_fgcol : c_bgcol;
			*p++ = pat & 0x08 ? c_fgcol : c_bgcol;
			*p++ = pat & 0x04 ? c_fgcol : c_bgcol;
			*p++ = pat & 0x02 ? c_fgcol : c_bgcol;
			*p++ = pat & 0x01 ? c_fgcol : c_bgcol;
		}
	}

	m_pattr = pattr;

	return 0;
}

void oric_state::update_keyboard()
{
	m_via->write_pb3((m_kbd_row[m_via_b & 7]->read() | m_psg_a) != 0xff);
}

void oric_state::update_psg()
{
	if(m_via_ca2)
		if(m_via_cb2)
			m_psg->address_w(m_via_a);
		else
			m_via->write_pa(m_psg->data_r());
	else if(m_via_cb2)
		m_psg->data_w(m_via_a);
}

void oric_state::update_irq()
{
	m_maincpu->set_input_line(m6502_device::IRQ_LINE, m_via_irq || m_ext_irq ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(oric_state::nmi_pressed)
{
	m_maincpu->set_input_line(m6502_device::NMI_LINE, newval ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(oric_state::via_a_w)
{
	m_via_a = data;
	m_cent_data_out->write(m_via_a);
	update_psg();
}

WRITE8_MEMBER(oric_state::via_b_w)
{
	m_via_b = data;
	update_keyboard();
	m_centronics->write_strobe(data & 0x10 ? 1 : 0);
	m_cassette->change_state(data & 0x40 ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,
								CASSETTE_MOTOR_DISABLED);
	m_cassette->output(data & 0x80 ? -1.0 : +1.0);
}

WRITE_LINE_MEMBER(oric_state::via_ca2_w)
{
	m_via_ca2 = state;
	update_psg();
}

WRITE_LINE_MEMBER(oric_state::via_cb2_w)
{
	m_via_cb2 = state;
	update_psg();
}

WRITE_LINE_MEMBER(oric_state::via_irq_w)
{
	m_via_irq = state;
	update_irq();
}

WRITE_LINE_MEMBER(oric_state::ext_irq_w)
{
	m_ext_irq = state;
	update_irq();
}

WRITE8_MEMBER(oric_state::psg_a_w)
{
	m_psg_a = data;
	update_keyboard();
}

TIMER_CALLBACK_MEMBER(oric_state::update_tape)
{
	if(!m_config->read())
		m_via->write_cb1(m_cassette->input() > 0.0038);
}

WRITE_LINE_MEMBER(oric_state::vblank_w)
{
	if(m_config->read())
		m_via->write_cb1(state);
}

void oric_state::video_start()
{
	m_blink_counter = 0;
	m_pattr = 0;
}

void oric_state::machine_start_common()
{
	m_via_a = 0xff;
	m_via_b = 0xff;
	m_psg_a = 0x00;
	m_via_ca2 = false;
	m_via_cb2 = false;
	m_via_irq = false;
	m_ext_irq = false;

	if (!m_tape_timer)
		m_tape_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(oric_state::update_tape), this));
}

void oric_state::machine_start()
{
	machine_start_common();
	m_bank_c000_r->set_base(m_rom->base());
	m_bank_e000_r->set_base(m_rom->base() + 0x2000);
	m_bank_f800_r->set_base(m_rom->base() + 0x3800);
}


void oric_state::machine_reset()
{
	m_tape_timer->adjust(attotime::from_hz(4800), 0, attotime::from_hz(4800));
}

void telestrat_state::machine_start()
{
	machine_start_common();
	for(int i=0; i<4; i++) {
		char name[32];
		sprintf(name, "fdc:%d", i);
		m_floppies[i] = subdevice<floppy_connector>(name)->get_device();
	}
	m_fdc_irq = m_fdc_drq = m_fdc_hld = false;
	m_acia_irq = false;

	memset(m_junk_read, 0x00, sizeof(m_junk_read));
	memset(m_junk_write, 0x00, sizeof(m_junk_write));
}

void telestrat_state::machine_reset()
{
	m_tape_timer->adjust(attotime::from_hz(4800), 0, attotime::from_hz(4800));
	m_port_314 = 0x00;
	m_via2_a = 0xff;
	remap();
}

void telestrat_state::update_irq()
{
	m_maincpu->set_input_line(m6502_device::IRQ_LINE,
								m_via_irq ||
								m_ext_irq ||
								(m_fdc_irq && (m_port_314 & P_IRQEN)) ||
								m_via2_irq ||
								m_acia_irq ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(telestrat_state::via2_a_w)
{
	m_via2_a = data;
	remap();
}

WRITE8_MEMBER(telestrat_state::via2_b_w)
{
	m_via2_b = data;
	uint8_t port = 0xff;
	if(!(m_via2_b & 0x40))
		port &= m_joy1->read();
	if(!(m_via2_b & 0x80))
		port &= m_joy2->read();

	m_via2->write_pb(port);
}

WRITE_LINE_MEMBER(telestrat_state::via2_ca2_w)
{
	m_via2_ca2 = state;
}

WRITE_LINE_MEMBER(telestrat_state::via2_cb2_w)
{
	m_via2_cb2 = state;
}

WRITE_LINE_MEMBER(telestrat_state::via2_irq_w)
{
	m_via2_irq = state;
	update_irq();
}

void telestrat_state::port_314_w(u8 data)
{
	m_port_314 = data;
	floppy_image_device *floppy = m_floppies[(m_port_314 >> 5) & 3];
	m_fdc->set_floppy(floppy);
	m_fdc->dden_w(m_port_314 & P_DDEN);
	if(floppy) {
		floppy->ss_w(m_port_314 & P_SS ? 1 : 0);
		floppy->mon_w(0);
	}
	update_irq();
}

u8 telestrat_state::port_314_r()
{
	return (m_fdc_irq && (m_port_314 & P_IRQEN)) ? 0x7f : 0xff;
}

u8 telestrat_state::port_318_r()
{
	return m_fdc_drq ? 0x7f : 0xff;
}


WRITE_LINE_MEMBER(telestrat_state::acia_irq_w)
{
	m_acia_irq = state;
	update_irq();
}

WRITE_LINE_MEMBER(telestrat_state::fdc_irq_w)
{
	m_fdc_irq = state;
	update_irq();
}

WRITE_LINE_MEMBER(telestrat_state::fdc_drq_w)
{
	m_fdc_drq = state;
}

WRITE_LINE_MEMBER(telestrat_state::fdc_hld_w)
{
	m_fdc_hld = state;
}

void telestrat_state::remap()
{
	// Theorically, these are cartridges.  There's no real point to
	// making them configurable, when only 4 existed and there are 7
	// slots.

	switch(m_via2_a & 7) {
	case 0:
		m_bank_c000_r->set_base(m_ram+0xc000);
		m_bank_c000_w->set_base(m_ram+0xc000);
		break;
	case 1:
	case 2:
	case 3:
		m_bank_c000_r->set_base(m_junk_read);
		m_bank_c000_w->set_base(m_junk_write);
		break;
	case 4:
		m_bank_c000_r->set_base(m_telmatic->base());
		m_bank_c000_w->set_base(m_junk_write);
		break;
	case 5:
		m_bank_c000_r->set_base(m_teleass->base());
		m_bank_c000_w->set_base(m_junk_write);
		break;
	case 6:
		m_bank_c000_r->set_base(m_hyperbas->base());
		m_bank_c000_w->set_base(m_junk_write);
		break;
	case 7:
		m_bank_c000_r->set_base(m_telmon24->base());
		m_bank_c000_w->set_base(m_junk_write);
		break;
	}
}



static INPUT_PORTS_START(oric)
	PORT_START("ROW0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)           PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)           PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)           PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("ROW1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)           PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)         PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)           PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)           PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)           PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("ROW2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)           PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)           PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)           PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("ROW3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-') PORT_CHAR(0xA3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("ROW4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)       PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)        PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)       PORT_CHAR(' ')

	PORT_START("ROW5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)           PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)           PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('u') PORT_CHAR('U')

	PORT_START("ROW6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)           PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)           PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)           PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)           PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)           PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("ROW7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)           PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)           PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('*')

	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NMI") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHANGED_MEMBER(DEVICE_SELF, oric_state, nmi_pressed, 0)

	/* vsync cable hardware. This is a simple cable connected to the video output
	to the monitor/television. The sync signal is connected to the cassette input
	allowing interrupts to be generated from the vsync signal. */
	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x00, "Tape input")
	PORT_CONFSETTING(   0x00, "Tape")
	PORT_CONFSETTING(   0x01, "VSync cable")
INPUT_PORTS_END

static INPUT_PORTS_START(orica)
	PORT_INCLUDE( oric )

	PORT_MODIFY("ROW5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Funct") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
INPUT_PORTS_END

static INPUT_PORTS_START(prav8d)
	PORT_START("ROW0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)                                   PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X \xd0\xac") PORT_CODE(KEYCODE_X)           PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)                                   PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V \xd0\x96") PORT_CODE(KEYCODE_V)           PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)                                   PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N \xd0\x9d") PORT_CODE(KEYCODE_N)           PORT_CHAR('N')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)                                   PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("ROW1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D \xd0\x94") PORT_CODE(KEYCODE_D)           PORT_CHAR('D')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q \xd0\xaf") PORT_CODE(KEYCODE_Q)           PORT_CHAR('Q')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)                PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F \xd0\xa4") PORT_CODE(KEYCODE_F)           PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R \xd0\xa0") PORT_CODE(KEYCODE_R)           PORT_CHAR('R')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T \xd0\xa2") PORT_CODE(KEYCODE_T)           PORT_CHAR('T')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J \xd0\x99") PORT_CODE(KEYCODE_J)           PORT_CHAR('J')

	PORT_START("ROW2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C \xd0\xa6") PORT_CODE(KEYCODE_C)           PORT_CHAR('C')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)                                   PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z \xd0\x97") PORT_CODE(KEYCODE_Z)           PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MK") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)                                   PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B \xd0\x91") PORT_CODE(KEYCODE_B)           PORT_CHAR('B')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)                                   PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M \xd0\x9c") PORT_CODE(KEYCODE_M)           PORT_CHAR('M')

	PORT_START("ROW3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("] \xd0\xa9") PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR(']')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)                          PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C/L") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) // this one is 5th line, 1st key from right
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)                               PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ \xd0\xa8") PORT_CODE(KEYCODE_COLON)       PORT_CHAR('[')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)                                   PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K \xd0\x9a") PORT_CODE(KEYCODE_K)           PORT_CHAR('K')

	PORT_START("ROW4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)                              PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP)       PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)                                PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)                               PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)                               PORT_CHAR(' ')

	PORT_START("ROW5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@ \xd0\xae") PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('@')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ \xd0\xad") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_BACKSPACE)          PORT_CHAR(8) // this one is 5th line, 1st key from left
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P \xd0\x9f") PORT_CODE(KEYCODE_P)           PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O \xd0\x9e") PORT_CODE(KEYCODE_O)           PORT_CHAR('O')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I \xd0\x98") PORT_CODE(KEYCODE_I)           PORT_CHAR('I')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U \xd0\xa3") PORT_CODE(KEYCODE_U)           PORT_CHAR('U')

	PORT_START("ROW6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W \xd0\x92") PORT_CODE(KEYCODE_W)           PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S \xd0\xa1") PORT_CODE(KEYCODE_S)           PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A \xd0\x90") PORT_CODE(KEYCODE_A)           PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E \xd0\x95") PORT_CODE(KEYCODE_E)           PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G \xd0\x93") PORT_CODE(KEYCODE_G)           PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H \xd0\xa5") PORT_CODE(KEYCODE_H)           PORT_CHAR('H')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y \xd0\xaa") PORT_CODE(KEYCODE_Y)           PORT_CHAR('Y')

	PORT_START("ROW7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)                              PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^ \xd0\xa7") PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR('^') // this one would be on 2nd line, 3rd key from 'P'
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)           PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)                              PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)                               PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)                                   PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L \xd0\x9b") PORT_CODE(KEYCODE_L)           PORT_CHAR('L')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)                                   PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NMI") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHANGED_MEMBER(DEVICE_SELF, oric_state, nmi_pressed, 0)

	/* vsync cable hardware. This is a simple cable connected to the video output
	to the monitor/television. The sync signal is connected to the cassette input
	allowing interrupts to be generated from the vsync signal. */
	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x00, "Tape input")
	PORT_CONFSETTING(   0x00, "Tape")
	PORT_CONFSETTING(   0x01, "VSync cable")
INPUT_PORTS_END

static INPUT_PORTS_START(telstrat)
	PORT_INCLUDE( orica )

// The telestrat does not have the NMI button
	PORT_MODIFY("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("JOY1")      /* left joystick port */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1)                  PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(1)

	PORT_START("JOY2")      /* right joystick port */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1)                  PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(2)
INPUT_PORTS_END


void oric_state::oric(machine_config &config, bool add_ext)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 12_MHz_XTAL / 12);
	m_maincpu->set_addrmap(AS_PROGRAM, &oric_state::oric_mem);

	config.set_maximum_quantum(attotime::from_hz(60));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12_MHz_XTAL / 2, 64*6, 0, 40*6, 312, 0, 28*8); // 260 lines in 60 Hz mode
	screen.set_screen_update(FUNC(oric_state::screen_update_oric));
	screen.screen_vblank().set(FUNC(oric_state::vblank_w));

	PALETTE(config, m_palette, palette_device::RGB_3BIT);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8912(config, m_psg, 12_MHz_XTAL / 12);
	m_psg->set_flags(AY8910_DISCRETE_OUTPUT);
	m_psg->set_resistors_load(4700, 4700, 4700);
	m_psg->port_a_write_callback().set(FUNC(oric_state::psg_a_w));
	m_psg->add_route(ALL_OUTPUTS, "mono", 0.25);

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_via, FUNC(via6522_device::write_ca1));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	/* cassette */
	CASSETTE(config, m_cassette, 0);
	m_cassette->set_formats(oric_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* via */
	VIA6522(config, m_via, 12_MHz_XTAL / 12);
	m_via->writepa_handler().set(FUNC(oric_state::via_a_w));
	m_via->writepb_handler().set(FUNC(oric_state::via_b_w));
	m_via->ca2_handler().set(FUNC(oric_state::via_ca2_w));
	m_via->cb2_handler().set(FUNC(oric_state::via_cb2_w));
	m_via->irq_handler().set(FUNC(oric_state::via_irq_w));

	/* extension port */
	ORICEXT_CONNECTOR(config, "ext", oricext_intf, nullptr, "maincpu").irq_callback().set(FUNC(oric_state::ext_irq_w));
}

void oric_state::prav8d(machine_config &config)
{
	oric(config, true);
}

FLOPPY_FORMATS_MEMBER( telestrat_state::floppy_formats )
	FLOPPY_ORIC_DSK_FORMAT
FLOPPY_FORMATS_END

static void telestrat_floppies(device_slot_interface &device)
{
	device.option_add("3dsdd", FLOPPY_3_DSDD);
}

void telestrat_state::telstrat(machine_config &config)
{
	oric(config, false);
	m_maincpu->set_addrmap(AS_PROGRAM, &telestrat_state::telestrat_mem);

	/* acia */
	mos6551_device &acia(MOS6551(config, "acia", 0));
	acia.set_xtal(1.8432_MHz_XTAL);
	acia.irq_handler().set(FUNC(telestrat_state::acia_irq_w));

	/* via */
	VIA6522(config, m_via2, 12_MHz_XTAL / 12);
	m_via2->writepa_handler().set(FUNC(telestrat_state::via2_a_w));
	m_via2->writepb_handler().set(FUNC(telestrat_state::via2_b_w));
	m_via2->ca2_handler().set(FUNC(telestrat_state::via2_ca2_w));
	m_via2->cb2_handler().set(FUNC(telestrat_state::via2_cb2_w));
	m_via2->irq_handler().set(FUNC(telestrat_state::via2_irq_w));

	/* microdisc */
	FD1793(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(FUNC(telestrat_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(telestrat_state::fdc_drq_w));
	m_fdc->hld_wr_callback().set(FUNC(telestrat_state::fdc_hld_w));
	m_fdc->set_force_ready(true);

	FLOPPY_CONNECTOR(config, "fdc:0", telestrat_floppies, "3dsdd", telestrat_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", telestrat_floppies, nullptr, telestrat_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:2", telestrat_floppies, nullptr, telestrat_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:3", telestrat_floppies, nullptr, telestrat_state::floppy_formats);
}


ROM_START(oric1)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD ("basic10.rom", 0, 0x04000, CRC(f18710b4) SHA1(333116e6884d85aaa4dfc7578a91cceeea66d016))
ROM_END

ROM_START(orica)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "ver11", "Basic 1.1")
	ROMX_LOAD ("basic11b.rom", 0, 0x04000, CRC(c3a92bef) SHA1(9451a1a09d8f75944dbd6f91193fc360f1de80ac), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "ver12", "Basic 1.2 (Pascal Leclerc)")      // 1987/1999 - various enhancements and bugfixes
	ROMX_LOAD ("basic12.rom",  0, 0x04000, CRC(dc4f22dc) SHA1(845e1a893de3dc0f856fdf2f69c3b73770b4094f), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "ver121", "Basic 1.21 (Pascal Leclerc)")        // 07.1999 - DRAW enhancement
	ROMX_LOAD ("basic121.rom", 0, 0x04000, CRC(0a2860b1) SHA1(b727d5c3bbc8cb1d510f224eb1e0d90d609e8506), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "ver122", "Basic 1.22 (Pascal Leclerc)")        // 08.2001 - added EUR symbol
	ROMX_LOAD ("basic122.rom", 0, 0x04000, CRC(5ef2a861) SHA1(9ab6dc47b6e9dc65a4137ce0f0f12fc2b6ca8442), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "ver11de", "Basic 1.1 DE")
	ROMX_LOAD( "bas11_de.rom", 0, 0x04000, CRC(65233b2d) SHA1(b01cabb1a21980a6785a2fe37a8f8572c892123f), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 5, "ver11es", "Basic 1.1 ES")
	ROMX_LOAD( "bas11_es.rom", 0, 0x04000, CRC(47bf26c7) SHA1(4fdbadd68db9ab8ad1cd56b4e5cbe51a9c3f11ae), ROM_BIOS(5))
	ROM_SYSTEM_BIOS( 6, "ver11fr", "Basic 1.1 FR")
	ROMX_LOAD( "bas11_fr.rom", 0, 0x04000, CRC(603b1fbf) SHA1(2a4583df3b59ca454d67d5631f242c96ec4cf99a), ROM_BIOS(6))
	ROM_SYSTEM_BIOS( 7, "ver11se", "Basic 1.1 SE")
	ROMX_LOAD( "bas11_se.rom", 0, 0x04000, CRC(a71523ac) SHA1(ce53acf84baec6ab5cbac9f9cefa71b3efeb2ead), ROM_BIOS(7))
	ROM_SYSTEM_BIOS( 8, "ver11uk", "Basic 1.1 UK")
	ROMX_LOAD( "bas11_uk.rom", 0, 0x04000, CRC(303370d1) SHA1(589ff66fac8e06d65af3369491faa67a71f1322a), ROM_BIOS(8))
	ROM_SYSTEM_BIOS( 9, "ver12es", "Basic 1.2 ES")
	ROMX_LOAD( "bas12es_le.rom", 0, 0x04000, CRC(70de4aeb) SHA1(b327418aa7d8a5a03c135e3d8acdd511df625893), ROM_BIOS(9))
	ROM_SYSTEM_BIOS( 10, "ver12fr", "Basic 1.2 FR")
	ROMX_LOAD( "bas12fr_le.rom", 0, 0x04000, CRC(47a437fc) SHA1(70271bc3ed5c3bf4d339d6f5de3de8c3c50ff573), ROM_BIOS(10))
	ROM_SYSTEM_BIOS( 11, "ver12ge", "Basic 1.2 GE")
	ROMX_LOAD( "bas12ge_le.rom", 0, 0x04000, CRC(f5f0dd52) SHA1(75359302452ee7b19537698f124aaefd333688d0), ROM_BIOS(11))
	ROM_SYSTEM_BIOS( 12, "ver12sw", "Basic 1.2 SW")
	ROMX_LOAD( "bas12sw_le.rom", 0, 0x04000, CRC(100abe68) SHA1(6211d5969c4d7a6acb86ed19c5e51a33a3bef431), ROM_BIOS(12))
	ROM_SYSTEM_BIOS( 13, "ver12uk", "Basic 1.2 UK")
	ROMX_LOAD( "bas12uk_le.rom", 0, 0x04000, CRC(00fce8a6) SHA1(d40558bdf61b8aba6260293c9424fd463be7fad8), ROM_BIOS(13))
	ROM_SYSTEM_BIOS( 14, "ver121es", "Basic 1.211 ES")
	ROMX_LOAD( "bas121es_le.rom", 0, 0x04000, CRC(87ec679b) SHA1(5de6a5f5121f69069c9b93d678046e814b5b64e9), ROM_BIOS(14))
	ROM_SYSTEM_BIOS( 15, "ver121fr", "Basic 1.211 FR")
	ROMX_LOAD( "bas121fr_le.rom", 0, 0x04000, CRC(e683dec2) SHA1(20df7ebc0f13aa835f286d50137f1a7ff7430c29), ROM_BIOS(15))
	ROM_SYSTEM_BIOS( 16, "ver121ge", "Basic 1.211 GE")
	ROMX_LOAD( "bas121ge_le.rom", 0, 0x04000, CRC(94fe32bf) SHA1(1024776d20030d602e432e50014502524658643a), ROM_BIOS(16))
	ROM_SYSTEM_BIOS( 17, "ver121sw", "Basic 1.211 SW")
	ROMX_LOAD( "bas121sw_le.rom", 0, 0x04000, CRC(e6ad11c7) SHA1(309c94a9861fcb770636dcde1801a5c68ca819b4), ROM_BIOS(17))
	ROM_SYSTEM_BIOS( 18, "ver121uk", "Basic 1.211 UK")
	ROMX_LOAD( "bas121uk_le.rom", 0, 0x04000, CRC(75aa1aa9) SHA1(ca99e244d9cbef625344c2054023504a4f9dcfe4), ROM_BIOS(18))
	ROM_SYSTEM_BIOS( 19, "ver122es", "Basic 1.22 ES")
	ROMX_LOAD( "bas122es_le.rom", 0, 0x04000, CRC(9144f9e0) SHA1(acf2094078af057e74a31d90d7010be51b9033fa), ROM_BIOS(19))
	ROM_SYSTEM_BIOS( 20, "ver122fr", "Basic 1.22 FR")
	ROMX_LOAD( "bas122fr_le.rom", 0, 0x04000, CRC(370cfda4) SHA1(fad9a0661256e59bcc2915578647573e4128e1bb), ROM_BIOS(20))
	ROM_SYSTEM_BIOS( 21, "ver122ge", "Basic 1.22 GE")
	ROMX_LOAD( "bas122ge_le.rom", 0, 0x04000, CRC(9a42bd62) SHA1(8a9c80f314daf4e5e64fa202e583b8a65796db8b), ROM_BIOS(21))
	ROM_SYSTEM_BIOS( 22, "ver122sw", "Basic 1.22 SW")
	ROMX_LOAD( "bas122sw_le.rom", 0, 0x04000, CRC(e7fd57a4) SHA1(c75cbf7cfafaa02712dc7ca2f972220aef86fb8d), ROM_BIOS(22))
	ROM_SYSTEM_BIOS( 23, "ver122uk", "Basic 1.22 UK")
	ROMX_LOAD( "bas122uk_le.rom", 0, 0x04000, CRC(9865bcd7) SHA1(2a92e2d119463e682bf10647e3880e26656d65b5), ROM_BIOS(23))
ROM_END

ROM_START(telstrat)
	ROM_REGION(0x4000, "telmatic", 0)
	ROM_LOAD ("telmatic.rom", 0, 0x2000, CRC(94358dc6) SHA1(35f92a0477a88f5cf564971125047ffcfa02ec10) )
	ROM_RELOAD (0x2000, 0x2000)

	ROM_REGION(0x4000, "teleass", 0)
	ROM_LOAD ("teleass.rom",  0, 0x4000, CRC(68b0fde6) SHA1(9e9af51dae3199cccf49ab3f0d47e2b9be4ba97d) )

	ROM_REGION(0x4000, "hyperbas", 0)
	ROM_LOAD ("hyperbas.rom", 0, 0x4000, CRC(1d96ab50) SHA1(f5f70a0eb59f8cd6c261e179ae78ef906f68ed63) )

	ROM_REGION(0x4000, "telmon24", 0)
	ROM_LOAD ("telmon24.rom", 0, 0x4000, CRC(aa727c5d) SHA1(86fc8dc0932f983efa199e31ae05a4424772f959) )
ROM_END

ROM_START(prav8d)
	ROM_REGION(0x4000, "maincpu", 0)   /* 0x10000 + 0x04000 + 0x00100 + 0x00200 */
	ROM_LOAD( "pravetzt.rom", 0, 0x4000, CRC(58079502) SHA1(7afc276cb118adff72e4f16698f94bf3b2c64146) )
//  ROM_LOAD_OPTIONAL( "8ddoslo.rom", 0x014000, 0x0100, CRC(0c82f636) SHA1(b29d151a0dfa3c7cd50439b51d0a8f95559bc2b6) )
//  ROM_LOAD_OPTIONAL( "8ddoshi.rom", 0x014100, 0x0200, CRC(66309641) SHA1(9c2e82b3c4d385ade6215fcb89f8b92e6fd2bf4b) )
ROM_END

ROM_START(prav8dd)
	ROM_REGION(0x4000, "maincpu", 0)   /* 0x10000 + 0x04000 + 0x00100 + 0x00200 */
	ROM_SYSTEM_BIOS( 0, "default", "Disk ROM, 1989")
	ROMX_LOAD( "8d.rom",       0, 0x4000, CRC(b48973ef) SHA1(fd47c977fc215a3b577596a7483df53e8a1e9c83), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "radosoft", "RadoSoft Disk ROM, 1992")
	ROMX_LOAD( "pravetzd.rom", 0, 0x4000, CRC(f8d23821) SHA1(f87ad3c5832773b6e0614905552a80c98dc8e2a5), ROM_BIOS(1) )
//  ROM_LOAD_OPTIONAL( "8ddoslo.rom", 0x014000, 0x0100, CRC(0c82f636) SHA1(b29d151a0dfa3c7cd50439b51d0a8f95559bc2b6) )
//  ROM_LOAD_OPTIONAL( "8ddoshi.rom", 0x014100, 0x0200, CRC(66309641) SHA1(9c2e82b3c4d385ade6215fcb89f8b92e6fd2bf4b) )
ROM_END


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS            INIT        COMPANY      FULLNAME                 FLAGS
COMP( 1983, oric1,    0,      0,      oric,     oric,     oric_state,      empty_init, "Tangerine", "Oric 1" ,               0 )
COMP( 1984, orica,    oric1,  0,      oric,     orica,    oric_state,      empty_init, "Tangerine", "Oric Atmos" ,           0 )
COMP( 1985, prav8d,   oric1,  0,      prav8d,   prav8d,   oric_state,      empty_init, "Pravetz",   "Pravetz 8D",            0 )
COMP( 1989, prav8dd,  oric1,  0,      prav8d,   prav8d,   oric_state,      empty_init, "Pravetz",   "Pravetz 8D (Disk ROM)", MACHINE_UNOFFICIAL )
COMP( 1986, telstrat, oric1,  0,      telstrat, telstrat, telestrat_state, empty_init, "Tangerine", "Oric Telestrat",        0 )
