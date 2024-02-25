// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "stvideo.h"
#include "screen.h"

bool v = false;


// Atari ST video generation

// Video in the ST is the result of the cooperation of three chips:
// - GLUE
// - MMU
// - Shifter

// The GLUE generates the video signals sync, blank and display enable
// (de).  The MMU generates the video read accesses and manages the
// dynamic ram in general (including dtack for it).  The shifter takes
// the video read results, holds the palette, and generate the pixels.

// The ST modes are split between color and monochrome.  Color is
// 320x200x4bpp or 640x200x2bpp at 50 or 60Hz, while monochrome is
// 640x400 at 71Hz.  It is very usual to mix parts of the screen at
// 320x200 and parts at 640x200.

// The clocks are built from Y2, a ~32MHz source (exact frequency
// varies between ntsc and pal countries, with some extra unique added
// in).  The usual exact frequency is 32084988 (for 50Hz systems).

// Pixel clock at 640x400 is 32Mhz, 640x200 is 16Mhz, 320x200 is 8Mhz.

// The shifter takes Y2 and generates a 16Mhz clock and internally a
// 8Mhz clock.  The MMU takes the 16MHz clock and generates a 8Mhz and
// a 4Mhz one.  The GLUE takes MMU's 8Mhz clock and generate
// internally its own 2Mhz clock.

// The MMU takes care of the arbitration between the CPU and video
// accesses to the ram.  Specifically it alternates between a CPU
// access and a video access, each taking 8 32MHz cycles.  The M68000
// only tries to do accesses on even cycles of the 8Mhz clock, and
// every access uses, from its point of view, 4 cycles, of which only
// two need driving the data bus.  So the CPU can do back-to-back
// 4-cycle accesses with the MMU transparently wedging the video
// accesses in the middle.  When for some rare instructions the access
// is shifted by 2 cycles the MMU delays the CPU to keep it on a
// multiple of 4 using the dtack signal.  This happens for accesses to
// RAM and to circuits that are "behind" the MMU, which the shifter
// is.  So accesses to shifter registers are also forced to multiples
// of 4.  Accesses to the MMU and GLUE registers do not have that
// constraint.

// Synchronization-wise, we use the CPU as reference and set its
// accesses at 32MHz clock 16n.  The video ram accessed are at clock
// 16n+8.

// The shifter pixel clock at 640x400 is every cycle.  The pixel clock
// at 640x200 is at clock 2n (it is used as clock input in the MMU).

// The pixel clock at 320x200 is at either 4n or 4n+2, chosen randomly
// at startup.  It is called the shifter wakeup state, or shifter wake
// state or even shifter WS.

// The 2Mhz GLUE clock is derived from the MMU 8Mhz clock, which puts
// it at 16n, 16n+4, 16n+8 or 16n+12, once again randomly.  That's the
// GLUE WS.  GLUEs tend to "choose" one WS when cold and a different
// one when warm (when power-cycling after some use, the reset pin
// does not change the phase unless the Test1 pin is set at the same
// time) and almost never reach the other two.

// The WS do not matter when the video is used "normally", changing
// video modes at vsync time.  Of course demomakers tried to go much
// farther, and the art was to play with the GLUE registers to make it
// activate the display enable signal for a much larger area than is
// normal (overscan) without making the shifter have a fit.  This was
// made possible by the glue triggering state changes using equality
// comparators on a series of counters, so a mode change at just the
// right time can ensure that a comparison succeeds or fails.  In
// addition to overscan this approach was used to (virtually) start
// the screen at any memory address (and not only a multiple of 256
// which is all the MMU supports) and even shift the screen
// horizontally by multiples of 4 pixels in hardware.


// Video signals generation by the GLUE:

// The GLUE generates fives signals:

// - hsync and vsync, the synchronization signals sent to the screen.
//   hsync is in addition connected to ipl2 and vsync to ipl4.

// - hblank and vblank, used in color modes only to force the output
//   to black to avoid the background color messing with the sync
//   signals and, when existing, the colorburst.  They're not used in
//   monochrome mode.

// - DE, display enable, tells the shifter and the MMU that pixels
//   should be visible.  It is also connected to the mfp timer b input,
//   making it count/interrupt at the end of the visible lines.


// Sync/blank uses a 7-bit counter for h and 9-bit for v.  They are
// loaded with a value (*hbase) at start of line/screen and count up
// until overflow.  The horizontal counter hsc counts on the 2Mhz
// internal GLUE clock.  The vertical counter vsc counts when hsync
// has a raising edge (e.g. hsync ends, the sync and blank signals are
// active low).  Vsync always stops at vsc overflow (512).

