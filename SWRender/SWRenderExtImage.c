/*
    Copyright (C) 2026 Glacc

    This file is part of sw-render.

    sw-render is free software: you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation, either
    version 3 of the License, or (at your option) any later version.

    sw-render is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with sw-render. If not,
    see <https://www.gnu.org/licenses/>. 
*/

#include "SWRenderExtImage.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#include <string.h>

#define __LOG_FILE__ "SWRenderExtImage.c"
#include "MacroLog.h"

typedef struct
{
    const uint8_t *data;
    size_t size;
    size_t offset;
}
AVIOMemData;

static int ConvertFramePixFmtFitToEncoder(SWRenderBuffer *src, AVFrame *dst, const AVCodec *encoder)
{
    int ret = 0;

    const enum AVPixelFormat *pix_fmt_list = NULL;
    int pix_fmt_cnt;
    avcodec_get_supported_config(NULL, encoder, AV_CODEC_CONFIG_PIX_FORMAT, 0, (const void **)&pix_fmt_list, &pix_fmt_cnt);

    enum AVPixelFormat *pix_fmt_none_term_list = aligned_alloc(sizeof(enum AVPixelFormat), sizeof(enum AVPixelFormat) * (pix_fmt_cnt + 1));
    if (!pix_fmt_none_term_list)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "pix_fmt_none_term_list memory allocation failed.\n");
        ret = -1;
        goto end;
    }
    memcpy(pix_fmt_none_term_list, pix_fmt_list, sizeof(enum AVPixelFormat) * pix_fmt_cnt);
    pix_fmt_none_term_list[pix_fmt_cnt] = AV_PIX_FMT_NONE;

    enum AVPixelFormat dst_pix_fmt = avcodec_find_best_pix_fmt_of_list(pix_fmt_none_term_list, AV_PIX_FMT_RGBA, true, NULL);
    if (dst_pix_fmt == -1)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_find_best_pix_fmt_of_list failed to find dst pixel format.\n");
        ret = -1;
        goto end;
    }

    av_frame_unref(dst);
    dst->width  = src->w;
    dst->height = src->h;
    dst->format = dst_pix_fmt;
    if ((ret = av_frame_get_buffer(dst, 0)) < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_frame_get_buffer failed.\n");
        goto end;
    }

    struct SwsContext *sws_ctx = sws_getContext(src->w, src->h, AV_PIX_FMT_RGBA, dst->width, dst->height, dst->format, SWS_BICUBIC, NULL, NULL, NULL);
    if (!sws_ctx)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "sws_alloc_context failed.\n");
        ret = -1;
        goto end;
    }

    uint8_t *data[8] = { (uint8_t *)src->data, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    int linesize[8] = { src->linesize, 0, 0, 0, 0, 0, 0, 0 };
    
    sws_scale(sws_ctx, (const uint8_t *const *)data, linesize, 0, src->h, (uint8_t *const *)dst->data, dst->linesize);

end:
    if (pix_fmt_none_term_list) free(pix_fmt_none_term_list);
    sws_free_context(&sws_ctx);

    return ret;
}

