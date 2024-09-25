// license: BSD-3-Clause
// copyright-holders: Devin Acker

/*
    Casio/Tandy "Zoomer" PDA (1993)

    This early pen-based PDA was created jointly by Casio (who created the hardware and BIOS)
    and Tandy, with the built-in software provided by GeoWorks and Palm Computing.
    It was sold by both Casio (as the Z-7000) and Tandy (as the Z-PDA), as well as AST Research
    as the GRiDPad 2390.

    Main board:
    LSI101: Casio/NEC uPD95130GD (V20-based CPU)
    LSI102: Casio/Fujitsu MBCG25173-5104 (I/O controller)
    LSI201: Toshiba TC35083 (10-bit ADC for touch screen)
    LSI401: Analog Devices MAX223 (RS232 interface)
    LSI601: NEC uPD65043GF-U01 (sound chip, SN76489-like with PCM streaming)

    Memory daughterboard:
    LSI1, LSI2: Fujitsu MB838200AL (8Mbit mask ROM, 16.4mm TSOP)
    LSI3, LSI4: Fujitsu MB838200AL (8Mbit mask ROM, 16.4mm TSOP, reverse pinout)
    LSI5, LSI6: Hitachi HM65V8512 (4Mbit SRAM)

    TODO:
        - PCMCIA (requires 8-bit SRAM or flash cards with specific attribute memory contents)
          See other TODOs for where/how it should be hooked up, including the card lock switch
        - Serial and infrared ports
        - Make suspended unit state persist after pressing the power button and then quitting MAME?
          (ports 70, 90-af and c0-c7 must be saved as nvram for this to work)
*/

#include "emu.h"

#include "zoomer_rtc.h"

#include "cpu/nec/nec.h"
#include "machine/bankdev.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "sound/upd65043gfu01.h"

#include "crsshair.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "zoomer.lh"

namespace {

class zoomer_state : public driver_device
{
public:
	zoomer_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_bank_2000(*this, "bank_2000_%u", 0),
		m_bank_a000(*this, "bank_a000_%u", 0),
		m_wp_view{
				{ *this, "wp_view0" }, { *this, "wp_view1" }, { *this, "wp_view2" }, { *this, "wp_view3" },
				{ *this, "wp_view4" }, { *this, "wp_view5" }, { *this, "wp_view6" }, { *this, "wp_view7" },
				{ *this, "wp_view8" }, { *this, "wp_view9" }, { *this, "wp_view10" }, { *this, "wp_view11" },
				{ *this, "wp_view12" }, { *this, "wp_view13" }, { *this, "wp_view14" }, {*this, "wp_view15" } },
		m_rtc(*this, "rtc"),
		m_psg(*this, "psg"),
		m_nvram(*this, "nvram", 0x100000, endianness_t::little),
		m_pen(*this, "PEN"),
		m_pen_x(*this, "PEN_X"),
		m_pen_y(*this, "PEN_Y")
	{ }

	void zoomer(machine_config &config) ATTR_COLD;

	template <int Num>
	void irq_set(int state);

	void power_w(int state);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static inline constexpr unsigned TIMER_RATE = (32768 / 8);

	void maincpu_map(address_map &map) ATTR_COLD;
	void maincpu_ems_map(address_map &map) ATTR_COLD;
	void maincpu_io_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void apo_w(u8 data);

	u8 ems_2000_bank_r(offs_t offset);
	void ems_2000_bank_w(offs_t offset, u8 data);
	u8 ems_a000_bank_r(offs_t offset);
	void ems_a000_bank_w(offs_t offset, u8 data);

	template <int Num>
	TIMER_CALLBACK_MEMBER(timer_irq) { irq_set<Num>(1); }

	u16 timer_count(u8 timer) const;
	u8 timer_r(offs_t offset);
	void timer_w(offs_t offset, u8 data);

	u8 irq_status_r(offs_t offset);
	void irq_ack_w(offs_t offset, u8 data);

	void update_irq();

	u16 pen_x_scaled();
	u16 pen_y_scaled();
	u8 pen_value_r(offs_t offset);
	void pen_select_w(u8 data);

	u8 lcd_ctrl_r(offs_t offset);
	void lcd_ctrl_w(offs_t offset, u8 data);

