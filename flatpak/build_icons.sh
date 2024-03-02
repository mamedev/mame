#!/bin/bash

for i in 16 24 32 48; do
    convert -thumbnail ${i}x${i} -alpha on -background none -flatten ../scripts/resources/windows/mame/mame.ico mame${i}x${i}.png
done

for i in 64 128 256; do
    convert -resize ${i}x${i} -gravity center -background none -extent ${i}x${i} ../docs/source/images/MAMElogo.svg mame${i}x${i}.png
done
