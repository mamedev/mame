The device_sound_interface
==========================

.. contents:: :local:


1. The sound system
-------------------

The device sound interface is the entry point for devices that handle
sound input and/or output.  The sound system is built on the concept
of *streams* which connect devices together with resampling and mixing
applied transparently as needed.  Microphones (audio input) and
speakers (audio output) are specific known devices which use the same
interface.

2. Devices using device_sound_interface
---------------------------------------

2.1 Initialisation
~~~~~~~~~~~~~~~~~~

Sound streams must be created in the device_start (or
interface_pre_start) method.

.. code-block:: C++

    sound_stream *stream_alloc(int inputs, int outputs, int sample_rate, sound_stream_flags flags = STREAM_DEFAULT_FLAGS);

A stream is created with ``stream_alloc``.  It takes the number of
input and output channels, the sample rate and optionally flags.

The sample rate can be SAMPLE_RATE_INPUT_ADAPTIVE,
SAMPLE_RATE_OUTPUT_ADAPTIVE or SAMPLE_RATE_ADAPTIVE.  In that case the
chosen sample rate is the highest one amongs the inputs, outputs or
both respectively.  In case of loop, the chosen sample rate is the
configured global sample rate.

The only available non-default flag is STREAM_SYNCHRONOUS.  When set,
the sound generation method will be called for every sample
individually.  It's necessary for dsps that run a program on every
sample. but on the other hand it's expensive, so only to be used when
required.

Devices can create multiple streams.  It's rare though.  Some yamaha
chips should but don't.  Inputs and outputs are numbered from 0 and
collate all streams in the order they are created.


2.2 Sound input/output
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    virtual void sound_stream_update(sound_stream &stream);

This method is required to be implemented to consume the input samples
and/or compute the output ones.  The stream to update for is passed as
the parameter.  See the streams section, specifically sample access,
to see how to write the method.


2.3 Stream information
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    int inputs() const;
    int outputs() const;
    std::pair<sound_stream *, int> input_to_stream_input(int inputnum) const;
    std::pair<sound_stream *, int> output_to_stream_output(int outputnum) const;

The method ``inputs`` returns the total number of inputs in the
streams created by the device.  The method ``outputs`` similarly
counts the outputs.  The other two methods allow to grab the stream
and channel number for the device corresponding to the global input or
output number.


2.4 Gain management
~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    float input_gain(int inputnum) const;
    float output_gain(int outputnum) const;
    void set_input_gain(int inputnum, float gain);
    void set_output_gain(int outputnum, float gain);
    void set_route_gain(int source_channel, device_sound_interface *target, int target_channel, float gain);

    float user_output_gain() const;
    float user_output_gain(int outputnum) const;
    void set_user_output_gain(float gain);
    void set_user_output_gain(int outputnum, float gain);

Those methods allow to set the gain on every step of the routes
between streams.  All gains are multipliers, with default value 1.0.
The steps are, from samples output in ``sound_stream_update`` to
samples read in the next device's ``sound_stream_update``:

* Per-channel output gain
* Per-channel user output gain
* Per-device user output gain
* Per-route gain
* Per-channel input gain

The user gains must not be set from the driver, they're for use by the
user interface (the sliders) and are saved in the game configuration.
The other gains are for driver/device use, and are saved in save
states.


2.5 Routing setup
~~~~~~~~~~~~~~~~~

.. code-block:: C++

    device_sound_interface &add_route(u32 output, const device_finder<T, R> &target, double gain, u32 channel = 0)
    device_sound_interface &add_route(u32 output, const char *target, double gain, u32 channel = 0);
    device_sound_interface &add_route(u32 output, device_sound_interface &target, double gain, u32 channel = 0);

    device_sound_interface &reset_routes();

Routes between devices, e.g. between streams, are set at configuration
time.  The method ``add_route`` must be called on the source device
and gives the channel on the source device, the target device, the
gain, and optionally the channel on the target device.  The constant
``ALL_OUTPUTS`` can be used to add a route from every channel of the
source to a given channel of the destination.

The method ``reset_routes`` is used to remove all the routes setup on
a given source device.


.. code-block:: C++

    u32 get_sound_requested_inputs() const;
    u32 get_sound_requested_outputs() const;
    u64 get_sound_requested_inputs_mask() const;
    u64 get_sound_requested_outputs_mask() const;

Those methods are useful for devices which want to behave differently
depending on what routes are setup on them.  You get either the max
number of requested channel plus one (which is the number of channels
when all channels are routed, but is more useful when there are gaps)
or a mask of use for channels 0-63.  Note that ``ALL_OUTPUTS`` does
not register any specific output or output count.



