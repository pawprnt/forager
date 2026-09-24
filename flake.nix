{
  description = "forager – local game launcher (C++)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};

      forager = pkgs.callPackage ./nix/default.nix { };
    in {
      packages.${system} = {
        inherit forager;
        default = forager;
      };

      devShells.${system}.default = pkgs.mkShell {
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
          echo "forager cpp dev shell"
        '';
      };
    };
}
