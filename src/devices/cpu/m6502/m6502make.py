#!/usr/bin/python
# license:BSD-3-Clause
# copyright-holders:Olivier Galibert

from __future__ import print_function

import io
import logging
import sys

USAGE = """
Usage:
%s prefix {opc.lst|-} disp.lst device.inc deviced.inc
"""
MAX_STATES = 0

def load_opcodes(fname):
    """Load opcodes from .lst file"""
    opcodes = []
    logging.info("load_opcodes: %s", fname)
    try:
        f = io.open(fname, "r")
    except Exception:
        err = sys.exc_info()[1]
        logging.error("cannot read opcodes file %s [%s]", fname, err)
        sys.exit(1)

    for line in f:
        if line.startswith("#"): continue
        line = line.rstrip()
        if not line: continue
        if line.startswith(" ") or line.startswith("\t"):
            # append instruction to last opcode
            if line == '\tprefetch();':
                opcodes[-1][1].append("\tprefetch_start();")
                opcodes[-1][1].append("\tIR = mintf->read_sync(PC);")
                opcodes[-1][1].append("\tprefetch_end();")
            elif line == '\tprefetch_noirq();':
                opcodes[-1][1].append("\tprefetch_start();")
                opcodes[-1][1].append("\tIR = mintf->read_sync(PC);")
                opcodes[-1][1].append("\tprefetch_end_noirq();")
            else:
                opcodes[-1][1].append(line)
        else:
            # add new opcode
            opcodes.append((line, []))
    return opcodes


def load_disp(fname):
    logging.info("load_disp: %s", fname)
    states = []
    try:
        f = io.open(fname, "r")
    except Exception:
        err = sys.exc_info()[1]
        logging.error("cannot read display file %s [%s]", fname, err)
        sys.exit(1)
    for line in f:
        if line.startswith("#"): continue
        line = line.strip()
        if not line: continue
        tokens = line.split()
        states += tokens
    return states

def emit(f, text):
    """write string to file"""
    print(text, file=f)


def identify_line_type(ins):
    if "eat-all-cycles" in ins: return "EAT"
    for s in ["read", "write"]:
        if s in ins:
            return "MEMORY"
    return "NONE"


def save_opcodes(f, device, opcodes):
    for name, instructions in opcodes:
        emit(f, "void %s_device::%s_full()" % (device, name))
        emit(f, "{")
        substate = 1
        for ins in instructions:
            line_type = identify_line_type(ins)
            if line_type == "EAT":
                emit(f, "\tdebugger_wait_hook();")
                emit(f, "\ticount = 0;")
                emit(f, "\tinst_substate = %d;" % substate)
                emit(f, "\treturn;")
                substate += 1
            elif line_type == "MEMORY":
                emit(f, ins)
                emit(f, "\ticount--;")
                emit(f, "\tif(icount <= 0) {")
                emit(f, "\t\tif(access_to_be_redone()) {")
                emit(f, "\t\t\ticount++;")
                emit(f, "\t\t\tinst_substate = %d;" % substate)
                emit(f, "\t\t} else")
                emit(f, "\t\t\tinst_substate = %d;" % (substate+1))
                emit(f, "\t\treturn;")
                emit(f, "\t}")
                substate += 2
            else:
                emit(f, ins)
        emit(f, "}")
        emit(f, "")

        emit(f, "void %s_device::%s_partial()" % (device, name))
        emit(f, "{")
        emit(f, "\tswitch(inst_substate) {")
        emit(f, "case 0:")
        substate = 1
        for ins in instructions:
            line_type = identify_line_type(ins)
            if line_type == "EAT":
                emit(f, "\tdebugger_wait_hook();")
                emit(f, "\ticount = 0;")
                emit(f, "\tinst_substate = %d;" % substate)
                emit(f, "\treturn;")
                emit(f, "\tcase %d:;" % substate)
                substate += 1
            elif line_type == "MEMORY":
                emit(f, "\t[[fallthrough]];")
                emit(f, "case %d:" % substate)
                emit(f, ins)
                emit(f, "\ticount--;")
                emit(f, "\tif(icount <= 0) {")
                emit(f, "\t\tif(access_to_be_redone()) {")
                emit(f, "\t\t\ticount++;")
                emit(f, "\t\t\tinst_substate = %d;" % substate)
                emit(f, "\t\t} else")
                emit(f, "\t\t\tinst_substate = %d;" % (substate+1))
                emit(f, "\t\treturn;")
                emit(f, "\t}")
                emit(f, "\t[[fallthrough]];")
                emit(f, "case %d:;" % (substate+1))
                substate += 2
            else:
                emit(f, ins)
        emit(f, "\tbreak;")
        emit(f, "}")
        emit(f, "\tinst_substate = 0;")
        emit(f, "}")
        emit(f, "")


