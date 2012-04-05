/*

ADP (Merkur?) games from '90 running on similar hardware.
(68k + HD63484 + YM2149)

Skeleton driver by TS -  analog at op.pl

TODO:
(almost everything)
 - add sound and i/o
 - protection in Fashion Gambler (NVRam based?)

Supported games :
- Quick Jack      ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1993")
- Skat TV           ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1994")
- Skat TV v. TS3  ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1995")
- Fashion Gambler ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1997")
- Backgammon        ("COPYRIGHT BY ADP LUEBBECKE GERMANY 1994")
- Funny Land de Luxe ("Copyright 1992-99 by Stella International Germany")
- Fun Station Spielekoffer 9 Spiele ("COPYRIGHT BY ADP LUEBBECKE GERMANY 2000")


Skat TV (Version TS3)
Three board stack.

CPU Board:
----------
 ____________________________________________________________
 |           ______________  ______________     ___________ |
 | 74HC245N  | t1 i       |  |KM681000ALP7|     |+        | |
 | 74HC573   |____________|  |____________|     |  3V Bat | |
 |                                              |         | |
 |           ______________  ______________     |        -| |
 |           | t1 ii      |  |KM681000ALP7|     |_________| |
 |     |||   |____________|  |____________| |||             |
 |     |||   ___________                    |||  M62X42B    |
 | X   |||   |         |                    |||             |
 |     |||   |68EC000 8|  74HC32   74HC245  |||  MAX691CPE  |
 |     |||   |         |  74AC138  74HC573  |||    74HC32   |
 |           |         |                                    |
 | 74HC573   |_________|  74HC08   74HC10  74HC32  74HC21   |
 |__________________________________________________________|

Parts:

 68EC000FN8         - Motorola 68k CPU
 KM681000ALP7       - 128K X 8 Bit Low Power CMOS Static RAM
 OKIM62X42B         - Real-time Clock ic With Built-in Crystal
 MAX691CPE          - P Reset ic With Watchdog And Battery Switchover
 X                    - 8MHz xtal
 3V Bat             - Lithium 3V power module

Video Board:
------------
 ____________________________________________________________
 |           ______________  ______________                 |
 |           | t2 i       |  |KM681000ALP7|     74HC573     |
 |           |____________|  |____________|                *|
 |                                              74HC573    *|
 |           ______________  ______________                *|
 |           | t2 ii      |  |KM681000ALP7|               P3|
 |       ||| |____________|  |____________|   |||          *|
 |       ||| ___________                      |||          *|
 |       ||| |         |                      |||          *|
 |       ||| | HD63484 |  74HC04   74HC00     |||         P6|
 |       ||| |         |  74HC74   74HC08     |||  74HC245  |
 |           |         |                                    |
 | 74HC573   |_________|  74HC166  74HC166 74HC166 74HC166  |
 |__________________________________________________________|

Parts:

 HD63484CP8         - Advanced CRT Controller
 KM681000ALP7       - 128K X 8 Bit Low Power CMOS Static RAM

Connectors:

 Two connectors to link with CPU Board
 Two connectors to link with Sound and I/O Board
 P3  - Monitor
 P6  - Lightpen

Sound  and I/O board:
---------------------
 _________________________________________________________________________________
 |                        TS271CN    74HC02                        ****  ****    |
 |*                      ________________                          P1    P2     *|
 |*         74HC574      | YM2149F      |                                       *|
 |*                  ||| |______________|   74HC393  74HC4015 |||               *|
 |P3        74HC245  |||                                      |||              P6|
 |*                  ||| ________________          X          ||| TL7705ACP     *|
 |*                  ||| |SCN68681C1N40 |                     |||               *|
 |*                  ||| |______________|   74HC32   74AC138  |||               *|
 |P7                 |||                                      |||              P8|
 |*                        TC428CPA                                             *|
 |*                                                                             *|
 |*    P11  P12    P13    P14       P15   P16   P17      P18   P19   P20  P21   *|
 |P9   **** *****  *****  ****  OO  ****  ****  *******  ****  ****  ***  *** P10|
 |_______________________________________________________________________________|

Parts:

 YM2149F         - Yamaha PSG
 SCN68681C1N40   - Dual Asynchronous Receiver/transmitter (DUART);
 TS271CN         - Programmable Low Power CMOS Single Op-amp
 TL7705ACP       - Supply Voltage Supervisor
 TC428CPA        - Dual CMOS High-speed Driver
 OO              - LEDs (red)
 X               - 3.6864MHz xtal

Connectors:

 Two connectors to link with Video Board
 P1  - Tueroeffn
 P2  - PSG In/Out
 P3  - Lautsprecher
 P6  - Service - Tast.
 P7  - Maschine (barely readable)
 P8  - Muenzeinheit
 P9  - Atzepter
 P10 - Reset Fadenfoul
 P11 - Netzteil
 P12 - Serienplan
 P13 - Serienplan 2
 P14 - Muenzeinheit 2
 P15 - I2C Bus
 P16 - Kodierg.
 P17 - TTL Ein-Aueg.
 P18 - Out
 P19 - In
 P20 - Serielle-S.
 P21 - Tuerschalter

There's also (external) JAMMA adapter - 4th board filled with resistors and diodes.



Funny Land de Luxe
------------------

Video board has additional chips:
  - Altera EPM7032 (PLD)
  - SG-615PH (32.0000M oscillator)
  - Bt481 (RAMDAC)

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/ay8910.h"
#include "video/h63484.h"
#include "machine/microtch.h"
#include "machine/68681.h"

class adp_state : public driver_device
{
public:
	adp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_h63484(*this, "h63484"),
		  m_microtouch(*this, "microtouch")
		{ }

	required_device<h63484_device> m_h63484;
	required_device<microtouch_device> m_microtouch;

	DECLARE_WRITE8_MEMBER(microtouch_tx);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	/* misc */
	UINT8 m_mux_data;
	UINT8 m_register_active;
	struct { int r,g,b,offs,offs_internal; } m_pal;

	/* devices */
	device_t *m_maincpu;
	device_t *m_duart;
	DECLARE_READ16_MEMBER(test_r);
	DECLARE_WRITE16_MEMBER(wh2_w);
	DECLARE_WRITE8_MEMBER(ramdac_io_w);
	DECLARE_READ8_MEMBER(h63484_rom_r);
};


