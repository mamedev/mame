// license:BSD-3-Clause
// copyright-holders:Anthony Kruize,Wilbert Pol
/***************************************************************************

  wswan.cpp

  Driver file to handle emulation of the Bandai WonderSwan
  By:

  Anthony Kruize
  Wilbert Pol

  Based on the WStech documentation by Judge and Dox.

  These systems were developed by Koto Laboratory

  Usage:
    Keep START button pressed during startup (or reset) to enter the internal
    configuration menu.

  Known issues/TODOs:
  - Perform video DMA at proper timing.
  - Add (real/proper) RTC support.
  - Fix wonderwitch
    - Make the flash rom changes save.

***************************************************************************/

#include "emu.h"
#include "wswan_v.h"

#include "wswansound.h"

#include "cpu/v30mz/v30mz.h"
#include "machine/nvram.h"
#include "bus/wswan/slot.h"
#include "bus/wswan/rom.h"
#include "render.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "wswan.lh"

#include <algorithm>

#define LOG_UNKNOWN (1 << 1)
#define LOG_EEPROM  (1 << 2)
#define LOG_DMA     (1 << 3)

#define LOG_ALL     (LOG_UNKNOWN | LOG_EEPROM | LOG_DMA)

#define VERBOSE     (0)

#include "logmacro.h"

#define LOGUNKNOWN(...) LOGMASKED(LOG_UNKNOWN, __VA_ARGS__)
#define LOGEEPROM(...)  LOGMASKED(LOG_EEPROM, __VA_ARGS__)
#define LOGDMA(...)     LOGMASKED(LOG_DMA, __VA_ARGS__)

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
		m_buttons(*this, "BUTTONS"),
		m_sound_output(*this, "SOUND_OUTPUT"),
		m_icons(*this, "icon%u", 0U)
	{ }

	void wswan(machine_config &config);
	void pockchv2(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(sound_output_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void wswan_base(machine_config &config);

	// Interrupt flags
	static constexpr u8 WSWAN_IFLAG_STX    = 0x01;
	static constexpr u8 WSWAN_IFLAG_KEY    = 0x02;
	static constexpr u8 WSWAN_IFLAG_RTC    = 0x04;
	static constexpr u8 WSWAN_IFLAG_SRX    = 0x08;
	static constexpr u8 WSWAN_IFLAG_LCMP   = 0x10;
	static constexpr u8 WSWAN_IFLAG_VBLTMR = 0x20;
	static constexpr u8 WSWAN_IFLAG_VBL    = 0x40;
	static constexpr u8 WSWAN_IFLAG_HBLTMR = 0x80;

	// Interrupts
	static constexpr u8 WSWAN_INT_STX    = 0;
	static constexpr u8 WSWAN_INT_KEY    = 1;
	static constexpr u8 WSWAN_INT_RTC    = 2;
	static constexpr u8 WSWAN_INT_SRX    = 3;
	static constexpr u8 WSWAN_INT_LCMP   = 4;
	static constexpr u8 WSWAN_INT_VBLTMR = 5;
	static constexpr u8 WSWAN_INT_VBL    = 6;
	static constexpr u8 WSWAN_INT_HBLTMR = 7;

	static constexpr u32 INTERNAL_EEPROM_SIZE = 1024;   // 16kbit on WSC
	static constexpr u32 INTERNAL_EEPROM_SIZE_WS = 64;  // 1kbit on WS

	// Labeled 12.3FXA on wonderswan color pcb
	static constexpr XTAL X1 = 12.288_MHz_XTAL;

	enum enum_system { TYPE_WSWAN = 0, TYPE_WSC };

	required_device<v30mz_cpu_device> m_maincpu;
	required_device<wswan_video_device> m_vdp;
	required_device<wswan_sound_device> m_sound;
	required_device<ws_cart_slot_device> m_cart;

	required_memory_region m_region_maincpu;
	required_ioport m_cursx;
	required_ioport m_cursy;
	required_ioport m_buttons;
	required_ioport m_sound_output;
	output_finder<6> m_icons;

	u16 m_ws_portram[128] = { };
	u8 m_internal_eeprom[INTERNAL_EEPROM_SIZE * 2] = { };
	u8 m_system_type = 0;
	bool m_bios_disabled = false;
	u8 m_rotate = 0;
	u32 m_vector = 0;
	u8 m_sys_control = 0;
	u8 m_irq_vector_base = 0;
	u8 m_serial_data = 0;
	u8 m_serial_control = 0;
	u8 m_irq_enable = 0;
	u8 m_irq_active = 0;
	u16 m_internal_eeprom_data = 0;
	u16 m_internal_eeprom_address = 0;
	u8 m_internal_eeprom_command = 0;
	u8 m_keypad = 0;

	u16 bios_r(offs_t offset, u16 mem_mask);
	u16 port_r(offs_t offset, u16 mem_mask);
	void port_w(offs_t offset, u16 data, u16 mem_mask);

	void set_irq_line(int irq);
	void common_start();

	void handle_irqs();
	void clear_irq_line(int irq);
	virtual u16 get_internal_eeprom_address();
	u32 get_vector() { return m_vector; }
	void set_icons(u8 data);
	void set_rotate_view();

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void snd_map(address_map &map) ATTR_COLD;
};


class wscolor_state : public wswan_state
{
public:
	wscolor_state(const machine_config &mconfig, device_type type, const char *tag) :
		wswan_state(mconfig, type, tag),
		m_dma_view(*this, "dma_view"),
		m_hypervoice_view(*this, "hypervoice_view")
	{ }

	void wscolor(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual u16 get_internal_eeprom_address() override;

private:
	static constexpr u8 SOUND_DMA_DIV[4] = { 6, 4, 2, 1 };

	memory_view m_dma_view;
	memory_view m_hypervoice_view;

	struct sound_dma_t
	{
		emu_timer *timer = nullptr;  // Timer
		u32       source = 0;        // Source address
		u32       source_reload = 0; // Source address, Value for reload
		u32       size = 0;          // Size
		u32       size_reload = 0;   // Size, Value for reload
		u8        control = 0;       // Control
	};
	sound_dma_t m_sound_dma;
	u16 m_dma_source_offset = 0;
	u16 m_dma_source_segment = 0;
	u16 m_dma_destination = 0;
	u16 m_dma_length = 0;
	u16 m_dma_control = 0;

	u16 dma_r(offs_t offset, u16 mem_mask);
	void dma_w(offs_t offset, u16 data, u16 mem_mask);
	void color_mode_view_w(int state);

	TIMER_CALLBACK_MEMBER(sound_dma_cb);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
};


void wswan_state::mem_map(address_map &map)
{
	map(0x00000, 0x03fff).rw(m_vdp, FUNC(wswan_video_device::vram_r), FUNC(wswan_video_device::vram_w));       // 16kb RAM / 4 colour tiles
	map(0x04000, 0x0ffff).noprw();       // nothing
	map(0xf0000, 0xfffff).r(FUNC(wswan_state::bios_r));
}


void wscolor_state::mem_map(address_map &map)
{
	map(0x00000, 0x0ffff).rw(m_vdp, FUNC(wswan_video_device::vram_r), FUNC(wswan_video_device::vram_w));       // 16/64kb RAM / 4 colour tiles, 16 colour tiles + palettes
	map(0xf0000, 0xfffff).r(FUNC(wscolor_state::bios_r));
}


void wswan_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(wswan_state::port_r), FUNC(wswan_state::port_w));   // I/O ports
}


