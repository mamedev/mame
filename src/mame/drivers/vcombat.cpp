// license:BSD-3-Clause
// copyright-holders:Jason Eckhardt, Andrew Gardner, Philip Bennett
/*
Virtual Combat hardware games.

Driver by Jason Eckhardt, Andrew Gardner and Phil Bennett.

----

There are two known games on this hardware.  Both are developed by
Kyle Hodgetts.

Virtual Combat (c) VR8 Inc. 1993
http://arcade.sonzogni.com/VRCombat/

Shadow Fighters (German) (c) Dutech 1993

----

There are two boards to this hardware.  The upper, which contains the
graphics ROMs and the i860, and the lower which contains the main
and sound CPUs.  Virtual Combat sports two upper boards which
output a different rasterization of the scene for each stereo eye.

UPPER: (Virtual Combat has an identical MIDDLE board also).
    Intel I860 XR processor
    MB8298-25P-SK RAMS x12 (silkscreen said 62256)
    Analog device ADV476KN50E (silkscreen said BT476)
    20 MHZ Oscillator
    8-way DIP switch
    574200D x4
    PAL palce24v10 x2 (next to the i860)
    Bt476 RAMDAC

LOWER:
    Motorola MC68000P12 x2
    12 MHz Oscillator x2
    Harris ADC0804LCN x2
    4 MB8298-25P-SK RAMS (in groups of 2 off by themselves)
    1 CXK58257SP-10L at each end of the SNDCPU ROMS and the CPU ROMS (4 chips total)
    Motorola MC6845P CRT controller
    2x 27C010A containing sound code
    Xx 27C040 containing sound data (VOC files)
    Dallas DS1220Y - closest to pin 64 of CPU - read as a 2716 - (silkscreened "6116")
    Xx 27c040 containing program code, etc.

----

NOTES : Shadow Fighters appears to have been dumped from an earlier
            revision of the hardware.  There are no IC labels, and
            lots of factory rework has been done to the bottom board.
        Because the board was so early for Shadow Fighters, there were
            no IC locations silkscreened on the PCB.  The locations
            from Virtual Combat have been used.
        The Shadow Fighters bottom board has an extra 20 mhz xtal on it.
        The data stored in "samples" is simply a series of
            Creative Media VOC files concatenated to each other.
        The sound program ("sound") is about 640 bytes long.
        The hardware is said to run at medium resolution.
        The SRAM module dump can likely be thrown away for both games.
        The PAL that's dumped for Shadow Fighters looks pretty bad.

TODO :  This is a partially working driver.  Most of the memory maps for
            all four CPUs are complete.  An Intel i860XR CPU core has now
            been incorporated into MAME.

    --------                Notes/questions                  ----------
     - Most of the memory maps and input ports are complete now.
       For "main", the only I/O locations that seem to be left
       mysterious are:
         0x60001C: Only read; might be a latch of some sort.  It is
               read each time M0's interrupt service routine syncs
               with the i860's to have them update the frame buffer.
               Might this be a HW blank bit so things look clean when
               the i860's do their updates?
               The two other times I see it read are just before
               and after one of the palette setups is done.
         0x600018: ? No info yet.
         0x704000: (VC only) Likely analog axis for VR headset
         0x703000: (VC only) Likely analog axis for VR headset

    ----------------------------------------------
*/

#include "emu.h"
#include "cpu/i860/i860.h"
#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/mc6845.h"
#include "video/tlc34076.h"
#include "layout/generic.h"
#include "screen.h"
#include "speaker.h"

#define VERBOSE 0
#include "logmacro.h"


