/***************************************************************************

    esq5505.c - Ensoniq ES5505 + ES5510 based synthesizers and samplers

    Ensoniq VFX, VFX-SD, EPS, EPS-16 Plus, SD-1, SD-1 32, and SQ-1 (SQ-1 Plus,
    SQ-2, and KS-32 are known to also be this architecture).

    The Taito sound system in taito_en.c is directly derived from the SQ-1.

    Driver by R. Belmont with thanks to Parduz, Christian Brunschen, and Phil Bennett

    Memory map:

    0x000000-0x007fff   work RAM low (64k for SQ-1 and later)
    0x200000-0x20001f   OTIS (5505) regs
    0x240000-0x2400ff   DMAC (68450) regs (EPS/EPS-16)
    0x260000-0x2601ff   ESP (5510) regs
    0x280000-0x28001f   DUART (68681) regs
    0x2C0000-0x2C0003   Floppy (WD1772) regs (VFX-SD, SD-1, and EPS/EPS-16)
    0x2e0000-0x2fffff   Expansion cartridge (VFX, VFX-SD, SD-1, SD-1 32 voice)
    0x300000-0x300003   EPS/EPS-16 SCSI (WD33C93, register at 300001, data at 300003)
    0x330000-0x37ffff   VFX-SD / SD-1 sequencer RAM
    0x340000-0x3bffff   EPS/EPS-16 sample RAM
    0xc00000-0xc3ffff   OS ROM
    0xff8000-0xffffff   work RAM hi (64k for SQ-1 and later)
 
    Note from es5700.pdf PLA equations:
	RAM if (A23/22/21 = 000 and FC is not 6) or (A23/22/21 = 111 and FC is not 7)
	ROM if (A23/22/21 = 110) or (A23/22/21 = 000 & FC is 6) 
 
    Interrupts:
    5505 interrupts are on normal autovector IRQ 1
    DMAC interrupts (EPS only) are on autovector IRQ 2
    68681 uses custom vector 0x40 (address 0x100) level 3

    VFX / VFX-SD / SD-1 / SD-1 32 panel button codes:
    2 = PROGRAM CONTROL
    3 = WRITE
    4 = WAVE
    5 = SELECT VOICE
    6 = MIXER/SHAPER
    7 =  EFFECT
    8 = COMPARE
    9 = COPY EFFECTS PARAMETERS
    10 = LFO
    11 = PITCH
    12 = ENV1
    13 = PITCH MOD
    14 = ENV2
    15 = FILTER
    16 = ENV3
    17 = OUTPUT
    18 = ERROR 20 (VFX) / SEQ. CONTROL
    19 = RECORD
    20 = MASTER
    21 = STORAGE
    22 = STOP/CONT
    23 = PLAY
    24 = MIDI
    25 = BUTTON 9
    26 = PSEL
    27 = STAT
    28 = EFFECT
    29 = SEQ?  (toggles INT0 / TRAX display)
    30 = TRACKS 1-6
    31 = TRACKS 7-12
    32 = ERROR 20 (VFX) / CLICK-REC
    33 = ERROR 20 (VFX) / LOCATE
    34 = BUTTON 8
    35 = BUTTON 7
    36 = VOLUME
    37 = PAN
    38 = TIMBRE
    39 = KEY ZONE
    40 = TRANSPOSE
    41 = RELEASE
    42 = SOFT TOP CENTER
    43 = SOFT TOP RIGHT
    44 = SOFT BOTTOM CENTER
    45 = SOFT BOTTOM RIGHT
    46 = BUTTON 3
    47 = BUTTON 4
    48 = BUTTON 5
    49 = BUTTON 6
    50 = SOFT BOTTOM LEFT
    51 = ERROR 202 (VFX) / SEQ.
    52 = CART
    53 = SOUNDS
    54 = PRESETS
    55 = BUTTON 0
    56 = BUTTON 1
    57 = BUTTON 2
    58 = SOFT TOP LEFT
    59 = ERROR 20 (VFX) / EDIT SEQUENCE
    60 = ERROR 20 (VFX) / EDIT SONG
    61 = ERROR 20 (VFX) / EDIT TRACK
    62 = DATA INCREMENT
    63 = DATA DECREMENT

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/es5506.h"
#include "machine/n68681.h"
#include "machine/wd_fdc.h"
#include "machine/hd63450.h"    // compatible with MC68450, which is what these really have
#include "formats/esq16_dsk.h"

#include "machine/esqvfd.h"

#define GENERIC (0)
#define EPS     (1)
#define SQ1     (2)

#define KEYBOARD_HACK (1)   // turn on to play the SQ-1, SD-1, and SD-1 32-voice: Z and X are program up/down, A/S/D/F/G/H/J/K/L and Q/W/E/R/T/Y/U play notes
#define HACK_VIA_MIDI	(1)

#if KEYBOARD_HACK
#if HACK_VIA_MIDI
static int program = 0;
#else
static int shift = 32;
#endif
#endif

class esq5505_state : public driver_device
{
public:
	esq5505_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
        m_maincpu(*this, "maincpu"),
        m_duart(*this, "duart"),
        m_fdc(*this, "wd1772"),
        m_epsvfd(*this, "epsvfd"),
        m_sq1vfd(*this, "sq1vfd"),
        m_vfd(*this, "vfd"),
        m_dmac(*this, "mc68450")
    { }

    required_device<m68000_device> m_maincpu;
    required_device<duartn68681_device> m_duart;
    optional_device<wd1772_t> m_fdc;
    optional_device<esq1x22_t> m_epsvfd;
    optional_device<esq2x40_sq1_t> m_sq1vfd;
    optional_device<esq2x40_t> m_vfd;
    optional_device<hd63450_device> m_dmac;

    virtual void machine_reset();

    DECLARE_READ16_MEMBER(es5510_dsp_r);
    DECLARE_WRITE16_MEMBER(es5510_dsp_w);
    DECLARE_READ16_MEMBER(mc68681_r);
    DECLARE_WRITE16_MEMBER(mc68681_w);
    DECLARE_READ16_MEMBER(lower_r);
    DECLARE_WRITE16_MEMBER(lower_w);

	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);
	DECLARE_WRITE8_MEMBER(duart_tx_a);
	DECLARE_WRITE8_MEMBER(duart_tx_b);
	DECLARE_READ8_MEMBER(duart_input);
	DECLARE_WRITE8_MEMBER(duart_output);

    int m_system_type;
    UINT8 m_duart_io;
    bool  m_bCalibSecondByte;

	DECLARE_FLOPPY_FORMATS( floppy_formats );

private:
    UINT16  es5510_dsp_ram[0x200];
    UINT32  es5510_gpr[0xc0];
    UINT32  es5510_dram[1<<24];
    UINT32  es5510_dol_latch;
    UINT32  es5510_dil_latch;
    UINT32  es5510_dadr_latch;
    UINT32  es5510_gpr_latch;
    UINT8   es5510_ram_sel;

	UINT16	*m_rom, *m_ram;

public:
	DECLARE_DRIVER_INIT(eps);
	DECLARE_DRIVER_INIT(common);
	DECLARE_DRIVER_INIT(sq1);
	DECLARE_DRIVER_INIT(denib);
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);
};

FLOPPY_FORMATS_MEMBER( esq5505_state::floppy_formats )
    FLOPPY_ESQIMG_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( ensoniq_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

void esq5505_state::machine_reset()
{
	m_rom = (UINT16 *)machine().root_device().memregion("osrom")->base();
	m_ram = (UINT16 *)machine().root_device().memshare("osram")->ptr();

	m_bCalibSecondByte = false;
}

READ16_MEMBER(esq5505_state::es5510_dsp_r)
{
//  printf("%06x: DSP read offset %04x (data is %04x)\n",space.device().safe_pc(),offset,es5510_dsp_ram[offset]);

	// VFX hack
	if (mame_stricmp(space.machine().system().name, "vfx") == 0)
	{
		if (space.device().safe_pc() == 0xc091f0)
		{
			return space.device().state().state_int(M68K_D2);
		}
	}

	switch(offset)
	{
		case 0x09: return (es5510_dil_latch >> 16) & 0xff;
		case 0x0a: return (es5510_dil_latch >> 8) & 0xff;
		case 0x0b: return (es5510_dil_latch >> 0) & 0xff; //TODO: docs says that this always returns 0
	}

	if (offset==0x12) return 0;

	if (offset==0x16) return 0x27;

	return es5510_dsp_ram[offset];
}

WRITE16_MEMBER(esq5505_state::es5510_dsp_w)
{
	UINT8 *snd_mem = (UINT8 *)space.machine().root_device().memregion("waverom")->base();

	COMBINE_DATA(&es5510_dsp_ram[offset]);

	switch (offset) {
		case 0x00: es5510_gpr_latch=(es5510_gpr_latch&0x00ffff)|((data&0xff)<<16); break;
		case 0x01: es5510_gpr_latch=(es5510_gpr_latch&0xff00ff)|((data&0xff)<< 8); break;
		case 0x02: es5510_gpr_latch=(es5510_gpr_latch&0xffff00)|((data&0xff)<< 0); break;

		/* 0x03 to 0x08 INSTR Register */
		/* 0x09 to 0x0b DIL Register (r/o) */

		case 0x0c: es5510_dol_latch=(es5510_dol_latch&0x00ffff)|((data&0xff)<<16); break;
		case 0x0d: es5510_dol_latch=(es5510_dol_latch&0xff00ff)|((data&0xff)<< 8); break;
		case 0x0e: es5510_dol_latch=(es5510_dol_latch&0xffff00)|((data&0xff)<< 0); break; //TODO: docs says that this always returns 0xff

		case 0x0f:
			es5510_dadr_latch=(es5510_dadr_latch&0x00ffff)|((data&0xff)<<16);
			if(es5510_ram_sel)
				es5510_dil_latch = es5510_dram[es5510_dadr_latch];
			else
				es5510_dram[es5510_dadr_latch] = es5510_dol_latch;
			break;

		case 0x10: es5510_dadr_latch=(es5510_dadr_latch&0xff00ff)|((data&0xff)<< 8); break;
		case 0x11: es5510_dadr_latch=(es5510_dadr_latch&0xffff00)|((data&0xff)<< 0); break;

		/* 0x12 Host Control */

		case 0x14: es5510_ram_sel = data & 0x80; /* bit 6 is i/o select, everything else is undefined */break;

		/* 0x16 Program Counter (test purpose, r/o?) */
		/* 0x17 Internal Refresh counter (test purpose) */
		/* 0x18 Host Serial Control */
		/* 0x1f Halt enable (w) / Frame Counter (r) */

		case 0x80: /* Read select - GPR + INSTR */
	//      logerror("ES5510:  Read GPR/INSTR %06x (%06x)\n",data,es5510_gpr[data]);

			/* Check if a GPR is selected */
			if (data<0xc0) {
				//es_tmp=0;
				es5510_gpr_latch=es5510_gpr[data];
			}// else es_tmp=1;
			break;

		case 0xa0: /* Write select - GPR */
	//      logerror("ES5510:  Write GPR %06x %06x (0x%04x:=0x%06x\n",data,es5510_gpr_latch,data,snd_mem[es5510_gpr_latch>>8]);
			if (data<0xc0)
				es5510_gpr[data]=snd_mem[es5510_gpr_latch>>8];
			break;

		case 0xc0: /* Write select - INSTR */
	//      logerror("ES5510:  Write INSTR %06x %06x\n",data,es5510_gpr_latch);
			break;

		case 0xe0: /* Write select - GPR + INSTR */
	//      logerror("ES5510:  Write GPR/INSTR %06x %06x\n",data,es5510_gpr_latch);
			break;
	}
}

