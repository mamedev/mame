// license:BSD-3-Clause
// copyright-holders:pSXAuthor, R. Belmont
#include "emu.h"
#include "spu.h"

static inline float ms_to_rate(const float ms)
{
	return 1.0f/(ms*((float)spu_base_frequency_hz/1000.0f));
}

static inline float s_to_rate(const float s)
{
	return ms_to_rate(s*1000.0f);
}

static float linear_rate[]=
{
	ms_to_rate(0.05f),
	ms_to_rate(0.06f),
	ms_to_rate(0.07f),
	ms_to_rate(0.09f),
	ms_to_rate(0.10f),
	ms_to_rate(0.12f),
	ms_to_rate(0.15f),
	ms_to_rate(0.18f),
	ms_to_rate(0.21f),
	ms_to_rate(0.24f),
	ms_to_rate(0.29f),
	ms_to_rate(0.36f),
	ms_to_rate(0.41f),
	ms_to_rate(0.48f),
	ms_to_rate(0.58f),
	ms_to_rate(0.73f),
	ms_to_rate(0.83f),
	ms_to_rate(0.97f),
	ms_to_rate(1.2f),
	ms_to_rate(1.5f),
	ms_to_rate(1.7f),
	ms_to_rate(1.9f),
	ms_to_rate(2.3f),
	ms_to_rate(2.9f),
	ms_to_rate(3.3f),
	ms_to_rate(3.9f),
	ms_to_rate(4.6f),
	ms_to_rate(5.8f),
	ms_to_rate(6.6f),
	ms_to_rate(7.7f),
	ms_to_rate(9.3f),
	ms_to_rate(12.0f),
	ms_to_rate(13.0f),
	ms_to_rate(15.0f),
	ms_to_rate(19.0f),
	ms_to_rate(23.0f),
	ms_to_rate(27.0f),
	ms_to_rate(31.0f),
	ms_to_rate(37.0f),
	ms_to_rate(46.0f),
	ms_to_rate(53.0f),
	ms_to_rate(62.0f),
	ms_to_rate(74.0f),
	ms_to_rate(93.0f),
	s_to_rate(0.11f),
	s_to_rate(0.12f),
	s_to_rate(0.15f),
	s_to_rate(0.19f),
	s_to_rate(0.21f),
	s_to_rate(0.25f),
	s_to_rate(0.30f),
	s_to_rate(0.37f),
	s_to_rate(0.42f),
	s_to_rate(0.50f),
	s_to_rate(0.59f),
	s_to_rate(0.74f),
	s_to_rate(0.85f),
	s_to_rate(0.99f),
	s_to_rate(1.2f),
	s_to_rate(1.5f),
	s_to_rate(1.7f),
	s_to_rate(2.0f),
	s_to_rate(2.4f),
	s_to_rate(3.0f),
	s_to_rate(3.4f),
	s_to_rate(4.0f),
	s_to_rate(4.8f),
	s_to_rate(5.9f),
	s_to_rate(6.8f),
	s_to_rate(7.9f),
	s_to_rate(9.5f),
	s_to_rate(12.0f),
	s_to_rate(14.0f),
	s_to_rate(16.0f),
	s_to_rate(19.0f),
	s_to_rate(24.0f),
	s_to_rate(27.0f),
	s_to_rate(32.0f),
	s_to_rate(38.0f),
	s_to_rate(48.0f),
	s_to_rate(54.0f),
	s_to_rate(63.0f),
	s_to_rate(76.0f),
	s_to_rate(95.0f),
	s_to_rate(109.0f),
	s_to_rate(127.0f),
	s_to_rate(152.0f),
	s_to_rate(190.0f),
	s_to_rate(218.0f),
	s_to_rate(254.0f),
	s_to_rate(304.0f),
	s_to_rate(380.0f),
	s_to_rate(436.0f),
	s_to_rate(508.0f),
	s_to_rate(608.0f),
	s_to_rate(760.0f),
	s_to_rate(872.0f),
	s_to_rate(1016.0f),
	s_to_rate(1216.0f),
	s_to_rate(1520.0f),
	s_to_rate(1744.0f),
	s_to_rate(2032.0f),
	s_to_rate(2432.0f),
	s_to_rate(3040.0f),
	s_to_rate(3488.0f),
	s_to_rate(4064.0f),
	s_to_rate(4864.0f),
	s_to_rate(6080.0f)
};

