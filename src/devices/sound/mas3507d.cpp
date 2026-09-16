// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
    Micronas MAS 3507D MPEG audio decoder

    Datasheet: https://www.mas-player.de/mp3/download/mas3507d.pdf

    TODO:
    - Datasheet says it has DSP and internal program ROM,
    but these are not dumped and hooked up
    - Support Broadcast mode, MPEG Layer 2
*/

#include "emu.h"
#include "mas3507d.h"
#include "mp3_audio.h"

#define LOG_READ     (1U << 1)
#define LOG_WRITE    (1U << 2)
#define LOG_REGISTER (1U << 3)
#define LOG_CONFIG   (1U << 4)
#define LOG_OTHER    (1U << 5)
// #define VERBOSE      (LOG_GENERAL | LOG_READ | LOG_WRITE | LOG_REGISTER | LOG_CONFIG | LOG_OTHER)
// #define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGREAD(...)     LOGMASKED(LOG_READ, __VA_ARGS__)
#define LOGWRITE(...)    LOGMASKED(LOG_WRITE, __VA_ARGS__)
#define LOGREGISTER(...) LOGMASKED(LOG_REGISTER, __VA_ARGS__)
#define LOGCONFIG(...)   LOGMASKED(LOG_CONFIG, __VA_ARGS__)
#define LOGOTHER(...)    LOGMASKED(LOG_OTHER, __VA_ARGS__)

ALLOW_SAVE_TYPE(mas3507d_device::i2c_bus_state_t)
ALLOW_SAVE_TYPE(mas3507d_device::i2c_bus_address_t)
ALLOW_SAVE_TYPE(mas3507d_device::i2c_subdest_t)
ALLOW_SAVE_TYPE(mas3507d_device::i2c_command_t)

// device type definition
DEFINE_DEVICE_TYPE(MAS3507D, mas3507d_device, "mas3507d", "Micronas MAS 3507D MPEG decoder")

mas3507d_device::mas3507d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MAS3507D, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, cb_mpeg_frame_sync(*this), cb_demand(*this)
	, i2c_bus_state(IDLE), i2c_bus_address(UNKNOWN), i2c_subdest(UNDEFINED), i2c_command(CMD_BAD)
	, i2c_scli(false), i2c_sclo(false), i2c_sdai(false), i2c_sdao(false)
	, i2c_bus_curbit(0), i2c_bus_curval(0), i2c_bytecount(0), i2c_io_bank(0), i2c_io_adr(0), i2c_io_count(0), i2c_io_val(0)
{
}

void mas3507d_device::device_start()
{
	stream = stream_alloc(0, 2, 44100);
	mp3dec = std::make_unique<mp3_audio>(reinterpret_cast<const uint8_t *>(&mp3data[0]));

	save_item(NAME(mp3data));
	save_item(NAME(samples));
	save_item(NAME(i2c_bus_state));
	save_item(NAME(i2c_bus_address));
	save_item(NAME(i2c_subdest));
	save_item(NAME(i2c_command));
	save_item(NAME(i2c_scli));
	save_item(NAME(i2c_sclo));
	save_item(NAME(i2c_sdai));
	save_item(NAME(i2c_sdao));
	save_item(NAME(i2c_bus_curbit));
	save_item(NAME(i2c_bus_curval));

	save_item(NAME(mp3data_count));
	save_item(NAME(decoded_frame_count));
	save_item(NAME(sample_count));
	save_item(NAME(samples_idx));
	save_item(NAME(is_muted));
	save_item(NAME(gain_ll));
	save_item(NAME(gain_rr));
	save_item(NAME(i2c_bytecount));
	save_item(NAME(i2c_io_bank));
	save_item(NAME(i2c_io_adr));
	save_item(NAME(i2c_io_count));
	save_item(NAME(i2c_io_val));
	save_item(NAME(i2c_sdao_data));

	save_item(NAME(frame_channels));

	mp3dec->register_save(*this);
}

void mas3507d_device::device_reset()
{
	i2c_scli = i2c_sdai = true;
	i2c_sclo = i2c_sdao = true;
	i2c_bus_state = IDLE;
	i2c_bus_address = UNKNOWN;
	i2c_bus_curbit = -1;
	i2c_bus_curval = 0;
	i2c_bytecount = 0;
	i2c_io_bank = 0;
	i2c_io_adr = 0;
	i2c_io_count = 0;
	i2c_io_val = 0;
	i2c_sdao_data = 0;

	is_muted = false;
	gain_ll = gain_rr = 1.0;

	frame_channels = 2;

	stream->set_sample_rate(44100);

	reset_playback();
}