static H63484_DISPLAY_PIXELS( acrtc_display_pixels )
{
	bitmap.pix16(y, x) = data & 0xf;
}

UINT32 adp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	/* graphics */
	m_h63484->update_screen(screen, bitmap, cliprect);

	return 0;
}


#if 0
static SCREEN_UPDATE_IND16( adp )
{
	adp_state *state = screen.machine().driver_data<adp_state>();

	state->m_h63484->update_screen(bitmap, cliprect);

	#if 0
	adp_state *state = screen.machine().driver_data<adp_state>();
	int x, y, b, src;

	b = ((hd63484_regs_r(state->m_hd63484, 0xcc/2, 0xffff) & 0x000f) << 16) + hd63484_regs_r(state->m_hd63484, 0xce/2, 0xffff);

	for (y = 0;y < 280;y++)
	{
		for (x = 0 ; x < (hd63484_regs_r(state->m_hd63484, 0xca/2, 0xffff) & 0x0fff) * 4 ; x += 4)
		{
			b &= (HD63484_RAM_SIZE - 1);
			src = hd63484_ram_r(state->m_hd63484, b, 0xffff);
			bitmap.pix16(y, x    ) = ((src & 0x000f) >>  0) << 0;
			bitmap.pix16(y, x + 1) = ((src & 0x00f0) >>  4) << 0;
			bitmap.pix16(y, x + 2) = ((src & 0x0f00) >>  8) << 0;
			bitmap.pix16(y, x + 3) = ((src & 0xf000) >> 12) << 0;
			b++;
		}
	}
if (!screen.machine().input().code_pressed(KEYCODE_O)) // debug: toggle window
	if ((hd63484_regs_r(state->m_hd63484, 0x06/2, 0xffff) & 0x0300) == 0x0300)
	{
		int sy = (hd63484_regs_r(state->m_hd63484, 0x94/2, 0xffff) & 0x0fff) - (hd63484_regs_r(state->m_hd63484, 0x88/2, 0xffff) >> 8);
		int h = hd63484_regs_r(state->m_hd63484, 0x96/2, 0xffff) & 0x0fff;
		int sx = ((hd63484_regs_r(state->m_hd63484, 0x92/2, 0xffff) >> 8) - (hd63484_regs_r(state->m_hd63484, 0x84/2, 0xffff) >> 8)) * 2 * 2;
		int w = (hd63484_regs_r(state->m_hd63484, 0x92/2, 0xffff) & 0xff) * 2;
		if (sx < 0) sx = 0;	// not sure about this (shangha2 title screen)

		b = (((hd63484_regs_r(state->m_hd63484, 0xdc/2, 0xffff) & 0x000f) << 16) + hd63484_regs_r(state->m_hd63484, 0xde/2, 0xffff));

		for (y = sy ; y <= sy + h && y < 280 ; y++)
		{
			for (x = 0 ; x < (hd63484_regs_r(state->m_hd63484, 0xca/2, 0xffff) & 0x0fff) * 4 ; x += 4)
			{
				b &= (HD63484_RAM_SIZE - 1);
				src = hd63484_ram_r(state->m_hd63484, b, 0xffff);

				if (x <= w && x + sx >= 0 && x + sx < (hd63484_regs_r(state->m_hd63484, 0xca/2, 0xffff) & 0x0fff) * 4)
				{
					bitmap.pix16(y, x + sx    ) = ((src & 0x000f) >>  0) << 0;
					bitmap.pix16(y, x + sx + 1) = ((src & 0x00f0) >>  4) << 0;
					bitmap.pix16(y, x + sx + 2) = ((src & 0x0f00) >>  8) << 0;
					bitmap.pix16(y, x + sx + 3) = ((src & 0xf000) >> 12) << 0;
				}
				b++;
			}
		}
	}
	#endif

	return 0;
}
#endif

