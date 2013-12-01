/*
 * nld_log.c
 *
 */

#include "nld_log.h"

NETLIB_START(log)
{
    register_input("I", m_I);

    pstring filename = "netlist_" + name() + ".log";
    m_file = fopen(filename, "w");
}

NETLIB_UPDATE(log)
{
    fprintf(m_file, "%e %e\n", netlist().time().as_double(), INPANALOG(m_I));
}

NETLIB_NAME(log)::~NETLIB_NAME(log)()
{
    fclose(m_file);
}
