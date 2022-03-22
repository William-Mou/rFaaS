
#ifndef __RDMALIB_RDMALIB_HPP__
#define __RDMALIB_RDMALIB_HPP__

#include <cstdint>
#include <string>
#include <array>
#include <unordered_map>
#include <vector>
#include <functional>

#ifdef USE_LIBFABRIC
#include <rdma/fabric.h>
#include <arpa/inet.h>
#else
#include <rdma/rdma_cma.h>
#endif

#include <rdmalib/buffer.hpp>
#include <rdmalib/connection.hpp>

namespace rdmalib {

  // Implemented as IPV4
  struct Address {
    #ifdef USE_LIBFABRIC
    fi_info* addrinfo;
    fi_info* hints;
    fid_fabric* fabric;
    #else
    rdma_addrinfo *addrinfo;
    rdma_addrinfo hints;
    #endif
    uint16_t _port;

    Address(const std::string & ip, int port, bool passive);
    Address(const std::string & sip, const std::string & dip, int port);

    ~Address();
  };

  struct RDMAActive {
    #ifndef USE_LIBFABRIC
    ConnectionConfiguration _cfg;
    #endif
    std::unique_ptr<Connection> _conn;
    Address _addr;
    #ifdef USE_LIBFABRIC
    sockaddr_in _remote_addr;
    fid_eq* _ec;
    fid_domain* _pd;
    #else
    rdma_event_channel * _ec;
    ibv_pd* _pd;
    #endif

    RDMAActive(const std::string & ip, int port, int recv_buf = 1, int max_inline_data = 0);
    ~RDMAActive();
    void allocate();
    bool connect(uint32_t secret = 0);
    void disconnect();
    #ifdef USE_LIBFABRIC
    fid_domain* pd() const;
    #else
    ibv_pd* pd() const;
    #endif
    Connection & connection();
    bool is_connected();
  };

  struct RDMAPassive {
    #ifndef USE_LIBFABRIC
    ConnectionConfiguration _cfg;
    #endif
    Address _addr;
    #ifdef USE_LIBFABRIC
    fid_eq* _ec;
    fid_domain* _pd;
    fid_pep* _pep;
    #else
    rdma_event_channel * _ec;
    rdma_cm_id* _listen_id;
    ibv_pd* _pd;
    #endif
    // Set of connections that have been
    std::unordered_set<Connection*> _active_connections;

    RDMAPassive(const std::string & ip, int port, int recv_buf = 1, bool initialize = true, int max_inline_data = 0);
    ~RDMAPassive();
    void allocate();
    #ifdef USE_LIBFABRIC
    fid_domain* pd() const;
    #else
    ibv_pd* pd() const;
    #endif
    // Blocking poll for new rdmacm events.
    // Returns connection pointer and connection change status.
    // When connection is REQUESTED and ESTABLISHED, the pointer points to a valid connection.
    // When the status is DISCONNECTED, the pointer points to a closed connection.
    // User should deallocate the closed connection.
    // When the status is UNKNOWN, the pointer is null.
    std::tuple<Connection*, ConnectionStatus> poll_events(bool share_cqs = false);
    bool nonblocking_poll_events(int timeout = 100);
    void accept(Connection* connection);
    void set_nonblocking_poll();
  };

  #ifdef USE_LIBFABRIC
  struct eventEntry {
    fid_t fid;
    struct fi_info *info;
    uint32_t secret;
  };
  #endif
}

#endif

