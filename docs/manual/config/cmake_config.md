
CMake; Build configuration

# CMake build configuration

The following table lists some configuration options specified using CMake defines (in addition to the standard options such as `CMAKE_BUILD_TYPE`):

<table>
<colgroup>
<col style="width: 20%" />
<col style="width: 80%" />
</colgroup>
<tbody>
<tr class="odd">
<td><code>-DBUILD_EXAMPLES=ON</code></td>
<td>Build the included examples</td>
</tr>
<tr class="even">
<td><code>-DBUILD_TESTING=ON</code></td>
<td>Build the test suite (this requires ), see <code class="interpreted-text" role="ref">contributing_to_dds</code>.</td>
</tr>
<tr class="odd">
<td><code>-DBUILD_IDLC=NO</code></td>
<td>Disable building the IDL compiler (affects building examples, tests and <code>ddsperf</code>)</td>
</tr>
<tr class="even">
<td><code>-DBUILD_DDSPERF=NO</code></td>
<td>Disable building the <code class="interpreted-text" role="ref">dsperf_tool</code> () tool for performance measurement.</td>
</tr>
<tr class="odd">
<td><code>-DENABLE_SSL=NO</code></td>
<td>Do not look for OpenSSL, remove TLS/TCP support and avoid building the plugins that implement authentication and encryption (default is <code>AUTO</code> to enable them if OpenSSL is found)</td>
</tr>
<tr class="even">
<td><code>-DENABLE_SHM=NO</code></td>
<td>Do not look for and disable <code class="interpreted-text" role="ref">shared_memory</code> (default is <code>AUTO</code> to enable it if iceoryx is found)</td>
</tr>
<tr class="odd">
<td><code>-DENABLE_SECURITY=NO</code></td>
<td>Do not build the security interfaces and hooks in the core code, nor the plugins (you can enable security without OpenSSL present, you'll just have to find plugins elsewhere in that case)</td>
</tr>
<tr class="even">
<td><code>-DENABLE_LIFESPAN=NO</code></td>
<td>Exclude support for finite lifespans QoS</td>
</tr>
<tr class="odd">
<td><code>-DENABLE_DEADLINE_MISSED=NO</code></td>
<td>Exclude support for finite deadline QoS settings</td>
</tr>
<tr class="even">
<td><code>-DENABLE_TYPE_DISCOVERY=NO</code></td>
<td>Exclude support for type discovery and checking type compatibility (effectively most of XTypes), requires also disabling topic discovery using <code>-DENABLE_TOPIC_DISCOVERY=NO</code></td>
</tr>
<tr class="odd">
<td><code>-DENABLE_TOPIC_DISCOVERY=NO</code></td>
<td>Exclude support for topic discovery</td>
</tr>
<tr class="even">
<td><code>-DENABLE_SOURCE_SPECIFIC_MULTICAST=NO</code></td>
<td>Disable support for source-specific multicast (disabling this and <code>-DENABLE_IPV6=NO</code> may be needed for QNX builds)</td>
</tr>
<tr class="odd">
<td><code>-DENABLE_IPV6=NO</code>:</td>
<td>Disable ipv6 support (disabling this and <code>-DENABLE_SOURCE_SPECIFIC_MULTICAST=NO</code> may be needed for QNX builds)</td>
</tr>
<tr class="even">
<td><code>-DBUILD_IDLC_XTESTS=NO</code></td>
<td>Include a set of tests for the IDL compiler that use the C back-end to compile an IDL file at (test) runtime, and use the C compiler to build a test application for the generated types, that is executed to do the actual testing (not supported on Windows)</td>
</tr>
</tbody>
</table>
