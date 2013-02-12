#!/bin/bash

echo "TORRENT_EXPORT int alert_get_type(void* v) {"
echo "  libtorrent::alert * a = (libtorrent::alert*) v;"
NAMES=$(grep TORRENT_EXPORT ../../../include/libtorrent/alert_types.hpp | \
  sed 's/struct TORRENT_EXPORT //g' | \
  sed 's/: .*//g' | \
  sed 's/$//g')

# drop tracker and torrent allerts
# since they are not complete, we can't cast them

NAMES=$(echo $NAMES | sed s/tracker_alert//)
NAMES=$(echo $NAMES | sed s/torrent_alert//)

for N in $NAMES; do
  echo "  if (libtorrent::alert_cast < libtorrent::$N > (a))"
  echo "    return at_$N;"
done

echo "  return at_alert;"
echo "}"
