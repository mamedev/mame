// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Driver for Casio PV-1000

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


// PV-1000 Sound device

class pv1000_sound_device : public device_t,
									public device_sound_interface
{
public:
	pv1000_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER(voice_w);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:

	// internal state
	struct
	{
		uint32_t  count;
		uint16_t  period;
		uint8_t   val;
	}       m_voice[4];

	sound_stream    *m_sh_channel;
};

DEFINE_DEVICE_TYPE(PV1000, pv1000_sound_device, "pv1000_sound", "NEC D65010G031")

pv1000_sound_device::pv1000_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PV1000, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pv1000_sound_device::device_start()
{
	m_sh_channel = machine().sound().stream_alloc(*this, 0, 1, clock() / 1024);

	save_item(NAME(m_voice[0].count));
	save_item(NAME(m_voice[0].period));
	save_item(NAME(m_voice[0].val));
	save_item(NAME(m_voice[1].count));
	save_item(NAME(m_voice[1].period));
	save_item(NAME(m_voice[1].val));
	save_item(NAME(m_voice[2].count));
	save_item(NAME(m_voice[2].period));
	save_item(NAME(m_voice[2].val));
	// are these ever used?
	save_item(NAME(m_voice[3].count));
	save_item(NAME(m_voice[3].period));
	save_item(NAME(m_voice[3].val));
}

WRITE8_MEMBER(pv1000_sound_device::voice_w)
{
	offset &= 0x03;
	m_voice[offset].period = data;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------


/*
 plgDavid's audio implementation/analysis notes:

 Sound appears to be 3 50/50 pulse voices made by cutting the main clock by 1024,
 then by the value of the 6bit period registers.
 This creates a surprisingly accurate pitch range.

 Note: the register periods are inverted.
 */

void pv1000_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];

	while (samples > 0)
	{
		*buffer=0;

		for (int i = 0; i < 3; i++)
		{
			uint32_t per = (0x3f - (m_voice[i].period & 0x3f));

			if (per != 0)   //OFF!
				*buffer += m_voice[i].val * 8192;

			m_voice[i].count++;

			if (m_voice[i].count >= per)
			{
				m_voice[i].count = 0;
				m_voice[i].val = !m_voice[i].val;
			}
		}

		buffer++;
		samples--;
	}
}


// PV-1000 System


class pv1000_state : public driver_device
{
public:
	pv1000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "pv1000_sound"),
		m_cart(*this, "cartslot"),
		m_p_videoram(*this, "videoram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void pv1000(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(io_w);
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(gfxram_w);
	uint8_t   m_io_regs[8];
	uint8_t   m_fd_data;

	emu_timer       *m_irq_on_timer;
	emu_timer       *m_irq_off_timer;
	uint8_t m_pcg_bank;
	uint8_t m_force_pattern;
	uint8_t m_fd_buffer_flag;
	uint8_t m_border_col;

	uint8_t * m_gfxram;
	void pv1000_postload();

	required_device<cpu_device> m_maincpu;
	required_device<pv1000_sound_device> m_sound;
	required_device<generic_slot_device> m_cart;
	required_shared_ptr<uint8_t> m_p_videoram;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_pv1000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(d65010_irq_on_cb);
	TIMER_CALLBACK_MEMBER(d65010_irq_off_cb);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( pv1000_cart );
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	void pv1000(address_map &map);
	void pv1000_io(address_map &map);
};


void pv1000_state::pv1000(address_map &map)
{
	//AM_RANGE(0x0000, 0x7fff)      // mapped by the cartslot
	map(0xb800, 0xbbff).ram().share("videoram");
	map(0xbc00, 0xbfff).ram().w(FUNC(pv1000_state::gfxram_w)).region("gfxram", 0);
}


void pv1000_state::pv1000_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xf8, 0xff).rw(FUNC(pv1000_state::io_r), FUNC(pv1000_state::io_w));
}


WRITE8_MEMBER( pv1000_state::gfxram_w )
{
	uint8_t *gfxram = memregion( "gfxram" )->base();

	gfxram[ offset ] = data;
	m_gfxdecode->gfx(1)->mark_dirty(offset/32);
}


WRITE8_MEMBER( pv1000_state::io_w )
{
	switch (offset)
	{
	case 0x00:
	case 0x01:
	case 0x02:
		//logerror("io_w offset=%02x, data=%02x (%03d)\n", offset, data , data);
		m_sound->voice_w(space, offset, data);
	break;

	case 0x03:
		//currently unknown use
	break;

	case 0x05:
		m_fd_data = 0xf;
		break;
//  case 0x06 VRAM + PCG location, always fixed at 0xb8xx
	case 0x07:
		/* ---- -xxx unknown, border color? */
		m_pcg_bank = (data & 0x20) >> 5;
		m_force_pattern = ((data & 0x10) >> 4); /* Dig Dug relies on this */
		m_border_col = data & 7;
		break;
	}

	m_io_regs[offset] = data;
}


READ8_MEMBER( pv1000_state::io_r )
{
	uint8_t data = m_io_regs[offset];

//  logerror("io_r offset=%02x\n", offset );

	switch ( offset )
	{
	case 0x04:
		/* Bit 1 = 1 => Data is available in port FD */
		/* Bit 0 = 1 => Buffer at port FD is empty */
		data = 0;
		data = m_fd_buffer_flag & 1;
		data |= m_fd_data ? 2 : 0;
		m_fd_buffer_flag &= ~1;
		break;
	case 0x05:
		static const char *const joynames[] = { "IN0", "IN1", "IN2", "IN3" };

		data = 0;

		for (int i = 0; i < 4; i++)
		{
			if (m_io_regs[5] & 1 << i)
			{
				data |= ioport(joynames[i])->read();
				m_fd_data &= ~(1 << i);
			}
		}

		//m_fd_data = 0;
		break;
	}

	return data;
}


