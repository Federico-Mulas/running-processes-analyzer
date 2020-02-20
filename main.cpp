#include "./arguments.hpp"
#include "./parser.hpp"
#include "./utils.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <curlpp/Easy.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <fstream>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>
#include <vector>

using json = nlohmann::json;
using namespace std;
using namespace rpa;

/**
 * @brief thread function, it parse `ps aux` command and write it to ostream
 *
 * the command name is searched, it works also including full path, but it
 * doesn't if the program name is a parameter
 *
 * @param app_list_lock a spin lock for the \p app_list
 * @param app_list the list of app to monitor. it is shared and protected by \p
 * app_list_lock
 * @param pause_mutex mutex utilized to pause the thread with \p pause_condition
 * @param pause_condition condition utilized to pause the thread with \p
 * pause_mutex
 * @param probe_frequency pause time between consequent probes
 * @param out outpust stream for info result
 */
void utilizationProber(atomic_flag &app_list_lock,
                       const vector<string> &app_list, mutex &pause_mutex,
                       condition_variable &pause_condition,
                       const chrono::seconds &probe_frequency, ostream &out) {
  ps_parser parser;
  try {
    while (true) {
      while (!app_list_lock.test_and_set(memory_order_acquire)) // spinlock
        ;
      // if the are app to monitor
      if (app_list.size() > 0) {
        parser.setSource(popen("ps aux", "r"));
        while (parser) {
          info i = parser.line();
          // filter info by command
          for (const string &command : app_list) {
            // lower read command since provided command are lowered
            if (end_with(to_lower(i.command), command))
              out << i.command << " " << i.pid << " " << i.cpu << " " << i.mem
                  << endl;
          }
        }
        app_list_lock.clear(memory_order_release); // end critical section
        this_thread::sleep_for(probe_frequency);
      } else {
        // wait for app to monitor
        app_list_lock.clear(memory_order_release); // end critical section
        unique_lock<mutex> lock(pause_mutex);
        pause_condition.wait(lock);
      }
    }
  } catch (std::system_error &err) {
    app_list_lock.clear(); // ensure clear of the lock
    cerr << err.what();
  }
}

int main(int argc, char **argv) {
  // parameters parsing
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  const chrono::seconds probe_frequency(FLAGS_probe_frequency);
  const chrono::seconds config_probe_frequency(FLAGS_probe_frequency);
  ostream *out;
  ofstream file_out;
  if (FLAGS_output_file == "") {
    out = &cout; // set cout by default
  } else {
    file_out.open(FLAGS_output_file);
    if (!file_out) {
      cerr << "error on output file " << FLAGS_output_file << endl;
      return 1;
    }
    out = &file_out;
  }

  vector<string> endpoints;
  if (FLAGS_config_file != "") {
    ifstream config_file(FLAGS_config_file);
    string s;
    while (config_file) {
      config_file >> s;
      if (s != "")
        endpoints.push_back(s);
    }
    if (endpoints.size() == 0) {
      cerr << "error on input file " << FLAGS_config_file
           << " missing endpoints" << endl;
      return 2;
    }
  } else {
    // add some sample endpoint by default
    // FIXME for release this branch must be removed in favor of an error
    endpoints = {"http://my-json-server.typicode.com/Federico-Mulas/"
                 "running-processes-analyzer/applications/1",
                 "http://my-json-server.typicode.com/Federico-Mulas/"
                 "running-processes-analyzer/applications/2",
                 "http://my-json-server.typicode.com/Federico-Mulas/"
                 "running-processes-analyzer/applications/3"};
  }

  // intial setup
  mutex pause_mutex;
  condition_variable pause_condition;
  atomic_flag app_list_lock(false); // clear the flag by default
  vector<string> app_list;
  thread prober(utilizationProber, ref(app_list_lock), ref(app_list),
                ref(pause_mutex), ref(pause_condition), ref(probe_frequency),
                ref(*out));

  json j;       // initialize json object outside to minimize allocations
  size_t i = 0; // index for navigate endpoints
  while (true) {
    const string &config_url = endpoints[i];
    i = (i + 1) % endpoints.size(); // navigate endpoints in circular manner
    try {
      // retriving app to monitor from endpoints
      curlpp::Cleanup cleaner;
      ostringstream s;
      s << curlpp::options::Url(config_url);
      j = json::parse(s.str());
      vector<string> new_app_list = j["applications"].get<vector<string>>();
      // lower commands
      for (string &i : new_app_list)
        rpa::to_lower(i);

      size_t n_apps = app_list.size();
      // if probe thread is waiting for app to monitor and we didn't received
      // something yet avoid lock and swaps
      if (!(n_apps == 0 && new_app_list.size() == 0)) {
        while (!app_list_lock.test_and_set(memory_order_acquire)) // spinlock
          ;
        app_list =
            new_app_list; // change the list using the spinlock (this
                          // perform a std::move so should be pretty fast)
        app_list_lock.clear(memory_order_release); // end critical section
        if (n_apps == 0) { // if thread was waiting, notify it
          pause_condition.notify_one();
        }
        this_thread::sleep_for(config_probe_frequency);
      }
    } catch (curlpp::LogicError &e) {
      cerr << e.what() << endl;
    } catch (curlpp::RuntimeError &e) {
      cerr << e.what() << endl;
    }
  }
}
