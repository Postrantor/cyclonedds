# Prerequisites {#software_prerequisites}

Install the following software on your machine:

> - A C compiler (For example, GCC or Clang on Linux, Visual Studio on Windows (MSVC), Clang on macOS).
> - version control system.
> - , version 3.10 or later, see `cmake_config`{.interpreted-text role="ref"}.
> - Optionally, , preferably version 1.1 later to use TLS over TCP.

To obtain the dependencies for , follow the platform-specific instructions:

::: tabs
::: group-tab
Linux

To install the dependencies, use a package manager. For example:

```bash
yum install git cmake gcc
apt-get install git cmake gcc
aptitude install git cmake gcc
# or others
```

:::

::: group-tab
macOS

Install XCode from the App Store.
:::

::: group-tab
Windows

Install Visual Studio Code for the C compiler, then install the .

```bash
choco install cmake
choco install git
```

Alternatively, to install the dependencies, use .
:::
:::

## Additional tools {#tools}

While developing for , additional tools and dependencies may be required. The following is a list of the suggested tools:

> - Shared memory
>
>   :
>
> - Unit testing / Development
>
>   :
>
> - Documentation
>
>   :
>
> - Security
>
>   :
