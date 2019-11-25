#include <variti/util/config.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <stdexcept>
#include <sstream>

namespace variti { namespace util {

namespace {

struct config_setting_hook
{
  bool visited{false};
};

config_setting_hook* hook(config_setting_t* h)
{
  return reinterpret_cast<config_setting_hook*>(config_setting_get_hook(h));
}

void print_name(std::ostream& os, config_setting_t* h)
{
  if (!config_setting_name(h)) {
    if (!config_setting_is_root(h))
      os << config_setting_index(h);
    else
      os << "root";
  } else
    os << config_setting_name(h);
}

void print_path(std::ostream& os, config_setting_t* h)
{
  if (!config_setting_is_root(h)) {
    print_path(os, config_setting_parent(h));
    os << ".";
  }
  print_name(os, h);
}

void visit_up(config_setting_t* h)
{
  for (; !config_setting_is_root(h) && !hook(h)->visited; h = config_setting_parent(h))
    hook(h)->visited = true;
}

template <typename F>
void for_each(const config_setting& st, F f)
{
  if (st.size())
    for (std::size_t i = 0; i < st.size(); ++i)
      for_each(st.lookup(i), f);
  else
    f(st);
}

}

void throw_bad_value(const config_setting& st)
{
  std::ostringstream oss;
  oss << "config bad value: " << st.path() << " at " << st.filename() << ":" << st.fileline();
  throw std::runtime_error(oss.str());
}

void throw_not_found(const config_setting& st)
{
  std::ostringstream oss;
  oss << "config not found: " << st.path() << " at " << st.filename() << ":" << st.fileline();
  throw std::runtime_error(oss.str());
}

void throw_not_found(const config_setting& st, const std::string& name)
{
  std::ostringstream oss;
  oss << "config not found: " << st.path() << "." << name << " at " << st.filename() << ":" << st.fileline();
  throw std::runtime_error(oss.str());
}

void throw_not_found(const config_setting& st, std::size_t indx)
{
  std::ostringstream oss;
  oss << "config not found: " << st.path() << "." << indx << " at " << st.filename() << ":" << st.fileline();
  throw std::runtime_error(oss.str());
}

config_setting::config_setting(config_setting_t* h, bool visit)
  : h(h)
{
  assert(h);
  if (!config_setting_get_hook(h))
    config_setting_set_hook(h, new config_setting_hook());
  if (visit)
    visit_up(h);
}

config_setting::~config_setting()
{
  h = nullptr;
}

bool config_setting::to_bool() const
{
  if (!is_bool())
    throw_bad_value(*this);
  return config_setting_get_bool(h);
}

double config_setting::to_double() const
{
  if (!is_double())
    throw_bad_value(*this);
  return config_setting_get_float(h);
}

std::int32_t config_setting::to_int32() const
{
  if (!is_int32())
    throw_bad_value(*this);
  return config_setting_get_int(h);
}

std::uint32_t config_setting::to_uint32() const
{
  if (!is_int32())
    throw_bad_value(*this);
  auto value = config_setting_get_int(h);
  if (value < 0)
    throw_bad_value(*this);
  return value;
}

std::int64_t config_setting::to_int64() const
{
  if (!is_int64())
    throw_bad_value(*this);
  return config_setting_get_int64(h);
}

std::uint64_t config_setting::to_uint64() const
{
  if (!is_int64())
    throw_bad_value(*this);
  auto value = config_setting_get_int64(h);
  if (value < 0)
    throw_bad_value(*this);
  return value;
}

std::string config_setting::to_string() const
{
  if (!is_string())
    throw_bad_value(*this);
  return config_setting_get_string(h) ? config_setting_get_string(h) : "";
}

bool config_setting::is_bool() const
{
  return config_setting_type(h) == CONFIG_TYPE_BOOL;
}

bool config_setting::is_double() const
{
  return config_setting_type(h) == CONFIG_TYPE_FLOAT;
}

bool config_setting::is_int32() const
{
  return config_setting_type(h) == CONFIG_TYPE_INT;
}

bool config_setting::is_int64() const
{
  return config_setting_type(h) == CONFIG_TYPE_INT64;
}

bool config_setting::is_string() const
{
  return config_setting_type(h) == CONFIG_TYPE_STRING;
}

bool config_setting::is_group() const
{
  return config_setting_type(h) == CONFIG_TYPE_GROUP;
}

bool config_setting::is_array() const
{
  return config_setting_type(h) == CONFIG_TYPE_ARRAY;
}

bool config_setting::is_list() const
{
  return config_setting_type(h) == CONFIG_TYPE_LIST;
}

bool config_setting::is_scalar() const
{
  return is_bool() || is_int32() || is_int64() || is_double() || is_string();
}

bool config_setting::is_root() const
{
  return config_setting_is_root(h);
}

std::string config_setting::path() const
{
  std::ostringstream oss;
  print_path(oss, h);
  return oss.str();
}

std::size_t config_setting::size() const
{
  return config_setting_length(h);
}

config_setting config_setting::parent() const
{
  return config_setting(config_setting_parent(h));
}

bool config_setting::exists(const std::string& name) const
{
  if (!is_group())
    return false;
  return config_setting_get_member(h, name.c_str());
}

config_setting config_setting::lookup(const std::string& name, bool visit) const
{
  assert(is_group());
  auto p = config_setting_get_member(h, name.c_str());
  if (!p)
    throw_not_found(*this);
  return config_setting(p, visit);
}

config_setting config_setting::lookup(std::size_t indx, bool visit) const
{
  assert(is_group() || is_array() || is_list());
  auto p = config_setting_get_elem(h, indx);
  if (!p)
    throw_not_found(*this);
  return config_setting(p, visit);
}

config_setting config_setting::operator[](const std::string& name) const
{
  return lookup(name, true);
}

config_setting config_setting::operator[](std::size_t indx) const
{
  return lookup(indx, true);
}

std::string config_setting::filename() const
{
  if (h->config->filenames && h->config->filenames[0])
    return h->config->filenames[0];
  return "";
}

std::size_t config_setting::fileline() const
{
  return config_setting_source_line(h);
}

bool config_setting::visited() const
{
  return boost::algorithm::starts_with(path(), "root.version") || hook(h)->visited;
}

std::ostream& operator<<(std::ostream& os, const config_setting& st)
{
  for_each(st,
    [&os](const config_setting& st) {
      print_path(os, st.h);
      os << "\n";
    });
  return os;
}

config::config(config_notify n)
  : n(n)
{
  h = (config_t*)malloc(sizeof(config_t));
  config_init(h);
  config_set_destructor(h, [](void* p) { delete reinterpret_cast<config_setting_hook*>(p); });
}

config::~config()
{
  if (n)
    for_each(root(), n);
  config_destroy(h);
  free(h);
}

void config::load(const std::string& filename)
{
  if (!config_read_file(h, filename.c_str()))
    throw std::runtime_error(std::string("config read file error: ") + filename);
}

config_setting config::root() const
{
  return config_setting(config_root_setting(h));
}

std::ostream& operator<<(std::ostream& os, const config& conf)
{
  os << conf.root();
  return os;
}

}}
