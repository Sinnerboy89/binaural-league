# The Game
I'm going to attempt to build and test a VST plugin with each solution _as close to the source as possible_. Whilst the open-sourcedness of the solution isn't a major factor in scoring, it does help me understand what's happening under the hood. If I can't build the plugin myself, I'll test what they give me. Imma do all this on Windows 10 x64 and VS, like the rapscallion I am.

# Contenders
* Google Resonance Audio https://resonance-audio.github.io/resonance-audio/
* Facebook 360 Spatial Workstation powered by Audio360 https://facebookincubator.github.io/facebook-360-spatial-workstation/#
* Steam Audio https://valvesoftware.github.io/steam-audio/
* IEM Plug-in Suite https://plugins.iem.at/
* Audio Ease 360pan Suite https://www.audioease.com/360/
* 3D Tune-in http://www.3d-tune-in.eu/
* Sound Particles https://soundparticles.com/
* Nahimic for Gamers https://www.nahimic.com/gamers/
* Waves Nx https://www.waves.com/nx
* SPARTA (Aalto) http://research.spa.aalto.fi/projects/sparta_vsts/
* Timber Sfera https://www.timber3d.com/
* Dolby Atmos https://www.dolby.com/technologies/dolby-atmos/
* Embody https://embody.co/
* Superpowered Audio Spatializer and Ambisonics https://superpowered.com/3d-audio-spatializer-ambisonics-vr-audio
* RealSpace3D Audio (Visisonics) https://realspace3daudio.com/
* Auro-3D Engine https://www.auro-3d.com/system/engine/
* SPAT Revolution (IRCAM) https://www.flux.audio/project/spat-revolution/
* DTS:X https://www.whathifi.com/advice/dtsx-what-it-how-can-you-get-it

# Contender 1: Google Resonance Audio
There's a paper, hooray: https://www.aes.org/e-lib/browse.cfm?elib=20446

## Building
https://resonance-audio.github.io/resonance-audio/ - The landing page is nice and clear, but the "Getting Started" section only shows examples for common platforms or middleware, and it's actually quite difficult to navigate to the source repo from here. Once there though (https://github.com/resonance-audio/resonance-audio, or alternatively click the submodule in this repo) the SDK build instructions look fairly straightforward - oh, but wait - when I try the third-party cloning script, it tries to grab repos that no longer exist. Fixed promptly here https://github.com/resonance-audio/resonance-audio/issues/37. SDK build and testing runs fine, apart from a hidden dependency on Ogg Vorbis, which wasn't mentioned in the pre-requisites. The geometrical acoustics unit tests also seem to want "Embree 2", which I'm not gonna touch at this stage, so skipping those.

## Experiments

# Contender 2: Facebook 360 Spatial Workstation powered by Audio360
Yes, let's hope using this isn't as daunting as the name.

## Building
I download the most recent version of the Rendering SDK from here https://facebook360.fb.com/downloads/27514-2/ (a hefty 356MB, curious to know what the hell is in here). Upon inspection, I find it's actually over 1GB - and that's because they've distributed the SDK with every target built already. Why? Because I often find myself looking for .dylibs and Unreal Engine plugin binaries for Android on my Windows machine just to, y'know see how they're doing. Screw hard drives, amirite?
