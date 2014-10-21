function f (listner_id, keyword)

id = player_get_id()

if listner_id == nil then
	listner_id = character_get_selected_character_id(id)
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
end

call_action(action,listner_id,id,keyword);

end
