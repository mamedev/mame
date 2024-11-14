// license:BSD-3-Clause
// copyright-holders: rfka01
/***************************************************************************

    Siemens Simatic PG-675 and PG-685

    driver skeleton by rfka01
    more skeleton by R. Belmont

****************************************************************************

The Simatic PG-685 is a programming device for Siemens' S5 line of industrial
controllers. They ran PCP/M-86 and MS-DOS specially adapted for the architecture.

http://oldcomputer.info/portables/pg685/index.htm

The portable case contains a monochrome monitor (with a socket provided to
drive an external monitor), a 5,25" floppy drive with 720KB capacity (DS,80
tracks, 9 sectors p.t., 512 Byters p.s.) and a MFM hard disk drive.
The PC is made up of several boards on a non-ISA bus backplane.

There are at least two versions and several options. The PG-685's settings are
contained in NVRAM, and have to be updated using a testdisk if the two AA
batteries run out.

For this, a key switch with a reset setting plays a crucial role. Set the key
to reset, insert disk in drive but don't close. Switch on machine, close drive
and set the switch to normal operation to start the setup.

Backplane: SCN2661B, D8253C-2, SAB 8259AP


6ES5685-OUA11

John Elliott's kindly analyzed the ROM of this machine; his findings are represented
in the preliminary memory map.

This machine only has a textmode screen, Tandon TM262 hard disk drive on a WD1010 controller,
Teac FD-55FV-13-U floppy drive on a Siemens (WD)-1797-02P controller, 768KB of RAM, HD68A45SP
display controller, upd8279c-25 keyboard controller.
Ports: Printer, V24, Module, AG-S5, Sinec H1, External Monitor

CPU/Video:      16KB BIOS/CHAR EPROM, NEC V20 CPU, SAB 8259AP, 12.288 MHz crystal, 2xHM6116LP-3,
                HD46505SP-1 (HD68A45SP), D8279C-2, D8251AFC
Module/Floppy:  2xP8255A, 4xHM6116LP-3, D8251AFC, 4.000000 MHz crystal, SAB 1797-02P, MM58167AN
HD:             4xD4016C, WD1010A-AL, 10,000000 MHz crystal
Memory:         27xTMS27C256-15, 9 empty sockets, 36 unsoldered pads


6ES5685-OUA12

This machine has the BMG (bit mapped graphics) option, that John Elliott described as a memory mapped
hercules card. There is a GEM/3 display driver that was indeed derived from the Hercules one.
The screen buffer starts at E000, the video card is at F9F0:80h, the beeper frequency at F9F0:36h,
the serial port at F9F0:38h.

Graphics screen, MiniScribe 8425 hard disk drive on a WD2010B-AL controller, Teac FD-55FR 511-U floppy drive
on a Siemens (WD)-1797-02P controller, 896KB of RAM, HD68A45SP display controller, upd8279c-25
keyboard controller
Ports: Printer, V24, Module, AG-S5, Sinec H1, External Monitor, E1

CPU/Mem.:       iR80286-10 CPU, N82C288, 19,660800 MHz crystal, 2x16KB EPROM (BIOS/CHAR), 24MHz crystal
                18.189 MHz crystal, D71059L, HD46505SP-1 (HD68A45SP), D8279C-2, N8251A, 2xSRM20256LM,
                RAM daughterbd:    4x514256-10
Module/Floppy:  2xi8255A, 4xHM6116LP-3, D8251AFC, 4.000000 MHz crystal, SAB 1797-02P, MM58167AN
HD:             SRM2064C-15, WD2010B-AL, 10,000000 MHz crystal


6ES5675-OUA11

The PG-675 shares the housing with the PG-685, but uses dual 48 tpi floppy drives instead of the harddisk/96 tpi
drive combo.

CPU/Video:      8KB BIOS/CHAR EPROM, Intel 8088 CPU, SAB 8259AP, 12,288 MHz crystal, 2xHM6116LP-3,
                HD46505SP-1 (HD68A45SP), D8279C-5, D8251AFC
Module/Floppy:  Crystal 4.000 MHz, SAB 1797-02P, 2xP8255A, MM58167AN, 4xHM6116LP-3, D8251AFC
Memory:         54x 64KBit RAM, 18 empty sockets, 9 bit and 4 bit wire straps

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i286.h"
#include "cpu/i86/i86.h"
#include "cpu/nec/nec.h"
#include "imagedev/floppy.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "machine/mm58167.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/scn_pci.h"
#include "machine/wd2010.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"
#include "screen.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pg685_state : public driver_device
{
public:
	pg685_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vram(*this, "framebuffer"),
		m_vram16(*this, "framebuffer16"),
		m_fontram(*this, "charcopy"),
		m_bppit(*this, "bppit"),
		m_fdc(*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_floppy1(*this, "fdc:1")
		{ }

	void pg685_backplane(machine_config &config);
	void pg685_module(machine_config &config);
	void pg685(machine_config &config);
	void pg675(machine_config &config);
	void pg685oua12(machine_config &config);

private:
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_UPDATE_ROW(crtc_update_row_oua12);

	uint8_t f9f04_r();
	void f9f04_w(uint8_t data);
	uint8_t f9f24_r();
	void f9f24_w(uint8_t data);
	void f9f32_w(uint8_t data);
	uint8_t f9f33_r();
	void f9f3e_w(uint8_t data);
	uint8_t f9f3f_r();
	uint8_t f9f78_r();
	void f9f78_w(uint8_t data);
	void f9f79_w(uint8_t data);
	void fdc_drq_w(int state);
	void fdc_intrq_w(int state);

	void pg675_mem(address_map &map) ATTR_COLD;
	void pg685_mem(address_map &map) ATTR_COLD;
	void pg685oua12_mem(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<uint8_t> m_vram;
	optional_shared_ptr<uint16_t> m_vram16;
	optional_shared_ptr<uint8_t> m_fontram;
	optional_device<pit8253_device> m_bppit;
	required_device<fd1797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
};

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void pg685_state::pg675_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xbffff).ram();
	map(0xf0000, 0xf1fff).ram();
	map(0xf9f00, 0xf9f01).rw("kbdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0xf9f02, 0xf9f02).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xf9f03, 0xf9f03).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xf9f04, 0xf9f04).rw(FUNC(pg685_state::f9f04_r), FUNC(pg685_state::f9f04_w));
	map(0xf9f06, 0xf9f07).rw("mainpic", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf9f08, 0xf9f09).rw("mainuart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xf9f20, 0xf9f23).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0xf9f24, 0xf9f24).rw(FUNC(pg685_state::f9f24_r), FUNC(pg685_state::f9f24_w));
	map(0xf9f28, 0xf9f2b).rw("modppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf9f2c, 0xf9f2f).rw("modppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf9f30, 0xf9f31).rw("moduart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xf9f32, 0xf9f32).w(FUNC(pg685_state::f9f32_w));
	map(0xf9f33, 0xf9f33).r(FUNC(pg685_state::f9f33_r));
	map(0xf9f40, 0xf9f5f).rw("rtc", FUNC(mm58167_device::read), FUNC(mm58167_device::write));
	map(0xfa000, 0xfa7ff).ram().share("charcopy");
	map(0xfb000, 0xfb7ff).ram().share("framebuffer");
	map(0xfc000, 0xfffff).rom().region("bios", 0);
}

void pg685_state::pg685_mem(address_map &map)
{
	map.unmap_value_high();
	pg675_mem(map);
	map(0xf9f34, 0xf9f37).rw(m_bppit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xf9f38, 0xf9f3b).rw("bpuart", FUNC(scn2661b_device::read), FUNC(scn2661b_device::write));
	map(0xf9f3c, 0xf9f3d).rw("bppic", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf9f3e, 0xf9f3e).w(FUNC(pg685_state::f9f3e_w));
	map(0xf9f70, 0xf9f77).rw("hdc", FUNC(wd2010_device::read), FUNC(wd2010_device::write));
	map(0xf9f78, 0xf9f78).rw(FUNC(pg685_state::f9f78_r), FUNC(pg685_state::f9f78_w));
	map(0xf9f79, 0xf9f79).w(FUNC(pg685_state::f9f79_w));
}

void pg685_state::pg685oua12_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xdffff).ram();
	map(0xe0000, 0xeffff).ram().share("framebuffer16");
	map(0xf0000, 0xf1fff).ram();
	map(0xf9f00, 0xf9f01).rw("kbdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0xf9f04, 0xf9f04).rw(FUNC(pg685_state::f9f04_r), FUNC(pg685_state::f9f04_w));
	map(0xf9f06, 0xf9f07).rw("mainpic", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf9f08, 0xf9f09).rw("mainuart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xf9f20, 0xf9f23).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0xf9f24, 0xf9f24).rw(FUNC(pg685_state::f9f24_r), FUNC(pg685_state::f9f24_w));
	map(0xf9f28, 0xf9f2b).rw("modppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf9f2c, 0xf9f2f).rw("modppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf9f30, 0xf9f31).rw("moduart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xf9f32, 0xf9f32).w(FUNC(pg685_state::f9f32_w));
	map(0xf9f33, 0xf9f33).r(FUNC(pg685_state::f9f33_r));
	map(0xf9f34, 0xf9f37).rw(m_bppit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xf9f38, 0xf9f3b).rw("bpuart", FUNC(scn2661b_device::read), FUNC(scn2661b_device::write));
	map(0xf9f3c, 0xf9f3d).rw("bppic", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf9f3e, 0xf9f3e).w(FUNC(pg685_state::f9f3e_w));
	map(0xf9f3f, 0xf9f3f).r(FUNC(pg685_state::f9f3f_r));
	map(0xf9f40, 0xf9f5f).rw("rtc", FUNC(mm58167_device::read), FUNC(mm58167_device::write));
	map(0xf9f70, 0xf9f77).rw("hdc", FUNC(wd2010_device::read), FUNC(wd2010_device::write));
	map(0xf9f78, 0xf9f78).rw(FUNC(pg685_state::f9f78_r), FUNC(pg685_state::f9f78_w));
	map(0xf9f79, 0xf9f79).w(FUNC(pg685_state::f9f79_w));
	map(0xf9f80, 0xf9f80).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xf9f81, 0xf9f81).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xfc000, 0xfffff).ram();    // BIOS RAM shadow
	map(0xffc000, 0xffffff).rom().region("bios", 0);
}


//**************************************************************************
//  I/O
//*************************************************************************

static INPUT_PORTS_START( pg685 )
INPUT_PORTS_END

uint8_t pg685_state::f9f04_r()
{
	// PCP/M-86 keyboard handling code also checks a couple of bits read
	logerror("Reading byte from F9F04\n");
	return 0xff;
}

void pg685_state::f9f04_w(uint8_t data)
{
	// PCP/M-86 keyboard handling code also checks a couple of bits read
	logerror("Writing %02X to F9F04\n", data);
}

void pg685_state::f9f32_w(uint8_t data)
{
	// 1D written at startup
	logerror("Writing %02X to F9F32\n", data);
}

uint8_t pg685_state::f9f33_r()
{
	// Printer present?
	logerror("Reading from F9F33\n");
	return 0xff;
}

void pg685_state::f9f3e_w(uint8_t data)
{
	m_bppit->write_gate0(BIT(data, 6));
	m_bppit->write_gate1(BIT(data, 7));

	// On PC16-11, D5 is AND-ed with the PIT's OUT2, and other bits are used to select the baud rate for a 8251.
}

uint8_t pg685_state::f9f3f_r()
{
	logerror("Reading from F9F3F\n");
	return 0xff;
}

//**************************************************************************
//  FLOPPY
//**************************************************************************

static void pg675_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

static void pg685_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}


uint8_t pg685_state::f9f24_r()
{
	logerror("Reading from F9F24\n");
	return 0xff;
}

void pg685_state::f9f24_w(uint8_t data)
{
	logerror("Writing %02X to F9F24\n", data);
}


//**************************************************************************
//  HARDDISK
//**************************************************************************

uint8_t pg685_state::f9f78_r()
{
	logerror("Reading from F9F78\n");
	return 0xff;
}

void pg685_state::f9f78_w(uint8_t data)
{
	// WD 1010 separate drive/head select register
	logerror("Writing %02X to F9F78\n", data);
}

void pg685_state::f9f79_w(uint8_t data)
{
	// another write-only register (possibly reset or interrupt control)
	logerror("Writing %02X to F9F79\n", data);
}

//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void pg685_state::machine_reset()
{
}

void pg685_state::video_start()
{
}

MC6845_UPDATE_ROW( pg685_state::crtc_update_row )
{
	static const uint32_t palette[2] = { 0x00d000, 0 };
	uint32_t  *p = &bitmap.pix(y);
	uint16_t  chr_base = ra;
	uint8_t const *const vram = (uint8_t *)m_vram.target();
	uint8_t const *const fontram = (uint8_t *)m_fontram.target();

	for ( int i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ma + i ) & 0x7ff;
		uint8_t chr = vram[ offset ];
		uint8_t data = fontram[ chr_base + chr * 16 ];
		uint8_t fg = 1;
		uint8_t bg = 0;

		*p++ = palette[( data & 0x80 ) ? fg : bg];
		*p++ = palette[( data & 0x40 ) ? fg : bg];
		*p++ = palette[( data & 0x20 ) ? fg : bg];
		*p++ = palette[( data & 0x10 ) ? fg : bg];
		*p++ = palette[( data & 0x08 ) ? fg : bg];
		*p++ = palette[( data & 0x04 ) ? fg : bg];
		*p++ = palette[( data & 0x02 ) ? fg : bg];
		*p++ = palette[( data & 0x01 ) ? fg : bg];
	}
}

MC6845_UPDATE_ROW( pg685_state::crtc_update_row_oua12 )
{
	static const uint32_t palette[2] = { 0x00d000, 0 };
	uint32_t  *p = &bitmap.pix(y);
	uint16_t  chr_base = ra;
	uint16_t const *const vram = (uint16_t *)m_vram16.target();
	uint8_t const *const fontram = (uint8_t *)memregion("chargen")->base();

	for ( int i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ma + i ) & 0x7ff;
		uint16_t chr = vram[ offset ] & 0xff;
		uint8_t data = fontram[ chr_base + chr * 16 ];
		uint8_t fg = 1;
		uint8_t bg = 0;

		*p++ = palette[( data & 0x80 ) ? fg : bg];
		*p++ = palette[( data & 0x40 ) ? fg : bg];
		*p++ = palette[( data & 0x20 ) ? fg : bg];
		*p++ = palette[( data & 0x10 ) ? fg : bg];
		*p++ = palette[( data & 0x08 ) ? fg : bg];
		*p++ = palette[( data & 0x04 ) ? fg : bg];
		*p++ = palette[( data & 0x02 ) ? fg : bg];
		*p++ = palette[( data & 0x01 ) ? fg : bg];
	}
}

//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void pg685_state::pg685_backplane(machine_config &config)
{
	PIT8253(config, m_bppit, 0);
	m_bppit->set_clk<0>(XTAL(12'288'000) / 10); // same input clock as for PC16-11?
	m_bppit->set_clk<1>(XTAL(12'288'000) / 10);
	m_bppit->set_clk<2>(XTAL(12'288'000) / 10);

	pic8259_device &bppic(PIC8259(config, "bppic", 0));
	bppic.out_int_callback().set_nop(); // configured in single 8086 mode?

	SCN2661B(config, "bpuart", 4915200);
}

void pg685_state::pg685_module(machine_config &config)
{
	FD1797(config, m_fdc, XTAL(4'000'000) / 2); // divider guessed
	m_fdc->intrq_wr_callback().set("mainpic", FUNC(pic8259_device::ir4_w));

	I8255(config, "modppi1", 0);
	I8255(config, "modppi2", 0);

	I8251(config, "moduart", XTAL(4'000'000) / 2); // divider guessed

	MM58167(config, "rtc", XTAL(32'768));
}

void pg685_state::pg675(machine_config &config)
{
	// main cpu
	I8088(config, m_maincpu, XTAL(15'000'000) / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &pg685_state::pg675_mem);
	m_maincpu->set_irq_acknowledge_callback("mainpic", FUNC(pic8259_device::inta_cb));

	pic8259_device &mainpic(PIC8259(config, "mainpic", 0));
	mainpic.out_int_callback().set_inputline(m_maincpu, 0);
	mainpic.in_sp_callback().set_constant(1);

	// i/o cpu

	// ram

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12288000, 882, 0, 720, 370, 0, 350 ); // not real values
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 12288000));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(pg685_state::crtc_update_row));

	// sound hardware

	// devices
	pg685_module(config);

	I8251(config, "mainuart", XTAL(12'288'000) / 6); // divider guessed

	// rs232 port

	// keyboard
	i8279_device &kbdc(I8279(config, "kbdc", XTAL(12'288'000) / 6)); // divider guessed
	kbdc.out_irq_callback().set("mainpic", FUNC(pic8259_device::ir0_w));

	// printer

	// floppy
	// m_fdc->intrq_wr_callback(FUNC(zorba_state::fdc_intrq_w));
	// m_fdc->drq_wr_callback(FUNC(zorba_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pg675_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pg675_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

void pg685_state::pg685(machine_config &config)
{
	// main cpu
	V20(config, m_maincpu, XTAL(15'000'000) / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &pg685_state::pg685_mem);
	m_maincpu->set_irq_acknowledge_callback("mainpic", FUNC(pic8259_device::inta_cb));

	pic8259_device &mainpic(PIC8259(config, "mainpic", 0));
	mainpic.out_int_callback().set_inputline(m_maincpu, 0);
	mainpic.in_sp_callback().set_constant(1);

	// i/o cpu

	// ram

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12288000, 882, 0, 720, 370, 0, 350 ); // not real values
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 12288000));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(pg685_state::crtc_update_row));

	// sound hardware

	// devices
	pg685_backplane(config);
	pg685_module(config);

	I8251(config, "mainuart", XTAL(12'288'000) / 6); // divider guessed

	// rs232 port

	// keyboard
	i8279_device &kbdc(I8279(config, "kbdc", XTAL(12'288'000) / 6)); // divider guessed
	kbdc.out_irq_callback().set("mainpic", FUNC(pic8259_device::ir0_w));

	// printer

	// floppy

	// m_fdc->drq_wr_callback(FUNC(zorba_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pg685_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	// harddisk
	wd2010_device& hdc(WD2010(config, "hdc", XTAL(10'000'000) / 2)); // divider guessed
	hdc.out_intrq_callback().set("mainpic", FUNC(pic8259_device::ir3_w));
}

void pg685_state::pg685oua12(machine_config &config)
{
	// main cpu
	I80286(config, m_maincpu, XTAL(20'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &pg685_state::pg685oua12_mem);
	m_maincpu->set_irq_acknowledge_callback("mainpic", FUNC(pic8259_device::inta_cb));

	pic8259_device &mainpic(PIC8259(config, "mainpic", 0));
	mainpic.out_int_callback().set_inputline(m_maincpu, 0);
	mainpic.in_sp_callback().set_constant(1);

	// i/o cpu

	// ram

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12288000, 882, 0, 720, 370, 0, 350 ); // not real values
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 12288000));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(pg685_state::crtc_update_row_oua12));

	// sound hardware

	// devices
	pg685_backplane(config);
	pg685_module(config);

	I8251(config, "mainuart", 12288000 / 6); // wrong

	// rs232 port

	// keyboard
	i8279_device &kbdc(I8279(config, "kbdc", 12288000 / 6)); // wrong
	kbdc.out_irq_callback().set("mainpic", FUNC(pic8259_device::ir0_w));

	// printer

	// floppy
	// m_fdc->drq_wr_callback(FUNC(zorba_state::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pg685_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	// harddisk
	wd2010_device& hdc(WD2010(config, "hdc", XTAL(10'000'000) / 2)); // divider guessed
	hdc.out_intrq_callback().set("mainpic", FUNC(pic8259_device::ir3_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( pg675 )
	ROM_REGION( 0x4000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD( "p79004-a7021 a2-a1.bin", 0x2000, 0x2000, CRC(c7602d28) SHA1(a470e0457cc83f989995cfbca1ebce0878a3c4e3) )
ROM_END

ROM_START( pg685 )
	ROM_REGION( 0x4000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD( "pg685_oua11_s79200-g2_a901-03.bin", 0x0000, 0x4000, CRC(db13f2db) SHA1(5f65ab14d9c8acdcc5482b27e727ca43b1a7daf3) )
ROM_END

ROM_START( pg685oua12 )
	ROM_REGION16_LE( 0x4000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD( "pg685_oua12_bios.bin", 0x0000, 0x4000, CRC(94b8499b) SHA1(e29086a88f1f9fa17921c3d157cce725d4591328))

	ROM_REGION( 0x4000, "chargen", 0 )
	ROM_LOAD( "pg685_oua12_s79200-g39_a901-01.bin", 0x0000, 0x4000, CRC(fa722110) SHA1(b57ee67a77ff45a2544a2ae5203bc2199adfe023))
ROM_END

} // anonymous namespace


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************
//    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT  CLASS        INIT        COMPANY    FULLNAME               FLAGS
COMP( 198?, pg675,      0,      0,      pg675,      pg685, pg685_state, empty_init, "Siemens", "Simatic PG675",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 198?, pg685,      0,      0,      pg685,      pg685, pg685_state, empty_init, "Siemens", "Simatic PG685 OUA11", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 198?, pg685oua12, pg685,  0,      pg685oua12, pg685, pg685_state, empty_init, "Siemens", "Simatic PG685 OUA12", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
