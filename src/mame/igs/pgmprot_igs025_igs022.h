// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi

class pgm_022_025_state : public pgm_state
{
public:
	pgm_022_025_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag),
			m_igs025(*this,"igs025"),
			m_igs022(*this,"igs022")

	{ }

	void pgm_dw3_decrypt();
	void pgm_killbld_decrypt();

	void init_killbld();
	void init_drgw3();
	DECLARE_MACHINE_RESET(killbld);
	DECLARE_MACHINE_RESET(dw3);

	void igs025_to_igs022_callback( void );

	required_device<igs025_device> m_igs025;
	required_device<igs022_device> m_igs022;
	void pgm_022_025(machine_config &config);
	void pgm_022_025_dw3(machine_config &config);
	void pgm_022_025_killbld(machine_config &config);
	void killbld_mem(address_map &map) ATTR_COLD;
};

INPUT_PORTS_EXTERN( killbld );
INPUT_PORTS_EXTERN( dw3 );
INPUT_PORTS_EXTERN( dw3j );
