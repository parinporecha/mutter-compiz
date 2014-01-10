#!/bin/bash

#: ${1?"Usage: $0 path_to_mutter_source"}

if test $# -eq 0; then
    echo "Usage: $0 path_to_mutter_source"
    exit 0
fi

mutter_path=$1

core=(boxes.c
      boxes-private.h
      edid.h
      edid-parse.c
      meta-idle-monitor.c
      meta-idle-monitor-private.h
      meta-xrandr-shared.h
      monitor.c
      monitor-config.c
      monitor-private.h
      monitor-xrandr.c	
)
meta=(boxes.h
      common.h
      meta-idle-monitor.h)

xml=(idle-monitor.xml  xrandr.xml)

echo "Copying..."

for file in ${core[@]}
do
   cp $1/src/core/$file src/
done

for file in ${meta[@]}
do
   cp $1/src/meta/$file src/
done

for file in ${xml[@]}
do
   cp $1/src/$file src/
done

echo "Done"
