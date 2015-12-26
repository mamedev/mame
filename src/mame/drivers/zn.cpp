// license:BSD-3-Clause
// copyright-holders:smf, R. Belmont
/***************************************************************************

  Sony ZN1/ZN2 - Arcade PSX Hardware
  ==================================
  Driver by smf & R Belmont
  Board notes by The Guru
  Thanks to Zinc Team, Peter Ferrie, Amuse & Miguel Angel Horna

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/psx/psx.h"
#include "cpu/z80/z80.h"
#include "video/psx.h"
#include "machine/at28c16.h"
#include "machine/nvram.h"
#include "machine/mb3773.h"
#include "machine/7200fifo.h"
#include "machine/cat702.h"
#include "machine/zndip.h"
#include "machine/ataintf.h"
#include "machine/vt83c461.h"
#include "audio/taitosnd.h"
#include "sound/2610intf.h"
#include "sound/ymz280b.h"
#include "sound/qsound.h"
#include "sound/spu.h"
#include "sound/ymf271.h"
#include "audio/taito_zm.h"

#define VERBOSE_LEVEL ( 0 )

class zn_state : public driver_device
{
public:
	zn_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_gpu(*this, "gpu"),
		m_gpu_screen(*this, "gpu:screen"),
		m_sio0(*this, "maincpu:sio0"),
		m_cat702_1(*this, "cat702_1"),
		m_cat702_2(*this, "cat702_2"),
		m_zndip(*this, "zndip"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ram(*this, "maincpu:ram"),
		m_cbaj_fifo1(*this, "cbaj_fifo1"),
		m_cbaj_fifo2(*this, "cbaj_fifo2"),
		m_mb3773(*this, "mb3773"),
		m_zoom(*this, "taito_zoom"),
		m_vt83c461(*this, "ide"),
		m_cat702_1_dataout(1),
		m_cat702_2_dataout(1),
		m_zndip_dataout(1)
	{
	}

	DECLARE_WRITE_LINE_MEMBER(sio0_sck){ m_cat702_1->write_clock(state);  m_cat702_2->write_clock(state); m_zndip->write_clock(state); }
	DECLARE_WRITE_LINE_MEMBER(sio0_txd){ m_cat702_1->write_datain(state);  m_cat702_2->write_datain(state); }
	DECLARE_WRITE_LINE_MEMBER(cat702_1_dataout){ m_cat702_1_dataout = state; update_sio0_rxd(); }
	DECLARE_WRITE_LINE_MEMBER(cat702_2_dataout){ m_cat702_2_dataout = state; update_sio0_rxd(); }
	DECLARE_WRITE_LINE_MEMBER(zndip_dataout){ m_zndip_dataout = state; update_sio0_rxd(); }
	void update_sio0_rxd() { m_sio0->write_rxd( m_cat702_1_dataout && m_cat702_2_dataout && m_zndip_dataout ); }
	DECLARE_CUSTOM_INPUT_MEMBER(jdredd_gun_mux_read);
	DECLARE_READ8_MEMBER(znsecsel_r);
	DECLARE_WRITE8_MEMBER(znsecsel_w);
	DECLARE_READ8_MEMBER(boardconfig_r);
	DECLARE_READ16_MEMBER(unknown_r);
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_READ16_MEMBER(capcom_kickharness_r);
	DECLARE_WRITE8_MEMBER(bank_coh1000c_w);
	DECLARE_WRITE8_MEMBER(qsound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(zn_qsound_w);
	DECLARE_WRITE8_MEMBER(bank_coh1000t_w);
	DECLARE_WRITE8_MEMBER(fx1a_sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(fx1b_fram_w);
	DECLARE_READ8_MEMBER(fx1b_fram_r);
	DECLARE_WRITE8_MEMBER(coh1002e_bank_w);
	DECLARE_WRITE8_MEMBER(coh1002e_sound_irq_w);
	DECLARE_WRITE16_MEMBER(bam2_mcu_w);
	DECLARE_READ16_MEMBER(bam2_mcu_r);
	DECLARE_READ16_MEMBER(bam2_unk_r);
	DECLARE_WRITE16_MEMBER(acpsx_00_w);
	DECLARE_WRITE16_MEMBER(acpsx_10_w);
	DECLARE_WRITE16_MEMBER(nbajamex_bank_w);
	DECLARE_WRITE16_MEMBER(nbajamex_80_w);
	DECLARE_READ16_MEMBER(nbajamex_08_r);
	DECLARE_READ16_MEMBER(nbajamex_80_r);
	DECLARE_WRITE8_MEMBER(coh1001l_bank_w);
	DECLARE_WRITE16_MEMBER(coh1001l_latch_w);
	DECLARE_WRITE16_MEMBER(coh1001l_sound_unk_w);
	DECLARE_WRITE8_MEMBER(coh1002v_bank_w);
	DECLARE_WRITE8_MEMBER(coh1002m_bank_w);
	DECLARE_READ8_MEMBER(cbaj_sound_main_status_r);
	DECLARE_READ8_MEMBER(cbaj_sound_z80_status_r);
	DECLARE_READ16_MEMBER(vt83c461_16_r);
	DECLARE_WRITE16_MEMBER(vt83c461_16_w);
	DECLARE_READ16_MEMBER(vt83c461_32_r);
	DECLARE_WRITE16_MEMBER(vt83c461_32_w);
	DECLARE_DRIVER_INIT(coh1000tb);
	DECLARE_MACHINE_RESET(coh1000c);
	DECLARE_MACHINE_RESET(glpracr);
	DECLARE_MACHINE_RESET(coh1000ta);
	DECLARE_MACHINE_RESET(coh1000tb);
	DECLARE_MACHINE_RESET(coh1002tb);
	DECLARE_MACHINE_RESET(coh1002e);
	DECLARE_MACHINE_RESET(bam2);
	DECLARE_MACHINE_RESET(nbajamex);
	DECLARE_MACHINE_RESET(coh1001l);
	DECLARE_MACHINE_RESET(coh1002v);
	DECLARE_MACHINE_RESET(coh1002m);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	INTERRUPT_GEN_MEMBER(qsound_interrupt);
	void atpsx_dma_read(UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size );
	void atpsx_dma_write(UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size );
	void jdredd_vblank(screen_device &screen, bool vblank_state);

protected:
	virtual void driver_start() override;

private:
	inline void ATTR_PRINTF(3,4) verboselog( int n_level, const char *s_fmt, ... );
	inline void psxwriteword( UINT32 *p_n_psxram, UINT32 n_address, UINT16 n_data );

	UINT8 m_n_znsecsel;

	UINT16 m_bam2_mcu_command;
	int m_jdredd_gun_mux;

	std::unique_ptr<UINT8[]> m_fx1b_fram;

	UINT16 m_vt83c461_latch;

	required_device<psxgpu_device> m_gpu;
	required_device<screen_device> m_gpu_screen;
	required_device<psxsio0_device> m_sio0;
	required_device<cat702_device> m_cat702_1;
	required_device<cat702_device> m_cat702_2;
	required_device<zndip_device> m_zndip;
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<ram_device> m_ram;
	optional_device<fifo7200_device> m_cbaj_fifo1;
	optional_device<fifo7200_device> m_cbaj_fifo2;
	optional_device<mb3773_device> m_mb3773;
	optional_device<taito_zoom_device> m_zoom;
	optional_device<vt83c461_device> m_vt83c461;

	int m_cat702_1_dataout;
	int m_cat702_2_dataout;
	int m_zndip_dataout;
};

inline void ATTR_PRINTF(3,4) zn_state::verboselog( int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine().describe_context(), buf );
	}
}

#ifdef UNUSED_FUNCTION
inline UINT16 zn_state::psxreadword( UINT32 *p_n_psxram, UINT32 n_address )
{
	return *( (UINT16 *)( (UINT8 *)p_n_psxram + WORD_XOR_LE( n_address ) ) );
}
#endif

inline void zn_state::psxwriteword( UINT32 *p_n_psxram, UINT32 n_address, UINT16 n_data )
{
	*( (UINT16 *)( (UINT8 *)p_n_psxram + WORD_XOR_LE( n_address ) ) ) = n_data;
}

static const UINT8 ac01[ 8 ] = { 0x80, 0x1c, 0xe2, 0xfa, 0xf9, 0xf1, 0x30, 0xc0 };
static const UINT8 ac02[ 8 ] = { 0xfc, 0x60, 0xe2, 0xfa, 0xf9, 0xf1, 0x30, 0xc0 };
static const UINT8 tw01[ 8 ] = { 0xc0, 0x18, 0xf9, 0x81, 0x82, 0xfe, 0x0c, 0xf0 };
static const UINT8 tw02[ 8 ] = { 0xf0, 0x81, 0x03, 0xfa, 0x18, 0x1c, 0x3c, 0xc0 };
static const UINT8 at01[ 8 ] = { 0xf8, 0xe1, 0xe2, 0xfe, 0x3c, 0x40, 0x70, 0xf0 };
static const UINT8 at02[ 8 ] = { 0xc0, 0x70, 0x78, 0xfa, 0xfe, 0x1c, 0xe1, 0x01 };
static const UINT8 cp01[ 8 ] = { 0xf0, 0x81, 0xc1, 0x20, 0xe2, 0xfe, 0x04, 0xf8 };
static const UINT8 cp02[ 8 ] = { 0xfc, 0xf1, 0x08, 0x18, 0xe2, 0xc2, 0x40, 0x80 }; /* brute forced */
static const UINT8 cp03[ 8 ] = { 0xc0, 0x10, 0x60, 0x7c, 0x04, 0xfa, 0x03, 0x01 };
static const UINT8 cp04[ 8 ] = { 0xf8, 0xe2, 0xe1, 0x81, 0x7c, 0x0c, 0x30, 0xc0 };
static const UINT8 cp05[ 8 ] = { 0x80, 0x08, 0x30, 0xc2, 0xfe, 0xfd, 0xe1, 0xe0 };
static const UINT8 cp06[ 8 ] = { 0xf0, 0x20, 0x3c, 0xfd, 0x81, 0x78, 0xfa, 0x02 };
static const UINT8 cp07[ 8 ] = { 0xf8, 0x60, 0x20, 0x3c, 0xfd, 0x03, 0xf2, 0xf0 }; /* brute forced */
static const UINT8 cp08[ 8 ] = { 0xe0, 0xf2, 0x70, 0x81, 0xc1, 0x3c, 0x04, 0xf8 };
static const UINT8 cp09[ 8 ] = { 0xfc, 0x20, 0x38, 0x08, 0xf1, 0x03, 0x82, 0x80 }; /* brute forced */
static const UINT8 cp10[ 8 ] = { 0xe0, 0x40, 0x38, 0x08, 0xf1, 0x03, 0xfe, 0xfc };
static const UINT8 cp11[ 8 ] = { 0xf0, 0x20, 0xe1, 0x81, 0x7c, 0x04, 0xfa, 0x02 }; /* brute forced */
static const UINT8 cp12[ 8 ] = { 0xfc, 0x82, 0x60, 0xe1, 0xf9, 0x38, 0x30, 0xf0 }; /* brute forced */
static const UINT8 cp13[ 8 ] = { 0x02, 0x70, 0x08, 0x04, 0x3c, 0x20, 0xe1, 0x01 };
static const UINT8 et01[ 8 ] = { 0x02, 0x08, 0x18, 0x1c, 0xfd, 0xc1, 0x40, 0x80 };
static const UINT8 et02[ 8 ] = { 0xc0, 0xe1, 0xe2, 0xfe, 0x7c, 0x70, 0x08, 0xf8 };
static const UINT8 et03[ 8 ] = { 0xc0, 0x08, 0xfa, 0xe2, 0xe1, 0xfd, 0x7c, 0x80 };
static const UINT8 et05[ 8 ] = { 0xf0, 0x03, 0xe2, 0x18, 0x78, 0x7c, 0x3c, 0xc0 };
static const UINT8 mg01[ 8 ] = { 0x80, 0xf2, 0x30, 0x38, 0xf9, 0xfd, 0x1c, 0xe0 };
static const UINT8 mg02[ 8 ] = { 0xe0, 0x7c, 0x40, 0xc1, 0xf9, 0xfa, 0xf2, 0xf0 };
static const UINT8 mg03[ 8 ] = { 0xc0, 0x04, 0x78, 0x82, 0x03, 0xf1, 0x10, 0xe0 };
static const UINT8 mg04[ 8 ] = { 0xf0, 0xe1, 0x81, 0x82, 0xfa, 0x04, 0x3c, 0xc0 };
static const UINT8 mg05[ 8 ] = { 0x80, 0xc2, 0x38, 0xf9, 0xfd, 0x0c, 0x10, 0xe0 }; /* brute forced */
static const UINT8 mg06[ 8 ] = { 0xc0, 0xe2, 0xe1, 0xfd, 0x04, 0x78, 0x70, 0xf0 };
static const UINT8 mg07[ 8 ] = { 0xe0, 0xc2, 0x38, 0xf9, 0xfd, 0x0c, 0x70, 0x80 };
static const UINT8 mg08[ 8 ] = { 0xf0, 0xfa, 0xf9, 0xc1, 0x20, 0x1c, 0x7c, 0x80 };
static const UINT8 mg09[ 8 ] = { 0xf0, 0x03, 0xe2, 0x18, 0x78, 0x7c, 0x3c, 0xc0 }; /* brute forced */
static const UINT8 mg10[ 8 ] = { 0xfc, 0xf2, 0x30, 0xc1, 0xf9, 0x78, 0x60, 0xe0 };
static const UINT8 mg11[ 8 ] = { 0x80, 0xc2, 0x38, 0xf9, 0xfd, 0x1c, 0x10, 0xf0 };
static const UINT8 mg12[ 8 ] = { 0x02, 0x40, 0x38, 0xf9, 0xfd, 0x1c, 0x10, 0xf0 };
static const UINT8 mg13[ 8 ] = { 0xc0, 0xe2, 0xe1, 0xf9, 0x04, 0x0c, 0x70, 0x80 };
static const UINT8 mg14[ 8 ] = { 0xfc, 0xf2, 0xfa, 0x18, 0x20, 0x40, 0x81, 0x01 };
static const UINT8 tt01[ 8 ] = { 0xe0, 0xf9, 0xfd, 0x7c, 0x70, 0x30, 0xc2, 0x02 };
static const UINT8 tt02[ 8 ] = { 0xfc, 0x60, 0xe1, 0xc1, 0x30, 0x08, 0xfa, 0x02 }; /* brute forced */
static const UINT8 tt03[ 8 ] = { 0xf0, 0x20, 0xe2, 0xfa, 0x78, 0x81, 0xfd, 0xfc }; /* brute forced */
static const UINT8 tt04[ 8 ] = { 0xc0, 0xe1, 0xe2, 0xfa, 0x78, 0x7c, 0x0c, 0xf0 };
static const UINT8 tt05[ 8 ] = { 0xc0, 0xf1, 0xf2, 0xe2, 0x60, 0x7c, 0x04, 0xf8 }; /* brute forced */
static const UINT8 tt06[ 8 ] = { 0xfc, 0x38, 0xfa, 0xf2, 0xf1, 0xe1, 0x60, 0x80 }; /* brute forced */
static const UINT8 tt07[ 8 ] = { 0x80, 0x10, 0xf1, 0x03, 0xfa, 0x38, 0x3c, 0xfc }; /* brute forced & dumped */
//static const UINT8 tt10[ 8 ] = { 0x80, 0x20, 0x38, 0x08, 0xf1, 0x03, 0xfe, 0xfc };
//static const UINT8 tt16[ 8 ] = { 0xc0, 0x04, 0xf9, 0xe1, 0x60, 0x70, 0xf2, 0x02 };
static const UINT8 kn01[ 8 ] = { 0xf8, 0xe1, 0xe2, 0xfe, 0x3c, 0x30, 0x70, 0x80 }; /* brute forced */
static const UINT8 kn02[ 8 ] = { 0x01, 0x18, 0xe2, 0xfe, 0x3c, 0x30, 0x70, 0x80 }; /* brute forced */

static const struct
{
	const char *s_name;
	const UINT8 *p_n_mainsec;
	const UINT8 *p_n_gamesec;
} zn_config_table[] =
{
	{ "nbajamex", ac01, ac02 }, /* black screen */
	{ "jdredd",   ac01, ac02 }, /* OK */
	{ "jdreddb",  ac01, ac02 }, /* OK */
	{ "primrag2", tw01, tw02 }, /* locks up when starting a game */
	{ "hvnsgate", at01, at02 }, /* OK */
	{ "ts2",      cp01, cp02 }, /* OK */
	{ "ts2a",     cp01, cp02 }, /* OK */
	{ "ts2j",     cp01, cp02 }, /* OK */
	{ "starglad", cp01, cp03 }, /* OK */
	{ "stargladj",cp01, cp03 }, /* OK */
	{ "sfex",     cp01, cp04 }, /* OK */
	{ "sfexa",    cp01, cp04 }, /* OK */
	{ "sfexj",    cp01, cp04 }, /* OK */
	{ "sfexu",    cp01, cp04 }, /* OK */
	{ "sfexp",    cp01, cp04 }, /* OK */
	{ "sfexpu1",  cp01, cp04 }, /* OK */
	{ "sfexpj",   cp01, cp04 }, /* OK */
	{ "sfexpj1",  cp01, cp04 }, /* OK */
	{ "glpracr",  cp01, cp05 }, /* OK */
	{ "glpracrj", cp01, cp05 }, /* OK */
	{ "rvschool", cp10, cp06 }, /* OK */
	{ "rvschoolu",cp10, cp06 }, /* OK */
	{ "rvschoola",cp10, cp06 }, /* OK */
	{ "jgakuen",  cp10, cp06 }, /* OK */
	{ "plsmaswd", cp10, cp07 }, /* OK */
	{ "stargld2", cp10, cp07 }, /* OK */
	{ "plsmaswda",cp10, cp07 }, /* OK */
	{ "sfex2",    cp10, cp08 }, /* OK ( random crashes on garuda ) */
	{ "sfex2j",   cp10, cp08 }, /* OK ( random crashes on garuda ) */
	{ "sfex2a",   cp10, cp08 }, /* OK ( random crashes on garuda ) */
	{ "sfex2h",   cp10, cp08 }, /* OK ( random crashes on garuda ) */
	{ "techromn", cp10, cp09 }, /* OK */
	{ "techromnu",cp10, cp09 }, /* OK */
	{ "kikaioh",  cp10, cp09 }, /* OK */
	{ "tgmj",     cp10, cp11 }, /* OK */
	{ "sfex2p",   cp10, cp12 }, /* OK */
	{ "sfex2pa",  cp10, cp12 }, /* OK */
	{ "sfex2pj",  cp10, cp12 }, /* OK */
	{ "sfex2ph",  cp10, cp12 }, /* OK */
	{ "strider2", cp10, cp13 }, /* OK ( random crashes on bosses ) */
	{ "strider2a",cp10, cp13 }, /* OK ( random crashes on bosses ) */
	{ "shiryu2",  cp10, cp13 }, /* OK ( random crashes on bosses ) */
	{ "beastrzr", et01, et02 }, /* OK */
	{ "bldyroar", et01, et02 }, /* OK */
	{ "beastrzrb",et01, et02 }, /* OK */
	{ "bldyror2", et01, et03 }, /* OK */
	{ "bldyror2u",et01, et03 }, /* OK */
	{ "bldyror2a",et01, et03 }, /* OK */
	{ "bldyror2j",et01, et03 }, /* OK */
	{ "bam2",     et01, et05 },
	{ "glpracr2", mg01, mg02 }, /* locks up when starting a game/entering test mode */
	{ "glpracr2j",mg01, mg02 }, /* locks up when starting a game/entering test mode */
	{ "glpracr2l",mg01, mg02 }, /* locks up when starting a game/entering test mode */
	{ "cbaj",     mg01, mg03 }, /* OK */
	{ "shngmtkb", mg01, mg04 }, /* OK */
	{ "doapp",    mg01, mg05 }, /* OK */
	{ "flamegun", mg01, mg06 }, /* OK */
	{ "flamegunj",mg01, mg06 }, /* OK */
	{ "lpadv",    mg01, mg07 },
	{ "glpracr3", mg01, mg08 },
	{ "glpracr3j",mg01, mg08 },
	{ "tondemo",  mg01, mg09 }, /* OK */
	{ "1on1gov",  mg01, mg10 }, /* OK */
	{ "brvblade", mg01, mg11 }, /* OK */
	{ "brvbladeu", mg01, mg11 }, /* OK */
	{ "brvbladea", mg01, mg11 }, /* OK */
	{ "brvbladej", mg01, mg11 }, /* OK */
	{ "tblkkuzu", mg01, mg12 }, /* OK */
	{ "tecmowcm", mg01, mg13 }, /* OK */
	{ "mfjump",   mg01, mg14 }, /* OK */
	{ "sfchamp",  tt01, tt02 }, /* OK */
	{ "sfchampo", tt01, tt02 }, /* OK */
	{ "sfchampu", tt01, tt02 }, /* OK */
	{ "sfchampj", tt01, tt02 }, /* OK */
	{ "psyforce", tt01, tt03 }, /* OK */
	{ "psyforcej",tt01, tt03 }, /* OK */
	{ "psyforcex",tt01, tt03 }, /* OK */
	{ "raystorm", tt01, tt04 }, /* OK */
	{ "raystormo",tt01, tt04 }, /* OK */
	{ "raystormu",tt01, tt04 }, /* OK */
	{ "raystormj",tt01, tt04 }, /* OK */
	{ "ftimpact", tt01, tt05 }, /* OK */
	{ "ftimpactu",tt01, tt05 }, /* OK */
	{ "ftimpactj",tt01, tt05 }, /* OK */
	{ "ftimpcta", tt01, tt05 }, /* OK */
	{ "mgcldate", tt01, tt06 }, /* OK */
	{ "mgcldtex", tt01, tt06 }, /* OK */
	{ "gdarius",  tt01, tt07 }, /* OK */
	{ "gdariusb", tt01, tt07 }, /* OK */
	{ "gdarius2", tt01, tt07 }, /* OK */
	{ "aerofgts", kn01, kn02 }, /* OK */
	{ "sncwgltd", kn01, kn02 }, /* OK */
	/* 2009-10 FP: to avoid crash when running BIOS sets alone, I added the entries below.        */
	/* Now we get "Error B930 - Cannot Find Program Rom". Is this the correct way to handle them? */
	{ "acpsx",    ac01, ac02 },
	{ "atpsx",    tw01, tw02 },
	{ "atluspsx", at01, at02 },
	{ "cpzn1",    cp01, cp02 },
	{ "cpzn2",    cp01, cp02 },
	{ "psarc95",  et01, et02 },
	{ "tps",      mg01, mg02 },
	{ "taitofx1", tt01, tt02 },
	{ "vspsx",    kn01, kn02 },
	{ nullptr, nullptr, nullptr }
};

READ8_MEMBER(zn_state::znsecsel_r)
{
	verboselog(2, "znsecsel_r( %08x, %08x )\n", offset, mem_mask );
	return m_n_znsecsel;
}

WRITE8_MEMBER(zn_state::znsecsel_w)
{
	verboselog(2, "znsecsel_w( %08x, %08x, %08x )\n", offset, data, mem_mask );

	m_cat702_1->write_select((data >> 2) & 1);
	m_cat702_2->write_select((data >> 3) & 1);
	m_zndip->write_select((data & 0x8c) != 0x8c);

	m_n_znsecsel = data;
}

READ8_MEMBER(zn_state::boardconfig_r)
{
	/*
	------00 mem=4M
	------01 mem=4M
	------10 mem=8M
	------11 mem=16M
	-----0-- smem=hM
	-----1-- smem=2M
	----0--- vmem=1M
	----1--- vmem=2M
	000----- rev=-2
	001----- rev=-1
	010----- rev=0
	011----- rev=1
	100----- rev=2
	101----- rev=3
	110----- rev=4
	111----- rev=5
	*/

	int boardconfig = 64 | 32;

	if( m_gpu_screen->height() == 1024 )
	{
		boardconfig |= 8;
	}

	switch( m_ram->size() )
	{
	case 0x400000:
		boardconfig |= 1;
		break;

	case 0x800000:
		boardconfig |= 2;
		break;

	case 0x1000000:
		boardconfig |= 3;
		break;
	}

	return boardconfig;
}

READ16_MEMBER(zn_state::unknown_r)
{
	verboselog(0, "unknown_r( %08x, %08x )\n", offset, mem_mask );
	return 0xffff;
}

WRITE8_MEMBER(zn_state::coin_w)
{
	/* 0x01=counter
	   0x02=coin lock 1
	   0x08=??
	   0x20=coin lock 2
	   0x80=??
	*/
	if( ( data & ~0x23 ) != 0 )
	{
		verboselog(0, "coin_w %08x\n", data );
	}
}

static ADDRESS_MAP_START( zn_map, AS_PROGRAM, 32, zn_state )
	AM_RANGE(0x1fa00000, 0x1fa00003) AM_READ_PORT("P1")
	AM_RANGE(0x1fa00100, 0x1fa00103) AM_READ_PORT("P2")
	AM_RANGE(0x1fa00200, 0x1fa00203) AM_READ_PORT("SERVICE")
	AM_RANGE(0x1fa00300, 0x1fa00303) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1fa10000, 0x1fa10003) AM_READ_PORT("P3")
	AM_RANGE(0x1fa10100, 0x1fa10103) AM_READ_PORT("P4")
	AM_RANGE(0x1fa10200, 0x1fa10203) AM_READ8(boardconfig_r, 0x000000ff)
	AM_RANGE(0x1fa10300, 0x1fa10303) AM_READWRITE8(znsecsel_r, znsecsel_w, 0x000000ff)
	AM_RANGE(0x1fa20000, 0x1fa20003) AM_WRITE8(coin_w, 0x000000ff)
	AM_RANGE(0x1fa30000, 0x1fa30003) AM_NOP /* ?? */
	AM_RANGE(0x1fa40000, 0x1fa40003) AM_READNOP /* ?? */
	AM_RANGE(0x1fa60000, 0x1fa60003) AM_READNOP /* ?? */
	AM_RANGE(0x1faf0000, 0x1faf07ff) AM_DEVREADWRITE8("at28c16", at28c16_device, read, write, 0xffffffff) /* eeprom */
	AM_RANGE(0x1fb20000, 0x1fb20007) AM_READ16(unknown_r, 0xffffffff)
ADDRESS_MAP_END

void zn_state::driver_start()
{
	int n_game;

	n_game = 0;
	while( zn_config_table[ n_game ].s_name != nullptr )
	{
		if( strcmp( machine().system().name, zn_config_table[ n_game ].s_name ) == 0 )
		{
			m_cat702_1->init( zn_config_table[ n_game ].p_n_mainsec );
			m_cat702_2->init( zn_config_table[ n_game ].p_n_gamesec );
			break;
		}
		n_game++;
	}
}

static MACHINE_CONFIG_START( zn1_1mb_vram, zn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8530CQ, XTAL_67_7376MHz )
	MCFG_CPU_PROGRAM_MAP( zn_map)

	MCFG_RAM_MODIFY("maincpu:ram")
	MCFG_RAM_DEFAULT_SIZE("4M")

	MCFG_DEVICE_MODIFY("maincpu:sio0")
	MCFG_PSX_SIO_SCK_HANDLER(DEVWRITELINE(DEVICE_SELF_OWNER, zn_state, sio0_sck))
	MCFG_PSX_SIO_TXD_HANDLER(DEVWRITELINE(DEVICE_SELF_OWNER, zn_state, sio0_txd))

	MCFG_DEVICE_ADD("cat702_1", CAT702, 0)
	MCFG_CAT702_DATAOUT_HANDLER(WRITELINE(zn_state, cat702_1_dataout))

	MCFG_DEVICE_ADD("cat702_2", CAT702, 0)
	MCFG_CAT702_DATAOUT_HANDLER(WRITELINE(zn_state, cat702_2_dataout))

	MCFG_DEVICE_ADD("zndip", ZNDIP, 0)
	MCFG_ZNDIP_DATAOUT_HANDLER(WRITELINE(zn_state, zndip_dataout))
	MCFG_ZNDIP_DSR_HANDLER(DEVWRITELINE("maincpu:sio0", psxsio0_device, write_dsr))
	MCFG_ZNDIP_DATA_HANDLER(IOPORT(":DSW"))

	// 5MHz NEC uPD78081 MCU:
	// we don't have a 78K0 emulation core yet..

	/* video hardware */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8561Q, 0x100000, XTAL_53_693175MHz )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SPU_ADD( "spu", XTAL_67_7376MHz/2 )
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.35)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.35)

	MCFG_AT28C16_ADD( "at28c16", NULL )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( zn1_2mb_vram, zn1_1mb_vram )
	MCFG_PSXGPU_REPLACE( "maincpu", "gpu", CXD8561Q, 0x200000, XTAL_53_693175MHz )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( zn2, zn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8661R, XTAL_100MHz )
	MCFG_CPU_PROGRAM_MAP( zn_map)

	MCFG_RAM_MODIFY("maincpu:ram")
	MCFG_RAM_DEFAULT_SIZE("4M")

	MCFG_DEVICE_MODIFY("maincpu:sio0")
	MCFG_PSX_SIO_SCK_HANDLER(DEVWRITELINE(DEVICE_SELF_OWNER, zn_state, sio0_sck))
	MCFG_PSX_SIO_TXD_HANDLER(DEVWRITELINE(DEVICE_SELF_OWNER, zn_state, sio0_txd))

	MCFG_DEVICE_ADD("cat702_1", CAT702, 0)
	MCFG_CAT702_DATAOUT_HANDLER(WRITELINE(zn_state, cat702_1_dataout))

	MCFG_DEVICE_ADD("cat702_2", CAT702, 0)
	MCFG_CAT702_DATAOUT_HANDLER(WRITELINE(zn_state, cat702_2_dataout))

	MCFG_DEVICE_ADD("zndip", ZNDIP, 0)
	MCFG_ZNDIP_DATAOUT_HANDLER(WRITELINE(zn_state, zndip_dataout))
	MCFG_ZNDIP_DSR_HANDLER(DEVWRITELINE("maincpu:sio0", psxsio0_device, write_dsr))
	MCFG_ZNDIP_DATA_HANDLER(IOPORT(":DSW"))

	// 5MHz NEC uPD78081 MCU:
	// we don't have a 78K0 emulation core yet..

	/* video hardware */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8654Q, 0x200000, XTAL_53_693175MHz )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SPU_ADD( "spu", XTAL_67_7376MHz/2 )
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.35)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.35)

	MCFG_AT28C16_ADD( "at28c16", NULL )
MACHINE_CONFIG_END

/*
Capcom ZN1 generic PCB Layout
----------------------------

Main board (Standard ZN1 Main Board with Capcom ZN1 BIOS)
ZN-1 1-659-709-12  COH-1000C
|--------------------------------------------------------|
|  LA4705             |---------------------------|      |
|                     |---------------------------|      |
|    AKM_AK4310VM      AT28C16                           |
|  VOL                                                   |
|       SW1            BIOS                              |
|  TD62064                                               |
|  TD62064        PALCE16V8                              |
|J                                                       |
|                                                        |
|A                EPM7032    814260    CXD2922CQ         |
|                                                        |
|M                                                       |
|                                                        |
|M                                                       |
|                *                                       |
|A           DSW                                         |
|                                CXD8561Q    CXD8530CQ   |
|                KM4132G271Q-12                          |
|CN505  CN506                   53.693MHz    67.737MHz   |
|            CAT702                                      |
|                                                        |
|CN504  CN503                                            |
|                                                        |
|                                                        |
|  NEC_78081G503         KM48V514BJ-12   KM48V514BJ-12   |
|            MC44200FT   KM48V514BJ-12   KM48V514BJ-12   |
|CN651  CN652            KM48V514BJ-12   KM48V514BJ-12   |
|                CN654   KM48V514BJ-12   KM48V514BJ-12   |
|--------------------------------------------------------|
Notes:
      CN506 - Connector for optional 3rd player controls
      CN505 - Connector for optional 4th player controls
      CN503 - Connector for optional 15kHz external video output (R,G,B,Sync, GND)
      CN504 - Connector for optional 2nd speaker (for stereo output)
      CN652 - Connector for optional trackball
      CN651 - Connector for optional analog controls
      CN654 - Connector for optional memory card
      SW1   - Slide switch for stereo or mono sound output
      DSW   - Dip switch (4 position, defaults all OFF)

      BIOS           - COH1000C.353, Capcom ZN1 BIOS, 4MBit MaskROM type M534002 (SOP40)
      AT28C16        - Atmel AT28C16 2K x8 EEPROM (SOP24)
      814260-70      - 256K x16 (4MBit) DRAM (SOJ40)
      KM4132G271Q-12 - 128K x32 x2 banks (32MBit) SGRAM (QFP100)
      *              - Unpopulated position for KM4132G271Q-12 SGRAM
      KM48V514BJ-6   - 512k x8 (4MBit) DRAM (SOJ28)
      EPM7032        - Altera EPM7032QC44-15 CPLD labelled 'ZN1A' (QFP44)
      CAT702         - Protection chip labelled 'CP01' (DIP20)
      PALCE16V8      - PAL, labelled 'ZN1A' (PLCC20)
      NEC_78081G503  - NEC uPD78081 MCU, 5MHz


Game board (Gallop Racer)

95681-2
|--------------------------------------|
|    |---------------------------|     |
|    |---------------------------|     |
|LM833    60MHz              1.3B  8MHz|
|                                      |
|                                  Z80 |
|NE5532        CAPCOM-Q1               |
|                                      |
|                                      |
|TDA1543   93C46   C.P.S.2-B           |
|                            3.3E  2.2E|
|                                      |
|CN2                                   |
|  CAT702                         8464 |
|                                      |
|                                      |
|  PAL1                                |
|                                      |
|               7.5H  6.4H   5.3H  4.2H|
|  PAL2                                |
|PAL3                                  |
|                                      |
|M54532                                |
|                                      |
|                                      |
|13.7K  12.6K  11.5K  10.4K  9.3K  8.2K|
|                                      |
|                                      |
|--------------------------------------|
Notes:
      CN2    - Standard 34 pin CAPCOM connector for extra player controls.
      CAT702 - protection chip labelled 'CP05' (DIP20)
      PAL1   - PAL16L8 stamped "CS1CNT"
      PAL2   - PALCE16V8 stamped "BANK01"
      PAL3   - PALCE16V8 stamped "CS0CNT"
      4.2H   - 27C4002 DIP40 EPROM labelled 'GPAJ_04'
      5.3H   - uPD23C32020CZ 32MBit DIP42 MaskROM labelled 'GRA-05M'
      6.4H   - uPD23C32020CZ 32MBit DIP42 MaskROM labelled 'GRA-06M'
      7.5H   - uPD23C32020CZ 32MBit DIP42 MaskROM labelled 'GRA-07M'
      8464   - 8K x8 SRAM
      LM833  - National Semiconductor Dual Audio Operational Amplifier
      NE5532 - Fairchild Semiconductor Dual Operational Amplifier
      M54532 - Mitsubishi M54532 IC, connected to CN2
      93C46  - 128Bytes EEPROM
      TDA1543- PHILIPS Dual 16-bit DAC
      CAPCOM-Q1 - Q-Sound chip also stamped DL-1425 45570 9420S 40 (C)92 AT&T (PLCC84)
      C.P.S.2-B - RF5C320 CAPCOM C.P.S.2-B DL-3129 (QFP208) - Ricoh custom? what is this chip for? Z80 northbridge?(address decode, etc)
      Z80 clock - 8.000MHz

      Unpopulated sockets - 1.3B, 2.2E, 3.3E, 8.2K, 9.3K, 10.4K, 11.5K, 12.6K & 13.7K
*/

READ16_MEMBER(zn_state::capcom_kickharness_r)
{
	/* required for buttons 4,5&6 */
	verboselog(2, "capcom_kickharness_r( %08x, %08x )\n", offset, mem_mask );
	return 0xffff;
}

WRITE8_MEMBER(zn_state::bank_coh1000c_w)
{
	membank( "bankedroms" )->set_base( memregion( "maskroms" )->base() + 0x400000 + ( data * 0x400000 ) );
}

WRITE8_MEMBER(zn_state::qsound_bankswitch_w)
{
	membank( "bank10" )->set_base( memregion( "audiocpu" )->base() + 0x10000 + ( ( data & 0x0f ) * 0x4000 ) );
}

INTERRUPT_GEN_MEMBER(zn_state::qsound_interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE);
}

