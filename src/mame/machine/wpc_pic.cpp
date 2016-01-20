// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "wpc_pic.h"

const device_type WPC_PIC = &device_creator<wpc_pic_device>;

wpc_pic_device::wpc_pic_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WPC_PIC, "Williams Pinball Controller PIC Security", tag, owner, clock, "wpc_pic", __FILE__),
	swarray(*this, ":SW")
{
	serial = "000 000000 00000 000";
}

wpc_pic_device::~wpc_pic_device()
{
}

void wpc_pic_device::set_serial(const char *_serial)
{
	serial = _serial;
}

READ8_MEMBER(wpc_pic_device::read)
{
	UINT8 data = 0x00;
	if(curcmd == 0x0d)
		data = count;

	else if((curcmd & 0xf0) == 0x70) {
		data = mem[curcmd & 0xf];
		scrambler = (scrambler >> 4) | (curcmd << 4);
		mem[ 5]= (mem[ 5]^scrambler) + mem[13];
		mem[13]= (mem[13]+scrambler) ^ mem[ 5];

	} else if(curcmd >= 0x16 && curcmd < 0x1e)
		data = swarray[curcmd - 0x16]->read();

	else
		logerror("%s: cmd=%02x (%04x)\n", tag().c_str(), curcmd, space.device().safe_pc());

	return data;
}

void wpc_pic_device::check_game_id()
{
	UINT32 cmp = (cmpchk[0] << 16) | (cmpchk[1] << 8) | cmpchk[2];
	for(int i=0; i<1000; i++) {
		UINT32 v = (i >> 8) * 0x3133 + (i & 0xff) * 0x3231;
		v = v & 0xffffff;
		if(v == cmp)
			logerror("%s: Detected game id %03d\n", tag().c_str(), i);
	}
}

WRITE8_MEMBER(wpc_pic_device::write)
{
	if(chk_count) {
		cmpchk[3-chk_count] = data;

		if(data != cmpchk[3-chk_count]) {
			logerror("%s: WARNING: validation error, checksum[%d] got %02x, expected %02x\n", tag().c_str(), 3-chk_count, data, chk[3-chk_count]);
			if(chk_count == 1)
				check_game_id();
		}

		chk_count--;
		return;
	}

	if(data == 0x00) {
		scrambler = 0xa5;
		count = 0x20;
		mem[ 5] = mem[0]^mem[15];
		mem[13] = mem[2]^mem[12];
	} else if(data == 0x0d)
		count = (count - 1) & 0x1f;
	else if(data == 0x20)
		chk_count = 3;
	else if((data < 0x16 || data >= 0x1e) && ((data & 0xf0) != 0x70))
		logerror("%s: write %02x (%04x)\n", tag().c_str(), data, space.device().safe_pc());

	curcmd = data;
}

void wpc_pic_device::serial_to_pic()
{
	UINT32 no[20];
	for(int i=0; i<20; i++)
		no[i] = serial[i] - '0';
	UINT32 v;

	mem[10] = 0x12; // Random?
	mem[ 2] = 0x34; // Random?

	v = (100*no[1] + 10*no[8] + no[5] + mem[10]*5)*0x1bcd + 0x1f3f0;
	mem[ 1] = v >> 16;
	mem[11] = v >> 8;
	mem[ 9] = v;

	v = (10000*no[2] + 1000*no[18] + 100*no[0] + 10*no[9] + no[7] + mem[10]*2 + mem[2])*0x107f + 0x71e259;
	mem[ 7] = v >> 24;
	mem[12] = v >> 16;
	mem[ 0] = v >> 8;
	mem[ 8] = v;

	v = (1000*no[19] + 100*no[4] + 10*no[6] + no[17] + mem[2])*0x245 + 0x3d74;
	mem[ 3] = v >> 16;
	mem[14] = v >> 8;
	mem[ 6] = v;

	v = 99999 - 10000*no[15] - 1000*no[14] - 100*no[13] - 10*no[12] - no[11];
	mem[15] = v >> 8;
	mem[ 4] = v;

	v = 100*no[0] + 10*no[1] + no[2];
	v = (v >> 8) * ((serial[17] << 8) | serial[19]) + (v & 0xff) * ((serial[18] << 8) | serial[17]);
	chk[0] = v >> 16;
	chk[1] = v >> 8;
	chk[2] = v;
}

void wpc_pic_device::device_start()
{
	save_item(NAME(mem));
	save_item(NAME(chk));
	save_item(NAME(curcmd));
	save_item(NAME(scrambler));
	save_item(NAME(count));
	save_item(NAME(chk_count));
}

void wpc_pic_device::device_reset()
{
	serial_to_pic();
	curcmd = 0x00;
	scrambler = 0x00;
	count = 0x00;
	chk_count = 0;
}
