---
Below is the full (generated) reference of XML you can use to configure . The title of each section is the XML XPath notation to the relevant option.
> 以下是您可以用于配置的 XML 的完整(生成)参考。每个部分的标题是相关选项的 XML XPATH 表示法。
---

- [//CycloneDDS](#cyclonedds)
  - [//CycloneDDS/Domain](#cycloneddsdomain)
  - [//CycloneDDS/Domain\[@Id\]](#cycloneddsdomainid)
    - [//CycloneDDS/Domain/Compatibility](#cycloneddsdomaincompatibility)
      - [//CycloneDDS/Domain/Compatibility/AssumeRtiHasPmdEndpoints](#cycloneddsdomaincompatibilityassumertihaspmdendpoints)
      - [//CycloneDDS/Domain/Compatibility/ExplicitlyPublishQosSetToDefault](#cycloneddsdomaincompatibilityexplicitlypublishqossettodefault)
      - [//CycloneDDS/Domain/Compatibility/ManySocketsMode](#cycloneddsdomaincompatibilitymanysocketsmode)
      - [//CycloneDDS/Domain/Compatibility/StandardsConformance](#cycloneddsdomaincompatibilitystandardsconformance)
    - [//CycloneDDS/Domain/Discovery](#cycloneddsdomaindiscovery)
      - [//CycloneDDS/Domain/Discovery/DSGracePeriod](#cycloneddsdomaindiscoverydsgraceperiod)
      - [//CycloneDDS/Domain/Discovery/DefaultMulticastAddress](#cycloneddsdomaindiscoverydefaultmulticastaddress)
      - [//CycloneDDS/Domain/Discovery/EnableTopicDiscoveryEndpoints](#cycloneddsdomaindiscoveryenabletopicdiscoveryendpoints)
      - [//CycloneDDS/Domain/Discovery/ExternalDomainId](#cycloneddsdomaindiscoveryexternaldomainid)
      - [//CycloneDDS/Domain/Discovery/LeaseDuration](#cycloneddsdomaindiscoveryleaseduration)
      - [//CycloneDDS/Domain/Discovery/MaxAutoParticipantIndex](#cycloneddsdomaindiscoverymaxautoparticipantindex)
      - [//CycloneDDS/Domain/Discovery/ParticipantIndex](#cycloneddsdomaindiscoveryparticipantindex)
      - [//CycloneDDS/Domain/Discovery/Peers](#cycloneddsdomaindiscoverypeers)
        - [//CycloneDDS/Domain/Discovery/Peers/Peer](#cycloneddsdomaindiscoverypeerspeer)
        - [//CycloneDDS/Domain/Discovery/Peers/Peer\[@Address\]](#cycloneddsdomaindiscoverypeerspeeraddress)
      - [//CycloneDDS/Domain/Discovery/Ports](#cycloneddsdomaindiscoveryports)
        - [//CycloneDDS/Domain/Discovery/Ports/Base](#cycloneddsdomaindiscoveryportsbase)
        - [//CycloneDDS/Domain/Discovery/Ports/DomainGain](#cycloneddsdomaindiscoveryportsdomaingain)
        - [//CycloneDDS/Domain/Discovery/Ports/MulticastDataOffset](#cycloneddsdomaindiscoveryportsmulticastdataoffset)
        - [//CycloneDDS/Domain/Discovery/Ports/MulticastMetaOffset](#cycloneddsdomaindiscoveryportsmulticastmetaoffset)
        - [//CycloneDDS/Domain/Discovery/Ports/ParticipantGain](#cycloneddsdomaindiscoveryportsparticipantgain)
        - [//CycloneDDS/Domain/Discovery/Ports/UnicastDataOffset](#cycloneddsdomaindiscoveryportsunicastdataoffset)
        - [//CycloneDDS/Domain/Discovery/Ports/UnicastMetaOffset](#cycloneddsdomaindiscoveryportsunicastmetaoffset)
      - [//CycloneDDS/Domain/Discovery/SPDPInterval](#cycloneddsdomaindiscoveryspdpinterval)
      - [//CycloneDDS/Domain/Discovery/SPDPMulticastAddress](#cycloneddsdomaindiscoveryspdpmulticastaddress)
      - [//CycloneDDS/Domain/Discovery/Tag](#cycloneddsdomaindiscoverytag)
    - [//CycloneDDS/Domain/General](#cycloneddsdomaingeneral)
      - [//CycloneDDS/Domain/General/AllowMulticast](#cycloneddsdomaingeneralallowmulticast)
      - [//CycloneDDS/Domain/General/DontRoute](#cycloneddsdomaingeneraldontroute)
      - [//CycloneDDS/Domain/General/EnableMulticastLoopback](#cycloneddsdomaingeneralenablemulticastloopback)
      - [//CycloneDDS/Domain/General/EntityAutoNaming](#cycloneddsdomaingeneralentityautonaming)
      - [//CycloneDDS/Domain/General/EntityAutoNaming\[@seed\]](#cycloneddsdomaingeneralentityautonamingseed)
      - [//CycloneDDS/Domain/General/ExternalNetworkAddress](#cycloneddsdomaingeneralexternalnetworkaddress)
      - [//CycloneDDS/Domain/General/ExternalNetworkMask](#cycloneddsdomaingeneralexternalnetworkmask)
      - [//CycloneDDS/Domain/General/FragmentSize](#cycloneddsdomaingeneralfragmentsize)
      - [//CycloneDDS/Domain/General/Interfaces](#cycloneddsdomaingeneralinterfaces)
        - [//CycloneDDS/Domain/General/Interfaces/NetworkInterface](#cycloneddsdomaingeneralinterfacesnetworkinterface)
        - [//CycloneDDS/Domain/General/Interfaces/NetworkInterface\[@address\]](#cycloneddsdomaingeneralinterfacesnetworkinterfaceaddress)
        - [//CycloneDDS/Domain/General/Interfaces/NetworkInterface\[@autodetermine\]](#cycloneddsdomaingeneralinterfacesnetworkinterfaceautodetermine)
        - [//CycloneDDS/Domain/General/Interfaces/NetworkInterface\[@multicast\]](#cycloneddsdomaingeneralinterfacesnetworkinterfacemulticast)
        - [//CycloneDDS/Domain/General/Interfaces/NetworkInterface\[@name\]](#cycloneddsdomaingeneralinterfacesnetworkinterfacename)
        - [//CycloneDDS/Domain/General/Interfaces/NetworkInterface\[@prefer\_multicast\]](#cycloneddsdomaingeneralinterfacesnetworkinterfaceprefer_multicast)
        - [//CycloneDDS/Domain/General/Interfaces/NetworkInterface\[@presence\_required\]](#cycloneddsdomaingeneralinterfacesnetworkinterfacepresence_required)
        - [//CycloneDDS/Domain/General/Interfaces/NetworkInterface\[@priority\]](#cycloneddsdomaingeneralinterfacesnetworkinterfacepriority)
      - [//CycloneDDS/Domain/General/MaxMessageSize](#cycloneddsdomaingeneralmaxmessagesize)
      - [//CycloneDDS/Domain/General/MaxRexmitMessageSize](#cycloneddsdomaingeneralmaxrexmitmessagesize)
      - [//CycloneDDS/Domain/General/MulticastRecvNetworkInterfaceAddresses](#cycloneddsdomaingeneralmulticastrecvnetworkinterfaceaddresses)
      - [//CycloneDDS/Domain/General/MulticastTimeToLive](#cycloneddsdomaingeneralmulticasttimetolive)
      - [//CycloneDDS/Domain/General/RedundantNetworking](#cycloneddsdomaingeneralredundantnetworking)
      - [//CycloneDDS/Domain/General/Transport](#cycloneddsdomaingeneraltransport)
      - [//CycloneDDS/Domain/General/UseIPv6](#cycloneddsdomaingeneraluseipv6)
    - [//CycloneDDS/Domain/Internal](#cycloneddsdomaininternal)
      - [//CycloneDDS/Domain/Internal/AccelerateRexmitBlockSize](#cycloneddsdomaininternalacceleraterexmitblocksize)
      - [//CycloneDDS/Domain/Internal/AckDelay](#cycloneddsdomaininternalackdelay)
      - [//CycloneDDS/Domain/Internal/AutoReschedNackDelay](#cycloneddsdomaininternalautoreschednackdelay)
      - [//CycloneDDS/Domain/Internal/BuiltinEndpointSet](#cycloneddsdomaininternalbuiltinendpointset)
      - [//CycloneDDS/Domain/Internal/BurstSize](#cycloneddsdomaininternalburstsize)
        - [//CycloneDDS/Domain/Internal/BurstSize/MaxInitTransmit](#cycloneddsdomaininternalburstsizemaxinittransmit)
        - [//CycloneDDS/Domain/Internal/BurstSize/MaxRexmit](#cycloneddsdomaininternalburstsizemaxrexmit)
      - [//CycloneDDS/Domain/Internal/ControlTopic](#cycloneddsdomaininternalcontroltopic)
      - [//CycloneDDS/Domain/Internal/DefragReliableMaxSamples](#cycloneddsdomaininternaldefragreliablemaxsamples)
      - [//CycloneDDS/Domain/Internal/DefragUnreliableMaxSamples](#cycloneddsdomaininternaldefragunreliablemaxsamples)
      - [//CycloneDDS/Domain/Internal/DeliveryQueueMaxSamples](#cycloneddsdomaininternaldeliveryqueuemaxsamples)
      - [//CycloneDDS/Domain/Internal/EnableExpensiveChecks](#cycloneddsdomaininternalenableexpensivechecks)
      - [//CycloneDDS/Domain/Internal/GenerateKeyhash](#cycloneddsdomaininternalgeneratekeyhash)
      - [//CycloneDDS/Domain/Internal/HeartbeatInterval](#cycloneddsdomaininternalheartbeatinterval)
      - [//CycloneDDS/Domain/Internal/HeartbeatInterval\[@max\]](#cycloneddsdomaininternalheartbeatintervalmax)
      - [//CycloneDDS/Domain/Internal/HeartbeatInterval\[@min\]](#cycloneddsdomaininternalheartbeatintervalmin)
      - [//CycloneDDS/Domain/Internal/HeartbeatInterval\[@minsched\]](#cycloneddsdomaininternalheartbeatintervalminsched)
      - [//CycloneDDS/Domain/Internal/LateAckMode](#cycloneddsdomaininternallateackmode)
      - [//CycloneDDS/Domain/Internal/LivelinessMonitoring](#cycloneddsdomaininternallivelinessmonitoring)
      - [//CycloneDDS/Domain/Internal/LivelinessMonitoring\[@Interval\]](#cycloneddsdomaininternallivelinessmonitoringinterval)
      - [//CycloneDDS/Domain/Internal/LivelinessMonitoring\[@StackTraces\]](#cycloneddsdomaininternallivelinessmonitoringstacktraces)
      - [//CycloneDDS/Domain/Internal/MaxParticipants](#cycloneddsdomaininternalmaxparticipants)
      - [//CycloneDDS/Domain/Internal/MaxQueuedRexmitBytes](#cycloneddsdomaininternalmaxqueuedrexmitbytes)
      - [//CycloneDDS/Domain/Internal/MaxQueuedRexmitMessages](#cycloneddsdomaininternalmaxqueuedrexmitmessages)
      - [//CycloneDDS/Domain/Internal/MaxSampleSize](#cycloneddsdomaininternalmaxsamplesize)
      - [//CycloneDDS/Domain/Internal/MeasureHbToAckLatency](#cycloneddsdomaininternalmeasurehbtoacklatency)
      - [//CycloneDDS/Domain/Internal/MonitorPort](#cycloneddsdomaininternalmonitorport)
      - [//CycloneDDS/Domain/Internal/MultipleReceiveThreads](#cycloneddsdomaininternalmultiplereceivethreads)
      - [//CycloneDDS/Domain/Internal/MultipleReceiveThreads\[@maxretries\]](#cycloneddsdomaininternalmultiplereceivethreadsmaxretries)
      - [//CycloneDDS/Domain/Internal/NackDelay](#cycloneddsdomaininternalnackdelay)
      - [//CycloneDDS/Domain/Internal/PreEmptiveAckDelay](#cycloneddsdomaininternalpreemptiveackdelay)
      - [//CycloneDDS/Domain/Internal/PrimaryReorderMaxSamples](#cycloneddsdomaininternalprimaryreordermaxsamples)
      - [//CycloneDDS/Domain/Internal/PrioritizeRetransmit](#cycloneddsdomaininternalprioritizeretransmit)
      - [//CycloneDDS/Domain/Internal/RediscoveryBlacklistDuration](#cycloneddsdomaininternalrediscoveryblacklistduration)
      - [//CycloneDDS/Domain/Internal/RediscoveryBlacklistDuration\[@enforce\]](#cycloneddsdomaininternalrediscoveryblacklistdurationenforce)
      - [//CycloneDDS/Domain/Internal/RetransmitMerging](#cycloneddsdomaininternalretransmitmerging)
      - [//CycloneDDS/Domain/Internal/RetransmitMergingPeriod](#cycloneddsdomaininternalretransmitmergingperiod)
      - [//CycloneDDS/Domain/Internal/RetryOnRejectBestEffort](#cycloneddsdomaininternalretryonrejectbesteffort)
      - [//CycloneDDS/Domain/Internal/SPDPResponseMaxDelay](#cycloneddsdomaininternalspdpresponsemaxdelay)
      - [//CycloneDDS/Domain/Internal/ScheduleTimeRounding](#cycloneddsdomaininternalscheduletimerounding)
      - [//CycloneDDS/Domain/Internal/SecondaryReorderMaxSamples](#cycloneddsdomaininternalsecondaryreordermaxsamples)
      - [//CycloneDDS/Domain/Internal/SocketReceiveBufferSize](#cycloneddsdomaininternalsocketreceivebuffersize)
      - [//CycloneDDS/Domain/Internal/SocketReceiveBufferSize\[@max\]](#cycloneddsdomaininternalsocketreceivebuffersizemax)
      - [//CycloneDDS/Domain/Internal/SocketReceiveBufferSize\[@min\]](#cycloneddsdomaininternalsocketreceivebuffersizemin)
      - [//CycloneDDS/Domain/Internal/SocketSendBufferSize](#cycloneddsdomaininternalsocketsendbuffersize)
      - [//CycloneDDS/Domain/Internal/SocketSendBufferSize\[@max\]](#cycloneddsdomaininternalsocketsendbuffersizemax)
      - [//CycloneDDS/Domain/Internal/SocketSendBufferSize\[@min\]](#cycloneddsdomaininternalsocketsendbuffersizemin)
      - [//CycloneDDS/Domain/Internal/SquashParticipants](#cycloneddsdomaininternalsquashparticipants)
      - [//CycloneDDS/Domain/Internal/SynchronousDeliveryLatencyBound](#cycloneddsdomaininternalsynchronousdeliverylatencybound)
      - [//CycloneDDS/Domain/Internal/SynchronousDeliveryPriorityThreshold](#cycloneddsdomaininternalsynchronousdeliveryprioritythreshold)
      - [//CycloneDDS/Domain/Internal/Test](#cycloneddsdomaininternaltest)
        - [//CycloneDDS/Domain/Internal/Test/XmitLossiness](#cycloneddsdomaininternaltestxmitlossiness)
      - [//CycloneDDS/Domain/Internal/UnicastResponseToSPDPMessages](#cycloneddsdomaininternalunicastresponsetospdpmessages)
      - [//CycloneDDS/Domain/Internal/UseMulticastIfMreqn](#cycloneddsdomaininternalusemulticastifmreqn)
      - [//CycloneDDS/Domain/Internal/Watermarks](#cycloneddsdomaininternalwatermarks)
        - [//CycloneDDS/Domain/Internal/Watermarks/WhcAdaptive](#cycloneddsdomaininternalwatermarkswhcadaptive)
        - [//CycloneDDS/Domain/Internal/Watermarks/WhcHigh](#cycloneddsdomaininternalwatermarkswhchigh)
        - [//CycloneDDS/Domain/Internal/Watermarks/WhcHighInit](#cycloneddsdomaininternalwatermarkswhchighinit)
        - [//CycloneDDS/Domain/Internal/Watermarks/WhcLow](#cycloneddsdomaininternalwatermarkswhclow)
      - [//CycloneDDS/Domain/Internal/WriteBatch](#cycloneddsdomaininternalwritebatch)
      - [//CycloneDDS/Domain/Internal/WriterLingerDuration](#cycloneddsdomaininternalwriterlingerduration)
    - [//CycloneDDS/Domain/Partitioning](#cycloneddsdomainpartitioning)
      - [//CycloneDDS/Domain/Partitioning/IgnoredPartitions](#cycloneddsdomainpartitioningignoredpartitions)
        - [//CycloneDDS/Domain/Partitioning/IgnoredPartitions/IgnoredPartition](#cycloneddsdomainpartitioningignoredpartitionsignoredpartition)
        - [//CycloneDDS/Domain/Partitioning/IgnoredPartitions/IgnoredPartition\[@DCPSPartitionTopic\]](#cycloneddsdomainpartitioningignoredpartitionsignoredpartitiondcpspartitiontopic)
      - [//CycloneDDS/Domain/Partitioning/NetworkPartitions](#cycloneddsdomainpartitioningnetworkpartitions)
        - [//CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition](#cycloneddsdomainpartitioningnetworkpartitionsnetworkpartition)
        - [//CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition\[@Address\]](#cycloneddsdomainpartitioningnetworkpartitionsnetworkpartitionaddress)
        - [//CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition\[@Interface\]](#cycloneddsdomainpartitioningnetworkpartitionsnetworkpartitioninterface)
        - [//CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition\[@Name\]](#cycloneddsdomainpartitioningnetworkpartitionsnetworkpartitionname)
      - [//CycloneDDS/Domain/Partitioning/PartitionMappings](#cycloneddsdomainpartitioningpartitionmappings)
        - [//CycloneDDS/Domain/Partitioning/PartitionMappings/PartitionMapping](#cycloneddsdomainpartitioningpartitionmappingspartitionmapping)
        - [//CycloneDDS/Domain/Partitioning/PartitionMappings/PartitionMapping\[@DCPSPartitionTopic\]](#cycloneddsdomainpartitioningpartitionmappingspartitionmappingdcpspartitiontopic)
        - [//CycloneDDS/Domain/Partitioning/PartitionMappings/PartitionMapping\[@NetworkPartition\]](#cycloneddsdomainpartitioningpartitionmappingspartitionmappingnetworkpartition)
    - [//CycloneDDS/Domain/SSL](#cycloneddsdomainssl)
      - [//CycloneDDS/Domain/SSL/CertificateVerification](#cycloneddsdomainsslcertificateverification)
      - [//CycloneDDS/Domain/SSL/Ciphers](#cycloneddsdomainsslciphers)
      - [//CycloneDDS/Domain/SSL/Enable](#cycloneddsdomainsslenable)
      - [//CycloneDDS/Domain/SSL/EntropyFile](#cycloneddsdomainsslentropyfile)
      - [//CycloneDDS/Domain/SSL/KeyPassphrase](#cycloneddsdomainsslkeypassphrase)
      - [//CycloneDDS/Domain/SSL/KeystoreFile](#cycloneddsdomainsslkeystorefile)
      - [//CycloneDDS/Domain/SSL/MinimumTLSVersion](#cycloneddsdomainsslminimumtlsversion)
      - [//CycloneDDS/Domain/SSL/SelfSignedCertificates](#cycloneddsdomainsslselfsignedcertificates)
      - [//CycloneDDS/Domain/SSL/VerifyClient](#cycloneddsdomainsslverifyclient)
    - [//CycloneDDS/Domain/Security](#cycloneddsdomainsecurity)
      - [//CycloneDDS/Domain/Security/AccessControl](#cycloneddsdomainsecurityaccesscontrol)
        - [//CycloneDDS/Domain/Security/AccessControl/Governance](#cycloneddsdomainsecurityaccesscontrolgovernance)
        - [//CycloneDDS/Domain/Security/AccessControl/Library](#cycloneddsdomainsecurityaccesscontrollibrary)
        - [//CycloneDDS/Domain/Security/AccessControl/Library\[@finalizeFunction\]](#cycloneddsdomainsecurityaccesscontrollibraryfinalizefunction)
        - [//CycloneDDS/Domain/Security/AccessControl/Library\[@initFunction\]](#cycloneddsdomainsecurityaccesscontrollibraryinitfunction)
        - [//CycloneDDS/Domain/Security/AccessControl/Library\[@path\]](#cycloneddsdomainsecurityaccesscontrollibrarypath)
        - [//CycloneDDS/Domain/Security/AccessControl/Permissions](#cycloneddsdomainsecurityaccesscontrolpermissions)
        - [//CycloneDDS/Domain/Security/AccessControl/PermissionsCA](#cycloneddsdomainsecurityaccesscontrolpermissionsca)
      - [//CycloneDDS/Domain/Security/Authentication](#cycloneddsdomainsecurityauthentication)
        - [//CycloneDDS/Domain/Security/Authentication/CRL](#cycloneddsdomainsecurityauthenticationcrl)
        - [//CycloneDDS/Domain/Security/Authentication/IdentityCA](#cycloneddsdomainsecurityauthenticationidentityca)
        - [//CycloneDDS/Domain/Security/Authentication/IdentityCertificate](#cycloneddsdomainsecurityauthenticationidentitycertificate)
        - [//CycloneDDS/Domain/Security/Authentication/IncludeOptionalFields](#cycloneddsdomainsecurityauthenticationincludeoptionalfields)
        - [//CycloneDDS/Domain/Security/Authentication/Library](#cycloneddsdomainsecurityauthenticationlibrary)
        - [//CycloneDDS/Domain/Security/Authentication/Library\[@finalizeFunction\]](#cycloneddsdomainsecurityauthenticationlibraryfinalizefunction)
        - [//CycloneDDS/Domain/Security/Authentication/Library\[@initFunction\]](#cycloneddsdomainsecurityauthenticationlibraryinitfunction)
        - [//CycloneDDS/Domain/Security/Authentication/Library\[@path\]](#cycloneddsdomainsecurityauthenticationlibrarypath)
        - [//CycloneDDS/Domain/Security/Authentication/Password](#cycloneddsdomainsecurityauthenticationpassword)
        - [//CycloneDDS/Domain/Security/Authentication/PrivateKey](#cycloneddsdomainsecurityauthenticationprivatekey)
        - [//CycloneDDS/Domain/Security/Authentication/TrustedCADirectory](#cycloneddsdomainsecurityauthenticationtrustedcadirectory)
      - [//CycloneDDS/Domain/Security/Cryptographic](#cycloneddsdomainsecuritycryptographic)
        - [//CycloneDDS/Domain/Security/Cryptographic/Library](#cycloneddsdomainsecuritycryptographiclibrary)
        - [//CycloneDDS/Domain/Security/Cryptographic/Library\[@finalizeFunction\]](#cycloneddsdomainsecuritycryptographiclibraryfinalizefunction)
        - [//CycloneDDS/Domain/Security/Cryptographic/Library\[@initFunction\]](#cycloneddsdomainsecuritycryptographiclibraryinitfunction)
        - [//CycloneDDS/Domain/Security/Cryptographic/Library\[@path\]](#cycloneddsdomainsecuritycryptographiclibrarypath)
    - [//CycloneDDS/Domain/SharedMemory](#cycloneddsdomainsharedmemory)
      - [//CycloneDDS/Domain/SharedMemory/Enable](#cycloneddsdomainsharedmemoryenable)
      - [//CycloneDDS/Domain/SharedMemory/Locator](#cycloneddsdomainsharedmemorylocator)
      - [//CycloneDDS/Domain/SharedMemory/LogLevel](#cycloneddsdomainsharedmemoryloglevel)
      - [//CycloneDDS/Domain/SharedMemory/Prefix](#cycloneddsdomainsharedmemoryprefix)
    - [//CycloneDDS/Domain/Sizing](#cycloneddsdomainsizing)
      - [//CycloneDDS/Domain/Sizing/ReceiveBufferChunkSize](#cycloneddsdomainsizingreceivebufferchunksize)
      - [//CycloneDDS/Domain/Sizing/ReceiveBufferSize](#cycloneddsdomainsizingreceivebuffersize)
    - [//CycloneDDS/Domain/TCP](#cycloneddsdomaintcp)
      - [//CycloneDDS/Domain/TCP/AlwaysUsePeeraddrForUnicast](#cycloneddsdomaintcpalwaysusepeeraddrforunicast)
      - [//CycloneDDS/Domain/TCP/Enable](#cycloneddsdomaintcpenable)
      - [//CycloneDDS/Domain/TCP/NoDelay](#cycloneddsdomaintcpnodelay)
      - [//CycloneDDS/Domain/TCP/Port](#cycloneddsdomaintcpport)
      - [//CycloneDDS/Domain/TCP/ReadTimeout](#cycloneddsdomaintcpreadtimeout)
      - [//CycloneDDS/Domain/TCP/WriteTimeout](#cycloneddsdomaintcpwritetimeout)
    - [//CycloneDDS/Domain/Threads](#cycloneddsdomainthreads)
      - [//CycloneDDS/Domain/Threads/Thread](#cycloneddsdomainthreadsthread)
      - [//CycloneDDS/Domain/Threads/Thread\[@Name\]](#cycloneddsdomainthreadsthreadname)
        - [//CycloneDDS/Domain/Threads/Thread/Scheduling](#cycloneddsdomainthreadsthreadscheduling)
          - [//CycloneDDS/Domain/Threads/Thread/Scheduling/Class](#cycloneddsdomainthreadsthreadschedulingclass)
          - [//CycloneDDS/Domain/Threads/Thread/Scheduling/Priority](#cycloneddsdomainthreadsthreadschedulingpriority)
        - [//CycloneDDS/Domain/Threads/Thread/StackSize](#cycloneddsdomainthreadsthreadstacksize)
    - [//CycloneDDS/Domain/Tracing](#cycloneddsdomaintracing)
      - [//CycloneDDS/Domain/Tracing/AppendToFile](#cycloneddsdomaintracingappendtofile)
      - [//CycloneDDS/Domain/Tracing/Category](#cycloneddsdomaintracingcategory)
      - [//CycloneDDS/Domain/Tracing/OutputFile](#cycloneddsdomaintracingoutputfile)
      - [//CycloneDDS/Domain/Tracing/PacketCaptureFile](#cycloneddsdomaintracingpacketcapturefile)
      - [//CycloneDDS/Domain/Tracing/Verbosity](#cycloneddsdomaintracingverbosity)

# //CycloneDDS

Children: [//CycloneDDS/Domain](#cycloneddsdomain)

CycloneDDS configuration

## //CycloneDDS/Domain

Attributes: [Id](<[//CycloneDDS/Domain[@Id]](#cycloneddsdomainid)>) Children: [//CycloneDDS/Domain/Compatibility](#cycloneddsdomaincompatibility), [//CycloneDDS/Domain/Discovery](#cycloneddsdomaindiscovery), [//CycloneDDS/Domain/General](#cycloneddsdomaingeneral), [//CycloneDDS/Domain/Internal](#cycloneddsdomaininternal), [//CycloneDDS/Domain/Partitioning](#cycloneddsdomainpartitioning), [//CycloneDDS/Domain/SSL](#cycloneddsdomainssl), [//CycloneDDS/Domain/Security](#cycloneddsdomainsecurity), [//CycloneDDS/Domain/SharedMemory](#cycloneddsdomainsharedmemory), [//CycloneDDS/Domain/Sizing](#cycloneddsdomainsizing), [//CycloneDDS/Domain/TCP](#cycloneddsdomaintcp), [//CycloneDDS/Domain/Threads](#cycloneddsdomainthreads), [//CycloneDDS/Domain/Tracing](#cycloneddsdomaintracing)

The General element specifying Domain related settings.

> 指定域相关设置的一般元素。

## //CycloneDDS/Domain[@Id]

[Text]: Domain id this configuration applies to, or "any" if it applies to all domain ids.

> 域 ID 此配置适用于或“任何”，如果它适用于所有域 ID。

The default value is: `any`

### //CycloneDDS/Domain/Compatibility

Children: [//CycloneDDS/Domain/Compatibility/AssumeRtiHasPmdEndpoints](#cycloneddsdomaincompatibilityassumertihaspmdendpoints), [//CycloneDDS/Domain/Compatibility/ExplicitlyPublishQosSetToDefault](#cycloneddsdomaincompatibilityexplicitlypublishqossettodefault), [//CycloneDDS/Domain/Compatibility/ManySocketsMode](#cycloneddsdomaincompatibilitymanysocketsmode), [//CycloneDDS/Domain/Compatibility/StandardsConformance](#cycloneddsdomaincompatibilitystandardsconformance)

The Compatibility element allows you to specify various settings related to compatibility with standards and with other DDSI implementations.

> 兼容性元素允许您指定与标准和其他 DDSI 实现相关的各种设置。

#### //CycloneDDS/Domain/Compatibility/AssumeRtiHasPmdEndpoints

[Boolean]: This option assumes ParticipantMessageData endpoints required by the liveliness protocol are present in RTI participants even when not properly advertised by the participant discovery protocol.

> 此选项假设即使参与者(participant)发现协议不正确地宣传，“liveliness 性协议”所需的参与者(participant)终点也存在于 RTI 参与者(participant)中。

The default value is: `false`

#### //CycloneDDS/Domain/Compatibility/ExplicitlyPublishQosSetToDefault

[Boolean]: This element specifies whether QoS settings set to default values are explicitly published in the discovery protocol. Implementations are to use the default value for QoS settings not published, which allows a significant reduction of the amount of data that needs to be exchanged for the discovery protocol, but this requires all implementations to adhere to the default values specified by the specifications.

> 此元素指定设置为默认值的 QoS 设置是否在 Discovery 协议中明确发布。实现是为未发布的 QoS 设置使用默认值，这允许大幅减少需要将需要交换为发现协议的数据量，但这需要所有实现来遵守规范指定的默认值。

When interoperability is required with an implementation that does not follow the specifications in this regard, setting this option to true will help.

> 如果需要在不遵循此方面的规格的实现中进行互操作性时，将此选项设置为 TRUE 将有所帮助。

The default value is: `false`

#### //CycloneDDS/Domain/Compatibility/ManySocketsMode

One of: false, true, single, none, many

This option specifies whether a network socket will be created for each domain participant on a host. The specification seems to assume that each participant has a unique address, and setting this option will ensure this to be the case. This is not the default.

> 此选项指定是否将为主机上的每个域参与者(participant)创建网络插座。该规范似乎假设每个参与者(participant)都有一个独特的地址，并且设置此选项将确保情况如此。这不是默认值。

Disabling it slightly improves performance and reduces network traffic somewhat. It also causes the set of port numbers needed by Cyclone DDS to become predictable, which may be useful for firewall and NAT configuration.

> 禁用它会稍微改善性能并在某种程度上减少网络流量。它还导致 Cyclone DDS 所需的端口号集成为可预测的，这可能对防火墙和 NAT 配置有用。

The default value is: `single`

#### //CycloneDDS/Domain/Compatibility/StandardsConformance

One of: lax, strict, pedantic

This element sets the level of standards conformance of this instance of the Cyclone DDS Service. Stricter conformance typically means less interoperability with other implementations. Currently, three modes are defined:

> 该元素设置了 Cyclone DDS 服务的这一实例的标准级别。更严格的一致性通常意味着与其他实现的互操作性更少。目前，定义了三种模式：

- pedantic: very strictly conform to the specification, ultimately for compliance testing, but currently of little value because it adheres even to what will most likely turn out to be editing errors in the DDSI standard. Arguably, as long as no errata have been published, the current text is in effect, and that is what pedantic currently does.

> - pedantic：非常严格符合规范，最终用于合规性测试，但目前的价值很小，因为它甚至遵守最有可能在 DDSI 标准中编辑错误的内容。可以说，只要没有发表勘误，当前文本就有效，这就是 Pedantic 当前的作用。

- strict: a relatively less strict view of the standard than does pedantic: it follows the established behaviour where the standard is obviously in error.

> - 严格：对标准的严格视图比 pedantic 较少：它遵循标准显然是错误的既定行为。

- lax: attempt to provide the smoothest possible interoperability, anticipating future revisions of elements in the standard in areas that other implementations do not adhere to, even though there is no good reason not to.

> -LAX：尝试提供最平稳的互操作性，预测标准中其他实施不遵守的区域中要素的未来修订，即使没有充分的理由不这样做。

The default value is: `lax`

### //CycloneDDS/Domain/Discovery

Children: [//CycloneDDS/Domain/Discovery/DSGracePeriod](#cycloneddsdomaindiscoverydsgraceperiod), [//CycloneDDS/Domain/Discovery/DefaultMulticastAddress](#cycloneddsdomaindiscoverydefaultmulticastaddress), [//CycloneDDS/Domain/Discovery/EnableTopicDiscoveryEndpoints](#cycloneddsdomaindiscoveryenabletopicdiscoveryendpoints), [//CycloneDDS/Domain/Discovery/ExternalDomainId](#cycloneddsdomaindiscoveryexternaldomainid), [//CycloneDDS/Domain/Discovery/LeaseDuration](#cycloneddsdomaindiscoveryleaseduration), [//CycloneDDS/Domain/Discovery/MaxAutoParticipantIndex](#cycloneddsdomaindiscoverymaxautoparticipantindex), [//CycloneDDS/Domain/Discovery/ParticipantIndex](#cycloneddsdomaindiscoveryparticipantindex), [//CycloneDDS/Domain/Discovery/Peers](#cycloneddsdomaindiscoverypeers), [//CycloneDDS/Domain/Discovery/Ports](#cycloneddsdomaindiscoveryports), [//CycloneDDS/Domain/Discovery/SPDPInterval](#cycloneddsdomaindiscoveryspdpinterval), [//CycloneDDS/Domain/Discovery/SPDPMulticastAddress](#cycloneddsdomaindiscoveryspdpmulticastaddress), [//CycloneDDS/Domain/Discovery/Tag](#cycloneddsdomaindiscoverytag)

The Discovery element allows you to specify various parameters related to the discovery of peers.

> 发现元素允许您指定与同行发现有关的各种参数。

#### //CycloneDDS/Domain/Discovery/DSGracePeriod

[Number-with-unit]: This setting controls for how long endpoints discovered via a Cloud discovery service will survive after the discovery service disappears, allowing reconnection without loss of data when the discovery service restarts (or another instance takes over).

> 此设置控制通过云发现服务发现的端点多长时间在发现服务消失后可以生存，从而允许重新连接在发现服务重新启动时(或另一个实例接管)而不会丢失数据。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.
The default value is: `30 s`

#### //CycloneDDS/Domain/Discovery/DefaultMulticastAddress

[Text]: This element specifies the default multicast address for all traffic other than participant discovery packets. It defaults to Discovery/SPDPMulticastAddress.

> 此元素为参与者(participant)发现数据包以外的所有流量指定了默认的多播地址。它默认为发现/spdpmulticastaddress。

The default value is: `auto`

#### //CycloneDDS/Domain/Discovery/EnableTopicDiscoveryEndpoints

[Boolean]: This element controls whether the built-in endpoints for topic discovery are created and used to exchange topic discovery information.

> 该元素控制是否创建了主题发现的内置端点，并用于交换主题发现信息。

The default value is: `false`

#### //CycloneDDS/Domain/Discovery/ExternalDomainId

[Text]: An override for the domain id is used to discovery and determine the port number mapping. This allows the creating of multiple domains in a single process while making them appear as a single domain on the network. The value "default" disables the override.

> 域 ID 的替代用于发现和确定端口号映射。这允许在单个过程中创建多个域，同时使它们在网络上显示为单个域。值“默认”禁用覆盖。

The default value is: `default`

#### //CycloneDDS/Domain/Discovery/LeaseDuration

[Number-with-unit]: This setting controls the default participant lease duration. The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

> 此设置控制默认参与者(participant)租赁持续时间。必须明确指定该单元。公认的单位：NS，美国，MS，S，Min，HR，Day。

The default value is: `10 s`

#### //CycloneDDS/Domain/Discovery/MaxAutoParticipantIndex

[Integer]: This element specifies the maximum DDSI participant index selected by this instance of the Cyclone DDS service if the Discovery/ParticipantIndex is "auto".

> 此元素指定了 DDSI DDS 服务的最大 DDSI 参与者(participant)索引，如果发现/参与者(participant)为“自动”。

The default value is: `9`

#### //CycloneDDS/Domain/Discovery/ParticipantIndex

[Text]: This element specifies the DDSI participant index used by this instance of the Cyclone DDS service for discovery purposes. Only one such participant id is used, independent of the number of actual DomainParticipants on the node. It is either:

> 此元素指定了此实例的 DDSI 参与者(participant)索引用于发现目的的 Cyclone DDS 服务。仅使用一个这样的参与者(participant) ID，与节点上的实际域参与者(participant)的数量无关。要么是：

- auto: which will attempt to automatically determine an available participant index (see also Discovery/MaxAutoParticipantIndex), or

> - 自动：它将尝试自动确定可用参与者(participant)索引(另请参见 Discovery/Maxautoparticipantexex)或

- a non-negative integer less than 120, or

> - 一个非负整数小于 120，或

- none:, which causes it to use arbitrary port numbers for unicast sockets which entirely removes the constraints on the participant index but makes unicast discovery impossible.

> - 无：它使其使用任意端口号作为单播插座的任意端口号，这完全消除了参与者(participant)索引的约束，但使 Unicast Discovery 变得不可能。

The default value is: `none`

#### //CycloneDDS/Domain/Discovery/Peers

Children: [//CycloneDDS/Domain/Discovery/Peers/Peer](#cycloneddsdomaindiscoverypeerspeer)

This element statically configures addresses for discovery.

> 此元素从静态配置发现地址。

##### //CycloneDDS/Domain/Discovery/Peers/Peer

Attributes: [Address](<[//CycloneDDS/Domain/Discovery/Peers/Peer[@Address]](#cycloneddsdomaindiscoverypeerspeeraddress)>)

This element statically configures addresses for discovery.

> 此元素从静态配置发现地址。

##### //CycloneDDS/Domain/Discovery/Peers/Peer[@Address]

[Text]: This element specifies an IP address to which discovery packets must be sent, in addition to the default multicast address (see also General/AllowMulticast). Both hostnames and a numerical IP address are accepted; the hostname or IP address may be suffixed with :PORT to explicitly set the port to which it must be sent. Multiple Peers may be specified.

> 此元素还指定了一个 IP 地址，除了默认的多播地址外，还必须发送发现数据包的 IP 地址(另请参见 General/lastermulticast)。主机名和数值 IP 地址均被接受；主机名或 IP 地址可以带有以下后缀：端口以显式设置必须发送的端口。可以指定多个同伴。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Discovery/Ports

Children: [//CycloneDDS/Domain/Discovery/Ports/Base](#cycloneddsdomaindiscoveryportsbase), [//CycloneDDS/Domain/Discovery/Ports/DomainGain](#cycloneddsdomaindiscoveryportsdomaingain), [//CycloneDDS/Domain/Discovery/Ports/MulticastDataOffset](#cycloneddsdomaindiscoveryportsmulticastdataoffset), [//CycloneDDS/Domain/Discovery/Ports/MulticastMetaOffset](#cycloneddsdomaindiscoveryportsmulticastmetaoffset), [//CycloneDDS/Domain/Discovery/Ports/ParticipantGain](#cycloneddsdomaindiscoveryportsparticipantgain), [//CycloneDDS/Domain/Discovery/Ports/UnicastDataOffset](#cycloneddsdomaindiscoveryportsunicastdataoffset), [//CycloneDDS/Domain/Discovery/Ports/UnicastMetaOffset](#cycloneddsdomaindiscoveryportsunicastmetaoffset)

The Ports element specifies various parameters related to the port numbers used for discovery. These all have default values specified by the DDSI 2.1 specification and rarely need to be changed.

> 端口元素指定与用于发现的端口号有关的各种参数。这些都具有 DDSI 2.1 规范指定的默认值，很少需要更改。

##### //CycloneDDS/Domain/Discovery/Ports/Base

[Integer]: This element specifies the base port number (refer to the DDSI 2.1 specification, section 9.6.1, constant PB).

> 此元素指定基本端口号(请参阅 DDSI 2.1 规范，第 9.6.1 节，常数 PB)。

The default value is: `7400`

##### //CycloneDDS/Domain/Discovery/Ports/DomainGain

[Integer]: This element specifies the domain gain, relating domain ids to sets of port numbers (refer to the DDSI 2.1 specification, section 9.6.1, constant DG).

> 该元素指定域增益，将域 ID 与端口号的集合有关(请参阅 DDSI 2.1 规范，第 9.6.1 节，常数 DG)。

The default value is: `250`

##### //CycloneDDS/Domain/Discovery/Ports/MulticastDataOffset

[Integer]: This element specifies the port number for multicast data traffic (refer to the DDSI 2.1 specification, section 9.6.1, constant d2).

> 此元素指定了多播数据流量的端口号(请参阅 DDSI 2.1 规范，第 9.6.1 节，常数 D2)。

The default value is: `1`

##### //CycloneDDS/Domain/Discovery/Ports/MulticastMetaOffset

[Integer]: This element specifies the port number for multicast meta traffic (refer to the DDSI 2.1 specification, section 9.6.1, constant d0).

> 此元素指定多播 Meta 流量的端口号(请参阅 DDSI 2.1 规范第 9.6.1 节，常数 D0)。

The default value is: `0`

##### //CycloneDDS/Domain/Discovery/Ports/ParticipantGain

[Integer]: This element specifies the participant gain, relating p0, participant index to sets of port numbers (refer to the DDSI 2.1 specification, section 9.6.1, constant PG).

> 该元素指定参与者(participant)的增益，即 P0，参与者(participant)索引与端口号的集合(请参阅 DDSI 2.1 规范，第 9.6.1 节，常数 PG)。

The default value is: `2`

##### //CycloneDDS/Domain/Discovery/Ports/UnicastDataOffset

[Integer]: This element specifies the port number for unicast data traffic (refer to the DDSI 2.1 specification, section 9.6.1, constant d3).

> 该元素指定单播数据流量的端口号(请参阅 DDSI 2.1 规范，第 9.6.1 节，常数 D3)。

The default value is: `11`

##### //CycloneDDS/Domain/Discovery/Ports/UnicastMetaOffset

[Integer]: This element specifies the port number for unicast meta traffic (refer to the DDSI 2.1 specification, section 9.6.1, constant d1).

> 此元素指定单播元流量的端口号(请参阅 DDSI 2.1 规范，第 9.6.1 节，常数 D1)。

The default value is: `10`

#### //CycloneDDS/Domain/Discovery/SPDPInterval

[Number-with-unit]: This element specifies the interval between spontaneous transmissions of participant discovery packets.

> 该元素指定参与者(participant)发现数据包的自发传输之间的间隔。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

> 必须明确指定该单元。公认的单位：NS，美国，MS，S，Min，HR，Day。

The default value is: `30 s`

#### //CycloneDDS/Domain/Discovery/SPDPMulticastAddress

[Text]: This element specifies the multicast address used as the destination for the participant discovery packets. In IPv4 mode the default is the (standardised) 239.255.0.1, in IPv6 mode it becomes ff02::ffff:239.255.0.1, which is a non-standardised link-local multicast address.

> 该元素指定用作参与者(participant)发现数据包目的地的多播地址。在 IPv4 模式下，默认值为(标准化的)239.255.0.1，在 IPv6 模式下，它变为 FF02 :: FFFF：239.255.0.1，它是一个非标准的链接链接多播地址。

The default value is: `239.255.0.1`

#### //CycloneDDS/Domain/Discovery/Tag

[Text]: String extension for domain id that remote participants must match to be discovered.

> 远程参与者(participant)必须匹配的域 ID 的字符串扩展名。

The default value is: `<empty>`

### //CycloneDDS/Domain/General

Children: [//CycloneDDS/Domain/General/AllowMulticast](#cycloneddsdomaingeneralallowmulticast), [//CycloneDDS/Domain/General/DontRoute](#cycloneddsdomaingeneraldontroute), [//CycloneDDS/Domain/General/EnableMulticastLoopback](#cycloneddsdomaingeneralenablemulticastloopback), [//CycloneDDS/Domain/General/EntityAutoNaming](#cycloneddsdomaingeneralentityautonaming), [//CycloneDDS/Domain/General/ExternalNetworkAddress](#cycloneddsdomaingeneralexternalnetworkaddress), [//CycloneDDS/Domain/General/ExternalNetworkMask](#cycloneddsdomaingeneralexternalnetworkmask), [//CycloneDDS/Domain/General/FragmentSize](#cycloneddsdomaingeneralfragmentsize), [//CycloneDDS/Domain/General/Interfaces](#cycloneddsdomaingeneralinterfaces), [//CycloneDDS/Domain/General/MaxMessageSize](#cycloneddsdomaingeneralmaxmessagesize), [//CycloneDDS/Domain/General/MaxRexmitMessageSize](#cycloneddsdomaingeneralmaxrexmitmessagesize), [//CycloneDDS/Domain/General/MulticastRecvNetworkInterfaceAddresses](#cycloneddsdomaingeneralmulticastrecvnetworkinterfaceaddresses), [//CycloneDDS/Domain/General/MulticastTimeToLive](#cycloneddsdomaingeneralmulticasttimetolive), [//CycloneDDS/Domain/General/RedundantNetworking](#cycloneddsdomaingeneralredundantnetworking), [//CycloneDDS/Domain/General/Transport](#cycloneddsdomaingeneraltransport), [//CycloneDDS/Domain/General/UseIPv6](#cycloneddsdomaingeneraluseipv6)

The General element specifies overall Cyclone DDS service settings.

> 一般元素指定整体 Cyclone DDS 服务设置。

#### //CycloneDDS/Domain/General/AllowMulticast

One of: \* Keyword: default \* Comma-separated list of: false, spdp, asm, ssm, true

This element controls whether Cyclone DDS uses multicasts for data traffic.

> 该元素控制 Cyclone DDS 是否使用多播进行数据流量。

It is a comma-separated list of some of the following keywords: "spdp", "asm", "ssm", or either of "false" or "true", or "default".

> 这是以下一些关键字的逗号分隔列表：“ spdp”，“ asm”，“ ssm”或“ false”或“ true”或“ true”或“ default”。

- spdp: enables the use of ASM (any-source multicast) for participant discovery, joining the multicast group on the discovery socket, transmitting SPDP messages to this group, but never advertising nor using any multicast address in any discovery message, thus forcing unicast communications for all endpoint discovery and user data.

> - SPDP：启用参与者(participant)发现的 ASM(任何源元播)，加入 Discovery 套接字上的多播组，将 SPDP 消息传输到该组，但切勿在任何发现消息中进行广告或使用任何多播地址，从而强迫所有端点发现和用户数据的单播通信。

- asm: enables the use of ASM for all traffic, including receiving SPDP but not transmitting SPDP messages via multicast

> - ASM：启用 ASM 用于所有流量，包括接收 SPDP，但不通过 Multicast 传输 SPDP 消息

- ssm: enables the use of SSM (source-specific multicast) for all non-SPDP traffic (if supported)

> - SSM：启用所有非 SPDP 流量(如果支持)的 SSM(特定于源的多播)

When set to "false" all multicasting is disabled. The default, "true" enables the full use of multicasts. Listening for multicasts can be controlled by General/MulticastRecvNetworkInterfaceAddresses.

> 设置为“ false”时，所有多播都将禁用。默认值“ true”可以充分使用多播。聆听多播可以由常规/多曲线处理 interfaceaddresses 控制。

"default" maps on spdp if the network is a WiFi network, on true if it is a wired network

> 如果网络是 WiFi 网络，则在 SPDP 上的“默认”地图，如果它是有线网络

The default value is: `default`

#### //CycloneDDS/Domain/General/DontRoute

[Boolean]: This element allows setting the SO_DONTROUTE option for outgoing packets to bypass the local routing tables. This is generally useful only when the routing tables cannot be trusted, which is highly unusual.

> 此元素允许设置 So_dontRoute 选项，以绕过绕过本地路由表。通常，只有当无法信任路由表(这是非常不寻常的)时，这通常才有用。

The default value is: `false`

#### //CycloneDDS/Domain/General/EnableMulticastLoopback

> [!NOTE]: 在网络协议相关的领域中，存在 “回环(loopback)” 的概念，可以对其进一步的解释吗
> 在网络协议的领域中，回环(loopback)指的是一个网络接口，它可以将发往该接口的数据包直接回送给发送方而不离开主机。这种操作通常被用于本地测试和诊断，它可以模拟一个完整的网络环境而不需要实际的网络连接，也可以用来检查网络接口是否工作正常。
> 在计算机操作系统中，回环(loopback)接口是一个虚拟的网络接口，通常具有 IP 地址 127.0.0.1，也称为本地回环地址。这个特殊的地址实
> 际上是指向本地计算机上的回环接口。当将数据包发往 127.0.0.1 时，
> 数据包会被操作系统内部的网络协议栈接收并回送给应用程序，而不会离开计算机。
> 回环接口经常被用于测试和调试应用程序，同时也可以用来实现一些网络服务的本地访问，比如 HTTP 服务、数据库等。此外，如果您的计算机没有网络连接，您仍然可以使用回环接口测试和开发网络应用程序。

[Boolean]: This element specifies whether Cyclone DDS allows IP multicast packets to be visible to all DDSI participants in the same node, including itself. It must be "true" for intra-node multicast communications. However, if a node runs only a single Cyclone DDS service and does not host any other DDSI-capable programs, it should be set to "false" for improved performance.

> 该元素指定 Cyclone DDS 是否允许在同一节点(包括自身)中所有 DDSI 参与者(participant)可见 IP 多播数据包。对于节点内部播放通信，必须是“ true”。但是，如果节点仅运行单个 Cyclone DDS 服务并且不托管任何其他具有 DDSI 功能的程序，则应将其设置为“ false”以提高性能。

The default value is: `true`

#### //CycloneDDS/Domain/General/EntityAutoNaming

Attributes: [seed](<[//CycloneDDS/Domain/General/EntityAutoNaming[@seed]](#cycloneddsdomaingeneralentityautonamingseed)>)

One of: empty, fancy

This element specifies the entity autonaming mode. By default set to 'empty' which means no name will be set (but you can still use dds_qset_entity_name). When set to 'fancy' participants, publishers, subscribers, writers, and readers will get randomly generated names. An autonamed entity will share a 3-letter prefix with their parent entity.

> 此元素指定实体自动启动模式。默认情况下，将设置为“空”，这意味着不会设置名称(但您仍然可以使用 dds_qset_entity_name)。设置为“fancy”参与者(participant)时，出版商，订阅者，writer 和 reader 将随机生成名称。一个自动化的实体将与其父实体共享 3 个字母的前缀。

The default value is: `empty`

#### //CycloneDDS/Domain/General/EntityAutoNaming[@seed]

[Text]: Provide an initial seed for the entity naming. Your string will be hashed to provide the random state. When provided, the same sequence of names is generated every run. Creating your entities in the same order will ensure they are the same between runs. If you run multiple nodes, set this via environment variable to ensure every node generates unique names. A random starting seed is chosen when left empty, (the default).

> 为实体命名提供初始种子。您的字符串将被哈希提供随机状态。提供时，每次运行都会生成相同的名称顺序。以相同的顺序创建您的实体将确保它们之间的运行相同。如果**运行多个节点，请通过环境变量设置此节点，以确保每个节点都会生成唯一的名称**。当空白时选择一个随机的**启动种子**(默认)。

The default value is: `<empty>`

#### //CycloneDDS/Domain/General/ExternalNetworkAddress

[Text]: This element allows explicitly overruling the network address Cyclone DDS advertises in the discovery protocol, which by default is the address of the preferred network interface (General/NetworkInterfaceAddress), to allow Cyclone DDS to communicate across a Network Address Translation (NAT) device.

> 此元素允许明确覆盖网络地址 Cyclone DDS 在 Discovery 协议中宣传的网络地址，默认情况下是首选网络接口(General/NetworkInterfaceAddress)的地址，以允许 Cyclone DDS 通过网络地址转换(NAT)设备进行通信。

The default value is: `auto`

#### //CycloneDDS/Domain/General/ExternalNetworkMask

[Text]: This element specifies the network mask of the external network address. This element is relevant only when an external network address (General/ExternalNetworkAddress) is explicitly configured. In this case locators received via the discovery protocol that are within the same external subnet (as defined by this mask) will be translated to an internal address by replacing the network portion of the external address with the corresponding portion of the preferred network interface address. This option is IPv4-only.

> 这个元素指定了外部网 络地址的网络掩码。该元素仅在显式配置外部网络地址(General/ExternalNetworkAddress )时才有用。在这种情况下，通过发现协议接收到的定位器如果与此掩码定义的相同外部子 网中，则会通过将外部地址的网络部分替换为优选网络接口地址的相应部分来转换为内部地 址。这个选项仅适用于 IPv4。

> 简而言之，这个元素是用于指定外部网络地址的网络掩码，在配置了外部网络地址时才会有 用。通过该掩码，可以将在同一外部子网中接收到的定位器转换为内部地址，从而实现内部 地址和外部地址之间的互通。同时，需要注意的是，该选项仅适用于 IPv4 网络协议。

The default value is: `0.0.0.0`

#### //CycloneDDS/Domain/General/FragmentSize

[Number-with-unit]: This element specifies the size of DDSI sample fragments generated by Cyclone DDS. Samples larger than FragmentSize are fragmented into fragments of FragmentSize bytes each, except the last one, which may be smaller. The DDSI spec mandates a minimum fragment size of 1025 bytes, but Cyclone DDS will do whatever size is requested, accepting fragments of which the size is at least the minimum of 1025 and FragmentSize.

> 该元素指定了 Cyclone DDS 生成的 DDSI 样品片段的大小。大于碎片的样品被碎片分成片段的碎片，除最后一个外，但可能更小。DDSI 规格的最小片段大小为 1025 个字节，但是 Cyclone DDS 将执行任何要求的尺寸，接受该尺寸至少为 1025 的片段，并且片段化。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `1344 B`

#### //CycloneDDS/Domain/General/Interfaces

Children: [//CycloneDDS/Domain/General/Interfaces/NetworkInterface](#cycloneddsdomaingeneralinterfacesnetworkinterface)

This element specifies the network interfaces for use by Cyclone DDS. Multiple interfaces can be specified with an assigned priority. The list in use will be sorted by priority. If interfaces have an equal priority, the specification order will be preserved.

> 该元素指定了 Cyclone DDS 使用的**网络接口。可以使用分配的优先级指定多个接口**。使用中的列表将按优先级进行排序。如果接口具有相等的优先级，则将保留规范订单。

> [!note]: 可以指定多个接口，并分配优先级

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface

Attributes: [address](<[//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@address]](#cycloneddsdomaingeneralinterfacesnetworkinterfaceaddress)>), [autodetermine](<[//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@autodetermine]](#cycloneddsdomaingeneralinterfacesnetworkinterfaceautodetermine)>), [multicast](<[//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@multicast]](#cycloneddsdomaingeneralinterfacesnetworkinterfacemulticast)>), [name](<[//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@name]](#cycloneddsdomaingeneralinterfacesnetworkinterfacename)>), [prefer_multicast](<[//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@prefer_multicast]](#cycloneddsdomaingeneralinterfacesnetworkinterfaceprefer_multicast)>), [presence_required](<[//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@presence_required]](#cycloneddsdomaingeneralinterfacesnetworkinterfacepresence_required)>), [priority](<[//CycloneDDS/Domain/General/Interfaces/NetworkInterface[@priority]](#cycloneddsdomaingeneralinterfacesnetworkinterfacepriority)>)

This element defines a network interface. You can set autodetermine="true" to autoselect the interface CycloneDDS considers the highest quality. If autodetermine="false" (the default), you must specify the name and/or address attribute. If you specify both, they must match the same interface.

> 该元素定义了网络接口。您可以设置自动定性=“ true”，以**自动选择接口 Cyclonedds 认为最高质量**。如果自动定性=“ false”(默认)，则必须指定名称和/或地址属性。如果两者都指定，则必须匹配相同的接口。

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@address]

[Text]: This attribute specifies the address of the interface. With ipv4 allows matching on the network part if the host part is set to zero.

> 此属性指定接口的地址。使用 IPv4，如果主机零件设置为零，则允许在网络部件上匹配。

The default value is: `<empty>`

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@autodetermine]

[Text]: If set to "true" an interface is automatically selected. Specifying a name or an address when automatic is set is considered an error.

> 如果设置为“ true”接口，则将自动选择。设置自动时指定名称或地址是错误。

The default value is: `false`

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@multicast]

[Text]: This attribute specifies whether the interface should use multicast. On its default setting, 'default', it will use the value as return by the operating system. If set to 'true', the interface will be assumed to be multicast capable even when the interface flags returned by the operating system state it is not (this provides a workaround for some platforms). If set to 'false', the interface will never be used for multicast. The default value is: `default`

> 此属性指定接口**是否应使用多播**。在其默认设置“默认设置”上，它将使用该值作为操作系统的返回。如果设置为“ true”，即使在操作系统状态返回的接口标志时，该接口也将是多播的(这为某些平台提供了解决方法)。**如果设置为“false”，则该接口将永远不会用于多播**。默认值是：`默认值

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@name]

[Text]: This attribute specifies the name of the interface.

> 此属性指定接口的名称。

The default value is: `<empty>`

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@prefer_multicast]

[Boolean]: When false (default), Cyclone DDS uses unicast for data whenever a single unicast suffices. Setting this to true makes it prefer multicasting data, falling back to unicast only when no multicast is available.

> 当 false(默认)时，只要单个单播就足够，cyclone dds 就会使用单播进行数据。**将其设置为 true，因此更喜欢多播数据**，只有在没有多播可用的情况下才能回到 unicast。

The default value is: `false`

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@presence_required]

[Boolean]: By default, all specified network interfaces must be present; if they are missing Cyclone will not start. By explicitly setting this setting for an interface, you can instruct Cyclone to ignore that interface if it is not present.

> 默认情况下，必须存在所有指定的网络接口；如果他们缺少 Cyclone ，则不会启动。通过明确为接口设置此设置，您可以指示 Cyclone 如果不存在该接口。

The default value is: `true`

##### //CycloneDDS/Domain/General/Interfaces/NetworkInterface[@priority]

[Text]: This attribute specifies the interface priority (decimal integer or default). The default value for loopback interfaces is 2, for all other interfaces it is 0.

> 此属性指定接口优先级(十进制整数或默认值)。环回接口的默认值为 2，对于所有其他接口为 0。

The default value is: `default`

#### //CycloneDDS/Domain/General/MaxMessageSize

[Number-with-unit]: This element specifies the maximum size of the UDP payload that Cyclone DDS will generate. Cyclone DDS will try to maintain this limit within the bounds of the DDSI specification, which means that in some cases (especially for very low values of MaxMessageSize) larger payloads may sporadically be observed (currently up to 1192 B).

> 此元素指定了 Cyclone DDS 将生成的 UDP 有效载荷的最大大小。Cyclone DDS 将尝试在 DDSI 规范的范围内维持此限制，这意味着在某些情况下(尤其是对于非常低的 MaxMessagesize 值)可能会偶尔观察到较大的有效载荷(当前最高为 1192 B)。

On some networks it may be necessary to set this item to keep the packetsize below the MTU to prevent IP fragmentation.

> 在某些网络上，可能有必要设置此项目以将小包大小保持在 MTU 下方，以防止 IP 碎片。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

> 必须明确指定该单元。公认单位：B(字节)，KB 和 KIB(2^10 个字节)，MB＆MIB(2^20 个字节)，GB＆GIB(2^30 字节)。

The default value is: `14720 B`

#### //CycloneDDS/Domain/General/MaxRexmitMessageSize

[Number-with-unit]: This element specifies the maximum size of the UDP payload that Cyclone DDS will generate for a retransmit. Cyclone DDS will try to maintain this limit within the bounds of the DDSI specification, which means that in some cases (especially for very low values) larger payloads may sporadically be observed (currently up to 1192 B).

> 该元素指定了 Cyclone DDS 将生成重传的 UDP 有效载荷的最大大小。Cyclone DDS 将尝试在 DDSI 规范的范围内维持此限制，这意味着在某些情况下(尤其是对于非常低的值)可能会偶尔观察到较大的有效载荷(目前最高为 1192 B)。

On some networks it may be necessary to set this item to keep the packetsize below the MTU to prevent IP fragmentation.

> 在某些网络上，可能有必要设置此项目以将小包大小保持在 MTU 下方，以防止 IP 碎片。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `1456 B`

#### //CycloneDDS/Domain/General/MulticastRecvNetworkInterfaceAddresses

[Text]: This element specifies which network interfaces Cyclone DDS listens to multicasts. The following options are available:

> 该元素指定哪些网络将 Cyclone DDS 倾听到多播。可用以下选项：

> - all: listen for multicasts on all multicast-capable interfaces; or
> - any: listen for multicasts on the operating system default interface; or
> - preferred: listen for multicasts on the preferred interface (General/Interface/NetworkInterface with the highest priority); or
> - none: does not listen for multicasts on any interface; or
> - a comma-separated list of network addresses: configures Cyclone DDS to listen for multicasts on all listed addresses.

> - 全部：在所有具有多播功能的界面上聆听多播；或者
> - 任何：在操作系统默认接口上聆听多播；或者
> - 首选：在首选接口(具有最高优先级的常规/接口/NetworkInterface)上收听多播；或者
> - 无：不听任何接口上的多播；或者
> - 网络地址的逗号分隔列表：配置 Cyclone DDS 以在所有列出的地址上收听多播。

If Cyclone DDS is in IPv6 mode and the address of the preferred network interface is a link-local address, "all" is treated as a synonym for "preferred" and a comma-separated list is treated as "preferred" if it contains the preferred interface and as "none" if not.

> 如果 Cyclone DDS 处于 IPv6 模式，并且首选网络接口的地址是链接 - 位置地址，则“ ALL”被视为“首选”的同义词，并且逗号分隔的列表被视为“首选”，如果它包含该列表，则将其视为首选界面，为“无”(如果不是)。

The default value is: `preferred`

#### //CycloneDDS/Domain/General/MulticastTimeToLive

[Integer]: This element specifies the time-to-live setting for outgoing multicast packets.

> 此元素指定了传出的多播数据包的时间设置。

The default value is: `32`

#### //CycloneDDS/Domain/General/RedundantNetworking

[Boolean]: When enabled, use selected network interfaces in parallel for redundancy.

> 启用后，**并行使用选定的网络接口进行冗余**。

The default value is: `false`

#### //CycloneDDS/Domain/General/Transport

One of: default, udp, udp6, tcp, tcp6, raweth

This element allows selecting the transport to be used (udp, udp6, tcp, tcp6, raweth)

> 该元素允许选择使用传输(udp，udp6，tcp，tcp6，raweth)

The default value is: `default`

#### //CycloneDDS/Domain/General/UseIPv6

One of: false, true, default

Deprecated (use Transport instead)

> 弃用(改用运输)

The default value is: `default`

### //CycloneDDS/Domain/Internal

Children: [//CycloneDDS/Domain/Internal/AccelerateRexmitBlockSize](#cycloneddsdomaininternalacceleraterexmitblocksize), [//CycloneDDS/Domain/Internal/AckDelay](#cycloneddsdomaininternalackdelay), [//CycloneDDS/Domain/Internal/AutoReschedNackDelay](#cycloneddsdomaininternalautoreschednackdelay), [//CycloneDDS/Domain/Internal/BuiltinEndpointSet](#cycloneddsdomaininternalbuiltinendpointset), [//CycloneDDS/Domain/Internal/BurstSize](#cycloneddsdomaininternalburstsize), [//CycloneDDS/Domain/Internal/ControlTopic](#cycloneddsdomaininternalcontroltopic), [//CycloneDDS/Domain/Internal/DefragReliableMaxSamples](#cycloneddsdomaininternaldefragreliablemaxsamples), [//CycloneDDS/Domain/Internal/DefragUnreliableMaxSamples](#cycloneddsdomaininternaldefragunreliablemaxsamples), [//CycloneDDS/Domain/Internal/DeliveryQueueMaxSamples](#cycloneddsdomaininternaldeliveryqueuemaxsamples), [//CycloneDDS/Domain/Internal/EnableExpensiveChecks](#cycloneddsdomaininternalenableexpensivechecks), [//CycloneDDS/Domain/Internal/GenerateKeyhash](#cycloneddsdomaininternalgeneratekeyhash), [//CycloneDDS/Domain/Internal/HeartbeatInterval](#cycloneddsdomaininternalheartbeatinterval), [//CycloneDDS/Domain/Internal/LateAckMode](#cycloneddsdomaininternallateackmode), [//CycloneDDS/Domain/Internal/LivelinessMonitoring](#cycloneddsdomaininternallivelinessmonitoring), [//CycloneDDS/Domain/Internal/MaxParticipants](#cycloneddsdomaininternalmaxparticipants), [//CycloneDDS/Domain/Internal/MaxQueuedRexmitBytes](#cycloneddsdomaininternalmaxqueuedrexmitbytes), [//CycloneDDS/Domain/Internal/MaxQueuedRexmitMessages](#cycloneddsdomaininternalmaxqueuedrexmitmessages), [//CycloneDDS/Domain/Internal/MaxSampleSize](#cycloneddsdomaininternalmaxsamplesize), [//CycloneDDS/Domain/Internal/MeasureHbToAckLatency](#cycloneddsdomaininternalmeasurehbtoacklatency), [//CycloneDDS/Domain/Internal/MonitorPort](#cycloneddsdomaininternalmonitorport), [//CycloneDDS/Domain/Internal/MultipleReceiveThreads](#cycloneddsdomaininternalmultiplereceivethreads), [//CycloneDDS/Domain/Internal/NackDelay](#cycloneddsdomaininternalnackdelay), [//CycloneDDS/Domain/Internal/PreEmptiveAckDelay](#cycloneddsdomaininternalpreemptiveackdelay), [//CycloneDDS/Domain/Internal/PrimaryReorderMaxSamples](#cycloneddsdomaininternalprimaryreordermaxsamples), [//CycloneDDS/Domain/Internal/PrioritizeRetransmit](#cycloneddsdomaininternalprioritizeretransmit), [//CycloneDDS/Domain/Internal/RediscoveryBlacklistDuration](#cycloneddsdomaininternalrediscoveryblacklistduration), [//CycloneDDS/Domain/Internal/RetransmitMerging](#cycloneddsdomaininternalretransmitmerging), [//CycloneDDS/Domain/Internal/RetransmitMergingPeriod](#cycloneddsdomaininternalretransmitmergingperiod), [//CycloneDDS/Domain/Internal/RetryOnRejectBestEffort](#cycloneddsdomaininternalretryonrejectbesteffort), [//CycloneDDS/Domain/Internal/SPDPResponseMaxDelay](#cycloneddsdomaininternalspdpresponsemaxdelay), [//CycloneDDS/Domain/Internal/ScheduleTimeRounding](#cycloneddsdomaininternalscheduletimerounding), [//CycloneDDS/Domain/Internal/SecondaryReorderMaxSamples](#cycloneddsdomaininternalsecondaryreordermaxsamples), [//CycloneDDS/Domain/Internal/SocketReceiveBufferSize](#cycloneddsdomaininternalsocketreceivebuffersize), [//CycloneDDS/Domain/Internal/SocketSendBufferSize](#cycloneddsdomaininternalsocketsendbuffersize), [//CycloneDDS/Domain/Internal/SquashParticipants](#cycloneddsdomaininternalsquashparticipants), [//CycloneDDS/Domain/Internal/SynchronousDeliveryLatencyBound](#cycloneddsdomaininternalsynchronousdeliverylatencybound), [//CycloneDDS/Domain/Internal/SynchronousDeliveryPriorityThreshold](#cycloneddsdomaininternalsynchronousdeliveryprioritythreshold), [//CycloneDDS/Domain/Internal/Test](#cycloneddsdomaininternaltest), [//CycloneDDS/Domain/Internal/UnicastResponseToSPDPMessages](#cycloneddsdomaininternalunicastresponsetospdpmessages), [//CycloneDDS/Domain/Internal/UseMulticastIfMreqn](#cycloneddsdomaininternalusemulticastifmreqn), [//CycloneDDS/Domain/Internal/Watermarks](#cycloneddsdomaininternalwatermarks), [//CycloneDDS/Domain/Internal/WriteBatch](#cycloneddsdomaininternalwritebatch), [//CycloneDDS/Domain/Internal/WriterLingerDuration](#cycloneddsdomaininternalwriterlingerduration)

The Internal elements deal with a variety of settings that are evolving and that are not necessarily fully supported. For the majority of the Internal settings the functionality is supported, but the right to change the way the options control the functionality is reserved. This includes renaming or moving options.

> 内部元素涉及各种不断发展且不一定得到充分支持的环境。对于大多数内部设置，支持该功能，但是更改选项控制功能的方式的权利保留。这包括重命名或移动选项。

#### //CycloneDDS/Domain/Internal/AccelerateRexmitBlockSize

[Integer]: Proxy readers that are assumed to still be retrieving historical data get this many samples retransmitted when they NACK something, even if some of these samples have sequence numbers outside the set covered by the NACK.

> 假定仍在检索历史数据的代理 reader 会在将某些样本纳入某些样本时，即使其中一些样本在 NACK 所涵盖的集合之外都具有序列编号，但它们都会重新进行了许多样本。

The default value is: `0`

#### //CycloneDDS/Domain/Internal/AckDelay

[Number-with-unit]: This setting controls the delay between sending identical acknowledgements.

> 此设置控制发送相同的确认之间的延迟。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.
The default value is: `10 ms`

#### //CycloneDDS/Domain/Internal/AutoReschedNackDelay

[Number-with-unit]: This setting controls the interval with which a reader will continue NACK'ing missing samples in the absence of a response from the writer, as a protection mechanism against writers incorrectly stopping the sending of HEARTBEAT messages.

> 此设置控制读取器在**没有写入器响应的情况下继续 NACK 丢失样本的时间间隔，作为防止写入器错误地停止发送 HEARTBEAT 消息的保护机制**。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `3 s`

#### //CycloneDDS/Domain/Internal/BuiltinEndpointSet

> [!NOTE]: 如何理解 cyclone dds 中 "Builtin Endpoint Set"
> Cyclone DDS 是一个基于 Data Distribution Service(DDS)标准的开源实现，它提供了实时、可靠、去中心化的数据通信能力。在 Cyclone DDS 中，Builtin Endpoint Set 是指一组预定义的特殊数据流(Endpoint)，用于处理 DDS 协议相关的信息交换。这些特殊的 Endpoint 一般不用于普通数据交换，而是用于 DDS 内部通信，从而实现不同 DDS 实体之间的协调和同步。
>
> Builtin Endpoint Set 由 4 个 Endpoint 组成，它们分别是：
>
> 1. PUBLICATION_BUILTIN_TOPIC：发布者内置主题，用于描述 DDS 实体提供的数据流；
> 2. SUBSCRIPTION_BUILTIN_TOPIC：订阅者内置主题，用于描述 DDS 实体需要接收的数据流；
> 3. PARTICIPANT_BUILTIN_TOPIC：参与者内置主题，用于描述 DDS 实体之间相互感知和相互协调的信息；
> 4. TOPIC_BUILTIN_TOPIC：主题内置主题，用于描述 DDS 实体上的数据流，这些数据流由发布者发布，由订阅者订阅。
>
> Builtin Endpoint Set 通常被用于实现 DDS 实体之间的自我发现、协作
> 和同步，从而确保 DDS 数据通信的稳定可靠性。通过使用 Builtin Endpoint Set，DDS 实体可以在启动时自动发现其他实体，并协调其加入 DDS
> 网络的过程，同时可以恢复丢失的数据、监控网络状态等。

One of: full, writers, minimal

This element controls which participants will have which built-in endpoints for the discovery and liveliness protocols. Valid values are:

> 该元素**控制参与者(participant)将拥有哪些内置端点**，以进行 discovery 和 liveliness 协议。有效值为：

- full: all participants have all endpoints;
- writers: all participants have the writers, but just one has the readers;
- minimal: only one participant has built-in endpoints.

> - 完整：所有参与者(participant)都有所有终点；
> - writer：所有参与者(participant)都有 writer，但只有一个 reader；
> - 最小：只有一个参与者(participant)具有内置端点。

The default is writers, as this is thought to be compliant and reasonably efficient. Minimal may or may not be compliant but is most efficient, and full is inefficient but certain to be compliant.

> 默认值是 writer，因为这被认为是合规且相当有效的。最少可能是合规，但最有效的，最有效，并且效率低下，但肯定是合规的。

The default value is: `writers`

#### //CycloneDDS/Domain/Internal/BurstSize

Children: [//CycloneDDS/Domain/Internal/BurstSize/MaxInitTransmit](#cycloneddsdomaininternalburstsizemaxinittransmit), [//CycloneDDS/Domain/Internal/BurstSize/MaxRexmit](#cycloneddsdomaininternalburstsizemaxrexmit)

> [!NOTE]: 如何理解 cyclone dds 中 "burst size"
> 在 Cyclone DDS 中，“burst size”是指一次发送数据时可发送的最大数据量。它指示了 DDS 实体在**一次发送中能够发送的最大数据量**，而**不需要进行带宽调整**。较大的 burst size 通常能够更快地传输大量数据，但**也可能会导致网络拥塞和过高的延迟，因此需要根据具体情况进行权衡和调整**。
>
> 在 Cyclone DDS 中，**默认的 burst size 设置为 4096 bytes**，这意味着 DDS 实体一次最多可以发送 4096 bytes 的数据。如果在一次发送中需要传输的数据超过 4096 bytes，则 DDS 实体将会分割这些数据，并多次地发送若干个 4096 bytes 的数据块。这种划分数据的方式能够更好地适应网络状况和实时性要求，提高了 DDS 的数据传输效率。
>
> Burst size 是 Cyclone DDS 中的一个重要参数，通常需要根据具体场景进行调整和配置。如果需要发送大量的数据，需要增加 Burst size 以提高传输效率；但如果带宽较小或网络拥塞，可能需要减小 Burst size 以避免网络问题。

Setting for controlling **the size of transmitting bursts**.

> 设置用于控制传输**突发的大小**。

> [!note]: 这个突发传输大小的设置很有意思！

##### //CycloneDDS/Domain/Internal/BurstSize/MaxInitTransmit

[Number-with-unit]: This element specifies how much more than the (presumed or discovered) receive buffer size may be sent when transmitting a sample for the first time, expressed as a percentage; the remainder will then be handled via retransmits. Usually, the receivers can keep up with the transmitter, at least on average, so generally it is better to hope for the best and recover. Besides, the retransmits will be unicast, and so any multicast advantage will be lost as well.

> 该元素指定在第一次传输样本时可以发送比(假定或发现的)**接收缓冲区大小多多少**，以百分比表示； **其余部分将通过重传处理**。通常，接收器可以跟上发射器，至少在平均水平上是这样，所以通常最好寄希望于最好的结果并恢复。此外，**重传将是单播的，因此也将失去多播的优势**。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `4294967295`

##### //CycloneDDS/Domain/Internal/BurstSize/MaxRexmit

[Number-with-unit]: This element specifies the amount of data to be retransmitted in response to one NACK.

> 该元素指定要根据一个 NACK 重传的数据量。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `1 MiB`

#### //CycloneDDS/Domain/Internal/ControlTopic

> [!NOTE]: 如何理解 cyclone dds 中 "control topic"
> 在 Cyclone DDS 中，控制主题(Control Topic)是 DDS 中的一种特殊主题，**用于传输 DDS 实体的状态信息及相关控制命令**。它允许发送者发布一些关于 DDS 实体状态的信息，如实体的可用性、负载信息、QoS 设置等；同时也允许 DDS 实体接收和响应一些控制命令，如请求加入 DDS 网络、请求离开 DDS 网络、修改实体状态等。通过控制主题，DDS 实体能够更加智能化地协调和管理 DDS 网络中的实体，提高 DDS 的可管理性和稳定性。
>
> Cyclone DDS 中的控制主题通常包含两个主题：
>
> 1. DDS 状态主题(**DDS Status Topic**)：用于描述 Cyclone DDS 实体的状态信息，如可用性、负载信息、QoS 设置等。
> 2. DDS 控制主题(**DDS Control Topic**)：用于发送控制命令，如请求加入 DDS 网络、请求离开 DDS 网络、修改实体状态等。
>
> 控制主题的创建和使用需要根据具体场景进行配置和调整。一般来说，在设计 DDS 应用程序时，我们需要考虑 DDS 实体的管理性和稳定性，通过合理地使用控制主题，可以更好地协调 DDS 实体之间的相互作用，提高 DDS 应用程序的可操作性和可靠性。

The ControlTopic element allows configured whether Cyclone DDS provides a special control interface via a predefined topic or not.

> ControlTopic 元素允许配置 Cyclone DDS 是否通过预定义的主题提供特殊的控制接口。

#### //CycloneDDS/Domain/Internal/DefragReliableMaxSamples

> [!NOTE]: 如何理解 cyclone dds 中 "Defrag Reliable Max Samples"
> 在 Cyclone DDS 中，Defrag Reliable Max Samples 是一种可靠性配置参数，用于配置传输可靠性保证情况下的最大丢失样本数。当 DDS 的发布者和订阅者之间**使用可靠通信传输数据时，可能会发生数据丢失现象，影响 DDS 系统的可靠性和稳定性**。为了避免数据丢失并提高 DDS 系统的可靠性，Cyclone DDS 提供了 Defrag Reliable Max Samples 参数，通过设置这个参数**可以降低数据丢失的影响**。
>
> 具体来说，Defrag Reliable Max Samples 参数包含两个部分：Defrag 和 Reliable Max Samples。其中，Defrag 表示 DDS 可以合并已被按序接收的数据块，并剔除其中的重复元素，从而形成一份连续的数据，提高数据接收的效率和精度。通过设置 Reliable Max Samples 参数，DDS 可以在一定程度上降低数据丢失的影响，并确保接收到的数据尽可能完整和有序。当 DDS 中丢失数据时，Reliable Max Samples 将限制最大丢失样本数，并尝试重新传输已丢失的数据块，从而保证数据的可靠性和稳定性。
>
> 需要注意的是，Defrag Reliable Max Samples 参数的**设置需要根据具体场景进行调整和优化**。当 DDS**数据量大时，建议将参数设置为较大的值**，以降低数据丢失的风险。同时，如果 DDS 系统的**可靠性要求较高，则应将参数设置为较小的值，以提高数据的可靠性和精度**。

> [!note]: THINK
>
> 就是说对于可靠性的设置也是有更精细的控制，这些设置都是根据具体的场景有区别的
> 整体来说就是一个调参的过程，那么是否可以实现一个控制算法，可以动态的调整这些配置
> 或者，源码的具体实现上也可以直接支持对环境的探测，就是“传感器”？

[Integer]: This element sets the maximum number of samples that can be defragmented simultaneously for a reliable writer. This has to be large enough to handle retransmissions of historical data in addition to new samples.

> 该元素设置了可靠的 writer 可以同时碎片片的最大样本数量。除了新样本外，这必须足够大，以便处理历史数据的重传。

The default value is: `16`

#### //CycloneDDS/Domain/Internal/DefragUnreliableMaxSamples

[Integer]: This element sets the maximum number of samples that can be defragmented simultaneously for best-effort writers.

> 该元素设置了可以同时对最佳 writer 进行碎片打碎的最大样本数量。

The default value is: `4`

#### //CycloneDDS/Domain/Internal/DeliveryQueueMaxSamples

[Integer]: This element controls the maximum size of a delivery queue, expressed in samples. Once a delivery queue is full, incoming samples destined for that queue are dropped until space becomes available again.

> 该元素控制在样品中表达的递送队列的最大尺寸。**一旦送货队列已满，就会删除该队列的传入样本，直到再次可用为止**。

The default value is: `256`

#### //CycloneDDS/Domain/Internal/EnableExpensiveChecks

One of: Comma-separated list of: whc, rhc, xevent, all Or empty

This element enables expensive checks in builds with assertions enabled and is ignored otherwise. Recognised categories are:

> 该元素可以在启用主张的构建中进行开销的检查，否则将被忽略。公认的类别是：

> - whc: writer history cache checking
> - rhc: reader history cache checking
> - xevent: xevent checking

> - whc：writer 历史记录缓存检查
> - rhc：读取器历史记录缓存检查
> - xevent：xevent 检查

In addition, there is the keyword all that enables all checks.

> 此外，还有所有启用所有检查的关键字。

The default value is: `<empty>`

> [!NOTE]: 在 cyclone dds 中对 "xevent" 概念该如何理解
> 在 Cyclone DDS 中，“xevent”是指**交叉事件**。它是一种通信模型，允许 DDS 中的**两个不同实体(例如发布者和订阅者)以异步方式相互通信**。这种通信方法将通信实体的上下文从传统的同步通信方法中分离出来，**在 DDS 中被称为异步回调机制**。
>
> **当一个 DDS 实体希望与另一个实体交互时，它可以使用异步回调机制将其操作排队。然后，当相应的操作能够成功完成时，xevent 将被触发，DDS 实体将能够处理其结果**。
>
> 在 Cyclone DDS 中，**xevent 被用于在异步发布和异步订阅之间通信**。例如，当 DDS 发布者(publisher)成功发布数据时，它会将相应的回调操作排队，然后**等待异步**订阅者(subscriber)成功处理该数据。**一旦订阅者处理完成，xevent 机制将触发，publisher 将能够处理回调操作并更新其状态**。

#### //CycloneDDS/Domain/Internal/GenerateKeyhash

[Boolean]: When true, include keyhashes in outgoing data for topics with keys.

> 如果是正确的话，请将钥匙扣包含在带有密钥主题的外发数据中。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/HeartbeatInterval

Attributes: [max](<[//CycloneDDS/Domain/Internal/HeartbeatInterval[@max]](#cycloneddsdomaininternalheartbeatintervalmax)>), [min](<[//CycloneDDS/Domain/Internal/HeartbeatInterval[@min]](#cycloneddsdomaininternalheartbeatintervalmin)>), [minsched](<[//CycloneDDS/Domain/Internal/HeartbeatInterval[@minsched]](#cycloneddsdomaininternalheartbeatintervalminsched)>)

[Number-with-unit]: This element allows configuring the base interval for sending writer heartbeats and the bounds within which it can vary.

> 该元素允许配置用于发送 writer 心跳的基本间隔以及它可以变化的边界。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `100 ms`

> [!NOTE]
> 默认心跳间隔，100ms

#### //CycloneDDS/Domain/Internal/HeartbeatInterval[@max]

[Number-with-unit]: This attribute sets the maximum interval for periodic heartbeats.

> 此属性设置了**周期性心跳的最大间隔**。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `8 s`

#### //CycloneDDS/Domain/Internal/HeartbeatInterval[@min]

[Number-with-unit]: This attribute sets the minimum interval that must have passed since the most recent heartbeat from a writer, before another asynchronous (not directly related to writing) will be sent.

> 此属性设置了自 writer 最近的心跳以来必须通过的最小间隔，然后才会发送另一种异步(与写作不直接相关)。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `5 ms`

#### //CycloneDDS/Domain/Internal/HeartbeatInterval[@minsched]

[Number-with-unit]: This attribute sets the minimum interval for periodic heartbeats. Other events may still cause heartbeats to go out.

> 此属性设置了周期性心跳的最小间隔。**其他事件仍可能导致心跳出去**。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `20 ms`

#### //CycloneDDS/Domain/Internal/LateAckMode

[Boolean]: Ack a sample only when it has been delivered, instead of when committed to delivering it.

> ACK 仅在交付样本时，而不是承诺交付它。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/LivelinessMonitoring

Attributes: [Interval](<[//CycloneDDS/Domain/Internal/LivelinessMonitoring[@Interval]](#cycloneddsdomaininternallivelinessmonitoringinterval)>), [StackTraces](<[//CycloneDDS/Domain/Internal/LivelinessMonitoring[@StackTraces]](#cycloneddsdomaininternallivelinessmonitoringstacktraces)>)

[Boolean]: This element controls whether or not implementation should internally monitor its own liveliness. If liveliness monitoring is enabled, **stack traces can be dumped automatically when some thread appears to have stopped making progress**.

> 这个元素控制实现是否应该**在内部监控它自己的活跃度**。如果启用了活跃度监控，则当某些线程似乎已停止取得进展时，可以自动转储堆栈跟踪。

> [!NOTE]: 这段话主要是关于 Cyclone DDS 中“活跃性监测(Liveliness Monitoring)”的内容。
> 活跃性监测是指在 DDS 中为了确保通信的持续性和可靠性，定期检查参与者(publisher 或 subscriber)是否仍然处于活跃状态的机制。如果参与者出现问题，如阻塞或崩溃，通信将无法继续，进而导致整个 DDS 系统的故障。
>
> 在 Cyclone DDS 中，上述文本中提到的 “element” 指的是 DDS 参与者(publisher 或 subscriber)的配置选项。具体而言，这个 element 是一个布尔值，控制 DDS 参与者是否进行自身的活跃性监测。
>
> 如果开启了活跃性监测，DDS 系统就会在一定时间间隔内检查参与者是否还处于活跃状态。如果发现参与者已经超过预定时间没有表现出任何活跃性，则系统会尝试重连或者发送活跃性通知。
>
> 在此基础上，该段文本还提到了一个附加的功能，即**当某个线程似乎已经停止或者没有在有效的时间段内完成工作，Cyclone DDS 可以自动地显示线程的堆栈跟踪信息**。这样，系统管理员就可以快速地检测到工作中出现的问题，并及时采取措施避免其进一步影响整个系统的稳定运行。

> [!note]: THINK
>
> 感觉这个机制和动态冗余备份的关系还是挺大的

The default value is: `false`

#### //CycloneDDS/Domain/Internal/LivelinessMonitoring[@Interval]

[Number-with-unit]: This element controls the interval to check whether threads have been making progress.

> 该元素控制间隔以检查线程是否一直在进步。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

The default value is: `1s`

#### //CycloneDDS/Domain/Internal/LivelinessMonitoring[@StackTraces]

[Boolean]: This element controls whether or not to write stack traces to the DDSI2 trace when a thread fails to make progress (on select platforms only).

> 此元素控制线程未能取得进程(仅在选择平台上)时，是否将堆栈跟踪写入 DDSI2 跟踪。

The default value is: `true`

#### //CycloneDDS/Domain/Internal/MaxParticipants

[Integer]: This elements configures the maximum number of DCPS domain participants this Cyclone DDS instance is willing to service. 0 is unlimited.

> 此元素配置了 DCPS 域参与者(participant)的最大数量此 Cyclone DDS 实例愿意服务。0 是无限的。

The default value is: `0`

#### //CycloneDDS/Domain/Internal/MaxQueuedRexmitBytes

[Number-with-unit]: This setting limits the maximum number of bytes queued for retransmission. The default value of 0 is unlimited unless an AuxiliaryBandwidthLimit has been set, in which case it becomes NackDelay \* AuxiliaryBandwidthLimit. It must be large enough to contain the largest sample that may need to be retransmitted.

> 此设置**限制排队等待重新传输的最大字节数**。默认值 0 是无限制的，除非已设置 AuxiliaryBandwidthLimit，在这种情况下它变为 NackDelay\* AuxiliaryBandwidthLimit。它必须足够大以包含可能需要重传的最大样本。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `512 kB`

#### //CycloneDDS/Domain/Internal/MaxQueuedRexmitMessages

[Integer]: This setting limits the maximum number of samples queued for retransmission.

> 此设置限制了排队**重新启动的最大样品数量**。

The default value is: `200`

#### //CycloneDDS/Domain/Internal/MaxSampleSize

[Number-with-unit]: This setting controls the maximum (CDR) serialised size of samples that Cyclone DDS will forward in either direction. Samples larger than this are discarded with a warning.

> 此设置控制 Cyclone DDS 将在**任一方向转发的样本的最大 (CDR) 序列化大小**。大于此的样本将被丢弃并发出警告。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

The default value is: `2147483647 B`

#### //CycloneDDS/Domain/Internal/MeasureHbToAckLatency

[Boolean]: This element enables heartbeat-to-ack latency among Cyclone DDS services by prepending timestamps to Heartbeat and AckNack messages and calculating round trip times. This is non-standard behaviour. The measured latencies are quite noisy and are currently not used anywhere.

> 该元素通过准备时间戳到心跳和 acknack 消息并计算往返时间来实现 Cyclone DDS 服务中的心跳到 ACK 潜伏期。这是非标准的行为。测得的潜伏期非常嘈杂，**目前在任何地方都没有使用**。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/MonitorPort

[Integer]: This element allows configuring a service that dumps a text description of part the internal state to TCP clients. By default (-1), this is disabled; specifying 0 means a kernel-allocated port is used; a positive number is used as the TCP port number.

> 此元素允许配置将内部状态的文本描述转换为 TCP 客户端的文本描述的服务。默认情况下(-1)，这是禁用的；指定 0 表示使用内核分配的端口；正数用作 TCP 端口号。

The default value is: `-1`

#### //CycloneDDS/Domain/Internal/MultipleReceiveThreads

Attributes: [maxretries](<[//CycloneDDS/Domain/Internal/MultipleReceiveThreads[@maxretries]](#cycloneddsdomaininternalmultiplereceivethreadsmaxretries)>)

One of: false, true, default

This element controls whether all traffic is handled by a single receive thread (false) or whether multiple receive threads may be used to improve latency (true). By default it is disabled on Windows because it appears that one cannot count on being able to send packets to oneself, which is necessary to stop the thread during shutdown. Currently multiple receive threads are only used for connectionless transport (e.g., UDP) and ManySocketsMode not set to single (the default).

> 此元素控制**所有流量是否由单个接收线程处理 (false) 或是否可以使用多个接收线程来改善延迟 (true)**。默认情况下，它在 Windows 上是禁用的，因为它似乎不能指望能够向自己发送数据包，而这是在关闭期间停止线程所必需的。**目前，多个接收线程仅用于无连接传输(例如，UDP)并且 ManySocketsMode 未设置为单一(默认)**。

> [!note]: THINK
>
> 多线程接收数据

The default value is: `default`

#### //CycloneDDS/Domain/Internal/MultipleReceiveThreads[@maxretries]

[Integer]: Receive threads dedicated to a single socket can only be triggered for termination by sending a packet. Reception of any packet will do, so termination failure due to packet loss is exceedingly unlikely, but to eliminate all risks, it will retry as many times as specified by this attribute before aborting.

> 专用于单个套接字的接收线程只能通过发送数据包来触发终止。接收任何数据包都可以，因此由于数据包丢失而导致终止失败的可能性极小，但为了消除所有风险，它将在中止之前重试此属性指定的次数。

The default value is: `4294967295`

#### //CycloneDDS/Domain/Internal/NackDelay

[Number-with-unit]: This setting controls the delay between receipt of a HEARTBEAT indicating missing samples and a NACK (ignored when the HEARTBEAT requires an answer). However, no NACK is sent if a NACK had been scheduled already for a response earlier than the delay requests: then that NACK will incorporate the latest information.

> 此设置控制接收指示丢失样本的 HEARTBEAT 和 NACK(当 HEARTBEAT 需要回答时忽略)之间的延迟。但是，如果在延迟请求之前已经为响应安排了 NACK，则不会发送 NACK：那么 NACK 将包含最新信息。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.
The default value is: `100 ms`

#### //CycloneDDS/Domain/Internal/PreEmptiveAckDelay

[Number-with-unit]: This setting controls the delay between the discovering a remote writer and sending a pre-emptive AckNack to discover the available range of data.

> 此设置控制发现远程写入器和发送先发制人的 AckNack 以发现可用数据范围之间的延迟。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.
The default value is: `10 ms`

#### //CycloneDDS/Domain/Internal/PrimaryReorderMaxSamples

[Integer]: This element sets the maximum size in samples of a primary re-order administration. Each proxy writer has one primary re-order administration to buffer the packet flow in case some packets arrive out of order. Old samples are forwarded to secondary re-order administrations associated with readers needing historical data.

> 该元素在主要重新施用的样品中设置了最大尺寸。每个代理 writer 都有一个主要的重新订购管理，以缓冲数据包流，以防一些数据包到达订单。旧样本将转发到与需要历史数据的 reader 相关的二次重新订购管理。

The default value is: `128`

#### //CycloneDDS/Domain/Internal/PrioritizeRetransmit

[Boolean]: This element controls whether retransmits are prioritized over new data, speeding up recovery.

> 该元素控制是否优先考虑重新启动，从而加快恢复的速度。

The default value is: `true`

> [!note]: THINK
>
> 这个或许也可以用于多节点的热备份
> 这个节点备份的点，单纯从功能实现上来看，可能没有那么容易，但是如果从功能安全的角度来看，或许还是挺有研究价值的。

#### //CycloneDDS/Domain/Internal/RediscoveryBlacklistDuration

Attributes: [enforce](<[//CycloneDDS/Domain/Internal/RediscoveryBlacklistDuration[@enforce]](#cycloneddsdomaininternalrediscoveryblacklistdurationenforce)>)

[Number-with-unit]: This element controls for how long a remote participant that was previously deleted will remain on a blacklist to prevent rediscovery, giving the software on a node time to perform any cleanup actions it needs to do. To some extent this delay is required internally by Cyclone DDS, but in the default configuration with the 'enforce' attribute set to false, Cyclone DDS will reallow rediscovery as soon as it has cleared its internal administration. Setting it to too small a value may result in the entry being pruned from the blacklist before Cyclone DDS is ready, it is therefore recommended to set it to at least several seconds.

> 此元素**控制先前删除的远程参与者将在黑名单中保留多长时间以防止重新发现**，从而使节点上的软件**有时间执行它需要执行的任何清理操作**。在某种程度上，Cyclone DDS 内部需要这种延迟，但**在“enforce”属性设置为 false 的默认配置中，Cyclone DDS 将在清除其内部管理后立即重新发现**。设置过小的值可能会导致条目在 Cyclone DDS 准备好之前被从黑名单中删除，因此建议至少设置几秒。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.
The default value is: `0s`

> [!note]: THINK
>
> 这个节点或许也有价值，对于多节点的冗余备份

#### //CycloneDDS/Domain/Internal/RediscoveryBlacklistDuration[@enforce]

[Boolean]: This attribute controls whether the configured time during which recently deleted participants will not be rediscovered (i.e., "black listed") is enforced and following complete removal of the participant in Cyclone DDS, or whether it can be rediscovered earlier provided all traces of that participant have been removed already.

> 此属性控制**是否强制执行配置的时间**，在此期间不会重新发现最近删除的参与者(即“黑名单”)以及在完全删除 Cyclone DDS 中的参与者之后，或者是否可以更早地重新发现它 该参与者的踪迹已被删除。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/RetransmitMerging

One of: never, adaptive, always

This elements controls the addressing and timing of retransmits. Possible values are:

> 该元素控制着重传和寻址时间。可能的值是：

- never: retransmit only to the NACK-ing reader;
- adaptive: attempt to combine retransmits needed for reliability, but send historical (transient-local) data to the requesting reader only;
- always: do not distinguish between different causes, always try to merge.

> - 从不：仅重新授予 nack-ing reader；
> - 自适应：尝试合并可靠性所需的重传，但仅向请求的 reader 发送历史(瞬态 - 本地)数据；
> - 总是：不要区分不同的原因，请始终尝试合并。

The default is never. See also Internal/RetransmitMergingPeriod.
The default value is: `never`

#### //CycloneDDS/Domain/Internal/RetransmitMergingPeriod

[Number-with-unit]: This setting determines the time window size in which a NACK of some sample is ignored because a retransmit of that sample has been multicasted too recently. This setting has no effect on unicasted retransmits.

> 此设置确定了忽略某些样本的 nack 的时间窗口大小，因为该样本的重传封面也已被多播。此设置对单播的转换没有影响。

See also Internal/RetransmitMerging.
The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.
The default value is: `5 ms`

#### //CycloneDDS/Domain/Internal/RetryOnRejectBestEffort

[Boolean]: Whether or not to locally retry pushing a received best-effort sample into the reader caches when resource limits are reached.

> 当达到资源限制时，是否要在本地重试将接收到的最佳样品推入读取器。

The default value is: `false`

> [!note]: THINK
>
> 如果节点的问题是由于这类原因导致的，那是否类似的参数也应该考虑到
> 还有一个想法，这里介绍的配置点很多，相对应的 QoS 没有那么多
> 所以，QoS 本质上就是对这些零散配置的组合而已，这样的话，自己根据具体的场景需要也能组合出一些 QoS？

#### //CycloneDDS/Domain/Internal/SPDPResponseMaxDelay

[Number-with-unit]: Maximum pseudo-random delay in milliseconds between discovering aremote participant and responding to it.

> 在发现 Areemote 参与者(participant)和对其做出响应之间，毫秒的最大伪随机延迟。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.
The default value is: `0 ms`

#### //CycloneDDS/Domain/Internal/ScheduleTimeRounding

> [!note]: THINK
>
> 感觉在找与节点冗余备份相关的 element 时候，还能找到一些关于编排调度、优先级相关的配置
> 这个可以一起来看！

[Number-with-unit]: This setting allows the timing of scheduled events to be rounded up so that more events can be handled in a single cycle of the event queue. The default is 0 and causes no rounding at all, i.e. are scheduled exactly, whereas a value of 10ms would mean that events are rounded up to the nearest 10 milliseconds.

> 此设置允许向上舍入**计划事件的时间，以便可以在事件队列的单个周期中处理更多事件**。 默认值为 0 并且根本不进行舍入，即准确安排，而 10ms 的值表示事件向上舍入到最接近的 10 毫秒。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.
The default value is: `0 ms`

#### //CycloneDDS/Domain/Internal/SecondaryReorderMaxSamples

[Integer]: This element sets the maximum size in samples of a secondary re-order administration. The secondary re-order administration is per reader needing historical data.

> 此元素设置二次再订购管理样本的最大大小。 二次重新排序管理是每个需要历史数据的读者。

The default value is: `128`

#### //CycloneDDS/Domain/Internal/SocketReceiveBufferSize

Attributes: [max](<[//CycloneDDS/Domain/Internal/SocketReceiveBufferSize[@max]](#cycloneddsdomaininternalsocketreceivebuffersizemax)>), [min](<[//CycloneDDS/Domain/Internal/SocketReceiveBufferSize[@min]](#cycloneddsdomaininternalsocketreceivebuffersizemin)>)

The settings in this element control the size of the socket receive buffers. The operating system provides some size receive buffer upon creation of the socket, this option can be used to increase the size of the buffer beyond that initially provided by the operating system. If the buffer size cannot be increased to the requested minimum size, an error is reported.

> 此元素中的设置**控制套接字接收缓冲区的大小**。 操作系统在创建套接字时提供一些大小的接收缓冲区，此选项可用于增加缓冲区的大小，使其超出操作系统最初提供的大小。 如果缓冲区大小无法增加到请求的最小大小，则会报告错误。

The default setting requests a buffer size of **1MiB** but accepts whatever is available after that.

#### //CycloneDDS/Domain/Internal/SocketReceiveBufferSize[@max]

[Number-with-unit]: This sets the size of the socket receive buffer to request, with the special value of "default" indicating that it should try to satisfy the minimum buffer size. If both are at "default", it will request 1MiB and accept anything. It is ignored if the maximum is set to less than the minimum.

> 这将套接字接收缓冲区的大小设置为请求，并具有“默认值”的特殊值，表明它应该尝试满足最小缓冲区大小。如果两者都处于“默认”状态，它将请求 1MIB 并接受任何东西。如果将最大值设置为最小值，则将忽略它。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).
The default value is: `default`

#### //CycloneDDS/Domain/Internal/SocketReceiveBufferSize[@min]

[Number-with-unit]: This sets the minimum acceptable socket receive buffer size, with the special value "default" indicating that whatever is available is acceptable.

> 这将设置最小可接受的插座接收缓冲区大小，其中特殊值“默认”表示可以接受的任何可用的东西。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).
The default value is: `default`

#### //CycloneDDS/Domain/Internal/SocketSendBufferSize

Attributes: [max](<[//CycloneDDS/Domain/Internal/SocketSendBufferSize[@max]](#cycloneddsdomaininternalsocketsendbuffersizemax)>), [min](<[//CycloneDDS/Domain/Internal/SocketSendBufferSize[@min]](#cycloneddsdomaininternalsocketsendbuffersizemin)>)

The settings in this element control the size of the socket send buffers. The operating system provides some size send buffer upon creation of the socket, this option can be used to increase the size of the buffer beyond that initially provided by the operating system. If the buffer size cannot be increased to the requested minimum size, an error is reported.

> 此元素中的设置控制套接字**发送缓冲区的大小**。 操作系统在创建套接字时提供一定大小的发送缓冲区，此选项可用于增加缓冲区的大小，使其超出操作系统最初提供的大小。 如果缓冲区大小无法增加到请求的最小大小，则会报告错误。

The default setting requires a buffer of **at least 64KiB**.

#### //CycloneDDS/Domain/Internal/SocketSendBufferSize[@max]

[Number-with-unit]: This sets the size of the socket send buffer to request, with the special value of "default" indicating that it should try to satisfy the minimum buffer size. If both are at "default", it will use whatever is the system default. It is ignored if the maximum is set to less than the minimum.

> 这将套接字发送缓冲区的大小设置为请求，并具有“默认值”的特殊值，表明它应该尝试满足最小缓冲区大小。如果两者都处于“默认”状态，它将使用系统默认设置。如果将最大值设置为最小值，则将忽略它。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).
The default value is: `default`

#### //CycloneDDS/Domain/Internal/SocketSendBufferSize[@min]

[Number-with-unit]: This sets the minimum acceptable socket send buffer size, with the special value "default" indicating that whatever is available is acceptable.

> 这将设置最小可接受的套接字发送缓冲区大小，其中特殊值“默认”表示可以接受的任何可用的东西。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).
The default value is: `64 KiB`

#### //CycloneDDS/Domain/Internal/SquashParticipants

[Boolean]: This element controls whether Cyclone DDS advertises all the domain participants it serves in DDSI (when set to false), or rather only one domain participant (the one corresponding to the Cyclone DDS process; when set to true). In the latter case, Cyclone DDS becomes the virtual owner of all readers and writers of all domain participants, dramatically reducing discovery traffic (a similar effect can be obtained by setting Internal/BuiltinEndpointSet to "minimal" but with less loss of information).

> 该元素控制 Cyclone DDS 是通告它在 DDSI 中服务的**所有域参与者(当设置为 false 时)，还是仅通告一个域参与者**(对应于 Cyclone DDS 进程；当设置为 true 时)。 在后一种情况下，Cyclone DDS **成为所有域参与者的所有读取器和写入器的虚拟所有者**，显着减少发现流量(通过将 Internal/BuiltinEndpointSet 设置为“最小”但信息丢失较少可以获得类似的效果)。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/SynchronousDeliveryLatencyBound

[Number-with-unit]: This element controls whether samples sent by a writer with QoS settings transport_priority >= SynchronousDeliveryPriorityThreshold and a latency_budget at most this element's value will be delivered synchronously from the "recv" thread, all others will be delivered asynchronously through delivery queues. This reduces latency at the expense of aggregate bandwidth.

> 此元素控制具有 QoS 设置 transport_priority >= SynchronousDeliveryPriorityThreshold 和 latency_budget 的 writer 发送的样本是否最多此元素的值**将从“recv”线程同步传递，所有其他将通过传递队列异步传递。 这以聚合带宽为代价减少了延迟**。

Valid values are finite durations with an explicit unit or the keyword 'inf' for infinity. Recognised units: ns, us, ms, s, min, hr, day.
The default value is: `inf`

> [!NOTE]:
> 这里提到了在 QoS 中与优先级相关的一项设定：`transport_priority`

#### //CycloneDDS/Domain/Internal/SynchronousDeliveryPriorityThreshold

[Integer]: This element controls whether samples sent by a writer with QoS settings latency_budget <= SynchronousDeliveryLatencyBound and transport_priority greater than or equal to this element's value will be delivered synchronously from the "recv" thread, all others will be delivered asynchronously through delivery queues. This reduces latency at the expense of aggregate bandwidth.

> 该元素控制 QoS 设置 latency_budget <= SynchronousDeliveryLatencyBound 和 transport_priority 大于或等于该元素值的 writer 发送的样本是否将从“recv”线程同步交付，所有其他样本将通过交付队列异步交付。 这以聚合带宽为代价减少了延迟。

The default value is: `0`

#### //CycloneDDS/Domain/Internal/Test

Children: [//CycloneDDS/Domain/Internal/Test/XmitLossiness](#cycloneddsdomaininternaltestxmitlossiness)

Testing options.

##### //CycloneDDS/Domain/Internal/Test/XmitLossiness

[Integer]: This element controls the fraction of outgoing packets to drop, specified as samples per thousand.

> 此元素控制要丢弃的传出数据包的分数，指定为每千个样本。

The default value is: `0`

> [!note]: THINK
>
> 所以，DDS 也是会丢包的，哈

#### //CycloneDDS/Domain/Internal/UnicastResponseToSPDPMessages

> [!NOTE]: cyclone dds 中 "SPDP" 的具体含义，请详细解释
> Cyclone DDS 中的 "SPDP" 是指 Simple Participant Discovery Protocol 简单参与者发现协议。
>
> SPDP 是一种 DDS 基础协议，它允许在 DDS 网络中注册、发现和监听参与者。它**通常使用组播实现**，在 DDS 网络中查找其他 DDS 参与者。当一个 DDS 参与者加入网络时，它会向 DDS 网络发送 SPDP 消息，以将自己注册到 DDS 网络中。当一个 DDS 参与者要查找其他 DDS 参与者时，它可以发送 SPDP 消息来发现这些参与者。
>
> SPDP 协议由以下两个主要成分组成：
>
> - SPDP 发现消息：这些消息允许 DDS 参与者将自己注册到 DDS 网络，并在网络上检查、发现和监听其他参与者。
> - SPDP 心跳消息：这些消息允许 DDS 参与者在 DDS 网络上持久存在，以提供长期稳定性和可靠性。
>
> 总之，SPDP 是许多 DDS 实现中的一种基础协议，用于在 DDS 网络中注册、发现和监听参与者，从而为整个 DDS 系统提供基础设施。

[Boolean]: This element controls whether the response to a newly discovered participant is sent as a unicasted SPDP packet instead of rescheduling the periodic multicasted one. There is no known benefit to setting this to false.

> 此元素控制对新发现的参与者的响应是否作为**单播 SPDP 数据包发送，而不是重新安排周期性多播数据包**。 将此设置为 false 没有已知的好处。

The default value is: `true`

#### //CycloneDDS/Domain/Internal/UseMulticastIfMreqn

[Integer]: Do not use.

The default value is: `0`

#### //CycloneDDS/Domain/Internal/Watermarks

Children: [//CycloneDDS/Domain/Internal/Watermarks/WhcAdaptive](#cycloneddsdomaininternalwatermarkswhcadaptive), [//CycloneDDS/Domain/Internal/Watermarks/WhcHigh](#cycloneddsdomaininternalwatermarkswhchigh), [//CycloneDDS/Domain/Internal/Watermarks/WhcHighInit](#cycloneddsdomaininternalwatermarkswhchighinit), [//CycloneDDS/Domain/Internal/Watermarks/WhcLow](#cycloneddsdomaininternalwatermarkswhclow)

Watermarks for flow-control.

##### //CycloneDDS/Domain/Internal/Watermarks/WhcAdaptive

[Boolean]: This element controls whether Cyclone DDS will adapt the high-water mark to current traffic conditions based on retransmit requests and transmit pressure.

> 该元素控制 Cyclone DDS 是否会根据重新印记请求和发射压力使高水标记适应当前的交通状况。

The default value is: `true`

##### //CycloneDDS/Domain/Internal/Watermarks/WhcHigh

[Number-with-unit]: This element sets the maximum allowed high-water mark for the Cyclone DDS WHCs, expressed in bytes. A writer is suspended when the WHC reaches this size.

> 此元素设置 Cyclone DDS WHC 的最大允许高水位线，以字节表示。 当 WHC 达到此大小时，写入器将被挂起。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).
The default value is: `500 kB`

##### //CycloneDDS/Domain/Internal/Watermarks/WhcHighInit

[Number-with-unit]: This element sets the initial level of the high-water mark for the Cyclone DDS WHCs, expressed in bytes.

> 该元素设置了以字节表示的 Cyclone DDS WHC 的高水位标记的初始水平。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).

> 必须明确指定该单元。公认单位：B(字节)，KB 和 KIB(2^10 个字节)，MB＆MIB(2^20 个字节)，GB＆GIB(2^30 字节)。

The default value is: `30 kB`

##### //CycloneDDS/Domain/Internal/Watermarks/WhcLow

[Number-with-unit]: This element sets the low-water mark for the Cyclone DDS WHCs, expressed in bytes. A suspended writer resumes transmitting when its Cyclone DDS WHC shrinks to this size.

> 该元素设置了以字节表示的 Cyclone DDS WHC 的低水标记。当悬浮的 writer 的 Cyclone DDS WHC 缩小到这种大小时，暂停的 writer 会继续传输。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).
The default value is: `1 kB`

#### //CycloneDDS/Domain/Internal/WriteBatch

[Boolean]: This element enables the batching of write operations. By default each write operation writes through the write cache and out onto the transport. Enabling write batching causes multiple small write operations to be aggregated within the write cache into a single larger write. This gives greater throughput at the expense of latency. Currently, there is no mechanism for the write cache to automatically flush itself, so that if write batching is enabled, the application may have to use the dds_write_flush function to ensure that all samples are written.

> 此元素启用**写入操作的批处理**。 默认情况下，每个写操作都通过写缓存写入并输出到传输上。 启用**写批处理会导致多个小的写操作在写缓存中聚合成一个更大的写操作。 这以延迟为代价提供了更大的吞吐量。 目前，写缓存没有自动刷新自己的机制，所以如果启用了写批处理，应用程序可能必须使用 dds_write_flush 函数来确保所有样本都被写入**。

The default value is: `false`

#### //CycloneDDS/Domain/Internal/WriterLingerDuration

[Number-with-unit]: This setting controls the maximum duration for which actual deletion of a reliable writer with unacknowledged data in its history will be postponed to provide proper reliable transmission. The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.

> **此设置控制最大持续时间，实际删除在其历史记录中具有未确认数据的可靠编写器将被推迟以提供适当的可靠传输**。 必须明确指定单位。 认可的单位：ns、us、ms、s、min、hr、day。

The default value is: `1 s`

### //CycloneDDS/Domain/Partitioning

Children: [//CycloneDDS/Domain/Partitioning/IgnoredPartitions](#cycloneddsdomainpartitioningignoredpartitions), [//CycloneDDS/Domain/Partitioning/NetworkPartitions](#cycloneddsdomainpartitioningnetworkpartitions), [//CycloneDDS/Domain/Partitioning/PartitionMappings](#cycloneddsdomainpartitioningpartitionmappings)

The Partitioning element specifies Cyclone DDS network partitions and how DCPS partition/topic combinations are mapped onto the network partitions.

> 分区元素指定 Cyclone DDS 网络分区以及如何将 DCP 分区/主题组合映射到网络分区。

#### //CycloneDDS/Domain/Partitioning/IgnoredPartitions

Children: [//CycloneDDS/Domain/Partitioning/IgnoredPartitions/IgnoredPartition](#cycloneddsdomainpartitioningignoredpartitionsignoredpartition)

The IgnoredPartitions element specifies DCPS partition/topic combinations that are not distributed over the network.

> 忽略分区元素指定了未通过网络分布的 DCP 分区/主题组合。

##### //CycloneDDS/Domain/Partitioning/IgnoredPartitions/IgnoredPartition

Attributes: [DCPSPartitionTopic](<[//CycloneDDS/Domain/Partitioning/IgnoredPartitions/IgnoredPartition[@DCPSPartitionTopic]](#cycloneddsdomainpartitioningignoredpartitionsignoredpartitiondcpspartitiontopic)>)

[Text]: This element can prevent certain combinations of DCPS partition and topic from being transmitted over the network. Cyclone DDS will completely ignore readers and writers for which all DCPS partitions as well as their topic is ignored, not even creating DDSI readers and writers to mirror the DCPS ones.

> 该元素可以防止 DCPS 分区和主题的某些组合通过网络传输。 Cyclone DDS 将完全忽略所有 DCPS 分区及其主题都被忽略的读写器，甚至不会创建 DDSI 读写器来镜像 DCPS 分区。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Partitioning/IgnoredPartitions/IgnoredPartition[@DCPSPartitionTopic]

[Text]: This attribute specifies a partition and a topic expression, separated by a single '.', which are used to determine if a given partition and topic will be ignored or not. The expressions may use the usual wildcards '\*' and '?'. Cyclone DDS will consider a wildcard DCPS partition to match an expression if a string that satisfies both expressions exists.

> 此属性指定分区和主题表达式，由单个“.”分隔，用于确定是否忽略给定的分区和主题。 表达式可以使用常用的通配符“\*”和“?”。 如果存在满足两个表达式的字符串，Cyclone DDS 将考虑使用通配符 DCPS 分区来匹配表达式。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Partitioning/NetworkPartitions

Children: [//CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition](#cycloneddsdomainpartitioningnetworkpartitionsnetworkpartition)

The NetworkPartitions element specifies the Cyclone DDS network partitions.

> 网络分配元素指定 Cyclone DDS 网络分区。

##### //CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition

Attributes: [Address](<[//CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition[@Address]](#cycloneddsdomainpartitioningnetworkpartitionsnetworkpartitionaddress)>), [Interface](<[//CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition[@Interface]](#cycloneddsdomainpartitioningnetworkpartitionsnetworkpartitioninterface)>), [Name](<[//CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition[@Name]](#cycloneddsdomainpartitioningnetworkpartitionsnetworkpartitionname)>)

[Text]: This element defines a Cyclone DDS network partition.

> 该元素定义了 Cyclone DDS 网络分区。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition[@Address]

[Text]: This attribute specifies the addresses associated with the network partition as a comma-separated list. The addresses are typically multicast addresses. Non-multicast addresses are allowed, provided the "Interface" attribute is not used: \* An address matching the address or the "external address" (see General/ExternalNetworkAddress; default is the actual address) of a configured interface results in adding the corresponding "external" address to the set of advertised unicast addresses.

> 此属性将与网络分区关联的地址指定为逗号分隔列表。这些地址通常是多播地址。允许使用非媒体地址，只要不使用“接口”属性：\*一个与地址或“外部地址”匹配的地址(请参阅“ eneral/externalnetworkAddress;默认地址是实际的地址)”会导致添加添加该接口广告单媒体地址集的相应“外部”地址。

- An address corresponding to the (external) address of a configured interface, but not the address of the host itself, for example, a match when masking the addresses with the netmask for IPv4, results in adding the external address. For IPv4, this requires the host part to be all-zero.

> - 与已配置的接口的(外部)地址相对应的地址，但不是主机本身的地址，例如，用 IPv4 的 NetMask 掩盖地址时匹配，导致添加外部地址。对于 IPv4，这要求主机零件全零。

Readers matching this network partition (cf. Partitioning/PartitionMappings) will advertise all addresses listed to the matching writers via the discovery protocol and will join the specified multicast groups. The writers will select the most suitable address from the addresses advertised by the readers.

> 匹配此网络分区的 reader(参见分区/partitionmappings)将通过发现协议向匹配 writer 列出的所有地址，并将加入指定的多播组。writer 将从 reader 宣传的地址中选择最合适的地址。

The unicast addresses advertised by a reader are the only unicast addresses a writer will use to send data to it and are used to select the subset of network interfaces to use for transmitting multicast data with the intent of reaching it.

> reader 广告宣传的单媒体地址是 writer 将使用数据发送给数据的唯一单媒体地址，并用于选择用于传输多播数据的网络接口子集，以达到其访问。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition[@Interface]

[Text]: This attribute takes a comma-separated list of interface name that the reader is willing to receive data on. This is implemented by adding the interface addresses to the set address set configured using the sibling "Address" attribute. See there for more details.

> 此属性获取 reader 愿意接收数据的接口名称的逗号分隔列表。这是通过将接口地址添加到使用同级“地址”属性配置的设置地址集中来实现的。有关更多详细信息，请参见那里。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Partitioning/NetworkPartitions/NetworkPartition[@Name]

[Text]: This attribute specifies the name of this Cyclone DDS network partition. Two network partitions cannot have the same name. Partition mappings (cf. Partitioning/PartitionMappings) refer to network partitions using these names.

> 此属性指定此 Cyclone DDS 网络分区的名称。两个网络分区不能具有相同的名称。分区映射(参见分区/partitionMappings)使用这些名称请参阅网络分区。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Partitioning/PartitionMappings

Children: [//CycloneDDS/Domain/Partitioning/PartitionMappings/PartitionMapping](#cycloneddsdomainpartitioningpartitionmappingspartitionmapping)

The PartitionMappings element specifies the mapping from DCPS partition/topic combinations to Cyclone DDS network partitions.

> PartitionMappings 元素指定了从 DCPS 分区/主题组合到 Cyclone DDS 网络分区的映射。

##### //CycloneDDS/Domain/Partitioning/PartitionMappings/PartitionMapping

Attributes: [DCPSPartitionTopic](<[//CycloneDDS/Domain/Partitioning/PartitionMappings/PartitionMapping[@DCPSPartitionTopic]](#cycloneddsdomainpartitioningpartitionmappingspartitionmappingdcpspartitiontopic)>), [NetworkPartition](<[//CycloneDDS/Domain/Partitioning/PartitionMappings/PartitionMapping[@NetworkPartition]](#cycloneddsdomainpartitioningpartitionmappingspartitionmappingnetworkpartition)>)

[Text]: This element defines a mapping from a DCPS partition/topic combination to a Cyclone DDS network partition. This allows partitioning data flows by using special multicast addresses for part of the data and possibly encrypting the data flow.

> 该元素将映射从 DCPS 分区/主题组合定义为 Cyclone DDS 网络分区。这允许通过使用特殊的多播地址将数据流划分数据，并可能加密数据流。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Partitioning/PartitionMappings/PartitionMapping[@DCPSPartitionTopic]

[Text]: This attribute specifies a partition and a topic expression, separated by a single '.', which are used to determine if a given partition and topic maps to the Cyclone DDS network partition named by the NetworkPartition attribute in this PartitionMapping element. The expressions may use the usual wildcards '\*' and '?'. Cyclone DDS will consider a wildcard DCPS partition to match an expression if there exists a string that satisfies both expressions.

> 该属性指定一个分区和一个主题表达式，由单个“.”分隔，用于确定给定的分区和主题是否映射到由该 PartitionMapping 元素中的 NetworkPartition 属性命名的 Cyclone DDS 网络分区。 表达式可以使用常用的通配符“\*”和“?”。 如果存在满足两个表达式的字符串，Cyclone DDS 将考虑使用通配符 DCPS 分区来匹配表达式。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Partitioning/PartitionMappings/PartitionMapping[@NetworkPartition]

[Text]: This attribute specifies which Cyclone DDS network partition is to be used for DCPS partition/topic combinations matching the DCPSPartitionTopic attribute within this PartitionMapping element.

> 该属性指定哪些 Cyclone DDS 网络分区用于 DCPS 分区/主题组合，与此分区映射元素中的 DCPSPartitionTopic 属性匹配。

The default value is: `<empty>`

### //CycloneDDS/Domain/SSL

Children: [//CycloneDDS/Domain/SSL/CertificateVerification](#cycloneddsdomainsslcertificateverification), [//CycloneDDS/Domain/SSL/Ciphers](#cycloneddsdomainsslciphers), [//CycloneDDS/Domain/SSL/Enable](#cycloneddsdomainsslenable), [//CycloneDDS/Domain/SSL/EntropyFile](#cycloneddsdomainsslentropyfile), [//CycloneDDS/Domain/SSL/KeyPassphrase](#cycloneddsdomainsslkeypassphrase), [//CycloneDDS/Domain/SSL/KeystoreFile](#cycloneddsdomainsslkeystorefile), [//CycloneDDS/Domain/SSL/MinimumTLSVersion](#cycloneddsdomainsslminimumtlsversion), [//CycloneDDS/Domain/SSL/SelfSignedCertificates](#cycloneddsdomainsslselfsignedcertificates), [//CycloneDDS/Domain/SSL/VerifyClient](#cycloneddsdomainsslverifyclient)

The SSL element allows specifying various parameters related to using SSL/TLS for DDSI over TCP.

> SSL 元素允许指定与使用 TCP 对 DDSI 使用 SSL/TLS 相关的各种参数。

> [!NOTE]: 可以解释一下 安全相关的技术 "SSL" 的具体内容吗
> SSL (Secure Sockets Layer，安全套接层)是一种加密技术，用于 HTTPS (HTTP over SSL)传输协议，以确保在客户端和服务端之间传输的数据是安全、完整和私密的。
>
> SSL 的主要目的是通过使用公开密钥加密算法，如 RSA 和对称密钥加密算法，如 AES 和 3DES，来保护网络通信中的隐私和数据完整性。SSL 技术还使用证书验证的方法来确保客户端与服务端之间的认证和通信是可信的。
>
> 以下是 SSL 技术的主要内容：
>
> 1. 证书验证
>    SSL 将使用数字证书验证客户端和服务端之间的连接。证书由证书颁发机构(CA)签名，以确保通信的可信性和保密性。如果证书无效，通信将不会被信任。
> 2. 公钥加密
>    SSL 公钥加密使用 RSA 算法。连接的客户端产生一个公开密钥，用于加密所有将要发送给服务端的信息。 只有服务端的私人密钥可以解密发给它的信息。
> 3. 会话及数据完整性验证
>    SSL 还使用指定的对称密钥加密算法(如 AES 和 3DES)来加密所有通信，并使用 HMAC 支持验证会话标识符和数据完整性。
> 4. 握手协议
>    SSL 握手协议在双方之间交换密钥和算法，并建立一个可信的 SSL 连接。双方主要执行以下步骤：
>
> - 客户端和服务端建立 SSL 连接并交换 SSL 握手参数。
> - SSL 握手协议验证身份和密钥信息，并启用数据加密和完整性。
> - 之后，双方就开始通过 SSL 连接进行加密通信。
>
> 总之，SSL 技术是一种常用的加密技术，它使用公开密钥加密算法、对称密钥加密算法、证书验证、会话及数据完整性验证以及握手协议等元素，确保网络通信的隐私、完整性和保密性。

#### //CycloneDDS/Domain/SSL/CertificateVerification

[Boolean]: If disabled this allows SSL connections to occur even if an X509 certificate fails verification.

> 如果禁用，即使 X509 证书失败了验证，也会允许 SSL 连接发生。

The default value is: `true`

#### //CycloneDDS/Domain/SSL/Ciphers

[Text]: The set of ciphers used by SSL/TLS

> SSL/TLS 使用的密码集

The default value is: `ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH`

#### //CycloneDDS/Domain/SSL/Enable

[Boolean]: This enables SSL/TLS for TCP.

> 这使 TCP 启用 SSL/TLS。

The default value is: `false`

#### //CycloneDDS/Domain/SSL/EntropyFile

[Text]: The SSL/TLS random entropy file name.

> SSL/TLS 随机熵文件名。

The default value is: `<empty>`

#### //CycloneDDS/Domain/SSL/KeyPassphrase

[Text]: The SSL/TLS key pass phrase for encrypted keys.

> SSL/TLS 密钥通过加密键的传递短语。

The default value is: `secret`

#### //CycloneDDS/Domain/SSL/KeystoreFile

[Text]: The SSL/TLS key and certificate store file name. The keystore must be in PEM format.

> SSL/TLS 密钥和证书存储文件名。密钥库必须采用 PEM 格式。

The default value is: `keystore`

#### //CycloneDDS/Domain/SSL/MinimumTLSVersion

[Text]: The minimum TLS version that may be negotiated, valid values are 1.2 and 1.3.

> 可以协商的最小 TLS 版本，有效值为 1.2 和 1.3。

The default value is: `1.3`

#### //CycloneDDS/Domain/SSL/SelfSignedCertificates

[Boolean]: This enables the use of self signed X509 certificates.

> 这可以使用自签名的 X509 证书。

The default value is: `false`

#### //CycloneDDS/Domain/SSL/VerifyClient

[Boolean]: This enables an SSL server to check the X509 certificate of a connecting client.

> 这使 SSL Server 可以检查连接客户端的 X509 证书。

The default value is: `true`

### //CycloneDDS/Domain/Security

Children: [//CycloneDDS/Domain/Security/AccessControl](#cycloneddsdomainsecurityaccesscontrol), [//CycloneDDS/Domain/Security/Authentication](#cycloneddsdomainsecurityauthentication), [//CycloneDDS/Domain/Security/Cryptographic](#cycloneddsdomainsecuritycryptographic)

This element is used to configure Cyclone DDS with the DDS Security specification plugins and settings.

> 此元素用于使用 DDS 安全规范插件和设置配置 Cyclone DD。

#### //CycloneDDS/Domain/Security/AccessControl

Children: [//CycloneDDS/Domain/Security/AccessControl/Governance](#cycloneddsdomainsecurityaccesscontrolgovernance), [//CycloneDDS/Domain/Security/AccessControl/Library](#cycloneddsdomainsecurityaccesscontrollibrary), [//CycloneDDS/Domain/Security/AccessControl/Permissions](#cycloneddsdomainsecurityaccesscontrolpermissions), [//CycloneDDS/Domain/Security/AccessControl/PermissionsCA](#cycloneddsdomainsecurityaccesscontrolpermissionsca)

This element configures the Access Control plugin of the DDS Security specification.

> 此元素配置了 DDS 安全规范的访问控制插件。

##### //CycloneDDS/Domain/Security/AccessControl/Governance

[Text]: URI to the shared Governance Document signed by the Permissions CA in S/MIME format

> URI 到由 CA 签署的共享治理文件以 S/MIME 格式签署

URI schemes: file, data Examples file URIs:

> URI 计划：文件，数据示例文件 uris：

```xml
<Governance><file:governance.smime></Governance>
<Governance><file:/home/myuser/governance.smime></Governance> <Governance><![CDATA[<data:,MIME-Version>: 1.0
```

Content-Type: multipart/signed; protocol="application/x-pkcs7-signature"; micalg="sha-256"; boundary="----F9A8A198D6F08E1285A292ADF14DD04F"

> 内容类型：多部分/签名；协议=“ application/x-pkcs7-signature”;micalg =“ SHA-256”;boundard =“ ----- f9a8a198d6f08e1285A292ADF14DD04F”

This is an S/MIME signed message

> 这是 S/MIME 签名的消息

```xml
------F9A8A198D6F08E1285A292ADF14DD04F
<?xml version="1.0" encoding="UTF-8"?>
<dds xmlns:xsi="<http://www.w3.org/2001/XMLSchema-instance>"
xsi:noNamespaceSchemaLocation="omg_shared_ca_governance.xsd">
<domain_access_rules>
  . . .
</domain_access_rules>
</dds>
...
------F9A8A198D6F08E1285A292ADF14DD04F
```

Content-Type: application/x-pkcs7-signature; name="smime.p7s"
Content-Transfer-Encoding: base64
Content-Disposition: attachment; filename="smime.p7s"
MIIDuAYJKoZIhv ...al5s=
------F9A8A198D6F08E1285A292ADF14DD04F

> 内容类型：应用程序/x-pkcs7-signature;名称=“ smime.p7s”
> 内容传输编码：base64
> 内容分解：附件；文件名=“ smime.p7s”
> miiduayjkozhv ... al5s =
> ------ F9A8A198D6F08E1285A292ADF14DD04F-

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/AccessControl/Library

Attributes: [finalizeFunction](<[//CycloneDDS/Domain/Security/AccessControl/Library[@finalizeFunction]](#cycloneddsdomainsecurityaccesscontrollibraryfinalizefunction)>), [initFunction](<[//CycloneDDS/Domain/Security/AccessControl/Library[@initFunction]](#cycloneddsdomainsecurityaccesscontrollibraryinitfunction)>), [path](<[//CycloneDDS/Domain/Security/AccessControl/Library[@path]](#cycloneddsdomainsecurityaccesscontrollibrarypath)>)

[Text]: This element **specifies the library to be loaded** as the DDS Security Access Control plugin.

> 此元素指定要加载的库作为 DDS 安全访问控制插件。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/AccessControl/Library[@finalizeFunction]

[Text]: This element names the finalization function of Access Control plugin. This function is called to let the plugin release its resources.

> 该元素将访问控制插件的最终化功能命名。此功能被调用以使插件发布其资源。

The default value is: `finalize_access_control`

##### //CycloneDDS/Domain/Security/AccessControl/Library[@initFunction]

[Text]: This element names the initialization function of Access Control plugin. This function is called after loading the plugin library for instantiation purposes. The Init function must return an object that implements the DDS Security Access Control interface.

> 该元素将访问控制插件的初始化功能命名。为实例化，加载插件库后，将调用此功能。INIT 函数必须返回实现 DDS 安全访问控制接口的对象。

The default value is: `init_access_control`

##### //CycloneDDS/Domain/Security/AccessControl/Library[@path]

[Text]: This element points to the path of Access Control plugin library.

> 此元素指向访问控制插件库的路径。

It can be either absolute path excluding file extension ( /usr/lib/dds_security_ac ) or single file without extension ( dds_security_ac ).

> 它可以是绝对路径，不包括文件扩展名(/usr/lib/dds_security_ac)，也可以是没有扩展名的单个文件(dds_security_ac)。

If a single file is supplied, the library is located by the current working directory, or LD_LIBRARY_PATH for Unix systems, and PATH for Windows systems.

> 如果提供单个文件，则库位于当前工作目录，或 Unix 系统的 LD_LIBRARY_PATH 和 Windows 系统的 PATH。

The default value is: `dds_security_ac`

##### //CycloneDDS/Domain/Security/AccessControl/Permissions

[Text]: URI to the DomainParticipant permissions document signed by the Permissions CA in S/MIME format

> URI 到域专用权限文件，由 CA 签署的 S/MIME 格式签署

The permissions document specifies the permissions to be applied to a domain. Example file URIs:

> 权限文件指定要应用于域的权限。示例文件 uris：

```xml
<Permissions>[file:permissions_document.p7s](file:permissions_document.p7s)</Permissions>
<Permissions>[file:/path_to/permissions_document.p7s](file:/path_to/permissions_document.p7s)</Permissions>
```

Example data URI:

```xml
<Permissions><![CDATA[data:,.........]]</Permissions>
```

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/AccessControl/PermissionsCA

[Text]: URI to an X509 certificate for the PermissionsCA in PEM format.

> URI 以 PEM 格式获得 Permissionsca 的 X509 证书。

Supported URI schemes: file, data

> 支持的 URI 计划：文件，数据

The file and data schemas shall refer to a X.509 v3 certificate (see X.509 v3 ITU-T Recommendation X.509 (2005) [39]) in PEM format. Examples: `<PermissionsCA>[file:permissions_ca.pem](file:permissions_ca.pem)</PermissionsCA>`

> 文件和数据模式应参考 X.509 V3 证书(请参见 X.509 V3 ITU-T 建议 X.509(2005)\ [39 ])，采用 PEM 格式。示例：< persissionsca> [文件：permissions \\\\ \_ ca.pem](文件：permissions_ca.pem)</permissionsca>

```xml
<PermissionsCA>[file:/home/myuser/permissions_ca.pem](file:/home/myuser/permissions_ca.pem)</PermissionsCA> <PermissionsCA>data:<strong>,</strong>-----BEGIN CERTIFICATE-----
MIIC3DCCAcQCCQCWE5x+Z ... PhovK0mp2ohhRLYI0ZiyYQ==
-----END CERTIFICATE-----</PermissionsCA>
```

The default value is: `<empty>`

#### //CycloneDDS/Domain/Security/Authentication

Children: [//CycloneDDS/Domain/Security/Authentication/CRL](#cycloneddsdomainsecurityauthenticationcrl), [//CycloneDDS/Domain/Security/Authentication/IdentityCA](#cycloneddsdomainsecurityauthenticationidentityca), [//CycloneDDS/Domain/Security/Authentication/IdentityCertificate](#cycloneddsdomainsecurityauthenticationidentitycertificate), [//CycloneDDS/Domain/Security/Authentication/IncludeOptionalFields](#cycloneddsdomainsecurityauthenticationincludeoptionalfields), [//CycloneDDS/Domain/Security/Authentication/Library](#cycloneddsdomainsecurityauthenticationlibrary), [//CycloneDDS/Domain/Security/Authentication/Password](#cycloneddsdomainsecurityauthenticationpassword), [//CycloneDDS/Domain/Security/Authentication/PrivateKey](#cycloneddsdomainsecurityauthenticationprivatekey), [//CycloneDDS/Domain/Security/Authentication/TrustedCADirectory](#cycloneddsdomainsecurityauthenticationtrustedcadirectory)

This element configures the Authentication plugin of the DDS Security specification.

> 此元素配置了 DDS 安全规范的身份验证插件。

##### //CycloneDDS/Domain/Security/Authentication/CRL

[Text]: Optional URI to load an X509 Certificate Revocation List

> 可选的 URI 加载 X509 证书撤销列表

Supported URI schemes: file, data

> 支持的 URI 计划：文件，数据

Examples:

```xml
<CRL><file:crl.pem></CRL>
<CRL><data:,-----BEGIN> X509 CRL----- MIIEpAIBAAKCAQEA3HIh...AOBaaqSV37XBUJg= -----END X509 CRL-----</CRL>
```

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Authentication/IdentityCA

[Text]: URI to the X509 certificate [39] of the Identity CA that is the signer of Identity Certificate. Supported URI schemes: file, data

> IURI 是身份证书签名的身份 CA 的 X509 证书\ [39 ]。支持的 URI 计划：文件，数据

The file and data schemas shall refer to a X.509 v3 certificate (see X.509 v3 ITU-T Recommendation X.509 (2005) [39]) in PEM format.

> 文件和数据模式应以 PEM 格式参考 X.509 V3 证书(请参阅 X.509 V3 ITU-T 建议 X.509(2005)\ [39 ])。

Examples:

```xml
<IdentityCA>[file:identity_ca.pem](file:identity_ca.pem)</IdentityCA>
<IdentityCA><data:,-----BEGIN> CERTIFICATE----- MIIC3DCCAcQCCQCWE5x+Z...PhovK0mp2ohhRLYI0ZiyYQ== -----END CERTIFICATE-----</IdentityCA>
```

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Authentication/IdentityCertificate

[Text]: An identity certificate will identify all participants in the OSPL instance.The content is URI to an X509 certificate signed by the IdentityCA in PEM format containing the signed public key.

> 身份证书将在 OSPL 实例中确定所有参与者(participant)。

Supported URI schemes: file, data

> 支持的 URI 计划：文件，数据

Examples:

```xml
<IdentityCertificate>[file:participant1_identity_cert.pem](file:participant1_identity_cert.pem)</IdentityCertificate>
<IdentityCertificate><data:,-----BEGIN> CERTIFICATE----- MIIDjjCCAnYCCQDCEu9...6rmT87dhTo= -----END CERTIFICATE-----</IdentityCertificate>
```

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Authentication/IncludeOptionalFields

[Boolean]: The authentication handshake tokens may contain optional fields to be included for finding interoperability problems. If this parameter is set to true the optional fields are included in the handshake token exchange.

> 身份验证握手令牌可能包含用于查找互操作性问题的可选字段。如果将此参数设置为 true，则将可选字段包含在握手令牌交换中。

The default value is: `false`

##### //CycloneDDS/Domain/Security/Authentication/Library

Attributes: [finalizeFunction](<[//CycloneDDS/Domain/Security/Authentication/Library[@finalizeFunction]](#cycloneddsdomainsecurityauthenticationlibraryfinalizefunction)>), [initFunction](<[//CycloneDDS/Domain/Security/Authentication/Library[@initFunction]](#cycloneddsdomainsecurityauthenticationlibraryinitfunction)>), [path](<[//CycloneDDS/Domain/Security/Authentication/Library[@path]](#cycloneddsdomainsecurityauthenticationlibrarypath)>)

[Text]: This element specifies the library to be loaded as the DDS Security Access Control plugin.

> 此元素指定要加载的库作为 DDS 安全访问控制插件。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Authentication/Library[@finalizeFunction]

[Text]: This element names the finalization function of the Authentication plugin. This function is called to let the plugin release its resources.

> 该元素命名身份验证插件的最终化函数。此功能被调用以使插件发布其资源。

The default value is: `finalize_authentication`

##### //CycloneDDS/Domain/Security/Authentication/Library[@initFunction]

[Text]: This element names the initialization function of the Authentication plugin. This function is called after loading the plugin library for instantiation purposes. The Init function must return an object that implements the DDS Security Authentication interface.

> 该元素将身份验证插件的初始化函数命名。为实例化，加载插件库后，将调用此功能。INIT 函数必须返回实现 DDS 安全身份验证接口的对象。

The default value is: `init_authentication`

##### //CycloneDDS/Domain/Security/Authentication/Library[@path]

[Text]: This element points to the path of the Authentication plugin library.

> 该元素指向身份验证插件库的路径。

It can be either absolute path excluding file extension ( `/usr/lib/dds_security_auth` ) or single file without extension ( dds_security_auth ).

> 它可以是绝对路径，不包括文件扩展名(/usr/lib/dds_security_auth)，也可以是没有扩展名的单个文件(dds_security_auth)。

If a single file is supplied, the library is located by the current working directory, or LD_LIBRARY_PATH for Unix systems, and PATH for Windows systems.
The default value is: `dds_security_auth`

##### //CycloneDDS/Domain/Security/Authentication/Password

[Text]: A password is used to decrypt the private_key.

> 密码用于解密 private_key。

The value of the password property shall be interpreted as the Base64 encoding of the AES-128 key that shall be used to decrypt the private_key using AES128-CBC.

> 密码属性的值应解释为 AES-128 密钥的基本 64 编码，该密钥应使用 AES128-CBC 解密私有 private_Key。

If the password property is not present, then the value supplied in the private_key property must contain the unencrypted private key.

> 如果不存在密码属性，则在私有 private_key 属性中提供的值必须包含未加密的私钥。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Authentication/PrivateKey

[Text]: URI to access the private Private Key for all of the participants in the OSPL federation.

> URI 可以访问 OSPL 联合会所有参与者(participant)的私人密钥。

Supported URI schemes: file, data

> 支持的 URI 计划：文件，数据

Examples:

```xml
<PrivateKey>[file:identity_ca_private_key.pem](file:identity_ca_private_key.pem)</PrivateKey>
<PrivateKey><data:,-----BEGIN> RSA PRIVATE KEY----- MIIEpAIBAAKCAQEA3HIh...AOBaaqSV37XBUJg== -----END RSA PRIVATE KEY-----</PrivateKey>
```

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Authentication/TrustedCADirectory

[Text]: Trusted CA Directory which contains trusted CA certificates as separated files.

> 值得信赖的 CA 目录，其中包含可信赖的 CA 证书作为分离的文件。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Security/Cryptographic

Children: [//CycloneDDS/Domain/Security/Cryptographic/Library](#cycloneddsdomainsecuritycryptographiclibrary)

This element configures the Cryptographic plugin of the DDS Security specification.

> 此元素配置了 DDS 安全规范的加密插件。

##### //CycloneDDS/Domain/Security/Cryptographic/Library

Attributes: [finalizeFunction](<[//CycloneDDS/Domain/Security/Cryptographic/Library[@finalizeFunction]](#cycloneddsdomainsecuritycryptographiclibraryfinalizefunction)>), [initFunction](<[//CycloneDDS/Domain/Security/Cryptographic/Library[@initFunction]](#cycloneddsdomainsecuritycryptographiclibraryinitfunction)>), [path](<[//CycloneDDS/Domain/Security/Cryptographic/Library[@path]](#cycloneddsdomainsecuritycryptographiclibrarypath)>)

[Text]: This element specifies the library to be loaded as the DDS Security Cryptographic plugin.

> 该元素指定要加载的库作为 DDS 安全加密插件。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Security/Cryptographic/Library[@finalizeFunction]

[Text]: This element names the finalization function of the Cryptographic plugin. This function is called to let the plugin release its resources.

> 该元素命名了加密插件的最终化函数。此功能被调用以使插件发布其资源。

The default value is: `finalize_crypto`

##### //CycloneDDS/Domain/Security/Cryptographic/Library[@initFunction]

[Text]: This element names the initialization function of the Cryptographic plugin. This function is called after loading the plugin library for instantiation purposes. The Init function must return an object that implements the DDS Security Cryptographic interface.

> 该元素将加密插件的初始化函数命名。为实例化，加载插件库后，将调用此功能。INIT 函数必须返回实现 DDS 安全加密接口的对象。

The default value is: `init_crypto`

##### //CycloneDDS/Domain/Security/Cryptographic/Library[@path]

[Text]: This element points to the path of the Cryptographic plugin library.

> 该元素指向密码插件库的路径。

It can be either absolute path excluding file extension ( /usr/lib/dds_security_crypto ) or single file without extension ( dds_security_crypto ).

> 它可以是绝对路径，不包括文件扩展名(/usr/lib/dds_security_crypto)，也可以是没有扩展名的单个文件(dds_security_crypto)。

If a single file is supplied, the is library located by the current working directory, or LD_LIBRARY_PATH for Unix systems, and PATH for Windows systems.
The default value is: `dds_security_crypto`

### //CycloneDDS/Domain/SharedMemory

Children: [//CycloneDDS/Domain/SharedMemory/Enable](#cycloneddsdomainsharedmemoryenable), [//CycloneDDS/Domain/SharedMemory/Locator](#cycloneddsdomainsharedmemorylocator), [//CycloneDDS/Domain/SharedMemory/LogLevel](#cycloneddsdomainsharedmemoryloglevel), [//CycloneDDS/Domain/SharedMemory/Prefix](#cycloneddsdomainsharedmemoryprefix)

The Shared Memory element allows specifying various parameters related to using shared memory.

> 共享内存元素允许指定与使用共享内存有关的各种参数。

#### //CycloneDDS/Domain/SharedMemory/Enable

> [!note]: THINK
>
> 这里开始就是和 SM 相关的内容了

[Boolean]: This element allows for enabling shared memory in Cyclone DDS.

> 该元素允许在 Cyclone DDS 中启用共享内存。

The default value is: `false`

#### //CycloneDDS/Domain/SharedMemory/Locator

[Text]: Explicitly set the Iceoryx locator used by Cyclone to check whether a pair of processes is attached to the same Iceoryx shared memory. The default is to use one of the MAC addresses of the machine, which should work well in most cases.

> 显式设置 Cyclone 使用的 Iceoryx 定位器以**检查一对进程是否附加到同一 Iceoryx 共享内存**。 默认是使用机器的 MAC 地址之一，这在大多数情况下应该能正常工作。

The default value is: `<empty>`

#### //CycloneDDS/Domain/SharedMemory/LogLevel

One of: off, fatal, error, warn, info, debug, verbose

This element decides the verbosity level of shared memory message:

> 该元素决定共享内存消息的详细级别：

- off: no log
- fatal: show fatal log
- error: show error log
- warn: show warn log
- info: show info log
- debug: show debug log
- verbose: show verbose log

> - 关闭：没有日志
> - 致命：显示致命日志
> - 错误：显示错误日志
> - 警告：显示警告日志
> - 信息：显示信息日志
> - 调试：显示调试日志
> - 冗长：显示冗长日志

If you don't want to see any log from shared memory, use off to disable logging.
The default value is: `info`

#### //CycloneDDS/Domain/SharedMemory/Prefix

[Text]: Override the Iceoryx service name used by Cyclone.

> 覆盖 Cyclone 使用的 ICEORYX 服务名称。

The default value is: `DDS_CYCLONE`

### //CycloneDDS/Domain/Sizing

Children: [//CycloneDDS/Domain/Sizing/ReceiveBufferChunkSize](#cycloneddsdomainsizingreceivebufferchunksize), [//CycloneDDS/Domain/Sizing/ReceiveBufferSize](#cycloneddsdomainsizingreceivebuffersize)

The Sizing element allows you to specify various configuration settings dealing with expected system sizes, buffer sizes, &c.

> Sizing 元素允许您指定处理预期系统大小、缓冲区大小等的各种配置设置。

#### //CycloneDDS/Domain/Sizing/ReceiveBufferChunkSize

[Number-with-unit]: This element specifies the size of one allocation unit in the receive buffer. It must be greater than the maximum packet size by a modest amount (too large packets are dropped). Each allocation is shrunk immediately after processing a message or freed straightaway.

> 该元素指定接收缓冲区中一个分配单元的大小。 它**必须比最大数据包大小稍大(丢弃太大的数据包)**。 每个分配在处理消息后立即收缩或立即释放。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).
The default value is: `128 KiB`

#### //CycloneDDS/Domain/Sizing/ReceiveBufferSize

[Number-with-unit]: This element sets the size of a single receive buffer. Many receive buffers may be needed. The minimum workable size is a little larger than Sizing/ReceiveBufferChunkSize, and the value used is taken as the configured value and the actual minimum workable size.

> 该元素设置单个接收缓冲区的大小。 可能需要许多接收缓冲区。 **minimum workable size 比 Sizing/ReceiveBufferChunkSize 略大**，使用的值作为配置值和实际的 minimum workable size。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).
The default value is: `1 MiB`

### //CycloneDDS/Domain/TCP

Children: [//CycloneDDS/Domain/TCP/AlwaysUsePeeraddrForUnicast](#cycloneddsdomaintcpalwaysusepeeraddrforunicast), [//CycloneDDS/Domain/TCP/Enable](#cycloneddsdomaintcpenable), [//CycloneDDS/Domain/TCP/NoDelay](#cycloneddsdomaintcpnodelay), [//CycloneDDS/Domain/TCP/Port](#cycloneddsdomaintcpport), [//CycloneDDS/Domain/TCP/ReadTimeout](#cycloneddsdomaintcpreadtimeout), [//CycloneDDS/Domain/TCP/WriteTimeout](#cycloneddsdomaintcpwritetimeout)

The **TCP element** allows you to specify various parameters related to running DDSI over TCP.

#### //CycloneDDS/Domain/TCP/AlwaysUsePeeraddrForUnicast

[Boolean]: Setting this to true means the unicast addresses in SPDP packets will be ignored, and the peer address from the TCP connection will be used instead. This may help work around incorrectly advertised addresses when using TCP.

> 将此设置为 true 意味着将忽略 SPDP 数据包中的单播地址，而是使用来自 TCP 连接的对等地址。 这可能有助于解决使用 TCP 时错误公布的地址。

The default value is: `false`

> [!NOTE]: cyclone dds 中的 spdp 的具体含义是什么  
> SPDP(Simple Participant Discovery Protocol，简单参与者发现协议)是 Cyclone DDS 中用于 DDS 发现和识别的协议之一。它定义了 DDS 网络中使用的广播格式，以使不同的 DDS 程序能够发现彼此并在同一组中进行通信。
> **SPDP 通过组播方式在网络中进行广播**，其目的是为了寻找并识别位于同一个 DDS 分区中的其他 DDS 参与者。SPDP 的广播消息包含参与者的唯一标识符(GUID)、参与者的类型以及参与者存活状态的信息等。这些信息在 DDS 网络中起到了关键作用，因为它们能够让不同的 DDS 利益相关者在经过协商后加入到一个共同的分区中来。

#### //CycloneDDS/Domain/TCP/Enable

One of: false, true, default

This element enables the optional TCP transport - deprecated, use General/Transport instead.

> 此元素启用可选的 TCP 传输 - **已弃用，请改用 General/Transport**。

The default value is: `default`

#### //CycloneDDS/Domain/TCP/NoDelay

[Boolean]: This element enables the TCP_NODELAY socket option, preventing multiple DDSI messages from being sent in the same TCP request. Setting this option typically optimises latency over throughput.

> 此元素启用 TCP_NODELAY 套接字选项，防止在同一 TCP 请求中发送多个 DDSI 消息。 设置此选项**通常会优化延迟而不是吞吐量**。

The default value is: `true`

#### //CycloneDDS/Domain/TCP/Port

[Integer]: This element specifies the TCP port number on which Cyclone DDS accepts connections. If the port is set, it is used in entity locators, published with DDSI discovery, dynamically allocated if zero, and disabled if -1 or not configured. If disabled other DDSI services will not be able to establish connections with the service, the service can only communicate by establishing connections to other services.

> 此元素指定 Cyclone DDS 接受连接的 TCP 端口号。如果设置了端口，则将其用于实体定位器，并在 DDSI Discovery 发表，如果零为零，则将其动态分配，如果配置-1 或未配置，则将其禁用。如果其他 DDSI 服务将无法与服务建立连接，则该服务只能通过与其他服务建立连接来通信。

The default value is: `-1`

#### //CycloneDDS/Domain/TCP/ReadTimeout

[Number-with-unit]: This element specifies the timeout for blocking TCP read operations. If this timeout expires then the connection is closed.

> 此元素指定了阻止 TCP 读取操作的超时。如果此超时到期，则连接已关闭。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.
The default value is: `2 s`

#### //CycloneDDS/Domain/TCP/WriteTimeout

[Number-with-unit]: This element specifies the timeout for blocking TCP write operations. If this timeout expires then the connection is closed.

> 此元素指定阻止 TCP 写入操作的超时。如果此超时到期，则连接已关闭。

The unit must be specified explicitly. Recognised units: ns, us, ms, s, min, hr, day.
The default value is: `2 s`

### //CycloneDDS/Domain/Threads

Children: [//CycloneDDS/Domain/Threads/Thread](#cycloneddsdomainthreadsthread)

This element is used to set thread properties.

> [!NOTE]
> 这里是关于线程的的相关操作，感觉应该比较有用

#### //CycloneDDS/Domain/Threads/Thread

Attributes: [Name](<[//CycloneDDS/Domain/Threads/Thread[@Name]](#cycloneddsdomainthreadsthreadname)>) Children: [//CycloneDDS/Domain/Threads/Thread/Scheduling](#cycloneddsdomainthreadsthreadscheduling), [//CycloneDDS/Domain/Threads/Thread/StackSize](#cycloneddsdomainthreadsthreadstacksize)

This element is used to set thread properties.

#### //CycloneDDS/Domain/Threads/Thread[@Name]

[Text]: The Name of the thread for which properties are being set. The following threads exist:

> 设置属性的线程的名称。存在以下线程：

> - gc: garbage collector thread involved in deleting entities;
> - recv: receive thread, taking data from the network and running the protocol state machine;
> - dq.builtins: delivery thread for DDSI-builtin data, primarily for discovery;
> - lease: DDSI liveliness monitoring;
> - tev: general timed-event handling, retransmits and discovery;
> - fsm: finite state machine thread for handling security handshake;
> - xmit.CHAN: transmit thread for channel CHAN;
> - dq.CHAN: delivery thread for channel CHAN;
> - tev.CHAN: timed-event thread for channel CHAN.

> - **gc：参与删除实体的垃圾收集器线程**；
> - recv：接收线程，从网络中获取数据并运行协议状态机；
> - dq.builtins：DDSI 内置数据的传递线程，主要用于发现；
> - lease：DDSI 活跃度监控；
> - tev：一般定时事件处理、重传和发现；
> - fsm：用于处理安全握手的有限状态机线程；
> - xmit.CHAN：通道 CHAN 的传输线程；
> - dq.CHAN：通道 CHAN 的传送线程；
> - tev.CHAN：通道 CHAN 的定时事件线程。

The default value is: `<empty>`

##### //CycloneDDS/Domain/Threads/Thread/Scheduling

Children: [//CycloneDDS/Domain/Threads/Thread/Scheduling/Class](#cycloneddsdomainthreadsthreadschedulingclass), [//CycloneDDS/Domain/Threads/Thread/Scheduling/Priority](#cycloneddsdomainthreadsthreadschedulingpriority)

This element configures the **scheduling properties of the thread**.

###### //CycloneDDS/Domain/Threads/Thread/Scheduling/Class

One of: realtime, timeshare, default

This element specifies the thread scheduling class (realtime, timeshare or default). The user may need special privileges from the underlying operating system to be able to assign some of the privileged scheduling classes.

> 此元素指定线程调度类(实时，分时度假或默认值)。用户可能需要基础操作系统的特殊特权，以便能够分配一些特权计划类。

The default value is: `default`

###### //CycloneDDS/Domain/Threads/Thread/Scheduling/Priority

[Text]: This element specifies the thread priority (decimal integer or default). Only priorities supported by the underlying operating system can be assigned to this element. The user may need special privileges from the underlying operating system to be able to assign some of the privileged priorities.

> 此元素指定线程优先级(十进制整数或默认值)。 **只有底层操作系统支持的优先级才能分配给该元素**。 用户可能需要底层操作系统的特殊权限才能分配一些特权优先级。

The default value is: `default`

##### //CycloneDDS/Domain/Threads/Thread/StackSize

[Number-with-unit]: This element configures the stack size for this thread. The default value default leaves the stack size at the operating system default.

> 此元素为此线程配置堆栈大小。默认值默认值在操作系统默认情况下将堆栈大小留在堆栈大小上。

The unit must be specified explicitly. Recognised units: B (bytes), kB & KiB (2^10 bytes), MB & MiB (2^20 bytes), GB & GiB (2^30 bytes).
The default value is: `default`

### //CycloneDDS/Domain/Tracing

Children: [//CycloneDDS/Domain/Tracing/AppendToFile](#cycloneddsdomaintracingappendtofile), [//CycloneDDS/Domain/Tracing/Category](#cycloneddsdomaintracingcategory), [//CycloneDDS/Domain/Tracing/OutputFile](#cycloneddsdomaintracingoutputfile), [//CycloneDDS/Domain/Tracing/PacketCaptureFile](#cycloneddsdomaintracingpacketcapturefile), [//CycloneDDS/Domain/Tracing/Verbosity](#cycloneddsdomaintracingverbosity)

The Tracing element controls the amount and type of information that is written into the tracing log by the DDSI service. This is useful to track the DDSI service during application development.

> Tracing 元素控制由 DDSI 服务写入跟踪日志的信息量和类型。 这对于在应用程序开发期间跟踪 DDSI 服务很有用。

#### //CycloneDDS/Domain/Tracing/AppendToFile

[Boolean]: This option specifies whether **the output should be appended to an existing log file**. The default is to create a new log file each time, which is generally the best option if a detailed log is generated.

> 此选项指定是否应将输出附加到现有日志文件中。默认值是每次创建一个新的日志文件，如果生成详细的日志，通常是最佳选择。

The default value is: `false`

#### //CycloneDDS/Domain/Tracing/Category

One of: \* Comma-separated list of: fatal, error, warning, info, config, discovery, data, radmin, timing, traffic, topic, tcp, plist, whc, throttle, rhc, content, shm, trace \* Or empty

This element enables individual logging categories. These are enabled in addition to those enabled by Tracing/Verbosity. Recognised categories are:

> 此元素可以实现各个记录类别。除了通过跟踪/冗长启用的功能之外，这些还启用了这些。公认的类别是：

> - fatal: all fatal errors, errors causing immediate termination
> - error: failures probably impacting correctness but not necessarily causing immediate termination
> - warning: abnormal situations that will likely not impact correctness
> - config: full dump of the configuration
> - info: general informational notices
> - discovery: all discovery activity
> - data: include data content of samples in traces
> - radmin: receive buffer administration
> - timing: periodic reporting of CPU loads per thread
> - traffic: periodic reporting of total outgoing data
> - whc: tracing of writer history cache changes
> - tcp: tracing of TCP-specific activity
> - topic: tracing of topic definitions
> - plist: tracing of discovery parameter list interpretation

> - 致命：所有致命错误，导致立即终止的错误
> - 错误：失败可能会影响正确性，但不一定会立即终止
> - 警告：异常情况可能不会影响正确性
> - 配置：配置的完整转储
> - 信息：一般信息通知
> - 发现：所有发现活动
> - 数据：在轨迹中包括样本的数据内容
> - radmin：接收缓冲区给药
> - 定时：CPU 负载的定期报告
> - 流量：定期报告总数据
> - WHC：writer 历史记录的跟踪缓存更改
> - TCP：TCP 特异性活动的追踪
> - 主题：主题定义的追踪
> - PLIST：发现参数列表的跟踪解释

In addition, there is the keyword trace that enables all but radmin, topic, plist and whc. The categorisation of tracing output is incomplete and hence most of the verbosity levels and categories are not of much use in the current release. This is an ongoing process and here we describe the target situation rather than the current situation. Currently, the most useful is trace.

> 此外，还有一个关键字跟踪可以使除 radmin，topic，plist 和 whc 之外的所有内容。跟踪输出的分类不完整，因此大多数的详细水平和类别在当前版本中没有太多使用。这是一个持续的过程，在这里我们描述目标情况而不是当前情况。目前，最有用的是跟踪。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Tracing/OutputFile

[Text]: This option specifies where the logging is printed to. Note that stdout and stderr are treated as special values, representing "standard out" and "standard error" respectively. No file is created unless logging categories are enabled using the Tracing/Verbosity or Tracing/EnabledCategory settings.

> 此选项指定记录的打印位置。请注意，Stdout 和 Stderr 被视为特殊值，分别表示“标准输出”和“标准误差”。除非使用跟踪/冗长或示踪/启用范围设置启用记录类别，**否则不会创建文件**。

The default value is: `cyclonedds.log`

#### //CycloneDDS/Domain/Tracing/PacketCaptureFile

[Text]: This option specifies the file to which received and sent packets will be logged in the "pcap" format suitable for analysis using common networking tools, such as WireShark. IP and UDP headers are fictitious, in particular the destination address of received packets. The TTL may be used to distinguish between sent and received packets: it is 255 for sent packets and 128 for received ones. Currently IPv4 only.

> 此选项指定接收和发送的数据包将以“pcap”格式记录到的文件，适合使用常见网络工具(例如 WireShark)进行分析。 IP 和 UDP 标头是虚构的，尤其是接收到的数据包的目标地址。 TTL 可用于区分发送和接收的数据包：发送数据包为 255，接收数据包为 128。 目前只有 IPv4。

The default value is: `<empty>`

#### //CycloneDDS/Domain/Tracing/Verbosity

One of: finest, finer, fine, config, info, warning, severe, none

This element enables standard groups of categories, based on a desired verbosity level. This is in addition to the categories enabled by the Tracing/Category setting. Recognised verbosity levels and the categories they map to are:

> 该元素基于所需的详细性水平实现标准类别组。这是由跟踪/类别设置启用的类别的补充。公认的详细水平及其所映射的类别是：

- none: no Cyclone DDS log
- severe: error and fatal
- warning: severe + warning
- info: warning + info
- config: info + config
- fine: config + discovery
- finer: fine + traffic and timing
- finest: finer + trace

> - 无：没有 Cyclone DDS 日志
> - 严重：错误和致命
> - 警告：严重 +警告
> - 信息：警告 +信息
> - 配置：信息 +配置
> - 罚款：config +发现
> - 更精细：罚款 +交通和时机
> - 最好： +跟踪

While none prevents any message from being written to a DDSI2 log file.

> 虽然没有一个消息将任何消息写入 DDSI2 日志文件。

The categorisation of tracing output is incomplete and hence most of the verbosity levels and categories are not of much use in the current release. This is an ongoing process and here we describe the target situation rather than the current situation. Currently, the most useful verbosity levels are config, fine and finest.

> 跟踪输出的分类不完整，因此大多数详细级别和类别在当前版本中用处不大。 这是一个持续的过程，在这里我们描述的是目标情况而不是当前情况。 目前，最有用的冗长级别是 config、fine 和 finest。

The default value is: `none`
