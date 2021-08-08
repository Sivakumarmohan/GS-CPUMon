#!/bin/bash

BRANCH_COMMAND=`git branch | sed -n '/\* /s///p'`
BRANCH=$BRANCH_COMMAND

if [ $BRANCH == 'main' ]
then
	DEV_VER=''
else
	GIT_HASH=`git rev-parse --short HEAD`
	DEV_VER=$BRANCH'_'$GIT_HASH
fi
export VERSION_SUFFIX=$DEV_VER
echo $VERSION_SUFFIX
