// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Miodrag Milanovic
// Thanks to the PinMAME guys for all their research
/******************************************************************************************
PINBALL
Williams WPC-95

If it says FACTORY SETTINGS, press F3, and wait for the game attract mode to commence.
To increase the volume, open the coin door (NUM2), press NUM+, close coin door (NUM2).
If coin door not closed, ball can't be ended.

Here are the key codes to enable play:

Game                             NUM  Start game              End ball
-------------------------------------------------------------------------------------------
Attack from Mars               50041  Hold QWER hit 1         QWER
Tales of the Arabian Nights    50047  Hold QWER hit 1         QWER
Scared Stiff                   50048  Hold QWER hit 1         QWER
Congo                          50050  Hold QWER hit 1         QWER
Junk Yard                      50052  Hold QWER hit 1         QWER
NBA Fastbreak                  50053  Hold QWER hit 1         QWER
Medievel Madness               50059  Hold QWER hit 1         QWER
No Good Gofers                 50061  Hold QW hit 1           QW
Cirqus Voltaire                50062  Hold QWER hit 1         QWER
The Champion Pub               50063  Hold QWER hit 1         QWER
Monster Bash                   50065  Hold QWER hit 1         QWER
Cactus Canyon                  50066  Hold QWER hit 1         QWER
Phantom Haus                   90001   (not a pinball, not working)
Safe Cracker                   90003  Hold QWE hit 1          QWE (timed game)
Ticket Tac Toe                 90005  Hold X hit 1            (n/a)

ToDo:
- Mechanical sounds
- Phantom Haus: This is a "payout machine" - it gives tokens as prizes - not a pinball.

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
#include "machine/timer.h"

#include "speaker.h"

namespace {

class wpc_95_state : public driver_device
{
public:
	wpc_95_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dcs(*this, "dcs")
		, m_rombank(*this, "rombank")
		, m_mainram(*this, "mainram")
		, m_nvram(*this, "nvram")
		, m_pic(*this, "pic")
		, m_lamp(*this, "lamp")
		, m_out(*this, "out")
	{
	}

	void tf95(machine_config &config) ATTR_COLD;
	void afm(machine_config &config) ATTR_COLD;
	void cc(machine_config &config) ATTR_COLD;
	void cv(machine_config &config) ATTR_COLD;
	void congo(machine_config &config) ATTR_COLD;
	void jy(machine_config &config) ATTR_COLD;
	void mm(machine_config &config) ATTR_COLD;
	void mb(machine_config &config) ATTR_COLD;
	void nbaf(machine_config &config) ATTR_COLD;
	void ngg(machine_config &config) ATTR_COLD;
	void ph(machine_config &config) ATTR_COLD;
	void ss(machine_config &config) ATTR_COLD;
	void totan(machine_config &config) ATTR_COLD;
	void cp(machine_config &config) ATTR_COLD;
	void ttt(machine_config &config) ATTR_COLD;

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void wpc_95(machine_config &config) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<dcs_audio_wpc_device> m_dcs;
	required_memory_bank m_rombank;
	required_shared_ptr<uint8_t> m_mainram;
	required_device<nvram_device> m_nvram;
	required_device<wpc_pic_device> m_pic;
	required_device<wpc_lamp_device> m_lamp;
	required_device<wpc_out_device> m_out;

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

	void wpc_95_map(address_map &map) ATTR_COLD;

	static const char *const lamps_cc[8][8];
	static const char *const outputs_cc[54];
	static const char *const lamps_cv[8][8];
	static const char *const outputs_cv[54];
	static const char *const lamps_congo[8][8];
	static const char *const outputs_congo[54];
	static const char *const lamps_jy[8][8];
	static const char *const outputs_jy[54];
	static const char *const lamps_mm[8][8];
	static const char *const outputs_mm[54];
	static const char *const lamps_mb[8][8];
	static const char *const outputs_mb[54];
	static const char *const lamps_nbaf[8][8];
	static const char *const outputs_nbaf[54];
	static const char *const lamps_ngg[8][8];
	static const char *const outputs_ngg[54];
	static const char *const lamps_ss[8][8];
	static const char *const outputs_ss[54];
	static const char *const lamps_totan[8][8];
	static const char *const outputs_totan[54];
	static const char *const lamps_cp[8][8];
	static const char *const outputs_cp[54];
	static const char *const lamps_ttt[8][8];
	static const char *const outputs_ttt[54];

	uint8_t m_firq_src = 0U, m_zc = 0U;
	uint16_t m_rtc_base_day = 0U;
};

class afm_state : public wpc_95_state
{
public:
	afm_state(const machine_config &mconfig, device_type type, const char *tag)
		: wpc_95_state(mconfig, type, tag)
		, m_saucer_leds(*this, "l:Saucer led %d", 0U)
	{
	}

	void afm(machine_config &config) ATTR_COLD;

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static const char *const lamps_afm[8][8];
	static const char *const outputs_afm[54];

	bool afm_led_handler(int sid, bool state);

	output_finder<16> m_saucer_leds;

	bool m_serial_clock_state = false, m_serial_data_state = false;
	int m_serial_clock_counter = 0;
	uint32_t m_serial_out_state = 0U;
};

class sc_state : public wpc_95_state
{
public:
	sc_state(const machine_config &mconfig, device_type type, const char *tag)
		: wpc_95_state(mconfig, type, tag)
		, m_lamps_extra(*this, lamps_sc_extra)
	{
	}

	void sc(machine_config &config) ATTR_COLD;

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static const char *const lamps_sc[8][8];
	static const char *const lamps_sc_extra[48];
	static const char *const outputs_sc[54];

	void sc_aux_lamps_handler_update(uint32_t &out, uint32_t mask, bool state, int id);
	bool sc_aux_lamps_handler(int sid, bool state);

	output_finder<48> m_lamps_extra;

	bool m_serial_clock_state = false, m_serial_data1_state = false, m_serial_data2_state = false, m_serial_enable = false;
	int m_serial_clock_counter = 0;
	uint32_t m_serial_out1_state = 0U, m_serial_out2_state = 0U;
};

void wpc_95_state::wpc_95_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("mainram");

	map(0x3000, 0x31ff).bankrw("dmd0");
	map(0x3200, 0x33ff).bankrw("dmd2");
	map(0x3400, 0x35ff).bankrw("dmd4");
	map(0x3600, 0x37ff).bankrw("dmd6");
	map(0x3800, 0x39ff).bankrw("dmd8");
	map(0x3a00, 0x3bff).bankrw("dmda");

	map(0x3fb8, 0x3fbf).m("dmd", FUNC(wpc_dmd_device::registers));

	map(0x3fdc, 0x3fdc).rw(FUNC(wpc_95_state::dcs_data_r), FUNC(wpc_95_state::dcs_data_w));
	map(0x3fdd, 0x3fdd).rw(FUNC(wpc_95_state::dcs_ctrl_r), FUNC(wpc_95_state::dcs_reset_w));

	map(0x3fe0, 0x3fe3).w(m_out, FUNC(wpc_out_device::out_w));
	map(0x3fe4, 0x3fe4).nopr().w(m_lamp, FUNC(wpc_lamp_device::row_w));
	map(0x3fe5, 0x3fe5).nopr().w(m_lamp, FUNC(wpc_lamp_device::col_w));
	map(0x3fe6, 0x3fe6).w(m_out, FUNC(wpc_out_device::gi_w));
	map(0x3fe7, 0x3fe7).portr("DSW");
	map(0x3fe8, 0x3fe8).portr("DOOR");
	map(0x3fe9, 0x3fe9).r(m_pic, FUNC(wpc_pic_device::read));
	map(0x3fea, 0x3fea).w(m_pic, FUNC(wpc_pic_device::write));

	map(0x3fee, 0x3fee).w(m_out, FUNC(wpc_out_device::out4_w));
	map(0x3fef, 0x3fef).portr("FLIPPERS");

	map(0x3ff2, 0x3ff2).w(m_out, FUNC(wpc_out_device::led_w));
	map(0x3ff3, 0x3ff3).nopr().w(FUNC(wpc_95_state::irq_ack_w));
	map(0x3ff4, 0x3ff7).m("shift", FUNC(wpc_shift_device::registers));
	map(0x3ff8, 0x3ff8).r(FUNC(wpc_95_state::firq_src_r)).nopw(); // ack?
	map(0x3ffa, 0x3ffb).r(FUNC(wpc_95_state::rtc_r));
	map(0x3ffc, 0x3ffc).w(FUNC(wpc_95_state::bank_w));
	map(0x3ffd, 0x3ffe).noprw(); // memory protection stuff?
	map(0x3fff, 0x3fff).rw(FUNC(wpc_95_state::zc_r), FUNC(wpc_95_state::watchdog_w));
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x8000, 0xffff).rom().region("maincpu", 0xf8000);
}

uint8_t wpc_95_state::dcs_data_r()
{
	return m_dcs->data_r();
}

void wpc_95_state::dcs_data_w(uint8_t data)
{
	m_dcs->data_w(data);
}

uint8_t wpc_95_state::dcs_ctrl_r()
{
	return m_dcs->control_r();
}

void wpc_95_state::dcs_reset_w(uint8_t data)
{
	m_dcs->reset_w(0);
	m_dcs->reset_w(1);
}

uint8_t wpc_95_state::rtc_r(offs_t offset)
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

uint8_t wpc_95_state::firq_src_r()
{
	return m_firq_src;
}

uint8_t wpc_95_state::zc_r()
{
	uint8_t res = m_zc;
	m_zc &= 0x7f;
	return res;
}

TIMER_DEVICE_CALLBACK_MEMBER(wpc_95_state::zc_timer)
{
	m_zc |= 0x80;
}

void wpc_95_state::bank_w(uint8_t data)
{
	m_rombank->set_entry(data & 0x3f);
}

void wpc_95_state::watchdog_w(uint8_t data)
{
}

void wpc_95_state::scanline_irq(int state)
{
	m_firq_src = 0x00;
	m_maincpu->set_input_line(1, state);
}

void wpc_95_state::irq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_maincpu->set_input_line(1, CLEAR_LINE);
}

void wpc_95_state::machine_start()
{
	m_rombank->configure_entries(0, 0x40, memregion("maincpu")->base(), 0x4000);
	m_nvram->set_base(m_mainram, m_mainram.bytes());

	save_item(NAME(m_firq_src));
	save_item(NAME(m_zc));

	// rtc_base_day not saved to give the system a better chance to
	// survive reload some days after unscathed.
}

void afm_state::machine_start()
{
	wpc_95_state::machine_start();

	save_item(NAME(m_serial_clock_state));
	save_item(NAME(m_serial_data_state));
	save_item(NAME(m_serial_clock_counter));
	save_item(NAME(m_serial_out_state));
}

void sc_state::machine_start()
{
	wpc_95_state::machine_start();

	save_item(NAME(m_serial_clock_state));
	save_item(NAME(m_serial_data1_state));
	save_item(NAME(m_serial_data2_state));
	save_item(NAME(m_serial_enable));
	save_item(NAME(m_serial_clock_counter));
	save_item(NAME(m_serial_out1_state));
	save_item(NAME(m_serial_out2_state));
}

void wpc_95_state::machine_reset()
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

void afm_state::machine_reset()
{
	wpc_95_state::machine_reset();

	m_serial_clock_state = m_serial_data_state = false;
	m_serial_clock_counter = 0;
	m_serial_out_state = 0;
}

void sc_state::machine_reset()
{
	wpc_95_state::machine_reset();

	m_serial_clock_state = m_serial_data1_state = m_serial_data2_state = false;
	m_serial_clock_counter = 0;
	m_serial_enable = false;
	m_serial_out1_state = 0;
	m_serial_out2_state = 0;
}

bool afm_state::afm_led_handler(int sid, bool state)
{
	switch(sid) {
	case 37:
		if(!m_serial_clock_state && state) {
			uint16_t mask = 1 << m_serial_clock_counter;
			bool prev_state = m_serial_out_state & mask;
			if(prev_state != m_serial_data_state) {
				m_saucer_leds[m_serial_clock_counter] = m_serial_data_state;
				if(m_serial_data_state)
					m_serial_out_state |= mask;
				else
					m_serial_out_state &= ~mask;
			}
			m_serial_clock_counter = (m_serial_clock_counter+1) & 15;
		}
		m_serial_clock_state = state;
		return true;

	case 38:
		m_serial_data_state = state;
		return true;
	}
	return false;
}

void sc_state::sc_aux_lamps_handler_update(uint32_t &out, uint32_t mask, bool state, int id)
{
	bool prev_state = out & mask;
	if(prev_state != state) {
		m_lamps_extra[id] = state;
		if(state)
			out |= mask;
		else
			out &= ~mask;
	}
}

bool sc_state::sc_aux_lamps_handler(int sid, bool state)
{
	switch(sid) {
	case 37:
		if(m_serial_enable && !state)
			m_serial_clock_counter = 0;

		m_serial_enable = state;
		return true;

	case 38:
		if(!m_serial_clock_state && state && !m_serial_enable) {
			uint32_t mask = 1 << m_serial_clock_counter;
			sc_aux_lamps_handler_update(m_serial_out1_state, mask, m_serial_data1_state, m_serial_clock_counter);
			sc_aux_lamps_handler_update(m_serial_out2_state, mask, m_serial_data2_state, m_serial_clock_counter+24);
			m_serial_clock_counter++;
		}
		m_serial_clock_state = state;
		return true;

	case 39:
		m_serial_data1_state = state;
		return true;

	case 40:
		m_serial_data2_state = state;
		return true;
	}
	return false;
}

const char *const afm_state::lamps_afm[][8] = {
	{ "l:Super jets",         "l:Super jackpot",      "l:Martian atk MB",   "l:Total annihil",    "l:Return to battle", "l:Conquer Mars",       "l:5-way combo",        "l:Drop target"        },
	{ "l:Big-o-Beam 1",       "l:Big-o-Beam 2",       "l:Big-o-Beam 3",     "l:L ramp jackpot",   "l:L ramp arrow",     "l:Lock 2",             "l:Lock 3",             "l:C ramp jackpot"     },
	{ "l:Tractor beam 1",     "l:Tractor beam 2",     "l:Tractor beam 3",   "l:R ramp jackpot",   "l:R ramp arrow",     "l:Martian attack",     "l:Rule Universe",      "l:Stroke of luck"     },
	{ "l:R loop arrow",       "l:C ramp arrow",       "l:Left top lane",    "l:Right top lane",   "l:L motor bank",     "l:C motor bank",       "l:R motor bank",       "l:MAR\"T\"IAN target" },
	{ "l:Attack Mars",        "l:New York, USA",      "l:London, England",  "l:Light lock",       "l:Lock 1",           "l:Pisa, Italy",        "l:Berlin, Germany",    "l:Paris, France"      },
	{ "l:MARTIA\"N\" target", "l:MARTI\"A\"N target", "l:Atomic blaster 1", "l:Atomic blaster 2", "l:Atomic blaster 3", "l:R loop jackpot",     "l:Extra ball",         "l:MART\"I\"AN target" },
	{ "l:Capture 1",          "l:Capture 2",          "l:Capture 3",        "l:L loop jackpot",   "l:L loop arrow",     "l:\"M\"ARTIAN target", "l:M\"A\"RTIAN target", "l:MA\"R\"TIAN target" },
	{ "l:Shoot again",        "l:Left outlane",       "l:Left return",      "l:Right return",     "l:Right outlane",    "l:Launch button",      "l:76", /* dummy */     "l:Start button"       }
};

const char *const afm_state::outputs_afm[] = {
	/*  1 */ "s:Auto plunger",   "s:Trough eject",    "s:Left popper",    "s:Right popper", "s:Left alien lo",   "s:Left alien hi",  "s:Knocker",      "s:Right alien hi",
	/*  9 */ "s:Left slingshot", "s:Right slingshot", "s:Left jet",       "s:Bottom jet",   "s:Right jet",       "s:Right alien lo", "s:Saucer shake", "s:Drop target",
	/* 17 */ "f:R ramp hi (2)",  "f:R ramp lo (2)",   "f:R side hi (2)",  "f:R side lo",    "f:Center arrow",    "f:Jets",           "f:Saucer dome",  "m:Motor bank",
	/* 25 */ "f:L ramp L (2)",   "f:L ramp R (2)",    "f:L side hi (2)",  "f:L side lo",    "s:R flip power",    "s:R flip hold",    "s:L flip power", "s:L flip hold",
	/* 33 */ "s:Right gate",     "s:Left gate",       "s:Diverter power", "s:Diverter hold", "u:37", /* dummy */ "u:38", /* dummy */ "f:Strobe light", "u:40", /* dummy */
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Bottom playfield", "g:Middle playfield", "g:Top playfield", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( afm )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Launch button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Right return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Shooter lane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Left return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Left popper")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Right popper")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Left top lane")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("MARTI\"A\"N")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("MARTIA\"N\"")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("MAR\"T\"IAN")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("MART\"I\"AN")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("L motor bank")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("C motor bank")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("R motor bank")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Right top lane")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Left slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Right slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Bottom jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Right jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("\"M\"ARTIAN")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("M\"A\"RTIAN")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("MA\"R\"TIAN")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("L ramp enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("C ramp enter")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("R ramp enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("L ramp exit")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("R ramp exit")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Motor bank down")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Motor bank up")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Right loop hi")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Right loop lo")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Left loop hi")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Left loop lo")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("L saucer tgt")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("R saucer tgt")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Drop target")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Center trough")

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_cc[][8] = {
	{ "l:Rank: stranger",     "l:Rank: partner",     "l:Rank: deputy",       "l:Rank: sheriff",      "l:Rank: marshall",     "l:Star: motherlode",   "l:L bonus X lane",    "l:R bonus X lane"   },
	{ "l:Bounty beacon",      "l:Jacktop beacon",    "l:Shoot to collect",   "l:EB lit beacon",      "l:Bounty (saloon)",    "l:Saloon arrow",       "l:Extra ball",        "l:Mine lock"        },
	{ "l:RC drop: badguy 3",  "l:L drop: badguy 1",  "l:L standup",          "l:R ramp: snd alarm",  "l:R ramp: shoot out",  "l:R ramp: save Polly", "l:R ramp: jackpot",   "l:R ramp: combo"    },
	{ "l:R loop: combo",      "l:R loop: jacktop",   "l:R loop: marksman",   "l:R loop: gunslinger", "l:R loop: good shot",  "l:L ret: quick draw",  "l:L gunfight pin",    "l:L out: gun fight" },
	{ "l:R drop: badguy 4",   "l:R standup (bot)",   "l:R standup (top)",    "l:C rmp: catch train", "l:C ramp: stop train", "l:C ramp: save Polly", "l:C ramp: jackpot",   "l:C ramp: combo"    },
	{ "l:L rmp: white water", "l:L rmp: water fall", "l:L ramp: save Polly", "l:L ramp: jackpot",    "l:L ramp: combo",      "l:R ret: quick draw",  "l:R out: special",    "l:R gunfight pin"   },
	{ "l:Star: stampede",     "l:Star: combo",       "l:Star: high noon",    "l:L loop: combo",      "l:L loop: jackpot",    "l:L loop: ride 'em",   "l:L loop: wild ride", "l:L loop: B Bronco" },
	{ "l:Star: Bart Bros",    "l:Shoot again",       "l:Star: show down",    "l:LC drop: badguy 2",  "l:74", /* dummy */     "l:75", /* dummy */     "l:76", /* dummy */    "l:Start button"     }
};

