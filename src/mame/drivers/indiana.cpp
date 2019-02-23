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

#include "emu.h"
#include "bus/rs232/keyboard.h"
#include "cpu/m68000/m68000.h"
#include "bus/isa/com.h"
#include "bus/isa/fdc.h"
#include "bus/isa/ide.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/isa/vga.h"
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

	void indiana(machine_config &config);

	void init_indiana();

private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	void indiana_mem(address_map &map);
};


void indiana_state::indiana_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0000ffff).mirror(0x7f800000).rom().region("user1", 0); // 64Kb of EPROM
	map(0x00100000, 0x00107fff).mirror(0x7f8f8000).ram(); // SRAM 32Kb of SRAM
	map(0x00200000, 0x002fffff).rw(MFP_TAG, FUNC(mc68901_device::read), FUNC(mc68901_device::write)).mirror(0x7f800000); // MFP
	map(0x00400000, 0x004fffff).rw(ISABUS_TAG, FUNC(isa16_device::io16_swap_r), FUNC(isa16_device::io16_swap_w)).mirror(0x7f800000); // 16 bit PC IO
	map(0x00500000, 0x005fffff).rw(ISABUS_TAG, FUNC(isa16_device::mem16_swap_r), FUNC(isa16_device::mem16_swap_w)).mirror(0x7f800000); // 16 bit PC MEM
	map(0x00600000, 0x006fffff).rw(ISABUS_TAG, FUNC(isa16_device::io_r), FUNC(isa16_device::io_w)).mirror(0x7f800000); // 8 bit PC IO
	map(0x00700000, 0x007fffff).rw(ISABUS_TAG, FUNC(isa16_device::mem_r), FUNC(isa16_device::mem_w)).mirror(0x7f800000); // 8 bit PC MEM
	map(0x80000000, 0x803fffff).ram(); // 4 MB RAM
	map(0xfffe0000, 0xfffe7fff).ram(); // SRAM mirror?
}


/* Input ports */
static INPUT_PORTS_START( indiana )
INPUT_PORTS_END


void indiana_state::machine_reset()
{
}

void indiana_state::init_indiana()
{
}

void indiana_isa_cards(device_slot_interface &device)
{
	// 8-bit
	device.option_add("fdc_at", ISA8_FDC_AT);
	device.option_add("comat", ISA8_COM_AT);
	device.option_add("vga", ISA8_VGA);

	// 16-bit
	device.option_add("ide", ISA16_IDE);
}

static DEVICE_INPUT_DEFAULTS_START( keyboard )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

MACHINE_CONFIG_START(indiana_state::indiana)
	/* basic machine hardware */
	MCFG_DEVICE_ADD(M68K_TAG, M68030, XTAL(16'000'000))
	MCFG_DEVICE_PROGRAM_MAP(indiana_mem)

	// FIXME: determine ISA bus clock
	isa16_device &isa(ISA16(config, ISABUS_TAG, 0));
	isa.set_custom_spaces();

	MCFG_DEVICE_ADD("isa1", ISA16_SLOT, 0, ISABUS_TAG, indiana_isa_cards, "vga", false)
	MCFG_DEVICE_ADD("isa2", ISA16_SLOT, 0, ISABUS_TAG, indiana_isa_cards, "fdc_at", false)
	MCFG_DEVICE_ADD("isa3", ISA16_SLOT, 0, ISABUS_TAG, indiana_isa_cards, "comat", false)
	MCFG_DEVICE_ADD("isa4", ISA16_SLOT, 0, ISABUS_TAG, indiana_isa_cards, "ide", false)

	mc68901_device &mfp(MC68901(config, MFP_TAG, XTAL(16'000'000)/4));
	mfp.set_timer_clock(XTAL(16'000'000)/4);
	mfp.set_rx_clock(0);
	mfp.set_tx_clock(0);
	mfp.out_so_cb().set("keyboard", FUNC(rs232_port_device::write_txd));

	rs232_port_device &keyboard(RS232_PORT(config, "keyboard", default_rs232_devices, "keyboard"));
	keyboard.rxd_handler().set(MFP_TAG, FUNC(mc68901_device::write_rx));
	keyboard.set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( indiana )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v9", "ver 0.9" )
	ROMX_LOAD( "prom0_9.bin", 0x0000, 0x10000, CRC(746ad75e) SHA1(7d5c123c8568b1e02ab683e8f3188d0fef78d740), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v8", "ver 0.8" )
	ROMX_LOAD( "prom0_8.bin", 0x0000, 0x10000, CRC(9d8dafee) SHA1(c824e5fe6eec08f51ef287c651a5034fe3c8b718), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v7", "ver 0.7" )
	ROMX_LOAD( "prom0_7.bin", 0x0000, 0x10000, CRC(d6a3b6bc) SHA1(01d8cee989ab29646d9d3f8b7262b10055653d41), ROM_BIOS(2))
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY               FULLNAME                          FLAGS
COMP( 1993, indiana, 0,      0,      indiana, indiana, indiana_state, init_indiana, "Indiana University", "Indiana University 68030 board", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
