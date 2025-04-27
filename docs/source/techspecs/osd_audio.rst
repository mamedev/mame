OSD audio support
=================

Introduction
------------

The audio support in Mame tries to allow the user to freely map
between the emulated system audio outputs (called speakers) and the
host system audio.  A part of it is the OSD support, where a
host-specific module ensures the interface between Mame and the host.
This is the documentation for that module.

Note: this is currenty output-only, but input should follow.


Capabitilies
------------

The OSD interface is designed to allow three levels of support,
depending on what the API allows and the amount of effort to expend.
Those are:

* Level 1: One or more audio targets, only one stream allowed per target (aka exclusive mode)
* Level 2: One or more audio targets, multiple streams per target
* Level 3: One or more audio targets, multiple streams per target, user-visible per-stream-channel volume control

In any case we support having the user use an external interface to
change the target of a stream and, in level 3, change the volumes.  By
support we mean storing the information in the per-game configuration
and keeping in the internal UI in sync.


Terminology
-----------

For this module, we use the terms:

* node: some object we can send audio to.  Can be physical, like speakers, or virtual, like an effect system.  It should have a unique, user-presentable name for the UI.
* port: a channel of a node, has a name (non-unique, like "front left") and a 3D position
* stream: a connection to a node with allows to send audio to it


Reference documentation
-----------------------

Adding a module
~~~~~~~~~~~~~~~

Adding a module is done by adding a cpp file to src/osd/modules/sound
which follows this structure,

.. code-block:: C++

    // License/copyright
    #include "sound_module.h"
    #include "modules/osdmodules.h"

    #ifdef MODULE_SUPPORT_KEY

    #include "modules/lib/osdobj_common.h"

    // [...]
    namespace osd {
    namespace {

    class sound_module_class : public osd_module, public sound_module
    {
       sound_module_class() : osd_module(OSD_SOUND_PROVIDER, "module_name"),
                              sound_module()
       // ...
    };

    }
    }
    #else
    namespace osd { namespace {
      MODULE_NOT_SUPPORTED(sound_module_class, OSD_SOUND_PROVIDER, "module_name")
    }}
    #endif

    MODULE_DEFINITION(SOUND_MODULE_KEY, osd::sound_module_class)

In that code, four names must be chosen:

* MODULE_SUPPORT_KEY some #define coming from the genie scripts to tell that this particular module can be compiled (like NO_USE_PIPEWIRE or SDLMAME_MACOSX)
* sound_module_class is the name of the class which makes up the module (like sound_coreaudio)
* module_name is the name to be used in -sound <xxx> to select that particular module (like coreaudio)
* SOUND_MODULE_KEY is a symbol that represents the module internally (like SOUND_COREAUDIO)

The file path needs to be added to scripts/src/osd/modules.lua in
osdmodulesbuild() and the module reference to
src/osd/modules/lib/osdobj_common.cpp in
osd_common_t::register_options with the line:

.. code-block:: C++

    REGISTER_MODULE(m_mod_man, SOUND_MODULE_KEY);

This should ensure that the module is reachable through -sound <xxx>
on the appropriate hosts.


Interface
~~~~~~~~~

The full interface is:

.. code-block:: C++

    virtual bool split_streams_per_source() const override;
    virtual bool external_per_channel_volume() const override;

    virtual int init(osd_interface &osd, osd_options const &options) override;
    virtual void exit() override;

    virtual uint32_t get_generation() override;
    virtual osd::audio_info get_information() override;
    virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) override;
    virtual uint32_t stream_source_open(uint32_t node, std::string name, uint32_t rate) override;
    virtual void stream_set_volumes(uint32_t id, const std::vector<float> &db) override;
    virtual void stream_close(uint32_t id) override;
    virtual void stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame) override;
    virtual void stream_source_update(uint32_t id, int16_t *buffer, int samples_this_frame) override;


The class sound_module provides default for minimum capabilities: one
stereo target and stream at default sample rate.  To support that,
only *init*, *exit* and *stream_update* need to be implemented.
*init* is called at startup and *exit* when quitting and can do
whatever they need to do.  *stream_sink_update* will be called on a
regular basis with a buffer of sample_this_frame*2*int16_t with the
audio to play.  From this point in the documentation we'll assume more
than a single stereo channel is wanted.


Capabilities
~~~~~~~~~~~~

Two methods are used by the module to indicate the level of capability
of the module:

* split_streams_per_source() should return true when having multiple streams for one target is expected (e.g. Level 2 or 3)
* external_per_channel_volume() should return true when the streams have per-channel volume control that can be externally controlled (e.g. Level 3)


Hardware information and generations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The core runs on the assumption that the host hardware capabilities
can change at any time (bluetooth devices coming and going, usb
hot-plugging...) and that the module has some way to keep tabs on what
is happening, possibly using multi-threading.  To keep it
lightweight-ish, we use the concept of a *generation* which is a
32-bits number that is incremented by the module every time something
changes.  The core checks the current generation value at least once
every update (once per frame, usually) and if it changed asks for the
new state and detects and handles the differences.  *generation*
should be "eventually stable", e.g. it eventually stops changing when
the user stops changing things all the time.  A systematic increment
every frame would be a bad idea.

.. code-block:: C++

    virtual uint32_t get_generation() override;

That method returns the current generation number.  It's called at a
minimum once per update, which usually means per frame.  It whould be
reasonably lightweight when nothing special happens.

