// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Sergey Svishchev
/***************************************************************************

    Elektronika MS 0515

    To do:
    - softlist
    . sound
    - overscan color
    - serial printer
    - ?? 512K memory expansion
    - ?? refresh rate change
    - ?? parallel printer
    - ?? cassette (only with Version A firmware)
    - ?? port 177770
    - ?? mc1702 (8086 co-processor)

    Docs:
    - http://www.tis.kz/docs/MC-0515/mc0515-ed.rar schematics etc.
    - http://www.tis.kz/docs/MC-0515/mc0515-to.rar user manual
    - http://www.tis.kz/docs/MC-0515/hc4-to.rar technical manual
    - http://www.tis.kz/docs/MC-0515/mc0515-po.rar diag manual
    - http://www.tis.kz/docs/MC-0515/mc0515-osa.rar OS manual

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/t11/t11.h"
#include "imagedev/floppy.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/ms7004.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"

#include "formats/ms0515_dsk.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "ms0515.lh"


#define LOG_GENERAL (1U <<  0)
#define LOG_BANK    (1U <<  1)
#define LOG_SYSREG  (1U <<  2)

//#define VERBOSE (LOG_GENERAL | LOG_BANK | LOG_SYSREG)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGBANK(format, ...)    LOGMASKED(LOG_BANK,   "%11.6f at %s: " format, machine().time().as_double(), machine().describe_context(), __VA_ARGS__)
#define LOGSYSREG(format, ...)  LOGMASKED(LOG_SYSREG, "%11.6f at %s: " format, machine().time().as_double(), machine().describe_context(), __VA_ARGS__)


namespace {

class ms0515_state : public driver_device
{
public:
	ms0515_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_fdc(*this, "vg93")
		, m_floppies(*this, "vg93:%u", 0U)
		, m_i8251line(*this, "i8251line")
		, m_rs232(*this, "rs232")
		, m_i8251kbd(*this, "i8251kbd")
		, m_ms7004(*this, "ms7004")
		, m_pit8253(*this, "pit8253")
		, m_speaker(*this, "speaker")
		, m_bank(*this, "bank%u", 0U)
		, m_led9(*this, "led9")
		, m_led16(*this, "led16")
		, m_led17(*this, "led17")
	{ }

	void ms0515(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void ms0515_palette(palette_device &palette) const;
	uint32_t screen_update_ms0515(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);

	void ms0515_bank_w(uint16_t data);

	uint16_t ms0515_halt_r();
	void ms0515_halt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void ms0515_porta_w(uint8_t data);
	uint8_t ms0515_portb_r();
	void ms0515_portc_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(write_keyboard_clock);
	DECLARE_WRITE_LINE_MEMBER(write_line_clock);
	DECLARE_WRITE_LINE_MEMBER(pit8253_out2_changed);

	static void floppy_formats(format_registration &fr);

	DECLARE_WRITE_LINE_MEMBER(irq2_w);
	DECLARE_WRITE_LINE_MEMBER(irq5_w);
	DECLARE_WRITE_LINE_MEMBER(irq8_w);
	DECLARE_WRITE_LINE_MEMBER(irq9_w);
	DECLARE_WRITE_LINE_MEMBER(irq11_w);

	void ms0515_mem(address_map &map);

	void irq_encoder(int irq, int state);

	required_device<t11_device> m_maincpu; // actual CPU is T11 clone, KR1807VM1
	required_device<ram_device> m_ram;
	required_device<kr1818vg93_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppies;
	required_device<i8251_device> m_i8251line;
	required_device<rs232_port_device> m_rs232;
	required_device<i8251_device> m_i8251kbd;
	required_device<ms7004_device> m_ms7004;
	required_device<pit8253_device> m_pit8253;
	required_device<speaker_sound_device> m_speaker;
	required_memory_bank_array<7> m_bank;
	output_finder<> m_led9;
	output_finder<> m_led16;
	output_finder<> m_led17;

	uint8_t *m_video_ram;
	uint8_t m_sysrega, m_sysregc;
	uint16_t m_bankreg, m_haltreg;
	uint16_t m_irqs;
	int m_blink;
	floppy_image_device *m_floppy;
};

void ms0515_state::ms0515_mem(address_map &map)
{
	map.unmap_value_high();
	map(0000000, 0017777).bankrw("bank0"); // RAM
	map(0020000, 0037777).bankrw("bank1"); // RAM
	map(0040000, 0057777).bankrw("bank2"); // RAM
	map(0060000, 0077777).bankrw("bank3"); // RAM
	map(0100000, 0117777).bankrw("bank4"); // RAM
	map(0120000, 0137777).bankrw("bank5"); // RAM
	map(0140000, 0157777).bankrw("bank6"); // RAM

	map(0160000, 0177377).rom().nopw();

	map(0177400, 0177437).w(FUNC(ms0515_state::ms0515_bank_w)); // Register for RAM expansion

	map(0177440, 0177440).r(m_i8251kbd, FUNC(i8251_device::data_r));
	map(0177442, 0177442).rw(m_i8251kbd, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0177460, 0177460).w(m_i8251kbd, FUNC(i8251_device::data_w));
	map(0177462, 0177462).w(m_i8251kbd, FUNC(i8251_device::control_w));

	map(0177500, 0177507).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0177520, 0177527).w(m_pit8253, FUNC(pit8253_device::write)).umask16(0x00ff);

	map(0177540, 0177547).noprw();
//  map(0177540, 0177541)
//  map(0177542, 0177543)
//  map(0177544, 0177545)  // i8255 for MS-7007 Keyboard
//  map(0177546, 0177547)

	map(0177600, 0177607).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);

	map(0177640, 0177640).rw(m_fdc, FUNC(kr1818vg93_device::status_r), FUNC(kr1818vg93_device::cmd_w));
	map(0177642, 0177642).rw(m_fdc, FUNC(kr1818vg93_device::track_r), FUNC(kr1818vg93_device::track_w));
	map(0177644, 0177644).rw(m_fdc, FUNC(kr1818vg93_device::sector_r), FUNC(kr1818vg93_device::sector_w));
	map(0177646, 0177646).rw(m_fdc, FUNC(kr1818vg93_device::data_r), FUNC(kr1818vg93_device::data_w));

	map(0177700, 0177700).r(m_i8251line, FUNC(i8251_device::data_r));
	map(0177702, 0177702).rw(m_i8251line, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0177720, 0177720).w(m_i8251line, FUNC(i8251_device::data_w));
	map(0177722, 0177722).w(m_i8251line, FUNC(i8251_device::control_w));

	map(0177770, 0177771).rw(FUNC(ms0515_state::ms0515_halt_r), FUNC(ms0515_state::ms0515_halt_w)); // read/write -- halt and system timer
}

/*
 * (page 15-16)
 *
 * 6-0  RAM banking
 * 7    VRAM access enable
 * 8    vblank IRQ line (1 -- assert)
 * 9    timer IRQ enable (1 -- enable)
 * 11-10 VRAM banking
 * 12   parallel port STROBE signal
 * 13   parallel port ... signal
 * 14-15 unused
 */
