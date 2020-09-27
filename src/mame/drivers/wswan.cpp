// license:BSD-3-Clause
// copyright-holders:Anthony Kruize,Wilbert Pol
/***************************************************************************

  wswan.cpp

  Driver file to handle emulation of the Bandai WonderSwan
  By:

  Anthony Kruize
  Wilbert Pol

  Based on the WStech documentation by Judge and Dox.

  Usage:
    Keep START button pressed during startup (or reset) to enter the internal
    configuration menu.

  Known issues/TODOs:
  - Add support for noise sound.
  - Add support for voice sound.
  - Add support for enveloped sound.
  - Perform video DMA at proper timing.
  - Add (real/proper) RTC support.
  - Swan Crystal can handle up to 512Mbit ROMs??????
  - SRAM sizes should be in kbit instead of kbytes(?). This raises a few
    interesting issues:
    - mirror of smaller <64KBYTE/512kbit SRAM sizes
    - banking when using 1M or 2M sram sizes
    - The units likely came with the name "WONDERSWAN" configured in the
      internal EEPOM

***************************************************************************/

#include "emu.h"
#include "cpu/v30mz/v30mz.h"
#include "machine/nvram.h"
#include "audio/wswan.h"
#include "video/wswan.h"
#include "bus/wswan/slot.h"
#include "bus/wswan/rom.h"
#include "emupal.h"
#include "render.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "wswan.lh"

#include <algorithm>


namespace {

class wswan_state : public driver_device
{
public:
	wswan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vdp(*this, "vdp"),
		m_sound(*this, "custom"),
		m_cart(*this, "cartslot"),
		m_region_maincpu(*this, "maincpu"),
		m_cursx(*this, "CURSX"),
		m_cursy(*this, "CURSY"),
		m_buttons(*this, "BUTTONS")
	{ }

	void wswan(machine_config &config);

protected:
	// Interrupt flags
	static const u8 WSWAN_IFLAG_STX    = 0x01;
	static const u8 WSWAN_IFLAG_KEY    = 0x02;
	static const u8 WSWAN_IFLAG_RTC    = 0x04;
	static const u8 WSWAN_IFLAG_SRX    = 0x08;
	static const u8 WSWAN_IFLAG_LCMP   = 0x10;
	static const u8 WSWAN_IFLAG_VBLTMR = 0x20;
	static const u8 WSWAN_IFLAG_VBL    = 0x40;
	static const u8 WSWAN_IFLAG_HBLTMR = 0x80;

	// Interrupts
	static const u8 WSWAN_INT_STX    = 0;
	static const u8 WSWAN_INT_KEY    = 1;
	static const u8 WSWAN_INT_RTC    = 2;
	static const u8 WSWAN_INT_SRX    = 3;
	static const u8 WSWAN_INT_LCMP   = 4;
	static const u8 WSWAN_INT_VBLTMR = 5;
	static const u8 WSWAN_INT_VBL    = 6;
	static const u8 WSWAN_INT_HBLTMR = 7;

	static const u32 INTERNAL_EEPROM_SIZE = 1024;   // 16kbit on WSC
	static const u32 INTERNAL_EEPROM_SIZE_WS = 64;  // 1kbit on WS

	enum enum_system { TYPE_WSWAN=0, TYPE_WSC };

	struct sound_dma_t
	{
		sound_dma_t() { }

		u32  source = 0; // Source address
		u16  size = 0;   // Size
		u8   enable = 0; // Enabled
	};

	required_device<cpu_device> m_maincpu;
	required_device<wswan_video_device> m_vdp;
	required_device<wswan_sound_device> m_sound;
	required_device<ws_cart_slot_device> m_cart;

	u8 m_ws_portram[256];
	u8 m_internal_eeprom[INTERNAL_EEPROM_SIZE * 2];
	u8 m_system_type;
	sound_dma_t m_sound_dma;
	u8 m_bios_disabled;
	u8 m_rotate;