static INPUT_PORTS_START( pv1000 )
	PORT_START( "IN0" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(2)

	PORT_START( "IN1" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY

	PORT_START( "IN2" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY

	PORT_START( "IN3" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


DEVICE_IMAGE_LOAD_MEMBER( pv1000_state, pv1000_cart )
{
	uint32_t size = m_cart->common_get_size("rom");

	if (size != 0x2000 && size != 0x4000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}


uint32_t pv1000_state::screen_update_pv1000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	bitmap.fill(m_border_col); // TODO: might be either black or colored by this register

	for ( y = 0; y < 24; y++ )
	{
		for ( x = 2; x < 30; x++ ) // left-right most columns are definitely masked by the border color
		{
			uint16_t tile = m_p_videoram[ y * 32 + x ];

			if ( tile < 0xe0 || m_force_pattern )
			{
				tile += ( m_pcg_bank << 8);
				m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, tile, 0, 0, 0, x*8, y*8 );
			}
			else
			{
				tile -= 0xe0;
				m_gfxdecode->gfx(1)->opaque(bitmap,cliprect, tile, 0, 0, 0, x*8, y*8 );
			}
		}
	}

	return 0;
}



/* Interrupt is triggering 16 times during vblank. */
/* we have chosen to trigger on scanlines 195, 199, 203, 207, 211, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255 */
TIMER_CALLBACK_MEMBER(pv1000_state::d65010_irq_on_cb)
{
	int vpos = m_screen->vpos();
	int next_vpos = vpos + 4;

	if(vpos == 195)
		m_fd_buffer_flag |= 1; /* TODO: exact timing of this */

	/* Set IRQ line and schedule release of IRQ line */
	m_maincpu->set_input_line(0, ASSERT_LINE );
	m_irq_off_timer->adjust( m_screen->time_until_pos(vpos, 380/2 ) );

	/* Schedule next IRQ trigger */
	if ( vpos >= 255 )
	{
		next_vpos = 195;
	}
	m_irq_on_timer->adjust( m_screen->time_until_pos(next_vpos, 0 ) );
}


TIMER_CALLBACK_MEMBER(pv1000_state::d65010_irq_off_cb)
{
	m_maincpu->set_input_line(0, CLEAR_LINE );
}


void pv1000_state::pv1000_postload()
{
	// restore GFX ram
	for (int i = 0; i < 0x400; i++)
		gfxram_w(m_maincpu->space(AS_PROGRAM), i, m_gfxram[i]);
}

void pv1000_state::machine_start()
{
	m_irq_on_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pv1000_state::d65010_irq_on_cb),this));
	m_irq_off_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pv1000_state::d65010_irq_off_cb),this));

	m_gfxram = memregion("gfxram")->base();
	save_pointer(NAME(m_gfxram), 0x400);

	if (m_cart->exists())
	{
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x7fff, read8sm_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));

		// FIXME: this is needed for gfx decoding, but there is probably a cleaner solution!
		std::string region_tag;
		memcpy(memregion("gfxrom")->base(), memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str())->base(), m_cart->get_rom_size());
	}

	save_item(NAME(m_io_regs));
	save_item(NAME(m_fd_data));
	save_item(NAME(m_pcg_bank));
	save_item(NAME(m_force_pattern));
	save_item(NAME(m_fd_buffer_flag));
	save_item(NAME(m_border_col));

	machine().save().register_postload(save_prepost_delegate(FUNC(pv1000_state::pv1000_postload), this));
}


void pv1000_state::machine_reset()
{
	m_io_regs[5] = 0;
	m_fd_data = 0;
	m_irq_on_timer->adjust(m_screen->time_until_pos(195, 0));
	m_irq_off_timer->adjust(attotime::never);
}


static const gfx_layout pv1000_3bpp_gfx =
{
	8, 8,           /* 8x8 characters */
	RGN_FRAC(1,1),
	3,
	{ 0, 8*8, 16*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8*4
};


static GFXDECODE_START( gfx_pv1000 )
	GFXDECODE_ENTRY( "gfxrom", 8, pv1000_3bpp_gfx, 0, 8 )
	GFXDECODE_ENTRY( "gfxram", 8, pv1000_3bpp_gfx, 0, 8 )
GFXDECODE_END


MACHINE_CONFIG_START(pv1000_state::pv1000)

	MCFG_DEVICE_ADD( "maincpu", Z80, 17897725/5 )
	MCFG_DEVICE_PROGRAM_MAP( pv1000 )
	MCFG_DEVICE_IO_MAP( pv1000_io )


	/* D65010G031 - Video & sound chip */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(17897725/3, 380, 0, 256, 262, 0, 192);
	m_screen->set_screen_update(FUNC(pv1000_state::screen_update_pv1000));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::BGR_3BIT);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pv1000);

	SPEAKER(config, "mono").front_center();
	PV1000(config, m_sound, 17897725).add_route(ALL_OUTPUTS, "mono", 1.00);

	/* Cartridge slot */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_linear_slot, "pv1000_cart")
	MCFG_GENERIC_MANDATORY
	MCFG_GENERIC_LOAD(pv1000_state, pv1000_cart)

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("pv1000");
MACHINE_CONFIG_END


ROM_START( pv1000 )
	ROM_REGION( 0x4000, "gfxrom", ROMREGION_ERASE00 )
	ROM_REGION( 0x400, "gfxram", ROMREGION_ERASE00 )
ROM_END


/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME    FLAGS */
CONS( 1983, pv1000, 0,      0,      pv1000,  pv1000, pv1000_state, empty_init, "Casio", "PV-1000",  MACHINE_SUPPORTS_SAVE )
