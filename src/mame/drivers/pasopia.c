// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Toshiba Pasopia

    TODO:
    - machine emulation needs merging with Pasopia 7 (video emulation is
      completely different tho)

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "video/mc6845.h"
#include "includes/pasopia.h"

class pasopia_state : public driver_device
{
public:
	pasopia_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi0(*this, "ppi8255_0"),
		m_ppi1(*this, "ppi8255_1"),
		m_ppi2(*this, "ppi8255_2"),
		m_ctc(*this, "z80ctc"),
		m_pio(*this, "z80pio"),
		m_crtc(*this, "crtc"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi0;
	required_device<i8255_device> m_ppi1;
	required_device<i8255_device> m_ppi2;
	required_device<z80ctc_device> m_ctc;
	required_device<z80pio_device> m_pio;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;

	DECLARE_READ8_MEMBER(pasopia_romram_r);
	DECLARE_WRITE8_MEMBER(pasopia_ram_w);
	DECLARE_WRITE8_MEMBER(pasopia_ctrl_w);
	DECLARE_WRITE8_MEMBER(vram_addr_lo_w);
	DECLARE_WRITE8_MEMBER(vram_latch_w);
	DECLARE_READ8_MEMBER(vram_latch_r);
	DECLARE_READ8_MEMBER(portb_1_r);
	DECLARE_WRITE8_MEMBER(vram_addr_hi_w);
	DECLARE_WRITE8_MEMBER(screen_mode_w);
	DECLARE_READ8_MEMBER(rombank_r);
	DECLARE_READ8_MEMBER(testa_r);
	DECLARE_READ8_MEMBER(testb_r);
	DECLARE_WRITE_LINE_MEMBER(testa_w);
	DECLARE_WRITE_LINE_MEMBER(testb_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(mux_r);
	DECLARE_READ8_MEMBER(keyb_r);
	DECLARE_WRITE8_MEMBER(mux_w);
	MC6845_UPDATE_ROW(crtc_update_row);

	UINT8 m_hblank;
	UINT16 m_vram_addr;
	UINT8 m_vram_latch;
	UINT8 m_attr_latch;
//  UINT8 m_gfx_mode;
	UINT8 m_mux_data;
	bool m_video_wl;
	bool m_ram_bank;
	UINT8 *m_p_vram;
	DECLARE_DRIVER_INIT(pasopia);
	TIMER_CALLBACK_MEMBER(pio_timer);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};

// needed to scan the keyboard, as the pio emulation doesn't do it.
TIMER_CALLBACK_MEMBER(pasopia_state::pio_timer)
{
	m_pio->port_b_write(keyb_r(generic_space(),0,0xff));
}

void pasopia_state::video_start()
{
}

MC6845_UPDATE_ROW( pasopia_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 *m_p_chargen = memregion("chargen")->base();
	UINT8 chr,gfx,fg=7,bg=0; // colours need to be determined
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		UINT8 inv=0;
		if (x == cursor_x) inv=0xff;
		mem = (ma + x) & 0xfff;
		chr = m_p_vram[mem];

		/* get pattern of pixels for that character scanline */
		gfx = m_p_chargen[(chr<<3) | ra] ^ inv;

		/* Display a scanline of a character */
		*p++ = palette[BIT(gfx, 7) ? fg : bg];
		*p++ = palette[BIT(gfx, 6) ? fg : bg];
		*p++ = palette[BIT(gfx, 5) ? fg : bg];
		*p++ = palette[BIT(gfx, 4) ? fg : bg];
		*p++ = palette[BIT(gfx, 3) ? fg : bg];
		*p++ = palette[BIT(gfx, 2) ? fg : bg];
		*p++ = palette[BIT(gfx, 1) ? fg : bg];
		*p++ = palette[BIT(gfx, 0) ? fg : bg];
	}
}

WRITE8_MEMBER( pasopia_state::pasopia_ctrl_w )
{
	m_ram_bank = BIT(data, 1);
	membank("bank1")->set_entry(m_ram_bank);
}

static ADDRESS_MAP_START(pasopia_map, AS_PROGRAM, 8, pasopia_state)
	AM_RANGE(0x0000,0x7fff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank2")
	AM_RANGE(0x8000,0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START(pasopia_io, AS_IO, 8, pasopia_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0x03) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x08,0x0b) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x10,0x10) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x11,0x11) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
//  0x18 - 0x1b pac2
//  0x1c - 0x1f something
	AM_RANGE(0x20,0x23) AM_DEVREADWRITE("ppi8255_2", i8255_device, read, write)
	AM_RANGE(0x28,0x2b) AM_DEVREADWRITE("z80ctc", z80ctc_device, read, write)
	AM_RANGE(0x30,0x33) AM_DEVREADWRITE("z80pio", z80pio_device, read, write)
//  0x38 printer
	AM_RANGE(0x3c,0x3c) AM_WRITE(pasopia_ctrl_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pasopia )
	PASOPIA_KEYBOARD
INPUT_PORTS_END

void pasopia_state::machine_start()
{
	m_p_vram = memregion("vram")->base();
	m_hblank = 0;
	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
}

void pasopia_state::machine_reset()
{
}

WRITE8_MEMBER( pasopia_state::vram_addr_lo_w )
{
	m_vram_addr = (m_vram_addr & 0x3f00) | data;
}

WRITE8_MEMBER( pasopia_state::vram_latch_w )
{
	m_vram_latch = data;
}

