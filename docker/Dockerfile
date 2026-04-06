# Multi-arch Dockerfile for Nomad Robot Software
# Supports both arm64 (Raspberry Pi 5) and amd64 (x86 PC)
# Uses build_local.sh for dependency compilation
# Build: docker buildx build --platform linux/arm64,linux/amd64 -t nomad:latest .

FROM ros:jazzy

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV COLCON_DEFAULTS_CONFIG_DIR=/ws/colcon_config

# Update apt repositories and install system dependencies
RUN apt-get update && apt-get install -y \
  # Essential build tools
  build-essential \
  cmake \
  ninja-build \
  git \
  wget \
  curl \
  pkg-config \
  sudo \
  # Python3 and tools
  python3-dev \
  python3-pip \
  python3-numpy \
  # OpenCV dependencies
  libeigen3-dev \
  libglew-dev \
  libboost-all-dev \
  ffmpeg \
  libavcodec-dev \
  libavutil-dev \
  libavformat-dev \
  libswscale-dev \
  libjpeg-dev \
  libpng-dev \
  libtiff-dev \
  libx11-dev \
  libxrandr-dev \
  libxi-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  libraw1394-dev \
  # Pangolin dependencies
  libepoxy-dev \
  libxkbcommon-x11-0 \
  libxkbcommon0 \
  # CAN bus support
  can-utils \
  iproute2 \
  # GPU support (NVIDIA)
  nvidia-cuda-toolkit \
  # Utilities
  nano \
  vim \
  htop \
  tmux 
# && rm -rf /var/lib/apt/lists/*

# Create workspace and copy entire repo
WORKDIR /ws
COPY . .

# Initialize git submodules for dependencies
RUN cd /ws/dependencies && \
  git submodule update --init --recursive || true

# Clean any existing build directories to avoid CMake cache issues
RUN find /ws/dependencies -type d -name "build" -exec rm -rf {} + 2>/dev/null || true

# Run build_local.sh to build all dependencies
RUN cd /ws/dependencies && \
  chmod +x build_local.sh && \
  ./build_local.sh

# Setup local environment paths
RUN . /ws/setup_local_env.sh && \
  echo "source /ws/setup_local_env.sh" >> /root/.bashrc

# Build ROS2 packages with local dependencies
RUN . /opt/ros/jazzy/setup.sh && \
  . /ws/setup_local_env.sh && \
  cd /ws && \
  sudo rosdep fix-permissions && \
  rosdep update && \
  colcon build --symlink-install

# Setup ROS2 environment in bashrc
RUN echo "source /opt/ros/jazzy/setup.bash" >> /root/.bashrc && \
  echo "source /ws/install/setup.bash" >> /root/.bashrc

# Set library paths from local_install for runtime
ENV CMAKE_PREFIX_PATH=/ws/local_install:$CMAKE_PREFIX_PATH
ENV LD_LIBRARY_PATH=/ws/local_install/lib:/ws/local_install/lib64:$LD_LIBRARY_PATH
ENV PKG_CONFIG_PATH=/ws/local_install/lib/pkgconfig:/ws/local_install/lib64/pkgconfig:$PKG_CONFIG_PATH
ENV PATH=/ws/local_install/bin:$PATH

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
  CMD pgrep -f "sleep" || exit 1

# Default entrypoint: infinite loop (exec into container to run commands)
CMD ["/bin/bash", "-c", "echo 'Nomad Robot Container Started. You can now exec into this container.'; while true; do sleep 10; done"]
