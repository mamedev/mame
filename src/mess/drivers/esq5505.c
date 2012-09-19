/***************************************************************************

    esq5505.c - Ensoniq ES5505 + ES5510 based synthesizers and samplers

    Ensoniq VFX, VFX-SD, EPS, EPS-16 Plus, SD-1, SD-1 32, and SQ-1 (SQ-1 Plus,
    SQ-2, and KS-32 are known to also be this architecture).

    The Taito sound system in taito_en.c is directly derived from the SQ-1.

    Driver by R. Belmont with thanks to Parduz, Christian Brunschen, and Phil Bennett

    Memory map:

    0x000000-0x00ffff   work RAM low
    0x200000-0x20001f   OTTO(5505) regs
    0x240000-0x2400ff   DMAC (68450) regs (EPS/EPS-16)
    0x260000-0x2601ff   ESP (5510) regs
    0x280000-0x28001f   DUART (68681) regs
    0x2C0000-0x2C0003   Floppy (WD1772) regs (VFX-SD, SD-1, and EPS/EPS-16)
    0x2E0000-0x2FFFFF   Expansion cartridge (VFX, unknown other models)
    0x300000-0x300003   EPS/EPS-16 SCSI (WD33C93, register at 300001, data at 300003)
    0x340000-0x3bffff   EPS/EPS-16 sample RAM
    0xc00000-0xc3ffff   OS ROM
    0xff0000-0xffffff   work RAM hi (may or may not be mirrored with work RAM low)

    Interrupts:
    68681 uses custom vector 0x40 (address 0x100)
    5505 interrupts are on normal autovector IRQ 1

    68681 GPIO lines (from VFX schematics):
    I0: CARTIN (if cartridge is present)

    O0: "AN0"
    O1: "AN1"
    O2: "AN2" (disk side select on EPS/EPS-16)
    O6: To ESPHALT pin on ES5510
    O7: "SACK"


***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/es5506.h"
#include "machine/68681.h"
#include "machine/wd1772.h"
#include "machine/hd63450.h"    // compatible with MC68450, which is what these really have
#include "formats/esq16_dsk.h"
#include "formats/mfi_dsk.h"
#include "formats/dfi_dsk.h"
#include "formats/ipf_dsk.h"

#include "machine/esqvfd.h"

#define GENERIC (0)
#define EPS     (1)
#define SQ1     (2)

#define KEYBOARD_HACK (0)   // turn on to play the SQ-1: Z and X are program up/down, A/S/D/F/G/H/J/K/L and Q/W/E/R/T/Y/U play notes

#if KEYBOARD_HACK
static int program = 0;
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
    required_device<device_t> m_duart;
    optional_device<wd1772_t> m_fdc;
    optional_device<esq1x22_t> m_epsvfd;
    optional_device<esq2x40_sq1_t> m_sq1vfd;
    optional_device<esq2x40_t> m_vfd;
    optional_device<device_t> m_dmac;

    virtual void machine_reset();

    DECLARE_READ16_MEMBER(es5510_dsp_r);
    DECLARE_WRITE16_MEMBER(es5510_dsp_w);
    DECLARE_READ16_MEMBER(mc68681_r);
    DECLARE_WRITE16_MEMBER(mc68681_w);

    int m_system_type;
    UINT8 m_duart_io;
    bool  m_bCalibSecondByte;

	static const floppy_format_type floppy_formats[];

private:
    UINT16  es5510_dsp_ram[0x200];
    UINT32  es5510_gpr[0xc0];
    UINT32  es5510_dram[1<<24];
    UINT32  es5510_dol_latch;
    UINT32  es5510_dil_latch;
    UINT32  es5510_dadr_latch;
    UINT32  es5510_gpr_latch;
    UINT8   es5510_ram_sel;
public:
	DECLARE_DRIVER_INIT(eps);
	DECLARE_DRIVER_INIT(common);
	DECLARE_DRIVER_INIT(sq1);
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);
};

const floppy_format_type esq5505_state::floppy_formats[] = {
	FLOPPY_ESQIMG_FORMAT, FLOPPY_IPF_FORMAT, FLOPPY_MFI_FORMAT, FLOPPY_DFI_FORMAT,
	NULL
};

static SLOT_INTERFACE_START( ensoniq_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

void esq5505_state::machine_reset()
{
	UINT8 *ROM = machine().root_device().memregion("osrom")->base();
	UINT8 *RAM = (UINT8 *)machine().root_device().memshare("osram")->ptr();

    memcpy(RAM, ROM, 256);

    // pick up the new vectors
    m_maincpu->reset();

    m_bCalibSecondByte = false;
}

READ16_MEMBER(esq5505_state::es5510_dsp_r)
{
//  logerror("%06x: DSP read offset %04x (data is %04x)\n",space.device().safe_pc(),offset,es5510_dsp_ram[offset]);

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

static ADDRESS_MAP_START( vfx_map, AS_PROGRAM, 16, esq5505_state )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM AM_MIRROR(0x30000) AM_SHARE("osram")
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE_LEGACY("ensoniq", es5505_r, es5505_w)
	AM_RANGE(0x260000, 0x2601ff) AM_READWRITE(es5510_dsp_r, es5510_dsp_w)
    AM_RANGE(0x280000, 0x28001f) AM_DEVREADWRITE8_LEGACY("duart", duart68681_r, duart68681_w, 0x00ff)
    AM_RANGE(0xc00000, 0xc1ffff) AM_ROM AM_REGION("osrom", 0)
    AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( vfxsd_map, AS_PROGRAM, 16, esq5505_state )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM AM_SHARE("osram") AM_MIRROR(0x30000)
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE_LEGACY("ensoniq", es5505_r, es5505_w)
	AM_RANGE(0x260000, 0x2601ff) AM_READWRITE(es5510_dsp_r, es5510_dsp_w)
    AM_RANGE(0x280000, 0x28001f) AM_DEVREADWRITE8_LEGACY("duart", duart68681_r, duart68681_w, 0x00ff)
    AM_RANGE(0x2c0000, 0x2c0007) AM_DEVREADWRITE8("wd1772", wd1772_t, read, write, 0x00ff)
    AM_RANGE(0x340000, 0x3bffff) AM_RAM // sequencer memory?
    AM_RANGE(0xc00000, 0xc3ffff) AM_ROM AM_REGION("osrom", 0)
    AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( eps_map, AS_PROGRAM, 16, esq5505_state )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM AM_MIRROR(0x30000) AM_SHARE("osram")
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE_LEGACY("ensoniq", es5505_r, es5505_w)
    AM_RANGE(0x240000, 0x2400ff) AM_DEVREADWRITE_LEGACY("mc68450", hd63450_r, hd63450_w)
    AM_RANGE(0x280000, 0x28001f) AM_DEVREADWRITE8_LEGACY("duart", duart68681_r, duart68681_w, 0x00ff)
    AM_RANGE(0x2c0000, 0x2c0007) AM_DEVREADWRITE8("wd1772", wd1772_t, read, write, 0x00ff)
    AM_RANGE(0x580000, 0x7fffff) AM_RAM         // sample RAM?
    AM_RANGE(0xc00000, 0xc0ffff) AM_ROM AM_REGION("osrom", 0)
    AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

static UINT16 esq5505_read_adc(device_t *device)
{
    esq5505_state *state = device->machine().driver_data<esq5505_state>();

    // bit 0 controls reading the battery; other bits likely
    // control other analog sources
    if (state->m_duart_io & 1)
    {
        return 0x5b00;              // vRef
    }
    else
    {
        return 0x7f00;              // vBattery
    }
}

static void duart_irq_handler(device_t *device, int state, UINT8 vector)
{
    esq5505_state *esq5505 = device->machine().driver_data<esq5505_state>();

//    printf("\nDUART IRQ: state %d vector %d\n", state, vector);
    if (state == ASSERT_LINE)
    {
        esq5505->m_maincpu->set_input_line_vector(1, vector);
        esq5505->m_maincpu->set_input_line(1, ASSERT_LINE);
    }
    else
    {
        esq5505->m_maincpu->set_input_line(1, CLEAR_LINE);
    }
};

static UINT8 duart_input(device_t *device)
{
    esq5505_state *state = device->machine().driver_data<esq5505_state>();

    return state->m_duart_io;
}

static void duart_output(device_t *device, UINT8 data)
{
    esq5505_state *state = device->machine().driver_data<esq5505_state>();
	floppy_connector *con = device->machine().device<floppy_connector>("fd0");
	floppy_image_device *floppy = con ? con->get_device() : 0;

    state->m_duart_io = data;

    if (floppy)
    {
        floppy->ss_w((data & 2)>>1);
    }

//    printf("DUART output: %02x (PC=%x)\n", data, state->m_maincpu->pc());
}

static void duart_tx(device_t *device, int channel, UINT8 data)
{
    esq5505_state *state = device->machine().driver_data<esq5505_state>();

    if (channel == 1)
    {
//        printf("ch %d: [%02x] (PC=%x)\n", channel, data, state->m_maincpu->pc());
        switch (state->m_system_type)
        {
            case GENERIC:
                state->m_vfd->write_char(data);
                break;

            case EPS:
                state->m_epsvfd->write_char(data);
                break;

            case SQ1:
                state->m_sq1vfd->write_char(data);
                break;
        }

        if (state->m_bCalibSecondByte)
        {
//            printf("KPC second byte %02x\n", data);
            if (data == 0xfd)   // calibration request
            {
                duart68681_rx_data(state->m_duart, 1, (UINT8)(FPTR)0xff);   // this is the correct response for "calibration OK"
            }
            state->m_bCalibSecondByte = false;
        }
        else if (data == 0xfb)   // request calibration
        {
            state->m_bCalibSecondByte = true;
        }
        else
        {
            // EPS-16+ wants a throwaway reply byte for each byte sent to the KPC
            // VFX-SD and SD-1 definitely don't :)
            if (state->m_system_type == EPS)
            {
                // 0xe7 must respond with any byte that isn't 0xc8 or the ROM dies.
                // 0x71 must respond with anything (return not checked)
                duart68681_rx_data(state->m_duart, 1, (UINT8)(FPTR)0x00);   // actual value of response is never checked
            }
        }
    }
}

static const duart68681_config duart_config =
{
	duart_irq_handler,
	duart_tx,
	duart_input,
	duart_output,

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
    esq5505_state *state = device.machine().driver_data<esq5505_state>();

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

            duart68681_rx_data(m_duart, 0, (UINT8)(FPTR)0xc0); // program change
            duart68681_rx_data(m_duart, 0, program); // program
        }
        else
        {
            duart68681_rx_data(m_duart, 0, (UINT8)(FPTR)0x90); // note on
            duart68681_rx_data(m_duart, 0, (UINT8)(FPTR)param);
            duart68681_rx_data(m_duart, 0, (UINT8)(FPTR)0x7f);
        }
    }
    else if (oldval == 1 && newval == 0)
    {
        if ((UINT8)(FPTR)param != 0x40)
        {
            duart68681_rx_data(m_duart, 0, (UINT8)(FPTR)0x80); // note off
            duart68681_rx_data(m_duart, 0, (UINT8)(FPTR)param);
            duart68681_rx_data(m_duart, 0, (UINT8)(FPTR)0x7f);
        }
    }
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
	NULL,       /* irq */
    esq5505_read_adc
};

