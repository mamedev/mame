// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Amstrad PenPad PDA 600

    PCB Layout (front):
         /-----------------------------------------------------------------------\
        /                U4                                          +---+        \
       /          X1  HD64610FP       U9       U6                    | C |         \
      /       U3                    HC157A   HC157A        U12       | N |          \
     |    KM681000ALT-8                                 HD64646FS    | 3 |           |
     |                                                               +---+           |
     |    U36          U2                                                            |
     |  TL061AC       42069                                      U35       U27      /
     |                                          U5             TCM5089   LP324M    /
     | +---+                                   41857                              |
     | | C |                 U1                            U31         U33        |
     | | N |             Z8S18016FSC             U43      41863    KM62256BLG-10  |
     | | 2 |                                    HC541                             |
     | +---+                                                          U32          \
     |        U30                                               U37  HC574A         \
     |     MAX222CWN                                   U24     HC20           +---+  |
     |                                                41864                   | C |  |
     |     U21           U25                                                  | N |  |
     |  MAX731CWE       MAX641                                                | 4 |  |
     |                                                                        +---+  |
     +-------------------------------------------------------------------------------+

    PCB Layout (back):
     +-------------------------------------------------------------------------------+
     |  BAT-           +-----+                                                 BAT+  |
     |                /       \                                                      |
     |               |   3V    |                               U23                   |
     |               |   Bat.  |                              HC00A        U20       |
     |                \       /      +------+       U26                  14052B      |
     |                 +-----+       |      |     3226NUT                           /
     |       U10        U41          |      |                                      /
     |     HC245A      HC541         |      |                     U28             |
     |                          X2   |      |                  MC145053D          |
     |                  U42          | CN1  |                                     |
     |   U22           HC541         |      |                                     |
     | 7673CBA                       |      |                                      \
     |          +----+               |      |                                       \
     |         / Beep \              |      |                                        |
     |         \      /              +------+                                        |
     |          +----+                           U11                                 |
      \        U18         U8      U7         KM62256BLG                            /
       \      HC00A      HC157A  HC157A                                            /
        \                                                                         /
         \-----------------------------------------------------------------------/


    Parts:
     U2  - Amstrad 42069 ROM, probably 27C1001
     U5  - Amstrad 41857 100 Pin, unknown
     U31 - Amstrad 41863 44 Pin, unknown
     U24 - Amstrad 41864 18 Pin, unknown
     X1  - 32768 XTAL
     X2  - 28.6MHz XTAL

    Connectors:
     CN1 - PCMCIA
     CN2 - 8 Pin Serial
     CN3 - LCD
     CN4 - Digitizer

    TODO:
    - Sound (DTMF tone generator).
    - Refactor the HD64646FS LCD controller into a device.
    - Dump the character recognition MCU (possible?).
    - Serial port doesn't work.

****************************************************************************/

#include "emu.h"

#include "pda600_copro.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/rs232/rs232.h"
#include "cpu/z180/z180.h"
#include "machine/nvram.h"
#include "machine/hd64610.h"
#include "machine/timer.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "pda600.lh"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

static constexpr u32 PDA600_SCREEN_X = 38;
static constexpr u32 PDA600_SCREEN_Y = 54;
static constexpr u32 PDA600_SCREEN_W = (PDA600_SCREEN_X * 2 + 240);
static constexpr u32 PDA600_SCREEN_H = (PDA600_SCREEN_Y * 2 + 320);
static constexpr u32 PDA600_CSIO_RATE = 48000;
static constexpr u32 PDA600_SERIAL_PORT_RATE = 9600;


namespace {

class pda600_state : public driver_device
{
public:
	pda600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_copro(*this, "copro")
		, m_card(*this, "pcmcia")
		, m_beep(*this, "beeper")
		, m_pen(*this, {"PEN", "PENX", "PENY"})
		, m_battery(*this, "BATTERY")
		, m_video_ram(*this, "videoram")
	{
	}

