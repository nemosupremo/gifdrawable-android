# GifDrawable
April 13, 2013

## Overview
GifDrawable is a BitmapDrawable class extended to provide animated gif capabilities to Android, specifically an ImageView. Natively the only way AFAIK to get animated gif in Android is to either user
a WebView, or the poorly documented Movie class. A WebView is overkill, and the Movie class isn't very
flexible.

The GifDrawable uses the JNI so that GIF frames can be drawn in sub 10ms. A attempted a pure Java
implementation and it was not very fast.

## Libraries
The GIFLIB <http://sourceforge.net/projects/giflib/> library is included.

You will need the latest version of the Android NDK to build <http://developer.android.com/tools/sdk/ndk/index.html>.

## Building

1. Copy the files in /jni to your project.
2. Copy the `com.droidtools.android.graphics.GifDrawable` to your project.
3. `cd` into the `jni` directory and run `ndk-build`

Once done, you should be set up to use the class with your project.

## Example
See the `com.droidtools.example.MainActivity` for an example of using the project.

## Supported Devices
Since the `<android/asset_manager.h>` is only supported in API Level 9, `gifFromAsset` only supports API Level 9 (Gingerbread). However if you remove those calls, then API Level 5 (Eclair) will be supported.

## Bugs

1. Large gifs can crash
2. Some dispose methods are unsupported.
3. `com.droidtools.android.graphics.GifDrawable.setLevel()` always returns true even if the gif did not need the change frames.

## License
Copyright (c) 2013 nimiwaribokoj@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.