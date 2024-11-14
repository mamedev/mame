// license:BSD-3-Clause
// copyright-holders:Robbbert,Vas Crabb
/***************************************************************************

Univac Terminals

2009-05-25 Skeleton driver

The terminals are models UTS10, UTS20, UTS30, UTS40, UTS50 and SVT1120.

There were other terminals (Uniscope 100/200/300/400) and UTS60, but
they had different hardware. Uniscope models are believed to use the i8080,
and the UTS60 was a colour graphics terminal with a MC68000 and 2 floppy drives.

The terminal has 2 screens selectable by the operator with the Fn + 1-2
buttons. Thus the user can have two sessions open at once, to different
mainframes or applications. The keyboard connected to the terminal with
a coiled cord and a 9-pin D-connector.

Sound is a beeper.

This driver is all guesswork; Unisys never released technical info
to customers. All parts on the PCBs have internal Unisys part numbers
instead of the manufacturer's numbers.

Notes:
* Port $C6 probably controls serial loopback
  - at a guess, bit 0 enables loopback on both channels
* The NVRAM is 4 bits wide on the LSBs, but (0x81) & 0x10 does something
  - NVRAM nybbles are read/written on the LSBs of 64 ports 0x80 to 0xb4
  - Nybbles are packed/unpacked into 32 bytes starting at 0xd7d7
  - On boot it reads (0x81) & 0x10, and if set preserves 0xd831 to 0xd863
  - This has to be some kind of warm boot detection, but how does it work?

You can use a debug trick to get UTS10 to boot:
- When it loops at @0B33, pc = B35 and g

How to create a FCC (field control code):
- Move the cursor to where you want the FCC to be
- Press FCC GEN
- Now you enter a sequence of 4 bytes
- 1. Video Attribute
- - Spacebar or N: Normal
- - L: Low intensity
- - O: Off
- - B: Blink (low-half)
- - 1: Rev-video/Normal
- - 2: Rev-video/half-intensity
- - 3: Rev-video/blink: normal-half
- - 4: Rev-video/blink: low
- 2. Tab-stop
- - Spacebar or S: No tab-stop
- - T: Tab-stop
- - 6: Tab-stop protected
- - 7: No tab protected
- 3. Data-entry control
- - Spacebar or U: Unprotected
- - P: Protected
- - A: Alpha only
- - N: Numeric only
- 4. Justified
- - Spacebar: Normal
- - R: Right-justified
- Press Spacebar to enable the new FCC and exit back to normal.

Control-page parameters. These vary depending on the terminal and feature set. Press FCTN and CTRL PAGE keys together.
You get a protected area covering the first 2 lines where you can configure the terminal. Settings are saved in the NVRAM.
Depending on the setting, it may take effect immediately (after exiting the control page), or after a reboot.
Entries may be in upper or lower case.
UC/NO : Upper and lower case can be entered
UC/YS : Lower case is automatically folded to upper case.
AB/LI : Alternate brightness is Low Intensity
AB/RV : Alternate brightness is Reverse Video
AB/NI : Alternate brightness is Normal Intensity
IL/RV : Indicator Line is Reverse Video
IL/NI : Indicator Line is Normal Intensity
KK/ON : Keyclick on
KK/OF : Keyclick off
SP/NS : Non-destructive spacebar (works like right-arrow)
SP/DS : Destructive spacebar
VO/01 : Video off after 1 minute (a blank screen saver)
VO/04 : Video off after 4 minutes
VO/16 : Video off after 16 minutes
VO/64 : Video off after 64 minutes
CC/ON : Control characters show
CC/OF : Control characters off (look like a space)
CS/LO : Cursor repeat slow
CS/HI : Cursor repeat fast
RI/xx : Set the RID (generally 21-2F)
SI/xx : Set the SID (generally 51-7F)
After entering the characters, press FCTN and CTRL PAGE keys again to save the setting.


****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "bus/uts_kbd/uts_kbd.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/z80daisy.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"
#include "video/dp8350.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_PARITY  (1U << 1)
#define LOG_NVRAM   (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_PARITY | LOG_NVRAM)
#include "logmacro.h"

#define LOGPARITY(...)  LOGMASKED(LOG_PARITY, __VA_ARGS__)
#define LOGNVRAM(...)   LOGMASKED(LOG_NVRAM, __VA_ARGS__)


namespace {

class univac_state : public driver_device
{
public:
	univac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_ctc(*this, "ctc")
		, m_keybclk(*this, "keybclk")
		, m_sio(*this, "sio")
		, m_alarm(*this, "alarm")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_keyboard(*this, "keyboard")
		, m_printer(*this, "printer")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
		, m_p_nvram(*this, "nvram")
		, m_parity_poison(false)
		, m_display_enable(false)
		, m_nvram_protect(false)
		, m_alarm_enable(false)
		, m_alarm_toggle(false)
		, m_loopback_control(false)
		, m_comm_rxd(true)
		, m_sio_txda(true)
		, m_aux_rxd(true)
		, m_sio_txdb(true)
		, m_sio_rtsb(true)
		, m_aux_dsr(true)
		, m_sio_wrdyb(true)
	{ }

	void uts10(machine_config &config);
	void uts20(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 ram_r(offs_t offset);
	u8 bank_r(offs_t offset);
	void ram_w(offs_t offset, u8 data);
	void bank_w(offs_t offset, u8 data);
	void nvram_w(offs_t offset, u8 data);

	void nvram_protect_w(int state);
	void select_disp_w(int state);
	void ram_control_w(int state);
	void parity_poison_w(int state);
	void display_enable_w(int state);
	void alarm_enable_w(int state);
	void sio_loopback_w(int state);
	void sio_txda_w(int state);
	void sio_txdb_w(int state);
	void aux_rxd_w(int state);
	void sio_rtsb_w(int state);
	void sio_wrdyb_w(int state);
	void aux_dsr_w(int state);
	void loopback_rxcb_w(int state);
	void porte6_w(int state);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void uts10_io_map(address_map &map) ATTR_COLD;
	void uts10_map(address_map &map) ATTR_COLD;

	required_device<z80_device>     m_maincpu;
	required_device<nvram_device>   m_nvram;
	required_device<z80ctc_device>  m_ctc;
	optional_device<clock_device>   m_keybclk;
	required_device<z80sio_device>  m_sio;
	required_device<speaker_sound_device> m_alarm;
	required_device<screen_device>  m_screen;
	required_device<palette_device> m_palette;

	required_device<uts_keyboard_port_device> m_keyboard;
	required_device<rs232_port_device> m_printer;

	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_videoram;
	required_shared_ptr<u8> m_p_nvram;
	std::unique_ptr<u8 []>  m_p_parity;

	u16 m_disp_mask = 0U;
	u16 m_bank_mask = 0U;
	bool m_parity_poison = false;
	bool m_display_enable = false;
	u8 m_framecnt = 0U;
	bool m_nvram_protect = false;

	bool m_alarm_enable = false;
	bool m_alarm_toggle = false;

	bool m_loopback_control = false;
	bool m_comm_rxd = false;
	bool m_sio_txda = false;
	bool m_aux_rxd = false;
	bool m_sio_txdb = false;
	bool m_sio_rtsb = false;
	bool m_aux_dsr = false;
	bool m_sio_wrdyb = false;
};



u8 univac_state::ram_r(offs_t offset)
{
	if (BIT(m_p_parity[offset >> 3], offset & 0x07) && !machine().side_effects_disabled())
	{
		LOGPARITY("parity check failed offset = %04X\n", offset);
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
	return m_p_videoram[offset];
}

u8 univac_state::bank_r(offs_t offset)
{
	return ram_r(offset ^ m_bank_mask);
}

void univac_state::ram_w(offs_t offset, u8 data)
{
	if (m_parity_poison)
	{
		LOGPARITY("poison parity offset = %04X\n", offset);
		m_p_parity[offset >> 3] |= u8(1) << (offset & 0x07);
	}
	else
	{
		m_p_parity[offset >> 3] &= ~(u8(1) << (offset & 0x07));
	}
	m_p_videoram[offset] = data;
}

void univac_state::bank_w(offs_t offset, u8 data)
{
	ram_w(offset ^ m_bank_mask, data);
}

void univac_state::nvram_w(offs_t offset, u8 data)
{
	// NVRAM is four bits wide, accessed in the low nybble
	// It's simplest to hack it when writing to make the upper bits read back high on the open bus
	// (But is it all open bus? Bit 4 is specifically tested in a few places...)
	if (m_nvram_protect)
		LOGNVRAM("%s: NVRAM write suppressed (address %02X, data %02X)\n", machine().describe_context(), offset + 0x80, data);
	else
		m_p_nvram[offset] = data | 0xf0;
}

void univac_state::nvram_protect_w(int state)
{
	// There seems to be some timing-based write protection related to the CTC's TRG0 input.
	// The present implementation is a crude approximation of a wild guess.
	if (state)
	{
		m_nvram_protect = m_screen->vpos() < 10;

		if (m_alarm_enable)
		{
			m_alarm_toggle = !m_alarm_toggle;
			m_alarm->level_w(m_alarm_toggle);
		}
	}
}

void univac_state::select_disp_w(int state)
{
	m_disp_mask = state ? 0x2000 : 0x0000;
}

void univac_state::ram_control_w(int state)
{
	m_bank_mask = state ? 0x2000 : 0x0000;
}

void univac_state::parity_poison_w(int state)
{
	m_parity_poison = state;
}

void univac_state::display_enable_w(int state)
{
	m_display_enable = state;
}

void univac_state::alarm_enable_w(int state)
{
	m_alarm_enable = state;
	if (!state)
	{
		m_alarm_toggle = false;
		m_alarm->level_w(0);
	}
}

void univac_state::sio_loopback_w(int state)
{
	if (state)
	{
		m_sio->rxa_w(m_sio_txda);
		m_sio->rxb_w(m_sio_txdb);
		m_sio->dcdb_w(m_sio_wrdyb);
		m_sio->ctsb_w(m_sio_wrdyb);
		m_sio->syncb_w(!m_sio_rtsb);
		m_printer->write_txd(1);
		m_printer->write_rts(1);
		m_keyboard->ready_w(0);
		if (m_keybclk.found())
			m_keybclk->set_clock_scale(0.0);
	}
	else
	{
		m_sio->rxa_w(m_comm_rxd);
		m_sio->rxb_w(m_aux_rxd);
		m_sio->dcdb_w(m_aux_dsr);
		m_sio->ctsb_w(m_aux_dsr); // likely ignored
		m_sio->syncb_w(1);
		m_printer->write_txd(m_sio_txdb);
		m_printer->write_rts(m_sio_rtsb);
		m_keyboard->ready_w(m_sio_wrdyb);
		if (m_keybclk.found())
			m_keybclk->set_clock_scale(1.0);
	}

	m_loopback_control = state;
}

void univac_state::sio_txda_w(int state)
{
	m_sio_txda = state;
	if (m_loopback_control)
		m_sio->rxa_w(state);
}

void univac_state::sio_txdb_w(int state)
{
	m_sio_txdb = state;
	if (m_loopback_control)
		m_sio->rxb_w(state);
	else
		m_printer->write_txd(state);
}

void univac_state::aux_rxd_w(int state)
{
	m_aux_rxd = state;
	if (!m_loopback_control)
		m_sio->rxb_w(state);
}

void univac_state::sio_rtsb_w(int state)
{
	m_sio_rtsb = state;
	if (m_loopback_control)
		m_sio->syncb_w(!state);
	else
		m_printer->write_rts(state);
}

void univac_state::sio_wrdyb_w(int state)
{
	m_sio_wrdyb = state;
	if (m_loopback_control)
	{
		m_sio->dcdb_w(state);
		m_sio->ctsb_w(state);
	}
	else
		m_keyboard->ready_w(state);
}

void univac_state::aux_dsr_w(int state)
{
	m_aux_dsr = state;
	if (!m_loopback_control)
	{
		m_sio->dcdb_w(state);
		m_sio->ctsb_w(state);
	}
}

void univac_state::loopback_rxcb_w(int state)
{
	if (m_loopback_control)
		m_sio->rxcb_w(state);
}

void univac_state::porte6_w(int state)
{
	//m_beep->set_state(state); // not sure what belongs here, but it isn't the beeper
}


void univac_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x4fff).rom().region("roms", 0);
	map(0x8000, 0xbfff).rw(FUNC(univac_state::bank_r), FUNC(univac_state::bank_w));
	map(0xc000, 0xffff).rw(FUNC(univac_state::ram_r), FUNC(univac_state::ram_w)).share("videoram");
}

void univac_state::uts10_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x4fff).rom().region("roms", 0);
	map(0x8000, 0x9fff).mirror(0x2000).rw(FUNC(univac_state::bank_r), FUNC(univac_state::bank_w));
	map(0xc000, 0xffff).rw(FUNC(univac_state::ram_r), FUNC(univac_state::ram_w)).share("videoram");
}

void univac_state::uts10_io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw(m_sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x20, 0x23).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x60, 0x60).nopw(); // values written here may or may not matter
	map(0x80, 0xbf).ram().w(FUNC(univac_state::nvram_w)).share("nvram");
	map(0xc0, 0xc7).w("latch_c0", FUNC(ls259_device::write_d0));
	map(0xe0, 0xe7).w("latch_e0", FUNC(ls259_device::write_d0));
}

void univac_state::io_map(address_map &map)
{
	uts10_io_map(map);
	map(0x40, 0x40).nopr(); // read only once, during self-test; result is discarded
	map(0x40, 0x47).w("latch_40", FUNC(ls259_device::write_d0));
}

/* Input ports */
static INPUT_PORTS_START( uts20 )
INPUT_PORTS_END


