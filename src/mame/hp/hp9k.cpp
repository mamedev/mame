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
428000-428fff - keyboard 8042 host status / data ports (8042 undumped)
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
//#include "machine/ins8250.h"
#include "emupal.h"
#include "screen.h"


namespace {

#define HP9816_CHDIMX 8
#define HP9816_CHDIMY 16
#define HP9816_ROWX 80
#define HP9816_ROWY 25

//

static uint8_t prom16a[256] = {
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
	0xff,0xfe,0x00,0x00,            // bottom minimum address for ram size
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
public:
	hp9k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_6845(*this, "mc6845"),
		m_gfxdecode(*this, "gfxdecode")
	{
		kbdBit=0;
		crtc_curreg=0;
		crtc_addrStartHi=0;
		crtc_addrStartLow=0;
		calc_prom_crc(prom16a);
	}

	void hp9k(machine_config &config);

	void init_hp9k();

private:

	int crtc_curreg;
	int crtc_addrStartHi;
	int crtc_addrStartLow;

	void calc_prom_crc(uint8_t* prom);
	void putChar(uint8_t thec,int x,int y,bitmap_ind16 &bitmap);

	uint8_t kbdBit;

	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_6845;

	uint8_t m_videoram[0x4000];
	uint8_t m_screenram[0x800];

	uint16_t hp9k_videoram_r(offs_t offset, uint16_t mem_mask = ~0);
	void hp9k_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t hp9k_prom_r(offs_t offset, uint16_t mem_mask = ~0);
	void hp9k_prom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t keyboard_r(offs_t offset, uint16_t mem_mask = ~0);
	void keyboard_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t leds_r(offs_t offset, uint16_t mem_mask = ~0);
	void leds_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	[[maybe_unused]] void kbd_put(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<gfxdecode_device> m_gfxdecode;
	void hp9k_mem(address_map &map) ATTR_COLD;
};

//

void hp9k_state::calc_prom_crc(uint8_t* prom)
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
		prom[0]=(uint8_t)(chksum>>8);
		prom[1]=(uint8_t)(chksum&0xff);
	}
}

uint16_t hp9k_state::keyboard_r(offs_t offset, uint16_t mem_mask)
{
	//printf("keyboard read at [%x] mask [%x]\n",offset,mem_mask);
	return 0x31;
}

void hp9k_state::keyboard_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("keyboard write of [%x] at [%x]\n",data,offset);
}

uint16_t hp9k_state::leds_r(offs_t offset, uint16_t mem_mask)
{
	//printf("warning: leds read at [%x] mm [%x]\n",offset,mem_mask);
	return 0;
}

void hp9k_state::leds_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("warning: leds write of [%x] at [%x] mm [%x]\n",data,offset,mem_mask);
}

uint16_t hp9k_state::hp9k_prom_r(offs_t offset, uint16_t mem_mask)
{
	//if (mem_mask!=0xff) printf("read of PROM at [%x] mem_mask [%x]\n",offset,mem_mask);
	int k=prom16a[offset&0xff];
	if (mem_mask==0xff00) return (k<<8);
	else return k;
	//return 0;
}

void hp9k_state::hp9k_prom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("Error: write to prom\n");
}

uint16_t hp9k_state::hp9k_videoram_r(offs_t offset, uint16_t mem_mask)
{
	offset&=0x3fff;

	//printf("videoram read at [%x] mem_mask [%x]\n",offset,mem_mask);

	if (offset==0x0001)
	{
		//printf("m6845 read at [%x] mem_mask [%x]\n",offset,mem_mask);
		return m_6845->register_r();
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

void hp9k_state::hp9k_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset&=0x3fff;

	if (offset==0x0000)
	{
		//printf("6845 address write [%x] at [%x] mask [%x]\n",data,offset,mem_mask);
		data&=0x1f;
		m_6845->address_w(data);
		crtc_curreg=data;
	}
	else if (offset==0x0001)
	{
		//printf("6845 register write [%x] at [%x] mask [%x]\n",data,offset,mem_mask);
		m_6845->register_w(data);
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

				//uint8_t *rom = machine().region("bootrom")->base();
				//rom[0x90B]=0x00;
			}
			else
			{
				m_screenram[offset&0x7ff]=data;
				m_videoram[offset&0x3fff]=data;
			}
	}
}

