error
=====
the single error/exception type
-------------------------------

.. code-block:: cpp

	class error : public std::runtime_error {
	public:
		error(const std::string& str): std::runtime_error("Lua: error: " + str) {}
	};

If an eror is thrown by Sol, it is going to be of this type. We use this in a single place: the default ``at_panic`` function we bind on construction of a :ref:`sol::state<set-panic>`. If you turn :doc:`off exceptions<../exceptions>`, the chances of you seeing this error are nil unless you specifically use it to pull errors out of things such as :doc:`sol::protected_function<protected_function>`.

As it derives from ``std::runtime_error``, which derives from ``std::exception``, you can catch it with a ``catch (const std::exception& )`` clause in your try/catch blocks. You can retrieve a string error from Lua (Lua pushes all its errors as string returns) by using this type with any of the get or lookup functions in Sol.