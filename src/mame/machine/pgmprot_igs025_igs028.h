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
	
	required_shared_ptr<UINT16> m_sharedprotram;
	required_device<igs025_device> m_igs025;
	required_device<igs028_device> m_igs028;

	void igs025_to_igs028_callback( void );

	DECLARE_DRIVER_INIT(olds);
	DECLARE_MACHINE_RESET(olds);
};

MACHINE_CONFIG_EXTERN( pgm_028_025_ol );
INPUT_PORTS_EXTERN( olds );
