#include "RC/RC.h"
#include <limits>
#include <stdlib.h>
#include <unistd.h>


class CStrArr {
  protected:
  char** arr;
  size_t len;

  void CopyIn(const CStrArr& other) {
    len = other.len;
    if (len == 0) {
      arr = NULL;
    }
    else {
      arr = new char*[len+1];
      for (size_t i=0; i<len; i++) {
        size_t slen = strlen(other.arr[i]);
        arr[i] = new char[slen+1];
        for (size_t c=0; c<slen; c++) {
          arr[i][c] = other.arr[i][c];
        }
        arr[i][slen] = '\0';
      }
      arr[len] = NULL;  // NULL terminate.
    }
  }

  void CopyIn(const RC::Data1D<RC::RStr>& set) {
    len = set.size();

    arr = new char*[len+1];
    for (size_t i=0; i<len; i++) {
      arr[i] = new char[set[i].size()+1];
      for (size_t c=0; c<set[i].size(); c++) {
        arr[i][c] = set[i][c];
      }
      arr[i][set[i].size()] = '\0';
    }
    arr[len] = NULL;  // NULL terminate.
  }
  void MoveIn(CStrArr&& other) {
    arr = other.arr;
    len = other.len;
    other.arr = NULL;
    other.len = 0;
  }

  public:
  CStrArr() : arr(0), len(0) { }

  CStrArr(const RC::Data1D<RC::RStr>& set) {
    CopyIn(set);
  }

  void Delete() {
    if (arr) {
      for (size_t i=0; i<len; i++) {
        delete[] (arr[i]);
      }
      delete[] (arr);
    }
    len = 0;
  }

  ~CStrArr() {
    Delete();
  }

  CStrArr(const CStrArr& other) {
    CopyIn(other);
  }

  CStrArr(CStrArr&& other) {
    MoveIn(std::move(other));
  }

  CStrArr& operator=(const CStrArr& other) {
    Delete();
    CopyIn(other);
    return *this;
  }

  CStrArr& operator=(const RC::Data1D<RC::RStr>& set) {
    Delete();
    CopyIn(set);
    return *this;
  }

  CStrArr& operator=(CStrArr&& other) {
    Delete();
    MoveIn(std::move(other));
    return *this;
  }

  char** Get() const { return arr; }
};


RC_MAIN {
  if (args.size() < 2) {
    std::cerr << args[0]
      << " <environment_name> <script.py> [script_arguments] ..." << std::endl;
    std::cerr << args[0]
      << " <environment_name> [python_arguments] ..." << std::endl;
    return -1;
  }

  RC::RStr env_name = args[1];
  RC::RStr py_file;
  RC::RStr py_file_dir;
  RC::Data1D<RC::RStr> py_args;
  if (args.size() > 2) {
    if (args[2].length()>0 && args[2][0] != '-' && RC::File::Exists(args[2])) {
      py_file = args[2];
      RC::RStr py_file_real = py_file;
      RC::Data1D<char> resolved(PATH_MAX);
      auto res = realpath(py_file.c_str(), resolved.Raw());
      if (res != NULL) {
        py_file_real = resolved.Raw();
      }
      py_file_dir = RC::File::Dirname(py_file_real).Chomp(RC::File::divider);
    }
    else {
      py_args += args[2];
    }
  }
  if (args.size() > 3) {
    py_args += args.Copy(3);
  }

  auto home_env = getenv("HOME");
  if (home_env == NULL) {
    std::cerr  << "HOME not found in environment!" << std::endl;
    return -1;
  }
  RC::RStr home(home_env);

  RC::Data1D<RC::RStr> path_list;
  if (! py_file_dir.empty()) {
    path_list += py_file_dir;
  }
  path_list += home + "/.venv";
  path_list += "/usr/local/share/venv";

  RC::RStr conda_env_txt = home + "/.conda/environments.txt";
  if (RC::File::Exists(conda_env_txt)) {
    RC::FileRead fr(conda_env_txt);
    RC::Data1D<RC::RStr> lines;
    fr.ReadAllLines(lines);
    for (auto line: lines) {
      line.Chomp();
      if (line.length() > env_name.length() &&
          line.substr(line.length()-env_name.length()-1) == "/" + env_name) {
        path_list += line.substr(0, line.length()-env_name.length()-1);
      }
    }
  }
  RC::RStr conda_rc = home + "/.condarc";
  if (RC::File::Exists(conda_rc)) {
    RC::FileRead fr(conda_rc);
    RC::Data1D<RC::RStr> lines;
    fr.ReadAllLines(lines);
    bool envs = false;
    for (auto line: lines) {
      line.Trim();
      if (!envs) {
        if (line == "envs_dirs:") {
          envs = true;
        }
        continue;
      }
      if (line.empty()) {
        continue;
      }
      if (line[line.length()-1] == ':') {
        break;
      }
      auto split = line.SplitFirst("-");
      split[1].Trim();
      if (! split[1].empty()) {
        path_list += split[1];
      }
    }
  }
  else {
    RC::RStr conda_default_user = home + "/.conda/envs";
    if (RC::File::Exists(conda_default_user)) {
      path_list += conda_default_user;
    }
    auto conda_env = getenv("CONDA_EXE");
    if (conda_env != NULL) {
      RC::RStr conda_default_sys(conda_env);
      conda_default_sys.Subst("bin/conda$", "env");
      if (RC::File::Exists(conda_default_sys)) {
        path_list += conda_default_sys;
      }
    }
  }

  RC::RStr py_exec;
  for (auto path: path_list) {
    RC::RStr py_try = path + "/" + env_name + "/bin/python";
    if (RC::File::Exists(py_try)) {
      py_exec = py_try;
      break;
    }
  }

  if (py_exec.empty()) {
    std::cerr << "Could not find " << env_name << "/bin/python in:\n  ";
    std::cerr << RC::RStr::Join(path_list, "/\n  ") << "/" << std::endl;
    return -1;
  }

  RC::Data1D<RC::RStr> cmd{py_exec};
  if (! py_file.empty()) {
    cmd += py_file;
  }
  cmd += py_args;
  CStrArr exec_args(cmd);

  if (execv(cmd[0].c_str(), exec_args.Get()) != 0) {
    std::cerr << RC::RStr::Join(cmd, " ") << std::endl;
    std::cerr << errno << ", " << RC::RStr::Errno() << std::endl;
    return -1;
  }

  return 0;
}

