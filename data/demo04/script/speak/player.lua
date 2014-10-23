function f (listner_id, keyword)

player_id = player_get_id()

if listner_id == nil then
	listner_id = character_get_selected_character_id(player_id)
	text = string.format("selected character = %s",listner_id)
	print_text_debug( text)
end

action = character_get_speak(listner_id);
if action == nil then
	text = string.format("%s can't speak",character_get_name(listner_id))
	print_text_id( player_id , text)
end

if keyword == nil then
	keyword = "start"
	player_type = character_get_type(player_id)
	filename = string.format("portrait/%s.gif",player_type)
	character_set_portrait(player_id,filename)
end

call_action(action,listner_id,player_id,keyword);

end
