// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Miodrag Milanovic

// Thanks to the PinMAME guys for all their research

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "audio/dcs.h"
#include "machine/nvram.h"
#include "video/wpc_dmd.h"
#include "machine/wpc_pic.h"
#include "machine/wpc_shift.h"
#include "machine/wpc_lamp.h"
#include "machine/wpc_out.h"

class wpc_95_state : public driver_device
{
public:
	wpc_95_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			maincpu(*this, "maincpu"),
			dcs(*this, "dcs"),
			rombank(*this, "rombank"),
			mainram(*this, "mainram"),
			nvram(*this, "nvram"),
			pic(*this, "pic"),
			lamp(*this, "lamp"),
			out(*this, "out")
	{ }

	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(watchdog_w);
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(firq_src_r);
	DECLARE_READ8_MEMBER(zc_r);
	DECLARE_READ8_MEMBER(dcs_data_r);
	DECLARE_WRITE8_MEMBER(dcs_data_w);
	DECLARE_READ8_MEMBER(dcs_ctrl_r);
	DECLARE_WRITE8_MEMBER(dcs_reset_w);
	DECLARE_READ8_MEMBER(rtc_r);

	void init();
	DECLARE_DRIVER_INIT(tf95);
	DECLARE_DRIVER_INIT(afm);
	DECLARE_DRIVER_INIT(cc);
	DECLARE_DRIVER_INIT(cv);
	DECLARE_DRIVER_INIT(congo);
	DECLARE_DRIVER_INIT(jy);
	DECLARE_DRIVER_INIT(mm);
	DECLARE_DRIVER_INIT(mb);
	DECLARE_DRIVER_INIT(nbaf);
	DECLARE_DRIVER_INIT(ngg);
	DECLARE_DRIVER_INIT(sc);
	DECLARE_DRIVER_INIT(ss);
	DECLARE_DRIVER_INIT(totan);
	DECLARE_DRIVER_INIT(cp);
	DECLARE_DRIVER_INIT(ttt);

	DECLARE_WRITE_LINE_MEMBER(scanline_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(zc_timer);

protected:
	// devices
	required_device<cpu_device> maincpu;
	required_device<dcs_audio_wpc_device> dcs;
	required_memory_bank rombank;
	required_shared_ptr<UINT8> mainram;
	required_device<nvram_device> nvram;
	required_device<wpc_pic_device> pic;
	required_device<wpc_lamp_device> lamp;
	required_device<wpc_out_device> out;

	// driver_device overrides
	virtual void machine_reset();

private:
	static const char *const lamps_afm[64];
	static const char *const outputs_afm[52];
	static const char *const lamps_cc[64];
	static const char *const outputs_cc[52];
	static const char *const lamps_cv[64];
	static const char *const outputs_cv[52];
	static const char *const lamps_congo[64];
	static const char *const outputs_congo[52];
	static const char *const lamps_jy[64];
	static const char *const outputs_jy[52];
	static const char *const lamps_mm[64];
	static const char *const outputs_mm[52];
	static const char *const lamps_mb[64];
	static const char *const outputs_mb[52];
	static const char *const lamps_nbaf[64];
	static const char *const outputs_nbaf[52];
	static const char *const lamps_ngg[64];
	static const char *const outputs_ngg[52];
	static const char *const lamps_sc[64];
	static const char *const lamps_sc_extra[48];
	static const char *const outputs_sc[52];
	static const char *const lamps_ss[64];
	static const char *const outputs_ss[52];
	static const char *const lamps_totan[64];
	static const char *const outputs_totan[52];
	static const char *const lamps_cp[64];
	static const char *const outputs_cp[52];
	static const char *const lamps_ttt[64];
	static const char *const outputs_ttt[52];

	UINT8 firq_src, zc;
	UINT16 rtc_base_day;

	bool serial_clock_state, serial_data1_state, serial_data2_state, serial_enable;
	int serial_clock_counter;
	UINT32 serial_out1_state, serial_out2_state;

	bool afm_led_handler(int sid, bool state);
	static void sc_aux_lamps_handler_update(UINT32 &out, UINT32 mask, bool state, int id);
	bool sc_aux_lamps_handler(int sid, bool state);
};

static ADDRESS_MAP_START( wpc_95_map, AS_PROGRAM, 8, wpc_95_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("mainram")

	AM_RANGE(0x3000, 0x31ff) AM_RAMBANK("dmd0")
	AM_RANGE(0x3200, 0x33ff) AM_RAMBANK("dmd2")
	AM_RANGE(0x3400, 0x35ff) AM_RAMBANK("dmd4")
	AM_RANGE(0x3600, 0x37ff) AM_RAMBANK("dmd6")
	AM_RANGE(0x3800, 0x39ff) AM_RAMBANK("dmd8")
	AM_RANGE(0x3a00, 0x3bff) AM_RAMBANK("dmda")

	AM_RANGE(0x3fb8, 0x3fbf) AM_DEVICE("dmd", wpc_dmd_device, registers)

	AM_RANGE(0x3fdc, 0x3fdc) AM_READWRITE(dcs_data_r, dcs_data_w)
	AM_RANGE(0x3fdd, 0x3fdd) AM_READWRITE(dcs_ctrl_r, dcs_reset_w)

	AM_RANGE(0x3fe0, 0x3fe3) AM_DEVWRITE("out", wpc_out_device, out_w)
	AM_RANGE(0x3fe4, 0x3fe4) AM_READNOP AM_DEVWRITE("lamp", wpc_lamp_device, row_w)
	AM_RANGE(0x3fe5, 0x3fe5) AM_READNOP AM_DEVWRITE("lamp", wpc_lamp_device, col_w)
	AM_RANGE(0x3fe6, 0x3fe6) AM_DEVWRITE("out", wpc_out_device, gi_w)
	AM_RANGE(0x3fe7, 0x3fe7) AM_READ_PORT("DSW")
	AM_RANGE(0x3fe8, 0x3fe8) AM_READ_PORT("DOOR")
	AM_RANGE(0x3fe9, 0x3fe9) AM_DEVREAD("pic", wpc_pic_device, read)
	AM_RANGE(0x3fea, 0x3fea) AM_DEVWRITE("pic", wpc_pic_device, write)

	AM_RANGE(0x3fee, 0x3fee) AM_DEVWRITE("out", wpc_out_device, out4_w)
	AM_RANGE(0x3fef, 0x3fef) AM_READ_PORT("FLIPPERS")

	AM_RANGE(0x3ff2, 0x3ff2) AM_DEVWRITE("out", wpc_out_device, led_w)
	AM_RANGE(0x3ff3, 0x3ff3) AM_READNOP AM_WRITE(irq_ack_w)
	AM_RANGE(0x3ff4, 0x3ff7) AM_DEVICE("shift", wpc_shift_device, registers)
	AM_RANGE(0x3ff8, 0x3ff8) AM_READ(firq_src_r) AM_WRITENOP // ack?
	AM_RANGE(0x3ffa, 0x3ffb) AM_READ(rtc_r)
	AM_RANGE(0x3ffc, 0x3ffc) AM_WRITE(bank_w)
	AM_RANGE(0x3ffd, 0x3ffe) AM_NOP // memory protection stuff?
	AM_RANGE(0x3fff, 0x3fff) AM_READWRITE(zc_r, watchdog_w)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("rombank")
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("maincpu", 0xf8000)
ADDRESS_MAP_END

READ8_MEMBER(wpc_95_state::dcs_data_r)
{
	return dcs->data_r();
}

WRITE8_MEMBER(wpc_95_state::dcs_data_w)
{
	dcs->data_w(data);
}

READ8_MEMBER(wpc_95_state::dcs_ctrl_r)
{
	return dcs->control_r();
}

WRITE8_MEMBER(wpc_95_state::dcs_reset_w)
{
	dcs->reset_w(0);
	dcs->reset_w(1);
}

READ8_MEMBER(wpc_95_state::rtc_r)
{
	system_time systime;
	machine().base_datetime(systime);

	// This may get wonky if the game is running on year change.  Find
	// something better to do at that time.

	UINT8 day = (systime.local_time.day - rtc_base_day) & 31;
	UINT8 hour = systime.local_time.hour;
	UINT8 min = systime.local_time.minute;

	switch(offset) {
	case 0:
		return ((day & 7) << 5) | hour;
	case 1:
		return ((day & 0x18) << 3) | min;
	default:
		return 0xff;
	}
}

READ8_MEMBER(wpc_95_state::firq_src_r)
{
	return firq_src;
}

READ8_MEMBER(wpc_95_state::zc_r)
{
	UINT8 res = zc;
	zc &= 0x7f;
	return res;
}

TIMER_DEVICE_CALLBACK_MEMBER(wpc_95_state::zc_timer)
{
	zc |= 0x80;
}

WRITE8_MEMBER(wpc_95_state::bank_w)
{
	rombank->set_entry(data & 0x3f);
}

WRITE8_MEMBER(wpc_95_state::watchdog_w)
{
}

WRITE_LINE_MEMBER(wpc_95_state::scanline_irq)
{
	firq_src = 0x00;
	maincpu->set_input_line(1, state);
}

WRITE8_MEMBER(wpc_95_state::irq_ack_w)
{
	maincpu->set_input_line(0, CLEAR_LINE);
	maincpu->set_input_line(1, CLEAR_LINE);
}

void wpc_95_state::machine_reset()
{
	firq_src = 0x00;
	zc = 0x00;

	/* The hardware seems to only have a minute/hour/day counter.  It
	   keeps the current day in nvram, and as long as you start the
	   machine at least once every 32 days (the day counter is 5 bits)
	   it updates it correctly.

	   So setup the correct memory zone to avoid the system bitching,
	   and requiring the user to fix it.
	*/
	system_time systime;
	machine().base_datetime(systime);
	mainram[0x1800] = systime.local_time.year >> 8;
	mainram[0x1801] = systime.local_time.year;
	mainram[0x1802] = systime.local_time.month+1;
	mainram[0x1803] = systime.local_time.mday;
	mainram[0x1804] = systime.local_time.weekday+1;
	mainram[0x1805] = 0;
	mainram[0x1806] = 1;
	UINT16 checksum = 0;
	for(int i=0x1800; i<=0x1806; i++)
		checksum += mainram[i];
	checksum = ~checksum;
	mainram[0x1807] = checksum >> 8;
	mainram[0x1808] = checksum;
	rtc_base_day = systime.local_time.day;

	serial_clock_state = serial_data1_state = serial_data2_state = false;
	serial_clock_counter = 0;
	serial_enable = false;
	serial_out1_state = 0;
	serial_out2_state = 0;
}

void wpc_95_state::init()
{
	rombank->configure_entries(0, 0x40, memregion("maincpu")->base(), 0x4000);
	nvram->set_base(mainram, mainram.bytes());

	save_item(NAME(firq_src));
	save_item(NAME(zc));
	save_item(NAME(serial_clock_state));
	save_item(NAME(serial_data1_state));
	save_item(NAME(serial_data2_state));
	save_item(NAME(serial_enable));
	save_item(NAME(serial_clock_counter));
	save_item(NAME(serial_out1_state));
	save_item(NAME(serial_out2_state));

	// rtc_base_day not saved to give the system a better chance to
	// survive reload some days after unscathed.
}

bool wpc_95_state::afm_led_handler(int sid, bool state)
{
	switch(sid) {
	case 37:
		if(!serial_clock_state && state) {
			UINT16 mask = 1 << serial_clock_counter;
			bool prev_state = serial_out1_state & mask;
			if(prev_state != serial_data1_state) {
				char buffer[32];
				sprintf(buffer, "l:Saucer led %d", serial_clock_counter);
				output_set_value(buffer, serial_data1_state);
				if(serial_data1_state)
					serial_out1_state |= mask;
				else
					serial_out1_state &= ~mask;
			}
			serial_clock_counter = (serial_clock_counter+1) & 15;
		}
		serial_clock_state = state;
		return true;

	case 38:
		serial_data1_state = state;
		return true;
	}
	return false;
}

void wpc_95_state::sc_aux_lamps_handler_update(UINT32 &out, UINT32 mask, bool state, int id)
{
	bool prev_state = out & mask;
	if(prev_state != state) {
		output_set_value(lamps_sc_extra[id], state);
		if(state)
			out |= mask;
		else
			out &= ~mask;
	}
}

bool wpc_95_state::sc_aux_lamps_handler(int sid, bool state)
{
	switch(sid) {
	case 37:
		if(serial_enable && !state)
			serial_clock_counter = 0;

		serial_enable = state;
		return true;

	case 38:
		if(!serial_clock_state && state && !serial_enable) {
			UINT32 mask = 1 << serial_clock_counter;
			sc_aux_lamps_handler_update(serial_out1_state, mask, serial_data1_state, serial_clock_counter);
			sc_aux_lamps_handler_update(serial_out2_state, mask, serial_data2_state, serial_clock_counter+24);
			serial_clock_counter++;
		}
		serial_clock_state = state;
		return true;

	case 39:
		serial_data1_state = state;
		return true;

	case 40:
		serial_data2_state = state;
		return true;
	}
	return false;
}

DRIVER_INIT_MEMBER(wpc_95_state, tf95)
{
	pic->set_serial("648 123456 12345 123");
	lamp->set_names(nullptr);
	out->set_names(nullptr);
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, afm)
{
	pic->set_serial("541 123456 12345 123");
	lamp->set_names(lamps_afm);
	out->set_names(outputs_afm);
	out->set_handler(wpc_out_device::handler_t(FUNC(wpc_95_state::afm_led_handler), this));
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, cc)
{
	pic->set_serial("566 123456 12345 123");
	lamp->set_names(lamps_cc);
	out->set_names(outputs_cc);
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, cv)
{
	pic->set_serial("562 123456 12345 123");
	lamp->set_names(lamps_cv);
	out->set_names(outputs_cv);
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, congo)
{
	pic->set_serial("550 123456 12345 123");
	lamp->set_names(lamps_congo);
	out->set_names(outputs_congo);
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, jy)
{
	pic->set_serial("552 123456 12345 123");
	lamp->set_names(lamps_jy);
	out->set_names(outputs_jy);
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, mm)
{
	pic->set_serial("559 123456 12345 123");
	lamp->set_names(lamps_mm);
	out->set_names(outputs_mm);
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, mb)
{
	pic->set_serial("565 123456 12345 123");
	lamp->set_names(lamps_mb);
	out->set_names(outputs_mb);
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, nbaf)
{
	pic->set_serial("553 123456 12345 123");
	lamp->set_names(lamps_nbaf);
	out->set_names(outputs_nbaf);
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, ngg)
{
	pic->set_serial("561 123456 12345 123");
	lamp->set_names(lamps_ngg);
	out->set_names(outputs_ngg);
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, sc)
{
	pic->set_serial("903 123456 12345 123");
	lamp->set_names(lamps_sc);
	out->set_names(outputs_sc);
	out->set_handler(wpc_out_device::handler_t(FUNC(wpc_95_state::sc_aux_lamps_handler), this));
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, ss)
{
	pic->set_serial("548 123456 12345 123");
	lamp->set_names(lamps_ss);
	out->set_names(outputs_ss);
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, totan)
{
	pic->set_serial("547 123456 12345 123");
	lamp->set_names(lamps_totan);
	out->set_names(outputs_totan);
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, cp)
{
	pic->set_serial("563 123456 12345 123");
	lamp->set_names(lamps_cp);
	out->set_names(outputs_cp);
	init();
}

DRIVER_INIT_MEMBER(wpc_95_state, ttt)
{
	pic->set_serial("905 123456 12345 123");
	lamp->set_names(lamps_ttt);
	out->set_names(outputs_ttt);
	init();
}

const char *const wpc_95_state::lamps_afm[64] = {
	"Super jets", "Super jackpot", "Martian atk MB", "Total annihil", "Return to battle", "Conquer Mars", "5-way combo", "Drop target",
	"Big-o-Beam 1", "Big-o-Beam 2", "Big-o-Beam 3", "L ramp jackpot", "L ramp arrow", "Lock 2", "Lock 3", "C ramp jackpot",
	"Tractor beam 1", "Tractor beam 2", "Tractor beam 3", "R ramp jackpot", "R ramp arrow", "Martian attack", "Rule Universe", "Stroke of luck",
	"R loop arrow", "C ramp arrow", "Left top lane", "Right top lane", "L motor bank", "C motor bank", "R motor bank", "MAR\"T\"IAN target",
	"Attack Mars", "New York, USA", "London, England", "Light lock", "Lock 1", "Pisa, Italy", "Berlin, Germany", "Paris, France",
	"MARTIA\"N\" target", "MARTI\"A\"N target", "Atomic blaster 1", "Atomic blaster 2", "Atomic blaster 3", "R loop jackpot", "Extra ball", "MART\"I\"AN target",
	"Capture 1", "Capture 2", "Capture 3", "L loop jackpot", "L loop arrow", "\"M\"ARTIAN target", "M\"A\"RTIAN target", "MA\"R\"TIAN target",
	"Shoot again", "Left outlane", "Left return", "Right return", "Right outlane", "Launch button", nullptr, "Start button"
};

const char *const wpc_95_state::outputs_afm[52] = {
	"s:Auto plunger", "s:Trough eject", "s:Left popper", "s:Right popper", "s:Left alien lo", "s:Left alien hi", "s:Knocker", "s:Right alien hi",
	"s:Left slingshot", "s:Right slingshot", "s:Left jet", "s:Bottom jet", "s:Right jet", "s:Right alien lo", "s;Saucer shake", "s:Drop target",
	"f:R ramp hi (2)", "f:R ramp lo (2)", "f:R side hi (2)", "f:R side lo", "f:Center arrow", "f:Jets", "f:Saucer dome", "m:Motor bank",
	"f:L ramp L (2)", "f:L ramp R (2)", "f:L side hi (2)", "f:L side lo", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	"s:Right gate", "s:Left gate", "s:Diverter power", "s:Diverter hold", nullptr, nullptr, "f:Strobe light", nullptr,
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:Bottom playfield", "g:Middle playfield", "g:Top playfield"
};

static INPUT_PORTS_START( afm )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Launch button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Shooter lane")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Left popper")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Right popper")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left top lane")

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MARTI\"A\"N")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MARTIA\"N\"")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MAR\"T\"IAN")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MART\"I\"AN")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L motor bank")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("C motor bank")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R motor bank")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right top lane")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bottom jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"M\"ARTIAN")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("M\"A\"RTIAN")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MA\"R\"TIAN")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L ramp enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("C ramp enter")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R ramp enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L ramp exit")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R ramp exit")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Motor bank down")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Motor bank up")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right loop hi")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right loop lo")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop hi")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop lo")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L saucer tgt")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R saucer tgt")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drop target")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Center trough")

	PORT_START("SW.7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_cc[64] = {
	"Rank: stranger", "Rank: partner", "Rank: deputy", "Rank: sheriff", "Rank: marshall", "Star: motherlode", "L bonus X lane", "R bonus X lane",
	"Bounty beacon", "Jacktop beacon", "Shoot to collect", "EB lit beacon", "Bounty (saloon)", "Saloon arrow", "Extra ball", "Mine lock",
	"RC drop: badguy 3", "L drop: badguy 1", "L standup", "R ramp: snd alarm", "R ramp: shoot out", "R ramp: save Polly", "R ramp: jackpot", "R ramp: combo",
	"R loop: combo", "R loop: jacktop", "R loop: marksman", "R loop: gunslinger", "R loop: good shot", "L ret: quick draw", "L gunfight pin", "L out: gun fight",
	"R drop: badguy 4", "R standup (bot)", "R standup (top)", "C rmp: catch train", "C ramp: stop train", "C ramp: save Polly", "C ramp: jackpot", "C ramp: combo",
	"L rmp: white water", "L rmp: water fall", "L ramp: save Polly", "L ramp: jackpot", "L ramp: combo", "R ret: quick draw", "R out: special", "R gunfight pin",
	"Star: stampede", "Star: combo", "Star: high noon", "L loop: combo", "L loop: jackpot", "L loop: ride 'em", "L loop: wild ride", "L loop: B Bronco",
	"Star: Bart Bros", "Shoot again", "Star: show down", "LC drop: badguy 2", nullptr, nullptr, nullptr, "Start button"
};

