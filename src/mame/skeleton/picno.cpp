// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************************************************

Konami Picno and Picno 2

Drawing tablet for children: a pen on a resistive tablet, video output to a TV.
Two cartridge slots: slot 2 takes the software cartridges (each with a printed overlay sheet),
slot 1 takes a memory card with a flash chip for saving pictures.

Information provided by Team Europe.

Chips: HD6435328F10 (H8/532 CPU with inbuilt ROM), HN62334BP (27c040 ROM), Konami custom chip 054715 (rectangular 100 pins),
       HM538121JP-10 (128K x 8 VRAM), M514256B-70J (256K x 4 DRAM), OKI M6585.
Crystals: D200L2 (Y1) and D214A3 (Y2), frequencies unknown.

The hardware of the Picno 1 and Picno 2 is completely the same. The Picno 1 has an Audio-Line-Out, which the Picno 2 does not have.

Maskrom of PICNO 1: RX001-Z8-V3J
Maskrom of PICNO 2: RX001-Z8-V4J

The CPU runs in mode 4 (maximum mode, internal ROM enabled). The internal ROM holds the BIOS
(jump table at 0x200); the mask ROM holds the built-in paint program, graphics and sound samples.

Memory map (everything below was derived from the BIOS and the cartridges):

  0x00000-0x07fff  internal ROM
  0x08000-0x09fff  8K window into VRAM, selected by E000
  0x0a000-0x0bfff  8K window into VRAM, selected by E001
                   (the BIOS keeps its variables in the invisible lines of VRAM through these windows)
  0x0e000-0x0e0ff  054715 registers
  0x0e400-0x0e5ff  VRAM row transfer port
  0x0e800-0x0e9ff  pen cursor pattern, 32x32 pixels, 4bpp
  0x0ea00-0x0ea1f  pen cursor palette, 16 colours xRGB555
  0x0ec00-0x0edff  bitmap palette, 256 colours xRGB555
  0x0ee00-0x0efff  RAM (used as scratch by some cartridges)
  0x0fb80-0x0ffff  internal RAM and I/O
  0x20000-0x2ffff  64K window into a cartridge slot, selected by E002
  0x30000-0x3ffff  64K window into a cartridge slot, selected by E003
  0x40000-0x5ffff  DRAM, work buffer for graphics
  0x60000-0x7ffff  VRAM, two 256x256 8bpp pages (0x60000 and 0x70000)
  0x80000-0xfffff  mask ROM

054715 registers:

  E000-E001  VRAM window banks: bit 7 = enable, bits 3-0 = 8K bank
  E002-E003  cartridge window banks: bits 7-4 = slot (1 = software cartridge, 2 = memory card),
             bits 3-0 = 64K page
  E010       bit 0: ADPCM playback start (write 1) / stop (write 0), reads back as busy
  E011       bit 0: IRQ1 enable for the end of a sample, cleared by the IRQ handler
  E012       bit 0: ADPCM source, 0 = mask ROM, 1 = cartridge
  E013-E015  ADPCM start address (20 bits)
  E016-E017  ADPCM length in bytes
  E020       bit 2: pen cursor enable (bits 0-1 are always set by the software)
  E022-E023  display start address bits 23-8 (only the 128K VRAM is decoded; E023 is always 0)
  E024       bit 0: vblank IRQ0 enable, reading acknowledges the IRQ
  E025       bit 4: vblank status
  E030       bit 0: cleared by the BIOS while it uses the row transfer port
  E040-E041  pen cursor X, Y
  E042-E043  pen cursor hotspot X, Y

VRAM row transfer port: while E030 bit 0 is clear, reading 0xe400 + n loads VRAM row n
(256 bytes) into the HM538121's serial register and writing 0xe400 + n stores the serial
register into row n. The BIOS uses it during vblank to clear screens (from a blank row) and to
copy lines.

Sound: the 054715 fetches 4-bit ADPCM (high nibble first) and feeds the M6585. Port 7 bits 4-5
select the M6585 sampling rate.