static const int num_linear_rates=ARRAY_LENGTH(linear_rate);

static const float pos_exp_rate[]=
{
	ms_to_rate(0.09f),
	ms_to_rate(0.11f),
	ms_to_rate(0.13f),
	ms_to_rate(0.16f),
	ms_to_rate(0.18f),
	ms_to_rate(0.21f),
	ms_to_rate(0.25f),
	ms_to_rate(0.32f),
	ms_to_rate(0.36f),
	ms_to_rate(0.42f),
	ms_to_rate(0.51f),
	ms_to_rate(0.64f),
	ms_to_rate(0.73f),
	ms_to_rate(0.85f),
	ms_to_rate(1.0f),
	ms_to_rate(1.3f),
	ms_to_rate(1.5f),
	ms_to_rate(1.7f),
	ms_to_rate(2.0f),
	ms_to_rate(2.5f),
	ms_to_rate(2.9f),
	ms_to_rate(3.4f),
	ms_to_rate(4.1f),
	ms_to_rate(5.1f),
	ms_to_rate(5.8f),
	ms_to_rate(6.8f),
	ms_to_rate(8.1f),
	ms_to_rate(10.0f),
	ms_to_rate(12.0f),
	ms_to_rate(14.0f),
	ms_to_rate(16.0f),
	ms_to_rate(20.0f),
	ms_to_rate(23.0f),
	ms_to_rate(27.0f),
	ms_to_rate(33.0f),
	ms_to_rate(41.0f),
	ms_to_rate(46.0f),
	ms_to_rate(54.0f),
	ms_to_rate(65.0f),
	ms_to_rate(81.0f),
	ms_to_rate(93.0f),
	s_to_rate(0.11f),
	s_to_rate(0.13f),
	s_to_rate(0.16f),
	s_to_rate(0.19f),
	s_to_rate(0.22f),
	s_to_rate(0.26f),
	s_to_rate(0.33f),
	s_to_rate(0.37f),
	s_to_rate(0.43f),
	s_to_rate(0.52f),
	s_to_rate(0.65f),
	s_to_rate(0.74f),
	s_to_rate(0.87f),
	s_to_rate(1.0f),
	s_to_rate(1.3f),
	s_to_rate(1.5f),
	s_to_rate(1.7f),
	s_to_rate(2.1f),
	s_to_rate(2.6f),
	s_to_rate(3.0f),
	s_to_rate(3.5f),
	s_to_rate(4.2f),
	s_to_rate(5.2f),
	s_to_rate(5.9f),
	s_to_rate(6.9f),
	s_to_rate(8.3f),
	s_to_rate(10.0f),
	s_to_rate(12.0f),
	s_to_rate(14.0f),
	s_to_rate(17.0f),
	s_to_rate(21.0f),
	s_to_rate(24.0f),
	s_to_rate(28.0f),
	s_to_rate(33.0f),
	s_to_rate(42.0f),
	s_to_rate(48.0f),
	s_to_rate(55.0f),
	s_to_rate(67.0f),
	s_to_rate(83.0f),
	s_to_rate(95.0f),
	s_to_rate(111.0f),
	s_to_rate(133.0f),
	s_to_rate(166.0f),
	s_to_rate(190.0f),
	s_to_rate(222.0f),
	s_to_rate(266.0f),
	s_to_rate(333.0f),
	s_to_rate(380.0f),
	s_to_rate(444.0f),
	s_to_rate(532.0f),
	s_to_rate(666.0f),
	s_to_rate(760.0f),
	s_to_rate(888.0f),
	s_to_rate(1064.0f),
	s_to_rate(1332.0f),
	s_to_rate(1520.0f),
	s_to_rate(1776.0f),
	s_to_rate(2128.0f),
	s_to_rate(2664.0f)
};

