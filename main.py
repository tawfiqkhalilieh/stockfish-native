import run_stockfish


out: tuple[int, ] = run_stockfish.run_stockfish("r1bqkbnr/ppp1pppp/2n5/3p4/3P4/5N2/PPP1PPPP/RNBQKB1R w KQkq - 2 3", 5, 5)

assert type(out) == tuple
assert len(out) == 5

print(out)
        

