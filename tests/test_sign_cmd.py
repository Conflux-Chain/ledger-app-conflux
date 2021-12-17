from hashlib import sha256
from sha3 import keccak_256

from ecdsa.curves import SECP256k1
from ecdsa.keys import VerifyingKey
from ecdsa.util import sigdecode_string

from boilerplate_client.exception import DeviceException
from boilerplate_client.transaction import Transaction
from boilerplate_client.utils import UINT64_MAX, enable_blind_sign, disable_blind_sign

def check_transaction(cmd, button, bip32_path, tx, num_clicks=6):
    pub_key, chain_code = cmd.get_public_key(bip32_path=bip32_path)
    pk: VerifyingKey = VerifyingKey.from_string(pub_key, curve=SECP256k1, hashfunc=sha256)
    v, sig = cmd.sign_tx(bip32_path=bip32_path, transaction=tx, button=button, num_clicks=num_clicks)
    assert pk.verify(signature=sig, data=tx.serialize(), hashfunc=keccak_256, sigdecode=sigdecode_string) is True

def check_transaction_fails(cmd, button, bip32_path, tx, num_clicks=6):
    pub_key, chain_code = cmd.get_public_key(bip32_path=bip32_path)
    pk: VerifyingKey = VerifyingKey.from_string(pub_key, curve=SECP256k1, hashfunc=sha256)

    try:
        v, sig = cmd.sign_tx(bip32_path=bip32_path, transaction=tx, button=button, num_clicks=num_clicks)
        assert(False)
    except:
        pass

def test_sign_tx_blind_sign_disabled(cmd, button):
    disable_blind_sign(cmd, button)

    # can blind sign simple transfer
    check_transaction(cmd, button, "m/44'/503'/0'/0/0", num_clicks=4, tx=Transaction())

    # cannot blind sign contract call
    check_transaction_fails(cmd, button, "m/44'/503'/0'/0/0", num_clicks=0, tx=Transaction(
        to="85d68694f81eE2389E7f910937f9A606615Df6FD",
        data= "d333638f"
    ))

def test_sign_tx_blind_sign_enabled(cmd, button):
    enable_blind_sign(cmd, button)

    check_transaction(cmd, button, "m/44'/503'/0'/0/0", num_clicks=4, tx=Transaction())

    check_transaction_fails(cmd, button, "m/44'/503'/0'/0/0", Transaction(
        to="85d68694f81eE2389E7f910937f9A606615Df6FD",
        data= "d333638f"
    ))

    check_transaction(cmd, button, "m/44'/503'/0'/0/1", Transaction(
        nonce=0x98967f, # max nonce displayed correctly
        chainId=0xffff, # maximum chain ID allowed

        gasPrice=1000000000000000000,
        gasLimit=1000000000000000000,
        storageLimit=0xffffffff,
        value=0xd3c21d5599586e4b0000, # 1_000_000.11 CFX

        # this is not displayed so let's try the largest value allowed
        epochHeight=0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff,

        # a real contract deployment transaction
        to="",
        data="608060405234801561001057600080fd5b50336000806101000a81548173ffffffffffffffffffffffffffffffffffffffff021916908373ffffffffffffffffffffffffffffffffffffffff1602179055506060600167ffffffffffffffff8111801561006b57600080fd5b5060405190808252806020026020018201604052801561009a5781602001602082028036833780820191505090505b5090506000816000815181106100ac57fe5b602002602001019073ffffffffffffffffffffffffffffffffffffffff16908173ffffffffffffffffffffffffffffffffffffffff168152505073088800000000000000000000000000000000000173ffffffffffffffffffffffffffffffffffffffff166310128d3e826040518263ffffffff1660e01b81526004018080602001828103825283818151815260200191508051906020019060200280838360005b8381101561016957808201518184015260208101905061014e565b5050505090500192505050600060405180830381600087803b15801561018e57600080fd5b505af11580156101a2573d6000803e3d6000fd5b5050505050610443806101b66000396000f3fe608060405234801561001057600080fd5b506004361061004c5760003560e01c806307546172146100515780631d073a7a14610085578063390d1670146100b9578063d333638f14610107575b600080fd5b610059610175565b604051808273ffffffffffffffffffffffffffffffffffffffff16815260200191505060405180910390f35b61008d610199565b604051808273ffffffffffffffffffffffffffffffffffffffff16815260200191505060405180910390f35b610105600480360360408110156100cf57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506101b1565b005b6101736004803603606081101561011d57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610323565b005b60008054906101000a900473ffffffffffffffffffffffffffffffffffffffff1681565b73088800000000000000000000000000000000000181565b60008054906101000a900473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff1614610272576040517f08c379a000000000000000000000000000000000000000000000000000000000815260040180806020018281038252600b8152602001807f6f776e6572206572726f7200000000000000000000000000000000000000000081525060200191505060405180910390fd5b8173ffffffffffffffffffffffffffffffffffffffff1663a9059cbb33836040518363ffffffff1660e01b8152600401808373ffffffffffffffffffffffffffffffffffffffff16815260200182815260200192505050602060405180830381600087803b1580156102e357600080fd5b505af11580156102f7573d6000803e3d6000fd5b505050506040513d602081101561030d57600080fd5b8101908080519060200190929190505050505050565b60005b81811015610407578373ffffffffffffffffffffffffffffffffffffffff166323b872dd338560006040518463ffffffff1660e01b8152600401808473ffffffffffffffffffffffffffffffffffffffff1681526020018373ffffffffffffffffffffffffffffffffffffffff1681526020018281526020019350505050602060405180830381600087803b1580156103be57600080fd5b505af11580156103d2573d6000803e3d6000fd5b505050506040513d60208110156103e857600080fd5b8101908080519060200190929190505050508080600101915050610326565b5050505056fea26469706673582212206407897ec0c4bdc03558473d2d51ee869547fd6f7cb4266dbabc232cace143a864736f6c634300060c0033"
    ))

    check_transaction(cmd, button, "m/44'/503'/0'/0/2", Transaction(
        nonce=18,
        gasPrice=1444897209,
        gasLimit=2000000,
        to="10109fC8DF283027b6285cc889F5aA624EaC1F55",
        value=1000000000,
        storageLimit=128,
        epochHeight=1,
        chainId=1029,
        data= ''
    ))

    check_transaction(cmd, button, "m/44'/503'/0'/0/3", num_clicks=7, tx=Transaction(
        to="10109fC8DF283027b6285cc889F5aA624EaC1F55",
        chainId=0xffffffff
    ))
