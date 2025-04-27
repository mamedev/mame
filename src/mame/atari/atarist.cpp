// license:BSD-3-Clause
// copyright-holders:Curt Coder, Olivier Galibert
#include "emu.h"

#include "ataristb.h"
#include "atarist_v.h"
#include "stkbd.h"
#include "stmmu.h"
#include "stvideo.h"

#include "bus/centronics/ctronics.h"
#include "bus/midi/midi.h"
#include "bus/rs232/rs232.h"
#include "bus/st/stcart.h"
#include "cpu/m68000/m68000.h"
#include "imagedev/floppy.h"
#include "machine/6850acia.h"
#include "machine/z80scc.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/mc68901.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/rp5c15.h"
#include "machine/wd_fdc.h"
#include "sound/ay8910.h"
#include "sound/lmc1992.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

/*

    TODO:

    - floppy write
    - floppy DMA transfer timer
    - mouse moves too fast?
    - UK keyboard layout for the special keys
    - accurate screen timing
    - STe DMA sound and LMC1992 Microwire mixer
    - Mega ST/STe MC68881 FPU
    - Mega STe 16KB cache
    - Mega STe LAN

    http://dev-docs.atariforge.org/
    http://info-coach.fr/atari/software/protection.php

*/

#include "formats/st_dsk.h"
#include "formats/pasti_dsk.h"
#include "formats/mfi_dsk.h"
#include "formats/dfi_dsk.h"
#include "formats/ipf_dsk.h"

#include "utf8.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define LOG 0

namespace {

#define M68000_TAG      "m68000"
#define YM2149_TAG      "ym2149"
#define MC6850_0_TAG    "mc6850_0"
#define MC6850_1_TAG    "mc6850_1"
#define Z8530_TAG       "z8530"
#define COP888_TAG      "u703"
#define RP5C15_TAG      "rp5c15"
#define YM3439_TAG      "ym3439"
#define MC68901_TAG     "mc68901"
#define LMC1992_TAG     "lmc1992"
#define WD1772_TAG      "wd1772"
#define SCREEN_TAG      "screen"
#define CENTRONICS_TAG  "centronics"
#define RS232_TAG       "rs232"

// Atari ST

#define Y1      XTAL(2'457'600)

// 32028400 also exists
#define Y2      32084988.0
#define Y2_NTSC 32042400.0

// STBook

#define U517    XTAL(16'000'000)
#define Y200    XTAL(2'457'600)
#define Y700    XTAL(10'000'000)

static const double DMASOUND_RATE[] = { Y2/640.0/8.0, Y2/640.0/4.0, Y2/640.0/2.0, Y2/640.0 };

class st_state : public driver_device
{
public:
	st_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, M68000_TAG),
			m_mainram(*this, "mainram", 0x400000, ENDIANNESS_BIG),
			m_ikbd(*this, "ikbd"),
			m_mmu(*this, "mmu"),
			m_stb(*this, "stb"),
			m_fdc(*this, WD1772_TAG),
			m_floppy(*this, WD1772_TAG ":%u", 0U),
			m_mfp(*this, MC68901_TAG),
			m_acia(*this, {MC6850_0_TAG, MC6850_1_TAG}),
			m_centronics(*this, CENTRONICS_TAG),
			m_cart(*this, "cartslot"),
			m_ramcfg(*this, RAM_TAG),
			m_rs232(*this, RS232_TAG),
			m_ymsnd(*this, YM2149_TAG),
			m_config(*this, "config"),
			m_monochrome(1),
			m_video(*this, "video"),
			m_videox(*this, "videox"),
			m_screen(*this, "screen")
	{ }

	void write_monochrome(int state);

	void st(machine_config &config);

protected:
	required_device<m68000_device> m_maincpu;
	memory_share_creator<u16> m_mainram;
	required_device<st_kbd_device> m_ikbd;
	required_device<st_mmu_device> m_mmu;
	optional_device<st_blitter_device> m_stb;
	required_device<wd1772_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<mc68901_device> m_mfp;
	required_device_array<acia6850_device, 2> m_acia;
	required_device<centronics_device> m_centronics;
	required_device<stcart_connector> m_cart;
	required_device<ram_device> m_ramcfg;
	required_device<rs232_port_device> m_rs232;
	required_device<ym2149_device> m_ymsnd;
	optional_ioport m_config;

	TIMER_CALLBACK_MEMBER(mouse_tick);

	void psg_pa_w(uint8_t data);

	void reset_w(int state);

	static void floppy_formats(format_registration &fr);

	int m_monochrome;
	optional_device<st_video_device> m_video;
	optional_device<stx_video_device> m_videox;
	required_device<screen_device> m_screen;

	void common(machine_config &config);
	void cpu_space_map(address_map &map) ATTR_COLD;
	void st_super_map(address_map &map) ATTR_COLD;
	void st_user_map(address_map &map) ATTR_COLD;
	void megast_super_map(address_map &map) ATTR_COLD;

	uint16_t fpu_r();
	void fpu_w(uint16_t data);

	virtual void machine_start() override ATTR_COLD;
};

class megast_state : public st_state
{
public:
	megast_state(const machine_config &mconfig, device_type type, const char *tag)
		: st_state(mconfig, type, tag)
	{ }

	void megast(machine_config &config);
};

class ste_state : public st_state
{
public:
	ste_state(const machine_config &mconfig, device_type type, const char *tag)
		: st_state(mconfig, type, tag),
			m_lmc1992(*this, LMC1992_TAG)
	{ }

	optional_device<lmc1992_device> m_lmc1992;

	uint8_t sound_dma_control_r();
	uint8_t sound_dma_base_r(offs_t offset);
	uint8_t sound_dma_counter_r(offs_t offset);
	uint8_t sound_dma_end_r(offs_t offset);
	uint8_t sound_mode_r();
	void sound_dma_control_w(uint8_t data);
	void sound_dma_base_w(offs_t offset, uint8_t data);
	void sound_dma_end_w(offs_t offset, uint8_t data);
	void sound_mode_w(uint8_t data);
	uint16_t microwire_data_r();
	void microwire_data_w(uint16_t data);
	uint16_t microwire_mask_r();
	void microwire_mask_w(uint16_t data);

	void write_monochrome(int state);

	void dmasound_set_state(int level);
	TIMER_CALLBACK_MEMBER(dmasound_tick);
	void microwire_shift();
	TIMER_CALLBACK_MEMBER(microwire_tick);
	void state_save();

	/* microwire state */
	uint16_t m_mw_data = 0U;
	uint16_t m_mw_mask = 0U;
	int m_mw_shift = 0;

	/* DMA sound state */
	uint32_t m_dmasnd_base = 0U;
	uint32_t m_dmasnd_end = 0U;
	uint32_t m_dmasnd_cntr = 0U;
	uint32_t m_dmasnd_baselatch = 0U;
	uint32_t m_dmasnd_endlatch = 0U;
	uint8_t m_dmasnd_ctrl = 0U;
	uint8_t m_dmasnd_mode = 0U;
	uint8_t m_dmasnd_fifo[8]{};
	uint8_t m_dmasnd_samples = 0U;
	int m_dmasnd_active = 0;

	// timers
	emu_timer *m_microwire_timer = 0;
	emu_timer *m_dmasound_timer = 0;

	void falcon40(machine_config &config);
	void tt030(machine_config &config);
	void falcon(machine_config &config);
	void ste(machine_config &config);
	void ste_super_map(address_map &map) ATTR_COLD;
protected:
	virtual void machine_start() override ATTR_COLD;
};

class megaste_state : public ste_state
{
public:
	megaste_state(const machine_config &mconfig, device_type type, const char *tag)
		: ste_state(mconfig, type, tag)
	{ }

	[[maybe_unused]] uint16_t cache_r();
	[[maybe_unused]] void cache_w(uint16_t data);

	uint16_t m_cache = 0;
	void megaste(machine_config &config);
	void megaste_super_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
};

class stbook_state : public ste_state
{
public:
	stbook_state(const machine_config &mconfig, device_type type, const char *tag)
		: ste_state(mconfig, type, tag),
			m_sw400(*this, "SW400")
	{ }

	required_ioport m_sw400;

	[[maybe_unused]] uint16_t config_r();
	[[maybe_unused]] void lcd_control_w(uint16_t data);

	[[maybe_unused]] void psg_pa_w(uint8_t data);
	uint8_t mfp_gpio_r();
	void stbook_map(address_map &map) ATTR_COLD;
protected:
	virtual void machine_start() override ATTR_COLD;
};


//**************************************************************************
//  FPU
//**************************************************************************

//-------------------------------------------------
//  fpu_r -
//-------------------------------------------------

uint16_t st_state::fpu_r()
{
	// HACK diagnostic cartridge wants to see this value
	return 0x0802;
}


void st_state::fpu_w(uint16_t data)
{
}

void st_state::write_monochrome(int state)
{
	m_monochrome = state;
	m_mfp->i7_w(m_monochrome);
}

void st_state::reset_w(int state)
{
	if (m_video.found())
		m_video->reset();
	if (m_videox.found())
		m_videox->reset();
	if (m_stb.found())
		m_stb->reset();
	m_mfp->reset();
	m_ikbd->reset();
	m_ymsnd->reset();
	m_fdc->soft_reset();
	//m_acsi->reset();
}



//**************************************************************************
//  DMA SOUND
//**************************************************************************

//-------------------------------------------------
//  dmasound_set_state -
//-------------------------------------------------

void ste_state::dmasound_set_state(int level)
{
	m_dmasnd_active = level;
	m_mfp->tai_w(m_dmasnd_active);
	m_mfp->i7_w(m_monochrome ^ m_dmasnd_active);

	if (level == 0)
	{
		m_dmasnd_baselatch = m_dmasnd_base;
		m_dmasnd_endlatch = m_dmasnd_end;
	}
	else
	{
		m_dmasnd_cntr = m_dmasnd_baselatch;
	}
}


void ste_state::write_monochrome(int state)
{
	m_monochrome = state;
	m_mfp->i7_w(m_monochrome ^ m_dmasnd_active);
}

//-------------------------------------------------
//  dmasound_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(ste_state::dmasound_tick)
{
	if (m_dmasnd_samples == 0)
	{
		for (auto & elem : m_dmasnd_fifo)
		{
			elem = m_mainram[m_dmasnd_cntr];
			m_dmasnd_cntr++;
			m_dmasnd_samples++;

			if (m_dmasnd_cntr == m_dmasnd_endlatch)
			{
				dmasound_set_state(0);
				break;
			}
		}
	}

	if (m_dmasnd_ctrl & 0x80)
	{
		if (LOG) logerror("DMA sound left  %i\n", m_dmasnd_fifo[7 - m_dmasnd_samples]);
		m_dmasnd_samples--;

		if (LOG) logerror("DMA sound right %i\n", m_dmasnd_fifo[7 - m_dmasnd_samples]);
		m_dmasnd_samples--;
	}
	else
	{
		if (LOG) logerror("DMA sound mono %i\n", m_dmasnd_fifo[7 - m_dmasnd_samples]);
		m_dmasnd_samples--;
	}

	if ((m_dmasnd_samples == 0) && (m_dmasnd_active == 0))
	{
		if ((m_dmasnd_ctrl & 0x03) == 0x03)
		{
			dmasound_set_state(1);
		}
		else
		{
			m_dmasound_timer->enable(0);
		}
	}
}


//-------------------------------------------------
//  sound_dma_control_r -
//-------------------------------------------------

uint8_t ste_state::sound_dma_control_r()
{
	return m_dmasnd_ctrl;
}


//-------------------------------------------------
//  sound_dma_base_r -
//-------------------------------------------------

uint8_t ste_state::sound_dma_base_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0x00:
		data = (m_dmasnd_base >> 16) & 0x3f;
		break;

	case 0x01:
		data = (m_dmasnd_base >> 8) & 0xff;
		break;

	case 0x02:
		data = m_dmasnd_base & 0xff;
		break;
	}

	return data;
}


//-------------------------------------------------
//  sound_dma_counter_r -
//-------------------------------------------------

uint8_t ste_state::sound_dma_counter_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0x00:
		data = (m_dmasnd_cntr >> 16) & 0x3f;
		break;

	case 0x01:
		data = (m_dmasnd_cntr >> 8) & 0xff;
		break;

	case 0x02:
		data = m_dmasnd_cntr & 0xff;
		break;
	}

	return data;
}