void wscolor_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(wscolor_state::port_r), FUNC(wscolor_state::port_w));   // I/O ports
	map(0x40, 0x53).view(m_dma_view);
	m_dma_view[0](0x40, 0x53).rw(FUNC(wscolor_state::dma_r), FUNC(wscolor_state::dma_w));
	map(0x64, 0x6b).view(m_hypervoice_view);
	m_hypervoice_view[0](0x64, 0x6b).rw(m_sound, FUNC(wswan_sound_device::hypervoice_r), FUNC(wswan_sound_device::hypervoice_w));
}


void wswan_state::snd_map(address_map &map)
{
	map(0x00000, 0x03fff).r(m_vdp, FUNC(wswan_video_device::vram_r));
}


static INPUT_PORTS_START(wswan)
	PORT_START("CURSX")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("X4 - Left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("X3 - Down")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("X2 - Right")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("X1 - Up")

	PORT_START("BUTTONS")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Button B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Button A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Start")

	PORT_START("CURSY")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Y4 - Left") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Y3 - Down") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Y2 - Right") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Y1 - Up") PORT_CODE(KEYCODE_W)

	PORT_START("SOUND_OUTPUT")
	PORT_CONFNAME(    0x01, 0x01, "Sound output select" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(wswan_state::sound_output_changed), 0)
	PORT_CONFSETTING( 0x00, "Internal speaker (Mono)" )
	PORT_CONFSETTING( 0x01, "External headphone (Stereo)" )
INPUT_PORTS_END


static void wswan_cart(device_slot_interface &device)
{
	device.option_add_internal("ws_rom",     WS_ROM_STD);
	device.option_add_internal("ws_sram",    WS_ROM_SRAM);
	device.option_add_internal("ws_eeprom",  WS_ROM_EEPROM);
	device.option_add_internal("wwitch",     WS_ROM_WWITCH);
}


void wswan_state::wswan_base(machine_config &config)
{
	// Basic machine hardware
	V30MZ(config, m_maincpu, X1 / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &wswan_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &wswan_state::io_map);
	m_maincpu->vector_cb().set(FUNC(wswan_state::get_vector));

	WSWAN_VIDEO(config, m_vdp, X1 / 4);
	m_vdp->set_screen("screen");
	m_vdp->set_irq_callback(FUNC(wswan_state::set_irq_line));
	m_vdp->icons_cb().set(FUNC(wswan_state::set_icons));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_screen_update("vdp", FUNC(wswan_video_device::screen_update));
	screen.set_raw(X1 / 4, 256, 0, wswan_video_device::WSWAN_X_PIXELS, 159, 0, wswan_video_device::WSWAN_Y_PIXELS);
	screen.set_palette("vdp");

	config.set_default_layout(layout_wswan);

	config.set_maximum_quantum(attotime::from_hz(60));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();
	WSWAN_SND(config, m_sound, X1 / 4);
	m_sound->set_addrmap(0, &wswan_state::snd_map);
	m_sound->add_route(0, "speaker", 0.50, 0);
	m_sound->add_route(1, "speaker", 0.50, 1);

	// cartridge
	WS_CART_SLOT(config, m_cart, X1 / 32, wswan_cart, nullptr);


}

