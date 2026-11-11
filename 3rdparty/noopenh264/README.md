noopenh264
==========

Fake implementation of the OpenH264 library we can link from
regardless of the actual library being available. Tags
with `v*` prefix correspond to OpenH264 release.

Licensing
=========

This project is based on the original repository from Cisco at
https://github.com/cisco/openh264, and as such it is licensed
according to Cisco Systems' own license at [1], with the sole exception
of the dummy implementation provided for the entry points defined in
`codec/api/svc/codec_api.h`, which are licensed LGPL-2.1-or-later.

See `COPYING.*` for more details.

Updating
========

When there is a new tag in https://github.com/cisco/openh264, start with
updating version in the toplevel meson.build to match the openh264 version. Also
update `major_version` if there is a soname version bump. Resync the headers in
the codec/api/wels directory.

Bugs
====

Bug reports and patches should be filed in GitLab.

Contact
=======

https://codeberg.org/distro-openh264/noopenh264
