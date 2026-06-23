// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************************************

AlphaSmart 3000

====================================================================================================

PCB:
            ___________                                                                _____________
           /    ::    |_______________________________________________________________/             \
   _______/ ::  ::                              _________      _________              ________       \_____________
  ||RS232|      ::   28F008B3                  |________|     |________| PCB REV 2.8 |_______|  BATT        | USB |
  ||_____|      ::                 74HC574                                                    CR2032   XTAL |_____|
  |__                  HY62U8200                       HC30M                                          A120I0E     |
   __| SP3223ECA                                                                                                  |
 _|_                     DragonBall EZ                                                                 PDIUSBD11D |
|___|<-Power             MC68EZ328PU16V                                    :)                              _______|
  |                                                                   Rise and Shout                      |
  |                            XTAL                                   the AlphaSmart's                    |
  |__   __      _____                     SW        HC132A             Out!!!                             |
     | |__|    /     | HC132A     HC74A  on/off                                       ____________________|
     |________/      |_______________________________________________________________/

The later AlphaSmart models' firmware can be updated using the Manager application (Windows / Mac) and a USB cable.
Each update comprises two files, the "os" and the "smallos". Those files do not include the full Operating System image.
Two version updaters known:
- System 3 Neo Jul 11 2013, 09:44:53 + OS 3KNeo Small ROM, included with Manager 3.93
- System 3 Neo Jan 27 2010, 13:44:00 + OS 3KNeo Small ROM, included with Manager 3.60

    TODO:
    - Dynamically load smart applets as cartridges or similar.
    - Support the Alphasmart Neo (needs driver for ST7675).
    - USB support?

***************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68328.h"
#include "machine/nvram.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "util/multibyte.h"

static const uint32_t SMALLOS_BEGIN = 0x00'0000;
static const uint32_t APPLETS_BEGIN = 0x00'8000;
static const uint32_t FULLOS_BEGIN  = 0x0c'0000;
static const uint32_t APPLETS_END   = FULLOS_BEGIN;
static const uint32_t ROM_SIZE      = 0x10'0000;
static const uint32_t RAM_SIZE      = 0x04'0000;

static const uint32_t APPLET_MAGIC_START = 0xc0ff'eead;
static const uint32_t APPLET_MAGIC_END   = 0xcafe'feed;

DECLARE_DEVICE_TYPE(ALPHASMART_APPLET, alphasmart_applet_device)

class alphasmart_applet_device : public device_t, public device_image_interface
{
public:
	alphasmart_applet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0)
		: device_t(mconfig, ALPHASMART_APPLET, tag, owner, clock)
		, device_image_interface(mconfig, *this)
	{ }

	// device_image_interface requirements
	virtual const char *image_interface() const noexcept override { return "alphasmart_applet"; }
	virtual const char *file_extensions() const noexcept override { return "os3kapp"; }
	virtual bool is_readable() const noexcept override { return true; }
	virtual bool is_writeable()	const noexcept override { return false; }
	virtual bool is_creatable()	const noexcept override { return false; }
	virtual bool is_reset_on_load()	const noexcept override { return false; }
	virtual const char *image_type_name() const noexcept override { return "SmartApplet"; }
	virtual const char *image_brief_type_name() const noexcept override { return "applet"; }

	const uint8_t *data() const { return m_data.data(); }
	uint32_t size() const { return m_data.size(); }
	bool loaded() const { return !m_data.empty(); }

protected:
	virtual void device_start() override { }

	virtual std::pair<std::error_condition, std::string> call_load() override
	{
		uint32_t const len = length();
		if (len < 12)
			return std::make_pair(image_error::INVALIDLENGTH, "File too small");

		m_data.resize(len);
		if (fread(m_data.data(), m_data.size()) != m_data.size())
		{
			m_data.clear();
			return std::make_pair(image_error::UNSPECIFIED, "Read error");
		}

		// Validate magic bytes and length
		uint32_t const sm = get_u32be(&m_data[0]);
		uint32_t const sz = get_u32be(&m_data[4]);
		uint32_t const em = get_u32be(&m_data[len - 4]);
		if (sm != APPLET_MAGIC_START || sz != len || em != APPLET_MAGIC_END)
		{
			m_data.clear();
			return std::make_pair(image_error::INVALIDIMAGE, "Invalid applet format");
		}

		return std::make_pair(std::error_condition(), std::string());
	}

	virtual void call_unload() override
	{
		m_data.clear();
	}