/***************************************************************************

    68681 DUART <-> Microtouch touch screen controller communication

***************************************************************************/

static void duart_irq_handler( device_t *device, UINT8 vector )
{
	adp_state *state = device->machine().driver_data<adp_state>();
	device_set_input_line_and_vector(state->m_maincpu, 4, HOLD_LINE, vector);
}

static void duart_tx( device_t *device, int channel, UINT8 data )
{
	adp_state *state = device->machine().driver_data<adp_state>();
	if (channel == 0)
	{
		state->m_microtouch->rx(*device->machine().memory().first_space(), 0, data);
	}
}

WRITE8_MEMBER( adp_state::microtouch_tx )
{
	duart68681_rx_data(m_duart, 0, data);
}

static UINT8 duart_input( device_t *device )
{
	return input_port_read(device->machine(), "DSW1");
}

static const microtouch_interface adb_microtouch_config =
{
	DEVCB_DRIVER_MEMBER(adp_state, microtouch_tx),
	NULL
};

static MACHINE_START( skattv )
{
	adp_state *state = machine.driver_data<adp_state>();

	state->m_maincpu = machine.device("maincpu");
	state->m_duart = machine.device("duart68681");
	//state->m_h63484 = machine.device("h63484");

	state->save_item(NAME(state->m_mux_data));
	state->save_item(NAME(state->m_register_active));

	#if 0
	/*
        ACRTC memory:

        00000-3ffff = RAM
        40000-7ffff = ROM
        80000-bffff = unused
        c0000-fffff = unused
    */

	// hack to handle acrt rom
	{
		UINT16 *rom = (UINT16*)machine.region("gfx1")->base();
		int i;

		device_t *hd63484 = machine.device("hd63484");

		for(i = 0; i < 0x40000/2; ++i)
		{
			hd63484_ram_w(hd63484, i + 0x00000/2, rom[i], 0xffff);
			hd63484_ram_w(hd63484, i + 0x40000/2, rom[i], 0xffff);
			hd63484_ram_w(hd63484, i + 0x80000/2, rom[i], 0xffff);
			hd63484_ram_w(hd63484, i + 0xc0000/2, rom[i], 0xffff);
		}
	}
	#endif
}

static MACHINE_RESET( skattv )
{
	adp_state *state = machine.driver_data<adp_state>();

	state->m_mux_data = 0;
	state->m_register_active = 0;
}

static const duart68681_config skattv_duart68681_config =
{
	duart_irq_handler,
	duart_tx,
	duart_input,
	NULL
};

static PALETTE_INIT( adp )
{
    int i;

    for (i = 0; i < machine.total_colors(); i++)
    {
        int bit0, bit1, bit2, r, g, b;


        // red component
        bit0 = (i >> 0) & 0x01;
        bit1 = (i >> 3) & 0x01;
        bit2 = (i >> 0) & 0x01;
        r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
        // green component
        bit0 = (i >> 1) & 0x01;
        bit1 = (i >> 3) & 0x01;
        bit2 = (i >> 1) & 0x01;
        g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
        // blue component
        bit0 = (i >> 2) & 0x01;
        bit1 = (i >> 3) & 0x01;
        bit2 = (i >> 2) & 0x01;
        b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

        palette_set_color(machine, i, MAKE_RGB(r,g,b));
    }
}

