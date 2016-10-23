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
	uint8_t         m_asic3_reg;
	uint8_t         m_asic3_latch[3];
	uint8_t         m_asic3_x;
	uint16_t        m_asic3_hilo;
	uint16_t        m_asic3_hold;

	void init_orlegend();
	void asic3_compute_hold(int,int);
	uint16_t pgm_asic3_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pgm_asic3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
};

INPUT_PORTS_EXTERN( orlegend );
INPUT_PORTS_EXTERN( orlegendt );
INPUT_PORTS_EXTERN( orlegendk );
MACHINE_CONFIG_EXTERN( pgm_asic3 );
