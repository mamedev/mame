// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    ISO/IEC 11172-2 MPEG-1 video support.

    Clean-room implementation derived directly from ISO/IEC 11172-2:1993
    and its technical corrigenda.

***************************************************************************/

#ifndef MAME_VIDEO_MPEG_VIDEO_H
#define MAME_VIDEO_MPEG_VIDEO_H

#pragma once

#include <span>

class device_t;

class mpeg_video
{
public:
	enum class decode_result
	{
		PICTURE,
		SEQUENCE_END,
		NEED_DATA,
		INVALID_DATA
	};

	struct picture_buffer
	{
		u8 *data;
		unsigned bytes;
	};

	struct picture_buffers
	{
		picture_buffer reconstructed;
		picture_buffer forward;
		picture_buffer backward;
	};

	mpeg_video(int maximum_width, int maximum_height) ATTR_COLD;

	// Decode one MPEG coded picture or sequence-end marker.
	// input        = next bytes of the elementary video stream
	// consumed     = bytes accepted from input, which the caller may discard
	// buffers      = reconstructed, forward and backward YCbCr picture buffers
	// width        = width of a completed output picture
	// height       = height of a completed output picture
	// frame_rate   = sequence picture rate
	//
	// returns PICTURE if a complete coded picture was reconstructed,
	// SEQUENCE_END if a standalone sequence-end marker was consumed, NEED_DATA
	// if more input is required, or INVALID_DATA if invalid syntax was skipped.
	// Partial syntax and any accepted lookahead are retained across calls.
	// NEED_DATA accepts all supplied bytes.  An event can consume zero bytes
	// when its syntax was retained by a preceding call.  Keep picture buffer
	// bindings unchanged across NEED_DATA; the references and previous
	// reconstructed contents are sampled at the picture header.

	decode_result decode(std::span<const u8> input, std::size_t &consumed, const picture_buffers &buffers,
						int &width, int &height, double &frame_rate);

	// Clear persistent decoding state.
	void clear() ATTR_COLD;

	// Register persistent decoding state with an owning device.
	void register_save_state(device_t &device, int index = 0) ATTR_COLD;

private:
	struct vlc_entry;
	struct dct_vlc_entry;
	template <typename T, std::size_t N, unsigned MaxBits> class vlc_decoder;

	struct limit_hit { };

	struct invalid_stream { };

	enum : u8
	{
		SCAN,
		SEQUENCE_HEADER,
		GROUP_HEADER,
		PICTURE_HEADER,
		PICTURE_EXTRA,
		SLICE_HEADER,
		SLICE_EXTRA,
		MACROBLOCK_ADDRESS,
		MACROBLOCK,
		MACROBLOCK_END,
		RECOVER
	};

	struct frame
	{
		std::vector<u8> y;
		std::vector<u8> cb;
		std::vector<u8> cr;
	};

	struct macroblock_type
	{
		bool quant;
		bool forward;
		bool backward;
		bool pattern;
		bool intra;
	};

	struct motion_vector
	{
		int horizontal;
		int vertical;
	};

	static constexpr u32 PICTURE_START_CODE = 0x00000100;
	static constexpr u32 USER_DATA_START_CODE = 0x000001b2;
	static constexpr u32 SEQUENCE_HEADER_CODE = 0x000001b3;
	static constexpr u32 EXTENSION_START_CODE = 0x000001b5;
	static constexpr u32 SEQUENCE_END_CODE = 0x000001b7;
	static constexpr u32 GROUP_START_CODE = 0x000001b8;
	static constexpr u32 START_CODE_PREFIX = 0x000001;

	static constexpr u8 TYPE_QUANT = 0x01;
	static constexpr u8 TYPE_FORWARD = 0x02;
	static constexpr u8 TYPE_BACKWARD = 0x04;
	static constexpr u8 TYPE_PATTERN = 0x08;
	static constexpr u8 TYPE_INTRA = 0x10;

	static const vlc_entry s_macroblock_address_increment[33];
	static const vlc_entry s_coded_block_pattern[63];
	static const vlc_entry s_motion_code[33];
	static const dct_vlc_entry s_dct_coefficient[110];
	static const vlc_entry s_i_macroblock_type[2];
	static const vlc_entry s_p_macroblock_type[7];
	static const vlc_entry s_b_macroblock_type[11];
	static const vlc_entry s_d_macroblock_type[1];
	static const vlc_entry s_dc_size_luminance[9];
	static const vlc_entry s_dc_size_chrominance[9];

