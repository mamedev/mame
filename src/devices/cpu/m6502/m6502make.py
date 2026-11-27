#!/usr/bin/python
# license:BSD-3-Clause
# copyright-holders:Olivier Galibert

from __future__ import print_function

import io
import logging
import os
import re
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
                opcodes[-1][1].append("\tm_IR = read_sync(m_PC);")
                opcodes[-1][1].append("\tprefetch_end();")
            elif line == '\tprefetch_noirq();':
                opcodes[-1][1].append("\tprefetch_start();")
                opcodes[-1][1].append("\tm_IR = read_sync(m_PC);")
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
    if "read" in ins: return "MEMORY_READ"
    if "write" in ins: return "MEMORY_WRITE"
    return "NONE"


def samples_interrupt(ins, single_cycle):
    return (single_cycle or "read_sync" not in ins) and "_noirq" not in ins


# CPUs polled where NESdev "CPU interrupts" documents it: once, in the final cycle of an instruction (before its last
# access); a branch before its operand fetch and, when taken across a page, before the PCH fixup, but not before its
# third cycle; never during the interrupt sequence itself (an NMI there either takes over the vector or is taken after
# the handler's first instruction). The other NMOS cores poll before every access of an instruction.
DOCUMENTED_POLL_DEVICES = {"rp2a03_core"}


def poll_lines(device, name, instructions, single_cycle):
    """indices of the instruction lines that poll interrupts before their access"""
    cands = [i for i, ins in enumerate(instructions)
             if identify_line_type(ins) in ("MEMORY_READ", "MEMORY_WRITE") and samples_interrupt(ins, single_cycle)]
    if device not in DOCUMENTED_POLL_DEVICES:
        return set(cands)
    if name.startswith("brk"):
        return set()
    if re.match(r"b(cc|cs|eq|mi|ne|pl|vc|vs)_rel$", name):
        # the operand fetch, and the PCH fixup read of a taken branch across a page; not the cycle in between
        return {i for i in cands if "m_TMP = read_pc()" in instructions[i] or "read_arg(" in instructions[i]}
    return {cands[-1]} if cands else set()


RDY_GATED_DEVICES = {"m6502", "m6510", "rp2a03_core"}
# every cycle spent halted on RDY is reported to dma_halt_cycle() with the address of the read being repeated
# (a no-op except on CPUs with a DMA unit that needs it, like the 2A03)
RDY_HALT_HOOK_DEVICES = RDY_GATED_DEVICES


# NESdev forum "PAL DMA behavior" (Fiskbit, lidnariq, Apr 2025; bus traces of an RP2A07 doing DMC DMA): the 2A07's DMA
# only halts the CPU on the first cycle of an instruction: it uses the 6502 core's SYNC output instead of R/W, so every
# cycle after the opcode fetch is treated the way the 2A03 treats a write cycle, and the DMA waits for the next opcode
# fetch. SYNC is also asserted on the first cycle of an interrupt sequence (an opcode fetch) and stays asserted while
# the CPU is halted there, so OAM DMA and DMC DMA during OAM DMA behave as on the 2A03.
def rdy_halt_condition(device, ins):
    if device == "rp2a03_core" and "read_sync" not in ins:
        return "!m_rdy_state && !m_dma_2a07"
    return "!m_rdy_state"


def read_address(ins):
    """address expression of the memory read performed by an instruction line"""
    for fn in ("read_sync", "read_arg_noirq", "read_arg", "read_pc_noirq", "read_pc", "read"):
        m = re.search(r"\b" + fn + r"\(", ins)
        if not m:
            continue
        if fn in ("read_pc", "read_pc_noirq"):
            return "m_PC"
        depth, start = 1, m.end()
        for i in range(start, len(ins)):
            if ins[i] == "(":
                depth += 1
            elif ins[i] == ")":
                depth -= 1
                if depth == 0:
                    return ins[start:i]
    raise ValueError("no read address in: " + ins)
CMOS_DEVICES = {"w65c02", "r65c02", "r65c19", "w65c02s", "m65ce02", "m4510", "w65816"}


