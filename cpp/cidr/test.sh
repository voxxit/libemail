#!/bin/bash

PID=$$

THREADS=15

if [ ! -z $1 ]; then
	THREADS=$1
fi


echo "Executing:[t.pthread] against:[/tmp/xbl.ips] using:[15] threads" > log.$PID.txt
for i in `seq 15`
do
	echo "Starting: $(date)" >> log.$PID.txt
	/usr/bin/time --output=TIME.$PID ./t.pthread whitelist $THREADS < /tmp/xbl.ips 2>&1 >/dev/null
	cat TIME.$PID >> log.$PID.txt
	echo "" >> log.$PID.txt
done

rm TIME.$PID
mv log.$PID.txt log.${THREADS}t.txt
