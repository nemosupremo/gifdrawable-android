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
package com.droidtools.example;

import com.droidtools.android.graphics.GifDrawable;

import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.app.Activity;
import android.graphics.drawable.Drawable;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

public class MainActivity extends Activity implements OnClickListener {

	TextView loadingText;
	ImageView exImage;
	Drawable currDrawable;
	
	private Handler mHandler;
	private Runnable mUpdateTimeTask;
	private long startTime = 0;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        loadingText = (TextView) findViewById(R.id.loadingText);
        exImage = (ImageView) findViewById(R.id.exImage);
        ((Button) findViewById(R.id.button1)).setOnClickListener(this);
        ((Button) findViewById(R.id.button2)).setOnClickListener(this);
        ((Button) findViewById(R.id.button3)).setOnClickListener(this);
        ((Button) findViewById(R.id.button4)).setOnClickListener(this);
        ((Button) findViewById(R.id.button5)).setOnClickListener(this);
        ((Button) findViewById(R.id.button6)).setOnClickListener(this);
        
        
        /* You should always load images in the background */
        BitmapWorkerTask task = new BitmapWorkerTask();
        task.execute("one.gif");
        
        /*
         * You could also have a similar loop in an 
         * in the onDraw() method of an ImageView.
         */
        mHandler = new Handler();
        mUpdateTimeTask = new Runnable() {    	
    	   public void run() {
    		   long time = System.currentTimeMillis();
	   			if (currDrawable != null) {
	   				if (currDrawable.setLevel((int)(((time - startTime)/10) % 10000))) {
	   					exImage.postInvalidate();
	   				}
	   			}
	   			mHandler.postDelayed(this, 10);
    	   }
    	};
    	mHandler.post(mUpdateTimeTask);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_main, menu);
        return true;
    }

	@Override
	public void onClick(View v) {
		BitmapWorkerTask task = new BitmapWorkerTask();
        task.execute((String)v.getTag());
	}
	
	class BitmapWorkerTask extends AsyncTask<String, Void, GifDrawable> {

	    public BitmapWorkerTask() {
	    	loadingText.setVisibility(View.VISIBLE);
	    	loadingText.setText("Loading...");
	    	exImage.setImageDrawable(null);
	    }

	    // Decode image in background.
	    @Override
	    protected GifDrawable doInBackground(String... params) {
	        String im = params[0];
	        return GifDrawable.gifFromAsset(getResources(), im);
	        //return GifDrawable.gifFromFile(getResources(), "/mnt/sdcard/downloads/a.gif");
	    }

	    // Once complete, see if ImageView is still around and set bitmap.
	    @Override
	    protected void onPostExecute(GifDrawable drawable) {
	        if (drawable != null) {
	            if (currDrawable != null) {
	            	((GifDrawable) currDrawable).recycle();
	            }
	            currDrawable = drawable;
	            exImage.setImageDrawable(drawable);
	            startTime = System.currentTimeMillis();
	            /*
	             * Only works where API Level >= 11
	               ObjectAnimator anim = ObjectAnimator.ofInt(drawable, "level", 0, 10000);
	               anim.setRepeatCount(ObjectAnimator.INFINITE);
	               anim.start();
	             * 
	             */
	            //loadingText.setText("" + drawable.getBitmap().getWidth());
	            loadingText.setVisibility(View.GONE);
	        } else {
	        	loadingText.setText("Error loading gif.");
	        }
	    }
	}
}
