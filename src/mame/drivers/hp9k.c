// license:BSD-3-Clause
// copyright-holders:Gabriele D'Antona, F. Ulivi

/***************************************************************************

HP 9816
1982
MESS driver by friol (dantonag (at) gmail.com)
System reference mainly from O.De Smet
Driver started on 10/01/2012

===

Memory map:

000000-00ffff - system rom
428000-428fff - keyboard
510000-51ffff - videoram
530000-53ffff - graphic memory
5f0000-5f3fff - system PROM
xxxxxx-ffffff - system RAM

===

- 0x408: startup code, 68000 registers test
- 0x1002: bootrom test
- 0x1d18: PROM test
- 0x103C: prints "bootrom failed"
- 0x1150: other boot test
- 0x1164: keyboard test
- 0x1202: post-kbd test
- 0x12c6: "loading memory" test

===

TODO: boot tests fail

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/mc6845.h"
#include "machine/terminal.h"
//#include "machine/ins8250.h"

#define HP9816_CHDIMX 8
#define HP9816_CHDIMY 16
#define HP9816_ROWX 80
#define HP9816_ROWY 25

//

static UINT8 prom16a[256] = {
	0x00,0x00,      // checksum
	0x00,           // 256 bytes idprom
		'2', '0', '1', '0', 'A', '0', '0', '0', '0', '0', '0',      // serial in ascii DDDDCSSSSSS date code, country, serial number
		'9', '8', '1', '6', 'A', ' ', ' ',                          // product number
	0xff,           // 8 bits processor board config
	0x01,           // keyboard 98203B
	0x02,           // CRT alpha see crtid for monitor
	0x03,           // HP-IB
	0x04,           // Graphics
	0xff,           // end
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,
	0xff,0xfe,0x00,0x00,            // bottom minimun address for ram size
	0xff,0xff,                      // 16 required IO cards here not used
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,
	0xff,0xff,0xff,0xff,            // boot msus to try before Boot list scan
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,   // Boot file name
	0x00,0x00,0x00,0x00,            // delay in millisec before boot scan
	0x00,                           // owner byte, HP format
	0x00,                           // id prom revision byte : 0x00
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,                                 // rest reserved at 0xFF
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

//

class hp9k_state : public driver_device
{
private:

	int crtc_curreg;
	int crtc_addrStartHi;
	int crtc_addrStartLow;

	void calc_prom_crc(UINT8* prom);
	void putChar(UINT8 thec,int x,int y,bitmap_ind16 &bitmap);

public:
	hp9k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	//m_terminal(*this, TERMINAL_TAG),
	m_6845(*this, "mc6845"),
	m_gfxdecode(*this, "gfxdecode")
	{
		kbdBit=0;
		crtc_curreg=0;
		crtc_addrStartHi=0;
		crtc_addrStartLow=0;
		calc_prom_crc(prom16a);
	}

	UINT8 kbdBit;

	required_device<cpu_device> m_maincpu;
	//required_device<> m_terminal;
	required_device<mc6845_device> m_6845;

	UINT8 m_videoram[0x4000];
	UINT8 m_screenram[0x800];

	DECLARE_DRIVER_INIT(hp9k);

	DECLARE_READ16_MEMBER(buserror_r);
	DECLARE_WRITE16_MEMBER(buserror_w);

	DECLARE_READ16_MEMBER(hp9k_videoram_r);
	DECLARE_WRITE16_MEMBER(hp9k_videoram_w);

	DECLARE_READ16_MEMBER(hp9k_prom_r);
	DECLARE_WRITE16_MEMBER(hp9k_prom_w);

	DECLARE_READ16_MEMBER(keyboard_r);
	DECLARE_WRITE16_MEMBER(keyboard_w);

	DECLARE_READ16_MEMBER(leds_r);
	DECLARE_WRITE16_MEMBER(leds_w);

	DECLARE_WRITE8_MEMBER(kbd_put);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<gfxdecode_device> m_gfxdecode;
};

//

void hp9k_state::calc_prom_crc(UINT8* prom)
{
	int chksum;
	int i;

	chksum = 0;
	for (i=0; i < 256; i+=2)
	{
		chksum += ((prom[i] << 8) | prom[i+1]);
		if (chksum&0x10000) chksum++;
		chksum&=0xffff;
	}

	chksum=(chksum+1)&0xffff;

	if (chksum!=0)
	{
		chksum=(0x10000-chksum);
		prom[0]=(UINT8)(chksum>>8);
		prom[1]=(UINT8)(chksum&0xff);
	}
}

READ16_MEMBER( hp9k_state::keyboard_r )
{
	//printf("keyboard read at [%x] mask [%x]\n",offset,mem_mask);
	return 0x31;
}

WRITE16_MEMBER( hp9k_state::keyboard_w )
{
	//printf("keyboard write of [%x] at [%x]\n",data,offset);
}

READ16_MEMBER( hp9k_state::leds_r )
{
	//printf("warning: leds read at [%x] mm [%x]\n",offset,mem_mask);
	return 0;
}

WRITE16_MEMBER( hp9k_state::leds_w )
{
	//printf("warning: leds write of [%x] at [%x] mm [%x]\n",data,offset,mem_mask);
}

READ16_MEMBER( hp9k_state::hp9k_prom_r )
{
	//if (mem_mask!=0xff) printf("read of PROM at [%x] mem_mask [%x]\n",offset,mem_mask);
	int k=prom16a[offset&0xff];
	if (mem_mask==0xff00) return (k<<8);
	else return k;
	//return 0;
}

WRITE16_MEMBER( hp9k_state::hp9k_prom_w )
{
	//printf("Error: write to prom\n");
}

READ16_MEMBER( hp9k_state::hp9k_videoram_r )
{
	offset&=0x3fff;

	//printf("videoram read at [%x] mem_mask [%x]\n",offset,mem_mask);

	if (offset==0x0001)
	{
		//printf("m6845 read at [%x] mem_mask [%x]\n",offset,mem_mask);
		return m_6845->register_r(space,0);
	}
	else
	{
		if (mem_mask==0xff00)
		{
			return m_videoram[offset]<<8;
		}
		else
		{
			return m_videoram[offset];
		}
	}
}

WRITE16_MEMBER( hp9k_state::hp9k_videoram_w )
{
	offset&=0x3fff;

	if (offset==0x0000)
	{
		//printf("6845 address write [%x] at [%x] mask [%x]\n",data,offset,mem_mask);
		data&=0x1f;
		m_6845->address_w( space, 0, data );
		crtc_curreg=data;
	}
	else if (offset==0x0001)
	{
		//printf("6845 register write [%x] at [%x] mask [%x]\n",data,offset,mem_mask);
		m_6845->register_w( space, 0, data );
		if (crtc_curreg==0x0c) crtc_addrStartHi=data;
		if (crtc_curreg==0x0d) crtc_addrStartLow=data;
	}
	else
	{
		//printf("videoram write [%x] at [%x]\n",data,offset);

		if (mem_mask==0xff00)
			{
				m_screenram[offset&0x7ff]=data>>8;
				m_videoram[offset&0x3fff]=data>>8;

				//UINT8 *rom = machine().region("bootrom")->base();
				//rom[0x90B]=0x00;
			}
			else
			{
				m_screenram[offset&0x7ff]=data;
				m_videoram[offset&0x3fff]=data;
			}
	}
}

READ16_MEMBER(hp9k_state::buserror_r)
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0;
}

WRITE16_MEMBER(hp9k_state::buserror_w)
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
}

static ADDRESS_MAP_START(hp9k_mem, AS_PROGRAM, 16, hp9k_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x000909) AM_ROM AM_REGION("bootrom",0)
	AM_RANGE(0x00090a, 0x00090d) AM_READWRITE(leds_r,leds_w)
	AM_RANGE(0x00090e, 0x00ffff) AM_ROM AM_REGION("bootrom",0x90e)
	AM_RANGE(0x010000, 0x427fff) AM_READWRITE(buserror_r,buserror_w)
	AM_RANGE(0x428000, 0x428fff) AM_READWRITE(keyboard_r,keyboard_w)
	AM_RANGE(0x429000, 0x50ffff) AM_READWRITE(buserror_r,buserror_w)
	AM_RANGE(0x510000, 0x51ffff) AM_READWRITE(hp9k_videoram_r,hp9k_videoram_w)
	AM_RANGE(0x520000, 0x52ffff) AM_READWRITE(buserror_r,buserror_w)
	AM_RANGE(0x530000, 0x53ffff) AM_RAM // graphic memory
	AM_RANGE(0x540000, 0x5effff) AM_READWRITE(buserror_r,buserror_w)
	AM_RANGE(0x5f0000, 0x5f3fff) AM_READWRITE(hp9k_prom_r,hp9k_prom_w)
	//AM_RANGE(0x5f0000, 0x5f3fff) AM_READWRITE(buserror_r,buserror_w)
	AM_RANGE(0x5f4000, 0xfbffff) AM_READWRITE(buserror_r,buserror_w)
	AM_RANGE(0xFC0000, 0xffffff) AM_RAM // system ram
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( hp9k )
INPUT_PORTS_END


DRIVER_INIT_MEMBER(hp9k_state,hp9k)
{
}

static const gfx_layout hp9k_charlayout =
{
	HP9816_CHDIMX, HP9816_CHDIMY,
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ STEP8(0,1) },
	/* y offsets */
	{ STEP16(0,8) },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( hp9k )
	GFXDECODE_ENTRY( "bootrom", 0x2000, hp9k_charlayout, 0, 1 )
