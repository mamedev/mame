// license:GPL-2.0+
// copyright-holders:NewRisingSun

#include "emu.h"
#include "vt369_adpcm.h"

DEFINE_DEVICE_TYPE(VT369_ADPCM_DECODER, vt369_adpcm_decoder_device, "vt369adpcm", "VRT VT369 ADPCM Decoder")

vt369_adpcm_decoder_device::vt369_adpcm_decoder_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VT369_ADPCM_DECODER, tag, owner, clock)
{
}

s32 vt369_adpcm_decoder_device::decode_packet(u8* packet)
{
	packet[ADPCM_POSITION] %= 48;
	int nibble = packet[ADPCM_FRAME] >> (packet[ADPCM_POSITION] & 1 ? 4 : 0);
	int index = packet[ADPCM_LEAD]
		- (packet[ADPCM_POSITION] >= 24 && packet[ADPCM_LEAD] & 0x40 ? 1 : 0)
		+ (packet[ADPCM_POSITION] >= 24 && packet[ADPCM_LEAD] & 0x80 ? 2 : 0);
	int step = stepTable[index & 0xF][nibble & 0xf];
	s8 output = packet[ADPCM_OUTPUT];
	switch (packet[ADPCM_LEAD] >> 4 & 3)
	{
	case 0: output = step;
		break;
	case 1: output = step + (s8)packet[ADPCM_OUTPUT];
		break;
	case 2: output = step + (s8)packet[ADPCM_OUTPUT] * 2 - (s8)packet[ADPCM_LASTOUTPUT];
		break;
	case 3: output = step + (s8)packet[ADPCM_OUTPUT] - ((s8)packet[ADPCM_LASTOUTPUT] >> 1);
		break;
	}
	packet[ADPCM_LASTOUTPUT] = packet[ADPCM_OUTPUT];
	packet[ADPCM_OUTPUT] = output;
	packet[ADPCM_POSITION]++;
	return output * (packet[ADPCM_VOLUME] & 0x7f);
}

void vt369_adpcm_decoder_device::device_start()
{
}

void vt369_adpcm_decoder_device::device_reset()
{
}
