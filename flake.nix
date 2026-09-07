{
  description = "forager – local game launcher";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        python = pkgs.python3;

        forager = python.pkgs.buildPythonApplication {
          pname = "forager";
          version = "0.5.0";
          format = "pyproject";
          src = ./.;

          nativeBuildInputs = [
            python.pkgs.setuptools
          ];

          propagatedBuildInputs = with python.pkgs; [
            pyside6
            evdev
            keyring
            pillow
            qrcode
          ];

          nativeCheckInputs = [
            python.pkgs.pytest
          ];

          # Tests need Qt display
          QT_QPA_PLATFORM = "offscreen";
          QTWEBENGINE_DISABLE_SANDBOX = "1";

          checkPhase = ''
            runHook preCheck
            python3 -m pytest scripts/testing/ -q --tb=short \
              --ignore=scripts/testing/ui/test_main_window_smoke.py \
              --ignore=scripts/testing/ui/test_store.py \
              -k "not test_account_name"
            runHook postCheck
          '';

          meta = with pkgs.lib; {
            description = "Steam-like game launcher for your local game library";
            homepage = "https://github.com/user/forager";
            license = licenses.mit;
            mainProgram = "forager";
          };
        };
      in
      {
        packages = {
          inherit forager;
          default = forager;
        };

        devShells.default = pkgs.mkShell {
          buildInputs = [
            (python.withPackages (ps: with ps; [
              pyside6
              pytest
              evdev
              keyring
              pillow
              qrcode
            ]))

            pkgs.xorg.libX11
            pkgs.xorg.libxcb
            pkgs.libxkbcommon
            pkgs.libGL
            pkgs.mesa
            pkgs.qt6.qtbase
            pkgs.fontconfig
            pkgs.freetype
            pkgs.xorg.xorgserver
          ];

          shellHook = ''
            export QT_QPA_PLATFORM=offscreen
            export QTWEBENGINE_DISABLE_SANDBOX=1
            export PYTHONPATH="src:$PYTHONPATH"
            echo "forager dev shell ready (nix + pytest)"
          '';
        };
      });
}
