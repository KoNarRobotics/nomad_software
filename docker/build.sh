#!/bin/bash
# Build Nomad Docker image and compress as .tar.gz for distribution
# Usage: ./build.sh        # Build for x86 (amd64)
#        ./build.sh -a     # Build for ARM64 (Raspberry Pi)

set -e

# Change to script directory
cd "$(dirname "$0")"

# Parse arguments
BUILD_ARM=false
if [ "$1" = "-a" ]; then
    BUILD_ARM=true
fi

# Determine platform and output names
if [ "$BUILD_ARM" = true ]; then
    PLATFORM="linux/arm64"
    IMAGE_NAME="nomad:arm64"
    OUTPUT_FILE="nomad-arm.tar.gz"
    echo "Building for ARM64 (Raspberry Pi)..."
else
    PLATFORM="linux/amd64"
    IMAGE_NAME="nomad:latest"
    OUTPUT_FILE="nomad.tar.gz"
    echo "Building for x86 (amd64)..."
fi

echo "Platform: $PLATFORM"
echo "Output: $OUTPUT_FILE"
echo ""

# Build Docker image
docker buildx build \
    --platform "$PLATFORM" \
    --load \
    --debug \
    -t "$IMAGE_NAME" \
    -f Dockerfile \
    ..

echo ""
echo "Compressing image to $OUTPUT_FILE..."
docker save "$IMAGE_NAME" | gzip > "$OUTPUT_FILE"

SIZE=$(du -h "$OUTPUT_FILE" | cut -f1)
echo "✓ Done! File size: $SIZE"
echo "  File: $OUTPUT_FILE"
echo ""
echo "To load on remote device:"
echo "  scp $OUTPUT_FILE user@remote-device:~/"
echo "  docker load < $OUTPUT_FILE"
