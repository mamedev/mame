// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Mitsubishi Multi 16

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pic8259.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


class multi16_state : public driver_device
{
public:
	multi16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic(*this, "pic8259"),
		m_crtc(*this, "crtc"),
		m_palette(*this, "palette"),
		m_p_vram(*this, "p_vram")
	{ }

	void multi16(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	DECLARE_WRITE8_MEMBER(multi16_6845_address_w);
	DECLARE_WRITE8_MEMBER(multi16_6845_data_w);
	required_shared_ptr<uint16_t> m_p_vram;
	uint8_t m_crtc_vreg[0x100], m_crtc_index;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_multi16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void multi16_io(address_map &map);
	void multi16_map(address_map &map);
};


void multi16_state::video_start()
{
}

#define mc6845_h_char_total     (m_crtc_vreg[0])
#define mc6845_h_display        (m_crtc_vreg[1])
#define mc6845_h_sync_pos       (m_crtc_vreg[2])
#define mc6845_sync_width       (m_crtc_vreg[3])
#define mc6845_v_char_total     (m_crtc_vreg[4])
#define mc6845_v_total_adj      (m_crtc_vreg[5])
#define mc6845_v_display        (m_crtc_vreg[6])
#define mc6845_v_sync_pos       (m_crtc_vreg[7])
#define mc6845_mode_ctrl        (m_crtc_vreg[8])
#define mc6845_tile_height      (m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start   (m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end     (m_crtc_vreg[0x0b])
#define mc6845_start_addr       (((m_crtc_vreg[0x0c]<<8) & 0x3f00) | (m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr      (((m_crtc_vreg[0x0e]<<8) & 0x3f00) | (m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr   (((m_crtc_vreg[0x10]<<8) & 0x3f00) | (m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr      (((m_crtc_vreg[0x12]<<8) & 0x3f00) | (m_crtc_vreg[0x13] & 0xff))


uint32_t multi16_state::screen_update_multi16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int count;
	int xi;

	count = 0;

	for(y=0;y<mc6845_v_display*8;y++)
	{
		for(x=0;x<(mc6845_h_display*8)/16;x++)
		{
			for(xi=0;xi<16;xi++)
			{
				int dot = (bitswap<16>(m_p_vram[count],7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8) >> (15-xi)) & 0x1;

				if(screen.visible_area().contains(x*16+xi, y))
					bitmap.pix16(y, x*16+xi) = m_palette->pen(dot);
			}

			count++;
		}
	}

	return 0;
}

void multi16_state::multi16_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x7ffff).ram();
	map(0xd8000, 0xdffff).ram().share("p_vram");
	map(0xe0000, 0xeffff).ram();
	map(0xf0000, 0xf3fff).mirror(0xc000).rom().region("ipl", 0);
}

WRITE8_MEMBER( multi16_state::multi16_6845_address_w )
{
	m_crtc_index = data;
	m_crtc->address_w(data);
}

WRITE8_MEMBER( multi16_state::multi16_6845_data_w )
{
	m_crtc_vreg[m_crtc_index] = data;
	m_crtc->register_w(data);
}

void multi16_state::multi16_io(address_map &map)
{
	map.unmap_value_high();
	map(0x02, 0x03).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)); // i8259
	map(0x40, 0x40).w(FUNC(multi16_state::multi16_6845_address_w));
	map(0x41, 0x41).w(FUNC(multi16_state::multi16_6845_data_w));
}

/* Input ports */
static INPUT_PORTS_START( multi16 )
INPUT_PORTS_END

void multi16_state::machine_start()
{
}


void multi16_state::machine_reset()
{
}


void multi16_state::multi16(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &multi16_state::multi16_map);
	m_maincpu->set_addrmap(AS_IO, &multi16_state::multi16_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 200);
	screen.set_visarea(0, 640-1, 0, 200-1);
	screen.set_screen_update(FUNC(multi16_state::screen_update_multi16));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(8);

	/* devices */
	H46505(config, m_crtc, 16000000/5);    /* unknown clock, hand tuned to get ~60 fps */
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);
}

/* ROM definition */
ROM_START( multi16 )
	ROM_REGION( 0x4000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "ipl.rom", 0x0000, 0x4000, CRC(5beb5e94) SHA1(d3b9dc9a08995a0f26af9671893417e795370306))
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY       FULLNAME    FLAGS
COMP( 1986, multi16, 0,      0,      multi16, multi16, multi16_state, empty_init, "Mitsubishi", "Multi 16", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
