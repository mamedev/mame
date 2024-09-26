// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

ELZET/K
ELZET/P
BCS

All documentation is in German.

The ELZET/P is a luggable CP/M computer that from the outside looks very
much like a Kaypro II. The floppy drives could be one under the other like
the Kaypro, or could be vertically-orientated side-by-side. The drives are
80 track quad-density with 800KB capacity.
Behind the drives is a cage that can hold up to 8 slots that plug into a
small motherboard.
CARDS: (some are optional)
- Floppy Disk Controller: choice of FDC 2: (basic) or FDC3 (also has 128K RAM)
    (main chips are PIO, DMA, MB8877A)
- Dynamic RAM: choice of 64k or 256k
- Centronics (contains PIO, CTC)
- EIC (contains PIO, DART, Z80B)
- Experimenter board (contains PIO, CTC)
- Video (contains MC6845, 2k vram, 2k attr-ram, chargen roms, 15MHz xtal)
- CPU (contains Z80, SIO, PIO, 4MHz xtal)

The keyboard plugs into the front by using a stereo audio plug, like you
have on a modern computer's line-out jack. There's no internal information
for it.


The ELZET/K is a similar computer but is a slightly smaller form factor.

The BCS is a box that can be used to convert over 300 floppy-disk formats.

***************************************************************************


To Do: Everything.
Status: Just a closet skeleton


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80sio.h"
#include "machine/z80pio.h"
#include "machine/wd_fdc.h"

namespace {

class elzet80_state : public driver_device
{
public:
	elzet80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ctc(*this, "ctc")
		, m_dma(*this, "dma")
		, m_pio(*this, "pio")
		, m_uart(*this, "uart")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		//, m_io_keyboard(*this, "LINE%d", 0U)
	{ }

	void elzet80(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	floppy_image_device *m_floppy = nullptr;
	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dma_device> m_dma;
	required_device<z80pio_device> m_pio;
	required_device<z80sio_device> m_uart;
	required_device<fd1793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	//required_ioport_array<8> m_io_keyboard;
};


void elzet80_state::mem_map(address_map &map)
{
}

void elzet80_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
}

static INPUT_PORTS_START( elzet80 )
INPUT_PORTS_END


void elzet80_state::machine_start()
{
}

void elzet80_state::machine_reset()
{
	m_floppy = nullptr;
}

static void elzet80_floppies(device_slot_interface &device)
{
	device.option_add("fdd", FLOPPY_525_QD);
}


void elzet80_state::elzet80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &elzet80_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &elzet80_state::io_map);

	// devices
	FD1793(config, m_fdc, 1000000);    // unknown where this is derived
	FLOPPY_CONNECTOR(config, "fdc:0", elzet80_floppies, "fdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", elzet80_floppies, "fdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	Z80PIO(config, m_pio, 0);
	Z80SIO(config, m_uart, 0);
	Z80CTC(config, m_ctc, 0);
	Z80DMA(config, m_dma, 0);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START(elzet80k)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "elzet80-k_cpu.bin",                  0x0000, 0x1000, CRC(e5300137) SHA1(5a9fcb52756a2e9008d53b734b874d33b069efb5) )

	ROM_REGION(0x2000, "chargen", 0) // same roms as P?
	ROM_LOAD( "elzet80-p_video80_ic1_prom3.bin",    0x0000, 0x1000, CRC(70008d71) SHA1(048efed186861050f0e6b47fec57149319de22b6) )
	ROM_LOAD( "elzet80-p_video80_ic2_prom4.bin",    0x1000, 0x1000, CRC(8db78b54) SHA1(7d0ce2811ee1f6c179295411fd8c3c2e67ea2842) )

	ROM_REGION(0x1000, "kbd", 0)
	ROM_LOAD( "elzet80-k_tastatur_d8748.bin",       0x0000, 0x0400, CRC(78bf6f1a) SHA1(95919363e73779899cbd7c143d10c245c35ca789) )

	ROM_REGION(0x1000, "bcs", 0)
	ROM_LOAD( "elzet80-bcs_cpuiec.bin",             0x0000, 0x1000, CRC(724fe45a) SHA1(b4b01c9bf11b35c48b0a4839d7530a18259b1ae9) )
ROM_END

ROM_START(elzet80p)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "elzet80-p_cpu.bin",                  0x0000, 0x1000, CRC(8b876e12) SHA1(0fb4dfe267a3fbc033fbce430d13fb22a4ad16ed) )

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "elzet80-p_video80_ic1_prom3.bin",    0x0000, 0x1000, CRC(70008d71) SHA1(048efed186861050f0e6b47fec57149319de22b6) )
	ROM_LOAD( "elzet80-p_video80_ic2_prom4.bin",    0x1000, 0x1000, CRC(8db78b54) SHA1(7d0ce2811ee1f6c179295411fd8c3c2e67ea2842) )

	ROM_REGION(0x1000, "kbd", 0)
	ROM_LOAD( "elzet80-p_tastatur_jk_ti2732.bin",  0x0000, 0x1000, CRC(88be9b29) SHA1(b83603dadce9d7e7367d1782ee78189156fe9a60) )

	ROM_REGION(0x1000, "bcs", 0)
	ROM_LOAD( "elzet80-bcs_cpuiec.bin",             0x0000, 0x1000, CRC(724fe45a) SHA1(b4b01c9bf11b35c48b0a4839d7530a18259b1ae9) )
ROM_END

} // Anonymous namespace

//    YEAR  NAME        PARENT    COMPAT    MACHINE     INPUT        CLASS           INIT             COMPANY          FULLNAME               FLAGS
COMP( 1982, elzet80k,   elzet80p, 0,        elzet80,   elzet80,    elzet80_state, empty_init,     "Giesler & Danne GmbH & Co. KG",  "Elzet/K 80",        MACHINE_IS_SKELETON )
COMP( 1982, elzet80p,   0,        0,        elzet80,   elzet80,    elzet80_state, empty_init,     "Giesler & Danne GmbH & Co. KG",  "Elzet/P 80",        MACHINE_IS_SKELETON )