class vcombat_state : public driver_device
{
public:
	vcombat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_tlc34076(*this, "tlc34076"),
		m_framebuffer_ctrl(*this, "fb_control"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_vid(*this, "vid_%u", 0U),
		m_dac(*this, "dac"),
		m_crtc(*this, "crtc"),
		m_vid_0_ram(*this, "vid_0_ram"),
		m_vid_1_ram(*this, "vid_1_ram"),
		m_m0_p0_c1(*this, "m0_p0_c1"),
		m_m0_p0_c2(*this, "m0_p0_c2"),
		m_m0_p1_c1(*this, "m0_p1_c1"),
		m_m0_p1_c2(*this, "m0_p1_c2")
	{ }

	void init_shadfgtr();
	void init_vcombat();

	void shadfgtr(machine_config &config);
	void vcombat(machine_config &config);

protected:
	virtual void machine_reset() override;

private:
	required_device<tlc34076_device> m_tlc34076;
	required_shared_ptr<uint16_t> m_framebuffer_ctrl;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device_array<i860_cpu_device, 2> m_vid;
	required_device<dac_word_interface> m_dac;
	optional_device<mc6845_device> m_crtc;
	required_shared_ptr<uint64_t> m_vid_0_ram;
	optional_shared_ptr<uint64_t> m_vid_1_ram;
	required_shared_ptr<uint64_t> m_m0_p0_c1;
	required_shared_ptr<uint64_t> m_m0_p0_c2;
	optional_shared_ptr<uint64_t> m_m0_p1_c1;
	optional_shared_ptr<uint64_t> m_m0_p1_c2;

	std::unique_ptr<uint16_t[]> m_m68k_framebuffer[2];
	std::unique_ptr<uint16_t[]> m_i860_framebuffer[2][2];
	int m_crtc_select;

	void main_video_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t control_1_r();
	uint16_t control_2_r();
	uint16_t control_3_r();
	void wiggle_i860_common(int which, uint16_t data);
	void wiggle_i860p0_pins_w(uint16_t data);
	void wiggle_i860p1_pins_w(uint16_t data);
	uint16_t main_irqiack_r();
	uint16_t sound_resetmain_r();
	void v0_fb_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void v1_fb_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void crtc_w(uint16_t data);
	void vcombat_dac_w(uint16_t data);
	DECLARE_WRITE_LINE_MEMBER(sound_update);

	uint32_t update_screen(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int index);
	uint32_t screen_update_vcombat_main(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_vcombat_aux(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void single_i860_map(address_map &map);
	void dual_i860_map(address_map &map);
	void sound_map(address_map &map);
	void vid_0_map(address_map &map);
	void vid_1_map(address_map &map);

	uint16_t m_c_r(offs_t offset, uint64_t *v);
	void m_c_w(offs_t offset, uint16_t data, uint16_t mem_mask, uint64_t *v);

	uint16_t m0_p0_c1_r(offs_t offset);
	void m0_p0_c1_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t m0_p0_c2_r(offs_t offset);
	void m0_p0_c2_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t m0_p1_c1_r(offs_t offset);
	void m0_p1_c1_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t m0_p1_c2_r(offs_t offset);
	void m0_p1_c2_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t vram_r(offs_t offset, uint64_t *v);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask, uint64_t *v);

	uint16_t vram_0_r(offs_t offset);
	void vram_0_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t vram_1_r(offs_t offset);
	void vram_1_w(offs_t offset, uint16_t data, uint16_t mem_mask);
};

uint32_t vcombat_state::update_screen(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int index)
{
	int y;
	const pen_t *const pens = m_tlc34076->pens();

	uint16_t *m68k_buf = m_m68k_framebuffer[(*m_framebuffer_ctrl & 0x20) ? 1 : 0].get();
	uint16_t *i860_buf = m_i860_framebuffer[index][0].get();

	/* TODO: It looks like the leftmost chunk of the ground should really be on the right side? */
	/*       But the i860 draws the background correctly, so it may be an original game issue. */
	/*       There's also some garbage in the upper-left corner. Might be related to this 'wraparound'. */
	/*       Or maybe it's related to the 68k's alpha?  It might come from the 68k side of the house.  */

	for (y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		int src_addr = 256/2 * y;
		const uint16_t *m68k_src = &m68k_buf[src_addr];
		const uint16_t *i860_src = &i860_buf[src_addr];
		uint32_t *dst = &bitmap.pix(y, cliprect.min_x);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			uint16_t m68k_pix = *m68k_src++;
			uint16_t i860_pix = *i860_src++;

			/* Draw two pixels */
			for (int i = 0; i < 2; ++i)
			{
				/* Vcombat's screen renders 'flopped' - very likely because VR headset displays may reflect off mirrors.
				Shadfgtr isn't flopped, so it's not a constant feature of the hardware. */

				/* Combine the two layers */
				if ((m68k_pix & 0xff) == 0)
					*dst++ = pens[i860_pix & 0xff];
				else
					*dst++ = pens[m68k_pix & 0xff];

				m68k_pix >>= 8;
				i860_pix >>= 8;
			}
		}
	}

	return 0;
}

uint32_t vcombat_state::screen_update_vcombat_main(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 0); }
uint32_t vcombat_state::screen_update_vcombat_aux(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 1); }


