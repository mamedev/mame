## Flatpak

This is a method for distributing MAME using the Flatpak format. The main reason for this is on locked down systems that only allow sandboxed user applications.

## Security

The use of Flatpak here is not intended to secure MAME or provide any isolation from the rest of your system. MAME requires access to many of your systems resources including, but not limited to, devices for controllers, X11 sessions, filesystems writable for configuration and readable for data files, networking, and audio. The Flatpak provided sandbox must be opened up in many areas for MAME to function properly.

## Building

`build_icons.sh` will download the logo from the [https://www.mamedev.org](https://www.mamedev.org) site and resize it to various sizes to use as an icon.

`build_local.sh` will invoke the `flatpak-builder` application to compile MAME and generate a Flatpak package.

`org.mamedev.MAME.yaml` is the main Flatpak manifest that includes build and install information.

### Manifest

An explanation for some of the options included in the `org.mamedev.MAME.yaml' manifest file.

`org.kde.Platform` is used for its Qt support, required to build the MAME debugger.

`--device=all` added to allow various input devices, controllers, etc.

`--filesystem=home` writable location used for ini, cfg, diff, nvram, etc.

`--filesystem=host:ro` primarily to be used for external data, roms, samples, etc.

`post-install` section used to copy MAME built-in directories. Because MAME will create a default configuration file with relative paths, these directories are copied alongside the binary.

### Patches

Currently these are the patches applied:

`0.244-ptrdiff_t.patch` includes `<cstddef>` header to find `ptrdiff_t` type in the 0.244 release.

`ldopts.patch` enables an option for the linker to strip debug symbols.

`use-system-lib-flac.patch` works around FORTIFY_SOURCE compiler option that causes errors in the included libflac.
