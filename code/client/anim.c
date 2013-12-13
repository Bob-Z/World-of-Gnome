/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013 carabobz@gmail.com

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

#include "sdl.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

anim_t * anim_load(context_t * ctx, const char * filename)
{
	anim_t * anim = NULL;
	anim_t * ret = NULL;
	AVFormatContext *pFormatCtx = NULL;
	int i;
	int videoStream;
	AVCodecContext *pCodecCtx = NULL;
	AVCodec *pCodec = NULL;
	AVFrame *pFrame = NULL;
	AVFrame *pFrameRGB = NULL;
	struct SwsContext * pSwsCtx = NULL;
	AVPacket packet;
	int frameFinished;
	int numBytes;
	uint8_t *buffer = NULL;

	wlog(LOGDEBUG,"Loading anim: %s\n",filename);

	anim = malloc(sizeof(anim_t));
	memset(anim,0,sizeof(anim_t));

        // Register all formats and codecs
        av_register_all();

        // Open video file
        if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0) {
		werr(LOGDEV,"Cannot open file %s\n",filename);
                goto error;
	}

        // Retrieve stream information
        if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		werr(LOGDEV,"Cannot find stream information for file %s\n",filename);
                goto error;
	}


        // Find the first video stream
        videoStream = -1;
        for (i = 0; i < pFormatCtx->nb_streams; i++)
                if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                        videoStream = i;
                        /* total stream duration (in nanosecond) / number of image in the stream / 1000 (to get milliseconds */
                        anim->delay = pFormatCtx->duration / pFormatCtx->streams[i]->duration / 1000;
                        /* If the above doesn't work try with frame_rate : 
                        pFormatCtx->streams[i]->r_frame_rate
                        */
			anim->num_frame = pFormatCtx->streams[i]->duration;
			anim->tex = (SDL_Texture**)malloc(anim->num_frame * sizeof(SDL_Texture*));
                        break;
                }

        if (videoStream == -1) {
		werr(LOGDEV,"Didn't find a video stream in %s\n",filename);
                goto error;
	}

        // Get a pointer to the codec context for the video stream
        pCodecCtx = pFormatCtx->streams[videoStream]->codec;

        // Find the decoder for the video stream
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL) {
		werr(LOGDEV,"Unsupported codec for %s\n",filename);
                goto error;
	}

        // Open codec
        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		werr(LOGDEV,"Could not open codec for %s\n",filename);
                goto error;
	}

        // Allocate video frame
        pFrame = avcodec_alloc_frame();

        // Allocate an AVFrame structure
        pFrameRGB = avcodec_alloc_frame();
        if (pFrameRGB == NULL) {
		werr(LOGDEV,"Could not allocate AVFrame structure for %s\n",filename);
                goto error;
	}

        // Determine required buffer size and allocate buffer
        numBytes = avpicture_get_size(PIX_FMT_RGBA, pCodecCtx->width,
                        pCodecCtx->height);
        buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

        // Assign appropriate parts of buffer to image planes in pFrameRGB
        // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
        // of AVPicture
        avpicture_fill((AVPicture *) pFrameRGB, buffer, PIX_FMT_RGBA,
                        pCodecCtx->width, pCodecCtx->height);

        pSwsCtx = sws_getContext(pCodecCtx->width,
                        pCodecCtx->height, pCodecCtx->pix_fmt,
                        pCodecCtx->width, pCodecCtx->height,
                        PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);

	anim->w = pCodecCtx->width;
	anim->h = pCodecCtx->height;

        if (pSwsCtx == NULL) {
		werr(LOGDEV,"Cannot initialize sws context for %s\n",filename);
                goto error;
	}

        // Read frames
        i = 0;
        while (av_read_frame(pFormatCtx, &packet) >= 0) {
                // Is this a packet from the video stream?
                if (packet.stream_index == videoStream) {
                        // Decode video frame
                        avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
                        // Did we get a video frame?
                        if (frameFinished) {
                                // Convert the image from its native format to ABGR
                                sws_scale(pSwsCtx,
                                        (const uint8_t * const *) pFrame->data,
                                        pFrame->linesize, 0, pCodecCtx->height,
                                        pFrameRGB->data,
                                        pFrameRGB->linesize);

				anim->tex[i] = SDL_CreateTexture(ctx->render, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width,pCodecCtx->height);
				if( anim->tex[i] == NULL ) {
					werr(LOGDEV,"SDL_CreateTexture error: %s\n",SDL_GetError());
				}
                                /* Copy decoded bits to render texture */
                                if (SDL_UpdateTexture(anim->tex[i],NULL,pFrameRGB->data[0],pFrameRGB->linesize[0]) < 0) {
					werr(LOGDEV,"SDL_UpdateTexture error: %s\n",SDL_GetError());
				}
				i++;
                        }
                }

                // Free the packet that was allocated by av_read_frame
                av_free_packet(&packet);
        }

	ret = anim;

error:
	if(ret == NULL) {
		if(anim) {
			if(anim->tex) {
				free(anim->tex);
			}
			free(anim);
		}
	}
#if 0
        // Free the RGB image
	if(buffer) {
		av_free(buffer);
	}

	if(pFrameRGB) {
		av_free(pFrameRGB);
	}

        // Free the YUV frame
	if(pFrame) {
		av_free(pFrame);
	}

        // Close the codec
	if(pCodecCtx) {
		avcodec_close(pCodecCtx);
	}

        // Close the video file
	if(pFormatCtx) {
		avformat_close_input(&pFormatCtx);
	}
#endif

	return ret;
}

void anim_reset_anim(anim_t * anim)
{
	anim->prev_time=0;
	anim->current_frame=0;
}

anim_t * anim_copy(anim_t * src)
{
	anim_t * new_anim;

	new_anim = malloc(sizeof(anim_t));
	new_anim->num_frame = src->num_frame;
	new_anim->tex = src->tex;
	new_anim->current_frame = src->current_frame;
	new_anim->w = src->w;
	new_anim->h = src->h;
	new_anim->delay = src->delay;
	new_anim->prev_time = src->prev_time;

	return new_anim;
}
