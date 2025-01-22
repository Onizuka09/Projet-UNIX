#!/bin/bash

echo "Building ... "

# Use getopt to handle options
options=$(getopt -o "SClh" -l "server,client,help" -n "$0" -- "$@")

# Check if getopt successfully parsed the arguments
if [ $? != 0 ]; then
    echo "Error in command line parsing. Exiting..."
    exit 1
fi

eval set -- "$options"

while true; do
  case "$1" in
    -S|--server)
      echo "Building Server program"
      gcc udp_serv.c logger.c -o serv  # Replace with your actual server source file
      shift
      ;;
    -C|--client)
      echo "Building Client program"
      gcc udp_clnt.c logger.c -o clnt # Replace with your actual client source file
      shift
      ;;
    -h|--help)
      echo "Usage: $0 --server|--client"
      echo "--server: Build the server program"
      echo "--client: Build the client program"
      exit 0
      ;;
    --)
      shift
      break
      ;;
    *)
      echo "error invalid argument $1"
      exit 1
      ;;
  esac
done

echo "Build completed."
