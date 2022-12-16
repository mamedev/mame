// license:BSD-3-Clause
// copyright-holders:Robbbert
/*****************************************************************************************

Amust Compak - also known as Amust Executive 816.

2014-03-21 Skeleton driver. [Robbbert]

An unusual-looking CP/M computer. The screen is a tiny CRT not much bigger
than a modern smartphone.

Z-80A @ 4MHz; 64KB dynamic RAM (8x 4164); 2KB video ram (6116); 2x 13cm drives;
80 track DD with data capacity of 790KB; in a lockable Samsonite briefcase.

There are no manuals or schematics known to exist.
The entire driver is guesswork.
The board has LH0080 (Z80A), 2x 8251, 2x 8255, 8253, uPD765A and a HD46505SP-2.
There is a piezo beeper. There are 3 crystals, X1 = 4.9152 (serial chips),
X2 = 16 (CPU), X3 = 14.31818 MHz (Video).
There are numerous jumpers, all of which perform unknown functions.

The keyboard is a plug-in unit, same idea as Kaypro and Zorba. It has these
chips: INS8035N-6, F74145, 74LS373N, SN75451BP, 2716 rom with label KBD-3.
Crystal: 3.579545 MHz

The main rom is identical between the 2 halves, except that the initial
crtc parameters are slightly different. I've chosen to ignore the first
half. (perhaps 50/60 Hz selectable by jumper?)

Preliminary I/O ports
---------------------
00-01 uart 1
02-03 uart 2
04-07 ppi 1
08-0b ppi 2
0d-0f crtc
10-11 fdc
14-17 pit

PIT.
Having the PIT on ports 14-17 seems to make sense. It sets counters 1 and 2
to mode 3, binary, initial count = 0x80. Counter 0 not used?


Floppy Parameters:
------------------
Double Density
Two Side
80 track
1024 byte sectors
5 sectors/track
800k capacity
128 directory entries
2k block size
Skew 1,3,5,2,4


Monitor Commands:
-----------------
B = Boot from floppy
(YES! Most useless monitor ever)


ToDo:
- Everything
- Floppy issues:
  - The loop to read a sector has no escape. The interrupt handler (which can't be found)
    needs to take another path when the sector is complete.
  - The loop uses "ini" to read a byte, but this doesn't clear DRQ, so memory rapidly fills
    up with garbage, mostly FF.
- Keyboard controller needs to be emulated.
- If booting straight to CP/M, the load message should be in the middle of the screen.
- Looks like port 5 has a row of function keys or similar. Need to be added.


*******************************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "sound/beep.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class amust_state : public driver_device
{
public:
	amust_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_p_chargen(*this, "chargen")
		, m_beep(*this, "beeper")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_beep_timer(*this, "beep_timer")
	{ }

	void amust(machine_config &config);

private:
	u8 port04_r();
	void port04_w(u8 data);
	u8 port05_r();
	u8 port06_r();
	void port06_w(u8 data);
	u8 port08_r();
	void port08_w(u8 data);
	u8 port09_r();
	u8 port0a_r();
	void port0a_w(u8 data);
	void port0d_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(hsync_w);
	DECLARE_WRITE_LINE_MEMBER(vsync_w);
	DECLARE_WRITE_LINE_MEMBER(drq_w);
	DECLARE_WRITE_LINE_MEMBER(intrq_w);
	void kbd_put(u8 data);
	MC6845_UPDATE_ROW(crtc_update_row);
	TIMER_DEVICE_CALLBACK_MEMBER(beep_timer);

	void io_map(address_map &map);
	void mem_map(address_map &map);
	void machine_reset() override;
	void machine_start() override;
	void do_int();

	u8 m_port04 = 0U;
	u8 m_port06 = 0U;
	u8 m_port08 = 0U;
	u8 m_port09 = 0U;
	u8 m_port0a = 0U;
	u8 m_term_data = 0U;
	bool m_drq = 0;
	//bool m_intrq = 0;
	bool m_hsync = 0;
	bool m_vsync = 0;
	std::unique_ptr<u8[]> m_vram;
	memory_passthrough_handler m_rom_shadow_tap;
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_memory_bank    m_bank1;
	required_region_ptr<u8> m_p_chargen;
	required_device<beep_device> m_beep;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<timer_device> m_beep_timer;
};

TIMER_DEVICE_CALLBACK_MEMBER(amust_state::beep_timer)
{
	m_beep->set_state(0);
}

//void amust_state::port00_w(u8 data)
//{
//  membank("bankr0")->set_entry(BIT(data, 6));
//  m_fdc->dden_w(BIT(data, 5));
//  floppy_image_device *floppy = nullptr;
//  if (BIT(data, 0)) floppy = m_floppy0->get_device();
//  m_fdc->set_floppy(floppy);
//  if (floppy)
//      floppy->ss_w(BIT(data, 4));
//}

void amust_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("mainram");
	map(0xf800, 0xffff).bankr("bank1");
}

void amust_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x01).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x02, 0x03).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x04, 0x07).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0d, 0x0d).nopr().w(FUNC(amust_state::port0d_w));
	map(0x0e, 0x0e).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x0f, 0x0f).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x10, 0x11).m(m_fdc, FUNC(upd765a_device::map));
	map(0x14, 0x17).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}

static void amust_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

/* Input ports */
static INPUT_PORTS_START( amust )
	PORT_START("P9")
	// bits 6,7 not used?
	// bit 5 - fdc intrq
	PORT_DIPNAME( 0x01, 0x01, "Bit0" ) // code @ FC99
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x02, "Bit1" )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x04, "Bit2" )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x08, "Bit3" )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x10, "Boot to Monitor" ) // code @ F895
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
INPUT_PORTS_END

