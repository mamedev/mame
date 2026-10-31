// license:BSD-3-Clause
// copyright-holders:MikeDX
/*******************************************************************************

Premium Poker (c) 1993 Taiyo Jidoki (TAIYO Co., Ltd.)

Five card draw poker with holds, a BIG/SMALL double up and a coin hopper.
Program ROMs are labelled P221 and the title string reads "Version 2.21";
the graphics ROMs are labelled V211.

PCB parts (from photos):
- Sharp LH0080E Z80E-CPU, 8.000 MHz crystal beside it
- 2x Fujitsu MB89363B (each one is a pair of 8255-type PPIs)
- 64-pin custom DIP "SPECIAL VI JAPAN & US" (video), 2148-type RAMs
- Small square QFP with a green program label: unidentified, probably the
  tone generator and/or the security/accounting device
- 24.000 MHz oscillator (use unknown, probably video)
- AMI 18CV8 PAL, 3 banks of 8 DIP switches, UDN2803A driver, battery
- 8 KB battery backed SRAM

The program uses no Z80 I/O ports; everything is memory mapped.

Memory map:
0000-5fff  pp3 (the top 8 KB of the 32 KB EPROM is blank and unmapped)
6000-7fff  battery backed RAM (the game keeps triple copies at +1000/+1800)
8000-9fff  text layer: 64x32 cells of 4 bytes (tile lo, tile hi, attr, unused)
a000-b7ff  card layer: three planes of 64x8 cells, same cell format
b800-b9ff  text palette, 256 x 12-bit (byte 0 = R<<4|G, byte 1 = B)
ba00-bbff  card palette
bc00-bc5f  card layer column scroll, 32 x 16 px columns per plane
be00-be63  PPIs (see below)
bec0       security device data (R)
bf40-bf7f  tone generator (W)
bf80       IRQ acknowledge (and probably a watchdog kick) (W)
bfa0       bit 0: pp4 bank (W)
bfc0       security device command (W)
c000-ffff  pp4, banked in 16 KB halves

PPI ports:
be00-be02  PPI0 A/B/C: multiplexed button matrix, row picked by be22
be20       PPI1 A: direct inputs (hopper, coin B, service, payout)
be21       PPI1 B: HOLD lamps
be22       PPI1 C: matrix row select (one-hot 01/02/04/08)
be40-be42  PPI2 A/B/C: DIP SW1, SW3, SW2 (note the order)
be60-be62  PPI3 A/B/C: button lamps, misc outputs, meters and hopper motor

The game boots to RAM ERROR on a blank NVRAM. Hold MEMORY CLEAR (F2) while
the machine starts to clear and initialise the battery RAM.

TODO / unverified:
- CPU clock: 8 MHz is assumed. Nothing in the code proves it, but at 4 MHz
  the hopper coin pulse filter rejects pulses shorter than about 40 ms.
- IRQ source and position in the frame, screen timings.
- Card plane select by prom2 per scanline is inferred, not traced on a PCB.
- prom1 role.
- Flip screen direction.
- Tone generator: identity, octave, wave register bits, level curve, noise.
- Security device: undumped MCU, high level emulated. The 'U' command and
  register 0C are not understood.
- BOOK 1 shows a DOOR OPEN counter, but no door switch input was found.

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


/*******************************************************************************

  Tone generator (unidentified chip), high level model, unverified

  Two identical 32-byte register blocks (bf40 and bf60). Each block has four
  tone voices at +00, +05, +0a, +0f and one more voice at +13 that the game
  never gives a wave register, so it is taken to be noise.

  +0/+1  16-bit divider N, low byte first. The game's note table is built as
         N = 2 MHz / f (A4 = 440 Hz is N = 4545), so f = clock / N with a
         2 MHz clock. The octave is not proven.
  +2     level, written bit reversed by the game (it keeps 0-3f internally)
  +3     "wave": the game only writes 0 (key off), 8 (normal tone) and 0c
         (two effects). Modelled as bit 3 = enable, bit 2 = narrower pulse.
  +1e/+1f written once at reset with e0/40, meaning unknown.

*******************************************************************************/

