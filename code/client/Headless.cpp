/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 */

#include "Headless.h"
#include <SDL.h>
#include <chrono>
#include <ctime>
#include <deque>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace
{
bool enabled = false;

std::mutex logMutex;
std::ostream *logStream = &std::cout;
std::unique_ptr<std::ofstream> g_logFile;

std::istream *eventStream = &std::cin;
std::unique_ptr<std::ifstream> g_eventFile;
std::deque<SDL_Event> eventQueue;
bool eventsStarted = false;
bool quitQueued = false;

std::vector<std::string> splitWords(const std::string &line)
{
	std::istringstream in(line);
	std::vector<std::string> words;
	std::string word;
	while (in >> word)
	{
		words.push_back(word);
	}
	return words;
}

int parseInt(const std::string &word, int defaultValue)
{
	try
	{
		return std::stoi(word);
	}
	catch (...)
	{
		return defaultValue;
	}
}

SDL_Scancode parseScancode(const std::string &word)
{
	// Numeric scancodes are accepted as-is, otherwise ask SDL to resolve
	// names such as "escape", "space" or "a".
	bool numeric = !word.empty();
	for (char c : word)
	{
		if ((c < '0') || (c > '9'))
		{
			numeric = false;
			break;
		}
	}
	if (numeric == true)
	{
		return static_cast<SDL_Scancode>(parseInt(word, SDL_SCANCODE_UNKNOWN));
	}
	return SDL_GetScancodeFromName(word.c_str());
}

void queueMouseMotion(int x, int y)
{
	SDL_Event event = {};
	event.type = SDL_MOUSEMOTION;
	event.motion.x = x;
	event.motion.y = y;
	event.motion.timestamp = SDL_GetTicks();
	eventQueue.push_back(event);
}

void queueLine(const std::string &line)
{
	std::vector<std::string> words = splitWords(line);
	if (words.empty())
	{
		return;
	}

	const std::string &command = words[0];
	if (command == "mouse_move")
	{
		if (words.size() >= 3)
		{
			queueMouseMotion(parseInt(words[1], 0), parseInt(words[2], 0));
		}
	}
	else if ((command == "mouse_down") || (command == "mouse_up"))
	{
		if (words.size() >= 4)
		{
			int button = parseInt(words[1], SDL_BUTTON_LEFT);
			int x = parseInt(words[2], 0);
			int y = parseInt(words[3], 0);

			// sdl_item hit-tests with the last motion coordinates, so emit
			// a motion event before every button event.
			queueMouseMotion(x, y);

			SDL_Event event = {};
			event.type = (command == "mouse_down") ? SDL_MOUSEBUTTONDOWN
					: SDL_MOUSEBUTTONUP;
			event.button.button = static_cast<Uint8>(button);
			event.button.x = x;
			event.button.y = y;
			event.button.clicks = 1;
			event.button.state = (command == "mouse_down") ? SDL_PRESSED
					: SDL_RELEASED;
			event.button.timestamp = SDL_GetTicks();
			eventQueue.push_back(event);
		}
	}
	else if ((command == "key_down") || (command == "key_up"))
	{
		if (words.size() >= 2)
		{
			SDL_Scancode scancode = parseScancode(words[1]);
			SDL_Event event = {};
			event.type = (command == "key_down") ? SDL_KEYDOWN : SDL_KEYUP;
			event.key.repeat = 0;
			event.key.keysym.scancode = scancode;
			event.key.keysym.sym = SDL_GetKeyFromScancode(scancode);
			event.key.timestamp = SDL_GetTicks();
			eventQueue.push_back(event);
		}
	}
	else if (command == "wait")
	{
		if (words.size() >= 2)
		{
			SDL_Delay(static_cast<Uint32>(parseInt(words[1], 0)));
		}
	}
	else if (command == "quit")
	{
		quitQueued = true;
		SDL_Event event = {};
		event.type = SDL_QUIT;
		eventQueue.push_back(event);
	}
	else if (command[0] != '#')
	{
		headless_log("unknown event command: " + line);
	}
}
}

bool headless_enabled()
{
	return enabled;
}

void headless_init(bool isEnabled, const std::string &logFile,
		const std::string &eventsFile)
{
	enabled = isEnabled;
	if (enabled == false)
	{
		return;
	}

	if (logFile.empty() == false)
	{
		auto file = std::make_unique<std::ofstream>(logFile);
		if (file->is_open())
		{
			g_logFile = std::move(file);
			logStream = g_logFile.get();
		}
		else
		{
			std::cerr << "headless: cannot open log file " << logFile
					<< ", using stdout" << std::endl;
		}
	}

	if (eventsFile.empty() == false)
	{
		auto file = std::make_unique<std::ifstream>(eventsFile);
		if (file->is_open())
		{
			g_eventFile = std::move(file);
			eventStream = g_eventFile.get();
		}
		else
		{
			std::cerr << "headless: cannot open events file " << eventsFile
					<< ", using stdin" << std::endl;
		}
	}

	// Timers and the keyboard state table are still provided by SDL; only
	// video/audio/font rendering is replaced by logging stubs.
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS);

	headless_log("enabled (events from "
			+ std::string((g_eventFile != nullptr) ? eventsFile : "stdin")
			+ ", log to "
			+ std::string((g_logFile != nullptr) ? logFile : "stdout") + ")");
}

void headless_log(const std::string &message)
{
	const auto now = std::chrono::system_clock::now();
	const std::time_t time = std::chrono::system_clock::to_time_t(now);
	const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
			now.time_since_epoch()).count() % 1000;

	std::tm localTime = {};
	localtime_r(&time, &localTime);

	std::ostringstream timestamp;
	timestamp << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
			<< '.' << std::setfill('0') << std::setw(3) << milliseconds;

	std::lock_guard<std::mutex> guard(logMutex);
	*logStream << '[' << timestamp.str() << "] [headless] " << message
			<< std::endl;
}

int headless_poll_event(void *voidEvent)
{
	SDL_Event *event = static_cast<SDL_Event*>(voidEvent);

	if (eventsStarted == false)
	{
		eventsStarted = true;
		SDL_Event enter = {};
		enter.type = SDL_WINDOWEVENT;
		enter.window.event = SDL_WINDOWEVENT_ENTER;
		eventQueue.push_back(enter);
	}

	if (eventQueue.empty() == true)
	{
		std::string line;
		while ((eventQueue.empty() == true)
				&& static_cast<bool>(std::getline(*eventStream, line)))
		{
			queueLine(line);
		}

		if ((eventQueue.empty() == true) && (quitQueued == false))
		{
			quitQueued = true;
			SDL_Event quit = {};
			quit.type = SDL_QUIT;
			eventQueue.push_back(quit);
		}
	}

	if (eventQueue.empty() == true)
	{
		return 0;
	}

	*event = eventQueue.front();
	eventQueue.pop_front();
	return 1;
}