WRITE8_MEMBER(zn_state::zn_qsound_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START(coh1000c_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1f000000, 0x1f3fffff) AM_ROM AM_REGION("maskroms", 0)
	AM_RANGE(0x1f400000, 0x1f7fffff) AM_ROMBANK("bankedroms")
	AM_RANGE(0x1fb00000, 0x1fb00003) AM_WRITE8(bank_coh1000c_w, 0x000000ff)
	AM_RANGE(0x1fb40010, 0x1fb40013) AM_READ16(capcom_kickharness_r, 0x0000ffff)
	AM_RANGE(0x1fb40020, 0x1fb40023) AM_READ16(capcom_kickharness_r, 0x0000ffff)
	AM_RANGE(0x1fb80000, 0x1fbfffff) AM_ROM AM_REGION("countryrom", 0)
	AM_RANGE(0x1fb60000, 0x1fb60003) AM_WRITE8(zn_qsound_w, 0x000000ff)

	AM_IMPORT_FROM(zn_map)
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(zn_state,coh1000c)
{
	membank("bankedroms")->set_base(memregion("maskroms")->base() + 0x400000 ); /* banked game rom */
}

MACHINE_RESET_MEMBER(zn_state,glpracr)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // glpracr qsound rom sockets are empty
	MACHINE_RESET_CALL_MEMBER(coh1000c);
}

static ADDRESS_MAP_START( qsound_map, AS_PROGRAM, 8, zn_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank10")       /* banked (contains music data) */
	AM_RANGE(0xd000, 0xd002) AM_DEVWRITE("qsound", qsound_device, qsound_w)
	AM_RANGE(0xd003, 0xd003) AM_WRITE(qsound_bankswitch_w)
	AM_RANGE(0xd007, 0xd007) AM_DEVREAD("qsound", qsound_device, qsound_r)
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( qsound_portmap, AS_IO, 8, zn_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static MACHINE_CONFIG_DERIVED( coh1000c, zn1_1mb_vram )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1000c_map)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(qsound_map)
	MCFG_CPU_IO_MAP(qsound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(zn_state, qsound_interrupt, 250) // measured (cps2.c)

	MCFG_MACHINE_RESET_OVERRIDE(zn_state, coh1000c)

	MCFG_QSOUND_ADD("qsound", QSOUND_CLOCK)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( glpracr, coh1000c )
	MCFG_MACHINE_RESET_OVERRIDE(zn_state, glpracr)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( coh1002c, zn1_2mb_vram )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1000c_map)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(qsound_map)
	MCFG_CPU_IO_MAP(qsound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(zn_state, qsound_interrupt, 250) // measured (cps2.c)

	MCFG_MACHINE_RESET_OVERRIDE(zn_state, coh1000c)

	MCFG_QSOUND_ADD("qsound", QSOUND_CLOCK)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/*

Capcom ZN2 generic PCB Layout
-----------------------------

PCB Layouts
-----------

1-665-825-11
ZN-2 COH-3000 (sticker says COH-3002C denoting Capcom ZN2 BIOS version)
|--------------------------------------------------------|
|  LA4705             |---------------------------|      |
|                     |---------------------------|      |
|    AKM_AK4310VM      AT28C16                           |
|  VOL                                                   |
|       S301           BIOS                              |
|                                                        |
|                                                        |
|J                                                       |
|                                                        |
|A              814260    CXD2925Q     EPM7064           |
|                                                        |
|M                                     67.73MHz          |
|                                                        |
|M                                                       |
|            S551    KM4132G271BQ-8                      |
|A                                                       |
|                                CXD8654Q    CXD8661R    |
|                    KM4132G271BQ-8                      |
|CN505  CN506                   53.693MHz    100MHz      |
|            CAT702                                      |
|                                                        |
|CN504  CN503                                            |
|                                                        |
|            MC44200FT                                   |
|  NEC_78081G503        KM416V1204BT-L5  KM416V1204BT-L5 |
|                                                        |
|CN651  CN652                 *                 *        |
|                CN654                                   |
|--------------------------------------------------------|
Notes:
      CN506 - Connector for optional 3rd player controls
      CN505 - Connector for optional 4th player controls
      CN503 - Connector for optional 15kHz external video output (R,G,B,Sync, GND)
      CN504 - Connector for optional 2nd speaker (for stereo output)
      CN652 - Connector for optional trackball
      CN651 - Connector for optional analog controls
      CN654 - Connector for optional memory card
      S301  - Slide switch for stereo or mono sound output
      S551  - Dip switch (4 position, defaults all OFF)

      BIOS           - COH-3002C.353, Capcom ZN2 BIOS 4MBit MaskROM type M534002 (SOP40)
      AT28C16        - Atmel AT28C16 2K x8 EEPROM
      814260-70      - 256K x16 (4MBit) DRAM
      KM4132G271BQ-8 - 128K x 32Bit x 2 Banks SGRAM
      KM416V1204BT-L5- 1M x16 EDO DRAM
      EPM7064        - Altera EPM7064QC100 CPLD (QFP100)
      CAT702         - Protection chip labelled 'CP10' (DIP20)
      *              - Unpopulated position for additional KM416V1204BT-L5 RAMs
      NEC_78081G503  - NEC uPD78081 MCU, 5MHz


Game board
(This covers at least Rival Schools and Street Fighter EX2, but likely all Capcom ZN2 games)

97695-1
|--------------------------------------|
|    |---------------------------|     |
|    |---------------------------|     |
|LM833    60MHz              1.3A  8MHz|
|                                      |
|                                  Z80 |
|NE5532        CAPCOM-Q1               |
|                                      |
|                                      |
|TDA1543   93C46   C.P.S.2-B           |
|                            3.3E  2.2E|
|                                      |
|CN2                                   |
|  CAT702                      N341256 |
|                                      |
|                                      |
|  PAL1                                |
|                                      |
|               7.5H  6.4H   5.3H  4.2H|
|  PAL2                                |
|PAL3                                  |
|                                      |
|M54532                                |
|                                      |
|                                      |
|13.7K  12.6K  11.5K  10.4K  9.3K  8.2K|
|                                      |
|                                      |
|--------------------------------------|
Notes:
      CN2    - Standard 34 pin CAPCOM connector for extra player controls.
      CAT702 - protection chip
                               SFEX2 labelled 'CP08' (DIP20)
                               Rival Schools labelled 'CP06' (DIP20)
      PAL1   - PAL16L8 stamped "CS1CNT"
      PAL2   - PALCE16V8 stamped "BANK02"
      PAL3   - PALCE16V8 stamped "MEMDEC"
      N341256- 32K x8 SRAM
      LM833  - National Semiconductor Dual Audio Operational Amplifier
      NE5532 - Fairchild Semiconductor Dual Operational Amplifier
      M54532 - Mitsubishi M54532 IC, connected to CN2
      93C46  - 128Bytes EEPROM
      TDA1543- PHILIPS Dual 16-bit DAC
      CAPCOM-Q1 - Q-Sound chip also stamped DL-1425 11008 9741T 74 (C)92 LUCENT (PLCC84)
      C.P.S.2-B - RF5C320 CAPCOM C.P.S.2-B DL-3129 (QFP208) - Ricoh custom? what is this chip for? Z80 northbridge?(address decode, etc)
      Z80 clock - 8.000MHz
      ROMs      -
                  SFEX2
                       1.3A   - uPD23C32020CZ 32MBit DIP42 MaskROM labelled 'EX2-01M'
                       2.2E   - 27C1001 DIP32 EPROM labelled 'EX2_02'
                       4.2H   - 27C240 DIP40 EPROM labelled 'EX2J_04'
                       5.3H   - 64MBit DIP42 MaskROM labelled 'EX2-05M'  \
                       6.4H   - 64MBit DIP42 MaskROM labelled 'EX2-06M'  |
                       7.5H   - 64MBit DIP42 MaskROM labelled 'EX2-07M'  |  Unknown type manufactured by Sharp
                       8.2K   - 64MBit DIP42 MaskROM labelled 'EX2-08M'  /
                       9.3K   - uPD23C32020CZ 32MBit DIP42 MaskROM labelled 'EX2-09M'
                       Unpopulated sockets on SFEX2 - 3.3E, 10.4K, 11.5K, 12.6K & 13.7K

                  Rival Schools
                       1.3A   - M533203E 32MBit DIP42 MaskROM labelled 'JST-01M'
                       2.2E   - 27C1001 DIP32 EPROM labelled 'JST_02'
                       3.3E   - 27C1001 DIP32 EPROM labelled 'JST_03'
                       4.2H   - 27C240 DIP40 EPROM labelled 'JSTJ_04A'
                       5.3H   - M533203E 32MBit DIP42 MaskROM labelled 'JST-05M'
                       6.4H   - M533203E 32MBit DIP42 MaskROM labelled 'JST-06M'
                       7.5H   - M533203E 32MBit DIP42 MaskROM labelled 'JST-07M'
                       8.2K   - TC5332202 32MBit DIP42 MaskROM labelled 'JST-08M'
                       9.3K   - TC5332202 32MBit DIP42 MaskROM labelled 'JST-09M'
                       10.4K  - TC5332202 32MBit DIP42 MaskROM labelled 'JST-10M'
                       11.5K  - GM23C32100A 32MBit DIP42 MaskROM labelled 'JST-11M'
                       12.6K  - GM23C32100A 32MBit DIP42 MaskROM labelled 'JST-12M'
                       13.7K  - GM23C32100A 32MBit DIP42 MaskROM labelled 'JST-13M'
                       Unpopulated sockets on Rival Schools - None
*/

static MACHINE_CONFIG_DERIVED(coh3002c, zn2)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1000c_map)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(qsound_map)
	MCFG_CPU_IO_MAP(qsound_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(zn_state, qsound_interrupt, 250) // measured (cps2.c)

	MCFG_MACHINE_RESET_OVERRIDE(zn_state, coh1000c)

	MCFG_QSOUND_ADD("qsound", QSOUND_CLOCK)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/*

Main board (Standard ZN1 Main Board with Taito ZN1 BIOS)
ZN-1 1-659-709-12  COH-1000T
|--------------------------------------------------------|
|  LA4705             |---------------------------|      |
|                     |---------------------------|      |
|    AKM_AK4310VM      AT28C16                           |
|  VOL                                                   |
|       SW1            BIOS                              |
|  TD62064                                               |
|  TD62064        PALCE16V8                              |
|J                                                       |
|                                                        |
|A                EPM7032    814260    CXD2922CQ         |
|                                                        |
|M                                                       |
|                                                        |
|M                                                       |
|                *                                       |
|A           DSW                                         |
|                                CXD8561Q    CXD8530CQ   |
|                KM4132G271Q-12                          |
|CN505  CN506                   53.693MHz    67.737MHz   |
|            CAT702                                      |
|                                                        |
|CN504  CN503                                            |
|                                                        |
|                                                        |
|  NEC_78081G503         KM48V514BJ-12   KM48V514BJ-12   |
|            MC44200FT   KM48V514BJ-12   KM48V514BJ-12   |
|CN651  CN652            KM48V514BJ-12   KM48V514BJ-12   |
|                CN654   KM48V514BJ-12   KM48V514BJ-12   |
|--------------------------------------------------------|
Notes:
      CN506 - Connector for optional 3rd player controls
      CN505 - Connector for optional 4th player controls
      CN503 - Connector for optional 15kHz external video output (R,G,B,Sync, GND)
      CN504 - Connector for optional 2nd speaker (for stereo output)
      CN652 - Connector for optional trackball
      CN651 - Connector for optional analog controls
      CN654 - Connector for optional memory card
      SW1   - Slide switch for stereo or mono sound output
      DSW   - Dip switch (4 position, defaults all OFF)

      BIOS           - COH1000T.353, Taito ZN1 BIOS, 4MBit MaskROM type M534002 (SOP40)
      AT28C16        - Atmel AT28C16 2K x8 EEPROM (SOP24)
      814260-70      - 256K x16 (4MBit) DRAM (SOJ40)
      KM4132G271Q-12 - 128K x32 x2 banks (32MBit) SGRAM (QFP100)
      *              - Unpopulated position for KM4132G271Q-12 SGRAM
      KM48V514BJ-6   - 512k x8 (4MBit) DRAM (SOJ28)
      EPM7032        - Altera EPM7032QC44-15 CPLD labelled 'ZN1A' (QFP44)
      CAT702         - Protection chip labelled 'TT01' (DIP20)
      PALCE16V8      - PAL, labelled 'ZN1A' (PLCC20)
      NEC_78081G503  - NEC uPD78081 MCU, 5MHz


Game board
----------

SROM PCB-A
K11X0643A PSYCHIC FORCE
M43J0308A 238102270
|--------------------------------------------|
|   |---------------------------|            |
|   |---------------------------|            |
|                                    IC1     |
|              CAT702    E22-05.2            |
|    E18-06                                  |
|                        E22-10.7            |
|                                    IC6     |
| MB3773                                     |
|                                            |
|                                            |
|              |---------|           IC12    |
| E22-01.15    |TAITO    |                   |
|              |TC0140SYT|                   |
|              |         |                   |
|              |         |          E22-02.16|
|              |---------|                   |
|  IC20                     16MHz            |
|                                   E22-03.19|
|                    84C000AM-6              |
|                                            |
|                                   E22-04.21|
|TL074                                       |
|                    LH5268AN-10LL           |
|    MB87078                         IC27    |
|                         E22-07.22          |
|TL074           YM2610                      |
|      Y3016                                 |
|--------------------------------------------|
Notes:
      IC1, IC6     \
      IC12, IC20   |   - Unpopulated positions for 16MBit SOP44 MaskROM
      IC27         /
      E22-01, E22-02 \
      E22-03, E22-04 / - 23C16000 16MBit SOP44 MaskROMs
      E22-05, E22-10   - 27C040 4MBit DIP32 EPROM
      E22-07           - 27C010 1MBit DIP32 EPROM
      E18-06           - AMD MACH111 CPLD stamped 'E18-06' (PLCC44)
      LH5268AN-10LL    - Sharp 8K x8 SRAM (SOP28)
      CAT702           - Protection chip labelled 'TT03' (DIP20)
      MB3773           - Power Supply Monitor with Watch Dog Timer (i.e. Reset IC)
      MB87078          - Electronic Volume Control IC
      84C000AM-6       - Z80-A; clock 4.000MHz
      YM2610           - clock 8.000MHz

Main board (Standard ZN1 Main Board with Taito ZN1 BIOS)
ZN-1 1-659-709-12  COH-1000T
K11X0831A RAY STORM
|--------------------------------------------------------|
|  LA4705             |---------------------------|      |
|                     |---------------------------|      |
|    AKM_AK4310VM      AT28C16                           |
|  VOL                                                   |
|       SW1            BIOS                              |
|  TD62064                                               |
|  TD62064        PALCE16V8                              |
|J                                                       |
|                                                        |
|A                EPM7032    814260    CXD2922CQ         |
|                                                        |
|M                                                       |
|                                                        |
|M                                                       |
|                *                                       |
|A           DSW                                         |
|                                CXD8561Q    CXD8530CQ   |
|                KM4132G271Q-12                          |
|CN505  CN506                   53.693MHz    67.737MHz   |
|            CAT702                                      |
|                                                        |
|CN504  CN503                                            |
|                                                        |
|                                                        |
|  NEC_78081G503         KM48V514BJ-12   KM48V514BJ-12   |
|            MC44200FT   KM48V514BJ-12   KM48V514BJ-12   |
|CN651  CN652            KM48V514BJ-12   KM48V514BJ-12   |
|                CN654   KM48V514BJ-12   KM48V514BJ-12   |
|--------------------------------------------------------|
Notes:
      CN506 - Connector for optional 3rd player controls
      CN505 - Connector for optional 4th player controls
      CN503 - Connector for optional 15kHz external video output (R,G,B,Sync, GND)
      CN504 - Connector for optional 2nd speaker (for stereo output)
      CN652 - Connector for optional trackball
      CN651 - Connector for optional analog controls
      CN654 - Connector for optional memory card
      SW1   - Slide switch for stereo or mono sound output
      DSW   - Dip switch (4 position, defaults all OFF)

      BIOS           - COH1000T.353, Taito ZN1 BIOS, 4MBit MaskROM type M534002 (SOP40)
      AT28C16        - Atmel AT28C16 2K x8 EEPROM (SOP24)
      814260-70      - 256K x16 (4MBit) DRAM (SOJ40)
      KM4132G271Q-12 - 128K x32 x2 banks (32MBit) SGRAM (QFP100)
      *              - Unpopulated position for KM4132G271Q-12 SGRAM
      KM48V514BJ-6   - 512k x8 (4MBit) DRAM (SOJ28)
      EPM7032        - Altera EPM7032QC44-15 CPLD labelled 'ZN1A' (QFP44)
      CAT702         - Protection chip labelled 'TT01' (DIP20)
      PALCE16V8      - PAL, labelled 'ZN1A' (PLCC20)
      NEC_78081G503  - NEC uPD78081 MCU, 5MHz


Game board

ZROM PCB
K91J0636A RAY STORM
M43J0311A 241103582
|--------------------------------------------|
|   |---------------------------|            |
|   |---------------------------|            |
|              CAT702                        |
|                         E24-06.3           |
|              E24-01                        |
|                         E24-05.4   E24-02.1|
|                                            |
|  25MHz                                     |
|                         LH52B256   E24-03.2|
|      MB3773                                |
|                    MN1020012A              |
|                                    IC12    |
|        MB87078                             |
|                                            |
| NJM2100                            IC13    |
|                                            |
| NJM2100                                    |
|                                    IC20    |
|              ZSG-2        E24-09.14        |
|                                            |
|                                    IC21    |
| TMS57002DPHA                               |
|                           M66220FP         |
|                                    IC25    |
|              IC28   E24-04.27              |
| LC321664                                   |
|                            FM1208S         |
|--------------------------------------------|
Notes:
      IC12, IC13 \
      IC20, IC21  |  - Unpopulated positions for 16MBit SOP44 MaskROM
      IC25       /
      IC28           - Unpopulated position for 32MBit SOP44 MaskROM
      E24-04         - TC5332201 32MBit SOP44 MaskROM
      E24-02, E24-03 - 23C16000 16MBit SOP44 MaskROM
      E24-06, E24-05 - M27C4001 4MBit DIP32 EPROM
      E24-09         - M27C4001 4MBit DIP32 EPROM
      E24-01         - AMD MACH111 CPLD stamped 'E24-01' (PLCC44)
      LH52B256       - Sharp 32K x8 SRAM (SOP28)
      LC321664       - Sanyo 64K x16 EDO DRAM (SOP40)
      MN1020012A     - Panasonic MN1020012A Sound CPU (QFP128)
      ZSG-2          - Zoom Corp ZSG-2 sound DSP (QFP100)
      TMS57002DPHA   - Texas Instruments TMS57002DPHA sound DSP (QFP80)
      M66220FP       - 256 x8bit Mail-Box Inter-MPU data transfer
      CAT702         - Protection chip labelled 'TT04' (DIP20)
      MB3773         - Power Supply Monitor with Watch Dog Timer (i.e. Reset IC)
      MB87078        - Electronic Volume Control IC
      FM1208S        - RAMTRON 4096bit Nonvolatile Ferroelectric RAM (512w x 8b)
*/

WRITE8_MEMBER(zn_state::bank_coh1000t_w)
{
	verboselog(1, "bank_coh1000t_w( %08x, %08x, %08x )\n", offset, data, mem_mask );

	m_mb3773->write_line_ck((data & 0x20) >> 5);

	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() + ( ( data & 3 ) * 0x800000 ) );
}

WRITE8_MEMBER(zn_state::fx1a_sound_bankswitch_w)
{
	membank( "bank10" )->set_base( memregion( "audiocpu" )->base() + 0x10000 + ( ( ( data - 1 ) & 0x07 ) * 0x4000 ) );
}

static ADDRESS_MAP_START(coh1000ta_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1f000000, 0x1f7fffff) AM_ROMBANK("bankedroms")
	AM_RANGE(0x1fb40000, 0x1fb40003) AM_WRITE8(bank_coh1000t_w, 0x000000ff)
	AM_RANGE(0x1fb80000, 0x1fb80003) AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x000000ff)
	AM_RANGE(0x1fb80000, 0x1fb80003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff0000)

	AM_IMPORT_FROM(zn_map)
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(zn_state,coh1000ta)
{
	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() );
}

static ADDRESS_MAP_START( fx1a_sound_map, AS_PROGRAM, 8, zn_state )
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank10")   /* Fallthrough */
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
	AM_RANGE(0xe200, 0xe200) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITENOP /* pan */
	AM_RANGE(0xee00, 0xee00) AM_NOP /* ? */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP /* ? */
	AM_RANGE(0xf200, 0xf200) AM_WRITE(fx1a_sound_bankswitch_w)
ADDRESS_MAP_END

/* handler called by the YM2610 emulator when the internal timers cause an IRQ */
WRITE_LINE_MEMBER(zn_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

static MACHINE_CONFIG_DERIVED( coh1000ta, zn1_1mb_vram )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1000ta_map)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_16MHz / 4)    /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(fx1a_sound_map)
	MCFG_MACHINE_RESET_OVERRIDE(zn_state, coh1000ta)

	MCFG_SOUND_ADD("ymsnd", YM2610B, XTAL_16MHz/2)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(zn_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.0)

	MCFG_MB3773_ADD("mb3773")

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END

WRITE8_MEMBER(zn_state::fx1b_fram_w)
{
	m_fx1b_fram[offset] = data;
}

READ8_MEMBER(zn_state::fx1b_fram_r)
{
	return m_fx1b_fram[offset];
}

static ADDRESS_MAP_START(coh1000tb_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1f000000, 0x1f7fffff) AM_ROMBANK("bankedroms")
	AM_RANGE(0x1fb00000, 0x1fb003ff) AM_READWRITE8(fx1b_fram_r, fx1b_fram_w, 0x00ff00ff)
	AM_RANGE(0x1fb40000, 0x1fb40003) AM_WRITE8(bank_coh1000t_w, 0x000000ff)
	AM_RANGE(0x1fb80000, 0x1fb80003) AM_DEVWRITE16("taito_zoom", taito_zoom_device, reg_data_w, 0x0000ffff)
	AM_RANGE(0x1fb80000, 0x1fb80003) AM_DEVWRITE16("taito_zoom", taito_zoom_device, reg_address_w, 0xffff0000)
	AM_RANGE(0x1fba0000, 0x1fba0003) AM_DEVWRITE16("taito_zoom", taito_zoom_device, sound_irq_w, 0x0000ffff)
	AM_RANGE(0x1fbc0000, 0x1fbc0003) AM_DEVREAD16("taito_zoom", taito_zoom_device, sound_irq_r, 0x0000ffff)
	AM_RANGE(0x1fbe0000, 0x1fbe01ff) AM_DEVREADWRITE8("taito_zoom", taito_zoom_device, shared_ram_r, shared_ram_w, 0x00ff00ff) // M66220FP for comm with the MN10200
	AM_IMPORT_FROM(zn_map)
ADDRESS_MAP_END

DRIVER_INIT_MEMBER(zn_state,coh1000tb)
{
	m_fx1b_fram = std::make_unique<UINT8[]>(0x200);
	machine().device<nvram_device>("fm1208s")->set_base(m_fx1b_fram.get(), 0x200);
}

MACHINE_RESET_MEMBER(zn_state,coh1000tb)
{
	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() ); /* banked game rom */
}

static MACHINE_CONFIG_DERIVED(coh1000tb, zn1_1mb_vram)

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1000tb_map)

	MCFG_MACHINE_RESET_OVERRIDE(zn_state, coh1000tb)
	MCFG_NVRAM_ADD_1FILL("fm1208s")

	MCFG_MB3773_ADD("mb3773")

	/* sound hardware */
	MCFG_SOUND_MODIFY("spu")
	MCFG_SOUND_ROUTES_RESET()
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.45)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.45)

	MCFG_FRAGMENT_ADD(taito_zoom_sound)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(coh1002tb, zn1_2mb_vram)

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1000tb_map)

	MCFG_MACHINE_RESET_OVERRIDE(zn_state, coh1000tb)
	MCFG_NVRAM_ADD_1FILL("fm1208s")

	MCFG_MB3773_ADD("mb3773")

	/* sound hardware */
	MCFG_SOUND_MODIFY("spu")
	MCFG_SOUND_ROUTES_RESET()
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.45)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.45)

	MCFG_FRAGMENT_ADD(taito_zoom_sound)
MACHINE_CONFIG_END

/*

Primal Rage 2
Atari, 1996

This game runs on Sony ZN1 hardware with a custom Atari top board.


PCB Layout
----------

Main board (Standard ZN1 Main Board with Atari BIOS)

ZN-1 1-659-709-12  COH-1000W
|--------------------------------------------------------|
|  LA4705             |---------------------------|      |
|                     |---------------------------|      |
|    AKM_AK4310VM      AT28C16                           |
|  VOL                                                   |
|       SW1            BIOS                              |
|  TD62064                                               |
|  TD62064        PALCE16V8                              |
|J                                                       |
|                                                        |
|A                EPM7032    814260    CXD2922CQ         |
|                                                        |
|M                                                       |
|                                                        |
|M                                                       |
|                KM4132G271Q-12                          |
|A           DSW                                         |
|                                CXD8561Q    CXD8530CQ   |
|                KM4132G271Q-12                          |
|CN505  CN506                   53.693MHz    67.737MHz   |
|            CAT702                                      |
|                                                        |
|CN504  CN503                                            |
|                                                        |
|                                                        |
|  NEC_78081G503         KM48V2104AT-6   KM48V2104AT-6   |
|            MC44200FT   KM48V2104AT-6   KM48V2104AT-6   |
|CN651  CN652                 *                *         |
|                CN654        *                *         |
|--------------------------------------------------------|
Notes:
      CN506 - Connector for optional 3rd player controls
      CN505 - Connector for optional 4th player controls
      CN503 - Connector for optional 15kHz external video output (R,G,B,Sync, GND)
      CN504 - Connector for optional 2nd speaker (for stereo output)
      CN652 - Connector for optional trackball
      CN651 - Connector for optional analog controls
      CN654 - Connector for optional memory card
      SW1   - Slide switch for stereo or mono sound output
      DSW   - Dip switch (4 position, defaults all OFF)

      BIOS           - COH1000W.353, Atari ZN1 BIOS, 4MBit MaskROM type M534002 (SOP40)
      AT28C16        - Atmel AT28C16 2K x8 EEPROM (SOP24)
      814260-70      - 256K x16 (4MBit) DRAM (SOJ40)
      KM4132G271Q-12 - 128K x32 x2 banks (32MBit) SGRAM (QFP100)
      KM48V2104AT-6  - Bank0: 2M x8 (16MBit) DRAM (SOP28).
                       * - Note Bank1 is empty.
      EPM7032        - Altera EPM7032QC44-15 CPLD labelled 'ZN1A' (QFP44)
      CAT702         - Protection chip labelled 'TW01' (DIP20)
      PALCE16V8      - PAL, labelled 'ZN1A' (PLCC20)
      NEC_78081G503  - NEC uPD78081 MCU, 5MHz


Game board

PSXTRA A055056-   055053-01 REV 1
Also printed on the board near the ROMs is....
"IM FEELING A LITTLE ANXIOUS, IF YOU KNOW WHAT I MEAN..."
|--------------------------------------|
|    |---------------------------|     |
|    |---------------------------|     |
|                                      |
|CAT702                           JGUN1|
|                                      |
|                                      |
|                           *1    JGUN2|
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|   DS1232S                       LED  |
|LED                                   |
|                                 IDE1 |
|                VT83C461              |
|                                      |
|   EPM7160ELC84                       |
|                                      |
|                                 IDE2 |
|                                      |
|                                      |
|*2                                    |
|PR2_036.U14   PR2_036.U16          NW1|
|      PR2_036.U15    PR2_036.U17   NW2|
|                            2631      |
|                            2631   NW3|
|--------------------------------------|
Notes:
      CAT702              - protection chip labelled 'TW02' (DIP20)
      JGUN1, JGUN2        - Connector for optional gun controllers
      ROMs U14 to U17     - 27C040 EPROM
      DS1232S             - Dallas DS1232 (reset IC, SOIC16)
      VT83C461            - VIA VT83C461 (IDE Hard Drive controller, QFP100)
      EPM7160ELC84        - Altera MAX EPM7160ELC84-10 (PLCC84 CPLD, labelled 'PSX PiD 9-19-96 2FDA')
      IDE1, IDE2          - 40 pin IDE Hard Drive connector, using Quantum Fireball 1080AT 1GB IDE hard drive.
      NW1                 - 8 pin RJ45 network connector labelled "SERIN"
      NW2                 - 8 pin RJ45 network connector labelled "SEROUT
      NW3                 - 8 pin RJ45 network connector labelled "SLONET"
      2631                - ICPL2631 IC (DIP8 x2)
      *1                  - Unpopulated position for PLCC44 IC
      *2                  - Unpopulated DIP28 socket
*/

void zn_state::atpsx_dma_read( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size )
{
//  logerror("DMA read: %d bytes (%d words) to %08x\n", n_size<<2, n_size, n_address);

	if (n_address < 0x10000)
	{
		logerror( "skip read to BIOS area\n" );
		return;
	}

	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* dma size is in 32-bit words, convert to words */
	n_size <<= 1;
	while( n_size > 0 )
	{
		psxwriteword( p_n_psxram, n_address, m_vt83c461->read_cs0(space, (UINT32) 0, (UINT32) 0xffff) );
		n_address += 2;
		n_size--;
	}
}

void zn_state::atpsx_dma_write( UINT32 *p_n_psxram, UINT32 n_address, INT32 n_size )
{
	logerror("DMA write from %08x for %d bytes\n", n_address, n_size<<2);
}

READ16_MEMBER(zn_state::vt83c461_16_r)
{
	int shift = (16 * (offset & 1));

	if( offset >= 0x30 / 2 && offset < 0x40 / 2 )
	{
		return m_vt83c461->read_config( space, ( offset / 2 ) & 3, mem_mask << shift ) >> shift;
	}
	else if( offset >= 0x1f0 / 2 && offset < 0x1f8 / 2 )
	{
		return m_vt83c461->read_cs0( space, ( offset / 2 ) & 1, (UINT32) mem_mask << shift ) >> shift;
	}
	else if( offset >= 0x3f0 / 2 && offset < 0x3f8 / 2 )
	{
		return m_vt83c461->read_cs1( space, ( offset / 2 ) & 1, (UINT32) mem_mask << shift ) >> shift;
	}
	else
	{
		logerror( "unhandled 16 bit read %04x %04x\n", offset, mem_mask );
		return 0xffff;
	}
}

WRITE16_MEMBER(zn_state::vt83c461_16_w)
{
	int shift = (16 * (offset & 1));

	if( offset >= 0x30 / 2 && offset < 0x40 / 2 )
	{
		m_vt83c461->write_config( space, ( offset / 2 ) & 3, data << shift, mem_mask << shift );
	}
	else if( offset >= 0x1f0 / 2 && offset < 0x1f8 / 2 )
	{
		m_vt83c461->write_cs0( space, ( offset / 2 ) & 1, (UINT32) data << shift, (UINT32) mem_mask << shift );
	}
	else if( offset >= 0x3f0 / 2 && offset < 0x3f8 / 2 )
	{
		m_vt83c461->write_cs1( space, ( offset / 2 ) & 1, (UINT32) data << shift, (UINT32) mem_mask << shift );
	}
	else
	{
		logerror( "unhandled 16 bit write %04x %04x %04x\n", offset, data, mem_mask );
	}
}

READ16_MEMBER(zn_state::vt83c461_32_r)
{
	if( offset == 0x1f0/2 )
	{
		UINT32 data = m_vt83c461->read_cs0(space, 0, 0xffffffff);
		m_vt83c461_latch = data >> 16;
		return data & 0xffff;
	}
	else if( offset == 0x1f2/2 )
	{
		return m_vt83c461_latch;
	}
	else
	{
		logerror( "unhandled 32 bit read %04x %04x\n", offset, mem_mask );
		return 0xffff;
	}
}

WRITE16_MEMBER(zn_state::vt83c461_32_w)
{
	logerror( "unhandled 32 bit write %04x %04x %04x\n", offset, data, mem_mask );
}

static ADDRESS_MAP_START(coh1000w_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1f000000, 0x1f1fffff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x1f000000, 0x1f000003) AM_WRITENOP
	AM_RANGE(0x1f7e8000, 0x1f7e8003) AM_NOP
	AM_RANGE(0x1f7e4000, 0x1f7e4fff) AM_READWRITE16(vt83c461_16_r, vt83c461_16_w, 0xffffffff)
	AM_RANGE(0x1f7f4000, 0x1f7f4fff) AM_READWRITE16(vt83c461_32_r, vt83c461_32_w, 0xffffffff)
	AM_IMPORT_FROM(zn_map)
ADDRESS_MAP_END

static MACHINE_CONFIG_DERIVED( coh1000w, zn1_2mb_vram )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1000w_map)

	MCFG_RAM_MODIFY("maincpu:ram")
	MCFG_RAM_DEFAULT_SIZE("8M")

	MCFG_VT83C461_ADD("ide", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("maincpu:irq", psxirq_device, intin10))
	MCFG_PSX_DMA_CHANNEL_READ( "maincpu", 5, psx_dma_read_delegate( FUNC( zn_state::atpsx_dma_read ), (zn_state *) owner ) )
	MCFG_PSX_DMA_CHANNEL_WRITE( "maincpu", 5, psx_dma_write_delegate( FUNC( zn_state::atpsx_dma_write ), (zn_state *) owner ) )
MACHINE_CONFIG_END

/*

Main board (Standard ZN1 Main Board with Raizing/8ing BIOS)

ZN-1 1-659-709-12  COH-1002E
|--------------------------------------------------------|
|  LA4705             |---------------------------|      |
|                     |---------------------------|      |
|    AKM_AK4310VM      AT28C16                           |
|  VOL                                                   |
|       SW1            BIOS                              |
|  TD62064                                               |
|  TD62064        PALCE16V8                              |
|J                                                       |
|                                                        |
|A                EPM7032    814260    CXD2925Q          |
|                                                        |
|M                                                       |
|                                                        |
|M                                                       |
|                KM4132G271Q-12                          |
|A           DSW                                         |
|                                CXD8561BQ   CXD8530CQ   |
|                KM4132G271Q-12                          |
|CN505  CN506                   53.693MHz    67.737MHz   |
|            CAT702                                      |
|                                                        |
|CN504  CN503                                            |
|                                                        |
|                                                        |
|  NEC_78081G503         KM48V514BJ-12   KM48V514BJ-12   |
|            MC44200FT   KM48V514BJ-12   KM48V514BJ-12   |
|CN651  CN652            KM48V514BJ-12   KM48V514BJ-12   |
|                CN654   KM48V514BJ-12   KM48V514BJ-12   |
|--------------------------------------------------------|
Notes:
      CN506 - Connector for optional 3rd player controls
      CN505 - Connector for optional 4th player controls
      CN503 - Connector for optional 15kHz external video output (R,G,B,Sync, GND)
      CN504 - Connector for optional 2nd speaker (for stereo output)
      CN652 - Connector for optional trackball
      CN651 - Connector for optional analog controls
      CN654 - Connector for optional memory card
      SW1   - Slide switch for stereo or mono sound output
      DSW   - Dip switch (4 position, defaults all OFF)

      BIOS           - COH1002E.353, Raizing/8ing ZN1 BIOS, 4MBit MaskROM type M53402CZ (SOP40)
      AT28C16        - Atmel AT28C16 2K x8 EEPROM (SOP24)
      814260-70      - 256K x16 (4MBit) DRAM (SOJ40)
      KM4132G271Q-12 - 128K x32 x2 banks (32MBit) SGRAM (QFP100)
      KM48V514BJ-6   - 512k x8 (4MBit) DRAM (SOJ28)
      EPM7032        - Altera EPM7032QC44-15 CPLD labelled 'ZN1A' (QFP44)
      CAT702         - Protection chip labelled 'ET01' (DIP20)
      PALCE16V8      - PAL, labelled 'ZN1A' (PLCC20)
      NEC_78081G503  - NEC uPD78081 MCU, 5MHz


Beastorizer Game board

RA9701 SUB
|--------------------------------------|
|    |---------------------------|     |
|    |---------------------------|     |
|NJM2060   RA-B.ROAR-1.217             |
|                    CAT702  GAL16V8B  |
|                                      |
|NJM2060                     B.ROAR.212|
|YAC513    RA-B.ROAR-2.216             |
|                                      |
|                                      |
|                            B.ROAR.214|
|          MAIN_IF2                    |
|                                      |
|                                      |
|                            B.ROAR.213|
|                                      |
|          SUB_IF2                     |
|                                      |
|16.93MHz                    B.ROAR.215|
|  YMF271-F                            |
|  (QFP128)                            |
|                                      |
|                            B.ROAR.042|
| RA-B.ROAR-3.326                      |
|                                      |
|                                      |
|  TMP68HC000N-16            B.ROAR.046|
|12MHz                 M628032  M628032|
|--------------------------------------|
Notes:
      CAT702              - protection chip labelled 'ET02' (DIP20)
      ROMs 217, 216 & 326 - surface mounted 32MBit MASK ROM (SOP44)
      ROMs 042 & 046      - 27C2001 EPROM
      ROMs 212 to 215     - 27C4001 EPROM
      MAIN_IF2 & SUB_IF2  - AMD Mach211 CPLD (PLCC44)
      M628032             - 32K x8 SRAM, equivalent to 62256 SRAM (SOJ28)
      68000 clock         - 12MHz
      YMF271-F clock      - 16.93MHz


Brave Blade / Bloody Roar 2 Game board

PS9805
|--------------------------------------|
|    |---------------------------|     |
|    |---------------------------|     |
|4560                    CAT702        |
|          FLASH0.021    GAL16V8B      |
|                                      |
| YAC516   FLASH1.024                  |
|                       ROM-1A.028     |
|         *FLASH2.025                  |
|                                      |
|                                      |
|             MAIN_IF2  ROM-1B.029     |
|16.93MHz                              |
|       YMF271-F                       |
|12MHz  (QFP128)                       |
|                       ROM-2A.026     |
|             SUB_IF2                  |
|                                      |
|                                      |
|    ROM-3.336          ROM-2B.210     |
|                                      |
|TMP68HC000N-16                        |
|                                      |
|                       *MASK4A.027    |
|                                      |
|    BR2_U0412.412    N341256          |
|                     N341256          |
|    BR2_U049.049       *MASK4B.016    |
|--------------------------------------|
Notes:
      *                   - Unpopulated ROM positions.
      CAT702              - protection chip labelled 'MG11' (DIP20)
      ROM-x               - surface mounted 32MBit MASK ROM (SOP44)
      ROMs 412 & 049      - 27C040 EPROM
      MASK4A              - smt solder pads (unpopulated)
      MASK4B              - DIP42 socket (unpopulated)
      FLASHx              - surface mounted TSOP56 16MBit FlashROM type Sharp LH28F160S5T-L10
      MAIN_IF2 & SUB_IF2  - AMD Mach211 CPLD (PLCC44)
      N341256             - 32K x8 SRAM, equivalent to 62256 SRAM (SOJ28)
      68000 clock         - 12MHz
      YMF271-F clock      - 16.93MHz


*/

