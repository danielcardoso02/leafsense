# Raspberry Pi Camera Setup Guide for LeafSense

## Current Status

Your system status:
- ✅ **Camera Hardware**: OV5647 connected and detected by kernel
- ✅ **Device Tree**: CSI interfaces present (`/proc/device-tree/soc/csi*`)
- ❌ **VideoCore**: Not accessible (`vcgencmd get_camera` shows `detected=0`)
- ❌ **Camera Utilities**: Not installed (no raspistill, libcamera-still, etc.)
- ❌ **Video Devices**: Present but cannot stream

**Result**: System generates test patterns instead of real camera captures.

---

## Quick Fix Attempt (Try This First!)

Before rebuilding Buildroot, let's try a simple device tree overlay:

### Step 1: Try Loading Camera Overlay

```bash
ssh root@10.42.0.196

# Try loading the OV5647 camera overlay
dtoverlay ov5647

# Check if camera is now detected
vcgencmd get_camera

# If successful, you should see: supported=1 detected=1
```

### Step 2: Make It Permanent (if it worked)

If the overlay worked, add it to boot configuration:

```bash
ssh root@10.42.0.196

# Check if config.txt exists
ls -la /boot/config.txt

# If it exists, add the overlay
echo "dtoverlay=ov5647" >> /boot/config.txt

# Reboot to test
reboot
```

### Step 3: Test Camera

After reboot:

```bash
ssh root@10.42.0.196

# Verify camera detected
vcgencmd get_camera

# Try capturing with v4l2 if available
v4l2-ctl --list-devices

# Restart LeafSense to use real camera
killall LeafSense
/opt/leafsense/LeafSense &
```

---

## Full Solution: Rebuild Buildroot with Camera Support

If the quick fix doesn't work, you need to rebuild Buildroot with proper camera support.

### Prerequisites

1. **Buildroot installed** (if not, see "Install Buildroot" section below)
2. **Cross-compilation toolchain**
3. **At least 20GB free disk space**
4. **2-4 hours for compilation**

### Step-by-Step Instructions

#### 1. Install Buildroot (if needed)

```bash
cd ~/Desktop/ESRG/2025-2026/Project/Rasp

# Download Buildroot 2025.08
wget https://buildroot.org/downloads/buildroot-2025.08.tar.gz

# Extract
tar xzf buildroot-2025.08.tar.gz

# Set environment variable
export BUILDROOT_DIR=$PWD/buildroot-2025.08
cd $BUILDROOT_DIR
```

#### 2. Configure Buildroot for Raspberry Pi 4

```bash
cd $BUILDROOT_DIR

# Start with Raspberry Pi 4 64-bit defconfig
make raspberrypi4_64_defconfig

# Open menu configuration
make menuconfig
```

#### 3. Enable Camera Support in menuconfig

Navigate through the menus and enable these options:

##### Target Packages
```
Target packages  --->
  Hardware handling  --->
    [*] rpi-userland
        [*]   Install test programs
    [*] v4l-utils
        [*]   v4l2-ctl
        [*]   v4l2-compliance
```

##### Kernel Configuration
```
Kernel  --->
  Kernel configuration  --->
    (* ) Using a custom (def)config file
```

Then create a kernel config fragment (see step 4 below).

##### System Configuration
```
System configuration  --->
  [*] Enable root login with password
  ()    Root password (leave empty or set your password)
  
  /dev management  --->
    (* ) Dynamic using devtmpfs + mdev
```

##### Filesystem Images
```
Filesystem images  --->
  [*] ext2/3/4 root filesystem
        ext2/3/4 variant (ext4)  --->
  (200M) exact size
```

#### 4. Create Kernel Config Fragment

Create `$BUILDROOT_DIR/board/raspberrypi4/linux-camera.config`:

