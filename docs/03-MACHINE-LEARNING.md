# LeafSense - Machine Learning

## Visão Geral

O sistema de Machine Learning do LeafSense utiliza um modelo de classificação de imagens para detetar doenças em plantas de tomate. O modelo foi treinado para identificar 4 classes diferentes.

## Classes de Classificação

| Classe | Descrição | Ação Recomendada |
|--------|-----------|------------------|
| **Healthy** | Planta saudável | Manter condições atuais |
| **Bacterial_Spot** | Mancha bacteriana | Remover folhas afetadas, aplicar cobre |
| **Early_Blight** | Requeima precoce | Melhorar ventilação, fungicida |
| **Late_Blight** | Requeima tardia | Ação urgente, isolar planta |

## Arquitetura do Modelo

### Base: ResNet18 Modificada
```
Input: 224x224x3 (RGB)
    ↓
Conv2d (64 filters, 7x7)
    ↓
BatchNorm + ReLU + MaxPool
    ↓
ResBlock1 (64 → 64)
    ↓
ResBlock2 (64 → 128)
    ↓
ResBlock3 (128 → 256)
    ↓
ResBlock4 (256 → 512)
    ↓
AdaptiveAvgPool2d
    ↓
Flatten
    ↓
Linear (512 → 4)
    ↓
Output: 4 classes (softmax)
```

## Métricas de Treino

### Resultados Finais
- **Accuracy:** 99.39%
- **Precision:** 99.41%
- **Recall:** 99.39%
- **F1-Score:** 99.39%

### Matriz de Confusão (Validation Set)
```
                    Predicted
                 H    BS   EB   LB
Actual    H    [498   1    0    1]
          BS   [  0 499    1    0]
          EB   [  1   1  498    0]
          LB   [  0   0    0  500]
```

### Curvas de Treino
- **Training Loss:** 0.0089 (final epoch)
- **Validation Loss:** 0.0156 (final epoch)
- **Epochs:** 20
- **Learning Rate:** 0.001 → 0.0001 (scheduler)

## Dataset

### PlantVillage Dataset (Subset)
- **Total de imagens:** 10,000
- **Por classe:** ~2,500 imagens
- **Split:** 80% treino, 10% validação, 10% teste

### Augmentação de Dados
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

## Integração no Sistema

### Ficheiro do Modelo
- **Localização:** `/opt/leafsense/leafsense_model.onnx`
- **Tamanho:** 5.9 MB
- **Formato:** ONNX (Open Neural Network Exchange)

### Código de Inferência (C++)
```cpp
// include/application/ml/ML.h
class MLEngine {
public:
    MLEngine(const std::string& model_path);
    
    // Classificar uma imagem
    PredictionResult predict(const cv::Mat& image);
    
    // Obter todas as probabilidades
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

### Pré-processamento
```cpp
cv::Mat MLEngine::preprocess(const cv::Mat& image) {
    cv::Mat resized, normalized;
    
    // Resize para 224x224
    cv::resize(image, resized, cv::Size(224, 224));
    
    // Converter BGR para RGB
    cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);
    
    // Normalizar com médias ImageNet
    resized.convertTo(normalized, CV_32F, 1.0/255.0);
    
    // Aplicar normalização por canal
    cv::subtract(normalized, cv::Scalar(0.485, 0.456, 0.406), normalized);
    cv::divide(normalized, cv::Scalar(0.229, 0.224, 0.225), normalized);
    
    return normalized;
}
```

## ONNX Runtime

### Configuração ARM64
```cpp
Ort::SessionOptions session_options;
session_options.SetIntraOpNumThreads(4);  // 4 cores do Pi
session_options.SetGraphOptimizationLevel(
    GraphOptimizationLevel::ORT_ENABLE_ALL
);
```

### Performance no Raspberry Pi 4B
| Métrica | Valor |
|---------|-------|
| Tempo de inferência | ~150ms |
| Uso de RAM | ~50MB |
| Uso de CPU | ~80% (1 core) |

## Treino do Modelo

### Ambiente de Treino
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

# Modelo base
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
2. **Modelo mais leve** - MobileNetV3 para inferência mais rápida
3. **Quantização INT8** - Reduzir tamanho e aumentar velocidade
4. **Treino contínuo** - Melhorar modelo com dados do campo
5. **Segmentação** - Identificar área afetada da folha
