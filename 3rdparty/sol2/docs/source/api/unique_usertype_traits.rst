unique_usertype_traits<T>
=========================
A trait for hooking special handles / pointers
----------------------------------------------

.. code-block:: cpp
	:caption: unique_usertype
	:name: unique-usertype

	template <typename T>
	struct unique_usertype_traits {
		typedef T type;
		typedef T actual_type;
		static const bool value = false;

		static bool is_null(const actual_type&) {...}

		static type* get (const actual_type&) {...}
	};

This is a customization point for users who need to *work with special kinds of pointers/handles*. For generic customization, please review the :doc:`customization tutorial<../tutorial/customization>` A traits type for alerting the library that a certain type is to be pushed as a special userdata with special deletion / destruction semantics. It is already defined for ``std::unique_ptr<T, D>`` and ``std::shared_ptr<T>``. You can specialize this to get ``unique_usertype_traits`` semantics with your code, for example with ``boost::shared_ptr<T>`` like so:

.. code-block:: cpp
	
	namespace sol {
		template <typename T>
		struct unique_usertype_traits<boost::shared_ptr<T>> {
			typedef T type;
			typedef boost::shared_ptr<T> actual_type;
			static const bool value = true;
    
			static bool is_null(const actual_type& value) {
				return value == nullptr;
			}

			static type* get (const actual_type& p) {
				return p.get();
			}
		}
	}

This will allow the framework to properly handle ``boost::shared_ptr<T>``, with ref-counting and all. The `type` is the  type that lua and sol will interact with, and will allow you to pull out a non-owning reference / pointer to the data when you just ask for a plain `T*` or `T&` or `T` using the getter functions and properties of Sol.

Note that if ``is_null`` triggers, a ``nil`` value will be pushed into Sol.