DO_EXEC_FULL_PROLOG="""\
void %(device)s_device::do_exec_full()
{
\tswitch(inst_state) {
"""

DO_EXEC_FULL_EPILOG="""\
\t}
}
"""

DO_EXEC_PARTIAL_PROLOG="""\
void %(device)s_device::do_exec_partial()
{
\tswitch(inst_state) {
"""

DO_EXEC_PARTIAL_EPILOG="""\
\t}
}
"""

DISASM_PROLOG="""\
const %(device)s_disassembler::disasm_entry %(device)s_disassembler::disasm_entries[0x%(disasm_count)x] = {
"""

DISASM_EPILOG="""\
};
"""

def save_tables(f, device, states):
    total_states = len(states)

    d = { "device": device,
          "disasm_count": total_states-1
          }

    emit(f, DO_EXEC_FULL_PROLOG % d)
    for n, state in enumerate(states):
        if state == ".": continue
        if n < total_states - 1:
            emit(f, "\tcase 0x%02x: %s_full(); break;" % (n, state))
        else:
            emit(f, "\tcase %s: %s_full(); break;" % ("STATE_RESET", state))
    emit(f, DO_EXEC_FULL_EPILOG % d)

    emit(f, DO_EXEC_PARTIAL_PROLOG % d)
    for n, state in enumerate(states):
        if state == ".": continue
        if n < total_states - 1:
            emit(f, "\tcase 0x%02x: %s_partial(); break;" % (n, state))
        else:
            emit(f, "\tcase %s: %s_partial(); break;" % ("STATE_RESET", state))
    emit(f, DO_EXEC_PARTIAL_EPILOG % d)

def save_dasm(f, device, states):
    total_states = len(states)

    d = { "device": device,
          "disasm_count": total_states-1
          }

    emit(f, DISASM_PROLOG % d )
    for n, state in enumerate(states):
        if state == ".": continue
        if n == total_states - 1: break
        tokens = state.split("_")
        opc = tokens[0]
        mode = tokens[-1]
        extra = "0"
        if opc in ["jsr", "bsr", "callf", "jpi", "jsb"]:
            extra = "STEP_OVER"
        elif opc in ["rts", "rti", "rtn", "retf", "tpi"]:
            extra = "STEP_OUT"
        elif opc in ["bcc", "bcs", "beq", "bmi", "bne", "bpl", "bvc", "bvs", "bbr", "bbs", "bbc", "bar", "bas"]:
            extra = "STEP_COND"
        emit(f, '\t{ "%s", DASM_%s, %s },' % (opc, mode, extra))
    emit(f, DISASM_EPILOG % d)

def saves(fname, device, opcodes, states):
    logging.info("saving: %s", fname)
    try:
        f = open(fname, "w")
    except Exception:
        err = sys.exc_info()[1]
        logging.error("cannot write file %s [%s]", fname, err)
        sys.exit(1)
    save_opcodes(f, device, opcodes)
    emit(f, "\n")
    save_tables(f, device, states)
    f.close()


def saved(fname, device, opcodes, states):
    logging.info("saving: %s", fname)
    try:
        f = open(fname, "w")
    except Exception:
        err = sys.exc_info()[1]
        logging.error("cannot write file %s [%s]", fname, err)
        sys.exit(1)
    save_dasm(f, device, states)
    f.close()


def main(argv):
    debug = False
    logformat=("%(levelname)s:"
               "%(module)s:"
               "%(lineno)d:"
               "%(threadName)s:"
               "%(message)s")
    if debug:
        logging.basicConfig(level=logging.INFO, format=logformat)
    else:
        logging.basicConfig(level=logging.WARNING, format=logformat)


    if len(argv) != 6:
        print(USAGE % argv[0])
        return 1

    mode = argv[1]
    device_name = argv[2]

    opcodes = []
    if argv[3] !=  "-":
        opcodes = load_opcodes(argv[3])
        logging.info("found %d opcodes", len(opcodes))
    else:
        logging.info("skipping opcode reading")


    states = load_disp(argv[4])
    logging.info("loaded %s states", len(states))

    assert (len(states) & 0xff) == 1
    if mode == 's':
        saves(argv[5], device_name, opcodes, states)
    else:
        saved(argv[5], device_name, opcodes, states)


# ======================================================================
if __name__ == "__main__":
    sys.exit(main(sys.argv))

