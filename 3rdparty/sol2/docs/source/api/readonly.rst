readonly
========
Routine to mark a member variable as read-only
----------------------------------------------

.. code-block:: cpp
	
	template <typename T>
	auto readonly( T&& value );

The goal of read-only is to protect a variable set on a usertype or a function. Simply wrap it around a member variable, e.g. ``sol::readonly( &my_class::my_member_variable )`` in the appropriate place to use it. If someone tries to set it, it will throw an error.
