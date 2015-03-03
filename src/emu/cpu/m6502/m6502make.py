#!/usr/bin/python

USAGE = """
Usage:
%s device_name {opc.lst|-} disp.lst device.inc
"""
import sys
import logging

MAX_STATES = 0

def load_opcodes(fname):
    """Load opcodes from .lst file"""
    opcodes = []
    logging.info("load_opcodes: %s", fname)
    try:
        f = open(fname, "rU")
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
            opcodes[-1][1].append(line)
        else:
            # add new opcode
            opcodes.append((line, []))
    return opcodes


def load_disp(fname):
    logging.info("load_disp: %s", fname)
    states = []
    try:
        f = open(fname, "rU")
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
    print >>f, text,

FULL_PROLOG="""\
void %(device)s::%(opcode)s_full()
{
"""

FULL_EPILOG="""\
}
"""

FULL_EAT_ALL="""\
\ticount=0; inst_substate = %(substate)s; return;
"""

FULL_MEMORY="""\
\tif(icount == 0) { inst_substate = %(substate)s; return; }
%(ins)s
\ticount--;
"""

FULL_NONE="""\
%(ins)s
"""

PARTIAL_PROLOG="""\
void %(device)s::%(opcode)s_partial()
{
switch(inst_substate) {
case 0:
"""

PARTIAL_EPILOG="""\
}
\tinst_substate = 0;
}

"""

PARTIAL_EAT_ALL="""\
\ticount=0; inst_substate = %(substate)s; return;
case %(substate)s:;
"""

PARTIAL_MEMORY="""\
\tif(icount == 0) { inst_substate = %(substate)s; return; }
case %(substate)s:
%(ins)s
\ticount--;
"""

PARTIAL_NONE="""\
%(ins)s
"""
def identify_line_type(ins):
    if "eat-all-cycles" in ins: return "EAT"
    for s in ["read", "write", "prefetch(", "prefetch_noirq("]:
        if s in ins:
            return "MEMORY"
    return "NONE"


def save_opcodes(f, device, opcodes):
    for name, instructions in opcodes:
        d = { "device": device,
              "opcode": name,
              }

        emit(f, FULL_PROLOG % d)
        substate = 1
        for ins in instructions:
            d["substate"] = str(substate)
            d["ins"] =  ins
            line_type = identify_line_type(ins)
            if line_type == "EAT":
                emit(f, FULL_EAT_ALL % d)
                substate += 1
            elif line_type == "MEMORY":
                emit(f, FULL_MEMORY % d)
                substate += 1
            else:
                emit(f, FULL_NONE %d)
        emit(f, FULL_EPILOG % d)

        emit(f, PARTIAL_PROLOG % d)
        substate = 1
        for ins in instructions:
            d["substate"] = str(substate)
            d["ins"] =  ins
            line_type = identify_line_type(ins)
            if line_type == "EAT":
                emit(f, PARTIAL_EAT_ALL % d)
                substate += 1
            elif line_type == "MEMORY":
                emit(f, PARTIAL_MEMORY % d)
                substate += 1
            else:
                emit(f, PARTIAL_NONE %d)
        emit(f, PARTIAL_EPILOG % d)


DO_EXEC_FULL_PROLOG="""\
void %(device)s::do_exec_full()
{
\tswitch(inst_state) {
"""

DO_EXEC_FULL_EPILOG="""\
\t}
}
"""

DO_EXEC_PARTIAL_PROLOG="""\
void %(device)s::do_exec_partial()
{
\tswitch(inst_state) {
"""

DO_EXEC_PARTIAL_EPILOG="""\
\t}
}
"""

DISASM_PROLOG="""\
const %(device)s::disasm_entry %(device)s::disasm_entries[0x%(disasm_count)x] = {
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
            emit(f, "\tcase 0x%02x: %s_full(); break;\n" % (n, state))
        else:
            emit(f, "\tcase %s: %s_full(); break;\n" % ("STATE_RESET", state))
    emit(f, DO_EXEC_FULL_EPILOG % d)

    emit(f, DO_EXEC_PARTIAL_PROLOG % d)
    for n, state in enumerate(states):
        if state == ".": continue
        if n < total_states - 1:
            emit(f, "\tcase 0x%02x: %s_partial(); break;\n" % (n, state))
        else:
            emit(f, "\tcase %s: %s_partial(); break;\n" % ("STATE_RESET", state))
    emit(f, DO_EXEC_PARTIAL_EPILOG % d)

    emit(f, DISASM_PROLOG % d )
    for n, state in enumerate(states):
        if state == ".": continue
        if n == total_states - 1: break
        tokens = state.split("_")
        opc = tokens[0]
        mode = tokens[-1]
        extra = "0"
        if opc in ["jsr", "bsr"]:
            extra =  "DASMFLAG_STEP_OVER"
        elif opc in ["rts", "rti", "rtn"]:
            extra = "DASMFLAG_STEP_OUT"
        emit(f, '\t{ "%s", DASM_%s, %s },\n' % (opc, mode, extra))
    emit(f, DISASM_EPILOG % d)

def save(fname, device, opcodes, states):
    logging.info("saving: %s", fname)
    try:
        f = open(fname, "w")
    except Exception:
        err = sys.exc_info()[1]
        logging.error("cannot write file %s [%s]", fname, err)
        sys.exit(1)
    save_opcodes(f,device, opcodes)
    emit(f, "\n")
    save_tables(f, device, states)
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


    if len(argv) != 5:
        print USAGE % argv[0]
        return 1

    device_name = argv[1]

    opcodes = []
    if argv[2] !=  "-":
        opcodes = load_opcodes(argv[2])
        logging.info("found %d opcodes", len(opcodes))
    else:
        logging.info("skipping opcode reading")


    states = load_disp(argv[3])
    logging.info("loaded %s states", len(states))

    assert (len(states) & 0xff) == 1
    save(argv[4], device_name, opcodes, states)


# ======================================================================
if __name__ == "__main__":
    sys.exit(main(sys.argv))