void vcombat_state::main_video_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int fb = (*m_framebuffer_ctrl & 0x20) ? 0 : 1;
	uint16_t old_data = m_m68k_framebuffer[fb][offset];

	/* Transparency mode? */
	if (*m_framebuffer_ctrl & 0x40)
	{
		if (data == 0x0000)
			return;

		if ((data & 0x00ff) == 0)
			data = (data & 0xff00) | (old_data & 0x00ff);
		if ((data & 0xff00) == 0)
			data = (data & 0x00ff) | (old_data & 0xff00);

		COMBINE_DATA(&m_m68k_framebuffer[fb][offset]);
	}
	else
	{
		COMBINE_DATA(&m_m68k_framebuffer[fb][offset]);
	}
}

uint16_t vcombat_state::control_1_r()
{
	return (ioport("IN0")->read() << 8);
}

uint16_t vcombat_state::control_2_r()
{
	return (ioport("IN1")->read() << 8);
}

uint16_t vcombat_state::control_3_r()
{
	return (ioport("IN2")->read() << 8);
}

void vcombat_state::wiggle_i860_common(int which, uint16_t data)
{
	int bus_hold = (data & 0x03) == 0x03;
	int reset = data & 0x10;
	if (!m_vid[which].found())
		return;

	i860_cpu_device &device = *m_vid[which];
	if (bus_hold)
	{
		LOG("M0 asserting bus HOLD to i860 #%d\n", which);
		device.i860_set_pin(DEC_PIN_BUS_HOLD, 1);
	}
	else
	{
		LOG("M0 clearing bus HOLD to i860 #%d\n", which);
		device.i860_set_pin(DEC_PIN_BUS_HOLD, 0);
	}

	if (reset)
	{
		LOG("M0 asserting RESET to i860 #%d\n", which);
		device.i860_set_pin(DEC_PIN_RESET, 1);
	}
	else
		device.i860_set_pin(DEC_PIN_RESET, 0);
}

void vcombat_state::wiggle_i860p0_pins_w(uint16_t data)
{
	wiggle_i860_common(0, data);
}

void vcombat_state::wiggle_i860p1_pins_w(uint16_t data)
{
	wiggle_i860_common(1, data);
}

uint16_t vcombat_state::main_irqiack_r()
{
	//LOG("M0: irq iack\n");
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
	//m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	return 0;
}

uint16_t vcombat_state::sound_resetmain_r()
{
	//LOG("M1: reset line to M0\n");
	//m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	return 0;
}

void vcombat_state::v0_fb_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	/* The frame buffer seems to sit on a 32-bit data bus, while the
	   i860 uses a 64-bit data bus.  Adjust accordingly.  */
	char *p = (char *)(m_i860_framebuffer[0][0].get());
	int m = mem_mask;
	int o = (offset << 2);
	if (m & 0xff000000) {
		p[o+3] = (data >> 24) & 0xff;
	}
	if (m & 0x00ff0000) {
		p[o+2] = (data >> 16) & 0xff;
	}
	if (m & 0x0000ff00) {
		p[o+1] = (data >> 8) & 0xff;
	}
	if (m & 0x000000ff) {
		p[o+0] = (data >> 0) & 0xff;
	}
}

/* This is just temporary so we can see what each i860 is doing to the
   framebuffer.  */
void vcombat_state::v1_fb_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	/* The frame buffer seems to sit on a 32-bit data bus, while the
	   i860 uses a 64-bit data bus.  Adjust accordingly.  */
	char *p = (char *)(m_i860_framebuffer[1][0].get());
	int m = mem_mask;
	int o = (offset << 2);
	if (m & 0xff000000) {
		p[o+3] = (data >> 24) & 0xff;
	}
	if (m & 0x00ff0000) {
		p[o+2] = (data >> 16) & 0xff;
	}
	if (m & 0x0000ff00) {
		p[o+1] = (data >> 8) & 0xff;
	}
	if (m & 0x000000ff) {
		p[o+0] = (data >> 0) & 0xff;
	}
}

