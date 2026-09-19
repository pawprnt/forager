{
  description = "forager C++ dev shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          cmake
          pkg-config
          qt6.qtbase
          qt6.qtwebengine
          qt6.qtsvg
          libevdev
          libsecret
          qrencode
          clang-tools
          just
        ];

        shellHook = ''
          echo "forager cpp dev shell"
        '';
      };
    };
}
