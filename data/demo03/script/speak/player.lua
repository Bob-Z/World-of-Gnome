function f (listner_id, keyword)

id = player_get_id()

script = character_get_speak(listner_id);

if keyword == nil then
	keyword = "start"
end

call_script(script,listner_id,id,keyword);

end