void mas3507d_device::i2c_scl_w(bool line)
{
	if(line == i2c_scli)
		return;
	i2c_scli = line;
	// FIXME: sda output should only change state when clock is low.
	// On wrong device-id ensure sda is left high (Think it's OK but not certain)
	if(i2c_scli) {
		if(i2c_bus_state == STARTED) {
			if(i2c_sdai)
				i2c_bus_curval |= 1 << i2c_bus_curbit;

			if(i2c_subdest == DATA_READ)
				i2c_sdao = BIT(i2c_sdao_data, i2c_bus_curbit + (i2c_bytecount * 8));
			else {
				i2c_sdao_data = 0;
				i2c_sdao = false;
			}

			i2c_bus_curbit --;
			if(i2c_bus_curbit == -1) {
				if(i2c_bus_address == UNKNOWN) {
					if(i2c_device_got_address(i2c_bus_curval)) {
						i2c_bus_state = ACK;
						i2c_bus_address = VALIDATED;
						i2c_bus_curval = 0;
					} else {
						i2c_bus_state = NAK;
						i2c_bus_address = WRONG;
					}
				} else if(i2c_bus_address == VALIDATED) {
					i2c_bus_state = ACK;
					i2c_device_got_byte(i2c_bus_curval);
				}
			}
		} else if(i2c_bus_state == ACK) {
			i2c_bus_state = ACK2;
			i2c_sdao = false;
		}
	} else {
		if(i2c_bus_state == ACK2) {
			i2c_bus_state = STARTED;
			i2c_bus_curbit = 7;
			i2c_bus_curval = 0;
			i2c_sdao = false;
		}
	}
}

void mas3507d_device::i2c_nak()
{
	assert(i2c_bus_state == ACK);
	i2c_bus_state = NAK;
}

void mas3507d_device::i2c_sda_w(bool line)
{
	if(line == i2c_sdai)
		return;
	i2c_sdai = line;

	if(i2c_scli) {
		if(!i2c_sdai) {
			i2c_bus_state = STARTED;
			i2c_bus_address = UNKNOWN;
			i2c_bus_curbit = 7;
			i2c_bus_curval = 0;
		} else {
			i2c_device_got_stop();
			i2c_bus_state = IDLE;
			i2c_bus_address = UNKNOWN;
			i2c_bus_curbit = 7;
			i2c_bus_curval = 0;
		}
	}
}

int mas3507d_device::i2c_scl_r()
{
	return i2c_scli && i2c_sclo;
}

int mas3507d_device::i2c_sda_r()
{
	return i2c_sdai && i2c_sdao;
}

bool mas3507d_device::i2c_device_got_address(uint8_t address)
{
	if(address == CMD_DEV_READ)
		i2c_subdest = DATA_READ;
	else
		i2c_subdest = UNDEFINED;

	return (address & 0xfe) == CMD_DEV_WRITE;
}

