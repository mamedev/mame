// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Miodrag Milanovic
// Thanks to the PinMAME guys for all their research
/******************************************************************************************
PINBALL
Williams WPC with DCS and Security PIC.

The security PIC code is generally XYY 123456 12345 123 (where XYY = first,4th,5th digit of
 the model number), but there are exceptions.

If it says FACTORY SETTINGS, press F3, and wait for the game attract mode to commence.
To increase the volume, open the coin door (NUM2), press NUM+, close coin door (NUM2).
If coin door not closed, ball can't be ended.

Here are the key codes to enable play:

Game                             NUM  Start game              End ball
--------------------------------------------------------------------------------------------------
Red & Ted's Road Show          50024  Hold QWER hit 1         QWER
No Fear Dangerous Sports       50025  Hold QWER hit 1         QWER
Indianapolis 500               50026  Hold QWER hit 1         QWER
The Flintstones                50029  Hold WER hit 1          WER then let go
Dirty Harry                    50030  Hold QWER hit 1         QWER
World Cup Soccer               50031  Hold QWER hit 1         QWER
The Shadow                     50032  Hold QWERT hit 1        QWERT
Corvette                       50036  Hold QWER hit 1         QWER
Theatre of Magic               50039  Hold QWER hit 1         QWER
Johnny Mnemonic                50042  Hold QWER hit 1         QWER
Who Dunnit                     50044  Hold QWER hit 1         QWER
Jack*Bot                       50051  Hold QWER hit 1         QWER
The Pinball Circus             60020  (not emulated)

ToDo:
- Mechanical sounds
- Sound Interface Error

*********************************************************************************************/

#include "emu.h"

#include "wpc_dmd.h"
#include "wpc_lamp.h"
#include "wpc_out.h"
#include "wpc_pic.h"
#include "wpc_shift.h"

#include "dcs.h"

#include "cpu/m6809/m6809.h"
#include "machine/nvram.h"

#include "speaker.h"

namespace {

class wpc_s_state : public driver_device
{
public:
	wpc_s_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dcs(*this, "dcs")
		, m_rombank(*this, "rombank")
		, m_mainram(*this, "mainram")
		, m_nvram(*this, "nvram")
		, m_pic(*this, "pic")
		, m_lamp(*this, "lamp")
		, m_out(*this, "out")
	{ }

	void corv(machine_config &config) ATTR_COLD;
	void dh(machine_config &config) ATTR_COLD;
	void i500(machine_config &config) ATTR_COLD;
	void jb(machine_config &config) ATTR_COLD;
	void jm(machine_config &config) ATTR_COLD;
	void nf(machine_config &config) ATTR_COLD;
	void rs(machine_config &config) ATTR_COLD;
	void fs(machine_config &config) ATTR_COLD;
	void ts(machine_config &config) ATTR_COLD;
	void tom(machine_config &config) ATTR_COLD;
	void tom14(machine_config &config) ATTR_COLD;
	void wd(machine_config &config) ATTR_COLD;
	void wcs(machine_config &config) ATTR_COLD;
	void tfs(machine_config &config) ATTR_COLD;

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void wpc_s(machine_config &config) ATTR_COLD;

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

	void scanline_irq(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(zc_timer);

	void wpc_s_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<dcs_audio_8k_device> m_dcs;
	required_memory_bank m_rombank;
	required_shared_ptr<uint8_t> m_mainram;
	required_device<nvram_device> m_nvram;
	required_device<wpc_pic_device> m_pic;
	required_device<wpc_lamp_device> m_lamp;
	required_device<wpc_out_device> m_out;

	static const char *const lamps_corv[8][8];
	static const char *const outputs_corv[54];
	static const char *const lamps_dh[8][8];
	static const char *const outputs_dh[54];
	static const char *const lamps_i500[8][8];
	static const char *const outputs_i500[54];
	static const char *const lamps_jb[8][8];
	static const char *const outputs_jb[54];
	static const char *const lamps_jm[8][8];
	static const char *const outputs_jm[54];
	static const char *const lamps_nf[8][8];
	static const char *const outputs_nf[54];
	static const char *const lamps_rs[8][8];
	static const char *const outputs_rs[54];
	//static const char *const lamps_fs[8][8];
	//static const char *const outputs_fs[54];
	//static const char *const lamps_ts[8][8];
	//static const char *const outputs_ts[54];
	//static const char *const lamps_tom[8][8];
	//static const char *const outputs_tom[54];
	//static const char *const lamps_wd[8][8];
	//static const char *const outputs_wd[54];
	//static const char *const lamps_wcs[8][8];
	//static const char *const outputs_wcs[54];
	uint8_t m_firq_src = 0U, m_zc = 0U;
	uint16_t m_rtc_base_day = 0U;
};

void wpc_s_state::wpc_s_map(address_map &map)
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

	map(0x3fdc, 0x3fdc).rw(FUNC(wpc_s_state::dcs_data_r), FUNC(wpc_s_state::dcs_data_w));
	map(0x3fdd, 0x3fdd).rw(FUNC(wpc_s_state::dcs_ctrl_r), FUNC(wpc_s_state::dcs_reset_w));

	map(0x3fe0, 0x3fe3).w(m_out, FUNC(wpc_out_device::out_w));
	map(0x3fe4, 0x3fe4).nopr().w(m_lamp, FUNC(wpc_lamp_device::row_w));
	map(0x3fe5, 0x3fe5).nopr().w(m_lamp, FUNC(wpc_lamp_device::col_w));
	map(0x3fe6, 0x3fe6).w(m_out, FUNC(wpc_out_device::gi_w));
	map(0x3fe7, 0x3fe7).portr("DSW");
	map(0x3fe8, 0x3fe8).portr("DOOR");
	map(0x3fe9, 0x3fe9).r(m_pic, FUNC(wpc_pic_device::read));
	map(0x3fea, 0x3fea).w(m_pic, FUNC(wpc_pic_device::write));

	map(0x3ff2, 0x3ff2).w(m_out, FUNC(wpc_out_device::led_w));
	map(0x3ff3, 0x3ff3).nopr().w(FUNC(wpc_s_state::irq_ack_w));
	map(0x3ff4, 0x3ff7).m("shift", FUNC(wpc_shift_device::registers));
	map(0x3ff8, 0x3ff8).r(FUNC(wpc_s_state::firq_src_r)).nopw(); // ack?
	map(0x3ffa, 0x3ffb).r(FUNC(wpc_s_state::rtc_r));
	map(0x3ffc, 0x3ffc).w(FUNC(wpc_s_state::bank_w));
	map(0x3ffd, 0x3ffe).noprw(); // memory protection stuff?
	map(0x3fff, 0x3fff).rw(FUNC(wpc_s_state::zc_r), FUNC(wpc_s_state::watchdog_w));

	map(0x4000, 0x7fff).bankr("rombank");
	map(0x8000, 0xffff).rom().region("maincpu", 0x78000);
}

uint8_t wpc_s_state::dcs_data_r()
{
	return m_dcs->data_r();
}

void wpc_s_state::dcs_data_w(uint8_t data)
{
	m_dcs->data_w(data);
}

uint8_t wpc_s_state::dcs_ctrl_r()
{
	return m_dcs->control_r();
}

void wpc_s_state::dcs_reset_w(uint8_t data)
{
	m_dcs->reset_w(0);
	m_dcs->reset_w(1);
}


uint8_t wpc_s_state::rtc_r(offs_t offset)
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

uint8_t wpc_s_state::firq_src_r()
{
	return m_firq_src;
}

uint8_t wpc_s_state::zc_r()
{
	uint8_t res = m_zc;
	m_zc &= 0x7f;
	return res;
}

TIMER_DEVICE_CALLBACK_MEMBER(wpc_s_state::zc_timer)
{
	m_zc |= 0x80;
}

void wpc_s_state::bank_w(uint8_t data)
{
	m_rombank->set_entry(data & 0x1f);
}

void wpc_s_state::watchdog_w(uint8_t data)
{
	// Mhhh?  Maybe it's not 3ff3, maybe it's going down by itself...
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

void wpc_s_state::scanline_irq(int state)
{
	m_firq_src = 0x00;
	m_maincpu->set_input_line(1, state);
}

void wpc_s_state::irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_maincpu->set_input_line(1, CLEAR_LINE);
}

void wpc_s_state::machine_start()
{
	m_rombank->configure_entries(0, 0x20, memregion("maincpu")->base(), 0x4000);
	m_nvram->set_base(m_mainram, m_mainram.bytes());

	save_item(NAME(m_firq_src));
	save_item(NAME(m_zc));
}

void wpc_s_state::machine_reset()
{
	m_firq_src = 0x00;
	m_zc = 0x00;

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

const char *const wpc_s_state::lamps_corv[][8] = {
	{ "l:L rollover",       "l:M rollover",    "l:R rollover",      "l:Skid pad arrow",  "l:Sticky tires",    "l:Skid pad jackpot", "l:Route 66 arrow", "l:Race today"      },
	{ "l:Inner loop arrow", "l:Fuelie",        "l:Nitrous",         "l:In loop jackpot", "l:R O loop arrow",  "l:ZO7 suspension",   "l:Big brakes",     "l:Super charger"   },
	{ "l:L O loop arrow",   "l:Lite lock",     "l:Qualify",         "l:Big block",       "l:ZR1 ramp lock",   "l:6 speed trans",    "l:Hi lift cams",   "l:ZR1 ramp arrow"  },
	{ "l:Corvette 6",       "l:Corvette 3",    "l:Corvette 1",      "l:Corvette 2",      "l:Corvette 4",      "l:L standup 3",      "l:L standup 2",    "l:L standup 1"     },
	{ "l:Corvette 9",       "l:Corvette 8",    "l:Pit stop",        "l:Corvette 7",      "l:Corvette 5",      "l:Pit stop arrow",   "l:Spinner arrow",  "l:Drive again"     },
	{ "l:L outer tail",     "l:L inner tail",  "l:Catch me",        "l:R inner tail",    "l:R outer tail",    "l:R standup arrow",  "l:Lite kickback",  "l:Start challenge" },
	{ "l:Kickback arrow",   "l:L return lane", "l:R return lane",   "l:R out lane",      "l:Million standup", "l:Side pipe 1",      "l:Side pipe 2",    "l:Side pipe 3"     },
	{ "l:R tree red",       "l:L tree red",    "l:Tree bot yellow", "l:Tree top yellow", "l:R tree green",    "l:L tree green",     "l:Buy in",         "l:Start button"    }
};

const char *const wpc_s_state::outputs_corv[] = {
	/*  1 */ "s:Trough eject",   "s:ZR1 low rev gate", "s:Kickback",      "s:Pit stop popper", "s:ZR1 up rev gate", "u:06", /* dummy */ "s:Knocker",        "s:Route 66 kickout",
	/*  9 */ "s:L slingshot",    "s:R slingshot",      "s:Left jet",      "s:Bottom jet",      "s:Right jet",       "u:14", /* dummy */ "s:ZR1 lockup",     "s:Loop gate",
	/* 17 */ "s:Race direction", "s:L race enable",    "s:R race enable", "f:Tenth corvette",  "f:Jets",            "f:R ramps",        "f:U L flipper",    "f:Catch me",
	/* 25 */ "f:ZR1 ramp",       "f:ZR1 underside",    "f:R rear panel",  "f:R standup",       "s:R flip power",    "s:R flip hold",    "s:L flip power",   "s:L flip hold",
	/* 33 */ "s:Diverter power", "s:Diverter hold",    "s:UL flip power", "s:UL flip hold",    "u:37", /* dummy */  "u:38", /* dummy */ "u:39", /* dummy */ "u:40", /* dummy */
	/* 41 */ "u:41", /* dummy */
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Upper left", "g:Upper right", "g:Lower left", "g:Lower right", "g:Back box title"
};

static INPUT_PORTS_START( corv )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Left out lane")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Right out lane")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Plunger")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("L return lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("R return lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Spinner")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Buy in button")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("1st gear (opt)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("2nd gear (opt)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("3rd gear (opt)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("4th gear (opt)")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Route 66 entry")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Pit stop popper")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Trough eject")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Inner loop entry")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("ZR1 bottom entry")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("ZR1 top entry")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Skid pad entry")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Skid pad exit")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Route 66 exit")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("L standup 3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("L standup 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("L standup 1")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("L race start")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("R race start")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("L race encoder")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("R race encoder")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Route 66 kickout")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Skid rte66 exit")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("L slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("R slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Bottom jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Right jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("L rollover")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("M rollover")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("R rollover")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("ZR1 full left")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("ZR1 full right")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("ZR1 exit")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("ZR1 lock ball 1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("ZR1 lock ball 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("ZR1 lock ball 3")

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Million standup")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Skid pad standup")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("R standup")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LALT) PORT_NAME("R rubber")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RALT) PORT_NAME("Jet rubber")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("L outer loop")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("R outer loop")

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper EOS")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_s_state::lamps_dh[][8] = {
	{ "l:Left rollover",   "l:Middle rollover", "l:Right rollover",   "l:Magnum jets",    "l:Magnum bullets",   "l:Lite extra ball",  "l:Lite shootout",  "l:Playfield promo" },
	{ "l:R ramp badge",    "l:Sil 6 bullet",    "l:R loop generic",   "l:Magna force",    "l:R ramp generic",   "l:R ramp jackpot",   "l:R loop HQ",      "l:Warehouse badge" },
	{ "l:Barroom brawl",   "l:Car chase",       "l:Warehouse raid",   "l:Letter bomb",    "l:Meet the mob",     "l:Stop scorpio",     "l:Crime wave",     "l:Bank rbr hry up" },
	{ "l:Safehouse badge", "l:L ramp badge",    "l:Sil 3 bullet",     "l:Super jackpot",  "l:L ramp generic",   "l:Ramp start mball", "l:Magazine award", "l:Contraband"      },
	{ "l:Safehouse arrow", "l:Sil 4 bullet",    "l:Sil 5 bullet",     "l:L loop HQ",      "l:Whse start mball", "l:Feel lucky",       "l:Right shootout", "l:Lite ransom"     },
	{ "l:L loop generic",  "l:Ricochet",        "l:Extra ball",       "l:HQ badge",       "l:Ransom",           "l:Sil 1 bullet",     "l:HQ",             "l:Sil 2 bullet"    },
	{ "l:60", /* dummy */  "l:61", /* dummy */  "l:62", /* dummy */   "l:63", /* dummy */ "l:64", /* dummy */   "l:65", /* dummy */   "l:Shhot again",    "l:Sil 7 bullet"    },
	{ "l:Sil 8 bullet",    "l:Left shootout",   "l:Lite magna force", "l:L/R jets",       "l:Bottom jet",       "l:Body armor",       "l:Extra ball",     "l:Start"           }
};

const char *const wpc_s_state::outputs_dh[] = {
	/*  1 */ "s:Ball release",  "s:Autoplunger",  "s:Gun launch",    "s:Top R popper",    "s:Gun popper",      "s:Drop target down", "s:Knocker",        "s:Trap door hi",
	/*  9 */ "s:Left sling",    "s:Right sling",  "s:Left jet",      "s:Middle jet",      "s:Right jet",       "s:Left popper",      "s:Right diverter", "s:Trap door hold",
	/* 17 */ "f:Headquarters",  "f:Top L popper", "f:Warehouse",     "s:Gun motor",       "f:Gun loaded",      "f:Right ramp",       "f:Right back",     "f:Left back",
	/* 25 */ "s:Drop reset",    "s:Top L popper", "s:Left diverter", "s:Right loop gate", "s:R flip power",    "s:R flip hold",      "s:L flip power",   "s:L flip hold",
	/* 33 */ "s:UR flip power", "s:UR flip hold", "s:R loop magnet", "s:Left loop gate",  "u:37", /* dummy */  "u:38", /* dummy */   "u:39", /* dummy */ "u:40", /* dummy */
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Right string", "g:Left string", "g:Backbox title", "g:Backbox face", "g:Bottom string"
};

static INPUT_PORTS_START( dh )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Gun handle trig")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Shooter lane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Right outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Right inlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Standup 8")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Ex ball button")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Left inlane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Standup 1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Standup 2")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Trough jam")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Right ramp make")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Left ramp enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Right loop")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Left ramp make")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Gun chamber")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Gun popper")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Warehouse popper")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Left popper")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Right ramp enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Drop target down")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Standup 6")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Standup 7")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Standup 5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Standup 4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Standup 3")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Right sling")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Middle jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Right jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Left rollover")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Middle rollover")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Right rollover")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Left loop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Top L popper")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Gun position")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Gun lockup")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper EOS")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_s_state::lamps_i500[][8] = {
	{ "l:Left lane",        "l:Center lane",      "l:Right lane",       "l:Upper eject top",  "l:Jet wrench",     "l:Extra ball",     "l:Victory lap",      "l:Turbo wrench"     },
	{ "l:Turbo lock 1",     "l:Turbo lock 2",     "l:Turbo lock 3",     "l:Light lock lamp",  "l:Light speedway", "l:\"Pass\"",       "l:Left ramp wrench", "l:Lft ramp standup" },
	{ "l:Hit the \"wall\"", "l:Hit \"the\" wall", "l:\"Hit\" the wall", "l:Lft ramp jackpot", "l:Increase boost", "l:Souvenir lamp",  "l:Left flip lane",   "l:Left outlane"     },
	{ "l:Super jets",       "l:Turbo boost",      "l:Checkered flag",   "l:Go for the pole",  "l:Quick pit",      "l:3X playfield",   "l:UR flip wrench",   "l:Right flip lane"  },
	{ "l:Dueling drivers",  "l:Super lightups",   "l:Caution flag",     "l:Extra ball flag",  "l:Wrong turn",     "l:Gasoline alley", "l:Right outlane",    "l:Shoot again"      },
	{ "l:Change setup",     "l:Award speedway",   "l:Hit the wall",     "l:Rt ramp jackpot",  "l:Pit stop",       "l:Fast laps",      "l:56", /* dummy */   "l:57" /* dummy */   },
	{ "l:Stnd1 lower RT",   "l:Stnd1 upper RT",   "l:Stnd1 upper left", "l:Stnd1 lower left", "l:Stnd2 lower RT", "l:Stnd2 upper RT", "l:Stnd2 upper left", "l:Stnd2 lower left" },
	{ "l:Stnd3 lower RT",   "l:Stnd3 upper RT",   "l:Stnd3 upper left", "l:Stnd3 lower left", "l:74", /* dummy */ "l:Launch button",  "l:Buy-in button",    "l:Start button"     }
};

const char *const wpc_s_state::outputs_i500[] = {
	/*  1 */ "s:Auto plunger",     "s:Upper popper",     "s:Upper eject",    "s:Lower eject",   "s:Turbo popper",    "u:06", /* dummy */ "s:Knocker",          "s:Left jet",
	/*  9 */ "s:Right jet",        "s:Center jet",       "s:Left sling",     "s:Right sling",   "s:Trough",          "u:14", /* dummy */ "f:Upper popper fls", "f:Top left corner",
	/* 17 */ "f:Top right corner", "s:Race track motor", "f:Orange car",     "f:Yellow car",    "f:Blue car",        "f:Green car",      "f:Lft jet flasher",  "f:Rt jet flasher",
	/* 25 */ "f:Cntr jet flasher", "f:Right side",       "f:Left side (2)",  "f:Rt ramp enter", "s:R flip power",    "s:R flip hold",    "s:L flip power",     "s:L flip hold",
	/* 33 */ "s:UR flip power",    "s:UR flip hold",     "s:Diverter power", "s:Diverter hold", "u:37", /* dummy */  "u:38", /* dummy */ "u:39", /* dummy */   "u:40", /* dummy */
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Upper lft plyfld", "g:Upper rt plyfld", "g:Lower playfield", "g:Backbox", "g:Title-coindoor"
};

