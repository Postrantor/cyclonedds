# TODO LIST

## Security

- Reassess Jeroen's comment:
  https://github.com/eclipse-cyclonedds/cyclonedds/pull/177#issuecomment-494040238

  > 5. If the security_api just becomes part of ddsc, and it should in my opinion, then I'd prefer you propagate the naming scheme as introduced in ddsrt etc and name the header files e.g. dds/ddssec/auth.h or something instead of dds/security/dds_security_api_authentication.h.

  > 如果 security_api 只是成为 ddsc 的一部分，并且我认为它应该如此，那么我更希望您传播 ddsrt 等中引入的命名方案并命名头文件，例如 dds/ddssec/auth.h 或其他东西而不是 dds/security/dds_security_api_authentication.h。

- Reassess Jeroen's comment:
  https://github.com/eclipse-cyclonedds/cyclonedds/pull/177#issuecomment-494040238

  > I've spent a great deal of time stripping out all the various different error codes and make it so that we simply use DDS*RETCODE* constants everywhere. This pull request reintroduces separate error codes and that's something I really don't approve of. The security error codes start at an offset of 100 and should nicely integrate with the other codes in dds/ddsrt/retcode.h. The messages should be retrievable using dds_strretcode if you ask me.

  > 我花了很多时间去除所有各种不同的错误代码，使我们可以在任何地方简单地使用 DDS*RETCODE* 常量。 这个拉取请求重新引入了单独的错误代码，这是我非常不赞成的。 安全错误代码从偏移量 100 开始，应该可以很好地与 dds/ddsrt/retcode.h 中的其他代码集成。 如果您问我，应该可以使用 dds_strretcode 检索这些消息。

- reassess Erik's comment
  https://github.com/eclipse-cyclonedds/cyclonedds/pull/177#issuecomment-490718462

  > GuidPrefix & BuiltinTopicKey change

- reassess erik's comment
  https://github.com/eclipse-cyclonedds/cyclonedds/pull/177#issuecomment-490718462

  > ddsrt_strchrs

- Reassess Jeroen's comment:
  https://github.com/eclipse-cyclonedds/cyclonedds/pull/177#issuecomment-494040238

  > If the security_api just becomes part of ddsc, and it should in my opinion, then I'd prefer you propagate the naming scheme as introduced in ddsrt etc and name the header files e.g. dds/ddssec/auth.h or something instead of dds/security/dds_security_api_authentication.h.

- Reassess Jeroen's comment:
  https://github.com/eclipse-cyclonedds/cyclonedds/pull/177#issuecomment-494040238
  > If the security_api just becomes part of ddsc, and it should in my opinion, then I'd prefer you propagate the naming scheme as introduced in ddsrt etc and name the header files e.g. dds/ddssec/auth.h or something instead of dds/security/dds_security_api_authentication.h.
