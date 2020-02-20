#include "./parser.hpp"
#include "./arguments.hpp"
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
#include <fstream>

using json = nlohmann::json;
using namespace std;
using namespace rpa;

void utilizationProber(atomic_flag &app_list_lock,
                       const vector<string> &app_list, mutex &pause_mutex,
                       condition_variable &pause_condition, const chrono::seconds &probe_frequency, ostream &out)
{
  ps_parser parser;
  try
  {
    while (true)
    {
      while (!app_list_lock.test_and_set(memory_order_acquire)) // spinlock
        ;
      if (app_list.size() > 0)
      {
        parser.setSource(popen("ps aux", "r"));
        while (parser)
        {
          info i = parser.line();
          for (const string &command : app_list)
          {
            if (end_with(i.command, command))
              out << i.command << " " << i.pid << " " << i.cpu << " " << i.mem << endl;
          }
        }
        app_list_lock.clear(memory_order_release); // end critical section
        this_thread::sleep_for(probe_frequency);
      }
      else
      {
        app_list_lock.clear(memory_order_release); // end critical section
        unique_lock<mutex> lock(pause_mutex);
        pause_condition.wait(lock);
      }
    }
  }
  catch (std::system_error &err)
  {
    app_list_lock.clear(); // ensure clear of the lock
    cerr << err.what();
  }
}

int main(int argc, char **argv)
{
  // parameters parsing
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  const chrono::seconds probe_frequency(FLAGS_probe_frequency);
  const chrono::seconds config_probe_frequency(FLAGS_probe_frequency);
  ostream *out;
  ofstream file_out;
  if (FLAGS_output_file == "")
  {
    out = &cout;
  }
  else
  {
    file_out.open(FLAGS_output_file);
    if (!file_out)
    {
      cerr << "error on output file " << FLAGS_output_file << endl;
      return 1;
    }
    out = &file_out;
  }

  vector<string> endpoints;
  if (FLAGS_config_file != "")
  {
    ifstream config_file(FLAGS_config_file);
    string s;
    while (config_file)
    {
      config_file >> s;
      if (s != "")
        endpoints.push_back(s);
    }
    if (endpoints.size() == 0)
    {
      cerr << "error on input file " << FLAGS_config_file << " missing endpoints" << endl;
      return 2;
    }
  }
  else
  {
    endpoints = {
        "http://my-json-server.typicode.com/Federico-Mulas/running-processes-analyzer/applications/1",
        "http://my-json-server.typicode.com/Federico-Mulas/running-processes-analyzer/applications/2",
        "http://my-json-server.typicode.com/Federico-Mulas/running-processes-analyzer/applications/3"};
  }

  //intial setup
  mutex pause_mutex;
  condition_variable pause_condition;
  atomic_flag app_list_lock(false);
  vector<string> app_list;
  thread prober(utilizationProber, ref(app_list_lock), ref(app_list),
                ref(pause_mutex), ref(pause_condition), ref(probe_frequency), ref(*out));

  json j;
  size_t i = 0;
  while (true)
  {
    const string &config_url = endpoints[i];
    i = (i + 1) % config_url.size();
    try
    {
      curlpp::Cleanup cleaner;
      ostringstream s;
      s << curlpp::options::Url(config_url);
      j = json::parse(s.str());
      // conversion to avoid locks
      vector<string> new_app_list = j["applications"].get<vector<string>>();
      for (string &i : new_app_list)
      {
        rpa::to_lower(i);
      }
      size_t n_apps = app_list.size();
      if (!(n_apps == 0 && new_app_list.size() == 0))
      {
        while (!app_list_lock.test_and_set(memory_order_acquire)) // spinlock
          ;
        app_list = new_app_list;
        app_list_lock.clear(memory_order_release); // end critical section
        if (n_apps == 0)
        {
          pause_condition.notify_one();
        }
        this_thread::sleep_for(config_probe_frequency);
      }
    }
    catch (curlpp::LogicError &e)
    {
      cerr << e.what() << endl;
    }
    catch (curlpp::RuntimeError &e)
    {
      cerr << e.what() << endl;
    }
  }
}
