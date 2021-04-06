
#ifndef __RFAAS_RESOURCES_HPP__
#define __RFAAS_RESOURCES_HPP__

#include <vector>
#include <cstdint>
#include <memory>
#include <cstring>

#include <cereal/types/vector.hpp> 
#include <cereal/types/string.hpp>

namespace rfaas {

  struct server_data
  {
    int16_t port;
    int16_t cores;
    char address[16];

    template <class Archive>
    void save(Archive & ar) const
    {
      ar(CEREAL_NVP(port), CEREAL_NVP(cores));//, CEREAL_NVP(address));
    }

    template <class Archive>
    void load(Archive & ar )
    {
      std::string addr;
      ar(CEREAL_NVP(port), CEREAL_NVP(cores), cereal::make_nvp("address", addr));
      strncpy(address, addr.c_str(), 16);
    }
  };

  struct servers
  {
    static std::unique_ptr<servers> _instance;
    std::vector<server_data> _data; 

    servers(int positions = 0);

    server_data & server(int idx);
    std::vector<int> select(int cores);

    template <class Archive>
    void save(Archive & ar) const
    {
      ar(CEREAL_NVP(_data));
    }

    template <class Archive>
    void load(Archive & ar )
    {
      ar(CEREAL_NVP(_data));
    }

    static servers & instance();
    static void deserialize(std::istream & in);
  };

};

#endif