void wswan_state::wswan(machine_config &config)
{
	wswan_base(config);

	// software lists
	SOFTWARE_LIST(config, "cart_list").set_original("wswan");
	SOFTWARE_LIST(config, "wsc_list").set_compatible("wscolor");
}

// while the pc2 software is compatible with a Wonderswan, the physical cartridges are not
void wswan_state::pockchv2(machine_config &config)
{
	wswan_base(config);

	// software lists
	SOFTWARE_LIST(config, "pc2_list").set_original("pockchalv2");
}

void wscolor_state::wscolor(machine_config &config)
{
	wswan(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &wscolor_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &wscolor_state::io_map);

	WSWAN_COLOR_VIDEO(config.replace(), m_vdp, X1 / 4);
	m_vdp->set_screen("screen");
	m_vdp->set_irq_callback(FUNC(wscolor_state::set_irq_line));
	m_vdp->icons_cb().set(FUNC(wscolor_state::set_icons));
	m_vdp->color_mode_cb().set(FUNC(wscolor_state::color_mode_view_w));

	// software lists
	config.device_remove("wsc_list");
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("wscolor");
	SOFTWARE_LIST(config, "ws_list").set_compatible("wswan");

	m_cart->set_must_be_loaded(true);
}

INPUT_CHANGED_MEMBER(wswan_state::sound_output_changed)
{
	m_sound->set_headphone_connected(BIT(m_sound_output->read(), 0));
}

void wswan_state::handle_irqs()
{
	bool set_irq_line = false;
	if (m_irq_enable & m_irq_active & WSWAN_IFLAG_HBLTMR)
	{
		m_vector = m_irq_vector_base + WSWAN_INT_HBLTMR;
		set_irq_line = true;
	}
	else if (m_irq_enable & m_irq_active & WSWAN_IFLAG_VBL)
	{
		m_vector = m_irq_vector_base + WSWAN_INT_VBL;
		set_irq_line = true;
	}
	else if (m_irq_enable & m_irq_active & WSWAN_IFLAG_VBLTMR)
	{
		m_vector = m_irq_vector_base + WSWAN_INT_VBLTMR;
		set_irq_line = true;
	}
	else if (m_irq_enable & m_irq_active & WSWAN_IFLAG_LCMP)
	{
		m_vector = m_irq_vector_base + WSWAN_INT_LCMP;
		set_irq_line = true;
	}
	else if (m_irq_enable & m_irq_active & WSWAN_IFLAG_SRX)
	{
		m_vector = m_irq_vector_base + WSWAN_INT_SRX;
		set_irq_line = true;
	}
	else if (m_irq_enable & m_irq_active & WSWAN_IFLAG_RTC)
	{
		m_vector = m_irq_vector_base + WSWAN_INT_RTC;
		set_irq_line = true;
	}
	else if (m_irq_enable & m_irq_active & WSWAN_IFLAG_KEY)
	{
		m_vector = m_irq_vector_base + WSWAN_INT_KEY;
		set_irq_line = true;
	}
	else if (m_irq_enable & m_irq_active & WSWAN_IFLAG_STX)
	{
		m_vector = m_irq_vector_base + WSWAN_INT_STX;
		set_irq_line = true;
	}
	m_maincpu->set_input_line(0, set_irq_line ? ASSERT_LINE : CLEAR_LINE);
}


void wswan_state::set_irq_line(int irq)
{
	if (m_irq_enable & irq)
	{
		m_irq_active |= irq;
		handle_irqs();
	}
}


TIMER_CALLBACK_MEMBER(wscolor_state::sound_dma_cb)
{
	if (BIT(m_sound_dma.control, 7))
	{
		if (BIT(m_sound_dma.control, 2))
		{
			// Sound DMA hold
			if (BIT(m_sound_dma.control, 4))
				m_sound->hypervoice_dma_w(0);
			else
				port_w(0x88 / 2, 0 << 8, 0xff00);
		}
		else
		{
			address_space &space = m_maincpu->space(AS_PROGRAM);
			/* TODO: Output sound DMA byte */
			if (BIT(m_sound_dma.control, 4))
				m_sound->hypervoice_dma_w(space.read_byte(m_sound_dma.source));
			else
				port_w(0x88 / 2, space.read_byte(m_sound_dma.source) << 8, 0xff00);
			m_sound_dma.size--;
			m_sound_dma.source = (m_sound_dma.source + (BIT(m_sound_dma.control, 6) ? -1 : 1)) & 0x0fffff;
			if (m_sound_dma.size == 0)
			{
				if (BIT(m_sound_dma.control, 3))
				{
					m_sound_dma.source = m_sound_dma.source_reload;
					m_sound_dma.size = m_sound_dma.size_reload;
				}
				else
				{
					m_sound_dma.control &= 0x7f;
					m_sound_dma.timer->adjust(attotime::never);
					return;
				}
			}
		}
		m_sound_dma.timer->adjust(attotime::from_ticks(SOUND_DMA_DIV[m_sound_dma.control & 3], X1 / 512));
	}
}


