// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*************************************************************************************

    Yamaha MU-100 : 32-voice polyphonic/multitimbral General MIDI/GS/XG tone module
    Preliminary driver by R. Belmont and O. Galibert

    CPU: Hitachi H8S/2655 (HD6432655F), strapped for mode 4 (24-bit address, 16-bit data, no internal ROM)
    Sound ASIC: Yamaha XS725A0
    RAM: 1 MSM51008 (1 meg * 1 bit = 128KBytes)

    I/O ports from service manual:

    Port 1:
        0 - LCD data, SW data, LED 1
        1 - LCD data, SW data, LED 2
        2 - LCD data, SW data, LED 3
        3 - LCD data, SW data, LED 4
        4 - LCD data, SW data, LED 5
        5 - LCD data, SW strobe data
        6 - LCD data, SW strobe data
        7 - LCD data, SW data, LED 6

    Port 2:
        0 - (out) LCD control RS
        1 - (out) LCD control R/W
        2 - (out) LCD control E
        3 - (out) LCD contrast A
        4 - (out) LCD contrast B
        5 - (out) LCD contrast C
        6 - (out) 1 MHz clock for serial
        7 - NC

    Port 3:
        4 - (out) A/D gain control 1
        5 - (out) A/D gain control 2

    Port 5:
        3 - (out) Reset signal for rotary encoder

    Port 6:
        1 - NC
        2 - (out) PB select (SW1)
        3 - (out) PB select (SW2)
        4 - (out) reset PB
        5 - (out) reset SWP30 (sound chip)
        6 - NC
        7 - (in) Plug detection for A/D input

    Port A:
        5 - (in) Off Line Detection
        6 - (out) Signal for rotary encoder (REB)
        7 - (out) Signal for rotary encoder (REA)

    Port F:
        0 - (out) (sws) LED,SW Strobe data latch
        1 - (out) (swd) SW data read control
        2 - (out) PB select (SW4)

    Port G:
        0 - (out) PB select (SW3)

    Analog input channels:
        0 - level input R
        2 - level output L
        4 - host SW type switch position
        6 - battery voltage
        7 - model check (0 for MU100, 0.5 for OEM, 1 for MU100R)

    Switch map at the connector (17=ground)
        09 8 play
        10 8 edit
        11 8 mute/solo
        12 8 part -
        13 8 part +
        14 8 util
        15 8 effect
        16 8 enter
        12 7 select <
        13 7 select >
        16 7 mode
        15 7 eq
        14 7 exit
        10 7 value -
        11 7 value +
           2 led play
           3 led edit
           4 led util
           5 led effect
           6 led mode
           1 led eq

     IC32:
        1 p10 c.2
        2 p11 c.3
        3 p12 c.4
        4 p13 c.5
        5 p14 c.6
        6 p15 c.7
        7 p16 c.8
        8 p17 c.1
        g sws

     IC33
        1 p17 c.09
        2 p16 c.10
        3 p15 c.11
        4 p14 c.12
        5 p13 c.13
        6 p12 c.14
        7 p11 c.15
        8 p10 c.16
        g swd

**************************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/h8s2655.h"
#include "video/hd44780.h"
#include "sound/swp30.h"

#include "debugger.h"
#include "screen.h"
#include "speaker.h"