	u8 lcdram_latch_r();
	void lcdram_w(offs_t offset, u8 data);

	required_device<v20_device> m_maincpu;
	required_device_array<address_map_bank_device, 4> m_bank_2000;  // 128kb windows (banked w/ 64kb granularity) at 20000-9ffff
	required_device_array<address_map_bank_device, 16> m_bank_a000; // 16kb windows at a0000-dffff
	memory_view m_wp_view[16]; // view used for enabling write-protect trap in a0000-dffff area
	required_device<zoomer_rtc_device> m_rtc;
	required_device<upd65043gfu01_device> m_psg;

	memory_share_creator<u8> m_nvram;

	required_ioport m_pen, m_pen_x, m_pen_y;

	u8 m_power;

	emu_timer *m_timer[3];
	u16 m_timer_rate[3];

	u16 m_irq_status;
	u16 m_pending_irq;

	u16 m_bank_2000_num[4];
	u16 m_bank_a000_num[16];

	u8 m_pen_select;

	u8 m_lcdram_latch;
	u16 m_lcd_ctrl;
};

enum
{
	IRQ_TIMER0,
	IRQ_TIMER1,
	IRQ_TIMER2,
	IRQ_KEYPAD,
	IRQ_SERIAL, // TODO
	IRQ_RTC_ALARM,
	IRQ_RTC_TICK,
	IRQ_PEN,
	IRQ_PCMCIA, // TODO
	IRQ_SOUND,
	IRQ_POWER,
	IRQ_PCMCIA_LOCK,   // TODO: PCMCIA card lock switch
	IRQ_PCMCIA_UNLOCK, // TODO: PCMCIA card lock switch
	IRQ_EMS
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void zoomer_state::maincpu_map(address_map &map)
{
	for (int i = 0; i < 4; i++)
	{
		const offs_t start = 0x20000 + (i << 17);
		map(start, start | 0x1ffff).rw(m_bank_2000[i], FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	}
	for (int i = 0; i < 16; i++)
	{
		const offs_t start = 0xa0000 + (i << 14);
		map(start, start | 0x3fff).rw(m_bank_a000[i], FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
		map(start, start | 0x3fff).view(m_wp_view[i]);
		m_wp_view[i][0](start, start | 0x3fff).lw8(NAME([this](u8 data) { irq_set<IRQ_EMS>(1); }));
	}

	map(0xd8000, 0xdffff).w(FUNC(zoomer_state::lcdram_w));
	map(0xe0000, 0xfffff).rom().region("maincpu", 0);
}

void zoomer_state::maincpu_ems_map(address_map &map)
{
	map(0x0000000, 0x00fffff).ram().share("nvram");
	map(0x0400000, 0x07fffff).rom().region("maincpu", 0);
	//map(0x1000000, 0x1ffffff) - TODO: PCMCIA
}

void zoomer_state::maincpu_io_map(address_map &map)
{
	map(0x0020, 0x0023).ram(); // TODO: irq related
	map(0x0024, 0x0025).r(FUNC(zoomer_state::irq_status_r));
	map(0x0026, 0x0027).w(FUNC(zoomer_state::irq_ack_w));
	map(0x0040, 0x004b).rw(FUNC(zoomer_state::timer_r), FUNC(zoomer_state::timer_w));
	map(0x004d, 0x004d).nopw(); // unknown, pen related
	map(0x0069, 0x0069).lr8(NAME([] { return 0x80; })); // suppress PCMCIA power usage warning
	map(0x0070, 0x0070).ram(); // suspend flag, used when pressing power switch or automatically powering off
	map(0x0071, 0x0071).lr8(NAME([] { return 0x07; })); // unknown, needed for BIOS to start up
	map(0x0072, 0x0072).portr("PORT72");
	map(0x0073, 0x0073).lr8(NAME([] { return 0x06; })); // PCMCIA slot voltage?
	map(0x0075, 0x0076).noprw(); // unknown, pen related
	map(0x0077, 0x0077).w(FUNC(zoomer_state::apo_w));
	map(0x0090, 0x00af).rw(FUNC(zoomer_state::ems_a000_bank_r), FUNC(zoomer_state::ems_a000_bank_w));
	map(0x00c0, 0x00c7).rw(FUNC(zoomer_state::ems_2000_bank_r), FUNC(zoomer_state::ems_2000_bank_w));
	map(0x00d0, 0x00d1).noprw(); // unknown, input related
	map(0x00d2, 0x00d2).portr("KEYPAD");
	map(0x00d3, 0x00d3).nopw(); // unknown, input related
	map(0x00d4, 0x00d4).portr("POWER");
	map(0x00d5, 0x00d5).nopw(); // unknown, pen related
	map(0x00d7, 0x00d7).lr8(NAME([] { return 0x01; })); // unknown, pen related
	map(0x00d8, 0x00d9).r(FUNC(zoomer_state::pen_value_r));
	map(0x00da, 0x00da).w(FUNC(zoomer_state::pen_select_w));
	map(0x00db, 0x00db).nopr(); // pen ADC ready
	map(0x00dc, 0x00dd).nopw(); // unknown, pen related
	map(0x00e0, 0x00ef).rw(m_rtc, FUNC(zoomer_rtc_device::read), FUNC(zoomer_rtc_device::write));
	map(0x0110, 0x011f).rw(m_psg, FUNC(upd65043gfu01_device::read), FUNC(upd65043gfu01_device::write));
	map(0x0204, 0x0205).rw(FUNC(zoomer_state::lcd_ctrl_r), FUNC(zoomer_state::lcd_ctrl_w));
	map(0x0213, 0x0213).r(FUNC(zoomer_state::lcdram_latch_r));
	/*
	TODO: port 414 = PCMCIA card status
	bit 0: ready
	bit 1: write protect
	bit 2: BVD1 (or 2?)
	bit 3: BVD2 (or 1?)
	bit 6: CD1 (or 2?)
	bit 7: CD2 (or 1?)
	*/
	map(0x0414, 0x0414).lr8(NAME([] { return 0xff; }));
	map(0x0800, 0x0fff).noprw(); // TODO: PCMCIA attribute memory?
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( zoomer )
	PORT_START("PORT72")
	PORT_CONFNAME(   0x01, IP_ACTIVE_HIGH, "Main Battery")
	PORT_CONFSETTING(0x00, "Normal")
	PORT_CONFSETTING(0x01, "Low")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN) // needs to be low to boot
	PORT_CONFNAME(   0x04, IP_ACTIVE_HIGH, "Backup Battery")
	PORT_CONFSETTING(0x00, "Normal")
	PORT_CONFSETTING(0x04, "Low")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN) // needs to be high to boot
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER)          PORT_NAME("PCMCIA Lock Switch") // TODO: PCMCIA lock switch (generates IRQ_PCMCIA_LOCK and IRQ_PCMCIA_UNLOCK)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	PORT_START("KEYPAD")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON3)        PORT_NAME("Button B")     PORT_WRITE_LINE_MEMBER(zoomer_state, irq_set<IRQ_KEYPAD>)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2)        PORT_NAME("Button A")     PORT_WRITE_LINE_MEMBER(zoomer_state, irq_set<IRQ_KEYPAD>)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_NAME("Cursor Up")    PORT_WRITE_LINE_MEMBER(zoomer_state, irq_set<IRQ_KEYPAD>)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_NAME("Cursor Left")  PORT_WRITE_LINE_MEMBER(zoomer_state, irq_set<IRQ_KEYPAD>)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_NAME("Cursor Down")  PORT_WRITE_LINE_MEMBER(zoomer_state, irq_set<IRQ_KEYPAD>)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_NAME("Cursor Right") PORT_WRITE_LINE_MEMBER(zoomer_state, irq_set<IRQ_KEYPAD>)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	PORT_START("POWER")
	PORT_BIT(0x7f, IP_ACTIVE_HIGH, IPT_UNKNOWN  )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_NAME("Power") PORT_WRITE_LINE_MEMBER(zoomer_state, power_w)

