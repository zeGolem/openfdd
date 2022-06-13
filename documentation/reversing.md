# Reverse engineering guide for USB RGB mice and keyboards

This guide will be mostly focused on getting a Windows VM with USB packet capture
up and running.  
This guide assumes you're running a Linux host, and wasn't tested for any other
environnement.

## 1. Windows VM

First, install Windows (preferably 10) in a VM using VirtualBox. Other virtualization
technologies may work, but only VirtualBox was tested.  
I won't detail this step much, because there are many in-depth guides for this already
and it isn't relevant to the other steps.

**NOTE**: Make sure your user is in the `virtualbox` group, and that you have the
VirtualBox kernel drivers installed, to make sure everything works well!

Then, install whichever the official software from the device's manufacturer in the VM.  
Your USB device won't be detected yet.

## 2. USB forwarding

Boot up your VM, and in the VirtualBox menubar, select "Devices", then "USB", and check the
box corresponding to the device you want to reverse engineer.

**NOTE**: If nothing shows up in the USB menu, make sure your user is part of the
`virtualbox` group, and that the kernel driver is correctly installed. Try restarting your
Linux host if you just updated your kernel.

**!! WARNING !!**: Checking the box will forward *all* the device's inputs to the VM. Make
sure you have a secondary mouse/keyboard if that's what you're reversing. If you get
stuck in the VM, the best solution is shutting it off. If your keyboard is stuck, press
Super (Windows) and R at the same time, then type `shutdown -s -t 0` and press "Enter".
If your mouse is stuck, go to the bottom left corner of the screen, click the Windows
icon, click the power icon, then click "Shutdown". If you can't see your mouse cursor
(common issue), use the selection highlight to navigate the menus.

**TIP**: If your cursor doesn't show up, try disabling VirtualBox's mouse integration,
from the menubar, select "Input" then untick "Mouse integration".

You can now check that the device's configuration software correctly detects the device.

## 3. Packet capture with Wireshark

We're now going to set up Wireshark for capturing USB traffic. Once Wireshark is installed
on your system, add your user to the `wireshark` group (you'll need to log out and back in
for this to take effect) and load the `usbmon` kernel module.

```bash
sudo modprobe usbmon
```

Now, open Wireshark. You should see a list of packet capture sources, including some
starting with `usbmon`. To know which one to use, you'll need to check which bus
the device you want to reverse engineer is using. For that use `lsusb`. Find the line
corresponding to your device, and check the number next to "Bus".

In Wireshark, select the `usbmon` with the same number. So, if your device has Bus 003,
pick `usbmon3` in Wireshark.

You'll now see all the traffic going through that USB bus.

### 3.1 Working with Wireshark

The Wireshark window has 3 main views: The topmost one lists all the packets being sent
and received, the middle one shows the decoded data from the currently highlighted packet
in the top view, and the bottom most one shows the raw data of the packet.

One of the first important things to do is to find out the address of your device. There
isn't a one-size-fits-all method for this, but what you mostly need to do is identify
the address ("x.y.z", with x, y and z all integers) from the "Source" or "Destination"
columns of the top view. Something that often helps is to do a specific action in the
device's configuration software in the VM, and stop the capture right after. Do this a few
times, and try to see if there are recurring packets, starting from the bottom (latest packet).

Once you found one specific packet, get its address from the "Source" or "Destination" columns.
Then, use a filter to only see packets for that address: In the filter rown above the top view,
type something like:

```
usb.addr == "3.7.0"
```

Then, do some actions in the configuration software to check you got the right address.

## 4. Do the reversing

At this point, you've got everything you need to start reversing! Just start messing around
with the configuration tool, and see what USB packets it produces. Check the raw data, and
try to understand how changing different values impacts it. You can click on "Setup Data" in
the middle view to highlight the important part of the packet, the rest was decoded by Wireshark,
and is just standard USB stuff you don't need to worry about for now.

Some tips for reversing:

* A good "first packet" is the "save" packet. There usually is a "save" or "confirm" button in
  the tool, and it should produce a short packet (usually 1 byte on SteelSeries devices) that's
  easy to spot and understand.
* Values don't always have the same scale in the configuration tool and in the packet being sent.
  Try to use the minimum and maximum values of sliders to find out the scale used.
* You don't need to understand every packet to make a functional driver, especially when starting
  work on a new device. It's fine to copy parts of packets the official software produced until
  you figure out what each value does

And most importantly, take your time and try to be creative! This is mostly guess work now, there is
no wrong way of going about it
