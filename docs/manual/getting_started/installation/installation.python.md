# Installing Python {#installing_python}

**Binaries or from source**

The Python API requires Python version 3.7 or higher (with 3.11 support provisional).

At runtime, there are several mechanisms to locate the appropriate library for the platform. If you get an exception about non-locatable libraries, or need to manage multiple installations, override the load location by setting the `CYCLONEDDS_HOME` environment variable.

> 在运行时，有几种机制可以为平台找到合适的库。 如果您收到有关不可定位库的异常，或者需要管理多个安装，请通过设置“CYCLONEDDS_HOME”环境变量来覆盖加载位置。

**Installing from PyPi**

The wheels (binary archives) on PyPi contain a pre-built binary of the CycloneDDS C library and IDL compiler. However, the pre-built package:

> - does not provide support for DDS Security,
> - does not provide support for shared memory via ,
> - comes with generic binaries that are not optimized per platform.

If you need these features, or cannot use the binaries for other reasons, install the Python API from source (see below). If the C library is not on the `PATH`, set the environment variable `CYCLONEDDS_HOME`.

Install using pip directly from PyPi.

```shell
pip install cyclonedds
```

**Installing from source**

Install directly from the GitHub link:

```shell
CYCLONEDDS_HOME="<cyclonedds-install-location>" pip install git+https://github.com/eclipse-cyclonedds/cyclonedds-python
```
