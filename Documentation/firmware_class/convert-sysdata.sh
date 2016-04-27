#!/bin/bash

set -e

if [[ $# -ne 1 ]]; then
	echo Usage: $0 ./path/to-driver/
	exit
fi

for i in Documentation/firmware_class/*.cocci; do
	export COCCI=$i
	make coccicheck MODE=patch M=$1
done
