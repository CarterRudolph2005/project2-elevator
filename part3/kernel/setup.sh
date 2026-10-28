#!/bin/bash
# Downloads Linux 6.16.12, adds the elevator syscalls (548-550),
# configures a small build, and starts compiling in the background.
# Run from anywhere: bash part3/kernel/setup.sh
set -euo pipefail

VER=6.16.12
KDIR=/usr/src/linux-$VER
HERE=$(cd "$(dirname "$0")" && pwd)

# 1. Build tools: install only the ones that are missing
need=()
for p in build-essential flex bison bc libelf-dev libssl-dev zstd cpio; do
	dpkg -s "$p" >/dev/null 2>&1 || need+=("$p")
done
if [ ${#need[@]} -gt 0 ]; then
	echo ">> Missing build tools: ${need[*]}"
	sudo apt-get update
	sudo apt-get install -y "${need[@]}"
else
	echo ">> All build tools already installed"
fi

# 2. Kernel source in /usr/src, owned by us, linked from ~
if [ ! -d "$KDIR" ]; then
	echo ">> Downloading linux-$VER"
	cd /usr/src
	sudo curl -fLO "https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-$VER.tar.xz"
	sudo tar -xf "linux-$VER.tar.xz"
	sudo rm "linux-$VER.tar.xz"
	sudo chown -R "$USER:$USER" "$KDIR"
fi
ln -sfn "$KDIR" ~/linux-$VER
cd "$KDIR"

# 3. syscalls/ directory
mkdir -p syscalls
cp "$HERE/../syscalls.c" syscalls/syscalls.c
echo 'obj-y := syscalls.o' > syscalls/Makefile

# 4. Syscall table
TBL=arch/x86/entry/syscalls/syscall_64.tbl
if ! grep -q start_elevator "$TBL"; then
	printf '548\tcommon\tstart_elevator\t\tsys_start_elevator\n' >> "$TBL"
	printf '549\tcommon\tissue_request\t\tsys_issue_request\n' >> "$TBL"
	printf '550\tcommon\tstop_elevator\t\tsys_stop_elevator\n' >> "$TBL"
fi

# 5. Prototypes, inserted right before the last #endif of syscalls.h
H=include/linux/syscalls.h
if ! grep -q sys_start_elevator "$H"; then
	cat > /tmp/elevator_protos <<'EOF'
asmlinkage long sys_start_elevator(void);
asmlinkage long sys_issue_request(int start_floor, int destination_floor, int type);
asmlinkage long sys_stop_elevator(void);
EOF
	L=$(grep -n '^#endif' "$H" | tail -1 | cut -d: -f1)
	sed -i "$((L - 1))r /tmp/elevator_protos" "$H"
fi

# 6. Top-level Makefile: build syscalls/ into the kernel
sed -i 's|^core-y[[:space:]]*:=[[:space:]]*$|core-y\t\t:= syscalls/|' Makefile

# Verify all four edits before spending hours compiling
grep -q 'core-y.*syscalls/' Makefile || { echo "!! Makefile edit failed"; exit 1; }
grep -q '^550.*stop_elevator' "$TBL" || { echo "!! syscall table edit failed"; exit 1; }
grep -q sys_issue_request "$H" || { echo "!! syscalls.h edit failed"; exit 1; }
[ -f syscalls/syscalls.c ] || { echo "!! syscalls.c missing"; exit 1; }
echo ">> Syscall edits verified"

# 7. Config: start from the running kernel, keep only loaded modules,
#    drop Ubuntu's certificates and debug info (slow + needs extra tools)
cp "/boot/config-$(uname -r)" .config
make olddefconfig
yes '' | make localmodconfig
scripts/config --set-str SYSTEM_TRUSTED_KEYS "" \
	--set-str SYSTEM_REVOCATION_KEYS "" \
	--disable DEBUG_INFO_BTF \
	--disable DEBUG_INFO_DWARF_TOOLCHAIN_DEFAULT \
	--disable DEBUG_INFO_DWARF4 \
	--disable DEBUG_INFO_DWARF5 \
	--enable DEBUG_INFO_NONE
make olddefconfig

# 8. Compile in the background so it survives closing the console
nohup make -j"$(nproc)" > ~/kernel-build.log 2>&1 &
echo ">> Build started (pid $!). Watch it with: tail -f ~/kernel-build.log"
echo ">> When it finishes, run: bash $HERE/install.sh"
