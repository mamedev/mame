The new SCSI subsystem
======================

Introduction
------------

The **nscsi** subsystem was created to allow an implementation to be closer to the physical reality, making it easier (hopefully) to implement new controller chips from the documentations.


Global structure
----------------

Parallel SCSI is built around a symmetric bus to which a number of devices are connected.  The bus is composed of 9 control lines (for
now, later SCSI versions may have more) and up to 32 data lines (but currently implemented chips only handle 8).  All the lines are open
collector, which means that either one or multiple chip connect the line to ground and the line, of course, goes to ground, or no chip
drives anything and the line stays at Vcc.  Also, the bus uses inverted logic, where ground means 1.  SCSI chips traditionally work
in logical and not physical levels, so the nscsi subsystem also works in logical levels and does a logical-or of all the outputs of the
devices.

Structurally, the implementation is done around two main classes: **nscsi_bus_devices** represents the bus, and **nscsi_device** represents an individual device.  A device only communicate with the bus, and the bus takes care of transparently handling the device discovery and communication.  In addition the **nscsi_full_device** class proposes a SCSI device with the SCSI protocol implemented making building generic SCSI devices like hard drives or CD-ROM readers easier.


Plugging in a SCSI bus in a driver
----------------------------------

The nscsi subsystem leverages the slot interfaces and the device naming to allow for a configurable yet simple bus implementation.

First you need to create a list of acceptable devices to plug on the bus.  This usually comprises of **cdrom**, **harddisk** and the controller chip.  For instance:

|
| static SLOT_INTERFACE_START( next_scsi_devices )
|     SLOT_INTERFACE("cdrom", NSCSI_CDROM)
|     SLOT_INTERFACE("harddisk", NSCSI_HARDDISK)
|     SLOT_INTERFACE_INTERNAL("ncr5390", NCR5390)
| SLOT_INTERFACE_END
|

The **_INTERNAL** interface indicates a device that is not user-selectable, which is useful for the controller.

Then in the machine config (or in a fragment config) you need to first add the bus, and then the (potential) devices as sub-devices of the bus with the SCSI ID as the name.  For instance you can use:

|
|     MCFG_NSCSI_BUS_ADD("scsibus")
|     MCFG_NSCSI_ADD("scsibus:0", next_scsi_devices, "cdrom", 0, 0, 0, false)
|     MCFG_NSCSI_ADD("scsibus:1", next_scsi_devices, "harddisk", 0, 0, 0, false)
|     MCFG_NSCSI_ADD("scsibus:2", next_scsi_devices, 0, 0, 0, 0, false)
|     MCFG_NSCSI_ADD("scsibus:3", next_scsi_devices, 0, 0, 0, 0, false)
|     MCFG_NSCSI_ADD("scsibus:4", next_scsi_devices, 0, 0, 0, 0, false)
|     MCFG_NSCSI_ADD("scsibus:5", next_scsi_devices, 0, 0, 0, 0, false)
|     MCFG_NSCSI_ADD("scsibus:6", next_scsi_devices, 0, 0, 0, 0, false)
|     MCFG_NSCSI_ADD("scsibus:7", next_scsi_devices, "ncr5390", 0, &next_ncr5390_interface, 10000000, true)
|

That configuration puts as default a CD-ROM reader on SCSI ID 0 and a hard drive on SCSI ID 1, and forces the controller on ID 7.  The
parameters of add are:

