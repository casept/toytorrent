let
  sources = import ./nix/sources.nix;
  niv = import sources.niv { inherit sources; };
  arion = import sources.arion {};
  pkgs = import sources.nixpkgs {};
in
pkgs.mkShell {
  CMAKE_INCLUDE_PATH = "${pkgs.curlFull.dev}/include";
  CMAKE_LIBRARY_PATH = "${pkgs.curlFull}/lib";
  CMAKE_PREFIX_PATH = "${pkgs.curlFull}";
  LOCALE_ARCHIVE = "${pkgs.glibcLocales}/lib/locale/locale-archive";
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
    pkgs.flawfinder
    pkgs.include-what-you-use
    pkgs.python3
    pkgs.valgrind
    pkgs.gdb
    (
      pkgs.callPackage ./nix/pkgs/camomilla {
        pythonXXPackages = pkgs.python39Packages;
      }
    )
    (pkgs.callPackage ./nix/pkgs/lizard {})

    # Libraries
    pkgs.botan2
    pkgs.curlFull
    (pkgs.callPackage ./nix/pkgs/cpr {})
    pkgs.fmt

    # For tests
    pkgs.gtest
    pkgs.aria2
    pkgs.opentracker
    pkgs.boost175

    # Nix support
    pkgs.niv
    arion.arion
  ];
}