GFXDECODE_END

void hp9k_state::putChar(UINT8 thec,int x,int y,bitmap_ind16 &bitmap)
{
	const UINT8* pchar=m_gfxdecode->gfx(0)->get_data(thec);

	for (int py=0;py<HP9816_CHDIMY;py++)
	{
		for (int px=0;px<HP9816_CHDIMX;px++)
		{
			UINT16 *dest=&bitmap.pix16((y*(HP9816_CHDIMY))+py,(x*(HP9816_CHDIMX))+px);
			*dest=pchar[px+(py*8)];
		}
	}
}

UINT32 hp9k_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//UINT8* pvram=&m_screenram[1];

	int startAddr=((crtc_addrStartLow&0xff)|((crtc_addrStartHi<<8)))&0x3fff;
	int chStart=startAddr&0x1fff;

	for (int r=0;r<HP9816_ROWY;r++)
	{
		for (int c=0;c<HP9816_ROWX;c++)
		{
			//UINT8 thec=pvram[((c+(r*80))+160+47)&0x7ff];
			//UINT8 thec=m_videoram[((c+(r*80))+startAddr)];
			UINT8 thec=m_screenram[chStart&0x7ff];
			putChar(thec,c,r,bitmap);
			chStart++;
		}
	}

	//putChar(0x44,0,0,pscr);
	return 0;
}

