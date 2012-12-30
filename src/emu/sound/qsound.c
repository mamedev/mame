/***************************************************************************

  Capcom System QSound(tm)
  ========================

  Driver by Paul Leaman (paul@vortexcomputing.demon.co.uk)
        and Miguel Angel Horna (mahorna@teleline.es)

  A 16 channel stereo sample player.

  QSpace position is simulated by panning the sound in the stereo space.

  Register
  0  xxbb   xx = unknown bb = start high address
  1  ssss   ssss = sample start address
  2  pitch
  3  unknown (always 0x8000)
  4  loop offset from end address
  5  end
  6  master channel volume
  7  not used
  8  Balance (left=0x0110  centre=0x0120 right=0x0130)
  9  unknown (most fixed samples use 0 for this register)

  Many thanks to CAB (the author of Amuse), without whom this probably would
  never have been finished.

  If anybody has some information about this hardware, please send it to me
  to mahorna@teleline.es or 432937@cepsz.unizar.es.
  http://teleline.terra.es/personal/mahorna

***************************************************************************/

#include "emu.h"
#include "qsound.h"

// Debug defines
#define LOG_WAVE 0
#define VERBOSE  0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


// device type definition
const device_type QSOUND = &device_creator<qsound_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  qsound_device - constructor
//-------------------------------------------------

