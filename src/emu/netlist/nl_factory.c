// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    nl_factory.c

    Discrete netlist implementation.

****************************************************************************

    Couriersud reserves the right to license the code under a less restrictive
    license going forward.

    Copyright Nicola Salmoria and the MAME team
    All rights reserved.

    Redistribution and use of this code or any derivative works are permitted
    provided that the following conditions are met:

    * Redistributions may not be sold, nor may they be used in a commercial
    product or activity.

    * Redistributions that are modified from the original source must include the
    complete source code, including the source code for all components used by a
    binary built from the modified sources. However, as a special exception, the
    source code distributed need not include anything that is normally distributed
    (in either source or binary form) with the major components (compiler, kernel,
    and so on) of the operating system on which the executable runs, unless that
    component itself accompanies the executable.

    * Redistributions must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.


****************************************************************************/

#include "nl_factory.h"
#include "nl_setup.h"

// ----------------------------------------------------------------------------------------
// net_device_t_base_factory
// ----------------------------------------------------------------------------------------

ATTR_COLD const nl_util::pstring_list net_device_t_base_factory::term_param_list()
{
	if (m_def_param.startsWith("+"))
		return nl_util::split(m_def_param.substr(1), ",");
	else
		return nl_util::pstring_list();
}

ATTR_COLD const nl_util::pstring_list net_device_t_base_factory::def_params()
{
	if (m_def_param.startsWith("+") || m_def_param.equals("-"))
		return nl_util::pstring_list();
	else
		return nl_util::split(m_def_param, ",");
}


netlist_factory_t::netlist_factory_t()
{
}

netlist_factory_t::~netlist_factory_t()
{
	for (net_device_t_base_factory * const *e = m_list.first(); e != NULL; e = m_list.next(e))
	{
		net_device_t_base_factory *p = *e;
		global_free(p);
	}
	m_list.clear();
}

netlist_device_t *netlist_factory_t::new_device_by_classname(const pstring &classname) const
{
	for (net_device_t_base_factory * const *e = m_list.first(); e != NULL; e = m_list.next(e))
	{
		net_device_t_base_factory *p = *e;
		if (strcmp(p->classname(), classname) == 0)
		{
			netlist_device_t *ret = p->Create();
			return ret;
		}
		p++;
	}
	return NULL; // appease code analysis
}

netlist_device_t *netlist_factory_t::new_device_by_name(const pstring &name, netlist_setup_t &setup) const
{
	net_device_t_base_factory *f = factory_by_name(name, setup);
	return f->Create();
}

net_device_t_base_factory * netlist_factory_t::factory_by_name(const pstring &name, netlist_setup_t &setup) const
{
	for (net_device_t_base_factory * const *e = m_list.first(); e != NULL; e = m_list.next(e))
	{
		net_device_t_base_factory *p = *e;
		if (strcmp(p->name(), name) == 0)
		{
			return p;
		}
		p++;
	}
	setup.netlist().error("Class %s not found!\n", name.cstr());
	return NULL; // appease code analysis
}
