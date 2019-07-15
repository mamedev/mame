// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    drivers/gridcomp.cpp

    Driver file for GRiD Compass series

    US patent 4,571,456 describes model 1101:

    - 15 MHz XTAL, produces
        - 5 MHz system clock for CPU, FPU, OSP
        - 7.5 MHz pixel clock
    - Intel 8086 - CPU
    - Intel 8087 - FPU
    - Intel 80130 - Operating System Processor, equivalent of:
        - 8259 PIC
        - 8254 PIT
    - Texas Instruments TMS9914 GPIB controller
    - Intel 7220 Bubble Memory Controller
        - 7110 Magnetic Bubble Memory modules and support chips
    - (unknown) - EAROM for machine ID
    - (unknown) - Real-Time Clock
    - (custom DMA logic)
    - Intel 8741 - keyboard MCU
    - Intel 8274 - UART
    - Intel 8255 - modem interface
        - 2x DAC0832LCN - DAC
        - MK5089N - DTMF generator
        - ...

    to do:

    - confirm differences between models except screen size
        - Compass 110x do not have GRiDROM slots.
        - Compass II (112x, 113x) have 4 of them.
    - keyboard: decode and add the rest of keycodes
    - EAROM, RTC
    - serial port, modem (incl. DTMF generator)
    - TMS9914 chip driver (incl. DMA)
    - GPIB storage devices (floppy, hard disk)

    missing dumps:

    - BIOS from models other than 1139 (CCOS and MS-DOS variants)
    - GRiDROM's
    - keyboard MCU
    - external floppy and hard disk (2101, 2102)

    to boot CCOS 3.0.1:
    - pad binary image (not .imd) to 384K
    - attach it as -memcard
    - use grid1129 with 'patched' ROM
    - start with -debug and add breakpoints:

    # bubble memory driver
    bp ff27a,1,{ax=ax*2;go}
    # boot loader
    bp 20618,1,{temp0=214A8;do w@(temp0+7)=120;do w@(temp0+9)=121;go}
    # CCOS kernel
    bp 0661a,1,{temp0=0f964;do w@(temp0+7)=120;do w@(temp0+9)=121;go}

***************************************************************************/

#include "emu.h"

#include "bus/ieee488/ieee488.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "machine/gridkeyb.h"
#include "machine/i7220.h"
#include "machine/i80130.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "machine/tms9914.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


//#define LOG_GENERAL (1U <<  0) //defined in logmacro.h already
#define LOG_KEYBOARD  (1U <<  1)
#define LOG_DEBUG     (1U <<  2)

//#define VERBOSE (LOG_DEBUG)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGKBD(...) LOGMASKED(LOG_KEYBOARD, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


#define I80130_TAG      "osp"

class gridcomp_state : public driver_device
{
public:
	gridcomp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_osp(*this, I80130_TAG)
		, m_modem(*this, "modem")
		, m_uart8274(*this, "uart8274")
		, m_speaker(*this, "speaker")
		, m_ram(*this, RAM_TAG)
	{ }

	void grid1129(machine_config &config);
	void grid1131(machine_config &config);
	void grid1121(machine_config &config);
	void grid1139(machine_config &config);
	void grid1109(machine_config &config);
	void grid1101(machine_config &config);

	void init_gridcomp();

private:
	required_device<cpu_device> m_maincpu;
	required_device<i80130_device> m_osp;
	required_device<i8255_device> m_modem;
	optional_device<i8274_new_device> m_uart8274;
	required_device<speaker_sound_device> m_speaker;
	required_device<ram_device> m_ram;

	DECLARE_MACHINE_START(gridcomp);
	DECLARE_MACHINE_RESET(gridcomp);

	IRQ_CALLBACK_MEMBER(irq_callback);

