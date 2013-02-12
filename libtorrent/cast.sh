#!/bin/bash

NAMES=$(grep TORRENT_EXPORT ../../../include/libtorrent/alert_types.hpp | \
  sed 's/struct TORRENT_EXPORT //g' | \
  sed 's/: .*//g' | \
  sed 's/$//g')

# drop tracker and torrent allerts
# since they are not complete, we can't cast them

NAMES=$(echo $NAMES | sed s/tracker_alert//)
NAMES=$(echo $NAMES | sed s/torrent_alert//)

for N in $NAMES; do
  echo "$N * cast_to_$N(void* pop);"
  echo "void free_$N($N * aa);"
done
