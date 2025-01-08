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
        * Coin data has to go through the datalogger to be counted
        * Verify WD FDC type
        * Hook up ACIA properly, clocks etc.
        * Hook up watchdog NMI
        * Verify clocks
        * Use real video timings
        * Create layouts

    Notes:
        * Toggle both 'Back door' and 'Key switch' to enter test mode
        * Opening the back door will disable coinup
        * Video hardware seems to match JPM System 5
        * IRQ 1 inits the PPIs, IRQ 2 does nothing

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "formats/guab_dsk.h"
#include "imagedev/floppy.h"
#include "machine/bacta_datalogger.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "machine/wd_fdc.h"
#include "sound/sn76496.h"
#include "video/ef9369.h"
#include "video/tms34061.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "guab.lh"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class guab_state : public driver_device
{
public:
	guab_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms34061(*this, "tms34061"),
		m_sn(*this, "snsnd"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:0"),
		m_palette(*this, "palette"),
		m_leds(*this, "led_%u", 0U),
		m_sound_buffer(0),
		m_sound_latch(false)
	{ }

	void guab(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	EF9369_COLOR_UPDATE(ef9369_color_update);
	void tms34061_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tms34061_r(offs_t offset, uint16_t mem_mask = ~0);
	uint32_t screen_update_guab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void output1_w(uint8_t data);
	void output2_w(uint8_t data);
	void output3_w(uint8_t data);
	void output4_w(uint8_t data);
	void output5_w(uint8_t data);
	void output6_w(uint8_t data);
	uint8_t sn76489_ready_r();
	void sn76489_buffer_w(uint8_t data);
	void system_w(uint8_t data);
	uint8_t watchdog_r();
	void watchdog_w(uint8_t data);

	static void floppy_formats(format_registration &fr);

	void guab_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<tms34061_device> m_tms34061;
	required_device<sn76489_device> m_sn;
	required_device<wd1773_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<palette_device> m_palette;

	output_finder<48> m_leds;

	uint8_t m_sound_buffer;
	bool m_sound_latch;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void guab_state::guab_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x040000, 0x04ffff).rom().region("maincpu", 0x10000);
	map(0x0c0000, 0x0c0007).rw("i8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0c0020, 0x0c0027).rw("i8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0c0040, 0x0c0047).rw("i8255_3", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0c0060, 0x0c0067).rw("i8255_4", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0c0080, 0x0c0083).rw("acia6850_1", FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0x0c00a0, 0x0c00a3).rw("acia6850_2", FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0x0c00c0, 0x0c00cf).rw("6840ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);
	map(0x0c00e0, 0x0c00e7).rw(m_fdc, FUNC(wd1773_device::read), FUNC(wd1773_device::write)).umask16(0x00ff);
	map(0x080000, 0x080fff).ram();
	map(0x100001, 0x100001).rw("ef9369", FUNC(ef9369_device::data_r), FUNC(ef9369_device::data_w));
	map(0x100002, 0x100003).nopr(); // uses a clr instruction on address which generates a dummy read
	map(0x100003, 0x100003).w("ef9369", FUNC(ef9369_device::address_w));
	map(0x800000, 0xb0ffff).rw(FUNC(guab_state::tms34061_r), FUNC(guab_state::tms34061_w));
	map(0xb10000, 0xb1ffff).ram();
	map(0xb80000, 0xb8ffff).ram();
	map(0xb90000, 0xb9ffff).ram();
}


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( guab )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME("50p")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )   PORT_NAME("100p")
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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("10p")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("20p")
INPUT_PORTS_END

static INPUT_PORTS_START( tenup )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME("50p")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )   PORT_NAME("100p")
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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("10p")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("20p")
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

EF9369_COLOR_UPDATE( guab_state::ef9369_color_update )
{
	m_palette->set_pen_color(entry, pal4bit(ca), pal4bit(cb), pal4bit(cc));
}

void guab_state::tms34061_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int func = (offset >> 19) & 3;
	int row = (offset >> 7) & 0xff;
	int col;

	if (func == 0 || func == 2)
		col = offset  & 0xff;
	else
		col = offset <<= 1;

	if (ACCESSING_BITS_8_15)
		m_tms34061->write(col, row, func, data >> 8);

	if (ACCESSING_BITS_0_7)
		m_tms34061->write(col | 1, row, func, data & 0xff);
}

