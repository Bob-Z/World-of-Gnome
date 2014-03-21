function f ()

id = player_get_id()

target = character_get_selected_character_id(id)
-- if the kill was from another avatar
if target ~= nil then
	text = string.format("%s named %s was killed on %s by %s",character_get_type(target), character_get_name(target), character_get_map(target), character_get_name(id))
	print_text_map( character_get_map(target) , text)

	if character_disconnect(target) ~= -1 then
	-- get an apple as a reward for this kill
		inventory_add(id,"apple")
	end
else
--else the kill was from environnment
	text = string.format("%s named %s die on %s",character_get_type(id), character_get_name(id), character_get_map(id))
	print_text_map( character_get_map(id) , text)
	character_disconnect(id)
end

end
