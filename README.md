userspace-vfd
=============

Userspace program for controlling Shuttle VFD
This (20x1 characters) LCD device is shipped with Shuttle models: SD365GM, SG33G5M.

This is an old source I've recovered from my old HDD. I was too lazy to create an configure.ac .. ;)

== Compilation ==

Requires *libusb-dev* (usb.h)

$ make

== Usage ==

As this sends commans to USB device, I'll probably need to be root.

// Display all icons and fill with text
# ./userspace-vfd --test

// Display message (truncatedi if greater than 20 characters)
# ./userspace-vfd -m 'Hello World!'

// Clear display (test and icons)
# ./userspace-vfd --clean

// Light one or several icons. Possibles values:
// clk, rad, mus, cd, tv, cam, rew, rec, play, pause, stop, ff, rev, rep, mute, vol0, vol1, ..., vol12, all
# ./userspace-vfd -i 'rec,play'