const char *const wpc_95_state::outputs_cc[] = {
	/*  1 */ "s:Autoplunger",   "s:#1 (L) drop tgt", "s:#2 (LC) drop tgt", "s:#3 (RC) drop tgt", "s:#4 (R) drop tgt", "s:Mine popper",     "u:07", /* dummy */  "s:Saloon popper",
	/*  9 */ "s:Through eject", "s:L slingshot",     "s:R slingshot",      "s:Left jet",         "s:Right jet",       "s:L gunfight post", "s:R gunfight post", "s:Bottom jet",
	/* 17 */ "s:Mine motor",    "f:Mine flasher",    "f:Front L flasher",  "f:Front R flasher",  "s:L loop gate",     "s:R loop gate",     "u:23", /* dummy */  "f:Beacon flasher",
	/* 25 */ "f:Mid R flasher", "f:Saloon flasher",  "f:Back R flasher",   "f:Back L flasher",   "s:R flip power",    "s:R flip hold",     "s:L flip power",    "s:L flip hold",
	/* 33 */ "s:Move bart toy", "u:34", /* dummy */  "u:35", /* dummy */   "s:Bart toy hat",     "s:Train reverse",   "s:Train forward",   "u:39", /* dummy */  "u:40", /* dummy */
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Illum string 1", "g:Illum string 2", "g:Illum string 3", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( cc )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Mine entrance")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("R return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Shooter lane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("L return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("R standup (bot)")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("L loop bottom")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Rt loop bottom")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Mine popper")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Saloon popper")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("R standup (top)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Beer mug switch")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("L bonus X lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Jet exit")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("L slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("R slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Right jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Bottom jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Right loop top")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("R bonus X lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Left loop top")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Drop #1 (L)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Drop #2 (LC)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Drop #3 (RC)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Drop #4 (R)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("R ramp make")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("R ramp enter")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Skill bowl")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Bot R ramp")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Train encoder")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Train home")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Saloon gate")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Saloon bart toy")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Mine home")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Mine encoder")

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("C ramp enter")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("L ramp make")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("C ramp make")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("L ramp enter")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("L standup (top)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("L standup (bot)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_cv[][8] = {
	{ "l:Cirqus \"R\"",    "l:Grid top",      "l:Cirqus \"Q\"",      "l:Cirqus \"U\"",     "l:Grid top/right",   "l:Cirqus \"S\"",  "l:Grid mid/right",  "l:Left jackpot"    },
	{ "l:Cirqus \"I\"",    "l:Cirqus \"C\"",  "l:Grid mid/left",     "l:Grid bot/left",    "l:Grid bottom",      "l:Grid middle",   "l:Grid bot/right",  "l:Grid top/left"   },
	{ "l:Side show",       "l:Left loop top", "l:Left loop 3",       "l:Left loop 2",      "l:Left loop 1",      "l:\"Multiball\"", "l:\"Lock\"",        "l:\"Spot marvel\"" },
	{ "l:Ringmaster left", "l:Ringmaster 2",  "l:Ringmaster 3",      "l:Ringmaster 4",     "l:Ringmaster right", "l:\"Special\"",   "l:\"Razz\"",        "l:\"Frenzie\""     },
	{ "l:Spin \"S\"",      "l:Spin \"P\"",    "l:Spin \"I\"",        "l:Spin \"N\"",       "l:Right loop top",   "l:Right loop 3",  "l:Right loop 2",    "l:Right loop 1"    },
	{ "l:Middle jackpot",  "l:Right jackpot", "l:\"Light\" standup", "l:\"Lock\" standup", "l:Ring \"R\"",       "l:Ring \"I\"",    "l:\"Shoot again\"", "l:Left outlane"    },
	{ "l:Wow \"W\" right", "l:Wow \"O\"",     "l:Wow \"W\" left",    "l:Ring \"N\"",       "l:Ring \"G\"",       "l:Right outlane", "l:Left inlane",     "l:\"Skill shot\""  },
	{ "l:\"Extra ball\"",  "l:Top jet",       "l:Middle jet",        "l:Lower jet",        "l:Right inlane",     "l:\"Volt\" left", "l:\"Volt\" right",  "l:Start button"    }
};

const char *const wpc_95_state::outputs_cv[] = {
	/*  1 */ "s:Plunger",       "s:Back box kick",    "s:Left loop magnet", "s:Middle jet",     "s:Ramp magnet",     "s:Diverter power", "s:Jet up",          "s:Jet release",
	/*  9 */ "s:Trough eject",  "s:Left sling",       "s:Right sling",      "s:Upper jet",      "s:Lower jet",       "s:Left saucer",    "s:Right saucer",    "s:Lock post",
	/* 17 */ "f:Join flasher",  "f:Ring 1 flasher",   "f:Ring 2 flasher",   "f:Ring 3 flasher", "f:Fl right/BB URT", "s:Motor enable",   "f:Jet flasher",     "f:Fl right/BB ULeft",
	/* 25 */ "f:FL upper left", "f:Fl URight/BB LLT", "f:Ringmaster FLx2",  "f:FL bear/BB LRT", "s:R flip power",    "s:R flip hold",    "s:L flip power",    "s:L flip hold",
	/* 33 */ "s:Popper",        "s:Diverter hold",    "s:Ringmaster mgnt",  "s:Upper post",     "f:Neon",            "u:38", /* dummy */ "s:Motor direction", "u:40", /* dummy */
	/* 41 */ "u:41", /* dummy */
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Playfield right", "g:Playfield middle", "g:Playfield left", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( cv )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Back box luck")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Wire ramp enter")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Left loop upper")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Top Eddy")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Right inlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Shooter lane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Right loop upper")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Inner loop left")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Left inlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Left outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Inner loop right")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Poppor opto")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("\"WOW\" targets")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Top targets")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Left lane")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Ringmaster up")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Ringmaster mid")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Ringmaster down")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Left ramp made")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Trough upper")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Trough middle")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Left loop enter")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Right sling")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Upper jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Middle jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Lower jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Skill shot")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Ring 'N', 'G'")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("\"Light\" standup")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("\"Lock\" standup")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Ramp enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Ramp magnet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Ramp made")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Ramp lock low")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Ramp lock mid")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Ramp lock high")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Left saucer")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Right saucer")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("Big ball rebound")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("\"Volt\" right")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("\"Volt\" left")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RALT) PORT_NAME("Right spinner")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Left spinner")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_congo[][8] = {
	{ "l:(C)ONGO",         "l:C(O)NGO",          "l:CO(N)GO",             "l:CON(G)O",            "l:CONG(O)",         "l:(A)MY",            "l:A(M)Y",            "l:AM(Y)"             },
	{ "l:ZI(N)J",          "l:ZIN(J)",           "l:Jet ex \"Collect\"",  "l:\"Jungle jackpot\"", "l:\"Skill fire\"",  "l:\"You\"",          "l:\"Map\"",          "l:Diamond Rt eject"  },
	{ "l:\"Autofire\"",    "l:Rt ramp ExBall",   "l:Rt ramp \"Collect\"", "l:Diamond Rt Ramp",    "l:Left eject eye",  "l:Diamond Lt eject", "l:\"Mystery\"",      "l:Rt ramp jackpot"   },
	{ "l:Diamond Lt loop", "l:\"We are\"",       "l:Lt loop ExBall",      "l:Lt loop \"Lock\"",   "l:Left bank bot",   "l:\"Skill shot\"",   "l:Left bank center", "l:Left bank top"     },
	{ "l:Left ramp \"P\"", "l:Left ramp \"A\"",  "l:Left ramp \"M\"",     "l:Diamond Lt ramp",    "l:Lt ramp jackpot", "l:(Z)INJ",           "l:Z(I)NJ",           "l:\"Kickback\""      },
	{ "l:Diamond in loop", "l:(G)RAY",           "l:G(R)AY",              "l:GR(A)Y",             "l:GRA(Y)",          "l:\"Watching\"",     "l:Satellite left",   "l:\"Super score\""   },
	{ "l:\"Travi\"",       "l:\"Com\"",          "l:\"Mine shaft\"",      "l:Up loop \"Lock\"",   "l:Diamond up loop", "l:Satellite right",  "l:Satellite center", "l:\"Perimeter def\"" },
	{ "l:(H)IPPO",         "l:H(I)PPO",          "l:HI(P)PO",             "l:HIP(P)O",            "l:HIPP(O)",         "l:\"Shoot again\"",  "l:Launch button",    "l:Start button"      }
};

const char *const wpc_95_state::outputs_congo[] = {
	/*  1 */ "s:Auto plunger",    "s:Kickback",          "s:2-way popper up",  "s:2-way popper dn",   "s:Ramp diverter",    "s:Volcano popper", "s:Backbox knocker", "s:Top loop post",
	/*  9 */ "sTrough eject:",    "s:Left slingshot",    "s:Right slingshot",  "s:Left jet",          "s:Right jet",        "s:Bottom jet",     "s:Gorilla left",    "s:Gorilla right",
	/* 17 */ "f:Amy flasher",     "f:Left ramp fls",     "f:2-way popper fls", "f:Skill shot fls",    "f:Gray gorilla fls", "s:\"Map\" eject",  "s:Left gate",       "s:Right gate",
	/* 25 */ "f:Lower right fls", "f:Right ramp fls",    "f:Volcano flasher",  "f:\"Perimeter def\"", "s:R flip power",     "s:R flip hold",    "s:L flip power",    "s:L flip hold",
	/* 33 */ "s:Up left post",    "s:\"Mystery\" eject", "s:UL flip power",    "s:UL flip hold",      "u:37", /* dummy */   "u:38", /* dummy */ "u:39", /* dummy */  "u:40", /* dummy */
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Playfld gorilla", "g:Playfield top", "g:Playfield bottom", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( congo )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Inner left loop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Upper loop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Jet exit")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Rt return lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Shooter lane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Launch button")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Rt eject rubber")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Lt return lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("\"You\" standup")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Volcano stack")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("\"Mystery\" eject")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Right eject")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Lock ball 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Lock ball 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Lock ball 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("\"Mine shaft\"")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Left loop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Left bank top")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Left bank center")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Left bank bottom")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("\"Travi\"")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("\"Com\"")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("2-way popper")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("\"We are\" standup")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("\"Watching\" standup")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("\"Perimeter def\"")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Left ramp enter")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Left ramp exit")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Left slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Right slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Right jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Bottom jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Right ramp enter")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Right ramp exit")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("(A)MY")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("A(M)Y")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("AM(Y)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("(C)ONGO")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("C(O)NGO")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("CO(N)GO")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("CON(G)O")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("CONG(O)")

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper EOS")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_jy[][8] = {
	{ "l:TL bank bot",      "l:TL bank mid",      "l:TL bank top",      "l:TR bank top",   "l:TR bank mid",     "l:TR bank bot",    "l:Right recycle",    "l:Right crane HU" },
	{ "l:Right 3 bank top", "l:Right 3 bank mid", "l:Right 3 bank bot", "l:Left bank bot", "l:Left bank mid",   "l:Left bank top",  "l:Fan",              "l:Bath tub"       },
	{ "l:Jackpot",          "l:Super jackpot",    "l:Multiball",        "l:Wrecking ball", "l:Radar adventure", "l:Jolopy race",    "l:Toilet adventure", "l:ATC adventure"  },
	{ "l:Gen bus",          "l:Toast",            "l:Magic bus",        "l:Collect junk",  "l:Coo coo clock",   "l:Television",     "l:Weather vane",     "l:Fish bowl"      },
	{ "l:Gen toilet",       "l:Window shopping",  "l:Left recycle",     "l:Left crane HU", "l:Shoot again",     "l:45", /* dummy */ "l:Toaster",          "l:Hair dryer"     },
	{ "l:Propeller",        "l:Outerspace",       "l:DO(G)",            "l:(D)OG",         "l:D(O)G",           "l:Choose junk",    "l:Angel sling",      "l:Bicycles"       },
	{ "l:Time machine",     "l:Start adventure",  "l:Extra ball",       "l:Toast 2",       "l:Gen sewer",       "l:Toaster gun",    "l:Gen alley",        "l:Devil sling"    },
	{ "l:Fire works",       "l:Toxic waste",      "l:Lite extra ball",  "l:Free game",     "l:Lite jackpot",    "l:Gen crane",      "l:76", /* dummy */   "l:Start button"   }
};

const char *const wpc_95_state::outputs_jy[] = {
	/*  1 */ "s:Auto plunger",   "s:Refridge popper",  "s:Power crane",    "u:04", /* dummy */ "s:Scoop down",     "s:Bus diverter",     "s:Knocker",        "u:08", /* dummy */
	/*  9 */ "s:Trough",         "s:Left sling",       "s:Right sling",    "u:12", /* dummy */ "u:13", /* dummy */ "u:14", /* dummy */   "s:Hold crane",     "s:Move dog",
	/* 17 */ "f:1 Fl dog face",  "f:1 Fl window shop", "f:2 Fl autofire",  "f:2 Fl left side", "s:Scoop up",       "f:1 Fl under crane", "f:2 Fl back left", "f:2 Fl back rght",
	/* 25 */ "f:1 Fl shooter",   "f:2 Fl scoop",       "f:1 Fl dog house", "f:2 Fls cars",     "s:R flip power",   "s:R flip hold",      "s:L flip power",   "s:L flip hold",
	/* 33 */ "u:33", /* dummy */ "u:34", /* dummy */   "u:35", /* dummy */ "u:36", /* dummy */ "u:37", /* dummy */ "u:38", /* dummy */   "u:39", /* dummy */ "u:40", /* dummy */
	/* 41 */ "u:41", /* dummy */
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Playfld string 1", "g:Playfld string 2", "g:Logo string", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( jy )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Toaster gun")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Rebound sw")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Top left crane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Left return lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Shooter lane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Rght return lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Crane down")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Lock up 2")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Lock up 1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Top right crane")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Past spinner")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("In the sewer")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Lock jam")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Past crane")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Ramp exit")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Car targ 1 left")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Car targ 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Car targ 3")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Right sling")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Car targ 4")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Car targ 5 rght")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("L L 3 bank bot")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("L L 3 bank mid")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("L L 3 bank top")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("U R 3 bank bot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("U R 3 bank mid")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("U R 3 bank top")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("U L 3 bank bot")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("U L 3 bank mid")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("U L 3 bank top")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Bowl entry")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Bowl exit")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Ramp entry")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Scoop down")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Scoop made")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Dog entry")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("R 3 bank bottom")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("R 3 bank middle")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("R 3 bank top")

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Spinner")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_mm[][8] = {
	{ "l:Right bank top",   "l:Right bank mid",   "l:Right bank bot",   "l:R ramp jackpot",   "l:Save damsel! (2)", "l:Dragon death",    "l:Dragon snack",     "l:Dragon breath"  },
	{ "l:R loop jackpot",   "l:R joust victory!", "l:R clash!",         "l:R charge!",        "l:Patron/peasants",  "l:Catapult ace",    "l:Joust champion",   "l:Castle crusher" },
	{ "l:Trolls!",          "l:Extra ball",       "l:Merlin's magic",   "l:Troll madness",    "l:Damsel madness",   "l:Peasant madness", "l:Catapult madness", "l:Joust madness"  },
	{ "l:L loop jackpot",   "l:L joust victory!", "l:L clash!",         "l:L charge!",        "l:Catapult jackpot", "l:Catapult slam!",  "l:Bam!",             "l:Wham!"          },
	{ "l:Center arrow",     "l:Battle/Kingdom",   "l:Master of trolls", "l:Defender/Damsels", "l:Left top lane",    "l:Right top lane",  "l:L troll target",   "l:R troll target" },
	{ "l:Francois D'Grimm", "l:King of Payne",    "l:Earl of Ego",      "l:L ramp jackpot",   "l:Revolt peasants!", "l:Ugly riot!",      "l:Angry mob!",       "l:Rabble rouser"  },
	{ "l:Howard Hurtz",     "l:Magic shield",     "l:Sir Psycho",       "l:Duke of Bourbon",  "l:Castle lock 2",    "l:Castle lock 1",   "l:Super jackpot",    "l:Super jets (2)" },
	{ "l:Right outlane",    "l:Right return",     "l:Left return",      "l:Left outlane",     "l:Castle lock 3",    "l:Shoot again",     "l:Launch button",    "l:Start button"   }
};