static const int num_pos_exp_rates=ARRAY_LENGTH(pos_exp_rate);

static const float neg_exp_rate[]=
{
	ms_to_rate(0.07f),
	ms_to_rate(0.09f),
	ms_to_rate(0.11f),
	ms_to_rate(0.14f),
	ms_to_rate(0.18f),
	ms_to_rate(0.21f),
	ms_to_rate(0.25f),
	ms_to_rate(0.31f),
	ms_to_rate(0.39f),
	ms_to_rate(0.45f),
	ms_to_rate(0.53f),
	ms_to_rate(0.64f),
	ms_to_rate(0.81f),
	ms_to_rate(0.93f),
	ms_to_rate(1.1f),
	ms_to_rate(1.3f),
	ms_to_rate(1.6f),
	ms_to_rate(1.9f),
	ms_to_rate(2.2f),
	ms_to_rate(2.6f),
	ms_to_rate(3.3f),
	ms_to_rate(3.8f),
	ms_to_rate(4.4f),
	ms_to_rate(5.3f),
	ms_to_rate(6.7f),
	ms_to_rate(7.6f),
	ms_to_rate(8.9f),
	ms_to_rate(11.0f),
	ms_to_rate(13.0f),
	ms_to_rate(15.0f),
	ms_to_rate(18.0f),
	ms_to_rate(21.0f),
	ms_to_rate(27.0f),
	ms_to_rate(31.0f),
	ms_to_rate(36.0f),
	ms_to_rate(43.0f),
	ms_to_rate(53.0f),
	ms_to_rate(61.0f),
	ms_to_rate(71.0f),
	ms_to_rate(86.0f),
	s_to_rate(0.11f),
	s_to_rate(0.12f),
	s_to_rate(0.14f),
	s_to_rate(0.17f),
	s_to_rate(0.21f),
	s_to_rate(0.24f),
	s_to_rate(0.29f),
	s_to_rate(0.34f),
	s_to_rate(0.43f),
	s_to_rate(0.49f),
	s_to_rate(0.57f),
	s_to_rate(0.68f),
	s_to_rate(0.86f),
	s_to_rate(0.98f),
	s_to_rate(1.1f),
	s_to_rate(1.4f),
	s_to_rate(1.7f),
	s_to_rate(2.0f),
	s_to_rate(2.3f),
	s_to_rate(2.7f),
	s_to_rate(3.4f),
	s_to_rate(3.9f),
	s_to_rate(4.6f),
	s_to_rate(5.5f),
	s_to_rate(6.8f),
	s_to_rate(7.8f),
	s_to_rate(9.1f),
	s_to_rate(11.0f),
	s_to_rate(14.0f),
	s_to_rate(16.0f),
	s_to_rate(18.0f),
	s_to_rate(22.0f),
	s_to_rate(27.0f),
	s_to_rate(31.0f),
	s_to_rate(36.0f),
	s_to_rate(44.0f),
	s_to_rate(55.0f),
	s_to_rate(63.0f),
	s_to_rate(73.0f),
	s_to_rate(88.0f),
	s_to_rate(109.0f),
	s_to_rate(125.0f),
	s_to_rate(146.0f),
	s_to_rate(175.0f),
	s_to_rate(219.0f),
	s_to_rate(250.0f),
	s_to_rate(292.0f),
	s_to_rate(350.0f),
	s_to_rate(438.0f),
	s_to_rate(500.0f),
	s_to_rate(584.0f),
	s_to_rate(700.0f),
	s_to_rate(876.0f),
	s_to_rate(1000.0f),
	s_to_rate(1168.0f),
	s_to_rate(1400.0f),
	s_to_rate(1752.0f),
	s_to_rate(2000.0f),
	s_to_rate(2336.0f),
	s_to_rate(2800.0f),
	s_to_rate(3504.0f),
	s_to_rate(4000.0f),
	s_to_rate(4672.0f),
	s_to_rate(5600.0f),
	s_to_rate(7008.0f),
	s_to_rate(8000.0f),
	s_to_rate(9344.0f),
	s_to_rate(11200.0f)
};