//-------------------------------------------------
//  sound_dma_end_r -
//-------------------------------------------------

uint8_t ste_state::sound_dma_end_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0x00:
		data = (m_dmasnd_end >> 16) & 0x3f;
		break;

	case 0x01:
		data = (m_dmasnd_end >> 8) & 0xff;
		break;

	case 0x02:
		data = m_dmasnd_end & 0xff;
		break;
	}

	return data;
}


//-------------------------------------------------
//  sound_mode_r -
//-------------------------------------------------

uint8_t ste_state::sound_mode_r()
{
	return m_dmasnd_mode;
}


//-------------------------------------------------
//  sound_dma_control_w -
//-------------------------------------------------

void ste_state::sound_dma_control_w(uint8_t data)
{
	m_dmasnd_ctrl = data & 0x03;

	if (m_dmasnd_ctrl & 0x01)
	{
		if (!m_dmasnd_active)
		{
			dmasound_set_state(1);
			m_dmasound_timer->adjust(attotime::zero, 0, attotime::from_hz(DMASOUND_RATE[m_dmasnd_mode & 0x03]));
		}
	}
	else
	{
		dmasound_set_state(0);
		m_dmasound_timer->enable(0);
	}
}


//-------------------------------------------------
//  sound_dma_base_w -
//-------------------------------------------------

void ste_state::sound_dma_base_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		m_dmasnd_base = (data << 16) & 0x3f0000;
		break;
	case 0x01:
		m_dmasnd_base = (m_dmasnd_base & 0x3f00fe) | (data << 8);
		break;
	case 0x02:
		m_dmasnd_base = (m_dmasnd_base & 0x3fff00) | (data & 0xfe);
		break;
	}

	if (!m_dmasnd_active)
	{
		m_dmasnd_baselatch = m_dmasnd_base;
	}
}


//-------------------------------------------------
//  sound_dma_end_w -
//-------------------------------------------------

void ste_state::sound_dma_end_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		m_dmasnd_end = (data << 16) & 0x3f0000;
		break;
	case 0x01:
		m_dmasnd_end = (m_dmasnd_end & 0x3f00fe) | (data & 0xff) << 8;
		break;
	case 0x02:
		m_dmasnd_end = (m_dmasnd_end & 0x3fff00) | (data & 0xfe);
		break;
	}

	if (!m_dmasnd_active)
	{
		m_dmasnd_endlatch = m_dmasnd_end;
	}
}


//-------------------------------------------------
//  sound_mode_w -
//-------------------------------------------------

void ste_state::sound_mode_w(uint8_t data)
{
	m_dmasnd_mode = data & 0x83;
}



//**************************************************************************
//  MICROWIRE
//**************************************************************************

//-------------------------------------------------
//  microwire_shift -
//-------------------------------------------------

void ste_state::microwire_shift()
{
	if (BIT(m_mw_mask, 15))
	{
		m_lmc1992->data_w(BIT(m_mw_data, 15));
		m_lmc1992->clock_w(1);
		m_lmc1992->clock_w(0);
	}

	// rotate mask and data left
	m_mw_mask = (m_mw_mask << 1) | BIT(m_mw_mask, 15);
	m_mw_data = (m_mw_data << 1) | BIT(m_mw_data, 15);
	m_mw_shift++;
}


//-------------------------------------------------
//  microwire_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(ste_state::microwire_tick)
{
	switch (m_mw_shift)
	{
	case 0:
		m_lmc1992->enable_w(0);
		microwire_shift();
		break;

	default:
		microwire_shift();
		break;

	case 15:
		microwire_shift();
		m_lmc1992->enable_w(1);
		m_mw_shift = 0;
		m_microwire_timer->enable(0);
		break;
	}
}


//-------------------------------------------------
//  microwire_data_r -
//-------------------------------------------------

uint16_t ste_state::microwire_data_r()
{
	return m_mw_data;
}


//-------------------------------------------------
//  microwire_data_w -
//-------------------------------------------------

void ste_state::microwire_data_w(uint16_t data)
{
	if (!m_microwire_timer->enabled())
	{
		m_mw_data = data;
		m_microwire_timer->adjust(attotime::zero, 0, attotime::from_usec(2));
	}
}


//-------------------------------------------------
//  microwire_mask_r -
//-------------------------------------------------

uint16_t ste_state::microwire_mask_r()
{
	return m_mw_mask;
}


//-------------------------------------------------
//  microwire_mask_w -
//-------------------------------------------------

void ste_state::microwire_mask_w(uint16_t data)
{
	if (!m_microwire_timer->enabled())
	{
		m_mw_mask = data;
	}
}



//**************************************************************************
//  CACHE
//**************************************************************************

//-------------------------------------------------
//  cache_r -
//-------------------------------------------------

uint16_t megaste_state::cache_r()
{
	return m_cache;
}


//-------------------------------------------------
//  cache_w -
//-------------------------------------------------

void megaste_state::cache_w(uint16_t data)
{
	m_cache = data;

	m_maincpu->set_unscaled_clock(BIT(data, 0) ? Y2/2 : Y2/4);
}



//**************************************************************************
//  STBOOK
//**************************************************************************

//-------------------------------------------------
//  config_r -
//-------------------------------------------------

uint16_t stbook_state::config_r()
{
	/*

	    bit     description

	    0       _POWER_SWITCH
	    1       _TOP_CLOSED
	    2       _RTC_ALARM
	    3       _SOURCE_DEAD
	    4       _SOURCE_LOW
	    5       _MODEM_WAKE
	    6       (reserved)
	    7       _EXPANSION_WAKE
	    8       (reserved)
	    9       (reserved)
	    10      (reserved)
	    11      (reserved)
	    12      (reserved)
	    13      SELF TEST
	    14      LOW SPEED FLOPPY
	    15      DMA AVAILABLE

	*/

	return (m_sw400->read() << 8) | 0xff;
}


//-------------------------------------------------
//  lcd_control_w -
//-------------------------------------------------

void stbook_state::lcd_control_w(uint16_t data)
{
	/*

	    bit     description

	    0       Shadow Chip OFF
	    1       _SHIFTER OFF
	    2       POWEROFF
	    3       _22ON
	    4       RS-232_OFF
	    5       (reserved)
	    6       (reserved)
	    7       MTR_PWR_ON

	*/
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( cpu_space_map )
//-------------------------------------------------

void st_state::cpu_space_map(address_map &map)
{
	map(0xfffff3, 0xfffff3).before_time(m_maincpu, FUNC(m68000_device::vpa_sync)).after_delay(m_maincpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 25; }));
	map(0xfffff5, 0xfffff5).before_time(m_maincpu, FUNC(m68000_device::vpa_sync)).after_delay(m_maincpu, FUNC(m68000_device::vpa_after)).lr8(NAME([this] () -> u8 { m_maincpu->set_input_line(2, CLEAR_LINE); return 26; }));
	map(0xfffff7, 0xfffff7).before_time(m_maincpu, FUNC(m68000_device::vpa_sync)).after_delay(m_maincpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 27; }));
	map(0xfffff9, 0xfffff9).before_time(m_maincpu, FUNC(m68000_device::vpa_sync)).after_delay(m_maincpu, FUNC(m68000_device::vpa_after)).lr8(NAME([this] () -> u8 { m_maincpu->set_input_line(4, CLEAR_LINE); return 28; }));
	map(0xfffffb, 0xfffffb).before_time(m_maincpu, FUNC(m68000_device::vpa_sync)).after_delay(m_maincpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 29; }));
	map(0xfffffd, 0xfffffd).r(m_mfp, FUNC(mc68901_device::get_vector));
	map(0xffffff, 0xffffff).before_time(m_maincpu, FUNC(m68000_device::vpa_sync)).after_delay(m_maincpu, FUNC(m68000_device::vpa_after)).lr8(NAME([] () -> u8 { return 31; }));
}

//-------------------------------------------------
//  ADDRESS_MAP( st_map )
//-------------------------------------------------

void st_state::st_super_map(address_map &map)
{
	// Ram mapped by the mmu
	map.unmap_value_high();
	map(0x000000, 0x000007).rom().region(M68000_TAG, 0);
	map(0x000000, 0x000007).before_delay(NAME([](offs_t) { return 64; })).w(m_maincpu, FUNC(m68000_device::berr_w));
	map(0x400000, 0xf9ffff).before_delay(NAME([](offs_t) { return 64; })).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0xfc0000, 0xfeffff).rom().region(M68000_TAG, 0);
	map(0xfc0000, 0xfeffff).before_delay(NAME([](offs_t) { return 64; })).w(m_maincpu, FUNC(m68000_device::berr_w));


	map(0xff8000, 0xff8fff).m(m_mmu, FUNC(st_mmu_device::map));
	map(0xff8000, 0xff8fff).m(m_video, FUNC(st_video_device::map));
	map(0xff8800, 0xff8800).after_delay(NAME([](offs_t) { return 1; })).rw(YM2149_TAG, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w)).mirror(0xfc);
	map(0xff8802, 0xff8802).after_delay(NAME([](offs_t) { return 1; })).rw(YM2149_TAG, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w)).mirror(0xfc);

	// no blitter on original ST

	map(0xfffa00, 0xfffa3f).after_delay(NAME([](offs_t) { return 4; })).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0xfffc00, 0xfffc03).rw(m_acia[0], FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0xff00);
	map(0xfffc04, 0xfffc07).rw(m_acia[1], FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0xff00);

	// Do a bus error where nothing answers in 64 cycles
	map(0xff0000, 0xff7fff).before_delay(NAME([](offs_t) { return 64; })).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0xff8100, 0xff81ff).before_delay(NAME([](offs_t) { return 64; })).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0xff8300, 0xff85ff).before_delay(NAME([](offs_t) { return 64; })).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0xff8700, 0xff87ff).before_delay(NAME([](offs_t) { return 64; })).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0xff8900, 0xfff9ff).before_delay(NAME([](offs_t) { return 64; })).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0xfffb00, 0xfffbff).before_delay(NAME([](offs_t) { return 64; })).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0xfffd00, 0xffffff).before_delay(NAME([](offs_t) { return 64; })).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
}

void st_state::st_user_map(address_map &map)
{
	// Ram mapped by the mmu
	map.unmap_value_high();
	map(0x000000, 0x0007ff).before_delay(NAME([](offs_t) { return 64; })).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0x400000, 0xf9ffff).before_delay(NAME([](offs_t) { return 64; })).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0xfc0000, 0xfeffff).rom().region(M68000_TAG, 0).w(m_maincpu, FUNC(m68000_device::berr_w));
	map(0xfc0000, 0xfeffff).before_delay(NAME([](offs_t) { return 64; })).w(m_maincpu, FUNC(m68000_device::berr_w));
	map(0xff0000, 0xffffff).before_delay(NAME([](offs_t) { return 64; })).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( megast_map )
//-------------------------------------------------

void st_state::megast_super_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x3fffff).ram().share(m_mainram);
	map(0x000000, 0x000007).rom().region(M68000_TAG, 0);
	//map(0xfa0000, 0xfbffff)      // mapped by the cartslot
	map(0xfc0000, 0xfeffff).rom().region(M68000_TAG, 0);
//  map(0xff7f30, 0xff7f31).rw(m_stb, FUNC(st_blitter_device::dst_inc_y_r), FUNC(st_blitter_device::dst_inc_y_w) // for TOS 1.02
	map(0xff8000, 0xff8fff).m(m_mmu, FUNC(st_mmu_device::map));
	map(0xff8000, 0xff8fff).m(m_video, FUNC(st_video_device::map));
#if 0
	map(0xff8200, 0xff8203).rw(m_videox, FUNC(stx_video_device::shifter_base_r), FUNC(stx_video_device::shifter_base_w)).umask16(0x00ff);
	map(0xff8204, 0xff8209).r(m_videox, FUNC(stx_video_device::shifter_counter_r)).umask16(0x00ff);
	map(0xff820a, 0xff820a).rw(m_videox, FUNC(stx_video_device::shifter_sync_r), FUNC(stx_video_device::shifter_sync_w));
	map(0xff8240, 0xff825f).rw(m_videox, FUNC(stx_video_device::shifter_palette_r), FUNC(stx_video_device::shifter_palette_w));
	map(0xff8260, 0xff8260).rw(m_videox, FUNC(stx_video_device::shifter_mode_r), FUNC(stx_video_device::shifter_mode_w));
