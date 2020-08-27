# binaural-league
One fairly well-qualified enthusiast's journey of analysis and discovery through the myriad of spatial/3D/binaural audio solutions currently available, and their attempts to rank them with various metrics of goodness. Inspired by the overwhelming choice of vendors all claiming to do the exact same thing, motivated by the underwhelming results of everything I've seen and heard so far (so expect razor-sharp criticism and perhaps a little sarcasm).

# The Game
I'm going to attempt to build a VST plugin with each solution _as close to the source as possible_. Whilst the open-sourcedness of the solution isn't a major factor in scoring, it does help me understand what's happening under the hood.

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
I download the most recent version of the Rendering SDK from here https://facebook360.fb.com/downloads/27514-2/ (a hefty 356MB, curious to know what the hell is in here).
