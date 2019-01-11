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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

// NB: This is effectively an aggregate of the X and Y inputs.
#define MCFG_SAA1043_TYPE(_type) \
	&downcast<saa1043_device &>(*device).set_type(_type);

#define MCFG_SAA1043_BC_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_bc_callback(DEVCB_##_write);

#define MCFG_SAA1043_FH2_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_fh2_callback(DEVCB_##_write);

#define MCFG_SAA1043_FH3_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_fh3_callback(DEVCB_##_write);

#define MCFG_SAA1043_FH80_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_fh80_callback(DEVCB_##_write);

#define MCFG_SAA1043_PH_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_ph_callback(DEVCB_##_write);

#define MCFG_SAA1043_NS_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_ns_callback(DEVCB_##_write);

#define MCFG_SAA1043_RI_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_ri_callback(DEVCB_##_write);

#define MCFG_SAA1043_WMP_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_wmp_callback(DEVCB_##_write);

#define MCFG_SAA1043_RR_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_rr_callback(DEVCB_##_write);

#define MCFG_SAA1043_V1_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_v1_callback(DEVCB_##_write);

#define MCFG_SAA1043_V2_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_v2_callback(DEVCB_##_write);

#define MCFG_SAA1043_CLP_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_clp_callback(DEVCB_##_write);

#define MCFG_SAA1043_DL_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_dl_callback(DEVCB_##_write);

#define MCFG_SAA1043_H1_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_h1_callback(DEVCB_##_write);

#define MCFG_SAA1043_H2_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_h2_callback(DEVCB_##_write);

#define MCFG_SAA1043_CB_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_cb_callback(DEVCB_##_write);

#define MCFG_SAA1043_CS_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_cs_callback(DEVCB_##_write);

#define MCFG_SAA1043_ID_CALLBACK(_write) \
	devcb = &downcast<saa1043_device &>(*device).set_id_callback(DEVCB_##_write);

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

	template <class Object> devcb_base &set_bc_callback(Object &&cb)   { m_outputs_hooked[BC] = true;   return m_outputs[BC].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_fh2_callback(Object &&cb)  { m_outputs_hooked[FH2] = true;  return m_outputs[FH2].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_fh3_callback(Object &&cb)  { m_outputs_hooked[FH3] = true;  return m_outputs[FH3].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_fh80_callback(Object &&cb) { m_outputs_hooked[FH80] = true; return m_outputs[FH80].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_ph_callback(Object &&cb)   { m_outputs_hooked[PH] = true;   return m_outputs[PH].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_ns_callback(Object &&cb)   { m_outputs_hooked[NS] = true;   return m_outputs[NS].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_ri_callback(Object &&cb)   { m_outputs_hooked[RI] = true;   return m_outputs[RI].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_wmp_callback(Object &&cb)  { m_outputs_hooked[WMP] = true;  return m_outputs[WMP].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_rr_callback(Object &&cb)   { m_outputs_hooked[RR] = true;   return m_outputs[RR].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_v1_callback(Object &&cb)   { m_outputs_hooked[V1] = true;   return m_outputs[V1].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_v2_callback(Object &&cb)   { m_outputs_hooked[V2] = true;   return m_outputs[V2].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_clp_callback(Object &&cb)  { m_outputs_hooked[CLP] = true;  return m_outputs[CLP].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dl_callback(Object &&cb)   { m_outputs_hooked[DL] = true;   return m_outputs[DL].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_h1_callback(Object &&cb)   { m_outputs_hooked[H1] = true;   return m_outputs[H1].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_h2_callback(Object &&cb)   { m_outputs_hooked[H2] = true;   return m_outputs[H2].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_cb_callback(Object &&cb)   { m_outputs_hooked[CB] = true;   return m_outputs[CB].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_cs_callback(Object &&cb)   { m_outputs_hooked[CS] = true;   return m_outputs[CS].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_id_callback(Object &&cb)   { m_outputs_hooked[ID] = true;   return m_outputs[ID].set_callback(std::forward<Object>(cb)); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	devcb_write_line m_outputs[OUT_COUNT];
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
