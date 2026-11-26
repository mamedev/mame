// license:BSD-3-Clause
// copyright-holders: Scott Stone, Peter Wilhelmsen

/***************************************************************************

Reel Magic - Casino Slots
Play System (Impera license)

1x XILINX XCS30 PQ240CKN9825 A2015802A
1x MC68HC000FN8
1x XILINX XC9572 TQ100AEM9917
1x XILINX XC95108 TQ100AEM9909
1x XILINX XC9536 VQ44ASJ9917
2x GM76C8128CLLFW70
4x GM76C256CLLFW70
2x TD62083AP
1x TL7705ACP
1x HIN232CP

1x XTAL: 58.00000Mhz

UNDUMPED:
1x PIC16C621A  ==> PROTECTED

The initial stack pointer, 0x018a9e, intentionally points into ROM.  The
first JSR attempts to push its return address at 0x018a9a, where the ROM
contains 0x00018a9e.  The subsequent RTS enters the bootstrap at 0x018a9e,
which sets up the RAM stack at 0xffdffc and uploads the FPGA configuration.
The configuration at 0x0c0000 is 0x7914 bytes long and is XORed with 0x5f;
the upload code derives this key from the high byte read at 0x800008.

The default EEPROM contents come from the original IC19/IC20 dumps.  They
include the event history log, which CLEAR ALL DATA does not erase.  The
default battery SRAM contents are the state after MASTER KEY -> BOOKKEEPING
-> CLEAR ALL DATA.

TODO / hardware measurements required:
- Verify CPU clock divider, raster totals, and display/IRQ control bits.
- Identify the physical sound playback device and measure its sample rate.
- Verify RTC status/control behavior and electronic operator-key variants.
- Verify complete SRAM address decoding and hopper timing.

The Xilinx bitstream is recovered, but its logic is modeled from observed
software behavior; this is not an FPGA netlist simulation.  The inferred
host-facing protocol of the undumped PIC is modeled below.

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/pic16_mid/pic16_mid.h"
#include "machine/eeprompar.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/dac.h"

#include "dirtc.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <algorithm>
#include <array>
#include <vector>


namespace {

/***************************************************************************

    Serial real time clock

    TODO: identify real model

    The register map (BCD digits at 0-6 and 8-d, interrupt interval at 7,
    control at e, test at f) and the read command (0x60 | register) used
    by the firmware match the Linux rtc-rs5c313 driver.  The firmware also
    sets bit 7 in the address (0xa0) and write data (0x90) commands, whose
    meaning is unknown.

    Only what the game uses is implemented: 24-hour mode, plain storage
    for the control bits (no ADJ/BSY) and for the test register.

***************************************************************************/

class reelmag_rtc_device : public device_t, public device_rtc_interface
{
public:
	reelmag_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void ce_w(int state);
	void sclk_w(int state);
	void sio_w(int state) { m_sio_in = state ? 1 : 0; }
	int sio_r() const { return m_sio_out; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
	virtual bool rtc_feature_leap_year() const override { return true; }

private:
	enum : u8
	{
		REG_SECOND = 0x0,
		REG_MINUTE = 0x2,
		REG_HOUR = 0x4,
		REG_WEEKDAY = 0x6,
		REG_INTERVAL = 0x7,
		REG_DAY = 0x8,
		REG_MONTH = 0xa,
		REG_YEAR = 0xc,
		REG_CONTROL = 0xe,
		REG_TEST = 0xf
	};

	TIMER_CALLBACK_MEMBER(clock_tick);

	void command_w(u8 command);
	void time_written();

	std::array<u8, 16> m_regs;
	bool m_ce;
	bool m_sclk;
	u8 m_sio_in;
	u8 m_sio_out;
	bool m_reading;
	u8 m_count;
	u8 m_shift;
	u8 m_address;
};

DEFINE_DEVICE_TYPE_PRIVATE(REELMAG_RTC, reelmag_rtc_device, reelmag_rtc_device, "reelmag_rtc", "Reel Magic serial RTC")

reelmag_rtc_device::reelmag_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, REELMAG_RTC, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, m_regs{}
	, m_ce(false)
	, m_sclk(false)
	, m_sio_in(0)
	, m_sio_out(0)
	, m_reading(false)
	, m_count(0)
	, m_shift(0)
	, m_address(0)
{
}

void reelmag_rtc_device::device_start()
{
	timer_alloc(FUNC(reelmag_rtc_device::clock_tick), this)->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));

	save_item(NAME(m_regs));
	save_item(NAME(m_ce));
	save_item(NAME(m_sclk));
	save_item(NAME(m_sio_in));
	save_item(NAME(m_sio_out));
	save_item(NAME(m_reading));
	save_item(NAME(m_count));
	save_item(NAME(m_shift));
	save_item(NAME(m_address));
}

