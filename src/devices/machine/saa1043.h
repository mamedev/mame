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
		BC,
		FH2,
		FH3,
		FH80,
		PH,
		NS,
		RI,
		WMP,
		RR,
		V1,
		V2,
		CLP,
		DL,
		H1,
		H2,
		CB,
		CS,
		ID,

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

	auto bc_callback()   { m_outputs_hooked[BC] = true;   return m_outputs[BC].bind(); }
	auto fh2_callback()  { m_outputs_hooked[FH2] = true;  return m_outputs[FH2].bind(); }
	auto fh3_callback()  { m_outputs_hooked[FH3] = true;  return m_outputs[FH3].bind(); }
	auto fh80_callback() { m_outputs_hooked[FH80] = true; return m_outputs[FH80].bind(); }
	auto ph_callback()   { m_outputs_hooked[PH] = true;   return m_outputs[PH].bind(); }
	auto ns_callback()   { m_outputs_hooked[NS] = true;   return m_outputs[NS].bind(); }
	auto ri_callback()   { m_outputs_hooked[RI] = true;   return m_outputs[RI].bind(); }
	auto wmp_callback()  { m_outputs_hooked[WMP] = true;  return m_outputs[WMP].bind(); }
	auto rr_callback()   { m_outputs_hooked[RR] = true;   return m_outputs[RR].bind(); }
	auto v1_callback()   { m_outputs_hooked[V1] = true;   return m_outputs[V1].bind(); }
	auto v2_callback()   { m_outputs_hooked[V2] = true;   return m_outputs[V2].bind(); }
	auto clp_callback()  { m_outputs_hooked[CLP] = true;  return m_outputs[CLP].bind(); }
	auto dl_callback()   { m_outputs_hooked[DL] = true;   return m_outputs[DL].bind(); }
	auto h1_callback()   { m_outputs_hooked[H1] = true;   return m_outputs[H1].bind(); }
	auto h2_callback()   { m_outputs_hooked[H2] = true;   return m_outputs[H2].bind(); }
	auto cb_callback()   { m_outputs_hooked[CB] = true;   return m_outputs[CB].bind(); }
	auto cs_callback()   { m_outputs_hooked[CS] = true;   return m_outputs[CS].bind(); }
	auto id_callback()   { m_outputs_hooked[ID] = true;   return m_outputs[ID].bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	devcb_write_line::array<OUT_COUNT> m_outputs;
	bool m_outputs_hooked[OUT_COUNT];
	emu_timer *m_timers[OUT_COUNT];
	signal_type m_type;

	attotime m_h;
	uint32_t m_line_count;

	static const uint32_t s_line_counts[4];
};

// device type definition
DECLARE_DEVICE_TYPE(SAA1043, saa1043_device)

#endif // MAME_MACHINE_SAA1043_H
