# autoplay
A program that detects CD/DVD/Blu-ray discs and runs a task window for Linux.

# Build and Install
```shell
mkdir build && cd build
cmake ..
make
sudo make install
```

# Starting autoplay service
```shell
systemctl --user daemon-reload
systemctl --user enable autoplay
systemctl --user start autoplay
systemctl --user status autoplay
```