void wswan_state::clear_irq_line(int irq)
{
	m_irq_active &= ~irq;
	handle_irqs();
}


void wswan_state::common_start()
{
	if (m_cart->exists())
		m_cart->save_nvram();

	m_icons.resolve();

	if (m_cart->exists())
	{
		// ROM
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x20000, 0x2ffff, read16s_delegate(*m_cart, FUNC(ws_cart_slot_device::read_rom20)));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x30000, 0x3ffff, read16s_delegate(*m_cart, FUNC(ws_cart_slot_device::read_rom30)));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x40000, 0xeffff, read16s_delegate(*m_cart, FUNC(ws_cart_slot_device::read_rom40)));

		// SRAM
		if (m_cart->get_type() == WS_SRAM || m_cart->get_type() == WWITCH)
		{
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x10000, 0x1ffff, read16s_delegate(*m_cart, FUNC(ws_cart_slot_device::read_ram)));
			m_maincpu->space(AS_PROGRAM).install_write_handler(0x10000, 0x1ffff, write16s_delegate(*m_cart, FUNC(ws_cart_slot_device::write_ram)));
		}
	}

	save_item(NAME(m_ws_portram));
	save_item(NAME(m_internal_eeprom));
	save_item(NAME(m_bios_disabled));
	save_item(NAME(m_rotate));

	save_item(NAME(m_vector));

	save_item(NAME(m_sys_control));
	save_item(NAME(m_irq_vector_base));
	save_item(NAME(m_serial_data));
	save_item(NAME(m_serial_control));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_active));
	save_item(NAME(m_internal_eeprom_data));
	save_item(NAME(m_internal_eeprom_address));
	save_item(NAME(m_internal_eeprom_command));
	save_item(NAME(m_keypad));
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

	m_sound_dma.timer = timer_alloc(FUNC(wscolor_state::sound_dma_cb), this);
	save_item(NAME(m_sound_dma.source));
	save_item(NAME(m_sound_dma.source_reload));
	save_item(NAME(m_sound_dma.size));
	save_item(NAME(m_sound_dma.size_reload));
	save_item(NAME(m_sound_dma.control));

	save_item(NAME(m_dma_source_offset));
	save_item(NAME(m_dma_source_segment));
	save_item(NAME(m_dma_destination));
	save_item(NAME(m_dma_length));
	save_item(NAME(m_dma_control));
}


void wswan_state::machine_reset()
{
	m_sound->set_headphone_connected(BIT(m_sound_output->read(), 0));
	m_bios_disabled = false;

	m_rotate = 0;

	m_vector = 0;
	m_irq_vector_base = 0;
	m_serial_control = 0;
	m_irq_enable = 0;
	m_irq_active = 0;
	m_internal_eeprom_data = 0;
	m_internal_eeprom_address = 0;
	m_internal_eeprom_command = 0;
	m_sys_control = (m_system_type == TYPE_WSC) ? 2 : 0;

	/* Intialize ports */
	std::fill(std::begin(m_ws_portram), std::end(m_ws_portram), 0);

	set_rotate_view();
}

void wscolor_state::machine_reset()
{
	wswan_state::machine_reset();

	m_dma_view.disable();
	m_hypervoice_view.disable();

	/* Initialize sound DMA */
	m_sound_dma.timer->adjust(attotime::never);
	m_sound_dma.source = m_sound_dma.source_reload = 0;
	m_sound_dma.size = m_sound_dma.size_reload = 0;
	m_sound_dma.control = 0;
}


void wscolor_state::color_mode_view_w(int state)
{
	if (state)
	{
		m_dma_view.select(0);
		m_hypervoice_view.select(0);
	}
	else
	{
		m_dma_view.disable();
		m_hypervoice_view.disable();
	}
}


u16 wswan_state::bios_r(offs_t offset, u16 mem_mask)
{
	if (!m_bios_disabled)
		return m_region_maincpu->as_u16(offset & ((m_region_maincpu->bytes() >> 1) - 1));
	else
		return m_cart->read_rom40(offset + (0xb0000 >> 1), mem_mask);
}