	required_memory_region m_region_maincpu;
	required_ioport m_cursx;
	required_ioport m_cursy;
	required_ioport m_buttons;

	u8 bios_r(offs_t offset);
	u8 port_r(offs_t offset);
	void port_w(offs_t offset, u8 data);

	void set_irq_line(int irq);
	void dma_sound_cb();
	void common_start();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void palette(palette_device &palette) const;

	void io_map(address_map &map);
	void mem_map(address_map &map);
	void snd_map(address_map &map);

	void register_save();
	void handle_irqs();
	void clear_irq_line(int irq);
	virtual u16 get_internal_eeprom_address();
};

class wscolor_state : public wswan_state
{
public:
	using wswan_state::wswan_state;
	void wscolor(machine_config &config);

protected:
	virtual void machine_start() override;
	void mem_map(address_map &map);
	void palette(palette_device &palette) const;
	virtual u16 get_internal_eeprom_address() override;
};

static const uint8_t ws_portram_init[256] =
{
	0x00, 0x00, 0x00/*?*/, 0xbb, 0x00, 0x00, 0x00, 0x26, 0xfe, 0xde, 0xf9, 0xfb, 0xdb, 0xd7, 0x7f, 0xf5,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x9e, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x99, 0xfd, 0xb7, 0xdf,
	0x30, 0x57, 0x75, 0x76, 0x15, 0x73, 0x70/*77?*/, 0x77, 0x20, 0x75, 0x50, 0x36, 0x70, 0x67, 0x50, 0x77,
	0x57, 0x54, 0x75, 0x77, 0x75, 0x17, 0x37, 0x73, 0x50, 0x57, 0x60, 0x77, 0x70, 0x77, 0x10, 0x73,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
	0x87, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x4f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xdb, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x42, 0x00, 0x83, 0x00,
	0x2f, 0x3f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1,
	0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1,
	0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1,
	0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1, 0xd1
};

void wswan_state::mem_map(address_map &map)
{
	map(0x00000, 0x03fff).rw(m_vdp, FUNC(wswan_video_device::vram_r), FUNC(wswan_video_device::vram_w));       // 16kb RAM / 4 colour tiles
	map(0x04000, 0x0ffff).noprw();       // nothing
	map(0xf0000, 0xfffff).r(FUNC(wswan_state::bios_r));
}


void wscolor_state::mem_map(address_map &map)
{
	map(0x00000, 0x0ffff).rw(m_vdp, FUNC(wswan_video_device::vram_r), FUNC(wswan_video_device::vram_w));       // 16kb RAM / 4 colour tiles, 16 colour tiles + palettes
	map(0xf0000, 0xfffff).r(FUNC(wscolor_state::bios_r));
}


void wswan_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(wswan_state::port_r), FUNC(wswan_state::port_w));   // I/O ports
}


void wswan_state::snd_map(address_map &map)
{
	map(0x00000, 0x03fff).r(m_vdp, FUNC(wswan_video_device::vram_r));
}


