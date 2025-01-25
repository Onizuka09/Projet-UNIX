#!/bin/bash

echo "Building ... "
# Use getopt to handle options
options=$(getopt -o "SClhr" -l "server,client,help,run" -n "$0" -- "$@")

# Check if getopt successfully parsed the arguments
if [ $? != 0 ]; then
    echo "Error in command line parsing. Exiting..."
    exit 1
fi
if [ -z $LD_LIBRARY_PATH ]; then
  export LD_LIBRARY_PATH=$(pwd)/gui_cl/GooeyGUI/lib/
else 
  export LD_LIBRARY_PATH=$(pwd)/gui_cl/GooeyGUI/lib/:$LD_LIBRARY_PATH
fi
eval set -- "$options"

# Function to build server
build_server() {
    echo "Building the main Server program & the services"
    gcc tcp_ser.c services.c network.c -o serM  # Replace with your actual server source file
    gcc tcp_cat_ser.c services.c network.c -g  -o catSer
    gcc tcp_elap_ser.c services.c network.c -g  -o elapSer
    gcc tcp_ls_ser.c services.c network.c -g  -o lsSer
    gcc tcp_date_ser.c services.c network.c -g  -o dateSer
}

# Function to build client
build_client() {
    echo "Building Client programs"
   
    gcc tcp_clnt.c services.c network.c ui.c -o gui_cl -I./GooeyGUI/include/ -I./GooeyGUI/internal/ -L./GooeyGUI/lib/ -lGooeyGUI -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -g3 -fsanitize=address 
}


# Loop through the parsed arguments
while true; do
  case "$1" in
    -S|--server)
      build_server
      shift
      ;;
    -C|--client)
      build_client
      shift
      ;;
    -h|--help)
      echo "Usage: $0 --server|--client [--run]"
      echo "--server: Build the server program"
      echo "--client: Build the client program"
      echo "--run: Run the programs in tmux after building"
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

# If the --run flag is set, run the programs in tmux
if [ "$RUN_TMUX" = true ]; then
    run_tmux
fi

echo "Build completed."