WRITE8_MEMBER(zn_state::coh1002e_bank_w)
{
	znsecsel_w( space, offset, data, mem_mask );

	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() + ( ( data & 3 ) * 0x800000 ) );
}

WRITE8_MEMBER(zn_state::coh1002e_sound_irq_w)
{
	m_audiocpu->set_input_line(2, HOLD_LINE); // irq 2 on the 68k
}

static ADDRESS_MAP_START(coh1002e_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1f000000, 0x1f7fffff) AM_ROMBANK("bankedroms")
	AM_RANGE(0x1fa10300, 0x1fa10303) AM_WRITE8(coh1002e_bank_w, 0x000000ff)
	AM_RANGE(0x1fb00000, 0x1fb00003) AM_WRITE8(soundlatch_byte_w, 0x000000ff)
	AM_RANGE(0x1fb00004, 0x1fb00007) AM_WRITE8(coh1002e_sound_irq_w, 0x000000ff)

	AM_IMPORT_FROM(zn_map)
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(zn_state,coh1002e)
{
	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() );
}

static ADDRESS_MAP_START( psarc_snd_map, AS_PROGRAM, 16, zn_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x0fffff) AM_RAM
	AM_RANGE(0x100000, 0x10001f) AM_DEVREADWRITE8("ymf", ymf271_device, read, write, 0x00ff )
	AM_RANGE(0x180008, 0x180009) AM_READ8(soundlatch_byte_r, 0x00ff )
	AM_RANGE(0x000000, 0x07ffff) AM_WRITENOP
	AM_RANGE(0x100020, 0xffffff) AM_WRITENOP
ADDRESS_MAP_END

static MACHINE_CONFIG_DERIVED(coh1002e, zn1_2mb_vram)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1002e_map)

	MCFG_CPU_ADD("audiocpu", M68000, XTAL_12MHz)
	MCFG_CPU_PROGRAM_MAP(psarc_snd_map)

	MCFG_MACHINE_RESET_OVERRIDE(zn_state, coh1002e)

	MCFG_SOUND_ADD("ymf", YMF271, XTAL_16_9344MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


/*

Bust A Move 2

Runs on ZN1 hardware
Lower PCB is common ZN1 with COH-1002E bios and ET01 sec chip
Top PCB is unique for this game....

MTR990601-(A)
|----------------------------------------------|
|  ALTERA-MAX   CN5  H8/3644       IDE-40      |
|  EPM7128STC100                               |
|                                              |
|            MTR-BAM-A01.U23   MTR-BAM-A06.U28 |
|   FLASH.U19                                  |
|            MTR-BAM-A02.U24   MTR-BAM-A07.U29 |
|   FLASH.U20                                  |
|            MTR-BAM-A03.U25   MTR-BAM-A08.U30 |
|  *FLASH.U21                                  |
|            MTR-BAM-A04.U26   MTR-BAM-A09.U31 |
|  *FLASH.U22                                  |
|SEC         MTR-BAM-A05.U27   MTR-BAM-A10.U32 |
|    CN3       4560   4560       TC9293        |
|----------------------------------------------|
Notes:
       * - Not populated
   FLASH - MX29F1610 SOP44 flashROMs
MTR-BAM* - DIP42 32MBit maskROMs
  TC9293 - Toshiba TC9293 Modulation System DAC with Analog Filter
    4560 - JRC 4560 Op Amp
     SEC - CAT702 security IC
 CN3/CN5 - Connectors for ? (controls?)
  IDE-40 - 40 Pin flat cable connector for IDE HDD
           HDD is 3.5" Quantum Fireball CR 4.3AT

*/

/*
    H8/3644 MCU comms: there are 4 16-bit read/write ports

    Port 0: (R) unknown
    Port 0: (W) bank for mask ROMs
    Port 1: (R) MCU status: bit 3 = busy, bit 2 = command successful, bits 1 & 0 = error
    Port 1: (W) MCU command (0x0000 = execute)
    Port 2: (R) unknown
    Port 2: (W) MIPS writes alternating 0xffff/0xfffe, possibly to watchdog the H8?
    Port 3: (R) unknown
    Port 3: (W) unknown

    8007f538 = detected device type.  0 = CDROM, 1 = HDD.
*/


WRITE16_MEMBER(zn_state::bam2_mcu_w)
{
	switch( offset )
	{
	case 0:
		membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() + ( ( data & 0xf ) * 0x400000 ) );
		break;

	case 1:
		m_bam2_mcu_command = data;
		logerror("MCU command: %04x (PC %08x)\n", m_bam2_mcu_command, space.device().safe_pc());
		break;
	}
}

READ16_MEMBER(zn_state::bam2_mcu_r)
{
	switch (offset)
	{
	case 0:
		logerror("MCU port 0 read @ PC %08x mask %08x\n", space.device().safe_pc(), mem_mask);
		break;

	case 2:
		logerror("MCU status read @ PC %08x mask %08x\n", space.device().safe_pc(), mem_mask);

		switch (m_bam2_mcu_command)
		{
			case 0x7f:      // first drive check
			case 0x1c:      // second drive check (causes HDD detected)
				return 1;   // return error
		}

		return 4;           // return OK
	}

	return 0;
}

READ16_MEMBER(zn_state::bam2_unk_r)
{
	return 0;
}

static ADDRESS_MAP_START(bam2_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1f000000, 0x1f3fffff) AM_ROM AM_REGION("bankedroms", 0)
	AM_RANGE(0x1f400000, 0x1f7fffff) AM_ROMBANK("bankedroms")
	AM_RANGE(0x1fa20000, 0x1fa20003) AM_READ16(bam2_unk_r, 0x0000ffff)
	AM_RANGE(0x1fb00000, 0x1fb00007) AM_READWRITE16(bam2_mcu_r, bam2_mcu_w, 0xffffffff)

	AM_IMPORT_FROM(zn_map)
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(zn_state,bam2)
{
	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() + 0x400000 );
}

static MACHINE_CONFIG_DERIVED( bam2, zn1_2mb_vram )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(bam2_map)

	MCFG_MACHINE_RESET_OVERRIDE(zn_state, bam2)
MACHINE_CONFIG_END

/*

Judge Dread

light-gun type shooting game

Uses the Sony ZN-1 hardware with Rom board and
Hard disk Drive

U35 and U36 eproms are 27c1001 are believed to be the bios
data.

Disk Drive is a Quantum ????2.1 GB??

connectors CN506 and CN505 are the gun inputs pins 13, 14, 15
white/blue/ black respectively.   The + 5 (red) is separate source (not
from the CN506 or Cn505).

You'll have to get the +5 for the guns from the jamma harness.


NBA Jam Extreme
Acclaim, 1996


Main board (Standard ZN1 Main Board with Acclaim ZN1 BIOS)
ZN-1 1-659-709-11  COH-1000A
|--------------------------------------------------------|
|  LA4705             |---------------------------|      |
|                     |---------------------------|      |
|    AKM_AK4310VM      AT28C16                           |
|  VOL                                                   |
|       SW1            BIOS                              |
|  TD62064                                               |
|  TD62064        PALCE16V8                              |
|J                                                       |
|                                                        |
|A                EPM7032   TC51V4260BJ-80   CXD2922CQ   |
|                                                        |
|M                                                       |
|                                                        |
|M                                                       |
|                KM4132G271Q-12                          |
|A           DSW                                         |
|                                CXD8561Q    CXD8530CQ   |
|                KM4132G271Q-12                          |
|CN505  CN506                   53.693MHz    67.737MHz   |
|            CAT702                                      |
|                                                        |
|CN504  CN503                                            |
|                                                        |
|                                                        |
|  NEC_78081G503         424805AL-A60    424805AL-A60    |
|            MC44200FT   424805AL-A60    424805AL-A60    |
|CN651  CN652            424805AL-A60    424805AL-A60    |
|                CN654   424805AL-A60    424805AL-A60    |
|--------------------------------------------------------|
Notes:
      CN506 - Connector for optional 3rd player controls
      CN505 - Connector for optional 4th player controls
      CN503 - Connector for optional 15kHz external video output (R,G,B,Sync, GND)
      CN504 - Connector for optional 2nd speaker (for stereo output)
      CN652 - Connector for optional trackball
      CN651 - Connector for optional analog controls
      CN654 - Connector for optional memory card
      SW1   - Slide switch for stereo or mono sound output
      DSW   - Dip switch (4 position, defaults all OFF)

      BIOS           - COH1000A.353, Acclaim ZN1 BIOS, 4MBit MaskROM type M534002 (SOP40)
      AT28C16        - Atmel AT28C16 2K x8 EEPROM (SOP24)
      TC51V4260BJ-80 - 256K x16 (4MBit) DRAM (SOJ40)
      KM4132G271Q-12 - 128K x32 x2 banks (32MBit) SGRAM (QFP100)
      424805AL-A60   - 512k x8 (4MBit) DRAM (SOJ28)
      EPM7032        - Altera EPM7032QC44-15 CPLD labelled 'ZN1A' (QFP44)
      CAT702         - Protection chip labelled 'AC01' (DIP20)
      PALCE16V8      - PAL, labelled 'ZN1A' (PLCC20)
      NEC_78081G503  - NEC uPD78081 MCU, 5MHz


Game board

PCB-100102
|--------------------------------------|
|LED |---------------------------|     |
|    |---------------------------|     |
|                                      |
|71256                                 |
|       ADM619AR                   CN1 |
|                                      |
|  BATT_3V     A1425       CAT702      |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|NBA0E.U41 NBA0O.U28 NBA4E.U17 NBA4O.U3|
|                                      |
|                                      |
|NBA1E.U42 NBA1O.U29 NBA5E.U18 NBA5O.U4|
|                                      |
|                                      |
|NBA2E.U43 NBA2O.U30 NBA6E.U19 NBA6O.U5|
|                                      |
|                                      |
|NBA3E.U44 NBA3O.U31       U20       U6|
|                                      |
| 360-MP-A1_EVEN.U35       U21         |
|                                      |
|  360-MP-A1_ODD.U36       U22         |
|                                      |
|--------------------------------------|
Notes:
      CN1      - 40 pin IDC connector (for 40 pin flat cable joining game PCB to sound PCB)
      CAT702   - protection chip labelled 'AC02' (DIP20)
      71256    - 32K x8 SRAM
      BATT_3V  - 3 Volt coin battery (CR2032)
      ADM619AR - 900MHz RF Transceiver (SOIC16, compatible to AD6190)
      A1425    - Actel A1425A-2 PQ100C 9536 (FPGA, QFP100, labelled 'PD-11010A REV A 05/21 C/S EEC1')
      U35, U36 - 27C080 DIP32 EPROM
      U21, U22 - Unpopulated positions for DIP32 EPROM
      U6, U20  - Unpopulated position for SOP44 MaskROM
      U3  to  U6  \
      U17 to U19  |  surface mounted 32MBit SOP44 MaskROM
      U28 to U31  |
      U41 to U44  /


Sound Board

PCB-100095
|---------------------------|
|  TDA7240A  TDA7240A       |
|                    LMC6484|
|                           |
|                    LMC6484|
|                           |
|CN1         AD1866  LMC6484|
|                           |
|                           |
|                  LED      |
|                           |
|                  LED      |
|              *        *   |
|                           |
|              *        *   |
|                           |
|           SND1.U49    *   |
| 16.67MHz                  |
|                           |
|           SND0.U48    *   |
| ADSP-2181                 |
|                           |
|             360-SND-A1.U52|
| 52258   MACH111           |
| 52258                     |
|---------------------------|
Notes:
      CN1       - 40 pin IDC connector (for 40 pin flat cable joining sound PCB to game PCB)
      AD1866    - Dual 16bit Audio DAC
      52258     - Sharp LH52258 32K x8 SRAM (SOJ28)
      TDA7240A  - 20W Bridge Amplifier
      LMC6484   - CMOS Quad Rail-to-Rail Input and Output Operational Amplifier
      MACH111   - AMD MACH111 CPLD (PLCC44, labelled '360 PLD-A1 CS=0794')
      ADSP-2181 - Analog Devices DSP (QFP128, 16Bit, 40 MIPs, 5V, 2 serial ports, 16Bit internal DMA
                  port, a byte DMA port, programmable timer, 80K on-chip memory configured as
                  16K words (24 Bit) RAM and 16K data (16Bit) RAM
      U48, U49  - 32MBit DIP42 MaskROM
      U52       - 27C040 DIP32 EPROM labelled '360-SND-A1 IC110345 CS = 7D5A'
      *         - Unpopulated DIP42 socket
*/

CUSTOM_INPUT_MEMBER(zn_state::jdredd_gun_mux_read)
{
	return m_jdredd_gun_mux;
}

void zn_state::jdredd_vblank(screen_device &screen, bool vblank_state)
{
	int x;
	int y;

	if( vblank_state )
	{
		m_jdredd_gun_mux = !m_jdredd_gun_mux;

		if( m_jdredd_gun_mux == 0 )
		{
			x = ioport("GUN1X")->read();
			y = ioport("GUN1Y")->read();
		}
		else
		{
			x = ioport("GUN2X")->read();
			y = ioport("GUN2Y")->read();
		}

		if( x > 0x393 && x < 0xcb2 &&
			y > 0x02d && y < 0x217 )
		{
			m_gpu->lightgun_set( x, y );
		}
	}
}

WRITE16_MEMBER(zn_state::acpsx_00_w)
{
	verboselog(0, "acpsx_00_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
}

WRITE16_MEMBER(zn_state::nbajamex_bank_w)
{
	UINT32 newbank = 0;

	verboselog(0, "nbajamex_bank_w( %08x, %08x, %08x )\n", offset, data, mem_mask );

	if (offset > 1)
	{
		logerror("Unknown banking offset %x!\n", offset);
	}

	if (offset == 1)
	{
		data -= 1;
	}

	if (data <= 1)
	{
		newbank = (data * 0x400000);
	}
	else if (data >= 0x10)
	{
		data -= 0x10;
		newbank = (data * 0x400000);
		newbank += 0x200000;
	}

	if (offset == 0)
	{
		membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() + newbank);
	}
	else if (offset == 1)
	{
		newbank += 0x200000;
		membank( "bankedroms2" )->set_base( memregion( "bankedroms" )->base() + newbank);
	}
}

WRITE16_MEMBER(zn_state::acpsx_10_w)
{
	verboselog(0, "acpsx_10_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
}

// all 16 bits goes to the external soundboard's latch (see sound test menu)
WRITE16_MEMBER(zn_state::nbajamex_80_w)
{
	verboselog(0, "nbajamex_80_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
	psxirq_device *psxirq = (psxirq_device *) machine().device("maincpu:irq");
	psxirq->intin10(1);
}

READ16_MEMBER(zn_state::nbajamex_08_r)
{
	UINT32 data = 0xffffffff;
	verboselog(0, "nbajamex_08_r( %08x, %08x, %08x )\n", offset, data, mem_mask );
	return data;
}

// possibly a readback from the external soundboard?
READ16_MEMBER(zn_state::nbajamex_80_r)
{
	UINT32 data = 0xffffffff;
	verboselog(0, "nbajamex_80_r( %08x, %08x, %08x )\n", offset, data, mem_mask );
	return data;
}

static ADDRESS_MAP_START(coh1000a_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1fbfff00, 0x1fbfff03) AM_WRITE16(acpsx_00_w, 0xffffffff)
	AM_RANGE(0x1fbfff10, 0x1fbfff13) AM_WRITE16(acpsx_10_w, 0xffff0000)

	AM_IMPORT_FROM(zn_map)
ADDRESS_MAP_END

static ADDRESS_MAP_START(nbajamex_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1f000000, 0x1f1fffff) AM_ROMBANK("bankedroms")
	AM_RANGE(0x1f200000, 0x1f7fffff) AM_ROMBANK("bankedroms2")
	AM_RANGE(0x1fbfff00, 0x1fbfff07) AM_WRITE16(nbajamex_bank_w, 0xffffffff)
	AM_RANGE(0x1fbfff08, 0x1fbfff0b) AM_READ16(nbajamex_08_r, 0xffff)
	AM_RANGE(0x1fbfff80, 0x1fbfff83) AM_READWRITE16(nbajamex_80_r, nbajamex_80_w, 0xffff)

	AM_IMPORT_FROM(coh1000a_map)
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(zn_state,nbajamex)
{
	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() );
	membank( "bankedroms2" )->set_base( memregion( "bankedroms" )->base() + 0x200000 );
}

static ADDRESS_MAP_START(jdredd_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1f000000, 0x1f1fffff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x1fbfff80, 0x1fbfff8f) AM_DEVREADWRITE16("ata", ata_interface_device, read_cs1, write_cs1, 0xffffffff)
	AM_RANGE(0x1fbfff90, 0x1fbfff9f) AM_DEVREADWRITE16("ata", ata_interface_device, read_cs0, write_cs0, 0xffffffff)

	AM_IMPORT_FROM(coh1000a_map)
ADDRESS_MAP_END

static MACHINE_CONFIG_DERIVED( coh1000a, zn1_2mb_vram )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1000a_map)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nbajamex, zn1_2mb_vram )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(nbajamex_map)
	MCFG_MACHINE_RESET_OVERRIDE(zn_state, nbajamex)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( jdredd, zn1_2mb_vram )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(jdredd_map)

	MCFG_DEVICE_MODIFY("gpu")
	MCFG_PSXGPU_VBLANK_CALLBACK(vblank_state_delegate( FUNC( zn_state::jdredd_vblank ), (zn_state *) owner))

	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("maincpu:irq", psxirq_device, intin10))
MACHINE_CONFIG_END

/*

Main board (Standard ZN1 Main Board with Atlus BIOS)

ZN-1 1-659-709-12  COH-1001L
|--------------------------------------------------------|
|  LA4705             |---------------------------|      |
|                     |---------------------------|      |
|    AKM_AK4310VM      AT28C16                           |
|  VOL                                                   |
|       SW1            BIOS                              |
|  TD62064                                               |
|  TD62064        PALCE16V8                              |
|J                                                       |
|                                                        |
|A                EPM7032     814260     CXD2925Q        |
|                                                        |
|M                                                       |
|                                                        |
|M                                                       |
|                                                        |
|A           DSW                                         |
|                                CXD8561Q    CXD8530CQ   |
|                MB81G83222-012                          |
|CN505  CN506                   53.693MHz    67.737MHz   |
|            CAT702                                      |
|                                                        |
|CN504  CN503                                            |
|                                                        |
|                                                        |
|  NEC_78081G503         KM48V514BJ-12   KM48V514BJ-12   |
|            MC44200FT   KM48V514BJ-12   KM48V514BJ-12   |
|CN651  CN652            KM48V514BJ-12   KM48V514BJ-12   |
|                CN654   KM48V514BJ-12   KM48V514BJ-12   |
|--------------------------------------------------------|
Notes:
      CN506 - Connector for optional 3rd player controls
      CN505 - Connector for optional 4th player controls
      CN503 - Connector for optional 15kHz external video output (R,G,B,Sync, GND)
      CN504 - Connector for optional 2nd speaker (for stereo output)
      CN652 - Connector for optional trackball
      CN651 - Connector for optional analog controls
      CN654 - Connector for optional memory card
      SW1   - Slide switch for stereo or mono sound output
      DSW   - Dip switch (4 position, defaults all OFF)

      BIOS           - coh1001l.353, Atlus ZN1 BIOS, 4MBit MaskROM type M534002 (SOP40)
      AT28C16        - Atmel AT28C16 2K x8 EEPROM
      814260         - 256K x16 (4MBit) DRAM
      MB81G83222-012 - 128K x32 x2 banks (32MBit) SGRAM
      KM48V514BJ-6   - 512k x8 (4MBit) DRAM
      EPM7032        - Altera EPM7032QC44-15 CPLD labelled 'ZN1A' (QFP44)
      CAT702         - Protection chip labelled 'AT01' (DIP20)
      PALCE16V8      - PAL, labelled 'ZN1A' (PLCC20)
      NEC_78081G503  - NEC uPD78081 MCU, 5MHz


Game board
----------

ATHG-01
|--------------------------------------|
|    |---------------------------|     |
|    |---------------------------|     |
|LM358  LM358  PAL(1) PAL(2)           |
|                                      |
|YAC513 LM358                          |
|                            62256     |
|              CAT702        62256     |
|                                      |
|                                      |
| 16.9344MHz                           |
| YMZ280B      ATHG-01B.18             |
|                                      |
|                       ATHG-02B.17    |
|                                      |
|                                      |
|              ATHG-03.22     10MHz    |
|                                      |
|ATHG-06.4134           ATHG-04.21     |
|                                      |
|     ATHG-05.4136            68000    |
|                                      |
|                                PAL(3)|
|                                      |
|ATHG-11.215  ATHG-09.210   ATHG-07.027|
|                                      |
|                                      |
|    ATHG-10.029    ATHG-08.028        |
|--------------------------------------|
Notes:
      PAL(1)  - labelled 'ROM1'
      PAL(2)  - labelled 'ROM2'
      PAL(3)  - labelled 'ROM3'
      CAT702  - Protection chip labelled 'AT02' (DIP20)
      62256   - 32K x8 SRAM

      ATHG-01B.18   - Main program (27C040 EPROM)
      ATHG-02B.17   /

      ATHG-03.22    - Sound program (27C010 EPROM)
      ATHG-04.21    /

      ATHG-05.4136  - Sound data (16MBit DIP42 MASKROM)
      ATHG-06.4134  /

      ATHG-07.027   - Graphics data (32MBit DIP42 MASKROM)
      ATHG-08.028   /
      ATHG-09.210   /
      ATHG-10.029   /
      ATHG-11.215   /

      68000 clock  - 10.000MHz
      YMZ280 clock - 16.9344MHz
      VSync        - 60Hz
*/

WRITE16_MEMBER(zn_state::coh1001l_sound_unk_w)
{
	// irq ack maybe?
	logerror("coh1001l_sound_unk_w: %04x %04x\n", data, mem_mask);
}

WRITE16_MEMBER(zn_state::coh1001l_latch_w)
{
	soundlatch_word_w(space, 0, data);
	m_audiocpu->set_input_line(3, HOLD_LINE);
}

WRITE8_MEMBER(zn_state::coh1001l_bank_w)
{
	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() + ( ( data & 3 ) * 0x800000 ) );
}

static ADDRESS_MAP_START(coh1001l_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1f000000, 0x1f7fffff) AM_ROMBANK("bankedroms")
	AM_RANGE(0x1fb00000, 0x1fb00003) AM_WRITE16(coh1001l_latch_w, 0x0000ffff)
	AM_RANGE(0x1fb00000, 0x1fb00003) AM_WRITE8(coh1001l_bank_w, 0x00ff0000)

	AM_IMPORT_FROM(zn_map)
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(zn_state,coh1001l)
{
	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() ); /* banked rom */
}

static ADDRESS_MAP_START( atlus_snd_map, AS_PROGRAM, 16, zn_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_READWRITE(soundlatch_word_r, coh1001l_sound_unk_w)
	AM_RANGE(0x200000, 0x200003) AM_DEVREADWRITE8("ymz", ymz280b_device, read, write, 0x00ff)
	AM_RANGE(0x700000, 0x70ffff) AM_RAM
ADDRESS_MAP_END


static MACHINE_CONFIG_DERIVED(coh1001l, zn1_2mb_vram)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1001l_map)

	MCFG_CPU_ADD("audiocpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(atlus_snd_map)

	MCFG_MACHINE_RESET_OVERRIDE(zn_state, coh1001l)

	MCFG_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MCFG_YMZ280B_IRQ_HANDLER(INPUTLINE("audiocpu", 2))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.35)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.35)
MACHINE_CONFIG_END

/*

Sonic Wings Limited (JPN Ver.)
(c)1996 Video System

Board:  PS based (ZN-1,COH-1002V)
    VS34 (ROM board)

Key:    Mother    KN01
    ROM board KN02

*/

WRITE8_MEMBER(zn_state::coh1002v_bank_w)
{
	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() + ( data * 0x100000 ) );
}

static ADDRESS_MAP_START(coh1002v_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1f000000, 0x1f27ffff) AM_ROM AM_REGION("fixedroms", 0)
	AM_RANGE(0x1fb00000, 0x1fbfffff) AM_ROMBANK("bankedroms")
	AM_RANGE(0x1fb00000, 0x1fb00003) AM_WRITE8(coh1002v_bank_w, 0x000000ff)

	AM_IMPORT_FROM(zn_map)
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(zn_state,coh1002v)
{
	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() ); /* banked rom */
}

static MACHINE_CONFIG_DERIVED( coh1002v, zn1_2mb_vram )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1002v_map)

	MCFG_MACHINE_RESET_OVERRIDE(zn_state, coh1002v)
MACHINE_CONFIG_END

/*

Main board (Standard ZN1 Main Board with Tecmo BIOS)

ZN-1 1-659-709-12  COH-1002M
|--------------------------------------------------------|
|  LA4705             |---------------------------|      |
|                     |---------------------------|      |
|    AKM_AK4310VM      AT28C16                           |
|  VOL                                                   |
|       SW1            BIOS                              |
|  TD62064                                               |
|  TD62064        PALCE16V8                              |
|J                                                       |
|                                                        |
|A                EPM7032   M5M44260     CXD2925Q        |
|                                                        |
|M                                                       |
|                                                        |
|M                                                       |
|                KM4132G271Q-12                          |
|A           DSW                                         |
|                                CXD8561Q    CXD8530CQ   |
|                KM4132G271Q-12                          |
|CN505  CN506                   53.693MHz    67.737MHz   |
|            CAT702                                      |
|                                                        |
|CN504  CN503                                            |
|                                                        |
|                                                        |
|  NEC_78081G503         KM48V514BJ-12   KM48V514BJ-12   |
|            MC44200FT   KM48V514BJ-12   KM48V514BJ-12   |
|CN651  CN652            KM48V514BJ-12   KM48V514BJ-12   |
|                CN654   KM48V514BJ-12   KM48V514BJ-12   |
|--------------------------------------------------------|
Notes:
      CN506 - Connector for optional 3rd player controls
      CN505 - Connector for optional 4th player controls
      CN503 - Connector for optional 15kHz external video output (R,G,B,Sync, GND)
      CN504 - Connector for optional 2nd speaker (for stereo output)
      CN652 - Connector for optional trackball
      CN651 - Connector for optional analog controls
      CN654 - Connector for optional memory card
      SW1   - Slide switch for stereo or mono sound output
      DSW   - Dip switch (4 position, defaults all OFF)

      BIOS           - COH1002M.353, Tecmo ZN1 BIOS, 4MBit MaskROM type M534002 (SOP40)
      AT28C16        - Atmel AT28C16 2K x8 EEPROM
      M5M44260       - 256K x16 (4MBit) DRAM
      KM4132G271Q-12 - 128K x32 x2 banks (32MBit) SGRAM
      KM48V514BJ-6   - 512k x8 (4MBit) DRAM
      EPM7032        - Altera EPM7032QC44-15 CPLD labelled 'ZN1A' (QFP44)
      CAT702         - Protection chip labelled 'MG01' (DIP20)
      PALCE16V8      - PAL, labelled 'ZN1A' (PLCC20)
      NEC_78081G503  - NEC uPD78081 MCU, 5MHz


Game board with sound

Tecmo TPS1-7
|--------------------------------------|
|    |---------------------------|     |
|    |---------------------------|     |
|LM358  GAL16V8B(1)  CAT702            |
|YAC513  LM358  CBAJ1.119  CBAJ2.120   |
|        LM358                         |
|                                      |
|                                      |
| CB-VO  CB-SE            CB-08  CB-07 |
|                                      |
|                                      |
|      CB-05     CB-03     CB-01       |
| CB-06     CB-04     CB-02     CB-00  |
|                                      |
|                                      |
|                                      |
|                                      |
|YMZ280B  16.9MHz  GAL16V8(2)  LH54020 |
|                  GAL16V8(3)          |
|                  GAL16V8(4)  LH54020 |
|                                      |
|                                      |
|                                      |
| 32MHz  GAL16V8(5)         GAL16V8(6) |
|                   D43001             |
|                                      |
|                                      |
|             CBAJZ80.3118     Z80     |
|--------------------------------------|
Notes:
      There are a few unpopulated positions on this game board, including
      4 unpopulated positions for 4x 32MBit smt SOP44 MASKROMs
      1 unpopulated position for uPD72103AG near the D43001 RAM
      2 unpopulated positions for 2 connectors near the Z80 ROM possibly for a network link?
      1 unpopulated position for a PAL16V8 near ROM 'CBAJ2'

      This board contains....
      PAL16V8B(1) labelled 'SOPROM1'
      PAL16V8B(2) labelled 'SOPROM3'
      PAL16V8B(3) labelled 'SOPROM4C'
      PAL16V8B(4) labelled 'SOPROM5B'
      PAL16V8B(5) labelled 'SOPROM6A'
      PAL16V8B(6) labelled 'SOPROM2B'
      CAT702 Protection chip labelled 'MG03' (DIP20)
      3 logic chips near main program ROMs.
      2x 4MBit EPROMs labelled 'CBAJ1' and 'CBAJ2'
      1x 2MBit EPROM labelled 'CBAJZ80'
      9x 32MBit smt SOP44 MASKROMs labelled 'CB-00' through 'CB-08' (Graphics)
      2x 32MBit smt SOP44 MASKROMs labelled 'CB-SE' and 'CB-V0' (connected to the YMZ280B)
      LH540202 - CMOS 1024 x 9 Asyncronous FIFO (PLCC32)
      D43001   - 32K x8 SRAM, equivalent to 62256 SRAM

      Z80 clock: 4.000MHz
      VSync    : 60Hz

Game board without sound

Tecmo TPS1-7
|--------------------------------------|
|    |---------------------------|     |
|    |---------------------------|     |
|       GAL16V8B    CAT702             |
|              SHMJ-B.119  SHMJ-A.120  |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|  SH-03.220       SH-01.218           |
|         SH-02.219       SH-00.217    |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|                                      |
|--------------------------------------|
Notes:
      There are many unpopulated positions on this game board. This game only contains
      the following parts...
      PAL16V8B labelled 'SOPROM1'
      CAT702 Protection chip labelled 'MG04' (DIP20)
      3 logic chips
      2x 4MBit EPROMs labelled 'SHMJ-B' and 'SHMJ-A'
      4x 32MBit smt SOP44 MASKROMs labelled 'SH03, 'SH02', 'SH01' & 'SH00'. There is space
      for 11 more 32MBit smt SOP44 MASKROMs.
*/

WRITE8_MEMBER(zn_state::coh1002m_bank_w)
{
	verboselog(1, "coh1002m_bank_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() + (data * 0x800000) );
}

static ADDRESS_MAP_START(coh1002m_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1f000000, 0x1f7fffff) AM_ROMBANK("bankedroms")
	AM_RANGE(0x1fb00004, 0x1fb00007) AM_WRITE8(coh1002m_bank_w, 0x00ff0000)

	AM_IMPORT_FROM(zn_map)
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(zn_state,coh1002m)
{
	membank( "bankedroms" )->set_base( memregion( "bankedroms" )->base() );
}

static MACHINE_CONFIG_DERIVED( coh1002m, zn1_2mb_vram )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1002m_map)

	MCFG_MACHINE_RESET_OVERRIDE(zn_state, coh1002m)
MACHINE_CONFIG_END

READ8_MEMBER(zn_state::cbaj_sound_main_status_r)
{
	// d1: fifo empty flag, other bits: unused(?)
	return m_cbaj_fifo2->ef_r() << 1;
}

static ADDRESS_MAP_START(coh1002msnd_map, AS_PROGRAM, 32, zn_state)
	AM_RANGE(0x1fb00000, 0x1fb00003) AM_DEVREAD8("cbaj_fifo2", fifo7200_device, data_byte_r, 0x000000ff)
	AM_RANGE(0x1fb00000, 0x1fb00003) AM_DEVWRITE8("cbaj_fifo1", fifo7200_device, data_byte_w, 0x000000ff)
	AM_RANGE(0x1fb00000, 0x1fb00003) AM_READ8(cbaj_sound_main_status_r, 0xff000000)

	AM_IMPORT_FROM(coh1002m_map)
ADDRESS_MAP_END

READ8_MEMBER(zn_state::cbaj_sound_z80_status_r)
{
	// d1: fifo empty flag, other bits: unused
	return m_cbaj_fifo1->ef_r() << 1;
}

static ADDRESS_MAP_START( cbaj_z80_map, AS_PROGRAM, 8, zn_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cbaj_z80_port_map, AS_IO, 8, zn_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x84, 0x85) AM_DEVREADWRITE("ymz", ymz280b_device, read, write)
	AM_RANGE(0x90, 0x90) AM_DEVREAD("cbaj_fifo1", fifo7200_device, data_byte_r)
	AM_RANGE(0x90, 0x90) AM_DEVWRITE("cbaj_fifo2", fifo7200_device, data_byte_w)
	AM_RANGE(0x91, 0x91) AM_READ(cbaj_sound_z80_status_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( coh1002ml_link_map, AS_PROGRAM, 8, zn_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( coh1002ml_link_port_map, AS_IO, 8, zn_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static MACHINE_CONFIG_DERIVED( coh1002msnd, coh1002m )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(coh1002msnd_map)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_32MHz/8)
	MCFG_CPU_PROGRAM_MAP(cbaj_z80_map)
	MCFG_CPU_IO_MAP(cbaj_z80_port_map)

	MCFG_FIFO7200_ADD("cbaj_fifo1", 0x400) // LH540202
	MCFG_FIFO7200_ADD("cbaj_fifo2", 0x400) // "

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* sound hardware */
	MCFG_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.35)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.35)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( coh1002ml, coh1002m )
	MCFG_CPU_ADD("link", Z80, 4000000) // ?
	MCFG_CPU_PROGRAM_MAP(coh1002ml_link_map)
	MCFG_CPU_IO_MAP(coh1002ml_link_port_map)
MACHINE_CONFIG_END


/*

Inputs

*/

static INPUT_PORTS_START( zn )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Freeze" )                PORT_DIPLOCATION("S551:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("S551:2") // bios testmode
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "S551:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "S551:4" ) // game testmode in some games
INPUT_PORTS_END

