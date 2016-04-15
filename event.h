#ifndef XXX_EVENT_H
#define XXX_EVENT_H

#include "Callbacks.h"

#include <jalson/jalson.h>

#include <functional>

namespace XXX {

struct Request_CB_Data;

struct event
{
  enum Type
  {
    session_state_event = 0,
    outbound_call_event,
    outbound_response_event,
    outbound_message,
    inbound_publish,
    outbound_subscribe,
    inbound_subscribed,
    router_session_connect_fail,
    inbound_message
  } type;

  session_handle src;
  t_connection_id user_conn_id;

  event(Type t,
        t_connection_id u = t_connection_id())
    : type(t),
      user_conn_id(u)
  {}

  virtual ~event(){}
};


struct ev_inbound_message : public event
{
  int msg_type; // WAMP message type
  jalson::json_array ja;

  void * user;
  Request_CB_Data* cb_data; // valid for responses to request

  unsigned int internal_req_id;

  ev_inbound_message(int __msgtype)
    : event(inbound_message),
      msg_type(__msgtype),
      user(nullptr),
      cb_data(nullptr)
  {
  }

  ~ev_inbound_message();
};


struct session_state_event : public event
{
  bool is_open;

  session_state_event(bool __session_open)
  : event( event::session_state_event ),
    is_open( __session_open )
  {}
};


struct outbound_message : public event
{
  session_handle destination;
  jalson::json_array ja;

  outbound_message()
  : event( event::outbound_message )
  {}
};


struct outbound_response_event : public event
{
  session_handle destination;
  int response_type;
  int request_type;
  t_request_id reqid;

  jalson::json_object options;
  std::string error_uri;  // used only for ERROR
  jalson::json_value subscription_id;

  rpc_args args;

  outbound_response_event()
    : event( event::outbound_response_event )
  {}
};



struct outbound_call_event : public event
{
  session_handle dest;
  std::string rpc_name;
  call_user_cb cb;
  void * cb_user_data;
  rpc_args args;
  unsigned int internal_req_id;

  outbound_call_event()
    : event( event::outbound_call_event ),
      cb_user_data( nullptr )
  {}
};


struct ev_inbound_publish : public event
{
  // 'is_internal' indicates the source of the publish is internal to the
  // program, rather than coming from an external client
  bool is_internal;
  std::string uri;
  jalson::json_value patch;  // TODO: maybe change to array?

  ev_inbound_publish(bool __is_internal,
                     const std::string & __topic_uri,
                     const jalson::json_value& __patch)
    : event( event::inbound_publish ),
      is_internal( __is_internal ),
      uri( __topic_uri ),
      patch( __patch )
  {}
};

// struct ev_outbound_event : public event
// {
//   std::string uri;
//   std::vector< session_handle > dest;
//   jalson::json_value event_msg;

//   ev_outbound_event(const std::string& __topic_uri,
//                    const std::vector< session_handle >& __dest,
//                    const jalson::json_value& __event_msg)
//   :  event( event::outbound_event ),
//      uri( __topic_uri ),
//      dest(__dest),
//      event_msg(__event_msg)
//   {
//   }
// };


struct ev_outbound_subscribe : public event
{
  session_handle dest;
  std::string uri;
  int internal_req_id;

  ev_outbound_subscribe(const std::string & __topic_uri)
  :  event( event::outbound_subscribe ),
     uri( __topic_uri )
  {}
};


struct ev_inbound_subscribed : public event
{
  session_handle src;
  unsigned int internal_req_id;
  jalson::json_array ja;

  ev_inbound_subscribed()
    : event( event::inbound_subscribed )
  {}

};


struct ev_router_session_connect_fail : public event
{
  int status;  /* 0 is no error */

  ev_router_session_connect_fail(t_connection_id __conn_id,
                                 int __status)
    : event( event::router_session_connect_fail, __conn_id),
      status(__status)
  {}

};

} // namespace xxx

#endif