READ16_MEMBER(esq5505_state::lower_r)
{
	offset &= 0x7fff;

	// get pointers when 68k resets
	if (!m_rom)
	{
		m_rom = (UINT16 *)machine().root_device().memregion("osrom")->base();
		m_ram = (UINT16 *)machine().root_device().memshare("osram")->ptr();
	}

	if (offset < 0x4000)
	{
		if (m68k_get_fc(m_maincpu) == 0x6)	// supervisor mode = ROM
		{
			return m_rom[offset];
		}
		else
		{
			return m_ram[offset];
		}
	}
	else
	{
		return m_ram[offset];
	}
}

WRITE16_MEMBER(esq5505_state::lower_w)
{
	offset &= 0x7fff;

	if (offset < 0x4000)
	{
		if (m68k_get_fc(m_maincpu) != 0x6)	// if not supervisor mode, RAM
		{
			COMBINE_DATA(&m_ram[offset]);
		}
		else
		{
			logerror("Write to ROM: %x @ %x (fc=%x)\n", data, offset, m68k_get_fc(m_maincpu));
		}
	}
	else
	{
		COMBINE_DATA(&m_ram[offset]);
	}
}

static ADDRESS_MAP_START( vfx_map, AS_PROGRAM, 16, esq5505_state )
	AM_RANGE(0x000000, 0x007fff) AM_READWRITE(lower_r, lower_w)
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE_LEGACY("ensoniq", es5505_r, es5505_w)
	AM_RANGE(0x260000, 0x2601ff) AM_READWRITE(es5510_dsp_r, es5510_dsp_w)
    AM_RANGE(0x280000, 0x28001f) AM_DEVREADWRITE8("duart", duartn68681_device, read, write, 0x00ff)
    AM_RANGE(0xc00000, 0xc1ffff) AM_ROM AM_REGION("osrom", 0)
    AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( vfxsd_map, AS_PROGRAM, 16, esq5505_state )
	AM_RANGE(0x000000, 0x007fff) AM_READWRITE(lower_r, lower_w)
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE_LEGACY("ensoniq", es5505_r, es5505_w)
	AM_RANGE(0x260000, 0x2601ff) AM_READWRITE(es5510_dsp_r, es5510_dsp_w)
    AM_RANGE(0x280000, 0x28001f) AM_DEVREADWRITE8("duart", duartn68681_device, read, write, 0x00ff)
    AM_RANGE(0x2c0000, 0x2c0007) AM_DEVREADWRITE8("wd1772", wd1772_t, read, write, 0x00ff)
    AM_RANGE(0x330000, 0x3bffff) AM_RAM // sequencer memory?
    AM_RANGE(0xc00000, 0xc3ffff) AM_ROM AM_REGION("osrom", 0)
    AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( eps_map, AS_PROGRAM, 16, esq5505_state )
	AM_RANGE(0x000000, 0x007fff) AM_READWRITE(lower_r, lower_w)
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE_LEGACY("ensoniq", es5505_r, es5505_w)
    AM_RANGE(0x240000, 0x2400ff) AM_DEVREADWRITE_LEGACY("mc68450", hd63450_r, hd63450_w)
    AM_RANGE(0x280000, 0x28001f) AM_DEVREADWRITE8("duart", duartn68681_device, read, write, 0x00ff)
    AM_RANGE(0x2c0000, 0x2c0007) AM_DEVREADWRITE8("wd1772", wd1772_t, read, write, 0x00ff)
    AM_RANGE(0x580000, 0x7fffff) AM_RAM         // sample RAM?
    AM_RANGE(0xc00000, 0xc0ffff) AM_ROM AM_REGION("osrom", 0)
    AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sq1_map, AS_PROGRAM, 16, esq5505_state )
	AM_RANGE(0x000000, 0x03ffff) AM_READWRITE(lower_r, lower_w)
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE_LEGACY("ensoniq", es5505_r, es5505_w)
	AM_RANGE(0x260000, 0x2601ff) AM_READWRITE(es5510_dsp_r, es5510_dsp_w)
    AM_RANGE(0x280000, 0x28001f) AM_DEVREADWRITE8("duart", duartn68681_device, read, write, 0x00ff)
    AM_RANGE(0x2c0000, 0x2c0007) AM_DEVREADWRITE8("wd1772", wd1772_t, read, write, 0x00ff)
    AM_RANGE(0x330000, 0x3bffff) AM_RAM // sequencer memory?
    AM_RANGE(0xc00000, 0xc3ffff) AM_ROM AM_REGION("osrom", 0)
    AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

