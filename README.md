## Nomad

### Connecting to OBC via SSH
0. Ask the admin to add your ssh key to the Nomad.
1. Connect to any network with internet access or directly to the Nomad network.
2. Connect over ssh using the following command:
```bash
ssh nomad@nomadt.nihilia.xyz
```


### Connecting to OBC via front panel USB 
1. Connect the USB cable to the panel and the computer.

2. Open the terminal and find whitch port its connected to. Usually it is connected to /dev/ttyUSB0. To find the port use the following command:
```bash
ls /dev/ttyUSB*
```
if you have a problem with interface disconnecting. 
And your log looks something like this:
```bash
[  107.000000] usb 3-4.2: ch34x converter now attached to ttyUSB0
[  107.000000] usb 3-4.2: usbfs: interface 0 claimed by ch34x while 'brltty' sets config #1
[  107.000000] ch34x ttyUSB0: ch34x converter now disconnected from ttyUSB0
[  107.000000] ch34x 3-4.2:1.0: device disconnected
```
You can remove brltty with the following command:
```bash
sudo apt remove brltty
```
[more info about the problem](https://github.com/juliagoda/CH341SER/issues/18)

3. Connect to the console using the following command [to exit the console press `Ctrl + a` then `Ctrl + x`]:
```bash
picocom -b 1500000  /dev/ttyUSB0
```
if you don't have picocom installed you can install it with the following command:
```bash
sudo apt install picocom
```