// Note that selecting 640x400 also selects 71Hz while 320x200 and
// 640x200 can be either 50 or 60Hz.  The GLUE itself does not care
// which mode it is between 320 and 640.

// freq         50         60         71
// hbase         0          1         72
// hs 0        102        102        122
// hs 1        112        112         72 (128)
// hb 0         98         99          -
// hb 1        121        121          -
// vbase       199        249         11
// vs 0        509        509        511
// vs 1        512        512        512
// vb 0        506        506          -
// vb 1        223        264          -
// area    128x313    127x263     56x501

// DE (display enable) also uses a pair of 7/9 bits counters which are
// set to 0 as long as the associated sync is 0 and count up when it's
// 1.  HDE counter first increments 0.5 GLUE cycles (8 Y2) after hsync
// goes 1, then every cycle (16 Y2) afterwards.  VDE counter
// increments when hsync goes 1, same as vsc.  Tests for changing de
// happen at the same time as the counter changes.

// freq         50         60         71
// hde 1        17         16          4
// hde 0        97         96         44

// vde 1        63         34         36
// vde 0       263        234        436

// DE is on whole cycles of the GLUE clock as vde & hde.  Which means,
// given that hde is on the inverted phase, that there is a delay of
// 0.5 cycles between a hde change and the corresponding de change.
//
// hde/vde are dropped by the associated sync if they were still 1 at
// that point.

// load_d1 = 0 if !de, 1 if load
// load_d2 = load_d1 on pixclk
// reload_delay_n = !reload on pixclk
// pxcntren = 1 if load_d1 up, load_d2 on reload down, 0 at reset
// rdelay = 0 if !reload_delay_n on down , 1 | (rdelay >> 1) if load up
// reload = (0 if rdelay[0] == 1 else pixcntr != 0) on pixclk
// pixcntr = (pixcntr+1 if pxctren else 4) on pixclk

// shift on pixclk
// load 3 on pixclk + load

// Timing in 32MHz cycles of a 50/60Hz line in a range when vde=1:
//
//    0   0.0 hde = 1
//    8   0.8 de = 1
//   23   1.7 address
//   28   1.c load = 0
//   31   1.f -address
//   32   2.0 load = 1
//   44   2.c load = 0
//   48   3.0 load = 1
// ...
// 1280  80.0 hde = 0
// 1288  80.8 de = 0
// 1292  80.c load = 0
// 1300  81.0 load = 1


// Shifter gets the 32MHz clock.  It outputs a 16MHz clock, and
// generates the pixel clocks internally (32, 16 or 8 Mhz).
//
// MMU gets the 16MHz clock, and generates a 8MHz and 4MHz clock.
//
// GLUE gets the 8MHz clock.

