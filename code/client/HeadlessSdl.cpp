/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 SDL/SDL_ttf/SDL_mixer replacements used in headless mode. Defining these
 symbols in the client executable interposes the calls made by libsdl_item
 and libcommon as well. Outside headless mode every call is forwarded to
 the real SDL library, so normal behaviour is unchanged.
 */

#include "Headless.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <cstdint>
#include <dlfcn.h>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
struct FakeTexture
{
	int w;
	int h;
};

struct FakeFont
{
	int size;
};

// The stubs below define all Mix_*/TTF_* symbols used by the client, so the
// linker would otherwise drop those libraries (--as-needed) and the real
// implementations could not be found via dlsym(RTLD_NEXT) in normal mode.
extern "C" const SDL_version* Mix_Linked_Version(void);
extern "C" const SDL_version* TTF_Linked_Version(void);
[[maybe_unused]] const void *keepLibrariesLoaded[] = {
	reinterpret_cast<const void*>(&Mix_Linked_Version),
	reinterpret_cast<const void*>(&TTF_Linked_Version)
};

struct FakeSurface
{
	std::unique_ptr<SDL_Surface> surface;
	std::unique_ptr<Uint8[]> pixels;
};

std::mutex fakeMutex;
std::unordered_map<SDL_Texture*, std::unique_ptr<FakeTexture>> textures;
std::unordered_map<TTF_Font*, std::unique_ptr<FakeFont>> fonts;
std::unordered_map<SDL_Surface*, std::unique_ptr<FakeSurface>> surfaces;

SDL_Texture* fakeTexture(int w, int h)
{
	auto texture = std::make_unique<FakeTexture>();
	texture->w = w;
	texture->h = h;
	auto *handle = reinterpret_cast<SDL_Texture*>(texture.get());
	std::lock_guard<std::mutex> guard(fakeMutex);
	textures.emplace(handle, std::move(texture));
	return handle;
}

TTF_Font* fakeFont(int size)
{
	auto font = std::make_unique<FakeFont>();
	font->size = size;
	auto *handle = reinterpret_cast<TTF_Font*>(font.get());
	std::lock_guard<std::mutex> guard(fakeMutex);
	fonts.emplace(handle, std::move(font));
	return handle;
}

SDL_Surface* fakeSurface(int w, int h, int depth)
{
	auto fake = std::make_unique<FakeSurface>();
	fake->surface = std::make_unique<SDL_Surface>();
	fake->surface->w = w;
	fake->surface->h = h;
	fake->surface->pitch = w * ((depth + 7) / 8);
	fake->pixels = std::make_unique<Uint8[]>(fake->surface->pitch * h);
	fake->surface->pixels = fake->pixels.get();

	auto *handle = fake->surface.get();
	std::lock_guard<std::mutex> guard(fakeMutex);
	surfaces.emplace(handle, std::move(fake));
	return handle;
}

int fontHeight(TTF_Font *font)
{
	std::lock_guard<std::mutex> guard(fakeMutex);
	auto it = fonts.find(font);
	return (it == fonts.end()) ? 12 : static_cast<int>(it->second->size * 1.25);
}

std::string pointerString(const void *pointer)
{
	std::ostringstream out;
	out << pointer;
	return out.str();
}

std::string safeText(const char *text)
{
	std::string result = (text != nullptr) ? text : "";
	for (char &c : result)
	{
		if ((c == '\n') || (c == '\r') || (c == '\t'))
		{
			c = ' ';
		}
	}
	if (result.size() > 80)
	{
		result.resize(80);
		result += "...";
	}
	return result;
}

#define HEADLESS_LOG(...) do { \
		std::ostringstream out; \
		out << __VA_ARGS__; \
		headless_log(out.str()); \
	} while (false)

#define HEADLESS_REAL(name) \
	static auto real_##name = reinterpret_cast<decltype(&name)>(dlsym(RTLD_NEXT, #name))
}

/******************************************************************************
  Window and renderer
  *****************************************************************************/
