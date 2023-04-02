---
title: Windows environment variables
---

To run executables on Windows, the required libraries (`ddsc.dll` and so on) must be available to the executables. Typically, these libraries are installed in system default locations and work out of the box. However, if they are not installed in those locations, you must change the library search path, either:

> 要在 Windows 上运行可执行文件，所需的库（`ddsc.dll` 等）必须可用于可执行文件。 通常，这些库安装在系统默认位置并且开箱即用。 但是，如果它们未安装在这些位置，则必须更改库搜索路径，或者：

- Execute the following command:

```PowerShell
set PATH=<install-location>\bin;%PATH%
```

- Set the path from the \"Environment variables\" Windows menu.

> [!Note]
> An alternative to make the required libraries available to the executables are to copy the necessary libraries for the executables\' directory. This is not recommended.