/*

bp a3fe
phase 4
[:video] [13329804.0 00a420 130820   8176 53319216 53319216  112/262 001/064 s:-- de:-v- 71Hz] res = 2 (2)     (ste only)
[:video] [13329806.0 00a422 130852   8178 53319224 53319224  114/262 003/064 s:-- de:-v- 50Hz] res = 0 (0)
[:video] [13329815.0 00a42c 130996   8187 53319260 53319260  123/262 012/064 s:-- de:-v- 60Hz] sync = 0 (1)    (+2 on 1/3)
[:video] [13329818.8 00a430 131052   8190 53319274 53319274  126/262 015/064 s:-- de:-v- 50Hz] sync = 2 (0)
[:video] [13329819.c 00a430 131072   8192 53319279 53319279  000/263 017/064 s:-- de:-v- 50Hz] hde 1
[:video] [13329820.4 00a430 131080   8192 53319281 53319281  000/263 017/064 s:-- de:hvd 50Hz] de 1 lc=53319286 le=0
[:video] [13329847.0 00a438 131508   8219 53319388 53319388  027/263 044/064 s:-- de:hvd 71Hz] res = 2 (2)
[:video] [13329849.0 00a43a 131540   8221 53319396 53319396  029/263 046/064 s:-- de:hvd 50Hz] res = 0 (0)     (stop on ws2)
[:video] [13329899.8 00a444 132348   8271 53319598 53319598  079/263 096/064 s:-- de:hvd 71Hz] res = 2 (2)
[:video] [13329902.0 00a446 132388   8274 53319608 53319608  082/263 099/064 s:-- de:hvd 50Hz] res = 0 (0)     (right border 2/3/4, avoids 97)
delta = cc

phase 3




phase 1:
    left border on ste?

[:video] [454400  28400 13309964  112/420 000/221 s:h- de:-v- 50Hz] glue sync
[:video] [454399  28399 13309963  111/420 000/222 s:-- de:-v- 71Hz] glue sync
[:video] [454399  28399 13309963  111/420 000/222 s:-- de:-v- 71Hz] res = 2 (2)
[:video] [454431  28401 13309965  113/420 002/222 s:-- de:-v- 50Hz] glue sync
[:video] [454431  28401 13309965  113/420 002/222 s:-- de:-v- 50Hz] res = 0 (0)

    +2 on ws1/ws3 but not on ws2/ws4.  move, exg, move.  test at hdec=16

[:video] [454575  28410 13309974  122/420 011/222 s:-- de:-v- 60Hz] glue sync
[:video] [454575  28410 13309974  122/420 011/222 s:-- de:-v- 60Hz] sync = 0 (1)
[:video] [454631  28414 13309978  126/420 015/222 s:-- de:-v- 50Hz] glue sync
[:video] [454631  28414 13309978  126/420 015/222 s:-- de:-v- 50Hz] sync = 2 (0)
[:video] [454656  28416 13309980  128/420 017/222 s:-- de:-v- 50Hz] glue sync
[:video] [454656  28416 13309980  000/421 017/222 s:-- de:-v- 50Hz] hde 1

    screen stop on ws2, test at hdec=44

[:video] [455087  28442 13310006  026/421 043/222 s:-- de:hvd 71Hz] glue sync
[:video] [455087  28442 13310006  026/421 043/222 s:-- de:hvd 71Hz] res = 2 (2)
[:video] [455088  28443 13310007  027/421 044/222 s:-- de:hvd 71Hz] glue sync
[:video] [455088  28443 13310007  027/421 044/222 s:-- de:hvd 71Hz] hde 0
[:video] [455096  28443 13310007  027/421 044/222 s:-- de:-v- 71Hz] de 0 lc=53240030 le=53240034
[:video] [455096  28443 13310007  027/421 044/222 s:-- de:-v- 71Hz] load rd=e
[:video] [455119  28444 13310008  028/421 045/222 s:-- de:-v- 50Hz] glue sync
[:video] [455119  28444 13310008  028/421 045/222 s:-- de:-v- 50Hz] res = 0 (0)

    right border is still active, test at hdec=97

[:video] [455927  28495 13310059  079/421 096/222 s:-- de:-v- 71Hz] glue sync
[:video] [455927  28495 13310059  079/421 096/222 s:-- de:-v- 71Hz] res = 2 (2)
[:video] [455967  28497 13310061  081/421 098/222 s:-- de:-v- 50Hz] glue sync
[:video] [455967  28497 13310061  081/421 098/222 s:-- de:-v- 50Hz] res = 0 (0)


-> detect as ws2

    phase ?

[:video] [454407  28400 53239859 53239860  112/420 001/222 s:-- de:-v- 71Hz] glue sync
[:video] [454407  28400 53239859 53239860  112/420 001/222 s:-- de:-v- 71Hz] res = 2 (2)
[:video] [454439  28402 53239867 53239868  114/420 003/222 s:-- de:-v- 50Hz] glue sync
[:video] [454439  28402 53239867 53239868  114/420 003/222 s:-- de:-v- 50Hz] res = 0 (0)

[:video] [454583  28411 53239903 53239904  123/420 012/222 s:-- de:-v- 60Hz] glue sync
[:video] [454583  28411 53239903 53239904  123/420 012/222 s:-- de:-v- 60Hz] sync = 0 (1)
[:video] [454640  28415 53239918 53239918  127/420 016/222 s:-- de:-v- 60Hz] glue sync
[:video] [454640  28415 53239918 53239918  127/420 016/222 s:-- de:-v- 60Hz] hde 1
[:video] [454639  28414 53239917 53239918  126/420 015/222 s:-- de:hv- 50Hz] glue sync
[:video] [454639  28414 53239917 53239918  126/420 015/222 s:-- de:hv- 50Hz] sync = 2 (0)

*/


#define LOG_MODE   (1U << 1) // Shows video mode real changes
#define LOG_SYNC   (1U << 2) // Shows syncs
#define LOG_EVENT  (1U << 3) // Shows timer events

#define VERBOSE (LOG_MODE|LOG_SYNC|LOG_EVENT)

//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGMODE(...)   LOGMASKED(LOG_MODE,  __VA_ARGS__)
#define LOGSYNC(...)   LOGMASKED(LOG_SYNC,  __VA_ARGS__)
#define LOGEVENT(...)  LOGMASKED(LOG_EVENT, __VA_ARGS__)

DEFINE_DEVICE_TYPE(ST_VIDEO, st_video_device, "st_video", "Atari ST Video")