extern "C" SDL_Window* SDL_CreateWindow(const char *title, int x, int y,
		int w, int h, Uint32 flags)
{
	HEADLESS_REAL(SDL_CreateWindow);
	if (headless_enabled() == false)
	{
		return real_SDL_CreateWindow(title, x, y, w, h, flags);
	}
	HEADLESS_LOG("SDL_CreateWindow(title=" << safeText(title) << ", x=" << x
			<< ", y=" << y << ", w=" << w << ", h=" << h
			<< ", flags=" << flags << ")");
	return reinterpret_cast<SDL_Window*>(1);
}

extern "C" SDL_Renderer* SDL_CreateRenderer(SDL_Window *window, int index,
		Uint32 flags)
{
	HEADLESS_REAL(SDL_CreateRenderer);
	if (headless_enabled() == false)
	{
		return real_SDL_CreateRenderer(window, index, flags);
	}
	HEADLESS_LOG("SDL_CreateRenderer(window=" << pointerString(window)
			<< ", index=" << index << ", flags=" << flags << ")");
	return reinterpret_cast<SDL_Renderer*>(1);
}

extern "C" int SDL_SetWindowFullscreen(SDL_Window *window, Uint32 flags)
{
	HEADLESS_REAL(SDL_SetWindowFullscreen);
	if (headless_enabled() == false)
	{
		return real_SDL_SetWindowFullscreen(window, flags);
	}
	HEADLESS_LOG("SDL_SetWindowFullscreen(window=" << pointerString(window)
			<< ", flags=" << flags << ")");
	return 0;
}

extern "C" int SDL_RenderSetLogicalSize(SDL_Renderer *renderer, int w, int h)
{
	HEADLESS_REAL(SDL_RenderSetLogicalSize);
	if (headless_enabled() == false)
	{
		return real_SDL_RenderSetLogicalSize(renderer, w, h);
	}
	HEADLESS_LOG("SDL_RenderSetLogicalSize(renderer="
			<< pointerString(renderer) << ", w=" << w << ", h=" << h << ")");
	return 0;
}

extern "C" SDL_bool SDL_SetHint(const char *name, const char *value)
{
	HEADLESS_REAL(SDL_SetHint);
	if (headless_enabled() == false)
	{
		return real_SDL_SetHint(name, value);
	}
	HEADLESS_LOG("SDL_SetHint(name=" << safeText(name) << ", value="
			<< safeText(value) << ")");
	return SDL_TRUE;
}

extern "C" int SDL_GetRendererOutputSize(SDL_Renderer *renderer, int *w, int *h)
{
	HEADLESS_REAL(SDL_GetRendererOutputSize);
	if (headless_enabled() == false)
	{
		return real_SDL_GetRendererOutputSize(renderer, w, h);
	}
	if (w != nullptr)
	{
		*w = 1024;
	}
	if (h != nullptr)
	{
		*h = 768;
	}
	HEADLESS_LOG("SDL_GetRendererOutputSize(renderer="
			<< pointerString(renderer) << ") -> 1024x768");
	return 0;
}

extern "C" int SDL_RenderCopyEx(SDL_Renderer *renderer, SDL_Texture *texture,
		const SDL_Rect *srcrect, const SDL_Rect *dstrect, const double angle,
		const SDL_Point *center, const SDL_RendererFlip flip)
{
	HEADLESS_REAL(SDL_RenderCopyEx);
	if (headless_enabled() == false)
	{
		return real_SDL_RenderCopyEx(renderer, texture, srcrect, dstrect,
				angle, center, flip);
	}
	HEADLESS_LOG("SDL_RenderCopyEx(texture=" << pointerString(texture)
			<< ", dst=(" << ((dstrect != nullptr) ? dstrect->x : -1) << ","
			<< ((dstrect != nullptr) ? dstrect->y : -1) << "), angle="
			<< angle << ", flip=" << static_cast<int>(flip) << ")");
	return 0;
}