void amust_state::do_int()
{
	bool sync = m_hsync | m_vsync;

	if ((BIT(m_port0a, 3) && sync)             // when writing to the screen, only do it during blanking
		|| (BIT(m_port0a, 5) && m_drq))        // when reading from floppy, only do it when DRQ is high.
	{
		//printf("%X,%X,%X ",m_port0a,sync,m_drq);
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0x00); // Z80
	}
	else
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

WRITE_LINE_MEMBER( amust_state::drq_w )
{
	m_drq = state;
	do_int();
	m_fdc->tc_w(1);
}

WRITE_LINE_MEMBER( amust_state::intrq_w )
{
	m_port09 = (m_port09 & 0xdf) | (state ? 0x20 : 0);
}

WRITE_LINE_MEMBER( amust_state::hsync_w )
{
	m_hsync = state;
	do_int();
}

WRITE_LINE_MEMBER( amust_state::vsync_w )
{
	m_vsync = state;
	do_int();
}

u8 amust_state::port04_r()
{
	return m_port04;
}

void amust_state::port04_w(u8 data)
{
	m_port04 = data;
}

u8 amust_state::port05_r()
{
	return 0;
}

u8 amust_state::port06_r()
{
	return m_port06;
}

// BIT 5 low while writing to screen
void amust_state::port06_w(u8 data)
{
	m_port06 = data;
}

u8 amust_state::port08_r()
{
	return m_port08;
}

// lower 8 bits of video address
void amust_state::port08_w(u8 data)
{
	m_port08 = data;
}

/*
d0 - something to do with type of disk
d1 -
d2 -
d3 -
d4 - H = go to monitor; L = boot from disk
d5 - status of fdc intrq; loops till NZ
d6 -
d7 -
*/
u8 amust_state::port09_r()
{
	logerror("%s\n",machine().describe_context());
	return (ioport("P9")->read() & 0x1f) | m_port09;
}

u8 amust_state::port0a_r()
{
	return m_port0a;
}

/* Bits 7,6,5,3 something to do
with selecting which device causes interrupt?
50, 58 = video sync
70 disk
D0 ?
Bit 4 low = beeper.
Lower 3 bits = upper part of video address */
void amust_state::port0a_w(u8 data)
{
	m_port0a = data;

	if (!BIT(data, 4))
	{
		m_beep->set_state(1);
		m_beep_timer->adjust(attotime::from_msec(150));
	}
	floppy_image_device *floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	if (floppy)
	{
		floppy->mon_w(0);

		//floppy->ss_w(0);   // side 0? hopefully fdc does this
	}
}

void amust_state::port0d_w(u8 data)
{
	uint16_t video_address = m_port08 | ((m_port0a & 7) << 8);
	m_vram[video_address] = data;
}

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8,                  /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                    /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_amust )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

