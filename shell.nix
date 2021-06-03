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
    # Dev tooling
    pkgs.clang_12
    pkgs.clang-tools
    pkgs.ccache
    pkgs.cmake
    pkgs.extra-cmake-modules
    pkgs.cmake-format
    pkgs.pkgconfig
    pkgs.ninja
    pkgs.cppcheck
    pkgs.include-what-you-use
    pkgs.python3
    pkgs.valgrind
    pkgs.gdb

    # Libraries
    pkgs.botan2
    pkgs.curlFull
    (pkgs.callPackage ./nix/pkgs/cpr { })
    pkgs.fmt

    # For tests
    pkgs.gtest
    pkgs.aria2
    pkgs.opentracker
    pkgs.boost

    # Nix support
    pkgs.niv
    arion.arion
  ];
}
