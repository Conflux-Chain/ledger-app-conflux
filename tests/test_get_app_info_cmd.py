def test_get_app_info(cmd):
    _, _, MAJOR, MINOR, PATCH = cmd.get_app_info()
    assert (MAJOR, MINOR, PATCH) == (1, 0, 0)
