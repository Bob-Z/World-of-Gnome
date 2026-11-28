/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef HEADLESS_H
#define HEADLESS_H

#include <string>

/*
 Headless mode replaces SDL's image/sound calls by log entries and feeds
 the event loop with synthetic SDL events.

 Events are read as one command per line (blank lines and # comments are
 ignored):

   mouse_move  <x> <y>
   mouse_down  <button> <x> <y>   # button: 1 = left, 3 = right
   mouse_up    <button> <x> <y>
   key_down    <scancode>         # SDL scancode number or name (escape, a, ...)
   key_up      <scancode>
   wait        <milliseconds>
   quit

 End of input generates a single SDL_QUIT event, so the client exits
 cleanly. A window-enter event is generated automatically at startup so
 mouse events are not ignored by sdl_item.
 */
bool headless_enabled();
void headless_init(bool enabled, const std::string &logFile,
		const std::string &eventsFile);
void headless_log(const std::string &message);
int headless_poll_event(void *event);

#endif /* HEADLESS_H */
