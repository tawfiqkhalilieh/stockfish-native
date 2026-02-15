# Native Stockfish Python Client

Native Stockfish is a minimal-overhead Python interface to the Stockfish chess engine.
Stockfish runs as a native process, with all UCI communication handled in C to avoid
Python becoming a performance bottleneck.

## Prerequisites

- Python 3.8+
- A compiled Stockfish binary. You can compile Stockfish from the official source or use the submodule included in this repository.

### Compiling Stockfish

If you have the `Stockfish` submodule checked out:

```bash
cd Stockfish/src
make -j profile-build
```

> Or you can use the binary from the website

## Installation

### From PyPi

```bash
pip install stockfish-native
```

### From Source

```bash
pip install .
```

## Usage

See `examples/basic_usage.py` for a complete example.

```python
import native_stockfish

# Path to your compiled Stockfish binary
stockfish_path = "./Stockfish/src/stockfish"

client = native_stockfish.StockfishClient(stockfish_path)
client.start()

# Analyze a position (FEN string)
# top_moves(fen, depth, count)
moves = client.top_moves("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 15, 3)
print(moves)

client.stop()
```

## API

### `StockfishClient(path)`

Initializes the client with the path to the Stockfish executable.

### `client.start()`

Starts the Stockfish engine process.

### `client.status()`

Returns the status of the engine (e.g., "running", "stopped").

### `client.top_moves(fen, depth, count)`

Analyzes the given FEN position.

- `fen`: The FEN string of the position to analyze.
- `depth`: The search depth.
- `count`: The number of top moves to return.

Returns a tuple of top moves.

### `client.stop()`

Stops the Stockfish engine process.