class taiyo_tone_device : public device_t, public device_sound_interface
{
public:
	taiyo_tone_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void write(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr unsigned CLOCK_DIVIDER = 16;
	static constexpr u8 VOICE_BASE[5] = { 0x00, 0x05, 0x0a, 0x0f, 0x13 };

	sound_stream *m_stream;
	u8 m_regs[0x40];
	u32 m_count[10];
	u16 m_lfsr[2];
};

DEFINE_DEVICE_TYPE(TAIYO_TONE, taiyo_tone_device, "taiyo_tone", "Taiyo tone generator (HLE)")

taiyo_tone_device::taiyo_tone_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TAIYO_TONE, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
{
}

void taiyo_tone_device::device_start()
{
	m_stream = stream_alloc(0, 1, clock() / CLOCK_DIVIDER);

	save_item(NAME(m_regs));
	save_item(NAME(m_count));
	save_item(NAME(m_lfsr));
}

void taiyo_tone_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill(std::begin(m_count), std::end(m_count), 0);
	m_lfsr[0] = m_lfsr[1] = 1;
}

void taiyo_tone_device::write(offs_t offset, u8 data)
{
	m_stream->update();
	m_regs[offset & 0x3f] = data;
}

void taiyo_tone_device::sound_stream_update(sound_stream &stream)
{
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		float sample = 0.0f;

		for (int block = 0; block < 2; block++)
		{
			u8 const *const regs = &m_regs[block * 0x20];

			for (int v = 0; v < 5; v++)
			{
				u8 const base = VOICE_BASE[v];
				u32 &count = m_count[block * 5 + v];
				u32 const n = regs[base] | (regs[base + 1] << 8);
				u8 const level = bitswap<8>(regs[base + 2], 0, 1, 2, 3, 4, 5, 6, 7);
				bool const noise = (v == 4);
				bool const enabled = noise || BIT(regs[base + 3], 3);

				if (!n || !level || !enabled)
					continue;

				bool high;
				count += CLOCK_DIVIDER;
				if (noise)
				{
					// 15-bit LFSR stepped at clock / N (a guess)
					u16 &lfsr = m_lfsr[block];
					while (count >= n)
					{
						count -= n;
						lfsr = (lfsr >> 1) | ((BIT(lfsr, 0) ^ BIT(lfsr, 1)) << 14);
					}
					high = BIT(lfsr, 0);
				}
				else
				{
					// square wave, or a 25% pulse for wave code 0c (a guess)
					count %= n;
					u32 const duty = BIT(regs[base + 3], 2) ? (n / 4) : (n / 2);
					high = count < duty;
				}

				sample += (high ? 1.0f : -1.0f) * (float(level) / 63.0f) * 0.1f;
			}
		}

		stream.put(0, sampindex, sample);
	}
}


/*******************************************************************************

  Security / accounting device, high level emulation

  The real part is an undumped microcontroller (probably the labelled QFP).
  The CPU writes ASCII commands one byte at a time to bfc0; the device
  answers one byte per NMI, which the CPU reads from bec0 (the read clears
  the NMI). It keeps thirteen 4-byte BCD registers, most significant byte
  first. The game restores registers 00-0b from battery RAM at every boot,
  so the device needs no saved state of its own.

  '?'          reply with the 65-byte copyright string (checked at boot,
               DATA ERROR on mismatch)
  'L' n        reply with the 4 bytes of register n
  'S' n b0-b3  set register n
  'I' n        BCD increment register n
  'C' n        clear register n
  'U' 0c 00 07 meaning unknown (perhaps 0c = f(00-07)); ignored

*******************************************************************************/

class taiyo_sec_hle_device : public device_t
{
public:
	taiyo_sec_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto nmi_handler() { return m_nmi_cb.bind(); }