static INPUT_PORTS_START( i500 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Ball launch")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Left outlane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Left flip lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Right flip lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Right outlane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Buy-in button")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Shooter lane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Left slingshot")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Right slingshot")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Three bank upper")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Three bank center")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Three bank lower")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Rt flip wrench")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Left ramp enter")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Left ramp made")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Left loop")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Right loop")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Top trough")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Left ramp standup")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Turbo wrench")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Jet bumpr wrench")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Left lane")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Center lane")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Right lane")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Ten point")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Lest ramp wrench")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Left light-up")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Center light-up")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Right light-up")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Upper popper")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Turbo popper")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Turbo ball sense")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Upper eject")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Lower kicker")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Turbo index")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Left jet")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Right jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Center jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right ramp enter")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Right ramp made")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper EOS")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_s_state::lamps_jb[][8] = {
	{ "l:Yellow arrow",    "l:Yellow 1 (hi)",    "l:Yellow 2",        "l:Yellow 3",         "l:Yellow 4",    "l:Yellow 5 (low)",  "l:Left outlane",     "l:L flipper lane"   },
	{ "l:Blue arrow",      "l:Blue 1 (hi)",      "l:Blue 2",          "l:Blue 3",           "l:Blue 4",      "l:Blue 5 (low)",    "l:Bonus 2X",         "l:Bonus 4X"         },
	{ "l:Amber arrow",     "l:Amber 1 (hi)",     "l:Amber 2",         "l:Amber 3",          "l:Amber 4",     "l:Amber 5 (low)",   "l:Shoot again",      "l:Bonus 5X"         },
	{ "l:Green arrow",     "l:Green 1 (hi)",     "l:Green 2",         "l:Green 3",          "l:Green 4",     "l:Green 5 (low)",   "l:Bonus 3X",         "l:Jack*Bot target"  },
	{ "l:Red arrow",       "l:Red 1 (hi)",       "l:Red 2",           "l:Red 3",            "l:Red 4",       "l:Red 5 (low)",     "l:R flipper lane",   "l:Right outlane"    },
	{ "l:Card 1 (L)",      "l:Card 2",           "l:Card 3",          "l:Card 4",           "l:Card 5 (R)",  "l:Casino run",      "l:Hit me",           "l:Low drop target"  },
	{ "l:Cashier mini-PF", "l:Meg ramp mini-PF", "l:Lite extra ball", "l:Jack*Bot mini-PF", "l:Game saucer", "l:Mega ramp",       "l:High drop target", "l:Cent drop target" },
	{ "l:Pinbot poker",    "l:Slot machine",     "l:Roll the dice",   "l:Keno",             "l:Cashier",     "l:Jack*Bot (ramp)", "l:Buy in button",    "l:Start button"     }
};

const char *const wpc_s_state::outputs_jb[] = {
	/*  1 */ "s:Ball release",   "u:02", /* dummy */  "s:Game saucer",      "s:Drop targets",    "s:Right eject hole", "s:Raise ramp",     "s:Knocker",          "s:Left eject hole",
	/*  9 */ "s:Left slingshot", "s:Right slingshot", "s:Lower jet bumper", "s:Left jet bumper", "s:Upper jet bumper", "s:Drop ramp",      "f:Right visor",      "f:Left visor",
	/* 17 */ "f:Center visor",   "f:Pinbot face",     "f:Jet bumpers",      "f:Lower left",      "f:Mid left",         "f:Lower right",    "f:Back panel 1 (L)", "f:Back panel 2",
	/* 25 */ "f:Back panel 3",   "f:Back panel 4",    "f:Back panel 5 (R)", "u:28", /* dummy */  "s:R flip power",     "s:R flip hold",    "s:L flip power",     "s:L flip hold",
	/* 33 */ "u:33", /* dummy */ "u:34", /* dummy */  "u:35", /* dummy */   "u:36", /* dummy */  "u:37", /* dummy */   "u:38", /* dummy */ "u:39", /* dummy */   "u:40", /* dummy */
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Playfield lower", "g:Playfield left", "g:Playfield upper", "g:Playfield right", "g:Insert"
};

static INPUT_PORTS_START( jb )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("L left 10 point")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("U left 10 point")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Ramp is down")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("High drop target")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Center drop target")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Low drop target")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Buy extra ball")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Left outlane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("L flipper lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("R flipper lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Right outlane")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Trough jam")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Ramp exit")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Ramp entrance")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Target under ramp")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Visor 1 (left)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Visor 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Visor 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Visor 4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Visor 5 (right)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Far Left Eject")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Left eject hole")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Right eject hole")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("5-bank 1 (upper)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("5-bank target 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("5-bank target 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("5-bank target 4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("5-bank 5 (lower)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Vortex upper")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Vortex center")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Vortex lower")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Upper jet bumper")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Left jet bumper")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Lower jet bumper")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Right slingshot")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Left slingshot")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Right 10 point")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Hit me target")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Ball shooter")

	PORT_START("X6")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Visor is closed")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Visor is open")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_s_state::lamps_jm[][8] = {
	{ "l:Mode ready",       "l:Download",        "l:Access code 2",   "l:Access code 1",    "l:Upload",         "l:Left jet lane",    "l:Middle jet lane", "l:Right jet lane"  },
	{ "l:Power down",       "l:NAS cure",        "l:R ramp block 4",  "l:Sector 6",         "l:R ramp block 2", "l:Hold bonus",       "l:R standup R blk", "l:R standup L blk" },
	{ "l:L ramp block 4",   "l:Extra ball",      "l:Sector 2",        "l:L ramp block 2",   "l:L ramp block 1", "l:Sector 1",         "l:26", /* dummy */  "l:Shoot again"     },
	{ "l:L loop top arrow", "l:L standup arrow", "l:R ramp block 1",  "l:Lite spinner",     "l:Big points",     "l:Gigabytes",        "l:Lite extra ball", "l:Quick multiball" },
	{ "l:Cyber matrix 13",  "l:Cyber matrix 23", "l:Cyber matrix 33", "l:Right outlane",    "l:Bonus held",     "l:Bonus 4x",         "l:Bonus 3x",        "l:Bonus 2x"        },
	{ "l:Cyber matrix 12",  "l:Cyber matrix 22", "l:Cyber matrix 32", "l:Right flip lane",  "l:Sector 5",       "l:Spinner millions", "l:Cyber lock 2",    "l:Inner loop top"  },
	{ "l:Cyber matrix 11",  "l:Cyber matrix 21", "l:Cyber matrix 31", "l:Popper top arrow", "l:Sector 3",       "l:Crazy Bob's",      "l:Mode start",      "l:Cyber lock 1"    },
	{ "l:R loop top arrow", "l:Cyber lock 3",    "l:Sector 7",        "l:Left outlane",     "l:Left flip lane", "l:Ball launch",      "l:Buy-in button",   "l:Start button"    }
};

const char *const wpc_s_state::outputs_jm[] = {
	/*  1 */ "s:Trough eject",     "s:Autoplunger",     "s:Popper",           "u:04", /* dummy */  "s:Clear matrix",    "s:Hand magnet",    "s:Knocker",         "u:08", /* dummy */
	/*  9 */ "s:Left sling",       "s:Right sling",     "s:Left jet",         "s:Bottom jet",      "s:Right jet",       "s:Crazy Bob's",    "s:Drop target up",  "s:Drop target down",
	/* 17 */ "f:Jets",             "f:Crazy Bob's",     "f:Left sling",       "f:Right sling",     "s:X mot direction", "s:X motor enable", "s:Y mot direction", "s:Y motor enable",
	/* 25 */ "f:Left ramp",        "f:Right ramp",      "f:Hand popper",      "f:R backpanel",     "s:R flip power",    "s:R flip hold",    "s:L flip power",    "s:L flip hold",
	/* 33 */ "s:L diverter power", "s:L diverter hold", "s:R diverter power", "s:R diverter hold", "u:37", /* dummy */  "u:38", /* dummy */ "u:39", /* dummy */  "u:40", /* dummy */
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:String 1", "g:String 2", "g:String 3", "g:String 4", "g:String 5"
};

static INPUT_PORTS_START( jm )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Ball launch")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("X hand home")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Left outlane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Left flip lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Right flip lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Right outlane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Buy in button")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("L slingshot")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("R slingshot")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("L standup")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("R standup")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Trough jam")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Popper ball 1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Y hand home")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("R rubber")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Left ramp enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Left ramp made")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Drop target")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Left jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Bottom jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Right jet")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Crazy Bob's")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Spinner")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Cyber matrix 11")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Cyber matrix 21")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Cyber matrix 31")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Right ramp enter")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Right ramp made")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Left loop")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Right loop")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Inner loop entry")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Cyber matrix 12")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Cyber matrix 22")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Cyber matrix 32")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Left jet lane")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Middle jet lane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Right jet lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("R hand control")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("L hand control")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Cyber matrix 13")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Cyber matrix 23")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Cyber matrix 33")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("X encoder A")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("X encoder B")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Y encoder B")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Y encoder A")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Shooter lane")

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Ball in hand")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_s_state::lamps_nf[][8] = {
	{ "l:15 million",   "l:10 million",      "l:5 million",        "l:L over the edge",  "l:Dirt",             "l:Alphalt",        "l:No limits",      "l:Water"           },
	{ "l:No fear",      "l:20 million",      "l:Light extra ball", "l:R over the edge",  "l:Air",              "l:Snow",           "l:Fear fest",      "l:Meet your maker" },
	{ "l:Skull",        "l:R track",         "l:Center jackpot",   "l:Extra ball",       "l:Start challenge",  "l:Center lock",    "l:R autofire (2)", "l:L hurry up"      },
	{ "l:L track",      "l:1st",             "l:2nd",              "l:3rd",              "l:Skydive",          "l:Drop jackpot",   "l:Drop lock",      "l:L autofire (2)"  },
	{ "l:Raceway",      "l:L ramp \"turn\"", "l:Super spin",       "l:L ramp \"start\"", "l:L ramp \"win!\"",  "l:Hill climb",     "l:Screamer",       "l:47" /* dummy */  },
	{ "l:Tube",         "l:Video mode",      "l:R ramp \"win!\"",  "l:R ramp \"turn\"",  "l:R ramp \"start\"", "l:L flipper lane", "l:L outlane",      "l:Kickback"        },
	{ "l:Light kb top", "l:Light kb bottom", "l:R flipper lane",   "l:R outlane",        "l:Hairpin",          "l:Downhill",       "l:Summit",         "l:R hurry up"      },
	{ "l:Shoot again",  "l:L skull eye",     "l:Jump now",         "l:Super jackpot",    "l:R skull eye",      "l:Ball launch",    "l:Buy-in button",  "l:Start button"    }
};

const char *const wpc_s_state::outputs_nf[] = {
	/*  1 */ "s:Right popper",     "s:Auto plunger",    "s:Right magnet",    "s:Kickback",       "s:Center magnet",   "s:Left magnet",    "s:Knocker",          "s:Drop target down",
	/*  9 */ "u:09", /* dummy */   "s:Right slingshot", "s:Left slingshot",  "s:Drop target up", "u:13", /* dummy */  "s:Trough",         "s:Eject",            "s:Skull mouth",
	/* 17 */ "f:Fls(2) flip rtrn", "f:Fls spinner",     "f:Fls no fear",     "f:Fls(3) rt ramp", "f:Fls(2) skull",    "f:Fls bkbox expl", "f:Fls(3) left ramp", "f:Fls top left",
	/* 25 */ "f:Fls(2) auto-fire", "f:Fls bkbox L top", "f:Fls bkbox R top", "f:Fls rt popper",  "s:R flip power",    "s:R flip hold",    "s:L flip power",     "s:L flip hold",
	/* 33 */ "s:UR flip power",    "s:UR flip hold",    "u:35", /* dummy */  "u:36", /* dummy */ "u:35", /* dummy */  "u:36", /* dummy */ "u:39", /* dummy */   "u:40", /* dummy */
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Playfield top", "g:Playfield right", "g:Playfield left", "g:Insert title", "g:Insert bkground"
};

static INPUT_PORTS_START( nf )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Ball launch")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Shooter lane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Spinner")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Right return")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Buy extra ball")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Kickback")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Left return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Left slingshot")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Right slingshot")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Trough stack")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1 (right)")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Center trough enter")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Left trough enter")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Right popper 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Right popper 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Left magnet")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Center magnet")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Right magnet")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Drop target")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Left wireform")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Inner loop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Light kb bottom")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Light kb top")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Right loop")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Eject hole")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Left loop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Left ramp enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Left ramp middle")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Right ramp enter")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Right ramp exit")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper EOS")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_s_state::lamps_rs[][8] = {
	{ "l:Salt lake",        "l:Denver",           "l:Butte",           "l:Minnesota",          "l:Kansas city",  "l:New york city",    "l:Ohio",               "l:Chicago"            },
	{ "l:Los angeles",      "l:Las vegas",        "l:Albuquerque",     "l:Dallas",             "l:New orleans",  "l:Nashville",        "l:Atlanta",            "l:Miami"              },
	{ "l:San francisco",    "l:Seatle",           "l:Left special",    "l:\"Sh here\" L loop", "l:Shoot again",  "l:Right special",    "l:Blasting zone",      "l:\"Sh here\" R ramp" },
	{ "l:Bad weather",      "l:Jets at max",      "l:Radio (2)",       "l:M plus wheel",       "l:Big millions", "l:Lite special",     "l:Lite big blast",     "l:Flying rocks"       },
	{ "l:Monday",           "l:Spinner at max",   "l:Hold bonus",      "l:Lite EB wheel",      "l:Lunch time!",  "l:Bob's freebie",    "l:\"Sh here\" L ramp", "l:You're there"       },
	{ "l:Wednesday",        "l:Tuesday",          "l:Thursday",        "l:Friday",             "l:Lock",         "l:Extra ball",       "l:\"Sh here\" R loop", "l:Lite Bob's"         },
	{ "l:Bonus 6x",         "l:Bonus 5x",         "l:Bonus 4x",        "l:Bonus 3x",           "l:Bonus 2x",     "l:Lite EB lower",    "l:Start city",         "l:M plus R ramp"      },
	{ "l:F rocks 5x blast", "l:F rocks rad riot", "l:F rocks ex ball", "l:Left bridge out",    "l:Bob's bunker", "l:Right bridge out", "l:Buy in button",      "l:Start button"       }
};

const char *const wpc_s_state::outputs_rs[] = {
	/*  1 */ "s:Trough",             "s:L left diverter",   "s:Lock-up pin",     "s:U left diverter",    "s:U right diverter",  "s:Start city",        "s:Knocker",         "s:Lock kickout",
	/*  9 */ "s:\"Ted\" eyes left",  "s:\"Ted\" lids down", "s:\"Ted\" lids up", "s:\"Ted\" eyes right", "s:\"Red\" lids down", "s:\"Red\" eyes left", "s:\"Red\" lids up", "s:\"Red\" eyes right",
	/* 17 */ "u:17", /* dummy */     "u:18", /* dummy */    "u:19", /* dummy */  "u:20", /* dummy */     "s:Left sling",        "s:Right sling",       "s:Bulldozer motor", "s:\"Red\" eject",
	/* 25 */ "s:Top jet",            "s:Left jet",          "s:Right jet",       "s:Shaker motor",       "u:29", /* dummy */    "u:30", /* dummy */    "u:31", /* dummy */  "u:32", /* dummy */
	/* 33 */ "u:33", /* dummy */     "u:34", /* dummy */    "u:35", /* dummy */  "u:36", /* dummy */     "f:Little flipper",    "f:Left ramp",        "f:Back white",       "f:Back yellow",
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Playfld/insert 1", "g:Playfld/insert 2", "g:Playfld/insert 3", "g:Right playfield", "g:Left playfield"
};

static INPUT_PORTS_START( rs )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Ted's mouth")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Dozer down")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Dozer up")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Right outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Right inlane 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Right inlane 1")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Buy-in")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Red's mouth")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Left inlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Blast zone 3-bank")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Skill shot lower")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Skill shot upper")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Right shooter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Radio 3-bank")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Red standup upper")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Red standup lower")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Hit Red")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Right loop exit")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Trough jam")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Right loop enter")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Hit bulldozer")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Hit Ted")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Spinner")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Lockup 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Lockup 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Lock kickout")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Right ramp exit left")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Left ramp exit")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Left ramp enter")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Left shooter")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Right sling")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Top jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Right jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Right ramp enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Right ramp exit centre")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Flying rocks 5x blast")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Flying rocks radio riot")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Flying rocks extra ball")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Flying rocks top")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Under blast zone")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Start city")

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("White standup")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Red standup")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Yellow standup")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Orange standup")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Middle left flipper top")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Middle left flipper bottom")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

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

#if 0
const char *const wpc_s_state::lamps_fs[][8] = {
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" }
};

const char *const wpc_s_state::outputs_fs[] = {
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:", "g:", "g:", "g:", "g:"
};
#endif

static INPUT_PORTS_START( fs )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Launch button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Ticket dispenser")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Shooter lane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Upper 3-bnk lt tgt T")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Upper 3-bnk cr tgt X")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Upper 3-bnk rt tgt I")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Extra ball button")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Machine exit")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Upper left single target")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Left lane rollover")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Inner left lane rollover")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Trough jam")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Ball popper")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Right ramp entrance")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Left ramp entrance")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("4-bnk drop target 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("4-bnk drop target 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("4-bnk drop target 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("4-bnk drop target 4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("3-bnk drop target 1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("3-bnk drop target 2")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("3-bnk drop target 3")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Centre lane rollover")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("L&R 3-bnk bottom targets")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("L&R 3-bnk middle targets")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("L&R 3-bnk top targets")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Lower left single target")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Right single target")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Dictabird")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Right sling")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Top left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Top right jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Bottom jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Top left rollover D")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Top centre rollover I")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Top right rollover G")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Left out rollover")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Left return rollover")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Right return rollover")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right out rollover")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Upper right lane rollover")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Upper right lane exit rollover")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Right ramp exit")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("Left ramp exit")

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

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

#if 0
const char *const wpc_s_state::lamps_ts[][8] = {
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" }
};

const char *const wpc_s_state::outputs_ts[] = {
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:", "g:", "g:", "g:", "g:"
};
#endif

static INPUT_PORTS_START( ts )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Gun trigger")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Right phurba control")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Right outlane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Right return lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Left return lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Left outlane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Buy-in button")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("'M'ongol")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("m'O'ngol")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("mongo'L'")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("mong'O'l")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Lamp ramp enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Right ramp enter")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Inner sanctum")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Left phurba control")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Left rubber")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Mini kicker")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Mini limit left")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Mini limit right")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Trough 5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Top trough")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Inner loop enter")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Shooter")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Wall target down")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("mo'N'gol")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("mon'G'ol")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Left loop enter")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Battle drop down")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Centre standup")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Right loop enter")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Mini exit tube")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Right sling")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Lockup right")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Lockup middle")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Lockup left")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Left eject")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Right eject")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Popper")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Mini left standup 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Mini left standup 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("Mini left standup 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Mini left standup 4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Left ramp left made")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left ramp right made")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right ramp left made")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Right ramp right made")

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Mini right standup 4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Mini right standup 3")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Mini right standup 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Mini right standup 1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Mini drop left")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Mini drop middle left")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Mini drop middle right")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Mini drop right")

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

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

#if 0
const char *const wpc_s_state::lamps_tom[][8] = {
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" }
};

const char *const wpc_s_state::outputs_tom[] = {
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:", "g:", "g:", "g:", "g:"
};
#endif