#endif
	map(0xff8800, 0xff8800).rw(YM2149_TAG, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0xff8802, 0xff8802).w(YM2149_TAG, FUNC(ay8910_device::data_w));
	map(0xff8a00, 0xff8a1f).rw(m_stb, FUNC(st_blitter_device::halftone_r), FUNC(st_blitter_device::halftone_w));
	map(0xff8a20, 0xff8a21).rw(m_stb, FUNC(st_blitter_device::src_inc_x_r), FUNC(st_blitter_device::src_inc_x_w));
	map(0xff8a22, 0xff8a23).rw(m_stb, FUNC(st_blitter_device::src_inc_y_r), FUNC(st_blitter_device::src_inc_y_w));
	map(0xff8a24, 0xff8a27).rw(m_stb, FUNC(st_blitter_device::src_r), FUNC(st_blitter_device::src_w));
	map(0xff8a28, 0xff8a2d).rw(m_stb, FUNC(st_blitter_device::end_mask_r), FUNC(st_blitter_device::end_mask_w));
	map(0xff8a2e, 0xff8a2f).rw(m_stb, FUNC(st_blitter_device::dst_inc_x_r), FUNC(st_blitter_device::dst_inc_x_w));
	map(0xff8a30, 0xff8a31).rw(m_stb, FUNC(st_blitter_device::dst_inc_y_r), FUNC(st_blitter_device::dst_inc_y_w));
	map(0xff8a32, 0xff8a35).rw(m_stb, FUNC(st_blitter_device::dst_r), FUNC(st_blitter_device::dst_w));
	map(0xff8a36, 0xff8a37).rw(m_stb, FUNC(st_blitter_device::count_x_r), FUNC(st_blitter_device::count_x_w));
	map(0xff8a38, 0xff8a39).rw(m_stb, FUNC(st_blitter_device::count_y_r), FUNC(st_blitter_device::count_y_w));
	map(0xff8a3a, 0xff8a3b).rw(m_stb, FUNC(st_blitter_device::op_r), FUNC(st_blitter_device::op_w));
	map(0xff8a3c, 0xff8a3d).rw(m_stb, FUNC(st_blitter_device::ctrl_r), FUNC(st_blitter_device::ctrl_w));
	map(0xfffa00, 0xfffa3f).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0xfffa40, 0xfffa57).rw(FUNC(st_state::fpu_r), FUNC(st_state::fpu_w));
	map(0xfffc00, 0xfffc03).rw(m_acia[0], FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0xff00);
	map(0xfffc04, 0xfffc07).rw(m_acia[1], FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0xff00);
	map(0xfffc20, 0xfffc3f).rw(RP5C15_TAG, FUNC(rp5c15_device::read), FUNC(rp5c15_device::write)).umask16(0x00ff);
}


//-------------------------------------------------
//  ADDRESS_MAP( ste_map )
//-------------------------------------------------

void ste_state::ste_super_map(address_map &map)
{
	// Ram mapped by the mmu
	map.unmap_value_high();
	map(0x000000, 0x000007).rom().region(M68000_TAG, 0);
	map(0x000000, 0x000007).before_delay(NAME([](offs_t) { return 64; })).w(m_maincpu, FUNC(m68000_device::berr_w));
	map(0x400000, 0xf9ffff).before_delay(NAME([](offs_t) { return 64; })).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	//map(0xfa0000, 0xfbffff)      // mapped by the cartslot
	map(0xe00000, 0xe3ffff).rom().region(M68000_TAG, 0);

	map(0xff8000, 0xff8fff).m(m_mmu, FUNC(st_mmu_device::map));
	map(0xff8800, 0xff8800).rw(YM2149_TAG, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w)).mirror(0xfc);
	map(0xff8802, 0xff8802).rw(YM2149_TAG, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w)).mirror(0xfc);
	map(0xff8901, 0xff8901).rw(FUNC(ste_state::sound_dma_control_r), FUNC(ste_state::sound_dma_control_w));
	map(0xff8902, 0xff8907).rw(FUNC(ste_state::sound_dma_base_r), FUNC(ste_state::sound_dma_base_w)).umask16(0x00ff);
	map(0xff8908, 0xff890d).r(FUNC(ste_state::sound_dma_counter_r)).umask16(0x00ff);
	map(0xff890e, 0xff8913).rw(FUNC(ste_state::sound_dma_end_r), FUNC(ste_state::sound_dma_end_w)).umask16(0x00ff);
	map(0xff8921, 0xff8921).rw(FUNC(ste_state::sound_mode_r), FUNC(ste_state::sound_mode_w));
	map(0xff8922, 0xff8923).rw(FUNC(ste_state::microwire_data_r), FUNC(ste_state::microwire_data_w));
	map(0xff8924, 0xff8925).rw(FUNC(ste_state::microwire_mask_r), FUNC(ste_state::microwire_mask_w));
	map(0xff8a00, 0xff8a1f).rw(m_stb, FUNC(st_blitter_device::halftone_r), FUNC(st_blitter_device::halftone_w));
	map(0xff8a20, 0xff8a21).rw(m_stb, FUNC(st_blitter_device::src_inc_x_r), FUNC(st_blitter_device::src_inc_x_w));
	map(0xff8a22, 0xff8a23).rw(m_stb, FUNC(st_blitter_device::src_inc_y_r), FUNC(st_blitter_device::src_inc_y_w));
	map(0xff8a24, 0xff8a27).rw(m_stb, FUNC(st_blitter_device::src_r), FUNC(st_blitter_device::src_w));
	map(0xff8a28, 0xff8a2d).rw(m_stb, FUNC(st_blitter_device::end_mask_r), FUNC(st_blitter_device::end_mask_w));
	map(0xff8a2e, 0xff8a2f).rw(m_stb, FUNC(st_blitter_device::dst_inc_x_r), FUNC(st_blitter_device::dst_inc_x_w));
	map(0xff8a30, 0xff8a31).rw(m_stb, FUNC(st_blitter_device::dst_inc_y_r), FUNC(st_blitter_device::dst_inc_y_w));
	map(0xff8a32, 0xff8a35).rw(m_stb, FUNC(st_blitter_device::dst_r), FUNC(st_blitter_device::dst_w));
	map(0xff8a36, 0xff8a37).rw(m_stb, FUNC(st_blitter_device::count_x_r), FUNC(st_blitter_device::count_x_w));
	map(0xff8a38, 0xff8a39).rw(m_stb, FUNC(st_blitter_device::count_y_r), FUNC(st_blitter_device::count_y_w));
	map(0xff8a3a, 0xff8a3b).rw(m_stb, FUNC(st_blitter_device::op_r), FUNC(st_blitter_device::op_w));
	map(0xff8a3c, 0xff8a3d).rw(m_stb, FUNC(st_blitter_device::ctrl_r), FUNC(st_blitter_device::ctrl_w));
	map(0xff9200, 0xff9201).portr("JOY0");
	map(0xff9202, 0xff9203).portr("JOY1");
	map(0xff9210, 0xff9211).portr("PADDLE0X");
	map(0xff9212, 0xff9213).portr("PADDLE0Y");
	map(0xff9214, 0xff9215).portr("PADDLE1X");
	map(0xff9216, 0xff9217).portr("PADDLE1Y");
	map(0xff9220, 0xff9221).portr("GUNX");
	map(0xff9222, 0xff9223).portr("GUNY");
	map(0xfffa00, 0xfffa3f).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0xfffc00, 0xfffc03).rw(m_acia[0], FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0xff00);
	map(0xfffc04, 0xfffc07).rw(m_acia[1], FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0xff00);
}


//-------------------------------------------------
//  ADDRESS_MAP( megaste_map )
//-------------------------------------------------

void megaste_state::megaste_super_map(address_map &map)
{
}


//-------------------------------------------------
//  ADDRESS_MAP( stbook_map )
//-------------------------------------------------
#if 0
void stbook_state::stbook_map(address_map &map)
{
	map(0x000000, 0x3fffff).ram().share(m_mainram);
//  map(0xd40000, 0xd7ffff).rom();
	map(0xe00000, 0xe3ffff).rom().region(M68000_TAG, 0);
//  map(0xe80000, 0xebffff).rom();
//  map(0xfa0000, 0xfbffff).rom(); // cartridge
	map(0xfc0000, 0xfeffff).rom().region(M68000_TAG, 0);
/*  map(0xf00000, 0xf1ffff).rw(FUNC(stbook_state::stbook_ide_r), FUNC(stbook_state::stbook_ide_w));
    map(0xff8000, 0xff8001).rw(FUNC(stbook_state::stbook_mmu_r), FUNC(stbook_state::stbook_mmu_w));
    map(0xff8200, 0xff8203).rw(m_videox, FUNC(stbook_video_device::stbook_shifter_base_r), FUNC(stbook_video_device::stbook_shifter_base_w));
    map(0xff8204, 0xff8209).rw(m_videox, FUNC(stbook_video_device::stbook_shifter_counter_r), FUNC(stbook_video_device::stbook_shifter_counter_w));
    map(0xff820a, 0xff820a).rw(m_videox, FUNC(stbook_video_device::stbook_shifter_sync_r), FUNC(stbook_video_device::stbook_shifter_sync_w));
    map(0xff820c, 0xff820d).rw(m_videox, FUNC(stbook_video_device::stbook_shifter_base_low_r), FUNC(stbook_video_device::stbook_shifter_base_low_w));
    map(0xff820e, 0xff820f).rw(m_videox, FUNC(stbook_video_device::stbook_shifter_lineofs_r), FUNC(stbook_video_device::stbook_shifter_lineofs_w));
    map(0xff8240, 0xff8241).rw(m_videox, FUNC(stbook_video_device::stbook_shifter_palette_r), FUNC(stbook_video_device::stbook_shifter_palette_w));
    map(0xff8260, 0xff8260).rw(m_videox, FUNC(stbook_video_device::stbook_shifter_mode_r), FUNC(stbook_video_device::stbook_shifter_mode_w));
    map(0xff8264, 0xff8265).rw(m_videox, FUNC(stbook_video_device::stbook_shifter_pixelofs_r), FUNC(stbook_video_device::stbook_shifter_pixelofs_w));
    map(0xff827e, 0xff827f).w(m_videox, FUNC(stbook_video_device::lcd_control_w));*/
	map(0xff8800, 0xff8800).rw(YM3439_TAG, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0xff8802, 0xff8802).w(YM3439_TAG, FUNC(ay8910_device::data_w));
/*  map(0xff8901, 0xff8901).rw(FUNC(stbook_state::sound_dma_control_r), FUNC(stbook_state::sound_dma_control_w));
    map(0xff8902, 0xff8907).rw(FUNC(stbook_state::sound_dma_base_r), FUNC(stbook_state::sound_dma_base_w)).umask16(0x00ff);
    map(0xff8908, 0xff890d).r(FUNC(stbook_state::sound_dma_counter_r)).umask16(0x00ff);
    map(0xff890e, 0xff8913).rw(FUNC(stbook_state::sound_dma_end_r), FUNC(stbook_state::sound_dma_end_w)).umask16(0x00ff);
    map(0xff8921, 0xff8921).rw(FUNC(stbook_state::sound_mode_r), FUNC(stbook_state::sound_mode_w));
    map(0xff8922, 0xff8923).rw(FUNC(stbook_state::microwire_data_r), FUNC(stbook_state::microwire_data_w));
    map(0xff8924, 0xff8925).rw(FUNC(stbook_state::microwire_mask_r), FUNC(stbook_state::microwire_mask_w));
    map(0xff8a00, 0xff8a1f).rw(m_stb, FUNC(st_blitter_device::halftone_r), FUNC(st_blitter_device::halftone_w));
    map(0xff8a20, 0xff8a21).rw(m_stb, FUNC(st_blitter_device::src_inc_x_r), FUNC(st_blitter_device::src_inc_x_w));
    map(0xff8a22, 0xff8a23).rw(m_stb, FUNC(st_blitter_device::src_inc_y_r), FUNC(st_blitter_device::src_inc_y_w));
    map(0xff8a24, 0xff8a27).rw(m_stb, FUNC(st_blitter_device::src_r), FUNC(st_blitter_device::src_w));
    map(0xff8a28, 0xff8a2d).rw(m_stb, FUNC(st_blitter_device::end_mask_r), FUNC(st_blitter_device::end_mask_w));
    map(0xff8a2e, 0xff8a2f).rw(m_stb, FUNC(st_blitter_device::dst_inc_x_r), FUNC(st_blitter_device::dst_inc_x_w));
    map(0xff8a30, 0xff8a31).rw(m_stb, FUNC(st_blitter_device::dst_inc_y_r), FUNC(st_blitter_device::dst_inc_y_w));
    map(0xff8a32, 0xff8a35).rw(m_stb, FUNC(st_blitter_device::dst_r), FUNC(st_blitter_device::dst_w));
    map(0xff8a36, 0xff8a37).rw(m_stb, FUNC(st_blitter_device::count_x_r), FUNC(st_blitter_device::count_x_w));
    map(0xff8a38, 0xff8a39).rw(m_stb, FUNC(st_blitter_device::count_y_r), FUNC(st_blitter_device::count_y_w));
    map(0xff8a3a, 0xff8a3b).rw(m_stb, FUNC(st_blitter_device::op_r), FUNC(st_blitter_device::op_w));
    map(0xff8a3c, 0xff8a3d).rw(m_stb, FUNC(st_blitter_device::ctrl_r), FUNC(st_blitter_device::ctrl_w));
    map(0xff9200, 0xff9201).r(FUNC(stbook_state::config_r));
    map(0xff9202, 0xff9203).rw(FUNC(stbook_state::lcd_contrast_r), FUNC(stbook_state::lcd_contrast_w));
    map(0xff9210, 0xff9211).rw(FUNC(stbook_state::power_r), FUNC(stbook_state::power_w));
    map(0xff9214, 0xff9215).rw(FUNC(stbook_state::reference_r), FUNC(stbook_state::reference_w));*/
}
#endif


