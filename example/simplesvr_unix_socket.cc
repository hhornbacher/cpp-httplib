//
//  simplesvr.cc
//
//  Copyright (c) 2019 Yuji Hirose. All rights reserved.
//  MIT License
//

#include <cstdio>
#include <httplib.h>
#include <iostream>

#define SOCKET_FILE "./srvsocket"

using namespace httplib;
using namespace std;

string dump_headers(const Headers &headers) {
  string s;
  char buf[BUFSIZ];

  for (const auto &x : headers) {
    snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
    s += buf;
  }

  return s;
}

string dump_multipart_files(const MultipartFormDataMap &files) {
  string s;
  char buf[BUFSIZ];

  s += "--------------------------------\n";

  for (const auto &x : files) {
    const auto &name = x.first;
    const auto &file = x.second;

    snprintf(buf, sizeof(buf), "name: %s\n", name.c_str());
    s += buf;

    snprintf(buf, sizeof(buf), "filename: %s\n", file.filename.c_str());
    s += buf;

    snprintf(buf, sizeof(buf), "content type: %s\n", file.content_type.c_str());
    s += buf;

    snprintf(buf, sizeof(buf), "text length: %zu\n", file.content.size());
    s += buf;

    s += "----------------\n";
  }

  return s;
}

string log(const Request &req, const Response &res) {
  string s;
  char buf[BUFSIZ];

  s += "================================\n";

  snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
           req.version.c_str(), req.path.c_str());
  s += buf;

  string query;
  for (auto it = req.params.begin(); it != req.params.end(); ++it) {
    const auto &x = *it;
    snprintf(buf, sizeof(buf), "%c%s=%s",
             (it == req.params.begin()) ? '?' : '&', x.first.c_str(),
             x.second.c_str());
    query += buf;
  }
  snprintf(buf, sizeof(buf), "%s\n", query.c_str());
  s += buf;

  s += dump_headers(req.headers);
  s += dump_multipart_files(req.files);

  s += "--------------------------------\n";

  snprintf(buf, sizeof(buf), "%d\n", res.status);
  s += buf;
  s += dump_headers(res.headers);

  return s;
}

int main(int argc, const char **argv) {
  Server svr;

  svr.Get("/hi", [](const Request &req, Response &res) {
    res.set_content("Hello World", "text/plain");
  });

  svr.set_error_handler([](const Request & /*req*/, Response &res) {
    const char *fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
    char buf[BUFSIZ];
    snprintf(buf, sizeof(buf), fmt, res.status);
    res.set_content(buf, "text/html");
  });

  svr.set_logger(
      [](const Request &req, const Response &res) { cout << log(req, res); });

  cout << "The server started at unix socket " << SOCKET_FILE << "..." << endl;

  svr.listen_unix_socket(SOCKET_FILE);

  return 0;
}
