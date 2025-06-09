// license:GPL-2.0+
// copyright-holders:NewRisingSun

#ifndef MAME_NINTENDO_VT369_ADPCM_H
#define MAME_NINTENDO_VT369_ADPCM_H

#pragma once

DECLARE_DEVICE_TYPE(VT369_ADPCM_DECODER, vt369_adpcm_decoder_device)

class vt369_adpcm_decoder_device : public device_t
{
public:
	// construction/destruction
	vt369_adpcm_decoder_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	s32 decode_packet(u8* packet);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr int ADPCM_LEAD = 0;
	static constexpr int ADPCM_FRAME = 1;
	static constexpr int ADPCM_VOLUME = 2;
	static constexpr int ADPCM_LASTOUTPUT = 3;
	static constexpr int ADPCM_OUTPUT = 4;
	static constexpr int ADPCM_POSITION = 5;

	static constexpr s16 stepTable[16][16] = {
		{0, 14, 28, 42, 56, 70, 84, 97, -111,   -97,    -84,    -70,    -56,    -42,    -28,    -14 },
		{0, 13, 26, 39, 52, 65, 78, 91, -104,   -91,    -78,    -65,    -52,    -39,    -26,    -13 },
		{0, 11, 21, 32, 43, 54, 64, 75, -86,    -75,    -64,    -54,    -43,    -32,    -21,    -11 },
		{0, 9,  18, 27, 35, 44, 53, 62, -71,    -62,    -53,    -44,    -35,    -27,    -18,    -9  },
		{0, 7,  13, 20, 27, 34, 40, 47, -54,    -47,    -40,    -34,    -27,    -20,    -13,    -7  },
		{0, 6,  11, 17, 22, 28, 33, 39, -44,    -39,    -33,    -28,    -22,    -17,    -11,    -6  },
		{0, 5,  9,  14, 18, 23, 27, 32, -36,    -32,    -27,    -23,    -18,    -14,    -9, -5  },
		{0, 4,  8,  11, 15, 19, 23, 26, -30,    -26,    -23,    -19,    -15,    -11,    -8, -4  },
		{0, 3,  6,  9,  12, 15, 17, 20, -23,    -20,    -17,    -15,    -12,    -9, -6, -3  },
		{0, 2,  5,  7,  10, 12, 14, 17, -19,    -17,    -14,    -12,    -10,    -7, -5, -2  },
		{0, 2,  4,  6,  8,  10, 12, 14, -16,    -14,    -12,    -10,    -8, -6, -4, -2  },
		{0, 2,  3,  5,  6,  8,  10, 11, -13,    -11,    -10,    -8, -6, -5, -3, -2  },
		{0, 1,  2,  4,  5,  6,  7,  9,  -10,    -9, -7, -6, -5, -4, -2, -1  },
		{0, 1,  2,  3,  4,  5,  6,  7,  -8, -7, -6, -5, -4, -3, -2, -1  },
		{0, 1,  2,  3,  3,  4,  5,  6,  -7, -6, -5, -4, -3, -3, -2, -1  },
		{0, 1,  1,  2,  3,  4,  4,  5,  -6, -5, -4, -4, -3, -2, -1, -1  }
	};

};

#endif // MAME_NINTENDO_VT369_ADPCM_H
