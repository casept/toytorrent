let
  sources = import ./nix/sources.nix;
  nixpkgs = import sources.nixpkgs { };
in (nixpkgs.callPackage (import ./derivation.nix) { })
