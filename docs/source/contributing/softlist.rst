.. _contributing-softlist:

Guidelines for Software Lists
=============================

.. contents:: :local:


.. _contributing-softlist-intro:

Introduction
------------

MAME’s software lists describe known software media for emulated
systems in a form that can be used to identify media image files for
known software, verify media image file integrity, and load media
image files for emulation.  Software lists are implemented as XML
files in the ``hash`` folder.  The XML structure is described in the
file ``hash/softwarelist.dtd``.

Philosophically, software list items should represent the original
media, rather than a specific dump of the media.  Ideally, it should be
possible for anyone with the media to dump it and produce the same image
file.  Of course, this isn’t always possible in practice – in particular
it’s problematic for inherently analog media, like home computer
software stored on audio tape cassettes.

MAME strives to document the best available media images.  It is not our
intention to propagate corrupted, truncated, defaced, watermarked, or
otherwise bad media images.  Where possible, file structures matching
the original media structure are preferred.  For example we prefer
individual files for separate ROM chips in cartridge media, and we use
disk images rather than archives of files extracted from the original
disks.


.. _contributing-softlist-itempart:

Items and parts
---------------

A software list is a collection of *items* and each item may have
multiple *parts*.  An item represents a piece of software, as
distributed as a complete package.  A part represents a single piece of
media within the package.  Parts can be mounted individually in
emulated media devices.  For example a piece of software distributed on
three floppy disks will be a single item, while each floppy disk will be
one part within that item.

Sometimes, logically separate parts of a single physical piece of media
are represented as separate parts within a software item.  For example
each side of an audio tape cassette is represented as a separate part.
However individual ROM chips within a cartridge may be separate files,
but they are *not* separate parts, as the cartridge is mounted as a
whole.

Each item is a ``software`` element.  The ``software`` element may have
the following attributes:

name (required)
    The short name identifying the item.  This is used for file names,
    command line arguments, database keys, URL fragments, and many other
    purposes.  It should be terse but still recognisable.  It must be
    unique within the software list.  Valid characters are lowercase
    English letters, decimal digits and underscores.  The maximum
    allowed length is sixteen characters.
cloneof (optional)
    The short name of the parent item if the item is a clone.  The
    parent must be within the same software list – parent/clone
    relationships spanning multiple software lists are not supported.
supported (optional)
    One of the values ``yes`` (fully usable in emulation), ``no`` (not
    usable in emulation), or ``partial`` (usable in emulation with
    limitations).  If the attribute is not present, it is equivalent to
    ``yes``.  Examples of partially supported software include games
    that are playable with graphical glitches, and office software where
    some but not all functionality works.

Each part is a ``part`` element within the ``software`` element.  The
``part`` element must have the following attributes:

name (required)
    The short name identifying the part.  This is used for command line
    arguments, database keys, URL fragments, and many other purposes.
    It must be unique within the item.  It is also used as the display
    name if a separate display name is not provided.  Valid characters
    are lowercase English letters, decimal digits and underscores.  The
    maximum allowed length is sixteen characters.
interface (required)
    This attribute is used to identify suitable emulated media devices
    for mounting the software part.  Applicable values depend on the
    emulated system.


.. _contributing-softlist-metadata:

Metadata
--------

Software lists support various kinds of metadata.  All software list
items require the following metadata elements to be present:

description
    This is the primary display name for the software item.  It should
    be the original name of the software, transliterated into English
    Latin script if necessary.  It must be unique within the software
    list.  If extra text besides the title itself is required for
    disambiguation, use lowercase outside of proper nouns, initialisms
    and verbatim quotes.
year
    The year of release or copyright year for the software.  If unknown,
    use an estimate with a question mark.  Items can be filtered by year
    in the software selection menu.
publisher
    The publisher of the software.  This may be the same as the
    developer if the software was self-published.  Items can be filtered
    by published in the software selection menu.

Most user-visible software item metadata is provided using ``info``
elements.  Each ``info`` element must have a ``name`` attribute and a
``value`` attribute.  The ``name`` attribute identifies the type of
metadata, and the ``value`` attribute is the metadata value itself.
Note that ``name`` attributes do not need to be unique within an item.
Multiple ``info`` elements with the same ``name`` may be present if
appropriate.  This is frequently seen for software sold using different
titles in different regions.

Prefer multiple ``info`` elements with the same ``name`` attribute over
combining multiple values into a single element.  For example if a piece
of software supports multiple user interface languages, use multiple
``info`` elements with ``name="language"`` attributes.  This makes
filtering and database queries more practical.

MAME displays metadata from ``info`` elements in the software selection
menu.  The following ``name`` attributes are recognised specifically,
and can show localised names:

alt_title
    Used for alternate titles.  Examples are different tiles used in
    different languages, scripts or regions, or different titles used
    on the title screen and packaging.  MAME searches alternate titles
    as well as the description.
author
    Author of the software.  Items can be filtered by author in the
    software selection menu.
barcode
    Barcode number identifying the software package (typically an EAN).
developer
    Developer responsible for implementing the software.  Items can be
    filtered by developer in the software selection menu.
distributor
    Party responsible for distributing the software to retailers (or
    customers in the case of direct sales).  Items can be filtered by
    distributor in the software selection menu.
install
    Installation instructions.
isbn
    ISBN for software included with a commercial book.
language
    User interface language supported by the software.
oem
    Original equipment manufacturer, typically used with customised
    versions of software distributed by a hardware vendor.
original_publisher
    The original publisher, for items representing software re-released
    by a different publisher.
partno
    Distributor’s part number for the software.
pcb
    Printed circuit board identifier, typically for cartridge media.
programmer
    Programmer who wrote the code for the software.
release
    Fine-grained release date for the software, if known.  Use
    YYYYMMDD format with no punctuation.  If only the month is known,
    use “xx” for the day digits.  For example ``199103xx`` or
    ``19940729``.
serial
    Number identifying the software within a series of releases.
usage
    Usage instructions.
version
    Version number of the software.