	PORT_START("PEN")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Pen") PORT_WRITE_LINE_MEMBER(zoomer_state, irq_set<IRQ_PEN>)

	PORT_START("PEN_X")
	PORT_BIT(0x3ff, 128, IPT_LIGHTGUN_X) PORT_NAME("Pen X") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0, 255) PORT_SENSITIVITY(45) PORT_KEYDELTA(30)

	PORT_START("PEN_Y")
	PORT_BIT(0x3ff, 179, IPT_LIGHTGUN_Y) PORT_NAME("Pen Y") PORT_CROSSHAIR(Y, 358.0 / 320, 0.0, 0) PORT_MINMAX(0, 357) PORT_SENSITIVITY(45) PORT_KEYDELTA(30)
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void zoomer_state::machine_start()
{
	m_irq_status = 0;
	m_pending_irq = 0;

	std::fill(std::begin(m_timer_rate), std::end(m_timer_rate), 0);
	std::fill(std::begin(m_bank_2000_num), std::end(m_bank_2000_num), 0);
	std::fill(std::begin(m_bank_a000_num), std::end(m_bank_a000_num), 0);

	m_pen_select = 0;

	m_lcdram_latch = 0;
	m_lcd_ctrl = 0;

	m_maincpu->space(AS_PROGRAM).install_ram(0x00000, 0x1ffff, m_nvram.begin());

	m_timer[0] = timer_alloc(FUNC(zoomer_state::timer_irq<IRQ_TIMER0>), this);
	m_timer[1] = timer_alloc(FUNC(zoomer_state::timer_irq<IRQ_TIMER1>), this);
	m_timer[2] = timer_alloc(FUNC(zoomer_state::timer_irq<IRQ_TIMER2>), this);

	save_item(NAME(m_power));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_pending_irq));
	save_item(NAME(m_timer_rate));
	save_item(NAME(m_bank_2000_num));
	save_item(NAME(m_bank_a000_num));
	save_item(NAME(m_pen_select));
	save_item(NAME(m_lcdram_latch));
	save_item(NAME(m_lcd_ctrl));
}