void vcombat_state::crtc_w(uint16_t data)
{
	if (m_crtc == nullptr)
		return;

	if (m_crtc_select == 0)
		m_crtc->address_w(data >> 8);
	else
		m_crtc->register_w(data >> 8);

	m_crtc_select ^= 1;
}

void vcombat_state::vcombat_dac_w(uint16_t data)
{
	m_dac->write(data >> 5);
	if (data & 0x801f)
		LOG("dac overflow %04x\n", data & 0x801f);
}

uint16_t vcombat_state::m_c_r(offs_t offset, uint64_t *v)
{
	return (*v) >> (offset << 4);
}

void vcombat_state::m_c_w(offs_t offset, uint16_t data, uint16_t mem_mask, uint64_t *v)
{
	uint64_t d = uint64_t(data) << (offset << 4);
	uint64_t m = uint64_t(mem_mask) << (offset << 4);

	*v = ((*v) & ~m) | d;
}

uint16_t vcombat_state::m0_p0_c1_r(offs_t offset)
{
	return m_c_r(offset, m_m0_p0_c1);
}

void vcombat_state::m0_p0_c1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_c_w(offset, data, mem_mask, m_m0_p0_c1);
}

uint16_t vcombat_state::m0_p0_c2_r(offs_t offset)
{
	return m_c_r(offset, m_m0_p0_c2);
}

void vcombat_state::m0_p0_c2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_c_w(offset, data, mem_mask, m_m0_p0_c2);
}

uint16_t vcombat_state::m0_p1_c1_r(offs_t offset)
{
	return m_c_r(offset, m_m0_p1_c1);
}

void vcombat_state::m0_p1_c1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_c_w(offset, data, mem_mask, m_m0_p1_c1);
}

uint16_t vcombat_state::m0_p1_c2_r(offs_t offset)
{
	return m_c_r(offset, m_m0_p1_c2);
}

void vcombat_state::m0_p1_c2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_c_w(offset, data, mem_mask, m_m0_p1_c2);
}


uint16_t vcombat_state::vram_r(offs_t offset, uint64_t *v)
{
	v += offset >> 2;
	return (*v) >> ((offset & 3) << 4);
}

void vcombat_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask, uint64_t *v)
{
	uint64_t d = uint64_t(data) << ((offset & 3) << 4);
	uint64_t m = uint64_t(mem_mask) << ((offset & 3) << 4);

	v += offset >> 2;
	*v = ((*v) & ~m) | d;
}

uint16_t vcombat_state::vram_0_r(offs_t offset)
{
	return vram_r(offset, m_vid_0_ram);
}

void vcombat_state::vram_0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	vram_w(offset, data, mem_mask, m_vid_0_ram);
}

uint16_t vcombat_state::vram_1_r(offs_t offset)
{
	return vram_r(offset, m_vid_1_ram);
}

void vcombat_state::vram_1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	vram_w(offset, data, mem_mask, m_vid_1_ram);
}

void vcombat_state::single_i860_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x20ffff).ram();
	map(0x300000, 0x30ffff).w(FUNC(vcombat_state::main_video_write));

	map(0x400000, 0x43ffff).rw(FUNC(vcombat_state::vram_0_r), FUNC(vcombat_state::vram_0_w));   /* First i860 shared RAM */
	map(0x440000, 0x440003).rw(FUNC(vcombat_state::m0_p0_c1_r), FUNC(vcombat_state::m0_p0_c1_w));      /* M0->P0 i860 #1 com 1 */
	map(0x480000, 0x480003).rw(FUNC(vcombat_state::m0_p0_c2_r), FUNC(vcombat_state::m0_p0_c2_w));      /* M0<-P0 i860 #1 com 2 */
	map(0x4c0000, 0x4c0003).w(FUNC(vcombat_state::wiggle_i860p0_pins_w)); /* i860 #1 stop/start/reset */

	map(0x600000, 0x600001).r(FUNC(vcombat_state::control_1_r));   /* IN0 port */
	map(0x600004, 0x600005).ram().share("share5");      /* M0<-M1 */
	map(0x600008, 0x600009).r(FUNC(vcombat_state::control_2_r));   /* IN1 port */
	map(0x60001c, 0x60001d).noprw();

	map(0x60000c, 0x60000d).w(FUNC(vcombat_state::crtc_w));
	map(0x600010, 0x600011).ram().share("fb_control");
	map(0x700000, 0x7007ff).ram().share("nvram");
	map(0x701000, 0x701001).r(FUNC(vcombat_state::main_irqiack_r));
	map(0x702000, 0x702001).r(FUNC(vcombat_state::control_3_r));
	map(0x705000, 0x705001).ram().share("share4");      /* M1->M0 */

	//map(0x703000, 0x703001)      /* Headset rotation axis? */
	//map(0x704000, 0x704001)      /* Headset rotation axis? */

	map(0x706000, 0x70601f).rw(m_tlc34076, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask16(0x00ff);
}