void univac_state::machine_start()
{
	// D7DC and D7DD are checked for valid RID and SID (usually 21 and 51) if not valid then NVRAM gets initialised.

	std::size_t const parity_bytes = (m_p_videoram.bytes() + 7) / 8;
	m_p_parity.reset(new u8[parity_bytes]);
	std::fill_n(m_p_parity.get(), parity_bytes, 0);

	save_pointer(NAME(m_p_parity), parity_bytes);
	save_item(NAME(m_bank_mask));
	save_item(NAME(m_parity_poison));
	save_item(NAME(m_display_enable));
	save_item(NAME(m_framecnt));
	save_item(NAME(m_nvram_protect));
	save_item(NAME(m_alarm_enable));
	save_item(NAME(m_alarm_toggle));
	save_item(NAME(m_loopback_control));
	save_item(NAME(m_comm_rxd));
	save_item(NAME(m_sio_txda));
	save_item(NAME(m_aux_rxd));
	save_item(NAME(m_sio_txdb));
	save_item(NAME(m_sio_rtsb));
	save_item(NAME(m_aux_dsr));
	save_item(NAME(m_sio_wrdyb));
	save_item(NAME(m_disp_mask));

	m_disp_mask = 0;
}

uint32_t univac_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_display_enable)
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	const pen_t *pen = m_palette->pens();

	uint16_t sy=0,ma=0;

	m_framecnt++;

	for (u8 y = 0; y < 25; y++)
	{
		for (u8 ra = 0; ra < 14; ra++)
		{
			uint32_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 80; x++)
			{
				u8 chr = ram_r(x ^ m_disp_mask);    // bit 7 = rv attribute (or dim, depending on control-page setting)

				uint16_t gfx = m_p_chargen[((chr & 0x7f)<<4) | ra];

				// chars 1C, 1D, 1F need special handling
				if ((chr >= 0x1c) && (chr <= 0x1f) && BIT(gfx, 7))
				{
					gfx &= 0x7f;
					if (m_framecnt & 16) // They also blink
						gfx = 0;
				}

				// reverse-video attribute
				if (BIT(chr, 7))
					gfx = ~gfx;

				/* Display a scanline of a character */
				for (int bit = 8; bit >= 0; bit--)
				{
					*p++ = pen[BIT(gfx, bit)];
				}
			}
		}
		ma += 80;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 14,                   /* 8 x 14 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_uts )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

static const z80_daisy_config daisy_chain[] =
{
	{ "sio" },
	{ "ctc" },
	{ nullptr }
};

// All frequencies confirmed
void univac_state::uts20(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 6); // 3.072 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &univac_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &univac_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	ls259_device &latch_40(LS259(config, "latch_40")); // actual type and location unknown
	latch_40.q_out_cb<1>().set(FUNC(univac_state::select_disp_w));
	latch_40.q_out_cb<3>().set(FUNC(univac_state::ram_control_w));

	ls259_device &latch_c0(LS259(config, "latch_c0")); // actual type and location unknown
	latch_c0.q_out_cb<0>().set(FUNC(univac_state::alarm_enable_w));
	latch_c0.q_out_cb<3>().set(FUNC(univac_state::display_enable_w));
	latch_c0.q_out_cb<4>().set(FUNC(univac_state::parity_poison_w));
	latch_c0.q_out_cb<6>().set(FUNC(univac_state::sio_loopback_w));

	ls259_device &latch_e0(LS259(config, "latch_e0")); // actual type and location unknown
	//latch_e0.q_out_cb<2>().set(FUNC(univac_state::reverse_video_w));
	latch_e0.q_out_cb<5>().set("crtc", FUNC(dp835x_device::refresh_control)).invert();
	latch_e0.q_out_cb<6>().set(FUNC(univac_state::porte6_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
	m_screen->set_screen_update(FUNC(univac_state::screen_update));
	PALETTE(config, m_palette, palette_device::MONOCHROME);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_uts);

	dp835x_device &crtc(DP835X_A(config, "crtc", 19'980'000));
	crtc.set_screen("screen");
	crtc.vblank_callback().set(m_ctc, FUNC(z80ctc_device::trg0));
	crtc.vblank_callback().append(m_ctc, FUNC(z80ctc_device::trg3));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	Z80CTC(config, m_ctc, 18.432_MHz_XTAL / 6);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<1>(18.432_MHz_XTAL / 12);
	m_ctc->set_clk<2>(18.432_MHz_XTAL / 12);
	m_ctc->zc_callback<0>().set(FUNC(univac_state::nvram_protect_w));
	m_ctc->zc_callback<1>().set(m_sio, FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<1>().append(m_sio, FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<2>().set(m_sio, FUNC(z80sio_device::txcb_w));
	m_ctc->zc_callback<2>().append(FUNC(univac_state::loopback_rxcb_w));

	CLOCK(config, m_keybclk, 18.432_MHz_XTAL / 60);
	m_keybclk->signal_handler().set(m_sio, FUNC(z80sio_device::rxcb_w));

	Z80SIO(config, m_sio, 18.432_MHz_XTAL / 6);
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_sio->out_txda_callback().set(FUNC(univac_state::sio_txda_w));
	m_sio->out_txdb_callback().set(FUNC(univac_state::sio_txdb_w));
	m_sio->out_rtsb_callback().set(FUNC(univac_state::sio_rtsb_w));
	m_sio->out_wrdyb_callback().set(FUNC(univac_state::sio_wrdyb_w));

	/* Sound */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_alarm).add_route(ALL_OUTPUTS, "mono", 0.05);

	UTS_KEYBOARD(config, m_keyboard, uts20_keyboards, "extw");
	m_keyboard->rxd_callback().set(FUNC(univac_state::aux_rxd_w));

	RS232_PORT(config, m_printer, default_rs232_devices, nullptr);
	m_printer->dcd_handler().set(FUNC(univac_state::aux_dsr_w));
}

