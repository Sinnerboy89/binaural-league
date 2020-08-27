Audio360 FFmpeg Decoder Example
================================

This example projects uses lib ffmpeg to demux, decode and spatialise a mkv (with 10 channels of spatial audio in the FB360 format).

Pre-requisites:
---------------

1. FFmpeg Libraries

On Mac the libraries can be installed using homebrew. https://brew.sh/
Run `brew install ffmpeg` from the Terminal to install ffmpeg.

On Windows, the libraries can be downloaded from https://ffmpeg.zeranoe.com/builds/. Ensure you select 64-bit Shared (from the Linking options).
Once downloaded, unzip and copy the folder named "bin" into the same directory as this project's built executable (FFmpegDecoder.exe).


Building
--------

On Mac, open the Xcode project in the library named "Xcode". Build and run the project to hear spatialised audio.
By default, the built executable plays back the mkv file in the Media folder.
