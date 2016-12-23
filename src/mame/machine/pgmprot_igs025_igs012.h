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

	DECLARE_DRIVER_INIT(drgw2);
	DECLARE_DRIVER_INIT(dw2v100x);
	DECLARE_DRIVER_INIT(drgw2c);
	DECLARE_DRIVER_INIT(drgw2j);
	DECLARE_DRIVER_INIT(drgw2hk);

	DECLARE_MACHINE_RESET(drgw2);
};

MACHINE_CONFIG_EXTERN( pgm_012_025_drgw2 );
