// license:BSD-3-Clause
// copyright-holders:Robbbert, R. Belmont
/***************************************************************************

    Peripheral Technology PT68K2/PT68K4 family

    2011-01-03 Skeleton driver.
    2013-09-30 Connected to a terminal
    2014-01-03 Connect real DUARTs, FDC, and TimeKeeper.  Settings now save properly, floppies can be read.
    2014-01-19 ISA bus and compatible cards, PC keyboard support, speaker support
    2014-09-20 Add PT68K2, add save states, we have a working SK*DOS disk!

This has the appearance of a PC, including pc power supply, slots, etc
on a conventional pc-like motherboard and case.

Some pics: http://www.wormfood.net/old_computers/

Source code and manuals for the HUMBUG BIOS and SK*DOS are at:
http://www.users.cloud9.net/~stark/sources.html

Usage:
    Start up and press Enter as prompted.  Type he to see a command list, or fd to boot from the
    first floppy drive.

    The stock NVRAM configures PT68k2 for 2 DSDD 5.25" drives, and PT68k4 for 2 DSHD 5.25" drives.

Chips:
    68230 Parallel Interface/Timer @ FE0081
    68681 DUART/Timer (x2) @ FE0001 and FE0041
    WD37C65 FDC (PC FDC compatible, even mapped as an ISA device)
    MK48T02 TimeKeeper @ odd bytes from FF0001 to FF0FFF.  even bytes in that range are a standard SRAM chip which is not backed up.
    Keyboard at FE01C1 (status/IRQ clear)/FE01C3 (AT scan codes)
    WD1002 HDD controller @ FE0141-FE014F.  "Monk" BIOS also supports an 8-bit ISA IDE card.

Video: ISA MDA or CGA/EGA/VGA-style boards
    ISA memory is C00001-DFFFFF odd bytes only.  So the MDA B0000 framebuffer becames (B0000*2) + C00001 = D60001.
    ISA I/O is at FA0001-FBFFFF odd bytes only, and the mapping is similar.

    HUMBUG BIOS tests MDA and CGA VRAM to determine existence, falls back to serial console if neither exists.  If both exist, MDA is used.
    VRAM is every other byte for ISA cards.  (Only 8 bit cards are supported).

    OP3 on DUART1 drives a speaker.
    IP2 on DUART1 signals if a new keyboard scan code is available.

IRQs:
    2: 68230 PIT
    4: DUART2
    5: DUART1
    6: PC FDC IRQ

TODO: 68230 device
      This system and SK*DOS don't like our ISA WDXT-GEN emulation so HDD installs are not currently possible.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "machine/timekpr.h"
#include "machine/wd_fdc.h"
#include "machine/pc_fdc.h"
#include "formats/imd_dsk.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"
#include "sound/speaker.h"
#include "softlist.h"

#define M68K_TAG "maincpu"
#define DUART1_TAG  "duart1"
#define DUART2_TAG  "duart2"
#define TIMEKEEPER_TAG  "timekpr"
#define ISABUS_TAG "isa"
#define KBDC_TAG "pc_kbdc"
#define SPEAKER_TAG "speaker"
#define WDFDC_TAG   "wdfdc"

class pt68k4_state : public driver_device
{
public:
	pt68k4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_base(*this, "rambase")
		, m_maincpu(*this, M68K_TAG)
		, m_duart1(*this, DUART1_TAG)
		, m_duart2(*this, DUART2_TAG)
		, m_isa(*this, ISABUS_TAG)
		, m_speaker(*this, SPEAKER_TAG)
		, m_wdfdc(*this, WDFDC_TAG)
	{ }

	DECLARE_READ8_MEMBER(hiram_r);
	DECLARE_WRITE8_MEMBER(hiram_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(keyboard_w);

	DECLARE_READ8_MEMBER(pia_stub_r);
	DECLARE_WRITE8_MEMBER(duart1_out);

	DECLARE_WRITE8_MEMBER(fdc_select_w);

	DECLARE_WRITE_LINE_MEMBER(duart1_irq);
	DECLARE_WRITE_LINE_MEMBER(duart2_irq);

	DECLARE_WRITE_LINE_MEMBER(irq5_w);

	DECLARE_WRITE_LINE_MEMBER(keyboard_clock_w);
	DECLARE_WRITE_LINE_MEMBER(keyboard_data_w);

	DECLARE_FLOPPY_FORMATS( floppy_formats );

private:
	virtual void machine_start();
	virtual void machine_reset();
	required_shared_ptr<UINT16> m_p_base;
	required_device<cpu_device> m_maincpu;
	required_device<mc68681_device> m_duart1;
	required_device<mc68681_device> m_duart2;
	required_device<isa8_device> m_isa;
	required_device<speaker_sound_device> m_speaker;
	optional_device<wd1772_t> m_wdfdc;

	void irq5_update();

	UINT8 m_hiram[0x800];

	bool m_kclk;
	UINT8 m_kdata;
	UINT8 m_scancode;
	UINT8 m_kbdflag;
	int m_kbit;
	int m_lastdrive;
	bool m_irq5_duart1, m_irq5_isa;
};

FLOPPY_FORMATS_MEMBER( pt68k4_state::floppy_formats )
	FLOPPY_IMD_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( pt68k_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

// XT keyboard interface - done in TTL instead of an 804x
WRITE_LINE_MEMBER(pt68k4_state::keyboard_clock_w)
{
//  printf("KCLK: %d\n", state ? 1 : 0);

	// rising edge?
	if ((state == ASSERT_LINE) && (!m_kclk))
	{
		if (m_kbit >= 1 && m_kbit <= 8)
		{
			m_scancode >>= 1;
			m_scancode |= m_kdata;
		}

		// stop bit?
		if (m_kbit == 9)
		{
//          printf("scancode %02x\n", m_scancode);
			m_kbit = 0;
			m_kbdflag = 0x80;
			m_duart1->ip2_w(CLEAR_LINE);
		}
		else
		{
			m_kbit++;
		}
	}

	m_kclk = (state == ASSERT_LINE) ? true : false;
}

WRITE_LINE_MEMBER(pt68k4_state::keyboard_data_w)
{
//  printf("KDATA: %d\n", state ? 1 : 0);
	m_kdata = (state == ASSERT_LINE) ? 0x80 : 0x00;
}

WRITE8_MEMBER(pt68k4_state::duart1_out)
{
	m_speaker->level_w((data >> 3) & 1);
}

READ8_MEMBER(pt68k4_state::pia_stub_r)
{
	return 0;
}

WRITE8_MEMBER(pt68k4_state::fdc_select_w)
{
	floppy_connector *con = machine().device<floppy_connector>(WDFDC_TAG":0");
	floppy_connector *con2 = machine().device<floppy_connector>(WDFDC_TAG":1");
	floppy_image_device *floppy = con ? con->get_device() : nullptr;
	floppy_image_device *floppy2 = con2 ? con2->get_device() : nullptr;
	int drive = data & 3;

	if (drive != m_lastdrive)
	{
		switch (drive)
		{
			case 0:
				m_wdfdc->set_floppy(floppy);
				break;

			case 1:
				m_wdfdc->set_floppy(floppy2);
				break;

			default:
				m_wdfdc->set_floppy(nullptr);
				break;
		}

		m_lastdrive = drive;
	}

	switch (drive)
	{
		case 0:
			floppy->ss_w((data & 0x40) ? 1 : 0);
			break;

		case 1:
			floppy2->ss_w((data & 0x40) ? 1 : 0);
			break;

		default:
			break;
	}
}

static ADDRESS_MAP_START(pt68k2_mem, AS_PROGRAM, 16, pt68k4_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x0fffff) AM_RAM AM_SHARE("rambase") // 1MB RAM
	AM_RANGE(0xf80000, 0xf8ffff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0xc00000, 0xdfffff) AM_DEVREADWRITE8(ISABUS_TAG, isa8_device, prog_r, prog_w, 0x00ff)
	AM_RANGE(0xfa0000, 0xfbffff) AM_DEVREADWRITE8(ISABUS_TAG, isa8_device, io_r, io_w, 0x00ff)
	AM_RANGE(0xfe0000, 0xfe001f) AM_DEVREADWRITE8(DUART1_TAG, mc68681_device, read, write, 0x00ff)
	AM_RANGE(0xfe0040, 0xfe005f) AM_DEVREADWRITE8(DUART2_TAG, mc68681_device, read, write, 0x00ff)
	AM_RANGE(0xfe0080, 0xfe00bf) AM_READ8(pia_stub_r, 0x00ff)
	AM_RANGE(0xfe00c0, 0xfe00ff) AM_WRITE8(fdc_select_w, 0x00ff)
	AM_RANGE(0xfe0100, 0xfe013f) AM_DEVREADWRITE8(WDFDC_TAG, wd1772_t, read, write, 0x00ff)
	AM_RANGE(0xfe01c0, 0xfe01c3) AM_READWRITE8(keyboard_r, keyboard_w, 0x00ff)
	AM_RANGE(0xff0000, 0xff0fff) AM_READWRITE8(hiram_r, hiram_w, 0xff00)
	AM_RANGE(0xff0000, 0xff0fff) AM_DEVREADWRITE8(TIMEKEEPER_TAG, timekeeper_device, read, write, 0x00ff)
ADDRESS_MAP_END

static ADDRESS_MAP_START(pt68k4_mem, AS_PROGRAM, 16, pt68k4_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x0fffff) AM_RAM AM_SHARE("rambase") // 1MB RAM (OS9 needs more)
	AM_RANGE(0xf80000, 0xf8ffff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0xc00000, 0xdfffff) AM_DEVREADWRITE8(ISABUS_TAG, isa8_device, prog_r, prog_w, 0x00ff)
	AM_RANGE(0xfa0000, 0xfbffff) AM_DEVREADWRITE8(ISABUS_TAG, isa8_device, io_r, io_w, 0x00ff)
	AM_RANGE(0xfe0000, 0xfe001f) AM_DEVREADWRITE8(DUART1_TAG, mc68681_device, read, write, 0x00ff)
	AM_RANGE(0xfe0040, 0xfe005f) AM_DEVREADWRITE8(DUART2_TAG, mc68681_device, read, write, 0x00ff)
	AM_RANGE(0xfe01c0, 0xfe01c3) AM_READWRITE8(keyboard_r, keyboard_w, 0x00ff)
	AM_RANGE(0xff0000, 0xff0fff) AM_READWRITE8(hiram_r, hiram_w, 0xff00)
	AM_RANGE(0xff0000, 0xff0fff) AM_DEVREADWRITE8(TIMEKEEPER_TAG, timekeeper_device, read, write, 0x00ff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pt68k4 )
INPUT_PORTS_END

/* built in keyboard: offset 0 reads 0x80 if key ready, 0 if not.  If key ready, offset 1 reads scancode.  Read or write to offs 0 clears key ready */
READ8_MEMBER(pt68k4_state::keyboard_r)
{
	if (offset == 0)
	{
		UINT8 rv = m_kbdflag;

		m_kbdflag = 0;
		m_duart1->ip2_w(ASSERT_LINE);

		return rv;
	}

	return m_scancode;
}

