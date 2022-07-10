#!/usr/bin/env sh
set -e
node ./tablegen-arm.js $@
node ./tablegen-x86.js $@
