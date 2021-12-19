# End-to-end tests

These tests are implemented in Python and can be executed either using the [Speculos](https://github.com/LedgerHQ/speculos) emulator or a Ledger Nano S/X.
Python dependencies are listed in [requirements.txt](requirements.txt), install them using [pip](https://pypi.org/project/pip/)

```
pip install -r requirements.txt
```

### Launch with Speculos

First start your application with Speculos

```
./path/to/speculos.py /path/to/app-conflux/bin/app.elf --display headless --apdu-port 9999 --button-port 42000 /speculos/app-conflux/bin/app.elf --seed "clown disagree excess state tray tongue use fat teach woman dizzy include"
```

then in the `tests` folder run

```
pytest --headless
```

### Launch with your Nano S/X

To run the tests on your Ledger Nano S/X you also need to install an optional dependency

```
pip install ledgercomm[hid]
```

Be sure to have you device connected through USB (without any other software interacting with it) and run

```
pytest --hid
```