static void esq5505_otis_irq(device_t *device, int state)
{
	#if 0	// 5505/06 IRQ generation needs (more) work
    esq5505_state *esq5505 = device->machine().driver_data<esq5505_state>();
    esq5505->m_maincpu->set_input_line(1, state);
	#endif
}

static UINT16 esq5505_read_adc(device_t *device)
{
    esq5505_state *state = device->machine().driver_data<esq5505_state>();

    // bit 0 controls reading the battery; other bits likely
    // control other analog sources
	// VFX & SD-1 32 voice schematics agree:
	// bit 0: reference
	// bit 1: battery
	// bit 2: vol
	// bit 3: pedal
	// bit 4: val
	// bit 5: mod wheel
	// bit 6: psel
	// bit 7: pitch wheel
	switch ((state->m_duart_io & 7) ^ 7)
	{
		case 0:		// vRef to check battery
			return 0x5b00;

		case 1: 	// battery voltage
			return 0x7f00;

			return 0x7f00;

		case 3: // pedal
		case 5: // mod wheel
			return 0;

		case 7: // pitch wheel
			return 0x3f00;

		case 2:	// volume control
		case 4: // val (?)
		case 6: // psel
			return 0x7f00;
	}

    if (state->m_duart_io & 1)
    {
        return 0x5b00;              // vRef
    }
    else
    {
        return 0x7f00;              // vBattery
    }
}

