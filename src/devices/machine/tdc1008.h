// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    tdc1008.h
    TRW TDC1008 VLSI Multiplier - Accumulator

***************************************************************************/

#ifndef MAME_MACHINE_TDC1008_TDC1008_H
#define MAME_MACHINE_TDC1008_TDC1008_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tdc1008_device

class tdc1008_device : public device_t
{
public:
	// construction/destruction
	tdc1008_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_WRITE8_MEMBER(x_w);
	DECLARE_WRITE8_MEMBER(y_w);
	DECLARE_WRITE_LINE_MEMBER(tsx_w);
	DECLARE_WRITE_LINE_MEMBER(tsm_w);
	DECLARE_WRITE_LINE_MEMBER(tsl_w);
	DECLARE_WRITE_LINE_MEMBER(clk_x_w);
	DECLARE_WRITE_LINE_MEMBER(clk_y_w);
	DECLARE_WRITE_LINE_MEMBER(clk_p_w);
	DECLARE_WRITE_LINE_MEMBER(prel_w);
	DECLARE_WRITE_LINE_MEMBER(rnd_w);
	DECLARE_WRITE_LINE_MEMBER(tc_w);
	DECLARE_WRITE_LINE_MEMBER(acc_w);
	DECLARE_WRITE_LINE_MEMBER(sub_w);

	// Output preloads by group
	DECLARE_WRITE8_MEMBER(xtp_w);
	DECLARE_WRITE8_MEMBER(msp_w);
	DECLARE_WRITE8_MEMBER(lsp_w);

	// Full output preload
	DECLARE_WRITE32_MEMBER(output_w);

	// Outputs by group
	auto xtp() { return m_xtp.bind(); }
	auto msp() { return m_msp.bind(); }
	auto lsp() { return m_lsp.bind(); }

	// Full output
	auto p() { return m_p.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void latch_flags();

	union input_reg
	{
		int8_t s;
		uint8_t u;
	};

	union output_reg
	{
		int32_t s;
		uint32_t u;
	};

	uint8_t m_x_in;
	uint8_t m_y_in;
	uint8_t m_xtp_in;
	uint8_t m_msp_in;
	uint8_t m_lsp_in;
	uint32_t m_p_in;
	bool m_tsx;
	bool m_tsm;
	bool m_tsl;
	bool m_clk_x;
	bool m_clk_y;
	bool m_clk_p;
	bool m_prel;
	bool m_rnd_in;
	bool m_rnd;
	bool m_tc_in;
	bool m_tc;
	bool m_acc_in;
	bool m_acc;
	bool m_sub_in;
	bool m_sub;

	input_reg m_x;
	input_reg m_y;
	output_reg m_p_out;

	devcb_write8 m_xtp;
	devcb_write8 m_msp;
	devcb_write8 m_lsp;
	devcb_write32 m_p;
};

// device type definition
DECLARE_DEVICE_TYPE(TDC1008, tdc1008_device)

#endif // MAME_MACHINE_TDC1008_TDC1008_H