void reelmag_rtc_device::device_reset()
{
	m_ce = false;
	m_sclk = false;
	m_sio_out = 0;
	m_reading = false;
	m_count = 0;
	m_shift = 0;
	m_address = 0;
}

void reelmag_rtc_device::device_post_load()
{
	time_written();
}

TIMER_CALLBACK_MEMBER(reelmag_rtc_device::clock_tick)
{
	advance_seconds();
}

void reelmag_rtc_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	auto const store = [this] (unsigned reg, int value)
	{
		m_regs[reg] = value % 10;
		m_regs[reg + 1] = (value / 10) % 10;
	};
	store(REG_SECOND, second);
	store(REG_MINUTE, minute);
	store(REG_HOUR, hour);
	store(REG_DAY, day);
	store(REG_MONTH, month);
	store(REG_YEAR, year % 100);
	// zero-based weekday, the firmware does not read it in its date/time routine
	m_regs[REG_WEEKDAY] = (day_of_week >= 1) ? ((day_of_week - 1) % 7) : 0;
}

void reelmag_rtc_device::time_written()
{
	auto const load = [this] (unsigned reg, int minimum, int maximum)
	{
		return std::clamp(m_regs[reg] + 10 * m_regs[reg + 1], minimum, maximum);
	};
	set_time(false,
			load(REG_YEAR, 0, 99),
			load(REG_MONTH, 1, 12),
			load(REG_DAY, 1, 31),
			std::clamp(m_regs[REG_WEEKDAY] + 1, 1, 7),
			load(REG_HOUR, 0, 23),
			load(REG_MINUTE, 0, 59),
			load(REG_SECOND, 0, 59));
}

void reelmag_rtc_device::ce_w(int state)
{
	if (state && !m_ce)
	{
		m_reading = false;
		m_count = 0;
		m_shift = 0;
	}
	m_ce = state != 0;
}

void reelmag_rtc_device::sclk_w(int state)
{
	bool const rising = state && !m_sclk;
	m_sclk = state != 0;
	if (!m_ce || !rising)
		return;

	if (m_reading)
	{
		// read data is shifted out most significant bit first
		m_sio_out = BIT(m_shift, 7 - m_count);
		if (++m_count == 8)
		{
			m_reading = false;
			m_count = 0;
			m_shift = 0;
		}
	}
	else
	{
		m_shift = (m_shift << 1) | m_sio_in;
		if (++m_count == 8)
		{
			u8 const command = m_shift;
			m_count = 0;
			m_shift = 0;
			command_w(command);
		}
	}
}

void reelmag_rtc_device::command_w(u8 command)
{
	switch (command & 0x70)
	{
	case 0x60: // read: one 4-bit register, returned in the low half of a byte
		m_address = command & 0x0f;
		m_shift = m_regs[m_address];
		m_reading = true;
		break;
	case 0x20: // address
		m_address = command & 0x0f;
		break;
	case 0x10: // write data to the selected register
		m_regs[m_address] = command & 0x0f;
		if (m_address == REG_CONTROL)
			m_regs[REG_CONTROL] &= ~2; // inferred: writing acknowledges the oscillator stop flag (XSTP)
		else if (m_address != REG_INTERVAL && m_address != REG_TEST)
			time_written();
		break;
	default:
		// no other commands have been observed in the firmware
		break;
	}
}


