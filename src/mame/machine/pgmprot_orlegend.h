// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, iq_132
/* for machine/pgmprot_orlegend.c type games */

class pgm_asic3_state : public pgm_state
{
public:
	pgm_asic3_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag) {
	}

	// ASIC 3 (oriental legends protection)
	UINT8         m_asic3_reg;
	UINT8         m_asic3_latch[3];
	UINT8         m_asic3_x;
	UINT16        m_asic3_hilo;
	UINT16        m_asic3_hold;

	DECLARE_DRIVER_INIT(orlegend);
	void asic3_compute_hold(int,int);
	DECLARE_READ16_MEMBER( pgm_asic3_r );
	DECLARE_WRITE16_MEMBER( pgm_asic3_w );
};

INPUT_PORTS_EXTERN( orlegend );
INPUT_PORTS_EXTERN( orlegendt );
INPUT_PORTS_EXTERN( orlegendk );
MACHINE_CONFIG_EXTERN( pgm_asic3 );