Memory card: the software uses the original Intel 28F0x0 algorithms: program (0x40, data,
0xc0 verify), chip erase (0x20 0x20, then 0xa0 erase verify per byte), read (0x00), identifier
(0x90). Two checks exist:
- saving and loading pictures (BIOS, used by all cartridges) accepts 1 Mbit chips only:
  0x89 0xb4 (Intel 28F010), 0x07 0x99, 0x1c 0xd0, 0x40 0x8f; anything else gives "wrong card"
- a second BIOS service (mask ROM 0x82373), called by Hello Kitty no Eikaiwa and Kiiroi
  Kyouryuu-kun, accepts only 0x89 0xbd (Intel 28F020, 2 Mbit)
The card type is selected in the machine configuration; the contents are kept as NVRAM.

Inputs: the tablet is read by the CPU's A/D converter (AN0-AN3), with port 7 bits 2-3 selecting
calibration measurements. Port 9: bit 0 = pen button (clicks on-screen buttons while the pen points at them),
bit 1 = pen up, bit 2 = cartridge present (slot 2), bit 3 = memory card present (slot 1).
The tablet is larger than the picture: below it is a strip of 12 fields whose pictures are printed
on each cartridge's overlay sheet. They are mapped to their own inputs.

TODO:
- CPU, video and M6585 clocks are guesses
- the tablet emulation reproduces what the BIOS expects, not the real panel
- artwork for the overlay sheets (the strip fields and their pictures are cartridge specific)

******************************************************************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/h8500/h8532.h"
#include "machine/nvram.h"
#include "sound/msm5205.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#define LOG_BANK    (1U << 1)
#define LOG_SOUND   (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

class picno_state : public driver_device
{
public:
	picno_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_cursor_palette(*this, "cursor_palette")
		, m_msm(*this, "msm")
		, m_cart(*this, "cartslot") // slot 2
		, m_maskrom(*this, "maskrom")
		, m_vram(*this, "vram")
		, m_cursor_ram(*this, "cursor_ram")
		, m_io_pen(*this, "PEN_%c", 'X')
		, m_io_buttons(*this, "BUTTONS")
		, m_io_strip(*this, "STRIP")
		, m_io_config(*this, "CONFIG")
	{ }

	void picno(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// 054715 register offsets
	enum : u8
	{
		REG_VWIN0 = 0x00, REG_VWIN1, REG_CWIN0, REG_CWIN1,
		REG_SND_CTRL = 0x10, REG_SND_IRQ, REG_SND_SRC, REG_SND_ADDR, REG_SND_LEN = 0x16,
		REG_VIDEO = 0x20, REG_DISP_START = 0x22, REG_VBL_IRQ = 0x24, REG_STATUS,
		REG_XFER = 0x30,
		REG_CURSOR_X = 0x40, REG_CURSOR_Y, REG_HOTSPOT_X, REG_HOTSPOT_Y
	};

	static constexpr int IRQ_VBLANK = 0;
	static constexpr int IRQ_SOUND = 1;

	void mem_map(address_map &map) ATTR_COLD;

	u8 reg_r(offs_t offset);
	void reg_w(offs_t offset, u8 data);
	template <int Win> u8 vram_window_r(offs_t offset);
	template <int Win> void vram_window_w(offs_t offset, u8 data);
	template <int Win> u8 cart_window_r(offs_t offset);
	template <int Win> void cart_window_w(offs_t offset, u8 data);
	u8 row_transfer_r(offs_t offset);
	void row_transfer_w(offs_t offset, u8 data);

	u8 cart_r(offs_t offset);
	u32 card_size() const;
	u8 card_r(offs_t offset);
	void card_w(offs_t offset, u8 data);
	void sample_start();
	void sample_stop();
	void msm_vck_w(int state);

	bool pen_position(int &x, int &y);
	template <int Ch> u16 tablet_r();
	u8 port9_r();
	void port7_w(u8 data);

	void vblank_w(int state);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<h8532_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<palette_device> m_cursor_palette;
	required_device<msm6585_device> m_msm;
	required_device<generic_slot_device> m_cart;
	required_region_ptr<u8> m_maskrom;
	required_shared_ptr<u8> m_vram;
	required_shared_ptr<u8> m_cursor_ram;
	required_ioport_array<2> m_io_pen;
	required_ioport m_io_buttons;
	required_ioport m_io_strip;
	required_ioport m_io_config;

	u8 m_regs[0x100];
	u8 m_port7 = 0;
	u8 m_row_latch[0x100];      // VRAM serial register

	// memory card: Intel 28F010 / 28F020
	enum card_mode : u8 { CARD_READ, CARD_READ_ID, CARD_PROGRAM, CARD_ERASE_SETUP };
	enum card_type : u8 { CARD_NONE, CARD_28F010, CARD_28F020 };
	static constexpr u32 CARD_SIZE = 0x40000; // largest supported chip
	std::unique_ptr<u8[]> m_card;
	u8 m_card_mode = CARD_READ;

	u32 m_snd_addr = 0;
	u32 m_snd_remain = 0;
	bool m_snd_low_nibble = false;
	bool m_snd_playing = false;
};


/**************************************************************************
    Bus
**************************************************************************/

u8 picno_state::cart_r(offs_t offset)
{
	return m_cart->exists() ? m_cart->read_rom(offset) : 0xff;
}

template <int Win>
u8 picno_state::vram_window_r(offs_t offset)
{
	const u8 bank = m_regs[REG_VWIN0 + Win];
	if (!BIT(bank, 7))
	{
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_BANK, "%s: read from disabled VRAM window %d (%02x)\n", machine().describe_context(), Win, bank);
		return 0xff;
	}
	return m_vram[((bank & 0x0f) << 13) | offset];
}