const u16 st_video_device::base_vsc[3]    = { 199, 249,  11 };
const u16 st_video_device::base_hsc[3]    = {   0,   1,  72 };
const u16 st_video_device::vsync_start[3] = { 509, 509, 511 };
const u16 st_video_device::vde_start[3]   = {  63,  34,  36 };
const u16 st_video_device::vde_end[3]     = { 263, 234, 436 };

const u32 st_video_device::cycles_per_screen[3] = { 128*313, 127*263, 56*501 };

static INPUT_PORTS_START(stvideo)
	PORT_START("config")
	PORT_CONFNAME( 15, 4, "GLUE wake state")
	PORT_CONFSETTING( 12, "DL3 / WS2" )
	PORT_CONFSETTING(  8, "DL4 / WS4" )
	PORT_CONFSETTING(  4, "DL5 / WS3" )
	PORT_CONFSETTING(  0, "DL6 / WS1" )
INPUT_PORTS_END

ioport_constructor st_video_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(stvideo);
}


st_video_device::st_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ST_VIDEO, tag, owner, clock),
	device_video_interface(mconfig, *this),
	device_palette_interface(mconfig, *this),
	m_phase(*this, "config"),
	m_de_cb(*this),
	m_hsync_cb(*this),
	m_vsync_cb(*this),
	m_ram(*this, finder_base::DUMMY_TAG),
	m_mmu(*this, finder_base::DUMMY_TAG),
	m_screen_bitmap(1024, 512)
{
}

void st_video_device::device_config_complete()
{
	if(has_screen())
		screen().set_raw(clock()/2, 512*2, 28*2, 450*2, 313, 34, 292);
}

void st_video_device::device_start()
{
	m_event_timer = timer_alloc(FUNC(st_video_device::timer_event), this);
	m_de_timer    = timer_alloc(FUNC(st_video_device::de_event   ), this);

	save_item(NAME(m_start_screen_time));
	save_item(NAME(m_shifter_base_x));
	save_item(NAME(m_shifter_update_time));
	save_item(NAME(m_prev_glue_tick));
	save_item(NAME(m_load_current));
	save_item(NAME(m_load_end));
	save_item(NAME(m_ir));
	save_item(NAME(m_rr));
	save_item(NAME(m_adr_base));
	save_item(NAME(m_adr_live));
	save_item(NAME(m_hsc_base));
	save_item(NAME(m_hdec_base));
	save_item(NAME(m_vsc));
	save_item(NAME(m_vdec));
	save_item(NAME(m_shifter_y));
	save_item(NAME(m_palette));
	save_item(NAME(m_sync));
	save_item(NAME(m_res));
	save_item(NAME(m_vsync));
	save_item(NAME(m_hsync));
	save_item(NAME(m_vde));
	save_item(NAME(m_hde));
	save_item(NAME(m_de));
	save_item(NAME(m_mode));
	save_item(NAME(m_pixcnt));
	save_item(NAME(m_rdelay));
	save_item(NAME(m_load_1));
	save_item(NAME(m_load_2));
	save_item(NAME(m_pixcnt_en));
	save_item(NAME(m_reload));
}

u64 st_video_device::time_now()
{
	// as_ticks is not clean, so round up
	return (machine().time().as_ticks(2*clock()) + 1) >> 1;
}

void st_video_device::next_event(u64 when)
{
	if(v)
		logerror("%s next event at %d.%x\n", context(), when >> 4, when & 15);
	// from_ticks/as_ticks is not stable, need target in the middle of the cycle
	attotime dt = attotime::from_ticks(2*when+1, 2*clock()) - machine().time();
	m_event_timer->adjust(dt);
}

void st_video_device::device_reset()
{
	m_sync = 3;
	m_res = 0;
	m_mode = M_60;
	m_hsc_base = 128;
	m_hdec_base = 0;
	m_vsc = 511;
	m_vdec = 0;
	m_vsync = 1;
	m_hsync = 0;
	m_start_screen_time = time_now() >> 4;
	m_hsc_base = m_hdec_base = 0;
	m_shifter_update_time = (time_now() | 15) + 1;
	m_shifter_base_x = 0;
	m_pixcnt = 4;
	m_pixcnt_en = false;
	m_rdelay = 0;
	m_load_1 = false;
	m_load_2 = false;
	m_reload = false;
	m_load_current = 0;
	m_load_end = 0;
	m_adr_base = 0;
	m_adr_live = 0;
	m_de = 0;
	m_vde = 0;
	m_hde = 0;
	m_rr[0] = m_rr[1] = m_rr[2] = m_rr[3] = 0;
	m_ir[0] = m_ir[1] = m_ir[2] = m_ir[3] = 0;
	m_prev_glue_tick = 0;

	m_screen_bitmap.fill(0x808080);

	next_event(m_start_screen_time);
}

