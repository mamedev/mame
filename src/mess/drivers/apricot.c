/***************************************************************************

    ACT Apricot PC/Xi

    - Needs Intel 8089 support (I/O processor)
    - Dump of the keyboard MCU ROM needed (can be dumped using test mode)

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/z80sio.h"
#include "machine/wd17xx.h"
#include "sound/sn76496.h"
#include "video/mc6845.h"
#include "machine/ram.h"
#include "imagedev/flopdrv.h"
#include "formats/apridisk.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class apricot_state : public driver_device
{
public:
	apricot_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_ram(*this, RAM_TAG),
	m_sn(*this, "ic7"),
	m_crtc(*this, "ic30"),
	m_ppi(*this, "ic17"),
	m_pic(*this, "ic31"),
	m_pit(*this, "ic16"),
	m_z80sio(*this, "ic15"),
	m_fdc(*this, "ic68")
	,
		m_screen_buffer(*this, "screen_buffer"){ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<device_t> m_sn;
	required_device<mc6845_device> m_crtc;
	required_device<i8255_device> m_ppi;
	required_device<device_t> m_pic;
	required_device<device_t> m_pit;
	required_device<device_t> m_z80sio;
	required_device<device_t> m_fdc;
	DECLARE_READ8_MEMBER(apricot_sysctrl_r);
	DECLARE_WRITE8_MEMBER(apricot_sysctrl_w);
	DECLARE_WRITE_LINE_MEMBER(apricot_pit8253_out1);
	DECLARE_WRITE_LINE_MEMBER(apricot_pit8253_out2);
	DECLARE_WRITE_LINE_MEMBER(apricot_wd2793_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(apricot_wd2793_drq_w);
	DECLARE_WRITE_LINE_MEMBER(apricot_mc6845_de);
	bool m_video_mode;
	bool m_display_on;
	bool m_display_enabled;
	required_shared_ptr<UINT16> m_screen_buffer;
	DECLARE_DRIVER_INIT(apricot);
};


/***************************************************************************
    I/O
***************************************************************************/

READ8_MEMBER( apricot_state::apricot_sysctrl_r )
{
	UINT8 data = 0;

	data |= m_display_enabled << 3;

	return data;
}

WRITE8_MEMBER( apricot_state::apricot_sysctrl_w )
{
	m_display_on = BIT(data, 3);
	m_video_mode = BIT(data, 4);
	if (!BIT(data, 5)) wd17xx_set_drive(m_fdc, BIT(data, 6));

	/* switch video modes */
	m_crtc->set_clock( m_video_mode ? XTAL_15MHz / 10 : XTAL_15MHz / 16);
	m_crtc->set_hpixels_per_column( m_video_mode ? 10 : 16);
}

static const i8255_interface apricot_i8255a_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(apricot_state, apricot_sysctrl_w),
	DEVCB_DRIVER_MEMBER(apricot_state, apricot_sysctrl_r),
	DEVCB_NULL
};

WRITE_LINE_MEMBER( apricot_state::apricot_pit8253_out1 )
{
	/* connected to the rs232c interface */
}

WRITE_LINE_MEMBER( apricot_state::apricot_pit8253_out2 )
{
	/* connected to the rs232c interface */
}

static const struct pit8253_config apricot_pit8253_intf =
{
	{
		{ XTAL_4MHz / 16,      DEVCB_LINE_VCC, DEVCB_DEVICE_LINE("ic31", pic8259_ir6_w) },
		{ 0 /*XTAL_4MHz / 2*/, DEVCB_LINE_VCC, DEVCB_DRIVER_LINE_MEMBER(apricot_state, apricot_pit8253_out1) },
		{ 0 /*XTAL_4MHz / 2*/, DEVCB_LINE_VCC, DEVCB_DRIVER_LINE_MEMBER(apricot_state, apricot_pit8253_out2) }
	}
};

static void apricot_sio_irq_w(device_t *device, int st)
{
	apricot_state *state = device->machine().driver_data<apricot_state>();
	pic8259_ir5_w(state->m_pic, st);
}

