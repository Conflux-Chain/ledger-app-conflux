from hashlib import sha256
from sha3 import keccak_256

from ecdsa.curves import SECP256k1
from ecdsa.keys import VerifyingKey
from ecdsa.util import sigdecode_string

from boilerplate_client.exception import DeviceException
from boilerplate_client.utils import enable_blind_sign

def check_message(cmd, button, bip32_path, msg_hex, num_clicks=5):
    pub_key, chain_code = cmd.get_public_key(bip32_path=bip32_path)
    pk: VerifyingKey = VerifyingKey.from_string(pub_key, curve=SECP256k1, hashfunc=sha256)
    v, sig = cmd.sign_personal(bip32_path=bip32_path, msg=bytes.fromhex(msg_hex), button=button, num_clicks=num_clicks)

    prefix = '\u0019Conflux Signed Message:\n' + str(len(msg_hex) // 2)
    encoded = prefix.encode("utf-8").hex() + msg_hex

    assert pk.verify(signature=sig, data=bytes.fromhex(encoded), hashfunc=keccak_256, sigdecode=sigdecode_string) is True

def test_sign_personal(cmd, button):
    enable_blind_sign(cmd, button)

    check_message(cmd, button, "m/44'/503'/0'/0/0", "11")
    check_message(cmd, button, "m/44'/503'/0'/0/1", "1122")
    check_message(cmd, button, "m/44'/503'/0'/0/2", "abcdefabcdef")

    # "Hello, world!" (utf-8) -> 0x48656c6c6f2c20776f726c6421 (hex)
    # message hash: 0x315f5bdb76d078c43b8ac0064e4a0164612b1fce77c869345bfc94c75894edd3
    check_message(cmd, button, "m/44'/503'/0'/0/0", "48656c6c6f2c20776f726c6421")

    # Blockchain Without Barriers\nConflux enables creators, communities, and markets to connect across borders and protocols (utf-8)
    # 0x426c6f636b636861696e20576974686f75742042617272696572730a436f6e666c757820656e61626c65732063726561746f72732c20636f6d6d756e69746965732c20616e64206d61726b65747320746f20636f6e6e656374206163726f737320626f726465727320616e642070726f746f636f6c73 (hex)
    # message hash: 0x4e21384f04f8259bb2a1edcba8216ed2b2eb5fc7e232be571afcca95fec2d4a5
    check_message(cmd, button, "m/44'/503'/0'/0/0", "426c6f636b636861696e20576974686f75742042617272696572730a436f6e666c757820656e61626c65732063726561746f72732c20636f6d6d756e69746965732c20616e64206d61726b65747320746f20636f6e6e656374206163726f737320626f726465727320616e642070726f746f636f6c73")


