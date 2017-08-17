#!/bin/bash

DIR_NAME=Axe

rm -Rf $DIR_NAME > /dev/null 2>&1
mkdir $DIR_NAME
cd $DIR_NAME
wget https://opengameart.org/sites/default/files/axe_8.png
cd ..

./cut_image.sh $DIR_NAME

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/$DIR_NAME.lic
echo Author: DarkwallLKE > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo License: GPL 2.0 >> $LIC_FILE
echo Source: https://opengameart.org/content/lpc-axe >> $LIC_FILE