static INPUT_PORTS_START( wswan )
	PORT_START("CURSX")
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("X4 - Left")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("X3 - Down")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("X2 - Right")
	PORT_BIT( 0x1, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("X1 - Up")

	PORT_START("BUTTONS")
	PORT_BIT( 0x8, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Button B")
	PORT_BIT( 0x4, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Button A")
	PORT_BIT( 0x2, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")

	PORT_START("CURSY")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Y4 - Left") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Y3 - Down") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Y2 - Right") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Y1 - Up") PORT_CODE(KEYCODE_W)
INPUT_PORTS_END

static GFXDECODE_START( gfx_wswan )
GFXDECODE_END


/* WonderSwan can display 16 shades of grey */
void wswan_state::palette(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
	{
		uint8_t const shade = i * (256 / 16);
		palette.set_pen_color(15 - i, shade, shade, shade);
	}
}


void wscolor_state::palette(palette_device &palette) const
{
	for (int i = 0; i < 4096; i++)
	{
		int const r = (i & 0x0f00) >> 8;
		int const g = (i & 0x00f0) >> 4;
		int const b = i & 0x000f;
		palette.set_pen_color(i, r << 4, g << 4, b << 4);
	}
}


static void wswan_cart(device_slot_interface &device)
{
	device.option_add_internal("ws_rom",     WS_ROM_STD);
	device.option_add_internal("ws_sram",    WS_ROM_SRAM);
	device.option_add_internal("ws_eeprom",  WS_ROM_EEPROM);
}


void wswan_state::wswan(machine_config &config)
{
	// Basic machine hardware
	V30MZ(config, m_maincpu, 3.072_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &wswan_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &wswan_state::io_map);

	WSWAN_VIDEO(config, m_vdp, 0);
	m_vdp->set_screen("screen");
	m_vdp->set_vdp_type(wswan_video_device::VDP_TYPE_WSWAN);
	m_vdp->set_irq_callback(FUNC(wswan_state::set_irq_line));
	m_vdp->set_dmasnd_callback(FUNC(wswan_state::dma_sound_cb));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
//  screen.set_refresh_rate(75);
//  screen.set_vblank_time(0);
	screen.set_screen_update("vdp", FUNC(wswan_video_device::screen_update));
//  screen.set_size(WSWAN_X_PIXELS, WSWAN_Y_PIXELS);
//  screen.set_visarea(0*8, WSWAN_X_PIXELS - 1, 0, WSWAN_Y_PIXELS - 1);
	screen.set_raw(3.072_MHz_XTAL, 256, 0, wswan_video_device::WSWAN_X_PIXELS, 159, 0, wswan_video_device::WSWAN_Y_PIXELS);
	screen.set_palette("palette");

	config.set_default_layout(layout_wswan);

	config.set_maximum_quantum(attotime::from_hz(60));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	GFXDECODE(config, "gfxdecode", "palette", gfx_wswan);
	PALETTE(config, "palette", FUNC(wswan_state::palette), 16);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	WSWAN_SND(config, m_sound, 3.072_MHz_XTAL);
	m_sound->set_addrmap(0, &wswan_state::snd_map);
	m_sound->add_route(0, "lspeaker", 0.50);
	m_sound->add_route(1, "rspeaker", 0.50);

	// cartridge
	WS_CART_SLOT(config, m_cart, 3.072_MHz_XTAL / 8, wswan_cart, nullptr);

	// software lists
	SOFTWARE_LIST(config, "cart_list").set_original("wswan");
	SOFTWARE_LIST(config, "wsc_list").set_compatible("wscolor");

	SOFTWARE_LIST(config, "pc2_list").set_compatible("pockchalv2");
}


void wscolor_state::wscolor(machine_config &config)
{
	wswan(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &wscolor_state::mem_map);

	m_vdp->set_vdp_type(wswan_video_device::VDP_TYPE_WSC);

	auto &palette(*subdevice<palette_device>("palette"));
	palette.set_entries(4096);
	palette.set_init(FUNC(wscolor_state::palette));

	// software lists
	config.device_remove("wsc_list");
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("wscolor");
	SOFTWARE_LIST(config, "ws_list").set_compatible("wswan");
}


void wswan_state::handle_irqs()
{
	if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_HBLTMR)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_HBLTMR); // V30MZ
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_VBL)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_VBL); // V30MZ
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_VBLTMR)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_VBLTMR); // V30MZ
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_LCMP)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_LCMP); // V30MZ
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_SRX)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_SRX); // V30MZ
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_RTC)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_RTC); // V30MZ
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_KEY)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_KEY); // V30MZ
	}
	else if (m_ws_portram[0xb2] & m_ws_portram[0xb6] & WSWAN_IFLAG_STX)
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_ws_portram[0xb0] + WSWAN_INT_STX); // V30MZ
	}
	else
	{
		m_maincpu->set_input_line(0, CLEAR_LINE);
	}
}


