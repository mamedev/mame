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

	// base = start of the MPEG video data block

	mpeg_video(const void *base, int maximum_width, int maximum_height);

	// Decode one MPEG coded picture or sequence-end marker.
	// pos          = position in bits relative to base
	// limit        = maximum accepted position in bits
	// buffers      = reconstructed, forward and backward YCbCr picture buffers
	// width        = width of a completed output picture
	// height       = height of a completed output picture
	// frame_rate   = sequence picture rate
	//
	// returns PICTURE if a complete coded picture was reconstructed,
	// SEQUENCE_END if a standalone sequence-end marker was consumed, NEED_DATA
	// if more input is required, or INVALID_DATA if invalid syntax was skipped.
	// pos is updated to the first unconsumed bit.

	decode_result decode_buffer(int &pos, int limit, const picture_buffers &buffers,
						int &width, int &height, double &frame_rate);

	// Clear persistent decoding state.
	void clear();

	// Register persistent decoding state with an owning device.
	void register_save_state(device_t &device, int index = 0);

private:
	struct limit_hit { };

	struct invalid_stream { };

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

	static const u8 s_default_intra_quantizer_matrix[64];
	static const u8 s_scan[64];
	static const double s_picture_rates[16];

	const u8 *m_base;
	int m_maximum_width;
	int m_maximum_height;
	int m_current_pos;
	int m_current_limit;

	s32 m_horizontal_size;
	s32 m_vertical_size;
	s32 m_mb_width;
	s32 m_mb_height;
	s32 m_luma_pitch;
	s32 m_chroma_pitch;
	double m_frame_rate;
	u8 m_intra_quantizer_matrix[64];
	u8 m_non_intra_quantizer_matrix[64];

	int m_picture_coding_type;
	bool m_full_pel_forward_vector;
	bool m_full_pel_backward_vector;
	int m_forward_f;
	int m_backward_f;
	int m_quantizer_scale;
	int m_macroblock_address;
	int m_dc_predictor[3];
	int m_forward_horizontal_previous;
	int m_forward_vertical_previous;
	int m_backward_horizontal_previous;
	int m_backward_vertical_previous;
	bool m_previous_b_forward;
	bool m_previous_b_backward;

	frame m_current_frame;
	frame m_forward_reference;
	frame m_backward_reference;
	double m_idct_basis[8][8];

	void sequence_header();
	void group_of_pictures();
	void picture(const picture_buffers &buffers);
	void slice(unsigned vertical_position);
	void macroblock(bool first_in_slice);
	void skipped_macroblock(int address);
	void block(unsigned index, bool intra);

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

	int macroblock_address_increment();
	macroblock_type macroblock_type_code();
	int coded_block_pattern();
	int motion_code();
	int dc_size(bool luminance);
	void dct_coefficient(bool first, int &run, int &level);

	void next_start_code();
	u32 peek(int count) const;
	u32 gb(int count);
};

#endif // MAME_VIDEO_MPEG_VIDEO_H
