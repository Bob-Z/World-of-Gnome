function f (target_id, aggro_on)

if( aggro_on == "0" ) then
	return
end

if( character_get_npc(target_id) == 1 ) then
	return
end

speaker_id = "yezul"

script = character_get_speak(speaker_id);

call_script(script,speaker_id,target_id);

end
