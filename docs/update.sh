#!/bin/sh

git fetch

LOCAL=$(git rev-parse @)
REMOTE=$(git rev-parse @{u})
BASE=$(git merge-base @ @{u})

if [ $LOCAL = $REMOTE ]; then
    /bin/true
elif [ $LOCAL = $BASE ]; then
    echo "Need to pull"
    git pull
    make clean
    # The environment variable added here adds bottom left download pane.
    MAMEDEV=1 make site
elif [ $REMOTE = $BASE ]; then
    echo "Need to push"
else
    echo "Diverged"
fi
