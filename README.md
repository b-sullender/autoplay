# autoplay
A program that detects CD/DVD/Blu-ray discs and runs a task window for Linux.

<img src="screenshot.png" alt="Screenshot" width="350">

For the best experience install all the recommended software:
```shell
sudo apt install -y sound-juicer brasero libbluray-bdj vlc
```

# Build and Install
```shell
mkdir build && cd build
cmake ..
make
sudo make install
```

# Enable and Start Autoplay Service
1. Reload daemon
   ```shell
   sudo systemctl daemon-reload
   ```
2. Enable autoplay to start at login
   - Enable for all users:
     ```shell
     sudo systemctl --global enable autoplay
     ```
   - Enable for only current user:
     ```shell
     systemctl --user enable autoplay
     ```
3. Start autoplay for current user
   ```shell
   systemctl --user start autoplay
   ```
   Alternatively, reboot the system.

# Building Debian Package
If you wish to build a Debian package, follow these steps:

1. **Ensure the `rules` file is executable**:  
   ```bash
   sudo chmod +x debian/rules
   ```

2. **Build the package**:  
   - Without signing:  
     ```bash
     sudo dpkg-buildpackage -uc -us
     ```
   - With signing (replace `<KEY_ID>` with your GPG key ID):  
     ```bash
     sudo dpkg-buildpackage -k<KEY_ID>
     ```
     *To list your GPG keys, use `gpg --list-keys`.*

3. **Cleanup build files**:  
   After building the package, remove temporary files with:  
   ```bash
   sudo debclean
   ```
   *This ensures your workspace is clean and free of leftover build artifacts.*

### Notes:
- Ensure you use `sudo` for all commands.
- Your newly created package files will be in the parent directory.
