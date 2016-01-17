// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "cpu/m68000/m68000.h"
#include "includes/bfm_sc45.h"

class bfm_sc5_state : public bfm_sc45_state
{
public:
	bfm_sc5_state(const machine_config &mconfig, device_type type, std::string tag)
		: bfm_sc45_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

protected:


public:
	required_device<m68000_base_device> m_maincpu;

	DECLARE_DRIVER_INIT(sc5);
	DECLARE_READ8_MEMBER( sc5_10202F0_r );
	DECLARE_WRITE8_MEMBER( sc5_10202F0_w );
	DECLARE_WRITE16_MEMBER( sc5_duart_w );

	DECLARE_READ8_MEMBER( sc5_mux1_r );
	DECLARE_WRITE8_MEMBER( sc5_mux1_w );
	DECLARE_WRITE8_MEMBER( sc5_mux2_w );

	DECLARE_WRITE_LINE_MEMBER(bfm_sc5_duart_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(bfm_sc5_duart_txa);
	DECLARE_READ8_MEMBER(bfm_sc5_duart_input_r);
	DECLARE_WRITE8_MEMBER(bfm_sc5_duart_output_w);
};