class reelmag_state : public driver_device
{
public:
	reelmag_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
		, m_eeprom(*this, "ic%u", 19U)
		, m_hopper(*this, "hopper")
		, m_rtc(*this, "rtc")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_vram(*this, "vram")
		, m_workram(*this, "workram")
		, m_flash(*this, "flash")
		, m_inputs(*this, "INPUTS")
	{
	}

	void reelmag(machine_config &config) ATTR_COLD;

	void init_reelmag() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<m68000_device> m_maincpu;
	required_device<dac_8bit_r2r_twos_complement_device> m_dac;
	required_device_array<eeprom_parallel_2864_device, 2> m_eeprom;
	required_device<ticket_dispenser_device> m_hopper;
	required_device<reelmag_rtc_device> m_rtc;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<u16> m_vram;
	required_shared_ptr<u16> m_workram;
	required_region_ptr<u8> m_flash;

	required_ioport m_inputs;

	tilemap_t *m_tilemap[8]{};

	u16 m_serial_latch = 0;
	u8 m_pic_phase = 0; // 0: idle, 1: receiving challenge, 2: sending response
	u8 m_pic_count = 0;
	u8 m_pic_output = 1;
	u32 m_pic_challenge = 0;
	u32 m_pic_response = 0;
	emu_timer *m_scanline_timer = nullptr;
	u32 m_sound_address = 0;
	bool m_sound_playing = false;

	u16 m_video_regs[16]{};
	u16 m_io_regs[3]{};

	void decode_graphics() ATTR_COLD;
	void decode_sound() ATTR_COLD;

	void sound_w(u16 data);
	u16 sound_status_r();
	TIMER_CALLBACK_MEMBER(sound_tick);

	u16 eeprom_r(address_space &space, offs_t offset);
	void eeprom_w(address_space &space, offs_t offset, u16 data, u16 mem_mask = ~0);

	u16 serial_r();
	void serial_w(u16 data);
	void serial_reset();
	u32 serial_pic_reply(u32 challenge) const;

	TIMER_CALLBACK_MEMBER(scanline_irq);

	u16 video_r(offs_t offset);
	void video_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 io_r(offs_t offset);
	void io_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	template <unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;
};


template <unsigned Layer>
TILE_GET_INFO_MEMBER(reelmag_state::get_tile_info)
{
	u32 const entry = (u32(m_vram[Layer * 0x4000 + tile_index * 2]) << 16) | m_vram[Layer * 0x4000 + tile_index * 2 + 1];
	// Bits 19/20 flip the tile vertically/horizontally.  The odds
	// digits reuse their top tile for the bottom with bit 19 set.
	// Attribute bits 31..25 select one of 128 banks of 256 colors.
	// Font attributes 0x70/0x72/0x74 choose gray/red/green ramps.
	tileinfo.set(0, entry & 0x1ffff, (entry >> 25) & 0x7f, TILE_FLIPXY((entry >> 19) & 3));
}

void reelmag_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(reelmag_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 128);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(reelmag_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 128);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(reelmag_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 128);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(reelmag_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 128);
	m_tilemap[4] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(reelmag_state::get_tile_info<4>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 128);
	m_tilemap[5] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(reelmag_state::get_tile_info<5>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 128);
	m_tilemap[6] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(reelmag_state::get_tile_info<6>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 128);
	m_tilemap[7] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(reelmag_state::get_tile_info<7>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 128);

	for (auto *const tilemap : m_tilemap)
		tilemap->set_transparent_pen(0);
}

u32 reelmag_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	if (!BIT(m_video_regs[0], 0))
		return 0;

	// Layer priority and scroll behavior remain provisional;
	// the boot screen uses map zero with zero scroll.
	for (unsigned layer = 0; layer < 8; ++layer)
	{
		m_tilemap[layer]->set_scrolly(0, m_video_regs[8 + layer] & 0x3ff);
		m_tilemap[layer]->draw(screen, bitmap, cliprect, 0, 0);
	}

	return 0;
}

void reelmag_state::sound_w(u16 data)
{
	// The ROM's sound table at 0x135a8 contains sample starts in 32-byte units.
	// 0xe000 is explicitly used by software to silence the sound hardware.
	m_sound_address = u32(data) << 5;
	m_sound_playing = (data != 0xe000) && (m_sound_address < m_flash.length());
	if (!m_sound_playing)
		m_dac->write(0);
}

