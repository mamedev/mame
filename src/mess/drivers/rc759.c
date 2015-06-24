// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Regnecentralen RC759

    TODO:
    - Emulate the Intel 82730 CRT controller and figure out the rest
    - Connect iSBX bus
    - (much later) move floppy/external printer to the slot interface

    Status: Hangs waiting for an answer from the 82730

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/i8255.h"
#include "machine/wd_fdc.h"
#include "imagedev/cassette.h"


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
	m_ppi(*this, "ppi"),
	m_cas(*this, "cas"),
	m_fdc(*this, "fdc"),
	m_floppy0(*this, "fdc:0"),
	m_floppy1(*this, "fdc:1"),
	m_vram(*this, "vram"),
	m_cas_enabled(0),
	m_drq_source(0),
	m_nvram_bank(0),
	m_gfx_mode(0),
	m_keyboard_enable(0)
	{ }

	DECLARE_READ8_MEMBER(keyboard_r);

	DECLARE_WRITE8_MEMBER(floppy_control_w);
	DECLARE_READ8_MEMBER(floppy_ack_r);
	DECLARE_WRITE8_MEMBER(floppy_reserve_w);
	DECLARE_WRITE8_MEMBER(floppy_release_w);

	DECLARE_READ8_MEMBER(ppi_porta_r);
	DECLARE_READ8_MEMBER(ppi_portb_r);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);

	DECLARE_READ8_MEMBER(palette_r);
	DECLARE_WRITE8_MEMBER(palette_w);

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
	required_device<i8255_device> m_ppi;
	required_device<cassette_image_device> m_cas;
	required_device<wd2797_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_shared_ptr<UINT16> m_vram;

	int m_cas_enabled;
	int m_drq_source;
	int m_nvram_bank;
	int m_gfx_mode;
	int m_keyboard_enable;
};


//**************************************************************************
//  I/O
//**************************************************************************

// pic ir1 keyboard
READ8_MEMBER( rc759_state::keyboard_r )
{
	logerror("keyboard_r\n");
	return 0xff;
}

READ8_MEMBER( rc759_state::ppi_porta_r )
{
	UINT8 data = 0;

	data |= m_cas->input() > 0 ? 1 : 0;
	data |= 1 << 1; // 0 = isbx module installed
	data |= 0 << 2; // option0 from isbx
	data |= 0 << 3; // option1 from isbx
	data |= 1 << 4; // mem ident0
	data |= 1 << 5; // mem ident1 (both 1 = 256k installed)
	data |= 0 << 6; // dpc connect (0 = external floppy/printer installed)
	data |= 1 << 7; // not used

	return data;
}

READ8_MEMBER( rc759_state::ppi_portb_r )
{
	UINT8 data = 0;

	data |= 1 << 0; // 0 = micronet controller installed
	data |= 0 << 1; // rtc type, 0 = cdp1879
	data |= 1 << 2; // sound generator detect
	data |= 1 << 3; // not used
	data |= 1 << 4; // not used
	data |= 0 << 5; // 0 = color monitor, 1 = monochrome
	data |= 1 << 6; // 0 = 15khz, 1 = 22khz monitor
	data |= 1 << 7; // not used

	return data;
}

