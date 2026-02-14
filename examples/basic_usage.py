import native_stockfish
import os
import sys

# Ensure the Stockfish binary path is correct. 
# You might need to change this path depending on where your compiled Stockfish binary is located.
# For this example, we assume it's at "../Stockfish/src/stockfish" relative to this script, 
# or provided as an environment variable STOCKFISH_PATH.
stockfish_path = os.environ.get("STOCKFISH_PATH", "Stockfish/src/stockfish")

if not os.path.exists(stockfish_path) and not os.path.exists(os.path.abspath(stockfish_path)):
    print(f"Error: Stockfish binary not found at {stockfish_path}")
    print("Please compile Stockfish or set the STOCKFISH_PATH environment variable.")
    sys.exit(1)

print(f"Using Stockfish binary at: {stockfish_path}")

try:
    client = native_stockfish.StockfishClient(stockfish_path)
    print("StockfishClient instantiated successfully")

    client.start()
    print(f"Client status: {client.status()}")

    # Example position: Start position
    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    
    # Get top 3 moves with a depth of 10
    print(f"Analyzing position: {fen}")
    top_moves = client.top_moves(fen, 10, 3)
    print("Top moves:", top_moves)

    client.stop()
    print("Client stopped.")

except Exception as e:
    print(f"An error occurred: {e}")
