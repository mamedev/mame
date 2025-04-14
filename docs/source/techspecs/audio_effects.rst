Audio effects
=============

.. contents:: :local:


1. Generalities
---------------

The audio effects are effects that are applied to the sound between
the speaker devices and the actual sound output.  In the current
implementation the effect chain is fixed (but not the effect
parameters), and the parameters are stored in the cfg files.  They are
honestly not really designed for extensibility yet, if ever.

Adding an effect requires working on four parts:

* audio_effects/aeffects.* for effect object creation and "publishing"
* audio_effects/youreffect.* for the effect implementation
* frontend/mame/ui/audioeffects.cpp to be able to instantiate the effect configuration menu
* frontend/mame/ui/audioyoureffect.* to implement the effect configuration menu

2. audio_effects/aeffects.*
---------------------------

The audio_effect class in the aeffect sources provides three things:

* an enum value to designate the effect type and which much match its
  position in the chain (iow, the effect chain follows the enum order),
  in the .h
* the effect name in the audio_effect::effect_names array in the .cpp
* the creation of a correct effect object in audio_effect::create in the .cpp



3. audio_effects/youreffect.*
-----------------------------

This is where you implement the effect.  It takes the shape of a
audio_effect_youreffect class which derives from audio_effect.

The methods to implement are:

.. code-block:: C++

    audio_effect_youreffect(u32 sample_rate, audio_effect *def);

    virtual int type() const override;
    virtual void config_load(util::xml::data_node const *ef_node) override;
    virtual void config_save(util::xml::data_node *ef_node) const override;
    virtual void default_changed() override;
    virtual u32 history_size() const; // optional

The constructor must pass the parameters to ``audio_effect`` and
initialize the effect parameters.  ``type`` must return the enum value
for the effect.  ``config_load`` and ``config_save`` should load or
save the effect parameters from/to the cfg file xml tree.
``default_changed`` is called when the parameters in ``m_default`` are
changed and the parameters may need to be updated.  ``history_size``
allows to tell how many samples should still be available of the
previous input frame.  Note that this number must not depend on the
parameters and only on the sample rate.

An effect have a number of parameters that can come from three sources:

* fixed default value
* equivalent effect object from the default effect chain
* user setting through the UI

The first two are recognized through the value of ``m_default`` which
gets the value of ``def`` in the constructor.  When it's nullptr, the
value to use when not set by the user is the fixed one, otherwise it's
the one in ``m_default``.

At a minimum an effect should have a parameter allowing to bypass it.

Managing a parameter uses four methods:

* ``type param() const;``  returns the current parameter value
* ``void set_param(type value);``  sets the current parameter value and marks it as set by the user
* ``bool isset_param() const;``  returns true is the parameter was set by the user
* ``void reset_param();``  resets the parameter to the default value (from m_default or fixed) and marks it as not set by the user

``config_save`` must only save the user-set parameters.
``config_load`` must retrieve the parameters that are present and mark
them as set by the user and reset all the others.

Finally the actual implementation goes into the ``apply`` method:

.. code-block:: C++

    virtual void apply(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest) override;

That method takes two buffers with the same number of channels and has
to apply the effect to ``src`` to produce ``dest``.  The
``output_buffer_flat`` is non-interleaved with independant per-channel
buffers.

To make bypassing easier, the ``copy(src, dest)`` method of
audio_effect allows to copy the samples from ``src`` to ``dest``
without changing them.

The effect application part should looks like:

.. code-block:: C++

    u32 samples = src.available_samples();
    dest.prepare_space(samples);
    u32 channels = src.channels();

    // generate channels * samples results and put them in dest

    dest.commit(samples);

To get pointers to the buffers, one uses:

.. code-block:: C++

    const sample_t *source = src.ptrs(channel, source_index); // source_index must be in [-history_size()..samples-1]
    sample_t *destination = dest.ptrw(channel, destination_index); // destination_index must be in [0..samples-1]

The samples pointed by source and destination are contiguous.  The
number of channels will not change from one apply call to another, the
number of samples will vary though.  Also the call happens in a
different thread than the main thread and also in a different thread
than the parameter setting calls are made from.




4. frontend/mame/ui/audioeffects.cpp
------------------------------------

There it suffices to add a creation of the menu
menu_audio_effect_youreffect in menu_audio_effects::handle.  The menu
effect will pick the effect names from audio_effect (in aeffect.*).


5. frontend/mame/ui/audioyoureffect.*
-------------------------------------

This is used to implement the configuration menu for the effect.  It's
a little complicated because it needs to generate the list of
parameters and their values, set left or right arrow flags depending
on the possible modifications, dim them (FLAG_INVERT) when not set by
the user, and manage the arrows and clear keys to change them.  Just
copy an existing one and change it as needed.