static int OpenAndDecodeInput(AVFormatContext *fmt_ctx, SWRenderBuffer *buffer)
{
    AVCodecContext *codec_ctx = NULL;
    AVPacket *pkt = NULL;
    AVFrame *frame = NULL;
    int ret;

    pkt = av_packet_alloc();
    if (!pkt)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_packet_alloc failed.\n");
        ret = -1;
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_frame_alloc failed.\n");
        ret = -1;
        goto end;
    }

    int first_video_stream_index = -1;
    AVCodecParameters *first_video_stream_codec_para = NULL;
    const AVCodec *first_video_stream_codec = NULL;

    avformat_find_stream_info(fmt_ctx, NULL);
    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        AVStream *stream_current = fmt_ctx->streams[i];
        // printf(LOCATION_PREFIX_STR "stream index %d, id %d, frames %ld\n", stream_current->index, stream_current->id, stream_current->nb_frames);

        AVCodecParameters *codec_para = stream_current->codecpar;
        const AVCodec *codec = avcodec_find_decoder(codec_para->codec_id);
        if (!codec)
        {
            fprintf(stderr, LOCATION_PREFIX_STR "avcodec_find_decoder failed for stream at index %d.\n", stream_current->index);
            continue;
        }

        if (codec_para->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if (first_video_stream_index < 0)
            {
                first_video_stream_index = stream_current->index;
                first_video_stream_codec = codec;
                first_video_stream_codec_para = codec_para;

                // printf(LOCATION_PREFIX_STR "found video stream in size %d x %d\n", codec_para->width, codec_para->height);
            }
        }
    }

    if (first_video_stream_index < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "could not find video stream in given data.\n");
        ret = -1;
        goto end;
    }

    codec_ctx = avcodec_alloc_context3(first_video_stream_codec);
    if (!codec_ctx)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_alloc_context3 failed.\n");
        ret = -1;
        goto end;
    }

    if (avcodec_parameters_to_context(codec_ctx, first_video_stream_codec_para) < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_parameters_to_context failed.\n");
        ret = -1;
        goto end;
    }
    if (avcodec_open2(codec_ctx, first_video_stream_codec, NULL) < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_open2 failed.\n");
        ret = -1;
        goto end;
    }
    
    bool got_frame = false;
    do
    {
        // [file] --read--> [pkt] --send--> [codec] --receive--> [frame]

        ret = av_read_frame(fmt_ctx, pkt);
        if (ret < 0)
        {
            fprintf(stderr, LOCATION_PREFIX_STR "av_read_frame failed.\n");
            break;
        }

        if (pkt->stream_index == first_video_stream_index)
        {
            ret = avcodec_send_packet(codec_ctx, pkt);
            if (ret < 0)
            {
                fprintf(stderr, LOCATION_PREFIX_STR "avcodec_send_packet failed.\n");
                break;
            }

            do
            {
                ret = avcodec_receive_frame(codec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0)
                {
                    fprintf(stderr, LOCATION_PREFIX_STR "avcodec_receive_frame failed.\n");
                    break;
                }

                // save content here?
                buffer->w = frame->width;
                buffer->h = frame->height;

                struct SwsContext *sws_ctx = sws_getContext(frame->width, frame->height, frame->format, buffer->w, buffer->h, AV_PIX_FMT_RGBA, SWS_LANCZOS, NULL, NULL, NULL);
                if (sws_ctx)
                {
                    // dst buffer allocation
                    SWRender_BufferFree(buffer);
                    ret = SWRender_BufferAlloc(buffer);
                    if (ret < 0)
                    {
                        fprintf(stderr, LOCATION_PREFIX_STR "SWRender_BufferAlloc failed.\n");
                        goto end_scale;
                    }

                    uint8_t *data[8] = { (uint8_t *)buffer->data, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
                    int linesize[8] = { buffer->linesize, 0, 0, 0, 0, 0, 0, 0 };
                    
                    // scale (convert)
                    sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0, frame->height, data, linesize);

                end_scale:
                    sws_freeContext(sws_ctx);
                }
                else
                    fprintf(stderr, LOCATION_PREFIX_STR "sws_getContext failed.\n");

                got_frame = true;

                break;
            }
            while (!got_frame);
        }

        av_packet_unref(pkt);
    }
    while (!got_frame);

    ret = 0;

end:
    avcodec_free_context(&codec_ctx);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    return ret;
}

#pragma region AVIOCallbacks

// https://salivity.github.io/ffmpeg/article/custom-i-o-protocol-in-libavformat-using-aviocontext

static int AVIOCallbackReadMem(void *opaque, uint8_t *buffer, int buffer_size)
{
    AVIOMemData *data = (AVIOMemData *)opaque;

    if (data->offset >= data->size)
        return AVERROR_EOF;

    size_t offset = data->offset;

    size_t bytes_to_read = buffer_size;
    size_t remaining_bytes = data->size - data->offset;
    if (bytes_to_read > remaining_bytes)
        bytes_to_read = remaining_bytes;
    
    memcpy(buffer, data->data + offset, bytes_to_read);

    data->offset = offset + bytes_to_read;

    return bytes_to_read;
}

static int64_t AVIOCallbackSeekMem(void *opaque, int64_t offset, int whence)
{
    AVIOMemData *data = (AVIOMemData *)opaque;

    int64_t new_offset = -1;

    switch (whence)
    {
        case SEEK_SET:
            new_offset = offset;
            break;
        case SEEK_CUR:
            new_offset = data->offset + offset;
            break;
        case SEEK_END:
            new_offset = data->size + offset;
            break;
        case AVSEEK_SIZE:
            return data->offset;
        default:
            return AVERROR(EINVAL);
    }

    if ((new_offset < 0) || ((size_t)new_offset > data->size))
        return AVERROR(EINVAL);

    data->offset = (size_t)new_offset;

    return new_offset;
}