static INPUT_PORTS_START( zn4w )
	PORT_INCLUDE( zn )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)

	PORT_MODIFY("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(3)

	PORT_MODIFY("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(4)
INPUT_PORTS_END

static INPUT_PORTS_START( zn6b )
	PORT_INCLUDE( zn )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0xcc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P3")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 )

	PORT_MODIFY("P4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( jdredd )
	PORT_INCLUDE( zn )

	PORT_MODIFY("P1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x6f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x6f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, zn_state,jdredd_gun_mux_read, NULL)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P3")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P4")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("GUN1X")
	PORT_BIT( 0xffff, 0x0822, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x393-1,0xcb2+1) PORT_SENSITIVITY(100) PORT_KEYDELTA(50) PORT_PLAYER(1)

	PORT_START("GUN1Y")
	PORT_BIT( 0xffff, 0x0122, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x02d-1,0x217+1) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUN2X")
	PORT_BIT( 0xffff, 0x0822, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x393-1,0xcb2+1) PORT_SENSITIVITY(100) PORT_KEYDELTA(50) PORT_PLAYER(2)

	PORT_START("GUN2Y")
	PORT_BIT( 0xffff, 0x0122, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x02d-1,0x217+1) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( primrag2 )
	PORT_INCLUDE( zn )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0xcc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P3")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
INPUT_PORTS_END


/*

ROM Definitions

*/

/* Capcom ZN1 */

#define CPZN1_BIOS \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_SYSTEM_BIOS( 0, "bios0", "standard" ) \
	ROMX_LOAD( "coh-1000c.353", 0x0000000, 0x080000, CRC(50033af6) SHA1(486d92ff6c7f1e54f8e0ef41cd9116eca0e10e1a), ROM_BIOS(1)) \
	ROM_SYSTEM_BIOS( 1, "bios1", "development" ) \
	ROMX_LOAD( "coh-1000c-devel.bin", 0x000000, 0x080000, CRC(f20f7fe5) SHA1(9aac7d3b3d0cc0bbbe4056164b73078dce41d91c), ROM_BIOS(2) ) \
	ROM_REGION( 0x2000, "mcu", 0 ) \
	ROM_LOAD( "upd78081.655", 0x0000, 0x2000, NO_DUMP ) /* internal rom :( */

ROM_START( cpzn1 )
	CPZN1_BIOS
	ROM_REGION32_LE( 0x80000, "countryrom", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x2400000, "maskroms", ROMREGION_ERASE00 )
	ROM_REGION( 0x50000, "audiocpu", ROMREGION_ERASE00 )
	ROM_REGION( 0x400000, "qsound", ROMREGION_ERASE00 )
ROM_END

/* 95681-2 */
ROM_START( ts2 )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "ts2u_04.2h", 0x0000000, 0x080000, CRC(ddb52e7c) SHA1(e77891abae7681d911ef6eba2e0920d81433ebe6) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "ts2-05m.3h", 0x0000000, 0x400000, CRC(7f4228e2) SHA1(3690a76d19d97e55bc7b05a8456328697cfd7a77) )
	ROM_LOAD( "ts2-06m",    0x0400000, 0x400000, CRC(cd7e0a27) SHA1(325b5f2e653cdea07cddc9d20d12b5ab50dca949) )
	ROM_LOAD( "ts2-08m",    0x0800000, 0x400000, CRC(b1f7f115) SHA1(3f416d2aac07aa73a99593b5a21b047da60cea6a) )
	ROM_LOAD( "ts2-10m.4k", 0x0c00000, 0x200000, CRC(ad90679a) SHA1(19dd30764f892ee7f89c78ccbccdaf4d6b0e6e09) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ts2_02.2e",  0x00000, 0x08000, CRC(2f45c461) SHA1(513b6b9b5a2f7c567c30c958e0e13834cd9bd266) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "ts2-01m.3b", 0x0000000, 0x400000, CRC(d7a505e0) SHA1(f1b0cdea712101f695bd326eccd753eb79a07490) )
ROM_END

/* 95681-2 */
ROM_START( ts2a )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "ts2u_04.2h", 0x0000000, 0x080000, CRC(ddb52e7c) SHA1(e77891abae7681d911ef6eba2e0920d81433ebe6) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "ts2-05m.3h", 0x0000000, 0x400000, CRC(7f4228e2) SHA1(3690a76d19d97e55bc7b05a8456328697cfd7a77) )
	ROM_LOAD( "ts2-06m.4h", 0x0400000, 0x200000, CRC(67be6797) SHA1(521f69439ba7666f54d1008a291f3391f3a16499) )
	ROM_LOAD( "ts2-07m.5h", 0x0600000, 0x200000, CRC(db73e2b8) SHA1(239243f44c41df765789c14cc4036bb02e2ab373) )
	ROM_LOAD( "ts2-08m.2k", 0x0800000, 0x200000, CRC(01a48431) SHA1(6e395af726da91909e07dac25bb9b70b3ccebd4e) )
	ROM_LOAD( "ts2-09m.3k", 0x0a00000, 0x200000, CRC(83f408de) SHA1(415787c4dca604dd5611e16936a0ffa981dedf78) )
	ROM_LOAD( "ts2-10m.4k", 0x0c00000, 0x200000, CRC(ad90679a) SHA1(19dd30764f892ee7f89c78ccbccdaf4d6b0e6e09) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ts2_02.2e",  0x00000, 0x08000, CRC(2f45c461) SHA1(513b6b9b5a2f7c567c30c958e0e13834cd9bd266) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "ts2-01m.3b", 0x0000000, 0x400000, CRC(d7a505e0) SHA1(f1b0cdea712101f695bd326eccd753eb79a07490) )
ROM_END

/* 95681-2 */
ROM_START( ts2j )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "ts2j_04.2h", 0x0000000, 0x080000, CRC(4aba8c5e) SHA1(a56001bf50bfc1b03036e88ae1febd1aac8c63c0) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "ts2-05m.3h", 0x0000000, 0x400000, CRC(7f4228e2) SHA1(3690a76d19d97e55bc7b05a8456328697cfd7a77) )
	ROM_LOAD( "ts2-06m",    0x0400000, 0x400000, CRC(cd7e0a27) SHA1(325b5f2e653cdea07cddc9d20d12b5ab50dca949) )
	ROM_LOAD( "ts2-08m",    0x0800000, 0x400000, CRC(b1f7f115) SHA1(3f416d2aac07aa73a99593b5a21b047da60cea6a) )
	ROM_LOAD( "ts2-10m.4k", 0x0c00000, 0x200000, CRC(ad90679a) SHA1(19dd30764f892ee7f89c78ccbccdaf4d6b0e6e09) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ts2_02.2e",  0x00000, 0x08000, CRC(2f45c461) SHA1(513b6b9b5a2f7c567c30c958e0e13834cd9bd266) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "ts2-01m.3b", 0x0000000, 0x400000, CRC(d7a505e0) SHA1(f1b0cdea712101f695bd326eccd753eb79a07490) )
ROM_END

/* 95681-2 */
ROM_START( starglad )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "ps1u_04.2h", 0x0000000, 0x080000, CRC(121fb234) SHA1(697d18d37afd95f302b40a5a6a78d8c92a41ea73) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "ps1-05m.3h", 0x0000000, 0x400000, CRC(8ad72c4f) SHA1(c848c37eb5365000b4d4720b5c08d89ddd8e2c33) )
	ROM_LOAD( "ps1-06m.4h", 0x0400000, 0x400000, CRC(95d8ed61) SHA1(e9f259d589dc38a8321a6fea1f5dac741cadc0ff) )
	ROM_LOAD( "ps1-07m.5h", 0x0800000, 0x400000, CRC(c06752db) SHA1(0884b308e9cd9dde8660b422bc8fec9a362bcb52) )
	ROM_LOAD( "ps1-08m.2k", 0x0c00000, 0x400000, CRC(381f9ded) SHA1(b7878a90740f5b3c5881ac7d46e2b84b18727337) )
	ROM_LOAD( "ps1-09m.3k", 0x1000000, 0x400000, CRC(bd894812) SHA1(9f0c3365e685a53ae793f4a256a6c177a843a424) )
	ROM_LOAD( "ps1-10m.4k", 0x1400000, 0x400000, CRC(ff80c18a) SHA1(8d01717eed6ec1f508fe7c445da941fb84ef7d22) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ps1_02a.2e", 0x00000, 0x08000, CRC(b854df92) SHA1(ea71a613b5b19ec7e9c6e342e7743d320582a6bb) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "ps1_03a.3e", 0x28000, 0x20000, CRC(a2562fbb) SHA1(3de02a4aa7ea620961ca2a5c331f38134033db79) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "ps1-01m.3b", 0x0000000, 0x400000, CRC(0bfb17aa) SHA1(cf4482785a2a33ad814c8b1461c5bc8e8e027895) )
ROM_END

/* 95681-2 */
ROM_START( stargladj )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "ps1j_04.2h", 0x0000000, 0x080000, CRC(f865006d) SHA1(6abb476777a4309b1a60f12e6313c051db195808) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "ps1-05m.3h", 0x0000000, 0x400000, CRC(8ad72c4f) SHA1(c848c37eb5365000b4d4720b5c08d89ddd8e2c33) )
	ROM_LOAD( "ps1-06m.4h", 0x0400000, 0x400000, CRC(95d8ed61) SHA1(e9f259d589dc38a8321a6fea1f5dac741cadc0ff) )
	ROM_LOAD( "ps1-07m.5h", 0x0800000, 0x400000, CRC(c06752db) SHA1(0884b308e9cd9dde8660b422bc8fec9a362bcb52) )
	ROM_LOAD( "ps1-08m.2k", 0x0c00000, 0x400000, CRC(381f9ded) SHA1(b7878a90740f5b3c5881ac7d46e2b84b18727337) )
	ROM_LOAD( "ps1-09m.3k", 0x1000000, 0x400000, CRC(bd894812) SHA1(9f0c3365e685a53ae793f4a256a6c177a843a424) )
	ROM_LOAD( "ps1-10m.4k", 0x1400000, 0x400000, CRC(ff80c18a) SHA1(8d01717eed6ec1f508fe7c445da941fb84ef7d22) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ps1_02a.2e", 0x00000, 0x08000, CRC(b854df92) SHA1(ea71a613b5b19ec7e9c6e342e7743d320582a6bb) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "ps1_03a.3e", 0x28000, 0x20000, CRC(a2562fbb) SHA1(3de02a4aa7ea620961ca2a5c331f38134033db79) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "ps1-01m.3b", 0x0000000, 0x400000, CRC(0bfb17aa) SHA1(cf4482785a2a33ad814c8b1461c5bc8e8e027895) )
ROM_END

/* 95681-2 */
/*
Labeled with an offical TECMO project label as listed below. Indented info is hand written:

PROJECT
  GRA
ROM No.
  EP-ENG
DATE
  96 / 10/ 17
TIME
  16:30

As well as a hand written sum16 checksum of D04B (which matches dump)
*/
ROM_START( glpracr )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "gra_ep-eng.2h", 0x0000000, 0x080000, CRC(f3ab9c85) SHA1(ce9d5d0406a6854975d5c71935fe917706334429) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "gra-05m.3h", 0x0000000, 0x400000, CRC(78053700) SHA1(38727c8cc34bb57b7b7e73041e382fb0361f184e) )
	ROM_LOAD( "gra-06m.4h", 0x0400000, 0x400000, CRC(d73b392b) SHA1(241ddf474cea035e81a2abc580d3c0395ee925bb) )
	ROM_LOAD( "gra-07m.5h", 0x0800000, 0x400000, CRC(acaefe3a) SHA1(32d596b0f975e1558fa7929c3166d8dad40a1c80) )

	/* Sockets 2.2E, 3.3E are not populated, pcb verified */
	ROM_REGION( 0x50000, "audiocpu", ROMREGION_ERASE00 ) /* 64k for the audio CPU (+banks) */

	/* Socket 1.3B is not populated, pcb verified */
	ROM_REGION( 0x400000, "qsound", ROMREGION_ERASE00 ) /* Q Sound Samples */
ROM_END

/* 95681-2 */
ROM_START( glpracrj )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "gpaj_04.2h", 0x0000000, 0x080000, CRC(53bf551c) SHA1(320632b5010630cee4c5ccb1578d5ee6d2754632) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "gra-05m.3h", 0x0000000, 0x400000, CRC(78053700) SHA1(38727c8cc34bb57b7b7e73041e382fb0361f184e) )
	ROM_LOAD( "gra-06m.4h", 0x0400000, 0x400000, CRC(d73b392b) SHA1(241ddf474cea035e81a2abc580d3c0395ee925bb) )
	ROM_LOAD( "gra-07m.5h", 0x0800000, 0x400000, CRC(acaefe3a) SHA1(32d596b0f975e1558fa7929c3166d8dad40a1c80) )

	/* Sockets 2.2E, 3.3E are not populated, pcb verified */
	ROM_REGION( 0x50000, "audiocpu", ROMREGION_ERASE00 ) /* 64k for the audio CPU (+banks) */

	/* Socket 1.3B is not populated, pcb verified */
	ROM_REGION( 0x400000, "qsound", ROMREGION_ERASE00 ) /* Q Sound Samples */
ROM_END

/* 95681-2 */
ROM_START( sfex )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "sfee_04a.2h", 0x0000000, 0x080000, CRC(092cfa2e) SHA1(8af38a3f4f89f661233995a672faf486e71b79bc) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "sfe-05m.3h", 0x0000000, 0x400000, CRC(eab781fe) SHA1(205476cb72c8dac915e140fb32243dfc5d209ba4) )
	ROM_LOAD( "sfe-06m.4h", 0x0400000, 0x400000, CRC(999de60c) SHA1(092882698c411fc5c3bcb43105bf1886f94b8e40) )
	ROM_LOAD( "sfe-07m.5h", 0x0800000, 0x400000, CRC(76117b0a) SHA1(027233199170fa6e5b32f28da2031638c6d3d14a) )
	ROM_LOAD( "sfe-08m.2k", 0x0c00000, 0x400000, CRC(a36bbec5) SHA1(fa22ea50d4d8bed2ded97a346f61b2f5f68769b9) )
	ROM_LOAD( "sfe-09m.3k", 0x1000000, 0x400000, CRC(62c424cc) SHA1(ea19c49b486473b150dbf8541286e225655496db) )
	ROM_LOAD( "sfe-10m.4k", 0x1400000, 0x400000, CRC(83791a8b) SHA1(534969797640834ca692c11d0ce7c3a060fc7e4b) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfe_02.2e",  0x00000, 0x08000, CRC(1908475c) SHA1(99f68cff2d92f5697eec0846201f6fb317d5dc08) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "sfe_03.3e",  0x28000, 0x20000, CRC(95c1e2e0) SHA1(383bbe9613798a3ac6944d18768280a840994e40) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "sfe-01m.3b", 0x0000000, 0x400000, CRC(f5afff0d) SHA1(7f9ac32ba0a3d9c6fef367e36a92d47c9ac1feb3) )
ROM_END

/* 95681-2 */
ROM_START( sfexu )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "sfeu_04b.2h", 0x0000000, 0x080000, CRC(de02bd29) SHA1(62a88a30f73db661f5b98fc7e2d34d51acb965cc) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "sfe-05m.3h", 0x0000000, 0x400000, CRC(eab781fe) SHA1(205476cb72c8dac915e140fb32243dfc5d209ba4) )
	ROM_LOAD( "sfe-06m.4h", 0x0400000, 0x400000, CRC(999de60c) SHA1(092882698c411fc5c3bcb43105bf1886f94b8e40) )
	ROM_LOAD( "sfe-07m.5h", 0x0800000, 0x400000, CRC(76117b0a) SHA1(027233199170fa6e5b32f28da2031638c6d3d14a) )
	ROM_LOAD( "sfe-08m.2k", 0x0c00000, 0x400000, CRC(a36bbec5) SHA1(fa22ea50d4d8bed2ded97a346f61b2f5f68769b9) )
	ROM_LOAD( "sfe-09m.3k", 0x1000000, 0x400000, CRC(62c424cc) SHA1(ea19c49b486473b150dbf8541286e225655496db) )
	ROM_LOAD( "sfe-10m.4k", 0x1400000, 0x400000, CRC(83791a8b) SHA1(534969797640834ca692c11d0ce7c3a060fc7e4b) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfe_02.2e",  0x00000, 0x08000, CRC(1908475c) SHA1(99f68cff2d92f5697eec0846201f6fb317d5dc08) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "sfe_03.3e",  0x28000, 0x20000, CRC(95c1e2e0) SHA1(383bbe9613798a3ac6944d18768280a840994e40) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "sfe-01m.3b", 0x0000000, 0x400000, CRC(f5afff0d) SHA1(7f9ac32ba0a3d9c6fef367e36a92d47c9ac1feb3) )
ROM_END

/* 95681-2 */
ROM_START( sfexa )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "sfea_04a.2h", 0x0000000, 0x080000, CRC(08247bd4) SHA1(07f356ef2827b3fbd0bfaf2010915315d9d60ef1) )    // could be sfea_04.2h

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "sfe-05m.3h", 0x0000000, 0x400000, CRC(eab781fe) SHA1(205476cb72c8dac915e140fb32243dfc5d209ba4) )
	ROM_LOAD( "sfe-06m.4h", 0x0400000, 0x400000, CRC(999de60c) SHA1(092882698c411fc5c3bcb43105bf1886f94b8e40) )
	ROM_LOAD( "sfe-07m.5h", 0x0800000, 0x400000, CRC(76117b0a) SHA1(027233199170fa6e5b32f28da2031638c6d3d14a) )
	ROM_LOAD( "sfe-08m.2k", 0x0c00000, 0x400000, CRC(a36bbec5) SHA1(fa22ea50d4d8bed2ded97a346f61b2f5f68769b9) )
	ROM_LOAD( "sfe-09m.3k", 0x1000000, 0x400000, CRC(62c424cc) SHA1(ea19c49b486473b150dbf8541286e225655496db) )
	ROM_LOAD( "sfe-10m.4k", 0x1400000, 0x400000, CRC(83791a8b) SHA1(534969797640834ca692c11d0ce7c3a060fc7e4b) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfe_02.2e",  0x00000, 0x08000, CRC(1908475c) SHA1(99f68cff2d92f5697eec0846201f6fb317d5dc08) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "sfe_03.3e",  0x28000, 0x20000, CRC(95c1e2e0) SHA1(383bbe9613798a3ac6944d18768280a840994e40) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "sfe-01m.3b", 0x0000000, 0x400000, CRC(f5afff0d) SHA1(7f9ac32ba0a3d9c6fef367e36a92d47c9ac1feb3) )
ROM_END

/* 95681-2 */
ROM_START( sfexj )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "sfej_04.2h", 0x0000000, 0x080000, CRC(ea100607) SHA1(27ef8c619804999d32d14fcc5ec783c057b4dc73) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "sfe-05m.3h", 0x0000000, 0x400000, CRC(eab781fe) SHA1(205476cb72c8dac915e140fb32243dfc5d209ba4) )
	ROM_LOAD( "sfe-06m.4h", 0x0400000, 0x400000, CRC(999de60c) SHA1(092882698c411fc5c3bcb43105bf1886f94b8e40) )
	ROM_LOAD( "sfe-07m.5h", 0x0800000, 0x400000, CRC(76117b0a) SHA1(027233199170fa6e5b32f28da2031638c6d3d14a) )
	ROM_LOAD( "sfe-08m.2k", 0x0c00000, 0x400000, CRC(a36bbec5) SHA1(fa22ea50d4d8bed2ded97a346f61b2f5f68769b9) )
	ROM_LOAD( "sfe-09m.3k", 0x1000000, 0x400000, CRC(62c424cc) SHA1(ea19c49b486473b150dbf8541286e225655496db) )
	ROM_LOAD( "sfe-10m.4k", 0x1400000, 0x400000, CRC(83791a8b) SHA1(534969797640834ca692c11d0ce7c3a060fc7e4b) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfe_02.2e",  0x00000, 0x08000, CRC(1908475c) SHA1(99f68cff2d92f5697eec0846201f6fb317d5dc08) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "sfe_03.3e",  0x28000, 0x20000, CRC(95c1e2e0) SHA1(383bbe9613798a3ac6944d18768280a840994e40) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "sfe-01m.3b", 0x0000000, 0x400000, CRC(f5afff0d) SHA1(7f9ac32ba0a3d9c6fef367e36a92d47c9ac1feb3) )
ROM_END

/* 95681-2 */
ROM_START( sfexp )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "sfpu_04a.2h", 0x0000000, 0x080000, CRC(4617adc2) SHA1(200307904349ad7e5d7d76d8c904b6b10424c7ef) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "sfp-05m.3h", 0x0000000, 0x400000, CRC(ac7dcc5e) SHA1(216de2de691a9bd7982d5d6b5b1e3e35ff381a2f) )
	ROM_LOAD( "sfp-06m.4h", 0x0400000, 0x400000, CRC(1d504758) SHA1(bd56141aba35dbb5b318445ba5db12eff7442221) )
	ROM_LOAD( "sfp-07m.5h", 0x0800000, 0x400000, CRC(0f585f30) SHA1(24ffdbc360f8eddb702905c99d315614327861a7) )
	ROM_LOAD( "sfp-08m.2k", 0x0c00000, 0x400000, CRC(65eabc61) SHA1(bbeb3bcd8dd8f7f88ed82412a81134a3d6f6ffd9) )
	ROM_LOAD( "sfp-09m.3k", 0x1000000, 0x400000, CRC(15f8b71e) SHA1(efb28fbe750f443550ee9718385355aae7e858c9) )
	ROM_LOAD( "sfp-10m.4k", 0x1400000, 0x400000, CRC(c1ecf652) SHA1(616e14ff63d38272730c810b933a6b3412e2da17) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfe_02.2e",  0x00000, 0x08000, CRC(1908475c) SHA1(99f68cff2d92f5697eec0846201f6fb317d5dc08) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "sfe_03.3e",  0x28000, 0x20000, CRC(95c1e2e0) SHA1(383bbe9613798a3ac6944d18768280a840994e40) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "sfe-01m.3b", 0x0000000, 0x400000, CRC(f5afff0d) SHA1(7f9ac32ba0a3d9c6fef367e36a92d47c9ac1feb3) )
ROM_END

/* 95681-2 */
ROM_START( sfexpu1 )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "sfpu_04.2h", 0x0000000, 0x080000, CRC(305e4ec0) SHA1(0df9572d7fc1bbc7131483960771d016fa5487a5) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "sfp-05m.3h", 0x0000000, 0x400000, CRC(ac7dcc5e) SHA1(216de2de691a9bd7982d5d6b5b1e3e35ff381a2f) )
	ROM_LOAD( "sfp-06m.4h", 0x0400000, 0x400000, CRC(1d504758) SHA1(bd56141aba35dbb5b318445ba5db12eff7442221) )
	ROM_LOAD( "sfp-07m.5h", 0x0800000, 0x400000, CRC(0f585f30) SHA1(24ffdbc360f8eddb702905c99d315614327861a7) )
	ROM_LOAD( "sfp-08m.2k", 0x0c00000, 0x400000, CRC(65eabc61) SHA1(bbeb3bcd8dd8f7f88ed82412a81134a3d6f6ffd9) )
	ROM_LOAD( "sfp-09m.3k", 0x1000000, 0x400000, CRC(15f8b71e) SHA1(efb28fbe750f443550ee9718385355aae7e858c9) )
	ROM_LOAD( "sfp-10m.4k", 0x1400000, 0x400000, CRC(c1ecf652) SHA1(616e14ff63d38272730c810b933a6b3412e2da17) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfe_02.2e",  0x00000, 0x08000, CRC(1908475c) SHA1(99f68cff2d92f5697eec0846201f6fb317d5dc08) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "sfe_03.3e",  0x28000, 0x20000, CRC(95c1e2e0) SHA1(383bbe9613798a3ac6944d18768280a840994e40) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "sfe-01m.3b", 0x0000000, 0x400000, CRC(f5afff0d) SHA1(7f9ac32ba0a3d9c6fef367e36a92d47c9ac1feb3) )
ROM_END

/* 95681-2 */
ROM_START( sfexpj )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "sfpj_04a.2h", 0x0000000, 0x080000, CRC(6e99a0f7) SHA1(8f22bc545dd0e3eff24ab62ce5af1998d48d3770) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "sfp-05m.3h", 0x0000000, 0x400000, CRC(ac7dcc5e) SHA1(216de2de691a9bd7982d5d6b5b1e3e35ff381a2f) )
	ROM_LOAD( "sfp-06m.4h", 0x0400000, 0x400000, CRC(1d504758) SHA1(bd56141aba35dbb5b318445ba5db12eff7442221) )
	ROM_LOAD( "sfp-07m.5h", 0x0800000, 0x400000, CRC(0f585f30) SHA1(24ffdbc360f8eddb702905c99d315614327861a7) )
	ROM_LOAD( "sfp-08m.2k", 0x0c00000, 0x400000, CRC(65eabc61) SHA1(bbeb3bcd8dd8f7f88ed82412a81134a3d6f6ffd9) )
	ROM_LOAD( "sfp-09m.3k", 0x1000000, 0x400000, CRC(15f8b71e) SHA1(efb28fbe750f443550ee9718385355aae7e858c9) )
	ROM_LOAD( "sfp-10m.4k", 0x1400000, 0x400000, CRC(c1ecf652) SHA1(616e14ff63d38272730c810b933a6b3412e2da17) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfe_02.2e",  0x00000, 0x08000, CRC(1908475c) SHA1(99f68cff2d92f5697eec0846201f6fb317d5dc08) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "sfe_03.3e",  0x28000, 0x20000, CRC(95c1e2e0) SHA1(383bbe9613798a3ac6944d18768280a840994e40) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "sfe-01m.3b", 0x0000000, 0x400000, CRC(f5afff0d) SHA1(7f9ac32ba0a3d9c6fef367e36a92d47c9ac1feb3) )
ROM_END

/* 95681-2 */
ROM_START( sfexpj1 )
	CPZN1_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "sfpj_04.2h", 0x0000000, 0x080000, CRC(18d043f5) SHA1(9e6e24a722d13888fbfd391ddb1a5045b162488c) )

	ROM_REGION32_LE( 0x2400000, "maskroms", 0 )
	ROM_LOAD( "sfp-05m.3h", 0x0000000, 0x400000, CRC(ac7dcc5e) SHA1(216de2de691a9bd7982d5d6b5b1e3e35ff381a2f) )
	ROM_LOAD( "sfp-06m.4h", 0x0400000, 0x400000, CRC(1d504758) SHA1(bd56141aba35dbb5b318445ba5db12eff7442221) )
	ROM_LOAD( "sfp-07m.5h", 0x0800000, 0x400000, CRC(0f585f30) SHA1(24ffdbc360f8eddb702905c99d315614327861a7) )
	ROM_LOAD( "sfp-08m.2k", 0x0c00000, 0x400000, CRC(65eabc61) SHA1(bbeb3bcd8dd8f7f88ed82412a81134a3d6f6ffd9) )
	ROM_LOAD( "sfp-09m.3k", 0x1000000, 0x400000, CRC(15f8b71e) SHA1(efb28fbe750f443550ee9718385355aae7e858c9) )
	ROM_LOAD( "sfp-10m.4k", 0x1400000, 0x400000, CRC(c1ecf652) SHA1(616e14ff63d38272730c810b933a6b3412e2da17) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfe_02.2e",  0x00000, 0x08000, CRC(1908475c) SHA1(99f68cff2d92f5697eec0846201f6fb317d5dc08) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "sfe_03.3e",  0x28000, 0x20000, CRC(95c1e2e0) SHA1(383bbe9613798a3ac6944d18768280a840994e40) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "sfe-01m.3b", 0x0000000, 0x400000, CRC(f5afff0d) SHA1(7f9ac32ba0a3d9c6fef367e36a92d47c9ac1feb3) )
ROM_END

/* Capcom ZN2 */

#define CPZN2_BIOS \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_LOAD( "coh-3002c.353", 0x0000000, 0x080000, CRC(e860ea8b) SHA1(66e7e1d4e426466b8f48a2ba055a91b475569504) ) \
	ROM_REGION( 0x2000, "mcu", 0 ) \
	ROM_LOAD( "upd78081.655", 0x0000, 0x2000, NO_DUMP ) /* internal rom :( */

ROM_START( cpzn2 )
	CPZN2_BIOS
	ROM_REGION32_LE( 0x80000, "countryrom", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x3000000, "maskroms", ROMREGION_ERASE00 )
	ROM_REGION( 0x50000, "audiocpu", ROMREGION_ERASE00 )
	ROM_REGION( 0x400000, "qsound", ROMREGION_ERASE00 )
ROM_END

/* 95681-2 */
ROM_START( rvschool )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "jste_04.2h", 0x0000000, 0x080000, CRC(1567555a) SHA1(3b93235076ab3c06914c83becf0da8e810b8917a) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "jst-05m.3h", 0x0000000, 0x400000, CRC(723372b8) SHA1(2a7c95d1f9a3f58c469dfc28ead1fd192eaaebd1) )
	ROM_LOAD( "jst-06m.4h", 0x0400000, 0x400000, CRC(4248988e) SHA1(4bdf7cac17d70ea85aa2002fc6b21a64d05e6e5a) )
	ROM_LOAD( "jst-07m.5h", 0x0800000, 0x400000, CRC(c84c5a16) SHA1(5c0ca7454189c766f1ca7305504ff1867007c8e6) )
	ROM_LOAD( "jst-08m.2k", 0x0c00000, 0x400000, CRC(791b57f3) SHA1(4ea12a0f7a7110d7dcbc55b3f02aa9a92dea4b12) )
	ROM_LOAD( "jst-09m.3k", 0x1000000, 0x400000, CRC(6df42048) SHA1(9e2b4a424de3918e5e54bc87fd9dcceff8d162be) )
	ROM_LOAD( "jst-10m.4k", 0x1400000, 0x400000, CRC(d7e22769) SHA1(733f96dce2586fc0a8af3cec18153085750c9a4d) )
	ROM_LOAD( "jst-11m.5k", 0x1800000, 0x400000, CRC(0a033ac5) SHA1(218b33cb51db99d3e9ee180da6a74460f4444fc6) )
	ROM_LOAD( "jst-12m.6k", 0x1c00000, 0x400000, CRC(43bd2ddd) SHA1(7f2976e394362cb648f620e430b3bf11b71485a6) )
	ROM_LOAD( "jst-13m.7k", 0x2000000, 0x400000, CRC(6b443235) SHA1(c764d8b742aa1c46bc8d37f36e864ef50a1ff4e4) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "jst_02.2e",  0x00000, 0x08000, CRC(7809e2c3) SHA1(0216a665f7978bc8db3f7fdab038e1c7aa120844) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "jst_03.3e",  0x28000, 0x20000, CRC(860ff24d) SHA1(eea72fa5eaf407a112a5b3daf60f7ac8ad191cc7) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "jst-01m.3b", 0x0000000, 0x400000, CRC(9a7c98f9) SHA1(764c6c4f41047e1f36d2dceac4aa9b943a9d529a) )
ROM_END

/* 95681-2 */
ROM_START( rvschoolu )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "jstu_04.2h", 0x0000000, 0x080000, CRC(d83724ae) SHA1(0890c0164116606acc600f646e82972d0d4f79b4) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "jst-05m.3h", 0x0000000, 0x400000, CRC(723372b8) SHA1(2a7c95d1f9a3f58c469dfc28ead1fd192eaaebd1) )
	ROM_LOAD( "jst-06m.4h", 0x0400000, 0x400000, CRC(4248988e) SHA1(4bdf7cac17d70ea85aa2002fc6b21a64d05e6e5a) )
	ROM_LOAD( "jst-07m.5h", 0x0800000, 0x400000, CRC(c84c5a16) SHA1(5c0ca7454189c766f1ca7305504ff1867007c8e6) )
	ROM_LOAD( "jst-08m.2k", 0x0c00000, 0x400000, CRC(791b57f3) SHA1(4ea12a0f7a7110d7dcbc55b3f02aa9a92dea4b12) )
	ROM_LOAD( "jst-09m.3k", 0x1000000, 0x400000, CRC(6df42048) SHA1(9e2b4a424de3918e5e54bc87fd9dcceff8d162be) )
	ROM_LOAD( "jst-10m.4k", 0x1400000, 0x400000, CRC(d7e22769) SHA1(733f96dce2586fc0a8af3cec18153085750c9a4d) )
	ROM_LOAD( "jst-11m.5k", 0x1800000, 0x400000, CRC(0a033ac5) SHA1(218b33cb51db99d3e9ee180da6a74460f4444fc6) )
	ROM_LOAD( "jst-12m.6k", 0x1c00000, 0x400000, CRC(43bd2ddd) SHA1(7f2976e394362cb648f620e430b3bf11b71485a6) )
	ROM_LOAD( "jst-13m.7k", 0x2000000, 0x400000, CRC(6b443235) SHA1(c764d8b742aa1c46bc8d37f36e864ef50a1ff4e4) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "jst_02.2e",  0x00000, 0x08000, CRC(7809e2c3) SHA1(0216a665f7978bc8db3f7fdab038e1c7aa120844) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "jst_03.3e",  0x28000, 0x20000, CRC(860ff24d) SHA1(eea72fa5eaf407a112a5b3daf60f7ac8ad191cc7) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "jst-01m.3b", 0x0000000, 0x400000, CRC(9a7c98f9) SHA1(764c6c4f41047e1f36d2dceac4aa9b943a9d529a) )
ROM_END

/* 95681-2 */
ROM_START( rvschoola )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "jsta_04.2h", 0x0000000, 0x080000, CRC(034b1011) SHA1(6773246be242ee336503d21d7d44a3884832eb1e) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "jst-05m.3h", 0x0000000, 0x400000, CRC(723372b8) SHA1(2a7c95d1f9a3f58c469dfc28ead1fd192eaaebd1) )
	ROM_LOAD( "jst-06m.4h", 0x0400000, 0x400000, CRC(4248988e) SHA1(4bdf7cac17d70ea85aa2002fc6b21a64d05e6e5a) )
	ROM_LOAD( "jst-07m.5h", 0x0800000, 0x400000, CRC(c84c5a16) SHA1(5c0ca7454189c766f1ca7305504ff1867007c8e6) )
	ROM_LOAD( "jst-08m.2k", 0x0c00000, 0x400000, CRC(791b57f3) SHA1(4ea12a0f7a7110d7dcbc55b3f02aa9a92dea4b12) )
	ROM_LOAD( "jst-09m.3k", 0x1000000, 0x400000, CRC(6df42048) SHA1(9e2b4a424de3918e5e54bc87fd9dcceff8d162be) )
	ROM_LOAD( "jst-10m.4k", 0x1400000, 0x400000, CRC(d7e22769) SHA1(733f96dce2586fc0a8af3cec18153085750c9a4d) )
	ROM_LOAD( "jst-11m.5k", 0x1800000, 0x400000, CRC(0a033ac5) SHA1(218b33cb51db99d3e9ee180da6a74460f4444fc6) )
	ROM_LOAD( "jst-12m.6k", 0x1c00000, 0x400000, CRC(43bd2ddd) SHA1(7f2976e394362cb648f620e430b3bf11b71485a6) )
	ROM_LOAD( "jst-13m.7k", 0x2000000, 0x400000, CRC(6b443235) SHA1(c764d8b742aa1c46bc8d37f36e864ef50a1ff4e4) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "jst_02.2e",  0x00000, 0x08000, CRC(7809e2c3) SHA1(0216a665f7978bc8db3f7fdab038e1c7aa120844) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "jst_03.3e",  0x28000, 0x20000, CRC(860ff24d) SHA1(eea72fa5eaf407a112a5b3daf60f7ac8ad191cc7) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "jst-01m.3b", 0x0000000, 0x400000, CRC(9a7c98f9) SHA1(764c6c4f41047e1f36d2dceac4aa9b943a9d529a) )
ROM_END

/* 95681-2 */
ROM_START( jgakuen )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "jstj_04.2h", 0x0000000, 0x080000, CRC(28b8000a) SHA1(9ebf74b453d775cadca9c2d7d8e2c7eb57bb9a38) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "jst-05m.3h", 0x0000000, 0x400000, CRC(723372b8) SHA1(2a7c95d1f9a3f58c469dfc28ead1fd192eaaebd1) )
	ROM_LOAD( "jst-06m.4h", 0x0400000, 0x400000, CRC(4248988e) SHA1(4bdf7cac17d70ea85aa2002fc6b21a64d05e6e5a) )
	ROM_LOAD( "jst-07m.5h", 0x0800000, 0x400000, CRC(c84c5a16) SHA1(5c0ca7454189c766f1ca7305504ff1867007c8e6) )
	ROM_LOAD( "jst-08m.2k", 0x0c00000, 0x400000, CRC(791b57f3) SHA1(4ea12a0f7a7110d7dcbc55b3f02aa9a92dea4b12) )
	ROM_LOAD( "jst-09m.3k", 0x1000000, 0x400000, CRC(6df42048) SHA1(9e2b4a424de3918e5e54bc87fd9dcceff8d162be) )
	ROM_LOAD( "jst-10m.4k", 0x1400000, 0x400000, CRC(d7e22769) SHA1(733f96dce2586fc0a8af3cec18153085750c9a4d) )
	ROM_LOAD( "jst-11m.5k", 0x1800000, 0x400000, CRC(0a033ac5) SHA1(218b33cb51db99d3e9ee180da6a74460f4444fc6) )
	ROM_LOAD( "jst-12m.6k", 0x1c00000, 0x400000, CRC(43bd2ddd) SHA1(7f2976e394362cb648f620e430b3bf11b71485a6) )
	ROM_LOAD( "jst-13m.7k", 0x2000000, 0x400000, CRC(6b443235) SHA1(c764d8b742aa1c46bc8d37f36e864ef50a1ff4e4) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "jst_02.2e",  0x00000, 0x08000, CRC(7809e2c3) SHA1(0216a665f7978bc8db3f7fdab038e1c7aa120844) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "jst_03.3e",  0x28000, 0x20000, CRC(860ff24d) SHA1(eea72fa5eaf407a112a5b3daf60f7ac8ad191cc7) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "jst-01m.3b", 0x0000000, 0x400000, CRC(9a7c98f9) SHA1(764c6c4f41047e1f36d2dceac4aa9b943a9d529a) )
