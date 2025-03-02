sudo ./mount-hdc
sudo cp ./linux-0.11/iam.c ./hdc/iam.c
sudo cp ./linux-0.11/whoami.c ./hdc/whoami.c
sudo umount hdc
./run
