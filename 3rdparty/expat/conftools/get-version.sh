#!/bin/sh
#
# USAGE: get-version.sh path/to/expat.h
#
# This script will print Expat's version number on stdout. For example:
#
#   $ ./conftools/get-version.sh ./lib/expat.h
#   1.95.3
#   $
#

if test $# = 0; then
  echo "ERROR: pathname for expat.h was not provided."
  echo ""
  echo "USAGE: $0 path/to/expat.h"
  exit 1
fi
if test $# != 1; then
  echo "ERROR: too many arguments were provided."
  echo ""
  echo "USAGE: $0 path/to/expat.h"
  exit 1
fi

hdr="$1"
if test ! -r "$hdr"; then
  echo "ERROR: '$hdr' does not exist, or is not readable."
  exit 1
fi

MAJOR_VERSION=$(sed -n -e '/MAJOR_VERSION/s/[^0-9]*//gp' "$hdr")
MINOR_VERSION=$(sed -n -e '/MINOR_VERSION/s/[^0-9]*//gp' "$hdr")
MICRO_VERSION=$(sed -n -e '/MICRO_VERSION/s/[^0-9]*//gp' "$hdr")

printf '%s.%s.%s' "$MAJOR_VERSION" "$MINOR_VERSION" "$MICRO_VERSION"