void st_video_device::map(address_map &map)
{
	map(0x201, 0x201).rw(FUNC(st_video_device::adr_base_h_r), FUNC(st_video_device::adr_base_h_w));
	map(0x203, 0x203).rw(FUNC(st_video_device::adr_base_m_r), FUNC(st_video_device::adr_base_m_w));
	map(0x205, 0x205).r(FUNC(st_video_device::adr_live_h_r));
	map(0x207, 0x207).r(FUNC(st_video_device::adr_live_m_r));
	map(0x209, 0x209).r(FUNC(st_video_device::adr_live_l_r));
	map(0x20a, 0x20a).rw(FUNC(st_video_device::sync_r), FUNC(st_video_device::sync_w));
	map(0x240, 0x25f).rw(FUNC(st_video_device::palette_r), FUNC(st_video_device::palette_w));
	map(0x260, 0x260).rw(FUNC(st_video_device::res_r), FUNC(st_video_device::res_w));
}

u32 st_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_screen_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}

void st_video_device::update_mode()
{
	m_mode = m_res & 2 ? M_71 : m_sync & 2 ? M_50 : M_60;
	glue_determine_next_event();
}

void st_video_device::hsync_on()
{
	if(!m_hsync) {
		m_hsync = 1;
		m_hsync_cb(1);
	}
}

void st_video_device::hsync_off(u64 glue_tick)
{
	if(m_hsync) {
		m_shifter_base_x = (glue_tick << 3) + m_phase->read() - 32;
		m_shifter_y ++;

		m_hdec_base = glue_tick - 1;

		m_hsync = 0;
		m_hsync_cb(0);
		if(m_vsync)
			return;

		m_vdec ++;
		if(m_vdec == vde_start[m_mode])
			m_vde = 1;
		else if(m_vdec == vde_end[m_mode]) {
			m_vde = 0;
			if(m_de)
				next_de_event((glue_tick | 1) + 1, 0);
		}

		if(v)
			logerror("%s hsync 0\n", context());
	}
}

void st_video_device::de_set(bool level)
{
	u64 now = time_now();
	shifter_sync(now);
	m_de = level;
	m_de_cb(m_de);
	if(level) {
		m_load_current = (now & ~15) + 16 + 8;
		m_load_end = 0;
	} else
		m_load_end = (now & ~15) + 16 + 8;

	if(v)
		logerror("%s de %d lc=%d le=%d delta=%d\n", context(now), m_de, m_load_current/4, m_load_end/4, m_load_current - now);
}

TIMER_CALLBACK_MEMBER(st_video_device::de_event)
{
	de_set(param);
}

void st_video_device::next_de_event(u64 glue_tick, int level)
{
	u64 when = (glue_tick << 3) + m_phase->read() - 32;
	attotime dt = attotime::from_ticks(2*when+1, 2*clock()) - machine().time();
	m_de_timer->adjust(dt, level);
}

void st_video_device::hde_on(u64 glue_tick)
{
	if(v)
		logerror("%s hde 1\n", context());
	if(m_vde && !m_hde) {
		m_hde = 1;
		if(m_mode == 0) {
			m_pixcnt_en = false;
			m_rdelay = 0;
		}
		next_de_event((glue_tick | 1) + 1, 1);
	}
}

void st_video_device::hde_off(u64 glue_tick)
{
	if(v)
		logerror("%s hde 0\n", context());
	if(m_hde) {
		m_hde = 0;
		next_de_event((glue_tick | 1) + 1, 0);
	}
}

void st_video_device::shifter_handle_load()
{
	m_pixcnt = m_pixcnt_en ? (m_pixcnt + 1) & 15 : 4;
	if(m_load_1 && !m_load_2) {
		if(v)
			logerror("%s activating pixcnt_en\n", context(m_shifter_update_time));
		m_pixcnt_en = true;
	}
	m_load_2 = m_load_1;
	if(!m_de)
		m_load_1 = false;
	if(m_shifter_update_time == m_load_end)
		m_load_current = 0;
	if(m_shifter_update_time == m_load_current) {
		// load edge
		m_ir[0] = m_ir[1];
		m_ir[1] = m_ir[2];
		m_ir[2] = m_ir[3];
		m_ir[3] = m_ram[m_adr_live >> 1];
		m_adr_live = (m_adr_live + 2) & 0x3fffff;
		m_rdelay = 8 | (m_rdelay >> 1);
		m_load_current += 16;
		if(m_de)
			m_load_1 = true;

		if(v)
			logerror("%s load rd=%x adr=%06x\n", context(m_shifter_update_time), m_rdelay, m_adr_live);
	}

	if(m_reload) {
		if(v)
			logerror("%s reload\n", context(m_shifter_update_time));
		m_rr[3] = m_ir[3];
		m_rr[2] = m_ir[2];
		m_rr[1] = m_ir[1];
		m_rr[0] = m_ir[0];
		m_rdelay = 0;
		m_pixcnt_en = m_load_2;
	}

	m_reload = (m_rdelay & 1) && !m_pixcnt;
}