static INPUT_PORTS_START( tom )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Shooter lane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Buy-in")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Left outlane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Left return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Right return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Right outlane")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Trough jam")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Subway opto")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Spinner")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Right lower target")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Lock 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Lock 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Lock 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Popper")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Left drain eddy")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Subway micro")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Right drain eddy")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Left bank target")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Captive ball rest")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Right lane enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Left lane enter")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Cube position 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Cube position 1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Cube position 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Cube position 3")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Right sling")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Bottom jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Middle jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Top jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Top lane 1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Top lane 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Centre ramp exit")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Right ramp exit")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Right ramp exit 2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Centre ramp enter")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Right ramp enter")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Captive ball top")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Loop left")

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Loop right")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Centre ramp targets")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Vanish lock 1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("Vanish lock 2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Trunk hit")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Right lane exit")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Left lane exit")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

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

#if 0
const char *const wpc_s_state::lamps_wd[][8] = {
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" }
};

const char *const wpc_s_state::outputs_wd[] = {
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:", "g:", "g:", "g:", "g:"
};
#endif

static INPUT_PORTS_START( wd )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("3-bank position 2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Slot index left")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Shooter lane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Right outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Right inlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Right loop")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Buy-in button")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Slot index centre")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Left inlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Left outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Left loop")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Trough jam")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Enter ramp")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Made ramp left")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Top left hole")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Post jets")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Right back popper")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Lower right popper")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Enter right hole")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Slot index right")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Lockup 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Top 4-bank")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("2nd 4-bank")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("3rd 4-bank")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Bottom 4-bank")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Mystery target")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Lower right lock 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Red")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Right sling")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Bottom jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Right jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Left 3-bank")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Centre 3-bank")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Right 3-bank")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Top 2-bank")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Bottom 2-bank")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("3-bank position up")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Up down ramp")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Scoop centre")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("Scoop right")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Scoop left")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Black")

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

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

#if 0
const char *const wpc_s_state::lamps_wcs[][8] = {
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" },
	{ "", "", "", "", "", "", "", "" }
};

const char *const wpc_s_state::outputs_wcs[] = {
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:", "s:", "s:", "s:", "s:", "s:", "s:", "s:",
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:", "g:", "g:", "g:", "g:"
};
#endif

static INPUT_PORTS_START( wcs )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Magna Goalie button")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Left flipper lane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Striker 3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Right return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Right outlane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START3) PORT_NAME("Buy extra ball")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Free kick target")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Kickback upper")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Spinner")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Light kickback")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Trough 5")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Trough stack")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Light magna goalie")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Ball shooter")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Goal trough")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Goal popper")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Goalie is left")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Goalie is right")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("TV ball popper")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Travel lane rollover")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Goalie target")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Skill shot front")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Skill shot centre")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Skill shot rear")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Right eject hole")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Upper eject hole")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Left eject hole")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Rollover 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Rollover 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Rollover 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Rollover 4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Tackle switch")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Striker 1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Striker 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Left ramp diverted")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Left ramp entrance")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Left ramp exit")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Right ramp entrance")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Lock mech low")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Lock mech high")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Right ramp exit")

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Left jet")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Upper jet")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("Lower jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Left sling")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Right sling")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Kickback")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Upper left lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Upper right lane")

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

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

static INPUT_PORTS_START( tfs )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("INP11")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("INP12")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("INP15")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("INP16")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("INP17")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("INP18")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("INP26")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("INP27")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("INP31")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("INP32")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("INP33")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("INP34")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("INP35")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("INP36")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("INP37")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("INP38")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("INP41")
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("INP52")
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("INP63")
	PORT_BIT(0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("INP71")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("INP72")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("INP73")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("INP74")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("INP75")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("INP76")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("INP77")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP78")

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP81")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP82")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP83")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP84")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP85")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP86")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP87")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP88")

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
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

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

void wpc_s_state::wpc_s(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, XTAL(8'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &wpc_s_state::wpc_s_map);
	m_maincpu->set_periodic_int(FUNC(wpc_s_state::irq0_line_assert), attotime::from_hz(XTAL(8'000'000)/8192.0));

	TIMER(config, "zero_crossing").configure_periodic(FUNC(wpc_s_state::zc_timer), attotime::from_hz(120)); // Mains power zero crossing

	WPC_SHIFT(config, "shift");
	WPC_PIC(config, m_pic);
	WPC_LAMP(config, m_lamp);
	WPC_OUT(config, m_out, 5);
	WPC_DMD(config, "dmd").scanline_callback().set(FUNC(wpc_s_state::scanline_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SPEAKER(config, "mono").front_center();

	DCS_AUDIO_8K(config, m_dcs);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->add_route(0, "mono", 1.0);
}

void wpc_s_state::corv(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("536 123456 12345 123");
	m_lamp->set_names(lamps_corv);
	m_out->set_names(outputs_corv);
}

void wpc_s_state::dh(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("530 123456 12345 123");
	m_lamp->set_names(lamps_dh);
	m_out->set_names(outputs_dh);
}

void wpc_s_state::i500(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("526 123456 12345 123");
	m_lamp->set_names(lamps_i500);
	m_out->set_names(outputs_i500);
}

void wpc_s_state::jb(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("551 123456 12345 123");
	m_lamp->set_names(lamps_jb);
	m_out->set_names(outputs_jb);
}

void wpc_s_state::jm(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("542 123456 12345 123");
	m_lamp->set_names(lamps_jm);
	m_out->set_names(outputs_jm);
}

void wpc_s_state::nf(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("525 123456 12345 123");
	m_lamp->set_names(lamps_nf);
	m_out->set_names(outputs_nf);
}

void wpc_s_state::rs(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("524 123456 12345 123");
	m_lamp->set_names(lamps_rs);
	m_out->set_names(outputs_rs);
}

void wpc_s_state::fs(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("529 123456 12345 123");
	//m_lamp->set_names(lamps_fs);
	//m_out->set_names(outputs_fs);
}

void wpc_s_state::ts(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("532 123456 12345 123");
	//m_lamp->set_names(lamps_ts);
	//m_out->set_names(outputs_ts);
}

void wpc_s_state::tom(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("539 123456 12345 124");
	//m_lamp->set_names(lamps_tom);
	//m_out->set_names(outputs_tom);
}

void wpc_s_state::tom14(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("124 123456 12345 123");
	//m_lamp->set_names(lamps_tom);
	//m_out->set_names(outputs_tom);
}

void wpc_s_state::wd(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("544 123456 12345 123");
	//m_lamp->set_names(lamps_wd);
	//m_out->set_names(outputs_wd);
}

void wpc_s_state::wcs(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("531 123456 12345 123");
	//m_lamp->set_names(lamps_wcs);
	//m_out->set_names(outputs_wcs);
}

void wpc_s_state::tfs(machine_config &config)
{
	wpc_s(config);

	m_pic->set_serial("648 123456 12345 123");
	//m_lamp->set_names(nullptr);
	//m_out->set_names(nullptr);
}

/*-----------------
/  Corvette #50036
/------------------*/
ROM_START( corv_21 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "corv_2_1.rom", 0x00000, 0x80000, CRC(4fe64c6d) SHA1(f68bca3c216b7b99575fce44bd257325dbcc4f47) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "corvsnd2", 0x000000, 0x080000, CRC(630d20a3) SHA1(c7b6cbc7f23c1f9c149a3ef32e84ca8797ff8026) )
	ROM_LOAD16_BYTE( "corvsnd3", 0x200000, 0x080000, CRC(6ace0353) SHA1(dec5b6f129ee6b7c0d03c1677d6b71672dd25a5a) )
	ROM_LOAD16_BYTE( "corvsnd4", 0x400000, 0x080000, CRC(87807278) SHA1(ba01b44c0ad6d10163a8aed2211539d541e69449) )
	ROM_LOAD16_BYTE( "corvsnd5", 0x600000, 0x080000, CRC(35f82c21) SHA1(ee14489e5629e9cd5622a56849fab65b94ff9b59) )
	ROM_LOAD16_BYTE( "corvsnd6", 0x800000, 0x080000, CRC(61e56d90) SHA1(41388523fca4839132d3f7e117bdac9ea9f4020c) )
	ROM_LOAD16_BYTE( "corvsnd7", 0xa00000, 0x080000, CRC(1417b547) SHA1(851acf77159a1ef99fc2934353eb887065568004) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "536_corvette_u22.u22", 0x0000, 0x2000, CRC(b73979eb) SHA1(9033fe771ec4cb4fd514806cdc59b64ae1fff1ac) )
ROM_END

ROM_START( corv_lx1 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6-lx1.rom", 0x00000, 0x80000, CRC(0e762e27) SHA1(830d9ccb00a7884e2c6d3bdf7aedac6f58af2397) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "su2-sl1.rom", 0x000000, 0x080000, CRC(141d280e) SHA1(ab1e8e38b9fa0e693837c93616f0821e25b31588) )
	ROM_LOAD16_BYTE( "corvsnd3",    0x200000, 0x080000, CRC(6ace0353) SHA1(dec5b6f129ee6b7c0d03c1677d6b71672dd25a5a) )
	ROM_LOAD16_BYTE( "corvsnd4",    0x400000, 0x080000, CRC(87807278) SHA1(ba01b44c0ad6d10163a8aed2211539d541e69449) )
	ROM_LOAD16_BYTE( "corvsnd5",    0x600000, 0x080000, CRC(35f82c21) SHA1(ee14489e5629e9cd5622a56849fab65b94ff9b59) )
	ROM_LOAD16_BYTE( "corvsnd6",    0x800000, 0x080000, CRC(61e56d90) SHA1(41388523fca4839132d3f7e117bdac9ea9f4020c) )
	ROM_LOAD16_BYTE( "corvsnd7",    0xa00000, 0x080000, CRC(1417b547) SHA1(851acf77159a1ef99fc2934353eb887065568004) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "536_corvette_u22.u22", 0x0000, 0x2000, CRC(b73979eb) SHA1(9033fe771ec4cb4fd514806cdc59b64ae1fff1ac) )
ROM_END

ROM_START( corv_lx2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6-lx2.rom", 0x00000, 0x80000, CRC(204c1e4a) SHA1(b6b1ada4ac57a0bf1c3322105936c91d5704ef18) )

	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "su2-sl1.rom", 0x000000, 0x080000, CRC(141d280e) SHA1(ab1e8e38b9fa0e693837c93616f0821e25b31588) )
	ROM_LOAD16_BYTE( "corvsnd3",    0x100000, 0x080000, CRC(6ace0353) SHA1(dec5b6f129ee6b7c0d03c1677d6b71672dd25a5a) )
	ROM_LOAD16_BYTE( "corvsnd4",    0x200000, 0x080000, CRC(87807278) SHA1(ba01b44c0ad6d10163a8aed2211539d541e69449) )
	ROM_LOAD16_BYTE( "corvsnd5",    0x300000, 0x080000, CRC(35f82c21) SHA1(ee14489e5629e9cd5622a56849fab65b94ff9b59) )
	ROM_LOAD16_BYTE( "corvsnd6",    0x400000, 0x080000, CRC(61e56d90) SHA1(41388523fca4839132d3f7e117bdac9ea9f4020c) )
	ROM_LOAD16_BYTE( "corvsnd7",    0x500000, 0x080000, CRC(1417b547) SHA1(851acf77159a1ef99fc2934353eb887065568004) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "536_corvette_u22.u22", 0x0000, 0x2000, CRC(b73979eb) SHA1(9033fe771ec4cb4fd514806cdc59b64ae1fff1ac) )
ROM_END

ROM_START( corv_px4 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6-px4.rom", 0x00000, 0x80000, CRC(a5f22149) SHA1(e0b0bce31b1e66e6b74930c3184f87ebec400f80) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "su2-sl1.rom", 0x000000, 0x080000, CRC(141d280e) SHA1(ab1e8e38b9fa0e693837c93616f0821e25b31588) )
	ROM_LOAD16_BYTE( "corvsnd3",    0x200000, 0x080000, CRC(6ace0353) SHA1(dec5b6f129ee6b7c0d03c1677d6b71672dd25a5a) )
	ROM_LOAD16_BYTE( "corvsnd4",    0x400000, 0x080000, CRC(87807278) SHA1(ba01b44c0ad6d10163a8aed2211539d541e69449) )
	ROM_LOAD16_BYTE( "corvsnd5",    0x600000, 0x080000, CRC(35f82c21) SHA1(ee14489e5629e9cd5622a56849fab65b94ff9b59) )
	ROM_LOAD16_BYTE( "corvsnd6",    0x800000, 0x080000, CRC(61e56d90) SHA1(41388523fca4839132d3f7e117bdac9ea9f4020c) )
	ROM_LOAD16_BYTE( "corvsnd7",    0xa00000, 0x080000, CRC(1417b547) SHA1(851acf77159a1ef99fc2934353eb887065568004) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "536_corvette_u22.u22", 0x0000, 0x2000, CRC(b73979eb) SHA1(9033fe771ec4cb4fd514806cdc59b64ae1fff1ac) )
ROM_END

ROM_START( corv_px3 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6-px3.rom", 0x00000, 0x80000, CRC(5a363db8) SHA1(19eea89bf1ab3cc84c5b67eac00d8be4e65249f6) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "su2-sp2.rom", 0x000000, 0x080000, CRC(9ae4bf31) SHA1(a33978d9ac8ffdc7b8719c803edd3e804f0dd886) )
	ROM_LOAD16_BYTE( "corvsnd3",    0x200000, 0x080000, CRC(6ace0353) SHA1(dec5b6f129ee6b7c0d03c1677d6b71672dd25a5a) )
	ROM_LOAD16_BYTE( "corvsnd4",    0x400000, 0x080000, CRC(87807278) SHA1(ba01b44c0ad6d10163a8aed2211539d541e69449) )
	ROM_LOAD16_BYTE( "corvsnd5",    0x600000, 0x080000, CRC(35f82c21) SHA1(ee14489e5629e9cd5622a56849fab65b94ff9b59) )
	ROM_LOAD16_BYTE( "corvsnd6",    0x800000, 0x080000, CRC(61e56d90) SHA1(41388523fca4839132d3f7e117bdac9ea9f4020c) )
	ROM_LOAD16_BYTE( "corvsnd7",    0xa00000, 0x080000, CRC(1417b547) SHA1(851acf77159a1ef99fc2934353eb887065568004) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "536_corvette_u22.u22", 0x0000, 0x2000, CRC(b73979eb) SHA1(9033fe771ec4cb4fd514806cdc59b64ae1fff1ac) )
ROM_END

ROM_START( corv_la1 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6-la1.rom", 0x00000, 0x80000, CRC(2e205fc6) SHA1(dbe7448f5a7eefc58b237202526f94f298a8b79d) )

	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "su2-sl1.rom", 0x000000, 0x080000, CRC(141d280e) SHA1(ab1e8e38b9fa0e693837c93616f0821e25b31588) )
	ROM_LOAD16_BYTE( "corvsnd3",    0x100000, 0x080000, CRC(6ace0353) SHA1(dec5b6f129ee6b7c0d03c1677d6b71672dd25a5a) )
	ROM_LOAD16_BYTE( "corvsnd4",    0x200000, 0x080000, CRC(87807278) SHA1(ba01b44c0ad6d10163a8aed2211539d541e69449) )
	ROM_LOAD16_BYTE( "corvsnd5",    0x300000, 0x080000, CRC(35f82c21) SHA1(ee14489e5629e9cd5622a56849fab65b94ff9b59) )
	ROM_LOAD16_BYTE( "corvsnd6",    0x400000, 0x080000, CRC(61e56d90) SHA1(41388523fca4839132d3f7e117bdac9ea9f4020c) )
	ROM_LOAD16_BYTE( "corvsnd7",    0x500000, 0x080000, CRC(1417b547) SHA1(851acf77159a1ef99fc2934353eb887065568004) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "536_corvette_u22.u22", 0x0000, 0x2000, CRC(b73979eb) SHA1(9033fe771ec4cb4fd514806cdc59b64ae1fff1ac) )
ROM_END

/*--------------------
/  Dirty Harry #50030
/--------------------*/
ROM_START( dh_lx2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "harr_lx2.rom", 0x00000, 0x80000, CRC(d92c2d35) SHA1(68f08120fbc510db46b1fd0e68ec07fe536f77ca) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "dh_snd.u2", 0x000000, 0x080000, CRC(dce5339a) SHA1(c89ec1c2f4f5201cbc40c7038cd1219b200066c7) )
	ROM_LOAD16_BYTE( "dh_snd.u3", 0x200000, 0x080000, CRC(27c30ada) SHA1(388c0e533d1d5c88ae020ef8d8b98db4c603c157) )
	ROM_LOAD16_BYTE( "dh_snd.u4", 0x400000, 0x080000, CRC(8bde0089) SHA1(8efdcc60daef06c65acf5cb805790d2b82d3c091) )
	ROM_LOAD16_BYTE( "dh_snd.u5", 0x600000, 0x080000, CRC(bfacfbdb) SHA1(aa443906a0945586ba5d2910972b333b5d316894) )
	ROM_LOAD16_BYTE( "dh_snd.u6", 0x800000, 0x080000, CRC(793dcfb8) SHA1(c9b35e0511962f9fc372f98e937ee5989109056d) )
ROM_END

ROM_START( dh_lf2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "harr_lf2.rom", 0x00000, 0x80000, CRC(c4931917) SHA1(f7a366fade194ad7b3671acf55d894e3c31992d0) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "dh_snd.u2", 0x000000, 0x080000, CRC(dce5339a) SHA1(c89ec1c2f4f5201cbc40c7038cd1219b200066c7) )
	ROM_LOAD16_BYTE( "dh_snd.u3", 0x200000, 0x080000, CRC(27c30ada) SHA1(388c0e533d1d5c88ae020ef8d8b98db4c603c157) )
	ROM_LOAD16_BYTE( "dh_snd.u4", 0x400000, 0x080000, CRC(8bde0089) SHA1(8efdcc60daef06c65acf5cb805790d2b82d3c091) )
	ROM_LOAD16_BYTE( "dh_snd.u5", 0x600000, 0x080000, CRC(bfacfbdb) SHA1(aa443906a0945586ba5d2910972b333b5d316894) )
	ROM_LOAD16_BYTE( "dh_snd.u6", 0x800000, 0x080000, CRC(793dcfb8) SHA1(c9b35e0511962f9fc372f98e937ee5989109056d) )
ROM_END

/*--------------------------
/  Indianapolis 500 #50026
/--------------------------*/
ROM_START( i500_11r )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "indy1_1r.rom", 0x00000, 0x80000, CRC(ec385bf5) SHA1(50d8f3e682e3a59f3df35f099e97858b2fd211ff) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "su2", 0x000000, 0x080000, CRC(d2f9ab24) SHA1(ac96fdce6545d1017fa3ed7567061ae2e0653750) )
	ROM_LOAD16_BYTE( "su3", 0x200000, 0x080000, CRC(067f4df6) SHA1(0adc116bfebefb17f27718bdd2401c336b07078f) )
	ROM_LOAD16_BYTE( "su4", 0x400000, 0x080000, CRC(229b96c2) SHA1(77eda81fd011fc818c3fde5e1094cfb3f12372c6) )
	ROM_LOAD16_BYTE( "su5", 0x600000, 0x080000, CRC(f0c006a5) SHA1(ead07bb131bd581c41ab0833f6269de7e574017c) )
	ROM_LOAD16_BYTE( "su6", 0x800000, 0x080000, CRC(a2b60d31) SHA1(0e0ddb310ec78e0963794994edd0c6bbc4863f4f) )
	ROM_LOAD16_BYTE( "su7", 0xa00000, 0x080000, CRC(94eea5a4) SHA1(afb00e799dbc01c67ed2c4aa399e8a7365ca3dd3) )
ROM_END

ROM_START( i500_11b )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "indy1_1b.rom", 0x00000, 0x80000, CRC(76a5de55) SHA1(858d9817b534fed470919fa5957709dd1e4216d8) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "su2", 0x000000, 0x080000, CRC(d2f9ab24) SHA1(ac96fdce6545d1017fa3ed7567061ae2e0653750) )
	ROM_LOAD16_BYTE( "su3", 0x200000, 0x080000, CRC(067f4df6) SHA1(0adc116bfebefb17f27718bdd2401c336b07078f) )
	ROM_LOAD16_BYTE( "su4", 0x400000, 0x080000, CRC(229b96c2) SHA1(77eda81fd011fc818c3fde5e1094cfb3f12372c6) )
	ROM_LOAD16_BYTE( "su5", 0x600000, 0x080000, CRC(f0c006a5) SHA1(ead07bb131bd581c41ab0833f6269de7e574017c) )
	ROM_LOAD16_BYTE( "su6", 0x800000, 0x080000, CRC(a2b60d31) SHA1(0e0ddb310ec78e0963794994edd0c6bbc4863f4f) )
	ROM_LOAD16_BYTE( "su7", 0xa00000, 0x080000, CRC(94eea5a4) SHA1(afb00e799dbc01c67ed2c4aa399e8a7365ca3dd3) )
ROM_END

ROM_START( i500_10r )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "indy1_0r.rom", 0x00000, 0x80000, CRC(df52d6a7) SHA1(8176028fa2f93f6b5878816e8b7124a8ec1ba765) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "su2", 0x000000, 0x080000, CRC(d2f9ab24) SHA1(ac96fdce6545d1017fa3ed7567061ae2e0653750) )
	ROM_LOAD16_BYTE( "su3", 0x200000, 0x080000, CRC(067f4df6) SHA1(0adc116bfebefb17f27718bdd2401c336b07078f) )
	ROM_LOAD16_BYTE( "su4", 0x400000, 0x080000, CRC(229b96c2) SHA1(77eda81fd011fc818c3fde5e1094cfb3f12372c6) )
	ROM_LOAD16_BYTE( "su5", 0x600000, 0x080000, CRC(f0c006a5) SHA1(ead07bb131bd581c41ab0833f6269de7e574017c) )
	ROM_LOAD16_BYTE( "su6", 0x800000, 0x080000, CRC(a2b60d31) SHA1(0e0ddb310ec78e0963794994edd0c6bbc4863f4f) )
	ROM_LOAD16_BYTE( "su7", 0xa00000, 0x080000, CRC(94eea5a4) SHA1(afb00e799dbc01c67ed2c4aa399e8a7365ca3dd3) )
ROM_END

/*-----------------
/  Jack*Bot #50051
/------------------*/
ROM_START( jb_10r )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jack1_0r.rom", 0x00000, 0x80000, CRC(0e1a900a) SHA1(894f642611d29ce11e13ef9dd68dba7dfc602a3a) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jbsnd_u2.rom", 0x000000, 0x080000, CRC(b116d59f) SHA1(c87c82d7fa4ca40d337fb59476b0b1c90dff86f0) )
	ROM_LOAD16_BYTE( "jbsnd_u3.rom", 0x200000, 0x080000, CRC(76ad3aad) SHA1(012b44c48d1cbb282eb763e40db40b141397f426) )
	ROM_LOAD16_BYTE( "jbsnd_u4.rom", 0x400000, 0x080000, CRC(038b1309) SHA1(a6e337476902ed9ec5123fe4e088a0608c0d5f48) )
	ROM_LOAD16_BYTE( "jbsnd_u5.rom", 0x600000, 0x080000, CRC(0957e2ad) SHA1(0fb4e3fdb949b0979721064162a41cfba84d0013) )
	ROM_LOAD16_BYTE( "jbsnd_u6.rom", 0x800000, 0x080000, CRC(7a1e2c3d) SHA1(0c6ccb937328509cb0a87e4c557a64c13bbed2db) )
ROM_END

ROM_START( jb_10b )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jack1_0b.rom", 0x00000, 0x80000, CRC(da3b2735) SHA1(f895b1548107052f469d8e3fa205bce6113962d9) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jbsnd_u2.rom", 0x000000, 0x080000, CRC(b116d59f) SHA1(c87c82d7fa4ca40d337fb59476b0b1c90dff86f0) )
	ROM_LOAD16_BYTE( "jbsnd_u3.rom", 0x200000, 0x080000, CRC(76ad3aad) SHA1(012b44c48d1cbb282eb763e40db40b141397f426) )
	ROM_LOAD16_BYTE( "jbsnd_u4.rom", 0x400000, 0x080000, CRC(038b1309) SHA1(a6e337476902ed9ec5123fe4e088a0608c0d5f48) )
	ROM_LOAD16_BYTE( "jbsnd_u5.rom", 0x600000, 0x080000, CRC(0957e2ad) SHA1(0fb4e3fdb949b0979721064162a41cfba84d0013) )
	ROM_LOAD16_BYTE( "jbsnd_u6.rom", 0x800000, 0x080000, CRC(7a1e2c3d) SHA1(0c6ccb937328509cb0a87e4c557a64c13bbed2db) )
ROM_END

ROM_START( jb_04a) // WPC-S 5-Board
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "jack0_4a.rom", 0x00000, 0x80000, CRC(cc02e024) SHA1(5de886e4631c3c99b83f6692482259e2a6bf0bca) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jbsnd_04a.u2", 0x000000, 0x080000, CRC(f90ebf0f) SHA1(d18362c1946504dd456f6edc28bb5812d584874a) )
	ROM_LOAD16_BYTE( "jbsnd_u3.rom", 0x200000, 0x080000, CRC(76ad3aad) SHA1(012b44c48d1cbb282eb763e40db40b141397f426) )
	ROM_LOAD16_BYTE( "jbsnd_u4.rom", 0x400000, 0x080000, CRC(038b1309) SHA1(a6e337476902ed9ec5123fe4e088a0608c0d5f48) )
	ROM_LOAD16_BYTE( "jbsnd_u5.rom", 0x600000, 0x080000, CRC(0957e2ad) SHA1(0fb4e3fdb949b0979721064162a41cfba84d0013) )
	ROM_LOAD16_BYTE( "jbsnd_u6.rom", 0x800000, 0x080000, CRC(7a1e2c3d) SHA1(0c6ccb937328509cb0a87e4c557a64c13bbed2db) )
ROM_END

/*------------------------
/  Johnny Mnemonic #50042
/------------------------*/
ROM_START( jm_12r )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "john1_2r.rom", 0x00000, 0x80000, CRC(fff07398) SHA1(3b9a51414498ef4c4a9d59ebd35348bca1cc7dfb) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jm_u2_s.1_0", 0x000000, 0x080000, CRC(4aeeff3d) SHA1(861b65b97715182385e2fe076af1fb2eb2ccc298) )
	ROM_LOAD16_BYTE( "jm_u3_s.1_0", 0x200000, 0x080000, CRC(9bf7bc43) SHA1(94fa83be84940a4db0143acc330aacded1d0d9ca) )
	ROM_LOAD16_BYTE( "jm_u4_s.1_0", 0x400000, 0x080000, CRC(2e044582) SHA1(0de30f6c223338a67f9332de038baf1398d9043e) )
	ROM_LOAD16_BYTE( "jm_u5_s.1_0", 0x600000, 0x080000, CRC(50cc06a7) SHA1(fa3072a8bc9be72fe974413094f0944d98cf3857) )
	ROM_LOAD16_BYTE( "jm_u6_s.1_0", 0x800000, 0x080000, CRC(bfc94707) SHA1(a1f4d35a4b1d80c8160e937458a8e5181f295f28) )
	ROM_LOAD16_BYTE( "jm_u7_s.1_0", 0xa00000, 0x080000, CRC(9d4d9e9d) SHA1(d6e074806eed6fedc169c4849a9dd9ac2beed07e) )
	ROM_LOAD16_BYTE( "jm_u8_s.1_0", 0xc00000, 0x080000, CRC(fc7af6c0) SHA1(a70dadf86d1af2122b58fdd85e938d50d113305f) )
ROM_END

ROM_START( jm_12b )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "john1_2b.rom", 0x00000, 0x80000, CRC(b039c37e) SHA1(b193a2e08eb47b32cb697bda2d8766fa6b702a8b) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jm_u2_s.1_0", 0x000000, 0x080000, CRC(4aeeff3d) SHA1(861b65b97715182385e2fe076af1fb2eb2ccc298) )
	ROM_LOAD16_BYTE( "jm_u3_s.1_0", 0x200000, 0x080000, CRC(9bf7bc43) SHA1(94fa83be84940a4db0143acc330aacded1d0d9ca) )
	ROM_LOAD16_BYTE( "jm_u4_s.1_0", 0x400000, 0x080000, CRC(2e044582) SHA1(0de30f6c223338a67f9332de038baf1398d9043e) )
	ROM_LOAD16_BYTE( "jm_u5_s.1_0", 0x600000, 0x080000, CRC(50cc06a7) SHA1(fa3072a8bc9be72fe974413094f0944d98cf3857) )
	ROM_LOAD16_BYTE( "jm_u6_s.1_0", 0x800000, 0x080000, CRC(bfc94707) SHA1(a1f4d35a4b1d80c8160e937458a8e5181f295f28) )
	ROM_LOAD16_BYTE( "jm_u7_s.1_0", 0xa00000, 0x080000, CRC(9d4d9e9d) SHA1(d6e074806eed6fedc169c4849a9dd9ac2beed07e) )
	ROM_LOAD16_BYTE( "jm_u8_s.1_0", 0xc00000, 0x080000, CRC(fc7af6c0) SHA1(a70dadf86d1af2122b58fdd85e938d50d113305f) )
ROM_END

ROM_START( jm_05r )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "john0_5r.rom", 0x00000, 0x80000, CRC(57df5654) SHA1(b27c66dac592dd9db84ee86836216581b4cde3b1) )

	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jm_u2_s.038", 0x000000, 0x080000, CRC(3b291732) SHA1(d0ce122b2a8c1ce04fe4ad1bef633514bb0c8f9f) )
	ROM_LOAD16_BYTE( "jm_u3_s.038", 0x100000, 0x080000, CRC(653c2fc5) SHA1(8d462314394f8babed89f7cf70ce2df534cae13e) )
	ROM_LOAD16_BYTE( "jm_u4_s.038", 0x200000, 0x080000, CRC(d0cfd604) SHA1(3844667135645653f1766d12d40752ddac2ec830) )
	ROM_LOAD16_BYTE( "jm_u5_s.038", 0x300000, 0x080000, CRC(58d5276d) SHA1(8b74b9fc87c6df015f32201499e25c0135a65568) )
	ROM_LOAD16_BYTE( "jm_u6_s.038", 0x400000, 0x080000, CRC(534ef536) SHA1(9706ca9f422f10f85e81cea965c31c96c662bc34) )
	ROM_LOAD16_BYTE( "jm_u7_s.038", 0x500000, 0x080000, CRC(ce07e128) SHA1(b65e2bd1263f597320b53300868343ce9b6bd395) )
	ROM_LOAD16_BYTE( "jm_u8_s.038", 0x600000, 0x080000, CRC(f463d70a) SHA1(7fd701dcdd8672d7e4d45f400d507cc0f4db2578) )