	DECLARE_READ16_MEMBER(grid_9ff0_r);
	DECLARE_READ16_MEMBER(grid_keyb_r);
	DECLARE_READ16_MEMBER(grid_gpib_r);
	DECLARE_WRITE16_MEMBER(grid_keyb_w);
	DECLARE_WRITE16_MEMBER(grid_gpib_w);

	uint32_t screen_update_110x(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_113x(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_generic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int px);

	void kbd_put(u16 data);

	void grid1101_io(address_map &map);
	void grid1101_map(address_map &map);
	void grid1121_map(address_map &map);

	bool m_kbd_ready;
	uint16_t m_kbd_data;

	uint16_t *m_videoram;
};


READ16_MEMBER(gridcomp_state::grid_9ff0_r)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = 0xbb66;
		break;
	}

	LOGDBG("9FF0: %02x == %02x\n", 0x9ff00 + (offset << 1), data);

	return data;
}

READ16_MEMBER(gridcomp_state::grid_keyb_r)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_kbd_data;
		m_kbd_data = 0xff;
		m_kbd_ready = false;
		m_osp->ir4_w(CLEAR_LINE);
		break;

	case 1:
		data = m_kbd_ready ? 2 : 0;
		break;
	}

	LOGKBD("%02x == %02x\n", 0xdffc0 + (offset << 1), data);

	return data;
}

WRITE16_MEMBER(gridcomp_state::grid_keyb_w)
{
	LOGKBD("%02x <- %02x\n", 0xdffc0 + (offset << 1), data);
}

void gridcomp_state::kbd_put(u16 data)
{
	m_kbd_data = data;
	m_kbd_ready = true;
	m_osp->ir4_w(ASSERT_LINE);
}


// reject all commands
READ16_MEMBER(gridcomp_state::grid_gpib_r)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = 0x10; // BO
		break;

	case 1:
		data = 0x40; // ERR
		m_osp->ir5_w(CLEAR_LINE);
		break;
	}

	LOG("GPIB %02x == %02x\n", 0xdff80 + (offset << 1), data);

	return data;
}

WRITE16_MEMBER(gridcomp_state::grid_gpib_w)
{
	switch (offset)
	{
	case 7:
		m_osp->ir5_w(ASSERT_LINE);
		break;
	}

	LOG("GPIB %02x <- %02x\n", 0xdff80 + (offset << 1), data);
}


uint32_t gridcomp_state::screen_update_generic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int px)
{
	int x, y, offset;
	uint16_t gfx, *p;

	for (y = 0; y < 240; y++)
	{
		p = &bitmap.pix16(y);

		offset = y * (px / 16);

		for (x = offset; x < offset + px / 16; x++)
		{
			gfx = m_videoram[x];

			for (int i = 15; i >= 0; i--)
			{
				*p++ = BIT(gfx, i);
			}
		}
	}

	return 0;
}

uint32_t gridcomp_state::screen_update_110x(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return screen_update_generic(screen, bitmap, cliprect, 320);
}

uint32_t gridcomp_state::screen_update_113x(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return screen_update_generic(screen, bitmap, cliprect, 512);
}


void gridcomp_state::init_gridcomp()
{
}

MACHINE_START_MEMBER(gridcomp_state, gridcomp)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_readwrite_bank(0, m_ram->size() - 1, "bank10");
	membank("bank10")->set_base(m_ram->pointer());

	m_videoram = (uint16_t *)m_maincpu->space(AS_PROGRAM).get_write_ptr(0x400);
}

MACHINE_RESET_MEMBER(gridcomp_state, gridcomp)
{
	m_kbd_ready = false;
}

IRQ_CALLBACK_MEMBER(gridcomp_state::irq_callback)
{
	return m_osp->inta_r();
}


