#include "emu.h"
#include "nmk004.h"

#define FM_FLAG_NEED_INITIALIZATION      (1<<0)
#define FM_FLAG_UNKNOWN2                 (1<<1)
#define FM_FLAG_NOTE_IS_PAUSE            (1<<2)
#define FM_FLAG_UNKNOWN3                 (1<<3)
#define FM_FLAG_MODULATE_NOTE            (1<<4)
#define FM_FLAG_MUST_SEND_KEYON          (1<<5)
#define FM_FLAG_MUST_SEND_CONFIGURATION  (1<<6)
#define FM_FLAG_ACTIVE                   (1<<7)

#define PSG_FLAG_ACTIVE                  (1<<0)
#define PSG_FLAG_NOTE_IS_PAUSE           (1<<1)
#define PSG_FLAG_NEED_INITIALIZATION     (1<<2)
#define PSG_FLAG_INITIALIZE_VOLUME       (1<<3)
#define PSG_FLAG_NOTE_IS_NOISE           (1<<5)
#define PSG_FLAG_NOISE_NOT_ENABLED       (1<<6)

#define EFFECTS_FLAG_NEED_INITIALIZATION (1<<0)
#define EFFECTS_FLAG_ACTIVE              (1<<7)

#define NOTE_PAUSE             0x0c

#define SAMPLE_TABLE_0      0xefe0
#define SAMPLE_TABLE_1      0xefe2
#define FM_MODULATION_TABLE 0xefe4
#define FM_NOTE_TABLE       0xefe6
#define NOTE_LENGTH_TABLE_1 0xefe8
#define NOTE_LENGTH_TABLE_2 0xefea
#define NOTE_LENGTH_TABLE_3 0xefec
#define PSG_VOLUME_TABLE    0xefee
#define COMMAND_TABLE       0xeff0
#define PSG_NOTE_TABLE      0xeff2


const device_type NMK004 = &device_creator<nmk004_device>;

