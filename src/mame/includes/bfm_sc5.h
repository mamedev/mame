#include "cpu/m68000/m68000.h"
#include "includes/bfm_sc45.h"

class bfm_sc5_state : public bfm_sc45_state
{
public:
	bfm_sc5_state(const machine_config &mconfig, device_type type, const char *tag)
		: bfm_sc45_state(mconfig, type, tag)
	{ }

protected:


public:
	DECLARE_DRIVER_INIT(sc5);
	DECLARE_WRITE_LINE_MEMBER(bfm_sc5_ym_irqhandler);
	DECLARE_READ8_MEMBER( sc5_10202F0_r );
	DECLARE_WRITE8_MEMBER( sc5_10202F0_w );
	DECLARE_WRITE16_MEMBER( sc5_duart_w );

	DECLARE_READ8_MEMBER( sc5_mux1_r );
	DECLARE_WRITE8_MEMBER( sc5_mux1_w );
	DECLARE_WRITE8_MEMBER( sc5_mux2_w );


};

