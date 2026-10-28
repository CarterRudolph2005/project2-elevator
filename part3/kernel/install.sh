#!/bin/bash
# Installs the compiled 6.16.12 kernel and makes GRUB boot it by default.
# Ubuntu's own kernel (7.0) is newer, so without this GRUB would keep booting that.
set -euo pipefail

cd /usr/src/linux-6.16.12
tail -3 ~/kernel-build.log
[ -f arch/x86/boot/bzImage ] || { echo "!! No bzImage: build not finished or failed"; exit 1; }

REL=$(make -s kernelrelease)
sudo make modules_install
sudo make install

TITLE="Ubuntu, with Linux $REL"
grep -q "'$TITLE'" /boot/grub/grub.cfg || { echo "!! GRUB entry '$TITLE' not found"; exit 1; }
sudo sed -i "s|^GRUB_DEFAULT=.*|GRUB_DEFAULT=\"Advanced options for Ubuntu>$TITLE\"|" /etc/default/grub
sudo update-grub

echo ">> Installed $REL. Now run: sudo reboot   then check: uname -r"
