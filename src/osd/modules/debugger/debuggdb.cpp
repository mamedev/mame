// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss
//============================================================
//
//  debuggdb.cpp - gdbserver interface
//
//============================================================

#include "emu.h"
#include "debug_module.h"
#include "modules/osdmodule.h"

#include "debug/debugcpu.h"
#include "debug/debugcon.h"
#include "debugger.h"

#define LOG(...) printf(__VA_ARGS__)
//#define LOG(...) 

class debug_gdb : public osd_module, public debug_module
{
public:
	debug_gdb()
	: osd_module(OSD_DEBUG_PROVIDER, "gdb"), debug_module(),
		m_machine(nullptr)
	{
	}

	virtual ~debug_gdb() { }

	virtual int init(const osd_options &options) override { return 0; }
	virtual void exit() override { }

	virtual void init_debugger(running_machine &machine) override;
	virtual void wait_for_debugger(device_t &device, bool firststop) override;
	virtual void debugger_update() override;

private:
	void handle_packet(const std::string& buf);
	void send_ack(const std::string& ack);
	void send_response(std::string response);

	bool useAck{ true };

	running_machine *m_machine;
	emu_file connection{ OPEN_FLAG_CREATE | OPEN_FLAG_READ | OPEN_FLAG_WRITE };
};

void debug_gdb::init_debugger(running_machine &machine)
{
	m_machine = &machine;
	if(connection.open("socket.127.0.0.1:2159") == osd_file::error::NONE) {
		LOG("connection open on 127.0.0.1:2159\n");
	}
}

void debug_gdb::wait_for_debugger(device_t &device, bool firststop)
{
/*	auto pc = downcast<cpu_device*>(m_machine->debugger().cpu().get_visible_cpu())->pc();
	if(firststop)
		printf("Hlo\n");
	if(firststop && pc == 0) // don't stop at machine_start
		m_machine->debugger().cpu().get_visible_cpu()->debug()->go();
*/

	if(connection.is_open()) {
		std::array<char, 512> buffer;
		auto bytesRead = connection.read(buffer.data(), buffer.size());
		if(bytesRead > 0) {
			std::string command(buffer.data(), bytesRead);
			LOG("debug_gdb: received %d bytes: >>%s<<\n", bytesRead, command.c_str());
			handle_packet(command);
		}
	}

	// copy from debugimgui.cpp
	device.machine().osd().update(false);
}

void debug_gdb::debugger_update()
{
	if(connection.is_open()) {
		std::array<char, 512> buffer;
		auto bytesRead = connection.read(buffer.data(), buffer.size());
		if(bytesRead > 0) {
			std::string command(buffer.data(), bytesRead);
			LOG("debug_gdb: received %d bytes: >>%s<<\n", bytesRead, command.c_str());
			handle_packet(command);
		}
	}
}

namespace {
	static constexpr char hex[]{ "0123456789abcdef" };
/*	static std::string hex8(uint8_t v) {
		std::string ret;
		ret += hex[v >> 4];
		ret += hex[v & 0xf];
		return ret;
	}
	static std::string hex32(uint32_t v) {
		std::string ret;
		for(int i = 28; i >= 0; i -= 4)
			ret += hex[(v >> i) & 0xf];
		return ret;
	}*/
}

void debug_gdb::send_ack(const std::string& ack) 
{
	if(useAck && !ack.empty()) {
		LOG("debug_gdb: <- %s\n", ack.c_str());
		if(connection.write(ack.data(), ack.length()) != ack.length())
			LOG("debug_gdb: error sending ack\n");
	}
}

void debug_gdb::send_response(std::string response) 
{
	if(!response.empty()) {
		LOG("debug_gdb: <- %s\n", response.substr(1).c_str());
		uint8_t cksum{};
		for(size_t i = 1; i < response.length(); i++)
			cksum += response[i];
		response += '#';
		response += hex[cksum >> 4];
		response += hex[cksum & 0xf];
		if(connection.write(response.data(), response.length()) != response.length())
			LOG("debug_gdb: error sending data.\n");
	}
}

