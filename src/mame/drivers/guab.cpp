// license: BSD-3-Clause
// copyright-holders: Philip Bennett, Dirk Best
/***************************************************************************

    JPM Give us a Break hardware

    preliminary driver by Phil Bennett

    Games supported:
        * Give us a Break [8 sets]
        * Criss Cross (Sweden) [non-working - need disk image]
        * Ten Up [2 sets]

    Looking for:
        * Numbers Game
        * Pac Quiz
        * Suit Pursuit
        * Treasure Trail?

    Known issues:
        * Neither game registers coins and I can't find where the credit
        count gets updated in the code. Each game requires a unique
        security PAL - maybe this is related? I'm poking the coin values
        directly into RAM for now.
        * Game hangs when you try to 'collect' cash
        * Verify WD FDC type
        * Are IRQs 1 or 2 connected to something?
        * Hook up ACIA properly (IRQ 4)
        * Hook up watchdog NMI
        * Verify clocks
        * Use real video timings
        * Create layouts

    Notes:
        * Toggle both 'Back door' and 'Key switch' to enter test mode
        * Video hardware seems to match JPM System 5

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "formats/guab_dsk.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/i8255.h"
#include "machine/wd_fdc.h"
#include "sound/sn76496.h"
#include "video/ef9369.h"
#include "video/tms34061.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "guab.lh"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class guab_state : public driver_device
{
public:
	guab_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms34061(*this, "tms34061"),
		m_sn(*this, "snsnd"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:0"),
		m_palette(*this, "palette"),
		m_sound_buffer(0), m_sound_latch(false)
	{ }

	EF9369_COLOR_UPDATE(ef9369_color_update);
	DECLARE_WRITE16_MEMBER(tms34061_w);
	DECLARE_READ16_MEMBER(tms34061_r);
	uint32_t screen_update_guab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(output1_w);
	DECLARE_WRITE8_MEMBER(output2_w);
	DECLARE_WRITE8_MEMBER(output3_w);
	DECLARE_WRITE8_MEMBER(output4_w);
	DECLARE_WRITE8_MEMBER(output5_w);
	DECLARE_WRITE8_MEMBER(output6_w);
	DECLARE_READ8_MEMBER(sn76489_ready_r);
	DECLARE_WRITE8_MEMBER(sn76489_buffer_w);
	DECLARE_WRITE8_MEMBER(system_w);
	DECLARE_READ8_MEMBER(watchdog_r);
	DECLARE_WRITE8_MEMBER(watchdog_w);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	void guab(machine_config &config);
	void guab_map(address_map &map);
protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<tms34061_device> m_tms34061;
	required_device<sn76489_device> m_sn;
	required_device<wd1773_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<palette_device> m_palette;

	uint8_t m_sound_buffer;
	bool m_sound_latch;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

ADDRESS_MAP_START(guab_state::guab_map)
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x040000, 0x04ffff) AM_ROM AM_REGION("maincpu", 0x10000)
	AM_RANGE(0x0c0000, 0x0c0007) AM_DEVREADWRITE8("i8255_1", i8255_device, read, write, 0x00ff)
	AM_RANGE(0x0c0020, 0x0c0027) AM_DEVREADWRITE8("i8255_2", i8255_device, read, write, 0x00ff)
	AM_RANGE(0x0c0040, 0x0c0047) AM_DEVREADWRITE8("i8255_3", i8255_device, read, write, 0x00ff)
	AM_RANGE(0x0c0060, 0x0c0067) AM_DEVREADWRITE8("i8255_4", i8255_device, read, write, 0x00ff)
	AM_RANGE(0x0c0080, 0x0c0083) AM_DEVREADWRITE8("acia6850_1", acia6850_device, read, write, 0x00ff)
	AM_RANGE(0x0c00a0, 0x0c00a3) AM_DEVREADWRITE8("acia6850_2", acia6850_device, read, write, 0x00ff)
	AM_RANGE(0x0c00c0, 0x0c00cf) AM_DEVREADWRITE8("6840ptm", ptm6840_device, read, write, 0x00ff)
	AM_RANGE(0x0c00e0, 0x0c00e7) AM_DEVREADWRITE8("fdc", wd1773_device, read, write, 0x00ff)
	AM_RANGE(0x080000, 0x080fff) AM_RAM
	AM_RANGE(0x100000, 0x100001) AM_DEVREADWRITE8("ef9369", ef9369_device, data_r, data_w, 0x00ff)
	AM_RANGE(0x100002, 0x100003) AM_DEVWRITE8("ef9369", ef9369_device, address_w, 0x00ff)
	AM_RANGE(0x800000, 0xb0ffff) AM_READWRITE(tms34061_r, tms34061_w)
	AM_RANGE(0xb10000, 0xb1ffff) AM_RAM
	AM_RANGE(0xb80000, 0xb8ffff) AM_RAM
	AM_RANGE(0xb90000, 0xb9ffff) AM_RAM
ADDRESS_MAP_END


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( guab )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME("50p") PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)50)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )   PORT_NAME("100p") PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)100)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Back door") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Cash door") PORT_CODE(KEYCODE_T) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key switch") PORT_CODE(KEYCODE_Y) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_UNUSED )  PORT_NAME("50p level")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_UNUSED )  PORT_NAME("100p level")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Select")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("D")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("10p") PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)10)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("20p") PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)20)
INPUT_PORTS_END

static INPUT_PORTS_START( tenup )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME("50p") PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)50)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )   PORT_NAME("100p") PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)100)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Back door") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Cash door") PORT_CODE(KEYCODE_T) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key switch") PORT_CODE(KEYCODE_Y) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_UNUSED )  PORT_NAME("10p level")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_UNUSED )  PORT_NAME("100p level")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Pass")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("10p")  PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)10)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("20p")  PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)20)
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

EF9369_COLOR_UPDATE( guab_state::ef9369_color_update )
{
	m_palette->set_pen_color(entry, pal4bit(ca), pal4bit(cb), pal4bit(cc));
}

WRITE16_MEMBER( guab_state::tms34061_w )
{
	int func = (offset >> 19) & 3;
	int row = (offset >> 7) & 0xff;
	int col;

	if (func == 0 || func == 2)
		col = offset  & 0xff;
	else
		col = offset <<= 1;

	if (ACCESSING_BITS_8_15)
		m_tms34061->write(space, col, row, func, data >> 8);

	if (ACCESSING_BITS_0_7)
		m_tms34061->write(space, col | 1, row, func, data & 0xff);
}

READ16_MEMBER( guab_state::tms34061_r )
{
	uint16_t data = 0;
	int func = (offset >> 19) & 3;
	int row = (offset >> 7) & 0xff;
	int col;

	if (func == 0 || func == 2)
		col = offset  & 0xff;
	else
		col = offset <<= 1;

	if (ACCESSING_BITS_8_15)
		data |= m_tms34061->read(space, col, row, func) << 8;

	if (ACCESSING_BITS_0_7)
		data |= m_tms34061->read(space, col | 1, row, func);

	return data;
}

uint32_t guab_state::screen_update_guab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tms34061->get_display_state();

	/* If blanked, fill with black */
	if (m_tms34061->m_display.blanked)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		uint8_t *src = &m_tms34061->m_display.vram[256 * y];
		uint16_t *dest = &bitmap.pix16(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			uint8_t pen = src[x >> 1];

			/* Draw two 4-bit pixels */
			*dest++ = m_palette->pen(pen >> 4);
			*dest++ = m_palette->pen(pen & 0x0f);
		}
	}

	return 0;
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void guab_state::machine_start()
{
	m_fdc->set_floppy(m_floppy->get_device());
}

