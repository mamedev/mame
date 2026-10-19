// license:BSD-3-Clause
// copyright-holders:grubbyplaya

#include "emu.h"
#include "t6m53_dasm.h"

u32 t6m53_disassembler::opcode_alignment() const {
	return 2;
}

inline u16 t6m53_disassembler::ef(u16 op) {
    return 0x100 | ((op >> 7) & 0x20) | ((op >> 4) & 0x1f);
}

const char *t6m53_disassembler::size_suffix(u16 op) {
	return BIT(op, 8) ? ".N" : ".B";
}

const char *t6m53_disassembler::compare_name(u16 op) {
	switch (op & 0x0e00) {
        case 0x0400: return "=";
        case 0x0600: return "!=";
        case 0x0c00: return "<";
        case 0x0e00: return ">=";
        default:     return "?";
	}
}

void t6m53_disassembler::print_location(std::ostream &stream, u16 location) {
	location &= 0x1ff;

	switch (location) {
        case 0x0ff: stream << "A"; return;
        case 0x100: stream << "I"; return;
        case 0x118: stream << "SP"; return;
        case 0x11c: stream << "DP"; return;
        case 0x1ff: stream << "(I)"; return;
        default: break;
	}

	stream << "R" << util::string_format("%03X", location);
}

void t6m53_disassembler::print_bit_location(std::ostream &stream, u16 op) {
	print_location(stream, (BIT(op, 9) << 8) | (((op & 0xf) << 4) | ((op >> 4) & 0xf)));
	stream << "." << (BIT(op, 8) | (BIT(op, 10) << 1));
}

void t6m53_disassembler::print_target(std::ostream &stream, u16 target) {
	if (target & 0x8000)
		stream << "CALL   $" << util::string_format("%04X", (target & 0x7fff) << 1);
	else
		stream << "JUMP   $" << util::string_format("%04X", target << 1);
}

void t6m53_disassembler::print_branch(std::ostream &stream, u16 op, u16 target, offs_t &flags) {
	switch (op & 0x0600) {
        case 0x0200:
            print_target(stream, target);
            flags |= BIT(target, 15) ? STEP_OVER : STEP_COND;
            break;

        case 0x0400:
            stream << "IF NO CARRY: ";
            print_target(stream, target);
            flags |= STEP_COND;
            break;

        case 0x0600:
            stream << "IF CARRY: ";
            print_target(stream, target);
            flags |= STEP_COND;
            break;

        default:
            break;
	}
}