const char *const wpc_95_state::outputs_mm[] = {
	/*  1 */ "s:Auto plunger",    "s:Trough eject",    "s:Left popper",     "s:Castle",        "s:Castle grate pwr", "s:Castle gate hold", "s:Knocker",          "s:Catapult",
	/*  9 */ "s:Right eject",     "s:Left slingshot",  "s:Right slingshot", "s:Left jet",      "s:Bottom jet",       "s:Right jet",        "s:Twr divert power", "s:Twr divert hold",
	/* 17 */ "f:L side lo (2)",   "f:Left ramp (2)",   "f:L side hi (2)",   "f:R side hi (2)", "f:Right ramp (2)",   "f:Cstl R side (2)",  "f:R side lo (2)",    "f:Moat (2)",
	/* 25 */ "f:Cstl L side (2)", "s:Tower lock post", "s:Right gate",      "s:Left gate",     "s:R flip power",     "s:R flip hold",      "s:L flip power",     "s:L flip hold",
	/* 33 */ "s:L troll power",   "s:L troll hold",    "s:R troll power",   "s:R troll hold",  "s:Drawbridge motor", "u:38", /* dummy */   "u:39", /* dummy */   "u:40", /* dummy */
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Bottom playfield", "g:Middle playfield", "g:Top playfield", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( mm )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Launch button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Catapult target")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("L troll target")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Right return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Shooter lane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("R troll target")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Left return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Right eject")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Left popper")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Castle gate")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Catapult")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Moat enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Castle lock")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("L troll (U/pfld)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("R troll (U/pfld)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Left top lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Right top lane")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Left slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Right slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Bottom jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Right jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Drawbridge up")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Drawbridge down")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Tower exit")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("L ramp enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("L ramp exit")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("R ramp enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("R ramp exit")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Left loop lo")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Left loop hi")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Right loop lo")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Right loop hi")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Right bank top")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Right bank mid")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right bank bot")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("L troll up")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("R troll up")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_mb[][8] = {
	{ "l:Monster mosh pit", "l:1/2 moon (2)", "l:Frank arrow",   "l:Drac-attack",      "l:Extra ball",     "l:Monsters of rock", "l:Monster bash",   "l:Mummy mayhem"     },
	{ "l:R ramp arrow",     "l:Rock CD",      "l:Right return",  "l:F moon fever (2)", "l:R gargle",       "l:R warm up",        "l:R primp",        "l:R loop arrow"     },
	{ "l:1/4 moon (2)",     "l:L blue tgt",   "l:Tomb treasure", "l:Drac standup top", "l:Right top lane", "l:Middle top lane",  "l:Left top lane",  "l:Drac standup bot" },
	{ "l:Left return",      "l:Left outlane", "l:3/4 moon (2)",  "l:R blue tgt",       "l:L ramp arrow",   "l:L primp",          "l:L warm up",      "l:L gargle"         },
	{ "l:Guitar",           "l:Drums",        "l:Bass guitar",   "l:Keyboard",         "l:Microphone",     "l:Saxophone",        "l:C loop arrow 3", "l:C blue tgt"       },
	{ "l:Creature",         "l:Bride",        "l:Frankenstein",  "l:Mummy",            "l:Wolfman",        "l:Dracula",          "l:Right outlane",  "l:Shoot again"      },
	{ "l:L frank arm",      "l:L frank leg",  "l:Frank torso",   "l:Frank head",       "l:R frank leg",    "l:R frank arm",      "l:L loop arrow",   "l:67", /* dummy */  },
	{ "l:Muck",             "l:Seaweed",      "l:Algae",         "l:Pond scum",        "l:C loop arrow 2", "l:C loop arrow 1",   "l:Launch button",  "l:Start button"     }
};

const char *const wpc_95_state::outputs_mb[] = {
	/*  1 */ "s:Auto plunger",   "s:Bride post",       "s:Mummy coffin",     "u:04", /* dummy */ "s:Left gate",      "s:Right gate",     "u:07", /* dummy */ "s:Ramp lock post",
	/*  9 */ "s:Trough eject",   "s:Left slignshot",   "s:Right slingshot",  "s:Left jet",       "s:Right jet",      "s:Bottom jet",     "s:Left eject",     "s:Right popper",
	/* 17 */ "f:Wolfman (2)",    "f:Bride",            "f:Frankenstein (2)", "f:Dracula coffin", "f:Creature (2)",   "f:Jets/mummy (2)", "f:Right popper",   "f:Frank arrow",
	/* 25 */ "f:Rock CD",        "f:Wolfman loop (2)", "u:27", /* dummy */   "u:28", /* dummy */ "s:R flip power",   "s:R flip hold",    "s:L flip power",   "s:L flip hold",
	/* 33 */ "u:33", /* dummy */ "u:34", /* dummy */   "u:35", /* dummy */   "u:36", /* dummy */ "u:37", /* dummy */ "u:38", /* dummy */ "u:39", /* dummy */ "u:40", /* dummy */
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Bottom playfield", "g:Top R playfield", "g:Top L playfield", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( mb )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Launch button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Drac standup top")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Drac standup bot")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Right return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Shooter lane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Tomb treasure")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Dracula target")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Left return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Left eject")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Right popper")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("L flip opto")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("R flip opto")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("L blue tgt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("C blue tgt")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("R blue tgt")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("L flip prox")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("R flip prox")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Left slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Right slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Right jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Bottom jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Left top lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Middle top lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Right top lane")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Left loop lo")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Left loop hi")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Right loop lo")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Right loop hi")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Center loop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("L ramp enter")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("L ramp exit")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("C ramp enter")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("R ramp enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("R ramp exit")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("R ramp lock")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Drac position 5")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Drac position 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Drac position 3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("Drac position 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Drac position 1")

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Up/dn bank up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Up/dn bank down")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Frank table down")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Frank table up")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("L up/dn bank tgt")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("R up/dn bank tgt")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Frank hit")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Center spinner")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_nbaf[][8] = {
	{ "l:'20' points",      "l:Free throw",         "l:3 pt",            "l:2 pt",                "l:Field goals",    "l:Multiballs",         "l:Shoot around",       "l:Around the world"   },
	{ "l:Power hoops",      "l:Fastbreak combo",    "l:Alley oop combo", "l:Slam dunk combo",     "l:Combos",         "l:Trophy",             "l:Tip-off combo",      "l:Stadium goodies"    },
	{ "l:Multiball hoops",  "l:Run&shoot hoops",    "l:Hook shot hoops", "l:Half CT hoops",       "l:Lite tip-off",   "l:R \"In the paint\"", "l:SHOO(T)",            "l:Lt return lane"     },
	{ "l:Champ ring 1",     "l:Champ ring 2",       "l:Rt return lane",  "l:Champ ring 4",        "l:Champ ring 3",   "l:L R standup",        "l:U R standup",        "l:Left outlane"       },
	{ "l:Soda",             "l:Question",           "l:Hot dog",         "l:Pizza",               "l:Crazy bob's",    "l:Extra ball",         "l:Right outlane",      "l:\"Shoot again\""    },
	{ "l:Ramps: 3 points",  "l:Tip-off",            "l:Fastbreak",       "l:Alley oop",           "l:Free throw",     "l:SH(O)OT",            "l:\"In the paint\" 4", "l:\"In the paint\" 3" },
	{ "l:L lite fastbreak", "l:Slam dunk",          "l:S(H)OOT",         "l:R lite fastbreak",    "l:Lite slam dunk", "l:SHO(O)T",            "l:\"In the paint\" 1", "l:\"In the paint\" 2" },
	{ "l:Lite alley oop",   "l:L \"In the paint\"", "l:(S)HOOT",         "l:(3)PT",               "l:3(P)T",          "l:3P(T)",              "l:Ball launch",        "l:Start button"       }
};

const char *const wpc_95_state::outputs_nbaf[] = {
	/*  1 */ "s:Autoplunger",   "u:02", /* dummy */  "s:L ramp diverter", "s:R loop diverter", "s:Eject",          "s:Loop gate",       "s:Backbox flipper", "s:Ball catch mag",
	/*  9 */ "s:Trough eject",  "s:Left sling",      "s:Right sling",     "s:Left jet",        "s:Middle jet",     "s:Right jet",       "s:Pass right 2",    "s:Pass left 2",
	/* 17 */ "f:Eject kickout", "f:Left jet bumper", "f:Upper left",      "f:Upper right",     "u:21", /* dummy */ "f:Trophy insert",   "u:23", /* dummy */  "f:Lower left/right",
	/* 25 */ "s:Pass right 1",  "s:Pass left 3",     "s:Pass right 3",    "s:Pass left 4",     "s:R flip power",   "s:R flip hold",     "s:L flip power",    "s:L flip hold",
	/* 33 */ "s:Shoot 1",       "s:Shoot 2",         "s:Shoot 3",         "s:Shoot 4",         "s:Motor enable",   "s:Motor direction", "s:Shot clk enable", "s:Shot clk count",
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:String 1", "g:String 2", "g:String 3", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( nbaf )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Launch button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Backbox basket")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Shooter lane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Lt return lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Rt return lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("L R standup")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Right jet")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Eject hole")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("U R standup")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Center ramp opto")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("R loop ent opto")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Right loop exit")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Standup '3'")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Standup 'P'")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Standup 'T'")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Right ramp enter")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Left ramp enter")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Left ramp made")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Left loop enter")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Left loop made")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Defender pos 4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Defender pos 4")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Defender lock pos")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Defender pos 2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Defender pos 1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Jets ball drain")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("L slingshot")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("R slingshot")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Left jet")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Middle jet")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("L loop ramp exit")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Right ramp made")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("\"In the paint\" 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("\"In the paint\" 3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("\"In the paint\" 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("\"In the paint\" 1")

	PORT_START("X6")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Basket made")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RALT) PORT_NAME("Basket hold")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_ngg[][8] = {
	{ "l:Outlane ex ball", "l:Kickback",      "l:Lower drv range", "l:Shoot again",   "l:Special",         "l:Wheel value",     "l:Jet lightning",  "l:Hole 8"          },
	{ "l:Hole 5",          "l:Hole 4",        "l:Hole 3",          "l:Hit bud",       "l:Hole 1",          "l:2X",              "l:Cart path 2",    "l:5X cart path"    },
	{ "l:Hole 6",          "l:Hole 7",        "l:Hole 2",          "l:Hit buzz",      "l:Hole 9",          "l:4X",              "l:Cart path 4",    "l:3X"              },
	{ "l:Driving range",   "l:Inc golf cart", "l:Inc buzz value",  "l:Inc bud value", "l:Newton drive",    "l:Collect",         "l:Ripoff",         "l:Left loop drive" },
	{ "l:(K)ICK",          "l:K(I)CK",        "l:KI(C)K",          "l:KIC(K)",        "l:Skill shot",      "l:Relight jackpot", "l:Rgt ramp lock",  "l:Rgt ramp drive"  },
	{ "l:4 strokes",       "l:3 strokes",     "l:2 strokes",       "l:5 strokes",     "l:7 strokes",       "l:6 strokes",       "l:Left spinner",   "l:Trap ready"      },
	{ "l:Adv trap",        "l:Center drive",  "l:Center lock",     "l:Get TNT",       "l:Ctr raise gofer", "l:Right spinner",   "l:Rgt loop drive", "l:Bottom jet"      },
	{ "l:Side ramp drive", "l:Extra ball",    "l:Multiball",       "l:Jackpot",       "l:Putt out",        "l:Top jet",         "l:Middle jet",     "l:Start button"    }
};

const char *const wpc_95_state::outputs_ngg[] = {
	/*  1 */ "s:Autofire",        "s:Kickback",        "s:Clubhouse kicker", "s:Left gofer up",   "s:Right gofer up",  "s:Jet popper",     "s:Left eject",      "s:Upper rgt eject",
	/*  9 */ "s:Trough eject",    "s:Left slingshot",  "s:Right slingshot",  "s:Top jet",         "s:Middle jet",      "s:Bottom jet",     "s:Left gofer down", "s:Rgt gofer down",
	/* 17 */ "f:Jet flash",       "f:Lower lft flash", "f:Left spinr flash", "f:Rgt spinr flash", "f:Lower rgt flash", "u:22", /* dummy */ "u:23", /* dummy */  "s:Underground pass",
	/* 25 */ "f:Sand trap flash", "f:Wheel flasher",   "s:Left ramp down",   "s:Right ramp down", "s:R flip power",    "s:R flip hold",    "s:L flip power",    "s:L flip hold",
	/* 33 */ "s:UR flip power",   "s:UR flip hold",    "s:Ball launch ramp", "u:36", /* dummy */  "s:Wheel spin CCW",  "s:Wheel spin CW",  "u:39", /* dummy */  "u:40", /* dummy */
	/* 41 */ "c:Coin meter",
	/* 42 */ "f:Upper right 1", "f:Upper right 2", "f:Upper right 3", "f:Upper pf right", "f:Upper pf left", "f:Upper left 3", "f:Upper left 2", "f:Upper left 1",
	/* 50 */ "g:Left side string", "g:Rgt side string", "g:Gofer spotlight", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( ngg )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Left ramp make")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Center ramp make")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Right inlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Shooter groove")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Jet adv standup")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Underground pass")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Left inlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Kickback")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Trough 5")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Trough 6")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Jet popper")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("L gofer down")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("R gofer down")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Putt out popper")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Rt popper jam")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Right popper")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Left ramp down")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Right ramp down")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Left slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Right slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Top jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Middle jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Bottom jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Top skill shot")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Mid skill shot")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Lower skill shot")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Left spinner")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Right spinner")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Inner wheel opto")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Outer wheel opto")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Left gofer 1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Left gofer 2")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Behind L gofer")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Hole in 1 made")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Left cart path")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Right cart path")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Right ramp make")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Golf cart")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("Right gofer 1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Right gofer 2")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Adv trap value")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Sand trap eject")

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("K-I-C-K advance")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("(K)ICK")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("K(I)CK")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("KI(C)K")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("KIC(K)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Captive ball")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper EOS")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const sc_state::lamps_sc[][8] = {
	{ "l:Lite deposit",     "l:Ctr timer \"10\"", "l:Disable computer", "l:Ctr timer \"5\"",  "l:Ctr timer \"0\"",  "l:Lite lock",       "l:Ctr timer \"55\"",  "l:Ctr timer \"50\""  },
	{ "l:Ctr timer \"15\"", "l:Ctr timer \"20\"", "l:Ctr timer \"25\"", "l:Ctr timer \"30\"", "l:Ctr timer \"35\"", "l:Call guard",      "l:Ctr timer \"45\"",  "l:Ctr timer \"40\""  },
	{ "l:Armor car-cellar", "l:Armor car-roof",   "l:Armor car-main",   "l:Bonus 2X",         "l:(A)LARM standup",  "l:ATM card",        "l:A(L)ARM standup",   "l:Ramp jackpot"      },
	{ "l:Bonus 5X+Outlane", "l:Bonus 5X",         "l:Bonus 4X",         "l:Bonus 3X",         "l:Ramp lock",        "l:AL(A)RM standup", "l:ALA(R)M standup",   "l:ALAR(M) standup"   },
	{ "l:Wheel arrow",      "l:Lire outlanes",    "l:Vault letter",     "l:Explosives",       "l:Note to teller",   "l:Top left lane",   "l:Top middle lane",   "l:Top right lane"    },
	{ "l:TR 3bank top",     "l:TR 3bank middle",  "l:TR 3bank bottom",  "l:TL 3bank top",     "l:TL 3bank middle",  "l:TL 3bank bottom", "l:Rt \"Extra time\"", "l:Right return"      },
	{ "l:BR 3bank top",     "l:BR 3bank middle",  "l:BR 3bank bottom",  "l:BL 3bank top",     "l:BL 3bank middle",  "l:BL 3bank bottom", "l:Left return",       "l:Lt \"Extra time\"" },
	{ "l:Top jet (Yellow)", "l:Left jet (Clear)", "l:Right jet (Red)",  "l:Bank left",        "l:Bank right",       "l:Movng break in",  "l:Roof break in",     "l:Start button"      }
};

const char *const sc_state::lamps_sc_extra[] = {
	"L24 \"!\"",      "L23 \"Teller\"", "L22 \"Dog\"",     "L21 \"?\"",      "L20 \"Alarm 3\"", "L19 \"$\"",      "L18 \"Dog\"",    "L17 \"Candy\"",
	"L16 \"$\"",      "L15 \"?\"",      "L14 \"Alarm 2\"", "L13 \"#\"",      "L12 \"<-->\"",    "L11 \"Teller\"", "L10 \"Bribe\"",  "L9 \"?\"",
	"L8 \"Alarm 1\"", "L7 \"$\"",       "L6 \"Dog\"",      "L5 \"Candy\"",   "L4 \"$\"",        "L3 \"?\"",       "L2 \"Alarm 4\"", "L1 \"$\"",
	"L48 \"Bribe\"",  "L47 \"?\"",      "L46 \"$\"",       "L45 \"?\"",      "L44 \"Cellar\"",  "L43 \"$\"",      "L42 \"?\"",      "L41 \"?\"",
	"L40 \"Vault\"",  "L39 \"Gate 1\"", "L38 \"?\"",       "L37 \"Gate 2\"", "L36 \"?\"",       "L35 \"Gate 3\"", "L34 \"Gate 4\"", "L33 \"?\"",
	"L32 \"?\"",      "L31 \"Bribe\"",  "L30 \"Roof\"",    "L29 \"Bribe\"",  "L28 \"$\"",       "L27 \"?\"",      "L26 \"$\"",      "L25 \"Main\"",
};

