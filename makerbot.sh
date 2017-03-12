#!/bin/bash

die() {
	kill $(jobs -pr)
	exit 0
}
trap 'die' SIGINT SIGTERM EXIT


misc/spintest 127.0.0.1 6503 script.spindle &

while [ 1 ]; do
	make
	ret=$?
	[ $ret ] && sleep 1.0s
	sleep 0.1s
done