static const z80sio_interface apricot_z80sio_intf =
{
	apricot_sio_irq_w,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


/***************************************************************************
    INTERRUPTS
***************************************************************************/

static IRQ_CALLBACK( apricot_irq_ack )
{
	apricot_state *state = device->machine().driver_data<apricot_state>();
	return pic8259_acknowledge(state->m_pic);
}

static const struct pic8259_interface apricot_pic8259_intf =
{
	DEVCB_CPU_INPUT_LINE("maincpu", 0),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};


/***************************************************************************
    FLOPPY
***************************************************************************/

WRITE_LINE_MEMBER( apricot_state::apricot_wd2793_intrq_w )
{
	pic8259_ir4_w(m_pic, state);
//  i8089 external terminate channel 1
}

WRITE_LINE_MEMBER( apricot_state::apricot_wd2793_drq_w )
{
//  i8089 data request channel 1
}

static const wd17xx_interface apricot_wd17xx_intf =
{
	DEVCB_LINE_GND,
	DEVCB_DRIVER_LINE_MEMBER(apricot_state, apricot_wd2793_intrq_w),
	DEVCB_DRIVER_LINE_MEMBER(apricot_state, apricot_wd2793_drq_w),
	{ FLOPPY_0, FLOPPY_1, NULL, NULL }
};


/***************************************************************************
    VIDEO EMULATION
***************************************************************************/

static SCREEN_UPDATE_RGB32( apricot )
{
	apricot_state *state = screen.machine().driver_data<apricot_state>();

	if (!state->m_display_on)
		state->m_crtc->screen_update( screen, bitmap, cliprect);
	else
		bitmap.fill(0, cliprect);

	return 0;
}

static MC6845_UPDATE_ROW( apricot_update_row )
{
	apricot_state *state = device->machine().driver_data<apricot_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *ram = state->m_ram->pointer();
	int i, x;

	if (state->m_video_mode)
	{
		/* text mode */
		for (i = 0; i < x_count; i++)
		{
			UINT16 code = state->m_screen_buffer[(ma + i) & 0x7ff];
			UINT16 offset = ((code & 0x7ff) << 5) | (ra << 1);
			UINT16 data = ram[offset + 1] << 8 | ram[offset];
			int fill = 0;

			if (BIT(code, 12) && BIT(data, 14)) fill = 1; /* strike-through? */
			if (BIT(code, 13) && BIT(data, 15)) fill = 1; /* underline? */

			/* draw 10 pixels of the character */
			for (x = 0; x <= 10; x++)
			{
				int color = fill ? 1 : BIT(data, x);
				if (BIT(code, 15)) color = !color; /* reverse? */
				bitmap.pix32(y, x + i*10) = palette[color ? 1 + BIT(code, 14) : 0];
			}
		}
	}
	else
	{
		/* graphics mode */
		fatalerror("Graphics mode not implemented!\n");
	}
}

WRITE_LINE_MEMBER( apricot_state::apricot_mc6845_de )
{
	m_display_enabled = state;
}

static const mc6845_interface apricot_mc6845_intf =
{
	"screen",
	10,
	NULL,
	apricot_update_row,
	NULL,
	DEVCB_DRIVER_LINE_MEMBER(apricot_state, apricot_mc6845_de),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};


/***************************************************************************
    DRIVER INIT
***************************************************************************/

DRIVER_INIT_MEMBER(apricot_state,apricot)
{
	address_space *prg = m_maincpu->space(AS_PROGRAM);

	UINT8 *ram = m_ram->pointer();
	UINT32 ram_size = m_ram->size();

	prg->unmap_readwrite(0x40000, 0xeffff);
	prg->install_ram(0x00000, ram_size - 1, ram);

	m_maincpu->set_irq_acknowledge_callback(apricot_irq_ack);

	m_video_mode = 0;
	m_display_on = 1;
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( apricot_mem, AS_PROGRAM, 16, apricot_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAMBANK("standard_ram")
	AM_RANGE(0x40000, 0xeffff) AM_RAMBANK("expansion_ram")
	AM_RANGE(0xf0000, 0xf0fff) AM_MIRROR(0x7000) AM_RAM AM_SHARE("screen_buffer")
	AM_RANGE(0xfc000, 0xfffff) AM_MIRROR(0x4000) AM_ROM AM_REGION("bootstrap", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( apricot_io, AS_IO, 16, apricot_state )
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE8_LEGACY("ic31", pic8259_r, pic8259_w, 0x00ff)
	AM_RANGE(0x40, 0x47) AM_DEVREADWRITE8_LEGACY("ic68", wd17xx_r, wd17xx_w, 0x00ff)
	AM_RANGE(0x48, 0x4f) AM_DEVREADWRITE8("ic17", i8255_device, read, write, 0x00ff)
	AM_RANGE(0x50, 0x51) AM_MIRROR(0x06) AM_DEVWRITE8("ic7", sn76489_new_device, write, 0x00ff)
	AM_RANGE(0x58, 0x5f) AM_DEVREADWRITE8_LEGACY("ic16", pit8253_r, pit8253_w, 0x00ff)
	AM_RANGE(0x60, 0x67) AM_DEVREADWRITE8("ic15", z80sio_device, read_alt, write_alt, 0x00ff)
	AM_RANGE(0x68, 0x69) AM_MIRROR(0x04) AM_DEVWRITE8("ic30", mc6845_device, address_w, 0x00ff)
	AM_RANGE(0x6a, 0x6b) AM_MIRROR(0x04) AM_DEVREADWRITE8("ic30", mc6845_device, register_r, register_w, 0x00ff)
//  AM_RANGE(0x70, 0x71) AM_MIRROR(0x04) 8089 channel attention 1
//  AM_RANGE(0x72, 0x73) AM_MIRROR(0x04) 8089 channel attention 2
	AM_RANGE(0x78, 0x7f) AM_NOP /* unavailable */
ADDRESS_MAP_END


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( apricot )
INPUT_PORTS_END


/***************************************************************************
    PALETTE
***************************************************************************/

static PALETTE_INIT( apricot )
{
	palette_set_color(machine, 0, MAKE_RGB(0x00, 0x00, 0x00)); /* black */
	palette_set_color(machine, 1, MAKE_RGB(0x00, 0x7f, 0x00)); /* low intensity */
	palette_set_color(machine, 2, MAKE_RGB(0x00, 0xff, 0x00)); /* high intensitiy */
}


/***************************************************************************
 	SOUND INTERFACE
 **************************************************************************/
 
 
//-------------------------------------------------
//  sn76496_config psg_intf
//-------------------------------------------------

static const sn76496_config psg_intf =
{
    DEVCB_NULL
};


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static LEGACY_FLOPPY_OPTIONS_START( apricot )
	LEGACY_FLOPPY_OPTION
	(
		apridisk, "dsk", "ACT Apricot disk image", apridisk_identify, apridisk_construct, NULL,
		HEADS(1-[2])
		TRACKS(70/[80])
		SECTORS([9]/18)
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1])
	)
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface apricot_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_3_5_DSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(apricot),
	"floppy_3_5",
	NULL
};

static MACHINE_CONFIG_START( apricot, apricot_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, XTAL_15MHz / 3)
	MCFG_CPU_PROGRAM_MAP(apricot_mem)
	MCFG_CPU_IO_MAP(apricot_io)

//  MCFG_CPU_ADD("ic71", I8089, XTAL_15MHz / 3)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(800, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 800-1, 0, 400-1)
	MCFG_SCREEN_REFRESH_RATE(72)
	MCFG_SCREEN_UPDATE_STATIC(apricot)
	MCFG_PALETTE_LENGTH(3)
	MCFG_PALETTE_INIT(apricot)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ic7", SN76489_NEW, XTAL_4MHz / 2)
	MCFG_SOUND_CONFIG(psg_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256k")
	MCFG_RAM_EXTRA_OPTIONS("384k,512k") /* with 1 or 2 128k expansion boards */

	/* Devices */
	MCFG_MC6845_ADD("ic30", MC6845, XTAL_15MHz / 10, apricot_mc6845_intf)
	MCFG_I8255A_ADD("ic17", apricot_i8255a_intf)
	MCFG_PIC8259_ADD("ic31", apricot_pic8259_intf)
	MCFG_PIT8253_ADD("ic16", apricot_pit8253_intf)
	MCFG_Z80SIO_ADD("ic15", 0, apricot_z80sio_intf)

	/* floppy */
	MCFG_WD2793_ADD("ic68", apricot_wd17xx_intf)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(apricot_floppy_interface)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apricotxi, apricot )
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( apricot )
	ROM_REGION(0x4000, "bootstrap", 0)
	ROM_LOAD16_BYTE("pc_bios_lo_001.bin", 0x0000, 0x2000, CRC(0c217cc2) SHA1(0d7a2b61e17966462b555115f962a175fadcf72a))
	ROM_LOAD16_BYTE("pc_bios_hi_001.bin", 0x0001, 0x2000, CRC(7c27f36c) SHA1(c801bbf904815f76ec6463e948f57e0118a26292))
ROM_END

ROM_START( apricotxi )
	ROM_REGION(0x4000, "bootstrap", 0)
	ROM_LOAD16_BYTE("lo_ve007.u11", 0x0000, 0x2000, CRC(e74e14d1) SHA1(569133b0266ce3563b21ae36fa5727308797deee)) /* LO Ve007 03.04.84 */
	ROM_LOAD16_BYTE("hi_ve007.u9",  0x0001, 0x2000, CRC(b04fb83e) SHA1(cc2b2392f1b4c04bb6ec8ee26f8122242c02e572)) /* HI Ve007 03.04.84 */
ROM_END


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

/*    YEAR  NAME       PARENT   COMPAT  MACHINE    INPUT    INIT     COMPANY  FULLNAME      FLAGS */
COMP( 1983, apricot,   0,       0,      apricot,   apricot, apricot_state, apricot, "ACT",   "Apricot PC", GAME_NOT_WORKING )
COMP( 1984, apricotxi, apricot, 0,      apricotxi, apricot, apricot_state, apricot, "ACT",   "Apricot Xi", GAME_NOT_WORKING )
