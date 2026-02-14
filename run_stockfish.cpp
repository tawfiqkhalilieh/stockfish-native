#include <Python.h>
#include <iostream>
#include <string>
#include <unistd.h>

static PyObject* run_stockfish(PyObject* self, PyObject* args) {

    /* 
     * connect to stockfish using pipes and send the following commands:
     *
     * uciok
     * readyok
     * setoption name MultiPV value 3
     * position set (fen)
     * go depth 10
     *
     * and then do some string manipulation to get the best move and the evaluation score from the output of stockfish
     *
     * and then return the list of best moves
     *
     * can also allow the user to send custom depth and multiPV values as arguments to the function
     * */
    std::string stockfish_output = "";

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
        close(in[0]); close(in[1]);
        close(out[0]); close(out[1]);

        execl("Stockfish/src/stockfish", "stockfish", NULL);
        _exit(1);  // Properly exit child if exec fails
    }

    // Parent process
    close(in[0]);   // Close read end of input pipe
    close(out[1]);  // Close write end of output pipe

    FILE* output = fdopen(out[0], "r");

    dprintf(in[1], "uci\nisready\n");

    char buf[2048];
    while (fgets(buf, sizeof(buf), output)) {
        std::string line(buf);
        if (line.find("readyok") != std::string::npos) break;
    }

    dprintf(in[1], "position startpos moves e2e4\ngo depth 15\n");

    while (fgets(buf, sizeof(buf), output)) {
        std::string line(buf);
        stockfish_output += line + "\n";
        if (line.find("bestmove") != std::string::npos) break;
    }

    return PyUnicode_FromString(stockfish_output.c_str());
}

// Method table
static PyMethodDef Methods[] = {
    {"run_stockfish", run_stockfish, METH_VARARGS, "Runs C-based Stockfish engine in Python!!!"},
    {nullptr, nullptr, 0, nullptr}
};

// Module definition
static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    "run_stockfish",
    "Minimal C++ extension to run Stockfish engine in Python",
    -1,
    Methods
};

// Module init
extern "C" PyMODINIT_FUNC PyInit_run_stockfish(void) {
    return PyModule_Create(&module);
}

