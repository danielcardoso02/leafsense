#!/usr/bin/env python3
"""
Download and prepare a lettuce disease dataset for training.

This script downloads sample images from the internet for testing.
For production, use a proper dataset like:
- PlantVillage (Kaggle)
- Roboflow Lettuce Disease datasets
- Your own collected images

Usage:
    python3 download_dataset.py
"""

import os
import urllib.request
import ssl
import random
from pathlib import Path

# Disable SSL verification for downloads (some sites have cert issues)
ssl._create_default_https_context = ssl._create_unverified_context

# =============================================================================
# Dataset Structure
# =============================================================================

DATASET_DIR = "dataset"
CLASSES = ["healthy", "nutrient_deficiency", "disease", "pest_damage"]

# Sample image URLs (placeholder - replace with real dataset)
# In production, download from Kaggle/Roboflow or use local images
SAMPLE_URLS = {
    "healthy": [
        # Healthy lettuce images would go here
    ],
    "nutrient_deficiency": [
        # Deficiency images would go here
    ],
    "disease": [
        # Disease images would go here  
    ],
    "pest_damage": [
        # Pest damage images would go here
    ]
}

# =============================================================================
# Create Synthetic Dataset (for testing pipeline)
# =============================================================================

def create_synthetic_dataset():
    """
    Creates a small synthetic dataset using color variations
    to test the training pipeline. Replace with real images for production.
    """
    try:
        from PIL import Image
        import numpy as np
    except ImportError:
        print("Installing Pillow...")
        os.system("pip3 install Pillow --user")
        from PIL import Image
        import numpy as np
    
    print("Creating synthetic dataset for pipeline testing...")
    print("(Replace with real lettuce images for production use)\n")
    
    base_dir = Path(DATASET_DIR)
    
    # Number of images per class (per split)
    n_train = 50
    n_val = 10
    
    for split in ["train", "val"]:
        n_images = n_train if split == "train" else n_val
        
        for class_idx, class_name in enumerate(CLASSES):
            class_dir = base_dir / split / class_name
            class_dir.mkdir(parents=True, exist_ok=True)
            
            for i in range(n_images):
                # Create synthetic image with class-specific color bias
                img_array = np.random.randint(0, 256, (224, 224, 3), dtype=np.uint8)
                
                if class_name == "healthy":
                    # Green bias
                    img_array[:, :, 1] = np.clip(img_array[:, :, 1] + 50, 0, 255)
                elif class_name == "nutrient_deficiency":
                    # Yellow bias (green + red)
                    img_array[:, :, 0] = np.clip(img_array[:, :, 0] + 40, 0, 255)
                    img_array[:, :, 1] = np.clip(img_array[:, :, 1] + 40, 0, 255)
                elif class_name == "disease":
                    # Brown/dark spots bias
                    img_array[:, :, 0] = np.clip(img_array[:, :, 0] + 30, 0, 255)
                    # Add random dark spots
                    for _ in range(10):
                        x, y = random.randint(20, 200), random.randint(20, 200)
                        r = random.randint(5, 20)
                        for dx in range(-r, r):
                            for dy in range(-r, r):
                                if dx*dx + dy*dy < r*r:
                                    nx, ny = x + dx, y + dy
                                    if 0 <= nx < 224 and 0 <= ny < 224:
                                        img_array[ny, nx] = img_array[ny, nx] // 2
                elif class_name == "pest_damage":
                    # Add random "holes" (black spots)
                    for _ in range(15):
                        x, y = random.randint(10, 210), random.randint(10, 210)
                        r = random.randint(3, 10)
                        for dx in range(-r, r):
                            for dy in range(-r, r):
                                if dx*dx + dy*dy < r*r:
                                    nx, ny = x + dx, y + dy
                                    if 0 <= nx < 224 and 0 <= ny < 224:
                                        img_array[ny, nx] = 0
                
                img = Image.fromarray(img_array)
                img.save(class_dir / f"{class_name}_{i:04d}.png")
            
            print(f"  Created {n_images} images for {split}/{class_name}")
    
    print(f"\n✓ Synthetic dataset created at: {DATASET_DIR}/")
    print("\nDataset structure:")
    print("  dataset/")
    print("  ├── train/")
    for cls in CLASSES:
        print(f"  │   ├── {cls}/ ({n_train} images)")
    print("  └── val/")
    for cls in CLASSES:
        print(f"      ├── {cls}/ ({n_val} images)")

# =============================================================================
# Download Real Dataset Instructions
# =============================================================================

def print_real_dataset_instructions():
    """Print instructions for getting real datasets"""
    print("""
================================================================================
                    HOW TO GET REAL LETTUCE DATASETS
================================================================================

Option 1: PlantVillage Dataset (Kaggle)
---------------------------------------
1. Go to: https://www.kaggle.com/datasets/emmarex/plantdisease
2. Download and extract
3. Select relevant leaf images
4. Organize into: dataset/train/<class>/ and dataset/val/<class>/

Option 2: Roboflow Lettuce Datasets
-----------------------------------
1. Go to: https://universe.roboflow.com/
2. Search for "lettuce disease" or "plant disease"
3. Download in "folder" format
4. Reorganize folders as needed

Option 3: Collect Your Own Images
---------------------------------
1. Take photos of healthy and diseased lettuce
2. Use at least 50+ images per class for good results
3. Vary lighting, angles, and backgrounds
4. Organize into folders by class

Folder Structure Required:
-------------------------
dataset/
├── train/
│   ├── healthy/          (healthy lettuce images)
│   ├── nutrient_deficiency/  (yellowing, pale leaves)
│   ├── disease/          (spots, mold, rot)
│   └── pest_damage/      (holes, bite marks)
└── val/
    ├── healthy/
    ├── nutrient_deficiency/
    ├── disease/
    └── pest_damage/

After organizing your dataset, run:
    python3 train_model.py --dataset ./dataset --epochs 20

================================================================================
""")

# =============================================================================
# Main
# =============================================================================

def main():
    print("=" * 60)
    print("LeafSense Dataset Preparation")
    print("=" * 60)
    
    if os.path.exists(DATASET_DIR):
        print(f"\nDataset folder already exists: {DATASET_DIR}/")
        response = input("Create synthetic dataset anyway? (y/n): ").lower()
        if response != 'y':
            print_real_dataset_instructions()
            return
    
    print("\nOptions:")
    print("  1. Create synthetic dataset (for testing pipeline)")
    print("  2. Show instructions for real datasets")
    
    choice = input("\nChoose option (1/2): ").strip()
    
    if choice == "1":
        create_synthetic_dataset()
        print("\nTo train the model, run:")
        print("  python3 train_model.py --dataset ./dataset --epochs 10")
    else:
        print_real_dataset_instructions()

if __name__ == "__main__":
    main()