void vcombat_state::dual_i860_map(address_map &map)
{
	single_i860_map(map);

	map(0x500000, 0x53ffff).rw(FUNC(vcombat_state::vram_1_r), FUNC(vcombat_state::vram_1_w));   /* Second i860 shared RAM */
	map(0x540000, 0x540003).rw(FUNC(vcombat_state::m0_p1_c1_r), FUNC(vcombat_state::m0_p1_c1_w));      /* M0->P1 i860 #2 com 1 */
	map(0x580000, 0x580003).rw(FUNC(vcombat_state::m0_p1_c2_r), FUNC(vcombat_state::m0_p1_c2_w));      /* M0<-P1 i860 #2 com 2 */
	map(0x5c0000, 0x5c0003).w(FUNC(vcombat_state::wiggle_i860p1_pins_w)); /* i860 #2 stop/start/reset */
}


/* The first i860 - middle board */
void vcombat_state::vid_0_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).ram().w(FUNC(vcombat_state::v0_fb_w));      /* Shared framebuffer - half of the bits lost to 32-bit bus */
	map(0x20000000, 0x20000007).ram().share("m0_p0_c1");      /* M0<-P0 com 1 (0x440000 in 68k-land) */
	map(0x40000000, 0x401fffff).rom().region("gfx", 0);
	map(0x80000000, 0x80000007).ram().share("m0_p0_c2");      /* M0->P0 com 2 (0x480000 in 68k-land) */
	map(0xc0000000, 0xc0000fff).noprw();                     /* Dummy D$ flush page. */
	map(0xfffc0000, 0xffffffff).ram().share("vid_0_ram");           /* Shared RAM with main */
}


/* The second i860 - top board */
void vcombat_state::vid_1_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).ram().w(FUNC(vcombat_state::v1_fb_w));      /* Half of the bits lost to 32-bit bus */
	map(0x20000000, 0x20000007).ram().share("m0_p1_c1");      /* M0->P1 com 1 (0x540000 in 68k-land) */
	map(0x40000000, 0x401fffff).rom().region("gfx", 0);
	map(0x80000000, 0x80000007).ram().share("m0_p1_c2");      /* M0<-P1 com 2      (0x580000 in 68k-land) */
	map(0xc0000000, 0xc0000fff).noprw();                     /* Dummy D$ flush page. */
	map(0xfffc0000, 0xffffffff).ram().share("vid_1_ram");           /* Shared RAM with main */
}


/* Sound CPU */
void vcombat_state::sound_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x08ffff).ram();
	map(0x0c0000, 0x0c0001).w(FUNC(vcombat_state::vcombat_dac_w));
	map(0x140000, 0x140001).r(FUNC(vcombat_state::sound_resetmain_r));   /* Ping M0's reset line */
	map(0x180000, 0x180001).ram().share("share4");   /* M1<-M0 */
	map(0x1c0000, 0x1c0001).ram().share("share5");   /* M1->M0 */
	map(0x200000, 0x37ffff).rom().region("samples", 0);
}


void vcombat_state::machine_reset()
{
	for (auto &i860 : m_vid)
		if (i860.found())
			i860->i860_set_pin(DEC_PIN_BUS_HOLD, 1);

	m_crtc_select = 0;
}


