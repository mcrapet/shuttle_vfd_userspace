userspace-vfd
=============

Userspace program for controlling Shuttle VFD
This (20x1 characters) LCD device is shipped with Shuttle models: SD365GM, SG33G5M.

This is an old source I've recovered from my old HDD. I was too lazy to create an configure.ac .. ;)

## Compilation

Requires *libusb-dev* package (usb.h)

```shell
$ make
```

## Usage

As this sends commands to USB device, You'll probably need to be root.

```shell
# Display all icons and fill with text
./userspace-vfd --test

# Display message (truncatedi if greater than 20 characters)
./userspace-vfd -m 'Hello World!'

# Clear display (test and icons)
./userspace-vfd --clean

# Light one or several icons. Comma separated list.
# Possibles values: clk, rad, mus, cd, tv, cam, rew, rec, play, pause,
# stop, ff, rev, rep, mute, vol0, vol1, vol2, ..., vol11, vol12, all
./userspace-vfd -i 'rec,play'

# Builtin clock display
./userspace-vfd --clock
```
