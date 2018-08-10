// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// handler_entry_read_delegate/handler_entry_write_delegate

// Executes an access through called a delegate, usually containing a handler or a lambda

template<int Width, int AddrShift, int Endian> class handler_entry_read_delegate : public handler_entry_read_address<Width, AddrShift, Endian>
{
public:
	using uX = typename handler_entry_size<Width>::uX;
	using READ = typename handler_entry_size<Width>::READ;
	using inh = handler_entry_read_address<Width, AddrShift, Endian>;

	handler_entry_read_delegate(address_space *space, READ delegate) : handler_entry_read_address<Width, AddrShift, Endian>(space, 0), m_delegate(delegate) {}
	~handler_entry_read_delegate() = default;

	uX read(offs_t offset, uX mem_mask) override;

	std::string name() const override;

private:
	READ m_delegate;
};

template<int Width, int AddrShift, int Endian> class handler_entry_write_delegate : public handler_entry_write_address<Width, AddrShift, Endian>
{
public:
	using uX = typename handler_entry_size<Width>::uX;
	using WRITE = typename handler_entry_size<Width>::WRITE;
	using inh = handler_entry_write_address<Width, AddrShift, Endian>;

	handler_entry_write_delegate(address_space *space, WRITE delegate) : handler_entry_write_address<Width, AddrShift, Endian>(space, 0), m_delegate(delegate) {}
	~handler_entry_write_delegate() = default;

	void write(offs_t offset, uX data, uX mem_mask) override;

	std::string name() const override;

private:
	WRITE m_delegate;
};


// handler_entry_read_ioport/handler_entry_write_ioport

// Accesses an ioport

template<int Width, int AddrShift, int Endian> class handler_entry_read_ioport : public handler_entry_read<Width, AddrShift, Endian>
{
public:
	using uX = typename handler_entry_size<Width>::uX;
	using inh = handler_entry_read<Width, AddrShift, Endian>;

	handler_entry_read_ioport(address_space *space, ioport_port *port) : handler_entry_read<Width, AddrShift, Endian>(space, 0), m_port(port) {}
	~handler_entry_read_ioport() = default;

	uX read(offs_t offset, uX mem_mask) override;

	std::string name() const override;

private:
	ioport_port *m_port;
};

template<int Width, int AddrShift, int Endian> class handler_entry_write_ioport : public handler_entry_write<Width, AddrShift, Endian>
{
public:
	using uX = typename handler_entry_size<Width>::uX;
	using inh = handler_entry_write<Width, AddrShift, Endian>;

	handler_entry_write_ioport(address_space *space, ioport_port *port) : handler_entry_write<Width, AddrShift, Endian>(space, 0), m_port(port) {}
	~handler_entry_write_ioport() = default;

	void write(offs_t offset, uX data, uX mem_mask) override;

	std::string name() const override;

private:
	ioport_port *m_port;
};
