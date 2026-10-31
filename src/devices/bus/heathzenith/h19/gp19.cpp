// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB) plus Northwest Digital Systems GP-19

  Known Issues:
    - 49/50 row mode only shows half the screen.
    - In 49/50 row mode, character descenders are cut off.
    - Screen saver does not disable the screen

****************************************************************************/

#include "emu.h"
#include "gp19.h"

#include "tlbbase.h"

#include <algorithm>


namespace bus::heathzenith::h19 {

namespace {

static constexpr XTAL GP19_DOT_CLOCK_1 = XTAL(20'282'000);   // 132 columns
static constexpr XTAL GP19_DOT_CLOCK_2 = XTAL(12'292'000);   // 80 columns
static constexpr XTAL GP19_DOT_CLOCK_3 = XTAL(10'644'000);   // Graphics mode

/**
 * Heath TLB plus Northwest Digital Systems GP-19
 */
class heath_gp19_tlb_device : public heath_tlb_device
{
public:
	heath_gp19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void latch_u5_w(u8 data);

	virtual MC6845_UPDATE_ROW(crtc_update_row) override;

	bool m_char_gen_a11;
	bool m_graphic_mode;
	bool m_col_132;
	bool m_reverse_video;
};

static INPUT_PORTS_START( gp19 )
	PORT_INCLUDE( tlb )

	PORT_MODIFY("SW402")
	PORT_DIPNAME( 0x80, 0x00, "Automatic Holdscreen")                        PORT_DIPLOCATION("SW402:8")
	PORT_DIPSETTING(    0x00, "Disabled (VT100 mode)")
	PORT_DIPSETTING(    0x80, "Enabled (Z19 mode)")

	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x01, "Trailing Characters for Tektronix Message")   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x01, "CR")
	PORT_DIPSETTING(    0x02, DEF_STR( None ) )
	PORT_DIPSETTING(    0x03, "CR,EOT")
	PORT_DIPNAME( 0x04, 0x00, "Shift Key")                                   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, "Shift key inverts CAPS LOCK")
	PORT_DIPNAME( 0x08, 0x00, "Terminal Transmission Rate")                  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Only limited by baud rate")
	PORT_DIPSETTING(    0x08, "One character per 16.7 msec")
	PORT_DIPNAME( 0x10, 0x10, "Character Set")                               PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "VT100")
	PORT_DIPSETTING(    0x10, "Zenith")
	PORT_DIPNAME( 0x20, 0x00, "ANSI Mode")                                   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "Zenith ANSI")
	PORT_DIPSETTING(    0x20, "EDT compatible")
	PORT_DIPNAME( 0x40, 0x00, "Tektronix Character Wrap")                    PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "Column 74")
	PORT_DIPSETTING(    0x40, "Column 73")
	PORT_DIPNAME( 0x80, 0x00, "Blank Screen")                                PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "After 10 mins")
	PORT_DIPSETTING(    0x80, "Never")
INPUT_PORTS_END


ROM_START( gp19 )
	// GP-19 ROMs
	ROM_REGION( 0x3000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gp19_rom_1_ver_1_1.u26",           0x0000, 0x1000, CRC(7cbe8e0d) SHA1(977df895b067fa4e4646b4518cffb08d4e6bb6e0))
	ROM_LOAD( "gp19_rom_2_ver_1_1.u25",           0x1000, 0x1000, CRC(a85e3bc4) SHA1(f64e851c5d3ef98a9a48e0fc482baa576773ff43))
	ROM_LOAD( "gp19_rom_3_ver_1_1.u24",           0x2000, 0x1000, CRC(878f577c) SHA1(0756435914dcfb981de4e40d5f81af3e0f5bb1c5))

	// GP-19 font
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "gp19_char_gen_cg_1.u21",           0x0000, 0x1000, CRC(49ec9242) SHA1(770a8c7b5b15bcfe465fd84326d0ae3dcaa85311))

	// Original keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "2716_444-37_h19keyb.u445",         0x0000, 0x0800, CRC(5c3e6972) SHA1(df49ce64ae48652346a91648c58178a34fb37d3c))
ROM_END


/**
 * Northwest Digital Systems GP-19 add-in board
 */
heath_gp19_tlb_device::heath_gp19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_tlb_device(mconfig, HEATH_GP19, tag, owner, clock)
{
}

void heath_gp19_tlb_device::device_add_mconfig(machine_config &config)
{
	heath_tlb_device::device_add_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &heath_gp19_tlb_device::mem_map);
	m_maincpu->set_addrmap(AS_IO, &heath_gp19_tlb_device::io_map);

	m_crtc->set_update_row_callback(FUNC(heath_gp19_tlb_device::crtc_update_row));
	m_crtc->set_clock(GP19_DOT_CLOCK_2 / 8);

	m_screen->set_raw(GP19_DOT_CLOCK_2, 776, 32, 672, 264, 10, 250);
}

