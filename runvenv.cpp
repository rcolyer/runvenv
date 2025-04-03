#include "RC/RC.h"
#include <limits>
#include <stdlib.h>

#ifdef WIN32
#include <process.h>
#define execv _execv
#else
#include <unistd.h>
#endif


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
  RC::RStr runbase = RC::File::Basename(args[0]);
  size_t args_off = 1;
#ifdef WIN32
  RC::RStr target_exec = "python.exe";
#else
  RC::RStr target_exec = "python";
#endif
  bool print_only = false;

  if (runbase == "runpip") {
    if (args.size() < 2) {
      std::cerr << args[0]
        << " <environment_name> [pip_arguments] ..." << std::endl;
      return -1;
    }
#ifdef WIN32
    target_exec = "pip.exe";
#else
    target_exec = "pip";
#endif
  }
  else if (runbase == "runipyth") {
    if (args.size() < 2) {
      std::cerr << args[0]
        << " <environment_name> [ipython_arguments] ..." << std::endl;
      return -1;
    }
    target_exec = "ipython";
  }
  else {
    if (args.size() < 2 || args[1].length() < 1 ||
      (args[1][0] == '-' && args.size() < 3)) {
      std::cerr << "Run python:" << std::endl;
      std::cerr << args[0]
        << " <environment_name> <script.py> [script_arguments] ..." << std::endl;
      std::cerr << args[0]
        << " <environment_name> [python_arguments] ..." << std::endl;
      std::cerr << "Run pip:" << std::endl;
      std::cerr << args[0]
        << " -p <environment_name> [pip_arguments] ..." << std::endl;
      std::cerr << "Run ipython (if installed):" << std::endl;
      std::cerr << args[0]
        << " -i <environment_name> [ipython_arguments] ..." << std::endl;
      std::cerr << "Print the pathname of which python executable corresponds to the environment:" << std::endl;
      std::cerr << args[0] << " -w <environment_name>" << std::endl;
      return -1;
    }

    if (args[1] == "-p") {
#ifdef WIN32
      target_exec = "pip.exe";
#else
      target_exec = "pip";
#endif
      args_off = 2;
    }
    else if (args[1] == "-i") {
      target_exec = "ipython";
      args_off = 2;
    }
    else if (args[1] == "-w") {
      print_only = true;
      args_off = 2;
    }
  }

  RC::RStr env_name = args[args_off];
#ifdef WIN32
  RC::Data1D<RC::RStr> bin_dirs{"/bin/", "/Scripts"};
#else
  RC::Data1D<RC::RStr> bin_dirs{"/bin/"};
#endif
  RC::RStr py_file;
  RC::RStr py_file_dir;
  RC::Data1D<RC::RStr> py_args;
  if (args.size() > args_off+1) {
    if (args[args_off+1].length()>0 && args[args_off+1][0] != '-' && RC::File::Exists(args[args_off+1])) {
      py_file = args[args_off+1];
      RC::RStr py_file_real = py_file;
      RC::Data1D<char> resolved(PATH_MAX);
      auto res = realpath(py_file.c_str(), resolved.Raw());
      if (res != NULL) {
        py_file_real = resolved.Raw();
      }
      py_file_dir = RC::File::Dirname(py_file_real).Chomp(RC::File::divider);
    }
    else {
      py_args += args[args_off+1];
    }
  }
  if (args.size() > args_off+2) {
    py_args += args.Copy(args_off+2);
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
    bool found = false;
    for (auto bin_dir: bin_dirs) {
      RC::RStr py_try = path + "/" + env_name + bin_dir + target_exec;
      if (RC::File::Exists(py_try)) {
        py_exec = py_try;
        found = true;
        break;
      }
    }
    if (found) { break; }
  }

  if (py_exec.empty()) {
    std::cerr << "Could not find " << env_name << bin_dirs[0] << target_exec
      << " in:\n  ";
    std::cerr << RC::RStr::Join(path_list, "/\n  ") << "/" << std::endl;
    return -1;
  }

  if (print_only) {
    std::cout << py_exec << std::endl;
    return 0;
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