void ms0515_state::ms0515_bank_w(uint16_t data)
{
	uint8_t *ram = m_ram->pointer();

	LOGBANK("Bank <- %04x (vblank %d timer %d)\n", data, BIT(data, 8), BIT(data, 9));

	if (BIT(data ^ m_bankreg, 8)) irq2_w(BIT(data, 8) ? ASSERT_LINE : CLEAR_LINE);

	m_bankreg = data;

	m_bank[0]->set_base(ram + 0000000 + BIT(data, 0) * 0160000);
	m_bank[1]->set_base(ram + 0020000 + BIT(data, 1) * 0160000);
	m_bank[2]->set_base(ram + 0040000 + BIT(data, 2) * 0160000);
	m_bank[3]->set_base(ram + 0060000 + BIT(data, 3) * 0160000);
	m_bank[4]->set_base(ram + 0100000 + BIT(data, 4) * 0160000);
	m_bank[5]->set_base(ram + 0120000 + BIT(data, 5) * 0160000);
	m_bank[6]->set_base(ram + 0140000 + BIT(data, 6) * 0160000);

	if (BIT(data, 7))
	{
		switch ((data >> 10) & 3)
		{
		case 0: // 000000 - 037777
			m_bank[0]->set_base(ram + 0000000 + 0340000);
			m_bank[1]->set_base(ram + 0020000 + 0340000);
			break;
		case 1: // 040000 - 077777
			m_bank[2]->set_base(ram + 0000000 + 0340000);
			m_bank[3]->set_base(ram + 0020000 + 0340000);
			break;
		case 2:
		case 3: // 100000 - 137777
			m_bank[4]->set_base(ram + 0000000 + 0340000);
			m_bank[5]->set_base(ram + 0020000 + 0340000);
			break;
		}
	}
}

uint16_t ms0515_state::ms0515_halt_r()
{
	return m_haltreg;
}

void ms0515_state::ms0515_halt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_haltreg);
}

