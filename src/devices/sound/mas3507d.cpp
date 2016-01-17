// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
//
// MAS 3507D MPEG audio decoder
//

#include "emu.h"
#include "mas3507d.h"

// device type definition
const device_type MAS3507D = &device_creator<mas3507d_device>;

mas3507d_device::mas3507d_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MAS3507D, "MAS3507D", tag, owner, clock, "mas3507d", __FILE__),
		device_sound_interface(mconfig, *this), i2c_bus_state(), i2c_bus_address(), i2c_scli(false), i2c_sclo(false), i2c_sdai(false), i2c_sdao(false),
	i2c_bus_curbit(0), i2c_bus_curval(0), i2c_subdest(), i2c_command(), i2c_bytecount(0), i2c_io_bank(0), i2c_io_adr(0), i2c_io_count(0), i2c_io_val(0)
{
}

void mas3507d_device::device_start()
{
}

void mas3507d_device::device_reset()
{
	i2c_scli = i2c_sdai = true;
	i2c_sclo = i2c_sdao = true;
	i2c_bus_state = IDLE;
	i2c_bus_address = UNKNOWN;
	i2c_bus_curbit = -1;
	i2c_bus_curval = 0;
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

bool mas3507d_device::i2c_device_got_address(UINT8 address)
{
	i2c_subdest = UNDEFINED;
	return (address & 0xfe) == 0x3a;
}

void mas3507d_device::i2c_device_got_byte(UINT8 byte)
{
	switch(i2c_subdest) {
	case UNDEFINED:
		if(byte == 0x68)
			i2c_subdest = DATA;
		else if(byte == 0x69)
			i2c_subdest = DATA;
		else if(byte == 0x6a)
			i2c_subdest = CONTROL;
		else
			i2c_subdest = BAD;
		i2c_bytecount = 0;
		break;
	case BAD:
		logerror("MAS I2C: Dropping byte %02x\n", byte);
		break;
	case DATA:
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
					UINT32 i2c_wordid = (i2c_bytecount - 6) >> 2;
					UINT32 i2c_offset = (i2c_bytecount - 6) & 3;
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

void mas3507d_device::mem_write(int bank, UINT32 adr, UINT32 val)
{
	switch(adr | (bank ? 0x10000 : 0)) {
	case 0x0032f: logerror("MAS3507D: OutputConfig = %05x\n", val); break;
	case 0x107f8: logerror("MAS3507D: left->left   gain = %05x\n", val); break;
	case 0x107f9: logerror("MAS3507D: left->right  gain = %05x\n", val); break;
	case 0x107fa: logerror("MAS3507D: right->left  gain = %05x\n", val); break;
	case 0x107fb: logerror("MAS3507D: right->right gain = %05x\n", val); break;
	default: logerror("MAS3507D: %d:%04x = %05x\n", bank, adr, val); break;
	}
}

void mas3507d_device::reg_write(UINT32 adr, UINT32 val)
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

void mas3507d_device::run_program(UINT32 adr)
{
	switch(adr) {
	case 0xfcb: logerror("MAS3507D: validate OutputConfig\n"); break;
	default: logerror("MAS3507D: run %04x\n", adr); break;
	}
}

void mas3507d_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
}