static MACHINE_CONFIG_START( vfx, esq5505_state )
	MCFG_CPU_ADD("maincpu", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(vfx_map)

    MCFG_ESQ2x40_ADD("vfd")

	MCFG_DUART68681_ADD("duart", 4000000, duart_config)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5505, 30476100/2)
	MCFG_SOUND_CONFIG(es5505_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(sq1, vfx)
    MCFG_ESQ2x40_REMOVE("vfd")
    MCFG_ESQ2x40_SQ1_ADD("sq1vfd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(eps, vfx)
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(eps_map)

    MCFG_ESQ2x40_REMOVE("vfd")
    MCFG_ESQ1x22_ADD("epsvfd")

    MCFG_WD1772x_ADD("wd1772", 8000000)
	MCFG_FLOPPY_DRIVE_ADD("fd0", ensoniq_floppies, "35dd", 0, esq5505_state::floppy_formats)

	MCFG_HD63450_ADD( "mc68450", dmac_interface )   // MC68450 compatible
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(vfxsd, vfx)
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(vfxsd_map)

    MCFG_WD1772x_ADD("wd1772", 8000000)
	MCFG_FLOPPY_DRIVE_ADD("fd0", ensoniq_floppies, "35dd", 0, esq5505_state::floppy_formats)
MACHINE_CONFIG_END

static INPUT_PORTS_START( vfx )
#if KEYBOARD_HACK
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
    PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('e') PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x4b)
    PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('r') PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x4c)
    PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('t') PORT_CHAR('T') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x4d)
    PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x4e)
    PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('u') PORT_CHAR('U') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x4f)

    PORT_START("KEY1")
    PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x0)
    PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0x1)
