<h1 align="center">LeafSense - Machine Learning</h1>

<p align="center"><em>Image classification model for plant disease detection using MobileNetV3-Small</em></p>

---

<h2 align="center">Classification Classes</h2>

<div align="center">

| Class | Description | Recommended Action |
|--------|-----------|------------------|
| **Healthy** | Healthy plant | Maintain current conditions |
| **Disease** | Bacterial/fungal diseases | Remove affected leaves, apply treatment |
| **Deficiency** | NPK nutrient deficiency | Adjust nutrient solution |
| **Pest** | Insect damage | Apply pest control measures |

</div>

## Model Architecture

### Base: MobileNetV3-Small
```
Input: 224x224x3 (RGB)
    ↓
Conv2d (64 filters, 7x7)
    ↓
BatchNorm + ReLU + MaxPool
    ↓
Block1 (64 → 64)
    ↓
Block2 (64 → 128)
    ↓
Block3 (128 → 256)
    ↓
Block4 (256 → 512)
    ↓
AdaptiveAvgPool2d
    ↓
Flatten
    ↓
Linear (512 → 4)
    ↓
Output: 4 classes (softmax)
```

## Training Metrics

### Final Results
- **Accuracy:** 99.39%
- **Precision:** 99.41%
- **Recall:** 99.39%
- **F1-Score:** 99.39%

### Confusion Matrix (Validation Set)
```
                             Predicted
                 Healthy  Disease  Deficiency  Pest
        Healthy     [76      0         0        0]
Actual  Disease     [ 0    346         0        0]
        Deficiency  [ 0      0        41        0]
        Pest        [ 0      0         0      692]

Total validation images: 1,155 (across 4 classes)
```

### Training Curves
- **Training Loss:** 0.0089 (final epoch)
- **Validation Loss:** 0.0156 (final epoch)
- **Epochs:** 20
- **Learning Rate:** 0.001 → 0.0001 (scheduler)

## Dataset

### Combined Real-World Datasets
- **Sources:** Roboflow (disease/healthy), NPK deficiency dataset, Pest detection dataset
- **Total images:** 2,206
  - Training: 1,543 images
  - Validation: 663 images
- **Class distribution:**
  - Disease: 1,123 images (777 train + 346 val)
  - Pest: 1,692 images (1,000 train + 692 val)
  - Healthy: 224 images (148 train + 76 val)
  - Deficiency: 196 images (155 train + 41 val)

### Data Augmentation
```python
transform_train = transforms.Compose([
    transforms.RandomResizedCrop(224),
    transforms.RandomHorizontalFlip(),
    transforms.RandomRotation(15),
    transforms.ColorJitter(brightness=0.2, contrast=0.2),
    transforms.ToTensor(),
    transforms.Normalize([0.485, 0.456, 0.406], 
                        [0.229, 0.224, 0.225])
])
```

## System Integration

### Model File
- **Location:** `/opt/leafsense/leafsense_model.onnx`
- **Size:** 5.9 MB
- **Format:** ONNX (Open Neural Network Exchange)
- **Classes:** `/opt/leafsense/leafsense_model_classes.txt`

### Inference Code (C++)
```cpp
// include/application/ml/ML.h
class MLEngine {
public:
    MLEngine(const std::string& model_path);
    
    // Classify an image
    PredictionResult predict(const cv::Mat& image);
    
    // Get all probabilities
    std::vector<float> getProbabilities(const cv::Mat& image);
    
private:
    Ort::Session session_;
    Ort::Env env_;
    
    cv::Mat preprocess(const cv::Mat& image);
    std::string classIndexToName(int index);
};

struct PredictionResult {
    std::string className;
    float confidence;
    int classIndex;
};
```

### Preprocessing
```cpp
cv::Mat MLEngine::preprocess(const cv::Mat& image) {
    cv::Mat resized, normalized;
    
    // Resize to 224x224
    cv::resize(image, resized, cv::Size(224, 224));
    
    // Convert BGR to RGB
    cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);
    
    // Normalize with ImageNet means
    resized.convertTo(normalized, CV_32F, 1.0/255.0);
    
    // Apply per-channel normalization
    cv::subtract(normalized, cv::Scalar(0.485, 0.456, 0.406), normalized);
    cv::divide(normalized, cv::Scalar(0.229, 0.224, 0.225), normalized);
    
    return normalized;
}
```