WRITE8_MEMBER( hp9k_state::kbd_put )
{
	kbdBit=data;
}


static MACHINE_CONFIG_START( hp9k, hp9k_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(hp9k_mem)

	/* video hardware */

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(HP9816_ROWX*HP9816_CHDIMX, HP9816_ROWY*HP9816_CHDIMY)
	MCFG_SCREEN_VISIBLE_AREA(0, (HP9816_ROWX*HP9816_CHDIMX)-1, 0, (HP9816_ROWY*HP9816_CHDIMY)-1)
	MCFG_SCREEN_UPDATE_DRIVER(hp9k_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hp9k)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_MC6845_ADD("mc6845", MC6845, "screen", XTAL_16MHz / 16)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( hp9816 )
	ROM_REGION16_BE(0x10000, "bootrom", 0)

	ROM_DEFAULT_BIOS("bios40")
	ROM_SYSTEM_BIOS(0, "bios40",  "Bios v4.0")
	ROMX_LOAD( "rom40.bin", 0x0000, 0x10000, CRC(36005480) SHA1(645a077ffd95e4c31f05cd8bbd6e4554b12813f1), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "bios30",  "Bios v3.0")
	ROMX_LOAD( "rom30.bin", 0x0000, 0x10000, CRC(05c07e75) SHA1(3066a65e6137482041f9a77d09ee2289fe0974aa), ROM_BIOS(2) )

ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1982, hp9816, 0,      0,      hp9k,   hp9k, hp9k_state,       hp9k,   "Hewlett Packard",  "HP 9816" ,  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
