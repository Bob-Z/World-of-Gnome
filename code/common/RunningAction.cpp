/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2020 carabobz@gmail.com

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "RunningAction.h"
#include "SdlLocking.h"

/*****************************************************************************/
RunningAction::RunningAction(Context * context, const std::string & action, const std::vector<std::string> params, int coolDownMs) :
		m_mutex(SDL_CreateMutex()), m_running(true), m_context(context), m_action(action), m_params(params), m_coolDownMs(coolDownMs)
{
}

/*****************************************************************************/
RunningAction::~RunningAction()
{
	SDL_DestroyMutex(m_mutex);
}

/*****************************************************************************/
void RunningAction::beginAction()
{
	SDL_LockMutex(m_mutex);
}

/*****************************************************************************/
void RunningAction::endAction()
{
	SDL_UnlockMutex(m_mutex);
}

/*****************************************************************************/
bool RunningAction::isRunning() const
{
	return m_running;
}

/*****************************************************************************/
void RunningAction::stop()
{
	SdlLocking lock(m_mutex);

	m_running = false;
}

/*****************************************************************************/
Context* RunningAction::getContext() const
{
	return m_context;
}

/*****************************************************************************/
const std::vector<std::string>& RunningAction::getParams() const
{
	return m_params;
}

/*****************************************************************************/
const std::string& RunningAction::getAction() const
{
	return m_action;
}

/*****************************************************************************/
const int RunningAction::getCoolDownMs() const
{
	return m_coolDownMs;
}
