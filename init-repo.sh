#!/bin/bash
# This setups up the submodules and puts them on the master branch
git submodule update --init --rebase
git submodule foreach 'git checkout master'