	u8 data_r();
	void cmd_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr unsigned NUM_REGS = 13;

	TIMER_CALLBACK_MEMBER(send_byte);
	void execute();
	void queue_reply(u8 data);

	devcb_write_line m_nmi_cb;
	emu_timer *m_send_timer;

	u8 m_regs[NUM_REGS * 4];
	u8 m_cmd[8];
	u8 m_cmd_len;
	u8 m_reply[128];
	u8 m_reply_head;
	u8 m_reply_count;
	u8 m_data;
	u8 m_nmi;
};

DEFINE_DEVICE_TYPE(TAIYO_SEC_HLE, taiyo_sec_hle_device, "taiyo_sec_hle", "Taiyo security device (HLE)")

taiyo_sec_hle_device::taiyo_sec_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TAIYO_SEC_HLE, tag, owner, clock)
	, m_nmi_cb(*this)
	, m_send_timer(nullptr)
{
}

void taiyo_sec_hle_device::device_start()
{
	m_send_timer = timer_alloc(FUNC(taiyo_sec_hle_device::send_byte), this);

	save_item(NAME(m_regs));
	save_item(NAME(m_cmd));
	save_item(NAME(m_cmd_len));
	save_item(NAME(m_reply));
	save_item(NAME(m_reply_head));
	save_item(NAME(m_reply_count));
	save_item(NAME(m_data));
	save_item(NAME(m_nmi));
}

void taiyo_sec_hle_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_cmd_len = 0;
	m_reply_head = 0;
	m_reply_count = 0;
	m_data = 0;
	m_nmi = 0;
	m_nmi_cb(CLEAR_LINE);
	m_send_timer->adjust(attotime::never);
}

void taiyo_sec_hle_device::queue_reply(u8 data)
{
	if (m_reply_count < std::size(m_reply))
	{
		m_reply[(m_reply_head + m_reply_count) % std::size(m_reply)] = data;
		m_reply_count++;
	}
}

TIMER_CALLBACK_MEMBER(taiyo_sec_hle_device::send_byte)
{
	if (m_nmi || !m_reply_count)
		return;

	m_data = m_reply[m_reply_head];
	m_reply_head = (m_reply_head + 1) % std::size(m_reply);
	m_reply_count--;
	m_nmi = 1;
	m_nmi_cb(ASSERT_LINE);
}

u8 taiyo_sec_hle_device::data_r()
{
	if (!machine().side_effects_disabled() && m_nmi)
	{
		m_nmi = 0;
		m_nmi_cb(CLEAR_LINE);

		// the NMI handler is not re-entrant, so leave it time to finish (spacing is a guess)
		if (m_reply_count)
			m_send_timer->adjust(attotime::from_usec(50));
	}
	return m_data;
}

void taiyo_sec_hle_device::cmd_w(u8 data)
{
	m_cmd[m_cmd_len++] = data;

	unsigned length;
	switch (m_cmd[0])
	{
	case '?': length = 1; break;
	case 'L': case 'I': case 'C': length = 2; break;
	case 'U': length = 4; break;
	case 'S': length = 6; break;
	default:
		logerror("unknown command byte %02x\n", m_cmd[0]);
		m_cmd_len = 0;
		return;
	}

	if (m_cmd_len >= length)
	{
		execute();
		m_cmd_len = 0;
	}
}

