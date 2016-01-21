// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Aaron Giles, R. Belmont, hap, Philip Bennett
/***************************************************************************

    Taito Ensoniq ES5505-based sound hardware

    TODO:

    * Implement ES5510 ESP
    * Where does the MB8421 go? Taito F3(and Super Chase) have 2 of them on
      the sound area, Taito JC has one.

****************************************************************************/

#include "emu.h"
#include "taito_en.h"


const device_type TAITO_EN = &device_creator<taito_en_device>;

taito_en_device::taito_en_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TAITO_EN, "Taito Ensoniq Sound System", tag, owner, clock, "taito_en", __FILE__),
	m_audiocpu(*this, ":audiocpu"),
	m_ensoniq(*this, ":ensoniq"),
	m_duart68681(*this, ":duart68681"),
	m_mb87078(*this, ":mb87078"),
	m_snd_shared_ram(*this, ":snd_shared"),
	m_es5510_dol_latch(0),
	m_es5510_dil_latch(0),
	m_es5510_dadr_latch(0),
	m_es5510_gpr_latch(0),
	m_es5510_ram_sel(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void taito_en_device::device_start()
{
	save_item(NAME(m_es5510_dsp_ram));
	save_item(NAME(m_es5510_gpr));
	save_item(NAME(m_es5510_dram));
	save_item(NAME(m_es5510_dol_latch));
	save_item(NAME(m_es5510_dil_latch));
	save_item(NAME(m_es5510_dadr_latch));
	save_item(NAME(m_es5510_gpr_latch));
	save_item(NAME(m_es5510_ram_sel));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void taito_en_device::device_reset()
{
	/* Sound cpu program loads to 0xc00000 so we use a bank */
	UINT16 *ROM = (UINT16 *)machine().root_device().memregion("audiocpu")->base();
	UINT16 *sound_ram = (UINT16 *)machine().root_device().memshare("share1")->ptr();
	machine().root_device().membank("bank1")->set_base(&ROM[0x80000]);
	machine().root_device().membank("bank2")->set_base(&ROM[0x90000]);
	machine().root_device().membank("bank3")->set_base(&ROM[0xa0000]);

	sound_ram[0] = ROM[0x80000]; /* Stack and Reset vectors */
	sound_ram[1] = ROM[0x80001];
	sound_ram[2] = ROM[0x80002];
	sound_ram[3] = ROM[0x80003];

	/* reset CPU to catch any banking of startup vectors */
	m_audiocpu->reset();
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


/*************************************
 *
 *  Handlers
 *
 *************************************/

READ8_MEMBER( taito_en_device::en_68000_share_r )
{
	switch (offset & 3)
	{
		case 0: return (m_snd_shared_ram[offset/4]&0xff000000)>>24;
		case 1: return (m_snd_shared_ram[offset/4]&0x00ff0000)>>16;
		case 2: return (m_snd_shared_ram[offset/4]&0x0000ff00)>>8;
		case 3: return (m_snd_shared_ram[offset/4]&0x000000ff)>>0;
	}

	return 0;
}

WRITE8_MEMBER( taito_en_device::en_68000_share_w )
{
	switch (offset & 3)
	{
		case 0: m_snd_shared_ram[offset/4] = (m_snd_shared_ram[offset/4]&0x00ffffff)|(data<<24);
		case 1: m_snd_shared_ram[offset/4] = (m_snd_shared_ram[offset/4]&0xff00ffff)|(data<<16);
		case 2: m_snd_shared_ram[offset/4] = (m_snd_shared_ram[offset/4]&0xffff00ff)|(data<<8);
		case 3: m_snd_shared_ram[offset/4] = (m_snd_shared_ram[offset/4]&0xffffff00)|(data<<0);
	}
}

WRITE16_MEMBER( taito_en_device::en_es5505_bank_w )
{
	UINT32 max_banks_this_game = (space.machine().root_device().memregion("ensoniq.0")->bytes()/0x200000)-1;

	/* mask out unused bits */
	data &= max_banks_this_game;
	m_ensoniq->voice_bank_w(offset,data<<20);
}

WRITE8_MEMBER( taito_en_device::en_volume_w )
{
	m_mb87078->data_w(data, offset ^ 1);
}


/*************************************
 *
 *  ES5510
 *
 *************************************/

//todo: hook up cpu/es5510

READ16_MEMBER( taito_en_device::es5510_dsp_r )
{
	switch (offset)
	{
		case 0x09: return (m_es5510_dil_latch >> 16) & 0xff;
		case 0x0a: return (m_es5510_dil_latch >> 8) & 0xff;
		case 0x0b: return (m_es5510_dil_latch >> 0) & 0xff; // TODO: docs says that this always returns 0
		default: break;
	}

	if (offset==0x12) return 0;

//  if (offset>4)
	if (offset==0x16) return 0x27;

	return m_es5510_dsp_ram[offset];
}

WRITE16_MEMBER( taito_en_device::es5510_dsp_w )
{
	UINT8 *snd_mem = (UINT8 *)space.machine().root_device().memregion("ensoniq.0")->base();

//  if (offset>4 && offset!=0x80  && offset!=0xa0  && offset!=0xc0  && offset!=0xe0)
//      logerror("%06x: DSP write offset %04x %04x\n",space.device().safe_pc(),offset,data);

	COMBINE_DATA(&m_es5510_dsp_ram[offset]);

	switch (offset) {
		case 0x00: m_es5510_gpr_latch=(m_es5510_gpr_latch&0x00ffff)|((data&0xff)<<16); break;
		case 0x01: m_es5510_gpr_latch=(m_es5510_gpr_latch&0xff00ff)|((data&0xff)<< 8); break;
		case 0x02: m_es5510_gpr_latch=(m_es5510_gpr_latch&0xffff00)|((data&0xff)<< 0); break;

		/* 0x03 to 0x08 INSTR Register */
		/* 0x09 to 0x0b DIL Register (r/o) */

		case 0x0c: m_es5510_dol_latch=(m_es5510_dol_latch&0x00ffff)|((data&0xff)<<16); break;
		case 0x0d: m_es5510_dol_latch=(m_es5510_dol_latch&0xff00ff)|((data&0xff)<< 8); break;
		case 0x0e: m_es5510_dol_latch=(m_es5510_dol_latch&0xffff00)|((data&0xff)<< 0); break; //TODO: docs says that this always returns 0xff

		case 0x0f:
			m_es5510_dadr_latch=(m_es5510_dadr_latch&0x00ffff)|((data&0xff)<<16);
			if(m_es5510_ram_sel)
				m_es5510_dil_latch = m_es5510_dram[m_es5510_dadr_latch];
			else
				m_es5510_dram[m_es5510_dadr_latch] = m_es5510_dol_latch;
			break;

		case 0x10: m_es5510_dadr_latch=(m_es5510_dadr_latch&0xff00ff)|((data&0xff)<< 8); break;
		case 0x11: m_es5510_dadr_latch=(m_es5510_dadr_latch&0xffff00)|((data&0xff)<< 0); break;

		/* 0x12 Host Control */

		case 0x14: m_es5510_ram_sel = data & 0x80; /* bit 6 is i/o select, everything else is undefined */break;

		/* 0x16 Program Counter (test purpose, r/o?) */
		/* 0x17 Internal Refresh counter (test purpose) */
		/* 0x18 Host Serial Control */
		/* 0x1f Halt enable (w) / Frame Counter (r) */

		case 0x80: /* Read select - GPR + INSTR */
	//      logerror("ES5510:  Read GPR/INSTR %06x (%06x)\n",data,m_es5510_gpr[data]);

			/* Check if a GPR is selected */
			if (data<0xc0) {
				//es_tmp=0;
				m_es5510_gpr_latch=m_es5510_gpr[data];
			}// else es_tmp=1;
			break;

		case 0xa0: /* Write select - GPR */
	//      logerror("ES5510:  Write GPR %06x %06x (0x%04x:=0x%06x\n",data,m_es5510_gpr_latch,data,snd_mem[m_es5510_gpr_latch>>8]);
			if (data<0xc0)
				m_es5510_gpr[data]=snd_mem[m_es5510_gpr_latch>>8];
			break;

		case 0xc0: /* Write select - INSTR */
	//      logerror("ES5510:  Write INSTR %06x %06x\n",data,m_es5510_gpr_latch);
			break;

		case 0xe0: /* Write select - GPR + INSTR */
	//      logerror("ES5510:  Write GPR/INSTR %06x %06x\n",data,m_es5510_gpr_latch);
			break;
	}
}


/*************************************
 *
 *  68000 memory map
 *
 *************************************/

static ADDRESS_MAP_START( en_sound_map, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM AM_MIRROR(0x30000) AM_SHARE("share1")
	AM_RANGE(0x140000, 0x140fff) AM_DEVREADWRITE8("taito_en", taito_en_device, en_68000_share_r, en_68000_share_w, 0xff00)
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE("ensoniq", es5505_device, read, write)
	AM_RANGE(0x260000, 0x2601ff) AM_DEVREADWRITE("taito_en", taito_en_device, es5510_dsp_r, es5510_dsp_w) //todo: hook up cpu/es5510
	AM_RANGE(0x280000, 0x28001f) AM_DEVREADWRITE8("duart68681", mc68681_device, read, write, 0x00ff)
	AM_RANGE(0x300000, 0x30003f) AM_DEVWRITE("taito_en", taito_en_device, en_es5505_bank_w)
	AM_RANGE(0x340000, 0x340003) AM_DEVWRITE8("taito_en", taito_en_device, en_volume_w, 0xff00)
	AM_RANGE(0xc00000, 0xc1ffff) AM_ROMBANK("bank1")
	AM_RANGE(0xc20000, 0xc3ffff) AM_ROMBANK("bank2")
	AM_RANGE(0xc40000, 0xc7ffff) AM_ROMBANK("bank3")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("share1")  // mirror
ADDRESS_MAP_END


/*************************************
 *
 *  MB87078 callback
 *
 *************************************/

WRITE8_MEMBER(taito_en_device::mb87078_gain_changed)
{
	if (offset > 1)
	{
		m_ensoniq->set_output_gain(offset & 1, data / 100.0);
	}
}


/*************************************
 *
 *  M68681 callback
 *
 *************************************/

WRITE_LINE_MEMBER(taito_en_device::duart_irq_handler)
{
	if (state == ASSERT_LINE)
	{
		m_audiocpu->set_input_line_vector(M68K_IRQ_6, m_duart68681->get_irq_vector());
		m_audiocpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
	}
	else
	{
		m_audiocpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}
}


/*************************************
 *
 *  Device interfaces
 *
 *************************************/

/*
    68681 I/O pin assignments
    (according to Gun Buster schematics):

    IP0: 5V         OP0-OP5: N/C
    IP1: 5V         OP6: ESPHALT
    IP2: 1MHz       OP7: N/C
    IP3: 0.5MHz
    IP4: 0.5MHz
    IP5: 1MHz
*/

/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( taito_en_sound )

	/* basic machine hardware */
	MCFG_TAITO_EN_ADD("taito_en")
	MCFG_CPU_ADD("audiocpu", M68000, XTAL_30_4761MHz / 2)
	MCFG_CPU_PROGRAM_MAP(en_sound_map)

	MCFG_MC68681_ADD("duart68681", XTAL_16MHz / 4)
	MCFG_MC68681_SET_EXTERNAL_CLOCKS(XTAL_16MHz/2/8, XTAL_16MHz/2/16, XTAL_16MHz/2/16, XTAL_16MHz/2/8)
	MCFG_MC68681_IRQ_CALLBACK(DEVWRITELINE("taito_en", taito_en_device, duart_irq_handler))

	MCFG_DEVICE_ADD("mb87078", MB87078, 0)
	MCFG_MB87078_GAIN_CHANGED_CB(DEVWRITE8("taito_en", taito_en_device, mb87078_gain_changed))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5505, XTAL_30_4761MHz / 2)
	MCFG_ES5505_REGION0("ensoniq.0")
	MCFG_ES5505_REGION1("ensoniq.0")
	MCFG_ES5506_CHANNELS(1)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.08)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.08)
MACHINE_CONFIG_END
