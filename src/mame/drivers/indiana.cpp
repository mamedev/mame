// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Indiana University 68030 board

        08/12/2009 Skeleton driver.
        01/20/2014 Added ISA bus and peripherals

        TODO: Text appears in VGA f/b (0x6B8000), but doesn't display?

        System often reads/writes 6003D4/5, might be a cut-down 6845,
        as it only uses registers C,D,E,F.

****************************************************************************/

#include "bus/rs232/keyboard.h"
#include "cpu/m68000/m68000.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "machine/mc68901.h"

#define M68K_TAG "maincpu"
#define ISABUS_TAG "isa"
#define MFP_TAG "mfp"

class indiana_state : public driver_device
{
public:
	indiana_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, M68K_TAG) { }
	DECLARE_DRIVER_INIT(indiana);
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START(indiana_mem, AS_PROGRAM, 32, indiana_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0000ffff) AM_MIRROR(0x7f800000) AM_ROM AM_REGION("user1", 0) // 64Kb of EPROM
	AM_RANGE(0x00100000, 0x00107fff) AM_MIRROR(0x7f8f8000) AM_RAM // SRAM 32Kb of SRAM
	AM_RANGE(0x00200000, 0x002fffff) AM_DEVREADWRITE8(MFP_TAG, mc68901_device, read, write, 0xffffffff) AM_MIRROR(0x7f800000) // MFP
	AM_RANGE(0x00400000, 0x004fffff) AM_DEVREADWRITE16(ISABUS_TAG, isa16_device, io16_swap_r, io16_swap_w, 0xffffffff) AM_MIRROR(0x7f800000) // 16 bit PC IO
	AM_RANGE(0x00500000, 0x005fffff) AM_DEVREADWRITE16(ISABUS_TAG, isa16_device, prog16_swap_r, prog16_swap_w, 0xffffffff) AM_MIRROR(0x7f800000) // 16 bit PC MEM
	AM_RANGE(0x00600000, 0x006fffff) AM_DEVREADWRITE8(ISABUS_TAG, isa16_device, io_r, io_w, 0xffffffff) AM_MIRROR(0x7f800000) // 8 bit PC IO
	AM_RANGE(0x00700000, 0x007fffff) AM_DEVREADWRITE8(ISABUS_TAG, isa16_device, prog_r, prog_w, 0xffffffff) AM_MIRROR(0x7f800000) // 8 bit PC MEM
	AM_RANGE(0x80000000, 0x803fffff) AM_RAM // 4 MB RAM
	AM_RANGE(0xfffe0000, 0xfffe7fff) AM_RAM // SRAM mirror?
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( indiana )
INPUT_PORTS_END


void indiana_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(indiana_state,indiana)
{
}

SLOT_INTERFACE_START( indiana_isa_cards )
	// 8-bit
	SLOT_INTERFACE("fdc_at", ISA8_FDC_AT)
	SLOT_INTERFACE("comat", ISA8_COM_AT)
	SLOT_INTERFACE("vga", ISA8_VGA)

	// 16-bit
	SLOT_INTERFACE("ide", ISA16_IDE)
SLOT_INTERFACE_END

static DEVICE_INPUT_DEFAULTS_START( keyboard )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static MACHINE_CONFIG_START( indiana, indiana_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(M68K_TAG, M68030, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(indiana_mem)

	MCFG_DEVICE_ADD(ISABUS_TAG, ISA16, 0)
	MCFG_ISA16_CPU(":" M68K_TAG)
	MCFG_ISA16_BUS_CUSTOM_SPACES()
	MCFG_ISA16_SLOT_ADD(ISABUS_TAG, "isa1", indiana_isa_cards, "vga", false)
	MCFG_ISA16_SLOT_ADD(ISABUS_TAG, "isa2", indiana_isa_cards, "fdc_at", false)
	MCFG_ISA16_SLOT_ADD(ISABUS_TAG, "isa3", indiana_isa_cards, "comat", false)
	MCFG_ISA16_SLOT_ADD(ISABUS_TAG, "isa4", indiana_isa_cards, "ide", false)

	MCFG_DEVICE_ADD(MFP_TAG, MC68901, XTAL_16MHz/4)
	MCFG_MC68901_TIMER_CLOCK(XTAL_16MHz/4)
	MCFG_MC68901_RX_CLOCK(0)
	MCFG_MC68901_TX_CLOCK(0)
	MCFG_MC68901_OUT_SO_CB(DEVWRITELINE("keyboard", rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD("keyboard", default_rs232_devices, "keyboard")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(MFP_TAG, mc68901_device, write_rx))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("keyboard", keyboard)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( indiana )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v9", "ver 0.9" )
	ROMX_LOAD( "prom0_9.bin", 0x0000, 0x10000, CRC(746ad75e) SHA1(7d5c123c8568b1e02ab683e8f3188d0fef78d740), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v8", "ver 0.8" )
	ROMX_LOAD( "prom0_8.bin", 0x0000, 0x10000, CRC(9d8dafee) SHA1(c824e5fe6eec08f51ef287c651a5034fe3c8b718), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "v7", "ver 0.7" )
	ROMX_LOAD( "prom0_7.bin", 0x0000, 0x10000, CRC(d6a3b6bc) SHA1(01d8cee989ab29646d9d3f8b7262b10055653d41), ROM_BIOS(3))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                  FULLNAME                               FLAGS */
COMP( 1993, indiana,  0,       0,    indiana,   indiana, indiana_state,  indiana,  "Indiana University", "Indiana University 68030 board", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