/*
 * b7 -- ROM bank
 * b6 -- cassette data out
 * b5 -- LED VD16
 * b4 -- LED VD9
 * b3 -- floppy side select (?? 1 -- top)
 * b2 -- floppy motor (0 -- on)
 * b1-0 -- floppy drive select
 *
 * DZ0 = drive 0 side 0 (bottom)
 * DZ1 = drive 1 side 0 (bottom)
 * DZ2 = drive 0 side 1 (top)
 * DZ3 = drive 1 side 1 (top)
 *
 * MZ1 = drive 1 side 0-1
 */
void ms0515_state::ms0515_porta_w(uint8_t data)
{
	LOGSYSREG("Sysreg A <- %02x\n", data);

	m_led16 = BIT(data, 5);
	m_led9 = BIT(data, 4);

	switch (data & 3)
	{
	case 0:
		m_floppy = m_floppies[0]->get_device();
		break;

	case 1:
		m_floppy = m_floppies[1]->get_device();
		break;

	default:
		m_floppy = nullptr;
		break;
	}

	if (m_floppy)
	{
		m_fdc->set_floppy(m_floppy);
		m_floppy->ss_w(!BIT(data, 3));
		m_floppy->mon_w(BIT(data, 2));
	}
	else
	{
		for (auto &floppy : m_floppies)
			if (floppy->get_device() != nullptr)
				floppy->get_device()->mon_w(1);
	}

	m_sysrega = data;
}

/*
 * b7 -- cassette data in
 * b6-5 -- reserved for IRPR-M (parallel) port
 * b4-3 -- DIP switches on video board, 00 -- 50 Hz, 01 -- 60 Hz, 1x -- 72 Hz
 * b2 -- floppy ready signal (0 -- ready)
 * b1 -- floppy drq (1 -- ready)
 * b0 -- floppy intrq (0 -- ready)
 */
uint8_t ms0515_state::ms0515_portb_r()
{
	uint8_t data;

	data = m_fdc->intrq_r();
	data |= m_fdc->drq_r() << 1;

	if (m_floppy)
	{
		data |= !m_floppy->ready_r() << 2;
	}

	LOGSYSREG("Sysreg B == %02x\n", data);

	return data;
}


/*
 * b7 -- sound out gate
 * b6 -- sound out route to speaker
 * b5 -- sound ??
 * b4 -- LED VD17
 * b3 -- video resolution, 0: 320x200, 1: 640x200
 * b2-0 -- overscan color
 */
void ms0515_state::ms0515_portc_w(uint8_t data)
{
	LOGSYSREG("Sysreg C <- %02x\n", data);

	m_pit8253->write_gate2(BIT(data, 7));
	m_led17 = BIT(data, 4);

	m_sysregc = data;
}

WRITE_LINE_MEMBER(ms0515_state::write_keyboard_clock)
{
	m_i8251kbd->write_txc(state);
	m_i8251kbd->write_rxc(state);
}

WRITE_LINE_MEMBER(ms0515_state::write_line_clock)
{
	m_i8251line->write_txc(state);
	m_i8251line->write_rxc(state);
}

WRITE_LINE_MEMBER(ms0515_state::pit8253_out2_changed)
{
	m_speaker->level_w(state);
}

void ms0515_state::machine_start()
{
	m_led9.resolve();
	m_led16.resolve();
	m_led17.resolve();
}

void ms0515_state::machine_reset()
{
	uint8_t *ram = m_ram->pointer();
	ms0515_bank_w(0);

	m_video_ram = ram + 0000000 + 0340000;
	m_blink = 0;
	m_haltreg = 0;
	m_irqs = 0;
	m_floppy = nullptr;
}

/* Input ports */
static INPUT_PORTS_START( ms0515 )
	PORT_START("SA1")
	PORT_DIPNAME(0x03, 0x00, "Refresh rate") PORT_DIPLOCATION("E:3,4")
	PORT_DIPSETTING(0x00, "50 Hz")
	PORT_DIPSETTING(0x01, "60 Hz")
	PORT_DIPSETTING(0x02, "72 Hz")
INPUT_PORTS_END

void ms0515_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_MS0515_FORMAT);
}

static void ms0515_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

