# Conan package

To simplify the build process even more a `conanfile.py` is provided. This file contains all commands needed to build libasdcp for all different target systems (Linux, Windows, macOS) and architectures (x86_64, armv8) plus its dependencies (OpenSSL and XercesC).
Before you begin make sure [conan package manager](https://conan.io/) is installed.

### Build conan package

```bash
$ conan create . --build missing
```

Optionally you can disable some libasdcp features. You can list them by running `$ conan inspect .` and observing the `options` entry.

To apply them provide them to the connan call:
```bash
$ cmake -o encryption_support=False -o xml_support=False -o jxs_support=False . --build missing
```