ROM_END

/* Hereafter Capcom games will use only the new game board 97695-1. */

/* 97695-1 */
/* An undumped japanese pcb of Shiritsu Justice Gakuen: Legion of Heroes running on the new game board 97695-1 and
   mounting the EPROM JSTJ_04A located @ 2H is proved to exist. */

/* 97695-1 */
ROM_START( sfex2 )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "ex2u_04a.2h", 0x0000000, 0x080000, CRC(8dc5317f) SHA1(c35224caf70662a0e45a74cbead294a51f9b9e16) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "ex2-05m.3h", 0x0000000, 0x800000, CRC(78726b17) SHA1(2da449df335ef133ebc3997bbad73ef4137f4771) )
	ROM_LOAD( "ex2-06m.4h", 0x0800000, 0x800000, CRC(be1075ed) SHA1(36dc673372f30f8b3ff5689ae568c5cd01fe2c07) )
	ROM_LOAD( "ex2-07m.5h", 0x1000000, 0x800000, CRC(6496c6ed) SHA1(054bcecbb04033abea14d9ffe6634b2bd11ca88b) )
	ROM_LOAD( "ex2-08m.2k", 0x1800000, 0x800000, CRC(3194132e) SHA1(d1324fcf0a8528fc683791d6342697a7e08674f4) )
	ROM_LOAD( "ex2-09m.3k", 0x2000000, 0x400000, CRC(075ae585) SHA1(6b88851db618fc3e96f1d740c46c1bc5be0ee21b) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ex2_02.2e",  0x00000, 0x08000, CRC(9489875e) SHA1(1fc9985ff98232c63ea8d05a69f7d77cdf72919f) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "ex2-01m.3a", 0x0000000, 0x400000, CRC(14a5bb0e) SHA1(dfe3c3a53bd4c58743d8039b5344d3afbe2a9c24) )
ROM_END

/* 97695-1 */
ROM_START( sfex2a )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "ex2a_04.2h", 0x0000000, 0x080000, CRC(ac9a872d) SHA1(4e237f5e2e5de58e587e9abc5767509b8d750004) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "ex2-05m.3h", 0x0000000, 0x800000, CRC(78726b17) SHA1(2da449df335ef133ebc3997bbad73ef4137f4771) )
	ROM_LOAD( "ex2-06m.4h", 0x0800000, 0x800000, CRC(be1075ed) SHA1(36dc673372f30f8b3ff5689ae568c5cd01fe2c07) )
	ROM_LOAD( "ex2-07m.5h", 0x1000000, 0x800000, CRC(6496c6ed) SHA1(054bcecbb04033abea14d9ffe6634b2bd11ca88b) )
	ROM_LOAD( "ex2-08m.2k", 0x1800000, 0x800000, CRC(3194132e) SHA1(d1324fcf0a8528fc683791d6342697a7e08674f4) )
	ROM_LOAD( "ex2-09m.3k", 0x2000000, 0x400000, CRC(075ae585) SHA1(6b88851db618fc3e96f1d740c46c1bc5be0ee21b) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ex2_02.2e",  0x00000, 0x08000, CRC(9489875e) SHA1(1fc9985ff98232c63ea8d05a69f7d77cdf72919f) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "ex2-01m.3a", 0x0000000, 0x400000, CRC(14a5bb0e) SHA1(dfe3c3a53bd4c58743d8039b5344d3afbe2a9c24) )
ROM_END

/* 97695-1 */
ROM_START( sfex2h )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "ex2h_04.2h", 0x0000000, 0x080000, CRC(68f2ef80) SHA1(212bb3c0d935c64f5e3b20e427e06d97404709d8) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "ex2-05m.3h", 0x0000000, 0x800000, CRC(78726b17) SHA1(2da449df335ef133ebc3997bbad73ef4137f4771) )
	ROM_LOAD( "ex2-06m.4h", 0x0800000, 0x800000, CRC(be1075ed) SHA1(36dc673372f30f8b3ff5689ae568c5cd01fe2c07) )
	ROM_LOAD( "ex2-07m.5h", 0x1000000, 0x800000, CRC(6496c6ed) SHA1(054bcecbb04033abea14d9ffe6634b2bd11ca88b) )
	ROM_LOAD( "ex2-08m.2k", 0x1800000, 0x800000, CRC(3194132e) SHA1(d1324fcf0a8528fc683791d6342697a7e08674f4) )
	ROM_LOAD( "ex2-09m.3k", 0x2000000, 0x400000, CRC(075ae585) SHA1(6b88851db618fc3e96f1d740c46c1bc5be0ee21b) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ex2_02.2e",  0x00000, 0x08000, CRC(9489875e) SHA1(1fc9985ff98232c63ea8d05a69f7d77cdf72919f) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "ex2-01m.3a", 0x0000000, 0x400000, CRC(14a5bb0e) SHA1(dfe3c3a53bd4c58743d8039b5344d3afbe2a9c24) )
ROM_END

/* 97695-1 */
ROM_START( sfex2j )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "ex2j_04.2h", 0x0000000, 0x080000, CRC(5d603586) SHA1(ff546d3bd011d6441e9672b88bab763d3cd89be2) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "ex2-05m.3h", 0x0000000, 0x800000, CRC(78726b17) SHA1(2da449df335ef133ebc3997bbad73ef4137f4771) )
	ROM_LOAD( "ex2-06m.4h", 0x0800000, 0x800000, CRC(be1075ed) SHA1(36dc673372f30f8b3ff5689ae568c5cd01fe2c07) )
	ROM_LOAD( "ex2-07m.5h", 0x1000000, 0x800000, CRC(6496c6ed) SHA1(054bcecbb04033abea14d9ffe6634b2bd11ca88b) )
	ROM_LOAD( "ex2-08m.2k", 0x1800000, 0x800000, CRC(3194132e) SHA1(d1324fcf0a8528fc683791d6342697a7e08674f4) )
	ROM_LOAD( "ex2-09m.3k", 0x2000000, 0x400000, CRC(075ae585) SHA1(6b88851db618fc3e96f1d740c46c1bc5be0ee21b) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ex2_02.2e",  0x00000, 0x08000, CRC(9489875e) SHA1(1fc9985ff98232c63ea8d05a69f7d77cdf72919f) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "ex2-01m.3a", 0x0000000, 0x400000, CRC(14a5bb0e) SHA1(dfe3c3a53bd4c58743d8039b5344d3afbe2a9c24) )
ROM_END

/* 97695-1 */
ROM_START( plsmaswd )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "sg2u_04.2h", 0x0000000, 0x080000, CRC(154187c0) SHA1(58cc0e9d32786b1c1d64ecee4667190456b36ef6) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "sg2-05m.3h", 0x0000000, 0x800000, CRC(f1759236) SHA1(fbe3a820a8c571dfb186eae68346e6461168ed48) )
	ROM_LOAD( "sg2-06m.4h", 0x0800000, 0x800000, CRC(33de4f72) SHA1(ab32af76b5682e3d9f67dadbaed35abc043912b4) )
	ROM_LOAD( "sg2-07m.5h", 0x1000000, 0x800000, CRC(72f724ba) SHA1(e6658b495d308d1de6710f87b5b9d346008b0c5a) )
	ROM_LOAD( "sg2-08m.2k", 0x1800000, 0x800000, CRC(9e169eee) SHA1(6141b1a7863fdfb200ca35d2893979a34dcc3f6c) )
	ROM_LOAD( "sg2-09m.3k", 0x2000000, 0x400000, CRC(33f73d4c) SHA1(954695a43e77b58585409678bd87c76adac1d855) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sg2_02.2e",  0x00000, 0x08000, CRC(415ee138) SHA1(626083c8705f012552691c450f95401ddc88065b) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "sg2_03.3e",  0x28000, 0x20000, CRC(43806735) SHA1(88d389bcc79cbd4fa1f4b62008e171a897e77652) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "sg2-01m.3a", 0x0000000, 0x400000, CRC(643ea27b) SHA1(40747432d5cfebac54d3824b6a6f26b5e7742fc1) )
ROM_END

/* 97695-1 */
ROM_START( plsmaswda )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "sg2a_04.2h", 0x0000000, 0x080000, CRC(66e5dada) SHA1(f2e50ee963b8a6aadf25a17b3ff6dcb428b8bdb2) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "sg2-05m.3h", 0x0000000, 0x800000, CRC(f1759236) SHA1(fbe3a820a8c571dfb186eae68346e6461168ed48) )
	ROM_LOAD( "sg2-06m.4h", 0x0800000, 0x800000, CRC(33de4f72) SHA1(ab32af76b5682e3d9f67dadbaed35abc043912b4) )
	ROM_LOAD( "sg2-07m.5h", 0x1000000, 0x800000, CRC(72f724ba) SHA1(e6658b495d308d1de6710f87b5b9d346008b0c5a) )
	ROM_LOAD( "sg2-08m.2k", 0x1800000, 0x800000, CRC(9e169eee) SHA1(6141b1a7863fdfb200ca35d2893979a34dcc3f6c) )
	ROM_LOAD( "sg2-09m.3k", 0x2000000, 0x400000, CRC(33f73d4c) SHA1(954695a43e77b58585409678bd87c76adac1d855) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sg2_02.2e",  0x00000, 0x08000, CRC(415ee138) SHA1(626083c8705f012552691c450f95401ddc88065b) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "sg2_03.3e",  0x28000, 0x20000, CRC(43806735) SHA1(88d389bcc79cbd4fa1f4b62008e171a897e77652) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "sg2-01m.3a", 0x0000000, 0x400000, CRC(643ea27b) SHA1(40747432d5cfebac54d3824b6a6f26b5e7742fc1) )
ROM_END

/* 97695-1 */
ROM_START( stargld2 )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "sg2j_04.2h", 0x0000000, 0x080000, CRC(cf4ce6ac) SHA1(52b6f61d79671c9c108b3dfbd3c2ac333285412c) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "sg2-05m.3h", 0x0000000, 0x800000, CRC(f1759236) SHA1(fbe3a820a8c571dfb186eae68346e6461168ed48) )
	ROM_LOAD( "sg2-06m.4h", 0x0800000, 0x800000, CRC(33de4f72) SHA1(ab32af76b5682e3d9f67dadbaed35abc043912b4) )
	ROM_LOAD( "sg2-07m.5h", 0x1000000, 0x800000, CRC(72f724ba) SHA1(e6658b495d308d1de6710f87b5b9d346008b0c5a) )
	ROM_LOAD( "sg2-08m.2k", 0x1800000, 0x800000, CRC(9e169eee) SHA1(6141b1a7863fdfb200ca35d2893979a34dcc3f6c) )
	ROM_LOAD( "sg2-09m.3k", 0x2000000, 0x400000, CRC(33f73d4c) SHA1(954695a43e77b58585409678bd87c76adac1d855) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sg2_02.2e",  0x00000, 0x08000, CRC(415ee138) SHA1(626083c8705f012552691c450f95401ddc88065b) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "sg2_03.3e",  0x28000, 0x20000, CRC(43806735) SHA1(88d389bcc79cbd4fa1f4b62008e171a897e77652) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "sg2-01m.3a", 0x0000000, 0x400000, CRC(643ea27b) SHA1(40747432d5cfebac54d3824b6a6f26b5e7742fc1) )
ROM_END

/* 97695-1 */
ROM_START( tgmj )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "atej_04.2h", 0x0000000, 0x080000, CRC(bb4bbb96) SHA1(808f4b29493e74efd661d561d11cbec2f4afd1c8) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "ate-05m.3h", 0x0000000, 0x400000, CRC(50977f5a) SHA1(78c2b1965957ff1756c25b76e549f11fc0001153) )
	ROM_LOAD( "ate-06m.4h", 0x0400000, 0x400000, CRC(05973f16) SHA1(c9262e8de14c4a9489f7050316012913c1caf0ff) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ate_02.2e",  0x00000, 0x08000, CRC(f4f6e82f) SHA1(ad6c49197a60f456367c9f78353741fb847819a1) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "ate-01m.3a", 0x0000000, 0x400000, CRC(a21c6521) SHA1(560e4855f6e00def5277bdd12064b49e55c3b46b) )
ROM_END

/* 97695-1 */
ROM_START( techromn )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "kioe_04.2h", 0x0000000, 0x080000, CRC(ebd33b09) SHA1(3f0226d275efc7b97c8d3431211f948aa1271d34) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "kio-05m.3h", 0x0000000, 0x800000, CRC(98e9eb24) SHA1(144773296c213ab09d626c915f90bb74e24487f0) )
	ROM_LOAD( "kio-06m.4h", 0x0800000, 0x800000, CRC(be8d7d73) SHA1(bcbbbd0b83503f2ed32527444e0da3afd774d3f7) )
	ROM_LOAD( "kio-07m.5h", 0x1000000, 0x800000, CRC(ffd81f18) SHA1(f8387a9d45e79f97ccdffabe755638a60f80ccf5) )
	ROM_LOAD( "kio-08m.2k", 0x1800000, 0x800000, CRC(17302226) SHA1(976ba7f48c9a52d24388cd63d02be08627cf2e30) )
	ROM_LOAD( "kio-09m.3k", 0x2000000, 0x800000, CRC(a34f2119) SHA1(50fa992eba5324a173fcc0923227c13cad4f97e5) )
	ROM_LOAD( "kio-10m.4k", 0x2800000, 0x800000, CRC(7400037a) SHA1(d58641e1d6bf1c6ca04f6c98d6809edaa7df75d3) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kio_02.2e",  0x00000, 0x08000, CRC(174309b3) SHA1(b35b9c3905d2fabaa8410f70f7b382e916c89733) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "kio_03.3e",  0x28000, 0x20000, CRC(0b313ae5) SHA1(0ea39305ca30f376930e39b134fd1a52200624fa) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "kio-01m.3a", 0x0000000, 0x400000, CRC(6dc5bd07) SHA1(e1755a48465f741691ea0fa1166cb2dc09210ed9) )
ROM_END

/* 97695-1 */
ROM_START( techromnu )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "kiou_04.2h", 0x0000000, 0x080000, CRC(08aca34a) SHA1(768a37f719af5d96993db5592b6505b013e0d6f4) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "kio-05m.3h", 0x0000000, 0x800000, CRC(98e9eb24) SHA1(144773296c213ab09d626c915f90bb74e24487f0) )
	ROM_LOAD( "kio-06m.4h", 0x0800000, 0x800000, CRC(be8d7d73) SHA1(bcbbbd0b83503f2ed32527444e0da3afd774d3f7) )
	ROM_LOAD( "kio-07m.5h", 0x1000000, 0x800000, CRC(ffd81f18) SHA1(f8387a9d45e79f97ccdffabe755638a60f80ccf5) )
	ROM_LOAD( "kio-08m.2k", 0x1800000, 0x800000, CRC(17302226) SHA1(976ba7f48c9a52d24388cd63d02be08627cf2e30) )
	ROM_LOAD( "kio-09m.3k", 0x2000000, 0x800000, CRC(a34f2119) SHA1(50fa992eba5324a173fcc0923227c13cad4f97e5) )
	ROM_LOAD( "kio-10m.4k", 0x2800000, 0x800000, CRC(7400037a) SHA1(d58641e1d6bf1c6ca04f6c98d6809edaa7df75d3) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kio_02.2e",  0x00000, 0x08000, CRC(174309b3) SHA1(b35b9c3905d2fabaa8410f70f7b382e916c89733) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "kio_03.3e",  0x28000, 0x20000, CRC(0b313ae5) SHA1(0ea39305ca30f376930e39b134fd1a52200624fa) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "kio-01m.3a", 0x0000000, 0x400000, CRC(6dc5bd07) SHA1(e1755a48465f741691ea0fa1166cb2dc09210ed9) )
ROM_END

/* 97695-1 */
ROM_START( kikaioh )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "kioj_04.2h", 0x0000000, 0x080000, CRC(3a2a3bc8) SHA1(3c4ae3cfe00a7f60ab2196ae042dab4a8eb6f597) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "kio-05m.3h", 0x0000000, 0x800000, CRC(98e9eb24) SHA1(144773296c213ab09d626c915f90bb74e24487f0) )
	ROM_LOAD( "kio-06m.4h", 0x0800000, 0x800000, CRC(be8d7d73) SHA1(bcbbbd0b83503f2ed32527444e0da3afd774d3f7) )
	ROM_LOAD( "kio-07m.5h", 0x1000000, 0x800000, CRC(ffd81f18) SHA1(f8387a9d45e79f97ccdffabe755638a60f80ccf5) )
	ROM_LOAD( "kio-08m.2k", 0x1800000, 0x800000, CRC(17302226) SHA1(976ba7f48c9a52d24388cd63d02be08627cf2e30) )
	ROM_LOAD( "kio-09m.3k", 0x2000000, 0x800000, CRC(a34f2119) SHA1(50fa992eba5324a173fcc0923227c13cad4f97e5) )
	ROM_LOAD( "kio-10m.4k", 0x2800000, 0x800000, CRC(7400037a) SHA1(d58641e1d6bf1c6ca04f6c98d6809edaa7df75d3) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "kio_02.2e",  0x00000, 0x08000, CRC(174309b3) SHA1(b35b9c3905d2fabaa8410f70f7b382e916c89733) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "kio_03.3e",  0x28000, 0x20000, CRC(0b313ae5) SHA1(0ea39305ca30f376930e39b134fd1a52200624fa) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "kio-01m.3a", 0x0000000, 0x400000, CRC(6dc5bd07) SHA1(e1755a48465f741691ea0fa1166cb2dc09210ed9) )
ROM_END

/* 97695-1 */
ROM_START( sfex2p )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "x2pu_04.2h", 0x0000000, 0x080000, CRC(2938118c) SHA1(4bdeeb9aa3dd54ef44aa3fc73d78d65297b1ed25) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "x2p-05m.3h", 0x0000000, 0x800000, CRC(4ee3110f) SHA1(704f8dca7d0b698659af9e3271ea5072dfd42b8b) )
	ROM_LOAD( "x2p-06m.4h", 0x0800000, 0x800000, CRC(4cd53a45) SHA1(39499ea6c9aa51c71f4fe44cc02f93d5a39e14ec) )
	ROM_LOAD( "x2p-07m.5h", 0x1000000, 0x800000, CRC(11207c2a) SHA1(0182652819f1c3a36e7b42e34ef86d2455a2dd90) )
	ROM_LOAD( "x2p-08m.2k", 0x1800000, 0x800000, CRC(3560c2cc) SHA1(8b0ce22d954387f7bb032b5220d1014ef68741e8) )
	ROM_LOAD( "x2p-09m.3k", 0x2000000, 0x800000, CRC(344aa227) SHA1(69dc6f511939bf7fa25c2531ecf307a7565fe7a8) )
	ROM_LOAD( "x2p-10m.4k", 0x2800000, 0x800000, CRC(2eef5931) SHA1(e5227529fb68eeb1b2f25813694173a75d906b52) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "x2p_02.2e",  0x00000, 0x08000, CRC(3705de5e) SHA1(847007ca271da64bf13ffbf496d4291429eee27a) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "x2p_03.3e",  0x28000, 0x20000, CRC(6ae828f6) SHA1(41c54165e87b846a845da581f408b96979288158) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "x2p-01m.3a", 0x0000000, 0x400000, CRC(14a5bb0e) SHA1(dfe3c3a53bd4c58743d8039b5344d3afbe2a9c24) )
ROM_END

/* 97695-1 */
ROM_START( sfex2pa )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "x2pa_04.2h", 0x0000000, 0x080000, CRC(c437d602) SHA1(150f0dfd9f2e4f9adc11f8960da1e6be250456b1) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "x2p-05m.3h", 0x0000000, 0x800000, CRC(4ee3110f) SHA1(704f8dca7d0b698659af9e3271ea5072dfd42b8b) )
	ROM_LOAD( "x2p-06m.4h", 0x0800000, 0x800000, CRC(4cd53a45) SHA1(39499ea6c9aa51c71f4fe44cc02f93d5a39e14ec) )
	ROM_LOAD( "x2p-07m.5h", 0x1000000, 0x800000, CRC(11207c2a) SHA1(0182652819f1c3a36e7b42e34ef86d2455a2dd90) )
	ROM_LOAD( "x2p-08m.2k", 0x1800000, 0x800000, CRC(3560c2cc) SHA1(8b0ce22d954387f7bb032b5220d1014ef68741e8) )
	ROM_LOAD( "x2p-09m.3k", 0x2000000, 0x800000, CRC(344aa227) SHA1(69dc6f511939bf7fa25c2531ecf307a7565fe7a8) )
	ROM_LOAD( "x2p-10m.4k", 0x2800000, 0x800000, CRC(2eef5931) SHA1(e5227529fb68eeb1b2f25813694173a75d906b52) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "x2p_02.2e",  0x00000, 0x08000, CRC(3705de5e) SHA1(847007ca271da64bf13ffbf496d4291429eee27a) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "x2p_03.3e",  0x28000, 0x20000, CRC(6ae828f6) SHA1(41c54165e87b846a845da581f408b96979288158) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "x2p-01m.3a", 0x0000000, 0x400000, CRC(14a5bb0e) SHA1(dfe3c3a53bd4c58743d8039b5344d3afbe2a9c24) )
ROM_END

/* 97695-1 */
ROM_START( sfex2ph )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "x2ph_04.2h", 0x0000000, 0x080000, CRC(a4f07439) SHA1(630be537062134da2d01866b9587e5c119c198bb) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "x2p-05m.3h", 0x0000000, 0x800000, CRC(4ee3110f) SHA1(704f8dca7d0b698659af9e3271ea5072dfd42b8b) )
	ROM_LOAD( "x2p-06m.4h", 0x0800000, 0x800000, CRC(4cd53a45) SHA1(39499ea6c9aa51c71f4fe44cc02f93d5a39e14ec) )
	ROM_LOAD( "x2p-07m.5h", 0x1000000, 0x800000, CRC(11207c2a) SHA1(0182652819f1c3a36e7b42e34ef86d2455a2dd90) )
	ROM_LOAD( "x2p-08m.2k", 0x1800000, 0x800000, CRC(3560c2cc) SHA1(8b0ce22d954387f7bb032b5220d1014ef68741e8) )
	ROM_LOAD( "x2p-09m.3k", 0x2000000, 0x800000, CRC(344aa227) SHA1(69dc6f511939bf7fa25c2531ecf307a7565fe7a8) )
	ROM_LOAD( "x2p-10m.4k", 0x2800000, 0x800000, CRC(2eef5931) SHA1(e5227529fb68eeb1b2f25813694173a75d906b52) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "x2p_02.2e",  0x00000, 0x08000, CRC(3705de5e) SHA1(847007ca271da64bf13ffbf496d4291429eee27a) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "x2p_03.3e",  0x28000, 0x20000, CRC(6ae828f6) SHA1(41c54165e87b846a845da581f408b96979288158) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "x2p-01m.3a", 0x0000000, 0x400000, CRC(14a5bb0e) SHA1(dfe3c3a53bd4c58743d8039b5344d3afbe2a9c24) )
ROM_END

/* 97695-1 */
ROM_START( sfex2pj )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "x2pj_04.2h", 0x0000000, 0x080000, CRC(c6d0aea3) SHA1(f48ee889dd743109f830063da3eb0f687db2d86c) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "x2p-05m.3h", 0x0000000, 0x800000, CRC(4ee3110f) SHA1(704f8dca7d0b698659af9e3271ea5072dfd42b8b) )
	ROM_LOAD( "x2p-06m.4h", 0x0800000, 0x800000, CRC(4cd53a45) SHA1(39499ea6c9aa51c71f4fe44cc02f93d5a39e14ec) )
	ROM_LOAD( "x2p-07m.5h", 0x1000000, 0x800000, CRC(11207c2a) SHA1(0182652819f1c3a36e7b42e34ef86d2455a2dd90) )
	ROM_LOAD( "x2p-08m.2k", 0x1800000, 0x800000, CRC(3560c2cc) SHA1(8b0ce22d954387f7bb032b5220d1014ef68741e8) )
	ROM_LOAD( "x2p-09m.3k", 0x2000000, 0x800000, CRC(344aa227) SHA1(69dc6f511939bf7fa25c2531ecf307a7565fe7a8) )
	ROM_LOAD( "x2p-10m.4k", 0x2800000, 0x800000, CRC(2eef5931) SHA1(e5227529fb68eeb1b2f25813694173a75d906b52) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "x2p_02.2e",  0x00000, 0x08000, CRC(3705de5e) SHA1(847007ca271da64bf13ffbf496d4291429eee27a) )
	ROM_CONTINUE(           0x10000, 0x18000 )
	ROM_LOAD( "x2p_03.3e",  0x28000, 0x20000, CRC(6ae828f6) SHA1(41c54165e87b846a845da581f408b96979288158) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "x2p-01m.3a", 0x0000000, 0x400000, CRC(14a5bb0e) SHA1(dfe3c3a53bd4c58743d8039b5344d3afbe2a9c24) )
ROM_END

/* 97695-1 */
ROM_START( strider2 )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "hr2u_04.2h", 0x0000000, 0x080000, CRC(b28b01c6) SHA1(ad40f550ce14f09cf34d51b9b0b7154c31c8936e) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "hr2-05m.3h", 0x0000000, 0x800000, CRC(18716fe8) SHA1(bb923f18120086054cd6fd91f77d27a190c1eed4) )
	ROM_LOAD( "hr2-06m.4h", 0x0800000, 0x800000, CRC(6f13b69c) SHA1(9a14ecc72631bc44053af71fe7e3934bedf1a71e) )
	ROM_LOAD( "hr2-07m.5h", 0x1000000, 0x800000, CRC(3925701b) SHA1(d93218d2b97cc0fc6c30221bd6b5e955520fbc46) )
	ROM_LOAD( "hr2-08m.2k", 0x1800000, 0x800000, CRC(d844c0dc) SHA1(6010cfbf4dc42fda182884d78e12dcb63df00249) )
	ROM_LOAD( "hr2-09m.3k", 0x2000000, 0x800000, CRC(cdd43e6b) SHA1(346a83deadecd56428276acefc2ce95249a49921) )
	ROM_LOAD( "hr2-10m.4k", 0x2800000, 0x400000, CRC(d95b3f37) SHA1(b6566c1184718f6c0986d13060894c0fb400c201) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "hr2_02.2e",  0x00000, 0x08000, CRC(acd8d385) SHA1(5edb61c3d66d2d09a28a71db52eee3a9f7db8c9d) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "hr2-01m.3a", 0x0000000, 0x200000, CRC(510a16d1) SHA1(05f10c2921a4d3b1fab4d0a4ea06351809bdbb07) )
	ROM_RELOAD(                         0x0200000, 0x200000 )
ROM_END

/* 97695-1 */
ROM_START( strider2a )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "hr2a_04.2h", 0x0000000, 0x080000, CRC(56ff9394) SHA1(fe8417965d945210ac098c6678c02f1c678bd13b) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "hr2-05m.3h", 0x0000000, 0x800000, CRC(18716fe8) SHA1(bb923f18120086054cd6fd91f77d27a190c1eed4) )
	ROM_LOAD( "hr2-06m.4h", 0x0800000, 0x800000, CRC(6f13b69c) SHA1(9a14ecc72631bc44053af71fe7e3934bedf1a71e) )
	ROM_LOAD( "hr2-07m.5h", 0x1000000, 0x800000, CRC(3925701b) SHA1(d93218d2b97cc0fc6c30221bd6b5e955520fbc46) )
	ROM_LOAD( "hr2-08m.2k", 0x1800000, 0x800000, CRC(d844c0dc) SHA1(6010cfbf4dc42fda182884d78e12dcb63df00249) )
	ROM_LOAD( "hr2-09m.3k", 0x2000000, 0x800000, CRC(cdd43e6b) SHA1(346a83deadecd56428276acefc2ce95249a49921) )
	ROM_LOAD( "hr2-10m.4k", 0x2800000, 0x400000, CRC(d95b3f37) SHA1(b6566c1184718f6c0986d13060894c0fb400c201) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "hr2_02.2e",  0x00000, 0x08000, CRC(acd8d385) SHA1(5edb61c3d66d2d09a28a71db52eee3a9f7db8c9d) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "hr2-01m.3a", 0x0000000, 0x200000, CRC(510a16d1) SHA1(05f10c2921a4d3b1fab4d0a4ea06351809bdbb07) )
	ROM_RELOAD(                         0x0200000, 0x200000 )
ROM_END

/* 97695-1 */
ROM_START( shiryu2 )
	CPZN2_BIOS

	ROM_REGION32_LE( 0x80000, "countryrom", 0 )
	ROM_LOAD( "hr2j_04.2h", 0x0000000, 0x080000, CRC(0824ee5f) SHA1(a296ffe03f0d947deb9803d05de3c240a26b52bb) )

	ROM_REGION32_LE( 0x3000000, "maskroms", 0 )
	ROM_LOAD( "hr2-05m.3h", 0x0000000, 0x800000, CRC(18716fe8) SHA1(bb923f18120086054cd6fd91f77d27a190c1eed4) )
	ROM_LOAD( "hr2-06m.4h", 0x0800000, 0x800000, CRC(6f13b69c) SHA1(9a14ecc72631bc44053af71fe7e3934bedf1a71e) )
	ROM_LOAD( "hr2-07m.5h", 0x1000000, 0x800000, CRC(3925701b) SHA1(d93218d2b97cc0fc6c30221bd6b5e955520fbc46) )
	ROM_LOAD( "hr2-08m.2k", 0x1800000, 0x800000, CRC(d844c0dc) SHA1(6010cfbf4dc42fda182884d78e12dcb63df00249) )
	ROM_LOAD( "hr2-09m.3k", 0x2000000, 0x800000, CRC(cdd43e6b) SHA1(346a83deadecd56428276acefc2ce95249a49921) )
	ROM_LOAD( "hr2-10m.4k", 0x2800000, 0x400000, CRC(d95b3f37) SHA1(b6566c1184718f6c0986d13060894c0fb400c201) )

	ROM_REGION( 0x50000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "hr2_02.2e",  0x00000, 0x08000, CRC(acd8d385) SHA1(5edb61c3d66d2d09a28a71db52eee3a9f7db8c9d) )
	ROM_CONTINUE(           0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* Q Sound Samples */
	ROM_LOAD16_WORD_SWAP( "hr2-01m.3a", 0x0000000, 0x200000, CRC(510a16d1) SHA1(05f10c2921a4d3b1fab4d0a4ea06351809bdbb07) )
	ROM_RELOAD(                         0x0200000, 0x200000 )
ROM_END

/* Tecmo */

#define TPS_BIOS \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_LOAD( "coh-1002m.353", 0x0000000, 0x080000, CRC(69ffbcb4) SHA1(03eb2febfab3fcde716defff291babd9392de965) ) \
	ROM_REGION( 0x2000, "mcu", 0 ) \
	ROM_LOAD( "upd78081.655", 0x0000, 0x2000, NO_DUMP ) /* internal rom :( */

ROM_START( tps )
	TPS_BIOS
	ROM_REGION32_LE( 0x02800000, "bankedroms", ROMREGION_ERASE00 )
ROM_END

/*

 TPS ROM addressing note: .216 *if present* goes at 0x400000, else nothing there.
 .217 goes at 0x800000, .218 at 0xc00000, .219 at 0x1000000, and so on.

*/



ROM_START( glpracr2 )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "gallop2u.119", 0x0000001, 0x100000, CRC(9899911c) SHA1(f043fb97760c53422ad6aeb214474c0be00017ce) )
	ROM_LOAD16_BYTE( "gallop2u.120", 0x0000000, 0x100000, CRC(fd69bd4b) SHA1(26a183bdc3b2fb3d93bd7694e429a676106f4e58) )
	ROM_LOAD( "gra2-0.217",          0x0800000, 0x400000, CRC(a077ffa3) SHA1(73492ec2145246276bfe25b27d7de4f6393124f4) )
	ROM_LOAD( "gra2-1.218",          0x0c00000, 0x400000, CRC(28ce033c) SHA1(4dc53e5c82fde683efd72c66b397d56aa72d52b9) )
	ROM_LOAD( "gra2-2.219",          0x1000000, 0x400000, CRC(0c9cb7da) SHA1(af23c11e69428413ff4d1c2746adb786de927cb5) )
	ROM_LOAD( "gra2-3.220",          0x1400000, 0x400000, CRC(264e3a0c) SHA1(c1509b16d7192b9f61dbceb299290239219adefd) )
	ROM_LOAD( "gra2-4.221",          0x1800000, 0x400000, BAD_DUMP CRC(2b070307) SHA1(43c028aaca297358f87c6633c2020d71e34317b8) ) // gra2-4.221                                      BADADDR  xxxxxxxxxxxxxxxxxx-xxx
	ROM_LOAD( "gra2-5.222",          0x1c00000, 0x400000, BAD_DUMP CRC(94a363c1) SHA1(4c53822a672ac99b001c9fe82f9d0f8496989e67) ) // gra2-5.222                                      BADADDR  xxxxxxxxxxxxxxxxxxx--x
	ROM_LOAD( "gra2-6.223",          0x2000000, 0x400000, CRC(8c6b4c4c) SHA1(0053f736dcd437c01da8cadd820e8af658ce6077) )
	ROM_LOAD( "gra2-7.323",          0x2400000, 0x400000, CRC(7dfb6c54) SHA1(6e9a9a4172f957ba354ddd82c30735a56c5934b1) )
ROM_END

ROM_START( glpracr2j )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "1.119",        0x0000001, 0x100000, CRC(0fe2d2df) SHA1(031369f4e1138e2ee293c321e5ee418e560b3f06) )
	ROM_LOAD16_BYTE( "2.120",        0x0000000, 0x100000, CRC(8e3fb1c0) SHA1(2126c1e43bee7cd938e0f2a3ea841da8811223cd) )
	ROM_LOAD( "gra2-0.217",          0x0800000, 0x400000, CRC(a077ffa3) SHA1(73492ec2145246276bfe25b27d7de4f6393124f4) )
	ROM_LOAD( "gra2-1.218",          0x0c00000, 0x400000, CRC(28ce033c) SHA1(4dc53e5c82fde683efd72c66b397d56aa72d52b9) )
	ROM_LOAD( "gra2-2.219",          0x1000000, 0x400000, CRC(0c9cb7da) SHA1(af23c11e69428413ff4d1c2746adb786de927cb5) )
	ROM_LOAD( "gra2-3.220",          0x1400000, 0x400000, CRC(264e3a0c) SHA1(c1509b16d7192b9f61dbceb299290239219adefd) )
	ROM_LOAD( "gra2-4.221",          0x1800000, 0x400000, BAD_DUMP CRC(2b070307) SHA1(43c028aaca297358f87c6633c2020d71e34317b8) ) // gra2-4.221                                      BADADDR  xxxxxxxxxxxxxxxxxx-xxx
	ROM_LOAD( "gra2-5.222",          0x1c00000, 0x400000, BAD_DUMP CRC(94a363c1) SHA1(4c53822a672ac99b001c9fe82f9d0f8496989e67) ) // gra2-5.222                                      BADADDR  xxxxxxxxxxxxxxxxxxx--x
	ROM_LOAD( "gra2-6.223",          0x2000000, 0x400000, CRC(8c6b4c4c) SHA1(0053f736dcd437c01da8cadd820e8af658ce6077) )
	ROM_LOAD( "gra2-7.323",          0x2400000, 0x400000, CRC(7dfb6c54) SHA1(6e9a9a4172f957ba354ddd82c30735a56c5934b1) )
ROM_END