static INPUT_PORTS_START( mu100 )
	PORT_START("P7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter")     PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Effect")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Util")      PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part +")    PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part -")    PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mute/Solo") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit")      PORT_CODE(KEYCODE_E)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play")      PORT_CODE(KEYCODE_A)

	PORT_START("P8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mode")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Eq")        PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit")      PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select >")  PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select <")  PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value +")   PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value -")   PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/*               40            41   42   43

reads
  .09 at 3e5a, tests & 0x8000
  .05 at 3e4a: read current to change bits (sets xaxx)
  .0a at 4070: read current to change bits (keeps 14-15)

01.0f at d320:  put bits 6-13 in r0l inverted, bits 14-15 in r0h 6-7, does complex calcs from it



[:] snd_w 00.09, f000 (0020a0)
[:] snd_w 00.10, 4000 (00d376)
[:] snd_w 00.00, 169b (00d3be)
[:] snd_w 00.01, e0ff (00d3d0)
[:] snd_w 00.02, 8000 (00d3f8)
[:] snd_w 00.03, 5010 (00d400)
[:] snd_w 00.04, 0000 (00d412)
[:] snd_w 00.05, fb00 (00d434)
[:] snd_w 00.20, e05d (00d43e)
[:] snd_w 00.22, 1fa3 (00d448)
[:] snd_w 00.24, 2000 (00d452)
[:] snd_w 00.26, 0257 (00d45c)
[:] snd_w 00.28, fda9 (00d466)
[:] snd_w 00.2a, 2000 (00d470)
[:] snd_w 00.06, 5d80 (00d48e)
[:] snd_w 00.07, 1414 (00d4ae) 1614 1614 1614
[:] snd_w 00.08, 18fe (00d4ce) 1afe 1afe 1afe
[:] snd_w 00.0a, 5f00 (00d510)
[:] snd_w 00.0b, 0000 (00d530)
[:] snd_w 00.0b, 6f00 (00d53a)
[:] snd_w 00.11, be29 (00d54e) be7e bdd2 be27
[:] snd_w 00.12, 0000 (00d56c)
[:] snd_w 00.13, 5c74 (00d570)           6f0b
[:] snd_w 00.14, 0000 (00d586)
[:] snd_w 00.15, 749d (00d58a) 749d 5951 5951
[:] snd_w 00.16, ee42 (00d5d6)
[:] snd_w 00.17, a89b (00d5da)
[:] snd_w 00.32, 0808 (00d5ea)
[:] snd_w 00.33, 182b (00d5fa)
[:] snd_w 00.34, ff10 (00d60a)
[:] snd_w 00.35, 0d00 (00d614)
[:] snd_w 00.36, 0800 (00d61e)
[:] snd_w 00.37, 0400 (00d628)
[:] snd_w 00.09, 70e0 (00d634)
[:] snd_r 00.0b (00d638)
[:] snd_r 00.0b (00d63c)
[:] snd_r 00.09 (00d658)
[:] snd_r 00.09 (00d65c)

    e3 = (note - ->m1b) * 100 + s8(->m2b)

    compute freq adjustment in 1/100th of semitone (ex. 737)
clamp to +/- 9599
(2d74)
div/rem by 300 -> e6 = 137, r6 = 2
lookup d14c[rem].b (0-299 -> 00-ff, perfectly linear), tack div on top -> 274
put sign back in, and with 3fff, -> g58






 9    = attenuation?
11    = 52|58 -> 15 flag, 14 zero, 13-0 cents frequency adjustment
12-13 = pre-size + flags
14-15 = post-size + flags
16-17 = address + flags + loop size?
32    = 6a/6b = pan l/r
33    = 6c/6d = ? / reverb attenuation
34    = 6e/6f = chorus attenuation / ?
35    = 70
36    = 72
37    = 74
3e    = ? / chorus lfo frequency
3f    = ? / chorus lfo frequency

09    = 0x7000 + 4c
read 0b twice, store the second is 2114c7

d510:
  r6 = ffec54 (0)
  if(!0) { ... }
  r5 = r6
  +0b = r5
  r5h = ffec50
  +0b = r5
  +11 = ffec58 | (ffec52 << 8)
  +12, +13 = ffec5c.l | (ffec5a << 24)
  +14, +15 = ffec60.l | (ffec64 << 24)

50: 6f 40 80 00 00 00 00 00 3e 27 00 00 00 00 6f 0b
60: 00 00 59 51 00 ee 00 42 e1 87 08 08 18 2b ff 10

2e1c computes everything

*/

class mu100_state : public driver_device
{
public:
	mu100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_swp30(*this, "swp30")
		, m_lcd(*this, "lcd")
		, m_ioport_p7(*this, "P7")
		, m_ioport_p8(*this, "P8")
	{ }

	void mu100(machine_config &config);

	int seq;

