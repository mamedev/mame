#!/bin/bash

flatpak-builder --user --install --force-clean --ccache --jobs=8 build org.mamedev.MAME.yaml
