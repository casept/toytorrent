let
  sources = import ./nix/sources.nix;
  niv = import sources.niv { inherit sources; };
  arion = import sources.arion { };
  pkgs = import sources.nixpkgs { };
in pkgs.mkShell {
  CMAKE_INCLUDE_PATH = "${pkgs.curlFull.dev}/include";
  CMAKE_LIBRARY_PATH = "${pkgs.curlFull}/lib";
  CMAKE_PREFIX_PATH = "${pkgs.curlFull}";
  buildInputs = [
    # Build tooling
    pkgs.cmake
    pkgs.extra-cmake-modules
    pkgs.cmake-format
    pkgs.pkgconfig
    pkgs.cppcheck
    pkgs.gtest
    pkgs.clang-tools

    # Libraries we depend on
    pkgs.botan2
    pkgs.curlFull
    (pkgs.callPackage ./nix/pkgs/cpr { })

    # Nix-specific
    pkgs.niv
    arion.arion
  ];
}