WRITE_LINE_MEMBER(esq5505_state::duart_irq_handler)
{
//    printf("\nDUART IRQ: state %d vector %d\n", state, vector);
    if (state == ASSERT_LINE)
    {
        m_maincpu->set_input_line_vector(M68K_IRQ_3, m_duart->get_irq_vector());
        m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);
    }
    else
    {
        m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
    }
};

READ8_MEMBER(esq5505_state::duart_input)
{
	floppy_connector *con = machine().device<floppy_connector>("wd1772:0");
	floppy_image_device *floppy = con ? con->get_device() : 0;
	UINT8 result = 0;	// DUART input lines are separate from the output lines

	// on VFX, bit 0 is 1 for 'cartridge present'.
	// on VFX-SD and later, bit 0 is 1 for floppy present, bit 1 is 1 for cartridge present
	if (mame_stricmp(machine().system().name, "vfx") == 0)
	{
		// todo: handle VFX cart-in when we support cartridges
	}
	else
	{
		if (floppy)
		{
			// ready_r returns true if the drive is *not* ready, false if it is
//          if (!floppy->ready_r())
			{
				result |= 1;
			}
		}
	}

    return result;
}

WRITE8_MEMBER(esq5505_state::duart_output)
{
	floppy_connector *con = machine().device<floppy_connector>("wd1772:0");
	floppy_image_device *floppy = con ? con->get_device() : 0;

    m_duart_io = data;

	/*
        EPS:
        bit 2 = SSEL

        VFX:
        bits 0/1/2 = analog sel
        bit 6 = ESPHALT
        bit 7 = SACK (?)

        VFX-SD & SD-1 (32):
        bits 0/1/2 = analog sel
        bit 3 = SSEL (disk side)
        bit 4 = DSEL (drive select?)
        bit 6 = ESPHALT
        bit 7 = SACK (?)
    */

    if (floppy)
    {
		if (m_system_type == EPS)
		{
			floppy->ss_w((data & 2)>>1);
		}
		else
		{
			floppy->ss_w(((data & 8)>>3)^1);
		}
    }

//    printf("DUART output: %02x (PC=%x)\n", data, m_maincpu->pc());
}

// MIDI send, we don't care yet
WRITE8_MEMBER(esq5505_state::duart_tx_a)
{
}