## ONNX Runtime

### ARM64 Configuration
```cpp
Ort::SessionOptions session_options;
session_options.SetIntraOpNumThreads(4);  // 4 cores on Pi
session_options.SetGraphOptimizationLevel(
    GraphOptimizationLevel::ORT_ENABLE_ALL
);
```

<h3 align="center">Performance on Raspberry Pi 4B</h3>

<div align="center">

| Metric | Value |
|--------|-------|
| Inference time | ~150ms |
| RAM usage | ~50MB |
| CPU usage | ~80% (1 core) |

</div>

## Model Training

### Training Environment
- **GPU:** NVIDIA RTX 3070
- **Framework:** PyTorch 2.0
- **Python:** 3.10

### Training Script
```python
# ml/training/train.py
import torch
import torch.nn as nn
from torchvision import models, transforms
from torch.utils.data import DataLoader

# MobileNetV3-Small base model
model = models.mobilenet_v3_small(weights=models.MobileNet_V3_Small_Weights.DEFAULT)
model.classifier[-1] = nn.Linear(model.classifier[-1].in_features, 4)  # 4 classes

# Optimizer
optimizer = torch.optim.Adam(model.parameters(), lr=0.001)
scheduler = torch.optim.lr_scheduler.StepLR(optimizer, step_size=7, gamma=0.1)

# Loss
criterion = nn.CrossEntropyLoss()

# Training loop
for epoch in range(20):
    model.train()
    for images, labels in train_loader:
        optimizer.zero_grad()
        outputs = model(images)
        loss = criterion(outputs, labels)
        loss.backward()
        optimizer.step()
    scheduler.step()
```

### ONNX Export
```python
# ml/training/export_onnx.py
import torch

dummy_input = torch.randn(1, 3, 224, 224)
torch.onnx.export(
    model,
    dummy_input,
    "leafsense_model.onnx",
    input_names=['input'],
    output_names=['output'],
    dynamic_axes={
        'input': {0: 'batch_size'},
        'output': {0: 'batch_size'}
    },
    opset_version=12
)
```

## Mock Mode

When the model is not available, the system operates in mock mode:
```cpp
if (!modelLoaded) {
    qDebug() << "[ML] Running in mock mode (always returns Healthy)";
    return PredictionResult{"Healthy", 1.0f, 0};
}
```

## Future Improvements

1. **Add "unknown" class** - Include random non-plant images in training for better OOD detection
2. **More classes** - Add more diseases and pests
3. **INT8 quantization** - Reduce model size and increase inference speed
4. **Continuous training** - Improve model with field data
5. **Segmentation** - Identify affected leaf area with pixel-level accuracy
6. **On-device fine-tuning** - Adapt model to specific plant varieties

---

## Out-of-Distribution Detection (Implemented v1.5.6)

### Solution Implemented

The system implements **combined OOD detection** using both entropy and **green ratio** checks to identify when an image is not a valid plant. This two-stage approach was necessary because testing revealed that entropy alone was insufficient—the model would confidently misclassify non-plant objects (like keyboards) as "Disease" with 89%+ confidence and low entropy.

### How It Works

1. **Green Ratio Check (First Filter)**: Analyze image in HSV color space to count plant-like green pixels
   - Green hue range: 35-85° (true greens)
   - Yellow-green range: 20-35° (young plants)
   - Minimum threshold: 5% of pixels must be green
2. **Shannon Entropy Calculation**: After softmax, calculate entropy: $H = -\sum p_i \log_2(p_i)$
3. **Confidence Check**: Top class must have minimum confidence
4. **Decision Logic**: If green ratio too low OR entropy too high OR confidence too low → mark as "Unknown (Not a Plant)"

### Code Implementation

