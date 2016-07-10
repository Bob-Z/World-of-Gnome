#!/bin/bash

rm -Rf Aprons > /dev/null 2>&1
if [ ! -f "Aprons.zip" ];then
	wget http://opengameart.org/sites/default/files/Aprons.zip
fi

echo Unzip archive
unzip "Aprons.zip" > /dev/null

./cut_image.sh Aprons

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/Aprons.lic
echo Author: Nila122 > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo License: GPL 2.0 >> $LIC_FILE
echo Source: http://opengameart.org/content/lpc-aprons >> $LIC_FILE
