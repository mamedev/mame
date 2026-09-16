// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***********************************************************************

    Philips SAA1043 Universal Sync Generator emulation

    TODO:
    - Everything.

************************************************************************
                            _____   _____
                    BC   1 |*    \_/     | 28  Vdd
                   FH2   2 |             | 27  ID
                    SI   3 |             | 26  CS
                   FH3   4 |             | 25  CB
                     X   5 |             | 24  H2
                     Y   6 |             | 23  H1
                    FD   7 |   SAA1043   | 22  DL
                  FH80   8 |             | 21  CLP
                   VCR   9 |             | 20  V2
                  OSCO  10 |             | 19  V1
                  OSCI  11 |             | 18  RR
                    PH  12 |             | 17  WMP
                    NS  13 |             | 16  RI
                   Vss  14 |_____________| 15  ECS

************************************************************************/

#ifndef MAME_MACHINE_SAA1043_H
#define MAME_MACHINE_SAA1043_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saa1043_device

class saa1043_device : public device_t
{
public:
	// construction/destruction
	saa1043_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	enum outputs : uint32_t
	{
		OUT_BC,
		OUT_FH2,
		OUT_FH3,
		OUT_FH80,
		OUT_PH,
		OUT_NS,
		OUT_RI,
		OUT_WMP,
		OUT_RR,
		OUT_V1,
		OUT_V2,
		OUT_CLP,
		OUT_DL,
		OUT_H1,
		OUT_H2,
		OUT_CB,
		OUT_CS,
		OUT_ID,

		OUT_COUNT
	};

	enum signal_type : uint32_t
	{
		PAL,
		SECAM,
		NTSC,
		PAL_M,
	};

	void set_type(signal_type type) { m_type = type; }

	auto bc_callback()   { return m_outputs[OUT_BC].bind(); }
	auto fh2_callback()  { return m_outputs[OUT_FH2].bind(); }
	auto fh3_callback()  { return m_outputs[OUT_FH3].bind(); }
	auto fh80_callback() { return m_outputs[OUT_FH80].bind(); }
	auto ph_callback()   { return m_outputs[OUT_PH].bind(); }
	auto ns_callback()   { return m_outputs[OUT_NS].bind(); }
	auto ri_callback()   { return m_outputs[OUT_RI].bind(); }
	auto wmp_callback()  { return m_outputs[OUT_WMP].bind(); }
	auto rr_callback()   { return m_outputs[OUT_RR].bind(); }
	auto v1_callback()   { return m_outputs[OUT_V1].bind(); }
	auto v2_callback()   { return m_outputs[OUT_V2].bind(); }
	auto clp_callback()  { return m_outputs[OUT_CLP].bind(); }
	auto dl_callback()   { return m_outputs[OUT_DL].bind(); }
	auto h1_callback()   { return m_outputs[OUT_H1].bind(); }
	auto h2_callback()   { return m_outputs[OUT_H2].bind(); }
	auto cb_callback()   { return m_outputs[OUT_CB].bind(); }
	auto cs_callback()   { return m_outputs[OUT_CS].bind(); }
	auto id_callback()   { return m_outputs[OUT_ID].bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(toggle_v2);

private:
	devcb_write_line::array<OUT_COUNT> m_outputs;
	emu_timer *m_timers[OUT_COUNT];
	signal_type m_type;

	attotime m_h;
	uint32_t m_line_count;

	static const uint32_t s_line_counts[4];
};

// device type definition
DECLARE_DEVICE_TYPE(SAA1043, saa1043_device)

#endif // MAME_MACHINE_SAA1043_H