void wswan_state::set_irq_line(int irq)
{
	if (m_ws_portram[0xb2] & irq)
	{
		m_ws_portram[0xb6] |= irq;
		handle_irqs();
	}
}


void wswan_state::dma_sound_cb()
{
	if ((m_sound_dma.enable & 0x88) == 0x80)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		/* TODO: Output sound DMA byte */
		port_w(0x89, space.read_byte(m_sound_dma.source));
		m_sound_dma.size--;
		m_sound_dma.source = (m_sound_dma.source + 1) & 0x0fffff;
		if (m_sound_dma.size == 0)
		{
			m_sound_dma.enable &= 0x7f;
		}
	}
}


void wswan_state::clear_irq_line(int irq)
{
	m_ws_portram[0xb6] &= ~irq;
	handle_irqs();
}


void wswan_state::register_save()
{
	save_item(NAME(m_ws_portram));
	save_item(NAME(m_internal_eeprom));
	save_item(NAME(m_bios_disabled));
	save_item(NAME(m_rotate));

	save_item(NAME(m_sound_dma.source));
	save_item(NAME(m_sound_dma.size));
	save_item(NAME(m_sound_dma.enable));

	if (m_cart->exists())
		m_cart->save_nvram();
}


void wswan_state::common_start()
{
	register_save();

	if (m_cart->exists())
	{
		// ROM
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x20000, 0x2ffff, read8sm_delegate(*m_cart, FUNC(ws_cart_slot_device::read_rom20)));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x30000, 0x3ffff, read8sm_delegate(*m_cart, FUNC(ws_cart_slot_device::read_rom30)));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x40000, 0xeffff, read8sm_delegate(*m_cart, FUNC(ws_cart_slot_device::read_rom40)));

		// SRAM
		if (m_cart->get_type() == WS_SRAM)
		{
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x10000, 0x1ffff, read8sm_delegate(*m_cart, FUNC(ws_cart_slot_device::read_ram)));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0x10000, 0x1ffff, write8sm_delegate(*m_cart, FUNC(ws_cart_slot_device::write_ram)));
		}
	}
}


void wswan_state::machine_start()
{
	common_start();
	subdevice<nvram_device>("nvram")->set_base(m_internal_eeprom, INTERNAL_EEPROM_SIZE_WS * 2);
	m_system_type = TYPE_WSWAN;
}


void wscolor_state::machine_start()
{
	common_start();
	subdevice<nvram_device>("nvram")->set_base(m_internal_eeprom, INTERNAL_EEPROM_SIZE * 2);
	m_system_type = TYPE_WSC;
}


void wswan_state::machine_reset()
{
	m_bios_disabled = 0;

	if (m_cart->exists())
		m_rotate = m_cart->get_is_rotated();
	else
		m_rotate = 0;

	/* Intialize ports */
	std::copy(std::begin(ws_portram_init), std::end(ws_portram_init), std::begin(m_ws_portram));

	render_target *target = machine().render().first_target();
	target->set_view(m_rotate);

	/* Initialize sound DMA */
	m_sound_dma = sound_dma_t();
}


u8 wswan_state::bios_r(offs_t offset)
{
	if (!m_bios_disabled)
		return m_region_maincpu->base()[offset & (m_region_maincpu->bytes() - 1)];
	else
		return m_cart->read_rom40(offset + 0xb0000);
}


