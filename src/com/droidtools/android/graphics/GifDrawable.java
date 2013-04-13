/*
 * Copyright (c) 2013 nimiwaribokoj@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 *  and/or sell copies of the Software, and to permit persons to whom the 
 *  Software is furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included 
 *  in all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 *  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */
package com.droidtools.android.graphics;

import android.content.res.AssetManager;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;

public class GifDrawable extends BitmapDrawable {
	
	private long mGifHandle;
	private int mWidth;
	private int mHeight;
	private int mLevel;
	
	private GifDrawable(Resources res, long gif_handle, int[] size) {
		super(res, Bitmap.createBitmap(size[0], size[1], Bitmap.Config.ARGB_8888));
		mGifHandle = gif_handle;
		mWidth = size[0];
		mHeight = size[1];
	}
	
	public static GifDrawable gifFromFile(Resources res, String filepath) {
		int[] size = new int[2];
		long gif_handle = loadGifFile(filepath, size);
		if (gif_handle != 0L) {
			return new GifDrawable(res, gif_handle, size);
		}
		return null;
	}
	
	public static GifDrawable gifFromAsset(Resources res, String filepath) {
		int[] size = new int[2];
		long gif_handle = loadGifAsset(res.getAssets(), filepath, size);
		if (gif_handle != 0L) {
			return new GifDrawable(res, gif_handle, size);
		}
		return null;
	}
	
	/* Native methods */
    private static native long loadGifFile(String filepath, int sz[]);
    private static native long loadGifAsset(AssetManager mgr, String filepath,  int sz[]);
    private static native boolean updateFrame(long gif_handle, Bitmap bitmap, int level);
    private static native void recycleGif(long gif_handle);
    
    @Override
    protected boolean onLevelChange(int level) {
    	return updateFrame(mGifHandle, getBitmap(), level);
    }
    
    @Override
    public void draw(Canvas canvas) {
    	super.draw(canvas);
    }
    
    public void recycle() {
    	recycleGif(mGifHandle);
    	getBitmap().recycle();
    }
    
    static {
        System.loadLibrary("gifdrawable");
    }
}
