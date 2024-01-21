#pragma once

#include "packet_config.hpp"

namespace network {
class Routing {
  public:
    virtual flit::node_id_t next(const flit::node_id_t &this_id, const packet::dst_t &dst) const = 0;
};

class DefaultRouting : public Routing {
  public:
    virtual flit::node_id_t next(const flit::node_id_t &this_id, const packet::dst_t &dst) const override;
};

}  // namespace network