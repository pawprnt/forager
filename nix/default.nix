{ lib
, stdenv
, cmake
, pkg-config
, wrapQtAppsHook
, qt6
, libevdev
, libsecret
, qrencode
, libglvnd
}:

stdenv.mkDerivation {
  pname = "forager";
  version = "0.5.0";

  src = ./..;

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
    libglvnd
  ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  postInstall = ''
    mkdir -p $out/share/applications
    mkdir -p $out/share/icons/hicolor/scalable/apps
    cp $src/packaging/forager.desktop $out/share/applications/forager.desktop
    cp $src/docs/forager.svg $out/share/icons/hicolor/scalable/apps/forager.svg
  '';

  meta = with lib; {
    description = "Steam-like game launcher for your local game library";
    homepage = "https://github.com/pawprnt/forager";
    license = licenses.agpl3Only;
    mainProgram = "forager";
    platforms = platforms.linux;
  };
}
