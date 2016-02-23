#ifndef XXX_IOLOOP_H
#define XXX_IOLOOP_H

#include "Callbacks.h"

#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <list>
#include <memory>
#include <set>

// TODO: try and use pointers & forward decls in order to move the uv.h into the
// .cc file.
#include <uv.h>

namespace XXX {

class Logger;
class IOLoop;
class IOHandle;

typedef std::function<void(void)> AsyncCallback;
typedef std::function<void(IOHandle* ,int, tcp_connect_attempt_cb, void*)> NewConnectionCallback;

// TODO: try to move to impl file
struct io_request
{
  std::string addr;
  int port = 0;
  uv_tcp_t * tcp_connect = nullptr;
  tcp_connect_attempt_cb user_cb;
  void* user_data = nullptr;
  Logger * logptr;
  io_request(Logger * __logptr) : logptr(__logptr) {}
};

// TODO: maybe the IO loop should also do the TCP connect stuff? I.e., it all goes there?
/* Intended to be a reusable event loop */
class IOLoop
{
public:
  IOLoop(Logger * logger,
         AsyncCallback timer_cb,
         AsyncCallback async_cb);
  ~IOLoop();

  void start();
  void stop();
  void on_timer();
  void on_async();
  void run_loop();


  void async_send();

  void add_passive_handle(IOHandle* iohandle);
  void add_active_handle(IOHandle* iohandle, int status, io_request*);

  void add_server(int port);
  void add_connection(std::string addr, int port,
                      tcp_connect_attempt_cb, void*);

  uv_loop_t* uv_loop() { return m_uv_loop; }

  Logger * __logptr;
  uv_loop_t*   m_uv_loop;
  std::unique_ptr<uv_timer_t> m_timer;
  uv_async_t   m_async;
  std::thread  m_thread;
  std::atomic_bool m_continue_loop;

  NewConnectionCallback m_new_client_cb;

private:

  void create_tcp_server_socket(int port);

  AsyncCallback  m_timer_cb;
  AsyncCallback  m_async_cb;

  std::vector< io_request > m_pending_requests;
  std::mutex                m_pending_requests_lock;


  std::list< IOHandle* >m_handles; // TODO: need lock? No.


  std::list< std::unique_ptr<uv_tcp_t> > m_server_handles;


};

} // namespace XXX


/*

#libuv

lubuv:  can I submit bytes to write, off the io_loop thread?

 Saul: The one and *only* thread-safe function is uv_async_send.

 */
#endif