3. Streams
----------

3.1 Generalities
~~~~~~~~~~~~~~~~

Streams are endpoints associated with devices and, when connected
together, ensure the transmission of audio data between them.  A
stream has a number of inputs (which can be zero) and outputs (same)
and one sample rate which is common to all inputs and outputs.  The
connections are setup at the machine configuration level and the sound
system ensures mixing and resampling is done transparently.

Samples in streams are encoded as sample_t.  In the current
implementation, this is a float.  Nominal values are between -1 and 1,
but clamping at the device level is not recommended (unless that's
what happens in hardware of course) because the gain values, volume
and effects can easily avoid saturation.

They are implemented in the class ``sound_stream``.


3.2 Characteristics
~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    device_t &device() const;
    bool input_adaptive() const;
    bool output_adaptive() const;
    bool synchronous() const;
    u32 input_count() const;
    u32 output_count() const;
    u32 sample_rate() const;
    attotime sample_period() const;


3.3 Sample access
~~~~~~~~~~~~~~~~~

.. code-block:: C++

    s32 samples() const;

    void put(s32 output, s32 index, sample_t sample);
    void put_clamp(s32 output, s32 index, sample_t sample, sample_t clamp = 1.0);
    void put_int(s32 output, s32 index, s32 sample, s32 max);
    void put_int_clamp(s32 output, s32 index, s32 sample, s32 maxclamp);
    void add(s32 output, s32 index, sample_t sample);
    void add_int(s32 output, s32 index, s32 sample, s32 max);
    void fill(s32 output, sample_t value, s32 start, s32 count);
    void fill(s32 output, sample_t value, s32 start);
    void fill(s32 output, sample_t value);
    void copy(s32 output, s32 input, s32 start, s32 count);
    void copy(s32 output, s32 input, s32 start);
    void copy(s32 output, s32 input);
    sample_t get(s32 input, s32 index) const;
    sample_t get_output(s32 output, s32 index) const;

Those are the methods used to implement ``sound_stream_update``.
First ``samples`` tells how many samples to consume and/or generate.
The to-generate samples, if any, are pre-cleared (e.g. set to zero).

Input samples are retrieved with ``get``, where ``input`` is the
stream channel number and ``index`` the sample number.

Generated samples are written with the put variants.  ``put`` sets a
sample_t in channel ``output`` at position ``index``.  ``put_clamp``
does the same but first clamps the value to +/-``clamp``.  ``put_int``
does it with an integer ``sample`` but pre-divides it by ``max``.
``put_int_clamp`` does the same but also pre-clamps within
-``maxclamp`` and ``maxclamp``-1, which is the normal range for a
2-complement value.

``add`` and ``add_int`` are similar but add the value of the sample to
what's there instead of replacing.  ``get_output`` gets the currently
stored output value.

``fill`` sets a range of the an output channel to a given ``value``.
``start`` tells where to start (default index 0), ``count`` how many
(default up to the end of the buffer).

``copy`` does the same as fill but gets its value from the indentical
position in an input channel.

Note that clamping should not be used unless it actually happens in
hardware.  Between gains and effects there is a fair chance saturation
can be avoided later in the chain.



3.4 Gain management
~~~~~~~~~~~~~~~~~~~

.. code-block:: C++

    float user_output_gain() const;
    void set_user_output_gain(float gain);
    float user_output_gain(s32 output) const;
    void set_user_output_gain(s32 output, float gain);

    float input_gain(s32 input) const;
    void set_input_gain(s32 input, float gain);
    void apply_input_gain(s32 input, float gain);
    float output_gain(s32 output) const;
    void set_output_gain(s32 output, float gain);
    void apply_output_gain(s32 output, float gain);


This is similar to the device gain control, with a twist: apply
multiplies the current gain by the given value.


3.5 Misc. actions
~~~~~~~~~~~~~~~~~

.. code-block:: C++

    void set_sample_rate(u32 sample_rate);
    void update();

The method ``set_sample_rate`` allows to change the sample rate of the
stream.  The method ``update`` triggers a call of
``sound_stream_update`` on the stream and the ones it depends on to
reach the current time in terms of samples.


4. Devices using device_mixer_interface
---------------------------------------

The device mixer interface is used for devices that want to relay
sound in the device tree without acting on it.  It's very useful on
for instance slot devices connectors, where the slot device may have
an audio connection with the main system.  They are routed like every
other sound device, create the streams automatically and copy input to
output.  Nothing needs to be done in the device.
