// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi

class pgm_012_025_state : public pgm_state
{
public:
	pgm_012_025_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag),
			m_igs025(*this,"igs025")
	{
	}


	required_device<igs025_device> m_igs025;

	void pgm_drgw2_decrypt();
	void drgw2_common_init();

	void init_drgw2();
	void init_dw2v100x();
	void init_drgw2c();
	void init_drgw2j();
	void init_drgw2hk();

	DECLARE_MACHINE_RESET(drgw2);
	void pgm_012_025_drgw2(machine_config &config);
	void drgw2_mem(address_map &map);
};
