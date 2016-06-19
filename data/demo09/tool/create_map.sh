#!/bin/bash

MAP_QTY=`ls -1 -d [0-9]* | wc -l`
echo Generating $MAP_QTY maps

for NUM in `ls -d [0-9]* | sort -h`;do
	MAP_NAME=C$NUM

	FILE_QTY=`ls -1 $NUM/WC_*_marquee.zip | wc -l`
	echo $FILE_QTY files in map $NUM

	LINE=`echo "scale = 0; sqrt($FILE_QTY)" | bc`
	COLUMN=`echo "scale = 0; $FILE_QTY/$LINE" | bc`
	let "TOTAL=$LINE*$COLUMN"
	if [ "$TOTAL" -lt "$FILE_QTY" ];then
		let "LINE=$LINE+1"
	fi

	let "COLUMN=$COLUMN*2"

	let "MAP_COLUMN=$COLUMN+1"
	let "MAP_LINE=$LINE+4"

	echo "bg_red = 192" > $MAP_NAME
	echo "bg_blue = 192" >> $MAP_NAME
	echo "bg_green = 192" >> $MAP_NAME

	FILE=`ls $NUM/WC_*_marquee.zip`

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
		echo "E$CURRENT_EVENT = {" >> $MAP_NAME
		echo "   pos_x = $CUR_X" >> $MAP_NAME
		echo "   pos_y = $CUR_Y" >> $MAP_NAME
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
	echo "   pos_x = 0" >> $MAP_NAME
	echo "   pos_y = $HALF_LINE" >> $MAP_NAME
	echo "   script = \"goto.lua\"" >> $MAP_NAME
	echo "   param = ( \"C$PREV_NUM\", \"1\", \"1\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	#Right exit event
	let "LAST_COLUMN=$MAP_COLUMN-1"
	let "NEXT_MAP=$NUM+1"
	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   pos_x = $LAST_COLUMN" >> $MAP_NAME
	echo "   pos_y = $HALF_LINE" >> $MAP_NAME
	echo "   script = \"goto.lua\"" >> $MAP_NAME
	if [ "$NEXT_MAP" -ge "$MAP_QTY" ];then
		echo "   param = ( \"M0_0\", \"10\", \"10\" )" >> $MAP_NAME
	else
		echo "   param = ( \"C$NEXT_MAP\", \"1\", \"1\" )" >> $MAP_NAME
	fi
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	#Left cancel events
	let "LAST_LINE=MAP_LINE-1"
	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   pos_x = 0" >> $MAP_NAME
	echo "   pos_y = 0" >> $MAP_NAME
	echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
	echo "   param = ( \"$NUM\", \"\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   pos_x = 0" >> $MAP_NAME
	echo "   pos_y = $LAST_LINE" >> $MAP_NAME
	echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
	echo "   param = ( \"$NUM\", \"\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	#Right cancel events
	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   pos_x = $LAST_COLUMN" >> $MAP_NAME
	echo "   pos_y = 0" >> $MAP_NAME
	echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
	echo "   param = ( \"$NUM\", \"\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   pos_x = $LAST_COLUMN" >> $MAP_NAME
	echo "   pos_y = $LAST_LINE" >> $MAP_NAME
	echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
	echo "   param = ( \"$NUM\", \"\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	echo "}"	>> $MAP_NAME
done
