/***************************************************************************

    Driver for Casio PV-1000

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cartslot.h"


class pv1000_sound_device : public device_t,
									public device_sound_interface
{
public:
	pv1000_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
};

extern const device_type PV1000;

const device_type PV1000 = &device_creator<pv1000_sound_device>;

pv1000_sound_device::pv1000_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PV1000, "NEC D65010G031", tag, owner, clock),
		device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void pv1000_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
static DEVICE_START( pv1000_sound );
void pv1000_sound_device::device_start()
{
	DEVICE_START_NAME( pv1000_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void pv1000_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}




class pv1000_state : public driver_device
{
public:
	pv1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_p_videoram(*this, "p_videoram")
		{ }

	DECLARE_WRITE8_MEMBER(pv1000_io_w);
	DECLARE_READ8_MEMBER(pv1000_io_r);
	DECLARE_WRITE8_MEMBER(pv1000_gfxram_w);
	UINT8   m_io_regs[8];
	UINT8   m_fd_data;
	struct
	{
		UINT32  count;
		UINT16  period;
		UINT8   val;
	}       m_voice[4];

	sound_stream    *m_sh_channel;
	emu_timer       *m_irq_on_timer;
	emu_timer       *m_irq_off_timer;
	UINT8 m_pcg_bank;
	UINT8 m_force_pattern;
	UINT8 m_fd_buffer_flag;
	UINT8 m_border_col;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<UINT8> m_p_videoram;
	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();
	UINT32 screen_update_pv1000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(d65010_irq_on_cb);
	TIMER_CALLBACK_MEMBER(d65010_irq_off_cb);
};


static ADDRESS_MAP_START( pv1000, AS_PROGRAM, 8, pv1000_state )
	AM_RANGE( 0x0000, 0x3fff ) AM_MIRROR( 0x4000 ) AM_ROM AM_REGION( "cart", 0 )
	AM_RANGE( 0xb800, 0xbbff ) AM_RAM AM_SHARE("p_videoram")
	AM_RANGE( 0xbc00, 0xbfff ) AM_RAM_WRITE( pv1000_gfxram_w ) AM_REGION( "gfxram", 0 )
ADDRESS_MAP_END


static ADDRESS_MAP_START( pv1000_io, AS_IO, 8, pv1000_state )
	ADDRESS_MAP_GLOBAL_MASK( 0xff )
	AM_RANGE( 0xf8, 0xff ) AM_READWRITE( pv1000_io_r, pv1000_io_w )
ADDRESS_MAP_END


WRITE8_MEMBER( pv1000_state::pv1000_gfxram_w )
{
	UINT8 *gfxram = memregion( "gfxram" )->base();

	gfxram[ offset ] = data;
	machine().gfx[1]->mark_dirty(offset/32);
}


WRITE8_MEMBER( pv1000_state::pv1000_io_w )
{
	switch ( offset )
	{
	case 0x00:
	case 0x01:
	case 0x02:
		//logerror("pv1000_io_w offset=%02x, data=%02x (%03d)\n", offset, data , data);
		m_voice[offset].period = data;
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


READ8_MEMBER( pv1000_state::pv1000_io_r )
{
	UINT8 data = m_io_regs[offset];

//  logerror("pv1000_io_r offset=%02x\n", offset );

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
		int i;

		data = 0;

		for(i=0;i<4;i++)
		{
			if(m_io_regs[5] & 1 << i)
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


void pv1000_state::palette_init()
{
	int i;

	for(i=0;i<8;i++)
		palette_set_color_rgb( machine(), i, pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
}


static DEVICE_IMAGE_LOAD( pv1000_cart )
{
	UINT8 *cart = image.device().machine().root_device().memregion("cart")->base();
	UINT32 size;

	if (image.software_entry() == NULL)
		size = image.length();
	else
		size = image.get_software_region_length("rom");


	if (size != 0x2000 && size != 0x4000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return IMAGE_INIT_FAIL;
	}

	if (image.software_entry() == NULL)
	{
		if (image.fread( cart, size) != size)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unable to fully read from file");
			return IMAGE_INIT_FAIL;
		}
	}
	else
		memcpy(cart, image.get_software_region("rom"), size);


	/* Mirror 8KB rom */
	if (size == 0x2000)
		memcpy(cart + 0x2000, cart, 0x2000);

	return IMAGE_INIT_PASS;
}


