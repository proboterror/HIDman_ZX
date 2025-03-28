# HIDman
## USB HID to XT / AT / PS/2 / Serial converter

HIDman is an open source device to allow the use of modern USB keyboards and mice on legacy PCs.

# Features

* (should) Support the majority of standard USB keyboards and mice, including ones that use wireless dongles.
* Also supports (some) USB Game controllers - buttons/axes are mapped to keypresses or mouse actions

# Quickstart Guide

## USB Connection

You can connect many different combinations of USB devices to HIDman.

The most obvious being to connect a keyboard to one USB port, and a mouse to the other one :

<img src=/images/usbsetup1.svg width=300/>

Or, you could perhaps connect a wireless keyboard+mouse dongle to one port, and a game controller to the other :

<img src=/images/usbsetup2.svg width=500/>

Hub support can be hit-and-miss. This is (mostly) not HIDman's fault - many modern hubs don't support low-speed USB devices properly.

# Firmware Update

Firmware development is continuing, so if you have problems it's always worth updating to the latest version.

If you're on windows, first install WCH's ISP tool - https://www.wch.cn/downloads/WCHISPTool_Setup_exe.html
If on Mac or Linux, install ch55xtool : https://github.com/MarsTechHAN/ch552tool

The next step is to put the HIDman in firmware update mode.

1. Disconnect everything from HIDman, including all USB devices and PCs. (failure to do this may result in damage to HIDman, your PC, or both).
2. Hold down HIDman's ⏻ power button.
3. Use a USB A-to-A cable to connect HIDman's LOWER USB port to a USB port on your modern PC.

After that, update instructions will depend on your operating system.

## Windows

Follow the instructions in this diagram :

<img src=/images/firmware2.svg width=800/>


## Linux or Mac

Pass the -f parameter to ch55xtool specify the firmware file to load. For example :

```
python3 ch55xtool.py -f hidman_axp_v1.1.bin
```

# Advanced Setups

## Splitter

Another way to take advantage of the Combined PS/2 port functionality is to use a widely-available PS/2 splitter :

<img src=/images/splitter.svg width=500/>

This also allows neater cabling, as you could connect both keyboard and mouse ports to the rear, and not have to connect to the front mouse port.


## Technical description

The HIDman is based around the CH559 from WCH, a remarkably flexible chip with **two** USB HOST ports. This makes it ideal for our purposes.

The code is forked from atc1441's excellent repository - https://github.com/atc1441/CH559sdccUSBHost

PCB and enclosure was designed in KiCad - source files are in the hardware directory.

Development is very active but it is usable in its current state.
