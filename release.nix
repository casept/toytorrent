let
  sources = import ./nix/sources.nix;
  nixpkgs = import sources.nixpkgs { };
  cpr = (nixpkgs.callPackage ./nix/pkgs/cpr { });
in (nixpkgs.callPackage (import ./derivation.nix) { cpr = cpr; })
