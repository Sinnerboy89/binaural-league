# The Game
I'm going to attempt to build and test a VST plugin with each solution _as close to the source as possible_. Whilst the open-sourcedness of the solution isn't a major factor in scoring, it does help me understand what's happening under the hood. If I can't build the plugin myself, I'll test what they give me. Imma do all this on Windows 10 x64 and VS, like the rapscallion I am.

# Contenders
In no particular order:
1. Google Resonance Audio https://resonance-audio.github.io/resonance-audio/
2. Facebook 360 Spatial Workstation powered by Audio360 https://facebookincubator.github.io/facebook-360-spatial-workstation/#
3. Steam Audio https://valvesoftware.github.io/steam-audio/
4. IEM Plug-in Suite https://plugins.iem.at/
5. Audio Ease 360pan Suite https://www.audioease.com/360/
6. 3D Tune-in http://www.3d-tune-in.eu/
7. Sound Particles https://soundparticles.com/
8. Nahimic for Gamers https://www.nahimic.com/gamers/
9. Waves Nx https://www.waves.com/nx
10. SPARTA (Aalto) http://research.spa.aalto.fi/projects/sparta_vsts/
11. Timber Sfera https://www.timber3d.com/
12. Dolby Atmos https://www.dolby.com/technologies/dolby-atmos/
13. Embody https://embody.co/
14. Superpowered Audio Spatializer and Ambisonics https://superpowered.com/3d-audio-spatializer-ambisonics-vr-audio
15. RealSpace3D Audio (Visisonics) https://realspace3daudio.com/
16. Auro-3D Engine https://www.auro-3d.com/system/engine/
17. SPAT Revolution (IRCAM) https://www.flux.audio/project/spat-revolution/
18. DTS:X https://www.whathifi.com/advice/dtsx-what-it-how-can-you-get-it

# Contender 1: Google Resonance Audio
There's a paper, hooray: https://www.aes.org/e-lib/browse.cfm?elib=20446

The source code is INCREDIBLY well organised and documented. I can read it like a book, and supplemented by the paper there's not a lot of mystery here, which is great. The main takeaways so far:
* Binaural rendering is via spherical harmonic convolution using HRIRs from the SADIE database (https://www.york.ac.uk/sadie-project/database.html). Selection appears to be determined by user only - no fancy pinna imaging ML or anthropometric values
* AmbiX format used as codec/intermediate format between mono/stereo/surround/binaural. Looks like you can crank this baby all the way up to 3rd order. Phwoooaar
* Decent consideration over source directivity, occlusion and spread
* Near-field (< 1 metre) is considered, but only with basic heuristic gain modification
* Room modelling included
* Thoughtful use of lock-free, low-latency and thread-safe audio processing
* Core DSP well cited and derived from fundamentals - there's very little "fudge" here. Biquad class is lovely. Instabilities cared for but one wonders if SVFs might have helped out?
* Nil consideration of headphone equalisation

Not incredibly clear if this is _exactly_ what Youtube uses for spatialisation, or if there's a closed source version too with extra gubbins. It would probably work in Google's favour if someone clarified this ("oh, yeah that's out of date now - the new closed version is waaaaaay better trust me etc.").

## Building
https://resonance-audio.github.io/resonance-audio/ - The landing page is nice and clear, but the "Getting Started" section only shows examples for common platforms or middleware, and it's actually quite difficult to navigate to the source repo from here. Once there though (https://github.com/resonance-audio/resonance-audio, or alternatively click the submodule in this repo) the SDK build instructions look fairly straightforward - oh, but wait - when I try the third-party cloning script, it tries to grab repos that no longer exist. Fixed promptly here https://github.com/resonance-audio/resonance-audio/issues/37. SDK build and testing runs fine, apart from a hidden dependency on Ogg Vorbis, which wasn't mentioned in the pre-requisites. The geometrical acoustics unit tests also seem to want "Embree 2", which I'm not gonna touch at this stage, so skipping those.
Hit a snag whilst attempting to build VST Monitor plugin on Windows - issue submitted here: https://github.com/resonance-audio/resonance-audio/issues/39

## Experiments

# Contender 2: Facebook 360 Spatial Workstation powered by Audio360
Yes, let's hope using this isn't as daunting as the name.

## Building
I download the most recent version of the Rendering SDK from here https://facebook360.fb.com/downloads/27514-2/ (a hefty 356MB, curious to know what the hell is in here). Upon inspection, I find it's actually over 1GB - and that's because they've distributed the SDK with every target built already.