	virtual void machine_reset() override {
	  timer_alloc()->adjust(attotime::from_double(5));
		seq = 0;
	}

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override {
		static u8 xnote[8] = { 60, 62, 64, 65, 67, 69, 71, 72 };
		static int note = 0;
		switch(seq) {
		case 0: push(0x90); logerror("swp30 midi keyon %d\n", note); timer.adjust(attotime::from_double(0.0005)); if(1) machine().debug_break(); break;
		case 1: push(xnote[note]); timer.adjust(attotime::from_double(0.0005)); break;
		case 2: push(0x7f); timer.adjust(attotime::from_double(0.5)); break;
		case 3: push(0x80); logerror("swp30 midi keyoff\n"); timer.adjust(attotime::from_double(0.0005)); if(0) machine().debug_break(); break;
		case 4: push(xnote[note]); timer.adjust(attotime::from_double(0.0005)); ; if(note == 8) note = 0; break;
		case 5: push(0x7f); timer.adjust(attotime::from_double(0.05)); seq=0; return;
		}
		seq ++;
	}

	void push(u8 val) {
		logerror("push %02x\n", val);
		machine().root_device().subdevice<h8_sci_device>("maincpu:sci0")->force(val);
	}

protected:
	virtual u16 adc7_r();

private:
	enum {
		P2_LCD_RS     = 0x01,
		P2_LCD_RW     = 0x02,
		P2_LCD_ENABLE = 0x04
	};

	required_device<h8s2655_device> m_maincpu;
	required_device<swp30_device> m_swp30;
	required_device<hd44780_device> m_lcd;
	required_ioport m_ioport_p7;
	required_ioport m_ioport_p8;

	u8 cur_p1, cur_p2, cur_p3, cur_p5, cur_p6, cur_pa, cur_pf, cur_pg;
	u8 cur_ic32;
	float contrast;

	u16 adc0_r();
	u16 adc2_r();
	u16 adc4_r();
	u16 adc6_r();

	void p1_w(u16 data);
	u16 p1_r();
	void p2_w(u16 data);
	void p3_w(u16 data);
	void p5_w(u16 data);
	void p6_w(u16 data);
	u16 p6_r();
	void pa_w(u16 data);
	u16 pa_r();
	void pf_w(u16 data);
	void pg_w(u16 data);

	float lightlevel(const u8 *src, const u8 *render);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual void machine_start() override;
	void mu100_iomap(address_map &map);
	void mu100_map(address_map &map);
	void swp30_map(address_map &map);
};

class mu100r_state : public mu100_state {
public:
	mu100r_state(const machine_config &mconfig, device_type type, const char *tag)
		: mu100_state(mconfig, type, tag)
	{ }

private:
	virtual u16 adc7_r() override;
};

#include "../drivers/ymmu100.hxx"

struct v1 {
	u32 m0;
	u8  m4;
	u8  m5;
	u16 m6;
	u8  m8;
	u8  m9;
	u16 ma;
	u8  mc;
	u8  md;
	u16 me;
};

// g base = ffec00

// fcf30: 0010 27 1b 00 00b0f5 00 006bc4 ee 402c3e
// ...
// fcfd0: 0048 c9 45 00 006f0b 00 005951 ee 42e187

//                            4772 616e 6450 2023  GrandP #
// f5b1e: 0225 007f 017f 58f0 8040 7000 4040 3c77
//        3c3f 3f3f 3f40 4040 4040 0100 441f 3c54
//        6c42 4042 45d7 093c 3f01 020d 404e 4640
//        406c 183c 505c 4045 3e3b 9748 902f 0b0d
//        2075 0000 0000 0050

//20f03e: 000f5b1e 0020de54 00000000 0000fdd5

#if 0
// samp2:
//   000000  - 00 20 27 1b 00 00b0f5 00 006bc4 ee 402c3e
//   004734  - 00 23 f9 1f 00 00bafd 00 007d3c ee 4075f0
//   009544  - 00 28 2b 23 00 00cbc6 00 007d50 e4 40c832
//   00e78b  - 00 2b c7 28 00 00c0a6 00 007b70 e4 4117b1
//   013691  - 00 32 29 2d 00 00bd5e 00 007c62 ee 4165e6


