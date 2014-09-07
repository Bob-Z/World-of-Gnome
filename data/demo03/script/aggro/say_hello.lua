function f (target_id, aggro_on)

id = player_get_id()

if character_get_npc(target_id) == 1 then
	return
end

if aggro_on == "0" then
	character_attribute_set(id,"already_aggro",0)
else
	if character_attribute_get(id,"already_aggro") == 0 then
		text = string.format("Hello %s, I am %s",character_get_name(target_id),character_get_name(id))
		print_text_id( target_id , text)
		character_attribute_set(id,"already_aggro",1)
	end
end

end
