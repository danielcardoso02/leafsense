#!/usr/bin/env python3
"""
LeafSense ML Training Script

This script trains a MobileNetV3-Small model for plant disease classification.
Expects a dataset organized in folders by class:

dataset/
├── train/
│   ├── healthy/
│   ├── nutrient_deficiency/
│   ├── disease/
│   └── pest_damage/
└── val/
    ├── healthy/
    ├── nutrient_deficiency/
    ├── disease/
    └── pest_damage/

Usage:
    python3 train_model.py --dataset ./dataset --epochs 20
"""

import os
import argparse
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader
from torchvision import datasets, transforms, models
from datetime import datetime

# =============================================================================
# Configuration
# =============================================================================

BATCH_SIZE = 32
IMAGE_SIZE = 224
NUM_WORKERS = 4

# =============================================================================
# Data Transforms
# =============================================================================

def get_transforms():
    """Returns train and validation transforms"""
    
    train_transforms = transforms.Compose([
        transforms.Resize((IMAGE_SIZE + 32, IMAGE_SIZE + 32)),
        transforms.RandomCrop(IMAGE_SIZE),
        transforms.RandomHorizontalFlip(),
        transforms.RandomVerticalFlip(),
        transforms.RandomRotation(30),
        transforms.ColorJitter(brightness=0.3, contrast=0.3, saturation=0.3, hue=0.1),
        transforms.RandomAffine(degrees=0, translate=(0.1, 0.1), scale=(0.9, 1.1)),
        transforms.ToTensor(),
        transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
    ])
    
    val_transforms = transforms.Compose([
        transforms.Resize((IMAGE_SIZE, IMAGE_SIZE)),
        transforms.ToTensor(),
        transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
    ])
    
    return train_transforms, val_transforms

# =============================================================================
# Model Creation
# =============================================================================

def create_model(num_classes, pretrained=True):
    """Creates MobileNetV3-Small model with custom classifier"""
    
    if pretrained:
        model = models.mobilenet_v3_small(weights=models.MobileNet_V3_Small_Weights.DEFAULT)
    else:
        model = models.mobilenet_v3_small(weights=None)
    
    # Replace classifier
    model.classifier[3] = nn.Linear(model.classifier[3].in_features, num_classes)
    
    return model

# =============================================================================
# Training Loop
# =============================================================================

def train_one_epoch(model, train_loader, criterion, optimizer, device):
    """Train for one epoch"""
    model.train()
    running_loss = 0.0
    correct = 0
    total = 0
    
    for batch_idx, (inputs, labels) in enumerate(train_loader):
        inputs, labels = inputs.to(device), labels.to(device)
        
        optimizer.zero_grad()
        outputs = model(inputs)
        loss = criterion(outputs, labels)
        loss.backward()
        optimizer.step()
        
        running_loss += loss.item()
        _, predicted = outputs.max(1)
        total += labels.size(0)
        correct += predicted.eq(labels).sum().item()
        
        if (batch_idx + 1) % 10 == 0:
            print(f"    Batch {batch_idx+1}/{len(train_loader)} | "
                  f"Loss: {loss.item():.4f} | "
                  f"Acc: {100.*correct/total:.2f}%")
    
    return running_loss / len(train_loader), 100. * correct / total

def validate(model, val_loader, criterion, device):
    """Validate model"""
    model.eval()
    running_loss = 0.0
    correct = 0
    total = 0
    
    with torch.no_grad():
        for inputs, labels in val_loader:
            inputs, labels = inputs.to(device), labels.to(device)
            outputs = model(inputs)
            loss = criterion(outputs, labels)
            
            running_loss += loss.item()
            _, predicted = outputs.max(1)
            total += labels.size(0)
            correct += predicted.eq(labels).sum().item()
    
    return running_loss / len(val_loader), 100. * correct / total

# =============================================================================
# Export to ONNX
# =============================================================================

def export_to_onnx(model, output_path, device):
    """Export trained model to ONNX format"""
    model.eval()
    model.to('cpu')
    
    dummy_input = torch.randn(1, 3, IMAGE_SIZE, IMAGE_SIZE)
    
    torch.onnx.export(
        model,
        dummy_input,
        output_path,
        input_names=["input"],
        output_names=["output"],
        dynamic_axes={
            "input": {0: "batch_size"},
            "output": {0: "batch_size"}
        },
        opset_version=12,
        export_params=True,
        do_constant_folding=True,
        dynamo=False
    )
    
    print(f"✓ Model exported to: {output_path}")

# =============================================================================
# Main Training Function
# =============================================================================

