#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>
#include "osdcomm.h"

#include <string>
#include <vector>

using namespace std;

enum { STATES = 0x101 };

const char *snames[STATES-0x100] = {
	"STATE_RESET"
};

struct opcode {
	string name;
	vector<string> instructions;
};

vector<opcode> opcodes;

struct table_t {
	string opcodes[STATES];
};

table_t table;

string device_name;

static void load_opcodes(const char *fname)
{
	char buf[4096];
	FILE *f;

	sprintf(buf, "Error opening %s for reading\n", fname);
	f = fopen(fname, "r");
	if(!f) {
		perror(buf);
		exit(1);
	}

	while(fgets(buf, sizeof(buf), f)) {
		char *p = buf;
		if(p[0] == '#' || p[0] == '\r' || p[0] == '\n' || p[0] == 0)
			continue;

		for(char *q = p; *q; q++)
			if(*q == '\r' || *q == '\n') {
				*q = 0;
				break;
			}

		if(p[0] != ' ' && p[0] != '\t') {
			opcodes.resize(opcodes.size()+1);
			opcodes.back().name = p;
		} else
			opcodes.back().instructions.push_back(p);
	}
	fclose(f);
}

static void load_disp(const char *fname)
{
	char buf[4096];
	FILE *f;

	sprintf(buf, "Error opening %s for reading\n", fname);
	f = fopen(fname, "r");
	if(!f) {
		perror(buf);
		exit(1);
	}

	int state = 0;

	while(fgets(buf, sizeof(buf), f)) {
		char *p = buf;
		if(p[0] == '#' || p[0] == '\r' || p[0] == '\n' || p[0] == 0)
			continue;

		while(state < STATES) {
			while(*p && (*p == '\n' || *p == '\r' || *p == ' ' || *p == '\t'))
				p++;
			if(!*p)
				break;

			char *q = p;
			while(*p != '\n' && *p != '\r' && *p != ' ' && *p != '\t')
				p++;
			table.opcodes[state++] = string(q, p);
		}
	}
	fclose(f);
}

enum { NONE, EAT_ALL, MEMORY };

static int identify_line_type(string inst)
{
	if(inst.find("eat-all-cycles") != string::npos)
		return EAT_ALL;
	if(inst.find("read") != string::npos ||
	   inst.find("write") != string::npos ||
	   inst.find("prefetch(") != string::npos ||
	   inst.find("prefetch_noirq(") != string::npos)
		return MEMORY;
	return NONE;
}

static void save_opcodes(FILE *f)
{
	for(unsigned int i=0; i != opcodes.size(); i++) {
		int substate;
		opcode &o = opcodes[i];

		fprintf(f, "void %s::%s_full()\n", device_name.c_str(), o.name.c_str());
		fprintf(f, "{\n");
		substate = 1;
		for(unsigned int j=0; j != o.instructions.size(); j++) {
			string inst = o.instructions[j];
			int type = identify_line_type(inst);
			if(type == EAT_ALL) {
				fprintf(f, "\ticount=0; inst_substate = %d; return;", substate);
				substate++;
			} else {
				if(type == MEMORY)
					fprintf(f, "\tif(icount == 0) { inst_substate = %d; return; }\n", substate);
				fprintf(f, "%s\n", inst.c_str());
				if(type == MEMORY) {
					fprintf(f, "\ticount--;\n");
					substate++;
				}
			}
		}
		fprintf(f, "}\n");

		fprintf(f, "void %s::%s_partial()\n", device_name.c_str(), o.name.c_str());
		fprintf(f, "{\n");
		fprintf(f, "switch(inst_substate) {\n");
		fprintf(f, "case 0:\n");
		substate = 1;
		for(unsigned int j=0; j != o.instructions.size(); j++) {
			string inst = o.instructions[j];
			int type = identify_line_type(inst);
			if(type == EAT_ALL) {
				fprintf(f, "\ticount=0; inst_substate = %d; return;", substate);
				fprintf(f, "case %d:;\n", substate);
				substate++;
			} else {
				if(type == MEMORY) {
					fprintf(f, "\tif(icount == 0) { inst_substate = %d; return; }\n", substate);
					fprintf(f, "case %d:\n", substate);
				}
				fprintf(f, "%s\n", inst.c_str());
				if(type == MEMORY) {
					fprintf(f, "\ticount--;\n");
					substate++;
				}
			}
		}
		fprintf(f, "}\n");
		fprintf(f, "\tinst_substate = 0;\n");
		fprintf(f, "}\n");
		fprintf(f, "\n");
	}
	fprintf(f, "\n");
}

