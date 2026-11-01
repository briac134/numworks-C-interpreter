#include <cstdlib>

extern "C" int _getpid() { return 1; }

extern "C" int _kill(int, int) { return -1; }

extern "C" int _link(const char *, const char *) { return -1; }

extern "C" int _unlink(const char *) { return -1; }

extern "C" void _exit(int status) {
  std::abort();
  (void)status;
  while (true) {
  }
}
