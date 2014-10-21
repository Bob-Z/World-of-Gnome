function f ()

id = player_get_id()

target = character_get_selected_character_id(id)
target_name = character_get_name(id)

if target_name == Taben then

end
if keyword == nil then
	keyword = "start"
end

if keyword == "start" then
	text = string.format("Hello %s, I am %s. Would you like to help me ?",player_name,oracle_name)
	speak_send(npc,"portrait/oracle.png",player,text,"","Yes, of course","yes","","No, thanks","bye")
end

if keyword == "yes" then
	item = item_create_from_template("artefact")
	map_add_item("cliff",item,5,5);
	text = string.format("Thanks %s, please go fetch the holy artefact !",player_name)
	speak_send(npc,"portrait/oracle.png",player,text)
end

if keyword == "bye" then
	speak_send(npc,"portrait/oracle.png",player,"See you !")
end
end
