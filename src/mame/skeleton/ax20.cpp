// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Axel AX-20

    27/12/2011 Skeleton driver.

    Hardware description:
    - CPU: I8088
    - FDC: I8272
    - PIT: I8253
    - PIC: I8259

    Also marketed under the Matra brand as MAX-20 ("M" for Matra ?)

****************************************************************************/

#include "emu.h"
#include "bus/isa/fdc.h"
#include "cpu/i86/i86.h"
#include "emupal.h"
#include "screen.h"


namespace {

class ax20_state : public driver_device
{
public:
	ax20_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_p_vram(*this, "p_vram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_fdc(*this, "fdc")
	{ }

	void ax20(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<i8272a_device> m_fdc;

	virtual void machine_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t unk_r();
	void tc_w(uint8_t data);
	void ctl_w(uint8_t data);

	void ax20_io(address_map &map) ATTR_COLD;
	void ax20_map(address_map &map) ATTR_COLD;
};

uint8_t ax20_state::unk_r()
{
	return 0;
}

void ax20_state::tc_w(uint8_t data)
{
	m_fdc->tc_w((data & 0xf0) == 0xf0);
}

void ax20_state::ctl_w(uint8_t data)
{
	m_fdc->subdevice<floppy_connector>("0")->get_device()->mon_w(!(data & 1));
}

uint32_t ax20_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for ( int y = 0; y < 24; y++ )
	{
		for ( int x = 0; x < 80; x++ )
		{
			uint16_t tile = m_p_vram[24 +  y * 128 + x ] & 0x7f;

			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, tile, 0, 0, 0, x*8, y*12);
		}
	}

	return 0;
}


void ax20_state::ax20_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x1ffff).ram();
	map(0x20000, 0x3ffff).ram(); //optional RAM
	map(0xf0400, 0xf0fff).ram().share("p_vram");
	map(0xff800, 0xfffff).rom().region("ipl", 0);
}

void ax20_state::ax20_io(address_map &map)
{
	map.unmap_value_high();
	map(0xffc0, 0xffc0).w(FUNC(ax20_state::tc_w));
	map(0xffd0, 0xffd0).w(FUNC(ax20_state::ctl_w));
	map(0xffe0, 0xffe0).r(FUNC(ax20_state::unk_r));
	map(0xff80, 0xff81).m(m_fdc, FUNC(i8272a_device::map));
}

/* Input ports */
static INPUT_PORTS_START( ax20 )
INPUT_PORTS_END


void ax20_state::machine_start()
{
}

static const gfx_layout ax20_charlayout =
{
	8, 12,
	128,
	1,
	{ 0 },
	{ 4*8+7, 4*8+6, 4*8+5, 4*8+4, 4*8+3, 4*8+2, 4*8+1, 4*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 8*9, 8*10, 8*11 },
	8*16
};

static GFXDECODE_START( gfx_ax20 )
	GFXDECODE_ENTRY( "chargen", 0x0000, ax20_charlayout, 0, 1 )
GFXDECODE_END

static void ax20_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void ax20_state::ax20(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, XTAL(14'318'181)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &ax20_state::ax20_map);
	m_maincpu->set_addrmap(AS_IO, &ax20_state::ax20_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(ax20_state::screen_update));
	screen.set_size(80*8, 24*12);
	screen.set_visarea(0, 80*8-1, 0, 24*12-1);
	screen.set_palette(m_palette);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ax20);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	I8272A(config, m_fdc, 8'000'000, true);

	/* Devices */
	FLOPPY_CONNECTOR(config, "fdc:0", ax20_floppies, "525dd", isa8_fdc_device::floppy_formats);
}

/* ROM definition */
ROM_START( ax20 )
	ROM_REGION( 0x0800, "ipl", 0 )
	ROM_LOAD( "ax20-s.rom", 0x0000, 0x0800, CRC(f11f95b9) SHA1(59949332dd431fcf8211c2d556e1f49351e90750))

	ROM_REGION( 0x4000, "chargen", 0 )
	ROM_LOAD( "ax20-g.rom", 0x0000, 0x0800, CRC(90bcef80) SHA1(922067fd7316de9e69b9600c793ada5c87197eeb))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT CLASS       INIT        COMPANY  FULLNAME  FLAGS
COMP( 1982, ax20, 0,      0,      ax20,    ax20, ax20_state, empty_init, "Axel",  "AX-20",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
