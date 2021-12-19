def test_get_app_info(cmd):
    _, _, MAJOR, MINOR, PATCH = cmd.get_app_info()
    assert (MAJOR, MINOR, PATCH) == (0, 0, 3)