ROM_END

/*----------------------------------
/  No Fear: Dangerous Sports #50025
/----------------------------------*/
ROM_START( nf_23x )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "nofe2_3x.rom", 0x00000, 0x80000, CRC(d853650b) SHA1(06d58f86c68ccdc242d6b96a22c6226758dc3e44) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nfu2s", 0x000000, 0x080000, CRC(136caeb9) SHA1(61a56b29a7655e8aab4987d300173e1acf27c77c) )
	ROM_LOAD16_BYTE( "nfu3s", 0x200000, 0x080000, CRC(983e5578) SHA1(374b1397abbdde5fd9257fd45fd8613c94fbd02d) )
	ROM_LOAD16_BYTE( "nfu4s", 0x400000, 0x080000, CRC(9469cd40) SHA1(8a1dd1088f24018f48b114c0b27f0331263d4eea) )
	ROM_LOAD16_BYTE( "nfu5s", 0x600000, 0x080000, CRC(e14d4315) SHA1(63d5ae800cc8a750ea2e3a87c646ab175b60abc7) )
	ROM_LOAD16_BYTE( "nfu6s", 0x800000, 0x080000, CRC(40a58903) SHA1(78f7e99f39efc83f3cf17801a30e6dc6e4864125) )
	ROM_LOAD16_BYTE( "nfu7s", 0xa00000, 0x080000, CRC(61002bdd) SHA1(e623399ff95f59a4ab7efdd7c69b1a1370479398) )
ROM_END

ROM_START( nf_23 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "nofe2_3.rom", 0x00000, 0x80000, CRC(47746cbc) SHA1(61606da50894c2f01c64dd5c3aef72cb17a0bc31) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nfu2s", 0x000000, 0x080000, CRC(136caeb9) SHA1(61a56b29a7655e8aab4987d300173e1acf27c77c) )
	ROM_LOAD16_BYTE( "nfu3s", 0x200000, 0x080000, CRC(983e5578) SHA1(374b1397abbdde5fd9257fd45fd8613c94fbd02d) )
	ROM_LOAD16_BYTE( "nfu4s", 0x400000, 0x080000, CRC(9469cd40) SHA1(8a1dd1088f24018f48b114c0b27f0331263d4eea) )
	ROM_LOAD16_BYTE( "nfu5s", 0x600000, 0x080000, CRC(e14d4315) SHA1(63d5ae800cc8a750ea2e3a87c646ab175b60abc7) )
	ROM_LOAD16_BYTE( "nfu6s", 0x800000, 0x080000, CRC(40a58903) SHA1(78f7e99f39efc83f3cf17801a30e6dc6e4864125) )
	ROM_LOAD16_BYTE( "nfu7s", 0xa00000, 0x080000, CRC(61002bdd) SHA1(e623399ff95f59a4ab7efdd7c69b1a1370479398) )
ROM_END

ROM_START( nf_23f )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "nofe2_3f.rom", 0x00000, 0x80000, CRC(996e5e75) SHA1(ca3d2c13388e674b26cebf7f9ae65c8a722b68b1) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nfu2s", 0x000000, 0x080000, CRC(136caeb9) SHA1(61a56b29a7655e8aab4987d300173e1acf27c77c) )
	ROM_LOAD16_BYTE( "nfu3s", 0x200000, 0x080000, CRC(983e5578) SHA1(374b1397abbdde5fd9257fd45fd8613c94fbd02d) )
	ROM_LOAD16_BYTE( "nfu4s", 0x400000, 0x080000, CRC(9469cd40) SHA1(8a1dd1088f24018f48b114c0b27f0331263d4eea) )
	ROM_LOAD16_BYTE( "nfu5s", 0x600000, 0x080000, CRC(e14d4315) SHA1(63d5ae800cc8a750ea2e3a87c646ab175b60abc7) )
	ROM_LOAD16_BYTE( "nfu6s", 0x800000, 0x080000, CRC(40a58903) SHA1(78f7e99f39efc83f3cf17801a30e6dc6e4864125) )
	ROM_LOAD16_BYTE( "nfu7s", 0xa00000, 0x080000, CRC(61002bdd) SHA1(e623399ff95f59a4ab7efdd7c69b1a1370479398) )
ROM_END

ROM_START( nf_22 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "nofe2_2a.rom", 0x00000, 0x80000, CRC(8694b32e) SHA1(fbdc45910ef5e34c90557491831854de3b4889a8) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nfu2s", 0x000000, 0x080000, CRC(136caeb9) SHA1(61a56b29a7655e8aab4987d300173e1acf27c77c) )
	ROM_LOAD16_BYTE( "nfu3s", 0x200000, 0x080000, CRC(983e5578) SHA1(374b1397abbdde5fd9257fd45fd8613c94fbd02d) )
	ROM_LOAD16_BYTE( "nfu4s", 0x400000, 0x080000, CRC(9469cd40) SHA1(8a1dd1088f24018f48b114c0b27f0331263d4eea) )
	ROM_LOAD16_BYTE( "nfu5s", 0x600000, 0x080000, CRC(e14d4315) SHA1(63d5ae800cc8a750ea2e3a87c646ab175b60abc7) )
	ROM_LOAD16_BYTE( "nfu6s", 0x800000, 0x080000, CRC(40a58903) SHA1(78f7e99f39efc83f3cf17801a30e6dc6e4864125) )
	ROM_LOAD16_BYTE( "nfu7s", 0xa00000, 0x080000, CRC(61002bdd) SHA1(e623399ff95f59a4ab7efdd7c69b1a1370479398) )
ROM_END

ROM_START( nf_20 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "nofe2_0a.rom", 0x00000, 0x80000, CRC(dceee809) SHA1(44aaeeb268d67eb48087bd8958f864d4c4ee5138) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nfu2s", 0x000000, 0x080000, CRC(136caeb9) SHA1(61a56b29a7655e8aab4987d300173e1acf27c77c) )
	ROM_LOAD16_BYTE( "nfu3s", 0x200000, 0x080000, CRC(983e5578) SHA1(374b1397abbdde5fd9257fd45fd8613c94fbd02d) )
	ROM_LOAD16_BYTE( "nfu4s", 0x400000, 0x080000, CRC(9469cd40) SHA1(8a1dd1088f24018f48b114c0b27f0331263d4eea) )
	ROM_LOAD16_BYTE( "nfu5s", 0x600000, 0x080000, CRC(e14d4315) SHA1(63d5ae800cc8a750ea2e3a87c646ab175b60abc7) )
	ROM_LOAD16_BYTE( "nfu6s", 0x800000, 0x080000, CRC(40a58903) SHA1(78f7e99f39efc83f3cf17801a30e6dc6e4864125) )
	ROM_LOAD16_BYTE( "nfu7s", 0xa00000, 0x080000, CRC(61002bdd) SHA1(e623399ff95f59a4ab7efdd7c69b1a1370479398) )
ROM_END

ROM_START( nf_10f )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "nofe1_0.rom", 0x00000, 0x80000, CRC(f8f6521c) SHA1(5c26f4878f257b157c2a1c46995ec8100fa20723) )

	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "snd-u2.sl1", 0x000000, 0x080000, CRC(84f48e27) SHA1(cdf0ff55c1493ea5ac7cef618c985f41442c6f60) )
	ROM_LOAD16_BYTE( "nfu3s",      0x100000, 0x080000, CRC(983e5578) SHA1(374b1397abbdde5fd9257fd45fd8613c94fbd02d) )
	ROM_LOAD16_BYTE( "nfu4s",      0x200000, 0x080000, CRC(9469cd40) SHA1(8a1dd1088f24018f48b114c0b27f0331263d4eea) )
	ROM_LOAD16_BYTE( "nfu5s",      0x300000, 0x080000, CRC(e14d4315) SHA1(63d5ae800cc8a750ea2e3a87c646ab175b60abc7) )
	ROM_LOAD16_BYTE( "nfu6s",      0x400000, 0x080000, CRC(40a58903) SHA1(78f7e99f39efc83f3cf17801a30e6dc6e4864125) )
	ROM_LOAD16_BYTE( "nfu7s",      0x500000, 0x080000, CRC(61002bdd) SHA1(e623399ff95f59a4ab7efdd7c69b1a1370479398) )
ROM_END

ROM_START( nf_08x )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "nofe0_8x.rom", 0x00000, 0x80000, CRC(64871e6a) SHA1(0e116104b06446b0d435f715c33535080cdd2378) )

	ROM_REGION16_LE( 0x800000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "snd-u2.sl1", 0x000000, 0x080000, CRC(84f48e27) SHA1(cdf0ff55c1493ea5ac7cef618c985f41442c6f60) )
	ROM_LOAD16_BYTE( "nfu3s",      0x100000, 0x080000, CRC(983e5578) SHA1(374b1397abbdde5fd9257fd45fd8613c94fbd02d) )
	ROM_LOAD16_BYTE( "nfu4s",      0x200000, 0x080000, CRC(9469cd40) SHA1(8a1dd1088f24018f48b114c0b27f0331263d4eea) )
	ROM_LOAD16_BYTE( "nfu5s",      0x300000, 0x080000, CRC(e14d4315) SHA1(63d5ae800cc8a750ea2e3a87c646ab175b60abc7) )
	ROM_LOAD16_BYTE( "nfu6s",      0x400000, 0x080000, CRC(40a58903) SHA1(78f7e99f39efc83f3cf17801a30e6dc6e4864125) )
	ROM_LOAD16_BYTE( "nfu7s",      0x500000, 0x080000, CRC(61002bdd) SHA1(e623399ff95f59a4ab7efdd7c69b1a1370479398) )
