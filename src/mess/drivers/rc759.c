// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Regnecentralen RC759 Piccoline

    Status: Error 32 (cassette data error)

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "machine/mm58167.h"
#include "machine/pic8259.h"
#include "machine/i8255.h"
#include "video/i82730.h"
#include "sound/speaker.h"
#include "sound/sn76496.h"
#include "machine/keyboard.h"
#include "bus/centronics/ctronics.h"
#include "machine/wd_fdc.h"
#include "imagedev/cassette.h"
#include "bus/isbx/isbx.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class rc759_state : public driver_device
{
public:
	rc759_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_pic(*this, "pic"),
	m_nvram(*this, "nvram"),
	m_ppi(*this, "ppi"),
	m_txt(*this, "txt"),
	m_cas(*this, "cas"),
	m_isbx(*this, "isbx"),
	m_speaker(*this, "speaker"),
	m_snd(*this, "snd"),
	m_rtc(*this, "rtc"),
	m_centronics(*this, "centronics"),
	m_fdc(*this, "fdc"),
	m_floppy0(*this, "fdc:0"),
	m_floppy1(*this, "fdc:1"),
	m_vram(*this, "vram"),
	m_config(*this, "config"),
	m_cas_enabled(0), m_cas_data(0),
	m_drq_source(0),
	m_nvram_bank(0),
	m_gfx_mode(0),
	m_keyboard_enable(0), m_keyboard_key(0x00),
	m_centronics_strobe(0), m_centronics_init(0), m_centronics_select_in(0), m_centronics_busy(0),
	m_centronics_ack(0), m_centronics_fault(0), m_centronics_perror(0), m_centronics_select(0),
	m_centronics_data(0xff)
	{ }

	DECLARE_WRITE8_MEMBER(keyb_put);
	DECLARE_READ8_MEMBER(keyboard_r);

	DECLARE_WRITE8_MEMBER(floppy_control_w);
	DECLARE_READ8_MEMBER(floppy_ack_r);
	DECLARE_WRITE8_MEMBER(floppy_reserve_w);
	DECLARE_WRITE8_MEMBER(floppy_release_w);

	DECLARE_READ8_MEMBER(ppi_porta_r);
	DECLARE_READ8_MEMBER(ppi_portb_r);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);

	DECLARE_WRITE_LINE_MEMBER(centronics_busy_w);
	DECLARE_WRITE_LINE_MEMBER(centronics_ack_w);
	DECLARE_WRITE_LINE_MEMBER(centronics_fault_w);
	DECLARE_WRITE_LINE_MEMBER(centronics_perror_w);
	DECLARE_WRITE_LINE_MEMBER(centronics_select_w);

	DECLARE_READ8_MEMBER(centronics_data_r);
	DECLARE_WRITE8_MEMBER(centronics_data_w);
	DECLARE_READ8_MEMBER(centronics_control_r);
	DECLARE_WRITE8_MEMBER(centronics_control_w);

	I82730_UPDATE_ROW(txt_update_row);
	DECLARE_WRITE16_MEMBER(txt_ca_w);
	DECLARE_WRITE16_MEMBER(txt_irst_w);
	DECLARE_READ8_MEMBER(palette_r);
	DECLARE_WRITE8_MEMBER(palette_w);

	DECLARE_WRITE_LINE_MEMBER(i186_timer0_w);
	DECLARE_WRITE_LINE_MEMBER(i186_timer1_w);

	void nvram_init(nvram_device &nvram, void *data, size_t size);
	DECLARE_READ8_MEMBER(nvram_r);
	DECLARE_WRITE8_MEMBER(nvram_w);
	DECLARE_READ8_MEMBER(rtc_r);
	DECLARE_WRITE8_MEMBER(rtc_w);
	DECLARE_READ8_MEMBER(irq_callback);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<nvram_device> m_nvram;
	required_device<i8255_device> m_ppi;
	required_device<i82730_device> m_txt;
	required_device<cassette_image_device> m_cas;
	required_device<isbx_slot_device> m_isbx;
	required_device<speaker_sound_device> m_speaker;
	required_device<sn76489a_device> m_snd;
	required_device<mm58167_device> m_rtc;
	required_device<centronics_device> m_centronics;
	required_device<wd2797_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_shared_ptr<UINT16> m_vram;
	required_ioport m_config;

	std::vector<UINT8> m_nvram_mem;

	int m_cas_enabled;
	int m_cas_data;
	int m_drq_source;
	int m_nvram_bank;
	int m_gfx_mode;
	int m_keyboard_enable;
	UINT8 m_keyboard_key;

	int m_centronics_strobe;
	int m_centronics_init;
	int m_centronics_select_in;
	int m_centronics_busy;
	int m_centronics_ack;
	int m_centronics_fault;
	int m_centronics_perror;
	int m_centronics_select;
	UINT8 m_centronics_data;
};