nmk004_device::nmk004_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NMK004, "NMK004", tag, owner, clock, "nmk004", __FILE__),
	m_rom(NULL),
	m_from_main(0),
	m_to_main(0),
	m_protection_check(0),
	m_last_command(0),
	m_oki_playing(0)
{
	memset(m_fm_control, 0, sizeof(m_fm_control));
	memset(m_psg_control, 0, sizeof(m_psg_control));
	memset(m_effects_control, 0, sizeof(m_effects_control));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void nmk004_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nmk004_device::device_start()
{
	/* we have to do this via a timer because we get called before the sound reset */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(nmk004_device::real_init), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nmk004_device::device_reset()
{
}

UINT8 nmk004_device::read8(int address)
{
	return m_rom[address];
}

UINT16 nmk004_device::read16(int address)
{
	return m_rom[address] + 256 * m_rom[address+1];
}



/*****************************

          OKI6295

*****************************/

void nmk004_device::oki_play_sample(int sample_no)
{
	UINT16 table_start = (sample_no & 0x80) ? read16(SAMPLE_TABLE_1) : read16(SAMPLE_TABLE_0);
	UINT8 byte1 = read8(table_start + 2 * (sample_no & 0x7f) + 0);
	UINT8 byte2 = read8(table_start + 2 * (sample_no & 0x7f) + 1);
	int chip = (byte1 & 0x80) >> 7;
	okim6295_device *okidevice = (chip) ? m_oki2device : m_oki1device;

	if ((byte1 & 0x7f) == 0)
	{
		// stop all channels
		okidevice->write_command( 0x78 );
	}
	else
	{
		int sample = byte1 & 0x7f;
		int ch = byte2 & 0x03;
		int force = (byte2 & 0x80) >> 7;

		if (!force && (m_oki_playing & (1 << (ch + 4*chip))))
			return;

		m_oki_playing |= 1 << (ch + 4*chip);

		// stop channel
		okidevice->write_command( 0x08 << ch );

		if (sample != 0)
		{
			UINT8 *rom = machine().root_device().memregion((chip == 0) ? "oki1" : "oki2")->base();
			int bank = (byte2 & 0x0c) >> 2;
			int vol = (byte2 & 0x70) >> 4;

			if (bank != 3)
				memcpy(rom + 0x20000,rom + 0x40000 + bank * 0x20000,0x20000);

			okidevice->write_command( 0x80 | sample );
			okidevice->write_command( (0x10 << ch) | vol );
		}
	}
}

void nmk004_device::oki_update_state(void)
{
	m_oki_playing = ((m_oki2device->read_status() & 0x0f) << 4) | (m_oki1device->read_status() & 0x0f);
}



/*****************************

     EFFECTS (OKI6295)

*****************************/

void nmk004_device::effects_update(int channel)
{
	struct effects_control *effects = &m_effects_control[channel];

	// advance the timers
	if (effects->timer)
		effects->timer--;


	if (effects->flags & EFFECTS_FLAG_NEED_INITIALIZATION)
	{
		effects->flags = EFFECTS_FLAG_ACTIVE;
		effects->timer = 0;
	}


	if (effects->flags & EFFECTS_FLAG_ACTIVE)
	{
		if (effects->timer == 0)
		{
			UINT8 token;

			do
			{
//logerror("channel %d address %04x token %02x\n",channel,effects->current,read8(effects->current));
				token = read8(effects->current++);

				if (token == 0x0ef || (token & 0xf0) == 0xf0)
				{
					switch (token)
					{
						case 0xef:  // play sample
							oki_play_sample(read8(effects->current++));
							break;

						case 0xf6:  // jump
							effects->current = read16(effects->current);
							break;

						case 0xf7:  // begin repeat loop
							effects->loop_times = read8(effects->current++);
							effects->loop_start = effects->current;
							break;

						case 0xf8:  // end repeat loop
							if (--effects->loop_times > 0)
								effects->current = effects->loop_start;
							break;

						case 0xf9:  // call subtable
							effects->return_address[effects->return_address_depth++] = effects->current + 2;
							effects->current = read16(effects->current);
							break;

						case 0xfa:  // return from subtable
							effects->current = effects->return_address[--effects->return_address_depth];
							break;

						case 0xfc:  // ??? (hachamf command 04)
							break;

						case 0xfd:  // ??? (hachamf command 04)
							break;

						case 0xff:  // end
							effects->flags = 0; // disable channel
							return;

						default:
							fatalerror("effects channel %d unsupported token %02x\n",channel,token);
					}
				}
			} while (token == 0xef || (token & 0xf0) == 0xf0);

			effects->current--;

			if ((read8(effects->current) & 0x80) == 0)
			{
				if (read8(effects->current++) != 0x0c)
				{
					// this shouldn't happen on the effects channels (but it happens e.g. hachamf command 04)

					logerror("effects channel %d invalid token %02x\n",channel,read8(effects->current));
				}
			}

			// optional note length (otherwise use the same length as the previous one)
			if (read8(effects->current) & 0x80)
			{
				UINT16 table_start = read16(NOTE_LENGTH_TABLE_1);

				effects->timer_duration = read16(table_start + 2 * (read8(effects->current++) & 0x7f));
			}

			effects->timer = effects->timer_duration;
		}
	}
}



/*****************************

        YM2203 - FM

*****************************/

void nmk004_device::fm_update(int channel)
{
	struct fm_control *fm = &m_fm_control[channel];
	address_space &space = machine().firstcpu->space(AS_PROGRAM);

	// advance the timers
	if (fm->timer1)
		fm->timer1--;

	if (fm->timer2)
		fm->timer2--;

	if (fm->modulation_timer)
		fm->modulation_timer--;


	if (fm->flags & FM_FLAG_NEED_INITIALIZATION)
	{
		fm->flags = FM_FLAG_ACTIVE;
		fm->timer1 = 0;
	}


	if (fm->flags & FM_FLAG_ACTIVE)
	{
		if (fm->timer1 == 0)
		{
			UINT8 token;

			do
			{
//logerror("channel %d address %04x token %02x\n",channel,fm->current,read8(fm->current));
				token = read8(fm->current++);

				if (token == 0x0ef || (token & 0xf0) == 0xf0)
				{
					int i;

					switch (token)
					{
//                      case 0xef:  // play sample
//                          oki_play_sample(read8(fm->current++));
//                          break;

						case 0xf0:  // slot (for keyon ym2203 command)
							fm->flags |= FM_FLAG_MUST_SEND_CONFIGURATION;
							fm->slot = read8(fm->current++);
							if (channel < 3 || !(m_fm_control[channel-3].flags & FM_FLAG_ACTIVE))
							{
								m_ymdevice->control_port_w(space, 0, 0x28);   // keyon/off
								m_ymdevice->write_port_w(space, 0, channel % 3);
							}
							break;

						case 0xf1:  // sound shape
							fm->flags |= FM_FLAG_MUST_SEND_CONFIGURATION;
							for (i = 0x00; i < 0x04; i++)
								fm->voice_params[i] = read8(fm->current++);
							break;

						case 0xf2:  // sound shape
							fm->flags |= FM_FLAG_MUST_SEND_CONFIGURATION;
							for (i = 0; i < 4; i++)
								fm->voice_volume[i] = read8(fm->current++);
							break;

						case 0xf3:  // sound shape
							fm->flags |= FM_FLAG_MUST_SEND_CONFIGURATION;
							for (i = 0x08; i < 0x18; i++)
								fm->voice_params[i] = read8(fm->current++);
							break;

						case 0xf4:  // set self-feedback
							fm->flags |= FM_FLAG_MUST_SEND_CONFIGURATION;
							fm->self_feedback = read8(fm->current++);
							break;

						case 0xf5:  // select note duration table
							fm->note_duration_table_select = read8(fm->current++);
							break;

						case 0xf6:  // jump
							fm->current = read16(fm->current);
							break;

						case 0xf7:  // begin repeat loop
							fm->loop_times = read8(fm->current++);
							fm->loop_start = fm->current;
							break;

						case 0xf8:  // end repeat loop
							if (--fm->loop_times > 0)
								fm->current = fm->loop_start;
							break;

						case 0xf9:  // call subtable
							fm->return_address[fm->return_address_depth++] = fm->current + 2;
							fm->current = read16(fm->current);
							break;

						case 0xfa:  // return from subtable
							fm->current = fm->return_address[--fm->return_address_depth];
							break;

						case 0xfb:  // set octave
							fm->octave = read8(fm->current++);
							break;

						case 0xfc:  // ???
							fm->flags |=  FM_FLAG_UNKNOWN2;
							fm->flags |=  FM_FLAG_UNKNOWN3;
							break;

						case 0xfd:  // ???
							fm->flags &= ~FM_FLAG_UNKNOWN2;
							break;

						case 0xfe:  // set note modulation
							fm->modulation_table_number = read8(fm->current++);
							if (fm->modulation_table_number == 0)
							{
								fm->flags &= ~FM_FLAG_MODULATE_NOTE;
							}
							else
							{
								UINT16 table_start = read16(FM_MODULATION_TABLE);

								fm->modulation_table = read16(table_start + 2 * (fm->modulation_table_number - 1));
								fm->modulation_timer = read16(fm->modulation_table);
								fm->modulation_table_position = fm->modulation_table + 2;
								fm->flags |= FM_FLAG_MODULATE_NOTE;
							}
							break;

						case 0xff:  // end
							fm->flags = FM_FLAG_MUST_SEND_CONFIGURATION;    // disable channel
							for (i = 0x04; i < 0x08; i++)
								fm->voice_params[i] = 0x7f;
							for (i = 0x14; i < 0x18; i++)
								fm->voice_params[i] = 0x0f;
							return;

						default:
							fatalerror("fm channel %d unsupported token %02x\n",channel,token);
					}
				}
			} while (token == 0xef || (token & 0xf0) == 0xf0);

			fm->current--;

			if ((read8(fm->current) & 0x80) == 0)
			{
				int note = read8(fm->current++);

fm->note = note;
				if ((note & 0x0f) == NOTE_PAUSE)
					fm->flags |=  FM_FLAG_NOTE_IS_PAUSE;
				else
				{
					UINT16 table_start = read16(FM_NOTE_TABLE);
					UINT16 period = read16(table_start + 2 * (note & 0x0f));
					UINT8 octave = ((fm->octave << 4) + note) & 0xf0;

					fm->flags &= ~FM_FLAG_NOTE_IS_PAUSE;
					fm->note_period = period | (octave << 7);

					fm->must_update_voice_params = 1;
				}
			}

			// optional note length (otherwise use the same length as the previous one)
			if (read8(fm->current) & 0x80)
			{
				UINT16 table_start;
				UINT8 duration = read8(fm->current++) & 0x7f;

				table_start = read16(NOTE_LENGTH_TABLE_1);
				fm->timer1_duration = read16(table_start + 2 * duration);

				table_start = fm->note_duration_table_select ? read16(NOTE_LENGTH_TABLE_3) : read16(NOTE_LENGTH_TABLE_2);
				fm->timer2_duration = read16(table_start + 2 * duration);
			}

			fm->timer1 = fm->timer1_duration;
			fm->timer2 = fm->timer2_duration;


			if (!(fm->flags & FM_FLAG_NOTE_IS_PAUSE) &&
				((fm->flags & FM_FLAG_UNKNOWN3) || !(fm->flags & FM_FLAG_UNKNOWN2)))
			{
				fm->flags &= ~FM_FLAG_UNKNOWN3;
				fm->flags |=  FM_FLAG_MUST_SEND_KEYON;
				fm->flags |=  FM_FLAG_MUST_SEND_CONFIGURATION;
			}
		}
	}



	if ((fm->flags & FM_FLAG_MODULATE_NOTE) && (fm->flags & FM_FLAG_MUST_SEND_KEYON))
	{
		fm->modulation_timer = read16(fm->modulation_table);
		fm->modulation_table_position = fm->modulation_table + 2;
	}

	if (!(fm->flags & FM_FLAG_MODULATE_NOTE) || (fm->flags & FM_FLAG_MUST_SEND_KEYON) || fm->must_update_voice_params)
	{
		int i;

		fm->must_update_voice_params = 0;

		for (i = 0; i < 4; i++)
			fm->voice_params[0x04 + i] = fm->voice_volume[i];

		fm->f_number = fm->note_period;
	}
	else
	{
		if (fm->modulation_timer == 0)
		{
			int i;
			UINT16 a;

			for (i = 0; i < 4; i++)
				fm->voice_params[0x04 + i] = fm->voice_volume[i];

			fm->modulation_table_position++;
			a = read8(fm->modulation_table_position++);
			if (a & 0x80)   // sign extend
				a |= 0xff00;
			a *= 4;

			fm->f_number = fm->note_period + a;

			fm->modulation_timer = read8(fm->modulation_table_position++);  // modulation_timer is UINT16 but this is just 8-bit

			if (read8(fm->modulation_table_position) == 0x80)   // end of table - repeat
			{
				fm->modulation_table_position = fm->modulation_table + 2;
			}
			else if (read8(fm->modulation_table_position) == 0x88)  // end of table - stop
			{
				fm->flags &= ~FM_FLAG_MODULATE_NOTE;
			}
		}
	}


#if 0
popmessage("%02x %02x %02x %02x %02x %02x",
		m_fm_control[0].note,
		m_fm_control[1].note,
		m_fm_control[2].note,
		m_fm_control[3].note,
		m_fm_control[4].note,
		m_fm_control[5].note);
#endif
#if 0
popmessage("%02x %02x%02x%02x%02x %02x %02x%02x%02x%02x %02x %02x%02x%02x%02x",
		m_fm_control[3].note,
		m_fm_control[3].voice_volume[0],
		m_fm_control[3].voice_volume[1],
		m_fm_control[3].voice_volume[2],
		m_fm_control[3].voice_volume[3],
		m_fm_control[4].note,
		m_fm_control[4].voice_volume[0],
		m_fm_control[4].voice_volume[1],
		m_fm_control[4].voice_volume[2],
		m_fm_control[4].voice_volume[3],
		m_fm_control[5].note,
		m_fm_control[5].voice_volume[0],
		m_fm_control[5].voice_volume[1],
		m_fm_control[5].voice_volume[2],
		m_fm_control[5].voice_volume[3]);
#endif
}


void nmk004_device::fm_voices_update(void)
{
	static const int ym2203_registers[0x18] =
	{
		0x30,0x38,0x34,0x3C,0x40,0x48,0x44,0x4C,0x50,0x58,0x54,0x5C,0x60,0x68,0x64,0x6C,
		0x70,0x78,0x74,0x7C,0x80,0x88,0x84,0x8C
	};
	int channel,i;

	address_space &space = machine().firstcpu->space(AS_PROGRAM);
	for (channel = 0; channel < 3;channel++)
	{
		struct fm_control *fm1 = &m_fm_control[channel];
		struct fm_control *fm2 = &m_fm_control[channel + 3];

		if (fm1->flags &  FM_FLAG_MUST_SEND_CONFIGURATION)
		{
			fm1->flags &= ~FM_FLAG_MUST_SEND_CONFIGURATION;

			for (i = 0; i < 0x18; i++)
			{
				m_ymdevice->control_port_w(space, 0, ym2203_registers[i] + channel);
				m_ymdevice->write_port_w(space, 0, fm1->voice_params[i]);
			}
		}

		if (fm2->flags &  FM_FLAG_MUST_SEND_CONFIGURATION)
		{
			fm2->flags &= ~FM_FLAG_MUST_SEND_CONFIGURATION;

			if (!(fm1->flags & FM_FLAG_ACTIVE))
			{
				for (i = 0; i < 0x18; i++)
				{
					m_ymdevice->control_port_w(space, 0, ym2203_registers[i] + channel);
					m_ymdevice->write_port_w(space, 0, fm2->voice_params[i]);
				}
			}
		}


		if (fm1->flags & FM_FLAG_ACTIVE)
		{
			m_ymdevice->control_port_w(space, 0, 0xb0 + channel); // self-feedback
			m_ymdevice->write_port_w(space, 0, fm1->self_feedback);

			m_ymdevice->control_port_w(space, 0, 0xa4 + channel); // F-number
			m_ymdevice->write_port_w(space, 0, fm1->f_number >> 8);

			m_ymdevice->control_port_w(space, 0, 0xa0 + channel); // F-number
			m_ymdevice->write_port_w(space, 0, fm1->f_number & 0xff);
		}
		else
		{
			m_ymdevice->control_port_w(space, 0, 0xb0 + channel); // self-feedback
			m_ymdevice->write_port_w(space, 0, fm2->self_feedback);

			m_ymdevice->control_port_w(space, 0, 0xa4 + channel); // F-number
			m_ymdevice->write_port_w(space, 0, fm2->f_number >> 8);

			m_ymdevice->control_port_w(space, 0, 0xa0 + channel); // F-number
			m_ymdevice->write_port_w(space, 0, fm2->f_number & 0xff);
		}



		if (fm1->flags & FM_FLAG_MUST_SEND_KEYON)
		{
			fm1->flags &= ~FM_FLAG_MUST_SEND_KEYON;

			m_ymdevice->control_port_w(space, 0, 0x28);   // keyon/off
			m_ymdevice->write_port_w(space, 0, fm1->slot | channel);
		}

		if (fm2->flags & FM_FLAG_MUST_SEND_KEYON)
		{
			fm2->flags &= ~FM_FLAG_MUST_SEND_KEYON;

			if (!(fm1->flags & FM_FLAG_ACTIVE))
			{
				m_ymdevice->control_port_w(space, 0, 0x28);   // keyon/off
				m_ymdevice->write_port_w(space, 0, fm2->slot | channel);
			}
		}
	}
}



/*****************************

        YM2203 - PSG

*****************************/

void nmk004_device::psg_update(int channel)
{
	struct psg_control *psg = &m_psg_control[channel];
	address_space &space = machine().firstcpu->space(AS_PROGRAM);

	// advance the timers
	if (psg->note_timer)
		psg->note_timer--;

	if (psg->volume_timer)
		psg->volume_timer--;

	if (psg->flags & PSG_FLAG_NEED_INITIALIZATION)
	{
		psg->flags &= ~PSG_FLAG_NEED_INITIALIZATION;
		psg->flags |=  PSG_FLAG_ACTIVE;

		if (psg->flags & PSG_FLAG_NOTE_IS_NOISE)
		{
			int enable;

			psg->flags &= ~PSG_FLAG_NOTE_IS_NOISE;
			psg->flags &= ~PSG_FLAG_NOISE_NOT_ENABLED;

			// enable noise, disable tone on this channel
			m_ymdevice->control_port_w(space, 0, 0x07);
			enable = m_ymdevice->read_port_r(space, 0);
			enable |=  (0x01 << channel);   // disable tone
			enable &= ~(0x08 << channel);   // enable noise
			m_ymdevice->write_port_w(space, 0, enable);
		}


		psg->note_timer = 0;
		psg->volume_timer = 0;
		psg->octave = 0;
	}

	if (psg->flags & PSG_FLAG_ACTIVE)
	{
		if (psg->note_timer == 0)
		{
			UINT8 token;

			do
			{
				token = read8(psg->current++);

				if ((token & 0xf0) == 0xf0)
				{
					int enable;

					switch (token)
					{
						case 0xf0:  // noise
							psg->flags |= PSG_FLAG_NOTE_IS_NOISE;
							break;

						case 0xf1:  // note
							psg->flags &= ~PSG_FLAG_NOTE_IS_NOISE;
							psg->flags &= ~PSG_FLAG_NOISE_NOT_ENABLED;

							// enable noise, disable tone on this channel
							m_ymdevice->control_port_w(space, 0, 0x07);
							enable = m_ymdevice->read_port_r(space, 0);
							enable |=  (0x01 << channel);   // disable tone
							enable &= ~(0x08 << channel);   // enable noise
							m_ymdevice->write_port_w(space, 0, enable);
							break;

						case 0xf2:  // set volume shape
						case 0xf3:
						case 0xf4:
						case 0xf5:
							psg->volume_shape = read8(psg->current++);
							break;

						case 0xf6:  // jump
							psg->current = read16(psg->current);
							break;

						case 0xf7:  // begin repeat loop
							psg->loop_times = read8(psg->current++);
							psg->loop_start = psg->current;
							break;

						case 0xf8:  // end repeat loop
							if (--psg->loop_times > 0)
								psg->current = psg->loop_start;
							break;

						case 0xf9:  // call subtable
							psg->return_address[psg->return_address_depth++] = psg->current + 2;
							psg->current = read16(psg->current);
							break;

						case 0xfa:  // return from subtable
							psg->current = psg->return_address[--psg->return_address_depth];
							break;

						case 0xfb:  // set octave
						case 0xfc:
						case 0xfd:
						case 0xfe:
							psg->octave = read8(psg->current++);
							break;

						case 0xff:  // end
							psg->flags = 0; // disable channel
							psg->volume_shape = 0;

							// mute channel
							m_ymdevice->control_port_w(space, 0, 8 + channel);
							m_ymdevice->write_port_w(space, 0, 0);
							return;
					}
				}
			} while ((token & 0xf0) == 0xf0);

			// token is the note to play
			psg->note = token;
			if ((psg->note & 0x0f) > NOTE_PAUSE)
			{
				fatalerror("PSG channel %d invalid note %02x\n",channel,psg->note);
			}

			// optional note length (otherwise use the same length as the previous one)
			if (read8(psg->current) & 0x80)
			{
				UINT16 table_start = read16(NOTE_LENGTH_TABLE_1);

				psg->note_length = read16(table_start + 2 * (read8(psg->current++) & 0x7f));
			}

			psg->note_timer = psg->note_length;
			psg->volume_timer = 0;

			psg->flags |= PSG_FLAG_INITIALIZE_VOLUME;

			if (psg->note == NOTE_PAUSE)
				psg->flags |=  PSG_FLAG_NOTE_IS_PAUSE;
			else
			{
				psg->flags &= ~PSG_FLAG_NOTE_IS_PAUSE;

				if (!(psg->flags & PSG_FLAG_NOTE_IS_NOISE))
				{
					UINT16 table_start = read16(PSG_NOTE_TABLE);
					UINT16 period = read16(table_start + 2 * (psg->note & 0x0f));
					UINT8 octave = psg->octave + ((psg->note & 0xf0) >> 4);

					period >>= octave;

					m_ymdevice->control_port_w(space, 0, 2 * channel + 1);
					m_ymdevice->write_port_w(space, 0, (period & 0x0f00) >> 8);
					m_ymdevice->control_port_w(space, 0, 2 * channel + 0);
					m_ymdevice->write_port_w(space, 0, (period & 0x00ff));

					psg->note_period_hi_bits = (period & 0x0f00) >> 8;
				}
				else
				{
					if (!(psg->flags & PSG_FLAG_NOISE_NOT_ENABLED))
					{
						int enable;

						psg->flags |= PSG_FLAG_NOISE_NOT_ENABLED;

						// disable noise, enable tone on this channel
						m_ymdevice->control_port_w(space, 0, 0x07);
						enable = m_ymdevice->read_port_r(space, 0);
						enable &= ~(0x01 << channel);   // enable tone
						enable |=  (0x08 << channel);   // disable noise
						m_ymdevice->write_port_w(space, 0, enable);
					}

					m_ymdevice->control_port_w(space, 0, 0x06);   // noise period
					m_ymdevice->write_port_w(space, 0, psg->note);
					psg->note_period_hi_bits = psg->note;
				}
			}
		}

		if (psg->volume_timer == 0)
		{
			UINT16 table_start = read16(PSG_VOLUME_TABLE);
			UINT16 vol_table_start = read16(table_start + 2 * psg->volume_shape);
			int volume;

			if (psg->flags & PSG_FLAG_INITIALIZE_VOLUME)
			{
				psg->flags &= ~PSG_FLAG_INITIALIZE_VOLUME;
				psg->volume_position = 0;
			}

			volume = read8(vol_table_start + psg->volume_position++);
			psg->volume_timer = read8(vol_table_start + psg->volume_position++);

			if (psg->flags & PSG_FLAG_NOTE_IS_PAUSE)
				volume = 0;

			// set volume
			m_ymdevice->control_port_w(space, 0, 8 + channel);
			m_ymdevice->write_port_w(space, 0, volume & 0x0f);
		}
	}
}



/*****************************

     Command processing

*****************************/

void nmk004_device::get_command(void)
{
	static const UINT8 from_main[] =
	{
		0x00,0x22,0x62,0x8c,0xc7,0x00,0x00,0x3f,0x7f,0x89,0xc7,0x00,0x00,0x2b,0x6b
	};
	static const UINT8 to_main[] =
	{
		0x82,0xc7,0x00,0x2c,0x6c,0x00,0x9f,0xc7,0x00,0x29,0x69,0x00,0x8b,0xc7,0x00
	};

	UINT8 cmd = m_from_main;

	if (m_protection_check < sizeof(to_main))
	{
		// startup handshake
		if (cmd == from_main[m_protection_check])
		{
			logerror("advance handshake to %02x\n",to_main[m_protection_check]);
			m_to_main = to_main[m_protection_check++];
		}
	}
	else
	{
		// send command back to main CPU to acknowledge reception
		m_to_main = cmd;
	}

	if (m_last_command != cmd)
	{
		UINT16 table_start = read16(COMMAND_TABLE);
		UINT16 cmd_table = read16(table_start + 2 * cmd);

		m_last_command = cmd;

		if ((cmd_table & 0xff00) == 0)
		{
			oki_play_sample(cmd_table);
		}
		else
		{
			int channel;

			while ((channel = read8(cmd_table++)) != 0xff)
			{
				table_start = read16(cmd_table);
				cmd_table += 2;

				if (channel == 0xef) break; // bioship bug?

//logerror("%04x: channel %d table %04x\n",cmd_table-3,channel,table_start);
				if (channel < FM_CHANNELS)
				{
					m_fm_control[channel].current = table_start;
					m_fm_control[channel].return_address_depth = 0;
					m_fm_control[channel].flags |= FM_FLAG_NEED_INITIALIZATION;
				}
				else
				{
					channel -= FM_CHANNELS;
					if (channel < PSG_CHANNELS)
					{
						m_psg_control[channel].current = table_start;
						m_psg_control[channel].return_address_depth = 0;
						m_psg_control[channel].flags |= PSG_FLAG_NEED_INITIALIZATION;
					}
					else
					{
						channel -= PSG_CHANNELS;
						if (channel >= EFFECTS_CHANNELS)
						{
							fatalerror("too many effects channels\n");
						}
						m_effects_control[channel].current = table_start;
						m_effects_control[channel].return_address_depth = 0;
						m_effects_control[channel].flags |= EFFECTS_FLAG_NEED_INITIALIZATION;
					}
				}
			}
		}
	}
}



void nmk004_device::update_music(void)
{
	int channel;

	for (channel = 0; channel < FM_CHANNELS; channel++)
		fm_update(channel);
	fm_voices_update();

	for (channel = 0; channel < PSG_CHANNELS; channel++)
		psg_update(channel);

	for (channel = 0; channel < EFFECTS_CHANNELS; channel++)
		effects_update(channel);
}



void nmk004_device::ym2203_irq_handler(int irq)
{
	if (irq)
	{
		address_space &space = machine().firstcpu->space(AS_PROGRAM);
		int status = m_ymdevice->status_port_r(space,0);

		if (status & 1) // timer A expired
		{
			oki_update_state();
			get_command();
			update_music();

			// restart timer
			m_ymdevice->control_port_w(space, 0, 0x27);
			m_ymdevice->write_port_w(space, 0, 0x15);
		}
	}
}


TIMER_CALLBACK_MEMBER( nmk004_device::real_init )
{
	static const UINT8 ym2203_init[] =
	{
		0x07,0x38,0x08,0x00,0x09,0x00,0x0A,0x00,0x24,0xB3,0x25,0x00,0x26,0xF9,0x27,0x15,
		0x28,0x00,0x28,0x01,0x28,0x02,0x40,0x00,0x41,0x00,0x42,0x00,0x44,0x00,0x45,0x00,
		0x46,0x00,0x48,0x00,0x49,0x00,0x4A,0x00,0x4C,0x00,0x4D,0x00,0x4E,0x00,0xFF,
	};
	int i;

	m_ymdevice = machine().device<ym2203_device>("ymsnd");
	m_oki1device = machine().device<okim6295_device>("oki1");
	m_oki2device = machine().device<okim6295_device>("oki2");

	m_rom = machine().root_device().memregion("audiocpu")->base();

	address_space &space = machine().firstcpu->space(AS_PROGRAM);

	if (m_ymdevice != NULL)
	{
		m_ymdevice->control_port_w(space, 0, 0x2f);

		i = 0;
		while (ym2203_init[i] != 0xff)
		{
			m_ymdevice->control_port_w(space, 0, ym2203_init[i++]);
			m_ymdevice->write_port_w(space, 0, ym2203_init[i++]);
		}
	}
	else
		return;

	m_oki_playing = 0;

	oki_play_sample(0);

	m_protection_check = 0;
}


WRITE16_MEMBER( nmk004_device::write )
{
	if (ACCESSING_BITS_0_7)
	{
//logerror("%06x: NMK004_w %02x\n",space.device().safe_pc(),data);
		m_from_main = data & 0xff;
	}
}

READ16_MEMBER( nmk004_device::read )
{
//static int last;
	int res = m_to_main;

//if (res != last) logerror("%06x: NMK004_r %02x\n",space.device().safe_pc(),res);
//last = res;

	return res;
}
