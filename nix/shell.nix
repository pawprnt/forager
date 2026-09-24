{ pkgs }:

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    cmake
    pkg-config
    just
    clang-tools
  ];

  buildInputs = with pkgs; [
    qt6.qtbase
    qt6.qtwebengine
    qt6.qtsvg
    libevdev
    libsecret
    qrencode
    libglvnd
  ];

  shellHook = ''
    export QT_QPA_PLATFORM=''${QT_QPA_PLATFORM:-offscreen}
    export QTWEBENGINE_DISABLE_SANDBOX=1
    echo "forager cpp dev shell"
  '';
}
