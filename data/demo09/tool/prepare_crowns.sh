#!/bin/bash

OUTPUT_DIR=Crowns
ZIP_FILE="[LPC] Crown.zip"

rm -Rf "$OUTPUT_DIR" > /dev/null 2>&1
if [ ! -f "$ZIP_FILE" ];then
	wget http://opengameart.org/sites/default/files/%5BLPC%5D%20Crown.zip
fi
echo Unzip archive
unzip "$ZIP_FILE" > /dev/null
BASE=`basename "$ZIP_FILE" .zip`
mv "$BASE" "$OUTPUT_DIR"

./cut_image.sh $OUTPUT_DIR

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/$OUTPUT_DIR.lic
echo Author: DarkwallLKE > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo Source: http://opengameart.org/content/lpc-crown >> $LIC_FILE