u16 wswan_state::port_r(offs_t offset, u16 mem_mask)
{
	u16 value = m_ws_portram[offset];

	if (offset < 0x40 / 2 || (offset > 0xa0 / 2 && offset < 0xb0 / 2))
	{
		return m_vdp->reg_r(offset, mem_mask);
	}
	if (offset >= 0x80 / 2 && offset <= 0x9f / 2)
	{
		return m_sound->port_r(offset, mem_mask);
	}

	switch (offset)
	{
		case 0x60 / 2:
			return m_vdp->reg_r(offset, mem_mask);
		case 0xa0 / 2:
			// Hardware type
			// Bit 0 - Disable/enable BIOS
			// Bit 1 - Determine monochrome/color
			// Bit 2 - Unknown, used to determine color/crystal
			// Bit 3 - Unknown
			// Bit 7 - Checked during start up, expects bit 7 set (part of cart unlock sequence?)
			return m_sys_control | 0x80;
		case 0xb0 / 2:
			return m_irq_vector_base | (m_serial_data << 8);
		case 0xb2 / 2:
			return m_irq_enable | (m_serial_control << 8);
		case 0xb4 / 2:
		  // Read controls
			// Bit 8-11  - Current state of input lines (read-only)
			// Bit 12-14 - Select line of inputs to read
			//       001 - Read Y cursors
			//       010 - Read X cursors
			//       100 - Read START,A,B buttons
			// Bit 15    - Unknown
			value = (m_keypad << 8) & 0xf0ff;
			switch (m_keypad & 0x70)
			{
			case 0x10:  // Read Y cursors: Y1 - Y2 - Y3 - Y4
				{
					u8 const input = m_cursy->read();
					if (m_rotate) // reorient controls if the console is rotated
					{
						if (BIT(input, 0)) value |= 0x0200;
						if (BIT(input, 1)) value |= 0x0400;
						if (BIT(input, 2)) value |= 0x0800;
						if (BIT(input, 3)) value |= 0x0100;
					}
					else
						value = value | (input << 8);
				}
				break;
			case 0x20:  // Read X cursors: X1 - X2 - X3 - X4
				{
					u8 const input = m_cursx->read();
					if (m_rotate) // reorient controls if the console is rotated
					{
						if (BIT(input, 0)) value |= 0x0200;
						if (BIT(input, 1)) value |= 0x0400;
						if (BIT(input, 2)) value |= 0x0800;
						if (BIT(input, 3)) value |= 0x0100;
					}
					else
						value = value | (input << 8);
				}
				break;
			case 0x40:  // Read buttons: START - A - B
				value = value | (m_buttons->read() << 8);
				break;
			}
			return value;
		case 0xb6 / 2:
			return m_irq_active;
		case 0xba / 2:
			return m_internal_eeprom_data;
		case 0xbc / 2:
			return m_internal_eeprom_address;
		case 0xbe / 2:
			return m_internal_eeprom_command;
		case 0xc0 / 2:
		case 0xc2 / 2:
		case 0xc4 / 2:  // Cartridge EEPROM data
		case 0xc6 / 2:
		case 0xc8 / 2:
		case 0xca / 2:  // RTC command & data
		case 0xcc / 2:
		case 0xce / 2:
			return m_cart->read_io(offset, mem_mask);
		default:
			if (!machine().side_effects_disabled())
				LOGUNKNOWN("%s: Read from unsupported port: %02x & %04x", machine().describe_context(), offset << 1, mem_mask);
			break;
	}

	return value;
}


