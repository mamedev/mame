// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
//
// MAS 3507D MPEG audio decoder
//

#include "emu.h"
#include "mas3507d.h"

#define MINIMP3_ONLY_MP3
#define MINIMP3_NO_STDIO
#define MINIMP3_IMPLEMENTATION
#define MAX_FRAME_SYNC_MATCHES 1
#include "minimp3/minimp3.h"
#include "minimp3/minimp3_ex.h"

// device type definition
DEFINE_DEVICE_TYPE(MAS3507D, mas3507d_device, "mas3507d", "MAS 3507D MPEG decoder")

mas3507d_device::mas3507d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MAS3507D, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, cb_sample(*this)
	, i2c_bus_state(), i2c_bus_address(), i2c_scli(false), i2c_sclo(false), i2c_sdai(false), i2c_sdao(false)
	, i2c_bus_curbit(0), i2c_bus_curval(0), i2c_subdest(), i2c_command(), i2c_bytecount(0), i2c_io_bank(0), i2c_io_adr(0), i2c_io_count(0), i2c_io_val(0)
{
}

void mas3507d_device::device_start()
{
	current_rate = 44100;
	stream = stream_alloc(0, 2, current_rate);
	cb_sample.resolve();
}

void mas3507d_device::device_reset()
{
	i2c_scli = i2c_sdai = true;
	i2c_sclo = i2c_sdao = true;
	i2c_bus_state = IDLE;
	i2c_bus_address = UNKNOWN;
	i2c_bus_curbit = -1;
	i2c_bus_curval = 0;

	mp3dec_init(&mp3_dec);
	memset(mp3data.data(), 0, mp3data.size());
	memset(samples.data(), 0, samples.size());
	mp3_count = 0;
	sample_count = 0;
	total_frame_count = 0;
	buffered_frame_count = 0;
}