ROM_START( glpracr2l )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "gra2b.119",    0x0000001, 0x100000, CRC(43abee7c) SHA1(ea0afc820d8480c12c9af54057877ff11a8012fb) )
	ROM_LOAD16_BYTE( "gra2a.120",    0x0000000, 0x100000, CRC(f60096d4) SHA1(5349d780d41a5711b483cd7eb66cd4e496b4fbe4) )
	ROM_LOAD( "gra2-0.217",          0x0800000, 0x400000, CRC(a077ffa3) SHA1(73492ec2145246276bfe25b27d7de4f6393124f4) )
	ROM_LOAD( "gra2-1.218",          0x0c00000, 0x400000, CRC(28ce033c) SHA1(4dc53e5c82fde683efd72c66b397d56aa72d52b9) )
	ROM_LOAD( "gra2-2.219",          0x1000000, 0x400000, CRC(0c9cb7da) SHA1(af23c11e69428413ff4d1c2746adb786de927cb5) )
	ROM_LOAD( "gra2-3.220",          0x1400000, 0x400000, CRC(264e3a0c) SHA1(c1509b16d7192b9f61dbceb299290239219adefd) )
	ROM_LOAD( "gra2-4.221",          0x1800000, 0x400000, BAD_DUMP CRC(2b070307) SHA1(43c028aaca297358f87c6633c2020d71e34317b8) ) // gra2-4.221                                      BADADDR  xxxxxxxxxxxxxxxxxx-xxx
	ROM_LOAD( "gra2-5.222",          0x1c00000, 0x400000, BAD_DUMP CRC(94a363c1) SHA1(4c53822a672ac99b001c9fe82f9d0f8496989e67) ) // gra2-5.222                                      BADADDR  xxxxxxxxxxxxxxxxxxx--x
	ROM_LOAD( "gra2-6.223",          0x2000000, 0x400000, CRC(8c6b4c4c) SHA1(0053f736dcd437c01da8cadd820e8af658ce6077) )
	ROM_LOAD( "gra2-7.323",          0x2400000, 0x400000, CRC(7dfb6c54) SHA1(6e9a9a4172f957ba354ddd82c30735a56c5934b1) )

	ROM_REGION( 0x040000, "link", 0 )
	ROM_LOAD( "link3118.bin",        0x0000000, 0x040000, CRC(a4d4761e) SHA1(3fb25dfa5220d25093588d9501e0666214491100) )
ROM_END

ROM_START( cbaj )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "cbaj_1.119",   0x0000001, 0x080000, CRC(814f8b4b) SHA1(17966038a692d0701139660f25725d7c10a2a928) )
	ROM_LOAD16_BYTE( "cbaj_2.120",   0x0000000, 0x080000, CRC(89286229) SHA1(18a84ef648ec3b79707eb42b55563adf38dffd0d) )
	ROM_LOAD( "cb-00.216",           0x0400000, 0x400000, CRC(3db68bea) SHA1(77ab334e0c02e608b11d8fdb9505b2301f6f9afb) )
	ROM_LOAD( "cb-01.217",           0x0800000, 0x400000, CRC(481040bc) SHA1(c6fe575b77d1eb5f613691dec5ed08929b72b955) )
	ROM_LOAD( "cb-02.218",           0x0c00000, 0x400000, CRC(858f116c) SHA1(e3546862d367d2fe88913fea3185b23bc6a9777d) )
	ROM_LOAD( "cb-03.219",           0x1000000, 0x400000, CRC(3576ea2a) SHA1(a5ee7bb9f4650e99ee067eb1cc28c62d9099a6cf) )
	ROM_LOAD( "cb-04.220",           0x1400000, 0x400000, CRC(551c4b29) SHA1(c3f8508a006b475491c9ea20eb64c3bea6b35afb) )
	ROM_LOAD( "cb-05.221",           0x1800000, 0x400000, CRC(7da453da) SHA1(85b2c93b9453e8c7791b530b7e036e4ef6abc077) )
	ROM_LOAD( "cb-06.222",           0x1c00000, 0x400000, CRC(833cb18b) SHA1(dbc390e1dbf3e7815eb3d170c0890d3785d8002c) )
	ROM_LOAD( "cb-07.223",           0x2000000, 0x400000, CRC(3b64ce9e) SHA1(a137da126295736bb7643655d52bd570004e87fd) )
	ROM_LOAD( "cb-08.323",           0x2400000, 0x400000, CRC(57cc482e) SHA1(603c3d13a6cd796c209a97aa7e63b77bdbf71580) )

	ROM_REGION( 0x040000, "audiocpu", 0 )
	ROM_LOAD( "cbaj_z80.3118",       0x0000000, 0x040000, CRC(92b02ad2) SHA1(f72317679ecbd8a0c3b081baaf9ff20a8c9ec00f) )

	ROM_REGION( 0x800000, "ymz", 0 ) /* YMZ280B Sound Samples */
	ROM_LOAD( "cb-vo.5120",   0x000000, 0x400000, CRC(afb05d6d) SHA1(0c08010579813814fbf8a978cf4376bab18697a4) )
	ROM_LOAD( "cb-se.5121",   0x400000, 0x400000, CRC(f12b3db9) SHA1(d5231ad664603050bdca2081b114b07fc905ddc2) )
ROM_END

ROM_START( shngmtkb )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "shmj-b.119",   0x0000001, 0x080000, CRC(65522c67) SHA1(b5981e5859aab742a87d6742feb9c55a3e6ba13f) )
	ROM_LOAD16_BYTE( "shmj-a.120",   0x0000000, 0x080000, CRC(a789defa) SHA1(f8f0d1c9e3492cda652a9561ef1d549b92f73efd) )
	ROM_LOAD( "sh-00.217",           0x0800000, 0x400000, CRC(081fed1c) SHA1(fb18add9521b8b104329871b4c1b8ae5e0254f8b) )
	ROM_LOAD( "sh-01.218",           0x0c00000, 0x400000, CRC(5a84ea96) SHA1(af4972cc10706999361d7505b975f5f1e1fc6761) )
	ROM_LOAD( "sh-02.219",           0x1000000, 0x400000, CRC(c8f80d76) SHA1(51e4eac6cec8e37e5b8c0e7d341feea574add7da) )
	ROM_LOAD( "sh-03.220",           0x1400000, 0x400000, CRC(daaa4c73) SHA1(eb31d4cadd9eba3d3431f3f6ef880bb2effa0b9f) )
ROM_END

ROM_START( doapp )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "doapp119.bin", 0x0000001, 0x100000, CRC(bbe04cef) SHA1(f2dae4810ca78075fc3007a6001531a455235a2e) )
	ROM_LOAD16_BYTE( "doapp120.bin", 0x0000000, 0x100000, CRC(b614d7e6) SHA1(373756d9b88b45c677e987ee1e5cb2d5446ecfe8) )
	ROM_LOAD( "doapp-0.216",         0x0400000, 0x400000, CRC(acc6c539) SHA1(a744567a3d75634098b1749103307981be9acbdd) )
	ROM_LOAD( "doapp-1.217",         0x0800000, 0x400000, CRC(14b961c4) SHA1(3fae1fcb4665ba8bad391881b26c2d087718d42f) )
	ROM_LOAD( "doapp-2.218",         0x0c00000, 0x400000, CRC(134f698f) SHA1(6422972cf5d30a0f09f0c20f042691d5969207b4) )
	ROM_LOAD( "doapp-3.219",         0x1000000, 0x400000, CRC(1c6540f3) SHA1(8631fde93a1da6325d7b31c7edf12c964f0ac4fc) )
	ROM_LOAD( "doapp-4.220",         0x1400000, 0x400000, CRC(f83bacf7) SHA1(5bd66da993f0db966581dde80dd7e5b377754412) )
	ROM_LOAD( "doapp-5.221",         0x1800000, 0x400000, CRC(e11e8b71) SHA1(b1d1b9532b5f074ce216a603436d5674d136865d) )
ROM_END

ROM_START( tondemo )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "u0119.bin",    0x0000001, 0x100000, CRC(5711e301) SHA1(005375d32c1eda9bd39e46326880a62506d06389) )
	ROM_LOAD16_BYTE( "u0120.bin",    0x0000000, 0x100000, CRC(0b8312c6) SHA1(93e0e4b796cc953daf7ed2ff2f327aed07cf833a) )
	ROM_LOAD( "tca-0.217",           0x0800000, 0x400000, CRC(ef175910) SHA1(b77aa9016804172d433d97d5fdc242a1361e941c) )
	ROM_LOAD( "tca-1.218",           0x0c00000, 0x400000, CRC(c3474e8a) SHA1(46dd0ae7cd2e54c639fe39d6965ef71ce6a1b921) )
	ROM_LOAD( "tca-2.219",           0x1000000, 0x400000, CRC(89b8e1a8) SHA1(70c5f0f2d0a7869e29b62b32fa485f941b683678) )
	ROM_LOAD( "tca-3.220",           0x1400000, 0x400000, CRC(4fcf8032) SHA1(3ea815548c3bda32b1d4e88454c29e5025431b1c) )
	ROM_LOAD( "tca-4.221",           0x1800000, 0x400000, CRC(c9e23f25) SHA1(145d4e7f0cb67d2552559ce90305a56802a253f9) )
ROM_END

ROM_START( glpracr3 )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "1.119", 0x0000001, 0x100000, CRC(89bdf567) SHA1(916accbcad52e9ee4e3b28a339138fe2bfbecdfe) )
	ROM_LOAD16_BYTE( "2.120", 0x0000000, 0x100000, CRC(042273fb) SHA1(eb98c4e74f385ddc6545b9250df5858b39fe361d) )
	ROM_LOAD( "gra3-0.216",          0x0400000, 0x400000, CRC(b405ee65) SHA1(8ba9872e4c166e3b659a2802554bf1e964f64620) )
	ROM_LOAD( "gra3-1.217",          0x0800000, 0x400000, CRC(a06f05ac) SHA1(ade224533d75c64cb188b78bdca908c1fa882492) )
	ROM_LOAD( "gra3-2.218",          0x0c00000, 0x400000, CRC(31793f9b) SHA1(310c2dff84d17c9ed7f59e249b22e9394edcb444) )
	ROM_LOAD( "gra3-3.219",          0x1000000, 0x400000, CRC(d59fb3eb) SHA1(2db2cc1d4884d54c415531053319f2b2ad65361f) )
	ROM_LOAD( "gra3-4.220",          0x1400000, 0x400000, CRC(59a0a105) SHA1(6a585c9eaa8d9b5dad798d9d28d73f04bc838114) )
	ROM_LOAD( "gra3-5.221",          0x1800000, 0x400000, CRC(4994fb17) SHA1(59b3e6c333e55ca8b6b4b00cd52b51e3e59a5657) )
	ROM_LOAD( "gra3-6.222",          0x1c00000, 0x400000, CRC(1362c1af) SHA1(eae5b3cb11d361b3aa3f572e49800c0b2e3544ca) )
	ROM_LOAD( "gra3-7.223",          0x2000000, 0x400000, CRC(73565e1f) SHA1(74311ee94e3abc8428b4a8b1c6c3dacd883b5646) )

	ROM_REGION( 0x200, "misc", 0 )
	ROM_LOAD( "rom1.gal16v8d.u0117.bin",          0x0000, 0x117, CRC(cf8ebc23) SHA1(0662f8ba418eb9187fb7a86cc8c0d86220dcdbf0) ) // unprotected, verified on PCB, near the MG08 security chip
ROM_END


ROM_START( glpracr3j )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "gra3u119.119", 0x0000001, 0x100000, CRC(aba69017) SHA1(670b895ee7d36bc5a00f6b0df7ce965517986617) )
	ROM_LOAD16_BYTE( "gra3u120.120", 0x0000000, 0x100000, CRC(8aa98d99) SHA1(9dc1ba89e37a5c2955ee027e4e5aa0ae71e09f9b) )
	ROM_LOAD( "gra3-0.216",          0x0400000, 0x400000, CRC(b405ee65) SHA1(8ba9872e4c166e3b659a2802554bf1e964f64620) )
	ROM_LOAD( "gra3-1.217",          0x0800000, 0x400000, CRC(a06f05ac) SHA1(ade224533d75c64cb188b78bdca908c1fa882492) )
	ROM_LOAD( "gra3-2.218",          0x0c00000, 0x400000, CRC(31793f9b) SHA1(310c2dff84d17c9ed7f59e249b22e9394edcb444) )
	ROM_LOAD( "gra3-3.219",          0x1000000, 0x400000, CRC(d59fb3eb) SHA1(2db2cc1d4884d54c415531053319f2b2ad65361f) )
	ROM_LOAD( "gra3-4.220",          0x1400000, 0x400000, CRC(59a0a105) SHA1(6a585c9eaa8d9b5dad798d9d28d73f04bc838114) )
	ROM_LOAD( "gra3-5.221",          0x1800000, 0x400000, CRC(4994fb17) SHA1(59b3e6c333e55ca8b6b4b00cd52b51e3e59a5657) )
	ROM_LOAD( "gra3-6.222",          0x1c00000, 0x400000, CRC(1362c1af) SHA1(eae5b3cb11d361b3aa3f572e49800c0b2e3544ca) )
	ROM_LOAD( "gra3-7.223",          0x2000000, 0x400000, CRC(73565e1f) SHA1(74311ee94e3abc8428b4a8b1c6c3dacd883b5646) )
ROM_END



ROM_START( tecmowcm )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "twm-ep.119",  0x0000001, 0x100000, CRC(5f2908fb) SHA1(fc7ac1f6e81543678705e6e510dbf786b1502444) )
	ROM_LOAD16_BYTE( "twm-ep.120",  0x0000000, 0x100000, CRC(1a0ef17a) SHA1(bb7123610d3791c08577b87c8be59a0dd2cc33f1) )
	ROM_LOAD( "twm-0.216",          0x0400000, 0x400000, CRC(39cbc56a) SHA1(931d0d729620ef20e5c4fd521bce45cdb1742127) )
	ROM_LOAD( "twm-1.217",          0x0800000, 0x400000, CRC(fae0687a) SHA1(383a86f55441be287075af046ebac6a5ab54e6cf) )
	ROM_LOAD( "twm-2.218",          0x0c00000, 0x400000, CRC(cb852264) SHA1(a7a2f3d6f723ddd80c57ac63522a1a0bf526a7b3) )
	ROM_LOAD( "twm-3.219",          0x1000000, 0x400000, CRC(7c9f6925) SHA1(32519a238810d02181eaf5c2344334c523fa77d1) )
	ROM_LOAD( "twm-4.220",          0x1400000, 0x400000, CRC(17cd0ec9) SHA1(37581530e974af692ab71471d0238801cd19c843) )
ROM_END

ROM_START( flamegun )
	TPS_BIOS

	ROM_REGION32_LE( 0x03400000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "flamegun.119",   0x0000001, 0x100000, CRC(bc8e9e29) SHA1(02e4f079f0ed864dbc056d5f64d33a0522c034fd) )
	ROM_LOAD16_BYTE( "flamegun.120",   0x0000000, 0x100000, CRC(387f3070) SHA1(6b12765f1d3ec5f3d1cdfd961fba72a319d65ff4) )
	ROM_LOAD( "fg00.216",              0x0400000, 0x400000, CRC(f33736ca) SHA1(7b18f9fef1df913b7ed3a2c97e9c4925790d86c5) )
	ROM_LOAD( "fg01.217",              0x0800000, 0x400000, CRC(8980ff44) SHA1(f677bf5d279ad7731730b7e36ebf33d554903ce6) )
	ROM_LOAD( "fg02.218",              0x0c00000, 0x400000, CRC(97d1b032) SHA1(1d23cd40ced002ae1fe2fb009d2f31d8612b125a) )
	ROM_LOAD( "fg03.219",              0x1000000, 0x400000, CRC(cf84508c) SHA1(2a1173e3751f5a8b2400219a75f23d2450a0ebd6) )
	ROM_LOAD( "fg04.220",              0x1400000, 0x400000, CRC(5cc333fa) SHA1(fd81e811ef2026b245e65c104d24ae1679baa0f5) )
	ROM_LOAD( "fg05.221",              0x1800000, 0x400000, CRC(9490bc1b) SHA1(c5ea133de0a271793601f2701267d3ca82781f60) )
	ROM_LOAD( "fg06.222",              0x1c00000, 0x400000, CRC(8c29b754) SHA1(304086196321b7d9748456ed1af8965ac6192942) )
	ROM_LOAD( "fg07.223",              0x2000000, 0x400000, CRC(f62cefe8) SHA1(a59a3a41258e8474f0aeb0e3b8c2f73caf47ece4) )
	ROM_LOAD( "fg08.323",              0x2400000, 0x400000, CRC(855959b4) SHA1(994b7c8bd883a41470791802d5a07a98a553096e) )
	ROM_LOAD( "fg09.324",              0x2800000, 0x400000, CRC(82f129b4) SHA1(c0dcbc908c12f7cecbb051a671649edd20bac32c) )
	ROM_LOAD( "fg0a.325",              0x2c00000, 0x400000, CRC(f8d2b20c) SHA1(d0c9e413d72772ab8710e217d228e001b28667c8) )
	ROM_LOAD( "fb0b.326",              0x3000000, 0x400000, CRC(ad78ec79) SHA1(7e37a90a64c70f2c0024eaf30e1e4e028c0d858e) )
ROM_END

ROM_START( flamegunj )
	TPS_BIOS

	ROM_REGION32_LE( 0x03400000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "fg_1.119",       0x0000001, 0x100000, CRC(1f2aa527) SHA1(de3a20d9aeb745fe82cd1d87bde26876e088483a) )
	ROM_LOAD16_BYTE( "fg_2.120",       0x0000000, 0x100000, CRC(a2cd4cad) SHA1(bf542eeb6e768b3e86bacdc79ab04be394ce3e63) )
	ROM_LOAD( "fg00.216",              0x0400000, 0x400000, CRC(f33736ca) SHA1(7b18f9fef1df913b7ed3a2c97e9c4925790d86c5) )
	ROM_LOAD( "fg01.217",              0x0800000, 0x400000, CRC(8980ff44) SHA1(f677bf5d279ad7731730b7e36ebf33d554903ce6) )
	ROM_LOAD( "fg02.218",              0x0c00000, 0x400000, CRC(97d1b032) SHA1(1d23cd40ced002ae1fe2fb009d2f31d8612b125a) )
	ROM_LOAD( "fg03.219",              0x1000000, 0x400000, CRC(cf84508c) SHA1(2a1173e3751f5a8b2400219a75f23d2450a0ebd6) )
	ROM_LOAD( "fg04.220",              0x1400000, 0x400000, CRC(5cc333fa) SHA1(fd81e811ef2026b245e65c104d24ae1679baa0f5) )
	ROM_LOAD( "fg05.221",              0x1800000, 0x400000, CRC(9490bc1b) SHA1(c5ea133de0a271793601f2701267d3ca82781f60) )
	ROM_LOAD( "fg06.222",              0x1c00000, 0x400000, CRC(8c29b754) SHA1(304086196321b7d9748456ed1af8965ac6192942) )
	ROM_LOAD( "fg07.223",              0x2000000, 0x400000, CRC(f62cefe8) SHA1(a59a3a41258e8474f0aeb0e3b8c2f73caf47ece4) )
	ROM_LOAD( "fg08.323",              0x2400000, 0x400000, CRC(855959b4) SHA1(994b7c8bd883a41470791802d5a07a98a553096e) )
	ROM_LOAD( "fg09.324",              0x2800000, 0x400000, CRC(82f129b4) SHA1(c0dcbc908c12f7cecbb051a671649edd20bac32c) )
	ROM_LOAD( "fg0a.325",              0x2c00000, 0x400000, CRC(f8d2b20c) SHA1(d0c9e413d72772ab8710e217d228e001b28667c8) )
	ROM_LOAD( "fb0b.326",              0x3000000, 0x400000, CRC(ad78ec79) SHA1(7e37a90a64c70f2c0024eaf30e1e4e028c0d858e) )
ROM_END

ROM_START( lpadv )
	TPS_BIOS

	ROM_REGION32_LE( 0xc00000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "lp_3.u0119",   0x000001, 0x100000, CRC(18cade44) SHA1(8a44156224c77c51f4f6ca61a0168e48dfcc6eda) )
	ROM_LOAD16_BYTE( "lp_4.u0120",   0x000000, 0x100000, CRC(12fffc02) SHA1(3294b65e4a0bbf501785565dd0c1f36f9bcea969) )
	ROM_LOAD( "rp00.u0216",   0x400000, 0x400000, CRC(d759d0d4) SHA1(47b009a5dfa81611276b1376bdab44dfad597e85) )
	ROM_LOAD( "rp01.u0217",   0x800000, 0x400000, CRC(5be576e1) SHA1(e24a96d179016d6d65205079874b35500760a642) )
ROM_END

ROM_START( mfjump )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "mfj-o.119",   0x0000001, 0x100000, CRC(0d724dc5) SHA1(2ba388fe6254c0cf3847fd173a414ee5ca31f4f4) )
	ROM_LOAD16_BYTE( "mfj-e.120",   0x0000000, 0x100000, CRC(86292bca) SHA1(b6a25ab828da3d5c8f6d945336513485708f3f5b) )
	ROM_LOAD( "mfj.216",            0x0400000, 0x400000, CRC(0d518dba) SHA1(100cd4d0a1e678e660336027f067a9a1f5cbad3e) )
ROM_END

ROM_START( tblkkuzu )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "tbk.u119",   0x0000001, 0x100000, CRC(621b07e1) SHA1(30773aaa333fdee7ef55db2f8adde010688abce1) )
	ROM_LOAD16_BYTE( "tbk.u120",   0x0000000, 0x100000, CRC(bb390f7d) SHA1(6bce88448fbb5308952f8c221e786be8aa51ceff) )
	ROM_LOAD( "tbk.u0216",          0x0400000, 0x400000, CRC(41f8285f) SHA1(3326ab83d96d51ed31fb5c2f30630ff480d45282) )
ROM_END

ROM_START( 1on1gov )
	TPS_BIOS

	ROM_REGION32_LE( 0x02800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "1on1.u119",   0x0000001, 0x100000, CRC(10aecc19) SHA1(ad2fe6011551935907568cc3b4028f481034537c) )
	ROM_LOAD16_BYTE( "1on1.u120",   0x0000000, 0x100000, CRC(eea158bd) SHA1(2b2a56fcce46557201bbbab7b170ee64549ddafe) )
	ROM_LOAD( "ooo-0.u0217",  0x0800000, 0x400000, CRC(8b42f365) SHA1(6035a370f477f0f33894f642717fa0b012540d36) )
	ROM_LOAD( "ooo-1.u0218",  0x0c00000, 0x400000, CRC(65162f46) SHA1(db420a2f0d996b32cd4b6e9352d46a36fa31eaaa) )
	ROM_LOAD( "ooo-2.u0219",  0x1000000, 0x400000, CRC(14cf3a84) SHA1(60175a1fb2c765e4c3d0e30e77961f84cfa8485c) )
	ROM_LOAD( "ooo-3.u0220",  0x1400000, 0x400000, CRC(9a45f6ff) SHA1(e0ee90c545c821bf1d6b4709b1e40f93314c51a6) )
	ROM_LOAD( "ooo-4.u0221",  0x1800000, 0x400000, CRC(ba20a1fd) SHA1(7893f50de730624b8447f39fc7e25e4e334df845) )
	ROM_LOAD( "ooo-5.u0222",  0x1c00000, 0x400000, CRC(eed1953d) SHA1(8d3e738a07b9c6b6ca55be7b47444b5e3725065c) )
	ROM_LOAD( "ooo-6.u0223",  0x2000000, 0x400000, CRC(f74f38b6) SHA1(ff7f0ebff85fc982f8d1c13d6649d4c7c5da6c45) )
	ROM_LOAD( "ooo-7.u0323",  0x2400000, 0x400000, CRC(0e58777c) SHA1(9f8ee3c6d6d8b1482522500e18217577056d8c98) )

	ROM_REGION( 0x800, "at28c16", 0 ) /* at28c16 */
	/* Factory defaulted NVRAM, counters blanked, required security code included - region can be changed in test menu (default Japanese) */
	ROM_LOAD( "at28c16",      0x0000000, 0x000800, CRC(fe992f29) SHA1(73c9c4d40abd8f3a95d4eb20f3c65f3a5cdd1203) )
ROM_END

/* video system */

#define KN_BIOS \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_LOAD( "coh-1002v.353", 0x0000000, 0x080000, CRC(5ff165f3) SHA1(8f59314c1093446b9bcb06d232244da6df78e206) ) \
	ROM_REGION( 0x2000, "mcu", 0 ) \
	ROM_LOAD( "upd78081.655", 0x0000, 0x2000, NO_DUMP ) /* internal rom :( */

ROM_START( vspsx )
	KN_BIOS
	ROM_REGION32_LE( 0x0280000, "fixedroms", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x1800000, "bankedroms", ROMREGION_ERASE00 )
ROM_END

/*

There is known to exist (but not dumped) an USA version with hand written labels:

1/8 PROG 0 USA AA9E @ IC5
1/8 PROG 1 USA 0A1E @ IC6
1/8 PROG 2 USA C3B3 @ IC7
1/8 PROG 3 USA E108 @ IC8
1/8 PROG 4 USA 9127 @ IC9

The SUM16 values differ from both the Japanese and Taiwanese versions.

*/

ROM_START( aerofgts )
	KN_BIOS

	ROM_REGION32_LE( 0x0280000, "fixedroms", 0 )
	ROM_LOAD( "vs.ic5",      0x0000000, 0x080000, CRC(453dd514) SHA1(189fe5598485de160f5c0add90235ed63ed92747) )
	ROM_LOAD( "vs.ic6",      0x0080000, 0x080000, CRC(24257295) SHA1(fe6d969c407f2c26c3af2a2a5015dc83b1675e9a) )
	ROM_LOAD( "vs.ic7",      0x0100000, 0x080000, CRC(df5ba2f7) SHA1(19153084e7cff632380b67a2fff800644a2fbf7d) )
	ROM_LOAD( "vs.ic8",      0x0180000, 0x080000, CRC(df638f92) SHA1(5704eed2ef798a91a0398948af0324955ec38534) )
	ROM_LOAD( "vs.ic9",      0x0200000, 0x080000, CRC(1294aa24) SHA1(07ca2cc3f117cfd48d2ca558d9435a3fc238c0fe) )

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD( "ic11.bin",     0x0000000, 0x400000, CRC(a93f6fee) SHA1(6f079643b50833f8fb497c49945ad23326cc9170) )
	ROM_LOAD( "ic12.bin",     0x0400000, 0x400000, CRC(9f584ef7) SHA1(12c04e198f17d1915f58e83aff45ca2e76773df8) )
	ROM_LOAD( "ic13.bin",     0x0800000, 0x400000, CRC(652e9c78) SHA1(a929b2944de72606338acb822c1031463e2b1cc5) )
	ROM_LOAD( "ic14.bin",     0x0c00000, 0x400000, CRC(c4ef1424) SHA1(1734a6ee6d0be94d24afefcf2a125b74747f53d0) )
	ROM_LOAD( "ic15.bin",     0x1000000, 0x400000, CRC(2551d816) SHA1(e1500d4bfa8cc55220c366a5852263ac2070da82) )
	ROM_LOAD( "ic16.bin",     0x1400000, 0x400000, CRC(21b401bc) SHA1(89374b80453c474aa1dd3a219422f557f95a262c) )
ROM_END

ROM_START( sncwgltd )
	KN_BIOS

	ROM_REGION32_LE( 0x0280000, "fixedroms", 0 )
	ROM_LOAD( "ic5.bin",      0x0000000, 0x080000, CRC(458f14aa) SHA1(b4e50be60ffb9b7911561dd35b6a7e0df3432a3a) )
	ROM_LOAD( "ic6.bin",      0x0080000, 0x080000, CRC(8233dd1e) SHA1(1422b4530d671e3b8b471ec16c20ef7c819ab762) )
	ROM_LOAD( "ic7.bin",      0x0100000, 0x080000, CRC(df5ba2f7) SHA1(19153084e7cff632380b67a2fff800644a2fbf7d) )
	ROM_LOAD( "ic8.bin",      0x0180000, 0x080000, CRC(e8145f2b) SHA1(3a1cb189426998856dfeda47267fde64be34c6ec) )
	ROM_LOAD( "ic9.bin",      0x0200000, 0x080000, CRC(605c9370) SHA1(9734549cae3028c089f4c9f2336ee374b3f950f8) )

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD( "ic11.bin",     0x0000000, 0x400000, CRC(a93f6fee) SHA1(6f079643b50833f8fb497c49945ad23326cc9170) )
	ROM_LOAD( "ic12.bin",     0x0400000, 0x400000, CRC(9f584ef7) SHA1(12c04e198f17d1915f58e83aff45ca2e76773df8) )
	ROM_LOAD( "ic13.bin",     0x0800000, 0x400000, CRC(652e9c78) SHA1(a929b2944de72606338acb822c1031463e2b1cc5) )
	ROM_LOAD( "ic14.bin",     0x0c00000, 0x400000, CRC(c4ef1424) SHA1(1734a6ee6d0be94d24afefcf2a125b74747f53d0) )
	ROM_LOAD( "ic15.bin",     0x1000000, 0x400000, CRC(2551d816) SHA1(e1500d4bfa8cc55220c366a5852263ac2070da82) )
	ROM_LOAD( "ic16.bin",     0x1400000, 0x400000, CRC(21b401bc) SHA1(89374b80453c474aa1dd3a219422f557f95a262c) )
ROM_END


/* Taito FX1a/FX1b */

#define TAITOFX1_BIOS \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_LOAD( "coh-1000t.353", 0x0000000, 0x080000, CRC(e3f23b6e) SHA1(e18907cf8c6ba54d96edba0a9a00487a90219e0d) ) \
	ROM_REGION( 0x2000, "mcu", 0 ) \
	ROM_LOAD( "upd78081.655", 0x0000, 0x2000, NO_DUMP ) /* internal rom :( */

ROM_START( taitofx1 )
	TAITOFX1_BIOS
	ROM_REGION32_LE( 0x01000000, "bankedroms", ROMREGION_ERASE00 )
	ROM_REGION( 0x080000, "audiocpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( ftimpcta )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e25-13.4",     0x0000001, 0x100000, CRC(7f078d7b) SHA1(df9800dd6885dbc33736c5143d877b0847221061) )
	ROM_LOAD16_BYTE( "e25-14.3",     0x0000000, 0x100000, CRC(0c5f474f) SHA1(ce7031ba860297b99cddd6d0177f07e03520faeb) )
	ROM_LOAD( "e25-01.1",            0x0400000, 0x400000, CRC(8cc4be0c) SHA1(9ca15558a83b7e332e50accf1f7852444a7ce730) )
	ROM_LOAD( "e25-02.2",            0x0800000, 0x400000, CRC(8e8b4c82) SHA1(55c9d4d3a08fc3226a75ab3a674be433af83e289) )
	ROM_LOAD( "e25-03.12",           0x0c00000, 0x400000, CRC(43b1c085) SHA1(6e53550e9be0d2f415fc6b4f3b8a71185c5370b2) )

	ROM_REGION( 0x080000, "mn10200", 0 )
	ROM_LOAD( "e25-10.14",    0x0000000, 0x080000, CRC(2b2ad1b1) SHA1(6d064d0b6805d43ce42929ac8f5645b56384f53c) )

	ROM_REGION32_LE( 0x600000, "zsg2", 0 )
	ROM_LOAD( "e25-04.27",    0x0000000, 0x400000, CRC(09a66d35) SHA1(f0df24bc9bfc9eb0f5150dc035c19fc5b8a39bf9) )
	ROM_LOAD( "e25-05.28",    0x0400000, 0x200000, CRC(3fb57636) SHA1(aa38bfac11ecf10fd55143cf4525a2a529be8bb6) )
ROM_END

ROM_START( ftimpact )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e25-09.4",     0x0000001, 0x080000, CRC(d457bfc7) SHA1(e974a9c3e7b0748ef89d78e76a7dbb763c42b6f7) )
	ROM_LOAD16_BYTE( "e25-07.3",     0x0000000, 0x080000, CRC(829be1cc) SHA1(64b139d7c3696ab2f0b9a4842c19a38fe6a8cede) )
	ROM_LOAD( "e25-01.1",            0x0400000, 0x400000, CRC(8cc4be0c) SHA1(9ca15558a83b7e332e50accf1f7852444a7ce730) )
	ROM_LOAD( "e25-02.2",            0x0800000, 0x400000, CRC(8e8b4c82) SHA1(55c9d4d3a08fc3226a75ab3a674be433af83e289) )
	ROM_LOAD( "e25-03.12",           0x0c00000, 0x400000, CRC(43b1c085) SHA1(6e53550e9be0d2f415fc6b4f3b8a71185c5370b2) )

	ROM_REGION( 0x080000, "mn10200", 0 )
	ROM_LOAD( "e25-10.14",    0x0000000, 0x080000, CRC(2b2ad1b1) SHA1(6d064d0b6805d43ce42929ac8f5645b56384f53c) )

	ROM_REGION32_LE( 0x600000, "zsg2", 0 )
	ROM_LOAD( "e25-04.27",    0x0000000, 0x400000, CRC(09a66d35) SHA1(f0df24bc9bfc9eb0f5150dc035c19fc5b8a39bf9) )
	ROM_LOAD( "e25-05.28",    0x0400000, 0x200000, CRC(3fb57636) SHA1(aa38bfac11ecf10fd55143cf4525a2a529be8bb6) )
ROM_END

ROM_START( ftimpactu )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e25-08.4",     0x0000001, 0x080000, CRC(a3508f51) SHA1(fd4c3cc186e280497dc905ebda92472d5b72b1b4) )
	ROM_LOAD16_BYTE( "e25-07.3",     0x0000000, 0x080000, CRC(829be1cc) SHA1(64b139d7c3696ab2f0b9a4842c19a38fe6a8cede) )
	ROM_LOAD( "e25-01.1",            0x0400000, 0x400000, CRC(8cc4be0c) SHA1(9ca15558a83b7e332e50accf1f7852444a7ce730) )
	ROM_LOAD( "e25-02.2",            0x0800000, 0x400000, CRC(8e8b4c82) SHA1(55c9d4d3a08fc3226a75ab3a674be433af83e289) )
	ROM_LOAD( "e25-03.12",           0x0c00000, 0x400000, CRC(43b1c085) SHA1(6e53550e9be0d2f415fc6b4f3b8a71185c5370b2) )

	ROM_REGION( 0x080000, "mn10200", 0 )
	ROM_LOAD( "e25-10.14",    0x0000000, 0x080000, CRC(2b2ad1b1) SHA1(6d064d0b6805d43ce42929ac8f5645b56384f53c) )

	ROM_REGION32_LE( 0x600000, "zsg2", 0 )
	ROM_LOAD( "e25-04.27",    0x0000000, 0x400000, CRC(09a66d35) SHA1(f0df24bc9bfc9eb0f5150dc035c19fc5b8a39bf9) )
	ROM_LOAD( "e25-05.28",    0x0400000, 0x200000, CRC(3fb57636) SHA1(aa38bfac11ecf10fd55143cf4525a2a529be8bb6) )
ROM_END

ROM_START( ftimpactj )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e25-06.4",     0x0000001, 0x080000, CRC(3a59deeb) SHA1(4377c5829fb5b6f5d0120caf992b1ee714897641) )
	ROM_LOAD16_BYTE( "e25-07.3",     0x0000000, 0x080000, CRC(829be1cc) SHA1(64b139d7c3696ab2f0b9a4842c19a38fe6a8cede) )
	ROM_LOAD( "e25-01.1",            0x0400000, 0x400000, CRC(8cc4be0c) SHA1(9ca15558a83b7e332e50accf1f7852444a7ce730) )
	ROM_LOAD( "e25-02.2",            0x0800000, 0x400000, CRC(8e8b4c82) SHA1(55c9d4d3a08fc3226a75ab3a674be433af83e289) )
	ROM_LOAD( "e25-03.12",           0x0c00000, 0x400000, CRC(43b1c085) SHA1(6e53550e9be0d2f415fc6b4f3b8a71185c5370b2) )

	ROM_REGION( 0x080000, "mn10200", 0 )
	ROM_LOAD( "e25-10.14",    0x0000000, 0x080000, CRC(2b2ad1b1) SHA1(6d064d0b6805d43ce42929ac8f5645b56384f53c) )

	ROM_REGION32_LE( 0x600000, "zsg2", 0 )
	ROM_LOAD( "e25-04.27",    0x0000000, 0x400000, CRC(09a66d35) SHA1(f0df24bc9bfc9eb0f5150dc035c19fc5b8a39bf9) )
	ROM_LOAD( "e25-05.28",    0x0400000, 0x200000, CRC(3fb57636) SHA1(aa38bfac11ecf10fd55143cf4525a2a529be8bb6) )
ROM_END