void taiyo_sec_hle_device::execute()
{
	// the same string is at 0090 in the program ROM, which compares the reply with it
	static constexpr char COPYRIGHT[] = "Copyright (C) TAIYO Co., Ltd. & A.Ueda  1933\r\nfor AMUSEMENT ONLY";
	static_assert(sizeof(COPYRIGHT) == 65);

	u8 const reg = m_cmd[1];
	bool const reg_valid = reg < NUM_REGS;
	u8 *const r = &m_regs[(reg_valid ? reg : 0) * 4];

	switch (m_cmd[0])
	{
	case '?':
		for (char c : COPYRIGHT)
			queue_reply(c);
		break;

	case 'L':
		for (int i = 0; i < 4; i++)
			queue_reply(reg_valid ? r[i] : 0);
		break;

	case 'S':
		if (reg_valid)
			std::copy_n(&m_cmd[2], 4, r);
		break;

	case 'I':
		if (reg_valid)
		{
			// BCD increment, least significant byte last
			for (int i = 3; i >= 0; i--)
			{
				u8 lo = (r[i] & 0x0f) + 1;
				u8 hi = r[i] >> 4;
				if (lo > 9)
				{
					lo = 0;
					hi++;
				}
				if (hi > 9)
				{
					r[i] = lo;
					continue;
				}
				r[i] = (hi << 4) | lo;
				break;
			}
		}
		break;

	case 'C':
		if (reg_valid)
			std::fill_n(r, 4, 0);
		break;

	case 'U':
		logerror("U %02x %02x %02x ignored\n", m_cmd[1], m_cmd[2], m_cmd[3]);
		break;
	}

	if (!reg_valid && m_cmd[0] != '?' && m_cmd[0] != 'U')
		logerror("%c with bad register %02x\n", m_cmd[0], reg);

	if (m_reply_count && !m_nmi)
		m_send_timer->adjust(attotime::from_usec(100));
}


namespace {

/*******************************************************************************
  Driver
*******************************************************************************/

class prempokr_state : public driver_device
{
public:
	prempokr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_hopper(*this, "hopper")
		, m_rombank(*this, "rombank")
		, m_textram(*this, "textram")
		, m_cardram(*this, "cardram")
		, m_palram(*this, "palram")
		, m_scrollram(*this, "scrollram")
		, m_prom2(*this, "prom2")
		, m_mux_rows(*this, "ROW%u", 1U)
		, m_lamps(*this, "lamp%u", 0U)
	{
	}

