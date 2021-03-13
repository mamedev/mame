The device_memory_interface
===========================

.. contents:: :local:


1. Capabilities
---------------

The device memory interface provides devices with the capability of
creating address spaces, to which address maps can be associated.
It’s used for any device that provides a (logical) address/data bus
that other devices can be connected to.  That’s mainly, but not solely,
CPUs.

The interface allows for an unlimited set of address spaces, numbered
with small, non-negative values.  The IDs index vectors, so they should
stay small to keep the lookup fast.  Spaces numbered 0-3 have associated
constant name:

+----+---------------+
| ID | Name          |
+====+===============+
| 0  | AS_PROGRAM    |
+----+---------------+
| 1  | AS_DATA       |
+----+---------------+
| 2  | AS_IO         |
+----+---------------+
| 3  | AS_OPCODES    |
+----+---------------+

Spaces 0 and 3, i.e. ``AS_PROGRAM`` and ``AS_OPCODES``, are special for
the debugger and some CPUs.  ``AS_PROGRAM`` is use by the debugger and
the CPUs as the space from which the CPU reads its instructions for the
disassembler.  When present, ``AS_OPCODES`` is used by the debugger and
some CPUs to read the opcode part of the instruction.  What opcode means
is device-dependant, for instance for the Z80 it's the initial byte(s)
which are read with the M1 signal asserted, while for the 68000 is means
every instruction word plus PC-relative accesses.  The main, but not
only, use of ``AS_OPCODES`` is to implement hardware decryption of
instructions separately from data.


2. Setup
--------

.. code-block:: C++

    std::vector<std::pair<int, const address_space_config *>> memory_space_config() const;

The device must override that method to provide a vector of pairs
comprising of a space number and an associated ``address_space_config``
describing its configuration.  Some examples to look up when needed:

* Standard two-space vector:
  `v60_device <https://git.redump.net/mame/tree/src/devices/cpu/v60/v60.cpp?h=mame0226>`_
* Conditional ``AS_OPCODES``:
  `z80_device <https://git.redump.net/mame/tree/src/devices/cpu/z80/z80.cpp?h=mame0226>`_
* Inherit configuration and add a space:
  `hd647180x_device <https://git.redump.net/mame/tree/src/devices/cpu/z180/hd647180x.cpp?h=mame0226>`_
* Inherit configuration and modify a space:
  `tmpz84c011_device <https://git.redump.net/mame/tree/src/devices/cpu/z80/tmpz84c011.cpp?h=mame0226>`_

.. code-block:: C++

    bool has_configured_map(int index = 0) const;

The ``has_configured_map`` method allows to test whether an
``address_map`` has been associated with a given space in the
``memory_space_config`` method .  That allows optional memory spaces to
be implemented, such as ``AS_OPCODES`` in certain CPU cores.


3. Associating maps to spaces
-----------------------------
Associating maps to spaces is done at the machine configuration level,
after the device is instantiated.

.. code-block:: C++

    void set_addrmap(int spacenum, T &obj, Ret (U::*func)(Params...));
    void set_addrmap(int spacenum, Ret (T::*func)(Params...));
    void set_addrmap(int spacenum, address_map_constructor map);

These function associate a map with a given space.  Address maps
associated with non-existent spaces are ignored (no warning given).  The
first form takes a reference to an object and a method to call on that
object.  The second form takes a method to call on the current device
being configured.  The third form takes an ``address_map_constructor``
to copy.  In each case, the function must be callable with reference to
an ``address_map`` object as an argument.

To remove a previously configured address map, call ``set_addrmap`` with
a default-constructed ``address_map_constructor`` (useful for removing a
map for an optional space in a derived machine configuration).

As an example, here’s the address map configuration for the main CPU in
the Hana Yayoi and Hana Fubuki machines, with all distractions removed:

.. code-block:: C++

    class hnayayoi_state : public driver_device
    {
    public:
        void hnayayoi(machine_config &config);
        void hnfubuki(machine_config &config);

    private:
	required_device<cpu_device> m_maincpu;

	void hnayayoi_map(address_map &map);
	void hnayayoi_io_map(address_map &map);
	void hnfubuki_map(address_map &map);
    };

    void hnayayoi_state::hnayayoi(machine_config &config)
    {
        Z80(config, m_maincpu, 20000000/4);
        m_maincpu->set_addrmap(AS_PROGRAM, &hnayayoi_state::hnayayoi_map);
        m_maincpu->set_addrmap(AS_IO, &hnayayoi_state::hnayayoi_io_map);
    }

    void hnayayoi_state::hnfubuki(machine_config &config)
    {
        hnayayoi(config);

        m_maincpu->set_addrmap(AS_PROGRAM, &hnayayoi_state::hnfubuki_map);
        m_maincpu->set_addrmap(AS_IO, address_map_constructor());
    }


4. Accessing the spaces
-----------------------

.. code-block:: C++

    address_space &space(int index = 0) const;

Returns the specified address space post-initialization.  The specified
address space must exist.

.. code-block:: C++

    bool has_space(int index = 0) const;

Indicates whether a given space actually exists.


5. MMU support for disassembler
-------------------------------

.. code-block:: C++

    bool translate(int spacenum, int intention, offs_t &address);

Does a logical to physical address translation through the device's
MMU.  spacenum gives the space number, intention for the type of the
future access (``TRANSLATE_(READ\|WRITE\|FETCH)(\|_USER\|_DEBUG)``)
and address is an in/out parameter holding the address to translate on
entry and the translated version on return.  Should return ``true`` if
the translation went correctly, or ``false`` if the address is unmapped.

Note that for some historical reason, the device itself must override
the virtual method ``memory_translate`` with the same signature.
