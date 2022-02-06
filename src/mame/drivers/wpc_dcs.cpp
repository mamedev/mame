// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Miodrag Milanovic
/******************************************************************************************
PINBALL
Williams WPC with DCS sound

If it says FACTORY SETTINGS, press F3, and wait for the game attract mode to commence.
To increase the volume, press NUM+.

Here are the key codes to enable play:

Game                                    NUM  Start game       End ball
--------------------------------------------------------------------------------------------------
**** Bally (Midway) ****
Judge Dredd                           20020  1 or T           (untested) should be INP81-87
**** Williams ****
Indiana Jones: The Pinball Adventure  50017  1                (untested) should be INP81-86
Popeye Saves the Earth                50022  1                Jiggle YZ,./Space until it registers
Star Trek: The Next Generation        50023  1                Hold =-[]\Enter then release them
Demolition Man                        50028  (not working)
**** Novelty Games ****
Addams Family Values                  60022  5 then 1 then the destination key

ToDo:
- Mechanical sounds
- Demolition Man: replace blown fuses

*********************************************************************************************/
#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "audio/dcs.h"
#include "machine/nvram.h"
#include "video/wpc_dmd.h"
#include "machine/wpc_shift.h"
#include "machine/wpc_lamp.h"
#include "machine/wpc_out.h"

namespace {

class wpc_dcs_state : public driver_device
{
public:
	wpc_dcs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dcs(*this, "dcs")
		, m_rombank(*this, "rombank")
		, m_mainram(*this, "mainram")
		, m_nvram(*this, "nvram")
		, m_lamp(*this, "lamp")
		, m_out(*this, "out")
		, m_io_keyboard(*this, "X%d", 0U)
	{ }

	void wpc_dcs(machine_config &config);

	void init();
	void init_dm();
	void init_ij();
	void init_jd();
	void init_pop();
	void init_sttng();
	void init_afv();

private:
	void bank_w(uint8_t data);
	void watchdog_w(uint8_t data);
	void irq_ack_w(uint8_t data);
	uint8_t firq_src_r();
	uint8_t zc_r();
	uint8_t dcs_data_r();
	void dcs_data_w(uint8_t data);
	uint8_t dcs_ctrl_r();
	void dcs_reset_w(uint8_t data);
	uint8_t rtc_r(offs_t offset);
	uint8_t switches_r();
	void switches_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(scanline_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(zc_timer);

	void wpc_dcs_map(address_map &map);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<dcs_audio_8k_device> m_dcs;
	required_memory_bank m_rombank;
	required_shared_ptr<uint8_t> m_mainram;
	required_device<nvram_device> m_nvram;
	required_device<wpc_lamp_device> m_lamp;
	required_device<wpc_out_device> m_out;
	required_ioport_array<8> m_io_keyboard;

	// driver_device overrides
	virtual void machine_reset() override;

	uint8_t m_firq_src = 0U, m_zc = 0U, m_row = 0U;
	uint16_t m_rtc_base_day = 0U;
};

void wpc_dcs_state::wpc_dcs_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("mainram");

	map(0x3000, 0x31ff).bankrw("dmd0");
	map(0x3200, 0x33ff).bankrw("dmd2");
	map(0x3400, 0x35ff).bankrw("dmd4");
	map(0x3600, 0x37ff).bankrw("dmd6");
	map(0x3800, 0x39ff).bankrw("dmd8");
	map(0x3a00, 0x3bff).bankrw("dmda");

	map(0x3fb8, 0x3fbf).m("dmd", FUNC(wpc_dmd_device::registers));

	map(0x3fd4, 0x3fd4).portr("FLIPPERS").w(m_out, FUNC(wpc_out_device::out4_w));

	map(0x3fdc, 0x3fdc).rw(FUNC(wpc_dcs_state::dcs_data_r), FUNC(wpc_dcs_state::dcs_data_w));
	map(0x3fdd, 0x3fdd).rw(FUNC(wpc_dcs_state::dcs_ctrl_r), FUNC(wpc_dcs_state::dcs_reset_w));

	map(0x3fe0, 0x3fe3).w(m_out, FUNC(wpc_out_device::out_w));
	map(0x3fe4, 0x3fe4).nopr().w(m_lamp, FUNC(wpc_lamp_device::row_w));
	map(0x3fe5, 0x3fe5).nopr().w(m_lamp, FUNC(wpc_lamp_device::col_w));
	map(0x3fe6, 0x3fe6).w(m_out, FUNC(wpc_out_device::gi_w));
	map(0x3fe7, 0x3fe7).portr("DSW");
	map(0x3fe8, 0x3fe8).portr("DOOR");
	map(0x3fe9, 0x3fe9).r(FUNC(wpc_dcs_state::switches_r));
	map(0x3fea, 0x3fea).w(FUNC(wpc_dcs_state::switches_w));

	map(0x3ff2, 0x3ff2).w(m_out, FUNC(wpc_out_device::led_w));
	map(0x3ff3, 0x3ff3).nopr().w(FUNC(wpc_dcs_state::irq_ack_w));
	map(0x3ff4, 0x3ff7).m("shift", FUNC(wpc_shift_device::registers));
	map(0x3ff8, 0x3ff8).r(FUNC(wpc_dcs_state::firq_src_r)).nopw(); // ack?
	map(0x3ffa, 0x3ffb).r(FUNC(wpc_dcs_state::rtc_r));
	map(0x3ffc, 0x3ffc).w(FUNC(wpc_dcs_state::bank_w));
	map(0x3ffd, 0x3ffe).noprw(); // memory protection stuff?
	map(0x3fff, 0x3fff).rw(FUNC(wpc_dcs_state::zc_r), FUNC(wpc_dcs_state::watchdog_w));
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x8000, 0xffff).rom().region("maincpu", 0x78000);
}

uint8_t wpc_dcs_state::dcs_data_r()
{
	return m_dcs->data_r();
}

void wpc_dcs_state::dcs_data_w(uint8_t data)
{
	m_dcs->data_w(data);
}

uint8_t wpc_dcs_state::dcs_ctrl_r()
{
	return m_dcs->control_r();
}

void wpc_dcs_state::dcs_reset_w(uint8_t data)
{
	m_dcs->reset_w(0);
	m_dcs->reset_w(1);
}

uint8_t wpc_dcs_state::switches_r()
{
	uint8_t res = 0xff;
	for(int i=0; i<8; i++)
		if(BIT(m_row, i))
			res &= m_io_keyboard[i]->read();
	return ~res;
}

void wpc_dcs_state::switches_w(uint8_t data)
{
	m_row = data;
}

uint8_t wpc_dcs_state::rtc_r(offs_t offset)
{
	system_time systime;
	machine().base_datetime(systime);

	// This may get wonky if the game is running on year change.  Find
	// something better to do at that time.

	uint8_t day = (systime.local_time.day - m_rtc_base_day) & 31;
	uint8_t hour = systime.local_time.hour;
	uint8_t min = systime.local_time.minute;

	switch(offset) {
	case 0:
		return ((day & 7) << 5) | hour;
	case 1:
		return ((day & 0x18) << 3) | min;
	default:
		return 0xff;
	}
}

uint8_t wpc_dcs_state::firq_src_r()
{
	return m_firq_src;
}

uint8_t wpc_dcs_state::zc_r()
{
	uint8_t res = m_zc;
	m_zc &= 0x7f;
	return res;
}

TIMER_DEVICE_CALLBACK_MEMBER(wpc_dcs_state::zc_timer)
{
	m_zc |= 0x80;
}

void wpc_dcs_state::bank_w(uint8_t data)
{
	m_rombank->set_entry(data & 0x1f);
}

void wpc_dcs_state::watchdog_w(uint8_t data)
{
	// Mhhh?  Maybe it's not 3ff3, maybe it's going down by itself...
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

WRITE_LINE_MEMBER(wpc_dcs_state::scanline_irq)
{
	m_firq_src = 0x00;
	m_maincpu->set_input_line(1, state);
}

void wpc_dcs_state::irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_maincpu->set_input_line(1, CLEAR_LINE);
}

void wpc_dcs_state::machine_reset()
{
	m_firq_src = 0x00;
	m_zc = 0x00;
	m_row = 0x00;

	/* The hardware seems to only have a minute/hour/day counter.  It
	   keeps the current day in nvram, and as long as you start the
	   machine at least once every 32 days (the day counter is 5 bits)
	   it updates it correctly.

	   So setup the correct memory zone to avoid the system bitching,
	   and requiring the user to fix it.
	*/
	system_time systime;
	machine().base_datetime(systime);
	m_mainram[0x1800] = systime.local_time.year >> 8;
	m_mainram[0x1801] = systime.local_time.year;
	m_mainram[0x1802] = systime.local_time.month+1;
	m_mainram[0x1803] = systime.local_time.mday;
	m_mainram[0x1804] = systime.local_time.weekday+1;
	m_mainram[0x1805] = 0;
	m_mainram[0x1806] = 1;
	uint16_t checksum = 0;
	for(int i=0x1800; i<=0x1806; i++)
		checksum += m_mainram[i];
	checksum = ~checksum;
	m_mainram[0x1807] = checksum >> 8;
	m_mainram[0x1808] = checksum;
	m_rtc_base_day = systime.local_time.day;
}

