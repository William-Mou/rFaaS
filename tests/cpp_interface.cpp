
#include <chrono>
#include <thread>
#include <fstream>

#include <spdlog/spdlog.h>
#include <cxxopts.hpp>

#include <rdmalib/rdmalib.hpp>
#include <rdmalib/recv_buffer.hpp>
#include <rdmalib/benchmarker.hpp>
#include <rdmalib/functions.hpp>

#include <rfaas/executor.hpp>
#include <rfaas/resources.hpp>

#include "warm_benchmark.hpp"

int main(int argc, char ** argv)
{
  auto opts = warm_benchmarker::options(argc, argv);
  if(opts.verbose)
    spdlog::set_level(spdlog::level::debug);
  else
    spdlog::set_level(spdlog::level::info);
  spdlog::set_pattern("[%H:%M:%S:%f] [T %t] [%l] %v ");
  spdlog::info("Executing serverless-rdma test warm_benchmarker!");

  // Read connection details to the managers
  std::ifstream in_cfg(opts.server_file);
  rfaas::servers::deserialize(in_cfg);
  in_cfg.close();
  rfaas::servers & cfg = rfaas::servers::instance();

  rfaas::executor executor(opts.address, opts.port, opts.recv_buf_size, opts.max_inline_data);
  executor.allocate(opts.flib, opts.numcores, opts.input_size, opts.hot_timeout, false);
  rdmalib::Buffer<char> in(opts.input_size, rdmalib::functions::Submission::DATA_HEADER_SIZE), out(opts.input_size);
  in.register_memory(executor._state.pd(), IBV_ACCESS_LOCAL_WRITE);
  out.register_memory(executor._state.pd(), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE);
  memset(in.data(), 0, opts.input_size);
  for(int i = 0; i < opts.input_size; ++i) {
    ((char*)in.data())[i] = 1;
  }

  spdlog::info("Blocking execution");
  executor.execute(opts.fname, in, out);
  spdlog::info("Blocking execution done");

  spdlog::info("Non-Blocking execution");
  auto f = executor.async(opts.fname, in, out);
  spdlog::info("NonBlocking execution done {}", f.get());

  // The result of future should arrive while polling for blocking result
  spdlog::info("Mixed execution");
  auto f2 = executor.async(opts.fname, in, out);
  executor.execute(opts.fname, in, out);
  spdlog::info("NonBlocking execution done {}", f2.get());

  executor.deallocate();


  return 0;
}
