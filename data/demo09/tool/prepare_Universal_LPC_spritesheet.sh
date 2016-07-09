#!/bin/bash

#rm -Rf Universal-LPC-spritesheet-master
if [ ! -f master.zip ];then
	wget https://github.com/jrconway3/Universal-LPC-spritesheet/archive/master.zip
fi
echo Unzip archive
unzip master.zip > /dev/null
./cut_image.sh Universal-LPC-spritesheet-master
