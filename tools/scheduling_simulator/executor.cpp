
#include "simulator.hpp"

namespace simulator {

  void Executor::handle_requests()
  {
    MPI_Request barrier_request;
    MPI_Request msg_request;
    RequestMessage allocation{_comm};
    ReplyMessage reply{_comm};

    int idx;
    MPI_Status status;

    int free_cores = _cores;

    MPI_Ibarrier(_comm, &barrier_request);

    _results.start_iteration();

    while(true) {

      allocation.recv_allocation(&msg_request);
      MPI_Request requests[] = {barrier_request, msg_request};
      MPI_Waitany(2, requests, &idx, &status);

      if(idx == 0) {

        _results.end_iteration();
        _logger.debug("Everyone finished, leaving.");
        MPI_Cancel(&msg_request);
        MPI_Request_free(&msg_request);
        break;

      } else {

        _logger.debug("Received allocation request for {} cores from client {}.", allocation.get_allocation(), status.MPI_SOURCE);
        // reply to the client
        int request = allocation.get_allocation();
        if(request <= free_cores) {

          _results.register_request(request, true);
          reply.set_cores(request);
          free_cores -= request;

        } else if(free_cores > 0) {

          _results.register_request(request, true);
          reply.set_cores(free_cores);
          free_cores = 0;

        } else {

          _results.register_request(request, false);
          reply.set_failure();

        }
        reply.send_reply(status.MPI_SOURCE);

      }
    }
  }

}
