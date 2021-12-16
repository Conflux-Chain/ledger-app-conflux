import struct
from typing import Tuple
import time

from ledgercomm import Transport

from boilerplate_client.boilerplate_cmd_builder import BoilerplateCommandBuilder, InsType
from boilerplate_client.button import Button
from boilerplate_client.exception import DeviceException
from boilerplate_client.transaction import Transaction


class BoilerplateCommand:
    def __init__(self,
                 transport: Transport,
                 debug: bool = False) -> None:
        self.transport = transport
        self.builder = BoilerplateCommandBuilder(debug=debug)
        self.debug = debug

    def get_app_and_version(self) -> Tuple[str, str]:
        sw, response = self.transport.exchange_raw(
            self.builder.get_app_and_version()
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=0x01)

        # response = format_id (1) ||
        #            app_name_len (1) ||
        #            app_name (var) ||
        #            version_len (1) ||
        #            version (var) ||
        offset: int = 0

        format_id: int = response[offset]
        offset += 1
        app_name_len: int = response[offset]
        offset += 1
        app_name: str = response[offset:offset + app_name_len].decode("ascii")
        offset += app_name_len
        version_len: int = response[offset]
        offset += 1
        version: str = response[offset:offset + version_len].decode("ascii")
        offset += version_len

        return app_name, version

    def get_version(self) -> Tuple[int, int, int]:
        sw, response = self.transport.exchange_raw(
            self.builder.get_version()
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_VERSION)

        # response = MAJOR (1) || MINOR (1) || PATCH (1)
        assert len(response) == 3

        major, minor, patch = struct.unpack(
            "BBB",
            response
        )  # type: int, int, int

        return major, minor, patch

    def get_app_name(self) -> str:
        sw, response = self.transport.exchange_raw(
            self.builder.get_app_name()
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_APP_NAME)

        return response.decode("ascii")

    def get_app_info(self) -> Tuple[bool, bool, int, int, int]:
        sw, response = self.transport.exchange_raw(
            self.builder.get_app_info()
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_VERSION)

        # response = flags(1) || MAJOR (1) || MINOR (1) || PATCH (1)
        assert len(response) == 4

        flags, major, minor, patch = struct.unpack(
            "BBBB",
            response
        )  # type: int, int, int, int

        blind_signing_enabled = (flags & 0x01) > 0
        detailed_display_enabled = (flags & 0x02) > 0

        return blind_signing_enabled, detailed_display_enabled, major, minor, patch

    def get_public_key(self, bip32_path: str, display: bool = False, getChaincode: bool = False) -> Tuple[bytes, bytes]:
        sw, response = self.transport.exchange_raw(
            self.builder.get_public_key(bip32_path=bip32_path,
                                        display=display,
                                        getChaincode=getChaincode)
        )  # type: int, bytes

        if sw != 0x9000:
            raise DeviceException(error_code=sw, ins=InsType.INS_GET_PUBLIC_KEY)

        # response = pub_key_len (1) ||
        #            pub_key (var) ||
        #            chain_code_len (1) ||
        #            chain_code (var)
        offset: int = 0

        pub_key_len: int = response[offset]
        offset += 1
        pub_key: bytes = response[offset:offset + pub_key_len]
        offset += pub_key_len

        chain_code: bytes = b""
        chain_code_len: int = -1

        if getChaincode:
            chain_code_len = response[offset]
            offset += 1
            chain_code = response[offset:offset + chain_code_len]
            offset += chain_code_len

        assert len(response) == 1 + pub_key_len + 1 + chain_code_len
        return pub_key, chain_code

    def sign_tx(self, bip32_path: str, transaction: Transaction, button: Button, num_clicks: int = 6) -> Tuple[int, bytes]:
        sw: int
        response: bytes = b""

        for is_last, chunk in self.builder.sign_tx(bip32_path=bip32_path, transaction=transaction):
            self.transport.send_raw(chunk)

            # Make sure we wait until transaction is displayed
            time.sleep(0.5)

            if is_last:
                for _ in range(num_clicks):
                    button.right_click()

                button.both_click()

            sw, response = self.transport.recv()  # type: int, bytes

            if sw != 0x9000:
                raise DeviceException(error_code=sw, ins=InsType.INS_SIGN_TX)

        return response[:1], response[1:]
