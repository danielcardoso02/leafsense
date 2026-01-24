#!/bin/bash
# =============================================================================
# LeafSense - Raspberry Pi Screenshot Tool
# =============================================================================
# Captures the Waveshare 3.5" touchscreen (fb1) and converts to PNG
#
# Usage:
#   ./screenshot_pi.sh <output_name>
#   ./screenshot_pi.sh gui_login
#   ./screenshot_pi.sh my_screenshot
#
# Requirements:
#   - Python3 with PIL (pip install Pillow)
#   - SSH access to Pi (root@10.42.0.196)
# =============================================================================

PI_IP="10.42.0.196"
PI_USER="root"
OUTPUT_NAME="${1:-screenshot}"
OUTPUT_DIR="$(dirname "$0")/../docs/latex/images"
TEMP_DIR="/tmp"

echo "📸 Capturing screenshot from Raspberry Pi..."

# Capture framebuffer on Pi
ssh ${PI_USER}@${PI_IP} "cat /dev/fb1 > /tmp/screenshot.raw"

# Transfer to local machine
scp ${PI_USER}@${PI_IP}:/tmp/screenshot.raw ${TEMP_DIR}/

# Convert to PNG
python3 << EOF
from PIL import Image
import struct

with open('${TEMP_DIR}/screenshot.raw', 'rb') as f:
    data = f.read()

img = Image.new('RGB', (480, 320))
pixels = img.load()

for y in range(320):
    for x in range(480):
        idx = (y * 480 + x) * 2
        if idx + 1 < len(data):
            pixel = struct.unpack('<H', data[idx:idx+2])[0]
            r = ((pixel >> 11) & 0x1F) << 3
            g = ((pixel >> 5) & 0x3F) << 2
            b = (pixel & 0x1F) << 3
            pixels[x, y] = (r, g, b)

img.save('${OUTPUT_DIR}/${OUTPUT_NAME}.png')
print('✅ Saved: ${OUTPUT_DIR}/${OUTPUT_NAME}.png')
EOF

# Cleanup
rm -f ${TEMP_DIR}/screenshot.raw
