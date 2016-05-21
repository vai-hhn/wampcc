
#include "SessionMan.h"

#include "event_loop.h"
#include "kernel.h"
#include "wamp_session.h"

#include "WampTypes.h"
#include "Logger.h"

#include <string.h>

#define MAX_PENDING_OPEN_SECS 3

namespace XXX
{

SessionMan::SessionMan(kernel& k)
  : m_kernel(k),
    __logptr(k.get_logger())
{
  m_kernel.get_event_loop()->set_session_man( this );
}


// void SessionMan::heartbeat_all()
// {
//   jalson::json_array msg;

//   msg.push_back(HEARTBEAT);
//   std::lock_guard<std::mutex> guard(m_sessions.lock);

//   for (auto i : m_sessions.active)
//   {
//     if ( i.second->is_open() && i.second->hb_interval_secs())
//     {
//       // do heartbeat check on an open session
//       if (i.second->duration_since_last() > i.second->hb_interval_secs()*3)
//       {
//         // expire sessions which appear inactive
//           _WARN_("closing session due to inactivity " << i.second->hb_interval_secs() << ", " << i.second->duration_since_last());
//           i.second->close();
//       }
//       else
//       {
//         i.second->send_msg(msg);
//       }
//     }

//     if (i.second->is_pending_open())
//     {
//       if (i.second->duration_pending_open() >= MAX_PENDING_OPEN_SECS )
//       {
//         // expire sessions which have spent too long in pending-open
//         _WARN_("closing session due to incomplete handshake");
//         i.second->close();
//       }
//     }
//   }
// }



void SessionMan::handle_event(ev_session_state_event* ev)
{
  auto sp = ev->src.lock();
  if (!sp) return;

  {
    std::lock_guard<std::mutex> guard(m_sessions.lock);

    t_sid sid ( sp->unique_id() );

    auto it = m_sessions.active.find( sid );

    if (it == m_sessions.active.end())
    {
      _ERROR_("ignoring session state event for non active session sid:" << sid);
      return;
    }


    if (ev->is_open == false)
    {
      m_sessions.active.erase( it );
      m_sessions.closed.push_back(sp);
    }
  }

  if (m_session_event_cb) m_session_event_cb(ev);
}

//----------------------------------------------------------------------

void SessionMan::handle_housekeeping_event()
{
  // this->heartbeat_all();

  std::vector< std::shared_ptr<wamp_session> > to_delete;

  {
    std::lock_guard<std::mutex> guard(m_sessions.lock);
    to_delete.swap( m_sessions.closed );
  }

  to_delete.clear(); // expected to call ~wamp_session
}


void SessionMan::add_session(std::shared_ptr<wamp_session> sp)
{
  /* IO thread */
  std::lock_guard<std::mutex> guard(m_sessions.lock);
  m_sessions.active[ sp->unique_id() ] = sp;
}


}