WRITE8_MEMBER(pt68k4_state::keyboard_w)
{
	m_kbdflag = 0;
	m_duart1->ip2_w(ASSERT_LINE);
}

READ8_MEMBER(pt68k4_state::hiram_r)
{
	return m_hiram[offset];
}

WRITE8_MEMBER(pt68k4_state::hiram_w)
{
	m_hiram[offset] = data;
}

void pt68k4_state::machine_start()
{
	save_item(NAME(m_hiram));
	save_item(NAME(m_kclk));
	save_item(NAME(m_kdata));
	save_item(NAME(m_scancode));
	save_item(NAME(m_kbdflag));
	save_item(NAME(m_kbit));
	save_item(NAME(m_lastdrive));
	save_item(NAME(m_irq5_duart1));
	save_item(NAME(m_irq5_isa));
}

void pt68k4_state::machine_reset()
{
	UINT8* user1 = memregion("roms")->base();
	memcpy((UINT8*)m_p_base.target(), user1, 8);

	m_maincpu->reset();

	m_kclk = true;
	m_kbit = 0;
	m_scancode = 0;
	m_kbdflag = 0;
	m_irq5_duart1 = CLEAR_LINE;
	m_irq5_isa = CLEAR_LINE;

	// set line to asserted (no key code ready)
	m_duart1->ip2_w(ASSERT_LINE);

	if (m_wdfdc)
	{
		floppy_connector *con = machine().device<floppy_connector>(WDFDC_TAG":0");
		floppy_image_device *floppy = con ? con->get_device() : nullptr;

		m_wdfdc->set_floppy(floppy);
		floppy->ss_w(0);

		m_lastdrive = 0;
	}
}

