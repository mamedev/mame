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

LED patterns

     80
    _____
   |     | 40
04 | 02  |
    _____
   |     | 20
08 |     |
    _____
     10
        76543210
PORT A: 111xyzzz

PA4/PA5 are the "enable" for the two LEDs

7seg Display    Bits
'0'  %11111100 $FC
'1'  %01100000 $60
'2'  %11011010 $DA
'3'  %11110010 $F2
'4'  %01100110 $66
'5'  %10110110 $B6
'6'  %10111110 $BE
'7'  %11100000 $E0
'8'  %11111110 $FE
'9'  %11100110 $E6
'A'  %11101110 $EE
'b'  %00111110 $3E
'C'  %10011100 $9C
'd'  %01111010 $7A
'E'  %10011110 $9E
'F'  %10001110 $8E
'L'  %00011100 $1C
'n'  %00101010 $2A
'o'  %00111010 $3A
'P'  %11001110 $CE
'r'  %00001010 $0A
'U' %01111100 $7C
'c' %00011010 $1A
'u' %01111000 $38

***************************************************************************/


#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6850acia.h"
#include "machine/6522via.h"
#include "machine/wd1772.h"
#include "formats/esq8_dsk.h"
#include "formats/mfi_dsk.h"
#include "formats/dfi_dsk.h"
#include "formats/ipf_dsk.h"
#include "sound/es5503.h"

class mirage_state : public driver_device
{
public:
    mirage_state(const machine_config &mconfig, device_type type, const char *tag)
    : driver_device(mconfig, type, tag),
    m_maincpu(*this, "maincpu"),
    m_fdc(*this, "wd1772")
    { }

    required_device<m6809e_device> m_maincpu;
    required_device<wd1772_t> m_fdc;

	virtual void machine_reset();

    int last_sndram_bank;

	static const floppy_format_type floppy_formats[];

    void fdc_intrq_w(bool state);
	DECLARE_DRIVER_INIT(mirage);
	virtual void video_start();
};

const floppy_format_type mirage_state::floppy_formats[] = {
	FLOPPY_ESQ8IMG_FORMAT, FLOPPY_IPF_FORMAT, FLOPPY_MFI_FORMAT, FLOPPY_DFI_FORMAT,
	NULL
};