void debug_gdb::handle_packet(const std::string& buf)
{
	if(buf.empty())
		return;

	std::string request{ buf }, ack{}, response;
	if(request[0] == '+') {
		request = request.substr(1);
	} else if(request[0] == '-') {
		LOG("debug_gdb: client non-ack'd our last packet\n");
		request = request.substr(1);
	}
	if(!request.empty() && request[0] == 0x03) {
		// Ctrl+C
		ack = "+";
		response = "$";
		response += "S05"; // SIGTRAP
		m_machine->debugger().debug_break();
	} else if(!request.empty() && request[0] == '$') {
		ack = "-";
		auto end = request.find('#');
		if(end != std::string::npos) {
			uint8_t cksum{};
			for(size_t i = 1; i < end; i++)
				cksum += request[i];
			if(request.length() >= end + 2) {
				if(tolower(request[end + 1]) == hex[cksum >> 4] && tolower(request[end + 2]) == hex[cksum & 0xf]) {
					request = request.substr(1, end - 1);
					LOG("debug_gdb: -> %s\n", request.c_str());
					ack = "+";
					response = "$";
					if(request.substr(0, strlen("qSupported")) == "qSupported") {
						response += "PacketSize=512;BreakpointCommands+;swbreak+;hwbreak+;QStartNoAckMode+;vContSupported+;";
					} else if(request.substr(0, strlen("qAttached")) == "qAttached") {
						response += "1";
					} else if(request.substr(0, strlen("qTStatus")) == "qTStatus") {
						response += "T0";
					} else if(request.substr(0, strlen("QStartNoAckMode")) == "QStartNoAckMode") {
						send_ack(ack);
						useAck = false;
						response += "OK";
					} else if(request.substr(0, strlen("qfThreadInfo")) == "qfThreadInfo") {
						response += "m1";
					} else if(request.substr(0, strlen("qsThreadInfo")) == "qsThreadInfo") {
						response += "l";
					} else if(request.substr(0, strlen("qC")) == "qC") {
						response += "QC1";
					} else if(request.substr(0, strlen("vCont?")) == "vCont?") {
						response += "vCont;c;C;s;S;t;r";
					} else if(request.substr(0, strlen("vCont;")) == "vCont;") {
						auto actions = request.substr(strlen("vCont;"));
						while(!actions.empty()) {
							std::string action;
							// split actions by ';'
							auto semi = actions.find(';');
							if(semi != std::string::npos) {
								action = actions.substr(0, semi);
								actions = actions.substr(semi + 1);
							} else {
								action = actions;
								actions.clear();
							}
							// thread specified by ':'
							auto colon = action.find(':');
							if(colon != std::string::npos) {
								// ignore thread ID
								action = action.substr(0, colon);
							}

							// hmm.. what to do with multiple actions?!

							if(action == "s") { // single-step
								// TODO
								send_ack(ack);
								return;
							} else if(action == "c") { // continue
								m_machine->debugger().cpu().set_execution_running();
								send_ack(ack);
								return;
							} else if(action[0] == 'r') { // keep stepping in range
								auto comma = action.find(',', 3);
								if(comma != std::string::npos) {
									auto start = strtoul(action.data() + 1, nullptr, 16);
									auto end = strtoul(action.data() + comma + 1, nullptr, 16);
									// TODO
									send_ack(ack);
									return;
								}
							} else {
								LOG("debug_gdb: unknown vCont action: %s\n", action.c_str());
							}
						}
					} else if(request[0] == 'H') {
						response += "OK";
					} else if(request[0] == 'T') {
						response += "OK";
					} else if(request[0] == 'D') { // detach
						response += "OK";
					} else if(request[0] == '?') { // reason for stopping
						response += "S05"; // SIGTRAP
					} else if(request[0] == 's') { // single-step
						assert(!"should have used vCont;s");
					} else if(request[0] == 'c') { // continue
						assert(!"should have used vCont;c");
					} else if(request[0] == 'k') { // kill
						m_machine->debugger().console().execute_command("quit", false);
						return;
					} else if(request.substr(0, 2) == "Z0") { // set software breakpoint
						auto comma = request.find(',', strlen("Z0"));
						if(comma != std::string::npos) {
							auto adr = strtoul(request.data() + strlen("Z0,"), nullptr, 16);
							// TODO
/*							for(auto& bpn : bpnodes) {
								if(bpn.enabled)
									continue;
								bpn.value1 = adr;
								bpn.type = BREAKPOINT_REG_PC;
								bpn.oper = BREAKPOINT_CMP_EQUAL;
								bpn.enabled = 1;
								trace_mode = 0;
								print_breakpoints();
								response += "OK";
								break;
							}*/
						} else
							response += "E01";
					} else if(request.substr(0, 2) == "z0") { // clear software breakpoint
						auto comma = request.find(',', strlen("z0"));
						if(comma != std::string::npos) {
							auto adr = strtoul(request.data() + strlen("z0,"), nullptr, 16);
							// TODO
/*							for(auto& bpn : bpnodes) {
								if(bpn.enabled && bpn.value1 == adr) {
									bpn.enabled = 0;
									trace_mode = 0;
									print_breakpoints();
									response += "OK";
									break;
								}*/
								// TODO: error when breakpoint not found
							}
						} else
							response += "E01";
					} else if(request[0] == 'g') { // get registers
						// TODO
						//response += get_registers();
					} else if(request[0] == 'm') { // read memory
						auto comma = request.find(',');
						if(comma != std::string::npos) {
							std::string mem;
							auto adr = strtoul(request.data() + strlen("m"), nullptr, 16);
							int len = strtoul(request.data() + comma + 1, nullptr, 16);
							LOG("debug_gdb: want 0x%x bytes at 0x%x\n", len, adr);
							// TODO
							/*while(len-- > 0) {
								auto data = debug_read_memory_8(adr);
								if(data == -1) {
									LOG("debug_gdb: error reading memory at 0x%x\n", len, adr);
									response += "E01";
									mem.clear();
									break;
								}
								data &= 0xff; // custom_bget seems to have a problem?
								mem += hex[data >> 4];
								mem += hex[data & 0xf];
								adr++;
							}*/
							response += mem;
						} else
							response += "E01";
					}
				} else
					LOG("debug_gdb: packet checksum mismatch: got %c%c, want %c%c\n", tolower(request[end + 1]), tolower(request[end + 2]), hex[cksum >> 4], hex[cksum & 0xf]);
			} else
				LOG("debug_gdb: packet checksum missing\n");
		} else
			LOG("debug_gdb: packet end marker '#' not found\n");
	send_ack(ack);
	send_response(response);
}

MODULE_DEFINITION(DEBUG_GDB, debug_gdb)
