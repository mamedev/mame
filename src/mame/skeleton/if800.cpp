// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    if800

****************************************************************************/


#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pic8259.h"
#include "video/upd7220.h"
#include "emupal.h"
#include "screen.h"


namespace {

class if800_state : public driver_device
{
public:
	if800_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_hgdc(*this, "upd7220"),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette") { }

	void if800(machine_config &config);

private:
	required_device<upd7220_device> m_hgdc;

	required_shared_ptr<uint16_t> m_video_ram;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	void if800_io(address_map &map) ATTR_COLD;
	void if800_map(address_map &map) ATTR_COLD;
	void upd7220_map(address_map &map) ATTR_COLD;
};

UPD7220_DISPLAY_PIXELS_MEMBER( if800_state::hgdc_display_pixels )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	int gfx = m_video_ram[address];

	for(int xi=0;xi<16;xi++)
	{
		uint8_t pen = BIT(gfx, xi);

		bitmap.pix(y, x + xi) = palette[pen];
	}
}

void if800_state::if800_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x0ffff).ram();
	map(0xfe000, 0xfffff).rom().region("ipl", 0);
}

void if800_state::if800_io(address_map &map)
{
	map.unmap_value_high();
//  map(0x0640, 0x065f) dma?
	map(0x0660, 0x0663).rw(m_hgdc, FUNC(upd7220_device::read), FUNC(upd7220_device::write)).umask16(0x00ff);
}

/* Input ports */
static INPUT_PORTS_START( if800 )
INPUT_PORTS_END

void if800_state::machine_start()
{
}


void if800_state::machine_reset()
{
}

void if800_state::upd7220_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram().share("video_ram");
}

void if800_state::if800(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &if800_state::if800_map);
	m_maincpu->set_addrmap(AS_IO, &if800_state::if800_io);


//  PIC8259(config, "pic8259", 0);
	UPD7220(config, m_hgdc, 8000000/4);
	m_hgdc->set_addrmap(0, &if800_state::upd7220_map);
	m_hgdc->set_display_pixels(FUNC(if800_state::hgdc_display_pixels));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update("upd7220", FUNC(upd7220_device::screen_update));

	PALETTE(config, m_palette).set_entries(8);

//  MCFG_VIDEO_START_OVERRIDE(if800_state,if800)
}

/* ROM definition */
ROM_START( if800 )
	ROM_REGION16_LE( 0x2000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x2000, CRC(36212491) SHA1(6eaa8885e2dccb6dd86def6c0c9be1870cee957f))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY         FULLNAME          FLAGS
COMP( 1985, if800, 0,      0,      if800,   if800, if800_state, empty_init, "Oki Electric", "if800 model 60", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
