# Gallery Tab & Camera Analysis

## 📷 Images Currently Captured

**What You're Seeing:**
The images are **TEST PATTERNS** (uniform green background with text overlay).

**Analysis of Downloaded Images:**
```
plant_19700101_014945.jpg:
  Size: 640×480 pixels
  Color: Uniform (61, 120, 40) RGB = Dark Green
  Pattern: Test pattern, not real camera capture
  
plant_19700101_014949.jpg:
  Size: 640×480 pixels  
  Color: Uniform (61, 120, 40) RGB = Dark Green
  Pattern: Test pattern, not real camera capture
```

All images show the same uniform green color → These are generated test patterns.

---

## 🔍 Why Camera Isn't Working

**Good News:** Camera IS detected by kernel!
```
Kernel messages show: ov5647@36 (Camera Module v1)
Device tree sees CSI connection
```

**Bad News:** VideoCore GPU utilities not reporting it
```
vcgencmd get_camera → supported=0 detected=0
```

**Root Cause:**
The camera hardware is physically connected and kernel sees it, BUT:
1. Buildroot system may not have full camera stack configured
2. Camera needs to be enabled in device tree or boot config
3. VideoCore firmware may need camera support enabled

**Why Test Patterns:**
Since OpenCV can't open the camera device, the code falls back to generating test patterns (the green images you see).

---

## 🎨 Gallery Tab Functionality

### What the Gallery Tab Does

**Purpose:** Display all captured plant images with ML analysis results

**Features:**
1. **Image Display** - Shows captured plant photos at full resolution
2. **ML Predictions** - Displays disease classification results
   - Class: Healthy, Nutrient Deficiency, Pest Damage, Disease
   - Confidence: Percentage (e.g., 99.9%)
3. **Navigation** - Browse through captured images
   - ◀ Previous button
   - ▶ Next button
   - Counter: "2/33" (current/total)
4. **Verification System** - Mark ML predictions as correct/incorrect

### Image Information Shown

For each image, the gallery displays:
```
┌──────────────────────────────────────────┐
│                                          │
│         [Plant Image Display]            │
│                                          │
└──────────────────────────────────────────┘
  2/33 - 2026-01-10 14:53:21 [PENDING] Pest Damage
  
  [◀ Previous]  [Confirm Issue]  [Next ▶]
```

**Status Indicators:**
- `[PENDING]` - ML prediction not yet verified by human
- `[VERIFIED]` - Human confirmed the ML prediction is correct

---

## 🔘 "Confirm Issue" Button

### What It Does

**Purpose:** Human verification of ML disease detection

**Functionality:**
1. **Marks Prediction as Verified**
   - Changes status from `[PENDING]` to `[VERIFIED]`
   - Visual feedback: Button becomes grayed out "Verified"
   - Future: Will update database `ml_detections.is_verified = 1`

2. **Use Case Scenario:**
   ```
   ML detects: "Pest Damage - 98.5% confidence"
   ↓
   You inspect image → YES, there are holes/damage
   ↓
   Click "Confirm Issue"
   ↓
   Status changes to [VERIFIED]
   ↓
   System knows: ML was correct, increase confidence
   ```

3. **Why This Matters:**
   - **Quality Control:** Ensure ML is accurate
   - **Training Data:** Verified images can improve model
   - **Decision Making:** Only act on verified threats
   - **Record Keeping:** Track which issues were real vs false alarms

### Button States

**Before Verification:**
```
┌──────────────────────────┐
│    Confirm Issue         │  ← Orange, clickable
└──────────────────────────┘
```

**After Verification:**
```
┌──────────────────────────┐
│       Verified           │  ← Gray, disabled
└──────────────────────────┘
```

### Future Database Integration

**Currently:** Only UI state changes (not persisted)

**Planned (TODO in code):**
```sql
UPDATE ml_detections 
SET is_verified = 1,
    verified_by = 'user',
    verified_at = CURRENT_TIMESTAMP
WHERE image_id = current_image_id;
```

This will allow tracking:
- Which predictions humans confirmed
- False positive rate (ML wrong, human corrects)
- Model accuracy over time

---

## 🔧 How to Enable Real Camera

### Problem Summary
- ✅ Camera physically connected (OV5647)
- ✅ Kernel detects camera (device tree)
- ❌ VideoCore GPU not initialized for camera
- ❌ OpenCV cannot access camera device
- ✅ System works with test patterns

### Solution Steps

**Option 1: Enable Camera in Buildroot**
1. Rebuild Buildroot with camera support:
   ```bash
   make menuconfig
   # Enable: Target packages → Hardware handling → bcm2835-userland
   # Enable: Kernel → Device tree → enable camera overlay
   make
   ```

**Option 2: Manual Camera Configuration**
1. Check if camera overlay is loaded:
   ```bash
   ssh root@10.42.0.196 'ls /proc/device-tree/soc/csi*'
   ```

2. Try to manually configure with media-ctl:
   ```bash
   ssh root@10.42.0.196 'media-ctl -d /dev/media0 --set-v4l2 "unicam:0[fmt:SRGGB10_1X10/640x480]"'
   ```

3. Test capture directly:
   ```bash
   ssh root@10.42.0.196 'raspistill -o test.jpg || libcamera-still -o test.jpg'
   ```

**Option 3: Use libcamera Instead**
Modern way to access Pi camera:
1. Install libcamera in Buildroot
2. Modify Cam.cpp to use libcamera API
3. Recompile LeafSense

---

## 📊 Gallery Workflow Example

**Typical Usage:**

1. **System Captures Image**
   - Every 30 minutes automatically
   - Or manual trigger from dashboard

2. **ML Analysis Runs**
   - Analyzes captured image
   - Classifies: Healthy/Nutrient Def/Pest/Disease
   - Calculates confidence: 0-100%

3. **Image Appears in Gallery**
   - Status: [PENDING]
   - Shows ML prediction

4. **Human Review**
   - Open Gallery tab
   - View image at full resolution
   - Check if ML is correct

5. **Verification**
   - If correct: Click "Confirm Issue"
   - If wrong: Note discrepancy (future: correct label)
   - Status changes to [VERIFIED]

6. **Action Taken**
   - For verified pest/disease: Take corrective action
   - For false alarms: Ignore, improve model later
   - Track accuracy over time

---

## 🎯 Summary

**Gallery Tab:**
- 📸 Shows all captured plant images
- 🤖 Displays ML disease detection results
- ✅ Allows human verification of predictions
- 📊 Navigation through image history

**Confirm Issue Button:**
- ✅ Marks ML prediction as human-verified
- 📝 Future: Saves verification to database
- 🎯 Purpose: Quality control and model validation

**Current Status:**
- Images are TEST PATTERNS (green backgrounds)
- Camera hardware connected but not properly initialized
- Need to enable camera support in Buildroot
- All other functionality working correctly

**To See Real Images:**
1. Enable camera in Buildroot configuration
2. Or use libcamera for modern Pi camera support
3. System will automatically use real camera once available

---

See also:
- [CAMERA_STATUS.txt](../CAMERA_STATUS.txt) - Why test patterns
- [docs/03-MACHINE-LEARNING.md](../docs/03-MACHINE-LEARNING.md) - ML details
- [docs/07-GUI.md](../docs/07-GUI.md) - GUI documentation
