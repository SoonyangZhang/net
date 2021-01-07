#include <cstring>
#include <limits>
#include <string>
#include "socket_address.h"
#include "net_endian.h"
#include "string_utils.h"
namespace basic{
SocketAddress::SocketAddress(IpAddress address, uint16_t port)
    : host_(address), port_(port) {}

SocketAddress::SocketAddress(const struct sockaddr_storage& saddr) {
  switch (saddr.ss_family) {
    case AF_INET: {
      const sockaddr_in* v4 = reinterpret_cast<const sockaddr_in*>(&saddr);
      host_ = IpAddress(v4->sin_addr);
      port_ = QuicheEndian::NetToHost16(v4->sin_port);
      break;
    }
    case AF_INET6: {
      const sockaddr_in6* v6 = reinterpret_cast<const sockaddr_in6*>(&saddr);
      host_ = IpAddress(v6->sin6_addr);
      port_ = QuicheEndian::NetToHost16(v6->sin6_port);
      break;
    }
    default:
      break;
  }
}

SocketAddress::SocketAddress(const sockaddr* saddr, int len) {
  sockaddr_storage storage;
  static_assert(std::numeric_limits<int>::max() >= sizeof(storage),
                "Cannot cast sizeof(storage) to int as it does not fit");
  if (len < static_cast<int>(sizeof(sockaddr)) ||
      (saddr->sa_family == AF_INET &&
       len < static_cast<int>(sizeof(sockaddr_in))) ||
      (saddr->sa_family == AF_INET6 &&
       len < static_cast<int>(sizeof(sockaddr_in6))) ||
      len > static_cast<int>(sizeof(storage))) {
    return;
  }
  memcpy(&storage, saddr, len);
  *this = SocketAddress(storage);
}

bool operator==(const SocketAddress& lhs, const SocketAddress& rhs) {
  return lhs.host_ == rhs.host_ && lhs.port_ == rhs.port_;
}

bool operator!=(const SocketAddress& lhs, const SocketAddress& rhs) {
  return !(lhs == rhs);
}

bool SocketAddress::IsInitialized() const {
  return host_.IsInitialized();
}

std::string SocketAddress::ToString() const {
  switch (host_.address_family()) {
    case IpAddressFamily::IP_V4:
      return StrCat(host_.ToString(), ":", port_);
    case IpAddressFamily::IP_V6:
      return StrCat("[", host_.ToString(), "]:", port_);
    default:
      return "";
  }
}

int SocketAddress::FromSocket(int fd) {
  sockaddr_storage addr;
  socklen_t addr_len = sizeof(addr);
  int result = getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &addr_len);

  bool success = result == 0 && addr_len > 0 &&
                 static_cast<size_t>(addr_len) <= sizeof(addr);
  if (success) {
    *this = SocketAddress(addr);
    return 0;
  }
  return -1;
}

SocketAddress SocketAddress::Normalized() const {
  return SocketAddress(host_.Normalized(), port_);
}

IpAddress SocketAddress::host() const {
  return host_;
}

uint16_t SocketAddress::port() const {
  return port_;
}

sockaddr_storage SocketAddress::generic_address() const {
  union {
    sockaddr_storage storage;
    sockaddr_in v4;
    sockaddr_in6 v6;
  } result;
  memset(&result.storage, 0, sizeof(result.storage));

  switch (host_.address_family()) {
    case IpAddressFamily::IP_V4:
      result.v4.sin_family = AF_INET;
      result.v4.sin_addr = host_.GetIPv4();
      result.v4.sin_port = QuicheEndian::HostToNet16(port_);
      break;
    case IpAddressFamily::IP_V6:
      result.v6.sin6_family = AF_INET6;
      result.v6.sin6_addr = host_.GetIPv6();
      result.v6.sin6_port = QuicheEndian::HostToNet16(port_);
      break;
    default:
      result.storage.ss_family = AF_UNSPEC;
      break;
  }
  return result.storage;
}    
}