.. code-block: C++

    virtual osd::audio_info get_information() override;

    struct audio_rate_range {
        uint32_t m_default_rate;
        uint32_t m_min_rate;
        uint32_t m_max_rate;
    };

    struct audio_info {
        struct node_info {
                std::string m_name;
                uint32_t m_id;
                audio_rate_range m_rate;
		std::vector<std::string> m_port_names;
		std::vector<std::array<double, 3>> m_port_positions;
		uint32_t m_sinks;
		uint32_t m_sources;
        };

        struct stream_info {
                uint32_t m_id;
                uint32_t m_node;
                std::vector<float> m_volumes;
        };

        uint32_t m_generation;
        uint32_t m_default_sink;
        uint32_t m_default_source;
        std::vector<node_info> m_nodes;
        std::vector<stream_info> m_streams;
    };

This method must provide all the information about the current state
of the host and the module.  This state is:

* m_generation:  The current generation number
* m_nodes: The vector available nodes (*node_info*)

  * m_name: The name of the node
  * m_id: The numeric ID of the node
  * m_rate: The minimum, maximum and preferred sample rate for the node
  * m_port_names: The vector of port names
  * m_port_positions: The vector of 3D position of the ports.  Refer to src/emu/speaker.h for the "standard" positions
  * m_sinks: Number of sinks (inputs)
  * m_sources: Number of sources (outputs)

* m_default_sink: ID of the node that is the current "system default" for audio output, 0 if there's no such concept
* m_default_source: same for audio input (currently unused)
* m_streams: The vector of active streams (*stream_info*)

  * m_id: The numeric ID of the stream
  * m_node: The target node of the stream
  * m_volumes: empty if *external_per_channel_volume* is false, current volume value per-channel otherwise

IDs, for nodes and streams, are (independant) 32-bit unsigned non-zero
values associated to respectively nodes and streams.  IDs should not
be reused.  A node that goes away then comes back should get a new ID.
A stream closing does not allow reuse of its ID.

If a node has both sources and sinks, the sources are *monitors* of
the sinks, e.g. they're loopbacks.  They should have the same count in
such a case.

When external control exists, a module should change the value of
*stream_info::m_node* when the user changes it, and same for
*stream_info::m_volumes*.  Generation number should be incremented
when this happens, so that the core knows to look for changes.

Volumes are floats in dB, where 0 means 100% and -96 means no sound.
audio.h provides osd::db_to_linear and osd::linear_to_db if such a
conversion is needed.

There is an inherent race condition with this system, because things
can change at any point after returning for the method.  The idea is
that the information returned must be internally consistent (a stream
should not point to a node ID that does not exist in the structure,
same for default sink) and that any external change from that state
should increment the generation number, but that's it.  Through the
generation system the core will eventually be in sync with reality.


Input and output streams
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) override;
    virtual uint32_t stream_source_open(uint32_t node, std::string name, uint32_t rate) override;
    virtual void stream_set_volumes(uint32_t id, const std::vector<float> &db) override;
    virtual void stream_close(uint32_t id) override;
    virtual void stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame) override;
    virtual void stream_source_update(uint32_t id, int16_t *buffer, int samples_this_frame) override;

Streams are the concept used to send or recieve audio from/to the host
audio system.  A stream is first opened through *stream_sink_open* for
speakers and *stream_source_open* for microphones and targets a
specific node at a specific sample rate.  It is given a name for use
by the host sound services for user UI purposes (currently the game
name if split_streams_per_source is false, the
speaker_device/microphone_device tag if true).  The returned ID must
be a non-zero, never-used-before for streams value in case of success.
Failures, like when the node went away between the get_information
call and the open one, should be silent and return zero.

*stream_set_volumes* is used only when *external_per_channel_volume*
is true and is used by the core to set the per-channel volume.  The
call should just be ignored if the stream ID does not exist (or is
zero).  Do not try to apply volumes in the module if the host API
doesn't provide for it, let the core handle it.

*stream_close* closes a stream, The call should just be ignored if the
stream ID does not exist (or is zero).

Opening a stream, closing a stream or changing the volume does not
need to touch the generation number.

*stream_sink_update* is the method used to send data to the node
through a given stream.  It provides a buffer of *samples_this_frame*
* *node channel count* channel-interleaved int16_t values.  The
lifetime of the data in the buffer or the buffer pointer itself is
undefined after return from the method call.  The call should just be
ignored if the stream ID does not exist (or is zero).

*stream_source_update* is the equivalent to retrieve data from a node,
writing to the buffer instead of reading from it.  The constraints are
identical.

When a stream goes away because the target node is lost it should just
be removed from the information, and the core will pick up the node
departure and close the stream.

Given the assumed raceness of the interface, all the methods should be
tolerant of obsolete or zero IDs being used by the core, and that is
why ID reuse must be avoided.  Also the update methods and the
open/close/volume ones may be called at the same time in different
threads.


Helper class *abuffer*
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    class abuffer {
    public:
        abuffer(uint32_t channels);
        void get(int16_t *data, uint32_t samples);
        void push(const int16_t *data, uint32_t samples);
        uint32_t channels() const;
    };

The class *abuffer* is a helper provided by *sound_module* to buffer
audio in output or output.  It automatically drops data when there is
an overflow and duplicates the last sample on underflow.  It must
first be initialized with the number of channels, which can be
retrieved with *channels()* if needed.  *push* sends
*samples* * *channels* 16-bits samples in the buffer.  *get* retrieves
*samples* * *channels* 16-bits samples from the buffer, on a fifo basis.

It is not protected against multithreading, but uses no class
variables.  So just don't read and write from one specific abuffer
instance at the same time.  The system sound interface mandated
locking should be enough to ensure that.
