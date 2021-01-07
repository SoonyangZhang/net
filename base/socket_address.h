#pragma once
#include  <string>
#include "ip_address.h"
namespace basic{
class SocketAddress {
 public:
  SocketAddress() {}
  SocketAddress(IpAddress address, uint16_t port);
  explicit SocketAddress(const struct sockaddr_storage& saddr);
  explicit SocketAddress(const sockaddr* saddr, int len);
  SocketAddress(const SocketAddress& other) = default;
  SocketAddress& operator=(const SocketAddress& other) = default;
  SocketAddress& operator=(SocketAddress&& other) = default;
  friend bool operator==(const SocketAddress& lhs,
                                             const SocketAddress& rhs);
  friend bool operator!=(const SocketAddress& lhs,
                                             const SocketAddress& rhs);

  bool IsInitialized() const;
  std::string ToString() const;
  int FromSocket(int fd);
  SocketAddress Normalized() const;

  IpAddress host() const;
  uint16_t port() const;
  sockaddr_storage generic_address() const;

 private:
  IpAddress host_;
  uint16_t port_ = 0;
};

inline std::ostream& operator<<(std::ostream& os,
                                const SocketAddress address) {
  os << address.ToString();
  return os;
}    
}
