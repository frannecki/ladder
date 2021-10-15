#include <iostream>
#include <vector>

#include <ArgumentParser.h>

namespace ladder {

static bool IsOption(const std::string& str) {
  return str.size() > 2 && str.substr(0, 2) == "--";
}

IArgumentParser::~IArgumentParser() { ; }

bool IArgumentParser::Init(int argc, char** argv) {
  InitOptions();

  std::vector<std::string> args;
  std::vector<bool> is_options;
  for (int i = 0; i < argc; ++i) {
    std::string cur_str = std::string(argv[i]);
    args.push_back(cur_str);
    is_options.push_back(IsOption(cur_str));
  }
  for (size_t i = 0; i < args.size(); ++i) {
    if (is_options[i]) {
      args[i] = args[i].substr(2, args[i].size() - 2);
      if (fields_options_.find(args[i]) != fields_options_.end()) {
        if (i + 1 == args.size() || is_options[i + 1]) {
          std::cerr << "[ArgumentParser] Option " << args[i]
                    << " should be followed by a value" << std::endl;
          return false;
        } else {
          options_.insert(
              std::pair<std::string, std::string>(args[i], args[i + 1]));
          ++i;
        }
      } else if (fields_enabled_.find(args[i]) != fields_enabled_.end()) {
        enabled_.insert(std::pair<std::string, bool>(args[i], true));
      } else {
        std::cerr << "[ArgumentParser] No such option: " << args[i]
                  << std::endl;
        return false;
      }
    }
  }

  return CheckRequired();
}

bool IArgumentParser::GetStringArg(const std::string& key, std::string& val) {
  auto iter = options_.find(key);
  if (iter == options_.end()) {
    return false;
  }
  val = iter->second;
  return true;
}

bool IArgumentParser::GetBoolArg(const std::string& key) {
  auto iter = enabled_.find(key);
  if (iter == enabled_.end()) {
    return false;
  }
  return iter->second;
}

bool IArgumentParser::CheckRequired() {
  for (auto iter = fields_options_.begin(); iter != fields_options_.end();
       ++iter) {
    if (iter->second && options_.find(iter->first) == options_.end()) {
      std::cerr << "[ArgumentParser] String Option: " << iter->first
                << " is required." << std::endl;
      return false;
    }
  }

  for (auto iter = fields_enabled_.begin(); iter != fields_enabled_.end();
       ++iter) {
    if (iter->second && enabled_.find(iter->first) == enabled_.end()) {
      std::cerr << "[ArgumentParser] Boolean Option: " << iter->first
                << " is required." << std::endl;
      return false;
    }
  }

  return true;
}

}  // namespace ladder
