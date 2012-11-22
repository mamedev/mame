/***************************************************************************

    drivers/esq1.c

    Ensoniq ESQ-1 Digital Wave Synthesizer
    Ensoniq SQ-80 Cross Wave Synthesizer
    Driver by R. Belmont

    Map for ESQ-1 and ESQ-m:
    0000-1fff: OS RAM
    2000-3fff: Cartridge
    4000-5fff: SEQRAM
    6000-63ff: ES5503 DOC
    6400-67ff: MC2681 DUART
    6800-6fff: AD7524 (CV_MUX)
    7000-7fff: OS ROM low (banked)
    8000-ffff: OS ROM high (fixed)

    Map for SQ-80:
    0000-1fff: OS RAM
    2000-3fff: Cartridge
    4000-5fff: DOSRAM or SEQRAM (banked)
    6000-63ff: ES5503 DOC
    6400-67ff: MC2681 DUART
    6800-6bff: AD7524 (CV_MUX)
    6c00-6dff: Mapper (bit 0 only - determines DOSRAM or SEQRAM at 4000)
    6e00-6fff: WD1772 FDC (not present on ESQ1)
    7000-7fff: OS ROM low (banked)
    8000-ffff: OS ROM high (fixed)

    CV_MUX area:
    write to        output goes to
    $68f8   $00     D/A converter
    $68f0   -$08    Filter Frequency (FF)
    $68e8   -$10    Filter Resonance (Q)
    $68d8   -$20    Final DCA (ENV4)
    $68b8   -$40    Panning (PAN)
    $6878   -$80    Floppy (Motor/LED on - SQ-80 only)

If SEQRAM is mapped at 4000, DUART port 2 determines the 32KB "master bank" and ports 0 and 1
determine which of the 4 8KB "sub banks" is visible.

Output ports 3 to 1 determine the 4kB page which should be shown at $7000 to $7fff.

IRQ sources are the DUART and the DRQ line from the FDC (SQ-80 only).
NMI is from the IRQ line on the FDC (again, SQ-80 only).

TODO:
    - VFD display
    - Keyboard
    - Analog filters and VCA on the back end of the 5503
    - SQ-80 support (additional banking, FDC)

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/es5503.h"
#include "machine/68681.h"
#include "machine/wd1772.h"

#include "machine/esqvfd.h"

#define WD1772_TAG		"wd1772"

#define KEYBOARD_HACK   (0)

class esq1_state : public driver_device
{
public:
	esq1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
        m_maincpu(*this, "maincpu"),
        m_duart(*this, "duart"),
        m_fdc(*this, WD1772_TAG),
        m_vfd(*this, "vfd")
    { }

    required_device<device_t> m_maincpu;
    required_device<device_t> m_duart;
    optional_device<wd1772_t> m_fdc;
    optional_device<esq2x40_t> m_vfd;

    DECLARE_READ8_MEMBER(wd1772_r);
    DECLARE_WRITE8_MEMBER(wd1772_w);
    DECLARE_READ8_MEMBER(seqdosram_r);
    DECLARE_WRITE8_MEMBER(seqdosram_w);
    DECLARE_WRITE8_MEMBER(mapper_w);

    int m_mapper_state;
    int m_seq_bank;
    UINT8 m_seqram[0x10000];
    UINT8 m_dosram[0x2000];
	virtual void machine_reset();
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);
};


static void esq1_doc_irq(device_t *device, int state)
{
}

static UINT8 esq1_adc_read(device_t *device)
{
	return 0x80;
}

void esq1_state::machine_reset()
{
	// set default OSROM banking
	membank("osbank")->set_base(machine().root_device().memregion("osrom")->base() );

    m_mapper_state = 0;
    m_seq_bank = 0;
}

READ8_MEMBER(esq1_state::wd1772_r)
{
    return m_fdc->read(space, offset&3);
}

WRITE8_MEMBER(esq1_state::wd1772_w)
{
    m_fdc->write(space, offset&3, data);
}

WRITE8_MEMBER(esq1_state::mapper_w)
{
    m_mapper_state = (data & 1) ^ 1;

//    printf("mapper_state = %d\n", data ^ 1);
}

READ8_MEMBER(esq1_state::seqdosram_r)
{
    if (m_mapper_state)
    {
        return m_dosram[offset];
    }
    else
    {
        return m_seqram[offset + m_seq_bank];
    }
}

WRITE8_MEMBER(esq1_state::seqdosram_w)
{
    if (m_mapper_state)
    {
        m_dosram[offset] = data;
    }
    else
    {
        m_seqram[offset + m_seq_bank] = data;
    }
}

static ADDRESS_MAP_START( esq1_map, AS_PROGRAM, 8, esq1_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM					// OSRAM
	AM_RANGE(0x4000, 0x5fff) AM_RAM					// SEQRAM
	AM_RANGE(0x6000, 0x63ff) AM_DEVREADWRITE("es5503", es5503_device, read, write)
	AM_RANGE(0x6400, 0x640f) AM_DEVREADWRITE_LEGACY("duart", duart68681_r, duart68681_w)
	AM_RANGE(0x7000, 0x7fff) AM_ROMBANK("osbank")
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("osrom", 0x8000)	// OS "high" ROM is always mapped here
ADDRESS_MAP_END

static ADDRESS_MAP_START( sq80_map, AS_PROGRAM, 8, esq1_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM					// OSRAM
	AM_RANGE(0x4000, 0x5fff) AM_RAM					// SEQRAM
//  AM_RANGE(0x4000, 0x5fff) AM_READWRITE(seqdosram_r, seqdosram_w)
	AM_RANGE(0x6000, 0x63ff) AM_DEVREADWRITE("es5503", es5503_device, read, write)
	AM_RANGE(0x6400, 0x640f) AM_DEVREADWRITE_LEGACY("duart", duart68681_r, duart68681_w)
    AM_RANGE(0x6c00, 0x6dff) AM_WRITE(mapper_w)
    AM_RANGE(0x6e00, 0x6fff) AM_READWRITE(wd1772_r, wd1772_w)
	AM_RANGE(0x7000, 0x7fff) AM_ROMBANK("osbank")
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("osrom", 0x8000)	// OS "high" ROM is always mapped here
ADDRESS_MAP_END

// from the schematics:
//
// DUART channel A is MIDI
// channel B is to the keyboard/display
// IP0 = tape in
// IP1 = sequencer expansion cartridge inserted
// IP2 = patch cartridge inserted
// IP3 & 4 are 0.5 MHz, IP 5 & 6 are 1 MHz (note 0.5 MHz / 16 = MIDI baud rate)
//
// OP0 = to display processor
// OP1/2/3 = bank select 0, 1, and 2
// OP4 = metronome low
// OP5 = metronome hi
// OP6/7 = tape out

static void duart_irq_handler(device_t *device, int state, UINT8 vector)
{
	device->machine().device("maincpu")->execute().set_input_line(0, state);
};

static UINT8 duart_input(device_t *device)
{
	return 0;
}

static void duart_output(device_t *device, UINT8 data)
{
	int bank = ((data >> 1) & 0x7);
	esq1_state *state = device->machine().driver_data<esq1_state>();
//  printf("DP [%02x]: %d mlo %d mhi %d tape %d\n", data, data&1, (data>>4)&1, (data>>5)&1, (data>>6)&3);
//  printf("[%02x] bank %d => offset %x (PC=%x)\n", data, bank, bank * 0x1000, device->machine().firstcpu->safe_pc());
    state->membank("osbank")->set_base(state->memregion("osrom")->base() + (bank * 0x1000) );

    state->m_seq_bank = (data & 0x8) ? 0x8000 : 0x0000;
    state->m_seq_bank += ((data>>1) & 3) * 0x2000;
//    printf("seqram_bank = %x\n", state->m_seq_bank);
}

static void duart_tx(device_t *device, int channel, UINT8 data)
{
    esq1_state *state = device->machine().driver_data<esq1_state>();

	if (channel == 1)
    {
        #if 0
        if (data >= 0x20 && data <= 0x7f)
        {
            printf("%c", data);
        }
        else
        {
            printf("[%02x]", data);
        }
        #endif

        state->m_vfd->write_char(data);
    }
}

#if KEYBOARD_HACK
INPUT_CHANGED_MEMBER(esq1_state::key_stroke)
{
    if (oldval == 0 && newval == 1)
    {
        printf("key on %02x\n", (int)(FPTR)param);
        duart68681_rx_data(m_duart, 1, (UINT8)(FPTR)param);
    }
    else if (oldval == 1 && newval == 0)
    {
        printf("key off %02x\n", (int)(FPTR)param);
//        duart68681_rx_data(m_duart, 1, (UINT8)(FPTR)param-0x40);
    }
}
#endif

static const duart68681_config duart_config =
{
	duart_irq_handler,
	duart_tx,
	duart_input,
	duart_output,

	500000, 500000,	// IP3, IP4
	1000000, 1000000, // IP5, IP6
};

static MACHINE_CONFIG_START( esq1, esq1_state )
	MCFG_CPU_ADD("maincpu", M6809E, 4000000)	// how fast is it?
	MCFG_CPU_PROGRAM_MAP(esq1_map)


	MCFG_DUART68681_ADD("duart", 4000000, duart_config)

    MCFG_ESQ2x40_ADD("vfd")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_ES5503_ADD("es5503", 7000000, esq1_doc_irq, esq1_adc_read)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(sq80, esq1)
    MCFG_CPU_MODIFY("maincpu")
    MCFG_CPU_PROGRAM_MAP(sq80_map)

	MCFG_WD1772x_ADD(WD1772_TAG, 4000000)
MACHINE_CONFIG_END

static INPUT_PORTS_START( esq1 )
    #if KEYBOARD_HACK
    PORT_START("KEY0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) 			PORT_CHAR('a') PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x80)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) 			PORT_CHAR('s') PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x81)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) 			PORT_CHAR('d') PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x82)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) 			PORT_CHAR('f') PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x83)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) 			PORT_CHAR('g') PORT_CHAR('G') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x84)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) 			PORT_CHAR('h') PORT_CHAR('H') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x85)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) 			PORT_CHAR('j') PORT_CHAR('J') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x86)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) 			PORT_CHAR('k') PORT_CHAR('K') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x87)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) 			PORT_CHAR('l') PORT_CHAR('L') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x88)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) 			PORT_CHAR('q') PORT_CHAR('Q') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x89)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x8a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) 			PORT_CHAR('e') PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x8b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) 			PORT_CHAR('r') PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x8c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) 			PORT_CHAR('t') PORT_CHAR('T') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x8d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) 			PORT_CHAR('y') PORT_CHAR('Y') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x8e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) 			PORT_CHAR('u') PORT_CHAR('U') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x8f)

	PORT_START("KEY1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) 			PORT_CHAR('1') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x0c)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) 			PORT_CHAR('2') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x0d)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) 			PORT_CHAR('3') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x0e)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) 			PORT_CHAR('4') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x15)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) 			PORT_CHAR('5') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x0f)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) 			PORT_CHAR('6') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x10)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) 			PORT_CHAR('7') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x11)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) 			PORT_CHAR('8') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x12)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) 			PORT_CHAR('9') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x13)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) 			PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, esq1_state, key_stroke, 0x14)
    #endif
INPUT_PORTS_END

ROM_START( esq1 )
    ROM_REGION(0x10000, "osrom", 0)
    ROM_LOAD( "3p5lo.bin",    0x0000, 0x8000, CRC(ed001ad8) SHA1(14d1150bccdbc15d90567cf1812aacdb3b6ee882) )
    ROM_LOAD( "3p5hi.bin",    0x8000, 0x8000, CRC(332c572f) SHA1(ddb4f62807eb2ab29e5ac6b5d209d2ecc74cf806) )

    ROM_REGION(0x20000, "es5503", 0)
    ROM_LOAD( "esq1wavlo.bin", 0x0000, 0x8000, CRC(4d04ac87) SHA1(867b51229b0a82c886bf3b216aa8893748236d8b) )
    ROM_LOAD( "esq1wavhi.bin", 0x8000, 0x8000, CRC(94c554a3) SHA1(ed0318e5253637585559e8cf24c06d6115bd18f6) )
ROM_END

ROM_START( sq80 )
    ROM_REGION(0x10000, "osrom", 0)
    ROM_LOAD( "sq80rom.low",  0x0000, 0x008000, CRC(97ecd9a0) SHA1(cadff16ebbc15b52cf1d3335d22dc930d430a058) )
    ROM_LOAD( "sq80rom.hig",  0x8000, 0x008000, CRC(f83962b1) SHA1(e3e5cf41f15a37f8bf29b88fb1c85c0fca9ea912) )

    ROM_REGION(0x40000, "es5503", 0)
    ROM_LOAD( "2202.bin",     0x0000, 0x010000, CRC(dffd538c) SHA1(e90f6ff3a7804b54c8a3b1b574ec9c223a6c2bf9) )
    ROM_LOAD( "2203.bin",     0x0000, 0x010000, CRC(9be8cceb) SHA1(1ee4d7e6d2171b44e88e464071bdc4b800b69c4a) )
    ROM_LOAD( "2204.bin",     0x0000, 0x010000, CRC(4937c6f7) SHA1(4505efb9b28fe6d4bcc1f79e81a70bb215c399cb) )
    ROM_LOAD( "2205.bin",     0x0000, 0x010000, CRC(0f917d40) SHA1(1cfae9c80088f4c90b3c9e0b284c3b91f7ff61b9) )

    ROM_REGION(0x8000, "kpc", 0)    // 68HC11 keyboard/front panel processor
    ROM_LOAD( "sq80_kpc_150.bin", 0x000000, 0x008000, CRC(8170b728) SHA1(3ad68bb03948e51b20d2e54309baa5c02a468f7c) )
ROM_END

CONS( 1986, esq1, 0   , 0, esq1, esq1, driver_device, 0, "Ensoniq", "ESQ-1", GAME_NOT_WORKING )
CONS( 1988, sq80, 0,    0, sq80, esq1, driver_device, 0, "Ensoniq", "SQ-80", GAME_NOT_WORKING )