void wswan_state::port_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset < 0x40 / 2 || (offset > 0xa0 / 2 && offset < 0xb0 / 2))
	{
		m_vdp->reg_w(offset, data, mem_mask);
		return;
	}

	switch (offset)
	{
		case 0x60 / 2:
			m_vdp->reg_w(offset, data, mem_mask);
			break;
		case 0x80 / 2:  // Audio 1 freq
		case 0x82 / 2:  // Audio 2 freq
		case 0x84 / 2:  // Audio 3 freq
		case 0x86 / 2:  // Audio 4 freq
		case 0x88 / 2:
			// Audio 1 volume
			// Bit 0-3 - Right volume audio channel 1
			// Bit 4-7 - Left volume audio channel 1
			// Audio 2 volume
			// Bit 8-11  - Right volume audio channel 2
			// Bit 12-15 - Left volume audio channel 2
		case 0x8a / 2:
			// Audio 3 volume
			// Bit 0-3 - Right volume audio channel 3
			// Bit 4-7 - Left volume audio channel 3
			// Audio 4 volume
			// Bit 8-11  - Right volume audio channel 4
			// Bit 12-15 - Left volume audio channel 4
		case 0x8c / 2:  // Sweep step / sweep time
		case 0x8e / 2:
			// Noise control
			// Bit 0-2 - Noise generator type
			// Bit 3   - Reset
			// Bit 4   - Enable
			// Bit 5-7 - Unknown
			// Sample location
			// Bit 8-15 - Sample address location 0 00xxxxxx xx000000
		case 0x90 / 2:
			// Audio control
			// Bit 0   - Audio 1 enable
			// Bit 1   - Audio 2 enable
			// Bit 2   - Audio 3 enable
			// Bit 3   - Audio 4 enable
			// Bit 4   - Unknown
			// Bit 5   - Audio 2 voice mode enable
			// Bit 6   - Audio 3 sweep mode enable
			// Bit 7   - Audio 4 noise mode enable
			// Audio output
			// Bit 8     - Mono select
			// Bit 9-10  - Output volume
			// Bit 11    - External stereo
			// Bit 12-14 - Unknown
			// Bit 15    - External speaker (Read-only, set by hardware)
		case 0x92 / 2:  // Noise counter shift register
		case 0x94 / 2:
			// Master volume
			// Bit 0-3 - Master volume
			// Bit 4-7 - Unknown
		case 0x9e / 2:  // WSC volume setting (0, 1, 2, 3) (TODO)
			m_sound->port_w(offset, data, mem_mask);
			break;
		case 0xa0 / 2:
			// Hardware type/system control
			// Bit 0   - Disable bios
			// Bit 1   - Hardware type: 0 = WS, 1 = WSC
			// Bit 2   - External bus width
			// Bit 3   - Cart ROM cycles (0 = 3 cycles, 1 = 1 cycle)
			// Bit 4-6 - Unknown
			// Bit 7   - Unknown, read during boot
			if (ACCESSING_BITS_0_7)
			{
				m_sys_control = (data & 0xfd) | ((m_system_type == TYPE_WSC) ? 2 : 0);
				if (BIT(data, 0) && !m_bios_disabled)
				{
					m_bios_disabled = true;
					if (m_cart->exists())
						m_maincpu->space(AS_PROGRAM).install_read_handler(0x40000, 0xfffff, read16s_delegate(*m_cart, FUNC(ws_cart_slot_device::read_rom40)));
				}
			}
			break;

		case 0xb0 / 2:
			// Interrupt base vector
			if (ACCESSING_BITS_0_7)
				m_irq_vector_base = data & 0xff;
			// Serial data (bit 8-15)
			if (ACCESSING_BITS_8_15)
				m_serial_data = data >> 8;
			break;
		case 0xb2 / 2:
			// Interrupt enable
			// Bit 0   - Serial transmit interrupt enable
			// Bit 1   - Key press interrupt enable
			// Bit 2   - RTC alarm interrupt enable
			// Bit 3   - Serial receive interrupt enable
			// Bit 4   - Drawing line detection interrupt enable
			// Bit 5   - VBlank timer interrupt enable
			// Bit 6   - VBlank interrupt enable
			// Bit 7   - HBlank timer interrupt enable
			if (ACCESSING_BITS_0_7)
				m_irq_enable = data & 0xff;
			// serial communication control
			// Bit 8     - Receive complete
			// Bit 9     - Error
			// Bit 10    - Send complete
			// Bit 11-12 - Unknown
			// Bit 13    - Send data interrupt generation
			// Bit 14    - Connection speed: 0 = 9600 bps, 1 = 38400 bps
			// Bit 15    - Receive data interrupt generation
			if (ACCESSING_BITS_8_15)
			{
				m_serial_data = 0xff;
				m_serial_control = data >> 8;
				if (BIT(m_serial_control, 7))
				{
					//              m_serial_data = 0x00;
					m_serial_control |= 0x04;
				}
				if (BIT(m_serial_control, 5))
				{
					//              m_serial_control |= 0x01;
				}
			}
			break;
		case 0xb4 / 2:
			if (ACCESSING_BITS_8_15)
			{
				m_keypad = (data & 0xf0ff) >> 8;
			}
			break;
		case 0xb6 / 2:
			// Interrupt acknowledge
			// Bit 0 - Serial transmit interrupt acknowledge
			// Bit 1 - Key press interrupt acknowledge
			// Bit 2 - RTC alarm interrupt acknowledge
			// Bit 3 - Serial receive interrupt acknowledge
			// Bit 4 - Drawing line detection interrupt acknowledge
			// Bit 5 - VBlank timer interrupt acknowledge
			// Bit 6 - VBlank interrupt acknowledge
			// Bit 7 - HBlank timer interrupt acknowledge
			if (ACCESSING_BITS_0_7)
			{
				clear_irq_line(data & 0xff);
				data = m_irq_active;
			}
			break;
		case 0xba / 2:  // Internal EEPROM data
			COMBINE_DATA(&m_internal_eeprom_data);
			break;
		case 0xbc / 2:  // Internal EEPROM address
			// (WS) Bit 0-5 - Internal EEPROM address
			// (WSC) Bit 0-8 - Internal EEPROM address bit 1-9
			// Bit 9-15 - Unknown
			COMBINE_DATA(&m_internal_eeprom_address);
			break;
		case 0xbe / 2:
			// Internal EEPROM command/status
			// Bit 0   - Read complete (read only)
			// Bit 1   - Write complete (read only)
			// Bit 2-3 - Unknown
			// Bit 4   - Read
			// Bit 5   - Write
			// Bit 6   - Protect
			// Bit 7   - Initialize
			if (ACCESSING_BITS_0_7)
			{
				m_internal_eeprom_command = data & 0xfc;
				if (BIT(m_internal_eeprom_command, 5))
				{
					u16 const addr = get_internal_eeprom_address();
					m_internal_eeprom[addr] = m_internal_eeprom_data & 0xff;
					m_internal_eeprom[addr + 1] = m_internal_eeprom_data >> 8;
					m_internal_eeprom_command |= 0x02;
				}
				else if (BIT(m_internal_eeprom_command, 4))
				{
					u16 const addr = get_internal_eeprom_address();
					m_internal_eeprom_data = m_internal_eeprom[addr] | (m_internal_eeprom[addr + 1] << 8);
					m_internal_eeprom_command |= 0x01;
				}
				else
				{
					LOGEEPROM("%s: Unsupported internal EEPROM command: %02X\n", machine().describe_context(), data & 0xff);
				}
			}
			break;
		case 0xc0 / 2:  // ROM bank $40000-$fffff and SRAM bank
		case 0xc2 / 2:  // ROM bank $20000-$2ffff and ROM bank $30000-$3ffff
		case 0xc4 / 2:
		case 0xc6 / 2:  // EEPROM address / command
		case 0xc8 / 2:  // EEPROM command
		case 0xca / 2:  // RTC command and RTC data
		case 0xcc / 2:
		case 0xce / 2:
			m_cart->write_io(offset, data, mem_mask);
			break;
		default:
			LOGUNKNOWN("%s: Write to unsupported port: %02x - %04x & %04x\n", machine().describe_context(), offset << 1, data, mem_mask);
			break;
	}

	// Update the port value
	COMBINE_DATA(&m_ws_portram[offset]);
}


