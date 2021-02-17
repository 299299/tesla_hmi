#include "messaging.hpp"
#include "impl_zmq.hpp"

namespace OP
{

  Context * Context::create(){
    return new ZMQContext();
  }

  SubSocket * SubSocket::create(){
    return new ZMQSubSocket();
  }

  SubSocket * SubSocket::create(Context * context, std::string endpoint){
    SubSocket *s = SubSocket::create();
    int r = s->connect(context, endpoint, "127.0.0.1");

    if (r == 0) {
      return s;
    } else {
      delete s;
      return NULL;
    }
  }

  SubSocket * SubSocket::create(Context * context, std::string endpoint, std::string address){
    SubSocket *s = SubSocket::create();
    int r = s->connect(context, endpoint, address);

    if (r == 0) {
      return s;
    } else {
      delete s;
      return NULL;
    }
  }

  SubSocket * SubSocket::create(Context * context, std::string endpoint, std::string address, bool conflate){
    SubSocket *s = SubSocket::create();
    int r = s->connect(context, endpoint, address, conflate);

    if (r == 0) {
      return s;
    } else {
      delete s;
      return NULL;
    }
  }

  PubSocket * PubSocket::create(){
    return new ZMQPubSocket();
  }

  PubSocket * PubSocket::create(Context * context, std::string endpoint){
    PubSocket *s = PubSocket::create();
    int r = s->connect(context, endpoint);

    if (r == 0) {
      return s;
    } else {
      delete s;
      return NULL;
    }
  }

  Poller * Poller::create(){
    return new ZMQPoller();;
  }

  Poller * Poller::create(std::vector<SubSocket*> sockets){
    Poller * p = Poller::create();

    for (auto s : sockets){
      p->registerSocket(s);
    }
    return p;
  }

  extern "C" Context * messaging_context_create() {
    return Context::create();
  }

  extern "C" SubSocket * messaging_subsocket_create(Context* context, const char* endpoint) {
    return SubSocket::create(context, std::string(endpoint));
  }

  extern "C" PubSocket * messaging_pubsocket_create(Context* context, const char* endpoint) {
    return PubSocket::create(context, std::string(endpoint));
  }

  extern "C" Poller * messaging_poller_create(SubSocket** sockets, int size) {
    std::vector<SubSocket*> socketsVec(sockets, sockets + size);
    return Poller::create(socketsVec);
  }

}
