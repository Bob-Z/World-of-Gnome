function f (npc,player,keyword)

name = character_get_name(npc);

if keyword == nil then
	keyword = "start"
end

if keyword == "start" then
	speak_send(name,player,"Hello","icon/greendot.gif","How are you ?","how","icon/greendot.gif","Goodbye","bye")
end

if keyword == "how" then
	speak_send(name,player,"Fine thank you","icon/greendot.gif","Goodbye","bye")
end

if keyword == "bye" then
	speak_send(name,player,"See you !")
end
end
