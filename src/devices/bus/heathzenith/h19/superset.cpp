// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB) with the Superset ROM

  Known Issues:
    - Screensaver freezes the screen instead of blanking the screen

****************************************************************************/

#include "emu.h"
#include "superset.h"

#include "tlbbase.h"

#include <algorithm>


namespace bus::heathzenith::h19 {

namespace {

/**
 * Heath TLB with Superset ROM
 */
class heath_superset_tlb_device : public heath_tlb_device
{
public:
	heath_superset_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	virtual MC6845_UPDATE_ROW(crtc_update_row) override;

	void dtr_internal(int data);
	void out1_internal(int data);
	void out2_internal(int data);

	virtual u8   crtc_reg_r(offs_t reg) override;
	virtual void crtc_reg_w(offs_t reg, u8 val) override;

	u8   m_selected_char_set;
	bool m_reverse_video_disabled;
};

static INPUT_PORTS_START( superset )
	PORT_INCLUDE( tlb )

	PORT_MODIFY("SW401")
	PORT_DIPNAME( 0x0f, 0x0c, "Baud Rate")     PORT_DIPLOCATION("SW401:1,2,3,4")
	PORT_DIPSETTING(    0x00, "75")
	PORT_DIPSETTING(    0x01, "110")
	PORT_DIPSETTING(    0x02, "150")
	PORT_DIPSETTING(    0x03, "300")
	PORT_DIPSETTING(    0x04, "600")
	PORT_DIPSETTING(    0x05, "1200")
	PORT_DIPSETTING(    0x06, "1800")
	PORT_DIPSETTING(    0x07, "2000")
	PORT_DIPSETTING(    0x08, "2400")
	PORT_DIPSETTING(    0x09, "3600")
	PORT_DIPSETTING(    0x0a, "4800")
	PORT_DIPSETTING(    0x0b, "7200")
	PORT_DIPSETTING(    0x0c, "9600")
	PORT_DIPSETTING(    0x0d, "19200")
	PORT_DIPSETTING(    0x0e, "38400")
	PORT_DIPSETTING(    0x0f, "134.5")
	// The manual's S401 diagram draws the leader lines for switches 4-7 in reverse. These
	// settings follow the firmware.
	PORT_DIPNAME( 0x10, 0x00, "Parity")        PORT_DIPLOCATION("SW401:5")
	PORT_DIPSETTING(    0x00, "Disabled")
	PORT_DIPSETTING(    0x10, "Enabled")
	PORT_DIPNAME( 0x20, 0x00, "Parity Type")   PORT_DIPLOCATION("SW401:6")
	PORT_DIPSETTING(    0x00, "Odd")
	PORT_DIPSETTING(    0x20, "Even")
	PORT_DIPNAME( 0x40, 0x00, "Word Length")   PORT_DIPLOCATION("SW401:7")
	PORT_DIPSETTING(    0x00, "8-bit" )
	PORT_DIPSETTING(    0x40, "7-bit" )
	PORT_DIPNAME( 0x80, 0x80, "Duplex")        PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x80, "Full")
INPUT_PORTS_END


ROM_START( superset )
	// SuperSet ROM
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "27256_101-402_superset_code.u437", 0x0000, 0x2000, CRC(a6896076) SHA1(a4e2d25028dc75a665c3f5a830a4e8cdecbd481c))
	// 1st 8k is unused, so overwrite it, with the next 16k (but only first 8k is valid)
	ROM_CONTINUE(0x0000, 0x4000)
	// overwrite at 8k starting address with the last 8k of the chip.
	ROM_CONTINUE(0x2000, 0x2000)

	// Superset SUPERFONT
	ROM_REGION( 0x8000, "chargen", 0 )
	ROM_LOAD( "27256_101-431_superset_font.u420", 0x0000, 0x8000, CRC(4c0688f6) SHA1(be6059913420ad66f5c839d619fdcb164ffae85a))

	// Superset Keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "2716_101-422_superset_kbd.u445",   0x0000, 0x0800, CRC(549d15b3) SHA1(981962e5e05bbdc5a66b0e86870853ce5596e877))
ROM_END


/**
 * Superset ROM
 *
 * Developed by TMSI
 */
heath_superset_tlb_device::heath_superset_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_tlb_device(mconfig, HEATH_SUPERSET, tag, owner, clock)
	, m_selected_char_set(0)
{
}

