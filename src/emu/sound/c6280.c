/*
    HuC6280 sound chip emulator
    by Charles MacDonald
    E-mail: cgfm2@hotmail.com
    WWW: http://cgfm2.emuviews.com

    Thanks to:

    - Paul Clifford for his PSG documentation.
    - Richard Bannister for the TGEmu-specific sound updating code.
    - http://www.uspto.gov for the PSG patents.
    - All contributors to the tghack-list.

    Changes:

    (03/30/2003)
    - Removed TGEmu specific code and added support functions for MAME.
    - Modified setup code to handle multiple chips with different clock and
      volume settings.

    Missing features / things to do:

    - Add LFO support. But do any games actually use it?

    - Add shared index for waveform playback and sample writes. Almost every
      game will reset the index prior to playback so this isn't an issue.

    - While the noise emulation is complete, the data for the pseudo-random
      bitstream is calculated by machine.rand() and is not a representation of what
      the actual hardware does.

    For some background on Hudson Soft's C62 chipset:

    - http://www.hudsonsoft.net/ww/about/about.html
    - http://www.hudson.co.jp/corp/eng/coinfo/history.html

    Legal information:

    Copyright Charles MacDonald

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "emu.h"
#include "c6280.h"

struct t_channel {
    UINT16 frequency;
    UINT8 control;
    UINT8 balance;
    UINT8 waveform[32];
    UINT8 index;
    INT16 dda;
    UINT8 noise_control;
    UINT32 noise_counter;
    UINT32 counter;
};

struct c6280_t {
	sound_stream *stream;
	device_t *device;
	device_t *cpudevice;
    UINT8 select;
    UINT8 balance;
    UINT8 lfo_frequency;
    UINT8 lfo_control;
    t_channel channel[8];
    INT16 volume_table[32];
    UINT32 noise_freq_tab[32];
    UINT32 wave_freq_tab[4096];
};

INLINE c6280_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == C6280);
	return (c6280_t *)downcast<c6280_device *>(device)->token();
}


/* only needed for io_buffer */
#include "cpu/h6280/h6280.h"


static void c6280_init(device_t *device, c6280_t *p, double clk, double rate)
{
	const c6280_interface *intf = (const c6280_interface *)device->static_config();
    int i;
    double step;

    /* Loudest volume level for table */
    double level = 65535.0 / 6.0 / 32.0;

    /* Clear context */
    memset(p, 0, sizeof(c6280_t));

    p->device = device;
    p->cpudevice = device->machine().device(intf->cpu);
    if (p->cpudevice == NULL)
    	fatalerror("c6280_init: no CPU found with tag of '%s'\n", device->tag());

    /* Make waveform frequency table */
    for(i = 0; i < 4096; i += 1)
    {
        step = ((clk / rate) * 4096) / (i+1);
        p->wave_freq_tab[(1 + i) & 0xFFF] = (UINT32)step;
    }

    /* Make noise frequency table */
    for(i = 0; i < 32; i += 1)
    {
        step = ((clk / rate) * 32) / (i+1);
        p->noise_freq_tab[i] = (UINT32)step;
    }

    /* Make volume table */
    /* PSG has 48dB volume range spread over 32 steps */
    step = 48.0 / 32.0;
    for(i = 0; i < 31; i++)
    {
        p->volume_table[i] = (UINT16)level;
        level /= pow(10.0, step / 20.0);
    }
    p->volume_table[31] = 0;
}


static void c6280_write(c6280_t *p, int offset, int data)
{
    t_channel *q = &p->channel[p->select];

    /* Update stream */
    p->stream->update();

    switch(offset & 0x0F)
    {
        case 0x00: /* Channel select */
            p->select = data & 0x07;
            break;

        case 0x01: /* Global balance */
            p->balance  = data;
            break;

        case 0x02: /* Channel frequency (LSB) */
            q->frequency = (q->frequency & 0x0F00) | data;
            q->frequency &= 0x0FFF;
            break;

        case 0x03: /* Channel frequency (MSB) */
            q->frequency = (q->frequency & 0x00FF) | (data << 8);
            q->frequency &= 0x0FFF;
            break;

        case 0x04: /* Channel control (key-on, DDA mode, volume) */

            /* 1-to-0 transition of DDA bit resets waveform index */
            if((q->control & 0x40) && ((data & 0x40) == 0))
            {
                q->index = 0;
            }
            q->control = data;
            break;

        case 0x05: /* Channel balance */
            q->balance = data;
            break;

        case 0x06: /* Channel waveform data */

            switch(q->control & 0xC0)
            {
                case 0x00:
                    q->waveform[q->index & 0x1F] = data & 0x1F;
                    q->index = (q->index + 1) & 0x1F;
                    break;

                case 0x40:
                    break;

                case 0x80:
                    q->waveform[q->index & 0x1F] = data & 0x1F;
                    q->index = (q->index + 1) & 0x1F;
                    break;

                case 0xC0:
                    q->dda = data & 0x1F;
                    break;
            }

            break;

        case 0x07: /* Noise control (enable, frequency) */
            q->noise_control = data;
            break;

        case 0x08: /* LFO frequency */
            p->lfo_frequency = data;
            break;

        case 0x09: /* LFO control (enable, mode) */
            p->lfo_control = data;
            break;

        default:
            break;
    }
}


