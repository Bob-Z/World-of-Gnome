#!/bin/bash

DIR_NAME=Long_Knife

rm -Rf $DIR_NAME > /dev/null 2>&1
mkdir $DIR_NAME
cd $DIR_NAME
wget https://opengameart.org/sites/default/files/long_knife.png
cd ..

./cut_image.sh $DIR_NAME

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/$DIR_NAME.lic
echo Author: elmerenges > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo License: GPL 2.0 >> $LIC_FILE
echo Source: https://opengameart.org/content/lpc-long-dagger >> $LIC_FILE
