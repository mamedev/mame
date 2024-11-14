// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Not much is known about this Hungarian Z80-based machine, not even the name
of the manufacturer. It comes with 64K of RAM, and 2x 5.25 floppy drives.
Video is a CGA card.

Notes found at vcfed:

1988, hungarian unknown, junior80
2x 5.25 FDD DSDD 80T, 64K ram, CGA graphics card, Serial card with 2 ports
 which is in addition to those ports on the main board.
So this are the main chips on the logic board:
Address / Chip
00H-03H I8255
00H - input port for keyboard scan codes
01H - input port for system jumpers and parallel interface extra lines
PB6-7 - extra line parallel port B
PB4-5 - extra line parallel port A
PB3 - 0 - parallel keyboard / 1-serial keyboard
PB2 - 0 - internal console / 1 - serial console
PB1 - disk drive 2,3 - 0=5.25", 1=8"
PB0 - disk drive 0,1 - 0=5.25", 1=8"
02H
PC7 - second channel I8253 - 0 deactivated, 1 activated
PC6 - speaker modulation
PC4-5 - extra lines parallel port B
PC2-3 - extra lines parallel port A
PC1 - activate / deactivate serial keyboard
PC0 - comutation 0-RAM / 1 -ROM
03H command port for I8255

10H-13H Z80A SIO
10H - dataport A
11H - dataport B
12H - command port A
13H - command port B
THE code in ROM programms z80SIO: port A unprogrammed, port B: 8 bits, 2 stop-bits, no parity,
 clockx16 - when the OS is loaded from disk, both channels are programmed identical.

20-23H Z80A CTC
20H - data port - interrupt vector for channel 0 (in 8mhz, out 250khz)
21H - command port data channel 1
22H - command port data channel 2
23H - command port data channel 3
both ROM and OS(from disk) programm the z80A ctc like this:
channel 0: counter with no interrupt
channel 1: counter with interrupt on falling edge of the page signal of the display unit
channel 2: counter with interrupt on falling edge of the keyboard interrupt
channel 3: counter with interrupt on rising edge ot the fdd controller interrupt

30H-38H i8257 DMA
30H - DMA channel 0 - memory address for DMA transfer start
31H - DMA channel 0 - number of bytes to transfer
32H - DMA channel 1 - memory address for DMA transfer start
33H - DMA channel 1 - number of bytes to transfer
34H - DMA channel 2 - memory address for DMA transfer start
35H - DMA channel 2 - number of bytes to transfer
36H - DMA channel 3 - memory address for DMA transfer start
37H - DMA channel 4 - number of bytes to transfer
38H - state command of DMA chip
the only programmed channel is channel 1, used for data transfer between ram and fdd controller

40H-41H - I8272 floppy drive controller
40H - command and state of I8272
41H - dataport
48H - commands for fdd controller
D6,D7 - x
D5 - reset I8272 (0 active)
D4 - type of disk drive (0 - 8", 1- 5.25")
D0-3 - start stop drive motor (drive 0-3)
49H - load the scan address into the hardware in graphics mode
4AH - command port of display controller (discrete logic!)
D6,7 - x
D5 - blinking activated (1 active)
D4 - mode selection 0 = 320x200 1=640x200
D3 - display 0=off 1 = on
D2 - validate acces display ram (16k display ram on board) - 1 active
D1 - 0 - graphics mode / 1 - text mode
D0 - 0 = 40x25, 1= 80x25
4BH - color selection and text page selection for display controller
D6-7 - selection of working text page
D5 - select color palette
D4 - backgroung
D3 - bright
D2 - red
D1 - green
D0 - blue

4CH - address port - row - select the first row displayed in text mode
4DH - selection port of clock for transmit and receive for Z80 SIO
D1-7 - x
D0 - 0= internal clock, 1= external clock
4FH - reset port for serial keyboard - keyboard interrupt is deactivated by writing or reading of 4FH

50H-53H - z80A PIO
50H - data port A
51H - data port B
52H - interrupt vector A
53H - interrupt vector B
programmed in rom and from os like this: port A input with interrupt, port B - output with interrupt

60H-6FH ramdisk

70H-73H - i8253
70H - dataport channel 0
71H - dataport channel 1
72H - dataport channel 2
73H - command port I8253
- i8253 programmed in rom and from os like: channel 0 unprogrammed,
channel 1: clock divider for 9600 bps clock
channel 2: clock divider for sound generator
when the os loads, channel 0 is programmed like channel 1