TIMER_CALLBACK_MEMBER(reelmag_state::sound_tick)
{
	if (!m_sound_playing)
		return;

	// 0x7f ends a sample.
	u8 const sample = m_flash[m_sound_address];
	if (sample == 0x7f)
	{
		m_sound_playing = false;
		m_dac->write(0);
		return;
	}

	m_dac->write(sample);
	if (++m_sound_address >= m_flash.length())
		m_sound_playing = false;
}

u16 reelmag_state::sound_status_r()
{
	// Bit 0 is active-high BUSY.  The bootstrap derives the bitstream XOR
	// key from the high byte: (0xd0 + 0x8f) & 0xff = 0x5f.
	return 0xd000 | (m_sound_playing ? 1 : 0);
}

// Each EEPROM has its own address line permutation; the data lines of both
// are permuted together across the 16-bit word.
u16 reelmag_state::eeprom_r(address_space &space, offs_t offset)
{
	u16 const raw = (u16(m_eeprom[0]->read(space, bitswap<13>(offset, 7, 10, 0, 9, 8, 3, 4, 2, 6, 5, 1, 11, 12))) << 8)
			| m_eeprom[1]->read(space, bitswap<13>(offset, 4, 5, 1, 6, 2, 3, 7, 8, 9, 10, 0, 12, 11));
	return bitswap<16>(raw, 14, 15, 12, 10, 13, 7, 11, 5, 6, 3, 4, 1, 2, 8, 0, 9);
}

void reelmag_state::eeprom_w(address_space &space, offs_t offset, u16 data, u16 mem_mask)
{
	// the data line permutation crosses the byte lanes, so partial writes need the old contents
	if (mem_mask != 0xffff)
		data = (eeprom_r(space, offset) & ~mem_mask) | (data & mem_mask);
	u16 const raw = bitswap<16>(data, 14, 15, 11, 13, 9, 12, 0, 2, 10, 7, 8, 5, 6, 3, 4, 1);
	m_eeprom[0]->write(bitswap<13>(offset, 7, 10, 0, 9, 8, 3, 4, 2, 6, 5, 1, 11, 12), raw >> 8);
	m_eeprom[1]->write(bitswap<13>(offset, 4, 5, 1, 6, 2, 3, 7, 8, 9, 10, 0, 12, 11), raw & 0xff);
}

u16 reelmag_state::video_r(offs_t offset)
{
	if (offset == 1)
		return (m_video_regs[offset] & ~0x0100) | (m_screen->vpos() & 0x0100);

	// The program samples this twice as the challenge for its serial key interface.
	if (offset == 4)
		return u16(m_maincpu->total_cycles());

	return m_video_regs[offset];
}

void reelmag_state::video_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset == 0 || offset >= 8)
		m_screen->update_partial(m_screen->vpos());
	COMBINE_DATA(&m_video_regs[offset]);
	if (offset == 1)
		m_scanline_timer->adjust(m_screen->time_until_pos(m_video_regs[1] & 0xff));
}

void reelmag_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
	m_tilemap[offset >> 14]->mark_tile_dirty((offset & 0x3fff) >> 1);
}

u16 reelmag_state::io_r(offs_t offset)
{
	if (offset == 1)
		return m_inputs->read();

	if (offset == 2)
		return serial_r();

	return m_io_regs[offset];
}

void reelmag_state::io_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_io_regs[offset]);

	if (offset == 1)
	{
		m_hopper->motor_w(BIT(m_io_regs[1], 9));

		// Four physical meter outputs; individual accounting assignments are unverified.
		for (unsigned coin = 0; coin < 4; ++coin)
			machine().bookkeeping().coin_counter_w(coin, BIT(m_io_regs[1], 15 - coin));
	}

	if (offset == 2)
		serial_w(m_io_regs[offset]);
}