extern "C" int SDL_RenderClear(SDL_Renderer *renderer)
{
	HEADLESS_REAL(SDL_RenderClear);
	if (headless_enabled() == false)
	{
		return real_SDL_RenderClear(renderer);
	}
	HEADLESS_LOG("SDL_RenderClear()");
	return 0;
}

extern "C" void SDL_RenderPresent(SDL_Renderer *renderer)
{
	HEADLESS_REAL(SDL_RenderPresent);
	if (headless_enabled() == false)
	{
		real_SDL_RenderPresent(renderer);
		return;
	}
	HEADLESS_LOG("SDL_RenderPresent()");
}

extern "C" int SDL_SetRenderDrawColor(SDL_Renderer *renderer, Uint8 r,
		Uint8 g, Uint8 b, Uint8 a)
{
	HEADLESS_REAL(SDL_SetRenderDrawColor);
	if (headless_enabled() == false)
	{
		return real_SDL_SetRenderDrawColor(renderer, r, g, b, a);
	}
	HEADLESS_LOG("SDL_SetRenderDrawColor(" << static_cast<int>(r) << ","
			<< static_cast<int>(g) << "," << static_cast<int>(b) << ","
			<< static_cast<int>(a) << ")");
	return 0;
}

/******************************************************************************
  Textures and surfaces
  *****************************************************************************/
extern "C" SDL_Texture* SDL_CreateTexture(SDL_Renderer *renderer, Uint32 format,
		int access, int w, int h)
{
	HEADLESS_REAL(SDL_CreateTexture);
	if (headless_enabled() == false)
	{
		return real_SDL_CreateTexture(renderer, format, access, w, h);
	}
	HEADLESS_LOG("SDL_CreateTexture(format=" << format << ", access=" << access
			<< ", w=" << w << ", h=" << h << ")");
	return fakeTexture(w, h);
}

extern "C" SDL_Texture* SDL_CreateTextureFromSurface(SDL_Renderer *renderer,
		SDL_Surface *surface)
{
	HEADLESS_REAL(SDL_CreateTextureFromSurface);
	if (headless_enabled() == false)
	{
		return real_SDL_CreateTextureFromSurface(renderer, surface);
	}
	int w = (surface != nullptr) ? surface->w : 0;
	int h = (surface != nullptr) ? surface->h : 0;
	HEADLESS_LOG("SDL_CreateTextureFromSurface(w=" << w << ", h=" << h << ")");
	return fakeTexture(w, h);
}

extern "C" int SDL_UpdateTexture(SDL_Texture *texture, const SDL_Rect *rect,
		const void *pixels, int pitch)
{
	HEADLESS_REAL(SDL_UpdateTexture);
	if (headless_enabled() == false)
	{
		return real_SDL_UpdateTexture(texture, rect, pixels, pitch);
	}
	HEADLESS_LOG("SDL_UpdateTexture(texture=" << pointerString(texture)
			<< ", pitch=" << pitch << ")");
	return 0;
}

extern "C" int SDL_QueryTexture(SDL_Texture *texture, Uint32 *format,
		int *access, int *w, int *h)
{
	HEADLESS_REAL(SDL_QueryTexture);
	if (headless_enabled() == false)
	{
		return real_SDL_QueryTexture(texture, format, access, w, h);
	}
	std::lock_guard<std::mutex> guard(fakeMutex);
	auto it = textures.find(texture);
	int tw = (it == textures.end()) ? 1 : it->second->w;
	int th = (it == textures.end()) ? 1 : it->second->h;
	if (format != nullptr)
	{
		*format = 0;
	}
	if (access != nullptr)
	{
		*access = 0;
	}
	if (w != nullptr)
	{
		*w = tw;
	}
	if (h != nullptr)
	{
		*h = th;
	}
	HEADLESS_LOG("SDL_QueryTexture(texture=" << pointerString(texture)
			<< ") -> " << tw << "x" << th);
	return 0;
}

