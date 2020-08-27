# binaural-league
One fairly well-qualified enthusiast's journey of analysis and discovery through the myriad of spatial/3D/binaural audio solutions currently available, and their attempts to rank them with various metrics of goodness. Inspired by the overwhelming choice of vendors all claiming to do the exact same thing, motivated by the underwhelming results of everything I've seen and heard so far (so expect razor-sharp criticism and perhaps a little sarcasm).

# Rules
* All installing, building and experiments have been done on my HP Probook G6 w/ Ubuntu 20.04 (unless stated otherwise). If I hit a blocker because of this, then I may reluctantly move over to my Windows 10 install on same machine
* Whilst platform compatibility, wrappers and applied example scenarios are noted and appreciated - when it comes to judging the solution itself, only the core SDK/engine in its native language will be considered. I don't care that your software runs in Unity 5.4.03.30.g5.patch2017 with minimal effort or can render in real-time on your dog's dumbphone, if it sounds sh\*t on my ATH-40x headphones on my dev laptop with simple demo code that I've written with your SDK and documentation, and performs badly on reasonable spectral and listening tests, then that's down to the fundamental maths/physics/DSP gubbins in your "solution".
I may rethink this mindset once I start sifting through the mountain below.

# Contenders
* Google Resonance Audio https://resonance-audio.github.io/resonance-audio/
* Audio360 (by Facebook) https://facebookincubator.github.io/facebook-360-spatial-workstation/#
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

## Building
https://resonance-audio.github.io/resonance-audio/ - The landing page is nice and clear, but the "Getting Started" section only shows examples for common platforms or middleware, and it's actually quite difficult to navigate to the source repo from here. Once there though (https://github.com/resonance-audio/resonance-audio, or alternatively click the submodule in this repo) the SDK build instructions look fairly straightforward - oh, but wait - when I try the third-party cloning script, it tries to grab repos that no longer exist (e.g. https://bitbucket.org/eigen/eigen). Great. Cloning correct repos myself (Eigen and pffft) - added issue for this https://github.com/resonance-audio/resonance-audio/issues/37. SDK build and testing runs fine, apart from a hidden dependency on Ogg Vorbis, which wasn't mentioned in the pre-requisites. The geometrical acoustics unit tests also seem to want "Embree 2", which I'm not gonna touch at this stage, so skipping that.