u8 wswan_state::port_r(offs_t offset)
{
	u8 value = m_ws_portram[offset];

	if (offset != 2)
		logerror("PC=%X: port read %02X\n", m_maincpu->pc(), offset);

	if (offset < 0x40 || (offset >= 0xa1 && offset < 0xb0))
		return m_vdp->reg_r(offset);

	switch (offset)
	{
		case 0x4a:      // Sound DMA source address (low)
			value = m_sound_dma.source & 0xff;
			break;
		case 0x4b:      // Sound DMA source address (high)
			value = (m_sound_dma.source >> 8) & 0xff;
			break;
		case 0x4c:      // Sound DMA source memory segment
			value = (m_sound_dma.source >> 16) & 0xff;
			break;
		case 0x4e:      // Sound DMA transfer size (low)
			value = m_sound_dma.size & 0xff;
			break;
		case 0x4f:      // Sound DMA transfer size (high)
			value = (m_sound_dma.size >> 8) & 0xff;
			break;
		case 0x52:      // Sound DMA start/stop
			value = m_sound_dma.enable;
			break;
		case 0x60:
			value = m_vdp->reg_r(offset);
			break;
		case 0xa0:      // Hardware type
			// Bit 0 - Disable/enable Bios
			// Bit 1 - Determine mono/color
			// Bit 2 - Unknown, used to determine color/crystal
			// Bit 3 - Unknown
			// Bit 7 - Checked during start up, expects bit 7 set
			value = value & ~ 0x02;
			if (m_system_type == TYPE_WSC)
				value |= 2;
			value |= 0x80;
			break;
		case 0xb5:  // Read controls
			// Bit 0-3 - Current state of input lines (read-only)
			// Bit 4-6 - Select line of inputs to read
			// 001 - Read Y cursors
			// 010 - Read X cursors
			// 100 - Read START,A,B buttons
			// Bit 7   - Unknown
			value = value & 0xf0;
			switch (value)
			{
			case 0x10:  // Read Y cursors: Y1 - Y2 - Y3 - Y4
				{
					u8 input = m_cursy->read();
					if (m_rotate) // reorient controls if the console is rotated
					{
						if (input & 0x01) value |= 0x02;
						if (input & 0x02) value |= 0x04;
						if (input & 0x04) value |= 0x08;
						if (input & 0x08) value |= 0x01;
					}
					else
						value = value | input;
				}
				break;
			case 0x20:  // Read X cursors: X1 - X2 - X3 - X4
				{
					u8 input = m_cursx->read();
					if (m_rotate) // reorient controls if the console is rotated
					{
						if (input & 0x01) value |= 0x02;
						if (input & 0x02) value |= 0x04;
						if (input & 0x04) value |= 0x08;
						if (input & 0x08) value |= 0x01;
					}
					else
						value = value | input;
				}
				break;
			case 0x40:  // Read buttons: START - A - B
				value = value | m_buttons->read();
				break;
			}
			break;
		case 0xc0:
		case 0xc1:
		case 0xc2:
		case 0xc3:
		case 0xc4:  // Cartridge EEPROM data
		case 0xc5:  // Cartridge EEPROM data
		case 0xc6:
		case 0xc7:
		case 0xc8:
		case 0xc9:
		case 0xca:
		case 0xcb:  // RTC data
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xcf:
			value = m_cart->read_io(offset & 0x0f);
			break;
	}

	return value;
}


