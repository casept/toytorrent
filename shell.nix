let
  sources = import ./nix/sources.nix;
  niv = import sources.niv { inherit sources; };
  arion = import sources.arion { };
  nixpkgs = import sources.nixpkgs { };
in { pkgs ? nixpkgs }:
pkgs.mkShell {
  CMAKE_INCLUDE_PATH = "${pkgs.curlFull.dev}/include";
  CMAKE_LIBRARY_PATH = "${pkgs.curlFull}/lib";
  CMAKE_PREFIX_PATH = "${pkgs.curlFull.dev}";
  buildInputs = [
    # Build tooling
    pkgs.cmake
    pkgs.extra-cmake-modules
    pkgs.pkgconfig
    pkgs.cppcheck
    pkgs.gtest

    # Libraries we depend on
    pkgs.botan2
    pkgs.curlFull
    pkgs.curlFull.dev
    (pkgs.callPackage ./nix/pkgs/cpr { })

    # Nix-specific
    niv.niv
    arion.arion
  ];
}