void hp9k_state::hp9k_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x000909).rom().region("bootrom", 0);
	map(0x00090a, 0x00090d).rw(FUNC(hp9k_state::leds_r), FUNC(hp9k_state::leds_w));
	map(0x00090e, 0x00ffff).rom().region("bootrom", 0x90e);
	map(0x010000, 0x427fff).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0x428000, 0x428fff).rw(FUNC(hp9k_state::keyboard_r), FUNC(hp9k_state::keyboard_w));
	map(0x429000, 0x50ffff).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0x510000, 0x51ffff).rw(FUNC(hp9k_state::hp9k_videoram_r), FUNC(hp9k_state::hp9k_videoram_w));
	map(0x520000, 0x52ffff).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0x530000, 0x53ffff).ram(); // graphic memory
	map(0x540000, 0x5effff).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0x5f0000, 0x5f3fff).rw(FUNC(hp9k_state::hp9k_prom_r), FUNC(hp9k_state::hp9k_prom_w));
	//map(0x5f0000, 0x5f3fff).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0x5f4000, 0xfbffff).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0xFC0000, 0xffffff).ram(); // system ram
}


/* Input ports */
static INPUT_PORTS_START( hp9k )
INPUT_PORTS_END


void hp9k_state::init_hp9k()
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

static GFXDECODE_START( gfx_hp9k )
	GFXDECODE_ENTRY( "bootrom", 0x2000, hp9k_charlayout, 0, 1 )
GFXDECODE_END

void hp9k_state::putChar(uint8_t thec,int x,int y,bitmap_ind16 &bitmap)
{
	uint8_t const *const pchar=m_gfxdecode->gfx(0)->get_data(thec);

	for (int py=0;py<HP9816_CHDIMY;py++)
	{
		for (int px=0;px<HP9816_CHDIMX;px++)
		{
			bitmap.pix((y*(HP9816_CHDIMY))+py,(x*(HP9816_CHDIMX))+px) = pchar[px+(py*8)];
		}
	}
}

uint32_t hp9k_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//uint8_t* pvram=&m_screenram[1];

	int startAddr=((crtc_addrStartLow&0xff)|((crtc_addrStartHi<<8)))&0x3fff;
	int chStart=startAddr&0x1fff;

	for (int r=0;r<HP9816_ROWY;r++)
	{
		for (int c=0;c<HP9816_ROWX;c++)
		{
			//uint8_t thec=pvram[((c+(r*80))+160+47)&0x7ff];
			//uint8_t thec=m_videoram[((c+(r*80))+startAddr)];
			uint8_t thec=m_screenram[chStart&0x7ff];
			putChar(thec,c,r,bitmap);
			chStart++;
		}
	}

	//putChar(0x44,0,0,pscr);
	return 0;
}

void hp9k_state::kbd_put(uint8_t data)
{
	kbdBit=data;
}


void hp9k_state::hp9k(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(8'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &hp9k_state::hp9k_mem);

	/* video hardware */

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(HP9816_ROWX*HP9816_CHDIMX, HP9816_ROWY*HP9816_CHDIMY);
	screen.set_visarea(0, (HP9816_ROWX*HP9816_CHDIMX)-1, 0, (HP9816_ROWY*HP9816_CHDIMY)-1);
	screen.set_screen_update(FUNC(hp9k_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_hp9k);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	MC6845(config, m_6845, XTAL(16'000'000) / 16);
	m_6845->set_screen("screen");
	m_6845->set_show_border_area(false);
	m_6845->set_char_width(8);
}

/* ROM definition */
ROM_START( hp9816 )
	ROM_REGION16_BE(0x10000, "bootrom", 0)

	ROM_DEFAULT_BIOS("bios40")
	ROM_SYSTEM_BIOS(0, "bios40",  "BIOS v4.0")
	ROMX_LOAD( "rom40.bin", 0x0000, 0x10000, CRC(36005480) SHA1(645a077ffd95e4c31f05cd8bbd6e4554b12813f1), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "bios30",  "BIOS v3.0")
	ROMX_LOAD( "rom30.bin", 0x0000, 0x10000, CRC(05c07e75) SHA1(3066a65e6137482041f9a77d09ee2289fe0974aa), ROM_BIOS(1) )

ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT       COMPANY            FULLNAME   FLAGS */
COMP( 1982, hp9816, 0,      0,      hp9k,    hp9k,  hp9k_state, init_hp9k, "Hewlett Packard", "HP 9816", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
