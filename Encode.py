import argparse
import json
import os
import re
import sys

from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.ciphers.aead import AESGCM


def normalize_private_key_hex(text):
    v = str(text or '').strip()
    if v.lower().startswith('0x'):
        v = v[2:]
    if not re.fullmatch(r'[0-9a-fA-F]{64}', v):
        raise ValueError('private_key_hex must be 64 hex chars (optionally 0x prefixed)')
    return v.lower()


def build_plaintext(asset1, asset2, asset3):
    a1 = str(asset1 or '').strip()
    a2 = str(asset2 or '').strip()
    a3 = str(asset3 or '').strip()
    if a1 == '' or a2 == '' or a3 == '':
        raise ValueError('Asset1/Asset2/Asset3 are required')
    # Asset1|Asset2|Asset3
    return f'{a1}|{a2}|{a3}'


def build_private_key(private_key_hex):
    if private_key_hex:
        private_value = int.from_bytes(bytes.fromhex(private_key_hex), byteorder='little')
        return ec.derive_private_key(private_value, ec.SECP256R1(), default_backend())
    return ec.generate_private_key(ec.SECP256R1(), default_backend())


def encode(private_key_hex, plaintext):
    # 固定的 peer 公钥（与原脚本一致） 
    #X = 09FE84177DF0F51E1B3C32CA347F4821F3AA81364D60677E36E9A0117CD51DC8
    #Y = 284E9334F773304F5EC968F93094FD4F705CD5B8E46B29A99CD1D33BAD35A178
    x_le = bytes.fromhex('09FE84177DF0F51E1B3C32CA347F4821F3AA81364D60677E36E9A0117CD51DC8')
    y_le = bytes.fromhex('284E9334F773304F5EC968F93094FD4F705CD5B8E46B29A99CD1D33BAD35A178')

    x_int = int.from_bytes(x_le[::-1], 'big')
    y_int = int.from_bytes(y_le[::-1], 'big')
    peer_public_numbers = ec.EllipticCurvePublicNumbers(x_int, y_int, ec.SECP256R1())
    peer_public_key = peer_public_numbers.public_key(default_backend())

    private_key = build_private_key(private_key_hex)

    public_numbers = private_key.public_key().public_numbers()
    loc_x = public_numbers.x
    loc_y = public_numbers.y

    shared_key = private_key.exchange(ec.ECDH(), peer_public_key)
    aes_key = shared_key[::-1][:16]

    aesgcm = AESGCM(aes_key)
    nonce = os.urandom(12)
    aad = b'optional-auth-data'

    ciphertext_full = aesgcm.encrypt(nonce, plaintext.encode('utf-8'), aad)
    ciphertext = ciphertext_full[:-16]
    tag = ciphertext_full[-16:]

    return {
        'ciphertext': ciphertext.hex(),
        'iv': nonce.hex(),
        'tag': tag.hex(),
        'aad': aad.hex(),
        'pub': loc_x.to_bytes(32, 'big')[::-1].hex() + loc_y.to_bytes(32, 'big')[::-1].hex(),
        'plaintext': plaintext
    }


def main():
    parser = argparse.ArgumentParser(description='Encode input for Register.storeCipherData')
    parser.add_argument('--private_key_hex', help='optional private key hex (64 chars, optional 0x prefix)')
    parser.add_argument('--asset1', required=True)
    parser.add_argument('--asset2', required=True)
    parser.add_argument('--asset3', required=True)
    args = parser.parse_args()

    private_hex = normalize_private_key_hex(args.private_key_hex) if args.private_key_hex else ''
    plaintext = build_plaintext(args.asset1, args.asset2, args.asset3)
    out = encode(private_hex, plaintext)
    sys.stdout.write(json.dumps(out, ensure_ascii=False))


if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        sys.stderr.write(str(e))
        sys.exit(1)
