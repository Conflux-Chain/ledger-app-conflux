from hashlib import sha256
from sha3 import keccak_256

from ecdsa.curves import SECP256k1
from ecdsa.keys import VerifyingKey
from ecdsa.util import sigdecode_string

from boilerplate_client.transaction import Transaction


def test_sign_tx(cmd, button):
    bip32_path: str = "m/44'/503'/1'/0/0" # 503 ~ Conflux

    pub_key, chain_code = cmd.get_public_key(
        bip32_path=bip32_path,
        display=False
    )  # type: bytes, bytes

    pk: VerifyingKey = VerifyingKey.from_string(
        pub_key,
        curve=SECP256k1,
        hashfunc=sha256
    )

    tx = Transaction(
        nonce=18,
        gasPrice=1444897209,
        gasLimit=2000000,
        to="10109fC8DF283027b6285cc889F5aA624EaC1F55",
        value=1000000000,
        storageLimit=128,
        epochHeight=1,
        chainId=1029,
        data= ''
    )

    v, der_sig = cmd.sign_tx(bip32_path=bip32_path,
                             transaction=tx,
                             button=button)

    assert pk.verify(signature=der_sig,
                     data=tx.serialize(),
                     hashfunc=keccak_256,
                     sigdecode=sigdecode_string) is True
