function f (npc,player,keyword)

oracle_name = character_get_name(npc);
player_name = character_get_name(player);

if keyword == nil then
	keyword = "start"
end

if keyword == "start" then
	text = string.format("Hello %s, I am %s. Would you like to help me ?",player_name,oracle_name)
	speak_send(npc,"portrait/oracle.png",player,text,
			"","Yes, of course","yes",
			"","No, thanks","bye")
end

if keyword == "yes" then
	item = item_create_from_template("artefact")
	map_add_item("cliff",item,5,5);
	text = string.format("Thanks %s, please go fetch the holy artefact !",player_name)
	speak_send(npc,"portrait/oracle.png",player,text,
			"","Let's go !","speak_end")
end

if keyword == "bye" then
	speak_send(npc,"portrait/oracle.png",player,"See you !",
			"","Bye","speak_end")
end

if keyword == "speak_end" then
	speak_send("","",player,"")
end
end
