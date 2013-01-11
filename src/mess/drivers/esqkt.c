/***************************************************************************

    esqkt.c - Ensoniq KT-76, KT-88, and E-Prime

    Driver by R. Belmont

    Hardware:
        CPU: 68EC020-16 CPU
        Serial/timers: SCN2681 (MC68681 clone)
        Sound: 2xES5506
        Effects: ES5510

    Memory map:

    0x000000-0x07FFFF   OS ROM
    0x200000-0x20003F   Master ES5506
    0x240000-0x24003F   Slave ES5506
    0x280000-0x2801FF   ES5510
    0x300000-0x30000F   68681 DUART
    0xFF0000-0xFFFFFF   OS RAM

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/es5506.h"
#include "machine/68681.h"

#include "machine/esqvfd.h"

#define KEYBOARD_HACK (1)   // turn on to play: Z and X are program up/down, A/S/D/F/G/H/J/K/L and Q/W/E/R/T/Y/U play notes
#define HACK_VIA_MIDI (1)

#if KEYBOARD_HACK
#if HACK_VIA_MIDI
static int program = 0;
#else
static int shift = 32;
#endif
#endif

class esqkt_state : public driver_device
{
public:
	esqkt_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_duart(*this, "duart"),
		m_sq1vfd(*this, "sq1vfd")
	{ }

	required_device<m68ec020_device> m_maincpu;
	required_device<duart68681_device> m_duart;
	required_device<esq2x40_sq1_t> m_sq1vfd;

	virtual void machine_reset();

	DECLARE_READ16_MEMBER(es5510_dsp_r);
	DECLARE_WRITE16_MEMBER(es5510_dsp_w);
	DECLARE_READ16_MEMBER(mc68681_r);
	DECLARE_WRITE16_MEMBER(mc68681_w);
	DECLARE_READ32_MEMBER(lower_r);
	DECLARE_WRITE32_MEMBER(lower_w);

	UINT8 m_duart_io;
	bool  m_bCalibSecondByte;

private:
	UINT16  es5510_dsp_ram[0x200];
	UINT32  es5510_gpr[0xc0];
	UINT32  es5510_dram[1<<24];
	UINT32  es5510_dol_latch;
	UINT32  es5510_dil_latch;
	UINT32  es5510_dadr_latch;
	UINT32  es5510_gpr_latch;
	UINT8   es5510_ram_sel;

	UINT32  *m_rom, *m_ram;

public:
	DECLARE_DRIVER_INIT(kt);
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);
};

void esqkt_state::machine_reset()
{
//  m_rom = (UINT32 *)machine().root_device().memregion("osrom")->base();
//  m_ram = (UINT32 *)machine().root_device().memshare("osram")->ptr();

//  memcpy(m_ram, m_rom, 8);
//  m_maincpu->reset();

	m_bCalibSecondByte = false;
}

READ16_MEMBER(esqkt_state::es5510_dsp_r)
{
//  printf("%06x: DSP read offset %04x (data is %04x)\n",space.device().safe_pc(),offset,es5510_dsp_ram[offset]);

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

WRITE16_MEMBER(esqkt_state::es5510_dsp_w)
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

READ32_MEMBER(esqkt_state::lower_r)
{
	offset &= 0x3fff;

	// get pointers when 68k resets
	if (!m_rom)
	{
		m_rom = (UINT32 *)machine().root_device().memregion("osrom")->base();
		m_ram = (UINT32 *)machine().root_device().memshare("osram")->ptr();
	}

	if (offset < 0x2000)
	{
		if (m68k_get_fc(m_maincpu) == 0x6)  // supervisor mode = ROM
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

WRITE32_MEMBER(esqkt_state::lower_w)
{
	offset &= 0x3fff;

	if (offset < 0x2000)
	{
		if (m68k_get_fc(m_maincpu) != 0x6)  // if not supervisor mode, RAM
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

static ADDRESS_MAP_START( kt_map, AS_PROGRAM, 32, esqkt_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM AM_REGION("osrom", 0)
	AM_RANGE(0x200000, 0x20003f) AM_DEVREADWRITE8_LEGACY("ensoniq", es5506_r, es5506_w, 0xffffffff)
	AM_RANGE(0x240000, 0x24003f) AM_DEVREADWRITE8_LEGACY("ensoniq2", es5506_r, es5506_w, 0xffffffff)
	AM_RANGE(0x280000, 0x2801ff) AM_READWRITE16(es5510_dsp_r, es5510_dsp_w, 0xffffffff)
	AM_RANGE(0x300000, 0x30000f) AM_DEVREADWRITE8_LEGACY("duart", duart68681_r, duart68681_w, 0xffffffff)
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

static void esq5506_otto_irq(device_t *device, int state)
{
	#if 0   // 5505/06 IRQ generation needs (more) work
	esqkt_state *esq5505 = device->machine().driver_data<esqkt_state>();
	esq5505->m_maincpu->set_input_line(1, state);
	#endif
}

static UINT16 esq5506_read_adc(device_t *device)
{
	esqkt_state *state = device->machine().driver_data<esqkt_state>();

	switch ((state->m_duart_io & 7) ^ 7)
	{
		case 0:     // vRef to check battery
			return 0x5b00;

		case 2:     // battery voltage
			return 0x7f00;

		default: // pedal
			return 0;
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

static void duart_irq_handler(device_t *device, int state, UINT8 vector)
{
	esqkt_state *esq5505 = device->machine().driver_data<esqkt_state>();

	esq5505->m_maincpu->set_input_line(M68K_IRQ_3, state);
};

static UINT8 duart_input(device_t *device)
{
	UINT8 result = 0;   // DUART input lines are separate from the output lines

	return result;
}

static void duart_output(device_t *device, UINT8 data)
{
	esqkt_state *state = device->machine().driver_data<esqkt_state>();

	state->m_duart_io = data;

//    printf("DUART output: %02x (PC=%x)\n", data, state->m_maincpu->pc());
}

static void duart_tx(device_t *device, int channel, UINT8 data)
{
	esqkt_state *state = device->machine().driver_data<esqkt_state>();

	if (channel == 1)
	{
//        printf("ch %d: [%02x] (PC=%x)\n", channel, data, state->m_maincpu->pc());

		state->m_sq1vfd->write_char(data);

		if (state->m_bCalibSecondByte)
		{
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
	}
}

static const duart68681_config duart_config =
{
	duart_irq_handler,
	duart_tx,
	duart_input,
	duart_output,

	1000000, 500000,    // IP3, IP4
	500000, 1000000, // IP5, IP6
};

#if KEYBOARD_HACK
INPUT_CHANGED_MEMBER(esqkt_state::key_stroke)
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
			duart68681_rx_data(m_duart, 1, val);
			duart68681_rx_data(m_duart, 1, 0x00);
		}
		else if (oldval == 1 && newval == 0)
		{
	//        printf("key off %x\n", (UINT8)(FPTR)param);
			duart68681_rx_data(m_duart, 1, val&0x7f);
			duart68681_rx_data(m_duart, 1, 0x00);
		}
	}
	#endif
}
#endif

static const es5506_interface es5506_config =
{
	"waverom",  /* Bank 0 */
	"waverom2", /* Bank 1 */
	"waverom3", /* Bank 0 */
	"waverom4", /* Bank 1 */
	esq5506_otto_irq, /* irq */
	esq5506_read_adc
};