extern "C" void SDL_DestroyTexture(SDL_Texture *texture)
{
	HEADLESS_REAL(SDL_DestroyTexture);
	if (headless_enabled() == false)
	{
		real_SDL_DestroyTexture(texture);
		return;
	}
	{
		std::lock_guard<std::mutex> guard(fakeMutex);
		textures.erase(texture);
	}
	HEADLESS_LOG("SDL_DestroyTexture(texture=" << pointerString(texture) << ")");
}

extern "C" SDL_Surface* SDL_CreateRGBSurface(Uint32 flags, int width,
		int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask,
		Uint32 Amask)
{
	HEADLESS_REAL(SDL_CreateRGBSurface);
	if (headless_enabled() == false)
	{
		return real_SDL_CreateRGBSurface(flags, width, height, depth, Rmask,
				Gmask, Bmask, Amask);
	}
	HEADLESS_LOG("SDL_CreateRGBSurface(w=" << width << ", h=" << height
			<< ", depth=" << depth << ", masks=" << Rmask << "/" << Gmask
			<< "/" << Bmask << "/" << Amask << ")");

	return fakeSurface(width, height, depth);
}

extern "C" void SDL_FreeSurface(SDL_Surface *surface)
{
	HEADLESS_REAL(SDL_FreeSurface);
	if (headless_enabled() == false)
	{
		real_SDL_FreeSurface(surface);
		return;
	}
	{
		std::lock_guard<std::mutex> guard(fakeMutex);
		surfaces.erase(surface);
	}
	HEADLESS_LOG("SDL_FreeSurface(surface=" << pointerString(surface) << ")");
}

/******************************************************************************
  Sound
  *****************************************************************************/
extern "C" SDL_RWops* SDL_RWFromFile(const char *file, const char *mode)
{
	HEADLESS_REAL(SDL_RWFromFile);
	if (headless_enabled() == false)
	{
		return real_SDL_RWFromFile(file, mode);
	}
	HEADLESS_LOG("SDL_RWFromFile(file=" << safeText(file) << ", mode="
			<< safeText(mode) << ")");
	return reinterpret_cast<SDL_RWops*>(1);
}

extern "C" int Mix_Init(int flags)
{
	HEADLESS_REAL(Mix_Init);
	if (headless_enabled() == false)
	{
		return real_Mix_Init(flags);
	}
	HEADLESS_LOG("Mix_Init(flags=" << flags << ")");
	return flags;
}

extern "C" int Mix_OpenAudio(int frequency, Uint16 format, int channels,
		int chunksize)
{
	HEADLESS_REAL(Mix_OpenAudio);
	if (headless_enabled() == false)
	{
		return real_Mix_OpenAudio(frequency, format, channels, chunksize);
	}
	HEADLESS_LOG("Mix_OpenAudio(frequency=" << frequency << ", format="
			<< format << ", channels=" << channels << ", chunksize="
			<< chunksize << ")");
	return 0;
}

extern "C" Mix_Chunk* Mix_LoadWAV_RW(SDL_RWops *source, int freesrc)
{
	HEADLESS_REAL(Mix_LoadWAV_RW);
	if (headless_enabled() == false)
	{
		return real_Mix_LoadWAV_RW(source, freesrc);
	}
	HEADLESS_LOG("Mix_LoadWAV_RW(freesrc=" << freesrc << ")");
	return reinterpret_cast<Mix_Chunk*>(1);
}

extern "C" int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops)
{
	HEADLESS_REAL(Mix_PlayChannel);
	if (headless_enabled() == false)
	{
		return real_Mix_PlayChannel(channel, chunk, loops);
	}
	HEADLESS_LOG("Mix_PlayChannel(channel=" << channel << ", loops="
			<< loops << ")");
	return 0;
}

extern "C" int Mix_HaltChannel(int channel)
{
	HEADLESS_REAL(Mix_HaltChannel);
	if (headless_enabled() == false)
	{
		return real_Mix_HaltChannel(channel);
	}
	HEADLESS_LOG("Mix_HaltChannel(channel=" << channel << ")");
	return 0;
}