//**************************************************************************
//  INPUT PORTS
//**************************************************************************


//-------------------------------------------------
//  INPUT_PORTS( st )
//-------------------------------------------------

static INPUT_PORTS_START( st )
	PORT_START("config")
	PORT_CONFNAME( 0x80, 0x80, "Monitor") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(st_state::write_monochrome))
	PORT_CONFSETTING( 0x00, "Monochrome (Atari SM124)" )
	PORT_CONFSETTING( 0x80, "Color (Atari SC1224)" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( ste )
//-------------------------------------------------

static INPUT_PORTS_START( ste )
	PORT_START("config")
	PORT_CONFNAME( 0x80, 0x80, "Monitor") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(ste_state::write_monochrome))
	PORT_CONFSETTING( 0x00, "Monochrome (Atari SM124)" )
	PORT_CONFSETTING( 0x80, "Color (Atari SC1435)" )

	PORT_START("JOY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(4) PORT_8WAY

	PORT_START("PADDLE0X")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("PADDLE0Y")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE_V ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("PADDLE1X")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("PADDLE1Y")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE_V ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("GUNX") // should be 10-bit
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUNY") // should be 10-bit
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( stbook )
//-------------------------------------------------

#if 0
static INPUT_PORTS_START( stbook )
	PORT_START("SW400")
	PORT_DIPNAME( 0x80, 0x80, "DMA sound hardware")
	PORT_DIPSETTING( 0x00, DEF_STR( No ) )
	PORT_DIPSETTING( 0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "WD1772 FDC")
	PORT_DIPSETTING( 0x40, "Low Speed (8 MHz)" )
	PORT_DIPSETTING( 0x00, "High Speed (16 MHz)" )
	PORT_DIPNAME( 0x20, 0x00, "Bypass Self Test")
	PORT_DIPSETTING( 0x00, DEF_STR( No ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Yes ) )
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END
#endif


//-------------------------------------------------
//  INPUT_PORTS( tt030 )
//-------------------------------------------------

static INPUT_PORTS_START( tt030 )
	PORT_INCLUDE(ste)
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( falcon )
//-------------------------------------------------

static INPUT_PORTS_START( falcon )
	PORT_INCLUDE(ste)
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  ay8910_interface psg_intf
//-------------------------------------------------

void st_state::psg_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0       SIDE 0
	    1       DRIVE 0
	    2       DRIVE 1
	    3       RTS
	    4       DTR
	    5       STROBE
	    6       GPO
	    7

	*/

	// drive select
	floppy_image_device *floppy = nullptr;
	if (!BIT(data, 1))
		floppy = m_floppy[0]->get_device();
	else if (!BIT(data, 2))
		floppy = m_floppy[1]->get_device();

	// side select
	if (floppy)
		floppy->ss_w(BIT(data, 0) ? 0 : 1);

	m_fdc->set_floppy(floppy);

	// request to send
	m_rs232->write_rts(BIT(data, 3));

	// data terminal ready
	m_rs232->write_dtr(BIT(data, 4));

	// centronics strobe
	m_centronics->write_strobe(BIT(data, 5));
}

//-------------------------------------------------
//  ay8910_interface stbook_psg_intf
//-------------------------------------------------

void stbook_state::psg_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0       SIDE 0
	    1       DRIVE 0
	    2       DRIVE 1
	    3       RTS
	    4       DTR
	    5       STROBE
	    6       IDE RESET
	    7       DDEN

	*/

	// drive select
	floppy_image_device *floppy = nullptr;
	if (!BIT(data, 1))
		floppy = m_floppy[0]->get_device();
	else if (!BIT(data, 2))
		floppy = m_floppy[1]->get_device();

	// side select
	if (floppy)
		floppy->ss_w(BIT(data, 0) ? 0 : 1);

	m_fdc->set_floppy(floppy);

	// request to send
	m_rs232->write_rts(BIT(data, 3));

	// data terminal ready
	m_rs232->write_dtr(BIT(data, 4));

	// centronics strobe
	m_centronics->write_strobe(BIT(data, 5));

	// density select
	m_fdc->dden_w(BIT(data, 7));
}

//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( st )
//-------------------------------------------------

void st_state::machine_start()
{
	m_mmu->set_ram_size(m_ramcfg->size());

	m_cart->map(m_maincpu->space(AS_PROGRAM));
	m_cart->map(m_maincpu->space(m68000_device::AS_USER_PROGRAM));

	/// TODO: get callbacks to trigger these.
	m_mfp->i0_w(1);
	m_mfp->i4_w(1);
	m_mfp->i5_w(1);
	m_mfp->i7_w(1);
}


//-------------------------------------------------
//  state_save -
//-------------------------------------------------

void ste_state::state_save()
{
	save_item(NAME(m_dmasnd_base));
	save_item(NAME(m_dmasnd_end));
	save_item(NAME(m_dmasnd_cntr));
	save_item(NAME(m_dmasnd_baselatch));
	save_item(NAME(m_dmasnd_endlatch));
	save_item(NAME(m_dmasnd_ctrl));
	save_item(NAME(m_dmasnd_mode));
	save_item(NAME(m_dmasnd_fifo));
	save_item(NAME(m_dmasnd_samples));
	save_item(NAME(m_dmasnd_active));
	save_item(NAME(m_mw_data));
	save_item(NAME(m_mw_mask));
	save_item(NAME(m_mw_shift));
}


//-------------------------------------------------
//  MACHINE_START( ste )
//-------------------------------------------------

void ste_state::machine_start()
{
	m_mmu->set_ram_size(m_ramcfg->size());

	m_cart->map(m_maincpu->space(AS_PROGRAM));
	m_cart->map(m_maincpu->space(m68000_device::AS_USER_PROGRAM));

	/* allocate timers */
	m_dmasound_timer = timer_alloc(FUNC(ste_state::dmasound_tick), this);
	m_microwire_timer = timer_alloc(FUNC(ste_state::microwire_tick), this);

	/* register for state saving */
	state_save();

	/// TODO: get callbacks to trigger these.
	m_mfp->i0_w(1);
	m_mfp->i4_w(1);
	m_mfp->i5_w(1);
	m_mfp->i7_w(1);
}


//-------------------------------------------------
//  MACHINE_START( megaste )
//-------------------------------------------------

void megaste_state::machine_start()
{
	ste_state::machine_start();

	save_item(NAME(m_cache));
}


//-------------------------------------------------
//  MACHINE_START( stbook )
//-------------------------------------------------

void stbook_state::machine_start()
{
	/* configure RAM size */
	address_space &program = m_maincpu->space(AS_PROGRAM);

	switch (m_ramcfg->size())
	{
	case 1024 * 1024:
		program.unmap_readwrite(0x100000, 0x3fffff);
		break;
	}

	m_cart->map(m_maincpu->space(AS_PROGRAM));
	m_cart->map(m_maincpu->space(m68000_device::AS_USER_PROGRAM));

	/* register for state saving */
	ste_state::state_save();

	/// TODO: get callbacks to trigger these.
	m_mfp->i0_w(1);
	m_mfp->i4_w(1);
	m_mfp->i5_w(1);
}

void st_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ST_FORMAT);
	fr.add(FLOPPY_MSA_FORMAT);
	fr.add(FLOPPY_PASTI_FORMAT);
	fr.add(FLOPPY_IPF_FORMAT);
}

static void atari_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

void st_state::common(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, Y2/4);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &st_state::cpu_space_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_USER_PROGRAM, &st_state::st_user_map);
	m_maincpu->reset_cb().set(FUNC(st_state::reset_w));

	ST_MMU(config, m_mmu);
	m_mmu->set_ram(m_mainram);
	m_mmu->set_cpu(m_maincpu);
	m_mmu->set_fdc(m_fdc);

	// sound
	YM2149(config, m_ymsnd, Y2/16);
	m_ymsnd->set_flags(AY8910_SINGLE_OUTPUT);
	m_ymsnd->set_resistors_load(RES_K(1), 0, 0);
	m_ymsnd->port_a_write_callback().set(FUNC(st_state::psg_pa_w));
	m_ymsnd->port_b_write_callback().set("cent_data_out", FUNC(output_latch_device::write));

	// devices
	WD1772(config, m_fdc, Y2/4);
	m_fdc->intrq_wr_callback().set(m_mfp, FUNC(mc68901_device::i5_w)).invert();
	m_fdc->drq_wr_callback().set(m_mmu, FUNC(st_mmu_device::fdc_drq_w));
	FLOPPY_CONNECTOR(config, WD1772_TAG ":0", atari_floppies, "35dd",  st_state::floppy_formats);
	FLOPPY_CONNECTOR(config, WD1772_TAG ":1", atari_floppies, nullptr, st_state::floppy_formats);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(m_mfp, FUNC(mc68901_device::i0_w));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	MC68901(config, m_mfp, Y2/8);
	m_mfp->set_timer_clock(Y1);
	m_mfp->out_irq_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	m_mfp->out_tdo_cb().set(m_mfp, FUNC(mc68901_device::tc_w));
	m_mfp->out_tdo_cb().append(m_mfp, FUNC(mc68901_device::rc_w));
	m_mfp->out_so_cb().set(m_rs232, FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_mfp, FUNC(mc68901_device::si_w));
	m_rs232->dcd_handler().set(m_mfp, FUNC(mc68901_device::i1_w));
	m_rs232->cts_handler().set(m_mfp, FUNC(mc68901_device::i2_w));
	m_rs232->ri_handler().set(m_mfp, FUNC(mc68901_device::i6_w));

	ST_KBD(config, m_ikbd);

	ACIA6850(config, m_acia[0]);
	m_acia[0]->txd_handler().set(m_ikbd, FUNC(st_kbd_device::tx_w));
	m_ikbd->rx_cb().set(m_acia[0], FUNC(acia6850_device::write_rxd));
	m_acia[0]->irq_handler().set("aciairq", FUNC(input_merger_device::in_w<0>));
	m_acia[0]->write_cts(0);
	m_acia[0]->write_dcd(0);

	ACIA6850(config, m_acia[1]);
	m_acia[1]->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia[1]->irq_handler().set("aciairq", FUNC(input_merger_device::in_w<1>));
	m_acia[1]->write_cts(0);
	m_acia[1]->write_dcd(0);

	input_merger_device &aciairq(INPUT_MERGER_ANY_HIGH(config, "aciairq"));
	aciairq.output_handler().set(m_mfp, FUNC(mc68901_device::i4_w)).invert();

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_acia[1], FUNC(acia6850_device::write_rxd));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	clock_device &acia_clock(CLOCK(config, "acia_clock", Y2/64)); // 500kHz
	acia_clock.signal_handler().set(m_acia[0], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia[0], FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia[1], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia[1], FUNC(acia6850_device::write_rxc));

	// cartridge
	
	STCART_CONNECTOR(config, m_cart, stcart_intf, nullptr);

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("st_flop");
	SOFTWARE_LIST(config, "cart_list").set_original("st_cart");
}

