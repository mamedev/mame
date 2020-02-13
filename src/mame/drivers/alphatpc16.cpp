// license:BSD-3-Clause
// copyright-holders:Carl

/******************************************************************************************
The Alphatronic PC 16 was modeled after the earlier 8 bit Alphatronic PC – a typical "breadbox"
home computer case with an integrated keyboard and external floppy disk drives. It came out in
the early phase of DOS computers, when IBM’s standard was not yet set into stone, but would
vanquish all "incompatible" solutions within short time. The start screen allows the selection
of functions by entering the first (two) letters of the command or using the F-key as shown on the screen.

CPU: 8088@5MHz - RAM: 64KB, can be extended to 128KB - ROM: 64KB (monitor and BASIC)

Keyboard: ALP: the key marked with a Greek alpha character makes the computer/printer combo a
          typewriter, everything is typed "through".
          BRK: BREAK stops a BASIC program or acts like CTRL-C
          GRAPH: allows the entering of semigraphics characters, locks into place like CAPSLK
          numeric and BTX function keys:
          UP, DWN, LFT and RWT are marked with their respective arrows and move the cursor
          DEL is marked (R) for BTX mode and reloads the page
          CLR is marked (i) in BTX mode for “info” and loads the start page
          = is marked with a page that’s being turned and is a "reveal" key, e.g. for quizzes
          <-/ is marked with an envelope, it loads BTX’s messaging service
          the + and – keys aren’t defined yet in BTX mode, they are marked with a page and an arrow each
         * shows a screen with a black/white contrast and turns BTX screen attributes on and off
          / has a telephone receiver and three lines and is the online/offline/dialing key

                                                  [RST] [F1 ] [F2] [F3 ] [F4 ] [F5 ] [F6 ]

[ESC] [1 !] [2 ”] [3 §] [4 $] [5 %] [6 &] [7 /] [8 (][9 )] [0 =] [ß ?] [´ `] [BRK] [GRAPH]   [ 7 ] [ 8 ] [ 9 ] [ * ] [ / ]
[TAB  ] [ Q ] [ W ] [ E ] [ R ] [ T ] [ Z ] [ U ] [ I ] [ O ] [ P ] [ Ü ] [+ *] [CTR] [| ]   [ 4 ] [ 5 ] [ 6 ] [ + ] [ - ]
[CAPSLK]  [ A ] [ S ] [ D ] [ F ] [ G ] [ H ] [ J ] [ K ] [ L ] [ Ö ] [ Ä ] [^ #] [  <-- ]   [ 1 ] [ 2 ] [ 3 ] [ = ] [<-/]
[SHFT] [< >] [ Y ] [ X ] [ C ] [ V ] [ B ] [ N ] [ M ] [, ;] [: .] [- _] [  SHFT  ]  [ALP]   [ 0 ] [UP ] [ . ] [DEL] [CLR]
                   [                      LEER                   ]                           [LFT] [DWN] [RGT] [INS] [HOM]

Graphics options: Standard monitor cassette (Cinch for bw and DIN for SCART/RGB connectors) 40/80x21/25
characters, Full graphics cassette (512x256 pixels, 16 colours, vector graphics, 64K video RAM),
BTX cassette (compatible with the standard monitor cassette but includes a modem for BTX functionality)
Floppy: 1 or 2 5.25” DSDD 40 tracks x 5 sectors x 1024 bytes
Connectors: joystick, cassette recorder (600/1200 BD) FSK, printer (recommended: TRD 7020 or
GABRIELE 9009 typewriter, V24), floppy, module slot
Options: 2 versions of an autonomous processor PCB (Z8671 based, programmable in TinyBasic
via the PC 16 Terminal, operates independently after programming), connects to the printer port
******************************************************************************************/

#include "emu.h"

#include "cpu/i86/i86.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "video/ef9345.h"
#include "machine/z80dart.h"
#include "machine/pic8259.h"
#include "machine/timer.h"
#include "machine/ram.h"
#include "sound/beep.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class alphatpc16_state : public driver_device
{
public:
	alphatpc16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic8259(*this, "pic8259"),
		m_ef9345(*this, "ef9345"),
		m_beep(*this, "beeper"),
		m_wdfdc(*this, "wdfdc"),
		m_ram(*this, RAM_TAG)
	{ }

public:
	void alphatpc16(machine_config &config);

private:
	virtual void machine_start() override;

	void apc16_io(address_map &map);
	void apc16_map(address_map &map);
	void apc16_z80_io(address_map &map);
	void apc16_z80_map(address_map &map);
	void ef9345(address_map &map);

	DECLARE_READ8_MEMBER(p1_r);
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_READ8_MEMBER(p2_r);
	DECLARE_WRITE8_MEMBER(p2_w);
	DECLARE_READ8_MEMBER(host_scsi_r);
	DECLARE_WRITE8_MEMBER(host_scsi_w);
	DECLARE_READ8_MEMBER(flop_scsi_r);
	DECLARE_WRITE8_MEMBER(flop_scsi_w);

	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic8259;
	required_device<ef9345_device> m_ef9345;
	required_device<beep_device> m_beep;
	required_device<wd1770_device> m_wdfdc;
	required_device<ram_device> m_ram;

	u8 m_p1, m_p2;
};

void alphatpc16_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->size() - 1, m_ram->pointer());
}

WRITE8_MEMBER(alphatpc16_state::p1_w)
{
	if((data == 0xff) && !BIT(m_p2, 7))
		m_p2 &= ~3; //??
	m_p1 = data;
}