***************************************************************************


To Do: Almost everything.
Status: Just a closet skeleton


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/i8257.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/z80pio.h"
//#include "bus/rs232/rs232.h"


namespace {

class junior80_state : public driver_device
{
public:
	junior80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ctc(*this, "ctc")
		, m_dma(*this, "dma")
		, m_pio(*this, "pio")
		, m_pit(*this, "pit")
		, m_ppi(*this, "ppi")
		, m_uart(*this, "uart")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		//, m_io_keyboard(*this, "LINE%u", 0U)
	{ }

	void junior80(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void drive_w(offs_t offset, u8 data);

	floppy_image_device *m_floppy = 0;
	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<i8257_device> m_dma;
	required_device<z80pio_device> m_pio;
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_device<z80sio_device> m_uart;
	required_device<i8272a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	//required_ioport_array<8> m_io_keyboard;
};


void junior80_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram();
	map(0x0000, 0x07ff).rom();
}

void junior80_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).mirror(0x0c).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x13).mirror(0x0c).rw(m_uart, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x20, 0x23).mirror(0x0c).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x30, 0x38).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x40, 0x41).mirror(0x06).m(m_fdc, FUNC(i8272a_device::map));
	map(0x48, 0x48).w(FUNC(junior80_state::drive_w));
	//map(0x49, 0x4d) video/graphics control
	//map(0x4f, 0x4f) kbd ack
	map(0x50, 0x53).mirror(0x0c).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	//map(0x60, 0x6f) ramdisk
	map(0x70, 0x73).mirror(0x0c).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}

static INPUT_PORTS_START( junior80 )
INPUT_PORTS_END


/*************************************
 *
 *              Port handlers.
 *
 *************************************/


void junior80_state::drive_w(offs_t offset, u8 data)
{
	m_floppy = nullptr;

	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 1)) m_floppy = m_floppy1->get_device();
	//if (BIT(data, 2)) m_floppy = m_floppy2->get_device();
	//if (BIT(data, 3)) m_floppy = m_floppy3->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->mon_w(0);
		//m_floppy->ss_w(BIT(data, 4));
	}

	//m_fdc->dden_w(!BIT(data, 6));
	m_fdc->set_unscaled_clock(BIT(data, 4) ? 4000000 : 8000000);
	if (!BIT(data, 5))
		m_fdc->reset();
}


/*************************************
 *  Machine              *
 *************************************/

void junior80_state::machine_start()
{
}

void junior80_state::machine_reset()
{
	m_floppy = nullptr;
}

static void junior80_floppies(device_slot_interface &device)
{
	device.option_add("fdd", FLOPPY_525_QD);
}


void junior80_state::junior80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2'500'000);  // 2.5 or 4MHz selectable by jumpers
	m_maincpu->set_addrmap(AS_PROGRAM, &junior80_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &junior80_state::io_map);

	// devices
	I8272A(config, m_fdc, 8_MHz_XTAL / 2);
	FLOPPY_CONNECTOR(config, "fdc:0", junior80_floppies, "fdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", junior80_floppies, "fdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	Z80PIO(config, m_pio, 0);
	Z80SIO(config, m_uart, 0);
	Z80CTC(config, m_ctc, 0);
	PIT8253(config, m_pit, 0);
	I8255(config, m_ppi, 0);

	I8257(config, m_dma, 0);
	//m_dma->out_hrq_cb().set(FUNC(junior80_state::hrq_w));
	//m_dma->in_memr_cb().set(FUNC(junior80_state::memory_r));
	//m_dma->out_memw_cb().set(FUNC(junior80_state::memory_w));
	//m_dma->in_ior_cb<1>().set(m_fdc, FUNC(upd765_device::data_r));
	//m_dma->out_iow_cb<1>().set(m_fdc, FUNC(upd765_device::data_w));
}


/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START(junior80)
	ROM_REGION(0x0800, "maincpu",0)
	ROM_LOAD( "junior80_seria_321-ok.ic46", 0x0000, 0x0800, CRC(07f09842) SHA1(c7591a1006ae59d6353859ca401c57ff6eb1d4ff) )
ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT    COMPAT    MACHINE     INPUT        CLASS           INIT            COMPANY          FULLNAME            FLAGS
COMP( 1988, junior80,   0,        0,        junior80,   junior80,    junior80_state, empty_init,     "<unknown>",     "Junior 80",        MACHINE_IS_SKELETON )