MC6845_UPDATE_ROW( amust_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 *p = &bitmap.pix(y);

	for (u16 x = 0; x < x_count; x++)
	{
		u8 inv = (x == cursor_x) ? 0xff : 0;
		u16 mem = (ma + x) & 0x7ff;
		u8 chr = m_vram[mem];
		u8 gfx;
		if (ra < 8)
			gfx = m_p_chargen[(chr<<3) | ra] ^ inv;
		else
			gfx = inv;

		/* Display a scanline of a character (8 pixels) */
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

void amust_state::machine_reset()
{
	m_bank1->set_entry(1);
	m_port04 = 0;
	m_port06 = 0;
	m_port08 = 0;
	m_port09 = 0;
	m_port0a = 0;
	m_hsync = false;
	m_vsync = false;
	m_drq = false;
	m_fdc->set_ready_line_connected(1); // always ready for minifloppy; controlled by fdc for 20cm
	m_fdc->set_unscaled_clock(4000000); // 4MHz for minifloppy; 8MHz for 20cm

	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xf800, 0xffff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x07ff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

void amust_state::machine_start()
{
	m_vram = make_unique_clear<u8[]>(0x800);
	save_pointer(NAME(m_vram), 0x800);
	save_item(NAME(m_port04));
	save_item(NAME(m_port06));
	save_item(NAME(m_port08));
	save_item(NAME(m_port09));
	save_item(NAME(m_port0a));
	save_item(NAME(m_term_data));
	save_item(NAME(m_drq));
	//save_item(NAME(m_intrq));
	save_item(NAME(m_hsync));
	save_item(NAME(m_vsync));
	m_bank1->configure_entry(0, m_ram+0xf800);
	m_bank1->configure_entry(1, m_rom);
}

void amust_state::amust(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(16'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &amust_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &amust_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	PALETTE(config, m_palette, palette_device::MONOCHROME);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_amust);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 800).add_route(ALL_OUTPUTS, "mono", 0.50);
	TIMER(config, m_beep_timer).configure_generic(FUNC(amust_state::beep_timer));

	/* Devices */
	hd6845s_device &crtc(HD6845S(config, "crtc", XTAL(14'318'181) / 8));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(amust_state::crtc_update_row));
	crtc.out_hsync_callback().set(FUNC(amust_state::hsync_w));
	crtc.out_vsync_callback().set(FUNC(amust_state::vsync_w));

	UPD765A(config, m_fdc, 8'000'000, true, true);
	m_fdc->drq_wr_callback().set(FUNC(amust_state::drq_w));
	m_fdc->intrq_wr_callback().set(FUNC(amust_state::intrq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", amust_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", amust_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	clock_device &uart_clock(CLOCK(config, "uart_clock", 153600));
	uart_clock.signal_handler().set("uart1", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart1", FUNC(i8251_device::write_rxc));

	i8251_device &uart1(I8251(config, "uart1", 0));
	uart1.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart1.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	uart1.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "keyboard"));
	rs232.rxd_handler().set("uart1", FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set("uart1", FUNC(i8251_device::write_cts));
	rs232.dsr_handler().set("uart1", FUNC(i8251_device::write_dsr));

	I8251(config, "uart2", 0);
	//uart2.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	//uart2.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	//uart2.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	PIT8253(config, "pit", 0);

	i8255_device &ppi1(I8255A(config, "ppi1"));
	ppi1.in_pa_callback().set(FUNC(amust_state::port04_r));
	ppi1.out_pa_callback().set(FUNC(amust_state::port04_w));
	ppi1.in_pb_callback().set(FUNC(amust_state::port05_r));
	ppi1.in_pc_callback().set(FUNC(amust_state::port06_r));
	ppi1.out_pc_callback().set(FUNC(amust_state::port06_w));

	i8255_device &ppi2(I8255A(config, "ppi2"));
	ppi2.in_pa_callback().set(FUNC(amust_state::port08_r));
	ppi2.out_pa_callback().set(FUNC(amust_state::port08_w));
	ppi2.in_pb_callback().set(FUNC(amust_state::port09_r));
	ppi2.in_pc_callback().set(FUNC(amust_state::port0a_r));
	ppi2.out_pc_callback().set(FUNC(amust_state::port0a_w));
}

/* ROM definition */
ROM_START( amust )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mon_h.ic25", 0x0000, 0x1000, CRC(10dceac6) SHA1(1ef80039063f7a6455563d59f1bcc23e09eca369) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "cg4.ic74",   0x000, 0x800, CRC(52e7b9d8) SHA1(cc6d457634eb688ccef471f72bddf0424e64b045) )

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "kbd_3.rom",  0x000, 0x800, CRC(d9441b35) SHA1(ce250ab1e892a13fd75182703f259855388c6bf4) )
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME               FLAGS
COMP( 1983, amust, 0,      0,      amust,   amust, amust_state, empty_init, "Amust", "Amust Executive 816", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