void reelmag_state::mem_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x20001f).rw(FUNC(reelmag_state::video_r), FUNC(reelmag_state::video_w));
	map(0x400000, 0x43ffff).ram().w(FUNC(reelmag_state::vram_w)).share(m_vram);
	map(0x600000, 0x60ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x800000, 0x800001).w(FUNC(reelmag_state::sound_w));
	map(0x800008, 0x800009).r(FUNC(reelmag_state::sound_status_r));
	map(0xa00000, 0xa00005).rw(FUNC(reelmag_state::io_r), FUNC(reelmag_state::io_w));
	map(0xc00000, 0xc03fff).rw(FUNC(reelmag_state::eeprom_r), FUNC(reelmag_state::eeprom_w));
	map(0xe00000, 0xe0ffff).mirror(0x1f0000).ram().share(m_workram); // the firmware uses 0xe00000 and 0xff0000; full decode unverified
}


static INPUT_PORTS_START( reelmag )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet Right")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop Reel 3 / Small")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop Reel 1 / Big")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet Left")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Credit Key") // attendant: key in/out with BIG/SMALL while held
	PORT_SERVICE_NO_TOGGLE( 0x0080, IP_ACTIVE_LOW ) // main menu: bookkeeping, settings, tests
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // boss book
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("JP Hit")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
INPUT_PORTS_END


// Firmware-derived model for the serial protection/key interface.
// The PIC is undumped; this implements the observed host protocol, not its code.
void reelmag_state::serial_reset()
{
	m_serial_latch = 0;
	m_pic_phase = 0;
	m_pic_count = 0;
	m_pic_output = 1;
	m_pic_challenge = 0;
	m_pic_response = 0;
}

u32 reelmag_state::serial_pic_reply(u32 challenge) const
{
	// The ROM decoder at 0xb03c expects [key high, key low, 0xf1, checksum].
	// 0xf1 is checked periodically at 0x3f1a even without an electronic key.
	constexpr u16 key_id = 0; // no electronic key
	std::array<u8, 4> const plain{
		u8(key_id >> 8), u8(key_id), 0xf1,
		u8((key_id >> 8) + (key_id & 0xff) + 0xf1)
	};
	u32 response = 0;
	for (unsigned i = 0; i < 4; ++i)
	{
		u8 const nonce_byte = challenge >> (24 - 8 * i);
		u8 const delta = (i == 3) ? plain[3] : (plain[i] ^ plain[3]);
		response = (response << 8) | u8(nonce_byte + delta);
	}
	return response;
}

u16 reelmag_state::serial_r()
{
	// Bit 0 is electronic key presence, independent of the challenge/response
	// ready signal; no key is fitted.
	return (u16(m_pic_output) << 1) | (u16(m_rtc->sio_r()) << 2);
}

void reelmag_state::serial_w(u16 data)
{
	u16 const previous = m_serial_latch;
	m_serial_latch = data;

	// RTC: active-high CE on bit 4, SCLK on bit 3, inverted outgoing data
	// on bit 2; incoming data on bit 2 of serial_r() is non-inverted.
	m_rtc->sio_w(!BIT(data, 2));
	m_rtc->ce_w(BIT(data, 4));
	m_rtc->sclk_w(BIT(data, 3));

	if (BIT(data, 4))
		return;

	if (BIT(previous, 4))
		return; // RTC deselection must not begin a PIC transaction.

	if (m_pic_phase == 0)
	{
		if (BIT(data, 0))
		{
			m_pic_phase = 1;
			m_pic_count = 0;
			m_pic_challenge = 0;
			m_pic_output = 1; // Ready, polled before sending the nonce.
		}
		return;
	}

	if (BIT(previous, 0) && !BIT(data, 0))
	{
		if (m_pic_phase == 1)
		{
			// Challenge is sent least-significant bit first.
			m_pic_challenge |= u32(BIT(data, 1)) << m_pic_count;

			if (++m_pic_count == 32)
			{
				m_pic_response = serial_pic_reply(m_pic_challenge);
				m_pic_phase = 2;
				m_pic_count = 0;
			}
		}
		else
		{
			// Response is read most-significant bit first after each clock.
			m_pic_output = BIT(m_pic_response, 31 - m_pic_count);

			if (++m_pic_count == 32)
				m_pic_phase = 0;
		}
	}
}