```bash
mkdir -p $BUILDROOT_DIR/board/raspberrypi4

cat > $BUILDROOT_DIR/board/raspberrypi4/linux-camera.config << 'EOF'
# Media support
CONFIG_MEDIA_SUPPORT=y
CONFIG_MEDIA_CAMERA_SUPPORT=y
CONFIG_VIDEO_DEV=y
CONFIG_VIDEO_V4L2=y
CONFIG_VIDEO_V4L2_SUBDEV_API=y

# Broadcom VideoCore camera
CONFIG_VIDEO_BCM2835=m
CONFIG_VIDEO_BCM2835_UNICAM=m

# Camera sensors
CONFIG_VIDEO_OV5647=m
CONFIG_VIDEO_IMX219=m

# Media controller
CONFIG_MEDIA_CONTROLLER=y
CONFIG_V4L2_MEM2MEM_DEV=m

# Camera ISP
CONFIG_VIDEO_BCM2835_ISP=m
EOF
```

#### 5. Update menuconfig with Kernel Fragment

Go back to `make menuconfig`:

```
Kernel  --->
  Kernel configuration  --->
    (board/raspberrypi4/linux-camera.config) Configuration fragment files
```

Save and exit.

#### 6. Build Buildroot

```bash
cd $BUILDROOT_DIR

# Clean previous build (if any)
make clean

# Start build (this takes 2-4 hours on first run)
make

# Output will be in: output/images/
```

#### 7. Flash SD Card

```bash
# Find your SD card device (be careful!)
lsblk

# Flash the image (replace sdX with your device)
sudo dd if=$BUILDROOT_DIR/output/images/sdcard.img \
        of=/dev/sdX \
        bs=4M \
        status=progress \
        oflag=sync

# Safely unmount
sync
sudo eject /dev/sdX
```

#### 8. Boot and Verify

Insert SD card into Raspberry Pi and boot:

```bash
# SSH into Pi
ssh root@10.42.0.196

# Check camera detection
vcgencmd get_camera
# Should show: supported=1 detected=1

# List video devices
v4l2-ctl --list-devices

# Test camera capture
raspistill -o /tmp/test.jpg

# View the image
ls -lh /tmp/test.jpg
```

#### 9. Deploy LeafSense

```bash
# On your PC
cd ~/Desktop/ESRG/2025-2026/Project/Rasp/leafsense-project

# Rebuild for new rootfs
./deploy/setup-onnxruntime-arm64.sh
mkdir -p build-arm64 && cd build-arm64
cmake -DCMAKE_TOOLCHAIN_FILE=../deploy/toolchain-rpi4.cmake ..
make -j$(nproc)

# Deploy
scp LeafSense root@10.42.0.196:/opt/leafsense/
ssh root@10.42.0.196 'killall LeafSense; /opt/leafsense/LeafSense &'
```

#### 10. Verify Real Camera Works

```bash
# Check logs
ssh root@10.42.0.196 'tail -f /var/log/leafsense.log'

# You should see:
# [Camera] Camera opened successfully!
# (NOT "WARNING: CAMERA HARDWARE NOT DETECTED")

# Check captured images
ssh root@10.42.0.196 'ls -lh /opt/leafsense/gallery/'

# Download and view an image
scp root@10.42.0.196:/opt/leafsense/gallery/plant_*.jpg /tmp/
xdg-open /tmp/plant_*.jpg

# Should show REAL photo, not uniform green!
```

---

## Troubleshooting

### Camera Still Not Detected After Rebuild

1. **Check boot config**:
   ```bash
   ssh root@10.42.0.196 'cat /boot/config.txt | grep camera'
   ```
   
   Should have:
   ```
   start_x=1
   gpu_mem=128
   ```

2. **Check kernel modules loaded**:
   ```bash
   ssh root@10.42.0.196 'lsmod | grep bcm2835'
   ssh root@10.42.0.196 'lsmod | grep ov5647'
   ```

3. **Load modules manually**:
   ```bash
   ssh root@10.42.0.196
   modprobe bcm2835-v4l2
   modprobe bcm2835-unicam
   modprobe ov5647
   ```

