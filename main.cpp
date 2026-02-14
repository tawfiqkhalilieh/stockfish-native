#include <iostream>
#include <string>
#include <unistd.h>

int main() {
    int in[2], out[2];
    if (pipe(in) || pipe(out)) return 1;

    if (!fork()) {
        dup2(in[0], 0); dup2(out[1], 1);
        for (int i = 0; i < 2; i++) { close(in[i]); close(out[i]); }
        execl("Stockfish/src/stockfish", "stockfish", NULL);
        return 1;
    }

    close(in[0]); close(out[1]);
    FILE* output = fdopen(out[0], "r");

    dprintf(in[1], "uci\nisready\n");

    char buf[2048];
    while (fgets(buf, sizeof(buf), output)) {
        if (std::string(buf).find("readyok") != std::string::npos) break;
    }

    dprintf(in[1], "position startpos moves e2e4\ngo depth 15\n");

    while (fgets(buf, sizeof(buf), output)) {
        std::cout << buf;
        if (std::string(buf).find("bestmove") != std::string::npos) break;
    }
}