	void prempokr(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<hopper_device> m_hopper;
	required_memory_bank m_rombank;
	required_shared_ptr<u8> m_textram;
	required_shared_ptr<u8> m_cardram;
	required_shared_ptr<u8> m_palram;
	required_shared_ptr<u8> m_scrollram;
	required_region_ptr<u8> m_prom2;
	required_ioport_array<4> m_mux_rows;
	output_finder<13> m_lamps; // 0-4 HOLD 1-5, 5 DEAL, 6 BET, 7 CANCEL, 8 PAYOUT, 9 TAKE SCORE, 10 DOUBLE UP, 11 SMALL, 12 BIG

	tilemap_t *m_text_tilemap = nullptr;
	u8 m_mux = 0;
	u8 m_misc = 0;
	u8 m_meters = 0;

	void main_map(address_map &map) ATTR_COLD;

	void vblank_irq(int state);
	void irq_ack_w(u8 data);
	void bank_w(u8 data);

	void textram_w(offs_t offset, u8 data);
	void palette_w(offs_t offset, u8 data);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	template <unsigned Port> u8 mux_r();
	void mux_w(u8 data);
	void hold_lamps_w(u8 data);
	void button_lamps_w(u8 data);
	void misc_w(u8 data);
	void meters_w(u8 data);
	void update_hopper();
};


void prempokr_state::machine_start()
{
	m_rombank->configure_entries(0, 2, memregion("pp4")->base(), 0x4000);

	save_item(NAME(m_mux));
	save_item(NAME(m_misc));
	save_item(NAME(m_meters));
}

void prempokr_state::machine_reset()
{
	m_rombank->set_entry(0);
	m_mux = 0;
	m_misc = 0;
	m_meters = 0;
	update_hopper();
}

void prempokr_state::device_post_load()
{
	m_text_tilemap->mark_all_dirty();
}


/*******************************************************************************
  Video
*******************************************************************************/

TILE_GET_INFO_MEMBER(prempokr_state::get_text_tile_info)
{
	u8 const *const cell = &m_textram[tile_index * 4];
	tileinfo.set(0, (cell[0] | (cell[1] << 8)) & 0x0fff, cell[2] & 0x0f, 0);
}

void prempokr_state::textram_w(offs_t offset, u8 data)
{
	m_textram[offset] = data;
	m_text_tilemap->mark_tile_dirty(offset >> 2);
}

void prempokr_state::palette_w(offs_t offset, u8 data)
{
	// entries 000-0ff text layer (b800), 100-1ff card layer (ba00)
	m_palram[offset] = data;
	offs_t const entry = offset >> 1;
	u8 const rg = m_palram[entry * 2];
	u8 const b = m_palram[entry * 2 + 1];
	m_palette->set_pen_color(entry, pal4bit(rg >> 4), pal4bit(rg & 0x0f), pal4bit(b & 0x0f));
}

void prempokr_state::video_start()
{
	m_text_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(prempokr_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_text_tilemap->set_transparent_pen(0);
}

u32 prempokr_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	// be61 bit 3 is cleared while the game redraws the whole screen: taken to be a display enable
	if (!BIT(m_misc, 3))
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	/*
	   Card layer: three 512x256 planes (a000, a800, b000) of 8x32 tiles. prom2
	   gives, for each scanline, which plane is shown (0-2) or none (3). This is
	   inferred from the ROM contents and how the game places its graphics; it
	   matches every screen the game draws. Each 16-pixel column of a plane has
	   its own vertical scroll, used to slide the cards in.
	*/
	gfx_element *const gfx = m_gfxdecode->gfx(1);
	pen_t const *const pens = m_palette->pens();
	bool const flip = flip_screen();

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u32 *const dest = &bitmap.pix(y);
		int const sy = (flip ? (255 - y) : y) & 0xff;
		u8 const plane = m_prom2[sy] & 0x03;

		if (plane == 3)
		{
			std::fill(&dest[cliprect.min_x], &dest[cliprect.max_x + 1], rgb_t::black());
			continue;
		}

		u8 const *const planeram = &m_cardram[plane * 0x800];
		u8 const *const scroll = &m_scrollram[plane * 0x20];

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int const sx = (flip ? (511 - x) : x) & 0x1ff;
			int const py = (sy + scroll[sx >> 4]) & 0xff;
			u8 const *const cell = &planeram[(py >> 5) * 0x100 + (sx >> 3) * 4];
			u32 const code = (cell[0] | (cell[1] << 8)) % gfx->elements();
			u8 const pix = gfx->get_data(code)[(py & 0x1f) * gfx->rowbytes() + (sx & 7)];
			dest[x] = pens[0x100 + (cell[2] & 0x0f) * 16 + pix];
		}
	}

	m_text_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/*******************************************************************************
  Interrupts, banking
*******************************************************************************/

void prempokr_state::vblank_irq(int state)
{
	// ~60 Hz, assumed to be vblank. The game re-enables interrupts inside its
	// handler and waits for the next one there, so IRQs must be able to nest.
	if (state)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}

void prempokr_state::irq_ack_w(u8 data)
{
	// written at IRQ entry and inside every long loop: IRQ acknowledge, probably also a watchdog kick
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

void prempokr_state::bank_w(u8 data)
{
	m_rombank->set_entry(BIT(data, 0));
}


/*******************************************************************************
  Inputs and outputs
*******************************************************************************/

template <unsigned Port>
u8 prempokr_state::mux_r()
{
	// rows are active low, so selecting several rows ANDs them
	u8 data = 0xff;
	for (unsigned row = 0; row < 4; row++)
	{
		if (BIT(m_mux, row))
			data &= m_mux_rows[row]->read() >> (Port * 8);
	}
	return data;
}

void prempokr_state::mux_w(u8 data)
{
	m_mux = data;
}

void prempokr_state::hold_lamps_w(u8 data)
{
	// bits 0-4 = HOLD 5-1; bits 5-7 are only set by the "all lamps on" test
	for (int i = 0; i < 5; i++)
		m_lamps[i] = BIT(data, 4 - i);
}

void prempokr_state::button_lamps_w(u8 data)
{
	// DEAL, BET, CANCEL, PAYOUT, TAKE SCORE, DOUBLE UP, SMALL, BIG
	for (int i = 0; i < 8; i++)
		m_lamps[5 + i] = BIT(data, i);
}

void prempokr_state::misc_w(u8 data)
{
	// bit 0: hopper motor (with be62 bit 7)
	// bit 2: screen flip (DIP SW2-8)
	// bit 3: display enable (probable)
	// bit 4: COIN A acceptor enable
	// bit 5: COIN A meter
	// bit 6: COIN B acceptor enable
	m_misc = data;
	update_hopper();
	flip_screen_set(BIT(data, 2));
	machine().bookkeeping().coin_lockout_w(0, !BIT(data, 4));
	machine().bookkeeping().coin_lockout_w(1, !BIT(data, 6));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 5));
}

