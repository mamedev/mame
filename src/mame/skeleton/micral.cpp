// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************

Bull (Originally R2E) Micral 80-22G

2015-10-01 Skeleton [Robbbert]

http://www.ti99.com/exelvision/website/index.php?page=r2e-micral-8022-g

This expensive, futuristic-looking design featured a motherboard and slots,
much like an ancient PC. The known chip complement is:
Z80A, 4MHz; 64KB RAM, 2KB BIOS ROM, 256x4 prom (7611);
CRT8002, TMS9937 (=CRT5037), 4KB video RAM, 256x4 prom (7611);
2x 5.25 inch floppy drives, one ST506 5MB hard drive;
CDP6402 UART. Sound is a beeper.
The keyboard has a uPD780C (=Z80) and 1KB of ROM.

The FDC and HDC are unknown.
No manuals, schematic or circuit description have been found.

Commands must be in uppercase. Reboot to exit each command.
Bx[,x]: ??
Gxxxx : go (writes a jump @FFED then jumps to FFEB)
T     : test
*     : echo keystrokes
enter : ??

Using generic keyboard via the uart for now. It's unknown how the real keyboard
communicates with the main cpu.

FFF8/9 are used for sending instructions to the screen. FFF9 is command/status,
and FFF8 is data. The codes 0-D seem to be for the CRT5037, but the results don't
make much sense. Code A0 is to write a byte to the current cursor position, and
B0 is to get the status.

Screen needs:
- Scrolling
- Proper character generator
- To be properly understood
- According to the web, graphics are possible. A screenshot shows reverse video
  exists.

Other things...
- Beeper
- 2 floppy drives
- keyboard
- unknown ports

--------------------
Honeywell Bull Questar/M

http://www.histoireinform.com/Histoire/+infos6/chr6inf3.htm
https://www.esocop.org/docs/Questar.pdf

*********************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
//#include "sound/beep.h"
#include "video/tms9927.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class micral_state : public driver_device
{
public:
	micral_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		//, m_beep(*this, "beeper")
		, m_p_chargen(*this, "chargen")
		, m_uart(*this, "uart")
		, m_crtc(*this, "crtc")
	{ }

	void micral(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	u8 keyin_r();
	u8 status_r();
	u8 unk_r();
	u8 video_r(offs_t offset);
	void video_w(offs_t offset, u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_kbd(address_map &map) ATTR_COLD;
	void mem_kbd(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u16 s_curpos = 0U;
	u8 s_command = 0U;
	u8 s_data = 0U;
	std::unique_ptr<u8[]> m_vram;
	memory_passthrough_handler m_rom_shadow_tap;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	//required_device<beep_device> m_beep;
	required_region_ptr<u8> m_p_chargen;
	required_device<ay31015_device> m_uart;
	required_device<crt5037_device> m_crtc;
};

u8 micral_state::status_r()
{
	return m_uart->dav_r() | 4;
}

u8 micral_state::unk_r()
{
	return 0x96;
}

u8 micral_state::keyin_r()
{
	m_uart->write_rdav(0);
	u8 result = m_uart->receive();
	m_uart->write_rdav(1);
	return result;
}

u8 micral_state::video_r(offs_t offset)
{
	if (offset)
		return 0x07;
	else
		return m_vram[s_curpos];
}

void micral_state::video_w(offs_t offset, u8 data)
{
	if (offset)
	{
		s_command = data;
		if (s_command == 0x0c)
			s_curpos = (s_curpos & 0xff00) | s_data;
		else
		if (s_command == 0x0d)
			s_curpos = (s_curpos & 0xff) | ((s_data & 0x1f) << 8);
		else
		if (s_command == 0xa0)
			m_vram[s_curpos] = s_data;

		//if (s_command < 0x10)
			//m_crtc->write(s_command, s_data);
	}
	else
	{
		s_data = data;
	}
}


void micral_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffef).ram().share("mainram");
	map(0xf800, 0xfeff).rom().region("maincpu", 0);
	map(0xfff6, 0xfff7); // .nopw(); // unknown ports
	map(0xfff8, 0xfff9).rw(FUNC(micral_state::video_r), FUNC(micral_state::video_w));
	map(0xfffa, 0xfffa).r(FUNC(micral_state::keyin_r));
	map(0xfffb, 0xfffb).r(FUNC(micral_state::unk_r));
	map(0xfffc, 0xfffc).r(FUNC(micral_state::status_r));
	map(0xfffd, 0xffff); // more unknown ports
}

void micral_state::mem_kbd(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).rom();
	map(0x8000, 0x8000).ram(); // byte returned to main cpu after receiving irq
	map(0x8001, 0x8001).portr("X0");
	map(0x8002, 0x8002).portr("X1");
	map(0x8004, 0x8004).portr("X2");
	map(0x8008, 0x8008).portr("X3");
	map(0x8010, 0x8010).portr("X4");
	map(0x8020, 0x8020).portr("X5");
	map(0x8040, 0x8040).portr("X6");
	map(0x8080, 0x8080).portr("X7");
	map(0x8100, 0x8100).portr("X8");
	map(0x8200, 0x8200).portr("X9");
	map(0x8400, 0x8400).portr("X10");
	map(0x8800, 0x8800).portr("X11");
	map(0x9000, 0x9000).portr("X12");
	map(0xa000, 0xa000).portr("X13");
}