static void save_tables(FILE *f)
{
	fprintf(f, "void %s::do_exec_full()\n", device_name.c_str());
	fprintf(f, "{\n");
	fprintf(f, "\tswitch(inst_state) {\n");
	for(int j=0; j<STATES; j++) {
		if(table.opcodes[j] != ".") {
			if(j < 0x100)
				fprintf(f, "\tcase 0x%02x: %s_full(); break;\n", j, table.opcodes[j].c_str());
			else
				fprintf(f, "\tcase %s: %s_full(); break;\n", snames[j-0x100], table.opcodes[j].c_str());
		}
	}
	fprintf(f, "\t}\n");
	fprintf(f, "}\n");
	fprintf(f, "void %s::do_exec_partial()\n", device_name.c_str());
	fprintf(f, "{\n");
	fprintf(f, "\tswitch(inst_state) {\n");
	for(int j=0; j<STATES; j++) {
		if(table.opcodes[j] != ".") {
			if(j < 0x100)
				fprintf(f, "\tcase 0x%02x: %s_partial(); break;\n", j, table.opcodes[j].c_str());
			else
				fprintf(f, "\tcase %s: %s_partial(); break;\n", snames[j-0x100], table.opcodes[j].c_str());
		}
	}
	fprintf(f, "\t}\n");
	fprintf(f, "}\n");
	fprintf(f, "const %s::disasm_entry %s::disasm_entries[0x100] = {\n", device_name.c_str(), device_name.c_str());
	for(int j=0; j<0x100; j++)
		if(table.opcodes[j] != ".") {
			string opcode = table.opcodes[j];
			string opc, fullopc, mode;
			string::iterator k, ke;
			for(k = opcode.begin(); k != opcode.end() && *k != '_'; k++);
			for(ke = opcode.end(); ke != opcode.begin() && ke[-1] != '_'; ke--);
			assert(k != opcode.end());
			opc = string(opcode.begin(), k);
			fullopc = string(opcode.begin(), ke-1);
			mode = string(ke, opcode.end());
			
			bool step_over = opc == "jsr" || opc == "bsr";
			bool step_out = opc == "rts" || opc == "rti" || opc == "rtn";
			bool per_bit = opc == "bbr" || opc == "bbs" || opc == "rmb" || opc == "smb";
			fprintf(f, "\t{ \"%s\", DASM_%s, %s, %s },\n",
					opc.c_str(), mode.c_str(), step_over ? "DASMFLAG_STEP_OVER" : step_out ? "DASMFLAG_STEP_OUT" : "0",
					per_bit ? "true" : "false");
		} else
			fprintf(f, "\t{ \"???\", DASM_imp, 0, false },\n");
	fprintf(f, "};\n");
}

static void save(const char *fname)
{
	char buf[4096];
	FILE *f;

	sprintf(buf, "Error opening %s for writing\n", fname);
	f = fopen(fname, "w");
	if(!f) {
		perror(buf);
		exit(1);
	}

	save_opcodes(f);
	save_tables(f);

	fclose(f);
}

int main(int argc, char *argv[])
{
	if(argc != 5) {
		fprintf(stderr, "Usage:\n%s device_name {opc.lst|-} disp.lst device.inc\n", argv[0]);
		exit(1);
	}

	device_name = argv[1];
	if(strcmp(argv[2], "-"))
	   load_opcodes(argv[2]);
	load_disp(argv[3]);
	save(argv[4]);


	return 0;
}
