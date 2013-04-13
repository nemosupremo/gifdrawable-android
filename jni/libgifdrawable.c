/*

Copyright (C) 2012 psnim2000@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include <jni.h>
#include <time.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <stdlib.h>
#include <stdio.h>

#include "gif_lib.h"

#define  LOG_TAG    "libgifdrawable"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  argb(a,r,g,b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)
#define  delay(ext) (10*((ext)->Bytes[2] << 8 | (ext)->Bytes[1]))
#define  dispose(ext) (((ext)->Bytes[0] & 0x1c) >> 2)
#define  transparency(ext) ((ext)->Bytes[0] & 1)
#define  trans_index(ext) ((ext)->Bytes[3])
#define  ZERO_DELAY 80
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef struct GifAnimInfo {
  int duration;
  int current_frame;

} GifAnimInfo;

int fd, bytesLeft;

/* Return current time in milliseconds */
double now_ms(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1000. + tv.tv_usec/1000.;
}

int getFrame(GifFileType* gif_handle, int level) {
  unsigned int i,j,k,ms;
  ExtensionBlock * ext = 0;
  SavedImage * frame;
  if (((GifAnimInfo *)gif_handle->UserData)->duration == 0) {
    return 0;
  }
  ms = (level * 10) % ((GifAnimInfo *)gif_handle->UserData)->duration;
  for (i=0,k=0; i<gif_handle->ImageCount; i++) {
    frame = &(gif_handle->SavedImages[i]);
    for (j=0; j<frame->ExtensionBlockCount; j++) {
      if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
        ext = &(frame->ExtensionBlocks[j]);
        break;
      }
    }

    if (ext == 0) {
      return 0;
    }
    k += delay(ext) == 0 ? ZERO_DELAY : delay(ext);
    if (ms <= k) {
      return i;
    }
  }
  LOGE("Frame error %d %d", ms, ((GifAnimInfo *)gif_handle->UserData)->duration);
  return -1;
}

void drawFrame(GifFileType* gif, AndroidBitmapInfo*  info, int* pixels, int frame_no, bool force_dispose_1) {
  GifColorType *bg;
  GifColorType *color;
  SavedImage * frame;
  ExtensionBlock * ext = 0;
  GifImageDesc * frameInfo;
  ColorMapObject * colorMap;
  int *line;
  int width, height,x,y,j,loc,n,inc,p;
  int* px;

  width = gif->SWidth;
  height = gif->SHeight;
  bg = &gif->SColorMap[gif->SBackGroundColor];

  frame = &(gif->SavedImages[frame_no]);
  frameInfo = &(frame->ImageDesc);
  if (frameInfo->ColorMap) {
      colorMap = frameInfo->ColorMap;
    } else {
      colorMap = gif->SColorMap;
  }

  for (j=0; j<frame->ExtensionBlockCount; j++) {
    if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
      ext = &(frame->ExtensionBlocks[j]);
      break;
    }
  }

  // For dispose = 1, we assume its been drawn
  px = pixels;
  if (ext && dispose(ext) == 1 && force_dispose_1 && frame_no > 0) {
    drawFrame(gif, info, pixels, frame_no-1, true);
  }
  else if (ext && dispose(ext) == 2 && bg) {
    for (y=0; y<height; y++) {
      line = (int*) px;
      for (x=0; x<width; x++) {
        line[x] = argb(255, bg->Red, bg->Green, bg->Blue);
      }
      px = (char*)px + info->stride;
    }
  } else if (ext && dispose(ext) == 3 && frame_no > 1) {
    drawFrame(gif, info, pixels, frame_no-2, true);
  }
  px = pixels;
  if (frameInfo->Interlace && false) {
    n = 0;
    inc = 8;
    p = 0;
    px = (char*)px + info->stride*frameInfo->Top;
    for (y=frameInfo->Top; y<frameInfo->Top+frameInfo->Height; y++) {
      for (x=frameInfo->Left; x<frameInfo->Left+frameInfo->Width; x++) {
        loc = (y - frameInfo->Top)*frameInfo->Width + (x - frameInfo->Left);
        if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {
          continue;
        }

        color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg : &colorMap->Colors[frame->RasterBits[loc]];
        if (color && color->Red && color->Green && color->Blue)
          line[x] = argb(255, color->Red, color->Green, color->Blue);
      }
      px = (char*)px + info->stride*inc;
      n += inc;
      if (n >= frameInfo->Height) {
        n = 0;
        switch(p) {
          case 0:
          px = (char *)pixels + info->stride*(4+frameInfo->Top);
          inc = 8;
          p++;
          break;
          case 1:
          px = (char *)pixels + info->stride*(2+frameInfo->Top);
          inc = 4;
          p++;
          break;
          case 2:
          px = (char *)pixels + info->stride*(1+frameInfo->Top);
          inc = 2;
          p++;
        }
      }
    }
  }
  else {
    px = (char*)px + info->stride*frameInfo->Top;
    for (y=frameInfo->Top; y<frameInfo->Top+frameInfo->Height; y++) {
      line = (int*) px;
      for (x=frameInfo->Left; x<frameInfo->Left+frameInfo->Width; x++) {
        loc = (y - frameInfo->Top)*frameInfo->Width + (x - frameInfo->Left);
        if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {
          continue;
        }
        color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg : &colorMap->Colors[frame->RasterBits[loc]];
        line[x] = argb(255, color->Red, color->Green, color->Blue);
      }
      px = (char*)px + info->stride;
    }
  }
}

