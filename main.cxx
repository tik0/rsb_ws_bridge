#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <mongoose.h>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <mutex>
#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>

// RSB
#include <rsb/Informer.h>
#include <rsb/Factory.h>
#include <rsb/Event.h>
#include <rsb/Handler.h>
#include <rsb/converter/Repository.h>
#include <rsb/converter/PredicateConverterList.h>
#include <rsb/converter/ByteArrayConverter.h>

// RST
#include <rsb/converter/ProtocolBufferConverter.h>
#include "SimpleConverter.h"

typedef unsigned char uchar;

namespace rsb_ws_bridge {
// For debugging
static int currentTimer, lastTimerCam = 0;  //currentTimer holds the actual Time in microseconds
static struct timeval tv;
// RSB
static rsb::ListenerPtr listenerPtr;
static rsb::ParticipantConfig config;
static rsb::Factory& factory = rsb::Factory::getInstance();
// RSB WS interaction
static bool newRsbData = false;
static std::string newRsbPayload;
static std::string futureScope;
static std::mutex mutex;
}

void receiveRsbData(boost::shared_ptr<std::string> e) {
  rsb_ws_bridge::mutex.lock();
  rsb_ws_bridge::newRsbData = true;
  std::cout << "Received RSB event with " << e->size() << " bytes" << std::endl;
  std::cout << "Payload: " << *e << std::endl;
  rsb_ws_bridge::newRsbPayload = *e;
  rsb_ws_bridge::mutex.unlock();
}

static int process_request(struct mg_connection *conn) {

  // If connection is a websocket connection -> all received signals are processed here
  if (conn->is_websocket) {

    // Process the received signal
    rsb_ws_bridge::futureScope = std::string(conn->content, conn->content_len);

    if (rsb_ws_bridge::futureScope[0] == '/') {
      std::cout << "Set scope to: " << rsb_ws_bridge::futureScope << std::endl;

      // Set the new listener configuration
      rsb_ws_bridge::listenerPtr = rsb_ws_bridge::factory.createListener(
          rsb::Scope(rsb_ws_bridge::futureScope), rsb_ws_bridge::config);
      rsb_ws_bridge::listenerPtr->addHandler(
          rsb::HandlerPtr(
              new rsb::DataFunctionHandler<std::string>(&receiveRsbData)));
    }

    //Getting connection closed Signal by Client
// 		std::cout << "conn->wsbits: " << conn->wsbits << std::endl;
    if (conn->wsbits == 136) {
      std::cout << "socket closed" << "/" << conn->wsbits << std::endl;
    }

    return
        conn->content_len == 4 && !memcmp(conn->content, "exit", 4) ?
            MG_FALSE : MG_TRUE;
  } else {
    //load everything that belongs to the html site automatically
    return MG_FALSE;
  }
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) {

  // If a new CLient connects
  if (ev == MG_WS_HANDSHAKE) {
  }
  // If a Client leaves
  if (ev == !MG_REPLY) {
  }

  // Handle client request (and websocket receives)
  if (ev == MG_REQUEST) {
    return process_request(conn);
  } else if (ev == MG_AUTH) {  // If not included you have to authenticate to load the homepage
    return MG_TRUE;
  }

  // Handle websocket sending
  if (ev == MG_POLL && conn->is_websocket) {
    rsb_ws_bridge::mutex.lock();
    if (rsb_ws_bridge::newRsbData) {
      std::cout << "Send WS event with " << rsb_ws_bridge::newRsbPayload.size()
                << " bytes" << std::endl;
      std::cout << "Payload: " << rsb_ws_bridge::newRsbPayload << std::endl;
      mg_websocket_write(conn, 2,
                         (const char*) rsb_ws_bridge::newRsbPayload.c_str(),
                         rsb_ws_bridge::newRsbPayload.size());
      rsb_ws_bridge::newRsbData = false;
    }
    rsb_ws_bridge::mutex.unlock();
    return MG_FALSE;
  }
  return MG_FALSE;
}

int main(int argc, char **argv) {

  // Create a config for anonymous deserialization
  {
    std::list<
        std::pair<rsb::converter::ConverterPredicatePtr,
            typename rsb::converter::Converter<std::string>::Ptr> > converters;
    // Use a converter which can handle everything.
    converters.push_back(
        std::make_pair(
            rsb::converter::ConverterPredicatePtr(
                new rsb::converter::AlwaysApplicable()),
            typename rsb::converter::Converter<std::string>::Ptr(
                new rsb::converter::ByteArrayConverter())));
    rsb::converter::ConverterSelectionStrategy<std::string>::Ptr css =
        rsb::converter::ConverterSelectionStrategy<std::string>::Ptr(
            new rsb::converter::PredicateConverterList<std::string>(
                converters.begin(), converters.end()));

    // Configure a Listener object.
    rsb_ws_bridge::config =
        rsb_ws_bridge::factory.getDefaultParticipantConfig();

    std::set<rsb::ParticipantConfig::Transport> transports =
        rsb_ws_bridge::config.getTransports();
    for (std::set<rsb::ParticipantConfig::Transport>::const_iterator it =
        transports.begin(); it != transports.end(); ++it) {
      rsb::ParticipantConfig::Transport& transport = rsb_ws_bridge::config
          .mutableTransport(it->getName());
      rsc::runtime::Properties options = transport.getOptions();
      options["converters"] = css;
      transport.setOptions(options);
    }
  }

  // Create the mongoose server
  struct mg_server *server = mg_create_server(NULL, ev_handler);
  mg_set_option(server, "listening_port", "8080");
  mg_set_option(server, "document_root", "./www");
  std::cout << "Started on port " << mg_get_option(server, "listening_port")
            << std::endl;

  // Begin of interation loop
  unsigned int counter = 0;
  for (;;) {

    // Handle the server
    mg_poll_server(server, 100 /*ms*/);

    // Print out the pollings per seconds, the server has to process
    gettimeofday(&rsb_ws_bridge::tv, 0);
    rsb_ws_bridge::currentTimer = rsb_ws_bridge::tv.tv_sec * 1000
        + rsb_ws_bridge::tv.tv_usec / 1000;
    if (rsb_ws_bridge::currentTimer - rsb_ws_bridge::lastTimerCam
        >= 1000 /*ms*/) {
      std::cout << "PPS: " << counter << " polls / "
                << rsb_ws_bridge::currentTimer - rsb_ws_bridge::lastTimerCam
                << " ms" << std::endl;
      rsb_ws_bridge::lastTimerCam = rsb_ws_bridge::currentTimer;
      counter = 0;
    }
    ++counter;
  }

  mg_destroy_server(&server);
  return 0;
}

