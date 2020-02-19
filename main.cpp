#include "./parser.hpp"
#include "./utils.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <curlpp/Easy.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>
#include <vector>

using json = nlohmann::json;
using namespace std;
using namespace rpa;

// static variables
static const char *config_url =
    "http://my-json-server.typicode.com/skatsev/testproject/posts/2";
static constexpr chrono::seconds probe_frequency = 5s;
static constexpr chrono::seconds config_probe_frequency = 3s;

void utilizationProber(atomic_flag &app_list_lock,
                       const vector<string> &app_list, mutex &pause_mutex,
                       condition_variable &pause_condition) {
  ps_parser parser;
  try {
    while (true) {
      while (!app_list_lock.test_and_set(memory_order_acquire)) // spinlock
        ;
      if (app_list.size() > 0) {
        parser.setSource(popen("ps aux", "r"));
        while (parser) {
          info i = parser.line();
          for (const string &command : app_list) {
            if (i.command == command)
              std::cout << i.command << " " << i.pid << " " << i.cpu << " "
                        << i.mem << std::endl;
          }
        }
        app_list_lock.clear(memory_order_release); // end critical section
        this_thread::sleep_for(probe_frequency);
      } else {
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

int main() {
  mutex pause_mutex;
  condition_variable pause_condition;
  atomic_flag app_list_lock(false);
  vector<string> app_list;
  thread prober(utilizationProber, ref(app_list_lock), ref(app_list),
                ref(pause_mutex), ref(pause_condition));

  json j;
  while (true) {
    try {
      curlpp::Cleanup cleaner;
      ostringstream s;
      s << curlpp::options::Url(config_url);
      j = json::parse(s.str());
      // conversion to avoid locks
      vector<string> new_app_list = j["applications"].get<vector<string>>();
      for (string &i : new_app_list) {
        rpa::to_lower(i);
        cout << i;
      }
      cout << endl;
      size_t n_apps = app_list.size();
      if (!(n_apps == 0 && new_app_list.size() == 0)) {
        while (!app_list_lock.test_and_set(memory_order_acquire)) // spinlock
          ;
        app_list = new_app_list;
        app_list_lock.clear(memory_order_release); // end critical section
        if (n_apps == 0) {
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