static long long loadGif(JNIEnv * env, GifFileType* gif, jarray size)
{

  SavedImage * frame = 0;
  GifImageDesc * frameInfo = 0;
  ExtensionBlock * ext = 0;
  int frame_delay,error,i,j;

  GifAnimInfo * gif_info = malloc(sizeof(GifAnimInfo));
  gif->UserData = (void *)gif_info;

  jint *elems = (*env)->GetIntArrayElements(env,size,NULL);
  elems[0] = gif->SWidth;
  elems[1] = gif->SHeight;
  (*env)->ReleaseIntArrayElements(env,size,elems,0);

  frame_delay = ZERO_DELAY;
  gif_info->duration = 0;
  gif_info->current_frame = -1;
  //LOGE("gif image count %d", gif->ImageCount);
  for (i=0; i<gif->ImageCount; i++) {
    frame = &(gif->SavedImages[i]);
    frameInfo = &(frame->ImageDesc);
    //LOGE("frameInfo %p", frameInfo);
    //LOGE("extCount %p", frame->ExtensionBlockCount);
    for (j=0; j<frame->ExtensionBlockCount; j++) {
      if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
        ext = &(frame->ExtensionBlocks[j]);
        break;
      }
    }

    if (ext) {
      frame_delay = delay(ext);

      if (frame_delay == 0)
        frame_delay = ZERO_DELAY;

      gif_info->duration += frame_delay;
    }
  }
  //LOGE("gif duration %d", gif_info->duration);
  return (long long) gif;


}

JNIEXPORT jlong JNICALL Java_com_droidtools_android_graphics_GifDrawable_loadGifFile(JNIEnv * env, jobject  obj, jstring filepath, jarray size)
{
  int error;

  const char *native_file_path = (*env)->GetStringUTFChars(env, filepath, 0);
  GifFileType* gif = DGifOpenFileName(native_file_path,&error);
  error = DGifSlurp(gif);
  (*env)->ReleaseStringUTFChars(env, filepath, native_file_path);
  return loadGif(env, gif, size);
}

static int readFunc(GifFileType* gif, GifByteType* bytes, int size)
{
  int szRead = read(fd, bytes, MIN(size, bytesLeft));
  bytesLeft -= szRead;
  return szRead;
}

JNIEXPORT jlong JNICALL Java_com_droidtools_android_graphics_GifDrawable_loadGifAsset(JNIEnv * env, jobject  obj, jobject assetManager, jstring filepath, jarray size)
{
  int error;

  AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
  const char *native_file_path = (*env)->GetStringUTFChars(env, filepath, 0);
  AAsset* asset = AAssetManager_open(mgr, native_file_path, AASSET_MODE_UNKNOWN);
  off_t start, length;
  fd = AAsset_openFileDescriptor(asset, &start, &length);
  bytesLeft = length;
  lseek(fd, start, SEEK_SET);
  if (fd < 0) {
    return 0;
  }
  GifFileType* gif = DGifOpen(NULL,&readFunc,&error);
  error = DGifSlurp(gif);
  (*env)->ReleaseStringUTFChars(env, filepath, native_file_path);
  AAsset_close(asset);
  return loadGif(env, gif, size);
}


JNIEXPORT jboolean JNICALL Java_com_droidtools_android_graphics_GifDrawable_updateFrame(JNIEnv * env, jobject obj, jlong jgif_handle,  jobject bitmap, jint level)
{
  GifFileType* gif_handle = (GifFileType *)jgif_handle;

  int frame = getFrame(gif_handle, level);
  if (((GifAnimInfo *)gif_handle->UserData)->current_frame == frame) {
    return JNI_FALSE;
  }

  AndroidBitmapInfo  info;
  void*              pixels;
  int                ret;

  if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
    LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
    return;
  }

  if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
    LOGE("Bitmap format is not RGBA_8888 !");
    return;
  }

  if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
    LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
  }

  //double now = now_ms();
  drawFrame(gif_handle, &info, (int*)pixels, frame, false);
  //now = now_ms() - now;
  //LOGE("Gif Time Taken : %.2f", now);
  AndroidBitmap_unlockPixels(env, bitmap);

  ((GifAnimInfo *)gif_handle->UserData)->current_frame = frame;

  /*
   * TODO : Check if you actually need to draw the frame, if not return JNI_FALSE
   */

  return JNI_TRUE;
}
JNIEXPORT void JNICALL Java_com_droidtools_android_graphics_GifDrawable_recycleGif(JNIEnv * env, jobject obj, jlong jgif_handle) {
  GifFileType* gif_handle = (GifFileType*)jgif_handle;
  free(gif_handle->UserData);
  DGifCloseFile(gif_handle);
  return;
}