static const int num_neg_exp_rates=ARRAY_LENGTH(neg_exp_rate);

static const float decay_rate[16]=
{
	ms_to_rate(0.07f),
	ms_to_rate(0.18f),
	ms_to_rate(0.39f),
	ms_to_rate(0.81f),
	ms_to_rate(1.6f),
	ms_to_rate(3.3f),
	ms_to_rate(6.7f),
	ms_to_rate(13.0f),
	ms_to_rate(27.0f),
	ms_to_rate(53.0f),
	s_to_rate(0.11f),
	s_to_rate(0.21f),
	s_to_rate(0.43f),
	s_to_rate(0.86f),
	s_to_rate(1.7f),
	s_to_rate(3.4f),
};

static const float linear_release_rate[]=
{
	ms_to_rate(0.04f),
	ms_to_rate(0.09f),
	ms_to_rate(0.18f),
	ms_to_rate(0.36f),
	ms_to_rate(0.73f),
	ms_to_rate(1.5f),
	ms_to_rate(2.9f),
	ms_to_rate(5.8f),
	ms_to_rate(12.0f),
	ms_to_rate(23.0f),
	ms_to_rate(46.0f),
	ms_to_rate(93.0f),
	s_to_rate(0.19f),
	s_to_rate(0.37f),
	s_to_rate(1.74f),
	s_to_rate(1.5f),
	s_to_rate(3.0f),
	s_to_rate(5.9f),
	s_to_rate(12.0f),
	s_to_rate(24.0f),
	s_to_rate(48.0f),
	s_to_rate(95.0f),
	s_to_rate(190.0f),
	s_to_rate(380.0f),
	s_to_rate(760.0f),
	s_to_rate(1520.0f),
	s_to_rate(3040.0f)
};

static const int num_linear_release_rates=ARRAY_LENGTH(linear_release_rate);

static const float exp_release_rate[]=
{
	ms_to_rate(0.07f),
	ms_to_rate(0.18f),
	ms_to_rate(0.39f),
	ms_to_rate(0.81f),
	ms_to_rate(1.6f),
	ms_to_rate(3.3f),
	ms_to_rate(6.7f),
	ms_to_rate(13.0f),
	ms_to_rate(27.0f),
	ms_to_rate(53.0f),
	s_to_rate(0.11f),
	s_to_rate(0.21f),
	s_to_rate(0.43f),
	s_to_rate(0.86f),
	s_to_rate(1.7f),
	s_to_rate(3.4f),
	s_to_rate(6.8f),
	s_to_rate(14.0f),
	s_to_rate(27.0f),
	s_to_rate(55.0f),
	s_to_rate(109.0f),
	s_to_rate(219.0f),
	s_to_rate(438.0f),
	s_to_rate(876.0f),
	s_to_rate(1752.0f),
	s_to_rate(3504.0f),
	s_to_rate(7008.0f)
};

static const int num_exp_release_rates=ARRAY_LENGTH(exp_release_rate);

//
//
//