void vcombat_state::init_vcombat()
{
	uint8_t *ROM = memregion("maincpu")->base();

	/* Allocate the 68000 framebuffers */
	m_m68k_framebuffer[0] = std::make_unique<uint16_t[]>(0x8000);
	m_m68k_framebuffer[1] = std::make_unique<uint16_t[]>(0x8000);

	/* First i860 */
	m_i860_framebuffer[0][0] = std::make_unique<uint16_t[]>(0x8000);
	m_i860_framebuffer[0][1] = std::make_unique<uint16_t[]>(0x8000);

	/* Second i860 */
	m_i860_framebuffer[1][0] = std::make_unique<uint16_t[]>(0x8000);
	m_i860_framebuffer[1][1] = std::make_unique<uint16_t[]>(0x8000);

	/* pc==4016 : jump 4038 ... There's something strange about how it waits at 402e (interrupts all masked out)
	   I think what is happening here is that M0 snags the first time
	   it hits this point based on a counter test just above this
	   instruction.  That counter gets updated just past this instruction.
	   However, the only way this can be passed is if the M0 CPU is
	   reset (the IPL=7, but irq 7 is a nop).  I am almost sure that M1
	   reads a latch, which resets M0 (probably to ensure M0 and M1 are
	   both alive) and gets passed this snag.  I tried to hook up a reset
	   which should work, but asserting the reset line on the m68k doesn't
	   seem to do anything.  Maybe the 68k emulator doesn't respond to
	   that, or I didn't do it correctly.  But I think that is what needs
	   to be done.  But it isn't crucial for emulation.  Shadow does not
	   have this snag. */
	ROM[0x4017] = 0x66;
}

void vcombat_state::init_shadfgtr()
{
	/* Allocate th 68000 frame buffers */
	m_m68k_framebuffer[0] = std::make_unique<uint16_t[]>(0x8000);
	m_m68k_framebuffer[1] = std::make_unique<uint16_t[]>(0x8000);

	/* Only one i860 */
	m_i860_framebuffer[0][0] = std::make_unique<uint16_t[]>(0x8000);
	m_i860_framebuffer[0][1] = std::make_unique<uint16_t[]>(0x8000);
	m_i860_framebuffer[1][0] = nullptr;
	m_i860_framebuffer[1][1] = nullptr;
}


static INPUT_PORTS_START( vcombat )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )   /* Left button */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )   /* Right button */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( shadfgtr )
	/* Check me */
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* Check me */
	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )
INPUT_PORTS_END


WRITE_LINE_MEMBER(vcombat_state::sound_update)
{
	/* Seems reasonable */
	m_soundcpu->set_input_line(M68K_IRQ_1, state ? ASSERT_LINE : CLEAR_LINE);
}

void vcombat_state::vcombat(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vcombat_state::dual_i860_map);
	m_maincpu->set_vblank_int("screen", FUNC(vcombat_state::irq1_line_assert));

	/* The middle board i860 */
	I860(config, m_vid[0], XTAL(20'000'000));
	m_vid[0]->set_addrmap(AS_PROGRAM, &vcombat_state::vid_0_map);

	/* The top board i860 */
	I860(config, m_vid[1], XTAL(20'000'000));
	m_vid[1]->set_addrmap(AS_PROGRAM, &vcombat_state::vid_1_map);

	/* Sound CPU */
	M68000(config, m_soundcpu, XTAL(12'000'000));
	m_soundcpu->set_addrmap(AS_PROGRAM, &vcombat_state::sound_map);
	m_soundcpu->set_periodic_int(FUNC(vcombat_state::irq1_line_hold), attotime::from_hz(15000)); /* Remove this if MC6845 is enabled */

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

/* Temporary hack for experimenting with timing. */
#if 0
	//config.set_maximum_quantum(attotime::from_hz(1200));
	config.m_perfect_cpu_quantum = subtag("maincpu");
#endif

	TLC34076(config, m_tlc34076, tlc34076_device::TLC34076_6_BIT);

	/* Disabled for now as it can't handle multiple screens */
