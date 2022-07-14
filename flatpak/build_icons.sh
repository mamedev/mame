#!/bin/bash

curl https://www.mamedev.org/MAMELogoR-trans.png -o logo.png
for i in 128; do
    convert logo.png -resize ${i}x${i} -gravity center -background none -extent ${i}x${i} mame${i}x${i}.png
done
