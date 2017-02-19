// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
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



// pin64_printer_t members

void pin64_printer_t::print_data(pin64_block_t* block) {
	pin64_data_t* data = block->data();

	printf("            CRC32: %08x\n", (uint32_t)block->crc32()); fflush(stdout);
	printf("            Data Size: %08x\n", (uint32_t)data->size()); fflush(stdout);
	printf("            Data: "); fflush(stdout);

	const uint32_t data_size = data->size();
	const uint32_t row_count = (data_size + 31) / 32;
	const uint8_t* bytes = data->bytes();
	for (uint32_t row = 0; row < row_count; row++) {
		const uint32_t row_index = row * 32;
		const uint32_t data_remaining = data_size - row_index;
		const uint32_t col_count = (data_remaining > 32 ? 32 : data_remaining);
		for (uint32_t col = 0; col < col_count; col++) {
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

void pin64_printer_t::print_command(int cmd_start, int cmd, std::unordered_map<util::crc32_t, pin64_block_t*>& blocks, std::vector<util::crc32_t>& commands) {
	pin64_block_t* block = blocks[commands[cmd]];
	pin64_data_t* data = block->data();

	printf("        Command %d:\n", cmd - cmd_start); fflush(stdout);
	const uint32_t cmd_size(data->get32());
	printf("        CRC32: %08x\n", (uint32_t)commands[cmd]); fflush(stdout);
	printf("            Packet Data Size: %d words\n", cmd_size); fflush(stdout);
	printf("            Packet Data: "); fflush(stdout);

	bool load_command = false;
	for (int i = 0; i < cmd_size; i++) {
		const uint64_t cmd_entry(data->get64());
		if (i == 0) {
			const uint8_t top_byte = uint8_t(cmd_entry >> 56) & 0x3f;
			if (top_byte == 0x30 || top_byte == 0x33 || top_byte == 0x34)
				load_command = true;
		}
		printf("%08x%08x\n", uint32_t(cmd_entry >> 32), (uint32_t)cmd_entry); fflush(stdout);

		if (i < (cmd_size - 1)) {
			printf("                         "); fflush(stdout);
		}
	}

	printf("            Data Block Present: %s\n", load_command ? "Yes" : "No"); fflush(stdout);

	if (load_command) {
		printf("            Data Block CRC32: %08x\n", data->get32()); fflush(stdout);
	}

	data->reset();
};



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
	if (m_capture_file)
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

	m_frames.push_back(0);
}

void pin64_t::finish() {
	if (!m_capture_file)
		return;

	finalize();
	print();

	write(m_capture_file);
	fclose(m_capture_file);
	m_capture_file = nullptr;

	clear();
}

void pin64_t::finalize() {
	finish_command();
	data_end();
}

void pin64_t::play(int index) {
}

void pin64_t::mark_frame(running_machine& machine) {
	if (m_capture_file) {
		if (m_frames.size() == m_capture_frames && m_capture_frames > 0) {
			printf("\n");
			finish();
			machine.popmessage("Done recording.");
		} else {
			printf("%d ", (uint32_t)m_commands.size());
			m_frames.push_back((uint32_t)m_commands.size());
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

	finish_command();

	m_current_command = new pin64_block_t();
	m_current_command->data()->put32(size);

	for (uint32_t i = 0 ; i < size; i++)
		m_current_command->data()->put64(cmd_data[i]);
}

void pin64_t::finish_command() {
	if (!m_current_command)
		return;

	m_current_command->finalize();
	if (m_blocks.find(m_current_command->crc32()) == m_blocks.end())
		m_blocks[m_current_command->crc32()] = m_current_command;

	m_commands.push_back(m_current_command->crc32());
}

void pin64_t::data_begin() {
	if (!capturing())
		return;

	if (m_current_data)
		data_end();

	m_current_data = new pin64_block_t();
}

pin64_data_t* pin64_t::data_block() {
	if (!capturing() || !m_current_data)
		return &m_dummy_data;

	return m_current_data->data();
}

void pin64_t::data_end() {
	if (!capturing() || !m_current_data)
		return;

	m_current_data->finalize();
	m_current_command->data()->put32(m_current_data->crc32());
	finish_command();

	if (m_blocks.find(m_current_data->crc32()) == m_blocks.end())
		m_blocks[m_current_data->crc32()] = m_current_data;

	m_current_data = nullptr;
}

size_t pin64_t::size() {
	return header_size() + block_directory_size() + cmdlist_directory_size() + cmdlist_size();
}

size_t pin64_t::header_size() {
	return sizeof(uint8_t) * 8  // "PIN64CAP"
		   + sizeof(uint32_t)   // total file size
		   + sizeof(uint32_t)   // start of block directory data
		   + sizeof(uint32_t)   // start of command-list directory data
		   + sizeof(uint32_t)   // start of blocks
		   + sizeof(uint32_t);  // start of commands
}

size_t pin64_t::block_directory_size() {
	return (m_blocks.size() + 1) * sizeof(uint32_t);
}

size_t pin64_t::cmdlist_directory_size() {
	return (m_frames.size() + 1) * sizeof(uint16_t);
}

size_t pin64_t::blocks_size() {
	size_t block_size = 0;
	for (std::pair<util::crc32_t, pin64_block_t*> block_pair : m_blocks)
		block_size += (block_pair.second)->size();

	return block_size;
}

size_t pin64_t::cmdlist_size() {
	return (m_commands.size() + 1) * sizeof(uint32_t);
}

void pin64_t::print()
{
	printf("Total Size:       %9x bytes\n", (uint32_t)size()); fflush(stdout);
	printf("Header Size:      %9x bytes\n", (uint32_t)header_size()); fflush(stdout);
	printf("Block Dir Size:   %9x bytes\n", (uint32_t)block_directory_size()); fflush(stdout);
	printf("Cmdlist Dir Size: %9x bytes\n", (uint32_t)cmdlist_directory_size()); fflush(stdout);
	printf("Blocks Size:      %9x bytes\n", (uint32_t)blocks_size()); fflush(stdout);
	printf("Cmdlist Size:     %9x bytes\n", (uint32_t)cmdlist_size()); fflush(stdout);

	printf("Command-List Count: %d\n", (uint32_t)m_frames.size()); fflush(stdout);
	for (int i = 0; i < m_frames.size(); i++) {
		printf("    List %d:\n", i); fflush(stdout);

		const int next_start = ((i == (m_frames.size() - 1)) ? m_commands.size() : m_frames[i+1]);
		for (int cmd = m_frames[i]; cmd < next_start; cmd++) {
			pin64_printer_t::print_command(m_frames[i], cmd, m_blocks, m_commands);
		}
		if (i == (m_frames.size() - 1)) {
			printf("\n"); fflush(stdout);
		}
	}

	printf("\nData Block Count: %d\n", (uint32_t)m_blocks.size()); fflush(stdout);
	int i = 0;
	for (std::pair<util::crc32_t, pin64_block_t*> block_pair : m_blocks) {
		printf("    Block %d:\n", i); fflush(stdout);

		pin64_printer_t::print_data((block_pair.second));
		if (i == (m_blocks.size() - 1)) {
			printf("\n"); fflush(stdout);
		}
		i++;
	}
}

void pin64_t::write(FILE* file) {
	const uint32_t size_total = size();
	const uint32_t size_header = header_size();
	const uint32_t size_block_dir = block_directory_size();
	const uint32_t size_cmdlist_dir = cmdlist_directory_size();
	const uint32_t size_blocks_dir = blocks_size();

	pin64_fileutil_t::write(file, CAP_ID, 8);
	pin64_fileutil_t::write(file, size_total);
	pin64_fileutil_t::write(file, size_header);
	pin64_fileutil_t::write(file, size_header + size_block_dir);
	pin64_fileutil_t::write(file, size_header + size_block_dir + size_cmdlist_dir);
	pin64_fileutil_t::write(file, size_header + size_block_dir + size_cmdlist_dir + size_blocks_dir);

	write_data_directory(file);
	write_cmdlist_directory(file);

	for (std::pair<util::crc32_t, pin64_block_t*> block_pair : m_blocks)
		(block_pair.second)->write(file);

	pin64_fileutil_t::write(file, m_commands.size());
	for (util::crc32_t crc : m_commands)
		pin64_fileutil_t::write(file, crc);
}

void pin64_t::write_data_directory(FILE* file) {
	pin64_fileutil_t::write(file, m_blocks.size());
	size_t offset(header_size());
	for (std::pair<util::crc32_t, pin64_block_t*> block_pair : m_blocks) {
		pin64_fileutil_t::write(file, offset);
		offset += (block_pair.second)->size();
	}
}

void pin64_t::write_cmdlist_directory(FILE* file) {
	pin64_fileutil_t::write(file, m_frames.size());
	for (uint32_t frame : m_frames)
		pin64_fileutil_t::write(file, frame);
}

void pin64_t::clear() {
	if (m_capture_file != nullptr) {
		fclose(m_capture_file);
		m_capture_file = nullptr;
	}

	for (std::pair<util::crc32_t, pin64_block_t*> block_pair : m_blocks)
		delete block_pair.second;

	m_blocks.clear();
	m_commands.clear();
	m_frames.clear();

	m_current_data = nullptr;
	m_current_command = nullptr;
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
