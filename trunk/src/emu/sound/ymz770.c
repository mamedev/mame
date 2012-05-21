/***************************************************************************

    ymz770.c

    Emulation by R. Belmont
    AMM decode by Olivier Galibert

***************************************************************************/

#include "emu.h"
#include "ymz770.h"

// device type definition
const device_type YMZ770 = &device_creator<ymz770_device>;

//**************************************************************************
//  Yamaha "AMM" decoder
//**************************************************************************

class amm {
public:
	amm();

	void init();
	bool run();

	void set_pointers(UINT8 *ptr, UINT8 *outptr) { data = ptr; result = outptr; size = rsize = 0; }
	int get_rsize() { return rsize; }

private:
	struct band_parameter_size {
	int band_count;
	int s1, s2, s3, s4, s5;
	};

	struct band_info {
	int modulo;
	double s1;
	int bits, cube_bits;
	int s4, s5;
	double range, s7, scale, offset;
	};

	double amplitude_table[64];

	static const int sample_rates[8];
	static const int band_parameter_indexed_values[5][32][17];
	static const int band_parameter_index_bits_count[5][32];
	static const band_parameter_size band_parameter_sizes[5];
	static const int init_band_counts[4];
	static const band_info band_infos[18];
	static const double synthesis_filter[512];

	int sampling_rate, last_frame_number;
	int param_index, forced_param_index;

	int channel_count, total_bands, init_bands;

	int band_param[2][32];
	int scfsi[2][32];
	int scalefactors[2][3][32];
	double amp_values[2][3][32];
	double bdata[2][3][32];
	double subbuffer[2][32];
	double audio_buffer[2][32*32];
	int audio_buffer_pos[2];

	int master_pos;

	void read_header(int &pos);
	int get_band_param(int &pos, int band);
	void read_band_params(int &pos);
	void read_scfci(int &pos);
	void read_band_amplitude_params(int &pos);
	void read_band_value_triplet(int &pos, int chan, int band);
	void build_amplitudes();
	void build_next_segments(int &pos, int step);
	void retrieve_subbuffer(int step);
	void handle_block(int &pos);
	void idct32(const double *input, double *output);
	void resynthesis(const double *input, double *output);
	void scale_and_clamp(const double *input, int offset, int step);

	UINT8 *data;
	int size;

	UINT8 *result;
	int rsize;

	int gb(int &pos, int count)
	{
		int v = 0;
		for(int i=0; i != count; i++) {
			v <<= 1;
			if(data[pos >> 3] & (0x80 >> (pos & 7)))
				v |= 1;
			pos++;
		}
		return v;
	}

	void w16(int offset, unsigned short value)
	{
		if (offset > (0x8fe*2))
		{
			fatalerror("AMM block decoded to %x bytes, buffer overflow!\n", offset);
		}

		*(unsigned short *)(result+offset) = value;
	}
};

void amm::read_header(int &pos)
{
  int full_rate = gb(pos, 1);
  gb(pos, 2); // must be 2
  gb(pos, 1); // unused
  int full_packets_count = gb(pos, 4); // max 12
  int srate_index = gb(pos, 2); // max 2
  sampling_rate = srate_index + 4 * (1 - full_rate);
  int last_packet_frame_id = gb(pos, 2); // max 2
  last_frame_number = 3*full_packets_count + last_packet_frame_id;
  int frame_type = gb(pos, 2);
  int init_band_count_index = gb(pos, 2);
  int normal_param_index = gb(pos, 3);
  gb(pos, 1); // must be zero
  param_index = forced_param_index >= 5 ? normal_param_index : forced_param_index; // max 4
  channel_count = frame_type != 3 ? 2 : 1;
  total_bands = band_parameter_sizes[param_index].band_count;
  init_bands = total_bands;
  if(frame_type == 1)
    init_bands = init_band_counts[init_band_count_index];
  if(init_bands > total_bands )
    init_bands = total_bands;
}

const int amm::sample_rates[8] = { 44100, 48000, 32000, 0, 22050, 24000, 16000, 0 };