//-------------------------------------------------
//  machine_config( st )
//-------------------------------------------------

void st_state::st(machine_config &config)
{
	common(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &st_state::st_super_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(m_video, FUNC(st_video_device::screen_update));

	ST_VIDEO(config, m_video, Y2);
	m_video->set_screen(m_screen);
	m_video->set_ram(m_mainram);
	m_video->set_mmu(m_mmu);
	m_video->de_cb().set(m_mfp, FUNC(mc68901_device::tbi_w));
	m_video->hsync_cb().set([this](int state) { if(!state) m_maincpu->set_input_line(2, ASSERT_LINE); });
	m_video->vsync_cb().set([this](int state) { if(!state) m_maincpu->set_input_line(4, ASSERT_LINE); });

	// sound hardware
	SPEAKER(config, "mono").front_center();
	m_ymsnd->add_route(ALL_OUTPUTS, "mono", 1.00);

	// internal ram
	RAM(config, m_ramcfg);
	m_ramcfg->set_default_size("1M"); // 1040ST
	m_ramcfg->set_extra_options("512K,256K"); // 520ST, 260ST
}


//-------------------------------------------------
//  machine_config( megast )
//-------------------------------------------------

void megast_state::megast(machine_config &config)
{
	common(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &megast_state::megast_super_map);

	ST_BLITTER(config, m_stb, Y2/4);
	m_stb->set_space(m_maincpu, AS_PROGRAM);
	m_stb->int_callback().set(m_mfp, FUNC(mc68901_device::i3_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(m_video, FUNC(st_video_device::screen_update));

	ST_VIDEO(config, m_video, Y2);
	m_video->set_screen(m_screen);
	m_video->set_ram(m_mainram);
	m_video->set_mmu(m_mmu);
	m_video->de_cb().set(m_mfp, FUNC(mc68901_device::tbi_w));
	m_video->hsync_cb().set_inputline(m_maincpu, 2);
	m_video->vsync_cb().set_inputline(m_maincpu, 4);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	m_ymsnd->add_route(ALL_OUTPUTS, "mono", 1.00);

	// devices
	RP5C15(config, RP5C15_TAG, XTAL(32'768));

	// internal ram
	RAM(config, m_ramcfg);
	m_ramcfg->set_default_size("4M"); // Mega ST 4
	m_ramcfg->set_extra_options("2M,1M"); // Mega ST 2, Mega ST 1
}


//-------------------------------------------------
//  machine_config( ste )
//-------------------------------------------------

void ste_state::ste(machine_config &config)
{
	common(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &ste_state::ste_super_map);

	ST_BLITTER(config, m_stb, Y2/4);
	m_stb->set_space(m_maincpu, AS_PROGRAM);
	m_stb->int_callback().set(m_mfp, FUNC(mc68901_device::i3_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(m_videox, FUNC(ste_video_device::screen_update));
	m_screen->set_raw(Y2/4, ATARIST_HTOT_PAL, ATARIST_HBEND_PAL, ATARIST_HBSTART_PAL, ATARIST_VTOT_PAL, ATARIST_VBEND_PAL, ATARIST_VBSTART_PAL);

	STE_VIDEO(config, m_videox, Y2);
	m_videox->set_screen(m_screen);
	m_videox->set_ram_space(m_maincpu, AS_PROGRAM);
	m_videox->de_callback().set(m_mfp, FUNC(mc68901_device::tbi_w));

	// sound hardware
	SPEAKER(config, "speaker", 2).front();
	m_ymsnd->add_route(0, "speaker", 0.50, 0);
	m_ymsnd->add_route(0, "speaker", 0.50, 1);
/*
    custom_device &custom_dac(CUSTOM(config, "custom", 0)); // DAC
    custom_dac.add_route(0, "speaker", 0.50);
    custom_dac.add_route(1, "speaker", 0.50);
*/
	LMC1992(config, LMC1992_TAG);

	// internal ram
	RAM(config, m_ramcfg);
	m_ramcfg->set_default_size("1M"); // 1040STe
	m_ramcfg->set_extra_options("512K"); // 520STe
}


//-------------------------------------------------
//  machine_config( megaste )
//-------------------------------------------------

void megaste_state::megaste(machine_config &config)
{
	ste(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &megaste_state::megaste_super_map);
	RP5C15(config, RP5C15_TAG, XTAL(32'768));
	SCC8530(config, Z8530_TAG, Y2/4);

	/* internal ram */
	m_ramcfg->set_default_size("4M"); // Mega STe 4
	m_ramcfg->set_extra_options("2M,1M"); // Mega STe 2, Mega STe 1
}


//-------------------------------------------------
//  machine_config( stbook )
//-------------------------------------------------
#if 0
void stbook_state::stbook(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, U517/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &stbook_state::stbook_map);
	m_maincpu->reset_cb().set(FUNC(st_state::reset_w));

	//COP888(config, COP888_TAG, Y700);

	ST_BLITTER(config, m_stb, U517/2);
	m_stb->set_space(m_maincpu, AS_PROGRAM);
	m_stb->int_callback().set(m_mfp, FUNC(mc68901_device::i3_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_screen_update(m_videox, FUNC(stbook_video_device::screen_update));
	m_screen->set_refresh_hz(60);
	m_screen->set_size(640, 400);
	m_screen->set_visarea(0, 639, 0, 399);

	STBOOK_VIDEO(config, m_videox, Y2);
	m_videox->set_screen(m_screen);
	m_videox->set_ram_space(m_maincpu, AS_PROGRAM);
	m_videox->de_callback().set(m_mfp, FUNC(mc68901_device::tbi_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ym3439_device &ym3439(YM3439(config, YM3439_TAG, U517/8));
	ym3439.set_flags(AY8910_SINGLE_OUTPUT);
	ym3439.set_resistors_load(RES_K(1), 0, 0);
	ym3439.port_a_write_callback().set(FUNC(stbook_state::psg_pa_w));
	ym3439.port_b_write_callback().set("cent_data_out", FUNC(output_latch_device::write));
	ym3439.add_route(ALL_OUTPUTS, "mono", 1.00);

	MC68901(config, m_mfp, U517/8);
	m_mfp->set_timer_clock(Y1);
	m_mfp->out_irq_cb().set_inputline(M68000_TAG, M68K_IRQ_6);
	m_mfp->out_tdo_cb().set(m_mfp, FUNC(mc68901_device::tc_w));
	m_mfp->out_tdo_cb().append(m_mfp, FUNC(mc68901_device::rc_w));
	m_mfp->out_so_cb().set(RS232_TAG, FUNC(rs232_port_device::write_txd));

	WD1772(config, m_fdc, U517/2);
	m_fdc->intrq_wr_callback().set(m_mfp, FUNC(mc68901_device::i5_w)).invert();
	m_fdc->drq_wr_callback().set(FUNC(st_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, WD1772_TAG ":0", atari_floppies, "35dd", 0, st_state::floppy_formats);
	FLOPPY_CONNECTOR(config, WD1772_TAG ":1", atari_floppies, 0,      0, st_state::floppy_formats);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(m_mfp, FUNC(mc68901_device::i0_w));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out");
	m_centronics->set_output_latch(cent_data_out);

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_mfp, FUNC(mc68901_device::si_w));
	m_rs232->dcd_handler().set(m_mfp, FUNC(mc68901_device::i1_w));
	m_rs232->cts_handler().set(m_mfp, FUNC(mc68901_device::i2_w));
	m_rs232->ri_handler().set(m_mfp, FUNC(mc68901_device::i6_w));

	ST_KBD(config, m_ikbd);

	ACIA6850(config, m_acia[0]);
	m_acia[0]->txd_handler().set(m_ikbd, FUNC(st_kbd_device::tx_w));
	m_ikbd->rx_cb().set(m_acia[0], FUNC(acia6850_device::write_rxd));
	m_acia[0]->irq_handler().set("aciairq", FUNC(input_merger_device::in_w<0>));
	m_acia[0]->write_cts(0);
	m_acia[0]->write_dcd(0);

	ACIA6850(config, m_acia[1]);
	m_acia[1]->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia[1]->irq_handler().set("aciairq", FUNC(input_merger_device::in_w<1>));
	m_acia[1]->write_cts(0);
	m_acia[1]->write_dcd(0);

	input_merger_device &aciairq(INPUT_MERGER_ANY_HIGH(config, "aciairq"));
	aciairq.output_handler().set(m_mfp, FUNC(mc68901_device::i4_w)).invert();

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_acia[1], FUNC(acia6850_device::write_rxd));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	clock_device &acia_clock(CLOCK(config, "acia_clock", Y2/64)); // 500kHz
	acia_clock.signal_handler().set(m_acia[0], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia[0], FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia[1], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia[1], FUNC(acia6850_device::write_rxc));

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "st_cart", "bin,rom");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_endian(ENDIANNESS_BIG);
	SOFTWARE_LIST(config, "cart_list").set_original("st_cart");

	/* internal ram */
	RAM(config, m_ramcfg).set_default_size("4M").set_extra_options("1M");
}
#endif

//-------------------------------------------------
//  machine_config( tt030 )
//-------------------------------------------------

void ste_state::tt030(machine_config &config)
{
	ste(config);
}


//-------------------------------------------------
//  machine_config( falcon )
//-------------------------------------------------

void ste_state::falcon(machine_config &config)
{
	ste(config);
}


//-------------------------------------------------
//  machine_config( falcon40 )
//-------------------------------------------------

void ste_state::falcon40(machine_config &config)
{
	ste(config);
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( st )
//-------------------------------------------------

ROM_START( st )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos100")
	ROM_SYSTEM_BIOS( 0, "tos099", "TOS 0.99 (Disk TOS)" )
	ROMX_LOAD( "tos099.bin", 0x00000, 0x04000, CRC(cee3c664) SHA1(80c10b31b63b906395151204ec0a4984c8cb98d6), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos100", "TOS 1.0 (ROM TOS)" )
	ROMX_LOAD( "tos100.bin", 0x00000, 0x30000, CRC(d331af30) SHA1(7bcc2311d122f451bd03c9763ade5a119b2f90da), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "tos102", "TOS 1.02 (MEGA TOS)" )
	ROMX_LOAD( "tos102.bin", 0x00000, 0x30000, CRC(d3c32283) SHA1(735793fdba07fe8d5295caa03484f6ef3de931f5), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104.bin", 0x00000, 0x30000, CRC(90f4fbff) SHA1(2487f330b0895e5d88d580d4ecb24061125e88ad), ROM_BIOS(3) )
ROM_END


//-------------------------------------------------
//  ROM( st_uk )
//-------------------------------------------------

ROM_START( st_uk )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos100")
	ROM_SYSTEM_BIOS( 0, "tos100", "TOS 1.0 (ROM TOS)" )
	ROMX_LOAD( "tos100uk.bin", 0x00000, 0x30000, CRC(1a586c64) SHA1(9a6e4c88533a9eaa4d55cdc040e47443e0226eb2), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos102", "TOS 1.02 (MEGA TOS)" )
	ROMX_LOAD( "tos102uk.bin", 0x00000, 0x30000, CRC(3b5cd0c5) SHA1(87900a40a890fdf03bd08be6c60cc645855cbce5), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104uk.bin", 0x00000, 0x30000, CRC(a50d1d43) SHA1(9526ef63b9cb1d2a7109e278547ae78a5c1db6c6), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  ROM( st_de )
//-------------------------------------------------

ROM_START( st_de )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos100")
	ROM_SYSTEM_BIOS( 0, "tos100", "TOS 1.0 (ROM TOS)" )
	ROMX_LOAD( "tos100de.bin", 0x00000, 0x30000, CRC(16e3e979) SHA1(663d9c87cfb44ae8ada855fe9ed3cccafaa7a4ce), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos102", "TOS 1.02 (MEGA TOS)" )
	ROMX_LOAD( "tos102de.bin", 0x00000, 0x30000, CRC(36a0058e) SHA1(cad5d2902e875d8bf0a14dc5b5b8080b30254148), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104de.bin", 0x00000, 0x30000, CRC(62b82b42) SHA1(5313733f91b083c6265d93674cb9d0b7efd02da8), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "tos10x", "TOS 1.0?" )
	ROMX_LOAD( "st 7c1 a4.u4", 0x00000, 0x08000, CRC(867fdd7e) SHA1(320d12acf510301e6e9ab2e3cf3ee60b0334baa0), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "st 7c1 a9.u7", 0x00001, 0x08000, CRC(30e8f982) SHA1(253f26ff64b202b2681ab68ffc9954125120baea), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "st 7c1 b0.u3", 0x10000, 0x08000, CRC(b91337ed) SHA1(21a338f9bbd87bce4a12d38048e03a361f58d33e), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "st 7a4 a6.u6", 0x10001, 0x08000, CRC(969d7bbe) SHA1(72b998c1f25211c2a96c81a038d71b6a390585c2), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "st 7c1 a2.u2", 0x20000, 0x08000, CRC(d0513329) SHA1(49855a3585e2f75b2af932dd4414ed64e6d9501f), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "st 7c1 b1.u5", 0x20001, 0x08000, CRC(c115cbc8) SHA1(2b52b81a1a4e0818d63f98ee4b25c30e2eba61cb), ROM_SKIP(1) | ROM_BIOS(3) )
ROM_END


//-------------------------------------------------
//  ROM( st_fr )
//-------------------------------------------------

ROM_START( st_fr )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos100")
	ROM_SYSTEM_BIOS( 0, "tos100", "TOS 1.0 (ROM TOS)" )
	ROMX_LOAD( "tos100fr.bin", 0x00000, 0x30000, CRC(2b7f2117) SHA1(ecb00a2e351a6205089a281b4ce6e08959953704), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos102", "TOS 1.02 (MEGA TOS)" )
	ROMX_LOAD( "tos102fr.bin", 0x00000, 0x30000, CRC(8688fce6) SHA1(f5a79aac0a4e812ca77b6ac51d58d98726f331fe), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104fr.bin", 0x00000, 0x30000, CRC(a305a404) SHA1(20dba880344b810cf63cec5066797c5a971db870), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "tos10x", "TOS 1.0?" )
	ROMX_LOAD( "c101658-001.u63", 0x00000, 0x08000, CRC(9c937f6f) SHA1(d4a3ea47568ef6233f3f2056e384b09eedd84961), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "c101661-001.u67", 0x00001, 0x08000, CRC(997298f3) SHA1(9e06d42df88557252a36791b514afe455600f679), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "c101657-001.u59", 0x10000, 0x08000, CRC(b63be6a1) SHA1(434f443472fc649568e4f8be6880f39c2def7819), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "c101660-001.u62", 0x10001, 0x08000, CRC(a813892c) SHA1(d041c113050dfb00166c4a7a52766e1b7eac9cab), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "c101656-001.u48", 0x20000, 0x08000, CRC(dbd93fb8) SHA1(cf9ec11e4bc2465490e7e6c981d9f61eae6cb359), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "c101659-001.u53", 0x20001, 0x08000, CRC(67c9785a) SHA1(917a17e9f83bee015c25b327780eebb11cb2c5a5), ROM_SKIP(1) | ROM_BIOS(3) )
ROM_END


//-------------------------------------------------
//  ROM( st_es )
//-------------------------------------------------

ROM_START( st_es )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos104")
	ROM_SYSTEM_BIOS( 0, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104es.bin", 0x00000, 0x30000, CRC(f4e8ecd2) SHA1(df63f8ac09125d0877b55d5ba1282779b7f99c16), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  ROM( st_nl )
//-------------------------------------------------

ROM_START( st_nl )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos104")
	ROM_SYSTEM_BIOS( 0, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104nl.bin", 0x00000, 0x30000, CRC(bb4370d4) SHA1(6de7c96b2d2e5c68778f4bce3eaf85a4e121f166), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  ROM( st_se )
//-------------------------------------------------

ROM_START( st_se )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos102")
	ROM_SYSTEM_BIOS( 0, "tos102", "TOS 1.02 (MEGA TOS)" )
	ROMX_LOAD( "tos102se.bin", 0x00000, 0x30000, CRC(673fd0c2) SHA1(433de547e09576743ae9ffc43d43f2279782e127), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104se.bin", 0x00000, 0x30000, CRC(80ecfdce) SHA1(b7ad34d5cdfbe86ea74ae79eca11dce421a7bbfd), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( st_sg )
//-------------------------------------------------

ROM_START( st_sg )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos102")
	ROM_SYSTEM_BIOS( 0, "tos102", "TOS 1.02 (MEGA TOS)" )
	ROMX_LOAD( "tos102sg.bin", 0x00000, 0x30000, CRC(5fe16c66) SHA1(45acb2fc4b1b13bd806c751aebd66c8304fc79bc), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104sg.bin", 0x00000, 0x30000, CRC(e58f0bdf) SHA1(aa40bf7203f02b2251b9e4850a1a73ff1c7da106), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( megast )
//-------------------------------------------------

ROM_START( megast )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos104")
	ROM_SYSTEM_BIOS( 0, "tos102", "TOS 1.02 (MEGA TOS)" ) // came in both 6 rom and 2 rom formats; 6 roms are 27512, and 2 roms are non-jedec RP231024 (TC531000 equivalent) 28-pin roms with A16 instead of /OE on pin 22
	ROMX_LOAD( "tos102.bin", 0x00000, 0x30000, CRC(d3c32283) SHA1(735793fdba07fe8d5295caa03484f6ef3de931f5), ROM_BIOS(0) )
	//For a C100167-001 revision B Mega ST motherboard, jumpered for 2 roms:
	//ROMX_LOAD( "c101629-001__(c)atari_1987__38__rp231024e__0564__8807_z07.rp231024.u9", 0x00000, 0x20000, NO_DUMP, ROM_BIOS(0)|ROM_SKIP(1) ) // in u9 HI-0 socket
	//ROMX_LOAD( "c101630-002__(c)atari_1987__38__rp231024e__0563__8809_z10.rp231024.u10", 0x00001, 0x20000, NO_DUMP, ROM_BIOS(0)|ROM_SKIP(1) ) // in u10 LO-0 socket
	//For a C100167-001 revision B Mega ST motherboard, jumpered for 6 roms:
	//ROMX_LOAD( "unknownmarkings_hi-0.27512.u9", 0x00000, 0x10000, NO_DUMP, ROM_BIOS(0)|ROM_SKIP(1) )
	//ROMX_LOAD( "unknownmarkings_lo-0.27512.u10", 0x00001, 0x10000, NO_DUMP, ROM_BIOS(0)|ROM_SKIP(1) )
	//ROMX_LOAD( "unknownmarkings_hi-1.27512.u6", 0x10000, 0x10000, NO_DUMP, ROM_BIOS(0)|ROM_SKIP(1) )
	//ROMX_LOAD( "unknownmarkings_lo-1.27512.u7", 0x10001, 0x10000, NO_DUMP, ROM_BIOS(0)|ROM_SKIP(1) )
	//ROMX_LOAD( "unknownmarkings_hi-2.27512.u3", 0x20000, 0x10000, NO_DUMP, ROM_BIOS(0)|ROM_SKIP(1) )
	//ROMX_LOAD( "unknownmarkings_lo-2.27512.u4", 0x20001, 0x10000, NO_DUMP, ROM_BIOS(0)|ROM_SKIP(1) )
	ROM_SYSTEM_BIOS( 1, "tos104", "TOS 1.04 (Rainbow TOS)" )  // came in both 6 rom and 2 rom formats; 6 roms are 27512, and 2 roms are non-jedec RP231024 (TC531000 equivalent) 28-pin roms with A16 instead of /OE on pin 22
	ROMX_LOAD( "tos104.bin", 0x00000, 0x30000, CRC(90f4fbff) SHA1(2487f330b0895e5d88d580d4ecb24061125e88ad), ROM_BIOS(1) )
	/*For a C100167-001 revision B Mega ST motherboard, jumpered for 2 roms:
	These came in an upgrade kit pouch with label:
	RAINBOW (TOS 1.4)
	CA400407 2CHIPSET
	(C) ATARI CORP.
	and an end-user pamphlet explaining the changes in 1.04 and a sheet giving installation instructions (jumper strapping to convert between 2/6 chip)
	*/
	//ROMX_LOAD( "rainbow_(tos_1.4)__c300683-002_h0__(c)atari_corp.rp231024e.u9", 0x00000, 0x20000, NO_DUMP, ROM_BIOS(1)|ROM_SKIP(1) )
	//ROMX_LOAD( "rainbow_(tos_1.4)__c300684-002_l0__(c)atari_corp.rp231024e.u10", 0x00001, 0x20000, NO_DUMP, ROM_BIOS(1)|ROM_SKIP(1) )
	//For a C100167-001 revision B Mega ST motherboard, jumpered for 6 roms:
	//ROMX_LOAD( "rainbow_(tos_1.4)__c300789-002_h0__(c)atari_corp.27512.u9", 0x00000, 0x10000, NO_DUMP, ROM_BIOS(1)|ROM_SKIP(1) )
	//ROMX_LOAD( "rainbow_(tos_1.4)__c300792-002_l0__(c)atari_corp.27512.u10", 0x00001, 0x10000, NO_DUMP, ROM_BIOS(1)|ROM_SKIP(1) )
	//ROMX_LOAD( "rainbow_(tos_1.4)__c300788-002_h1__(c)atari_corp.27512.u6", 0x10000, 0x10000, NO_DUMP, ROM_BIOS(1)|ROM_SKIP(1) )
	//ROMX_LOAD( "rainbow_(tos_1.4)__c300791-002_l1__(c)atari_corp.27512.u7", 0x10001, 0x10000, NO_DUMP, ROM_BIOS(1)|ROM_SKIP(1) )
	//ROMX_LOAD( "rainbow_(tos_1.4)__c300787-002_h2__(c)atari_corp.27512.u3", 0x20000, 0x10000, NO_DUMP, ROM_BIOS(1)|ROM_SKIP(1) )
	//ROMX_LOAD( "rainbow_(tos_1.4)__c300790-002_l2__(c)atari_corp.27512.u4", 0x20001, 0x10000, NO_DUMP, ROM_BIOS(1)|ROM_SKIP(1) )
ROM_END


//-------------------------------------------------
//  ROM( megast_uk )
//-------------------------------------------------

ROM_START( megast_uk )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos104")
	ROM_SYSTEM_BIOS( 0, "tos102", "TOS 1.02 (MEGA TOS)" )
	ROMX_LOAD( "tos102uk.bin", 0x00000, 0x30000, CRC(3b5cd0c5) SHA1(87900a40a890fdf03bd08be6c60cc645855cbce5), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104uk.bin", 0x00000, 0x30000, CRC(a50d1d43) SHA1(9526ef63b9cb1d2a7109e278547ae78a5c1db6c6), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( megast_de )
//-------------------------------------------------

ROM_START( megast_de )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos104")
	ROM_SYSTEM_BIOS( 0, "tos102", "TOS 1.02 (MEGA TOS)" )
	ROMX_LOAD( "tos102de.bin", 0x00000, 0x30000, CRC(36a0058e) SHA1(cad5d2902e875d8bf0a14dc5b5b8080b30254148), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104de.bin", 0x00000, 0x30000, CRC(62b82b42) SHA1(5313733f91b083c6265d93674cb9d0b7efd02da8), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( megast_fr )
//-------------------------------------------------

ROM_START( megast_fr )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos104")
	ROM_SYSTEM_BIOS( 0, "tos102", "TOS 1.02 (MEGA TOS)" )
	ROMX_LOAD( "tos102fr.bin", 0x00000, 0x30000, CRC(8688fce6) SHA1(f5a79aac0a4e812ca77b6ac51d58d98726f331fe), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104fr.bin", 0x00000, 0x30000, CRC(a305a404) SHA1(20dba880344b810cf63cec5066797c5a971db870), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( megast_se )
//-------------------------------------------------

ROM_START( megast_se )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos104")
	ROM_SYSTEM_BIOS( 0, "tos102", "TOS 1.02 (MEGA TOS)" )
	ROMX_LOAD( "tos102se.bin", 0x00000, 0x30000, CRC(673fd0c2) SHA1(433de547e09576743ae9ffc43d43f2279782e127), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104se.bin", 0x00000, 0x30000, CRC(80ecfdce) SHA1(b7ad34d5cdfbe86ea74ae79eca11dce421a7bbfd), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( megast_sg )
//-------------------------------------------------

ROM_START( megast_sg )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos104")
	ROM_SYSTEM_BIOS( 0, "tos102", "TOS 1.02 (MEGA TOS)" )
	ROMX_LOAD( "tos102sg.bin", 0x00000, 0x30000, CRC(5fe16c66) SHA1(45acb2fc4b1b13bd806c751aebd66c8304fc79bc), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104sg.bin", 0x00000, 0x30000, CRC(e58f0bdf) SHA1(aa40bf7203f02b2251b9e4850a1a73ff1c7da106), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( stacy )
//-------------------------------------------------

#if 0
ROM_START( stacy )
	ROM_REGION16_BE( 0x30000, M68000_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "tos104", "TOS 1.04 (Rainbow TOS)" )
	ROMX_LOAD( "tos104.bin", 0x00000, 0x30000, CRC(a50d1d43) SHA1(9526ef63b9cb1d2a7109e278547ae78a5c1db6c6), ROM_BIOS(0) )
ROM_END
#endif


//-------------------------------------------------
//  ROM( ste )
//-------------------------------------------------

ROM_START( ste )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos206")
	ROM_SYSTEM_BIOS( 0, "tos106", "TOS 1.06 (STE TOS, Revision 1)" )
	ROMX_LOAD( "tos106.bin", 0x00000, 0x40000, CRC(a2e25337) SHA1(6a850810a92fdb1e64d005a06ea4079f51c97145), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos162", "TOS 1.62 (STE TOS, Revision 2)" )
	ROMX_LOAD( "tos162.bin", 0x00000, 0x40000, CRC(1c1a4eba) SHA1(42b875f542e5b728905d819c83c31a095a6a1904), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "tos206", "TOS 2.06 (ST/STE TOS)" )
	ROMX_LOAD( "tos206.bin", 0x00000, 0x40000, CRC(3f2f840f) SHA1(ee58768bdfc602c9b14942ce5481e97dd24e7c83), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  ROM( ste_uk )
//-------------------------------------------------

ROM_START( ste_uk )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos206")
	ROM_SYSTEM_BIOS( 0, "tos106", "TOS 1.06 (STE TOS, Revision 1)" )
	ROMX_LOAD( "tos106uk.bin", 0x00000, 0x40000, CRC(d72fea29) SHA1(06f9ea322e74b682df0396acfaee8cb4d9c90cad), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos162", "TOS 1.62 (STE TOS, Revision 2)" )
	ROMX_LOAD( "tos162uk.bin", 0x00000, 0x40000, CRC(d1c6f2fa) SHA1(70db24a7c252392755849f78940a41bfaebace71), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "tos206", "TOS 2.06 (ST/STE TOS)" )
	ROMX_LOAD( "tos206uk.bin", 0x00000, 0x40000, CRC(08538e39) SHA1(2400ea95f547d6ea754a99d05d8530c03f8b28e3), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  ROM( ste_de )
//-------------------------------------------------

ROM_START( ste_de )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos206")
	ROM_SYSTEM_BIOS( 0, "tos106", "TOS 1.06 (STE TOS, Revision 1)" )
	ROMX_LOAD( "tos106de.bin", 0x00000, 0x40000, CRC(7c67c5c9) SHA1(3b8cf5ffa41b252eb67f8824f94608fa4005d6dd), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos162", "TOS 1.62 (STE TOS, Revision 2)" )
	ROMX_LOAD( "tos162de.bin", 0x00000, 0x40000, CRC(2cdeb5e5) SHA1(10d9f61705048ee3dcbec67df741bed49b922149), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "tos206", "TOS 2.06 (ST/STE TOS)" )
	ROMX_LOAD( "tos206de.bin", 0x00000, 0x40000, CRC(143cd2ab) SHA1(d1da866560734289c4305f1028c36291d331d417), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  ROM( ste_es )
//-------------------------------------------------

ROM_START( ste_es )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos106")
	ROM_SYSTEM_BIOS( 0, "tos106", "TOS 1.06 (STE TOS, Revision 1)" )
	ROMX_LOAD( "tos106es.bin", 0x00000, 0x40000, CRC(5cd2a540) SHA1(3a18f342c8288c0bc1879b7a209c73d5d57f7e81), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  ROM( ste_fr )
//-------------------------------------------------

ROM_START( ste_fr )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos206")
	ROM_SYSTEM_BIOS( 0, "tos106", "TOS 1.06 (STE TOS, Revision 1)" )
	ROMX_LOAD( "tos106fr.bin", 0x00000, 0x40000, CRC(b6e58a46) SHA1(7d7e3cef435caa2fd7733a3fbc6930cb9ea7bcbc), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos162", "TOS 1.62 (STE TOS, Revision 2)" )
	ROMX_LOAD( "tos162fr.bin", 0x00000, 0x40000, CRC(0ab003be) SHA1(041e134da613f718fca8bd47cd7733076e8d7588), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "tos206", "TOS 2.06 (ST/STE TOS)" )
	ROMX_LOAD( "tos206fr.bin", 0x00000, 0x40000, CRC(e3a99ca7) SHA1(387da431e6e3dd2e0c4643207e67d06cf33618c3), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  ROM( ste_it )
//-------------------------------------------------

ROM_START( ste_it )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos106")
	ROM_SYSTEM_BIOS( 0, "tos106", "TOS 1.06 (STE TOS, Revision 1)" )
	ROMX_LOAD( "tos106it.bin", 0x00000, 0x40000, CRC(d3a55216) SHA1(28dc74e5e0fa56b685bbe15f9837f52684fee9fd), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  ROM( ste_se )
//-------------------------------------------------

ROM_START( ste_se )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos206")
	ROM_SYSTEM_BIOS( 0, "tos162", "TOS 1.62 (STE TOS, Revision 2)" )
	ROMX_LOAD( "tos162se.bin", 0x00000, 0x40000, CRC(90f124b1) SHA1(6e5454e861dbf4c46ce5020fc566c31202087b88), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos206", "TOS 2.06 (ST/STE TOS)" )
	ROMX_LOAD( "tos206se.bin", 0x00000, 0x40000, CRC(be61906d) SHA1(ebdf5a4cf08471cd315a91683fcb24e0f029d451), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( ste_sg )
//-------------------------------------------------

ROM_START( ste_sg )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos206")
	ROM_SYSTEM_BIOS( 0, "tos206", "TOS 2.06 (ST/STE TOS)" )
	ROMX_LOAD( "tos206sg.bin", 0x00000, 0x40000, CRC(8c4fe57d) SHA1(c7a9ae3162f020dcac0c2a46cf0c033f91b98644), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  ROM( megaste )
//-------------------------------------------------

ROM_START( megaste )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos206")
	ROM_SYSTEM_BIOS( 0, "tos205", "TOS 2.05 (Mega STE TOS)" )
	ROMX_LOAD( "atari mega ste 205 018 tms27c010.bin", 0x00000, 0x20000, CRC(befac3ab) SHA1(5b49f101f15a4d1c89cfd1d7ce3fec84a5ca36d0), ROM_BIOS(0) | ROM_SKIP(1) )
	ROMX_LOAD( "atari mega ste 205 019 tms27c010.bin", 0x00001, 0x20000, CRC(ea2a136d) SHA1(c3c259293de562d2a0fac4d41f95cf3d42ad6df4), ROM_BIOS(0) | ROM_SKIP(1) )
	ROM_SYSTEM_BIOS( 1, "tos206", "TOS 2.06 (ST/STE TOS)" )
	ROMX_LOAD( "tos206.bin", 0x00000, 0x40000, CRC(3f2f840f) SHA1(ee58768bdfc602c9b14942ce5481e97dd24e7c83), ROM_BIOS(1) )
	ROMX_LOAD( "tos206.bin", 0x00000, 0x40000, CRC(3f2f840f) SHA1(ee58768bdfc602c9b14942ce5481e97dd24e7c83), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( megaste_uk )
//-------------------------------------------------

ROM_START( megaste_uk )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos206")
#if 0
	ROM_SYSTEM_BIOS( 0, "tos202", "TOS 2.02 (Mega STE TOS)" )
	ROMX_LOAD( "tos202uk.bin", 0x00000, 0x40000, NO_DUMP, ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos205", "TOS 2.05 (Mega STE TOS)" )
	ROMX_LOAD( "tos205uk.bin", 0x00000, 0x40000, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "tos206", "TOS 2.06 (ST/STE TOS)" )
	ROMX_LOAD( "tos206uk.bin", 0x00000, 0x40000, CRC(08538e39) SHA1(2400ea95f547d6ea754a99d05d8530c03f8b28e3), ROM_BIOS(2) )
#else
	ROM_SYSTEM_BIOS( 0, "tos206", "TOS 2.06 (ST/STE TOS)" )
	ROMX_LOAD( "tos206uk.bin", 0x00000, 0x40000, CRC(08538e39) SHA1(2400ea95f547d6ea754a99d05d8530c03f8b28e3), ROM_BIOS(0) )
#endif
ROM_END


//-------------------------------------------------
//  ROM( megaste_fr )
//-------------------------------------------------

ROM_START( megaste_fr )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos206")
	ROM_SYSTEM_BIOS( 0, "tos205", "TOS 2.05 (Mega STE TOS)" )
	ROMX_LOAD( "tos205fr.bin", 0x00000, 0x40000, CRC(27b83d2f) SHA1(83963b0feb0d119b2ca6f51e483e8c20e6ab79e1), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos206", "TOS 2.06 (ST/STE TOS)" )
	ROMX_LOAD( "tos206fr.bin", 0x00000, 0x40000, CRC(e3a99ca7) SHA1(387da431e6e3dd2e0c4643207e67d06cf33618c3), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( megaste_de )
//-------------------------------------------------

ROM_START( megaste_de )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos206")
	ROM_SYSTEM_BIOS( 0, "tos205", "TOS 2.05 (Mega STE TOS)" )
	ROMX_LOAD( "tos205de.bin", 0x00000, 0x40000, CRC(518b24e6) SHA1(084e083422f8fd9ac7a2490f19b81809c52b91b4), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos206", "TOS 2.06 (ST/STE TOS)" )
	ROMX_LOAD( "tos206de.bin", 0x00000, 0x40000, CRC(143cd2ab) SHA1(d1da866560734289c4305f1028c36291d331d417), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( megaste_es )
//-------------------------------------------------

ROM_START( megaste_es )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos205")
	ROM_SYSTEM_BIOS( 0, "tos205", "TOS 2.05 (Mega STE TOS)" )
	ROMX_LOAD( "tos205es.bin", 0x00000, 0x40000, CRC(2a426206) SHA1(317715ad8de718b5acc7e27ecf1eb833c2017c91), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  ROM( megaste_it )
//-------------------------------------------------

ROM_START( megaste_it )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos205")
	ROM_SYSTEM_BIOS( 0, "tos205", "TOS 2.05 (Mega STE TOS)" )
	ROMX_LOAD( "tos205it.bin", 0x00000, 0x40000, CRC(b28bf5a1) SHA1(8e0581b442384af69345738849cf440d72f6e6ab), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  ROM( megaste_se )
//-------------------------------------------------

ROM_START( megaste_se )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos206")
	ROM_SYSTEM_BIOS( 0, "tos205", "TOS 2.05 (Mega STE TOS)" )
	ROMX_LOAD( "tos205se.bin", 0x00000, 0x40000, CRC(6d49ccbe) SHA1(c065b1a9a2e42e5e373333e99be829028902acaa), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos206", "TOS 2.06 (ST/STE TOS)" )
	ROMX_LOAD( "tos206se.bin", 0x00000, 0x40000, CRC(be61906d) SHA1(ebdf5a4cf08471cd315a91683fcb24e0f029d451), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( stbook )
//-------------------------------------------------

#if 0
ROM_START( stbook )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "tos208", "TOS 2.08" )
	ROMX_LOAD( "tos208.bin", 0x00000, 0x40000, NO_DUMP, ROM_BIOS(0) )

	ROM_REGION( 0x1000, COP888_TAG, 0 )
	ROM_LOAD( "cop888c0.u703", 0x0000, 0x1000, NO_DUMP )
ROM_END
#endif


//-------------------------------------------------
//  ROM( stpad )
//-------------------------------------------------

#if 0
ROM_START( stpad )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "tos205", "TOS 2.05" )
	ROMX_LOAD( "tos205.bin", 0x00000, 0x40000, NO_DUMP, ROM_BIOS(0) )
ROM_END
#endif


//-------------------------------------------------
//  ROM( tt030 )
//-------------------------------------------------

ROM_START( tt030 )
	ROM_REGION32_BE( 0x80000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos306")
	ROM_SYSTEM_BIOS( 0, "tos306", "TOS 3.06 (TT TOS)" )
	ROMX_LOAD( "tos306.bin", 0x00000, 0x80000, CRC(e65adbd7) SHA1(b15948786278e1f2abc4effbb6d40786620acbe8), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  ROM( tt030_uk )
//-------------------------------------------------

ROM_START( tt030_uk )
	ROM_REGION32_BE( 0x80000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos306")
	ROM_SYSTEM_BIOS( 0, "tos306", "TOS 3.06 (TT TOS)" )
	ROMX_LOAD( "tos306uk.bin", 0x00000, 0x80000, CRC(75dda215) SHA1(6325bdfd83f1b4d3afddb2b470a19428ca79478b), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  ROM( tt030_de )
//-------------------------------------------------

ROM_START( tt030_de )
	ROM_REGION32_BE( 0x80000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos306")
	ROM_SYSTEM_BIOS( 0, "tos306", "TOS 3.06 (TT TOS)" )
	ROMX_LOAD( "tos306de.bin", 0x00000, 0x80000, CRC(4fcbb59d) SHA1(80af04499d1c3b8551fc4d72142ff02c2182e64a), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  ROM( tt030_fr )
//-------------------------------------------------

ROM_START( tt030_fr )
	ROM_REGION32_BE( 0x80000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos306")
	ROM_SYSTEM_BIOS( 0, "tos306", "TOS 3.06 (TT TOS)" )
	ROMX_LOAD( "tos306fr.bin", 0x00000, 0x80000, CRC(1945511c) SHA1(6bb19874e1e97dba17215d4f84b992c224a81b95), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  ROM( tt030_pl )
//-------------------------------------------------

ROM_START( tt030_pl )
	ROM_REGION32_BE( 0x80000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos306")
	ROM_SYSTEM_BIOS( 0, "tos306", "TOS 3.06 (TT TOS)" )
	ROMX_LOAD( "tos306pl.bin", 0x00000, 0x80000, CRC(4f2404bc) SHA1(d122b8ceb202b52754ff0d442b1c81f8b4de3436), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  ROM( fx1 )
//-------------------------------------------------

#if 0
ROM_START( fx1 )
	ROM_REGION16_BE( 0x40000, M68000_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "tos207", "TOS 2.07" )
	ROMX_LOAD( "tos207.bin", 0x00000, 0x40000, NO_DUMP, ROM_BIOS(0) )
ROM_END
#endif


//-------------------------------------------------
//  ROM( falcon30 )
//-------------------------------------------------

ROM_START( falcon30 )
	ROM_REGION32_BE( 0x80000, M68000_TAG, 0 )
	ROM_DEFAULT_BIOS("tos404")
	ROM_SYSTEM_BIOS( 0, "tos400", "TOS 4.00" )
	ROMX_LOAD( "tos400.bin", 0x00000, 0x80000, CRC(4fcf2471) SHA1(aed6e21226544a5b5ea4eaa0fad4d4c31767cbe9), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "tos401", "TOS 4.01" )
	ROMX_LOAD( "tos401.bin", 0x00000, 0x80000, CRC(4a1f42af) SHA1(7157f6a8aff275cfbb5ea6aa8e788dda8a977e56), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "tos402", "TOS 4.02" )
	ROMX_LOAD( "tos402.bin", 0x00000, 0x80000, CRC(63f82f23) SHA1(75de588f6bbc630fa9c814f738195da23b972cc6), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "tos404", "TOS 4.04" )
	ROMX_LOAD( "tos404.bin", 0x00000, 0x80000, CRC(028b561d) SHA1(27dcdb31b0951af99023b2fb8c370d8447ba6ebc), ROM_BIOS(3) )
ROM_END


//-------------------------------------------------
//  ROM( falcon40 )
//-------------------------------------------------

ROM_START( falcon40 )
	ROM_REGION32_BE( 0x80000, M68000_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "tos492", "TOS 4.92" )
	ROMX_LOAD( "tos492.bin", 0x00000, 0x7d314, CRC(bc8e497f) SHA1(747a38042844a6b632dcd9a76d8525fccb5eb892), ROM_BIOS(0) )
ROM_END

} // anonymous namespace



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT    COMPAT  MACHINE   INPUT   CLASS          INIT        COMPANY  FULLNAME                 FLAGS
COMP( 1985, st,         0,        0,      st,       st,     st_state,      empty_init, "Atari", "ST (USA)",              MACHINE_NOT_WORKING )
COMP( 1985, st_uk,      st,       0,      st,       st,     st_state,      empty_init, "Atari", "ST (UK)",               MACHINE_NOT_WORKING )
COMP( 1985, st_de,      st,       0,      st,       st,     st_state,      empty_init, "Atari", "ST (Germany)",          MACHINE_NOT_WORKING )
COMP( 1985, st_es,      st,       0,      st,       st,     st_state,      empty_init, "Atari", "ST (Spain)",            MACHINE_NOT_WORKING )
COMP( 1985, st_fr,      st,       0,      st,       st,     st_state,      empty_init, "Atari", "ST (France)",           MACHINE_NOT_WORKING )
COMP( 1985, st_nl,      st,       0,      st,       st,     st_state,      empty_init, "Atari", "ST (Netherlands)",      MACHINE_NOT_WORKING )
COMP( 1985, st_se,      st,       0,      st,       st,     st_state,      empty_init, "Atari", "ST (Sweden)",           MACHINE_NOT_WORKING )
COMP( 1985, st_sg,      st,       0,      st,       st,     st_state,      empty_init, "Atari", "ST (Switzerland)",      MACHINE_NOT_WORKING )
COMP( 1987, megast,     st,       0,      megast,   st,     megast_state,  empty_init, "Atari", "MEGA ST (USA)",         MACHINE_NOT_WORKING )
COMP( 1987, megast_uk,  st,       0,      megast,   st,     megast_state,  empty_init, "Atari", "MEGA ST (UK)",          MACHINE_NOT_WORKING )
COMP( 1987, megast_de,  st,       0,      megast,   st,     megast_state,  empty_init, "Atari", "MEGA ST (Germany)",     MACHINE_NOT_WORKING )
COMP( 1987, megast_fr,  st,       0,      megast,   st,     megast_state,  empty_init, "Atari", "MEGA ST (France)",      MACHINE_NOT_WORKING )
COMP( 1987, megast_se,  st,       0,      megast,   st,     megast_state,  empty_init, "Atari", "MEGA ST (Sweden)",      MACHINE_NOT_WORKING )
COMP( 1987, megast_sg,  st,       0,      megast,   st,     megast_state,  empty_init, "Atari", "MEGA ST (Switzerland)", MACHINE_NOT_WORKING )
COMP( 1989, ste,        0,        0,      ste,      ste,    ste_state,     empty_init, "Atari", "STe (USA)",             MACHINE_NOT_WORKING )
COMP( 1989, ste_uk,     ste,      0,      ste,      ste,    ste_state,     empty_init, "Atari", "STe (UK)",              MACHINE_NOT_WORKING )
COMP( 1989, ste_de,     ste,      0,      ste,      ste,    ste_state,     empty_init, "Atari", "STe (Germany)",         MACHINE_NOT_WORKING )
COMP( 1989, ste_es,     ste,      0,      ste,      ste,    ste_state,     empty_init, "Atari", "STe (Spain)",           MACHINE_NOT_WORKING )
COMP( 1989, ste_fr,     ste,      0,      ste,      ste,    ste_state,     empty_init, "Atari", "STe (France)",          MACHINE_NOT_WORKING )
COMP( 1989, ste_it,     ste,      0,      ste,      ste,    ste_state,     empty_init, "Atari", "STe (Italy)",           MACHINE_NOT_WORKING )
COMP( 1989, ste_se,     ste,      0,      ste,      ste,    ste_state,     empty_init, "Atari", "STe (Sweden)",          MACHINE_NOT_WORKING )
COMP( 1989, ste_sg,     ste,      0,      ste,      ste,    ste_state,     empty_init, "Atari", "STe (Switzerland)",     MACHINE_NOT_WORKING )
//COMP( 1990, stbook,     ste,      0,      stbook,   stbook, stbook_state,  empty_init, "Atari", "STBook",                MACHINE_NOT_WORKING )
COMP( 1990, tt030,      0,        0,      tt030,    tt030,  ste_state,     empty_init, "Atari", "TT030 (USA)",           MACHINE_NOT_WORKING )
COMP( 1990, tt030_uk,   tt030,    0,      tt030,    tt030,  ste_state,     empty_init, "Atari", "TT030 (UK)",            MACHINE_NOT_WORKING )
COMP( 1990, tt030_de,   tt030,    0,      tt030,    tt030,  ste_state,     empty_init, "Atari", "TT030 (Germany)",       MACHINE_NOT_WORKING )
COMP( 1990, tt030_fr,   tt030,    0,      tt030,    tt030,  ste_state,     empty_init, "Atari", "TT030 (France)",        MACHINE_NOT_WORKING )
COMP( 1990, tt030_pl,   tt030,    0,      tt030,    tt030,  ste_state,     empty_init, "Atari", "TT030 (Poland)",        MACHINE_NOT_WORKING )
COMP( 1991, megaste,    ste,      0,      megaste,  st,     megaste_state, empty_init, "Atari", "MEGA STe (USA)",        MACHINE_NOT_WORKING )
COMP( 1991, megaste_uk, ste,      0,      megaste,  st,     megaste_state, empty_init, "Atari", "MEGA STe (UK)",         MACHINE_NOT_WORKING )
COMP( 1991, megaste_de, ste,      0,      megaste,  st,     megaste_state, empty_init, "Atari", "MEGA STe (Germany)",    MACHINE_NOT_WORKING )
COMP( 1991, megaste_es, ste,      0,      megaste,  st,     megaste_state, empty_init, "Atari", "MEGA STe (Spain)",      MACHINE_NOT_WORKING )
COMP( 1991, megaste_fr, ste,      0,      megaste,  st,     megaste_state, empty_init, "Atari", "MEGA STe (France)",     MACHINE_NOT_WORKING )
COMP( 1991, megaste_it, ste,      0,      megaste,  st,     megaste_state, empty_init, "Atari", "MEGA STe (Italy)",      MACHINE_NOT_WORKING )
COMP( 1991, megaste_se, ste,      0,      megaste,  st,     megaste_state, empty_init, "Atari", "MEGA STe (Sweden)",     MACHINE_NOT_WORKING )
COMP( 1992, falcon30,   0,        0,      falcon,   falcon, ste_state,     empty_init, "Atari", "Falcon030",             MACHINE_NOT_WORKING )
COMP( 1992, falcon40,   falcon30, 0,      falcon40, falcon, ste_state,     empty_init, "Atari", "Falcon040 (prototype)", MACHINE_NOT_WORKING )
//COMP( 1989, stacy,      st,       0,      stacy,    stacy,  st_state,      empty_init, "Atari", "Stacy",                 MACHINE_NOT_WORKING )
//COMP( 1991, stpad,      ste,      0,      stpad,    stpad,  st_state,      empty_init, "Atari", "STPad (prototype)",     MACHINE_NOT_WORKING )
//COMP( 1992, fx1,        0,        0,      falcon,   falcon, ste_state,     empty_init, "Atari", "FX-1 (prototype)",      MACHINE_NOT_WORKING )