uint16_t guab_state::tms34061_r(offs_t offset, uint16_t mem_mask)
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
		data |= m_tms34061->read(col, row, func) << 8;

	if (ACCESSING_BITS_0_7)
		data |= m_tms34061->read(col | 1, row, func);

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
		uint8_t const *const src = &m_tms34061->m_display.vram[256 * y];
		uint16_t *dest = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			uint8_t const pen = src[x >> 1];

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
	m_leds.resolve();

	m_fdc->set_floppy(m_floppy->get_device());
}

uint8_t guab_state::watchdog_r()
{
	// only read after writing the sequence below
	return 0xff;
}

void guab_state::watchdog_w(uint8_t data)
{
	// watchdog?
	// writes b 3 1 5 d a 2 0 4 0 8   b 3 1 5 d a 2 0 4 0 8
	// then later toggles between 0 and f
}

void guab_state::system_w(uint8_t data)
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
void guab_state::output1_w(uint8_t data)
{
	m_leds[0] = BIT(data, 0); // cash in (ten up: cash in)
	m_leds[1] = BIT(data, 1); // cash out (ten up: cash out)
	m_leds[2] = BIT(data, 2);
	m_leds[3] = BIT(data, 3);
	m_leds[4] = BIT(data, 4);
	m_leds[5] = BIT(data, 5);
	m_leds[6] = BIT(data, 6); // (ten up: 10p/100p drive)
	m_leds[7] = BIT(data, 7);
}

void guab_state::output2_w(uint8_t data)
{
	m_leds[8] = BIT(data, 0);
	m_leds[9] = BIT(data, 1);
	m_leds[10] = BIT(data, 2); // start (ten up: start)
	m_leds[11] = BIT(data, 3); // (ten up: feature 6)
	m_leds[12] = BIT(data, 4); // (ten up: feature 11)
	m_leds[13] = BIT(data, 5); // (ten up: feature 13)
	m_leds[14] = BIT(data, 6); // lamp a (ten up: feature 12)
	m_leds[15] = BIT(data, 7); // lamp b (ten up: pass)
}

void guab_state::output3_w(uint8_t data)
{
	m_leds[16] = BIT(data, 0); // select (ten up: collect)
	m_leds[17] = BIT(data, 1); // (ten up: feature 14)
	m_leds[18] = BIT(data, 2); // (ten up: feature 9)
	m_leds[19] = BIT(data, 3); //   (ten up: lamp a)
	m_leds[20] = BIT(data, 4); // lamp c (ten up: lamp b)
	m_leds[21] = BIT(data, 5); // lamp d (ten up: lamp c)
	m_leds[22] = BIT(data, 6);
	m_leds[23] = BIT(data, 7);
}

void guab_state::output4_w(uint8_t data)
{
	m_leds[24] = BIT(data, 0); // feature 1 (ten up: feature 1)
	m_leds[25] = BIT(data, 1); // feature 2 (ten up: feature 10)
	m_leds[26] = BIT(data, 2); // feature 3 (ten up: feature 7)
	m_leds[27] = BIT(data, 3); // feature 4 (ten up: feature 2)
	m_leds[28] = BIT(data, 4); // feature 5 (ten up: feature 8)
	m_leds[29] = BIT(data, 5); // feature 6 (ten up: feature 3)
	m_leds[30] = BIT(data, 6); // feature 7 (ten up: feature 4)
	m_leds[31] = BIT(data, 7); // feature 8 (ten up: feature 5)
}

void guab_state::output5_w(uint8_t data)
{
	m_leds[32] = BIT(data, 0);
	m_leds[33] = BIT(data, 1);
	m_leds[34] = BIT(data, 2);
	m_leds[35] = BIT(data, 3);
	m_leds[36] = BIT(data, 4);
	m_leds[37] = BIT(data, 5);
	m_leds[38] = BIT(data, 6);
	m_leds[39] = BIT(data, 7); // mech lamp (ten up: mech lamp)
}

void guab_state::output6_w(uint8_t data)
{
	m_leds[40] = BIT(data, 0);
	m_leds[41] = BIT(data, 1);
	m_leds[42] = BIT(data, 2);
	m_leds[43] = BIT(data, 3);
	m_leds[44] = BIT(data, 4); // 50p drive (ten up: 10p drive)
	m_leds[45] = BIT(data, 5); // 100p drive (ten up: 100p drive)
	m_leds[46] = BIT(data, 6);
	m_leds[47] = BIT(data, 7);
}


//**************************************************************************
//  AUDIO
//**************************************************************************

uint8_t guab_state::sn76489_ready_r()
{
	// bit 7 connected to sn76489 ready output (0 = ready)
	return ~(m_sn->ready_r() << 7);
}