4. **Check dmesg for errors**:
   ```bash
   ssh root@10.42.0.196 'dmesg | grep -i camera'
   ssh root@10.42.0.196 'dmesg | grep -i ov5647'
   ```

### raspistill Command Not Found

rpi-userland may not have installed correctly. Check:

```bash
ssh root@10.42.0.196
ls -la /opt/vc/bin/
# Should see: raspistill, raspivid, vcgencmd, etc.

# If empty, rpi-userland didn't install
# Go back to menuconfig and ensure it's enabled
```

### Video Devices Present But Can't Open

```bash
ssh root@10.42.0.196

# Check permissions
ls -la /dev/video*

# Try different video devices
for i in {0..31}; do
  v4l2-ctl --device=/dev/video$i --all 2>&1 | grep -A5 "Driver Info"
done

# Look for one that shows "bcm2835" or "ov5647"
```

### OpenCV Can't Open Camera in LeafSense

Even if raspistill works, OpenCV might need specific video device:

Modify `src/drivers/sensors/Cam.cpp`:

```cpp
// Try specific device numbers
for (int dev = 0; dev < 32; dev++) {
    cap.open(dev, cv::CAP_V4L2);
    if (cap.isOpened()) {
        std::cout << "[Camera] Opened /dev/video" << dev << std::endl;
        break;
    }
}
```

---

## Alternative: Use libcamera (Modern Approach)

Instead of legacy VideoCore, use libcamera:

### Enable in Buildroot

```
Target packages  --->
  Libraries  --->
    Graphics  --->
      [*] libcamera
          [*] libcamera pipeline handlers  --->
              [*] Raspberry Pi
          [*] libcamera tools
```

### Modify LeafSense Code

Replace OpenCV camera code with libcamera-based capture.

---

## Testing Checklist

After camera enablement:

- [ ] `vcgencmd get_camera` shows `supported=1 detected=1`
- [ ] `raspistill -o /tmp/test.jpg` captures image
- [ ] `ls /dev/video*` shows devices
- [ ] `v4l2-ctl --list-devices` shows bcm2835-unicam
- [ ] LeafSense logs show "Camera opened successfully!"
- [ ] Gallery images show real photos (not uniform green)
- [ ] ML detection works on real leaf images

---

## Quick Reference Commands

```bash
# Check camera status
ssh root@10.42.0.196 'vcgencmd get_camera'

# List video devices
ssh root@10.42.0.196 'ls -la /dev/video*'

# Test camera capture
ssh root@10.42.0.196 'raspistill -v -o /tmp/test.jpg'

# Check LeafSense is using real camera
ssh root@10.42.0.196 'cat /var/log/leafsense.log | grep Camera'

# Restart LeafSense
ssh root@10.42.0.196 'killall LeafSense; /opt/leafsense/LeafSense &'

# View captured images
ssh root@10.42.0.196 'ls -lh /opt/leafsense/gallery/'
scp root@10.42.0.196:/opt/leafsense/gallery/plant_*.jpg /tmp/
xdg-open /tmp/plant_*.jpg
```

---

## Summary

**Current Problem**: Camera hardware connected but VideoCore not initialized

**Quick Fix**: Try `dtoverlay ov5647` first (5 minutes)

**Full Fix**: Rebuild Buildroot with rpi-userland and camera drivers (2-4 hours)

**Result**: LeafSense will automatically use real camera instead of test patterns

---

## Need Help?

- See [docs/04-RASPBERRY-PI-DEPLOYMENT.md](../docs/04-RASPBERRY-PI-DEPLOYMENT.md) for deployment details
- See [docs/00-TERMINOLOGY.md](../docs/00-TERMINOLOGY.md) for term definitions
- Check [docs/08-TROUBLESHOOTING.md](../docs/08-TROUBLESHOOTING.md) for common issues

Good luck! 🎥📸
