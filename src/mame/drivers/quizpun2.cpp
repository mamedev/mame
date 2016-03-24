// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

Quiz Punch 2 (C)1989 Space Computer

Preliminary driver by Luca Elia

- It uses an unknown DIP40 device for protection, that supplies
  the address to jump to (same as mosaic.c) and handles the EEPROM

PCB Layout
----------

|---------------------------------------------|
|U1    U26        6116                   32MHz|
|                             U120 U119       |
|U2                                  U118 U117|
|      U27        6116                        |
|                                             |
|                                             |
|      U28        6116        Z80             |
|                         U2A     93C46   U111|
|                                      6264   |
|      U29                    *           8MHz|
|  U20            6116                        |
|                                6116         |
|  U21 U30                                    |
|                 6116                        |
|  U22  6116                           DSW1(8)|
|VOL    YM2203 Z80                            |
|    YM3014     MAHJONG28                     |
|---------------------------------------------|
Notes:
      * - Unknown DIP40 chip. +5V on pin 17, GND on pin 22
          pins 4,3,2 of 93C46 tied to unknown chip on pins 23,24,25
      All clocks unknown, PCB not working
      Possibly Z80's @ 4MHz and YM2203 @ 2MHz
      PCB marked 'Ducksan Trading Co. Ltd. Made In Korea'




Quiz Punch (C)1988 Space Computer
Ducksan 1989

88-01-14-0775
|--------------------------------------------------|
|VOL  MC1455                        02.U2   01.U1  |
|UPC1241                                           |
|LM358  05.U22 04.U21  03.U20                      |
|YM3014B  6116   10.U30 09.U29 08.U28 07.U27 06.U26|
|YM2203  Z80A                                      |
|                                                  |
|J              6116   6116   6116   6116   6116   |
|A                                                 |
|M                                                 |
|M                       |------------|            |
|A                       |            |            |
|                        |  EPOXY     |            |
|                        |  MODULE    |            |
|                        |            |            |
|    6116       6116     |            |            |
|                        |------------|    14.U120 |
|                               GM76C88    13.U119 |
|                                          12.U118 |
|                                          11.U117 |
|   DSW(8)                   8MHz 15.U111     32MHz|
|--------------------------------------------------|
Notes:
       Z80A - clock 4.000MHz (8/2)
     YM2203 - clock 4.000MHz (8/2)
     Epoxy Module likely contains a Z80A (an input clock of 4.000MHz is present) and possibly a ROM
     VSync - 59.3148Hz
     HSync - 15.2526kHz

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"


enum prot_state { STATE_IDLE = 0, STATE_ADDR_R, STATE_ROM_R, STATE_EEPROM_R, STATE_EEPROM_W };
struct prot_t {
	prot_state state;
	int wait_param;
	int param;
	int cmd;
	int addr;
};