#pragma endregion

int SWRender_LoadImageFromFileToBuffer(const char *filepath, SWRenderBuffer *buffer)
{
    AVFormatContext *fmt_ctx = NULL;
    int ret;

    fmt_ctx = avformat_alloc_context();
    if (!fmt_ctx)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avformat_alloc_context failed.\n");
        ret = -1;
        goto end;
    }

    if ((ret = avformat_open_input(&fmt_ctx, filepath, NULL, NULL)))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avformat_open_input failed.\n");
        goto end;
    }

    ret = OpenAndDecodeInput(fmt_ctx, buffer);

end:
    avformat_close_input(&fmt_ctx);
    avformat_free_context(fmt_ctx);

    return ret;
}

int SWRender_LoadImageFromMemoryToBuffer(const uint8_t *src, const size_t size, SWRenderBuffer *buffer)
{
    AVFormatContext *fmt_ctx = NULL;
    int ret;

    fmt_ctx = avformat_alloc_context();
    if (!fmt_ctx)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avformat_alloc_context failed.\n");
        ret = -1;
        goto end;
    }

    uint8_t *avio_buffer = (uint8_t *)av_malloc(SWRENDER_IMG_AVIO_BUF_SIZE);
    if (!avio_buffer)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_malloc failed.\n");
        ret = -1;
        goto end;
    }

    AVIOMemData data = { src, size, 0 };

    AVIOContext *avio_ctx = avio_alloc_context(avio_buffer, SWRENDER_IMG_AVIO_BUF_SIZE, 0, &data, AVIOCallbackReadMem, NULL, AVIOCallbackSeekMem);
    if (!avio_ctx)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avio_alloc_context failed.\n");
        av_freep(&avio_buffer);
        ret = -1;
        goto end;
    }

    fmt_ctx->pb = avio_ctx;

    if ((ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL)))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avformat_open_input failed.\n");
        goto end;
    }

    ret = OpenAndDecodeInput(fmt_ctx, buffer);

end:
    avformat_close_input(&fmt_ctx);
    avformat_free_context(fmt_ctx);

    av_freep(&avio_ctx->buffer);
    avio_context_free(&avio_ctx);

    return ret;
}

int SWRender_SaveImageFromBuffer(SWRenderBuffer *buffer, const char *filepath, enum AVCodecID codec_id)
{
    int ret;

    FILE *file = fopen(filepath, "wb");
    if (!file)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "failed to open file \'%s\'\n", filepath);
        return -1;
    }

    const AVCodec *codec = avcodec_find_encoder(codec_id);
    if (!codec)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_find_encoder failed.\n");
        ret = -1;
        goto end;
    }

    AVFrame *dst_frame = av_frame_alloc();
    if (!dst_frame)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_frame_alloc failed.\n");
        ret = -1;
        goto end;
    }

    if (ConvertFramePixFmtFitToEncoder(buffer, dst_frame, codec))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "frame format convertion failed.\n");
        ret = -1;
        goto end;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_alloc_context3 failed.\n");
        ret = -1;
        goto end;
    }

    codec_ctx->width  = buffer->w;
    codec_ctx->height = buffer->h;
    codec_ctx->time_base = (AVRational){ 1, 25 };
    codec_ctx->framerate = (AVRational){ 25, 1 };
    codec_ctx->pix_fmt = dst_frame->format;

    if (avcodec_open2(codec_ctx, codec, NULL) < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_open2 failed.\n");
        goto end;
    }

    AVPacket *pkt = av_packet_alloc();
    if (!pkt)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_packet_alloc failed.\n");
        goto end;
    }

    if ((ret = avcodec_send_frame(codec_ctx, dst_frame)) < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_send_frame failed.\n");
        goto end;
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(codec_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0)
        {
            fprintf(stderr, LOCATION_PREFIX_STR "avcodec_receive_packet failed.\n");
            break;
        }

        fwrite(pkt->data, 1, pkt->size, file);

        av_packet_unref(pkt);
    }

end:
    av_frame_free(&dst_frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_ctx);
    fclose(file);

    return ret;
}