spu_device::reverb_preset spu_device::reverb_presets[]=
{
	{
		"Reverb off",
		{
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
		}
	},

	{
		"Room",
		{
			0x007D, 0x005B, 0x6D80, 0x54B8, 0xBED0, 0x0000, 0x0000, 0xBA80,
			0x5800, 0x5300, 0x04D6, 0x0333, 0x03F0, 0x0227, 0x0374, 0x01EF,
			0x0334, 0x01B5, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x01B4, 0x0136, 0x00B8, 0x005C, 0x8000, 0x8000
		},
		{ // made up from studio small      [used by BOF3]
			-0.3112f,
			0.7832f,

			{ { 20.7f,   31.3f,40.1711f,51.6803f },
				{ 21.7025f,30.2f,42.6655f,48.6691f } },
			0.8f,

			0.0f,
			0.0f
		}
	},

	{
		"Studio Small",
		{
			0x0033, 0x0025, 0x70F0, 0x4FA8, 0xBCE0, 0x4410, 0xC0F0, 0x9C00,
			0x5280, 0x4EC0, 0x03E4, 0x031B, 0x03A4, 0x02AF, 0x0372, 0x0266,
			0x031C, 0x025D, 0x025C, 0x018E, 0x022F, 0x0135, 0x01D2, 0x00B7,
			0x018F, 0x00B5, 0x00B4, 0x0080, 0x004C, 0x0026, 0x8000, 0x8000
		},
		{ // made up from studio large      [used by Fighters Impact]
			-0.3112f,
			0.7832f,

			{ { 24.7f,44.3f,49.1711f,59.6803f },
				{ 27.7025f,38.2f,51.6655f,59.6691f } },
			0.6462f,

			0.0f,
			0.0f
		}
	},

	{
		"Studio Medium",
		{
			0x00B1, 0x007F, 0x70F0, 0x4FA8, 0xBCE0, 0x4510, 0xBEF0, 0xB4C0,
			0x5280, 0x4EC0, 0x0904, 0x076B, 0x0824, 0x065F, 0x07A2, 0x0616,
			0x076C, 0x05ED, 0x05EC, 0x042E, 0x050F, 0x0305, 0x0462, 0x02B7,
			0x042F, 0x0265, 0x0264, 0x01B2, 0x0100, 0x0080, 0x8000, 0x8000
		},
		{ // made up from studio large
			-0.3112f,
			0.7832f,

			{ { 37.7f,62.3f,82.1711f,71.6803f },
				{ 43.7025f,62.2f,85.6655f,78.6691f } },
			0.6462f,

			0.0f,
			0.0f
		}
	},

	{
		"Studio Large",
		{
			0x00E3, 0x00A9, 0x6F60, 0x4FA8, 0xBCE0, 0x4510, 0xBEF0, 0xA680,
			0x5680, 0x52C0, 0x0DFB, 0x0B58, 0x0D09, 0x0A3C, 0x0BD9, 0x0973,
			0x0B59, 0x08DA, 0x08D9, 0x05E9, 0x07EC, 0x04B0, 0x06EF, 0x03D2,
			0x05EA, 0x031D, 0x031C, 0x0238, 0x0154, 0x00AA, 0x8000, 0x8000
		},
		{ // tuned from xenogears ost
			-0.3112f,
			0.7832f,

			{ { 37.7f,62.3f,82.1711f,71.6803f },
				{ 43.7025f,62.2f,85.6655f,78.6691f } },
			0.8462f,

			0.0f,
			0.0f
		}
	},

	{
		"Hall",
		{
			0x01A5, 0x0139, 0x6000, 0x5000, 0x4C00, 0xB800, 0xBC00, 0xC000,
			0x6000, 0x5C00, 0x15BA, 0x11BB, 0x14C2, 0x10BD, 0x11BC, 0x0DC1,
			0x11C0, 0x0DC3, 0x0DC0, 0x09C1, 0x0BC4, 0x07C1, 0x0A00, 0x06CD,
			0x09C2, 0x05C1, 0x05C0, 0x041A, 0x0274, 0x013A, 0x8000, 0x8000
		},
		{ // made up
			-0.4222f,
			0.8889f,

			{ { 20.37f,79.63f,107.40f,94.44f },
				{ 31.47f,72.22f,116.66f,105.55f } },
			0.8889f,

			0.0f,
			0.0f
		}
	},

	{
		"Space Echo",
		{
			0x033D, 0x0231, 0x7E00, 0x5000, 0xB400, 0xB000, 0x4C00, 0xB000,
			0x6000, 0x5400, 0x1ED6, 0x1A31, 0x1D14, 0x183B, 0x1BC2, 0x16B2,
			0x1A32, 0x15EF, 0x15EE, 0x1055, 0x1334, 0x0F2D, 0x11F6, 0x0C5D,
			0x1056, 0x0AE1, 0x0AE0, 0x07A2, 0x0464, 0x0232, 0x8000, 0x8000
		},
		{ // made up
			0.3951f,
			0.7552f,

			{
				{ 36.7138f, 134.6171f, 24.4758f, 194.0596f },
				{   54.1932f,   90.9070f,    29.7174f, 200.0000f },
			},

			0.9301f,

			109.2563f,
			0.4222f
		}
	},

	{
		"Echo",
		{
			0x0001, 0x0001, 0x7FFF, 0x7FFF, 0x0000, 0x0000, 0x0000, 0x8100,
			0x0000, 0x0000, 0x1FFF, 0x0FFF, 0x1005, 0x0005, 0x0000, 0x0000,
			0x1005, 0x0005, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x1004, 0x1002, 0x0004, 0x0002, 0x8000, 0x8000
		}
	},

	{
		"Delay",
		{
			0x0001, 0x0001, 0x7FFF, 0x7FFF, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x1FFF, 0x0FFF, 0x1005, 0x0005, 0x0000, 0x0000,
			0x1005, 0x0005, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			0x0000, 0x0000, 0x1004, 0x1002, 0x0004, 0x0002, 0x8000, 0x8000,
		}
	},

	{
		"Half Echo",
		{
			0x0017, 0x0013, 0x70F0, 0x4FA8, 0xBCE0, 0x4510, 0xBEF0, 0x8500,
			0x5F80, 0x54C0, 0x0371, 0x02AF, 0x02E5, 0x01DF, 0x02B0, 0x01D7,
			0x0358, 0x026A, 0x01D6, 0x011E, 0x012D, 0x00B1, 0x011F, 0x0059,
			0x01A0, 0x00E3, 0x0058, 0x0040, 0x0028, 0x0014, 0x8000, 0x8000
		},
		{ // made up from space echo
			0.3951f,
			0.7552f,

			{
				{ 36.7138f, 134.6171f, 24.4758f, 194.0596f },
				{   54.1932f,   90.9070f,    29.7174f, 200.0000f },
			},

			0.7301f,

			109.2563f,
			0.7222f
		}
	},

	{ nullptr }
};

