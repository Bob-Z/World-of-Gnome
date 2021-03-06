#!/bin/bash

DEST_DIR=output

IS_CHECK_OK=0

function check_sex {
	IS_CHECK_OK=1

	if [ "$2" == "" ];then
		return
	fi

	FILE_NAME=$1
	SEX=$2

	if [ $SEX == male ];then
		echo $FILE_NAME | grep -i female > /dev/null 2>&1
		if [ $? == 0 ];then
			IS_CHECK_OK=0
		fi
	fi

	if [ $SEX == female ];then
		echo $FILE_NAME | grep -i male | grep -v -i female > /dev/null 2>&1
		if [ $? == 0 ];then
			IS_CHECK_OK=0
		fi
	fi

}

function create_map {
	NUM=$1
	SEX=$2

	MAP_NAME=C$NUM$SEX

	FILE=`ls $NUM/WC_*_marquee.zip`
	# Show slash rather than walk cycle for weapons
	if [ $NUM == 24 ];then
		FILE=`ls $NUM/SL_*_marquee.zip`
	fi

	# Count valid num of files
	FILE_QTY=0
	for f in $FILE;do
		SHORT_NAME=`echo $f | sed 's/.*WC_//g' | sed 's/_marquee.zip//g'`
		# use slash rather than walk cycle for weapons
		if [ $NUM == 24 ];then
			SHORT_NAME=`echo $f | sed 's/.*SL_//g' | sed 's/_marquee.zip//g'`
		fi

		check_sex $SHORT_NAME $SEX
		if [ $IS_CHECK_OK == 0 ];then
			continue
		fi
		let "FILE_QTY=$FILE_QTY+1"
	done
	echo $FILE_QTY files in map $MAP_NAME

	if [ $FILE_QTY -gt 0 ];then
		LINE=`echo "scale = 0; sqrt($FILE_QTY)" | bc`
		COLUMN=`echo "scale = 0; $FILE_QTY/$LINE" | bc`
	else
		LINE=2
		COLUMN=2
	fi

	let "TOTAL=$LINE*$COLUMN"
	if [ "$TOTAL" -lt "$FILE_QTY" ];then
		let "LINE=$LINE+1"
	fi

	# Let a free space between each item column
	let "COLUMN=$COLUMN*2"

	let "MAP_COLUMN=$COLUMN+1"
	let "MAP_LINE=$LINE+4"

	echo "bg_red = 192" > $MAP_NAME
	echo "bg_blue = 192" >> $MAP_NAME
	echo "bg_green = 192" >> $MAP_NAME

	echo "attribute = {" >> $MAP_NAME
        echo "  x = {"	>> $MAP_NAME
        echo "        current = $NUM"	>> $MAP_NAME
	echo "      }"	>> $MAP_NAME
	echo "}"	>> $MAP_NAME
	echo "width = $MAP_COLUMN"	>> $MAP_NAME
	echo "height = $MAP_LINE"	>> $MAP_NAME
	echo "tile_width = 64"	>> $MAP_NAME
	echo "tile_height = 64"	>> $MAP_NAME
	echo "warp_x = 0"	>> $MAP_NAME
	echo "warp_y = 0"	>> $MAP_NAME
	echo "layer0 = {"	>> $MAP_NAME
	echo "        set = ["	>> $MAP_NAME

	#First 2 lines
	echo "\"tile/fog.png\"," >> $MAP_NAME #upper left cancel tile
	for i in `seq 2 $COLUMN`;do
		echo "\"\"," >> $MAP_NAME
	done
	echo "\"tile/fog.png\"," >> $MAP_NAME #upper right cancel tile
	for i in `seq 0 $COLUMN`;do
		echo "\"\"," >> $MAP_NAME
	done

	CURRENT_TILE=0
	CUR_X=1
	CUR_Y=1
	let "HALF_LINE=($LINE/2)+1"
	for f in $FILE;do
		SHORT_NAME=`echo $f | sed 's/.*WC_//g' | sed 's/_marquee.zip//g'`
		# use slash rather than walk cycle for weapons
		if [ $NUM == 24 ];then
			SHORT_NAME=`echo $f | sed 's/.*SL_//g' | sed 's/_marquee.zip//g'`
		fi

		check_sex $SHORT_NAME $SEX
		if [ $IS_CHECK_OK == 0 ];then
			continue
		fi

		if [ "$CUR_Y" = "$HALF_LINE" ];then
			if [ "$CUR_X" = 1 ];then
				echo "\"tile/exit.gif\"," >> $MAP_NAME
			else
				echo "\"\"," >> $MAP_NAME
			fi
		else
			echo "\"\"," >> $MAP_NAME
		fi
		echo "\"character/$f\"," >> $MAP_NAME
		let "CURRENT_TILE=$CURRENT_TILE+1"
		let "CUR_X=$CUR_X+2"
		if [ "$CUR_X" -ge $COLUMN ];then
			if [ "$CUR_Y" = "$HALF_LINE" ];then
				echo "\"tile/exit.gif\"," >> $MAP_NAME
			else
				echo "\"\"," >> $MAP_NAME
			fi
			let "CUR_Y=$CUR_Y+1"
			let "CUR_X=1"
		fi
	done

	
	if [ ! "$CUR_X" = "1" ];then # Last line was not complete
		let "ADD_TILE=$MAP_COLUMN-$CUR_X+1"
		for i in `seq 1 $ADD_TILE`;do
			echo "\"\"," >> $MAP_NAME
		done
	fi

	#last 2 lines
	for i in `seq 0 $COLUMN`;do
		echo "\"\"," >> $MAP_NAME
	done
	echo "\"tile/fog.png\"," >> $MAP_NAME #lower left cancel tile
	for i in `seq 2 $COLUMN`;do
		echo "\"\"," >> $MAP_NAME
	done
	echo "\"tile/fog.png\"" >> $MAP_NAME #lower right cancel tile

	#end of set
	echo "        ]"	>> $MAP_NAME
	echo "}"	>> $MAP_NAME

	######################
	#Events
	######################
	echo "event_list: {"	>> $MAP_NAME

	CUR_X=1
	CUR_Y=2
	CURRENT_EVENT=0
	for f in $FILE;do
		SHORT_NAME=`echo $f | sed 's/.*WC_//g' | sed 's/_marquee.zip//g'`
		# use slash rather than walk cycle for weapons
		if [ $NUM == 24 ];then
			SHORT_NAME=`echo $f | sed 's/.*SL_//g' | sed 's/_marquee.zip//g'`
		fi

		check_sex $SHORT_NAME $SEX
		if [ $IS_CHECK_OK == 0 ];then
			continue
		fi

		#Special case for body selection map
		if [ $NUM == 0 ];then
			#Get sex
			echo $SHORT_NAME | grep -i female > /dev/null 2>&1
			if [ $? == 0 ];then
				SET_SEX=female
			else
				SET_SEX=male
			fi
			SET_RACE=human
			echo $SHORT_NAME | grep darkelf > /dev/null 2>&1
			if [ $? == 0 ];then
				SET_RACE=darkelf 
			fi
			echo $SHORT_NAME | grep orc > /dev/null 2>&1
			if [ $? == 0 ];then
				SET_RACE=orc 
			fi
			echo $SHORT_NAME | grep skeleton > /dev/null 2>&1
			if [ $? == 0 ];then
				SET_RACE=skeleton 
			fi
			#Add corresponding event
			echo "E$CURRENT_EVENT = {" >> $MAP_NAME
			echo "   tile_x = $CUR_X" >> $MAP_NAME
			echo "   tile_y = $CUR_Y" >> $MAP_NAME
			echo "   script = \"set_body.lua\"" >> $MAP_NAME
			echo "   param = ( \"$SET_SEX\", \"$SET_RACE\" )" >> $MAP_NAME
			echo "}" >> $MAP_NAME
			let "CURRENT_EVENT=$CURRENT_EVENT+1"
		fi

		echo "E$CURRENT_EVENT = {" >> $MAP_NAME
		echo "   tile_x = $CUR_X" >> $MAP_NAME
		echo "   tile_y = $CUR_Y" >> $MAP_NAME
		echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
		echo "   param = ( \"$NUM\", \"$SHORT_NAME\" )" >> $MAP_NAME
		echo "}" >> $MAP_NAME

		let "CUR_X=$CUR_X+2"
		if [ "$CUR_X" -gt "$COLUMN" ];then
			CUR_X=1
			let "CUR_Y=$CUR_Y+1"
		fi
		let "CURRENT_EVENT=$CURRENT_EVENT+1"
	done

	#Left exit event
	let "PREV_NUM=$NUM-1"
	let "HALF_LINE=$HALF_LINE+1"
	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   tile_x = 0" >> $MAP_NAME
	echo "   tile_y = $HALF_LINE" >> $MAP_NAME
	echo "   script = \"goto_character_map.lua\"" >> $MAP_NAME
	echo "   param = ( \"$PREV_NUM\", \"1\", \"1\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	#Right exit event
	let "LAST_COLUMN=$MAP_COLUMN-1"
	let "NEXT_MAP=$NUM+1"
	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   tile_x = $LAST_COLUMN" >> $MAP_NAME
	echo "   tile_y = $HALF_LINE" >> $MAP_NAME
	if [ "$NEXT_MAP" -ge "$MAP_QTY" ];then
		echo "   script = \"goto.lua\"" >> $MAP_NAME
		echo "   param = ( \"M0_0\", \"10\", \"10\" )" >> $MAP_NAME
	else
		echo "   script = \"goto_character_map.lua\"" >> $MAP_NAME
		echo "   param = ( \"$NEXT_MAP\", \"1\", \"1\" )" >> $MAP_NAME
	fi
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	#Left cancel events
	let "LAST_LINE=MAP_LINE-1"
	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   tile_x = 0" >> $MAP_NAME
	echo "   tile_y = 0" >> $MAP_NAME
	echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
	echo "   param = ( \"$NUM\", \"\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   tile_x = 0" >> $MAP_NAME
	echo "   tile_y = $LAST_LINE" >> $MAP_NAME
	echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
	echo "   param = ( \"$NUM\", \"\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	#Right cancel events
	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   tile_x = $LAST_COLUMN" >> $MAP_NAME
	echo "   tile_y = 0" >> $MAP_NAME
	echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
	echo "   param = ( \"$NUM\", \"\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   tile_x = $LAST_COLUMN" >> $MAP_NAME
	echo "   tile_y = $LAST_LINE" >> $MAP_NAME
	echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
	echo "   param = ( \"$NUM\", \"\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	echo "}"	>> $MAP_NAME

}

cd $DEST_DIR

MAP_QTY=`ls -1 -d [0-9]* | wc -l`
echo Generating $MAP_QTY maps

for NUM in `ls -d [0-9]* | sort -h`;do
	if [ $NUM == 0 ];then
		create_map $NUM
	else
		create_map $NUM male
		create_map $NUM female
	fi
done