//  MC6845(config, m_crtc, 6000000 / 16);
//  m_crtc->set_screen("screen");
	config.set_default_layout(layout_dualhsxs);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(12'000'000) / 2, 400, 0, 256, 291, 0, 208);
	screen.set_screen_update(FUNC(vcombat_state::screen_update_vcombat_main));

	screen_device &aux(SCREEN(config, "aux", SCREEN_TYPE_RASTER));
	aux.set_raw(XTAL(12'000'000) / 2, 400, 0, 256, 291, 0, 208);
	aux.set_screen_update(FUNC(vcombat_state::screen_update_vcombat_aux));

	SPEAKER(config, "speaker").front_center();
	DAC_10BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 1.0); // unknown DAC
}


void vcombat_state::shadfgtr(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vcombat_state::single_i860_map);
	m_maincpu->set_vblank_int("screen", FUNC(vcombat_state::irq1_line_assert));

	/* The middle board i860 */
	I860(config, m_vid[0], XTAL(20'000'000));
	m_vid[0]->set_addrmap(AS_PROGRAM, &vcombat_state::vid_0_map);

	/* Sound CPU */
	M68000(config, m_soundcpu, XTAL(12'000'000));
	m_soundcpu->set_addrmap(AS_PROGRAM, &vcombat_state::sound_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TLC34076(config, m_tlc34076, tlc34076_device::TLC34076_6_BIT);

	MC6845(config, m_crtc, XTAL(20'000'000) / 4 / 16);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(16);
	m_crtc->out_hsync_callback().set(FUNC(vcombat_state::sound_update));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(20'000'000) / 4, 320, 0, 256, 277, 0, 224);
	screen.set_screen_update(FUNC(vcombat_state::screen_update_vcombat_main));

	SPEAKER(config, "speaker").front_center();
	DAC_10BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 1.0); // unknown DAC
}


ROM_START( vcombat )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ep8v2.b49", 0x00000, 0x80000, CRC(98d5a45d) SHA1(099e314f11c93ad6e642ceaa311e2a5b6fd7193c) )
	ROM_LOAD16_BYTE( "ep7v2.b51", 0x00001, 0x80000, CRC(06185bcb) SHA1(855b11ae7644d6c7c1c935b2f5aec484071ca870) )

	ROM_REGION( 0x40000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "ep1v2.b42", 0x00000, 0x20000, CRC(560b2e6c) SHA1(e35c0466a1e14beab080e3155f873e9c2a1c028b) )
	ROM_LOAD16_BYTE( "ep6v2.b33", 0x00001, 0x20000, CRC(37928a5d) SHA1(7850be26dbd356cdeef2a0d87738de16420f6291) )

	ROM_REGION16_BE( 0x180000, "samples", 0 )
	ROM_LOAD16_BYTE( "ep2v2.b41", 0x000000, 0x80000, CRC(7dad3458) SHA1(deae5ebef0346250d3f9744933423253a336bb67) )
	ROM_LOAD16_BYTE( "ep4v2.b37", 0x000001, 0x80000, CRC(b0be2e91) SHA1(66f3a9f5abeb4b95ac806e4bb165f938dca38b2d) )
	ROM_LOAD16_BYTE( "ep3v2.b40", 0x100000, 0x40000, CRC(8c491526) SHA1(95c6bcbe0adcfffb12fd2b86c9f4ca26aa188bbf) )
	ROM_LOAD16_BYTE( "ep5v2.b36", 0x100001, 0x40000, CRC(7592b2eb) SHA1(92a540726306d7adbf207fe86a4c4fa66958f90b) )

	ROM_REGION( 0x800, "user1", 0 ) /* The SRAM module */
	ROM_LOAD( "ds1220y.b53", 0x000, 0x800, CRC(b21cfe5f) SHA1(898ace3cd0913ea4b0dc84320219777773ef856f) )

	/* These roms are identical on both of the upper boards */
	ROM_REGION64_LE( 0x200000, "gfx", 0 )
	ROM_LOAD64_WORD( "9.u55",  0x000000, 0x80000, CRC(a276e18b) SHA1(6d60e519196a4858b82241504592413df498e12f) )
	ROM_LOAD64_WORD( "10.u57", 0x000002, 0x80000, CRC(8921f20e) SHA1(6e9ca2eaad3e1108ba0e1d7792fd5d0305bec201) )
	ROM_LOAD64_WORD( "11.u54", 0x000004, 0x80000, CRC(a83094ce) SHA1(c3512375fecdb5e7eb02a4aa140ae4efe0233cb8) )
	ROM_LOAD64_WORD( "12.u56", 0x000006, 0x80000, CRC(0cdffd4f) SHA1(65ace78711b3ef6e0ff9a7ad7343b5558e652f6c) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "pal1_w2.u51", 0x000, 0x1f1, CRC(af497420) SHA1(03aa82189d91ae194dd5a6e7b9dbdb7cd473ddb6) )
	ROM_LOAD( "pal2_w2.u52", 0x200, 0x1f1, CRC(4a6df05d) SHA1(236b951e5daf927c050d0f35558c171a020156ab) )
