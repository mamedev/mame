// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    drivers/mirage.c

    Ensoniq Mirage Sampler
    Preliminary driver by R. Belmont

    Map for Mirage:
    0000-7fff: 32k window on 128k of sample RAM
    8000-bfff: main RAM
    c000-dfff: optional expansion RAM
    e100-e101: 6850 UART (for MIDI)
    e200-e2ff: 6522 VIA
    e408-e40f: filter cut-off frequency
    e410-e417: filter resonance
    e418-e41f: multiplexer address pre-set
    e800-e803: WD1770 FDC
    ec00-ecef: ES5503 "DOC" sound chip
    f000-ffff: boot ROM

    NMI: IRQ from WD1772
    IRQ: DRQ from WD1772 wire-ORed with IRQ from ES5503 wire-ORed with IRQ from VIA6522
    FIRQ: IRQ from 6850 UART

    LED / switch matrix:

            A           B           C             D         E         F         G        DP
    ROW 0:  LOAD UPPER  LOAD LOWER  SAMPLE UPPER  PLAY SEQ  LOAD SEQ  SAVE SEQ  REC SEQ  SAMPLE LOWER
    ROW 1:  3           6           9             5         8         0         2        Enter
    ROW 2:  1           4           7             up arrow  PARAM     dn arrow  VALUE    CANCEL
    L. AN:  SEG A       SEG B       SEG C         SEG D     SEG E     SEG F     SEG G    SEG DP (decimal point)
    R. AN:  SEG A       SEG B       SEG C         SEG D     SEG E     SEG F     SEG G    SEG DP

    Column number in VIA port A bits 0-2 is converted to discrete lines by a 74LS145.
    Port A bit 3 is right anode, bit 4 is left anode
    ROW 0 is read on VIA port A bit 5, ROW 1 in port A bit 6, and ROW 2 in port A bit 7.

    Keyboard models talk to the R6500 through the VIA shifter: CA2 is handshake, CB1 is shift clock, CB2 is shift data.
    This is unconnected on the rackmount version.

***************************************************************************/


#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "imagedev/floppy.h"
#include "machine/6850acia.h"
#include "machine/6522via.h"
#include "machine/wd_fdc.h"
#include "formats/esq8_dsk.h"
#include "sound/es5503.h"
#include "video/pwm.h"
#include "speaker.h"

#include "mirage.lh"

class enmirage_state : public driver_device
{
public:
	enmirage_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_fdc(*this, "wd1772"),
		m_floppy_connector(*this, "wd1772:0"),
		m_via(*this, "via6522")
	{
	}

	void mirage(machine_config &config);

	void init_mirage();

private:

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	DECLARE_WRITE8_MEMBER(mirage_via_write_porta);
	DECLARE_WRITE8_MEMBER(mirage_via_write_portb);
	DECLARE_WRITE_LINE_MEMBER(mirage_doc_irq);
	DECLARE_READ8_MEMBER(mirage_adc_read);

	void mirage_map(address_map &map);

	virtual void machine_reset() override;

	required_device<mc6809e_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<wd1772_device> m_fdc;
	required_device<floppy_connector> m_floppy_connector;
	required_device<via6522_device> m_via;

	int last_sndram_bank;
};

FLOPPY_FORMATS_MEMBER( enmirage_state::floppy_formats )
	FLOPPY_ESQ8IMG_FORMAT
FLOPPY_FORMATS_END

static void ensoniq_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

WRITE_LINE_MEMBER(enmirage_state::mirage_doc_irq)
{
//    m_maincpu->set_input_line(M6809_IRQ_LINE, state);
}

READ8_MEMBER(enmirage_state::mirage_adc_read)
{
	return 0x00;
}

void enmirage_state::machine_reset()
{
	last_sndram_bank = 0;
	membank("sndbank")->set_base(memregion("es5503")->base() );
}

void enmirage_state::mirage_map(address_map &map)
{
	map(0x0000, 0x7fff).bankrw("sndbank");  // 32k window on 128k of wave RAM
	map(0x8000, 0xbfff).ram();         // main RAM
	map(0xc000, 0xdfff).ram();         // expansion RAM
	map(0xe100, 0xe101).rw("acia6850", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xe200, 0xe2ff).m(m_via, FUNC(via6522_device::map));
	map(0xe400, 0xe4ff).noprw();
	map(0xe800, 0xe803).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write));
	map(0xec00, 0xecef).rw("es5503", FUNC(es5503_device::read), FUNC(es5503_device::write));
	map(0xf000, 0xffff).rom().region("osrom", 0);
}