void gridcomp_state::grid1101_map(address_map &map)
{
	map.unmap_value_high();
	map(0xdfe80, 0xdfe83).rw("i7220", FUNC(i7220_device::read), FUNC(i7220_device::write)).umask16(0x00ff);
	map(0xdfea0, 0xdfeaf).unmaprw(); // ??
	map(0xdfec0, 0xdfecf).rw(m_modem, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff); // incl. DTMF generator
	map(0xdff40, 0xdff5f).noprw();   // ?? machine ID EAROM, RTC
	map(0xdff80, 0xdff8f).rw("hpib", FUNC(tms9914_device::read), FUNC(tms9914_device::write)).umask16(0x00ff);
	map(0xdffc0, 0xdffcf).rw(FUNC(gridcomp_state::grid_keyb_r), FUNC(gridcomp_state::grid_keyb_w)); // Intel 8741 MCU
	map(0xfc000, 0xfffff).rom().region("user1", 0);
}

void gridcomp_state::grid1121_map(address_map &map)
{
	map.unmap_value_high();
	map(0x90000, 0x97fff).unmaprw(); // ?? ROM slot
	map(0x9ff00, 0x9ff0f).unmaprw(); // AM_READ(grid_9ff0_r) // ?? ROM?
	map(0xc0000, 0xcffff).unmaprw(); // ?? ROM slot -- signature expected: 0x4554, 0x5048
	map(0xdfe00, 0xdfe1f).unmaprw(); // AM_DEVREADWRITE8("uart8274", i8274_new_device, ba_cd_r, ba_cd_w, 0x00ff)
	map(0xdfe40, 0xdfe4f).unmaprw(); // ?? diagnostic 8274
	map(0xdfe80, 0xdfe83).rw("i7220", FUNC(i7220_device::read), FUNC(i7220_device::write)).umask16(0x00ff);
	map(0xdfea0, 0xdfeaf).unmaprw(); // ??
	map(0xdfec0, 0xdfecf).rw(m_modem, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff); // incl. DTMF generator
	map(0xdff40, 0xdff5f).noprw();   // ?? machine ID EAROM, RTC
	map(0xdff80, 0xdff8f).rw("hpib", FUNC(tms9914_device::read), FUNC(tms9914_device::write)).umask16(0x00ff);
	map(0xdffc0, 0xdffcf).rw(FUNC(gridcomp_state::grid_keyb_r), FUNC(gridcomp_state::grid_keyb_w)); // Intel 8741 MCU
	map(0xfc000, 0xfffff).rom().region("user1", 0);
}

void gridcomp_state::grid1101_io(address_map &map)
{
	map(0x0000, 0x000f).m(m_osp, FUNC(i80130_device::io_map));
}

static INPUT_PORTS_START( gridcomp )
INPUT_PORTS_END

/*
 * IRQ0 serial
 * IRQ1 bubble
 * IRQ2 modem
 * IRQ3 system tick || vert sync
 * IRQ4 keyboard
 * IRQ5 gpib
 * IRQ6 8087
 * IRQ7 ring
 */