#endif
INPUT_PORTS_END

ROM_START( vfx )
    ROM_REGION(0x20000, "osrom", 0)
    ROM_LOAD16_BYTE( "vfx210b-low.bin",  0x000000, 0x010000, CRC(c51b19cd) SHA1(2a125b92ffa02ae9d7fb88118d525491d785e87e) )
    ROM_LOAD16_BYTE( "vfx210b-high.bin", 0x000001, 0x010000, CRC(59853be8) SHA1(8e07f69d53f80885d15f624e0b912aeaf3212ee4) )

    ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
    ROM_LOAD( "u14.bin",  0x000000, 0x080000, NO_DUMP ) // type 234000 on the schematic
    ROM_LOAD( "u15.bin",  0x080000, 0x080000, NO_DUMP )
    ROM_LOAD( "u16.bin",  0x100000, 0x080000, NO_DUMP )
ROM_END

ROM_START( vfxsd )
    ROM_REGION(0x40000, "osrom", 0)
    ROM_LOAD16_BYTE( "vfxsd_200_lower.bin", 0x000000, 0x010000, CRC(7bd31aea) SHA1(812bf73c4861a5d963f128def14a4a98171c93ad) )
    ROM_LOAD16_BYTE( "vfxsd_200_upper.bin", 0x000001, 0x010000, CRC(9a40efa2) SHA1(e38a2a4514519c1573361cb1526139bfcf94e45a) )

    ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
    ROM_LOAD( "u54.bin",  0x000000, 0x010000, NO_DUMP )
    ROM_LOAD( "u55.bin",  0x010000, 0x010000, NO_DUMP )
    ROM_LOAD( "u56.bin",  0x020000, 0x010000, NO_DUMP )
    ROM_LOAD( "u57.bin",  0x030000, 0x080000, NO_DUMP )
    ROM_LOAD( "u58.bin",  0x038000, 0x080000, NO_DUMP )
    ROM_LOAD( "u59.bin",  0x040000, 0x010000, NO_DUMP )
    ROM_LOAD( "u60.bin",  0x050000, 0x080000, NO_DUMP )

    ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00)