extern "C" int Mix_Volume(int channel, int volume)
{
	HEADLESS_REAL(Mix_Volume);
	if (headless_enabled() == false)
	{
		return real_Mix_Volume(channel, volume);
	}
	HEADLESS_LOG("Mix_Volume(channel=" << channel << ", volume=" << volume
			<< ")");
	return volume;
}

/******************************************************************************
  Fonts
  *****************************************************************************/
extern "C" int TTF_Init(void)
{
	HEADLESS_REAL(TTF_Init);
	if (headless_enabled() == false)
	{
		return real_TTF_Init();
	}
	HEADLESS_LOG("TTF_Init()");
	return 0;
}

extern "C" TTF_Font* TTF_OpenFont(const char *file, int ptsize)
{
	HEADLESS_REAL(TTF_OpenFont);
	if (headless_enabled() == false)
	{
		return real_TTF_OpenFont(file, ptsize);
	}
	HEADLESS_LOG("TTF_OpenFont(file=" << safeText(file) << ", ptsize="
			<< ptsize << ")");
	return fakeFont(ptsize);
}

extern "C" int TTF_FontHeight(const TTF_Font *font)
{
	HEADLESS_REAL(TTF_FontHeight);
	if (headless_enabled() == false)
	{
		return real_TTF_FontHeight(font);
	}
	int height = fontHeight(const_cast<TTF_Font*>(font));
	HEADLESS_LOG("TTF_FontHeight() -> " << height);
	return height;
}

extern "C" int TTF_SizeText(TTF_Font *font, const char *text, int *w, int *h)
{
	HEADLESS_REAL(TTF_SizeText);
	if (headless_enabled() == false)
	{
		return real_TTF_SizeText(font, text, w, h);
	}
	int height = fontHeight(font);
	int width = static_cast<int>(safeText(text).size()) * height / 2;
	if (w != nullptr)
	{
		*w = width;
	}
	if (h != nullptr)
	{
		*h = height;
	}
	HEADLESS_LOG("TTF_SizeText(text=" << safeText(text) << ") -> "
			<< width << "x" << height);
	return 0;
}

extern "C" SDL_Surface* TTF_RenderText_Blended(TTF_Font *font,
		const char *text, SDL_Color fg)
{
	HEADLESS_REAL(TTF_RenderText_Blended);
	if (headless_enabled() == false)
	{
		return real_TTF_RenderText_Blended(font, text, fg);
	}
	std::string content = safeText(text);
	int height = fontHeight(font);
	int width = static_cast<int>(content.size()) * height / 2;
	if (width < 1)
	{
		width = 1;
	}
	HEADLESS_LOG("TTF_RenderText_Blended(text=" << content << ") -> "
			<< width << "x" << height);

	return fakeSurface(width, height, 32);
}

/******************************************************************************
  Input state and events
  *****************************************************************************/
extern "C" Uint32 SDL_GetMouseState(int *x, int *y)
{
	HEADLESS_REAL(SDL_GetMouseState);
	if (headless_enabled() == false)
	{
		return real_SDL_GetMouseState(x, y);
	}
	if (x != nullptr)
	{
		*x = 0;
	}
	if (y != nullptr)
	{
		*y = 0;
	}
	return 0;
}

extern "C" const Uint8* SDL_GetKeyboardState(int *numkeys)
{
	HEADLESS_REAL(SDL_GetKeyboardState);
	if (headless_enabled() == false)
	{
		return real_SDL_GetKeyboardState(numkeys);
	}
	static std::vector<Uint8> keys(SDL_NUM_SCANCODES, 0);
	if (numkeys != nullptr)
	{
		*numkeys = static_cast<int>(keys.size());
	}
	return keys.data();
}

extern "C" const char* SDL_GetError(void)
{
	HEADLESS_REAL(SDL_GetError);
	if (headless_enabled() == false)
	{
		return real_SDL_GetError();
	}
	return "headless";
}

extern "C" int SDL_PollEvent(SDL_Event *event)
{
	HEADLESS_REAL(SDL_PollEvent);
	if (headless_enabled() == false)
	{
		return real_SDL_PollEvent(event);
	}
	return headless_poll_event(event);
}