ROM_START( gdarius )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e39-06.4",     0x0000001, 0x100000, CRC(2980c30d) SHA1(597321642125c3ae37581c2d9abc2723c7909996) )
	ROM_LOAD16_BYTE( "e39-05.3",     0x0000000, 0x100000, CRC(750e5b13) SHA1(68fe9cbd7d506cfd587dccc40b6ae0b0b6ee7c29) )
	ROM_LOAD( "e39-01.1",            0x0400000, 0x400000, CRC(bdaaa251) SHA1(a42daa706ee859c2b66be179e08c0ad7990f919e) )
	ROM_LOAD( "e39-02.2",            0x0800000, 0x400000, CRC(a47aab5d) SHA1(64b58e47035ad9d8d6dcaf475cbcc3ad85f4d82f) )
	ROM_LOAD( "e39-03.12",           0x0c00000, 0x400000, CRC(a883b6a5) SHA1(b8d00d944c90f8cd9c2b076688f4c68b2e6d557a) )

	ROM_REGION( 0x080000, "mn10200", 0 )
	ROM_LOAD( "e39-07.14",    0x0000000, 0x080000, CRC(2252c7c1) SHA1(92b9908e0d87cad6587f1acc0eef69eaae8c6a98) )

	ROM_REGION32_LE( 0x400000, "zsg2", 0 )
	ROM_LOAD( "e39-04.27",    0x0000000, 0x400000, CRC(6ee35e68) SHA1(fdfe63203d8cecf84cb869039fb893d5b63cdd67) )
ROM_END

ROM_START( gdariusb )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e39-08.ic4",   0x0000001, 0x100000, CRC(835049db) SHA1(2b230c8fd6c6ea4e30740fda28f631344b018b79) )
	ROM_LOAD16_BYTE( "e39-10.ic3",   0x0000000, 0x100000, CRC(6ba4d941) SHA1(75f2d8c920d29102c09e041fc3198e32ad57dbaf) )
	ROM_LOAD( "e39-01.1",            0x0400000, 0x400000, CRC(bdaaa251) SHA1(a42daa706ee859c2b66be179e08c0ad7990f919e) )
	ROM_LOAD( "e39-02.2",            0x0800000, 0x400000, CRC(a47aab5d) SHA1(64b58e47035ad9d8d6dcaf475cbcc3ad85f4d82f) )
	ROM_LOAD( "e39-03.12",           0x0c00000, 0x400000, CRC(a883b6a5) SHA1(b8d00d944c90f8cd9c2b076688f4c68b2e6d557a) )

	ROM_REGION( 0x080000, "mn10200", 0 )
	ROM_LOAD( "e39-07.14",    0x0000000, 0x080000, CRC(2252c7c1) SHA1(92b9908e0d87cad6587f1acc0eef69eaae8c6a98) )

	ROM_REGION32_LE( 0x400000, "zsg2", 0 )
	ROM_LOAD( "e39-04.27",    0x0000000, 0x400000, CRC(6ee35e68) SHA1(fdfe63203d8cecf84cb869039fb893d5b63cdd67) )
ROM_END

ROM_START( gdarius2 )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e39-12.4",     0x0000001, 0x100000, CRC(b23266c3) SHA1(80aaddaaf10e40280ade4c7d11f45ddab47ee9a6) )
	ROM_LOAD16_BYTE( "e39-11.3",     0x0000000, 0x100000, CRC(766f73df) SHA1(9ce24c153920d259bc7fdef0778083eb6d639be3) )
	ROM_LOAD( "e39-01.1",            0x0400000, 0x400000, CRC(bdaaa251) SHA1(a42daa706ee859c2b66be179e08c0ad7990f919e) )
	ROM_LOAD( "e39-02.2",            0x0800000, 0x400000, CRC(a47aab5d) SHA1(64b58e47035ad9d8d6dcaf475cbcc3ad85f4d82f) )
	ROM_LOAD( "e39-03.12",           0x0c00000, 0x400000, CRC(a883b6a5) SHA1(b8d00d944c90f8cd9c2b076688f4c68b2e6d557a) )

	ROM_REGION( 0x080000, "mn10200", 0 )
	ROM_LOAD( "e39-07.14",    0x0000000, 0x080000, CRC(2252c7c1) SHA1(92b9908e0d87cad6587f1acc0eef69eaae8c6a98) )

	ROM_REGION32_LE( 0x400000, "zsg2", 0 )
	ROM_LOAD( "e39-04.27",    0x0000000, 0x400000, CRC(6ee35e68) SHA1(fdfe63203d8cecf84cb869039fb893d5b63cdd67) )
ROM_END

ROM_START( mgcldate )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e32-05.2",     0x0000001, 0x080000, CRC(72fc7f7b) SHA1(50d9e84bc74fb63ec1900ab149051888bc3d03a5) )
	ROM_LOAD16_BYTE( "e32-06.7",     0x0000000, 0x080000, CRC(d11c3881) SHA1(f7046c5bed4818152edcf697a49664b0bcf12a1b) )
	ROM_LOAD( "e32-01.1",            0x0400000, 0x400000, CRC(cf5f1d01) SHA1(5417f8aef5c8d0e9e63ba8c68efb5b3ef37b4693) )
	ROM_LOAD( "e32-02.6",            0x0800000, 0x400000, CRC(61c8438c) SHA1(bdbe6079cc634c0cd6580f76619eb2944c9a31d9) )
	ROM_LOAD( "e32-03.12",           0x0c00000, 0x200000, CRC(190d1618) SHA1(838a651d32752015baa7e8caea62fd739631b8be) )

	ROM_REGION( 0x2c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "e32-07.22",           0x0000000, 0x004000, CRC(adf3feb5) SHA1(bae5bc3fad99a92a3492be1b775dab861007eb3b) )
	ROM_CONTINUE(                    0x0010000, 0x01c000 ) /* banked stuff */

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "e32-04.15",           0x0000000, 0x400000, CRC(c72f9eea) SHA1(7ab8b412a8ed00a42016acb7d13d3b074155780a) )
ROM_END

ROM_START( mgcldtex )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e32-08.2",     0x0000001, 0x100000, CRC(3d42cd28) SHA1(9017922e835a359ba5126c8a9e8c27380a5ce081) )
	ROM_LOAD16_BYTE( "e32-09.7",     0x0000000, 0x100000, CRC(db7ec115) SHA1(fa6f18de71ba997389d887d7ffe745aa25e24c20) )
	ROM_LOAD( "e32-01.1",            0x0400000, 0x400000, CRC(cf5f1d01) SHA1(5417f8aef5c8d0e9e63ba8c68efb5b3ef37b4693) )
	ROM_LOAD( "e32-02.6",            0x0800000, 0x400000, CRC(61c8438c) SHA1(bdbe6079cc634c0cd6580f76619eb2944c9a31d9) )
	ROM_LOAD( "e32-03.12",           0x0c00000, 0x200000, CRC(190d1618) SHA1(838a651d32752015baa7e8caea62fd739631b8be) )

	ROM_REGION( 0x2c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "e32-10.22",           0x0000000, 0x004000, CRC(adf3feb5) SHA1(bae5bc3fad99a92a3492be1b775dab861007eb3b) )
	ROM_CONTINUE(                    0x0010000, 0x01c000 ) /* banked stuff */

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "e32-04.15",           0x0000000, 0x400000, CRC(c72f9eea) SHA1(7ab8b412a8ed00a42016acb7d13d3b074155780a) )
ROM_END

ROM_START( psyforce )
	/* It is VERY ODD that Taito had 2 different labels for the same data (E22-06* & E22-10*) but is verified correct! */
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e22-09+.2",    0x0000001, 0x080000, CRC(997e4500) SHA1(4a90b452c9a877ccec55a11f36c4cbc6df1f1f41) ) /* Labled as E22-09* */
	ROM_LOAD16_BYTE( "e22-06+.7",    0x0000000, 0x080000, CRC(f6341d63) SHA1(99dc27aa694ae5951148054291912a486726e8c9) ) /* Labled as E22-06* */
	ROM_LOAD( "e22-02.16",           0x0800000, 0x200000, CRC(03b50064) SHA1(0259537e86b266b3f34308c4fc0bcc04c037da71) )
	ROM_LOAD( "e22-03.19",           0x0a00000, 0x200000, CRC(8372f839) SHA1(646b3919b6be63412c11850ec1524685abececc0) )
	ROM_LOAD( "e22-04.21",           0x0c00000, 0x200000, CRC(397b71aa) SHA1(48743c362503c1d2dbeb3c8be4cb2aaaae015b88) )

	ROM_REGION( 0x2c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "e22-07.22",           0x0000000, 0x004000, CRC(739af589) SHA1(dbb4d1c6d824a99ccf27168e2c21644e19811523) )
	ROM_CONTINUE(                    0x0010000, 0x01c000 ) /* banked stuff */

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "e22-01.15",           0x000000,  0x200000, CRC(808b8340) SHA1(d8bde850dd9b5b71e94ea707d2d728754f907977) )
ROM_END

ROM_START( psyforcej )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e22-05+.2",    0x0000001, 0x080000, CRC(7770242c) SHA1(dd37575d3d9ffdef60fe0e4cab6c9e42d087f714) ) /* Labled as E22-05* */
	ROM_LOAD16_BYTE( "e22-10+.7",    0x0000000, 0x080000, CRC(f6341d63) SHA1(99dc27aa694ae5951148054291912a486726e8c9) ) /* Labled as E22-10* */
	ROM_LOAD( "e22-02.16",           0x0800000, 0x200000, CRC(03b50064) SHA1(0259537e86b266b3f34308c4fc0bcc04c037da71) )
	ROM_LOAD( "e22-03.19",           0x0a00000, 0x200000, CRC(8372f839) SHA1(646b3919b6be63412c11850ec1524685abececc0) )
	ROM_LOAD( "e22-04.21",           0x0c00000, 0x200000, CRC(397b71aa) SHA1(48743c362503c1d2dbeb3c8be4cb2aaaae015b88) )

	ROM_REGION( 0x2c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "e22-07.22",           0x0000000, 0x004000, CRC(739af589) SHA1(dbb4d1c6d824a99ccf27168e2c21644e19811523) )
	ROM_CONTINUE(                    0x0010000, 0x01c000 ) /* banked stuff */

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "e22-01.15",           0x000000,  0x200000, CRC(808b8340) SHA1(d8bde850dd9b5b71e94ea707d2d728754f907977) )
ROM_END

ROM_START( psyforcex )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e22-11.2",     0x0000001, 0x080000, CRC(a263b41f) SHA1(a797f1eb74a7ba7aeefabd9f5d55e6eec2df46e2) )
	ROM_LOAD16_BYTE( "e22-12.7",     0x0000000, 0x080000, CRC(7426ffc5) SHA1(24b0132241e2e49109e585b082bf4ab67f86b294) )
	ROM_LOAD( "e22-02.16",           0x0800000, 0x200000, CRC(03b50064) SHA1(0259537e86b266b3f34308c4fc0bcc04c037da71) )
	ROM_LOAD( "e22-03.19",           0x0a00000, 0x200000, CRC(8372f839) SHA1(646b3919b6be63412c11850ec1524685abececc0) )
	ROM_LOAD( "e22-04.21",           0x0c00000, 0x200000, CRC(397b71aa) SHA1(48743c362503c1d2dbeb3c8be4cb2aaaae015b88) )

	ROM_REGION( 0x2c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "e22-07.22",           0x0000000, 0x004000, CRC(739af589) SHA1(dbb4d1c6d824a99ccf27168e2c21644e19811523) )
	ROM_CONTINUE(                    0x0010000, 0x01c000 ) /* banked stuff */

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "e22-01.15",           0x000000,  0x200000, CRC(808b8340) SHA1(d8bde850dd9b5b71e94ea707d2d728754f907977) )
ROM_END


ROM_START( raystorm )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e24-xx.ic4", 0x0000001, 0x080000, CRC(33f63638) SHA1(fdda33ffc9902b3605a3272fae5a614e93856a86) ) /* Need to verify actual label */
	ROM_LOAD16_BYTE( "e24-xx.ic3", 0x0000000, 0x080000, CRC(5eeed3b2) SHA1(d8bb1613d7285eabdc6f0a2d231d2eeeb52f307b) ) /* Need to verify actual label */
	ROM_LOAD( "e24-02.1",          0x0400000, 0x400000, CRC(9f70950d) SHA1(b3e4f925a61ae2e5dd4cc5d7ec3030a0d5c2c04d) )
	ROM_LOAD( "e24-03.2",          0x0800000, 0x400000, CRC(6c1f0a5d) SHA1(1aac37a7ff23e54021a4cec18c9bb93242337180) )

	ROM_REGION16_LE( 0x080000, "mn10200", 0 )
	ROM_LOAD( "e24-09.14",    0x0000000, 0x080000, CRC(808589e1) SHA1(46ada4c6d68c2462186a0b962abb435ee740c0ba) )

	ROM_REGION32_LE( 0x400000, "zsg2", 0 )
	ROM_LOAD( "e24-04.27",    0x0000000, 0x400000, CRC(f403493a) SHA1(3e49fd2a060a3893e26f14cc3cf47c4ba91e17d4) )
ROM_END

ROM_START( raystormo )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e24-08.4",     0x0000001, 0x080000, CRC(ae071b95) SHA1(0e1597220808d6e3998ef1e9d88779e0187ba0af) )
	ROM_LOAD16_BYTE( "e24-06.3",     0x0000000, 0x080000, CRC(d70cdf46) SHA1(da6163d69d3ea9c1e3f4b7961a548f1f9d8d9909) )
	ROM_LOAD( "e24-02.1",            0x0400000, 0x400000, CRC(9f70950d) SHA1(b3e4f925a61ae2e5dd4cc5d7ec3030a0d5c2c04d) )
	ROM_LOAD( "e24-03.2",            0x0800000, 0x400000, CRC(6c1f0a5d) SHA1(1aac37a7ff23e54021a4cec18c9bb93242337180) )

	ROM_REGION( 0x080000, "mn10200", 0 )
	ROM_LOAD( "e24-09.14",    0x0000000, 0x080000, CRC(808589e1) SHA1(46ada4c6d68c2462186a0b962abb435ee740c0ba) )

	ROM_REGION32_LE( 0x400000, "zsg2", 0 )
	ROM_LOAD( "e24-04.27",    0x0000000, 0x400000, CRC(f403493a) SHA1(3e49fd2a060a3893e26f14cc3cf47c4ba91e17d4) )
ROM_END

ROM_START( raystormu )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e24-07.4",     0x0000001, 0x080000, CRC(d9002b03) SHA1(bdb0aa88536c4c98c150ece87387930b3dbdd258) )
	ROM_LOAD16_BYTE( "e24-06.3",     0x0000000, 0x080000, CRC(d70cdf46) SHA1(da6163d69d3ea9c1e3f4b7961a548f1f9d8d9909) )
	ROM_LOAD( "e24-02.1",            0x0400000, 0x400000, CRC(9f70950d) SHA1(b3e4f925a61ae2e5dd4cc5d7ec3030a0d5c2c04d) )
	ROM_LOAD( "e24-03.2",            0x0800000, 0x400000, CRC(6c1f0a5d) SHA1(1aac37a7ff23e54021a4cec18c9bb93242337180) )

	ROM_REGION( 0x080000, "mn10200", 0 )
	ROM_LOAD( "e24-09.14",    0x0000000, 0x080000, CRC(808589e1) SHA1(46ada4c6d68c2462186a0b962abb435ee740c0ba) )

	ROM_REGION32_LE( 0x400000, "zsg2", 0 )
	ROM_LOAD( "e24-04.27",    0x0000000, 0x400000, CRC(f403493a) SHA1(3e49fd2a060a3893e26f14cc3cf47c4ba91e17d4) )
ROM_END

ROM_START( raystormj )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e24-05.4",     0x0000001, 0x080000, CRC(40097ab9) SHA1(67e73568b35515c2c5a9119e97ac4709baff8c5a) )
	ROM_LOAD16_BYTE( "e24-06.3",     0x0000000, 0x080000, CRC(d70cdf46) SHA1(da6163d69d3ea9c1e3f4b7961a548f1f9d8d9909) )
	ROM_LOAD( "e24-02.1",            0x0400000, 0x400000, CRC(9f70950d) SHA1(b3e4f925a61ae2e5dd4cc5d7ec3030a0d5c2c04d) )
	ROM_LOAD( "e24-03.2",            0x0800000, 0x400000, CRC(6c1f0a5d) SHA1(1aac37a7ff23e54021a4cec18c9bb93242337180) )

	ROM_REGION( 0x080000, "mn10200", 0 )
	ROM_LOAD( "e24-09.14",    0x0000000, 0x080000, CRC(808589e1) SHA1(46ada4c6d68c2462186a0b962abb435ee740c0ba) )

	ROM_REGION32_LE( 0x400000, "zsg2", 0 )
	ROM_LOAD( "e24-04.27",    0x0000000, 0x400000, CRC(f403493a) SHA1(3e49fd2a060a3893e26f14cc3cf47c4ba91e17d4) )
ROM_END

ROM_START( sfchamp )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e18-12.2",     0x0000001, 0x080000, CRC(72304685) SHA1(2e6f645871e19a49fcdfbdca49c6be415471eadf) ) /* Ver 2.5O */
	ROM_LOAD16_BYTE( "e18-13.7",     0x0000000, 0x080000, CRC(fa4d01ee) SHA1(27efd8e2107d71213d35f2a58762ed8812f809d3) ) /* Ver 2.5O */
	ROM_LOAD( "e18-02.12",           0x0600000, 0x200000, CRC(c7b4fe29) SHA1(7f823bd61abf2b15d3ba62bca829a5b1acacfd09) )
	ROM_LOAD( "e18-03.16",           0x0800000, 0x200000, CRC(76392346) SHA1(2c5b70c4708208f866feea0472fcc72333061124) )
	ROM_LOAD( "e18-04.19",           0x0a00000, 0x200000, CRC(fc3731da) SHA1(58948aad8d7bb7a8449d2bf12e9d5e6d7b4426b5) )
	ROM_LOAD( "e18-05.21",           0x0c00000, 0x200000, CRC(2e984c50) SHA1(6d8255e38c67d68bf489c9885663ed2edf148188) )

	ROM_REGION( 0x2c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "e18-09.22",           0x0000000, 0x004000, CRC(bb5a5319) SHA1(0bb700cafc157d3af663cc9bebb8167487ff2852) )
	ROM_CONTINUE(                    0x0010000, 0x01c000 ) /* banked stuff */

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "e18-01.15",           0x0000000, 0x200000, CRC(dbd1408c) SHA1(ef81064f2f95e5ae25eb1f10d1e78f27f9e294f5) )
ROM_END

ROM_START( sfchampo )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e18-11.2",     0x0000001, 0x080000, CRC(f5462f30) SHA1(44eb03a9b51e2d8dd14fe2ed36dbcf17035a22c7) ) /* Ver 2.4O */
	ROM_LOAD16_BYTE( "e18-08.7",     0x0000000, 0x080000, CRC(6a5558cd) SHA1(75b26bcaaa213283e7e0dace69ee58f305b4572d) ) /* Ver 2.4O */
	ROM_LOAD( "e18-02.12",           0x0600000, 0x200000, CRC(c7b4fe29) SHA1(7f823bd61abf2b15d3ba62bca829a5b1acacfd09) )
	ROM_LOAD( "e18-03.16",           0x0800000, 0x200000, CRC(76392346) SHA1(2c5b70c4708208f866feea0472fcc72333061124) )
	ROM_LOAD( "e18-04.19",           0x0a00000, 0x200000, CRC(fc3731da) SHA1(58948aad8d7bb7a8449d2bf12e9d5e6d7b4426b5) )
	ROM_LOAD( "e18-05.21",           0x0c00000, 0x200000, CRC(2e984c50) SHA1(6d8255e38c67d68bf489c9885663ed2edf148188) )

	ROM_REGION( 0x2c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "e18-09.22",           0x0000000, 0x004000, CRC(bb5a5319) SHA1(0bb700cafc157d3af663cc9bebb8167487ff2852) )
	ROM_CONTINUE(                    0x0010000, 0x01c000 ) /* banked stuff */

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "e18-01.15",           0x0000000, 0x200000, CRC(dbd1408c) SHA1(ef81064f2f95e5ae25eb1f10d1e78f27f9e294f5) )
ROM_END

ROM_START( sfchampu )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e18-10.2",     0x0000001, 0x080000, CRC(82411fa6) SHA1(0aa1764b7ff68258ef76a41355c50d5067262d75) )
	ROM_LOAD16_BYTE( "e18-08.7",     0x0000000, 0x080000, CRC(6a5558cd) SHA1(75b26bcaaa213283e7e0dace69ee58f305b4572d) )
	ROM_LOAD( "e18-02.12",           0x0600000, 0x200000, CRC(c7b4fe29) SHA1(7f823bd61abf2b15d3ba62bca829a5b1acacfd09) )
	ROM_LOAD( "e18-03.16",           0x0800000, 0x200000, CRC(76392346) SHA1(2c5b70c4708208f866feea0472fcc72333061124) )
	ROM_LOAD( "e18-04.19",           0x0a00000, 0x200000, CRC(fc3731da) SHA1(58948aad8d7bb7a8449d2bf12e9d5e6d7b4426b5) )
	ROM_LOAD( "e18-05.21",           0x0c00000, 0x200000, CRC(2e984c50) SHA1(6d8255e38c67d68bf489c9885663ed2edf148188) )

	ROM_REGION( 0x2c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "e18-09.22",           0x0000000, 0x004000, CRC(bb5a5319) SHA1(0bb700cafc157d3af663cc9bebb8167487ff2852) )
	ROM_CONTINUE(                    0x0010000, 0x01c000 ) /* banked stuff */

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "e18-01.15",           0x0000000, 0x200000, CRC(dbd1408c) SHA1(ef81064f2f95e5ae25eb1f10d1e78f27f9e294f5) )
ROM_END

ROM_START( sfchampj )
	TAITOFX1_BIOS

	ROM_REGION32_LE( 0x01000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "e18-07.2",     0x0000001, 0x080000, CRC(1b484e1c) SHA1(f29f40a9988475d8abbb126095b0716133c087a0) )
	ROM_LOAD16_BYTE( "e18-08.7",     0x0000000, 0x080000, CRC(6a5558cd) SHA1(75b26bcaaa213283e7e0dace69ee58f305b4572d) )
	ROM_LOAD( "e18-02.12",           0x0600000, 0x200000, CRC(c7b4fe29) SHA1(7f823bd61abf2b15d3ba62bca829a5b1acacfd09) )
	ROM_LOAD( "e18-03.16",           0x0800000, 0x200000, CRC(76392346) SHA1(2c5b70c4708208f866feea0472fcc72333061124) )
	ROM_LOAD( "e18-04.19",           0x0a00000, 0x200000, CRC(fc3731da) SHA1(58948aad8d7bb7a8449d2bf12e9d5e6d7b4426b5) )
	ROM_LOAD( "e18-05.21",           0x0c00000, 0x200000, CRC(2e984c50) SHA1(6d8255e38c67d68bf489c9885663ed2edf148188) )

	ROM_REGION( 0x2c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "e18-09.22",           0x0000000, 0x004000, CRC(bb5a5319) SHA1(0bb700cafc157d3af663cc9bebb8167487ff2852) )
	ROM_CONTINUE(                    0x0010000, 0x01c000 ) /* banked stuff */

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "e18-01.15",           0x0000000, 0x200000, CRC(dbd1408c) SHA1(ef81064f2f95e5ae25eb1f10d1e78f27f9e294f5) )
ROM_END

/* Eighting/Raizing */

#define PSARC95_BIOS \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_LOAD( "coh-1002e.353", 0x000000, 0x080000, CRC(910f3a8b) SHA1(cd68532967a25f476a6d73473ec6b6f4df2e1689) ) \
	ROM_REGION( 0x2000, "mcu", 0 ) \
	ROM_LOAD( "upd78081.655", 0x0000, 0x2000, NO_DUMP ) /* internal rom :( */

ROM_START( psarc95 )
	PSARC95_BIOS
	ROM_REGION32_LE( 0x1800000, "bankedroms", ROMREGION_ERASE00 )
	ROM_REGION( 0x080000, "audiocpu", ROMREGION_ERASE00 )
	ROM_REGION( 0x400000, "ymf", ROMREGION_ERASE00 )
ROM_END

ROM_START( beastrzr )
	PSARC95_BIOS

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "b.roar_u0213",   0x000001, 0x080000, CRC(2c586534) SHA1(a38dfc3a45446d24a1caac89b0f560989d46ded5) ) /* For U0212 & U0213, 8ing used indentical rom labels */
	ROM_LOAD16_BYTE( "b.roar_u0212",   0x000000, 0x080000, CRC(1c85d7fb) SHA1(aa406a42c424cc16a9e5330c68dda9acf8760088) ) /* even though the content changes between versions   */
	ROM_LOAD16_BYTE( "b.roar-u0215",   0x100001, 0x080000, CRC(31c8e055) SHA1(2811789ab6221b972d1e3ffe98916587990f7564) )
	ROM_LOAD16_BYTE( "b.roar-u0214",   0x100000, 0x080000, CRC(1cdc450a) SHA1(9215e5fec52f7c5c0070feb621eb9c77f98e2362) )
	ROM_LOAD( "ra-b-roar_rom-1.u0217", 0x400000, 0x400000, CRC(11f1ba36) SHA1(d41ae686c2c607640cbadf906215c89134758050) )
	ROM_LOAD( "ra-b.roar_rom-2.u0216", 0x800000, 0x400000, CRC(d46d46b7) SHA1(1c42cb5dcda4b26c08c4ecf95efeadaf3a1d1dd2) )

	ROM_REGION( 0x080000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "b.roar-u046",  0x000001, 0x040000, CRC(d4bb261a) SHA1(9a295b1354ef15f37ea09bb209cf0cb98437c462) )
	ROM_LOAD16_BYTE( "b.roar-u042",  0x000000, 0x040000, CRC(4d537f88) SHA1(1760367d70a81606e29885ea315185d2c2a9409b) )

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "ra-b.roar3_rom-3.u0326", 0x000000, 0x400000, CRC(b74cc4d1) SHA1(eb5485582a12959ae06927a2f1d8a7e63e0f956f) )
ROM_END

ROM_START( bldyroar )
	PSARC95_BIOS

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "b.roar-u0213",   0x000001, 0x080000, CRC(63769342) SHA1(7231188073b997b039467db85ce7c85383daf591) ) /* For U0212 & U0213, 8ing used indentical rom labels */
	ROM_LOAD16_BYTE( "b.roar-u0212",   0x000000, 0x080000, CRC(966b7169) SHA1(63e025cacb84e89d30b40ed6cfa5c63d84c298c4) ) /* even though the content changes between versions   */
	ROM_LOAD16_BYTE( "b.roar-u0215",   0x100001, 0x080000, CRC(31c8e055) SHA1(2811789ab6221b972d1e3ffe98916587990f7564) )
	ROM_LOAD16_BYTE( "b.roar-u0214",   0x100000, 0x080000, CRC(1cdc450a) SHA1(9215e5fec52f7c5c0070feb621eb9c77f98e2362) )
	ROM_LOAD( "ra-b-roar_rom-1.u0217", 0x400000, 0x400000, CRC(11f1ba36) SHA1(d41ae686c2c607640cbadf906215c89134758050) )
	ROM_LOAD( "ra-b.roar_rom-2.u0216", 0x800000, 0x400000, CRC(d46d46b7) SHA1(1c42cb5dcda4b26c08c4ecf95efeadaf3a1d1dd2) )

	ROM_REGION( 0x080000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "b.roar-u046",  0x000001, 0x040000, CRC(d4bb261a) SHA1(9a295b1354ef15f37ea09bb209cf0cb98437c462) )
	ROM_LOAD16_BYTE( "b.roar-u042",  0x000000, 0x040000, CRC(4d537f88) SHA1(1760367d70a81606e29885ea315185d2c2a9409b) )

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "ra-b.roar3_rom-3.u0326", 0x000000, 0x400000, CRC(b74cc4d1) SHA1(eb5485582a12959ae06927a2f1d8a7e63e0f956f) )
ROM_END

ROM_START( beastrzrb ) /* bootleg board */
	PSARC95_BIOS

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD( "27c160.1",     0x000000, 0x200000, CRC(820855e2) SHA1(18bdd4d0b4a92ae4fde457e1f37c813be6eece71) )
	ROM_LOAD( "27c160.4",     0x400000, 0x200000, CRC(2d2b25f4) SHA1(77d8ad94602e71f16b47de47bc2e0a97957c530b) )
	ROM_LOAD( "27c160.5",     0x600000, 0x200000, CRC(10fe6f4d) SHA1(9faee2faa6d741e1caf25edd093644be5723aa5c) )
	ROM_LOAD( "27c160.2",     0x800000, 0x200000, CRC(1712af34) SHA1(3a78997a2ad0fec1b09828b47150a4be611cd9ad) )
	ROM_LOAD( "27c800.3",     0xb00000, 0x100000, CRC(7192eb4e) SHA1(bb276a38261099d91080d8613dc7500322f6fcab) )

	ROM_REGION( 0x080000, "audiocpu", 0 )
/*  http://www.atmel.com/dyn/products/product_card.asp?family_id=604&family_name=8051+Architecture&part_id=1939 */
	ROM_LOAD( "at89c4051",    0x000000, 0x001000, NO_DUMP ) /* cat 702 replacement or sound cpu? */

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "27c4096.1",    0x000000, 0x080000, CRC(217734a1) SHA1(de4f519215123c09b3b5f27509b4d74604b5e03d) )
	ROM_LOAD( "27c4096.2",    0x080000, 0x080000, CRC(d1f2a9b2) SHA1(d1475a453ce4e3b9f2ff59abedf0f57ba3c408fe) )
	ROM_LOAD( "27c240.3",     0x100000, 0x080000, CRC(509cdc8b) SHA1(8b92b79be09de56e7d40c2d02fcbeca92bb60226) ) /* bad dump? (only contains 8k of data) */
ROM_END

ROM_START( brvblade )
	TPS_BIOS

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD( "flash0.021",      0x0000000, 0x200000, CRC(97e12c63) SHA1(382970617a363f6c98ee741f26be6a75c9752bdb) )
	ROM_LOAD( "flash1.024",      0x0200000, 0x200000, CRC(d9d40a34) SHA1(c91dbc6f85404e9397fa79a4bac28e8c3c1a5228) )
	ROM_LOAD( "ra-bbl_rom1.028", 0x0800000, 0x400000, CRC(418535e0) SHA1(7c443e651704f2cd552565c35f4a93f2dc250558) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "spu1u412.bin", 0x0000001, 0x080000, CRC(6408b5b2) SHA1(ba60aa1074df87e98fa260211e9ec99cea25023f) )
	ROM_LOAD16_BYTE( "spu0u049.bin", 0x0000000, 0x080000, CRC(c9df8ed9) SHA1(00a58522189091c48d781b6703e4378e04343c33) )

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "ra-bbl_rom2.336", 0x000000, 0x400000, CRC(cd052c02) SHA1(d955a70a89b3b1a0b505a05c0887c399fe7a2c68) )

	ROM_REGION( 0x800, "at28c16", 0 ) /* at28c16 */
	ROM_LOAD( "at28c16_world",      0x000, 0x800, CRC(fe7f7d34) SHA1(18f6ae14e57afa668b3eef821b4cf0a7599a21ac) ) /* preprogrammed mainboard flash for region */
ROM_END

ROM_START( brvbladeu )
	TPS_BIOS

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD( "flash0.021",      0x0000000, 0x200000, CRC(97e12c63) SHA1(382970617a363f6c98ee741f26be6a75c9752bdb) )
	ROM_LOAD( "flash1.024",      0x0200000, 0x200000, CRC(d9d40a34) SHA1(c91dbc6f85404e9397fa79a4bac28e8c3c1a5228) )
	ROM_LOAD( "ra-bbl_rom1.028", 0x0800000, 0x400000, CRC(418535e0) SHA1(7c443e651704f2cd552565c35f4a93f2dc250558) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "spu1u412.bin", 0x0000001, 0x080000, CRC(6408b5b2) SHA1(ba60aa1074df87e98fa260211e9ec99cea25023f) )
	ROM_LOAD16_BYTE( "spu0u049.bin", 0x0000000, 0x080000, CRC(c9df8ed9) SHA1(00a58522189091c48d781b6703e4378e04343c33) )

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "ra-bbl_rom2.336", 0x000000, 0x400000, CRC(cd052c02) SHA1(d955a70a89b3b1a0b505a05c0887c399fe7a2c68) )

	ROM_REGION( 0x800, "at28c16", 0 ) /* at28c16 */
	ROM_LOAD( "at28c16_usa",      0x000, 0x800, CRC(0a2c042f) SHA1(147651d2e55873a82295214b3b0bd6c46cdad239) ) /* preprogrammed mainboard flash for region */
ROM_END

ROM_START( brvbladej )
	TPS_BIOS

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD( "flash0.021",      0x0000000, 0x200000, CRC(97e12c63) SHA1(382970617a363f6c98ee741f26be6a75c9752bdb) )
	ROM_LOAD( "flash1.024",      0x0200000, 0x200000, CRC(d9d40a34) SHA1(c91dbc6f85404e9397fa79a4bac28e8c3c1a5228) )
	ROM_LOAD( "ra-bbl_rom1.028", 0x0800000, 0x400000, CRC(418535e0) SHA1(7c443e651704f2cd552565c35f4a93f2dc250558) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "spu1u412.bin", 0x0000001, 0x080000, CRC(6408b5b2) SHA1(ba60aa1074df87e98fa260211e9ec99cea25023f) )
	ROM_LOAD16_BYTE( "spu0u049.bin", 0x0000000, 0x080000, CRC(c9df8ed9) SHA1(00a58522189091c48d781b6703e4378e04343c33) )

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "ra-bbl_rom2.336", 0x000000, 0x400000, CRC(cd052c02) SHA1(d955a70a89b3b1a0b505a05c0887c399fe7a2c68) )

	ROM_REGION( 0x800, "at28c16", 0 ) /* at28c16 */
	ROM_LOAD( "at28c16_japan",      0x000, 0x800, CRC(59e2d326) SHA1(53f48b6ad7243aa92b54863515f104a7a54b2810) ) /* preprogrammed mainboard flash for region */
ROM_END

ROM_START( brvbladea )
	TPS_BIOS

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD( "flash0.021",      0x0000000, 0x200000, CRC(97e12c63) SHA1(382970617a363f6c98ee741f26be6a75c9752bdb) )
	ROM_LOAD( "flash1.024",      0x0200000, 0x200000, CRC(d9d40a34) SHA1(c91dbc6f85404e9397fa79a4bac28e8c3c1a5228) )
	ROM_LOAD( "ra-bbl_rom1.028", 0x0800000, 0x400000, CRC(418535e0) SHA1(7c443e651704f2cd552565c35f4a93f2dc250558) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "spu1u412.bin", 0x0000001, 0x080000, CRC(6408b5b2) SHA1(ba60aa1074df87e98fa260211e9ec99cea25023f) )
	ROM_LOAD16_BYTE( "spu0u049.bin", 0x0000000, 0x080000, CRC(c9df8ed9) SHA1(00a58522189091c48d781b6703e4378e04343c33) )

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "ra-bbl_rom2.336", 0x000000, 0x400000, CRC(cd052c02) SHA1(d955a70a89b3b1a0b505a05c0887c399fe7a2c68) )

	ROM_REGION( 0x800, "at28c16", 0 ) /* at28c16 */
	ROM_LOAD( "at28c16_asia",      0x000, 0x800, CRC(adb1aa3d) SHA1(118a17b8a15108666e4f9f2d1798031b3b893536) ) /* preprogrammed mainboard flash for region */
ROM_END


ROM_START( bldyror2 )
	PSARC95_BIOS

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD( "flash0.021",      0x0000000, 0x200000, CRC(fa7602e1) SHA1(6fb6af09656fbb86d2abda35804b2ed4a4cd7461) )
	ROM_LOAD( "flash1.024",      0x0200000, 0x200000, CRC(03465a69) SHA1(7c29aff2bf19c379873d3927c260892c78281882) )
	ROM_LOAD( "rom-1a.028",      0x0800000, 0x400000, CRC(0e711461) SHA1(1d0bd80e6885432ef0623babde28e5760b714bfa) )
	ROM_LOAD( "rom-1b.29",       0x0c00000, 0x400000, CRC(0cf153f9) SHA1(53bb9f8642079f56d8e925792b069362df666819) )
	ROM_LOAD( "rom-2a.026",      0x1000000, 0x400000, CRC(b71d955d) SHA1(49fce452c70ceafc8a149fa9ff073589b7261882) )
	ROM_LOAD( "rom-2b.210",      0x1400000, 0x400000, CRC(89959dde) SHA1(99d54b9876f38f5e625334bbd1439618cdf01d56) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "br2_u0412.412", 0x000001, 0x080000, CRC(e254dd8a) SHA1(5b8fcafcf2176e0b55efcf37799d7c0d97e01bdc) )
	ROM_LOAD16_BYTE( "br2_u049.049",  0x000000, 0x080000, CRC(10dc855b) SHA1(4e6e3a71911c8976ae07c2b6cac5a36f98193def) )

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "rom-3.336",       0x000000, 0x400000, CRC(b74cc4d1) SHA1(eb5485582a12959ae06927a2f1d8a7e63e0f956f) )

	ROM_REGION( 0x800, "at28c16", 0 ) /* at28c16 */
	ROM_LOAD( "at28c16_world",      0x0000000, 0x000800, CRC(01b42397) SHA1(853553a38e81e64a17c040173b29c7bfd6f79f31) ) /* preprogrammed mainboard flash for region */
