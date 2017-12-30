#!/bin/bash

OUTPUT_DIR=LizardHeadgear
ZIP_FILE="lizard headwear.zip"

rm -Rf "$OUTPUT_DIR" > /dev/null 2>&1
if [ ! -f "$ZIP_FILE" ];then
	wget https://opengameart.org/sites/default/files/lizard%20headwear.zip
fi
echo Unzip archive
unzip "$ZIP_FILE" > /dev/null
BASE=`basename "$ZIP_FILE" .zip`
mv "$BASE" "$OUTPUT_DIR"

./cut_image.sh $OUTPUT_DIR

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/$OUTPUT_DIR.lic
echo Author: Nila122 > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo Source: https://opengameart.org/content/lpc-lizard-headgear >> $LIC_FILE