	void pda600(machine_config &config);
	void power_off_w(int state) { m_maincpu->set_input_line(Z180_INPUT_LINE_IRQ1, state); }

private:
	required_device<z180_device> m_maincpu;
	required_device<pda600_copro_device> m_copro;
	required_device<generic_slot_device> m_card;
	required_device<beep_device> m_beep;
	required_ioport_array<3> m_pen;
	required_ioport m_battery;
	required_shared_ptr<u8> m_video_ram;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_w(offs_t offset, u8 data);
	u8 io_r(offs_t offset);
	void pcmcia_w(offs_t offset, u8 data);
	u8 pcmcia_r(offs_t offset);
	void tone_w(u8 data);
	TIMER_CALLBACK_MEMBER(csio_clk_timer);
	TIMER_CALLBACK_MEMBER(serl_clk_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(pen_update_timer);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(card_load);
	DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER(card_unload);

	void pda600_io(address_map &map) ATTR_COLD;
	void pda600_mem(address_map &map) ATTR_COLD;

	emu_timer * m_csio_clk_timer;
	emu_timer * m_serl_clk_timer;
	u8   m_pen_data[6]  = {};
	u8   m_pen_shift    = 0;
	u8   m_pen_cnt      = 0;
	u32  m_card_size    = 0;
	u8   m_rtc_irq      = 0;
	u8   m_serl_clk     = 0;
	u8   m_io_regs[4]   = {};
	u8   m_lcd_ar       = 0;
	u8   m_lcd_regs[32] = {};
};


void pda600_state::tone_w(u8 data)
{
	// xxxx ---- TCM5089 Column 1-4
	// ---- xxxx TCM5089 Row 1-4

	// TODO: DTMF tone encoder

	if (data & 0x7f)
		popmessage("DTMF tone %02X", data);

	m_beep->set_state(BIT(data, 7));
}


u8 pda600_state::io_r(offs_t offset)
{
	return m_io_regs[offset];
}


void pda600_state::io_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0:
		// xx-- ---- PCMCIA bank select
		// ---x -x-- Serial
		if (BIT(m_io_regs[offset] ^ data, 4))
		{
			auto period = attotime::zero;
			if (BIT(data, 4))
				period = attotime::from_hz(PDA600_SERIAL_PORT_RATE * 16 * 2);

			m_serl_clk_timer->adjust(period, 0, period);
		}
		break;
	case 1:
		break;
	case 2:
		// ---- xxxx LCD contrast level
		// ---x ---- Reset the coprocessor
		// --x- ---- Wake up the coprocessor
		if ((m_io_regs[offset] ^ data) & 0x0f)
			logerror("Set LCD contrast level: %d\n", data & 0x0f);

		m_copro->wakeup_w(BIT(data, 4));
		m_copro->reset_w(BIT(data, 5));
		break;
	case 3:
		// --X- ---- ??
		break;
	}

	m_io_regs[offset] = data;
}


u8 pda600_state::pcmcia_r(offs_t offset)
{
	offset |= (u32)(m_io_regs[0] & 0xc0) << 13;
	if (offset < m_card_size)
		return m_card->read_ram(offset);

	return 0xff;
}


void pda600_state::pcmcia_w(offs_t offset, u8 data)
{
	offset |= (u32)(m_io_regs[0] & 0xc0) << 13;
	if (offset < m_card_size)
		m_card->write_ram(offset, data);
}


void pda600_state::pda600_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x9ffff).rw(FUNC(pda600_state::pcmcia_r), FUNC(pda600_state::pcmcia_w));
	map(0xa0000, 0xa7fff).ram().share("videoram");
	map(0xe0000, 0xfffff).ram().share("nvram");
}

void pda600_state::pda600_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x3f).noprw(); /* Z180 internal registers */
	map(0x40, 0x43).rw(FUNC(pda600_state::io_r), FUNC(pda600_state::io_w));
	map(0x80, 0x8f).rw("rtc", FUNC(hd64610_device::read), FUNC(hd64610_device::write));
	map(0xc0, 0xc0).lw8(NAME([this](u8 data) { m_lcd_ar = data & 0x1f; }));
	map(0xc1, 0xc1).lw8(NAME([this](u8 data) { m_lcd_regs[m_lcd_ar] = data; }));
}

/* Input ports */
static INPUT_PORTS_START( pda600 )
	PORT_START("POWER")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_WRITE_LINE_MEMBER(pda600_state, power_off_w)

	PORT_START("BATTERY")
	PORT_CONFNAME(0x0f, 0x0f, "Main battery status")
	PORT_CONFSETTING(0x0f, "Good")
	PORT_CONFSETTING(0x0c, "Fair")
	PORT_CONFSETTING(0x06, "Low")
	PORT_CONFSETTING(0x00, "Replace")
	PORT_CONFNAME(0x10, 0x00, "Backup battery status")
	PORT_CONFSETTING(0x00, "Good")
	PORT_CONFSETTING(0x10, "Replace")

	PORT_START("PENX")
	PORT_BIT(0x3ff, 0x000, IPT_LIGHTGUN_X) PORT_SENSITIVITY(40) PORT_CROSSHAIR(X, 1, 0, 0) PORT_MINMAX(0, 0x3ff) PORT_KEYDELTA(1)

	PORT_START("PENY")
	PORT_BIT(0x3ff, 0x000, IPT_LIGHTGUN_Y) PORT_SENSITIVITY(40) PORT_CROSSHAIR(Y, 1, 0, 0) PORT_MINMAX(0, 0x3ff) PORT_KEYDELTA(1)

	PORT_START("PEN")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Pen")
