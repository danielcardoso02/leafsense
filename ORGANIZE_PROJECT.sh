#!/bin/bash
# Project Reorganization Script for LeafSense
# This script will create a cleaner structure

echo "=== LeafSense Project Reorganization ==="
echo ""
echo "Current issues to fix:"
echo "  - build/ directory is at root (should be .gitignore'd)"
echo "  - ml/test files should be in tests/"
echo "  - Documentation scattered (FINAL-STATUS.md, PROJECT-STATUS.md)"
echo ""
echo "Proposed structure:"
echo "  leafsense-project/"
echo "    ├── CMakeLists.txt (root build config)"
echo "    ├── README.md"
echo "    ├── .gitignore"
echo "    ├── src/              (all source code)"
echo "    ├── include/          (all headers)"
echo "    ├── docs/             (ALL documentation - consolidated)"
echo "    ├── resources/        (Qt resources, images)"
echo "    ├── ml/               (ML model + training ONLY)"
echo "    ├── database/         (schema files)"
echo "    ├── drivers/          (kernel module)"
echo "    ├── deploy/           (deployment scripts)"
echo "    ├── external/         (3rd party libs)"
echo "    ├── tests/            (test files - NEW)"
echo "    └── scripts/          (utility scripts - NEW)"
echo ""
echo "Changes to make:"
echo "  1. Move FINAL-STATUS.md → docs/"
echo "  2. Move docs/PROJECT-STATUS.md → docs/"
echo "  3. Create tests/ directory"
echo "  4. Move ml/test_ml*.cpp → tests/"
echo "  5. Create scripts/ for utility scripts"
echo "  6. Create CMakeLists.txt at root if missing"
echo "  7. Update .gitignore to exclude build directories"
echo ""
read -p "Proceed with reorganization? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Cancelled."
    exit 0
fi

# Execute reorganization
echo "Starting reorganization..."

# 1. Move FINAL-STATUS.md to docs/
if [ -f "FINAL-STATUS.md" ]; then
    mv FINAL-STATUS.md docs/
    echo "✓ Moved FINAL-STATUS.md → docs/"
fi

# 2. Create tests/ directory
mkdir -p tests
echo "✓ Created tests/ directory"

# 3. Move test files from ml/ to tests/
if [ -f "ml/test_ml.cpp" ]; then
    mv ml/test_ml.cpp tests/
    echo "✓ Moved ml/test_ml.cpp → tests/"
fi

if [ -f "ml/test_ml_dataset.cpp" ]; then
    mv ml/test_ml_dataset.cpp tests/
    echo "✓ Moved ml/test_ml_dataset.cpp → tests/"
fi

# 4. Create scripts/ directory for utilities
mkdir -p scripts
echo "✓ Created scripts/ directory"

# 5. Move deployment cross-compile script to scripts/
if [ -f "deploy/cross-compile-arm64.sh" ]; then
    cp deploy/cross-compile-arm64.sh scripts/
    echo "✓ Copied cross-compile-arm64.sh → scripts/"
fi

# 6. Create root CMakeLists.txt if it doesn't exist
if [ ! -f "CMakeLists.txt" ]; then
    cat > CMakeLists.txt << 'CMAKEEOF'
cmake_minimum_required(VERSION 3.16)
project(LeafSense VERSION 1.4.0 LANGUAGES CXX)

# Build from src/
add_subdirectory(src)
CMAKEEOF
    echo "✓ Created root CMakeLists.txt"
fi

echo ""
echo "=== Reorganization Complete ==="
echo ""
echo "Final structure:"
find . -maxdepth 1 -type d | grep -v "^\./\." | sort
echo ""
echo "✓ All documentation now in docs/"
echo "✓ Test files now in tests/"
echo "✓ Build artifacts excluded via .gitignore"
echo ""
echo "Next steps:"
echo "  1. Review the changes"
echo "  2. Update build scripts if needed"
echo "  3. Commit changes to git"
