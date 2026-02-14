import native_stockfish

client = native_stockfish.StockfishClient("Stockfish/src/stockfish");
print("StockfishClient instantiated successfully")


client.start()
print(client.status())   # "running"

print(client.top_moves("r1bqkbnr/ppp1pppp/2n5/3p4/3P4/5N2/PPP1PPPP/RNBQKB1R w KQkq - 2 3", 5, 5))
client.stop()
