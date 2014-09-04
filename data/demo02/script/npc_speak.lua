function f (npc,player,keyword)

if keyword == nil then
	keyword = "start"
end

if keyword == "start" then
	speak_send(npc,player,"Hello","icon/reddot.png","How are you ?","how","icon/reddot.png","Goodbye","bye")
end

if keyword == "how" then
	speak_send(npc,player,"Fine thank you","icon/reddot.png","Goodbye","bye")
end

if keyword == "bye" then
	speak_send(npc,player,"See you !")
end
end
