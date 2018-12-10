// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi

class pgm_022_025_state : public pgm_state
{
public:
	pgm_022_025_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_state(mconfig, type, tag)
		, m_sharedprotram(*this, "sharedprotram")
		, m_igs025(*this, "igs025")
		, m_igs022(*this, "igs022")
	{ }

	void pgm_022_025(machine_config &config);

protected:
	required_shared_ptr<u16> m_sharedprotram;

	void igs025_to_igs022_callback( void );

	required_device<igs025_device> m_igs025;
	required_device<igs022_device> m_igs022;
	void mem_map(address_map &map);
};

class pgm_022_025_killbld_state : public pgm_022_025_state
{
public:
	pgm_022_025_killbld_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_022_025_state(mconfig, type, tag)
	{ }

	void driver_init() override;

	void pgm_022_025_killbld(machine_config &config);

protected:
	virtual void machine_reset() override;

private:
	void decrypt();
	void killbld_map(address_map &map);
};

class pgm_022_025_dw3_state : public pgm_022_025_state
{
public:
	pgm_022_025_dw3_state(const machine_config &mconfig, device_type type, const char *tag)
		: pgm_022_025_state(mconfig, type, tag)
	{ }

	void driver_init() override;

	void pgm_022_025_dw3(machine_config &config);

protected:
	virtual void machine_reset() override;

private:
	void decrypt();
	void dw3_map(address_map &map);
};

INPUT_PORTS_EXTERN( killbld );
INPUT_PORTS_EXTERN( dw3 );
INPUT_PORTS_EXTERN( dw3j );