static SLOT_INTERFACE_START( ensoniq_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

void mirage_state::fdc_intrq_w(bool state)
{
    m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

static void mirage_doc_irq(device_t *device, int state)
{
}

static UINT8 mirage_adc_read(device_t *device)
{
	return 0x80;
}

void mirage_state::video_start()
{
}

static SCREEN_UPDATE_RGB32( mirage )
{
	return 0;
}

void mirage_state::machine_reset()
{
	last_sndram_bank = 0;
	membank("sndbank")->set_base(machine().root_device().memregion("es5503")->base() );
}

static ADDRESS_MAP_START( mirage_map, AS_PROGRAM, 8, mirage_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK("sndbank")	// 32k window on 128k of wave RAM
	AM_RANGE(0x8000, 0xbfff) AM_RAM			// main RAM
    AM_RANGE(0xc000, 0xdfff) AM_RAM         // expansion RAM
	AM_RANGE(0xe100, 0xe100) AM_DEVREADWRITE("acia6850", acia6850_device, status_read, control_write)
	AM_RANGE(0xe101, 0xe101) AM_DEVREADWRITE("acia6850", acia6850_device, data_read, data_write)
	AM_RANGE(0xe200, 0xe2ff) AM_DEVREADWRITE("via6522", via6522_device, read, write)
	AM_RANGE(0xe800, 0xe803) AM_DEVREADWRITE("wd1772", wd1772_t, read, write)
    AM_RANGE(0xec00, 0xecef) AM_DEVREADWRITE("es5503", es5503_device, read, write)
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION("osrom", 0)
ADDRESS_MAP_END

// port A: front panel
static WRITE8_DEVICE_HANDLER( mirage_via_write_porta )
{
//  printf("PORT A: %02x\n", data);
}

// port B:
//  bit 7: OUT UART clock
//  bit 4: OUT disk enable (motor on?)
//  bit 3: OUT sample/play
//  bit 2: OUT mic line/in
//  bit 1: OUT upper/lower bank (64k halves)
//  bit 0: OUT bank 0/bank 1 (32k halves)

static WRITE8_DEVICE_HANDLER( mirage_via_write_portb )
{
	int bank = 0;
    mirage_state *state = device->machine().driver_data<mirage_state>();

	// handle sound RAM bank switching
	bank = (data & 2) ? (64*1024) : 0;
	bank += (data & 1) ? (32*1024) : 0;
	if (bank != state->last_sndram_bank)
	{
		state->last_sndram_bank = bank;
		state->membank("sndbank")->set_base(state->memregion("es5503")->base() + bank);
	}
}

// port A: front panel
static READ8_DEVICE_HANDLER( mirage_via_read_porta )
{
	return 0;
}

// port B:
//  bit 6: IN FDC disk loaded
//  bit 5: IN 5503 sync (?)
static READ8_DEVICE_HANDLER( mirage_via_read_portb )
{
	return 0x60;
}

// external sync pulse
static READ8_DEVICE_HANDLER( mirage_via_read_ca1 )
{
	return 0;
}

// keyscan
static READ8_DEVICE_HANDLER( mirage_via_read_cb1 )
{
	return 0;
}

// keyscan
static READ8_DEVICE_HANDLER( mirage_via_read_ca2 )
{
	return 0;
}


// keyscan
static READ8_DEVICE_HANDLER( mirage_via_read_cb2 )
{
	return 0;
}

const via6522_interface mirage_via =
{
	DEVCB_HANDLER(mirage_via_read_porta),
	DEVCB_HANDLER(mirage_via_read_portb),
	DEVCB_HANDLER(mirage_via_read_ca1),
	DEVCB_HANDLER(mirage_via_read_cb1),
	DEVCB_HANDLER(mirage_via_read_ca2),
	DEVCB_HANDLER(mirage_via_read_cb2),
	DEVCB_HANDLER(mirage_via_write_porta),
	DEVCB_HANDLER(mirage_via_write_portb),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_CPU_INPUT_LINE("maincpu", M6809_IRQ_LINE)
};

static ACIA6850_INTERFACE( mirage_acia6850_interface )
{
	0,				// tx clock
	0,				// rx clock
	DEVCB_NULL,			// rx in
	DEVCB_NULL,			// rx out
	DEVCB_NULL,			// cts in
	DEVCB_NULL,			// rts out
	DEVCB_NULL,			// dcd in
	DEVCB_CPU_INPUT_LINE("maincpu", M6809_FIRQ_LINE)
};

static MACHINE_CONFIG_START( mirage, mirage_state )
	MCFG_CPU_ADD("maincpu", M6809E, 4000000)
	MCFG_CPU_PROGRAM_MAP(mirage_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_STATIC(mirage)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 1, 239)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_ES5503_ADD("es5503", 7000000, mirage_doc_irq, mirage_adc_read)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_VIA6522_ADD("via6522", 1000000, mirage_via)

	MCFG_ACIA6850_ADD("acia6850", mirage_acia6850_interface)

    MCFG_WD1772x_ADD("wd1772", 8000000)
	MCFG_FLOPPY_DRIVE_ADD("fd0", ensoniq_floppies, "35dd", 0, mirage_state::floppy_formats)
MACHINE_CONFIG_END

static INPUT_PORTS_START( mirage )
INPUT_PORTS_END

ROM_START( enmirage )
	ROM_REGION(0x1000, "osrom", 0)
	ROM_LOAD( "mirage.bin",   0x0000, 0x1000, CRC(9fc7553c) SHA1(ec6ea5613eeafd21d8f3a7431a35a6ff16eed56d) )

	ROM_REGION(0x20000, "es5503", ROMREGION_ERASE)
ROM_END

DRIVER_INIT_MEMBER(mirage_state,mirage)
{

    floppy_connector *con = machine().device<floppy_connector>("fd0");
	floppy_image_device *floppy = con ? con->get_device() : 0;
    if (floppy)
    {
        m_fdc->set_floppy(floppy);
        m_fdc->setup_intrq_cb(wd1772_t::line_cb(FUNC(mirage_state::fdc_intrq_w), this));

        floppy->ss_w(0);
    }
}

CONS( 1984, enmirage, 0, 0, mirage, mirage, mirage_state, mirage, "Ensoniq", "Ensoniq Mirage", GAME_NOT_WORKING )