//**************************************************************************
//  I/O
//**************************************************************************

WRITE8_MEMBER( rc759_state::keyb_put )
{
	m_keyboard_key = data;
	m_pic->ir1_w(1);
}

READ8_MEMBER( rc759_state::keyboard_r )
{
	logerror("keyboard_r\n");

	m_pic->ir1_w(0);

	if (m_keyboard_enable)
		return m_keyboard_key;
	else
		return 0x00;
}

READ8_MEMBER( rc759_state::ppi_porta_r )
{
	UINT8 data = 0;

	data |= m_cas_enabled ? m_cas_data : (m_cas->input() > 0 ? 1 : 0);
	data |= m_isbx->mpst_r() << 1;
	data |= m_isbx->opt0_r() << 2;
	data |= m_isbx->opt1_r() << 3;
	data |= 1 << 4; // mem ident0
	data |= 1 << 5; // mem ident1 (both 1 = 256k installed)
	data |= 1 << 6; // dpc connect (0 = external floppy/printer installed)
	data |= 1 << 7; // not used

	return data;
}

READ8_MEMBER( rc759_state::ppi_portb_r )
{
	UINT8 data = 0;

	data |= 1 << 0; // 0 = micronet controller installed
	data |= 1 << 1; // rtc type, mm58167/cdp1879
	data |= m_snd->ready_r() << 2;
	data |= 1 << 3; // not used
	data |= 1 << 4; // not used
	data |= m_config->read(); // monitor type and frequency
	data |= 1 << 7; // 0 = enable remote hardware debug (using an isbx351 module)

	return data;
}