template <int Win>
void picno_state::vram_window_w(offs_t offset, u8 data)
{
	const u8 bank = m_regs[REG_VWIN0 + Win];
	if (!BIT(bank, 7))
	{
		LOGMASKED(LOG_BANK, "%s: write to disabled VRAM window %d (%02x)\n", machine().describe_context(), Win, bank);
		return;
	}
	m_vram[((bank & 0x0f) << 13) | offset] = data;
}

u8 picno_state::row_transfer_r(offs_t offset)
{
	if (!BIT(m_regs[REG_XFER], 0) && !machine().side_effects_disabled())
		std::copy_n(&m_vram[offset << 8], 0x100, m_row_latch);
	return 0xff;
}

void picno_state::row_transfer_w(offs_t offset, u8 data)
{
	if (!BIT(m_regs[REG_XFER], 0))
		std::copy_n(m_row_latch, 0x100, &m_vram[offset << 8]);
}

template <int Win>
u8 picno_state::cart_window_r(offs_t offset)
{
	const u8 bank = m_regs[REG_CWIN0 + Win];
	const offs_t addr = (bank & 0x0f) << 16 | offset;
	switch (bank >> 4)
	{
	case 1: return cart_r(addr);
	case 2: return card_r(addr);
	default:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_BANK, "%s: read from cartridge window %d with unknown bank %02x\n", machine().describe_context(), Win, bank);
		return 0xff;
	}
}

template <int Win>
void picno_state::cart_window_w(offs_t offset, u8 data)
{
	const u8 bank = m_regs[REG_CWIN0 + Win];
	if ((bank >> 4) == 2)
		card_w((bank & 0x0f) << 16 | offset, data);
	else
		LOGMASKED(LOG_BANK, "%s: write to cartridge window %d with bank %02x: %02x\n", machine().describe_context(), Win, bank, data);
}


/**************************************************************************
    Memory card (Intel 28F010 / 28F020)

    The 28F0x0 family has no internal write state machine: the software
    times the program and erase pulses itself and checks the result with the
    verify commands, which simply read back the array here.
**************************************************************************/

u32 picno_state::card_size() const
{
	switch (m_io_config->read() & 0x03)
	{
	case CARD_28F010: return 0x20000;
	case CARD_28F020: return 0x40000;
	default:          return 0;
	}
}

