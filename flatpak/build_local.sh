#!/bin/bash

flatpak-builder --user --install --force-clean --ccache --jobs=8 build org.mamedev.MAME.yaml
flatpak install --user --reinstall --assumeyes "$(pwd)/.flatpak-builder/cache" org.mamedev.MAME.Debug