const int amm::band_parameter_indexed_values[5][32][17] = {
  {
    {  0,  1,  3,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, -1, },
    {  0,  1,  3,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, -1, },
    {  0,  1,  3,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
  },
  {
    {  0,  1,  3,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, -1, },
    {  0,  1,  3,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, -1, },
    {  0,  1,  3,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 17, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  3,  4,  5,  6, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
  },
  {
    {  0,  1,  2,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
  },
  {
    {  0,  1,  2,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
  },
  {
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, -1, },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4,  5,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0,  1,  2,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
    {  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, },
  }
};

const int amm::band_parameter_index_bits_count[5][32] = {
  { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 0, 0, 0, 0, 0, },
  { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 0, 0, },
  { 4, 4, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
  { 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
  { 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, },
};

const amm::band_parameter_size amm::band_parameter_sizes[5] = {
  { 27,  88, 104, 120, 135, 147 },
  { 30,  94, 110, 126, 141, 153 },
  {  8,  26,  40,  52,  52,  52 },
  { 12,  38,  52,  64,  76,  76 },
  { 30,  75,  91, 103, 114, 122 },
};

const int amm::init_band_counts[4] = { 4, 8, 12, 16 };

const amm::band_info amm::band_infos[18] = {
  { 0x0000,  0.00,  0,  0, 0,  0,           0,          0,                 0,         0 },
  { 0x0003,  7.00,  2,  5, 3,  9, 1-1.0/    4, -1.0/    4,   1/(1-1.0/    4), 1.0/    2 },
  { 0x0005, 11.00,  3,  7, 5, 25, 1-3.0/    8, -3.0/    8,   1/(1-3.0/    8), 1.0/    2 },
  { 0x0007, 16.00,  3,  9, 0,  0, 1-1.0/    8, -1.0/    8,   1/(1-1.0/    8), 1.0/    4 },
  { 0x0009, 20.84,  4, 10, 9, 81, 1-7.0/   16, -7.0/   16,   1/(1-7.0/   16), 1.0/    2 },
  { 0x000f, 25.28,  4, 12, 0,  0, 1-1.0/   16, -1.0/   16,   1/(1-1.0/   16), 1.0/    8 },
  { 0x001f, 31.59,  5, 15, 0,  0, 1-1.0/   32, -1.0/   32,   1/(1-1.0/   32), 1.0/   16 },
  { 0x003f, 37.75,  6, 18, 0,  0, 1-1.0/   64, -1.0/   64,   1/(1-1.0/   64), 1.0/   32 },
  { 0x007f, 43.84,  7, 21, 0,  0, 1-1.0/  128, -1.0/  128,   1/(1-1.0/  128), 1.0/   64 },
  { 0x00ff, 49.89,  8, 24, 0,  0, 1-1.0/  256, -1.0/  256,   1/(1-1.0/  256), 1.0/  128 },
  { 0x01ff, 55.93,  9, 27, 0,  0, 1-1.0/  512, -1.0/  512,   1/(1-1.0/  512), 1.0/  256 },
  { 0x03ff, 61.96, 10, 30, 0,  0, 1-1.0/ 1024, -1.0/ 1024,   1/(1-1.0/ 1024), 1.0/  512 },
  { 0x07ff, 67.98, 11, 33, 0,  0, 1-1.0/ 2048, -1.0/ 2048,   1/(1-1.0/ 2048), 1.0/ 1024 },
  { 0x0fff, 74.01, 12, 36, 0,  0, 1-1.0/ 4096, -1.0/ 4096,   1/(1-1.0/ 4096), 1.0/ 2048 },
  { 0x1fff, 80.03, 13, 39, 0,  0, 1-1.0/ 8192, -1.0/ 8192,   1/(1-1.0/ 8192), 1.0/ 4096 },
  { 0x3fff, 86.05, 14, 42, 0,  0, 1-1.0/16384, -1.0/16384,   1/(1-1.0/16384), 1.0/ 8192 },
  { 0x7fff, 92.01, 15, 45, 0,  0, 1-1.0/32768, -1.0/32768,   1/(1-1.0/32768), 1.0/16384 },
  { 0xffff, 98.01, 16, 48, 0,  0, 1-1.0/65536, -1.0/65536,   1/(1-1.0/65536), 1.0/32768 },
};

const double amm::synthesis_filter[512] = {
  +0.000000000, -0.000015259, -0.000015259, -0.000015259, -0.000015259, -0.000015259, -0.000015259, -0.000030518,
  -0.000030518, -0.000030518, -0.000030518, -0.000045776, -0.000045776, -0.000061035, -0.000061035, -0.000076294,
  -0.000076294, -0.000091553, -0.000106812, -0.000106812, -0.000122070, -0.000137329, -0.000152588, -0.000167847,
  -0.000198364, -0.000213623, -0.000244141, -0.000259399, -0.000289917, -0.000320435, -0.000366211, -0.000396729,
  -0.000442505, -0.000473022, -0.000534058, -0.000579834, -0.000625610, -0.000686646, -0.000747681, -0.000808716,
  -0.000885010, -0.000961304, -0.001037598, -0.001113892, -0.001205444, -0.001296997, -0.001388550, -0.001480103,
  -0.001586914, -0.001693726, -0.001785278, -0.001907349, -0.002014160, -0.002120972, -0.002243042, -0.002349854,
  -0.002456665, -0.002578735, -0.002685547, -0.002792358, -0.002899170, -0.002990723, -0.003082275, -0.003173828,
  +0.003250122, +0.003326416, +0.003387451, +0.003433228, +0.003463745, +0.003479004, +0.003479004, +0.003463745,
  +0.003417969, +0.003372192, +0.003280640, +0.003173828, +0.003051758, +0.002883911, +0.002700806, +0.002487183,
  +0.002227783, +0.001937866, +0.001617432, +0.001266479, +0.000869751, +0.000442505, -0.000030518, -0.000549316,
  -0.001098633, -0.001693726, -0.002334595, -0.003005981, -0.003723145, -0.004486084, -0.005294800, -0.006118774,
  -0.007003784, -0.007919312, -0.008865356, -0.009841919, -0.010848999, -0.011886597, -0.012939453, -0.014022827,
  -0.015121460, -0.016235352, -0.017349243, -0.018463135, -0.019577026, -0.020690918, -0.021789550, -0.022857666,
  -0.023910522, -0.024932861, -0.025909424, -0.026840210, -0.027725220, -0.028533936, -0.029281616, -0.029937744,
  -0.030532837, -0.031005860, -0.031387330, -0.031661987, -0.031814575, -0.031845093, -0.031738280, -0.031478880,
  +0.031082153, +0.030517578, +0.029785156, +0.028884888, +0.027801514, +0.026535034, +0.025085450, +0.023422241,
  +0.021575928, +0.019531250, +0.017257690, +0.014801025, +0.012115479, +0.009231567, +0.006134033, +0.002822876,
  -0.000686646, -0.004394531, -0.008316040, -0.012420654, -0.016708374, -0.021179200, -0.025817871, -0.030609130,
  -0.035552980, -0.040634155, -0.045837402, -0.051132202, -0.056533813, -0.061996460, -0.067520140, -0.073059080,
  -0.078628540, -0.084182740, -0.089706420, -0.095169070, -0.100540160, -0.105819700, -0.110946655, -0.115921020,
  -0.120697020, -0.125259400, -0.129562380, -0.133590700, -0.137298580, -0.140670780, -0.143676760, -0.146255500,
  -0.148422240, -0.150115970, -0.151306150, -0.151962280, -0.152069090, -0.151596070, -0.150497440, -0.148773200,
  -0.146362300, -0.143264770, -0.139450070, -0.134887700, -0.129577640, -0.123474120, -0.116577150, -0.108856200,
  +0.100311280, +0.090927124, +0.080688480, +0.069595340, +0.057617188, +0.044784546, +0.031082153, +0.016510010,
  +0.001068115, -0.015228271, -0.032379150, -0.050354004, -0.069168090, -0.088775635, -0.109161380, -0.130310060,
  -0.152206420, -0.174789430, -0.198059080, -0.221984860, -0.246505740, -0.271591200, -0.297210700, -0.323318480,
  -0.349868770, -0.376800540, -0.404083250, -0.431655880, -0.459472660, -0.487472530, -0.515609740, -0.543823240,
  -0.572036740, -0.600219700, -0.628295900, -0.656219500, -0.683914200, -0.711318970, -0.738372800, -0.765029900,
  -0.791214000, -0.816864000, -0.841949460, -0.866363500, -0.890090940, -0.913055400, -0.935195900, -0.956481930,
  -0.976852400, -0.996246340, -1.014617900, -1.031936600, -1.048156700, -1.063217200, -1.077117900, -1.089782700,
  -1.101211500, -1.111373900, -1.120224000, -1.127746600, -1.133926400, -1.138763400, -1.142211900, -1.144287100,
  +1.144989000, +1.144287100, +1.142211900, +1.138763400, +1.133926400, +1.127746600, +1.120224000, +1.111373900,
  +1.101211500, +1.089782700, +1.077117900, +1.063217200, +1.048156700, +1.031936600, +1.014617900, +0.996246340,
  +0.976852400, +0.956481930, +0.935195900, +0.913055400, +0.890090940, +0.866363500, +0.841949460, +0.816864000,
  +0.791214000, +0.765029900, +0.738372800, +0.711318970, +0.683914200, +0.656219500, +0.628295900, +0.600219700,
  +0.572036740, +0.543823240, +0.515609740, +0.487472530, +0.459472660, +0.431655880, +0.404083250, +0.376800540,
  +0.349868770, +0.323318480, +0.297210700, +0.271591200, +0.246505740, +0.221984860, +0.198059080, +0.174789430,
  +0.152206420, +0.130310060, +0.109161380, +0.088775635, +0.069168090, +0.050354004, +0.032379150, +0.015228271,
  -0.001068115, -0.016510010, -0.031082153, -0.044784546, -0.057617188, -0.069595340, -0.080688480, -0.090927124,
  +0.100311280, +0.108856200, +0.116577150, +0.123474120, +0.129577640, +0.134887700, +0.139450070, +0.143264770,
  +0.146362300, +0.148773200, +0.150497440, +0.151596070, +0.152069090, +0.151962280, +0.151306150, +0.150115970,
  +0.148422240, +0.146255500, +0.143676760, +0.140670780, +0.137298580, +0.133590700, +0.129562380, +0.125259400,
  +0.120697020, +0.115921020, +0.110946655, +0.105819700, +0.100540160, +0.095169070, +0.089706420, +0.084182740,
  +0.078628540, +0.073059080, +0.067520140, +0.061996460, +0.056533813, +0.051132202, +0.045837402, +0.040634155,
  +0.035552980, +0.030609130, +0.025817871, +0.021179200, +0.016708374, +0.012420654, +0.008316040, +0.004394531,
  +0.000686646, -0.002822876, -0.006134033, -0.009231567, -0.012115479, -0.014801025, -0.017257690, -0.019531250,
  -0.021575928, -0.023422241, -0.025085450, -0.026535034, -0.027801514, -0.028884888, -0.029785156, -0.030517578,
  +0.031082153, +0.031478880, +0.031738280, +0.031845093, +0.031814575, +0.031661987, +0.031387330, +0.031005860,
  +0.030532837, +0.029937744, +0.029281616, +0.028533936, +0.027725220, +0.026840210, +0.025909424, +0.024932861,
  +0.023910522, +0.022857666, +0.021789550, +0.020690918, +0.019577026, +0.018463135, +0.017349243, +0.016235352,
  +0.015121460, +0.014022827, +0.012939453, +0.011886597, +0.010848999, +0.009841919, +0.008865356, +0.007919312,
  +0.007003784, +0.006118774, +0.005294800, +0.004486084, +0.003723145, +0.003005981, +0.002334595, +0.001693726,
  +0.001098633, +0.000549316, +0.000030518, -0.000442505, -0.000869751, -0.001266479, -0.001617432, -0.001937866,
  -0.002227783, -0.002487183, -0.002700806, -0.002883911, -0.003051758, -0.003173828, -0.003280640, -0.003372192,
  -0.003417969, -0.003463745, -0.003479004, -0.003479004, -0.003463745, -0.003433228, -0.003387451, -0.003326416,
  +0.003250122, +0.003173828, +0.003082275, +0.002990723, +0.002899170, +0.002792358, +0.002685547, +0.002578735,
  +0.002456665, +0.002349854, +0.002243042, +0.002120972, +0.002014160, +0.001907349, +0.001785278, +0.001693726,
  +0.001586914, +0.001480103, +0.001388550, +0.001296997, +0.001205444, +0.001113892, +0.001037598, +0.000961304,
  +0.000885010, +0.000808716, +0.000747681, +0.000686646, +0.000625610, +0.000579834, +0.000534058, +0.000473022,
  +0.000442505, +0.000396729, +0.000366211, +0.000320435, +0.000289917, +0.000259399, +0.000244141, +0.000213623,
  +0.000198364, +0.000167847, +0.000152588, +0.000137329, +0.000122070, +0.000106812, +0.000106812, +0.000091553,
  +0.000076294, +0.000076294, +0.000061035, +0.000061035, +0.000045776, +0.000045776, +0.000030518, +0.000030518,
  +0.000030518, +0.000030518, +0.000015259, +0.000015259, +0.000015259, +0.000015259, +0.000015259, +0.000015259,
};

int amm::get_band_param(int &pos, int band)
{
  int bit_count = band_parameter_index_bits_count[param_index][band];
  int index = gb(pos, bit_count);
  return band_parameter_indexed_values[param_index][band][index];
}

void amm::read_band_params(int &pos)
{
  int band = 0;

  while(band < init_bands) {
    for(int chan=0; chan < channel_count; chan++)
      band_param[chan][band] = get_band_param(pos, band);
    band++;
  }

  while(band < total_bands) {
    int val = get_band_param(pos, band);
    band_param[0][band] = val;
    band_param[1][band] = val;
    band++;
  }

  while(band < 32) {
    band_param[0][band] = 0;
    band_param[1][band] = 0;
    band++;
  }
}

void amm::read_scfci(int &pos)
{
  memset(scfsi, 0, sizeof(scfsi));
  for(int band=0; band < total_bands; band++)
    for(int chan=0; chan < channel_count; chan++)
      if(band_param[chan][band])
	scfsi[chan][band] = gb(pos, 2);
}

void amm::read_band_amplitude_params(int &pos)
{
  memset(scalefactors, 0, sizeof(scalefactors));
  for(int band=0; band < total_bands; band++)
    for(int chan=0; chan<channel_count; chan++)
	if(band_param[chan][band]) {
	  switch(scfsi[chan][band]) {
	  case 0:
	    scalefactors[chan][0][band] = gb(pos, 6);
	    scalefactors[chan][1][band] = gb(pos, 6);
	    scalefactors[chan][2][band] = gb(pos, 6);
	    break;

	  case 1: {
	    int val = gb(pos, 6);
	    scalefactors[chan][0][band] = val;
	    scalefactors[chan][1][band] = val;
	    scalefactors[chan][2][band] = gb(pos, 6);
	    break;
	  }

	  case 2: {
	    int val = gb(pos, 6);
	    scalefactors[chan][0][band] = val;
	    scalefactors[chan][1][band] = val;
	    scalefactors[chan][2][band] = val;
	    break;
	  }

	  case 3: {
	    scalefactors[chan][0][band] = gb(pos, 6);
	    int val = gb(pos, 6);
	    scalefactors[chan][1][band] = val;
	    scalefactors[chan][2][band] = val;
	    break;
	  }
	  }
	}
}

void amm::build_amplitudes()
{
  memset(amp_values, 0, sizeof(amp_values));

  for(int band=0; band < total_bands; band++)
    for(int chan=0; chan<channel_count; chan++)
      if(band_param[chan][band])
	for(int step=0; step<3; step++)
	  amp_values[chan][step][band] = amplitude_table[scalefactors[chan][step][band]];
}

void amm::read_band_value_triplet(int &pos, int chan, int band)
{
  double buffer[3];

  int band_idx = band_param[chan][band];
  switch(band_idx) {
  case 0:
    bdata[chan][0][band] = 0;
    bdata[chan][1][band] = 0;
    bdata[chan][2][band] = 0;
    return;

  case 1:
  case 2:
  case 4: {
    int modulo = band_infos[band_idx].modulo;
    int val  = gb(pos, band_infos[band_idx].cube_bits);
    buffer[0] = val % modulo;
    val = val / modulo;
    buffer[1] = val % modulo;
    val = val / modulo;
    buffer[2] = val % modulo;
    break;
  }

  default: {
    int bits = band_infos[band_idx].bits;
    buffer[0] = gb(pos, bits);
    buffer[1] = gb(pos, bits);
    buffer[2] = gb(pos, bits);
    break;
  }
  }

  double scale = 1 << (band_infos[band_idx].bits - 1);

  bdata[chan][0][band] = ((buffer[0] - scale) / scale + band_infos[band_idx].offset) * band_infos[band_idx].scale;
  bdata[chan][1][band] = ((buffer[1] - scale) / scale + band_infos[band_idx].offset) * band_infos[band_idx].scale;
  bdata[chan][2][band] = ((buffer[2] - scale) / scale + band_infos[band_idx].offset) * band_infos[band_idx].scale;
}

void amm::build_next_segments(int &pos, int step)
{
  int band = 0;
  while(band < init_bands) {
    for(int chan=0; chan<channel_count; chan++) {
      read_band_value_triplet(pos, chan, band);
      double amp = amp_values[chan][step][band];
      bdata[chan][0][band] *= amp;
      bdata[chan][1][band] *= amp;
      bdata[chan][2][band] *= amp;
    }
    band++;
  }

  while(band < init_bands) {
    read_band_value_triplet(pos, 0, band);
    bdata[1][0][band] = bdata[0][0][band];
    bdata[1][1][band] = bdata[0][1][band];
    bdata[1][2][band] = bdata[0][2][band];

    for(int chan=0; chan<channel_count; chan++) {
      double amp = amp_values[chan][step][band];
      bdata[chan][0][band] *= amp;
      bdata[chan][1][band] *= amp;
      bdata[chan][2][band] *= amp;
    }
    band++;
  }

  while(band < 32) {
    bdata[0][0][band] = 0;
    bdata[0][1][band] = 0;
    bdata[0][2][band] = 0;
    bdata[1][0][band] = 0;
    bdata[1][1][band] = 0;
    bdata[1][2][band] = 0;
    band++;
  }
}

void amm::retrieve_subbuffer(int step)
{
  for(int chan=0; chan<channel_count; chan++)
    memcpy(subbuffer[chan], bdata[chan][step], 32*sizeof(subbuffer[0][0]));
}

void amm::idct32(const double *input, double *output)
{
  // Simplest idct32 ever, non-fast at all
  for(int i=0; i<32; i++) {
    double s = 0;
    for(int j=0; j<32; j++)
      s += input[j] * cos(i*(2*j+1)*M_PI/64);
    output[i] = s;
  }
}

void amm::resynthesis(const double *input, double *output)
{
  memset(output, 0, 32*sizeof(output[0]));
  for(int j=0; j<64*8; j+=64) {
    for(int i=0; i<16; i++) {
      output[   i] += input[   i+j]*synthesis_filter[   i+j] - input[32-i+j]*synthesis_filter[32+i+j];
    }
    output[16] -= input[16+j]*synthesis_filter[32+16+j];
	for(int i=17; i<32; i++) {
      output[i] -= input[32-i+j]*synthesis_filter[i+j] + input[i+j]*synthesis_filter[32+i+j];
	}
  }
}

void amm::scale_and_clamp(const double *input, int offset, int step)
{
  for(int i=0; i<32; i++) {
    double val = input[i]*32768 + 0.5;
    short cval;
    if(val <= -32768)
      cval = -32768;
    else if(val >= 32767)
      cval = 32767;
    else
      cval = int(val);
    w16(offset, cval);
    offset += step;
  }
}

amm::amm()
{
  forced_param_index = 5;

  for(int i=0; i<63; i++)
    amplitude_table[i] = pow(2, (3-i)/3.0);
  amplitude_table[63] = 1e-20;

  memset(audio_buffer, 0, sizeof(audio_buffer));
  audio_buffer_pos[0] = 16*32;
  audio_buffer_pos[1] = 16*32;
}

void amm::handle_block(int &pos)
{
  gb(pos, 12); // Must be 0xfff
//  printf("sync: %03x\n", h);
  read_header(pos);
//  printf("sample rate: %d\n", sample_rates[sampling_rate]);
  read_band_params(pos);
  read_scfci(pos);
  read_band_amplitude_params(pos);
  build_amplitudes();

  // Supposed to stop at last_frame_number when it's not 12*3+2 = 38
  for(int upper_step = 0; upper_step<3; upper_step++)
    for(int middle_step = 0; middle_step < 4; middle_step++) {
      build_next_segments(pos, upper_step);
      for(int lower_step = 0; lower_step < 3; lower_step++) {
	retrieve_subbuffer(lower_step);

	int base_offset = rsize;
	for(int chan=0; chan<channel_count; chan++) {
	  double resynthesis_buffer[32];
	  idct32(subbuffer[chan], audio_buffer[chan] + audio_buffer_pos[chan]);
	  resynthesis(audio_buffer[chan] + audio_buffer_pos[chan] + 16, resynthesis_buffer);
	  scale_and_clamp(resynthesis_buffer, base_offset+2*chan, 2*channel_count);
	  audio_buffer_pos[chan] -= 32;
	  if(audio_buffer_pos[chan]<0) {
	    memmove(audio_buffer[chan]+17*32, audio_buffer[chan], 15*32*sizeof(audio_buffer[chan][0]));
	    audio_buffer_pos[chan] = 16*32;
	  }
	}
	rsize += 32*2*channel_count;
      }
    }
  pos = (pos+7) & ~7;
}

void amm::init()
{
	master_pos = 0;
}

// returns true if this is the last block
bool amm::run()
{
	rsize = 0;
	handle_block(master_pos);

	// "last_frame_number" for the current chunk is 36 means there are more segments, otherwise not
	if (last_frame_number == 36)
	{
		return false;
	}

	return true;
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ymz770_device - constructor
//-------------------------------------------------

ymz770_device::ymz770_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YMZ770, "Yamaha YMZ770", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void ymz770_device::device_start()
{
	// create the stream
	m_stream = machine().sound().stream_alloc(*this, 0, 2, 16000, this);

	for (int i = 0; i < 8; i++)
	{
		channels[i].is_playing = false;
		channels[i].is_seq_playing = false;
		channels[i].decoder = new amm;
	}

	rom_base = device().machine().root_device().memregion(":ymz770")->base();

	save_item(NAME(cur_reg));
	for (int i = 0; i < 8; i++)
	{
		save_item(NAME(channels[i].phrase), i);
		save_item(NAME(channels[i].pan), i);
		save_item(NAME(channels[i].volume), i);
		save_item(NAME(channels[i].control), i);
		save_item(NAME(channels[i].is_playing), i);
		save_item(NAME(channels[i].last_block), i);
		save_item(NAME(channels[i].output_remaining), i);
		save_item(NAME(channels[i].output_ptr), i);
		save_item(NAME(channels[i].sequence), i);
		save_item(NAME(channels[i].seqcontrol), i);
		save_item(NAME(channels[i].seqdelay), i);
		save_item(NAME(channels[i].is_seq_playing), i);
		save_item(NAME(channels[i].output_data), i);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ymz770_device::device_reset()
{
	for (int i = 0; i < 8; i++)
	{
		channels[i].phrase = 0;
		channels[i].pan = 8;
		channels[i].volume = 0;
		channels[i].control = 0;
		channels[i].sequence = 0;
		channels[i].seqcontrol = 0;
		channels[i].seqdelay = 0;
		channels[i].is_playing = false;
		channels[i].is_seq_playing = false;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void ymz770_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *outL, *outR;
	int i, ch;

	outL = outputs[0];
	outR = outputs[1];

	for (i = 0; i < samples; i++)
	{
		INT32 mix;

		mix = 0;

		for (ch = 0; ch < 8; ch++)
		{
			if (channels[ch].is_seq_playing)
			{
				if (channels[ch].seqdelay != 0)
				{
					channels[ch].seqdelay--;
				}
				else
				{
					int reg = *channels[ch].seqdata++;
					UINT8 data = *channels[ch].seqdata++;
					switch (reg)
					{
						case 0x0f:
							if (channels[ch].seqcontrol & 1)
							{
								UINT8 sqn = channels[ch].sequence;
								UINT32 pptr = rom_base[(4*sqn)+1+0x400]<<16 | rom_base[(4*sqn)+2+0x400]<<8 | rom_base[(4*sqn)+3+0x400];
								channels[ch].seqdata = &rom_base[pptr];
							}
							else
							{
								channels[ch].is_seq_playing = false;
							}
							break;
						case 0x0e:
							channels[ch].seqdelay = 32 - 1;
							break;
						default:
							cur_reg = reg;
							internal_reg_write(1, data);
							break;
					}
				}
			}
			if (channels[ch].is_playing)
			{
				if (channels[ch].output_remaining > 0)
				{
					mix += (channels[ch].output_data[channels[ch].output_ptr++]*2*channels[ch].volume);
					channels[ch].output_remaining--;
				}
				else
				{
					if (channels[ch].last_block)
					{
						if (channels[ch].control & 1)
						{
								UINT8 phrase = channels[ch].phrase;
								UINT32 pptr = rom_base[(4*phrase)+1]<<16 | rom_base[(4*phrase)+2]<<8 | rom_base[(4*phrase)+3];

								channels[ch].decoder->set_pointers(&rom_base[pptr], (UINT8 *)&channels[ch].output_data[0]);
								channels[ch].decoder->init();
						}
						else
						{
								channels[ch].is_playing = false;
						}
					}

					if (channels[ch].is_playing)
					{
						channels[ch].last_block = channels[ch].decoder->run();
						channels[ch].output_remaining = (channels[ch].decoder->get_rsize()/2)-1;
						channels[ch].output_ptr = 1;

						mix += (channels[ch].output_data[0]*2*channels[ch].volume);
					}
				}
			}
		}

		outL[i] = outR[i] = mix>>8;
	}
}

//-------------------------------------------------
//  read - read from the chip's registers and internal RAM
//-------------------------------------------------

READ8_MEMBER( ymz770_device::read )
{
	return 0;
}

void ymz770_device::internal_reg_write(int offset, UINT8 data)
{
	if (!offset)
	{
		cur_reg = data;
		return;
	}

	if (cur_reg >= 0x40 && cur_reg <= 0x5f)
	{
		cur_reg -= 0x40;

		int voice = cur_reg / 4;
		int reg = cur_reg % 4;

		switch (reg)
		{
			case 0:
				channels[voice].phrase = data;
				break;

			case 1:
				channels[voice].volume = data;
				break;

			case 2:
				channels[voice].pan = data;
				break;

			case 3:
				if (data & 6)
				{
					UINT8 phrase = channels[voice].phrase;
					UINT32 pptr = rom_base[(4*phrase)+1]<<16 | rom_base[(4*phrase)+2]<<8 | rom_base[(4*phrase)+3];

					channels[voice].decoder->set_pointers(&rom_base[pptr], (UINT8 *)&channels[voice].output_data[0]);
					channels[voice].decoder->init();
					channels[voice].output_remaining = 0;
					channels[voice].output_ptr = 0;
					channels[voice].last_block = false;

					channels[voice].is_playing = true;
				}
				else
				{
					channels[voice].is_playing = false;
				}

				channels[voice].control = data;
				break;
		}
	}
	else if (cur_reg >= 0x80)
	{
		int voice = (cur_reg & 0x70)>>4;
		int reg = cur_reg & 0x0f;
		switch (reg)
		{
			case 0:
				channels[voice].sequence = data;
				break;
			case 1:
				if (data & 6)
				{
					UINT8 sqn = channels[voice].sequence;
					UINT32 pptr = rom_base[(4*sqn)+1+0x400]<<16 | rom_base[(4*sqn)+2+0x400]<<8 | rom_base[(4*sqn)+3+0x400];
					channels[voice].seqdata = &rom_base[pptr];
					channels[voice].seqdelay = 0;
					channels[voice].is_seq_playing = true;
				}
				else
				{
						channels[voice].is_seq_playing = false;
				}
				channels[voice].seqcontrol = data;
				break;
		}
	}
}

//-------------------------------------------------
//  write - write to the chip's registers and internal RAM
//-------------------------------------------------

WRITE8_MEMBER( ymz770_device::write )
{
	m_stream->update();
	internal_reg_write(offset, data);
}