READ16_MEMBER(adp_state::test_r)
{
	int value = 0xffff;

	switch (m_mux_data)
	{
		case 0x00: value = input_port_read(machine(), "x0"); break;
		case 0x01: value = input_port_read(machine(), "x1"); break;
		case 0x02: value = input_port_read(machine(), "x2"); break;
		case 0x03: value = input_port_read(machine(), "1P_UP"); break;
		case 0x04: value = input_port_read(machine(), "1P_B1"); break;
		case 0x05: value = input_port_read(machine(), "x5"); break;
		case 0x06: value = input_port_read(machine(), "1P_RIGHT"); break;
		case 0x07: value = input_port_read(machine(), "1P_DOWN"); break;
		case 0x08: value = input_port_read(machine(), "1P_LEFT"); break;
		case 0x09: value = input_port_read(machine(), "x9"); break;
		case 0x0a: value = input_port_read(machine(), "x10"); break;
		case 0x0b: value = input_port_read(machine(), "x11"); break;
		case 0x0c: value = input_port_read(machine(), "x12"); break;
		case 0x0d: value = input_port_read(machine(), "x13"); break;
		case 0x0e: value = input_port_read(machine(), "1P_START"); break;
		case 0x0f: value = input_port_read(machine(), "1P_COIN"); break;
	}

	m_mux_data++;
	m_mux_data &= 0xf;
/*
    switch (machine().rand() & 3)
    {
        case 0:
            return 0;
        case 1:
            return 0xffff;
        default:
            return machine().rand() & 0xffff;
    }
*/
	return value | (machine().rand() & 0x0000);
}

/*???*/
WRITE16_MEMBER(adp_state::wh2_w)
{
	m_register_active = data;
}

static READ8_DEVICE_HANDLER(t2_r)
{
	UINT8 res;
	int h,w;
	res = 0;
	h = device->machine().primary_screen->height();
	w = device->machine().primary_screen->width();

//  popmessage("%d %d",h,w);

	if (device->machine().primary_screen->hpos() > h)
		res|= 0x20; //hblank

	if (device->machine().primary_screen->vpos() > w)
		res|= 0x40; //vblank

	return res;
}

