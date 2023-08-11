gnome-terminal -- bash -c "cd ../prgm/finger_pos/build;./finger_pos 4;exec bash -i"
gnome-terminal -- bash -c "cd ../build;exec bash -i"
ssh -t adev@192.168.127.2 "screen -c .tscreenrc;bash -l"