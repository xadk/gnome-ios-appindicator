#!/bin/bash

nodemon -e 'json,sh,c,h' --exec './build.sh && ./dist/iosindicator' --signal SIGHUP
