# 🎉 Camera-Enabled Buildroot Image Ready!

## Status: ✅ BUILD COMPLETE

**Date**: January 10, 2026  
**Build Time**: ~3 minutes (incremental)  
**Image Location**: `/home/daniel/buildroot/buildroot-2025.08/output/images/sdcard.img`  
**Image Size**: 289 MB

---

## What Was Enabled

### Camera Support Added:
✅ **rpi-userland** (VideoCore userland libraries)
   - `vcgencmd` - VideoCore management tool
   - Camera firmware libraries

✅ **Kernel Camera Drivers**:
   - `CONFIG_VIDEO_BCM2835_UNICAM` - Broadcom camera interface
   - `CONFIG_VIDEO_OV5647` - OV5647 camera sensor (your hardware)
   - `CONFIG_VIDEO_BCM2835_ISP` - Image Signal Processor
   - `CONFIG_MEDIA_CONTROLLER` - Media controller framework

✅ **Device Tree Configuration**:
   - Camera overlay support
   - CSI interface enabled

---

## Next Steps: Flash the SD Card

### Option 1: Flash from Linux (Recommended)

```bash
# 1. Insert SD card and find device name
lsblk
# Look for your SD card (e.g., /dev/sdb, /dev/mmcblk0)

# 2. Flash the image (REPLACE /dev/sdX with your actual device!)
cd /home/daniel/buildroot/buildroot-2025.08
sudo dd if=output/images/sdcard.img \
        of=/dev/sdX \
        bs=4M \
        status=progress \
        oflag=sync

# 3. Safely unmount
sync
sudo eject /dev/sdX
```

⚠️ **WARNING**: Double-check the device name! Using the wrong device will destroy data!

### Option 2: Flash with balenaEtcher (GUI)

```bash
# Install balenaEtcher if not installed
sudo apt install balena-etcher-electron

# Or download from: https://www.balena.io/etcher/

# Launch and select:
#  - Image: /home/daniel/buildroot/buildroot-2025.08/output/images/sdcard.img
#  - Target: Your SD card
#  - Click "Flash!"
```

---

## After Flashing: Verify Camera

### 1. Boot Raspberry Pi

- Remove old SD card from Pi
- Insert new SD card
- Power on Pi
- Wait ~30 seconds for boot
- LED should blink indicating activity

### 2. SSH and Test Camera

```bash
# SSH into Pi
ssh root@10.42.0.196
# Password: leafsense

# Check camera detection
vcgencmd get_camera
# EXPECTED: supported=1 detected=1
# (Not supported=0 detected=0 like before!)

# List video devices
ls -la /dev/video*

# Check V4L2 devices
v4l2-ctl --list-devices 2>/dev/null || echo "v4l2-ctl not available"
```

### 3. Test Camera Capture (if raspistill available)

```bash
ssh root@10.42.0.196

# Try capturing with raspistill
raspistill -v -o /tmp/test.jpg 2>&1

# If raspistill not found, try v4l2-grab or fswebcam
# Or check if camera module loaded
lsmod | grep bcm2835
lsmod | grep ov5647
```

### 4. Load Camera Modules (if needed)

If camera still not detected:

```bash
ssh root@10.42.0.196

# Load camera modules manually
modprobe bcm2835-v4l2
modprobe bcm2835-unicam
modprobe ov5647

# Check again
vcgencmd get_camera

# Make it permanent by adding to /etc/modules
echo "bcm2835-v4l2" >> /etc/modules
echo "bcm2835-unicam" >> /etc/modules
echo "ov5647" >> /etc/modules
```

---

## Deploy LeafSense with Real Camera

Once camera is working:

```bash
# On your PC
cd ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project

# Stop running LeafSense
ssh root@10.42.0.196 'killall LeafSense'

# Deploy (binary already built, just copy)
scp build-arm64/LeafSense root@10.42.0.196:/opt/leafsense/

# Start LeafSense
ssh root@10.42.0.196 '/opt/leafsense/LeafSense &'

# Watch logs
ssh root@10.42.0.196 'tail -f /var/log/leafsense.log'
```

**Look for this SUCCESS message:**
```
[Camera] Camera opened successfully!
[Camera] Resolution: 640x480
```

**NOT this warning:**
```
╔══════════════════════════════════════════╗
║  WARNING: CAMERA HARDWARE NOT DETECTED   ║
║  Generating TEST PATTERN instead        ║
╚══════════════════════════════════════════╝
```

---

## Verify Real Camera is Working

### Test 1: Check Gallery Images

