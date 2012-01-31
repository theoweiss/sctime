#!/bin/sh

[ -n "$SCTIME_DIR" ] || SCTIME_DIR="$HOME"/.sctime
[ -n "$SCTIME_ZEITKONTENFILE" ] || SCTIME_ZEITKONTENFILE="$SCTIME_DIR"/zeitkonten
[ -n "$SCTIME_BEREITSCHAFTSFILE" ] || SCTIME_BEREITSCHAFTSFILE="$SCTIME_DIR"/bereitschaft

args=""
[ -f "$SCTIME_ZEITKONTENFILE" ] && args="$args --zeitkontenfile=\"\$SCTIME_ZEITKONTENFILE\""
[ -f "$SCTIME_BEREITSCHAFTSFILE" ] && args="$args --bereitschaftsfile=\"\$SCTIME_BEREITSCHAFTSFILE\""

eval exec -a $0 $0.bin $args