const char *const sc_state::outputs_sc[] = {
	/*  1 */ "s:Big kick",         "s:Right token tube", "s:Move tgt reset",  "s:Left token tube", "s:Bank kick",      "s:Top popper up",  "s:Ramp divertor",  "s:Kickback (ramp)",
	/*  9 */ "s:Trough eject",     "s:Left slingshot",   "s:Right slingshot", "s:Left jet",        "s:Right jet",      "s:Top jet",        "s:Top L 3 bank",   "s:Top R 3 bank",
	/* 17 */ "f:Back left",        "f:Jets+Bk Rt (2)",   "f:Right middle",    "f:Right bottom",    "f:Left middle",    "f:Left bottom",    "f:Light rope 1",   "f:Light rope 2",
	/* 25 */ "s:Top popper eject", "s:Top light+motor",  "s:Bot L 3 bank",    "s:Bot R 3 bank",    "s:R flip power",   "s:R flip hold",    "s:L flip power",   "s:L flip hold",
	/* 33 */ "s:UR flip power",    "s:UR flip hold",     "s:Auto plunger",    "s:Lockup release",  "u:37", /* dummy */ "u:38", /* dummy */ "u:39", /* dummy */ "u:40", /* dummy */
	/* 41 */ "u:41", /* dummy */
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Illum string 1", "g:Aux lamp 1 power", "g:Illum string 3", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( sc )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("TP trough (roof)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("TP trough (move)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Right orbit")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Ballshooter")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("UR flip rollover")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Left return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Right return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Left orbit")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Lockup 1 front")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Lockup 2 rear")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Kickback")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Left big kick")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Token chute exit")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Left jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Right jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Top jet")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Left slingshot")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Right slingshot")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("(A)LARM standup")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("A(L)ARM standup")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("AL(A)RM standup")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("ALA(R)M standup")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("ALAR(M) standup")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Moving target C")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Moving target B")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Moving target A")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("TL 3bank top")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("TL 3bank middle")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("TL 3bank bottom")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("TR 3bank bottom")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("TR 3bank middle")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("TR 3bank top")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Top left lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Top popper")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("BL 3bank top")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("BL 3bank middle")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("BL 3bank bottom")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("BR 3bank bottom")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("BR 3bank middle")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("BR 3bank top")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Bank kickout")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Top right lane")

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Left token lvl")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Right token lvl")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Ramp entrance")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Ramp made")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Wheel channel A")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Wheel channel B")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper EOS")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RALT) PORT_NAME("Token coin slot")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_ss[][8] = {
	{ "l:Stiff level 7",   "l:Stiff level 6",    "l:Stiff level 5",    "l:Stiff level 4",    "l:Stiff level 3",    "l:Stiff level 2",    "l:Stiff level 1",    "l:Ramp left eye"   },
	{ "l:Stiff level 8",   "l:Stiff level 9",    "l:Scared stiff",     "l:Center leaper",    "l:Three bank lower", "l:Three bank mid",   "l:Three bank upper", "l:Spider popper"   },
	{ "l:Crate left eye",  "l:Crate cnt lt eye", "l:Crate cnt rt eye", "l:Crate right eye",  "l:Left outlane",     "l:Right leaper",     "l:Rt ramp jackpot",  "l:Lt spin spider"  },
	{ "l:Left leaper",     "l:Lft ramp jackpot", "l:Light lock",       "l:Ramp right eye",   "l:Right outlane",    "l:Skill shot",       "l:Crate jackpot",    "l:Extra ball"      },
	{ "l:Ramp item",       "l:Coffin mult item", "l:Leaper item",      "l:Coffin spotlight", "l:Shoot again",      "l:Lock lamp",        "l:Left loop center", "l:Left loop upper" },
	{ "l:Laboratory item", "l:Crate item",       "l:Skull item",       "l:Web award 2",      "l:Web award 3",      "l:Web award 4",      "l:Web award 5",      "l:Web award 6"     },
	{ "l:Web award 7",     "l:Web award 8",      "l:Web award 9",      "l:Web award 10",     "l:Web award 11",     "l:Web award 12",     "l:Web award 13",     "l:Web award 14"    },
	{ "l:Web award 15",    "l:Web award 16",     "l:Web award 1",      "l:Left skull lane",  "l:Enter skull lane", "l:Right skull lane", "l:76", /* dummy */   "l:Start button"    }
};

const char *const wpc_95_state::outputs_ss[] = {
	/*  1 */ "s:Auto plunger",     "s:Loop gate",       "s:Right popper",    "s:Coffin popper",   "s:Coffin door",    "s:Crate kickout",   "s:Knocker",          "s:Crate post power",
	/*  9 */ "s:Trough eject",     "s:Left sling",      "s:Right sling",     "s:Center jet",      "s:Upper jet",      "s:Lower jet",       "s:Upper slingshot",  "s:Crate post hold",
	/* 17 */ "f:Top jet flasher",  "f:Mid jet flasher", "f:Lower jet flash", "f:Playfield bolts", "f:Blue skull fl",  "f:U right flasher", "f:Lft ramp flasher", "f:C left flasher",
	/* 25 */ "f:White skull fl",   "f:Center TV",       "f:U left flasher",  "f:C right flasher", "s:R flip power",   "s:R flip hold",     "s:L flip power",     "s:L flip hold",
	/* 33 */ "s:Lft divert power", "s:Lft divert hold", "f:L left flasher",  "f:L right flasher", "u:37", /* dummy */ "u:38", /* dummy */  "u:39", /* dummy */   "u:40", /* dummy */
	/* 41 */ "u:41", /* dummy */
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Upper playfield", "g:Center playfield", "g:Lower playfield", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( ss )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Wheel index")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Rt flipper lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Shooter lane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Extra ball lane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Lft flipper lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Single standup")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Right popper")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Left kickout")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Crate entrance")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Coffin left")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Coffin center")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Coffin right")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Left ramp enter")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Right ramp enter")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Left ramp made")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Right ramp made")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Coffin entrance")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Left slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Right slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Upper jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Center jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Lower jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Upper slingshot")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Crate sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Left loop")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Three bank upper")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Three bank mid")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Three bank lower")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Left leaper")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Center leaper")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Right leaper")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Rt ramp 10 point")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Right loop")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left skull lane")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Center skull lane")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Right skull lane")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Secret passage")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_totan[][8] = {
	{ "l:Jewel 1 (left)", "l:Jewel 2",   "l:Jewel 3",        "l:Jewel 4",         "l:Jewel 5",          "l:Jewel 6",  "l:Jewel 7 (right)",  "l:Shoot again"     },
	{ "l:Jackpot",        "l:(G)ENIE",   "l:G(E)NIE",        "l:GE(N)IE",         "l:GEN(I)E",          "l:GENI(E)",  "l:Multiball",        "l:Outlane special" },
	{ "l:Magic carpet",   "l:Action 3",  "l:Ramp arrow rgt", "l:Ramp arrow left", "l:Smoke 1 (bot)",    "l:Smoke 2",  "l:Smoke 3",          "l:Amulet"          },
	{ "l:Smoke 6",        "l:Smoke 7",   "l:Smoke 8",        "l:Smoke 9",         "l:Smoke 10",         "l:Smoke 11", "l:Smoke 12",         "l:Smoke 13"        },
	{ "l:Smoke 14 (top)", "l:Lamp 15",   "l:Lamp 30",        "l:Lamp 60",         "l:Smoke 4",          "l:Smoke 5",  "l:Shoot star right", "l:Shoot star left" },
	{ "l:Make a wish",    "l:(B)AZAAR",  "l:B(A)ZAAR",       "l:BA(Z)AAR",        "l:BAZ(A)AR",         "l:BAZA(A)R", "l:BAZAA(R)",         "l:Center lock"     },
	{ "l:Action 2",       "l:Left lock", "l:Harem advance",  "l:Left tiger loop", "l:Action 1",         "l:Wish 1",   "l:Wish 2",           "l:Wish 3"          },
	{ "l:Right lock",     "l:Action 5",  "l:Extra ball",     "l:Rgt tiger loop",  "l:Captive ball rgt", "l:Action 4", "l:Captive ball lft", "l:Start button"    }
};

const char *const wpc_95_state::outputs_totan[] = {
	/*  1 */ "s:Left cage",       "s:Right cage",    "s:Vanish drop",     "s:Lock eject",    "s:Bazaar eject",   "s:Lock magnet",    "s:Knocker",        "s:Ramp magnet coil",
	/*  9 */ "s:Trough eject",    "s:Left sling",    "s:Right sling",     "s:Left jet",      "s:Right jet",      "s:Middle jet",     "s:Left kick",      "f:Left ej flash",
	/* 17 */ "f:Inlane flashers", "f:Final battle",  "f:Left loop flash", "f:Bazaar flash",  "s:Ramp diverter",  "f:Rub lamp flash", "f:Magic lamp fls", "f:Right loop flash",
	/* 25 */ "f:Start tale fls",  "f:Jet flashers",  "f:Top loop flash",  "f:Ramp flash",    "s:R flip power",   "s:R flip hold",    "s:L flip power",   "s:L flip hold",
	/* 33 */ "s:Left div high",   "s:Left div hold", "s:Vanish magnet",   "s:Loop post div", "u:37", /* dummy */ "u:38", /* dummy */ "u:39", /* dummy */ "u:40", /* dummy */
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Illum string 1", "g:Illum string 2", "g:Illum string 3", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( totan )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Harem passage")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Vanish tunnel")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Ramp enter")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Right inlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Ball shooter")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Genie standup")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Bazaar eject")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Left inlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Left wire make")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Left cage opto")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Right cage opto")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Left eject")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Ramp made left")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Genie target")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Left loop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Inner loop left")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Inner loop right")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Mini standups")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Ramp made right")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Right captive ball")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Right slign")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Right jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Middle jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Lamp spin CCW")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Lamp spin CW")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Left captive ball")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Left standups")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Right standups")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Top skill")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Middle skill")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Bottom skill")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Lock 1 (bot)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Lock 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Lock 3 (top)")

	PORT_START("X6")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_cp[][8] = {
	{ "l:Heavy bag cmplt",   "l:Jump rope cmplt",  "l:Speed bag cmplt", "l:Right jab combo",  "l:Lock",            "l:Rgt start fight",  "l:Right jackpot",    "l:Right jab"        },
	{ "l:Bout 1",            "l:Bout 2",           "l:Bout 3",          "l:Bout 4",           "l:Jump rope",       "l:Left jab combo",   "l:Cnt start fight",  "l:Left jab"         },
	{ "l:Low blue arrow",    "l:Left hook to win", "l:White arrow",     "l:Thrown towel",     "l:Cntr blue arrow", "l:Low yellow arrow", "l:Top yellow arrow", "l:27" /* dummy */   },
	{ "l:Left hook",         "l:Body blow",        "l:Right hook",      "l:Center jackpot",   "l:Left KO boxer",   "l:Hurry up",         "l:Heavy bag",        "l:Right KO boxer"   },
	{ "l:Jackpots cmplt",    "l:Pub champion",     "l:Won by KO",       "l:Multiballs cmplt", "l:Training cmplt",  "l:Speed bag",        "l:Left jackpot",     "l:Balcony"          },
	{ "l:Ultimate chlng",    "l:Poker night",      "l:Extra ball",      "l:Spittn gallery",   "l:Lft start fight", "l:The corner",       "l:Right return",     "l:Rght second wind" },
	{ "l:Raid",              "l:Fistacuff",        "l:Multibrawl",      "l:Three bank top",   "l:Three bank mid",  "l:Three bank bot",   "l:Left return",      "l:Left KO"          },
	{ "l:Right KO",          "l:Left second wind", "l:Top blue arrow",  "l:Center KO",        "l:Ball save post",  "l:Shoot again",      "l:Launch button",    "l:Start button"     }
};

const char *const wpc_95_state::outputs_cp[] = {
	/*  1 */ "s:Auto plunger",    "s:Trough eject",     "s:Left scoop power", "s:Rght scoop power", "s:Corner kickout", "s:Post power",     "s:Rope magnet",    "s:Post divertor",
	/*  9 */ "s:Left scoop hold", "s:Rght scoop hold",  "s:Arm 2",            "s:Post hold",        "s:Arm 1",          "s:Popper",         "s:Left sling",     "s:Right sling",
	/* 17 */ "f:3 fl boxer spot", "f:2 fl dngr zone",   "f:1 fl back pan",    "f:Lock kickout",     "f:Corner kickout", "f:2 boxer red",    "f:Jp rope spot",   "f:Spd bag spot",
	/* 25 */ "s:Rope motor",      "s:Toggle direction", "s:Motor on/off",     "s:Lock pin",         "s:R flip power",   "s:R flip hold",    "s:L flip power",   "s:L flip hold",
	/* 33 */ "s:Rope popper",     "s:Ramp divertor",    "s:Left speed bag",   "s:Right speed bag",  "u:37", /* dummy */ "u:38", /* dummy */ "u:39", /* dummy */ "u:40", /* dummy */
	/* 41 */ "s:Coin meter",
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Illum string 1", "g:Illum string 2", "g:Illum string 3", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( cp )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Made ramp")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Heavy bag")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Lock up 1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Right return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Shooter lane")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Ball launch")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Three bank mid")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Left return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Popper")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Trough 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trough 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Trough 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Trough 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Left jab made")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Corner eject")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Right jab made")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Boxer pole cntr")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Behnd left scoop")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Behnd rght scoop")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Enter ramp")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Jump rope")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Bag pole center")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Boxer pole right")
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Boxer pole left")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Right sling")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Three bank bottom")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Three bank top")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Left half guy")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Rght half guy")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Lock up 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Lock up 3")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Left scoop up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Right scoop up")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Power shot")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Rope cam")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Speed bag")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Boxer gut 1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Boxer gut 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Boxer head")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Exit rope")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Enter speed bag")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("Enter lockup")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Top of ramp")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Enter rope")

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_ttt[][8] = {
	{ "l:Hole \"5\"",     "l:Hole \"6\"",     "l:Hole \"7\"",     "l:Hole \"8\"",     "l:Hole \"9\"",     "l:05", /* dummy */  "l:06", /* dummy */  "l:07" /* dummy */ },
	{ "l:10", /* dummy */ "l:Grid \"1\"",     "l:Grid \"2\"",     "l:Grid \"3\"",     "l:Grid \"4\"",     "l:Grid \"5\"",      "l:Grid \"6\"",      "l:Grid \"7\"",    },
	{ "l:Grid \"8\"",     "l:Grid \"9\"",     "l:Left post",      "l:Right post",     "l:Hole \"1\"",     "l:Hole \"2\"",      "l:Hole \"3\"",      "l:Hole \"4\"",    },
	{ "l:Back panel 1",   "l:Back panel 2",   "l:Back panel 3",   "l:Back panel 4",   "l:Back panel 5",   "l:Back panel 6",    "l:36", /* dummy */  "l:37" /* dummy */ },
	{ "l:Left sling up",  "l:Left sling low", "l:Left ret left",  "l:Left ret right", "l:Right sling up", "l:Right sling low", "l:Right ret right", "l:Right ret left" },
	{ "l:50", /* dummy */ "l:51", /* dummy */ "l:52", /* dummy */ "l:53", /* dummy */ "l:54", /* dummy */ "l:55", /* dummy */  "l:56", /* dummy */  "l:57" /* dummy */ },
	{ "l:60", /* dummy */ "l:61", /* dummy */ "l:62", /* dummy */ "l:63", /* dummy */ "l:64", /* dummy */ "l:65", /* dummy */  "l:66", /* dummy */  "l:67" /* dummy */ },
	{ "l:70", /* dummy */ "l:71", /* dummy */ "l:72", /* dummy */ "l:73", /* dummy */ "l:74", /* dummy */ "l:75", /* dummy */  "l:76", /* dummy */  "l:Start button"   }
};

const char *const wpc_95_state::outputs_ttt[] = {
	/*  1 */ "u:01", /* dummy */ "u:02", /* dummy */ "u:03", /* dummy */ "u:04", /* dummy */  "u:05", /* dummy */  "u:06", /* dummy */  "u:07", /* dummy */  "u:08", /* dummy */
	/*  9 */ "s:Kicker",         "f:Fl left post",   "f:Fl right post",  "f:Fl insert 1",     "f:Fl insert 2",     "f:Fl insert 3",     "f:Fl insert 4",     "u:16", /* dummy */
	/* 17 */ "f:Left flasher",   "f:Right flasher",  "f:Fl left sling",  "f:Fl left return",  "f:Fl back panel 1", "f:Fl back panel 2", "f:Fl back panel 3", "u:24", /* dummy */
	/* 25 */ "u:25", /* dummy */ "s:Ticket lamp",    "f:Fl right sling", "f:Fl right return", "s:R flip power",    "s:R flip hold",     "s:L flip power",    "s:L flip hold",
	/* 33 */ "s:R post power",   "s:R post hold",    "s:L post power",   "s:L post hold",     "u:37", /* dummy */  "u:38", /* dummy */  "u:39", /* dummy */  "u:40", /* dummy */
	/* 41 */ "u:41", /* dummy */
	/* 42 */ "u:42", "u:43", "u:44", "u:45", "u:46", "u:47", "u:48", "u:49", /* dummy row */
	/* 50 */ "g:Insert top", "g:Insert middle", "g:Insert bottom", "u:53", /* dummy */ "u:54" /* dummy */
};

static INPUT_PORTS_START( ttt )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Right sling")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Right post")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Left post")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Left sling")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Hole \"9\"")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Hole \"8\"")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Hole \"7\"")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hole \"6\"")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Hole \"5\"")
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Kicker opto")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Hole \"4\"")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Hole \"3\"")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Hole \"2\"")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Hole \"1\"")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Ticket opto")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Tickets low")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Ticket test")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X5")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

void wpc_95_state::wpc_95(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, XTAL(8'000'000)/4); // 68B09E
	m_maincpu->set_addrmap(AS_PROGRAM, &wpc_95_state::wpc_95_map);
	m_maincpu->set_periodic_int(FUNC(wpc_95_state::irq0_line_assert), attotime::from_hz(XTAL(8'000'000)/8192.0));

	TIMER(config, "zero_crossing").configure_periodic(FUNC(wpc_95_state::zc_timer), attotime::from_hz(120)); // Mains power zero crossing

	WPC_PIC(config, m_pic);
	WPC_LAMP(config, m_lamp);
	WPC_OUT(config, m_out, 3);
	WPC_SHIFT(config, "shift");
	WPC_DMD(config, "dmd").scanline_callback().set(FUNC(wpc_95_state::scanline_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SPEAKER(config, "mono").front_center();

	DCS_AUDIO_WPC(config, m_dcs);
	m_dcs->set_maincpu_tag(m_maincpu);
	m_dcs->add_route(0, "mono", 1.0);
}

void wpc_95_state::tf95(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("648 123456 12345 123");
	//m_lamp->set_names(nullptr);
	//m_out->set_names(nullptr);
}

void afm_state::afm(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("541 123456 12345 123");
	m_lamp->set_names(lamps_afm);
	m_out->set_names(outputs_afm);
	m_out->set_handler(wpc_out_device::handler_t(&afm_state::afm_led_handler, this));
}

void wpc_95_state::cc(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("566 123456 12345 123");
	m_lamp->set_names(lamps_cc);
	m_out->set_names(outputs_cc);
}

void wpc_95_state::cv(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("562 123456 12345 123");
	m_lamp->set_names(lamps_cv);
	m_out->set_names(outputs_cv);
}

void wpc_95_state::congo(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("550 123456 12345 123");
	m_lamp->set_names(lamps_congo);
	m_out->set_names(outputs_congo);
}

void wpc_95_state::jy(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("552 123456 12345 123");
	m_lamp->set_names(lamps_jy);
	m_out->set_names(outputs_jy);
}

void wpc_95_state::mm(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("559 123456 12345 123");
	m_lamp->set_names(lamps_mm);
	m_out->set_names(outputs_mm);
}

void wpc_95_state::mb(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("565 123456 12345 123");
	m_lamp->set_names(lamps_mb);
	m_out->set_names(outputs_mb);
}

void wpc_95_state::nbaf(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("553 123456 12345 123");
	m_lamp->set_names(lamps_nbaf);
	m_out->set_names(outputs_nbaf);
}

void wpc_95_state::ngg(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("561 123456 12345 123");
	m_lamp->set_names(lamps_ngg);
	m_out->set_names(outputs_ngg);
}

void wpc_95_state::ph(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("901 100031 64739 123");
	//m_lamp->set_names(lamps_ph); // TODO
	//m_out->set_names(outputs_ph); // TODO
}

void sc_state::sc(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("903 123456 12345 123");
	m_lamp->set_names(lamps_sc);
	m_out->set_names(outputs_sc);
	m_out->set_handler(wpc_out_device::handler_t(&sc_state::sc_aux_lamps_handler, this));
}

void wpc_95_state::ss(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("548 123456 12345 123");
	m_lamp->set_names(lamps_ss);
	m_out->set_names(outputs_ss);
}

void wpc_95_state::totan(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("547 123456 12345 123");
	m_lamp->set_names(lamps_totan);
	m_out->set_names(outputs_totan);
}

void wpc_95_state::cp(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("563 123456 12345 123");
	m_lamp->set_names(lamps_cp);
	m_out->set_names(outputs_cp);
}

void wpc_95_state::ttt(machine_config &config)
{
	wpc_95(config);

	m_pic->set_serial("905 123456 12345 123");
	m_lamp->set_names(lamps_ttt);
	m_out->set_names(outputs_ttt);
}

/*-------------------------
/  Attack From Mars #50041
/-------------------------*/
ROM_START( afm_11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "mars1_1.rom", 0x00000, 0x80000, CRC(13b174d9) SHA1(57952f3184496b0316e4cf301e0181cb9de3519a) )
	ROM_RELOAD(              0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "afm_s2.l1", 0x000000, 0x100000, CRC(6e39d96e) SHA1(b34e31bb1734c86614f153f7201163aaa9943cec) )
	ROM_LOAD16_BYTE( "afm_s3.l1", 0x200000, 0x100000, CRC(1cbce9b1) SHA1(7f258bfe1904a879a2cb007419483f4fee91e072) )
	ROM_LOAD16_BYTE( "afm_s4.l1", 0x400000, 0x100000, CRC(5ff7fbb7) SHA1(ebaf825d3b90b6acee1920e6703801a4bcddfc5b) )
ROM_END

ROM_START( afm_10 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "afm_1_00.g11", 0x00000, 0x80000, CRC(1a30fe95) SHA1(218674e63ce4efeecb266f35f0f315758f7c72fc) )
	ROM_RELOAD(               0x80000, 0x80000)

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "afm_1_00.s2", 0x000000, 0x100000, CRC(610ff107) SHA1(9590f809a05cb2bda4979fa16f165e2e994719a0) )
	ROM_LOAD16_BYTE( "afm_s3.l1",   0x200000, 0x100000, CRC(1cbce9b1) SHA1(7f258bfe1904a879a2cb007419483f4fee91e072) )
	ROM_LOAD16_BYTE( "afm_s4.l1",   0x400000, 0x100000, CRC(5ff7fbb7) SHA1(ebaf825d3b90b6acee1920e6703801a4bcddfc5b) )