ROM_END

ROM_START( bldyror2u )
	PSARC95_BIOS

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD( "flash0.021",      0x0000000, 0x200000, CRC(fa7602e1) SHA1(6fb6af09656fbb86d2abda35804b2ed4a4cd7461) )
	ROM_LOAD( "flash1.024",      0x0200000, 0x200000, CRC(03465a69) SHA1(7c29aff2bf19c379873d3927c260892c78281882) )
	ROM_LOAD( "rom-1a.028",      0x0800000, 0x400000, CRC(0e711461) SHA1(1d0bd80e6885432ef0623babde28e5760b714bfa) )
	ROM_LOAD( "rom-1b.29",       0x0c00000, 0x400000, CRC(0cf153f9) SHA1(53bb9f8642079f56d8e925792b069362df666819) )
	ROM_LOAD( "rom-2a.026",      0x1000000, 0x400000, CRC(b71d955d) SHA1(49fce452c70ceafc8a149fa9ff073589b7261882) )
	ROM_LOAD( "rom-2b.210",      0x1400000, 0x400000, CRC(89959dde) SHA1(99d54b9876f38f5e625334bbd1439618cdf01d56) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "br2_u0412.412", 0x000001, 0x080000, CRC(e254dd8a) SHA1(5b8fcafcf2176e0b55efcf37799d7c0d97e01bdc) )
	ROM_LOAD16_BYTE( "br2_u049.049",  0x000000, 0x080000, CRC(10dc855b) SHA1(4e6e3a71911c8976ae07c2b6cac5a36f98193def) )

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "rom-3.336",       0x000000, 0x400000, CRC(b74cc4d1) SHA1(eb5485582a12959ae06927a2f1d8a7e63e0f956f) )

	ROM_REGION( 0x800, "at28c16", 0 ) /* at28c16 */
	ROM_LOAD( "at28c16_usa",      0x0000000, 0x000800, CRC(b78d6fc3) SHA1(49d8b6f44c31d74f36cba981af7f4c7e23dd9007) ) /* preprogrammed mainboard flash for region */
ROM_END

ROM_START( bldyror2j )
	PSARC95_BIOS

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD( "flash0.021",      0x0000000, 0x200000, CRC(fa7602e1) SHA1(6fb6af09656fbb86d2abda35804b2ed4a4cd7461) )
	ROM_LOAD( "flash1.024",      0x0200000, 0x200000, CRC(03465a69) SHA1(7c29aff2bf19c379873d3927c260892c78281882) )
	ROM_LOAD( "rom-1a.028",      0x0800000, 0x400000, CRC(0e711461) SHA1(1d0bd80e6885432ef0623babde28e5760b714bfa) )
	ROM_LOAD( "rom-1b.29",       0x0c00000, 0x400000, CRC(0cf153f9) SHA1(53bb9f8642079f56d8e925792b069362df666819) )
	ROM_LOAD( "rom-2a.026",      0x1000000, 0x400000, CRC(b71d955d) SHA1(49fce452c70ceafc8a149fa9ff073589b7261882) )
	ROM_LOAD( "rom-2b.210",      0x1400000, 0x400000, CRC(89959dde) SHA1(99d54b9876f38f5e625334bbd1439618cdf01d56) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "br2_u0412.412", 0x000001, 0x080000, CRC(e254dd8a) SHA1(5b8fcafcf2176e0b55efcf37799d7c0d97e01bdc) )
	ROM_LOAD16_BYTE( "br2_u049.049",  0x000000, 0x080000, CRC(10dc855b) SHA1(4e6e3a71911c8976ae07c2b6cac5a36f98193def) )

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "rom-3.336",       0x000000, 0x400000, CRC(b74cc4d1) SHA1(eb5485582a12959ae06927a2f1d8a7e63e0f956f) )

	ROM_REGION( 0x800, "at28c16", 0 ) /* at28c16 */
	ROM_LOAD( "at28c16_japan",      0x0000000, 0x000800, CRC(6cb55630) SHA1(b840bc0339485dd82f7c9aa669faf90ae371218f) ) /* preprogrammed mainboard flash for region */
ROM_END

ROM_START( bldyror2a )
	PSARC95_BIOS

	ROM_REGION32_LE( 0x1800000, "bankedroms", 0 )
	ROM_LOAD( "flash0.021",      0x0000000, 0x200000, CRC(fa7602e1) SHA1(6fb6af09656fbb86d2abda35804b2ed4a4cd7461) )
	ROM_LOAD( "flash1.024",      0x0200000, 0x200000, CRC(03465a69) SHA1(7c29aff2bf19c379873d3927c260892c78281882) )
	ROM_LOAD( "rom-1a.028",      0x0800000, 0x400000, CRC(0e711461) SHA1(1d0bd80e6885432ef0623babde28e5760b714bfa) )
	ROM_LOAD( "rom-1b.29",       0x0c00000, 0x400000, CRC(0cf153f9) SHA1(53bb9f8642079f56d8e925792b069362df666819) )
	ROM_LOAD( "rom-2a.026",      0x1000000, 0x400000, CRC(b71d955d) SHA1(49fce452c70ceafc8a149fa9ff073589b7261882) )
	ROM_LOAD( "rom-2b.210",      0x1400000, 0x400000, CRC(89959dde) SHA1(99d54b9876f38f5e625334bbd1439618cdf01d56) )

	ROM_REGION( 0x100000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "br2_u0412.412", 0x000001, 0x080000, CRC(e254dd8a) SHA1(5b8fcafcf2176e0b55efcf37799d7c0d97e01bdc) )
	ROM_LOAD16_BYTE( "br2_u049.049",  0x000000, 0x080000, CRC(10dc855b) SHA1(4e6e3a71911c8976ae07c2b6cac5a36f98193def) )

	ROM_REGION( 0x400000, "ymf", 0 )
	ROM_LOAD( "rom-3.336",       0x000000, 0x400000, CRC(b74cc4d1) SHA1(eb5485582a12959ae06927a2f1d8a7e63e0f956f) )

	ROM_REGION( 0x800, "at28c16", 0 ) /* at28c16 */
	ROM_LOAD( "at28c16_asia",      0x0000000, 0x000800, CRC(da8c1a64) SHA1(14cbb751f498c96d9d8fce3eea3781ebc45f6291) ) /* preprogrammed mainboard flash for region */
ROM_END

ROM_START( bam2 )
	PSARC95_BIOS

	ROM_REGION32_LE( 0x2c00000, "bankedroms", 0 )
	ROM_LOAD( "u19",             0x0000000, 0x200000, CRC(4d9f2337) SHA1(b156fd461d9d5141c60dbcd9ecd26b4f277b7919) )
	ROM_LOAD( "u20",             0x0200000, 0x200000, CRC(1efb3c55) SHA1(d86e21a10fbcbcc759ba78b200dc2a10cb945b4c) )
	ROM_LOAD( "mtr-bam-a01.u23", 0x0400000, 0x400000, CRC(5ed9e2dd) SHA1(85ac746735ec2fd89cd9082a3ab4ac6b4d9e8f4a) )
	ROM_LOAD( "mtr-bam-a02.u24", 0x0800000, 0x400000, CRC(be335265) SHA1(7e09a166fe6d0e9e96c99fd472afb4db023ad217) )
	ROM_LOAD( "mtr-bam-a03.u25", 0x0c00000, 0x400000, CRC(bf71791b) SHA1(b3eb791770838fc74e3535340610164166b63af8) )
	ROM_LOAD( "mtr-bam-a04.u26", 0x1000000, 0x400000, CRC(d3aa62b5) SHA1(958b34fa2fa21c25f34972d4c288ef46e088d6e3) )
	ROM_LOAD( "mtr-bam-a05.u27", 0x1400000, 0x400000, CRC(bd94d0ae) SHA1(97fe7b25768be2f57d8e823ec445c0ee92f07c02) )
	ROM_LOAD( "mtr-bam-a06.u28", 0x1800000, 0x400000, CRC(b972c0b4) SHA1(e5ef170d0e71b7e02463462e1ea31c21ae890d14) )
	ROM_LOAD( "mtr-bam-a07.u29", 0x1c00000, 0x400000, CRC(e8f716c1) SHA1(b15aafb0c9f3484a7ee41b5e6728af08d6a7bd8b) )
	ROM_LOAD( "mtr-bam-a08.u30", 0x2000000, 0x400000, CRC(6e691ff1) SHA1(3fdcf3403e9ffd99b98e789930fc805dc2bc7692) )
	ROM_LOAD( "mtr-bam-a09.u31", 0x2400000, 0x400000, CRC(e4bd7cec) SHA1(794d10b15a22aeed89082f4db2f3cb94aa7d807d) )
	ROM_LOAD( "mtr-bam-a10.u32", 0x2800000, 0x400000, CRC(37fd1fa0) SHA1(afe846a817e499c405a5fd4ad83094270640faf3) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE("bam2", 0, SHA1(634d9a745a82c567fc4d7ce48e3570d88326c5f9) )
ROM_END

/* Atari PSX */

#define TW_BIOS \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_LOAD( "coh-1000w.353", 0x000000, 0x080000, CRC(45e8a4b4) SHA1(815488d8563c85f97fbc3384ff21f08e4c88b7b7) ) \
	ROM_REGION( 0x2000, "mcu", 0 ) \
	ROM_LOAD( "upd78081.655", 0x0000, 0x2000, NO_DUMP ) /* internal rom :( */

ROM_START( atpsx )
	TW_BIOS
	ROM_REGION32_LE( 0x200000, "roms", ROMREGION_ERASE00 )
ROM_END

ROM_START( primrag2 )
	TW_BIOS

	ROM_REGION32_LE( 0x200000, "roms", 0 )
	ROM_LOAD16_BYTE( "pr2_036.u16",  0x000001, 0x080000, CRC(3ee39e4f) SHA1(dbd859b54fb9be33effc14eb847dcd829024eea3) )
	ROM_LOAD16_BYTE( "pr2_036.u14",  0x000000, 0x080000, CRC(c86450cd) SHA1(19c3c50d839a9efb6ffa9ada8a072f56697c1abb) )
	ROM_LOAD16_BYTE( "pr2_036.u17",  0x100001, 0x080000, CRC(3681516c) SHA1(714f73ea4ac190c36a6eb2308616a4aecabc4e69) )
	ROM_LOAD16_BYTE( "pr2_036.u15",  0x100000, 0x080000, CRC(4b24bd54) SHA1(7f27cd524d10e5869aab6d4dc6a4217d049c475d) )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "primrag2", 0, SHA1(bc615068ddf4fd967f770ee01c02f285c052c4c5) )
ROM_END

/* Acclaim PSX */

#define AC_BIOS \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_LOAD( "coh-1000a.353", 0x0000000, 0x080000, CRC(8d8d0764) SHA1(7ee83d317190bb1cef2f8f01c81eaaae47150ebb) ) \
	ROM_REGION( 0x2000, "mcu", 0 ) \
	ROM_LOAD( "upd78081.655", 0x0000, 0x2000, NO_DUMP ) /* internal rom :( */

ROM_START( acpsx )
	AC_BIOS
	ROM_REGION32_LE( 0x2000000, "roms", ROMREGION_ERASE00 )
ROM_END

ROM_START( nbajamex )
	AC_BIOS

	ROM_REGION32_LE( 0x2000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "360mpa1o.u36", 0x0000001, 0x100000, CRC(c433e827) SHA1(1d2a5a6990a1b1864e63ce3ba7306d48ebbd4775) )
	ROM_LOAD16_BYTE( "360mpa1e.u35", 0x0000000, 0x100000, CRC(d8f5b2f7) SHA1(e38609d314721b8b612e047406e2888395917b0d) )
	ROM_LOAD16_BYTE( "nbax0o.u28",   0x0200001, 0x200000, CRC(be13c5af) SHA1(eee5c9d985384ecfe4f00fae27d66fbefc15b28e) )
	ROM_LOAD16_BYTE( "nbax0e.u41",   0x0200000, 0x200000, CRC(077f4355) SHA1(63c52bb82943b52bb0906d114acd5ea8643068b6) )
	ROM_LOAD16_BYTE( "nbax1o.u29",   0x0600001, 0x200000, CRC(3650e85b) SHA1(a36bfa235c8e3bb516e178f54d3c5e3955c7e918) )
	ROM_LOAD16_BYTE( "nbax1e.u42",   0x0600000, 0x200000, CRC(f1212cf9) SHA1(b2f80af3ec4d559056e86f695d89d1d32b500f50) )
	ROM_LOAD16_BYTE( "nbax2o.u30",   0x0a00001, 0x200000, CRC(ccbb6125) SHA1(998eda99182b984f88f5fc58095cb35bf232a26b) )
	ROM_LOAD16_BYTE( "nbax2e.u43",   0x0a00000, 0x200000, CRC(c20ab628) SHA1(7ffe5005e1913da56770452ae2f907a4a270ab24) )
	ROM_LOAD16_BYTE( "nbax3o.u31",   0x0e00001, 0x200000, CRC(d5238edf) SHA1(d1eb30ec65dd6cfa8cbb2b36af3a83820d1de99a) )
	ROM_LOAD16_BYTE( "nbax3e.u44",   0x0e00000, 0x200000, CRC(07ba00a3) SHA1(c14bffd35ee715b07d6065b454b0443438ab6536) )
	ROM_LOAD16_BYTE( "nbax4o.u3",    0x1200001, 0x200000, CRC(1cf16a34) SHA1(a7e984a2db846854f1c4a9a2fdefd0d17ce3108c) )
	ROM_LOAD16_BYTE( "nbax4e.u17",   0x1200000, 0x200000, CRC(b5977765) SHA1(08acdfe413a5a9182ca117f44b7acac9dac9ecea) )
	ROM_LOAD16_BYTE( "nbax5o.u4",    0x1600001, 0x200000, CRC(5272754b) SHA1(c35ba5377eb812991e4bf0d954a34af90b986341) )
	ROM_LOAD16_BYTE( "nbax5e.u18",   0x1600000, 0x200000, CRC(0eb917da) SHA1(d6c8991ba7cd492668757658ee64078d0e82b596) )
	ROM_LOAD16_BYTE( "nbax6o.u5",    0x1a00001, 0x200000, CRC(b1dfb42e) SHA1(fb9627e228bf2a744842eb44afbca4a6232cadb2) )
	ROM_LOAD16_BYTE( "nbax6e.u19",   0x1a00000, 0x200000, CRC(6f17d8c1) SHA1(22cf263efb64cf62030e02b641c485debe75944d) )

	ROM_REGION( 0x0a0000, "cpu1", 0 ) /* 512k for the audio CPU, ADSP-2181 (+banks) */
	ROM_LOAD( "360snda1.u52", 0x000000, 0x08000, CRC(36d8a628) SHA1(944a01c9128f5e90c7dba3557a3ecb2c5ca90831) )
	ROM_CONTINUE(             0x010000, 0x78000 )

	ROM_REGION( 0x400000, "unknown", 0 )
	ROM_LOAD( "sound0.u48",   0x000000, 0x200000, CRC(38873b67) SHA1(b2f8d32270ae604c099a1b9b71d2e06468c7d4a9) )
	ROM_LOAD( "sound1.u49",   0x200000, 0x200000, CRC(57014589) SHA1(d360ff1c52424bd91a5a8d1a2a9c10bf7abb0602) )
ROM_END

ROM_START( jdredd )
	AC_BIOS

	ROM_REGION32_LE( 0x200000, "roms", 0 )
	ROM_LOAD16_BYTE( "j-dread.u36",  0x000001, 0x020000, CRC(37addbf9) SHA1(a4061a1ba9e230f080f0bfea69bf77efe9264a92) )
	ROM_LOAD16_BYTE( "j-dread.u35",  0x000000, 0x020000, CRC(c1e17191) SHA1(82901439b1a51b9aadb4df4b9d944f26697a1460) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "jdreddc", 0, SHA1(eee205f83e5f590f8baf36452c873d7063156bd0) )
ROM_END

ROM_START( jdreddb )
	AC_BIOS

	ROM_REGION32_LE( 0x200000, "roms", 0 )
	ROM_LOAD16_BYTE( "j-dread.u36",  0x000001, 0x020000, CRC(37addbf9) SHA1(a4061a1ba9e230f080f0bfea69bf77efe9264a92) )
	ROM_LOAD16_BYTE( "j-dread.u35",  0x000000, 0x020000, CRC(c1e17191) SHA1(82901439b1a51b9aadb4df4b9d944f26697a1460) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "jdreddb", 0, SHA1(20f696fa6e1fbf97793bac2a794631c5dd4fb39a) )
ROM_END

/* Atlus */

#define ATLUS_BIOS \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_LOAD( "coh-1001l.353", 0x000000, 0x080000, CRC(6721146b) SHA1(9511d24bfe25eb180fb2db0835b131cb4a12730e) ) \
	ROM_REGION( 0x2000, "mcu", 0 ) \
	ROM_LOAD( "upd78081.655", 0x0000, 0x2000, NO_DUMP ) /* internal rom :( */

ROM_START( atluspsx )
	ATLUS_BIOS
	ROM_REGION32_LE( 0x02000000, "bankedroms", ROMREGION_ERASE00 )
	ROM_REGION( 0x040000, "audiocpu", ROMREGION_ERASE00 )
ROM_END

ROM_START( hvnsgate )
	ATLUS_BIOS

	ROM_REGION32_LE( 0x02000000, "bankedroms", 0 )
	ROM_LOAD16_BYTE( "athg-01b.18",  0x0000001, 0x080000, CRC(e820136f) SHA1(2bc3465928dd08060736a2a67d98864d634275d6) )
	ROM_LOAD16_BYTE( "athg-02b.17",  0x0000000, 0x080000, CRC(11bfa89b) SHA1(f23e4c9d8eb90bd3bb3327d9950edd7a467ce8da) )
	ROM_LOAD( "athg-07.027",         0x0100000, 0x400000, CRC(46411f67) SHA1(2e8f37c3d9d7f5f3c79fca8ffeaf4c2fd1634b91) )
	ROM_LOAD( "athg-08.028",         0x0500000, 0x400000, CRC(85289345) SHA1(6385fe27451b80f97e7bad823b3b59eff3efa541) )
	ROM_LOAD( "athg-09.210",         0x0900000, 0x400000, CRC(19e558b5) SHA1(c195bc7dc3cfe4f099d27afdebd6f9cfe064e1df) )
	ROM_LOAD( "athg-10.029",         0x0d00000, 0x400000, CRC(748f936e) SHA1(134e78ea71bb9646f36cc503c704496a2b622ee9) )
	ROM_LOAD( "athg-11.215",         0x1100000, 0x200000, CRC(ac8e53bd) SHA1(002c4be1aa57d810c5d810c475631d9f14e1d2ab) )

	ROM_REGION( 0x040000, "audiocpu", 0 ) /* 68000 code, 10.000MHz */
	ROM_LOAD16_BYTE( "athg-04.21",   0x000001, 0x020000, CRC(18523e85) SHA1(0ecc2116760f05fca8e5366b0a97dfe26fa9bc0c) )
	ROM_LOAD16_BYTE( "athg-03.22",   0x000000, 0x020000, CRC(7eef7e68) SHA1(65b8ae18ef4ff636c548326a360b481aeb316869) )

	ROM_REGION( 0x800000, "ymz", ROMREGION_ERASE00 ) /* YMZ280B Sound Samples */
	ROM_LOAD( "athg-05.4136", 0x400000, 0x200000, CRC(74469a15) SHA1(0faa883900d7fd2e5240f486db33b3d868f1f05f) )
	ROM_LOAD( "athg-06.4134", 0x600000, 0x200000, CRC(443ade73) SHA1(6ef6aa68c525b9749833125dcab929d1d65d3b90) )
ROM_END

/* Capcom ZN1 */

/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the coh-1000c.353 file, so that we do not have to include */
/* it in every zip file */
GAME( 1995, cpzn1,    0,         coh1000c, zn,   driver_device, 0, ROT0, "Capcom", "ZN1", MACHINE_IS_BIOS_ROOT )

GAME( 1995, ts2,       cpzn1,    coh1000c, zn6b, driver_device, 0, ROT0, "Capcom / Takara", "Battle Arena Toshinden 2 (USA 951124)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1995, ts2a,      ts2,      coh1000c, zn6b, driver_device, 0, ROT0, "Capcom / Takara", "Battle Arena Toshinden 2 (USA 951124) Older", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1995, ts2j,      ts2,      coh1000c, zn6b, driver_device, 0, ROT0, "Capcom / Takara", "Battle Arena Toshinden 2 (Japan 951124)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, starglad,  cpzn1,    coh1000c, zn6b, driver_device, 0, ROT0, "Capcom", "Star Gladiator Episode I: Final Crusade (USA 960627)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, stargladj, starglad, coh1000c, zn6b, driver_device, 0, ROT0, "Capcom", "Star Gladiator Episode I: Final Crusade (Japan 960627)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, glpracr,   cpzn1,    glpracr,  zn,   driver_device, 0, ROT0, "Tecmo", "Gallop Racer (English Ver 10.17.K)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, glpracrj,  glpracr,  glpracr,  zn,   driver_device, 0, ROT0, "Tecmo", "Gallop Racer (Japanese Ver 9.01.12)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, sfex,      cpzn1,    coh1002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX (Euro 961219)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, sfexu,     sfex,     coh1002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX (USA 961219)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, sfexa,     sfex,     coh1002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX (Asia 961219)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, sfexj,     sfex,     coh1002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX (Japan 961130)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, sfexp,     cpzn1,    coh1002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX Plus (USA 970407)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, sfexpu1,   sfexp,    coh1002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX Plus (USA 970311)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, sfexpj,    sfexp,    coh1002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX Plus (Japan 970407)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, sfexpj1,   sfexp,    coh1002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX Plus (Japan 970311)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

/* Capcom ZN2 */

/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the coh-3002c.353 file, so that we do not have to include */
/* it in every zip file */
GAME( 1997, cpzn2,    0,         coh3002c, zn,   driver_device, 0, ROT0, "Capcom", "ZN2", MACHINE_IS_BIOS_ROOT )

GAME( 1997, rvschool,  cpzn2,    coh3002c, zn6b, driver_device, 0, ROT0, "Capcom", "Rival Schools: United By Fate (Euro 971117)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, rvschoolu, rvschool, coh3002c, zn6b, driver_device, 0, ROT0, "Capcom", "Rival Schools: United By Fate (USA 971117)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, rvschoola, rvschool, coh3002c, zn6b, driver_device, 0, ROT0, "Capcom", "Rival Schools: United By Fate (Asia 971117)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, jgakuen,   rvschool, coh3002c, zn6b, driver_device, 0, ROT0, "Capcom", "Shiritsu Justice Gakuen: Legion of Heroes (Japan 971117)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, sfex2,     cpzn2,    coh3002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX2 (USA 980526)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, sfex2a,    sfex2,    coh3002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX2 (Asia 980312)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, sfex2h,    sfex2,    coh3002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX2 (Hispanic 980312)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, sfex2j,    sfex2,    coh3002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX2 (Japan 980312)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, plsmaswd,  cpzn2,    coh3002c, zn6b, driver_device, 0, ROT0, "Capcom", "Plasma Sword: Nightmare of Bilstein (USA 980316)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, plsmaswda, plsmaswd, coh3002c, zn6b, driver_device, 0, ROT0, "Capcom", "Plasma Sword: Nightmare of Bilstein (Asia 980316)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, stargld2,  plsmaswd, coh3002c, zn6b, driver_device, 0, ROT0, "Capcom", "Star Gladiator 2: Nightmare of Bilstein (Japan 980316)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, tgmj,      cpzn2,    coh3002c, zn4w, driver_device, 0, ROT0, "Arika / Capcom", "Tetris The Grand Master (Japan 980710)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, techromn,  cpzn2,    coh3002c, zn6b, driver_device, 0, ROT0, "Capcom", "Tech Romancer (Euro 980914)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, techromnu, techromn, coh3002c, zn6b, driver_device, 0, ROT0, "Capcom", "Tech Romancer (USA 980914)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, kikaioh,   techromn, coh3002c, zn6b, driver_device, 0, ROT0, "Capcom", "Choukou Senki Kikaioh (Japan 980914)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, sfex2p,    cpzn2,    coh3002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX2 Plus (USA 990611)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, sfex2pa,   sfex2p,   coh3002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX2 Plus (Asia 990611)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, sfex2ph,   sfex2p,   coh3002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX2 Plus (Hispanic 990611)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, sfex2pj,   sfex2p,   coh3002c, zn6b, driver_device, 0, ROT0, "Capcom / Arika", "Street Fighter EX2 Plus (Japan 990611)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, strider2,  cpzn2,    coh3002c, zn,   driver_device, 0, ROT0, "Capcom", "Strider 2 (USA 991213)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING ) // random hangs / crashes
GAME( 1999, strider2a, strider2, coh3002c, zn,   driver_device, 0, ROT0, "Capcom", "Strider 2 (Asia 991213)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, shiryu2,   strider2, coh3002c, zn,   driver_device, 0, ROT0, "Capcom", "Strider Hiryu 2 (Japan 991213)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING  )

/* Atari */

/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the coh-1000w.353 file, so that we do not have to include */
/* it in every zip file */
GAME( 1996, atpsx,    0,        coh1000w, zn,       driver_device, 0, ROT0, "Atari", "Atari PSX", MACHINE_IS_BIOS_ROOT )

GAME( 1996, primrag2, atpsx,    coh1000w, primrag2, driver_device, 0, ROT0, "Atari", "Primal Rage 2 (Ver 0.36a)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

/* Acclaim */

/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the coh-1000a.353 file, so that we do not have to include */
/* it in every zip file */
GAME( 1995, acpsx,    0,        coh1000a, zn,     driver_device, 0, ROT0, "Acclaim", "Acclaim PSX", MACHINE_IS_BIOS_ROOT )

GAME( 1996, nbajamex, acpsx,    nbajamex, zn,     driver_device, 0, ROT0, "Acclaim", "NBA Jam Extreme", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, jdredd,   acpsx,    jdredd,   jdredd, driver_device, 0, ROT0, "Acclaim", "Judge Dredd (Rev C Dec. 17 1997)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, jdreddb,  jdredd,   jdredd,   jdredd, driver_device, 0, ROT0, "Acclaim", "Judge Dredd (Rev B Nov. 26 1997)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

/* Tecmo */

/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the coh-1002m.353 file, so that we do not have to include */
/* it in every zip file */
GAME( 1997, tps,      0,         coh1002m,    zn, driver_device, 0, ROT0, "Tecmo", "TPS", MACHINE_IS_BIOS_ROOT )

GAME( 1997, glpracr2,  tps,      coh1002m,    zn, driver_device, 0, ROT0, "Tecmo", "Gallop Racer 2 (USA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1997, glpracr2j, glpracr2, coh1002m,    zn, driver_device, 0, ROT0, "Tecmo", "Gallop Racer 2 (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1997, glpracr2l, glpracr2, coh1002ml,   zn, driver_device, 0, ROT0, "Tecmo", "Gallop Racer 2 Link HW (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1998, doapp,     tps,      coh1002m,    zn, driver_device, 0, ROT0, "Tecmo", "Dead Or Alive ++ (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, cbaj,      tps,      coh1002msnd, zn, driver_device, 0, ROT0, "UEP Systems", "Cool Boarders Arcade Jam", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, shngmtkb,  tps,      coh1002m,    zn, driver_device, 0, ROT0, "Sunsoft / Activision", "Shanghai Matekibuyuu", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, tondemo,   tps,      coh1002m,    zn, driver_device, 0, ROT0, "Tecmo", "Tondemo Crisis (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, glpracr3,  tps,      coh1002m,    zn, driver_device, 0, ROT0, "Tecmo", "Gallop Racer 3 (Export)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, glpracr3j, glpracr3, coh1002m,    zn, driver_device, 0, ROT0, "Tecmo", "Gallop Racer 3 (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, flamegun,  tps,      coh1002m,    zn, driver_device, 0, ROT0, "Gaps Inc.", "Flame Gunner", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, flamegunj, flamegun, coh1002m,    zn, driver_device, 0, ROT0, "Gaps Inc.", "Flame Gunner (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1999, lpadv,     tps,      coh1002m,    zn, driver_device, 0, ROT0, "Amuse World", "Logic Pro Adventure (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 2000, tblkkuzu,  tps,      coh1002m,    zn, driver_device, 0, ROT0, "Tamsoft / D3 Publisher", "The Block Kuzushi (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 2000, 1on1gov,   tps,      coh1002m,    zn, driver_device, 0, ROT0, "Tecmo", "1 on 1 Government (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 2000, tecmowcm,  tps,      coh1002m,    zn, driver_device, 0, ROT0, "Tecmo", "Tecmo World Cup Millennium (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 2001, mfjump,    tps,      coh1002m,    zn, driver_device, 0, ROT0, "Tecmo", "Monster Farm Jump (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

/* Video System */

/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the coh-1002v.353 file, so that we do not have to include */
/* it in every zip file */
GAME( 1996, vspsx,    0,        coh1002v, zn, driver_device, 0, ROT0,   "Video System Co.", "Video System PSX", MACHINE_IS_BIOS_ROOT )

GAME( 1996, aerofgts, vspsx,    coh1002v, zn, driver_device, 0, ROT270, "Video System Co.", "Aero Fighters Special (Taiwan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, sncwgltd, aerofgts, coh1002v, zn, driver_device, 0, ROT270, "Video System Co.", "Sonic Wings Limited (Japan)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

/* Taito */

/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the coh-1000t.353 file, so that we do not have to include */
/* it in every zip file */
GAME( 1995, taitofx1, 0,         coh1000ta, zn, driver_device, 0, ROT0, "Taito", "Taito FX1", MACHINE_IS_BIOS_ROOT )

GAME( 1995, sfchamp,   taitofx1, coh1000ta, zn, driver_device, 0, ROT0, "Taito", "Super Football Champ (Ver 2.5O)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1995, sfchampo,  sfchamp,  coh1000ta, zn, driver_device, 0, ROT0, "Taito", "Super Football Champ (Ver 2.4O)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1995, sfchampu,  sfchamp,  coh1000ta, zn, driver_device, 0, ROT0, "Taito", "Super Football Champ (Ver 2.4A)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1995, sfchampj,  sfchamp,  coh1000ta, zn, driver_device, 0, ROT0, "Taito", "Super Football Champ (Ver 2.4J)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1995, psyforce,  taitofx1, coh1000ta, zn, driver_device, 0, ROT0, "Taito", "Psychic Force (Ver 2.4O)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1995, psyforcej, psyforce, coh1000ta, zn, driver_device, 0, ROT0, "Taito", "Psychic Force (Ver 2.4J)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1995, psyforcex, psyforce, coh1000ta, zn, driver_device, 0, ROT0, "Taito", "Psychic Force EX (Ver 2.0J)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, mgcldate,  mgcldtex, coh1000ta, zn, driver_device, 0, ROT0, "Taito", "Magical Date / Magical Date - dokidoki kokuhaku daisakusen (Ver 2.02J)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, mgcldtex,  taitofx1, coh1000ta, zn, driver_device, 0, ROT0, "Taito", "Magical Date EX / Magical Date - sotsugyou kokuhaku daisakusen (Ver 2.01J)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

GAME( 1996, raystorm,  taitofx1, coh1000tb, zn, zn_state, coh1000tb, ROT0, "Taito", "Ray Storm (Ver 2.06A)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, raystormo, raystorm, coh1000tb, zn, zn_state, coh1000tb, ROT0, "Taito", "Ray Storm (Ver 2.05O)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, raystormu, raystorm, coh1000tb, zn, zn_state, coh1000tb, ROT0, "Taito", "Ray Storm (Ver 2.05A)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, raystormj, raystorm, coh1000tb, zn, zn_state, coh1000tb, ROT0, "Taito", "Ray Storm (Ver 2.05J)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, ftimpact,  ftimpcta, coh1000tb, zn, zn_state, coh1000tb, ROT0, "Taito", "Fighters' Impact (Ver 2.02O)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, ftimpactu, ftimpcta, coh1000tb, zn, zn_state, coh1000tb, ROT0, "Taito", "Fighters' Impact (Ver 2.02A)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1996, ftimpactj, ftimpcta, coh1000tb, zn, zn_state, coh1000tb, ROT0, "Taito", "Fighters' Impact (Ver 2.02J)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, ftimpcta,  taitofx1, coh1000tb, zn, zn_state, coh1000tb, ROT0, "Taito", "Fighters' Impact A (Ver 2.00J)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, gdarius,   gdarius2, coh1002tb, zn, zn_state, coh1000tb, ROT0, "Taito", "G-Darius (Ver 2.01J)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, gdariusb,  gdarius2, coh1002tb, zn, zn_state, coh1000tb, ROT0, "Taito", "G-Darius (Ver 2.02A)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, gdarius2,  taitofx1, coh1002tb, zn, zn_state, coh1000tb, ROT0, "Taito", "G-Darius Ver.2 (Ver 2.03J)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

/* Eighting / Raizing */

/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the coh-1002e.353 file, so that we do not have to include */
/* it in every zip file */
GAME( 1997, psarc95,  0,         coh1002e, zn,   driver_device, 0, ROT0, "Eighting / Raizing", "PS Arcade 95", MACHINE_IS_BIOS_ROOT )

GAME( 1997, beastrzr,  psarc95,  coh1002e, zn,   driver_device, 0, ROT0, "Eighting / Raizing", "Beastorizer (USA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, bldyroar,  beastrzr, coh1002e, zn,   driver_device, 0, ROT0, "Eighting / Raizing", "Bloody Roar (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, beastrzrb, beastrzr, coh1002e, zn,   driver_device, 0, ROT0, "bootleg", "Beastorizer (USA bootleg)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

/* The region on these is determined from the NVRAM, it can't be changed from the test menu, it's pre-programmed */
GAME( 1998, bldyror2,  psarc95,  coh1002e, zn6b, driver_device, 0, ROT0, "Eighting / Raizing", "Bloody Roar 2 (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, bldyror2u, bldyror2, coh1002e, zn6b, driver_device, 0, ROT0, "Eighting / Raizing", "Bloody Roar 2 (USA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, bldyror2a, bldyror2, coh1002e, zn6b, driver_device, 0, ROT0, "Eighting / Raizing", "Bloody Roar 2 (Asia)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1998, bldyror2j, bldyror2, coh1002e, zn6b, driver_device, 0, ROT0, "Eighting / Raizing", "Bloody Roar 2 (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

/* The region on these is determined from the NVRAM, it can't be changed from the test menu, it's pre-programmed */
GAME( 2000, brvblade,  tps,      coh1002e, zn,   driver_device, 0, ROT270, "Eighting / Raizing", "Brave Blade (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 2000, brvbladeu, brvblade, coh1002e, zn,   driver_device, 0, ROT270, "Eighting / Raizing", "Brave Blade (USA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 2000, brvbladea, brvblade, coh1002e, zn,   driver_device, 0, ROT270, "Eighting / Raizing", "Brave Blade (Asia)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 2000, brvbladej, brvblade, coh1002e, zn,   driver_device, 0, ROT270, "Eighting / Raizing", "Brave Blade (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

/* Bust a Move 2 uses the PSARC95 bios and ET series security but the top board is completely different */
GAME( 1999, bam2,     psarc95,  bam2,     zn,    driver_device, 0, ROT0, "Metro / Enix / Namco", "Bust a Move 2 (Japanese ROM ver. 1999/07/17 10:00:00)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

/* Atlus */

/* A dummy driver, so that the bios can be debugged, and to serve as */
/* parent for the coh-1002l.353 file, so that we do not have to include */
/* it in every zip file */
GAME( 1996, atluspsx,  0,       coh1001l, zn, driver_device, 0, ROT0, "Atlus", "Atlus PSX", MACHINE_IS_BIOS_ROOT )

GAME( 1996, hvnsgate, atluspsx, coh1001l, zn, driver_device, 0, ROT0, "Racdym / Atlus", "Heaven's Gate", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