private:
	std::vector<uint8_t> m_data;
};

DEFINE_DEVICE_TYPE(ALPHASMART_APPLET, alphasmart_applet_device, "alphasmart_applet", "AlphaSmart Applet")

namespace
{

class alphasmart3k_state : public driver_device
{
public:
	alphasmart3k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "ks0066_%u", 0U)
		, m_nvram(*this, "nvram")
		, m_nvram_base(*this, "nvram", RAM_SIZE, ENDIANNESS_BIG)
		, m_flash_base(*this, "flash", ROM_SIZE, ENDIANNESS_BIG)
		, m_keyboard(*this, "COL.%u", 0)
		, m_applet(*this, "applet")
	{
	}

	void alphasmart3k(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(kb_irq);

void init_empty_flash(nvram_device &nvram, void *base, size_t size)
{
	// Copy reset vectors from flash into NVRAM
	memory_region *rom = memregion("maincpu");
	memcpy(m_nvram_base, rom->base(), 0x400);
	memcpy(&m_flash_base[0], rom->base(), rom->bytes());
}

protected:
	required_device<mc68328_base_device> m_maincpu;
	required_device_array<ks0066_device, 2> m_lcdc;
	required_device<nvram_device> m_nvram;
	memory_share_creator<uint16_t> m_nvram_base;
	memory_share_creator<uint16_t> m_flash_base;
	required_ioport_array<16> m_keyboard;
	required_device<alphasmart_applet_device> m_applet;

	std::unique_ptr<bitmap_ind16> m_tmp_bitmap;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void alphasmart_palette(palette_device &palette) const;
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t kb_r();
	uint8_t kb_matrixh_r();
	void kb_matrixh_w(uint8_t data);

	template <int Line> void port_a_w(int state);
	template <int Line> int port_a_r();
	template <int Line> int port_d_r();

	template <int Line> int port_c_r();
	template <int Line> void port_c_w(int state);

private:
	void main_map(address_map &map) ATTR_COLD;

	uint8_t m_matrix[2];
	uint8_t m_port_a;
	uint8_t m_port_d;

	uint8_t m_port_c;
};

void alphasmart3k_state::alphasmart_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

uint32_t alphasmart3k_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_lcdc[0]->screen_update(screen, *m_tmp_bitmap, cliprect);
	copybitmap(bitmap, *m_tmp_bitmap, 0, 0, 0, 0, cliprect);
	m_lcdc[1]->screen_update(screen, *m_tmp_bitmap, cliprect);
	copybitmap(bitmap, *m_tmp_bitmap, 0, 0, 0, 18,cliprect);
	return 0;
}