void st_video_device::shifter_sync(u64 now)
{
	if(!now)
		now = time_now();
	switch(m_res & 3) {
	case 0: {
		if(m_shifter_update_time & 3) {
			if((now & ~3) <= m_shifter_update_time)
				return;
			m_shifter_update_time = (m_shifter_update_time | 3) + 1;
		}

		// Expand pixels by a factor of 2 to make mid/low resolutions
		// combinable on the same screen

		u32 x = 2*((m_shifter_update_time - m_shifter_base_x) >> 2);
		u32 *dest = &m_screen_bitmap.pix(m_shifter_y >= 512 ? 511 : m_shifter_y, x);
		while(m_shifter_update_time < now) {
			u8 color = 0;
			m_rr[0] = (m_rr[0] << 1);
			m_rr[1] = (m_rr[1] << 1);
			m_rr[2] = (m_rr[2] << 1);
			m_rr[3] = (m_rr[3] << 1);
			shifter_handle_load();

			color |= m_rr[3] & 0x8000 ? 8 : 0;
			color |= m_rr[2] & 0x8000 ? 4 : 0;
			color |= m_rr[1] & 0x8000 ? 2 : 0;
			color |= m_rr[0] & 0x8000 ? 1 : 0;

			if(x < 1024) {
				auto c = pen(color);
				*dest++ = c;
				*dest++ = c;
			}
			x += 2;
			m_shifter_update_time += 4;
		}

		break;
	}

	case 1: {
		if(m_shifter_update_time & 1) {
			if((now & ~1) <= m_shifter_update_time)
				return;
			m_shifter_update_time = (m_shifter_update_time | 1) + 1;
		}

		u32 x = (m_shifter_update_time - m_shifter_base_x) >> 1;
		u32 *dest = &m_screen_bitmap.pix(m_shifter_y >= 512 ? 511 : m_shifter_y, x);
		while(m_shifter_update_time < now) {
			u8 color = 0;
			m_rr[0] = (m_rr[0] << 1) | (m_rr[2] & 0x8000 ? 1 : 0);
			m_rr[1] = (m_rr[1] << 1) | (m_rr[3] & 0x8000 ? 1 : 0);
			m_rr[2] = (m_rr[2] << 1);
			m_rr[3] = (m_rr[3] << 1);
			shifter_handle_load();

			color |= m_rr[1] & 0x8000 ? 2 : 0;
			color |= m_rr[0] & 0x8000 ? 1 : 0;

			if(x < 1024)
				*dest++ = pen(color);
			x += 1;
			m_shifter_update_time += 2;
		}

		break;
	}

	case 2: {
		u32 x = m_shifter_update_time - m_shifter_base_x;
		u32 *dest = &m_screen_bitmap.pix(m_shifter_y >= 512 ? 511 : m_shifter_y, x);
		while(m_shifter_update_time < now) {
			u8 color = 0;
			u8 mono = m_palette[0] & 1;
			m_rr[0] = (m_rr[0] << 1) | (m_rr[1] & 0x8000 ? 1 : 0);
			m_rr[1] = (m_rr[1] << 1) | (m_rr[2] & 0x8000 ? 1 : 0);
			m_rr[2] = (m_rr[2] << 1) | (m_rr[3] & 0x8000 ? 1 : 0);
			m_rr[3] = (m_rr[3] << 1) | (mono ^ 1);
			shifter_handle_load();

			color |= m_rr[0] & 0x8000 ? 1 : 0;
			if(x < 1024)
				*dest++ = color^mono ? 0xffffff : 0x000000;
			x += 1;
			m_shifter_update_time += 1;
		}

		break;
	}
	}
}