void univac_state::uts10(machine_config &config)
{
	uts20(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &univac_state::uts10_map);
	m_maincpu->set_addrmap(AS_IO, &univac_state::uts10_io_map);

	config.device_remove("keybclk");
	m_ctc->zc_callback<2>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));

	config.device_remove("latch_40");
	subdevice<ls259_device>("latch_c0")->q_out_cb<6>().set_nop();
	subdevice<ls259_device>("latch_e0")->q_out_cb<7>().set(FUNC(univac_state::sio_loopback_w)).invert();

	UTS_KEYBOARD(config.replace(), m_keyboard, uts10_keyboards, "extw");
	m_keyboard->rxd_callback().set(FUNC(univac_state::aux_rxd_w));
}


/* ROM definition */
ROM_START( uts10 )
	ROM_REGION( 0x5000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "f3577_1.bin",  0x0000, 0x0800, CRC(f7d47484) SHA1(84c01d054df19e8da44c242a67d97f643bdabc4c) )
	ROM_LOAD( "f3577_2.bin",  0x0800, 0x0800, CRC(7c1045f0) SHA1(732e8c111a346476c59bcfda73f0f826cdcd7eb3) )
	ROM_LOAD( "f3577_3.bin",  0x1000, 0x0800, CRC(10f47af2) SHA1(a61b693af264bfa6565c43b4fe473833f8aba046) )
	ROM_LOAD( "f3577_4.bin",  0x1800, 0x0800, CRC(bed8924c) SHA1(1fe3e118cc1c17f4c8b9c0025257822b99fcde38) )
	ROM_LOAD( "f3577_5.bin",  0x2000, 0x0800, CRC(38d671b5) SHA1(3fb3feaaddb08af5ba50a9c08511cbb3949a7985) )
	ROM_LOAD( "f3577_6.bin",  0x2800, 0x0800, CRC(6dbe9c4a) SHA1(11bc4b7c99811bd26423a15b33d02a86fa0bfd17) )

	ROM_REGION( 0x0800, "chargen", 0 ) // possibly some bitrot, see h,m,n in F4 displayer
	ROM_LOAD( "chr_5565.bin", 0x0000, 0x0800, CRC(7d99744f) SHA1(2db330ca94a91f7b2ac2ac088ae9255f5bb0a7b4) )