ROM_END

/*--------------------------------
/  Red and Ted's Road Show #50024
/--------------------------------*/
ROM_START( rs_l6 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "rshw_l6.rom", 0x00000, 0x80000, CRC(3986d402) SHA1(1a67e5bafb7a6aa1d42b2e631e2294a3c1403038) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rs_u2_s.l1", 0x000000, 0x080000, CRC(5a2db20c) SHA1(34ce236cc874b820db2d2268cc77815ed7ca061b) )
	ROM_LOAD16_BYTE( "rs_u3_s.l1", 0x200000, 0x080000, CRC(719be036) SHA1(fa975a6a93fcaefddcbd1c0b97c49bd9f9608ad4) )
	ROM_LOAD16_BYTE( "rs_u4_s.l1", 0x400000, 0x080000, CRC(d452d007) SHA1(b850bc8e17d8940f93c1e7b6a0ab786b092694b3) )
	ROM_LOAD16_BYTE( "rs_u5_s.l1", 0x600000, 0x080000, CRC(1faa04c9) SHA1(817bbd7fc0781d84af6c40cb477adf83cef07ab2) )
	ROM_LOAD16_BYTE( "rs_u6_s.l1", 0x800000, 0x080000, CRC(eee00add) SHA1(96d664ca73ac896e918d7011c1cda3e55e3731b7) )
	ROM_LOAD16_BYTE( "rs_u7_s.l1", 0xa00000, 0x080000, CRC(3a222a54) SHA1(2a788e4ac573bf1d128e5bef9357e62c805014b9) )
	ROM_LOAD16_BYTE( "rs_u8_s.l1", 0xc00000, 0x080000, CRC(c70f2210) SHA1(9be9f271d81d15a4eb123f1377b0c077eef97774) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "524_road_show.u22", 0x0000, 0x2000, CRC(e78ea6a0) SHA1(bf0200177c9d4764b3164cbee37e9a5a35ab996f) )
ROM_END

ROM_START( rs_la5 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6_la5.rom", 0x00000, 0x80000, CRC(61e63268) SHA1(79e32f489c51e7e79e892d36f586af14ab9aa2a5) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rs_u2_s.l1", 0x000000, 0x080000, CRC(5a2db20c) SHA1(34ce236cc874b820db2d2268cc77815ed7ca061b) )
	ROM_LOAD16_BYTE( "rs_u3_s.l1", 0x200000, 0x080000, CRC(719be036) SHA1(fa975a6a93fcaefddcbd1c0b97c49bd9f9608ad4) )
	ROM_LOAD16_BYTE( "rs_u4_s.l1", 0x400000, 0x080000, CRC(d452d007) SHA1(b850bc8e17d8940f93c1e7b6a0ab786b092694b3) )
	ROM_LOAD16_BYTE( "rs_u5_s.l1", 0x600000, 0x080000, CRC(1faa04c9) SHA1(817bbd7fc0781d84af6c40cb477adf83cef07ab2) )
	ROM_LOAD16_BYTE( "rs_u6_s.l1", 0x800000, 0x080000, CRC(eee00add) SHA1(96d664ca73ac896e918d7011c1cda3e55e3731b7) )
	ROM_LOAD16_BYTE( "rs_u7_s.l1", 0xa00000, 0x080000, CRC(3a222a54) SHA1(2a788e4ac573bf1d128e5bef9357e62c805014b9) )
	ROM_LOAD16_BYTE( "rs_u8_s.l1", 0xc00000, 0x080000, CRC(c70f2210) SHA1(9be9f271d81d15a4eb123f1377b0c077eef97774) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "524_road_show.u22", 0x0000, 0x2000, CRC(e78ea6a0) SHA1(bf0200177c9d4764b3164cbee37e9a5a35ab996f) )
ROM_END

ROM_START( rs_lx5 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6_lx5.rom", 0x00000, 0x80000, CRC(a2de6ee3) SHA1(90fea1100d5f79c885e693d713b9a113d43131bb) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rs_u2_s.l1", 0x000000, 0x080000, CRC(5a2db20c) SHA1(34ce236cc874b820db2d2268cc77815ed7ca061b) )
	ROM_LOAD16_BYTE( "rs_u3_s.l1", 0x200000, 0x080000, CRC(719be036) SHA1(fa975a6a93fcaefddcbd1c0b97c49bd9f9608ad4) )
	ROM_LOAD16_BYTE( "rs_u4_s.l1", 0x400000, 0x080000, CRC(d452d007) SHA1(b850bc8e17d8940f93c1e7b6a0ab786b092694b3) )
	ROM_LOAD16_BYTE( "rs_u5_s.l1", 0x600000, 0x080000, CRC(1faa04c9) SHA1(817bbd7fc0781d84af6c40cb477adf83cef07ab2) )
	ROM_LOAD16_BYTE( "rs_u6_s.l1", 0x800000, 0x080000, CRC(eee00add) SHA1(96d664ca73ac896e918d7011c1cda3e55e3731b7) )
	ROM_LOAD16_BYTE( "rs_u7_s.l1", 0xa00000, 0x080000, CRC(3a222a54) SHA1(2a788e4ac573bf1d128e5bef9357e62c805014b9) )
	ROM_LOAD16_BYTE( "rs_u8_s.l1", 0xc00000, 0x080000, CRC(c70f2210) SHA1(9be9f271d81d15a4eb123f1377b0c077eef97774) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "524_road_show.u22", 0x0000, 0x2000, CRC(e78ea6a0) SHA1(bf0200177c9d4764b3164cbee37e9a5a35ab996f) )
ROM_END

ROM_START( rs_la4 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6_la4.rom", 0x00000, 0x80000, CRC(d957a038) SHA1(bd78b62eda2046a72eaaee2fff973fe3589f7d88) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rs_u2_s.l1", 0x000000, 0x080000, CRC(5a2db20c) SHA1(34ce236cc874b820db2d2268cc77815ed7ca061b) )
	ROM_LOAD16_BYTE( "rs_u3_s.l1", 0x200000, 0x080000, CRC(719be036) SHA1(fa975a6a93fcaefddcbd1c0b97c49bd9f9608ad4) )
	ROM_LOAD16_BYTE( "rs_u4_s.l1", 0x400000, 0x080000, CRC(d452d007) SHA1(b850bc8e17d8940f93c1e7b6a0ab786b092694b3) )
	ROM_LOAD16_BYTE( "rs_u5_s.l1", 0x600000, 0x080000, CRC(1faa04c9) SHA1(817bbd7fc0781d84af6c40cb477adf83cef07ab2) )
	ROM_LOAD16_BYTE( "rs_u6_s.l1", 0x800000, 0x080000, CRC(eee00add) SHA1(96d664ca73ac896e918d7011c1cda3e55e3731b7) )
	ROM_LOAD16_BYTE( "rs_u7_s.l1", 0xa00000, 0x080000, CRC(3a222a54) SHA1(2a788e4ac573bf1d128e5bef9357e62c805014b9) )
	ROM_LOAD16_BYTE( "rs_u8_s.l1", 0xc00000, 0x080000, CRC(c70f2210) SHA1(9be9f271d81d15a4eb123f1377b0c077eef97774) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "524_road_show.u22", 0x0000, 0x2000, CRC(e78ea6a0) SHA1(bf0200177c9d4764b3164cbee37e9a5a35ab996f) )
ROM_END

ROM_START( rs_lx4 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "rshw_lx4.rom", 0x00000, 0x80000, CRC(866f16a5) SHA1(09180ca87b1b4a9f8f81d81fc2d08092f357205a) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rs_u2_s.l1", 0x000000, 0x080000, CRC(5a2db20c) SHA1(34ce236cc874b820db2d2268cc77815ed7ca061b) )
	ROM_LOAD16_BYTE( "rs_u3_s.l1", 0x200000, 0x080000, CRC(719be036) SHA1(fa975a6a93fcaefddcbd1c0b97c49bd9f9608ad4) )
	ROM_LOAD16_BYTE( "rs_u4_s.l1", 0x400000, 0x080000, CRC(d452d007) SHA1(b850bc8e17d8940f93c1e7b6a0ab786b092694b3) )
	ROM_LOAD16_BYTE( "rs_u5_s.l1", 0x600000, 0x080000, CRC(1faa04c9) SHA1(817bbd7fc0781d84af6c40cb477adf83cef07ab2) )
	ROM_LOAD16_BYTE( "rs_u6_s.l1", 0x800000, 0x080000, CRC(eee00add) SHA1(96d664ca73ac896e918d7011c1cda3e55e3731b7) )
	ROM_LOAD16_BYTE( "rs_u7_s.l1", 0xa00000, 0x080000, CRC(3a222a54) SHA1(2a788e4ac573bf1d128e5bef9357e62c805014b9) )
	ROM_LOAD16_BYTE( "rs_u8_s.l1", 0xc00000, 0x080000, CRC(c70f2210) SHA1(9be9f271d81d15a4eb123f1377b0c077eef97774) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "524_road_show.u22", 0x0000, 0x2000, CRC(e78ea6a0) SHA1(bf0200177c9d4764b3164cbee37e9a5a35ab996f) )
ROM_END

ROM_START( rs_lx3 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6-lx3.rom", 0x00000, 0x80000, CRC(5df17d02) SHA1(94b262c91f906d68d2a6ee9432042a202bf04d35) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rs_u2_s.l1", 0x000000, 0x080000, CRC(5a2db20c) SHA1(34ce236cc874b820db2d2268cc77815ed7ca061b) )
	ROM_LOAD16_BYTE( "rs_u3_s.l1", 0x200000, 0x080000, CRC(719be036) SHA1(fa975a6a93fcaefddcbd1c0b97c49bd9f9608ad4) )
	ROM_LOAD16_BYTE( "rs_u4_s.l1", 0x400000, 0x080000, CRC(d452d007) SHA1(b850bc8e17d8940f93c1e7b6a0ab786b092694b3) )
	ROM_LOAD16_BYTE( "rs_u5_s.l1", 0x600000, 0x080000, CRC(1faa04c9) SHA1(817bbd7fc0781d84af6c40cb477adf83cef07ab2) )
	ROM_LOAD16_BYTE( "rs_u6_s.l1", 0x800000, 0x080000, CRC(eee00add) SHA1(96d664ca73ac896e918d7011c1cda3e55e3731b7) )
	ROM_LOAD16_BYTE( "rs_u7_s.l1", 0xa00000, 0x080000, CRC(3a222a54) SHA1(2a788e4ac573bf1d128e5bef9357e62c805014b9) )
	ROM_LOAD16_BYTE( "rs_u8_s.l1", 0xc00000, 0x080000, CRC(c70f2210) SHA1(9be9f271d81d15a4eb123f1377b0c077eef97774) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "524_road_show.u22", 0x0000, 0x2000, CRC(e78ea6a0) SHA1(bf0200177c9d4764b3164cbee37e9a5a35ab996f) )
ROM_END

ROM_START( rs_lx2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "rshw_lx2.rom", 0x00000, 0x80000, CRC(317210d0) SHA1(38adcf9c72552bd371b096080b172c63d0f843d3) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rs_u2_s.l1", 0x000000, 0x080000, CRC(5a2db20c) SHA1(34ce236cc874b820db2d2268cc77815ed7ca061b) )
	ROM_LOAD16_BYTE( "rs_u3_s.l1", 0x200000, 0x080000, CRC(719be036) SHA1(fa975a6a93fcaefddcbd1c0b97c49bd9f9608ad4) )
	ROM_LOAD16_BYTE( "rs_u4_s.l1", 0x400000, 0x080000, CRC(d452d007) SHA1(b850bc8e17d8940f93c1e7b6a0ab786b092694b3) )
	ROM_LOAD16_BYTE( "rs_u5_s.l1", 0x600000, 0x080000, CRC(1faa04c9) SHA1(817bbd7fc0781d84af6c40cb477adf83cef07ab2) )
	ROM_LOAD16_BYTE( "rs_u6_s.l1", 0x800000, 0x080000, CRC(eee00add) SHA1(96d664ca73ac896e918d7011c1cda3e55e3731b7) )
	ROM_LOAD16_BYTE( "rs_u7_s.l1", 0xa00000, 0x080000, CRC(3a222a54) SHA1(2a788e4ac573bf1d128e5bef9357e62c805014b9) )
	ROM_LOAD16_BYTE( "rs_u8_s.l1", 0xc00000, 0x080000, CRC(c70f2210) SHA1(9be9f271d81d15a4eb123f1377b0c077eef97774) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "524_road_show.u22", 0x0000, 0x2000, CRC(e78ea6a0) SHA1(bf0200177c9d4764b3164cbee37e9a5a35ab996f) )
ROM_END

ROM_START( rs_pa2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "rs-u6.pa2", 0x00000, 0x80000, CRC(674fd680) SHA1(68a4ef5c40fea6b5eae69fbd0ea4339b5757d572) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rs_u2_s.p3", 0x000000, 0x080000, CRC(3a987553) SHA1(bb174cc48533e19906b5ef9d099670e03a04cc89) )
	ROM_LOAD16_BYTE( "rs_u3_s.l1", 0x200000, 0x080000, CRC(719be036) SHA1(fa975a6a93fcaefddcbd1c0b97c49bd9f9608ad4) )
	ROM_LOAD16_BYTE( "rs_u4_s.l1", 0x400000, 0x080000, CRC(d452d007) SHA1(b850bc8e17d8940f93c1e7b6a0ab786b092694b3) )
	ROM_LOAD16_BYTE( "rs_u5_s.l1", 0x600000, 0x080000, CRC(1faa04c9) SHA1(817bbd7fc0781d84af6c40cb477adf83cef07ab2) )
	ROM_LOAD16_BYTE( "rs_u6_s.l1", 0x800000, 0x080000, CRC(eee00add) SHA1(96d664ca73ac896e918d7011c1cda3e55e3731b7) )
	ROM_LOAD16_BYTE( "rs_u7_s.l1", 0xa00000, 0x080000, CRC(3a222a54) SHA1(2a788e4ac573bf1d128e5bef9357e62c805014b9) )
	ROM_LOAD16_BYTE( "rs_u8_s.l1", 0xc00000, 0x080000, CRC(c70f2210) SHA1(9be9f271d81d15a4eb123f1377b0c077eef97774) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "524_road_show.u22", 0x0000, 0x2000, CRC(e78ea6a0) SHA1(bf0200177c9d4764b3164cbee37e9a5a35ab996f) )
ROM_END

/*------------------------
/  The Flintstones #50029
/------------------------*/
ROM_START( fs_lx5 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "flin_lx5.rom", 0x00000, 0x80000, CRC(06707244) SHA1(d86d4564fb27a81e8ab896e2efaf05f4f4a4a152) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fs_u2_s.l1", 0x000000, 0x080000, CRC(aa3da768) SHA1(b9ab9d716f03c3fa4dc7352993477c021a07138a) )
	ROM_LOAD16_BYTE( "fs_u3_s.l1", 0x200000, 0x080000, CRC(e8a0b2d1) SHA1(5fd7ff4a194f845db53573a1a44efbfffed292f9) )
	ROM_LOAD16_BYTE( "fs_u4_s.l1", 0x400000, 0x080000, CRC(a5de69f4) SHA1(a7e7f35964ec8b40a971920c2c6cf2ecb730bc60) )
	ROM_LOAD16_BYTE( "fs_u5_s.l1", 0x600000, 0x080000, CRC(74b4d495) SHA1(98a145c07694db7b56f5c6ba84bc631fb5c18bae) )
	ROM_LOAD16_BYTE( "fs_u6_s.l1", 0x800000, 0x080000, CRC(3c7f7a04) SHA1(45e017dc36922ad2ff420724f912e109a75a15a3) )
	ROM_LOAD16_BYTE( "fs_u7_s.l1", 0xa00000, 0x080000, CRC(f32b9271) SHA1(19308cb54ae6fc6343ab7411546b251ba66b0905) )
	ROM_LOAD16_BYTE( "fs_u8_s.l1", 0xc00000, 0x080000, CRC(a7aafa3e) SHA1(54dca32dc2bec5432cd3664bb5aa45d367560b96) )
	ROM_LOAD16_BYTE( "fs_u9_s.l1", 0xe00000, 0x080000, CRC(0a6664fb) SHA1(751a726e3ea6a808bb137f3563d54acd1580836d) )
ROM_END

ROM_START( fs_lx2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "flin_lx2.rom", 0x00000, 0x80000, CRC(cbab53cd) SHA1(e58ac50326f7acae4d732c2db92e86dd8162e760) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fs_u2_s.l1", 0x000000, 0x080000, CRC(aa3da768) SHA1(b9ab9d716f03c3fa4dc7352993477c021a07138a) )
	ROM_LOAD16_BYTE( "fs_u3_s.l1", 0x200000, 0x080000, CRC(e8a0b2d1) SHA1(5fd7ff4a194f845db53573a1a44efbfffed292f9) )
	ROM_LOAD16_BYTE( "fs_u4_s.l1", 0x400000, 0x080000, CRC(a5de69f4) SHA1(a7e7f35964ec8b40a971920c2c6cf2ecb730bc60) )
	ROM_LOAD16_BYTE( "fs_u5_s.l1", 0x600000, 0x080000, CRC(74b4d495) SHA1(98a145c07694db7b56f5c6ba84bc631fb5c18bae) )
	ROM_LOAD16_BYTE( "fs_u6_s.l1", 0x800000, 0x080000, CRC(3c7f7a04) SHA1(45e017dc36922ad2ff420724f912e109a75a15a3) )
	ROM_LOAD16_BYTE( "fs_u7_s.l1", 0xa00000, 0x080000, CRC(f32b9271) SHA1(19308cb54ae6fc6343ab7411546b251ba66b0905) )
	ROM_LOAD16_BYTE( "fs_u8_s.l1", 0xc00000, 0x080000, CRC(a7aafa3e) SHA1(54dca32dc2bec5432cd3664bb5aa45d367560b96) )
	ROM_LOAD16_BYTE( "fs_u9_s.l1", 0xe00000, 0x080000, CRC(0a6664fb) SHA1(751a726e3ea6a808bb137f3563d54acd1580836d) )
ROM_END

ROM_START( fs_sp2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "flin_lx5.rom", 0x00000, 0x80000, CRC(06707244) SHA1(d86d4564fb27a81e8ab896e2efaf05f4f4a4a152) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "su2-sp2.rom", 0x000000, 0x080000, CRC(8c627583) SHA1(ddbd5bd06ee83b126025b487d94998da9106ff3f) )
	ROM_LOAD16_BYTE( "fs_u3_s.l1",  0x200000, 0x080000, CRC(e8a0b2d1) SHA1(5fd7ff4a194f845db53573a1a44efbfffed292f9) )
	ROM_LOAD16_BYTE( "fs_u4_s.l1",  0x400000, 0x080000, CRC(a5de69f4) SHA1(a7e7f35964ec8b40a971920c2c6cf2ecb730bc60) )
	ROM_LOAD16_BYTE( "fs_u5_s.l1",  0x600000, 0x080000, CRC(74b4d495) SHA1(98a145c07694db7b56f5c6ba84bc631fb5c18bae) )
	ROM_LOAD16_BYTE( "fs_u6_s.l1",  0x800000, 0x080000, CRC(3c7f7a04) SHA1(45e017dc36922ad2ff420724f912e109a75a15a3) )
	ROM_LOAD16_BYTE( "fs_u7_s.l1",  0xa00000, 0x080000, CRC(f32b9271) SHA1(19308cb54ae6fc6343ab7411546b251ba66b0905) )
	ROM_LOAD16_BYTE( "fs_u8_s.l1",  0xc00000, 0x080000, CRC(a7aafa3e) SHA1(54dca32dc2bec5432cd3664bb5aa45d367560b96) )
	ROM_LOAD16_BYTE( "fs_u9_s.l1",  0xe00000, 0x080000, CRC(0a6664fb) SHA1(751a726e3ea6a808bb137f3563d54acd1580836d) )
ROM_END

ROM_START( fs_lx4 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "flin_lx4.rom", 0x00000, 0x80000, CRC(fca5634c) SHA1(8d713c0ba94cfc446fef823d45e268bccb5c6fcc) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fs_u2_s.l1", 0x000000, 0x080000, CRC(aa3da768) SHA1(b9ab9d716f03c3fa4dc7352993477c021a07138a) )
	ROM_LOAD16_BYTE( "fs_u3_s.l1", 0x200000, 0x080000, CRC(e8a0b2d1) SHA1(5fd7ff4a194f845db53573a1a44efbfffed292f9) )
	ROM_LOAD16_BYTE( "fs_u4_s.l1", 0x400000, 0x080000, CRC(a5de69f4) SHA1(a7e7f35964ec8b40a971920c2c6cf2ecb730bc60) )
	ROM_LOAD16_BYTE( "fs_u5_s.l1", 0x600000, 0x080000, CRC(74b4d495) SHA1(98a145c07694db7b56f5c6ba84bc631fb5c18bae) )
	ROM_LOAD16_BYTE( "fs_u6_s.l1", 0x800000, 0x080000, CRC(3c7f7a04) SHA1(45e017dc36922ad2ff420724f912e109a75a15a3) )
	ROM_LOAD16_BYTE( "fs_u7_s.l1", 0xa00000, 0x080000, CRC(f32b9271) SHA1(19308cb54ae6fc6343ab7411546b251ba66b0905) )
	ROM_LOAD16_BYTE( "fs_u8_s.l1", 0xc00000, 0x080000, CRC(a7aafa3e) SHA1(54dca32dc2bec5432cd3664bb5aa45d367560b96) )
	ROM_LOAD16_BYTE( "fs_u9_s.l1", 0xe00000, 0x080000, CRC(0a6664fb) SHA1(751a726e3ea6a808bb137f3563d54acd1580836d) )
ROM_END

ROM_START( fs_lx3 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "flin_lx3.rom", 0x00000, 0x80000, CRC(2298c267) SHA1(6536213d758d05d0c85bf39e9ccc34cac1d849d4) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fs_u2_s.l1", 0x000000, 0x080000, CRC(aa3da768) SHA1(b9ab9d716f03c3fa4dc7352993477c021a07138a) )
	ROM_LOAD16_BYTE( "fs_u3_s.l1", 0x200000, 0x080000, CRC(e8a0b2d1) SHA1(5fd7ff4a194f845db53573a1a44efbfffed292f9) )
	ROM_LOAD16_BYTE( "fs_u4_s.l1", 0x400000, 0x080000, CRC(a5de69f4) SHA1(a7e7f35964ec8b40a971920c2c6cf2ecb730bc60) )
	ROM_LOAD16_BYTE( "fs_u5_s.l1", 0x600000, 0x080000, CRC(74b4d495) SHA1(98a145c07694db7b56f5c6ba84bc631fb5c18bae) )
	ROM_LOAD16_BYTE( "fs_u6_s.l1", 0x800000, 0x080000, CRC(3c7f7a04) SHA1(45e017dc36922ad2ff420724f912e109a75a15a3) )
	ROM_LOAD16_BYTE( "fs_u7_s.l1", 0xa00000, 0x080000, CRC(f32b9271) SHA1(19308cb54ae6fc6343ab7411546b251ba66b0905) )
	ROM_LOAD16_BYTE( "fs_u8_s.l1", 0xc00000, 0x080000, CRC(a7aafa3e) SHA1(54dca32dc2bec5432cd3664bb5aa45d367560b96) )
	ROM_LOAD16_BYTE( "fs_u9_s.l1", 0xe00000, 0x080000, CRC(0a6664fb) SHA1(751a726e3ea6a808bb137f3563d54acd1580836d) )
ROM_END

ROM_START( fs_la5 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "flin_la5.rom", 0x00000, 0x80000, CRC(dbca5636) SHA1(7cc1756e799aebd5124a6092d66c33c9b3616715) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fs_u2_s.l1", 0x000000, 0x080000, CRC(aa3da768) SHA1(b9ab9d716f03c3fa4dc7352993477c021a07138a) )
	ROM_LOAD16_BYTE( "fs_u3_s.l1", 0x200000, 0x080000, CRC(e8a0b2d1) SHA1(5fd7ff4a194f845db53573a1a44efbfffed292f9) )
	ROM_LOAD16_BYTE( "fs_u4_s.l1", 0x400000, 0x080000, CRC(a5de69f4) SHA1(a7e7f35964ec8b40a971920c2c6cf2ecb730bc60) )
	ROM_LOAD16_BYTE( "fs_u5_s.l1", 0x600000, 0x080000, CRC(74b4d495) SHA1(98a145c07694db7b56f5c6ba84bc631fb5c18bae) )
	ROM_LOAD16_BYTE( "fs_u6_s.l1", 0x800000, 0x080000, CRC(3c7f7a04) SHA1(45e017dc36922ad2ff420724f912e109a75a15a3) )
	ROM_LOAD16_BYTE( "fs_u7_s.l1", 0xa00000, 0x080000, CRC(f32b9271) SHA1(19308cb54ae6fc6343ab7411546b251ba66b0905) )
	ROM_LOAD16_BYTE( "fs_u8_s.l1", 0xc00000, 0x080000, CRC(a7aafa3e) SHA1(54dca32dc2bec5432cd3664bb5aa45d367560b96) )
	ROM_LOAD16_BYTE( "fs_u9_s.l1", 0xe00000, 0x080000, CRC(0a6664fb) SHA1(751a726e3ea6a808bb137f3563d54acd1580836d) )
ROM_END

/*--------------------------
/ The Pinball Circus #60020
/--------------------------*/

/*-------------------
/  The Shadow #50032
/--------------------*/
ROM_START( ts_lh6 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "shad_h6.rom", 0x00000, 0x080000, CRC(0a72268d) SHA1(97836afc23c4160bca462f14c115b17e58fe5a48) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ts_u2_s.l1", 0x000000, 0x080000, CRC(f1486cfb) SHA1(a916917cb4e46b5d1e04eb4dd52b4193e48d4da8) )
	ROM_LOAD16_BYTE( "ts_u3_s.l1", 0x200000, 0x080000, CRC(b9e39c3f) SHA1(183730dcaa84f8b83b6d26521e90fdb0fc558b4c) )
	ROM_LOAD16_BYTE( "ts_u4_s.l1", 0x400000, 0x080000, CRC(a1d1ab66) SHA1(5380f347cb3970bac4aab5917a51d2d64fbca541) )
	ROM_LOAD16_BYTE( "ts_u5_s.l1", 0x600000, 0x080000, CRC(ab8cf435) SHA1(86d7f9eca3e49e184700a0ac0f672349fc1241bb) )
	ROM_LOAD16_BYTE( "ts_u6_s.l1", 0x800000, 0x080000, CRC(63b8d2db) SHA1(a662a3280a377ac91fdf55d98d2204e024668706) )
	ROM_LOAD16_BYTE( "ts_u7_s.l1", 0xa00000, 0x080000, CRC(62b5db14) SHA1(13832c8573623f9d541de8b814aa10cfb527be99) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "532_the_shadow.u22", 0x0000, 0x2000, CRC(3d47e89f) SHA1(fdefa59cd8d569dd2700899fb46a55faf76b04dc) )
ROM_END

ROM_START( ts_la2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cpu-u6l2.rom", 0x00000, 0x080000, CRC(e4cff76a) SHA1(37c01f8c6e88186f3b88808bbfee75005ca4008d) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ts_u2_s.l1", 0x000000, 0x080000, CRC(f1486cfb) SHA1(a916917cb4e46b5d1e04eb4dd52b4193e48d4da8) )
	ROM_LOAD16_BYTE( "ts_u3_s.l1", 0x200000, 0x080000, CRC(b9e39c3f) SHA1(183730dcaa84f8b83b6d26521e90fdb0fc558b4c) )
	ROM_LOAD16_BYTE( "ts_u4_s.l1", 0x400000, 0x080000, CRC(a1d1ab66) SHA1(5380f347cb3970bac4aab5917a51d2d64fbca541) )
	ROM_LOAD16_BYTE( "ts_u5_s.l1", 0x600000, 0x080000, CRC(ab8cf435) SHA1(86d7f9eca3e49e184700a0ac0f672349fc1241bb) )
	ROM_LOAD16_BYTE( "ts_u6_s.l1", 0x800000, 0x080000, CRC(63b8d2db) SHA1(a662a3280a377ac91fdf55d98d2204e024668706) )
	ROM_LOAD16_BYTE( "ts_u7_s.l1", 0xa00000, 0x080000, CRC(62b5db14) SHA1(13832c8573623f9d541de8b814aa10cfb527be99) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "532_the_shadow.u22", 0x0000, 0x2000, CRC(3d47e89f) SHA1(fdefa59cd8d569dd2700899fb46a55faf76b04dc) )
ROM_END

ROM_START( ts_la4 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6-la4.rom", 0x00000, 0x080000, CRC(5915cf6d) SHA1(1957988c51b791f76130b8960e9ee61ce17b2088) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ts_u2_s.l1", 0x000000, 0x080000, CRC(f1486cfb) SHA1(a916917cb4e46b5d1e04eb4dd52b4193e48d4da8) )
	ROM_LOAD16_BYTE( "ts_u3_s.l1", 0x200000, 0x080000, CRC(b9e39c3f) SHA1(183730dcaa84f8b83b6d26521e90fdb0fc558b4c) )
	ROM_LOAD16_BYTE( "ts_u4_s.l1", 0x400000, 0x080000, CRC(a1d1ab66) SHA1(5380f347cb3970bac4aab5917a51d2d64fbca541) )
	ROM_LOAD16_BYTE( "ts_u5_s.l1", 0x600000, 0x080000, CRC(ab8cf435) SHA1(86d7f9eca3e49e184700a0ac0f672349fc1241bb) )
	ROM_LOAD16_BYTE( "ts_u6_s.l1", 0x800000, 0x080000, CRC(63b8d2db) SHA1(a662a3280a377ac91fdf55d98d2204e024668706) )
	ROM_LOAD16_BYTE( "ts_u7_s.l1", 0xa00000, 0x080000, CRC(62b5db14) SHA1(13832c8573623f9d541de8b814aa10cfb527be99) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "532_the_shadow.u22", 0x0000, 0x2000, CRC(3d47e89f) SHA1(fdefa59cd8d569dd2700899fb46a55faf76b04dc) )
ROM_END

ROM_START( ts_lx4 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6-lx4.rom", 0x00000, 0x080000, CRC(1d908d38) SHA1(9dbc770ea7b22e27439399f92d81f736a12ddf9f) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ts_u2_s.l1", 0x000000, 0x080000, CRC(f1486cfb) SHA1(a916917cb4e46b5d1e04eb4dd52b4193e48d4da8) )
	ROM_LOAD16_BYTE( "ts_u3_s.l1", 0x200000, 0x080000, CRC(b9e39c3f) SHA1(183730dcaa84f8b83b6d26521e90fdb0fc558b4c) )
	ROM_LOAD16_BYTE( "ts_u4_s.l1", 0x400000, 0x080000, CRC(a1d1ab66) SHA1(5380f347cb3970bac4aab5917a51d2d64fbca541) )
	ROM_LOAD16_BYTE( "ts_u5_s.l1", 0x600000, 0x080000, CRC(ab8cf435) SHA1(86d7f9eca3e49e184700a0ac0f672349fc1241bb) )
	ROM_LOAD16_BYTE( "ts_u6_s.l1", 0x800000, 0x080000, CRC(63b8d2db) SHA1(a662a3280a377ac91fdf55d98d2204e024668706) )
	ROM_LOAD16_BYTE( "ts_u7_s.l1", 0xa00000, 0x080000, CRC(62b5db14) SHA1(13832c8573623f9d541de8b814aa10cfb527be99) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "532_the_shadow.u22", 0x0000, 0x2000, CRC(3d47e89f) SHA1(fdefa59cd8d569dd2700899fb46a55faf76b04dc) )
ROM_END

ROM_START( ts_lx5 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "shad_x5.rom", 0x00000, 0x080000, CRC(bb545f83) SHA1(c2851f7169ca3d28399468967c04e69835f61536) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ts_u2_s.l1", 0x000000, 0x080000, CRC(f1486cfb) SHA1(a916917cb4e46b5d1e04eb4dd52b4193e48d4da8) )
	ROM_LOAD16_BYTE( "ts_u3_s.l1", 0x200000, 0x080000, CRC(b9e39c3f) SHA1(183730dcaa84f8b83b6d26521e90fdb0fc558b4c) )
	ROM_LOAD16_BYTE( "ts_u4_s.l1", 0x400000, 0x080000, CRC(a1d1ab66) SHA1(5380f347cb3970bac4aab5917a51d2d64fbca541) )
	ROM_LOAD16_BYTE( "ts_u5_s.l1", 0x600000, 0x080000, CRC(ab8cf435) SHA1(86d7f9eca3e49e184700a0ac0f672349fc1241bb) )
	ROM_LOAD16_BYTE( "ts_u6_s.l1", 0x800000, 0x080000, CRC(63b8d2db) SHA1(a662a3280a377ac91fdf55d98d2204e024668706) )
	ROM_LOAD16_BYTE( "ts_u7_s.l1", 0xa00000, 0x080000, CRC(62b5db14) SHA1(13832c8573623f9d541de8b814aa10cfb527be99) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "532_the_shadow.u22", 0x0000, 0x2000, CRC(3d47e89f) SHA1(fdefa59cd8d569dd2700899fb46a55faf76b04dc) )
ROM_END

ROM_START( ts_pa1 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "cpu-u6p1.rom", 0x00000, 0x080000, CRC(835b8167) SHA1(70c00dbe7a7c1a188ef9fe303558e248fdf7230a) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "su2-sp2.rom", 0x000000, 0x080000, CRC(ba17f74b) SHA1(9c1f00ea27986d025bcaa6b2ffe8c7c4d2216893) )
	ROM_LOAD16_BYTE( "ts_u3_s.l1",  0x200000, 0x080000, CRC(b9e39c3f) SHA1(183730dcaa84f8b83b6d26521e90fdb0fc558b4c) )
	ROM_LOAD16_BYTE( "ts_u4_s.l1",  0x400000, 0x080000, CRC(a1d1ab66) SHA1(5380f347cb3970bac4aab5917a51d2d64fbca541) )
	ROM_LOAD16_BYTE( "ts_u5_s.l1",  0x600000, 0x080000, CRC(ab8cf435) SHA1(86d7f9eca3e49e184700a0ac0f672349fc1241bb) )
	ROM_LOAD16_BYTE( "ts_u6_s.l1",  0x800000, 0x080000, CRC(63b8d2db) SHA1(a662a3280a377ac91fdf55d98d2204e024668706) )
	ROM_LOAD16_BYTE( "ts_u7_s.l1",  0xa00000, 0x080000, CRC(62b5db14) SHA1(13832c8573623f9d541de8b814aa10cfb527be99) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "532_the_shadow.u22", 0x0000, 0x2000, CRC(3d47e89f) SHA1(fdefa59cd8d569dd2700899fb46a55faf76b04dc) )
ROM_END

ROM_START( ts_lf6 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6-lf6.rom", 0x00000, 0x080000, CRC(a1692f1a) SHA1(9df2ecd991a08c661cc22f91dfc6c3dfffcfc3e5) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ts_u2_s.l1", 0x000000, 0x080000, CRC(f1486cfb) SHA1(a916917cb4e46b5d1e04eb4dd52b4193e48d4da8) )
	ROM_LOAD16_BYTE( "ts_u3_s.l1", 0x200000, 0x080000, CRC(b9e39c3f) SHA1(183730dcaa84f8b83b6d26521e90fdb0fc558b4c) )
	ROM_LOAD16_BYTE( "ts_u4_s.l1", 0x400000, 0x080000, CRC(a1d1ab66) SHA1(5380f347cb3970bac4aab5917a51d2d64fbca541) )
	ROM_LOAD16_BYTE( "ts_u5_s.l1", 0x600000, 0x080000, CRC(ab8cf435) SHA1(86d7f9eca3e49e184700a0ac0f672349fc1241bb) )
	ROM_LOAD16_BYTE( "ts_u6_s.l1", 0x800000, 0x080000, CRC(63b8d2db) SHA1(a662a3280a377ac91fdf55d98d2204e024668706) )
	ROM_LOAD16_BYTE( "ts_u7_s.l1", 0xa00000, 0x080000, CRC(62b5db14) SHA1(13832c8573623f9d541de8b814aa10cfb527be99) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "532_the_shadow.u22", 0x0000, 0x2000, CRC(3d47e89f) SHA1(fdefa59cd8d569dd2700899fb46a55faf76b04dc) )
ROM_END

ROM_START( ts_lf4 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "shad_lf4.rom", 0x00000, 0x080000, CRC(5ff11d88) SHA1(04a2a1cbe9883ec95981bab783886302ec2151d0) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ts_u2_s.l1", 0x000000, 0x080000, CRC(f1486cfb) SHA1(a916917cb4e46b5d1e04eb4dd52b4193e48d4da8) )
	ROM_LOAD16_BYTE( "ts_u3_s.l1", 0x200000, 0x080000, CRC(b9e39c3f) SHA1(183730dcaa84f8b83b6d26521e90fdb0fc558b4c) )
	ROM_LOAD16_BYTE( "ts_u4_s.l1", 0x400000, 0x080000, CRC(a1d1ab66) SHA1(5380f347cb3970bac4aab5917a51d2d64fbca541) )
	ROM_LOAD16_BYTE( "ts_u5_s.l1", 0x600000, 0x080000, CRC(ab8cf435) SHA1(86d7f9eca3e49e184700a0ac0f672349fc1241bb) )
	ROM_LOAD16_BYTE( "ts_u6_s.l1", 0x800000, 0x080000, CRC(63b8d2db) SHA1(a662a3280a377ac91fdf55d98d2204e024668706) )
	ROM_LOAD16_BYTE( "ts_u7_s.l1", 0xa00000, 0x080000, CRC(62b5db14) SHA1(13832c8573623f9d541de8b814aa10cfb527be99) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "532_the_shadow.u22", 0x0000, 0x2000, CRC(3d47e89f) SHA1(fdefa59cd8d569dd2700899fb46a55faf76b04dc) )
ROM_END

ROM_START( ts_lm6 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6-lm6.rom", 0x00000, 0x080000, CRC(56f15859) SHA1(1fd4d64cff8413903474843dbcfcca3d59b33cd8) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ts_u2_s.l1", 0x000000, 0x080000, CRC(f1486cfb) SHA1(a916917cb4e46b5d1e04eb4dd52b4193e48d4da8) )
	ROM_LOAD16_BYTE( "ts_u3_s.l1", 0x200000, 0x080000, CRC(b9e39c3f) SHA1(183730dcaa84f8b83b6d26521e90fdb0fc558b4c) )
	ROM_LOAD16_BYTE( "ts_u4_s.l1", 0x400000, 0x080000, CRC(a1d1ab66) SHA1(5380f347cb3970bac4aab5917a51d2d64fbca541) )
	ROM_LOAD16_BYTE( "ts_u5_s.l1", 0x600000, 0x080000, CRC(ab8cf435) SHA1(86d7f9eca3e49e184700a0ac0f672349fc1241bb) )
	ROM_LOAD16_BYTE( "ts_u6_s.l1", 0x800000, 0x080000, CRC(63b8d2db) SHA1(a662a3280a377ac91fdf55d98d2204e024668706) )
	ROM_LOAD16_BYTE( "ts_u7_s.l1", 0xa00000, 0x080000, CRC(62b5db14) SHA1(13832c8573623f9d541de8b814aa10cfb527be99) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "532_the_shadow.u22", 0x0000, 0x2000, CRC(3d47e89f) SHA1(fdefa59cd8d569dd2700899fb46a55faf76b04dc) )
ROM_END

/*-------------------------
/  Theatre Of Magic #50039
/-------------------------*/
ROM_START( tom_14h )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "1_40h.u6", 0x00000, 0x80000, CRC(4181db9b) SHA1(027ada8518207d5a841ec3cc8c7842c7b3841f70) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "tm_u2_s.l2", 0x000000, 0x080000, CRC(b128fbba) SHA1(59101f9f4f43c240630dfbdc7fb432a9939f122d) )
	ROM_LOAD16_BYTE( "tm_u3_s.l2", 0x200000, 0x080000, CRC(128c7d3c) SHA1(1bd5b56d3f9c8485498746ae6c4d65a1e053161a) )
	ROM_LOAD16_BYTE( "tm_u4_s.l2", 0x400000, 0x080000, CRC(3d9b2354) SHA1(a39917c0cceda33288594652c47fd0385a85b8b1) )
	ROM_LOAD16_BYTE( "tm_u5_s.l2", 0x600000, 0x080000, CRC(44247b60) SHA1(519b9d6eab4fe05676382c5f99ea87d4f7a12c5e) )
	ROM_LOAD16_BYTE( "tm_u6_s.l2", 0x800000, 0x080000, CRC(f366bbe5) SHA1(aca23649a54521748e90aa9a182b9bbdde126409) )
	ROM_LOAD16_BYTE( "tm_u7_s.l2", 0xa00000, 0x080000, CRC(f98e9e38) SHA1(bf8c204cfbbf5f9d59b7ad03d1784d37c638712c) )
ROM_END

ROM_START( tom_13 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tom1_3x.rom", 0x00000, 0x80000, CRC(aff4d14c) SHA1(9896f3034bb7a59c9e241d16bf231fefc0ae1fd0) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "tm_u2_s.l2", 0x000000, 0x080000, CRC(b128fbba) SHA1(59101f9f4f43c240630dfbdc7fb432a9939f122d) )
	ROM_LOAD16_BYTE( "tm_u3_s.l2", 0x200000, 0x080000, CRC(128c7d3c) SHA1(1bd5b56d3f9c8485498746ae6c4d65a1e053161a) )
	ROM_LOAD16_BYTE( "tm_u4_s.l2", 0x400000, 0x080000, CRC(3d9b2354) SHA1(a39917c0cceda33288594652c47fd0385a85b8b1) )
	ROM_LOAD16_BYTE( "tm_u5_s.l2", 0x600000, 0x080000, CRC(44247b60) SHA1(519b9d6eab4fe05676382c5f99ea87d4f7a12c5e) )
	ROM_LOAD16_BYTE( "tm_u6_s.l2", 0x800000, 0x080000, CRC(f366bbe5) SHA1(aca23649a54521748e90aa9a182b9bbdde126409) )
	ROM_LOAD16_BYTE( "tm_u7_s.l2", 0xa00000, 0x080000, CRC(f98e9e38) SHA1(bf8c204cfbbf5f9d59b7ad03d1784d37c638712c) )
ROM_END

ROM_START( tom_12 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tom1_2x.rom", 0x00000, 0x80000, CRC(bd8dd884) SHA1(2cb74ae5082d8ceaf89b8ef4df86f78cb5ba6463) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "tm_u2_s.l2", 0x000000, 0x080000, CRC(b128fbba) SHA1(59101f9f4f43c240630dfbdc7fb432a9939f122d) )
	ROM_LOAD16_BYTE( "tm_u3_s.l2", 0x200000, 0x080000, CRC(128c7d3c) SHA1(1bd5b56d3f9c8485498746ae6c4d65a1e053161a) )
	ROM_LOAD16_BYTE( "tm_u4_s.l2", 0x400000, 0x080000, CRC(3d9b2354) SHA1(a39917c0cceda33288594652c47fd0385a85b8b1) )
	ROM_LOAD16_BYTE( "tm_u5_s.l2", 0x600000, 0x080000, CRC(44247b60) SHA1(519b9d6eab4fe05676382c5f99ea87d4f7a12c5e) )
	ROM_LOAD16_BYTE( "tm_u6_s.l2", 0x800000, 0x080000, CRC(f366bbe5) SHA1(aca23649a54521748e90aa9a182b9bbdde126409) )
	ROM_LOAD16_BYTE( "tm_u7_s.l2", 0xa00000, 0x080000, CRC(f98e9e38) SHA1(bf8c204cfbbf5f9d59b7ad03d1784d37c638712c) )
ROM_END

ROM_START( tom_12a )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tom1_2a.rom", 0x00000, 0x80000, CRC(e560ebef) SHA1(8f975c71f6f70503f237d87655be67182c40fb9e) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "tm_u2_s.l2", 0x000000, 0x080000, CRC(b128fbba) SHA1(59101f9f4f43c240630dfbdc7fb432a9939f122d) )
	ROM_LOAD16_BYTE( "tm_u3_s.l2", 0x200000, 0x080000, CRC(128c7d3c) SHA1(1bd5b56d3f9c8485498746ae6c4d65a1e053161a) )
	ROM_LOAD16_BYTE( "tm_u4_s.l2", 0x400000, 0x080000, CRC(3d9b2354) SHA1(a39917c0cceda33288594652c47fd0385a85b8b1) )
	ROM_LOAD16_BYTE( "tm_u5_s.l2", 0x600000, 0x080000, CRC(44247b60) SHA1(519b9d6eab4fe05676382c5f99ea87d4f7a12c5e) )
	ROM_LOAD16_BYTE( "tm_u6_s.l2", 0x800000, 0x080000, CRC(f366bbe5) SHA1(aca23649a54521748e90aa9a182b9bbdde126409) )
	ROM_LOAD16_BYTE( "tm_u7_s.l2", 0xa00000, 0x080000, CRC(f98e9e38) SHA1(bf8c204cfbbf5f9d59b7ad03d1784d37c638712c) )
ROM_END

ROM_START( tom_10f )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "tom1_0f.rom", 0x00000, 0x80000, CRC(be7626ad) SHA1(2f7918b9d2d0618671d7a8676cf69ee76e86bcb9) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "tm_u2_s.l2", 0x000000, 0x080000, CRC(b128fbba) SHA1(59101f9f4f43c240630dfbdc7fb432a9939f122d) )
	ROM_LOAD16_BYTE( "tm_u3_s.l2", 0x200000, 0x080000, CRC(128c7d3c) SHA1(1bd5b56d3f9c8485498746ae6c4d65a1e053161a) )
	ROM_LOAD16_BYTE( "tm_u4_s.l2", 0x400000, 0x080000, CRC(3d9b2354) SHA1(a39917c0cceda33288594652c47fd0385a85b8b1) )
	ROM_LOAD16_BYTE( "tm_u5_s.l2", 0x600000, 0x080000, CRC(44247b60) SHA1(519b9d6eab4fe05676382c5f99ea87d4f7a12c5e) )
	ROM_LOAD16_BYTE( "tm_u6_s.l2", 0x800000, 0x080000, CRC(f366bbe5) SHA1(aca23649a54521748e90aa9a182b9bbdde126409) )
	ROM_LOAD16_BYTE( "tm_u7_s.l2", 0xa00000, 0x080000, CRC(f98e9e38) SHA1(bf8c204cfbbf5f9d59b7ad03d1784d37c638712c) )
ROM_END

ROM_START( tom_06 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6-06a.rom", 0x00000, 0x80000, CRC(dc1d6681) SHA1(7e60e9fd6e953e3c2899ae2fb2900982f078a4ba) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "tm_u2_s.l2", 0x000000, 0x080000, CRC(b128fbba) SHA1(59101f9f4f43c240630dfbdc7fb432a9939f122d) )
	ROM_LOAD16_BYTE( "tm_u3_s.l2", 0x200000, 0x080000, CRC(128c7d3c) SHA1(1bd5b56d3f9c8485498746ae6c4d65a1e053161a) )
	ROM_LOAD16_BYTE( "tm_u4_s.l2", 0x400000, 0x080000, CRC(3d9b2354) SHA1(a39917c0cceda33288594652c47fd0385a85b8b1) )
	ROM_LOAD16_BYTE( "tm_u5_s.l2", 0x600000, 0x080000, CRC(44247b60) SHA1(519b9d6eab4fe05676382c5f99ea87d4f7a12c5e) )
	ROM_LOAD16_BYTE( "tm_u6_s.l2", 0x800000, 0x080000, CRC(f366bbe5) SHA1(aca23649a54521748e90aa9a182b9bbdde126409) )
	ROM_LOAD16_BYTE( "tm_u7_s.l2", 0xa00000, 0x080000, CRC(f98e9e38) SHA1(bf8c204cfbbf5f9d59b7ad03d1784d37c638712c) )
ROM_END

/*-------------------
/  Who Dunnit #50044
/--------------------*/
ROM_START( wd_12 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "whod1_2.rom", 0x00000, 0x80000, CRC(d49be363) SHA1(a265110170e1debf4a566d91c12e0e4c93838d08) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wdu2_10.rom", 0x000000, 0x080000, CRC(2fd534be) SHA1(9fcbcfb9bc6fc410398413dea71a6fcbe69f761f) )
	ROM_LOAD16_BYTE( "wdu3_10.rom", 0x200000, 0x080000, CRC(be9b312f) SHA1(53038a8a264da4e62455240f2016309462c28275) )
	ROM_LOAD16_BYTE( "wdu4_10.rom", 0x400000, 0x080000, CRC(46965682) SHA1(b12c21a17090480c0960aec808908f2d37c4b498) )
	ROM_LOAD16_BYTE( "wdu5_10.rom", 0x600000, 0x080000, CRC(0a787015) SHA1(e01a19ac0a1b674e2b348d77e584275ef1359cd7) )
	ROM_LOAD16_BYTE( "wdu6_10.rom", 0x800000, 0x080000, CRC(d2e05659) SHA1(3f926dac710adadc38afd70618a84c9f049ebfd0) )
	ROM_LOAD16_BYTE( "wdu7_10.rom", 0xa00000, 0x080000, CRC(36285ca2) SHA1(d42f04aa62b9859ce2452fa05da2049fe39e9411) )
ROM_END

ROM_START( wd_12g )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "whod1_2.rom", 0x00000, 0x80000, CRC(d49be363) SHA1(a265110170e1debf4a566d91c12e0e4c93838d08) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wdu2_20g.rom", 0x000000, 0x080000, CRC(2fe0ce7e) SHA1(ae148809b8f8925376bc6b6b0478176cae490a2b) )
	ROM_LOAD16_BYTE( "wdu3_20g.rom", 0x200000, 0x080000, CRC(f01142ab) SHA1(ee2620b6238df0069c9b10d1fee3ea0607b022da) )
	ROM_LOAD16_BYTE( "wdu4_10.rom",  0x400000, 0x080000, CRC(46965682) SHA1(b12c21a17090480c0960aec808908f2d37c4b498) )
	ROM_LOAD16_BYTE( "wdu5_10.rom",  0x600000, 0x080000, CRC(0a787015) SHA1(e01a19ac0a1b674e2b348d77e584275ef1359cd7) )
	ROM_LOAD16_BYTE( "wdu6_10.rom",  0x800000, 0x080000, CRC(d2e05659) SHA1(3f926dac710adadc38afd70618a84c9f049ebfd0) )
	ROM_LOAD16_BYTE( "wdu7_10.rom",  0xa00000, 0x080000, CRC(36285ca2) SHA1(d42f04aa62b9859ce2452fa05da2049fe39e9411) )
ROM_END

ROM_START( wd_10r )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "whod1_0.rom", 0x00000, 0x80000, CRC(85c29cfe) SHA1(5156d3699f16ac366c063149113ec78232ba787b) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wdu2_10.rom", 0x000000, 0x080000, CRC(2fd534be) SHA1(9fcbcfb9bc6fc410398413dea71a6fcbe69f761f) )
	ROM_LOAD16_BYTE( "wdu3_10.rom", 0x200000, 0x080000, CRC(be9b312f) SHA1(53038a8a264da4e62455240f2016309462c28275) )
	ROM_LOAD16_BYTE( "wdu4_10.rom", 0x400000, 0x080000, CRC(46965682) SHA1(b12c21a17090480c0960aec808908f2d37c4b498) )
	ROM_LOAD16_BYTE( "wdu5_10.rom", 0x600000, 0x080000, CRC(0a787015) SHA1(e01a19ac0a1b674e2b348d77e584275ef1359cd7) )
	ROM_LOAD16_BYTE( "wdu6_10.rom", 0x800000, 0x080000, CRC(d2e05659) SHA1(3f926dac710adadc38afd70618a84c9f049ebfd0) )
	ROM_LOAD16_BYTE( "wdu7_10.rom", 0xa00000, 0x080000, CRC(36285ca2) SHA1(d42f04aa62b9859ce2452fa05da2049fe39e9411) )
ROM_END

ROM_START( wd_11 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "whod1_1.rom", 0x00000, 0x80000, CRC(85cab586) SHA1(3940bff8dfa240f8c0ed96c96f58ab66effbdea5) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wdu2_10.rom", 0x000000, 0x080000, CRC(2fd534be) SHA1(9fcbcfb9bc6fc410398413dea71a6fcbe69f761f) )
	ROM_LOAD16_BYTE( "wdu3_10.rom", 0x200000, 0x080000, CRC(be9b312f) SHA1(53038a8a264da4e62455240f2016309462c28275) )
	ROM_LOAD16_BYTE( "wdu4_10.rom", 0x400000, 0x080000, CRC(46965682) SHA1(b12c21a17090480c0960aec808908f2d37c4b498) )
	ROM_LOAD16_BYTE( "wdu5_10.rom", 0x600000, 0x080000, CRC(0a787015) SHA1(e01a19ac0a1b674e2b348d77e584275ef1359cd7) )
	ROM_LOAD16_BYTE( "wdu6_10.rom", 0x800000, 0x080000, CRC(d2e05659) SHA1(3f926dac710adadc38afd70618a84c9f049ebfd0) )
	ROM_LOAD16_BYTE( "wdu7_10.rom", 0xa00000, 0x080000, CRC(36285ca2) SHA1(d42f04aa62b9859ce2452fa05da2049fe39e9411) )
ROM_END

ROM_START( wd_10f )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6_10f.rom", 0x00000, 0x80000, CRC(86ca3749) SHA1(fa011a39c260f9c3fd8c6f5d18f803f6f0bfe7a0) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wdu2_10.rom", 0x000000, 0x080000, CRC(2fd534be) SHA1(9fcbcfb9bc6fc410398413dea71a6fcbe69f761f) )
	ROM_LOAD16_BYTE( "wdu3_10.rom", 0x200000, 0x080000, CRC(be9b312f) SHA1(53038a8a264da4e62455240f2016309462c28275) )
	ROM_LOAD16_BYTE( "wdu4_10.rom", 0x400000, 0x080000, CRC(46965682) SHA1(b12c21a17090480c0960aec808908f2d37c4b498) )
	ROM_LOAD16_BYTE( "wdu5_10.rom", 0x600000, 0x080000, CRC(0a787015) SHA1(e01a19ac0a1b674e2b348d77e584275ef1359cd7) )
	ROM_LOAD16_BYTE( "wdu6_10.rom", 0x800000, 0x080000, CRC(d2e05659) SHA1(3f926dac710adadc38afd70618a84c9f049ebfd0) )
	ROM_LOAD16_BYTE( "wdu7_10.rom", 0xa00000, 0x080000, CRC(36285ca2) SHA1(d42f04aa62b9859ce2452fa05da2049fe39e9411) )
ROM_END

ROM_START( wd_10g )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6_10g.rom", 0x00000, 0x80000, CRC(fbc17e3f) SHA1(7d9a8c7dda06bb4353517417fdc65d87b6c94167) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wdu2_20g.rom", 0x000000, 0x080000, CRC(2fe0ce7e) SHA1(ae148809b8f8925376bc6b6b0478176cae490a2b) )
	ROM_LOAD16_BYTE( "wdu3_20g.rom", 0x200000, 0x080000, CRC(f01142ab) SHA1(ee2620b6238df0069c9b10d1fee3ea0607b022da) )
	ROM_LOAD16_BYTE( "wdu4_10.rom",  0x400000, 0x080000, CRC(46965682) SHA1(b12c21a17090480c0960aec808908f2d37c4b498) )
	ROM_LOAD16_BYTE( "wdu5_10.rom",  0x600000, 0x080000, CRC(0a787015) SHA1(e01a19ac0a1b674e2b348d77e584275ef1359cd7) )
	ROM_LOAD16_BYTE( "wdu6_10.rom",  0x800000, 0x080000, CRC(d2e05659) SHA1(3f926dac710adadc38afd70618a84c9f049ebfd0) )
	ROM_LOAD16_BYTE( "wdu7_10.rom",  0xa00000, 0x080000, CRC(36285ca2) SHA1(d42f04aa62b9859ce2452fa05da2049fe39e9411) )
ROM_END

ROM_START( wd_03r )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6_03r.rom", 0x00000, 0x80000, CRC(8901868a) SHA1(35d8173865208a08a819275b4d76db3f050f61f1) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "u2-s031.rom", 0x000000, 0x080000, CRC(a265cd93) SHA1(2cebb00119f8fc2022959be2107669c9b4b65bfb) )
	ROM_LOAD16_BYTE( "u3-s031.rom", 0x200000, 0x080000, CRC(16105ac9) SHA1(7b40cc9a30dd4f675fda979f41a22364aa9ea452) )
	ROM_LOAD16_BYTE( "u4-s031.rom", 0x400000, 0x080000, CRC(07d52ef3) SHA1(4bd7dd97316c7244b556b4bd0a8475e282abaa25) )
	ROM_LOAD16_BYTE( "u5-s031.rom", 0x600000, 0x080000, CRC(14fea24c) SHA1(a63e2a7796b89d5a6bca419ceaa14888ae22b7a6) )
	ROM_LOAD16_BYTE( "u6-s031.rom", 0x800000, 0x080000, CRC(d15d073e) SHA1(063412a51de3b6c2bdbde0c3f84132d70c935fb4) )
	ROM_LOAD16_BYTE( "u7-s031.rom", 0xa00000, 0x080000, CRC(d252f599) SHA1(bdce67187c027b713b6ef88f6cd4f025de469929) )
ROM_END

ROM_START( wd_048r )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wd_048r.rom", 0x00000, 0x80000, CRC(45653baa) SHA1(788d5195e61605e151796f5fff9ca8d00820c7a3) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "u2-s031.rom", 0x000000, 0x080000, CRC(a265cd93) SHA1(2cebb00119f8fc2022959be2107669c9b4b65bfb) )
	ROM_LOAD16_BYTE( "u3-s031.rom", 0x200000, 0x080000, CRC(16105ac9) SHA1(7b40cc9a30dd4f675fda979f41a22364aa9ea452) )
	ROM_LOAD16_BYTE( "u4-s031.rom", 0x400000, 0x080000, CRC(07d52ef3) SHA1(4bd7dd97316c7244b556b4bd0a8475e282abaa25) )
	ROM_LOAD16_BYTE( "u5-s031.rom", 0x600000, 0x080000, CRC(14fea24c) SHA1(a63e2a7796b89d5a6bca419ceaa14888ae22b7a6) )
	ROM_LOAD16_BYTE( "u6-s031.rom", 0x800000, 0x080000, CRC(d15d073e) SHA1(063412a51de3b6c2bdbde0c3f84132d70c935fb4) )
	ROM_LOAD16_BYTE( "u7-s031.rom", 0xa00000, 0x080000, CRC(d252f599) SHA1(bdce67187c027b713b6ef88f6cd4f025de469929) )
ROM_END

/*--------------------------
/  World Cup Soccer #50031
/--------------------------*/
ROM_START( wcs_l2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wcup_lx2.rom", 0x00000, 0x80000, CRC(0e4514e8) SHA1(4ef8b78777b8caf1a1ab8f63383c8a7a74d5189a) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wcup_u2.rom", 0x000000, 0x080000, CRC(92252f28) SHA1(962a58ea910bcb90c82c81456a888d45f23fcd9a) )
	ROM_LOAD16_BYTE( "wcup_u3.rom", 0x200000, 0x080000, CRC(83f541ad) SHA1(2d81d89e43f350caba60d5bec8a66560f8556ad8) )
	ROM_LOAD16_BYTE( "wcup_u4.rom", 0x400000, 0x080000, CRC(1540c505) SHA1(aca5a421a0fd067f5411fae2fc3c7c3bcfa1b12f) )
	ROM_LOAD16_BYTE( "wcup_u5.rom", 0x600000, 0x080000, CRC(bddad8d4) SHA1(ae6bb1ca3d97a56d1ba984060a1c1ef6c7a00159) )
	ROM_LOAD16_BYTE( "wcup_u6.rom", 0x800000, 0x080000, CRC(00f46c12) SHA1(64e99eb32908dbb7b90ee8fa92a20aacf800aeac) )
	ROM_LOAD16_BYTE( "wcup_u7.rom", 0xa00000, 0x080000, CRC(fff01703) SHA1(fb8d7212fe562e9933941b7bfc707aed1eb74e79) )
	ROM_LOAD16_BYTE( "wcup_u8.rom", 0xc00000, 0x080000, CRC(670cd382) SHA1(89548420c3b6b8a3d7621b10c538ee1dc6a7be62) )
ROM_END

ROM_START( wcs_l1 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wcs_lx1_1.bin", 0x00000, 0x80000, CRC(b9066ffc) SHA1(9fae6be9c074462e38ab3504b0885ef118e426bf) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wcup_u2.rom", 0x000000, 0x080000, CRC(92252f28) SHA1(962a58ea910bcb90c82c81456a888d45f23fcd9a) )
	ROM_LOAD16_BYTE( "wcup_u3.rom", 0x200000, 0x080000, CRC(83f541ad) SHA1(2d81d89e43f350caba60d5bec8a66560f8556ad8) )
	ROM_LOAD16_BYTE( "wcup_u4.rom", 0x400000, 0x080000, CRC(1540c505) SHA1(aca5a421a0fd067f5411fae2fc3c7c3bcfa1b12f) )
	ROM_LOAD16_BYTE( "wcup_u5.rom", 0x600000, 0x080000, CRC(bddad8d4) SHA1(ae6bb1ca3d97a56d1ba984060a1c1ef6c7a00159) )
	ROM_LOAD16_BYTE( "wcup_u6.rom", 0x800000, 0x080000, CRC(00f46c12) SHA1(64e99eb32908dbb7b90ee8fa92a20aacf800aeac) )
	ROM_LOAD16_BYTE( "wcup_u7.rom", 0xa00000, 0x080000, CRC(fff01703) SHA1(fb8d7212fe562e9933941b7bfc707aed1eb74e79) )
	ROM_LOAD16_BYTE( "wcup_u8.rom", 0xc00000, 0x080000, CRC(670cd382) SHA1(89548420c3b6b8a3d7621b10c538ee1dc6a7be62) )
ROM_END

ROM_START( wcs_la2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wcup_la2.rom", 0x00000, 0x80000, CRC(c13a73f9) SHA1(38b540c42d15580c27f34680dc26432db77644c4) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wcup_u2.rom", 0x000000, 0x080000, CRC(92252f28) SHA1(962a58ea910bcb90c82c81456a888d45f23fcd9a) )
	ROM_LOAD16_BYTE( "wcup_u3.rom", 0x200000, 0x080000, CRC(83f541ad) SHA1(2d81d89e43f350caba60d5bec8a66560f8556ad8) )
	ROM_LOAD16_BYTE( "wcup_u4.rom", 0x400000, 0x080000, CRC(1540c505) SHA1(aca5a421a0fd067f5411fae2fc3c7c3bcfa1b12f) )
	ROM_LOAD16_BYTE( "wcup_u5.rom", 0x600000, 0x080000, CRC(bddad8d4) SHA1(ae6bb1ca3d97a56d1ba984060a1c1ef6c7a00159) )
	ROM_LOAD16_BYTE( "wcup_u6.rom", 0x800000, 0x080000, CRC(00f46c12) SHA1(64e99eb32908dbb7b90ee8fa92a20aacf800aeac) )
	ROM_LOAD16_BYTE( "wcup_u7.rom", 0xa00000, 0x080000, CRC(fff01703) SHA1(fb8d7212fe562e9933941b7bfc707aed1eb74e79) )
	ROM_LOAD16_BYTE( "wcup_u8.rom", 0xc00000, 0x080000, CRC(670cd382) SHA1(89548420c3b6b8a3d7621b10c538ee1dc6a7be62) )
ROM_END

ROM_START( wcs_p3 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "wcup_px3.rom", 0x00000, 0x80000, CRC(617ea2bc) SHA1(f8e025b62d509126fb4ba425ac4a025dcf13ad99) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wcup_u2.rom", 0x000000, 0x080000, CRC(92252f28) SHA1(962a58ea910bcb90c82c81456a888d45f23fcd9a) )
	ROM_LOAD16_BYTE( "wcup_u3.rom", 0x200000, 0x080000, CRC(83f541ad) SHA1(2d81d89e43f350caba60d5bec8a66560f8556ad8) )
	ROM_LOAD16_BYTE( "wcup_u4.rom", 0x400000, 0x080000, CRC(1540c505) SHA1(aca5a421a0fd067f5411fae2fc3c7c3bcfa1b12f) )
	ROM_LOAD16_BYTE( "wcup_u5.rom", 0x600000, 0x080000, CRC(bddad8d4) SHA1(ae6bb1ca3d97a56d1ba984060a1c1ef6c7a00159) )
	ROM_LOAD16_BYTE( "wcup_u6.rom", 0x800000, 0x080000, CRC(00f46c12) SHA1(64e99eb32908dbb7b90ee8fa92a20aacf800aeac) )
	ROM_LOAD16_BYTE( "wcup_u7.rom", 0xa00000, 0x080000, CRC(fff01703) SHA1(fb8d7212fe562e9933941b7bfc707aed1eb74e79) )
	ROM_LOAD16_BYTE( "wcup_u8.rom", 0xc00000, 0x080000, CRC(670cd382) SHA1(89548420c3b6b8a3d7621b10c538ee1dc6a7be62) )
ROM_END

ROM_START( wcs_p2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6-pa2.rom", 0x00000, 0x80000, CRC(8fcb11b3) SHA1(b8549db3dc096b8b3f684bee35bf5dea3d966957) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wcup_u2.rom", 0x000000, 0x080000, CRC(92252f28) SHA1(962a58ea910bcb90c82c81456a888d45f23fcd9a) )
	ROM_LOAD16_BYTE( "wcup_u3.rom", 0x200000, 0x080000, CRC(83f541ad) SHA1(2d81d89e43f350caba60d5bec8a66560f8556ad8) )
	ROM_LOAD16_BYTE( "wcup_u4.rom", 0x400000, 0x080000, CRC(1540c505) SHA1(aca5a421a0fd067f5411fae2fc3c7c3bcfa1b12f) )
	ROM_LOAD16_BYTE( "wcup_u5.rom", 0x600000, 0x080000, CRC(bddad8d4) SHA1(ae6bb1ca3d97a56d1ba984060a1c1ef6c7a00159) )
	ROM_LOAD16_BYTE( "wcup_u6.rom", 0x800000, 0x080000, CRC(00f46c12) SHA1(64e99eb32908dbb7b90ee8fa92a20aacf800aeac) )
	ROM_LOAD16_BYTE( "wcup_u7.rom", 0xa00000, 0x080000, CRC(fff01703) SHA1(fb8d7212fe562e9933941b7bfc707aed1eb74e79) )
	ROM_LOAD16_BYTE( "wcup_u8.rom", 0xc00000, 0x080000, CRC(670cd382) SHA1(89548420c3b6b8a3d7621b10c538ee1dc6a7be62) )
ROM_END

/*------------------------------------------
/ Test Fixture Security generation (#584-S)
/------------------------------------------*/
ROM_START( tfs_12 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u6_12.rom", 0x00000, 0x80000, CRC(12687d19) SHA1(bcc3116328a8c6f0ed430a6d2343d01fcdf2459f) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "u2_10.rom", 0x000000, 0x080000, CRC(d705b41e) SHA1(a7811b4bb1b2b5f7e3d1a809da3363b97dfca680) )
ROM_END

} // anonymous namespace


GAME(1994,  corv_21,    0,          corv,   corv, wpc_s_state,  empty_init, ROT0,  "Bally",        "Corvette (2.1)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  corv_px4,   corv_21,    corv,   corv, wpc_s_state,  empty_init, ROT0,  "Bally",        "Corvette (PX4 Prototype)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  corv_px3,   corv_21,    corv,   corv, wpc_s_state,  empty_init, ROT0,  "Bally",        "Corvette (PX3 Prototype)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  corv_lx1,   corv_21,    corv,   corv, wpc_s_state,  empty_init, ROT0,  "Bally",        "Corvette (LX1)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  corv_lx2,   corv_21,    corv,   corv, wpc_s_state,  empty_init, ROT0,  "Bally",        "Corvette (LX2)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  corv_la1,   corv_21,    corv,   corv, wpc_s_state,  empty_init, ROT0,  "Bally",        "Corvette (LA1)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  dh_lx2,     0,          dh,     dh,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Dirty Harry (LX-2)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  dh_lf2,     dh_lx2,     dh,     dh,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Dirty Harry (LF-2)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  i500_11r,   0,          i500,   i500, wpc_s_state,  empty_init, ROT0,  "Bally",        "Indianapolis 500 (1.1R)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  i500_10r,   i500_11r,   i500,   i500, wpc_s_state,  empty_init, ROT0,  "Bally",        "Indianapolis 500 (1.0R)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  i500_11b,   i500_11r,   i500,   i500, wpc_s_state,  empty_init, ROT0,  "Bally",        "Indianapolis 500 (1.1 Belgium)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  jb_10r,     0,          jb,     jb,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Jack*Bot (1.0R)",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  jb_10b,     jb_10r,     jb,     jb,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Jack*Bot (1.0B) (Belgium/Canada)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  jb_04a,     jb_10r,     jb,     jb,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Jack*Bot (0.4A prototype)",                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  jm_12r,     0,          jm,     jm,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Johnny Mnemonic (1.2R)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  jm_12b,     jm_12r,     jm,     jm,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Johnny Mnemonic (1.2B) Belgium",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  jm_05r,     jm_12r,     jm,     jm,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Johnny Mnemonic (0.5R)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  nf_23x,     0,          nf,     nf,   wpc_s_state,  empty_init, ROT0,  "Williams",     "No Fear: Dangerous Sports (2.3X)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  nf_23,      nf_23x,     nf,     nf,   wpc_s_state,  empty_init, ROT0,  "Williams",     "No Fear: Dangerous Sports (2.3)",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  nf_23f,     nf_23x,     nf,     nf,   wpc_s_state,  empty_init, ROT0,  "Williams",     "No Fear: Dangerous Sports (2.3F)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  nf_22,      nf_23x,     nf,     nf,   wpc_s_state,  empty_init, ROT0,  "Williams",     "No Fear: Dangerous Sports (2.2)",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  nf_20,      nf_23x,     nf,     nf,   wpc_s_state,  empty_init, ROT0,  "Williams",     "No Fear: Dangerous Sports (2.0)",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  nf_10f,     nf_23x,     nf,     nf,   wpc_s_state,  empty_init, ROT0,  "Williams",     "No Fear: Dangerous Sports (1.0F)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  nf_08x,     nf_23x,     nf,     nf,   wpc_s_state,  empty_init, ROT0,  "Williams",     "No Fear: Dangerous Sports (0.8X)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  rs_l6,      0,          rs,     rs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Red and Ted's Road Show (L-6)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  rs_la5,     rs_l6,      rs,     rs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Red and Ted's Road Show (La-5)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  rs_lx5,     rs_l6,      rs,     rs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Red and Ted's Road Show (Lx-5)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  rs_la4,     rs_l6,      rs,     rs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Red and Ted's Road Show (La-4)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  rs_lx4,     rs_l6,      rs,     rs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Red and Ted's Road Show (Lx-4)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  rs_lx3,     rs_l6,      rs,     rs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Red and Ted's Road Show (Lx-3)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  rs_lx2,     rs_l6,      rs,     rs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Red and Ted's Road Show (Lx-2)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  rs_pa2,     rs_l6,      rs,     rs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "Red and Ted's Road Show (PA-2 prototype)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  fs_lx5,     0,          fs,     fs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "The Flintstones (LX-5)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  fs_lx2,     fs_lx5,     fs,     fs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "The Flintstones (LX-2)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  fs_sp2,     fs_lx5,     fs,     fs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "The Flintstones (SP-2)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  fs_lx3,     fs_lx5,     fs,     fs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "The Flintstones (LX-3)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  fs_lx4,     fs_lx5,     fs,     fs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "The Flintstones (LX-4)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  fs_la5,     fs_lx5,     fs,     fs,   wpc_s_state,  empty_init, ROT0,  "Williams",     "The Flintstones (LA-5)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  ts_lx5,     0,          ts,     ts,   wpc_s_state,  empty_init, ROT0,  "Bally",        "The Shadow (LX-5)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  ts_lh6,     ts_lx5,     ts,     ts,   wpc_s_state,  empty_init, ROT0,  "Bally",        "The Shadow (LH-6)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  ts_lx4,     ts_lx5,     ts,     ts,   wpc_s_state,  empty_init, ROT0,  "Bally",        "The Shadow (LX-4)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  ts_la4,     ts_lx5,     ts,     ts,   wpc_s_state,  empty_init, ROT0,  "Bally",        "The Shadow (LA-4)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  ts_la2,     ts_lx5,     ts,     ts,   wpc_s_state,  empty_init, ROT0,  "Bally",        "The Shadow (LA-2)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  ts_pa1,     ts_lx5,     ts,     ts,   wpc_s_state,  empty_init, ROT0,  "Bally",        "The Shadow (PA-1)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  ts_lf6,     ts_lx5,     ts,     ts,   wpc_s_state,  empty_init, ROT0,  "Bally",        "The Shadow (LF-6) French",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  ts_lf4,     ts_lx5,     ts,     ts,   wpc_s_state,  empty_init, ROT0,  "Bally",        "The Shadow (LF-4) French",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  ts_lm6,     ts_lx5,     ts,     ts,   wpc_s_state,  empty_init, ROT0,  "Bally",        "The Shadow (LM-6) Mild",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  tom_13,     0,          tom,    tom,  wpc_s_state,  empty_init, ROT0,  "Bally",        "Theatre Of Magic (1.3X)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(2005,  tom_14h,    tom_13,     tom14,  tom,  wpc_s_state,  empty_init, ROT0,  "Bally",        "Theatre Of Magic (1.4H)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  tom_12,     tom_13,     tom,    tom,  wpc_s_state,  empty_init, ROT0,  "Bally",        "Theatre Of Magic (1.2X)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  tom_12a,    tom_13,     tom,    tom,  wpc_s_state,  empty_init, ROT0,  "Bally",        "Theatre Of Magic (1.2A)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  tom_10f,    tom_13,     tom,    tom,  wpc_s_state,  empty_init, ROT0,  "Bally",        "Theatre Of Magic (1.0 French)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  tom_06,     tom_13,     tom,    tom,  wpc_s_state,  empty_init, ROT0,  "Bally",        "Theatre Of Magic (0.6a)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  wd_12,      0,          wd,     wd,   wpc_s_state,  empty_init, ROT0,  "Bally",        "Who Dunnit (1.2)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  wd_12g,     wd_12,      wd,     wd,   wpc_s_state,  empty_init, ROT0,  "Bally",        "Who Dunnit (1.2 Germany)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  wd_11,      wd_12,      wd,     wd,   wpc_s_state,  empty_init, ROT0,  "Bally",        "Who Dunnit (1.1)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  wd_10r,     wd_12,      wd,     wd,   wpc_s_state,  empty_init, ROT0,  "Bally",        "Who Dunnit (1.0 R)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  wd_10g,     wd_12,      wd,     wd,   wpc_s_state,  empty_init, ROT0,  "Bally",        "Who Dunnit (1.0 Germany)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  wd_10f,     wd_12,      wd,     wd,   wpc_s_state,  empty_init, ROT0,  "Bally",        "Who Dunnit (1.0 French)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  wd_03r,     wd_12,      wd,     wd,   wpc_s_state,  empty_init, ROT0,  "Bally",        "Who Dunnit (0.3 R)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1995,  wd_048r,    wd_12,      wd,     wd,   wpc_s_state,  empty_init, ROT0,  "Bally",        "Who Dunnit (0.48 R)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  wcs_l2,     0,          wcs,    wcs,  wpc_s_state,  empty_init, ROT0,  "Bally",        "World Cup Soccer (Lx-2)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  wcs_l1,     wcs_l2,     wcs,    wcs,  wpc_s_state,  empty_init, ROT0,  "Bally",        "World Cup Soccer (Lx-1)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  wcs_la2,    wcs_l2,     wcs,    wcs,  wpc_s_state,  empty_init, ROT0,  "Bally",        "World Cup Soccer (La-2)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  wcs_p2,     wcs_l2,     wcs,    wcs,  wpc_s_state,  empty_init, ROT0,  "Bally",        "World Cup Soccer (Pa-2)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  wcs_p3,     wcs_l2,     wcs,    wcs,  wpc_s_state,  empty_init, ROT0,  "Bally",        "World Cup Soccer (Px-3)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1994,  tfs_12,     0,          tfs,    tfs,  wpc_s_state,  empty_init, ROT0,  "Bally",        "WPC Test Fixture: Security (1.2)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
