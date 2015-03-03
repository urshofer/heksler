# heksler

Heksler is a simple video slicer designed for speed. After setting in- and out-points, heckler transcodes and exports the slice. Heksler also has a cut detection or shot transition feature.

Automatic Cinema Branch
-----------------------

Heklser work as a standalone application if compiled from the master branch. The automatic cinema branch includes functions to upload and tag the video slices directly into a automatic cinema server. For more information about the automatic cinema project refer to http://www.automatic-cinema.com.

Prerequisites
-------------

You need __ffmpeg__ on your system. Grab a copy here: http://ffmpegmac.net

Compatibility
-------------

Heksler runs on osx. The source is built with openFrameworks and uses ffmpeg to transcode the video, so it would be easy to port it to other operating systems.

Compilation
-----------

Please refer to the openFrameworks Documentation on how to compile a openFrameworks on OSX.
This repository is organized with submodules. In order to check it out, you have to do the following steps:

1. Check out the main repository:
    $ git clone https://github.com/urshofer/heksler heksler
2. Change into the checked out directory:
    $ cd heksler
3. Load all submodules:
    $ git submodule update --init --recursive

The xCode Project is in the directory __openframeworks/apps/myApps/heksler_osx__

FFMPEG
------

After the compilation, the xCode Project has a Copy Files build phase, where it tries to bundle your local ffmpeg installation with the newly built application. By default, xCode looks for ffmpeg in /usr/local/bin. You can change the path if you have ffmpeg installed locally or in a different place.