WRITE8_MEMBER(esq5505_state::duart_tx_b)
{
/*	if (data >= 'A' && data <= 'z')
	{
		printf("ch 1: [%02x](%c) (PC=%x)\n", data, data, m_maincpu->pc());
	}
	else
	{
		printf("ch 1: [%02x] (PC=%x)\n", data, m_maincpu->pc());
	}*/

	switch (m_system_type)
	{
		case GENERIC:
			m_vfd->write_char(data);
			break;

		case EPS:
			m_epsvfd->write_char(data);
			break;

		case SQ1:
			m_sq1vfd->write_char(data);
			break;
	}

	if (m_bCalibSecondByte)
	{
		if (data == 0xfd)   // calibration request
		{
			m_duart->duart68681_rx_data(1, (UINT8)(FPTR)0xff);   // this is the correct response for "calibration OK"
		}
		m_bCalibSecondByte = false;
	}
	else if (data == 0xfb)   // request calibration
	{   		
		m_bCalibSecondByte = true;
	}
	else
	{
		// EPS wants a throwaway reply byte for each byte sent to the KPC
		// VFX-SD and SD-1 definitely don't :)
		if (m_system_type == EPS)
		{
			if (data == 0xe7)
			{
				m_duart->duart68681_rx_data(1, (UINT8)(FPTR)0x00);   // actual value of response is never checked
			}
			else if (data == 0x71)
			{
				m_duart->duart68681_rx_data(1, (UINT8)(FPTR)0x00);   // actual value of response is never checked
			}
			else
			{
				m_duart->duart68681_rx_data(1, data);   // actual value of response is never checked
			}
		}
	}
}

static const duartn68681_config duart_config =
{
	DEVCB_DRIVER_LINE_MEMBER(esq5505_state, duart_irq_handler),
	DEVCB_DRIVER_MEMBER(esq5505_state, duart_tx_a),
	DEVCB_DRIVER_MEMBER(esq5505_state, duart_tx_b),
	DEVCB_DRIVER_MEMBER(esq5505_state, duart_input),
	DEVCB_DRIVER_MEMBER(esq5505_state, duart_output),

	500000, 500000,	// IP3, IP4
	1000000, 1000000, // IP5, IP6
};

static void esq_dma_end(running_machine &machine, int channel, int irq)
{
	device_t *device = machine.device("mc68450");

	if (irq != 0)
	{
        printf("DMAC IRQ, vector = %x\n", hd63450_get_vector(device, channel));
	}
}

static void esq_dma_error(running_machine &machine, int channel, int irq)
{
	device_t *device = machine.device("mc68450");
	if(irq != 0)
	{
        printf("DMAC error, vector = %x\n", hd63450_get_error_vector(device, channel));
	}
}

static int esq_fdc_read_byte(running_machine &machine, int addr)
{
    esq5505_state *state = machine.driver_data<esq5505_state>();

	return state->m_fdc->data_r();
}

static void esq_fdc_write_byte(running_machine &machine, int addr, int data)
{
    esq5505_state *state = machine.driver_data<esq5505_state>();
    state->m_fdc->data_w(data & 0xff);
}

#if KEYBOARD_HACK
INPUT_CHANGED_MEMBER(esq5505_state::key_stroke)
{
    #if HACK_VIA_MIDI
    // send a MIDI Note On
    if (oldval == 0 && newval == 1)
    {
        if ((UINT8)(FPTR)param < 0x40)
        {
            int code = (int)(FPTR)param;

            if (code == 0)
            {
                program--;
                if (program < 0)
                {
                    program = 0;
                }
                printf("program to %d\n", program);
            }
            if (code == 1)
            {
                program++;
                if (program > 127)
                {
                    program = 127;
                }
                printf("program to %d\n", program);
            }

			m_duart->duart68681_rx_data(0, (UINT8)(FPTR)0xc0); // program change
			m_duart->duart68681_rx_data(0, program); // program
        }
        else
        {
			m_duart->duart68681_rx_data(0, (UINT8)(FPTR)0x90); // note on
			m_duart->duart68681_rx_data(0, (UINT8)(FPTR)param);
			m_duart->duart68681_rx_data(0, (UINT8)(FPTR)0x7f);
        }
    }
    else if (oldval == 1 && newval == 0)
    {
        if ((UINT8)(FPTR)param != 0x40)
        {
			m_duart->duart68681_rx_data(0, (UINT8)(FPTR)0x80); // note off
			m_duart->duart68681_rx_data(0, (UINT8)(FPTR)param);
			m_duart->duart68681_rx_data(0, (UINT8)(FPTR)0x7f);
        }
    }
    #else
	int val = (UINT8)(FPTR)param;

	if (val < 0x60)
	{
		if (oldval == 0 && newval == 1)
		{
			if (val == 0 && shift > 0)
			{
				shift -= 32;
				printf("New shift %d\n", shift);
			}
			else if (val == 1 && shift < 32)
			{
				shift += 32;
				printf("New shift %d\n", shift);
			}
		}
	}
	else
	{
		val += shift;
		if (oldval == 0 && newval == 1)
		{
			printf("key pressed %d\n", val&0x7f);
			m_duart->duart68681_rx_data(1, val);
			m_duart->duart68681_rx_data(1, 0x00);
		}
		else if (oldval == 1 && newval == 0)
		{
	//        printf("key off %x\n", (UINT8)(FPTR)param);
			m_duart->duart68681_rx_data(1, val&0x7f);
			m_duart->duart68681_rx_data(1, 0x00);
		}
	}
    #endif
}
#endif