class quizpun2_state : public driver_device
{
public:
	quizpun2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_fg_ram(*this, "fg_ram"),
		m_bg_ram(*this, "bg_ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	struct prot_t m_prot;
	required_shared_ptr<UINT8> m_fg_ram;
	required_shared_ptr<UINT8> m_bg_ram;
	tilemap_t *m_bg_tmap;
	tilemap_t *m_fg_tmap;
	DECLARE_WRITE8_MEMBER(bg_ram_w);
	DECLARE_WRITE8_MEMBER(fg_ram_w);
	DECLARE_READ8_MEMBER(quizpun2_protection_r);
	DECLARE_WRITE8_MEMBER(quizpun2_protection_w);
	DECLARE_WRITE8_MEMBER(quizpun2_rombank_w);
	DECLARE_WRITE8_MEMBER(quizpun2_irq_ack);
	DECLARE_WRITE8_MEMBER(quizpun2_soundlatch_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_quizpun2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


#define VERBOSE_PROTECTION_LOG 0

/***************************************************************************
                                Video Hardware
***************************************************************************/

TILE_GET_INFO_MEMBER(quizpun2_state::get_bg_tile_info)
{
	UINT16 code = m_bg_ram[ tile_index * 2 ] + m_bg_ram[ tile_index * 2 + 1 ] * 256;
	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

TILE_GET_INFO_MEMBER(quizpun2_state::get_fg_tile_info)
{
	UINT16 code  = m_fg_ram[ tile_index * 4 ] + m_fg_ram[ tile_index * 4 + 1 ] * 256;
	UINT8  color = m_fg_ram[ tile_index * 4 + 2 ];
	SET_TILE_INFO_MEMBER(1, code, color & 0x0f, 0);
}

WRITE8_MEMBER(quizpun2_state::bg_ram_w)
{
	m_bg_ram[offset] = data;
	m_bg_tmap->mark_tile_dirty(offset/2);
}

WRITE8_MEMBER(quizpun2_state::fg_ram_w)
{
	m_fg_ram[offset] = data;
	m_fg_tmap->mark_tile_dirty(offset/4);
}

void quizpun2_state::video_start()
{
	m_bg_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(quizpun2_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,8,16,0x20,0x20);
	m_fg_tmap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(quizpun2_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS,8,16,0x20,0x20);

	m_bg_tmap->set_transparent_pen(0);
	m_fg_tmap->set_transparent_pen(0);
}

UINT32 quizpun2_state::screen_update_quizpun2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int msk = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	if (layers_ctrl & 1)    m_bg_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	else                    bitmap.fill(m_palette->black_pen(), cliprect);

bitmap.fill(m_palette->black_pen(), cliprect);
	if (layers_ctrl & 2)    m_fg_tmap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


/***************************************************************************
                                Protection

    ROM checksum:   write 0x80 | (0x00-0x7f), write 0, read 2 bytes
    Read address:   write 0x80 | param1 & 0x07f (0x00), write param2 & 0x7f, read 2 bytes
    Read EEPROM:    write 0x20 | (0x00-0x0f), write 0, read 8 bytes
    Write EEPROM:   write 0x00 | (0x00-0x0f), write 0, write 8 bytes

***************************************************************************/

void quizpun2_state::machine_reset()
{
	struct prot_t &prot = m_prot;
	prot.state = STATE_IDLE;
	prot.wait_param = 0;
	prot.param = 0;
	prot.cmd = 0;
	prot.addr = 0;
}

static void log_protection( address_space &space, const char *warning )
{
	quizpun2_state *state = space.machine().driver_data<quizpun2_state>();
	struct prot_t &prot = state->m_prot;
	state->logerror("%04x: protection - %s (state %x, wait %x, param %02x, cmd %02x, addr %02x)\n", space.device().safe_pc(), warning,
		prot.state,
		prot.wait_param,
		prot.param,
		prot.cmd,
		prot.addr
	);
}

READ8_MEMBER(quizpun2_state::quizpun2_protection_r)
{
	struct prot_t &prot = m_prot;
	UINT8 ret;

	switch ( prot.state )
	{
		case STATE_ROM_R:       // Checksum of MCU addresses 0-ff (0x8e9c^0xffff expected)
			if      (prot.addr == 0xfe) ret = 0x8e ^ 0xff;
			else if (prot.addr == 0xff) ret = 0x9c ^ 0xff;
			else                        ret = 0x00;
			break;

		case STATE_ADDR_R:      // Address to jump to (big endian!)
			switch ( prot.param )
			{
				case 0x19:  // Print
					ret = 0x0b95 >> ((prot.addr & 1) ? 0 : 8);
					break;

				case 0x44:  // Clear screen?
					ret = 0x1bd9 >> ((prot.addr & 1) ? 0 : 8);  // needed, but should also clear the screen
					break;

				case 0x45:  // Backup RAM check
					ret = 0x2242 >> ((prot.addr & 1) ? 0 : 8);
					break;

				default:
					log_protection(space, "unknown address");
					ret = 0x2e59 >> ((prot.addr & 1) ? 0 : 8);  // return the address of: XOR A, RET
			}
			break;

		case STATE_EEPROM_R:        // EEPROM read
		{
			UINT8 *eeprom = memregion("eeprom")->base();
			ret = eeprom[prot.addr];
			break;
		}

		default:
			log_protection(space, "unknown read");
			ret = 0x00;
	}

#if VERBOSE_PROTECTION_LOG
	log_protection(space, "info READ");
#endif

	prot.addr++;

	return ret;
}

WRITE8_MEMBER(quizpun2_state::quizpun2_protection_w)
{
	struct prot_t &prot = m_prot;

	switch ( prot.state )
	{
		case STATE_EEPROM_W:
		{
			UINT8 *eeprom = memregion("eeprom")->base();
			eeprom[prot.addr] = data;
			prot.addr++;
			if ((prot.addr % 8) == 0)
				prot.state = STATE_IDLE;
			break;
		}

		default:
			if (prot.wait_param)
			{
				prot.param = data;
				prot.wait_param = 0;

				// change state:

				if (prot.cmd & 0x80)
				{
					if (prot.param == 0x00)
					{
						prot.state = STATE_ROM_R;
						prot.addr = (prot.cmd & 0x7f) * 2;
					}
					else if (prot.cmd == 0x80)
					{
						prot.state = STATE_ADDR_R;
						prot.addr = 0;
					}
					else
						log_protection(space, "unknown command");
				}
				else if (prot.cmd >= 0x00 && prot.cmd <= 0x0f )
				{
					prot.state = STATE_EEPROM_W;
					prot.addr = (prot.cmd & 0x0f) * 8;
				}
				else if (prot.cmd >= 0x20 && prot.cmd <= 0x2f )
				{
					prot.state = STATE_EEPROM_R;
					prot.addr = (prot.cmd & 0x0f) * 8;
				}
				else
				{
					prot.state = STATE_IDLE;
					log_protection(space, "unknown command");
				}
			}
			else
			{
				prot.cmd = data;
				prot.wait_param = 1;
			}
			break;
	}

#if VERBOSE_PROTECTION_LOG
	log_protection(space, "info WRITE");
#endif
}


/***************************************************************************
                            Memory Maps - Main CPU
***************************************************************************/

WRITE8_MEMBER(quizpun2_state::quizpun2_rombank_w)
{
	UINT8 *ROM = memregion("maincpu")->base();
	membank("bank1")->set_base(&ROM[ 0x10000 + 0x2000 * (data & 0x1f) ] );
}

WRITE8_MEMBER(quizpun2_state::quizpun2_irq_ack)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

WRITE8_MEMBER(quizpun2_state::quizpun2_soundlatch_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( quizpun2_map, AS_PROGRAM, 8, quizpun2_state )
	AM_RANGE( 0x0000, 0x7fff ) AM_ROM
	AM_RANGE( 0x8000, 0x9fff ) AM_ROMBANK("bank1")

	AM_RANGE( 0xa000, 0xbfff ) AM_RAM_WRITE(fg_ram_w ) AM_SHARE("fg_ram")   // 4 * 800
	AM_RANGE( 0xc000, 0xc7ff ) AM_RAM_WRITE(bg_ram_w ) AM_SHARE("bg_ram")   // 4 * 400
	AM_RANGE( 0xc800, 0xcfff ) AM_RAM                                       //

	AM_RANGE( 0xd000, 0xd3ff ) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE( 0xe000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( quizpun2_io_map, AS_IO, 8, quizpun2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x40, 0x40 ) AM_WRITE(quizpun2_irq_ack )
	AM_RANGE( 0x50, 0x50 ) AM_WRITE(quizpun2_soundlatch_w )
	AM_RANGE( 0x60, 0x60 ) AM_WRITE(quizpun2_rombank_w )
	AM_RANGE( 0x80, 0x80 ) AM_READ_PORT( "DSW" )
	AM_RANGE( 0x90, 0x90 ) AM_READ_PORT( "IN0" )
	AM_RANGE( 0xa0, 0xa0 ) AM_READ_PORT( "IN1" )
	AM_RANGE( 0xe0, 0xe0 ) AM_READWRITE(quizpun2_protection_r, quizpun2_protection_w )
ADDRESS_MAP_END


/***************************************************************************
                            Memory Maps - Sound CPU
***************************************************************************/

static ADDRESS_MAP_START( quizpun2_sound_map, AS_PROGRAM, 8, quizpun2_state )
	AM_RANGE( 0x0000, 0xf7ff ) AM_ROM
	AM_RANGE( 0xf800, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( quizpun2_sound_io_map, AS_IO, 8, quizpun2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_WRITENOP  // IRQ end
	AM_RANGE( 0x20, 0x20 ) AM_WRITENOP  // NMI end
	AM_RANGE( 0x40, 0x40 ) AM_READ(soundlatch_byte_r )
	AM_RANGE( 0x60, 0x61 ) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write )
ADDRESS_MAP_END


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( quizpun2 )
	PORT_START("DSW")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x30, "Play Time" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0x00, "9" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x16x8 =
{
	8, 16,
	RGN_FRAC(1, 1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP16(0,8*8) },
	8*16*8
};

static const gfx_layout layout_8x16x2 =
{
	8, 16,
	RGN_FRAC(1, 1),
	2,
	{ 0,1 },
	{ STEP4(3*2,-2),STEP4(7*2,-2) },
	{ STEP16(0,8*2) },
	8*16*2
};

static GFXDECODE_START( quizpun2 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x16x8, 0,  1*2 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x16x2, 0, 64*2 )
	GFXDECODE_ENTRY( "gfx3", 0, layout_8x16x2, 0, 64*2 )
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

static MACHINE_CONFIG_START( quizpun2, quizpun2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_8MHz / 2) // 4 MHz?
	MCFG_CPU_PROGRAM_MAP(quizpun2_map)
	MCFG_CPU_IO_MAP(quizpun2_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", quizpun2_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz / 2)    // 4 MHz?
	MCFG_CPU_PROGRAM_MAP(quizpun2_sound_map)
	MCFG_CPU_IO_MAP(quizpun2_sound_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", quizpun2_state,  irq0_line_hold)
	// NMI generated by main CPU


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(quizpun2_state, screen_update_quizpun2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", quizpun2)
	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_8MHz / 4 ) // 2 MHz?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( quizpun2 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "u111", 0x00000, 0x08000, CRC(14bdaffc) SHA1(7fb5988ea565d7cbe3c8e2cdb9402d3cf81507d7) )
	ROM_LOAD( "u117", 0x10000, 0x10000, CRC(e9d1d05e) SHA1(c24104e023d12db8c9199d3e18750414aa511e40) )
	ROM_LOAD( "u118", 0x20000, 0x10000, CRC(1f232707) SHA1(3f5f44611f25c556521333f15daf3e2128cc1538) BAD_DUMP ) // fails rom check
	ROM_LOAD( "u119", 0x30000, 0x10000, CRC(c73b82f7) SHA1(d5c683440e9db46dd5859b519b3f32da80352626) )
	ROM_LOAD( "u120", 0x40000, 0x10000, CRC(700648b8) SHA1(dfa824166dfe7361d7c2ab0d8aa1ada882916cb9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "u22", 0x00000, 0x10000, CRC(f40768b5) SHA1(4410f71850357ec1d10a3a114bb540966e72781b) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "mcu.bin", 0x0000, 0x1000, NO_DUMP ) // could be a state machine instead

	ROM_REGION( 0x40000, "gfx1", 0 )    // 8x16x8
	ROM_LOAD( "u21", 0x00000, 0x10000, CRC(8ac86759) SHA1(2eac9ceee4462ce905aa08ff4f5a6215e0b6672f) )
	ROM_LOAD( "u20", 0x10000, 0x10000, CRC(67640a46) SHA1(5b33850afbb89db9ce9044a578423bfe3a55420d) )
	ROM_LOAD( "u29", 0x20000, 0x10000, CRC(cd8ff05b) SHA1(25e5be914fe49ff96a3c04de0c0e266a79068930) )
	ROM_LOAD( "u30", 0x30000, 0x10000, CRC(8612b443) SHA1(1033a378b21023eca471f43309d49461494b5ea1) )

	ROM_REGION( 0x6000, "gfx2", 0 ) // 8x16x2
	ROM_LOAD( "u26", 0x1000, 0x1000, CRC(151de8af) SHA1(2159ab030043e69d63cc9fbbc772f5bae8ab3f9d) )
	ROM_CONTINUE(    0x0000, 0x1000 )
	ROM_LOAD( "u27", 0x3000, 0x1000, CRC(2afdafea) SHA1(4c116a1e8a91f2e309646063139763b837e24bc7) )
	ROM_CONTINUE(    0x2000, 0x1000 )
	ROM_LOAD( "u28", 0x5000, 0x1000, CRC(c8bd85ad) SHA1(e7f0882f669edea1bb4634c263872f63da6a3290) )
	ROM_CONTINUE(    0x4000, 0x1000 )

	ROM_REGION( 0x20000, "gfx3", 0 )    // 8x16x2
	ROM_LOAD( "u1", 0x00000, 0x10000, CRC(58506040) SHA1(9d8bed2585e8f188a20270fccd9cfbdb91e48599) )
	ROM_LOAD( "u2", 0x10000, 0x10000, CRC(9294a19c) SHA1(cd7109262e5f68b946c84aa390108bcc47ee1300) )

	ROM_REGION( 0x80, "eeprom", 0 ) // EEPROM (tied to the unknown DIP40)
	ROM_LOAD( "93c46", 0x00, 0x80, CRC(4d244cc8) SHA1(6593d5b7ac1ebb77fee4648ad1d3d9b59a25fdc8) )

	ROM_REGION( 0x2000, "unknown", 0 )
	ROM_LOAD( "u2a", 0x0000, 0x2000, CRC(13afc2bd) SHA1(0d9c8813525dfc7a844e72d2cf84261db3d10a23) )
ROM_END

ROM_START( quizpun )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "15.u111", 0x00000, 0x08000, CRC(0ffe42d9) SHA1(f91e499800923d185a5d3514fc4c50e5c86378bf) )
	ROM_LOAD( "11.u117", 0x10000, 0x10000, CRC(13541476) SHA1(5e81e4143fbc8fa68c2c7d54792a432e97964d7f) )
	ROM_LOAD( "12.u118", 0x20000, 0x10000, CRC(678b57c1) SHA1(83869e5b6fe528c0b072f7d97338febc31db9f8b) )
	ROM_LOAD( "13.u119", 0x30000, 0x10000, CRC(9c0ee0de) SHA1(14b148f3ca951a5a9010b4d253e3ba7d35708403) )
	ROM_LOAD( "14.u120", 0x40000, 0x10000, CRC(21c11262) SHA1(e50678fafdf775a49ef96f8837b124824a2d1ca2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "05.u22", 0x00000, 0x10000, CRC(515f337e) SHA1(21b2cca95b5da934fd8139892c2ee2c623d51a4e) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "mcu.bin", 0x0000, 0x1000, NO_DUMP ) // could be a state machine instead

	ROM_REGION( 0x40000, "gfx1", 0 )    // 8x16x8
	ROM_LOAD( "04.u21", 0x00000, 0x10000, CRC(fa8d64f4) SHA1(71badabf8f34f246dec83323a1cddbe74deb91bd) )
	ROM_LOAD( "03.u20", 0x10000, 0x10000, CRC(8dda8167) SHA1(42838cf6866fb1d59c5bb3b477053aac448e7760) )
	ROM_LOAD( "09.u29", 0x20000, 0x10000, CRC(b9f28569) SHA1(1395cd226d314ee57385eed25f28b68607bfda53) )
	ROM_LOAD( "10.u30", 0x30000, 0x10000, CRC(db5762c0) SHA1(606dc4a3e6b8034f063f11dcf0a2b1db59838f4c) )

	ROM_REGION( 0xc000, "gfx2", 0 ) // 8x16x2
	ROM_LOAD( "06.u26", 0x1000, 0x1000, CRC(6d071b6d) SHA1(19565c8d768eeecd4119677915cc06f3ea18a47a) )
	ROM_CONTINUE(    0x0000, 0x1000 )
	ROM_LOAD( "07.u27", 0x3000, 0x1000, CRC(0f8b516e) SHA1(8bfabfd0bd28a1c7ddd01586fe9757b241feb59b) )
	ROM_CONTINUE(    0x2000, 0x1000 )
	ROM_CONTINUE(    0x6000, 0x6000 ) // ??
	ROM_LOAD( "08.u28", 0x5000, 0x1000, CRC(51c0c5cb) SHA1(0c7bfc9b6b3ce0cdd5c0e36df2b4d90f9cff7fae) )
	ROM_CONTINUE(    0x4000, 0x1000 )

	ROM_REGION( 0x20000, "gfx3", 0 )    // 8x16x2
	ROM_LOAD( "01.u1", 0x00000, 0x10000, CRC(58506040) SHA1(9d8bed2585e8f188a20270fccd9cfbdb91e48599) )
	ROM_LOAD( "02.u2", 0x10000, 0x10000, CRC(9294a19c) SHA1(cd7109262e5f68b946c84aa390108bcc47ee1300) )

	ROM_REGION( 0x80, "eeprom", ROMREGION_ERASEFF )

ROM_END

GAME( 1988, quizpun, 0, quizpun2, quizpun2, driver_device, 0, ROT270, "Space Computer", "Quiz Punch", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 1989, quizpun2, 0, quizpun2, quizpun2, driver_device, 0, ROT270, "Space Computer", "Quiz Punch 2", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
