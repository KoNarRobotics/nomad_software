# Nomad Robot Docker Setup

Multi-architecture containerized build system for the Nomad robot software, supporting both x86 (powerful control nodes) and ARM64 (Raspberry Pi 5 edge nodes).

## Overview

- **Single Docker image** for all devices (compiled once, deployed everywhere)
- **Multi-arch support** (amd64 and arm64)
- **Hardware access** (GPU, CAN, USB devices)
- **Infinite loop default** - exec into containers to run commands manually

## Prerequisites

### On Build Machine (x86 PC)
- Docker Desktop or Docker with buildx
- git
- ~50GB free disk space
- 1Gb network for deployment

### On Deployment Devices
- Docker and docker-compose installed
- 1TB SSD (you mentioned you have this)
- 1Gb network

## Build Instructions

### Step 1: Setup buildx (first time only)

```bash
# Docker Desktop has this built-in, but if using regular Docker:
docker run --privileged --rm tonistiigi/binfmt --install all
```

### Step 2: Build the Image

From the root of the nomad_software repository:

```bash
cd docker

# Make build script executable
chmod +x build.sh

# Build both amd64 and arm64 for local testing (amd64 only)
./build.sh

# Or specify platforms:
export BUILD_PLATFORMS=linux/amd64
./build.sh

# Or for ARM64 only (for testing on Raspberry Pi):
export BUILD_PLATFORMS=linux/arm64
./build.sh
```

For **multi-platform builds to registry** (recommended for production):

```bash
export REGISTRY=docker.io/your-username
export BUILD_PLATFORMS=linux/amd64,linux/arm64
./build.sh
```

This takes ~20-30 minutes on a modern x86 PC to compile everything.

## Deployment

### Option 1: Local Development (x86)

```bash
cd docker

# Start container
docker-compose up -d

# Connect to it
docker exec -it nomad_robot /bin/bash

# Inside container, run ROS2 commands:
source /opt/ros/jazzy/setup.bash
source /ws/install/setup.bash
ros2 node list
```

### Option 2: x86 Control Node

```bash
cd docker

# Start with x86 overrides
docker-compose -f docker-compose.yml -f docker-compose.x86.yml up -d

# Exec in
docker exec -it nomad_control /bin/bash
```

### Option 3: Raspberry Pi 5 Edge Nodes

On each Raspberry Pi:

```bash
# Pull the image (if pushed to registry)
docker pull docker.io/your-username/nomad:latest

# Or copy from x86 if no registry:
# scp the image or use docker save/load

# Create docker directory
mkdir -p ~/nomad/docker
cd ~/nomad/docker

# Copy compose files
scp <user@x86>:/path/to/nomad_software/docker/docker-compose*.yml .

# Start with ARM64 overrides
docker-compose -f docker-compose.yml -f docker-compose.arm64.yml up -d

# Connect
docker exec -it nomad_edge /bin/bash
```

## Container Access

### Basic Operations

```bash
# List running containers
docker ps

# View container logs
docker logs nomad_robot

# Enter container shell
docker exec -it nomad_robot /bin/bash

# Stop container
docker-compose down

# Restart
docker-compose restart
```

### Running ROS2

Inside the container:

```bash
# Source setup scripts
source /opt/ros/jazzy/setup.bash
source /ws/install/setup.bash

# List nodes
ros2 node list

# Run specific packages
ros2 launch nomad_bringup some_launch.py

# Later, replace the infinite loop with actual services in docker-compose
```

### Hardware Access

All configured in `docker-compose.yml`:

**GPUs**
```bash
# Inside container
nvidia-smi

# All NVIDIA devices are accessible
```

**CAN Bus**
```bash
# List CAN devices (from host before starting container)
ip link show | grep can

# Inside container
candump can0
```

**USB/Serial**
```bash
# List USB devices
ls /dev/ttyUSB*
ls /dev/ttyACM*

# Inside container, use them directly
```

