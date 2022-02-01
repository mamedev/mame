// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

Sanyo MBC-200

Machine MBC-1200 is identical but sold outside of Japan

16 x HM6116P-3 2K x 8 SRAM soldered onboard (so 32k ram)
4 x HM6116P-3 2K x 8 SRAM socketed (so 8k ram)
4 x MB83256 32K x 8 socketed (128k ram)
Floppy = 5.25"
MBC1200 has one floppy while MBC1250 has 2. The systems are otherwise identical.

Keyboard communicates via RS232 to uart at E0,E1. The processor and rom for it
are undumped / unknown. The input codes are not ascii, so using custom code until
the required details become available.

On back side:
- keyboard DIN connector
- Centronics printer port
- RS-232C 25pin connector

SBASIC:
Running programs: the file names used within SBASIC must be in
uppercase. For example, run "DEMO" .
You can also run a basic program from CP/M: sbasic "GRAPHICS" .
To Break, press either ^N or ^O (display freezes), then ^C .
Some control keys: 0x14 = Home; 0x8 = Left/BS; 0xA = Down; 0xB = Up; 0xC = Right.
GAIJI.BAS doesn't work because GAIJI.FNT is missing.

TODO:
- Other connections to the various PPI's
- UART connections
- Any other devices?

2011-10-31 Skeleton driver.
2014-05-18 Made rom get copied into ram, boot code from disk
           requires that ram is there otherwise you get
           a MEMORY ERROR. CP/M now loads.

2016-07-16 Added keyboard and sound.

****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/keyboard.h"
#include "machine/wd_fdc.h"
#include "sound/beep.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class mbc200_state : public driver_device
{
public:
	mbc200_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_crtc(*this, "crtc")
		, m_ppi_m(*this, "ppi_m")
		, m_vram(*this, "vram")
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_beep(*this, "beeper")
		, m_speaker(*this, "speaker")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
	{ }

	void mbc200(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u8 p2_porta_r();
	void p1_portc_w(u8 data);
	void pm_porta_w(u8 data);
	void pm_portb_w(u8 data);
	u8 keyboard_r(offs_t offset);
	void kbd_put(u8 data);
	MC6845_UPDATE_ROW(update_row);
	required_device<palette_device> m_palette;

	void main_mem(address_map &map);
	void main_io(address_map &map);
	void sub_mem(address_map &map);
	void sub_io(address_map &map);

	u8 m_comm_latch = 0U;
	u8 m_term_data = 0U;
	required_device<mc6845_device> m_crtc;
	required_device<i8255_device> m_ppi_m;
	required_shared_ptr<u8> m_vram;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_device<beep_device> m_beep;
	required_device<speaker_sound_device> m_speaker;
	required_device<mb8876_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};


void mbc200_state::main_mem(address_map &map)
{
	map(0x0000, 0xffff).ram().share("mainram");
}

void mbc200_state::p1_portc_w(u8 data)
{
	m_speaker->level_w(BIT(data,4)); // used by beep command in basic
}

void mbc200_state::pm_porta_w(u8 data)
{
	machine().scheduler().synchronize(); // force resync
	//printf("A %02x %c\n",data,data);
	m_comm_latch = data; // to slave CPU
}

void mbc200_state::pm_portb_w(u8 data)
{
	floppy_image_device *floppy = nullptr;

	// to be verified
	switch (data & 0x01)
	{
	case 0: floppy = m_floppy0->get_device(); break;
	case 1: floppy = m_floppy1->get_device(); break;
	}

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 7));
	}
	m_beep->set_state(BIT(data, 1)); // key-click
}

void mbc200_state::main_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	//map(0xe0, 0xe1).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xe0, 0xe1).r(FUNC(mbc200_state::keyboard_r)).nopw();
	map(0xe4, 0xe7).rw(m_fdc, FUNC(mb8876_device::read), FUNC(mb8876_device::write));
	map(0xe8, 0xeb).rw(m_ppi_m, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xec, 0xed).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
}



void mbc200_state::sub_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x7fff).ram();
	map(0x8000, 0xffff).ram().share("vram");
}

u8 mbc200_state::p2_porta_r()
{
	machine().scheduler().synchronize(); // force resync
	u8 tmp = m_comm_latch;
	m_comm_latch = 0;
	m_ppi_m->pc6_w(0); // ppi_ack
	return tmp;
}

