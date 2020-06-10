# This is my fork of the famous BusTub simple DB created at CMU for educational purpose

BusTub is a relational database management system built at [Carnegie Mellon University](https://db.cs.cmu.edu) for the [Introduction to Database Systems](https://15445.courses.cs.cmu.edu) (15-445/645) course. This system was developed for educational purposes and should not be used in production environments. Its GitHub url is https://github.com/cmu-db/bustub.

The focus of this repository is learning, therefore, it is not guaranteed that its implementation is correct.

## Build

### Linux / Mac
To ensure that you have the proper packages installed on your machine, run `sudo build_support/packages.sh`. Then run

```
mkdir build
cd build
cmake ..
make
```
Debug mode:

```
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```
Debug build enables [AddressSanitizer](https://github.com/google/sanitizers), which can generate false positives for overflow on STL containers. If you encounter this, define the environment variable `ASAN_OPTIONS=detect_container_overflow=0`.

## Testing
```
cd build
make check-tests
```
