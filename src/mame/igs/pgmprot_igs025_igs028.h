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

	void init_olds();

	void pgm_028_025_ol(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_shared_ptr<u16> m_sharedprotram;
	required_device<igs025_device> m_igs025;
	required_device<igs028_device> m_igs028;

	void igs025_to_igs028_callback( void );

	void olds_mem(address_map &map) ATTR_COLD;
};

INPUT_PORTS_EXTERN( olds );
