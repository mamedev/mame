// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#pragma once

#ifndef PIN64_H
#define PIN64_H

#include <cstdio>
#include <vector>

#include "emu.h"

class pin64_fileutil_t
{
public:
	static void write(FILE* file, uint32_t data);
	static void write(FILE* file, const uint8_t* data, uint32_t size);
};

class pin64_block_t
{
public:
	pin64_block_t()
		: m_crc32{ 0 }
		, m_offset(0)
		, m_old_offset(0)
	{ }

	void put8(uint8_t data);
	void put16(uint16_t data);
	void put32(uint32_t data);

	void clear();
	void finalize();
	void reset();

	void write(FILE* file);

	// getters
	uint8_t get8();
	uint8_t get8(uint32_t offset, bool update_current = true);
	uint16_t get16();
	uint16_t get16(uint32_t offset, bool update_current = true);
	uint32_t get32();
	uint32_t get32(uint32_t offset, bool update_current = true);

	util::crc32_t crc32() const { return m_crc32; }
	uint8_t* bytes() { return &m_data[0]; }
	uint32_t data_size();
	uint32_t size();

private:
	void update_offset(uint32_t offset, bool update_current = true);

	util::crc32_t m_crc32;
	std::vector<uint8_t> m_data;

	uint32_t m_offset;
	uint32_t m_old_offset;
};

class pin64_t
{
public:
	pin64_t()
		: m_capture_file(nullptr)
		, m_capture_index(~0)
		, m_capture_frames(0)
		, m_current_frame(nullptr)
	{ }
	~pin64_t();

	void start(int frames = 0);
	void finish();
	void mark_frame(running_machine& machine);

	void clear();

	bool capturing() const { return m_capture_file != nullptr; }

	uint32_t size();

private:
	class pin64_frame_t {
	public:
		pin64_frame_t() { }
		~pin64_frame_t();

		uint32_t size();

		pin64_block_t* start_block();

		void write(FILE* file);
		void clear();

		pin64_block_t& commands() { return m_commands; }
		pin64_block_t** blocks();

	private:
		uint32_t header_size();
		uint32_t directory_size();
		uint32_t blocks_size();

		void fill_directory(std::vector<uint32_t>& directory);

		pin64_block_t* m_current_block;
		std::vector<pin64_block_t*> m_blocks;
		pin64_block_t m_commands;

		static const uint8_t FRAME_ID[8];
	};

	void write(FILE* file);
	pin64_t::pin64_frame_t* start_frame();

	uint32_t header_size();
	uint32_t directory_size();
	uint32_t frames_size();

	void fill_directory(std::vector<uint32_t>& directory);
	void init_capture_index();

	FILE *m_capture_file;
	int32_t m_capture_index;
	int m_capture_frames;

	pin64_frame_t* m_current_frame;
	std::vector<pin64_frame_t*> m_frames;

	static const uint8_t CAP_ID[8];
};

#endif // PIN64_H