READ8_MEMBER( pasopia_state::vram_latch_r )
{
	return m_p_vram[m_vram_addr];
}

READ8_MEMBER( pasopia_state::portb_1_r )
{
	/*
	x--- ---- attribute latch
	-x-- ---- hblank
	--x- ---- vblank
	---x ---- LCD system mode, active low
	*/
	UINT8 grph_latch,lcd_mode;

	m_hblank ^= 0x40; //TODO
	grph_latch = (m_p_vram[m_vram_addr | 0x4000] & 0x80);
	lcd_mode = 0x10;

	return m_hblank | lcd_mode | grph_latch; //bit 4: LCD mode
}


WRITE8_MEMBER( pasopia_state::vram_addr_hi_w )
{
	m_attr_latch = (data & 0x80) | (m_attr_latch & 0x7f);
	if ( BIT(data, 6) && !m_video_wl )
	{
		m_p_vram[m_vram_addr] = m_vram_latch;
		m_p_vram[m_vram_addr | 0x4000] = m_attr_latch;
	}

	m_video_wl = BIT(data, 6);
	m_vram_addr = (m_vram_addr & 0xff) | ((data & 0x3f) << 8);
}

WRITE8_MEMBER( pasopia_state::screen_mode_w )
{
	//m_gfx_mode = (data & 0xe0) >> 5; unused variable
	m_attr_latch = (m_attr_latch & 0x80) | (data & 7);
	printf("Screen Mode=%02x\n",data);
}

READ8_MEMBER( pasopia_state::rombank_r )
{
	return (m_ram_bank) ? 4 : 0;
}

READ8_MEMBER( pasopia_state::mux_r )
{
	return m_mux_data;
}

READ8_MEMBER( pasopia_state::keyb_r )
{
	const char *const keynames[3][4] = { { "KEY0", "KEY1", "KEY2", "KEY3" },
											{ "KEY4", "KEY5", "KEY6", "KEY7" },
											{ "KEY8", "KEY9", "KEYA", "KEYB" } };
	int i,j;
	UINT8 res;

	res = 0;
	for(j=0;j<3;j++)
	{
		if(m_mux_data & 0x10 << j)
		{
			for(i=0;i<4;i++)
			{
				if(m_mux_data & 1 << i)
					res |= ioport(keynames[j][i])->read();
			}
		}
	}

	return res ^ 0xff;
}

WRITE8_MEMBER( pasopia_state::mux_w )
{
	m_mux_data = data;
}

static const gfx_layout p7_chars_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( pasopia )
	GFXDECODE_ENTRY( "chargen", 0x0000, p7_chars_8x8, 0, 4 )
GFXDECODE_END

static const z80_daisy_config pasopia_daisy[] =
{
	{ "z80ctc" },
	{ "z80pio" },
//  { "upd765" }, /* TODO */
	{ NULL }
};



DRIVER_INIT_MEMBER(pasopia_state,pasopia)
{
/*
We preset all banks here, so that bankswitching will incur no speed penalty.
0000 indicates ROMs, 10000 indicates RAM.
*/
	UINT8 *p_ram = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &p_ram[0x00000], 0x10000);
	membank("bank2")->configure_entry(0, &p_ram[0x10000]);

	machine().scheduler().timer_pulse(attotime::from_hz(500), timer_expired_delegate(FUNC(pasopia_state::pio_timer),this));
}

static MACHINE_CONFIG_START( pasopia, pasopia_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(pasopia_map)
	MCFG_CPU_IO_MAP(pasopia_io)
	MCFG_CPU_CONFIG(pasopia_daisy)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", h46505_device, screen_update)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pasopia)
	MCFG_PALETTE_ADD("palette", 8)

	/* Devices */
	MCFG_MC6845_ADD("crtc", H46505, "screen", XTAL_4MHz/4)   /* unknown clock, hand tuned to get ~60 fps */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(pasopia_state, crtc_update_row)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(pasopia_state, vram_addr_lo_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(pasopia_state, vram_latch_w))
	MCFG_I8255_IN_PORTC_CB(READ8(pasopia_state, vram_latch_r))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(pasopia_state, screen_mode_w))
	MCFG_I8255_IN_PORTB_CB(READ8(pasopia_state, portb_1_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pasopia_state, vram_addr_hi_w))

	MCFG_DEVICE_ADD("ppi8255_2", I8255A, 0)
	MCFG_I8255_IN_PORTC_CB(READ8(pasopia_state, rombank_r))

	MCFG_DEVICE_ADD("z80ctc", Z80CTC, XTAL_4MHz)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("z80ctc", z80ctc_device, trg1))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE("z80ctc", z80ctc_device, trg2))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE("z80ctc", z80ctc_device, trg3))

	MCFG_DEVICE_ADD("z80pio", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(pasopia_state, mux_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(pasopia_state, mux_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(pasopia_state, keyb_r))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pasopia )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "tbasic.rom", 0x0000, 0x8000, CRC(f53774ff) SHA1(bbec45a3bad8d184505cc6fe1f6e2e60a7fb53f2))

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x0000, 0x0800, BAD_DUMP CRC(a91c45a9) SHA1(a472adf791b9bac3dfa6437662e1a9e94a88b412)) //stolen from pasopia7

	ROM_REGION( 0x8000, "vram", ROMREGION_ERASE00 )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    INIT      COMPANY      FULLNAME       FLAGS */
COMP( 1986, pasopia, 0,      0,       pasopia,   pasopia, pasopia_state, pasopia, "Toshiba",   "Pasopia", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
