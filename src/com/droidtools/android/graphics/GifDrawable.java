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