static STREAM_UPDATE( c6280_update )
{
    static const int scale_tab[] = {
        0x00, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F,
        0x10, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F
    };
    int ch;
    int i;
    c6280_t *p = (c6280_t *)param;

    int lmal = (p->balance >> 4) & 0x0F;
    int rmal = (p->balance >> 0) & 0x0F;
    int vll, vlr;

    lmal = scale_tab[lmal];
    rmal = scale_tab[rmal];

    /* Clear buffer */
    for(i = 0; i < samples; i++)
    {
        outputs[0][i] = 0;
        outputs[1][i] = 0;
    }

    for(ch = 0; ch < 6; ch++)
    {
        /* Only look at enabled channels */
        if(p->channel[ch].control & 0x80)
        {
            int lal = (p->channel[ch].balance >> 4) & 0x0F;
            int ral = (p->channel[ch].balance >> 0) & 0x0F;
            int al  = p->channel[ch].control & 0x1F;

            lal = scale_tab[lal];
            ral = scale_tab[ral];

            /* Calculate volume just as the patent says */
            vll = (0x1F - lal) + (0x1F - al) + (0x1F - lmal);
            if(vll > 0x1F) vll = 0x1F;

            vlr = (0x1F - ral) + (0x1F - al) + (0x1F - rmal);
            if(vlr > 0x1F) vlr = 0x1F;

            vll = p->volume_table[vll];
            vlr = p->volume_table[vlr];

            /* Check channel mode */
            if((ch >= 4) && (p->channel[ch].noise_control & 0x80))
            {
                /* Noise mode */
                UINT32 step = p->noise_freq_tab[(p->channel[ch].noise_control & 0x1F) ^ 0x1F];
                for(i = 0; i < samples; i += 1)
                {
                    static int data = 0;
                    p->channel[ch].noise_counter += step;
                    if(p->channel[ch].noise_counter >= 0x800)
                    {
                        data = (p->device->machine().rand() & 1) ? 0x1F : 0;
                    }
                    p->channel[ch].noise_counter &= 0x7FF;
                    outputs[0][i] += (INT16)(vll * (data - 16));
                    outputs[1][i] += (INT16)(vlr * (data - 16));
                }
            }
            else
            if(p->channel[ch].control & 0x40)
            {
                /* DDA mode */
                for(i = 0; i < samples; i++)
                {
                    outputs[0][i] += (INT16)(vll * (p->channel[ch].dda - 16));
                    outputs[1][i] += (INT16)(vlr * (p->channel[ch].dda - 16));
                }
            }
            else
            {
                /* Waveform mode */
                UINT32 step = p->wave_freq_tab[p->channel[ch].frequency];
                for(i = 0; i < samples; i += 1)
                {
                    int offset;
                    INT16 data;
                    offset = (p->channel[ch].counter >> 12) & 0x1F;
                    p->channel[ch].counter += step;
                    p->channel[ch].counter &= 0x1FFFF;
                    data = p->channel[ch].waveform[offset];
                    outputs[0][i] += (INT16)(vll * (data - 16));
                    outputs[1][i] += (INT16)(vlr * (data - 16));
                }
            }
        }
    }
}


/*--------------------------------------------------------------------------*/
/* MAME specific code                                                       */
/*--------------------------------------------------------------------------*/

static DEVICE_START( c6280 )
{
    int rate = device->clock()/16;
    c6280_t *info = get_safe_token(device);

    /* Initialize PSG emulator */
    c6280_init(device, info, device->clock(), rate);

    /* Create stereo stream */
    info->stream = device->machine().sound().stream_alloc(*device, 0, 2, rate, info, c6280_update);
}

READ8_DEVICE_HANDLER( c6280_r )
{
    c6280_t *info = get_safe_token(device);
	return h6280io_get_buffer(info->cpudevice);
}

WRITE8_DEVICE_HANDLER( c6280_w )
{
    c6280_t *info = get_safe_token(device);
	h6280io_set_buffer(info->cpudevice, data);
	c6280_write(info, offset, data);
}

const device_type C6280 = &device_creator<c6280_device>;

c6280_device::c6280_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, C6280, "HuC6280", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(c6280_t);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void c6280_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c6280_device::device_start()
{
	DEVICE_START_NAME( c6280 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void c6280_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


