#!/bin/bash

rm -Rf Bandage > /dev/null 2>&1
if [ ! -f "Bandages.zip" ];then
	wget http://opengameart.org/sites/default/files/Bandages.zip
fi

echo Unzip archive
unzip "Bandages.zip" > /dev/null

./cut_image.sh Bandage

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/Bandages.lic
echo Author: Nila122 > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo License: GPL 2.0 >> $LIC_FILE
echo Source: http://opengameart.org/content/lpc-bandages >> $LIC_FILE
