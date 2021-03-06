/**
 * Copyright European Organization for Nuclear Research (CERN)
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Authors:
 * - Mario Lassnig, <mario.lassnig@cern.ch>, 2012
 */

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <curl/curl.h>
#include <json/json.h>

#include "rucio_connect.h"

/* CURL callback */
size_t write_fp(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t actual_size = size * nmemb;
  mem_t *mem = (mem_t *)stream;

  mem->memory = (char *)realloc(mem->memory, mem->size + actual_size + 1);
  if (mem->memory == NULL) {
    std::cerr << "out of memory, cannot allocate " << mem->size + actual_size + 1 << " bytes";
    return 0;
  }

  memmove(&(mem->memory[mem->size]), ptr, actual_size);
  mem->size += actual_size;
  mem->memory[mem->size] = 0;

  return actual_size;
}

namespace Rucio {

RucioConnect::RucioConnect(std::string host, std::string port, std::string auth_token, std::string ca_cert) {

  full_host = "https://" + host + ":" + port;
  full_auth = std::string("Rucio-Auth-Token: ") + auth_token;

  headers = NULL;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_fp);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "plugin_rucio/0.1");
  curl_easy_setopt(curl, CURLOPT_CAINFO, ca_cert.c_str());

  headers = curl_slist_append(headers, full_auth.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
}

RucioConnect::~RucioConnect() {
  curl_easy_cleanup(curl);
  curl_global_cleanup();
}

json_object *RucioConnect::http_get_json(std::string url) {
  json_object *tmp_j;

  chunk.memory = (char *)malloc(1);
  chunk.size = 0;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_perform(curl);
  tmp_j = json_tokener_parse(chunk.memory);
  if (!tmp_j) {
    std::cerr << "cannot parse json: " << chunk.memory << std::endl;
  }
  free(chunk.memory);

  return tmp_j;
}

std::deque<std::string> RucioConnect::list_scopes() {
  std::deque<std::string> response;

  json_object *tmp_j = http_get_json(full_host + "/scopes/");
  for (i = 0; i < json_object_array_length(tmp_j); ++i) {
    response.push_back(json_object_get_string(json_object_array_get_idx(tmp_j, i)));
  }
  json_object_put(tmp_j);

  return response;
}

std::deque<did_t> RucioConnect::list_dids(std::string scope, std::string did) {
  std::deque<did_t> response;

  json_object *tmp_j;
  if (did.empty()) {
    tmp_j = http_get_json(full_host + "/dids/" + scope + "/");
  } else {
    tmp_j = http_get_json(full_host + "/dids/" + scope + "/" + did + "/dids");
  }

  did_t tmp_did;
  for (i = 0; i < json_object_array_length(tmp_j); ++i) {
    json_object *tmp_jj = json_object_array_get_idx(tmp_j, i);
    tmp_did.did = json_object_get_string(json_object_object_get(tmp_jj, "did"));
    tmp_did.scope = json_object_get_string(json_object_object_get(tmp_jj, "scope"));
    tmp_did.type = json_object_get_string(json_object_object_get(tmp_jj, "type"));
    json_object_put(tmp_jj);
    response.push_back(tmp_did);
  }

  return response;
}

std::deque<replica_t> RucioConnect::list_replicas(std::string scope, std::string did) {
  std::string url;
  http_get_json(url);

  std::deque<replica_t> dummy;
  return dummy;
}

did_t RucioConnect::get_did(std::string scope, std::string did) {
  json_object *tmp_j = http_get_json(full_host + "/dids/" + scope + "/" + did);

  did_t tmp_did;

  if (tmp_j != NULL) {
    tmp_did.did = json_object_get_string(json_object_object_get(tmp_j, "did"));
    tmp_did.scope = json_object_get_string(json_object_object_get(tmp_j, "scope"));
    tmp_did.type = json_object_get_string(json_object_object_get(tmp_j, "type"));
  }
  json_object_put(tmp_j);

  return tmp_did;
}
}
