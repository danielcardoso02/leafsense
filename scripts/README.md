# Scripts Directory

Utility scripts for building, deploying, and managing LeafSense.

## Available Scripts

### Build Scripts

- `cross-compile-arm64.sh` - Cross-compile LeafSense for ARM64

### Usage

```bash
# Cross-compile for Raspberry Pi
cd scripts
./cross-compile-arm64.sh
```

## Notes

- Build scripts assume toolchain is set up (see [deploy/README.md](../deploy/README.md))
- Deployment scripts are in the `deploy/` directory