void wpc_dcs_state::init()
{
	m_rombank->configure_entries(0, 0x20, memregion("maincpu")->base(), 0x4000);
	m_nvram->set_base(m_mainram, m_mainram.bytes());

	save_item(NAME(m_firq_src));
	save_item(NAME(m_zc));
	save_item(NAME(m_row));

	// rtc_base_day not saved to give the system a better chance to
	// survive reload some days after unscathed.
}

void wpc_dcs_state::init_dm()
{
	m_lamp->set_names(nullptr);
	m_out->set_names(nullptr);
	init();
}

void wpc_dcs_state::init_ij()
{
	m_lamp->set_names(nullptr);
	m_out->set_names(nullptr);
	init();
}

void wpc_dcs_state::init_jd()
{
	m_lamp->set_names(nullptr);
	m_out->set_names(nullptr);
	init();
}

void wpc_dcs_state::init_pop()
{
	m_lamp->set_names(nullptr);
	m_out->set_names(nullptr);
	init();
}

void wpc_dcs_state::init_sttng()
{
	m_lamp->set_names(nullptr);
	m_out->set_names(nullptr);
	init();
}

void wpc_dcs_state::init_afv()
{
	m_lamp->set_names(nullptr);
	m_out->set_names(nullptr);
	init();
}

static INPUT_PORTS_START( wpc_dcs )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Buy-In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("INP12")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP15")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP16")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP17")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP18")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Ticket Dispenser")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD )  // always closed
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP25")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP26")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP27")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP28")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP31")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP32")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP33")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP34")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP35")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP36")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP37")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP38")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP48")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP51")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP52")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP53")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP54")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP55")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP56")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP57")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP58")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP61")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP62")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP63")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP64")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP65")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP66")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP67")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP68")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP71")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP72")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP73")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP74")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP75")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP76")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP77")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP78")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP81")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP82")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP83")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP84")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP85")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP86")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP87")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP88")

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Volume Down/Down")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Volume Up/Up")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0x01,0x01,"Switch 1") PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x01,DEF_STR( On ))
	PORT_DIPNAME(0x02,0x02,"Switch 2") PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x02,DEF_STR( On ))
	PORT_DIPNAME(0x04,0x00,"W20") PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x04,DEF_STR( On ))
	PORT_DIPNAME(0x08,0x00,"W19") PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x08,DEF_STR( On ))
	PORT_DIPNAME(0xf0,0x00,"Country") PORT_DIPLOCATION("SWA:5,6,7,8")
	PORT_DIPSETTING(0x00,"USA 1")
	PORT_DIPSETTING(0x10,"France 1")
	PORT_DIPSETTING(0x20,"Germany")
	PORT_DIPSETTING(0x30,"France 2")
	PORT_DIPSETTING(0x40,"Unknown 1")
	PORT_DIPSETTING(0x50,"Unknown 2")
	PORT_DIPSETTING(0x60,"Unknown 3")
	PORT_DIPSETTING(0x70,"Unknown 4")
	PORT_DIPSETTING(0x80,"Export 1")
	PORT_DIPSETTING(0x90,"France 3")
	PORT_DIPSETTING(0xa0,"Export 2")
	PORT_DIPSETTING(0xb0,"France 4")
	PORT_DIPSETTING(0xc0,"UK")
	PORT_DIPSETTING(0xd0,"Europe")
	PORT_DIPSETTING(0xe0,"Spain")
	PORT_DIPSETTING(0xf0,"USA 2")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")

INPUT_PORTS_END

