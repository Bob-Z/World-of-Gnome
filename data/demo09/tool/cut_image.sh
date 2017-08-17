#!/bin/bash

#set -x

cd "$1"

SPRITE_TEMPO=200
MARQUEE_TEMPO=500

NUM_IMAGE_W=13
NUM_IMAGE_H=21

DIR=`ls -d ./ */`

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
	if [ "$d" == "_build/" ];then
		continue
	fi

	echo Processing $d
	cd "$d" > /dev/null 2>&1
	if [ ! $? == 0 ];then
		continue
	fi

	if [ "$d" = "./" ];then
		LIST=`find -maxdepth 1 -iname "*.png" -type f`  # Recursive search
	else
		LIST=`find -iname "*.png" -type f`  # Recursive search
	fi

	for FILE in $LIST; do
		IMG_W=`identify -format "%w" "$FILE"`
		IMG_H=`identify -format "%h" "$FILE"`
		let "CROP_W=$IMG_W/$NUM_IMAGE_W"
		let "CROP_H=$IMG_H/$NUM_IMAGE_H"
		if [ ! $CROP_W == 64 ];then
			echo Skipping $FILE, wrong size: ${IMG_W}x${IMG_H}
			continue
		fi
		if [ ! $CROP_H == 64 ];then
			echo Skipping $FILE, wrong size: ${IMG_W}x${IMG_H}
			continue
		fi
		FILENAME=`echo $FILE | cut -c 3- | sed 's/\//_/g' | sed 's/ /_/g'`
		#echo FILE= $FILE
		#echo FILENAME= $FILENAME
		cp "$FILE" "$FILENAME" > /dev/null 2>&1
		rm tmp_tiles*.png > /dev/null 2>&1
		convert -crop ${CROP_W}x${CROP_H} "$FILENAME"  tmp_tiles%d.png
		BASENAME=`basename "$FILENAME" .png`
		INDEX_PIC=0
		INDEX_ACTION=0
		for action in $ACTION_PREFIX;do
			#For normal sprites
			for orient in $ORIENT_PREFIX;do
				rm timing > /dev/null 2>&1
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
				#Marquee: First image of each orientation
				FILE_SIZE=$(wc -c < "tmp_tiles$INDEX_PIC.png")
				if [ $FILE_SIZE -gt 340 ]; then
					zip ${action}${BASENAME}_marquee.zip tmp_tiles$INDEX_PIC.png > /dev/null
				fi

				#Static file per orientation
				cp tmp_tiles$INDEX_PIC.png ${action}${orient}$BASENAME.png
				#Animated file per orientation
				while [ $CURRENT_LOOP -lt $NUM_PIC ];do
					zip ${action}${orient}$BASENAME.zip tmp_tiles$INDEX_PIC.png > /dev/null
					echo $SPRITE_TEMPO >> timing
					((CURRENT_LOOP=$CURRENT_LOOP+1))
					((INDEX_PIC=$INDEX_PIC+1))
				done
				zip ${action}${orient}$BASENAME.zip timing > /dev/null
				((INDEX_PIC=$INDEX_PIC+$NUM_IMAGE_W-$NUM_PIC))

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
			zip ${action}${BASENAME}_marquee.zip timing > /dev/null
		done

		# Clean-up
		rm tmp_tiles*
		rm timing
	done
	cd - > /dev/null 2>&1
done