uint32_t ms0515_state::screen_update_ms0515(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int addr = 0;

	if (BIT(m_sysregc, 3))
	{
		uint8_t fg = m_sysregc & 7;
		uint8_t bg = fg ^ 7;
		for (int y = 0; y < 200; y++)
		{
			int horpos = 0;
			for (int x = 0; x < 40; x++)
			{
				uint16_t code = (m_video_ram[addr++] << 8);
				code |= m_video_ram[addr++];
				for (int b = 0; b < 16; b++)
				{
					// In lower res mode we will just double pixels
					bitmap.pix(y, horpos++) = ((code >> (15 - b)) & 0x01) ? bg : fg;
				}
			}
		}
	}
	else
	{
		for (int y = 0; y < 200; y++)
		{
			int horpos = 0;
			for (int x = 0; x < 40; x++)
			{
				uint8_t code = m_video_ram[addr++];
				uint8_t attr = m_video_ram[addr++];
				uint8_t fg = (attr & 7) + BIT(attr, 6) * 8;
				uint8_t bg = ((attr >> 3) & 7) + BIT(attr, 6) * 8;
				if (BIT(attr, 7) && (m_blink == 20))
				{
					uint8_t tmp = fg;
					fg = bg;
					bg = tmp;
					m_blink = -1;
				}
				for (int b = 0; b < 8; b++)
				{
					// In lower res mode we will just double pixels
					bitmap.pix(y, horpos++) = ((code >> (7 - b)) & 0x01) ? fg : bg;
					bitmap.pix(y, horpos++) = ((code >> (7 - b)) & 0x01) ? fg : bg;
				}
			}
		}
	}
	m_blink++;
	return 0;
}

WRITE_LINE_MEMBER(ms0515_state::screen_vblank)
{
//  irq2_w(state ? ASSERT_LINE : CLEAR_LINE);
	if (BIT(m_bankreg, 9))
		irq11_w(state ? ASSERT_LINE : CLEAR_LINE);
}

void ms0515_state::ms0515_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0, 0, 0));
	palette.set_pen_color(1, rgb_t(0, 0, 127));
	palette.set_pen_color(2, rgb_t(127, 0, 0));
	palette.set_pen_color(3, rgb_t(127, 0, 127));
	palette.set_pen_color(4, rgb_t(0, 127, 0));
	palette.set_pen_color(5, rgb_t(0, 127, 127));
	palette.set_pen_color(6, rgb_t(127, 127, 0));
	palette.set_pen_color(7, rgb_t(127, 127, 127));

	palette.set_pen_color(8, rgb_t(127, 127, 127));
	palette.set_pen_color(9, rgb_t(127, 127, 255));
	palette.set_pen_color(10, rgb_t(255, 127, 127));
	palette.set_pen_color(11, rgb_t(255, 127, 255));
	palette.set_pen_color(12, rgb_t(127, 255, 127));
	palette.set_pen_color(13, rgb_t(127, 255, 255));
	palette.set_pen_color(14, rgb_t(255, 255, 127));
	palette.set_pen_color(15, rgb_t(255, 255, 255));
}

