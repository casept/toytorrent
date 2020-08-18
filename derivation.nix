{ stdenv, cmake, gtest }:
stdenv.mkDerivation {
  name = "toytorrent";
  src = ./.;
  nativeBuildInputs = [ cmake gtest ];
}
