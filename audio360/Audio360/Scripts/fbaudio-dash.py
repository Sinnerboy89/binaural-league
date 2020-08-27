#!/usr/bin/env python

# MIT License
# Copyright (c) 2018-present, Facebook, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# This script uses FFmpeg and shaka packager to create a DASH manifest
# and assets from a FB360 Matroska file created with the FB360 Encoder app.
#
# The packaging command will create a DASH manifest, a webm container for audio and
# a mp4 container for video. If necessary, you can always swap the video out for 
# another version/codec you might want to use.
#
# Requirements:
# 1. FFmpeg (either in the system PATH or use -f to specify a custom path)
# 2. Shaka Packager (https://github.com/google/shaka-packager) (Use -p to specify the path)
#
# Usage:
# python fbdaudio-dash.py -i INPUT_MKV -p PATH_TO_SHAKA_PACKAGER -o OUTPUT_MANIFEST
# python fbdaudio-dash.py -i input.mkv -p /path/to/shaka/packager -o manifest.mpd 

import logging as log
import os
import sys
import subprocess
import shutil
import tempfile
import argparse

log.basicConfig(
        level=log.DEBUG,
        format='%(asctime)s %(levelname)-8s: %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S %z')

def testCmd(cmd):
	""" Returns True if a command executes """
	if not cmd:
		return False
	try:
		subprocess.check_output(cmd)
	except:
		return False
	return True

def system(argv, exit=True):
	""" Execute, exit the process with the same error code if it fails """
	argv = [str(x) for x in argv]
	log.info('executing %s', argv)
	res = subprocess.call(argv)
	if not exit:
		return res
	if res != 0:
		log.error('Failed and exiting with ' + str(res))
		sys.exit(res)

def outputFile(outdir, file, ext = None):
	""" Join a path to a file, optionally add an extension """
	name = os.path.basename(file)
	if ext:
		name = os.path.splitext(name)[0] + ext
	return os.path.join(outdir, name)

def demuxFile(ffmpeg, file, outdir):
	""" Demux audio and video into new containers """
	""" We need to demux first to get shaka packager to play ball """
	outAudio = outputFile(outdir, file, '.webm')
	outVideo = outputFile(outdir, file, '.mp4')
	system([ffmpeg,
		'-y', 
		'-i', file, 
		'-c:a', 'copy', 
		'-vn',
		outAudio])
	system([ffmpeg, 
		'-y',
		'-i', file, 
		'-c:v', 'copy',
		'-movflags', 'faststart', 
		'-an',
		outVideo])
	return [outVideo, outAudio]

def runPackager(packager, videoIn, audioIn, manifest):
	""" Generate a dash manifest and assets using shaka packager """
	outPath =  os.path.split(manifest)[0]
	outVideo = outputFile(outPath, videoIn, '.mp4')
	outAudio = outputFile(outPath, audioIn, '.webm')
	return system([packager, 
				'in=' + videoIn + ',stream=video,output=' + outVideo,
				'in=' + audioIn + ',stream=audio,output=' + outAudio, 
				'--mpd_output', manifest])

def generateDash(opts):
	ffmpeg = opts.ffmpeg[0] if opts.ffmpeg else 'ffmpeg'
	packager = opts.packager[0] if opts.packager else 'packager'

	if not testCmd([ffmpeg, '-version']):
		log.error('FFmpeg path is not valid: ' + ffmpeg)
		exit(1)

	if not testCmd([packager]):
		log.error('shaka packager path is not valid: ' + packager)
		exit(1)

	tempDir = opts.temp[0] if opts.temp else tempfile.gettempdir()

	demuxed = demuxFile(ffmpeg, opts.input[0], tempDir)
	return runPackager(packager, 
					demuxed[0], 
					demuxed[1], 
					opts.output_manifest[0])

p = argparse.ArgumentParser(description="Uses FFmpeg and shaka packager to create a DASH manifest " +
							"and assets from FB360 MKV files that are created with the FB360 Encoder app. "+
							"Requirements: "+
							"1. FFmpeg (either in the system PATH or use -f to specify a custom path). "+
							"2. Shaka Packager (https://github.com/google/shaka-packager) (Use -p to specify the path)",
							usage="\npython fbdaudio-dash.py -i INPUT_MKV -p PATH_TO_SHAKA_PACKAGER -o OUTPUT_MANIFEST\n"+
							"python fbdaudio-dash.py -i input.mkv -p /path/to/shaka/packager -o manifest.mpd")
p.add_argument('-i', '--input', help='Input FB360 MKV file', nargs=1)
p.add_argument('-f', '--ffmpeg', help='Optional path to FFmpeg executable.', nargs=1, default='')
p.add_argument('-p', '--packager', help='Optional path to Shaka Packager.', nargs=1, default='')
p.add_argument('-t', '--temp', help='Optional path to a temp dir.', nargs=1, default='')
p.add_argument('-o', '--output_manifest', help='Output path and name for the manifest', nargs=1)

if __name__ == '__main__':
    generateDash(p.parse_args())