// g34 = 0x43 = note

// checksums
//   2.ef = (0/4000000) | (1 << n)
//   3.e = 0000
//   3.f = 0001
//   4.e = 8000
//   wait until 4.f.b != 0
//   read 5.ef and compare
//   4.e = 0000

//   0..13 for 32M, 0..12 for 16M

//   list of checksums at 58712/58766
//   ff49 ffa0
//   feb3 ff03
//   fd93 fdd7
//   fb76 fbb0
//   f79d f7e7

// f1e40 = select instrument
void f1e40()
{
	er3 = g04; // 20de54
	r5 = g29; // 7f - global volume?
	r4 = er3->mcb; // 40 - global volume?
	r5 = r4*r5 >> 6; // -> 7f
	r5 += 2*(er3->md - 0x40);
	r5 = clamp(r5, 1, 0x7f);
	g29 = r5;
	r5 = g20f034; // 01
	if(r5 & 1) {
		r4 = er3->m1b; // 00
		r4 = lcb098[r4]; // byte, 00
		if(!r4) {
			r4 = er3->m2b;
			r6 = g20f031;
			if(!r6)
				r4 = lcb118[r4];
			else {
				r6 = g20f034;
				if(r6 == 3)
					r4 = lcb118[r4];
				else
					r4 = lcb218[r4];
			}
		} else {
			if(r4 != 0x48)
				r4 = lcb198[er3->m2b];
		}
	} else
		r4 = lcb518[er3->m1b];

	// r4 -> 55
	r4 = (r4 << 8) | (er3->m3b << 1);
	r4 = cb710 + lb4898[r4 >> 1]; // dword 2a404 -> f5b14 005a GrandP #
	g0c = r4 + 0xe;


}

void f2ace(er4 *instrument) // f5b1e
{
	r6 = (er4->m0 << 7) | er4->m1;
	if(r6 < 0x125)
		r6 = lf692e + tfccde[r6].w;
	else if(r6 < 0x147)
		r6 = lfcf30 + tfdf80[r6 - 0x125].w;
	else
		r6 = lfdfc4 + t100c34[r6 - 0x147].w;
	// r6 = fcf30
	while(r6->m3 & 0x7f <= g34)
		r6 += 0x10;
}

void f2d0a(...)
{
	//
	f2f26(); // calc freq

	// ...
	g3f = f6b14(); // r6h
	f330a(); // sets 6a/6b
	f2e1c(); // sets 5a/64/5c/66/60/65
	f3038(); // sets 38
	f31f8(er4); // sets 46 47 48 4a
	f32a4(); // sets 40
	f3486(); // sets 90/92/94/96/98/9a
	f309c();
}

void f2e1c(er4 **instrument) // 20f03e -> f5b1e
{
	v1 *xv1 = g18; // fcfd0
	g5a = xv1->m4;
	g64 = xv1->m8;
	er1 = (xv1->m5 << 16) | xv1->m6;
	er2 = (xv1->md << 16) | xv1->me;
	er3 = (xv1->m9 << 16) | xv1->ma;
	er6 = *er4;
	r0 = xv1->mc << 8;
	r5 = (er6->m43 << 7) + er6->m44;
	if(!(g64 & 0x80)) {
		er1 = max(0, er1 - er5);

	} else {
		// 2ae2
	}
	// 2ef8
	g5c = er1;
	g66 = er2;
	g60 = er3;
	g65 = r0 >> 8;
}
#endif

void mu100_state::machine_start()
{
	cur_p1 = cur_p2 = cur_p3 = cur_p5 = cur_p6 = cur_pa = cur_pf = cur_pg = cur_ic32 = 0xff;
	contrast = 1.0;
}