void pt68k4_state::irq5_update()
{
	if ((m_irq5_duart1) || (m_irq5_isa))
	{
		m_maincpu->set_input_line(M68K_IRQ_5, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER(pt68k4_state::duart1_irq)
{
	m_irq5_duart1 = state;
	irq5_update();
}

WRITE_LINE_MEMBER(pt68k4_state::irq5_w)
{
	m_irq5_isa = state;
	irq5_update();
}

WRITE_LINE_MEMBER(pt68k4_state::duart2_irq)
{
	m_maincpu->set_input_line(M68K_IRQ_4, state);
}

// these are cards supported by the HUMBUG and Monk BIOSes
SLOT_INTERFACE_START( pt68k4_isa8_cards )
	SLOT_INTERFACE("mda", ISA8_MDA)
	SLOT_INTERFACE("cga", ISA8_CGA)
	SLOT_INTERFACE("ega", ISA8_EGA) // Monk only
	SLOT_INTERFACE("vga", ISA8_VGA) // Monk only
	SLOT_INTERFACE("fdc_at", ISA8_FDC_AT)
	SLOT_INTERFACE("wdxt_gen", ISA8_WDXT_GEN)
	SLOT_INTERFACE("lpt", ISA8_LPT)
	SLOT_INTERFACE("xtide", ISA8_XTIDE) // Monk only
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( pt68k2, pt68k4_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(M68K_TAG, M68000, XTAL_16MHz/2)    // 68k2 came in 8, 10, and 12 MHz versions
	MCFG_CPU_PROGRAM_MAP(pt68k2_mem)

	MCFG_MC68681_ADD("duart1", XTAL_3_6864MHz)
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(pt68k4_state, duart1_irq))
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(pt68k4_state, duart1_out))

	MCFG_MC68681_ADD("duart2", XTAL_3_6864MHz)

	MCFG_DEVICE_ADD(KBDC_TAG, PC_KBDC, 0)
	MCFG_PC_KBDC_OUT_CLOCK_CB(WRITELINE(pt68k4_state, keyboard_clock_w))
	MCFG_PC_KBDC_OUT_DATA_CB(WRITELINE(pt68k4_state, keyboard_data_w))
	MCFG_PC_KBDC_SLOT_ADD(KBDC_TAG, "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83)

	MCFG_M48T02_ADD(TIMEKEEPER_TAG)

	MCFG_WD1772_ADD(WDFDC_TAG, XTAL_16MHz / 2)
	MCFG_FLOPPY_DRIVE_ADD(WDFDC_TAG":0", pt68k_floppies, "525dd", pt68k4_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WDFDC_TAG":1", pt68k_floppies, "525dd", pt68k4_state::floppy_formats)

	MCFG_DEVICE_ADD(ISABUS_TAG, ISA8, 0)
	MCFG_ISA8_CPU(":" M68K_TAG)
	MCFG_ISA8_BUS_CUSTOM_SPACES()
	MCFG_ISA_OUT_IRQ5_CB(WRITELINE(pt68k4_state, irq5_w))
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa1", pt68k4_isa8_cards, "cga", false)
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa2", pt68k4_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa3", pt68k4_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa4", pt68k4_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa5", pt68k4_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa6", pt68k4_isa8_cards, nullptr, false)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_SOFTWARE_LIST_ADD("flop525_list", "pt68k2")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pt68k4, pt68k4_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(M68K_TAG, M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(pt68k4_mem)

	// add the DUARTS.  first one has the console on channel A at 19200.
	MCFG_MC68681_ADD("duart1", XTAL_16MHz / 4)
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(pt68k4_state, duart1_irq))
	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(pt68k4_state, duart1_out))

	MCFG_MC68681_ADD("duart2", XTAL_16MHz / 4)

	MCFG_DEVICE_ADD(KBDC_TAG, PC_KBDC, 0)
	MCFG_PC_KBDC_OUT_CLOCK_CB(WRITELINE(pt68k4_state, keyboard_clock_w))
	MCFG_PC_KBDC_OUT_DATA_CB(WRITELINE(pt68k4_state, keyboard_data_w))
	MCFG_PC_KBDC_SLOT_ADD(KBDC_TAG, "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83)

	MCFG_M48T02_ADD(TIMEKEEPER_TAG)

	MCFG_DEVICE_ADD(ISABUS_TAG, ISA8, 0)
	MCFG_ISA8_CPU(":" M68K_TAG)
	MCFG_ISA8_BUS_CUSTOM_SPACES()
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa1", pt68k4_isa8_cards, "fdc_at", false)
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa2", pt68k4_isa8_cards, "cga", false)
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa3", pt68k4_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa4", pt68k4_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa5", pt68k4_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa6", pt68k4_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD(ISABUS_TAG, "isa7", pt68k4_isa8_cards, nullptr, false)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_SOFTWARE_LIST_ADD("flop525_list", "pt68k2")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pt68k2 )
	ROM_REGION16_BE( 0x10000, "roms", 0 )
	ROM_LOAD16_BYTE( "hum_u20.bin",  0x000000, 0x008000, CRC(69db483a) SHA1(9dfea73e4d7deef7c66a27cca92eb7c9ff767215) )
	ROM_LOAD16_BYTE( "hum_u27.bin",  0x000001, 0x008000, CRC(54441b06) SHA1(0e2d63b1cd01f88f37fc4859c11c252c4fea220b) )

	ROM_REGION(0x800, TIMEKEEPER_TAG, 0)
	ROM_LOAD( "u21_ds1220.bin", 0x000000, 0x000800, CRC(7a6b75ce) SHA1(07663860aa6cc21aed04a568ff9c05bc75d62e4f) )
ROM_END

ROM_START( pt68k4 )
	ROM_REGION16_BE( 0x10000, "roms", 0 )
	ROM_SYSTEM_BIOS( 0, "humbug", "Humbug" )
	ROMX_LOAD( "humpta40.bin", 0x0000, 0x8000, CRC(af67ff64) SHA1(da9fa31338c6847bb0e66118679b1ec01f6dc30b), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "humpta41.bin", 0x0001, 0x8000, CRC(a8b16e27) SHA1(218802f6e20d14cff736bb7423f06ce2f66e074c), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "monk", "Monk" )
	ROMX_LOAD( "monk_0.bin", 0x0000, 0x8000, CRC(420d6a4b) SHA1(fca8c53c9c3c8ebd09370499cf34f4cc75ed9463), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "monk_1.bin", 0x0001, 0x8000, CRC(fc495e82) SHA1(f7b720d87db4d72a23e6c42d2cdd03216db04b60), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_REGION(0x800, TIMEKEEPER_TAG, 0)
	ROM_LOAD( "u21_ds1220_k4.bin", 0x000000, 0x000800, CRC(753472e6) SHA1(58dc8bcc86191e4a4429fe6a9b4fdd7788abb0cd) )

	ROM_REGION( 0x0900, "proms", 0 )
	ROM_LOAD_OPTIONAL( "20l8.u71",    0x0000, 0x000149, CRC(77365121) SHA1(5ecf490ead119966a5c097d90740acde60462ab0) )
	ROM_LOAD_OPTIONAL( "16l8.u53",    0x0200, 0x000109, CRC(cb6a9984) SHA1(45b9b14e7b45cda6f0edfcbb9895b6a14eacb852) )
	ROM_LOAD_OPTIONAL( "22v10.u40",   0x0400, 0x0002e1, CRC(24df92e4) SHA1(c183113956bb0db132b6f37b239ca0bb7fac2d82) )
	ROM_LOAD_OPTIONAL( "16l8.u11",    0x0700, 0x000109, CRC(397a1363) SHA1(aca2a02e1bf1f7cdb9b0ca24ebecb0b01ae472e8) )
ROM_END

/* Driver */
/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   CLASS          INIT     COMPANY             FULLNAME       FLAGS */
COMP( 1988, pt68k2,  0,       0,     pt68k2,    pt68k4, driver_device, 0,  "Peripheral Technology", "PT68K2", MACHINE_SUPPORTS_SAVE )
COMP( 1990, pt68k4,  0,       0,     pt68k4,    pt68k4, driver_device, 0,  "Peripheral Technology", "PT68K4", MACHINE_SUPPORTS_SAVE )