ROM_END

ROM_START( afm_113 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "afm_1_13.bin", 0x000000, 0x100000, CRC(e1fbd81b) SHA1(0ff35253d8eac7b75abb3e4db84cdcca458182cd) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "afm_s2.l1", 0x000000, 0x100000, CRC(6e39d96e) SHA1(b34e31bb1734c86614f153f7201163aaa9943cec) )
	ROM_LOAD16_BYTE( "afm_s3.l1", 0x200000, 0x100000, CRC(1cbce9b1) SHA1(7f258bfe1904a879a2cb007419483f4fee91e072) )
	ROM_LOAD16_BYTE( "afm_s4.l1", 0x400000, 0x100000, CRC(5ff7fbb7) SHA1(ebaf825d3b90b6acee1920e6703801a4bcddfc5b) )
ROM_END

ROM_START( afm_113b )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "afm_113b.bin", 0x000000, 0x100000, CRC(34fd2d7d) SHA1(57a41bd686286429880e63696d7d9d3990ca5d05) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "afm_s2.l1", 0x000000, 0x100000, CRC(6e39d96e) SHA1(b34e31bb1734c86614f153f7201163aaa9943cec) )
	ROM_LOAD16_BYTE( "afm_s3.l1", 0x200000, 0x100000, CRC(1cbce9b1) SHA1(7f258bfe1904a879a2cb007419483f4fee91e072) )
	ROM_LOAD16_BYTE( "afm_s4.l1", 0x400000, 0x100000, CRC(5ff7fbb7) SHA1(ebaf825d3b90b6acee1920e6703801a4bcddfc5b) )
ROM_END

ROM_START( afm_11u )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "marsu1_1.rom", 0x00000, 0x80000, CRC(bc1c0a0a) SHA1(859b40aaae46623f9b3519b5e422977d1724b715) )
	ROM_RELOAD(               0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "afm_s2.l1", 0x000000, 0x100000, CRC(6e39d96e) SHA1(b34e31bb1734c86614f153f7201163aaa9943cec) )
	ROM_LOAD16_BYTE( "afm_s3.l1", 0x200000, 0x100000, CRC(1cbce9b1) SHA1(7f258bfe1904a879a2cb007419483f4fee91e072) )
	ROM_LOAD16_BYTE( "afm_s4.l1", 0x400000, 0x100000, CRC(5ff7fbb7) SHA1(ebaf825d3b90b6acee1920e6703801a4bcddfc5b) )
ROM_END

/*----------------------
/  Cactus Canyon #50066
/----------------------*/
ROM_START( cc_12 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cc_g11.1_2", 0x000000, 0x100000, CRC(17ad9266) SHA1(b18c4e2cc9f4269904c05e5e414675a94f96e955) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sav2_8.rom", 0x000000, 0x100000, CRC(94928841) SHA1(953586d6abe8222a6cd6b74e417fa4ce078efa43) )
	ROM_LOAD16_BYTE( "sav3_8.rom", 0x200000, 0x100000, CRC(a22b13f0) SHA1(5df6ea9d5059cd04bdb369c1c7255b09d64b3c65) )
	ROM_LOAD16_BYTE( "sav4_8.rom", 0x400000, 0x100000, CRC(fe8324e2) SHA1(72c56d094cb4185a083a7da81fd527a908ce9de0) )
	ROM_LOAD16_BYTE( "sav5_8.rom", 0x600000, 0x100000, CRC(1b2a1ff3) SHA1(2d9a5952c7ac000c47d87d198ff7ca62913ec73f) )
	ROM_LOAD16_BYTE( "sav6_8.rom", 0x800000, 0x100000, CRC(2cccf10e) SHA1(3b9b9c87ab3c0d74eaacde416d18f3357f8302bd) )
	ROM_LOAD16_BYTE( "sav7_8.rom", 0xa00000, 0x100000, CRC(90fb1277) SHA1(502c920e1d54d285a4d4af401e574f785149da47) )
ROM_END

ROM_START( cc_13 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cc_g11.1_3", 0x000000, 0x100000, CRC(7741fa4e) SHA1(adaf6b07d2f2714e87e367db28d15ae0145b6ae6) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sav2_8.rom", 0x000000, 0x100000, CRC(94928841) SHA1(953586d6abe8222a6cd6b74e417fa4ce078efa43) )
	ROM_LOAD16_BYTE( "sav3_8.rom", 0x200000, 0x100000, CRC(a22b13f0) SHA1(5df6ea9d5059cd04bdb369c1c7255b09d64b3c65) )
	ROM_LOAD16_BYTE( "sav4_8.rom", 0x400000, 0x100000, CRC(fe8324e2) SHA1(72c56d094cb4185a083a7da81fd527a908ce9de0) )
	ROM_LOAD16_BYTE( "sav5_8.rom", 0x600000, 0x100000, CRC(1b2a1ff3) SHA1(2d9a5952c7ac000c47d87d198ff7ca62913ec73f) )
	ROM_LOAD16_BYTE( "sav6_8.rom", 0x800000, 0x100000, CRC(2cccf10e) SHA1(3b9b9c87ab3c0d74eaacde416d18f3357f8302bd) )
	ROM_LOAD16_BYTE( "sav7_8.rom", 0xa00000, 0x100000, CRC(90fb1277) SHA1(502c920e1d54d285a4d4af401e574f785149da47) )
ROM_END

ROM_START( cc_10 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cc_g11.1_0", 0x000000, 0x100000, CRC(c4e2e838) SHA1(3223dd03353dead0f41626b04c9f019d6fe1528c) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sav2_8.rom", 0x000000, 0x100000, CRC(94928841) SHA1(953586d6abe8222a6cd6b74e417fa4ce078efa43) )
	ROM_LOAD16_BYTE( "sav3_8.rom", 0x200000, 0x100000, CRC(a22b13f0) SHA1(5df6ea9d5059cd04bdb369c1c7255b09d64b3c65) )
	ROM_LOAD16_BYTE( "sav4_8.rom", 0x400000, 0x100000, CRC(fe8324e2) SHA1(72c56d094cb4185a083a7da81fd527a908ce9de0) )
	ROM_LOAD16_BYTE( "sav5_8.rom", 0x600000, 0x100000, CRC(1b2a1ff3) SHA1(2d9a5952c7ac000c47d87d198ff7ca62913ec73f) )
	ROM_LOAD16_BYTE( "sav6_8.rom", 0x800000, 0x100000, CRC(2cccf10e) SHA1(3b9b9c87ab3c0d74eaacde416d18f3357f8302bd) )
	ROM_LOAD16_BYTE( "sav7_8.rom", 0xa00000, 0x100000, CRC(90fb1277) SHA1(502c920e1d54d285a4d4af401e574f785149da47) )
ROM_END

ROM_START( cc_104 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cc_g11.104", 0x000000, 0x100000, CRC(21a7b816) SHA1(0e67da694b8713e15e04bf0c49a48e14f057a737) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sav2_8.rom", 0x000000, 0x100000, CRC(94928841) SHA1(953586d6abe8222a6cd6b74e417fa4ce078efa43) )
	ROM_LOAD16_BYTE( "sav3_8.rom", 0x200000, 0x100000, CRC(a22b13f0) SHA1(5df6ea9d5059cd04bdb369c1c7255b09d64b3c65) )
	ROM_LOAD16_BYTE( "sav4_8.rom", 0x400000, 0x100000, CRC(fe8324e2) SHA1(72c56d094cb4185a083a7da81fd527a908ce9de0) )
	ROM_LOAD16_BYTE( "sav5_8.rom", 0x600000, 0x100000, CRC(1b2a1ff3) SHA1(2d9a5952c7ac000c47d87d198ff7ca62913ec73f) )
	ROM_LOAD16_BYTE( "sav6_8.rom", 0x800000, 0x100000, CRC(2cccf10e) SHA1(3b9b9c87ab3c0d74eaacde416d18f3357f8302bd) )
	ROM_LOAD16_BYTE( "sav7_8.rom", 0xa00000, 0x100000, CRC(90fb1277) SHA1(502c920e1d54d285a4d4af401e574f785149da47) )
ROM_END