u8 picno_state::card_r(offs_t offset)
{
	const u32 size = card_size();
	if (!size)
		return 0xff;

	offset &= size - 1;
	if (m_card_mode == CARD_READ_ID)
		return BIT(offset, 0) ? (size == 0x20000 ? 0xb4 : 0xbd) : 0x89; // Intel 28F010 / 28F020
	return m_card[offset];
}

void picno_state::card_w(offs_t offset, u8 data)
{
	const u32 size = card_size();
	if (!size)
		return;

	offset &= size - 1;
	switch (m_card_mode)
	{
	case CARD_PROGRAM:
		m_card[offset] &= data; // programming can only clear bits
		m_card_mode = CARD_READ;
		return;

	case CARD_ERASE_SETUP:
		if (data == 0x20)
			std::fill_n(m_card.get(), size, 0xff);
		m_card_mode = CARD_READ;
		return;

	default:
		break;
	}

	switch (data)
	{
	case 0x00: // read
	case 0xa0: // erase verify
	case 0xc0: // program verify
	case 0xff: // reset
		m_card_mode = CARD_READ;
		break;
	case 0x20: m_card_mode = CARD_ERASE_SETUP; break;
	case 0x40: m_card_mode = CARD_PROGRAM; break;
	case 0x90: m_card_mode = CARD_READ_ID; break;
	default:
		logerror("%s: unknown memory card command %02x\n", machine().describe_context(), data);
		break;
	}
}

/**************************************************************************
    054715 registers
**************************************************************************/

u8 picno_state::reg_r(offs_t offset)
{
	u8 data = m_regs[offset];

	switch (offset)
	{
	case REG_STATUS:
		data = (data & ~0x10) | (m_screen->vblank() ? 0x10 : 0x00);
		break;

	case REG_VBL_IRQ:
		if (!machine().side_effects_disabled())
			m_maincpu->set_input_line(IRQ_VBLANK, CLEAR_LINE);
		break;
	}
	return data;
}

void picno_state::reg_w(offs_t offset, u8 data)
{
	const u8 old = m_regs[offset];
	m_regs[offset] = data;

	switch (offset)
	{
	case REG_SND_CTRL:
		if (BIT(data, 0) && !BIT(old, 0))
			sample_start();
		else if (!BIT(data, 0) && m_snd_playing)
			sample_stop();
		break;

	case REG_SND_IRQ:
		if (!BIT(data, 0))
			m_maincpu->set_input_line(IRQ_SOUND, CLEAR_LINE);
		break;

	case REG_VBL_IRQ:
		if (!BIT(data, 0))
			m_maincpu->set_input_line(IRQ_VBLANK, CLEAR_LINE);
		break;
	}
}


/**************************************************************************
    Sound
**************************************************************************/

void picno_state::sample_start()
{
	m_snd_addr = (m_regs[REG_SND_ADDR] << 16 | m_regs[REG_SND_ADDR + 1] << 8 | m_regs[REG_SND_ADDR + 2]) & 0xfffff;
	m_snd_remain = m_regs[REG_SND_LEN] << 8 | m_regs[REG_SND_LEN + 1];
	m_snd_low_nibble = false;
	m_snd_playing = true;
	m_msm->reset_w(0);
	LOGMASKED(LOG_SOUND, "sample start: %s %05x, %04x bytes\n", BIT(m_regs[REG_SND_SRC], 0) ? "cartridge" : "mask ROM", m_snd_addr, m_snd_remain);
}

void picno_state::sample_stop()
{
	m_snd_playing = false;
	m_regs[REG_SND_CTRL] &= ~0x01;
	m_msm->reset_w(1);
}

