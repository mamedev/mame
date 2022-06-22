#!/bin/bash

curl https://www.mamedev.org/MAMELogoR-trans.png -o logo.png
for i in 32 48 64 128 256; do
    convert logo.png -resize ${i}x${i} mame${i}x${i}.png
done
