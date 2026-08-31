{ lib
, buildPythonPackage
, fetchFromGitHub
, setuptools
, pyside6
, evdev
, keyring
, pillow
, qrcode
}:

buildPythonPackage {
  pname = "forager";
  version = "0.5.1";
  pyproject = true;

  src = fetchFromGitHub {
    owner = "pawprnt";
    repo = "forager";
    rev = "v0.5.1";
    hash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";
  };

  build-system = [ setuptools ];

  propagatedBuildInputs = [
    pyside6
    evdev
    keyring
    pillow
    qrcode
  ];

  pythonImportsCheck = [ "forager" ];

  meta = with lib; {
    description = "Steam-like game launcher for your local game library";
    homepage = "https://github.com/pawprnt/forager";
    license = licenses.agpl3Only;
    mainProgram = "forager";
  };
}
