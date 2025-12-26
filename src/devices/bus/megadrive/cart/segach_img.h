// license: BSD-3-Clause
// copyright-holders: Nathan Misner, superctr

#ifndef MAME_BUS_MEGADRIVE_CART_SEGACH_IMG_H
#define MAME_BUS_MEGADRIVE_CART_SEGACH_IMG_H

#pragma once

class megadrive_segach_us_img
{
	public:
		static constexpr unsigned NUM_PIPES = 10;
		static constexpr unsigned PACKET_LEN = 288;
		static constexpr unsigned FRAME_LEN = 288 * 10;
		static constexpr unsigned PACKET_DATA_LEN = 246;

		static void or_bits(u8* source, unsigned sbitoff, u8* dest, int numbit)
		{
			sbitoff--;
			unsigned sbytes = sbitoff >> 3;
			sbitoff &= 7;

			unsigned dbitoff = 0;
			unsigned dbytes = 0;

			while (numbit)
			{
				if (sbitoff == 8)
				{
					sbytes++;
					sbitoff = 0;
				}
				if (dbitoff == 8)
				{
					dbytes++;
					dbitoff = 0;
				}
				if (source[sbytes] & (1 << sbitoff))
				{
					dest[dbytes] |= (1 << dbitoff);
				}
				sbitoff++;
				dbitoff++;
				numbit--;
			}
		}
		static void deweave(std::array<std::array<u8, PACKET_LEN>, 10>& pipes, u8 *data)
		{
			unsigned cursor = 0;
			for (unsigned i = 0; i < PACKET_LEN; i += 2)
			{
				for (unsigned j = 0; j < NUM_PIPES; j++)
				{
					pipes[j][i] = data[cursor++];
					pipes[j][i + 1] = data[cursor++];
				}
			}
		}
		static void deinterleave(std::array<u8, PACKET_LEN>& data)
		{
			u8 interFrame[PACKET_LEN] = { 0 };
			unsigned packetAoff = 0;
			unsigned packetBoff = 225;
			unsigned packetXoff = packetAoff;
			unsigned packet = 0;

			or_bits(&data[0], 1, interFrame, 27);
			while (packet < 9)
			{
				if (packet == 0)
				{
					packetAoff += 27;
					packetBoff += 27;
					packetXoff += 27;
				}
				if (packet == 4)
				{
					packetBoff += 27;
				}
				if (packet == 6)
				{
					packetAoff += 27;
					packetXoff += 27;
				}

				unsigned bitCount = 0;
				unsigned pc = 0;
				unsigned packetAbit = 0;
				unsigned packetBbit = 0;
				unsigned *bitPtr = &packetAbit;
				while (bitCount < 450)
				{
					if (pc == 2)
					{
						pc = 0;
						if (bitPtr == &packetAbit)
							bitPtr = &packetBbit;
						else
							bitPtr = &packetAbit;
					}

					if (bitCount == 225)
					{
						bitPtr = &packetBbit;
						pc = 0;
					}

					unsigned packetBits;
					if (bitPtr == &packetAbit)
						packetBits = *bitPtr + packetAoff;
					else
						packetBits = *bitPtr + packetBoff;

					if ((packet == 4) && (bitCount == 225))
						packetXoff += 27;

					// copy bit #sourceBits to bit #packetBits
					unsigned sourceBits = bitCount + packetXoff;
					if (data[sourceBits >> 3] & (1 << (sourceBits & 7)))
						interFrame[packetBits >> 3] |= (1 << (packetBits & 7));
					(*bitPtr)++;
					bitCount++;
					pc++;
				}

				packetAoff += 450;
				packetBoff += 450;
				packetXoff = packetAoff;
				packet += 2;
			}
			memcpy(&data[0], interFrame, PACKET_LEN);
		}
		static void get_data(u8 *in, u8 *out)
		{
			u8 rData[PACKET_DATA_LEN] = { 0 };

			int bitoff = 140;
			for (int i = 0; i < PACKET_DATA_LEN;)
			{
				if (bitoff == 1153)
				{
					bitoff += 27;
				}

				if (i == 0)
				{
					or_bits(in, bitoff, rData, 96);
					bitoff += 96;
					i += 12;
				}
				else
				{
					or_bits(in, bitoff, rData + i, 208);
					bitoff += 208;
					i += 26;
				}
				// skip bch & parity
				bitoff += 17;
			}

			for (int i = 0; i < PACKET_DATA_LEN; i++)
			{
				out[i] = bitswap<8>(rData[i], 0,1,2,3,4,5,6,7);
			}
		}
};

#endif
