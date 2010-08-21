/***************************************************************************

    mconfig.c

    Machine configuration macros and functions.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include <ctype.h>


//**************************************************************************
//  MACHINE CONFIGURATIONS
//**************************************************************************

//-------------------------------------------------
//  machine_config - constructor
//-------------------------------------------------

machine_config::machine_config(const machine_config_token *tokens)
	: m_driver_data_alloc(NULL),
	  m_minimum_quantum(attotime_zero),
	  m_perfect_cpu_quantum(NULL),
	  m_watchdog_vblank_count(0),
	  m_watchdog_time(attotime_zero),
	  m_machine_start(NULL),
	  m_machine_reset(NULL),
	  m_nvram_handler(NULL),
	  m_memcard_handler(NULL),
	  m_video_attributes(0),
	  m_gfxdecodeinfo(NULL),
	  m_total_colors(0),
	  m_default_layout(NULL),
	  m_init_palette(NULL),
	  m_video_start(NULL),
	  m_video_reset(NULL),
	  m_video_eof(NULL),
	  m_video_update(NULL),
	  m_sound_start(NULL),
	  m_sound_reset(NULL),
	  m_parse_level(0)
{
	// parse tokens into the config
	detokenize(tokens);
}


//-------------------------------------------------
//  ~machine_config - destructor
//-------------------------------------------------

machine_config::~machine_config()
{
}


//-------------------------------------------------
//  detokenize - detokenize a machine config
//-------------------------------------------------

void machine_config::detokenize(const machine_config_token *tokens, const device_config *owner)
{
	device_config *device = NULL;
	astring tempstring;

	// increase the parse level
	m_parse_level++;

	// loop over tokens until we hit the end
	UINT32 entrytype = MCONFIG_TOKEN_INVALID;
	while (entrytype != MCONFIG_TOKEN_END)
	{
		device_type devtype;
		const char *tag;
		UINT64 data64;
		UINT32 clock;

		// unpack the token from the first entry
		TOKEN_GET_UINT32_UNPACK1(tokens, entrytype, 8);
		switch (entrytype)
		{
			// end
			case MCONFIG_TOKEN_END:
				break;

			// including
			case MCONFIG_TOKEN_INCLUDE:
				detokenize(TOKEN_GET_PTR(tokens, tokenptr), owner);
				break;

			// device management
			case MCONFIG_TOKEN_DEVICE_ADD:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, clock, 32);
				devtype = TOKEN_GET_PTR(tokens, devtype);
				tag = owner->subtag(tempstring, TOKEN_GET_STRING(tokens));
				device = m_devicelist.append(tag, (*devtype)(*this, tag, owner, clock));
				break;

			case MCONFIG_TOKEN_DEVICE_REPLACE:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, clock, 32);
				devtype = TOKEN_GET_PTR(tokens, devtype);
				tag = owner->subtag(tempstring, TOKEN_GET_STRING(tokens));
				device = m_devicelist.replace(tag, (*devtype)(*this, tag, owner, clock));
				break;

			case MCONFIG_TOKEN_DEVICE_REMOVE:
				tag = TOKEN_GET_STRING(tokens);
				m_devicelist.remove(owner->subtag(tempstring, tag));
				device = NULL;
				break;

			case MCONFIG_TOKEN_DEVICE_MODIFY:
				tag = TOKEN_GET_STRING(tokens);
				device = m_devicelist.find(owner->subtag(tempstring, tag).cstr());
				if (device == NULL)
					fatalerror("Unable to find device: tag=%s\n", tempstring.cstr());
				break;

			case MCONFIG_TOKEN_DEVICE_CLOCK:
			case MCONFIG_TOKEN_DEVICE_CONFIG:
			case MCONFIG_TOKEN_DEVICE_INLINE_DATA16:
			case MCONFIG_TOKEN_DEVICE_INLINE_DATA32:
			case MCONFIG_TOKEN_DEVICE_INLINE_DATA64:

			case MCONFIG_TOKEN_DIEXEC_DISABLE:
			case MCONFIG_TOKEN_DIEXEC_VBLANK_INT:
			case MCONFIG_TOKEN_DIEXEC_PERIODIC_INT:

			case MCONFIG_TOKEN_DIMEMORY_MAP:

			case MCONFIG_TOKEN_DISOUND_ROUTE:
			case MCONFIG_TOKEN_DISOUND_RESET:

			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_1:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_2:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_3:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_4:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_5:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_6:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_7:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_8:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_9:
			case MCONFIG_TOKEN_DEVICE_CONFIG_DATA32:
			case MCONFIG_TOKEN_DEVICE_CONFIG_DATA64:
			case MCONFIG_TOKEN_DEVICE_CONFIG_DATAFP32:
			case MCONFIG_TOKEN_DEVICE_CONFIG_CUSTOM_FREE:
				assert(device != NULL);
				device->process_token(entrytype, tokens);
				break;


			// core parameters
			case MCONFIG_TOKEN_DRIVER_DATA:
				m_driver_data_alloc = TOKEN_GET_PTR(tokens, driver_data_alloc);
				break;

			case MCONFIG_TOKEN_QUANTUM_TIME:
				TOKEN_EXTRACT_UINT64(tokens, data64);
				m_minimum_quantum = UINT64_ATTOTIME_TO_ATTOTIME(data64);
				break;

			case MCONFIG_TOKEN_QUANTUM_PERFECT_CPU:
				m_perfect_cpu_quantum = TOKEN_GET_STRING(tokens);
				break;

			case MCONFIG_TOKEN_WATCHDOG_VBLANK:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, m_watchdog_vblank_count, 24);
				break;

			case MCONFIG_TOKEN_WATCHDOG_TIME:
				TOKEN_EXTRACT_UINT64(tokens, data64);
				m_watchdog_time = UINT64_ATTOTIME_TO_ATTOTIME(data64);
				break;

			// core functions
			case MCONFIG_TOKEN_MACHINE_START:
				m_machine_start = TOKEN_GET_PTR(tokens, machine_start);
				break;

			case MCONFIG_TOKEN_MACHINE_RESET:
				m_machine_reset = TOKEN_GET_PTR(tokens, machine_reset);
				break;

			case MCONFIG_TOKEN_NVRAM_HANDLER:
				m_nvram_handler = TOKEN_GET_PTR(tokens, nvram_handler);
				break;

			case MCONFIG_TOKEN_MEMCARD_HANDLER:
				m_memcard_handler = TOKEN_GET_PTR(tokens, memcard_handler);
				break;

			// core video parameters
			case MCONFIG_TOKEN_VIDEO_ATTRIBUTES:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, m_video_attributes, 24);
				break;

			case MCONFIG_TOKEN_GFXDECODE:
				m_gfxdecodeinfo = TOKEN_GET_PTR(tokens, gfxdecode);
				break;

			case MCONFIG_TOKEN_PALETTE_LENGTH:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, m_total_colors, 24);
				break;

			case MCONFIG_TOKEN_DEFAULT_LAYOUT:
				m_default_layout = TOKEN_GET_STRING(tokens);
				break;

			// core video functions
			case MCONFIG_TOKEN_PALETTE_INIT:
				m_init_palette = TOKEN_GET_PTR(tokens, palette_init);
				break;

			case MCONFIG_TOKEN_VIDEO_START:
				m_video_start = TOKEN_GET_PTR(tokens, video_start);
				break;

			case MCONFIG_TOKEN_VIDEO_RESET:
				m_video_reset = TOKEN_GET_PTR(tokens, video_reset);
				break;

			case MCONFIG_TOKEN_VIDEO_EOF:
				m_video_eof = TOKEN_GET_PTR(tokens, video_eof);
				break;

			case MCONFIG_TOKEN_VIDEO_UPDATE:
				m_video_update = TOKEN_GET_PTR(tokens, video_update);
				break;

			// core sound functions
			case MCONFIG_TOKEN_SOUND_START:
				m_sound_start = TOKEN_GET_PTR(tokens, sound_start);
				break;

			case MCONFIG_TOKEN_SOUND_RESET:
				m_sound_reset = TOKEN_GET_PTR(tokens, sound_reset);
				break;

			default:
				fatalerror("Invalid token %d in machine config\n", entrytype);
				break;
		}
	}

	// if we started at parse level 0 (and are thus at level 1), do post-processing
	if (m_parse_level == 1)
	{
		// process any device-specific machine configurations
		for (const device_config *devconfig = m_devicelist.first(); devconfig != NULL; devconfig = devconfig->next())
			if (!devconfig->m_config_complete)
			{
				tokens = devconfig->machine_config_tokens();
				if (tokens != NULL)
					detokenize(tokens, devconfig);
			}

		// then notify all devices that their configuration is complete
		for (device_config *devconfig = m_devicelist.first(); devconfig != NULL; devconfig = devconfig->next())
			if (!devconfig->m_config_complete)
			{
				devconfig->config_complete();
				devconfig->m_config_complete = true;
			}
	}

	// bump down the parse level
	m_parse_level--;
}
