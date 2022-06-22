#!/bin/bash

flatpak-builder --user --install --force-clean --jobs=8 build org.mamedev.MAME.yaml
