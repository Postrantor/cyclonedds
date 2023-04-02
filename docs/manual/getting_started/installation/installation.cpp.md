# Installing C++ API {#installing_cpp}

The C++ API is an implementation of the DDS , that is, a C++ binding for .

> C++API 是 DDS 的一种实现，即的 C++绑定。

The C++ API consists of the following:

- An IDL compiler backend that uses an IDL data model to generate their C++ representation and artifacts.
- A software layer that maps some DDS APIs onto the C API, and to lower the overhead when managing data, direct access to the core APIs.

> - IDL 编译器后端，使用 IDL 数据模型生成其 C++表示和工件。
> - 一个软件层，它将一些 DDS API 映射到 C API 上，并在管理数据时降低开销，直接访问核心 API。

**Obtaining C++ API**

Obtain via Git from the repository hosted on GitHub:

```bash
git clone https://github.com/eclipse-cyclonedds/cyclonedds-cxx.git
cd cyclonedds
```

**Building C++ API**

To build and install the required libraries for your applications, use the following:

> 要为应用程序构建和安装所需的库，请使用以下操作：

```bash
cd build
cmake -DCMAKE_PREFIX_PATH=<core-install-location> -DCMAKE_INSTALL_PREFIX=<install-location> -DBUILD_EXAMPLES=ON ..
cmake --build . --parallel
```

On Windows you can build C++ with one of several generators. Usually, if you omit the `-G <generator-name>` it picks a sensible default. However, if the project does not work, or does something unexpected, refer to the [CMake generators documentation](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html). For example, \"Visual Studio 15 2017 Win64\" targets a 64-bit build using Visual Studio 2017.

> 在 Windows 上，您可以使用几个生成器中的一个生成 C++。通常，如果省略`-G <generator-name>`，它会选择一个合理的默认值。但是，如果项目不起作用，或者做了一些意想不到的事情，请参阅[CMake generators 文档](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html). 例如，“Visual Studio 15 2017 Win64\”针对使用 Visual Studio 2017 的 64 位构建。
