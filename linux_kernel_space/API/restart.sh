./stop.sh
make && sudo insmod chess_api.ko && lsmod | grep "chess_api"
