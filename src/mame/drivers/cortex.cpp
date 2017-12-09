// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Powertran Cortex

2012-04-20 Skeleton driver.

ftp://ftp.whtech.com/Powertran Cortex/
http://www.powertrancortex.com/index.html

Uses Texas Instruments parts and similar to other TI computers.
It was designed by TI engineers, so it may perhaps be a clone
of another TI or the Geneve.

Chips:
TMS9995   - CPU
TMS9929   - Video
TMS9911   - DMA to floppy (not emulated)
TMS9909   - Floppy Disk Controller (not emulated)
TMS9902   - UART (x2) (not usable with rs232.h)
AY-5-2376 - Keyboard controller

All input to be in uppercase. Note that "lowercase" is just smaller uppercase,
and is not acceptable as input.


ToDo:
- Screen corrupts when scrolling
- Unemulated devices
- Keyboard to use AY device
- Banking
- Memory manager device
- Various CRU I/O

Note that the MAME implementation of CRU addresses is not the same as real
hardware. For writing, MAME uses the correct address (R12/2 + offset), with
the bit (0 or 1), being in 'data'. However, for reading, 8 CRU bits are
packed into a single address-byte (CRU 0 = bit 0, etc). So the address is
(R12/2 + offset) >> 3.

****************************************************************************/


#include "emu.h"
#include "cpu/tms9900/tms9995.h"
#include "machine/74259.h"
#include "video/tms9928a.h"
//#include "machine/tms9902.h"
#include "machine/keyboard.h"

class cortex_state : public driver_device
{
public:
	cortex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "ram")
		{ }

	void kbd_put(u8 data);
	DECLARE_WRITE_LINE_MEMBER(keyboard_ack_w);
	DECLARE_READ8_MEMBER(pio_r);
	DECLARE_READ8_MEMBER(keyboard_r);

private:
	bool m_cru0005;
	uint8_t m_term_data;
	virtual void machine_reset() override;
	required_device<tms9995_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_ram;
};

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, cortex_state )
	AM_RANGE(0x0000, 0xefff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0xf100, 0xf11f) AM_RAM // memory mapping unit
	AM_RANGE(0xf120, 0xf120) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0xf121, 0xf121) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	//AM_RANGE(0xf140, 0xf147) // fdc tms9909
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, cortex_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0007) AM_MIRROR(0x18) AM_DEVWRITE("control", ls259_device, write_d0)
	AM_RANGE(0x0000, 0x0000) AM_READ(pio_r)
	AM_RANGE(0x0001, 0x0001) AM_READ(keyboard_r)
	// read ranges are incorrect - should be 1/8th of current values.
	//AM_RANGE(0x0080, 0x00bf) AM_DEVREADWRITE("uart1", tms9902_device, cruread, cruwrite) // RS232
	//AM_RANGE(0x0180, 0x01bf) AM_DEVREADWRITE("uart2", tms9902_device, cruread, cruwrite) // Cassette
	//AM_RANGE(0x01c0, 0x01ff) // DMA controller - TMS9911
	//AM_RANGE(0x0800, 0x080f) AM_WRITE(cent_data_w)
	//AM_RANGE(0x0810, 0x0811) AM_WRITE(cent_strobe_w)
	//AM_RANGE(0x0812, 0x0813) AM_READ(cent_stat_r)
	//AM_RANGE(0x1ee0, 0x1eef) AM_READWRITE(cpu_int_r,cpu_int_w)
	//AM_RANGE(0x1fda, 0x1fdb) AM_READWRITE(cpu_int1_r,cpu_int1_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( cortex )
INPUT_PORTS_END

READ8_MEMBER( cortex_state::pio_r )
{
	return (m_cru0005 ? 0x20 : 0) | 0xdf;
}

READ8_MEMBER( cortex_state::keyboard_r )
{
	return m_term_data;
}

WRITE_LINE_MEMBER( cortex_state::keyboard_ack_w )
{
	if (!state)
	{
		m_maincpu->set_input_line(INT_9995_INT4, CLEAR_LINE);
		m_cru0005 = 1;
	}
}

void cortex_state::kbd_put(u8 data)
{
	m_term_data = data;
	m_cru0005 = 0;
	m_maincpu->set_input_line(INT_9995_INT4, ASSERT_LINE);
}

void cortex_state::machine_reset()
{
	m_cru0005 = 1;
	uint8_t* ROM = memregion("maincpu")->base();
	memcpy(m_p_ram, ROM, 0x6000);
	m_maincpu->ready_line(ASSERT_LINE);
}

static MACHINE_CONFIG_START( cortex )
	/* basic machine hardware */
	/* TMS9995 CPU @ 12.0 MHz */
	// Standard variant, no overflow int
	// No lines connected yet
	MCFG_TMS99xx_ADD("maincpu", TMS9995, XTAL_12MHz, mem_map, io_map)

	MCFG_DEVICE_ADD("control", LS259, 0) // IC64
	//MCFG_ADDRESSABLE_LATCH_Q0_OUT_CB(WRITELINE(cortex_state, basic_led_w))
	MCFG_ADDRESSABLE_LATCH_Q1_OUT_CB(WRITELINE(cortex_state, keyboard_ack_w))
	//MCFG_ADDRESSABLE_LATCH_Q2_OUT_CB(WRITELINE(cortex_state, ebus_int_ack_w))
	//MCFG_ADDRESSABLE_LATCH_Q3_OUT_CB(WRITELINE(cortex_state, ebus_to_en_w))
	//MCFG_ADDRESSABLE_LATCH_Q4_OUT_CB(WRITELINE(cortex_state, disk_size_w))
	//MCFG_ADDRESSABLE_LATCH_Q5_OUT_CB(WRITELINE(cortex_state, eprom_on_off_w))
	//MCFG_ADDRESSABLE_LATCH_Q6_OUT_CB(WRITELINE(cortex_state, bell_en_w))

	/* video hardware */
	MCFG_DEVICE_ADD( "tms9928a", TMS9929A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(PUT(cortex_state, kbd_put))

	//MCFG_DEVICE_ADD("uart1", TMS9902, XTAL_12MHz / 4)
	//MCFG_DEVICE_ADD("uart2", TMS9902, XTAL_12MHz / 4)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( cortex )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "basic", "Cortex Bios")
	ROMX_LOAD( "cortex.ic47", 0x0000, 0x2000, CRC(bdb8c7bd) SHA1(340829dcb7a65f2e830fd5aff82a312e3ed7918f), ROM_BIOS(1))
	ROMX_LOAD( "cortex.ic46", 0x2000, 0x2000, CRC(4de459ea) SHA1(00a42fe556d4ffe1f85b2ce369f544b07fbd06d9), ROM_BIOS(1))
	ROMX_LOAD( "cortex.ic45", 0x4000, 0x2000, CRC(b0c9b6e8) SHA1(4e20c3f0b7546b803da4805cd3b8616f96c3d923), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "forth", "FIG-Forth")
	ROMX_LOAD( "forth.ic47",  0x0000, 0x2000, CRC(999034be) SHA1(0dcc7404c38aa0ae913101eb0aa98da82104b5d4), ROM_BIOS(2))
	ROMX_LOAD( "forth.ic46",  0x2000, 0x2000, CRC(8eca54cc) SHA1(0f1680e941ef60bb9bde9a4b843b78f30dff3202), ROM_BIOS(2))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   STATE         INIT  COMPANY                  FULLNAME  FLAGS
COMP( 1982, cortex, 0,      0,       cortex,    cortex, cortex_state, 0,    "Powertran Cybernetics", "Cortex", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