void wswan_state::port_w(offs_t offset, u8 data)
{
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	logerror("PC=%X: port write %02X <- %02X\n", m_maincpu->pc(), offset, data);

	if (offset < 0x40 || (offset >= 0xa1 && offset < 0xb0))
	{
		m_vdp->reg_w(offset, data);
		return;
	}

	switch (offset)
	{
		case 0x40:  // DMA source address (low)
		case 0x41:  // DMA source address (high)
		case 0x42:  // DMA source bank
		case 0x43:  // DMA destination bank
		case 0x44:  // DMA destination address (low)
		case 0x45:  // DMA destination address (high)
		case 0x46:  // Size of copied data (low)
		case 0x47:  // Size of copied data (high)
			break;
		case 0x48:  // DMA control
			// Bit 0-6 - Unknown
			// Bit 7   - DMA stop/start
			if (data & 0x80)
			{
				u32 src, dst;
				u16 length;

				src = m_ws_portram[0x40] + (m_ws_portram[0x41] << 8) + (m_ws_portram[0x42] << 16);
				dst = m_ws_portram[0x44] + (m_ws_portram[0x45] << 8) + (m_ws_portram[0x43] << 16);
				length = m_ws_portram[0x46] + (m_ws_portram[0x47] << 8);
				for ( ; length > 0; length--)
				{
					mem.write_byte(dst, mem.read_byte(src));
					src++;
					dst++;
				}
#ifdef MAME_DEBUG
				logerror("DMA  src:%X dst:%X length:%d\n", src, dst, length);
#endif
				m_ws_portram[0x40] = src & 0xff;
				m_ws_portram[0x41] = (src >> 8) & 0xff;
				m_ws_portram[0x44] = dst & 0xff;
				m_ws_portram[0x45] = (dst >> 8) & 0xff;
				m_ws_portram[0x46] = length & 0xff;
				m_ws_portram[0x47] = (length >> 8) & 0xff;
				data &= 0x7f;
			}
			break;
		case 0x4a:  // Sound DMA source address (low)
			m_sound_dma.source = (m_sound_dma.source & 0x0fff00) | data;
			break;
		case 0x4b:  // Sound DMA source address (high)
			m_sound_dma.source = (m_sound_dma.source & 0x0f00ff) | (data << 8);
			break;
		case 0x4c:  // Sound DMA source memory segment
			// Bit 0-3 - Sound DMA source address segment
			// Bit 4-7 - Unknown
			m_sound_dma.source = (m_sound_dma.source & 0xffff) | ((data & 0x0f) << 16);
			break;
		case 0x4d:  // Unknown
			break;
		case 0x4e:  // Sound DMA transfer size (low)
			m_sound_dma.size = (m_sound_dma.size & 0xff00) | data;
			break;
		case 0x4f:  // Sound DMA transfer size (high)
			m_sound_dma.size = (m_sound_dma.size & 0xff) | (data << 8);
			break;
		case 0x50:  // Unknown
		case 0x51:  // Unknown
			break;
		case 0x52:  // Sound DMA start/stop
			// Bit 0-6 - Unknown
			// Bit 7   - Sound DMA stop/start
			m_sound_dma.enable = data;
			break;
		case 0x60:
			m_vdp->reg_w(offset, data);
			break;
		case 0x80:  // Audio 1 freq (lo)
		case 0x81:  // Audio 1 freq (hi)
		case 0x82:  // Audio 2 freq (lo)
		case 0x83:  // Audio 2 freq (hi)
		case 0x84:  // Audio 3 freq (lo)
		case 0x85:  // Audio 3 freq (hi)
		case 0x86:  // Audio 4 freq (lo)
		case 0x87:  // Audio 4 freq (hi)
		case 0x88:  // Audio 1 volume
			// Bit 0-3 - Right volume audio channel 1
			// Bit 4-7 - Left volume audio channel 1
		case 0x89:  // Audio 2 volume
			// Bit 0-3 - Right volume audio channel 2
			// Bit 4-7 - Left volume audio channel 2
		case 0x8a:  // Audio 3 volume
			// Bit 0-3 - Right volume audio channel 3
			// Bit 4-7 - Left volume audio channel 3
		case 0x8b:  // Audio 4 volume
			// Bit 0-3 - Right volume audio channel 4
			// Bit 4-7 - Left volume audio channel 4
		case 0x8c:  // Sweep step
		case 0x8d:  // Sweep time
		case 0x8e:  // Noise control
			// Bit 0-2 - Noise generator type
			// Bit 3   - Reset
			// Bit 4   - Enable
			// Bit 5-7 - Unknown
		case 0x8f:  // Sample location
			// Bit 0-7 - Sample address location 0 00xxxxxx xx000000
		case 0x90:  // Audio control
			// Bit 0   - Audio 1 enable
			// Bit 1   - Audio 2 enable
			// Bit 2   - Audio 3 enable
			// Bit 3   - Audio 4 enable
			// Bit 4   - Unknown
			// Bit 5   - Audio 2 voice mode enable
			// Bit 6   - Audio 3 sweep mode enable
			//  Bit 7   - Audio 4 noise mode enable
		case 0x91:  // Audio output
			// Bit 0   - Mono select
			// Bit 1-2 - Output volume
			// Bit 3   - External stereo
			// Bit 4-6 - Unknown
			// Bit 7   - External speaker (Read-only, set by hardware)
		case 0x92:  // Noise counter shift register (lo)
		case 0x93:  // Noise counter shift register (hi)
			// Bit 0-6 - Noise counter shift register bit 8-14
			// bit 7   - Unknown
		case 0x94:  // Master volume
			// Bit 0-3 - Master volume
			// Bit 4-7 - Unknown
			m_sound->port_w(offset, data);
			break;
		case 0x9E:  // WSC volume setting (0, 1, 2, 3)
			break;
		case 0xa0:  // Hardware type/system control
			// Bit 0   - Enable cartridge slot and/or disable bios
			// Bit 1   - Hardware type: 0 = WS, 1 = WSC
			// Bit 2   - Unknown, written during boot
			// Bit 3   - Unknown, written during boot
			// Bit 4-6 - Unknown
			// Bit 7   - Unknown, read during boot
			if ((data & 0x01) && !m_bios_disabled)
				m_bios_disabled = 1;
			break;

		case 0xb0:  // Interrupt base vector
			break;
		case 0xb1:  // Communication byte
			break;
		case 0xb2:  // Interrupt enable
			// Bit 0   - Serial transmit interrupt enable
			// Bit 1   - Key press interrupt enable
			// Bit 2   - RTC alarm interrupt enable
			// Bit 3   - Serial receive interrupt enable
			// Bit 4   - Drawing line detection interrupt enable
			// Bit 5   - VBlank timer interrupt enable
			// Bit 6   - VBlank interrupt enable
			// Bit 7   - HBlank timer interrupt enable
			break;
		case 0xb3:  // serial communication control
			// Bit 0   - Receive complete
			// Bit 1   - Error
			// Bit 2   - Send complete
			// Bit 3-4 - Unknown
			// Bit 5   - Send data interrupt generation
			// Bit 6   - Connection speed: 0 = 9600 bps, 1 = 38400 bps
			// Bit 7   - Receive data interrupt generation
			m_ws_portram[0xb1] = 0xff;
			if (data & 0x80)
			{
				//              m_ws_portram[0xb1] = 0x00;
				data |= 0x04;
			}
			if (data & 0x20)
			{
				//              data |= 0x01;
			}
			break;
		case 0xb5:  // Read controls
			// Bit 0-3 - Current state of input lines (read-only)
			// Bit 4-6 - Select line of inputs to read
			// 001 - Read Y cursors
			// 010 - Read X cursors
			// 100 - Read START,A,B buttons
			// Bit 7   - Unknown
			break;
		case 0xb6:  // Interrupt acknowledge
			// Bit 0   - Serial transmit interrupt acknowledge
			// Bit 1   - Key press interrupt acknowledge
			// Bit 2   - RTC alarm interrupt acknowledge
			// Bit 3   - Serial receive interrupt acknowledge
			// Bit 4   - Drawing line detection interrupt acknowledge
			// Bit 5   - VBlank timer interrupt acknowledge
			// Bit 6   - VBlank interrupt acknowledge
			// Bit 7   - HBlank timer interrupt acknowledge
			clear_irq_line(data);
			data = m_ws_portram[0xb6];
			break;
		case 0xba:  // Internal EEPROM data (low)
		case 0xbb:  // Internal EEPROM data (high)
			break;
		case 0xbc:  // Internal EEPROM address (low)
			// (WS) Bit 0-5 = Internal EEPROM address
			// (WSC) Bit 0-7 - Internal EEPROM address bit 1-8
		case 0xbd:  // Internal EEPROM address (high)
			// (WSC) Bit 0   - Internal EEPROM address bit 9(?)
			// Bit 1-7 - Unknown
			break;
		case 0xbe:  // Internal EEPROM command/status
			// Bit 0   - Read complete (read only)
			// Bit 1   - Write complete (read only)
			// Bit 2-3 - Unknown
			// Bit 4   - Read
			// Bit 5   - Write
			// Bit 6   - Protect
			// Bit 7   - Initialize
			if (data & 0x20)
			{
				u16 addr = get_internal_eeprom_address();
				m_internal_eeprom[addr] = m_ws_portram[0xba];
				m_internal_eeprom[addr + 1] = m_ws_portram[0xbb];
				data |= 0x02;
			}
			else if ( data & 0x10 )
			{
				u16 addr = get_internal_eeprom_address();
				m_ws_portram[0xba] = m_internal_eeprom[addr];
				m_ws_portram[0xbb] = m_internal_eeprom[addr + 1];
				data |= 0x01;
			}
			else
			{
				logerror( "Unsupported internal EEPROM command: %X\n", data );
			}
			break;
		case 0xc0:  // ROM bank $40000-$fffff
		case 0xc1:  // SRAM bank
		case 0xc2:  // ROM bank $20000-$2ffff
		case 0xc3:  // ROM bank $30000-$3ffff
		case 0xc4:
		case 0xc5:
		case 0xc6:  // EEPROM address / command
		case 0xc7:  // EEPROM address / command
		case 0xc8:  // EEPROM command
		case 0xc9:
		case 0xca:  // RTC command
		case 0xcb:  // RTC data
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xcf:
			m_cart->write_io(offset & 0x0f, data);
			break;
		default:
			logerror( "Write to unsupported port: %X - %X\n", offset, data );
			break;
	}

	// Update the port value
	m_ws_portram[offset] = data;
}


