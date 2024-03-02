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

`--persist=.mame` writable location used for cfg, diff, nvram, etc.

`--filesystem=host:ro` primarily to be used for external data, roms, samples, etc.

### Configuring

The default ini paths include the home `~/.mame` directory and two relative paths '.' and 'ini'.  The relative paths are not helpful inside the sandbox, and the `~/.mame` contents shouldn't be overwritten with each new MAME release.  Instead the default ini search path is patched to `$HOME/.mame;/app/share/mame/ini`.  This will allow a default base ini in `/app/share/mame/ini/mame.ini` and can be overridden in `~/.mame/mame.ini`.