TIMER_CALLBACK_MEMBER(reelmag_state::scanline_irq)
{
	if (BIT(m_video_regs[0], 1)) // inferred raster interrupt enable
		m_maincpu->set_input_line(1, HOLD_LINE);

	m_scanline_timer->adjust(m_screen->frame_period());
}

void reelmag_state::machine_start()
{
	save_item(NAME(m_video_regs));
	save_item(NAME(m_io_regs));
	save_item(NAME(m_serial_latch));
	save_item(NAME(m_pic_phase));
	save_item(NAME(m_pic_count));
	save_item(NAME(m_pic_output));
	save_item(NAME(m_pic_challenge));
	save_item(NAME(m_pic_response));
	save_item(NAME(m_sound_address));
	save_item(NAME(m_sound_playing));

	m_scanline_timer = timer_alloc(FUNC(reelmag_state::scanline_irq), this);

	// A 12-bit divider of the board oscillator fits the musical sample pitches.
	// Verify the divider on real hardware.
	attotime const sample_period = attotime::from_hz(58_MHz_XTAL / 4096);
	timer_alloc(FUNC(reelmag_state::sound_tick), this)->adjust(sample_period, 0, sample_period);
}


void reelmag_state::machine_reset()
{
	serial_reset();

	m_scanline_timer->adjust(m_screen->time_until_pos(0xc0)); // provisional raster IRQ line until video register 1 is written

	std::fill(std::begin(m_video_regs), std::end(m_video_regs), 0);
	std::fill(std::begin(m_io_regs), std::end(m_io_regs), 0);

	m_hopper->motor_w(0);

	for (unsigned coin = 0; coin < 4; coin++)
		machine().bookkeeping().coin_counter_w(coin, 0);

	m_sound_playing = false;
	m_dac->write(0);
}


static GFXDECODE_START( gfx_reelmag )
	GFXDECODE_ENTRY( "reels", 0, gfx_8x8x8_raw, 0, 128 )
GFXDECODE_END


void reelmag_state::reelmag(machine_config &config)
{
	M68000(config, m_maincpu, 58_MHz_XTAL / 12); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &reelmag_state::mem_map);

	PIC16C621A(config, "pic", 58_MHz_XTAL / 12).set_disable(); // undumped

	HOPPER(config, m_hopper, attotime::from_msec(100)); // mechanical rate unverified

	EEPROM_2864(config, m_eeprom[0]);
	EEPROM_2864(config, m_eeprom[1]);

	NVRAM(config, "workram", nvram_device::DEFAULT_ALL_0);

	REELMAG_RTC(config, m_rtc);

	// Provisional raster timing until measured on the board.
	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(50); // firmware counts 50 diagnostic IRQs per second
	m_screen->set_size(512, 312);
	m_screen->set_visarea(0, 327, 0, 255); // the game draws 41 columns of 8-pixel tiles
	m_screen->set_screen_update(FUNC(reelmag_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_reelmag);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x8000);


	// sound hardware
	SPEAKER(config, "mono").front_center();

	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "mono", 0.5); // DAC type unknown
}


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( reelmag )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "m27c800.ic18",  0x000000, 0x100000, CRC(2af3d8e7) SHA1(729cd2c1011d8018cf8d77c2d118d1815e30f475) )

	ROM_REGION16_LE( 0x4010, "pic", ROMREGION_ERASEFF )
	ROM_LOAD( "pic16c621.bin", 0x0000, 0x4010, NO_DUMP ) // read protected

	ROM_REGION( 0x800000, "reels", 0 )
	ROM_LOAD( "m27c160.ic3",  0x000000, 0x200000, CRC(707a835a) SHA1(4edbb2279298f330514512147166b9382c79861d) )
	ROM_LOAD( "m27c160.ic4",  0x200000, 0x200000, CRC(d5590a3c) SHA1(69dacb370b630fd7fee3ddd4beeb34a336dd2d16) )
	ROM_LOAD( "m27c160.ic5",  0x400000, 0x200000, CRC(72da9809) SHA1(19516432c4cfc33c3db20aab0c64fafb72ed1a19) )
	ROM_LOAD( "m27c160.ic6",  0x600000, 0x200000, CRC(0f3274d0) SHA1(1abb45ebc74a09f1832cf80775a35966e8d5cd84) )

	ROM_REGION( 0x200000, "flash", 0 )
	ROM_LOAD( "mx29f161.ic24",0x000000, 0x200000, CRC(b479c2db) SHA1(cac4c38ef26d307bfd3ffff77859ea90cf554418) )

	ROM_REGION( 0x2000, "ic19", 0 )
	ROM_LOAD( "m28c64.ic19",  0x000000, 0x002000, CRC(d0238e5c) SHA1(513bb97487d33c3b844877104bb2af3220851583) )

	ROM_REGION( 0x2000, "ic20", 0 )
	ROM_LOAD( "m28c64.ic20",  0x000000, 0x002000, CRC(4e6abd42) SHA1(5b1741b755f0fddd94e16d41d5d39a03f37fb23b) )

	ROM_REGION16_BE( 0x10000, "workram", 0 ) // after CLEAR ALL DATA
	ROM_LOAD( "workram", 0x00000, 0x10000, CRC(3879270f) SHA1(10d591dd6e00f447421af49b3690654a5fe8cd80) )
