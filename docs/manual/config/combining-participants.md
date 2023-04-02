> &lt;a href="<https://cunit.sourceforge.net/>" target="\_blank"&gt;CUnit&lt;/a&gt;

> &lt;a href="<https://conan.io/>" target="\_blank"&gt;Conan&lt;/a&gt;

> &lt;a href="<https://www.eclipse.org/legal/ECA.php/>"&gt;Eclipse Contributor Agreement&lt;/a&gt;

> &lt;a href="<https://accounts.eclipse.org/user/eca/>"&gt;Eclipse Contributor Agreement&lt;/a&gt;

> &lt;a href="<https://www.omg.org/spec/DDSI-RTPS>" target="\_blank"&gt;DDSI specification&lt;/a&gt;

> &lt;a href="<https://www.omg.org/spec/DDS/>" target="\_blank"&gt;OMG DDS specification&lt;/a&gt;

> &lt;a href="<https://www.omg.org/spec/DDSI-RTPS/2.1>" target="\_blank"&gt;DDSI-RTPS 2.1&lt;/a&gt;

> &lt;a href="<https://www.omg.org/spec/DDSI-RTPS/2.2>" target="\_blank"&gt;DDSI-RTPS 2.2&lt;/a&gt;

> &lt;a href="<https://www.omg.org/spec/DDSI-RTPS/2.5>" target="\_blank"&gt;DDSI-RTPS 2.5&lt;/a&gt;

> &lt;a href="<https://git-scm.com/>" target="\_blank"&gt;Git&lt;/a&gt;

> &lt;a href="<https://cmake.org/>" target="\_blank"&gt;CMake&lt;/a&gt;

> &lt;a href="<https://cmake.org/cmake/help/latest/guide/using-dependencies/index.html#guide:Using%20Dependencies%20Guide>" target="\_blank"&gt;Using dependencies guide&lt;/a&gt;

> &lt;a href="<https://www.openssl.org/>" target="\_blank"&gt;OpenSSL&lt;/a&gt;

> &lt;a href="<https://projects.eclipse.org/proposals/eclipse-iceoryx/>" target="\_blank"&gt;Eclipse iceoryx&lt;/a&gt;

> &lt;a href="<https://cunit.sourceforge.net/>" target="\_blank"&gt;CUnit&lt;/a&gt;

> &lt;a href="<https://www.sphinx-doc.org/en/master/>" target="\_blank"&gt;Sphinx&lt;/a&gt;

> &lt;a href="<https://chocolatey.org/>" target="\_blank"&gt;chocolatey package manager&lt;/a&gt;

> &lt;a href="<https://scoop.sh/>" target="\_blank"&gt;Scoop&lt;/a&gt;

> &lt;a href"<https://www.omg.org/spec/IDL/4.2>" target="\_blank"&gt;OMG IDL&lt;/a&gt;

! Participant, Multiple participants

# Combining multiple participants

If a single process creates multiple participants, these are mirrored in DDSI participants, where a single process can resemble an extensive system with many participants. To simulate the existence of only one participant, which owns all endpoints on that node, set: `Internal/SquashParticipants <//CycloneDDS/Domain/Internal/SquashParticipants>` to `true`. This reduces the background messages because far fewer liveliness assertions need to be sent. However, there are some downsides:

-   This option makes it impossible for the tooling to show the system topology.
-   If multiple DCPS domain participants are combined into a single DDSI domain participant, the liveliness monitoring features related to domain participants are affected . For the "automatic" liveliness setting, this is not an issue (see ).
-   The QoS of the sole participant is the first participant created in the process. That is, no matter what other participants specify as their "user data", it is not visible on remote nodes.

Another option is to set `Internal/BuiltinEndpointSet <//CycloneDDS/Domain/Internal/BuiltinEndpointSet>` to `minimal`. Only the first participant has these Writers and publishes data on all entities.

> Note
>
> This is not fully compatible with other implementations as it means endpoint discovery data can be received for a participant that has not yet been discovered.
