#!/bin/bash


echo "Building ... "

options=$( getopt -l "server,client,help" -o "SerClh" -a -- $@ )
eval set -- $options
while true; do
	case "$1" in
		-ser|--server) echo "Building Server program $2" ; shift 2 ;;
		-cl|--client) eco "building Client program "; shift ;;
		--) shift; break ;; # the end of the options to be parsed
		*) echo "error invalid argument $1"; exit 0;;
	esac
done

