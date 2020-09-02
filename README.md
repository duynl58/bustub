# Build

## Link: https://15445.courses.cs.cmu.edu/fall2019/

- [x] Buffer Pool Manager
- [x] Hash Index
- [x] Query Execution
- [ ] Logging & Recovery (Partially done)

As I recently join a university to pursue a Ph.D. degree, probably this won't receive any more commit. Note that all code here is tested with the sample tests (as well as some are mine added later), so it is not guaranteed that it is 100% correct.

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
