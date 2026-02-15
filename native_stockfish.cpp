#include <Python.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <signal.h>
#include <sys/wait.h>

typedef struct {
  // might find some use for this in the future, maybe in handling exceptions
  PyObject_HEAD int engine_running;
  const char *stockfish_path;
  pid_t stockfish_process;
  int (*dprintf)(int, const char *format, ...);
  int write_fd;
  FILE *output;
  int in[2];
  int out[2];
} StockfishClient;

static PyObject *Stockfish_start(StockfishClient *self, PyObject *args) {
  self->engine_running = 1;
  Py_RETURN_NONE;
}

static PyObject *Stockfish_stop(StockfishClient *self, PyObject *args) {
  self->engine_running = 0;
  Py_RETURN_NONE;
}

static PyObject *Stockfish_status(StockfishClient *self, PyObject *args) {
  if (self->engine_running)
    return PyUnicode_FromString("running");
  else
    return PyUnicode_FromString("stopped");
}

static int Stockfish_init(StockfishClient *self, PyObject *args,
                          PyObject *kwds) {

  const char *stockfish_path_param;

  if (!PyArg_ParseTuple(args, "s", &stockfish_path_param)) {
    return -1; // Python exception is automatically set
  }

  int in[2], out[2];
  if (pipe(in) || pipe(out)) {
    return 0;
  }

  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    dup2(in[0], STDIN_FILENO);
    dup2(out[1], STDOUT_FILENO);
    // Close unused pipe ends
    close(in[0]);
    close(in[1]);
    close(out[0]);
    close(out[1]);

    execl(stockfish_path_param, "stockfish", NULL);
    _exit(1); // Properly exit child if exec fails
  }

  // Parent process
  close(in[0]);  // Close read end of input pipe
  close(out[1]); // Close write end of output pipe

  self->dprintf = dprintf;
  self->write_fd = in[1];
  self->output = fdopen(out[0], "r");
  self->stockfish_process = pid;
  self->engine_running = 0;
  self->stockfish_path = stockfish_path_param;
  self->in[0] = in[0];
  self->in[1] = in[1];
  self->out[0] = out[0];
  self->out[1] = out[1];
  return 0;
}

static void Stockfish_dealloc(StockfishClient *self) {
  if (self->output) {
    fclose(self->output);
    self->output = NULL;
  }
  if (self->write_fd > 0) {
    close(self->write_fd);
    self->write_fd = -1;
  }
  
  if (self->stockfish_process > 0) {
      kill(self->stockfish_process, SIGTERM);
      waitpid(self->stockfish_process, NULL, 0);
      self->stockfish_process = 0;
  }

  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *top_moves(StockfishClient *self, PyObject *args) {
  const char *fen;
  int depth;
  int multiv;

  if (!PyArg_ParseTuple(args, "sii", &fen, &depth, &multiv)) {
    return NULL; // Python exception is automatically set
  }

  std::string cmd = "uci\nisready\nsetoption name MultiPV value " +
                    std::to_string(multiv) + "\n";

  dprintf(self->in[1], "%s", cmd.c_str());

  char buf[2048];
  while (fgets(buf, sizeof(buf), self->output)) {
    std::string line(buf);
    if (line.find("readyok") != std::string::npos)
      break;
  }

  char cmd2[200];
  snprintf(cmd2, sizeof(cmd2), "position fen %s\ngo depth %d\n", fen, depth);

  dprintf(self->in[1], "%s", cmd2);

  std::vector<std::string> found_moves(multiv, "");

  while (fgets(buf, sizeof(buf), self->output)) {
    std::string line(buf);
    std::string search_str = "info depth " + std::to_string(depth);
    if (line.find(search_str) != std::string::npos) {
      int pv_index = 1;
      size_t multipv_pos = line.find(" multipv ");
      if (multipv_pos != std::string::npos) {
        try {
          pv_index = std::stoi(line.substr(multipv_pos + 9));
        } catch (...) {
          pv_index = 1;
        }
      }

      if (pv_index >= 1 && pv_index <= multiv) {
        size_t pv_pos = line.find(" pv ");
        if (pv_pos != std::string::npos) {
          size_t move_start = pv_pos + 4; // Length of " pv " is 4
          size_t move_end = line.find(" ", move_start);
          std::string move_str;
          if (move_end != std::string::npos) {
            move_str = line.substr(move_start, move_end - move_start);
          } else {
            move_str = line.substr(move_start);
            if (!move_str.empty() && move_str.back() == '\n') {
              move_str.pop_back();
            }
          }
          found_moves[pv_index - 1] = move_str;
        }
      }
    }

    if (line.find("bestmove") != std::string::npos)
      break;
  }

  PyObject *result_tuple = PyTuple_New(multiv);
  for (int i = 0; i < multiv; ++i) {
    PyObject *str_obj = PyUnicode_FromString(found_moves[i].c_str());
    PyTuple_SetItem(result_tuple, i, str_obj);
  }

  return result_tuple;
}

static PyMethodDef Stockfish_methods[] = {
    {"start", (PyCFunction)Stockfish_start, METH_NOARGS, "Start engine"},
    {"stop", (PyCFunction)Stockfish_stop, METH_NOARGS, "Stop engine"},
    {"status", (PyCFunction)Stockfish_status, METH_NOARGS, "Get status"},
    {"top_moves", (PyCFunction)top_moves, METH_VARARGS, "Get top moves"},
    {NULL}};

static PyTypeObject StockfishType = {PyVarObject_HEAD_INIT(NULL, 0)};

static int init_stockfish_type() {
  StockfishType.tp_name = "native_stockfish.StockfishClient";
  StockfishType.tp_basicsize = sizeof(StockfishClient);
  StockfishType.tp_flags = Py_TPFLAGS_DEFAULT;
  StockfishType.tp_new = PyType_GenericNew;
  StockfishType.tp_init = (initproc)Stockfish_init;
  StockfishType.tp_dealloc = (destructor)Stockfish_dealloc;
  StockfishType.tp_methods = Stockfish_methods;

  return PyType_Ready(&StockfishType);
}

static PyModuleDef moduledef = {PyModuleDef_HEAD_INIT, "native_stockfish",
                                "Stockfish C++ client", -1, NULL};

PyMODINIT_FUNC PyInit_native_stockfish(void) {
  if (init_stockfish_type() < 0)
    return NULL;

  PyObject *m = PyModule_Create(&moduledef);
  if (!m)
    return NULL;

  Py_INCREF(&StockfishType);
  PyModule_AddObject(m, "StockfishClient", (PyObject *)&StockfishType);

  return m;
}
