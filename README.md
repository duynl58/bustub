# Build

## Linux / Mac
To ensure that you have the proper packages installed on your machine, run `sudo build_support/packages.sh`. Then run:

```
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```
Debug build enables [AddressSanitizer](https://github.com/google/sanitizers), which can generate false positives for overflow on STL containers. If you encounter this, define the environment variable `ASAN_OPTIONS=detect_container_overflow=0`.

# Testing
```
cd build
make check-tests
```