UINT32 pv1000_state::screen_update_pv1000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	bitmap.fill(m_border_col); // TODO: might be either black or colored by this register

	for ( y = 0; y < 24; y++ )
	{
		for ( x = 2; x < 30; x++ ) // left-right most columns are definitely masked by the border color
		{
			UINT16 tile = m_p_videoram[ y * 32 + x ];

			if ( tile < 0xe0 || m_force_pattern )
			{
				tile += ( m_pcg_bank << 8);
				drawgfx_opaque( bitmap, cliprect, machine().gfx[0], tile, 0, 0, 0, x*8, y*8 );
			}
			else
			{
				tile -= 0xe0;
				drawgfx_opaque( bitmap, cliprect, machine().gfx[1], tile, 0, 0, 0, x*8, y*8 );
			}
		}
	}

	return 0;
}


/*
 plgDavid's audio implementation/analysis notes:

 Sound appears to be 3 50/50 pulse voices made by cutting the main clock by 1024,
 then by the value of the 6bit period registers.
 This creates a surprisingly accurate pitch range.

 Note: the register periods are inverted.
 */

static STREAM_UPDATE( pv1000_sound_update )
{
	pv1000_state *state = device->machine().driver_data<pv1000_state>();
	stream_sample_t *buffer = outputs[0];

	while ( samples > 0 )
	{
		*buffer=0;

		for (size_t i=0;i<3;i++)
		{
			UINT32 per = (0x3F-(state->m_voice[i].period & 0x3f));

			if( per != 0)//OFF!
				*buffer += state->m_voice[i].val * 8192;

			state->m_voice[i].count++;

			if (state->m_voice[i].count >= per)
			{
				state->m_voice[i].count = 0;
				state->m_voice[i].val = !state->m_voice[i].val;
			}
		}

		buffer++;
		samples--;
	}

}


static DEVICE_START( pv1000_sound )
{
	pv1000_state *state = device->machine().driver_data<pv1000_state>();
	state->m_sh_channel = device->machine().sound().stream_alloc(*device, 0, 1, device->clock()/1024, 0, pv1000_sound_update );
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


void pv1000_state::machine_start()
{
	m_irq_on_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pv1000_state::d65010_irq_on_cb),this));
	m_irq_off_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pv1000_state::d65010_irq_off_cb),this));
}


void pv1000_state::machine_reset()
{

	m_io_regs[5] = 0;
	m_fd_data = 0;
	m_irq_on_timer->adjust( m_screen->time_until_pos(195, 0 ) );
	m_irq_off_timer->adjust( attotime::never );
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


static GFXDECODE_START( pv1000 )
	GFXDECODE_ENTRY( "cart", 8, pv1000_3bpp_gfx, 0, 8 )
	GFXDECODE_ENTRY( "gfxram", 8, pv1000_3bpp_gfx, 0, 8 )
GFXDECODE_END


static MACHINE_CONFIG_START( pv1000, pv1000_state )

	MCFG_CPU_ADD( "maincpu", Z80, 17897725/5 )
	MCFG_CPU_PROGRAM_MAP( pv1000 )
	MCFG_CPU_IO_MAP( pv1000_io )


	/* D65010G031 - Video & sound chip */
	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_RAW_PARAMS( 17897725/3, 380, 0, 256, 262, 0, 192 )
	MCFG_SCREEN_UPDATE_DRIVER(pv1000_state, screen_update_pv1000)

	MCFG_PALETTE_LENGTH( 8 )
	MCFG_GFXDECODE( pv1000 )

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD( "pv1000_sound", PV1000, 17897725 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

	/* Cartridge slot */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("pv1000_cart")
	MCFG_CARTSLOT_LOAD(pv1000_cart)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","pv1000")
MACHINE_CONFIG_END


ROM_START( pv1000 )
	ROM_REGION( 0x4000, "cart", ROMREGION_ERASE00 )
	ROM_REGION( 0x400, "gfxram", ROMREGION_ERASE00 )
ROM_END


/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  INIT    COMPANY   FULLNAME    FLAGS */
CONS( 1983, pv1000,  0,      0,      pv1000,  pv1000, driver_device,   0,   "Casio",  "PV-1000",  0 )