ROM_END

ROM_START( uts20 )
	ROM_REGION( 0x5000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "uts20a.rom", 0x0000, 0x1000, CRC(1a7b4b4e) SHA1(c3732e25b4b7c7a80172e3fe55c77b923cf511eb) )
	ROM_LOAD( "uts20b.rom", 0x1000, 0x1000, CRC(7f8de87b) SHA1(a85f404ad9d560df831cc3e651a4b45e4ed30130) )
	ROM_LOAD( "uts20c.rom", 0x2000, 0x1000, CRC(4e334705) SHA1(ff1a730551b42f29d20af8ecc4495fd30567d35b) )
	ROM_LOAD( "uts20d.rom", 0x3000, 0x1000, CRC(76757cf7) SHA1(b0509d9a35366b21955f83ec3685163844c4dbf1) )
	ROM_LOAD( "uts20e.rom", 0x4000, 0x1000, CRC(0dfc8062) SHA1(cd681020bfb4829d4cebaf1b5bf618e67b55bda3) )

	// character generator not dumped, using the one from 'UTS10' for now
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "chr_5565.bin", 0x0000, 0x0800, BAD_DUMP CRC(7d99744f) SHA1(2db330ca94a91f7b2ac2ac088ae9255f5bb0a7b4) )
ROM_END

} // Anonymous namespace

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY          FULLNAME  FLAGS
COMP( 1981, uts10, uts20,  0,      uts10,   uts20, univac_state, empty_init, "Sperry Univac", "UTS-10", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1980, uts20, 0,      0,      uts20,   uts20, univac_state, empty_init, "Sperry Univac", "UTS-20", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