const char *const wpc_95_state::outputs_cc[52] = {
	"s:Autoplunger", "s:#1 (L) drop tgt", "s:#2 (LC) drop tgt", "s:#3 (RC) drop tgt", "s:#4 (R) drop tgt", "s:Mine popper", nullptr, "s:Saloon popper",
	"s:Through eject", "s:L slingshot", "s:R slingshot", "s:Left jet", "s:Right jet", "s:L gunfight post", "s:R gunfight post", "s:Bottom jet",
	"s:Mine motor", "f:Mine flasher", "f:Front L flasher", "f:Front R flasher", "s:L loop gate", "s:R loop gate", nullptr, "f:Beacon flasher",
	"f:Mid R flasher", "f:Saloon flasher", "f:Back R flasher", "f:Back L flasher", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	"s:Move bart toy", nullptr, nullptr, "s:Bart toy hat", "s:Train reverse", "s:Train forward", nullptr, nullptr,
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:Illum string 1", "g:Illum string 2", "g:Illum string 3"
};

static INPUT_PORTS_START( cc )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Mine entrance")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Shooter lane")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R standup (bot)")

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L loop bottom")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Rt loop bottom")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mine popper")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Saloon popper")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R standup (top)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Beer mug switch")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L bonus X lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Jet exit")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bottom jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right loop top")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R bonus X lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop top")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drop #1 (L)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drop #2 (LC)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drop #3 (RC)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drop #4 (R)")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R ramp make")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R ramp enter")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Skill bowl")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bot R ramp")

	PORT_START("SW.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Train encoder")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Train home")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Saloon gate")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Saloon bart toy")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mine home")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mine encoder")

	PORT_START("SW.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("C ramp enter")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L ramp make")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("C ramp make")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L ramp enter")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L standup (top)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L standup (bot)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_cv[64] = {
	"Cirqus \"R\"", "Grid top", "Cirqus \"Q\"", "Cirqus \"U\"", "Grid top/right", "Cirqus \"S\"", "Grid mid/right", "Left jackpot",
	"Cirqus \"I\"", "Cirqus \"C\"", "Grid mid/left", "Grid bot/left", "Grid bottom", "Grid middle", "Grid bot/right", "Grid top/left",
	"Side show", "Left loop top", "Left loop 3", "Left loop 2", "Left loop 1", "\"Multiball\"", "\"Lock\"", "\"Spot marvel\"",
	"Ringmaster left", "Ringmaster 2", "Ringmaster 3", "Ringmaster 4", "Ringmaster right", "\"Special\"", "\"Razz\"", "\"Frenzie\"",
	"Spin \"S\"", "Spin \"P\"", "Spin \"I\"", "Spin \"N\"", "Right loop top", "Right loop 3", "Right loop 2", "Right loop 1",
	"Middle jackpot", "Right jackpot", "\"Light\" standup", "\"Lock\" standup", "Ring \"R\"", "Ring \"I\"", "\"Shoot again\"", "Left outlane",
	"Wow \"W\" right", "Wow \"O\"", "Wow \"W\" left", "Ring \"N\"", "Ring \"G\"", "Right outlane", "Left inlane", "\"Skill shot\"",
	"\"Extra ball\"", "Top jet", "Middle jet", "Lower jet", "Right inlane", "\"Volt\" left", "\"Volt\" right", "Start button"
};

const char *const wpc_95_state::outputs_cv[52] = {
	"s:Plunger", "s:Back box kick", "s:Left loop magnet", "s:Middle jet", "s:Ramp magnet", "s:Diverter power", "s:Jet up", "s:Jet release",
	"s:Trough eject", "s:Left sling", "s:Right sling", "s:Upper jet", "s:Lower jet", "s:Left saucer", "s:Right saucer", "s:Lock post",
	"f:Join flasher", "f:Ring 1 flasher", "f:Ring 2 flasher", "f:Ring 3 flasher", "f:Fl right/BB URT", "s:Motor enable", "f:Jet flasher", "f:Fl right/BB ULeft",
	"f:FL upper left", "f:Fl URight/BB LLT", "f:Ringmaster FLx2", "f:FL bear/BB LRT", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	"s:Popper", "s:Diverter hold", "s:Ringmaster mgnt", "s:Upper post", "f:Neon", nullptr, "s:Motor direction", nullptr,
	nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:Playfield right", "g:Playfield middle", "g:Playfield left"
};

static INPUT_PORTS_START( cv )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Back box luck")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Wire ramp enter")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop upper")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top Eddy")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right inlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Shooter lane")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right loop upper")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Inner loop left")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left inlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Inner loop right")

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Poppor opto")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"WOW\" targets")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top targets")

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left lane")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ringmaster up")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ringmaster mid")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ringmaster down")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left ramp made")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Trough upper")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Trough middle")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop enter")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right sling")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Upper jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Middle jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lower jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Skill shot")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ring 'N', 'G'")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"Light\" standup")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"Lock\" standup")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp magnet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp made")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp lock low")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp lock mid")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp lock high")

	PORT_START("SW.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left saucer")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right saucer")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Big ball rebound")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"Volt\" right")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"Volt\" left")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Right spinner")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Left spinner")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_congo[64] = {
	"(C)ONGO", "C(O)NGO", "CO(N)GO", "CON(G)O", "CONG(O)", "(A)MY", "A(M)Y", "AM(Y)",
	"ZI(N)J", "ZIN(J)", "Jet ex \"Collect\"", "\"Jungle jackpot\"", "\"Skill fire\"", "\"You\"", "\"Map\"", "Diamond Rt eject",
	"\"Autofire\"", "Rt ramp ExBall", "Rt ramp \"Collect\"", "Diamond Rt Ramp", "Left eject eye", "Diamond Lt eject", "\"Mystery\"", "Rt ramp jackpot",
	"Diamond Lt loop", "\"We are\"", "Lt loop ExBall", "Lt loop \"Lock\"", "Left bank bot", "\"Skill shot\"", "Left bank center", "Left bank top",
	"Left ramp \"P\"", "Left ramp \"A\"", "Left ramp \"M\"", "Diamond Lt ramp", "Lt ramp jackpot", "(Z)INJ", "Z(I)NJ", "\"Kickback\"",
	"Diamond in loop", "(G)RAY", "G(R)AY", "GR(A)Y", "GRA(Y)", "\"Watching\"", "Satellite left", "\"Super score\"",
	"\"Travi\"", "\"Com\"", "\"Mine shaft\"", "Up loop \"Lock\"", "Diamond up loop", "Satellite right", "Satellite center", "\"Perimeter def\"",
	"(H)IPPO", "H(I)PPO", "HI(P)PO", "HIP(P)O", "HIPP(O)", "\"Shoot again\"", "Launch button", "Start button"
};

const char *const wpc_95_state::outputs_congo[52] = {
	"s:Auto plunger", "s:Kickback", "s:2-way popper up", "s:2-way popper dn", "s:Ramp diverter", "s:Volcano popper", "s:Backbox knocker", "s:Top loop post",
	"sTrough eject:", "s:Left slingshot", "s:Right slingshot", "s:Left jet", "s:Right jet", "s:Bottom jet", "s:Gorilla left", "s:Gorilla right",
	"f:Amy flasher", "f:Left ramp fls", "f:2-way popper fls", "f:Skill shot fls", "f:Gray gorilla fls", "s:\"Map\" eject", "s:Left gate", "s:Right gate",
	"f:Lower right fls", "f:Right ramp fls", "f:Volcano flasher", "f:\"Perimeter def\"", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	"s:Up left post", "s:\"Mystery\" eject", "s:UL flip power", "s:UL flip hold", nullptr, nullptr, nullptr, nullptr,
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:Playfld gorilla", "g:Playfield top", "g:Playfield bottom"
};

static INPUT_PORTS_START( congo )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Inner left loop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Upper loop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Jet exit")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Rt return lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Shooter lane")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Launch button")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Rt eject rubber")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lt return lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"You\" standup")

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Volcano stack")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"Mystery\" eject")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right eject")

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Lock ball 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Lock ball 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Lock ball 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"Mine shaft\"")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left bank top")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left bank center")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left bank bottom")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"Travi\"")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"Com\"")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2-way popper")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"We are\" standup")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"Watching\" standup")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"Perimeter def\"")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left ramp enter")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left ramp exit")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bottom jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right ramp enter")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right ramp exit")

	PORT_START("SW.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("(A)MY")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A(M)Y")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("AM(Y)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("(C)ONGO")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("C(O)NGO")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("CO(N)GO")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("CON(G)O")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("CONG(O)")

	PORT_START("SW.7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper EOS")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_jy[64] = {
	"TL bank bot", "TL bank mid", "TL bank top", "TR bank top", "TR bank mid", "TR bank bot", "Right recycle", "Right crane HU",
	"Right 3 bank top", "Right 3 bank mid", "Right 3 bank bot", "Left bank bot", "Left bank mid", "Left bank top", "Fan", "Bath tub",
	"Jackpot", "Super jackpot", "Multiball", "Wrecking ball", "Radar adventure", "Jolopy race", "Toilet adventure", "ATC adventure",
	"Gen bus", "Toast", "Magic bus", "Collect junk", "Coo coo clock", "Television", "Weather vane", "Fish bowl",
	"Gen toilet", "Window shopping", "Left recycle", "Left crane HU", "Shoot again", nullptr, "Toaster", "Hair dryer",
	"Propeller", "Outerspace", "DO(G)", "(D)OG", "D(O)G", "Choose junk", "Angel sling", "Bicycles",
	"Time machine", "Start adventure", "Extra ball", "Toast 2", "Gen sewer", "Toaster gun", "Gen alley", "Devil sling",
	"Fire works", "Toxic waste", "Lite extra ball", "Free game", "Lite jackpot", "Gen crane", nullptr, "Start button"
};

const char *const wpc_95_state::outputs_jy[52] = {
	"s:Auto plunger", "s:Refridge popper", "s:Power crane", nullptr, "s:Scoop down", "s:Bus diverter", "s:Knocker", nullptr,
	"s:Trough", "s:Left sling", "s:Right sling", nullptr, nullptr, nullptr, "s:Hold crane", "s:Move dog",
	"f:1 Fl dog face", "f:1 Fl window shop", "f:2 Fl autofire", "f:2 Fl left side", "s:Scoop up", "f:1 Fl under crane", "f:2 Fl back left", "f:2 Fl back rght",
	"f:1 Fl shooter", "f:2 Fl scoop", "f:1 Fl dog house", "f:2 Fls cars", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:Playfld string 1", "g:Playfld string 2", "g:Logo string"
};

static INPUT_PORTS_START( jy )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Toaster gun")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Rebound sw")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top left crane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left return lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Shooter lane")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Rght return lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Crane down")

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Lock up 2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Lock up 1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top right crane")

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Past spinner")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("In the sewer")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Lock jam")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Past crane")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp exit")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Car targ 1 left")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Car targ 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Car targ 3")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right sling")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Car targ 4")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Car targ 5 rght")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L L 3 bank bot")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L L 3 bank mid")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L L 3 bank top")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("U R 3 bank bot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("U R 3 bank mid")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("U R 3 bank top")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("U L 3 bank bot")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("U L 3 bank mid")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("U L 3 bank top")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bowl entry")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bowl exit")

	PORT_START("SW.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp entry")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Scoop down")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Scoop made")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Dog entry")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R 3 bank bottom")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R 3 bank middle")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R 3 bank top")

	PORT_START("SW.7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Spinner")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_mm[64] = {
	"Right bank top", "Right bank mid", "Right bank bot", "R ramp jackpot", "Save damsel! (2)", "Dragon death", "Dragon snack", "Dragon breath",
	"R loop jackpot", "R joust victory!", "R clash!", "R charge!", "Patron/peasants", "Catapult ace", "Joust champion", "Castle crusher",
	"Trolls!", "Extra ball", "Merlin's magic", "Troll madness", "Damsel madness", "Peasant madness", "Catapult madness", "Joust madness",
	"L loop jackpot", "L joust victory!", "L clash!", "L charge!", "Catapult jackpot", "Catapult slam!", "Bam!", "Wham!",
	"Center arrow", "Battle/Kingdom", "Master of trolls", "Defender/Damsels", "Left top lane", "Right top lane", "L troll target", "R troll target",
	"Francois D'Grimm", "King of Payne", "Earl of Ego", "L ramp jackpot", "Revolt peasants!", "Ugly riot!", "Angry mob!", "Rabble rouser",
	"Howard Hurtz", "Magic shield", "Sir Psycho", "Duke of Bourbon", "Castle lock 2", "Castle lock 1", "Super jackpot", "Super jets (2)",
	"Right outlane", "Right return", "Left return", "Left outlane", "Castle lock 3", "Shoot again", "Launch button", "Start button"
};

