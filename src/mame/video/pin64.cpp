// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "pin64.h"

#define CAP_NAME "pin64_%d.cap"

// pin64_fileutil_t members

void pin64_fileutil_t::write(FILE* file, uint32_t data)
{
	if (file == nullptr)
		return;

	uint8_t temp(data >> 24);
	fwrite(&temp, 1, 1, file);

	temp = (uint8_t)(data >> 16);
	fwrite(&temp, 1, 1, file);

	temp = (uint8_t)(data >> 8);
	fwrite(&temp, 1, 1, file);

	temp = (uint8_t)data;
	fwrite(&temp, 1, 1, file);
}

void pin64_fileutil_t::write(FILE* file, const uint8_t* data, uint32_t size)
{
	if (file == nullptr)
		return;

	fwrite(data, 1, size, file);
}

// pin64_block_t members

void pin64_block_t::put8(uint8_t data)
{
	m_data.push_back(data);
	m_offset++;
}

void pin64_block_t::put16(uint16_t data)
{
	put8((uint8_t)(data >> 8));
	put8((uint8_t)data);
}

void pin64_block_t::put32(uint32_t data)
{
	put16((uint16_t)(data >> 16));
	put16((uint16_t)data);
}

uint8_t pin64_block_t::get8()
{
	if (m_offset >= m_data.size())
		fatalerror("PIN64: Call to get8() at end of block (requested offset %x, size %x)\n", m_offset, (uint32_t)m_data.size());

	uint8_t ret = m_data[m_offset];
	m_offset++;

	return ret;
}

uint16_t pin64_block_t::get16()
{
	uint16_t ret = 0;
	ret |= get8() << 8;
	ret |= get8();

	return ret;
}

uint32_t pin64_block_t::get32()
{
	uint32_t ret = 0;
	ret |= get16() << 16;
	ret |= get16();

	return ret;
}

uint8_t pin64_block_t::get8(uint32_t offset, bool update_current)
{
	update_offset(offset, update_current);

	uint8_t ret = get8();
	m_offset = m_old_offset;
	return ret;
}

uint16_t pin64_block_t::get16(uint32_t offset, bool update_current)
{
	update_offset(offset, update_current);

	uint16_t ret = get16();
	m_offset = m_old_offset;
	return ret;
}

uint32_t pin64_block_t::get32(uint32_t offset, bool update_current)
{
	update_offset(offset, update_current);

	uint32_t ret = get32();
	m_offset = m_old_offset;
	return ret;
}

uint32_t pin64_block_t::data_size()
{
	return m_data.size();
}

uint32_t pin64_block_t::size()
{
	return sizeof(uint32_t) * 2 + data_size();
}

void pin64_block_t::clear()
{
	reset();
	m_crc32 = 0;
	m_data.clear();
}

void pin64_block_t::reset()
{
	m_old_offset = 0;
	m_offset = 0;
}

void pin64_block_t::finalize()
{
	if (m_data.size() > 0)
		m_crc32 = util::crc32_creator::simple(&m_data[0], m_data.size());
	else
		m_crc32 = ~0;
	reset();
}

void pin64_block_t::write(FILE* file)
{
	pin64_fileutil_t::write(file, m_crc32);
	pin64_fileutil_t::write(file, m_data.size());
	if (m_data.size() > 0)
		pin64_fileutil_t::write(file, &m_data[0], m_data.size());
}

void pin64_block_t::update_offset(uint32_t offset, bool update_current)
{
	m_old_offset = (update_current ? offset : m_offset);
	m_offset = offset;
}

// pin64_frame_t members

const uint8_t pin64_t::pin64_frame_t::FRAME_ID[8]  = { 'P', 'I', 'N', '6', '4', 'F', 'R', 'M' };

pin64_t::pin64_frame_t::~pin64_frame_t()
{
	clear();
}

uint32_t pin64_t::pin64_frame_t::size()
{
	return header_size() + directory_size() + blocks_size() + m_commands.size();
}

uint32_t pin64_t::pin64_frame_t::header_size()
{
	return sizeof(uint8_t) * 8  // "PIN64FRM"
	       + sizeof(uint32_t)   // total file size
	       + sizeof(uint32_t)   // start of directory data
	       + sizeof(uint32_t)   // start of block data
	       + sizeof(uint32_t);  // start of command data
}

uint32_t pin64_t::pin64_frame_t::directory_size()
{
	return m_blocks.size() * sizeof(uint32_t);
}

uint32_t pin64_t::pin64_frame_t::blocks_size()
{
	uint32_t block_size = 0;
	for (pin64_block_t* block : m_blocks)
		block_size += block->size();

	return block_size;
}

pin64_block_t* pin64_t::pin64_frame_t::start_block()
{
	m_blocks.push_back(new pin64_block_t());
	return m_blocks[m_blocks.size() - 1];
}

