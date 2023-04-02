If you do not require the examples, use `-DBUILD_EXAMPLES=OFF` to omit them.

To install after a successful build:

```bash
cmake --build . --target install
```

The install step copies everything to:

> - `<install-location>/lib`
> - `<install-location>/bin`
> - `<install-location>/include/ddsc`
> - `<install-location>/share/CycloneDDS`

> [!NOTE]
> Depending on the installation location, you may need administrator privileges.

At this point, you are ready to use Cyclone DDS in your projects.

> Note
> Build types
> The default build type is a release build that includes debugging information (`RelWithDebInfo`). This build is suitable for applications because it allows the resulting application to be more easily debugged while still maintaining high performance. If you prefer a [Debug]{.title-ref} or pure [Release]{.title-ref} build, add `-DCMAKE_BUILD_TYPE=<build-type>` to your CMake invocation.