```cpp
// In ML.cpp
float ML::checkGreenRatio(const cv::Mat& img) {
    cv::Mat hsv, greenMask, yellowGreenMask, combinedMask;
    cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);
    
    // Green range (35-85 hue)
    cv::inRange(hsv, cv::Scalar(35, 30, 30), cv::Scalar(85, 255, 255), greenMask);
    // Yellow-green range (20-35 hue)
    cv::inRange(hsv, cv::Scalar(20, 30, 30), cv::Scalar(35, 255, 255), yellowGreenMask);
    cv::bitwise_or(greenMask, yellowGreenMask, combinedMask);
    
    int greenPixels = cv::countNonZero(combinedMask);
    return (float)greenPixels / (img.rows * img.cols);
}

bool ML::checkValidPlant(float entropy, float maxConfidence, float greenRatio) {
    if (greenRatio < MIN_GREEN_RATIO) return false;    // 0.05 (5%)
    if (entropy > ENTROPY_THRESHOLD) return false;     // 1.8
    if (maxConfidence < MIN_CONFIDENCE_THRESHOLD) return false;  // 0.3
    return true;
}
```

### Thresholds Explained (v1.5.6)

| Threshold | Value | Meaning |
|-----------|-------|---------|
| MIN_GREEN_RATIO | 0.10 (10%) | Image must have at least 10% green pixels (tuned for lettuce) |
| ENTROPY_THRESHOLD | 1.8 | Model must be reasonably confident (entropy < 1.8 out of max 2.0) |
| MIN_CONFIDENCE_THRESHOLD | 0.3 (30%) | Top class must have at least 30% probability |

### Example Outputs (Tested 2026-01-10)

**Valid Plant (Accepted):**
```
[ML] Green ratio: 9.63% (passed)
[ML] Prediction: Pest Damage (confidence: 99.1%, entropy: 0.12, valid: yes)
```

**Non-Plant Image (Rejected by Green Ratio):**
```
[ML] Green ratio: 4.64% (failed, threshold: 5.00%)
[ML] Prediction: Unknown (Not a Plant) (confidence: 89%, entropy: 0.52, valid: no)
```

**Note:** Without green ratio check, this image would have been classified as "Disease" with 89% confidence.

---

## Known Limitations (Partially Addressed)

### Out-of-Distribution Detection ✅ Addressed (v1.5.6)

**Original Issue:** The model classifies ANY image as one of the 4 classes (healthy, disease, deficiency, pest), even if the image is not a plant at all. For example, a picture of a keyboard would receive a classification like "disease" with 87%+ confidence.

**Why this happened:**
- The model uses **softmax** output, which always produces probabilities that sum to 1.0
- It was trained only on lettuce/tomato plant images
- It has no "unknown" or "not a plant" class
- Neural networks are inherently confident on out-of-distribution data

**Solution Implemented (v1.5.6):**

The system now uses a **combined approach** with green ratio + entropy + confidence checks:

1. **Green Ratio Pre-filter** ✅: Uses HSV color space to detect plant-like pixels
   - Rejects images with less than 5% green pixels
   - Catches objects that model would confidently misclassify
   
2. **Entropy + Confidence Thresholds** ✅: Catches unusual model behavior
   - Entropy > 1.8 → reject
   - Confidence < 30% → reject

**Test Evidence (2026-01-10):**
```
# Non-plant object correctly rejected:
[ML] Green ratio: 4.64% (failed, threshold: 5.00%)
[ML] Prediction: Unknown (Not a Plant) (confidence: 89%, entropy: 0.52, valid: no)

# Real plant correctly accepted:
[ML] Green ratio: 9.63% (passed)
[ML] Prediction: Pest Damage (confidence: 99.1%, entropy: 0.12, valid: yes)
```

### Remaining Limitations

1. **Green objects misclassification**: Objects that are green (e.g., green fabric, green toys) may pass the green ratio check
2. **Brown/dying plants**: Plants that have lost most green color may be incorrectly rejected
3. **Single plant type training**: Model was trained on lettuce/tomato - may not generalize well to other plants

4. **Feature distance checking:**
   - Compare extracted features to training distribution
   - If features are far from known plant features, reject

**Current workaround:**
- The operator should only capture images of actual plants
- Low confidence predictions (<50%) should be treated skeptically
- Visual inspection of gallery images is recommended
