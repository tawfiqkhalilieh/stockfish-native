#include <Python.h>
#include <string>
#include <unistd.h>
#include <vector>

static PyObject *run_stockfish(PyObject *self, PyObject *args) {

  std::string value;

  const char *fen;
  int depth;
  int multiv;

  if (!PyArg_ParseTuple(args, "sii", &fen, &depth, &multiv)) {
    return NULL; // Python exception is automatically set
  }

  // the code from here assumes that I'm gonna deal with first arg as fen, and
  // the other as depth ( str, int, int ), not gonna handle edge cases here, I
  // assume the user of such code should have the cognitive ability and read :D

  /*
   * connect to stockfish using pipes and send the following commands:
   *
   * uciok ( v )
   * readyok ( v )
   * setoption name MultiPV value ( arg[3] ) (v)
   * position set (fen) (v)
   * go depth 10
   *
   * and then do some string manipulation to get the best move and the
   * evaluation score from the output of stockfish
   *
   * and then return the list of best moves
   *
   * can also allow the user to send custom depth and multiPV values as
   * arguments to the function
   * */



  int in[2], out[2];
  if (pipe(in) || pipe(out)) {
    return PyUnicode_FromString("Error creating pipes");
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

    execl("Stockfish/src/stockfish", "stockfish", NULL);
    _exit(1); // Properly exit child if exec fails
  }

  // Parent process
  close(in[0]);  // Close read end of input pipe
  close(out[1]); // Close write end of output pipe

  FILE *output = fdopen(out[0], "r");

  std::string cmd = "uci\nisready\nsetoption name MultiPV value " +
                    std::to_string(multiv) + "\n";
  dprintf(in[1], "%s", cmd.c_str());

  char buf[2048];
  while (fgets(buf, sizeof(buf), output)) {
    std::string line(buf);
    if (line.find("readyok") != std::string::npos)
      break;
  }

  char cmd2[200];
  snprintf(cmd2, sizeof(cmd2), "position fen %s\ngo depth %d\n", fen, depth);

  dprintf(in[1], "%s", cmd2);

  std::vector<std::string> found_moves(multiv, "");

  while (fgets(buf, sizeof(buf), output)) {
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

// Method table
static PyMethodDef Methods[] = {{"run_stockfish", run_stockfish, METH_VARARGS,
                                 "Runs C-based Stockfish engine in Python!!!"},
                                {nullptr, nullptr, 0, nullptr}};

// Module definition
static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT, "run_stockfish",
    "Minimal C++ extension to run Stockfish engine in Python", -1, Methods};

// Module init
extern "C" PyMODINIT_FUNC PyInit_run_stockfish(void) {
  return PyModule_Create(&module);
}