//**************************************************************************
void zoomer_state::machine_reset()
{
	m_power = 1;
	m_psg->set_output_gain(ALL_OUTPUTS, 1.0);
}

//**************************************************************************
u32 zoomer_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	auto &crosshair = machine().crosshair().get_crosshair(0);
	crosshair.set_screen(crosshair.y() < 1.0 ? &screen : CROSSHAIR_SCREEN_NONE);

	if (!m_power || !BIT(m_lcd_ctrl, 7))
	{
		bitmap.fill(0, cliprect);
	}
	else
	{
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
			{
				const u8 data = m_nvram[0xf8000 + (y << 6) + (x >> 3)];
				bitmap.pix(y, x) = BIT(data, 7 - (x & 7));
			}
		}
	}

	return 0;
}

//**************************************************************************
void zoomer_state::power_w(int state)
{
	if (state)
	{
		irq_set<IRQ_POWER>(1);
		m_power = 1;
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_psg->set_output_gain(ALL_OUTPUTS, 1.0);
	}
}

//**************************************************************************
void zoomer_state::apo_w(u8 data)
{
	if (BIT(data, 7))
	{
		m_power = 0;
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_psg->set_output_gain(ALL_OUTPUTS, 0.0);
	}
}

//**************************************************************************
u8 zoomer_state::ems_2000_bank_r(offs_t offset)
{
	return m_bank_2000_num[offset >> 1] >> (BIT(offset, 0) * 8);
}

//**************************************************************************
void zoomer_state::ems_2000_bank_w(offs_t offset, u8 data)
{
	const offs_t bank = offset >> 1;

	if (BIT(offset, 0))
	{
		m_bank_2000_num[bank] &= 0x00ff;
		m_bank_2000_num[bank] |= (data << 8);
	}
	else
	{
		m_bank_2000_num[bank] &= 0xff00;
		m_bank_2000_num[bank] |= data;
	}

	m_bank_2000[bank]->set_bank(m_bank_2000_num[bank]);
}

//**************************************************************************
u8 zoomer_state::ems_a000_bank_r(offs_t offset)
{
	return m_bank_a000_num[offset >> 1] >> (BIT(offset, 0) * 8);
}

//**************************************************************************
void zoomer_state::ems_a000_bank_w(offs_t offset, u8 data)
{
	const offs_t bank = offset >> 1;

	if (BIT(offset, 0))
	{
		m_bank_a000_num[offset >> 1] &= 0x00ff;
		m_bank_a000_num[offset >> 1] |= (data << 8);

		if (BIT(data, 7))
			m_wp_view[bank].select(0);
		else
			m_wp_view[bank].disable();
	}
	else
	{
		m_bank_a000_num[offset >> 1] &= 0xff00;
		m_bank_a000_num[offset >> 1] |= data;
	}

	m_bank_a000[bank]->set_bank(m_bank_a000_num[bank]);
}