static const hd63450_intf dmac_interface =
{
	"maincpu",  // CPU - 68000
	{attotime::from_usec(32),attotime::from_nsec(450),attotime::from_usec(4),attotime::from_hz(15625/2)},  // Cycle steal mode timing (guesstimate)
	{attotime::from_usec(32),attotime::from_nsec(450),attotime::from_nsec(50),attotime::from_nsec(50)}, // Burst mode timing (guesstimate)
	esq_dma_end,
	esq_dma_error,
    { esq_fdc_read_byte, 0, 0, 0 },     // ch 0 = fdc, ch 1 = 340001 (ADC?)
    { esq_fdc_write_byte, 0, 0, 0 }
};

static const es5505_interface es5505_config =
{
	"waverom",	/* Bank 0 */
	"waverom2",	/* Bank 1 */
	esq5505_otis_irq, /* irq */
    esq5505_read_adc
};

static MACHINE_CONFIG_START( vfx, esq5505_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(vfx_map)

    MCFG_ESQ2x40_ADD("vfd")

	MCFG_DUARTN68681_ADD("duart", 4000000, duart_config)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5505, XTAL_10MHz)
	MCFG_SOUND_CONFIG(es5505_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 2.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(eps, vfx)
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(eps_map)

    MCFG_ESQ2x40_REMOVE("vfd")
    MCFG_ESQ1x22_ADD("epsvfd")

    MCFG_WD1772x_ADD("wd1772", 8000000)
	MCFG_FLOPPY_DRIVE_ADD("wd1772:0", ensoniq_floppies, "35dd", 0, esq5505_state::floppy_formats)

	MCFG_HD63450_ADD( "mc68450", dmac_interface )   // MC68450 compatible
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(vfxsd, vfx)
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(vfxsd_map)

    MCFG_WD1772x_ADD("wd1772", 8000000)
	MCFG_FLOPPY_DRIVE_ADD("wd1772:0", ensoniq_floppies, "35dd", 0, esq5505_state::floppy_formats)
MACHINE_CONFIG_END

// 32-voice machines with the VFX-SD type config
static MACHINE_CONFIG_START(vfx32, esq5505_state)
	MCFG_CPU_ADD("maincpu", M68000, XTAL_30_4761MHz / 2)
	MCFG_CPU_PROGRAM_MAP(vfxsd_map)

    MCFG_ESQ2x40_ADD("vfd")

	MCFG_DUARTN68681_ADD("duart", 4000000, duart_config)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5505, XTAL_30_4761MHz / 2)
	MCFG_SOUND_CONFIG(es5505_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 2.0)

    MCFG_WD1772x_ADD("wd1772", 8000000)
	MCFG_FLOPPY_DRIVE_ADD("wd1772:0", ensoniq_floppies, "35dd", 0, esq5505_state::floppy_formats)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(sq1, vfx32)
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(sq1_map)

	MCFG_ESQ2x40_REMOVE("vfd")
    MCFG_ESQ2x40_SQ1_ADD("sq1vfd")
MACHINE_CONFIG_END

static INPUT_PORTS_START( vfx )
#if KEYBOARD_HACK
#if HACK_VIA_MIDI
    PORT_START("KEY0")
    PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x40)
    PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x41)
    PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x42)
    PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x43)
    PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x44)
    PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x45)
    PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x46)
    PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x47)
    PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x48)
    PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x49)
    PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x4a)
    PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x4b)
    PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x4c)
    PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x4d)
    PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x4e)
    PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x4f)

    PORT_START("KEY1")
    PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x0)
    PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x1)
#else
    PORT_START("KEY0")
    PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x80)
    PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x81)
    PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x82)
    PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x83)
    PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x84)
    PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x85)
    PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x86)
    PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x87)
    PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x88)
    PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x89)
    PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x8a)
    PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x8b)
    PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x8c)
    PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x8d)
    PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x8e)
    PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x8f)

    PORT_START("KEY1")
    PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x90)
    PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x91)
    PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x92)
    PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x93)
    PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x94)
    PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x95)
    PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x96)
    PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x97)
    PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x98)
    PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x99)
    PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x9a)
    PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x9b)
    PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x9c)
    PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x9d)
    PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x9e)
    PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x9f)

    PORT_START("KEY2")
    PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0)
    PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 1)
#endif
#endif
INPUT_PORTS_END

ROM_START( vfx )
    ROM_REGION(0x40000, "osrom", 0)
    ROM_LOAD16_BYTE( "vfx210b-low.bin",  0x000000, 0x010000, CRC(c51b19cd) SHA1(2a125b92ffa02ae9d7fb88118d525491d785e87e) )
    ROM_LOAD16_BYTE( "vfx210b-high.bin", 0x000001, 0x010000, CRC(59853be8) SHA1(8e07f69d53f80885d15f624e0b912aeaf3212ee4) )

    ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
    ROM_LOAD16_BYTE( "u14.bin", 0x000001, 0x080000, CRC(85592299) SHA1(1aa7cf612f91972baeba15991d9686ccde01599c) )
    ROM_LOAD16_BYTE( "u15.bin", 0x100001, 0x080000, CRC(c0055975) SHA1(5a22f1d5e437c6277eb0cfb1ff1b3f8dcdea1cc6) )

    ROM_REGION(0x80000, "nibbles", ROMREGION_ERASE00)
    ROM_LOAD( "u16.bin", 0x000000, 0x080000, CRC(c3ddaf95) SHA1(44a7bd89cd7e82952cc5100479e110c385246559) )