u16 wscolor_state::dma_r(offs_t offset, u16 mem_mask)
{
	offset += 0x40 / 2;
	u16 const value = m_ws_portram[offset];

	switch (offset)
	{
		case 0x40 / 2:  // DMA source address
			return m_dma_source_offset;
		case 0x42 / 2:  // DMA source bank/segment
			return m_dma_source_segment;
		case 0x44 / 2:  // DMA destination address
			return m_dma_destination;
		case 0x46 / 2:  // DMA size (in bytes)
			return m_dma_length;
		case 0x48 / 2:  // DMA control
			return m_dma_control;
		case 0x4a / 2:
			// Sound DMA source address
			return m_sound_dma.source & 0xffff;
		case 0x4c / 2:
			// Sound DMA source memory segment
			return (m_sound_dma.source >> 16) & 0xffff;
		case 0x4e / 2:
			// Sound DMA transfer size (low 16 bits)
			return m_sound_dma.size & 0xffff;
		case 0x50 / 2:
			// Sound DMA transfer size (high 4 bits)
			return (m_sound_dma.size >> 16) & 0xffff;
		case 0x52 / 2:
			// Sound DMA control
			return m_sound_dma.control;
		default:
			if (!machine().side_effects_disabled())
				LOGDMA("%s: Read from unknown DMA port: %02x & %04x", machine().describe_context(), offset << 1, mem_mask);
			break;
	}
	return value;
}


void wscolor_state::dma_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset += 0x40 / 2;
	switch (offset)
	{
		case 0x40 / 2:  // DMA source address
			COMBINE_DATA(&m_dma_source_offset);
			m_dma_source_offset &= 0xfffe;
			break;
		case 0x42 / 2:  // DMA source bank/segment
			COMBINE_DATA(&m_dma_source_segment);
			m_dma_source_segment &= 0x000f;
			break;
		case 0x44 / 2:  // DMA destination address
			COMBINE_DATA(&m_dma_destination);
			m_dma_destination &= 0xfffe;
			break;
		case 0x46 / 2:  // DMA size (in bytes)
			COMBINE_DATA(&m_dma_length);
			break;
		case 0x48 / 2:  // DMA control
			// Bit 0-6 - Unknown
			// Bit 7   - DMA stop/start
			if (ACCESSING_BITS_0_7)
			{
				if (BIT(data, 7))
				{
					address_space &mem = m_maincpu->space(AS_PROGRAM);
					u32 src = m_dma_source_offset | (m_dma_source_segment << 16);
					u32 dst = m_dma_destination;
					u16 length = m_dma_length;
					s32 const inc = BIT(data, 6) ? -2 : 2;
					if (length)
						m_maincpu->adjust_icount(-(5 + length));
					for ( ; length > 0; length -= 2)
					{
						mem.write_word(dst, mem.read_word(src));
						src += inc;
						dst += inc;
					}
					m_dma_source_offset = src & 0xffff;
					m_dma_source_segment = src >> 16;
					m_dma_destination = dst & 0xffff;
					m_dma_length = length & 0xffff;
					data &= 0x7f;
					m_dma_control = data;
				}
			}
			break;
		case 0x4a / 2:
			// Sound DMA source address (low)
			if (ACCESSING_BITS_0_7)
			{
				m_sound_dma.source = (m_sound_dma.source & 0x0fff00) | (data & 0xff);
				m_sound_dma.source_reload = (m_sound_dma.source_reload & 0x0fff00) | (data & 0xff);
			}
			// Sound DMA source address (high)
			if (ACCESSING_BITS_8_15)
			{
				m_sound_dma.source = (m_sound_dma.source & 0x0f00ff) | (data & 0xff00);
				m_sound_dma.source_reload = (m_sound_dma.source_reload & 0x0f00ff) | (data & 0xff00);
			}
			break;
		case 0x4c / 2:
			// Sound DMA source memory segment
			// Bit 0-3 - Sound DMA source address segment
			// Bit 4-7 - Unknown
			if (ACCESSING_BITS_0_7)
			{
				m_sound_dma.source = (m_sound_dma.source & 0xffff) | ((data & 0x0f) << 16);
				m_sound_dma.source_reload = (m_sound_dma.source_reload & 0xffff) | ((data & 0x0f) << 16);
			}
			break;
		case 0x4e / 2:
			// Sound DMA transfer size
			// Sound DMA transfer size (bit 0-7)
			if (ACCESSING_BITS_0_7)
			{
				m_sound_dma.size = (m_sound_dma.size & 0x0fff00) | (data & 0xff);
				m_sound_dma.size_reload = (m_sound_dma.size_reload & 0x0fff00) | (data & 0xff);
			}
			// Sound DMA transfer size (bit 8-15)
			if (ACCESSING_BITS_8_15)
			{
				m_sound_dma.size = (m_sound_dma.size & 0x0f00ff) | (data & 0xff00);
				m_sound_dma.size_reload = (m_sound_dma.size_reload & 0x0f00ff) | (data & 0xff00);
			}
			break;
		case 0x50 / 2:
			// Sound DMA transfer size (high 4 bits)
			// Bit 0-3 - Sound DMA transfer size (high 4 bits)
			// Bit 4-7 - Unknown
			if (ACCESSING_BITS_0_7)
			{
				m_sound_dma.size = (m_sound_dma.size & 0xffff) | ((data & 0x0f) << 16);
				m_sound_dma.size_reload = (m_sound_dma.size_reload & 0xffff) | ((data & 0x0f) << 16);
			}
			break;
		case 0x52 / 2:
			// Sound DMA control
			// Bit 0-1 - Sound DMA frequency (4000hz, 6000hz, 12000hz, 24000hz)
			// Bit 2   - Sound DMA hold mode (0 = normal playback, 1 = hold)
			// Bit 3   - Sound DMA repeat mode (0 = one-shot, 1 = auto-repeat)
			// Bit 4   - Sound DMA target (0 = channel 2, 1 = hyper voice)
			// Bit 6   - Sound DMA direction (0 = increment, 1 = decrement)
			// Bit 7   - Sound DMA stop/start
			if (ACCESSING_BITS_0_7)
			{
				m_sound_dma.control = data & 0xff;
				if (BIT(m_sound_dma.control, 7))
					m_sound_dma.timer->adjust(attotime::from_ticks(SOUND_DMA_DIV[m_sound_dma.control & 3], X1 / 512));
				else
					m_sound_dma.timer->adjust(attotime::never);
			}
			break;
		default:
			LOGDMA("%s: Write to unknown DMA port: %x - %x\n", machine().describe_context(), offset, data);
			break;
	}
	// Update the port value
	COMBINE_DATA(&m_ws_portram[offset]);
}