void alphasmart3k_state::machine_start()
{
	m_tmp_bitmap = std::make_unique<bitmap_ind16>(6 * 40, 9 * 4);

	// Hold CPU in reset until flash is ready
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void alphasmart3k_state::machine_reset()
{
	if (m_applet->loaded())
	{
		uint32_t const sz = m_applet->size();
		uint8_t const *src = m_applet->data();
		uint8_t *dst = (uint8_t *)&m_flash_base[0];

		// Scan flash to find next free slot after existing applets
		uint32_t offset = APPLETS_BEGIN;
		while (offset + 8 <= APPLETS_END)
		{
			uint32_t const sm = get_u32be(&dst[offset]);
			if (sm != APPLET_MAGIC_START) break;
			uint32_t const sz = get_u32be(&dst[offset + 4]);
			if (sz < 8 || offset + sz > APPLETS_END) break;
			uint32_t const em = get_u32be(&dst[offset + sz - 4]);
			if (em != APPLET_MAGIC_END) break;

			offset += sz;
		}

printf("%02X%02X%02X%02X %02X%02X%02X%02X\n", dst[0], dst[1], dst[2], dst[3], dst[offset], dst[offset+1], dst[offset+2], dst[offset+3]);
		if (offset + sz <= APPLETS_END)
		{
/*
			uint32_t words = sz / 2;
			for (uint32_t i = 0; i < words; i++)
			{
				dst[offset + i] = get_u16be(&src[i * 2]);
			}
*/
			uint16_t *dest = &m_flash_base[0];
printf("%04X%04X %04X%04X\n", dest[0], dest[1], dest[offset/2], dest[offset/2+1]);
			memcpy(dst + offset, src, sz);
			logerror("alphasmart: installed applet at flash+0x%06X size 0x%X\n", offset, sz);
printf("%04X%04X %04X%04X\n", dest[0], dest[1], dest[offset/2], dest[offset/2+1]);
		}
		else
		{
			logerror("alphasmart: applet too large to fit, flash full\n");
		}

		// Auto-eject: applet is now in flash, medium no longer needed
		m_applet->unload();
	}

	m_matrix[0] = m_matrix[1] = 0;
	m_port_a = 0;
	m_port_d = 0;

	m_port_c = 0;

    // Release CPU now that initialization is complete
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

void alphasmart3k_state::main_map(address_map &map)
{
	map(0x0000'0000, 0x0003'ffff).ram().share("nvram");
	map(0x0040'0000, 0x004f'ffff).ram().share("flash");
	map(0x0060'0000, 0x0060'0000).rw(FUNC(alphasmart3k_state::kb_matrixh_r), FUNC(alphasmart3k_state::kb_matrixh_w));
}

INPUT_CHANGED_MEMBER(alphasmart3k_state::kb_irq)
{
	// IRQ on every key transition
	m_maincpu->set_input_line(M68K_IRQ_4, HOLD_LINE);
}

uint8_t alphasmart3k_state::kb_r()
{
	uint16_t matrix = (m_matrix[1]<<8) | m_matrix[0];
	uint8_t data = 0xff;

	for(int i=0; i<16; i++)
		if (!BIT(matrix, (i - 1) & 0xF))
			data &= m_keyboard[i]->read();

	return data;
}

uint8_t alphasmart3k_state::kb_matrixh_r()
{
	return m_matrix[1];
}

void alphasmart3k_state::kb_matrixh_w(uint8_t data)
{
	m_matrix[1] = data;
	m_port_d = kb_r();
	//m_maincpu->irq4_w(false);
}

template <int Line>
void alphasmart3k_state::port_a_w(int state)
{
	m_port_a &= ~(1 << Line);
	m_port_a |= state << Line;
	if (Line == 6)
	{
		m_matrix[0] = m_port_a & 0x7F;
	}
}

template <int Line>
int alphasmart3k_state::port_a_r()
{
	return BIT(m_port_a, Line);
}

template <int Line>
int alphasmart3k_state::port_d_r()
{
	return BIT(m_port_d, Line);
}

template <int Line>
int alphasmart3k_state::port_c_r()
{
	return BIT(m_port_c, Line);
}

template <int Line>
void alphasmart3k_state::port_c_w(int state)
{
	m_port_c &= ~(1 << Line);
	m_port_c |= state << Line;
	if (Line >= 6 && state)
	{
		if (BIT(m_port_c, 4))
		{
			m_port_c &= 0xF0;
			uint8_t res = m_lcdc[Line - 6]->read(BIT(m_port_c, 5));
			m_port_c |= res >> 4;
		}
		else
		{
			m_lcdc[Line - 6]->write(BIT(m_port_c, 5), (m_port_c & 0xF) << 4);
		}
	}
}

static INPUT_PORTS_START( alphasmart3k )
	PORT_START("COL.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)   PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_NAME("F6 (File 6)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('k')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('i') PORT_CHAR('I')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('_') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('*')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Left Alt/Option") PORT_CHAR(UCHAR_MAMEKEY(LALT))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Right Alt/Option") PORT_CHAR(UCHAR_MAMEKEY(RALT))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)   PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_NAME("F7 (File 7)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('o') PORT_CHAR('O')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)   PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_NAME("F8 (File 8)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR('(')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')    PORT_CHAR('\"') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p') PORT_CHAR('P')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR(')')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_START("COL.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LWIN) PORT_CODE(KEYCODE_PGUP)  PORT_NAME("Command") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)  PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_NAME("Clear File") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_START("COL.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)  PORT_CHAR(UCHAR_MAMEKEY(END))    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_START("COL.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_START("COL.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)  PORT_CHAR(UCHAR_MAMEKEY(F11)) PORT_NAME("Find") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)   PORT_CHAR(UCHAR_MAMEKEY(UP))     PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_START("COL.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Delete") PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)   PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_NAME("F5 (File 5)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)   PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_NAME("Print") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)  PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_NAME("Spell Check") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)   PORT_NAME("Return") PORT_CHAR(13) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_START("COL.10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)   PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("F3 (File 3)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)   PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_NAME("F4 (File 4)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)    PORT_CHAR('d')  PORT_CHAR('D')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)    PORT_CHAR('e')  PORT_CHAR('E')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)   PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_NAME("F2 (File 2)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)    PORT_CHAR('3')  PORT_CHAR('#')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)    PORT_CHAR('c')  PORT_CHAR('C')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)    PORT_CHAR('s')  PORT_CHAR('S')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)    PORT_CHAR('w')  PORT_CHAR('W')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)   PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("F1 (File 1)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)    PORT_CHAR('2')  PORT_CHAR('@')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)    PORT_CHAR('x')  PORT_CHAR('X')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)  PORT_CHAR('\t')   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)    PORT_CHAR('a')  PORT_CHAR('A')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)    PORT_CHAR('q')  PORT_CHAR('Q')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)    PORT_CHAR('1')  PORT_CHAR('!')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)    PORT_CHAR('z')  PORT_CHAR('z')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_START("COL.13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)    PORT_CHAR('t')  PORT_CHAR('T')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)    PORT_CHAR('g')  PORT_CHAR('G')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)    PORT_CHAR('f')  PORT_CHAR('F')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)    PORT_CHAR('r')  PORT_CHAR('R')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)    PORT_CHAR('5')  PORT_CHAR('%')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)    PORT_CHAR('4')  PORT_CHAR('$')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)    PORT_CHAR('v')  PORT_CHAR('V')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)    PORT_CHAR('b')  PORT_CHAR('B')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_START("COL.14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.15")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)    PORT_CHAR('y')  PORT_CHAR('Y')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)    PORT_CHAR('h')  PORT_CHAR('H')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)    PORT_CHAR('j')  PORT_CHAR('J')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)    PORT_CHAR('u')  PORT_CHAR('U')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)    PORT_CHAR('6')  PORT_CHAR('^')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)    PORT_CHAR('7')  PORT_CHAR('&')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)    PORT_CHAR('m')  PORT_CHAR('M')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)    PORT_CHAR('n')  PORT_CHAR('N')  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(alphasmart3k_state::kb_irq), 0)