std::string st_video_device::context(u64 now)
{
	static const char *const mt[3] = { "50Hz", "60Hz", "71Hz" };
	if(!now)
		now = time_now();

	auto [glue_tick, hsc, hdec] = compute_glue_tick(now);

	u64 mtick = machine().root_device().subdevice<cpu_device>("m68000")->total_cycles();
	u32 pc = machine().root_device().subdevice<cpu_device>("m68000")->pcbase();

	return util::string_format("[%s %06x %6d.%d %6d %6d %4d %4d %03d/%03d %03d/%03d s:%c%c de:%c%c%c %s]",
							   util::string_format("%08d.%x", now >> 4, now & 15),
							   pc,
							   (glue_tick - m_start_screen_time) >> 1,
							   (glue_tick - m_start_screen_time) & 1,
							   mtick,
							   glue_tick,
							   glue_tick - m_hsc_base,
							   glue_tick - m_hdec_base,
							   hsc, m_vsc,
							   hdec, m_vdec,
							   m_hsync ? 'h' : '-', m_vsync ? 'v' : '-',
							   m_hde ? 'h' : '-', m_vde ? 'v' : '-',
							   m_de ? 'd' : '-',
							   mt[m_mode]);
}

std::tuple<u64, u16, u16> st_video_device::compute_glue_tick(u64 now)
{
	// Glue tick is doubled to count up and down edges

	u64 phase = m_phase->read();
	u64 glue_tick = (now + 32 - phase) >> 3; // Avoid negative values after adjustement, with some margin

	u16 hsc = (glue_tick - m_hsc_base) >> 1;
	u16 hdec = m_hsync ? 0 : ((glue_tick - m_hdec_base) >> 1) & 127;
	return std::make_tuple(glue_tick, hsc, hdec);
}

void st_video_device::glue_sync()
{
	static const char *const mt[3] = { "50Hz", "60Hz", "71Hz" };
	u64 now = time_now();
	shifter_sync(now);

	auto [glue_tick, hsc, hdec] = compute_glue_tick(now);

	if(glue_tick == m_prev_glue_tick)
		return;

	bool hsc_edge  = (glue_tick >= m_prev_glue_tick+2) || !(glue_tick & 1);
	bool hdec_edge = (glue_tick >= m_prev_glue_tick+2) ||  (glue_tick & 1);

	m_prev_glue_tick = glue_tick;

	if(v)
		logerror("%s glue sync tick=%d.%d\n", context(), glue_tick >> 1, glue_tick & 1);

	if(hsc >= 128) {
		if(m_mode == M_71) {
			hsync_off(glue_tick);
			hdec = 0;
		}
		m_vsc ++;
		if(m_vsc == 512) {
			m_vsync = 0;
			m_adr_live = m_adr_base;
			m_vdec = 0;
			m_vsync_cb(0);
			u64 dt = glue_tick - m_start_screen_time;
			// We put the (0, 0) position at hsync off, vsync off.  We
			// remove the blanked zone at 50 and 60Hz.

			if(dt != 2*cycles_per_screen[m_mode]) {
				logerror("Video mode: %s\n", mt[m_mode]);
				logerror("dt=%d cps=%d / %d\n", dt, cycles_per_screen[m_mode], 2*cycles_per_screen[m_mode]);
				switch(m_mode) {
				case 0: screen().configure(128*8, 313, rectangle(8*(121-112), 8*(128-112 + 98-0), 223-199, 506-199-1), attotime::from_ticks(16*cycles_per_screen[m_mode], clock()).as_attoseconds()); break;
				case 1: screen().configure(127*8, 263, rectangle(8*(121-112), 8*(128-112 + 99-1), 264-249, 506-249-1), attotime::from_ticks(16*cycles_per_screen[m_mode], clock()).as_attoseconds()); break;
				case 2: screen().configure(56*16, 501, rectangle(0, 16*(122-72), 0, 511-11-1), attotime::from_ticks(16*cycles_per_screen[m_mode], clock()).as_attoseconds()); break;
				}
			}
			m_vsc = base_vsc[m_mode];
			m_start_screen_time = glue_tick;
			m_shifter_y = 0;

		} else if(m_vsc == vsync_start[m_mode]) {
			m_vsync = 1;
			m_vsync_cb(1);
			m_vde = 0;
			if(m_de)
				next_de_event((glue_tick | 1) + 1, 0);
			m_adr_live = m_adr_base;
		}

		hsc = base_hsc[m_mode];
		m_hsc_base = glue_tick - 2*hsc;
	}

	switch(m_mode) {
	case M_50:
		if(hsc_edge) {
			if(hsc == 102) {
				hsync_on();
				if(m_hde)
					hde_off(glue_tick);
			}
			else if(hsc == 112) {
				hsync_off(glue_tick);
				hdec = 0;
				m_hdec_base = glue_tick - 1;
			}
		}
		if(hdec_edge) {
			if(hdec == 17)
				hde_on(glue_tick);
			else if(hdec == 97)
				hde_off(glue_tick);
		}
		break;

	case M_60:
		if(hsc_edge) {
			if(hsc == 102) {
				hsync_on();
				if(m_hde)
					hde_off(glue_tick);
			} else if(hsc == 112) {
				hsync_off(glue_tick);
				hdec = 0;
			}
		}
		if(hdec_edge) {
			if(hdec == 16)
				hde_on(glue_tick);
			else if(hdec == 96)
				hde_off(glue_tick);
		}
		break;

	case M_71:
		if(hsc_edge) {
			if(hsc == 122) {
				hsync_on();
				if(m_hde)
					hde_off(glue_tick);
			}
		}
		if(hdec_edge) {
			if(hdec == 4)
				hde_on(glue_tick);
			else if(hdec == 44)
				hde_off(glue_tick);
		}
		break;
	}
}


