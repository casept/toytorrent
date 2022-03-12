{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    nix-direnv.url = "github:nix-community/nix-direnv";
    arion.url = "github:hercules-ci/arion";
  };

  outputs = { self, nixpkgs, flake-utils, nix-direnv, arion }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShell = pkgs.mkShell {
          CMAKE_INCLUDE_PATH = "${pkgs.curlFull.dev}/include:${pkgs.botan2}/include/botan-2";
          CMAKE_LIBRARY_PATH = "${pkgs.curlFull}/lib";
          CMAKE_PREFIX_PATH = "${pkgs.curlFull}";
          LOCALE_ARCHIVE = "${pkgs.glibcLocales}/lib/locale/locale-archive";
          buildInputs = with pkgs; [
            # Dev tooling
            pkgs.clang_13
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
            (pkgs.callPackage ./nix/pkgs/lizard { })

            # Libraries
            pkgs.botan2
            pkgs.curlFull
            (pkgs.callPackage ./nix/pkgs/cpr { })
            pkgs.fmt_8

            # For tests
            pkgs.gtest
            pkgs.aria2
            pkgs.opentracker
            pkgs.boost175

            # Nix support
            pkgs.rnix-lsp
            pkgs.nixpkgs-fmt
            arion
          ];
        };
      });
}