void mas3507d_device::i2c_scl_w(bool line)
{
	if(line == i2c_scli)
		return;
	i2c_scli = line;

	if(i2c_scli) {
		if(i2c_bus_state == STARTED) {
			if(i2c_sdai)
				i2c_bus_curval |= 1 << i2c_bus_curbit;
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
			i2c_sdao = true;
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
	if (address == 0x3b) {
		i2c_subdest = DATA_READ;
	} else {
		i2c_subdest = UNDEFINED;
	}

	return (address & 0xfe) == 0x3a;
}

void mas3507d_device::i2c_device_got_byte(uint8_t byte)
{
	switch(i2c_subdest) {
	case UNDEFINED:
		if(byte == 0x68)
			i2c_subdest = DATA_WRITE;
		else if(byte == 0x69)
			i2c_subdest = DATA_READ;
		else if(byte == 0x6a)
			i2c_subdest = CONTROL;
		else
			i2c_subdest = BAD;

		i2c_bytecount = 0;
		i2c_io_val = 0;

		break;

	case BAD:
		logerror("MAS I2C: Dropping byte %02x\n", byte);
		break;

	case DATA_READ:
		// Default Read
		// This should return the current MPEGFrameCount value when called

		// TODO: Figure out how to use this data exactly (chip docs are a little unclear to me)
		i2c_io_val <<= 8;
		i2c_io_val |= byte;
		i2c_bytecount++;

		logerror("MAS I2C: DATA_READ %d %08x\n", i2c_bytecount, i2c_io_val);

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
				logerror("MAS I2C: READ_CTRL\n");
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
				logerror("MAS I2C: READ_REG\n");
				break;
			case 0xe: case 0xf:
				i2c_io_bank = (byte >> 4) & 1;
				i2c_command = CMD_READ_MEM;
				logerror("MAS I2C: READ_MEM\n");
				break;
			default:
				i2c_command = CMD_BAD;
				logerror("MAS I2C: BAD\n");
				break;
			}
		} else {
			switch(i2c_command) {
			default:
				logerror("MAS I2C: Ignoring byte %02x\n", byte);
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
		logerror("MAS I2C: Control byte %02x\n", byte);
		break;
	}
}

void mas3507d_device::i2c_device_got_stop()
{
	logerror("MAS I2C: got stop\n");
}

int gain_to_db(double val) {
	return round(20 * log10((0x100000 - val) / 0x80000));
}

float gain_to_percentage(int val) {
	if (val == 0) {
		return 0; // Special case for muting it seems
	}

	double db = gain_to_db(val);

	return powf(10.0, (db + 6) / 20.0);
}

void mas3507d_device::mem_write(int bank, uint32_t adr, uint32_t val)
{
	switch(adr | (bank ? 0x10000 : 0)) {
	case 0x0032f: logerror("MAS3507D: OutputConfig = %05x\n", val); break;
	case 0x107f8:
		logerror("MAS3507D: left->left   gain = %05x (%d dB, %f%%)\n", val, gain_to_db(val), gain_to_percentage(val));
		stream->set_output_gain(0, gain_to_percentage(val));
		break;
	case 0x107f9:
		logerror("MAS3507D: left->right  gain = %05x (%d dB, %f%%)\n", val, gain_to_db(val), gain_to_percentage(val));
		break;
	case 0x107fa:
		logerror("MAS3507D: right->left  gain = %05x (%d dB, %f%%)\n", val, gain_to_db(val), gain_to_percentage(val));
		break;
	case 0x107fb:
		logerror("MAS3507D: right->right gain = %05x (%d dB, %f%%)\n", val, gain_to_db(val), gain_to_percentage(val));
		stream->set_output_gain(1, gain_to_percentage(val));
		break;
	default: logerror("MAS3507D: %d:%04x = %05x\n", bank, adr, val); break;
	}
}

void mas3507d_device::reg_write(uint32_t adr, uint32_t val)
{
	switch(adr) {
	case 0x8e: logerror("MAS3507D: DCCF = %05x\n", val); break;
	case 0xaa: logerror("MAS3507D: Mute/bypass = %05x\n", val); break;
	case 0xe6: logerror("MAS3507D: StartupConfig = %05x\n", val); break;
	case 0xe7: logerror("MAS3507D: Kprescale = %05x\n", val); break;
	case 0x6b: logerror("MAS3507D: Kbass = %05x\n", val); break;
	case 0x6f: logerror("MAS3507D: Ktreble = %05x\n", val); break;
	default: logerror("MAS3507D: reg %02x = %05x\n", adr, val); break;
	}
}

void mas3507d_device::run_program(uint32_t adr)
{
	switch(adr) {
	case 0xfcb: logerror("MAS3507D: validate OutputConfig\n"); break;
	default: logerror("MAS3507D: run %04x\n", adr); break;
	}
}

void mas3507d_device::fill_buffer()
{
	while(mp3_count + 2 < mp3data.size()) {
		u16 v = cb_sample();
		mp3data[mp3_count++] = v >> 8;
		mp3data[mp3_count++] = v;
	}

	int scount = mp3dec_decode_frame(&mp3_dec, mp3data.begin(), mp3_count, samples.begin(), &mp3_info);

	if(!scount) {
		int to_drop = mp3_info.frame_bytes;
		// At 1MHz, we can transfer around 2082 bytes/video frame.  So
		// that puts a boundary on how much we're ready to drop
		if(to_drop > 2082 || !to_drop)
			to_drop = 2082;
		std::copy(mp3data.begin() + to_drop, mp3data.end(), mp3data.begin());
		mp3_count -= to_drop;
		return;
	}

	std::copy(mp3data.begin() + mp3_info.frame_bytes, mp3data.end(), mp3data.begin());
	mp3_count -= mp3_info.frame_bytes;

	sample_count = scount;

	if(mp3_info.hz != current_rate) {
		current_rate = mp3_info.hz;
		stream->set_sample_rate(current_rate);
	}
}

void mas3507d_device::append_buffer(stream_sample_t **outputs, int &pos, int scount)
{
	if(!sample_count)
		return;

	buffered_frame_count = scount;

	int s1 = scount - pos;
	if(s1 > sample_count)
		s1 = sample_count;

	if(mp3_info.channels == 1) {
		for(int i=0; i<s1; i++) {
			stream_sample_t v = samples[i];
			outputs[0][i+pos] = v;
			outputs[1][i+pos] = v;
		}
	} else {
		for(int i=0; i<s1; i++) {
			outputs[0][i+pos] = samples[i*2];
			outputs[1][i+pos] = samples[i*2+1];
		}
	}

	if(s1 == sample_count) {
		pos += s1;
		sample_count = 0;
		total_frame_count += s1;
		return;
	}

	if(mp3_info.channels == 1)
		std::copy(samples.begin() + s1, samples.begin() + sample_count, samples.begin());
	else
		std::copy(samples.begin() + s1*2, samples.begin() + sample_count*2, samples.begin());

	pos += s1;
	sample_count -= s1;
	total_frame_count += s1;
}

void mas3507d_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int csamples)
{
	int pos = 0;

	append_buffer(outputs, pos, csamples);
	for(;;) {
		if(pos == csamples)
			return;
		fill_buffer();
		if(!sample_count) {
			// In the case of a bad frame or no frames being around, reset the state of the decoder
			mp3dec_init(&mp3_dec);
			memset(mp3data.data(), 0, mp3data.size());
			memset(samples.data(), 0, samples.size());
			mp3_count = 0;
			sample_count = 0;
			total_frame_count = 0;
			buffered_frame_count = 0;

			for(int i=pos; i != csamples; i++) {
				outputs[0][i] = 0;
				outputs[1][i] = 0;
			}
			return;
		}
		append_buffer(outputs, pos, csamples);
	}
}