void wswan_state::set_icons(u8 data)
{
	// Bit 0 - LCD sleep icon enable
	// Bit 1 - Vertical position icon enable
	// Bit 2 - Horizontal position icon enable
	// Bit 3 - Dot 1 icon enable
	// Bit 4 - Dot 2 icon enable
	// Bit 5 - Dot 3 icon enable
	for (int i = 0; i < 6; i++)
	{
		m_icons[i] = BIT(data, i);
	}

	u8 const old_rotate = m_rotate;

	if ((!BIT(data, 2) && BIT(data, 1)) || (BIT(data, 2) && !BIT(data, 1)))
	{
		m_rotate = (!BIT(data, 2) && BIT(data, 1)) ? 1 : 0;

		if (old_rotate != m_rotate)
		{
			set_rotate_view();
		}
	}
}


void wswan_state::set_rotate_view()
{
	render_target *target = machine().render().first_target();
	target->set_view(m_rotate);
}


u16 wswan_state::get_internal_eeprom_address()
{
	return (m_internal_eeprom_address & 0x3f) << 1;
}


u16 wscolor_state::get_internal_eeprom_address()
{
	return (m_internal_eeprom_address & 0x1ff) << 1;
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
	// Empty file containing just the name 'WONDERSWANCOLOR' (from Youtube videos)
	ROM_LOAD("internal_eeprom.wsc", 0x000, 0x800, BAD_DUMP CRC(ca11afc9) SHA1(0951845f01f83bee497268a63b5fb7baccfeff7c))
ROM_END

// this currently uses the wswan internal ROMs, the real ones should be different
ROM_START(pockchv2)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("boot.rom", 0x0000, 0x1000, BAD_DUMP CRC(7f35f890) SHA1(4015bcacea76bb0b5bbdb13c5358f7e1abb986a1))

	ROM_REGION(0x80, "nvram", 0)
	// Need a dump from an original new unit
	// Empty file containing just the name 'WONDERSAN'
	ROM_LOAD("internal_eeprom.ws", 0x00, 0x80, BAD_DUMP CRC(b1dff316) SHA1(7b76c3d59c9add9501f95e8bfc34427773fcbd28))
ROM_END

// SwanCrystal has the name 'SWANCRYSTAL' (from Youtube videos)

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS          INIT        COMPANY   FULLNAME
CONS( 1999, wswan,    0,      0,      wswan,    wswan, wswan_state,   empty_init, "Bandai", "WonderSwan",       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
CONS( 2000, wscolor,  wswan,  0,      wscolor,  wswan, wscolor_state, empty_init, "Bandai", "WonderSwan Color", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

CONS( 2002, pockchv2, wswan,  0,      pockchv2, wswan, wswan_state,   empty_init, "Benesse Corporation", "Pocket Challenge V2",       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