WRITE8_MEMBER( rc759_state::ppi_portc_w )
{
	m_cas_enabled = BIT(data, 0);
	m_cas->change_state(BIT(data, 1) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
	m_drq_source = (data >> 2) & 0x03;
	m_nvram_bank = (data >> 4) & 0x03;
	m_gfx_mode = BIT(data, 6);
	m_keyboard_enable = BIT(data, 7);

	logerror("ppi_portc_w: cas_enabled: %d, cas_motor: %d, drq_source: %d, nvram_bank: %d, gfx_mode: %d, keyb_enable: %d\n",
		m_cas_enabled, BIT(data, 1), m_drq_source, m_nvram_bank, m_gfx_mode, m_keyboard_enable);
}

WRITE_LINE_MEMBER( rc759_state::centronics_busy_w )
{
	m_centronics_busy = state;
	m_pic->ir6_w(state);
}

WRITE_LINE_MEMBER( rc759_state::centronics_ack_w )
{
	m_centronics_ack = state;
}

WRITE_LINE_MEMBER( rc759_state::centronics_fault_w )
{
	m_centronics_fault = state;
}

WRITE_LINE_MEMBER( rc759_state::centronics_perror_w )
{
	m_centronics_perror = state;
}

WRITE_LINE_MEMBER( rc759_state::centronics_select_w )
{
	m_centronics_select = state;
}

READ8_MEMBER( rc759_state::centronics_data_r )
{
	return m_centronics_data;
}

WRITE8_MEMBER( rc759_state::centronics_data_w )
{
	m_centronics_data = data;

	m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_data1(BIT(data, 1));
	m_centronics->write_data2(BIT(data, 2));
	m_centronics->write_data3(BIT(data, 3));
	m_centronics->write_data4(BIT(data, 4));
	m_centronics->write_data5(BIT(data, 5));
	m_centronics->write_data6(BIT(data, 6));
	m_centronics->write_data7(BIT(data, 7));
}

READ8_MEMBER( rc759_state::centronics_control_r )
{
	UINT8 data = 0;

	data |= m_centronics_busy << 0;
	data |= m_centronics_ack << 1;
	data |= m_centronics_fault << 2;
	data |= m_centronics_perror << 3;
	data |= m_centronics_select << 4;
	data |= !m_centronics_strobe << 5;
	data |= !m_centronics_init << 6;
	data |= !m_centronics_select_in << 7;

	return data;
}

WRITE8_MEMBER( rc759_state::centronics_control_w )
{
	logerror("centronics_control_w: %02x\n", data);

	m_centronics_strobe = BIT(data, 0);
	m_centronics_init = BIT(data, 2);
	m_centronics_select_in = BIT(data, 4);

	m_centronics->write_strobe(m_centronics_strobe);
	m_centronics->write_autofd(BIT(data, 1));
	m_centronics->write_init(m_centronics_init);
	m_centronics->write_select_in(m_centronics_select_in);
}

WRITE8_MEMBER( rc759_state::floppy_control_w )
{
	logerror("floppy_control_w: %02x\n", data);

	switch (BIT(data, 0))
	{
	case 0: m_fdc->set_floppy(m_floppy0->get_device()); break;
	case 1: m_fdc->set_floppy(m_floppy1->get_device()); break;
	}

	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(!BIT(data, 1));
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(!BIT(data, 2));

	// bit 3, enable precomp
	// bit 4, precomp 125/250 nsec

	m_fdc->dden_w(BIT(data, 5));
	m_fdc->set_unscaled_clock(BIT(data, 6) ? 2000000 : 1000000);
	m_fdc->set_force_ready(BIT(data, 7));
}

READ8_MEMBER( rc759_state::floppy_ack_r )
{
	logerror("floppy_ack_r\n");
	return 0xff;
}

WRITE8_MEMBER( rc759_state::floppy_reserve_w )
{
	logerror("floppy_reserve_w: %02x\n", data);
}

WRITE8_MEMBER( rc759_state::floppy_release_w )
{
	logerror("floppy_release_w: %02x\n", data);
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

I82730_UPDATE_ROW( rc759_state::txt_update_row )
{
	for (int i = 0; i < x_count; i++)
	{
		UINT16 gfx = m_vram[(data[i] & 0x3ff) << 4 | lc];

		// pretty crude detection if char sizes have been initialized, need something better
		if ((gfx & 0xff) == 0)
			continue;

		// figure out char width
		int width;
		for (width = 0; width < 16; width++)
			if (BIT(gfx, width) == 0)
				break;

		width = 15 - width;

		for (int p = 0; p < width; p++)
			bitmap.pix32(y, i * width + p) = BIT(gfx, 15 - p) ? rgb_t::white : rgb_t::black;
	}
}

WRITE16_MEMBER( rc759_state::txt_ca_w )
{
	m_txt->ca_w(1);
	m_txt->ca_w(0);
}

WRITE16_MEMBER( rc759_state::txt_irst_w )
{
	m_txt->irst_w(1);
	m_txt->irst_w(0);
}

READ8_MEMBER( rc759_state::palette_r )
{
	logerror("palette_r(%02x)\n", offset);
	return 0xff;
}

WRITE8_MEMBER( rc759_state::palette_w )
{
	logerror("palette_w(%02x): %02x\n", offset, data);
}


//**************************************************************************
//  SOUND/RTC
//**************************************************************************

READ8_MEMBER( rc759_state::rtc_r )
{
	logerror("rtc_r(%02x)\n", offset);
	return 0xff;
}

WRITE8_MEMBER( rc759_state::rtc_w )
{
	logerror("rtc_w(%02x): %02x\n", offset, data);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

WRITE_LINE_MEMBER( rc759_state::i186_timer0_w )
{
	if (m_cas_enabled)
	{
		m_cas_data = state;
		m_cas->output(state ? -1.0 : 1.0);
	}
}

WRITE_LINE_MEMBER( rc759_state::i186_timer1_w )
{
	m_speaker->level_w(state);
}

// 256x4 nvram is bank-switched using ppi port c, bit 4 and 5
void rc759_state::nvram_init(nvram_device &nvram, void *data, size_t size)
{
	memset(data, 0x00, size);
	memset(data, 0xaa, 1);
}

READ8_MEMBER( rc759_state::nvram_r )
{
	offs_t addr = (m_nvram_bank << 6) | offset;

	logerror("nvram_r(%02x)\n", addr);

	if (addr & 1)
		return (m_nvram_mem[addr >> 1] & 0xf0) >> 4;
	else
		return (m_nvram_mem[addr >> 1] & 0x0f) >> 0;
}

WRITE8_MEMBER( rc759_state::nvram_w )
{
	offs_t addr = (m_nvram_bank << 6) | offset;

	logerror("nvram_w(%02x): %02x\n", addr, data);

	if (addr & 1)
		m_nvram_mem[addr >> 1] = ((data << 4) & 0xf0) | (m_nvram_mem[addr >> 1] & 0x0f);
	else
		m_nvram_mem[addr >> 1] = (m_nvram_mem[addr >> 1] & 0xf0) | (data & 0x0f);
}

READ8_MEMBER( rc759_state::irq_callback )
{
	return m_pic->acknowledge();
}

void rc759_state::machine_start()
{
	m_nvram_mem.resize(256 / 2);
	m_nvram->set_base(&m_nvram_mem[0], 256 / 2);
}

void rc759_state::machine_reset()
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( rc759_map, AS_PROGRAM, 16, rc759_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM
	AM_RANGE(0xd8000, 0xdffff) AM_MIRROR(0x08000) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xf8000, 0xfffff) AM_MIRROR(0x10000) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rc759_io, AS_IO, 16, rc759_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000, 0x003) AM_MIRROR(0x0c) AM_DEVREADWRITE8("pic", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x020, 0x021) AM_READ8(keyboard_r, 0x00ff)
	AM_RANGE(0x056, 0x057) AM_NOP // in reality, access to sound and rtc is a bit more involved
	AM_RANGE(0x05a, 0x05b) AM_DEVWRITE8("snd", sn76489a_device, write, 0x00ff)
	AM_RANGE(0x05c, 0x05d) AM_READWRITE8(rtc_r, rtc_w, 0x00ff)
//	AM_RANGE(0x060, 0x06f) AM_WRITE8(crt_control_w, 0x00ff)
	AM_RANGE(0x070, 0x077) AM_MIRROR(0x08) AM_DEVREADWRITE8("ppi", i8255_device, read, write, 0x00ff)
	AM_RANGE(0x080, 0x0ff) AM_READWRITE8(nvram_r, nvram_w, 0x00ff)
//  AM_RANGE(0x100, 0x101) net
	AM_RANGE(0x180, 0x1bf) AM_READWRITE8(palette_r, palette_w, 0x00ff)
	AM_RANGE(0x230, 0x231) AM_WRITE(txt_irst_w)
	AM_RANGE(0x240, 0x241) AM_WRITE(txt_ca_w)
	AM_RANGE(0x250, 0x251) AM_READWRITE8(centronics_data_r, centronics_data_w, 0x00ff)
	AM_RANGE(0x260, 0x261) AM_READWRITE8(centronics_control_r, centronics_control_w, 0x00ff)
	AM_RANGE(0x280, 0x287) AM_DEVREADWRITE8("fdc", wd2797_t, read, write, 0x00ff)
	AM_RANGE(0x288, 0x289) AM_WRITE8(floppy_control_w, 0x00ff)
//  AM_RANGE(0x28a, 0x28b) external printer data
//  AM_RANGE(0x28d, 0x28d) external printer control
	AM_RANGE(0x28e, 0x28f) AM_READWRITE8(floppy_ack_r, floppy_reserve_w, 0x00ff)
	AM_RANGE(0x290, 0x291) AM_WRITE8(floppy_release_w, 0x00ff)
//	AM_RANGE(0x292, 0x293) AM_READWRITE8(printer_ack_r, printer_reserve_w, 0x00ff)
//	AM_RANGE(0x294, 0x295) AM_WRITE8(printer_release_w, 0x00ff)
	AM_RANGE(0x300, 0x30f) AM_DEVREADWRITE8("isbx", isbx_slot_device, mcs0_r, mcs0_w, 0x00ff)
	AM_RANGE(0x310, 0x31f) AM_DEVREADWRITE8("isbx", isbx_slot_device, mcs1_r, mcs1_w, 0x00ff)
//	AM_RANGE(0x320, 0x321) isbx dma ack
//	AM_RANGE(0x330, 0x331) isbx tc
ADDRESS_MAP_END


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( rc759 )
	PORT_START("config")
	PORT_CONFNAME(0x20, 0x00, "Monitor Type")
	PORT_CONFSETTING(0x00, "Color")
	PORT_CONFSETTING(0x20, "Monochrome")
	PORT_CONFNAME(0x40, 0x00, "Monitor Frequency")
	PORT_CONFSETTING(0x00, "15 kHz")
	PORT_CONFSETTING(0x40, "22 kHz")
INPUT_PORTS_END


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static SLOT_INTERFACE_START( rc759_floppies )
	SLOT_INTERFACE("hd", FLOPPY_525_HD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( rc759, rc759_state )
	MCFG_CPU_ADD("maincpu", I80186, 6000000)
	MCFG_CPU_PROGRAM_MAP(rc759_map)
	MCFG_CPU_IO_MAP(rc759_io)
	MCFG_80186_IRQ_SLAVE_ACK(READ8(rc759_state, irq_callback))
	MCFG_80186_TMROUT0_HANDLER(WRITELINE(rc759_state, i186_timer0_w))
	MCFG_80186_TMROUT1_HANDLER(WRITELINE(rc759_state, i186_timer1_w))

	// interrupt controller
	MCFG_PIC8259_ADD("pic", DEVWRITELINE("maincpu", i80186_cpu_device, int0_w), VCC, NULL)

	// nvram
	MCFG_NVRAM_ADD_CUSTOM_DRIVER("nvram", rc759_state, nvram_init)

	// ppi
	MCFG_DEVICE_ADD("ppi", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(rc759_state, ppi_porta_r))
	MCFG_I8255_IN_PORTB_CB(READ8(rc759_state, ppi_portb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(rc759_state, ppi_portc_w))

	// rtc
	MCFG_DEVICE_ADD("rtc", MM58167, XTAL_32_768kHz)
	MCFG_MM58167_IRQ_CALLBACK(DEVWRITELINE("pic", pic8259_device, ir3_w))

	// video
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(1250000 * 16, 896, 96, 816, 377, 4, 364) // 22 kHz setting
	MCFG_SCREEN_UPDATE_DEVICE("txt", i82730_device, screen_update)

	MCFG_I82730_ADD("txt", "maincpu", 1250000)
	MCFG_VIDEO_SET_SCREEN("screen")
	MCFG_I82730_UPDATE_ROW_CB(rc759_state, txt_update_row)
	MCFG_I82730_SINT_HANDLER(DEVWRITELINE("pic", pic8259_device, ir4_w))

	// keyboard
    MCFG_DEVICE_ADD("keyb", GENERIC_KEYBOARD, 0)
    MCFG_GENERIC_KEYBOARD_CB(WRITE8(rc759_state, keyb_put))

	// cassette
	MCFG_CASSETTE_ADD("cas")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED)

	// sound
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("snd", SN76489A, XTAL_20MHz / 10)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	// internal centronics
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(rc759_state, centronics_busy_w))
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(rc759_state, centronics_ack_w))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(rc759_state, centronics_fault_w))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(rc759_state, centronics_perror_w))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(rc759_state, centronics_select_w))

	// isbx slot
	MCFG_ISBX_SLOT_ADD("isbx", 0, isbx_cards, NULL)
	MCFG_ISBX_SLOT_MINTR0_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, int1_w))
	MCFG_ISBX_SLOT_MINTR1_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, int3_w))
	MCFG_ISBX_SLOT_MDRQT_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, drq0_w))

	// floppy disk controller
	MCFG_WD2797_ADD("fdc", 1000000)
