#!/bin/bash

echo "enum alert_type {"
grep TORRENT_EXPORT ../../../include/libtorrent/alert_types.hpp | \
  sed 's/struct TORRENT_EXPORT //g' | \
  sed 's/: .*//g' | \
  sed 's/$/,/g' | \
  sed "s/^[ 	]*/  at_/g"
echo "  at_alert"
echo "};"
