// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/************************************
      Seta PCM emulation
      sound emulation by Tomasz Slanina
      based on ST-0016 emulation

      used by
	  - ST-0026 NiLe (srmp6, 8 voices)
	  - ST-0032 (jclub2, 16 voices)
	  - Capcom CPS3 sound hardware has similarity?

      Register Format (32 byte per voices)

      00-1f: Voice 0
      Offset Bit               Description
             fedcba98 76543210 
      04     xxxxxxxx xxxxxxxx Start position LSB
      06     xxxxxxxx xxxxxxxx Start position MSB
      0a     -------- ----xxx- Used but unknown
             -------- -----x-- See below for NiLe specific? notes
             -------- -------x Loop enable
      0c     xxxxxxxx xxxxxxxx Frequency
      0e     xxxxxxxx xxxxxxxx Loop Start position LSB
      12     xxxxxxxx xxxxxxxx Loop Start position MSB
      14     xxxxxxxx xxxxxxxx Loop End position LSB
      16     xxxxxxxx xxxxxxxx Loop End position MSB
      18     xxxxxxxx xxxxxxxx End position LSB
      1a     xxxxxxxx xxxxxxxx End position MSB
      1c     xxxxxxxx xxxxxxxx Right Volume
      1e     xxxxxxxx xxxxxxxx Left Volume

      20-3f: Voice 1
      ...
	  e0-ff: Voice 7

      100: Keyon/off, Bit 0-7 means Voice 0-7
      110: Used but unknown

      below for 16 voice configurations:
      100-11f: Voice 8
      120-13f: Voice 9
      ...
      1e0-1ff: Voice 15

      200: Keyon/off, Bit 0-15 means Voice 0-15
      210: Used but unknown

      Other registers are unknown/unused

      TODO:
      - Verify loop and flag bit behavior from real hardware

************************************/

#include "emu.h"
#include "setapcm.h"

template class setapcm_device<8, 371>;
template class setapcm_device<16, 384>;

template<unsigned MaxVoices, unsigned Divider>
constexpr unsigned setapcm_device<MaxVoices, Divider>::MAX_VOICES;
template<unsigned MaxVoices, unsigned Divider>
constexpr unsigned setapcm_device<MaxVoices, Divider>::CLOCK_DIVIDER;

DEFINE_DEVICE_TYPE(NILE_SOUND,   nile_sound_device,   "nile_sound",   "Seta ST-0026 NiLe (Sound)")
DEFINE_DEVICE_TYPE(ST0032_SOUND, st0032_sound_device, "st0032_sound", "Seta ST-0032 (Sound)")
