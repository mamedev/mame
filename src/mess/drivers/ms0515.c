/***************************************************************************

        Elektronika MS-0515

        06/08/2012 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/t11/t11.h"
#include "machine/wd17xx.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"
#include "machine/ram.h"
#include "machine/i8255.h"

class ms0515_state : public driver_device
{
public:
	ms0515_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

	DECLARE_WRITE16_MEMBER(ms0515_bank_w);
	DECLARE_WRITE8_MEMBER(ms0515_sys_w);

	virtual void machine_reset();

	UINT8 *m_video_ram;
	UINT8 m_sysreg;
	int m_blink;
	virtual void palette_init();
	UINT32 screen_update_ms0515(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

static ADDRESS_MAP_START(ms0515_mem, AS_PROGRAM, 16, ms0515_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0000000, 0017777) AM_RAMBANK("bank0") // RAM
	AM_RANGE(0020000, 0037777) AM_RAMBANK("bank1") // RAM
	AM_RANGE(0040000, 0057777) AM_RAMBANK("bank2") // RAM
	AM_RANGE(0060000, 0077777) AM_RAMBANK("bank3") // RAM
	AM_RANGE(0100000, 0117777) AM_RAMBANK("bank4") // RAM
	AM_RANGE(0120000, 0137777) AM_RAMBANK("bank5") // RAM
	AM_RANGE(0140000, 0157777) AM_RAMBANK("bank6") // RAM

	AM_RANGE(0160000, 0177377) AM_ROM

	AM_RANGE(0177400, 0177437) AM_WRITE(ms0515_bank_w) // Register for RAM expansion

	//AM_RANGE(0177440, 0177441)  // read data
	//AM_RANGE(0177460, 0177461)  // write data   Elektronika MS-7004 Keyboard
	//AM_RANGE(0177442, 0177443)  // read
	//AM_RANGE(0177462, 0177463)  // write

	//AM_RANGE(0177500, 0177501)
	//AM_RANGE(0177502, 0177503)
	//AM_RANGE(0177504, 0177505)  // i8253
	//AM_RANGE(0177506, 0177507)

	//AM_RANGE(0177540, 0177541)
	//AM_RANGE(0177542, 0177543)
	//AM_RANGE(0177544, 0177545)  // i8255 for MS-7007 Keyboard
	//AM_RANGE(0177546, 0177547)

	AM_RANGE(0177600, 0177607) AM_DEVREADWRITE8("ppi8255_1", i8255_device, read, write, 0x00ff)

	AM_RANGE(0177640, 0177641) AM_DEVREADWRITE8_LEGACY("vg93", wd17xx_status_r, wd17xx_command_w,0x00ff)
	AM_RANGE(0177642, 0177643) AM_DEVREADWRITE8_LEGACY("vg93", wd17xx_track_r, wd17xx_track_w,0x00ff)
	AM_RANGE(0177644, 0177645) AM_DEVREADWRITE8_LEGACY("vg93", wd17xx_sector_r, wd17xx_sector_w,0x00ff)
	AM_RANGE(0177646, 0177647) AM_DEVREADWRITE8_LEGACY("vg93", wd17xx_data_r, wd17xx_data_w,0x00ff)

	//AM_RANGE(0177700, 0177701) // read data
	//AM_RANGE(0177720, 0177721) // write data     // protocol S2
	//AM_RANGE(0177702, 0177703) // read status
	//AM_RANGE(0177722, 0177723) // write control

	//AM_RANGE(0177770, 0177771) // read/write
ADDRESS_MAP_END

WRITE16_MEMBER(ms0515_state::ms0515_bank_w)
{
	UINT8 *ram = machine().device<ram_device>(RAM_TAG)->pointer();
	membank("bank0")->set_base(ram + 0000000 + BIT(data,0) * 0160000);
	membank("bank1")->set_base(ram + 0020000 + BIT(data,1) * 0160000);
	membank("bank2")->set_base(ram + 0040000 + BIT(data,2) * 0160000);
	membank("bank3")->set_base(ram + 0060000 + BIT(data,3) * 0160000);
	membank("bank4")->set_base(ram + 0100000 + BIT(data,4) * 0160000);
	membank("bank5")->set_base(ram + 0120000 + BIT(data,5) * 0160000);
	membank("bank6")->set_base(ram + 0140000 + BIT(data,6) * 0160000);
	if (BIT(data,7)) {
		switch((data >> 10) % 3)
		{
			case 0: // 000000 - 037777
					membank("bank0")->set_base(ram + 0000000 + 0340000);
					membank("bank1")->set_base(ram + 0020000 + 0340000);
					break;
			case 1: // 040000 - 077777
					membank("bank2")->set_base(ram + 0000000 + 0340000);
					membank("bank3")->set_base(ram + 0020000 + 0340000);
					break;
			case 2:
			case 3: // 100000 - 137777
					membank("bank4")->set_base(ram + 0000000 + 0340000);
					membank("bank5")->set_base(ram + 0020000 + 0340000);
					break;
		}
	}
}

WRITE8_MEMBER(ms0515_state::ms0515_sys_w)
{
	m_sysreg = data;
}

void ms0515_state::machine_reset()
{
	UINT8 *ram = machine().device<ram_device>(RAM_TAG)->pointer();
	ms0515_bank_w(machine().driver_data()->generic_space(),0,0);

	m_video_ram = ram + 0000000 + 0340000;
	m_blink = 0;

	floppy_mon_w(machine().device(FLOPPY_0), 0); // turn it on
}

/* Input ports */
static INPUT_PORTS_START( ms0515 )
INPUT_PORTS_END

