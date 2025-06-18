## Nomad

## IP configuration:
### IP config:
- GATE: `192.168.20.1`
- DHCP: `192.168.20.1`
- MASK: `255.255.255.0`
- DHCP range: `192.168.20.100 - 250`
- STATIC addresses allowed : `192.168.20.2 - 99`
- VPN: `10.10.0.1/16`

### IP LIST:
- Router: `192.168.20.1`
- OBC: `192.168.20.2`, `10.10.0.6`
- KonARM (sdrac): `192.168.20.10`
- Livox 360: `192.168.20.20`


## Software Diagram

[Diagram](https://drive.google.com/file/d/13Ft7TU0lAM-w3s6LpOIg76KfH13r2Nhr/view?usp=sharing)

## Connecting to OBC

#### SSH - Basic Steps

1. Ask the admin to add your ssh key to the Nomad.

#### SSH - With Nomad WIFI

1. Connect to the Nomad network using the following credentials:

```bash
SSID: NomadT
Password: NomadToliwka
```

2. Connect over ssh using the following command:

```bash
ssh ubuntu@192.168.20.2
```

#### SSH - With VPN (Remote access)

1. Request config file to VPN from the admin.
2. Connect to the VPN network using the following command:

```bash
scripts/./vpn_connect.sh  <path to your vpn config>
```

3. Connect over ssh using the following command:

```bash
ssh ubuntu@10.10.0.6
```

### Connecting to OBC via front panel USB

1. Connect the USB cable to the front panel and OBC.

2. Open the terminal and find which port its connected to. Usually it is connected to /dev/ttyUSB0. To find the port use the following command:

```bash
ls /dev/ttyUSB*
```

3. Connect to the console using the following command [to exit the console press `Ctrl + a` then `Ctrl + x`]:

```bash
picocom -b 1500000  /dev/ttyUSB0
```

if you don't have picocom installed you can install it with the following command:

```bash
sudo apt install picocom
```

###### Issues:

if you have a problem with interface disconnecting or not being visible.
And your log looks something like this:

```bash
[  107.000000] usb 3-4.2: ch34x converter now attached to ttyUSB0
[  107.000000] usb 3-4.2: usbfs: interface 0 claimed by ch34x while 'brltty' sets config #1
[  107.000000] ch34x ttyUSB0: ch34x converter now disconnected from ttyUSB0
[  107.000000] ch34x 3-4.2:1.0: device disconnected
```

You can fix it by removing brltty package with the following command:

```bash
sudo apt remove brltty
```

[more info about the problem](https://github.com/juliagoda/CH341SER/issues/18)

### Real time problem scheduler

When you start ros yut get error that scheduling is not configured.
[link](https://control.ros.org/master/doc/ros2_control/controller_manager/doc/userdoc.html)
