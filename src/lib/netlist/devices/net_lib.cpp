// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    netlib.c

    Discrete netlist implementation.

****************************************************************************/

#include "net_lib.h"
#include "../nl_factory.h"
#include "../solver/nld_solver.h"


#define xstr(s) # s

#define NETLIB_DEVICE_DECL(chip) extern factory::constructor_ptr_t decl_ ## chip;

#define ENTRYX1(nic, name, defparam, decl) factory.register_device( decl (pstring(# name), pstring(xstr(nic)), pstring(defparam)) );
#define ENTRYX(nic, name, defparam) { NETLIB_DEVICE_DECL(nic) ENTRYX1(NETLIB_NAME(nic), name, defparam, decl_ ## nic) }

namespace netlist
{
	namespace devices
	{
	void initialize_factory(factory::list_t &factory)
{
	ENTRYX(R,                   RES,                    "R")
	ENTRYX(POT,                 POT,                    "R")
	ENTRYX(POT2,                POT2,                   "R")
	ENTRYX(C,                   CAP,                    "C")
	ENTRYX(L,                   IND,                    "L")
	ENTRYX(D,                   DIODE,                  "MODEL")
	ENTRYX(VS,                  VS,                     "V")
	ENTRYX(CS,                  CS,                     "I")
	ENTRYX(VCVS,                VCVS,                   "")
	ENTRYX(VCCS,                VCCS,                   "")
	ENTRYX(CCCS,                CCCS,                   "")
	ENTRYX(LVCCS,               LVCCS,                  "")
	ENTRYX(opamp,               OPAMP,                  "MODEL")
	ENTRYX(dummy_input,         DUMMY_INPUT,            "")
	ENTRYX(frontier,            FRONTIER_DEV,           "+I,+G,+Q")   // not intended to be used directly
	ENTRYX(function,            AFUNC,                  "N,FUNC")   // only for macro devices - NO FEEDBACK loops
	ENTRYX(QBJT_EB,             QBJT_EB,                "MODEL")
	ENTRYX(QBJT_switch,         QBJT_SW,                "MODEL")
	ENTRYX(logic_input,         TTL_INPUT,              "IN")
	ENTRYX(logic_input,         LOGIC_INPUT,            "IN,FAMILY")
	ENTRYX(analog_input,        ANALOG_INPUT,           "IN")
	ENTRYX(log,                 LOG,                    "+I")
	ENTRYX(logD,                LOGD,                   "+I,+I2")
	ENTRYX(clock,               CLOCK,                  "FREQ")
	ENTRYX(extclock,            EXTCLOCK,               "FREQ,PATTERN")
	ENTRYX(mainclock,           MAINCLOCK,              "FREQ")
	ENTRYX(gnd,                 GND,                    "")
	ENTRYX(netlistparams,       PARAMETER,              "")
	ENTRYX(solver,              SOLVER,                 "FREQ")
	ENTRYX(res_sw,              RES_SWITCH,             "+IN,+P1,+P2")
	ENTRYX(switch1,             SWITCH,                 "")
	ENTRYX(switch2,             SWITCH2,                "")
	ENTRYX(nicRSFF,             NETDEV_RSFF,            "")
	ENTRYX(nicDelay,            NETDEV_DELAY,           "")
	ENTRYX(2716,                EPROM_2716,             "+GQ,+EPQ,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+A9,+A10")
	ENTRYX(2102A,               RAM_2102A,              "+CEQ,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+A9,+RWQ,+DI")
	ENTRYX(7450,                TTL_7450_ANDORINVERT,   "+A,+B,+C,+D")
	ENTRYX(7448,                TTL_7448,               "+A,+B,+C,+D,+LTQ,+BIQ,+RBIQ")
	ENTRYX(7473,                TTL_7473,               "+CLK,+J,+K,+CLRQ")
	ENTRYX(7473A,               TTL_7473A,              "+CLK,+J,+K,+CLRQ")
	ENTRYX(7474,                TTL_7474,               "+CLK,+D,+CLRQ,+PREQ")
	ENTRYX(7475,                TTL_7475,               "")
	ENTRYX(7477,                TTL_7477,               "")
	ENTRYX(7483,                TTL_7483,               "+A1,+A2,+A3,+A4,+B1,+B2,+B3,+B4,+C0")
	ENTRYX(7485,                TTL_7485,               "+A0,+A1,+A2,+A3,+B0,+B1,+B2,+B3,+LTIN,+EQIN,+GTIN")
	ENTRYX(7490,                TTL_7490,               "+A,+B,+R1,+R2,+R91,+R92")
	ENTRYX(7493,                TTL_7493,               "+CLKA,+CLKB,+R1,+R2")
	ENTRYX(74107,               TTL_74107,              "+CLK,+J,+K,+CLRQ")
	ENTRYX(74107A,              TTL_74107A,             "+CLK,+J,+K,+CLRQ")
	ENTRYX(74123,               TTL_74123,              "")
	ENTRYX(74153,               TTL_74153,              "+C0,+C1,+C2,+C3,+A,+B,+G")
	ENTRYX(74161,               TTL_74161,              "+A,+B,+C,+D,+CLRQ,+LOADQ,+CLK,+ENABLEP,+ENABLET")
	ENTRYX(74164,               TTL_74164,              "+A,+B,+CLRQ,+CLK")
	ENTRYX(74165,               TTL_74165,              "+CLK,+CLKINH,+SH_LDQ,+SER,+A,+B,+C,+D,+E,+F,+G,+H")
	ENTRYX(74166,               TTL_74166,              "+CLK,+CLKINH,+SH_LDQ,+SER,+A,+B,+C,+D,+E,+F,+G,+H,+CLRQ")
	ENTRYX(74174,               TTL_74174,              "+CLK,+D1,+D2,+D3,+D4,+D5,+D6,+CLRQ")
	ENTRYX(74175,               TTL_74175,              "+CLK,+D1,+D2,+D3,+D4,+CLRQ")
	ENTRYX(74192,               TTL_74192,              "+A,+B,+C,+D,+CLEAR,+LOADQ,+CU,+CD")
	ENTRYX(74193,               TTL_74193,              "+A,+B,+C,+D,+CLEAR,+LOADQ,+CU,+CD")
	ENTRYX(74194,               TTL_74194,              "+CLK,+S0,+S1,+SRIN,+A,+B,+C,+D,+SLIN,+CLRQ")
	ENTRYX(74365,               TTL_74365,              "+G1Q,+G2Q,+A1,+A2,+A3,+A4,+A5,+A6")
	//ENTRY(74279,              TTL_74279,              "") // only dip available
	ENTRYX(SN74LS629,           SN74LS629,              "CAP")
	ENTRYX(82S16,               TTL_82S16,              "")
	ENTRYX(82S115,              PROM_82S115,            "+CE1Q,+CE2,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+STROBE")
	ENTRYX(82S123,              PROM_82S123,            "+CEQ,+A0,+A1,+A2,+A3,+A4")
	ENTRYX(82S126,              PROM_82S126,            "+CE1Q,+CE2Q,+A0,+A1,+A2,+A3,+A4,+A5,+A6,+A7")
	ENTRYX(9310,                TTL_9310,               "")
	ENTRYX(9316,                TTL_9316,               "+CLK,+ENP,+ENT,+CLRQ,+LOADQ,+A,+B,+C,+D")
	ENTRYX(9322,                TTL_9322,               "+SELECT,+A1,+B1,+A2,+B2,+A3,+B3,+A4,+B4,+STROBE")
	ENTRYX(9334,                TTL_9334,               "+CQ,+EQ,+D,+A0,+A1,+A2")
	ENTRYX(AM2847,              TTL_AM2847,             "+CP,+INA,+INB,+INC,+IND,+RCA,+RCB,+RCC,+RCD")
	ENTRYX(CD4020,              CD4020,                 "")
	ENTRYX(CD4066_GATE,         CD4066_GATE,            "")
	ENTRYX(CD4316_GATE,         CD4316_GATE,            "")
	ENTRYX(schmitt_trigger,     SCHMITT_TRIGGER,        "MODEL")
	/* entries with suffix WI are legacy only */
	ENTRYX(CD4020,              CD4020_WI,              "+IP,+RESET,+VDD,+VSS")
	//ENTRY(4066,                 CD_4066,              "+A,B")
	ENTRYX(NE555,               NE555,                  "")
	ENTRYX(r2r_dac,             R2R_DAC,                "VIN,R,N")
	ENTRYX(tristate,            TTL_TRISTATE,           "+CEQ1,+D1,+CEQ2,+D2")
	ENTRYX(tristate3,           TTL_TRISTATE3,          "")
	ENTRYX(2102A_dip,           RAM_2102A_DIP,          "")
	ENTRYX(2716_dip,            EPROM_2716_DIP,         "")
	ENTRYX(4538_dip,            CD4538_DIP,             "")
	ENTRYX(7448_dip,            TTL_7448_DIP,           "")
	ENTRYX(7450_dip,            TTL_7450_DIP,           "")
	ENTRYX(7473_dip,            TTL_7473_DIP,           "")
	ENTRYX(7473A_dip,           TTL_7473A_DIP,          "")
	ENTRYX(7474_dip,            TTL_7474_DIP,           "")
	ENTRYX(7475_dip,            TTL_7475_DIP,           "")
	ENTRYX(7477_dip,            TTL_7477_DIP,           "")
	ENTRYX(7483_dip,            TTL_7483_DIP,           "")
	ENTRYX(7485_dip,            TTL_7485_DIP,           "")
	ENTRYX(7490_dip,            TTL_7490_DIP,           "")
	ENTRYX(7493_dip,            TTL_7493_DIP,           "")
	ENTRYX(74107_dip,           TTL_74107_DIP,          "")
	ENTRYX(74123_dip,           TTL_74123_DIP,          "")
	ENTRYX(74153_dip,           TTL_74153_DIP,          "")
	ENTRYX(74161_dip,           TTL_74161_DIP,          "")
	ENTRYX(74164_dip,           TTL_74164_DIP,          "")
	ENTRYX(74165_dip,           TTL_74165_DIP,          "")
	ENTRYX(74166_dip,           TTL_74166_DIP,          "")
	ENTRYX(74174_dip,           TTL_74174_DIP,          "")
	ENTRYX(74175_dip,           TTL_74175_DIP,          "")
	ENTRYX(74192_dip,           TTL_74192_DIP,          "")
	ENTRYX(74193_dip,           TTL_74193_DIP,          "")
	ENTRYX(74194_dip,           TTL_74194_DIP,          "")
	ENTRYX(74365_dip,           TTL_74365_DIP,          "")
	ENTRYX(82S16_dip,           TTL_82S16_DIP,          "")
	ENTRYX(82S115_dip,          PROM_82S115_DIP,        "")
	ENTRYX(82S123_dip,          PROM_82S123_DIP,        "")
	ENTRYX(82S126_dip,          PROM_82S126_DIP,        "")
	ENTRYX(9602_dip,            TTL_9602_DIP,           "")
	ENTRYX(9310_dip,            TTL_9310_DIP,           "")
	ENTRYX(9316_dip,            TTL_9316_DIP,           "")
	ENTRYX(9322_dip,            TTL_9322_DIP,           "")
	ENTRYX(9334_dip,            TTL_9334_DIP,           "")
	ENTRYX(AM2847_dip,          TTL_AM2847_DIP,         "")
	ENTRYX(SN74LS629_dip,       SN74LS629_DIP,          "1.CAP1,2.CAP2")
	ENTRYX(NE555_dip,           NE555_DIP,              "")
	ENTRYX(MM5837_dip,          MM5837_DIP,             "")
}

	} //namespace devices
} // namespace netlist