void prempokr_state::meters_w(u8 data)
{
	// bits 1-6: COIN B, SERVICE IN, KEY IN, KEY OUT, HOPPER OUT, EMPTY OUT meters
	// bit 7: hopper motor (with be61 bit 0)
	m_meters = data;
	update_hopper();
	for (int i = 1; i <= 6; i++)
		machine().bookkeeping().coin_counter_w(i, BIT(data, i));
}

void prempokr_state::update_hopper()
{
	// the two motor lines are always switched together; which is run and which
	// is enable or brake is unknown
	m_hopper->motor_w(BIT(m_misc, 0) && BIT(m_meters, 7));
}


/*******************************************************************************
  Address map
*******************************************************************************/

void prempokr_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().region("maincpu", 0);
	map(0x6000, 0x7fff).ram().share("nvram");
	map(0x8000, 0x9fff).ram().w(FUNC(prempokr_state::textram_w)).share(m_textram);
	map(0xa000, 0xb7ff).ram().share(m_cardram);
	map(0xb800, 0xbbff).ram().w(FUNC(prempokr_state::palette_w)).share(m_palram);
	map(0xbc00, 0xbc5f).ram().share(m_scrollram);
	map(0xbe00, 0xbe03).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xbe20, 0xbe23).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xbe40, 0xbe43).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xbe60, 0xbe63).rw("ppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xbec0, 0xbec0).r("security", FUNC(taiyo_sec_hle_device::data_r));
	map(0xbf40, 0xbf7f).w("tone", FUNC(taiyo_tone_device::write));
	map(0xbf80, 0xbf80).w(FUNC(prempokr_state::irq_ack_w));
	map(0xbfa0, 0xbfa0).w(FUNC(prempokr_state::bank_w));
	map(0xbfc0, 0xbfc0).w("security", FUNC(taiyo_sec_hle_device::cmd_w));
	map(0xc000, 0xffff).bankr(m_rombank);
}


/*******************************************************************************
  Input ports

  The matrix rows are 24 bits each: bits 0-7 read at be00, 8-15 at be01,
  16-23 at be02. Names are the game's own, from its input test page.
*******************************************************************************/