void mas3507d_device::i2c_device_got_byte(uint8_t byte)
{
	switch(i2c_subdest) {
	case UNDEFINED:
		if(byte == CMD_DATA_WRITE)
			i2c_subdest = DATA_WRITE;
		else if(byte == CMD_DATA_READ) {
			i2c_subdest = DATA_READ;

			// Default read, returns the current frame count
			i2c_sdao_data = ((decoded_frame_count >> 8) & 0xff)
							| ((decoded_frame_count & 0xff) << 8)
							| (((decoded_frame_count >> 24) & 0xff) << 16)
							| (((decoded_frame_count >> 16) & 0xff) << 24);
		}
		else if(byte == CMD_CONTROL_WRITE)
			i2c_subdest = CONTROL;
		else
			i2c_subdest = BAD;

		i2c_bytecount = 0;
		i2c_io_val = 0;

		break;

	case BAD:
		LOGOTHER("MAS I2C: Dropping byte %02x\n", byte);
		break;

	case DATA_READ:
		switch(i2c_bytecount) {
		case 0: i2c_io_val = byte; break;
		case 1: i2c_io_val |= byte << 8; break;
		case 2: i2c_nak(); return;
		}

		LOGREAD("MAS I2C: DATA_READ %d %02x %08x\n", i2c_bytecount, byte, i2c_io_val);

		i2c_bytecount++;

		break;

	case DATA_WRITE:
		if(!i2c_bytecount) {
			switch(byte >> 4) {
			case 0: case 1:
				i2c_command = CMD_RUN;
				i2c_io_adr = byte << 8;
				break;
			case 3:
				i2c_command = CMD_READ_CTRL;
				LOGWRITE("MAS I2C: READ_CTRL\n");
				break;
			case 9:
				i2c_io_adr = (byte & 15) << 4;
				i2c_command = CMD_WRITE_REG;
				break;
			case 0xa: case 0xb:
				i2c_io_bank = (byte >> 4) & 1;
				i2c_command = CMD_WRITE_MEM;
				break;
			case 0xd:
				i2c_command = CMD_READ_REG;
				LOGWRITE("MAS I2C: READ_REG\n");
				break;
			case 0xe: case 0xf:
				i2c_io_bank = (byte >> 4) & 1;
				i2c_command = CMD_READ_MEM;
				LOGWRITE("MAS I2C: READ_MEM\n");
				break;
			default:
				i2c_command = CMD_BAD;
				LOGWRITE("MAS I2C: BAD\n");
				break;
			}
		} else {
			switch(i2c_command) {
			default:
				LOGWRITE("MAS I2C: Ignoring byte %02x\n", byte);
				break;

			case CMD_WRITE_REG:
				switch(i2c_bytecount) {
				case 1: i2c_io_adr |= byte >> 4; i2c_io_val = byte & 15; break;
				case 2: i2c_io_val |= byte << 12; break;
				case 3: i2c_io_val |= byte << 4; reg_write(i2c_io_adr, i2c_io_val); break;
				case 4: i2c_nak(); return;
				}
				break;

			case CMD_RUN:
				if(i2c_bytecount > 1) {
					i2c_nak();
					return;
				}
				i2c_io_adr |= byte;
				run_program(i2c_io_adr);
				break;

			case CMD_WRITE_MEM:
				switch(i2c_bytecount) {
				case 2: i2c_io_count = byte << 8; break;
				case 3: i2c_io_count |= byte; break;
				case 4: i2c_io_adr = byte << 8; break;
				case 5: i2c_io_adr |= byte; break;
				}
				if(i2c_bytecount >= 6) {
					uint32_t i2c_wordid = (i2c_bytecount - 6) >> 2;
					uint32_t i2c_offset = (i2c_bytecount - 6) & 3;
					if(i2c_wordid >= i2c_io_count) {
						i2c_nak();
						return;
					}
					switch(i2c_offset) {
					case 0: i2c_io_val = byte << 8; break;
					case 1: i2c_io_val |= byte; break;
					case 3: i2c_io_val |= (byte & 15) << 16; mem_write(i2c_io_bank, i2c_io_adr + i2c_wordid, i2c_io_val); break;
					}
				}
				break;
			}
		}

		i2c_bytecount++;
		break;

	case CONTROL:
		LOGOTHER("MAS I2C: Control byte %02x\n", byte);
		break;
	}
}

void mas3507d_device::i2c_device_got_stop()
{
	LOGOTHER("MAS I2C: got stop\n");
}

int mas3507d_device::gain_to_db(double val) {
	return round(20 * log10((0x100000 - val) / 0x80000));
}

float mas3507d_device::gain_to_percentage(int val)
{
	if(val == 0)
		return 0; // Special case for muting it seems

	double db = gain_to_db(val);

	return powf(10.0, (db + 6) / 20.0);
}

void mas3507d_device::mem_write(int bank, uint32_t adr, uint32_t val)
{
	switch(adr | (bank ? 0x10000 : 0)) {
	case 0x0032d: LOGCONFIG("MAS3507D: PLLOffset48 = %05x\n", val); break;
	case 0x0032e: LOGCONFIG("MAS3507D: PLLOffset44 = %05x\n", val); break;
	case 0x0032f: LOGCONFIG("MAS3507D: OutputConfig = %05x\n", val); break;
	case 0x107f8:
		gain_ll = gain_to_percentage(val);
		LOGCONFIG("MAS3507D: left->left   gain = %05x (%d dB, %f%%)\n", val, gain_to_db(val), gain_ll);
		break;
	case 0x107f9:
		LOGCONFIG("MAS3507D: left->right  gain = %05x (%d dB, %f%%)\n", val, gain_to_db(val), gain_to_percentage(val));
		break;
	case 0x107fa:
		LOGCONFIG("MAS3507D: right->left  gain = %05x (%d dB, %f%%)\n", val, gain_to_db(val), gain_to_percentage(val));
		break;
	case 0x107fb:
		gain_rr = gain_to_percentage(val);
		LOGCONFIG("MAS3507D: right->right gain = %05x (%d dB, %f%%)\n", val, gain_to_db(val), gain_rr);
		break;
	default: LOGCONFIG("MAS3507D: %d:%04x = %05x\n", bank, adr, val); break;
	}
}

