from io import BytesIO
from typing import Union
from rlp import encode

from boilerplate_client.utils import (read, read_uint, read_varint,
                                      write_varint, UINT64_MAX)

# https://stackoverflow.com/a/30375198
def int_to_bytes(x: int) -> bytes:
    return x.to_bytes((x.bit_length() + 7) // 8, 'big')


class TransactionError(Exception):
    pass


class Transaction:
    def __init__(
        self,
        nonce: int,
        gasPrice: int,
        gasLimit: int,
        to: str,
        value: int,
        data: str,
        storageLimit: int,
        epochHeight: int,
        chainId: int
    ) -> None:
        self.nonce: int = nonce
        self.gasPrice: int = gasPrice
        self.gasLimit: int = gasLimit
        self.to: str = to
        self.value: int = value
        self.data: str = data
        self.storageLimit: int = storageLimit
        self.epochHeight: int = epochHeight
        self.chainId: int = chainId

        if not (0 <= self.nonce <= UINT64_MAX):
            raise TransactionError(f"Bad nonce: '{self.nonce}'!")

        if not (0 <= self.value <= UINT64_MAX):
            raise TransactionError(f"Bad value: '{self.value}'!")

        if len(self.to) != 40:
            raise TransactionError(f"Bad address: '{self.to}'!")

    def serialize(self) -> bytes:
        return encode([
            int_to_bytes(self.nonce),
            int_to_bytes(self.gasPrice),
            int_to_bytes(self.gasLimit),
            bytes.fromhex(self.to),
            int_to_bytes(self.value),
            int_to_bytes(self.storageLimit),
            int_to_bytes(self.epochHeight),
            int_to_bytes(self.chainId),
            bytes.fromhex(self.data)
        ])