READ8_MEMBER( guab_state::watchdog_r )
{
	// only read after writing the sequence below
	return 0xff;
}

WRITE8_MEMBER( guab_state::watchdog_w )
{
	// watchdog?
	// writes b 3 1 5 d a 2 0 4 0 8   b 3 1 5 d a 2 0 4 0 8
	// then later toggles between 0 and f
}

WRITE8_MEMBER( guab_state::system_w )
{
	// bit 0, sound latch
	if (m_sound_latch != bool(BIT(data, 0)))
	{
		// falling edge
		if (!m_sound_latch)
			m_sn->write(m_sound_buffer);

		m_sound_latch = bool(BIT(data, 0));
	}

	// bit 3, floppy drive side select
	m_floppy->get_device()->ss_w(BIT(data, 3));

	// one of those bits will probably control the motor, we just let it run all the time for now
	m_floppy->get_device()->mon_w(0);
}


//**************************************************************************
//  INPUTS/OUTPUTS
//**************************************************************************

INPUT_CHANGED_MEMBER( guab_state::coin_inserted )
{
	if (newval == 0)
	{
		uint32_t credit;
		address_space &space = m_maincpu->space(AS_PROGRAM);

		/* Get the current credit value and add the new coin value */
		credit = space.read_dword(0x8002c) + (uint32_t)(uintptr_t)param;
		space.write_dword(0x8002c, credit);
	}
}