static const es5506_interface es5506_2_config =
{
	"waverom",  /* Bank 0 */
	"waverom2", /* Bank 1 */
	"waverom3", /* Bank 0 */
	"waverom4", /* Bank 1 */
	NULL,
	NULL
};

static MACHINE_CONFIG_START( kt, esqkt_state )
	MCFG_CPU_ADD("maincpu", M68EC020, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(kt_map)

	MCFG_ESQ2x40_SQ1_ADD("sq1vfd")

	MCFG_DUART68681_ADD("duart", 4000000, duart_config)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5506, XTAL_16MHz)
	MCFG_SOUND_CONFIG(es5506_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 2.0)
	MCFG_SOUND_ADD("ensoniq2", ES5506, XTAL_16MHz)
	MCFG_SOUND_CONFIG(es5506_2_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 2.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 2.0)
MACHINE_CONFIG_END

static INPUT_PORTS_START( kt )
#if KEYBOARD_HACK
#if HACK_VIA_MIDI
	PORT_START("KEY0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x40)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x41)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x42)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x43)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x44)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x45)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x46)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x47)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x48)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x49)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x4a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x4b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x4c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x4d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x4e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x4f)

	PORT_START("KEY1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x0)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x1)
#else
	PORT_START("KEY0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x80)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x81)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x82)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x83)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x84)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x85)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x86)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x87)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x88)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x89)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x8a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x8b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x8c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x8d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x8e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x8f)

	PORT_START("KEY1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x90)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x91)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x92)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x93)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x94)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x95)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x96)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x97)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x98)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x99)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x9a)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x9b)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x9c)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x9d)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x9e)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0x9f)

	PORT_START("KEY2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 0)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, esqkt_state, key_stroke, 1)
#endif
#endif
INPUT_PORTS_END

ROM_START( kt76 )
	ROM_REGION(0x80000, "osrom", 0)
	ROM_LOAD32_WORD( "kt76_162_lo.bin", 0x000000, 0x020000, CRC(1a1ab910) SHA1(dcc80db2297fd25993e090c2e5bb7f947319a8bf) )
	ROM_LOAD32_WORD( "kt76_162_hi.bin", 0x000002, 0x040000, CRC(de16d236) SHA1(c55fca86453e90e8c34a048bed45817063237370) )

	ROM_REGION(0x200000, "waverom", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom2", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom3", ROMREGION_ERASE00)
	ROM_REGION(0x200000, "waverom4", ROMREGION_ERASE00)
ROM_END

DRIVER_INIT_MEMBER(esqkt_state, kt)
{
	m_duart_io = 0;
}

CONS( 1996, kt76, 0, 0, kt, kt, esqkt_state, kt, "Ensoniq", "KT-76", GAME_NOT_WORKING )
