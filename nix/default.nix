{ lib
, stdenv
, fetchFromGitHub
, cmake
, pkg-config
, wrapQtAppsHook
, qt6
, libevdev
, libsecret
, qrencode
}:

stdenv.mkDerivation {
  pname = "forager";
  version = "0.5.0-cpp";

  src = fetchFromGitHub {
    owner = "pawprnt";
    repo = "forager";
    rev = "cpp";
    # placeholder — first build will fail with "hash mismatch", nix will print
    # the actual hash. replace this line with the real hash and rebuild.
    hash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";
  };

  nativeBuildInputs = [
    cmake
    pkg-config
    wrapQtAppsHook
  ];

  buildInputs = [
    qt6.qtbase
    qt6.qtwebengine
    qt6.qtsvg
    libevdev
    libsecret
    qrencode
  ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
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

  meta = with lib; {
    description = "Steam-like game launcher for your local game library";
    homepage = "https://github.com/pawprnt/forager";
    license = licenses.agpl3Only;
    mainProgram = "forager";
    platforms = platforms.linux;
  };
}
