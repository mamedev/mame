// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    MPEG audio support.  Only layer2 and variants for now.

***************************************************************************/

#ifndef __MPEG_AUDIO_H__
#define __MPEG_AUDIO_H__

class mpeg_audio {
public:
	// Accepted layers.  Beware that AMM is incompatible with L2 (and
	// not automatically recognizable) and that 2.5 implies 2.

	enum {
		L1 = 1,
		L2 = 2,
		L2_5 = 4,
		L3 = 8,
		AMM = 16
	};

	// base           = Start of the mpeg data block
	// accepted       = Binary or of accepted layers
	// lsb_first      = Read bits out of bytes lsb-first rather than msb first
	// position_align = Position alignment after reading a block (0 = pure bitstream, must be a power of 2 otherwise)

	mpeg_audio(const void *base, unsigned int accepted, bool lsb_first, int position_align);

	// Decode one mpeg buffer.
	// pos            = position in *bits* relative to base
	// limit          = maximum accepted position in bits
	// output         = output samples, interleaved
	// output_samples = number of samples written to output per channel
	// sample_rate    = output sample rate
	// channels       = number of channels written to output (total sample count is output_samples*channels)
	//
	// returns true if the buffer was complete and the new position in pos, false otherwise
	//
	// Sample rate and channels can change every buffer.  That's mpeg
	// for you.  Channels rarely changes, sample rate sometimes do,
	// especially in amm samples (drops to half at the end).
	//
	// One call to output buffer will generate 0 or 1 frame, which is
	// 384 samples per channel in layer I and 1152 otherwise (up to
	// 1152 in the amm case, <1152 indicating end of stream).

	bool decode_buffer(int &pos, int limit, short *output,
						int &output_samples, int &sample_rate, int &channels);


	// Clear audio buffer
	void clear();

private:
	struct limit_hit {};

	struct band_info {
		int modulo;
		double s1;
		int bits, cube_bits;
		int s4, s5;
		double range, s7, scale, offset;
	};

	static const double scalefactors[64];
	static const int sample_rates[8];
	static const int layer2_param_index[2][4][16];
	static const int band_parameter_indexed_values[5][32][17];
	static const int band_parameter_index_bits_count[5][32];
	static const int joint_band_counts[4], total_band_counts[5];
	static const band_info band_infos[18];
	static const double synthesis_filter[512];

	const UINT8 *base;
	int accepted, position_align;

	int sampling_rate, last_frame_number;
	int param_index;

	int channel_count, total_bands, joint_bands;

	int band_param[2][32];
	int scfsi[2][32];
	int scf[2][3][32];
	double amp_values[2][3][32];
	double bdata[2][3][32];
	double subbuffer[2][32];
	double audio_buffer[2][32*32];
	int audio_buffer_pos[2];

	int current_pos, current_limit;

	void read_header_amm(bool layer25);
	void read_header_mpeg2(bool layer25);
	void read_data_mpeg2();
	void decode_mpeg2(short *output, int &output_samples);

	int get_band_param(int band);
	void read_band_params();
	void read_scfci();
	void read_band_amplitude_params();
	void read_band_value_triplet(int chan, int band);
	void build_amplitudes();
	void build_next_segments(int step);
	void retrieve_subbuffer(int step);
	void idct32(const double *input, double *output);
	void resynthesis(const double *input, double *output);
	void scale_and_clamp(const double *input, short *output, int step);


	static int do_gb_msb(const unsigned char *data, int &pos, int count);
	static int do_gb_lsb(const unsigned char *data, int &pos, int count);

	int (*do_gb)(const unsigned char *data, int &pos, int count);

	inline int gb(int count)
	{
		if(current_pos + count > current_limit)
			throw limit_hit();

		return do_gb(base, current_pos, count);
	}
};

#endif
