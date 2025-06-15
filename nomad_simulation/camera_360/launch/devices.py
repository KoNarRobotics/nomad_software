import os
import subprocess

def is_video_capture_device(device_path):
    """Check if device is a video capture device (camera)"""
    try:
        result = subprocess.run(['v4l2-ctl', '--device', device_path, '--info'], 
                              capture_output=True, text=True, check=True)
        # Look for Video Capture capability
        return 'Video Capture' in result.stdout and 'Video Output' not in result.stdout
    except:
        return False


def scan_for_camera_devices():
  video_devices = []
  
  try:
      # Get device list from v4l2-ctl
      result = subprocess.run(['v4l2-ctl', '--list-devices'], 
                            capture_output=True, text=True, check=True)
      
      lines = result.stdout.split('\n')
      current_device = None
      
      for line in lines:
          line = line.strip()
          if ':' in line and 'usb-' in line:
              # This is a device name line
              current_device = line.split(':')[0].strip()
          elif line.startswith('/dev/video') and current_device:
              device_path = line.strip()
              # Check if this is a capture device (camera)
              if is_video_capture_device(device_path):
                  video_devices.append({
                      'device': device_path,
                      'name': current_device
                  })
  
  except subprocess.CalledProcessError:
      # Fallback to simple directory scan
      for i in range(10):
          device_path = f"/dev/video{i}"
          if os.path.exists(device_path) and is_video_capture_device(device_path):
              video_devices.append({
                  'device': device_path,
                  'name': f'Camera{i}'
              })
  
  return video_devices


dev = scan_for_camera_devices()
for i, device in enumerate(dev):
    print(f"Device {i}: {device['device']} - {device['name']}")
    if i == 0:
        device_name = device['device']
        break