//	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE("pic", pic8259_device, ir0_w))
//	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, drq1_w))

	// floppy drives
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", rc759_floppies, "hd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", rc759_floppies, "hd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( rc759 )
	ROM_REGION(0x8000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "1-21", "1? Version 2.1")
	ROMX_LOAD("rc759-1-2.1.rom", 0x0000, 0x8000, CRC(3a777d56) SHA1(a8592d61d5e1f92651a6f5e41c4ba14c9b6cc39b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "1-51", "1? Version 5.1")
	ROMX_LOAD("rc759-1-5.1.rom", 0x0000, 0x8000, CRC(e1d53845) SHA1(902dc5ce28efd26b4f9c631933e197c2c187a7f1), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "2-40", "2? Version 4.0")
	ROMX_LOAD("rc759-2-4.0.rom", 0x0000, 0x8000, CRC(d3cb752a) SHA1(f50afe5dfa1b33a36a665d32d57c8c41d6685005), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "2-51", "2? Version 5.1")
	ROMX_LOAD("rc759-2-5.1.rom", 0x0000, 0x8000, CRC(00a31948) SHA1(23c4473c641606a56473791773270411d1019248), ROM_BIOS(4))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

COMP( 1984, rc759, 0, 0, rc759, rc759, driver_device, 0, "Regnecentralen", "RC759 Piccoline", GAME_NOT_WORKING | GAME_NO_SOUND )
