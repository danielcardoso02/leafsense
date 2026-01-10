# LeafSense - Machine Learning

## Overview

The LeafSense Machine Learning system uses an image classification model to detect diseases in tomato plants. The model was trained to identify 4 different classes.

## Classification Classes

| Class | Description | Recommended Action |
|--------|-----------|------------------|
| **Healthy** | Healthy plant | Maintain current conditions |
| **Disease** | Bacterial/fungal diseases | Remove affected leaves, apply treatment |
| **Deficiency** | NPK nutrient deficiency | Adjust nutrient solution |
| **Pest** | Insect damage | Apply pest control measures |

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

## Metrics de Treino

### Resultados Finais
- **Accuracy:** 99.39%
- **Precision:** 99.41%
- **Recall:** 99.39%
- **F1-Score:** 99.39%

### Confusion Matrix (Validation Set)
```
                    Predicted
                 Healthy  Disease  Deficiency  Pest
Actual  Healthy     [76      0         0        0]
        Disease     [ 0    346         0        0]
        Deficiency  [ 0      0        41        0]
        Pest        [ 0      0         0      692]

Total validation images: 1,155 (across 4 classes)
```

### Curvas de Treino
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

### Augmentação de Data
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

## Integration no System

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

### Performance no Raspberry Pi 4B
| Metric | Value |
|---------|-------|
| Tempo de inferência | ~150ms |
| Usage de RAM | ~50MB |
| Usage de CPU | ~80% (1 core) |

## Treino do Template

### Environment de Treino
- **GPU:** NVIDIA RTX 3070
- **Framework:** PyTorch 2.0
- **Python:** 3.10

### Script de Treino
```python
# ml/training/train.py
import torch
import torch.nn as nn
from torchvision import models, transforms
from torch.utils.data import DataLoader

# Template base
model = models.resnet18(pretrained=True)
model.fc = nn.Linear(512, 4)  # 4 classes

# Otimizador
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

### Exportação para ONNX
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

## Modo Mock

Quando o modelo não está disponível, o sistema opera em modo mock:
```cpp
if (!modelLoaded) {
    qDebug() << "[ML] Running in mock mode (always returns Healthy)";
    return PredictionResult{"Healthy", 1.0f, 0};
}
```

## Melhorias Futuras

1. **Mais classes** - Adicionar mais doenças e pragas
2. **Template mais leve** - MobileNetV3 para inferência mais rápida
3. **Quantização INT8** - Reduzir tamanho e aumentar velocidade
4. **Treino contínuo** - Melhorar modelo com dados do campo
5. **Segmentação** - Identificar área afetada da folha