void gridcomp_state::grid1101(machine_config &config)
{
	I8086(config, m_maincpu, XTAL(15'000'000) / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &gridcomp_state::grid1101_map);
	m_maincpu->set_addrmap(AS_IO, &gridcomp_state::grid1101_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(gridcomp_state::irq_callback));

	MCFG_MACHINE_START_OVERRIDE(gridcomp_state, gridcomp)
	MCFG_MACHINE_RESET_OVERRIDE(gridcomp_state, gridcomp)

	I80130(config, m_osp, XTAL(15'000'000)/3);
	m_osp->irq().set_inputline("maincpu", 0);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD)); // actually a kind of EL display
	screen.set_color(rgb_t::amber());
	screen.set_screen_update(FUNC(gridcomp_state::screen_update_110x));
	screen.set_raw(XTAL(15'000'000)/2, 424, 0, 320, 262, 0, 240); // XXX 66 Hz refresh
	screen.screen_vblank().set(m_osp, FUNC(i80130_device::ir3_w));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	grid_keyboard_device &keyboard(GRID_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(gridcomp_state::kbd_put));

	i7220_device &i7220(I7220(config, "i7220", XTAL(4'000'000)));
	i7220.set_data_size(3); // 3 1-Mbit MBM's
	i7220.irq_callback().set(I80130_TAG, FUNC(i80130_device::ir1_w));
	i7220.drq_callback().set(I80130_TAG, FUNC(i80130_device::ir1_w));

	tms9914_device &hpib(TMS9914(config, "hpib", XTAL(4'000'000)));
	hpib.int_write_cb().set(I80130_TAG, FUNC(i80130_device::ir5_w));
	hpib.dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	hpib.dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	hpib.eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	hpib.dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	hpib.nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	hpib.ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	hpib.ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	hpib.srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	hpib.atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	hpib.ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));

	ieee488_device &ieee(IEEE488(config, IEEE488_TAG));
	ieee.eoi_callback().set("hpib", FUNC(tms9914_device::eoi_w));
	ieee.dav_callback().set("hpib", FUNC(tms9914_device::dav_w));
	ieee.nrfd_callback().set("hpib", FUNC(tms9914_device::nrfd_w));
	ieee.ndac_callback().set("hpib", FUNC(tms9914_device::ndac_w));
	ieee.ifc_callback().set("hpib", FUNC(tms9914_device::ifc_w));
	ieee.srq_callback().set("hpib", FUNC(tms9914_device::srq_w));
	ieee.atn_callback().set("hpib", FUNC(tms9914_device::atn_w));
	ieee.ren_callback().set("hpib", FUNC(tms9914_device::ren_w));
	IEEE488_SLOT(config, "ieee_rem", 0, remote488_devices, nullptr);

	I8274_NEW(config, m_uart8274, XTAL(4'032'000));

	I8255(config, "modem", 0);

	RAM(config, m_ram).set_default_size("256K").set_default_value(0);
}

void gridcomp_state::grid1109(machine_config &config)
{
	grid1101(config);
	m_ram->set_default_size("512K");
}

void gridcomp_state::grid1121(machine_config &config)
{
	grid1101(config);
	m_maincpu->set_clock(XTAL(24'000'000) / 3); // XXX
	m_maincpu->set_addrmap(AS_PROGRAM, &gridcomp_state::grid1121_map);
}

void gridcomp_state::grid1129(machine_config &config)
{
	grid1121(config);
	m_ram->set_default_size("512K");
}

void gridcomp_state::grid1131(machine_config &config)
{
	grid1121(config);
	subdevice<screen_device>("screen")->set_screen_update(FUNC(gridcomp_state::screen_update_113x));
	subdevice<screen_device>("screen")->set_raw(XTAL(15'000'000)/2, 720, 0, 512, 262, 0, 240); // XXX
}

void gridcomp_state::grid1139(machine_config &config)
{
	grid1131(config);
	m_ram->set_default_size("512K");
}