// from vt240.cpp
void ms0515_state::irq_encoder(int irq, int state)
{
	if (state == ASSERT_LINE)
		m_irqs |= (1 << irq);
	else
		m_irqs &= ~(1 << irq);

	int i;
	for (i = 15; i > 0; i--)
	{
		if (m_irqs & (1 << i)) break;
	}
	m_maincpu->set_input_line(t11_device::CP3_LINE, (i & 8) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP2_LINE, (i & 4) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP1_LINE, (i & 2) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(t11_device::CP0_LINE, (i & 1) ? ASSERT_LINE : CLEAR_LINE);
}

/*
 * interrupts (p. 21-22)
 *
 * IRQ  CPx  Pri Vec Device
 * ---  ---  --- --- ------
 * 11   LHLL 6   100 timer
 * 9    LHHL 6   110 serial RX
 * 8    LHHH 6   114 serial TX
 * 5    HLHL 5   130 7004 keyboard
 * 3    HHLL 4   060 7007 keyboard
 * 2    HHLH 4   064 vblank
 */

WRITE_LINE_MEMBER(ms0515_state::irq2_w)
{
	irq_encoder(2, state);
}

WRITE_LINE_MEMBER(ms0515_state::irq5_w)
{
	irq_encoder(5, state);
}

WRITE_LINE_MEMBER(ms0515_state::irq8_w)
{
	irq_encoder(8, state);
}

WRITE_LINE_MEMBER(ms0515_state::irq9_w)
{
	irq_encoder(9, state);
}

WRITE_LINE_MEMBER(ms0515_state::irq11_w)
{
	irq_encoder(11, state);
}

void ms0515_state::ms0515(machine_config &config)
{
	/* basic machine hardware */
	T11(config, m_maincpu, XTAL(15'000'000) / 2); // actual CPU is T11 clone, KR1807VM1
	m_maincpu->set_initial_mode(0xf2ff);
	m_maincpu->set_addrmap(AS_PROGRAM, &ms0515_state::ms0515_mem);

	/* video hardware -- 50 Hz refresh rate */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(15'000'000), 958, 0, 640, 313, 0, 200);
	screen.set_screen_update(FUNC(ms0515_state::screen_update_ms0515));
	screen.screen_vblank().set(FUNC(ms0515_state::screen_vblank));
	screen.set_palette("palette");
	config.set_default_layout(layout_ms0515);

	PALETTE(config, "palette", FUNC(ms0515_state::ms0515_palette), 16);

	KR1818VG93(config, m_fdc, 1000000);
	FLOPPY_CONNECTOR(config, "vg93:0", ms0515_floppies, "525qd", ms0515_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "vg93:1", ms0515_floppies, "525qd", ms0515_state::floppy_formats).enable_sound(true);

	i8255_device &ppi(I8255(config, "ppi8255_1"));
	ppi.out_pa_callback().set(FUNC(ms0515_state::ms0515_porta_w));
	ppi.in_pb_callback().set(FUNC(ms0515_state::ms0515_portb_r));
	ppi.out_pc_callback().set(FUNC(ms0515_state::ms0515_portc_w));

	// serial connection to printer
	I8251(config, m_i8251line, 0);
	m_i8251line->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_i8251line->rxrdy_handler().set(FUNC(ms0515_state::irq9_w));
	m_i8251line->txrdy_handler().set(FUNC(ms0515_state::irq8_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_i8251line, FUNC(i8251_device::write_rxd));
	m_rs232->cts_handler().set(m_i8251line, FUNC(i8251_device::write_cts));
	m_rs232->dsr_handler().set(m_i8251line, FUNC(i8251_device::write_dsr));

//  clock_device &line_clock(CLOCK(config, "line_clock", 4800*16)); // 8251 is set to /16 on the clock input
//  line_clock.signal_handler().set(FUNC(ms0515_state::write_line_clock));

	// serial connection to MS7004 keyboard
	I8251(config, m_i8251kbd, 0);
	m_i8251kbd->rxrdy_handler().set(FUNC(ms0515_state::irq5_w));
	m_i8251kbd->txd_handler().set("ms7004", FUNC(ms7004_device::write_rxd));

	MS7004(config, m_ms7004, 0);
	m_ms7004->tx_handler().set(m_i8251kbd, FUNC(i8251_device::write_rxd));
	m_ms7004->rts_handler().set(m_i8251kbd, FUNC(i8251_device::write_cts));

	// baud rate is supposed to be 4800 but keyboard is slightly faster
	clock_device &keyboard_clock(CLOCK(config, "keyboard_clock", 4960*16));
	keyboard_clock.signal_handler().set(FUNC(ms0515_state::write_keyboard_clock));

	PIT8253(config, m_pit8253, 0);
	m_pit8253->set_clk<0>(XTAL(2'000'000));
//  m_pit8253->out_handler<0>().set(FUNC(ms0515_state::write_keyboard_clock));
	m_pit8253->set_clk<1>(XTAL(2'000'000));
	m_pit8253->out_handler<0>().set(FUNC(ms0515_state::write_line_clock));
	m_pit8253->set_clk<2>(XTAL(2'000'000));
	m_pit8253->out_handler<2>().set(FUNC(ms0515_state::pit8253_out2_changed));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.45);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("128K");
}

/* ROM definition */
ROM_START( ms0515 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "b" )
	ROM_SYSTEM_BIOS( 0, "a", "Version A" )
	ROMX_LOAD( "7004l.bin", 0xc000, 0x2000, CRC(b08b3b73) SHA1(c12fd4672598cdf499656dcbb4118d787769d589), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "7004h.bin", 0xc001, 0x2000, CRC(515dcf99) SHA1(edd34300fd642c89ce321321e1b12493cd16b7a5), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "b", "Version B" )
	ROMX_LOAD( "0515l.rf4", 0xc000, 0x2000, CRC(85b608a4) SHA1(5b1bb0586d8f7a8a21de69200b08e0b28a318999), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "0515h.rf4", 0xc001, 0x2000, CRC(e3ff6da9) SHA1(3febccf40abc2e3ca7db3f6f3884be117722dd8b), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY        FULLNAME   FLAGS
COMP( 1990, ms0515, 0,      0,      ms0515,  ms0515, ms0515_state, empty_init, "Elektronika", "MS 0515", 0 )