//
//
//

float spu_device::get_linear_rate(const int n)
{
	if (n>=num_linear_rates) return 0.0f;
	return linear_rate[n]*freq_multiplier;
}

float spu_device::get_linear_rate_neg_phase(const int n)
{
	if (n==0) return ms_to_rate(0.04f);
	return get_linear_rate(n-1);
}

float spu_device::get_pos_exp_rate(const int n)
{
	if (n>=num_pos_exp_rates) return 0.0f;
	return pos_exp_rate[n]*freq_multiplier;
}

float spu_device::get_pos_exp_rate_neg_phase(const int n)
{
	if (n==0) return ms_to_rate(0.04f);
	return get_pos_exp_rate(n-1);
}

float spu_device::get_neg_exp_rate(const int n)
{
	if (n>=num_neg_exp_rates) return 0.0f;
	return -neg_exp_rate[n]*freq_multiplier;
}

float spu_device::get_neg_exp_rate_neg_phase(const int n)
{
	if (n==0) return -ms_to_rate(0.04f);
	return get_neg_exp_rate(n-1);
}

float spu_device::get_decay_rate(const int n)
{
	return decay_rate[n]*freq_multiplier;
}

float spu_device::get_sustain_level(const int n)
{
	return ((float)(n+1))/16.0f;
}

float spu_device::get_linear_release_rate(const int n)
{
	if (n>=num_linear_release_rates) return 0.0f;
	return linear_release_rate[n]*freq_multiplier;
}

float spu_device::get_exp_release_rate(const int n)
{
	if (n>=num_exp_release_rates) return 0.0f;
	return exp_release_rate[n]*freq_multiplier;
}

spu_device::reverb_preset *spu_device::find_reverb_preset(const unsigned short *param)
{
	for (int i=0; reverb_presets[i].name; i++)
	{
		int j;

		for (j=0; j<32; j++)
			if (reverb_presets[i].param[j]!=param[j])
				break;
		if (j==32) return &reverb_presets[i];
	}

	return nullptr;
}
