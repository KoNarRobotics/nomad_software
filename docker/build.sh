#!/bin/bash
# Build Nomad Docker image with optional compression
# Usage: ./build.sh        # Build for x86
#        ./build.sh -c     # Build for x86 and compress to .tar.zst
#        ./build.sh -a     # Build for ARM64
#        ./build.sh -a -c  # Build for ARM64 and compress to .tar.zst

set -e

# Change to script directory
cd "$(dirname "$0")"

# Parse arguments
BUILD_ARM=false
COMPRESS=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -a) BUILD_ARM=true ;;
        -c) COMPRESS=true ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done

# Determine platform and output names
if [ "$BUILD_ARM" = true ]; then
    PLATFORM="linux/arm64"
    IMAGE_NAME="nomad:arm64"
    OUTPUT_FILE="nomad-arm.tar.zst"
    echo "Building for ARM64"
else
    PLATFORM="linux/amd64"
    IMAGE_NAME="nomad:latest"
    OUTPUT_FILE="nomad.tar.zst"
    echo "Building for x86"
fi

echo "Platform: $PLATFORM"

# Build Docker image
docker buildx build \
    --platform "$PLATFORM" \
    --load \
    -t "$IMAGE_NAME" \
    -f Dockerfile \
    ..

echo ""
if [ "$COMPRESS" = true ]; then
    echo "Saving and compressing image to $OUTPUT_FILE..."
    rm -f "$OUTPUT_FILE"
    SIZE=$(docker image inspect "$IMAGE_NAME" --format='{{.Size}}')
    docker save "$IMAGE_NAME" | pv -N "Saving & Compressing" -s $SIZE | zstd -T0 > "$OUTPUT_FILE"

    echo "To load on remote device:"
    echo "  scp $OUTPUT_FILE user@remote-device:~/"
    echo "  zstd -d $OUTPUT_FILE"
    echo "  docker load < $(basename "$OUTPUT_FILE" .zst)"
else
    echo "✓ Docker image built successfully!"
    echo "  Image: $IMAGE_NAME"
    echo ""
    echo "To save as tar file, use: ./build.sh -c"
fi
