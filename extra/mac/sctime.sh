#!/bin/sh

if [ -f "$HOME/.sctime/zeitkonten" ] ; then
	exec -a $0 $0.bin --zeitkontenfile=$HOME/.sctime/zeitkonten
fi

exec -a $0 $0.bin