qsound_device::qsound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, QSOUND, "Q-Sound", tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_data(0),
	  m_stream(NULL),
	  m_sample_rom_length(0),
	  m_sample_rom(NULL),
	  m_frq_ratio(0.0f),
	  m_fpRawDataL(NULL),
	  m_fpRawDataR(NULL)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qsound_device::device_start()
{
	int i;

	m_sample_rom = (QSOUND_SRC_SAMPLE *)*region();
	m_sample_rom_length = region()->bytes();

	memset(m_channel, 0, sizeof(m_channel));

	m_frq_ratio = 16.0;

	/* Create pan table */
	for (i=0; i<33; i++)
	{
		m_pan_table[i]=(int)((256/sqrt(32.0)) * sqrt((double)i));
	}

	LOG(("Pan table\n"));
	for (i=0; i<33; i++)
		LOG(("%02x ", m_pan_table[i]));

	/* Allocate stream */
	m_stream = stream_alloc(0, 2, clock() / QSOUND_CLOCKDIV);

	if (LOG_WAVE)
	{
		m_fpRawDataR=fopen("qsoundr.raw", "w+b");
		m_fpRawDataL=fopen("qsoundl.raw", "w+b");
	}

	/* state save */
	for (i=0; i<QSOUND_CHANNELS; i++)
	{
		save_item(NAME(m_channel[i].bank), i);
		save_item(NAME(m_channel[i].address), i);
		save_item(NAME(m_channel[i].pitch), i);
		save_item(NAME(m_channel[i].loop), i);
		save_item(NAME(m_channel[i].end), i);
		save_item(NAME(m_channel[i].vol), i);
		save_item(NAME(m_channel[i].pan), i);
		save_item(NAME(m_channel[i].key), i);
		save_item(NAME(m_channel[i].lvol), i);
		save_item(NAME(m_channel[i].rvol), i);
		save_item(NAME(m_channel[i].lastdt), i);
		save_item(NAME(m_channel[i].offset), i);
	}
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void qsound_device::device_stop()
{
	if (m_fpRawDataR)
	{
		fclose(m_fpRawDataR);
	}
	m_fpRawDataR = NULL;
	if (m_fpRawDataL)
	{
		fclose(m_fpRawDataL);
	}
	m_fpRawDataL = NULL;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void qsound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int i,j;
	int rvol, lvol, count;
	struct QSOUND_CHANNEL *pC=&m_channel[0];
	stream_sample_t  *datap[2];

	datap[0] = outputs[0];
	datap[1] = outputs[1];
	memset( datap[0], 0x00, samples * sizeof(*datap[0]) );
	memset( datap[1], 0x00, samples * sizeof(*datap[1]) );

	for (i=0; i<QSOUND_CHANNELS; i++)
	{
		if (pC->key)
		{
			QSOUND_SAMPLE *pOutL=datap[0];
			QSOUND_SAMPLE *pOutR=datap[1];
			rvol=(pC->rvol*pC->vol)>>8;
			lvol=(pC->lvol*pC->vol)>>8;

			for (j=samples-1; j>=0; j--)
			{
				count=(pC->offset)>>16;
				pC->offset &= 0xffff;
				if (count)
				{
					pC->address += count;
					if (pC->address >= pC->end)
					{
						if (!pC->loop)
						{
							/* Reached the end of a non-looped sample */
							pC->key=0;
							break;
						}
						/* Reached the end, restart the loop */
						pC->address = (pC->end - pC->loop) & 0xffff;
					}
					pC->lastdt=m_sample_rom[(pC->bank+pC->address)%(m_sample_rom_length)];
				}

				(*pOutL) += ((pC->lastdt * lvol) >> 6);
				(*pOutR) += ((pC->lastdt * rvol) >> 6);
				pOutL++;
				pOutR++;
				pC->offset += pC->pitch;
			}
		}
		pC++;
	}

	if (m_fpRawDataL)
		fwrite(datap[0], samples*sizeof(QSOUND_SAMPLE), 1, m_fpRawDataL);
	if (m_fpRawDataR)
		fwrite(datap[1], samples*sizeof(QSOUND_SAMPLE), 1, m_fpRawDataR);
}


WRITE8_MEMBER( qsound_device::qsound_w )
{
	switch (offset)
	{
		case 0:
			m_data=(m_data&0xff)|(data<<8);
			break;

		case 1:
			m_data=(m_data&0xff00)|data;
			break;

		case 2:
			qsound_set_command(data, m_data);
			break;

		default:
			logerror("%s: unexpected qsound write to offset %d == %02X\n", machine().describe_context(), offset, data);
			break;
	}
}


READ8_MEMBER( qsound_device::qsound_r )
{
	/* Port ready bit (0x80 if ready) */
	return 0x80;
}


void qsound_device::qsound_set_command(int data, int value)
{
	int ch=0,reg=0;
	if (data < 0x80)
	{
		ch=data>>3;
		reg=data & 0x07;
	}
	else
	{
		if (data < 0x90)
		{
			ch=data-0x80;
			reg=8;
		}
		else
		{
			if (data >= 0xba && data < 0xca)
			{
				ch=data-0xba;
				reg=9;
			}
			else
			{
				/* Unknown registers */
				ch=99;
				reg=99;
			}
		}
	}

	switch (reg)
	{
		case 0: /* Bank */
			ch=(ch+1)&0x0f;	/* strange ... */
			m_channel[ch].bank=(value&0x7f)<<16;
#ifdef MAME_DEBUG
			if (!(value & 0x8000))
				popmessage("Register3=%04x",value);
#endif

			break;
		case 1: /* start */
			m_channel[ch].address=value;
			break;
		case 2: /* pitch */
			m_channel[ch].pitch=value * 16;
			if (!value)
			{
				/* Key off */
				m_channel[ch].key=0;
			}
			break;
		case 3: /* unknown */
			m_channel[ch].reg3=value;
#ifdef MAME_DEBUG
			if (value != 0x8000)
				popmessage("Register3=%04x",value);
#endif
			break;
		case 4: /* loop offset */
			m_channel[ch].loop=value;
			break;
		case 5: /* end */
			m_channel[ch].end=value;
			break;
		case 6: /* master volume */
			if (value==0)
			{
				/* Key off */
				m_channel[ch].key=0;
			}
			else if (m_channel[ch].key==0)
			{
				/* Key on */
				m_channel[ch].key=1;
				m_channel[ch].offset=0;
				m_channel[ch].lastdt=0;
			}
			m_channel[ch].vol=value;
			break;

		case 7:  /* unused */
#ifdef MAME_DEBUG
				popmessage("UNUSED QSOUND REG 7=%04x",value);
#endif

			break;
		case 8:
			{
			   int pandata=(value-0x10)&0x3f;
			   if (pandata > 32)
			   {
					pandata=32;
			   }
			   m_channel[ch].rvol=m_pan_table[pandata];
			   m_channel[ch].lvol=m_pan_table[32-pandata];
			   m_channel[ch].pan = value;
			}
			break;
		 case 9:
			m_channel[ch].reg9=value;
/*
#ifdef MAME_DEBUG
            popmessage("QSOUND REG 9=%04x",value);
#endif
*/
			break;
	}
	LOG(("QSOUND WRITE %02x CH%02d-R%02d =%04x\n", data, ch, reg, value));
}