float mu100_state::lightlevel(const u8 *src, const u8 *render)
{
	u8 l = *src;
	if(l == 0)
		return 1.0;
	int slot = (src[1] << 8) | src[2];
	if(slot >= 0xff00)
		return (255-l)/255.0;

	int bit = slot & 7;
	int adr = (slot >> 3);
	if(render[adr] & (1 << bit))
		return 1-(1-(255-l)/255.0f)*contrast;
	return 0.95f;
}

u32 mu100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const u8 *render = m_lcd->render();
	const u8 *src = ymmu100_bkg + 15;

	for(int y=0; y<241; y++) {
		u32 *pix = reinterpret_cast<u32 *>(bitmap.raw_pixptr(y));
		for(int x=0; x<800; x++) {
			float light = lightlevel(src, render);
			u32 col = (int(0xef*light) << 16) | (int(0xf5*light) << 8);
			*pix++ = col;
			src += 3;
		}
		for(int x=800; x<900; x++)
			*pix++ = 0;
	}

	for(int i=0; i<6; i++)
		if(cur_ic32 & (1 << (i == 5 ? 7 : i))) {
			int x = 830 + 40*(i & 1);
			int y = 55 + 65*(i >> 1);
			for(int yy=-9; yy <= 9; yy++) {
				int dx = int(sqrt((float)(99-yy*yy)));
				u32 *pix = reinterpret_cast<u32 *>(bitmap.raw_pixptr(y+yy)) + (x-dx);
				for(int xx=0; xx<2*dx+1; xx++)
					*pix++ = 0x00ff00;
			}
		}
	return 0;
}

void mu100_state::mu100_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x200000, 0x21ffff).ram(); // 128K work RAM
	map(0x400000, 0x401fff).m(m_swp30, FUNC(swp30_device::map));
}

u16 mu100_state::adc0_r()
{
	logerror("adc0_r\n");
	return 0;
}

u16 mu100_state::adc2_r()
{
	logerror("adc2_r\n");
	return 0;
}

// Put the host switch to pure midi
u16 mu100_state::adc4_r()
{
	return 0;
}

// Battery level
u16 mu100_state::adc6_r()
{
	logerror("adc6_r\n");
	return 0x3ff;
}

// model detect.  pulled to GND (0) on MU100, to 0.5Vcc on the card version, to Vcc on MU100R
u16 mu100_state::adc7_r()
{
	return 0;
}

u16 mu100r_state::adc7_r()
{
	return 0x3ff;
}

void mu100_state::p1_w(u16 data)
{
	cur_p1 = data;
}

u16 mu100_state::p1_r()
{
	if((cur_p2 & P2_LCD_ENABLE)) {
		if(cur_p2 & P2_LCD_RW) {
			if(cur_p2 & P2_LCD_RS)
				return m_lcd->data_read();
			else
				return m_lcd->control_read();
		} else
			return 0x00;
	}

	if(!(cur_pf & 0x02)) {
		u8 val = 0xff;
		if(!(cur_ic32 & 0x20))
			val &= m_ioport_p7->read();
		if(!(cur_ic32 & 0x40))
			val &= m_ioport_p8->read();
		return val;
	}

	return 0xff;
}

void mu100_state::p2_w(u16 data)
{
	// LCB enable edge
	if(!(cur_p2 & P2_LCD_ENABLE) && (data & P2_LCD_ENABLE)) {
		if(!(cur_p2 & P2_LCD_RW)) {
			if(cur_p2 & P2_LCD_RS)
				m_lcd->data_write(cur_p1);
			else
				m_lcd->control_write(cur_p1);
		}
	}
	contrast = (8 - ((cur_p2 >> 3) & 7))/8.0;
	cur_p2 = data;
}

void mu100_state::p3_w(u16 data)
{
	cur_p3 = data;
	logerror("A/D gain control %d\n", (data >> 4) & 3);
}

void mu100_state::p5_w(u16 data)
{
	cur_p5 = data;
	logerror("Rotary reset %d\n", (data >> 3) & 1);
}

void mu100_state::p6_w(u16 data)
{
	cur_p6 = data;
	logerror("pbsel %d pbreset %d soundreset %d\n", (data >> 2) & 3, (data >> 4) & 1, (data >> 5) & 1);
}