- device tag, comprised of bus-tag:scsi-id
- the list of acceptable devices
- the device name as per the list, if one is to be there by default
- the device input config, if any (and there usually isn't one)
- the device configuration structure, usually for the controller only
- the frequency, usually for the controller only
- "**false**" for a user-modifiable slot, "**true**" for a fixed slot

The full device name, for mapping purposes, will be **bus-tag:scsi-id:device-type**, i.e. "*scsibus:7:ncr5390*" for our
controller here.


Creating a new SCSI device using nscsi_device
---------------------------------------------

The base class "**nscsi_device**" is to be used for down-to-the-metal devices, i.e. SCSI controller chips.  The class provides three
variables and one method.  The first variable, **scsi_bus**, is a pointer to the **nscsi_bus_device**. The second, **scsi_refid**, is an opaque reference to pass to the bus on some operations. Finally, **scsi_id** gives the SCSI ID as per the device tag. It's written once at startup and never written or read afterwards, the device can do whatever it wants with the value or the variable.

The virtual method **scsi_ctrl_changed** is called when watched-for control lines change. Which lines are watched is defined through the bus.

The bus proposes five methods to access the lines.  The read methods are **ctrl_r()** and **data_r()**.  The meaning of the control bits are defined in the **S_\*** enum of **nscsi_device**. The bottom three bits (**INP**, **CTL** and **MSG**) are setup so that masking with 7 (**S_PHASE_MASK**) gives the traditional numbers for the phases, which are also available with the **S_PHASE_\*** enum.

Writing the data lines is done with **data_w(scsi_refid, value)**.

Writing the control lines is done with **ctrl_w(scsi_refid, value, mask-of-lines-to-change)**. To change all control lines in one call use **S_ALL** as the mask.

Of course, what is read is the logical-or of all of what is driven by all devices.

Finally, the method **ctrl_wait_w(scsi_id, value, mask-of-wait-lines-to-change)** allows to select which control lines are
watched. The watch mask is per-device, and the device method **scsi_ctrl_changed** is called whenever a control line in the mask changes due to an action of another device (not itself, to avoid an annoying and somewhat useless recursion).

Implementing the controller is then just a matter of following the state machines descriptions, at least if they're available.  The only
part often not described is the arbitration/selection, which is documented in the SCSI standard though.  For an initiator (which is what the controller essentially always is), it goes like this:

* wait for the bus to be idle
* assert the data line which number is your scsi_id (1 << scsi_id)
* assert the busy line
* wait the arbitration time
* check that the of the active data lines the one with the highest number is yours

  * if no, the arbitration was lost, stop driving anything and restart at the beginning

* assert the select line (at that point, the bus is yours)
* wait a short while
* keep your data line asserted, assert the data line which number is the SCSI ID of the target
* wait a short while
* assert the atn line if needed, de-assert busy
* wait for busy to be asserted or timeout

  * timeout means nobody is answering at that id, de-assert everything and stop

* wait a short while for de-skewing
* de-assert the data bus and the select line
* wait a short while

and then you're done, you're connected with the target until the target de-asserts the busy line, either because you asked it to or just
to annoy you. The de-assert is called a disconnect.

The **ncr5390** is an example of how to use a two-level state machine to handle all the events.


Creating a new SCSI device using **nscsi_full_device**
------------------------------------------------------

The base class "**nscsi_full_device**" is used to create HLE-d SCSI devices intended for generic uses, like hard drives, CD-ROMs, perhaps scanners, etc.  The class provides the SCSI protocol handling, leaving only the command handling and (optionally) the message handling to the implementation.

The class currently only support target devices.

The first method to implement is **scsi_command()**.  That method is called when a command has fully arrived. The command is available in **scsi_cmdbuf[]**, and its length is in **scsi_cmdsize** (but the length is generally useless, the command first byte giving it).  The 4096-bytes **scsi_cmdbuf** array is then freely modifiable.

In **scsi_command()**, the device can either handle the command or pass it up with **nscsi_full_device::scsi_command()**.

To handle the command, a number of methods are available:

- **get_lun(lua-set-in-command)** will give you the LUN to work on (the in-command one can be overriden by a message-level one).

- **bad_lun()** replies to the host that the specific LUN is unsupported.

- **scsi_data_in(buffer-id, size)** sends size bytes from buffer *buffer-id*

- **scsi_data_out(buffer-id, size)** receives size bytes into buffer *buffer-id*

- **scsi_status_complete(status)** ends the command with a given status.

- **sense(deferred, key)** prepares the sense buffer for a subsequent request-sense command, which is useful when returning a check-condition status.

The **scsi_data_\*** and **scsi_status_complete** commands are queued, the command handler should call them all without waiting.

*buffer-id* identifies a buffer.  0, aka **SBUF_MAIN**, targets the **scsi_cmdbuf** buffer. Other acceptable values are 2 or more. 2+ ids are handled through the **scsi_get_data** method for read and **scsi_put_data** for write.

**UINT8 device::scsi_get_data(int id, int pos)** must return byte pos of buffer id, upcalling in **nscsi_full_device** for id < 2.

**void device::scsi_put_data(int id, int pos, UINT8 data)** must write byte pos in buffer id, upcalling in **nscsi_full_device** for id < 2.

**scsi_get_data** and **scsi_put_data** should do the external reads/writes when needed.

The device can also override **scsi_message** to handle SCSI messages other than the ones generically handled, and it can also override some of the timings (but a lot of them aren't used, beware).

A number of enums are defined to make things easier. The **SS_\*** enum gives status returns (with **SS_GOOD** for all's well).  The **SC_\*** enum gives the scsi commands.  The **SM_\*** enum gives the SCSI messages, with the exception of identify (which is **80-ff**, doesn't really fit in an enum).


What's missing in **scsi_full_device**
--------------------------------------

- **Initiator support** We have no initiator device to HLE at that point.

- **Delays** A scsi_delay command would help giving more realistic timings to the CD-ROM reader in particular.

- **Disconnected operation** Would first require delays and in addition an emulated OS that can handle it.

- **16-bits wide operation** needs an OS and an initiator that can handle it.


What's missing in the ncr5390 (and probably future other controllers)
---------------------------------------------------------------------

- **Bus free detection** Right now the bus is considered free if the controllers isn't using it, which is true. This may change once disconnected operation is in.
- **Target commands** We don't emulate (vs. HLE) any target yet.
