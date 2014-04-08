// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    netlib.c

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

#include "net_lib.h"
#include "nld_system.h"

NETLIST_START(diode_models)
	NET_MODEL(".model 1N914 D(Is=2.52n Rs=.568 N=1.752 Cjo=4p M=.4 tt=20n Iave=200m Vpk=75 mfg=OnSemi type=silicon)")
	NET_MODEL(".model 1N4148 D(Is=2.52n Rs=.568 N=1.752 Cjo=4p M=.4 tt=20n Iave=200m Vpk=75 mfg=OnSemi type=silicon)")
NETLIST_END()

NETLIST_START(bjt_models)
	NET_MODEL(".MODEL BC237B NPN(IS=1.8E-14 ISE=5.0E-14 ISC=1.72E-13 XTI=3 BF=400 BR=35.5 IKF=0.14 IKR=0.03 XTB=1.5 VAF=80 VAR=12.5 VJE=0.58 VJC=0.54 RE=0.6 RC=0.25 RB=0.56 CJE=13E-12 CJC=4E-12 XCJC=0.75 FC=0.5 NF=0.9955 NR=1.005 NE=1.46 NC=1.27 MJE=0.33 MJC=0.33 TF=0.64E-9 TR=50.72E-9 EG=1.11 KF=0 AF=1 VCEO=45V ICRATING=100M MFG=ZETEX)")
	NET_MODEL(".model BC556B PNP(IS=3.83E-14 NF=1.008 ISE=1.22E-14 NE=1.528 BF=344.4 IKF=0.08039 VAF=21.11 NR=1.005 ISC=2.85E-13 NC=1.28 BR=14.84 IKR=0.047 VAR=32.02 RB=1 IRB=1.00E-06 RBM=1 RE=0.6202 RC=0.5713 XTB=0 EG=1.11 XTI=3 CJE=1.23E-11 VJE=0.6106 MJE=0.378 TF=5.60E-10 XTF=3.414 VTF=5.23 ITF=0.1483 PTF=0 CJC=1.08E-11 VJC=0.1022 MJC=0.3563 XCJC=0.6288 TR=1.00E-32 CJS=0 VJS=0.75 MJS=0.333 FC=0.8027 Vceo=65 Icrating=100m mfg=Philips)")
	NET_MODEL(".model 2SA1015 PNP(Is=295.1E-18 Xti=3 Eg=1.11 Vaf=100 Bf=110 Xtb=1.5 Br=10.45 Rc=15 Cjc=66.2p Mjc=1.054 Vjc=.75 Fc=.5 Cje=5p Mje=.3333 Vje=.75 Tr=10n Tf=1.661n VCEO=45V ICrating=150M MFG=Toshiba)")
	NET_MODEL(".model 2SC1815 NPN(Is=2.04f Xti=3 Eg=1.11 Vaf=6 Bf=400 Ikf=20m Xtb=1.5 Br=3.377 Rc=1 Cjc=1p Mjc=.3333 Vjc=.75 Fc=.5 Cje=25p Mje=.3333 Vje=.75 Tr=450n Tf=20n Itf=0 Vtf=0 Xtf=0 VCEO=45V ICrating=150M MFG=Toshiba)")
NETLIST_END()


#define xstr(s) # s
#define ENTRY1(_nic, _name, _defparam) register_device<_nic>( # _name, xstr(_nic), _defparam );
#define ENTRY(_nic, _name, _defparam) ENTRY1(NETLIB_NAME(_nic), _name, _defparam)

netlist_factory_t::netlist_factory_t()
{
}

netlist_factory_t::~netlist_factory_t()
{
	for (net_device_t_base_factory * const *e = m_list.first(); e != NULL; e = m_list.next(e))
	{
		net_device_t_base_factory *p = *e;
		delete p;
	}
	m_list.clear();
}