void picno_state::msm_vck_w(int state)
{
	if (!m_snd_playing)
		return;

	if (!m_snd_remain)
	{
		sample_stop();
		if (BIT(m_regs[REG_SND_IRQ], 0))
			m_maincpu->set_input_line(IRQ_SOUND, ASSERT_LINE);
		return;
	}

	const u8 data = BIT(m_regs[REG_SND_SRC], 0) ? cart_r(m_snd_addr) : m_maskrom[m_snd_addr & (m_maskrom.bytes() - 1)];
	if (!m_snd_low_nibble)
	{
		m_msm->data_w(data >> 4);
	}
	else
	{
		m_msm->data_w(data & 0x0f);
		m_snd_addr = (m_snd_addr + 1) & 0xfffff;
		m_snd_remain--;
	}
	m_snd_low_nibble = !m_snd_low_nibble;
}


/**************************************************************************
    Inputs
**************************************************************************/

// Returns the pen position in screen coordinates (y beyond 191 is the field strip).
// Holding one of the strip inputs moves the pen onto that field and counts as a touch.
bool picno_state::pen_position(int &x, int &y)
{
	// X centres of the 12 strip fields, from the BIOS hit-test table at 0x7ea8
	static constexpr u8 STRIP_X[12] = { 0x03, 0x13, 0x29, 0x3f, 0x55, 0x6b, 0x94, 0xaa, 0xc0, 0xd6, 0xec, 0xfb };

	const u16 strip = m_io_strip->read();
	for (int i = 0; i < 12; i++)
	{
		if (BIT(strip, i))
		{
			x = STRIP_X[i];
			y = 0xe8;
			return true;
		}
	}

	x = m_io_pen[0]->read();
	y = m_io_pen[1]->read() * 192 / 256;
	return false;
}

// With port 7 bit 2 or 3 set the BIOS takes calibration readings (pen up) and stores them as
// ref0-ref3. With both clear, each axis is read as two halves around the centre (0x80):
//   X = 0x80 - AN0 * 0x84 / ref0  or  0x80 + AN1 * 0x84 / ref1
//   Y = 0x80 - AN2 * 0x84 / ref2  or  0x80 + AN3 * 0x44 / ref3  (AN3 >= 0x60 is used unscaled)
// The values returned here produce the wanted position through exactly this arithmetic.
template <int Ch>
u16 picno_state::tablet_r()
{
	if (BIT(m_port7, 2) || BIT(m_port7, 3))
		return (Ch == 3) ? (0x40 << 2) : (0x80 << 2);

	// smallest raw value whose scaled result (integer division) reaches the target
	auto raw = [] (int target, int num, int den) { return (target * den + num - 1) / num; };

	int x, y;
	pen_position(x, y);

	int value = 0;
	switch (Ch)
	{
	case 0: if (x < 0x80) value = raw(0x80 - x, 0x84, 0x80); break;
	case 1: if (x >= 0x80) value = raw(x - 0x80, 0x84, 0x80); break;
	case 2: if (y < 0x80) value = raw(0x80 - y, 0x84, 0x80); break;
	case 3:
		if (y >= 0x80)
		{
			value = raw(y - 0x80, 0x44, 0x40);
			if (value >= 0x60)
				value = y - 0x80;
		}
		break;
	}
	return std::clamp(value, 0, 0xff) << 2; // 10-bit converter
}

u8 picno_state::port9_r()
{
	int x, y;
	u8 data = m_io_buttons->read();
	if (pen_position(x, y))
		data &= ~0x02; // a strip field is held: pen down
	if (!m_cart->exists())
		data &= ~0x04;
	if (!card_size())
		data &= ~0x08;
	return data;
}

void picno_state::port7_w(u8 data)
{
	m_port7 = data;
	m_msm->playmode_w(0x04 | BIT(data, 4) | BIT(data, 5) << 1); // 4-bit data, S1/S2 select the sampling rate
}

static INPUT_PORTS_START( picno )
	PORT_START("PEN_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(4) PORT_NAME("Pen X")

	PORT_START("PEN_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(4) PORT_NAME("Pen Y")

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Pen Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Pen Touch")
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED ) // bits 2-3: cartridge / memory card present, see port9_r

	PORT_START("STRIP") // fields printed on the overlay sheet below the picture, left to right
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Strip Field 1")  PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Strip Field 2")  PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Strip Field 3")  PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Strip Field 4")  PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Strip Field 5")  PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Strip Field 6")  PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Strip Field 7")  PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Strip Field 8")  PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Strip Field 9")  PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Strip Field 10") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x400, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Strip Field 11") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x800, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Strip Field 12") PORT_CODE(KEYCODE_EQUALS)

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x03, 0x01, "Memory Card (Slot 1)" )
	PORT_CONFSETTING(    0x00, "Not inserted" )
	PORT_CONFSETTING(    0x01, "Intel 28F010 (128K)" )
	PORT_CONFSETTING(    0x02, "Intel 28F020 (256K)" )
