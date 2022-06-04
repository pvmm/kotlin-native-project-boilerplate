#!/bin/sh

SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
DIRNAME=$(basename "$SCRIPTPATH")

echo $DIRNAME
