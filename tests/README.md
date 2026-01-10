# Tests Directory

This directory contains test programs and validation utilities for LeafSense.

## Contents

- `test_ml.cpp` - Machine learning inference testing
- `test_ml_dataset.cpp` - ML dataset validation

## Running Tests

```bash
# Compile test_ml
g++ -std=c++17 test_ml.cpp -o test_ml \
    -I../external/onnxruntime-arm64/include \
    -L../external/onnxruntime-arm64/lib \
    -lonnxruntime \
    `pkg-config --cflags --libs opencv4`

# Run
./test_ml ../ml/leafsense_model.onnx test_image.jpg
```

## Notes

- Tests should be kept separate from production source code
- Use for development and validation purposes only
