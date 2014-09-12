function f (listner_id, keyword)

id = player_get_id()

target = character_get_selected_character_id(id)
if target == nil then
	print_text_debug("no id selected")
	print_text_id(id, "Please, select a character to talk to")
	return
end

script = character_get_speak(target);

if keyword == nil then
	keyword = "start"
end

call_script(script,target,id,keyword);

end