def save_opcodes(f, device, opcodes):
    rdy_gated = device in RDY_GATED_DEVICES
    halt_hook = device in RDY_HALT_HOOK_DEVICES
    interrupt_sampled = device not in CMOS_DEVICES
    for name, instructions in opcodes:
        single_cycle = sum(identify_line_type(ins) in ("MEMORY_READ", "MEMORY_WRITE") for ins in instructions) == 1
        polls = poll_lines(device, name, instructions, single_cycle)
        emit(f, "void %s_device::%s_full()" % (device, name))
        emit(f, "{")
        substate = 1
        for idx, ins in enumerate(instructions):
            line_type = identify_line_type(ins)
            if line_type == "EAT":
                emit(f, "\tdebugger_wait_hook();")
                emit(f, "\tm_icount = 0;")
                emit(f, "\tm_inst_substate = %d;" % substate)
                emit(f, "\treturn;")
                substate += 1
            elif line_type in ("MEMORY_READ", "MEMORY_WRITE"):
                if rdy_gated and line_type == "MEMORY_READ":
                    emit(f, "\twhile(%s) {" % rdy_halt_condition(device, ins))
                    if halt_hook:
                        emit(f, "\t\tdma_halt_cycle(%s);" % read_address(ins))
                    emit(f, "\t\tm_icount--;")
                    emit(f, "\t\tif(m_icount <= 0) {")
                    emit(f, "\t\t\tm_inst_substate = %d;" % substate)
                    emit(f, "\t\t\treturn;")
                    emit(f, "\t\t}")
                    emit(f, "\t}")
                if interrupt_sampled and idx in polls:
                    emit(f, "\tsample_interrupt();")
                emit(f, ins)
                emit(f, "\tm_icount--;")
                emit(f, "\tif(m_icount <= 0) {")
                emit(f, "\t\tif(access_to_be_redone()) {")
                emit(f, "\t\t\tm_icount++;")
                emit(f, "\t\t\tm_inst_substate = %d;" % (substate+1))
                emit(f, "\t\t} else")
                emit(f, "\t\t\tm_inst_substate = %d;" % (substate+2))
                emit(f, "\t\treturn;")
                emit(f, "\t}")
                substate += 3
            else:
                emit(f, ins)
        emit(f, "}")
        emit(f, "")

        emit(f, "void %s_device::%s_partial()" % (device, name))
        emit(f, "{")
        emit(f, "\tswitch(m_inst_substate) {")
        emit(f, "case 0:")
        substate = 1
        for idx, ins in enumerate(instructions):
            line_type = identify_line_type(ins)
            if line_type == "EAT":
                emit(f, "\tdebugger_wait_hook();")
                emit(f, "\tm_icount = 0;")
                emit(f, "\tm_inst_substate = %d;" % substate)
                emit(f, "\treturn;")
                emit(f, "\tcase %d:;" % substate)
                substate += 1
            elif line_type in ("MEMORY_READ", "MEMORY_WRITE"):
                emit(f, "\t[[fallthrough]];")
                emit(f, "case %d:" % substate)
                if rdy_gated and line_type == "MEMORY_READ":
                    emit(f, "\twhile(%s) {" % rdy_halt_condition(device, ins))
                    if halt_hook:
                        emit(f, "\t\tdma_halt_cycle(%s);" % read_address(ins))
                    emit(f, "\t\tm_icount--;")
                    emit(f, "\t\tif(m_icount <= 0) {")
                    emit(f, "\t\t\tm_inst_substate = %d;" % substate)
                    emit(f, "\t\t\treturn;")
                    emit(f, "\t\t}")
                    emit(f, "\t}")
                if interrupt_sampled and idx in polls:
                    emit(f, "\tsample_interrupt();")
                emit(f, "\t[[fallthrough]];")
                emit(f, "case %d:" % (substate+1))
                emit(f, ins)
                emit(f, "\tm_icount--;")
                emit(f, "\tif(m_icount <= 0) {")
                emit(f, "\t\tif(access_to_be_redone()) {")
                emit(f, "\t\t\tm_icount++;")
                emit(f, "\t\t\tm_inst_substate = %d;" % (substate+1))
                emit(f, "\t\t} else")
                emit(f, "\t\t\tm_inst_substate = %d;" % (substate+2))
                emit(f, "\t\treturn;")
                emit(f, "\t}")
                emit(f, "\t[[fallthrough]];")
                emit(f, "case %d:;" % (substate+2))
                substate += 3
            else:
                emit(f, ins)
        emit(f, "\tbreak;")
        emit(f, "}")
        emit(f, "\tm_inst_substate = 0;")
        emit(f, "}")
        emit(f, "")


DO_EXEC_FULL_PROLOG="""\
void %(device)s_device::do_exec_full()
{
\tswitch(m_inst_state) {
"""

DO_EXEC_FULL_EPILOG="""\
\t}
}
"""

DO_EXEC_PARTIAL_PROLOG="""\
void %(device)s_device::do_exec_partial()
{
\tswitch(m_inst_state) {
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
        if opc in ["jsr", "bsr", "callf", "jpi", "jsb", "jsl"]:
            extra = "STEP_OVER"
        elif opc in ["rts", "rti", "rtn", "retf", "tpi", "rtl"]:
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

    # A CPU polled at the documented points gets its own copy of every instruction its dispatch uses, generated with that
    # rule, taking the ones its own list does not override from the base NMOS list beside it
    if device_name in DOCUMENTED_POLL_DEVICES and argv[3] != "-":
        own = {name for name, _ in opcodes}
        used = set(states)
        for name, instructions in load_opcodes(os.path.join(os.path.dirname(argv[3]), "om6502.lst")):
            if name in used and name not in own:
                opcodes.append((name, instructions))

    assert (len(states) & 0xff) == 1
    if mode == 's':
        saves(argv[5], device_name, opcodes, states)
    else:
        saved(argv[5], device_name, opcodes, states)


# ======================================================================
if __name__ == "__main__":
    sys.exit(main(sys.argv))
