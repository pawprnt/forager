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

      devShells.${system}.default = import ./nix/shell.nix { inherit pkgs; };
    };
}