static ADDRESS_MAP_START( skattv_mem, AS_PROGRAM, 16, adp_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x800080, 0x800081) AM_DEVREADWRITE("h63484", h63484_device, status_r, address_w)
	AM_RANGE(0x800082, 0x800083) AM_DEVREADWRITE("h63484", h63484_device, data_r, data_w)
	AM_RANGE(0x800100, 0x800101) AM_READWRITE(test_r,wh2_w) //related to input
	AM_RANGE(0x800140, 0x800143) AM_DEVREADWRITE8_LEGACY("aysnd", ay8910_r, ay8910_address_data_w, 0x00ff) //18b too
	AM_RANGE(0x800180, 0x80019f) AM_DEVREADWRITE8_LEGACY("duart68681", duart68681_r, duart68681_w, 0xff )
	AM_RANGE(0xffc000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( quickjac_mem, AS_PROGRAM, 16, adp_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
//  AM_RANGE(0x400000, 0x40001f) ?
	AM_RANGE(0x800080, 0x800081) AM_DEVREADWRITE("h63484", h63484_device, status_r, address_w) // bad
	AM_RANGE(0x800082, 0x800083) AM_DEVREADWRITE("h63484", h63484_device, data_r, data_w) // bad
	AM_RANGE(0x800140, 0x800143) AM_DEVREADWRITE8_LEGACY("aysnd", ay8910_r, ay8910_address_data_w, 0x00ff) //18b too
	AM_RANGE(0x800180, 0x80019f) AM_DEVREADWRITE8_LEGACY("duart68681", duart68681_r, duart68681_w, 0xff )
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( backgamn_mem, AS_PROGRAM, 16, adp_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x100000, 0x10003f) AM_RAM
	AM_RANGE(0x200000, 0x20003f) AM_RAM
	AM_RANGE(0x400000, 0x40001f) AM_DEVREADWRITE8_LEGACY("duart68681", duart68681_r, duart68681_w, 0xff )
	AM_RANGE(0x500000, 0x503fff) AM_RAM //work RAM
	AM_RANGE(0x600006, 0x600007) AM_NOP //(r) is discarded (watchdog?)
ADDRESS_MAP_END

WRITE8_MEMBER(adp_state::ramdac_io_w)
{
	switch(offset)
	{
		case 0:
			m_pal.offs = data;
			m_pal.offs_internal = 0;
			break;
		case 2:
			//mask pen reg
			break;
		case 1:
			switch(m_pal.offs_internal)
			{
				case 0:
					m_pal.r = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					m_pal.offs_internal++;
					break;
				case 1:
					m_pal.g = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					m_pal.offs_internal++;
					break;
				case 2:
					m_pal.b = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					palette_set_color(machine(), m_pal.offs, MAKE_RGB(m_pal.r, m_pal.g, m_pal.b));
					m_pal.offs_internal = 0;
					m_pal.offs++;
					m_pal.offs&=0xff;
					break;
			}

			break;
	}
}

static ADDRESS_MAP_START( funland_mem, AS_PROGRAM, 16, adp_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x800080, 0x800081) AM_DEVREADWRITE("h63484", h63484_device, status_r, address_w)
	AM_RANGE(0x800082, 0x800083) AM_DEVREADWRITE("h63484", h63484_device, data_r, data_w)
	AM_RANGE(0x800088, 0x80008d) AM_WRITE8(ramdac_io_w, 0x00ff)
	AM_RANGE(0x800100, 0x800101) AM_RAM //???
	AM_RANGE(0x800140, 0x800143) AM_DEVREADWRITE8_LEGACY("aysnd", ay8910_r, ay8910_address_data_w, 0x00ff) //18b too
	AM_RANGE(0x800180, 0x80019f) AM_DEVREADWRITE8_LEGACY("duart68681", duart68681_r, duart68681_w, 0xff )
	AM_RANGE(0xfc0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( fstation_mem, AS_PROGRAM, 16, adp_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	//400000-40001f?
	AM_RANGE(0x800080, 0x800081) AM_DEVREADWRITE("h63484", h63484_device, status_r, address_w)
	AM_RANGE(0x800082, 0x800083) AM_DEVREADWRITE("h63484", h63484_device, data_r, data_w)
	AM_RANGE(0x800100, 0x800101) AM_RAM //???
	AM_RANGE(0x800140, 0x800143) AM_DEVREADWRITE8_LEGACY("aysnd", ay8910_r, ay8910_address_data_w, 0x00ff) //18b too
	AM_RANGE(0x800180, 0x80019f) AM_DEVREADWRITE8_LEGACY("duart68681", duart68681_r, duart68681_w, 0xff )
	AM_RANGE(0xfc0000, 0xffffff) AM_RAM
ADDRESS_MAP_END


#if 0
static INPUT_PORTS_START( adp )

INPUT_PORTS_END
#endif

static INPUT_PORTS_START( skattv )
	PORT_START("DSW1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH,  IPT_COIN5    )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_COIN6    )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_BILL1    )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN  )

	PORT_START("x0") //vblank status?
	PORT_DIPNAME( 0x0004,0x0004, "SW0" )
	PORT_DIPSETTING(     0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )

	PORT_START("x1")
	PORT_DIPNAME( 0x0004,0x0004, "SW1" ) //another up button
	PORT_DIPSETTING(     0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )

	PORT_START("x2")
	PORT_DIPNAME( 0x0004,0x0004, "SW2" )
	PORT_DIPSETTING(     0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_START("1P_UP")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_START("1P_B1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_START("x5")
	PORT_DIPNAME( 0x0004,0x0004, "SW5" )
	PORT_DIPSETTING(     0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_START("1P_RIGHT")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_START("1P_DOWN")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW, IPT_UNUSED  )
	PORT_START("1P_LEFT")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_START("x9")
	PORT_DIPNAME( 0x0004,0x0004, "SW9" )
	PORT_DIPSETTING(     0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_START("x10") //button 2
	PORT_DIPNAME( 0x0004,0x0004, "SW10" )
	PORT_DIPSETTING(     0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_START("x11")
	PORT_DIPNAME( 0x0004,0x0004, "SW11" )
	PORT_DIPSETTING(     0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_START("x12") //button 3
	PORT_DIPNAME( 0x0004,0x0004, "SW12" )
	PORT_DIPSETTING(     0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_START("x13")
	PORT_DIPNAME( 0x0004,0x0004, "SW13" )
	PORT_DIPSETTING(     0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(     0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_START("1P_START")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
	PORT_START("1P_COIN")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW,  IPT_UNUSED  )
INPUT_PORTS_END

/*
static INTERRUPT_GEN( adp_int )
{
    device_set_input_line(device, 1, HOLD_LINE); // ??? All irqs have the same vector, and the mask used is 0 or 7
}
*/
static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(t2_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

READ8_MEMBER(adp_state::h63484_rom_r)
{
	UINT8 *rom = machine().region("gfx1")->base();

	return rom[offset];
}

static ADDRESS_MAP_START( adp_h63484_map, AS_0, 8, adp_state )
	AM_RANGE(0x00000, 0x7ffff) AM_DEVREADWRITE("h63484",h63484_device, vram_r,vram_w)
	AM_RANGE(0x80000, 0xbffff) AM_READ(h63484_rom_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fashiong_h63484_map, AS_0, 8, adp_state )
	AM_RANGE(0x00000, 0x7ffff) AM_DEVREADWRITE("h63484",h63484_device, vram_r,vram_w)
//  AM_RANGE(0x40000, 0x7ffff) AM_ROM AM_REGION("gfx1", 0)
ADDRESS_MAP_END

static H63484_INTERFACE( adp_h63484_intf )
{
	"screen",
	acrtc_display_pixels
};

static MACHINE_CONFIG_START( quickjac, adp_state )

	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(quickjac_mem)
//  MCFG_CPU_VBLANK_INT("screen", adp_int)

	MCFG_MACHINE_START(skattv)
	MCFG_MACHINE_RESET(skattv)

	MCFG_DUART68681_ADD( "duart68681", XTAL_8_664MHz / 2, skattv_duart68681_config )
	MCFG_MICROTOUCH_ADD( "microtouch", adb_microtouch_config )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(384, 280)
	MCFG_SCREEN_VISIBLE_AREA(0, 384-1, 0, 280-1)
	MCFG_SCREEN_UPDATE_DRIVER(adp_state, screen_update)

	MCFG_PALETTE_LENGTH(0x10)

	MCFG_PALETTE_INIT(adp)

	MCFG_H63484_ADD("h63484", 0, adp_h63484_intf, adp_h63484_map)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 3686400/2)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( skattv, adp_state )

	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(skattv_mem)
//  MCFG_CPU_VBLANK_INT("screen", adp_int)

	MCFG_MACHINE_START(skattv)
	MCFG_MACHINE_RESET(skattv)

	MCFG_DUART68681_ADD( "duart68681", XTAL_8_664MHz / 2, skattv_duart68681_config )
	MCFG_MICROTOUCH_ADD( "microtouch", adb_microtouch_config )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(384, 280)
	MCFG_SCREEN_VISIBLE_AREA(0, 384-1, 0, 280-1)
	MCFG_SCREEN_UPDATE_DRIVER(adp_state, screen_update)

	MCFG_PALETTE_LENGTH(0x10)

	MCFG_PALETTE_INIT(adp)

	MCFG_H63484_ADD("h63484", 0, adp_h63484_intf, adp_h63484_map)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 3686400/2)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( backgamn, adp_state )

	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(backgamn_mem)

	MCFG_DUART68681_ADD( "duart68681", XTAL_8_664MHz / 2, skattv_duart68681_config )
	MCFG_MICROTOUCH_ADD( "microtouch", adb_microtouch_config )

	MCFG_MACHINE_START(skattv)
	MCFG_MACHINE_RESET(skattv)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(adp_state, screen_update)

	MCFG_PALETTE_LENGTH(0x10)

//  MCFG_PALETTE_INIT(adp)

	MCFG_H63484_ADD("h63484", 0, adp_h63484_intf, adp_h63484_map)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 3686400/2)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( fashiong, skattv )
	MCFG_DEVICE_REMOVE("h63484")
	MCFG_H63484_ADD("h63484", 0, adp_h63484_intf, fashiong_h63484_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( funland, fashiong )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(funland_mem)

	MCFG_PALETTE_LENGTH(0x100)
	MCFG_PALETTE_INIT(all_black)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( fstation, fashiong )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(fstation_mem)
MACHINE_CONFIG_END


ROM_START( quickjac )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "quick_jack_index_a.1.u2.bin", 0x00000, 0x10000, CRC(c2fba6fe) SHA1(f79e5913f9ded1e370cc54dd55860263b9c51d61) )
	ROM_LOAD16_BYTE( "quick_jack_index_a.2.u6.bin", 0x00001, 0x10000, CRC(210cb89b) SHA1(8eac60d40b60e845f9c02fee6c447f125ba5d1ab) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "quick_jack_video_inde_a.1.u2.bin", 0x00000, 0x20000, CRC(73c27fc6) SHA1(12429bc0009b7754e08d2b6a5e1cd8251ab66e2d) )
	ROM_LOAD16_BYTE( "quick_jack_video_inde_a.2.u6.bin", 0x00001, 0x20000, CRC(61d55be2) SHA1(bc17dc91fd1ef0f862eb0d7dbbbfa354a8403eb8) )
ROM_END

ROM_START( skattv )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "f2_i.bin", 0x00000, 0x20000, CRC(3cb8b431) SHA1(e7930876b6cd4cba837c3da05d6948ef9167daea) )
	ROM_LOAD16_BYTE( "f2_ii.bin", 0x00001, 0x20000, CRC(0db1d2d5) SHA1(a29b0299352e0b2b713caf02aa7978f2a4b34e37) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "f1_i.bin", 0x00000, 0x20000, CRC(4869a889) SHA1(ad9f3fcdfd3630f9ad5b93a9d2738de9fc3514d3) )
	ROM_LOAD16_BYTE( "f1_ii.bin", 0x00001, 0x20000, CRC(17681537) SHA1(133685854b2080aaa3d0cced0287bc454d1f3bfc) )
ROM_END

ROM_START( skattva )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "skat_tv_version_ts3.1.u2.bin", 0x00000, 0x20000, CRC(68f82fe8) SHA1(d5f9cb600531cdd748616d8c042b6a151ebe205a) )
	ROM_LOAD16_BYTE( "skat_tv_version_ts3.2.u6.bin", 0x00001, 0x20000, CRC(4f927832) SHA1(bbe013005fd00dd42d12939eab5c80ec44a54b71) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "skat_tv_videoprom_t2.1.u2.bin", 0x00000, 0x20000, CRC(de6f275b) SHA1(0c396fa4d1975c8ccc4967d330b368c0697d2124) )
	ROM_LOAD16_BYTE( "skat_tv_videoprom_t2.2.u5.bin", 0x00001, 0x20000, CRC(af3e60f9) SHA1(c88976ea42cf29a092fdee18377b32ffe91e9f33) )
ROM_END

ROM_START( backgamn )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "b_f2_i.bin", 0x00000, 0x10000, CRC(9e42937c) SHA1(85d462a560b85b03ee9d341e18815b7c396118ac) )
	ROM_LOAD16_BYTE( "b_f2_ii.bin", 0x00001, 0x10000, CRC(8e0ee50c) SHA1(2a05c337db1131b873646aa4109593636ebaa356) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "b_f1_i.bin", 0x00000, 0x20000, NO_DUMP )
	ROM_LOAD16_BYTE( "b_f1_ii.bin", 0x00001, 0x20000, NO_DUMP )
ROM_END

ROM_START( fashiong )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fashion_gambler_s6_i.bin", 0x00000, 0x80000, CRC(827a164d) SHA1(dc16380226cabdefbfd893cb50cbfca9e134be40) )
	ROM_LOAD16_BYTE( "fashion_gambler_s6_ii.bin", 0x00001, 0x80000, CRC(5a2466d1) SHA1(c113a2295beed2011c70887a1f2fcdec00b055cb) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fashion_gambler_video_s2_i.bin", 0x00000, 0x80000, CRC(d1ee9133) SHA1(e5fdfa303a3317f8f5fbdc03438ee97415afff4b) )
	ROM_LOAD16_BYTE( "fashion_gambler_video_s2_ii.bin", 0x00001, 0x80000, CRC(07b1e722) SHA1(594cbe9edfea6b04a4e49d1c1594f1c3afeadef5) )

	ROM_REGION( 0x4000, "user1", 0 )
	//nvram - 16 bit
	ROM_LOAD16_BYTE( "m48z08post.bin", 0x0000, 0x2000, CRC(2d317a04) SHA1(c690c0d4b2259231d642ab5a30fcf389ba987b70) )
	ROM_LOAD16_BYTE( "m48z08posz.bin", 0x0001, 0x2000, CRC(7c5a4b78) SHA1(262d0d7f5b24e356ab54eb2450bbaa90e3fb5464) )
ROM_END

ROM_START( fashiong2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fashion_gambler_f3_i.u2", 0x00000, 0x80000, CRC(2939279a) SHA1(75798ea41dd713d294ea341cbcdb73a76d9f63f4) )
	ROM_LOAD16_BYTE( "fashion_gambler_f3_ii.u6.bin", 0x00001, 0x80000, CRC(7d48e9ab) SHA1(603e946b95c53ee75c9ca10751316e723242424f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fashion_gambler_video_f2_i.u2", 0x00000, 0x80000, CRC(54ea6f10) SHA1(a1284ec34e4e78acba08dc00d5ba47c3457531f8) )
	ROM_LOAD16_BYTE( "fashion_gambler_video_f2_ii.u5", 0x00001, 0x80000, CRC(c292a278) SHA1(9f66531ae9f202d364f47c7ed3551483fc9d27b0) )

	ROM_REGION( 0x4000, "user1", 0 )
	//nvram - 16 bit - taken from parent
	ROM_LOAD16_BYTE( "m48z08post.bin", 0x0000, 0x2000, CRC(2d317a04) SHA1(c690c0d4b2259231d642ab5a30fcf389ba987b70) )
	ROM_LOAD16_BYTE( "m48z08posz.bin", 0x0001, 0x2000, CRC(7c5a4b78) SHA1(262d0d7f5b24e356ab54eb2450bbaa90e3fb5464) )
ROM_END

ROM_START( funlddlx )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fldl_f6_1.bin", 0x00001, 0x80000, CRC(85c74040) SHA1(24a7d3e6acbaf73ef9817379bef64c38a9ff7896) )
	ROM_LOAD16_BYTE( "fldl_f6_2.bin", 0x00000, 0x80000, CRC(93bf1a4b) SHA1(5b4353feba1e0d4402cd26f4855e3803e6be43b9) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "flv_f1_i.bin", 0x00000, 0x80000, CRC(286fccdc) SHA1(dd23deda625e486a7cfe1f3268731d10053a96e9) )
	ROM_LOAD16_BYTE( "flv_f1_ii.bin", 0x00001, 0x80000, CRC(2aa904e6) SHA1(864530b136dd488d619cc95f48e7dce8d93d88e0) )
ROM_END

ROM_START( fstation )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "spielekoffer_9_sp_fun_station_f1.i", 0x00000, 0x80000, CRC(4572efbd) SHA1(e0a91d32ab4096767cafb743523d038f5e0d3238) )
	ROM_LOAD16_BYTE( "spielekoffer_9_sp_fun_station_f1.ii", 0x00001, 0x80000, CRC(a972184d) SHA1(1849e71e696039f07b7b67c4172c7999e81664c3) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "spielekoffer_video_9_sp_f1.i", 0x00000, 0x80000, CRC(b6eb971e) SHA1(14e3272c66a82db0f77123974eea28f308209b1b) )
	ROM_LOAD16_BYTE( "spielekoffer_video_9_sp_f1.ii", 0x00001, 0x80000, CRC(64138dcb) SHA1(1b629915cba32f8f6164ae5075c175b522b4a323) )
ROM_END


GAME( 1990, backgamn,  0,        backgamn,    skattv,    0, ROT0,  "ADP",     "Backgammon", GAME_NOT_WORKING )
GAME( 1993, quickjac,  0,        quickjac,    skattv,    0, ROT0,  "ADP",     "Quick Jack", GAME_NOT_WORKING )
GAME( 1994, skattv,    0,        skattv,      skattv,    0, ROT0,  "ADP",     "Skat TV", GAME_NOT_WORKING )
GAME( 1995, skattva,   skattv,   skattv,      skattv,    0, ROT0,  "ADP",     "Skat TV (version TS3)", GAME_NOT_WORKING )
GAME( 1997, fashiong,  0,        fashiong,    skattv,    0, ROT0,  "ADP",     "Fashion Gambler (set 1)", GAME_NOT_WORKING )
GAME( 1997, fashiong2, fashiong, fashiong,    skattv,    0, ROT0,  "ADP",     "Fashion Gambler (set 2)", GAME_NOT_WORKING )
GAME( 1999, funlddlx,  0,        funland,     skattv,    0, ROT0,  "Stella",  "Funny Land de Luxe", GAME_NOT_WORKING )
GAME( 2000, fstation,  0,        fstation,    skattv,    0, ROT0,  "ADP",     "Fun Station Spielekoffer 9 Spiele", GAME_NOT_WORKING )
