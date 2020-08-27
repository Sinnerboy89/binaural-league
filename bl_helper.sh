# update submodules
git submodule update

# alias
git config --global alias.update '!git pull && git submodule update --init --recursive'

# update everything (alias set up)
git update