## Architecture

```
x86 PC (Build + Control)
└─ docker build (multi-arch)
   ├─ Stage 1: System dependencies
   ├─ Stage 2: Build Livox-SDK2, Pangolin, OpenCV, ORB-SLAM3
   ├─ Stage 3: Build ROS2 workspace (colcon)
   └─ Stage 4: Runtime setup + infinite loop
   
   Deployment:
   ├─ x86: docker-compose + docker-compose.x86.yml
   └─ ARM64 (Pis): docker-compose + docker-compose.arm64.yml
```

## File Structure

```
docker/
├── Dockerfile              # Multi-arch build definition
├── .dockerignore          # Optimize build context
├── docker-compose.yml     # Base service config (all devices)
├── docker-compose.x86.yml # x86 overrides
├── docker-compose.arm64.yml # ARM64 overrides
├── build.sh               # Build script with multi-arch support
└── README.md              # This file
```

## Customization

### Change Default Command

Edit `docker-compose.yml` and replace the `CMD` line in Dockerfile:

```dockerfile
# Instead of infinite loop:
CMD ["/bin/bash", "-c", "source /ws/install/setup.bash && ros2 launch ..."]
```

### Add Volumes for Config

Uncomment in `docker-compose.yml`:

```yaml
volumes:
  - ./configs:/ws/configs:ro
  - ./data:/ws/data:rw
```

Then create `configs/` directory with your ROS2 configs.

### Limit Resources

Modify `docker-compose.yml`:

```yaml
deploy:
  resources:
    limits:
      cpus: '4'
      memory: '16G'
```

## Troubleshooting

### Build fails with "Permission denied"

```bash
chmod +x docker/build.sh
```

### buildx not found

Install Docker Desktop or enable binfmt:

```bash
docker run --privileged --rm tonistiigi/binfmt --install all
```

### Can't access GPU inside container

Ensure NVIDIA Container Toolkit is installed:

```bash
distribution=$(. /etc/os-release;echo $ID$VERSION_ID)
curl -s -L https://nvidia.github.io/nvidia-docker/gpgkey | apt-key add -
curl -s -L https://nvidia.github.io/nvidia-docker/$distribution/nvidia-docker.list | \
  tee /etc/apt/sources.list.d/nvidia-docker.list
apt-get update && apt-get install -y nvidia-container-toolkit
systemctl restart docker
```

### CAN/USB devices not visible in container

- Check host permissions: `ls -la /dev/can0` or `ls -la /dev/ttyUSB0`
- Container has `privileged: true` and `devices:` configured
- Restart docker daemon if needed

### Multi-platform build not working

- Only docker Desktop or buildx-enabled Docker works
- Single-platform: set `BUILD_PLATFORMS=linux/amd64` in `build.sh`

## Performance Notes

| Task | x86 PC | Raspberry Pi 5 |
|------|--------|---|
| Build time | ~1 min (first time: ~20-30 min) | NOT recommended (use x86 build) |
| Image pull | ~30-60 sec (on 1Gb network) | ~30-60 sec |
| Container startup | <2 sec | <2 sec |
| ORB-SLAM3 frame rate | Real-time (~30 fps) | Limited by CPU/GPU |

## Next Steps

1. **Build**: Run `./build.sh` on your x86 PC
2. **Test locally**: `docker-compose up && docker exec -it nomad_robot /bin/bash`
3. **Deploy to devices**: Copy image/pull to Raspberry Pi nodes
4. **Start services**: In each device's docker directory, run compose commands
5. **Override default command**: Later, modify docker-compose to run specific ROS2 launches

## Notes

- Container runs as `root` by default
- Default infinite loop allows manual flexibility - exec in and run whatever you want
- All configs are baked into the image - for runtime config changes, use volumes
- Multi-arch build may take longer due to QEMU emulation
- For production, push to Docker registry (faster than SCP/load)