ROM_END

ROM_START( sd1 )
    ROM_REGION(0x40000, "osrom", 0)
    ROM_LOAD16_BYTE( "sd1_410_lo.bin", 0x000000, 0x020000, CRC(faa613a6) SHA1(60066765cddfa9d3b5d09057d8f83fb120f4e65e) )
    ROM_LOAD16_BYTE( "sd1_410_hi.bin", 0x000001, 0x010000, CRC(618c0aa8) SHA1(74acf458aa1d04a0a7a0cd5855c49e6855dbd301) )

    ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
    ROM_LOAD( "sd1-waves-1.bin",  0x000000, 0x080000, NO_DUMP )
    ROM_LOAD( "sd1-waves-2.bin",  0x100000, 0x080000, NO_DUMP )
ROM_END

ROM_START( sd132 )
    ROM_REGION(0x40000, "osrom", 0)
    ROM_LOAD16_BYTE( "sd1_32_402_lo.bin", 0x000000, 0x020000, CRC(5da2572b) SHA1(cb6ddd637ed13bfeb40a99df56000479e63fc8ec) )
    ROM_LOAD16_BYTE( "sd1_32_402_hi.bin", 0x000001, 0x010000, CRC(fc45c210) SHA1(23b81ebd9176112e6eae0c7c75b39fcb1656c953) )

    ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
    ROM_LOAD( "sd1-waves-1.bin",  0x000000, 0x080000, NO_DUMP )
    ROM_LOAD( "sd1-waves-2.bin",  0x100000, 0x080000, NO_DUMP )