INPUT_PORTS_END

void alphasmart3k_state::alphasmart3k(machine_config &config)
{
	// Basic machine hardware
	MC68EZ328(config, m_maincpu, 16'000'000); // MC68EZ328PU16V, clock unverified
	m_maincpu->set_addrmap(AS_PROGRAM, &alphasmart3k_state::main_map);

	m_maincpu->in_port_a<0>().set(FUNC(alphasmart3k_state::port_a_r<0>));
	m_maincpu->in_port_a<1>().set(FUNC(alphasmart3k_state::port_a_r<1>));
	m_maincpu->in_port_a<2>().set(FUNC(alphasmart3k_state::port_a_r<2>));
	m_maincpu->in_port_a<3>().set(FUNC(alphasmart3k_state::port_a_r<3>));
	m_maincpu->in_port_a<4>().set(FUNC(alphasmart3k_state::port_a_r<4>));
	m_maincpu->in_port_a<5>().set(FUNC(alphasmart3k_state::port_a_r<5>));
	m_maincpu->in_port_a<6>().set(FUNC(alphasmart3k_state::port_a_r<6>));
	m_maincpu->in_port_a<7>().set(FUNC(alphasmart3k_state::port_a_r<7>));

	m_maincpu->out_port_a<0>().set(FUNC(alphasmart3k_state::port_a_w<0>));
	m_maincpu->out_port_a<1>().set(FUNC(alphasmart3k_state::port_a_w<1>));
	m_maincpu->out_port_a<2>().set(FUNC(alphasmart3k_state::port_a_w<2>));
	m_maincpu->out_port_a<3>().set(FUNC(alphasmart3k_state::port_a_w<3>));
	m_maincpu->out_port_a<4>().set(FUNC(alphasmart3k_state::port_a_w<4>));
	m_maincpu->out_port_a<5>().set(FUNC(alphasmart3k_state::port_a_w<5>));
	m_maincpu->out_port_a<6>().set(FUNC(alphasmart3k_state::port_a_w<6>));
	m_maincpu->out_port_a<7>().set(FUNC(alphasmart3k_state::port_a_w<7>));

	m_maincpu->in_port_d<0>().set(FUNC(alphasmart3k_state::port_d_r<0>));
	m_maincpu->in_port_d<1>().set(FUNC(alphasmart3k_state::port_d_r<1>));
	m_maincpu->in_port_d<2>().set(FUNC(alphasmart3k_state::port_d_r<2>));
	m_maincpu->in_port_d<3>().set(FUNC(alphasmart3k_state::port_d_r<3>));
	m_maincpu->in_port_d<4>().set(FUNC(alphasmart3k_state::port_d_r<4>));
	m_maincpu->in_port_d<5>().set(FUNC(alphasmart3k_state::port_d_r<5>));
	m_maincpu->in_port_d<6>().set(FUNC(alphasmart3k_state::port_d_r<6>));
	m_maincpu->in_port_d<7>().set(FUNC(alphasmart3k_state::port_d_r<7>));

	m_maincpu->in_port_g<4>().set_constant(1);

	m_maincpu->in_port_c<0>().set(FUNC(alphasmart3k_state::port_c_r<0>));
	m_maincpu->in_port_c<1>().set(FUNC(alphasmart3k_state::port_c_r<1>));
	m_maincpu->in_port_c<2>().set(FUNC(alphasmart3k_state::port_c_r<2>));
	m_maincpu->in_port_c<3>().set(FUNC(alphasmart3k_state::port_c_r<3>));
	m_maincpu->in_port_c<4>().set(FUNC(alphasmart3k_state::port_c_r<4>));
	m_maincpu->in_port_c<5>().set(FUNC(alphasmart3k_state::port_c_r<5>));
	m_maincpu->in_port_c<6>().set(FUNC(alphasmart3k_state::port_c_r<6>));
	m_maincpu->in_port_c<7>().set(FUNC(alphasmart3k_state::port_c_r<7>));

	m_maincpu->out_port_c<0>().set(FUNC(alphasmart3k_state::port_c_w<0>));
	m_maincpu->out_port_c<1>().set(FUNC(alphasmart3k_state::port_c_w<1>));
	m_maincpu->out_port_c<2>().set(FUNC(alphasmart3k_state::port_c_w<2>));
	m_maincpu->out_port_c<3>().set(FUNC(alphasmart3k_state::port_c_w<3>));
	m_maincpu->out_port_c<4>().set(FUNC(alphasmart3k_state::port_c_w<4>));
	m_maincpu->out_port_c<5>().set(FUNC(alphasmart3k_state::port_c_w<5>));
	m_maincpu->out_port_c<6>().set(FUNC(alphasmart3k_state::port_c_w<6>));
	m_maincpu->out_port_c<7>().set(FUNC(alphasmart3k_state::port_c_w<7>));

	// Values from AlphaSmart 2000, not confirmed for AlphaSmart 3000
	// AlphaSmart 3000 uses a Data Image CM4040 LCD display, LCD is 40x4 according to ref
	for (auto &lcdc : m_lcdc)
	{
		KS0066(config, lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
		lcdc->set_default_bios_tag("f05");
		lcdc->set_lcd_size(2, 40);
	}

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(alphasmart3k_state::screen_update));
	screen.set_size(6*40, 9*4);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(alphasmart3k_state::alphasmart_palette), 2);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "flash", nvram_device::DEFAULT_ALL_1).set_custom_handler(FUNC(alphasmart3k_state::init_empty_flash));
	ALPHASMART_APPLET(config, m_applet);
}

// ROM definitions

ROM_START( asma3k )
	ROM_REGION( ROM_SIZE, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "smallos3krom.os3kos", SMALLOS_BEGIN, 0x0'3A96, CRC(dbf15afb) SHA1(8c3e631e877b85f453f05905036735c89404a308) )
	ROM_LOAD( "os3krom.os3kos",      FULLOS_BEGIN,  0x4'0000, CRC(66de6a17) SHA1(7d800836ca69479b9bb05612d138f9c10428a262) )
	ROM_CONTINUE(                    0x4000, 0x3BF0 )
ROM_END

} // anonymous namespace

//    YEAR  NAME    PARENT COMPAT MACHINE       INPUT         CLASS               INIT        COMPANY             FULLNAME           FLAGS
COMP( 2000, asma3k, 0,     0,     alphasmart3k, alphasmart3k, alphasmart3k_state, empty_init, "AlphaSmart, Inc.", "AlphaSmart 3000", MACHINE_NO_SOUND_HW )
