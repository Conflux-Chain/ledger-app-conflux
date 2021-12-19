def test_get_public_key(cmd):
    # These tests are using the mnemonic "clown disagree excess state tray tongue use fat teach woman dizzy include".
    # You can pass it to Speculos using the `--seed` parameter.
    #
    # To generate the pubkey and chaincode using js:
    #
    # ```js
    # const ethers = require('ethers');
    # const node = ethers.utils.HDNode.fromMnemonic('...');
    # console.log(node.derivePath("m/44'/503'/0'/0/0"));
    # ```
    #
    # Uncompressed public key is: 0x04 + x-coordinate + y-coordinate
    # Compressed public key is: 0x02 + x-coordinate if y is even, 0x03 + x-coordinate if y is odd
    # (Source: https://bitcointalk.org/index.php?topic=644919.0)
    # You can use this tool to convert between uncompressed and compressed public keys: https://learnmeabitcoin.com/technical/public-key.

    pub_key, chain_code = cmd.get_public_key(bip32_path="m/44'/503'/0'/0/0", display=False, getChaincode=True)
    assert pub_key.hex() == "044b0e7aa12422c8bd4b2bed18bba775e0271e6b9a99a1bf5340572aceeb80e0350588d6dcc89707412b1276e4cdfbc5fc65910696c689465611b12dda5b0700db"
    assert chain_code.hex() == "0aa4aff738d5f103af841557530f6f1f70de579f5092b7a7791f0a03b348a015"

    pub_key, chain_code = cmd.get_public_key(bip32_path="m/44'/503'/0'/0/0", display=False, getChaincode=False)
    assert pub_key.hex() == "044b0e7aa12422c8bd4b2bed18bba775e0271e6b9a99a1bf5340572aceeb80e0350588d6dcc89707412b1276e4cdfbc5fc65910696c689465611b12dda5b0700db"
    assert chain_code.hex() == ""

    pub_key, chain_code = cmd.get_public_key(bip32_path="m/44'/1'/0'/0/0", display=False, getChaincode=True)
    assert pub_key.hex() == "04d5732cfb8b7ebf1cf7c8e3a2f698dd157affacc871f76b75ef4fc2ee82b89814fa51af502fefc23caa7d9be319d076acf2d70606e10719731e8aaafdeb188350"
    assert chain_code.hex() == "9064770a4186b1bd597c390ba0a50042a2fca52cd72662f1b039735332ec41ee"

    pub_key, chain_code = cmd.get_public_key(bip32_path="m/44'/503'/1'/0/0", display=False, getChaincode=True)
    assert pub_key.hex() == "04ed2bf23b16f5ff6bc41e1db530e43d611871354489ac59c32b04a652c4e967541122fb252da05a7bb39f88878c15c58cd2a5bc359c574ff63f0c4ef4827ea011"
    assert chain_code.hex() == "ab6ab09604fc6b33c63601bd0f842c57b6ae22a0812e13269730944616d7be07"

    pub_key, chain_code = cmd.get_public_key(bip32_path="m/44'/503'/0'/1/0", display=False, getChaincode=True)
    assert pub_key.hex() == "04ad86edf4b4f4eaf1e4543a939e1f23460b6b99cb45ea7d7233995bf2e21aa071c7ae91aa1ca9ec70405dccdf575b66addddfd3a9559436bc949014fede8eca76"
    assert chain_code.hex() == "c17897587167f022ea3e183fb696d44762b75276f1aa91199882fa7722c6d498"

    pub_key, chain_code = cmd.get_public_key(bip32_path="m/44'/503'/0'/0/1", display=False, getChaincode=True)
    assert pub_key.hex() == "04aa3de0d79a5e921ad2f7cf31d5789a66fab3c79cb5dd1a7945a6a5c56fb4713ae9a9973980141a1290738650b94df908d5c58f0b83e43cdf9b8702233428df8b"
    assert chain_code.hex() == "cc9f814e6f39914313a75e4a3c26cd89d82add537ac4e5724a480b44591f7a37"

    pub_key, chain_code = cmd.get_public_key(bip32_path="m/44'/503'/0'/0/2", display=False, getChaincode=True)
    assert pub_key.hex() == "04e472c39e03abadd38330551810a64ba99e11370fea7e168ffd7409d94189ec5cd4fdc11f4120b0087437f2dedae2a434bec44c630d28b8c62500dd5a2a70b370"
    assert chain_code.hex() == "f397a6debf4396975d5fd480dbd461ea2253d17b3b25c86982693a125b583ae3"