void heath_superset_tlb_device::device_add_mconfig(machine_config &config)
{
	heath_tlb_device::device_add_mconfig(config);

	// part of the Superset upgrade was to upgrade the CPU to 3 MHz.
	m_maincpu->set_clock(H19_3MHZ);
	m_maincpu->set_addrmap(AS_PROGRAM, &heath_superset_tlb_device::mem_map);
	m_maincpu->set_addrmap(AS_IO, &heath_superset_tlb_device::io_map);

	// per line updates are needed for onscreen menu to display properly
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);

	m_crtc->set_update_row_callback(FUNC(heath_superset_tlb_device::crtc_update_row));

	// link up the serial port outputs to font chip.
	m_ace->out_dtr_callback().set(FUNC(heath_superset_tlb_device::dtr_internal));
	m_ace->out_out1_callback().set(FUNC(heath_superset_tlb_device::out1_internal));
	m_ace->out_out2_callback().set(FUNC(heath_superset_tlb_device::out2_internal));
}

void heath_superset_tlb_device::device_start()
{
	heath_tlb_device::device_start();

	save_item(NAME(m_selected_char_set));
	save_item(NAME(m_reverse_video_disabled));

	m_selected_char_set      = 0;
	m_reverse_video_disabled = false;
}

void heath_superset_tlb_device::mem_map(address_map &map)
{
	heath_tlb_device::mem_map(map);

	// update rom space for the 16k ROM.
	map(0x0000, 0x3fff).rom();

	// page 2 memory
	map(0x8000, 0x87ff).ram();
}

void heath_superset_tlb_device::io_map(address_map &map)
{
	heath_tlb_device::io_map(map);

	map(0x61, 0x61).mirror(0x12).select(0x0c).rw(FUNC(heath_superset_tlb_device::crtc_reg_r), FUNC(heath_superset_tlb_device::crtc_reg_w));
}

const tiny_rom_entry *heath_superset_tlb_device::device_rom_region() const
{
	return ROM_NAME(superset);
}

ioport_constructor heath_superset_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(superset);
}

u8 heath_superset_tlb_device::crtc_reg_r(offs_t reg)
{
	if (!machine().side_effects_disabled())
	{
		m_reverse_video_disabled = bool(BIT(reg, 3));
	}

	return heath_tlb_device::crtc_reg_r(reg);
}

void heath_superset_tlb_device::crtc_reg_w(offs_t reg, u8 val)
{
	m_reverse_video_disabled = bool(BIT(reg, 3));

	heath_tlb_device::crtc_reg_w(reg, val);
}

MC6845_UPDATE_ROW(heath_superset_tlb_device::crtc_update_row)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32               *p       = &bitmap.pix(y);

	if (de)
	{
		for (int x = 0; x < x_count; x++)
		{
			u8  inv                  = (x == cursor_x) ? 0xff : 0;
			u8  chr                  = m_p_videoram[(ma + x) & 0x7ff];
			u16 char_set_base_offset = (m_selected_char_set << 11);

			if (chr & 0x80)
			{
				if (m_reverse_video_disabled)
				{
					// set A13
					char_set_base_offset |= 0x2000;
				}
				else
				{
					inv ^= 0xff;
				}

				chr &= 0x7f;
			}

			// get pattern of pixels for the character scanline
			u8 const gfx = m_p_chargen[char_set_base_offset | (chr << 4) | ra] ^ inv;

			// Display a scanline of a character (8 pixels)
			for (int b = 0; 8 > b; ++b)
			{
				*p++ = palette[BIT(gfx, b)];
			}
		}
	}
	else
	{
		std::fill_n(p, bitmap.rowpixels(), palette[0]);
	}
}

// DTR (pin 33) to A14 (pin 27) of 27256 ROM
void heath_superset_tlb_device::dtr_internal(int data)
{
	m_selected_char_set = (m_selected_char_set & 0x03) | ((data & 0x01) << 3);
}

// OUT1 (pin 34) to A12 (pin 2) of 27256 ROM
void heath_superset_tlb_device::out1_internal(int data)
{
	m_selected_char_set = (m_selected_char_set & 0x09) | ((data & 0x01) << 1);
}

// OUT2 (pin 31) to A11 on board
void heath_superset_tlb_device::out2_internal(int data)
{
	m_selected_char_set = (m_selected_char_set & 0x0a) | (data & 0x01);
}

} // anonymous namespace

} // namespace bus::heathzenith::h19

DEFINE_DEVICE_TYPE_PRIVATE(HEATH_SUPERSET, device_heath_tlb_card_interface, bus::heathzenith::h19::heath_superset_tlb_device, "heath_superset_tlb", "Heath Terminal Logic Board w/ Superset ROM")