ROM_END

ROM_START( vfxsd )
    ROM_REGION(0x40000, "osrom", 0)
    ROM_LOAD16_BYTE( "vfxsd_200_lower.bin", 0x000000, 0x010000, CRC(7bd31aea) SHA1(812bf73c4861a5d963f128def14a4a98171c93ad) )
    ROM_LOAD16_BYTE( "vfxsd_200_upper.bin", 0x000001, 0x010000, CRC(9a40efa2) SHA1(e38a2a4514519c1573361cb1526139bfcf94e45a) )

    ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
    ROM_LOAD16_BYTE( "u57.bin", 0x000001, 0x080000, CRC(85592299) SHA1(1aa7cf612f91972baeba15991d9686ccde01599c) )
    ROM_LOAD16_BYTE( "u58.bin", 0x100001, 0x080000, CRC(c0055975) SHA1(5a22f1d5e437c6277eb0cfb1ff1b3f8dcdea1cc6) )

    ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00)

    ROM_REGION(0x80000, "nibbles", 0)
    ROM_LOAD( "u60.bin", 0x000000, 0x080000, CRC(c3ddaf95) SHA1(44a7bd89cd7e82952cc5100479e110c385246559) )
ROM_END

ROM_START( sd1 )
    ROM_REGION(0x40000, "osrom", 0)
    ROM_LOAD16_BYTE( "sd1_410_lo.bin", 0x000000, 0x020000, CRC(faa613a6) SHA1(60066765cddfa9d3b5d09057d8f83fb120f4e65e) )
    ROM_LOAD16_BYTE( "sd1_410_hi.bin", 0x000001, 0x010000, CRC(618c0aa8) SHA1(74acf458aa1d04a0a7a0cd5855c49e6855dbd301) )

    ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)  // BS=0 region (12-bit)
    ROM_LOAD16_BYTE( "u34.bin", 0x000001, 0x080000, CRC(85592299) SHA1(1aa7cf612f91972baeba15991d9686ccde01599c) )
    ROM_LOAD16_BYTE( "u35.bin", 0x100001, 0x080000, CRC(c0055975) SHA1(5a22f1d5e437c6277eb0cfb1ff1b3f8dcdea1cc6) )

    ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00) // BS=1 region (16-bit)
	ROM_LOAD16_WORD_SWAP( "u38.bin", 0x000000, 0x100000, CRC(a904190e) SHA1(e4fd4e1130906086fb4182dcb8b51269969e2836) )
	ROM_LOAD16_WORD_SWAP( "u37.bin", 0x100000, 0x100000, CRC(d706cef3) SHA1(24ba35248509e9ca45110e2402b8085006ea0cfc) )

    ROM_REGION(0x80000, "nibbles", 0)
    ROM_LOAD( "u36.bin", 0x000000, 0x080000, CRC(c3ddaf95) SHA1(44a7bd89cd7e82952cc5100479e110c385246559) )
ROM_END

ROM_START( sd132 )
    ROM_REGION(0x40000, "osrom", 0)
    ROM_LOAD16_BYTE( "sd1_32_402_lo.bin", 0x000000, 0x020000, CRC(5da2572b) SHA1(cb6ddd637ed13bfeb40a99df56000479e63fc8ec) )
    ROM_LOAD16_BYTE( "sd1_32_402_hi.bin", 0x000001, 0x010000, CRC(fc45c210) SHA1(23b81ebd9176112e6eae0c7c75b39fcb1656c953) )

    ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)  // BS=0 region (12-bit)
    ROM_LOAD16_BYTE( "u34.bin", 0x000001, 0x080000, CRC(85592299) SHA1(1aa7cf612f91972baeba15991d9686ccde01599c) )
    ROM_LOAD16_BYTE( "u35.bin", 0x100001, 0x080000, CRC(c0055975) SHA1(5a22f1d5e437c6277eb0cfb1ff1b3f8dcdea1cc6) )

    ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00) // BS=1 region (16-bit)
	ROM_LOAD16_WORD_SWAP( "u38.bin", 0x000000, 0x100000, CRC(a904190e) SHA1(e4fd4e1130906086fb4182dcb8b51269969e2836) )
	ROM_LOAD16_WORD_SWAP( "u37.bin", 0x100000, 0x100000, CRC(d706cef3) SHA1(24ba35248509e9ca45110e2402b8085006ea0cfc) )

    ROM_REGION(0x80000, "nibbles", ROMREGION_ERASE00)
    ROM_LOAD( "u36.bin", 0x000000, 0x080000, CRC(c3ddaf95) SHA1(44a7bd89cd7e82952cc5100479e110c385246559) )
ROM_END