INPUT_PORTS_END


void pda600_state::machine_start()
{
	// state saving
	save_item(NAME(m_pen_data));
	save_item(NAME(m_pen_shift));
	save_item(NAME(m_pen_cnt));
	save_item(NAME(m_card_size));
	save_item(NAME(m_rtc_irq));
	save_item(NAME(m_serl_clk));
	save_item(NAME(m_io_regs));
	save_item(NAME(m_lcd_ar));
	save_item(NAME(m_lcd_regs));

	m_csio_clk_timer = timer_alloc(FUNC(pda600_state::csio_clk_timer), this);
	m_serl_clk_timer = timer_alloc(FUNC(pda600_state::serl_clk_timer), this);
}


void pda600_state::machine_reset()
{
	m_pen_shift = -1;
	m_pen_cnt   = 0;
	m_lcd_ar    = 0;
	m_rtc_irq   = 0;
	m_serl_clk  = 0;
	std::fill(std::begin(m_pen_data), std::end(m_pen_data), 0U);
	std::fill(std::begin(m_io_regs), std::end(m_io_regs), 0U);
	std::fill(std::begin(m_lcd_regs), std::end(m_lcd_regs), 0U);
}


u32 pda600_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(1);

	if (!BIT(m_lcd_regs[0x16], 4))
		return 0;

	const u16 width  = (m_lcd_regs[0x11] << 8) | m_lcd_regs[0x12];
	const u16 height = (m_lcd_regs[0x13] << 8) | m_lcd_regs[0x14];

	for (int y = 0; y <= height; y++)
		for (int x = 0; x < width; x++)
		{
			const s32 dst_y = PDA600_SCREEN_Y + y;
			const s32 dst_x = PDA600_SCREEN_X + x * 8;
			u8 data = m_video_ram[y * m_lcd_regs[1] + x] ^ 0xff;

			for (int px = 0; px < 8; px++)
			{
				if (cliprect.contains(dst_x + px, dst_y))
					bitmap.pix(dst_y, dst_x + px) = BIT(data, 7);

				data <<= 1;
			}
		}

	return 0;
}

static const gfx_layout pda600_charlayout_8 =
{
	8, 8,
	49,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout pda600_charlayout_13 =
{
	8, 13,
	123,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 8*9, 8*10, 8*11, 8*12 },
	8*13
};

static const gfx_layout pda600_charlayout_13a =
{
	8, 13,
	132,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 8*9, 8*10, 8*11, 8*12 },
	8*13
};

static const gfx_layout pda600_charlayout_19 =
{
	8, 19,
	32,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 8*9, 8*10, 8*11, 8*12, 8*13, 8*14, 8*15, 8*16, 8*17, 8*18 },
	8*19
};

static const gfx_layout pda600_charlayout_19a =
{
	8, 19,
	11,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 8*9, 8*10, 8*11, 8*12, 8*13, 8*14, 8*15, 8*16, 8*17, 8*18 },
	8*19
};

static GFXDECODE_START( gfx_pda600 )
	GFXDECODE_ENTRY( "maincpu", 0x45cd, pda600_charlayout_19, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x4892, pda600_charlayout_19a, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x4d73, pda600_charlayout_8, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x5b8f, pda600_charlayout_13, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x61d3, pda600_charlayout_13a, 0, 1 )
GFXDECODE_END


TIMER_DEVICE_CALLBACK_MEMBER(pda600_state::pen_update_timer)
{
	u8 pen = m_pen[0]->read();

	// Reduce the update rate when the pen is not held down
	if (!pen && ++m_pen_cnt < 20)
		return;
	else
		m_pen_cnt = 0;

	m_pen_data[0] = 0x80;               // Start of new data
	m_pen_data[0] |= pen << 6;          // Pen up/down
	m_pen_data[0] |= m_rtc_irq << 5;    // RTC IRQ status
	m_pen_data[0] |= m_battery->read(); // Battery status

	// Pen position (updated only when the pen is down)
	if (pen)
	{
		u16 penx = m_pen[1]->read();
		u16 peny = m_pen[2]->read();
		m_pen_data[1] = penx & 0x7f;
		m_pen_data[2] = (penx >> 7) & 0x7f;
		m_pen_data[3] = peny & 0x7f;
		m_pen_data[4] = (peny >> 7) & 0x7f;
	}

	// Data checksum
	m_pen_data[5] = m_pen_data[0] + m_pen_data[1] + m_pen_data[2] + m_pen_data[3] + m_pen_data[4];

	// Start CSIO clock
	m_pen_shift = 0;
	m_csio_clk_timer->adjust(attotime::zero);
}


