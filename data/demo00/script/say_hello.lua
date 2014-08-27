function f ()

id = player_get_id()

aggro_id = parameter0

if aggro_id == "" then
	character_attribute_set(id,"already_aggro",0)
else
	if character_attribute_get(id,"already_aggro") == 0 then
		text = string.format("Hello %s",character_get_name(aggro_id))
		print_text_id( aggro_id , text)
		character_attribute_set(id,"already_aggro",1)
	end
end

end
