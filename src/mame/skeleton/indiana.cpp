// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, AJR
/***************************************************************************

        Indiana University 68030 board

        08/12/2009 Skeleton driver.
        01/20/2014 Added ISA bus and peripherals

        TODO: Text appears in VGA f/b (0x6B8000), but doesn't display?
        TODO: Keyboard doesn't work very well. Scancodes are often lost
              because the 68030 doesn't poll the MFP frequently enough.

****************************************************************************/

#include "emu.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/m68000/m68000.h"
#include "bus/isa/com.h"
#include "bus/isa/fdc.h"
#include "bus/isa/ide.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/isa/vga.h"
#include "machine/mc68901.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#define ISABUS_TAG "isa"

class indiana_state : public driver_device
{
public:
	indiana_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void indiana(machine_config &config);

	void init_indiana();

protected:
	virtual void machine_reset() override;

private:
	void indiana_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
};


void indiana_state::indiana_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0000ffff).mirror(0x7f800000).rom().region("user1", 0); // 64Kb of EPROM
	map(0x00100000, 0x00107fff).mirror(0x7f8f8000).ram(); // SRAM 32Kb of SRAM
	map(0x00200000, 0x002fffff).rw("mfp", FUNC(mc68901_device::read), FUNC(mc68901_device::write)).mirror(0x7f800000); // MFP
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

void indiana_state::indiana(machine_config &config)
{
	/* basic machine hardware */
	M68030(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &indiana_state::indiana_mem);

	isa16_device &isa(ISA16(config, ISABUS_TAG, 16_MHz_XTAL / 2)); // OSC = CLK = CLK8
	isa.set_custom_spaces();
	isa.irq3_callback().set_inputline(m_maincpu, M68K_IRQ_5);
	isa.irq4_callback().set_inputline(m_maincpu, M68K_IRQ_4);
	isa.irq5_callback().set_inputline(m_maincpu, M68K_IRQ_3);
	isa.irq6_callback().set_inputline(m_maincpu, M68K_IRQ_2);
	isa.irq7_callback().set_inputline(m_maincpu, M68K_IRQ_1);
	isa.irq2_callback().set("mfp", FUNC(mc68901_device::i7_w)); // IRQ9
	isa.irq10_callback().set("mfp", FUNC(mc68901_device::i6_w));
	isa.irq11_callback().set("mfp", FUNC(mc68901_device::i5_w));
	isa.irq12_callback().set("mfp", FUNC(mc68901_device::i4_w));
	isa.irq14_callback().set("mfp", FUNC(mc68901_device::i3_w));
	isa.irq15_callback().set("mfp", FUNC(mc68901_device::i2_w));

	ISA16_SLOT(config, "isa1", 0, ISABUS_TAG, indiana_isa_cards, "vga", false);
	ISA16_SLOT(config, "isa2", 0, ISABUS_TAG, indiana_isa_cards, "fdc_at", false);
	ISA16_SLOT(config, "isa3", 0, ISABUS_TAG, indiana_isa_cards, "comat", false);
	ISA16_SLOT(config, "isa4", 0, ISABUS_TAG, indiana_isa_cards, "ide", false);

	pc_kbdc_device &kbd(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_IBM_PC_AT_84));
	kbd.out_data_cb().set("mfp", FUNC(mc68901_device::i0_w));
	kbd.out_data_cb().append("mfp", FUNC(mc68901_device::si_w));
	kbd.out_clock_cb().set("mfp", FUNC(mc68901_device::i1_w));
	kbd.out_clock_cb().append("mfp", FUNC(mc68901_device::rc_w));

	mc68901_device &mfp(MC68901(config, "mfp", 16_MHz_XTAL / 4));
	mfp.set_timer_clock(16_MHz_XTAL / 16);
	mfp.out_irq_cb().set_inputline(m_maincpu, M68K_IRQ_6);
	mfp.out_tdo_cb().set("speaker", FUNC(speaker_sound_device::level_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);
}

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
