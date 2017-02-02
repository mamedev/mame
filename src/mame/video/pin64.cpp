// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "pin64.h"

#define CAP_NAME "pin64_%d.cap"

// pin64_fileutil_t members

void pin64_fileutil_t::write(FILE* file, uint32_t data) {
	if (!file)
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

void pin64_fileutil_t::write(FILE* file, const uint8_t* data, uint32_t size) {
	if (!file)
		return;

	fwrite(data, 1, size, file);
}



// pin64_data_t members

void pin64_data_t::put8(uint8_t data) {
	m_data.push_back(data);
	m_offset++;
}

void pin64_data_t::put16(uint16_t data) {
	put8((uint8_t)(data >> 8));
	put8((uint8_t)data);
}

void pin64_data_t::put32(uint32_t data) {
	put16((uint16_t)(data >> 16));
	put16((uint16_t)data);
}

void pin64_data_t::put64(uint64_t data) {
	put32((uint32_t)(data >> 32));
	put32((uint32_t)data);
}

uint8_t pin64_data_t::get8() {
	if (m_offset >= m_data.size())
		fatalerror("PIN64: Call to pin64_data_t::get8() at end of block (requested offset %x, size %x)\n", m_offset, (uint32_t)m_data.size());

	uint8_t ret = m_data[m_offset];
	m_offset++;

	return ret;
}

uint16_t pin64_data_t::get16() {
	uint16_t ret = (uint16_t)get8() << 8;
	return ret | get8();
}

uint32_t pin64_data_t::get32() {
	uint32_t ret = (uint32_t)get16() << 16;
	return ret | get16();
}

uint64_t pin64_data_t::get64() {
	uint64_t ret = (uint64_t)get32() << 32;
	return ret | get32();
}

uint8_t pin64_data_t::get8(uint32_t offset, bool temp_access) {
	update_offset(offset, temp_access);

	uint8_t ret = get8();
	m_offset = m_old_offset;
	return ret;
}

uint16_t pin64_data_t::get16(uint32_t offset, bool temp_access) {
	update_offset(offset, temp_access);

	uint16_t ret = get16();
	m_offset = m_old_offset;
	return ret;
}

uint32_t pin64_data_t::get32(uint32_t offset, bool temp_access) {
	update_offset(offset, temp_access);

	uint32_t ret = get32();
	m_offset = m_old_offset;
	return ret;
}

uint64_t pin64_data_t::get64(uint32_t offset, bool temp_access) {
	update_offset(offset, temp_access);

	uint32_t ret = get64();
	m_offset = m_old_offset;
	return ret;
}

void pin64_data_t::reset() {
	m_old_offset = 0;
	m_offset = 0;
}

void pin64_data_t::clear() {
	reset();
	m_data.clear();
}

void pin64_data_t::update_offset(uint32_t offset, bool update_current) {
	m_old_offset = (update_current ? offset : m_offset);
	m_offset = offset;
}



// pin64_block_t members

void pin64_block_t::finalize() {
	if (m_data.size() > 0)
		m_crc32 = util::crc32_creator::simple(m_data.bytes(), m_data.size());
	else
		m_crc32 = ~0;
	m_data.reset();
}

void pin64_block_t::clear() {
	m_crc32 = 0;
	m_data.clear();
}

void pin64_block_t::print() {
	printf("            CRC32: %08x\n", (uint32_t)m_crc32); fflush(stdout);
	printf("            Data Size: %08x\n", (uint32_t)m_data.size()); fflush(stdout);
	printf("            Data: "); fflush(stdout);

	const uint32_t data_size = m_data.size();
	const uint32_t row_count = (data_size + 31) / 32;
	const uint8_t* bytes = m_data.bytes();
	for (uint32_t row = 0; row < row_count; row++) {
		const uint32_t row_index = row * 32;
		const uint32_t data_remaining = data_size - row_index;
		const uint32_t col_count = (data_remaining > 32 ? 32 : data_remaining);
		for (uint32_t col = 0; col < col_count; col++)
		{
			printf("%02x ", bytes[row_index + col]); fflush(stdout);
		}

		if (row == (row_count - 1)) {
			printf("\n"); fflush(stdout);
		} else {
			printf("\n                  "); fflush(stdout);
		}
	}
	printf("\n"); fflush(stdout);
}

void pin64_command_block_t::print() {
	printf("        CRC32: %08x\n", (uint32_t)m_crc32); fflush(stdout);
	printf("        Data Size: %08x\n", (uint32_t)m_data.size()); fflush(stdout);

	uint32_t cmd_index = 0;
	while (m_data.offset() < m_data.size()) {
		printf("        Command %d:\n", cmd_index); fflush(stdout);
		const uint32_t cmd_size(m_data.get32());
		printf("            Packet Data Size: %d words\n", cmd_size); fflush(stdout);
		printf("            Packet Data: "); fflush(stdout);
		for (int i = 0; i < cmd_size; i++) {
			const uint64_t cmd_entry(m_data.get64());
			printf("%08x%08x\n", uint32_t(cmd_entry >> 32), (uint32_t)cmd_entry); fflush(stdout);

			if (i < (cmd_size - 1))
				printf("                         ");
		}

		const uint8_t data_flag(m_data.get8());
		bool data_block_present(data_flag != 0);
		printf("            Data Block Present: %02x (%s)\n", data_flag, data_block_present ? "Yes" : "No"); fflush(stdout);

		if (data_block_present) {
			printf("            Data Block CRC32: %08x\n", m_data.get32()); fflush(stdout);
		}

		cmd_index++;
	}

	m_data.reset();
};

void pin64_block_t::write(FILE* file) {
	pin64_fileutil_t::write(file, m_crc32);
	pin64_fileutil_t::write(file, m_data.size());
	if (m_data.size() > 0)
		pin64_fileutil_t::write(file, m_data.bytes(), m_data.size());
}

uint32_t pin64_block_t::size() {
	return sizeof(uint32_t) // data CRC32
	     + sizeof(uint32_t) // data size
	     + m_data.size();   // data
}



// pin64_t members

const uint8_t pin64_t::CAP_ID[8]  = { 'P', 'I', 'N', '6', '4', 'C', 'A', 'P' };

pin64_t::~pin64_t() {
	if (m_capture_file != nullptr)
		finish();

	clear();
}

void pin64_t::start(int frames)
{
	if (m_capture_index == ~0)
		init_capture_index();

	if (m_capture_file)
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

pin64_command_block_t* pin64_t::start_command_block() {
	if (!m_capture_file)
		return nullptr;

	if (m_current_cmdblock)
		m_current_cmdblock->finalize();

	m_cmdblocks.push_back(new pin64_command_block_t());
	return m_cmdblocks[m_cmdblocks.size() - 1];
}

void pin64_t::play(int index)
{
}

void pin64_t::mark_frame(running_machine& machine) {
	if (m_capture_file) {
		if (m_cmdblocks.size() == m_capture_frames && m_capture_frames > 0) {
			finish();
		} else {
			m_current_cmdblock = start_command_block();
		}
	}

#if PIN64_ENABLE_CAPTURE
	if (machine.input().code_pressed_once(KEYCODE_N) && !m_capture_file) {
		start(1);
		machine.popmessage("Capturing PIN64 snapshot to pin64_%d.cap", m_capture_index - 1);
	} else if (machine.input().code_pressed_once(KEYCODE_M)) {
		if (m_capture_file) {
			finish();
			machine.popmessage("Done recording.");
		} else {
			start();
			machine.popmessage("Recording PIN64 movie to pin64_%d.cap", m_capture_index - 1);
		}
	}
#endif
}

void pin64_t::command(uint64_t* cmd_data, uint32_t size) {
	if (!capturing())
		return;

	if (!m_current_cmdblock)
		return;

	m_current_cmdblock->data()->put32(size);

	for (uint32_t i = 0 ; i < size; i++)
		m_current_cmdblock->data()->put64(cmd_data[i]);

	const uint8_t cmd_byte((cmd_data[0] >> 56) & 0x3f);
	if (cmd_byte == 0x30 || cmd_byte == 0x33 || cmd_byte == 0x34)
		m_current_cmdblock->data()->put8(0xff);
	else
		m_current_cmdblock->data()->put8(0x00);
}

void pin64_t::data_begin() {
	if (!capturing() || !m_current_cmdblock)
		return;

	m_blocks.push_back(new pin64_block_t());
	m_current_block = m_blocks[m_blocks.size() - 1];
}

pin64_data_t* pin64_t::data_block() {
	if (!capturing() || !m_current_cmdblock)
		return &m_dummy_data;

	return m_current_block->data();
}

void pin64_t::data_end() {
	if (!capturing() || !m_current_cmdblock)
		return;

	m_current_block->finalize();

	m_current_cmdblock->data()->put32(m_current_block->crc32());

	const uint32_t last_index = m_blocks.size() - 1;
	if (m_blocks.size() > 1) {
		int i = 0;
		for (i = 0; i < last_index; i++) {
			if (m_blocks[i]->crc32() == block().crc32()) {
				break;
			}
		}
		if (i != last_index) {
			m_blocks.erase(m_blocks.begin() + last_index);
		}
	}
}

uint32_t pin64_t::size() {
	return header_size() + sizeof(uint32_t) * (m_blocks.size() + m_cmdblocks.size() + 2) + blocks_size() + cmdblocks_size();
}

uint32_t pin64_t::header_size() {
	return sizeof(uint8_t) * 8  // "PIN64CAP"
	       + sizeof(uint32_t)   // total file size
	       + sizeof(uint32_t)   // start of data block directory data
	       + sizeof(uint32_t)   // start of command-list directory data
	       + sizeof(uint32_t)   // start of data blocks
	       + sizeof(uint32_t);  // start of command list
}

uint32_t pin64_t::blocks_size() {
	uint32_t block_size = 0;
	for (pin64_block_t* block : m_blocks)
		block_size += block->size();

	return block_size;
}

uint32_t pin64_t::cmdblocks_size() {
	uint32_t cmdblock_size = 0;
	for (pin64_command_block_t* cmdblock : m_cmdblocks)
		cmdblock_size += cmdblock->size();

	return cmdblock_size;
}

void pin64_t::print()
{
	printf("Total Size:       %9x bytes\n", size()); fflush(stdout);
	printf("Header Size:      %9x bytes\n", header_size()); fflush(stdout);
	printf("Cmdlist Dir Size: %9x bytes\n", uint32_t(m_cmdblocks.size() * sizeof(uint32_t))); fflush(stdout);
	printf("Datablk Dir Size: %9x bytes\n", uint32_t(m_blocks.size() * sizeof(uint32_t))); fflush(stdout);
	printf("Cmdlist Size:     %9x bytes\n", cmdblocks_size()); fflush(stdout);
	printf("Datablk Size:     %9x bytes\n", blocks_size()); fflush(stdout);

	printf("Command-List Count: %d\n", (uint32_t)m_cmdblocks.size()); fflush(stdout);
	for (int i = 0; i < m_cmdblocks.size(); i++) {
		printf("    List %d:\n", i); fflush(stdout);

		m_cmdblocks[i]->print();
		if (i == (m_cmdblocks.size() - 1))
		{
			printf("\n"); fflush(stdout);
		}
	}

	printf("\nData Block Count: %d\n", (uint32_t)m_blocks.size()); fflush(stdout);
	for (int i = 0; i < m_blocks.size(); i++) {
		printf("    Block %d:\n", i); fflush(stdout);

		m_blocks[i]->print();
		if (i == (m_blocks.size() - 1))
		{
			printf("\n"); fflush(stdout);
		}
	}
}

void pin64_t::write(FILE* file) {
	const uint32_t size_total = size();
	const uint32_t size_header = header_size();
	const uint32_t size_block_dir = uint32_t((m_blocks.size() + 1) * sizeof(uint32_t));
	const uint32_t size_cmdblock_dir = uint32_t((m_cmdblocks.size() + 1) * sizeof(uint32_t));
	const uint32_t size_blocks_dir = blocks_size();

	pin64_fileutil_t::write(file, CAP_ID, 8);
	pin64_fileutil_t::write(file, size_total);
	pin64_fileutil_t::write(file, size_header);
	pin64_fileutil_t::write(file, size_header + size_block_dir);
	pin64_fileutil_t::write(file, size_header + size_block_dir + size_cmdblock_dir);
	pin64_fileutil_t::write(file, size_header + size_block_dir + size_cmdblock_dir + size_blocks_dir);

	write_block_directory(file);
	write_cmdblock_directory(file);

	for (pin64_block_t* block : m_blocks)
		block->write(file);

	for (pin64_command_block_t* block : m_cmdblocks)
		block->write(file);
}

void pin64_t::write_block_directory(FILE* file) {
	pin64_fileutil_t::write(file, m_blocks.size());
	uint32_t offset(header_size() + (m_blocks.size() + m_cmdblocks.size() + 2));
	for (pin64_block_t* block : m_blocks) {
		pin64_fileutil_t::write(file, offset);
		offset += block->size();
	}
}

void pin64_t::write_cmdblock_directory(FILE* file) {
	pin64_fileutil_t::write(file, m_cmdblocks.size());
	uint32_t offset(header_size() + (m_blocks.size() + m_cmdblocks.size() + 2) * sizeof(uint32_t) + blocks_size());
	for (pin64_command_block_t* block : m_cmdblocks) {
		pin64_fileutil_t::write(file, offset);
		offset += block->size();
	}
}

void pin64_t::clear() {
	if (m_capture_file != nullptr) {
		fclose(m_capture_file);
		m_capture_file = nullptr;
	}

	for (pin64_block_t* block : m_blocks)
		delete block;

	m_blocks.clear();
	m_current_block = nullptr;

	for (pin64_command_block_t* block : m_cmdblocks)
		delete block;

	m_cmdblocks.clear();
	m_current_cmdblock = nullptr;
}

void pin64_t::init_capture_index()
{
	char name_buf[256];
	bool found = true;

	m_capture_index = 0;

	do {
		sprintf(name_buf, CAP_NAME, m_capture_index);

		FILE* temp = fopen(name_buf, "rb");
		if (temp == nullptr) {
			break;
		} else {
			fclose(temp);
			m_capture_index++;
		}
	} while(found);
}