void pin64_t::pin64_frame_t::write(FILE* file) {
	const uint32_t size_total = size();
	const uint32_t size_header = header_size();
	const uint32_t size_dir = directory_size();
	const uint32_t size_blocks = blocks_size();

	std::vector<uint32_t> directory;
	fill_directory(directory);

	pin64_fileutil_t::write(file, FRAME_ID, 8);
	pin64_fileutil_t::write(file, size_total);
	pin64_fileutil_t::write(file, size_header);
	pin64_fileutil_t::write(file, size_header + size_dir);
	pin64_fileutil_t::write(file, size_header + size_dir + size_blocks);

	for (uint32_t entry : directory)
		pin64_fileutil_t::write(file, entry);

	for (pin64_block_t* block : m_blocks)
		block->write(file);

	m_commands.write(file);
}

void pin64_t::pin64_frame_t::fill_directory(std::vector<uint32_t>& directory) {
	uint32_t offset = header_size() + directory_size();
	for (pin64_block_t* block : m_blocks) {
		directory.push_back(offset);
		offset += block->size();
	}
}

void pin64_t::pin64_frame_t::clear() {
	for (pin64_block_t* block : m_blocks)
		delete block;

	m_blocks.clear();
	m_commands.clear();

	m_current_block = nullptr;
}

pin64_block_t** pin64_t::pin64_frame_t::blocks() {
	return (m_blocks.size() == 0) ? nullptr : &m_blocks[0];
}

// pin64_t members

const uint8_t pin64_t::CAP_ID[8]  = { 'P', 'I', 'N', '6', '4', 'C', 'A', 'P' };

pin64_t::~pin64_t()
{
	if (m_capture_file != nullptr)
		finish();

	clear();
}

void pin64_t::start(int frames)
{
	if (m_capture_index == ~0)
		init_capture_index();

	if (m_capture_file != nullptr)
		fatalerror("PIN64: Call to start() while already capturing\n");

	char name_buf[256];
	sprintf(name_buf, CAP_NAME, m_capture_index);
	m_capture_index++;

	m_capture_file = fopen(name_buf, "wb");

	m_capture_frames = frames;
}

void pin64_t::finish() {
	if (!m_capture_file)
		return;

	write(m_capture_file);
	fclose(m_capture_file);
	m_capture_file = nullptr;

	clear();
}

pin64_t::pin64_frame_t* pin64_t::start_frame()
{
	if (!m_capture_file)
		return nullptr;

	m_frames.push_back(new pin64_frame_t());
	return m_frames[m_frames.size() - 1];
}

void pin64_t::mark_frame(running_machine& machine) {
	if (m_capture_file) {
		if (m_frames.size() == m_capture_frames) {
			finish();
		} else {
			m_current_frame = start_frame();
		}
	}

	if (machine.input().code_pressed_once(KEYCODE_N) && !m_capture_file) {
		start(1);
	} else if (machine.input().code_pressed_once(KEYCODE_M)) {
		if (m_capture_file) {
			finish();
		} else {
			start();
		}
	}
}

uint32_t pin64_t::size()
{
	return header_size() + directory_size() + frames_size();
}

uint32_t pin64_t::header_size()
{
	return sizeof(uint8_t) * 8  // "PIN64CAP"
	       + sizeof(uint32_t)   // total file size
	       + sizeof(uint32_t)   // start of directory data
	       + sizeof(uint32_t);  // start of frame data
}

uint32_t pin64_t::directory_size()
{
	return m_frames.size() * sizeof(uint32_t);
}

uint32_t pin64_t::frames_size()
{
	uint32_t frames_size = 0;
	for (pin64_frame_t* frame : m_frames)
		frames_size += frame->size();

	return frames_size;
}

void pin64_t::write(FILE* file) {
	const uint32_t size_total = size();
	const uint32_t size_header = header_size();
	const uint32_t size_dir = directory_size();
	const uint32_t size_frames = frames_size();

	std::vector<uint32_t> directory;
	fill_directory(directory);

	pin64_fileutil_t::write(file, CAP_ID, 8);
	pin64_fileutil_t::write(file, size_total);
	pin64_fileutil_t::write(file, size_header);
	pin64_fileutil_t::write(file, size_header + size_dir);
	pin64_fileutil_t::write(file, size_header + size_dir + size_frames);

	for (uint32_t entry : directory)
		pin64_fileutil_t::write(file, entry);

	for (pin64_frame_t* frame : m_frames)
		frame->write(file);
}

void pin64_t::fill_directory(std::vector<uint32_t>& directory) {
	uint32_t offset = header_size() + directory_size();
	for (pin64_frame_t* frame : m_frames) {
		directory.push_back(offset);
		offset += frame->size();
	}
}

void pin64_t::clear() {
	if (m_capture_file != nullptr) {
		fclose(m_capture_file);
		m_capture_file = nullptr;
	}

	for (pin64_frame_t* frame : m_frames)
		delete frame;

	m_frames.clear();
	m_current_frame = nullptr;
}

void pin64_t::init_capture_index()
{
	char name_buf[256];
	bool found = true;

	m_capture_index = 0;

	do
	{
		sprintf(name_buf, CAP_NAME, m_capture_index);

		FILE* temp = fopen(name_buf, "rb");
		if (temp == nullptr)
		{
			break;
		}
		else
		{
			fclose(temp);
			m_capture_index++;
		}
	} while(found);
}
