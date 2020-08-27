# update submodules
git submodule update

# alias
git config --global alias.update '!git pull && git submodule update --init --recursive'

# update everything (alias set up)
git update


# --------------------------------------------
# --------- Google Resonance Audio -----------
# --------------------------------------------

git submodule add https://github.com/resonance-audio/resonance-audio.git resonance

# build/clone third-party deps
cd /home/chrisb/binaural-league/resonance/third_party
./clone_core_deps.sh

# build the API
cd ..
./build.sh -t=RESONANCE_AUDIO_API

# run unit tests
./build.sh -t=RESONANCE_AUDIO_TESTS