static INPUT_PORTS_START( prempokr )
	PORT_START("ROW1")
	PORT_BIT( 0x000200, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Key In")
	PORT_BIT( 0x001000, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Coin A")
	PORT_BIT( 0x008000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test")
	PORT_BIT( 0x020000, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_NAME("Memory Clear")
	PORT_BIT( 0xfd6dff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW2")
	PORT_BIT( 0x000001, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small")
	PORT_BIT( 0x000004, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")
	PORT_BIT( 0x000008, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out")
	PORT_BIT( 0x000020, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0xffffd2, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW3")
	PORT_BIT( 0x000040, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1")
	PORT_BIT( 0x000002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2")
	PORT_BIT( 0x000010, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3")
	PORT_BIT( 0x000080, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4")
	PORT_BIT( 0x000100, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5")
	PORT_BIT( 0xfffe2d, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ROW4")
	PORT_BIT( 0x000200, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Double Up")
	PORT_BIT( 0x000800, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw")
	PORT_BIT( 0x002000, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel")
	PORT_BIT( 0x004000, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Books")
	PORT_BIT( 0x010000, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Raise")
	PORT_BIT( 0xfe95ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIRECT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service In")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Empty") PORT_TOGGLE PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Motor On") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Payout")

	// switch n is bit 8-n, ON reads 0; the factory setting is all OFF
	PORT_START("SW1")
	PORT_DIPNAME( 0xe0, 0xe0, "Main Game %" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, "72%" )
	PORT_DIPSETTING(    0x80, "74%" )
	PORT_DIPSETTING(    0x40, "76%" )
	PORT_DIPSETTING(    0xc0, "78%" )
	PORT_DIPSETTING(    0x20, "80%" )
	PORT_DIPSETTING(    0xa0, "82%" )
	PORT_DIPSETTING(    0x60, "84%" )
	PORT_DIPSETTING(    0xe0, "86%" )
	PORT_DIPNAME( 0x10, 0x10, "Auto Hold" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Minimum Bet" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x02, "Double Up Game" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x01, "Winning Points" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, "To Credit" )
	PORT_DIPSETTING(    0x00, "By Hopper" )

	PORT_START("SW2")
	PORT_DIPNAME( 0xf0, 0xf0, "Coin A" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x80, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_10C ) )
	PORT_DIPNAME( 0x0e, 0x0e, "Coin B (Key In = 10x)" ) PORT_DIPLOCATION("SW2:5,6,7")
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_20C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_25C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_50C ) )
	PORT_DIPNAME( 0x01, 0x01, "Screen Flip-Flop" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW3")
	PORT_DIPNAME( 0xc0, 0xc0, "Credit Limit" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0xc0, "20000" )
	PORT_DIPSETTING(    0x40, "30000" )
	PORT_DIPSETTING(    0x80, "50000" )
	PORT_DIPSETTING(    0x00, "No Limit" )
	PORT_DIPNAME( 0x20, 0x20, "At Credit Limit" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x20, "Game Stops on Maximum Limit" )
	PORT_DIPSETTING(    0x00, "Count Above Limit for Last Game Won" )
	PORT_DIPNAME( 0x18, 0x18, "Maximum Bet" ) PORT_DIPLOCATION("SW3:4,5")
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x10, "15" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x04, 0x04, "Hold Sw." ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x04, "Hold or Cancel" )
	PORT_DIPSETTING(    0x00, "Hold Only" )
	PORT_DIPNAME( 0x02, 0x02, "Coin A Mode" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x02, "Credit Mode" )
	PORT_DIPSETTING(    0x00, "Bet Mode" )
	PORT_DIPNAME( 0x01, 0x01, "Message" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
INPUT_PORTS_END


/*******************************************************************************
  Graphics
*******************************************************************************/

static const gfx_layout cards_layout =
{
	8, 32,
	RGN_FRAC(1, 1),
	4,
	{ STEP4(0, 1) },
	{ STEP8(0, 4) },
	{ STEP32(0, 8 * 4) },
	8 * 32 * 4
};

static GFXDECODE_START( gfx_prempokr )
	GFXDECODE_ENTRY( "text",  0, gfx_8x8x4_packed_msb, 0x000, 16 )
	GFXDECODE_ENTRY( "cards", 0, cards_layout,         0x100, 16 )
GFXDECODE_END


/*******************************************************************************
  Machine config
*******************************************************************************/

void prempokr_state::prempokr(machine_config &config)
{
	// 8.000 MHz crystal next to a Z80E. The clock is unproven: 8 MHz is chosen
	// because at 4 MHz the hopper coin filter needs pulses over about 40 ms.
	Z80(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &prempokr_state::main_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// two MB89363B, each one a pair of 8255s; which pair is in which package is unknown
	i8255_device &ppi0(I8255(config, "ppi0"));
	ppi0.in_pa_callback().set(FUNC(prempokr_state::mux_r<0>));
	ppi0.in_pb_callback().set(FUNC(prempokr_state::mux_r<1>));
	ppi0.in_pc_callback().set(FUNC(prempokr_state::mux_r<2>));

	i8255_device &ppi1(I8255(config, "ppi1"));
	ppi1.in_pa_callback().set_ioport("DIRECT");
	ppi1.out_pb_callback().set(FUNC(prempokr_state::hold_lamps_w));
	ppi1.out_pc_callback().set(FUNC(prempokr_state::mux_w));

	i8255_device &ppi2(I8255(config, "ppi2"));
	ppi2.in_pa_callback().set_ioport("SW1");
	ppi2.in_pb_callback().set_ioport("SW3");
	ppi2.in_pc_callback().set_ioport("SW2");

	i8255_device &ppi3(I8255(config, "ppi3"));
	ppi3.out_pa_callback().set(FUNC(prempokr_state::button_lamps_w));
	ppi3.out_pb_callback().set(FUNC(prempokr_state::misc_w));
	ppi3.out_pc_callback().set(FUNC(prempokr_state::meters_w));

	// sensor pulse must be at least 20 polls (about 20 ms at 8 MHz); 60 ms on, 60 ms off
	HOPPER(config, m_hopper, attotime::from_msec(60));

	TAIYO_SEC_HLE(config, "security").nmi_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// video: timings are a guess (24 MHz oscillator / 2, 768 x 262); the game
	// draws only in lines 16-239 of its 256-line planes
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(24_MHz_XTAL / 2, 768, 0, 512, 262, 16, 240);
	screen.set_screen_update(FUNC(prempokr_state::screen_update));
	screen.screen_vblank().set(FUNC(prempokr_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_prempokr);
	PALETTE(config, m_palette).set_entries(0x200);

	SPEAKER(config, "mono").front_center();
	TAIYO_TONE(config, "tone", 8_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*******************************************************************************
  ROM definitions
*******************************************************************************/

ROM_START( prempokr )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "pp3_p221.j15", 0x0000, 0x8000, CRC(328a5eea) SHA1(b3b68f57504600fc94dc46c341fb3b018f02a710) ) // 6000-7fff blank

	ROM_REGION( 0x8000, "pp4", 0 ) // data, palettes and the TEST / BOOKS code, banked at c000
	ROM_LOAD( "pp4_p221.j17", 0x0000, 0x8000, CRC(0972a97e) SHA1(ae8b1788f9d42c2a65c6df4840724f3dccf8a1fc) )

	ROM_REGION( 0x20000, "text", 0 )
	ROM_LOAD( "pp2_v211.k12", 0x00000, 0x20000, CRC(9c2e26a3) SHA1(91ce8ec24686515ee6f4b1d5f338a6fb6472ce8a) )

	ROM_REGION( 0x20000, "cards", 0 )
	ROM_LOAD( "pp1_v211.e15", 0x00000, 0x20000, CRC(e4104333) SHA1(03cfa4a80f3b08a320849315b3d2a8de9e415701) )

	ROM_REGION( 0x800, "prom1", 0 ) // only 4 non-zero values, repeated: probably sync/blank timing, unused here
	ROM_LOAD( "prom1.129", 0x000, 0x800, CRC(9b61c4ff) SHA1(dfe4ad100784d3fb2e1f14a3e8fe2c3a61a118d2) )

	ROM_REGION( 0x800, "prom2", 0 ) // card plane select per scanline (inferred), 256 bytes repeated 8 times
	ROM_LOAD( "prom2.129", 0x000, 0x800, CRC(09962efa) SHA1(dd0a6e05af2b20a5fb0c324a3e011e9ac79a125f) )
ROM_END

} // anonymous namespace


GAME( 1993, prempokr, 0, prempokr, prempokr, prempokr_state, empty_init, ROT0, "Taiyo Jidoki", "Premium Poker (V2.21)", MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
