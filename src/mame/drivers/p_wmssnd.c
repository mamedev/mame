/************************************************************************************************
*************************************************************************************************
 Old PinMAME code below for reference ONLY
*************************************************************************************************
************************************************************************************************/

#if 0

#include "driver.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "cpu/adsp2100/adsp2100.h"
#include "machine/6821pia.h"
#include "sound/2151intf.h"
#include "sound/hc55516.h"
#include "sound/dac.h"
#include "core.h"
#include "sndbrd.h"
#include "s11.h"
#include "wpc.h"
#include "wmssnd.h"

//This awful hack is here to prevent the bug where the speech pitch is too low on pre-dcs games when
//the YM2151 is not outputing music. In the hardware the YM2151's Timer A is set to control the FIRQ of the sound cpu 6809.
//The 6809 will output CVSD speech data based on the speed of the FIRQ. The faster the speed, the higher the
//pitch. For some reason, when the YM2151 is not outputting sound, the FIRQ rate goes down.. Def. some kind of
//MAME core bug with timing, but I can't find it. I really hope someone can fix this hack someday..SJE 09/17/03
#define PREDCS_FIRQ_HACK

/*----------------------
/    System 3 - 7
/-----------------------*/
#define S67S_PIA0 6

/* sound board interface */
static void s67s_init(struct sndbrdData *brdData);
static WRITE_HANDLER(s67s_cmd_w);
static void s67s_diag(int button);

const struct sndbrdIntf s67sIntf = {
  "WMSS67", s67s_init, NULL, s67s_diag, s67s_cmd_w, s67s_cmd_w, NULL, NULL, NULL, SNDBRD_NOCTRLSYNC
};

/* machine interface */
static MEMORY_READ_START(s67s_readmem )
  { 0x0000, 0x007f, MRA_RAM },
  { 0x0400, 0x0403, pia_r(S67S_PIA0) },
  { 0x3000, 0x7fff, MRA_ROM },
  { 0x8400, 0x8403, pia_r(S67S_PIA0) },
  { 0xb000, 0xffff, MRA_ROM },
MEMORY_END

static MEMORY_WRITE_START(s67s_writemem )
  { 0x0000, 0x007f, MWA_RAM },
  { 0x0400, 0x0403, pia_w(S67S_PIA0) },
  { 0x8400, 0x8403, pia_w(S67S_PIA0) },
MEMORY_END
static struct DACinterface      s67s_dacInt     = { 1, { 50 }};
static struct hc55516_interface s67s_hc55516Int = { 1, { 100 }};

MACHINE_DRIVER_START(wmssnd_s67s)
  MDRV_CPU_ADD(M6808, 3579000/4)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(s67s_readmem, s67s_writemem)
  MDRV_INTERLEAVE(50)
  MDRV_SOUND_ADD(DAC, s67s_dacInt)
  MDRV_SOUND_ADD(HC55516, s67s_hc55516Int)
  MDRV_SOUND_ADD(SAMPLES, samples_interface)
MACHINE_DRIVER_END

static void s67s_piaIrq(int state);
static const struct pia6821_interface s67s_pia = {
 /* PIA0  (0400)
    PA0-7  DAC
    PB0-7  Sound input
    CB1    Sound input != 0x1f
    CB2    Speech clock
    CA2    Speech data
    CA1    NC */
 /* in  : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
 /* out : A/B,CA/B2       */ DAC_0_data_w, 0, hc55516_0_digit_w, hc55516_0_clock_w,
 /* irq : A/B             */ s67s_piaIrq, s67s_piaIrq
};

static struct {
  struct sndbrdData brdData;
} s67slocals;

static void s67s_init(struct sndbrdData *brdData) {
  s67slocals.brdData = *brdData;
  pia_config(S67S_PIA0, PIA_STANDARD_ORDERING, &s67s_pia);
}

static WRITE_HANDLER(s67s_cmd_w) {
  if (s67slocals.brdData.subType) { // don't use sound dips
	data &= 0x7f;
  } else {
    data = (data & 0x1f) | (core_getDip(0)<<5);
  }
  pia_set_input_b(S67S_PIA0, data);
  if (s67slocals.brdData.subType) {
    pia_set_input_cb1(S67S_PIA0, !((data & 0x7f) == 0x7f));
  } else {
    pia_set_input_cb1(S67S_PIA0, !((data & 0x1f) == 0x1f));
  }
}
static void s67s_diag(int button) {
  cpu_set_nmi_line(s67slocals.brdData.cpuNo, button ? ASSERT_LINE : CLEAR_LINE);
}