const char *const wpc_95_state::outputs_mm[52] = {
	"s:Auto plunger", "s:Trough eject", "s:Left popper", "s:Castle", "s:Castle grate pwr", "s:Castle gate hold", "s:Knocker", "s:Catapult",
	"s:Right eject", "s:Left slingshot", "s:Right slingshot", "s:Left jet", "s:Bottom jet", "s:Right jet", "s:Twr divert power", "s:Twr divert hold",
	"f:L side lo (2)", "f:Left ramp (2)", "f:L side hi (2)", "f:R side hi (2)", "f:Right ramp (2)", "f:Cstl R side (2)", "f:R side lo (2)", "f:Moat (2)",
	"f:Cstl L side (2)", "s:Tower lock post", "s:Right gate", "s:Left gate", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	"s:L troll power", "s:L troll hold", "s:R troll power", "s:R troll hold", "s:Drawbridge motor", nullptr, nullptr, nullptr,
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:Bottom playfield", "g:Middle playfield", "g:Top playfield"
};

static INPUT_PORTS_START( mm )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Launch button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Catapult target")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L troll target")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Shooter lane")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R troll target")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right eject")

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Left popper")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Castle gate")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Catapult")

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Moat enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Castle lock")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L troll (U/pfld)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R troll (U/pfld)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left top lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right top lane")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bottom jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drawbridge up")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drawbridge down")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Tower exit")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L ramp enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L ramp exit")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R ramp enter")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R ramp exit")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop lo")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop hi")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right loop lo")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right loop hi")

	PORT_START("SW.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right bank top")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right bank mid")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right bank bot")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L troll up")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R troll up")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_mb[64] = {
	"Monster mosh pit", "1/2 moon (2)", "Frank arrow", "Drac-attack", "Extra ball", "Monsters of rock", "Monster bash", "Mummy mayhem",
	"R ramp arrow", "Rock CD", "Right return", "F moon fever (2)", "R gargle", "R warm up", "R primp", "R loop arrow",
	"1/4 moon (2)", "L blue tgt", "Tomb treasure", "Drac standup top", "Right top lane", "Middle top lane", "Left top lane", "Drac standup bot",
	"Left return", "Left outlane", "3/4 moon (2)", "R blue tgt", "L ramp arrow", "L primp", "L warm up", "L gargle",
	"Guitar", "Drums", "Bass guitar", "Keyboard", "Microphone", "Saxophone", "C loop arrow 3", "C blue tgt",
	"Creature", "Bride", "Frankenstein", "Mummy", "Wolfman", "Dracula", "Right outlane", "Shoot again",
	"L frank arm", "L frank leg", "Frank torso", "Frank head", "R frank leg", "R frank arm", "L loop arrow", nullptr,
	"Muck", "Seaweed", "Algae", "Pond scum", "C loop arrow 2", "C loop arrow 1", "Launch button", "Start button"
};

const char *const wpc_95_state::outputs_mb[52] = {
	"s:Auto plunger", "s:Bride post", "s:Mummy coffin", nullptr, "s:Left gate", "s:Right gate", nullptr, "s:Ramp lock post",
	"s:Trough eject", "s:Left slignshot", "s:Right slingshot", "s:Left jet", "s:Right jet", "s:Bottom jet", "s:Left eject", "s:Right popper",
	"f:Wolfman (2)", "f:Bride", "f:Frankenstein (2)", "f:Dracula coffin", "f:Creature (2)", "f:Jets/mummy (2)", "f:Right popper", "f:Frank arrow",
	"f:Rock CD", "f:Wolfman loop (2)", nullptr, nullptr, "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:Bottom playfield", "g:Top R playfield", "g:Top L playfield"
};

static INPUT_PORTS_START( mb )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Launch button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drac standup top")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drac standup bot")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Shooter lane")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Tomb treasure")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Dracula target")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left eject")

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Right popper")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L flip opto")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R flip opto")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L blue tgt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("C blue tgt")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R blue tgt")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L flip prox")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R flip prox")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bottom jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left top lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Middle top lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right top lane")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop lo")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop hi")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right loop lo")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right loop hi")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Center loop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L ramp enter")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L ramp exit")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("C ramp enter")

	PORT_START("SW.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R ramp enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R ramp exit")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R ramp lock")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drac position 5")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drac position 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drac position 3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drac position 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Drac position 1")

	PORT_START("SW.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Up/dn bank up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Up/dn bank down")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Frank table down")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Frank table up")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L up/dn bank tgt")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R up/dn bank tgt")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Frank hit")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Center spinner")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_nbaf[64] = {
	"'20' points", "Free throw", "3 pt", "2 pt", "Field goals", "Multiballs", "Shoot around", "Around the world",
	"Power hoops", "Fastbreak combo", "Alley oop combo", "Slam dunk combo", "Combos", "Trophy", "Tip-off combo", "Stadium goodies",
	"Multiball hoops", "Run&shoot hoops", "Hook shot hoops", "Half CT hoops", "Lite tip-off", "R \"In the paint\"", "SHOO(T)", "Lt return lane",
	"Champ ring 1", "Champ ring 2", "Rt return lane", "Champ ring 4", "Champ ring 3", "L R standup", "U R standup", "Left outlane",
	"Soda", "Question", "Hot dog", "Pizza", "Crazy bob's", "Extra ball", "Right outlane", "\"Shoot again\"",
	"Ramps: 3 points", "Tip-off", "Fastbreak", "Alley oop", "Free throw", "SH(O)OT", "\"In the paint\" 4", "\"In the paint\" 3",
	"L lite fastbreak", "Slam dunk", "S(H)OOT", "R lite fastbreak", "Lite slam dunk", "SHO(O)T", "\"In the paint\" 1", "\"In the paint\" 2",
	"Lite alley oop", "L \"In the paint\"", "(S)HOOT", "(3)PT", "3(P)T", "3P(T)", "Ball launch", "Start button"
};

const char *const wpc_95_state::outputs_nbaf[52] = {
	"s:Autoplunger", "s:", "s:L ramp diverter", "s:R loop diverter", "s:Eject", "s:Loop gate", "s:Backbox flipper", "s:Ball catch mag",
	"s:Trough eject", "s:Left sling", "s:Right sling", "s:Left jet", "s:Middle jet", "s:Right jet", "s:Pass right 2", "s:Pass left 2",
	"f:Eject kickout", "f:Left jet bumper", "f:Upper left", "f:Upper right", nullptr, "f:Trophy insert", nullptr, "f:Lower left/right",
	"s:Pass right 1", "s:Pass left 3", "s:Pass right 3", "s:Pass left 4", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	"s:Shoot 1", "s:Shoot 2", "s:Shoot 3", "s:Shoot 4", "s:Motor enable", "s:Motor direction", "s:Shot clk enable", "s:Shot clk count",
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:String 1", "g:String 2", "g:String 3"
};

static INPUT_PORTS_START( nbaf )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Launch button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Backbos basket")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Shhoter lane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lt return lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Rt return lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L R standup")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right jet")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Eject hole")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("U R standup")

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Center ramp opto")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R loop ent opto")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right loop exit")

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Standup '3'")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Standup 'P'")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Standup 'T'")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right ramp enter")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left ramp enter")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left ramp made")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop enter")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop made")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Defender pos 4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Defender pos 4")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Defender lock pos")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Defender pos 2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Defender pos 1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Jets ball drain")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L slingshot")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("R slingshot")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left jet")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Middle jet")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L loop ramp exit")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right ramp made")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"In the paint\" 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"In the paint\" 3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"In the paint\" 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("\"In the paint\" 1")

	PORT_START("SW.6")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Basket made")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Basket hold")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_ngg[64] = {
	"Outlane ex ball", "Kickback", "Lower drv range", "Shoot again", "Special", "Wheel value", "Jet lightning", "Hole 8",
	"Hole 5", "Hole 4", "Hole 3", "Hit bud", "Hole 1", "2X", "Cart path 2", "5X cart path",
	"Hole 6", "Hole 7", "Hole 2", "Hit buzz", "Hole 9", "4X", "Cart path 4", "3X",
	"Driving range", "Inc golf cart", "Inc buzz value", "Inc bud value", "Newton drive", "Collect", "Ripoff", "Left loop drive",
	"(K)ICK", "K(I)CK", "KI(C)K", "KIC(K)", "Skill shot", "Relight jackpot", "Rgt ramp lock", "Rgt ramp drive",
	"4 strokes", "3 strokes", "2 strokes", "5 strokes", "7 strokes", "6 strokes", "Left spinner", "Trap ready",
	"Adv trap", "Center drive", "Center lock", "Get TNT", "Ctr raise gofer", "Right spinner", "Rgt loop drive", "Bottom jet",
	"Side ramp drive", "Extra ball", "Multiball", "Jackpot", "Putt out", "Top jet", "Middle jet", "Start button"
};

const char *const wpc_95_state::outputs_ngg[52] = {
	"s:Autofire", "s:Kickback", "s:Clubhouse kicker", "s:Left gofer up", "s:Right gofer up", "s:Jet popper", "s:Left eject", "s:Upper rgt eject",
	"s:Trough eject", "s:Left slingshot", "s:Right slingshot", "s:Top jet", "s:Middle jet", "s:Bottom jet", "s:Left gofer down", "s:Rgt gofer down",
	"f:Jet flash", "f:Lower lft flash", "f:Left spinr flash", "f:Rgt spinr flash", "f:Lower rgt flash", nullptr, nullptr, "s:Underground pass",
	"f:Sand trap flash", "f:Wheel flasher", "s:Left ramp down", "s:Right ramp down", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	"s:UR flip power", "s:UR flip hold", "s:Ball launch ramp", nullptr, "s:Wheel spin CCW", "s:Wheel spin CW", nullptr, nullptr,
	"c:Coin meter",
	"f:Upper right 1", "f:Upper right 2", "f:Upper right 3", "f:Upper pf right", "f:Upper pf left", "f:Upper left 3", "f:Upper left 2", "f:Upper left 1",
	"g:Left side string", "g:Rgt side string", "g:Gofer spotlight"
};

static INPUT_PORTS_START( ngg )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left ramp make")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Center ramp make")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right inlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Shooter groove")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Jet adv standup")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Underground pass")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left inlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Kickback")

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Jet popper")

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L gofer down")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R gofer down")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Putt out popper")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Rt popper jam")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Right popper")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left ramp down")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right ramp down")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Middle jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bottom jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top skill shot")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Mid skill shot")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lower skill shot")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left spinner")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right spinner")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Inner wheel opto")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Outer wheel opto")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left gofer 1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left gofer 2")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Behind L gofer")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Hole in 1 made")

	PORT_START("SW.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left cart path")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right cart path")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right ramp make")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Golf cart")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right gofer 1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right gofer 2")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Adv trap value")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Sand trap eject")

	PORT_START("SW.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("K-I-C-K advance")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("(K)ICK")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("K(I)CK")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("KI(C)K")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("KIC(K)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Captive ball")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper EOS")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_sc[64] = {
	"Lite deposit", "Ctr timer \"10\"", "Disable computer", "Ctr timer \"5\"", "Ctr timer \"0\"", "Lite lock", "Ctr timer \"55\"", "Ctr timer \"50\"",
	"Ctr timer \"15\"", "Ctr timer \"20\"", "Ctr timer \"25\"", "Ctr timer \"30\"", "Ctr timer \"35\"", "Call guard", "Ctr timer \"45\"", "Ctr timer \"40\"",
	"Armor car-cellar", "Armor car-roof", "Armor car-main", "Bonus 2X", "(A)LARM standup", "ATM card", "A(L)ARM standup", "Ramp jackpot",
	"Bonus 5X+Outlane", "Bonus 5X", "Bonus 4X", "Bonus 3X", "Ramp lock", "AL(A)RM standup", "ALA(R)M standup", "ALAR(M) standup",
	"Wheel arrow", "Lire outlanes", "Vault letter", "Explosives", "Note to teller", "Top left lane", "Top middle lane", "Top right lane",
	"TR 3bank top", "TR 3bank middle", "TR 3bank bottom", "TL 3bank top", "TL 3bank middle", "TL 3bank bottom", "Rt \"Extra time\"", "Right return",
	"BR 3bank top", "BR 3bank middle", "BR 3bank bottom", "BL 3bank top", "BL 3bank middle", "BL 3bank bottom", "Left return", "Lt \"Extra time\"",
	"Top jet (Yellow)", "Left jet (Clear)", "Right jet (Red)", "Bank left", "Bank right", "Movng break in", "Roof break in", "Start button"
};

const char *const wpc_95_state::lamps_sc_extra[48] = {
	"L24 \"!\"", "L23 \"Teller\"", "L22 \"Dog\"", "L21 \"?\"", "L20 \"Alarm 3\"", "L19 \"$\"", "L18 \"Dog\"", "L17 \"Candy\"",
	"L16 \"$\"", "L15 \"?\"", "L14 \"Alarm 2\"", "L13 \"#\"", "L12 \"<-->\"", "L11 \"Teller\"", "L10 \"Bribe\"", "L9 \"?\"",
	"L8 \"Alarm 1\"", "L7 \"$\"", "L6 \"Dog\"", "L5 \"Candy\"", "L4 \"$\"", "L3 \"?\"", "L2 \"Alarm 4\"", "L1 \"$\"",
	"L48 \"Bribe\"", "L47 \"?\"", "L46 \"$\"", "L45 \"?\"", "L44 \"Cellar\"", "L43 \"$\"", "L32 \"?\"", "L41 \"?\"",
	"L40 \"Vault\"", "L39 \"Gate 1\"", "L38 \"?\"", "L37 \"Gate 2\"", "L36 \"?\"", "L35 \"Gate 3\"", "L34 \"Gate 4\"", "L33 \"?\"",
	"L32 \"?\"", "L31 \"Bribe\"", "L30 \"Roof\"", "L29 \"Bribe\"", "L28 \"$\"", "L27 \"?\"", "L26 \"$\"", "L5 \"Main\"",
};

const char *const wpc_95_state::outputs_sc[52] = {
	"s:Big kick", "s:Right token tube", "s:Move tgt reset", "s:Left token tube", "s:Bank kick", "s:Top popper up", "s:Ramp divertor", "s:Kickback (ramp)",
	"s:Trough eject", "s:Left slingshot", "s:Right slingshot", "s:Left jet", "s:Right jet", "s:Top jet", "s:Top L 3 bank", "s:Top R 3 bank",
	"f:Back left", "f:Jets+Bk Rt (2)", "f:Right middle", "f:Right bottom", "f:Left middle", "f:Left bottom", "f:Light rope 1", "f:Light rope 2",
	"s:Top popper eject", "s:Top light+motor", "s:Bot L 3 bank", "s:Bot R 3 bank", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	"s:UR flip power", "s:UR flip hold", "s:Auto plunger", "s:Lockup release", nullptr, nullptr, nullptr, nullptr,
	nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:Illum string 1", "g:Aux lamp 1 power", "g:Illum string 3"
};

static INPUT_PORTS_START( sc )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TP trough (roof)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TP trough (move)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right orbit")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ballshooter")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("UR flip rollover")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left orbit")

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Lockup 1 front")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Lockup 2 rear")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Kickback")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Left big kick")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Tokn chute exit")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top jet")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left slingshot")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right slingshot")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("(A)LARM standup")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("A(L)ARM standup")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("AL(A)RM standup")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("ALA(R)M standup")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("ALAR(M) standup")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Movng target C")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Movng target B")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Movng target A")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TL 3bank top")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TL 3bank middle")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TL 3bank bottom")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TR 3bank bottom")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TR 3bank middle")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TR 3bank top")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top left lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top popper")

	PORT_START("SW.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BL 3bank top")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BL 3bank middle")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BL 3bank bottom")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BR 3bank bottom")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BR 3bank middle")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BR 3bank top")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bank kickout")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top right lane")

	PORT_START("SW.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left token lvl")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right token lvl")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp entrance")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp made")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Wheel channel A")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Wheel channel B")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper EOS")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Token coin slot")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_ss[64] = {
	"Stiff level 7", "Stiff level 6", "Stiff level 5", "Stiff level 4", "Stiff level 3", "Stiff level 2", "Stiff level 1", "Ramp left eye",
	"Stiff level 8", "Stiff level 9", "Scared stiff", "Center leaper", "Three bank lower", "Three bank mid", "Three bank upper", "Spider popper",
	"Crate left eye", "Crate cnt lt eye", "Crate cnt rt eye", "Crate right eye", "Left outlane", "Right leaper", "Rt ramp jackpot", "Lt spin spider",
	"Left leaper", "Lft ramp jackpot", "Light lock", "Ramp right eye", "Right outlane", "Skill shot", "Crate jackpot", "Extra ball",
	"Ramp item", "Coffin mult item", "Leaper item", "Coffin spotlight", "Shoot again", "Lock lamp", "Left loop center", "Left loop upper",
	"Laboratory item", "Crate item", "Skull item", "Web award 2", "Web award 3", "Web award 4", "Web award 5", "Web award 6",
	"Web award 7", "Web award 8", "Web award 9", "Web award 10", "Web award 11", "Web award 12", "Web award 13", "Web award 14",
	"Web award 15", "Web award 16", "Web award 1", "Left skull lane", "Enter skull lane", "Right skull lane", nullptr, "Start button"
};