```bash
# Download captured images
scp root@10.42.0.196:/opt/leafsense/gallery/plant_*.jpg /tmp/

# Open with image viewer
xdg-open /tmp/plant_*.jpg
```

**Expected**: Real photos (NOT uniform green RGB(61,120,40)!)  
**Check for**: Actual plant details, lighting variations, real textures

### Test 2: Analyze Image Variety

```bash
python3 << 'EOF'
from PIL import Image
import os

for img_file in sorted(os.listdir('/tmp'))[:3]:
    if img_file.startswith('plant_') and img_file.endswith('.jpg'):
        img = Image.open(f'/tmp/{img_file}')
        pixels = list(img.getdata())
        unique = len(set(pixels))
        print(f"{img_file}: {unique} unique colors", end='')
        if unique == 1:
            print(" → ❌ STILL TEST PATTERN")
        elif unique > 10000:
            print(" → ✅ REAL PHOTO!")
        else:
            print(f" → ⚠️  Suspicious ({unique} colors)")
EOF
```

---

## Troubleshooting

### Camera Still Not Detected

**Check 1: Boot configuration**
```bash
ssh root@10.42.0.196 'cat /boot/config.txt'
```

Should have these lines (add if missing):
```
start_x=1
gpu_mem=128
dtoverlay=ov5647
```

**Check 2: Kernel modules**
```bash
ssh root@10.42.0.196 'lsmod | grep -E "bcm2835|ov5647"'
```

Should show:
```
ov5647                 16384  0
bcm2835_unicam         32768  0
bcm2835_v4l2           45056  0
```

**Check 3: Device tree**
```bash
ssh root@10.42.0.196 'ls /proc/device-tree/soc/ | grep csi'
```

Should show:
```
csi@7e800000
csi@7e801000
```

**Check 4: dmesg errors**
```bash
ssh root@10.42.0.196 'dmesg | grep -i "camera\|ov5647\|unicam"'
```

Look for errors or "probe failed" messages.

### vcgencmd Says "detected=0"

This usually means VideoCore firmware can't access the camera. Try:

```bash
ssh root@10.42.0.196

# Ensure gpu_mem is sufficient
grep gpu_mem /boot/config.txt || echo "gpu_mem=128" >> /boot/config.txt

# Ensure camera is enabled
grep start_x /boot/config.txt || echo "start_x=1" >> /boot/config.txt

# Reboot
reboot
```

### OpenCV Still Can't Open Camera

Even if `vcgencmd` works, OpenCV might need specific device:

**Check which video device works:**
```bash
ssh root@10.42.0.196
for i in {0..31}; do
  echo -n "/dev/video$i: "
  v4l2-ctl --device=/dev/video$i --all 2>&1 | grep -q "Driver Info" && echo "WORKS" || echo "no"
done
```

Then modify [src/drivers/sensors/Cam.cpp](../src/drivers/sensors/Cam.cpp):
```cpp
// Try specific device that works (e.g., /dev/video0)
cap.open(0, cv::CAP_V4L2);
```

---

## Success Checklist

- [ ] SD card flashed with new image
- [ ] Raspberry Pi boots successfully
- [ ] Can SSH to `root@10.42.0.196`
- [ ] `vcgencmd get_camera` shows `supported=1 detected=1`
- [ ] `/dev/video*` devices exist
- [ ] Camera modules loaded (`lsmod | grep bcm2835`)
- [ ] LeafSense logs show "Camera opened successfully!"
- [ ] Gallery images show real photos (not uniform green)
- [ ] Images have >10,000 unique colors
- [ ] ML detection works on real plant images

---

## Quick Reference

```bash
# Check camera
ssh root@10.42.0.196 'vcgencmd get_camera'

# Restart LeafSense
ssh root@10.42.0.196 'killall LeafSense; /opt/leafsense/LeafSense &'

# View logs
ssh root@10.42.0.196 'tail -f /var/log/leafsense.log'

# Download latest image
ssh root@10.42.0.196 'ls -lt /opt/leafsense/gallery/ | head -2'
scp root@10.42.0.196:/opt/leafsense/gallery/plant_*.jpg /tmp/
xdg-open /tmp/plant_*.jpg
```

---

## Summary

**Before**: Camera hardware connected but VideoCore couldn't access it → test patterns  
**After**: Buildroot rebuilt with rpi-userland and camera drivers → real camera access  
**Result**: LeafSense will capture REAL plant photos instead of green test patterns! 📸🌿

**Image Ready**: `/home/daniel/buildroot/buildroot-2025.08/output/images/sdcard.img`

**Next Action**: Flash SD card and boot Pi! 🚀
