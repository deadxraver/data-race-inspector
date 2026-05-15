# Data Race Inspector

A kernel module to detect data races on block IO.
Kernel version 6.12.73 and instructions are provided for Debian 13.

## Setup VM

[instructions](https://github.com/deadxraver/device-mapper-proxy#setup-vm-if-needed)

## Install dependencies

```bash
sudo apt-get update
sudo apt install build-essential linux-headers-`uname -r` dmsetup
```

## Build & Test

```bash
make
sudo insmod source/dri.ko
```

If an error message
`insmod: ERROR: could not insert module source/dri.ko: Unknown symbol in module`
appears, insert `dm_mod` manually:

```bash
sudo modprobe dm_mod
```

To create a data race, run script `test.sh` and check kernel journal,
a warning should appear there.

```bash
sudo bash test.sh
sudo dmesg | grep 'data race'
# [  309.348638] [dri]: data race, sectors [272, 287]
```

## Cleanup

To remove module first delete the devices if
you have created any:

```bash
sudo dmsetup remove <dev_name>
```

After that remove the module:

```bash
sudo rmmod dri
```

And also object files can be removed by running `make clean`.
