customization traits
====================

These are customization points within the library to help you make sol2 work for the types in your framework and types. 

To learn more about various customizable traits, visit:

* :ref:`containers detection trait<container-detection>`
	- This is how to work with containers when you have an compiler error when serializing a type that has ``begin`` and ``end`` functions but isn't exactly a container.
* :doc:`unique usertype (custom pointer) traits<api/unique_usertype_traits>`
	- This is how to deal with unique usertypes, e.g. ``boost::shared_ptr``, reference-counted pointers, etc.
	- Useful for custom pointers from all sorts of frameworks or handle types that employ very specific kinds of destruction semantics and access.
* :doc:`customization point tutorial<tutorial/customization>`
	- This is how to customize a type to work with sol2.
	- Can be used for specializations to push strings and other class types that are not natively ``std::string`` or ``const char*``.