offs_t t6m53_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &addrs) {
	const u16 op = (opcodes.r8(pc) << 8) | opcodes.r8(pc + 1);
	const u16 addr = (opcodes.r8(pc + 2) << 8) | opcodes.r8(pc + 3);
	offs_t flags = SUPPORTED;
	offs_t words = 1;

	auto print_mnemonic = [&](const char *mnemonic) {
		util::stream_format(stream, "%-8s", mnemonic);
	};

    if ((op & 0xFE00) == 0x0000) {
        bool first = true;
        auto append_inst = [&](const char* fmt, auto... args) {
            if (!first)
                stream << "; ";
            util::stream_format(stream, fmt, args...);
            first = false;
        };

        if (op & 0x04) {
            switch (op & 0x07) {
                case 0x04:
                    append_inst("READU%s", BIT(op, 8) ? ".N" : "");
                    break;

                case 0x05:
                    append_inst("READD%s", BIT(op, 8) ? ".N" : "");
                    break;

                case 0x06:
                    append_inst("WRITEU%s", BIT(op, 8) ? ".N" : "");
                    break;

                case 0x07:
                    append_inst("WRITED%s", BIT(op, 8) ? ".N" : "");
                    break;
            }
        } else if ((op & 0x03) == op) {
			util::stream_format(stream, "%-8s$%03X", "SYS", op & 0x1ff);
        }

        if ((op & 0x0018) == 0x0018)
            append_inst("SHR%-8sA");

        if (op & 0x0020)
            append_inst("SCANKEYS");

        if (op & 0x0040) {
            append_inst("RET");
            flags |= STEP_OUT;
        }

        if (op & 0x0080)
            append_inst("JUMP    (104)");
    } else if ((op & 0x0ff0) == 0x0f00) {
		util::stream_format(stream, "%-8s$%X", "REP", (op & 0x0f) + 1);
	} else if (op < 0x0400) {
		util::stream_format(stream, "%-8s", util::string_format("LOAD%s", size_suffix(op)));
		stream << "JYX, (I)";
	} else if (op < 0x0600) {
		util::stream_format(stream, "%-8s", util::string_format("LOAD%s", size_suffix(op)));
		stream << "(I), JYX";
	} else if (op < 0x0800) {
		util::stream_format(stream, "%-8s", util::string_format("XCHG%s", size_suffix(op)));
		stream << "(I), JYX";
	} else if (op < 0x0a00) {
		print_mnemonic("LOAD");
		stream << "I, ";
		print_location(stream, op & 0x1ff);
	} else if (op < 0x0c00) {
		words = 2;
		print_mnemonic("LOAD");
		stream << "I, ";
		print_location(stream, op & 0x1ff);
		stream << "; ";
		print_target(stream, addr);
		flags |= BIT(addr, 15) ? STEP_OVER : STEP_COND;
	} else if (op < 0x0e00) {
		print_mnemonic("LOAD");
		stream << "(I+), #$" << util::string_format("%02X", op & 0xff);
	} else if (op < 0x1000) {
		print_mnemonic("LOAD");
		print_location(stream, ef(op));
		stream << ", #$" << util::string_format("%02X", op & 0xf);
	} else if (op < 0x1400) {
		util::stream_format(stream, "%-8s", util::string_format("LOAD%s", size_suffix(op)));
		stream << "A, WYX";
	} else if (op < 0x1800) {
		util::stream_format(stream, "%-8s", util::string_format("XCHG%s", size_suffix(op)));
		stream << "A, WYX";
	} else if (op < 0x1c00) {
		util::stream_format(stream, "%-8s", util::string_format("LOAD%s", size_suffix(op)));
		stream << "WYX, A";
	} else if (op < 0x1e00) {
		print_mnemonic("LOAD");
		stream << "A, #$" << util::string_format("%02X", op & 0xff);
	} else if (op < 0x2000) {
		print_mnemonic("LOAD.N");
		print_location(stream, ef(op));
		stream << ", #$" << util::string_format("%01X", op & 0x0f);
	} else if (op < 0x2800) {
		print_mnemonic("TOGGLE");
		print_bit_location(stream, op);
	} else if (op < 0x3000) {
		print_mnemonic("CLEAR");
		print_bit_location(stream, op);
	} else if (op < 0x3800) {
		print_mnemonic("SET");
		print_bit_location(stream, op);
	} else if (op < 0x3c00) {
		util::stream_format(stream, "%-8s", util::string_format("NOT%s", size_suffix(op)));
		stream << "WYX";
	} else if (op < 0x3e00) {
		util::stream_format(stream, "%-8s", util::string_format("XOR%s", size_suffix(op)));
		stream << "JYX";
	} else if (op < 0x4000) {
		util::stream_format(stream, "%-8s", util::string_format("OR%s", size_suffix(op)));
		stream << "JYX";
	} else if (op < 0x6000) {
		words = 2;
		print_mnemonic("IF");
		print_location(stream, ef(op));
		stream << " " << compare_name(op) << " #$" << util::string_format("%02X", op & 0xff) << ": ";
		print_target(stream, addr);
		flags |= STEP_COND;
	} else if (op < 0x7000) {
		const char *name = BIT(op, 8) ? "IF.N" : "IF.B";
		words = 2;
		print_mnemonic(name);
		stream << "(I) " << compare_name(op) << " #$" << util::string_format("%02X", op & (BIT(op, 8) ? 0x0f : 0xff)) << ": ";
		print_target(stream, addr);
		flags |= STEP_COND;
	} else if (op < 0x8000) {
		const char *name = BIT(op, 8) ? "IF.N" : "IF.B";
		words = 2;
		print_mnemonic(name);
		stream << "A " << compare_name(op) << " #$" << util::string_format("%02X", op & (BIT(op, 8) ? 0x0f : 0xff)) << ": ";
		print_target(stream, addr);
		flags |= STEP_COND;
	} else if (op < 0xa000) {
		const char *mnemonic;
		switch (op & 0x1800) {
            case 0x0000: mnemonic = "ADD"; break;
            case 0x0800: mnemonic = "DADD"; break;
            case 0x1000: mnemonic = "SUB"; break;
            default:     mnemonic = "DSUB"; break;
		}

		util::stream_format(stream, "%-8s", util::string_format("%s%s", mnemonic, size_suffix(op)));
		stream << "JYX";

		if ((op & 0x0600) != 0) {
			words = 2;
			stream << "; ";
			print_branch(stream, op, addr, flags);
		}
	} else if (op < 0xb000) {
		words = 2;
		print_mnemonic("IF");
		stream << (BIT(op, 11) ? "SET   " : "CLEAR ");
		print_bit_location(stream, op);
		stream << ": ";
		print_target(stream, addr);
		flags |= STEP_COND;
	} else if (op == 0xb000) {
		stream << "BREAK";
	} else if (op < 0xc000) {
		const char *name = BIT(op, 8) ? "IF.N" : "IF.B";
		words = 2;
		print_mnemonic(name);
		stream << "JYX " << compare_name(op) << " $" << util::string_format("%02X", op & 0xff) << ": ";
		print_target(stream, addr);
		flags |= STEP_COND;
	} else if (op < 0xe000) {
        const char *mnemonic = (op & 0x0800) ? "DADD" : "ADD";
		print_mnemonic(mnemonic);
		stream << "JYX, #$" << util::string_format("%01X", op & 0xf);

		if ((op & 0x0600) != 0)
		{
			words = 2;
			stream << "; ";
			print_branch(stream, op, addr, flags);
		}
	} else if (op < 0xf000) {
		const char *mnemonic = (op & 0x1000) ? "SUB" : "ADD";
		util::stream_format(stream, "%-8s", util::string_format("%s%s", mnemonic, size_suffix(op)));
		stream << "(I), #$" << util::string_format("%02X", op & (BIT(op, 8) ? 0x0f : 0xff));

		if ((op & 0x0600) != 0) {
			words = 2;
			stream << "; ";
			print_branch(stream, op, addr, flags);
		}
	} else {
		const char *mnemonic = (op & 0x1000) ? "SUB" : "ADD";
		util::stream_format(stream, "%-8s", util::string_format("%s%s", mnemonic, size_suffix(op)));
		stream << "A, #$" << util::string_format("%02X", op & (BIT(op, 8) ? 0x0f : 0xff));

		if ((op & 0x0600) != 0) {
			words = 2;
			stream << "; ";
			print_branch(stream, op, addr, flags);
		}
	}

	return (words * 2) | flags;
}