void st_video_device::glue_determine_next_event()
{
	u64 now = time_now();

	auto [glue_tick, hsc, hdec] = compute_glue_tick(now);

	u16 hsc_next = 128;
	u16 hdec_next = 128;

	switch(m_mode) {
	case M_50:
		if(hsc < 102)
			hsc_next = 102;
		else if(hsc < 112)
			hsc_next = 112;
		if(hdec < 17)
			hdec_next = 17;
		else if(hdec < 97)
			hdec_next = 97;
		break;

	case M_60:
		if(hsc < 102)
			hsc_next = 102;
		else if(hsc < 112)
			hsc_next = 112;
		if(hdec < 16)
			hdec_next = 16;
		else if(hdec < 96)
			hdec_next = 96;
		break;

	case M_71:
		if(hsc < 122)
			hsc_next = 122;
		if(hdec < 4)
			hdec_next = 4;
		else if(hdec < 44)
			hdec_next = 44;
		break;
	}


	u16 hsc_delta = ((hsc_next - hsc) << 1) - (glue_tick & 1);
	u16 hdec_delta = ((hdec_next - hdec) << 1) - (~glue_tick & 1);

	u16 delta = hsc_delta < hdec_delta || m_hsync ? hsc_delta : hdec_delta;

	if(v) {
		u64 gt = glue_tick + delta - m_start_screen_time;
		logerror("%s next hsc %d hdec %d targetting %d.%d\n", context(), hsc_next, hdec_next, gt >> 1, gt & 1);
	}

	next_event(((glue_tick + delta) << 3) + m_phase->read() - 32);
}

TIMER_CALLBACK_MEMBER(st_video_device::timer_event)
{
	glue_sync();
	glue_determine_next_event();
}

void st_video_device::adr_base_h_w(u8 data)
{
	m_adr_base = (m_adr_base & 0x00ffff) | (data << 16);
}

u8 st_video_device::adr_base_h_r()
{
	return m_adr_base >> 16;
}

void st_video_device::adr_base_m_w(u8 data)
{
	m_adr_base = (m_adr_base & 0xff00ff) | (data << 8);
}

u8 st_video_device::adr_base_m_r()
{
	return m_adr_base >> 8;
}

u8 st_video_device::adr_live_h_r()
{
	shifter_sync();
	return m_adr_live >> 16;
}

u8 st_video_device::adr_live_m_r()
{
	shifter_sync();
	return m_adr_live >> 8;
}

u8 st_video_device::adr_live_l_r()
{
	shifter_sync();
	logerror("%s adr %06x - %06x\n", context(), m_adr_base, m_adr_live);
	u32 pc = machine().root_device().subdevice<cpu_device>("m68000")->pcbase();
	if(pc == 0x8644)
		v = false; // level 16
	if(pc == 0x84f0)
		v = false; // main menu
	//  v = true;
	return m_adr_live;
}


void st_video_device::sync_w(u8 data)
{
	shifter_sync();
	m_sync = data & 0x03;
	update_mode();
	logerror("%s sync = %d (%d)\n", context(), m_sync, m_mode);
}

u8 st_video_device::sync_r()
{
	return m_sync;
}


void st_video_device::res_w(u8 data)
{
	shifter_sync();
	m_res = data & 0x03;
	update_mode();
	logerror("%s res = %d (%d)\n", context(), m_res, m_mode);
}

u8 st_video_device::res_r()
{
	return m_res;
}

void st_video_device::palette_w(offs_t offset, u16 data, u16 mem_mask)
{
	shifter_sync();

	static const u8 exp[8] = { 0x00, 0x24, 0x49, 0x6d, 0x92, 0xb6, 0xdb, 0xff };
	COMBINE_DATA(&m_palette[offset]);
	u16 color = m_palette[offset];
	set_pen_color(offset, exp[(color >> 8) & 7], exp[(color >> 4) & 7], exp[color & 7]);
}

u16 st_video_device::palette_r(offs_t offset)
{
	return m_palette[offset] | 0xf888;
}


