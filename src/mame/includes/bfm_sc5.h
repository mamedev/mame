#include "cpu/m68000/m68000.h"
#include "includes/bfm_sc45.h"

class bfm_sc5_state : public driver_device
{
public:
	bfm_sc5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_vfd0(*this, "vfd0")
	{ }

protected:


public:


	// devices
	required_device<cpu_device> m_maincpu;
	optional_device<bfm_bda_t> m_vfd0;

	// serial vfd
	int vfd_enabled;
	bool vfd_old_clock;

	UINT8 vfd_ser_value;
	int vfd_ser_count;

	DECLARE_DRIVER_INIT(sc5);
	DECLARE_WRITE_LINE_MEMBER(bfm_sc5_ym_irqhandler);
	DECLARE_READ8_MEMBER( sc5_10202F0_r );
	DECLARE_WRITE8_MEMBER( sc5_10202F0_w );

};
