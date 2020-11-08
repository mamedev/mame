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
    Finds an I/O port from a device’s input port definitions.  The target is the
    ``ioport_port`` object.
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

Object finders are declared as members of the device class:

.. code-block:: C++

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

In the constructor, we set the initial target for the object finders:

.. code-block:: C++

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
definition, respectively:

.. code-block:: C++

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
function:

.. code-block:: C++

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
much the same way as pointers:

.. code-block:: C++

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

For convenience, object finders that target the base pointer of memory regions
and shares can be indexed like arrays.


Connections between devices
---------------------------

Devices need to be connected together within a system.  For example the Sun SBus
device needs access to the host CPU and address space.  Here’s how we declare
the object finders in the device class (with all distractions removed):

.. code-block:: C++

    DECLARE_DEVICE_TYPE(SBUS, sbus_device)

    class sbus_device : public device_t, public device_memory_interface
    {
        template <typename T, typename U>
        sbus_device(
                machine_config const &mconfig, char const *tag, device_t *owner, u32 clock,
                T &&cpu_tag,
                U &&space_tag, int space_num) :
            sbus_device(mconfig, tag, owner, clock)
        {
            set_cpu(std::forward<T>(cpu_tag));
            set_type1space(std::forward<U>(space_tag), space_num);
        }

        sbus_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
            device_t(mconfig, type, tag, owner, clock),
            device_memory_interface(mconfig, *this),
            m_maincpu(*this, finder_base::DUMMY_TAG),
            m_type1space(*this, finder_base::DUMMY_TAG, -1)
        {
        }

        template <typename T> void set_cpu(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
        template <typename T> void set_type1space(T &&tag, int num) { m_type1space.set_tag(std::forward<T>(tag), num); }

    protected:
        required_device<sparc_base_device> m_maincpu;
        required_address_space m_type1space;
    };

There are several things to take note of here:

* Object finder members are declared for the things the device needs to access.
* The device doesn’t know how it will fit into a larger system, the object
  finders are constructed with dummy arguments.
* Configuration member functions are provided to set the tag for the host CPU,
  and the tag and index for the type 1 address space.
* In addition to the standard device constructor, a constructor with additional
  parameters for setting the CPU and type 1 address space is provided.

The constant ``finder_base::DUMMY_TAG`` is guaranteed to be invalid and will not
resolve to an object.  This makes it easy to detect incomplete configuration and
report an error.  Address spaces are numbered from zero, so a negative address
space number is invalid.

The member functions for configuring object finders take a universal reference
to a tag-like object (templated type with ``&&`` qualifier), as well as any
other parameters needed by the specific type of object finder.  An address space
finder needs an address space number in addition to a tag-like object.

So what’s a tag-like object?  Three things are supported:

* A C string pointer (``char const *``) representing a tag relative to the
  device being configured.  Note that the object finder will not copy the
  string.  The caller must ensure it remains valid until resolution and/or
  validation is complete.
* Another object finder.  The object finder will take on its current target.
* For device finders, a reference to an instance of the target device type,
  setting the target to that device.  Note that this will not work if the device
  is subsequently replaced in the machine configuration.  It’s most often used
  with ``*this``.

The additional constructor that sets initial configuration delegates to the
standard constructor and then calls the configuration member functions.  It’s
purely for convenience.

When we want to instantiate this device and hook it up, we do this::

    SPARCV7(config, m_maincpu, 20'000'000);

    ADDRESS_MAP_BANK(config, m_type1space);

    SBUS(config, m_sbus, 20'000'000);
    m_sbus->set_cpu(m_maincpu);
    m_sbus->set_type1space(m_type1space, 0);

We supply the same object finders to instantiate the CPU and address space
devices, and to configure the SBus device.

Note that we could also use literal C strings to configure the SBus device, at
the cost of needing to update the tags in multiple places if they change::

    SBUS(config, m_sbus, 20'000'000);
    m_sbus->set_cpu("maincpu");
    m_sbus->set_type1space("type1", 0);

If we want to use the convenience constructor, we just supply additional
arguments when instantiating the device::

    SBUS(config, m_sbus, 20'000'000, m_maincpu, m_type1space, 0);


Object finder arrays
--------------------

Many systems have multiple similar devices, I/O ports or other resources that
can be logically organised as an array.  To simplify these use cases, object
finder array types are provided.  The object finder array type names have
``_array`` added to them:

+------------------------+------------------------------+
| required_device        | required_device_array        |
+------------------------+------------------------------+
| optional_device        | optional_device_array        |
+------------------------+------------------------------+
| required_memory_region | required_memory_region_array |
+------------------------+------------------------------+
| optional_memory_region | optional_memory_region_array |
+------------------------+------------------------------+
| required_memory_bank   | required_memory_bank_array   |
+------------------------+------------------------------+
| optional_memory_bank   | optional_memory_bank_array   |
+------------------------+------------------------------+
| memory_bank_creator    | memory_bank_array_creator    |
+------------------------+------------------------------+
| required_ioport        | required_ioport_array        |
+------------------------+------------------------------+
| optional_ioport        | optional_ioport_array        |
+------------------------+------------------------------+
| required_address_space | required_address_space_array |
+------------------------+------------------------------+
| optional_address_space | optional_address_space_array |
+------------------------+------------------------------+
| required_region_ptr    | required_region_ptr_array    |
+------------------------+------------------------------+
| optional_region_ptr    | optional_region_ptr_array    |
+------------------------+------------------------------+
| required_shared_ptr    | required_shared_ptr_array    |
+------------------------+------------------------------+
| optional_shared_ptr    | optional_shared_ptr_array    |
+------------------------+------------------------------+
| memory_share_creator   | memory_share_array_creator   |
+------------------------+------------------------------+

A common case for an object array finder is a key matrix:

.. code-block:: C++

    class keyboard_base : public device_t, public device_mac_keyboard_interface
    {
    protected:
        keyboard_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
            device_t(mconfig, type, tag, owner, clock),
            device_mac_keyboard_interface(mconfig, *this),
            m_rows(*this, "ROW%u", 0U)
        {
        }

        u8 bus_r()
        {
            u8 result(0xffU);
            for (unsigned i = 0U; m_rows.size() > i; ++i)
            {
                if (!BIT(m_row_drive, i))
                    result &= m_rows[i]->read();
            }
            return result;
        }

        required_ioport_array<10> m_rows;
    };

Constructing an object finder array is similar to constructing an object finder,
except that rather than just a tag you supply a tag format string and index
offset.  In this case, the tags of the I/O ports in the array will be ``ROW0``,
``ROW1``, ``ROW2``, … ``ROW9``.  Note that the object finder array allocates
dynamic storage for the tags, which remain valid until destruction.

The object finder array is used in much the same way as a ``std::array`` of the
underlying object finder type.  It supports indexing, iterators, and range-based
``for`` loops.

Because an index offset is specified, the tags don’t need to use zero-based
indices.  It’s common to use one-based indexing like this:

.. code-block:: C++

    class dooyong_state : public driver_device
    {
    protected:
        dooyong_state(machine_config const &mconfig, device_type type, char const *tag) :
            driver_device(mconfig, type, tag),
            m_bg(*this, "bg%u", 1U),
            m_fg(*this, "fg%u", 1U)
        {
        }

        optional_device_array<dooyong_rom_tilemap_device, 2> m_bg;
        optional_device_array<dooyong_rom_tilemap_device, 2> m_fg;
    };

This causes ``m_bg`` to find devices with tags ``bg1`` and ``bg2``, while
``m_fg`` finds devices with tags ``fg1`` and ``fg2``.  Note that the indexes
into the object finder arrays are still zero-based like any other C array.

It’s also possible to other format conversions, like hexadecimal (``%x`` and
``%X``) or character (``%c``):

.. code-block:: C++

    class eurit_state : public driver_device
    {
    public:
        eurit_state(machine_config const &mconfig, device_type type, char const *tag) :
            driver_device(mconfig, type, tag),
            m_keys(*this, "KEY%c", 'A')
        {
        }

    private:
        required_ioport_array<5> m_keys;
    };

In this case, the key matrix ports use tags ``KEYA``, ``KEYB``, ``KEYC``,
``KEYD`` and ``KEYE``.

When the tags don’t follow a simple ascending sequence, you can supply a
brace-enclosed initialiser list of tags:

.. code-block:: C++

    class seabattl_state : public driver_device
    {
    public:
        seabattl_state(machine_config const &mconfig, device_type type, char const *tag) :
            driver_device(mconfig, type, tag),
            m_digits(*this, { "sc_thousand", "sc_hundred", "sc_half", "sc_unity", "tm_half", "tm_unity" })
        {
        }

    private:
        required_device_array<dm9368_device, 6> m_digits;
    };

If the underlying object finders require additional constructor arguments,
supply them after the tag format and index offset (the same values will be used
for all elements of the array):

.. code-block:: C++

    class dreamwld_state : public driver_device
    {
    public:
        dreamwld_state(machine_config const &mconfig, device_type type, char const *tag) :
            driver_device(mconfig, type, tag),
            m_vram(*this, "vram_%u", 0U, 0x2000U, ENDIANNESS_BIG)
        {
        }

    private:
        memory_share_array_creator<u16, 2> m_vram;
    };

This finds or creates memory shares with tags ``vram_0`` and ``vram_1``, each of
of which is 8 KiB organised as 4,096 big-Endian 16-bit words.


Optional object finders
-----------------------

Optional object finders don’t raise an error if the target object isn’t found.
This is useful in two situations: ``driver_device`` implementations (state
classes) representing a family of systems where some components aren’t present
in all configurations, and devices that can optionally use a resource.  Optional
object finders provide additional member functions for testing whether the
target object was found.

Optional system components
~~~~~~~~~~~~~~~~~~~~~~~~~~

Often a class is used to represent a family of related systems.  If a component
isn’t present in all configurations, it may be convenient to use an optional
object finder to access it.  We’ll use the Sega X-board device as an example:

.. code-block:: C++

    class segaxbd_state : public device_t
    {
    protected:
        segaxbd_state(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock) :
            device_t(mconfig, type, tag, owner, clock),
            m_soundcpu(*this, "soundcpu"),
            m_soundcpu2(*this, "soundcpu2"),
            m_segaic16vid(*this, "segaic16vid"),
            m_pc_0(0),
            m_lastsurv_mux(0),
            m_adc_ports(*this, "ADC%u", 0),
            m_mux_ports(*this, "MUX%u", 0)
        {
        }

	optional_device<z80_device> m_soundcpu;
	optional_device<z80_device> m_soundcpu2;
	required_device<mb3773_device> m_watchdog;
	required_device<segaic16_video_device> m_segaic16vid;
        bool m_adc_reverse[8];
        u8 m_pc_0;
        u8 m_lastsurv_mux;
        optional_ioport_array<8> m_adc_ports;
        optional_ioport_array<4> m_mux_ports;
    };

The ``optional_device`` and ``optional_ioport_array`` members are declared and
constructed in the usual way.  Before accessing the target object, we call an
object finder’s ``found()`` member function to check whether it’s present in the
system (the explicit cast-to-Boolean operator can be used for the same purpose):

.. code-block:: C++

    void segaxbd_state::pc_0_w(u8 data)
    {
        m_pc_0 = data;

        m_watchdog->write_line_ck(BIT(data, 6));

        m_segaic16vid->set_display_enable(data & 0x20);

        if (m_soundcpu.found())
            m_soundcpu->set_input_line(INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
        if (m_soundcpu2.found())
            m_soundcpu2->set_input_line(INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
    }

Optional I/O ports provide a convenience member function called ``read_safe``
that reads the port value if present, or returns the supplied default value
otherwise:

.. code-block:: C++

    u8 segaxbd_state::analog_r()
    {
        int const which = (m_pc_0 >> 2) & 7;
        u8 value = m_adc_ports[which].read_safe(0x10);

        if (m_adc_reverse[which])
            value = 255 - value;

        return value;
    }

    uint8_t segaxbd_state::lastsurv_port_r()
    {
        return m_mux_ports[m_lastsurv_mux].read_safe(0xff);
    }

The ADC ports return 0x10 (16 decimal) if they are not present, while the
multiplexed digital ports return 0xff (255 decimal) if they are not present.
Note that ``read_safe`` is a member of the ``optional_ioport`` itself, and not
a member of the target ``ioport_port`` object (the ``optional_ioport`` is not
dereferenced when using it).

There are some disadvantages to using optional object finders:

* There’s no way to distinguish between the target not being present, and the
  target not being found due to mismatched tags, making it more error-prone.
* Checking whether the target is present may use CPU branch prediction
  resources, potentially hurting performance if it happens very frequently.

Consider whether optional object finders are the best solution, or whether
creating a derived class for the system with additional components is more
appropriate.

Optional resources
~~~~~~~~~~~~~~~~~~

Some devices can optionally use certain resources.  If the host system doesn’t
supply them, the device will still function, although some functionality may not
be available.  For example, the Virtual Boy cartridge slot responds to three
address spaces, called EXP, CHIP and ROM.  If the host system will never use one
or more of them, it doesn’t need to supply a place for the cartridge to install
the corresponding handlers.  (For example a copier may only use the ROM space.)

Let’s look at how this is implemented.  The Virtual Boy cartridge slot device
declares ``optional_address_space`` members for the three address spaces,
``offs_t`` members for the base addresses in these spaces, and inline member
functions for configuring them:

.. code-block:: C++

    class vboy_cart_slot_device :
            public device_t,
            public device_image_interface,
            public device_single_card_slot_interface<device_vboy_cart_interface>
    {
    public:
        vboy_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0U);

	template <typename T> void set_exp(T &&tag, int no, offs_t base)
        {
            m_exp_space.set_tag(std::forward<T>(tag), no);
            m_exp_base = base;
        }
	template <typename T> void set_chip(T &&tag, int no, offs_t base)
        {
            m_chip_space.set_tag(std::forward<T>(tag), no);
            m_chip_base = base;
        }
	template <typename T> void set_rom(T &&tag, int no, offs_t base)
        {
            m_rom_space.set_tag(std::forward<T>(tag), no);
            m_rom_base = base;
        }

    protected:
        virtual void device_start() override;

    private:
        optional_address_space m_exp_space;
        optional_address_space m_chip_space;
        optional_address_space m_rom_space;
        offs_t m_exp_base;
        offs_t m_chip_base;
        offs_t m_rom_base;

	device_vboy_cart_interface *m_cart;
    };

    DECLARE_DEVICE_TYPE(VBOY_CART_SLOT, vboy_cart_slot_device)

The object finders are constructed with dummy values for the tags and space
numbers (``finder_base::DUMMY_TAG`` and -1):

.. code-block:: C++

    vboy_cart_slot_device::vboy_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
        device_t(mconfig, VBOY_CART_SLOT, tag, owner, clock),
        device_image_interface(mconfig, *this),
        device_single_card_slot_interface<device_vboy_cart_interface>(mconfig, *this),
        m_exp_space(*this, finder_base::DUMMY_TAG, -1, 32),
        m_chip_space(*this, finder_base::DUMMY_TAG, -1, 32),
        m_rom_space(*this, finder_base::DUMMY_TAG, -1, 32),
        m_exp_base(0U),
        m_chip_base(0U),
        m_rom_base(0U),
        m_cart(nullptr)
    {
    }

To help detect configuration errors, we’ll check for cases where address spaces
have been configured but aren’t present:

.. code-block:: C++

    void vboy_cart_slot_device::device_start()
    {
        if (!m_exp_space && ((m_exp_space.finder_tag() != finder_base::DUMMY_TAG) || (m_exp_space.spacenum() >= 0)))
            throw emu_fatalerror("%s: Address space %d of device %s not found (EXP)\n", tag(), m_exp_space.spacenum(), m_exp_space.finder_tag());

        if (!m_chip_space && ((m_chip_space.finder_tag() != finder_base::DUMMY_TAG) || (m_chip_space.spacenum() >= 0)))
            throw emu_fatalerror("%s: Address space %d of device %s not found (CHIP)\n", tag(), m_chip_space.spacenum(), m_chip_space.finder_tag());

        if (!m_rom_space && ((m_rom_space.finder_tag() != finder_base::DUMMY_TAG) || (m_rom_space.spacenum() >= 0)))
            throw emu_fatalerror("%s: Address space %d of device %s not found (ROM)\n", tag(), m_rom_space.spacenum(), m_rom_space.finder_tag());

        m_cart = get_card_device();
    }
