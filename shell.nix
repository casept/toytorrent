let
  sources = import ./nix/sources.nix;
  niv = import sources.niv { inherit sources; };
  arion = import sources.arion { };
  nixpkgs = import sources.nixpkgs { };
in { pkgs ? nixpkgs }:
pkgs.mkShell {
  buildInputs = [
    # Build tooling
    pkgs.cmake
    pkgs.cppcheck

    # Libraries we depend on
    pkgs.botan2

    # Nix-specific
    niv.niv
    arion.arion
  ];
}