WRITE8_MEMBER( guab_state::output1_w )
{
	output().set_value("led_0", BIT(data, 0)); // cash in (ten up: cash in)
	output().set_value("led_1", BIT(data, 1)); // cash out (ten up: cash out)
	output().set_value("led_2", BIT(data, 2));
	output().set_value("led_3", BIT(data, 3));
	output().set_value("led_4", BIT(data, 4));
	output().set_value("led_5", BIT(data, 5));
	output().set_value("led_6", BIT(data, 6)); // (ten up: 10p/100p drive)
	output().set_value("led_7", BIT(data, 7));
}

WRITE8_MEMBER( guab_state::output2_w )
{
	output().set_value("led_8", BIT(data, 0));
	output().set_value("led_9", BIT(data, 1));
	output().set_value("led_10", BIT(data, 2)); // start (ten up: start)
	output().set_value("led_11", BIT(data, 3)); // (ten up: feature 6)
	output().set_value("led_12", BIT(data, 4)); // (ten up: feature 11)
	output().set_value("led_13", BIT(data, 5)); // (ten up: feature 13)
	output().set_value("led_14", BIT(data, 6)); // lamp a (ten up: feature 12)
	output().set_value("led_15", BIT(data, 7)); // lamp b (ten up: pass)
}

WRITE8_MEMBER( guab_state::output3_w )
{
	output().set_value("led_16", BIT(data, 0)); // select (ten up: collect)
	output().set_value("led_17", BIT(data, 1)); // (ten up: feature 14)
	output().set_value("led_18", BIT(data, 2)); // (ten up: feature 9)
	output().set_value("led_19", BIT(data, 3)); //   (ten up: lamp a)
	output().set_value("led_20", BIT(data, 4)); // lamp c (ten up: lamp b)
	output().set_value("led_21", BIT(data, 5)); // lamp d (ten up: lamp c)
	output().set_value("led_22", BIT(data, 6));
	output().set_value("led_23", BIT(data, 7));
}

WRITE8_MEMBER( guab_state::output4_w )
{
	output().set_value("led_24", BIT(data, 0)); // feature 1 (ten up: feature 1)
	output().set_value("led_25", BIT(data, 1)); // feature 2 (ten up: feature 10)
	output().set_value("led_26", BIT(data, 2)); // feature 3 (ten up: feature 7)
	output().set_value("led_27", BIT(data, 3)); // feature 4 (ten up: feature 2)
	output().set_value("led_28", BIT(data, 4)); // feature 5 (ten up: feature 8)
	output().set_value("led_29", BIT(data, 5)); // feature 6 (ten up: feature 3)
	output().set_value("led_30", BIT(data, 6)); // feature 7 (ten up: feature 4)
	output().set_value("led_31", BIT(data, 7)); // feature 8 (ten up: feature 5)
}

WRITE8_MEMBER( guab_state::output5_w )
{
	output().set_value("led_32", BIT(data, 0));
	output().set_value("led_33", BIT(data, 1));
	output().set_value("led_34", BIT(data, 2));
	output().set_value("led_35", BIT(data, 3));
	output().set_value("led_36", BIT(data, 4));
	output().set_value("led_37", BIT(data, 5));
	output().set_value("led_38", BIT(data, 6));
	output().set_value("led_39", BIT(data, 7)); // mech lamp (ten up: mech lamp)
}

WRITE8_MEMBER( guab_state::output6_w )
{
	output().set_value("led_40", BIT(data, 0));
	output().set_value("led_41", BIT(data, 1));
	output().set_value("led_42", BIT(data, 2));
	output().set_value("led_43", BIT(data, 3));
	output().set_value("led_44", BIT(data, 4)); // 50p drive (ten up: 10p drive)
	output().set_value("led_45", BIT(data, 5)); // 100p drive (ten up: 100p drive)
	output().set_value("led_46", BIT(data, 6));
	output().set_value("led_47", BIT(data, 7));
}


//**************************************************************************
//  AUDIO
//**************************************************************************

READ8_MEMBER( guab_state::sn76489_ready_r )
{
	// bit 7 connected to sn76489 ready output (0 = ready)
	return ~(m_sn->ready_r() << 7);
}

WRITE8_MEMBER( guab_state::sn76489_buffer_w )
{
	m_sound_buffer = data;
}


//**************************************************************************
//  FLOPPY DRIVE
//**************************************************************************

FLOPPY_FORMATS_MEMBER( guab_state::floppy_formats )
	FLOPPY_GUAB_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( guab_floppies )
	SLOT_INTERFACE("dd", FLOPPY_35_DD)
SLOT_INTERFACE_END


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