ROM_END

ROM_START( shadfgtr )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "shadfgtr.b49", 0x00000, 0x80000, CRC(2d9d31a1) SHA1(45854915bcb9db2e4076a7f26a0a349077cd10bc) )
	ROM_LOAD16_BYTE( "shadfgtr.b51", 0x00001, 0x80000, CRC(03d0f075) SHA1(06013a4363305a23d7e8ba8fe2fa961cd540391d) )

	ROM_REGION( 0x40000, "soundcpu", 0 )
	ROM_LOAD16_BYTE( "shadfgtr.b42", 0x00000, 0x20000, CRC(f8605dcd) SHA1(1b29f47856ccc757bc96674682ae48f87e6b0e54) )
	ROM_LOAD16_BYTE( "shadfgtr.b33", 0x00001, 0x20000, CRC(291d59ac) SHA1(cc4904c2ac8ef6a12033c10893246a438ac44014) )

	ROM_REGION16_BE( 0x180000, "samples", 0 )
	ROM_LOAD16_BYTE( "shadfgtr.b41", 0x00000, 0x80000, CRC(9e4b4df3) SHA1(8101197275e9f728acdeef85737eecbdec132b27) )
	ROM_LOAD16_BYTE( "shadfgtr.b37", 0x00001, 0x80000, CRC(98446ba2) SHA1(1c8cc0d9c5de54d9e53699a5ab281579d15edc96) )

	ROM_REGION( 0x800, "user1", 0 ) /* The SRAM module */
	ROM_LOAD( "shadfgtr.b53", 0x000, 0x800, CRC(e766a3ab) SHA1(e7696ec08d5c86f64d768480f43edbd19ded162d) )

	ROM_REGION64_LE( 0x200000, "gfx", 0 )
	ROM_LOAD64_WORD( "shadfgtr.u55", 0x000000, 0x80000, CRC(e807631d) SHA1(9027ff7dc60b808434dac292c08f0630d3d52186) )
	ROM_LOAD64_WORD( "shadfgtr.u57", 0x000002, 0x80000, CRC(60d701d7) SHA1(936473b5e3b2e9e9e3b50cf977fc5a670a097850) )
	ROM_LOAD64_WORD( "shadfgtr.u54", 0x000004, 0x80000, CRC(c45d68d6) SHA1(a133e4f13d3af18bccf0d060a659d64ac699b159) )
	ROM_LOAD64_WORD( "shadfgtr.u56", 0x000006, 0x80000, CRC(fb76db5a) SHA1(fa546f465df113c13037abed1162bfa6f9b1dc9b) )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "shadfgtr.u51", 0x000, 0x1f1, CRC(bab58337) SHA1(c4a79c8e53aeadb7f64d49d214b607b5b36f144e) )
	/* The second upper-board PAL couldn't be read */
ROM_END

//    YEAR  NAME      PARENT  MACHINE   INPUT     STATE          INIT      MONITOR              COMPANY         FULLNAME           FLAGS
GAME( 1993, vcombat,  0,      vcombat,  vcombat,  vcombat_state, init_vcombat,  ORIENTATION_FLIP_X,  "VR8 Inc.",     "Virtual Combat",  MACHINE_NOT_WORKING )
GAME( 1993, shadfgtr, 0,      shadfgtr, shadfgtr, vcombat_state, init_shadfgtr, ROT0,                "Dutech Inc.",  "Shadow Fighters", MACHINE_NOT_WORKING )
