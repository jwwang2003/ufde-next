# FDE Source Code

## About

Still lots of warnings caused by the old code. I will try to fix them later if I have time.

## Compile

### macOS

```bash
export LIBRARY_PATH=${LIBRARY_PATH}:/usr/local/opt/icu4c/lib
mkdir build && cd build
cmake -GNinja ..
```