ROM_START( grid1101 )
	ROM_REGION16_LE(0x10000, "user1", 0)

	ROM_SYSTEM_BIOS(0, "ccos", "ccos bios")
	ROMX_LOAD("1101even.bin", 0x0000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("1101odd.bin",  0x0001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

ROM_START( grid1109 )
	ROM_REGION16_LE(0x10000, "user1", 0)

	ROM_SYSTEM_BIOS(0, "ccos", "ccos bios")
	ROMX_LOAD("1109even.bin", 0x0000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("1109odd.bin",  0x0001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

ROM_START( grid1121 )
	ROM_REGION16_LE(0x10000, "user1", 0)

	ROM_SYSTEM_BIOS(0, "ccos", "ccos bios")
	ROMX_LOAD("1121even.bin", 0x0000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("1121odd.bin",  0x0001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

ROM_START( grid1129 )
	ROM_REGION16_LE(0x10000, "user1", 0)
	ROM_DEFAULT_BIOS("patched")

	ROM_SYSTEM_BIOS(0, "ccos", "ccos bios")
	ROMX_LOAD("1129even.bin", 0x0000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("1129odd.bin",  0x0001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "patched", "patched 1139 bios")
	ROMX_LOAD("1139even.bin", 0x0000, 0x2000, CRC(67071849) SHA1(782239c155fa5821f8dbd2607cee9152d175e90e), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("1139odd.bin",  0x0001, 0x2000, CRC(13ed4bf0) SHA1(f7087f86dbbc911bee985125bccd2417e0374e8e), ROM_SKIP(1) | ROM_BIOS(1))

	// change bubble driver setup to read floppy images with 512-byte sectors
	ROM_FILL(0x3114,1,0x00)
	ROM_FILL(0x3115,1,0x02)
	ROM_FILL(0x3116,1,0xf8)
	ROM_FILL(0x3117,1,0x01)

	// move work area from 0440h:XXXX to 0298h:XXXX
	ROM_FILL(0x23,1,0x98)
	ROM_FILL(0x24,1,0x2)
	ROM_FILL(0xbc,1,0x98)
	ROM_FILL(0xbd,1,0x2)
	ROM_FILL(0x14e,1,0xc1)  //
	ROM_FILL(0x14f,1,0x2)   //
	ROM_FILL(0x15a,1,0xc2)  //
	ROM_FILL(0x15b,1,0x2)   //
	ROM_FILL(0x17b,1,0x45)  //
	ROM_FILL(0x17c,1,0x3)   //
	ROM_FILL(0x28c,1,0x98)
	ROM_FILL(0x28d,1,0x2)
	ROM_FILL(0x28f,1,0x98)
	ROM_FILL(0x290,1,0x2)
	ROM_FILL(0x2b9,1,0x98)
	ROM_FILL(0x2ba,1,0x2)
	ROM_FILL(0x2d0,1,0x98)
	ROM_FILL(0x2d1,1,0x2)
	ROM_FILL(0x31a,1,0x98)
	ROM_FILL(0x31b,1,0x2)
	ROM_FILL(0x3a0,1,0x98)
	ROM_FILL(0x3a1,1,0x2)
	ROM_FILL(0x3a3,1,0x98)
	ROM_FILL(0x3a4,1,0x2)
	ROM_FILL(0x3e2,1,0x98)
	ROM_FILL(0x3e3,1,0x2)
	ROM_FILL(0x43e,1,0x98)
	ROM_FILL(0x43f,1,0x2)
	ROM_FILL(0x46d,1,0x98)
	ROM_FILL(0x46e,1,0x2)
	ROM_FILL(0x4fe,1,0x98)
	ROM_FILL(0x4ff,1,0x2)
	ROM_FILL(0x512,1,0x98)
	ROM_FILL(0x513,1,0x2)
	ROM_FILL(0x768,1,0x98)
	ROM_FILL(0x769,1,0x2)
	ROM_FILL(0x79e,1,0x98)
	ROM_FILL(0x79f,1,0x2)
	ROM_FILL(0x7f5,1,0x98)
	ROM_FILL(0x7f6,1,0x2)
	ROM_FILL(0x92a,1,0x98)
	ROM_FILL(0x92b,1,0x2)
	ROM_FILL(0xe50,1,0x98)
	ROM_FILL(0xe51,1,0x2)
	ROM_FILL(0xfa6,1,0x98)
	ROM_FILL(0xfa7,1,0x2)
	ROM_FILL(0x15fe,1,0xce) //
	ROM_FILL(0x15ff,1,0x2)  //
	ROM_FILL(0x1628,1,0xd0) //
	ROM_FILL(0x1629,1,0x2)  //
	ROM_FILL(0x1700,1,0x98)
	ROM_FILL(0x1701,1,0x2)
	ROM_FILL(0x1833,1,0xd6) //
	ROM_FILL(0x1834,1,0x2)  //
	ROM_FILL(0x184a,1,0xd6) //
	ROM_FILL(0x184b,1,0x2)  //
	ROM_FILL(0x1a2e,1,0xd6) //
	ROM_FILL(0x1a2f,1,0x2)  //
	ROM_FILL(0x19c2,1,0x98)
	ROM_FILL(0x19c3,1,0x2)
	ROM_FILL(0x1ee0,1,0x98)
	ROM_FILL(0x1ee1,1,0x2)
	ROM_FILL(0x1f1d,1,0x98)
	ROM_FILL(0x1f1e,1,0x2)
	ROM_FILL(0x1f40,1,0x98)
	ROM_FILL(0x1f41,1,0x2)
	ROM_FILL(0x2253,1,0x98)
	ROM_FILL(0x2254,1,0x2)
	ROM_FILL(0x2437,1,0x98)
	ROM_FILL(0x2438,1,0x2)
	ROM_FILL(0x283a,1,0x98)
	ROM_FILL(0x283b,1,0x2)
	ROM_FILL(0x2868,1,0x98)
	ROM_FILL(0x2869,1,0x2)
	ROM_FILL(0x288f,1,0x98)
	ROM_FILL(0x2890,1,0x2)
	ROM_FILL(0x2942,1,0x98)
	ROM_FILL(0x2943,1,0x2)
	ROM_FILL(0x295c,1,0x98)
	ROM_FILL(0x295d,1,0x2)
	ROM_FILL(0x2a5e,1,0x98)
	ROM_FILL(0x2a5f,1,0x2)
	ROM_FILL(0x315c,1,0xc9) //
	ROM_FILL(0x315d,1,0x2)  //
	ROM_FILL(0x3160,1,0xce) //
	ROM_FILL(0x3161,1,0x2)  //
	ROM_FILL(0x3164,1,0xcf) //
	ROM_FILL(0x3165,1,0x2)  //
ROM_END

ROM_START( grid1131 )
	ROM_REGION16_LE(0x10000, "user1", 0)

	ROM_SYSTEM_BIOS(0, "ccos", "ccos bios")
	ROMX_LOAD("1131even.bin", 0x0000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("1131odd.bin",  0x0001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

ROM_START( grid1139 )
	ROM_REGION16_LE(0x10000, "user1", 0)

	ROM_SYSTEM_BIOS(0, "normal", "normal bios")
	ROMX_LOAD("1139even.bin", 0x0000, 0x2000, CRC(67071849) SHA1(782239c155fa5821f8dbd2607cee9152d175e90e), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("1139odd.bin",  0x0001, 0x2000, CRC(13ed4bf0) SHA1(f7087f86dbbc911bee985125bccd2417e0374e8e), ROM_SKIP(1) | ROM_BIOS(0))
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY           FULLNAME           FLAGS
COMP( 1982, grid1101, 0,        0,      grid1101, gridcomp, gridcomp_state, empty_init, "GRiD Computers", "Compass 1101",    MACHINE_IS_SKELETON )
COMP( 1982, grid1109, grid1101, 0,      grid1109, gridcomp, gridcomp_state, empty_init, "GRiD Computers", "Compass 1109",    MACHINE_IS_SKELETON )
COMP( 1984, grid1121, 0,        0,      grid1121, gridcomp, gridcomp_state, empty_init, "GRiD Computers", "Compass II 1121", MACHINE_IS_SKELETON )
COMP( 1984, grid1129, grid1121, 0,      grid1129, gridcomp, gridcomp_state, empty_init, "GRiD Computers", "Compass II 1129", MACHINE_IS_SKELETON )
COMP( 1984, grid1131, grid1121, 0,      grid1131, gridcomp, gridcomp_state, empty_init, "GRiD Computers", "Compass II 1131", MACHINE_IS_SKELETON )
COMP( 1984, grid1139, grid1121, 0,      grid1139, gridcomp, gridcomp_state, empty_init, "GRiD Computers", "Compass II 1139", MACHINE_IS_SKELETON )
