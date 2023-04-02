Configuration parameters for are expressed in XML and grouped together in an XML file. To use a custom XML configuration in an application, you must set the `CYCLONEDDS_URI` environment variable to the location of the configuration file. For example:

> 的配置参数以 XML 表示并组合在一个 XML 文件中。 要在应用程序中使用自定义 XML 配置，您必须将“CYCLONEDDS_URI”环境变量设置为配置文件的位置。 例如：

Windows

```powershell
set CYCLONEDDS_URI=file://%USERPROFILE%/CycloneDDS/my-config.xml
```

Linux

```bash
export CYCLONEDDS_URI="file://$HOME/CycloneDDS/my-config.xml"
```

The following shows an example XML configuration:

```xml
<?xml version="1.0" encoding="utf-8"?>
<CycloneDDS
  xmlns="https://cdds.io/config"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="https://cdds.io/config https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/master/etc/cyclonedds.xsd"
>
  <Domain Id="any">
    <General>
      <Interfaces>
        <NetworkInterface autodetermine="true" priority="default" multicast="default" />
      </Interfaces>
      <AllowMulticast>default</AllowMulticast>
      <MaxMessageSize>65500B</MaxMessageSize>
    </General>
    <Tracing>
      <Verbosity>config</Verbosity>
      <OutputFile>
        ${HOME}/dds/log/cdds.log.${CYCLONEDDS_PID}
      </OutputFile>
    </Tracing>
  </Domain>
</CycloneDDS>
```

For a full listing of the configuration settings (and default value for each parameter) refer to the `configuration_reference`, which is generated directly from the source code.

The configuration does not depend exclusively on the xml file. The content of the xml can be set directly into the envrionment variable `CYCLONEDDS_URI`. In the following block a example is given for windows and linux. On windows it is important to set the quotation mark directly after the `set` command, otherwise `<` and `>` has to be escaped with `^`.

> **配置并不完全依赖于 xml 文件**。 xml 的内容可以直接设置到环境变量“CYCLONEDDS_URI”中。 在以下块中，给出了适用于 Windows 和 Linux 的示例。 在 Windows 上，在 `set` 命令后直接设置引号很重要，否则 `<` 和 `>` 必须用 `^` 转义。

Windows

```powershell
set "CYCLONEDDS_URI=<CycloneDDS><Domain><General><NetworkInterfaceAddress>127.0.0.1</NetworkInterfaceAddress></General></Domain></CycloneDDS>"
set CYCLONEDDS_URI=^<CycloneDDS^>^<Domain^>^<General^>^<NetworkInterfaceAddress^>127.0.0.1^</NetworkInterfaceAddress^>^</General^>^</Domain^>^</CycloneDDS^>
```

Linux

```bash
export CYCLONEDDS_URI="<CycloneDDS><Domain><General><NetworkInterfaceAddress>127.0.0.1</NetworkInterfaceAddress></General></Domain></CycloneDDS>"
```

The example configuration above is helpfull if you are developing on a machine with activated firewall. Otherwise it would not be possible to send and receive messages between apps on the local machine. The ip `127.0.0.1` expresses that the communication shall be restricted to your pc only (localhost).

> 如果您在激活防火墙的机器上进行开发，上面的示例配置会很有帮助。 否则，将无法在本地计算机上的应用程序之间发送和接收消息。 ip `127.0.0.1` 表示通信仅限于您的电脑（本地主机）。

# Configuration log files

When editing configuration files, the `cdds.log` can be very useful for providing information about the build. To determine the information included in the log file, change the `Tracing/Verbosity <//CycloneDDS/Domain/Tracing/Verbosity>` settings.

> 编辑配置文件时，“cdds.log”对于提供有关构建的信息非常有用。 要确定日志文件中包含的信息，请更改 Tracing/Verbosity <//CycloneDDS/Domain/Tracing/Verbosity> 设置。

cmake_config ddsi_concepts discovery-behavior discovery-config network-config combining-participants data-path-config network-interfaces thread-config reporting-tracing conformance config_file_reference

> cmake_config ddsi_concepts discovery-behavior discovery-config network-config 结合参与者数据路径配置网络接口线程配置报告跟踪一致性 config_file_reference