//**************************************************************************
u16 zoomer_state::timer_count(u8 timer) const
{
	return m_timer_rate[timer] - m_timer[timer]->remaining().as_ticks(TIMER_RATE);
}

//**************************************************************************
u8 zoomer_state::timer_r(offs_t offset)
{
	const u8 timer = offset >> 2;

	switch (offset & 3)
	{
	case 0: return m_timer_rate[timer];
	case 1: return m_timer_rate[timer] >> 8;
	case 2: return timer_count(timer);
	case 3: return timer_count(timer) >> 8;
	}

	return 0;
}

//**************************************************************************
void zoomer_state::timer_w(offs_t offset, u8 data)
{
	const u8 timer = offset >> 2;

	switch (offset & 3)
	{
	case 0:
		m_timer_rate[timer] &= 0x0f00;
		m_timer_rate[timer] |= data;
		break;
	case 1:
		m_timer_rate[timer] &= 0x00ff;
		m_timer_rate[timer] |= ((data & 0xf) << 8);
		break;
	default:
		break;
	}

	if (!m_timer_rate[timer])
	{
		m_timer[timer]->adjust(attotime::never);
	}
	else
	{
		/*
		period needs to be based on value+1 for the GEOS system clock to not run fast
		(for some reason it reads the RTC only on boot, and then uses an OS-level software timer to keep time after that)
		*/
		const attotime period = attotime::from_ticks(m_timer_rate[timer] + 1, TIMER_RATE);

		/*
		DOS allows performing a factory reset by holding A+B on boot, which it checks by enabling input polling,
		waiting briefly in a loop, and then disabling input polling. in order for the input poll/debounce code
		to be called the expected number of times before polling is disabled again, the timer IRQ needs to go off
		immediately when set
		*/
		if (m_timer[timer]->expire().is_never())
			m_timer[timer]->adjust(attotime::zero, 0, period);
		else
			m_timer[timer]->adjust(m_timer[timer]->remaining(), 0, period);
	}
}

//**************************************************************************
u8 zoomer_state::irq_status_r(offs_t offset)
{
	return m_irq_status >> (BIT(offset, 0) * 8);
}

//**************************************************************************
void zoomer_state::irq_ack_w(offs_t offset, u8 data)
{
	const u16 clear = data << (BIT(offset, 0) * 8);

	if (BIT(clear, 15))
	{
		// clear current irq only (used by default handler)
		m_irq_status &= ~m_pending_irq;
		m_pending_irq = 0;
	}
	else
	{
		m_irq_status &= ~clear;
		m_pending_irq &= ~clear;
	}

	update_irq();
}

//**************************************************************************
void zoomer_state::update_irq()
{
	if (m_pending_irq) return;

	int state = CLEAR_LINE;
	int vector = 0;

	for (int i = 0; i < 14; i++)
	{
		if (BIT(m_irq_status, i))
		{
			state = ASSERT_LINE;
			vector = 0x70 + i;
			m_pending_irq = 1 << i;
			break;
		}
	}

	m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, state, vector);
}

//**************************************************************************
template <int Num>
void zoomer_state::irq_set(int state)
{
	if (state)
	{
		m_irq_status |= (1 << Num);
		update_irq();
	}
}

//**************************************************************************
u16 zoomer_state::pen_x_scaled()
{
	// 0...255 scaled to 0x069..396
	static constexpr float scale = 0x32d / 255.0;
	return 0x69 + (m_pen_x->read() * scale);
}

//**************************************************************************
u16 zoomer_state::pen_y_scaled()
{
	// 0...357 scaled to 0x044..3d4
	static constexpr float scale = 0x390 / 357.0;
	return 0x44 + (m_pen_y->read() * scale);
}