static void s67s_piaIrq(int state) {
  cpu_set_irq_line(s67slocals.brdData.cpuNo, M6808_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

/*----------------------------
/  S11 CPU board sound
/-----------------------------*/
#define S11S_PIA0  6
#define S11S_BANK0 1
#define S11S_BANK1 2

/* sound board interface */
static void s11s_init(struct sndbrdData *brdData);
static void s11s_diag(int button);
static WRITE_HANDLER(s11s_manCmd_w);
static WRITE_HANDLER(s11s_bankSelect);
const struct sndbrdIntf s11sIntf = {
  "WMSS11", s11s_init, NULL, s11s_diag, s11s_manCmd_w, soundlatch_w, NULL, CAT3(pia_,S11S_PIA0,_ca1_w), NULL
};
/* machine interface */
static MEMORY_READ_START(s9s_readmem)
  { 0x0000, 0x0fff, MRA_RAM},
  { 0x2000, 0x2003, pia_r(S11S_PIA0)},
  { 0x8000, 0xffff, MRA_ROM}, /* U22 */
MEMORY_END
static MEMORY_WRITE_START(s9s_writemem)
  { 0x0000, 0x0fff, MWA_RAM },
  { 0x2000, 0x2003, pia_w(S11S_PIA0)},
MEMORY_END

static struct DACinterface      s9s_dacInt     = { 1, { 50 }};
static struct hc55516_interface s9s_hc55516Int = { 1, { 100 }};

MACHINE_DRIVER_START(wmssnd_s9s)
  MDRV_CPU_ADD(M6808, 1000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(s9s_readmem, s9s_writemem)
  MDRV_INTERLEAVE(50)
  MDRV_SOUND_ADD(DAC,    s9s_dacInt)
  MDRV_SOUND_ADD(HC55516,s9s_hc55516Int)
  MDRV_SOUND_ADD(SAMPLES, samples_interface)
MACHINE_DRIVER_END

static MEMORY_READ_START(s11s_readmem)
  { 0x0000, 0x0fff, MRA_RAM},
  { 0x2000, 0x2003, pia_r(S11S_PIA0) },
  { 0x8000, 0xbfff, MRA_BANKNO(S11S_BANK0)}, /* U22 */
  { 0xc000, 0xffff, MRA_BANKNO(S11S_BANK1)}, /* U21 */
MEMORY_END
static MEMORY_WRITE_START(s11s_writemem)
  { 0x0000, 0x0fff, MWA_RAM },
  { 0x1000, 0x1000, s11s_bankSelect},
  { 0x2000, 0x2003, pia_w(S11S_PIA0)},
MEMORY_END

static void s11cs_ym2151IRQ(int state);
static struct DACinterface      s11xs_dacInt2     = { 2, { 50,50 }};
static struct hc55516_interface s11b2s_hc55516Int = { 1, { 80 }};
static struct hc55516_interface s11xs_hc55516Int2 = { 2, { 80,80 }};
static struct YM2151interface   s11cs_ym2151Int  = {
  1, 3579545, /* Hz */
  { YM3012_VOL(10,MIXER_PAN_CENTER,30,MIXER_PAN_CENTER) },
  { s11cs_ym2151IRQ }
};

MACHINE_DRIVER_START(wmssnd_s11s)
  MDRV_CPU_ADD(M6808, 1000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(s11s_readmem, s11s_writemem)
  MDRV_INTERLEAVE(50)
  MDRV_SOUND_ADD(DAC,    s9s_dacInt)
  MDRV_SOUND_ADD(HC55516,s9s_hc55516Int)
  MDRV_SOUND_ADD(SAMPLES, samples_interface)
MACHINE_DRIVER_END

MACHINE_DRIVER_START(wmssnd_s11xs)
  MDRV_IMPORT_FROM(wmssnd_s11cs)
  MDRV_CPU_ADD(M6808, 1000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(s11s_readmem, s11s_writemem)
  MDRV_INTERLEAVE(50)
  MDRV_SOUND_REPLACE("dac",  DAC,    s11xs_dacInt2)
  MDRV_SOUND_REPLACE("cvsd", HC55516,s11xs_hc55516Int2)
MACHINE_DRIVER_END

MACHINE_DRIVER_START(wmssnd_s11b2s)
  MDRV_IMPORT_FROM(wmssnd_s11js)
  MDRV_CPU_ADD(M6808, 1000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(s11s_readmem, s11s_writemem)
  MDRV_INTERLEAVE(50)
  MDRV_SOUND_ADD_TAG("dac", DAC,    s11xs_dacInt2)
  MDRV_SOUND_ADD_TAG("cvsd",HC55516,s11b2s_hc55516Int)
MACHINE_DRIVER_END

static void s11s_piaIrq(int state);
static const struct pia6821_interface s11s_pia[] = {{
 /* PIA 0 (sound 2000) S11 */
 /* PA0 - PA7 (I) Sound Select Input (soundlatch) */
 /* PB0 - PB7 DAC */
 /* CA1       (I) Sound H.S */
 /* CB1       (I) 1ms */
 /* CA2       55516 Clk */
 /* CB2       55516 Dig */
 /* subType 0 and 4 for S9 and S11S */
 /* in  : A/B,CA/B1,CA/B2 */ soundlatch_r, 0, PIA_UNUSED_VAL(0), 0, 0, 0,
 /* out : A/B,CA/B2       */ 0, DAC_0_data_w, hc55516_0_clock_w, hc55516_0_digit_w,
 /* irq : A/B             */ s11s_piaIrq, s11s_piaIrq
},{
 /* subType 1 for S11X */
 /* in  : A/B,CA/B1,CA/B2 */ soundlatch_r, 0, 0, 0, 0, 0,
 /* out : A/B,CA/B2       */ 0, DAC_1_data_w, hc55516_1_clock_w, hc55516_1_digit_w,
 /* irq : A/B             */ s11s_piaIrq, s11s_piaIrq
},{
 /* subType 2 for S11B2 */
 /* in  : A/B,CA/B1,CA/B2 */ soundlatch_r, 0, 0, 0, 0, 0,
 /* out : A/B,CA/B2       */ 0, DAC_1_data_w, hc55516_0_clock_w, hc55516_0_digit_w,
 /* irq : A/B             */ s11s_piaIrq, s11s_piaIrq
}};

static struct {
  struct sndbrdData brdData;
} s11slocals;

static void s11s_init(struct sndbrdData *brdData) {
  int i;
  s11slocals.brdData = *brdData;
  pia_config(S11S_PIA0, PIA_STANDARD_ORDERING, &s11s_pia[s11slocals.brdData.subType & 3]);
  if (s11slocals.brdData.subType) {
    cpu_setbank(S11S_BANK0, s11slocals.brdData.romRegion+0xc000);
    cpu_setbank(S11S_BANK1, s11slocals.brdData.romRegion+0x4000);
  }
  for (i=0; i < 0x1000; i++) memory_region(S9S_CPUREGION)[i] = 0xff;
  if (core_gameData->hw.gameSpecific2) {
    hc55516_set_gain(0, core_gameData->hw.gameSpecific2);
  }
}
static WRITE_HANDLER(s11s_manCmd_w) {
  soundlatch_w(0, data); pia_set_input_ca1(S11S_PIA0, 1); pia_set_input_ca1(S11S_PIA0, 0);
}
static WRITE_HANDLER(s11s_bankSelect) {
  cpu_setbank(S11S_BANK0, s11slocals.brdData.romRegion + 0x8000+((data&0x01)<<14));
  cpu_setbank(S11S_BANK1, s11slocals.brdData.romRegion + 0x0000+((data&0x02)<<13));
}

static void s11s_piaIrq(int state) {
  cpu_set_irq_line(s11slocals.brdData.cpuNo, M6808_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

static void s11s_diag(int button) {
  cpu_set_nmi_line(s11slocals.brdData.cpuNo, button ? ASSERT_LINE : CLEAR_LINE);
}

/*--------------------------
/ Pennant Fever sound board.
/ Thanks to Destruk for
/ buying the schematics! :)
/---------------------------*/
#define S9P_PIA0    6

static struct {
  struct sndbrdData brdData;
} s9plocals;

static WRITE_HANDLER(s9p_hs_w) { pia_set_input_ca1(0, data); }
static WRITE_HANDLER(ext_hs_w) { pia_set_input_ca1(S9P_PIA0, data); }
static void s9p_piaIrq(int state) {
  cpu_set_irq_line(s9plocals.brdData.cpuNo, M6808_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

static const struct pia6821_interface s9p_pia = {
 /* PIA 0 (4000) */
 /* PA0 - PA7 CPU interface (MDx) */
 /* PB0 - PB7 DAC */
 /* in  : A/B,CA/B1,CA/B2 */
  soundlatch_r, 0, 0, 0, 0, 0,
 /* out : A/B,CA/B2       */
  0, DAC_0_data_w, s9p_hs_w, 0,
 /* irq : A/B             */
  s9p_piaIrq, s9p_piaIrq
};

static void s9p_init(struct sndbrdData *brdData) {
  s9plocals.brdData = *brdData;
  pia_config(S9P_PIA0, PIA_STANDARD_ORDERING, &s9p_pia);
}

static void s9p_diag(int state) {
  cpu_set_nmi_line(s9plocals.brdData.cpuNo, state ? ASSERT_LINE : CLEAR_LINE);
}

static MEMORY_READ_START(s9p_readmem)
  { 0x0000, 0x00ff, MRA_RAM },
  { 0x4000, 0x4003, pia_r(S9P_PIA0) },
  { 0xc000, 0xffff, MRA_ROM },
MEMORY_END

static MEMORY_WRITE_START(s9p_writemem)
  { 0x0000, 0x00ff, MWA_RAM },
  { 0x4000, 0x4003, pia_w(S9P_PIA0) },
MEMORY_END

static struct DACinterface      s9p_dacInt      = { 1, { 60 }};

const struct sndbrdIntf s9psIntf = {
  "WMSS9P", s9p_init, NULL, s9p_diag, soundlatch_w, soundlatch_w, NULL, ext_hs_w
};

MACHINE_DRIVER_START(wmssnd_s9ps)
  MDRV_CPU_ADD(M6808, 1000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(s9p_readmem, s9p_writemem)
  MDRV_INTERLEAVE(50)
  MDRV_SOUND_ADD_TAG("dac",  DAC,    s9p_dacInt)
  MDRV_SOUND_ADD(SAMPLES, samples_interface)
MACHINE_DRIVER_END

/*--------------------------
/    System 11C sound board
/---------------------------*/
#define S11CS_PIA0    7
#define S11CS_BANK0   4

static void s11cs_piaIrqA(int state);
static void s11cs_piaIrqB(int state);
static WRITE_HANDLER(s11cs_pia0ca2_w);
static WRITE_HANDLER(s11cs_pia0cb2_w);
static WRITE_HANDLER(s11cs_rombank_w);
static WRITE_HANDLER(s11cs_manCmd_w);
static void s11cs_init(struct sndbrdData *brdData);

static struct {
  struct sndbrdData brdData;
  int ignore;
} s11clocals;

static WRITE_HANDLER(cslatch2_w) {
  if (!s11clocals.ignore)
    soundlatch2_w(offset, data);
  else s11clocals.ignore--;
}

const struct sndbrdIntf s11csIntf = {
  "WMSS11C", s11cs_init, NULL, NULL, s11cs_manCmd_w,
  cslatch2_w, NULL,
  CAT3(pia_,S11CS_PIA0,_cb1_w), soundlatch3_r
};

static MEMORY_READ_START(s11cs_readmem)
  { 0x0000, 0x1fff, MRA_RAM },
  { 0x2001, 0x2001, YM2151_status_port_0_r }, /* 2001-2fff odd */
  { 0x4000, 0x4003, pia_r(S11CS_PIA0) },      /* 4000-4fff */
  { 0x8000, 0xffff, MRA_BANKNO(S11CS_BANK0) },
MEMORY_END

static WRITE_HANDLER(odd_w) {
  logerror("Star Trax sound write: %02x:%02x\n", offset, data);
}
static MEMORY_WRITE_START(s11cs_writemem)
  { 0x0000, 0x1fff, MWA_RAM },
  { 0x2000, 0x2000, YM2151_register_port_0_w },     /* 2000-2ffe even */
  { 0x2001, 0x2001, YM2151_data_port_0_w },         /* 2001-2fff odd */
  { 0x4000, 0x4003, pia_w(S11CS_PIA0) },            /* 4000-4fff */
  { 0x6000, 0x6000, hc55516_0_digit_clock_clear_w },/* 6000-67ff */
  { 0x6800, 0x6800, hc55516_0_clock_set_w },        /* 6800-6fff */
  { 0x7800, 0x7800, s11cs_rombank_w },              /* 7800-7fff */
  { 0x9c00, 0x9cff, odd_w },
MEMORY_END

static struct DACinterface      s11cs_dacInt      = { 1, { 50 }};
static struct hc55516_interface s11cs_hc55516Int  = { 1, { 100 }};

MACHINE_DRIVER_START(wmssnd_s11cs)
  MDRV_CPU_ADD(M6809, 2000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(s11cs_readmem, s11cs_writemem)
  MDRV_INTERLEAVE(50)
  MDRV_SOUND_ADD(YM2151, s11cs_ym2151Int)
  MDRV_SOUND_ADD_TAG("dac",  DAC,    s11cs_dacInt)
  MDRV_SOUND_ADD_TAG("cvsd", HC55516,s11cs_hc55516Int)
  MDRV_SOUND_ADD(SAMPLES, samples_interface)
MACHINE_DRIVER_END

static const struct pia6821_interface s11cs_pia = {
 /* PIA 0 (4000) */
 /* PA0 - PA7 DAC */
 /* PB0 - PB7 CPU interface (MDx) */
 /* CA1       YM2151 IRQ */
 /* CB1       (I) CPU interface (MCB2) */
 /* CA2       YM 2151 pin 3 (Reset ?) */
 /* CB2       CPU interface (MCB1) */
 /* in  : A/B,CA/B1,CA/B2 */
  0, soundlatch2_r, 0, 0, 0, 0,
 /* out : A/B,CA/B2       */
  DAC_0_data_w, soundlatch3_w, s11cs_pia0ca2_w, s11cs_pia0cb2_w,
 /* irq : A/B             */
  s11cs_piaIrqA, s11cs_piaIrqB
};

static WRITE_HANDLER(s11cs_rombank_w) {
  cpu_setbank(S11CS_BANK0, s11clocals.brdData.romRegion + 0x10000*(data & 0x03) + 0x8000*((data & 0x04)>>2));
}
static void s11cs_init(struct sndbrdData *brdData) {
  s11clocals.brdData = *brdData;
  s11clocals.ignore = core_gameData->hw.gameSpecific1 & S11_SNDDELAY ? 7 : 0;
  pia_config(S11CS_PIA0, PIA_STANDARD_ORDERING, &s11cs_pia);
  cpu_setbank(S11CS_BANK0, s11clocals.brdData.romRegion);
}
static WRITE_HANDLER(s11cs_manCmd_w) {
  cslatch2_w(0, data); pia_set_input_cb1(S11CS_PIA0, 1); pia_set_input_cb1(S11CS_PIA0, 0);
}

static WRITE_HANDLER(s11cs_pia0ca2_w) { if (!data) YM2151_sh_reset(); }
static WRITE_HANDLER(s11cs_pia0cb2_w) { sndbrd_data_cb(s11clocals.brdData.boardNo,data); }

static void s11cs_ym2151IRQ(int state) { pia_set_input_ca1(S11CS_PIA0, !state); }
static void s11cs_piaIrqA(int state) {
  cpu_set_irq_line(s11clocals.brdData.cpuNo, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}
static void s11cs_piaIrqB(int state) {
  cpu_set_nmi_line(s11clocals.brdData.cpuNo, state ? ASSERT_LINE : CLEAR_LINE);
}

/*--------------------------
/   S11 Jokerz sound board
/---------------------------*/
#define S11JS_BANK0   4

static void s11js_ym2151IRQ(int state);
static void s11js_init(struct sndbrdData *brdData);
static WRITE_HANDLER(s11js_reply_w);
static WRITE_HANDLER(s11js_rombank_w);
static WRITE_HANDLER(s11js_ctrl_w);
static WRITE_HANDLER(s11js_manCmd_w);

static struct {
  struct sndbrdData brdData;
  int irqen;
  int ignore;
} s11jlocals;

static WRITE_HANDLER(jlatch2_w) {
  if (!s11jlocals.ignore)
    soundlatch2_w(offset, data);
  else s11jlocals.ignore--;
}

const struct sndbrdIntf s11jsIntf = {
  "WMSS11J", s11js_init, NULL, NULL, s11js_manCmd_w,
  jlatch2_w, soundlatch3_r,
  s11js_ctrl_w, NULL, 0
};

static WRITE_HANDLER(s11js_odd_w) {
  logerror("%04x: Jokerz ROM write to 0xf8%02x = %02x\n", activecpu_get_previouspc(), offset, data);
}
static WRITE_HANDLER(s11js_dac_w) {
  logerror("%04x: Jokerz DAC write = %02x\n", activecpu_get_previouspc(), data);
}
static WRITE_HANDLER(s11js_sync_w) {
  logerror("%04x: Jokerz SYNC write = %02x\n", activecpu_get_previouspc(), data);
}
static READ_HANDLER(s11js_port_r) {
  s11jlocals.irqen = 0;
  cpu_set_irq_line(s11jlocals.brdData.cpuNo, M6809_IRQ_LINE, CLEAR_LINE);
  return soundlatch2_r(offset);
}

static MEMORY_READ_START(s11js_readmem)
  { 0x0000, 0x1fff, MRA_RAM },
  { 0x2001, 0x2001, YM2151_status_port_0_r }, /* 2001-2fff odd */
  { 0x3400, 0x3400, s11js_port_r },
  { 0x4000, 0xbfff, MRA_BANKNO(S11JS_BANK0) },
  { 0xc000, 0xffff, MRA_ROM },
MEMORY_END

static MEMORY_WRITE_START(s11js_writemem)
  { 0x0000, 0x1fff, MWA_RAM },
  { 0x2000, 0x2000, YM2151_register_port_0_w },     /* 2000-2ffe even */
  { 0x2001, 0x2001, YM2151_data_port_0_w },         /* 2001-2fff odd */
  { 0x2800, 0x2800, s11js_reply_w },
  { 0x3000, 0x3000, s11js_dac_w },
  { 0x3800, 0x3800, s11js_rombank_w },
  { 0x3c00, 0x3c00, s11js_sync_w },
  { 0xf800, 0xf8ff, s11js_odd_w },
MEMORY_END

static struct YM2151interface   s11js_ym2151Int  = {
  1, 3579545, /* Hz */
  { YM3012_VOL(30,MIXER_PAN_LEFT,30,MIXER_PAN_RIGHT) },
  { s11js_ym2151IRQ }
};

MACHINE_DRIVER_START(wmssnd_s11js)
  MDRV_CPU_ADD(M6809, 2000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(s11js_readmem, s11js_writemem)
  MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
  MDRV_SOUND_ADD(           YM2151, s11js_ym2151Int)
  MDRV_SOUND_ADD(SAMPLES, samples_interface)
MACHINE_DRIVER_END

static WRITE_HANDLER(s11js_rombank_w) {
  cpu_setbank(S11JS_BANK0, s11jlocals.brdData.romRegion + 0x8000*(data & 0x01));
}
static void s11js_init(struct sndbrdData *brdData) {
  s11jlocals.brdData = *brdData;
  s11jlocals.ignore = 10;
//  cpu_setbank(S11JS_BANK0, s11jlocals.brdData.romRegion);
}
static WRITE_HANDLER(s11js_ctrl_w) {
  if (!s11jlocals.ignore) {
    if (!data) s11jlocals.irqen++;
    if (s11jlocals.irqen) cpu_set_irq_line(s11jlocals.brdData.cpuNo, M6809_IRQ_LINE, ASSERT_LINE);
  }
}
static WRITE_HANDLER(s11js_reply_w) {
  soundlatch3_w(0,data);
  sndbrd_data_cb(s11jlocals.brdData.boardNo, 0); sndbrd_data_cb(s11jlocals.brdData.boardNo, 1);
}
static WRITE_HANDLER(s11js_manCmd_w) {
//printf("m:%02x ", data); // the manual commands are not passed through, why???
  jlatch2_w(0, data); s11js_ctrl_w(0,0);
}
static void s11js_ym2151IRQ(int state) {
  cpu_set_irq_line(s11jlocals.brdData.cpuNo, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


/*------------------
/  WPC sound board
/-------------------*/
#define WPCS_BANK0  4

/*-- internal sound interface --*/
static WRITE_HANDLER(wpcs_latch_w);
static READ_HANDLER(wpcs_latch_r);

/*-- external interfaces --*/
static void wpcs_init(struct sndbrdData *brdData);
static READ_HANDLER(wpcs_data_r);
static WRITE_HANDLER(wpcs_data_w);
static READ_HANDLER(wpcs_ctrl_r);
static WRITE_HANDLER(wpcs_ctrl_w);
const struct sndbrdIntf wpcsIntf = { "WPCS", wpcs_init, NULL, NULL, wpcs_data_w, wpcs_data_w, wpcs_data_r, wpcs_ctrl_w, wpcs_ctrl_r };

/*-- other memory handlers --*/
static WRITE_HANDLER(wpcs_rombank_w);
static WRITE_HANDLER(wpcs_volume_w);
static void wpcs_ym2151IRQ(int state);

/*-- local data --*/
static struct {
  struct sndbrdData brdData;
  int replyAvail;
  int volume;
} locals;

static WRITE_HANDLER(wpcs_rombank_w) {
  /* the hardware can actually handle 1M chip but no games uses it */
  /* if such ROM appear the region must be doubled and mask set to 0x1f */
  /* this would be much easier if the region was filled in opposit order */
  /* but I don't want to change it now */
  int bankBase = data & 0x0f;
#ifdef MAME_DEBUG
  /* this register can no be read but this makes debugging easier */
  *(memory_region(REGION_CPU1+locals.brdData.cpuNo) + 0x2000) = data;
#endif /* MAME_DEBUG */

  switch ((~data) & 0xe0) {
    case 0x80: /* U18 */
      bankBase |= 0x00; break;
    case 0x40: /* U15 */
      bankBase |= 0x10; break;
    case 0x20: /* U14 */
      bankBase |= 0x20; break;
    default:
      DBGLOG(("WPCS:Unknown bank %x\n",data)); return;
  }
  cpu_setbank(WPCS_BANK0, locals.brdData.romRegion + (bankBase<<15));
}

static WRITE_HANDLER(wpcs_volume_w) {
  if (data & 0x01) {
    if ((locals.volume > 0) && (data & 0x02))
      locals.volume -= 1;
    else if ((locals.volume < 0xff) && ((data & 0x02) == 0))
      locals.volume += 1;
    /* DBGLOG(("Volume set to %d\n",locals.volume)); */
    {
      int ch;
      for (ch = 0; ch < MIXER_MAX_CHANNELS; ch++) {
        if (mixer_get_name(ch) != NULL)
          mixer_set_volume(ch, locals.volume * 100 / 127);
      }
    }
  }
}

static WRITE_HANDLER(wpcs_latch_w) {
  locals.replyAvail = TRUE; soundlatch2_w(0,data);
  sndbrd_data_cb(locals.brdData.boardNo, data);
}

static READ_HANDLER(wpcs_latch_r) {
  cpu_set_irq_line(locals.brdData.cpuNo, M6809_IRQ_LINE, CLEAR_LINE);
  return soundlatch_r(0);
}

static void wpcs_ym2151IRQ(int state) {
  cpu_set_irq_line(locals.brdData.cpuNo, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

#ifdef PREDCS_FIRQ_HACK

	//This value is based on a lot of trial and error and comparing the same voice sample played with and without music playing
	#define FIRQ_HACK_RATE        2400

	extern int YM2151ReadOutputFlag(int chip);

	//Force the FIRQ to toggle @ the specified rate, but only while the 2151 is not outputting sound
	static void firq_hack(int data) {
		static int last = 0;
		if(!YM2151ReadOutputFlag(0)) {
			if(last)
			{
				cpu_set_irq_line(locals.brdData.cpuNo, M6809_FIRQ_LINE, CLEAR_LINE);
				last = 0;
			}
			else
			{
				cpu_set_irq_line(locals.brdData.cpuNo, M6809_FIRQ_LINE, ASSERT_LINE);
				last++;
			}
		}

	}
#endif

static MEMORY_READ_START(wpcs_readmem)
  { 0x0000, 0x1fff, MRA_RAM },
  { 0x2401, 0x2401, YM2151_status_port_0_r }, /* 2401-27ff odd */
  { 0x3000, 0x3000, wpcs_latch_r }, /* 3000-33ff */
  { 0x4000, 0xbfff, CAT2(MRA_BANK, WPCS_BANK0) }, //32K
  { 0xc000, 0xffff, MRA_ROM }, /* same as page 7f */	//16K
MEMORY_END

static MEMORY_WRITE_START(wpcs_writemem)
  { 0x0000, 0x1fff, MWA_RAM },
  { 0x2000, 0x2000, wpcs_rombank_w }, /* 2000-23ff */
  { 0x2400, 0x2400, YM2151_register_port_0_w }, /* 2400-27fe even */
  { 0x2401, 0x2401, YM2151_data_port_0_w },     /* 2401-27ff odd */
  { 0x2800, 0x2800, DAC_0_data_w }, /* 2800-2bff */
  { 0x2c00, 0x2c00, hc55516_0_clock_set_w },  /* 2c00-2fff */
  { 0x3400, 0x3400, hc55516_0_digit_clock_clear_w }, /* 3400-37ff */
  { 0x3800, 0x3800, wpcs_volume_w }, /* 3800-3bff */
  { 0x3c00, 0x3c00, wpcs_latch_w },  /* 3c00-3fff */
MEMORY_END
//NOTE: These volume levels sound really good compared to my own Funhouse and T2. (Dac=100%,CVSD=80%,2151=15%)
static struct DACinterface      wpcs_dacInt     = { 1, { 100 }};
static struct hc55516_interface wpcs_hc55516Int = { 1, { 100 }};
static struct YM2151interface   wpcs_ym2151Int  = {
  1, 3579545, /* Hz */
  { YM3012_VOL(15,MIXER_PAN_CENTER,15,MIXER_PAN_CENTER) },
  { wpcs_ym2151IRQ }
};

MACHINE_DRIVER_START(wmssnd_wpcs)
  MDRV_CPU_ADD(M6809, 2000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(wpcs_readmem, wpcs_writemem)
  MDRV_INTERLEAVE(50)
  MDRV_SOUND_ADD(YM2151, wpcs_ym2151Int)
  MDRV_SOUND_ADD(DAC,    wpcs_dacInt)
  MDRV_SOUND_ADD(HC55516,wpcs_hc55516Int)
  MDRV_SOUND_ADD(SAMPLES, samples_interface)
#ifdef PREDCS_FIRQ_HACK
  //Force the FIRQ to toggle @ the specified rate, but only while the 2151 is not outputting sound
  MDRV_TIMER_ADD(firq_hack, FIRQ_HACK_RATE)
#endif
MACHINE_DRIVER_END

/*---------------------
/  Interface functions
/----------------------*/
static READ_HANDLER(wpcs_data_r) {
  locals.replyAvail = FALSE; return soundlatch2_r(0);
}

static WRITE_HANDLER(wpcs_data_w) {
  soundlatch_w(0, data); cpu_set_irq_line(locals.brdData.cpuNo, M6809_IRQ_LINE, ASSERT_LINE);
}

static READ_HANDLER(wpcs_ctrl_r) {
  return locals.replyAvail;
}

static WRITE_HANDLER(wpcs_ctrl_w) { /*-- a write here resets the CPU --*/
  cpunum_set_reset_line(locals.brdData.cpuNo, PULSE_LINE);
}

static void wpcs_init(struct sndbrdData *brdData) {
  locals.brdData = *brdData;
  /* the non-paged ROM is at the end of the image. move it to its correct place */
  memcpy(memory_region(REGION_CPU1+locals.brdData.cpuNo) + 0x00c000, locals.brdData.romRegion + 0x07c000, 0x4000);
  wpcs_rombank_w(0,0);
  hc55516_set_gain(0, 4250);
}

/*--------------------
/  DCS sound board
/---------------------*/
/*-- ADSP core functions --*/
static void adsp_init(data8_t *(*getBootROM)(int soft),
               void (*txData)(UINT16 start, UINT16 size, UINT16 memStep, int sRate));
static void adsp_boot(int soft);
static void adsp_txCallback(int port, INT32 data);
static WRITE16_HANDLER(adsp_control_w);

/*-- bank handlers --*/
static WRITE16_HANDLER(dcs1_ROMbankSelect1_w);
static WRITE16_HANDLER(dcs2_ROMbankSelect1_w);
static WRITE16_HANDLER(dcs2_ROMbankSelect2_w);
static WRITE16_HANDLER(dcs2_RAMbankSelect_w);
static READ16_HANDLER(dcs2_RAMbankSelect_r);
static READ16_HANDLER(dcs_ROMbank_r);
static READ16_HANDLER(dcs2_RAMbank_r);
static WRITE16_HANDLER(dcs2_RAMbank_w);

/*-- sound interface handlers --*/
/* once the ADSP core is updated to handle PM mapping */
/* these can be static */
/*static*/ READ16_HANDLER(dcs_latch_r);
/*static*/ WRITE16_HANDLER(dcs_latch_w);
/* ADSP patch need to know if we are using dcs95 soundboard */
int WPC_gWPC95;

/*-- sound generation --*/
static int dcs_custStart(const struct MachineSound *msound);
static void dcs_custStop(void);
static void dcs_dacUpdate(int num, INT16 *buffer, int length);
static void dcs_txData(UINT16 start, UINT16 size, UINT16 memStep, int sRate);

/*-- external interface --*/
static READ_HANDLER(dcs_data_r);
static WRITE_HANDLER(dcs_data_w);
static READ_HANDLER(dcs_ctrl_r);
static WRITE_HANDLER(dcs_ctrl_w);
static void dcs_init(struct sndbrdData *brdData);

/*-- local data --*/
#define DCS_BUFFER_SIZE	  4096
#define DCS_BUFFER_MASK	  (DCS_BUFFER_SIZE - 1)

static struct {
 int     enabled;
 UINT32  sOut, sIn, sStep;
 INT16  *buffer;
 int     stream;
} dcs_dac;

static struct {
  struct sndbrdData brdData;
  UINT8  *cpuRegion;
  UINT16  ROMbank1, ROMbank2;
  UINT16  RAMbank;
  UINT8  *ROMbankPtr;
  UINT16 *RAMbankPtr;
  int     replyAvail;
} dcslocals;

static struct CustomSound_interface dcs_custInt = { dcs_custStart, dcs_custStop, 0 };

static MEMORY_READ16_START(dcs1_readmem)
  { ADSP_DATA_ADDR_RANGE(0x0000, 0x1fff), MRA16_RAM },
  { ADSP_DATA_ADDR_RANGE(0x2000, 0x2fff), dcs_ROMbank_r },
  { ADSP_DATA_ADDR_RANGE(0x3800, 0x39ff), MRA16_RAM },
  { ADSP_PGM_ADDR_RANGE (0x0000, 0x0800), MRA16_RAM }, /* Internal boot RAM */
  { ADSP_PGM_ADDR_RANGE (0x1000, 0x2fff), MRA16_RAM }, /* External RAM */
  { ADSP_PGM_ADDR_RANGE (0x3000, 0x3000), dcs_latch_r },
MEMORY_END

static MEMORY_WRITE16_START(dcs1_writemem)
  { ADSP_DATA_ADDR_RANGE(0x0000, 0x1fff), MWA16_RAM },
  { ADSP_DATA_ADDR_RANGE(0x2000, 0x2fff), MWA16_ROM },
  { ADSP_DATA_ADDR_RANGE(0x3000, 0x3000), dcs1_ROMbankSelect1_w },
  { ADSP_DATA_ADDR_RANGE(0x3800, 0x39ff), MWA16_RAM },
  { ADSP_DATA_ADDR_RANGE(0x3fe0, 0x3fff), adsp_control_w },
  { ADSP_PGM_ADDR_RANGE (0x0000, 0x0800), MWA16_RAM },
  { ADSP_PGM_ADDR_RANGE (0x1000, 0x2fff), MWA16_RAM },
  { ADSP_PGM_ADDR_RANGE (0x3000, 0x3000), dcs_latch_w },
MEMORY_END

MACHINE_DRIVER_START(wmssnd_dcs1)
  MDRV_CPU_ADD(ADSP2105, 10000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(dcs1_readmem, dcs1_writemem)
  MDRV_INTERLEAVE(50)
  MDRV_SOUND_ADD(CUSTOM, dcs_custInt)
  MDRV_SOUND_ADD(SAMPLES, samples_interface)
MACHINE_DRIVER_END

static MEMORY_READ16_START(dcs2_readmem)
  { ADSP_DATA_ADDR_RANGE(0x0000, 0x07ff), dcs_ROMbank_r },
  { ADSP_DATA_ADDR_RANGE(0x1000, 0x1fff), MRA16_RAM },
  { ADSP_DATA_ADDR_RANGE(0x2000, 0x2fff), dcs2_RAMbank_r },
  { ADSP_DATA_ADDR_RANGE(0x3200, 0x3200), dcs2_RAMbankSelect_r },
  { ADSP_DATA_ADDR_RANGE(0x3300, 0x3300), dcs_latch_r },
  { ADSP_DATA_ADDR_RANGE(0x3800, 0x39ff), MRA16_RAM },
  { ADSP_PGM_ADDR_RANGE (0x0000, 0x0800), MRA16_RAM }, /* Internal boot RAM */
  { ADSP_PGM_ADDR_RANGE (0x1000, 0x3fff), MRA16_RAM }, /* External RAM */
MEMORY_END

static MEMORY_WRITE16_START(dcs2_writemem)
  { ADSP_DATA_ADDR_RANGE(0x0000, 0x07ff), MWA16_ROM },
  { ADSP_DATA_ADDR_RANGE(0x1000, 0x1fff), MWA16_RAM },
  { ADSP_DATA_ADDR_RANGE(0x2000, 0x2fff), dcs2_RAMbank_w },
  { ADSP_DATA_ADDR_RANGE(0x3000, 0x3000), dcs2_ROMbankSelect1_w },
  { ADSP_DATA_ADDR_RANGE(0x3100, 0x3100), dcs2_ROMbankSelect2_w },
  { ADSP_DATA_ADDR_RANGE(0x3200, 0x3200), dcs2_RAMbankSelect_w },
  { ADSP_DATA_ADDR_RANGE(0x3300, 0x3300), dcs_latch_w },
  { ADSP_DATA_ADDR_RANGE(0x3800, 0x39ff), MWA16_RAM },
  { ADSP_DATA_ADDR_RANGE(0x3fe0, 0x3fff), adsp_control_w },
  { ADSP_PGM_ADDR_RANGE (0x0000, 0x0800), MWA16_RAM }, /* Internal boot RAM */
  { ADSP_PGM_ADDR_RANGE (0x1000, 0x3fff), MWA16_RAM }, /* External RAM */
MEMORY_END

MACHINE_DRIVER_START(wmssnd_dcs2)
  MDRV_CPU_ADD(ADSP2105, 10000000)
  MDRV_CPU_FLAGS(CPU_AUDIO_CPU)
  MDRV_CPU_MEMORY(dcs2_readmem, dcs2_writemem)
  MDRV_INTERLEAVE(50)
  MDRV_SOUND_ADD(CUSTOM, dcs_custInt)
  MDRV_SOUND_ADD(SAMPLES, samples_interface)
MACHINE_DRIVER_END

/*----------------
/ Sound interface
/-----------------*/
const struct sndbrdIntf dcsIntf = { "DCS", dcs_init, NULL, NULL, dcs_data_w, dcs_data_w, dcs_data_r, dcs_ctrl_w, dcs_ctrl_r };

/*---------------
/  Bank handlers
/----------------*/
#define DCS1_ROMBANKBASE(bank) \
  (dcslocals.brdData.romRegion + (((bank) & 0x7ff)<<12))
#define DCS2_ROMBANKBASE(bankH, bankL) \
  (dcslocals.brdData.romRegion + (((bankH) & 0x1c)<<18) + (((bankH) & 0x01)<<19) + (((bankL) & 0xff)<<11))
#define DCS2_RAMBANKBASE(bank) \
  ((UINT16 *)(((bank) & 0x08) ? memory_region(DCS_BANKREGION) : \
              (dcslocals.cpuRegion + ADSP2100_DATA_OFFSET + (0x2000<<1))))

static WRITE16_HANDLER(dcs1_ROMbankSelect1_w) {
  dcslocals.ROMbank1 = data;
  dcslocals.ROMbankPtr = DCS1_ROMBANKBASE(dcslocals.ROMbank1);
}
static WRITE16_HANDLER(dcs2_ROMbankSelect1_w) {
  dcslocals.ROMbank1 = data;
  dcslocals.ROMbankPtr = DCS2_ROMBANKBASE(dcslocals.ROMbank2,dcslocals.ROMbank1);
}
static WRITE16_HANDLER(dcs2_ROMbankSelect2_w) {
  dcslocals.ROMbank2 = data;
  dcslocals.ROMbankPtr = DCS2_ROMBANKBASE(dcslocals.ROMbank2,dcslocals.ROMbank1);
}
static WRITE16_HANDLER(dcs2_RAMbankSelect_w) {
  dcslocals.RAMbank  = data;
  dcslocals.RAMbankPtr = DCS2_RAMBANKBASE(dcslocals.RAMbank);
}
static READ16_HANDLER (dcs2_RAMbankSelect_r) {
  return dcslocals.RAMbank;
}

static READ16_HANDLER(dcs_ROMbank_r)   { return dcslocals.ROMbankPtr[offset]; }

static READ16_HANDLER(dcs2_RAMbank_r)  { return dcslocals.RAMbankPtr[offset]; }

static WRITE16_HANDLER(dcs2_RAMbank_w) { dcslocals.RAMbankPtr[offset] = data; }

static data8_t *dcs_getBootROM(int soft) {
  return (data8_t *)(dcslocals.brdData.romRegion +
                    (soft ? ((dcslocals.ROMbank1 & 0xff)<<12) : 0));
}

/*----------------------
/ Other memory handlers
/-----------------------*/
/* These should be static but the patched ADSP core requires them */

/*static*/ READ16_HANDLER(dcs_latch_r) {
  cpu_set_irq_line(dcslocals.brdData.cpuNo, ADSP2105_IRQ2, CLEAR_LINE);
#if 1
  return soundlatch_r(0);
#else
  { int x = soundlatch_r(0); DBGLOG(("Latch_r: %02x\n",x)); return x; }
#endif
}

/*static*/ WRITE16_HANDLER(dcs_latch_w) {
  soundlatch2_w(0, data);
  dcslocals.replyAvail = TRUE;
  sndbrd_data_cb(dcslocals.brdData.boardNo, data);
}

static int dcs_custStart(const struct MachineSound *msound) {
  /*-- clear DAC data --*/
  memset(&dcs_dac,0,sizeof(dcs_dac));

  /*-- allocate a DAC stream --*/
  dcs_dac.stream = stream_init("DCS DAC", 100, 32000, 0, dcs_dacUpdate);

  /*-- allocate memory for our buffer --*/
  dcs_dac.buffer = malloc(DCS_BUFFER_SIZE * sizeof(INT16));

  dcs_dac.sStep = 0x10000;

  return (dcs_dac.buffer == 0);
}

static void dcs_custStop(void) {
  if (dcs_dac.buffer)
    { free(dcs_dac.buffer); dcs_dac.buffer = NULL; }
}

static void dcs_dacUpdate(int num, INT16 *buffer, int length) {
  if (!dcs_dac.enabled)
    memset(buffer, 0, length * sizeof(INT16));
  else {
    int ii;
    /* fill in with samples until we hit the end or run out */
    for (ii = 0; ii < length; ii++) {
      if (dcs_dac.sOut == dcs_dac.sIn) break;
      buffer[ii] = dcs_dac.buffer[dcs_dac.sOut];
      dcs_dac.sOut = (dcs_dac.sOut + 1) & DCS_BUFFER_MASK;
    }
    /* fill the rest with the last sample */
    for ( ; ii < length; ii++)
      buffer[ii] = dcs_dac.buffer[(dcs_dac.sOut - 1) & DCS_BUFFER_MASK];
  }
}

/*-------------------
/ Exported interface
/---------------------*/
static READ_HANDLER(dcs_data_r) {
  dcslocals.replyAvail = FALSE;
  return soundlatch2_r(0) & 0xff;
}

static WRITE_HANDLER(dcs_data_w) {
  DBGLOG(("Latch_w: %02x\n",data));
  soundlatch_w(0, data); cpu_set_irq_line(dcslocals.brdData.cpuNo, ADSP2105_IRQ2, ASSERT_LINE);
}

static WRITE_HANDLER(dcs_ctrl_w) {
  DBGLOG(("ctrl_w: %02x\n",data));

  // Tom: Removed the next line which now prevents some "sound board interface error"
  // if (dcslocals.brdData.subType < 2)
  {
#ifdef WPCDCSSPEEDUP
    // probably a bug in the mame reset handler
    // if a cpu is suspended for some reason a reset will not wake it up
    cpu_triggerint(dcslocals.brdData.cpuNo);
#endif /* WPCDCSSPEEDUP */
    // Reset is triggered on write so just pulse the line
    cpunum_set_reset_line(dcslocals.brdData.cpuNo, PULSE_LINE);
    adsp_boot(0);
  }
}

static READ_HANDLER(dcs_ctrl_r) {
  return dcslocals.replyAvail ? 0x80 : 0x00;
}

/*----------------------------
/ Checksum in DCS
/ GEN_SECURITY: Page 0x0004
/ WPC95:        Page 0x000c
/
/ Pages, StartPage, ChkSum
/
-----------------------------*/

/*-- handle bug in ADSP core */
static OPBASE_HANDLER(opbaseoveride) { return -1; }

static void dcs_init(struct sndbrdData *brdData) {
  memset(&dcslocals, 0, sizeof(dcslocals));
  dcslocals.brdData = *brdData;
  dcslocals.cpuRegion = memory_region(REGION_CPU1+dcslocals.brdData.cpuNo);
  memory_set_opbase_handler(dcslocals.brdData.cpuNo, opbaseoveride);
  /*-- initialize our structure --*/
  dcslocals.ROMbankPtr = dcslocals.brdData.romRegion;
  dcslocals.RAMbankPtr = (UINT16 *)memory_region(DCS_BANKREGION);

  WPC_gWPC95 = brdData->subType;

  adsp_init(dcs_getBootROM, dcs_txData);

  /*-- clear all interrupts --*/
  cpu_set_irq_line(dcslocals.brdData.cpuNo, ADSP2105_IRQ0, CLEAR_LINE );
  cpu_set_irq_line(dcslocals.brdData.cpuNo, ADSP2105_IRQ1, CLEAR_LINE );
  cpu_set_irq_line(dcslocals.brdData.cpuNo, ADSP2105_IRQ2, CLEAR_LINE );

  /*-- speed up startup by disable checksum --*/
#if 0
  if (options.cheat) {
    if (core_gameData->gen & GEN_WPC95)
      *((UINT16 *)(memory_region(WPC_MEMREG_SROM) + 0x6000)) = 0x0000;
    else
      *((UINT16 *)(memory_region(WPC_MEMREG_SROM) + 0x4000)) = 0x0000;
  }
#endif
  /*-- boot ADSP2100 --*/
  adsp_boot(0);
}

/*-----------------
/  local functions
/------------------*/
/*-- autobuffer SPORT transmission  --*/
/*-- copy data to transmit into dac buffer --*/
static void dcs_txData(UINT16 start, UINT16 size, UINT16 memStep, int sRate) {
  UINT16 *mem = ((UINT16 *)(dcslocals.cpuRegion + ADSP2100_DATA_OFFSET)) + start;
  int idx;

  stream_update(dcs_dac.stream, 0);
  if (size == 0) /* No data, stop playing */
    { dcs_dac.enabled = FALSE; return; }
  /*-- For some reason can't the sample rate of streams be changed --*/
  /*-- The DCS samplerate is now hardcoded to 32K. Seems to work with all games */
  // if (!dcs_dac.enabled) stream_set_sample_frequency(dcs_dac.stream,sRate);
  /*-- size is the size of the buffer not the number of samples --*/
#if MAMEVER >= 3716
  for (idx = 0; idx < size; idx += memStep) {
    dcs_dac.buffer[dcs_dac.sIn] = mem[idx];
    dcs_dac.sIn = (dcs_dac.sIn + 1) & DCS_BUFFER_MASK;
  }
#else /* MAMEVER */
  size /= memStep;
  sStep = sRate * 65536.0 / (double)(Machine->sample_rate);

  /*-- copy samples into buffer --*/
  while ((idx>>16) < size) {
    dcs_dac.buffer[dcs_dac.sIn] = mem[(idx>>16)*memStep];
    dcs_dac.sIn = (dcs_dac.sIn + 1) & DCS_BUFFER_MASK;
    idx += sStep;
  }
#endif /* MAMEVER */
  /*-- enable the dac playing --*/
  dcs_dac.enabled = TRUE;
}
#define DCS_IRQSTEPS 4
/*--------------------------------------------------*/
/*-- This should actually be part of the CPU core --*/
/*--------------------------------------------------*/
enum {
  S1_AUTOBUF_REG = 15, S1_RFSDIV_REG, S1_SCLKDIV_REG, S1_CONTROL_REG,
  S0_AUTOBUF_REG, S0_RFSDIV_REG, S0_SCLKDIV_REG, S0_CONTROL_REG,
  S0_MCTXLO_REG, S0_MCTXHI_REG, S0_MCRXLO_REG, S0_MCRXHI_REG,
  TIMER_SCALE_REG, TIMER_COUNT_REG, TIMER_PERIOD_REG, WAITSTATES_REG,
  SYSCONTROL_REG
};

static struct {
 UINT16  ctrlRegs[32];
 void   *irqTimer;
 data8_t *(*getBootROM)(int soft);
 void   (*txData)(UINT16 start, UINT16 size, UINT16 memStep, int sRate);
} adsp; /* = {{0},NULL,dcs_getBootROM,dcs_txData};*/
static void adsp_irqGen(int dummy);

static void adsp_init(data8_t *(*getBootROM)(int soft),
                     void (*txData)(UINT16 start, UINT16 size, UINT16 memStep, int sRate)) {
  /* stupid timer/machine init handling in MAME */
  if (adsp.irqTimer) timer_remove(adsp.irqTimer);
  /*-- reset control registers etc --*/
  memset(&adsp, 0, sizeof(adsp));
  adsp.getBootROM = getBootROM;
  adsp.txData = txData;
  adsp.irqTimer = timer_alloc(adsp_irqGen);
  /*-- initialize the ADSP Tx callback --*/
  adsp2105_set_tx_callback(adsp_txCallback);
}

#if MAMEVER < 6300
static void adsp2105_load_boot_data(data8_t *srcdata, data32_t *dstdata) {
  UINT32 size = 8 * (srcdata[3] + 1), i;
  for (i = 0; i < size; i++) {
    UINT32 opcode = (srcdata[i*4+0] << 16) | (srcdata[i*4+1] << 8) | srcdata[i*4+2];
    ADSP2100_WRPGM(&dstdata[i], opcode);
  }
}
#endif /* MAMEVER */

static void adsp_boot(int soft) {
  adsp2105_load_boot_data(adsp.getBootROM(soft), (UINT32 *)(dcslocals.cpuRegion+ADSP2100_PGM_OFFSET));
  timer_enable(adsp.irqTimer, FALSE);
}

static WRITE16_HANDLER(adsp_control_w) {
  adsp.ctrlRegs[offset] = data;
  switch (offset) {
    case SYSCONTROL_REG:
      if (data & 0x0200) {
        /* boot force */
        DBGLOG(("boot force\n"));
        cpunum_set_reset_line(dcslocals.brdData.cpuNo, PULSE_LINE);
        adsp_boot(1);
        adsp.ctrlRegs[SYSCONTROL_REG] &= ~0x0200;
      }
      /* see if SPORT1 got disabled */
      if ((data & 0x0800) == 0) {
        dcs_txData(0, 0, 0, 0);
        /* nuke the timer */
        timer_enable(adsp.irqTimer, FALSE);
      }
      break;
    case S1_AUTOBUF_REG:
      /* autobuffer off: nuke the timer */
      if ((data & 0x0002) == 0) {
        adsp.txData(0, 0, 0, 0);
        /* nuke the timer */
        timer_enable(adsp.irqTimer, FALSE);
      }
      break;
    case S1_CONTROL_REG:
      if (((data>>4) & 3) == 2)
	DBGLOG(("Oh no!, the data is compresed with u-law encoding\n"));
      if (((data>>4) & 3) == 3)
	DBGLOG(("Oh no!, the data is compresed with A-law encoding\n"));
      break;
  } /* switch */
}

static struct {
  UINT16 start;
  UINT16 size, step;
  int    sRate;
  int    iReg;
  int    last;
  int    irqCount;
} adsp_aBufData;

static void adsp_irqGen(int dummy) {
  int next;

  if (adsp_aBufData.irqCount < DCS_IRQSTEPS) {
    adsp_aBufData.irqCount += 1;
#ifdef WPCDCSSPEEDUP
    /* wake up suspended cpu by simulating an interrupt trigger */
    cpu_triggerint(dcslocals.brdData.cpuNo);
#endif /* WPCDCSSPEEDUP */
  }
  else {
    adsp_aBufData.irqCount = 1;
    adsp_aBufData.last = 0;
    cpu_set_irq_line(dcslocals.brdData.cpuNo, ADSP2105_IRQ1, PULSE_LINE);
  }

  next = (adsp_aBufData.size / adsp_aBufData.step * adsp_aBufData.irqCount /
          DCS_IRQSTEPS - 1) * adsp_aBufData.step;

  cpunum_set_reg(dcslocals.brdData.cpuNo, ADSP2100_I0 + adsp_aBufData.iReg,
                 adsp_aBufData.start + next);

  adsp.txData(adsp_aBufData.start + adsp_aBufData.last, (next - adsp_aBufData.last),
              adsp_aBufData.step, adsp_aBufData.sRate);

  adsp_aBufData.last = next;

}

static void adsp_txCallback(int port, INT32 data) {
  if (port != 1)
    { DBGLOG(("tx0 not handled\n")); return; };
  /*-- remove any pending timer --*/
  timer_enable(adsp.irqTimer, FALSE);
  if ((adsp.ctrlRegs[SYSCONTROL_REG] & 0x0800) == 0)
    DBGLOG(("tx1 without SPORT1 enabled\n"));
  else if ((adsp.ctrlRegs[S1_AUTOBUF_REG] & 0x0002) == 0)
    DBGLOG(("SPORT1 without autobuffer"));
  else {
    int ireg, mreg;

    ireg = (adsp.ctrlRegs[S1_AUTOBUF_REG]>>9) & 7;
    mreg = ((adsp.ctrlRegs[S1_AUTOBUF_REG]>>7) & 3) | (ireg & 0x04);

    /* start = In, size = Ln, step = Mn */
    adsp_aBufData.step  = activecpu_get_reg(ADSP2100_M0 + mreg);
    adsp_aBufData.size  = activecpu_get_reg(ADSP2100_L0 + ireg);
    /*-- assume that the first sample comes from the memory position before --*/
    adsp_aBufData.start = activecpu_get_reg(ADSP2100_I0 + ireg) - adsp_aBufData.step;
    adsp_aBufData.sRate = Machine->drv->cpu[dcslocals.brdData.cpuNo].cpu_clock /
                          (2 * (adsp.ctrlRegs[S1_SCLKDIV_REG] + 1)) / 16;
    adsp_aBufData.iReg = ireg;
    adsp_aBufData.irqCount = adsp_aBufData.last = 0;
    adsp_irqGen(0); /* first part, rest is handled via the timer */
    /*-- fire the irq timer --*/
    timer_adjust(adsp.irqTimer, 0, 0, TIME_IN_HZ(adsp_aBufData.sRate) *
                      adsp_aBufData.size / adsp_aBufData.step / DCS_IRQSTEPS);
    DBGLOG(("DCS size=%d,step=%d,rate=%d\n",adsp_aBufData.size,adsp_aBufData.step,adsp_aBufData.sRate));
    return;
  }
  /*-- if we get here, something went wrong. Disable transmission --*/
  adsp.txData(0, 0, 0, 0);
}

#ifdef WPCDCSSPEEDUP

UINT32 dcs_speedup(UINT32 pc) {
  UINT16 *ram1source, *ram2source, volume;
  int ii;

  /* DCS and DCS95 uses different buffers */
  if (pc > 0x2000) {
    UINT32 volumeOP = *(UINT32 *)&OP_ROM[ADSP2100_PGM_OFFSET + ((pc+0x2b84-0x2b44)<<2)];

    ram1source = (UINT16 *)(dcslocals.cpuRegion + ADSP2100_DATA_OFFSET + (0x1000<<1));
    ram2source = dcslocals.RAMbankPtr;
    volume = ram1source[((volumeOP>>4)&0x3fff)-0x1000];
    /*DBGLOG(("OP=%6x addr=%4x V=%4x\n",volumeOP,(volumeOP>>4)&0x3fff,volume));*/
  }
  else {
    UINT32 volumeOP = *(UINT32 *)&OP_ROM[ADSP2100_PGM_OFFSET + ((pc+0x2b84-0x2b44)<<2)];

    ram1source = (UINT16 *)(dcslocals.cpuRegion + ADSP2100_DATA_OFFSET + (0x0700<<1));
    ram2source = (UINT16 *)(dcslocals.cpuRegion + ADSP2100_DATA_OFFSET + (0x3800<<1));
    volume = ram2source[((volumeOP>>4)&0x3fff)-0x3800];
    /*DBGLOG(("OP=%6x addr=%4x V=%4x\n",volumeOP,(volumeOP>>4)&0x3fff,volume));*/
  }
  {
    UINT16 *i0, *i2;
			/* 2B44     I0 = $2000 >>> (3800) <<< */
    i0 = &ram2source[0];
			/* 2B45:    I2 = $2080 >>> (3880) <<< */
    i2 = &ram2source[0x0080];
			/* 2B46     M2 = $3FFF */
			/* 2B47     M3 = $3FFD */
			/* 2B48     CNTR = $0040 */
			/* 2B49     DO $2B53 UNTIL CE */
    /* M0 = 0, M1 = 1, M2 = -1 */
    for (ii = 0; ii < 0x0040; ii++) {
      INT16 ax0 , ay0, ax1, ay1, ar;
			/* 2B4A       AX0 = DM(I0,M1) */
      ax0 = *i0++;
			/* 2B4B       AY0 = DM(I2,M0) */
      ay0 = *i2;
			/* 2B4C       AR = AX0 + AY0, AX1 = DM(I0,M2) */
      ax1 = *i0--;
      ar = ax0 + ay0;
			/* 2B4D       AR = AX0 - AY0, DM(I0,M1) = AR */
      *i0++ = ar;
      ar = ax0 - ay0;
			/* 2B4E       DM(I2,M1) = AR */
      *i2++ = ar;
			/* 2B4F       AY1 = DM(I2,M0) */
      ay1 = *i2;
			/* 2B50       AR = AX1 + AY1 */
      ar = ax1 + ay1;
			/* 2B51       DM(I0,M1) = AR */
      *i0++ = ar;
			/* 2B52       AR = AX1 - AY1 */
      ar = ax1 - ay1;
  			/* 2B53       DM(I2,M1) = AR */
      *i2++ = ar;
    }
  }
  {
    int mem63d, mem63e, mem63f;
    int jj,kk;
  			/* 2B54     AR = $0002 */
			/* 2B55     DM($15EB) = AR (063d) */
    mem63d = 2;
			/* 2B56     SI = $0040 */
			/* 2B57     DM($15ED) = SI (063e) */
    mem63e = 0x40;
			/* 2B58     SR = LSHIFT SI BY -1 (LO) */
			/* 2B59     DM($15EF) = SR0 (063f) */
    mem63f = mem63e >> 1;
			/* 2B5A     M0 = $3FFF */
			/* 2B5B     CNTR = $0006 */
			/* 2B5C     DO $2B80 UNTIL CE */

    /* M0 = -1, M1 = 1, M5 = 1 */
    for (ii = 0; ii < 6; ii++) {
      UINT16 *i0, *i1, *i2, *i4, *i5;
      INT16 m2, m3;
			/* 2B5D       I4 = $1080 >>> (0780) <<< */
      i4 = &ram1source[0x0080];
			/* 2B5E       I5 = $1000 >>> (0700) <<< */
      i5 = &ram1source[0x0000];
			/* 2B5F       I0 = $2000 >>> (3800) <<< */
      i0 = &ram2source[0x0000];
			/* 2B60       I1 = $2000 >>> (3800) <<< */
      i1 = &ram2source[0x0000];
			/* 2B61       AY0 = DM($15ED) (063e) */
			/* 2B62       M2 = AY0 */
      m2 = mem63e;
			/* 2B63       MODIFY (I1,M2) */
      i1 += m2;
			/* 2B64       I2 = I1 */
      i2 = i1;
			/* 2B65       AR = AY0 - 1 */
			/* 2B66       M3 = AR */
      m3 = mem63e - 1;
			/* 2B67       CNTR = DM($15EB) (063d) */
			/* 2B68       DO $2B79 UNTIL CE */

      for (jj = 0; jj < mem63d; jj++) {
        INT16 mx0, mx1, my0, my1;
			/* 2B6A         MY0 = DM(I4,M5) */
        my0 = *i4++;
			/* 2B6B         MY1 = DM(I5,M5) */
        my1 = *i5++;
			/* 2B6C         MX0 = DM(I1,M1) */
        mx0 = *i1++;
			/* 2B69         CNTR = DM($15EF) (063f) */
			/* 2B6D         DO $2B76 UNTIL CE */
        for (kk = 0; kk < mem63f; kk++) {
          INT16 ax0, ay0, ay1, ar;
          INT32 tmp, mr;
			/* 2B6E           MR = MX0 * MY0 (SS), MX1 = DM(I1,M1) */
          mx1 = *i1++;
          tmp = ((mx0 * my0)<<1);
          mr = tmp;
			/* 2B6F           MR = MR - MX1 * MY1 (RND), AY0 = DM(I0,M1) */
          ay0 = *i0++;
          tmp = ((mx1 * my1)<<1);
          mr = (mr - tmp + 0x8000) & (((tmp & 0xffff) == 0x8000) ? 0xfffeffff : 0xffffffff);
			/* 2B70           MR = MX1 * MY0 (SS), AX0 = MR1 */
          ax0 = mr>>16;
          tmp = (mx1 * my0)<<1;
          mr = tmp;
			/* 2B71           MR = MR + MX0 * MY1 (RND), AY1 = DM(I0,M0) */
          ay1 = *i0--; /* M0 = -1 */
          tmp = ((mx0 * my1)<<1);
          mr = (mr + tmp + 0x8000) & (((tmp & 0xffff) == 0x8000) ? 0xfffeffff : 0xffffffff);
			/* 2B72           AR = AY0 - AX0, MX0 = DM(I1,M1) */
          mx0 = *i1++;
          ar = ay0 - ax0;
			/* 2B73           AR = AX0 + AY0, DM(I0,M1) = AR */
          *i0++ = ar;
          ar = ax0 + ay0;
			/* 2B74           AR = AY1 - MR1, DM(I2,M1) = AR */
          *i2++ = ar;
          ar = ay1 - (mr>>16);
			/* 2B75           AR = MR1 + AY1, DM(I0,M1) = AR */
          *i0++ = ar;
          ar = (mr>>16) + ay1;
			/* 2B76           DM(I2,M1) = AR */
          *i2++ = ar;
        }
			/* 2B77         MODIFY (I2,M2) */
        i2 += m2;
			/* 2B78         MODIFY (I1,M3) */
        i1 += m3;
			/* 2B79         MODIFY (I0,M2) */
        i0 += m2;
      }
			/* 2B7A       SI = DM($15EB) (063d) */
			/* 2B7B:      SR = LSHIFT SI BY 1 (LO) */
			/* 2B7C:      DM($15EB) = SR0 (063d) */
      mem63d <<= 1;
			/* 2B7D       SI = DM($15EF) (063f) */
			/* 2B7E       DM($15ED) = SI (063e) */
      mem63e = mem63f;
			/* 2B7F       SR = LSHIFT SI BY -1 (LO) */
			/* 2B80       DM($15EF) = SR0 (063f) */
      mem63f >>= 1;
    }
  }
  { /* Volume scaling */
    UINT16 *i0;
    UINT16 my0;
			/* 2B81     M0 = $0000 */
			/* 2B82     I0 = $2000 >>> (3800) <<< */
    i0 = &ram2source[0x0000];
			/* 2B84     MY0 = DM($15FD) (390e) */
    my0 = volume;
			/* 2B83     CNTR = $0100 */
			/* 2B85     DO $2B89 UNTIL CE */
    /* M0 = 0, M1 = 1 */
    for (ii = 0; ii < 0x0100; ii++) {
      INT16 mx0;
      INT32 mr;
			/* 2B86       MX0 = DM(I0,M0) */
      mx0 = *i0;
			/* 2B87       MR = MX0 * MY0 (SU) */
      mr = (mx0 * my0)<<1;
			/* 2B88       IF MV SAT MR */
      /* This instruction limits MR to 32 bits */
      /* In reality the volume will never be higher than 0x8000 so */
      /* this is not needed */
			/* 2B89       DM(I0,M1) = MR1 */
      *i0++ = mr>>16;
    }
  }
  activecpu_set_reg(ADSP2100_PC, pc + 0x2b89 - 0x2b44);
  return 0; /* execute a NOP */
}

#endif /* WPCDCSSPEEDUP */

#endif