WRITE8_MEMBER( rc759_state::ppi_portc_w )
{
	logerror("ppi_portc_w: %02x\n", data);

	m_cas_enabled = BIT(data, 0);
	m_cas->change_state(BIT(data, 1) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
	m_drq_source = (data >> 2) & 0x03;
	m_nvram_bank = (data >> 4) & 0x03;
	m_gfx_mode = BIT(data, 6);
	m_keyboard_enable = BIT(data, 7);
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
//  MACHINE EMULATION
//**************************************************************************

// pic ir3 rtc
READ8_MEMBER( rc759_state::rtc_r )
{
	logerror("rtc_r(%02x)\n", offset);
	return 0xff;
}

WRITE8_MEMBER( rc759_state::rtc_w )
{
	logerror("rtc_w(%02x): %02x\n", offset, data);
}

// 256x4 nvram is bank-switched using the ppi port c, bit 4 and 5
READ8_MEMBER( rc759_state::nvram_r )
{
	logerror("nvram_r(%02x)\n", offset);
	return 0xff;
}

WRITE8_MEMBER( rc759_state::nvram_w )
{
	logerror("nvram_w(%02x): %02x\n", offset, data);
}

READ8_MEMBER( rc759_state::irq_callback )
{
	return m_pic->acknowledge();
}

void rc759_state::machine_start()
{
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
	AM_RANGE(0x000, 0x003) AM_DEVREADWRITE8("pic", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x020, 0x021) AM_READ8(keyboard_r, 0x00ff)
	AM_RANGE(0x050, 0x05f) AM_READWRITE8(rtc_r, rtc_w, 0x00ff) // and sound
//  AM_RANGE(0x060, 0x06f) AM_WRITE8(crt_control_w, 0x00ff)
	AM_RANGE(0x070, 0x077) AM_DEVREADWRITE8("ppi", i8255_device, read, write, 0x00ff)
	AM_RANGE(0x080, 0x0ff) AM_READWRITE8(nvram_r, nvram_w, 0x00ff)
//  AM_RANGE(0x100, 0x101) net
	AM_RANGE(0x180, 0x1bf) AM_READWRITE8(palette_r, palette_w, 0x00ff)
//  AM_RANGE(0x230, 0x231) crt reset
//  AM_RANGE(0x240, 0x241) crt ch. att.
//  AM_RANGE(0x250, 0x251) local printer data (centronics)
//  AM_RANGE(0x260, 0x261) local printer control (centronics)
	AM_RANGE(0x280, 0x287) AM_DEVREADWRITE8("fdc", wd2797_t, read, write, 0x00ff)
	AM_RANGE(0x288, 0x289) AM_WRITE8(floppy_control_w, 0x00ff)
//  AM_RANGE(0x28a, 0x28b) external printer data
//  AM_RANGE(0x28d, 0x28d) external printer control
	AM_RANGE(0x28e, 0x28f) AM_READWRITE8(floppy_ack_r, floppy_reserve_w, 0x00ff)
	AM_RANGE(0x290, 0x291) AM_WRITE8(floppy_release_w, 0x00ff)
//  AM_RANGE(0x292, 0x293) AM_READWRITE8(printer_ack_r, printer_reserve_w, 0x00ff)
//  AM_RANGE(0x294, 0x295) AM_WRITE8(printer_release_w, 0x00ff)
//  AM_RANGE(0x300, 0x30f) isbx1
//  AM_RANGE(0x310, 0x31f) isbx2
//  AM_RANGE(0x320, 0x321) isbx dma ack
//  AM_RANGE(0x330, 0x331) isbx tc
ADDRESS_MAP_END


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

	// interrupt controller
	MCFG_PIC8259_ADD("pic", DEVWRITELINE("maincpu", i80186_cpu_device, int0_w), VCC, NULL)

	// ppi
	MCFG_DEVICE_ADD("ppi", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(rc759_state, ppi_porta_r))
	MCFG_I8255_IN_PORTB_CB(READ8(rc759_state, ppi_portb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(rc759_state, ppi_portc_w))

	// floppy disk controller
	MCFG_WD2797_ADD("fdc", 1000000)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE("pic", pic8259_device, ir0_w))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, drq1_w))

	// floppy drives
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", rc759_floppies, "hd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", rc759_floppies, "hd", floppy_image_device::default_floppy_formats)

	// cassette
	MCFG_CASSETTE_ADD("cas")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED)
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

COMP( 1984, rc759, 0, 0, rc759, 0, driver_device, 0, "Regnecentralen", "RC759", GAME_NOT_WORKING | GAME_NO_SOUND )
