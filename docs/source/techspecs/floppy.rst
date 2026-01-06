The new floppy subsystem
========================

1. Introduction
---------------

The new floppy subsystem aims at emulating the behaviour of floppies and floppy controllers at a level low enough that protections work as a matter of course.  It reaches its goal by following the real hardware configuration:

- a floppy image class keeps in memory the magnetic state of the floppy surface and its physical characteristics

- an image handler class talks with the floppy image class to simulate the floppy drive, providing all the signals you have on a floppy drive connector

- floppy controller devices talk with the image handler and provide the register interfaces to the host we all know and love

- format handling classes are given the task of statelessly converting to and from an on-disk image format to the in-memory magnetic state format the floppy image class manages


2. Floppy storage 101
---------------------

2.1. Floppy disk
~~~~~~~~~~~~~~~~

A floppy disk is a disc that stores magnetic orientations on their surface disposed in a series on concentric circles called tracks or cylinders [1]_.  Its main characteristics are its size (goes from a diameter of around 2.8" to 8") , its number of writable sides (1 or 2) and its magnetic resistivity.  The magnetic resistivity indicates how close magnetic orientation changes can happen and the information kept.  That's one third of what defines the term "density" that is so often used for floppies (the other two are floppy drive head size and bit-level encoding).

The magnetic orientations are always binary, e.g. they're one way or the opposite, there's no intermediate state.  Their direction can either be tangentially to the track, i.e. in the direction of or opposite to the rotation, or in the case of perpendicular recording the direction is perpendicular to the disc surface (hence the name). Perpendicular recording allows for closer orientation changes by writing the magnetic information more deeply, but arrived late in the technology lifetime.  2.88Mb disks and the floppy children (Zip drives, etc.) used perpendicular recording.  For simulation purposes the direction is not important, only the fact that only two orientations are possible is.  Two more states are possible though: a portion of a track can be demagnetized (no orientation) or damaged (no orientation and can't be written to).

A specific position in the disk rotation triggers an index pulse. That position can be detected through a hole in the surface (very visible in 5.25" and 3" floppies for instance) or through a specific position of the rotating center (3.5" floppies, perhaps others).  This index pulse is used to designate the beginning of the track, but is not used by every system.  Older 8" floppies have multiple index holes used to mark the beginning of sectors (called hard sectoring) but one of them is positioned differently to be recognized as the track start, and the others are at fixed positions relative to the origin one.


2.2. Floppy drive
~~~~~~~~~~~~~~~~~

A floppy drive is what reads and writes a floppy disk.  It includes an assembly capable of rotating the disk at a fixed speed and one or two magnetic heads tied to a positioning motor to access the tracks.

The head width and positioning motor step size decides how many tracks are written on the floppy.  Total number of tracks goes from 32 to 84 depending on the floppy and drive, with the track 0 being the most exterior (longer) one of the concentric circles, and the highest numbered the smallest interior circle.  As a result the tracks with the lowest numbers have the lowest physical magnetic orientation density, hence the best reliability.  Which is why important and/or often changed structures like the boot block or the fat allocation table are at track 0.  That is also where the terminology "stepping in" to increase the track number and "stepping out" to decrease it comes from.  The number of tracks available is the second part of what is usually behind the term "density".

A sensor detects when the head is on track 0 and the controller is not supposed to try to go past it.  In addition physical blocks prevent the head from going out of the correct track range.  Some systems (Apple II, some C64) do not take the track 0 sensor into account and just wham the head against the track 0 physical block, giving a well-known crash noise and eventually damaging the head alignment.

Also, some systems (Apple II and C64 again) have direct access to the phases of the head positioning motor, allowing to trick the head into going between tracks, in middle or even quarter positions.  That was not usable to write more tracks, since the head width did not change, but since reliable reading was only possible with the correct position it was used for some copy protection systems.

The disk rotates at a fixed speed for a given track.  The most usual speed is 300 rpm for every track, with 360 rpm found for HD 5.25" floppies and most 8" ones, and a number of different values like 90 rpm for the earlier floppies or 150 rpm for an HD floppy in an Amiga. Having a fixed rotational speed for the whole disk is called Constant Angular Velocity (CAV, almost everybody) or Zoned Constant Angular Velocity (ZCAV, C64) depending on whether the read/write bitrate is constant or track-dependant.  Some systems (Apple II, Mac) vary the rotational speed depending on the track (something like 394 rpm up to 590 rpm) to end up with a Constant Linear Velocity (CLV).  The idea behind ZCAV/CLV is to get more bits out of the media by keeping the minimal spacing between magnetic orientation transitions close to the best the support can do.  It seems that the complexity was not deemed worth it since almost no system does it.

Finally, after the disc rotates and the head is over the proper track reading happens.  The reading is done through an inductive head, which gives it the interesting characteristic of not reading the magnetic orientation directly but instead of being sensitive to orientation inversions, called flux transitions.  This detection is weak and somewhat uncalibrated, so an amplifier with Automatic Gain Calibration (AGC) and a peak detector are put behind the head to deliver clean pulses.  The AGC slowly increases the amplification level until a signal goes over the threshold, then modulates its gain so that said signal is at a fixed position over the threshold.  Afterwards the increase happens again.  This makes the amplifier calibrate itself to the signals read from the floppy as long as flux transitions happen often enough.  Too long and the amplification level will reach a point where the random noise the head picks from the environment is amplified over the threshold, creating a pulse where none should be. Too long in our case happens to be around 16-20us with no transitions. That means a long enough zone with a fixed magnetic orientation or no orientation at all (demagnetized or damaged) is going to be read as a series of random pulses after a brief delay.  This is used by protections and is known as "weak bits", which read differently each time they're accessed.

A second level of filtering happens after the peak detector.  When two transitions are a little close (but still over the media threshold) a bouncing effect happens between them giving two very close pulses in the middle in addition to the two normal pulses.  The floppy drive detects when pulses are too close and filter them out, leaving the normal ones.  As a result, if one writes a train of high-frequency pulses to the floppy they will be read back as a train of too close pulses (weak because they're over the media tolerance, but picked up by the AGC anyway, only somewhat unreliably) they will be all filtered out, giving a large amount of time without any pulse in the output signal.  This is used by some protections since it's not writable with a normally clocked controller.

Writing is symmetrical, with a series of pulses sent which make the write head invert the magnetic field orientation each time a pulse is received.

So, in conclusion, the floppy drive provides inputs to control disk rotation and head position (and choice when double-sided), and the data goes both way as a train of pulses representing magnetic orientation inversions.  The absolute value of the orientation itself is never known.


2.3. Floppy controller
~~~~~~~~~~~~~~~~~~~~~~

The task of the floppy controller is to turn the signals to/from the floppy drive into something the main CPU can digest.  The level of support actually done by the controller is extremely variable from one device to the other, from pretty much nothing (Apple II, C64) through minimal (Amiga) to complete (Western Digital chips, uPD765 family). Usual functions include drive selection, motor control, track seeking and of course reading and writing data.  Of these only the last two need to be described, the rest is obvious.

The data is structured at two levels: how individual bits (or nibbles, or bytes) are encoded on the surface, and how these are grouped in individually-addressable sectors.  Two standards exist for these, called FM and MFM, and in addition a number of systems use their home-grown variants.  Moreover, some systems such as the Amiga use a standard bit-level encoding (MFM) but a homegrown sector-level organisation.


2.3.1. Bit-level encodings
''''''''''''''''''''''''''

2.3.1.1. Cell organization
``````````````````````````

All floppy controllers, even the wonkiest like the Apple II one, start by dividing the track in equally-sized cells.  They're angular sections in the middle of which a magnetic orientation inversion may be present.  From a hardware point of view the cells are seen as durations, which combined with the floppy rotation give the section. For instance the standard MFM cell size for a 3" double-density floppy is 2us, which combined with the also standard 300 rpm rotational speed gives an angular size of 1/100000th of a turn.  Another way of saying it is that there are 100K cells in a 3" DD track.

In every cell there may or may not be a magnetic orientation transition, e.g. a pulse coming from (reading) or going to (writing) the floppy drive.  A cell with a pulse is traditionally noted '1', and one without '0'.  Two constraints apply to the cell contents though. First, pulses must not be too close together or they'll blur each-other and/or be filtered out.  The limit is slightly better than 1/50000th of a turn for single and double density floppies, half that for HD floppies, and half that again for ED floppies with perpendicular recording.  Second, they must not be too away from each other or either the AGC is going to get wonky and introduce phantom pulses or the controller is going to lose sync and get a wrong timing on the cells on reading.  Conservative rule of thumb is not to have more than three consecutive '0' cells.

Of course protections play with that to make formats not reproducible by the system controller, either breaking the three-zeroes rule or playing with the cells durations/sizes.

Bit encoding is then the art of transforming raw data into a cell 0/1 configuration that respects the two constraints.

2.3.1.2. FM encoding
````````````````````

The very first encoding method developed for floppies is called Frequency Modulation, or FM.  The cell size is set at slightly over the physical limit, e.g. 4us.  That means it is possible to reliably have consecutive '1' cells.  Each bit is encoded on two cells:

- the first cell, called the clock bit, is '1'

- the second cell, called data bit, is the bit

Since every other cell at least is '1' there is no risk of going over three zeroes.

The name Frequency Modulation simply derives from the fact that a 0 is encoded with one period of a 125Khz pulse train while a 1 is two periods of a 250Khz pulse train.

2.3.1.3. MFM encoding
`````````````````````
The FM encoding has been superseded by the Modified Frequency Modulation encoding, which can cram exactly twice as much data on the same surface, hence its other name of "double density".  The cell size is set at slightly over half the physical limit, e.g. 2us usually. The constraint means that two '1' cells must be separated by at least one '0' cell.  Each bit is once again encoded on two cells:

- the first cell, called the clock bit, is '1' if both the previous and current data bits are 0, '0' otherwise

- the second cell, called data bit, is the bit

The minimum space rule is respected since a '1' clock bit is by definition surrounded by two '0' data bits, and a '1' data bit is surrounded by two '0' clock bits.  The longest '0'-cell string possible is when encoding 101 which gives x10001, respecting the maximum of three zeroes.

2.3.1.4. GCR encodings
``````````````````````

Group Coded Recording, or GCR, encodings are a class of encodings where strings of bits at least nibble-size are encoded into a given cell stream given by a table.  It has been used in particular by the Apple II, the Mac and the C64, and each system has its own table, or tables.

2.3.1.5. Other encodings
````````````````````````

Other encodings exist, like M2FM, but they're very rare and system-specific.

2.3.1.6. Reading back encoded data
``````````````````````````````````

Writing encoded data is easy: you only need a clock at the appropriate frequency and send or not a pulse on the clock edges.  Reading back the data is where the fun is.  Cells are a logical construct and not a physical measurable entity.  Rotational speeds very around the defined one (±2% is not rare), and local perturbations (air turbulence, surface distance…) make the instantaneous speed very variable in general.  So to extract the cell values stream, the controller must dynamically synchronize with the pulse train that the floppy head picks up.  The principle is simple: a cell-sized duration window is built within which the presence of at least one pulse indicates the cell is a '1', and the absence of any a '0'.  After reaching the end of the window, the starting time is moved appropriately to try to keep the observed pulse at the exact middle of the window.  This allows the phase to be corrected on every '1' cell, making the synchronization work if the rotational speed is not too off.  Subsequent generations of controllers used Phase-Locked Loops (PLLs) which vary both phase and window duration to adapt better to inaccurate rotational speeds, usually with a tolerance of ±15%.

Once the cell data stream is extracted, decoding depends on the encoding.  In the FM and MFM case the only question is to recognize data bits from clock bits, while in GCR the start position of the first group should be found.  That second level of synchronization is handled at a higher level using patterns not found in a normal stream.


2.3.2. Sector-level organization
''''''''''''''''''''''''''''''''

Floppies have been designed for read/write random access to reasonably sized blocks of data.  Track selection allows for a first level of random access and sizing, but the ~6K of a double density track would be too big a block to handle.  256/512 bytes are considered a more appropriate value.  To that end data on a track is organized as a series of (sector header, sector data) pairs where the sector header indicates important information like the sector number and size, and the sector data contains the data.  Sectors have to be broken in two parts because while reading is easy, read the header then read the data if you want it, writing requires reading the header to find the correct place then once that is done switching on the writing head for the data.  Starting writing is not instantaneous and will not be perfectly phase-aligned with the read header, so space for synchronization is required between header and data.

In addition somewhere in the sector header and in the sector data are pretty much always added some kind of checksum allowing to know whether the data was damaged or not.

FM and MFM have (not always used) standard sector layout methods.

2.3.2.1. FM sector layout
`````````````````````````

The standard "PC" track/sector layout for FM is as such:

- A number of FM-encoded 0xff (usually 40)

- 6 FM-encoded 0x00 (giving a steady 125KHz pulse train)

- The 16-cell stream 1111011101111010 (f77a, clock 0xd7, data 0xfc)

- A number of FM-encoded 0xff (usually 26, very variable)

Then for each sector:
- 6 FM-encoded 0x00 (giving a steady 125KHz pulse train)

- The 16-cell stream 1111010101111110 (f57e, clock 0xc7, data 0xfe)

- Sector header, e.g. FM encoded track, head, sector, size code and two bytes of crc

- 11 FM-encoded 0xff

- 6 FM-encoded 0x00 (giving a steady 125KHz pulse train)

- The 16-cell stream 1111010101101111 (f56f, clock 0xc7, data 0xfb)

- FM-encoded sector data followed by two bytes of crc

- A number of FM-encoded 0xff (usually 48, very variable)

The track is finished with a stream of '1' cells.

The 125KHz pulse trains are used to lock the PLL to the signal correctly.  The specific 16-cells streams allow to distinguish between clock and data bits by providing a pattern that is not supposed to happen in normal FM-encoded data.  In the sector header track numbers start at 0, heads are 0/1 depending on the size, sector numbers usually start at 1 and size code is 0 for 128 bytes, 1 for 256, 2 for 512, etc.

The CRC is a cyclic redundancy check of the data bits starting with the mark just after the pulse train using polynom 0x11021.

The Western Digital-based controllers usually get rid of everything but some 0xff before the first sector and allow a better use of space as a result.

2.3.2.2. MFM sector layout
``````````````````````````

The standard "PC" track/sector layout for MFM is as such:

- A number of MFM-encoded 0x4e (usually 80)

- 12 FM-encoded 0x00 (giving a steady 250KHz pulse train)

- 3 times the 16-cell stream 0101001000100100 (5224, clock 0x14, data 0xc2)

- The MFM-encoded value 0xfc

- A number of MFM-encoded 0x4e (usually 50, very variable)

Then for each sector:

- 12 FM-encoded 0x00 (giving a steady 250KHz pulse train)

- 3 times the 16-cell stream 0100010010001001 (4489, clock 0x0a, data 0xa1)

- Sector header, e.g. MFM-encoded 0xfe, track, head, sector, size code and two bytes of crc

- 22 MFM-encoded 0x4e

- 12 MFM-encoded 0x00 (giving a steady 250KHz pulse train)

- 3 times the 16-cell stream 0100010010001001 (4489, clock 0x0a, data 0xa1)

- MFM-encoded 0xfb, sector data followed by two bytes of crc

- A number of MFM-encoded 0x4e (usually 84, very variable)

The track is finished with a stream of MFM-encoded 0x4e.

The 250KHz pulse trains are used to lock the PLL to the signal correctly.  The cell pattern 4489 does not appear in normal MFM-encoded data and is used for clock/data separation.

As for FM, the Western Digital-based controllers usually get rid of everything but some 0x4e before the first sector and allow a better use of space as a result.

2.3.2.3. Formatting and write splices
`````````````````````````````````````

To be usable, a floppy must have the sector headers and default sector data written on every track before using it.  The controller starts writing at a given place, often the index pulse but on some systems whenever the command is sent, and writes until a complete turn is done.  That's called formatting the floppy.  At the point where the writing stops there is a synchronization loss since there is no chance the cell stream clock warps around perfectly.  This brutal phase change is called a write splice, specifically the track write splice. It is the point where writing should start if one wants to raw copy the track to a new floppy.

Similarly two write splices are created when a sector is written at the start and end of the data block part.  They're not supposed to happen on a mastered disk though, even if there are some rare exceptions.


3. The new implementation
-------------------------

3.1. Floppy disk representation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


The floppy disk contents are represented by the class floppy_image.  It contains information of the media type and a representation of the magnetic state of the surface.

The media type is divided in two parts.  The first half indicates the physical form factor, i.e. all medias with that form factor can be physically inserted in a reader that handles it.  The second half indicates the variants which are usually detectable by the reader, such as density and number of sides.

Track data consists of a series of 32-bits lsb-first values representing magnetic cells.  Bits 0-27 indicate the absolute position of the start of the cell (not the size), and bits 28-31 the type.  Type can be:

- 0, MG_A -> Magnetic orientation A

- 1, MG_B -> Magnetic orientation B

- 2, MG_N -> Non-magnetized zone (neutral)

- 3, MG_D -> Damaged zone, reads as neutral but cannot be changed by writing

The position is in angular units of 1/200,000,000th of a turn.  It corresponds to one nanosecond when the drive rotates at 300 rpm.

The last cell implicit end position is of course 200,000,000.

Unformatted tracks are encoded as zero-size.

The "track splice" information indicates where to start writing if you try to rewrite a physical disk with the data.  Some preservation formats encode that information, it is guessed for others.  The write track function of fdcs should set it.  The representation is the angular position relative to the index.

3.2. Converting to and from the internal representation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

3.2.1. Class and interface
''''''''''''''''''''''''''

We need to be able to convert on-disk formats of the floppy data to and from the internal representation.  This is done through classes derived from floppy_image_format_t.  The interface to be implemented includes:
- **name()** gives the short name of the on-disk format

- **description()** gives a short description of the format

- **extensions()** gives a comma-separated list of file name extensions found for that format

- **supports_save()** returns true is converting to that external format is supported

- **identify(file, form factor)** gives a 0-100 score for the file to be of that format:

  - **0** = not that format

  - **100** = certainly that format

  - **50** = format identified from file size only

- **load(file, form factor, floppy_image)** loads an image and converts it into the internal representation

- **save(file, floppy_image)** (if implemented) converts from the internal representation and saves an image

All of these methods are supposed to be stateless.

3.2.2. Conversion helper methods
''''''''''''''''''''''''''''''''


A number of methods are provided to simplify writing the converter classes.


3.2.2.1. Load-oriented conversion methods
`````````````````````````````````````````


| **generate_track_from_bitstream(track number,**
|                               **head number,**
|                               **UINT8 \*cell stream,**
|                               **int cell count,**
|                               **floppy image)**
|

  Takes a stream of cell types (0/1), MSB-first, converts it to the internal format and stores it at the given track and head in the given image.

| **generate_track_from_levels(track number,**
|                            **head number,**
|                            **UINT32 \*cell levels,**
|                            **int cell count,**
|                            **splice position,**
|                            **floppy image)**

  Takes a variant of the internal format where each value represents a cell, the position part of the values is the size of the cell and the level part is MG_0, MG_1 for normal cell types, MG_N, MG_D for unformatted/damaged cells, and MG_W for Dungeon-Master style weak bits.  Converts it into the internal format.  The sizes are normalized so that they total to a full turn.

| **normalize_times(UINT32 \*levels,**
|                 **int level_count)**

  Takes an internal-format buffer where the position part represents angle until the next change and turns it into a normal positional stream, first ensuring that the total size is normalized to a full turn.


3.2.2.2. Save-oriented conversion methods
`````````````````````````````````````````

| **generate_bitstream_from_track(track number,**
|                               **head number,**
|                               **base cell size**,
|                               **UINT8 \*cell stream,**
|                               **int &cell_stream_size,**
|                               **floppy image)**

  Extract a cell 0/1 stream from the internal format using a PLL setup with an initial cell size set to 'base cell size' and a +/- 25% tolerance.


| **struct desc_xs { int track, head, size; const UINT8 \*data }**
| **extract_sectors_from_bitstream_mfm_pc(...)**
| **extract_sectors_from_bitstream_fm_pc(const UINT8 \*cell stream,**
|                                      **int cell_stream_size,**
|                                      **desc_xs \*sectors,**
|                                      **UINT8 \*sectdata,**
|                                      **int sectdata_size)**

  Extract standard mfm or fm sectors from a regenerated cell stream.  Sectors must point to an array of 256 desc_xs.

  An existing sector is recognizable by having ->data non-null. Sector data is written in sectdata up to sectdata_size bytes.


| **get_geometry_mfm_pc(...)**
| **get_geometry_fm_pc(floppy image,**
|                     **base cell size,**
|                     **int &track_count,**
|                     **int &head_count,**
|                     **int &sector_count)**

  Extract the geometry (heads, tracks, sectors) from a pc-ish floppy image by checking track 20.


| **get_track_data_mfm_pc(...)**
| **get_track_data_fm_pc(track number,**
|                      **head number,**
|                      **floppy image,**
|                      **base cell size,**
|                      **sector size,**
|                      **sector count,**
|                      **UINT8 \*sector data)**
|

  Extract what you'd get by reading in order 'sector size'-sized sectors from number 1 to sector count and put the result in sector data.


3.3. Floppy drive
~~~~~~~~~~~~~~~~~

  The class floppy_image_interface simulates the floppy drive.  That includes a number of control signals, reading, and writing.  Control signal changes must be synchronized, e.g. fired off a timer to ensure the current time is the same for all devices.

3.3.1. Control signals
''''''''''''''''''''''

  Due to the way they're usually connected to CPUs (e.g. directly on an I/O port), the control signals work with physical instead of logical values.  Which means than in general 0 means active, 1 means inactive. Some signals also have a callback associated called when they change.

**mon_w(state) / mon_r()**

  Motor on signal, rotates on 0.


**idx_r() / setup_index_pulse_cb(cb)**

  Index signal, goes 0 at start of track for about 2ms.  Callback is synchronized.  Only happens when a disk is in and the motor is running.


**ready_r() / setup_ready_cb(cb)**

  Ready signal, goes to 1 when the disk is removed or the motor stopped.  Goes to 0 after two index pulses.


**wpt_r() / setup_wpt_cb(cb)**

  Write protect signal (1 = readonly).  Callback is unsynchronized.


**dskchg_r()**

  Disk change signal, goes to 1 when a disk is change, goes to 0 on track change.


**dir_w(dir)**

  Selects track stepping direction (1 = out = decrease track number).


**stp_w(state)**

  Step signal, moves by one track on 1->0 transition.


**trk00_r()**

  Track 0 sensor, returns 0 when on track 0.


**ss_w(ss) / ss_r()**

  Side select


3.3.2. Read/write interface
'''''''''''''''''''''''''''

The read/write interface is designed to work asynchronously, e.g. somewhat independently of the current time.



.. [1] Cylinder is a hard-drive term somewhat improperly used for floppies.  It comes from the fact that hard-drives are similar to floppies but include a series of stacked disks with a read/write head on each.  The heads are physically linked and all point to the same circle on every disk at a given time, making the accessed area look like a cylinder.  Hence the name.