ROM_END

ROM_START( sq1 )
    ROM_REGION(0x20000, "osrom", 0)
    ROM_LOAD16_BYTE( "esq5505lo.bin",    0x000000, 0x010000, CRC(b004cf05) SHA1(567b0dae2e35b06e39da108f9c041fd9bc38fa35) )
    ROM_LOAD16_BYTE( "esq5505up.bin",    0x000001, 0x010000, CRC(2e927873) SHA1(06a948cb71fa254b23f4b9236f29035d10778da1) )

    ROM_REGION(0x200000, "waverom", 0)
    ROM_LOAD16_BYTE( "sq1-u25.bin",  0x000001, 0x080000, CRC(26312451) SHA1(9f947a11592fd8420fc581914bf16e7ade75390c) )
    ROM_LOAD16_BYTE( "sq1-u26.bin",  0x100001, 0x080000, CRC(2edaa9dc) SHA1(72fead505c4f44e5736ff7d545d72dfa37d613e2) )
ROM_END

ROM_START( eps )
    ROM_REGION(0x10000, "osrom", 0)
    ROM_LOAD16_BYTE( "eps-l.bin",    0x000000, 0x008000, CRC(382beac1) SHA1(110e31edb03fcf7bbde3e17423b21929e5b32db2) )
    ROM_LOAD16_BYTE( "eps-h.bin",    0x000001, 0x008000, CRC(d8747420) SHA1(460597751386eb5f08465699b61381c4acd78065) )

    ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)  // EPS-16 has no ROM sounds
ROM_END

DRIVER_INIT_MEMBER(esq5505_state,common)
{

    m_system_type = GENERIC;
    m_duart_io = 0;

	floppy_connector *con = machine().device<floppy_connector>("fd0");
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

CONS( 1988, eps,   0, 0, eps,   vfx, esq5505_state, eps,    "Ensoniq", "EPS", GAME_NOT_WORKING )   // custom VFD: one alphanumeric 22-char row, one graphics-capable row (alpha row can also do bar graphs)
CONS( 1989, vfx,   0, 0, vfx,   vfx, esq5505_state, common, "Ensoniq", "VFX", GAME_NOT_WORKING )       // 2x40 VFD
CONS( 1989, vfxsd, 0, 0, vfxsd, vfx, esq5505_state, common, "Ensoniq", "VFX-SD", GAME_NOT_WORKING )    // 2x40 VFD
CONS( 1990, sd1,   0, 0, vfxsd, vfx, esq5505_state, common, "Ensoniq", "SD-1", GAME_NOT_WORKING )      // 2x40 VFD
CONS( 1990, sd132, 0, 0, vfxsd, vfx, esq5505_state, common, "Ensoniq", "SD-1 32", GAME_NOT_WORKING )   // 2x40 VFD
CONS( 1990, sq1,   0, 0, sq1,   vfx, esq5505_state, sq1,    "Ensoniq", "SQ-1", GAME_NOT_WORKING )      // LCD of some sort