void guab_state::sn76489_buffer_w(uint8_t data)
{
	m_sound_buffer = data;
}


//**************************************************************************
//  FLOPPY DRIVE
//**************************************************************************

void guab_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_GUAB_FORMAT);
}

static void guab_floppies(device_slot_interface &device)
{
	device.option_add("dd", FLOPPY_35_DD);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void guab_state::guab(machine_config &config)
{
	/* TODO: Verify clock */
	M68000(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &guab_state::guab_map);

	/* TODO: Use real video timings */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(guab_state::screen_update_guab));
	screen.set_palette(m_palette);


	PALETTE(config, m_palette).set_entries(ef9369_device::NUMCOLORS);

	EF9369(config, "ef9369").set_color_update_callback(FUNC(guab_state::ef9369_color_update));

	TMS34061(config, m_tms34061, 0);
	m_tms34061->set_rowshift(8);  /* VRAM address is (row << rowshift) | col */
	m_tms34061->set_vram_size(0x40000);
	m_tms34061->int_callback().set_inputline("maincpu", 5);
	m_tms34061->set_screen("screen");

	SPEAKER(config, "mono").front_center();

	/* TODO: Verify clock */
	SN76489(config, m_sn, 2000000);
	m_sn->add_route(ALL_OUTPUTS, "mono", 0.50);

	ptm6840_device &ptm(PTM6840(config, "6840ptm", 1000000));
	ptm.set_external_clocks(0, 0, 0);
	ptm.irq_callback().set_inputline("maincpu", 3);

	i8255_device &ppi1(I8255(config, "i8255_1"));
	ppi1.in_pa_callback().set_ioport("IN0");
	ppi1.in_pb_callback().set_ioport("IN1");
	ppi1.in_pc_callback().set_ioport("IN2");

	i8255_device &ppi2(I8255(config, "i8255_2"));
	ppi2.out_pa_callback().set(FUNC(guab_state::output1_w));
	ppi2.out_pb_callback().set(FUNC(guab_state::output2_w));
	ppi2.out_pc_callback().set(FUNC(guab_state::output3_w));

	i8255_device &ppi3(I8255(config, "i8255_3"));
	ppi3.out_pa_callback().set(FUNC(guab_state::output4_w));
	ppi3.out_pb_callback().set(FUNC(guab_state::output5_w));
	ppi3.out_pc_callback().set(FUNC(guab_state::output6_w));

	i8255_device &ppi4(I8255(config, "i8255_4"));
	ppi4.in_pa_callback().set(FUNC(guab_state::sn76489_ready_r));
	ppi4.out_pa_callback().set(FUNC(guab_state::sn76489_buffer_w));
	ppi4.out_pb_callback().set(FUNC(guab_state::system_w));
	ppi4.in_pc_callback().set(FUNC(guab_state::watchdog_r));
	ppi4.out_pc_callback().set(FUNC(guab_state::watchdog_w));

	bacta_datalogger_device &bacta(BACTA_DATALOGGER(config, "bacta", 0));

	acia6850_device &acia1(ACIA6850(config, "acia6850_1", 0));
	acia1.txd_handler().set("bacta", FUNC(bacta_datalogger_device::write_txd));
	acia1.irq_handler().set_inputline("maincpu", 4);

	bacta.rxd_handler().set("acia6850_1", FUNC(acia6850_device::write_rxd));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 19200)); // source? the ptm doesn't seem to output any common baud values
	acia_clock.signal_handler().set("acia6850_1", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia6850_1", FUNC(acia6850_device::write_rxc));

	ACIA6850(config, "acia6850_2", 0);

	// floppy
	WD1773(config, m_fdc, 8000000);
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, 6);
	FLOPPY_CONNECTOR(config, "fdc:0", guab_floppies, "dd", guab_state::floppy_formats).set_fixed(true);

	SOFTWARE_LIST(config, "floppy_list").set_original("guab");

	config.set_default_layout(layout_guab);
}


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

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  MACHINE  INPUT  CLASS       INIT        ROTATION  COMPANY  FULLNAME                FLAGS
GAME( 1986, guab,     0,      guab,    guab,  guab_state, empty_init, ROT0,     "JPM",   "Give us a Break",      0 )
GAME( 1986, crisscrs, 0,      guab,    guab,  guab_state, empty_init, ROT0,     "JPM",   "Criss Cross (Sweden)", MACHINE_NOT_WORKING )
GAME( 1988, tenup,    0,      guab,    tenup, guab_state, empty_init, ROT0,     "JPM",   "Ten Up",               0 )