INPUT_PORTS_END


/**************************************************************************
    Video
**************************************************************************/

void picno_state::vblank_w(int state)
{
	if (state && BIT(m_regs[REG_VBL_IRQ], 0))
		m_maincpu->set_input_line(IRQ_VBLANK, ASSERT_LINE);
}

u32 picno_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *const pens = m_palette->pens();
	const u32 start = (m_regs[REG_DISP_START] << 8 | m_regs[REG_DISP_START + 1]) << 8;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u32 *const dst = &bitmap.pix(y);
		const u32 line = start + (y << 8);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			dst[x] = pens[m_vram[(line + x) & 0x1ffff]];
	}

	if (BIT(m_regs[REG_VIDEO], 2))
	{
		const pen_t *const cursor_pens = m_cursor_palette->pens();
		const int sx = m_regs[REG_CURSOR_X] - m_regs[REG_HOTSPOT_X];
		const int sy = m_regs[REG_CURSOR_Y] - m_regs[REG_HOTSPOT_Y];

		for (int y = 0; y < 32; y++)
		{
			const int py = (sy + y) & 0xff;
			if (py < cliprect.min_y || py > cliprect.max_y)
				continue;

			for (int x = 0; x < 32; x++)
			{
				const int px = (sx + x) & 0xff;
				if (px < cliprect.min_x || px > cliprect.max_x)
					continue;

				const u8 pix = BIT(m_cursor_ram[y * 16 + x / 2], (x & 1) ? 0 : 4, 4);
				if (pix)
					bitmap.pix(py, px) = cursor_pens[pix];
			}
		}
	}
	return 0;
}


/**************************************************************************
    Machine
**************************************************************************/

void picno_state::mem_map(address_map &map)
{
	map(0x08000, 0x09fff).rw(FUNC(picno_state::vram_window_r<0>), FUNC(picno_state::vram_window_w<0>));
	map(0x0a000, 0x0bfff).rw(FUNC(picno_state::vram_window_r<1>), FUNC(picno_state::vram_window_w<1>));
	map(0x0e000, 0x0e0ff).rw(FUNC(picno_state::reg_r), FUNC(picno_state::reg_w));
	map(0x0e400, 0x0e5ff).rw(FUNC(picno_state::row_transfer_r), FUNC(picno_state::row_transfer_w));
	map(0x0e800, 0x0e9ff).ram().share(m_cursor_ram);
	map(0x0ea00, 0x0ea1f).ram().w(m_cursor_palette, FUNC(palette_device::write8)).share("cursor_palette");
	map(0x0ec00, 0x0edff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x0ee00, 0x0efff).ram();
	map(0x20000, 0x2ffff).rw(FUNC(picno_state::cart_window_r<0>), FUNC(picno_state::cart_window_w<0>));
	map(0x30000, 0x3ffff).rw(FUNC(picno_state::cart_window_r<1>), FUNC(picno_state::cart_window_w<1>));
	map(0x40000, 0x5ffff).ram();
	map(0x60000, 0x7ffff).ram().share(m_vram);
	map(0x80000, 0xfffff).rom().region("maskrom", 0);
}

void picno_state::machine_start()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill(std::begin(m_row_latch), std::end(m_row_latch), 0);

	m_card = std::make_unique<u8[]>(CARD_SIZE);
	subdevice<nvram_device>("card")->set_base(m_card.get(), CARD_SIZE);

	save_item(NAME(m_regs));
	save_pointer(NAME(m_card), CARD_SIZE);
	save_item(NAME(m_card_mode));
	save_item(NAME(m_port7));
	save_item(NAME(m_row_latch));
	save_item(NAME(m_snd_addr));
	save_item(NAME(m_snd_remain));
	save_item(NAME(m_snd_low_nibble));
	save_item(NAME(m_snd_playing));
}