TIMER_CALLBACK_MEMBER(pda600_state::csio_clk_timer)
{
	if (m_pen_shift < 48)
	{
		m_maincpu->cks_w(0);
		m_maincpu->rxs_cts1_w(BIT(m_pen_data[m_pen_shift / 8], m_pen_shift & 7));
		m_maincpu->cks_w(1);
		m_pen_shift++;

		// If there is still data to send, reschedule the timer
		if (m_pen_shift < 48)
		{
			auto delay = attotime::from_hz(PDA600_CSIO_RATE);

			// A delay is added after each byte to allow the maincpu to read the transmitted data
			if (!(m_pen_shift & 7))
				delay += attotime::from_usec(150);

			m_csio_clk_timer->adjust(delay);
		}
	}
}


TIMER_CALLBACK_MEMBER(pda600_state::serl_clk_timer)
{
	// External clock for the Z180 ASCI0
	m_serl_clk ^= 1;
	m_maincpu->cka0_w(m_serl_clk);
}


DEVICE_IMAGE_LOAD_MEMBER(pda600_state::card_load)
{
	if (!image.loaded_through_softlist())
	{
		const u64 size = image.length();
		m_card->ram_alloc(size);

		if (size != image.fread(m_card->get_ram_base(), size))
			return std::make_pair(image_error::UNSPECIFIED, std::string());

		m_card_size = size;
	}
	else
	{
		m_card_size = image.get_software_region_length("rom");

		if (m_card_size == 0)
			return std::make_pair(image_error::BADSOFTWARE, "rom data area is missing or empty");

		m_card->ram_alloc(m_card_size);
		memcpy(m_card->get_ram_base(), image.get_software_region("rom"), m_card_size);
	}

	m_card->battery_load(m_card->get_ram_base(), m_card_size, nullptr);
	return std::make_pair(std::error_condition(), std::string());
}


DEVICE_IMAGE_UNLOAD_MEMBER(pda600_state::card_unload)
{
	m_card->battery_save(m_card->get_ram_base(), m_card_size);
	memset(m_card->get_ram_base(), 0xff, m_card_size);
	m_card_size = 0;
}


void pda600_state::pda600(machine_config &config)
{
	/* basic machine hardware */
	Z8S180(config, m_maincpu, 28'636'363_Hz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &pda600_state::pda600_mem);
	m_maincpu->set_addrmap(AS_IO, &pda600_state::pda600_io);
	m_maincpu->txa0_wr_callback().set("serial", FUNC(rs232_port_device::write_txd));
	m_maincpu->rts0_wr_callback().set("serial", FUNC(rs232_port_device::write_rts));
	m_maincpu->txa1_wr_callback().set(m_copro, FUNC(pda600_copro_device::write_txd));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(PDA600_SCREEN_W, PDA600_SCREEN_H);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(pda600_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_pda600);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	// NVRAM needs to be filled with random data to fail the checksum and be initialized correctly
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	hd64610_device &rtc(HD64610(config, "rtc", 32.768_kHz_XTAL));
	rtc.irq().set([this](int state) { m_rtc_irq = state; }).invert();

	TIMER(config, "pen_update_timer").configure_periodic(FUNC(pda600_state::pen_update_timer), attotime::from_hz(100));

	GENERIC_CARTSLOT(config, m_card, generic_romram_plain_slot, "pda600", "bin");
	m_card->set_device_load(FUNC(pda600_state::card_load));
	m_card->set_device_unload(FUNC(pda600_state::card_unload));
	m_card->set_interface("pcmcia");

	PDA600_COPRO_HLE(config, m_copro, 28'636'363_Hz_XTAL / 2);
	m_copro->tx_callback().set(m_maincpu, FUNC(z180_device::rxa1_w));
	m_copro->tone_callback().set(FUNC(pda600_state::tone_w));

	rs232_port_device &rs232(RS232_PORT(config, "serial", default_rs232_devices, "printer"));
	rs232.rxd_handler().set(m_maincpu, FUNC(z180_device::rxa0_w));
	rs232.cts_handler().set(m_maincpu, FUNC(z180_device::cts0_w));
	rs232.cts_handler().append_inputline(m_maincpu, Z180_INPUT_LINE_DREQ0).invert();

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1633).add_route(ALL_OUTPUTS, "mono", 0.80);    // TODO: replace with TCM5089

	// software lists
	SOFTWARE_LIST(config, "card_list").set_original("pda600");

	config.set_default_layout(layout_pda600);
}

/* ROM definition */
ROM_START( pda600 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pdarom.bin", 0x00000, 0x20000, CRC(f793a6c5) SHA1(ab14b0fdcedb927c66357368a2bfff605ba758fb))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY        FULLNAME          FLAGS */
COMP( 1993, pda600, 0,      0,      pda600,  pda600, pda600_state, empty_init, "Amstrad plc", "PenPad PDA 600", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