void wpc_dcs_state::wpc_dcs(machine_config &config)
{
	/* basic machine hardware */
	M6809(config, m_maincpu, XTAL(8'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &wpc_dcs_state::wpc_dcs_map);
	m_maincpu->set_periodic_int(FUNC(wpc_dcs_state::irq0_line_assert), attotime::from_hz(XTAL(8'000'000)/8192.0));

	TIMER(config, "zero_crossing").configure_periodic(FUNC(wpc_dcs_state::zc_timer), attotime::from_hz(120)); // Mains power zero crossing

	WPC_LAMP(config, m_lamp, 0);
	WPC_OUT(config, m_out, 0, 3);
	WPC_SHIFT(config, "shift", 0);
	WPC_DMD(config, "dmd", 0).scanline_callback().set(FUNC(wpc_dcs_state::scanline_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	DCS_AUDIO_8K(config, m_dcs, 0);
}

/*----------------------
/ Demolition Man #50028
/----------------------*/
ROM_START(dm_pa2)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("u6-pa2.rom", 0x00000, 0x80000, CRC(862be56a) SHA1(95e1f899963762cb1a9de4eb5d6d57183ed1da38))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("dmsndp4.u2", 0x000000, 0x080000, CRC(8581116b) SHA1(ab24fa4aadf27761c9013adb84cfef9bfda27d44))
	ROM_LOAD16_BYTE("dmsndp4.u3", 0x200000, 0x080000, CRC(fe79fc89) SHA1(4ef1ef0d66d43fa66af1ecb17c14141760859084))
	ROM_LOAD16_BYTE("dmsndp4.u4", 0x400000, 0x080000, CRC(18407309) SHA1(499d62e4b434d48870fe532bb85106868df17c9b))
	ROM_LOAD16_BYTE("dmsndp4.u5", 0x600000, 0x080000, CRC(f2006c93) SHA1(16656ae6ff18aad0965c5a14882138508925313a))
	ROM_LOAD16_BYTE("dmsndp4.u6", 0x800000, 0x080000, CRC(bc17ba11) SHA1(a794599bc334762ddb79e1d0219ad20383139728))
	ROM_LOAD16_BYTE("dmsndp4.u7", 0xa00000, 0x080000, CRC(8760ed90) SHA1(cf8808f7cd347c47fa12e73a6bb5a54303fb7c49))
ROM_END

ROM_START(dm_px5)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("dman_px5.rom", 0x00000, 0x80000, CRC(42673371) SHA1(77570902c1ca13956fa65214184bce79bcc67173))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("dmsndp4.u2", 0x000000, 0x080000, CRC(8581116b) SHA1(ab24fa4aadf27761c9013adb84cfef9bfda27d44))
	ROM_LOAD16_BYTE("dmsndp4.u3", 0x200000, 0x080000, CRC(fe79fc89) SHA1(4ef1ef0d66d43fa66af1ecb17c14141760859084))
	ROM_LOAD16_BYTE("dmsndp4.u4", 0x400000, 0x080000, CRC(18407309) SHA1(499d62e4b434d48870fe532bb85106868df17c9b))
	ROM_LOAD16_BYTE("dmsndp4.u5", 0x600000, 0x080000, CRC(f2006c93) SHA1(16656ae6ff18aad0965c5a14882138508925313a))
	ROM_LOAD16_BYTE("dmsndp4.u6", 0x800000, 0x080000, CRC(bc17ba11) SHA1(a794599bc334762ddb79e1d0219ad20383139728))
	ROM_LOAD16_BYTE("dmsndp4.u7", 0xa00000, 0x080000, CRC(8760ed90) SHA1(cf8808f7cd347c47fa12e73a6bb5a54303fb7c49))
ROM_END

ROM_START(dm_la1)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("dman_la1.rom", 0x00000, 0x80000, CRC(be7c1965) SHA1(ed3b1016febc819b8c9f34953067bf0cdf3f33e6))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("dm_u2_s.l1", 0x000000, 0x080000, CRC(f72dc72e) SHA1(a1267c32f70b4bfe6058d7e28d82006541fe3d6c))
	ROM_LOAD16_BYTE("dm_u3_s.l2", 0x200000, 0x080000, CRC(2b65a66e) SHA1(7796082ecd7af29a240190aff654320375502a8b))
	ROM_LOAD16_BYTE("dm_u4_s.l2", 0x400000, 0x080000, CRC(9d6815fe) SHA1(fb4be63dee54a883884f1600565011cb9740a866))
	ROM_LOAD16_BYTE("dm_u5_s.l2", 0x600000, 0x080000, CRC(9f614c27) SHA1(f8f2f083b644517582a748bda0a3f69c14583f13))
	ROM_LOAD16_BYTE("dm_u6_s.l2", 0x800000, 0x080000, CRC(3efc2c0e) SHA1(bc4efdee44ff635771629a2bde79e230b7643f31))
	ROM_LOAD16_BYTE("dm_u7_s.l2", 0xa00000, 0x080000, CRC(75066af1) SHA1(4d70bce8a96343afcf02c89240b11faf19e11f02))
ROM_END

ROM_START(dm_lx3)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("dman_lx3.rom", 0x00000, 0x80000, CRC(5aa57674) SHA1(e02d91a705799866bd741b998d93413ec5bced25))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("dm_u2_s.l2", 0x000000, 0x080000, CRC(85fb8bce) SHA1(f2e912113d08b230e32aeeb4143485f266574fa2))
	ROM_LOAD16_BYTE("dm_u3_s.l2", 0x200000, 0x080000, CRC(2b65a66e) SHA1(7796082ecd7af29a240190aff654320375502a8b))
	ROM_LOAD16_BYTE("dm_u4_s.l2", 0x400000, 0x080000, CRC(9d6815fe) SHA1(fb4be63dee54a883884f1600565011cb9740a866))
	ROM_LOAD16_BYTE("dm_u5_s.l2", 0x600000, 0x080000, CRC(9f614c27) SHA1(f8f2f083b644517582a748bda0a3f69c14583f13))
	ROM_LOAD16_BYTE("dm_u6_s.l2", 0x800000, 0x080000, CRC(3efc2c0e) SHA1(bc4efdee44ff635771629a2bde79e230b7643f31))
	ROM_LOAD16_BYTE("dm_u7_s.l2", 0xa00000, 0x080000, CRC(75066af1) SHA1(4d70bce8a96343afcf02c89240b11faf19e11f02))
ROM_END

ROM_START(dm_lx4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("dman_lx4.rom", 0x00000, 0x80000, CRC(c2d0f493) SHA1(26ee970827dd96f3b3c56aa548cf7629ed6a16c1))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("dm_u2_s.l2", 0x000000, 0x080000, CRC(85fb8bce) SHA1(f2e912113d08b230e32aeeb4143485f266574fa2))
	ROM_LOAD16_BYTE("dm_u3_s.l2", 0x200000, 0x080000, CRC(2b65a66e) SHA1(7796082ecd7af29a240190aff654320375502a8b))
	ROM_LOAD16_BYTE("dm_u4_s.l2", 0x400000, 0x080000, CRC(9d6815fe) SHA1(fb4be63dee54a883884f1600565011cb9740a866))
	ROM_LOAD16_BYTE("dm_u5_s.l2", 0x600000, 0x080000, CRC(9f614c27) SHA1(f8f2f083b644517582a748bda0a3f69c14583f13))
	ROM_LOAD16_BYTE("dm_u6_s.l2", 0x800000, 0x080000, CRC(3efc2c0e) SHA1(bc4efdee44ff635771629a2bde79e230b7643f31))
	ROM_LOAD16_BYTE("dm_u7_s.l2", 0xa00000, 0x080000, CRC(75066af1) SHA1(4d70bce8a96343afcf02c89240b11faf19e11f02))
ROM_END

ROM_START(dm_h5)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("dman_h5.rom", 0x00000, 0x80000, CRC(bdcc62f7) SHA1(d6f3181970f3f71a876e9a2166156eb8fc405af0))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("dm.2", 0x000000, 0x080000, CRC(03dae358) SHA1(e6ab35a0c530eda90bd2d65af7bff82af08c39f3))
	ROM_LOAD16_BYTE("dm.3", 0x200000, 0x080000, CRC(3b924d3f) SHA1(5bd6126cc6a6c662de0bc311c047441bc29919b2))
	ROM_LOAD16_BYTE("dm.4", 0x400000, 0x080000, CRC(ff8985da) SHA1(b382c301744ce208f4710b3dd2342457d02f0ce9))
	ROM_LOAD16_BYTE("dm.5", 0x600000, 0x080000, CRC(76f09bd0) SHA1(1e4861ddc12069733f7e1d25192df97b0d9b09ee))
	ROM_LOAD16_BYTE("dm.6", 0x800000, 0x080000, CRC(2897aca8) SHA1(d910289e10422e22b4a3e1e296a4a167da1eaa5b))
	ROM_LOAD16_BYTE("dm.7", 0xa00000, 0x080000, CRC(6b1b9137) SHA1(4064f4fc230ba17b68819ff889335d9b6d9bba3e))
	ROM_LOAD16_BYTE("dm.8", 0xc00000, 0x080000, CRC(5b333818) SHA1(007b8c117516b6023b376f95ff13831111f4dc20))
	ROM_LOAD16_BYTE("dm.9", 0xe00000, 0x080000, CRC(4c1a34e8) SHA1(3eacc3c63b2d9db57fc86447f1408635b987ef69))
ROM_END

ROM_START(dm_h6)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("dman_h6.rom", 0x00000, 0x80000, CRC(3a079b80) SHA1(94a7ee94819ec878ced5e07745bf52b6c65e06c9))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("dm.2", 0x000000, 0x080000, CRC(03dae358) SHA1(e6ab35a0c530eda90bd2d65af7bff82af08c39f3))
	ROM_LOAD16_BYTE("dm.3", 0x200000, 0x080000, CRC(3b924d3f) SHA1(5bd6126cc6a6c662de0bc311c047441bc29919b2))
	ROM_LOAD16_BYTE("dm.4", 0x400000, 0x080000, CRC(ff8985da) SHA1(b382c301744ce208f4710b3dd2342457d02f0ce9))
	ROM_LOAD16_BYTE("dm.5", 0x600000, 0x080000, CRC(76f09bd0) SHA1(1e4861ddc12069733f7e1d25192df97b0d9b09ee))
	ROM_LOAD16_BYTE("dm.6", 0x800000, 0x080000, CRC(2897aca8) SHA1(d910289e10422e22b4a3e1e296a4a167da1eaa5b))
	ROM_LOAD16_BYTE("dm.7", 0xa00000, 0x080000, CRC(6b1b9137) SHA1(4064f4fc230ba17b68819ff889335d9b6d9bba3e))
	ROM_LOAD16_BYTE("dm.8", 0xc00000, 0x080000, CRC(5b333818) SHA1(007b8c117516b6023b376f95ff13831111f4dc20))
	ROM_LOAD16_BYTE("dm.9", 0xe00000, 0x080000, CRC(4c1a34e8) SHA1(3eacc3c63b2d9db57fc86447f1408635b987ef69))
ROM_END

/*----------------------
/  Indiana Jones #50017
/----------------------*/
ROM_START(ij_l7)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("ijone_l7.rom", 0x00000, 0x80000, CRC(4658c877) SHA1(b47ab064ff954bd182919f714ed8930cf0bed896))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("ijsnd_l3.u2", 0x000000, 0x080000, CRC(fbd91a0d) SHA1(8d9a74f04f6088f18dfbb578893410abc21a0e42))
	ROM_LOAD16_BYTE("ijsnd_l3.u3", 0x200000, 0x080000, CRC(3f12a996) SHA1(5f5d2853e671d13fafdb2972f52a823e18f27643))
	ROM_LOAD16_BYTE("ijsnd_l3.u4", 0x400000, 0x080000, CRC(05a92937) SHA1(e4e53e2899a7e7cbcd6ce7e3331bb8aa13321aa6))
	ROM_LOAD16_BYTE("ijsnd_l3.u5", 0x600000, 0x080000, CRC(e6fe417c) SHA1(d990ed218fe296ad9a015d77519b8d954d252035))
	ROM_LOAD16_BYTE("ijsnd_l3.u6", 0x800000, 0x080000, CRC(975f3e48) SHA1(16c56500b18e551bcd2e0c7e4c55ddab4791ac84))
	ROM_LOAD16_BYTE("ijsnd_l3.u7", 0xa00000, 0x080000, CRC(2d9cd098) SHA1(8d26c84cbd4ab2a5c9f4be3ea95a79fd125248e3))
	ROM_LOAD16_BYTE("ijsnd_l3.u8", 0xc00000, 0x080000, CRC(45e35bd7) SHA1(782b406be341d55d22a96acb8c2459f3058940df))
ROM_END

ROM_START(ij_lg7)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("u6-lg7.rom", 0x00000, 0x80000, CRC(c168a9f7) SHA1(732cc0863da06bce3d9793d57d67ba03c4c2f4d7))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("ijsnd_l3.u2", 0x000000, 0x080000, CRC(fbd91a0d) SHA1(8d9a74f04f6088f18dfbb578893410abc21a0e42))
	ROM_LOAD16_BYTE("ijsnd_l3.u3", 0x200000, 0x080000, CRC(3f12a996) SHA1(5f5d2853e671d13fafdb2972f52a823e18f27643))
	ROM_LOAD16_BYTE("ijsnd_l3.u4", 0x400000, 0x080000, CRC(05a92937) SHA1(e4e53e2899a7e7cbcd6ce7e3331bb8aa13321aa6))
	ROM_LOAD16_BYTE("ijsnd_l3.u5", 0x600000, 0x080000, CRC(e6fe417c) SHA1(d990ed218fe296ad9a015d77519b8d954d252035))
	ROM_LOAD16_BYTE("ijsnd_l3.u6", 0x800000, 0x080000, CRC(975f3e48) SHA1(16c56500b18e551bcd2e0c7e4c55ddab4791ac84))
	ROM_LOAD16_BYTE("ijsnd_l3.u7", 0xa00000, 0x080000, CRC(2d9cd098) SHA1(8d26c84cbd4ab2a5c9f4be3ea95a79fd125248e3))
	ROM_LOAD16_BYTE("ijsnd_l3.u8", 0xc00000, 0x080000, CRC(45e35bd7) SHA1(782b406be341d55d22a96acb8c2459f3058940df))
ROM_END

ROM_START(ij_l6)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("ijone_l6.rom", 0x00000, 0x80000, CRC(8c44b880) SHA1(9bc2cd91ea4d98e6509d6c1e2e34622e83c5a4d7))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("ijsnd_l3.u2", 0x000000, 0x080000, CRC(fbd91a0d) SHA1(8d9a74f04f6088f18dfbb578893410abc21a0e42))
	ROM_LOAD16_BYTE("ijsnd_l3.u3", 0x200000, 0x080000, CRC(3f12a996) SHA1(5f5d2853e671d13fafdb2972f52a823e18f27643))
	ROM_LOAD16_BYTE("ijsnd_l3.u4", 0x400000, 0x080000, CRC(05a92937) SHA1(e4e53e2899a7e7cbcd6ce7e3331bb8aa13321aa6))
	ROM_LOAD16_BYTE("ijsnd_l3.u5", 0x600000, 0x080000, CRC(e6fe417c) SHA1(d990ed218fe296ad9a015d77519b8d954d252035))
	ROM_LOAD16_BYTE("ijsnd_l3.u6", 0x800000, 0x080000, CRC(975f3e48) SHA1(16c56500b18e551bcd2e0c7e4c55ddab4791ac84))
	ROM_LOAD16_BYTE("ijsnd_l3.u7", 0xa00000, 0x080000, CRC(2d9cd098) SHA1(8d26c84cbd4ab2a5c9f4be3ea95a79fd125248e3))
	ROM_LOAD16_BYTE("ijsnd_l3.u8", 0xc00000, 0x080000, CRC(45e35bd7) SHA1(782b406be341d55d22a96acb8c2459f3058940df))
ROM_END

ROM_START(ij_l5)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("ijone_l5.rom", 0x00000, 0x80000, CRC(bf46ff92) SHA1(1afb1aadf115ae7d7f54bfea1fcca71a9de6ebb0))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("ijsnd_l2.u2", 0x000000, 0x080000, CRC(508d27c5) SHA1(da9787905c6f11d16e9a62047f15c5780017b551))
	ROM_LOAD16_BYTE("ijsnd_l3.u3", 0x200000, 0x080000, CRC(3f12a996) SHA1(5f5d2853e671d13fafdb2972f52a823e18f27643))
	ROM_LOAD16_BYTE("ijsnd_l3.u4", 0x400000, 0x080000, CRC(05a92937) SHA1(e4e53e2899a7e7cbcd6ce7e3331bb8aa13321aa6))
	ROM_LOAD16_BYTE("ijsnd_l3.u5", 0x600000, 0x080000, CRC(e6fe417c) SHA1(d990ed218fe296ad9a015d77519b8d954d252035))
	ROM_LOAD16_BYTE("ijsnd_l3.u6", 0x800000, 0x080000, CRC(975f3e48) SHA1(16c56500b18e551bcd2e0c7e4c55ddab4791ac84))
	ROM_LOAD16_BYTE("ijsnd_l3.u7", 0xa00000, 0x080000, CRC(2d9cd098) SHA1(8d26c84cbd4ab2a5c9f4be3ea95a79fd125248e3))
	ROM_LOAD16_BYTE("ijsnd_l3.u8", 0xc00000, 0x080000, CRC(45e35bd7) SHA1(782b406be341d55d22a96acb8c2459f3058940df))
ROM_END

ROM_START(ij_l4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("ij_l4.u6", 0x00000, 0x80000, CRC(5f2c3130) SHA1(b748932a1c0ac622e00744314fafef857f59026d))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("ijsnd_l2.u2", 0x000000, 0x080000, CRC(508d27c5) SHA1(da9787905c6f11d16e9a62047f15c5780017b551))
	ROM_LOAD16_BYTE("ijsnd_l3.u3", 0x200000, 0x080000, CRC(3f12a996) SHA1(5f5d2853e671d13fafdb2972f52a823e18f27643))
	ROM_LOAD16_BYTE("ijsnd_l3.u4", 0x400000, 0x080000, CRC(05a92937) SHA1(e4e53e2899a7e7cbcd6ce7e3331bb8aa13321aa6))
	ROM_LOAD16_BYTE("ijsnd_l3.u5", 0x600000, 0x080000, CRC(e6fe417c) SHA1(d990ed218fe296ad9a015d77519b8d954d252035))
	ROM_LOAD16_BYTE("ijsnd_l3.u6", 0x800000, 0x080000, CRC(975f3e48) SHA1(16c56500b18e551bcd2e0c7e4c55ddab4791ac84))
	ROM_LOAD16_BYTE("ijsnd_l3.u7", 0xa00000, 0x080000, CRC(2d9cd098) SHA1(8d26c84cbd4ab2a5c9f4be3ea95a79fd125248e3))
	ROM_LOAD16_BYTE("ijsnd_l3.u8", 0xc00000, 0x080000, CRC(45e35bd7) SHA1(782b406be341d55d22a96acb8c2459f3058940df))
ROM_END

ROM_START(ij_l3)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("ijone_l3.rom", 0x00000, 0x80000, CRC(0555c593) SHA1(1a73946fff9ae40e5499fcfa2d9f8330a25b8bae))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("ijsnd_l1.u2", 0x000000, 0x080000, CRC(89061ade) SHA1(0bd5ec961c780c4d46296aee7f2cb63b72e990f5))
	ROM_LOAD16_BYTE("ijsnd_l3.u3", 0x200000, 0x080000, CRC(3f12a996) SHA1(5f5d2853e671d13fafdb2972f52a823e18f27643))
	ROM_LOAD16_BYTE("ijsnd_l3.u4", 0x400000, 0x080000, CRC(05a92937) SHA1(e4e53e2899a7e7cbcd6ce7e3331bb8aa13321aa6))
	ROM_LOAD16_BYTE("ijsnd_l3.u5", 0x600000, 0x080000, CRC(e6fe417c) SHA1(d990ed218fe296ad9a015d77519b8d954d252035))
	ROM_LOAD16_BYTE("ijsnd_l3.u6", 0x800000, 0x080000, CRC(975f3e48) SHA1(16c56500b18e551bcd2e0c7e4c55ddab4791ac84))
	ROM_LOAD16_BYTE("ijsnd_l3.u7", 0xa00000, 0x080000, CRC(2d9cd098) SHA1(8d26c84cbd4ab2a5c9f4be3ea95a79fd125248e3))
	ROM_LOAD16_BYTE("ijsnd_l3.u8", 0xc00000, 0x080000, CRC(45e35bd7) SHA1(782b406be341d55d22a96acb8c2459f3058940df))
ROM_END

/*--------------------
/  Judge Dredd #20020
/--------------------*/
ROM_START(jd_l7)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("jdrd_l7.rom", 0x00000, 0x80000, CRC(87b2a5c3) SHA1(e487e9ff78353ee96d5fb5f036b1a6cef586f5b4))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("jdsu2_l3.bin", 0x000000, 0x080000, CRC(7a59ec18) SHA1(ee073d4bea198fd66de3508f67061b7d19f12edc))
	ROM_LOAD16_BYTE("jdsu3_l3.bin", 0x200000, 0x080000, CRC(42f52faa) SHA1(3fac9d3ddfe21877929eaa4cb7101a690745b163))
	ROM_LOAD16_BYTE("jdsnd_u4.bin", 0x400000, 0x080000, CRC(93f6ebc1) SHA1(5cb306afa693e60887069745588dfd5b930c5951))
	ROM_LOAD16_BYTE("jdsnd_u5.bin", 0x600000, 0x080000, CRC(c9f28ba6) SHA1(8447372428e3b9fc86a98286c05f95a13abe26b0))
	ROM_LOAD16_BYTE("jdsnd_u6.bin", 0x800000, 0x080000, CRC(ef0bf094) SHA1(c0860cecd436d352fe2c2208533ff6dc71bfced1))
	ROM_LOAD16_BYTE("jdsnd_u7.bin", 0xa00000, 0x080000, CRC(aebab88b) SHA1(d3f1be60a6840d9d085e22b43aafea1354771980))
	ROM_LOAD16_BYTE("jdsnd_u8.bin", 0xc00000, 0x080000, CRC(77604893) SHA1(a9a4a66412096edd88ee7adfd960eef6f5d16476))
	ROM_LOAD16_BYTE("jdsnd_u9.bin", 0xe00000, 0x080000, CRC(885b7c70) SHA1(be3bb42aeda3020a72c527f52c5330d0bafa9966))
ROM_END

ROM_START(jd_l1)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("jd_l1.u6", 0x00000, 0x80000, CRC(09a4b1d8) SHA1(9f941bbeb6e58d918d374694c7ff2a67f1084cc0))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("jdsnd_u2.bin", 0x000000, 0x080000, CRC(d8f453c6) SHA1(5dd677fde46436dbf2d2e9058f06dd3048600234))
	ROM_LOAD16_BYTE("jdsnd_u3.bin", 0x200000, 0x080000, CRC(0a11f673) SHA1(ab556477a25e3493555b8a281ca86677caec8947))
	ROM_LOAD16_BYTE("jdsnd_u4.bin", 0x400000, 0x080000, CRC(93f6ebc1) SHA1(5cb306afa693e60887069745588dfd5b930c5951))
	ROM_LOAD16_BYTE("jdsnd_u5.bin", 0x600000, 0x080000, CRC(c9f28ba6) SHA1(8447372428e3b9fc86a98286c05f95a13abe26b0))
	ROM_LOAD16_BYTE("jdsnd_u6.bin", 0x800000, 0x080000, CRC(ef0bf094) SHA1(c0860cecd436d352fe2c2208533ff6dc71bfced1))
	ROM_LOAD16_BYTE("jdsnd_u7.bin", 0xa00000, 0x080000, CRC(aebab88b) SHA1(d3f1be60a6840d9d085e22b43aafea1354771980))
	ROM_LOAD16_BYTE("jdsnd_u8.bin", 0xc00000, 0x080000, CRC(77604893) SHA1(a9a4a66412096edd88ee7adfd960eef6f5d16476))
	ROM_LOAD16_BYTE("jdsnd_u9.bin", 0xe00000, 0x080000, CRC(885b7c70) SHA1(be3bb42aeda3020a72c527f52c5330d0bafa9966))
ROM_END

ROM_START(jd_l6)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("jd_l6.u6", 0x00000, 0x80000, CRC(0a74cba4) SHA1(1872fd86bbfa772eac9cc2ef2634a90b72b3d5e2))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("jdsu2_l3.bin", 0x000000, 0x080000, CRC(7a59ec18) SHA1(ee073d4bea198fd66de3508f67061b7d19f12edc))
	ROM_LOAD16_BYTE("jdsu3_l3.bin", 0x200000, 0x080000, CRC(42f52faa) SHA1(3fac9d3ddfe21877929eaa4cb7101a690745b163))
	ROM_LOAD16_BYTE("jdsnd_u4.bin", 0x400000, 0x080000, CRC(93f6ebc1) SHA1(5cb306afa693e60887069745588dfd5b930c5951))
	ROM_LOAD16_BYTE("jdsnd_u5.bin", 0x600000, 0x080000, CRC(c9f28ba6) SHA1(8447372428e3b9fc86a98286c05f95a13abe26b0))
	ROM_LOAD16_BYTE("jdsnd_u6.bin", 0x800000, 0x080000, CRC(ef0bf094) SHA1(c0860cecd436d352fe2c2208533ff6dc71bfced1))
	ROM_LOAD16_BYTE("jdsnd_u7.bin", 0xa00000, 0x080000, CRC(aebab88b) SHA1(d3f1be60a6840d9d085e22b43aafea1354771980))
	ROM_LOAD16_BYTE("jdsnd_u8.bin", 0xc00000, 0x080000, CRC(77604893) SHA1(a9a4a66412096edd88ee7adfd960eef6f5d16476))
	ROM_LOAD16_BYTE("jdsnd_u9.bin", 0xe00000, 0x080000, CRC(885b7c70) SHA1(be3bb42aeda3020a72c527f52c5330d0bafa9966))
ROM_END

ROM_START(jd_l5)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("jd_l5.u6", 0x00000, 0x80000, CRC(879b091e) SHA1(eaf1c86c0e72e8cdfa9ac942fc54ef4f70a65175))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("jdsu2_l3.bin", 0x000000, 0x080000, CRC(7a59ec18) SHA1(ee073d4bea198fd66de3508f67061b7d19f12edc))
	ROM_LOAD16_BYTE("jdsu3_l3.bin", 0x200000, 0x080000, CRC(42f52faa) SHA1(3fac9d3ddfe21877929eaa4cb7101a690745b163))
	ROM_LOAD16_BYTE("jdsnd_u4.bin", 0x400000, 0x080000, CRC(93f6ebc1) SHA1(5cb306afa693e60887069745588dfd5b930c5951))
	ROM_LOAD16_BYTE("jdsnd_u5.bin", 0x600000, 0x080000, CRC(c9f28ba6) SHA1(8447372428e3b9fc86a98286c05f95a13abe26b0))
	ROM_LOAD16_BYTE("jdsnd_u6.bin", 0x800000, 0x080000, CRC(ef0bf094) SHA1(c0860cecd436d352fe2c2208533ff6dc71bfced1))
	ROM_LOAD16_BYTE("jdsnd_u7.bin", 0xa00000, 0x080000, CRC(aebab88b) SHA1(d3f1be60a6840d9d085e22b43aafea1354771980))
	ROM_LOAD16_BYTE("jdsnd_u8.bin", 0xc00000, 0x080000, CRC(77604893) SHA1(a9a4a66412096edd88ee7adfd960eef6f5d16476))
	ROM_LOAD16_BYTE("jdsnd_u9.bin", 0xe00000, 0x080000, CRC(885b7c70) SHA1(be3bb42aeda3020a72c527f52c5330d0bafa9966))
ROM_END

ROM_START(jd_l4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("jd_l4.u6", 0x00000, 0x80000, CRC(cc6f1068) SHA1(aef2a2eeb9110074eebff91318179ce97aba14ba))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("jdsu2_l3.bin", 0x000000, 0x080000, CRC(7a59ec18) SHA1(ee073d4bea198fd66de3508f67061b7d19f12edc))
	ROM_LOAD16_BYTE("jdsu3_l3.bin", 0x200000, 0x080000, CRC(42f52faa) SHA1(3fac9d3ddfe21877929eaa4cb7101a690745b163))
	ROM_LOAD16_BYTE("jdsnd_u4.bin", 0x400000, 0x080000, CRC(93f6ebc1) SHA1(5cb306afa693e60887069745588dfd5b930c5951))
	ROM_LOAD16_BYTE("jdsnd_u5.bin", 0x600000, 0x080000, CRC(c9f28ba6) SHA1(8447372428e3b9fc86a98286c05f95a13abe26b0))
	ROM_LOAD16_BYTE("jdsnd_u6.bin", 0x800000, 0x080000, CRC(ef0bf094) SHA1(c0860cecd436d352fe2c2208533ff6dc71bfced1))
	ROM_LOAD16_BYTE("jdsnd_u7.bin", 0xa00000, 0x080000, CRC(aebab88b) SHA1(d3f1be60a6840d9d085e22b43aafea1354771980))
	ROM_LOAD16_BYTE("jdsnd_u8.bin", 0xc00000, 0x080000, CRC(77604893) SHA1(a9a4a66412096edd88ee7adfd960eef6f5d16476))
	ROM_LOAD16_BYTE("jdsnd_u9.bin", 0xe00000, 0x080000, CRC(885b7c70) SHA1(be3bb42aeda3020a72c527f52c5330d0bafa9966))
ROM_END

/*-------------------------------
/ Popeye Saves The Earth #50022
/-------------------------------*/
ROM_START(pop_lx5)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("peye_lx5.rom", 0x00000, 0x80000, CRC(ee1f7a67) SHA1(f02518546de93256b00bc1f5b92452a10f9e56dd))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("popsndl2.u2", 0x000000, 0x080000, CRC(00590f2d) SHA1(540ad9825dbaace55bf36a6cee98bef06f240e15))
	ROM_LOAD16_BYTE("popsndl2.u3", 0x200000, 0x080000, CRC(87032b27) SHA1(9488d177418b53ceb37686cf6f4f58800b306d85))
	ROM_LOAD16_BYTE("popsndl2.u4", 0x400000, 0x080000, CRC(b0808aa8) SHA1(bebe6ec3c3e675e096084b6ed61065ad48dc5c3f))
	ROM_LOAD16_BYTE("popsndl2.u5", 0x600000, 0x080000, CRC(3662206b) SHA1(c2714665db18e9ae540a8f922d7ebb3058638563))
	ROM_LOAD16_BYTE("popsndl2.u6", 0x800000, 0x080000, CRC(84a5f317) SHA1(f1b9710d109e28fe3255e36dafa2be23656d0445))
	ROM_LOAD16_BYTE("popsndl2.u7", 0xa00000, 0x080000, CRC(b8fde2c7) SHA1(ee82a7b1ad32e1231356ce42c4ad3109150a9992))
ROM_END

ROM_START(pop_pa3)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("peye_pa3.rom", 0x00000, 0x80000, CRC(1cee3ae7) SHA1(e5b05fcf8aac98993940a2cda2552ff93ee3a518))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("popsndp0.u2", 0x000000, 0x080000, CRC(1e3a98a4) SHA1(2f871f354df7684d0b4aa31e2d2bb4035072bb4a))
	ROM_LOAD16_BYTE("popsndl2.u3", 0x200000, 0x080000, CRC(87032b27) SHA1(9488d177418b53ceb37686cf6f4f58800b306d85))
	ROM_LOAD16_BYTE("popsndl2.u4", 0x400000, 0x080000, CRC(b0808aa8) SHA1(bebe6ec3c3e675e096084b6ed61065ad48dc5c3f))
	ROM_LOAD16_BYTE("popsndl2.u5", 0x600000, 0x080000, CRC(3662206b) SHA1(c2714665db18e9ae540a8f922d7ebb3058638563))
	ROM_LOAD16_BYTE("popsndl2.u6", 0x800000, 0x080000, CRC(84a5f317) SHA1(f1b9710d109e28fe3255e36dafa2be23656d0445))
	ROM_LOAD16_BYTE("popsndl2.u7", 0xa00000, 0x080000, CRC(b8fde2c7) SHA1(ee82a7b1ad32e1231356ce42c4ad3109150a9992))
ROM_END

ROM_START(pop_la4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("peye_la4.rom", 0x00000, 0x80000, CRC(11cedcf7) SHA1(e0219060cf09a757edf19875a224801b3179664c))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("popsndl2.u2", 0x000000, 0x080000, CRC(00590f2d) SHA1(540ad9825dbaace55bf36a6cee98bef06f240e15))
	ROM_LOAD16_BYTE("popsndl2.u3", 0x200000, 0x080000, CRC(87032b27) SHA1(9488d177418b53ceb37686cf6f4f58800b306d85))
	ROM_LOAD16_BYTE("popsndl2.u4", 0x400000, 0x080000, CRC(b0808aa8) SHA1(bebe6ec3c3e675e096084b6ed61065ad48dc5c3f))
	ROM_LOAD16_BYTE("popsndl2.u5", 0x600000, 0x080000, CRC(3662206b) SHA1(c2714665db18e9ae540a8f922d7ebb3058638563))
	ROM_LOAD16_BYTE("popsndl2.u6", 0x800000, 0x080000, CRC(84a5f317) SHA1(f1b9710d109e28fe3255e36dafa2be23656d0445))
	ROM_LOAD16_BYTE("popsndl2.u7", 0xa00000, 0x080000, CRC(b8fde2c7) SHA1(ee82a7b1ad32e1231356ce42c4ad3109150a9992))
ROM_END


/*--------------------------------------
/ Star Trek: The Next Generation #50023
/--------------------------------------*/
ROM_START(sttng_l7)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("trek_lx7.rom", 0x00000, 0x80000, CRC(d439fdbb) SHA1(12d1c72cd6cc18db53e51ebb4c1e55ca9bcf9908))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("ng_u2_s.l1", 0x000000, 0x080000, CRC(c3bd7bf5) SHA1(2476ff90232a52d667a407fac81ee4db028b94e5))
	ROM_LOAD16_BYTE("ng_u3_s.l1", 0x200000, 0x080000, CRC(9456cac7) SHA1(83e415e0f21bb5418f3677dbc13433e056c523ab))
	ROM_LOAD16_BYTE("ng_u4_s.l1", 0x400000, 0x080000, CRC(179d22a4) SHA1(456b7189e23d4e2bd7e2a6249fa2a73bf0e12194))
	ROM_LOAD16_BYTE("ng_u5_s.l1", 0x600000, 0x080000, CRC(231a3e72) SHA1(081b1a042e62ccb723788059d6c1e00b9b32c778))
	ROM_LOAD16_BYTE("ng_u6_s.l1", 0x800000, 0x080000, CRC(bb21377d) SHA1(229fb42a1f8b22727a809e5d63f26f045a2adda5))
	ROM_LOAD16_BYTE("ng_u7_s.l1", 0xa00000, 0x080000, CRC(d81b39f0) SHA1(3443e7327c755b85a5b390f7fcd0e9923890425a))
	ROM_LOAD16_BYTE("ng_u8_s.l1", 0xc00000, 0x080000, CRC(c9fb065e) SHA1(c148178ee0ea787acc88078db01d17073e75fdc7))
ROM_END

ROM_START(sttng_x7)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("trek_x7.rom", 0x00000, 0x80000, CRC(4e71c9c7) SHA1(8a7ec42dfb4a6902ba745548b40e84de5305c295))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("ngs_u2.rom", 0x000000, 0x080000, CRC(e9fe68fe) SHA1(3d7631aa5ddd52f7c3c00cd091e212430faea249))
	ROM_LOAD16_BYTE("ngs_u3.rom", 0x200000, 0x080000, CRC(368cfd89) SHA1(40ddc12b2cabbcf73ababf753f3a2fd4bcc10737))
	ROM_LOAD16_BYTE("ngs_u4.rom", 0x400000, 0x080000, CRC(8e79a513) SHA1(4b763d7445acd921a0a6d64d18b5df8ff9e3257e))
	ROM_LOAD16_BYTE("ngs_u5.rom", 0x600000, 0x080000, CRC(46049eb0) SHA1(02991bf1d33ac1df91f459b2d37cf7e07e347b04))
	ROM_LOAD16_BYTE("ngs_u6.rom", 0x800000, 0x080000, CRC(e0124da0) SHA1(bfdba059d084c93122ad291aa8def61f43c26d47))
	ROM_LOAD16_BYTE("ngs_u7.rom", 0xa00000, 0x080000, CRC(dc1c74d0) SHA1(21b6b4d2cdd5086bcbbc7ee7a2abdc550a45d2e3))
	ROM_LOAD16_BYTE("ng_u8_s.l1", 0xc00000, 0x080000, CRC(c9fb065e) SHA1(c148178ee0ea787acc88078db01d17073e75fdc7))
ROM_END

ROM_START(sttng_s7)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("trek_lx7.rom", 0x00000, 0x80000, CRC(d439fdbb) SHA1(12d1c72cd6cc18db53e51ebb4c1e55ca9bcf9908))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("su2-sp1.rom", 0x000000, 0x080000, CRC(bdef8b2c) SHA1(188d8d2a652844e9885bd9e9ad4143927ddc6fee))
	ROM_LOAD16_BYTE("ng_u3_s.l1", 0x200000, 0x080000, CRC(9456cac7) SHA1(83e415e0f21bb5418f3677dbc13433e056c523ab))
	ROM_LOAD16_BYTE("ng_u4_s.l1", 0x400000, 0x080000, CRC(179d22a4) SHA1(456b7189e23d4e2bd7e2a6249fa2a73bf0e12194))
	ROM_LOAD16_BYTE("ng_u5_s.l1", 0x600000, 0x080000, CRC(231a3e72) SHA1(081b1a042e62ccb723788059d6c1e00b9b32c778))
	ROM_LOAD16_BYTE("ng_u6_s.l1", 0x800000, 0x080000, CRC(bb21377d) SHA1(229fb42a1f8b22727a809e5d63f26f045a2adda5))
	ROM_LOAD16_BYTE("ng_u7_s.l1", 0xa00000, 0x080000, CRC(d81b39f0) SHA1(3443e7327c755b85a5b390f7fcd0e9923890425a))
	ROM_LOAD16_BYTE("ng_u8_s.l1", 0xc00000, 0x080000, CRC(c9fb065e) SHA1(c148178ee0ea787acc88078db01d17073e75fdc7))
ROM_END

ROM_START(sttng_p8)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sttng_p8.u6", 0x00000, 0x80000, CRC(bf599f45) SHA1(ec660f99030f89bdfe3d04cc38fd450d6bbedf7d))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("ng_u2_s.l1", 0x000000, 0x080000, CRC(c3bd7bf5) SHA1(2476ff90232a52d667a407fac81ee4db028b94e5))
	ROM_LOAD16_BYTE("ng_u3_s.l1", 0x200000, 0x080000, CRC(9456cac7) SHA1(83e415e0f21bb5418f3677dbc13433e056c523ab))
	ROM_LOAD16_BYTE("ng_u4_s.l1", 0x400000, 0x080000, CRC(179d22a4) SHA1(456b7189e23d4e2bd7e2a6249fa2a73bf0e12194))
	ROM_LOAD16_BYTE("ng_u5_s.l1", 0x600000, 0x080000, CRC(231a3e72) SHA1(081b1a042e62ccb723788059d6c1e00b9b32c778))
	ROM_LOAD16_BYTE("ng_u6_s.l1", 0x800000, 0x080000, CRC(bb21377d) SHA1(229fb42a1f8b22727a809e5d63f26f045a2adda5))
	ROM_LOAD16_BYTE("ng_u7_s.l1", 0xa00000, 0x080000, CRC(d81b39f0) SHA1(3443e7327c755b85a5b390f7fcd0e9923890425a))
	ROM_LOAD16_BYTE("ng_u8_s.l1", 0xc00000, 0x080000, CRC(c9fb065e) SHA1(c148178ee0ea787acc88078db01d17073e75fdc7))
ROM_END

ROM_START(sttng_p5)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sttng_p5.u6", 0x00000, 0x80000, CRC(c1b80a8e) SHA1(90dd99efd41ec5405c631ad374a369f9fcb7217e))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("ng_u2_s.l1", 0x000000, 0x080000, CRC(c3bd7bf5) SHA1(2476ff90232a52d667a407fac81ee4db028b94e5))
	ROM_LOAD16_BYTE("ng_u3_s.l1", 0x200000, 0x080000, CRC(9456cac7) SHA1(83e415e0f21bb5418f3677dbc13433e056c523ab))
	ROM_LOAD16_BYTE("ng_u4_s.l1", 0x400000, 0x080000, CRC(179d22a4) SHA1(456b7189e23d4e2bd7e2a6249fa2a73bf0e12194))
	ROM_LOAD16_BYTE("ng_u5_s.l1", 0x600000, 0x080000, CRC(231a3e72) SHA1(081b1a042e62ccb723788059d6c1e00b9b32c778))
	ROM_LOAD16_BYTE("ng_u6_s.l1", 0x800000, 0x080000, CRC(bb21377d) SHA1(229fb42a1f8b22727a809e5d63f26f045a2adda5))
	ROM_LOAD16_BYTE("ng_u7_s.l1", 0xa00000, 0x080000, CRC(d81b39f0) SHA1(3443e7327c755b85a5b390f7fcd0e9923890425a))
	ROM_LOAD16_BYTE("ng_u8_s.l1", 0xc00000, 0x080000, CRC(c9fb065e) SHA1(c148178ee0ea787acc88078db01d17073e75fdc7))
ROM_END

ROM_START(sttng_p4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("sttng_p4.u6", 0x00000, 0x80000, CRC(836774f0) SHA1(5784f77eaad41ccf07446874720be146fd562c68))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("ng_u2_s.l1", 0x000000, 0x080000, CRC(c3bd7bf5) SHA1(2476ff90232a52d667a407fac81ee4db028b94e5))
	ROM_LOAD16_BYTE("ng_u3_s.l1", 0x200000, 0x080000, CRC(9456cac7) SHA1(83e415e0f21bb5418f3677dbc13433e056c523ab))
	ROM_LOAD16_BYTE("ng_u4_s.l1", 0x400000, 0x080000, CRC(179d22a4) SHA1(456b7189e23d4e2bd7e2a6249fa2a73bf0e12194))
	ROM_LOAD16_BYTE("ng_u5_s.l1", 0x600000, 0x080000, CRC(231a3e72) SHA1(081b1a042e62ccb723788059d6c1e00b9b32c778))
	ROM_LOAD16_BYTE("ng_u6_s.l1", 0x800000, 0x080000, CRC(bb21377d) SHA1(229fb42a1f8b22727a809e5d63f26f045a2adda5))
	ROM_LOAD16_BYTE("ng_u7_s.l1", 0xa00000, 0x080000, CRC(d81b39f0) SHA1(3443e7327c755b85a5b390f7fcd0e9923890425a))
	ROM_LOAD16_BYTE("ng_u8_s.l1", 0xc00000, 0x080000, CRC(c9fb065e) SHA1(c148178ee0ea787acc88078db01d17073e75fdc7))
ROM_END

ROM_START(sttng_g7)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("trek_lg7.rom", 0x00000, 0x80000, CRC(e723b8a1) SHA1(77c3f8ea378772ce45bb8de818069fc08cbc4574))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("ng_u2_s.l1", 0x000000, 0x080000, CRC(c3bd7bf5) SHA1(2476ff90232a52d667a407fac81ee4db028b94e5))
	ROM_LOAD16_BYTE("ng_u3_s.l1", 0x200000, 0x080000, CRC(9456cac7) SHA1(83e415e0f21bb5418f3677dbc13433e056c523ab))
	ROM_LOAD16_BYTE("ng_u4_s.l1", 0x400000, 0x080000, CRC(179d22a4) SHA1(456b7189e23d4e2bd7e2a6249fa2a73bf0e12194))
	ROM_LOAD16_BYTE("ng_u5_s.l1", 0x600000, 0x080000, CRC(231a3e72) SHA1(081b1a042e62ccb723788059d6c1e00b9b32c778))
	ROM_LOAD16_BYTE("ng_u6_s.l1", 0x800000, 0x080000, CRC(bb21377d) SHA1(229fb42a1f8b22727a809e5d63f26f045a2adda5))
	ROM_LOAD16_BYTE("ng_u7_s.l1", 0xa00000, 0x080000, CRC(d81b39f0) SHA1(3443e7327c755b85a5b390f7fcd0e9923890425a))
	ROM_LOAD16_BYTE("ng_u8_s.l1", 0xc00000, 0x080000, CRC(c9fb065e) SHA1(c148178ee0ea787acc88078db01d17073e75fdc7))
ROM_END

ROM_START(sttng_l1)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("trek_lx1.rom", 0x00000, 0x80000, CRC(390befc0) SHA1(2059891e3fc3034d600274c3915371123c964d28))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("ng_u2_s.l1", 0x000000, 0x080000, CRC(c3bd7bf5) SHA1(2476ff90232a52d667a407fac81ee4db028b94e5))
	ROM_LOAD16_BYTE("ng_u3_s.l1", 0x200000, 0x080000, CRC(9456cac7) SHA1(83e415e0f21bb5418f3677dbc13433e056c523ab))
	ROM_LOAD16_BYTE("ng_u4_s.l1", 0x400000, 0x080000, CRC(179d22a4) SHA1(456b7189e23d4e2bd7e2a6249fa2a73bf0e12194))
	ROM_LOAD16_BYTE("ng_u5_s.l1", 0x600000, 0x080000, CRC(231a3e72) SHA1(081b1a042e62ccb723788059d6c1e00b9b32c778))
	ROM_LOAD16_BYTE("ng_u6_s.l1", 0x800000, 0x080000, CRC(bb21377d) SHA1(229fb42a1f8b22727a809e5d63f26f045a2adda5))
	ROM_LOAD16_BYTE("ng_u7_s.l1", 0xa00000, 0x080000, CRC(d81b39f0) SHA1(3443e7327c755b85a5b390f7fcd0e9923890425a))
	ROM_LOAD16_BYTE("ng_u8_s.l1", 0xc00000, 0x080000, CRC(c9fb065e) SHA1(c148178ee0ea787acc88078db01d17073e75fdc7))
ROM_END

ROM_START(sttng_l2)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("trek_lx2.rom", 0x00000, 0x80000, CRC(e2557554) SHA1(7d8502ab9df340d60fd72e6964740bc7a2da2065))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("ng_u2_s.l1", 0x000000, 0x080000, CRC(c3bd7bf5) SHA1(2476ff90232a52d667a407fac81ee4db028b94e5))
	ROM_LOAD16_BYTE("ng_u3_s.l1", 0x200000, 0x080000, CRC(9456cac7) SHA1(83e415e0f21bb5418f3677dbc13433e056c523ab))
	ROM_LOAD16_BYTE("ng_u4_s.l1", 0x400000, 0x080000, CRC(179d22a4) SHA1(456b7189e23d4e2bd7e2a6249fa2a73bf0e12194))
	ROM_LOAD16_BYTE("ng_u5_s.l1", 0x600000, 0x080000, CRC(231a3e72) SHA1(081b1a042e62ccb723788059d6c1e00b9b32c778))
	ROM_LOAD16_BYTE("ng_u6_s.l1", 0x800000, 0x080000, CRC(bb21377d) SHA1(229fb42a1f8b22727a809e5d63f26f045a2adda5))
	ROM_LOAD16_BYTE("ng_u7_s.l1", 0xa00000, 0x080000, CRC(d81b39f0) SHA1(3443e7327c755b85a5b390f7fcd0e9923890425a))
	ROM_LOAD16_BYTE("ng_u8_s.l1", 0xc00000, 0x080000, CRC(c9fb065e) SHA1(c148178ee0ea787acc88078db01d17073e75fdc7))
ROM_END

ROM_START(sttng_l3)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("trek_lx3.rom", 0x00000, 0x80000, CRC(400e7887) SHA1(23d5e9796f0c3c66121da53088df6f5275348f4a))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("ng_u2_s.l1", 0x000000, 0x080000, CRC(c3bd7bf5) SHA1(2476ff90232a52d667a407fac81ee4db028b94e5))
	ROM_LOAD16_BYTE("ng_u3_s.l1", 0x200000, 0x080000, CRC(9456cac7) SHA1(83e415e0f21bb5418f3677dbc13433e056c523ab))
	ROM_LOAD16_BYTE("ng_u4_s.l1", 0x400000, 0x080000, CRC(179d22a4) SHA1(456b7189e23d4e2bd7e2a6249fa2a73bf0e12194))
	ROM_LOAD16_BYTE("ng_u5_s.l1", 0x600000, 0x080000, CRC(231a3e72) SHA1(081b1a042e62ccb723788059d6c1e00b9b32c778))
	ROM_LOAD16_BYTE("ng_u6_s.l1", 0x800000, 0x080000, CRC(bb21377d) SHA1(229fb42a1f8b22727a809e5d63f26f045a2adda5))
	ROM_LOAD16_BYTE("ng_u7_s.l1", 0xa00000, 0x080000, CRC(d81b39f0) SHA1(3443e7327c755b85a5b390f7fcd0e9923890425a))
	ROM_LOAD16_BYTE("ng_u8_s.l1", 0xc00000, 0x080000, CRC(c9fb065e) SHA1(c148178ee0ea787acc88078db01d17073e75fdc7))
ROM_END

ROM_START(sttng_l5)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("trek_lx5.rom", 0x00000, 0x80000, CRC(e004f3a7) SHA1(c724641106115e3f14bbe3998771823d0ac12d69))
	ROM_REGION16_LE(0x1000000, "dcs",0)
	ROM_LOAD16_BYTE("ng_u2_s.l1", 0x000000, 0x080000, CRC(c3bd7bf5) SHA1(2476ff90232a52d667a407fac81ee4db028b94e5))
	ROM_LOAD16_BYTE("ng_u3_s.l1", 0x200000, 0x080000, CRC(9456cac7) SHA1(83e415e0f21bb5418f3677dbc13433e056c523ab))
	ROM_LOAD16_BYTE("ng_u4_s.l1", 0x400000, 0x080000, CRC(179d22a4) SHA1(456b7189e23d4e2bd7e2a6249fa2a73bf0e12194))
	ROM_LOAD16_BYTE("ng_u5_s.l1", 0x600000, 0x080000, CRC(231a3e72) SHA1(081b1a042e62ccb723788059d6c1e00b9b32c778))
	ROM_LOAD16_BYTE("ng_u6_s.l1", 0x800000, 0x080000, CRC(bb21377d) SHA1(229fb42a1f8b22727a809e5d63f26f045a2adda5))
	ROM_LOAD16_BYTE("ng_u7_s.l1", 0xa00000, 0x080000, CRC(d81b39f0) SHA1(3443e7327c755b85a5b390f7fcd0e9923890425a))
	ROM_LOAD16_BYTE("ng_u8_s.l1", 0xc00000, 0x080000, CRC(c9fb065e) SHA1(c148178ee0ea787acc88078db01d17073e75fdc7))
ROM_END


/*-------------------------------------------
/ Addams Family Values #60022 (Coin Dropper)
/-------------------------------------------*/
ROM_START(afv_l4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD("afv_u6.l4", 0x00000, 0x80000, CRC(37369339) SHA1(e44a91faca80ffa00d6db78e2df7aa9bf14e957c))
	ROM_REGION16_LE(0x1000000, "dcs", 0)
	ROM_LOAD16_BYTE("afv_su2.l1", 0x000000, 0x080000, CRC(1aa878fc) SHA1(59a89071001b5da6ab56d691721a015773f5f0b5))
ROM_END

} // Anonymous namespace

GAME(1994,  dm_lx4,     0,          wpc_dcs,    wpc_dcs, wpc_dcs_state, init_dm,    ROT0,   "Williams",  "Demolition Man (LX-4)",                            MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  dm_pa2,     dm_lx4,     wpc_dcs,    wpc_dcs, wpc_dcs_state, init_dm,    ROT0,   "Williams",  "Demolition Man (PA-2)",                            MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  dm_px5,     dm_lx4,     wpc_dcs,    wpc_dcs, wpc_dcs_state, init_dm,    ROT0,   "Williams",  "Demolition Man (PX-5)",                            MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  dm_la1,     dm_lx4,     wpc_dcs,    wpc_dcs, wpc_dcs_state, init_dm,    ROT0,   "Williams",  "Demolition Man (LA-1)",                            MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  dm_lx3,     dm_lx4,     wpc_dcs,    wpc_dcs, wpc_dcs_state, init_dm,    ROT0,   "Williams",  "Demolition Man (LX-3)",                            MACHINE_IS_SKELETON_MECHANICAL )
GAME(1995,  dm_h5,      dm_lx4,     wpc_dcs,    wpc_dcs, wpc_dcs_state, init_dm,    ROT0,   "Williams",  "Demolition Man (H-5)",                             MACHINE_IS_SKELETON_MECHANICAL )
GAME(1995,  dm_h6,      dm_lx4,     wpc_dcs,    wpc_dcs, wpc_dcs_state, init_dm,    ROT0,   "Williams",  "Demolition Man (H-6)",                             MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  ij_l7,      0,          wpc_dcs,    wpc_dcs, wpc_dcs_state, init_ij,    ROT0,   "Williams",  "Indiana Jones (L-7)",                              MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  ij_lg7,     ij_l7,      wpc_dcs,    wpc_dcs, wpc_dcs_state, init_ij,    ROT0,   "Williams",  "Indiana Jones (LG-7)",                             MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  ij_l6,      ij_l7,      wpc_dcs,    wpc_dcs, wpc_dcs_state, init_ij,    ROT0,   "Williams",  "Indiana Jones (L-6)",                              MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  ij_l5,      ij_l7,      wpc_dcs,    wpc_dcs, wpc_dcs_state, init_ij,    ROT0,   "Williams",  "Indiana Jones (L-5)",                              MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  ij_l4,      ij_l7,      wpc_dcs,    wpc_dcs, wpc_dcs_state, init_ij,    ROT0,   "Williams",  "Indiana Jones (L-4)",                              MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  ij_l3,      ij_l7,      wpc_dcs,    wpc_dcs, wpc_dcs_state, init_ij,    ROT0,   "Williams",  "Indiana Jones (L-3)",                              MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  jd_l7,      0,          wpc_dcs,    wpc_dcs, wpc_dcs_state, init_jd,    ROT0,   "Bally",     "Judge Dredd (L-7)",                                MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  jd_l1,      jd_l7,      wpc_dcs,    wpc_dcs, wpc_dcs_state, init_jd,    ROT0,   "Bally",     "Judge Dredd (L-1)",                                MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  jd_l6,      jd_l7,      wpc_dcs,    wpc_dcs, wpc_dcs_state, init_jd,    ROT0,   "Bally",     "Judge Dredd (L-6)",                                MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  jd_l5,      jd_l7,      wpc_dcs,    wpc_dcs, wpc_dcs_state, init_jd,    ROT0,   "Bally",     "Judge Dredd (L-5)",                                MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  jd_l4,      jd_l7,      wpc_dcs,    wpc_dcs, wpc_dcs_state, init_jd,    ROT0,   "Bally",     "Judge Dredd (L-4)",                                MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  pop_lx5,    0,          wpc_dcs,    wpc_dcs, wpc_dcs_state, init_pop,   ROT0,   "Bally",     "Popeye Saves The Earth (LX-5)",                    MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  pop_la4,    pop_lx5,    wpc_dcs,    wpc_dcs, wpc_dcs_state, init_pop,   ROT0,   "Bally",     "Popeye Saves The Earth (LA-4)",                    MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  pop_pa3,    pop_lx5,    wpc_dcs,    wpc_dcs, wpc_dcs_state, init_pop,   ROT0,   "Bally",     "Popeye Saves The Earth (PA-3)",                    MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  sttng_l7,   0,          wpc_dcs,    wpc_dcs, wpc_dcs_state, init_sttng, ROT0,   "Williams",  "Star Trek: The Next Generation (LX-7)",            MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  sttng_l5,   sttng_l7,   wpc_dcs,    wpc_dcs, wpc_dcs_state, init_sttng, ROT0,   "Williams",  "Star Trek: The Next Generation (LX-5)",            MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  sttng_x7,   sttng_l7,   wpc_dcs,    wpc_dcs, wpc_dcs_state, init_sttng, ROT0,   "Williams",  "Star Trek: The Next Generation (LX-7 Special)",    MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  sttng_p8,   sttng_l7,   wpc_dcs,    wpc_dcs, wpc_dcs_state, init_sttng, ROT0,   "Williams",  "Star Trek: The Next Generation (P-8)",             MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  sttng_p5,   sttng_l7,   wpc_dcs,    wpc_dcs, wpc_dcs_state, init_sttng, ROT0,   "Williams",  "Star Trek: The Next Generation (P-5)",             MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  sttng_p4,   sttng_l7,   wpc_dcs,    wpc_dcs, wpc_dcs_state, init_sttng, ROT0,   "Williams",  "Star Trek: The Next Generation (P-4)",             MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  sttng_s7,   sttng_l7,   wpc_dcs,    wpc_dcs, wpc_dcs_state, init_sttng, ROT0,   "Williams",  "Star Trek: The Next Generation (LX-7) SP1",        MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  sttng_g7,   sttng_l7,   wpc_dcs,    wpc_dcs, wpc_dcs_state, init_sttng, ROT0,   "Williams",  "Star Trek: The Next Generation (LG-7)",            MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  sttng_l1,   sttng_l7,   wpc_dcs,    wpc_dcs, wpc_dcs_state, init_sttng, ROT0,   "Williams",  "Star Trek: The Next Generation (LX-1)",            MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  sttng_l2,   sttng_l7,   wpc_dcs,    wpc_dcs, wpc_dcs_state, init_sttng, ROT0,   "Williams",  "Star Trek: The Next Generation (LX-2)",            MACHINE_IS_SKELETON_MECHANICAL )
GAME(1994,  sttng_l3,   sttng_l7,   wpc_dcs,    wpc_dcs, wpc_dcs_state, init_sttng, ROT0,   "Williams",  "Star Trek: The Next Generation (LX-3)",            MACHINE_IS_SKELETON_MECHANICAL )
GAME(1993,  afv_l4,     0,          wpc_dcs,    wpc_dcs, wpc_dcs_state, init_afv,   ROT0,   "Williams",  "Addams Family Values (Coin Dropper L-4)",          MACHINE_IS_SKELETON_MECHANICAL )
