import argparse
import binascii
import hashlib

import torrent_parser as tp


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", dest="percent", type=int, required=False, default=100)
    parser.add_argument("torrent_file", nargs="?")
    parser.add_argument("downloaded_file", nargs="?")
    return parser.parse_args()


def main():
    args = parse_args()
    data = tp.parse_torrent_file(args.torrent_file)

    piece_hashes = data["info"]["pieces"]
    print(f"Total pieces {len(piece_hashes)}")
    piece_length = data["info"]["piece length"]
    print(f"Piece length {piece_length}")

    pieces_to_check_count = len(piece_hashes) * args.percent // 100

    print(f"Going to check first {pieces_to_check_count} pieces of file")

    piece_id = 0
    with open(args.downloaded_file, "rb") as f:
        while piece_id < pieces_to_check_count:
            piece_bytes: bytes = f.read(piece_length)
            if not piece_bytes:
                break

            file_piece_hash = binascii.hexlify(hashlib.sha1(piece_bytes).digest()).decode()
            assert file_piece_hash == piece_hashes[piece_id], f"Hash mismatch for piece #{piece_id}"
            print(f"Checked piece #{piece_id}. Hash match")
            piece_id += 1

    assert piece_id == pieces_to_check_count, "Downloaded pieces amount is too small"


if __name__ == "__main__":
    main()
