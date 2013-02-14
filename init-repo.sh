#!/bin/bash
# This setups up the submodules and puts them on the master branch
git submodule init
git submodule update
git submodule foreach 'git checkout master'