// port A: front panel
// bits 0-2: column select from 0-7
// bits 3/4 = right and left LED enable
// bits 5/6/7 keypad rows 0/1/2 return
WRITE8_MEMBER(enmirage_state::mirage_via_write_porta)
{
	u8 segdata = data & 7;
	m_display->matrix(((data >> 3) & 3) ^ 3, (1<<segdata));
}

// port B:
//  bit 7: OUT UART clock
//  bit 4: OUT disk enable (motor on?)
//  bit 3: OUT sample/play
//  bit 2: OUT mic line/in
//  bit 1: OUT upper/lower bank (64k halves)
//  bit 0: OUT bank 0/bank 1 (32k halves)

WRITE8_MEMBER(enmirage_state::mirage_via_write_portb)
{
	int bank = 0;

	// handle sound RAM bank switching
	bank = (data & 2) ? (64*1024) : 0;
	bank += (data & 1) ? (32*1024) : 0;
	if (bank != last_sndram_bank)
	{
		last_sndram_bank = bank;
		membank("sndbank")->set_base(memregion("es5503")->base() + bank);
	}
}

void enmirage_state::mirage(machine_config &config)
{
	MC6809E(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &enmirage_state::mirage_map);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	es5503_device &es5503(ES5503(config, "es5503", 7000000));
	es5503.set_channels(2);
	es5503.irq_func().set(FUNC(enmirage_state::mirage_doc_irq));
	es5503.adc_func().set(FUNC(enmirage_state::mirage_adc_read));
	es5503.add_route(0, "lspeaker", 1.0);
	es5503.add_route(1, "rspeaker", 1.0);

	VIA6522(config, m_via, 1000000);
	m_via->writepa_handler().set(FUNC(enmirage_state::mirage_via_write_porta));
	m_via->writepb_handler().set(FUNC(enmirage_state::mirage_via_write_portb));
	m_via->irq_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	PWM_DISPLAY(config, m_display).set_size(2, 8);
	m_display->set_segmask(0x3, 0xff);
	config.set_default_layout(layout_mirage);

	acia6850_device &acia6850(ACIA6850(config, "acia6850", 0));
	acia6850.irq_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);

	WD1772(config, m_fdc, 8000000);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, M6809_IRQ_LINE);

	FLOPPY_CONNECTOR(config, "wd1772:0", ensoniq_floppies, "35dd", enmirage_state::floppy_formats);
}

static INPUT_PORTS_START( mirage )
INPUT_PORTS_END

ROM_START( enmirage )
	ROM_REGION(0x1000, "osrom", 0)
	ROM_LOAD( "mirage.bin",   0x0000, 0x1000, CRC(9fc7553c) SHA1(ec6ea5613eeafd21d8f3a7431a35a6ff16eed56d) )

	ROM_REGION(0x20000, "es5503", ROMREGION_ERASE)
ROM_END

void enmirage_state::init_mirage()
{
	floppy_image_device *floppy = m_floppy_connector ? m_floppy_connector->get_device() : nullptr;
	if (floppy)
	{
		m_fdc->set_floppy(floppy);

		floppy->ss_w(0);
	}

	// port A: front panel
	m_via->write_pa0(0);
	m_via->write_pa1(0);
	m_via->write_pa2(0);
	m_via->write_pa3(0);
	m_via->write_pa4(0);
	m_via->write_pa5(0);
	m_via->write_pa6(0);
	m_via->write_pa7(0);

	// port B:
	//  bit 6: IN FDC disk ready
	//  bit 5: IN 5503 sync (?)
	m_via->write_pb0(0);
	m_via->write_pb1(0);
	m_via->write_pb2(0);
	m_via->write_pb3(0);
	m_via->write_pb4(0);
	m_via->write_pb5(1);
	m_via->write_pb6(0);    // how to determine if a disk is inserted?
	m_via->write_pb7(0);
}

CONS( 1984, enmirage, 0, 0, mirage, mirage, enmirage_state, init_mirage, "Ensoniq", "Mirage", MACHINE_NOT_WORKING )
