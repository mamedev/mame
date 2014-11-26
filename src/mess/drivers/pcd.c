/***************************************************************************

    Siemens PC-D

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

    Skeleton driver

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/mc2661.h"
#include "machine/omti5100.h"
#include "machine/wd_fdc.h"
#include "machine/mc146818.h"
#include "sound/speaker.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pcd_state : public driver_device
{
public:
	pcd_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_pic1(*this, "pic1"),
	m_pic2(*this, "pic2"),
	m_speaker(*this, "speaker"),
	m_sasi(*this, "sasi"),
	m_fdc(*this, "fdc"),
	m_rtc(*this, "rtc")
	{ }

	DECLARE_WRITE_LINE_MEMBER( pic1_irq );
	DECLARE_READ8_MEMBER( pic1_slave_ack_r );
	TIMER_DEVICE_CALLBACK_MEMBER( timer0_tick );
	DECLARE_WRITE_LINE_MEMBER( i186_timer1_w );

	DECLARE_READ8_MEMBER( crt_data_r );
	DECLARE_WRITE8_MEMBER( crt_data_w );
	DECLARE_READ8_MEMBER( crt_status_r );

protected:
	// driver_device overrides
	virtual void machine_start();

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<speaker_sound_device> m_speaker;
	required_device<omti5100_device> m_sasi;
	required_device<wd2793_t> m_fdc;
	required_device<mc146818_device> m_rtc;
};


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void pcd_state::machine_start()
{
}

WRITE_LINE_MEMBER( pcd_state::pic1_irq )
{
	logerror("pic1 irq: %d\n", state);
	m_maincpu->int0_w(state); // ?
}

READ8_MEMBER( pcd_state::pic1_slave_ack_r )
{
	if (offset == 0) // irq 0
		return m_pic2->acknowledge();

	return 0x00;
}

TIMER_DEVICE_CALLBACK_MEMBER( pcd_state::timer0_tick )
{
	m_maincpu->tmrin0_w(0);
	m_maincpu->tmrin0_w(1);
}

WRITE_LINE_MEMBER( pcd_state::i186_timer1_w )
{
	m_speaker->level_w(state);
}

READ8_MEMBER( pcd_state::crt_data_r )
{
	logerror("crt_data_r @ %02x\n", offset);
	return 0xff;
}

WRITE8_MEMBER( pcd_state::crt_data_w )
{
	logerror("crt_data_w %02x @ %02x\n", data, offset);
}

READ8_MEMBER( pcd_state::crt_status_r )
{
	logerror("crt_status_r @ %02x\n", offset);
	return 0xff;
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( pcd_map, AS_PROGRAM, 16, pcd_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM // fixed 256k for now
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pcd_io, AS_IO, 16, pcd_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf840, 0xf841) AM_DEVREADWRITE8("pic1", pic8259_device, read, write, 0xff00)
	AM_RANGE(0xf900, 0xf907) AM_DEVREADWRITE8("fdc", wd2793_t, read, write, 0x00ff)
//  AM_RANGE(0xf940, 0xf941) // sasi controller here?
	AM_RANGE(0xf980, 0xf981) AM_READWRITE8(crt_data_r, crt_data_w, 0x00ff) AM_READ8(crt_status_r, 0xff00)
//  AM_RANGE(0xfa00, 0xfa7f) // pcs4-n (peripheral chip select)
ADDRESS_MAP_END


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static SLOT_INTERFACE_START( pcd_floppies )
	SLOT_INTERFACE("55f", TEAC_FD_55F)
	SLOT_INTERFACE("55g", TEAC_FD_55G)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( pcd, pcd_state )
	MCFG_CPU_ADD("maincpu", I80186, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(pcd_map)
	MCFG_CPU_IO_MAP(pcd_io)
	MCFG_80186_TMROUT1_HANDLER(WRITELINE(pcd_state, i186_timer1_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer0_tick", pcd_state, timer0_tick, attotime::from_hz(XTAL_16MHz / 2 / 16))

	MCFG_PIC8259_ADD("pic1", WRITELINE(pcd_state, pic1_irq), VCC, READ8(pcd_state, pic1_slave_ack_r))
	MCFG_PIC8259_ADD("pic2", DEVWRITELINE("pic1", pic8259_device, ir0_w), GND, NULL)

#if 0
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256K")
	MCFG_RAM_EXTRA_OPTIONS("512K,1M")
#endif

	// nvram
	MCFG_NVRAM_ADD_1FILL("nvram")

	// sasi controller
	MCFG_OMTI5100_ADD("sasi")

	// floppy disk controller
	MCFG_WD2793x_ADD("fdc", XTAL_16MHz/2/8)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE("pic1", pic8259_device, ir6_w))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, drq1_w))

	// floppy drives
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pcd_floppies, "55g", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pcd_floppies, "55g", floppy_image_device::default_floppy_formats)

	// usart
	MCFG_DEVICE_ADD("usart1", MC2661, XTAL_4_9152MHz)
	MCFG_MC2661_TXEMT_DSCHG_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir2_w))
	MCFG_DEVICE_ADD("usart2", MC2661, XTAL_4_9152MHz)
	MCFG_MC2661_TXEMT_DSCHG_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir3_w))
	MCFG_DEVICE_ADD("usart3", MC2661, XTAL_4_9152MHz)
	MCFG_MC2661_TXEMT_DSCHG_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir4_w))

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	// rtc
	MCFG_MC146818_ADD("rtc", XTAL_32_768kHz)
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir7_w))
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( pcd )
	ROM_REGION(0x4000, "bios", 0)
	ROM_LOAD16_BYTE("s26361-d359.d42", 0x0001, 0x2000, CRC(e20244dd) SHA1(0ebc5ddb93baacd9106f1917380de58aac64fe73))
	ROM_LOAD16_BYTE("s26361-d359.d43", 0x0000, 0x2000, CRC(e03db2ec) SHA1(fcae8b0c9e7543706817b0a53872826633361fda))

	// gfx card (scn2674 with 8741), to be moved
	ROM_REGION(0x400, "graphics", 0)
	ROM_LOAD("s36361-d321-v1.bin", 0x000, 0x400, CRC(69baeb2a) SHA1(98b9cd0f38c51b4988a3aed0efcf004bedd115ff))

	// keyboard (8035), to be moved
	ROM_REGION(0x1000, "keyboard", 0)
	ROM_LOAD("pcd_keyboard.bin", 0x0000, 0x1000, CRC(d227d6cb) SHA1(3d6140764d3d043428c941826370ebf1597c63bd))
ROM_END


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

COMP( 1984, pcd, 0, 0, pcd, 0, driver_device, 0, "Siemens", "PC-D", GAME_NOT_WORKING )