u16 wswan_state::get_internal_eeprom_address() {
	return (m_ws_portram[0xbc] & 0x3f) << 1;
}


u16 wscolor_state::get_internal_eeprom_address() {
	return (((m_ws_portram[0xbd] << 8) | m_ws_portram[0xbc]) & 0x1FF) << 1;
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(wswan)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("boot.rom", 0x0000, 0x1000, CRC(7f35f890) SHA1(4015bcacea76bb0b5bbdb13c5358f7e1abb986a1))

	ROM_REGION(0x80, "nvram", 0)
	// Need a dump from an original new unit
	// Empty file containing just the name 'WONDERSAN'
	ROM_LOAD("internal_eeprom.ws", 0x00, 0x80, BAD_DUMP CRC(b1dff316) SHA1(7b76c3d59c9add9501f95e8bfc34427773fcbd28))
ROM_END

ROM_START(wscolor)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("boot.rom", 0x0000, 0x2000, CRC(cb06d9c3) SHA1(c5ad0b8af45d762662a69f50b64161b9c8919efb))

	ROM_REGION(0x800, "nvram", 0)
	// Need a dump from an original new unit
	// Empty file containing just the name 'WONDERSAN'
	ROM_LOAD("internal_eeprom.wsc", 0x000, 0x800, BAD_DUMP CRC(9e29725c) SHA1(a903c2cb5f4bb94b67326ff87a2d91605dceffff))
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS          INIT        COMPANY   FULLNAME
CONS( 1999, wswan,   0,      0,      wswan,   wswan, wswan_state,   empty_init, "Bandai", "WonderSwan",       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
CONS( 2000, wscolor, wswan,  0,      wscolor, wswan, wscolor_state, empty_init, "Bandai", "WonderSwan Color", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