void netlist_factory_t::initialize()
{
	ENTRY(R,                    RES,                    "R")
	ENTRY(POT,                  POT,                    "R")
	ENTRY(C,                    CAP,                    "C")
	ENTRY(D,                    DIODE,                  "model")
	ENTRY(VCVS,                 VCVS,                   "-")        // FIXME: STD parameters ?
	ENTRY(VCCS,                 VCCS,                   "-")
	ENTRY(QBJT_EB,              QBJT_EB,                "model")
	ENTRY(QBJT_switch,          QBJT_SW,                "model")
	ENTRY(ttl_input,            TTL_INPUT,              "IN")
	ENTRY(analog_input,         ANALOG_INPUT,           "IN")
	ENTRY(log,                  LOG,                    "+I")
	ENTRY(logD,                 LOGD,                   "+I,I2")
	ENTRY(clock,                CLOCK,                  "FREQ")
	ENTRY(mainclock,            MAINCLOCK,              "FREQ")
	ENTRY(solver,               SOLVER,                 "FREQ")
	ENTRY(gnd,                  GND,                    "-")
	ENTRY(switch2,              SWITCH2,                "-")
	ENTRY(nicRSFF,              NETDEV_RSFF,            "+S,R")
	ENTRY(7400,                 TTL_7400_NAND,          "+A,B")
	ENTRY(7402,                 TTL_7402_NOR,           "+A,B")
	ENTRY(7404,                 TTL_7404_INVERT,        "+A")
	ENTRY(7410,                 TTL_7410_NAND,          "+A,B,C")
	ENTRY(7420,                 TTL_7420_NAND,          "+A,B,C,D")
	ENTRY(7425,                 TTL_7425_NOR,           "+A,B,C,D")
	ENTRY(7427,                 TTL_7427_NOR,           "+A,B,C")
	ENTRY(7430,                 TTL_7430_NAND,          "+A,B,C,D,E,F,G,H")
	ENTRY(7450,                 TTL_7450_ANDORINVERT,   "+A,B,C,D")
	ENTRY(7486,                 TTL_7486_XOR,           "+A,B")
	ENTRY(7448,                 TTL_7448,               "+A,B,C,D,LTQ,BIQ,RBIQ")
	ENTRY(7474,                 TTL_7474,               "+CLK,D,CLRQ,PREQ")
	ENTRY(7483,                 TTL_7483,               "+A1,A2,A3,A4,B1,B2,B3,B4,C0")
	ENTRY(7490,                 TTL_7490,               "+A,B,R1,R2,R91,R92")
	ENTRY(7493,                 TTL_7493,               "+CLKA,CLKB,R1,R2")
	ENTRY(74107,                TTL_74107,              "+CLK,J,K,CLRQ")
	ENTRY(74107A,               TTL_74107A,             "+CLK,J,K,CLRQ")
	ENTRY(74153,                TTL_74153,              "+C0,C1,C2,C3,A,B,G")
	ENTRY(SN74LS629,            SN74LS629,              "CAP")
	ENTRY(9316,                 TTL_9316,               "+CLK,ENP,ENT,CLRQ,LOADQ,A,B,C,D")
	ENTRY(NE555,                NE555,                  "-")
	ENTRY(4066_dip,             CD_4066_DIP,            "-")
	ENTRY(7400_dip,             TTL_7400_DIP,           "-")
	ENTRY(7402_dip,             TTL_7402_DIP,           "-")
	ENTRY(7404_dip,             TTL_7404_DIP,           "-")
	ENTRY(7410_dip,             TTL_7410_DIP,           "-")
	ENTRY(7420_dip,             TTL_7420_DIP,           "-")
	ENTRY(7425_dip,             TTL_7425_DIP,           "-")
	ENTRY(7427_dip,             TTL_7427_DIP,           "-")
	ENTRY(7430_dip,             TTL_7430_DIP,           "-")
	ENTRY(7448_dip,             TTL_7448_DIP,           "-")
	ENTRY(7450_dip,             TTL_7450_DIP,           "-")
	ENTRY(7474_dip,             TTL_7474_DIP,           "-")
	ENTRY(7483_dip,             TTL_7483_DIP,           "-")
	ENTRY(7486_dip,             TTL_7486_DIP,           "-")
	ENTRY(7490_dip,             TTL_7490_DIP,           "-")
	ENTRY(7493_dip,             TTL_7493_DIP,           "-")
	ENTRY(74107_dip,            TTL_74107_DIP,          "-")
	ENTRY(74153_dip,            TTL_74153_DIP,          "-")
	ENTRY(9316_dip,             TTL_9316_DIP,           "-")
	ENTRY(SN74LS629_dip,        SN74LS629_DIP,          "1.CAP1,2.CAP2")
	ENTRY(NE555_dip,            NE555_DIP,              "-")
}

netlist_device_t *netlist_factory_t::new_device_by_classname(const pstring &classname, netlist_setup_t &setup) const
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
	setup.netlist().error("Class %s not found!\n", classname.cstr());
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