def main(args):
    print("=" * 60)
    print("LeafSense ML Training")
    print("=" * 60)
    
    # Device
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Using device: {device}")
    
    # Load data
    train_transforms, val_transforms = get_transforms()
    
    train_path = os.path.join(args.dataset, "train")
    val_path = os.path.join(args.dataset, "val")
    
    if not os.path.exists(train_path):
        print(f"Error: Training data not found at {train_path}")
        print("\nPlease organize your dataset as:")
        print("  dataset/train/<class_name>/*.jpg")
        print("  dataset/val/<class_name>/*.jpg")
        return
    
    train_dataset = datasets.ImageFolder(train_path, transform=train_transforms)
    
    if os.path.exists(val_path):
        val_dataset = datasets.ImageFolder(val_path, transform=val_transforms)
    else:
        # Split train if no val folder
        print("No validation folder found, using 20% of training data for validation")
        train_size = int(0.8 * len(train_dataset))
        val_size = len(train_dataset) - train_size
        train_dataset, val_dataset = torch.utils.data.random_split(
            train_dataset, [train_size, val_size]
        )
    
    train_loader = DataLoader(
        train_dataset, 
        batch_size=BATCH_SIZE, 
        shuffle=True, 
        num_workers=NUM_WORKERS,
        pin_memory=True
    )
    
    val_loader = DataLoader(
        val_dataset, 
        batch_size=BATCH_SIZE, 
        shuffle=False, 
        num_workers=NUM_WORKERS,
        pin_memory=True
    )
    
    # Get class info
    if hasattr(train_dataset, 'classes'):
        classes = train_dataset.classes
    else:
        classes = train_dataset.dataset.classes
    
    num_classes = len(classes)
    
    print(f"\nClasses found: {classes}")
    print(f"Number of classes: {num_classes}")
    print(f"Training samples: {len(train_dataset)}")
    print(f"Validation samples: {len(val_dataset)}")
    
    # Create model
    model = create_model(num_classes, pretrained=True)
    model = model.to(device)
    
    # Loss and optimizer
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.AdamW(model.parameters(), lr=args.lr, weight_decay=0.01)
    scheduler = optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=args.epochs)
    
    # Training loop
    best_acc = 0.0
    print(f"\nStarting training for {args.epochs} epochs...")
    print("-" * 60)
    
    for epoch in range(args.epochs):
        print(f"\nEpoch {epoch+1}/{args.epochs}")
        print("-" * 40)
        
        train_loss, train_acc = train_one_epoch(
            model, train_loader, criterion, optimizer, device
        )
        
        val_loss, val_acc = validate(model, val_loader, criterion, device)
        
        scheduler.step()
        
        print(f"\n  Train Loss: {train_loss:.4f} | Train Acc: {train_acc:.2f}%")
        print(f"  Val Loss:   {val_loss:.4f} | Val Acc:   {val_acc:.2f}%")
        
        # Save best model
        if val_acc > best_acc:
            best_acc = val_acc
            torch.save({
                'epoch': epoch,
                'model_state_dict': model.state_dict(),
                'optimizer_state_dict': optimizer.state_dict(),
                'best_acc': best_acc,
                'classes': classes,
            }, args.output.replace('.onnx', '_best.pth'))
            print(f"  → Saved best model (Val Acc: {val_acc:.2f}%)")
    
    print("\n" + "=" * 60)
    print(f"Training complete! Best Val Acc: {best_acc:.2f}%")
    print("=" * 60)
    
    # Load best model for export
    checkpoint = torch.load(args.output.replace('.onnx', '_best.pth'))
    model.load_state_dict(checkpoint['model_state_dict'])
    
    # Export to ONNX
    print("\nExporting to ONNX...")
    export_to_onnx(model, args.output, device)
    
    # Save class names
    class_file = args.output.replace('.onnx', '_classes.txt')
    with open(class_file, 'w') as f:
        for cls in classes:
            f.write(cls + '\n')
    print(f"✓ Class names saved to: {class_file}")
    
    print("\n✓ Done! Copy the .onnx file to the build folder to use it.")

# =============================================================================
# Entry Point
# =============================================================================

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Train LeafSense ML model")
    
    parser.add_argument("--dataset", type=str, default="./dataset",
                        help="Path to dataset folder")
    parser.add_argument("--epochs", type=int, default=20,
                        help="Number of training epochs")
    parser.add_argument("--lr", type=float, default=0.001,
                        help="Learning rate")
    parser.add_argument("--output", type=str, default="leafsense_model.onnx",
                        help="Output ONNX model path")
    
    args = parser.parse_args()
    main(args)
