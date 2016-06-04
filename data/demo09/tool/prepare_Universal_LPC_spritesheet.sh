#!/bin/bash

#rm -Rf Universal-LPC-spritesheet-master
if [ ! -f master.zip ];then
	wget https://github.com/jrconway3/Universal-LPC-spritesheet/archive/master.zip
fi
unzip master.zip
cd Universal-LPC-spritesheet-master

SPRITE_TEMPO=200
MARQUEE_TEMPO=500

DIR=`ls`

IFS=$'\n'

ACTION_PREFIX="SC_
TH_
WC_
SL_
SH_
HU_"

ORIENT_PREFIX="N_
W_
S_
E_"

PIC_ACTION[0]=0
PIC_ACTION[1]=7
PIC_ACTION[2]=8
PIC_ACTION[3]=9
PIC_ACTION[4]=6
PIC_ACTION[5]=13
PIC_ACTION[6]=6

for d in $DIR;do
	echo $d
	cd $d > /dev/null 2>&1
	if [ $? == 0 ];then
		LIST=`find -iname "*.png" -type f`
		for l in $LIST; do
			FILENAME=`echo $l | cut -c 3- | sed 's/\//_/g' | sed 's/ /_/g'`
			#echo l= $l
			#echo FILENAME= $FILENAME
			cp "$l" "$FILENAME"
			rm tmp_tiles*.png
			convert -crop 64x64 "$FILENAME"  tmp_tiles%d.png
			BASENAME=`basename "$FILENAME" .png`
			LINE_PIC=13
			INDEX_PIC=0
			INDEX_ACTION=0
			for action in $ACTION_PREFIX;do
				#For normal sprites
				for orient in $ORIENT_PREFIX;do
					rm timing
					#For first set of action, set the number of picture for this action
					if [ $orient == "N_" ];then
						((INDEX_ACTION=$INDEX_ACTION+1))
					fi
					#For HU (hurt) action, there is no orientation
					if [ $action == "HU_" ];then
						orient=""
					fi

					NUM_PIC=${PIC_ACTION[$INDEX_ACTION]}
					CURRENT_LOOP=0
					#Marquee
					zip ${action}${BASENAME}_marquee.zip tmp_tiles$INDEX_PIC.png
					cp tmp_tiles$INDEX_PIC.png ${action}${orient}$BASENAME.png
					while [ $CURRENT_LOOP -lt $NUM_PIC ];do
						zip ${action}${orient}$BASENAME.zip tmp_tiles$INDEX_PIC.png
						echo $SPRITE_TEMPO >> timing
						((CURRENT_LOOP=$CURRENT_LOOP+1))
						((INDEX_PIC=$INDEX_PIC+1))
					done
					zip ${action}${orient}$BASENAME.zip timing
					((INDEX_PIC=$INDEX_PIC+$LINE_PIC-$NUM_PIC))

					#For HU (hurt) action, there is no orientation
					if [ $action == "HU_" ];then
						break
					fi

				done
				#For marquees
				rm timing
				echo $MARQUEE_TEMPO > timing
				echo $MARQUEE_TEMPO >> timing
				echo $MARQUEE_TEMPO >> timing
				echo $MARQUEE_TEMPO >> timing
				zip ${action}${BASENAME}_marquee.zip timing
			done
		done
		cd - > /dev/null 2>&1
	fi
done
