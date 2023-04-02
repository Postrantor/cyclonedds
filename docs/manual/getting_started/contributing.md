<a href="<https://cunit.sourceforge.net/>" target="\_blank"\>CUnit</a>
<a href="<https://conan.io/>" target="\_blank"\>Conan</a>
<a href="<https://www.eclipse.org/legal/ECA.php/>"\>Eclipse Contributor Agreement</a>
<a href="<https://accounts.eclipse.org/user/eca/>"\>Eclipse Contributor Agreement</a>
<a href="<https://www.omg.org/spec/DDSI-RTPS>" target="\_blank"\>DDSI specification</a>
<a href="<https://www.omg.org/spec/DDS/>" target="\_blank"\>OMG DDS specification</a>
<a href="<https://www.omg.org/spec/DDSI-RTPS/2.1>" target="\_blank"\>DDSI-RTPS 2.1</a>
<a href="<https://www.omg.org/spec/DDSI-RTPS/2.2>" target="\_blank"\>DDSI-RTPS 2.2</a>
<a href="<https://www.omg.org/spec/DDSI-RTPS/2.5>" target="\_blank"\>DDSI-RTPS 2.5</a>
<a href="<https://git-scm.com/>" target="\_blank"\>Git</a>
<a href="<https://cmake.org/>" target="\_blank"\>CMake</a>

<a href="<https://www.openssl.org/>" target="\_blank"\>OpenSSL</a>

<a href="<https://cunit.sourceforge.net/>" target="\_blank"\>CUnit</a>
<a href="<https://www.sphinx-doc.org/en/master/>" target="\_blank"\>Sphinx</a>
<a href="<https://chocolatey.org/>" target="\_blank"\>chocolatey package manager</a>
<a href="<https://scoop.sh/>" target="\_blank"\>Scoop</a>
<a href"<https://www.omg.org/spec/IDL/4.2>" target="\_blank"\>OMG IDL</a>

# Contributing to {#contributing_to_dds}

We welcome all contributions to the project, including questions, examples, bug fixes, enhancements or improvements to the documentation, etc.

> 我们欢迎所有对项目的贡献，包括问题、示例、错误修复、文档的增强或改进等。

> [!TIP]
> Contributing to means donating your code to the Eclipse foundation. It requires that you sign the using . In summary, this means that your contribution is yours to give away, and that you allow others to use and distribute it. However, don\'t take legal advice from this getting started guide, read the terms linked above.
> 贡献意味着将您的代码捐赠给 Eclipse 基金会。它要求您在使用中签名。总之，这意味着你的贡献是你的，你允许他人使用和分发。但是，不要从本入门指南中获取法律建议，请阅读上面链接的条款。

To contribute code, it may be helpful to know that build configurations for Azure DevOps Pipelines are present in the repositories. There is a test suite using CTest and that can be built locally.

> 为了贡献代码，了解 Azure DevOps 管道的构建配置是否存在于存储库中可能会有所帮助。有一个使用 CTest 的测试套件，可以在本地构建。

The following sections explain how to do this for the different operating systems.

> 以下部分解释了如何针对不同的操作系统执行此操作。

## Linux and macOS

Set the CMake variable `BUILD_TESTING` to `ON` when configuring, e.g.:

> 配置时，将 CMake 变量“BUILD_TESTING”设置为“ON”，例如：

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON ..
cmake --build .
ctest
```

This build requires . You can install this yourself, or you can choose to instead rely on the packaging system that the CI build infrastructure also uses. In that case, install Conan in the build directory before running CMake:

> 此生成需要。您可以自己安装，也可以选择依赖 CI 构建基础结构也使用的打包系统。在这种情况下，请在运行 CMake 之前将 Conan 安装在构建目录中：

```bash
conan install .. --build missing
```

## Windows

Set the CMake variable `BUILD_TESTING` to `ON` when configuring, e.g.:

> 配置时，将 CMake 变量“BUILD_TESTING”设置为“ON”，例如：

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON ..
cmake --build .
ctest
```

This build requires . You can install this yourself, or you can choose to instead rely on the packaging system that the CI build infrastructure also uses. In that case, install Conan in the build directory before running CMake:

> 此生成需要。您可以自己安装，也可以选择依赖 CI 构建基础结构也使用的打包系统。在这种情况下，请在运行 CMake 之前将 Conan 安装在构建目录中：

```bash
conan install .. --build missing
```

This automatically downloads and builds CUnit (and currently OpenSSL for transport security).

> 这会自动下载和构建 CUnit（以及目前用于传输安全的 OpenSSL）。

> [!NOTE]
> Depending on the generator, you may also need to add switches to select the architecture and build type, e.g.:
> 根据生成器的不同，您可能还需要添加交换机来选择体系结构和构建类型，例如：

```bash
conan install -s arch=x86_64 -s build_type=Debug ..
```