void mas3507d_device::reg_write(uint32_t adr, uint32_t val)
{
	switch(adr) {
	case 0x8e: LOGCONFIG("MAS3507D: DCCF = %05x\n", val); break;
	case 0xaa:
		LOGCONFIG("MAS3507D: Mute/bypass = %05x\n", val);
		is_muted = val == 1;
		break;
	case 0xe6: LOGCONFIG("MAS3507D: StartupConfig = %05x\n", val); break;
	case 0xe7: LOGCONFIG("MAS3507D: Kprescale = %05x\n", val); break;
	case 0x6b: LOGCONFIG("MAS3507D: Kbass = %05x\n", val); break;
	case 0x6f: LOGCONFIG("MAS3507D: Ktreble = %05x\n", val); break;
	default: LOGCONFIG("MAS3507D: reg %02x = %05x\n", adr, val); break;
	}
}

void mas3507d_device::run_program(uint32_t adr)
{
	switch(adr) {
	case 0xfcb: LOGCONFIG("MAS3507D: validate OutputConfig\n"); break;
	default: LOGCONFIG("MAS3507D: run %04x\n", adr); break;
	}
}

void mas3507d_device::sid_w(uint8_t byte)
{
	if (mp3data_count >= mp3data.size()) {
		std::copy(mp3data.begin() + 1, mp3data.end(), mp3data.begin());
		mp3data_count--;
	}

	mp3data[mp3data_count++] = byte;

	cb_demand(mp3data_count < mp3data.size());
}

void mas3507d_device::fill_buffer()
{
	cb_mpeg_frame_sync(0);

	int pos = 0, frame_sample_rate = 0;
	bool decoded_frame = mp3dec->decode_buffer(pos, mp3data_count, &samples[0], sample_count, frame_sample_rate, frame_channels);
	samples_idx = 0;

	if (!decoded_frame || sample_count == 0) {
		// Frame decode failed
		if (mp3data_count >= mp3data.size()) {
			std::copy(mp3data.begin() + 1, mp3data.end(), mp3data.begin());
			mp3data_count--;
		}

		cb_demand(1); // always request more data when nothing could be decoded to force potentially stale data out of the buffer
		return;
	}

	std::copy(mp3data.begin() + pos, mp3data.end(), mp3data.begin());
	mp3data_count -= pos;

	stream->set_sample_rate(frame_sample_rate);

	decoded_frame_count++;
	cb_mpeg_frame_sync(1);

	cb_demand(mp3data_count < mp3data.size());
}

void mas3507d_device::append_buffer(sound_stream &stream, int &pos, int scount)
{
	const int bytes_per_sample = std::min(frame_channels, 2); // More than 2 channels is unsupported here
	const int s1 = std::min(scount - pos, sample_count);
	const sound_stream::sample_t sample_scale = 1.0 / 32768.0;
	const sound_stream::sample_t mute_scale = is_muted ? 0.0 : 1.0;

	for(int i = 0; i < s1; i++) {
		const sound_stream::sample_t lsamp_mixed = sound_stream::sample_t(samples[samples_idx * bytes_per_sample]) * sample_scale * mute_scale * gain_ll;
		const sound_stream::sample_t rsamp_mixed = sound_stream::sample_t(samples[samples_idx * bytes_per_sample + (bytes_per_sample >> 1)]) * sample_scale * mute_scale * gain_rr;

		stream.put(0, pos, lsamp_mixed);
		stream.put(1, pos, rsamp_mixed);

		samples_idx++;
		pos++;

		if(samples_idx >= sample_count) {
			sample_count = 0;
			return;
		}
	}
}

void mas3507d_device::reset_playback()
{
	std::fill(mp3data.begin(), mp3data.end(), 0);
	std::fill(samples.begin(), samples.end(), 0);

	mp3dec->clear();
	mp3data_count = 0;
	sample_count = 0;
	decoded_frame_count = 0;
	samples_idx = 0;
}

void mas3507d_device::sound_stream_update(sound_stream &stream)
{
	int csamples = stream.samples();
	int pos = 0;

	while(pos < csamples) {
		if(sample_count == 0)
			fill_buffer();

		if(sample_count <= 0)
			return;

		append_buffer(stream, pos, csamples);
	}
}