const char *const wpc_95_state::outputs_ss[52] = {
	"s:Auto plunger", "s:Loop gate", "s:Right popper", "s:Coffin popper", "s:Coffin door", "s:Crate kickout", "s:Knocker", "s:Crate post power",
	"s:Trough eject", "s:Left sling", "s:Right sling", "s:Center jet", "s:Upper jet", "s:Lower jet", "s:Upper slingshot", "s:Crate post hold",
	"f:Top jet flasher", "f:Mid jet flasher", "f:Lower jet flash", "f:Playfield bolts", "f:Blue skull fl", "f:U right flasher", "f:Lft ramp flasher", "f:C left flasher",
	"f:White skull fl", "f:Center TV", "f:U left flasher", "f:C right flasher", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	"s:Lft divert power", "s:Lft divert hold", "f:L left flasher", "f:L right flasher", nullptr, nullptr, nullptr, nullptr,
	nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:Upper playfield", "g:Center playfield", "g:Lower playfield"
};

static INPUT_PORTS_START( ss )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Wheel index")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Rt flipper lane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Shooter lane")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Extra ball lane")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lft flipper lane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Single standup")

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Right popper")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Left kickout")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Crate entrance")

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coffin left")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coffin center")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coffin right")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Left ramp enter")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Right ramp enter")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Left ramp made")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Right ramp made")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coffin entrance")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left slingshot")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right slingshot")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Upper jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Center jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lower jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Upper slingshot")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Crate sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Three bank upper")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Three bank mid")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Three bank lower")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left leaper")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Center leaper")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right leaper")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Rt ramp 10 point")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right loop")

	PORT_START("SW.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left skull lane")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Center skull lane")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right skull lane")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Secret passage")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_totan[64] = {
	"Jewel 1 (left)", "Jewel 2", "Jewel 3", "Jewel 4", "Jewel 5", "Jewel 6", "Jewel 7 (right)", "Shoot again",
	"Jackpot", "(G)ENIE", "G(E)NIE", "GE(N)IE", "GEN(I)E", "GENI(E)", "Multiball", "Outlane special",
	"Magic carpet", "Action 3", "Ramp arrow rgt", "Ramp arrow left", "Smoke 1 (bot)", "Smoke 2", "Smoke 3", "Amulet",
	"Smoke 6", "Smoke 7", "Smoke 8", "Smoke 9", "Smoke 10", "Smoke 11", "Smoke 12", "Smoke 13",
	"Smoke 14 (top)", "Lamp 15", "Lamp 30", "Lamp 60", "Smoke 4", "Smoke 5", "Shoot star right", "Shoot star left",
	"Make a wish", "(B)AZAAR", "B(A)ZAAR", "BA(Z)AAR", "BAZ(A)AR", "BAZA(A)R", "BAZAA(R)", "Center lock",
	"Action 2", "Left lock", "Harem advance", "Left tiger loop", "Action 1", "Wish 1", "Wish 2", "Wish 3",
	"Right lock", "Action 5", "Extra ball", "Rgt tiger loop", "Captive ball rgt", "Action 4", "Captive ball lft", "Start button"
};

const char *const wpc_95_state::outputs_totan[52] = {
	"s:Left cage", "s:Right cage", "s:Vanish drop", "s:Lock eject", "s:Bazaar eject", "s:Lock magnet", "s:Knocker", "s:Ramp magnet coil",
	"s:Trough eject", "s:Left sling", "s:Right sling", "s:Left jet", "s:Right jet", "s:Middle jet", "s:Left kick", "f:Left ej flash",
	"f:Inlane flashers", "f:Final battle", "f:Left loop flash", "f:Bazaar flash", "s:Ramp diverter", "f:Rub lamp flash", "f:Magic lamp fls", "f:Right loop flash",
	"f:Start tale fls", "f:Jet flashers", "f:Top loop flash", "f:Ramp flash", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	"s:Left div high", "s:Left div hold", "s:Vanish magnet", "s:Loop post div", "s:", "s:", "s:", "s:",
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:Illum string 1", "g:Illum string 2", "g:Illum string 3"
};

static INPUT_PORTS_START( totan )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Harem passage")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Vanish tunnel")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp enter")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right inlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ball shooter")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Genie standup")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bazaar eject")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left inlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left wire make")

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Left cage opto")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Right cage opto")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left eject")

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp made left")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Genie target")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left loop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Inner loop left")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Inner loop right")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Mini standups")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ramp made right")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right captive ball")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right slign")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left jet")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right jet")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Middle jet")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lamp spin CCW")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lamp spin CW")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left captive ball")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left standups")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right standups")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top skill")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Middle skill")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Bottom skill")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lock 1 (bot)")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lock 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lock 3 (top)")

	PORT_START("SW.6")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_cp[64] = {
	"Heavy bag cmplt", "Jump rope cmplt", "Speed bag cmplt", "Right jab combo", "Lock", "Rgt start fight", "Right jackpot", "Right jab",
	"Bout 1", "Bout 2", "Bout 3", "Bout 4", "Jump rope", "Left jab combo", "Cnt start fight", "Left jab",
	"Low blue arrow", "Left hook to win", "White arrow", "Thrown towel", "Cntr blue arrow", "Low yellow arrow", "Top yellow arrow", nullptr,
	"Left hook", "Body blow", "Right hook", "Center jackpot", "Left KO boxer", "Hurry up", "Heavy bag", "Right KO boxer",
	"Jackpots cmplt", "Pub champion", "Won by KO", "Multiballs cmplt", "Training cmplt", "Speed bag", "Left jackpot", "Balcony",
	"Ultimate chlng", "Poker night", "Extra ball", "Spittn gallery", "Lft start fight", "The corner", "Right return", "Rght second wind",
	"Raid", "Fistacuff", "Multibrawl", "Three bank top", "Three bank mid", "Three bank bot", "Left return", "Left KO",
	"Right KO", "Left second wind", "Top blue arrow", "Center KO", "Ball save post", "Shoot again", "Launch button", "Start button"
};

const char *const wpc_95_state::outputs_cp[52] = {
	"s:Auto plunger", "s:Trough eject", "s:Left scoop power", "s:Rght scoop power", "s:Corner kickout", "s:Post power", "s:Rope magnet", "s:Post divertor",
	"s:Left scoop hold", "s:Rght scoop hold", "s:Arm 2", "s:Post hold", "s:Arm 1", "s:Popper", "s:Left sling", "s:Right sling",
	"f:3 fl boxer spot", "f:2 fl dngr zone", "f:1 fl back pan", "f:Lock kickout", "f:Corner kickout", "f:2 boxer red", "f:Jp rope spot", "f:Spd bag spot",
	"s:Rope motor", "s:Toggle direction", "s:Motor on/off", "s:Lock pin", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	"s:Rope popper", "s:Ramp divertor", "s:Left speed bag", "s:Right speed bag", "s:", "s:", "s:", "s:",
	"s:Coin meter",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:Illum string 1", "g:Illum string 2", "g:Illum string 3"
};

static INPUT_PORTS_START( cp )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Made ramp")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Heavy bag")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lock up 1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left outlane")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right return")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Shooter lane")

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Ball launch")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Three bank mid")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left return")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right outlane")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Popper")

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough eject")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Trough ball 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Left jab made")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Corner eject")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Right jab made")

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Boxer pole cntr")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Behnd left scoop")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Behnd rght scoop")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter ramp")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Jump rope")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bag pole center")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Boxer pole right")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Boxer pole left")

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left sling")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right sling")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Three bank bottom")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Three bank top")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left half guy")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Rght half guy")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lock up 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lock up 3")

	PORT_START("SW.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left scoop up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right scoop up")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Power shot")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Rope cam")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Speed bag")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Boxer gut 1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Boxer gut 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Boxer head")

	PORT_START("SW.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Exit rope")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter speed bag")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter lockup")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top of ramp")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter rope")

	PORT_START("SW.7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

const char *const wpc_95_state::lamps_ttt[64] = {
	"Hole \"5\"", "Hole \"6\"", "Hole \"7\"", "Hole \"8\"", "Hole \"9\"", nullptr, nullptr, nullptr,
	nullptr, "Grid \"1\"", "Grid \"2\"", "Grid \"3\"", "Grid \"4\"", "Grid \"5\"", "Grid \"6\"", "Grid \"7\"",
	"Grid \"8\"", "Grid \"9\"", "Left post", "Right post", "Hole \"1\"", "Hole \"2\"", "Hole \"3\"", "Hole \"4\"",
	"Back panel 1", "Back panel 2", "Back panel 3", "Back panel 4", "Back panel 5", "Back panel 6", nullptr, nullptr,
	"Left sling up", "Left sling low", "Left ret left", "Left ret right", "Right sling up", "Right sling low", "Right ret right", "Right ret left",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "Start button"
};

const char *const wpc_95_state::outputs_ttt[52] = {
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"s:Kicker", "f:Fl left post", "f:Fl right post", "f:Fl insert 1", "f:Fl insert 2", "f:Fl insert 3", "f:Fl insert 4", nullptr,
	"f:Left flasher", "f:Right flasher", "f:Fl left sling", "f:Fl left return", "f:Fl back panel 1", "f:Fl back panel 2", "f:Fl back panel 3", nullptr,
	nullptr, "s:Ticket lamp", "f:Fl right sling", "f:Fl right return", "s:R flip power", "s:R flip hold", "s:L flip power", "s:L flip hold",
	"s:R post power", "s:R post hold", "s:L post power", "s:L post hold", nullptr, nullptr, nullptr, nullptr,
	nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"g:Insert top", "g:Insert middle", "g:Insert bottom"
};

static INPUT_PORTS_START( ttt )
	PORT_START("SW.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Start button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Plumb bob tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right sling")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Right post")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Slam tilt")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Coin door closed") PORT_TOGGLE PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left post")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Left sling")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole \"9\"")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole \"8\"")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole \"7\"")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole \"6\"")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole \"5\"")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Kicker opto")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole \"4\"")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole \"3\"")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole \"2\"")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Hole \"1\"")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ticket opto")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Tickets low")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Ticket test")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.5")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.6")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SW.7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DOOR")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("Left coin chute")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("Center coin chute")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("Right coin chute")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("4th coin chute")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1) PORT_NAME("Service credit/Escape")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Down/Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Volume Up/Up") PORT_CODE(KEYCODE_UP)
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_HIGH ) PORT_NAME("Begin test/Enter")

	PORT_START("DSW")
	PORT_DIPNAME(0xff, 0xfc, "Country") PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0xfc, "America")
	PORT_DIPSETTING(   0xdc, "European")
	PORT_DIPSETTING(   0x3c, "French")
	PORT_DIPSETTING(   0x7c, "German")
	PORT_DIPSETTING(   0xec, "Spain")

	PORT_START("FLIPPERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper EOS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("R Flipper Button")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper EOS")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("L Flipper Button")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UR Flipper Button")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("UL Flipper Button")
INPUT_PORTS_END

