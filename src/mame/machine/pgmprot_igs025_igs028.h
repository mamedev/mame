// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi

class pgm_028_025_state : public pgm_state
{
public:
	pgm_028_025_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag),
			m_sharedprotram(*this, "sharedprotram"),
			m_igs025(*this,"igs025"),
			m_igs028(*this,"igs028")

	{
	}

	required_shared_ptr<uint16_t> m_sharedprotram;
	required_device<igs025_device> m_igs025;
	required_device<igs028_device> m_igs028;

	void igs025_to_igs028_callback( void );

	void init_olds();
	DECLARE_MACHINE_RESET(olds);
	void pgm_028_025_ol(machine_config &config);
	void olds_mem(address_map &map);
};

INPUT_PORTS_EXTERN( olds );
