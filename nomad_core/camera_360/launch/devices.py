import os
import subprocess

def is_video_capture_device(device_path):
    """Check if device is a video capture device (camera)"""
    try:
        result = subprocess.run(['v4l2-ctl', '--device', device_path, '--list-formats-ext'], 
                              capture_output=True, text=True, check=True)
        # Look for Video Capture capability and if it have andy type of output format
        return 'Video Capture' in result.stdout and 'fps' in result.stdout
    except:
        return False


def scan_for_camera_devices():
  video_devices = []
  try:
    # Get device list from v4l2-ctl
    result = subprocess.run(['v4l2-ctl', '--list-devices'], 
                          capture_output=True, text=True, check=True)
    
    lines = result.stdout.split('\n')    
    for line in lines:
      line = line.strip()
      if line.startswith('/dev/video'):
        device_path = line
        if is_video_capture_device(device_path):
          video_devices.append(device_path)
  
  except subprocess.CalledProcessError:
      # Fallback to simple directory scan
    for i in range(10):
      device_path = f"/dev/video{i}"
      if os.path.exists(device_path) and is_video_capture_device(device_path):
        video_devices.append(device_path)

  return video_devices


dev = scan_for_camera_devices()
print("Found camera devices:", dev)