u16 mu100_state::p6_r()
{
	logerror("plug in detect read\n");
	return 0x00;
}

void mu100_state::pa_w(u16 data)
{
	cur_pa = data;
	logerror("rotary encoder %d\n", (data >> 6) & 3);
}

u16 mu100_state::pa_r()
{
	logerror("offline detect read\n");
	return 0x00;
}

void mu100_state::pf_w(u16 data)
{
	if(!(cur_pf & 0x01) && (data & 0x01))
		cur_ic32 = cur_p1;
	cur_pf = data;
}

void mu100_state::pg_w(u16 data)
{
	cur_pg = data;
	logerror("pbsel3 %d\n", data & 1);
}

void mu100_state::mu100_iomap(address_map &map)
{
	map(h8_device::PORT_1, h8_device::PORT_1).rw(FUNC(mu100_state::p1_r), FUNC(mu100_state::p1_w));
	map(h8_device::PORT_2, h8_device::PORT_2).w(FUNC(mu100_state::p2_w));
	map(h8_device::PORT_3, h8_device::PORT_3).w(FUNC(mu100_state::p3_w));
	map(h8_device::PORT_5, h8_device::PORT_5).w(FUNC(mu100_state::p5_w));
	map(h8_device::PORT_6, h8_device::PORT_6).rw(FUNC(mu100_state::p6_r), FUNC(mu100_state::p6_w));
	map(h8_device::PORT_A, h8_device::PORT_A).rw(FUNC(mu100_state::pa_r), FUNC(mu100_state::pa_w));
	map(h8_device::PORT_F, h8_device::PORT_F).w(FUNC(mu100_state::pf_w));
	map(h8_device::PORT_G, h8_device::PORT_G).w(FUNC(mu100_state::pg_w));
	map(h8_device::ADC_0, h8_device::ADC_0).r(FUNC(mu100_state::adc0_r));
	map(h8_device::ADC_2, h8_device::ADC_2).r(FUNC(mu100_state::adc2_r));
	map(h8_device::ADC_4, h8_device::ADC_4).r(FUNC(mu100_state::adc4_r));
	map(h8_device::ADC_6, h8_device::ADC_6).r(FUNC(mu100_state::adc6_r));
	map(h8_device::ADC_7, h8_device::ADC_7).r(FUNC(mu100_state::adc7_r));
}

void mu100_state::swp30_map(address_map &map)
{
	   map(0x000000*4, 0x200000*4-1).rom().region("swp30",         0).mirror(4*0x200000);
	   map(0x400000*4, 0x500000*4-1).rom().region("swp30",  0x800000).mirror(4*0x300000);
	   map(0x800000*4, 0xa00000*4-1).rom().region("swp30", 0x1000000).mirror(4*0x200000); // Missing roms...
}

void mu100_state::mu100(machine_config &config)
{
	H8S2655(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mu100_state::mu100_map);
	m_maincpu->set_addrmap(AS_IO, &mu100_state::mu100_iomap);

	HD44780(config, m_lcd);
	m_lcd->set_lcd_size(4, 20);

	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_LCD);
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate, asynchronous updating anyway */
	screen.set_screen_update(FUNC(mu100_state::screen_update));
	screen.set_size(900, 241);
	screen.set_visarea(0, 899, 0, 240);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SWP30(config, m_swp30);
	m_swp30->set_addrmap(0, &mu100_state::swp30_map);
	m_swp30->add_route(0, "lspeaker", 1.0);
	m_swp30->add_route(1, "rspeaker", 1.0);

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set("maincpu:sci0", FUNC(h8_sci_device::rx_w));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);

	m_maincpu->subdevice<h8_sci_device>("sci0")->tx_handler().set(mdout, FUNC(midi_port_device::write_txd));
}

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios))