//**************************************************************************
u8 zoomer_state::pen_value_r(offs_t offset)
{
	u16 value = 0;

	if (m_pen->read() & 1)
	{
		switch (m_pen_select)
		{
		case 0x9: value = pen_x_scaled() ^ 0x3ff; break;
		case 0xb: value = pen_x_scaled(); break;
		case 0xd: value = pen_y_scaled() ^ 0x3ff; break;
		case 0xf: value = pen_y_scaled(); break;
		}
	}

	return value >> (BIT(offset, 0) * 8);
}

//**************************************************************************
void zoomer_state::pen_select_w(u8 data)
{
	m_pen_select = data;
}

//**************************************************************************
u8 zoomer_state::lcd_ctrl_r(offs_t offset)
{
	return m_lcd_ctrl >> (BIT(offset, 0) * 8);
}

//**************************************************************************
void zoomer_state::lcd_ctrl_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
	{
		m_lcd_ctrl &= 0x00ff;
		m_lcd_ctrl |= (data << 8);
	}
	else
	{
		m_lcd_ctrl &= 0xff00;
		m_lcd_ctrl |= data;
	}
}

//**************************************************************************
u8 zoomer_state::lcdram_latch_r()
{
	return m_lcdram_latch;
}

//**************************************************************************
void zoomer_state::lcdram_w(offs_t offset, u8 data)
{
	offset |= 0xf8000; // TODO: can VRAM be relocated?

	switch (BIT(m_lcd_ctrl, 10, 2))
	{
	case 0: m_nvram[offset] = data; break;
	case 1: m_nvram[offset] |= data; break;
	case 2: m_nvram[offset] &= data; break;
	// GEOS seems to read VRAM freely via EMS, but the BIOS does it this way instead
	case 3: m_lcdram_latch = m_nvram[offset]; break;
	}
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void zoomer_state::zoomer(machine_config &config)
{
	V20(config, m_maincpu, 7.3728_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &zoomer_state::maincpu_map);
	m_maincpu->set_addrmap(AS_IO, &zoomer_state::maincpu_io_map);

	for (int i = 0; i < 4; i++)
		ADDRESS_MAP_BANK(config, m_bank_2000[i]).set_map(&zoomer_state::maincpu_ems_map).set_options(ENDIANNESS_LITTLE, 8, 25, 0x10000);
	for (int i = 0; i < 16; i++)
		ADDRESS_MAP_BANK(config, m_bank_a000[i]).set_map(&zoomer_state::maincpu_ems_map).set_options(ENDIANNESS_LITTLE, 8, 25, 0x4000);

	ZOOMER_RTC(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->alarm_cb().set(FUNC(zoomer_state::irq_set<IRQ_RTC_ALARM>));
	m_rtc->tick_cb().set(FUNC(zoomer_state::irq_set<IRQ_RTC_TICK>));

	NVRAM(config, "nvram");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(256, 320);
	screen.set_visarea_full();
	screen.set_palette("palette");
	screen.set_screen_update(FUNC(zoomer_state::screen_update));

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	SPEAKER(config, "speaker").front_center();

	UPD65043GFU01(config, m_psg, 7.3728_MHz_XTAL);
	m_psg->add_route(0, "speaker", 1.0);
	m_psg->irq_cb().set(FUNC(zoomer_state::irq_set<IRQ_SOUND>));

	config.set_default_layout(layout_zoomer);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( zoomer )
	ROM_REGION(0x400000, "maincpu", 0)
	ROM_LOAD("mb838200al700.lsi1", 0x000000, 0x100000, CRC(5fe48af6) SHA1(790adb436d80409b216996ee02f78164832b5de6))
	ROM_LOAD("mb838200al701.lsi2", 0x100000, 0x100000, CRC(3104492b) SHA1(9c752c381d5506d7175e3c9645b5d2c731cc0990))
	ROM_LOAD("mb838200al702.lsi3", 0x200000, 0x100000, CRC(6013e531) SHA1(955c8c0bdf4bc3e262e5f2fe8b0f7d224a855c96))
	ROM_LOAD("mb838200al703.lsi4", 0x300000, 0x100000, CRC(64038705) SHA1(81ce3e8564d0579cf57a670517ebd2dbf35996c5))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY          FULLNAME                   FLAGS
COMP( 1993, zoomer, 0,      0,      zoomer,  zoomer, zoomer_state, empty_init, "Casio / Tandy", "Zoomer (Z-PDA / Z-7000)", MACHINE_SUPPORTS_SAVE )
