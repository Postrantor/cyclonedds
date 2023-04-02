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

# Performance test

Reliable message throughput is over 1MS/s for very small samples and is roughly 90% of GbE with 100 byte samples. Latency is about 30us when measured using `_dsperf_tool`{.interpreted-text role="ref"} between two Intel(R) Xeon(R) CPU E3-1270 V2 @ 3.50GHz (2012 hardware) running Ubuntu 16.04, with the executables built on Ubuntu 18.04 using gcc 7.4.0 for a default (that is, "RelWithDebInfo") build.

> **对于非常小的样本，可靠的消息吞吐量超过 1MS/s，对于 100 字节样本，大约是 GbE 的 90%。**在运行 Ubuntu 16.04 的两个 Intel（R）Xeon（R）CPU E3-1270 V2@3.50GHz（2012 硬件）之间，使用`_dsperf_tool`｛.depreced text role=“ref”｝测量延迟约为 30us，在 Ubuntu 18.04 上构建的可执行文件使用 gcc 7.4.0 作为默认版本（即“RelWithDebInfo”）。

The following show the performance of a reliable message:

> 以下显示了可靠消息的性能：

![Throughput async listener rate](../_static/gettingstarted-figures/throughput-async-listener-rate.png)
![Latency sync listener](../_static/gettingstarted-figures/latency-sync-listener.png)

This is with the subscriber in listener mode, using asynchronous delivery for the throughput test.

> 这是针对处于侦听器模式的订阅者，使用异步传递进行吞吐量测试。

The configuration is a marginally tweaked out-of-the-box configuration: an increased maximum message size and fragment size, and an increased high-water mark for the reliability window on the writer side.

> 该配置是一种稍微调整过的开箱即用配置：增加了最大消息大小和片段大小，并增加了写入端可靠性窗口的高水位线。

## Test setup

The details of the test setup can be found via the following links:

## Test results

data underlying the graphs (including CPU usage):

![Throughput async listener cpu](../_static/gettingstarted-figures/throughput-async-listener-cpu.png)
![Latency sync listener bwcpu](../_static/gettingstarted-figures/latency-sync-listener-bwcpu.png)
![Throughput async listener memory](../_static/gettingstarted-figures/throughput-async-listener-memory.png)