READ8_MEMBER(alphatpc16_state::p1_r)
{
	return m_p1;
}

WRITE8_MEMBER(alphatpc16_state::p2_w)
{
	m_beep->set_state(!BIT(data, 3));
	m_pic8259->ir0_w(BIT(data, 5));
	m_p2 = data;
}

READ8_MEMBER(alphatpc16_state::p2_r)
{
	return m_p2;
}

WRITE8_MEMBER(alphatpc16_state::host_scsi_w)
{
}

READ8_MEMBER(alphatpc16_state::host_scsi_r)
{
	return 0;
}

WRITE8_MEMBER(alphatpc16_state::flop_scsi_w)
{
}

READ8_MEMBER(alphatpc16_state::flop_scsi_r)
{
	return 0;
}

void alphatpc16_state::apc16_map(address_map &map)
{
	map(0x80020, 0x8002f).rw(m_ef9345, FUNC(ef9345_device::data_r), FUNC(ef9345_device::data_w));
	map(0x82000, 0x82001).rw("i8741", FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0x84000, 0x84003).rw("z80dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x86000, 0x86001).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x8a000, 0x8a003).rw(FUNC(alphatpc16_state::host_scsi_r), FUNC(alphatpc16_state::host_scsi_w));
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void alphatpc16_state::apc16_io(address_map &map)
{
}

void alphatpc16_state::apc16_z80_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("flop", 0);
	map(0x4000, 0x47ff).ram();
}

void alphatpc16_state::apc16_z80_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x20, 0x23).rw(m_wdfdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write));
	map(0x62, 0x63) .rw(FUNC(alphatpc16_state::flop_scsi_r), FUNC(alphatpc16_state::flop_scsi_w));
}

static INPUT_PORTS_START( alphatpc16 )
INPUT_PORTS_END

void alphatpc16_state::alphatpc16(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, 15_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &alphatpc16_state::apc16_map);
	m_maincpu->set_addrmap(AS_IO, &alphatpc16_state::apc16_io);
	m_maincpu->set_irq_acknowledge_callback(m_pic8259, FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic8259, 0);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	z80dart_device &dart(Z80DART(config, "z80dart", 15_MHz_XTAL / 3)); // clock?
	dart.out_int_callback().set(m_pic8259, FUNC(pic8259_device::ir7_w));

	cpu_device& z80(Z80(config, "z80", 8_MHz_XTAL / 2));
	z80.set_addrmap(AS_PROGRAM, &alphatpc16_state::apc16_z80_map);
	z80.set_addrmap(AS_IO, &alphatpc16_state::apc16_z80_io);
	WD1770(config, m_wdfdc, 8_MHz_XTAL);

	i8741a_device& i8741(I8741A(config, "i8741", 4.608_MHz_XTAL));
	i8741.p1_in_cb().set(FUNC(alphatpc16_state::p1_r));
	i8741.p1_out_cb().set(FUNC(alphatpc16_state::p1_w));
	i8741.p2_in_cb().set(FUNC(alphatpc16_state::p2_r));
	i8741.p2_out_cb().set(FUNC(alphatpc16_state::p2_w));

	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 1000).add_route(ALL_OUTPUTS, "mono", 1.00); // Unknown freq

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_screen_update(m_ef9345, FUNC(ef9345_device::screen_update));
	screen.set_size(492, 270);
	screen.set_visarea(00, 492-1, 00, 270-1);
	PALETTE(config, "palette").set_entries(8);

	EF9345(config, m_ef9345, 0);
	m_ef9345->set_palette_tag("palette");

	TIMER(config, "scanline").configure_scanline(NAME([this](timer_device &t, void *ptr, s32 p){m_ef9345->update_scanline((uint16_t)p);}), screen, 0, 10);

	// these are supported by the bios, they may not have been available on real hardware
	RAM(config, m_ram).set_default_size("64K").set_extra_options("128K,192K,256K,384K,448K,512K");
}

ROM_START( alphatpc16 )
	ROM_REGION(0x10000,"bios", 0)
	ROM_LOAD("cdae03_04_16.bin", 0x0000, 0x8000, CRC(0ff5b549) SHA1(f5612e7864c06da586087645fed97c78e84a5d04))
	ROM_LOAD("cdae02_07_15.bin", 0x8000, 0x8000, CRC(22fd3acb) SHA1(ddab380dd15326ca699d6b4b7f4bf7c1a9d498ea))

	ROM_REGION(0x400, "i8741", 0)
	ROM_LOAD("d8741ad.bin", 0x000, 0x400, CRC(e71d5d9f) SHA1(deda490491d3ee08f47bd23bb29dc92b3806d3f2))

	ROM_REGION(0x4000, "flop", 0)
	ROM_LOAD("cdae04_03_21.bin", 0x0000, 0x4000, CRC(11bc6551) SHA1(28c1f02fdc040035aba249c4ad21de9b5ec95298))

	ROM_REGION( 0x4000, "ef9345", 0 )
	ROM_LOAD( "charset.rom", 0x0000, 0x2000, BAD_DUMP CRC(b2f49eb3) SHA1(d0ef530be33bfc296314e7152302d95fdf9520fc) )                // from dcvg5k
ROM_END

COMP( 198?, alphatpc16,  0, 0, alphatpc16, alphatpc16, alphatpc16_state, empty_init, "Triumph-Adler", "alphatronic PC-16",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

