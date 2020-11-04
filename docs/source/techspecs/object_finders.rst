Object Finders
==============

.. contents:: :local:


Introduction
------------

Object finders are an important part of the glue MAME provides to tie the
devices that make up an emulated system together.  Object finders are used to
specify connections between devices, to efficiently access resources, and to
check that necessary resources are available on validation.

Object finders search for a target object by tag relative to a base device.
Some types of object finder require additional parameters.

Most object finders have required and optional versions.  The required versions
will raise an error if the target object is not found.  This will prevent a
device from starting or cause a validation error.  The optional versions will
log a verbose message if the target object is not found, and provide additional
members for testing whether the target object was found or not.

Object finder classes are declared in the header src/emu/devfind.h and have
Doxygen format API documentation.


Types of object finder
----------------------

required_device<DeviceClass>, optional_device<DeviceClass>
    Finds a device.  The template argument ``DeviceClass`` should be a class
    derived from ``device_t`` or ``device_interface``.
required_memory_region, optional_memory_region
    Finds a memory region, usually from ROM definitions.  The target is the
    ``memory_region`` object.
required_memory_bank, optional_memory_bank
    Finds a memory bank instantiated in an address map.  The target is the
    ``memory_bank`` object.
memory_bank_creator
    Finds a memory bank instantiated in an address map, or creates it if it
    doesn’t exist.  The target is the ``memory_bank`` object.  There is no
    optional version, because the target object will always be found or
    created.
required_ioport, optional_ioport
    Finds and I/O port from a device’s input port definitions.  The target is
    the ``ioport_port`` object.
required_address_space, optional_address_space
    Finds a device’s address space.  The target is the ``address_space`` object.
required_region_ptr<PointerType>, optional_region_ptr<PointerType>
    Finds the base pointer of a memory region, usually from ROM definitions.
    The template argument ``PointerType`` is the target type (usually an
    unsigned integer type).  The target is the first element in the memory
    region.
required_shared_ptr<PointerType>, optional_shared_ptr<PointerType>
    Finds the base pointer of a memory share instantiated in an address map.
    The template argument ``PointerType`` is the target type (usually an
    unsigned integer type).  The target is the first element in the memory
    share.
memory_share_creator<PointerType>
    Finds the base pointer of a memory share instantiated in an address map, or
    creates it if it doesn’t exist.  The template argument ``PointerType`` is
    the target type (usually an unsigned integer type).  The target is the first
    element in the memory share.  There is no optional version, because the
    target object will always be found or created.


Finding resources
-----------------

We’ll start with a simple example of a device that uses object finders to access
its own child devices, inputs and ROM region.  The code samples here are based
on the Apple II Parallel Printer Interface card, but a lot of things have been
removed for clarity.

Object finders are declared as members of the device class::

    class a2bus_parprn_device : public device_t, public device_a2bus_card_interface
    {
    public:
        a2bus_parprn_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

        virtual void write_c0nx(u8 offset, u8 data) override;
        virtual u8 read_cnxx(u8 offset) override;

    protected:
        virtual tiny_rom_entry const *device_rom_region() const override;
        virtual void device_add_mconfig(machine_config &config) override;
        virtual ioport_constructor device_input_ports() const override;

    private:
        required_device<centronics_device>      m_printer_conn;
        required_device<output_latch_device>    m_printer_out;
        required_ioport                         m_input_config;
        required_region_ptr<u8>                 m_prom;
    };

We want to find a ``centronics_device``, an ``output_latch_device``, an I/O
port, and an 8-bit memory region.

In the constructor, we set the initial target for the object finders::

    a2bus_parprn_device::a2bus_parprn_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
        device_t(mconfig, A2BUS_PARPRN, tag, owner, clock),
        device_a2bus_card_interface(mconfig, *this),
        m_printer_conn(*this, "prn"),
        m_printer_out(*this, "prn_out"),
        m_input_config(*this, "CFG"),
        m_prom(*this, "prom")
    {
    }

Each object finder takes a base device and tag as constructor arguments.  The
base device supplied at construction serves two purposes.  Most obviously, the
tag is specified relative to this device.  Possibly more importantly, the object
finder registers itself with this device so that it will be called to perform
validation and object resolution.

Note that the object finders *do not* copy the tag strings.  The caller must
ensure the tag string remains valid until after validation and/or object
resolution is complete.

The memory region and I/O port come from the ROM definition and input
definition, respectively::

    namespace {

    ROM_START(parprn)
        ROM_REGION(0x100, "prom", 0)
        ROM_LOAD( "prom.b4", 0x0000, 0x0100, BAD_DUMP CRC(00b742ca) SHA1(c67888354aa013f9cb882eeeed924e292734e717) )
    ROM_END

    INPUT_PORTS_START(parprn)
        PORT_START("CFG")
        PORT_CONFNAME(0x01, 0x00, "Acknowledge latching edge")
        PORT_CONFSETTING(   0x00, "Falling (/Y-B)")
        PORT_CONFSETTING(   0x01, "Rising (Y-B)")
        PORT_CONFNAME(0x06, 0x02, "Printer ready")
        PORT_CONFSETTING(   0x00, "Always (S5-C-D)")
        PORT_CONFSETTING(   0x02, "Acknowledge latch (Z-C-D)")
        PORT_CONFSETTING(   0x04, "ACK (Y-C-D)")
        PORT_CONFSETTING(   0x06, "/ACK (/Y-C-D)")
        PORT_CONFNAME(0x08, 0x00, "Strobe polarity")
        PORT_CONFSETTING(   0x00, "Negative (S5-A-/X, GND-X)")
        PORT_CONFSETTING(   0x08, "Positive (S5-X, GND-A-/X)")
        PORT_CONFNAME(0x10, 0x10, "Character width")
        PORT_CONFSETTING(   0x00, "7-bit")
        PORT_CONFSETTING(   0x10, "8-bit")
    INPUT_PORTS_END

    } // anonymous namespace

    tiny_rom_entry const *a2bus_parprn_device::device_rom_region() const
    {
        return ROM_NAME(parprn);
    }

    ioport_constructor a2bus_parprn_device::device_input_ports() const
    {
        return INPUT_PORTS_NAME(parprn);
    }

Note that the tags ``"prom"`` and ``"CFG"`` match the tags passed to the object
finders on construction.

Child devices are instantiated in the device’s machine configuration member
function::

    void a2bus_parprn_device::device_add_mconfig(machine_config &config)
    {
        CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
        m_printer_conn->ack_handler().set(FUNC(a2bus_parprn_device::ack_w));

        OUTPUT_LATCH(config, m_printer_out);
        m_printer_conn->set_output_latch(*m_printer_out);
    }

Object finders are passed to device types to provide tags when instantiating
child devices.  After instantiating a child device in this way, the object
finder can be used like a pointer to the device until the end of the machine
configuration member function.  Note that to use an object finder like this,
its base device must be the same as the device being configured (the ``this``
pointer of the machine configuration member function).

After the emulated machine has been started, the object finders can be used in
much the same way as pointers::

    void a2bus_parprn_device::write_c0nx(u8 offset, u8 data)
    {
        ioport_value const cfg(m_input_config->read());

        m_printer_out->write(data & (BIT(cfg, 8) ? 0xffU : 0x7fU));
        m_printer_conn->write_strobe(BIT(~cfg, 3));
    }


    u8 a2bus_parprn_device::read_cnxx(u8 offset)
    {
        offset ^= 0x40U;
        return m_prom[offset];
    }

For convenience, object finders that target the base addresses of memory regions
and shares can be indexed like arrays.
