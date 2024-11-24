# CMake build

As an alternative to configure (autotools), CMake build files are provided that simplifies the build process for all target systems (Linux, Windows, macOS) and architectures (x86_64, armv8)
In order to use them, you will need to install CMake binaries on your system.
Note: CMake 3.21.1 or higher is required.

### Configuration

```bash
$ mkdir build
$ cd build/
$ cmake ..
```

Optionally you can disable some libasdcp features. You can list them by running: `$ cmake -LH`

To apply them provide them to the cmake call:
```bash
$ cmake -DWITHOUT_SSL=ON -DWITHOUT_XML=ON -DUSE_RANDOM_UUID=OFF ..
```

#### Dependencies

By default libasdcp needs two dependencies (if not deactivated): OpenSSL and XercesC
Make sure both are installed before running CMake. If they are not found automatically you have to provide the installation paths of the dependencies to CMake (see `$ cmake -LH`).

### Build/Install

```bash
$ cmake --build . --parallel
```
