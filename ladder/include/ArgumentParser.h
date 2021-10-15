#ifndef LADDER_ARGUMENT_PARSER_H
#define LADDER_ARGUMENT_PARSER_H

#include <map>
#include <set>
#include <string>

namespace ladder {

class IArgumentParser {
 public:
  virtual ~IArgumentParser();
  bool Init(int argc, char** argv);
  bool GetBoolArg(const std::string& key);
  bool GetStringArg(const std::string& key, std::string& val);

 protected:
  bool CheckRequired();
  virtual void InitOptions() = 0;

  std::map<std::string, std::string> options_;
  std::map<std::string, bool> enabled_;
  std::map<std::string, bool> fields_options_;
  std::map<std::string, bool> fields_enabled_;
};

}  // namespace ladder

#endif
