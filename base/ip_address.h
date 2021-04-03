// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif
#include <vector>
#include <ostream>
#include <string>
#include "ip_address_family.h"
namespace basic{
#define QUIC_EXPORT_PRIVATE
// Represents an IP address.
class IpAddress {
 public:
  // Sizes of IP addresses of different types, in bytes.
  enum : size_t {
    kIPv4AddressSize = 32 / 8,
    kIPv6AddressSize = 128 / 8,
    kMaxAddressSize = kIPv6AddressSize,
  };

  // TODO(fayang): Remove Loopback*() and use TestLoopback*() in tests.
  static IpAddress Loopback4();
  static IpAddress Loopback6();
  static IpAddress Any4();
  static IpAddress Any6();

  IpAddress();
  IpAddress(const IpAddress& other) = default;
  explicit IpAddress(const in_addr& ipv4_address);
  explicit IpAddress(const in6_addr& ipv6_address);
  IpAddress& operator=(const IpAddress& other) = default;
  IpAddress& operator=(IpAddress&& other) = default;
  QUIC_EXPORT_PRIVATE friend bool operator==(IpAddress lhs,
                                             IpAddress rhs);
  QUIC_EXPORT_PRIVATE friend bool operator!=(IpAddress lhs,
                                             IpAddress rhs);

  bool IsInitialized() const;
  IpAddressFamily address_family() const;
  int AddressFamilyToInt() const;
  // Returns the address as a sequence of bytes in network-byte-order. IPv4 will
  // be 4 bytes. IPv6 will be 16 bytes.
  std::string ToPackedString() const;
  // Returns string representation of the address.
  std::string ToString() const;
  // Normalizes the address representation with respect to IPv4 addresses, i.e,
  // mapped IPv4 addresses ("::ffff:X.Y.Z.Q") are converted to pure IPv4
  // addresses.  All other IPv4, IPv6, and empty values are left unchanged.
  IpAddress Normalized() const;
  // Returns an address suitable for use in IPv6-aware contexts.  This is the
  // opposite of NormalizeIPAddress() above.  IPv4 addresses are converted into
  // their IPv4-mapped address equivalents (e.g. 192.0.2.1 becomes
  // ::ffff:192.0.2.1).  IPv6 addresses are a noop (they are returned
  // unchanged).
  IpAddress DualStacked() const;
  bool FromPackedString(const char* data, size_t length);
  bool FromString(std::string str);
  bool IsIPv4() const;
  bool IsIPv6() const;
  bool InSameSubnet(const IpAddress& other, int subnet_length);

  in_addr GetIPv4() const;
  in6_addr GetIPv6() const;

 private:
  union {
    in_addr v4;
    in6_addr v6;
    uint8_t bytes[kMaxAddressSize];
    char chars[kMaxAddressSize];
  } address_;
  IpAddressFamily family_;
};

inline std::ostream& operator<<(std::ostream& os, const IpAddress address) {
  os << address.ToString();
  return os;
}
void GetLocalIpAddress(std::vector<IpAddress>&ips);
}