MACHINE_CONFIG_START(guab_state::guab)
	/* TODO: Verify clock */
	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(guab_map)

	/* TODO: Use real video timings */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(guab_state, screen_update_guab)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", ef9369_device::NUMCOLORS)

	MCFG_EF9369_ADD("ef9369")
	MCFG_EF9369_COLOR_UPDATE_CB(guab_state, ef9369_color_update)

	MCFG_DEVICE_ADD("tms34061", TMS34061, 0)
	MCFG_TMS34061_ROWSHIFT(8)  /* VRAM address is (row << rowshift) | col */
	MCFG_TMS34061_VRAM_SIZE(0x40000)
	MCFG_TMS34061_INTERRUPT_CB(INPUTLINE("maincpu", 5))

	MCFG_SPEAKER_STANDARD_MONO("mono")

	/* TODO: Verify clock */
	MCFG_SOUND_ADD("snsnd", SN76489, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_ADD("6840ptm", PTM6840, 1000000)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_IRQ_CB(INPUTLINE("maincpu", 3))

	MCFG_DEVICE_ADD("i8255_1", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	MCFG_DEVICE_ADD("i8255_2", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(guab_state, output1_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(guab_state, output2_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(guab_state, output3_w))

	MCFG_DEVICE_ADD("i8255_3", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(guab_state, output4_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(guab_state, output5_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(guab_state, output6_w))

	MCFG_DEVICE_ADD("i8255_4", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(guab_state, sn76489_ready_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(guab_state, sn76489_buffer_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(guab_state, system_w))
	MCFG_I8255_IN_PORTC_CB(READ8(guab_state, watchdog_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(guab_state, watchdog_w))

	MCFG_DEVICE_ADD("acia6850_1", ACIA6850, 0)

	MCFG_DEVICE_ADD("acia6850_2", ACIA6850, 0)

	// floppy
	MCFG_WD1773_ADD("fdc", 8000000)
	MCFG_WD_FDC_DRQ_CALLBACK(INPUTLINE("maincpu", 6))

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", guab_floppies, "dd", guab_state::floppy_formats)
	MCFG_SLOT_FIXED(true)

	MCFG_SOFTWARE_LIST_ADD("floppy_list", "guab")

	MCFG_DEFAULT_LAYOUT(layout_guab)
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( guab )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "guab1a1.rom", 0x00000, 0x8000, CRC(f23a9d7d) SHA1(f933e131bdcf21cfa6001c8e20fd11d94c7a9450) )
	ROM_LOAD16_BYTE( "guab1b1.rom", 0x00001, 0x8000, CRC(af3b5492) SHA1(6fd7f29e6ed2fadccc9246f1ebd049c3f9aeff13) )
	ROM_LOAD16_BYTE( "guab2a1.rom", 0x10000, 0x8000, CRC(ae7a162c) SHA1(d69721818b8e4daba776a678b62bc7f44f371a3f) )
	ROM_LOAD16_BYTE( "guab2b1.rom", 0x10001, 0x8000, CRC(29aa26a0) SHA1(8d425ad845ccfcd8995dbf6adc1ca17989a5d3ea) )
ROM_END

ROM_START( crisscrs )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "crisscross_swe_2a1.ic49", 0x00000, 0x8000, CRC(a7ca8828) SHA1(a28482bb2bc1248a9b5c0b57904c382246a632cc) )
	ROM_LOAD16_BYTE( "crisscross_swe_2b1.ic48", 0x00001, 0x8000, CRC(7e280cae) SHA1(18c76459e39549ddba5f0cd7921013ef4f816826) )
ROM_END

ROM_START( tenup )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tu-11.bin", 0x00000, 0x8000, CRC(01843086) SHA1(106a226900e8cf929f89edf801c627f02e4afce3) )
	ROM_LOAD16_BYTE( "tu-12.bin", 0x00001, 0x8000, CRC(1c7f32b1) SHA1(2b14e2206695ae53909ae838a5c036248d9ab940) )
	ROM_LOAD16_BYTE( "tu-13.bin", 0x10000, 0x8000, CRC(d19e2bf7) SHA1(76a9cbd4f604ad39eb0e319a9a6d5a6739b0ed8c) )
	ROM_LOAD16_BYTE( "tu-14.bin", 0x10001, 0x8000, CRC(fd8a0c3c) SHA1(f87289ce6f0d2bc9b7d3a0b6deff38ba3aadf391) )
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  MACHINE  INPUT  CLASS       INIT  ROTATION  COMPANY  FULLNAME                FLAGS
GAME( 1986, guab,     0,      guab,    guab,  guab_state, 0,    ROT0,     "JPM",   "Give us a Break",      0 )
GAME( 1986, crisscrs, 0,      guab,    guab,  guab_state, 0,    ROT0,     "JPM",   "Criss Cross (Sweden)", MACHINE_NOT_WORKING )
GAME( 1988, tenup,    0,      guab,    tenup, guab_state, 0,    ROT0,     "JPM",   "Ten Up",               0 )