	static const vlc_decoder<vlc_entry, 33, 11> s_macroblock_address_increment_decoder;
	static const vlc_decoder<vlc_entry, 63, 9> s_coded_block_pattern_decoder;
	static const vlc_decoder<vlc_entry, 33, 11> s_motion_code_decoder;
	static const vlc_decoder<dct_vlc_entry, 110, 16> s_dct_coefficient_decoder;
	static const vlc_decoder<vlc_entry, 2, 2> s_i_macroblock_type_decoder;
	static const vlc_decoder<vlc_entry, 7, 6> s_p_macroblock_type_decoder;
	static const vlc_decoder<vlc_entry, 11, 6> s_b_macroblock_type_decoder;
	static const vlc_decoder<vlc_entry, 1, 1> s_d_macroblock_type_decoder;
	static const vlc_decoder<vlc_entry, 9, 7> s_dc_size_luminance_decoder;
	static const vlc_decoder<vlc_entry, 9, 8> s_dc_size_chrominance_decoder;

	static const u8 s_default_intra_quantizer_matrix[64];
	static const u8 s_scan[64];
	static const double s_picture_rates[16];

	// A macroblock contains at most six blocks of 64 coefficients, with at
	// most 28 bits per escape-coded coefficient, plus header and motion bits.
	static constexpr unsigned INPUT_BUFFER_BYTES = (6 * 64 * 28 + 128) / 8;
	u8 m_input_buffer[INPUT_BUFFER_BYTES];
	u32 m_input_bytes;
	u32 m_current_pos;
	u8 m_phase;
	bool m_in_picture;
	bool m_have_slice;
	bool m_first_in_slice;
	u8 m_slice_vertical_position;
	s32 m_address_increment;
	std::span<const u8> m_input;
	std::size_t m_consumed;
	int m_maximum_width;
	int m_maximum_height;

	s32 m_horizontal_size;
	s32 m_vertical_size;
	s32 m_mb_width;
	s32 m_mb_height;
	s32 m_luma_pitch;
	s32 m_chroma_pitch;
	double m_frame_rate;
	u8 m_intra_quantizer_matrix[64];
	u8 m_non_intra_quantizer_matrix[64];

	s32 m_picture_coding_type;
	bool m_full_pel_forward_vector;
	bool m_full_pel_backward_vector;
	s32 m_forward_f;
	s32 m_backward_f;
	s32 m_quantizer_scale;
	s32 m_macroblock_address;
	s32 m_dc_predictor[3];
	s32 m_forward_horizontal_previous;
	s32 m_forward_vertical_previous;
	s32 m_backward_horizontal_previous;
	s32 m_backward_vertical_previous;
	bool m_previous_b_forward;
	bool m_previous_b_backward;

	frame m_current_frame;
	frame m_forward_reference;
	frame m_backward_reference;
	double m_idct_basis[8][8];

	void sequence_header();
	void group_of_pictures();
	void picture_header(const picture_buffers &buffers);
	void slice_header();
	void macroblock();
	void skipped_macroblock(int address);
	void block(unsigned index, bool intra, int *quantized);
	void reconstruct_block(unsigned index, bool intra, const int *quantized);

	void reset_dc_predictors();
	void decode_motion_vector(bool forward, motion_vector &vector);
	static int decode_motion_component(int code, int residual, int f, int &previous, bool full_pel);
	void predict_macroblock(bool forward, bool backward, motion_vector forward_vector, motion_vector backward_vector);
	void predict_plane(u8 *destination, int destination_pitch, const u8 *reference, int reference_pitch,
					int x, int y, int width, int height, motion_vector vector, bool chroma, bool average) const;
	void put_block(unsigned index, const int *values, bool intra);
	void inverse_dct(const int *coefficients, int *values, bool dc_only) const;
	void read_frame(frame &destination, const u8 *source, unsigned source_bytes) const;
	void write_frame(const frame &source, u8 *output, unsigned output_bytes) const;

	template <std::size_t N> static constexpr vlc_entry make_vlc(const char (&text)[N], int value);
	template <std::size_t N> static constexpr dct_vlc_entry make_dct_vlc(const char (&text)[N], unsigned run, unsigned level);
	template <unsigned MaxBits, typename T, std::size_t N> static constexpr auto make_vlc_decoder(const T (&table)[N]);
	template <typename T, std::size_t N, unsigned MaxBits, typename P, typename S>
	static const T *decode_vlc(const T (&table)[N], const vlc_decoder<T, N, MaxBits> &decoder,
			int available, P &&peek, S &&skip);

	int macroblock_address_increment();
	macroblock_type macroblock_type_code();
	int coded_block_pattern();
	int motion_code();
	int dc_size(bool luminance);
	void dct_coefficient(bool first, int &run, int &level);

	void discard_consumed_bytes();
	int available_bits() const;
	u32 peek(int count);
	u32 gb(int count);
};

#endif // MAME_VIDEO_MPEG_VIDEO_H
