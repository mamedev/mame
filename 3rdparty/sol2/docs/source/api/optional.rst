optional<T>
===========

This is an implemention of `optional from the standard library`_. If it detects that a proper optional exists, it will attempt to use it. This is mostly an implementation detail, used in the :ref:`sol::stack::check_get<stack-check-get>` and :ref:`sol::stack::get\<optional\<T>><stack-get>` and ``optional<T> maybe_value = table["arf"];`` implementations for additional safety reasons.

.. _optional from the standard library: http://en.cppreference.com/w/cpp/utility/optional