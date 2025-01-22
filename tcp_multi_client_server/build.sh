#!/bin/bash

echo "Building ... "

# Use getopt to handle options
options=$(getopt -o "SClhr" -l "server,client,help,run" -n "$0" -- "$@")

# Check if getopt successfully parsed the arguments
if [ $? != 0 ]; then
    echo "Error in command line parsing. Exiting..."
    exit 1
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
    gcc tcp_clnt.c services.c network.c -o Cl1 # Replace with your actual client source file
    gcc tcp_clnt.c services.c network.c -o Cl2 # Replace with your actual client source file
    gcc tcp_clnt.c services.c network.c -o Cl3 # Replace with your actual client source file
}

# Function to launch programs in tmux
run_tmux() {
    echo "Running programs in tmux..."

    # Define the programs you want to run
    PROGRAMS=("./serM" "./lsSer" "./catSer" "./elapSer" "./dateSer" "./Cl1" "./Cl2" "./Cl3") # Add your commands here

    # Name of the tmux session
    SESSION_NAME="multi_program_runner"

    # Start a new tmux session
    tmux new-session -d -s "$SESSION_NAME"

    # Run the first program in the first (default) pane
    tmux send-keys "${PROGRAMS[0]}" C-m

    # Loop through the rest of the programs
    for ((i = 1; i < ${#PROGRAMS[@]}; i++)); do
      # Split the current pane and run the next program
      tmux split-window -v
      tmux send-keys "${PROGRAMS[i]}" C-m
      
      # Select the next pane for splitting
      tmux select-layout tiled
    done

    # Set the layout to tiled for a better view
    tmux select-layout tiled

    # Attach to the session
    tmux attach-session -t "$SESSION_NAME"

    echo "Programs launched in tmux."
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
    -r|--run)
      RUN_TMUX=true
      shift
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
