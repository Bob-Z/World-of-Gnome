function f ()

player_id = player_get_id()

listner_id = character_get_selected_character_id(player_id)

--if keyword == nil then
--	keyword = "start"
--	player_type = character_get_type(player_id)
--	filename = string.format("portrait/%s.gif",player_type)
--	character_set_portrait(player_id,filename)
--end

action = string.format("speak/%s",listner_id)

call_action(action)

end