/*------------------------
/  Cirqus Voltaire #50062
/------------------------*/
ROM_START( cv_14 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cirq_14.rom", 0x000000, 0x100000, CRC(7a8bf999) SHA1(b33baabf4f6cbf8615cc00eb1286238c5aea386a) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD( "s2v1_0.rom",        0x000000, 0x080000, CRC(79dbb8ee) SHA1(f76c0db93b89beaf1e90c5f2199262e296fb1b78) )
	ROM_RELOAD(                    0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "s3v0_4.rom", 0x200000, 0x100000, CRC(8c6c0c56) SHA1(792431cc5b06c3d5028168297614f5eb7e8af34f) )
	ROM_LOAD16_BYTE( "s4v0_4.rom", 0x400000, 0x100000, CRC(a9014b78) SHA1(abffe32ab729fb39ab2360d850c8b5476094fd92) )
	ROM_LOAD16_BYTE( "s5v0_4.rom", 0x600000, 0x100000, CRC(7e07a2fc) SHA1(f908363c968c15c0dc62e32695e5e2d0ca869391) )
	ROM_LOAD16_BYTE( "s6v0_4.rom", 0x800000, 0x100000, CRC(36ca43d3) SHA1(b599f88649c220143aa44cd5213e725e62afb0bc) )
ROM_END

ROM_START( cv_20h )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cv200h.rom", 0x000000, 0x100000, CRC(138a0c3c) SHA1(dd6d4b5519ca161bd6779ed60cc7f52542a10147) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "s2v1_0.rom", 0x000000, 0x080000, CRC(79dbb8ee) SHA1(f76c0db93b89beaf1e90c5f2199262e296fb1b78) )
	ROM_RELOAD(                    0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "s3v0_4.rom", 0x200000, 0x100000, CRC(8c6c0c56) SHA1(792431cc5b06c3d5028168297614f5eb7e8af34f) )
	ROM_LOAD16_BYTE( "s4v0_4.rom", 0x400000, 0x100000, CRC(a9014b78) SHA1(abffe32ab729fb39ab2360d850c8b5476094fd92) )
	ROM_LOAD16_BYTE( "s5v0_4.rom", 0x600000, 0x100000, CRC(7e07a2fc) SHA1(f908363c968c15c0dc62e32695e5e2d0ca869391) )
	ROM_LOAD16_BYTE( "s6v0_4.rom", 0x800000, 0x100000, CRC(36ca43d3) SHA1(b599f88649c220143aa44cd5213e725e62afb0bc) )
ROM_END

ROM_START( cv_10 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11_100.rom", 0x000000, 0x100000, CRC(00028589) SHA1(46639c45abbdc59ca0f861824eca3efa10547123) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "s2v1_0.rom", 0x000000, 0x080000, CRC(79dbb8ee) SHA1(f76c0db93b89beaf1e90c5f2199262e296fb1b78) )
	ROM_RELOAD(                    0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "s3v0_4.rom", 0x200000, 0x100000, CRC(8c6c0c56) SHA1(792431cc5b06c3d5028168297614f5eb7e8af34f) )
	ROM_LOAD16_BYTE( "s4v0_4.rom", 0x400000, 0x100000, CRC(a9014b78) SHA1(abffe32ab729fb39ab2360d850c8b5476094fd92) )
	ROM_LOAD16_BYTE( "s5v0_4.rom", 0x600000, 0x100000, CRC(7e07a2fc) SHA1(f908363c968c15c0dc62e32695e5e2d0ca869391) )
	ROM_LOAD16_BYTE( "s6v0_4.rom", 0x800000, 0x100000, CRC(36ca43d3) SHA1(b599f88649c220143aa44cd5213e725e62afb0bc) )
ROM_END

ROM_START( cv_11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11_110.rom", 0x000000, 0x100000, CRC(c7a4c104) SHA1(a96d34b2cf94591879de5b7838db0c98c9abfad8) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "s2v1_0.rom", 0x000000, 0x080000, CRC(79dbb8ee) SHA1(f76c0db93b89beaf1e90c5f2199262e296fb1b78) )
	ROM_RELOAD(                    0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "s3v0_4.rom", 0x200000, 0x100000, CRC(8c6c0c56) SHA1(792431cc5b06c3d5028168297614f5eb7e8af34f) )
	ROM_LOAD16_BYTE( "s4v0_4.rom", 0x400000, 0x100000, CRC(a9014b78) SHA1(abffe32ab729fb39ab2360d850c8b5476094fd92) )
	ROM_LOAD16_BYTE( "s5v0_4.rom", 0x600000, 0x100000, CRC(7e07a2fc) SHA1(f908363c968c15c0dc62e32695e5e2d0ca869391) )
	ROM_LOAD16_BYTE( "s6v0_4.rom", 0x800000, 0x100000, CRC(36ca43d3) SHA1(b599f88649c220143aa44cd5213e725e62afb0bc) )
ROM_END

ROM_START( cv_13 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cv_g11.1_3", 0x000000, 0x100000, CRC(58b3bea0) SHA1(243f15c6b383921faf735caece2073cb6f88601a) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD( "s2v1_0.rom",        0x000000, 0x080000, CRC(79dbb8ee) SHA1(f76c0db93b89beaf1e90c5f2199262e296fb1b78) )
	ROM_RELOAD(                    0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "s3v0_4.rom", 0x200000, 0x100000, CRC(8c6c0c56) SHA1(792431cc5b06c3d5028168297614f5eb7e8af34f) )
	ROM_LOAD16_BYTE( "s4v0_4.rom", 0x400000, 0x100000, CRC(a9014b78) SHA1(abffe32ab729fb39ab2360d850c8b5476094fd92) )
	ROM_LOAD16_BYTE( "s5v0_4.rom", 0x600000, 0x100000, CRC(7e07a2fc) SHA1(f908363c968c15c0dc62e32695e5e2d0ca869391) )
	ROM_LOAD16_BYTE( "s6v0_4.rom", 0x800000, 0x100000, CRC(36ca43d3) SHA1(b599f88649c220143aa44cd5213e725e62afb0bc) )
ROM_END

ROM_START( cv_d52 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cv_g11.d52", 0x000000, 0x100000, CRC(2b6b2822) SHA1(177ddd826b7dee060d090cd79f972836a23d6df9) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF ) // needs different audio ROMs, not dumped for now
	ROM_LOAD( "s2v1_0.rom",        0x000000, 0x080000, BAD_DUMP CRC(79dbb8ee) SHA1(f76c0db93b89beaf1e90c5f2199262e296fb1b78) )
	ROM_RELOAD(                    0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "s3v0_4.rom", 0x200000, 0x100000, BAD_DUMP CRC(8c6c0c56) SHA1(792431cc5b06c3d5028168297614f5eb7e8af34f) )
	ROM_LOAD16_BYTE( "s4v0_4.rom", 0x400000, 0x100000, BAD_DUMP CRC(a9014b78) SHA1(abffe32ab729fb39ab2360d850c8b5476094fd92) )
	ROM_LOAD16_BYTE( "s5v0_4.rom", 0x600000, 0x100000, BAD_DUMP CRC(7e07a2fc) SHA1(f908363c968c15c0dc62e32695e5e2d0ca869391) )
	ROM_LOAD16_BYTE( "s6v0_4.rom", 0x800000, 0x100000, BAD_DUMP CRC(36ca43d3) SHA1(b599f88649c220143aa44cd5213e725e62afb0bc) )
ROM_END

/*-----------------
/  Congo #50050
/------------------*/
ROM_START( congo_21 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cg_g11.2_1", 0x00000, 0x80000, CRC(5d8435bf) SHA1(1356758fd788bbb3c7ab29abaaea7d2baac75f55) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cgs2v1_1.rom", 0x000000, 0x100000, CRC(2b7637ae) SHA1(5b5d7214c632a506b986c892b39b1356b2909598) )
	ROM_LOAD16_BYTE( "cgs3v1_0.rom", 0x200000, 0x100000, CRC(6cfd9fe0) SHA1(a76267f865c645648c8cb27aec2d05062a4a20b5) )
	ROM_LOAD16_BYTE( "cgs4v1_0.rom", 0x400000, 0x100000, CRC(2a1980e7) SHA1(0badf27c2b8bc7b0074dc5e606d64490470bc108) )
ROM_END

ROM_START( congo_20 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cong2_00.rom", 0x00000, 0x80000, CRC(e1a256ac) SHA1(f1f7a1865b5a0220e2f2ef492059df158451ca5b) )
	ROM_RELOAD(               0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cgs2v1_1.rom", 0x000000, 0x100000, CRC(2b7637ae) SHA1(5b5d7214c632a506b986c892b39b1356b2909598) )
	ROM_LOAD16_BYTE( "cgs3v1_0.rom", 0x200000, 0x100000, CRC(6cfd9fe0) SHA1(a76267f865c645648c8cb27aec2d05062a4a20b5) )
	ROM_LOAD16_BYTE( "cgs4v1_0.rom", 0x400000, 0x100000, CRC(2a1980e7) SHA1(0badf27c2b8bc7b0074dc5e606d64490470bc108) )
ROM_END

ROM_START( congo_13 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cong1_30.rom", 0x00000, 0x80000, CRC(e68c0404) SHA1(e851f42e6bd0e910fc87b9500cbacac3c088b488) )
	ROM_RELOAD(               0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cgs2v1_1.rom", 0x000000, 0x100000, CRC(2b7637ae) SHA1(5b5d7214c632a506b986c892b39b1356b2909598) )
	ROM_LOAD16_BYTE( "cgs3v1_0.rom", 0x200000, 0x100000, CRC(6cfd9fe0) SHA1(a76267f865c645648c8cb27aec2d05062a4a20b5) )
	ROM_LOAD16_BYTE( "cgs4v1_0.rom", 0x400000, 0x100000, CRC(2a1980e7) SHA1(0badf27c2b8bc7b0074dc5e606d64490470bc108) )
ROM_END

ROM_START( congo_11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cong1_10.rom", 0x00000, 0x80000, CRC(b0b0ffd9) SHA1(26343f3bfbacf85b3f4db5aa3dad39216311a2da) )
	ROM_RELOAD(               0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cgs2v1_1.rom", 0x000000, 0x100000, CRC(2b7637ae) SHA1(5b5d7214c632a506b986c892b39b1356b2909598) )
	ROM_LOAD16_BYTE( "cgs3v1_0.rom", 0x200000, 0x100000, CRC(6cfd9fe0) SHA1(a76267f865c645648c8cb27aec2d05062a4a20b5) )
	ROM_LOAD16_BYTE( "cgs4v1_0.rom", 0x400000, 0x100000, CRC(2a1980e7) SHA1(0badf27c2b8bc7b0074dc5e606d64490470bc108) )
ROM_END

ROM_START( congo_11s10 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cong1_10.rom", 0x00000, 0x80000, CRC(b0b0ffd9) SHA1(26343f3bfbacf85b3f4db5aa3dad39216311a2da) )
	ROM_RELOAD(               0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "su2-100.rom", 0x000000, 0x80000, CRC(c4b59ac9) SHA1(a0bc5150120777c771a181496ced71bd3f92a311) )
	ROM_LOAD16_BYTE( "su3-100.rom", 0x200000, 0x80000, CRC(1d4dbc9a) SHA1(3fac6ffb1af806d1dfcf71d85b0be21e7ea4b8d2) )
	ROM_LOAD16_BYTE( "su4-100.rom", 0x400000, 0x80000, CRC(a3e9fd93) SHA1(7d767ddf22080f9886621a5130929d7afce90472) )
	ROM_LOAD16_BYTE( "su5-100.rom", 0x600000, 0x80000, CRC(c397b3f6) SHA1(ef4cc5a08a55ae941f42d2b02213cc5c85d67b43) )
	ROM_LOAD16_BYTE( "su6-100.rom", 0x800000, 0x80000, CRC(f89a29a2) SHA1(63f69ae6a886d9eac44627edd5ee561bdb3dd418) )
	ROM_LOAD16_BYTE( "su7-100.rom", 0xa00000, 0x80000, CRC(d1244d35) SHA1(7c5b3fcf8a35c417c778cd9bc741b92aaffeb444) )
ROM_END

/*------------------
/  Junk Yard #50052
/------------------*/
ROM_START( jy_12 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "jy_g11.1_2", 0x00000, 0x80000, CRC(16fb4bb3) SHA1(fbd8b37c129f7e07e8c44b7a33754ee346473377) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jy_s2.rom", 0x000000, 0x080000, CRC(1a1bc2ca) SHA1(db949d49560a26fc280cd9e746aa99dfafbd6daa) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "jy_s3.rom", 0x200000, 0x100000, CRC(0fc36a8e) SHA1(335013ebe08d34a24b0b472c6d5f042e455facee) )
	ROM_LOAD16_BYTE( "jy_s4.rom", 0x400000, 0x100000, CRC(0aebcd77) SHA1(62aee2685c0ae4bc1df8e4a4515ca34a078c72ad) )
	ROM_LOAD16_BYTE( "jy_s5.rom", 0x600000, 0x100000, CRC(f18ad10b) SHA1(1d02a388b43d3863030e01bf567f30337d37b2e8) )
ROM_END

ROM_START( jy_11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "jy_g11.1_1", 0x00000, 0x80000, CRC(2810fcb9) SHA1(58bb828e4d37a0ac65108a4dfb4ba25615b2b6f7) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jy_s2.rom", 0x000000, 0x080000, CRC(1a1bc2ca) SHA1(db949d49560a26fc280cd9e746aa99dfafbd6daa) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "jy_s3.rom", 0x200000, 0x100000, CRC(0fc36a8e) SHA1(335013ebe08d34a24b0b472c6d5f042e455facee) )
	ROM_LOAD16_BYTE( "jy_s4.rom", 0x400000, 0x100000, CRC(0aebcd77) SHA1(62aee2685c0ae4bc1df8e4a4515ca34a078c72ad) )
	ROM_LOAD16_BYTE( "jy_s5.rom", 0x600000, 0x100000, CRC(f18ad10b) SHA1(1d02a388b43d3863030e01bf567f30337d37b2e8) )
ROM_END

ROM_START( jy_03 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "jy_cpu.0_3", 0x00000, 0x80000, CRC(015d0253) SHA1(733740645fb300f48f57a74dea5fa31758628d24) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "jy_s2.0_2", 0x000000, 0x100000, CRC(8a6e0eaa) SHA1(758609b946d10dca70c46da0403d8ed36f8cbd5c) )
	ROM_LOAD16_BYTE( "jy_s3.0_2", 0x200000, 0x100000, CRC(53987d09) SHA1(c1f34d564a8f69413878a7adc089181e562a347c) )
	ROM_LOAD16_BYTE( "jy_s4.0_2", 0x400000, 0x100000, CRC(f82481cd) SHA1(a8283ac4a2dee636f4ec17d3fddf09920c7e3802) )
	ROM_LOAD16_BYTE( "jy_s5.0_2", 0x600000, 0x100000, CRC(5adb2d4c) SHA1(566a27238d643aaf7764e23bc1ce46cc5d7883dd) )
ROM_END

/*--------------------------
/  Medieval Madness #50059
/--------------------------*/
ROM_START( mm_109 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "mm_1_09.bin", 0x000000, 0x100000, CRC(9bac4d0c) SHA1(92cbe21802e1a77feff77b78f4dbbdbffb7b14bc) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mm_s2.1_0",   0x000000, 0x080000, CRC(c55c3b71) SHA1(95febbf16645dd897bdd459ccad9501d2457d1f1) )
	ROM_RELOAD(                     0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "mm_sav3.rom", 0x200000, 0x100000, CRC(ed1be570) SHA1(ead4c4f89d63ee0b46d8a8bcd8650d506542d1ee) )
	ROM_LOAD16_BYTE( "mm_sav4.rom", 0x400000, 0x100000, CRC(9c89eacf) SHA1(594a2aa81e34658862a9b7f0a83cf514182f2a2d) )
	ROM_LOAD16_BYTE( "mm_sav5.rom", 0x600000, 0x100000, CRC(45089e30) SHA1(e83492109c59e8a2f1ba9e1f793788b97d150a9b) )
	ROM_LOAD16_BYTE( "mm_sav6.rom", 0x800000, 0x100000, CRC(439d55f2) SHA1(d80e7268223157d864674261d140322634fb3bc2) )
ROM_END

ROM_START( mm_109b )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "mm_109b.bin", 0x000000, 0x100000, CRC(4eaab86a) SHA1(694cbb1154e7374275becfbe4f743fb8d31df8fb) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mm_s2.1_0",   0x000000, 0x080000, CRC(c55c3b71) SHA1(95febbf16645dd897bdd459ccad9501d2457d1f1) )
	ROM_RELOAD(                     0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "mm_sav3.rom", 0x200000, 0x100000, CRC(ed1be570) SHA1(ead4c4f89d63ee0b46d8a8bcd8650d506542d1ee) )
	ROM_LOAD16_BYTE( "mm_sav4.rom", 0x400000, 0x100000, CRC(9c89eacf) SHA1(594a2aa81e34658862a9b7f0a83cf514182f2a2d) )
	ROM_LOAD16_BYTE( "mm_sav5.rom", 0x600000, 0x100000, CRC(45089e30) SHA1(e83492109c59e8a2f1ba9e1f793788b97d150a9b) )
	ROM_LOAD16_BYTE( "mm_sav6.rom", 0x800000, 0x100000, CRC(439d55f2) SHA1(d80e7268223157d864674261d140322634fb3bc2) )
ROM_END

ROM_START( mm_109c )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "mm_1_09c.bin", 0x000000, 0x100000, CRC(d9e5189f) SHA1(fc01855c139d408559605fe9932236250cd566a8) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mm_s2.1_0",   0x000000, 0x080000, CRC(c55c3b71) SHA1(95febbf16645dd897bdd459ccad9501d2457d1f1) )
	ROM_RELOAD(                     0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "mm_sav3.rom", 0x200000, 0x100000, CRC(ed1be570) SHA1(ead4c4f89d63ee0b46d8a8bcd8650d506542d1ee) )
	ROM_LOAD16_BYTE( "mm_sav4.rom", 0x400000, 0x100000, CRC(9c89eacf) SHA1(594a2aa81e34658862a9b7f0a83cf514182f2a2d) )
	ROM_LOAD16_BYTE( "mm_sav5.rom", 0x600000, 0x100000, CRC(45089e30) SHA1(e83492109c59e8a2f1ba9e1f793788b97d150a9b) )
	ROM_LOAD16_BYTE( "mm_sav6.rom", 0x800000, 0x100000, CRC(439d55f2) SHA1(d80e7268223157d864674261d140322634fb3bc2) )
ROM_END

ROM_START( mm_10 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "mm_g11.1_0", 0x00000, 0x80000, CRC(6bd735c6) SHA1(3922df00e785610837230d5d9c24b9e082aa6fb6) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mm_s2.1_0",   0x000000, 0x080000, CRC(c55c3b71) SHA1(95febbf16645dd897bdd459ccad9501d2457d1f1) )
	ROM_RELOAD(                     0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "mm_sav3.rom", 0x200000, 0x100000, CRC(ed1be570) SHA1(ead4c4f89d63ee0b46d8a8bcd8650d506542d1ee) )
	ROM_LOAD16_BYTE( "mm_sav4.rom", 0x400000, 0x100000, CRC(9c89eacf) SHA1(594a2aa81e34658862a9b7f0a83cf514182f2a2d) )
	ROM_LOAD16_BYTE( "mm_sav5.rom", 0x600000, 0x100000, CRC(45089e30) SHA1(e83492109c59e8a2f1ba9e1f793788b97d150a9b) )
	ROM_LOAD16_BYTE( "mm_sav6.rom", 0x800000, 0x100000, CRC(439d55f2) SHA1(d80e7268223157d864674261d140322634fb3bc2) )
ROM_END

ROM_START( mm_10u )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "mmu_g11.1_0", 0x00000, 0x80000, CRC(265e6192) SHA1(bd9606df5fb85b2048a07db0927e4a856c344276) )
	ROM_RELOAD(              0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mm_s2.1_0",   0x000000, 0x080000, CRC(c55c3b71) SHA1(95febbf16645dd897bdd459ccad9501d2457d1f1) )
	ROM_RELOAD(                     0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "mm_sav3.rom", 0x200000, 0x100000, CRC(ed1be570) SHA1(ead4c4f89d63ee0b46d8a8bcd8650d506542d1ee) )
	ROM_LOAD16_BYTE( "mm_sav4.rom", 0x400000, 0x100000, CRC(9c89eacf) SHA1(594a2aa81e34658862a9b7f0a83cf514182f2a2d) )
	ROM_LOAD16_BYTE( "mm_sav5.rom", 0x600000, 0x100000, CRC(45089e30) SHA1(e83492109c59e8a2f1ba9e1f793788b97d150a9b) )
	ROM_LOAD16_BYTE( "mm_sav6.rom", 0x800000, 0x100000, CRC(439d55f2) SHA1(d80e7268223157d864674261d140322634fb3bc2) )
ROM_END

ROM_START( mm_05 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11-050.rom", 0x00000, 0x80000, CRC(d211ad16) SHA1(539fb0c4ca6fe19ac6140f5792c5b7cd51f737ce) )
	ROM_RELOAD(              0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "s2-020.rom",  0x000000, 0x080000, CRC(ee009ce4) SHA1(36843b2f1a07cf1e23bdff9b7347ceeca7e915bc) )
	ROM_RELOAD(                     0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "mm_sav3.rom", 0x200000, 0x100000, CRC(ed1be570) SHA1(ead4c4f89d63ee0b46d8a8bcd8650d506542d1ee) )
	ROM_LOAD16_BYTE( "mm_sav4.rom", 0x400000, 0x100000, CRC(9c89eacf) SHA1(594a2aa81e34658862a9b7f0a83cf514182f2a2d) )
	ROM_LOAD16_BYTE( "mm_sav5.rom", 0x600000, 0x100000, CRC(45089e30) SHA1(e83492109c59e8a2f1ba9e1f793788b97d150a9b) )
	ROM_LOAD16_BYTE( "mm_sav6.rom", 0x800000, 0x100000, CRC(439d55f2) SHA1(d80e7268223157d864674261d140322634fb3bc2) )
ROM_END

/*---------------------
/  Monster Bash #50065
/---------------------*/
ROM_START( mb_106 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "mb_1_06.bin", 0x000000, 0x100000, CRC(381a8822) SHA1(b0b5bf58accff24a4023c102952c89c1f116a174) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mb_s2.rom", 0x000000, 0x080000, CRC(9152f559) SHA1(68d455d8b875101caedafd21b59c447f326566dd) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "mb_s3.rom", 0x200000, 0x100000, CRC(c3cd6e81) SHA1(b041979c8955907090b30960780cecb19258bd5e) )
	ROM_LOAD16_BYTE( "mb_s4.rom", 0x400000, 0x100000, CRC(00b88352) SHA1(5da75e0b400eb71583681e06088eb97fc12e7f17) )
	ROM_LOAD16_BYTE( "mb_s5.rom", 0x600000, 0x100000, CRC(dae16105) SHA1(15878ef8685f3e9fc8eb2a2401581d30fe706e89) )
	ROM_LOAD16_BYTE( "mb_s6.rom", 0x800000, 0x100000, CRC(3975d5da) SHA1(6dbb34a827c0956e6aef1401c12cba88ae370e1f) )
	ROM_LOAD16_BYTE( "mb_s7.rom", 0xa00000, 0x100000, CRC(c242fb78) SHA1(c5a2a37ff3414d1e946cddb69b5e8f067b50bcc6) )
ROM_END

ROM_START( mb_106b )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "mb_106b.bin", 0x000000, 0x100000, CRC(c7c5d855) SHA1(96a43a955c0abaef8d6af1b64eaf50a7eeb69fe0) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mb_s2.rom", 0x000000, 0x080000, CRC(9152f559) SHA1(68d455d8b875101caedafd21b59c447f326566dd) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "mb_s3.rom", 0x200000, 0x100000, CRC(c3cd6e81) SHA1(b041979c8955907090b30960780cecb19258bd5e) )
	ROM_LOAD16_BYTE( "mb_s4.rom", 0x400000, 0x100000, CRC(00b88352) SHA1(5da75e0b400eb71583681e06088eb97fc12e7f17) )
	ROM_LOAD16_BYTE( "mb_s5.rom", 0x600000, 0x100000, CRC(dae16105) SHA1(15878ef8685f3e9fc8eb2a2401581d30fe706e89) )
	ROM_LOAD16_BYTE( "mb_s6.rom", 0x800000, 0x100000, CRC(3975d5da) SHA1(6dbb34a827c0956e6aef1401c12cba88ae370e1f) )
	ROM_LOAD16_BYTE( "mb_s7.rom", 0xa00000, 0x100000, CRC(c242fb78) SHA1(c5a2a37ff3414d1e946cddb69b5e8f067b50bcc6) )
ROM_END

ROM_START( mb_10 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "mb_g11.1_0", 0x000000, 0x100000, CRC(6b8db967) SHA1(e24d801ed9d326b9d4ddb26100c85cfd8e697d17) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mb_s2.rom", 0x000000, 0x080000, CRC(9152f559) SHA1(68d455d8b875101caedafd21b59c447f326566dd) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "mb_s3.rom", 0x200000, 0x100000, CRC(c3cd6e81) SHA1(b041979c8955907090b30960780cecb19258bd5e) )
	ROM_LOAD16_BYTE( "mb_s4.rom", 0x400000, 0x100000, CRC(00b88352) SHA1(5da75e0b400eb71583681e06088eb97fc12e7f17) )
	ROM_LOAD16_BYTE( "mb_s5.rom", 0x600000, 0x100000, CRC(dae16105) SHA1(15878ef8685f3e9fc8eb2a2401581d30fe706e89) )
	ROM_LOAD16_BYTE( "mb_s6.rom", 0x800000, 0x100000, CRC(3975d5da) SHA1(6dbb34a827c0956e6aef1401c12cba88ae370e1f) )
	ROM_LOAD16_BYTE( "mb_s7.rom", 0xa00000, 0x100000, CRC(c242fb78) SHA1(c5a2a37ff3414d1e946cddb69b5e8f067b50bcc6) )
ROM_END

/*----------------------
/  NBA Fastbreak #50053
/----------------------*/
ROM_START( nbaf_31 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "fb_g11.3_1", 0x00000, 0x80000, CRC(acd84ec2) SHA1(bd641b26e7a577be9f8705b21de4a694400945ce) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fb-s2.3_0", 0x000000, 0x100000, CRC(4594abd3) SHA1(d14654f0c2d29c28cae604e2dbcc9adf361b28a9) )
	ROM_LOAD16_BYTE( "fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648) )
	ROM_LOAD16_BYTE( "fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9) )
	ROM_LOAD16_BYTE( "fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d) )
	ROM_LOAD16_BYTE( "fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0) )
ROM_END

ROM_START( nbaf_31a )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "fb_g11.3_1", 0x00000, 0x80000, CRC(acd84ec2) SHA1(bd641b26e7a577be9f8705b21de4a694400945ce) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fb-s2.1_0", 0x000000, 0x080000, CRC(32f42a82) SHA1(387636c8e9f8525e7442ccdced735392db113044) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648) )
	ROM_LOAD16_BYTE( "fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9) )
	ROM_LOAD16_BYTE( "fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d) )
	ROM_LOAD16_BYTE( "fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0) )
ROM_END

ROM_START( nbaf_11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11-11.rom", 0x00000, 0x80000, CRC(debfb64a) SHA1(7f50246f5fde1e7fc295be6b6bbd455e244e4c99) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fb-s2.1_0", 0x000000, 0x080000, CRC(32f42a82) SHA1(387636c8e9f8525e7442ccdced735392db113044) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648) )
	ROM_LOAD16_BYTE( "fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9) )
	ROM_LOAD16_BYTE( "fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d) )
	ROM_LOAD16_BYTE( "fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0) )
ROM_END

ROM_START( nbaf_11a )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11-11.rom", 0x00000, 0x80000, CRC(debfb64a) SHA1(7f50246f5fde1e7fc295be6b6bbd455e244e4c99) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fb-s2.2_0", 0x000000, 0x100000, CRC(f950f481) SHA1(8d7c54c5f27a85889179ee690512fa69b1357bb6) )
	ROM_LOAD16_BYTE( "fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648) )
	ROM_LOAD16_BYTE( "fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9) )
	ROM_LOAD16_BYTE( "fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d) )
	ROM_LOAD16_BYTE( "fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0) )
ROM_END

ROM_START( nbaf_11s )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11-11.rom", 0x00000, 0x80000, CRC(debfb64a) SHA1(7f50246f5fde1e7fc295be6b6bbd455e244e4c99) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fb-s2.0_4", 0x000000, 0x080000, CRC(6a96f42b) SHA1(b6019bccdf62c9cf044a88d35019ebf0593b24d7) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648) )
	ROM_LOAD16_BYTE( "fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9) )
	ROM_LOAD16_BYTE( "fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d) )
	ROM_LOAD16_BYTE( "fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0) )
ROM_END

ROM_START( nbaf_115 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11-115", 0x00000, 0x80000, CRC(c0ed9848) SHA1(196d13cf93fe61db36d3bd936549210875a88948) )
	ROM_RELOAD(          0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fb-s2.1_0", 0x000000, 0x080000, CRC(32f42a82) SHA1(387636c8e9f8525e7442ccdced735392db113044) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648) )
	ROM_LOAD16_BYTE( "fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9) )
	ROM_LOAD16_BYTE( "fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d) )
	ROM_LOAD16_BYTE( "fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0) )
ROM_END

ROM_START( nbaf_21 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11-21.rom", 0x00000, 0x80000, CRC(598d33d0) SHA1(98c2bfcca573a6e790a4d3ba306953ff0fb3b042) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fb-s2.1_0", 0x000000, 0x080000, CRC(32f42a82) SHA1(387636c8e9f8525e7442ccdced735392db113044) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648) )
	ROM_LOAD16_BYTE( "fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9) )
	ROM_LOAD16_BYTE( "fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d) )
	ROM_LOAD16_BYTE( "fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0) )
ROM_END

ROM_START( nbaf_22 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11-22.rom", 0x00000, 0x80000, CRC(2e7a9685) SHA1(2af250a947089469c942cf2c570063bdebd4abe4) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fb-s2.1_0", 0x000000, 0x080000, CRC(32f42a82) SHA1(387636c8e9f8525e7442ccdced735392db113044) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648) )
	ROM_LOAD16_BYTE( "fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9) )
	ROM_LOAD16_BYTE( "fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d) )
	ROM_LOAD16_BYTE( "fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0) )
ROM_END

ROM_START( nbaf_23 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11-23.rom", 0x00000, 0x80000, CRC(a6ceb6de) SHA1(055387ee7da57e1a8fbce803a0dd9e67d6dbb1bd) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fb-s2.1_0", 0x000000, 0x080000, CRC(32f42a82) SHA1(387636c8e9f8525e7442ccdced735392db113044) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648) )
	ROM_LOAD16_BYTE( "fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9) )
	ROM_LOAD16_BYTE( "fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d) )
	ROM_LOAD16_BYTE( "fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0) )
ROM_END

/*-----------------------
/  No Good Gofers #50061
/-----------------------*/
ROM_START( ngg_13 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "go_g11.1_3", 0x00000, 0x80000, CRC(64e73117) SHA1(ce7ba5a6d309677e51dcbc9e3058f98e69d1e917) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nggsndl1.s2", 0x000000, 0x080000, CRC(6263866d) SHA1(c72a2f176aa24e91ecafe1704affd16b86d671e2) )
	ROM_RELOAD(                     0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "nggsndl1.s3", 0x200000, 0x100000, CRC(6871b20d) SHA1(0109c02282806016a6b22f7dfe3ac964931ba609) )
	ROM_LOAD16_BYTE( "nggsndl1.s4", 0x400000, 0x100000, CRC(86ed8f5a) SHA1(231f6313adff89ef4cec0d9f25b13e69ea96213d) )
	ROM_LOAD16_BYTE( "nggsndl1.s5", 0x600000, 0x100000, CRC(ea2062f0) SHA1(f8e45c1fcc6b8677a0745a5d83ca93b77fbde752) )
	ROM_LOAD16_BYTE( "nggsndl1.s6", 0x800000, 0x100000, CRC(b1b8b514) SHA1(e16651bcb2eae747987dc3c13a5dc20a33c0a1f8) )
ROM_END

ROM_START( ngg_10 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ngg_10.rom", 0x00000, 0x80000, CRC(6680f6c1) SHA1(8ac37e3ea427c998f84a0c9c55e3f1e1da395870) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "nggsndl1.s2", 0x000000, 0x080000, CRC(6263866d) SHA1(c72a2f176aa24e91ecafe1704affd16b86d671e2) )
	ROM_RELOAD(                     0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "nggsndl1.s3", 0x200000, 0x100000, CRC(6871b20d) SHA1(0109c02282806016a6b22f7dfe3ac964931ba609) )
	ROM_LOAD16_BYTE( "nggsndl1.s4", 0x400000, 0x100000, CRC(86ed8f5a) SHA1(231f6313adff89ef4cec0d9f25b13e69ea96213d) )
	ROM_LOAD16_BYTE( "nggsndl1.s5", 0x600000, 0x100000, CRC(ea2062f0) SHA1(f8e45c1fcc6b8677a0745a5d83ca93b77fbde752) )
	ROM_LOAD16_BYTE( "nggsndl1.s6", 0x800000, 0x100000, CRC(b1b8b514) SHA1(e16651bcb2eae747987dc3c13a5dc20a33c0a1f8) )
ROM_END

ROM_START( ngg_p06 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ngg0_6.rom", 0x00000, 0x80000, CRC(e0e0d331) SHA1(e1b91eccec6034bcd2029c15596aa0b129c9e53f) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ngg_s2.0_2",  0x000000, 0x080000, CRC(dde128d5) SHA1(214ee807d2323ecb407a3d116b038e15c60e5580) )
	ROM_RELOAD(                     0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "nggsndl1.s3", 0x200000, 0x100000, CRC(6871b20d) SHA1(0109c02282806016a6b22f7dfe3ac964931ba609) )
	ROM_LOAD16_BYTE( "nggsndl1.s4", 0x400000, 0x100000, CRC(86ed8f5a) SHA1(231f6313adff89ef4cec0d9f25b13e69ea96213d) )
	ROM_LOAD16_BYTE( "nggsndl1.s5", 0x600000, 0x100000, CRC(ea2062f0) SHA1(f8e45c1fcc6b8677a0745a5d83ca93b77fbde752) )
	ROM_LOAD16_BYTE( "nggsndl1.s6", 0x800000, 0x100000, CRC(b1b8b514) SHA1(e16651bcb2eae747987dc3c13a5dc20a33c0a1f8) )
ROM_END

/*-----------------------
/  Phantom Haus #?
/-----------------------*/
ROM_START( ph_04 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11_040.rom", 0x00000, 0x80000, CRC(8473f464) SHA1(829f2c0c772639e56f747d4274ad98967290bd43) )
	ROM_RELOAD(              0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "s2_030.rom",  0x000000, 0x080000, CRC(bbeb510c) SHA1(05088b6fa89cd203099189c31d132cd062ab8357) )
	ROM_RELOAD(                     0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "phs3_02.rom", 0x200000, 0x100000, CRC(710568a7) SHA1(8c7c28bacc2777722a54bee32375240c9a7441d8) )
	ROM_LOAD16_BYTE( "phs4_02.rom", 0x400000, 0x100000, CRC(390df0f2) SHA1(fdf1bb96e26c0245d04be610e1a84cd4e03059ff) )
	ROM_LOAD16_BYTE( "phs5_02.rom", 0x600000, 0x100000, CRC(b524513a) SHA1(5e372e409b4d1fc5bcda55af5dc2cbf1dc66e56b) )

	ROM_REGION( 0x2000, "serial_security_pic", 0 ) // provides game ID code and serial number
	ROM_LOAD( "901_phantom_haus_g10.g10", 0x0000, 0x2000, CRC(60697878) SHA1(84bc70b552e6c0f2f58e3a36050463ad3ebcd878) )
ROM_END

/*---------------------
/  Safe Cracker #90003
/---------------------*/
ROM_START( sc_18 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "safe_18g.rom", 0x00000, 0x80000, CRC(aeb4b669) SHA1(2925eb11133526ddff8ae92bb53f9b45c6ed8134) )
	ROM_RELOAD(               0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54) )
	ROM_LOAD16_BYTE( "safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731) )
	ROM_LOAD16_BYTE( "safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d) )
ROM_END

ROM_START( sc_18n )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "safe_18n.rom", 0x00000, 0x80000, CRC(4d5d5626) SHA1(2d6f201d47f24df2195f10267ec1426cf0a087c9) )
	ROM_RELOAD(               0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54) )
	ROM_LOAD16_BYTE( "safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731) )
	ROM_LOAD16_BYTE( "safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d) )
ROM_END

ROM_START( sc_18s2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "safe_18g.rom", 0x00000, 0x80000, CRC(aeb4b669) SHA1(2925eb11133526ddff8ae92bb53f9b45c6ed8134) )
	ROM_RELOAD(               0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "su2-24g.rom", 0x000000, 0x100000, CRC(712ce42e) SHA1(5d3b8e3eccdd1bc09a92de610161dd51293181b1) )
	ROM_LOAD16_BYTE( "safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731) )
	ROM_LOAD16_BYTE( "safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d) )
ROM_END

ROM_START( sc_17 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11-17g.rom", 0x00000, 0x80000, CRC(f3d64156) SHA1(9226664b59c7b65ac39e2f32597efc45672cf505) )
	ROM_RELOAD(              0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54) )
	ROM_LOAD16_BYTE( "safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731) )
	ROM_LOAD16_BYTE( "safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d) )
ROM_END

ROM_START( sc_17n )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11-17n.rom", 0x00000, 0x80000, CRC(97628907) SHA1(3435f496e1850bf433add1bc403e3148de05c13a) )
	ROM_RELOAD(              0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54) )
	ROM_LOAD16_BYTE( "safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731) )
	ROM_LOAD16_BYTE( "safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d) )
ROM_END

ROM_START( sc_14 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11-14.rom", 0x00000, 0x80000, CRC(1103f976) SHA1(6d6d23af1cd03f63b94a0ceb9711be51dce202f8) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54) )
	ROM_LOAD16_BYTE( "safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731) )
	ROM_LOAD16_BYTE( "safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d) )
ROM_END

ROM_START( sc_10 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11-10.rom", 0x00000, 0x80000, CRC(752a00f7) SHA1(86dbd0203f2a651382179f433fa49ca92d9828ae) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54) )
	ROM_LOAD16_BYTE( "safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731) )
	ROM_LOAD16_BYTE( "safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d) )
ROM_END

ROM_START( sc_091 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "sc_091.bin", 0x00000, 0x80000, CRC(b6f5307b) SHA1(93fab74db3aa62c2dd70d3a1d5664716c6548284) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54) )
	ROM_LOAD16_BYTE( "safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731) )
	ROM_LOAD16_BYTE( "safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d) )
ROM_END

/*---------------------
/  Scared Stiff #50048
/---------------------*/
ROM_START( ss_15 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ss_g11.1_5", 0x00000, 0x80000, CRC(5de8d0a0) SHA1(91cdd5f4e1654fd4dbde8b9cb03db935cba5d876) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sssnd_11.s2", 0x000000, 0x100000, CRC(1b080a72) SHA1(be80e99e6bcc482379fe99519a4122f3b225be30) )
	ROM_LOAD16_BYTE( "sssnd_11.s3", 0x200000, 0x100000, CRC(c4f2e08a) SHA1(e20ff622a3f475db11f1f44d36a6669e160437a3) )
	ROM_LOAD16_BYTE( "sssnd_11.s4", 0x400000, 0x100000, CRC(258b0a27) SHA1(83763b98907cf38e6f7b9fe4f26ce93a54ba3568) )
ROM_END

ROM_START( ss_14 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "stiffg11.1_4", 0x00000, 0x80000, CRC(17359ed6) SHA1(2ae549064a3666ea8b0b09aff9f5551db906d1d2) )
	ROM_RELOAD(               0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sssnd_11.s2", 0x000000, 0x100000, CRC(1b080a72) SHA1(be80e99e6bcc482379fe99519a4122f3b225be30) )
	ROM_LOAD16_BYTE( "sssnd_11.s3", 0x200000, 0x100000, CRC(c4f2e08a) SHA1(e20ff622a3f475db11f1f44d36a6669e160437a3) )
	ROM_LOAD16_BYTE( "sssnd_11.s4", 0x400000, 0x100000, CRC(258b0a27) SHA1(83763b98907cf38e6f7b9fe4f26ce93a54ba3568) )
ROM_END

ROM_START( ss_12 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "stiffg11.1_2", 0x00000, 0x80000, CRC(70eca59c) SHA1(07d50a32a4fb287780c4e6c1cb6fbeba97480219) )
	ROM_RELOAD(               0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sssnd_11.s2", 0x000000, 0x100000, CRC(1b080a72) SHA1(be80e99e6bcc482379fe99519a4122f3b225be30) )
	ROM_LOAD16_BYTE( "sssnd_11.s3", 0x200000, 0x100000, CRC(c4f2e08a) SHA1(e20ff622a3f475db11f1f44d36a6669e160437a3) )
	ROM_LOAD16_BYTE( "sssnd_11.s4", 0x400000, 0x100000, CRC(258b0a27) SHA1(83763b98907cf38e6f7b9fe4f26ce93a54ba3568) )
ROM_END

ROM_START( ss_11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "stiffg11.1_1", 0x00000, 0x80000, CRC(210615f4) SHA1(8f376c68d072f9b276d45452133107cdd0a3ea68) )
	ROM_RELOAD(               0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sssnd_10.s2", 0x000000, 0x100000, CRC(964974f1) SHA1(9b86d4c192eadbad85cf7676c74feed08b680581) )
	ROM_LOAD16_BYTE( "sssnd_10.s3", 0x200000, 0x100000, CRC(c4f2e08a) SHA1(e20ff622a3f475db11f1f44d36a6669e160437a3) )
	ROM_LOAD16_BYTE( "sssnd_10.s4", 0x400000, 0x100000, CRC(258b0a27) SHA1(83763b98907cf38e6f7b9fe4f26ce93a54ba3568) )
ROM_END

ROM_START( ss_03 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ss_g11.0_3", 0x00000, 0x80000, CRC(5b9755d6) SHA1(207d9ea858c76c4991747b401dc83183c1ddf7e4) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sssnd_s2.22", 0x000000, 0x100000, CRC(12b92d7a) SHA1(69151ffb5d2befe28e1ed2c8153c2227552b681c) )
	ROM_LOAD16_BYTE( "sssnd_s3.21", 0x200000, 0x100000, CRC(c4f2e08a) SHA1(e20ff622a3f475db11f1f44d36a6669e160437a3) )
	ROM_LOAD16_BYTE( "sssnd_s4.21", 0x400000, 0x100000, CRC(258b0a27) SHA1(83763b98907cf38e6f7b9fe4f26ce93a54ba3568) )
ROM_END

ROM_START( ss_01 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "ss_g11.rom", 0x00000, 0x80000, CRC(affd278f) SHA1(e6f41da169fa15c25cfaac22057f3e491da18fc5) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ss_s2.rom",   0x000000, 0x100000, CRC(ad079cbc) SHA1(77c7f676fc2f46e22b74b381638725269f7d23f4) )
	ROM_LOAD16_BYTE( "sssnd_s3.21", 0x200000, 0x100000, CRC(c4f2e08a) SHA1(e20ff622a3f475db11f1f44d36a6669e160437a3) )
	ROM_LOAD16_BYTE( "sssnd_s4.21", 0x400000, 0x100000, CRC(258b0a27) SHA1(83763b98907cf38e6f7b9fe4f26ce93a54ba3568) )
ROM_END

/*------------------------------------
/  Tales Of The Arabian Nights #50047
/------------------------------------*/
ROM_START( totan_14 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "an_g11.1_4", 0x00000, 0x80000, CRC(54db749e) SHA1(8f8b44febf3b672107e7715ec16e39dd91ee4cbb) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ans2v1_1.rom", 0x000000, 0x100000, CRC(0d023f90) SHA1(e411f7824df89374cf3385a2660d5bc91e0e9ef0) )
	ROM_LOAD16_BYTE( "ans3v1_0.rom", 0x200000, 0x100000, CRC(3f677813) SHA1(b1e67c74b927c0c8cb76be8794a04a53fdf643d4) )
	ROM_LOAD16_BYTE( "ans4v1_0.rom", 0x400000, 0x100000, CRC(c26dff5f) SHA1(d86323f0df15cf7abd4480d173e6b217ef715396) )
	ROM_LOAD16_BYTE( "ans5v1_0.rom", 0x600000, 0x080000, CRC(32ca1602) SHA1(e4c7235b5d387bdde16ebef4d3aeeb7276c69d6d) )
	ROM_RELOAD(                      0x680000, 0x080000 )
ROM_END

ROM_START( totan_13 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "arab1_3.rom", 0x00000, 0x80000, CRC(2e4b9439) SHA1(ba564c5984d3b68eaeba27d06f3acd95d26073ee) )
	ROM_RELOAD(              0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ans2v1_1.rom", 0x000000, 0x100000, CRC(0d023f90) SHA1(e411f7824df89374cf3385a2660d5bc91e0e9ef0) )
	ROM_LOAD16_BYTE( "ans3v1_0.rom", 0x200000, 0x100000, CRC(3f677813) SHA1(b1e67c74b927c0c8cb76be8794a04a53fdf643d4) )
	ROM_LOAD16_BYTE( "ans4v1_0.rom", 0x400000, 0x100000, CRC(c26dff5f) SHA1(d86323f0df15cf7abd4480d173e6b217ef715396) )
	ROM_LOAD16_BYTE( "ans5v1_0.rom", 0x600000, 0x080000, CRC(32ca1602) SHA1(e4c7235b5d387bdde16ebef4d3aeeb7276c69d6d) )
	ROM_RELOAD(                      0x680000, 0x080000 )
ROM_END

ROM_START( totan_12 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "arab1_2.rom", 0x00000, 0x80000, CRC(f9ae3796) SHA1(06e4ce89cab2e0fe5039de4261f7b5ebd4c11c0b) )
	ROM_RELOAD(              0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ans2v1_1.rom", 0x000000, 0x100000, CRC(0d023f90) SHA1(e411f7824df89374cf3385a2660d5bc91e0e9ef0) )
	ROM_LOAD16_BYTE( "ans3v1_0.rom", 0x200000, 0x100000, CRC(3f677813) SHA1(b1e67c74b927c0c8cb76be8794a04a53fdf643d4) )
	ROM_LOAD16_BYTE( "ans4v1_0.rom", 0x400000, 0x100000, CRC(c26dff5f) SHA1(d86323f0df15cf7abd4480d173e6b217ef715396) )
	ROM_LOAD16_BYTE( "ans5v1_0.rom", 0x600000, 0x080000, CRC(32ca1602) SHA1(e4c7235b5d387bdde16ebef4d3aeeb7276c69d6d) )
	ROM_RELOAD(                      0x680000, 0x080000 )
ROM_END

ROM_START( totan_04 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "an_g11.0_4", 0x00000, 0x80000, CRC(20da3800) SHA1(c8c048f35b1828f9ee1e7fc3201f1a316974b924) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ans2v1_0.rom", 0x000000, 0x100000, CRC(1b78fe52) SHA1(2ad394c2d0f05eac3c32c9957f327d680a734451) )
	ROM_LOAD16_BYTE( "ans3v1_0.rom", 0x200000, 0x100000, CRC(3f677813) SHA1(b1e67c74b927c0c8cb76be8794a04a53fdf643d4) )
	ROM_LOAD16_BYTE( "ans4v1_0.rom", 0x400000, 0x100000, CRC(c26dff5f) SHA1(d86323f0df15cf7abd4480d173e6b217ef715396) )
	ROM_LOAD16_BYTE( "ans5v1_0.rom", 0x600000, 0x080000, CRC(32ca1602) SHA1(e4c7235b5d387bdde16ebef4d3aeeb7276c69d6d) )
	ROM_RELOAD(                      0x680000, 0x080000 )
ROM_END

/*--------------------------
/  The Champion Pub #50063
/--------------------------*/
ROM_START( cp_16 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cp_g11.1_6", 0x00000, 0x80000, CRC(d6d0b921) SHA1(6784bd5116d239f307310d4a1ddac1068292dd60) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp_s2.bin", 0x000000, 0x080000, CRC(e0b67f6f) SHA1(48fbf01eca4fd6250df18bb5f35959100f40f6e0) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "cp_s3.bin", 0x200000, 0x100000, CRC(68accf24) SHA1(9f86ac84ef8130592e471f1da0e05ba811dbc38b) )
	ROM_LOAD16_BYTE( "cp_s4.bin", 0x400000, 0x100000, CRC(50d1c920) SHA1(00b247853ef1f91c6245746c9311f8463b9335d1) )
	ROM_LOAD16_BYTE( "cp_s5.bin", 0x600000, 0x100000, CRC(69af347a) SHA1(d15683e6297603104e4ba777224331c24565be7c) )
	ROM_LOAD16_BYTE( "cp_s6.bin", 0x800000, 0x100000, CRC(76ca4fed) SHA1(8995e518c8dafbdd8bf994533b71f42172057b27) )
	ROM_LOAD16_BYTE( "cp_s7.bin", 0xa00000, 0x100000, CRC(be619157) SHA1(b18acde4f683b5f8b2248b46bb3dc7c3e0ab1c26) )
ROM_END

ROM_START( cp_15 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "cp_g11.1_5", 0x00000, 0x80000, CRC(4255bfcb) SHA1(4ec17e6c0e07fd8d52af9d33776007930d8422c6) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp_s2.bin", 0x000000, 0x080000, CRC(e0b67f6f) SHA1(48fbf01eca4fd6250df18bb5f35959100f40f6e0) )
	ROM_RELOAD(                   0x100000, 0x080000 )
	ROM_LOAD16_BYTE( "cp_s3.bin", 0x200000, 0x100000, CRC(68accf24) SHA1(9f86ac84ef8130592e471f1da0e05ba811dbc38b) )
	ROM_LOAD16_BYTE( "cp_s4.bin", 0x400000, 0x100000, CRC(50d1c920) SHA1(00b247853ef1f91c6245746c9311f8463b9335d1) )
	ROM_LOAD16_BYTE( "cp_s5.bin", 0x600000, 0x100000, CRC(69af347a) SHA1(d15683e6297603104e4ba777224331c24565be7c) )
	ROM_LOAD16_BYTE( "cp_s6.bin", 0x800000, 0x100000, CRC(76ca4fed) SHA1(8995e518c8dafbdd8bf994533b71f42172057b27) )
	ROM_LOAD16_BYTE( "cp_s7.bin", 0xa00000, 0x100000, CRC(be619157) SHA1(b18acde4f683b5f8b2248b46bb3dc7c3e0ab1c26) )
ROM_END

/*-----------------------
/ Ticket Tac Toe #90005
/-----------------------*/
ROM_START( ttt_10 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tikt1_0.rom", 0x00000, 0x80000, CRC(bf1d0382) SHA1(3d26413400915594e9f1cc08a551c05526b94223) )
	ROM_RELOAD(              0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "ttt_s2.rom", 0x000000, 0x100000, CRC(faae93eb) SHA1(672758544b260d7751ac296f5beb2e271e77c50a) )
	ROM_LOAD16_BYTE( "ttt_s3.rom", 0x200000, 0x100000, CRC(371ba9b3) SHA1(de6a8cb78e08a434f6668dd4a93cad857acba310) )
ROM_END

/*----------------------------
/ Test Fixture WPC95 (#60048)
/----------------------------*/
ROM_START( tf95_12 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "g11_12.rom", 0x00000, 0x80000, CRC(259a2b23) SHA1(16f8c15e046809e0b1587b0c981d36f4d8a750ca) )
	ROM_RELOAD(             0x80000, 0x80000 )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "s2_10.rom", 0x000000, 0x100000, CRC(ceff7fe4) SHA1(ff2574f65e09d446b9e446abd58159a7d100059b) )
ROM_END

} // anonymous namespace


GAME(1996,  tf95_12,    0,          tf95,  afm,    wpc_95_state,   empty_init, ROT0, "Bally",                "WPC 95 Test Fixture (1.2)",              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1995,  afm_113,    0,          afm,   afm,    afm_state,      empty_init, ROT0, "Bally",                "Attack From Mars (1.13, Free play)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1995,  afm_113b,   afm_113,    afm,   afm,    afm_state,      empty_init, ROT0, "Bally",                "Attack From Mars (1.13b)",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1995,  afm_11,     afm_113,    afm,   afm,    afm_state,      empty_init, ROT0, "Bally",                "Attack From Mars (1.1)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1995,  afm_11u,    afm_113,    afm,   afm,    afm_state,      empty_init, ROT0, "Bally",                "Attack From Mars (1.1 Ultrapin)",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1995,  afm_10,     afm_113,    afm,   afm,    afm_state,      empty_init, ROT0, "Bally",                "Attack From Mars (1.0)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1998,  cc_13,      0,          cc,    cc,     wpc_95_state,   empty_init, ROT0, "Bally",                "Cactus Canyon (1.3)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1998,  cc_12,      cc_13,      cc,    cc,     wpc_95_state,   empty_init, ROT0, "Bally",                "Cactus Canyon (1.2)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1998,  cc_10,      cc_13,      cc,    cc,     wpc_95_state,   empty_init, ROT0, "Bally",                "Cactus Canyon (1.0)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1998,  cc_104,     cc_13,      cc,    cc,     wpc_95_state,   empty_init, ROT0, "Bally",                "Cactus Canyon (1.04 Test 0.2)",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  cv_14,      0,          cv,    cv,     wpc_95_state,   empty_init, ROT0, "Bally",                "Cirqus Voltaire (1.4)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  cv_20h,     cv_14,      cv,    cv,     wpc_95_state,   empty_init, ROT0, "Bally",                "Cirqus Voltaire (2.0H)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  cv_10,      cv_14,      cv,    cv,     wpc_95_state,   empty_init, ROT0, "Bally",                "Cirqus Voltaire (1.0)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  cv_11,      cv_14,      cv,    cv,     wpc_95_state,   empty_init, ROT0, "Bally",                "Cirqus Voltaire (1.1)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  cv_13,      cv_14,      cv,    cv,     wpc_95_state,   empty_init, ROT0, "Bally",                "Cirqus Voltaire (1.3)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  cv_d52,     cv_14,      cv,    cv,     wpc_95_state,   empty_init, ROT0, "Bally",                "Cirqus Voltaire (D.52 prototype)",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK ) // needs different audio ROMs
GAME(1995,  congo_21,   0,          congo, congo,  wpc_95_state,   empty_init, ROT0, "Williams",             "Congo (2.1)",                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1995,  congo_20,   congo_21,   congo, congo,  wpc_95_state,   empty_init, ROT0, "Williams",             "Congo (2.0)",                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1995,  congo_13,   congo_21,   congo, congo,  wpc_95_state,   empty_init, ROT0, "Williams",             "Congo (1.3)",                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1995,  congo_11,   congo_21,   congo, congo,  wpc_95_state,   empty_init, ROT0, "Williams",             "Congo (1.1)",                            MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1995,  congo_11s10,congo_21,   congo, congo,  wpc_95_state,   empty_init, ROT0, "Williams",             "Congo (1.1, DCS sound 1.0)",             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  jy_12,      0,          jy,    jy,     wpc_95_state,   empty_init, ROT0, "Williams",             "Junk Yard (1.2)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  jy_11,      jy_12,      jy,    jy,     wpc_95_state,   empty_init, ROT0, "Williams",             "Junk Yard (1.1)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  jy_03,      jy_12,      jy,    jy,     wpc_95_state,   empty_init, ROT0, "Williams",             "Junk Yard (0.3)",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1999,  mm_10,      0,          mm,    mm,     wpc_95_state,   empty_init, ROT0, "Williams",             "Medieval Madness (1.0)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1999,  mm_10u,     mm_10,      mm,    mm,     wpc_95_state,   empty_init, ROT0, "Williams",             "Medieval Madness (1.0 Ultrapin)",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1999,  mm_109,     mm_10,      mm,    mm,     wpc_95_state,   empty_init, ROT0, "Williams",             "Medieval Madness (1.09)",                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1999,  mm_109b,    mm_10,      mm,    mm,     wpc_95_state,   empty_init, ROT0, "Williams",             "Medieval Madness (1.09B)",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1999,  mm_109c,    mm_10,      mm,    mm,     wpc_95_state,   empty_init, ROT0, "Williams",             "Medieval Madness (1.09C Profanity)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  mm_05,      mm_10,      mm,    mm,     wpc_95_state,   empty_init, ROT0, "Williams",             "Medieval Madness (0.50)",                MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1998,  mb_10,      0,          mb,    mb,     wpc_95_state,   empty_init, ROT0, "Williams",             "Monster Bash (1.0)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1998,  mb_106,     mb_10,      mb,    mb,     wpc_95_state,   empty_init, ROT0, "Williams",             "Monster Bash (1.06)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1998,  mb_106b,    mb_10,      mb,    mb,     wpc_95_state,   empty_init, ROT0, "Williams",             "Monster Bash (1.06b)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  nbaf_31,    0,          nbaf,  nbaf,   wpc_95_state,   empty_init, ROT0, "Bally",                "NBA Fastbreak (3.1 - S3.0)",             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  nbaf_31a,   nbaf_31,    nbaf,  nbaf,   wpc_95_state,   empty_init, ROT0, "Bally",                "NBA Fastbreak (3.1 - S1.0)",             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  nbaf_11s,   nbaf_31,    nbaf,  nbaf,   wpc_95_state,   empty_init, ROT0, "Bally",                "NBA Fastbreak (1.1 - S0.4)",             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  nbaf_11,    nbaf_31,    nbaf,  nbaf,   wpc_95_state,   empty_init, ROT0, "Bally",                "NBA Fastbreak (1.1)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  nbaf_11a,   nbaf_31,    nbaf,  nbaf,   wpc_95_state,   empty_init, ROT0, "Bally",                "NBA Fastbreak (1.1 - S2.0)",             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  nbaf_115,   nbaf_31,    nbaf,  nbaf,   wpc_95_state,   empty_init, ROT0, "Bally",                "NBA Fastbreak (1.15)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  nbaf_21,    nbaf_31,    nbaf,  nbaf,   wpc_95_state,   empty_init, ROT0, "Bally",                "NBA Fastbreak (2.1)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  nbaf_22,    nbaf_31,    nbaf,  nbaf,   wpc_95_state,   empty_init, ROT0, "Bally",                "NBA Fastbreak (2.2)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  nbaf_23,    nbaf_31,    nbaf,  nbaf,   wpc_95_state,   empty_init, ROT0, "Bally",                "NBA Fastbreak (2.3)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  ngg_13,     0,          ngg,   ngg,    wpc_95_state,   empty_init, ROT0, "Williams",             "No Good Gofers (1.3)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  ngg_p06,    ngg_13,     ngg,   ngg,    wpc_95_state,   empty_init, ROT0, "Williams",             "No Good Gofers (p0.6)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1997,  ngg_10,     ngg_13,     ngg,   ngg,    wpc_95_state,   empty_init, ROT0, "Williams",             "No Good Gofers (1.0)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  ph_04,      0,          ph,    ngg,    wpc_95_state,   empty_init, ROT0, "Williams",             "Phantom Haus (0.4)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1998,  sc_18,      0,          sc,    sc,     sc_state,       empty_init, ROT0, "Bally",                "Safe Cracker (1.8)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1998,  sc_18n,     sc_18,      sc,    sc,     sc_state,       empty_init, ROT0, "Bally",                "Safe Cracker (1.8N)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1998,  sc_18s2,    sc_18,      sc,    sc,     sc_state,       empty_init, ROT0, "Bally",                "Safe Cracker (1.8 German sound)",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  sc_17,      sc_18,      sc,    sc,     sc_state,       empty_init, ROT0, "Bally",                "Safe Cracker (1.7)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  sc_17n,     sc_18,      sc,    sc,     sc_state,       empty_init, ROT0, "Bally",                "Safe Cracker (1.7N)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  sc_14,      sc_18,      sc,    sc,     sc_state,       empty_init, ROT0, "Bally",                "Safe Cracker (1.4)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  sc_10,      sc_18,      sc,    sc,     sc_state,       empty_init, ROT0, "Bally",                "Safe Cracker (1.0)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  sc_091,     sc_18,      sc,    sc,     sc_state,       empty_init, ROT0, "Bally",                "Safe Cracker (0.91)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  ss_15,      0,          ss,    ss,     wpc_95_state,   empty_init, ROT0, "Bally",                "Scared Stiff (1.5)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  ss_14,      ss_15,      ss,    ss,     wpc_95_state,   empty_init, ROT0, "Bally",                "Scared Stiff (1.4)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  ss_12,      ss_15,      ss,    ss,     wpc_95_state,   empty_init, ROT0, "Bally",                "Scared Stiff (1.2)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  ss_11,      ss_15,      ss,    ss,     wpc_95_state,   empty_init, ROT0, "Bally",                "Scared Stiff (1.1)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  ss_03,      ss_15,      ss,    ss,     wpc_95_state,   empty_init, ROT0, "Bally",                "Scared Stiff (0.3)",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  ss_01,      ss_15,      ss,    ss,     wpc_95_state,   empty_init, ROT0, "Bally",                "Scared Stiff (D0.1R with sound rev.25)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  totan_14,   0,          totan, totan,  wpc_95_state,   empty_init, ROT0, "Williams",             "Tales Of The Arabian Nights (1.4)",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  totan_13,   totan_14,   totan, totan,  wpc_95_state,   empty_init, ROT0, "Williams",             "Tales Of The Arabian Nights (1.3)",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  totan_12,   totan_14,   totan, totan,  wpc_95_state,   empty_init, ROT0, "Williams",             "Tales Of The Arabian Nights (1.2)",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  totan_04,   totan_14,   totan, totan,  wpc_95_state,   empty_init, ROT0, "Williams",             "Tales Of The Arabian Nights (0.4)",      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1998,  cp_16,      0,          cp,    cp,     wpc_95_state,   empty_init, ROT0, "Bally",                "The Champion Pub (1.6)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1998,  cp_15,      cp_16,      cp,    cp,     wpc_95_state,   empty_init, ROT0, "Bally",                "The Champion Pub (1.5)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME(1996,  ttt_10,     0,          ttt,   ttt,    wpc_95_state,   empty_init, ROT0, "Williams",             "Ticket Tac Toe (1.0)",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