ROM_END


void reelmag_state::decode_graphics()
{
	// Decoded in place to 8x8 tiles with one byte per pixel.
	memory_region *const region = memregion("reels");
	u8 *const rom = region->base();
	std::vector<u8> const buffer(rom, rom + region->bytes());

	// Logical bit 0 is tile row bit 0. The source is a WORD address in each ROM.
	static constexpr unsigned ADDRESS_LINE[20] = {
		3, 11, 6, 13, 1, 12, 5, 9, 10, 4, 14, 8, 7, 0, 2, 15, 16, 17, 18, 19
	};
	for (u32 row = 0; row < 0x100000; ++row)
	{
		u32 source = 0;
		for (unsigned bit = 0; bit < 20; ++bit)
			source |= BIT(row, bit) << ADDRESS_LINE[bit];

		for (unsigned x = 0; x < 8; ++x)
		{
			u8 pixel = 0;
			for (unsigned chip = 0; chip < 4; ++chip)
			{
				u32 const offset = chip * 0x200000 + source * 2;
				pixel |= BIT(buffer[offset] ^ 0x92, 7 - x) << (chip * 2);
				pixel |= BIT(buffer[offset + 1] ^ 0x24, 7 - x) << (chip * 2 + 1);
			}
			rom[row * 8 + x] = pixel;
		}
	}
}

void reelmag_state::decode_sound()
{
	// Decoded in place to signed 8-bit samples.
	std::vector<u8> const buffer(m_flash.target(), m_flash.target() + m_flash.length());
	for (u32 i = 0; i < m_flash.length(); ++i)
	{
		u32 const address = bitswap<21>(i, 20, 19, 18, 17, 16, 11, 4, 6, 2, 9, 8, 12, 13, 3, 7, 10, 1, 15, 5, 14, 0);
		m_flash[i] = buffer[address] ^ ((i & 1) ? 0xa4 : 0x12);
	}
}

void reelmag_state::init_reelmag()
{
	decode_graphics();
	decode_sound();

	// After descrambling, the sum of all 16-bit words is zero (checked by the program at 0x001f88).
	u16 *const rom = reinterpret_cast<u16 *>(memregion("maincpu")->base());
	std::vector<u16> const buffer(rom, rom + 0x100000 / 2);

	for (u32 i = 0; i < buffer.size(); i++)
	{
		u32 const address = bitswap<19>(i, 18, 16, 14, 17, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 12, 15);
		rom[i] = bitswap<16>(buffer[address], 8, 7, 0, 9, 15, 5, 6, 13, 10, 4, 2, 12, 11, 1, 3, 14);
	}

}

} // anonymous namespace


GAME( 199?, reelmag, 0, reelmag, reelmag, reelmag_state, init_reelmag, ROT0, "Play System (Impera license)", "Reel Magic (Impera, INT v1.0-IMP)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
