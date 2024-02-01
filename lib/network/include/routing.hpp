#pragma once

#include <types.hpp>

namespace network {
class Routing {
  public:
    virtual ip_address_t next(const ip_address_t &this_id, const ip_address_t &packet_dst) const = 0;
};

class DefaultRouting : public Routing {
  public:
    virtual ip_address_t next(const ip_address_t &this_id, const ip_address_t &global_dst) const override {
        return BROADCAST_ADDRESS;
    }
};

}  // namespace network