ROM_START( mu100 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "bios0", "xu50720 (v1.11, Aug. 3, 1999)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "xu50720.ic11", 0x000000, 0x200000, CRC(1126a8a4) SHA1(e90b8bd9d14297da26ba12f4d9a4f2d22cd7d34a) )
	ROM_SYSTEM_BIOS( 1, "bios1", "xt71420 (v1.05, Sep. 19, 1997)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "xt71420.ic11", 0x000000, 0x200000, CRC(0e5b3bae) SHA1(3148c5bd59a3d00809d3ab1921216215fe2582c5) )

	ROM_REGION( 0x1800000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "sx518b0.ic34", 0x000000, 0x400000, CRC(2550d44f) SHA1(fd3cce228c7d389a2fde25c808a5b26080588cba) )
	ROM_LOAD32_WORD( "sx743b0.ic35", 0x000002, 0x400000, CRC(a9109a6c) SHA1(a67bb49378a38a2d809bd717d286e18bc6496db0) )
	ROM_LOAD32_WORD( "xt445a0-828.ic36", 0x800000, 0x200000, CRC(225c2280) SHA1(23b5e046fd2e2ac01af3e6dc6357c5c6547b286b) )
	ROM_LOAD32_WORD( "xt461a0-829.ic37", 0x800002, 0x200000, CRC(a1d138a3) SHA1(46a7a7225cd7e1818ba551325d2af5ac1bf5b2bf) )
	ROM_LOAD32_WORD( "xt963a0.ic38", 0x1000000, 0x400000, NO_DUMP )
	ROM_LOAD32_WORD( "xt964a0.ic39", 0x1000002, 0x400000, NO_DUMP )

	ROM_REGION( 0x1000, "lcd", 0)
	// Hand made, 3 characters unused
	ROM_LOAD( "mu100-font.bin", 0x0000, 0x1000, BAD_DUMP CRC(a7d6c1d6) SHA1(9f0398d678bdf607cb34d83ee535f3b7fcc97c41) )
ROM_END

// Identical to the mu100
ROM_START( mu100r )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "bios0", "xu50720 (v1.11, Aug. 3, 1999)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "xu50720.ic11", 0x000000, 0x200000, CRC(1126a8a4) SHA1(e90b8bd9d14297da26ba12f4d9a4f2d22cd7d34a) )
	ROM_SYSTEM_BIOS( 1, "bios1", "xt71420 (v1.05, Sep. 19, 1997)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "xt71420.ic11", 0x000000, 0x200000, CRC(0e5b3bae) SHA1(3148c5bd59a3d00809d3ab1921216215fe2582c5) )

	ROM_REGION( 0x1800000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "sx518b0.ic34", 0x000000, 0x400000, CRC(2550d44f) SHA1(fd3cce228c7d389a2fde25c808a5b26080588cba) )
	ROM_LOAD32_WORD( "sx743b0.ic35", 0x000002, 0x400000, CRC(a9109a6c) SHA1(a67bb49378a38a2d809bd717d286e18bc6496db0) )
	ROM_LOAD32_WORD( "xt445a0-828.ic36", 0x800000, 0x200000, CRC(225c2280) SHA1(23b5e046fd2e2ac01af3e6dc6357c5c6547b286b) )
	ROM_LOAD32_WORD( "xt461a0-829.ic37", 0x800002, 0x200000, CRC(a1d138a3) SHA1(46a7a7225cd7e1818ba551325d2af5ac1bf5b2bf) )
	ROM_LOAD32_WORD( "xt963a0.ic38", 0x1000000, 0x400000, NO_DUMP )
	ROM_LOAD32_WORD( "xt964a0.ic39", 0x1000002, 0x400000, NO_DUMP )

	ROM_REGION( 0x1000, "lcd", 0)
	// Hand made, 3 characters unused
	ROM_LOAD( "mu100-font.bin", 0x0000, 0x1000, BAD_DUMP CRC(a7d6c1d6) SHA1(9f0398d678bdf607cb34d83ee535f3b7fcc97c41) )
ROM_END

CONS( 1997, mu100,  0,     0, mu100, mu100, mu100_state,  empty_init, "Yamaha", "MU100",                  0 )
CONS( 1997, mu100r, mu100, 0, mu100, mu100, mu100r_state, empty_init, "Yamaha", "MU100 Rackable version", 0 )
