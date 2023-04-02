---
title: Installing
---

Obtain via Git from the repository hosted on GitHub:

> 通过 Git 从 GitHub 上托管的存储库中获取：

    ```bash
    git clone https://github.com/eclipse-cyclonedds/cyclonedds.git
    cd cyclonedds
    ```

> Building

To build and install the required libraries for your applications, use the following:

> 要为应用程序构建和安装所需的库，请使用以下操作：

    ```bash
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=<install-location> -DBUILD_EXAMPLES=ON ..
    cmake --build . --parallel
    make install
    ```

You can build with one of several generators. Usually, if you omit the `-G <generator-name>`, it selects a sensible default. If it does not work, or selects something unexpected, refer to the [CMake generators documentation](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html). For example, \"Visual Studio 15 2017 Win64\" targets a 64-bit build using Visual Studio 2017.

> 您可以使用几个生成器中的一个进行构建。通常，如果省略“-G<生成器名称>”，它会选择一个合理的默认值。如果它不起作用，或者选择了一些意外的东西，请参阅[CMake generators 文档](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html). 例如，“Visual Studio 15 2017 Win64\”针对使用 Visual Studio 2017 的 64 位构建。

If you need to reduce the footprint, or have issues with the [FindOpenSSL.cmake]{.title-ref} script, you can explicitly disable it by setting `-DENABLE\_SSL=NO` to the CMake invocation. For further information, refer to [FindOpenSSL](https://cmake.org/cmake/help/latest/module/FindOpenSSL.html).

> 如果需要减少占用空间，或者[FindOpenSSL.cmake]｛.title-ref｝脚本有问题，可以通过将“-DENABLE_SL=NO”设置为 cmake 调用来显式禁用它。有关更多信息，请参阅[FindOpenSSL](https://cmake.org/cmake/help/latest/module/FindOpenSSL.html).
