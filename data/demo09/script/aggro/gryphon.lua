function f (target_id, aggro_on)

id = player_get_id()
old_aggro = character_attribute_tag_get(id,"aggro_target")

if aggro_on == "0" then
	if target_id == old_aggro then
		character_attribute_tag_set(id,"aggro_target","")
		character_set_ai_script(id,"ai/ai_random.lua")
	end
else
	if ( old_aggro == nil ) or ( old_aggro == "" ) then
text = string.format("Aggro ON for %s to %s",id,target_id)
print_text_debug(text)
		character_attribute_tag_set(id,"aggro_target",target_id)
		character_set_ai_script(id,"ai/hunt.lua")
		character_wake_up(id);
	end
end

end