void heath_gp19_tlb_device::device_start()
{
	heath_tlb_device::device_start();

	save_item(NAME(m_char_gen_a11));
	save_item(NAME(m_graphic_mode));
	save_item(NAME(m_col_132));
	save_item(NAME(m_reverse_video));

	m_char_gen_a11  = false;
	m_graphic_mode  = false;
	m_col_132       = false;
	m_reverse_video = false;
}

void heath_gp19_tlb_device::mem_map(address_map &map)
{
	map.unmap_value_high();

	// ROMs 1, 2, 3
	map(0x0000, 0x2fff).rom();

	// Optional external board - Program ROM 4(not aware if any ever existed), external I/O
	// map(0x3000, 0x3fff).rom();

	map(0x4000, 0x40ff).mirror(0x3f00).ram();
	map(0x6000, 0x67ff).mirror(0x1800).ram();
	map(0x8000, 0xbfff).mirror(0x4000).ram().share(m_p_videoram);
}

void heath_gp19_tlb_device::io_map(address_map &map)
{
	heath_tlb_device::io_map(map);

	// Latch U5
	map(0x68, 0x68).mirror(0x07).w(FUNC(heath_gp19_tlb_device::latch_u5_w));

	// Switch on GP-19 board
	map(0x70, 0x70).mirror(0x07).portr("SW1");

	// Optional Auxiliary I/O connector(not aware if any ever existed)
	// map(0x78, 0x78).mirror(0x07);
}

/**
 * U5 Latch
 *
 * Q0 Graph/character H
 * Q1 132/80 columns H
 * Q2 Reverse Screen H
 * Q3 Alt Character L
 * Q4-Q7 LED indicators (debug LEDs on circuit board)
 */
void heath_gp19_tlb_device::latch_u5_w(u8 data)
{
	m_graphic_mode  = bool(BIT(data, 0));
	m_col_132       = bool(BIT(data, 1));
	m_reverse_video = bool(BIT(data, 2));
	m_char_gen_a11  = bool(BIT(data, 3));

	// TODO handle LED indicators

	if (m_graphic_mode)
	{
		m_crtc->set_clock(GP19_DOT_CLOCK_3 / 8);
	}
	else if (m_col_132)
	{
		m_crtc->set_clock(GP19_DOT_CLOCK_1 / 8);
	}
	else
	{
		m_crtc->set_clock(GP19_DOT_CLOCK_2 / 8);
	}
}

MC6845_UPDATE_ROW(heath_gp19_tlb_device::crtc_update_row)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32               *p       = &bitmap.pix(y);

	if (de)
	{
		u8 screen_inv = m_reverse_video ? 0xff : 0x00;

		if (m_graphic_mode)
		{
			for (int x = 0; x < x_count; x++)
			{
				u8 const gfx = m_p_videoram[((ma << 1) + (ra * x_count) + x) & 0x3fff] ^ screen_inv;

				for (int b = 0; 8 > b; ++b)
				{
					*p++ = palette[BIT(gfx, b)];
				}
			}
		}
		else
		{
			u16 const base = m_char_gen_a11 ? 0x800 : 0x0;

			for (int x = 0; x < x_count; x++)
			{
				u8 inv = (x == cursor_x) ? 0xff : 0;
				u8 chr = m_p_videoram[(ma + x) & 0x3fff];

				if (chr & 0x80)
				{
					inv ^= 0xff;
					chr &= 0x7f;
				}

				inv ^= screen_inv;

				// select proper character set
				u8 const gfx = m_p_chargen[base | (chr << 4) | ra] ^ inv;

				// Display a scanline of a character (8 pixels)
				for (int b = 0; 8 > b; ++b)
				{
					*p++ = palette[BIT(gfx, b)];
				}
			}
		}
	}
	else
	{
		std::fill_n(p, x_count * 8, palette[0]);
	}
}

const tiny_rom_entry *heath_gp19_tlb_device::device_rom_region() const
{
	return ROM_NAME(gp19);
}

ioport_constructor heath_gp19_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(gp19);
}

} // anonymous namespace

} // namespace bus::heathzenith::h19

DEFINE_DEVICE_TYPE_PRIVATE(HEATH_GP19, device_heath_tlb_card_interface, bus::heathzenith::h19::heath_gp19_tlb_device, "heath_gp19_tlb", "Heath Terminal Logic Board plus Northwest Digital Systems GP-19")