static MACHINE_CONFIG_START( wpc_95, wpc_95_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, XTAL_8MHz/4)
	MCFG_CPU_PROGRAM_MAP(wpc_95_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(wpc_95_state, irq0_line_assert, XTAL_8MHz/8192.0)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("zero_crossing", wpc_95_state, zc_timer, attotime::from_hz(120)) // Mains power zero crossing

	MCFG_WPC_PIC_ADD("pic")
	MCFG_WPC_LAMP_ADD("lamp")
	MCFG_WPC_OUT_ADD("out", 3)
	MCFG_WPC_SHIFT_ADD("shift")
	MCFG_WPC_DMD_ADD("dmd", WRITELINE(wpc_95_state, scanline_irq))

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_DEVICE_ADD("dcs", DCS_AUDIO_WPC, 0)
MACHINE_CONFIG_END

/*-----------------
/  Attack From Mars #50041
/------------------*/
ROM_START(afm_11)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("mars1_1.rom", 0x00000, 0x080000, CRC(13b174d9) SHA1(57952f3184496b0316e4cf301e0181cb9de3519a))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("afm_s2.l1", 0x000000, 0x100000, CRC(6e39d96e) SHA1(b34e31bb1734c86614f153f7201163aaa9943cec))
	ROM_LOAD16_BYTE("afm_s3.l1", 0x200000, 0x100000, CRC(1cbce9b1) SHA1(7f258bfe1904a879a2cb007419483f4fee91e072))
	ROM_LOAD16_BYTE("afm_s4.l1", 0x400000, 0x100000, CRC(5ff7fbb7) SHA1(ebaf825d3b90b6acee1920e6703801a4bcddfc5b))
ROM_END

ROM_START(afm_10)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("afm_1_00.g11", 0x00000, 0x080000, CRC(1a30fe95) SHA1(218674e63ce4efeecb266f35f0f315758f7c72fc))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("afm_1_00.s2", 0x000000, 0x100000, CRC(610ff107) SHA1(9590f809a05cb2bda4979fa16f165e2e994719a0))
	ROM_LOAD16_BYTE("afm_s3.l1", 0x200000, 0x100000, CRC(1cbce9b1) SHA1(7f258bfe1904a879a2cb007419483f4fee91e072))
	ROM_LOAD16_BYTE("afm_s4.l1", 0x400000, 0x100000, CRC(5ff7fbb7) SHA1(ebaf825d3b90b6acee1920e6703801a4bcddfc5b))
ROM_END

ROM_START(afm_113)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("afm_1_13.bin", 0x000000, 0x100000, CRC(e1fbd81b) SHA1(0ff35253d8eac7b75abb3e4db84cdcca458182cd))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("afm_s2.l1", 0x000000, 0x100000, CRC(6e39d96e) SHA1(b34e31bb1734c86614f153f7201163aaa9943cec))
	ROM_LOAD16_BYTE("afm_s3.l1", 0x200000, 0x100000, CRC(1cbce9b1) SHA1(7f258bfe1904a879a2cb007419483f4fee91e072))
	ROM_LOAD16_BYTE("afm_s4.l1", 0x400000, 0x100000, CRC(5ff7fbb7) SHA1(ebaf825d3b90b6acee1920e6703801a4bcddfc5b))
ROM_END

ROM_START(afm_113b)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("afm_113b.bin", 0x00000, 0x100000, CRC(34fd2d7d) SHA1(57a41bd686286429880e63696d7d9d3990ca5d05))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("afm_s2.l1", 0x000000, 0x100000, CRC(6e39d96e) SHA1(b34e31bb1734c86614f153f7201163aaa9943cec))
	ROM_LOAD16_BYTE("afm_s3.l1", 0x200000, 0x100000, CRC(1cbce9b1) SHA1(7f258bfe1904a879a2cb007419483f4fee91e072))
	ROM_LOAD16_BYTE("afm_s4.l1", 0x400000, 0x100000, CRC(5ff7fbb7) SHA1(ebaf825d3b90b6acee1920e6703801a4bcddfc5b))
ROM_END

ROM_START(afm_11u)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("marsu1_1.rom", 0x00000, 0x080000, CRC(bc1c0a0a) SHA1(859b40aaae46623f9b3519b5e422977d1724b715))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("afm_s2.l1", 0x000000, 0x100000, CRC(6e39d96e) SHA1(b34e31bb1734c86614f153f7201163aaa9943cec))
	ROM_LOAD16_BYTE("afm_s3.l1", 0x200000, 0x100000, CRC(1cbce9b1) SHA1(7f258bfe1904a879a2cb007419483f4fee91e072))
	ROM_LOAD16_BYTE("afm_s4.l1", 0x400000, 0x100000, CRC(5ff7fbb7) SHA1(ebaf825d3b90b6acee1920e6703801a4bcddfc5b))
ROM_END

/*-----------------
/  Cactus Canyon #50066
/------------------*/
ROM_START(cc_12)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cc_g11.1_2", 0x00000, 0x100000, CRC(17ad9266) SHA1(b18c4e2cc9f4269904c05e5e414675a94f96e955))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("sav2_8.rom", 0x000000, 0x100000, CRC(94928841) SHA1(953586d6abe8222a6cd6b74e417fa4ce078efa43))
	ROM_LOAD16_BYTE("sav3_8.rom", 0x200000, 0x100000, CRC(a22b13f0) SHA1(5df6ea9d5059cd04bdb369c1c7255b09d64b3c65))
	ROM_LOAD16_BYTE("sav4_8.rom", 0x400000, 0x100000, CRC(fe8324e2) SHA1(72c56d094cb4185a083a7da81fd527a908ce9de0))
	ROM_LOAD16_BYTE("sav5_8.rom", 0x600000, 0x100000, CRC(1b2a1ff3) SHA1(2d9a5952c7ac000c47d87d198ff7ca62913ec73f))
	ROM_LOAD16_BYTE("sav6_8.rom", 0x800000, 0x100000, CRC(2cccf10e) SHA1(3b9b9c87ab3c0d74eaacde416d18f3357f8302bd))
	ROM_LOAD16_BYTE("sav7_8.rom", 0xa00000, 0x100000, CRC(90fb1277) SHA1(502c920e1d54d285a4d4af401e574f785149da47))
ROM_END

ROM_START(cc_13)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cc_g11.1_3", 0x00000, 0x100000, CRC(7741fa4e) SHA1(adaf6b07d2f2714e87e367db28d15ae0145b6ae6))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("sav2_8.rom", 0x000000, 0x100000, CRC(94928841) SHA1(953586d6abe8222a6cd6b74e417fa4ce078efa43))
	ROM_LOAD16_BYTE("sav3_8.rom", 0x200000, 0x100000, CRC(a22b13f0) SHA1(5df6ea9d5059cd04bdb369c1c7255b09d64b3c65))
	ROM_LOAD16_BYTE("sav4_8.rom", 0x400000, 0x100000, CRC(fe8324e2) SHA1(72c56d094cb4185a083a7da81fd527a908ce9de0))
	ROM_LOAD16_BYTE("sav5_8.rom", 0x600000, 0x100000, CRC(1b2a1ff3) SHA1(2d9a5952c7ac000c47d87d198ff7ca62913ec73f))
	ROM_LOAD16_BYTE("sav6_8.rom", 0x800000, 0x100000, CRC(2cccf10e) SHA1(3b9b9c87ab3c0d74eaacde416d18f3357f8302bd))
	ROM_LOAD16_BYTE("sav7_8.rom", 0xa00000, 0x100000, CRC(90fb1277) SHA1(502c920e1d54d285a4d4af401e574f785149da47))
ROM_END

ROM_START(cc_10)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cc_g11.1_0", 0x00000, 0x100000, CRC(c4e2e838) SHA1(3223dd03353dead0f41626b04c9f019d6fe1528c))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("sav2_8.rom", 0x000000, 0x100000, CRC(94928841) SHA1(953586d6abe8222a6cd6b74e417fa4ce078efa43))
	ROM_LOAD16_BYTE("sav3_8.rom", 0x200000, 0x100000, CRC(a22b13f0) SHA1(5df6ea9d5059cd04bdb369c1c7255b09d64b3c65))
	ROM_LOAD16_BYTE("sav4_8.rom", 0x400000, 0x100000, CRC(fe8324e2) SHA1(72c56d094cb4185a083a7da81fd527a908ce9de0))
	ROM_LOAD16_BYTE("sav5_8.rom", 0x600000, 0x100000, CRC(1b2a1ff3) SHA1(2d9a5952c7ac000c47d87d198ff7ca62913ec73f))
	ROM_LOAD16_BYTE("sav6_8.rom", 0x800000, 0x100000, CRC(2cccf10e) SHA1(3b9b9c87ab3c0d74eaacde416d18f3357f8302bd))
	ROM_LOAD16_BYTE("sav7_8.rom", 0xa00000, 0x100000, CRC(90fb1277) SHA1(502c920e1d54d285a4d4af401e574f785149da47))
ROM_END

ROM_START(cc_104)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cc_g11.104", 0x00000, 0x100000, CRC(21a7b816) SHA1(0e67da694b8713e15e04bf0c49a48e14f057a737))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("sav2_8.rom", 0x000000, 0x100000, CRC(94928841) SHA1(953586d6abe8222a6cd6b74e417fa4ce078efa43))
	ROM_LOAD16_BYTE("sav3_8.rom", 0x200000, 0x100000, CRC(a22b13f0) SHA1(5df6ea9d5059cd04bdb369c1c7255b09d64b3c65))
	ROM_LOAD16_BYTE("sav4_8.rom", 0x400000, 0x100000, CRC(fe8324e2) SHA1(72c56d094cb4185a083a7da81fd527a908ce9de0))
	ROM_LOAD16_BYTE("sav5_8.rom", 0x600000, 0x100000, CRC(1b2a1ff3) SHA1(2d9a5952c7ac000c47d87d198ff7ca62913ec73f))
	ROM_LOAD16_BYTE("sav6_8.rom", 0x800000, 0x100000, CRC(2cccf10e) SHA1(3b9b9c87ab3c0d74eaacde416d18f3357f8302bd))
	ROM_LOAD16_BYTE("sav7_8.rom", 0xa00000, 0x100000, CRC(90fb1277) SHA1(502c920e1d54d285a4d4af401e574f785149da47))
ROM_END

/*-----------------
/  Cirqus Voltaire #50062
/------------------*/
ROM_START(cv_14)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cirq_14.rom", 0x00000, 0x100000, CRC(7a8bf999) SHA1(b33baabf4f6cbf8615cc00eb1286238c5aea386a))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD("s2v1_0.rom", 0x000000, 0x080000, CRC(79dbb8ee) SHA1(f76c0db93b89beaf1e90c5f2199262e296fb1b78))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("s3v0_4.rom", 0x200000, 0x100000, CRC(8c6c0c56) SHA1(792431cc5b06c3d5028168297614f5eb7e8af34f))
	ROM_LOAD16_BYTE("s4v0_4.rom", 0x400000, 0x100000, CRC(a9014b78) SHA1(abffe32ab729fb39ab2360d850c8b5476094fd92))
	ROM_LOAD16_BYTE("s5v0_4.rom", 0x600000, 0x100000, CRC(7e07a2fc) SHA1(f908363c968c15c0dc62e32695e5e2d0ca869391))
	ROM_LOAD16_BYTE("s6v0_4.rom", 0x800000, 0x100000, CRC(36ca43d3) SHA1(b599f88649c220143aa44cd5213e725e62afb0bc))
ROM_END

ROM_START(cv_20h)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cv200h.rom", 0x00000, 0x100000, CRC(138a0c3c) SHA1(dd6d4b5519ca161bd6779ed60cc7f52542a10147))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("s2v1_0.rom", 0x000000, 0x080000, CRC(79dbb8ee) SHA1(f76c0db93b89beaf1e90c5f2199262e296fb1b78))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("s3v0_4.rom", 0x200000, 0x100000, CRC(8c6c0c56) SHA1(792431cc5b06c3d5028168297614f5eb7e8af34f))
	ROM_LOAD16_BYTE("s4v0_4.rom", 0x400000, 0x100000, CRC(a9014b78) SHA1(abffe32ab729fb39ab2360d850c8b5476094fd92))
	ROM_LOAD16_BYTE("s5v0_4.rom", 0x600000, 0x100000, CRC(7e07a2fc) SHA1(f908363c968c15c0dc62e32695e5e2d0ca869391))
	ROM_LOAD16_BYTE("s6v0_4.rom", 0x800000, 0x100000, CRC(36ca43d3) SHA1(b599f88649c220143aa44cd5213e725e62afb0bc))
ROM_END

ROM_START(cv_10)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11_100.rom", 0x00000, 0x100000, CRC(00028589) SHA1(46639c45abbdc59ca0f861824eca3efa10547123))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("s2v1_0.rom", 0x000000, 0x080000, CRC(79dbb8ee) SHA1(f76c0db93b89beaf1e90c5f2199262e296fb1b78))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("s3v0_4.rom", 0x200000, 0x100000, CRC(8c6c0c56) SHA1(792431cc5b06c3d5028168297614f5eb7e8af34f))
	ROM_LOAD16_BYTE("s4v0_4.rom", 0x400000, 0x100000, CRC(a9014b78) SHA1(abffe32ab729fb39ab2360d850c8b5476094fd92))
	ROM_LOAD16_BYTE("s5v0_4.rom", 0x600000, 0x100000, CRC(7e07a2fc) SHA1(f908363c968c15c0dc62e32695e5e2d0ca869391))
	ROM_LOAD16_BYTE("s6v0_4.rom", 0x800000, 0x100000, CRC(36ca43d3) SHA1(b599f88649c220143aa44cd5213e725e62afb0bc))
ROM_END

ROM_START(cv_11)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11_110.rom", 0x00000, 0x100000, CRC(c7a4c104) SHA1(a96d34b2cf94591879de5b7838db0c98c9abfad8))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("s2v1_0.rom", 0x000000, 0x080000, CRC(79dbb8ee) SHA1(f76c0db93b89beaf1e90c5f2199262e296fb1b78))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("s3v0_4.rom", 0x200000, 0x100000, CRC(8c6c0c56) SHA1(792431cc5b06c3d5028168297614f5eb7e8af34f))
	ROM_LOAD16_BYTE("s4v0_4.rom", 0x400000, 0x100000, CRC(a9014b78) SHA1(abffe32ab729fb39ab2360d850c8b5476094fd92))
	ROM_LOAD16_BYTE("s5v0_4.rom", 0x600000, 0x100000, CRC(7e07a2fc) SHA1(f908363c968c15c0dc62e32695e5e2d0ca869391))
	ROM_LOAD16_BYTE("s6v0_4.rom", 0x800000, 0x100000, CRC(36ca43d3) SHA1(b599f88649c220143aa44cd5213e725e62afb0bc))
ROM_END

ROM_START(cv_13)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cv_g11.1_3", 0x00000, 0x100000, CRC(58b3bea0) SHA1(243f15c6b383921faf735caece2073cb6f88601a))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD("s2v1_0.rom", 0x000000, 0x080000, CRC(79dbb8ee) SHA1(f76c0db93b89beaf1e90c5f2199262e296fb1b78))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("s3v0_4.rom", 0x200000, 0x100000, CRC(8c6c0c56) SHA1(792431cc5b06c3d5028168297614f5eb7e8af34f))
	ROM_LOAD16_BYTE("s4v0_4.rom", 0x400000, 0x100000, CRC(a9014b78) SHA1(abffe32ab729fb39ab2360d850c8b5476094fd92))
	ROM_LOAD16_BYTE("s5v0_4.rom", 0x600000, 0x100000, CRC(7e07a2fc) SHA1(f908363c968c15c0dc62e32695e5e2d0ca869391))
	ROM_LOAD16_BYTE("s6v0_4.rom", 0x800000, 0x100000, CRC(36ca43d3) SHA1(b599f88649c220143aa44cd5213e725e62afb0bc))
ROM_END

/*-----------------
/  Congo #50050
/------------------*/
ROM_START(congo_21)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cg_g11.2_1", 0x00000, 0x80000, CRC(5d8435bf) SHA1(1356758fd788bbb3c7ab29abaaea7d2baac75f55))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("cgs2v1_1.rom", 0x000000, 0x100000, CRC(2b7637ae) SHA1(5b5d7214c632a506b986c892b39b1356b2909598))
	ROM_LOAD16_BYTE("cgs3v1_0.rom", 0x200000, 0x100000, CRC(6cfd9fe0) SHA1(a76267f865c645648c8cb27aec2d05062a4a20b5))
	ROM_LOAD16_BYTE("cgs4v1_0.rom", 0x400000, 0x100000, CRC(2a1980e7) SHA1(0badf27c2b8bc7b0074dc5e606d64490470bc108))
ROM_END

ROM_START(congo_20)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cong2_00.rom", 0x00000, 0x80000, CRC(e1a256ac) SHA1(f1f7a1865b5a0220e2f2ef492059df158451ca5b))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("cgs2v1_1.rom", 0x000000, 0x100000, CRC(2b7637ae) SHA1(5b5d7214c632a506b986c892b39b1356b2909598))
	ROM_LOAD16_BYTE("cgs3v1_0.rom", 0x200000, 0x100000, CRC(6cfd9fe0) SHA1(a76267f865c645648c8cb27aec2d05062a4a20b5))
	ROM_LOAD16_BYTE("cgs4v1_0.rom", 0x400000, 0x100000, CRC(2a1980e7) SHA1(0badf27c2b8bc7b0074dc5e606d64490470bc108))
ROM_END

ROM_START(congo_13)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cong1_30.rom", 0x00000, 0x80000, CRC(e68c0404) SHA1(e851f42e6bd0e910fc87b9500cbacac3c088b488))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("cgs2v1_1.rom", 0x000000, 0x100000, CRC(2b7637ae) SHA1(5b5d7214c632a506b986c892b39b1356b2909598))
	ROM_LOAD16_BYTE("cgs3v1_0.rom", 0x200000, 0x100000, CRC(6cfd9fe0) SHA1(a76267f865c645648c8cb27aec2d05062a4a20b5))
	ROM_LOAD16_BYTE("cgs4v1_0.rom", 0x400000, 0x100000, CRC(2a1980e7) SHA1(0badf27c2b8bc7b0074dc5e606d64490470bc108))
ROM_END

ROM_START(congo_11)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cong1_10.rom", 0x00000, 0x80000, CRC(b0b0ffd9) SHA1(26343f3bfbacf85b3f4db5aa3dad39216311a2da))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("cgs2v1_1.rom", 0x000000, 0x100000, CRC(2b7637ae) SHA1(5b5d7214c632a506b986c892b39b1356b2909598))
	ROM_LOAD16_BYTE("cgs3v1_0.rom", 0x200000, 0x100000, CRC(6cfd9fe0) SHA1(a76267f865c645648c8cb27aec2d05062a4a20b5))
	ROM_LOAD16_BYTE("cgs4v1_0.rom", 0x400000, 0x100000, CRC(2a1980e7) SHA1(0badf27c2b8bc7b0074dc5e606d64490470bc108))
ROM_END

/*-----------------
/  Junk Yard #50052
/------------------*/
ROM_START(jy_12)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("jy_g11.1_2", 0x00000, 0x80000, CRC(16fb4bb3) SHA1(fbd8b37c129f7e07e8c44b7a33754ee346473377))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("jy_s2.rom", 0x000000, 0x080000, CRC(1a1bc2ca) SHA1(db949d49560a26fc280cd9e746aa99dfafbd6daa))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("jy_s3.rom", 0x200000, 0x100000, CRC(0fc36a8e) SHA1(335013ebe08d34a24b0b472c6d5f042e455facee))
	ROM_LOAD16_BYTE("jy_s4.rom", 0x400000, 0x100000, CRC(0aebcd77) SHA1(62aee2685c0ae4bc1df8e4a4515ca34a078c72ad))
	ROM_LOAD16_BYTE("jy_s5.rom", 0x600000, 0x100000, CRC(f18ad10b) SHA1(1d02a388b43d3863030e01bf567f30337d37b2e8))
ROM_END

ROM_START(jy_11)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("jy_g11.1_1", 0x00000, 0x80000, CRC(2810fcb9) SHA1(58bb828e4d37a0ac65108a4dfb4ba25615b2b6f7))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("jy_s2.rom", 0x000000, 0x080000, CRC(1a1bc2ca) SHA1(db949d49560a26fc280cd9e746aa99dfafbd6daa))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("jy_s3.rom", 0x200000, 0x100000, CRC(0fc36a8e) SHA1(335013ebe08d34a24b0b472c6d5f042e455facee))
	ROM_LOAD16_BYTE("jy_s4.rom", 0x400000, 0x100000, CRC(0aebcd77) SHA1(62aee2685c0ae4bc1df8e4a4515ca34a078c72ad))
	ROM_LOAD16_BYTE("jy_s5.rom", 0x600000, 0x100000, CRC(f18ad10b) SHA1(1d02a388b43d3863030e01bf567f30337d37b2e8))
ROM_END

ROM_START(jy_03)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("jy_cpu.0_3", 0x00000, 0x80000, CRC(015d0253) SHA1(733740645fb300f48f57a74dea5fa31758628d24))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("jy_s2.0_2", 0x000000, 0x100000, CRC(8a6e0eaa) SHA1(758609b946d10dca70c46da0403d8ed36f8cbd5c))
	ROM_LOAD16_BYTE("jy_s3.0_2", 0x200000, 0x100000, CRC(53987d09) SHA1(c1f34d564a8f69413878a7adc089181e562a347c))
	ROM_LOAD16_BYTE("jy_s4.0_2", 0x400000, 0x100000, CRC(f82481cd) SHA1(a8283ac4a2dee636f4ec17d3fddf09920c7e3802))
	ROM_LOAD16_BYTE("jy_s5.0_2", 0x600000, 0x100000, CRC(5adb2d4c) SHA1(566a27238d643aaf7764e23bc1ce46cc5d7883dd))
ROM_END

/*-----------------
/  Medieval Madness #50059
/------------------*/
ROM_START(mm_109)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("mm_1_09.bin", 0x00000, 0x100000, CRC(9bac4d0c) SHA1(92cbe21802e1a77feff77b78f4dbbdbffb7b14bc))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("mm_s2.1_0", 0x000000, 0x080000, CRC(c55c3b71) SHA1(95febbf16645dd897bdd459ccad9501d2457d1f1))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("mm_sav3.rom", 0x200000, 0x100000, CRC(ed1be570) SHA1(ead4c4f89d63ee0b46d8a8bcd8650d506542d1ee))
	ROM_LOAD16_BYTE("mm_sav4.rom", 0x400000, 0x100000, CRC(9c89eacf) SHA1(594a2aa81e34658862a9b7f0a83cf514182f2a2d))
	ROM_LOAD16_BYTE("mm_sav5.rom", 0x600000, 0x100000, CRC(45089e30) SHA1(e83492109c59e8a2f1ba9e1f793788b97d150a9b))
	ROM_LOAD16_BYTE("mm_sav6.rom", 0x800000, 0x100000, CRC(439d55f2) SHA1(d80e7268223157d864674261d140322634fb3bc2))
ROM_END

ROM_START(mm_109b)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("mm_109b.bin", 0x00000, 0x100000, CRC(4eaab86a) SHA1(694cbb1154e7374275becfbe4f743fb8d31df8fb))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("mm_s2.1_0", 0x000000, 0x080000, CRC(c55c3b71) SHA1(95febbf16645dd897bdd459ccad9501d2457d1f1))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("mm_sav3.rom", 0x200000, 0x100000, CRC(ed1be570) SHA1(ead4c4f89d63ee0b46d8a8bcd8650d506542d1ee))
	ROM_LOAD16_BYTE("mm_sav4.rom", 0x400000, 0x100000, CRC(9c89eacf) SHA1(594a2aa81e34658862a9b7f0a83cf514182f2a2d))
	ROM_LOAD16_BYTE("mm_sav5.rom", 0x600000, 0x100000, CRC(45089e30) SHA1(e83492109c59e8a2f1ba9e1f793788b97d150a9b))
	ROM_LOAD16_BYTE("mm_sav6.rom", 0x800000, 0x100000, CRC(439d55f2) SHA1(d80e7268223157d864674261d140322634fb3bc2))
ROM_END

ROM_START(mm_109c)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("mm_1_09c.bin", 0x00000, 0x100000, CRC(d9e5189f) SHA1(fc01855c139d408559605fe9932236250cd566a8))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("mm_s2.1_0", 0x000000, 0x080000, CRC(c55c3b71) SHA1(95febbf16645dd897bdd459ccad9501d2457d1f1))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("mm_sav3.rom", 0x200000, 0x100000, CRC(ed1be570) SHA1(ead4c4f89d63ee0b46d8a8bcd8650d506542d1ee))
	ROM_LOAD16_BYTE("mm_sav4.rom", 0x400000, 0x100000, CRC(9c89eacf) SHA1(594a2aa81e34658862a9b7f0a83cf514182f2a2d))
	ROM_LOAD16_BYTE("mm_sav5.rom", 0x600000, 0x100000, CRC(45089e30) SHA1(e83492109c59e8a2f1ba9e1f793788b97d150a9b))
	ROM_LOAD16_BYTE("mm_sav6.rom", 0x800000, 0x100000, CRC(439d55f2) SHA1(d80e7268223157d864674261d140322634fb3bc2))
ROM_END

ROM_START(mm_10)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("mm_g11.1_0", 0x00000, 0x080000, CRC(6bd735c6) SHA1(3922df00e785610837230d5d9c24b9e082aa6fb6))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("mm_s2.1_0", 0x000000, 0x080000, CRC(c55c3b71) SHA1(95febbf16645dd897bdd459ccad9501d2457d1f1))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("mm_sav3.rom", 0x200000, 0x100000, CRC(ed1be570) SHA1(ead4c4f89d63ee0b46d8a8bcd8650d506542d1ee))
	ROM_LOAD16_BYTE("mm_sav4.rom", 0x400000, 0x100000, CRC(9c89eacf) SHA1(594a2aa81e34658862a9b7f0a83cf514182f2a2d))
	ROM_LOAD16_BYTE("mm_sav5.rom", 0x600000, 0x100000, CRC(45089e30) SHA1(e83492109c59e8a2f1ba9e1f793788b97d150a9b))
	ROM_LOAD16_BYTE("mm_sav6.rom", 0x800000, 0x100000, CRC(439d55f2) SHA1(d80e7268223157d864674261d140322634fb3bc2))
ROM_END

ROM_START(mm_10u)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("mmu_g11.1_0", 0x00000, 0x080000, CRC(265e6192) SHA1(bd9606df5fb85b2048a07db0927e4a856c344276))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("mm_s2.1_0", 0x000000, 0x080000, CRC(c55c3b71) SHA1(95febbf16645dd897bdd459ccad9501d2457d1f1))
	ROM_RELOAD(0x000000 +0x100000, 0x080000)
	ROM_LOAD16_BYTE("mm_sav3.rom", 0x200000, 0x100000, CRC(ed1be570) SHA1(ead4c4f89d63ee0b46d8a8bcd8650d506542d1ee))
	ROM_LOAD16_BYTE("mm_sav4.rom", 0x400000, 0x100000, CRC(9c89eacf) SHA1(594a2aa81e34658862a9b7f0a83cf514182f2a2d))
	ROM_LOAD16_BYTE("mm_sav5.rom", 0x600000, 0x100000, CRC(45089e30) SHA1(e83492109c59e8a2f1ba9e1f793788b97d150a9b))
	ROM_LOAD16_BYTE("mm_sav6.rom", 0x800000, 0x100000, CRC(439d55f2) SHA1(d80e7268223157d864674261d140322634fb3bc2))
ROM_END

ROM_START(mm_05)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11-050.rom", 0x00000, 0x080000, CRC(d211ad16) SHA1(539fb0c4ca6fe19ac6140f5792c5b7cd51f737ce))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("s2-020.rom", 0x000000, 0x080000, CRC(ee009ce4) SHA1(36843b2f1a07cf1e23bdff9b7347ceeca7e915bc))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("mm_sav3.rom", 0x200000, 0x100000, CRC(ed1be570) SHA1(ead4c4f89d63ee0b46d8a8bcd8650d506542d1ee))
	ROM_LOAD16_BYTE("mm_sav4.rom", 0x400000, 0x100000, CRC(9c89eacf) SHA1(594a2aa81e34658862a9b7f0a83cf514182f2a2d))
	ROM_LOAD16_BYTE("mm_sav5.rom", 0x600000, 0x100000, CRC(45089e30) SHA1(e83492109c59e8a2f1ba9e1f793788b97d150a9b))
	ROM_LOAD16_BYTE("mm_sav6.rom", 0x800000, 0x100000, CRC(439d55f2) SHA1(d80e7268223157d864674261d140322634fb3bc2))
ROM_END

/*-----------------
/  Monster Bash #50065
/------------------*/
ROM_START(mb_106)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("mb_1_06.bin", 0x00000, 0x100000, CRC(381a8822) SHA1(b0b5bf58accff24a4023c102952c89c1f116a174))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("mb_s2.rom", 0x000000, 0x080000, CRC(9152f559) SHA1(68d455d8b875101caedafd21b59c447f326566dd))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("mb_s3.rom", 0x200000, 0x100000, CRC(c3cd6e81) SHA1(b041979c8955907090b30960780cecb19258bd5e))
	ROM_LOAD16_BYTE("mb_s4.rom", 0x400000, 0x100000, CRC(00b88352) SHA1(5da75e0b400eb71583681e06088eb97fc12e7f17))
	ROM_LOAD16_BYTE("mb_s5.rom", 0x600000, 0x100000, CRC(dae16105) SHA1(15878ef8685f3e9fc8eb2a2401581d30fe706e89))
	ROM_LOAD16_BYTE("mb_s6.rom", 0x800000, 0x100000, CRC(3975d5da) SHA1(6dbb34a827c0956e6aef1401c12cba88ae370e1f))
	ROM_LOAD16_BYTE("mb_s7.rom", 0xa00000, 0x100000, CRC(c242fb78) SHA1(c5a2a37ff3414d1e946cddb69b5e8f067b50bcc6))
ROM_END

ROM_START(mb_106b)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("mb_106b.bin", 0x00000, 0x100000, CRC(c7c5d855) SHA1(96a43a955c0abaef8d6af1b64eaf50a7eeb69fe0))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("mb_s2.rom", 0x000000, 0x080000, CRC(9152f559) SHA1(68d455d8b875101caedafd21b59c447f326566dd))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("mb_s3.rom", 0x200000, 0x100000, CRC(c3cd6e81) SHA1(b041979c8955907090b30960780cecb19258bd5e))
	ROM_LOAD16_BYTE("mb_s4.rom", 0x400000, 0x100000, CRC(00b88352) SHA1(5da75e0b400eb71583681e06088eb97fc12e7f17))
	ROM_LOAD16_BYTE("mb_s5.rom", 0x600000, 0x100000, CRC(dae16105) SHA1(15878ef8685f3e9fc8eb2a2401581d30fe706e89))
	ROM_LOAD16_BYTE("mb_s6.rom", 0x800000, 0x100000, CRC(3975d5da) SHA1(6dbb34a827c0956e6aef1401c12cba88ae370e1f))
	ROM_LOAD16_BYTE("mb_s7.rom", 0xa00000, 0x100000, CRC(c242fb78) SHA1(c5a2a37ff3414d1e946cddb69b5e8f067b50bcc6))
ROM_END

ROM_START(mb_10)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("mb_g11.1_0", 0x00000, 0x100000, CRC(6b8db967) SHA1(e24d801ed9d326b9d4ddb26100c85cfd8e697d17))
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("mb_s2.rom", 0x000000, 0x080000, CRC(9152f559) SHA1(68d455d8b875101caedafd21b59c447f326566dd))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("mb_s3.rom", 0x200000, 0x100000, CRC(c3cd6e81) SHA1(b041979c8955907090b30960780cecb19258bd5e))
	ROM_LOAD16_BYTE("mb_s4.rom", 0x400000, 0x100000, CRC(00b88352) SHA1(5da75e0b400eb71583681e06088eb97fc12e7f17))
	ROM_LOAD16_BYTE("mb_s5.rom", 0x600000, 0x100000, CRC(dae16105) SHA1(15878ef8685f3e9fc8eb2a2401581d30fe706e89))
	ROM_LOAD16_BYTE("mb_s6.rom", 0x800000, 0x100000, CRC(3975d5da) SHA1(6dbb34a827c0956e6aef1401c12cba88ae370e1f))
	ROM_LOAD16_BYTE("mb_s7.rom", 0xa00000, 0x100000, CRC(c242fb78) SHA1(c5a2a37ff3414d1e946cddb69b5e8f067b50bcc6))
ROM_END

/*-----------------
/  NBA Fastbreak #50053
/------------------*/
ROM_START(nbaf_31)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("fb_g11.3_1", 0x00000, 0x80000, CRC(acd84ec2) SHA1(bd641b26e7a577be9f8705b21de4a694400945ce))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("fb-s2.3_0", 0x000000, 0x100000, CRC(4594abd3) SHA1(d14654f0c2d29c28cae604e2dbcc9adf361b28a9))
	ROM_LOAD16_BYTE("fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648))
	ROM_LOAD16_BYTE("fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9))
	ROM_LOAD16_BYTE("fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d))
	ROM_LOAD16_BYTE("fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0))
ROM_END

ROM_START(nbaf_31a)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("fb_g11.3_1", 0x00000, 0x80000, CRC(acd84ec2) SHA1(bd641b26e7a577be9f8705b21de4a694400945ce))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("fb-s2.1_0", 0x000000, 0x080000, CRC(32f42a82) SHA1(387636c8e9f8525e7442ccdced735392db113044))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648))
	ROM_LOAD16_BYTE("fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9))
	ROM_LOAD16_BYTE("fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d))
	ROM_LOAD16_BYTE("fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0))
ROM_END

ROM_START(nbaf_11)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11-11.rom", 0x00000, 0x80000, CRC(debfb64a) SHA1(7f50246f5fde1e7fc295be6b6bbd455e244e4c99))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("fb-s2.1_0", 0x000000, 0x080000, CRC(32f42a82) SHA1(387636c8e9f8525e7442ccdced735392db113044))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648))
	ROM_LOAD16_BYTE("fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9))
	ROM_LOAD16_BYTE("fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d))
	ROM_LOAD16_BYTE("fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0))
ROM_END

ROM_START(nbaf_11a)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11-11.rom", 0x00000, 0x80000, CRC(debfb64a) SHA1(7f50246f5fde1e7fc295be6b6bbd455e244e4c99))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("fb-s2.2_0", 0x000000, 0x100000, CRC(f950f481) SHA1(8d7c54c5f27a85889179ee690512fa69b1357bb6))
	ROM_LOAD16_BYTE("fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648))
	ROM_LOAD16_BYTE("fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9))
	ROM_LOAD16_BYTE("fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d))
	ROM_LOAD16_BYTE("fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0))
ROM_END

ROM_START(nbaf_11s)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11-11.rom", 0x00000, 0x80000, CRC(debfb64a) SHA1(7f50246f5fde1e7fc295be6b6bbd455e244e4c99))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("fb-s2.0_4", 0x000000, 0x080000, CRC(6a96f42b) SHA1(b6019bccdf62c9cf044a88d35019ebf0593b24d7))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648))
	ROM_LOAD16_BYTE("fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9))
	ROM_LOAD16_BYTE("fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d))
	ROM_LOAD16_BYTE("fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0))
ROM_END

ROM_START(nbaf_115)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11-115", 0x00000, 0x80000, CRC(c0ed9848) SHA1(196d13cf93fe61db36d3bd936549210875a88948))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("fb-s2.1_0", 0x000000, 0x080000, CRC(32f42a82) SHA1(387636c8e9f8525e7442ccdced735392db113044))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648))
	ROM_LOAD16_BYTE("fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9))
	ROM_LOAD16_BYTE("fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d))
	ROM_LOAD16_BYTE("fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0))
ROM_END

ROM_START(nbaf_21)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11-21.rom", 0x00000, 0x80000, CRC(598d33d0) SHA1(98c2bfcca573a6e790a4d3ba306953ff0fb3b042))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("fb-s2.1_0", 0x000000, 0x080000, CRC(32f42a82) SHA1(387636c8e9f8525e7442ccdced735392db113044))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648))
	ROM_LOAD16_BYTE("fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9))
	ROM_LOAD16_BYTE("fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d))
	ROM_LOAD16_BYTE("fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0))
ROM_END

ROM_START(nbaf_22)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11-22.rom", 0x00000, 0x80000, CRC(2e7a9685) SHA1(2af250a947089469c942cf2c570063bdebd4abe4))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("fb-s2.1_0", 0x000000, 0x080000, CRC(32f42a82) SHA1(387636c8e9f8525e7442ccdced735392db113044))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648))
	ROM_LOAD16_BYTE("fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9))
	ROM_LOAD16_BYTE("fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d))
	ROM_LOAD16_BYTE("fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0))
ROM_END

ROM_START(nbaf_23)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11-23.rom", 0x00000, 0x80000, CRC(a6ceb6de) SHA1(055387ee7da57e1a8fbce803a0dd9e67d6dbb1bd))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("fb-s2.1_0", 0x000000, 0x080000, CRC(32f42a82) SHA1(387636c8e9f8525e7442ccdced735392db113044))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("fb-s3.1_0", 0x200000, 0x100000, CRC(033aa54a) SHA1(9221f3013f204a9a857aced5d774c606a7e48648))
	ROM_LOAD16_BYTE("fb-s4.1_0", 0x400000, 0x100000, CRC(6965a7c5) SHA1(7e72bbd3bad9accc8da1754c57c24ebdf13e57b9))
	ROM_LOAD16_BYTE("fb-s5.1_0", 0x600000, 0x100000, CRC(db50b79a) SHA1(9753d599cd822b55ed64bcf64955f625dc51997d))
	ROM_LOAD16_BYTE("fb-s6.1_0", 0x800000, 0x100000, CRC(f1633371) SHA1(a707748d3298ffb6d10d8308f4dae7982b540fa0))
ROM_END

/*-----------------
/  No Good Gofers #50061
/------------------*/
ROM_START(ngg_13)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("go_g11.1_3", 0x00000, 0x80000, CRC(64e73117) SHA1(ce7ba5a6d309677e51dcbc9e3058f98e69d1e917))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("nggsndl1.s2", 0x000000, 0x080000, CRC(6263866d) SHA1(c72a2f176aa24e91ecafe1704affd16b86d671e2))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("nggsndl1.s3", 0x200000, 0x100000, CRC(6871b20d) SHA1(0109c02282806016a6b22f7dfe3ac964931ba609))
	ROM_LOAD16_BYTE("nggsndl1.s4", 0x400000, 0x100000, CRC(86ed8f5a) SHA1(231f6313adff89ef4cec0d9f25b13e69ea96213d))
	ROM_LOAD16_BYTE("nggsndl1.s5", 0x600000, 0x100000, CRC(ea2062f0) SHA1(f8e45c1fcc6b8677a0745a5d83ca93b77fbde752))
	ROM_LOAD16_BYTE("nggsndl1.s6", 0x800000, 0x100000, CRC(b1b8b514) SHA1(e16651bcb2eae747987dc3c13a5dc20a33c0a1f8))
ROM_END

ROM_START(ngg_10)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("ngg_10.rom", 0x00000, 0x80000, CRC(6680f6c1) SHA1(8ac37e3ea427c998f84a0c9c55e3f1e1da395870))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("nggsndl1.s2", 0x000000, 0x080000, CRC(6263866d) SHA1(c72a2f176aa24e91ecafe1704affd16b86d671e2))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("nggsndl1.s3", 0x200000, 0x100000, CRC(6871b20d) SHA1(0109c02282806016a6b22f7dfe3ac964931ba609))
	ROM_LOAD16_BYTE("nggsndl1.s4", 0x400000, 0x100000, CRC(86ed8f5a) SHA1(231f6313adff89ef4cec0d9f25b13e69ea96213d))
	ROM_LOAD16_BYTE("nggsndl1.s5", 0x600000, 0x100000, CRC(ea2062f0) SHA1(f8e45c1fcc6b8677a0745a5d83ca93b77fbde752))
	ROM_LOAD16_BYTE("nggsndl1.s6", 0x800000, 0x100000, CRC(b1b8b514) SHA1(e16651bcb2eae747987dc3c13a5dc20a33c0a1f8))
ROM_END

ROM_START(ngg_p06)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("ngg0_6.rom", 0x00000, 0x80000, CRC(e0e0d331) SHA1(e1b91eccec6034bcd2029c15596aa0b129c9e53f))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("ngg_s2.0_2", 0x000000, 0x080000, CRC(dde128d5) SHA1(214ee807d2323ecb407a3d116b038e15c60e5580))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("nggsndl1.s3", 0x200000, 0x100000, CRC(6871b20d) SHA1(0109c02282806016a6b22f7dfe3ac964931ba609))
	ROM_LOAD16_BYTE("nggsndl1.s4", 0x400000, 0x100000, CRC(86ed8f5a) SHA1(231f6313adff89ef4cec0d9f25b13e69ea96213d))
	ROM_LOAD16_BYTE("nggsndl1.s5", 0x600000, 0x100000, CRC(ea2062f0) SHA1(f8e45c1fcc6b8677a0745a5d83ca93b77fbde752))
	ROM_LOAD16_BYTE("nggsndl1.s6", 0x800000, 0x100000, CRC(b1b8b514) SHA1(e16651bcb2eae747987dc3c13a5dc20a33c0a1f8))
ROM_END

/*-----------------
/  Safe Cracker #90003
/------------------*/
ROM_START(sc_18)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("safe_18g.rom", 0x00000, 0x80000, CRC(aeb4b669) SHA1(2925eb11133526ddff8ae92bb53f9b45c6ed8134))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54))
	ROM_LOAD16_BYTE("safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731))
	ROM_LOAD16_BYTE("safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d))
ROM_END

ROM_START(sc_18n)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("safe_18n.rom", 0x00000, 0x80000, CRC(4d5d5626) SHA1(2d6f201d47f24df2195f10267ec1426cf0a087c9))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54))
	ROM_LOAD16_BYTE("safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731))
	ROM_LOAD16_BYTE("safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d))
ROM_END

ROM_START(sc_18s2)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("safe_18g.rom", 0x00000, 0x80000, CRC(aeb4b669) SHA1(2925eb11133526ddff8ae92bb53f9b45c6ed8134))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("su2-24g.rom", 0x000000, 0x100000, CRC(712ce42e) SHA1(5d3b8e3eccdd1bc09a92de610161dd51293181b1))
	ROM_LOAD16_BYTE("safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731))
	ROM_LOAD16_BYTE("safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d))
ROM_END

ROM_START(sc_17)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11-17g.rom", 0x00000, 0x80000, CRC(f3d64156) SHA1(9226664b59c7b65ac39e2f32597efc45672cf505))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54))
	ROM_LOAD16_BYTE("safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731))
	ROM_LOAD16_BYTE("safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d))
ROM_END

ROM_START(sc_17n)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11-17n.rom", 0x00000, 0x80000, CRC(97628907) SHA1(3435f496e1850bf433add1bc403e3148de05c13a))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54))
	ROM_LOAD16_BYTE("safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731))
	ROM_LOAD16_BYTE("safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d))
ROM_END

ROM_START(sc_14)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11-14.rom", 0x00000, 0x80000, CRC(1103f976) SHA1(6d6d23af1cd03f63b94a0ceb9711be51dce202f8))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54))
	ROM_LOAD16_BYTE("safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731))
	ROM_LOAD16_BYTE("safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d))
ROM_END

ROM_START(sc_091)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("sc_091.bin", 0x00000, 0x80000, CRC(b6f5307b) SHA1(93fab74db3aa62c2dd70d3a1d5664716c6548284))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("safsnds2.rom", 0x000000, 0x100000, CRC(20e14c63) SHA1(61b1c000a7afe5d0e9c31093e3fa963d6a594d54))
	ROM_LOAD16_BYTE("safsnds3.rom", 0x200000, 0x100000, CRC(99e318e7) SHA1(918f9013da82b29a559cb474bce93fb4ce88b731))
	ROM_LOAD16_BYTE("safsnds4.rom", 0x400000, 0x100000, CRC(9c8a23eb) SHA1(a0ee1174c8af0f262f9bec950da588cc9eb8747d))
ROM_END

/*-----------------
/  Scared Stiff #50048
/------------------*/
ROM_START(ss_15)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("ss_g11.1_5", 0x00000, 0x80000, CRC(5de8d0a0) SHA1(91cdd5f4e1654fd4dbde8b9cb03db935cba5d876))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("sssnd_11.s2", 0x000000, 0x100000, CRC(1b080a72) SHA1(be80e99e6bcc482379fe99519a4122f3b225be30))
	ROM_LOAD16_BYTE("sssnd_11.s3", 0x200000, 0x100000, CRC(c4f2e08a) SHA1(e20ff622a3f475db11f1f44d36a6669e160437a3))
	ROM_LOAD16_BYTE("sssnd_11.s4", 0x400000, 0x100000, CRC(258b0a27) SHA1(83763b98907cf38e6f7b9fe4f26ce93a54ba3568))
ROM_END

ROM_START(ss_14)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("stiffg11.1_4", 0x00000, 0x80000, CRC(17359ed6) SHA1(2ae549064a3666ea8b0b09aff9f5551db906d1d2))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("sssnd_11.s2", 0x000000, 0x100000, CRC(1b080a72) SHA1(be80e99e6bcc482379fe99519a4122f3b225be30))
	ROM_LOAD16_BYTE("sssnd_11.s3", 0x200000, 0x100000, CRC(c4f2e08a) SHA1(e20ff622a3f475db11f1f44d36a6669e160437a3))
	ROM_LOAD16_BYTE("sssnd_11.s4", 0x400000, 0x100000, CRC(258b0a27) SHA1(83763b98907cf38e6f7b9fe4f26ce93a54ba3568))
ROM_END

ROM_START(ss_12)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("stiffg11.1_2", 0x00000, 0x80000, CRC(70eca59c) SHA1(07d50a32a4fb287780c4e6c1cb6fbeba97480219))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("sssnd_11.s2", 0x000000, 0x100000, CRC(1b080a72) SHA1(be80e99e6bcc482379fe99519a4122f3b225be30))
	ROM_LOAD16_BYTE("sssnd_11.s3", 0x200000, 0x100000, CRC(c4f2e08a) SHA1(e20ff622a3f475db11f1f44d36a6669e160437a3))
	ROM_LOAD16_BYTE("sssnd_11.s4", 0x400000, 0x100000, CRC(258b0a27) SHA1(83763b98907cf38e6f7b9fe4f26ce93a54ba3568))
ROM_END

ROM_START(ss_03)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("ss_g11.0_3", 0x00000, 0x80000, CRC(5b9755d6) SHA1(207d9ea858c76c4991747b401dc83183c1ddf7e4))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("sssnd_s2.22", 0x000000, 0x100000, CRC(12b92d7a) SHA1(69151ffb5d2befe28e1ed2c8153c2227552b681c))
	ROM_LOAD16_BYTE("sssnd_s3.21", 0x200000, 0x100000, CRC(c4f2e08a) SHA1(e20ff622a3f475db11f1f44d36a6669e160437a3))
	ROM_LOAD16_BYTE("sssnd_s4.21", 0x400000, 0x100000, CRC(258b0a27) SHA1(83763b98907cf38e6f7b9fe4f26ce93a54ba3568))
ROM_END

ROM_START(ss_01)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("ss_g11.rom", 0x00000, 0x80000, CRC(affd278f) SHA1(e6f41da169fa15c25cfaac22057f3e491da18fc5))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("ss_s2.rom", 0x000000, 0x100000, CRC(ad079cbc) SHA1(77c7f676fc2f46e22b74b381638725269f7d23f4))
	ROM_LOAD16_BYTE("sssnd_s3.21", 0x200000, 0x100000, CRC(c4f2e08a) SHA1(e20ff622a3f475db11f1f44d36a6669e160437a3))
	ROM_LOAD16_BYTE("sssnd_s4.21", 0x400000, 0x100000, CRC(258b0a27) SHA1(83763b98907cf38e6f7b9fe4f26ce93a54ba3568))
ROM_END

/*-----------------
/  Tales Of The Arabian Nights #50047
/------------------*/
ROM_START(totan_14)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("an_g11.1_4", 0x00000, 0x80000, CRC(54db749e) SHA1(8f8b44febf3b672107e7715ec16e39dd91ee4cbb))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("ans2v1_1.rom", 0x000000, 0x100000, CRC(0d023f90) SHA1(e411f7824df89374cf3385a2660d5bc91e0e9ef0))
	ROM_LOAD16_BYTE("ans3v1_0.rom", 0x200000, 0x100000, CRC(3f677813) SHA1(b1e67c74b927c0c8cb76be8794a04a53fdf643d4))
	ROM_LOAD16_BYTE("ans4v1_0.rom", 0x400000, 0x100000, CRC(c26dff5f) SHA1(d86323f0df15cf7abd4480d173e6b217ef715396))
	ROM_LOAD16_BYTE("ans5v1_0.rom", 0x600000, 0x080000, CRC(32ca1602) SHA1(e4c7235b5d387bdde16ebef4d3aeeb7276c69d6d))
	ROM_RELOAD(0x600000+0x080000, 0x080000)
ROM_END

ROM_START(totan_13)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("arab1_3.rom", 0x00000, 0x80000, CRC(2e4b9439) SHA1(ba564c5984d3b68eaeba27d06f3acd95d26073ee))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("ans2v1_1.rom", 0x000000, 0x100000, CRC(0d023f90) SHA1(e411f7824df89374cf3385a2660d5bc91e0e9ef0))
	ROM_LOAD16_BYTE("ans3v1_0.rom", 0x200000, 0x100000, CRC(3f677813) SHA1(b1e67c74b927c0c8cb76be8794a04a53fdf643d4))
	ROM_LOAD16_BYTE("ans4v1_0.rom", 0x400000, 0x100000, CRC(c26dff5f) SHA1(d86323f0df15cf7abd4480d173e6b217ef715396))
	ROM_LOAD16_BYTE("ans5v1_0.rom", 0x600000, 0x080000, CRC(32ca1602) SHA1(e4c7235b5d387bdde16ebef4d3aeeb7276c69d6d))
	ROM_RELOAD(0x600000+0x080000, 0x080000)
ROM_END

ROM_START(totan_12)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("arab1_2.rom", 0x00000, 0x80000, CRC(f9ae3796) SHA1(06e4ce89cab2e0fe5039de4261f7b5ebd4c11c0b))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("ans2v1_1.rom", 0x000000, 0x100000, CRC(0d023f90) SHA1(e411f7824df89374cf3385a2660d5bc91e0e9ef0))
	ROM_LOAD16_BYTE("ans3v1_0.rom", 0x200000, 0x100000, CRC(3f677813) SHA1(b1e67c74b927c0c8cb76be8794a04a53fdf643d4))
	ROM_LOAD16_BYTE("ans4v1_0.rom", 0x400000, 0x100000, CRC(c26dff5f) SHA1(d86323f0df15cf7abd4480d173e6b217ef715396))
	ROM_LOAD16_BYTE("ans5v1_0.rom", 0x600000, 0x080000, CRC(32ca1602) SHA1(e4c7235b5d387bdde16ebef4d3aeeb7276c69d6d))
	ROM_RELOAD(0x600000+0x080000, 0x080000)
ROM_END

ROM_START(totan_04)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("an_g11.0_4", 0x00000, 0x80000, CRC(20da3800) SHA1(c8c048f35b1828f9ee1e7fc3201f1a316974b924))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("ans2v1_1.rom", 0x000000, 0x100000, CRC(0d023f90) SHA1(e411f7824df89374cf3385a2660d5bc91e0e9ef0))
	ROM_LOAD16_BYTE("ans3v1_0.rom", 0x200000, 0x100000, CRC(3f677813) SHA1(b1e67c74b927c0c8cb76be8794a04a53fdf643d4))
	ROM_LOAD16_BYTE("ans4v1_0.rom", 0x400000, 0x100000, CRC(c26dff5f) SHA1(d86323f0df15cf7abd4480d173e6b217ef715396))
	ROM_LOAD16_BYTE("ans5v1_0.rom", 0x600000, 0x080000, CRC(32ca1602) SHA1(e4c7235b5d387bdde16ebef4d3aeeb7276c69d6d))
	ROM_RELOAD(0x600000 +0x080000, 0x080000)
ROM_END

/*-----------------
/  The Champion Pub #50063
/------------------*/
ROM_START(cp_16)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cp_g11.1_6", 0x00000, 0x80000, CRC(d6d0b921) SHA1(6784bd5116d239f307310d4a1ddac1068292dd60))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("cp_s2.bin", 0x000000, 0x080000, CRC(e0b67f6f) SHA1(48fbf01eca4fd6250df18bb5f35959100f40f6e0))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("cp_s3.bin", 0x200000, 0x100000, CRC(68accf24) SHA1(9f86ac84ef8130592e471f1da0e05ba811dbc38b))
	ROM_LOAD16_BYTE("cp_s4.bin", 0x400000, 0x100000, CRC(50d1c920) SHA1(00b247853ef1f91c6245746c9311f8463b9335d1))
	ROM_LOAD16_BYTE("cp_s5.bin", 0x600000, 0x100000, CRC(69af347a) SHA1(d15683e6297603104e4ba777224331c24565be7c))
	ROM_LOAD16_BYTE("cp_s6.bin", 0x800000, 0x100000, CRC(76ca4fed) SHA1(8995e518c8dafbdd8bf994533b71f42172057b27))
	ROM_LOAD16_BYTE("cp_s7.bin", 0xa00000, 0x100000, CRC(be619157) SHA1(b18acde4f683b5f8b2248b46bb3dc7c3e0ab1c26))
ROM_END

ROM_START(cp_15)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("cp_g11.1_5", 0x00000, 0x80000, CRC(4255bfcb) SHA1(4ec17e6c0e07fd8d52af9d33776007930d8422c6))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("cp_s2.bin", 0x000000, 0x080000, CRC(e0b67f6f) SHA1(48fbf01eca4fd6250df18bb5f35959100f40f6e0))
	ROM_RELOAD(0x000000+0x100000, 0x080000)
	ROM_LOAD16_BYTE("cp_s3.bin", 0x200000, 0x100000, CRC(68accf24) SHA1(9f86ac84ef8130592e471f1da0e05ba811dbc38b))
	ROM_LOAD16_BYTE("cp_s4.bin", 0x400000, 0x100000, CRC(50d1c920) SHA1(00b247853ef1f91c6245746c9311f8463b9335d1))
	ROM_LOAD16_BYTE("cp_s5.bin", 0x600000, 0x100000, CRC(69af347a) SHA1(d15683e6297603104e4ba777224331c24565be7c))
	ROM_LOAD16_BYTE("cp_s6.bin", 0x800000, 0x100000, CRC(76ca4fed) SHA1(8995e518c8dafbdd8bf994533b71f42172057b27))
	ROM_LOAD16_BYTE("cp_s7.bin", 0xa00000, 0x100000, CRC(be619157) SHA1(b18acde4f683b5f8b2248b46bb3dc7c3e0ab1c26))
ROM_END

/*-------------
/ Ticket Tac Toe #90005
/--------------*/
ROM_START(ttt_10)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("tikt1_0.rom", 0x00000, 0x80000, CRC(bf1d0382) SHA1(3d26413400915594e9f1cc08a551c05526b94223))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("ttt_s2.rom", 0x000000, 0x100000, CRC(faae93eb) SHA1(672758544b260d7751ac296f5beb2e271e77c50a))
	ROM_LOAD16_BYTE("ttt_s3.rom", 0x200000, 0x100000, CRC(371ba9b3) SHA1(de6a8cb78e08a434f6668dd4a93cad857acba310))
ROM_END

/*--------------
/ Test Fixture WPC95
/---------------*/
ROM_START(tf95_12)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("g11_12.rom", 0x00000, 0x80000, CRC(259a2b23) SHA1(16f8c15e046809e0b1587b0c981d36f4d8a750ca))
	ROM_RELOAD(0x80000, 0x80000)
	ROM_REGION16_LE(0x1000000, "dcs", ROMREGION_ERASEFF)
	ROM_LOAD16_BYTE("s2_10.rom", 0x000000, 0x100000, CRC(ceff7fe4) SHA1(ff2574f65e09d446b9e446abd58159a7d100059b))
ROM_END

GAME(1996,  tf95_12,    0,          wpc_95, afm,    wpc_95_state,   tf95,   ROT0,   "Bally",                "WPC 95 Test Fixture (1.2)",              MACHINE_MECHANICAL)
GAME(1995,  afm_113,    0,          wpc_95, afm,    wpc_95_state,   afm,    ROT0,   "Bally",                "Attack From Mars (1.13, Free play)",     MACHINE_MECHANICAL)
GAME(1995,  afm_113b,   afm_113,    wpc_95, afm,    wpc_95_state,   afm,    ROT0,   "Bally",                "Attack From Mars (1.13b)",               MACHINE_MECHANICAL)
GAME(1995,  afm_11,     afm_113,    wpc_95, afm,    wpc_95_state,   afm,    ROT0,   "Bally",                "Attack From Mars (1.1)",                 MACHINE_MECHANICAL)
GAME(1995,  afm_11u,    afm_113,    wpc_95, afm,    wpc_95_state,   afm,    ROT0,   "Bally",                "Attack From Mars (1.1 Ultrapin)",        MACHINE_MECHANICAL)
GAME(1995,  afm_10,     afm_113,    wpc_95, afm,    wpc_95_state,   afm,    ROT0,   "Bally",                "Attack From Mars (1.0)",                 MACHINE_MECHANICAL)
GAME(1998,  cc_13,      0,          wpc_95, cc,     wpc_95_state,   cc,     ROT0,   "Bally",                "Cactus Canyon (1.3)",                    MACHINE_MECHANICAL)
GAME(1998,  cc_12,      cc_13,      wpc_95, cc,     wpc_95_state,   cc,     ROT0,   "Bally",                "Cactus Canyon (1.2)",                    MACHINE_MECHANICAL)
GAME(1998,  cc_10,      cc_13,      wpc_95, cc,     wpc_95_state,   cc,     ROT0,   "Bally",                "Cactus Canyon (1.0)",                    MACHINE_MECHANICAL)
GAME(1998,  cc_104,     cc_13,      wpc_95, cc,     wpc_95_state,   cc,     ROT0,   "Bally",                "Cactus Canyon (1.04 Test 0.2)",          MACHINE_MECHANICAL)
GAME(1997,  cv_14,      0,          wpc_95, cv,     wpc_95_state,   cv,     ROT0,   "Bally",                "Cirqus Voltaire (1.4)",                  MACHINE_MECHANICAL)
GAME(1997,  cv_20h,     cv_14,      wpc_95, cv,     wpc_95_state,   cv,     ROT0,   "Bally",                "Cirqus Voltaire (2.0H)",                 MACHINE_MECHANICAL)
GAME(1997,  cv_10,      cv_14,      wpc_95, cv,     wpc_95_state,   cv,     ROT0,   "Bally",                "Cirqus Voltaire (1.0)",                  MACHINE_MECHANICAL)
GAME(1997,  cv_11,      cv_14,      wpc_95, cv,     wpc_95_state,   cv,     ROT0,   "Bally",                "Cirqus Voltaire (1.1)",                  MACHINE_MECHANICAL)
GAME(1997,  cv_13,      cv_14,      wpc_95, cv,     wpc_95_state,   cv,     ROT0,   "Bally",                "Cirqus Voltaire (1.3)",                  MACHINE_MECHANICAL)
GAME(1995,  congo_21,   0,          wpc_95, congo,  wpc_95_state,   congo,  ROT0,   "Williams",             "Congo (2.1)",                            MACHINE_MECHANICAL)
GAME(1995,  congo_20,   congo_21,   wpc_95, congo,  wpc_95_state,   congo,  ROT0,   "Williams",             "Congo (2.0)",                            MACHINE_MECHANICAL)
GAME(1995,  congo_13,   congo_21,   wpc_95, congo,  wpc_95_state,   congo,  ROT0,   "Williams",             "Congo (1.3)",                            MACHINE_MECHANICAL)
GAME(1995,  congo_11,   congo_21,   wpc_95, congo,  wpc_95_state,   congo,  ROT0,   "Williams",             "Congo (1.1)",                            MACHINE_MECHANICAL)
GAME(1996,  jy_12,      0,          wpc_95, jy,     wpc_95_state,   jy,     ROT0,   "Williams",             "Junk Yard (1.2)",                        MACHINE_MECHANICAL)
GAME(1996,  jy_11,      jy_12,      wpc_95, jy,     wpc_95_state,   jy,     ROT0,   "Williams",             "Junk Yard (1.1)",                        MACHINE_MECHANICAL)
GAME(1996,  jy_03,      jy_12,      wpc_95, jy,     wpc_95_state,   jy,     ROT0,   "Williams",             "Junk Yard (0.3)",                        MACHINE_MECHANICAL)
GAME(1999,  mm_10,      0,          wpc_95, mm,     wpc_95_state,   mm,     ROT0,   "Williams",             "Medieval Madness (1.0)",                 MACHINE_MECHANICAL)
GAME(1999,  mm_10u,     mm_10,      wpc_95, mm,     wpc_95_state,   mm,     ROT0,   "Williams",             "Medieval Madness (1.0 Ultrapin)",        MACHINE_MECHANICAL)
GAME(1999,  mm_109,     mm_10,      wpc_95, mm,     wpc_95_state,   mm,     ROT0,   "Williams",             "Medieval Madness (1.09)",                MACHINE_MECHANICAL)
GAME(1999,  mm_109b,    mm_10,      wpc_95, mm,     wpc_95_state,   mm,     ROT0,   "Williams",             "Medieval Madness (1.09B)",               MACHINE_MECHANICAL)
GAME(1999,  mm_109c,    mm_10,      wpc_95, mm,     wpc_95_state,   mm,     ROT0,   "Williams",             "Medieval Madness (1.09C Profanity)",     MACHINE_MECHANICAL)
GAME(1997,  mm_05,      mm_10,      wpc_95, mm,     wpc_95_state,   mm,     ROT0,   "Williams",             "Medieval Madness (0.50)",                MACHINE_MECHANICAL)
GAME(1998,  mb_10,      0,          wpc_95, mb,     wpc_95_state,   mb,     ROT0,   "Williams",             "Monster Bash (1.0)",                     MACHINE_MECHANICAL)
GAME(1998,  mb_106,     mb_10,      wpc_95, mb,     wpc_95_state,   mb,     ROT0,   "Williams",             "Monster Bash (1.06)",                    MACHINE_MECHANICAL)
GAME(1998,  mb_106b,    mb_10,      wpc_95, mb,     wpc_95_state,   mb,     ROT0,   "Williams",             "Monster Bash (1.06b)",                   MACHINE_MECHANICAL)
GAME(1997,  nbaf_31,    0,          wpc_95, nbaf,   wpc_95_state,   nbaf,   ROT0,   "Bally",                "NBA Fastbreak (3.1 - S3.0)",             MACHINE_MECHANICAL)
GAME(1997,  nbaf_31a,   nbaf_31,    wpc_95, nbaf,   wpc_95_state,   nbaf,   ROT0,   "Bally",                "NBA Fastbreak (3.1 - S1.0)",             MACHINE_MECHANICAL)
GAME(1997,  nbaf_11s,   nbaf_31,    wpc_95, nbaf,   wpc_95_state,   nbaf,   ROT0,   "Bally",                "NBA Fastbreak (1.1 - S0.4)",             MACHINE_MECHANICAL)
GAME(1997,  nbaf_11,    nbaf_31,    wpc_95, nbaf,   wpc_95_state,   nbaf,   ROT0,   "Bally",                "NBA Fastbreak (1.1)",                    MACHINE_MECHANICAL)
GAME(1997,  nbaf_11a,   nbaf_31,    wpc_95, nbaf,   wpc_95_state,   nbaf,   ROT0,   "Bally",                "NBA Fastbreak (1.1 - S2.0)",             MACHINE_MECHANICAL)
GAME(1997,  nbaf_115,   nbaf_31,    wpc_95, nbaf,   wpc_95_state,   nbaf,   ROT0,   "Bally",                "NBA Fastbreak (1.15)",                   MACHINE_MECHANICAL)
GAME(1997,  nbaf_21,    nbaf_31,    wpc_95, nbaf,   wpc_95_state,   nbaf,   ROT0,   "Bally",                "NBA Fastbreak (2.1)",                    MACHINE_MECHANICAL)
GAME(1997,  nbaf_22,    nbaf_31,    wpc_95, nbaf,   wpc_95_state,   nbaf,   ROT0,   "Bally",                "NBA Fastbreak (2.2)",                    MACHINE_MECHANICAL)
GAME(1997,  nbaf_23,    nbaf_31,    wpc_95, nbaf,   wpc_95_state,   nbaf,   ROT0,   "Bally",                "NBA Fastbreak (2.3)",                    MACHINE_MECHANICAL)
GAME(1997,  ngg_13,     0,          wpc_95, ngg,    wpc_95_state,   ngg,    ROT0,   "Williams",             "No Good Gofers (1.3)",                   MACHINE_MECHANICAL)
GAME(1997,  ngg_p06,    ngg_13,     wpc_95, ngg,    wpc_95_state,   ngg,    ROT0,   "Williams",             "No Good Gofers (p0.6)",                  MACHINE_MECHANICAL)
GAME(1997,  ngg_10,     ngg_13,     wpc_95, ngg,    wpc_95_state,   ngg,    ROT0,   "Williams",             "No Good Gofers (1.0)",                   MACHINE_MECHANICAL)
GAME(1998,  sc_18,      0,          wpc_95, sc,     wpc_95_state,   sc,     ROT0,   "Bally",                "Safe Cracker (1.8)",                     MACHINE_MECHANICAL)
GAME(1998,  sc_18n,     sc_18,      wpc_95, sc,     wpc_95_state,   sc,     ROT0,   "Bally",                "Safe Cracker (1.8N)",                    MACHINE_MECHANICAL)
GAME(1998,  sc_18s2,    sc_18,      wpc_95, sc,     wpc_95_state,   sc,     ROT0,   "Bally",                "Safe Cracker (1.8 alternate sound)",     MACHINE_MECHANICAL)
GAME(1996,  sc_17,      sc_18,      wpc_95, sc,     wpc_95_state,   sc,     ROT0,   "Bally",                "Safe Cracker (1.7)",                     MACHINE_MECHANICAL)
GAME(1996,  sc_17n,     sc_18,      wpc_95, sc,     wpc_95_state,   sc,     ROT0,   "Bally",                "Safe Cracker (1.7N)",                    MACHINE_MECHANICAL)
GAME(1996,  sc_14,      sc_18,      wpc_95, sc,     wpc_95_state,   sc,     ROT0,   "Bally",                "Safe Cracker (1.4)",                     MACHINE_MECHANICAL)
GAME(1996,  sc_091,     sc_18,      wpc_95, sc,     wpc_95_state,   sc,     ROT0,   "Bally",                "Safe Cracker (0.91)",                    MACHINE_MECHANICAL)
GAME(1996,  ss_15,      0,          wpc_95, ss,     wpc_95_state,   ss,     ROT0,   "Bally",                "Scared Stiff (1.5)",                     MACHINE_MECHANICAL)
GAME(1996,  ss_14,      ss_15,      wpc_95, ss,     wpc_95_state,   ss,     ROT0,   "Bally",                "Scared Stiff (1.4)",                     MACHINE_MECHANICAL)
GAME(1996,  ss_12,      ss_15,      wpc_95, ss,     wpc_95_state,   ss,     ROT0,   "Bally",                "Scared Stiff (1.2)",                     MACHINE_MECHANICAL)
GAME(1996,  ss_03,      ss_15,      wpc_95, ss,     wpc_95_state,   ss,     ROT0,   "Bally",                "Scared Stiff (0.3)",                     MACHINE_MECHANICAL)
GAME(1996,  ss_01,      ss_15,      wpc_95, ss,     wpc_95_state,   ss,     ROT0,   "Bally",                "Scared Stiff (D0.1R with sound rev.25)", MACHINE_MECHANICAL)
GAME(1996,  totan_14,   0,          wpc_95, totan,  wpc_95_state,   totan,  ROT0,   "Williams",             "Tales Of The Arabian Nights (1.4)",      MACHINE_MECHANICAL)
GAME(1996,  totan_13,   totan_14,   wpc_95, totan,  wpc_95_state,   totan,  ROT0,   "Williams",             "Tales Of The Arabian Nights (1.3)",      MACHINE_MECHANICAL)
GAME(1996,  totan_12,   totan_14,   wpc_95, totan,  wpc_95_state,   totan,  ROT0,   "Williams",             "Tales Of The Arabian Nights (1.2)",      MACHINE_MECHANICAL)
GAME(1996,  totan_04,   totan_14,   wpc_95, totan,  wpc_95_state,   totan,  ROT0,   "Williams",             "Tales Of The Arabian Nights (0.4)",      MACHINE_MECHANICAL)
GAME(1998,  cp_16,      0,          wpc_95, cp,     wpc_95_state,   cp,     ROT0,   "Bally",                "The Champion Pub (1.6)",                 MACHINE_MECHANICAL)
GAME(1998,  cp_15,      cp_16,      wpc_95, cp,     wpc_95_state,   cp,     ROT0,   "Bally",                "The Champion Pub (1.5)",                 MACHINE_MECHANICAL)
GAME(1996,  ttt_10,     0,          wpc_95, ttt,    wpc_95_state,   ttt,    ROT0,   "Williams",             "Ticket Tac Toe (1.0)",                   MACHINE_MECHANICAL)
