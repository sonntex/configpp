#pragma once

#include <functional>
#include <string>
#include <cstdint>

#include <libconfig.h>

namespace variti { namespace util {

struct config;
struct config_setting;

using config_notify = std::function<void(const config_setting&)>;

void throw_bad_value(const config_setting& st);
void throw_not_found(const config_setting& st);
void throw_not_found(const config_setting& st, const std::string& name);
void throw_not_found(const config_setting& st, std::size_t indx);

struct config_setting
{
  config_setting(config_setting_t* h, bool visit = false);
 ~config_setting();

  config_setting(const config_setting&) = default;
  config_setting& operator=(const config_setting&) = default;
  config_setting(config_setting&&) = default;
  config_setting& operator=(config_setting&&) = default;

  bool to_bool() const;
  double to_double() const;
  std::int32_t to_int32() const;
  std::uint32_t to_uint32() const;
  std::int64_t to_int64() const;
  std::uint64_t to_uint64() const;
  std::string to_string() const;

  bool is_bool() const;
  bool is_double() const;
  bool is_int32() const;
  bool is_int64() const;
  bool is_string() const;
  bool is_group() const;
  bool is_array() const;
  bool is_list() const;
  bool is_scalar() const;
  bool is_root() const;

  std::string path() const;

  std::size_t size() const;

  config_setting parent() const;

  bool exists(const std::string& name) const;

  config_setting lookup(const std::string& name, bool visit = false) const;
  config_setting lookup(std::size_t indx, bool visit = false) const;

  config_setting operator[](const std::string& name) const;
  config_setting operator[](std::size_t indx) const;

  std::string filename() const;
  std::size_t fileline() const;

  bool visited() const;

  config_setting_t* h;
};

std::ostream& operator<<(std::ostream& os, const config_setting& st);

struct config
{
  config(config_notify n = nullptr);
 ~config();

  config(const config&) = delete;
  config& operator=(const config&) = delete;
  config(config&&) = delete;
  config& operator=(config&&) = delete;

  void load(const std::string& filename);

  config_setting root() const;

  config_notify n;
  config_t* h;
};

std::ostream& operator<<(std::ostream& os, const config& conf);

template <typename T>
void lookup(const config_setting& st, const std::string& name, T& dst, bool required = true)
{
  if (st.exists(name))
    dst = static_cast<typename std::remove_reference<T>::type>(st.lookup(name, true));
  else if (required)
    throw_not_found(st, name);
}

template <typename T>
void lookup_def(const config_setting& st, const std::string& name, T& dst)
{
  lookup(st, name, dst, false);
}

template <typename T, typename D>
void lookup_def(const config_setting& st, const std::string& name, T& dst, const D& def)
{
  try {
    lookup(st, name, dst);
  } catch (...) {
    dst = def;
  }
}

template <typename T>
void lookup(const config_setting& st, std::size_t indx, T& dst, bool required = true)
{
  if (indx < st.size())
    dst = static_cast<typename std::remove_reference<T>::type>(st.lookup(indx, true));
  else if (required)
    throw_not_found(st, indx);
}

template <typename T>
void lookup_def(const config_setting& st, std::size_t indx, T& dst)
{
  lookup(st, indx, dst, false);
}

template <typename T, typename D>
void lookup_def(const config_setting& st, std::size_t indx, T& dst, const D& def)
{
  try {
    lookup(st, indx, dst);
  } catch (...) {
    dst = def;
  }
}

}}
