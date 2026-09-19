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
  version = "0.5.0";
  pyproject = true;

  src = fetchFromGitHub {
    owner = "pawprnt";
    repo = "forager";
    rev = "v0.5.1";
    # placeholder — first build will fail with "hash mismatch", nix will print
    # the actual hash. replace this line with the real hash and rebuild.
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

  postInstall = ''
    mkdir -p $out/share/applications
    mkdir -p $out/share/icons/hicolor/scalable/apps

    cat > $out/share/applications/forager.desktop << 'EOF'
[Desktop Entry]
Type=Application
Name=forager
GenericName=Game Launcher
Comment=Steam-like game launcher for your local game library
Exec=forager
Icon=forager
Terminal=false
Categories=Game;Qt;
StartupNotify=true
EOF

    cp $src/readme/forager.svg $out/share/icons/hicolor/scalable/apps/forager.svg
  '';

  pythonImportsCheck = [ "forager" ];

  meta = with lib; {
    description = "Steam-like game launcher for your local game library";
    homepage = "https://github.com/pawprnt/forager";
    license = licenses.agpl3Only;
    mainProgram = "forager";
  };
}
