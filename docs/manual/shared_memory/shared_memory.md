::: index
single: Shared memory single: iceoryx; Shared memory
:::

# Shared memory exchange {#shared_memory}

::: {.toctree maxdepth="1" hidden=""}
shared_mem_config shared_mem_examples limitations loan_mechanism developer_hints
:::

This section describes how to support shared memory exchange in , which is based on .

## Prerequisites

depends on several packages (_cmake_, _libacl1_, _libncurses5_, _pkgconfig_ and _maven_).

::: note
::: title
Note
:::

The following steps were done on Ubuntu 20.04.
:::

1.  Install the prerequisite packages:

    ```bash
    sudo apt install cmake libacl1-dev libncurses5-dev pkg-config maven
    ```

2.  Get and build iceoryx. The following assumes that the istall is in your home directory:

    ```bash
    git clone https://github.com/eclipse-iceoryx/iceoryx.git -b release_2.0
    cd iceoryx
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install -DBUILD_SHARED_LIBS=ON -Hiceoryx_meta
    cmake --build build --config Release --target install
    ```

3.  Get and build it with shared memory support:

    ```bash
    git clone https://github.com/eclipse-cyclonedds/cyclonedds.git
    cd cyclonedds
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install -DBUILD_EXAMPLES=On -DCMAKE_PREFIX_PATH=~/iceoryx/install/
    cmake --build build --config Release --target install
    ```

When the compiler has finished, the files for both iceoryx and can be found in the specified install directories.