void picno_state::machine_reset()
{
	m_card_mode = CARD_READ;
	sample_stop();
	m_maincpu->set_input_line(IRQ_VBLANK, CLEAR_LINE);
	m_maincpu->set_input_line(IRQ_SOUND, CLEAR_LINE);
}

void picno_state::picno(machine_config &config)
{
	HD6435328(config, m_maincpu, 20'000'000); // TODO: clock is a guess, divided by 2 in the cpu
	m_maincpu->set_mode(4);
	m_maincpu->set_addrmap(AS_PROGRAM, &picno_state::mem_map);
	m_maincpu->write_port7().set(FUNC(picno_state::port7_w));
	m_maincpu->read_port9().set(FUNC(picno_state::port9_r));
	m_maincpu->read_adc<0>().set(FUNC(picno_state::tablet_r<0>));
	m_maincpu->read_adc<1>().set(FUNC(picno_state::tablet_r<1>));
	m_maincpu->read_adc<2>().set(FUNC(picno_state::tablet_r<2>));
	m_maincpu->read_adc<3>().set(FUNC(picno_state::tablet_r<3>));

	SCREEN(config, m_screen);
	m_screen->set_raw(21.477272_MHz_XTAL / 4, 341, 0, 256, 262, 0, 192); // TODO: verify, Y2 is likely the NTSC 21.47727 MHz clock
	m_screen->set_screen_update(FUNC(picno_state::screen_update));
	m_screen->screen_vblank().set(FUNC(picno_state::vblank_w));

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 256);
	PALETTE(config, m_cursor_palette).set_format(palette_device::xRGB_555, 16);

	// no speaker in the unit, but there's a couple of sockets on the back
	SPEAKER(config, "speaker").front_center();
	MSM6585(config, m_msm, 640'000); // TODO: verify clock
	m_msm->vck_legacy_callback().set(FUNC(picno_state::msm_vck_w));
	m_msm->set_prescaler_selector(msm6585_device::S80);
	m_msm->add_route(ALL_OUTPUTS, "speaker", 1.0);

	NVRAM(config, "card", nvram_device::DEFAULT_ALL_1);

	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "picno_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("picno");
}


/**************************************************************************
    ROM definitions
**************************************************************************/

ROM_START( picno )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "hd6435328f10.u5", 0x00000, 0x08000, CRC(41c381ab) SHA1(d4ab3e932fd513f0ef9d9f03097e59a8d3baffdc) ) // internal rom

	ROM_REGION(0x80000, "maskrom", 0)
	ROM_LOAD( "rx001-z8-v3j.u2", 0x00000, 0x80000, CRC(e3c8929d) SHA1(1716f09b0a594b3782d257330282d77b6ca6fa0d) ) // HN62334BP
ROM_END

ROM_START( picno2 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "hd6435328f10.u5", 0x00000, 0x08000, CRC(8c36c7f9) SHA1(9aa7672738f6b7d82557792ce12cd70335278673) ) // internal rom

	ROM_REGION(0x80000, "maskrom", 0)
	ROM_LOAD( "rx001-z8-v4j.u2", 0x00000, 0x80000, CRC(ae89a9a5) SHA1(51ed458ffd151e19019beb23517263efce4be272) ) // HN62334BP
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY   FULLNAME   FLAGS
CONS( 1993, picno,  0,      0,      picno,   picno, picno_state, empty_init, "Konami", "Picno",   MACHINE_IMPERFECT_CONTROLS | MACHINE_SUPPORTS_SAVE )
CONS( 1993, picno2, 0,      0,      picno,   picno, picno_state, empty_init, "Konami", "Picno 2", MACHINE_IMPERFECT_CONTROLS | MACHINE_SUPPORTS_SAVE )