ROM_START( sq1 )
    ROM_REGION(0x40000, "osrom", 0)
    ROM_LOAD16_BYTE( "sq1lo.bin",    0x000000, 0x010000, CRC(b004cf05) SHA1(567b0dae2e35b06e39da108f9c041fd9bc38fa35) )
    ROM_LOAD16_BYTE( "sq1up.bin",    0x000001, 0x010000, CRC(2e927873) SHA1(06a948cb71fa254b23f4b9236f29035d10778da1) )

    ROM_REGION(0x200000, "waverom", 0)
    ROM_LOAD16_BYTE( "sq1-u25.bin",  0x000001, 0x080000, CRC(26312451) SHA1(9f947a11592fd8420fc581914bf16e7ade75390c) )
    ROM_LOAD16_BYTE( "sq1-u26.bin",  0x100001, 0x080000, CRC(2edaa9dc) SHA1(72fead505c4f44e5736ff7d545d72dfa37d613e2) )

    ROM_REGION(0x80000, "nibbles", ROMREGION_ERASE00)
ROM_END

ROM_START( sqrack )
	ROM_REGION(0x40000, "osrom", 0)
	ROM_LOAD16_BYTE( "sqr-102-lower.bin", 0x000000, 0x010000, CRC(186c85ad) SHA1(801c5cf82823ce31a88688fbee4c11ea5ffdbc10) ) 
	ROM_LOAD16_BYTE( "sqr-102-upper.bin", 0x000001, 0x010000, CRC(088c9d31) SHA1(30627f21d893888b6159c481bea08e3eedd21902) ) 

	ROM_REGION(0x200000, "waverom", 0)
	ROM_LOAD16_BYTE( "sq1-u25.bin",  0x000001, 0x080000, CRC(26312451) SHA1(9f947a11592fd8420fc581914bf16e7ade75390c) )
	ROM_LOAD16_BYTE( "sq1-u26.bin",  0x100001, 0x080000, CRC(2edaa9dc) SHA1(72fead505c4f44e5736ff7d545d72dfa37d613e2) )

	ROM_REGION(0x80000, "nibbles", ROMREGION_ERASE00)
ROM_END

ROM_START( eps )
    ROM_REGION(0x10000, "osrom", 0)
    ROM_LOAD16_BYTE( "eps-l.bin",    0x000000, 0x008000, CRC(382beac1) SHA1(110e31edb03fcf7bbde3e17423b21929e5b32db2) )
    ROM_LOAD16_BYTE( "eps-h.bin",    0x000001, 0x008000, CRC(d8747420) SHA1(460597751386eb5f08465699b61381c4acd78065) )

    ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)  // EPS-16 has no ROM sounds

    ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00)
ROM_END

DRIVER_INIT_MEMBER(esq5505_state,common)
{
    m_system_type = GENERIC;
    m_duart_io = 0;

	floppy_connector *con = machine().device<floppy_connector>("wd1772:0");
	floppy_image_device *floppy = con ? con->get_device() : 0;
    if (floppy)
    {
        m_fdc->set_floppy(floppy);
        floppy->ss_w(0);
    }
}

DRIVER_INIT_MEMBER(esq5505_state,eps)
{

    DRIVER_INIT_CALL(common);
    m_system_type = EPS;
}

DRIVER_INIT_MEMBER(esq5505_state,sq1)
{

    DRIVER_INIT_CALL(common);
    m_system_type = SQ1;
}

DRIVER_INIT_MEMBER(esq5505_state,denib)
{
    UINT8 *pNibbles = (UINT8 *)machine().root_device().memregion("nibbles")->base();
    UINT8 *pBS0L = (UINT8 *)machine().root_device().memregion("waverom")->base();
	UINT8 *pBS0H = pBS0L + 0x100000;

    DRIVER_INIT_CALL(common);

    // create the 12 bit samples by patching in the nibbles from the nibble ROM
	// low nibbles go with the lower ROM, high nibbles with the upper ROM
    for (int i = 0; i < 0x80000; i++)
    {
        *pBS0L = (*pNibbles & 0x0f) << 4;
        *pBS0H = (*pNibbles & 0xf0);
        pBS0L += 2;
        pBS0H += 2;
        pNibbles++;
    }
}

CONS( 1988, eps,   0, 0,   eps,   vfx, esq5505_state, eps,    "Ensoniq", "EPS", GAME_NOT_WORKING )   // custom VFD: one alphanumeric 22-char row, one graphics-capable row (alpha row can also do bar graphs)
CONS( 1989, vfx,   0, 0,   vfx,   vfx, esq5505_state, denib,  "Ensoniq", "VFX", GAME_NOT_WORKING )       // 2x40 VFD
CONS( 1989, vfxsd, 0, 0,   vfxsd, vfx, esq5505_state, denib,  "Ensoniq", "VFX-SD", GAME_NOT_WORKING )    // 2x40 VFD
CONS( 1990, sd1,   0, 0,   vfxsd, vfx, esq5505_state, denib,  "Ensoniq", "SD-1", GAME_NOT_WORKING )      // 2x40 VFD
CONS( 1990, sd132, sd1, 0, vfx32, vfx, esq5505_state, denib,  "Ensoniq", "SD-1 32", GAME_NOT_WORKING )   // 2x40 VFD
CONS( 1990, sq1,   0, 0,   sq1,   vfx, esq5505_state, sq1,    "Ensoniq", "SQ-1", GAME_NOT_WORKING )      // 2x16 LCD
CONS( 1990, sqrack,sq1, 0, sq1,   vfx, esq5505_state, sq1,    "Ensoniq", "SQ-Rack", GAME_NOT_WORKING )   // 2x16 LCD                      

