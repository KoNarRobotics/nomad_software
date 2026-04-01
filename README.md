# Nomad

## Services

### Installing services

To install the services on the rover, run the following command:

```bash
./scripts/./make_service.sh
```

### Services running on the rover:

- nomad_base.service (diffbot)
- nomad_nav.service (navigation)

### You can see status using:

- To see their status use:

```bash
sudo systemctl status nomad_nav.service
```

- To view log messages live use:

```bash
sudo journalctl -u nomad_nav.service -f
```

## IP configuration:

### IP config local:

- GATE: `192.168.20.1`
- DHCP: `192.168.20.1`
- MASK: `255.255.255.0`
- DHCP range: `192.168.20.100 - 250`
- STATIC addresses allowed : `192.168.20.2 - 99`

### IP config VPN:

- GATE: `10.10.0.1`
- DHCP: `10.10.0.1`
- MASK: `255.255.0.0`
- DHCP: `None`
- STATIC addresses allowed : `10.10.0.2 - 10.10.255.254`

### IP LIST:

local IP [, vpn IP]

- Router: `192.168.20.1`
- OBC: `192.168.20.2`, `10.10.0.6`
- KonARM (sdrac): `192.168.20.10`
- Livox 360: `192.168.20.20`
- PC Rtx: `192.168.20.30`

## Software Diagram

DrawIO amazing software diagram presenting how its all connected:
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


### Streaming video from OBC camera



ffmpeg -f v4l2 -pixel_format mjpeg -framerate 30 -s 1280x720 -i /dev/video0 -preset ultrafast -tune zero_latency -b:v 100M -maxrate 120M -bufsize 600M -f mpegts -omit_video_pes_length 1 udp://10.10.0.8:2000

ffplay -fflags nobuffer -flags low_delay -probesize 32 -analyzeduration 1 -strict experimental -framedrop -f mpegts -vf setpts=0 udp://10.10.0.6:2000


### Issues:

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

## Navigation base on

https://docs.nav2.org/setup_guides/index.html#table-of-contents

## Sensor Fusion based on

https://github.com/methylDragon/ros-sensor-fusion-tutorial/blob/master/01%20-%20ROS%20and%20Sensor%20Fusion%20Tutorial.md

### Real time problem scheduler

When you start ros and get error that scheduling is not configured.
[Then check this out ](https://control.ros.org/master/doc/ros2_control/controller_manager/doc/userdoc.html).



# runing with irium camera

```bash
export GSCAM_CONFIG='v4l2src device=/dev/video0 io-mode=2 ! video/x-raw,format=I420,width=1280,height=720,framerate=30/1 ! videoconvert ! video/x-raw,format=RGB'
```
```bash
ros2 run gscam gscam_node --ros-args -p camera_name:=iriun_cam -p frame_id:=iriun_cam_optical -p image_encoding:=rgb8
```

# Testing MC modules
- To generate plugins from yaml config file:
```bash
python3 nomad_core/moducard_ros/moducard_generators/mc_generator.py -r --force nomad_core/mc_drivers/CONFIG_YAML_NAME.yaml
```
- Remember to build after generating plugins:
```bash
./compile.sh
```
- To run main driver module (it won't run unless there is at least one plugin available):
```bash
ros2 run mc_plugin_driver mc_plugin_driver_node --ros-args -p config_file:="nomad_core/nomad_bringup/bringup/config/mc_config.yaml"
```