void mbc200_state::sub_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x70, 0x73).rw("ppi_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb0, 0xb0).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xb1, 0xb1).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xd0, 0xd3).rw("ppi_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

/* Input ports */
static INPUT_PORTS_START( mbc200 )
INPUT_PORTS_END

u8 mbc200_state::keyboard_r(offs_t offset)
{
	u8 data = 0;
	if (offset)
	{
		if (m_term_data)
		{
			data = 2;
			// handle CTRL key pressed
			if (m_term_data < 0x20)
			{
				data |= 8;
				m_term_data |= 0x40;
			}
		}
	}
	else
	{
		data = m_term_data;
		m_term_data = 0;
	}

	return data;
}

// convert standard control keys to expected code;
void mbc200_state::kbd_put(u8 data)
{
	switch (data)
	{
		case 0x0e:
			m_term_data = 0xe2;
			break;
		case 0x0f:
			m_term_data = 0xe3;
			break;
		case 0x08:
			m_term_data = 0xe4;
			break;
		case 0x09:
			m_term_data = 0xe5;
			break;
		case 0x0a:
			m_term_data = 0xe6;
			break;
		case 0x0d:
			m_term_data = 0xe7;
			break;
		case 0x1b:
			m_term_data = 0xe8;
			break;
		default:
			m_term_data = data;
	}
}

void mbc200_state::machine_start()
{
	save_item(NAME(m_comm_latch));
	save_item(NAME(m_term_data));
}

void mbc200_state::machine_reset()
{
	memcpy(m_ram, m_rom, 0x1000);
}

static void mbc200_floppies(device_slot_interface &device)
{
	device.option_add("qd", FLOPPY_525_QD);
}

MC6845_UPDATE_ROW( mbc200_state::update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);

	for (u16 x = 0; x < x_count; x++)
	{
		u16 mem = (ma+x)*4+ra;
		u8 gfx = m_vram[mem & 0x7fff];
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

static const gfx_layout charlayout =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_mbc200 )
	GFXDECODE_ENTRY( "subcpu", 0x1800, charlayout, 0, 1 )
GFXDECODE_END


void mbc200_state::mbc200(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // NEC D780C-1
	m_maincpu->set_addrmap(AS_PROGRAM, &mbc200_state::main_mem);
	m_maincpu->set_addrmap(AS_IO, &mbc200_state::main_io);

	z80_device &subcpu(Z80(config, "subcpu", 8_MHz_XTAL / 2)); // NEC D780C-1
	subcpu.set_addrmap(AS_PROGRAM, &mbc200_state::sub_mem);
	subcpu.set_addrmap(AS_IO, &mbc200_state::sub_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 400);
	screen.set_visarea(0, 640-1, 0, 400-1);
	screen.set_screen_update("crtc", FUNC(hd6845s_device::screen_update));
	GFXDECODE(config, "gfxdecode", m_palette, gfx_mbc200);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	HD6845S(config, m_crtc, 8_MHz_XTAL / 4); // HD46505SP
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(mbc200_state::update_row));

	// sound
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1000).add_route(ALL_OUTPUTS, "mono", 0.50); // frequency unknown
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	I8255(config, "ppi_1").out_pc_callback().set(FUNC(mbc200_state::p1_portc_w));
	I8255(config, "ppi_2").in_pa_callback().set(FUNC(mbc200_state::p2_porta_r));

	I8255(config, m_ppi_m);
	m_ppi_m->out_pa_callback().set(FUNC(mbc200_state::pm_porta_w));
	m_ppi_m->out_pb_callback().set(FUNC(mbc200_state::pm_portb_w));

	I8251(config, "uart1", 0); // INS8251N

	I8251(config, "uart2", 0); // INS8251A

	MB8876(config, m_fdc, 8_MHz_XTAL / 8); // guess
	FLOPPY_CONNECTOR(config, "fdc:0", mbc200_floppies, "qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", mbc200_floppies, "qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	/* Keyboard */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(mbc200_state::kbd_put));

	/* software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("mbc200");
}

/* ROM definition */
ROM_START( mbc200 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "d2732a.bin",  0x0000, 0x1000, CRC(bf364ce8) SHA1(baa3a20a5b01745a390ef16628dc18f8d682d63b))

	ROM_REGION( 0x3000, "subcpu", ROMREGION_ERASEFF )
	ROM_LOAD( "m5l2764.bin", 0x0000, 0x2000, CRC(377300a2) SHA1(8563172f9e7f84330378a8d179f4138be5fda099))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME   FLAGS
COMP( 1982, mbc200, 0,      0,      mbc200,  mbc200, mbc200_state, empty_init, "Sanyo", "MBC-200", MACHINE_SUPPORTS_SAVE )
