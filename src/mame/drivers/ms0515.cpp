// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Elektronika MS-0515

        06/08/2012 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/t11/t11.h"
#include "machine/wd_fdc.h"
#include "machine/ram.h"
#include "machine/i8255.h"

class ms0515_state : public driver_device
{
public:
	ms0515_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ram(*this, RAM_TAG),
			m_floppy(*this, "vg93:0:525qd")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<floppy_image_device> m_floppy;

	DECLARE_WRITE16_MEMBER(ms0515_bank_w);
	DECLARE_WRITE8_MEMBER(ms0515_sys_w);

	virtual void machine_reset() override;

	UINT8 *m_video_ram;
	UINT8 m_sysreg;
	int m_blink;
	DECLARE_PALETTE_INIT(ms0515);
	UINT32 screen_update_ms0515(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(ms0515_portc_w);
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

	AM_RANGE(0177640, 0177641) AM_DEVREADWRITE8("vg93", fd1793_t, status_r, cmd_w,0x00ff)
	AM_RANGE(0177642, 0177643) AM_DEVREADWRITE8("vg93", fd1793_t, track_r, track_w,0x00ff)
	AM_RANGE(0177644, 0177645) AM_DEVREADWRITE8("vg93", fd1793_t, sector_r, sector_w,0x00ff)
	AM_RANGE(0177646, 0177647) AM_DEVREADWRITE8("vg93", fd1793_t, data_r, data_w,0x00ff)

	//AM_RANGE(0177700, 0177701) // read data
	//AM_RANGE(0177720, 0177721) // write data     // protocol S2
	//AM_RANGE(0177702, 0177703) // read status
	//AM_RANGE(0177722, 0177723) // write control

	//AM_RANGE(0177770, 0177771) // read/write
ADDRESS_MAP_END

WRITE16_MEMBER(ms0515_state::ms0515_bank_w)
{
	UINT8 *ram = m_ram->pointer();
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
	UINT8 *ram = m_ram->pointer();
	ms0515_bank_w(machine().driver_data()->generic_space(),0,0);

	m_video_ram = ram + 0000000 + 0340000;
	m_blink = 0;

	m_floppy->mon_w(0); // turn it on
}

/* Input ports */
static INPUT_PORTS_START( ms0515 )
INPUT_PORTS_END

// disk format: 80 tracks, 1 head, 10 sectors, 512 bytes sector length, first sector id 0

static SLOT_INTERFACE_START( ms0515_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD ) // 720 KB
SLOT_INTERFACE_END

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

PALETTE_INIT_MEMBER(ms0515_state, ms0515)
{
	palette.set_pen_color(0, rgb_t(0, 0, 0));
	palette.set_pen_color(1, rgb_t(0, 0, 127));
	palette.set_pen_color(2, rgb_t(127, 0, 0));
	palette.set_pen_color(3, rgb_t(127, 0, 127));
	palette.set_pen_color(4, rgb_t(0, 127, 0));
	palette.set_pen_color(5, rgb_t(0, 127, 127));
	palette.set_pen_color(6, rgb_t(127, 127, 0));
	palette.set_pen_color(7, rgb_t(127, 127, 127));

	palette.set_pen_color(8, rgb_t(127, 127, 127));
	palette.set_pen_color(9, rgb_t(127, 127, 255));
	palette.set_pen_color(10, rgb_t(255, 127, 127));
	palette.set_pen_color(11, rgb_t(255, 127, 255));
	palette.set_pen_color(12, rgb_t(127, 255, 127));
	palette.set_pen_color(13, rgb_t(127, 255, 255));
	palette.set_pen_color(14, rgb_t(255, 255, 127));
	palette.set_pen_color(15, rgb_t(255, 255, 255));
}

WRITE8_MEMBER(ms0515_state::ms0515_portc_w)
{
	m_sysreg = data;
}

static MACHINE_CONFIG_START( ms0515, ms0515_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",T11, XTAL_4MHz) // Need proper CPU here
	MCFG_T11_INITIAL_MODE(0xf2ff)
	MCFG_CPU_PROGRAM_MAP(ms0515_mem)

	MCFG_DEVICE_ADD("vg93", FD1793, 1000000)
	MCFG_FLOPPY_DRIVE_ADD("vg93:0", ms0515_floppies, "525qd", floppy_image_device::default_floppy_formats)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(ms0515_state, screen_update_ms0515)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(ms0515_state, ms0515)

	MCFG_DEVICE_ADD("ppi8255_1", I8255, 0)
	MCFG_I8255_OUT_PORTC_CB(WRITE8(ms0515_state, ms0515_portc_w))

	//MCFG_DEVICE_ADD("ppi8255_2", I8255, 0)

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
COMP( ????, ms0515,  0,       0,    ms0515,     ms0515,  driver_device, 0,      "Elektronika",   "MS-0515",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