static const struct t11_setup ms0515_data =
{
	0xf2ff
};

static const wd17xx_interface ms0515_wd17xx_interface =
{
	DEVCB_LINE_VCC,
	DEVCB_NULL,
	DEVCB_NULL,
	{ FLOPPY_0, NULL, NULL, NULL }
};


static LEGACY_FLOPPY_OPTIONS_START(ms0515)
	LEGACY_FLOPPY_OPTION(ms0515, "dsk", "MS0515 disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([80])
		SECTORS([10])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([0]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface ms0515_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(ms0515),
	NULL,
	NULL
};

UINT32 ms0515_state::screen_update_ms0515(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y, x, b;
	int addr = 0;
	if (BIT(m_sysreg,3))  {
		for (y = 0; y < 200; y++)
		{
			int horpos = 0;
			for (x = 0; x < 40; x++)
			{
				UINT16 code = (m_video_ram[addr++] << 8);
				code += m_video_ram[addr++];
				for (b = 0; b < 16; b++)
				{
					// In lower res mode we will just double pixels
					bitmap.pix16(y, horpos++) =  ((code >> (15-b)) & 0x01) ? 0 : 7;
				}
			}
		}
	} else {
		for (y = 0; y < 200; y++)
		{
			int horpos = 0;
			for (x = 0; x < 40; x++)
			{
				UINT8 code = m_video_ram[addr++];
				UINT8 attr = m_video_ram[addr++];
				UINT8 fg = (attr & 7) + BIT(attr,6)*8;
				UINT8 bg = ((attr >> 3) & 7) + BIT(attr,6)*8;
				if (BIT(attr,7) && (m_blink == 20)) {
					UINT8 tmp = fg;
					fg = bg; bg = tmp;
					m_blink = -1;
				}
				for (b = 0; b < 8; b++)
				{
					// In lower res mode we will just double pixels
					bitmap.pix16(y, horpos++) =  ((code >> (7-b)) & 0x01) ? fg : bg;
					bitmap.pix16(y, horpos++) =  ((code >> (7-b)) & 0x01) ? fg : bg;
				}
			}
		}
	}
	m_blink++;
	return 0;
}

void ms0515_state::palette_init()
{
	palette_set_color(machine(), 0, MAKE_RGB(0, 0, 0));
	palette_set_color(machine(), 1, MAKE_RGB(0, 0, 127));
	palette_set_color(machine(), 2, MAKE_RGB(127, 0, 0));
	palette_set_color(machine(), 3, MAKE_RGB(127, 0, 127));
	palette_set_color(machine(), 4, MAKE_RGB(0, 127, 0));
	palette_set_color(machine(), 5, MAKE_RGB(0, 127, 127));
	palette_set_color(machine(), 6, MAKE_RGB(127, 127, 0));
	palette_set_color(machine(), 7, MAKE_RGB(127, 127, 127));

	palette_set_color(machine(), 8, MAKE_RGB(127, 127, 127));
	palette_set_color(machine(), 9, MAKE_RGB(127, 127, 255));
	palette_set_color(machine(), 10, MAKE_RGB(255, 127, 127));
	palette_set_color(machine(), 11, MAKE_RGB(255, 127, 255));
	palette_set_color(machine(), 12, MAKE_RGB(127, 255, 127));
	palette_set_color(machine(), 13, MAKE_RGB(127, 255, 255));
	palette_set_color(machine(), 14, MAKE_RGB(255, 255, 127));
	palette_set_color(machine(), 15, MAKE_RGB(255, 255, 255));
}

static WRITE8_DEVICE_HANDLER(ms0515_portc_w)
{
	ms0515_state *state = device->machine().driver_data<ms0515_state>();
	state->m_sysreg = data;
}
I8255A_INTERFACE( ms0515_ppi8255_interface_1 )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(ms0515_portc_w)
};

static MACHINE_CONFIG_START( ms0515, ms0515_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",T11, XTAL_4MHz) // Need proper CPU here
	MCFG_CPU_CONFIG(ms0515_data)
	MCFG_CPU_PROGRAM_MAP(ms0515_mem)

	MCFG_FD1793_ADD( "vg93", ms0515_wd17xx_interface )

	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, ms0515_floppy_interface)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(ms0515_state, screen_update_ms0515)

	MCFG_PALETTE_LENGTH(16)

	MCFG_I8255_ADD( "ppi8255_1", ms0515_ppi8255_interface_1 )
	//MCFG_I8255_ADD( "ppi8255_2", ms0515_ppi8255_interface_2 )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ms0515 )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "7004l.bin", 0xc000, 0x2000, CRC(b08b3b73) SHA1(c12fd4672598cdf499656dcbb4118d787769d589))
	ROM_LOAD16_BYTE( "7004h.bin", 0xc001, 0x2000, CRC(515dcf99) SHA1(edd34300fd642c89ce321321e1b12493cd16b7a5))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT               COMPANY   FULLNAME       FLAGS */
COMP( ????, ms0515,  0,       0,	ms0515, 	ms0515,  driver_device, 0,  	"Elektronika",   "MS-0515",		GAME_NOT_WORKING | GAME_NO_SOUND)

