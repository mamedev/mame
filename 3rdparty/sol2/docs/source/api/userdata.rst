userdata
========
reference to a userdata
-----------------------

.. code-block:: cpp
	:caption: (light\_)userdata reference

	class userdata : public reference;

	class light_userdata : public reference;

These type is meant to hold a reference to a (light) userdata from Lua and make it easy to push an existing userdata onto the stack. It is essentially identical to :doc:`reference<reference>` in every way, just with a definitive C++ type.