void micral_state::io_kbd(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("X14");
}

/* Input ports */
static INPUT_PORTS_START( micral )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 01
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 03
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) // 2A
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) // 22
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) // 28
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) // 94
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) // 90
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 29

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 3E, 3C
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // '^'
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) // 5B
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // OB
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) // 3F
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // ':','/'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 08
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 06

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 02
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) // 91
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) // 27
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) // '-'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) // '_'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) // 8E
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) // '+'

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // '@', '#'
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 9C, '%'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 05
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 7F
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)

	PORT_START("X11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // ';','.'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // '!','&'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 0A
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00

	PORT_START("X12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 95,FE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 97,FC
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 9D,'$'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 96,'\'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 99,84
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 9A,92

	PORT_START("X13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00

	PORT_START("X14")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RCONTROL) // ?? don't look for a new keypress
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) // ??
INPUT_PORTS_END

uint32_t micral_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=0;

	for (uint8_t y = 0; y < 24; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = 0; x < 80; x++)
			{
				uint8_t gfx = 0;
				if (ra < 9)
				{
					uint8_t chr = m_vram[x+ma];
					gfx = m_p_chargen[(chr<<4) | ra ];
					if (((s_curpos & 0xff)==x) && ((s_curpos >> 8)==y))
						gfx ^= 0xff;
				}
				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=256;
	}
	return 0;
}

void micral_state::machine_reset()
{
	// no idea if these are hard-coded, or programmable
	m_uart->write_xr(0);
	m_uart->write_xr(1);
	m_uart->write_swe(0);
	m_uart->write_np(1);
	m_uart->write_tsb(0);
	m_uart->write_nb1(1);
	m_uart->write_nb2(1);
	m_uart->write_eps(1);
	m_uart->write_cs(1);
	m_uart->write_cs(0);

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

void micral_state::machine_start()
{
	m_vram = make_unique_clear<u8[]>(0x2000);
	save_pointer(NAME(m_vram), 0x2000);
	save_item(NAME(s_curpos));
	save_item(NAME(s_command));
	save_item(NAME(s_data));
}

void micral_state::micral(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &micral_state::mem_map);
	// no i/o ports on main cpu
	z80_device &keyboard(Z80(config, "keyboard", XTAL(1'000'000))); // freq unknown
	keyboard.set_addrmap(AS_PROGRAM, &micral_state::mem_kbd);
	keyboard.set_addrmap(AS_IO, &micral_state::io_kbd);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(250));
	screen.set_screen_update(FUNC(micral_state::screen_update));
	screen.set_size(640, 240);
	screen.set_visarea(0, 639, 0, 239);
	screen.set_palette("palette");
	PALETTE(config, "palette", palette_device::MONOCHROME);
	//GFXDECODE(config, "gfxdecode", "palette", gfx_micral);

	CRT5037(config, m_crtc, 4000000 / 8);  // xtal freq unknown
	m_crtc->set_char_width(8);  // unknown
	//m_crtc->vsyn_callback().set(TMS5501_TAG, FUNC(tms5501_device::sens_w));
	m_crtc->set_screen("screen");

	/* sound hardware */
	//SPEAKER(config, "mono").front_center();
	//BEEP(config, m_beep, 2000).add_route(ALL_OUTPUTS, "mono", 0.50);

	AY31015(config, m_uart); // CDP6402
	m_uart->read_si_callback().set("rs232", FUNC(rs232_port_device::rxd_r));
	m_uart->write_so_callback().set("rs232", FUNC(rs232_port_device::write_txd));

	clock_device &uart_clock(CLOCK(config, "uart_clock", 153600));
	uart_clock.signal_handler().set(m_uart, FUNC(ay31015_device::write_tcp));
	uart_clock.signal_handler().append(m_uart, FUNC(ay31015_device::write_rcp));

	RS232_PORT(config, "rs232", default_rs232_devices, "keyboard");
}

ROM_START( micral )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "8022g.rom",    0x0000, 0x0800, CRC(882732a9) SHA1(3f37b82c450a54aedec209bd46fcbcf131c86313) )

	ROM_REGION( 0x0400, "keyboard", 0 )
	ROM_LOAD( "2010221.rom",  0x0000, 0x0400, CRC(65123378) SHA1(401f0a648b78bf1662a1cd2546e83ba8e3cb7a42) )

	// Using the chargen from 'c10' for now.
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

ROM_START( questarm )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "qm_547_1.rom", 0x0000, 0x0800, CRC(8e6dc953) SHA1(b31375af8c6769578d2000fff3e751e94e7ae4d4) )

	// using the keyboard ROM from 'micral' for now
	ROM_REGION( 0x0400, "keyboard", 0 )
	ROM_LOAD( "2010221.rom",  0x0000, 0x0400, CRC(65123378) SHA1(401f0a648b78bf1662a1cd2546e83ba8e3cb7a42) )

	// Using the chargen from 'c10' for now.
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY           FULLNAME         FLAGS
COMP( 1981, micral,   0,      0,      micral,  micral, micral_state, empty_init, "Bull R2E",       "Micral 80-22G", MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )
COMP( 1982, questarm, micral, 0,      micral,  micral, micral_state, empty_init, "Honeywell Bull", "Questar/M",     MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )
